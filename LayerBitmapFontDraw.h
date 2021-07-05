//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Base Layer Bitmap implementation
//---------------------------------------------------------------------------
#ifndef LayerBitmapFontDrawH
#define LayerBitmapFontDrawH

#include "tvpfontstruc.h"
#include "ComplexRect.h"

class tTVPFontInfo
{
public:
	tTVPFont Font;
	tTVPPrerenderedFont *PrerenderedFont;
	tjs_int AscentOfsX;
	tjs_int AscentOfsY;
	tjs_uint32 FontHash;
};

extern void DrawText(tjs_int bmppitch, tjs_uint8* bmpdata,
	tTVPFontInfo *fontinfo,
	const tTVPRect &destrect, tjs_int x, tjs_int y, const ttstr &text,
	tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa = 255,
		bool holdalpha = true, bool aa = true, tjs_int shlevel = 0,
		tjs_uint32 shadowcolor = 0,
		tjs_int shwidth = 0, tjs_int shofsx = 0, tjs_int shofsy = 0,
		tTVPComplexRect *updaterects = NULL);

#endif
