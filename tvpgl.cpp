// Copied tvpgl functions, because we can't rely on the core to have them.

#include "tvpgl.h"

/* fast_int_hypot from http://demo.and.or.jp/makedemo/effect/math/hypot/fast_hypot.c */
TVP_GL_FUNC_STATIC_DECL(tjs_uint, fast_int_hypot, (tjs_int lx, tjs_int ly))
{
	tjs_uint len1, len2,t,length;

/*	lx = abs(lx); */
/*	ly = abs(ly); */
	if(lx<0) lx = -lx;
	if(ly<0) ly = -ly;
	/*
		CWD
		XOR EAX,EDX
		SUB EAX,EDX
	*/
	
	if (lx >= ly)
	{
		len1 = lx ; len2 = ly;
	}
	else
	{
		len1 = ly ; len2 = lx;
	}

	t = len2 + (len2 >> 1u) ;
	length = len1 - (len1 >> 5u) - (len1 >> 7u) + (t >> 2u) + (t >> 6u) ;
	return length;
}

/*export*/
TVP_GL_FUNC_DECL(void, TVPChBlurMulCopy_c, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level) )
{
	tjs_int a, b;
	{
		int ___index = 0;
		len -= (4-1);

		while(___index < len)
		{
			a = (src[(___index+(0*2))] * level >> 18);
			b = (src[(___index+(0*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(0*2))] = a;
			dest[(___index+(0*2+1))] = b;
			a = (src[(___index+(1*2))] * level >> 18);
			b = (src[(___index+(1*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(1*2))] = a;
			dest[(___index+(1*2+1))] = b;
			___index += 4;
		}

		len += (4-1);

		while(___index < len)
		{
			a = (src[___index] * level >> 18);;
			if(a>=255) a = 255;
			dest[___index] = a;;
			___index ++;
		}
	}
}


/*export*/
TVP_GL_FUNC_DECL(void, TVPChBlurAddMulCopy_c, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level) )
{
	tjs_int a, b;
	{
		int ___index = 0;
		len -= (4-1);

		while(___index < len)
		{
			a = dest[(___index+(0*2))] +(src[(___index+(0*2))] * level >> 18);
			b = dest[(___index+(0*2+1))] +(src[(___index+(0*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(0*2))] = a;
			dest[(___index+(0*2+1))] = b;
			a = dest[(___index+(1*2))] +(src[(___index+(1*2))] * level >> 18);
			b = dest[(___index+(1*2+1))] +(src[(___index+(1*2+1))] * level >> 18);
			if(a>=255) a = 255;
			if(b>=255) b = 255;
			dest[(___index+(1*2))] = a;
			dest[(___index+(1*2+1))] = b;
			___index += 4;
		}

		len += (4-1);

		while(___index < len)
		{
			a = dest[___index] +(src[___index] * level >> 18);;
			if(a>=255) a = 255;;
			dest[___index] = a;;
			___index ++;
		}
	}
}

/*export*/
TVP_GL_FUNC_DECL(void, TVPChBlurCopy_c, (tjs_uint8 *dest, tjs_int destpitch, tjs_int destwidth, tjs_int destheight, const tjs_uint8 * src, tjs_int srcpitch, tjs_int srcwidth, tjs_int srcheight, tjs_int blurwidth, tjs_int blurlevel) )
{
	tjs_int lvsum, x, y;

	/* clear destination */
	memset(dest, 0u, destpitch*destheight);

	/* compute filter level */
	lvsum = 0;
	for(y = -blurwidth; y <= blurwidth; y++)
	{
		for(x = -blurwidth; x <= blurwidth; x++)
		{
			tjs_int len = TVP_GL_FUNCNAME(fast_int_hypot)(x, y);
			if(len <= blurwidth)
				lvsum += (blurwidth - len +1);
		}
	}

	if(lvsum) lvsum = (1u<<18u)/lvsum; else lvsum=(1u<<18u);

	/* apply */
	for(y = -blurwidth; y <= blurwidth; y++)
	{
		for(x = -blurwidth; x <= blurwidth; x++)
		{
			tjs_int len = TVP_GL_FUNCNAME(fast_int_hypot)(x, y);
			if(len <= blurwidth)
			{
				tjs_int sy;

				len = blurwidth - len +1u;
				len *= lvsum;
				len *= blurlevel;
				len >>= 8u;
				for(sy = 0u; sy < srcheight; sy++)
				{
					TVPChBlurAddMulCopy(dest + (y + sy + blurwidth)*destpitch + x + blurwidth, 
						src + sy * srcpitch, srcwidth, len);
				}
			}
		}
	}
}
