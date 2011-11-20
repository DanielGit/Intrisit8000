//[PROPERTY]===========================================[PROPERTY]
//			*****  诺亚神舟操作系统V2  *****
//		--------------------------------------
//				  重采样处理函数文件   
//		--------------------------------------
//				 版权: 新诺亚舟科技
//			 ---------------------------
//				  版   本   历   史
//		--------------------------------------
//  版本	日前		说明		
//  V0.1	20010-6	  Init.			 Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]


#include <kernel/kernel.h>
#include <direct/medialib/resample.h>
#include "ResampleTab.h"

#define IBUFFSIZE	1024						 /* Input buffer size */

#if defined(CONFIG_ARCH_XBURST) && !defined(WIN32)
extern void MxuEnable(void);
extern int MxuResampleFilterP1(short *Xp, short Ph);
extern int MxuResampleFilterN1(short *Xp, short Ph);
#endif

typedef struct _RESAMPLE_OBJ 
{
	int Chs;
	int iRate;
	int oRate;
	int iSamples;
	int oSamples;
	int FastMode;
	int Dt;
	
	int Time;
	int Nx;
	int Xp;
	int Ncreep;
	int Xoff;
	int Xread;

	int cSamples;
	short cBuf[2*IBUFFSIZE];

	short *iBuf;
	short iLeft[IBUFFSIZE];
	short iRight[IBUFFSIZE];
	short *oLeft;
	short *oRight;
}RESAMPLE_OBJ;
typedef RESAMPLE_OBJ *PRESAMPLE_OBJ;

#define Nhc       8
#define Na        7
#define Np       (Nhc+Na)
#define Npc      (1<<Nhc)
#define Amask    ((1<<Na)-1)
#define Pmask    ((1<<Np)-1)
#define Nh       16
#define Nb       16
#define Nhxn     14
#define Nhg      (Nh-Nhxn)
#define NLpScl   13


/* Description of constants:
 *
 * Npc - is the number of look-up values available for the lowpass filter
 *    between the beginning of its impulse response and the "cutoff time"
 *    of the filter.  The cutoff time is defined as the reciprocal of the
 *    lowpass-filter cut off frequence in Hz.  For example, if the
 *    lowpass filter were a sinc function, Npc would be the index of the
 *    impulse-response lookup-table corresponding to the first zero-
 *    crossing of the sinc function.  (The inverse first zero-crossing
 *    time of a sinc function equals its nominal cutoff frequency in Hz.)
 *    Npc must be a power of 2 due to the details of the current
 *    implementation. The default value of 512 is sufficiently high that
 *    using linear interpolation to fill in between the table entries
 *    gives approximately 16-bit accuracy in filter coefficients.
 *
 * Nhc - is log base 2 of Npc.
 *
 * Na - is the number of bits devoted to linear interpolation of the
 *    filter coefficients.
 *
 * Np - is Na + Nhc, the number of bits to the right of the binary point
 *    in the integer "time" variable. To the left of the point, it indexes
 *    the input array (X), and to the right, it is interpreted as a number
 *    between 0 and 1 sample of the input X.  Np must be less than 16 in
 *    this implementation.
 *
 * Nh - is the number of bits in the filter coefficients. The sum of Nh and
 *    the number of bits in the input data (typically 16) cannot exceed 32.
 *    Thus Nh should be 16.  The largest filter coefficient should nearly
 *    fill 16 bits (32767).
 *
 * Nb - is the number of bits in the input data. The sum of Nb and Nh cannot
 *    exceed 32.
 *
 * Nhxn - is the number of bits to right shift after multiplying each input
 *    sample times a filter coefficient. It can be as great as Nh and as
 *    small as 0. Nhxn = Nh-2 gives 2 guard bits in the multiply-add
 *    accumulation.  If Nhxn=0, the accumulation will soon overflow 32 bits.
 *
 * Nhg - is the number of guard bits in mpy-add accumulation (equal to Nh-Nhxn)
 *
 * NLpScl - is the number of bits allocated to the unity-gain normalization
 *    factor.  The output of the lowpass filter is multiplied by LpScl and
 *    then right-shifted NLpScl bits. To avoid overflow, we must have 
 *    Nb+Nhg+NLpScl < 32.
 */


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static int ResampleRead(PRESAMPLE_OBJ obj, int off)
{
	int i;
	int	rsamps, zsamps;
	short *lbuf, *rbuf;
	short *ibuf, *cbuf;
	short *mbuf;
	
	rsamps = IBUFFSIZE - off;   /* Calculate number of samples to get */
	ibuf = obj->iBuf;
	cbuf = obj->cBuf;
	mbuf = obj->cBuf;
	rbuf = obj->iRight + off;
	lbuf = obj->iLeft + off;

	// 检查是否结束
	if(!obj->iSamples && !obj->cSamples)
		return 0;

	// 检查cbuf中和ibuf中的数据是否足够
	if(rsamps > (obj->cSamples + obj->iSamples))
	{
		// 检查是否是最后一帧
		if(!obj->iSamples)
		{
			zsamps = rsamps - obj->cSamples;
			
			if(obj->Chs == 1)
			{
				for(i=0; i<obj->cSamples; i++)
					*lbuf++ = *cbuf++;
				for(i=0; i<zsamps; i++)
					*lbuf++ = 0;
			}
			else
			{
				for(i=0; i<obj->cSamples; i++)
				{
					*lbuf++ = *cbuf++;
					*rbuf++ = *cbuf++;
				}
				for(i=0; i<zsamps; i++)
				{
					*lbuf++ = 0;
					*rbuf++ = 0;
				}
			}
			obj->cSamples = 0;
			return 1;
		}
		else
		{
			if(obj->Chs == 1)
			{
				cbuf += obj->cSamples;
				for(i=0; i<obj->iSamples; i++)
					*cbuf++ = *ibuf++;
			}
			else
			{
				cbuf += obj->cSamples * 2;
				for(i=0; i<obj->iSamples; i++)
				{
					*cbuf++ = *ibuf++;
					*cbuf++ = *ibuf++;
				}
			}
			obj->cSamples += obj->iSamples;
			obj->iSamples = 0;
			obj->iBuf = ibuf;
			return 0;
		}	
	}

	// 复制cBuf中的数据到iLeft, iRight
	if(obj->cSamples)
	{
		int rtemp;

		if(rsamps >= obj->cSamples)
		{
			rtemp = obj->cSamples;
			rsamps -= obj->cSamples;
			obj->cSamples = 0;
		}
		else
		{
			rtemp = rsamps;
			obj->cSamples -= rsamps;
			rsamps = 0;
		}	

		if(obj->Chs == 1)
		{
			for(i=0; i<rtemp; i++)
				*lbuf++ = *cbuf++;
			for(i=0; i<obj->cSamples; i++)
				*mbuf++ = *cbuf++;
		}
		else
		{
			for(i=0; i<rtemp; i++)
			{
				*lbuf++ = *cbuf++;
				*rbuf++ = *cbuf++;
			}
			for(i=0; i<obj->cSamples; i++)
			{
				*mbuf++ = *cbuf++;
				*mbuf++ = *cbuf++;
			}
		}
	}

	// 复制ibuf中的数据到iLeft, iRight
	if(rsamps)
	{
		if(obj->Chs == 1)
		{
			for(i=0; i<rsamps; i++)
				*lbuf++ = *ibuf++;
		}
		else
		{
			for(i=0; i<rsamps; i++)
			{
				*lbuf++ = *ibuf++;
				*rbuf++ = *ibuf++;
			}
		}
		obj->iSamples -= rsamps;
	}
	obj->iBuf = ibuf;
	return 1;
}



////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static inline short ResampleConvert(int v, int scl)
{
	v >>= scl;
	if(v > 32767)
		v = 32767;
	else if(v < -32768)
		v = -32768;
	return (short) v;
}


#if !defined(CONFIG_ARCH_XBURST) || defined(WIN32)
////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static int ResampleFilter(short *Xp, short Ph, short Inc)
{
	const short *hp;
	short a = 0;
	int v, t, i;
	int start;
	
	start = Ph >> Na;
	a = (short)(Ph & Amask);
	v = 0;
	if(Inc == 1)
	{
		if(Ph == 0)	
			start = 256;
		else if(start == 255)
			start = 257;
	}
	hp = &FilterImpTab[start << 4];
	if(a)
	{
		a <<= 2;
		for(i=0; i<6; i++)
		{
			t = *hp++;				/* Get filter coeff */
			t += ((*hp++)*a) >> 16;	/* t is now interp'd filter coeff */
			v += t * (*Xp);						/* The filter output */
			Xp += Inc;					/* Input signal step. NO CHECK ON BOUNDS */
		} 
	}
	else 
	{
		for(i=0; i<6; i++)
		{
			v += (*hp) * (*Xp);		/* Mult coeff by input sample */
			hp += 2;	/* Filter coeff step */
			Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
		}
	}
	return v;
}
#endif

////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static int ResampleFine(PRESAMPLE_OBJ obj, int *Time, int Nx, int right)
{
	short *X, *Y;
	short *Xp, *Ystart;
	int v;
	int endTime;			  /* When Time reaches EndTime, return to user */
	
	if(right)
	{
		X = obj->iRight;
		Y = obj->oRight;
	}
	else
	{
		X = obj->iLeft;
		Y = obj->oLeft;
	}
	Ystart = Y;
	endTime = *Time + (1<<Np) * Nx;
	while (*Time < endTime)
	{
		Xp = &X[*Time>>Np];	  /* Ptr to current input sample */
		/* Perform left-wing inner product */
#if defined(CONFIG_ARCH_XBURST) && !defined(WIN32)
		v = MxuResampleFilterN1(Xp, (short)(*Time&Pmask));
		/* Perform right-wing inner product */
		/* previous (triggers warning): (short)((-*Time)&Pmask),1); */
		v += MxuResampleFilterP1(Xp, (short)((((*Time)^Pmask)+1)&Pmask));
#else
		v = ResampleFilter(Xp, (short)(*Time&Pmask), -1);
		/* Perform right-wing inner product */
		/* previous (triggers warning): (short)((-*Time)&Pmask),1); */
		v += ResampleFilter(Xp+1, (short)((((*Time)^Pmask)+1)&Pmask), 1);
#endif
		v >>= 16;			  /* Make guard bits */
		v *= FILTER_SCALE;			 /* Normalize for unity filter gain */
		*Y++ = ResampleConvert(v, NLpScl);   /* strip guard bits, deposit output */
		*Time += obj->Dt;		   /* Move to next sample by time increment */
	}
	return (Y - Ystart);		/* Return the number of output samples */
}




////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
static int ResampleFast(PRESAMPLE_OBJ obj, int *Time, int Nx, int right)
{
	short iconst;
	short *X, *Y;
	short *Xp, *Ystart;
	int v,x1,x2;
	int endTime;			  /* When Time reaches EndTime, return to user */
	
	if(right)
	{
		X = obj->iRight;
		Y = obj->oRight;
	}
	else
	{
		X = obj->iLeft;
		Y = obj->oLeft;
	}
	Ystart = Y;
	endTime = *Time + (1 << Np) * Nx;
	while (*Time < endTime)
	{
		iconst = (short)((*Time) & Pmask);
		Xp = &X[(*Time) >> Np];	  /* Ptr to current input sample */
		x1 = *Xp++;
		x2 = *Xp;
		x1 *= ((1 << Np) - iconst);
		x2 *= iconst;
		v = x1 + x2;
		*Y++ = ResampleConvert(v, Np);   /* Deposit output */
		*Time += obj->Dt;			   /* Move to next sample by time increment */
	}
	return (Y - Ystart);			/* Return number of output samples */
}


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int Resample(void *handle, PAUDIO_FILTER filter)
{
	int time2;		  /* Current time/pos in input sample */
	int Nout;
	int i, ycnt;
	short *lbuf, *rbuf;
	int osamples;
	PRESAMPLE_OBJ obj = (PRESAMPLE_OBJ)handle;
	int time = obj->Time;  
	int nx = obj->Nx;    
	int xp = obj->Xp;    
	int ncreep = obj->Ncreep;
	int xoff = obj->Xoff;  
	int xread = obj->Xread; 

	lbuf = filter->oBuf;
	rbuf = filter->oBuf + 1;
	obj->iBuf = filter->iBuf;
	if(obj->Chs == 1)
	{	
		obj->iSamples = filter->iSize / sizeof(short);
		osamples = filter->oSize / sizeof(short);
	}
	else
	{
		obj->iSamples = filter->iSize / (sizeof(short) * 2);
		osamples = filter->oSize / (sizeof(short) * 2);
	}
	
#if defined(CONFIG_ARCH_XBURST) && !defined(WIN32)
	MxuEnable();
#endif
	
	ycnt = 0;
	while(ResampleRead(obj, (int)xread) && (ycnt < osamples))
	{
		/* Resample stuff in input buffer */
    	time2 = time;
		if(obj->FastMode)
		{
			Nout = ResampleFast(obj, &time, nx, 0);
			if(obj->Chs == 2)
				Nout=ResampleFast(obj, &time2, nx, 1);
		}
		else
		{
			Nout = ResampleFine(obj, &time, nx, 0);
			if(obj->Chs == 2)
				Nout=ResampleFine(obj, &time2, nx, 1);
		}	
								
		time -= (nx << Np);				/* Move converter Nx samples back in time */
		xp += nx;						/* Advance by number of samples processed */
		ncreep = (time >> Np) - xoff;	/* Calc time accumulation in Time */
		if(ncreep) 
		{
			time -= (ncreep << Np);		/* Remove time accumulation */
			xp += ncreep;				/* and add it to read pointer */
		}
		for(i=0; i<IBUFFSIZE-xp+xoff; i++)
		{ 
			/* Copy part of input signal */
			obj->iLeft[i] = obj->iLeft[i+xp-xoff]; /* that must be re-used */
			if(obj->Chs == 2)
				obj->iRight[i] = obj->iRight[i+xp-xoff]; /* that must be re-used */
		}
		xread = i;				/* Pos in input buff to read new data into */
		xp = xoff;
		
		ycnt += Nout;
		if(ycnt > osamples)
		{
			Nout -= (ycnt-osamples);
			ycnt = osamples;
			kprintf("RESAMPLE: Output array overflow0!\n");
		}
		
		/* Check to see if output buff overflowed */
		if(Nout > obj->oSamples)	
		{	
			kprintf("RESAMPLE: Output array overflow1!\n");
			return -1;
		}
		
		if(obj->Chs==1) 
		{
			for(i = 0; i < Nout; i++)
				*lbuf++ = obj->oLeft[i];
		} 
		else
		{
			for(i = 0; i < Nout; i++) 
			{
				lbuf[2*i] = obj->oLeft[i];
				rbuf[2*i] = obj->oRight[i];
			}
			lbuf += 2*Nout;
			rbuf += 2*Nout;
		}
	};
	
	obj->Time = time;  
	obj->Nx = nx;    
	obj->Xp = xp;    
	obj->Ncreep = ncreep;
	obj->Xoff = xoff;  
	obj->Xread = xread; 
	filter->oSize = ycnt * sizeof(short) * obj->Chs;
	return filter->oSize;					/* Return # of samples in output file */
}


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void *ResampleCreate(int chs, int irate, int orate, int mode)
{
	PRESAMPLE_OBJ obj;
	int osize;
	
	// 检查输入参数是否正确
	if((chs != 1) && (chs != 2))
		return NULL;
	if((irate != 8000) && (irate != 11025) 
		&& (irate != 16000) && (irate != 22050) 
		&& (irate != 32000) && (irate != 44100) 
		&& (irate != 24000) && (irate != 48000))
		return NULL;
	if((orate != 8000) && (orate != 11025) 
		&& (orate != 16000) && (orate != 22050) 
		&& (orate != 32000) && (orate != 44100) 
		&& (orate != 24000) && (orate != 48000))
		return NULL;
	if(orate < irate)
		return NULL;
	
	// 申请对象
	obj = kmalloc(sizeof(RESAMPLE_OBJ));
	if(obj == NULL)
		return NULL;
	kmemset(obj, 0x00, sizeof(RESAMPLE_OBJ));
	osize = (orate + (irate >> 1)) * (IBUFFSIZE * sizeof(short)) / irate;
	obj->oSamples = osize / sizeof(short);
	obj->oLeft = kmalloc(osize * chs);
	if(obj->oLeft == NULL)
	{
		kfree(obj);
		return NULL;
	}
	obj->oRight = obj->oLeft + obj->oSamples;
	obj->Chs = chs;
	obj->iRate = irate;
	obj->oRate = orate;
//	if(irate >= 32000)
//		obj->FastMode = 1;
//	else
		obj->FastMode = mode;
	obj->Dt = (irate * (1 << Np) + (orate / 2)) / orate;
	
	/* Check input buffer size */
	if(obj->FastMode)
		obj->Xoff = 10;
	else
		obj->Xoff = ((FILTER_NMULT+1) / 2) + 10;
	
	obj->Nx = IBUFFSIZE - 2 * obj->Xoff;
	obj->Xp = obj->Xoff;
	obj->Xread = obj->Xoff;	
	obj->Time = (obj->Xoff << Np);
	return obj;
}



////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void ResampleDestroy(void *handle)
{
	PRESAMPLE_OBJ obj = (PRESAMPLE_OBJ)handle;
	if(obj)
	{
		if(obj->oLeft)
			kfree(obj->oLeft);
		kfree(obj);
	}
}


