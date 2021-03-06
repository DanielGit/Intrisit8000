/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/// \file
/// \ingroup Properties Command2Property OSDMsgStack

#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>

#ifdef __MINIOS__
#include <mplaylib.h>
#else
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#if defined(__MINGW32__) || defined(__CYGWIN__)
#define _UWIN 1  /*disable Non-underscored versions of non-ANSI functions as otherwise int eof would conflict with eof()*/
#include <windows.h>
#endif

#ifndef __MINGW32__
#include <sys/ioctl.h>
#include <sys/wait.h>
#else
#define	SIGHUP	1	/* hangup */
#define	SIGQUIT	3	/* quit */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGPIPE	13	/* broken pipe */
#endif

#ifdef __MINIOS__
#undef HAVE_RTC
#endif

#ifdef HAVE_RTC
#ifdef __linux__
#include <linux/rtc.h>
#else
#include <rtc.h>
#define RTC_IRQP_SET RTCIO_IRQP_SET
#define RTC_PIE_ON   RTCIO_PIE_ON
#endif /* __linux__ */
#endif /* HAVE_RTC */

/*
 * In Mac OS X the SDL-lib is built upon Cocoa. The easiest way to
 * make it all work is to use the builtin SDL-bootstrap code, which
 * will be done automatically by replacing our main() if we include SDL.h.
 */
#if defined(__APPLE__) && defined(CONFIG_SDL)
#ifdef CONFIG_SDL_SDL_H
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif
#endif

#include "gui/interface.h"
#include "input/input.h"
#include "libao2/audio_out.h"
#include "libass/ass_mp.h"
#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "libmenu/menu.h"
#include "libmpcodecs/dec_audio.h"
#include "libmpcodecs/dec_video.h"
#include "libmpcodecs/mp_image.h"
#include "libmpcodecs/vd.h"
#include "libmpcodecs/vf.h"
#include "libmpdemux/demuxer.h"
#include "libmpdemux/stheader.h"
#include "libvo/font_load.h"
#include "libvo/sub.h"
#include "libvo/video_out.h"
#include "stream/cache2.h"
#include "stream/stream.h"
#include "stream/stream_bd.h"
#include "stream/stream_dvdnav.h"
#include "stream/stream_radio.h"
#include "stream/tv.h"
#include "access_mpcontext.h"
#include "cfg-mplayer-def.h"
#include "codec-cfg.h"
#include "command.h"
#include "edl.h"
#include "help_mp.h"
#include "m_config.h"
#include "m_option.h"
#include "m_property.h"
#include "m_struct.h"
#include "metadata.h"
#include "mixer.h"
#include "mp_core.h"
#include "mp_fifo.h"
#include "mp_msg.h"
#include "mpcommon.h"
#include "mplayer.h"
#include "osdep/getch2.h"
#include "osdep/priority.h"
#include "osdep/timer.h"
#include "parser-cfg.h"
#include "parser-mpcmd.h"
#include "path.h"
#include "playtree.h"
#include "playtreeparser.h"
#include "spudec.h"
#include "subreader.h"
#include "vobsub.h"
#include "eosd.h"
#include "osdep/getch2.h"
#include "osdep/timer.h"

#include "udp_sync.h"

#ifdef CONFIG_X11
#include "libvo/x11_common.h"
#endif
#ifdef CONFIG_DVBIN
#include "stream/dvbin.h"
#endif
#ifdef CONFIG_DVDREAD
#include "stream/stream_dvd.h"
#endif

#include "libjzcommon/jzasm.h"

int xpos=-1;
int ypos=-1;
int tv_mode=-1;

#ifdef JZC_CRC_VER
FILE *crc_fp;
#endif

#ifdef JZC_PMON_P0ed
extern FILE *pmon_p0_fp;
#endif

#ifdef JZ4740_MXU_OPT
unsigned int mxucr;
extern unsigned int enable_jz4740_mxu();
extern unsigned int disable_jz4740_mxu();
#endif

int slave_mode=0;
int player_idle_mode=0;
int quiet=1;
int enable_mouse_movements=0;
float start_volume = -1;
double start_pts = MP_NOPTS_VALUE;
char *heartbeat_cmd;
int use_jz_buf=0;

#define ROUND(x) ((int)((x)<0 ? (x)-0.5 : (x)+0.5))


m_config_t* mconfig;

//**************************************************************************//
//             Config file
//**************************************************************************//

static int cfg_inc_verbose(m_option_t *conf){ ++verbose; return 0;}

static int cfg_include(m_option_t *conf, char *filename){
	return m_config_parse_config_file(mconfig, filename);
}

static int max_framesize=0;

int noconsolecontrols=0;
//**************************************************************************//

// Not all functions in mplayer.c take the context as an argument yet
static MPContext mpctx_s = {
    .osd_function = OSD_PLAY,
    .begin_skip = MP_NOPTS_VALUE,
    .play_tree_step = 1,
    .global_sub_pos = -1,
    .set_of_sub_pos = -1,
    .file_format = DEMUXER_TYPE_UNKNOWN,
    .loop_times = -1,
#ifdef CONFIG_DVBIN
    .last_dvb_step = 1,
#endif
};

static MPContext *mpctx = &mpctx_s;

int fixed_vo=0;

// benchmark:
double video_time_usage=0;
double vout_time_usage=0;
static double audio_time_usage=0;
static int total_time_usage_start=0;
static int total_frame_cnt=0;
static int drop_frame_cnt=0; // total number of dropped frames
int benchmark=0;

// options:
#define DEFAULT_STARTUP_DECODE_RETRY 8
       int auto_quality=0;
static int output_quality=0;

float playback_speed=1.0;

int use_gui=0;

#ifdef CONFIG_GUI
int enqueue=0;
#endif

static int list_properties = 0;

int osd_level=1;
// if nonzero, hide current OSD contents when GetTimerMS() reaches this
unsigned int osd_visible;
int osd_duration = 1000;

int term_osd = 1;
static char* term_osd_esc = "\x1b[A\r\x1b[K";
static char* playing_msg = NULL;
// seek:
static double seek_to_sec;
static off_t seek_to_byte=0;
static off_t step_sec=0;
static int loop_seek=0;

static m_time_size_t end_at = { .type = END_AT_NONE, .pos = 0 };

// A/V sync:
       int autosync=30; // 30 might be a good default value.

// may be changed by GUI:  (FIXME!)
float rel_seek_secs=0;
int abs_seek_pos=0;

// codecs:
char **audio_codec_list=NULL; // override audio codec
char **video_codec_list=NULL; // override video codec
char **audio_fm_list=NULL;    // override audio codec family
char **video_fm_list=NULL;    // override video codec family

// streaming:
int audio_id=-1;
int video_id=-1;
int dvdsub_id=-1;
// this dvdsub_id was selected via slang
// use this to allow dvdnav to follow -slang across stream resets,
// in particular the subtitle ID for a language changes
int dvdsub_lang_id;
int vobsub_id=-1;
char* audio_lang=NULL;
char* dvdsub_lang=NULL;
static char* spudec_ifo=NULL;
char* filename=NULL; //"MI2-Trailer.avi";
int forced_subs_only=0;
int file_filter=1;

// cache2:
       int stream_cache_size=-1;
#ifdef CONFIG_STREAM_CACHE
extern int cache_fill_status;

float stream_cache_min_percent=20.0;
float stream_cache_seek_min_percent=50.0;
#else
#define cache_fill_status 0
#endif

// dump:
static char *stream_dump_name="stream.dump";
       int stream_dump_type=0;

// A-V sync:
static float default_max_pts_correction=-1;
static float max_pts_correction=0;//default_max_pts_correction;
static float c_total=0;
       float audio_delay=0;
static int ignore_start=0;

static int softsleep=0;

       double force_fps=0;
static int force_srate=0;
static int audio_output_format=-1; // AF_FORMAT_UNKNOWN
       int frame_dropping=1; // option  0=no drop  1= drop vo  2= drop decode
static int play_n_frames=-1;
static int play_n_frames_mf=-1;

// screen info:
char** video_driver_list=NULL;
char** audio_driver_list=NULL;

// sub:
char *font_name=NULL;
char *sub_font_name=NULL;
float font_factor=0.75;
char **sub_name=NULL;
float sub_delay=0;
float sub_fps=0;
int   sub_auto = 1;
char *vobsub_name=NULL;
/*DSP!!char *dsp=NULL;*/
int   subcc_enabled=0;
int suboverlap_enabled = 1;

char* current_module=NULL; // for debugging


#ifdef CONFIG_MENU
static vf_info_t* libmenu_vfs[] = {
  &vf_info_menu,
  NULL
};
static vf_instance_t* vf_menu = NULL;
int use_menu = 0;
static char* menu_cfg = NULL;
static char* menu_root = "main";
#endif


#ifdef HAVE_RTC
static int nortc = 1;
static char* rtc_device;
#endif

edl_record_ptr edl_records = NULL; ///< EDL entries memory area
edl_record_ptr next_edl_record = NULL; ///< only for traversing edl_records
short edl_decision = 0; ///< 1 when an EDL operation has been made.
short edl_needs_reset = 0; ///< 1 if we need to reset EDL next pointer
short edl_backward = 0; ///< 1 if we need to skip to the beginning of the next EDL record
FILE* edl_fd = NULL; ///< fd to write to when in -edlout mode.
// Number of seconds to add to the seek when jumping out
// of EDL scene in backward direction. This is needed to
// have some time after the seek to decide what to do next
// (next seek, pause,...), otherwise after the seek it will
// enter the same scene again and skip forward immediately
float edl_backward_delay = 2;
int edl_start_pts = 0; ///< Automatically add/sub this from EDL start/stop pos
int use_filedir_conf;
int use_filename_title;

unsigned int initialized_flags=0;

/// step size of mixer changes
int volstep = 3;

#ifdef CONFIG_CRASH_DEBUG
static char *prog_path;
static int crash_debug = 0;
#endif

/* This header requires all the global variable declarations. */
#include "cfg-mplayer.h"


#define mp_basename2(s) (strrchr(s,'/')==NULL?(char*)s:(strrchr(s,'/')+1))


//**************************************************************************//
//  minios system
//
#ifdef NOAH_OS
#include "mplayer_noahos.h"
#include "mp_core.h"

#define DISABLE_MAIN
#define PATH_MAX 4096
static MEDIALIB_STATUS PlayerStatus;
static int parameter_num;
static char* parameter_buf[2];
static float time_frame=0; // Timer
static int frame_time_remaining=0; // flag
static int blit_frame=0;
static int bOpenForInfo = 0;

extern Jz47_AV_Decoder *jz47_av_decp;
/*extern void  IpuOutmodeChanged(int x,int y,int w, int h);
extern char* noah_get_file_name();*/
extern int ipu_image_completed;
extern unsigned int mp_memory_empty;
#endif
int opt_exit = 0;
//**************************************************************************//
const void *mpctx_get_video_out(MPContext *mpctx)
{
    return mpctx->video_out;
}

const void *mpctx_get_audio_out(MPContext *mpctx)
{
    return mpctx->audio_out;
}

void *mpctx_get_demuxer(MPContext *mpctx)
{
    return mpctx->demuxer;
}

void *mpctx_get_playtree_iter(MPContext *mpctx)
{
    return mpctx->playtree_iter;
}

void *mpctx_get_mixer(MPContext *mpctx)
{
    return &mpctx->mixer;
}

int mpctx_get_global_sub_size(MPContext *mpctx)
{
    return mpctx->global_sub_size;
}

int mpctx_get_osd_function(MPContext *mpctx)
{
    return mpctx->osd_function;
}

static int is_valid_metadata_type (metadata_t type) {
  switch (type)
  {
  /* check for valid video stream */
  case META_VIDEO_CODEC:
  case META_VIDEO_BITRATE:
  case META_VIDEO_RESOLUTION:
  {
    if (!mpctx->sh_video)
      return 0;
    break;
  }

  /* check for valid audio stream */
  case META_AUDIO_CODEC:
  case META_AUDIO_BITRATE:
  case META_AUDIO_SAMPLES:
  {
    if (!mpctx->sh_audio)
      return 0;
    break;
  }

  /* check for valid demuxer */
  case META_INFO_TITLE:
  case META_INFO_ARTIST:
  case META_INFO_ALBUM:
  case META_INFO_YEAR:
  case META_INFO_COMMENT:
  case META_INFO_TRACK:
  case META_INFO_GENRE:
  {
    if (!mpctx->demuxer)
      return 0;
    break;
  }

  default:
    break;
  }

  return 1;
}

static char *get_demuxer_info (char *tag) {
  char **info = mpctx->demuxer->info;
  int n;

  if (!info || !tag)
    return NULL;

  for (n = 0; info[2*n] != NULL ; n++)
    if (!strcasecmp (info[2*n], tag))
      break;

  return info[2*n+1] ? strdup (info[2*n+1]) : NULL;
}

char *get_metadata (metadata_t type) {
  char *meta = NULL;
  sh_audio_t * const sh_audio = mpctx->sh_audio;
  sh_video_t * const sh_video = mpctx->sh_video;

  if (!is_valid_metadata_type (type))
    return NULL;

  switch (type)
  {
  case META_NAME:
  {
    return strdup (mp_basename2 (filename));
  }

  case META_VIDEO_CODEC:
  {
    if (sh_video->format == 0x10000001)
      meta = strdup ("mpeg1");
    else if (sh_video->format == 0x10000002)
      meta = strdup ("mpeg2");
    else if (sh_video->format == 0x10000004)
      meta = strdup ("mpeg4");
    else if (sh_video->format == 0x10000005)
      meta = strdup ("h264");
    else if (sh_video->format >= 0x20202020)
    {
      meta = malloc (8);
      sprintf (meta, "%.4s", (char *) &sh_video->format);
    }
    else
    {
      meta = malloc (8);
      sprintf (meta, "0x%08X", sh_video->format);
    }
    return meta;
  }

  case META_VIDEO_BITRATE:
  {
    meta = malloc (16);
    sprintf (meta, "%d kbps", (int) (sh_video->i_bps * 8 / 1024));
    return meta;
  }

  case META_VIDEO_RESOLUTION:
  {
    meta = malloc (16);
    sprintf (meta, "%d x %d", sh_video->disp_w, sh_video->disp_h);
    return meta;
  }

  case META_AUDIO_CODEC:
  {
    if (sh_audio->codec && sh_audio->codec->name)
      meta = strdup (sh_audio->codec->name);
    return meta;
  }

  case META_AUDIO_BITRATE:
  {
    meta = malloc (16);
    sprintf (meta, "%d kbps", (int) (sh_audio->i_bps * 8/1000));
    return meta;
  }

  case META_AUDIO_SAMPLES:
  {
    meta = malloc (16);
    sprintf (meta, "%d Hz, %d ch.", sh_audio->samplerate, sh_audio->channels);
    return meta;
  }

  /* check for valid demuxer */
  case META_INFO_TITLE:
    return get_demuxer_info ("Title");

  case META_INFO_ARTIST:
    return get_demuxer_info ("Artist");

  case META_INFO_ALBUM:
    return get_demuxer_info ("Album");

  case META_INFO_YEAR:
    return get_demuxer_info ("Year");

  case META_INFO_COMMENT:
    return get_demuxer_info ("Comment");

  case META_INFO_TRACK:
    return get_demuxer_info ("Track");

  case META_INFO_GENRE:
    return get_demuxer_info ("Genre");

  default:
    break;
  }

  return meta;
}

static void print_file_properties(const MPContext *mpctx, const char *filename)
{
  double video_start_pts = MP_NOPTS_VALUE;
  mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_FILENAME=%s\n",
	  filename_recode(filename));
  mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_DEMUXER=%s\n", mpctx->demuxer->desc->name);
  if (mpctx->sh_video) {
    /* Assume FOURCC if all bytes >= 0x20 (' ') */
    if (mpctx->sh_video->format >= 0x20202020)
	mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_VIDEO_FORMAT=%.4s\n", (char *)&mpctx->sh_video->format);
    else
	mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_VIDEO_FORMAT=0x%08X\n", mpctx->sh_video->format);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_VIDEO_BITRATE=%d\n", mpctx->sh_video->i_bps*8);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_VIDEO_WIDTH=%d\n", mpctx->sh_video->disp_w);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_VIDEO_HEIGHT=%d\n", mpctx->sh_video->disp_h);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_VIDEO_FPS=%5.3f\n", mpctx->sh_video->fps);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_VIDEO_ASPECT=%1.4f\n", mpctx->sh_video->aspect);
    video_start_pts = ds_get_next_pts(mpctx->d_video);
  }
  if (mpctx->sh_audio) {
    /* Assume FOURCC if all bytes >= 0x20 (' ') */
    if (mpctx->sh_audio->format >= 0x20202020)
      mp_msg(MSGT_IDENTIFY,MSGL_INFO, "ID_AUDIO_FORMAT=%.4s\n", (char *)&mpctx->sh_audio->format);
    else
      mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_AUDIO_FORMAT=%d\n", mpctx->sh_audio->format);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_AUDIO_BITRATE=%d\n", mpctx->sh_audio->i_bps*8);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_AUDIO_RATE=%d\n", mpctx->sh_audio->samplerate);
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_AUDIO_NCH=%d\n", mpctx->sh_audio->channels);
    start_pts = ds_get_next_pts(mpctx->d_audio);
  }
  if (video_start_pts != MP_NOPTS_VALUE) {
    if (start_pts == MP_NOPTS_VALUE || !mpctx->sh_audio ||
        (mpctx->sh_video && video_start_pts < start_pts))
      start_pts = video_start_pts;
  }
  if (start_pts != MP_NOPTS_VALUE)
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_START_TIME=%.2lf\n", start_pts);
  else
    mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_START_TIME=unknown\n");
  mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_LENGTH=%.2lf\n", demuxer_get_time_length(mpctx->demuxer));
  mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_SEEKABLE=%d\n",
         mpctx->stream->seek && (!mpctx->demuxer || mpctx->demuxer->seekable));
  if (mpctx->demuxer) {
      if (mpctx->demuxer->num_chapters == 0)
          stream_control(mpctx->demuxer->stream, STREAM_CTRL_GET_NUM_CHAPTERS, &mpctx->demuxer->num_chapters);
      mp_msg(MSGT_IDENTIFY,MSGL_INFO,"ID_CHAPTERS=%d\n", mpctx->demuxer->num_chapters);
  }
}

#ifdef CONFIG_DVDNAV
static void mp_dvdnav_context_free(MPContext *ctx){
    if (ctx->nav_smpi) free_mp_image(ctx->nav_smpi);
    ctx->nav_smpi = NULL;
    if (ctx->nav_buffer) free(ctx->nav_buffer);
    ctx->nav_buffer = NULL;
    ctx->nav_start = NULL;
    ctx->nav_in_size = 0;
}
#endif

void uninit_player(unsigned int mask){
  mask &= initialized_flags;

  mp_msg(MSGT_CPLAYER,MSGL_DBG2,"\n*** uninit(0x%X)\n",mask);

  if(mask&INITIALIZED_ACODEC){
    initialized_flags&=~INITIALIZED_ACODEC;
    current_module="uninit_acodec";
    if(mpctx->sh_audio) uninit_audio(mpctx->sh_audio);
#ifdef CONFIG_GUI
    if (use_gui) guiGetEvent(guiSetAfilter, (char *)NULL);
#endif
    mpctx->sh_audio=NULL;
    mpctx->mixer.afilter = NULL;
  }

  if(mask&INITIALIZED_VCODEC){
    initialized_flags&=~INITIALIZED_VCODEC;
    current_module="uninit_vcodec";
    if(mpctx->sh_video) uninit_video(mpctx->sh_video);
    mpctx->sh_video=NULL;
#ifdef CONFIG_MENU
    vf_menu=NULL;
#endif
  }

  if(mask&INITIALIZED_DEMUXER){
    initialized_flags&=~INITIALIZED_DEMUXER;
    current_module="free_demuxer";
    if(mpctx->demuxer){
	mpctx->stream=mpctx->demuxer->stream;
	free_demuxer(mpctx->demuxer);
    }
    mpctx->demuxer=NULL;
  }

  // kill the cache process:
  if(mask&INITIALIZED_STREAM){
    initialized_flags&=~INITIALIZED_STREAM;
    current_module="uninit_stream";
    if(mpctx->stream) free_stream(mpctx->stream);
    mpctx->stream=NULL;
  }

  if(mask&INITIALIZED_VO){
    initialized_flags&=~INITIALIZED_VO;
    current_module="uninit_vo";
#ifdef JZ47_OPT
#ifndef __MINIOS__
    set_lcd_fg0();
#endif
#endif
    mpctx->video_out->uninit();
    mpctx->video_out=NULL;
#ifdef CONFIG_DVDNAV
    mp_dvdnav_context_free(mpctx);
#endif
    if (vo_spudec){
      current_module="uninit_spudec";
      spudec_free(vo_spudec);
      vo_spudec=NULL;
    }
  }

  // Must be after libvo uninit, as few vo drivers (svgalib) have tty code.
#ifndef __MINIOS__
  if(mask&INITIALIZED_GETCH2){
    initialized_flags&=~INITIALIZED_GETCH2;
    current_module="uninit_getch2";
    mp_msg(MSGT_CPLAYER,MSGL_DBG2,"\n[[[uninit getch2]]]\n");
  // restore terminal:
    getch2_disable();
  }
#endif

  if(mask&INITIALIZED_VOBSUB){
    initialized_flags&=~INITIALIZED_VOBSUB;
    current_module="uninit_vobsub";
    if(vo_vobsub) vobsub_close(vo_vobsub);
    vo_vobsub=NULL;
  }

  if(mask&INITIALIZED_AO){
    initialized_flags&=~INITIALIZED_AO;
    current_module="uninit_ao";
    if (mpctx->edl_muted) mixer_mute(&mpctx->mixer);
    if (mpctx->audio_out) mpctx->audio_out->uninit(mpctx->eof?0:1);
    mpctx->audio_out=NULL;
  }

#ifdef CONFIG_GUI
  if(mask&INITIALIZED_GUI){
    initialized_flags&=~INITIALIZED_GUI;
    current_module="uninit_gui";
    guiDone();
  }
#endif

  if(mask&INITIALIZED_INPUT){
    initialized_flags&=~INITIALIZED_INPUT;
    current_module="uninit_input";
    mp_input_uninit();
#ifdef CONFIG_MENU
    if (use_menu)
      menu_uninit();
#endif
  }

#ifdef __MINIOS__
#ifndef NOAH_OS
	unsigned char my_err;
	if(mp_sem_pause)
		os_SemaphoreDelete ( mp_sem_pause , 1 , &my_err) ;
	if(mp_sem_seek)
		os_SemaphoreDelete ( mp_sem_seek , 1 , &my_err) ;
//	if(audio_sem_filemode)
//		os_SemaphoreDelete ( audio_sem_filemode , 1 , &my_err) ;
	mp_sem_pause = NULL;
	mp_sem_seek = NULL;
//	audio_sem_filemode = NULL;
#endif
#endif
  current_module=NULL;
}

void exit_player_with_rc(enum exit_reason how, int rc)
{
#ifdef __MINIOS__
#ifndef NOAH_OS
	if(videoshow)
	{
		if(!get_display_is_direct())
			lcd_reset_frame();
		else
			ipu_image_stop();
	}
#endif
#endif

#ifdef JZC_CRC_VER
  extern short crc_code;
  if(crc_fp != NULL){
    fprintf(crc_fp, "%s: \t0x%x\n", filename, crc_code);
    fclose(crc_fp);
  }
#endif //JZC_CRC_VER

#ifdef JZC_PMON_P0ed
  fprintf(pmon_p0_fp,"=======================================\n");
  if(pmon_p0_fp != NULL)
    fclose(pmon_p0_fp);
#endif //JZC_PMON_P0

#ifdef CONFIG_NETWORKING
  if (udp_master)
    send_udp(udp_ip, udp_port, "bye");
#endif /* CONFIG_NETWORKING */

  if (mpctx->user_muted && !mpctx->edl_muted) mixer_mute(&mpctx->mixer);
  uninit_player(INITIALIZED_ALL);
#if defined(__MINGW32__) || defined(__CYGWIN__)
  timeEndPeriod(1);
#endif
#ifdef CONFIG_X11
#ifdef CONFIG_GUI
  if ( !use_gui )
#endif
  vo_uninit();	// Close the X11 connection (if any is open).
#endif

#ifdef CONFIG_FREETYPE
  current_module="uninit_font";
  if (sub_font && sub_font != vo_font) free_font_desc(sub_font);
  sub_font = NULL;
  if (vo_font) free_font_desc(vo_font);
  vo_font = NULL;
  done_freetype();
#endif
  free_osd_list();

#ifdef CONFIG_ASS
  ass_library_done(ass_library);
  ass_library = NULL;
#endif

#ifdef JZ4740_MXU_OPT
  disable_jz4740_mxu (mxucr);
#endif

  current_module="exit_player";

// free mplayer config
  if(mconfig)
    m_config_free(mconfig);
  mconfig = NULL;

  if(mpctx->playtree_iter)
    play_tree_iter_free(mpctx->playtree_iter);
  mpctx->playtree_iter = NULL;
  if(mpctx->playtree)
    play_tree_free(mpctx->playtree, 1);
  mpctx->playtree = NULL;


  if(edl_records != NULL) free(edl_records); // free mem allocated for EDL
  edl_records = NULL;
  switch(how) {
  case EXIT_QUIT:
    mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_ExitingHow,MSGTR_Exit_quit);
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_EXIT=QUIT\n");
    break;
  case EXIT_EOF:
    mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_ExitingHow,MSGTR_Exit_eof);
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_EXIT=EOF\n");
    break;
  case EXIT_ERROR:
    mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_ExitingHow,MSGTR_Exit_error);
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_EXIT=ERROR\n");
    break;
  default:
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_EXIT=NONE\n");
  }
  mp_msg(MSGT_CPLAYER,MSGL_DBG2,"max framesize was %d bytes\n",max_framesize);
#ifdef JZ47_OPT
#ifndef __MINIOS__
  jz47_exit_memalloc ();
#endif
#endif
#ifdef JZC_HW_MEDIA
  VAE_unmap();
#endif

#ifndef __MINIOS__
  exit(rc);
#endif
}

void exit_player(enum exit_reason how)
{
  exit_player_with_rc(how, 1);
}

#ifndef __MINIOS__
#ifndef __MINGW32__
static void child_sighandler(int x){
  pid_t pid;
  while((pid=waitpid(-1,NULL,WNOHANG)) > 0);
}
#endif

static void exit_sighandler(int x){
  static int sig_count=0;
#ifdef CONFIG_CRASH_DEBUG
  if (!crash_debug || x != SIGTRAP)
#endif
  ++sig_count;
  if(initialized_flags==0 && sig_count>1) exit(1);
  if(sig_count==5)
    {
      /* We're crashing bad and can't uninit cleanly :(
       * by popular request, we make one last (dirty)
       * effort to restore the user's
       * terminal. */
      getch2_disable();
      exit(1);
    }
  if(sig_count==6) exit(1);
  if(sig_count>6){
    // can't stop :(
#ifndef __MINGW32__
    kill(getpid(),SIGKILL);
#endif
  }
  mp_msg(MSGT_CPLAYER,MSGL_FATAL,"\n" MSGTR_IntBySignal,x,
      current_module?current_module:"unknown"
  );
  mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_SIGNAL=%d\n", x);
  if(sig_count<=1)
  switch(x){
  case SIGINT:
  case SIGPIPE:
  case SIGQUIT:
  case SIGTERM:
  case SIGKILL:
      async_quit_request = 1;
      return;  // killed from keyboard (^C) or killed [-9]
  case SIGILL:
#if CONFIG_RUNTIME_CPUDETECT
      mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_Exit_SIGILL_RTCpuSel);
#else
      mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_Exit_SIGILL);
#endif
  case SIGFPE:
  case SIGSEGV:
      mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_Exit_SIGSEGV_SIGFPE);
  default:
      mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_Exit_SIGCRASH);
#ifdef CONFIG_CRASH_DEBUG
      if (crash_debug) {
        int gdb_pid;
        mp_msg(MSGT_CPLAYER, MSGL_INFO, "Forking...\n");
        gdb_pid = fork();
        mp_msg(MSGT_CPLAYER, MSGL_INFO, "Forked...\n");
        if (gdb_pid == 0) { // We are the child
          char spid[20];
          snprintf(spid, sizeof(spid), "%i", getppid());
          getch2_disable(); // allow terminal to work properly with gdb
          if (execlp("gdb", "gdb", prog_path, spid, "-ex", "bt", NULL) == -1)
            mp_msg(MSGT_CPLAYER, MSGL_ERR, "Couldn't start gdb\n");
        } else if (gdb_pid < 0)
          mp_msg(MSGT_CPLAYER, MSGL_ERR, "Couldn't fork\n");
        else {
          waitpid(gdb_pid, NULL, 0);
        }
        if (x == SIGTRAP) return;
      }
#endif
  }
  getch2_disable();
  exit(1);
}

static void parse_cfgfiles( m_config_t* conf )
{
char *conffile;
int conffile_fd;
if (!disable_system_conf &&
    m_config_parse_config_file(conf, MPLAYER_CONFDIR "/mplayer.conf") < 0)
  exit_player(EXIT_NONE);
if ((conffile = get_path("")) == NULL) {
  mp_msg(MSGT_CPLAYER,MSGL_WARN,MSGTR_NoHomeDir);
} else {
#ifdef __MINGW32__
  mkdir(conffile);
#else
  mkdir(conffile, 0777);
#endif
  free(conffile);
  if ((conffile = get_path("config")) == NULL) {
    mp_msg(MSGT_CPLAYER,MSGL_ERR,MSGTR_GetpathProblem);
  } else {
    if ((conffile_fd = open(conffile, O_CREAT | O_EXCL | O_WRONLY, 0666)) != -1) {
      mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_CreatingCfgFile, conffile);
      write(conffile_fd, default_config, strlen(default_config));
      close(conffile_fd);
    }
    if (!disable_user_conf &&
        m_config_parse_config_file(conf, conffile) < 0)
      exit_player(EXIT_NONE);
    free(conffile);
  }
}
}
#endif

#define PROFILE_CFG_PROTOCOL "protocol."

static void load_per_protocol_config (m_config_t* conf, const char *const file)
{
    char *str;
    char protocol[strlen (PROFILE_CFG_PROTOCOL) + strlen (file) + 1];
    m_profile_t *p;

    /* does filename actually uses a protocol ? */
    str = strstr (file, "://");
    if (!str)
        return;

    sprintf (protocol, "%s%s", PROFILE_CFG_PROTOCOL, file);
    protocol[strlen (PROFILE_CFG_PROTOCOL)+strlen(file)-strlen(str)] = '\0';
    p = m_config_get_profile (conf, protocol);
    if (p)
    {
        mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_LoadingProtocolProfile, protocol);
        m_config_set_profile(conf,p);
    }
}

#define PROFILE_CFG_EXTENSION "extension."

static void load_per_extension_config (m_config_t* conf, const char *const file)
{
    char *str;
    char extension[strlen (PROFILE_CFG_EXTENSION) + 8];
    m_profile_t *p;

    /* does filename actually have an extension ? */
    str = strrchr (filename, '.');
    if (!str)
        return;

    sprintf (extension, PROFILE_CFG_EXTENSION);
    strncat (extension, ++str, 7);
    p = m_config_get_profile (conf, extension);
    if (p)
    {
      mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_LoadingExtensionProfile, extension);
      m_config_set_profile(conf,p);
    }
}

#define PROFILE_CFG_VO "vo."
#define PROFILE_CFG_AO "ao."

static void load_per_output_config (m_config_t* conf, char *cfg, char *out)
{
    char profile[strlen (cfg) + strlen (out) + 1];
    m_profile_t *p;

    sprintf (profile, "%s%s", cfg, out);
    p = m_config_get_profile (conf, profile);
    if (p)
    {
      mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_LoadingExtensionProfile, profile);
      m_config_set_profile(conf,p);
    }
}

/**
 * Tries to load a config file
 * @return 0 if file was not found, 1 otherwise
 */
static int try_load_config(m_config_t *conf, const char *file)
{
    struct stat st;
    if (stat(file, &st))
        return 0;
    mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_LoadingConfig, file);
    m_config_parse_config_file (conf, file);
    return 1;
}

static void load_per_file_config (m_config_t* conf, const char *const file)
{
    char *confpath;
    char cfg[PATH_MAX];
    char *name;

    if (strlen(file) > PATH_MAX - 14) {
        mp_msg(MSGT_CPLAYER, MSGL_WARN, "Filename is too long, can not load file or directory specific config files\n");
        return;
    }
    sprintf (cfg, "%s.conf", file);

    name = strrchr(cfg, '/');
    if (HAVE_DOS_PATHS) {
        char *tmp = strrchr(cfg, '\\');
        if (!name || tmp > name)
            name = tmp;
        tmp = strrchr(cfg, ':');
        if (!name || tmp > name)
            name = tmp;
    }
    if (!name)
	name = cfg;
    else
	name++;

    if (use_filedir_conf) {
        char dircfg[PATH_MAX];
        strcpy(dircfg, cfg);
        strcpy(dircfg + (name - cfg), "mplayer.conf");
        try_load_config(conf, dircfg);

        if (try_load_config(conf, cfg))
            return;
    }

    if ((confpath = get_path (name)) != NULL)
    {
	try_load_config(conf, confpath);

	free (confpath);
    }
}

/* When libmpdemux performs a blocking operation (network connection or
 * cache filling) if the operation fails we use this function to check
 * if it was interrupted by the user.
 * The function returns a new value for eof. */
static int libmpdemux_was_interrupted(int eof) {
  mp_cmd_t* cmd;
  if((cmd = mp_input_get_cmd(0,0,0)) != NULL) {
       switch(cmd->id) {
       case MP_CMD_QUIT:
	 exit_player_with_rc(EXIT_QUIT, (cmd->nargs > 0)? cmd->args[0].v.i : 0);
       case MP_CMD_PLAY_TREE_STEP: {
	 eof = (cmd->args[0].v.i > 0) ? PT_NEXT_ENTRY : PT_PREV_ENTRY;
	 mpctx->play_tree_step = (cmd->args[0].v.i == 0) ? 1 : cmd->args[0].v.i;
       } break;
       case MP_CMD_PLAY_TREE_UP_STEP: {
	 eof = (cmd->args[0].v.i > 0) ? PT_UP_NEXT : PT_UP_PREV;
       } break;
       case MP_CMD_PLAY_ALT_SRC_STEP: {
	 eof = (cmd->args[0].v.i > 0) ?  PT_NEXT_SRC : PT_PREV_SRC;
       } break;
       }
       mp_cmd_free(cmd);
  }
  return eof;
}

#define mp_basename(s) (strrchr(s,'\\')==NULL?(mp_basename2(s)):(strrchr(s,'\\')+1))

static int playtree_add_playlist(play_tree_t* entry)
{
  play_tree_add_bpf(entry,filename);

#ifdef CONFIG_GUI
  if (use_gui) {
    if (entry) {
      import_playtree_playlist_into_gui(entry, mconfig);
      play_tree_free_list(entry,1);
    }
  } else
#endif
  {
  if(!entry) {
    entry = mpctx->playtree_iter->tree;
    if(play_tree_iter_step(mpctx->playtree_iter,1,0) != PLAY_TREE_ITER_ENTRY) {
        return PT_NEXT_ENTRY;
    }
    if(mpctx->playtree_iter->tree == entry ) { // Loop with a single file
      if(play_tree_iter_up_step(mpctx->playtree_iter,1,0) != PLAY_TREE_ITER_ENTRY) {
	return PT_NEXT_ENTRY;
      }
    }
    play_tree_remove(entry,1,1);
    return PT_NEXT_SRC;
  }
  play_tree_insert_entry(mpctx->playtree_iter->tree,entry);
  play_tree_set_params_from(entry,mpctx->playtree_iter->tree);
  entry = mpctx->playtree_iter->tree;
  if(play_tree_iter_step(mpctx->playtree_iter,1,0) != PLAY_TREE_ITER_ENTRY) {
    return PT_NEXT_ENTRY;
  }
  play_tree_remove(entry,1,1);
  }
  return PT_NEXT_SRC;
}

void add_subtitles(char *filename, float fps, int noerr)
{
    sub_data *subd;
#ifdef CONFIG_ASS
    ASS_Track *asst = 0;
#endif

    if (filename == NULL || mpctx->set_of_sub_size >= MAX_SUBTITLE_FILES) {
	return;
    }

    subd = sub_read_file(filename, fps);
#ifdef CONFIG_ASS
    if (ass_enabled)
#ifdef CONFIG_ICONV
        asst = ass_read_stream(ass_library, filename, sub_cp);
#else
        asst = ass_read_stream(ass_library, filename, 0);
#endif
    if (ass_enabled && subd && !asst)
        asst = ass_read_subdata(ass_library, subd, fps);

    if (!asst && !subd)
#else
    if(!subd)
#endif
        mp_msg(MSGT_CPLAYER, noerr ? MSGL_WARN : MSGL_ERR, MSGTR_CantLoadSub,
		filename_recode(filename));

#ifdef CONFIG_ASS
    if (!asst && !subd) return;
    mpctx->set_of_ass_tracks[mpctx->set_of_sub_size] = asst;
#else
    if (!subd) return;
#endif
    mpctx->set_of_subtitles[mpctx->set_of_sub_size] = subd;
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_FILE_SUB_ID=%d\n", mpctx->set_of_sub_size);
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_FILE_SUB_FILENAME=%s\n",
	   filename_recode(filename));
    ++mpctx->set_of_sub_size;
    mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_AddedSubtitleFile, mpctx->set_of_sub_size,
	    filename_recode(filename));
}

// FIXME: if/when the GUI calls this, global sub numbering gets (potentially) broken.
void update_set_of_subtitles(void)
    // subdata was changed, set_of_sub... have to be updated.
{
    sub_data ** const set_of_subtitles = mpctx->set_of_subtitles;
    int i;
    if (mpctx->set_of_sub_size > 0 && subdata == NULL) { // *subdata was deleted
        for (i = mpctx->set_of_sub_pos + 1; i < mpctx->set_of_sub_size; ++i)
            set_of_subtitles[i-1] = set_of_subtitles[i];
        set_of_subtitles[mpctx->set_of_sub_size-1] = NULL;
        --mpctx->set_of_sub_size;
        if (mpctx->set_of_sub_size > 0) subdata = set_of_subtitles[mpctx->set_of_sub_pos=0];
    }
    else if (mpctx->set_of_sub_size > 0 && subdata != NULL) { // *subdata was changed
        set_of_subtitles[mpctx->set_of_sub_pos] = subdata;
    }
    else if (mpctx->set_of_sub_size <= 0 && subdata != NULL) { // *subdata was added
        set_of_subtitles[mpctx->set_of_sub_pos=mpctx->set_of_sub_size] = subdata;
        ++mpctx->set_of_sub_size;
    }
}

void init_vo_spudec(void) {
  if (vo_spudec)
    spudec_free(vo_spudec);
  vo_spudec = NULL;

  // we currently can't work without video stream
  if (!mpctx->sh_video)
    return;

  if (spudec_ifo) {
    unsigned int palette[16], width, height;
    current_module="spudec_init_vobsub";
    if (vobsub_parse_ifo(NULL,spudec_ifo, palette, &width, &height, 1, -1, NULL) >= 0)
      vo_spudec=spudec_new_scaled(palette, width, height, NULL, 0);
  }

#ifdef CONFIG_DVDREAD
  if (vo_spudec==NULL && mpctx->stream->type==STREAMTYPE_DVD) {
    current_module="spudec_init_dvdread";
    vo_spudec=spudec_new_scaled(((dvd_priv_t *)(mpctx->stream->priv))->cur_pgc->palette,
                                mpctx->sh_video->disp_w, mpctx->sh_video->disp_h,
                                NULL, 0);
  }
#endif

#ifdef CONFIG_DVDNAV
  if (vo_spudec==NULL && mpctx->stream->type==STREAMTYPE_DVDNAV) {
    unsigned int *palette = mp_dvdnav_get_spu_clut(mpctx->stream);
    current_module="spudec_init_dvdnav";
    vo_spudec=spudec_new_scaled(palette, mpctx->sh_video->disp_w, mpctx->sh_video->disp_h, NULL, 0);
  }
#endif

  if (vo_spudec==NULL) {
    sh_sub_t *sh = mpctx->d_sub->sh;
    current_module="spudec_init_normal";
    vo_spudec=spudec_new_scaled(NULL, mpctx->sh_video->disp_w, mpctx->sh_video->disp_h, sh->extradata, sh->extradata_len);
    spudec_set_font_factor(vo_spudec,font_factor);
  }

  if (vo_spudec!=NULL) {
    mp_property_do("sub_forced_only", M_PROPERTY_SET, &forced_subs_only, mpctx);
  }
}

#ifndef __MINIOS__
/**
 * \brief append a formatted string
 * \param buf buffer to print into
 * \param pos position of terminating 0 in buf
 * \param len maximum number of characters in buf, not including terminating 0
 * \param format printf format string
 */
static void saddf(char *buf, unsigned *pos, int len, const char *format, ...)
{
  va_list va;
  va_start(va, format);
  *pos += vsnprintf(&buf[*pos], len - *pos, format, va);
  va_end(va);
  if (*pos >= len ) {
    buf[len] = 0;
    *pos = len;
  }
}

/**
 * \brief append time in the hh:mm:ss.f format
 * \param buf buffer to print into
 * \param pos position of terminating 0 in buf
 * \param len maximum number of characters in buf, not including terminating 0
 * \param time time value to convert/append
 */
static void sadd_hhmmssf(char *buf, unsigned *pos, int len, float time) {
  int64_t tenths = 10 * time;
  int f1 = tenths % 10;
  int ss = (tenths /  10) % 60;
  int mm = (tenths / 600) % 60;
  int hh = tenths / 36000;
  if (time <= 0) {
    saddf(buf, pos, len, "unknown");
    return;
  }
  if (hh > 0)
    saddf(buf, pos, len, "%2d:", hh);
  if (hh > 0 || mm > 0)
    saddf(buf, pos, len, "%02d:", mm);
  saddf(buf, pos, len, "%02d.%1d", ss, f1);
}

/**
 * \brief print the status line
 * \param a_pos audio position
 * \param a_v A-V desynchronization
 * \param corr amount out A-V synchronization
 */
static void print_status(float a_pos, float a_v, float corr)
{
  sh_video_t * const sh_video = mpctx->sh_video;
  int width;
  char *line;
  unsigned pos = 0;
  get_screen_size();
  if (screen_width > 0)
    width = screen_width;
  else
  width = 80;
#if defined(__MINGW32__) || defined(__CYGWIN__) || defined(__OS2__)
  /* Windows command line is broken (MinGW's rxvt works, but we
   * should not depend on that). */
  width--;
#endif
  line = malloc(width + 1); // one additional char for the terminating null

  // Audio time
  if (mpctx->sh_audio) {
    saddf(line, &pos, width, "A:%6.1f ", a_pos);
    if (!sh_video) {
      float len = demuxer_get_time_length(mpctx->demuxer);
      saddf(line, &pos, width, "(");
      sadd_hhmmssf(line, &pos, width, a_pos);
      saddf(line, &pos, width, ") of %.1f (", len);
      sadd_hhmmssf(line, &pos, width, len);
      saddf(line, &pos, width, ") ");
    }
  }

  // Video time
  if (sh_video)
    saddf(line, &pos, width, "V:%6.1f ", sh_video->pts);

  // A-V sync
  if (mpctx->sh_audio && sh_video)
    saddf(line, &pos, width, "A-V:%7.3f ct:%7.3f ", a_v, corr);

  // Video stats
  if (sh_video)
    saddf(line, &pos, width, "%3d/%3d ",
      (int)sh_video->num_frames,
      (int)sh_video->num_frames_decoded);

  // CPU usage
  if (sh_video) {
    if (sh_video->timer > 0.5)
      saddf(line, &pos, width, "%2d%% %2d%% %4.1f%% ",
        (int)(100.0*video_time_usage*playback_speed/(double)sh_video->timer),
        (int)(100.0*vout_time_usage*playback_speed/(double)sh_video->timer),
        (100.0*audio_time_usage*playback_speed/(double)sh_video->timer));
    else
      saddf(line, &pos, width, "??%% ??%% ??,?%% ");
  } else if (mpctx->sh_audio) {
    if (mpctx->delay > 0.5)
      saddf(line, &pos, width, "%4.1f%% ",
        100.0*audio_time_usage/(double)mpctx->delay);
    else
      saddf(line, &pos, width, "??,?%% ");
  }

  // VO stats
  if (sh_video)
    saddf(line, &pos, width, "%d %d ", drop_frame_cnt, output_quality);

#ifdef CONFIG_STREAM_CACHE
  // cache stats
  if (stream_cache_size > 0)
    saddf(line, &pos, width, "%d%% ", cache_fill_status);
#endif

  // other
  if (playback_speed != 1)
    saddf(line, &pos, width, "%4.2fx ", playback_speed);

  // end
  if (erase_to_end_of_line) {
    line[pos] = 0;
    mp_msg(MSGT_STATUSLINE, MSGL_STATUS, "%s%s\r", line, erase_to_end_of_line);
  } else {
    memset(&line[pos], ' ', width - pos);
    line[width] = 0;
    mp_msg(MSGT_STATUSLINE, MSGL_STATUS, "%s\r", line);
  }
  free(line);
}
#endif

/**
 * \brief build a chain of audio filters that converts the input format
 * to the ao's format, taking into account the current playback_speed.
 * \param sh_audio describes the requested input format of the chain.
 * \param ao_data describes the requested output format of the chain.
 */
int build_afilter_chain(sh_audio_t *sh_audio, ao_data_t *ao_data)
{
  int new_srate;
  int result;
  if (!sh_audio)
  {
#ifdef CONFIG_GUI
    if (use_gui) guiGetEvent(guiSetAfilter, (char *)NULL);
#endif
    mpctx->mixer.afilter = NULL;
    return 0;
  }
  if(af_control_any_rev(sh_audio->afilter,
                        AF_CONTROL_PLAYBACK_SPEED | AF_CONTROL_SET,
                        &playback_speed)) {
    new_srate = sh_audio->samplerate;
  } else {
    new_srate = sh_audio->samplerate * playback_speed;
    if (new_srate != ao_data->samplerate) {
      // limits are taken from libaf/af_resample.c
      if (new_srate < 8000)
        new_srate = 8000;
      if (new_srate > 192000)
        new_srate = 192000;
      playback_speed = (float)new_srate / (float)sh_audio->samplerate;
    }
  }
  result =  init_audio_filters(sh_audio, new_srate,
           &ao_data->samplerate, &ao_data->channels, &ao_data->format);
  mpctx->mixer.afilter = sh_audio->afilter;
#ifdef CONFIG_GUI
  if (use_gui) guiGetEvent(guiSetAfilter, (char *)sh_audio->afilter);
#endif
  return result;
}


typedef struct mp_osd_msg mp_osd_msg_t;
struct mp_osd_msg {
    /// Previous message on the stack.
    mp_osd_msg_t* prev;
    /// Message text.
    char msg[128];
    int  id,level,started;
    /// Display duration in ms.
    unsigned  time;
};

/// OSD message stack.
static mp_osd_msg_t* osd_msg_stack = NULL;

/**
 *  \brief Add a message on the OSD message stack
 *
 *  If a message with the same id is already present in the stack
 *  it is pulled on top of the stack, otherwise a new message is created.
 *
 */

void set_osd_msg(int id, int level, int time, const char* fmt, ...) {
    mp_osd_msg_t *msg,*last=NULL;
    va_list va;
    int r;

    // look if the id is already in the stack
    for(msg = osd_msg_stack ; msg && msg->id != id ;
	last = msg, msg = msg->prev);
    // not found: alloc it
    if(!msg) {
        msg = calloc(1,sizeof(mp_osd_msg_t));
        msg->prev = osd_msg_stack;
        osd_msg_stack = msg;
    } else if(last) { // found, but it's not on top of the stack
        last->prev = msg->prev;
        msg->prev = osd_msg_stack;
        osd_msg_stack = msg;
    }
    // write the msg
    va_start(va,fmt);
    r = vsnprintf(msg->msg, 128, fmt, va);
    va_end(va);
    if(r >= 128) msg->msg[127] = 0;
    // set id and time
    msg->id = id;
    msg->level = level;
    msg->time = time;

}

/**
 *  \brief Remove a message from the OSD stack
 *
 *  This function can be used to get rid of a message right away.
 *
 */

void rm_osd_msg(int id) {
    mp_osd_msg_t *msg,*last=NULL;

    // Search for the msg
    for(msg = osd_msg_stack ; msg && msg->id != id ;
	last = msg, msg = msg->prev);
    if(!msg) return;

    // Detach it from the stack and free it
    if(last)
        last->prev = msg->prev;
    else
        osd_msg_stack = msg->prev;
    free(msg);
}

/**
 *  \brief Remove all messages from the OSD stack
 *
 */

static void clear_osd_msgs(void) {
    mp_osd_msg_t* msg = osd_msg_stack, *prev = NULL;
    while(msg) {
        prev = msg->prev;
        free(msg);
        msg = prev;
    }
    osd_msg_stack = NULL;
}

/**
 *  \brief Get the current message from the OSD stack.
 *
 *  This function decrements the message timer and destroys the old ones.
 *  The message that should be displayed is returned (if any).
 *
 */

static mp_osd_msg_t* get_osd_msg(void) {
    mp_osd_msg_t *msg,*prev,*last = NULL;
    static unsigned last_update = 0;
    unsigned now = GetTimerMS();
    unsigned diff;
    char hidden_dec_done = 0;

    if (osd_visible) {
	// 36000000 means max timed visibility is 1 hour into the future, if
	// the difference is greater assume it's wrapped around from below 0
	if (osd_visible - now > 36000000) {
	    osd_visible = 0;
	    vo_osd_progbar_type = -1; // disable
	    vo_osd_changed(OSDTYPE_PROGBAR);
	    if (mpctx->osd_function != OSD_PAUSE)
		mpctx->osd_function = OSD_PLAY;
	}
    }

    if(!last_update) last_update = now;
    diff = now >= last_update ? now - last_update : 0;

    last_update = now;

    // Look for the first message in the stack with high enough level.
    for(msg = osd_msg_stack ; msg ; last = msg, msg = prev) {
        prev = msg->prev;
        if(msg->level > osd_level && hidden_dec_done) continue;
        // The message has a high enough level or it is the first hidden one
        // in both cases we decrement the timer or kill it.
        if(!msg->started || msg->time > diff) {
            if(msg->started) msg->time -= diff;
            else msg->started = 1;
            // display it
            if(msg->level <= osd_level) return msg;
            hidden_dec_done = 1;
            continue;
        }
        // kill the message
        free(msg);
        if(last) {
            last->prev = prev;
            msg = last;
        } else {
            osd_msg_stack = prev;
            msg = NULL;
        }
    }
    // Nothing found
    return NULL;
}

/**
 * \brief Display the OSD bar.
 *
 * Display the OSD bar or fall back on a simple message.
 *
 */

void set_osd_bar(int type,const char* name,double min,double max,double val) {

    if(osd_level < 1) return;

    if(mpctx->sh_video) {
        osd_visible = (GetTimerMS() + 1000) | 1;
        vo_osd_progbar_type = type;
        vo_osd_progbar_value = 256*(val-min)/(max-min);
        vo_osd_changed(OSDTYPE_PROGBAR);
        return;
    }

    set_osd_msg(OSD_MSG_BAR,1,osd_duration,"%s: %d %%",
                name,ROUND(100*(val-min)/(max-min)));
}

/**
 * \brief Display text subtitles on the OSD
 */
void set_osd_subtitle(subtitle *subs) {
    int i;
    vo_sub = subs;
    vo_osd_changed(OSDTYPE_SUBTITLE);
    if (!mpctx->sh_video) {
        // reverse order, since newest set_osd_msg is displayed first
        for (i = SUB_MAX_TEXT - 1; i >= 0; i--) {
            if (!subs || i >= subs->lines || !subs->text[i])
                rm_osd_msg(OSD_MSG_SUB_BASE + i);
            else {
                // HACK: currently display time for each sub line except the last is set to 2 seconds.
                int display_time = i == subs->lines - 1 ? 180000 : 2000;
                set_osd_msg(OSD_MSG_SUB_BASE + i, 1, display_time, "%s", subs->text[i]);
            }
        }
    }
}

/**
 * \brief Update the OSD message line.
 *
 * This function displays the current message on the vo OSD or on the term.
 * If the stack is empty and the OSD level is high enough the timer
 * is displayed (only on the vo OSD).
 *
 */

static void update_osd_msg(void) {
    mp_osd_msg_t *msg;
    static char osd_text[128] = "";
    static char osd_text_timer[128];

    // we need some mem for vo_osd_text
    vo_osd_text = (unsigned char*)osd_text;

    // Look if we have a msg
    if((msg = get_osd_msg())) {
        if(strcmp(osd_text,msg->msg)) {
            strncpy((char*)osd_text, msg->msg, 127);
            if(mpctx->sh_video) vo_osd_changed(OSDTYPE_OSD); else
            if(term_osd) mp_msg(MSGT_CPLAYER,MSGL_STATUS,"%s%s\n",term_osd_esc,msg->msg);
        }
        return;
    }

    if(mpctx->sh_video) {
        // fallback on the timer
        if(osd_level>=2) {
            int len = demuxer_get_time_length(mpctx->demuxer);
            int percentage = -1;
            char percentage_text[10];
            int pts = demuxer_get_current_time(mpctx->demuxer);

            if (mpctx->osd_show_percentage)
                percentage = demuxer_get_percent_pos(mpctx->demuxer);

            if (percentage >= 0)
                snprintf(percentage_text, 9, " (%d%%)", percentage);
            else
                percentage_text[0] = 0;

            if (osd_level == 3)
                snprintf(osd_text_timer, 63,
                         "%c %02d:%02d:%02d / %02d:%02d:%02d%s",
                         mpctx->osd_function,pts/3600,(pts/60)%60,pts%60,
                         len/3600,(len/60)%60,len%60,percentage_text);
            else
                snprintf(osd_text_timer, 63, "%c %02d:%02d:%02d%s",
                         mpctx->osd_function,pts/3600,(pts/60)%60,
                         pts%60,percentage_text);
        } else
            osd_text_timer[0]=0;

        // always decrement the percentage timer
        if(mpctx->osd_show_percentage)
            mpctx->osd_show_percentage--;

        if(strcmp(osd_text,osd_text_timer)) {
            strncpy(osd_text, osd_text_timer, 63);
            vo_osd_changed(OSDTYPE_OSD);
        }
        return;
    }

    // Clear the term osd line
    if(term_osd && osd_text[0]) {
        osd_text[0] = 0;
        printf("%s\n",term_osd_esc);
    }
}

///@}
// OSDMsgStack


void reinit_audio_chain(void) {
    if (!mpctx->sh_audio)
        return;
    if (!(initialized_flags & INITIALIZED_ACODEC)) {
        current_module="init_audio_codec";
        mp_msg(MSGT_CPLAYER,MSGL_INFO,"==========================================================================\n");
        if(!init_best_audio_codec(mpctx->sh_audio,audio_codec_list,audio_fm_list)){
            goto init_error;
        }
        initialized_flags|=INITIALIZED_ACODEC;
        mp_msg(MSGT_CPLAYER,MSGL_INFO,"==========================================================================\n");
    }


    if (!(initialized_flags & INITIALIZED_AO)) {
        current_module="af_preinit";
        ao_data.samplerate=force_srate;
        ao_data.channels=0;
        ao_data.format=audio_output_format;
        // first init to detect best values
        if(!init_audio_filters(mpctx->sh_audio,   // preliminary init
                               // input:
                               mpctx->sh_audio->samplerate,
                               // output:
                               &ao_data.samplerate, &ao_data.channels, &ao_data.format)){
            mp_msg(MSGT_CPLAYER,MSGL_ERR,MSGTR_AudioFilterChainPreinitError);
            exit_player(EXIT_ERROR);
        }
        current_module="ao2_init";
        mpctx->audio_out = init_best_audio_out(audio_driver_list,
                                               0, // plugin flag
                                               ao_data.samplerate,
                                               ao_data.channels,
                                               ao_data.format, 0);
        if(!mpctx->audio_out){
            mp_msg(MSGT_CPLAYER,MSGL_ERR,MSGTR_CannotInitAO);
            goto init_error;
        }
        initialized_flags|=INITIALIZED_AO;
        mp_msg(MSGT_CPLAYER,MSGL_INFO,"AO: [%s] %dHz %dch %s (%d bytes per sample)\n",
               mpctx->audio_out->info->short_name,
               ao_data.samplerate, ao_data.channels,
               af_fmt2str_short(ao_data.format),
               af_fmt2bits(ao_data.format)/8 );
        mp_msg(MSGT_CPLAYER,MSGL_V,"AO: Description: %s\nAO: Author: %s\n",
               mpctx->audio_out->info->name, mpctx->audio_out->info->author);
        if(strlen(mpctx->audio_out->info->comment) > 0)
            mp_msg(MSGT_CPLAYER,MSGL_V,"AO: Comment: %s\n", mpctx->audio_out->info->comment);
    }

    // init audio filters:
    current_module="af_init";
    if(!build_afilter_chain(mpctx->sh_audio, &ao_data)) {
        mp_msg(MSGT_CPLAYER,MSGL_ERR,MSGTR_NoMatchingFilter);
        goto init_error;
    }
    mpctx->mixer.audio_out = mpctx->audio_out;
    mpctx->mixer.volstep = volstep;
    return;

init_error:
    uninit_player(INITIALIZED_ACODEC|INITIALIZED_AO); // close codec and possibly AO
    mpctx->sh_audio=mpctx->d_audio->sh=NULL; // -> nosound
    mpctx->d_audio->id = -2;
}


///@}
// Command2Property


// Return pts value corresponding to the end point of audio written to the
// ao so far.
static double written_audio_pts(sh_audio_t *sh_audio, demux_stream_t *d_audio)
{
    double buffered_output;
    // first calculate the end pts of audio that has been output by decoder
    double a_pts = sh_audio->pts;
    if (a_pts != MP_NOPTS_VALUE)
	// Good, decoder supports new way of calculating audio pts.
	// sh_audio->pts is the timestamp of the latest input packet with
	// known pts that the decoder has decoded. sh_audio->pts_bytes is
	// the amount of bytes the decoder has written after that timestamp.
	a_pts += sh_audio->pts_bytes / (double) sh_audio->o_bps;
    else {
	// Decoder doesn't support new way of calculating pts (or we're
	// being called before it has decoded anything with known timestamp).
	// Use the old method of audio pts calculation: take the timestamp
	// of last packet with known pts the decoder has read data from,
	// and add amount of bytes read after the beginning of that packet
	// divided by input bps. This will be inaccurate if the input/output
	// ratio is not constant for every audio packet or if it is constant
	// but not accurately known in sh_audio->i_bps.

	a_pts = d_audio->pts;
	// ds_tell_pts returns bytes read after last timestamp from
	// demuxing layer, decoder might use sh_audio->a_in_buffer for bytes
	// it has read but not decoded
	if (sh_audio->i_bps)
	    a_pts += (ds_tell_pts(d_audio) - sh_audio->a_in_buffer_len) /
		(double)sh_audio->i_bps;
    }
    // Now a_pts hopefully holds the pts for end of audio from decoder.
    // Substract data in buffers between decoder and audio out.

    // Decoded but not filtered
    a_pts -= sh_audio->a_buffer_len / (double)sh_audio->o_bps;

    // Data buffered in audio filters, measured in bytes of "missing" output
    buffered_output = af_calc_delay(sh_audio->afilter);

    // Data that was ready for ao but was buffered because ao didn't fully
    // accept everything to internal buffers yet
    buffered_output += sh_audio->a_out_buffer_len;

    // Filters divide audio length by playback_speed, so multiply by it
    // to get the length in original units without speedup or slowdown
    a_pts -= buffered_output * playback_speed / ao_data.bps;

    return a_pts;
}

// Return pts value corresponding to currently playing audio.
double playing_audio_pts(sh_audio_t *sh_audio, demux_stream_t *d_audio,
				const ao_functions_t *audio_out)
{
    return written_audio_pts(sh_audio, d_audio) - playback_speed *
	audio_out->get_delay();
}

static int check_framedrop(double frame_time) {
	// check for frame-drop:
	current_module = "check_framedrop";
	if (mpctx->sh_audio && !mpctx->d_audio->eof) {
		static int dropped_frames;
	    float delay = playback_speed*mpctx->audio_out->get_delay();
	    float d = delay-mpctx->delay;
	    ++total_frame_cnt;
	    // we should avoid dropping too many frames in sequence unless we
	    // are too late. and we allow 500ms A-V delay here:
	    if (d < -dropped_frames*frame_time-0.500 &&
				mpctx->osd_function != OSD_PAUSE) {
		++drop_frame_cnt;
		++dropped_frames;
		return frame_dropping;
	    } else
		dropped_frames = 0;
	}
	return 0;
}

static int generate_video_frame(sh_video_t *sh_video, demux_stream_t *d_video)
{
    unsigned char *start;
    int in_size;
    int hit_eof=0;
    double pts;

    while (1) {
	int drop_frame = check_framedrop(sh_video->frametime);
	void *decoded_frame;
	current_module = "decode video";
	// XXX Time used in this call is not counted in any performance
	// timer now, OSD is not updated correctly for filter-added frames
	if (vf_output_queued_frame(sh_video->vfilter))
	    break;
	current_module = "video_read_frame";
	in_size = ds_get_packet_pts(d_video, &start, &pts);
	if (in_size < 0) {
	    // try to extract last frames in case of decoder lag
	    in_size = 0;
	    pts = MP_NOPTS_VALUE;
	    hit_eof = 1;
	}
	if (in_size > max_framesize)
	    max_framesize = in_size;
	current_module = "decode video";
	decoded_frame = decode_video(sh_video, start, in_size, drop_frame, pts);
	if (decoded_frame) {
	    update_subtitles(sh_video, sh_video->pts, mpctx->d_sub, 0);
	    update_teletext(sh_video, mpctx->demuxer, 0);
	    update_osd_msg();
	    current_module = "filter video";
	    if (filter_video(sh_video, decoded_frame, sh_video->pts))
		break;
	} else if (drop_frame)
	    return -1;
	if (hit_eof)
	    return 0;
    }
    return 1;
}

#ifdef HAVE_RTC
    int rtc_fd = -1;
#endif

static float timing_sleep(float time_frame)
{
#ifdef HAVE_RTC
    if (rtc_fd >= 0){
	// -------- RTC -----------
	current_module="sleep_rtc";
        while (time_frame > 0.000) {
	    unsigned long rtc_ts;
	    if (read(rtc_fd, &rtc_ts, sizeof(rtc_ts)) <= 0)
		mp_msg(MSGT_CPLAYER, MSGL_ERR, MSGTR_LinuxRTCReadError, strerror(errno));
    	    time_frame -= GetRelativeTime();
	}
    } else
#endif
    {
	// assume kernel HZ=100 for softsleep, works with larger HZ but with
	// unnecessarily high CPU usage
	float margin = softsleep ? 0.011 : 0;
	current_module = "sleep_timer";
	while (time_frame > margin) {
	    usec_sleep(1000000 * (time_frame - margin));
	    time_frame -= GetRelativeTime();
	}
	if (softsleep){
	    current_module = "sleep_soft";
	    if (time_frame < 0)
		mp_msg(MSGT_AVSYNC, MSGL_WARN, MSGTR_SoftsleepUnderflow);
	    while (time_frame > 0)
		time_frame-=GetRelativeTime(); // burn the CPU
	}
    }
    return time_frame;
}

static int select_subtitle(MPContext *mpctx)
{
  // find the best sub to use
  int id;
  int found = 0;
  mpctx->global_sub_pos = -1; // no subs by default
  if (vobsub_id >= 0) {
    // if user asks for a vobsub id, use that first.
    id = vobsub_id;
    found = mp_property_do("sub_vob", M_PROPERTY_SET, &id, mpctx) == M_PROPERTY_OK;
  }

  if (!found && dvdsub_id >= 0) {
    // if user asks for a dvd sub id, use that next.
    id = dvdsub_id;
    found = mp_property_do("sub_demux", M_PROPERTY_SET, &id, mpctx) == M_PROPERTY_OK;
  }

  if (!found) {
    // if there are text subs to use, use those.  (autosubs come last here)
    id = 0;
    found = mp_property_do("sub_file", M_PROPERTY_SET, &id, mpctx) == M_PROPERTY_OK;
  }

  if (!found && dvdsub_id == -1) {
    // finally select subs by language and container hints
    if (dvdsub_id == -1 && dvdsub_lang)
      dvdsub_id = demuxer_sub_track_by_lang(mpctx->demuxer, dvdsub_lang);
    if (dvdsub_id == -1)
      dvdsub_id = demuxer_default_sub_track(mpctx->demuxer);
    if (dvdsub_id >= 0) {
      id = dvdsub_id;
      found = mp_property_do("sub_demux", M_PROPERTY_SET, &id, mpctx) == M_PROPERTY_OK;
    }
  }
  return found;
}

#ifdef CONFIG_DVDNAV
#ifndef FF_B_TYPE
#define FF_B_TYPE 3
#endif
/// store decoded video image
static mp_image_t * mp_dvdnav_copy_mpi(mp_image_t *to_mpi,
                                       mp_image_t *from_mpi) {
    mp_image_t *mpi;

    /// Do not store B-frames
    if (from_mpi->pict_type == FF_B_TYPE)
        return to_mpi;

    if (to_mpi &&
        to_mpi->w == from_mpi->w &&
        to_mpi->h == from_mpi->h &&
        to_mpi->imgfmt == from_mpi->imgfmt)
        mpi = to_mpi;
    else {
        if (to_mpi)
            free_mp_image(to_mpi);
        if (from_mpi->w == 0 || from_mpi->h == 0)
            return NULL;
        mpi = alloc_mpi(from_mpi->w,from_mpi->h,from_mpi->imgfmt);
    }

    copy_mpi(mpi,from_mpi);
    return mpi;
}

static void mp_dvdnav_reset_stream (MPContext *ctx) {
    if (ctx->sh_video) {
        /// clear video pts
        ctx->d_video->pts = 0.0f;
        ctx->sh_video->pts = 0.0f;
        ctx->sh_video->i_pts = 0.0f;
        ctx->sh_video->last_pts = 0.0f;
        ctx->sh_video->num_buffered_pts = 0;
        ctx->sh_video->num_frames = 0;
        ctx->sh_video->num_frames_decoded = 0;
        ctx->sh_video->timer = 0.0f;
        ctx->sh_video->stream_delay = 0.0f;
        ctx->sh_video->timer = 0;
        ctx->demuxer->stream_pts = MP_NOPTS_VALUE;
    }

    if (ctx->sh_audio) {
        /// free audio packets and reset
        ds_free_packs(ctx->d_audio);
        audio_delay -= ctx->sh_audio->stream_delay;
        ctx->delay =- audio_delay;
        ctx->audio_out->reset();
        resync_audio_stream(ctx->sh_audio);
    }

    audio_delay = 0.0f;
    mpctx->sub_counts[SUB_SOURCE_DEMUX] = mp_dvdnav_number_of_subs(mpctx->stream);
    if (dvdsub_lang && dvdsub_id == dvdsub_lang_id) {
        dvdsub_lang_id = mp_dvdnav_sid_from_lang(ctx->stream, dvdsub_lang);
        if (dvdsub_lang_id != dvdsub_id) {
            dvdsub_id = dvdsub_lang_id;
            select_subtitle(ctx);
        }
    }

    /// clear all EOF related flags
    ctx->d_video->eof = ctx->d_audio->eof = ctx->stream->eof = 0;
}

/// Restore last decoded DVDNAV (still frame)
static mp_image_t *mp_dvdnav_restore_smpi(int *in_size,
                                          unsigned char **start,
                                          mp_image_t *decoded_frame)
{
    if (mpctx->stream->type != STREAMTYPE_DVDNAV)
        return decoded_frame;

    /// a change occured in dvdnav stream
    if (mp_dvdnav_cell_has_changed(mpctx->stream,0)) {
        mp_dvdnav_read_wait(mpctx->stream, 1, 1);
        mp_dvdnav_context_free(mpctx);
        mp_dvdnav_reset_stream(mpctx);
        mp_dvdnav_read_wait(mpctx->stream, 0, 1);
        mp_dvdnav_cell_has_changed(mpctx->stream,1);
    }

    if (*in_size < 0) {
        float len;

        /// Display still frame, if any
        if (mpctx->nav_smpi && !mpctx->nav_buffer)
            decoded_frame = mpctx->nav_smpi;

        /// increment video frame : continue playing after still frame
        len = demuxer_get_time_length(mpctx->demuxer);
        if (mpctx->sh_video->pts >= len &&
            mpctx->sh_video->pts > 0.0 && len > 0.0) {
            mp_dvdnav_skip_still(mpctx->stream);
            mp_dvdnav_skip_wait(mpctx->stream);
        }
        mpctx->sh_video->pts += 1 / mpctx->sh_video->fps;

        if (mpctx->nav_buffer) {
            *start = mpctx->nav_start;
            *in_size = mpctx->nav_in_size;
            if (mpctx->nav_start)
                memcpy(*start,mpctx->nav_buffer,mpctx->nav_in_size);
        }
    }

    return decoded_frame;
}

/// Save last decoded DVDNAV (still frame)
static void mp_dvdnav_save_smpi(int in_size,
                                unsigned char *start,
                                mp_image_t *decoded_frame)
{
    if (mpctx->stream->type != STREAMTYPE_DVDNAV)
        return;

    if (mpctx->nav_buffer)
        free(mpctx->nav_buffer);

    mpctx->nav_buffer = malloc(in_size);
    mpctx->nav_start = start;
    mpctx->nav_in_size = mpctx->nav_buffer ? in_size : -1;
    if (mpctx->nav_buffer)
        memcpy(mpctx->nav_buffer,start,in_size);

    if (decoded_frame && mpctx->nav_smpi != decoded_frame)
        mpctx->nav_smpi = mp_dvdnav_copy_mpi(mpctx->nav_smpi,decoded_frame);
}
#endif /* CONFIG_DVDNAV */

static void adjust_sync_and_print_status(int between_frames, float timing_error)
{
    current_module="av_sync";

    if(mpctx->sh_audio){
	double a_pts, v_pts;

	if (autosync)
	    /*
	     * If autosync is enabled, the value for delay must be calculated
	     * a bit differently.  It is set only to the difference between
	     * the audio and video timers.  Any attempt to include the real
	     * or corrected delay causes the pts_correction code below to
	     * try to correct for the changes in delay which autosync is
	     * trying to measure.  This keeps the two from competing, but still
	     * allows the code to correct for PTS drift *only*.  (Using a delay
	     * value here, even a "corrected" one, would be incompatible with
	     * autosync mode.)
	     */
	    a_pts = written_audio_pts(mpctx->sh_audio, mpctx->d_audio) - mpctx->delay;
	else
	    a_pts = playing_audio_pts(mpctx->sh_audio, mpctx->d_audio, mpctx->audio_out);

	v_pts = mpctx->sh_video->pts;

	{
	    static int drop_message=0;
	    double AV_delay = a_pts - audio_delay - v_pts;
	    double x;
	    if (AV_delay>0.5 && drop_frame_cnt>50 && drop_message==0){
		++drop_message;
		mp_msg(MSGT_AVSYNC,MSGL_WARN,MSGTR_SystemTooSlow);
	    }
	    if (autosync)
		x = AV_delay*0.1f;
	    else
		/* Do not correct target time for the next frame if this frame
		 * was late not because of wrong target time but because the
		 * target time could not be met */
		x = (AV_delay + timing_error * playback_speed) * 0.1f;
	    if (x < -max_pts_correction)
		x = -max_pts_correction;
	    else if (x> max_pts_correction)
		x = max_pts_correction;
	    if (default_max_pts_correction >= 0)
		max_pts_correction = default_max_pts_correction;
	    else
		max_pts_correction = mpctx->sh_video->frametime*0.10; // +-10% of time
	    if (!between_frames) {
		mpctx->delay+=x;
		c_total+=x;
	    }
#ifndef __MINIOS__
	    if(!quiet)
		print_status(a_pts - audio_delay, AV_delay, c_total);
#endif
	}

    }
 else {
	// No audio:

#ifndef __MINIOS__
	if (!quiet)
	    print_status(0, 0, 0);
#endif
    }
}

static int fill_audio_out_buffers(void)
{
    unsigned int t;
    double tt;
    int playsize;
    int playflags=0;
    int audio_eof=0;
    int bytes_to_write;
    int format_change = 0;
    sh_audio_t * const sh_audio = mpctx->sh_audio;

    if (!ipu_image_completed && mpctx->sh_video)
	return 1;

    current_module="play_audio";

    while (1) {
	int sleep_time;
	// all the current uses of ao_data.pts seem to be in aos that handle
	// sync completely wrong; there should be no need to use ao_data.pts
	// in get_space()
	ao_data.pts = ((mpctx->sh_video?mpctx->sh_video->timer:0)+mpctx->delay)*90000.0;
	bytes_to_write = mpctx->audio_out->get_space();
	if (mpctx->sh_video || bytes_to_write >= ao_data.outburst)
	    break;

	// handle audio-only case:
	// this is where mplayer sleeps during audio-only playback
	// to avoid 100% CPU use
	sleep_time = (ao_data.outburst - bytes_to_write) * 1000 / ao_data.bps;
	if (sleep_time < 10) sleep_time = 10; // limit to 100 wakeups per second
	usec_sleep(sleep_time * 1000);
    }

    while (bytes_to_write) {
	int res;
	playsize = bytes_to_write;
	if (playsize > MAX_OUTBURST)
	    playsize = MAX_OUTBURST;
	bytes_to_write -= playsize;

	// Fill buffer if needed:
	current_module="decode_audio";
	t = GetTimer();
	if (!format_change) {
	    res = mp_decode_audio(sh_audio, playsize);
	    format_change = res == -2;
	}
	if (!format_change && res < 0) // EOF or error
	    if (mpctx->d_audio->eof) {
		mpctx->audio_out->play(sh_audio->a_out_buffer, 0, playflags);
		mpctx->eof = 1;
		audio_eof = 1;
		if (sh_audio->a_out_buffer_len == 0)
		    return 0;
	    }
	t = GetTimer() - t;
	tt = t*0.000001f; audio_time_usage+=tt;
	if (playsize > sh_audio->a_out_buffer_len) {
	    playsize = sh_audio->a_out_buffer_len;
	    if (audio_eof || format_change)
		playflags |= AOPLAY_FINAL_CHUNK;
	}
	if (!playsize)
	    break;

	// play audio:
	current_module="play_audio";

	// Is this pts value actually useful for the aos that access it?
	// They're obviously badly broken in the way they handle av sync;
	// would not having access to this make them more broken?
	ao_data.pts = ((mpctx->sh_video?mpctx->sh_video->timer:0)+mpctx->delay)*90000.0;
	playsize = mpctx->audio_out->play(sh_audio->a_out_buffer, playsize, playflags);

	if (playsize > 0) {
	    sh_audio->a_out_buffer_len -= playsize;
	    memmove(sh_audio->a_out_buffer, &sh_audio->a_out_buffer[playsize],
		    sh_audio->a_out_buffer_len);
	    mpctx->delay += playback_speed*playsize/(double)ao_data.bps;
	}
	else if ((format_change || audio_eof) && mpctx->audio_out->get_delay() < .04) {
	    // Sanity check to avoid hanging in case current ao doesn't output
	    // partial chunks and doesn't check for AOPLAY_FINAL_CHUNK
	    mp_msg(MSGT_CPLAYER, MSGL_WARN, "Audio output truncated at end.\n");
	    sh_audio->a_out_buffer_len = 0;
	}
    }
    if (format_change) {
	uninit_player(INITIALIZED_AO);
	reinit_audio_chain();
    }
    return 1;
}

static int sleep_until_update(float *time_frame, float *aq_sleep_time)
{
    int frame_time_remaining = 0;
    current_module="calc_sleep_time";
#ifdef CONFIG_NETWORKING
    if (udp_slave) {
        int udp_master_exited = udp_slave_sync(mpctx);
        if (udp_master_exited) {
            mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_MasterQuit);
            exit_player(EXIT_QUIT);
        }
        return 0;
    }
#endif /* CONFIG_NETWORKING */

    *time_frame -= GetRelativeTime(); // reset timer

    if (mpctx->sh_audio && !mpctx->d_audio->eof) {
	float delay = mpctx->audio_out->get_delay();
	mp_dbg(MSGT_AVSYNC, MSGL_DBG2, "delay=%f\n", delay);
	
	if (autosync) {
	    /*
	     * Adjust this raw delay value by calculating the expected
	     * delay for this frame and generating a new value which is
	     * weighted between the two.  The higher autosync is, the
	     * closer to the delay value gets to that which "-nosound"
	     * would have used, and the longer it will take for A/V
	     * sync to settle at the right value (but it eventually will.)
	     * This settling time is very short for values below 100.
	     */
	    float predicted = mpctx->delay / playback_speed + *time_frame;
	    float difference = delay - predicted;
	    delay = predicted + difference / (float)autosync;
	}
	
	*time_frame = delay - mpctx->delay / playback_speed;

	// delay = amount of audio buffered in soundcard/driver
	if (delay > 0.25) delay=0.25; else
	if (delay < 0.10) delay=0.10;
	if (*time_frame > delay*0.6) {
	    // sleep time too big - may cause audio drops (buffer underrun)
	    frame_time_remaining = 1;
	    *time_frame = delay*0.5;
	}
    } else {
	// If we're lagging more than 200 ms behind the right playback rate,
	// don't try to "catch up".
	// If benchmark is set always output frames as fast as possible
	// without sleeping.
	if (*time_frame < -0.2 || benchmark)
	    *time_frame = 0;
    }

    *aq_sleep_time += *time_frame;
    //============================== SLEEP: ===================================
    // flag 256 means: libvo driver does its timing (dvb card)
    if (*time_frame > 0.001 && !(vo_flags&256))
	*time_frame = timing_sleep(*time_frame);

#ifdef CONFIG_NETWORKING
    if (udp_master) {
      char current_time[256];
      sprintf(current_time, "%f", mpctx->sh_video->pts);
      send_udp(udp_ip, udp_port, current_time);
    }
#endif /* CONFIG_NETWORKING */
    return frame_time_remaining;
}

int reinit_video_chain(void) {
    sh_video_t * const sh_video = mpctx->sh_video;
    double ar=-1.0;
    //================== Init VIDEO (codec & libvo) ==========================
    if(!fixed_vo || !(initialized_flags&INITIALIZED_VO)){
    current_module="preinit_libvo";

    //shouldn't we set dvideo->id=-2 when we fail?
    vo_config_count=0;
    //if((mpctx->video_out->preinit(vo_subdevice))!=0){
    if(!(mpctx->video_out=init_best_video_out(video_driver_list))){
      mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_ErrorInitializingVODevice);
      goto err_out;
    }
    initialized_flags|=INITIALIZED_VO;
  }

  if(stream_control(mpctx->demuxer->stream, STREAM_CTRL_GET_ASPECT_RATIO, &ar) != STREAM_UNSUPPORTED)
      mpctx->sh_video->stream_aspect = ar;
  current_module="init_video_filters";
  {
    char* vf_arg[] = { "_oldargs_", (char*)mpctx->video_out , NULL };
    sh_video->vfilter=vf_open_filter(NULL,"vo",vf_arg);
  }
#ifdef CONFIG_MENU
  if(use_menu) {
    char* vf_arg[] = { "_oldargs_", menu_root, NULL };
    vf_menu = vf_open_plugin(libmenu_vfs,sh_video->vfilter,"menu",vf_arg);
    if(!vf_menu) {
      mp_msg(MSGT_CPLAYER,MSGL_ERR,MSGTR_CantOpenLibmenuFilterWithThisRootMenu,menu_root);
      use_menu = 0;
    }
  }
  if(vf_menu)
    sh_video->vfilter=vf_menu;
#endif

#ifdef CONFIG_ASS
  if(ass_enabled) {
    int i;
    int insert = 1;
    if (vf_settings)
      for (i = 0; vf_settings[i].name; ++i)
        if (strcmp(vf_settings[i].name, "ass") == 0) {
          insert = 0;
          break;
        }
    if (insert) {
      char* vf_arg[] = {"auto", "1", NULL};
      vf_instance_t* vf_ass = vf_open_filter(sh_video->vfilter,"ass",vf_arg);
      if (vf_ass)
        sh_video->vfilter=vf_ass;
      else
        mp_msg(MSGT_CPLAYER,MSGL_ERR, "ASS: cannot add video filter\n");
    }
  }
#endif

  sh_video->vfilter=append_filters(sh_video->vfilter);
  eosd_init(sh_video->vfilter);

#ifdef CONFIG_ASS
  if (ass_enabled)
    eosd_ass_init(ass_library);
#endif

  current_module="init_video_codec";

  mp_msg(MSGT_CPLAYER,MSGL_INFO,"==========================================================================\n");
  init_best_video_codec(sh_video,video_codec_list,video_fm_list);
  mp_msg(MSGT_CPLAYER,MSGL_INFO,"==========================================================================\n");

  if(!sh_video->initialized){
    if(!fixed_vo) uninit_player(INITIALIZED_VO);
    goto err_out;
  }

  initialized_flags|=INITIALIZED_VCODEC;

  if (sh_video->codec)
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_VIDEO_CODEC=%s\n", sh_video->codec->name);

  sh_video->last_pts = MP_NOPTS_VALUE;
  sh_video->num_buffered_pts = 0;
  sh_video->next_frame_time = 0;

  if(auto_quality>0){
    // Auto quality option enabled
    output_quality=get_video_quality_max(sh_video);
    if(auto_quality>output_quality) auto_quality=output_quality;
    else output_quality=auto_quality;
    mp_msg(MSGT_CPLAYER,MSGL_V,"AutoQ: setting quality to %d.\n",output_quality);
    set_video_quality(sh_video,output_quality);
  }

  // ========== Init display (sh_video->disp_w*sh_video->disp_h/out_fmt) ============

  current_module="init_vo";

  return 1;

err_out:
  mpctx->sh_video = mpctx->d_video->sh = NULL;
  return 0;
}

#ifdef USE_IPU_THROUGH_MODE
unsigned int disp_buf0 = 0, disp_buf1 = 0, disp_buf2 = 0;
void clear_dispbuf()
{
    disp_buf0 = 0;
    disp_buf1 = 0;    
    disp_buf2 = 0;    
}
unsigned int get_disp_buf0()
{
    return disp_buf0;
    
}
unsigned int get_disp_buf1()
{
    return disp_buf1;
    
}
unsigned int get_disp_buf2()
{
    return disp_buf2;
    
}
#endif

static double update_video(int *blit_frame)
{
    sh_video_t * const sh_video = mpctx->sh_video;
    //--------------------  Decode a frame: -----------------------
    double frame_time;
    *blit_frame = 0; // Don't blit if we hit EOF
    if (!correct_pts) {
	unsigned char* start=NULL;
	void *decoded_frame = NULL;
	int drop_frame=0;
	int in_size;

    do {
	current_module = "video_read_frame";
	frame_time = sh_video->next_frame_time;
	in_size = video_read_frame(sh_video, &sh_video->next_frame_time,
				   &start, force_fps);
#ifdef CONFIG_DVDNAV
	/// wait, still frame or EOF
	if (mpctx->stream->type == STREAMTYPE_DVDNAV && in_size < 0) {
	    if (mp_dvdnav_is_eof(mpctx->stream))
                return -1;
	    if (mpctx->d_video)
                mpctx->d_video->eof = 0;
	    if (mpctx->d_audio)
                mpctx->d_audio->eof = 0;
	    mpctx->stream->eof = 0;
	} else
#endif
	if (in_size < 0)
	    return -1;
	if (in_size > max_framesize)
	    max_framesize = in_size; // stats
	drop_frame = check_framedrop(frame_time);
	sh_video->timer += frame_time;
	if (mpctx->sh_audio)
	    mpctx->delay -= frame_time;
	current_module = "decode_video";
#ifdef CONFIG_DVDNAV
	decoded_frame = mp_dvdnav_restore_smpi(&in_size,&start,decoded_frame);
	/// still frame has been reached, no need to decode
	if (in_size > 0 && !decoded_frame)
#endif
	decoded_frame = decode_video(sh_video, start, in_size, drop_frame, sh_video->pts);

	// Time-based PTS recalculation.
	// The key to maintaining A-V sync is to not touch PTS until the proper frame is reached
	if (sh_video->pts != MP_NOPTS_VALUE) {
	    if (sh_video->last_pts != MP_NOPTS_VALUE) {
		double pts = sh_video->last_pts + frame_time;
		double ptsdiff = fabs(pts - sh_video->pts);

		// Allow starting PTS recalculation at the appropriate frame only
		mpctx->framestep_found |= (ptsdiff <= frame_time * 1.5);

		// replace PTS only if we're not too close and not too far
		// and a correctly timed frame has been found, otherwise
		// keep pts to eliminate rounding errors or catch up with stream
		if (ptsdiff > frame_time * 20)
		    mpctx->framestep_found = 0;
		if (ptsdiff * 10 > frame_time && mpctx->framestep_found)
		    sh_video->pts = pts;
		else
		    mp_dbg(MSGT_AVSYNC,MSGL_DBG2,"Keeping PTS at %6.2f\n", sh_video->pts);
	    }
	    sh_video->last_pts = sh_video->pts;
	}
	// video_read_frame can change fps (e.g. for ASF video)
	vo_fps = sh_video->fps;
	update_subtitles(sh_video, sh_video->pts, mpctx->d_sub, 0);
	update_teletext(sh_video, mpctx->demuxer, 0);
	update_osd_msg();
#ifdef CONFIG_DVDNAV
	/// save back last still frame for future display
	mp_dvdnav_save_smpi(in_size,start,decoded_frame);
#endif
    } while (!decoded_frame);

	current_module = "filter_video";
#ifdef USE_IPU_THROUGH_MODE
	if(decoded_frame){				
	  disp_buf0 = disp_buf1;
	  disp_buf1 = disp_buf2;
	  disp_buf2 = (unsigned int)((mp_image_t *)decoded_frame)->planes[0];
	  *blit_frame = filter_video(sh_video, decoded_frame,
				     sh_video->pts);
	}
#else
	*blit_frame = (decoded_frame && filter_video(sh_video, decoded_frame,
						    sh_video->pts));
#endif
    }
    else {
	int res = generate_video_frame(sh_video, mpctx->d_video);
	if (!res)
	    return -1;
	((vf_instance_t *)sh_video->vfilter)->control(sh_video->vfilter,
					    VFCTRL_GET_PTS, &sh_video->pts);
	if (sh_video->pts == MP_NOPTS_VALUE) {
	    mp_msg(MSGT_CPLAYER, MSGL_ERR, "pts after filters MISSING\n");
	    sh_video->pts = sh_video->last_pts;
	}
	if (sh_video->last_pts == MP_NOPTS_VALUE)
	    sh_video->last_pts= sh_video->pts;
	else if (sh_video->last_pts > sh_video->pts) {
	    // make a guess whether this is some kind of discontinuity
	    // we should jump along with or some wron timestamps we
	    // should replace instead
            if (sh_video->pts < sh_video->last_pts - 20 * sh_video->frametime)
		sh_video->last_pts = sh_video->pts;
	    else
	        sh_video->pts = sh_video->last_pts + sh_video->frametime;
	    mp_msg(MSGT_CPLAYER, MSGL_V, "pts value < previous\n");
	}
	frame_time = sh_video->pts - sh_video->last_pts;
	sh_video->last_pts = sh_video->pts;
	sh_video->timer += frame_time;
	if(mpctx->sh_audio)
	    mpctx->delay -= frame_time;
	*blit_frame = res > 0;
    }
    return frame_time;
}

static void pause_loop(void)
{
#ifndef NOAH_OS
    mp_cmd_t* cmd;
    if (!quiet) {
        // Small hack to display the pause message on the OSD line.
        // The pause string is: "\n == PAUSE == \r" so we need to
        // take the first and the last char out
        if (term_osd && !mpctx->sh_video) {
            char msg[128] = MSGTR_Paused;
            int mlen = strlen(msg);
            msg[mlen-1] = '\0';
            set_osd_msg(OSD_MSG_PAUSE, 1, 0, "%s", msg+1);
            update_osd_msg();
        } else
            mp_msg(MSGT_CPLAYER,MSGL_STATUS,MSGTR_Paused);
        mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_PAUSED\n");
    }
#ifdef CONFIG_GUI
    if (use_gui)
        guiGetEvent(guiCEvent, (char *)guiSetPause);
#endif
    if (mpctx->video_out && mpctx->sh_video && vo_config_count)
        mpctx->video_out->control(VOCTRL_PAUSE, NULL);

    if (mpctx->audio_out && mpctx->sh_audio)
        mpctx->audio_out->pause(); // pause audio, keep data if possible

    while ( (cmd = mp_input_get_cmd(20, 1, 1)) == NULL || cmd->pausing == 4) {
        if (cmd) {
          cmd = mp_input_get_cmd(0,1,0);
          run_command(mpctx, cmd);
          mp_cmd_free(cmd);
          continue;
        }
        if (mpctx->sh_video && mpctx->video_out && vo_config_count)
            mpctx->video_out->check_events();
#ifdef CONFIG_GUI
        if (use_gui) {
            guiEventHandling();
            guiGetEvent(guiReDraw, NULL);
            if (guiIntfStruct.Playing!=2 || (rel_seek_secs || abs_seek_pos))
                break;
        }
#endif
#ifdef CONFIG_MENU
        if (vf_menu)
            vf_menu_pause_update(vf_menu);
#endif
#ifdef __MINIOS__
        if (mpctx->osd_function != OSD_PAUSE)
        	break;
#endif
        usec_sleep(20000);
    }
    if (cmd && cmd->id == MP_CMD_PAUSE) {
        cmd = mp_input_get_cmd(0,1,0);
        mp_cmd_free(cmd);
    }
    mpctx->osd_function=OSD_PLAY;
    if (mpctx->audio_out && mpctx->sh_audio) {
        if (mpctx->eof) // do not play remaining audio if we e.g.  switch to the next file
          mpctx->audio_out->reset();
        else
          mpctx->audio_out->resume(); // resume audio
    }
    if (mpctx->video_out && mpctx->sh_video && vo_config_count)
        mpctx->video_out->control(VOCTRL_RESUME, NULL); // resume video
    (void)GetRelativeTime(); // ignore time that passed during pause
#ifdef CONFIG_GUI
    if (use_gui) {
        if (guiIntfStruct.Playing == guiSetStop)
            mpctx->eof = 1;
        else
            guiGetEvent(guiCEvent, (char *)guiSetPlay);
    }
#endif
#endif
}

static void edl_loadfile(void)
{
    if (edl_filename) {
        if (edl_records) {
            free_edl(edl_records);
            edl_needs_reset = 1;
        }
        next_edl_record = edl_records = edl_parse_file();
    }
}

// Execute EDL command for the current position if one exists
static void edl_update(MPContext *mpctx)
{
    if (!edl_records) {
        return;
    }

    if (!mpctx->sh_video) {
	mp_msg(MSGT_CPLAYER, MSGL_ERR, MSGTR_EdlNOsh_video);
	free_edl(edl_records);
	next_edl_record = NULL;
	edl_records = NULL;
	return;
    }

    // This indicates that we need to reset next EDL record according
    // to new PTS due to seek or other condition
    if (edl_needs_reset) {
        edl_needs_reset = 0;
        mpctx->edl_muted = 0;
        next_edl_record = edl_records;

        // Find next record, also skip immediately if we are already
        // inside any record
        while (next_edl_record) {
            if (next_edl_record->start_sec > mpctx->sh_video->pts)
                break;
            if (next_edl_record->stop_sec >= mpctx->sh_video->pts) {
                if (edl_backward) {
                    mpctx->osd_function = OSD_REW;
                    edl_decision = 1;
                    abs_seek_pos = 0;
                    rel_seek_secs = -(mpctx->sh_video->pts -
                                      next_edl_record->start_sec +
                                      edl_backward_delay);
                    mp_msg(MSGT_CPLAYER, MSGL_DBG4, "EDL_SKIP: pts [%f], "
                           "offset [%f], start [%f], stop [%f], length [%f]\n",
                           mpctx->sh_video->pts, rel_seek_secs,
                           next_edl_record->start_sec, next_edl_record->stop_sec,
                           next_edl_record->length_sec);
                    return;
                }
                break;
            }

            if (next_edl_record->action == EDL_MUTE)
                mpctx->edl_muted = !mpctx->edl_muted;

            next_edl_record = next_edl_record->next;
	}
        if ((mpctx->user_muted | mpctx->edl_muted) != mpctx->mixer.muted)
            mixer_mute(&mpctx->mixer);
    }

    if (next_edl_record &&
        mpctx->sh_video->pts >= next_edl_record->start_sec) {
        if (next_edl_record->action == EDL_SKIP) {
            mpctx->osd_function = OSD_FFW;
            edl_decision = 1;
            abs_seek_pos = 0;
            rel_seek_secs = next_edl_record->stop_sec - mpctx->sh_video->pts;
            mp_msg(MSGT_CPLAYER, MSGL_DBG4, "EDL_SKIP: pts [%f], offset [%f], "
                   "start [%f], stop [%f], length [%f]\n",
                   mpctx->sh_video->pts, rel_seek_secs,
                   next_edl_record->start_sec, next_edl_record->stop_sec,
                   next_edl_record->length_sec);
        }
	else if (next_edl_record->action == EDL_MUTE) {
            mpctx->edl_muted = !mpctx->edl_muted;
            if ((mpctx->user_muted | mpctx->edl_muted) != mpctx->mixer.muted)
                mixer_mute(&mpctx->mixer);
            mp_msg(MSGT_CPLAYER, MSGL_DBG4, "EDL_MUTE: [%f]\n",
                   next_edl_record->start_sec );
        }
        next_edl_record = next_edl_record->next;
    }
}

// style & SEEK_ABSOLUTE == 0 means seek relative to current position, == 1 means absolute
// style & SEEK_FACTOR == 0 means amount in seconds, == 2 means fraction of file length
// return -1 if seek failed (non-seekable stream?), 0 otherwise
static int seek(MPContext *mpctx, double amount, int style)
{
    current_module = "seek";
    if (demux_seek(mpctx->demuxer, amount, audio_delay, style) == 0)
	return -1;

    mpctx->startup_decode_retry = DEFAULT_STARTUP_DECODE_RETRY;
    if (mpctx->sh_video) {
	current_module = "seek_video_reset";
	if (vo_config_count)
	    mpctx->video_out->control(VOCTRL_RESET, NULL);
	mpctx->num_buffered_frames = 0;
	mpctx->delay = 0;
	mpctx->time_frame = 0;
	mpctx->framestep_found = 0;
	// Not all demuxers set d_video->pts during seek, so this value
	// (which is used by at least vobsub and edl code below) may
	// be completely wrong (probably 0).
	mpctx->sh_video->pts = mpctx->d_video->pts;
	update_subtitles(mpctx->sh_video, mpctx->sh_video->pts, mpctx->d_sub, 1);
	update_teletext(mpctx->sh_video, mpctx->demuxer, 1);
    }

    if (mpctx->sh_audio) {
	current_module = "seek_audio_reset";
	mpctx->audio_out->reset(); // stop audio, throwing away buffered data
	if (!mpctx->sh_video)
	    update_subtitles(NULL, mpctx->sh_audio->pts, mpctx->d_sub, 1);
    }

    if (vo_vobsub && mpctx->sh_video) {
	current_module = "seek_vobsub_reset";
	vobsub_seek(vo_vobsub, mpctx->sh_video->pts);
    }

#ifdef CONFIG_ASS
    if (ass_enabled && mpctx->d_sub->sh && ((sh_sub_t *)mpctx->d_sub->sh)->ass_track)
        ass_flush_events(((sh_sub_t *)mpctx->d_sub->sh)->ass_track);
#endif

    if (edl_records) {
        edl_needs_reset = 1;
        edl_backward = amount < 0;
    }

    c_total = 0;
    max_pts_correction = 0.1;
    audio_time_usage = 0; video_time_usage = 0; vout_time_usage = 0;
    drop_frame_cnt = 0;

    current_module = NULL;
    return 0;
}

#define WDT_CLK_DIV1    0
#define WDT_CLK_DIV4    1
#define WDT_CLK_DIV16   2
#define WDT_CLK_DIV64   3
#define WDT_CLK_DIV256  4
#define WDT_CLK_DIV1024 5

#define WDT_CLK_DIV_SHF 3

#define WDT_EXT_INPUT   (1 << 2)
#define WDT_RTC_INPUT   (1 << 1)
#define WDT_PCK_INPUT   (1 << 0)

#define WDT_TCSR_ADDR16   (0xb000200c)
#define WDT_TER_ADDR8     (0xb0002004)
#define WDT_TDR_ADDR16    (0xb0002000)
#define WDT_TCNT_ADDR16   (0xb0002008) 

void watchdog_init (int msec)
{
	*(volatile unsigned char *) WDT_TER_ADDR8 = 0;
	*(volatile unsigned short *)WDT_TCSR_ADDR16 = WDT_RTC_INPUT | (WDT_CLK_DIV64 << WDT_CLK_DIV_SHF);
	*(volatile unsigned short *)WDT_TDR_ADDR16 =  msec * 32768 / 1000 / 64;
	*(volatile unsigned short *)WDT_TCNT_ADDR16 = 0;
}

void watchdog_enable ()
{
	*(volatile unsigned char *) WDT_TER_ADDR8 = 1;
}

void watchdog_disable ()
{
	*(volatile unsigned char *) WDT_TER_ADDR8 = 0;
}

void watchdog_kick ()
{
	*(volatile unsigned short *)WDT_TCNT_ADDR16 = 0;
}

////////////////////////////////////////////////////
// 功能: 初始化MPLAYER库
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int InitMplayerLib()
{
parameter_num = 2;
parameter_buf[0] = "";
parameter_buf[1] = noah_get_file_name();
kprintf("mplayer file = %s\n",parameter_buf[1]);
PlayerStatus = MEDIALIB_INIT;

char * mem_ptr;
/* Flag indicating whether MPlayer should exit without playing anything. */
int opt_exit = 0;

//float a_frame=0;    // Audio

int i;

int gui_no_filename=0;

#ifdef JZ47_OPT
 mxucr = enable_jz4740_mxu ();
#endif
	
#ifdef JZC_HW_MEDIA
 VAE_map();
#endif

mp_memory_init();

  {
	  unsigned int eaddr, tmp;
	  eaddr = i_mfc0_2(30, 0);  //Error PC
    kprintf ("\n\n+++++++ Error PC = 0x%08x +++++++\n\n", eaddr);
    tmp = i_mfc0_2(12, 0);    // Status
    kprintf ("\n\n+++++++ Status = 0x%08x +++++++\n\n", tmp);
    tmp = tmp | (1 << 29);  //enable float
    i_mtc0_2 (tmp, 12, 0);
    tmp = i_mfc0_2(12, 0);    // Status
    kprintf ("\n\n+++++++ Status = 0x%08x +++++++\n\n", tmp);    
//    watchdog_init (10000);
//    watchdog_enable ();
//    kprintf ("\n\n watch dog init and enable complete \n\n");
  }

#ifdef JZC_CRC_VER
 crc_fp = fopen("jz4760e_crc.log", "aw");
 if(crc_fp == NULL)
   mp_msg(NULL,NULL,"Error: Open crc log file Failed!\n");
#endif

#ifdef JZC_PMON_P0ed
#if defined(STA_INSN)
# define PMON_P0_FILE_NAME "jz4760_pmon_p0.insn"
#elif defined(STA_UINSN)
# define PMON_P0_FILE_NAME "jz4760_pmon_p0.insn"
#elif defined(STA_CCLK)
# define PMON_P0_FILE_NAME "jz4760_pmon_p0.cclk"
#elif defined(STA_DCC)
# define PMON_P0_FILE_NAME "jz4760_pmon_p0.cc"
#elif defined(STA_ICC)
# define PMON_P0_FILE_NAME "jz4760_pmon_p0.cc"
#elif defined(STA_TLB)
# define PMON_P0_FILE_NAME "jz4760_pmon_p0.tlb"
#else
# error "If JZC_PMON_P0 defined, one of STA_INSN/STA_CCLK/STA_DCC/STA_ICC must be defined!"
#endif
 pmon_p0_fp = fopen(PMON_P0_FILE_NAME, "aw");
 if(pmon_p0_fp == NULL)
   mp_msg(NULL,NULL,"jz4760_pmon_p0 open failed!\n");
#endif // JZC_PMON_P0
  InitTimer();
  srand(GetTimerMS()); 
  mp_msg_init();
#ifdef JZ47_OPT
  ipu_image_start();  
#endif
  if(!codecs_file || !parse_codec_cfg(codecs_file)){
	if(!parse_codec_cfg(mem_ptr=get_path("codecs.conf"))){
	  if(!parse_codec_cfg(MPLAYER_CONFDIR "/codecs.conf")){
		if(!parse_codec_cfg(NULL)){
		  exit_player_with_rc(EXIT_NONE, 0);
		}
		mp_msg(MSGT_CPLAYER,MSGL_V,MSGTR_BuiltinCodecsConf);
	  }
	}
	free( mem_ptr ); // release the buffer created by get_path()
  }

  return OPEN_SUCCESS_ERR;
}


////////////////////////////////////////////////////
// 功能: 打开MPLAYER库
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int IsOpenForInfo(void)
{
	return bOpenForInfo;
}


////////////////////////////////////////////////////
// 功能: 打开MPLAYER库
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int OpenMplayerLib(int info)
{
  mpctx->eof = 0;
  bOpenForInfo = info;
  // init global sub numbers
  mpctx->global_sub_size = 0;
  memset(mpctx->sub_counts, 0, sizeof(mpctx->sub_counts));
  filename = parameter_buf[1];
//---------------------------------------------------------------------------
    if (mpctx->video_out && vo_config_count)
        mpctx->video_out->control(VOCTRL_RESUME, NULL);
//============ Open & Sync STREAM --- fork cache2 ====================

  mpctx->stream=NULL;
  mpctx->demuxer=NULL;
  if (mpctx->d_audio) {
    //free_demuxer_stream(mpctx->d_audio);
    mpctx->d_audio=NULL;
  }
  if (mpctx->d_video) {
    //free_demuxer_stream(d_video);
    mpctx->d_video=NULL;
  }
  mpctx->sh_audio=NULL;
  mpctx->sh_video=NULL;

  current_module="open_stream";
  mpctx->stream=open_stream(filename,0,&mpctx->file_format);
  if(!mpctx->stream) { // error...
    mpctx->eof = libmpdemux_was_interrupted(PT_NEXT_ENTRY);
    return STREAM_OPEN_ERR;
  }
  initialized_flags|=INITIALIZED_STREAM;

  mpctx->stream->start_pos+=seek_to_byte;

if(stream_dump_type==5){
  unsigned char buf[4096];
  int len;
  FILE *f;
  current_module="dumpstream";
  stream_reset(mpctx->stream);
  stream_seek(mpctx->stream,mpctx->stream->start_pos);
  f=fopen(stream_dump_name,"wb");
  if(!f){
    mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_CantOpenDumpfile);
    exit_player(EXIT_ERROR);
  }
  if (dvd_chapter > 1) {
    int chapter = dvd_chapter - 1;
    stream_control(mpctx->stream, STREAM_CTRL_SEEK_TO_CHAPTER, &chapter);
  }
  while(!mpctx->stream->eof && !async_quit_request){
      len=stream_read(mpctx->stream,buf,4096);
      if(len>0) {
        if(fwrite(buf,len,1,f) != 1) {
          mp_msg(MSGT_MENCODER,MSGL_FATAL,MSGTR_ErrorWritingFile,stream_dump_name);
          exit_player(EXIT_ERROR);
        }
      }
      if(dvd_last_chapter > 0) {
        int chapter = -1;
        if (stream_control(mpctx->stream, STREAM_CTRL_GET_CURRENT_CHAPTER,
                           &chapter) == STREAM_OK && chapter + 1 > dvd_last_chapter)
          break;
      }
  }
  if(fclose(f)) {
    mp_msg(MSGT_MENCODER,MSGL_FATAL,MSGTR_ErrorWritingFile,stream_dump_name);
    exit_player(EXIT_ERROR);
  }
  mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_CoreDumped);
  exit_player_with_rc(EXIT_EOF, 0);
}

//============ Open DEMUXERS --- DETECT file type =======================
current_module="demux_open";

mpctx->demuxer=demux_open(mpctx->stream,mpctx->file_format,audio_id,video_id,dvdsub_id,filename);
if(!mpctx->demuxer)
  return DEMUXER_OPEN_ERR;
initialized_flags|=INITIALIZED_DEMUXER;
current_module="demux_open2";

//file_format=demuxer->file_format;

mpctx->d_audio=mpctx->demuxer->audio;
mpctx->d_video=mpctx->demuxer->video;
mpctx->d_sub=mpctx->demuxer->sub;

select_audio(mpctx->demuxer, audio_id, audio_lang);
// DUMP STREAMS:
if((stream_dump_type)&&(stream_dump_type<4)){
  FILE *f;
  demux_stream_t *ds=NULL;
  current_module="dump";
  // select stream to dump
  switch(stream_dump_type){
  case 1: ds=mpctx->d_audio;break;
  case 2: ds=mpctx->d_video;break;
  case 3: ds=mpctx->d_sub;break;
  }
  if(!ds){
      mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_DumpSelectedStreamMissing);
      exit_player(EXIT_ERROR);
  }
  // disable other streams:
  if(mpctx->d_audio && mpctx->d_audio!=ds) {ds_free_packs(mpctx->d_audio); mpctx->d_audio->id=-2; }
  if(mpctx->d_video && mpctx->d_video!=ds) {ds_free_packs(mpctx->d_video); mpctx->d_video->id=-2; }
  if(mpctx->d_sub && mpctx->d_sub!=ds) {ds_free_packs(mpctx->d_sub); mpctx->d_sub->id=-2; }
  // let's dump it!
  f=fopen(stream_dump_name,"wb");
  if(!f){
    mp_msg(MSGT_CPLAYER,MSGL_FATAL,MSGTR_CantOpenDumpfile);
    exit_player(EXIT_ERROR);
  }
  while(!ds->eof){
    unsigned char* start;
    int in_size=ds_get_packet(ds,&start);
    if( (mpctx->demuxer->file_format==DEMUXER_TYPE_AVI || mpctx->demuxer->file_format==DEMUXER_TYPE_ASF || mpctx->demuxer->file_format==DEMUXER_TYPE_MOV)
	&& stream_dump_type==2) fwrite(&in_size,1,4,f);
    if(in_size>0) fwrite(start,in_size,1,f);
    if(dvd_last_chapter>0) {
      int cur_chapter = demuxer_get_current_chapter(mpctx->demuxer);
      if(cur_chapter!=-1 && cur_chapter+1>dvd_last_chapter)
        break;
    }
  }
  fclose(f);
  mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_CoreDumped);
  exit_player_with_rc(EXIT_EOF, 0);
}

mpctx->sh_audio=mpctx->d_audio->sh;
mpctx->sh_video=mpctx->d_video->sh;

if(mpctx->sh_video){

  current_module="video_read_properties";
  if(!video_read_properties(mpctx->sh_video)) {
    mp_msg(MSGT_CPLAYER,MSGL_ERR,MSGTR_CannotReadVideoProperties);
    mpctx->sh_video=mpctx->d_video->sh=NULL;
  } else {
    mp_msg(MSGT_CPLAYER,MSGL_V,MSGTR_FilefmtFourccSizeFpsFtime,
	   mpctx->demuxer->file_format,mpctx->sh_video->format, mpctx->sh_video->disp_w,mpctx->sh_video->disp_h,
	   mpctx->sh_video->fps,mpctx->sh_video->frametime
	   );

    /* need to set fps here for output encoders to pick it up in their init */
    if(force_fps){
      mpctx->sh_video->fps=force_fps;
      mpctx->sh_video->frametime=1.0f/mpctx->sh_video->fps;
    }
    vo_fps = mpctx->sh_video->fps;

    if(!mpctx->sh_video->fps && !force_fps && !correct_pts){
      mp_msg(MSGT_CPLAYER,MSGL_ERR,MSGTR_FPSnotspecified);
      correct_pts = 1;
    }
  }

}

if(!mpctx->sh_video && !mpctx->sh_audio){
    mp_msg(MSGT_CPLAYER,MSGL_FATAL, MSGTR_NoStreamFound);
    return AUDIO_VIDEO_OPEN_ERR; // exit_player(MSGTR_Exit_error);
}

/* display clip info */
//demux_info_print(mpctx->demuxer);
int i;
if(mpctx->sh_video) {
// after reading video params we should load subtitles because
// we know fps so now we can adjust subtitle time to ~6 seconds AST
// check .sub
  double fps = mpctx->sh_video ? mpctx->sh_video->fps : 25;
}

//print_file_properties(mpctx, filename);

if(!mpctx->sh_video) goto main; // audio-only

if(!reinit_video_chain()) {
  if(!mpctx->sh_video){
    if(!mpctx->sh_audio)  return AUDIO_VIDEO_OPEN_ERR;
    goto main; // exit_player(MSGTR_Exit_error);
  }
}

//================== MAIN: ==========================
main:
current_module="main";

//================ SETUP AUDIO ==========================

if(mpctx->sh_audio){
  reinit_audio_chain();
  if (mpctx->sh_audio && mpctx->sh_audio->codec)
    mp_msg(MSGT_IDENTIFY,MSGL_INFO, "ID_AUDIO_CODEC=%s\n", mpctx->sh_audio->codec->name);
}

return OPEN_SUCCESS_ERR;
}

////////////////////////////////////////////////////
// 功能: 开始播放MPLAYER库
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int PLayMplayerLib()
{
	current_module="av_init";
	
	if(mpctx->sh_video){
	  mpctx->sh_video->timer=0;
	  if (! ignore_start)
		audio_delay += mpctx->sh_video->stream_delay;
	}
	if(mpctx->sh_audio){
	  if (start_volume >= 0)
		mixer_setvolume(&mpctx->mixer, start_volume, start_volume);
	  if (! ignore_start)
		audio_delay -= mpctx->sh_audio->stream_delay;
	  mpctx->delay=-audio_delay;
	}

if(!mpctx->sh_audio){
  mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_NoSound);
  mp_msg(MSGT_CPLAYER,MSGL_V,"Freeing %d unused audio chunks.\n",mpctx->d_audio->packs);
  ds_free_packs(mpctx->d_audio); // free buffered chunks
  //mpctx->d_audio->id=-2;         // do not read audio chunks
  //uninit_player(INITIALIZED_AO); // close device
}
if(!mpctx->sh_video){
   mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_Video_NoVideo);
   mp_msg(MSGT_CPLAYER,MSGL_V,"Freeing %d unused video chunks.\n",mpctx->d_video->packs);
   ds_free_packs(mpctx->d_video);
   mpctx->d_video->id=-2;
   //if(!fixed_vo) uninit_player(INITIALIZED_VO);
}

if (!mpctx->sh_video && !mpctx->sh_audio)
    return AUDIO_VIDEO_OPEN_ERR;

//if(demuxer->file_format!=DEMUXER_TYPE_AVI) pts_from_bps=0; // it must be 0 for mpeg/asf!
if(force_fps && mpctx->sh_video){
  vo_fps = mpctx->sh_video->fps=force_fps;
  mpctx->sh_video->frametime=1.0f/mpctx->sh_video->fps;
  mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_FPSforced,mpctx->sh_video->fps,mpctx->sh_video->frametime);
}

if(mpctx->loop_times>1) mpctx->loop_times--; else
if(mpctx->loop_times==1) mpctx->loop_times = -1;

mp_msg(MSGT_CPLAYER,MSGL_INFO,MSGTR_StartPlaying);

total_time_usage_start=GetTimer();
audio_time_usage=0; video_time_usage=0; vout_time_usage=0;
total_frame_cnt=0; drop_frame_cnt=0; // fix for multifile fps benchmark
play_n_frames=play_n_frames_mf;
mpctx->startup_decode_retry = DEFAULT_STARTUP_DECODE_RETRY;

if(play_n_frames==0){
  mpctx->eof=PT_NEXT_ENTRY; return VIDEO_FRAMES_ERR;
}

if (seek_to_sec) {
    seek(mpctx, seek_to_sec, SEEK_ABSOLUTE);
    end_at.pos += seek_to_sec;
}

if (end_at.type == END_AT_SIZE) {
    mp_msg(MSGT_CPLAYER, MSGL_WARN, MSGTR_MPEndposNoSizeBased);
    end_at.type = END_AT_NONE;
}

if(mp_memory_empty )
{
	mpctx->eof = 1;
	PlayerStatus = MEDIALIB_ERR;
	return VIDEO_FRAMES_ERR;
}

PlayerStatus = MEDIALIB_PLAYING;

return OPEN_SUCCESS_ERR;
}

////////////////////////////////////////////////////
// 功能: 得到PCM数据
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int GetMplayerDataLib()
{
	if(mpctx->eof)
		{
			PlayerStatus = MEDIALIB_END;
			return PLAY_FINISH_ERR;
		}

if(!mpctx->sh_audio && mpctx->d_audio->sh) {
  mpctx->sh_audio = mpctx->d_audio->sh;
  mpctx->sh_audio->ds = mpctx->d_audio;
  reinit_audio_chain();
}

if (mpctx->sh_audio)
    if (!fill_audio_out_buffers())
	// at eof, all audio at least written to ao
	if (!mpctx->sh_video){
	    mpctx->eof = PT_NEXT_ENTRY;
	    PlayerStatus = MEDIALIB_END;
		return PLAY_FINISH_ERR;
	}

if(mp_memory_empty )
{
	mpctx->eof = 1;
	PlayerStatus = MEDIALIB_ERR;
	return MALLOC_ERROR;
}

return OPEN_SUCCESS_ERR;


}

////////////////////////////////////////////////////
// 功能: 得到视频一帧数据
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int GetMplayerVideoDataLib()
{
	float aq_sleep_time=0;
	double frame_time;

	if(mpctx->eof)
	{
		PlayerStatus = MEDIALIB_END;
		return PLAY_FINISH_ERR;
	}

if(mpctx->sh_video){
	if (!mpctx->num_buffered_frames) {
		double frame_time = update_video(&blit_frame);
		while (!blit_frame && mpctx->startup_decode_retry > 0) {
			double delay = mpctx->delay;
			// these initial decode failures are probably due to codec delay,
			// ignore them and also their probably nonsense durations
			update_video(&blit_frame);
			mpctx->delay = delay;
			mpctx->startup_decode_retry--;
		}
		mpctx->startup_decode_retry = 0;
		mp_dbg(MSGT_AVSYNC,MSGL_DBG2,"*** ftime=%5.3f ***\n",frame_time);
		if (mpctx->sh_video->vf_initialized < 0) {
		mp_msg(MSGT_CPLAYER,MSGL_FATAL, MSGTR_NotInitializeVOPorVO);
		mpctx->eof = 1;
		PlayerStatus = MEDIALIB_END;
		return PLAY_FINISH_ERR;
		}
		if (frame_time < 0)
		mpctx->eof = 1;
		else {
		// might return with !eof && !blit_frame if !correct_pts
		mpctx->num_buffered_frames += blit_frame;
		mpctx->time_frame += frame_time / playback_speed;  // for nosound
		}
	}

frame_time_remaining = sleep_until_update(&mpctx->time_frame, &aq_sleep_time);

//====================== FLIP PAGE (VIDEO BLT): =========================

if (!edl_needs_reset) {
    current_module="flip_page";
    if (!frame_time_remaining && blit_frame) {
        mpctx->num_buffered_frames--;
    }
}
//====================== A-V TIMESTAMP CORRECTION: =========================

adjust_sync_and_print_status(frame_time_remaining, mpctx->time_frame);

if (play_n_frames >= 0 && !frame_time_remaining && blit_frame) {
	--play_n_frames;
	if (play_n_frames <= 0) mpctx->eof = PT_NEXT_ENTRY;
}

// FIXME: add size based support for -endpos
 if (end_at.type == END_AT_TIME &&
         !frame_time_remaining && end_at.pos <= mpctx->sh_video->pts)
     mpctx->eof = PT_NEXT_ENTRY;

} // end if(mpctx->sh_video)

return (int)(mpctx->sh_video->pts*1000);
}

////////////////////////////////////////////////////
// 功能: 关闭MPLAYER库
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int MplayerCloseLib()
{
	uninit_player(INITIALIZED_ALL-(INITIALIZED_GUI+INITIALIZED_INPUT+(fixed_vo?INITIALIZED_VO:0)));		
//  watchdog_disable ();
	vo_sub_last = vo_sub=NULL;
	subdata=NULL;
 	mpctx->eof = 1;
 	exit_player_with_rc(EXIT_EOF, 0);
	return OPEN_SUCCESS_ERR;
}

////////////////////////////////////////////////////
// 功能: 得到音视频信息
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int MplayerGetInfo()
{
	if( !mpctx->sh_video && !mpctx->sh_audio )
			return AUDIO_VIDEO_OPEN_ERR;
	
		//得到AUDIO信息
		if( mpctx->sh_audio )
		{
			jz47_av_decp->MediaInfo.bHasAudio = 1;
			jz47_av_decp->MediaInfo.bAllowSeek = mpctx->demuxer->seekable;
			jz47_av_decp->MediaInfo.TotalTime = demuxer_get_time_length(mpctx->demuxer) * 1000;
			jz47_av_decp->MediaInfo.AudioBitrate = mpctx->sh_audio->i_bps;
			jz47_av_decp->MediaInfo.AudioSamplerate = mpctx->sh_audio->samplerate;
			jz47_av_decp->MediaInfo.AudioChannels = mpctx->sh_audio->channels;
			strcpy(jz47_av_decp->AudioCodec,mpctx->sh_audio->codec->name);
		}
		else
		{	//没有音频
			jz47_av_decp->MediaInfo.bHasAudio = 0;
			jz47_av_decp->MediaInfo.bAllowSeek = 0;
			jz47_av_decp->MediaInfo.TotalTime = 0;
			jz47_av_decp->MediaInfo.AudioBitrate = 0;
			jz47_av_decp->MediaInfo.AudioSamplerate = 0;
			jz47_av_decp->MediaInfo.AudioChannels = 0;
			strcpy(jz47_av_decp->AudioCodec,"NULL");
		}
	
		//得到VIDEO信息
		if( mpctx->sh_video )
		{
			jz47_av_decp->MediaInfo.bHasVideo = 1;
			jz47_av_decp->MediaInfo.VideoWidth = mpctx->sh_video->disp_w;
			jz47_av_decp->MediaInfo.VideoHeight = mpctx->sh_video->disp_h;
			jz47_av_decp->MediaInfo.VideoFps = mpctx->sh_video->fps;
			jz47_av_decp->MediaInfo.VideoBitrate = mpctx->sh_video->i_bps;
			strcpy(jz47_av_decp->VideoCodec,mpctx->sh_video->codec->name);
		}
		else
		{	//没有视频
			jz47_av_decp->MediaInfo.bHasVideo = 0;
			jz47_av_decp->MediaInfo.VideoWidth = 0;
			jz47_av_decp->MediaInfo.VideoHeight = 0;
			jz47_av_decp->MediaInfo.VideoFps = 0;
			jz47_av_decp->MediaInfo.VideoBitrate = 0;
			strcpy(jz47_av_decp->VideoCodec,"NULL");
		}
	
		return 0;

}

////////////////////////////////////////////////////
// 功能: 设置播放区域
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void noah_set_lcd_size(int x, int y, int w, int h)
{
	jz47_av_decp->UsrLcdPosX = x;
	jz47_av_decp->UsrLcdPosY = y;
	jz47_av_decp->UsrLcdWidth = w;
	jz47_av_decp->UsrLcdHeight = h;
	IpuOutmodeChanged(jz47_av_decp->UsrLcdPosX,jz47_av_decp->UsrLcdPosY,
		jz47_av_decp->UsrLcdWidth,jz47_av_decp->UsrLcdHeight);
}

////////////////////////////////////////////////////
// 功能: 得到当前的播放状态
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
MEDIALIB_STATUS GetMplayerStatus()
{
	return PlayerStatus;
}

////////////////////////////////////////////////////
// 功能: 播放错误
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMplayerError()
{
	PlayerStatus = MEDIALIB_ERR;
}

////////////////////////////////////////////////////
// 功能: 暂停播放
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMplayerPause()
{
	PlayerStatus = MEDIALIB_PAUSE;
}

////////////////////////////////////////////////////
// 功能: 停止播放
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMplayerStop()
{
	PlayerStatus = MEDIALIB_STOP;
}

////////////////////////////////////////////////////
// 功能: 重新播放
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMplayerResume()
{
	GetRelativeTime();	// ignore time that passed during pause
	PlayerStatus = MEDIALIB_PLAYING;
}

////////////////////////////////////////////////////
// 功能: 设置播放完毕
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMplayerEnd()
{
	PlayerStatus = MEDIALIB_END;
}

////////////////////////////////////////////////////
// 功能: seek播放
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
void SetMplayerSeek(int f)
{
	seek(mpctx, f / 1000.0f ,1);

	rel_seek_secs=0;
	abs_seek_pos=0;
	GetRelativeTime();	// ignore time that passed during pause
}

////////////////////////////////////////////////////
// 功能: 得到播放时间
// 输入: 
// 输出:
// 返回: 
// 说明: 
////////////////////////////////////////////////////
int GetMplayerCurTime()
{
	if(!mpctx->audio_out) return -1;
	float f = playing_audio_pts(mpctx->sh_audio, mpctx->d_audio, mpctx->audio_out);
	f = f* 1000.0;
	return (int)f;
}

long labs(long x)
{
  if (x < 0)
    {
      x = -x;
    }
  return x;
}

