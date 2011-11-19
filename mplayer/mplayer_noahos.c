#include "sys/time.h"
#include "mplayer_noahos.h"

#define MSGSIZE_MAX 1024

#define OSC_CLOCK       12000000
//Register OP
#define INREG8(x)           ( (unsigned char)(*(volatile unsigned char * const)(x)) )
#define OUTREG8(x, y)       *(volatile unsigned char * const)(x) = (y)
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))

#define INREG16(x)           ( (unsigned short)(*(volatile unsigned short * const)(x)) )
#define OUTREG16(x, y)       *(volatile unsigned short * const)(x) = (y)
#define SETREG16(x, y)      OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)      OUTREG16(x, INREG16(x)&~(y))

#define INREG32(x)           ( (unsigned int)(*(volatile unsigned int* const)(x)) )
#define OUTREG32(x, y)       *(volatile unsigned int * const)(x) = (y)
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))

//TCU, OST
#define TCU_BASE_U_VIRTUAL	( 0xB0002000 )
#define OST_BASE_U_VIRTUAL	( 0xB0002000 )
#define TCU_TESR_OFFSET			( 0x14 )	//  W,  16, 0x??
#define TCU_TECR_OFFSET			( 0x18 )	//  W,  16, 0x??
#define TCU_TMSR_OFFSET			( 0x34 )	//  W, 32, 0x????????
#define	OST_DR_OFFSET			  ( 0xe0 )	// RW, 32, 0x????
#define	OST_CNT_OFFSET			( 0xe4 )	// RW, 32, 0x????
#define	OST_CSR_OFFSET			( 0xeC )	// RW, 16, 0x0000
#define	TCU_OSTCL		        ( 1 << 15 )
#define	TCU_OSTEN		        ( 1 << 15 )
#define	TCU_OSTMST		      ( 1 << 15 )
#define	TCU_CLK_EXTAL				( 0x1 << 2)
#define	TCU_CLK_PRESCALE_CLK4			( 0x1 << 3)
#define A_TCU_TESR				( TCU_BASE_U_VIRTUAL + TCU_TESR_OFFSET )
#define A_TCU_TECR				( TCU_BASE_U_VIRTUAL + TCU_TECR_OFFSET )
#define A_TCU_TMSR				( TCU_BASE_U_VIRTUAL + TCU_TMSR_OFFSET )
#define A_OST_DR				( OST_BASE_U_VIRTUAL + OST_DR_OFFSET )
#define A_OST_CNT				( OST_BASE_U_VIRTUAL + OST_CNT_OFFSET )
#define A_OST_CSR				( OST_BASE_U_VIRTUAL + OST_CSR_OFFSET )

Jz47_AV_Decoder *jz47_av_decp;
void noah_kprintf(const char *format, ... );
extern void ipu_init_lcd_size(int x,int y,int w, int h);
#undef kprintf
//=================== NOAH OS and timer =========================
void noah_os_init (struct JZ47_AV_Decoder *priv)
{
	jz47_av_decp = priv;
	noah_kprintf ("++ buf = 0x%08x, size = 0x%08x steam = 0x%08x, plugstream = 0x%08x+++\n", 
	         jz47_av_decp->malloc_buf, jz47_av_decp->malloc_size,  jz47_av_decp->stream,  jz47_av_decp->plugstream);
	mplayer_memory_set(jz47_av_decp->malloc_buf, jz47_av_decp->malloc_size);
	ipu_init_lcd_size(jz47_av_decp->UsrLcdPosX,jz47_av_decp->UsrLcdPosY,jz47_av_decp->UsrLcdWidth,jz47_av_decp->UsrLcdHeight);
}
int Init_PerformanceCounter(void)
{
	OUTREG16(A_TCU_TECR, TCU_OSTCL);

	OUTREG16(A_OST_CSR, TCU_CLK_EXTAL | TCU_CLK_PRESCALE_CLK4);
	OUTREG32(A_OST_DR, 0xFFFFFFFF);
	OUTREG32(A_OST_CNT, 0);

	SETREG32(A_TCU_TESR, TCU_OSTEN);

	return ((OSC_CLOCK / 4 ) / 1000000);
}

unsigned int Get_PerformanceCounter(void)
{
     return (INREG32(A_OST_CNT));
}

void Stop_PerformanceCounter(void)
{
	noah_kprintf("WARMING: Performance Counter will STOP!!!\n");

	OUTREG32(A_TCU_TMSR, TCU_OSTMST);
	OUTREG16(A_TCU_TECR, TCU_OSTCL);
}

void JZ_StopTimer(void)
{
	Stop_PerformanceCounter();
}

static unsigned int prev_time = 0, cur_time = 0;
static unsigned long long time_of_day = 0;
int gettimeofday(struct timeval now_tv, struct timezone zone)
{
	unsigned cur_time = Get_PerformanceCounter() / (OSC_CLOCK / 4 / 1000000);
	if (cur_time < prev_time)
	{
		time_of_day = time_of_day + 0x100000000LL;
		time_of_day = time_of_day + (unsigned long long) cur_time - (unsigned long long)prev_time;
	}
	else
		time_of_day = time_of_day + (unsigned long long) cur_time - (unsigned long long)prev_time;
	
	prev_time = cur_time;
	
	 now_tv.tv_sec  = (int)(time_of_day / 1000000);
   now_tv.tv_usec = (int)(time_of_day % 1000000);
   return 0;
}

void BUFF_TimeDly (unsigned int usec)
{
	//kprintf("mplayer time delay = %d\n",usec / 1000);
	jz47_av_decp->os_msleep (usec / 1000);
}
// ================ LCD ===================
/*
*/
int lcd_get_width ()
{
  return jz47_av_decp->lcd_width;
}

int lcd_get_height ()
{
  return jz47_av_decp->lcd_height;
}

int lcd_get_line_length ()
{
  return jz47_av_decp->lcd_line_length;
}

void* lcd_get_frame ()
{
  return (void *)jz47_av_decp->lcd_frame_buffer;
}
// ================ NOAH OS I/O stream =================

#define SEEK_SET               0
#define SEEK_CUR               1
#define SEEK_END               2

void* jzfs_Open ()
{
	return jz47_av_decp->stream;
}

int jzfs_Close ()
{
	return 1;
}

long jzfs_Tell (void *stream)
{
	return jz47_av_decp->os_ftell (stream);
}

long jzfs_Read (void *ptr, long size, long nmemb, void *stream)
{
	long len;
	len = jz47_av_decp->os_fread (ptr, size, nmemb, stream);
//	kprintf("jzfs_Read: len = %d, size = %d\n",len,size * nmemb);
	return len;
}

long jzfs_Write (void *ptr, long size, long nmemb, void *stream)
{
	return jz47_av_decp->os_fwrite (ptr, size, nmemb, stream);
}

long jzfs_Seek (void *stream, long offset, int whence)
{
	return jz47_av_decp->os_fseek (stream, offset, whence);
}

void* BUFF_Open (char *p)
{
	return jz47_av_decp->stream;
}

void BUFF_Close (void *stream)
{
	return;
}

long BUFF_Seek (void *stream, long offset, int whence)
{
	return jz47_av_decp->os_fseek (stream, offset, whence);
}

long BUFF_DirectRead (void *stream, void *ptr, long size)
{
	long len;
	len = jz47_av_decp->os_fread (ptr, 1, size, stream);
//	kprintf("BUFF_DirectRead: len = %d,size = %d\n",len,size);
	
	return len;
}

long BUFF_GetFileSize (void *stream)
{
	long len, cur;
	cur = jz47_av_decp->os_ftell (stream);
	jz47_av_decp->os_fseek (stream, 0, SEEK_END);
	len = jz47_av_decp->os_ftell (stream);
	jz47_av_decp->os_fseek (stream, cur, SEEK_SET);
	return len;
}

// ================ NOAH OS audio driver functions =================

int noahos_audio_init(int rate,int channels,int format,int flags)
{
	return jz47_av_decp->os_audio_init (rate, channels, format, flags);
}

void noahos_audio_uninit(int immed)
{
  jz47_av_decp->os_audio_uninit (immed);
}

void noahos_audio_reset(void)
{
	jz47_av_decp->os_audio_reset ();
}

void noahos_audio_pause(void)
{
	jz47_av_decp->os_audio_pause ();
}

void noahos_audio_resume(void)
{
	jz47_av_decp->os_audio_resume ();
}

int noahos_audio_get_space(void)
{
	return jz47_av_decp->os_audio_get_space ();
}

int noahos_audio_play(void* data,int len,int flags)
{
	return jz47_av_decp->os_audio_play (data, len, flags);
}

float noahos_audio_get_delay(void)
{
	return jz47_av_decp->os_audio_get_delay ();
}

int noahos_audio_control(int cmd,void *arg)
{
	return jz47_av_decp->os_audio_control (cmd, arg);
}

#undef printf
#define printf(x,y...) noah_kprintf(x,##y)

void noah_kprintf(const char *format, ... )
{
	va_list va;
	char tmp[MSGSIZE_MAX];
	
	if( !jz47_av_decp->kprintf_enable )
		return;
	
	va_start(va, format);
	vsprintf(tmp, format, va);
	va_end(va);
	kprintf("%s", tmp);
}

void noah_open_osd(int w, int h)
{
	jz47_av_decp->os_open_video_osd(w,h);
}

int noah_get_ipu_status()
{
	return jz47_av_decp->fIpuEnable;
}

char* noah_get_file_name()
{
	return jz47_av_decp->FileName;
}

int noah_get_out_format()
{
	return jz47_av_decp->OutFormat;
}

int noah_long_jump(int err)
{
	if( err )
		return 0;

	if( jz47_av_decp->fMplayerInit )
		jz47_av_decp->os_audio_long_jump((void*)jz47_av_decp->stream,jz47_av_decp->AudioJumpBuf,1);

	return 1;
}
