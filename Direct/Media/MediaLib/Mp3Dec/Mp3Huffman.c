//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－哈夫曼解压缩部分
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


typedef struct _HFM_PTBL
{
	struct _HFM_TBL const *tbl;
	BYTE linbits;
}HFM_PTBL;


extern const HFM_TBL Mp3HuffTable0[];
extern const HFM_TBL Mp3HuffTable1[];
extern const HFM_TBL Mp3HuffTable2[];
extern const HFM_TBL Mp3HuffTable3[];
extern const HFM_TBL Mp3HuffTable5[];
extern const HFM_TBL Mp3HuffTable6[];
extern const HFM_TBL Mp3HuffTable7[];
extern const HFM_TBL Mp3HuffTable8[];
extern const HFM_TBL Mp3HuffTable9[];
extern const HFM_TBL Mp3HuffTable10[];
extern const HFM_TBL Mp3HuffTable11[];
extern const HFM_TBL Mp3HuffTable12[];
extern const HFM_TBL Mp3HuffTable13[];
extern const HFM_TBL Mp3HuffTable15[];
extern const HFM_TBL Mp3HuffTable16[];
extern const HFM_TBL Mp3HuffTable24[];

extern const HFM_WTBL Mp3HuffTableA[];
extern const HFM_WTBL Mp3HuffTableB[];


HFM_PTBL const HuffPairTable[32] = 
{
	{Mp3HuffTable0,   0},	//  0
	{Mp3HuffTable1,   0},	//  1
	{Mp3HuffTable2,   0},	//  2
	{Mp3HuffTable3,   0},	//  3
	{NULL,         0},  //  4
	{Mp3HuffTable5,   0},	//  5
	{Mp3HuffTable6,   0},	//  6
	{Mp3HuffTable7,   0},	//  7
	{Mp3HuffTable8,   0},	//  8
	{Mp3HuffTable9,   0},	//  9
	{Mp3HuffTable10,  0},	// 10
	{Mp3HuffTable11,  0},	// 11
	{Mp3HuffTable12,  0},	// 12
	{Mp3HuffTable13,  0},	// 13
	{NULL,         0},	// 14
	{Mp3HuffTable15,  0},	// 15
	{Mp3HuffTable16,  1},	// 16
	{Mp3HuffTable16,  2},	// 17
	{Mp3HuffTable16,  3},	// 18
	{Mp3HuffTable16,  4},	// 19
	{Mp3HuffTable16,  6},	// 20
	{Mp3HuffTable16,  8},	// 21
	{Mp3HuffTable16, 10},	// 22
	{Mp3HuffTable16, 13},	// 23
	{Mp3HuffTable24,  4},	// 24
	{Mp3HuffTable24,  5},	// 25
	{Mp3HuffTable24,  6},	// 26
	{Mp3HuffTable24,  7},	// 27
	{Mp3HuffTable24,  8},	// 28
	{Mp3HuffTable24,  9},	// 29
	{Mp3HuffTable24, 11},	// 30
	{Mp3HuffTable24, 13} 	// 31
};



static struct
{
	unsigned char slen1;
	unsigned char slen2;
}
const _ScfLenTable[16] =
{
	{ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
	{ 3, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 },
	{ 2, 1 }, { 2, 2 }, { 2, 3 }, { 3, 1 },
	{ 3, 2 }, { 3, 3 }, { 4, 2 }, { 4, 3 }
};

extern const SFB_WIDTH SfbWidthTable[9];

/*
* number of LSF scalefactor band values
* derived from section 2.4.3.2 of ISO/IEC 13818-3
*/
static const BYTE nsfb_table[6][3][4] =
{
    {
		{ 6, 5, 5, 5 },
		{ 9, 9, 9, 9 },
		{ 6, 9, 9, 9 }
	},
	{
		{ 6, 5, 7, 3 },
		{ 9, 9, 12, 6 },
		{ 6, 9, 12, 6 }
	},
		
	{
		{ 11, 10, 0, 0 },
		{ 18, 18, 0, 0 },
		{ 15, 18, 0, 0 }
	},
		
	{
		{ 7, 7, 7, 0 },
		{ 12, 12, 12, 0 },
		{ 6, 15, 12, 0 }
	},
		
	{
		{ 6, 6, 6, 3 },
		{ 12, 9, 9, 6 },
		{ 6, 12, 9, 6 }
	},
		
	{
		{ 8, 8, 5, 0 },
		{ 15, 12, 9, 0 },
		{ 6, 18, 9, 0 }
	}
};

static const BYTE _XMod5Table[] = 
{
	0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
	0, 1, 2, 3, 4, 0, 1, 2, 3, 4,
	0, 1, 2, 3, 4, 0, 1, 2, 3, 4
};


static const BYTE _XDiv5Table[] = 
{
	0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 5, 5, 5, 5, 5
};


static const BYTE _XMod3Table[] = 
{
	0, 1, 2, 0, 1, 2, 0, 1, 2,
	0, 1, 2, 0, 1, 2, 0, 1, 2,
	0, 1, 2, 0, 1, 2, 0, 1, 2
};


static const BYTE _XDiv3Table[] = 
{
	0, 0, 0, 1, 1, 1, 2, 2, 2,
	3, 3, 3, 4, 4, 4, 5, 5, 5,
	6, 6, 6, 7, 7, 7, 8, 8, 8
};

struct 
{
	BYTE slen0;
	BYTE slen1;
	BYTE slen2;
}
const _SlenTable0[] = 
{
	{0x0, 0x0, 0x0},
	{0x0, 0x0, 0x1},
	{0x0, 0x0, 0x2},
	{0x0, 0x0, 0x3},
	{0x0, 0x0, 0x4},
	{0x0, 0x0, 0x5},
	{0x0, 0x1, 0x0},
	{0x0, 0x1, 0x1},
	{0x0, 0x1, 0x2},
	{0x0, 0x1, 0x3},
	{0x0, 0x1, 0x4},
	{0x0, 0x1, 0x5},
	{0x0, 0x2, 0x0},
	{0x0, 0x2, 0x1},
	{0x0, 0x2, 0x2},
	{0x0, 0x2, 0x3},
	{0x0, 0x2, 0x4},
	{0x0, 0x2, 0x5},
	{0x0, 0x3, 0x0},
	{0x0, 0x3, 0x1},
	{0x0, 0x3, 0x2},
	{0x0, 0x3, 0x3},
	{0x0, 0x3, 0x4},
	{0x0, 0x3, 0x5},
	{0x0, 0x4, 0x0},
	{0x0, 0x4, 0x1},
	{0x0, 0x4, 0x2},
	{0x0, 0x4, 0x3},
	{0x0, 0x4, 0x4},
	{0x0, 0x4, 0x5},
	{0x0, 0x5, 0x0},
	{0x0, 0x5, 0x1},
	{0x0, 0x5, 0x2},
	{0x0, 0x5, 0x3},
	{0x0, 0x5, 0x4},
	{0x0, 0x5, 0x5},
	{0x1, 0x0, 0x0},
	{0x1, 0x0, 0x1},
	{0x1, 0x0, 0x2},
	{0x1, 0x0, 0x3},
	{0x1, 0x0, 0x4},
	{0x1, 0x0, 0x5},
	{0x1, 0x1, 0x0},
	{0x1, 0x1, 0x1},
	{0x1, 0x1, 0x2},
	{0x1, 0x1, 0x3},
	{0x1, 0x1, 0x4},
	{0x1, 0x1, 0x5},
	{0x1, 0x2, 0x0},
	{0x1, 0x2, 0x1},
	{0x1, 0x2, 0x2},
	{0x1, 0x2, 0x3},
	{0x1, 0x2, 0x4},
	{0x1, 0x2, 0x5},
	{0x1, 0x3, 0x0},
	{0x1, 0x3, 0x1},
	{0x1, 0x3, 0x2},
	{0x1, 0x3, 0x3},
	{0x1, 0x3, 0x4},
	{0x1, 0x3, 0x5},
	{0x1, 0x4, 0x0},
	{0x1, 0x4, 0x1},
	{0x1, 0x4, 0x2},
	{0x1, 0x4, 0x3},
	{0x1, 0x4, 0x4},
	{0x1, 0x4, 0x5},
	{0x1, 0x5, 0x0},
	{0x1, 0x5, 0x1},
	{0x1, 0x5, 0x2},
	{0x1, 0x5, 0x3},
	{0x1, 0x5, 0x4},
	{0x1, 0x5, 0x5},
	{0x2, 0x0, 0x0},
	{0x2, 0x0, 0x1},
	{0x2, 0x0, 0x2},
	{0x2, 0x0, 0x3},
	{0x2, 0x0, 0x4},
	{0x2, 0x0, 0x5},
	{0x2, 0x1, 0x0},
	{0x2, 0x1, 0x1},
	{0x2, 0x1, 0x2},
	{0x2, 0x1, 0x3},
	{0x2, 0x1, 0x4},
	{0x2, 0x1, 0x5},
	{0x2, 0x2, 0x0},
	{0x2, 0x2, 0x1},
	{0x2, 0x2, 0x2},
	{0x2, 0x2, 0x3},
	{0x2, 0x2, 0x4},
	{0x2, 0x2, 0x5},
	{0x2, 0x3, 0x0},
	{0x2, 0x3, 0x1},
	{0x2, 0x3, 0x2},
	{0x2, 0x3, 0x3},
	{0x2, 0x3, 0x4},
	{0x2, 0x3, 0x5},
	{0x2, 0x4, 0x0},
	{0x2, 0x4, 0x1},
	{0x2, 0x4, 0x2},
	{0x2, 0x4, 0x3},
	{0x2, 0x4, 0x4},
	{0x2, 0x4, 0x5},
	{0x2, 0x5, 0x0},
	{0x2, 0x5, 0x1},
	{0x2, 0x5, 0x2},
	{0x2, 0x5, 0x3},
	{0x2, 0x5, 0x4},
	{0x2, 0x5, 0x5},
	{0x3, 0x0, 0x0},
	{0x3, 0x0, 0x1},
	{0x3, 0x0, 0x2},
	{0x3, 0x0, 0x3},
	{0x3, 0x0, 0x4},
	{0x3, 0x0, 0x5},
	{0x3, 0x1, 0x0},
	{0x3, 0x1, 0x1},
	{0x3, 0x1, 0x2},
	{0x3, 0x1, 0x3},
	{0x3, 0x1, 0x4},
	{0x3, 0x1, 0x5},
	{0x3, 0x2, 0x0},
	{0x3, 0x2, 0x1},
	{0x3, 0x2, 0x2},
	{0x3, 0x2, 0x3},
	{0x3, 0x2, 0x4},
	{0x3, 0x2, 0x5},
	{0x3, 0x3, 0x0},
	{0x3, 0x3, 0x1},
	{0x3, 0x3, 0x2},
	{0x3, 0x3, 0x3},
	{0x3, 0x3, 0x4},
	{0x3, 0x3, 0x5},
	{0x3, 0x4, 0x0},
	{0x3, 0x4, 0x1},
	{0x3, 0x4, 0x2},
	{0x3, 0x4, 0x3},
	{0x3, 0x4, 0x4},
	{0x3, 0x4, 0x5},
	{0x3, 0x5, 0x0},
	{0x3, 0x5, 0x1},
	{0x3, 0x5, 0x2},
	{0x3, 0x5, 0x3},
	{0x3, 0x5, 0x4},
	{0x3, 0x5, 0x5},
	{0x4, 0x0, 0x0},
	{0x4, 0x0, 0x1},
	{0x4, 0x0, 0x2},
	{0x4, 0x0, 0x3},
	{0x4, 0x0, 0x4},
	{0x4, 0x0, 0x5},
	{0x4, 0x1, 0x0},
	{0x4, 0x1, 0x1},
	{0x4, 0x1, 0x2},
	{0x4, 0x1, 0x3},
	{0x4, 0x1, 0x4},
	{0x4, 0x1, 0x5},
	{0x4, 0x2, 0x0},
	{0x4, 0x2, 0x1},
	{0x4, 0x2, 0x2},
	{0x4, 0x2, 0x3},
	{0x4, 0x2, 0x4},
	{0x4, 0x2, 0x5},
	{0x4, 0x3, 0x0},
	{0x4, 0x3, 0x1},
	{0x4, 0x3, 0x2},
	{0x4, 0x3, 0x3},
	{0x4, 0x3, 0x4},
	{0x4, 0x3, 0x5},
	{0x4, 0x4, 0x0},
	{0x4, 0x4, 0x1},
	{0x4, 0x4, 0x2},
	{0x4, 0x4, 0x3},
	{0x4, 0x4, 0x4},
	{0x4, 0x4, 0x5},
	{0x4, 0x5, 0x0},
	{0x4, 0x5, 0x1},
	{0x4, 0x5, 0x2},
	{0x4, 0x5, 0x3},
	{0x4, 0x5, 0x4},
	{0x4, 0x5, 0x5}
};

struct 
{
	BYTE slen0;
	BYTE slen1;
	BYTE slen2;
}
const _SlenTable1[] = 
{
	{0x0, 0x0, 0x0},
	{0x0, 0x0, 0x1},
	{0x0, 0x0, 0x2},
	{0x0, 0x0, 0x3},
	{0x0, 0x1, 0x0},
	{0x0, 0x1, 0x1},
	{0x0, 0x1, 0x2},
	{0x0, 0x1, 0x3},
	{0x0, 0x2, 0x0},
	{0x0, 0x2, 0x1},
	{0x0, 0x2, 0x2},
	{0x0, 0x2, 0x3},
	{0x0, 0x3, 0x0},
	{0x0, 0x3, 0x1},
	{0x0, 0x3, 0x2},
	{0x0, 0x3, 0x3},
	{0x1, 0x0, 0x0},
	{0x1, 0x0, 0x1},
	{0x1, 0x0, 0x2},
	{0x1, 0x0, 0x3},
	{0x1, 0x1, 0x0},
	{0x1, 0x1, 0x1},
	{0x1, 0x1, 0x2},
	{0x1, 0x1, 0x3},
	{0x1, 0x2, 0x0},
	{0x1, 0x2, 0x1},
	{0x1, 0x2, 0x2},
	{0x1, 0x2, 0x3},
	{0x1, 0x3, 0x0},
	{0x1, 0x3, 0x1},
	{0x1, 0x3, 0x2},
	{0x1, 0x3, 0x3},
	{0x2, 0x0, 0x0},
	{0x2, 0x0, 0x1},
	{0x2, 0x0, 0x2},
	{0x2, 0x0, 0x3},
	{0x2, 0x1, 0x0},
	{0x2, 0x1, 0x1},
	{0x2, 0x1, 0x2},
	{0x2, 0x1, 0x3},
	{0x2, 0x2, 0x0},
	{0x2, 0x2, 0x1},
	{0x2, 0x2, 0x2},
	{0x2, 0x2, 0x3},
	{0x2, 0x3, 0x0},
	{0x2, 0x3, 0x1},
	{0x2, 0x3, 0x2},
	{0x2, 0x3, 0x3},
	{0x3, 0x0, 0x0},
	{0x3, 0x0, 0x1},
	{0x3, 0x0, 0x2},
	{0x3, 0x0, 0x3},
	{0x3, 0x1, 0x0},
	{0x3, 0x1, 0x1},
	{0x3, 0x1, 0x2},
	{0x3, 0x1, 0x3},
	{0x3, 0x2, 0x0},
	{0x3, 0x2, 0x1},
	{0x3, 0x2, 0x2},
	{0x3, 0x2, 0x3},
	{0x3, 0x3, 0x0},
	{0x3, 0x3, 0x1},
	{0x3, 0x3, 0x2},
	{0x3, 0x3, 0x3}
};


#define BitCacheRead(ret, bits)								\
{															\
	if(BitRemain < bits)								\
		BitRemain = Mp3BufMainCache(buf, cache, BitRemain);						\
	BitRemain -= bits;								\
	ret = (cache->Cache.DWord >> BitRemain);	\
	ret &= ~((0xffffffffL << bits));							\
}														


#define BitCacheRead1(ret)									\
{															\
	if(!BitRemain)								\
		BitRemain = Mp3BufMainCache(buf, cache, BitRemain);						\
	BitRemain--;									\
	ret = (cache->Cache.DWord >> BitRemain);	\
	ret &= 0x0001;											\
}														

#define BitCacheRead4(ret)									\
{															\
	if(BitRemain < 4)								\
		BitRemain = Mp3BufMainCache(buf, cache, BitRemain);						\
	BitRemain -= 4;									\
	ret = (cache->Cache.DWord >> BitRemain);	\
	ret &= 0x000f;											\
}														


#define BitCacheRead6( ret )									\
{															\
	if(BitRemain < 6)								\
		BitRemain = Mp3BufMainCache(buf, cache, BitRemain);						\
	BitRemain -= 6;									\
	ret = (cache->Cache.DWord >> BitRemain);	\
	ret &= 0x003f;											\
}														


#define BitCacheRead8( ret )									\
{															\
	if(BitRemain < 8)								\
		BitRemain = Mp3BufMainCache(buf, cache, BitRemain);						\
	BitRemain -= 8;									\
	ret = (cache->Cache.DWord >> BitRemain);	\
	ret &= 0x00ff;											\
}														

#define BitCacheRead10( ret )									\
{															\
	if(BitRemain < 10)								\
		BitRemain = Mp3BufMainCache(buf, cache, BitRemain);						\
	BitRemain -= 10;									\
	ret = (cache->Cache.DWord >> BitRemain);	\
	ret &= 0x03ff;											\
}														

#define BitCacheBack(bits)								\
{														\
	BitRemain += bits;							\
}														



////////////////////////////////////////////////////
// 功能: 低采样率比例因子解压
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3ScaleDecodeLsf(MP3_STREAM *stream, short ch, short BitRemain)
{
	unsigned int scalefac_compress, index, part, n, i;
	BYTE slen[4];
	unsigned char const *nsfb;
	MP3_SIDEINFO *si;
	MP3_BUF *buf;
	MP3_CHSI *channel, *gr1ch;
	MP3_MCACHE *cache;
	
	si = &stream->SideInfo;
	buf = &stream->Buffer;
	channel = &stream->SideInfo.Channel[0][ch];
	gr1ch = &stream->SideInfo.Channel[1][1];
	cache = &stream->Cache;
	scalefac_compress = channel->ScfCompress;
	index = (channel->BlockType == 2) ?
		((channel->Flags & MP3_MIXED_BLOCK) ? 2 : 1) : 0;
	
	if(!(((stream->Header.ModeEx == 0x01) || (stream->Header.ModeEx == 0x3))
		&& (ch == 1)))
	{
		if (scalefac_compress < 400)
		{
			slen[0] = _XDiv5Table[scalefac_compress >> 4];
			slen[1] = _XMod5Table[scalefac_compress >> 4];
			slen[2] = (scalefac_compress & 0x0f) >> 2;
			slen[3] = scalefac_compress & 0x03;
			
			nsfb = nsfb_table[0][index];
		}
		else if (scalefac_compress < 500)
		{
			scalefac_compress -= 400;
			
			slen[0] = _XDiv5Table[scalefac_compress >> 2];
			slen[1] = _XMod5Table[scalefac_compress >> 2];
			slen[2] = scalefac_compress & 0x03;
			slen[3] = 0;
			
			nsfb = nsfb_table[1][index];
		}
		else
		{
			scalefac_compress -= 500;
			
			slen[0] = _XDiv3Table[scalefac_compress];
			slen[1] = _XMod3Table[scalefac_compress];
			slen[2] = 0;
			slen[3] = 0;
			channel->Flags |= MP3_PREFLAG;
			nsfb = nsfb_table[2][index];
		}
		
		n = 0;
		for (part = 0; part < 4; ++part)
		{
			for (i = 0; i < nsfb[part]; ++i)
			{
				BitCacheRead(channel->Scalefac[n], slen[part]);
				n++;
			}
			cache->BitCount += nsfb[part] * slen[part];
		}
		
		while (n < 39)
			channel->Scalefac[n++] = 0;
	}
	else
	{
		scalefac_compress >>= 1;
		
		if (scalefac_compress < 180)
		{
			slen[0] = _SlenTable0[scalefac_compress].slen0;
			slen[1] = _SlenTable0[scalefac_compress].slen1;
			slen[2] = _SlenTable0[scalefac_compress].slen2;
			slen[3] = 0;
			
			nsfb = nsfb_table[3][index];
		}
		else if (scalefac_compress < 244)
		{
			scalefac_compress -= 180;
			
			slen[0] = _SlenTable1[scalefac_compress].slen0;
			slen[1] = _SlenTable1[scalefac_compress].slen1;
			slen[2] = _SlenTable1[scalefac_compress].slen2;
			slen[3] = 0;
			
			nsfb = nsfb_table[4][index];
		}
		else
		{
			scalefac_compress -= 244;
			
			slen[0] = _XDiv3Table[scalefac_compress];
			slen[1] = _XMod3Table[scalefac_compress];
			slen[2] = 0;
			slen[3] = 0;
			
			nsfb = nsfb_table[5][index];
		}
		
		n = 0;
		for (part = 0; part < 4; ++part)
		{
			WORD max, scf;
			max = (1 << nsfb[part]) - 1;
			for (i = 0; i < nsfb[part]; ++i)
			{
				BitCacheRead(scf, slen[part]);
				channel->Scalefac[n] = scf;
				gr1ch->Scalefac[n++] = (max == scf);
			}
			cache->BitCount += nsfb[part] * slen[part];
		}
		
		while (n < 39)
		{
			gr1ch->Scalefac[n] = 0;
			channel->Scalefac[n++] = 0;
		}
	}
	return BitRemain;
}


////////////////////////////////////////////////////
// 功能: 普通采样率下的比例因子解压
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3ScaleDecodeNsf(MP3_STREAM *stream, short gr, short ch, short BitRemain)
{
	BYTE slen1, slen2;
	MP3_BUF *buf;
	MP3_CHSI *channel;
	MP3_MCACHE *cache;
	WORD *scf;
			
	buf = &stream->Buffer;
	cache = &stream->Cache;
	channel = &stream->SideInfo.Channel[gr][ch];
	slen1 = _ScfLenTable[channel->ScfCompress].slen1;
	slen2 = _ScfLenTable[channel->ScfCompress].slen2;
	scf = (WORD*) channel->Scalefac;
	
	if (channel->BlockType == 2)
	{
		short nsfb;
		
		if(!(channel->Flags & MP3_MIXED_BLOCK))
		{
			BitCacheRead(*scf, slen1);
			scf++;
			cache->BitCount += 18 * slen1;
		}
		else
		{
			cache->BitCount += 17 * slen1;
		}
		
		nsfb = 3;
		while(nsfb--)
		{
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
		}
		BitCacheRead(*scf, slen1);
		scf++;
		BitCacheRead(*scf, slen1);
		scf++;
		
		nsfb = 3;
		while (nsfb--)
		{
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
		}
		cache->BitCount += 18 * slen2;
		
		*scf++ = 0;
		*scf++ = 0;
		*scf++ = 0;
	}
	else
	{
		WORD *gr0scf;
		WORD scfsi = stream->SideInfo.ScfSI[ch];
		gr0scf = (WORD*)stream->SideInfo.Channel[0][ch].Scalefac;
		if(gr == 0)
			scfsi = 0;
		if (scfsi & 0x8)
		{
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
		}
		else
		{
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			cache->BitCount += 6 * slen1;
			gr0scf += 6;
		}
		
		if (scfsi & 0x4)
		{
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
		}
		else
		{
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			BitCacheRead(*scf, slen1);
			scf++;
			cache->BitCount += 5 * slen1;
			gr0scf += 5;
		}
		
		if (scfsi & 0x2)
		{
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
		}
		else
		{
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			cache->BitCount += 5 * slen2;
			gr0scf += 5;
		}
		
		if (scfsi & 0x1)
		{
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
			*scf++ = *gr0scf++;
		}
		else
		{
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			BitCacheRead(*scf, slen2);
			scf++;
			cache->BitCount += 5 * slen2;
		}
		channel->Scalefac[21] = 0;
	}
	return BitRemain;
}



////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3HuffmanDecode(MP3_STREAM *stream)
{
	HFM_TBL const *root, *entry;
	int index;
	int BitRemain;
	int linbits, bits;
	int BitCount;
	int line, x, y, v, w, tmp;
	int exp;
	MP3_MCACHE *cache;
	MP3_BUF *buf;
	MP3_REQUANTIZE requantize;
	const REQ_FLOAT *reqtable;
	short big_values, region_values, sfb_values;
	short region;
	short gr, ngr, ch, nch;
	MP3INT *freqline;
	short *exponent;
	const BYTE *sfbwidth;

	buf = &stream->Buffer;
	cache = &stream->Cache;
	cache->Cache.DWord = 0;
	BitRemain = 0;
	nch = (stream->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
	ngr = (stream->Header.Mpeg == MP3_MPEG1) ? 2 : 1;
	requantize = stream->ReqExponent;
	reqtable = stream->ReqTable;
	for(gr=0; gr<ngr; gr++)
	{
		
		for(ch=0; ch<nch; ch++)
		{
			MP3_CHSI *channel;

			channel = &stream->SideInfo.Channel[gr][ch];
			freqline = stream->FreqLine[gr][ch];

			//////////////////////////////////////////////////
			// 比例因子解压
			//////////////////////////////////////////////////
			cache->BitCount = 0;
			if(ngr == 1)
				BitRemain = Mp3ScaleDecodeLsf(stream, ch, BitRemain);
			else
				BitRemain = Mp3ScaleDecodeNsf(stream, gr, ch, BitRemain);

			//////////////////////////////////////////////////
			// 生成反量化指数因子
			//////////////////////////////////////////////////
			Mp3RequantizeExponent(channel, stream->DecodeLines);
			
			//////////////////////////////////////////////////
			// 大值区HUFFMAN解压
			//////////////////////////////////////////////////
			sfbwidth = channel->SfbWidth;
			sfb_values = *sfbwidth++;
			exponent = channel->Exponent;
			exp = *exponent++;
			big_values = channel->BigValues;
			region_values = channel->Reg0Count + 1;
			linbits = HuffPairTable[channel->TableSelect[0]].linbits;
			root = HuffPairTable[channel->TableSelect[0]].tbl;
			if(root == NULL)
				return -1;
			channel->ZeroSbf = 0;
			line = 0;
			region = 0;
			BitCount = cache->BitCount;
			while(big_values--)
			{
				BitCacheRead6(index);
				entry = root + index;
				while(entry->next)
				{
					bits = entry->bits;
					BitCacheRead(index, bits);
					entry = entry->next;
					entry += index;
				}
				BitCacheBack(entry->bits);
				BitCount += entry->total;
				x = entry->x;
				y = entry->y;
				if(linbits)
				{
					bits = linbits + 1;
					if(x == 15)
					{
						BitCacheRead(index, bits);
						x = (index >> 1) + 15;
						if(index & 1)
							x = -x;
						if(y && (y != 15))
						{
							BitCacheRead1(index);
							if(index)
								y = -y;
							BitCount++;
						}
						BitCount += bits;
					}
					if(y == 15)
					{
						BitCount += bits;
						BitCacheRead(index, bits);
						y = (index >> 1) + 15;
						if(index & 1)
							y = -y;
					}
				}
				if(x)
					x = requantize(x, exp, reqtable);
				if(y)
					y = requantize(y, exp, reqtable);
				*freqline++ = x;
				*freqline++ = y;

				line += 2;
				if(line >= stream->DecodeLines)
					break;
				// 检查是否换区(region0->region1->region2)
				sfb_values -= 2;
				if(!sfb_values)
				{
					exp = *exponent++;
					sfb_values = *sfbwidth++;
					if(--region_values == 0)
					{
						region++;
						if(region == 1)
						{
							region_values = channel->Reg1Count + 1;
							root = HuffPairTable[channel->TableSelect[1]].tbl;
							linbits = HuffPairTable[channel->TableSelect[1]].linbits;
							if(root == NULL)
								return -1;
						}
						else
						{
							region_values = stream->DecodeLines;
							root = HuffPairTable[channel->TableSelect[2]].tbl;
							linbits = HuffPairTable[channel->TableSelect[2]].linbits;
							if(root == NULL)
								return -1;
						}
					}
					channel->ZeroSbf++;
				}
			}
				
			//////////////////////////////////////////////////
			// 0-1区HUFFMAN解压
			//////////////////////////////////////////////////
			tmp = requantize(1, exp, reqtable);
			while((channel->Part23Len > BitCount) && (line < stream->DecodeLines))
			{
				const HFM_WTBL *ptbl;

				if(!sfb_values)
				{
					exp = *exponent++;
					sfb_values = *sfbwidth++;
					channel->ZeroSbf++;
					tmp = requantize(1, exp, reqtable);
				}
				sfb_values -= 2;
				
				if(!(channel->Flags & MP3_COUNT1_TABLE))
				{
					BitCacheRead10(index);
					ptbl = Mp3HuffTableA + index;
				}
				else
				{
					BitCacheRead8(index);
					ptbl = Mp3HuffTableB + index;
				}
				BitCacheBack(ptbl->bits);
				BitCount += ptbl->total;
				v = ptbl->v;
				if(v)
					v *= tmp;
				w = ptbl->w;
				if(w)
					w *= tmp;

				if(!sfb_values)
				{
					exp = *exponent++;
					sfb_values = *sfbwidth++;
					channel->ZeroSbf++;
					tmp = requantize(1, exp, reqtable);
				}
				sfb_values -= 2;
				
				x = ptbl->x;
				if(x)
					x *= tmp;
				y = ptbl->y;
				if(y)
					y *= tmp;
				*freqline++ = v;
				*freqline++ = w;
				*freqline++ = x;
				*freqline++ = y;

				line += 4;
			}
			
			//////////////////////////////////////////////////
			// 0游程HUFFMAN解压
			//////////////////////////////////////////////////
			{
				short remain0 = channel->Part23Len - BitCount;
				if(remain0 > 0)
				{
					short remain1;
					remain1 = remain0 - BitRemain;
					if(remain1 > 0)
					{
						Mp3BufMainPass(buf, (WORD)(remain1 >> 3));
						BitRemain = -(remain1 & 0x07);
					}
					else
					{
						BitCacheRead(index, remain0);
					}
				}
				else if(remain0 < 0)
				{
					line -= 4;
					freqline -= 4;
					BitCacheBack(-remain0);
				}
			}
			channel->ZeroFreqLine = line;
			while(line < stream->DecodeLines)
			{
				*freqline++ = 0;
				*freqline++ = 0;
				line += 2;
			}
		}
	}
	return 0;
}

#endif // CONFIG_DECODE_MP3_ENABLE

