#pragma once

#include "tjsCommHead.h"

#define TVPChBlurMulCopy TVPChBlurMulCopy_c
#define TVPChBlurAddMulCopy TVPChBlurAddMulCopy_c
#define TVPChBlurCopy TVPChBlurCopy_c
#define TVPAlphaBlend TVPAlphaBlend_c
#define TVPAlphaBlend_HDA TVPAlphaBlend_HDA_c
#define TVPAlphaBlend_HDA_o TVPAlphaBlend_HDA_o_c
#define TVPAlphaBlend_a TVPAlphaBlend_a_c
#define TVPAlphaBlend_ao TVPAlphaBlend_ao_c
#define TVPAlphaBlend_d TVPAlphaBlend_d_c
#define TVPAlphaBlend_do TVPAlphaBlend_do_c
#define TVPAlphaBlend_o TVPAlphaBlend_o_c
#define TVPApplyColorMap TVPApplyColorMap_c
#define TVPApplyColorMap_HDA TVPApplyColorMap_HDA_c
#define TVPApplyColorMap_HDA_o TVPApplyColorMap_HDA_o_c
#define TVPApplyColorMap_a TVPApplyColorMap_a_c
#define TVPApplyColorMap_ao TVPApplyColorMap_ao_c
#define TVPApplyColorMap_d TVPApplyColorMap_d_c
#define TVPApplyColorMap_do TVPApplyColorMap_do_c
#define TVPApplyColorMap_o TVPApplyColorMap_o_c
#define TVPApplyColorMap65 TVPApplyColorMap65_c
#define TVPApplyColorMap65_HDA TVPApplyColorMap65_HDA_c
#define TVPApplyColorMap65_HDA_o TVPApplyColorMap65_HDA_o_c
#define TVPApplyColorMap65_a TVPApplyColorMap65_a_c
#define TVPApplyColorMap65_ao TVPApplyColorMap65_ao_c
#define TVPApplyColorMap65_d TVPApplyColorMap65_d_c
#define TVPApplyColorMap65_do TVPApplyColorMap65_do_c
#define TVPApplyColorMap65_o TVPApplyColorMap65_o_c
#define TVPBLExpand1BitTo8BitPal TVPBLExpand1BitTo8BitPal_c
#define TVPChBlurAddMulCopy65 TVPChBlurAddMulCopy65_c
#define TVPChBlurMulCopy65 TVPChBlurMulCopy65_c
#define TVPChBlurCopy65 TVPChBlurCopy65_c
#define TVPRemoveOpacity TVPRemoveOpacity_c
#define TVPRemoveOpacity_o TVPRemoveOpacity_o_c
#define TVPRemoveOpacity65 TVPRemoveOpacity65_c
#define TVPRemoveOpacity65_o TVPRemoveOpacity65_o_c

#define TVP_INLINE_FUNC inline
#define TVP_GL_FUNCNAME(funcname) funcname
#define TVP_GL_FUNC_DECL(rettype, funcname, arg)  rettype TVP_GL_FUNCNAME(funcname) arg
#define TVP_GL_FUNC_STATIC_DECL(rettype, funcname, arg)  static rettype TVP_GL_FUNCNAME(funcname) arg
#define TVP_GL_FUNC_INLINE_DECL(rettype, funcname, arg)  static rettype TVP_INLINE_FUNC TVP_GL_FUNCNAME(funcname) arg
#define TVP_GL_FUNC_EXTERN_DECL(rettype, funcname, arg)  extern rettype TVP_GL_FUNCNAME(funcname) arg
#define TVP_GL_FUNC_PTR_EXTERN_DECL TVP_GL_FUNC_EXTERN_DECL

TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPCreateTable, (void));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPChBlurMulCopy, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level) );
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPChBlurAddMulCopy, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level) );
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPChBlurCopy, (tjs_uint8 *dest, tjs_int destpitch, tjs_int destwidth, tjs_int destheight, const tjs_uint8 * src, tjs_int srcpitch, tjs_int srcwidth, tjs_int srcheight, tjs_int blurwidth, tjs_int blurlevel) );
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend_HDA, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend_HDA_o, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend_a, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend_ao, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend_d, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend_do, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPAlphaBlend_o, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap_HDA, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap_HDA_o, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap_ao, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap_d, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap_do, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap_o, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65_HDA, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65_HDA_o, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65_a, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65_ao, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65_d, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65_do, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPApplyColorMap65_o, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPBLExpand1BitTo8BitPal, (tjs_uint8 *dest, const tjs_uint8 *buf, tjs_int len, const tjs_uint32 *pal));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPChBlurAddMulCopy65, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPChBlurMulCopy65, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPChBlurCopy65, (tjs_uint8 *dest, tjs_int destpitch, tjs_int destwidth, tjs_int destheight, const tjs_uint8 * src, tjs_int srcpitch, tjs_int srcwidth, tjs_int srcheight, tjs_int blurwidth, tjs_int blurlevel));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPRemoveOpacity, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPRemoveOpacity_o, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_int strength));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPRemoveOpacity65, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len));
TVP_GL_FUNC_PTR_EXTERN_DECL(void, TVPRemoveOpacity65_o, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_int strength));
