
#include "tjsCommHead.h"

#include <algorithm>
#include <vector>

#include "tvpfontstruc.h"
#include "TVPSysFont.h"
#include "FontSystem.h"
#include "LayerBitmapFontManagement.h"

static bool IsInitDefalutFontName = false;
static tjs_string TVPDefaultFaceNames;
static tjs_string TVPDefaultFontName = TJS_W("DEFAULT_GUI_FONT");

/**
 * ストックフォント指定の場合、システムからフォルト名を取得して、そのオブジェクトをデフォルトとする
 */
const tjs_char *TVPGetDefaultFontName() {
	if( IsInitDefalutFontName ) {
		return TVPDefaultFontName.c_str();
	}

	// コマンドラインで指定がある場合、そのフォントを使用する
	tTJSVariant opt;
	if(TVPGetCommandLine(TJS_W("-deffont"), &opt)) {
		ttstr str(opt);
		TVPDefaultFontName = str.c_str() ;
	}
	IsInitDefalutFontName =  true;

	// システム定義のフォントかどうかチェックする
	ttstr name = ttstr(TVPDefaultFontName.c_str());
	HGDIOBJ obj = NULL;
	if( name == ttstr(TJS_W("ANSI_FIXED_FONT")) ) {
		obj = ::GetStockObject(ANSI_FIXED_FONT);
	} else if( name == ttstr(TJS_W("ANSI_VAR_FONT")) ) {
		obj = ::GetStockObject(ANSI_VAR_FONT);
	} else if( name == ttstr(TJS_W("DEVICE_DEFAULT_FONT")) ) {
		obj = ::GetStockObject(DEVICE_DEFAULT_FONT);
	} else if( name == ttstr(TJS_W("DEFAULT_GUI_FONT")) ) {
		obj = ::GetStockObject(DEFAULT_GUI_FONT);
	} else if( name == ttstr(TJS_W("OEM_FIXED_FONT")) ) {
		obj = ::GetStockObject(OEM_FIXED_FONT);
	} else if( name == ttstr(TJS_W("SYSTEM_FONT")) ) {
		obj = ::GetStockObject(SYSTEM_FONT);
	} else if( name == ttstr(TJS_W("SYSTEM_FIXED_FONT")) ) {
		obj = ::GetStockObject(SYSTEM_FIXED_FONT);
	}
	if( obj != NULL ) {
		HFONT font = (HFONT)obj;
		LOGFONT logfont={0};
		::GetObject( font, sizeof(LOGFONT), &logfont );
		TVPDefaultFontName = logfont.lfFaceName ;
	}
	return TVPDefaultFontName.c_str();
}
/**
 * Windowsの場合はデフォルトフォントと同じ
 */
const tjs_string TVPGetDefaultFaceNames() {
	if( TVPDefaultFaceNames.length() != 0 ) {
		return TVPDefaultFaceNames.c_str();
	} else {
		TVPDefaultFaceNames = tjs_string( TVPGetDefaultFontName() );
		return TVPDefaultFaceNames.c_str();
	}
}
void TVPSetDefaultFontName( const tjs_char * name ) {
	TVPDefaultFontName = name ;
	IsInitDefalutFontName = true;
}
void tTVPSysFont::InitializeMemDC() {
	BITMAPINFO bmpinfo;
	ZeroMemory( &bmpinfo, sizeof(bmpinfo) );
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biBitCount = 32;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biWidth = 32;
	bmpinfo.bmiHeader.biHeight = 32;

	hMemDC_ = ::CreateCompatibleDC( NULL );
	char * Bits;
	hBmp_ = ::CreateDIBSection( NULL, &bmpinfo, DIB_RGB_COLORS, (void **)(&Bits), NULL, 0 );
	hOldBmp_ = (HBITMAP)::SelectObject( hMemDC_, hBmp_ );
}

tTVPSysFont::tTVPSysFont() : hFont_((HFONT)INVALID_HANDLE_VALUE), hOldFont_((HFONT)INVALID_HANDLE_VALUE), hMemDC_((HDC)INVALID_HANDLE_VALUE),
	hBmp_((HBITMAP)INVALID_HANDLE_VALUE), hOldBmp_((HBITMAP)INVALID_HANDLE_VALUE) {

	InitializeMemDC();

	HFONT hFont = (HFONT)::GetStockObject( ANSI_FIXED_FONT );
	LOGFONT logfont={0};
	::GetObject( hFont_, sizeof(LOGFONT), &logfont );
	logfont.lfHeight = -12;
	logfont.lfWidth = 0;
	logfont.lfCharSet = DEFAULT_CHARSET;
	TJS_strncpy( logfont.lfFaceName, TVPGetDefaultFontName(), LF_FACESIZE );
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	ApplyFont( &logfont );
}
tTVPSysFont::tTVPSysFont( const tTVPFont &font ) : hFont_((HFONT)INVALID_HANDLE_VALUE), hOldFont_((HFONT)INVALID_HANDLE_VALUE), hMemDC_((HDC)INVALID_HANDLE_VALUE),
	hBmp_((HBITMAP)INVALID_HANDLE_VALUE), hOldBmp_((HBITMAP)INVALID_HANDLE_VALUE) {

	InitializeMemDC();

	HFONT hFont = (HFONT)::GetStockObject( ANSI_FIXED_FONT );
	LOGFONT LogFont={0};
	LogFont.lfHeight = -std::abs(font.Height);
	LogFont.lfItalic = (font.Flags & TVP_TF_ITALIC) ? TRUE:FALSE;
	LogFont.lfWeight = (font.Flags & TVP_TF_BOLD) ? 700 : 400;
	LogFont.lfUnderline = (font.Flags & TVP_TF_UNDERLINE) ? TRUE:FALSE;
	LogFont.lfStrikeOut = (font.Flags & TVP_TF_STRIKEOUT) ? TRUE:FALSE;
	LogFont.lfEscapement = LogFont.lfOrientation = font.Angle;
	LogFont.lfCharSet = DEFAULT_CHARSET;
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfQuality = DEFAULT_QUALITY;
	LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	tjs_string face = TVPFontSystem->GetBeingFont(font.Face.c_str());
	TJS_strncpy( LogFont.lfFaceName, face.c_str(), LF_FACESIZE -1);
	LogFont.lfFaceName[LF_FACESIZE-1] = 0;

	ApplyFont( &LogFont );
}
tTVPSysFont::~tTVPSysFont() {
	::SelectObject( hMemDC_, hOldBmp_ );
	if( INVALID_HANDLE_VALUE != hOldFont_ ) {
		::SelectObject( hMemDC_, hOldFont_ );
	}
	if( hFont_ != INVALID_HANDLE_VALUE ) {
		::DeleteObject( hFont_ );
	}
	::DeleteObject( hBmp_ );
	::DeleteDC( hMemDC_ );
}

int tTVPSysFont::GetAscentHeight() {
	int otmSize = ::GetOutlineTextMetrics( hMemDC_, 0, NULL );
	char *otmBuf = new char[otmSize];
	OUTLINETEXTMETRIC *otm = (OUTLINETEXTMETRIC*)otmBuf;
	::GetOutlineTextMetrics( hMemDC_, otmSize, otm );
	int result = otm->otmAscent;
	//int result = otm->otmMacAscent;
	delete[] otmBuf;
	return result;
}

bool tTVPSysFont::Assign( const tTVPSysFont* font ) {
	LOGFONT logfont = {0};
	font->GetFont( &logfont );
	return ApplyFont( &logfont );
}
bool tTVPSysFont::Assign( const tTVPFont &font ) {
	LOGFONT LogFont={0};
	LogFont.lfHeight = -std::abs(font.Height);
	LogFont.lfItalic = (font.Flags & TVP_TF_ITALIC) ? TRUE:FALSE;
	LogFont.lfWeight = (font.Flags & TVP_TF_BOLD) ? 700 : 400;
	LogFont.lfUnderline = (font.Flags & TVP_TF_UNDERLINE) ? TRUE:FALSE;
	LogFont.lfStrikeOut = (font.Flags & TVP_TF_STRIKEOUT) ? TRUE:FALSE;
	LogFont.lfEscapement = LogFont.lfOrientation = font.Angle;
	LogFont.lfCharSet = DEFAULT_CHARSET;
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfQuality = DEFAULT_QUALITY;
	LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	tjs_string face = TVPFontSystem->GetBeingFont(font.Face.c_str());
	TJS_strncpy( LogFont.lfFaceName, face.c_str(), LF_FACESIZE -1);
	LogFont.lfFaceName[LF_FACESIZE-1] = 0;
	return ApplyFont( &LogFont );
}
bool tTVPSysFont::ApplyFont( const LOGFONT* info ) {
	HFONT hFont = ::CreateFontIndirect( info );
	if( hFont == nullptr ) return false;
	if( hFont_ != INVALID_HANDLE_VALUE ) {
		HFONT hOld = (HFONT)::SelectObject( hMemDC_, hFont );
		//assert( hOld == hFont_ );
		::DeleteObject( hOld );
		hFont_ = hFont;
	} else {
		hOldFont_ = (HFONT)::SelectObject( hMemDC_, hFont );
		hFont_ = hFont;
	}
	return true;
}
void tTVPSysFont::GetFont( LOGFONT* font ) const {
	::GetObject( hFont_, sizeof(LOGFONT), font );
}
bool tTVPSysFont::AssignDefaultUIFont() {
	HGDIOBJ obj =::GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT logfont = {0};
	::GetObject( (HFONT)obj, sizeof(LOGFONT), &logfont );
	return ApplyFont( &logfont );
}
//---------------------------------------------------------------------------
struct tTVPFSEnumFontsProcData {
	std::vector<ttstr> &List;
	tjs_uint32 Flags;
	BYTE CharSet;
	tTVPFSEnumFontsProcData(std::vector<ttstr> & list, tjs_uint32 flags, BYTE charSet ) :
		List(list), Flags(flags), CharSet(charSet) {
	}
};
//---------------------------------------------------------------------------
/**
 * @param lpelfe : pointer to logical-font data
 * @param lpntme : pointer to physical-font data
 * @param FontType : type of font
 */
static int CALLBACK TVPFSFEnumFontsProc( ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM userdata ) {
	// enumerate fonts
	tTVPFSEnumFontsProcData *data = reinterpret_cast<tTVPFSEnumFontsProcData*>(userdata);
	if( data->Flags & TVP_FSF_FIXEDPITCH ) {
		// fixed pitch only ?
		if(lpntme->ntmTm.tmPitchAndFamily & TMPF_FIXED_PITCH) return 1;
	}

	if( data->Flags & TVP_FSF_SAMECHARSET ) {
		// same character set only ?
		if(lpelfe->elfLogFont.lfCharSet != data->CharSet ) return 1;
	}

	if( data->Flags & TVP_FSF_IGNORESYMBOL ) {
		if(lpelfe->elfLogFont.lfCharSet == SYMBOL_CHARSET ) return 1;
	}

	if( data->Flags & TVP_FSF_NOVERTICAL ) {
		// not to list vertical fonts up ?
		if(lpelfe->elfLogFont.lfFaceName[0] == TJS_W('@') ) return 1;
	}

	if( data->Flags & TVP_FSF_TRUETYPEONLY ) {
		// true type or opentype only ?
		bool is_outline = (lpntme->ntmTm.ntmFlags &  NTM_PS_OPENTYPE) || (lpntme->ntmTm.ntmFlags &  NTM_TT_OPENTYPE) || (FontType & TRUETYPE_FONTTYPE);
		if(!is_outline) return 1;
	}

	ttstr facename( lpelfe->elfLogFont.lfFaceName );
	if(std::find(data->List.begin(), data->List.end(), facename) == data->List.end())
		data->List.push_back(facename); // not insert the same face twice

	return 1;
}
// フォント名からキャラセットを得る
static int CALLBACK TVPFSFEnumCurFaceFontsProc( ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM userdata ) {
	BYTE* data = reinterpret_cast<BYTE*>(userdata);
	*data = lpelfe->elfLogFont.lfCharSet;
	// lpntme->ntmTm.tmDefaultChar;
	return 0;
}
static void SetDefaultLogFont( LOGFONT& l, const tjs_char* face ) {
	l.lfHeight = -12;
	l.lfWidth = 0;
	l.lfEscapement = 0;
	l.lfOrientation = 0;
	l.lfWeight = 400;
	l.lfItalic = FALSE;
	l.lfUnderline = FALSE;
	l.lfStrikeOut = FALSE;
	l.lfCharSet = DEFAULT_CHARSET;
	l.lfOutPrecision = OUT_DEFAULT_PRECIS;
	l.lfQuality = DEFAULT_QUALITY;
	l.lfPitchAndFamily = 0;
	if( face ) {
		TJS_strncpy( l.lfFaceName, face, LF_FACESIZE );
	} else {
		l.lfFaceName[0] = '\0';
	}
}
void TVPGetFontList(std::vector<ttstr> & list, tjs_uint32 flags, const tTVPFont & font ) {
	LOGFONT l;
	SetDefaultLogFont( l, NULL );

	LOGFONT clf;
	memcpy( &clf, &l, sizeof(LOGFONT) );
	TJS_strncpy( clf.lfFaceName, font.Face.c_str(), LF_FACESIZE );

	HDC dc = ::GetDC(NULL);
	BYTE charSet = DEFAULT_CHARSET;
	::EnumFontFamiliesEx( dc, &clf, (FONTENUMPROC)TVPFSFEnumCurFaceFontsProc, reinterpret_cast<LPARAM>(&charSet), 0);
	tTVPFSEnumFontsProcData data( list, flags, charSet );
	::EnumFontFamiliesEx( dc, &l, (FONTENUMPROC)TVPFSFEnumFontsProc, reinterpret_cast<LPARAM>(&data), 0);
 	::ReleaseDC(NULL, dc);
}
tjs_uint8 TVPGetCharSetFromFaceName( const tjs_char* face ) {
	LOGFONT l;
	SetDefaultLogFont( l, face );
	HDC dc = ::GetDC(NULL);
	BYTE charSet = DEFAULT_CHARSET;
	::EnumFontFamiliesEx( dc, &l, (FONTENUMPROC)TVPFSFEnumCurFaceFontsProc, reinterpret_cast<LPARAM>(&charSet), 0);
 	::ReleaseDC(NULL, dc);
	return charSet;
}
#if 0
static int CALLBACK TVPFSFEnumDefaultCharProc( ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM userdata ) {
	tjs_char* data = reinterpret_cast<tjs_char*>(userdata);
	// *data = lpntme->ntmTm.tmDefaultChar;
	*data = lpntme->ntmTm.tmBreakChar;
	return 0;
}
tjs_char TVPGetDefaultCharFromFaceName( const tjs_char* face ) {
	LOGFONT l;
	SetDefaultLogFont( l, face );
	HDC dc = ::GetDC(NULL);
	tjs_char defchar = 0;
	::EnumFontFamiliesEx( dc, &l, (FONTENUMPROC)TVPFSFEnumDefaultCharProc, reinterpret_cast<LPARAM>(&defchar), 0);
 	::ReleaseDC(NULL, dc);
	return defchar;
}
#endif
static int CALLBACK EnumAllFontsProc( LOGFONT *lplf, TEXTMETRIC *lptm, DWORD type, LPARAM data ) {
	std::vector<tjs_string>* list = (std::vector<tjs_string>*)data;
	list->push_back( tjs_string( lplf->lfFaceName) );
	return 1;
}
void TVPGetAllFontList( std::vector<tjs_string>& list ) {
	HDC dc = ::GetDC(NULL);
	::EnumFonts(dc, NULL, (FONTENUMPROC)EnumAllFontsProc,(LPARAM)&list );
 	::ReleaseDC(NULL, dc);
}


