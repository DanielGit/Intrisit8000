//[PROPERTY]===========================================[PROPERTY]
//            *****  ŵ�����۲���ϵͳV2  *****
//        --------------------------------------
//    	          �������ſ�������ֺ���
//        --------------------------------------
//                 ��Ȩ: ��ŵ���ۿƼ�
//             ---------------------------
//                  ��   ��   ��   ʷ
//        --------------------------------------
//  �汾    ��ǰ		˵��		
//  V0.1    2009-5      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]

#include <kernel/kernel.h>
#include <kernel/irq.h>
#include <kernel/device.h>
#include <kernel/karch.h>
#include <kernel/sync.h>
#include <kernel/timer.h>
#include <platform/platform.h>
#include <direct/media.h>
#include <gui/winmsg.h>
#include <filesys/fs.h>
#include <config.h>

#ifdef CONFIG_MAKE_BIOS
#undef CONFIG_MAKE_BIOS
#endif 
#ifndef CONFIG_MPLAYER_ENABLE
#define CONFIG_MPLAYER_ENABLE
#endif

//#if defined(CONFIG_MAKE_BIOS) && defined(CONFIG_MPLAYER_ENABLE)
//#undef CONFIG_MPLAYER_ENABLE
//#endif 

//����������Ƶ���ųߴ�
#if defined(CONFIG_MAC_ND800) || defined(CONFIG_MAC_BDS7100) || defined(CONFIG_MAC_ASKMI1388)
#define HEIGHT_DEFINITION_MOVIE_W	640		//�����Ӱ(xvid,realvid,h264)���ŵ����ߴ�
#define HEIGHT_DEFINITION_MOVIE_H	480
#define NORMAL_DEFINITION_MOVIE_W	800		//������Ӱ���ŵ����ߴ�
#define NORMAL_DEFINITION_MOVIE_H	640
#else
#define HEIGHT_DEFINITION_MOVIE_W	800
#define HEIGHT_DEFINITION_MOVIE_H	640
#define NORMAL_DEFINITION_MOVIE_W	1280
#define NORMAL_DEFINITION_MOVIE_H	720
#endif


extern BYTE* pMediaPcmData;
extern DWORD nMediaPcmLen;
extern BYTE _end_mplayer_data;
extern BYTE _start_mplayer_data;
extern BYTE _end_mplayer_sbss;
extern BYTE _start_mplayer_sbss;
extern BYTE _end_mplayer_bss;
extern BYTE _start_mplayer_bss;
extern void FreeMplayerMem();
extern void ClockDelay(DWORD usec);
extern void WinClipMask(RECT *rect);
extern BYTE* LcdcVideoOsdBuf(void);
extern void DacReinit(HANDLE hdac);
extern void DacWaiteDma(HANDLE hdac);
extern void SetMuteMode(char mode);
extern MEDIALIB_TYPE CheckInnerPlayerType(PAK_VFILE vfile);
extern int JzCbCheckFile(PAK_VFILE vfile);

static BYTE *pMplayerDataMalloc,*pMplayerBssMalloc,*pMplayerSbssMalloc;

static PINNER_PLAYER_DRV InnerPlayerDrvList[] = 
{
	&WavDrvList,
	&Mp3DrvList,
#ifdef CONFIG_DECODE_MIDI_ENABLE
	&MidiDrvList,
#else
	NULL,
#endif
	NULL
};

////////////////////////////////////////////////////
// ����: 
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
void SaveMplayerVar()
{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	int data_size,bss_size,sbss_size;
	BYTE *pData;
	data_size = &_end_mplayer_data - &_start_mplayer_data;
	bss_size = &_end_mplayer_bss - &_start_mplayer_bss;
	sbss_size = &_end_mplayer_sbss - &_start_mplayer_sbss;
	kprintf("save mplayer var: data_size = 0x%x, bss_size = 0x%x, sbss_size = 0x%x\n",data_size,bss_size,sbss_size);

	if( data_size )
	{
		pMplayerDataMalloc = (BYTE*)JzCbMalloc(data_size);
		pData = &_start_mplayer_data;
		kmemcpy(pMplayerDataMalloc,pData,data_size);
	}
	
	if( bss_size )
	{
		pMplayerBssMalloc = (BYTE*)JzCbMalloc(bss_size);
		pData = &_start_mplayer_bss;
		kmemcpy(pMplayerBssMalloc,pData,bss_size);
	}

	if( sbss_size )
	{
		pMplayerSbssMalloc = (BYTE*)JzCbMalloc(sbss_size);
		pData = &_start_mplayer_sbss;
		kmemcpy(pMplayerSbssMalloc,pData,sbss_size);
	}
#endif
}
////////////////////////////////////////////////////
// ����: 
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
void LoadMplayerVar()
{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	int data_size,bss_size,sbss_size;
	BYTE *pData;
	data_size = &_end_mplayer_data - &_start_mplayer_data;
	bss_size = &_end_mplayer_bss - &_start_mplayer_bss;
	sbss_size = &_end_mplayer_sbss - &_start_mplayer_sbss;

	if( data_size )
	{
		pData = &_start_mplayer_data;
		kmemcpy(pData,pMplayerDataMalloc,data_size);
	}
	
	if( bss_size )
	{
		pData = &_start_mplayer_bss;
		kmemcpy(pData,pMplayerBssMalloc,bss_size);
	}

	if( sbss_size )
	{
		pData = &_start_mplayer_sbss;
		kmemcpy(pData,pMplayerSbssMalloc,sbss_size);
	}
#endif
}

////////////////////////////////////////////////////
// ����: �ӳ�N������
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
static void mdelay(unsigned int msec)
{
	unsigned int i;

	for (i = 0; i < msec; i++)
		ClockDelay(1000);
}

////////////////////////////////////////////////////
// ����: ��ʼ����Ƶ����Ƶ��
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibInit(PAK_OBJECT obj)
{
	DWORD *pFileData,flag;
	DWORD pFileSize,i;
	PAK_VFILE File;

	//Ϊ�˷�ֹ�Բ�ȫ0��Ƶ����Ƶ�������ݴ�
	File = &obj->File;
	pFileData = (DWORD*)kmalloc(1024 * sizeof( DWORD ));
	pFileSize = JzCbFileRead((BYTE*)pFileData,1,1024 * sizeof(DWORD),File);
	for( i = 0 , flag = 0 ; i < ( pFileSize / sizeof( DWORD ) ) ; i++ )
	{
		if( pFileData[i] )
			flag = 1;
	}

	kfree(pFileData);
	if( !flag )
		return -2;		//�ļ�ȫ0���˳�

	//ʹ���ڲ����������������ûص�����
	if( !obj->PlayerType )
		return 1;
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	LoadMplayerVar();
	if( MediaLibCbInit(obj) < 0 )
		return -1;
	InitMplayerLib();
#endif
	return 1;
}

////////////////////////////////////////////////////
// ����: ��ʼ���ص�������
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibCbInit(PAK_OBJECT obj)
{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	PMEDIA_TASK Task;
#endif

	//ʹ���ڲ����������������ûص�����
	if( !obj->PlayerType )
		return 1;
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	Task = &obj->Task;
	obj->JzAvDecoder = GetCbDecoder();
	AkCbInit(obj->JzAvDecoder,obj);
	AkCbReinit(obj->JzAvDecoder,obj);
	noah_set_lcd_size(Task->ShowRect.x,Task->ShowRect.y,Task->ShowRect.w,Task->ShowRect.h);
	if( !obj->JzAvDecoder->malloc_buf )
		return -1;
#endif
	return 1;
}

////////////////////////////////////////////////////
// ����: ����Ƶ����Ƶ��
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibOpen(PAK_OBJECT obj, int info)
{
	MEDIALIB_TYPE player_type;
	PINNER_PLAYER_DRV pDevice;

	if( !obj->PlayerType )
	{	//ʹ���ڲ�������

		//��ʼ���ڲ�����������
		obj->InnerPlayer.type         = MEDIALIB_UNKNOW;
		obj->InnerPlayer.status       = MEDIALIB_INIT;
		obj->InnerPlayer.cur_time     = 0;
		obj->InnerPlayer.total_time   = 0;
		obj->InnerPlayer.mp3_pcm_len  = 0;
		obj->InnerPlayer.wav_data_len = 0;
		obj->InnerPlayer.pDevice      = NULL;

		//�õ��ڲ�������������
		player_type = CheckInnerPlayerType(&obj->File);
		obj->InnerPlayer.status = player_type;

		//�õ��ڲ�������������
		pDevice = InnerPlayerDrvList[player_type];
		obj->InnerPlayer.pDevice = pDevice;
		if( pDevice == NULL )
		{
			kdebug(mod_media, PRINT_ERROR, "inner player open failed, pDevice is null, player_type = %d\n",player_type);
			obj->InnerPlayer.status = MEDIALIB_UNKNOW;
			return -1;
		}

		//���ڲ��������豸
		if( pDevice->Open(obj,&obj->File) <= 0)
		{
			kdebug(mod_media, PRINT_ERROR, "inner player open failed\n");
			obj->InnerPlayer.status = MEDIALIB_UNKNOW;
			return -1;
		}
		return 1;
	}
	else
	{	//ʹ���ⲿ������
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
		int ret;
		ret = OpenMplayerLib(info);
		if( ret < 0 )
			kdebug(mod_media, PRINT_ERROR, "open failed flag = %d\n",ret);
		return ret;
#endif
	}
	return 1;
}

////////////////////////////////////////////////////
// ����: ��ʼ������Ƶ����Ƶ
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibPlay(PAK_OBJECT obj)
{
	if( !obj->PlayerType )
	{	//ʹ���ⲿ������
		obj->InnerPlayer.status = MEDIALIB_PLAYING;
	}
	else
	{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
		return PLayMplayerLib();
#endif
	}
	return 0;
}

////////////////////////////////////////////////////
// ����: �ر���Ƶ����Ƶ��
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibClose(PAK_OBJECT obj)
{
	PINNER_PLAYER_DRV pDevice;

	if( !obj->PlayerType )
	{
		pDevice = obj->InnerPlayer.pDevice;
		if( pDevice )
			pDevice->Close(obj,&obj->File);
		else
			kdebug(mod_media, PRINT_WARNING, ">>>>>>>> MediaLibClose: device is null\n");
	}
	else
	{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
		PAK_VFILE vfile;
		MplayerCloseLib();

		//�ͷ�mplayerʹ�õ��ڴ�
		FreeMplayerMem();

		//�ͷŲ���ļ�
		//vfile = (PAK_VFILE)obj->JzAvDecoder->plugstream;
		//kfclose(vfile->File);
		//kfree(vfile);

		kfree(obj->JzAvDecoder);
		return 1;
#endif
	}
	return 1;
}

////////////////////////////////////////////////////
// ����: �õ���ǰ�����ļ�����Ϣ
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibGetInfo(PAK_OBJECT obj)
{
	PAK_VFILE vfile;
	PINNER_PLAYER_DRV pDevice;

	vfile = &obj->File;
	if( !obj->PlayerType )
	{	//�ڲ�������
		pDevice = obj->InnerPlayer.pDevice;
		if( pDevice )
		{
			if( pDevice->Info(obj,vfile) == 0 )
			{
				obj->Info.MediaType = MEDIALIB_UNKNOW;
				kdebug(mod_media, PRINT_ERROR, ">>>>>>>> MediaLibGetInfo: info is error\n");
				return -1;
			}
			kdebug(mod_media, PRINT_INFO, "TotalTime = %d\n",obj->Info.TotalTime);
			kdebug(mod_media, PRINT_INFO, "AudioBitrate = %d\n",obj->Info.AudioBitrate);
			kdebug(mod_media, PRINT_INFO, "AudioSamplerate = %d\n",obj->Info.AudioSamplerate);
			obj->InnerPlayer.type = obj->InnerPlayer.status;
			return 1;
		}
		else
		{
			kdebug(mod_media, PRINT_ERROR, ">>>>>>>> MediaLibGetInfo: device is null\n");
			return -1;
		}
	}
	else
	{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
		MplayerGetInfo();
		kdebug(mod_media, PRINT_INFO, "========================== media information ==========================\n");
		kmemcpy(&obj->Info,&obj->JzAvDecoder->MediaInfo,sizeof(MEDIA_INFO));
		kdebug(mod_media, PRINT_INFO, "obj = %x\n",obj);
		kdebug(mod_media, PRINT_INFO, "bHasAudio = %d\n",obj->Info.bHasAudio);
		kdebug(mod_media, PRINT_INFO, "bHasVideo = %d\n",obj->Info.bHasVideo);
		kdebug(mod_media, PRINT_INFO, "bAllowSeek = %d\n",obj->Info.bAllowSeek);
		kdebug(mod_media, PRINT_INFO, "TotalTime = %d\n",obj->Info.TotalTime);
		kdebug(mod_media, PRINT_INFO, "AudioBitrate = %d\n",obj->Info.AudioBitrate);
		kdebug(mod_media, PRINT_INFO, "AudioSamplerate = %d\n",obj->Info.AudioSamplerate);
		kdebug(mod_media, PRINT_INFO, "AudioChannels = %d\n",obj->Info.AudioChannels);
		kdebug(mod_media, PRINT_INFO, "VideoWidth = %d\n",obj->Info.VideoWidth);
		kdebug(mod_media, PRINT_INFO, "VideoHeight = %d\n",obj->Info.VideoHeight);
		kdebug(mod_media, PRINT_INFO, "VideoFps = %d\n",obj->Info.VideoFps);
		kdebug(mod_media, PRINT_INFO, "VideoBitrate = %d\n",obj->Info.VideoBitrate);
		kdebug(mod_media, PRINT_INFO, "AudioCodec = %s\n",obj->JzAvDecoder->AudioCodec);
		kdebug(mod_media, PRINT_INFO, "VideoCodec = %s\n",obj->JzAvDecoder->VideoCodec);
		kdebug(mod_media, PRINT_INFO, "=======================================================================\n");

		if( obj->Info.bHasVideo )
		{	//������Ƶ���ж���Ƶ���ųߴ��Ƿ����1080x720��������ڣ��򲻽��в��ţ���ֹ�ڴ治�㣬���¸������������
			if( kstrcmp(obj->JzAvDecoder->VideoCodec,"xvid") == 0 || kstrcmp(obj->JzAvDecoder->VideoCodec,"realvid") == 0 || kstrcmp(obj->JzAvDecoder->VideoCodec,"h264") == 0 )
			{
				if( obj->Info.VideoHeight > HEIGHT_DEFINITION_MOVIE_H || obj->Info.VideoWidth > HEIGHT_DEFINITION_MOVIE_W )
				{
					kdebug(mod_media, PRINT_ERROR, "VideoCodec = %s\n",obj->JzAvDecoder->VideoCodec);
					kdebug(mod_media, PRINT_ERROR, "video widht = %d, height = %d\n",obj->Info.VideoWidth,obj->Info.VideoHeight);
					kdebug(mod_media, PRINT_ERROR, "vido is to large can't play\n");
					return -1;
				}

			}
			else
			{
				if( obj->Info.VideoHeight > NORMAL_DEFINITION_MOVIE_H || obj->Info.VideoWidth > NORMAL_DEFINITION_MOVIE_W )
				{
					kdebug(mod_media, PRINT_ERROR, "VideoCodec = %s\n",obj->JzAvDecoder->VideoCodec);
					kdebug(mod_media, PRINT_ERROR, "video widht = %d, height = %d\n",obj->Info.VideoWidth,obj->Info.VideoHeight);
					kdebug(mod_media, PRINT_ERROR, "vido is to large can't play\n");
					return -1;
				}
			}
		}
		SetMplayerAudioSpace(obj);
		obj->Info.MediaType = 1;
#endif
	}
	return 1;
}

////////////////////////////////////////////////////
// ����: �õ���ǰ����״̬
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
MEDIALIB_STATUS MediaLibGetStatus(PAK_OBJECT obj)
{
	if( !obj->PlayerType )
		return obj->InnerPlayer.status;
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	return GetMplayerStatus();
#endif
	return 0;
}

////////////////////////////////////////////////////
// ����: ��ͣ���� 
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibPause(PAK_OBJECT obj)
{
	if( !obj->PlayerType )
	{
		obj->InnerPlayer.status = MEDIALIB_PAUSE;
		DacSetStatus(obj->hDac,1);
	}
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	else
	{
		obj->JzAvDecoder->fIpuEnable = 1;
		DacSetStatus(obj->hDac,1);
		WinClipMask(&obj->PauseRect);
		SetMplayerPause();
	}
#endif
	mdelay(100);
	return 1;
}

////////////////////////////////////////////////////
// ����: ��������
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibResume(PAK_OBJECT obj)
{
	mdelay(100);
	if( !obj->PlayerType )
	{
		obj->InnerPlayer.status = MEDIALIB_PLAYING;
		DacSetStatus(obj->hDac,0);
	}
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	else
	{
		SetMplayerResume();
		WinClipMask(&obj->Task.ShowRect);
		DacSetStatus(obj->hDac,0);
		obj->JzAvDecoder->fIpuEnable = 0;
	}
#endif
	return 1;
}

////////////////////////////////////////////////////
// ����: ֹͣ����
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibStop(PAK_OBJECT obj)
{
	if( !obj->PlayerType )
		obj->InnerPlayer.status = MEDIALIB_STOP;
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	else
		SetMplayerStop();
#endif
	return 1;
}

////////////////////////////////////////////////////
// ����: SEEK��ָ��ʱ�䲥��
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MediaLibSeek(PAK_OBJECT obj, unsigned long ms)
{
	PINNER_PLAYER_DRV pDevice;

	if( !obj->PlayerType )
	{
		pDevice = obj->InnerPlayer.pDevice;
		obj->InnerPlayer.status = MEDIALIB_SEEK;
		mdelay(10);
		pDevice->Seek(obj,ms);
		mdelay(10);
		obj->InnerPlayer.status = MEDIALIB_PLAYING;
	}
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	else
	{
		kdebug(mod_media, PRINT_INFO, ">>>>>>>> seek to %dms <<<<<<<<\n",ms);
		SetMplayerPause();
		mdelay(10);
//		DacWriteEnd(obj->hDac,0);
		SetMplayerSeek(ms);
//		DacWaiteDma(obj->hDac);
		SetMplayerResume();
		mdelay(100);
	}
#endif
	return 1;
}

////////////////////////////////////////////////////
// ����: �õ���Ƶ���ݵ�PCM����
// ����: 
// ���:
// ����: PCM���ݳ���
// ˵��: 
////////////////////////////////////////////////////
int MediaLibGetPcm(PAK_OBJECT obj,BYTE* pcm,int size)
{
	PINNER_PLAYER_DRV pDevice;

	if( !obj->PlayerType )
	{
		pDevice = obj->InnerPlayer.pDevice;
		if( pDevice )
			return pDevice->GetPcmData(obj,pcm,size);
		else
		{
			kdebug(mod_media, PRINT_ERROR, ">>>>>>>> MediaLibGetPcm: device is null\n");
			return -1;
		}
	}
	else
	{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
		int ret,res;
		PAK_VFILE vfile;
		vfile = (PAK_VFILE)obj->JzAvDecoder->stream;
		res = GetMplayerDataLib();
		if( JzCbCheckFile(vfile) < 0 )
		{
			kdebug(mod_media, PRINT_ERROR, "audio file error\n");
			SetMplayerError();
			return -1;
		}
		ret = nMediaPcmLen & (~0x80000000);
		if(nMediaPcmLen == 0x80000000)
			return -1;
		if(nMediaPcmLen & 0x80000000)
			nMediaPcmLen = 0x80000000;
		return ret;
#endif
	}
	return 0;
}

////////////////////////////////////////////////////
// ����: �õ���Ƶ���ݵ�PCM����
// ����: 
// ���:
// ����: PCM���ݳ���
// ˵��: 
////////////////////////////////////////////////////
int MediaLibGetYuv(PAK_OBJECT obj)
{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	int ret;
	PAK_VFILE vfile;
#endif

	if( !obj->PlayerType )
		return 0;
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	vfile = (PAK_VFILE)obj->JzAvDecoder->stream;
	if( JzCbCheckFile(vfile) < 0 )
	{
		kdebug(mod_media, PRINT_ERROR, "video file error\n");
		SetMplayerError();
		return -1;
	}
	ret = GetMplayerVideoDataLib();
	return ret;
#else
	return 0;
#endif
}

////////////////////////////////////////////////////
// ����: �õ���ǰ��Ƶ���ŵ�ʱ�䣬������VIDEOͬ��
// ����: 
// ���:
// ����: ��ǰ��Ƶ���ŵ�ʱ��
// ˵��: 
////////////////////////////////////////////////////
int MediaLibGetAudioCurTime(PAK_OBJECT obj)
{
	if( !obj->PlayerType )
		return obj->InnerPlayer.cur_time;
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	return GetMplayerCurTime();
#endif
	return 0;
}

////////////////////////////////////////////////////
// ����: ������Ƶ����ʾλ���Լ���С
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
void SetVideoPosAndSize(PAK_OBJECT obj,PMEDIA_TASK task)
{
	int video_x,video_y,video_width,video_height;
	int win_width,win_height,x,y;

	//videoԭʼ�ߴ�
	video_width  = obj->Info.VideoWidth;
	video_height = obj->Info.VideoHeight;

	//�õ����ڵĳߴ�
	x = video_x = task->ShowRect.x;
	y = video_y = task->ShowRect.y;
	win_width   = task->ShowRect.w;
	win_height  = task->ShowRect.h;
	kdebug(mod_media, PRINT_INFO, "Usr Set: x = %d, y = %d, w = %d, h = %d\n",x,y,win_width,win_height);
	kdebug(mod_media, PRINT_INFO, "Video info: w = %d, h = %d\n",video_width,video_height);

	//���ȣ��߶�Ϊ0�����Զ�Ĭ��Ϊ��Ļ���ȣ��߶�
	win_width  = ( win_width == 0 ) ? CONFIG_MAINLCD_XSIZE : win_width;
	win_height = ( win_height == 0 ) ? CONFIG_MAINLCD_YSIZE : win_height;

	//���ȣ��߶� Ϊ-1����ʾ�Զ����ó�ԭʼ�ߴ�
	win_width  = ( win_width == -1 ) ? video_width : win_width;
	win_height = ( win_height == -1 ) ? video_height : win_height;

	//��Ƶ�ķŴ������ܳ���2��
	win_width  = ((video_width * 2) < win_width) ? video_width * 2 : win_width;
	win_height = ((video_height * 2) < win_height) ? video_height * 2 : win_height;

	//��Ƶ�ķŴ�������С��2��
	win_width  = ((video_width / 32) > win_width) ? video_width / 32 : win_width;
	win_height = ((video_height / 32) > win_height) ? video_height / 32 : win_height;

	//x,y = -1����Ļ������ʾ
	task->ShowRect.x = (x == -1) ? (CONFIG_MAINLCD_XSIZE - win_width) / 2 : x;
	task->ShowRect.y = (y == -1) ? (CONFIG_MAINLCD_YSIZE - win_height) / 2 : y;

	//����������������ʾ�����������ʾ
	task->ShowRect.x = (task->ShowRect.w > win_width) ? (task->ShowRect.x + (task->ShowRect.w - win_width) / 2) : task->ShowRect.x;
	task->ShowRect.y = (task->ShowRect.h > win_height) ? (task->ShowRect.y + (task->ShowRect.h - win_height) / 2) : task->ShowRect.y;
	
	task->ShowRect.w      = win_width;
	task->ShowRect.h      = win_height;

	kdebug(mod_media, PRINT_INFO, "media info: x = %d, y = %d, width = %d, height = %d\n",
		task->ShowRect.x,task->ShowRect.y,task->ShowRect.w,task->ShowRect.h);

}
////////////////////////////////////////////////////
// ����: ����û����õ�����
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
void MediaClearUsrScreen( PMEDIA_TASK task,BYTE format )
{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	int   w,h;
	int   bpp,i;
	RECT  cr;
	BYTE *pScreen,*pos;

	pScreen = LcdcVideoOsdBuf();
	cr.x = task->ShowRect.x;
	cr.y = task->ShowRect.y;
	cr.w = task->ShowRect.w;
	cr.h = task->ShowRect.h;
	kdebug(mod_media, PRINT_INFO, "clean rect = (%d,%d,%d,%d)\n",cr.x,cr.y,cr.w,cr.h);

	if( task->ShowRect.w == 0 || task->ShowRect.h == 0 )
		return;

	if( format == 6 )
		bpp = 4;
	else if( format == 9 )
		bpp = 2;
	else
		return;

	kdebug(mod_media, PRINT_INFO, "bpp = %d\n",bpp);
	if( task->ShowRect.w != CONFIG_MAINLCD_XSIZE )
	{	// ���Ȳ�����Ļ����
		for( h = task->ShowRect.y ; h < (task->ShowRect.y + task->ShowRect.h) ; h++ )
		{
			if( h >= CONFIG_MAINLCD_YSIZE)
				break;

			for( w = task->ShowRect.x ; w < (task->ShowRect.x + task->ShowRect.w) ; w++)
			{
				if( w >= CONFIG_MAINLCD_XSIZE )
					break;

				pos = pScreen + (h * CONFIG_MAINLCD_XSIZE + w) * bpp;
				for( i = 0 ; i < bpp ; i++ )
					*pos++ = 0;
			}
		}
	}
	else
	{	// ����Ϊ��Ļ����
		if( task->ShowRect.h == CONFIG_MAINLCD_YSIZE)
			kmemset(pScreen,0,CONFIG_MAINLCD_XSIZE * CONFIG_MAINLCD_YSIZE * bpp );
		else
		{
			for( h = task->ShowRect.y ; h < (task->ShowRect.y + task->ShowRect.h) ; h++ )
			{
				if( h >= CONFIG_MAINLCD_YSIZE)
					break;

				pos = pScreen + (h * CONFIG_MAINLCD_XSIZE) * bpp;
				kmemset(pos,0,CONFIG_MAINLCD_XSIZE * bpp);
			}
		}
	}
#endif
}

////////////////////////////////////////////////////
// ����: ������Ƶ����ʾλ���Լ���С
// ����: 
// ���:
// ����: 
// ˵��: 
////////////////////////////////////////////////////
int MplayerResizeVideo(PAK_OBJECT obj,PMEDIA_TASK task)
{
#if defined(CONFIG_MPLAYER_ENABLE) && !defined(CONFIG_MAKE_BIOS)
	RECT cr;
	if( obj->Info.bHasVideo )
	{	//��Ƶ�ļ�����Ҫ������Ƶ����ʾ�ߴ�
		MediaLibPause(obj);

		//�����û�������ʾ����
		cr.x = task->ShowRect.x;
		cr.y = task->ShowRect.y;
		cr.w = task->ShowRect.w;
		cr.h = task->ShowRect.h;

		//�����û���ʾ�������¼���VIDEOʵ����ʾ����
		SetVideoPosAndSize(obj,task);

		//����VIDEOʵ�������С
		AkCbReinit(obj->JzAvDecoder,obj);

		//�޸�IPU��ʾ����
		noah_set_lcd_size(task->ShowRect.x,task->ShowRect.y,task->ShowRect.w,task->ShowRect.h);

		//�����û���������
		task->ShowRect.x = cr.x;
		task->ShowRect.y = cr.y;
		task->ShowRect.w = cr.w;
		task->ShowRect.h = cr.h;

		//��ʼ���²��ţ����Ұ����û������������ü�������
		MediaLibResume(obj);

		//����û���������
		MediaClearUsrScreen(task,obj->JzAvDecoder->OutFormat);
		return 1;
	}
	else
	{
		//����û���������
		WinClipMask(&obj->Task.ShowRect);
		MediaClearUsrScreen(task,obj->JzAvDecoder->OutFormat);
	}
#endif
	return 0;
}