/////////////////////////////////////////////
//                                         //
//    Copyright (C) 2021-2021 Julian Uy    //
//  https://sites.google.com/site/awertyb  //
//                                         //
//   See details of license at "LICENSE"   //
//                                         //
/////////////////////////////////////////////

#include "ncbind/ncbind.hpp"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "LayerBitmapFontManagement.h"
#include "FontSystem.h"
#include "PrerenderedFont.h"
#include "FontRasterizer.h"
#include "LayerBitmapFontDraw.h"
#include "RectItf.h"

class Font {
private:
	bool FontChanged;
	tjs_int GlobalFontState;

	// v--- these can be recreated in ApplyFont if FontChanged flag is set
	tTVPFontInfo FontInfo;
	double RadianAngle;

	tjs_int TextWidth;
	tjs_int TextHeight;
	ttstr CachedText;
	
public:
	Font() {
		this->FontInfo.Font = TVPFontSystem->GetDefaultFont();
		this->FontInfo.PrerenderedFont = NULL;
		this->FontChanged = true;
	}
	~Font() {}

	void ApplyFont(void)
	{
		if(FontChanged || GlobalFontState != TVPGlobalFontStateMagic)
		{
			FontChanged = false;
			GlobalFontState = TVPGlobalFontStateMagic;
			CachedText.Clear();
			TextWidth = TextHeight = 0;

			if(this->FontInfo.PrerenderedFont) this->FontInfo.PrerenderedFont->Release();
			this->FontInfo.PrerenderedFont = TVPGetPrerenderedMappedFont(this->FontInfo.Font);

			// compute ascent offset
			GetCurrentRasterizer()->ApplyFont(this->FontInfo.Font);
			tjs_int ascent = GetCurrentRasterizer()->GetAscentHeight();
			RadianAngle = this->FontInfo.Font.Angle * (M_PI/1800);
			tjs_real angle90 = RadianAngle + M_PI_2;
			this->FontInfo.AscentOfsX = static_cast<tjs_int>(-std::cos(angle90) * ascent);
			this->FontInfo.AscentOfsY = static_cast<tjs_int>(std::sin(angle90) * ascent);

			// compute font hash
			this->FontInfo.FontHash = TJS::tTJSHashFunc<ttstr>::Make(this->FontInfo.Font.Face);
			this->FontInfo.FontHash ^= this->FontInfo.Font.Height ^ this->FontInfo.Font.Flags ^ this->FontInfo.Font.Angle;
		}
		else
		{
			GetCurrentRasterizer()->ApplyFont(this->FontInfo.Font);
		}
	}

	void GetTextSize(const ttstr & text)
	{
		ApplyFont();

		if(text != CachedText)
		{
			CachedText = text;

			if(this->FontInfo.PrerenderedFont)
			{
				tjs_uint width = 0;
				const tjs_char *buf = text.c_str();
				while(*buf)
				{
					const tTVPPrerenderedCharacterItem * item =
						this->FontInfo.PrerenderedFont->Find(*buf);
					if(item != NULL)
					{
						width += item->Inc;
					}
					else
					{
						tjs_int w, h;
						GetCurrentRasterizer()->GetTextExtent( *buf, w, h );
						width += w;
					}
					buf++;
				}
				TextWidth = width;
				TextHeight = std::abs(this->FontInfo.Font.Height);
			}
			else
			{
				tjs_uint width = 0;
				const tjs_char *buf = text.c_str();

				while(*buf)
				{
					tjs_int w, h;
					GetCurrentRasterizer()->GetTextExtent( *buf, w, h );
					width += w;
					buf++;
				}
				TextWidth = width;
				TextHeight = std::abs(this->FontInfo.Font.Height);
			}
		}
	}

	tjs_int getTextWidth(ttstr text) {
		GetTextSize(text);
		return TextWidth;
	}

	tjs_int getTextHeight(ttstr text) {
		GetTextSize(text);
		return TextHeight;
	}

	tjs_real getEscWidthX(ttstr text) {
		GetTextSize(text);
		return std::cos(RadianAngle) * TextWidth;
	}

	tjs_real getEscWidthY(ttstr text) {
		GetTextSize(text);
		return std::sin(RadianAngle) * (-TextWidth);
	}

	tjs_real getEscHeightX(ttstr text) {
		GetTextSize(text);
		return std::sin(RadianAngle) * TextHeight;
	}

	tjs_real getEscHeightY(ttstr text) {
		GetTextSize(text);
		return std::cos(RadianAngle) * TextHeight;
	}

	tTJSVariant getGlyphDrawRect(ttstr text) {
		tTVPRect rt;
		ApplyFont();
		GetCurrentRasterizer()->GetGlyphDrawRect( text, rt );
		iTJSDispatch2 *disp = TVPCreateRectObject( rt.left, rt.top, rt.right, rt.bottom );
		tTJSVariant result = tTJSVariant(disp, disp);
		disp->Release();
		return result;
	}

	tTJSVariant getList(tjs_int64 in_flags) {
		ApplyFont();
		tjs_uint32 flags = static_cast<tjs_uint32>(in_flags);

		std::vector<ttstr> list;
		TVPFontSystem->GetFontList(list, flags, this->FontInfo.Font);

		iTJSDispatch2 *dsp;
		dsp = TJSCreateArrayObject();
		tTJSVariant result(dsp, dsp);
		dsp->Release();

		for (tjs_uint i = 0; i < list.size(); i += 1)
		{
			tTJSVariant tmp = list[i];
			dsp->PropSetByNum(TJS_MEMBERENSURE, i, &tmp, dsp);
		}
		return result;
	}

	void mapPrerenderedFont(ttstr storage) {
		ApplyFont();
		TVPMapPrerenderedFont(this->FontInfo.Font, storage);
		this->FontChanged = true;
	}

	void unmapPrerenderedFont() {
		ApplyFont();
		TVPUnmapPrerenderedFont(this->FontInfo.Font);
		this->FontChanged = true;
	}

	static tTJSVariant addFont(ttstr fontname) {
		std::vector<ttstr> faces;
		TVPFontSystem->AddExtraFont(fontname.c_str(), &faces);

		iTJSDispatch2 *dsp;
		dsp = TJSCreateArrayObject();
		tTJSVariant result(dsp, dsp);
		dsp->Release();
		for (tjs_uint i = 0; i < faces.size(); i += 1)
		{
			tTJSVariant tmp = ttstr(faces[i]);
			dsp->PropSetByNum(TJS_MEMBERENSURE, i, &tmp, dsp);
		}
		TVPGlobalFontStateMagic += 1;
		return result;
	}

	ttstr getFace() {
		return this->FontInfo.Font.Face;
	}
	void setFace(ttstr face) {
		if(this->FontInfo.Font.Face != face)
		{
			this->FontInfo.Font.Face = face;
			this->FontChanged = true;
		}
	}

	tjs_int getHeight() {
		return this->FontInfo.Font.Height;
	}
	void setHeight(tjs_int height) {
		if(height < 0) height = -height; // TVP2 does not support negative value of height
		if(this->FontInfo.Font.Height != height)
		{
			this->FontInfo.Font.Height = height;
			this->FontChanged = true;
		}
	}

	bool getBold() {
		return 0!=(this->FontInfo.Font.Flags & TVP_TF_BOLD);
	}
	void setBold(bool bold) {
		if( (0!=(this->FontInfo.Font.Flags & TVP_TF_BOLD)) != bold)
		{
			this->FontInfo.Font.Flags &= ~TVP_TF_BOLD;
			if(bold) this->FontInfo.Font.Flags |= TVP_TF_BOLD;
			this->FontChanged = true;
		}
	}

	bool getItalic() {
		return 0!=(this->FontInfo.Font.Flags & TVP_TF_ITALIC);
	}
	void setItalic(bool italic) {
		if( (0!=(this->FontInfo.Font.Flags & TVP_TF_ITALIC)) != italic)
		{
			this->FontInfo.Font.Flags &= ~TVP_TF_ITALIC;
			if(italic) this->FontInfo.Font.Flags |= TVP_TF_ITALIC;
			this->FontChanged = true;
		}
	}

	bool getStrikeout() {
		return 0!=(this->FontInfo.Font.Flags & TVP_TF_STRIKEOUT);
	}
	void setStrikeout(bool strikeout) {
		if( (0!=(this->FontInfo.Font.Flags & TVP_TF_STRIKEOUT)) != strikeout)
		{
			this->FontInfo.Font.Flags &= ~TVP_TF_STRIKEOUT;
			if(strikeout) this->FontInfo.Font.Flags |= TVP_TF_STRIKEOUT;
			this->FontChanged = true;
		}
	}

	bool getUnderline() {
		return 0!=(this->FontInfo.Font.Flags & TVP_TF_UNDERLINE);
	}
	void setUnderline(bool underline) {
		if( (0!=(this->FontInfo.Font.Flags & TVP_TF_UNDERLINE)) != underline)
		{
			this->FontInfo.Font.Flags &= ~TVP_TF_UNDERLINE;
			if(underline) this->FontInfo.Font.Flags |= TVP_TF_UNDERLINE;
			this->FontChanged = true;
		}
	}

	tjs_int getAngle() {
		return this->FontInfo.Font.Angle;
	}
	void setAngle(tjs_int angle) {
		if (this->FontInfo.Font.Angle != angle)
		{
			angle = angle % 3600;
			if(angle < 0) angle += 3600;
			this->FontInfo.Font.Angle = angle;
			this->FontChanged = true;
		}
	}

	bool getFaceIsFileName() {
		return 0 != (this->FontInfo.Font.Flags & TVP_TF_FONTFILE);
	}
	void setFaceIsFileName(bool faceIsFileName) {
		if ((0!=(this->FontInfo.Font.Flags & TVP_TF_FONTFILE)) != faceIsFileName)
		{
			this->FontInfo.Font.Flags &= ~TVP_TF_FONTFILE;
			if(faceIsFileName) this->FontInfo.Font.Flags |= TVP_TF_FONTFILE;
			this->FontChanged = true;
		}
	}

	static tjs_int getRasterizer() {
		return TVPGetFontRasterizer();
	}
	static void setRasterizer(tjs_int rasterizer) {
		TVPSetFontRasterizer(rasterizer);
	}

	static ttstr getDefaultFaceName() {
		return ttstr(TVPFontSystem->GetDefaultFontName());
	}
	static void setDefaultFaceName(ttstr name) {
		TVPFontSystem->SetDefaultFontName( name.c_str() );
	}

	tTJSVariant drawText(tTJSVariant bmpobject, tTJSVariant destrect, tjs_int x, tjs_int y, ttstr text, tjs_uint32 color, tjs_int bltmode, tjs_int opa, tjs_int holdalpha, tjs_int aa, tjs_int shlevel, tjs_uint32 shadowcolor, tjs_int shwidth, tjs_int shofsx, tjs_int shofsy) {
		tTVPRect ClipRect;
		tTJSVariantClosure ClipRect_clo = destrect.AsObjectClosureNoAddRef();
		if (ClipRect_clo.Object)
		{
			tTJSVariant val;
			static ttstr left_name(TJS_W("left"));
			ClipRect_clo.PropGet(0, left_name.c_str(), left_name.GetHint(), &val, ClipRect_clo.Object);
			ClipRect.left = (tjs_int)val;
			static ttstr top_name(TJS_W("top"));
			ClipRect_clo.PropGet(0, top_name.c_str(), top_name.GetHint(), &val, ClipRect_clo.Object);
			ClipRect.top = (tjs_int)val;
			static ttstr right_name(TJS_W("right"));
			ClipRect_clo.PropGet(0, right_name.c_str(), right_name.GetHint(), &val, ClipRect_clo.Object);
			ClipRect.right = (tjs_int)val;
			static ttstr bottom_name(TJS_W("bottom"));
			ClipRect_clo.PropGet(0, bottom_name.c_str(), bottom_name.GetHint(), &val, ClipRect_clo.Object);
			ClipRect.bottom = (tjs_int)val;
		}

		tjs_int bmppitch = 0;
		tjs_uint8* bmpdata = NULL;
		tTJSVariantClosure bmpobject_clo = bmpobject.AsObjectClosureNoAddRef();
		if (bmpobject_clo.Object)
		{
			tTJSVariant val;
			static ttstr mainImageBufferPitch_name(TJS_W("mainImageBufferPitch"));
			if (TJS_FAILED(bmpobject_clo.PropGet(0, mainImageBufferPitch_name.c_str(), mainImageBufferPitch_name.GetHint(), &val, bmpobject_clo.Object)))
			{
				static ttstr bufferPitch_name(TJS_W("bufferPitch"));
				bmpobject_clo.PropGet(0, bufferPitch_name.c_str(), bufferPitch_name.GetHint(), &val, bmpobject_clo.Object);
			}
			bmppitch = (tjs_int)val;
			static ttstr mainImageBufferForWrite_name(TJS_W("mainImageBufferForWrite"));
			if (TJS_FAILED(bmpobject_clo.PropGet(0, mainImageBufferForWrite_name.c_str(), mainImageBufferForWrite_name.GetHint(), &val, bmpobject_clo.Object)))
			{
				static ttstr bufferForWrite_name(TJS_W("bufferForWrite"));
				bmpobject_clo.PropGet(0, bufferForWrite_name.c_str(), bufferForWrite_name.GetHint(), &val, bmpobject_clo.Object);
			}
			bmpdata = reinterpret_cast<tjs_uint8*>((tjs_intptr_t)(tjs_int64)val);
		}

		tTVPComplexRect r;

		if (bmpdata)
		{
			ApplyFont();
			DrawText(bmppitch, bmpdata, &this->FontInfo, ClipRect, x, y, text, color, (tTVPBBBltMethod)bltmode, opa, holdalpha != 0, aa != 0, shlevel, shadowcolor, shwidth, shofsx, shofsy, &r);
		}

		if (r.GetCount())
		{
			const tTVPRect& ur = r.GetBound();
			iTJSDispatch2 *ret = TVPCreateRectObject( ur.left, ur.top, ur.right, ur.bottom );
			tTJSVariant rret(ret, ret);
			ret->Release();
			return rret;
		}
		return tTJSVariant();
	}
};

NCB_REGISTER_CLASS(Font)
{
	Constructor();

	NCB_METHOD(getTextWidth);
	NCB_METHOD(getTextHeight);
	NCB_METHOD(getEscWidthX);
	NCB_METHOD(getEscWidthY);
	NCB_METHOD(getEscHeightX);
	NCB_METHOD(getEscHeightY);
	NCB_METHOD(getGlyphDrawRect);
	NCB_METHOD(getList);
	NCB_METHOD(mapPrerenderedFont);
	NCB_METHOD(unmapPrerenderedFont);
	NCB_METHOD(addFont);
	NCB_METHOD(drawText);

	NCB_PROPERTY(face, getFace, setFace);
	NCB_PROPERTY(height, getHeight, setHeight);
	NCB_PROPERTY(bold, getBold, setBold);
	NCB_PROPERTY(italic, getItalic, setItalic);
	NCB_PROPERTY(strikeout, getStrikeout, setStrikeout);
	NCB_PROPERTY(underline, getUnderline, setUnderline);
	NCB_PROPERTY(angle, getAngle, setAngle);
	NCB_PROPERTY(faceIsFileName, getFaceIsFileName, setFaceIsFileName);
	NCB_PROPERTY(rasterizer, getRasterizer, setRasterizer);
	NCB_PROPERTY(defaultFaceName, getDefaultFaceName, setDefaultFaceName);
};

static iTJSDispatch2 *font_class = NULL;

static void PreRegistCallback()
{
	TVPInializeFontRasterizers();
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	if (global)
	{
		global->DeleteMember(0, TJS_W("Font"), NULL, global);
		tTJSVariant layer_val;
		global->PropGet(0, TJS_W("Layer"), 0, &layer_val, global);
		tTJSVariantClosure layer_valclosure = layer_val.AsObjectClosureNoAddRef();
		if (layer_valclosure.Object)
		{
			layer_valclosure.DeleteMember(TJS_IGNOREPROP, TJS_W("font"), 0, NULL);
			layer_valclosure.DeleteMember(TJS_IGNOREPROP, TJS_W("drawText"), 0, NULL);
		}
	}
}


static void PostRegistCallback()
{
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	if (global)
	{
		tTJSVariant val;
		tjs_error hr = global->PropGet(0, TJS_W("Font"), 0, &val, global);
		global->Release();
		font_class = val.AsObject();
	}
}

static void PreUnregistCallback()
{
	if (font_class)
	{
		font_class->Release();
		font_class = NULL;
	}
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK(PreUnregistCallback);

class LayerKrFreetype
{
public:
	LayerKrFreetype(iTJSDispatch2 *__objthis)
	{
		if (font_class)
		{
			font_class->CreateNew(0, NULL, NULL, &this->font_object, 0, NULL, font_class);
		}
		if (__objthis)
		{
			this->_objthis = __objthis;
		}
	}
	~LayerKrFreetype()
	{
		if (this->font_object)
		{
			this->font_object->Release();
			this->font_object = NULL;
		}
	}

	tTJSVariant GetFont()
	{
		return tTJSVariant(this->font_object, this->font_object);
	}

	static tjs_error TJS_INTF_METHOD drawText(
		tTJSVariant	*result,
		tjs_int numparams,
		tTJSVariant **param,
		LayerKrFreetype *objthis)
	{
		if (numparams < 4)
		{
			return TJS_E_BADPARAMCOUNT;
		}
		tjs_int x = *param[0];
		tjs_int y = *param[1];
		ttstr text = *param[2];
		tjs_uint32 color = static_cast<tjs_uint32>((tjs_int64)*param[3]);
		tjs_int opa = (numparams >= 5 && param[4]->Type() != tvtVoid)?(tjs_int)*param[4] : (tjs_int)255;
		bool aa = (numparams >= 6 && param[5]->Type() != tvtVoid)? param[5]->operator bool() : true;
		tjs_int shadowlevel = (numparams >= 7 && param[6]->Type() != tvtVoid)? (tjs_int)*param[6] : 0;
		tjs_uint32 shadowcolor = (numparams >= 8 && param[7]->Type() != tvtVoid)? static_cast<tjs_uint32>((tjs_int64)*param[7]) : 0;
		tjs_int shadowwidth = (numparams >= 9 && param[8]->Type() != tvtVoid)? (tjs_int)*param[8] : 0;
		tjs_int shadowofsx = (numparams >=10 && param[9]->Type() != tvtVoid)? (tjs_int)*param[9] : 0;
		tjs_int shadowofsy = (numparams >=11 && param[10]->Type() != tvtVoid)? (tjs_int)*param[10] : 0;

		tTVPDrawFace DrawFace = dfOpaque; // (actual) current drawing layer face
		tTVPDrawFace Face = dfAuto; // (outward) current drawing layer face
		tTVPLayerType DisplayType = ltOpaque; // actual Type

		tTJSVariant objthis_val;
		if (objthis->_objthis)
		{
			objthis_val = tTJSVariant(objthis->_objthis, objthis->_objthis);
		}
		tTJSVariantClosure objthis_valclosure = objthis_val.AsObjectClosureNoAddRef();
		
		if (objthis_valclosure.Object)
		{
			tTJSVariant val;
			static ttstr face_name(TJS_W("face"));
			objthis_valclosure.PropGet(0, face_name.c_str(), face_name.GetHint(), &val, NULL);
			Face = (tTVPDrawFace)(tjs_int)val;
			static ttstr type_name(TJS_W("type"));
			objthis_valclosure.PropGet(0, type_name.c_str(), type_name.GetHint(), &val, NULL);
			DisplayType = (tTVPLayerType)(tjs_int)val;
		}

		// set DrawFace from Face and Type
		// Face = Layer.face
		if(Face == dfAuto)
		{
			// DrawFace is chosen automatically from the layer type
			// DisplayType = Layer.type
			switch(DisplayType)
			{
		//	case ltBinder: ltBinder
			case ltOpaque:				DrawFace = dfOpaque;			break;
			case ltAlpha:				DrawFace = dfAlpha;				break;
			case ltAdditive:			DrawFace = dfOpaque;			break;
			case ltSubtractive:			DrawFace = dfOpaque;			break;
			case ltMultiplicative:		DrawFace = dfOpaque;			break;
		//	case ltEffect: ltBinder
		//	case ltFilter: ltBinder
			case ltDodge:				DrawFace = dfOpaque;			break;
			case ltDarken:				DrawFace = dfOpaque;			break;
			case ltLighten:				DrawFace = dfOpaque;			break;
			case ltScreen:				DrawFace = dfOpaque;			break;
			case ltAddAlpha:			DrawFace = dfAddAlpha;			break;
			case ltPsNormal:			DrawFace = dfAlpha;				break;
			case ltPsAdditive:			DrawFace = dfAlpha;				break;
			case ltPsSubtractive:		DrawFace = dfAlpha;				break;
			case ltPsMultiplicative:	DrawFace = dfAlpha;				break;
			case ltPsScreen:			DrawFace = dfAlpha;				break;
			case ltPsOverlay:			DrawFace = dfAlpha;				break;
			case ltPsHardLight:			DrawFace = dfAlpha;				break;
			case ltPsSoftLight:			DrawFace = dfAlpha;				break;
			case ltPsColorDodge:		DrawFace = dfAlpha;				break;
			case ltPsColorDodge5:		DrawFace = dfAlpha;				break;
			case ltPsColorBurn:			DrawFace = dfAlpha;				break;
			case ltPsLighten:			DrawFace = dfAlpha;				break;
			case ltPsDarken:			DrawFace = dfAlpha;				break;
			case ltPsDifference:	 	DrawFace = dfAlpha;				break;
			case ltPsDifference5:	 	DrawFace = dfAlpha;				break;
			case ltPsExclusion:			DrawFace = dfAlpha;				break;
			default:
								DrawFace = dfOpaque;			break;
			}
		}
		else
		{
			DrawFace = Face;
		}

		tTVPBBBltMethod met = bmAlpha;

		switch(DrawFace)
		{
		case dfAlpha:
			met = bmAlphaOnAlpha;
			break;
		case dfAddAlpha:
			if(opa<0) TVPThrowExceptionMessage(TJS_W("Negative opacity not supported on this face"));
			met = bmAlphaOnAddAlpha;
			break;
		case dfOpaque:
			met = bmAlpha;
			break;
		default:
			TVPThrowExceptionMessage(TJS_W("Not drawble face type %1"), TJS_W("drawText"));
		}

		color = TVPToActualColor(color);

		tTVPRect ClipRect;
		if (objthis_valclosure.Object)
		{
			tTJSVariant val;
			static ttstr clipLeft_name(TJS_W("clipLeft"));
			objthis_valclosure.PropGet(0, clipLeft_name.c_str(), clipLeft_name.GetHint(), &val, NULL);
			ClipRect.left = (tjs_int)val;
			static ttstr clipTop_name(TJS_W("clipTop"));
			objthis_valclosure.PropGet(0, clipTop_name.c_str(), clipTop_name.GetHint(), &val, NULL);
			ClipRect.top = (tjs_int)val;
			static ttstr clipWidth_name(TJS_W("clipWidth"));
			objthis_valclosure.PropGet(0, clipWidth_name.c_str(), clipWidth_name.GetHint(), &val, NULL);
			ClipRect.right = ClipRect.left + ((tjs_int)val);
			static ttstr clipHeight_name(TJS_W("clipHeight"));
			objthis_valclosure.PropGet(0, clipHeight_name.c_str(), clipHeight_name.GetHint(), &val, NULL);
			ClipRect.bottom = ClipRect.top + ((tjs_int)val);
		}

		bool HoldAlpha = false;
		if (objthis_valclosure.Object)
		{
			tTJSVariant val;
			static ttstr holdAlpha_name(TJS_W("holdAlpha"));
			objthis_valclosure.PropGet(0, holdAlpha_name.c_str(), holdAlpha_name.GetHint(), &val, NULL);
			HoldAlpha = ((tjs_int)val) != 0;
		}

		tjs_int ImageLeft = 0;
		tjs_int ImageTop = 0;
		if (objthis_valclosure.Object)
		{
			tTJSVariant val;
			static ttstr imageLeft_name(TJS_W("imageLeft"));
			objthis_valclosure.PropGet(0, imageLeft_name.c_str(), imageLeft_name.GetHint(), &val, NULL);
			ImageLeft = (tjs_int)val;
			static ttstr imageTop_name(TJS_W("imageTop"));
			objthis_valclosure.PropGet(0, imageTop_name.c_str(), imageTop_name.GetHint(), &val, NULL);
			ImageTop = (tjs_int)val;
		}

		if (objthis->font_object)
		{
			tTJSVariantClosure val_closure;
			{
				tTJSVariant val(objthis->font_object, objthis->font_object);
				if (val.Type() == tvtObject)
				{
					val_closure = val.AsObjectClosureNoAddRef();
				}
			}
			if (val_closure.Object)
			{
				tTJSVariantClosure rresult_closure;
				{
					tTJSVariant args[15];
					args[0] = objthis_val;
					{
						iTJSDispatch2 *rect = TVPCreateRectObject(ClipRect.left, ClipRect.top, ClipRect.right, ClipRect.bottom);
						args[1] = tTJSVariant(rect, rect);
						rect->Release();
					}
					args[2] = x;
					args[3] = y;
					args[4] = text;
					args[5] = (tTVInteger)color;
					args[6] = met;
					args[7] = opa;
					args[8] = HoldAlpha;
					args[9] = aa;
					args[10] = shadowlevel;
					args[11] = (tTVInteger)shadowcolor;
					args[12] = shadowwidth;
					args[13] = shadowofsx;
					args[14] = shadowofsy;
					tTJSVariant *pargs[15] = {args +0, args +1, args +2, args +3, args +4, args +5, args +6, args +7, args +8, args +9, args +10, args +11, args +12, args +13, args +14};
					{
						tTJSVariant drawtext_val;
						static ttstr drawText_name(TJS_W("drawText"));
						val_closure.PropGet(0, drawText_name.c_str(), drawText_name.GetHint(), &drawtext_val, NULL);
						if (drawtext_val.Type() == tvtObject)
						{
							tTJSVariant rresult;
							tTJSVariantClosure drawtext_valclosure = drawtext_val.AsObjectClosure();
							drawtext_valclosure.FuncCall(0, NULL, 0, &rresult, 15, pargs, NULL);
							if (rresult.Type() == tvtObject)
							{
								rresult_closure = rresult.AsObjectClosure();
							}
						}
					}
				}
				tTVPRect update_rect;
				if (rresult_closure.Object)
				{
					tTJSVariant val;
					static ttstr left_name(TJS_W("left"));
					objthis_valclosure.PropGet(0, left_name.c_str(), left_name.GetHint(), &val, NULL);
					update_rect.left = (tjs_int)val;
					static ttstr top_name(TJS_W("top"));
					objthis_valclosure.PropGet(0, top_name.c_str(), top_name.GetHint(), &val, NULL);
					update_rect.top = (tjs_int)val;
					static ttstr right_name(TJS_W("right"));
					objthis_valclosure.PropGet(0, right_name.c_str(), right_name.GetHint(), &val, NULL);
					update_rect.right = (tjs_int)val;
					static ttstr bottom_name(TJS_W("bottom"));
					objthis_valclosure.PropGet(0, bottom_name.c_str(), bottom_name.GetHint(), &val, NULL);
					update_rect.bottom = (tjs_int)val;
				}
				if (ImageLeft != 0 || ImageTop != 0)
				{
					update_rect.add_offsets(ImageLeft, ImageTop);
				}
				{
					tTJSVariant args[4];
					args[0] = (tjs_int)update_rect.left;
					args[1] = (tjs_int)update_rect.top;
					args[2] = (tjs_int)update_rect.right;
					args[3] = (tjs_int)update_rect.bottom;
					tTJSVariant *pargs[4] = {args +0, args +1, args +2, args +3};
					if (objthis_valclosure.Object)
					{
						static ttstr update_name(TJS_W("update"));
						val_closure.FuncCall(0, update_name.c_str(), update_name.GetHint(), NULL, 4, pargs, NULL);
					}
				}
			}
		}
		return TJS_S_OK;
	}
private:
	iTJSDispatch2 *_objthis;
	iTJSDispatch2 *font_object;
};


NCB_GET_INSTANCE_HOOK(LayerKrFreetype)
{
	NCB_INSTANCE_GETTER(objthis)
	{
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj)
		{
			obj = new ClassT(objthis);
			SetNativeInstance(objthis, obj);
		}
		return obj;
	}

	~NCB_GET_INSTANCE_HOOK_CLASS () {}
};


NCB_ATTACH_CLASS_WITH_HOOK(LayerKrFreetype, Layer)
{
	RawCallback("drawText", &Class::drawText, 0);

	Property("font", &Class::GetFont, 0);
};

