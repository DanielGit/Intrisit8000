//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台  *****
//        --------------------------------------
//         	         MP3解码头文件
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
//  V0.1    2005-8      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]


#include <config.h>
#include <kernel/kernel.h>

#ifndef _MP3PRV_H
#define _MP3PRV_H

// 使用MAC宏单元定义
#if defined(WIN32) || defined(CONFIG_MCU_C33L27)
#define USE_MAC
#endif

// EPSON平台使用内部RAM地址及DSTRAM地址定义
#if defined(CONFIG_MCU_C33L27)
#define INNER_RAM_ADDRESS		0x4300		// 合成多相滤波器使用(占用2K空间)
#define INNER_STACK_ADDRESS		0x5000		// MP3解压使用栈空间(占用1K空间)
#define DST_RAM_ADDRESS			0x84000		// DST-RAM起始地址(占用2K空间)
#endif


// 整数类型及整数运算定义
#define MP3INT				int
#define SIZEOF_INT			4
#define IMUL(x, y)			(((MP3INT)(x) * (MP3INT)(y))>>13)
#define IMULNS(x, y)		((MP3INT)(x) * (MP3INT)(y))
#define IMULSHIFT(x)		((MP3INT)((x) >> 13))
#define IMUL2(x, y)			(((MP3INT)(x) * (MP3INT)(y))>>12)
#define IMUL2NS(x, y)		((MP3INT)(x) * (MP3INT)(y))
#define IMUL2SHIFT(x)		((MP3INT)((x) >> 12))
#define MACF(z, x, y)		(z = (((MP3INT)(x) * (MP3INT)(y))>>13))
#define MACN(z, x, y)		(z += (((MP3INT)(x) * (MP3INT)(y))>>13))
#define CONST_INT(x)		((MP3INT)((DWORD)(x)>>15))
#define CONST2_INT(x)		((MP3INT)((DWORD)(x)>>16))
#define CONST_SHORT(x)		((short)((DWORD)(x)>>15))
#define CONST2_SHORT(x)		((short)((DWORD)(x)>>16))

// MP3解压频率线线数
#define MP3_DECODE_LINE			576

// 缓冲区大小定义
#define MP3_FRAME_BUF_SIZE		2048
#define MP3_MAIN_BUF_SIZE		2048


typedef DWORD REQ_FLOAT;

typedef int (*MP3_DATACALLBACK)(DWORD id, BYTE* buf, int size);
typedef int (*MP3_REQUANTIZE)(int value, int exp, const REQ_FLOAT *power);

#define MP3_FLAG_PRIVATEDBIT		0x0001
#define MP3_FLAG_FREEFORMAT			0x0008
#define MP3_FLAG_PROTECTION			0x0010  	// frame has CRC protection 
#define MP3_FLAG_COPYRIGHT			0x0020  	// frame is copyright 
#define MP3_FLAG_ORIGINAL			0x0040  	// frame is original (else copy) 
#define MP3_FLAG_PADDING			0x0080  	// frame has additional slot 


#define MP3_MPEG1					0x03
#define MP3_MPEG2					0x02
#define MP3_MPEG25					0x00

#define MP3_MODE_STEREO				0x00
#define MP3_MODE_JOINTSTEREO		0x01
#define MP3_MODE_DUALCHANNEL		0x02
#define MP3_MODE_SINGLECHANNEL		0x03

#define MP3_MODEX_ISTEREO			0x01
#define MP3_MODEX_MSSTEREO			0x02

#define MP3_MIXED_BLOCK				0x10
#define MP3_PREFLAG					0x04
#define MP3_SCALEFAC_SCALE			0x02
#define MP3_COUNT1_TABLE			0x01

typedef struct _MP3_MCACHE
{
	union
	{
		DWORD DWord;
		BYTE  Byte[4];
	}Cache;					// Cache
	short	BitCount;		// 读取计数器
}MP3_MCACHE;


typedef struct _MP3_BUF
{
	DWORD TaskStack;				// 任务堆栈指针
	DWORD Mp3Stack;					// MP3解压堆栈指针
	DWORD CallbackId;				// 回调函数ID号
	MP3_DATACALLBACK pCallback;		// 缓冲区回调函数
	
	union
	{
		DWORD DWord;
		BYTE  Byte[4];
	}Cache;					// Cache
	WORD	BitRemain;		// 剩余计数器
	short FHead;
	short FTail;

	// 主数据缓冲区部分
	short MTail;				// 当前帧主数据结束位置
	short MHead;				// 当前帧主数据开始位置
	short MainBufReady;
	short MainBufRemain;
	BYTE MainBuf[MP3_MAIN_BUF_SIZE+4]; 	// 主数据缓冲区
	BYTE FrameBuf[MP3_FRAME_BUF_SIZE]; 	// 帧数据缓冲区
}MP3_BUF;


// MP3头信息结构
typedef struct _MP3_HEADER
{
	BYTE Mpeg;
	BYTE Mode;			// channel mode (see above) 
	BYTE ModeEx;		// additional mode info 
	BYTE Emphasis;		// de-emphasis to use (see above) 

	DWORD Bitrate;		// stream bitrate (bps) 
	DWORD Samplerate;	// sampling frequency (Hz) 

	WORD Crc;			// frame CRC accumulator 
	WORD Flags;			// flags (see below) 
}MP3_HEADER;

// MP3每个通道的边信息数据结构
typedef struct _MP3_CHSI
{
	WORD Part23Len;
	WORD BigValues;
	WORD ScfCompress;
	short GlobalGain;

	BYTE Flags;
	BYTE BlockType;
	BYTE TableSelect[3];
	BYTE SubblockGain[3];
	BYTE Reg0Count;
	BYTE Reg1Count;
	BYTE ZeroSbf;			// 零游程起始自带
	WORD ZeroFreqLine;		// 零游程起始频率线
	BYTE const *SfbWidth;	
	short Scalefac[39];
	short Exponent[39];	
}MP3_CHSI;

// MP3边信息数据结构
typedef struct _MP3_SIDEINFO
{
	WORD MainDataBegin;
	BYTE PrivateBits;
	BYTE ScfSI[2];
	MP3_CHSI Channel[2][2];
}MP3_SIDEINFO;

// MP3多相合成滤波器数据结构
typedef struct _MP3_SYNTH
{
	DWORD SampleRate;
	int Phase;
	short Samples;
	short Channels;
	short XSlot[2];
	MP3INT Filter[2][2][2][16][8];	/* polyphase filterbank outputs */
	short Pcm[2][MP3_DECODE_LINE*2];
	short PrePcm[2];
}MP3_SYNTH;

// MP3流处理数据结构
typedef struct _MP3_STREAM
{
	MP3_BUF Buffer;			// 保持该项在结构体中的第一位置！！
	MP3_HEADER Header;
	MP3_SIDEINFO SideInfo;
	MP3_SYNTH Synth;
	short ScfbSelect;			// 比例因子带选择项
	short MuteFrame;			// 静音计数处理
	MP3INT FreqLine[2][2][MP3_DECODE_LINE*2];
	MP3INT Overlap[2][MP3_DECODE_LINE];
	MP3INT SbSample[2][MP3_DECODE_LINE*2];
	MP3INT Alias[MP3_DECODE_LINE];
	MP3_MCACHE Cache;
	MP3_REQUANTIZE ReqExponent;
	const REQ_FLOAT *ReqTable;
	int DecodeLines;
	int Mp3SynthVX[2][256];
}MP3_STREAM;

typedef struct 
{
	BYTE const *l;
	BYTE const *s;
	BYTE const *m;
}SFB_WIDTH;

typedef struct _MEDIA_PCMINFO
{
	DWORD	Samplerate;
	DWORD	Channels;
	DWORD	Samples;
	DWORD	Buffers;
}MEDIA_PCMINFO;

typedef struct _REORD_TABLE
{
	const short *s;
	const short *m;
}REORD_TABLE;

typedef struct _HFM_TBL
{
	struct _HFM_TBL const *next;
	signed char x;
	signed char y;
	signed char bits;			// 返回比较位数、下一比较位数
	signed char total;
}HFM_TBL;

typedef struct _HFM_WTBL
{
	signed char v;
	signed char w;
	signed char x;
	signed char y;
	signed char bits;
	signed char total;
}HFM_WTBL;

typedef struct _MP3_INFO
{
	int MediaType;
	DWORD TotalTime;
	
	int AudioCodec;
	int AudioBitrate;
	int AudioSamplerate;
	int AudioChannels;
}MP3_INFO;

void Mp3BufInit(MP3_BUF *buf);
int Mp3BufMainCache(MP3_BUF *buf, MP3_MCACHE *cache, int BitRemain);
void Mp3BufMainSave(MP3_BUF *buf, WORD size);
void Mp3BufMainSavePass(MP3_BUF *buf, WORD size);
void Mp3BufMainHead(MP3_BUF *buf, WORD size);
void Mp3BufCache(MP3_BUF *buf);
BYTE Mp3BufGetByte(MP3_BUF *buf, short *err);
WORD Mp3BufGetBits(MP3_BUF *buf, BYTE bits);
void Mp3BufCacheReset(MP3_BUF *buf);
void Mp3BufMainPass(MP3_BUF *buf, WORD size);
int Mp3BufGetRemain(MP3_BUF *buf);
int Mp3BufMainInsuffice(MP3_STREAM *stream, int size);


void Mp3HeaderInit(MP3_HEADER *header);
int Mp3HeaderDecode(MP3_STREAM *stream);
DWORD Mp3HeaderBitrate(MP3_HEADER *header);
int Mp3HeaderMaindataLen(MP3_HEADER *header);
int Mp3HeaderExpectLen(MP3_HEADER *header);
int Mp3HeaderSideinfoLen(MP3_HEADER *header);

void Mp3SinfoInit(MP3_SIDEINFO *si);
int Mp3SinfoDecode(MP3_STREAM *stream);

void Mp3ScalefacBandWidth(MP3_STREAM *stream);

int Mp3HuffmanDecode(MP3_STREAM *stream);

void Mp3RequantizeInit(MP3_STREAM *stream);
int Mp3Requantize(int value, int exp, const REQ_FLOAT *power);
void Mp3RequantizeExponent(MP3_CHSI *channel, int lines);

int Mp3Stereo(MP3_STREAM *stream);

void Mp3AliasReduce(MP3_STREAM *stream);

int Mp3Imdct(MP3_STREAM *stream);

int Mp3SynthInit(MP3_STREAM *stream);
int Mp3Synth(MP3_STREAM *stream);
int Mp3SynthHalfInit(MP3_STREAM *stream);
int Mp3SynthHalf(MP3_STREAM *stream);


void MacInit(int mode, int acc);
void MacAddr(int *addr);
int MacResult(void);
void MacOne(int m0);
void MacFour(int m0, int m1, int m2, int m3);
int MacRepeat(unsigned int times, int *HeadADD_1, int *HeadADD_2);
int MacRepeat8(unsigned int times, int *HeadADD_1, int *HeadADD_2);
int MacRepeat16(unsigned int times, int *HeadADD_1, int *HeadADD_2);


typedef void (*_MAC_INIT)(int mode, int acc);
typedef void (*_MAC_ADDR)(int *addr);
typedef int (*_MAC_RESULT)(void);
typedef void (*_MAC_ONE)(int m0);
typedef void (*_MAC_FOUR)(int m0, int m1, int m2, int m3);
typedef int (*_MAC_REPEAT)(unsigned int times, int *HeadADD_1, int *HeadADD_2);
typedef int (*_MAC_REPEAT8)(unsigned int times, int *HeadADD_1, int *HeadADD_2);
typedef int (*_MAC_REPEAT16)(unsigned int times, int *HeadADD_1, int *HeadADD_2);



#endif // _MP3PRV_H

