//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Base Layer Bitmap implementation
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include <memory>
#include <stdlib.h>
#include <math.h>

#include "ComplexRect.h"
#include "tvpgl.h"
#include "tjsHashSearch.h"
#include "StringUtil.h"
#include "CharacterData.h"
#include "PrerenderedFont.h"
#include "FontSystem.h"
#include "FreeType.h"
#include "FreeTypeFontRasterizer.h"
#ifdef _WIN32
#include "TVPSysFont.h"
#endif

#include "LayerBitmapFontDraw.h"
#include "LayerBitmapFontManagement.h"

struct tTVPDrawTextData
{
	tTVPRect rect;
	tjs_int bmppitch;
	tjs_uint8* bmpdata;
	tjs_int opa;
	bool holdalpha;
	tTVPBBBltMethod bltmode;
};
bool InternalDrawText(tTVPCharacterData *data, tjs_int x,
	tjs_int y, tjs_uint32 color, tTVPDrawTextData *dtdata, tTVPRect &drect)
{
	tjs_uint8 *sl;
	tjs_int h;
	tjs_int w;
	tjs_uint8 *bp;
	tjs_int pitch;

	// setup destination and source rectangle
	drect.left = x + data->OriginX;
	drect.top = y + data->OriginY;
	drect.right = drect.left + data->BlackBoxX;
	drect.bottom = drect.top + data->BlackBoxY;

	tTVPRect srect;
	srect.left = srect.top = 0;
	srect.right = data->BlackBoxX;
	srect.bottom = data->BlackBoxY;

	// check boundary
	if(drect.left < dtdata->rect.left)
	{
		srect.left += (dtdata->rect.left - drect.left);
		drect.left = dtdata->rect.left;
	}

	if(drect.right > dtdata->rect.right)
	{
		srect.right -= (drect.right - dtdata->rect.right);
		drect.right = dtdata->rect.right;
	}

	if(srect.left >= srect.right) return false; // not drawable

	if(drect.top < dtdata->rect.top)
	{
		srect.top += (dtdata->rect.top - drect.top);
		drect.top = dtdata->rect.top;
	}

	if(drect.bottom > dtdata->rect.bottom)
	{
		srect.bottom -= (drect.bottom - dtdata->rect.bottom);
		drect.bottom = dtdata->rect.bottom;
	}

	if(srect.top >= srect.bottom) return false; // not drawable


	// blend to the bitmap
	pitch = data->Pitch;
	sl = dtdata->bmpdata + dtdata->bmppitch * drect.top;
	h = drect.bottom - drect.top;
	w = drect.right - drect.left;
	bp = data->GetData() + pitch * srect.top;
	if( data->Gray == 256 ) {
		if(dtdata->bltmode == bmAlphaOnAlpha)
		{
			if(dtdata->opa > 0)
			{
				if(dtdata->opa == 255)
				{
					while(h--)
						TVPApplyColorMap_d((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color), sl += dtdata->bmppitch,
							bp += pitch;
				}
				else
				{
					while(h--)
						TVPApplyColorMap_do((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				}
			}
			else
			{
				// opacity removal
				if(dtdata->opa == -255)
				{
					while(h--)
						TVPRemoveOpacity((tjs_uint32*)sl + drect.left,
							bp + srect.left, w), sl += dtdata->bmppitch,
							bp += pitch;
				}
				else
				{
					while(h--)
						TVPRemoveOpacity_o((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, -dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				}
			}
		}
		else if(dtdata->bltmode == bmAlphaOnAddAlpha)
		{
			if(dtdata->opa == 255)
			{
				while(h--)
					TVPApplyColorMap_a((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color), sl += dtdata->bmppitch,
						bp += pitch;
			}
			else
			{
				while(h--)
					TVPApplyColorMap_ao((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
						bp += pitch;
			}
		}
		else
		{
			if(dtdata->opa == 255)
			{
				if(dtdata->holdalpha)
					while(h--)
						TVPApplyColorMap_HDA((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color), sl += dtdata->bmppitch,
							bp += pitch;
				else
					while(h--)
						TVPApplyColorMap((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color), sl += dtdata->bmppitch,
							bp += pitch;
			}
			else
			{
				if(dtdata->holdalpha)
					while(h--)
						TVPApplyColorMap_HDA_o((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				else
					while(h--)
						TVPApplyColorMap_o((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
			}
		}
	} else if( data->FullColored ) {
		if(dtdata->bltmode == bmAlphaOnAlpha)
		{
			if(dtdata->opa > 0)
			{
				if(dtdata->opa == 255)
				{
					while(h--)
						TVPAlphaBlend_d((tjs_uint32*)sl + drect.left,
							(tjs_uint32*)bp + srect.left, w), sl += dtdata->bmppitch,
							bp += pitch;
				}
				else
				{
					while(h--)
						TVPAlphaBlend_do((tjs_uint32*)sl + drect.left,
							(tjs_uint32*)bp + srect.left, w, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				}
			}
		}
		else if(dtdata->bltmode == bmAlphaOnAddAlpha)
		{
			if(dtdata->opa == 255)
			{
				while(h--)
					TVPAlphaBlend_a((tjs_uint32*)sl + drect.left,
						(tjs_uint32*)bp + srect.left, w), sl += dtdata->bmppitch,
						bp += pitch;
			}
			else
			{
				while(h--)
					TVPAlphaBlend_ao((tjs_uint32*)sl + drect.left,
						(tjs_uint32*)bp + srect.left, w, dtdata->opa), sl += dtdata->bmppitch,
						bp += pitch;
			}
		}
		else
		{
			if(dtdata->opa == 255)
			{
				if(dtdata->holdalpha)
					while(h--)
						TVPAlphaBlend_HDA((tjs_uint32*)sl + drect.left,
							(tjs_uint32*)bp + srect.left, w), sl += dtdata->bmppitch,
							bp += pitch;
				else
					while(h--)
						TVPAlphaBlend((tjs_uint32*)sl + drect.left,
							(tjs_uint32*)bp + srect.left, w), sl += dtdata->bmppitch,
							bp += pitch;
			}
			else
			{
				if(dtdata->holdalpha)
					while(h--)
						TVPAlphaBlend_HDA_o((tjs_uint32*)sl + drect.left,
							(tjs_uint32*)bp + srect.left, w, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				else
					while(h--)
						TVPAlphaBlend_o((tjs_uint32*)sl + drect.left,
							(tjs_uint32*)bp + srect.left, w, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
			}
		}
	} else {
		if(dtdata->bltmode == bmAlphaOnAlpha)
		{
			if(dtdata->opa > 0)
			{
				if(dtdata->opa == 255)
				{
					while(h--)
						TVPApplyColorMap65_d((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color), sl += dtdata->bmppitch,
							bp += pitch;
				}
				else
				{
					while(h--)
						TVPApplyColorMap65_do((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				}
			}
			else
			{
				// opacity removal
				if(dtdata->opa == -255)
				{
					while(h--)
						TVPRemoveOpacity65((tjs_uint32*)sl + drect.left,
							bp + srect.left, w), sl += dtdata->bmppitch,
							bp += pitch;
				}
				else
				{
					while(h--)
						TVPRemoveOpacity65_o((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, -dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				}
			}
		}
		else if(dtdata->bltmode == bmAlphaOnAddAlpha)
		{
			if(dtdata->opa == 255)
			{
				while(h--)
					TVPApplyColorMap65_a((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color), sl += dtdata->bmppitch,
						bp += pitch;
			}
			else
			{
				while(h--)
					TVPApplyColorMap65_ao((tjs_uint32*)sl + drect.left,
						bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
						bp += pitch;
			}
		}
		else
		{
			if(dtdata->opa == 255)
			{
				if(dtdata->holdalpha)
					while(h--)
						TVPApplyColorMap65_HDA((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color), sl += dtdata->bmppitch,
							bp += pitch;
				else
					while(h--)
						TVPApplyColorMap65((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color), sl += dtdata->bmppitch,
							bp += pitch;
			}
			else
			{
				if(dtdata->holdalpha)
					while(h--)
						TVPApplyColorMap65_HDA_o((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
				else
					while(h--)
						TVPApplyColorMap65_o((tjs_uint32*)sl + drect.left,
							bp + srect.left, w, color, dtdata->opa), sl += dtdata->bmppitch,
							bp += pitch;
			}
		}
	}
	return true;
}
void DrawTextSingle(tjs_int bmppitch, tjs_uint8* bmpdata,
	tTVPFontInfo *fontinfo,
	const tTVPRect &destrect,
	tjs_int x, tjs_int y, const ttstr &text,
		tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa,
			bool holdalpha, bool aa, tjs_int shlevel,
			tjs_uint32 shadowcolor,
			tjs_int shwidth, tjs_int shofsx, tjs_int shofsy,
			tTVPComplexRect *updaterects)
{
	// text drawing function for single character

	if(bltmode == bmAlphaOnAlpha)
	{
		if(opa < -255) opa = -255;
		if(opa > 255) opa = 255;
	}
	else
	{
		if(opa < 0) opa = 0;
		if(opa > 255 ) opa = 255;
	}

	if(opa == 0) return; // nothing to do

	const tjs_char *p = text.c_str();
	tTVPDrawTextData dtdata;
	dtdata.rect = destrect;
	dtdata.bmppitch = bmppitch;
	dtdata.bmpdata = bmpdata;
	dtdata.bltmode = bltmode;
	dtdata.opa = opa;
	dtdata.holdalpha = holdalpha;

	tTVPFontAndCharacterData font;
	font.Font = fontinfo->Font;
	font.Antialiased = aa;
	font.Hinting = true;
	font.BlurLevel = shlevel;
	font.BlurWidth = shwidth;
	font.FontHash = fontinfo->FontHash;

	font.Character = *p;

	font.Blured = false;
	tTVPCharacterData * shadow = NULL;
	tTVPCharacterData * data = NULL;

	try
	{
		data = TVPGetCharacter(font, fontinfo->PrerenderedFont, fontinfo->AscentOfsX, fontinfo->AscentOfsY);

		if(shlevel != 0)
		{
			if(shlevel == 255 && shwidth == 0)
			{
				// normal shadow
				shadow = data;
				shadow->AddRef();
			}
			else
			{
				// blured shadow
				font.Blured = true;
				shadow =
					TVPGetCharacter(font, fontinfo->PrerenderedFont, fontinfo->AscentOfsX, fontinfo->AscentOfsY);
			}
		}


		if(data)
		{

			if(data->BlackBoxX != 0 && data->BlackBoxY != 0)
			{
				tTVPRect drect;
				tTVPRect shadowdrect;

				bool shadowdrawn = false;

				if(shadow)
				{
					shadowdrawn = InternalDrawText(shadow, x + shofsx, y + shofsy,
						shadowcolor, &dtdata, shadowdrect);
				}

				bool drawn = InternalDrawText(data, x, y, color, &dtdata, drect);
				if(updaterects)
				{
					if(!shadowdrawn)
					{
						if(drawn) updaterects->Or(drect);
					}
					else
					{
						if(drawn)
						{
							tTVPRect d;
							TVPUnionRect(&d, drect, shadowdrect);
							updaterects->Or(d);
						}
						else
						{
							updaterects->Or(shadowdrect);
						}
					}
				}
			}
		}
	}
	catch(...)
	{
		if(data) data->Release();
		if(shadow) shadow->Release();
		throw;
	}

	if(data) data->Release();
	if(shadow) shadow->Release();
}
//---------------------------------------------------------------------------
// structure for holding data for a character
struct tTVPCharacterDrawData
{
	tTVPCharacterData * Data; // main character data
	tTVPCharacterData * Shadow; // shadow character data
	tjs_int X, Y;
	tTVPRect ShadowRect;
	bool ShadowDrawn;

	tTVPCharacterDrawData(
		tTVPCharacterData * data,
		tTVPCharacterData * shadow,
		tjs_int x, tjs_int y)
	{
		Data = data;
		Shadow = shadow;
		X = x;
		Y = y;
		ShadowDrawn = false;

		if(Data) Data->AddRef();
		if(Shadow) Shadow->AddRef();
	}

	~tTVPCharacterDrawData()
	{
		if(Data) Data->Release();
		if(Shadow) Shadow->Release();
	}

	tTVPCharacterDrawData(const tTVPCharacterDrawData & rhs)
	{
		Data = Shadow = NULL;
		*this = rhs;
	}

	void operator = (const tTVPCharacterDrawData & rhs)
	{
		X = rhs.X;
		Y = rhs.Y;
		ShadowRect = rhs.ShadowRect;
		ShadowDrawn = rhs.ShadowDrawn;

		if(Data != rhs.Data)
		{
			if(Data) Data->Release();
			Data = rhs.Data;
			if(Data) Data->AddRef();
		}
		if(Shadow != rhs.Shadow)
		{
			if(Shadow) Shadow->Release();
			Shadow = rhs.Shadow;
			if(Shadow) Shadow->AddRef();
		}
	}
};
//---------------------------------------------------------------------------
void DrawTextMultiple(tjs_int bmppitch, tjs_uint8 *bmpdata,
	tTVPFontInfo *fontinfo,
	const tTVPRect &destrect,
	tjs_int x, tjs_int y, const ttstr &text,
		tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa,
			bool holdalpha, bool aa, tjs_int shlevel,
			tjs_uint32 shadowcolor,
			tjs_int shwidth, tjs_int shofsx, tjs_int shofsy,
			tTVPComplexRect *updaterects)
{
	// text drawing function for multiple characters

	if(bltmode == bmAlphaOnAlpha)
	{
		if(opa < -255) opa = -255;
		if(opa > 255) opa = 255;
	}
	else
	{
		if(opa < 0) opa = 0;
		if(opa > 255 ) opa = 255;
	}

	if(opa == 0) return; // nothing to do

	const tjs_char *p = text.c_str();
	tTVPDrawTextData dtdata;
	dtdata.rect = destrect;
	dtdata.bmppitch = bmppitch;
	dtdata.bmpdata = bmpdata;
	dtdata.bltmode = bltmode;
	dtdata.opa = opa;
	dtdata.holdalpha = holdalpha;

	tTVPFontAndCharacterData font;
	font.Font = fontinfo->Font;
	font.Antialiased = aa;
	font.Hinting = true;
	font.BlurLevel = shlevel;
	font.BlurWidth = shwidth;
	font.FontHash = fontinfo->FontHash;


	std::vector<tTVPCharacterDrawData> drawdata;
	drawdata.reserve(text.GetLen());

	// prepare all drawn characters
	while(*p) // while input string is remaining
	{
		font.Character = *p;

		font.Blured = false;
		tTVPCharacterData * data = NULL;
		tTVPCharacterData * shadow = NULL;
		try
		{
			data =
				TVPGetCharacter(font, fontinfo->PrerenderedFont, fontinfo->AscentOfsX, fontinfo->AscentOfsY);

			if(data)
			{
				if(shlevel != 0)
				{
					if(shlevel == 255 && shwidth == 0)
					{
						// normal shadow
						// shadow is the same as main character data
						shadow = data;
						shadow->AddRef();
					}
					else
					{
						// blured shadow
						font.Blured = true;
						shadow =
							TVPGetCharacter(font, fontinfo->PrerenderedFont, fontinfo->AscentOfsX, fontinfo->AscentOfsY);
					}
				}


				if(data->BlackBoxX != 0 && data->BlackBoxY != 0)
				{
					// append to array
					drawdata.push_back(tTVPCharacterDrawData(data, shadow, x, y));
				}

				// step to the next character position
				x += data->Metrics.CellIncX;
				if(data->Metrics.CellIncY != 0)
				{
					// Windows 9x returns negative CellIncY.
					// so we must verify whether CellIncY is proper.
					if(fontinfo->Font.Angle < 1800)
					{
						if(data->Metrics.CellIncY > 0) data->Metrics.CellIncY = - data->Metrics.CellIncY;
					}
					else
					{
						if(data->Metrics.CellIncY < 0) data->Metrics.CellIncY = - data->Metrics.CellIncY;
					}
					y += data->Metrics.CellIncY;
				}
			}
		}
		catch(...)
		{
			 if(data) data->Release();
			 if(shadow) shadow->Release();
			 throw;
		}
		if(data) data->Release();
		if(shadow) shadow->Release();

		p++;
	}

	// draw shadows first
	if(shlevel != 0)
	{
		for(std::vector<tTVPCharacterDrawData>::iterator i = drawdata.begin();
			i != drawdata.end(); i++)
		{
			tTVPCharacterData * shadow = i->Shadow;

			if(shadow)
			{
				i->ShadowDrawn = InternalDrawText(shadow, i->X + shofsx, i->Y + shofsy,
					shadowcolor, &dtdata, i->ShadowRect);
			}
		}
	}

	// then draw main characters
	// and compute returning update rectangle
	for(std::vector<tTVPCharacterDrawData>::iterator i = drawdata.begin();
		i != drawdata.end(); i++)
	{
		tTVPCharacterData * data = i->Data;
		tTVPRect drect;

		bool drawn = InternalDrawText(data, i->X, i->Y, color, &dtdata, drect);
		if(updaterects)
		{
			if(!i->ShadowDrawn)
			{
				if(drawn) updaterects->Or(drect);
			}
			else
			{
				if(drawn)
				{
					tTVPRect d;
					TVPUnionRect(&d, drect, i->ShadowRect);
					updaterects->Or(d);
				}
				else
				{
					updaterects->Or(i->ShadowRect);
				}
			}
		}
	}
}

void DrawText(tjs_int bmppitch, tjs_uint8* bmpdata,
	tTVPFontInfo *fontinfo,
	const tTVPRect &destrect, tjs_int x, tjs_int y, const ttstr &text,
	tjs_uint32 color, tTVPBBBltMethod bltmode, tjs_int opa = 255,
		bool holdalpha = true, bool aa = true, tjs_int shlevel = 0,
		tjs_uint32 shadowcolor = 0,
		tjs_int shwidth = 0, tjs_int shofsx = 0, tjs_int shofsy = 0,
		tTVPComplexRect *updaterects = NULL)
{
	tjs_int len = text.GetLen();
	if(len == 0) return;
	if(len >= 2)
		DrawTextMultiple(
			bmppitch, bmpdata,
			fontinfo,
			destrect, x, y, text,
			color, bltmode, opa,
			holdalpha, aa, shlevel,
			shadowcolor, shwidth, shofsx, shofsy,
			updaterects);
	else    /* if len == 1 */
		DrawTextSingle(
			bmppitch, bmpdata,
			fontinfo,
			destrect, x, y, text,
			color, bltmode, opa,
			holdalpha, aa, shlevel,
			shadowcolor, shwidth, shofsx, shofsy,
			updaterects);
}
