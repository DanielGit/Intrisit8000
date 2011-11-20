//[PROPERTY]===========================================[PROPERTY]
//            *****   ���Ӵʵ�ƽ̨  *****
//        --------------------------------------
//       	        MP3����ʱ��Ԥ��
//        --------------------------------------
//                 ��Ȩ: ��ŵ���ۿƼ�
//             ---------------------------
//                  ��   ��   ��   ʷ
//        --------------------------------------
//  �汾    ��ǰ		˵��		
//  V0.1    2006-3      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]

#include "Mp3Prv.h"

#ifdef CONFIG_DECODE_MP3_ENABLE

typedef struct _MP3_TIME_CALC
{
	MP3_BUF Buffer;
	MP3_HEADER Header;
	int ReadSize;
}MP3_TIME_CALC;

#define MP3_MPEG1					0x03
#define MP3_MPEG2					0x02
#define MP3_MPEG25					0x00

#define MP3_FLAG_PRIVATEDBIT		0x0001
#define MP3_FLAG_FREEFORMAT			0x0008
#define MP3_FLAG_PROTECTION			0x0010  	// frame has CRC protection 
#define MP3_FLAG_COPYRIGHT			0x0020  	// frame is copyright 
#define MP3_FLAG_ORIGINAL			0x0040  	// frame is original (else copy) 
#define MP3_FLAG_PADDING			0x0080  	// frame has additional slot 

#define MP3_MODE_SINGLECHANNEL		0x03

static const DWORD _BitrateTable0[] =
{
	0,     8000,  16000, 24000,  32000,  40000,  48000, 56000,
		64000, 80000, 96000, 112000, 128000, 144000, 160000
};

static const DWORD _BitrateTable1[] =
{
	0,      32000,  40000,  48000, 56000,   64000,  80000, 96000,
		112000, 128000, 160000, 192000, 224000, 256000, 320000 
		
};

static const DWORD _SamplerateTable[3][3] =
{
	{44100, 48000, 32000},
	{22050, 24000, 16000},
	{11025, 12000, 8000 }
};

////////////////////////////////////////////////////
// ����:
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static int _Mp3FileBufGetData(MP3_TIME_CALC *Mp3TimeCalc, BYTE* buf, int size)
{
	int ret;

	ret = Mp3TimeCalc->Buffer.pCallback(Mp3TimeCalc->Buffer.CallbackId,buf,size);
	if(ret < size)
		kmemset(buf + ret, 0x00, size - ret);
	Mp3TimeCalc->ReadSize += ret;

	return ret;
}


////////////////////////////////////////////////////
// ����: ��֡�������ж�ȡһ���ֽ�����
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static BYTE _Mp3FileBufGetByte(MP3_TIME_CALC *Mp3TimeCalc, short *err)
{
	MP3_BUF *buf;

	buf = &Mp3TimeCalc->Buffer;
	if(buf->FHead >= buf->FTail)
	{
		buf->FHead = 0;
		buf->FTail = _Mp3FileBufGetData(Mp3TimeCalc, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
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
// ����: ��֡�����������ݱ��浽�����ݻ�����
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static void _Mp3FileBufPass(MP3_TIME_CALC *Mp3TimeCalc, WORD size)
{
	WORD passsize;
	MP3_BUF *buf;
	
	buf = &Mp3TimeCalc->Buffer;
	while(size)
	{
		// ���Ŀ�Ļ������������
		if(buf->FHead == buf->FTail)
		{
			buf->FHead = 0;
			buf->FTail = _Mp3FileBufGetData(Mp3TimeCalc, buf->FrameBuf, MP3_FRAME_BUF_SIZE);
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
// ����:
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
void SkipMp3File(MP3_TIME_CALC *Mp3TimeCalc, int n)
{
	short err;
	while(n--)
		_Mp3FileBufGetByte(Mp3TimeCalc, &err);
}

////////////////////////////////////////////////////
// ����:
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static int _Mp3FileHeaderDecode(MP3_TIME_CALC *Mp3TimeCalc, int *first_frame)
{
	BYTE synch;
	short synchmax, err;
	int ret = -1;
	
	// MP3֡ͬ������
	synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
	synchmax = 1;
	while(synchmax <= 31*1024)
	{
		while(synch != 0xff)
		{
			// �޷��ҵ�ͬ����
			synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
			if((synchmax++ >= 31*1024) || (err < 0))
				return -1;
		}
		synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
		synchmax++;
		if((synch & 0xe0) == 0xe0)
		{
			// ����Ƿ�ΪMPEG1/MPEG2/MPEG2.5 LAYER3
			if(((synch & 0x18) == 0x08) || ((synch & 0x06) != 0x02))
				continue;
			
			kmemset(&Mp3TimeCalc->Header, 0, sizeof(MP3_HEADER));
			Mp3TimeCalc->Header.Mpeg = (synch & 0x18) >> 3;
			if(!(synch & 0x01))
				Mp3TimeCalc->Header.Flags |= MP3_FLAG_PROTECTION;
			
			// ��ȡ�����ʺͲ�����
			synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
			if(((synch >> 2) & 0x03) == 0x03)
				continue;
			switch(Mp3TimeCalc->Header.Mpeg)
			{
			case MP3_MPEG25:	// MPEG2.5
				Mp3TimeCalc->Header.Bitrate = _BitrateTable0[synch >> 4];
				Mp3TimeCalc->Header.Samplerate = _SamplerateTable[2][(synch >> 2) & 0x03];
				break;
			case MP3_MPEG2:		// MPEG2
				Mp3TimeCalc->Header.Bitrate = _BitrateTable0[synch >> 4];
				Mp3TimeCalc->Header.Samplerate = _SamplerateTable[1][(synch >> 2) & 0x03];
				break;
			case MP3_MPEG1:		// MPEG1
				Mp3TimeCalc->Header.Bitrate = _BitrateTable1[synch >> 4];
				Mp3TimeCalc->Header.Samplerate = _SamplerateTable[0][(synch >> 2) & 0x03];
				break;
			}

			// ��鲨���ʺͲ������Ƿ���ȷ
			if((Mp3TimeCalc->Header.Bitrate == 0) || (Mp3TimeCalc->Header.Samplerate == 0))
				continue;
			
			if(synch & 0x02)
				Mp3TimeCalc->Header.Flags |= MP3_FLAG_PADDING;
			if(synch & 0x01)
				Mp3TimeCalc->Header.Flags |= MP3_FLAG_PRIVATEDBIT;

			// ��ȡ������ģʽ
			synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
			Mp3TimeCalc->Header.Mode = synch >> 6;
			Mp3TimeCalc->Header.ModeEx = (synch >> 4) & 0x03;
			
			// ��ȡCRC��Ϣ
			if(Mp3TimeCalc->Header.Flags & 0x0010)
			{
				Mp3TimeCalc->Header.Crc = ((WORD)_Mp3FileBufGetByte(Mp3TimeCalc, &err)) << 8;
				Mp3TimeCalc->Header.Crc |= (WORD)_Mp3FileBufGetByte(Mp3TimeCalc, &err);
			}
			ret = 0;
			break;
		}
	}
	//if it is first frame && find the "xing" string, take it
	//as vbr, get the total frames of file
	if(*first_frame == 1 && ret == 0)
	{
		BYTE tmp[4], xing[] = "xing";
		int i, j, offset;
		//"xing" may appear at 36, 21 and 13 offset of the first frame
		offset = 13 - 4;
		j = 0;
xing_again:
		SkipMp3File(Mp3TimeCalc, offset);
		for(i = 0; i < 4; i++)
		{
			tmp[i] = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
		}
		for(i = 0; i < 4; i++)
		{
			if(ktolower(tmp[i]) != xing[i])
			{
				if(j == 0)
				{
					offset = 21 - 13 - 4;
					j++;
				}
				else if(j == 1)
				{
					offset = 36 - 21 - 4;
					j++;
				}
				else
				{
					*first_frame = 0;
					return 0;
				}
				goto xing_again;
			}
		}
		if( i >= 4)
		{
			offset = 44 - 36 - 4;
			SkipMp3File(Mp3TimeCalc, offset);
			*first_frame = (int)_Mp3FileBufGetByte(Mp3TimeCalc, &err) << 24;
			*first_frame |= (int)_Mp3FileBufGetByte(Mp3TimeCalc, &err) << 16;
			*first_frame |= (int)_Mp3FileBufGetByte(Mp3TimeCalc, &err) << 8;
			*first_frame |= (int)_Mp3FileBufGetByte(Mp3TimeCalc, &err);
		}
	}
	return ret;	
}


////////////////////////////////////////////////////
// ����:  
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static int _Mp3FilePassMp3Id3(MP3_TIME_CALC *Mp3TimeCalc)
{
	char header[10];
	DWORD id3size;
	
	// ��ȡͷ��Ϣ
	if( Mp3TimeCalc->Buffer.pCallback(Mp3TimeCalc->Buffer.CallbackId,(BYTE*)header,10) != 10)
		return 0;

	// ����ͷ��Ϣ
	if(kstrncmp(header, "ID3", 3) != 0)
	{
		Mp3TimeCalc->Buffer.pCallback(Mp3TimeCalc->Buffer.CallbackId,0,-10);
		return 1;
	}
	
	// ��ȡID3���ݴ�С
	id3size = (((DWORD)header[6]) << 21);
	id3size += (((DWORD)header[7]) << 14);
	id3size += (((DWORD)header[8]) << 7);
	id3size += (((DWORD)header[9]) << 0);
	
	// ����ID3ͷ��Ϣ
	Mp3TimeCalc->Buffer.pCallback(Mp3TimeCalc->Buffer.CallbackId,0,id3size);
	return id3size + 10;
}


////////////////////////////////////////////////////
// ����: ��ȡȥ��ͷ��Ϣ���֡����(�ֽ���)
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static int _Mp3FileHeaderLen(MP3_HEADER *header)
{
	int padslot, slots;
	
	padslot = (header->Flags & MP3_FLAG_PADDING) ? 1 : 0;
	if(header->Mpeg == MP3_MPEG1)
	{
		// MP3֡����
		slots = (144 * header->Bitrate / header->Samplerate) + padslot;
		
		// ͷ�ֽ���
		slots -= 4;
		
		// CRC�ֽ���
		if(header->Flags & MP3_FLAG_PROTECTION)
			slots -= 2;
	}
	else
	{
		// MP3֡����
		slots = (72 * header->Bitrate / header->Samplerate) + padslot;
		
		// ͷ�ֽ���
		slots -= 4;
		
		// CRC�ֽ���
		if(header->Flags & MP3_FLAG_PROTECTION)
			slots -= 2;
	}
	return slots;
}


////////////////////////////////////////////////////
// ����:  ����MP3�ļ�����ʱ��
// ����: 
// ���:
// ����:  ʱ��(��λms)
////////////////////////////////////////////////////
int GetMp3PlayTime(MP3_INFO* Info, void *callback, DWORD id,int estimate,DWORD file_size)
{
	int fsize = 0;
	int totaltime;
	int deframes;
	MP3_TIME_CALC *Mp3TimeCalc;
	int avg_bitrate;
	int frame_cnt = 1;

	// �����ڴ�
	Mp3TimeCalc = (MP3_TIME_CALC *)kmalloc(sizeof(MP3_TIME_CALC));
	if(Mp3TimeCalc == NULL)
	{
		kprintf("GetMp3PlayTime malloc failed\n");
		return -1;
	}
	kmemset(Mp3TimeCalc, 0x00, sizeof(MP3_TIME_CALC));
	Mp3TimeCalc->Buffer.CallbackId = id;
	Mp3TimeCalc->Buffer.pCallback = (MP3_DATACALLBACK)callback;

	// ���ļ�
	if(estimate)
		fsize = file_size;

	if( _Mp3FilePassMp3Id3(Mp3TimeCalc) == 0 )
	{
		kprintf("id3 read error\n");
		kfree(Mp3TimeCalc);
		return 0;
	}

	// ��ѹͷ��Ϣ������ָ����ѹ֡����ʱ��
	totaltime = 0;
	deframes = 0;
	avg_bitrate = 0;
	while(1)
	{
		// ����Ƿ��˳�ѭ��
		if(estimate && (deframes >= estimate))
			break;

		// ͬ��֡ͷ�����һ�ȡͷ��Ϣ
		if(_Mp3FileHeaderDecode(Mp3TimeCalc, &frame_cnt) < 0)
			break;

		// ��ȡ��������Ϣ����ȡMP3�����ݵ��仺����
		if(Mp3TimeCalc->Header.Bitrate != 0)
		{
			int size;
			size = _Mp3FileHeaderLen(&Mp3TimeCalc->Header);
			_Mp3FileBufPass(Mp3TimeCalc, size);
		}

		// ��ȡһ֡�ĵ�λʱ��, ��ȡMP3��Ϣ
		avg_bitrate += Mp3TimeCalc->Header.Bitrate;
		if( deframes == 0 )
		{
			Info->AudioCodec      = Mp3TimeCalc->Header.Mpeg;
			Info->AudioBitrate    = Mp3TimeCalc->Header.Bitrate;
			Info->AudioSamplerate = Mp3TimeCalc->Header.Samplerate;
			Info->AudioChannels   = (Mp3TimeCalc->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
		}

		if(++deframes == 5)
		{
			if(Mp3TimeCalc->Header.Mpeg == MP3_MPEG1)
				totaltime = (36 << 5) * 1000 / Mp3TimeCalc->Header.Samplerate; 
			else
				totaltime = (18 << 5) * 1000 / Mp3TimeCalc->Header.Samplerate;

			Info->AudioCodec      = Mp3TimeCalc->Header.Mpeg;
			Info->AudioBitrate    = Mp3TimeCalc->Header.Bitrate;
			Info->AudioSamplerate = Mp3TimeCalc->Header.Samplerate;
			Info->AudioChannels   = (Mp3TimeCalc->Header.Mode == MP3_MODE_SINGLECHANNEL) ? 1 : 2;
		}
	}

	if( deframes < 5 )
	{
		//����ƽ��������
		avg_bitrate /= deframes;

		totaltime = 26 * deframes;
	}
	else if( deframes < estimate )
	{
		//����ƽ��������
		avg_bitrate /= deframes;

		totaltime = totaltime * deframes;
	}
	else
	{
		//����ƽ��������
		avg_bitrate /= deframes;

		// �����ܵĲ���ʱ��
		if(frame_cnt != 0 && frame_cnt != 1)
			totaltime *= frame_cnt;			//����XING��ǩ������ֱ�Ӷ�ȡ���ܵ�fram����
		else
		{	//��XING��ǩ����������
			totaltime *= deframes;
			if(estimate)
			{
				int size;

				//�õ�һ������ݳ���
				size = Mp3TimeCalc->ReadSize - (Mp3TimeCalc->Buffer.FTail - Mp3TimeCalc->Buffer.FHead);
				if(size)
					totaltime = totaltime * (fsize / size);
				else
					totaltime = 0;
			}
		}
	}


	//������������
	Info->TotalTime       = totaltime;

	kfree(Mp3TimeCalc);
	return totaltime;
}
#endif // DECODE_MP3_ENABLE
