//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－反量化处理部分
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

// The Layer III formula for requantization and scaling is defined by
// section 2.4.3.4.7.1 of ISO/IEC 11172-3, as follows:
// 
//   long blocks:
//   xr[i] = sign(is[i]) * abs(is[i])^(4/3) *
//           2^((1/4) * (global_gain - 210)) *
//           2^-(multiplier *
//                 (scalefac_l[sfb] + preflag * pretab[sfb]))
// 
//   short blocks:
//   xr[i] = sign(is[i]) * abs(is[i])^(4/3) *
//           2^((1/4) * (global_gain - 210 - 8 * subblock_gain[w])) *
//           2^-(multiplier * scalefac_s[sfb][w])
// 
//   where:
//   multiplier = (scalefac_scale + 1) / 2
// 
// The routines Mp3Requantizechannel->Scalefac() and Mp3Requantize() facilitate this
// calculation.


// scalefactor band preemphasis (used only when preflag is set)
static const BYTE PreTable[22] = 
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0
};
static const BYTE PreTableNull[22] = 
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const short ReqRootTable[] = 
{
	0x0983 /* 2^(-3/4) == 0.59460355750136 */,
	0x0b50 /* 2^(-2/4) == 0.70710678118655 */,
	0x0d74 /* 2^(-1/4) == 0.84089641525371 */,
	0x1000 /* 2^( 0/4) == 1.00000000000000 */,
	0x1307 /* 2^(+1/4) == 1.18920711500272 */,
	0x16a1 /* 2^(+2/4) == 1.41421356237310 */,
	0x1ae9 /* 2^(+3/4) == 1.68179283050743 */
};

extern const REQ_FLOAT RequantizeTable[];

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3RequantizeInit(MP3_STREAM *stream)
{
	stream->ReqTable = RequantizeTable;
	stream->ReqExponent = Mp3Requantize;
}



////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3Requantize(int value, int exp, const REQ_FLOAT *power)
{
	DWORD requantized;
	int root;
	int neg;
	int ret;

	if(exp < 0)
	{
		int tmp;
		tmp = -exp;
		root = (int)ReqRootTable[3 - (tmp & 3)];
		exp = -(tmp >> 2);
	}
	else
	{
		root = (int)ReqRootTable[3 + (exp & 3)];
		exp >>= 2;
	}

	neg = 0;
	if(value < 0)
	{
		neg = 1;
		value = -value;
	}

	requantized = power[value];
	exp += power[value] & 0x1f;
	exp -= 17;

	if(exp >= -12)
		requantized = 0x7fff;
	else if(exp >= -31)
		requantized >>= -exp;
	else
		return 0;

	ret = ((int)requantized * root) >> 12;
	if(neg)
		return -ret;
	return ret;
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3RequantizeExponent(MP3_CHSI *channel, int lines)
{
	int gain;
	int multiplier;
	BYTE const *sfbwidth;
	int l;
	const short *scalefac;
	const BYTE *pretbl;
	short *exponent;
	
	multiplier = (channel->Flags & MP3_SCALEFAC_SCALE) ? 2 : 1;
	gain = channel->GlobalGain - 211;
	scalefac = channel->Scalefac;
	exponent = channel->Exponent;
	sfbwidth = channel->SfbWidth;
	l = 0;
	if(channel->Flags & MP3_PREFLAG)
		pretbl = PreTable;
	else
		pretbl = PreTableNull;

	if (channel->BlockType == 2)
	{
		int gain0, gain1, gain2;

		if (channel->Flags & MP3_MIXED_BLOCK)
		{
			while (l < 36)
			{
				*exponent++ = gain - (((*scalefac++) + (*pretbl++)) << multiplier);
				l += *sfbwidth++;
			}
		}
		
		gain0 = gain - (channel->SubblockGain[0] << 3);
		gain1 = gain - (channel->SubblockGain[1] << 3);
		gain2 = gain - (channel->SubblockGain[2] << 3);
		while (l < lines)
		{
			*exponent++ = gain0 - ((*scalefac++) << multiplier);
			*exponent++ = gain1 - ((*scalefac++) << multiplier);
			*exponent++ = gain2 - ((*scalefac++) << multiplier);
			l += *sfbwidth++;
			l += *sfbwidth++;
			l += *sfbwidth++;
		}
	}
	else
	{
		while(l < lines)
		{
			*exponent++ = gain - (((*scalefac++) + (*pretbl++)) << multiplier);
			l += *sfbwidth++;
		}
	}
}

#endif // CONFIG_DECODE_MP3_ENABLE

