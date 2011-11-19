#include "mplaylib.h"
#include "config.h"
#include "libaf/af_format.h"
#include "audio_out.h"
#include "audio_out_internal.h"

extern float noahos_audio_get_delay(void);

static ao_info_t info = 
{
	"RAW PCM/WAVE file writer audio output",
	"pcm",
	"Atmosfear",
	""
};

LIBAO_EXTERN(noahos)

// to set/get/query special features/parameters
static int control(int cmd,void *arg){
    return noahos_audio_control (cmd, arg);
}

// open & setup audio device
// return: 1=success 0=fail
static int init(int rate,int channels,int format,int flags){

    ao_data.buffersize= 4096*4;
    ao_data.outburst= 4096;
    ao_data.channels=channels;
    ao_data.samplerate=rate;
    ao_data.format=format;
    ao_data.bps=channels*rate;
    if (format != AF_FORMAT_U8 && format != AF_FORMAT_S8)
	    ao_data.bps*=2;
    
    return noahos_audio_init (rate, channels, format, flags);
}

// close audio device
static void uninit(int immed)
{
  noahos_audio_uninit (immed);
}

// stop playing and empty buffers (for seeking/pause)
static void reset(void)
{
    noahos_audio_reset ();
}
// stop playing, keep buffers (for pause)
static void audio_pause(void)
{
  noahos_audio_pause ();
}

// resume playing, after audio_pause()
static void audio_resume(void)
{
	return noahos_audio_resume();
}

// return: how many bytes can be played without blocking
static int get_space(void)
{
  return noahos_audio_get_space ();
}

// plays 'len' bytes of 'data'
// it should round it down to outburst*n
// return: number of bytes played
static int play(void* data,int len,int flags)
{
  return noahos_audio_play (data, len, ((flags & AOPLAY_FINAL_CHUNK) != 0));
}

// return: delay in seconds between first and last sample in buffer
static float get_delay(void)
{
  return noahos_audio_get_delay ();
}

