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

#include "config.h"
#ifdef __MINIOS__
#include <mplaylib.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include <unistd.h>

#include "mp_msg.h"
#include "help_mp.h"

#include "osdep/timer.h"
#include "osdep/shmem.h"

#include "stream/stream.h"
#include "libmpdemux/demuxer.h"
#include "libmpdemux/parse_es.h"

#include "codec-cfg.h"

#include "libvo/video_out.h"

#include "libmpdemux/stheader.h"
#include "vd.h"
#include "vf.h"
#include "eosd.h"

#include "dec_video.h"

#ifdef CONFIG_DYNAMIC_PLUGINS
#include <dlfcn.h>
#endif

// ===================================================================

extern double video_time_usage;
extern double vout_time_usage;

#include "cpudetect.h"

int field_dominance = -1;

int divx_quality = 0;

const vd_functions_t *mpvdec = NULL;

int get_video_quality_max(sh_video_t *sh_video)
{
    vf_instance_t *vf = sh_video->vfilter;
    if (vf) {
        int ret = vf->control(vf, VFCTRL_QUERY_MAX_PP_LEVEL, NULL);
        if (ret > 0) {
            mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_UsingExternalPP, ret);
            return ret;
        }
    }
    if (mpvdec) {
        int ret = mpvdec->control(sh_video, VDCTRL_QUERY_MAX_PP_LEVEL, NULL);
        if (ret > 0) {
            mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_UsingCodecPP, ret);
            return ret;
        }
    }
//  mp_msg(MSGT_DECVIDEO,MSGL_INFO,"[PP] Sorry, postprocessing is not available\n");
    return 0;
}

void set_video_quality(sh_video_t *sh_video, int quality)
{
    vf_instance_t *vf = sh_video->vfilter;
    if (vf) {
        int ret = vf->control(vf, VFCTRL_SET_PP_LEVEL, (void *) (&quality));
        if (ret == CONTROL_TRUE)
            return;             // success
    }
    if (mpvdec)
        mpvdec->control(sh_video, VDCTRL_SET_PP_LEVEL, (void *) (&quality));
}

int set_video_colors(sh_video_t *sh_video, const char *item, int value)
{
    vf_instance_t *vf = sh_video->vfilter;
    vf_equalizer_t data;

    data.item  = item;
    data.value = value;

    mp_dbg(MSGT_DECVIDEO, MSGL_V, "set video colors %s=%d \n", item, value);
    if (vf) {
        int ret = vf->control(vf, VFCTRL_SET_EQUALIZER, &data);
        if (ret == CONTROL_TRUE)
            return 1;
    }
    /* try software control */
    if (mpvdec)
        if (mpvdec->control(sh_video, VDCTRL_SET_EQUALIZER, item,
                            (int *) value) == CONTROL_OK)
            return 1;
    mp_msg(MSGT_DECVIDEO, MSGL_V, MSGTR_VideoAttributeNotSupportedByVO_VD,
           item);
    return 0;
}

int get_video_colors(sh_video_t *sh_video, const char *item, int *value)
{
    vf_instance_t *vf = sh_video->vfilter;
    vf_equalizer_t data;

    data.item = item;

    mp_dbg(MSGT_DECVIDEO, MSGL_V, "get video colors %s \n", item);
    if (vf) {
        int ret = vf->control(vf, VFCTRL_GET_EQUALIZER, &data);
        if (ret == CONTROL_TRUE) {
            *value = data.value;
            return 1;
        }
    }
    /* try software control */
    if (mpvdec)
        return mpvdec->control(sh_video, VDCTRL_GET_EQUALIZER, item, value);
    return 0;
}

int set_rectangle(sh_video_t *sh_video, int param, int value)
{
    vf_instance_t *vf = sh_video->vfilter;
    int data[] = { param, value };

    mp_dbg(MSGT_DECVIDEO, MSGL_V, "set rectangle \n");
    if (vf) {
        int ret = vf->control(vf, VFCTRL_CHANGE_RECTANGLE, data);
        if (ret)
            return 1;
    }
    return 0;
}

void resync_video_stream(sh_video_t *sh_video)
{
    sh_video->timer            = 0;
    sh_video->next_frame_time  = 0;
    sh_video->num_buffered_pts = 0;
    sh_video->last_pts         = MP_NOPTS_VALUE;
    if (mpvdec)
        mpvdec->control(sh_video, VDCTRL_RESYNC_STREAM, NULL);
}

int get_current_video_decoder_lag(sh_video_t *sh_video)
{
    int ret;

    if (!mpvdec)
        return -1;
    ret = mpvdec->control(sh_video, VDCTRL_QUERY_UNSEEN_FRAMES, NULL);
    if (ret >= 10)
        return ret - 10;
    return -1;
}

void uninit_video(sh_video_t *sh_video)
{
    if (!sh_video->initialized)
        return;
    mp_msg(MSGT_DECVIDEO, MSGL_V, MSGTR_UninitVideoStr, sh_video->codec->drv);
    mpvdec->uninit(sh_video);
#ifdef CONFIG_DYNAMIC_PLUGINS
    if (sh_video->dec_handle)
        dlclose(sh_video->dec_handle);
#endif
    vf_uninit_filter_chain(sh_video->vfilter);
    eosd_uninit();
    sh_video->initialized = 0;
}

void vfm_help(void)
{
    int i;
    mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_AvailableVideoFm);
    mp_msg(MSGT_IDENTIFY, MSGL_INFO, "ID_VIDEO_DRIVERS\n");
    mp_msg(MSGT_DECVIDEO, MSGL_INFO, "   vfm:    info:  (comment)\n");
    for (i = 0; mpcodecs_vd_drivers[i] != NULL; i++)
        mp_msg(MSGT_DECVIDEO, MSGL_INFO, "%8s  %s (%s)\n",
               mpcodecs_vd_drivers[i]->info->short_name,
               mpcodecs_vd_drivers[i]->info->name,
               mpcodecs_vd_drivers[i]->info->comment);
}

static int init_video(sh_video_t *sh_video, char *codecname, char *vfm,
                      int status, stringset_t *selected)
{
    int force = 0;
    unsigned int orig_fourcc = sh_video->bih ? sh_video->bih->biCompression : 0;
    sh_video->codec = NULL;
    sh_video->vf_initialized = 0;
    if (codecname && codecname[0] == '+') {
        codecname = &codecname[1];
        force = 1;
    }

    while (1) {
        int i;
        int orig_w, orig_h;
        // restore original fourcc:
        if (sh_video->bih)
            sh_video->bih->biCompression = orig_fourcc;
        if (!(sh_video->codec =
              find_video_codec(sh_video->format,
                               sh_video->bih ? ((unsigned int *) &sh_video->bih->biCompression) : NULL,
                               sh_video->codec, force)))
            break;
        // ok we found one codec
        if (stringset_test(selected, sh_video->codec->name))
            continue;           // already tried & failed
        if (codecname && strcmp(sh_video->codec->name, codecname))
            continue;           // -vc
        if (vfm && strcmp(sh_video->codec->drv, vfm))
            continue;           // vfm doesn't match
        if (!force && sh_video->codec->status < status)
            continue;           // too unstable
        stringset_add(selected, sh_video->codec->name); // tagging it
        // ok, it matches all rules, let's find the driver!
        for (i = 0; mpcodecs_vd_drivers[i] != NULL; i++)
//          if(mpcodecs_vd_drivers[i]->info->id==sh_video->codec->driver) break;
            if (!strcmp
                (mpcodecs_vd_drivers[i]->info->short_name,
                 sh_video->codec->drv))
                break;
        mpvdec = mpcodecs_vd_drivers[i];
#ifdef CONFIG_DYNAMIC_PLUGINS
        if (!mpvdec) {
            /* try to open shared decoder plugin */
            int buf_len;
            char *buf;
            vd_functions_t *funcs_sym;
            vd_info_t *info_sym;

            buf_len = strlen(MPLAYER_LIBDIR) +
                      strlen(sh_video->codec->drv) + 16;
            buf = malloc(buf_len);
            if (!buf)
                break;
            snprintf(buf, buf_len, "%s/mplayer/vd_%s.so", MPLAYER_LIBDIR,
                     sh_video->codec->drv);
            mp_msg(MSGT_DECVIDEO, MSGL_DBG2,
                   "Trying to open external plugin: %s\n", buf);
            sh_video->dec_handle = dlopen(buf, RTLD_LAZY);
            if (!sh_video->dec_handle)
                break;
            snprintf(buf, buf_len, "mpcodecs_vd_%s", sh_video->codec->drv);
            funcs_sym = dlsym(sh_video->dec_handle, buf);
            if (!funcs_sym || !funcs_sym->info || !funcs_sym->init
                || !funcs_sym->uninit || !funcs_sym->control
                || !funcs_sym->decode)
                break;
            info_sym = funcs_sym->info;
            if (strcmp(info_sym->short_name, sh_video->codec->drv))
                break;
            free(buf);
            mpvdec = funcs_sym;
            mp_msg(MSGT_DECVIDEO, MSGL_V,
                   "Using external decoder plugin (%s/mplayer/vd_%s.so)!\n",
                   MPLAYER_LIBDIR, sh_video->codec->drv);
        }
#endif
        if (!mpvdec) {          // driver not available (==compiled in)
            mp_msg(MSGT_DECVIDEO, MSGL_WARN,
                   MSGTR_VideoCodecFamilyNotAvailableStr,
                   sh_video->codec->name, sh_video->codec->drv);
            continue;
        }
        orig_w = sh_video->bih ? sh_video->bih->biWidth  : sh_video->disp_w;
        orig_h = sh_video->bih ? sh_video->bih->biHeight : sh_video->disp_h;
        sh_video->disp_w = orig_w;
        sh_video->disp_h = orig_h;
        // it's available, let's try to init!
        if (sh_video->codec->flags & CODECS_FLAG_ALIGN16) {
            // align width/height to n*16
            sh_video->disp_w = (sh_video->disp_w + 15) & (~15);
            sh_video->disp_h = (sh_video->disp_h + 15) & (~15);
        }
        if (sh_video->bih) {
            sh_video->bih->biWidth  = sh_video->disp_w;
            sh_video->bih->biHeight = sh_video->disp_h;
        }
        // init()
        mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_OpeningVideoDecoder,
               mpvdec->info->short_name, mpvdec->info->name);
        // clear vf init error, it is no longer relevant
        if (sh_video->vf_initialized < 0)
            sh_video->vf_initialized = 0;
        if (!mpvdec->init(sh_video)) {
            mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_VDecoderInitFailed);
            sh_video->disp_w = orig_w;
            sh_video->disp_h = orig_h;
            if (sh_video->bih) {
                sh_video->bih->biWidth  = sh_video->disp_w;
                sh_video->bih->biHeight = sh_video->disp_h;
            }
            continue;           // try next...
        }
        // Yeah! We got it!
        sh_video->initialized = 1;
        return 1;
    }
    return 0;
}

int init_best_video_codec(sh_video_t *sh_video, char **video_codec_list,
                          char **video_fm_list)
{
    char *vc_l_default[2] = { "", (char *) NULL };
    stringset_t selected;
    // hack:
    if (!video_codec_list)
        video_codec_list = vc_l_default;
    // Go through the codec.conf and find the best codec...
    sh_video->initialized = 0;
    stringset_init(&selected);
    while (!sh_video->initialized && *video_codec_list) {
        char *video_codec = *(video_codec_list++);
        if (video_codec[0]) {
            if (video_codec[0] == '-') {
                // disable this codec:
                stringset_add(&selected, video_codec + 1);
            } else {
                // forced codec by name:
                mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_ForcedVideoCodec,
                       video_codec);
                init_video(sh_video, video_codec, NULL, -1, &selected);
            }
        } else {
            int status;
            // try in stability order: UNTESTED, WORKING, BUGGY. never try CRASHING.
            if (video_fm_list) {
                char **fmlist = video_fm_list;
                // try first the preferred codec families:
                while (!sh_video->initialized && *fmlist) {
                    char *video_fm = *(fmlist++);
                    mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_TryForceVideoFmtStr,
                           video_fm);
                    for (status = CODECS_STATUS__MAX;
                         status >= CODECS_STATUS__MIN; --status)
                        if (init_video
                            (sh_video, NULL, video_fm, status, &selected))
                            break;
                }
            }
            if (!sh_video->initialized)
                for (status = CODECS_STATUS__MAX; status >= CODECS_STATUS__MIN;
                     --status)
                    if (init_video(sh_video, NULL, NULL, status, &selected))
                        break;
        }
    }
    stringset_free(&selected);

    if (!sh_video->initialized) {
        mp_msg(MSGT_DECVIDEO, MSGL_ERR, MSGTR_CantFindVideoCodec,
               sh_video->format);
        return 0;               // failed
    }

    mp_msg(MSGT_DECVIDEO, MSGL_INFO, MSGTR_SelectedVideoCodec,
           sh_video->codec->name, sh_video->codec->drv, sh_video->codec->info);
    return 1;                   // success
}
static void ptr_square(void * start_ptr,int size,int h,int w, int stride){
  unsigned int* start_int=(int*)start_ptr;
  unsigned short* start_short=(short*)start_ptr;
  unsigned char* start_byte=(char*)start_ptr;
  int i, j;
  if(size==4){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	kprintf("0x%08x,",start_int[i*stride+j]);
      }
      kprintf("\n");
    }
  }
  if(size==2){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	kprintf("0x%04x,",start_short[i*stride+j]);
      }
      kprintf("\n");
    }
  }
  if(size==1){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	kprintf("0x%02x,",start_byte[i*stride+j]);
      }
      kprintf("\n");
    }
  }
}

static void print_frminfo(mp_image_t *s, int *yidx, int *uidx, int *vidx){
  int i,j;
  char *ptr = s->planes[0];

  if (yidx[0] >= 0){
    for (j = yidx[2]; j < yidx[3]; j++){ 
      for (i = yidx[0]; i < yidx[1]; i++){
        kprintf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + j*s->stride[0]*16 + i*16), 1, 16, 16, s->stride[0]);
      }
    }
  }

  ptr = s->planes[1];
  if (uidx[0] >= 0){
    for (j = uidx[2]; j < uidx[3]; j++){
      for (i = uidx[0]; i < uidx[1]; i++){
        kprintf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + j*s->stride[1]*8 + i*8), 1, 8, 8, s->stride[1]);
      }
    }
  }

  ptr = s->planes[2];
  if (vidx[0] >= 0){
    for (j = vidx[2]; j < vidx[3]; j++){
      for (i = vidx[0]; i < vidx[1]; i++){
        kprintf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + j*s->stride[1]*8 + i*8), 1, 8, 8, s->stride[1]);
      }
    }
  }
}
#define CRC_32 0x04c11db7
unsigned int image_crc32=0;
unsigned int crc32_tab[256]={
         0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
         0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
         0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
         0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
         0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
         0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
         0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
         0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
         0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
         0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
         0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
         0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
         0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
         0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
         0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
         0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
         0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
         0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
         0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
         0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
         0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
         0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
         0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
         0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
         0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
         0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
         0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
         0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
         0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
         0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
         0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
         0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
         0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
         0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
         0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
         0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
         0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
         0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
         0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
         0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
         0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
         0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
         0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
         0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
         0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
         0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
         0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
         0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
         0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
         0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
         0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
         0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
         0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
         0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
         0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
         0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
         0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
         0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
         0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
         0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
         0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
         0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
         0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
         0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4, };

unsigned int frame_no=0;
unsigned int crc32_verify(unsigned char *buf, int line, int col, int stride)
{
  int i, j, counter = 0;

  for (j = 0; j < line; j++)
  {
    for (i = 0; i < col; i++){
      image_crc32 = ((image_crc32 << 8) | buf[i]) ^ (crc32_tab[(image_crc32 >> 24) & 0xff]);
    }
    buf += stride;
  }
}

void *decode_video(sh_video_t *sh_video, unsigned char *start, int in_size,
                   int drop_frame, double pts)
{
    mp_image_t *mpi = NULL;
    unsigned int t = GetTimer();
    unsigned int t2;
    double tt;
    int delay;

    mpi = mpvdec->decode(sh_video, start, in_size, drop_frame);
//		unsigned int img_area = mpi->w * mpi->h;
		static int mpFrame=0;
		if (mpi) {
//			crc32_verify(start, 1, in_size, in_size);
//			crc32_verify(mpi->planes[0], mpi->h, mpi->w, mpi->w);
//			crc32_verify(mpi->planes[1], mpi->h/2, mpi->w/2, mpi->w/2);
//			crc32_verify(mpi->planes[2], mpi->h/2, mpi->w/2, mpi->w/2);
		
//			kprintf ("++++	%d	%d	image_crc32 = 0x%08x     %d\n", mpi->w,mpi->h,image_crc32,in_size);		
		if (0){
		if (mpFrame == 0){
		  int yidx[4];
		  int uidx[4];
		  int vidx[4];
			  
		  //printf("Frame34 cu 0x%08x\n", s->current_picture.data[0]);
		  yidx[0] = 0;
		  yidx[1] = mpi->width/16;
		  //yidx[1] = 3;
		  yidx[2] = 0;
		  yidx[3] = mpi->height/16;
		  //yidx[3] = 1;
		
		  uidx[0] = 0;
		  uidx[1] = mpi->width/16;
		  uidx[2] = 0;
		  uidx[3] = mpi->height/16;
		  //uidx[3] = 31;
				  
		  vidx[0] = 0;
		  vidx[1] = mpi->width/16;
		  vidx[2] = 0;
		  vidx[3] = mpi->height/16;
			  
		  print_frminfo(mpi, yidx, uidx, vidx);
		} 
		}
			mpFrame++;
		}

    //------------------------ frame decoded. --------------------

    if (mpi && mpi->type == MP_IMGTYPE_INCOMPLETE) {
	mpi = NULL;
    }

    delay = get_current_video_decoder_lag(sh_video);
    if (correct_pts && pts != MP_NOPTS_VALUE) {
        if (sh_video->num_buffered_pts ==
            sizeof(sh_video->buffered_pts) / sizeof(double))
            mp_msg(MSGT_DECVIDEO, MSGL_ERR, "Too many buffered pts\n");
        else {
            int i, j;
            for (i = 0; i < sh_video->num_buffered_pts; i++)
                if (sh_video->buffered_pts[i] < pts)
                    break;
            for (j = sh_video->num_buffered_pts; j > i; j--)
                sh_video->buffered_pts[j] = sh_video->buffered_pts[j - 1];
            sh_video->buffered_pts[i] = pts;
            sh_video->num_buffered_pts++;
        }
    }

#if HAVE_MMX
    // some codecs are broken, and doesn't restore MMX state :(
    // it happens usually with broken/damaged files.
    if (gCpuCaps.has3DNow) {
        __asm__ volatile ("femms\n\t":::"memory");
    } else if (gCpuCaps.hasMMX) {
        __asm__ volatile ("emms\n\t":::"memory");
    }
#endif

    t2 = GetTimer();
    t = t2 - t;
    tt = t * 0.000001f;
    video_time_usage += tt;

    if (!mpi || drop_frame)
        return NULL;            // error / skipped frame

    if (field_dominance == 0)
        mpi->fields |= MP_IMGFIELD_TOP_FIRST;
    else if (field_dominance == 1)
        mpi->fields &= ~MP_IMGFIELD_TOP_FIRST;

    if (correct_pts) {
        if (sh_video->num_buffered_pts) {
            sh_video->num_buffered_pts--;
            sh_video->pts = sh_video->buffered_pts[sh_video->num_buffered_pts];
        } else {
            mp_msg(MSGT_CPLAYER, MSGL_ERR,
                   "No pts value from demuxer to " "use for frame!\n");
            sh_video->pts = MP_NOPTS_VALUE;
        }
        if (delay >= 0) {
            // limit buffered pts only afterwards so we do not get confused
            // by packets that produce no output (e.g. a single field of a
            // H.264 frame).
            if (delay > sh_video->num_buffered_pts)
#if 0
                // this is disabled because vd_ffmpeg reports the same lag
                // after seek even when there are no buffered frames,
                // leading to incorrect error messages
                mp_msg(MSGT_DECVIDEO, MSGL_ERR, "Not enough buffered pts\n");
#else
                ;
#endif
            else
                sh_video->num_buffered_pts = delay;
        }
    }
    return mpi;
}

int filter_video(sh_video_t *sh_video, void *frame, double pts)
{
    mp_image_t *mpi = frame;
    unsigned int t2 = GetTimer();
    vf_instance_t *vf = sh_video->vfilter;
    // apply video filters and call the leaf vo/ve
    int ret = vf->put_image(vf, mpi, pts);
    if (ret > 0) {
        // draw EOSD first so it ends up below the OSD.
        // Note that changing this is will not work right with vf_ass and the
        // vos currently always draw the EOSD first in paused mode.
#ifdef CONFIG_ASS
        vf->control(vf, VFCTRL_DRAW_EOSD, NULL);
#endif
        vf->control(vf, VFCTRL_DRAW_OSD, NULL);
    }

    t2 = GetTimer() - t2;
    vout_time_usage += t2 * 0.000001;

    return ret;
}
