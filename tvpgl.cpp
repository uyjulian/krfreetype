// Copied tvpgl functions, because we can't rely on the core to have them.

#include "tvpgl.h"
#include <simde/x86/mmx.h>

static unsigned char TVPOpacityOnOpacityTable[256*256];
static unsigned char TVPNegativeMulTable[256*256];
/* following two are for 65-level anti-aliased letter drawing */
/* ( TVPApplyColorMap65_d and TVPApplyColorMap65_do ) */
static unsigned char TVPOpacityOnOpacityTable65[65*256];
static unsigned char TVPNegativeMulTable65[65*256];

TVP_GL_FUNC_DECL(void, TVPCreateTable, (void))
{
	int a,b;

	for(a=0; a<256; a++)
	{
		for(b=0; b<256; b++)
		{
			float c;
			int ci;
			int addr = b*256+ a;

			if(a)
			{
				float at = (float)(a/255.0), bt = (float)(b/255.0);
				c = bt / at;
				c /= (float)( (1.0 - bt + c) );
				ci = (int)(c*255);
				if(ci>=256) ci = 255; /* will not overflow... */
			}
			else
			{
				ci=255;
			}

			TVPOpacityOnOpacityTable[addr]=(unsigned char)ci;
				/* higher byte of the index is source opacity */
				/* lower byte of the index is destination opacity */
		
			TVPNegativeMulTable[addr] = (unsigned char)
				( 255 - (255-a)*(255-b)/ 255 ); 
		}
	}

	for(a=0; a<256; a++)
	{
		for(b=0; b<65; b++)
		{
			float c;
			int ci;
			int addr = b*256+ a;
			int bb;

			if(a)
			{
				float at = (float)(a / 255.0), bt = (float)(b / 64.0);
				c = bt / at;
				c /= (float)( (1.0 - bt + c) );
				ci = (int)(c*255);
				if(ci>=256) ci = 255; /* will not overflow... */
			}
			else
			{
				ci=255;
			}

			TVPOpacityOnOpacityTable65[addr]=(unsigned char)ci;
				/* higher byte of the index is source opacity */
				/* lower byte of the index is destination opacity */

			bb = b * 4;
			if(bb > 255) bb = 255;
			TVPNegativeMulTable65[addr] = (unsigned char)
				( 255 - (255-a)*(255-bb)/ 255 ); 
		}
	}

}

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

TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPSaturatedAdd, (tjs_uint32 a, tjs_uint32 b))
{
	/* Add each byte of packed 8bit values in two 32bit uint32, with saturation. */
	tjs_uint32 tmp = (  ( a & b ) + ( ((a ^ b)>>1u) & 0x7f7f7f7fu)  ) & 0x80808080u;
	tmp = (tmp<<1u) - (tmp>>7u);
	return (a + b - tmp) | tmp;
}

TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPMulColor, (tjs_uint32 color, tjs_uint32 fac))
{
	return (((((color & 0x00ff00u) * fac) & 0x00ff0000u) +
			(((color & 0xff00ffu) * fac) & 0xff00ff00u) ) >> 8u);
}

TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPAlphaAndColorToAdditiveAlpha, (tjs_uint32 alpha, tjs_uint32 color))
{
	return TVP_GL_FUNCNAME(TVPMulColor)(color, alpha) + (color & 0xff000000u);
}

TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPAlphaToAdditiveAlpha, (tjs_uint32 a))
{
	return TVP_GL_FUNCNAME(TVPAlphaAndColorToAdditiveAlpha)(a >> 24u, a);
}

TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPAddAlphaBlend_a_ca, (tjs_uint32 dest, tjs_uint32 sopa, tjs_uint32 sopa_inv, tjs_uint32 src))
{
	/*
		Di = sat(Si, (1-Sa)*Di)
		Da = Sa + Da - SaDa
	*/

	tjs_uint32 dopa = dest >> 24u;
	dopa = dopa + sopa - (dopa*sopa >> 8u);
	dopa -= (dopa >> 8u); /* adjust alpha */
	return (dopa << 24u) + 
		TVP_GL_FUNCNAME(TVPSaturatedAdd)((((dest & 0xff00ffu)*sopa_inv >> 8u) & 0xff00ffu) +
			(((dest & 0xff00u)*sopa_inv >> 8u) & 0xff00u), src);
}

TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPAddAlphaBlend_a_a, (tjs_uint32 dest, tjs_uint32 src))
{
	/*
		Di = sat(Si, (1-Sa)*Di)
		Da = Sa + Da - SaDa
	*/

	tjs_uint32 dopa = dest >> 24u;
	tjs_uint32 sopa = src >> 24u;
	dopa = dopa + sopa - (dopa*sopa >> 8u);
	dopa -= (dopa >> 8u); /* adjust alpha */
	sopa ^= 0xffu;
	src &= 0xffffffu;
	return (dopa << 24u) + 
		TVP_GL_FUNCNAME(TVPSaturatedAdd)((((dest & 0xff00ffu)*sopa >> 8u) & 0xff00ffu) +
			(((dest & 0xff00u)*sopa >> 8u) & 0xff00u), src);
}


TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPAddAlphaBlend_a_d, (tjs_uint32 dest, tjs_uint32 src))
{
	return TVP_GL_FUNCNAME(TVPAddAlphaBlend_a_a)(dest, TVP_GL_FUNCNAME(TVPAlphaToAdditiveAlpha)(src));
}

TVP_GL_FUNC_INLINE_DECL(tjs_uint32, TVPAddAlphaBlend_a_d_o, (tjs_uint32 dest, tjs_uint32 src, tjs_int opa))
{
	src = (src & 0xffffffu) + ((((src >> 24u) * opa) >> 8u) << 24u);
	return TVP_GL_FUNCNAME(TVPAddAlphaBlend_a_d)(dest, src);
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len))
{
	tjs_uint32   *    v3; // edi
	const tjs_uint32 *v4; // ebp
	tjs_uint32 *      v5; // esi
	tjs_uint32        v6; // eax
	simde__m64        v8; // mm4
	simde__m64        v9; // mm1

	if (len > 0)
	{
		v3 = dest;
		v4 = src;
		v5 = &dest[len];
		if (dest < v5)
		{
			do
			{
				v6 = *v4;
				if (*v4 < 0xFF000000)
				{
					v8  = simde_mm_set1_pi16(v6 >> 24);
					v9  = simde_m_punpcklbw(simde_mm_cvtsi32_si64(*v3), simde_mm_setzero_si64());
					*v3 = simde_mm_cvtsi64_si32(
						simde_m_packuswb(
							simde_m_paddb(v9, simde_m_psrlwi(simde_m_pmullw(simde_m_psubw(simde_m_punpcklbw(simde_mm_cvtsi32_si64(v6), simde_mm_setzero_si64()), v9), v8), 8u)),
							simde_mm_setzero_si64()));
				}
				else
				{
					*v3 = v6;
				}
				++v3;
				++v4;
			} while (v3 < v5);
		}
	}
	simde_m_empty();
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_HDA_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len))
{
	int          v3; // ecx
	tjs_uint32   v4; // eax
	simde__m64   v6; // mm2
	simde__m64   v7; // mm4
	tjs_uint32   v8; // eax
	simde__m64   v9; // mm1

	if (len - 1 >= 0)
	{
		v3 = len - 1;
		do
		{
			v4       = src[v3];
			v6       = simde_mm_cvtsi32_si64(v4);
			v7       = simde_mm_set1_pi16(v4 >> 24);
			v8       = dest[v3];
			v9       = simde_m_punpcklbw(simde_mm_cvtsi32_si64(v8), simde_mm_setzero_si64());
			dest[v3] = (v8 & 0xFF000000) | (simde_mm_cvtsi64_si32(
											 simde_m_packuswb(
												 simde_m_psrlwi(
													 simde_m_paddw(
														 simde_m_psllwi(v9, 8u),
														 simde_m_pmullw(simde_m_psubw(simde_m_punpcklbw(v6, simde_mm_setzero_si64()), v9), v7)),
													 8u),
												 simde_mm_setzero_si64())) &
											 0xFFFFFF);
			--v3;
		} while (v3 >= 0);
	}
	simde_m_empty();
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_HDA_o_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa))
{
	tjs_uint32 d1, s, d, sopa;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			s = *src;
			src++;
			d = *dest;
			sopa = ((s >> 24u) * opa) >> 8u;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + (((s & 0xff00ffu) - d1) * sopa >> 8u)) & 0xff00ffu) + (d & 0xff000000u);
			d &= 0xff00u;
			s &= 0xff00u;
			*dest = d1 + ((d + ((s - d) * sopa >> 8u)) & 0xff00u);
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_a_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len))
{
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			dest[___index] = TVP_GL_FUNCNAME(TVPAddAlphaBlend_a_d)(dest[___index], src[___index]);
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_ao_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa))
{
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			dest[___index] = TVP_GL_FUNCNAME(TVPAddAlphaBlend_a_d_o)(dest[___index], src[___index], opa);
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_d_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len))
{
	tjs_uint32 *      v3;  // edi
	const tjs_uint32 *v4;  // ebp
	simde__m64        v5;  // mm7
	tjs_uint32 *      v6;  // esi
	int               v7;  // eax
	simde__m64        v8;  // mm1
	simde__m64        v10; // mm4

	if (len > 0)
	{
		v3 = dest;
		v4 = src;
		v5 = simde_mm_cvtsi32_si64(0xFFFFFFu);
		v6 = &dest[len];
		if (dest < v6)
		{
			do
			{
				if (*v4 <= 0xFFFFFF)
				{
					++v3;
					++v4;
					continue;
				}
				v7  = (*v3 >> 24) + ((*v4 >> 16) & 0xFF00);
				v8  = simde_m_punpcklbw(simde_m_pand(simde_mm_cvtsi32_si64(*v3), v5), simde_mm_setzero_si64());
				v10 = simde_mm_set1_pi16(TVPOpacityOnOpacityTable[v7]);
				*v3 = (TVPNegativeMulTable[v7] << 24) | simde_mm_cvtsi64_si32(
															simde_m_packuswb(
																simde_m_psrlwi(
																	simde_m_paddw(
																		simde_m_psllwi(v8, 8u),
																		simde_m_pmullw(
																			simde_m_psubw(
																				simde_m_punpcklbw(simde_m_pand(simde_mm_cvtsi32_si64(*v4), v5), simde_mm_setzero_si64()),
																				v8),
																			v10)),
																	8u),
																simde_mm_setzero_si64()));
				++v3;
				++v4;
			} while (v3 < v6);
		}
	}
	simde_m_empty();
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_do_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa))
{
	tjs_uint32 d1, s, d, sopa, addr, destalpha;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			s = *src;
			src++;
			d = *dest;
			addr = (( (s>>24u)*opa) & 0xff00u) + (d>>24u);
			destalpha = TVPNegativeMulTable[addr]<<24u;
			sopa = TVPOpacityOnOpacityTable[addr];
			d1 = d & 0xff00ffu;
			d1 = (d1 + (((s & 0xff00ffu) - d1) * sopa >> 8u)) & 0xff00ffu;
			d &= 0xff00u;
			s &= 0xff00u;
			*dest = d1 + ((d + ((s - d) * sopa >> 8u)) & 0xff00u) + destalpha;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPAlphaBlend_o_c, (tjs_uint32 *dest, const tjs_uint32 *src, tjs_int len, tjs_int opa))
{
	tjs_uint32   *v4; // edi
	tjs_uint32   *v5; // ebp
	tjs_uint32 *  v6; // esi
	simde__m64    v8; // mm4
	simde__m64    v9; // mm1

	if (len > 0)
	{
		v4 = dest;
		v5 = (tjs_uint32   *)src;
		v6 = &dest[len];
		if (dest < v6)
		{
			do
			{
				v8  = simde_mm_set1_pi16((unsigned int)opa * (tjs_uint64)*v5 >> 32);
				v9  = simde_m_punpcklbw(simde_mm_cvtsi32_si64(*v4), simde_mm_setzero_si64());
				*v4 = simde_mm_cvtsi64_si32(
					simde_m_packuswb(
						simde_m_psrlwi(
							simde_m_paddw(simde_m_psllwi(v9, 8u), simde_m_pmullw(simde_m_psubw(simde_m_punpcklbw(simde_mm_cvtsi32_si64(*v5), simde_mm_setzero_si64()), v9), v8)),
							8u),
						simde_mm_setzero_si64()));
				++v4;
				++v5;
			} while (v4 < v6);
		}
	}
	simde_m_empty();
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32 d1, d, sopa;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			sopa = *src;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + ((c1 - d1) * sopa >> 8)) & 0xff00ffu);
			d &= 0xff00u;
			*dest = d1 | ((d + ((color - d) * sopa >> 8)) & 0x00ff00u);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_HDA_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32 d1, d, sopa;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			sopa = *src;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + ((c1 - d1) * sopa >> 8)) & 0xff00ffu) + (d & 0xff000000u);
			d &= 0xff00u;
			*dest = d1 | ((d + ((color - d) * sopa >> 8)) & 0x00ff00u);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_HDA_o_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 d1, d, sopa;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			sopa = (*src * opa) >> 8u;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + ((c1 - d1) * sopa >> 8)) & 0xff00ffu) + (d & 0xff000000u);
			d &= 0x00ff00u;
			*dest = d1 | ((d + ((color - d) * sopa >> 8)) & 0x00ff00u);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_a_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			tjs_int s_tmp = *src;
			tjs_uint32 tmp =
				((s_tmp * (c1    & 0xff00ffu) >> 8) & 0xff00ffu) + 
				((s_tmp * (color & 0x00ff00u) >> 8) & 0x00ff00u);
			s_tmp <<= (8u - 8);
			s_tmp -= (s_tmp >> 8u); /* adjust alpha */
			*dest = TVP_GL_FUNCNAME(TVPAddAlphaBlend_a_ca)(*dest, s_tmp, s_tmp ^ 0xffu, tmp);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_ao_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			tjs_int s_tmp = (*src * opa) >> 8u;
			tjs_uint32 tmp =
				((s_tmp * (c1    & 0xff00ffu) >> 8) & 0xff00ffu) + 
				((s_tmp * (color & 0x00ff00u) >> 8) & 0x00ff00u);
			s_tmp <<= (8u - 8);
			s_tmp -= (s_tmp >> 8u); /* adjust alpha */
			*dest = TVP_GL_FUNCNAME(TVPAddAlphaBlend_a_ca)(*dest, s_tmp, s_tmp ^ 0xffu, tmp);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_d_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32 d1, d, sopa, addr, destalpha;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			addr = (*src<<8u) + (d>>24u);
			destalpha = TVPNegativeMulTable[addr]<<24u;
			sopa = TVPOpacityOnOpacityTable[addr];
			d1 = d & 0xff00ffu;
			d1 = (d1 + ((c1 - d1) * sopa >> 8u)) & 0xff00ffu;
			d &= 0x00ff00u;
			*dest = d1 + ((d + ((color - d) * sopa >> 8u)) & 0x00ff00u) + destalpha;
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_do_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 d1, d, sopa, addr, destalpha;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			addr = ((*src * opa) & 0xff00u) + (d>>24u);
			destalpha = TVPNegativeMulTable[addr]<<24u;
			sopa = TVPOpacityOnOpacityTable[addr];
			d1 = d & 0xff00ffu;
			d1 = (d1 + ((c1 - d1) * sopa >> 8u)) & 0xff00ffu;
			d &= 0x00ff00u;
			*dest = d1 + ((d + ((color - d) * sopa >> 8u)) & 0x00ff00u) + destalpha;
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap_o_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 d1, d, sopa;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			sopa = (*src * opa) >> 8u;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + ((c1 - d1) * sopa >> 8)) & 0xff00ffu);
			d &= 0x00ff00u;
			*dest = d1 | ((d + ((color - d) * sopa >> 8)) & 0x00ff00u);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32   *   v4;  // edi
	const tjs_uint8 *v5;  // ebp
	tjs_uint32 *     v6;  // edx
	simde__m64       v7;  // mm7
	simde__m64       v9;  // mm4
	simde__m64       v10; // mm1

	if (len > 0)
	{
		v4 = dest;
		v5 = src;
		v6 = &dest[len];
		v7 = simde_m_punpcklbw(simde_mm_cvtsi32_si64(color), simde_mm_setzero_si64());
		if (dest < v6)
		{
			do
			{
				v9  = simde_mm_set1_pi16(*v5);
				v10 = simde_m_punpcklbw(simde_mm_cvtsi32_si64(*v4), simde_mm_setzero_si64());
				*v4 = simde_mm_cvtsi64_si32(simde_m_packuswb(simde_m_paddw(v10, simde_m_psrawi(simde_m_pmullw(simde_m_psubw(v7, v10), v9), 6u)), simde_mm_setzero_si64()));
				++v4;
				++v5;
			} while (v4 < v6);
		}
	}
	simde_m_empty();
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_HDA_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32 d1, d, sopa;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			sopa = *src;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + ((c1 - d1) * sopa >> 6)) & 0xff00ffu) + (d & 0xff000000u);
			d &= 0xff00u;
			*dest = d1 | ((d + ((color - d) * sopa >> 6)) & 0x00ff00u);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_HDA_o_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 d1, d, sopa;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			sopa = (*src * opa) >> 8u;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + ((c1 - d1) * sopa >> 6)) & 0xff00ffu) + (d & 0xff000000u);
			d &= 0x00ff00u;
			*dest = d1 | ((d + ((color - d) * sopa >> 6)) & 0x00ff00u);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_a_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32   *   v4;  // edi
	const tjs_uint8 *v5;  // ebp
	tjs_uint32 *     v6;  // esi
	simde__m64       v7;  // mm7
	simde__m64       v9;  // mm3
	simde__m64       v10; // mm3
	simde__m64       v11; // mm1

	if (len > 0)
	{
		v4 = dest;
		v5 = src;
		v6 = &dest[len];
		v7 = simde_m_por(simde_m_punpcklbw(simde_mm_cvtsi32_si64(color & 0xFFFFFF), simde_mm_setzero_si64()), simde_m_psllqi(simde_mm_cvtsi32_si64(0x100u), 0x30u));
		if (dest < v6)
		{
			do
			{
				v9  = simde_mm_set1_pi16(*v5);
				v10 = v9;
				v11 = simde_m_punpcklbw(simde_mm_cvtsi32_si64(*v4), simde_mm_setzero_si64());
				*v4 = simde_mm_cvtsi64_si32(
					simde_m_packuswb(
						simde_m_paddw(simde_m_psubw(v11, simde_m_psrlwi(simde_m_pmullw(v11, v10), 6u)), simde_m_psrlwi(simde_m_pmullw(v10, v7), 6u)),
						simde_mm_setzero_si64()));
				++v5;
				++v4;
			} while (v4 < v6);
		}
	}
	simde_m_empty();
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_ao_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			tjs_int s_tmp = (*src * opa) >> 8u;
			tjs_uint32 tmp =
				((s_tmp * (c1    & 0xff00ffu) >> 6) & 0xff00ffu) + 
				((s_tmp * (color & 0x00ff00u) >> 6) & 0x00ff00u);
			s_tmp <<= (8u - 6);
			s_tmp -= (s_tmp >> 8u); /* adjust alpha */
			*dest = TVP_GL_FUNCNAME(TVPAddAlphaBlend_a_ca)(*dest, s_tmp, s_tmp ^ 0xffu, tmp);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_d_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color))
{
	tjs_uint32 *     v4;  // edi
	const tjs_uint8 *v5;  // ebp
	tjs_uint32 *     v6;  // esi
	simde__m64       v7;  // mm7
	int              v8;  // eax
	simde__m64       v9;  // mm1
	simde__m64       v11; // mm4

	if (len > 0)
	{
		v4 = dest;
		v5 = src;
		v6 = &dest[len];
		v7 = simde_m_punpcklbw(simde_mm_cvtsi32_si64(color & 0xFFFFFF), simde_mm_setzero_si64());
		if (dest < v6)
		{
			do
			{
				v8  = (*v4 >> 24) + (*v5 << 8);
				v9  = simde_m_punpcklbw(simde_m_psrlqi(simde_m_psllqi(simde_mm_cvtsi32_si64(*v4), 0x28u), 0x28u), simde_mm_setzero_si64());
				v11 = simde_mm_set1_pi16(TVPOpacityOnOpacityTable65[v8]);
				*v4 = (TVPNegativeMulTable65[v8] << 24) | simde_mm_cvtsi64_si32(
															  simde_m_packuswb(
																  simde_m_psrlwi(
																	  simde_m_paddw(simde_m_psllwi(v9, 8u), simde_m_pmullw(simde_m_psubw(v7, v9), v11)),
																	  8u),
																  simde_mm_setzero_si64()));
				++v5;
				++v4;
			} while (v4 < v6);
		}
	}
	simde_m_empty();
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_do_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 d1, d, sopa, addr, destalpha;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			addr = ((*src * opa) & 0xff00u) + (d>>24u);
			destalpha = TVPNegativeMulTable65[addr]<<24u;
			sopa = TVPOpacityOnOpacityTable65[addr];
			d1 = d & 0xff00ffu;
			d1 = (d1 + ((c1 - d1) * sopa >> 8u)) & 0xff00ffu;
			d &= 0x00ff00u;
			*dest = d1 + ((d + ((color - d) * sopa >> 8u)) & 0x00ff00u) + destalpha;
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPApplyColorMap65_o_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_uint32 color, tjs_int opa))
{
	tjs_uint32 d1, d, sopa;
	tjs_uint32 c1 = color & 0xff00ffu;
	color = color & 0x00ff00u;
	for(int lu_n = 0; lu_n < len; lu_n++)
	{
		{
			d = *dest;
			sopa = (*src * opa) >> 8u;
			d1 = d & 0xff00ffu;
			d1 = ((d1 + ((c1 - d1) * sopa >> 6)) & 0xff00ffu);
			d &= 0x00ff00u;
			*dest = d1 | ((d + ((color - d) * sopa >> 6)) & 0x00ff00u);
			src++;
			dest++;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPBLExpand1BitTo8BitPal_c, (tjs_uint8 *dest, const tjs_uint8 *buf, tjs_int len, const tjs_uint32 *pal))
{
	tjs_uint8 p[2];
	tjs_uint8 *d=dest, *dlim;
	tjs_uint8 b;

	p[0u] = pal[0u]&0xffu, p[1u] = pal[1u]&0xffu;
	dlim = dest + len-7u;
	while(d < dlim)
	{
		b = *(buf++);
		d[0u] = p[(tjs_uint)(b&(tjs_uint)0x80u)>>7u];
		d[1u] = p[(tjs_uint)(b&(tjs_uint)0x40u)>>6u];
		d[2u] = p[(tjs_uint)(b&(tjs_uint)0x20u)>>5u];
		d[3u] = p[(tjs_uint)(b&(tjs_uint)0x10u)>>4u];
		d[4u] = p[(tjs_uint)(b&(tjs_uint)0x08u)>>3u];
		d[5u] = p[(tjs_uint)(b&(tjs_uint)0x04u)>>2u];
		d[6u] = p[(tjs_uint)(b&(tjs_uint)0x02u)>>1u];
		d[7u] = p[(tjs_uint)(b&(tjs_uint)0x01u)   ];
		d += 8u;
	}
	dlim = dest + len;
	b = *buf;
	while(d<dlim)
	{
		*(d++) = (b&0x80u) ? p[1u] : p[0u];
		b<<=1u;
	}
}

TVP_GL_FUNC_DECL(void, TVPChBlurAddMulCopy65_c, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level))
{
	tjs_int a;
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			a = dest[___index] +(src[___index] * level >> 18u);;
			if(a>=64u) a = 64u;;
			dest[___index] = a;;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPChBlurMulCopy65_c, (tjs_uint8 *dest, const tjs_uint8 *src, tjs_int len, tjs_int level))
{
	tjs_int a;
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			a = (src[___index] * level >> 18u);;
			if(a>=64u) a = 64u;;
			dest[___index] = a;;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPChBlurCopy65_c, (tjs_uint8 *dest, tjs_int destpitch, tjs_int destwidth, tjs_int destheight, const tjs_uint8 * src, tjs_int srcpitch, tjs_int srcwidth, tjs_int srcheight, tjs_int blurwidth, tjs_int blurlevel))
{
	tjs_int lvsum, x, y;

	/* clear destination */
	memset(dest, 0u, destpitch*destheight);

	/* compute filter level */
	lvsum = 0u;
	for(y = -blurwidth; y <= blurwidth; y++)
	{
		for(x = -blurwidth; x <= blurwidth; x++)
		{
			tjs_int len = TVP_GL_FUNCNAME(fast_int_hypot)(x, y);
			if(len <= blurwidth)
				lvsum += (blurwidth - len +1u);
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
				for(sy = 0; sy < srcheight; sy++)
				{
					TVPChBlurAddMulCopy65(dest + (y + sy + blurwidth)*destpitch + x + blurwidth, 
						src + sy * srcpitch, srcwidth, len);
				}
			}
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPRemoveOpacity_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len))
{
	tjs_uint32 d;
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			d = dest[___index];;
			dest[___index] = (d & 0xffffffu) + ( (((d>>24u) * (255u-src[___index])) << 16u) & 0xff000000u);;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPRemoveOpacity_o_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_int strength))
{
	tjs_uint32 d;

	if(strength > 127u) strength ++; /* adjust for error */
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			d = dest[___index];;
			dest[___index] = (d & 0xffffffu) + ( (((d>>24u) * (65535u-src[___index]*strength )) << 8u) & 0xff000000u);;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPRemoveOpacity65_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len))
{
	tjs_uint32 d;
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			d = dest[___index];;
			dest[___index] = (d & 0xffffffu) + ( (((d>>24u) * (64u-src[___index])) << 18u) & 0xff000000u);;
		}
	}
}

TVP_GL_FUNC_DECL(void, TVPRemoveOpacity65_o_c, (tjs_uint32 *dest, const tjs_uint8 *src, tjs_int len, tjs_int strength))
{
	tjs_uint32 d;

	if(strength > 127u) strength ++; /* adjust for error */
	{
		for(int ___index = 0; ___index < len; ___index++)
		{
			d = dest[___index];;
			dest[___index] = (d & 0xffffffu) + ( (((d>>24u) * (16384u-src[___index]*strength )) << 10u) & 0xff000000u);;
		}
	}
}
