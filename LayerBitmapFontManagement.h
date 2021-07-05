//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Base Layer Bitmap implementation
//---------------------------------------------------------------------------
#ifndef LayerBitmapFontManagementH
#define LayerBitmapFontManagementH

#include "tvpfontstruc.h"
#include "ComplexRect.h"
#include "PrerenderedFont.h"
#include "FontSystem.h"
#include "CharacterData.h"

//---------------------------------------------------------------------------
extern void TVPSetFontCacheForLowMem();
//---------------------------------------------------------------------------

class tTVPCharacterData;
class tTVPPrerenderedFont;

extern tTVPPrerenderedFont * TVPGetPrerenderedMappedFont(const tTVPFont &font);
extern tTVPCharacterData * TVPGetCharacter(const tTVPFontAndCharacterData & font, tTVPPrerenderedFont *pfont, tjs_int aofsx, tjs_int aofsy);

extern FontSystem* TVPFontSystem;
extern tjs_int TVPGlobalFontStateMagic;

extern void TVPSetFontRasterizer( tjs_int index );
extern tjs_int TVPGetFontRasterizer();

extern void TVPMapPrerenderedFont(const tTVPFont & font, const ttstr & storage);
extern void TVPUnmapPrerenderedFont(const tTVPFont & font);

extern void TVPInializeFontRasterizers();

extern TJS::tTJSHashTable<ttstr, tTVPPrerenderedFont *> TVPPrerenderedFonts;

#endif
