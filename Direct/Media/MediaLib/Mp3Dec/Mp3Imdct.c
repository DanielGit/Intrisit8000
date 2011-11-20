//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－IMDCT算法部分
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
//  V0.1    2005-8      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]

#include "Mp3Prv.h"

#ifdef CONFIG_DECODE_MP3_ENABLE



/*
* windowing coefficients for long blocks
* derived from section 2.4.3.4.10.3 of ISO/IEC 11172-3
*
* _LongWindowTable[i] = sin((PI / 36) * (i + 1/2))
*/
static short const _LongWindowTable[36] = 
{
	CONST_SHORT(0x00b2aa3e) /* 0.043619387 */, CONST_SHORT(0x0216a2a2) /* 0.130526192 */,
		CONST_SHORT(0x03768962) /* 0.216439614 */, CONST_SHORT(0x04cfb0e2) /* 0.300705800 */,
		CONST_SHORT(0x061f78aa) /* 0.382683432 */, CONST_SHORT(0x07635284) /* 0.461748613 */,
		CONST_SHORT(0x0898c779) /* 0.537299608 */, CONST_SHORT(0x09bd7ca0) /* 0.608761429 */,
		CONST_SHORT(0x0acf37ad) /* 0.675590208 */, CONST_SHORT(0x0bcbe352) /* 0.737277337 */,
		CONST_SHORT(0x0cb19346) /* 0.793353340 */, CONST_SHORT(0x0d7e8807) /* 0.843391446 */,
		
		CONST_SHORT(0x0e313245) /* 0.887010833 */, CONST_SHORT(0x0ec835e8) /* 0.923879533 */,
		CONST_SHORT(0x0f426cb5) /* 0.953716951 */, CONST_SHORT(0x0f9ee890) /* 0.976296007 */,
		CONST_SHORT(0x0fdcf549) /* 0.991444861 */, CONST_SHORT(0x0ffc19fd) /* 0.999048222 */,
		CONST_SHORT(0x0ffc19fd) /* 0.999048222 */, CONST_SHORT(0x0fdcf549) /* 0.991444861 */,
		CONST_SHORT(0x0f9ee890) /* 0.976296007 */, CONST_SHORT(0x0f426cb5) /* 0.953716951 */,
		CONST_SHORT(0x0ec835e8) /* 0.923879533 */, CONST_SHORT(0x0e313245) /* 0.887010833 */,
		
		CONST_SHORT(0x0d7e8807) /* 0.843391446 */, CONST_SHORT(0x0cb19346) /* 0.793353340 */,
		CONST_SHORT(0x0bcbe352) /* 0.737277337 */, CONST_SHORT(0x0acf37ad) /* 0.675590208 */,
		CONST_SHORT(0x09bd7ca0) /* 0.608761429 */, CONST_SHORT(0x0898c779) /* 0.537299608 */,
		CONST_SHORT(0x07635284) /* 0.461748613 */, CONST_SHORT(0x061f78aa) /* 0.382683432 */,
		CONST_SHORT(0x04cfb0e2) /* 0.300705800 */, CONST_SHORT(0x03768962) /* 0.216439614 */,
		CONST_SHORT(0x0216a2a2) /* 0.130526192 */, CONST_SHORT(0x00b2aa3e) /* 0.043619387 */,
};

static short const _ShortWindowTable[12] = 
{
	CONST_SHORT(0x0216a2a2) /* 0.130526192 */, 
		CONST_SHORT(0x061f78aa) /* 0.382683432 */,
		CONST_SHORT(0x09bd7ca0) /* 0.608761429 */, 
		CONST_SHORT(0x0cb19346) /* 0.793353340 */,
		CONST_SHORT(0x0ec835e8) /* 0.923879533 */, 
		CONST_SHORT(0x0fdcf549) /* 0.991444861 */,
		CONST_SHORT(0x0fdcf549) /* 0.991444861 */, 
		CONST_SHORT(0x0ec835e8) /* 0.923879533 */,
		CONST_SHORT(0x0cb19346) /* 0.793353340 */, 
		CONST_SHORT(0x09bd7ca0) /* 0.608761429 */,
		CONST_SHORT(0x061f78aa) /* 0.382683432 */, 
		CONST_SHORT(0x0216a2a2) /* 0.130526192 */
};


/*
* IMDCT coefficients for short blocks
* derived from section 2.4.3.4.10.2 of ISO/IEC 11172-3
*
* _ShortImdctTable[i/even][k] = cos((PI / 24) * (2 *       (i / 2) + 7) * (2 * k + 1))
* _ShortImdctTable[i /odd][k] = cos((PI / 24) * (2 * (6 + (i-1)/2) + 7) * (2 * k + 1))
*/
static short const _ShortImdctTable[] = 
{
	/*  0 */  
	CONST_SHORT(0x09bd7ca0) /*  0.608761429 */,
		-CONST_SHORT(0x0ec835e8) /* -0.923879533 */,
		-CONST_SHORT(0x0216a2a2) /* -0.130526192 */,
		CONST_SHORT(0x0fdcf549) /*  0.991444861 */,
		-CONST_SHORT(0x061f78aa) /* -0.382683432 */,
		-CONST_SHORT(0x0cb19346) /* -0.793353340 */,
		
		/*  6 */  
		-CONST_SHORT(0x0cb19346) /* -0.793353340 */,
		CONST_SHORT(0x061f78aa) /*  0.382683432 */,
		CONST_SHORT(0x0fdcf549) /*  0.991444861 */,
		CONST_SHORT(0x0216a2a2) /*  0.130526192 */,
		-CONST_SHORT(0x0ec835e8) /* -0.923879533 */,
		-CONST_SHORT(0x09bd7ca0) /* -0.608761429 */,
		
		/*  1 */  
		CONST_SHORT(0x061f78aa) /*  0.382683432 */,
		-CONST_SHORT(0x0ec835e8) /* -0.923879533 */,
		CONST_SHORT(0x0ec835e8) /*  0.923879533 */,
		-CONST_SHORT(0x061f78aa) /* -0.382683432 */,
		-CONST_SHORT(0x061f78aa) /* -0.382683432 */,
		CONST_SHORT(0x0ec835e8) /*  0.923879533 */,
		
		/*  7 */  
		-CONST_SHORT(0x0ec835e8) /* -0.923879533 */,
		-CONST_SHORT(0x061f78aa) /* -0.382683432 */,
		CONST_SHORT(0x061f78aa) /*  0.382683432 */,
		CONST_SHORT(0x0ec835e8) /*  0.923879533 */,
		CONST_SHORT(0x0ec835e8) /*  0.923879533 */,
		CONST_SHORT(0x061f78aa) /*  0.382683432 */ ,
		
		/*  2 */  
		CONST_SHORT(0x0216a2a2) /*  0.130526192 */,
		-CONST_SHORT(0x061f78aa) /* -0.382683432 */,
		CONST_SHORT(0x09bd7ca0) /*  0.608761429 */,
		-CONST_SHORT(0x0cb19346) /* -0.793353340 */,
		CONST_SHORT(0x0ec835e8) /*  0.923879533 */,
		-CONST_SHORT(0x0fdcf549) /* -0.991444861 */ ,
		
		/*  8 */  
		-CONST_SHORT(0x0fdcf549) /* -0.991444861 */,
		-CONST_SHORT(0x0ec835e8) /* -0.923879533 */,
		-CONST_SHORT(0x0cb19346) /* -0.793353340 */,
		-CONST_SHORT(0x09bd7ca0) /* -0.608761429 */,
		-CONST_SHORT(0x061f78aa) /* -0.382683432 */,
		-CONST_SHORT(0x0216a2a2) /* -0.130526192 */ 
};


/* scale[i] = 2 * cos(PI * (2 * i + 1) / (2 * 18)) */
static short const sdctII_scale[9] = 
{
	CONST2_SHORT(0x1fe0d3b4), CONST2_SHORT(0x1ee8dd47), CONST2_SHORT(0x1d007930),
		CONST2_SHORT(0x1a367e59), CONST2_SHORT(0x16a09e66), CONST2_SHORT(0x125abcf8),
		CONST2_SHORT(0x0d8616bc), CONST2_SHORT(0x08483ee1), CONST2_SHORT(0x02c9fad7)
};

/* scale[i] = 2 * cos(PI * (2 * i + 1) / (4 * 18)) */
static short const DctIV_scale[18] = 
{
	CONST2_SHORT(0x1ff833fa), CONST2_SHORT(0x1fb9ea93), CONST2_SHORT(0x1f3dd120),
		CONST2_SHORT(0x1e84d969), CONST2_SHORT(0x1d906bcf), CONST2_SHORT(0x1c62648b),
		CONST2_SHORT(0x1afd100f), CONST2_SHORT(0x1963268b), CONST2_SHORT(0x1797c6a4),
		CONST2_SHORT(0x159e6f5b), CONST2_SHORT(0x137af940), CONST2_SHORT(0x11318ef3),
		CONST2_SHORT(0x0ec6a507), CONST2_SHORT(0x0c3ef153), CONST2_SHORT(0x099f61c5),
		CONST2_SHORT(0x06ed12c5), CONST2_SHORT(0x042d4544), CONST2_SHORT(0x0165547c)
};
	

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
/*
* windowing coefficients for short blocks
* derived from section 2.4.3.4.10.3 of ISO/IEC 11172-3
*
* _ShortWindowTable[i] = sin((PI / 12) * (i + 1/2))
*/
static inline void Mp3FastSdct(MP3INT const x[9], MP3INT y[18])
{
	int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
	int a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25;
	int m0, m1, m2, m3, m4, m5, m6, m7;
	
	enum 
	{
		c0 = CONST2_SHORT(0x1f838b8d),     /* 2 * cos( 1 * PI / 18) */
			c1 = CONST2_SHORT(0x1bb67ae8),     /* 2 * cos( 3 * PI / 18) */
			c2 = CONST2_SHORT(0x18836fa3),     /* 2 * cos( 4 * PI / 18) */
			c3 = CONST2_SHORT(0x1491b752),     /* 2 * cos( 5 * PI / 18) */
			c4 = CONST2_SHORT(0x0af1d43a),     /* 2 * cos( 7 * PI / 18) */
			c5 = CONST2_SHORT(0x058e86a0),     /* 2 * cos( 8 * PI / 18) */
			c6 = -CONST2_SHORT(0x1e11f642)   /* 2 * cos(16 * PI / 18) */
	};
	
	a0 = x[3] + x[5];
	a1 = x[3] - x[5];
	a2 = x[6] + x[2];
	a3 = x[6] - x[2];
	a4 = x[1] + x[7];
	a5 = x[1] - x[7];
	a6 = x[8] + x[0];
	a7 = x[8] - x[0];
	
	a8 = a0 + a2;
	a9 = a0 - a2;
	a10 = a0 - a6;
	a11 = a2 - a6;
	a12 = a8 + a6;
	a13 = a1 - a3;
	a14 = a13 + a7;
	a15 = a3 + a7;
	a16 = a1 - a7;
	a17 = a1 + a3;
	
	m0 = IMUL2(a17, -c3);
	m1 = IMUL2(a16, -c0);
	m2 = IMUL2(a15, -c4);
	m3 = IMUL2(a14, -c1);
	m4 = IMUL2(a5, -c1);
	m5 = IMUL2(a11, -c6);
	m6 = IMUL2(a10, -c5);
	m7 = IMUL2(a9, -c2);
	
	a18 = x[4] + a4;
	a19 = 2 * x[4] - a4;
	a20 = a19 + m5;
	a21 = a19 - m5;
	a22 = a19 + m6;
	a23 = m4 + m2;
	a24 = m4 - m2;
	a25 = m4 + m1;
	
	/* output to every other slot for convenience */
	y[ 0] = a18 + a12;
	y[ 2] = m0 - a25;
	y[ 4] = m7 - a20;
	y[ 6] = m3;
	y[ 8] = a21 - m6;
	y[10] = a24 - m1;
	y[12] = a12 - 2 * a18;
	y[14] = a23 + m0;
	y[16] = a22 + m7;
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static inline void sdctII(MP3INT const x[18], MP3INT X[18])
{
	MP3INT tmp[9];
	MP3INT *ptmp;
	const MP3INT *pca, *pcb;
	const short *pscale;
	int t;
	int i;
	
	/* divide the 18-point SDCT-II into two 9-point SDCT-IIs */
	
	/* even input butterfly */
	pca = x;
	pcb = &x[17];
	ptmp = tmp;
	for (i = 0; i < 3; i++)
	{
		*ptmp++ = (*pca++) + (*pcb--);
		*ptmp++ = (*pca++) + (*pcb--);
		*ptmp++ = (*pca++) + (*pcb--);
	}
	
	Mp3FastSdct(tmp, &X[0]);
	
	/* odd input butterfly and scaling */
	pca = x;
	pcb = &x[17];
	ptmp = tmp;
	pscale = sdctII_scale;
	for (i = 0; i < 3; i++)
	{
		t = (*pca++) - (*pcb--);
		*ptmp++ = IMUL2(t, *pscale++);
		t = (*pca++) - (*pcb--);
		*ptmp++ = IMUL2(t, *pscale++);
		t = (*pca++) - (*pcb--);
		*ptmp++ = IMUL2(t, *pscale++);
	}
	
	Mp3FastSdct(tmp, &X[1]);
	
	/* output accumulation */
	ptmp = &X[3];
	t = X[1];
	for (i = 0; i < 2; i++)
	{
		*ptmp -= t;
		t = *ptmp;
		ptmp += 2;

		*ptmp -= t;
		t = *ptmp;
		ptmp += 2;

		*ptmp -= t;
		t = *ptmp;
		ptmp += 2;

		*ptmp -= t;
		t = *ptmp;
		ptmp += 2;
	}
}

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static inline void Mp3FastDctIV(MP3INT const y[18], MP3INT X[18])
{
	MP3INT tmp[18];
	const MP3INT *py;
	const short *pscale;
	MP3INT *px, *ptmp;
	int t;
	int i;
	
	/* scaling */
	ptmp = tmp;
	py = y;
	pscale = DctIV_scale;
	for (i = 0; i < 6; i++)
	{
		*ptmp++ = IMUL2(*py++, *pscale++);
		*ptmp++ = IMUL2(*py++, *pscale++);
		*ptmp++ = IMUL2(*py++, *pscale++);
	}
	
	/* SDCT-II */
	sdctII(tmp, X);
	
	/* DctIV_scale reduction and output accumulation */
	px = &X[1];
	t = X[0] >> 1;
	X[0] = t;
	for (i = 0; i < 4; i++)
	{
		t = (*px >> 1) - t;
		*px++ = t;
		t = (*px >> 1) - t;
		*px++ = t;
		t = (*px >> 1) - t;
		*px++ = t;
		t = (*px >> 1) - t;
		*px++ = t;
	}
	t = (*px >> 1) - t;
	*px++ = t;
}

/*
* NAME:	Mp3Imdct36
* DESCRIPTION:	perform X[18]->x[36] IMDCT using Szu-Wei Lee's fast algorithm
*/
////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static void Mp3Imdct36(MP3INT const x[18], MP3INT y[36])
{
	MP3INT tmp[18];
	MP3INT *py, *ptmp;
	int i;
	
	/* DCT-IV */
	
	Mp3FastDctIV(x, tmp);
	
	/* convert 18-point DCT-IV to 36-point IMDCT */
	py = y;
	ptmp = &tmp[9];
	for (i = 0; i < 3; i++)
	{
		*py++ = *ptmp++;
		*py++ = *ptmp++;
		*py++ = *ptmp++;
	}
	ptmp = &tmp[17];
	for (; i < 9; i++)
	{
		*py++ = -(*ptmp--);
		*py++ = -(*ptmp--);
		*py++ = -(*ptmp--);
	}
	ptmp = tmp;
	for (; i < 12; i++)
	{
		*py++ = -(*ptmp++);
		*py++ = -(*ptmp++);
		*py++ = -(*ptmp++);
	}
}


/*
* NAME:	III_imdct_l()
* DESCRIPTION:	perform IMDCT and windowing for long blocks
*/
////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3ImdctL(MP3INT *X, MP3INT *z, unsigned int block_type)
{
	unsigned int i;
	const short *pwindow;
	
	/* IMDCT */
	Mp3Imdct36(X, z);
	
	/* windowing */
	switch (block_type)
	{
	case 0:     /* normal window */
		pwindow = _LongWindowTable;
		for (i = 0; i < 9; i++)
		{
			*z = IMUL(*z, *pwindow++);
			z++;
			*z = IMUL(*z, *pwindow++);
			z++;
			*z = IMUL(*z, *pwindow++);
			z++;
			*z = IMUL(*z, *pwindow++);
			z++;
		}
		break;
		
	case 1:     /* start block */
		pwindow = _LongWindowTable;
		for (i = 0; i < 6; i++)
		{
			*z = IMUL(*z, *pwindow++);
			z++;
			*z = IMUL(*z, *pwindow++);
			z++;
			*z = IMUL(*z, *pwindow++);
			z++;
		}
		/*  (i = 18; i < 24; ++i) z[i] unchanged */
		z += 6;
		
		pwindow = &_ShortWindowTable[6];
		for (; i < 12; i++)
		{
			*z = IMUL(*z, *pwindow++);
			z++;
		}
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		break;
		
	case 3:     /* stop block */
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		*z++ = 0;
		pwindow = _ShortWindowTable;
		for(i = 0; i < 6; i++)
		{
			*z = IMUL(*z, *pwindow++);
			z++;
		}
		/*  (i = 12; i < 18; ++i) z[i] unchanged */
		z += 6;
		
		pwindow = &_LongWindowTable[18];
		for (; i < 12; i++)
		{
			*z = IMUL(*z, *pwindow++);
			z++;
			*z = IMUL(*z, *pwindow++);
			z++;
			*z = IMUL(*z, *pwindow++);
			z++;
		}
		break;
	}
}


// DESCRIPTION:	perform IMDCT and windowing for short blocks
////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3ImdctS(MP3INT *X, MP3INT *z)
{
	MP3INT y[36], *yptr, *px;
	const short *wptr;
	int w, i;
	int lo;
	
	/* IMDCT */
	yptr = &y[0];
	
	for (w = 0; w < 3; ++w)
	{
		const short *s;
		s = _ShortImdctTable;
		
		for (i = 0; i < 3; ++i)
		{
			px = X;
			lo = IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo = IMULSHIFT(lo);
			yptr[i + 0] = lo;
			yptr[5 - i] = -lo;
			
			px = X;
			lo = IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo += IMULNS(*px++, *s++);
			lo = IMULSHIFT(lo);
			
			yptr[ i + 6] = lo;
			yptr[11 - i] = lo;
		}
		
		yptr += 12;
		X += 6;
	}
	
	/* windowing, overlapping and concatenation */
	yptr = &y[0];
	wptr = &_ShortWindowTable[0];
	
	for (i = 0; i < 6; ++i)
	{
		z[i + 0] = 0;
		z[i + 6] = IMUL(yptr[ 0 + 0], wptr[0]);
		
		MACF(lo, yptr[ 0 + 6], wptr[6]);
		MACN(lo, yptr[12 + 0], wptr[0]);
		
		z[i + 12] = lo;
		
		MACF(lo, yptr[12 + 6], wptr[6]);
		MACN(lo, yptr[24 + 0], wptr[0]);
		
		z[i + 18] = lo;
		
		z[i + 24] = IMUL(yptr[24 + 6], wptr[6]);
		z[i + 30] = 0;
		
		++yptr;
		++wptr;
	}
}



/*
 * NAME:	III_overlap()
 * DESCRIPTION:	perform overlap-add of windowed IMDCT outputs
 */
////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3Overlap(MP3INT const *output, MP3INT *overlap, MP3INT *sample, int full)
{
	MP3INT *ptmp;
	int i = 6;
	if(full)
		full = 32;
	else
		full = 16;
	ptmp = (MP3INT*) output + 18;
	while(i--)
	{
		*sample = (*output++) + *overlap;
		sample += full;
		*overlap++ = *ptmp++;

		*sample = (*output++) + *overlap;
		sample += full;
		*overlap++ = *ptmp++;

		*sample = (*output++) + *overlap;
		sample += full;
		*overlap++ = *ptmp++;
	}
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static void Mp3OverlapZ(MP3INT *overlap, MP3INT *sample, int full)
{
	int i = 6;

	if(full)
		full = 32;
	else
		full = 16;
	while(i--)
	{
		*sample = *overlap;
		sample += full;
		*overlap++ = 0;		

		*sample = *overlap;
		sample += full;
		*overlap++ = 0;	
		
		*sample = *overlap;
		sample += full;
		*overlap++ = 0;		
	}
}

/*
 * NAME:	III_freqinver()
 * DESCRIPTION:	perform subband frequency inversion for odd sample lines
 */
////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3FreqInver(MP3INT *sample, int full)
{
	int i;

	if(full)
	{
		sample += 33;
		for(i=0; i<9; i++)
		{
			int j;
			for(j=0; j<4; j++)
			{
				*sample = -*sample;
				sample += 2;
				*sample = -*sample;
				sample += 2;
				*sample = -*sample;
				sample += 2;
				*sample = -*sample;
				sample += 2;
			}
			sample += 32;
		}
	}
	else
	{
		sample += 17;
		for(i=0; i<9; i++)
		{
			*sample = -*sample;
			sample += 2;
			*sample = -*sample;
			sample += 2;
			*sample = -*sample;
			sample += 2;
			*sample = -*sample;
			sample += 2;
			*sample = -*sample;
			sample += 2;
			*sample = -*sample;
			sample += 2;
			*sample = -*sample;
			sample += 2;
			*sample = -*sample;
			sample += 18;
		}
	}
}



////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3Imdct(MP3_STREAM *stream)
{
	short ch, nch, gr, ngr;
	MP3INT l, i;
	MP3INT output[36];
	int full;

	if(stream->DecodeLines == MP3_DECODE_LINE)
		full = 1;
	else
		full = 0;
	nch = (stream->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
	ngr = (stream->Header.Mpeg == MP3_MPEG1) ? 2 : 1;
	for(gr=0; gr<ngr; gr++)
	{
		for(ch=0; ch<nch; ch++)
		{
			MP3_CHSI *channel;
			MP3INT *freqline, *overlap;
			MP3INT *sample;

			freqline = stream->FreqLine[gr][ch];
			overlap = stream->Overlap[ch];
			if(gr == 1)
				sample = &stream->SbSample[ch][stream->DecodeLines];
			else
				sample = &stream->SbSample[ch][0];
			channel = &stream->SideInfo.Channel[gr][ch];

			/* subbands 0-1 */
			if (channel->BlockType != 2 || (channel->Flags & MP3_MIXED_BLOCK))
			{
				BYTE block_type;

				block_type = channel->BlockType;
				if (channel->Flags & MP3_MIXED_BLOCK)
					block_type = 0;

				/* long blocks */
				Mp3ImdctL(freqline, output, block_type);
				Mp3Overlap(output, overlap, sample++, full);
				Mp3ImdctL(freqline + 18, output, block_type);
				Mp3Overlap(output, overlap+18, sample++, full);
				l = 36;
			}
			else
			{
				/* short blocks */
				Mp3ImdctS(freqline, output);
				Mp3Overlap(output, overlap, sample++, full);
				Mp3ImdctS(freqline + 18, output);
				Mp3Overlap(output, overlap+18, sample++, full);
				l = 36;
			}
			i = stream->DecodeLines - 1;
			while((i>0) && (*(freqline+i) == 0))
				i--;
			i++;

			if (channel->BlockType != 2)
			{
				/* long blocks */
				while(l < i)
				{
					Mp3ImdctL(freqline+l, output, channel->BlockType);
					Mp3Overlap(output, overlap+l, sample++, full);
					l += 18;
				}
			}
			else
			{
				/* short blocks */
				while(l < i)
				{
					Mp3ImdctS(freqline+l, output);
					Mp3Overlap(output, overlap+l, sample++, full);
					l += 18;
				}
			}

			/* remaining (zero) subbands */
			while(l < stream->DecodeLines)
			{
				Mp3OverlapZ(overlap+l, sample++, full);
				l += 18;
			}

			if(gr == 1)
				sample = &stream->SbSample[ch][stream->DecodeLines];
			else
				sample = &stream->SbSample[ch][0];
			Mp3FreqInver(sample, full);
		}
	}
	return 0;
}


#endif // CONFIG_DECODE_MP3_ENABLE

