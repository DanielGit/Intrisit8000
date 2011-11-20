//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//       	        MP3播放时间预估
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
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
// 功能:
// 输入: 
// 输出:
// 返回: 
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
// 功能: 从帧缓冲区中读取一个字节数据
// 输入: 
// 输出:
// 返回: 
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
// 功能: 把帧缓冲区的数据保存到主数据缓冲区
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static void _Mp3FileBufPass(MP3_TIME_CALC *Mp3TimeCalc, WORD size)
{
	WORD passsize;
	MP3_BUF *buf;
	
	buf = &Mp3TimeCalc->Buffer;
	while(size)
	{
		// 检查目的缓冲区数据情况
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
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
void SkipMp3File(MP3_TIME_CALC *Mp3TimeCalc, int n)
{
	short err;
	while(n--)
		_Mp3FileBufGetByte(Mp3TimeCalc, &err);
}

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static int _Mp3FileHeaderDecode(MP3_TIME_CALC *Mp3TimeCalc, int *first_frame)
{
	BYTE synch;
	short synchmax, err;
	int ret = -1;
	
	// MP3帧同步处理
	synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
	synchmax = 1;
	while(synchmax <= 31*1024)
	{
		while(synch != 0xff)
		{
			// 无法找到同步字
			synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
			if((synchmax++ >= 31*1024) || (err < 0))
				return -1;
		}
		synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
		synchmax++;
		if((synch & 0xe0) == 0xe0)
		{
			// 检查是否为MPEG1/MPEG2/MPEG2.5 LAYER3
			if(((synch & 0x18) == 0x08) || ((synch & 0x06) != 0x02))
				continue;
			
			kmemset(&Mp3TimeCalc->Header, 0, sizeof(MP3_HEADER));
			Mp3TimeCalc->Header.Mpeg = (synch & 0x18) >> 3;
			if(!(synch & 0x01))
				Mp3TimeCalc->Header.Flags |= MP3_FLAG_PROTECTION;
			
			// 获取波特率和采样率
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

			// 检查波特率和采样率是否正确
			if((Mp3TimeCalc->Header.Bitrate == 0) || (Mp3TimeCalc->Header.Samplerate == 0))
				continue;
			
			if(synch & 0x02)
				Mp3TimeCalc->Header.Flags |= MP3_FLAG_PADDING;
			if(synch & 0x01)
				Mp3TimeCalc->Header.Flags |= MP3_FLAG_PRIVATEDBIT;

			// 获取立体声模式
			synch = _Mp3FileBufGetByte(Mp3TimeCalc, &err);
			Mp3TimeCalc->Header.Mode = synch >> 6;
			Mp3TimeCalc->Header.ModeEx = (synch >> 4) & 0x03;
			
			// 获取CRC信息
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
// 功能:  
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static int _Mp3FilePassMp3Id3(MP3_TIME_CALC *Mp3TimeCalc)
{
	char header[10];
	DWORD id3size;
	
	// 读取头信息
	if( Mp3TimeCalc->Buffer.pCallback(Mp3TimeCalc->Buffer.CallbackId,(BYTE*)header,10) != 10)
		return 0;

	// 分析头信息
	if(kstrncmp(header, "ID3", 3) != 0)
	{
		Mp3TimeCalc->Buffer.pCallback(Mp3TimeCalc->Buffer.CallbackId,0,-10);
		return 1;
	}
	
	// 获取ID3数据大小
	id3size = (((DWORD)header[6]) << 21);
	id3size += (((DWORD)header[7]) << 14);
	id3size += (((DWORD)header[8]) << 7);
	id3size += (((DWORD)header[9]) << 0);
	
	// 跳过ID3头信息
	Mp3TimeCalc->Buffer.pCallback(Mp3TimeCalc->Buffer.CallbackId,0,id3size);
	return id3size + 10;
}


////////////////////////////////////////////////////
// 功能: 获取去除头信息后的帧长度(字节数)
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static int _Mp3FileHeaderLen(MP3_HEADER *header)
{
	int padslot, slots;
	
	padslot = (header->Flags & MP3_FLAG_PADDING) ? 1 : 0;
	if(header->Mpeg == MP3_MPEG1)
	{
		// MP3帧长度
		slots = (144 * header->Bitrate / header->Samplerate) + padslot;
		
		// 头字节数
		slots -= 4;
		
		// CRC字节数
		if(header->Flags & MP3_FLAG_PROTECTION)
			slots -= 2;
	}
	else
	{
		// MP3帧长度
		slots = (72 * header->Bitrate / header->Samplerate) + padslot;
		
		// 头字节数
		slots -= 4;
		
		// CRC字节数
		if(header->Flags & MP3_FLAG_PROTECTION)
			slots -= 2;
	}
	return slots;
}


////////////////////////////////////////////////////
// 功能:  计算MP3文件播放时间
// 输入: 
// 输出:
// 返回:  时间(单位ms)
////////////////////////////////////////////////////
int GetMp3PlayTime(MP3_INFO* Info, void *callback, DWORD id,int estimate,DWORD file_size)
{
	int fsize = 0;
	int totaltime;
	int deframes;
	MP3_TIME_CALC *Mp3TimeCalc;
	int avg_bitrate;
	int frame_cnt = 1;

	// 申请内存
	Mp3TimeCalc = (MP3_TIME_CALC *)kmalloc(sizeof(MP3_TIME_CALC));
	if(Mp3TimeCalc == NULL)
	{
		kprintf("GetMp3PlayTime malloc failed\n");
		return -1;
	}
	kmemset(Mp3TimeCalc, 0x00, sizeof(MP3_TIME_CALC));
	Mp3TimeCalc->Buffer.CallbackId = id;
	Mp3TimeCalc->Buffer.pCallback = (MP3_DATACALLBACK)callback;

	// 打开文件
	if(estimate)
		fsize = file_size;

	if( _Mp3FilePassMp3Id3(Mp3TimeCalc) == 0 )
	{
		kprintf("id3 read error\n");
		kfree(Mp3TimeCalc);
		return 0;
	}

	// 解压头信息，计算指定解压帧播放时间
	totaltime = 0;
	deframes = 0;
	avg_bitrate = 0;
	while(1)
	{
		// 检查是否退出循环
		if(estimate && (deframes >= estimate))
			break;

		// 同步帧头，并且获取头信息
		if(_Mp3FileHeaderDecode(Mp3TimeCalc, &frame_cnt) < 0)
			break;

		// 获取波特率信息，提取MP3主数据到其缓冲区
		if(Mp3TimeCalc->Header.Bitrate != 0)
		{
			int size;
			size = _Mp3FileHeaderLen(&Mp3TimeCalc->Header);
			_Mp3FileBufPass(Mp3TimeCalc, size);
		}

		// 获取一帧的单位时间, 获取MP3信息
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
		//计算平均比特率
		avg_bitrate /= deframes;

		totaltime = 26 * deframes;
	}
	else if( deframes < estimate )
	{
		//计算平均比特率
		avg_bitrate /= deframes;

		totaltime = totaltime * deframes;
	}
	else
	{
		//计算平均比特率
		avg_bitrate /= deframes;

		// 估算总的播放时间
		if(frame_cnt != 0 && frame_cnt != 1)
			totaltime *= frame_cnt;			//存在XING标签，可以直接读取出总的fram个数
		else
		{	//无XING标签，进行评估
			totaltime *= deframes;
			if(estimate)
			{
				int size;

				//得到一桢的数据长度
				size = Mp3TimeCalc->ReadSize - (Mp3TimeCalc->Buffer.FTail - Mp3TimeCalc->Buffer.FHead);
				if(size)
					totaltime = totaltime * (fsize / size);
				else
					totaltime = 0;
			}
		}
	}


	//设置声音变量
	Info->TotalTime       = totaltime;

	kfree(Mp3TimeCalc);
	return totaltime;
}
#endif // DECODE_MP3_ENABLE
