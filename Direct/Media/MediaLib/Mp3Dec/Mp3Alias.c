//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－重排序和反混叠部分
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

extern const REORD_TABLE Mp3ReorderTable[];


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3AliasReduce(MP3_STREAM *stream)
{
	const short *order;
	short gr, ngr, ch, nch;
	short l, el;
	
	nch = (stream->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
	ngr = (stream->Header.Mpeg == MP3_MPEG1) ? 2 : 1;
	for(gr=0; gr<ngr; gr++)
	{
		for(ch=0; ch<nch; ch++)
		{
			MP3_CHSI *channel;
			MP3INT *xch, *ptmp;
			
			channel = &stream->SideInfo.Channel[gr][ch];
			xch = stream->FreqLine[gr][ch];
			el = stream->DecodeLines;
			
			// 短块进行重排序
			if(channel->BlockType == 2)
			{
				order = Mp3ReorderTable[stream->ScfbSelect].s;
				if(channel->Flags & MP3_MIXED_BLOCK)
				{
					order = Mp3ReorderTable[stream->ScfbSelect].m;
					order += 36;
					ptmp = &stream->Alias[36];
					for(l = 36; l<el; )
					{
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						l += 6;
					}
					ptmp = &stream->Alias[36];
					for(l = 36; l<el; )
					{
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						l += 6;
					}
				}
				else
				{
					l = 0;
					ptmp = stream->Alias;
					while(l < el)
					{
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						*ptmp++ = xch[*order++];
						l += 6;
					}
					ptmp = stream->Alias;
					for(l = 0; l<el; )
					{
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						*xch++ = *ptmp++;
						l += 6;
					}
				}
			}
			// 长块进行混叠降低运算
			else
			{
				MP3INT *xi, *xd;
				int tmpi, tmpd;

				l = 18;
				while(l < el)
				{
					xi = xch + 17;
					xd = xch + 18;
					tmpi = IMULNS(*xi, (CONST_SHORT(0x0db84a81)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x083b5fe7)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x083b5fe7)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0db84a81)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);

					tmpi = IMULNS(*xi, (CONST_SHORT(0x0e1b9d7f)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x078c36d2)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x078c36d2)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0e1b9d7f)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);

					tmpi = IMULNS(*xi, (CONST_SHORT(0x0f31adcf)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x05039814)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x05039814)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0f31adcf)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);

					tmpi = IMULNS(*xi, (CONST_SHORT(0x0fbba815)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x02e91dd1)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x02e91dd1)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0fbba815)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);
					
					tmpi = IMULNS(*xi, (CONST_SHORT(0x0feda417)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x0183603a)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x0183603a)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0feda417)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);

					tmpi = IMULNS(*xi, (CONST_SHORT(0x0ffc8fc8)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x00a7cb87)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x00a7cb87)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0ffc8fc8)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);

					tmpi = IMULNS(*xi, (CONST_SHORT(0x0fff964c)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x003a2847)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x003a2847)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0fff964c)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);

					tmpi = IMULNS(*xi, (CONST_SHORT(0x0ffff8d3)));
					tmpi += IMULNS(*xd, (CONST_SHORT(0x000f27b4)));
					tmpd = IMULNS(*xi, (-CONST_SHORT(0x000f27b4)));
					tmpd += IMULNS(*xd, (CONST_SHORT(0x0ffff8d3)));
					*xi-- = IMULSHIFT(tmpi);
					*xd++ = IMULSHIFT(tmpd);

					l += 18;
					xch += 18;
				}
			}
		}
	}
}

#endif // CONFIG_DECODE_MP3_ENABLE

