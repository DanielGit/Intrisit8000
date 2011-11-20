//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－立体声解码部分
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


extern const SFB_WIDTH SfbWidthTable[9]; 

/*
* coefficients for intensity stereo processing
* derived from section 2.4.3.4.9.3 of ISO/IEC 11172-3
*
* is_ratio[i] = tan(i * (PI / 12))
* is_table[i] = is_ratio[i] / (1 + is_ratio[i])
*/
static short const IsTable[7] = 
{
	CONST_SHORT(0x00000000) /* 0.000000000 */,
		CONST_SHORT(0x0361962f) /* 0.211324865 */,
		CONST_SHORT(0x05db3d74) /* 0.366025404 */,
		CONST_SHORT(0x08000000) /* 0.500000000 */,
		CONST_SHORT(0x0a24c28c) /* 0.633974596 */,
		CONST_SHORT(0x0c9e69d1) /* 0.788675135 */,
		CONST_SHORT(0x10000000) /* 1.000000000 */
};

/*
* coefficients for LSF intensity stereo processing
* derived from section 2.4.3.2 of ISO/IEC 13818-3
*
* is_lsf_table[0][i] = (1 / sqrt(sqrt(2)))^(i + 1)
* is_lsf_table[1][i] = (1 /      sqrt(2)) ^(i + 1)
*/
static short const LsfIsTable[2][15] = 
{
	{
		CONST_SHORT(0x0d744fcd) /* 0.840896415 */,
			CONST_SHORT(0x0b504f33) /* 0.707106781 */,
			CONST_SHORT(0x09837f05) /* 0.594603558 */,
			CONST_SHORT(0x08000000) /* 0.500000000 */,
			CONST_SHORT(0x06ba27e6) /* 0.420448208 */,
			CONST_SHORT(0x05a8279a) /* 0.353553391 */,
			CONST_SHORT(0x04c1bf83) /* 0.297301779 */,
			CONST_SHORT(0x04000000) /* 0.250000000 */,
			CONST_SHORT(0x035d13f3) /* 0.210224104 */,
			CONST_SHORT(0x02d413cd) /* 0.176776695 */,
			CONST_SHORT(0x0260dfc1) /* 0.148650889 */,
			CONST_SHORT(0x02000000) /* 0.125000000 */,
			CONST_SHORT(0x01ae89fa) /* 0.105112052 */,
			CONST_SHORT(0x016a09e6) /* 0.088388348 */,
			CONST_SHORT(0x01306fe1) /* 0.074325445 */
	},
	{
			CONST_SHORT(0x0b504f33) /* 0.707106781 */,
				CONST_SHORT(0x08000000) /* 0.500000000 */,
				CONST_SHORT(0x05a8279a) /* 0.353553391 */,
				CONST_SHORT(0x04000000) /* 0.250000000 */,
				CONST_SHORT(0x02d413cd) /* 0.176776695 */,
				CONST_SHORT(0x02000000) /* 0.125000000 */,
				CONST_SHORT(0x016a09e6) /* 0.088388348 */,
				CONST_SHORT(0x01000000) /* 0.062500000 */,
				CONST_SHORT(0x00b504f3) /* 0.044194174 */,
				CONST_SHORT(0x00800000) /* 0.031250000 */,
				CONST_SHORT(0x005a827a) /* 0.022097087 */,
				CONST_SHORT(0x00400000) /* 0.015625000 */,
				CONST_SHORT(0x002d413d) /* 0.011048543 */,
				CONST_SHORT(0x00200000) /* 0.007812500 */,
				CONST_SHORT(0x0016a09e) /* 0.005524272 */
		}
};



////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3Stereo(MP3_STREAM *stream)
{
	MP3INT *chl, *chr;
	int m, s;
	int sfbi, l;
	const unsigned char *sfbwidth;
	short gr, ngr;
	MP3_CHSI *channell, *channelr;

	// 立体声检查
	if(stream->Header.Mode == MP3_MODE_SINGLECHANNEL)
		return 0;
	ngr = (stream->Header.Mpeg == MP3_MPEG1) ? 2 : 1;
	for(gr=0; gr<ngr; gr++)
	{
		channell = &stream->SideInfo.Channel[gr][0];
		channelr = &stream->SideInfo.Channel[gr][1];
		chl = stream->FreqLine[gr][0];
		chr = stream->FreqLine[gr][1];
		
		/* intensity stereo */
		if (stream->Header.ModeEx & MP3_MODEX_ISTEREO)
		{
			short is_pos, i;
			
			// 处理Intensity部分
			if (stream->Header.Mpeg == MP3_MPEG1)
			{
				sfbwidth = channelr->SfbWidth;
				l = sfbi = 0;
				while(l < stream->DecodeLines)
				{
					if(sfbi < channelr->ZeroSbf)
					{
						l += sfbwidth[sfbi];
						chl += sfbwidth[sfbi];
						chr += sfbwidth[sfbi];
						sfbi++;
						continue;
					}
					is_pos = channelr->Scalefac[sfbi];
					if(is_pos >= 7)
					{
						l += sfbwidth[sfbi];
						chl += l;
						chr += l;
						sfbi++;
						continue;
					}
					
					i = sfbwidth[sfbi];
					sfbi++;
					l += i;
					while(i-- > 0)
					{
						m = *chl;
						*chl++ = IMUL(m, IsTable[is_pos]);
						*chr++ = IMUL(m, IsTable[6 - is_pos]);
					}
				}
			}
			else
			{
				const short *lsf_scale;
				short *illegal_pos;
				
				illegal_pos = stream->SideInfo.Channel[1][1].Scalefac;
				lsf_scale = LsfIsTable[channelr->ScfCompress & 0x1];
				sfbwidth = channelr->SfbWidth;
				l = sfbi = 0;
				while(l < stream->DecodeLines)
				{
					if(sfbi < channelr->ZeroSbf)
					{
						l += sfbwidth[sfbi];
						chl += sfbwidth[sfbi];
						chr += sfbwidth[sfbi];
						sfbi++;
						continue;
					}
					if(!illegal_pos[sfbi])
					{
						l += sfbwidth[sfbi];
						chl += sfbwidth[sfbi];
						chr += sfbwidth[sfbi];
						sfbi++;
						continue;
					}
					
					is_pos = channelr->Scalefac[sfbi];
					i = sfbwidth[sfbi];
					sfbi++;
					l += i;
					while(i-- > 0)
					{
						m = *chl;
						
						if(is_pos == 0)
						{
							*chr = m;
						}
						else
						{
							s = IMUL(m, lsf_scale[(is_pos - 1) >> 1]);
							if (is_pos & 1)
							{
								*chl = s;
								*chr = m;
							}
							else
								*chr = s;
						}
						chl++;
						chr++;
					}
				}
			}
		}

		// 处理MS-STEREO
		chl = stream->FreqLine[gr][0];
		chr = stream->FreqLine[gr][1];
		if (stream->Header.ModeEx & MP3_MODEX_MSSTEREO)
		{
			l = (channelr->ZeroFreqLine > channell->ZeroFreqLine) ?
				channelr->ZeroFreqLine : channell->ZeroFreqLine;
			while(l-- > 0)
			{
				m = *chl;
				s = *chr;
				*chl++ = ((m + s) * 181) >> 8;
				*chr++ = ((m - s) * 181) >> 8;
			}
		}
	}
	return 0;
}


#endif // CONFIG_DECODE_MP3_ENABLE

