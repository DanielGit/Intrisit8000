//[PROPERTY]===========================================[PROPERTY]
//            *****   ���Ӵʵ�ƽ̨  *****
//        --------------------------------------
//         	    MP3���룭������������
//        --------------------------------------
//                 ��Ȩ: ��ŵ���ۿƼ�
//             ---------------------------
//                  ��   ��   ��   ʷ
//        --------------------------------------
//  �汾    ��ǰ		˵��		
//  V0.1    2005-8      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]

#include "Mp3Prv.h"

#ifdef CONFIG_DECODE_MP3_ENABLE


////////////////////////////////////////////////////
// ����:
// ����: 
// ���:
// ����: 
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
// ����:
// ����: 
// ���:
// ����: 
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
// ����: ��ȡ�����ݵ�Cache��
// ����: 
// ���:
// ����: 
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
// ����: ��������ݻ��������Ƿ����㹻�����ݹ���ѹ
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
int Mp3BufMainInsuffice(MP3_STREAM *stream, int size)
{
	int ngr, gr;
	int nch, ch;
	int part2_3bits;
	int miss;

	// ��黺�����Ƿ�������״̬
	if(stream->Buffer.MainBufReady)
		return 1;

	// ��ȡ��ǰ��ѹ������ֽ���
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

	// ������������е������Ƿ��㹻��ѹ
	miss = stream->SideInfo.MainDataBegin - (stream->Buffer.MainBufRemain - size);
	if(miss <= 0)
	{
		stream->Buffer.MainBufReady = 1;
		return 1;
	}
	miss -= part2_3bits;

	// ������������е������Ƿ��㹻��ѹ
	stream->Buffer.MainBufRemain -= miss;
	return 0;
}


////////////////////////////////////////////////////
// ����: ��ȡ��MainDataBuf��д�����ݵĵ�ַ
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
void Mp3BufMainHead(MP3_BUF *buf, WORD offset)
{
	if(buf->MHead >= offset)
		buf->MHead = buf->MHead - offset;
	else
		buf->MHead = MP3_MAIN_BUF_SIZE + buf->MHead - offset;
}


////////////////////////////////////////////////////
// ����: ��ȡ���ݵ�Cache��
// ����: 
// ���:
// ����: 
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
// ����: ��֡�����������ݱ��浽�����ݻ�����
// ����: 
// ���:
// ����: 
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
		// ���Ŀ�Ļ������������
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
		dstsize = MP3_MAIN_BUF_SIZE - buf->MTail;		// ��ȡĿ�Ļ�������С
		srcsize = buf->FTail - buf->FHead;				// ��ȡԴ��������С
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
// ����: ��֡�����������ݱ��浽�����ݻ�����
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
void Mp3BufMainSavePass(MP3_BUF *buf, WORD size)
{
	WORD passsize;

	while(size)
	{
		// ���Ŀ�Ļ������������
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
		// �Թ�����
		passsize = buf->FTail - buf->FHead;
		passsize = (size > passsize) ? passsize : size;
		if(!passsize)
			break;
		buf->FHead += passsize;
		size -= passsize;
	}

}


////////////////////////////////////////////////////
// ����: ��֡�������ж�ȡһ���ֽ�����
// ����: 
// ���:
// ����: 
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
// ����: ��֡�������ж�ȡһ���ֽ�����
// ����: 
// ���:
// ����: 
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
// ����: 
// ����: 
// ���:
// ����: 
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
// ����: 
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
void Mp3BufCacheReset(MP3_BUF *buf)
{
	buf->BitRemain = 0;
	buf->Cache.DWord = 0;
}


////////////////////////////////////////////////////
// ����: 
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
void Mp3BufMainPass(MP3_BUF *buf, WORD size)
{
	buf->MHead += size;
	buf->MHead &= (MP3_MAIN_BUF_SIZE - 1);
}


////////////////////////////////////////////////////
// ����: 
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
int Mp3BufGetRemain(MP3_BUF *buf)
{
	return buf->FTail - buf->FHead;
}


#endif // CONFIG_DECODE_MP3_ENABLE

