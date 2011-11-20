//[PROPERTY]===========================================[PROPERTY]
//            *****   ���Ӵʵ�ƽ̨  *****
//        --------------------------------------
//         	    MP3���룭ͷ��Ϣ������
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
void Mp3HeaderInit(MP3_HEADER *header)
{
	header->Mode = 0;
	header->ModeEx = 0;
	header->Emphasis = 0;
	header->Bitrate = 0;
	header->Samplerate = 0;
	header->Crc = 0;
	header->Flags = 0;
}


////////////////////////////////////////////////////
// ����:  
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static BYTE Mp3HeaderPassId3(MP3_BUF *buf, BYTE synch, short* err)
{
	DWORD id3size;

	if(synch != 'I')
	{
		synch = Mp3BufGetByte(buf, err);
		return synch;
	}
	
	while(synch == 'I')
	{	
		synch = Mp3BufGetByte(buf, err);
		if(synch == 'D')
		{
			synch = Mp3BufGetByte(buf, err);
			if(synch == '3')
			{
				synch = Mp3BufGetByte(buf, err);
				synch = Mp3BufGetByte(buf, err);
				synch = Mp3BufGetByte(buf, err);
				synch = Mp3BufGetByte(buf, err);
				id3size = (((DWORD)synch) << 21);
				synch = Mp3BufGetByte(buf, err);
				id3size += (((DWORD)synch) << 14);
				synch = Mp3BufGetByte(buf, err);
				id3size += (((DWORD)synch) << 7);
				synch = Mp3BufGetByte(buf, err);
				id3size += (((DWORD)synch) << 0);
				
				id3size++;
				while(id3size--)
					synch = Mp3BufGetByte(buf, err);
			}
		}
	}
	return synch;
}

////////////////////////////////////////////////////
// ����:  
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
static int Mp3HeaderCheckTag(MP3_BUF *buf, BYTE synch)
{
	short err;

	err = 0;
	if(synch == 'T')
	{
		synch = Mp3BufGetByte(buf, &err);
		if(synch == 'A')
		{
			synch = Mp3BufGetByte(buf, &err);
			if(synch == 'G')
				return 1;
		}
	}
	return err;
}


////////////////////////////////////////////////////
// ����:
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
int Mp3HeaderDecode(MP3_STREAM *stream)
{
	BYTE synch;
	short synchmax, err;
	MP3_BUF *buf;
	
	// MP3֡ͬ������
	buf = &stream->Buffer;
	synch = Mp3BufGetByte(buf, &err);
	synchmax = 1;
	while(synchmax <= 31*1024)
	{
		while(synch != 0xff)
		{
			// ����Ƿ�ΪTAG
			if( Mp3HeaderCheckTag(buf, synch) != 0 )
			{
				kprintf("mp3 header is 'TAG'\n");
				return -1;
			}
			
			// �Թ�ID3��Ϣ
			synch = Mp3HeaderPassId3(buf, synch, &err);

			// �޶�ͬ���ֽ���
			if((synchmax++ >= 31*1024) || (err < 0))
			{
				if( err < 0 )
					kprintf("mp3 file end\n");
				else
					kprintf("mp3 header decoder error\n");
				return -2;
			}
		}
		synch = Mp3BufGetByte(buf, &err);
		synchmax++;
		if((synch & 0xe0) == 0xe0)
		{
			// ����Ƿ�ΪMPEG1/MPEG2/MPEG2.5 LAYER3
			if(((synch & 0x18) == 0x08) || ((synch & 0x06) != 0x02))
				continue;
			Mp3HeaderInit(&stream->Header);			

			stream->Header.Mpeg = (synch & 0x18) >> 3;
			if(!(synch & 0x01))
				stream->Header.Flags |= MP3_FLAG_PROTECTION;

			// ��ȡ�����ʺͲ�����
			synch = Mp3BufGetByte(buf, &err);
			if(((synch >> 2) & 0x03) == 0x03)
				continue;
			switch(stream->Header.Mpeg)
			{
			case MP3_MPEG25:	// MPEG2.5
				stream->Header.Bitrate = _BitrateTable0[synch >> 4];
				stream->Header.Samplerate = _SamplerateTable[2][(synch >> 2) & 0x03];
				break;
			case MP3_MPEG2:		// MPEG2
				stream->Header.Bitrate = _BitrateTable0[synch >> 4];
				stream->Header.Samplerate = _SamplerateTable[1][(synch >> 2) & 0x03];
				break;
			case MP3_MPEG1:		// MPEG1
				stream->Header.Bitrate = _BitrateTable1[synch >> 4];
				stream->Header.Samplerate = _SamplerateTable[0][(synch >> 2) & 0x03];
				break;
			}
			// ��鲨���ʺͲ������Ƿ���ȷ
			if((stream->Header.Bitrate == 0) || (stream->Header.Samplerate == 0))
				continue;
	
			if(synch & 0x02)
				stream->Header.Flags |= MP3_FLAG_PADDING;
			if(synch & 0x01)
				stream->Header.Flags |= MP3_FLAG_PRIVATEDBIT;

			// ��ȡ������ģʽ
			synch = Mp3BufGetByte(buf, &err);
			stream->Header.Mode = synch >> 6;
			stream->Header.ModeEx = (synch >> 4) & 0x03;
			
			// ��ȡ��Ȩ��Ϣ
			if(synch & 0x08)
				stream->Header.Flags |= MP3_FLAG_COPYRIGHT;
			if(synch & 0x04)
				stream->Header.Flags |= MP3_FLAG_ORIGINAL;

			// ��ȡEmphasis��Ϣ
			stream->Header.Emphasis = synch & 0x03;

			// ��ȡCRC��Ϣ
			if(stream->Header.Flags & MP3_FLAG_PROTECTION)
			{
				stream->Header.Crc = ((WORD)Mp3BufGetByte(buf, &err)) << 8;
				stream->Header.Crc |= (WORD)Mp3BufGetByte(buf, &err);
			}

			// ���ý�ѹLINES
#if defined(CONFIG_MCU_C33L27)
			if((stream->Header.Samplerate == 8000) || (stream->Header.Samplerate == 11025))
				stream->DecodeLines = MP3_DECODE_LINE;
			else
				stream->DecodeLines = MP3_DECODE_LINE/2;
#else
			stream->DecodeLines = MP3_DECODE_LINE;
#endif
			return 0;
		}
	}
	kprintf("is not mp3 header\n");
	return -1;	
}

////////////////////////////////////////////////////
// ����: 
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
DWORD Mp3HeaderBitrate(MP3_HEADER *header)
{
	return header->Bitrate;
}


////////////////////////////////////////////////////
// ����: ��ȡ֡����(�ֽ���)
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
int Mp3HeaderSideinfoLen(MP3_HEADER *header)
{
	if(header->Mpeg == MP3_MPEG1)
	{
		// ����Ϣ�ֽ���
		if(header->Mode == MP3_MODE_SINGLECHANNEL)
			return 17;
		else
			return 32;
	}
	else
	{
		// ����Ϣ�ֽ���
		if(header->Mode == MP3_MODE_SINGLECHANNEL)
			return 9;
		else
			return 17;
	}
}


////////////////////////////////////////////////////
// ����: ��ȡ֡����(�ֽ���)
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
int Mp3HeaderMaindataLen(MP3_HEADER *header)
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

		// ����Ϣ�ֽ���
		if(header->Mode == MP3_MODE_SINGLECHANNEL)
			slots -= 17;
		else
			slots -= 32;
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

		// ����Ϣ�ֽ���
		if(header->Mode == MP3_MODE_SINGLECHANNEL)
			slots -= 9;
		else
			slots -= 17;
	}
	return slots;
}


////////////////////////////////////////////////////
// ����: ��ȡȥ��ͷ��Ϣ���֡����(�ֽ���)
// ����: 
// ���:
// ����: 
////////////////////////////////////////////////////
int Mp3HeaderExpectLen(MP3_HEADER *header)
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


#endif // CONFIG_DECODE_MP3_ENABLE

