//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－边信息解码部分
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

///////////////////////////////////////
//  long block scalefactor band widths
///////////////////////////////////////
static BYTE const sfb_48000_long[] =
{
	4, 4, 4, 4, 4, 4, 6, 6, 6, 8, 10,
    12, 16, 18, 22, 28, 34, 40, 46, 54, 54, 192
};

static BYTE const sfb_44100_long[] =
{
	4, 4, 4, 4, 4, 4, 6, 6, 8, 8, 10,
    12, 16, 20, 24, 28, 34, 42, 50, 54, 76, 158
};

static BYTE const sfb_32000_long[] =
{
	4, 4, 4, 4, 4, 4, 6, 6, 8, 10, 12,
    16, 20, 24, 30, 38, 46, 56, 68, 84, 102, 26
};

static BYTE const sfb_24000_long[] =
{
	6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
    18, 22, 26, 32, 38, 46, 54, 62, 70, 76, 36
};

static BYTE const sfb_22050_long[] =
{
	6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
    20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54
};

static BYTE const sfb_8000_long[] = 
{
	12, 12, 12, 12, 12, 12, 16, 20, 24, 28, 32,
	40, 48, 56, 64, 76, 90, 2, 2, 2, 2, 2
};

#define sfb_16000_long  sfb_22050_long
#define sfb_12000_long  sfb_16000_long
#define sfb_11025_long  sfb_12000_long



///////////////////////////////////////
//  short block scalefactor band widths
///////////////////////////////////////
static BYTE const sfb_48000_short[] =
{
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6,
    6, 6, 6, 6, 6, 10, 10, 10, 12, 12, 12, 14, 14,
    14, 16, 16, 16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};

static BYTE const sfb_44100_short[] =
{
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6,
    6, 6, 8, 8, 8, 10, 10, 10, 12, 12, 12, 14, 14,
    14, 18, 18, 18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};

static BYTE const sfb_32000_short[] =
{
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6,
    6, 6, 8, 8, 8, 12, 12, 12, 16, 16, 16, 20, 20,
    20, 26, 26, 26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};

static BYTE const sfb_24000_short[] =
{
	4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 8,
	8, 8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
	18, 24, 24, 24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};

static BYTE const sfb_22050_short[] =
{
	4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6,
	6, 6, 8, 8, 8, 10, 10, 10, 14, 14, 14, 18, 18,
	18, 26, 26, 26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};

static BYTE const sfb_16000_short[] =
{
	4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 8,
	8, 8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
	18, 24, 24, 24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};

static BYTE const sfb_8000_short[] = 
{
	8, 8, 8, 8, 8, 8, 8, 8, 8, 12, 12, 12, 16,
	16, 16, 20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36,
	36, 2, 2, 2, 2, 2, 2, 2, 2, 2, 26, 26, 26
};

#define sfb_12000_short  sfb_16000_short
#define sfb_11025_short  sfb_12000_short


///////////////////////////////////////
//  mixed block scalefactor band widths
///////////////////////////////////////
static BYTE const sfb_48000_mixed[] =
{
	4, 4, 4, 4, 4, 4, 6, 6,				// long
	4, 4, 4, 6, 6, 6, 6, 6, 6, 10,
	10, 10, 12, 12, 12, 14, 14, 14, 16, 16,
    16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};

static BYTE const sfb_44100_mixed[] =
{
	4, 4, 4, 4, 4, 4, 6, 6,				// long
	4, 4, 4, 6, 6, 6, 8, 8, 8, 10,
	10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
    18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};

static BYTE const sfb_32000_mixed[] =
{
	4, 4, 4, 4, 4, 4, 6, 6,				// long
	4, 4, 4, 6, 6, 6, 8, 8, 8, 12,
	12, 12, 16, 16, 16, 20, 20, 20, 26, 26,
    26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};

static BYTE const sfb_24000_mixed[] =
{
	6, 6, 6, 6, 6, 6,					// long
	6, 6, 6, 8, 8, 8, 10, 10, 10, 12,
	12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
	24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};

static BYTE const sfb_22050_mixed[] =
{
	6, 6, 6, 6, 6, 6,					// long
	6, 6, 6, 6, 6, 6, 8, 8, 8, 10,
	10, 10, 14, 14, 14, 18, 18, 18, 26, 26,
	26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};

static BYTE const sfb_16000_mixed[] =
{
	6, 6, 6, 6, 6, 6,					// long
	6, 6, 6, 8, 8, 8, 10, 10, 10, 12,
	12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
	24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};

static BYTE const sfb_8000_mixed[] = 
{
	12, 12, 12,								// long
	4, 4, 4, 8, 8, 8, 12, 12, 12, 16, 16, 16,
	20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36, 36,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 26, 26, 26
};

#define sfb_12000_mixed  sfb_16000_mixed
#define sfb_11025_mixed  sfb_16000_mixed

const SFB_WIDTH SfbWidthTable[9] = 
{
	{ sfb_48000_long, sfb_48000_short, sfb_48000_mixed },
	{ sfb_44100_long, sfb_44100_short, sfb_44100_mixed },
	{ sfb_32000_long, sfb_32000_short, sfb_32000_mixed },
	{ sfb_24000_long, sfb_24000_short, sfb_24000_mixed },
	{ sfb_22050_long, sfb_22050_short, sfb_22050_mixed },
	{ sfb_16000_long, sfb_16000_short, sfb_16000_mixed },
	{ sfb_12000_long, sfb_12000_short, sfb_12000_mixed },
	{ sfb_11025_long, sfb_11025_short, sfb_11025_mixed },
	{ sfb_8000_long,  sfb_8000_short,  sfb_8000_mixed  }
};


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3SinfoDecode(MP3_STREAM *stream)
{
	int gr, ngr, ch, nch, id;
	MP3_SIDEINFO *si;
	MP3_BUF *buf;
	WORD tmp;
	DWORD sfreq;
	BYTE const *sfbwidth;

	// 48000 => 0, 44100 => 1, 32000 => 2,
	// 24000 => 3, 22050 => 4, 16000 => 5
	sfreq = stream->Header.Samplerate;
	if(stream->Header.Mpeg == MP3_MPEG25)
		sfreq <<= 1;
	sfreq = ((sfreq >> 7) & 0x000f) +
	         ((sfreq >> 15) & 0x0001) - 8;
	if(stream->Header.Mpeg == MP3_MPEG25)
		sfreq += 3;
	stream->ScfbSelect = (short) sfreq;

	si = &stream->SideInfo;
	buf = &stream->Buffer;
	Mp3BufCacheReset(buf);
	nch = (stream->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
	id = (stream->Header.Mpeg == MP3_MPEG1);
	if(id)
	{
		si->MainDataBegin = Mp3BufGetBits(buf, 9);
		if(nch == 1)
		{
			tmp = Mp3BufGetBits(buf, 9);
			si->PrivateBits = (BYTE)(tmp >> 5);
			si->ScfSI[0] = (BYTE)(tmp & 0x1f);
		}
		else
		{
			tmp = Mp3BufGetBits(buf, 11);
			si->PrivateBits = (BYTE)(tmp >> 8);
			si->ScfSI[0] = (BYTE)(tmp >> 4) & 0x0f;
			si->ScfSI[1] = (BYTE)(tmp & 0x0f);
		}
		ngr = 2;
	}
	else
	{
		si->MainDataBegin = Mp3BufGetBits(buf, 8);
		if(nch == 1)
			si->PrivateBits = (BYTE)Mp3BufGetBits(buf, 1);
		else
			si->PrivateBits = (BYTE)Mp3BufGetBits(buf, 2);
		ngr = 1;
	}
	
	for (gr = 0; gr < ngr; gr++)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			MP3_CHSI *channel = &stream->SideInfo.Channel[gr][ch];
			
			channel->Flags = 0;
			channel->Part23Len = Mp3BufGetBits(buf, 12);
			channel->BigValues = Mp3BufGetBits(buf, 9);
			channel->GlobalGain = Mp3BufGetBits(buf, 8);
			if(id)
				channel->ScfCompress = Mp3BufGetBits(buf, 4);
			else
				channel->ScfCompress = Mp3BufGetBits(buf, 9);
						
			// window_switching_flag
			if(Mp3BufGetBits(buf, 1))
			{
				tmp = Mp3BufGetBits(buf, 3);
				channel->BlockType = (BYTE) (tmp >> 1);
				
				channel->Reg0Count = 7;
				channel->Reg1Count = 36;
				
				if(tmp & 1)
					channel->Flags |= MP3_MIXED_BLOCK;
				else if(channel->BlockType == 2)
					channel->Reg0Count = 8;
				
				tmp = Mp3BufGetBits(buf, 10);
				channel->TableSelect[0] = (BYTE)(tmp >> 5);
				channel->TableSelect[1] = (BYTE)(tmp & 0x1f);
				channel->TableSelect[2] = 0xcc;
				
				tmp = Mp3BufGetBits(buf, 9);
				channel->SubblockGain[0] = (BYTE)(tmp >> 6);
				channel->SubblockGain[1] = (BYTE)(tmp >> 3) & 0x07;
				channel->SubblockGain[2] = (BYTE)(tmp & 0x7);
			}
			else
			{
				channel->BlockType = 0;
				
				tmp = Mp3BufGetBits(buf, 15);
				channel->TableSelect[0] = (BYTE)(tmp >> 10);
				channel->TableSelect[1] = (BYTE)(tmp >> 5) & 0x1f;
				channel->TableSelect[2] = (BYTE)(tmp & 0x1f);
				
				tmp = Mp3BufGetBits(buf, 7);
				channel->Reg0Count = (BYTE)(tmp >> 3);
				channel->Reg1Count = (BYTE)(tmp & 0x7);
			}

			if(id)
				channel->Flags |= Mp3BufGetBits(buf, 3);
			else
				channel->Flags |= Mp3BufGetBits(buf, 2);

			// 获取比例因子选择信息
			if (channel->BlockType == 2)
			{
				sfbwidth = SfbWidthTable[sfreq].s;
				if (channel->Flags & MP3_MIXED_BLOCK)
					sfbwidth = SfbWidthTable[sfreq].m;
			}
			else
			{
				sfbwidth = SfbWidthTable[sfreq].l;
			}
			channel->SfbWidth = sfbwidth;
		}
	}
	return 0;
}

#endif // CONFIG_DECODE_MP3_ENABLE

