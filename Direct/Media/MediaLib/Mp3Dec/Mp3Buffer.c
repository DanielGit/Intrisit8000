//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－缓冲区处理部分
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


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static int Mp3BufGetData(MP3_BUF *mp3buf, BYTE* buf, int size)
{
	int ret;

	ret = mp3buf->pCallback(mp3buf->CallbackId, buf, size);
	if(ret < size)
		kmemset(buf + ret, 0x00, size - ret);
	
	return ret;
}


////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3BufInit(MP3_BUF *buf)
{
	buf->FHead = 0;
	buf->FTail = 0;
	kmemset(buf->FrameBuf, 0x00, MP3_FRAME_BUF_SIZE);

	buf->MTail = 0;
	buf->MHead = 0;
	buf->MainBufRemain = 0;
	buf->MainBufReady = 0;
	kmemset(buf->MainBuf, 0x00, MP3_MAIN_BUF_SIZE);
}


////////////////////////////////////////////////////
// 功能: 读取主数据到Cache中
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3BufMainCache(MP3_BUF *buf, MP3_MCACHE *cache, int BitRemain)
{
	BYTE *p;
	p = &buf->MainBuf[buf->MHead];
	if(!BitRemain)
	{
		cache->Cache.Byte[3] = *p++;
		cache->Cache.Byte[2] = *p++;
		cache->Cache.Byte[1] = *p++;
		cache->Cache.Byte[0] = *p++;
		buf->MHead += 4;
		buf->MHead &= (MP3_MAIN_BUF_SIZE-1);
		return 32;
	}
	else if(BitRemain <= 8)
	{
		cache->Cache.DWord <<= 24;
		cache->Cache.Byte[2] = *p++;
		cache->Cache.Byte[1] = *p++;
		cache->Cache.Byte[0] = *p++;
		buf->MHead += 3;
		buf->MHead &= (MP3_MAIN_BUF_SIZE-1);
		return (BitRemain + 24);
	}
	else
	{
		cache->Cache.DWord <<= 16;
		cache->Cache.Byte[1] = *p++;
		cache->Cache.Byte[0] = *p++;
		buf->MHead += 2;
		buf->MHead &= (MP3_MAIN_BUF_SIZE-1);
		return (BitRemain + 16);
	}
	return BitRemain;
}


////////////////////////////////////////////////////
// 功能: 检查主数据缓冲区中是否有足够的数据供解压
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3BufMainInsuffice(MP3_STREAM *stream, int size)
{
	int ngr, gr;
	int nch, ch;
	int part2_3bits;
	int miss;

	// 检查缓冲区是否进入就绪状态
	if(stream->Buffer.MainBufReady)
		return 1;

	// 获取当前解压所需的字节数
	ngr = (stream->Header.Mpeg == MP3_MPEG1) ? 2 : 1;
	nch = (stream->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
	part2_3bits = 0;
	for (gr = 0; gr < ngr; gr++)
	{
		for (ch = 0; ch < nch; ++ch)
		{
			MP3_CHSI *channel = &stream->SideInfo.Channel[gr][ch];
			part2_3bits += channel->Part23Len;
		}
	}
	part2_3bits = (part2_3bits + 7) / 8;

	// 检查主缓冲区中的数据是否足够解压
	miss = stream->SideInfo.MainDataBegin - (stream->Buffer.MainBufRemain - size);
	if(miss <= 0)
	{
		stream->Buffer.MainBufReady = 1;
		return 1;
	}
	miss -= part2_3bits;

	// 检查主缓冲区中的数据是否足够解压
	stream->Buffer.MainBufRemain -= miss;
	return 0;
}


////////////////////////////////////////////////////
// 功能: 获取向MainDataBuf中写入数据的地址
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3BufMainHead(MP3_BUF *buf, WORD offset)
{
	if(buf->MHead >= offset)
		buf->MHead = buf->MHead - offset;
	else
		buf->MHead = MP3_MAIN_BUF_SIZE + buf->MHead - offset;
}


////////////////////////////////////////////////////
// 功能: 读取数据到Cache中
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3BufCache(MP3_BUF *buf)
{
	if(buf->BitRemain <= 8)
	{
		buf->Cache.DWord <<= 24;
		if(buf->FHead == MP3_FRAME_BUF_SIZE)
		{
			buf->FHead = 0;
			buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
			if(buf->FTail < 0)
				buf->FTail = 0;
		}
		buf->Cache.Byte[2] = buf->FrameBuf[buf->FHead++];
		if(buf->FHead == MP3_FRAME_BUF_SIZE)
		{
			buf->FHead = 0;
			buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
			if(buf->FTail < 0)
				buf->FTail = 0;
		}
		buf->Cache.Byte[1] = buf->FrameBuf[buf->FHead++];
		if(buf->FHead == MP3_FRAME_BUF_SIZE)
		{
			buf->FHead = 0;
			buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
			if(buf->FTail < 0)
				buf->FTail = 0;
		}
		buf->Cache.Byte[0] = buf->FrameBuf[buf->FHead++];
		buf->BitRemain += 24;
	}
	else
	{
		buf->Cache.DWord <<= 16;
		if(buf->FHead == MP3_FRAME_BUF_SIZE)
		{
			buf->FHead = 0;
			buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
			if(buf->FTail < 0)
				buf->FTail = 0;
		}
		buf->Cache.Byte[1] = buf->FrameBuf[buf->FHead++];
		if(buf->FHead == MP3_FRAME_BUF_SIZE)
		{
			buf->FHead = 0;
			buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
			if(buf->FTail < 0)
				buf->FTail = 0;
		}
		buf->Cache.Byte[0] = buf->FrameBuf[buf->FHead++];
		buf->BitRemain += 16;
	}
}


////////////////////////////////////////////////////
// 功能: 把帧缓冲区的数据保存到主数据缓冲区
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3BufMainSave(MP3_BUF *buf, WORD size)
{
	WORD srcsize, dstsize;
	WORD transize;

	buf->MHead = buf->MTail;
	buf->MainBufRemain += size;
	if(buf->BitRemain >= 24)
	{
		buf->MainBuf[buf->MTail++] = buf->Cache.Byte[2];
		buf->MTail &= (MP3_MAIN_BUF_SIZE - 1);
		buf->MainBuf[buf->MTail++] = buf->Cache.Byte[1];
		buf->MTail &= (MP3_MAIN_BUF_SIZE - 1);
		buf->MainBuf[buf->MTail++] = buf->Cache.Byte[0];
		buf->MTail &= (MP3_MAIN_BUF_SIZE - 1);
		size -= 3;
	}
	else if(buf->BitRemain >= 16)
	{
		buf->MainBuf[buf->MTail++] = buf->Cache.Byte[1];
		buf->MTail &= (MP3_MAIN_BUF_SIZE - 1);
		buf->MainBuf[buf->MTail++] = buf->Cache.Byte[0];
		buf->MTail &= (MP3_MAIN_BUF_SIZE - 1);
		size -= 2;
	}
	else if(buf->BitRemain >= 8)
	{
		buf->MainBuf[buf->MTail++] = buf->Cache.Byte[0];
		buf->MTail &= (MP3_MAIN_BUF_SIZE - 1);
		size -= 1;
	}
	while(size)
	{
		// 检查目的缓冲区数据情况
		if(buf->FHead == buf->FTail)
		{
			buf->FHead = 0;
			buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
			if(buf->FTail <= 0)
			{
				buf->FTail = 0;
				return;
			}
		}
		dstsize = MP3_MAIN_BUF_SIZE - buf->MTail;		// 获取目的缓冲区大小
		srcsize = buf->FTail - buf->FHead;				// 获取源缓冲区大小
		transize = (dstsize > srcsize) ? srcsize : dstsize;
		transize = (transize > size) ? size : transize;
		if(!transize)
			break;
		kmemcpy(&buf->MainBuf[buf->MTail], &buf->FrameBuf[buf->FHead], transize);
		buf->MTail += transize;
		buf->MTail &= (MP3_MAIN_BUF_SIZE-1);
		buf->FHead += transize;
		size -= transize;
	}
	buf->MainBuf[MP3_MAIN_BUF_SIZE+0] = buf->MainBuf[0];
	buf->MainBuf[MP3_MAIN_BUF_SIZE+1] = buf->MainBuf[1];
	buf->MainBuf[MP3_MAIN_BUF_SIZE+2] = buf->MainBuf[2];
	buf->MainBuf[MP3_MAIN_BUF_SIZE+3] = buf->MainBuf[3];
}



////////////////////////////////////////////////////
// 功能: 把帧缓冲区的数据保存到主数据缓冲区
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3BufMainSavePass(MP3_BUF *buf, WORD size)
{
	WORD passsize;

	while(size)
	{
		// 检查目的缓冲区数据情况
		if(buf->FHead == buf->FTail)
		{
			buf->FHead = 0;
			buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
			if(buf->FTail <= 0)
			{
				buf->FTail = 0;
				return;
			}
		}
		// 略过数据
		passsize = buf->FTail - buf->FHead;
		passsize = (size > passsize) ? passsize : size;
		if(!passsize)
			break;
		buf->FHead += passsize;
		size -= passsize;
	}

}


////////////////////////////////////////////////////
// 功能: 从帧缓冲区中读取一个字节数据
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
BYTE Mp3BufGetByte(MP3_BUF *buf, short *err)
{
	if(buf->FHead >= buf->FTail)
	{
		buf->FHead = 0;
		buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
		if(buf->FTail <= 0)
		{
			*err = -1;
			buf->FTail = 0;
			return 0;
		}
	}
	*err = 0;
	return buf->FrameBuf[buf->FHead++];
}

////////////////////////////////////////////////////
// 功能: 从帧缓冲区中读取一个字节数据
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
BYTE __Mp3BufGetByte(MP3_BUF *buf, int offset, short *err)
{
	if(buf->FHead >= buf->FTail)
	{
		buf->FHead = 0;
		buf->FTail = Mp3BufGetData(buf, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
		if(buf->FTail <= 0)
		{
			*err = -1;
			buf->FTail = 0;
			return 0;
		}
	}
	*err = 0;
	return buf->FrameBuf[buf->FHead+offset];
}


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
WORD Mp3BufGetBits(MP3_BUF *buf, BYTE bits)
{
	WORD ret;
	if(buf->BitRemain < bits)
		Mp3BufCache(buf);
	buf->BitRemain -= bits;
	ret = (WORD)(buf->Cache.DWord >> buf->BitRemain);
	ret &= ~((0xffffffffL << bits));
	return ret;
}														


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3BufCacheReset(MP3_BUF *buf)
{
	buf->BitRemain = 0;
	buf->Cache.DWord = 0;
}


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void Mp3BufMainPass(MP3_BUF *buf, WORD size)
{
	buf->MHead += size;
	buf->MHead &= (MP3_MAIN_BUF_SIZE - 1);
}


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3BufGetRemain(MP3_BUF *buf)
{
	return buf->FTail - buf->FHead;
}


#endif // CONFIG_DECODE_MP3_ENABLE

