#pragma once

#include "tjsCommHead.h"

#define TVPChBlurMulCopy TVPChBlurMulCopy_c
#define TVPChBlurAddMulCopy TVPChBlurAddMulCopy_c
#define TVPChBlurCopy TVPChBlurCopy_c

#define TVP_GL_FUNCNAME(funcname) funcname
#define TVP_GL_FUNC_DECL(rettype, funcname, arg)  rettype TVP_GL_FUNCNAME(funcname) arg
#define TVP_GL_FUNC_STATIC_DECL(rettype, funcname, arg)  static rettype TVP_GL_FUNCNAME(funcname) arg
#define TVP_GL_FUNC_EXTERN_DECL(rettype, funcname, arg)  extern rettype TVP_GL_FUNCNAME(funcname) arg

TVP_GL_FUNC_EXTERN_DECL(void, TVPChBlurMulCopy_c, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level) );
TVP_GL_FUNC_EXTERN_DECL(void, TVPChBlurAddMulCopy_c, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level) );
TVP_GL_FUNC_EXTERN_DECL(void, TVPChBlurCopy_c, (tjs_uint8 *dest, tjs_int destpitch, tjs_int destwidth, tjs_int destheight, const tjs_uint8 * src, tjs_int srcpitch, tjs_int srcwidth, tjs_int srcheight, tjs_int blurwidth, tjs_int blurlevel) );
