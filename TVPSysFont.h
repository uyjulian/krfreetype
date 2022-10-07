#ifndef __TVP_SYS_FONT_H__
#define __TVP_SYS_FONT_H__

class tTVPSysFont {
	HFONT hFont_;
	HFONT hOldFont_;
	HDC hMemDC_;
	HBITMAP hBmp_;
	HBITMAP hOldBmp_;

private:
	void InitializeMemDC();

public:
	tTVPSysFont();
	tTVPSysFont( const tTVPFont &font );
	~tTVPSysFont();

	int GetAscentHeight();
	bool Assign( const tTVPSysFont* font );
	bool Assign( const tTVPFont &font );
	bool ApplyFont( const LOGFONT* info );
	bool AssignDefaultUIFont();
	void GetFont( LOGFONT* font ) const;
	HDC GetDC() { return hMemDC_; }
};

extern void TVPGetAllFontList( std::vector<tjs_string>& list );
extern void TVPGetFontList(std::vector<ttstr> & list, tjs_uint32 flags, const tTVPFont & font );
extern tjs_uint8 TVPGetCharSetFromFaceName( const tjs_char* face );

extern const tjs_char *TVPGetDefaultFontName();
extern void TVPSetDefaultFontName( const tjs_char * name );
extern const tjs_string TVPGetDefaultFaceNames();

#endif // __TVP_SYS_FONT_H__
