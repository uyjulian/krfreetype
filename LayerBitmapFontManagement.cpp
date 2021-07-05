

#define _USE_MATH_DEFINES
#include "tjsCommHead.h"

#include <memory>
#include <stdlib.h>
#include <math.h>

#include "LayerBitmapFontManagement.h"
#include "ComplexRect.h"
#include "tvpgl.h"
#include "tjsHashSearch.h"

#include "StringUtil.h"
#include "CharacterData.h"
#include "PrerenderedFont.h"
#include "FontSystem.h"
#include "FreeType.h"
#include "FreeTypeFontRasterizer.h"

//---------------------------------------------------------------------------
// prototypes
//---------------------------------------------------------------------------
void TVPClearFontCache();
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// default FONT retrieve function
//---------------------------------------------------------------------------
FontSystem* TVPFontSystem = NULL;
tjs_int TVPGlobalFontStateMagic = 0;
	// this is for checking global font status' change


enum {
	FONT_RASTER_FREE_TYPE,
	FONT_RASTER_EOT
};
static FontRasterizer* TVPFontRasterizers[FONT_RASTER_EOT];
static bool TVPFontRasterizersInit = false;
static tjs_int TVPCurrentFontRasterizers = FONT_RASTER_FREE_TYPE;

void TVPInializeFontRasterizers() {
	if( TVPFontRasterizersInit == false ) {
		TVPFontRasterizers[FONT_RASTER_FREE_TYPE] = new FreeTypeFontRasterizer();

		TVPFontSystem = new FontSystem();
		TVPFontRasterizersInit = true;
	}
}
#if 0
void TVPUninitializeFontRasterizers() {
	for( tjs_int i = 0; i < FONT_RASTER_EOT; i++ ) {
		if( TVPFontRasterizers[i] ) {
			TVPFontRasterizers[i]->Release();
			TVPFontRasterizers[i] = NULL;
		}
	}
	if( TVPFontSystem ) {
		delete TVPFontSystem;
		TVPFontSystem = NULL;
	}
}
static tTVPAtExit
	TVPUninitializeFontRaster(TVP_ATEXIT_PRI_RELEASE, TVPUninitializeFontRasterizers);
#endif

void TVPSetFontRasterizer( tjs_int index ) {
	if( TVPCurrentFontRasterizers != index && index >= 0 && index < FONT_RASTER_EOT ) {
		TVPCurrentFontRasterizers = index;
		TVPClearFontCache(); // ラスタライザが切り替わる時、キャッシュはクリアしてしまう
		TVPGlobalFontStateMagic++; // ApplyFont が走るようにする
	}
}
tjs_int TVPGetFontRasterizer() {
	return TVPCurrentFontRasterizers;
}
FontRasterizer* GetCurrentRasterizer() {
	return TVPFontRasterizers[TVPCurrentFontRasterizers];
}

//---------------------------------------------------------------------------
#define TVP_CH_MAX_CACHE_COUNT 1300
#define TVP_CH_MAX_CACHE_COUNT_LOW 100
#define TVP_CH_MAX_CACHE_HASH_SIZE 512
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// Pre-rendered font management
//---------------------------------------------------------------------------
TJS::tTJSHashTable<ttstr, tTVPPrerenderedFont *> TVPPrerenderedFonts;



//---------------------------------------------------------------------------
// tTVPPrerenderedFontMap
//---------------------------------------------------------------------------
struct tTVPPrerenderedFontMap
{
	tTVPFont Font; // mapped font
	tTVPPrerenderedFont * Object; // prerendered font object
};
static std::vector<tTVPPrerenderedFontMap> TVPPrerenderedFontMapVector;
//---------------------------------------------------------------------------
void TVPMapPrerenderedFont(const tTVPFont & font, const ttstr & storage)
{
	// map specified font to specified prerendered font
	ttstr fn = TVPGetPlacedPath(storage);
	if(fn.IsEmpty()) TVPThrowExceptionMessage(TJS_W("Cannot find storage %1"), fn);

	// search or retrieve specified storage
	tTVPPrerenderedFont * object;

	tTVPPrerenderedFont ** found = TVPPrerenderedFonts.Find(fn);
	if(!found)
	{
		// not yet exist; create
		object = new tTVPPrerenderedFont(fn);
	}
	else
	{
		// already exist
		object = *found;
		object->AddRef();
	}

	// search existing mapped font
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		if(i->Font == font)
		{
			// found font
			// replace existing
			i->Object->Release();
			i->Object = object;
			break;
		}
	}
	if(i == TVPPrerenderedFontMapVector.end())
	{
		// not found
		tTVPPrerenderedFontMap map;
		map.Font = font;
		map.Object = object;
		TVPPrerenderedFontMapVector.push_back(map); // add
	}

	TVPGlobalFontStateMagic ++; // increase magic number

	TVPClearFontCache(); // clear font cache
}
//---------------------------------------------------------------------------
void TVPUnmapPrerenderedFont(const tTVPFont & font)
{
	// unmap specified font
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		if(i->Font == font)
		{
			// found font
			// replace existing
			i->Object->Release();
			TVPPrerenderedFontMapVector.erase(i);
			TVPGlobalFontStateMagic ++; // increase magic number
			TVPClearFontCache();
			return;
		}
	}
}
//---------------------------------------------------------------------------
#if 0
static void TVPUnmapAllPrerenderedFonts()
{
	// unmap all prerendered fonts
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		i->Object->Release();
	}
	TVPPrerenderedFontMapVector.clear();
	TVPGlobalFontStateMagic ++; // increase magic number
}
//---------------------------------------------------------------------------
static tTVPAtExit TVPUnmapAllPrerenderedFontsAtExit
	(TVP_ATEXIT_PRI_PREPARE, TVPUnmapAllPrerenderedFonts);
#endif
//---------------------------------------------------------------------------
tTVPPrerenderedFont * TVPGetPrerenderedMappedFont(const tTVPFont &font)
{
	// search mapped prerendered font
	std::vector<tTVPPrerenderedFontMap>::iterator i;
	for(i = TVPPrerenderedFontMapVector.begin();
		i !=TVPPrerenderedFontMapVector.end(); i++)
	{
		if(i->Font == font)
		{
			// found font
			// replace existing
			i->Object->AddRef();

			// note that the object is AddRefed
			return i->Object;
		}
	}
	return NULL;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
typedef tTJSRefHolder<tTVPCharacterData> tTVPCharacterDataHolder;

typedef
TJS::tTJSHashCache<tTVPFontAndCharacterData, tTVPCharacterDataHolder,
	tTVPFontHashFunc, TVP_CH_MAX_CACHE_HASH_SIZE> tTVPFontCache;
tTVPFontCache TVPFontCache(TVP_CH_MAX_CACHE_COUNT);
//---------------------------------------------------------------------------
void TVPSetFontCacheForLowMem()
{
	// set character cache limit
	TVPFontCache.SetMaxCount(TVP_CH_MAX_CACHE_COUNT_LOW);
}
//---------------------------------------------------------------------------
void TVPClearFontCache()
{
	TVPFontCache.Clear();
}
//---------------------------------------------------------------------------
struct tTVPClearFontCacheCallback : public tTVPCompactEventCallbackIntf
{
	virtual void TJS_INTF_METHOD OnCompact(tjs_int level)
	{
		if(level >= TVP_COMPACT_LEVEL_MINIMIZE)
		{
			// clear the font cache on application minimize
			TVPClearFontCache();
		}
	}
} static TVPClearFontCacheCallback;
static bool TVPClearFontCacheCallbackInit = false;
//---------------------------------------------------------------------------
tTVPCharacterData * TVPGetCharacter(const tTVPFontAndCharacterData & font, tTVPPrerenderedFont *pfont, tjs_int aofsx, tjs_int aofsy)
{
	// returns specified character data.
	// draw a character if needed.

	// compact interface initialization
	if(!TVPClearFontCacheCallbackInit)
	{
		TVPAddCompactEventHook(&TVPClearFontCacheCallback);
		TVPClearFontCacheCallbackInit = true;
	}

	// make hash and search over cache
	tjs_uint32 hash = tTVPFontCache::MakeHash(font);

	tTVPCharacterDataHolder * ptr = TVPFontCache.FindAndTouchWithHash(font, hash);
	if(ptr)
	{
		// found in the cache
		return ptr->GetObject();
	}

	// not found in the cache

	// look prerendered font
	const tTVPPrerenderedCharacterItem *pitem = NULL;
	if(pfont)
		pitem = pfont->Find(font.Character);

	if(pitem)
	{
		// prerendered font
		tTVPCharacterData *data = new tTVPCharacterData();
		data->BlackBoxX = pitem->Width;
		data->BlackBoxY = pitem->Height;
		data->Metrics.CellIncX = pitem->IncX;
		data->Metrics.CellIncY = pitem->IncY;
		data->OriginX = pitem->OriginX + aofsx;
		data->OriginY = -pitem->OriginY + aofsy;

		data->Antialiased = font.Antialiased;

		data->FullColored = false;

		data->Blured = font.Blured;
		data->BlurWidth = font.BlurWidth;
		data->BlurLevel = font.BlurLevel;

		try
		{
			if(data->BlackBoxX && data->BlackBoxY)
			{
				// render
				tjs_int newpitch =  data->CalcAlignSize( pitem->Width );
				data->Pitch = newpitch;

				data->Alloc(newpitch * data->BlackBoxY);

				pfont->Retrieve(pitem, data->GetData(), newpitch);

				// apply blur
				if(font.Blured) data->Blur(); // nasty ...

				// add to hash table
				tTVPCharacterDataHolder holder(data);
				TVPFontCache.AddWithHash(font, hash, holder);
			}
		}
		catch(...)
		{
			data->Release();
			throw;
		}

		return data;
	}
	else
	{
		// render font
		tTVPCharacterData *data = GetCurrentRasterizer()->GetBitmap( font, aofsx, aofsy );

		// add to hash table
		tTVPCharacterDataHolder holder(data);
		TVPFontCache.AddWithHash(font, hash, holder);
		return data;
	}
}
//---------------------------------------------------------------------------
