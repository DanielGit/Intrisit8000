//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－合成多相滤波器
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

#define SYN_CONV(x)		((short)x)


#ifdef USE_MAC
extern const int Mp3SynthHalfDW[];
#else
extern const MP3INT Mp3SynthHalfDW[];
#endif

#define COS3201		CONST_SHORT(0x0fec46d2)
#define COS3202		CONST_SHORT(0x0fb14be8)
#define COS3203		CONST_SHORT(0x0f4fa0ab)
#define COS3204		CONST_SHORT(0x0ec835e8)
#define COS3205		CONST_SHORT(0x0e1c5979)
#define COS3206		CONST_SHORT(0x0d4db315)
#define COS3207		CONST_SHORT(0x0c5e4036)
#define COS3208		CONST_SHORT(0x0b504f33)
#define COS3209		CONST_SHORT(0x0a267993)
#define COS3210		CONST_SHORT(0x08e39d9d)
#define COS3211		CONST_SHORT(0x078ad74e)
#define COS3212		CONST_SHORT(0x061f78aa)
#define COS3213		CONST_SHORT(0x04a5018c)
#define COS3214		CONST_SHORT(0x031f1708)
#define COS3215		CONST_SHORT(0x01917a6c)

#define COS0401		COS3208
#define COS0801		COS3204
#define COS0805		(-COS3212)
#define COS1601		COS3202
#define COS1605		COS3210
#define COS1609		(-COS3214)
#define COS1613		(-COS3206)
#define COS3217		(-COS3215)
#define COS3221		(-COS3211)
#define COS3225		(-COS3207)
#define COS3229		(-COS3203)


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static void Mp3SynthDct16(MP3INT *s, int *x)
{
	int t0, t1, t2, t3, t4, t5, t6, t7;
	int o0, o1, o2, o3, o4, o5, o6, o7;
	int e0, e1, e2, e3, e4, e5, e6, e7;

	t0 = s[0] + s[15];
	o0 = s[0] - s[15];
	o0 = IMUL(o0, COS3201);
	t1 = s[2] + s[13];
	o1 = s[2] - s[13];
	o1 = IMUL(o1, COS3205);
	t2 = s[4] + s[11];
	o2 = s[4] - s[11];
	o2 = IMUL(o2, COS3209);
	t3 = s[6] + s[9];
	o3 = s[6] - s[9];
	o3 = IMUL(o3, COS3213);
	t4 = s[8] + s[7];
	o4 = s[8] - s[7];
	o4 = IMUL(o4, COS3217);
	t5 = s[10] + s[5];
	o5 = s[10] - s[5];
	o5 = IMUL(o5, COS3221);
	t6 = s[12] + s[3];
	o6 = s[12] - s[3];
	o6 = IMUL(o6, COS3225);
	t7 = s[14] + s[1];
	o7 = s[14] - s[1];
	o7 = IMUL(o7, COS3229);

	// 偶数部分8点DCT
	e0 = t0 + t4;
	e4 = t0 - t4;
	e4 = IMUL(e4, COS1601);
	e1 = t1 + t5;
	e5 = t1 - t5;
	e5 = IMUL(e5, COS1605);
	e2 = t2 + t6;
	e6 = t2 - t6;
	e6 = IMUL(e6, COS1609);
	e3 = t3 + t7;
	e7 = t3 - t7;
	e7 = IMUL(e7, COS1613);

	t0 = e0 + e2;
	t2 = e0 - e2;
	t2 = IMUL(t2, COS0801);
	t1 = e1 + e3;
	t3 = e1 - e3;
	t3 = IMUL(t3, COS0805);
	t4 = e4 + e6;
	t6 = e4 - e6;
	t6 = IMUL(t6, COS0801);
	t5 = e5 + e7;
	t7 = e5 - e7;
	t7 = IMUL(t7, COS0805);

	e0 = t0 + t1;
	e1 = t0 - t1;
	e1 = IMUL(e1, COS0401);
	e2 = t2 + t3;
	e3 = t2 - t3;
	e3 = IMUL(e3, COS0401);
	e4 = t4 + t5;
	e5 = t4 - t5;
	e5 = IMUL(e5, COS0401);
	e6 = t6 + t7;
	e7 = t6 - t7;
	e7 = IMUL(e7, COS0401);

	e3 = (e3 << 1) - e2;
	e7 = (e7 << 1) - e6;
	e6 = (e6 << 1) - e4;
	e5 = (e5 << 1) - e6;
	e7 = (e7 << 1) - e5;

	// 奇数部分8点DCT
	t0 = o0 + o4;
	t4 = o0 - o4;
	t4 = IMUL(t4, COS1601);
	t1 = o1 + o5;
	t5 = o1 - o5;
	t5 = IMUL(t5, COS1605);
	t2 = o2 + o6;
	t6 = o2 - o6;
	t6 = IMUL(t6, COS1609);
	t3 = o3 + o7;
	t7 = o3 - o7;
	t7 = IMUL(t7, COS1613);

	o0 = t0 + t2;
	o2 = t0 - t2;
	o2 = IMUL(o2, COS0801);
	o1 = t1 + t3;
	o3 = t1 - t3;
	o3 = IMUL(o3, COS0805);
	o4 = t4 + t6;
	o6 = t4 - t6;
	o6 = IMUL(o6, COS0801);
	o5 = t5 + t7;
	o7 = t5 - t7;
	o7 = IMUL(o7, COS0805);

	t0 = o0 + o1;
	t1 = o0 - o1;
	t1 = IMUL(t1, COS0401);
	t2 = o2 + o3;
	t3 = o2 - o3;
	t3 = IMUL(t3, COS0401);
	t4 = o4 + o5;
	t5 = o4 - o5;
	t5 = IMUL(t5, COS0401);
	t6 = o6 + o7;
	t7 = o6 - o7;
	t7 = IMUL(t7, COS0401);

	t3 = (t3 << 1) - t2;
	t7 = (t7 << 1) - t6;
	t6 = (t6 << 1) - t4;
	t5 = (t5 << 1) - t6;
	t7 = (t7 << 1) - t5;

	t4 = (t4 << 1) - t0;
	t2 = (t2 << 1) - t4;
	t6 = (t6 << 1) - t2;
	t1 = (t1 << 1) - t6;
	t5 = (t5 << 1) - t1;
	t3 = (t3 << 1) - t5;
	t7 = (t7 << 1) - t3;

	x[8*0] = e0;
	x[8*8] = e1;
	x[8*4] = e2;
	x[8*12] = e3;
	x[8*2] = e4;
	x[8*10] = e5;
	x[8*6] = e6;
	x[8*14] = e7;
	
	x[8*1] = t0;
	x[8*9] = t1;
	x[8*5] = t2;
	x[8*13] = t3;
	x[8*3] = t4;
	x[8*11] = t5;
	x[8*7] = t6;
	x[8*15] = t7;
}


#ifdef USE_MAC
#define SYNTHEVEN_MACFN()					\
{											\
	pw += 8;								\
	MacInit(0, 0);							\
	pcmdata = MacRepeat8(8, px, (int*)pw);	\
	pcmdata = IMUL2(pcmdata, vol);			\
	if(pcmdata > 0x7fff)					\
		pcmdata = 0x7fff;					\
	else if(pcmdata < -0x7fff)				\
		pcmdata = -0x7fff;					\
	pw += 8;								\
}

#define SYNTHEVEN_MACF()					\
{											\
	pw += 8;								\
	MacInit(0, 0);							\
	MacRepeat8(8, px, (int*)pw);			\
	pw += 8;								\
}

#define SYNTHEVEN_MACN()					\
{											\
	pw += 8;								\
	pcmdata = MacRepeat8(8, px, (int*)pw);	\
	pcmdata = IMUL2(pcmdata, vol);			\
	if(pcmdata > 0x7fff)					\
		pcmdata = 0x7fff;					\
	else if(pcmdata < -0x7fff)				\
		pcmdata = -0x7fff;					\
	pw += 8;								\
}

#define SYNTHODD_MACFN()					\
{											\
	pw += 9;								\
	MacInit(0, 0);							\
	pcmdata = MacRepeat8(8, px, (int*)pw);	\
	pcmdata = IMUL2(pcmdata, vol);			\
	if(pcmdata > 0x7fff)					\
		pcmdata = 0x7fff;					\
	else if(pcmdata < -0x7fff)				\
		pcmdata = -0x7fff;					\
	pw += 8;								\
}

#define SYNTHODD_MACF()						\
{											\
	pw += 9;								\
	MacInit(0, 0);							\
	MacRepeat8(8, px, (int*)pw);			\
	pw += 8;								\
}

#define SYNTHODD_MACN()						\
{											\
	pw += 7;								\
	pcmdata = MacRepeat8(8, px, (int*)pw);	\
	pcmdata = IMUL2(pcmdata, vol);			\
	if(pcmdata > 0x7fff)					\
		pcmdata = 0x7fff;					\
	else if(pcmdata < -0x7fff)				\
		pcmdata = -0x7fff;					\
	pw += 8;								\
}

#else
#define SYN_SHIFT(x)			((MP3INT)((x)>>13))

#define SYNTHEVEN_MACF()				\
{										\
	pw += 8;							\
	pcmdata = IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
}

#define SYNTHEVEN_MACFN()				\
{										\
	SYNTHEVEN_MACF()					\
	pcmdata = SYN_SHIFT(pcmdata);		\
	pcmdata = IMUL2(pcmdata, vol);		\
	if(pcmdata > 0x7fff)				\
		pcmdata = 0x7fff;				\
	else if(pcmdata < -0x7fff)			\
		pcmdata = -0x7fff;				\
}

#define SYNTHEVEN_MACN()				\
{										\
	pw += 8;							\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata = SYN_SHIFT(pcmdata);		\
	pcmdata = IMUL2(pcmdata, vol);		\
	if(pcmdata > 0x7fff)				\
		pcmdata = 0x7fff;				\
	else if(pcmdata < -0x7fff)			\
		pcmdata = -0x7fff;				\
}


#define SYNTHODD_MACF()					\
{										\
	pw += 9;							\
	pcmdata = IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
}

#define SYNTHODD_MACFN()				\
{										\
	SYNTHODD_MACF()						\
	pcmdata = SYN_SHIFT(pcmdata);		\
	pcmdata = IMUL2(pcmdata, vol);		\
	if(pcmdata > 0x7fff)				\
		pcmdata = 0x7fff;				\
	else if(pcmdata < -0x7fff)			\
		pcmdata = -0x7fff;				\
}

#define SYNTHODD_MACN()					\
{										\
	pw += 7;							\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata += IMUL2NS(*px++, *pw++);	\
	pcmdata = SYN_SHIFT(pcmdata);		\
	pcmdata = IMUL2(pcmdata, vol);		\
	if(pcmdata > 0x7fff)				\
		pcmdata = 0x7fff;				\
	else if(pcmdata < -0x7fff)			\
		pcmdata = -0x7fff;				\
}
#endif

#ifndef SYNTH_USE_ASM

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static void Mp3SynthEvenSlot(int *x, short *pcm, int slot, int vol)
{
#if defined(CONFIG_MCU_C33L27) && !defined(WIN32)
	int pw;
#else
	int *pw;
#endif
	int *px;
	int pcmdata;
	
	// PCM[0]
#if defined(CONFIG_MCU_C33L27) && !defined(WIN32)
	pw = DST_RAM_ADDRESS/4 - (slot >> 1);
#else
	pw = (int*)Mp3SynthHalfDW - (slot >> 1);
#endif
	px = &x[64];
	SYNTHEVEN_MACF();
	px = &x[192];
	SYNTHEVEN_MACN();
	pcm[0] = SYN_CONV(pcmdata);

	// PCM[1]
	px = &x[72];
	SYNTHEVEN_MACF();
	px = &x[184];
	SYNTHEVEN_MACN();
	pcm[1] = SYN_CONV(pcmdata);

	// PCM[2]
	px = &x[80];
	SYNTHEVEN_MACF();
	px = &x[176];
	SYNTHEVEN_MACN();
	pcm[2] = SYN_CONV(pcmdata);

	// PCM[3]
	px = &x[88];
	SYNTHEVEN_MACF();
	px = &x[168];
	SYNTHEVEN_MACN();
	pcm[3] = SYN_CONV(pcmdata);

	// PCM[4]
	px = &x[96];
	SYNTHEVEN_MACF();
	px = &x[160];
	SYNTHEVEN_MACN();
	pcm[4] = SYN_CONV(pcmdata);

	// PCM[5]
	px = &x[104];
	SYNTHEVEN_MACF();
	px = &x[152];
	SYNTHEVEN_MACN();
	pcm[5] = SYN_CONV(pcmdata);

	// PCM[6]
	px = &x[112];
	SYNTHEVEN_MACF();
	px = &x[144];
	SYNTHEVEN_MACN();
	pcm[6] = SYN_CONV(pcmdata);

	// PCM[7]
	px = &x[120];
	SYNTHEVEN_MACF();
	px = &x[136];
	SYNTHEVEN_MACN();
	pcm[7] = SYN_CONV(pcmdata);

	// PCM[8]
	pw += 16;
	px = &x[128];
	SYNTHEVEN_MACFN();
	pcm[8] = SYN_CONV(pcmdata);

	// PCM[9]
	px = &x[120];
	SYNTHEVEN_MACF();
	px = &x[136];
	SYNTHEVEN_MACN();
	pcm[9] = SYN_CONV(pcmdata);

	// PCM[10]
	px = &x[112];
	SYNTHEVEN_MACF();
	px = &x[144];
	SYNTHEVEN_MACN();
	pcm[10] = SYN_CONV(pcmdata);

	// PCM[11]
	px = &x[104];
	SYNTHEVEN_MACF();
	px = &x[152];
	SYNTHEVEN_MACN();
	pcm[11] = SYN_CONV(pcmdata);

	// PCM[12]
	px = &x[96];
	SYNTHEVEN_MACF();
	px = &x[160];
	SYNTHEVEN_MACN();
	pcm[12] = SYN_CONV(pcmdata);

	// PCM[13]
	px = &x[88];
	SYNTHEVEN_MACF();
	px = &x[168];
	SYNTHEVEN_MACN();
	pcm[13] = SYN_CONV(pcmdata);

	// PCM[14]
	px = &x[80];
	SYNTHEVEN_MACF();
	px = &x[176];
	SYNTHEVEN_MACN();
	pcm[14] = SYN_CONV(pcmdata);

	// PCM[15]
	px = &x[72];
	SYNTHEVEN_MACF();
	px = &x[184];
	SYNTHEVEN_MACN();
	pcm[15] = SYN_CONV(pcmdata);
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static void Mp3SynthOddSlot(int *x, short *pcm, int slot, int vol)
{
#if defined(CONFIG_MCU_C33L27) && !defined(WIN32)
	int pw;
#else
	int *pw;
#endif
	int *px;
	int pcmdata;

	// PCM[0]
#if defined(CONFIG_MCU_C33L27) && !defined(WIN32)
	pw = DST_RAM_ADDRESS/4 - 1 - (slot >> 1);
#else
	pw = (int*)Mp3SynthHalfDW - 1 - (slot >> 1);
#endif
	px = &x[192];
	SYNTHODD_MACF();
	px = &x[64];
	SYNTHODD_MACN();
	pcm[0] = SYN_CONV(pcmdata);

	// PCM[1]
	px = &x[200];
	SYNTHODD_MACF();
	px = &x[56];
	SYNTHODD_MACN();
	pcm[1] = SYN_CONV(pcmdata);

	// PCM[2]
	px = &x[208];
	SYNTHODD_MACF();
	px = &x[48];
	SYNTHODD_MACN();
	pcm[2] = SYN_CONV(pcmdata);

	// PCM[3]
	px = &x[216];
	SYNTHODD_MACF();
	px = &x[40];
	SYNTHODD_MACN();
	pcm[3] = SYN_CONV(pcmdata);

	// PCM[4]
	px = &x[224];
	SYNTHODD_MACF();
	px = &x[32];
	SYNTHODD_MACN();
	pcm[4] = SYN_CONV(pcmdata);

	// PCM[5]
	px = &x[232];
	SYNTHODD_MACF();
	px = &x[24];
	SYNTHODD_MACN();
	pcm[5] = SYN_CONV(pcmdata);

	// PCM[6]
	px = &x[240];
	SYNTHODD_MACF();
	px = &x[16];
	SYNTHODD_MACN();
	pcm[6] = SYN_CONV(pcmdata);

	// PCM[7]
	px = &x[248];
	SYNTHODD_MACF();
	px = &x[8];
	SYNTHODD_MACN();
	pcm[7] = SYN_CONV(pcmdata);

	// PCM[8]
	pw += 24 - 9;
	px = &x[0];
	SYNTHODD_MACFN();
	pcm[8] = SYN_CONV(pcmdata);

	// PCM[9]
	px = &x[248];
	SYNTHODD_MACF();
	px = &x[8];
	SYNTHODD_MACN();
	pcm[9] = SYN_CONV(pcmdata);

	// PCM[10]
	px = &x[240];
	SYNTHODD_MACF();
	px = &x[16];
	SYNTHODD_MACN();
	pcm[10] = SYN_CONV(pcmdata);
	
	// PCM[11]
	px = &x[232];
	SYNTHODD_MACF();
	px = &x[24];
	SYNTHODD_MACN();
	pcm[11] = SYN_CONV(pcmdata);

	// PCM[12]
	px = &x[224];
	SYNTHODD_MACF();
	px = &x[32];
	SYNTHODD_MACN();
	pcm[12] = SYN_CONV(pcmdata);

	// PCM[13]
	px = &x[216];
	SYNTHODD_MACF();
	px = &x[40];
	SYNTHODD_MACN();
	pcm[13] = SYN_CONV(pcmdata);

	// PCM[14]
	px = &x[208];
	SYNTHODD_MACF();
	px = &x[48];
	SYNTHODD_MACN();
	pcm[14] = SYN_CONV(pcmdata);

	// PCM[15]
	px = &x[200];
	SYNTHODD_MACF();
	px = &x[56];
	SYNTHODD_MACN();
	pcm[15] = SYN_CONV(pcmdata);
}

#endif

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3SynthHalfInit(MP3_STREAM *stream) 
{
	int i;
#if defined(CONFIG_MCU_C33L27) && !defined(WIN32)
	int *pdst, *psrc;
#endif	

#if defined(CONFIG_MCU_C33L27) && !defined(WIN32)

	// 合成多项滤波X向量初始化
	pdst = (int *) INNER_RAM_ADDRESS;
	for(i=0; i<0x200; i++)
		*pdst++ = 0;
		
	// 加载D窗系数到
	pdst = (int *) DST_RAM_ADDRESS;
	psrc = (int *) Mp3SynthHalfDW;
	for(i=0; i<0x200; i++)
		*pdst++ = *psrc++;

	// 打开MAC单元
	*(volatile BYTE *)0x300020 = 0x96;
	*(volatile BYTE *)0x30001a |= 0x01;
#else
	for(i=0; i<256; i++)
	{
		stream->Mp3SynthVX[0][i] = 0;
		stream->Mp3SynthVX[1][i] = 0;
	}
#endif
	stream->Synth.XSlot[0] = 0;
	stream->Synth.XSlot[1] = 0;

	return 0;
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3SynthHalf(MP3_STREAM *stream) 
{
	short ch, nch, s, ns;
	MP3INT *sbsample;
	short *pcm;
	int *x;
	int vol;
	short slot, start;

	nch = (stream->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
	ns = (stream->Header.Mpeg == MP3_MPEG1) ? 36 : 18;
	vol = 0xfff;
	for (ch = 0; ch < nch; ++ch)
	{
		sbsample = stream->SbSample[ch];
		if((stream->Header.Samplerate  == 8000) || (stream->Header.Samplerate  == 11025))
			pcm = &stream->Synth.Pcm[ch][ns << 4];
		else
			pcm = stream->Synth.Pcm[ch];
		slot = stream->Synth.XSlot[ch];
#if defined(CONFIG_MCU_C33L27) && !defined(WIN32)
		if(ch)
			x = (int*)INNER_RAM_ADDRESS;
		else
			x = (int*)(INNER_RAM_ADDRESS+0x400);
#else
		x = stream->Mp3SynthVX[ch];
#endif		
		
		for (s = 0; s < ns; ++s)
		{
			if(slot & 0x01)
			{
				start = 128 + (slot >> 1);
				Mp3SynthDct16(sbsample, &x[start]);
				Mp3SynthOddSlot(x, pcm, slot, vol);
			}
			else
			{
				start = slot >> 1;
				Mp3SynthDct16(sbsample, &x[start]);
				Mp3SynthEvenSlot(x, pcm, slot, vol);
			}
			if(slot == 0)
				slot = 15;
			else
				slot--;
			pcm += 16;
			sbsample += 16;
		}
		stream->Synth.XSlot[ch] = slot;
	}

	if((stream->Header.Samplerate  == 8000) || (stream->Header.Samplerate  == 11025))
	{
		for (ch = 0; ch < nch; ++ch)
		{
			short *dst;
			short *src;
			int samps;
			int y0, y1;

			samps = ns << 4;
			dst = stream->Synth.Pcm[ch];
			src = dst + samps;
			y0 = stream->Synth.PrePcm[ch];
			y1 = y0;
			while(samps--)
			{
				y1 = *src++;
				*dst++ = y0;
				*dst++ = (short)(y0 + (y1 - y0) / 2);
				y0 = y1;
			}
			stream->Synth.PrePcm[ch] = y1;
		}
		stream->Synth.Channels = nch;
		stream->Synth.SampleRate = stream->Header.Samplerate;
		stream->Synth.Samples = ns << 5;
	}
	else
	{
		stream->Synth.Channels = nch;
		stream->Synth.SampleRate = stream->Header.Samplerate >> 1;
		stream->Synth.Samples = ns << 4;
	}
	return 0;
}

#ifdef SYNTH_USE_ASM

static void Mp3SynthEvenSlot(int *x, short *pcm, int slot, int vol)
{
	// %r0		MacInit
	// %r1		MacRepeat8
	// %r2		x
	// %r3		pcm
	
	// %r4
	
	// %r6		8
	// %r7		px
	// %r8		pw
	// %r9		vol
	
	asm("pushn	%r3");
	asm("push	%r10");
	asm("push	%r11");
	asm("xld.w	%r0, MacInit");
	asm("xld.w	%r1, MacRepeat8");
	asm("ld.w	%r2, %r6");
	asm("ld.w	%r3, %r7");
	asm("xld.w	%r10, 0x7fff");
	asm("xld.w	%r11, -0x7fff");
	
	// pw = DST_RAM_ADDRESS/4 - (slot >> 1);
	asm("sra	%r8, 1");
	asm("xld.w	%r4, 0x21000");
	asm("sub	%r4, %r8");
	asm("ld.w	%r8, %r4");
//	asm("popn	%r3");
//	asm("ret");
	
	// PCM[0]
//	px = &x[64];
//	SYNTHEVEN_MACF();
//	px = &x[192];
//	SYNTHEVEN_MACN();
//	pcm[0] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 64*4");
	asm("add	%r8, 8");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 192*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");
	
	
	// PCM[1]
//	px = &x[72];
//	SYNTHEVEN_MACF();
//	px = &x[184];
//	SYNTHEVEN_MACN();
//	pcm[1] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 72*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 184*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[2]
//	px = &x[80];
//	SYNTHEVEN_MACF();
//	px = &x[176];
//	SYNTHEVEN_MACN();
//	pcm[2] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 80*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 176*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[3]
//	px = &x[88];
//	SYNTHEVEN_MACF();
//	px = &x[168];
//	SYNTHEVEN_MACN();
//	pcm[3] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 88*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 168*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[4]
//	px = &x[96];
//	SYNTHEVEN_MACF();
//	px = &x[160];
//	SYNTHEVEN_MACN();
//	pcm[4] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 96*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 160*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[5]
//	px = &x[104];
//	SYNTHEVEN_MACF();
//	px = &x[152];
//	SYNTHEVEN_MACN();
//	pcm[5] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 104*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 152*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[6]
//	px = &x[112];
//	SYNTHEVEN_MACF();
//	px = &x[144];
//	SYNTHEVEN_MACN();
//	pcm[6] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 112*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 144*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[7]
//	px = &x[120];
//	SYNTHEVEN_MACF();
//	px = &x[136];
//	SYNTHEVEN_MACN();
//	pcm[7] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 120*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 136*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[8]
//	pw += 16;
//	px = &x[128];
//	SYNTHEVEN_MACFN();
//	pcm[8] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 128*4");
	asm("add	%r7, %r2");
	asm("call.d	%r1");
	asm("add	%r8, 32");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[9]
//	px = &x[120];
//	SYNTHEVEN_MACF();
//	px = &x[136];
//	SYNTHEVEN_MACN();
//	pcm[9] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 120*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 136*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[10]
//	px = &x[112];
//	SYNTHEVEN_MACF();
//	px = &x[144];
//	SYNTHEVEN_MACN();
//	pcm[10] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 112*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 144*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[11]
//	px = &x[104];
//	SYNTHEVEN_MACF();
//	px = &x[152];
//	SYNTHEVEN_MACN();
//	pcm[11] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 104*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 152*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[12]
//	px = &x[96];
//	SYNTHEVEN_MACF();
//	px = &x[160];
//	SYNTHEVEN_MACN();
//	pcm[12] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 96*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 160*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[13]
//	px = &x[88];
//	SYNTHEVEN_MACF();
//	px = &x[168];
//	SYNTHEVEN_MACN();
//	pcm[13] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 88*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 168*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[14]
//	px = &x[80];
//	SYNTHEVEN_MACF();
//	px = &x[176];
//	SYNTHEVEN_MACN();
//	pcm[14] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 80*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 176*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[15]
//	px = &x[72];
//	SYNTHEVEN_MACF();
//	px = &x[184];
//	SYNTHEVEN_MACN();
//	pcm[15] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 72*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 184*4");
	asm("add	%r8, 16");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");
	
	asm("pop	%r11");
	asm("pop	%r10");
	asm("popn	%r3");
}

static void Mp3SynthOddSlot(int *x, short *pcm, int slot, int vol)
{
	
	// %r0		MacInit
	// %r1		MacRepeat8
	// %r2		x
	// %r3		pcm
	
	// %r4
	
	// %r6		8
	// %r7		px
	// %r8		pw
	// %r9		vol
	
	asm("pushn	%r3");
	asm("push	%r10");
	asm("push	%r11");
	asm("xld.w	%r0, MacInit");
	asm("xld.w	%r1, MacRepeat8");
	asm("ld.w	%r2, %r6");
	asm("ld.w	%r3, %r7");
	asm("xld.w	%r10, 0x7fff");
	asm("xld.w	%r11, -0x7fff");
	
	// pw = DST_RAM_ADDRESS/4 - 1 - (slot >> 1);
	asm("sra	%r8, 1");
	asm("xld.w	%r4, 0x20fff");
	asm("sub	%r4, %r8");
	asm("ld.w	%r8, %r4");
	
//	px = &x[192];
//	SYNTHODD_MACF();
//	px = &x[64];
//	SYNTHODD_MACN();
//	pcm[0] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 192*4");
	asm("add	%r8, 9");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 64*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[1]
//	px = &x[200];
//	SYNTHODD_MACF();
//	px = &x[56];
//	SYNTHODD_MACN();
//	pcm[1] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 200*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 56*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[2]
//	px = &x[208];
//	SYNTHODD_MACF();
//	px = &x[48];
//	SYNTHODD_MACN();
//	pcm[2] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 208*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 48*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[3]
//	px = &x[216];
//	SYNTHODD_MACF();
//	px = &x[40];
//	SYNTHODD_MACN();
//	pcm[3] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 216*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 40*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[4]
//	px = &x[224];
//	SYNTHODD_MACF();
//	px = &x[32];
//	SYNTHODD_MACN();
//	pcm[4] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 224*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 32*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[5]
//	px = &x[232];
//	SYNTHODD_MACF();
//	px = &x[24];
//	SYNTHODD_MACN();
//	pcm[5] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 232*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 24*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[6]
//	px = &x[240];
//	SYNTHODD_MACF();
//	px = &x[16];
//	SYNTHODD_MACN();
//	pcm[6] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 240*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 16*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[7]
//	px = &x[248];
//	SYNTHODD_MACF();
//	px = &x[8];
//	SYNTHODD_MACN();
//	pcm[7] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 248*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 8*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[8]
//	pw += 24 - 9;
//	px = &x[0];
//	SYNTHODD_MACFN();
//	pcm[8] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("add	%r8, 32");
	asm("call.d	%r1");
	asm("ld.w	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[9]
//	px = &x[248];
//	SYNTHODD_MACF();
//	px = &x[8];
//	SYNTHODD_MACN();
//	pcm[9] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 248*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 8*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[10]
//	px = &x[240];
//	SYNTHODD_MACF();
//	px = &x[16];
//	SYNTHODD_MACN();
//	pcm[10] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 240*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 16*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");
	
	// PCM[11]
//	px = &x[232];
//	SYNTHODD_MACF();
//	px = &x[24];
//	SYNTHODD_MACN();
//	pcm[11] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 232*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 24*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[12]
//	px = &x[224];
//	SYNTHODD_MACF();
//	px = &x[32];
//	SYNTHODD_MACN();
//	pcm[12] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 224*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 32*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[13]
//	px = &x[216];
//	SYNTHODD_MACF();
//	px = &x[40];
//	SYNTHODD_MACN();
//	pcm[13] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 216*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 40*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[14]
//	px = &x[208];
//	SYNTHODD_MACF();
//	px = &x[48];
//	SYNTHODD_MACN();
//	pcm[14] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 208*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 48*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	// PCM[15]
//	px = &x[200];
//	SYNTHODD_MACF();
//	px = &x[56];
//	SYNTHODD_MACN();
//	pcm[15] = SYN_CONV(pcmdata);
	asm("ld.w	%r6, 0");
	asm("call.d	%r0");
	asm("ld.w	%r7, 0");
	
	asm("ld.w	%r6, 8");
	asm("xld.w	%r7, 200*4");
	asm("add	%r8, 17");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("xld.w	%r7, 56*4");
	asm("add	%r8, 15");
	asm("call.d	%r1");
	asm("add	%r7, %r2");
	
	asm("mlt.w	%r4, %r9");
	asm("ld.w	%r4, %alr");
	asm("sra	%r4, 0x0c");
	asm("cmp	%r4, %r10");
	asm("jrle	0x3");
	asm("ld.w	%r4, %r10");
	asm("jp		0x4");
	asm("cmp	%r4, %r11");
	asm("jrge	0x2");
	asm("ld.w	%r4, %r11");
	asm("ld.h	[%r3]+, %r4");

	asm("pop	%r11");
	asm("pop	%r10");
	asm("popn	%r3");
}

#endif

#endif // CONFIG_DECODE_MP3_ENABLE

