//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	    MP3解码－主函数部分
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

extern DWORD TimerCount();
#if defined(CONFIG_ARCH_XBURST)	&& !defined(WIN32)	
extern void MxuEnable();
#endif
extern BYTE __Mp3BufGetByte(MP3_BUF *buf, int offset, short *err);

static int Mp3DecodeFirstFrame = 0;

////////////////////////////////////////////////////
// 功能:
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static int Mp3DecodeReInit(MP3_STREAM *stream)
{
	// 缓冲区初始化
	Mp3BufInit(&stream->Buffer);
	
	// 多相合成滤波器初始化
	Mp3SynthInit(stream);

	// 静音处理初始化
	stream->MuteFrame = 0;

	Mp3DecodeFirstFrame = 0;
	return 0;
}


////////////////////////////////////////////////////
// 功能: 解压一帧MP3数据
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static int _Mp3DecodeFrame(MP3_STREAM *stream)
{
	int size;
	
#if defined(CONFIG_ARCH_XBURST)	&& !defined(WIN32)	
	MxuEnable();
#endif

mp3_again:
	// 同步帧头，并且获取头信息
	if(Mp3HeaderDecode(stream) < 0)
	{
		//kprintf("%s,%d : Mp3HeaderDecode error\n",__FILE__,__LINE__);
		return -1;
	}

	// 获取边信息
	if(Mp3SinfoDecode(stream) < 0)
	{
		kprintf("%s,%d : Mp3SinfoDecode error\n",__FILE__,__LINE__);
		return -1;
	}

	// 获取波特率信息，提取MP3主数据到其缓冲区
	if(Mp3HeaderBitrate(&stream->Header) != 0)
	{
		size = Mp3HeaderMaindataLen(&stream->Header);
		Mp3BufMainSave(&stream->Buffer, size);
		if(Mp3DecodeFirstFrame == 0)
		{
			BYTE tmp1, tmp2;
			short err;
			tmp1 = __Mp3BufGetByte(&stream->Buffer, 0, &err);
			tmp2 = __Mp3BufGetByte(&stream->Buffer, 1, &err);
			kprintf("%02x, %02x, %d\n", tmp1, tmp2, err);
			if((tmp1 != 0xff || (tmp2 & 0xe0) != 0xe0) && err >= 0)
			{
				stream->Buffer.MTail = 0;
				stream->Buffer.MHead = 0;
				stream->Buffer.MainBufRemain = 0;
				stream->Buffer.MainBufReady = 0;
				goto mp3_again;
			}
			else
				Mp3DecodeFirstFrame = 1;
		}
	}
	else
	{
		kprintf("%s,%d : Mp3SinfoDecode error\n",__FILE__,__LINE__);
		return -3;
	}

	// 检查主缓冲区中是否有足够的数据
	if(!Mp3BufMainInsuffice(stream, size))
	{
		kprintf("%s,%d : Mp3SinfoDecode error\n",__FILE__,__LINE__);
		return -5;
	}

	// 重新定位主数据缓冲区区位读取指针
	Mp3BufMainHead(&stream->Buffer, stream->SideInfo.MainDataBegin);

	// 比例因子解压、HUFFMAN解压和反量化处理
	if(Mp3HuffmanDecode(stream) < 0)
	{
		if(stream->MuteFrame < 2)
		{
			stream->MuteFrame = 0;
			kprintf("%s,%d : Mp3SinfoDecode error\n",__FILE__,__LINE__);
			return 0;
		}
		kprintf("%s,%d : Mp3SinfoDecode error\n",__FILE__,__LINE__);
		return -4;
	}
	
	// 立体声解码
	Mp3Stereo(stream);

	// 混叠降低和重排序
	Mp3AliasReduce(stream);

	// IMDCT处理、加窗处理
	Mp3Imdct(stream);

	// 多项合成滤波处理
	Mp3Synth(stream);
	return 0;
}


////////////////////////////////////////////////////
// 功能: 解压一帧MP3数据
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3DecodeFrame(HANDLE hmp3)
{
	MP3_STREAM *stream;
	int ret;
	int quit;

	stream = (MP3_STREAM*)hmp3;
	if(!stream)
		return -1;
	quit = 0;
	do
	{
		ret = _Mp3DecodeFrame(stream);
	}while((ret == -5) && (++quit < 100));
	return ret;
}


////////////////////////////////////////////////////
// 功能: 跳过一帧MP3数据解压
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
static int Mp3DecodePassFrame(MP3_STREAM *stream)
{
	int size;

	// 同步帧头，并且获取头信息
	if(Mp3HeaderDecode(stream) < 0)
	{
		kprintf("Mp3DecodePassFrame: -1\n");
		return -1;
	}

	// 获取波特率信息，提取MP3主数据到其缓冲区
	if(Mp3HeaderBitrate(&stream->Header) != 0)
	{
		size = Mp3HeaderExpectLen(&stream->Header);
		Mp3BufMainSavePass(&stream->Buffer, size);
		stream->MuteFrame = 0;
	}
	else
	{
		kprintf("Mp3DecodePassFrame: -3\n");
		return -3;
	}
	return 0;
}


////////////////////////////////////////////////////
// 功能: 数据缓冲区剩余数据字节数
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3DecodeBufRemain(MP3_STREAM *stream)
{
	return Mp3BufGetRemain(&stream->Buffer);
}



////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3DecodeSeek(HANDLE hmp3, DWORD ms)
{
	DWORD start,nf,rel;
	MP3_STREAM *stream = (MP3_STREAM*)hmp3;

	//MP3 SEEK到最开始的位置
	start = stream->Buffer.pCallback(stream->Buffer.CallbackId,0,0);
	Mp3DecodeReInit(stream);
	if( ms == 0 )
		return 0;		//直接跳到最开始

	nf = ms / 26;
	rel = nf * 26;
	kprintf("Mp3DecodeSeek : ms = %d, nf = %d,rel = %d\n",ms , nf,rel);
	while( nf )
	{
		if( Mp3DecodePassFrame(stream) < 0 )
			break;
		nf--;
	}
	kprintf("Mp3DecodeSeek end: nf = %d\n",nf);
	Mp3DecodeReInit(stream);
	return rel;
}


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
HANDLE Mp3DecodeCreate()
{
	MP3_STREAM *stream;

	stream = kmalloc(sizeof(MP3_STREAM));
	if(stream == NULL)
		return NULL;
	kmemset(stream, 0x00, sizeof(MP3_STREAM));
	Mp3BufInit(&stream->Buffer);
	Mp3RequantizeInit(stream);
	Mp3SynthInit(stream);

	Mp3DecodeFirstFrame = 0;

	return stream;
}


////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3DecodeDestroy(HANDLE hmp3)
{
	if(hmp3)
		kfree(hmp3);
	return 0;
}



////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3DecodeGetPcm(HANDLE hmp3, short *buf, int *max)
{
	MP3_STREAM *stream = (MP3_STREAM*)hmp3;
	short *left_ch = stream->Synth.Pcm[0];
	short *right_ch = stream->Synth.Pcm[1];
	int nchannels = stream->Synth.Channels;
	int nsamples = stream->Synth.Samples;

	if(nsamples > *max)
		nsamples = *max;
	if(nchannels == 2)
		*max = nsamples * (2 * sizeof(short));
	else
		*max = nsamples * sizeof(short);
	nsamples >>= 2;
	if(nchannels == 2)
	{
		while(nsamples--)
		{
			*buf++ = *left_ch++;
			*buf++ = *right_ch++;
			*buf++ = *left_ch++;
			*buf++ = *right_ch++;
			*buf++ = *left_ch++;
			*buf++ = *right_ch++;
			*buf++ = *left_ch++;
			*buf++ = *right_ch++;
		}
	}
	else
	{
		while(nsamples--)
		{
			*buf++ = *left_ch++;
			*buf++ = *left_ch++;
			*buf++ = *left_ch++;
			*buf++ = *left_ch++;
		}
	}
	return 0;
}

////////////////////////////////////////////////////
// 功能: 
// 输入: 
// 输出:
// 返回: 
////////////////////////////////////////////////////
int Mp3DecodeSetCallback(HANDLE hmp3, void *callback, DWORD id)
{
	MP3_STREAM *stream;
	
	stream = (MP3_STREAM*)hmp3;
	if(callback)
	{
		stream->Buffer.CallbackId = id;
		stream->Buffer.pCallback = (MP3_DATACALLBACK)callback;
		return 0;
	}
	return -1;
}

#endif // CONFIG_DECODE_MP3_ENABLE

