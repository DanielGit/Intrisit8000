/*
 * VC-1 and WMV3 decoder
 * Copyright (c) 2006-2007 Konstantin Shishkov
 * Partly based on vc9.c (c) 2005 Anonymous, Alex Beregszaszi, Michael Niedermayer
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * VC-1 and WMV3 decoder
 *
 */
#include "internal.h"
#include "dsputil.h"
#include "avcodec.h"
#include "mpegvideo.h"
#include "h263.h"
#include "vc1.h"
#include "vc1data.h"
#include "vc1acdata.h"
#include "msmpeg4data.h"
#include "unary.h"
#include "simple_idct.h"
#include "mathops.h"
#include "vdpau_internal.h"

#include "jzsoc/t_motion.h"
/*------JZ OPT area------*/
#define JZ-IN-MODIFY
/*----------------------*/
#ifdef JZ-IN-MODIFY
#undef printf
#undef fprintf
short mpFrame = 0;

#ifdef JZC_CRC_VER
#include "../libjzcommon/crc.c"
short crc_code;
#endif//JZC_CRC_VER
#ifdef JZC_PMON_P0
#include "../libjzcommon/jz4760e_pmon.h"
FILE *pmon_p0_fp;
extern char* filename;
int vc1_pmon_p0_frmcnt = 0;
PMON_CREAT(vc1vlc);
PMON_CREAT(vc1wait);
PMON_CREAT(taskwhile);
#endif//JZC_PMON_P0
#endif//jz-in-modify

#ifdef JZC_DCORE_OPT
//uint8_t luty[256],lutuv[256];
//uint32_t current_picture_ptr[3];
#include "jzsoc/vc1_tcsm0.h"
#include "jzsoc/vc1_tcsm1.h"
#include "jzsoc/vc1_dcore.h"
#include "jzsoc/jz4760_dcsc.h"
#include "../libjzcommon/com_config.h"
#include "../libjzcommon/jzasm.h"
VC1_MB_DecARGs *task_fifo_mem;
int *frame_info_mem;
VC1_Frame_GlbARGs * dFRM;
VC1_MB_DecARGs *dMB;
volatile int * task_fifo_wp;
volatile int * task_fifo_wp_d1;
volatile int * task_fifo_wp_d2;
volatile int * tcsm1_fifo_wp;
volatile int * tcsm0_fifo_rp;
#endif

#if 1 //#ifdef JZ_LINUX_OS
extern unsigned int vc1_auxcodes_len, vc1_aux_task_codes[];
#endif

#include "jzsoc/t_intpid.h"
#ifdef JZC_TLB_OPT
volatile int tlb_i;
#include "jzsoc/t_vputlb.h"
#endif

#ifdef JZC_MXU_OPT
//#include "jzsoc/jzasm.h"
#include "jzsoc/jzmedia.h"
#include "vc1vlc_mxu.c"
#endif

#ifdef  JZC_TCSM_OPT
#include "jzsoc/jz4760_tcsm_init.c"
#endif

#ifdef JZC_VLC_HW_OPT
volatile char * vc1_hw_bs_buffer;
int vc1_hw_codingset1=0xab;
int vc1_hw_codingset2=0xab;

#include "vlc_tables.h"
#include "vlc_bs.c"
#endif
/*------JZ OPT area------*/

#ifdef JZC_PMON_P1
#undef fprintf
FILE *pmon_p1_fp;
extern char* filename;
int pmon_p1_frmcnt = 0;
int *pmon_p1_ptr=TCSM1_VUCADDR(TCSM1_PMON_BUF);
#endif //JZC_PMON_P0

#ifdef JZC_ROTA90_OPT
uint8_t *rota_y, *rota_c;
short crc_code_rota;
#endif
extern int use_jz_buf;
static uint8_t *buf2_pre_alloc_buf;

#undef NDEBUG
#include <assert.h>

#define MB_INTRA_VLC_BITS 8
#define DC_VLC_BITS 8
#define AC_VLC_BITS 8
//static const uint16_t table_mb_intra[64][2];

int dma_len = 0;
#if 0
static const uint16_t vlc_offs[] = {
  0,   520,   552,   616,  1128,  1160, 1224, 1740, 1772, 1836, 1900, 2436,
  2986,  3050,  3610,  4154,  4218,  4746, 5326, 5390, 5902, 6554, 7658, 8620,
  9262, 10202, 10756, 11310, 12228, 15078
};
#else
static const uint16_t vlc_offs[] = {
       0,   274,   306,   370,  626,  658, 722, 986, 1018, 1082, 1146, 1434,
    1772,  1836,  2156,  2486,  2550,  2846, 3174, 3238, 3550, 3948, 5312, 6440,
    6938, 8010, 8334, 8658, 9682, 11546
};
#endif
/**
 * Init VC-1 specific tables and VC1Context members
 * @param v The VC1Context to initialize
 * @return Status
 */
static int vc1_init_common(VC1Context *v)
{
  static int done = 0;
  int i = 0;
  static VLC_TYPE vlc_table[11546][2];

  v->hrd_rate = v->hrd_buffer = NULL;

  /* VLC tables */
  // fixed to avoid mpeg4 and wmv decoder conflict when playing together  (by Daniel)
  if(!done || 1)
    {

      vc1_hw_codingset1=0xab;   // Set it's init value to do re-init the vlc tables
      vc1_hw_codingset2=0xab;   // Set it's init value to do re-init the vlc tables

      INIT_VLC_STATIC(&ff_vc1_bfraction_vlc, VC1_BFRACTION_VLC_BITS, 23,
		      ff_vc1_bfraction_bits, 1, 1,
		      ff_vc1_bfraction_codes, 1, 1, 1 << VC1_BFRACTION_VLC_BITS);
      INIT_VLC_STATIC(&ff_vc1_norm2_vlc, VC1_NORM2_VLC_BITS, 4,
		      ff_vc1_norm2_bits, 1, 1,
		      ff_vc1_norm2_codes, 1, 1, 1 << VC1_NORM2_VLC_BITS);
      INIT_VLC_STATIC(&ff_vc1_norm6_vlc, VC1_NORM6_VLC_BITS, 64,
		      ff_vc1_norm6_bits, 1, 1,
		      ff_vc1_norm6_codes, 2, 2, 556);
      INIT_VLC_STATIC(&ff_vc1_imode_vlc, VC1_IMODE_VLC_BITS, 7,
		      ff_vc1_imode_bits, 1, 1,
		      ff_vc1_imode_codes, 1, 1, 1 << VC1_IMODE_VLC_BITS);
      for (i=0; i<3; i++)
        {
	  ff_vc1_ttmb_vlc[i].table = &vlc_table[vlc_offs[i*3+0]];
	  ff_vc1_ttmb_vlc[i].table_allocated = vlc_offs[i*3+1] - vlc_offs[i*3+0];
	  init_vlc(&ff_vc1_ttmb_vlc[i], VC1_TTMB_VLC_BITS, 16,
		   ff_vc1_ttmb_bits[i], 1, 1,
		   ff_vc1_ttmb_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
	  ff_vc1_ttblk_vlc[i].table = &vlc_table[vlc_offs[i*3+1]];
	  ff_vc1_ttblk_vlc[i].table_allocated = vlc_offs[i*3+2] - vlc_offs[i*3+1];
	  init_vlc(&ff_vc1_ttblk_vlc[i], VC1_TTBLK_VLC_BITS, 8,
		   ff_vc1_ttblk_bits[i], 1, 1,
		   ff_vc1_ttblk_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
	  ff_vc1_subblkpat_vlc[i].table = &vlc_table[vlc_offs[i*3+2]];
	  ff_vc1_subblkpat_vlc[i].table_allocated = vlc_offs[i*3+3] - vlc_offs[i*3+2];
	  init_vlc(&ff_vc1_subblkpat_vlc[i], VC1_SUBBLKPAT_VLC_BITS, 15,
		   ff_vc1_subblkpat_bits[i], 1, 1,
		   ff_vc1_subblkpat_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
        }
      for(i=0; i<4; i++)
        {
	  ff_vc1_4mv_block_pattern_vlc[i].table = &vlc_table[vlc_offs[i*3+9]];
	  ff_vc1_4mv_block_pattern_vlc[i].table_allocated = vlc_offs[i*3+10] - vlc_offs[i*3+9];
	  init_vlc(&ff_vc1_4mv_block_pattern_vlc[i], VC1_4MV_BLOCK_PATTERN_VLC_BITS, 16,
		   ff_vc1_4mv_block_pattern_bits[i], 1, 1,
		   ff_vc1_4mv_block_pattern_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
	  ff_vc1_cbpcy_p_vlc[i].table = &vlc_table[vlc_offs[i*3+10]];
	  ff_vc1_cbpcy_p_vlc[i].table_allocated = vlc_offs[i*3+11] - vlc_offs[i*3+10];
	  init_vlc(&ff_vc1_cbpcy_p_vlc[i], VC1_CBPCY_P_VLC_BITS, 64,
		   ff_vc1_cbpcy_p_bits[i], 1, 1,
		   ff_vc1_cbpcy_p_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
	  ff_vc1_mv_diff_vlc[i].table = &vlc_table[vlc_offs[i*3+11]];
	  ff_vc1_mv_diff_vlc[i].table_allocated = vlc_offs[i*3+12] - vlc_offs[i*3+11];
	  init_vlc(&ff_vc1_mv_diff_vlc[i], VC1_MV_DIFF_VLC_BITS, 73,
		   ff_vc1_mv_diff_bits[i], 1, 1,
		   ff_vc1_mv_diff_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
        }
      for(i=0; i<8; i++){
	ff_vc1_ac_coeff_table[i].table = &vlc_table[vlc_offs[i+21]];
	ff_vc1_ac_coeff_table[i].table_allocated = vlc_offs[i+22] - vlc_offs[i+21];
	init_vlc(&ff_vc1_ac_coeff_table[i], AC_VLC_BITS, vc1_ac_sizes[i],
		 &vc1_ac_tables[i][0][1], 8, 4,
		 &vc1_ac_tables[i][0][0], 8, 4, INIT_VLC_USE_NEW_STATIC);
      }
      done = 1;
    }

  /* Other defaults */
  v->pq = -1;
  v->mvrange = 0; /* 7.1.1.18, p80 */

  return 0;
}

/***********************************************************************/
/**
 * @defgroup vc1bitplane VC-1 Bitplane decoding
 * @see 8.7, p56
 * @{
 */

/**
 * Imode types
 * @{
 */
enum Imode {
  IMODE_RAW,
  IMODE_NORM2,
  IMODE_DIFF2,
  IMODE_NORM6,
  IMODE_DIFF6,
  IMODE_ROWSKIP,
  IMODE_COLSKIP
};
/** @} */ //imode defines

/** Do motion compensation over 1 macroblock
 * Mostly adapted hpel_motion and qpel_motion from mpegvideo.c
 */
#ifdef JZC_DCORE_OPTn
static void vc1_mc_1mv(VC1Context *v, int dir)
{
  MpegEncContext *s = &v->s;
  DSPContext *dsp = &v->s.dsp;
  uint8_t *srcY, *srcU, *srcV;
  int dxy, mx, my, uvmx, uvmy, src_x, src_y, uvsrc_x, uvsrc_y;

  if(!dFRM->last_data[0])return;
  
  mx = dMB->vc1_mv[dir][0];
  my = dMB->vc1_mv[dir][1];

  uvmx = (mx + ((mx & 3) == 3)) >> 1;
  uvmy = (my + ((my & 3) == 3)) >> 1;
  if(dFRM->fastuvmc) {
    uvmx = uvmx + ((uvmx<0)?(uvmx&1):-(uvmx&1));
    uvmy = uvmy + ((uvmy<0)?(uvmy&1):-(uvmy&1));
  }
  if(!dir) {
    srcY = dFRM->last_data[0];
    srcU = dFRM->last_data[1];
    srcV = dFRM->last_data[2];
  } else {
    srcY = dFRM->next_data[0];
    srcU = dFRM->next_data[1];
    srcV = dFRM->next_data[2];
  }

  src_x = dMB->mb_x * 16 + (mx >> 2);
  src_y = dMB->mb_y * 16 + (my >> 2);
  uvsrc_x = dMB->mb_x * 8 + (uvmx >> 2);
  uvsrc_y = dMB->mb_y * 8 + (uvmy >> 2);

  if(dFRM->profile != PROFILE_ADVANCED){
    src_x   = av_clip(  src_x, -16, dFRM->mb_width  * 16);
    src_y   = av_clip(  src_y, -16, dFRM->mb_height * 16);
    uvsrc_x = av_clip(uvsrc_x,  -8, dFRM->mb_width  *  8);
    uvsrc_y = av_clip(uvsrc_y,  -8, dFRM->mb_height *  8);
  }else{
    src_x   = av_clip(  src_x, -17, dFRM->coded_width);
    src_y   = av_clip(  src_y, -18, dFRM->coded_height + 1);
    uvsrc_x = av_clip(uvsrc_x,  -8, dFRM->coded_width  >> 1);
    uvsrc_y = av_clip(uvsrc_y,  -8, dFRM->coded_height >> 1);
  }

  srcY += src_y * dFRM->linesize + src_x;
  srcU += uvsrc_y * dFRM->uvlinesize + uvsrc_x;
  srcV += uvsrc_y * dFRM->uvlinesize + uvsrc_x;

  /* for grayscale we should not try to read from unknown area */
  if(dFRM->flags & CODEC_FLAG_GRAY) {
    srcU = dFRM->edge_emu_buffer + 18 * dFRM->linesize;
    srcV = dFRM->edge_emu_buffer + 18 * dFRM->linesize;
  }

  if(dFRM->rangeredfrm || (dMB->mv_mode == MV_PMODE_INTENSITY_COMP)
     || (unsigned)(src_x - dFRM->mspel) > dFRM->h_edge_pos - (mx&3) - 16 - dFRM->mspel*3
     || (unsigned)(src_y - dFRM->mspel) > dFRM->v_edge_pos - (my&3) - 16 - dFRM->mspel*3){
    uint8_t *uvbuf= dFRM->edge_emu_buffer + 19 * dFRM->linesize;

    srcY -= dFRM->mspel * (1 + dFRM->linesize);
    ff_emulated_edge_mc(dFRM->edge_emu_buffer, srcY, dFRM->linesize, 17+dFRM->mspel*2, 17+dFRM->mspel*2,
			src_x - dFRM->mspel, src_y - dFRM->mspel, dFRM->h_edge_pos, dFRM->v_edge_pos);
    srcY = dFRM->edge_emu_buffer;
    ff_emulated_edge_mc(uvbuf     , srcU, dFRM->uvlinesize, 8+1, 8+1,
			uvsrc_x, uvsrc_y, dFRM->h_edge_pos >> 1, dFRM->v_edge_pos >> 1);
    ff_emulated_edge_mc(uvbuf + 16, srcV, dFRM->uvlinesize, 8+1, 8+1,
			uvsrc_x, uvsrc_y, dFRM->h_edge_pos >> 1, dFRM->v_edge_pos >> 1);
    srcU = uvbuf;
    srcV = uvbuf + 16;
    /* if we deal with range reduction we need to scale source blocks */
    if(dFRM->rangeredfrm) {
      int i, j;
      uint8_t *src, *src2;

      src = srcY;
      for(j = 0; j < 17 + dFRM->mspel*2; j++) {
	for(i = 0; i < 17 + dFRM->mspel*2; i++) src[i] = ((src[i] - 128) >> 1) + 128;
	src += dFRM->linesize;
      }
      src = srcU; src2 = srcV;
      for(j = 0; j < 9; j++) {
	for(i = 0; i < 9; i++) {
	  src[i] = ((src[i] - 128) >> 1) + 128;
	  src2[i] = ((src2[i] - 128) >> 1) + 128;
	}
	src += dFRM->uvlinesize;
	src2 += dFRM->uvlinesize;
      }
    }
    /* if we deal with intensity compensation we need to scale source blocks */
    if(dMB->mv_mode == MV_PMODE_INTENSITY_COMP) {
      int i, j;
      uint8_t *src, *src2;

      src = srcY;
      for(j = 0; j < 17 + dFRM->mspel*2; j++) {
	for(i = 0; i < 17 + dFRM->mspel*2; i++) src[i] = luty[src[i]];
	src += dFRM->linesize;
      }
      src = srcU; src2 = srcV;
      for(j = 0; j < 9; j++) {
	for(i = 0; i < 9; i++) {
	  src[i] = lutuv[src[i]];
	  src2[i] = lutuv[src2[i]];
	}
	src += dFRM->uvlinesize;
	src2 += dFRM->uvlinesize;
      }
    }
    srcY += dFRM->mspel * (1 + dFRM->linesize);
  }

  if(dFRM->mspel) {
    dxy = ((my & 3) << 2) | (mx & 3);
    dsp->put_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0], srcY    , dFRM->linesize, dFRM->rnd);
    dsp->put_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0]+8, srcY + 8, dFRM->linesize, dFRM->rnd);
    srcY += dFRM->linesize * 8;
    dsp->put_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0]+8*dFRM->linesize, srcY, dFRM->linesize, dFRM->rnd);
    dsp->put_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0]+8*dFRM->linesize + 8, srcY + 8, dFRM->linesize, dFRM->rnd);
  } else { // hpel mc - always used for luma
    dxy = (my & 2) | ((mx & 2) >> 1);
    if(!dFRM->rnd)
      dsp->put_pixels_tab[0][dxy](current_picture_ptr[0], srcY, dFRM->linesize, 16);
    else
      dsp->put_no_rnd_pixels_tab[0][dxy](current_picture_ptr[0], srcY, dFRM->linesize, 16);
  }

  if(dFRM->flags & CODEC_FLAG_GRAY) return;
  /* Chroma MC always uses qpel bilinear */
  uvmx = (uvmx&3)<<1;
  uvmy = (uvmy&3)<<1;
  if(!dFRM->rnd){
    dsp->put_h264_chroma_pixels_tab[0](current_picture_ptr[1], srcU, dFRM->uvlinesize, 8, uvmx, uvmy);
    dsp->put_h264_chroma_pixels_tab[0](current_picture_ptr[2], srcV, dFRM->uvlinesize, 8, uvmx, uvmy);
  }else{
    dsp->put_no_rnd_vc1_chroma_pixels_tab[0](current_picture_ptr[1], srcU, dFRM->uvlinesize, 8, uvmx, uvmy);
    dsp->put_no_rnd_vc1_chroma_pixels_tab[0](current_picture_ptr[2], srcV, dFRM->uvlinesize, 8, uvmx, uvmy);
  }
}
#endif
/** Do motion compensation for 4-MV macroblock - luminance block
 */
#ifdef JZC_DCORE_OPTn
static void vc1_mc_4mv_luma(VC1Context *v, int n)
{
  MpegEncContext *s = &v->s;
  DSPContext *dsp = &v->s.dsp;
  uint8_t *srcY;
  int dxy, mx, my, src_x, src_y;
  int off;

  if(!dFRM->last_data[0]) return;

  mx = dMB->vc1_mv[n][0];
  my = dMB->vc1_mv[n][1];

  srcY = dFRM->last_data[0];

  off = dFRM->linesize * 4 * (n&2) + (n&1) * 8;

  src_x = dMB->mb_x * 16 + (n&1) * 8 + (mx >> 2);
  src_y = dMB->mb_y * 16 + (n&2) * 4 + (my >> 2);

  if(dFRM->profile != PROFILE_ADVANCED){
    src_x   = av_clip(  src_x, -16, dFRM->mb_width  * 16);
    src_y   = av_clip(  src_y, -16, dFRM->mb_height * 16);
  }else{
    src_x   = av_clip(  src_x, -17, dFRM->coded_width);
    src_y   = av_clip(  src_y, -18, dFRM->coded_height + 1);
  }

  srcY += src_y * dFRM->linesize + src_x;

  if(dFRM->rangeredfrm || (dMB->mv_mode == MV_PMODE_INTENSITY_COMP)
     || (unsigned)(src_x - dFRM->mspel) > dFRM->h_edge_pos - (mx&3) - 8 - dFRM->mspel*2
     || (unsigned)(src_y - dFRM->mspel) > dFRM->v_edge_pos - (my&3) - 8 - dFRM->mspel*2){
    srcY -= dFRM->mspel * (1 + dFRM->linesize);
    ff_emulated_edge_mc(dFRM->edge_emu_buffer, srcY, dFRM->linesize, 9+dFRM->mspel*2, 9+dFRM->mspel*2,
			src_x - dFRM->mspel, src_y - dFRM->mspel, dFRM->h_edge_pos, dFRM->v_edge_pos);
    srcY = dFRM->edge_emu_buffer;
    /* if we deal with range reduction we need to scale source blocks */
    if(dFRM->rangeredfrm) {
      int i, j;
      uint8_t *src;

      src = srcY;
      for(j = 0; j < 9 + dFRM->mspel*2; j++) {
	for(i = 0; i < 9 + dFRM->mspel*2; i++) src[i] = ((src[i] - 128) >> 1) + 128;
	src += dFRM->linesize;
      }
    }
    /* if we deal with intensity compensation we need to scale source blocks */
    if(dMB->mv_mode == MV_PMODE_INTENSITY_COMP) {
      int i, j;
      uint8_t *src;

      src = srcY;
      for(j = 0; j < 9 + dFRM->mspel*2; j++) {
	for(i = 0; i < 9 + dFRM->mspel*2; i++) src[i] = luty[src[i]];
	src += dFRM->linesize;
      }
    }
    srcY += dFRM->mspel * (1 + dFRM->linesize);
  }

  if(dFRM->mspel) {
    dxy = ((my & 3) << 2) | (mx & 3);
    dsp->put_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0] + off, srcY, dFRM->linesize, dFRM->rnd);
  } else { // hpel mc - always used for luma
    dxy = (my & 2) | ((mx & 2) >> 1);
    if(!dFRM->rnd)
      dsp->put_pixels_tab[1][dxy](current_picture_ptr[0] + off, srcY, dFRM->linesize, 8);
    else
      dsp->put_no_rnd_pixels_tab[1][dxy](current_picture_ptr[0] + off, srcY, dFRM->linesize, 8);
  }
}
#endif

#ifdef JZC_MXU_OPT
static inline int mid_pred_mxu(int a, int b, int c)
{
  S32I2M(xr1,a);
  S32I2M(xr2,b);
  S32I2M(xr3,c);

  S32MIN(xr4,xr1,xr2);
  S32MAX(xr5,xr1,xr2);
  S32MAX(xr6,xr4,xr3);
  S32MIN(xr7,xr6,xr5);

  return S32M2I(xr7);
}
#define mid_pred mid_pred_mxu

static inline int median4(int a, int b, int c, int d)
{
  S32I2M(xr1,a);
  S32I2M(xr2,b);
  S32I2M(xr3,c);
  S32I2M(xr4,d);

  S32MAX(xr5,xr1,xr2);
  S32MAX(xr6,xr3,xr4);

  S32MIN(xr7,xr1,xr2);
  S32MIN(xr8,xr3,xr4);

  S32MIN(xr1,xr5,xr6);
  S32MAX(xr2,xr7,xr8);

  D32ADD_AA(xr3,xr1,xr2,xr4);
  
  return S32M2I(xr3)/2;
}
#else
static inline int median4(int a, int b, int c, int d)
{
  if(a < b) {
    if(c < d) return (FFMIN(b, d) + FFMAX(a, c)) / 2;
    else      return (FFMIN(b, c) + FFMAX(a, d)) / 2;
  } else {
    if(c < d) return (FFMIN(a, d) + FFMAX(b, c)) / 2;
    else      return (FFMIN(a, c) + FFMAX(b, d)) / 2;
  }
}
#endif
/** Do motion compensation for 4-MV macroblock - both chroma blocks
 */
#ifdef JZC_DCORE_OPTn
static void vc1_mc_4mv_chroma(VC1Context *v)
{
  MpegEncContext *s = &v->s;
  DSPContext *dsp = &v->s.dsp;
  uint8_t *srcU, *srcV;
  int uvmx, uvmy, uvsrc_x, uvsrc_y;
  int i, idx, tx = 0, ty = 0;
  int mvx[4], mvy[4], intra[4];
  static const int count[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

  if(!dFRM->last_data[0])return;
  if(dFRM->flags & CODEC_FLAG_GRAY) return;

  for(i = 0; i < 4; i++) {
    mvx[i] = dMB->vc1_mv[i][0];
    mvy[i] = dMB->vc1_mv[i][1];
    intra[i] = dMB->vc1_blk_index[i];
  }

  /* calculate chroma MV vector from four luma MVs */
  idx = (intra[3] << 3) | (intra[2] << 2) | (intra[1] << 1) | intra[0];
  if(!idx) { // all blocks are inter
    tx = median4(mvx[0], mvx[1], mvx[2], mvx[3]);
    ty = median4(mvy[0], mvy[1], mvy[2], mvy[3]);
  } else if(count[idx] == 1) { // 3 inter blocks
    switch(idx) {
    case 0x1:
      tx = mid_pred(mvx[1], mvx[2], mvx[3]);
      ty = mid_pred(mvy[1], mvy[2], mvy[3]);
      break;
    case 0x2:
      tx = mid_pred(mvx[0], mvx[2], mvx[3]);
      ty = mid_pred(mvy[0], mvy[2], mvy[3]);
      break;
    case 0x4:
      tx = mid_pred(mvx[0], mvx[1], mvx[3]);
      ty = mid_pred(mvy[0], mvy[1], mvy[3]);
      break;
    case 0x8:
      tx = mid_pred(mvx[0], mvx[1], mvx[2]);
      ty = mid_pred(mvy[0], mvy[1], mvy[2]);
      break;
    }
  } else if(count[idx] == 2) {
    int t1 = 0, t2 = 0;
    for(i=0; i<3;i++) if(!intra[i]) {t1 = i; break;}
    for(i= t1+1; i<4; i++)if(!intra[i]) {t2 = i; break;}
    tx = (mvx[t1] + mvx[t2]) / 2;
    ty = (mvy[t1] + mvy[t2]) / 2;
  } else {
    return; //no need to do MC for inter blocks
  }

  uvmx = (tx + ((tx&3) == 3)) >> 1;
  uvmy = (ty + ((ty&3) == 3)) >> 1;
  if(dFRM->fastuvmc) {
    uvmx = uvmx + ((uvmx<0)?(uvmx&1):-(uvmx&1));
    uvmy = uvmy + ((uvmy<0)?(uvmy&1):-(uvmy&1));
  }

  uvsrc_x = dMB->mb_x * 8 + (uvmx >> 2);
  uvsrc_y = dMB->mb_y * 8 + (uvmy >> 2);

  if(dFRM->profile != PROFILE_ADVANCED){
    uvsrc_x = av_clip(uvsrc_x,  -8, dFRM->mb_width  *  8);
    uvsrc_y = av_clip(uvsrc_y,  -8, dFRM->mb_height *  8);
  }else{
    uvsrc_x = av_clip(uvsrc_x,  -8, dFRM->coded_width  >> 1);
    uvsrc_y = av_clip(uvsrc_y,  -8, dFRM->coded_height >> 1);
  }

  srcU = dFRM->last_data[1] + uvsrc_y * dFRM->uvlinesize + uvsrc_x;
  srcV = dFRM->last_data[2] + uvsrc_y * dFRM->uvlinesize + uvsrc_x;
  if(dFRM->rangeredfrm || (dMB->mv_mode == MV_PMODE_INTENSITY_COMP)
     || (unsigned)uvsrc_x > (dFRM->h_edge_pos >> 1) - 9
     || (unsigned)uvsrc_y > (dFRM->v_edge_pos >> 1) - 9){
    ff_emulated_edge_mc(dFRM->edge_emu_buffer     , srcU, dFRM->uvlinesize, 8+1, 8+1,
			uvsrc_x, uvsrc_y, dFRM->h_edge_pos >> 1, dFRM->v_edge_pos >> 1);
    ff_emulated_edge_mc(dFRM->edge_emu_buffer + 16, srcV, dFRM->uvlinesize, 8+1, 8+1,
			uvsrc_x, uvsrc_y, dFRM->h_edge_pos >> 1, dFRM->v_edge_pos >> 1);
    srcU = dFRM->edge_emu_buffer;
    srcV = dFRM->edge_emu_buffer + 16;

    /* if we deal with range reduction we need to scale source blocks */
    if(dFRM->rangeredfrm) {
      int i, j;
      uint8_t *src, *src2;

      src = srcU; src2 = srcV;
      for(j = 0; j < 9; j++) {
	for(i = 0; i < 9; i++) {
	  src[i] = ((src[i] - 128) >> 1) + 128;
	  src2[i] = ((src2[i] - 128) >> 1) + 128;
	}
	src += dFRM->uvlinesize;
	src2 += dFRM->uvlinesize;
      }
    }
    /* if we deal with intensity compensation we need to scale source blocks */
    if(dMB->mv_mode == MV_PMODE_INTENSITY_COMP) {
      int i, j;
      uint8_t *src, *src2;

      src = srcU; src2 = srcV;
      for(j = 0; j < 9; j++) {
	for(i = 0; i < 9; i++) {
	  src[i] = lutuv[src[i]];
	  src2[i] = lutuv[src2[i]];
	}
	src += dFRM->uvlinesize;
	src2 += dFRM->uvlinesize;
      }
    }
  }

  /* Chroma MC always uses qpel bilinear */
  uvmx = (uvmx&3)<<1;
  uvmy = (uvmy&3)<<1;
  if(!dFRM->rnd){
    dsp->put_h264_chroma_pixels_tab[0](current_picture_ptr[1], srcU, dFRM->uvlinesize, 8, uvmx, uvmy);
    dsp->put_h264_chroma_pixels_tab[0](current_picture_ptr[2], srcV, dFRM->uvlinesize, 8, uvmx, uvmy);
  }else{
    dsp->put_no_rnd_vc1_chroma_pixels_tab[0](current_picture_ptr[1], srcU, dFRM->uvlinesize, 8, uvmx, uvmy);
    dsp->put_no_rnd_vc1_chroma_pixels_tab[0](current_picture_ptr[2], srcV, dFRM->uvlinesize, 8, uvmx, uvmy);
  }
}
#endif

#ifdef JZC_VLC_HW_OPT
#define get_bits get_bits_hw
#define get_bits1 get_bits1_hw
#define decode210 decode210_hw
#define decode012 decode012_hw
#define get_unary get_unary_hw
#define get_vlc2 get_vlc2_hw
#endif
/** Predict and set motion vector
 */
#ifdef JZC_MXU_OPT
void vc1_pred_mv(MpegEncContext *s, int n, int dmv_x, int dmv_y, int mv1, int r_x, int r_y, uint8_t* is_intra)
{
    int xy, wrap, off = 0;
    int16_t *A, *B, *C;
    int px,py;
    int sum;

    /* scale MV difference to be quad-pel */
    dmv_x <<= 1 - s->quarter_sample;
    dmv_y <<= 1 - s->quarter_sample;

    wrap = s->b8_stride;
    xy = s->block_index[n];

    int *cur_mv0=s->current_picture.motion_val[0][xy];
    int *cur_mv1=s->current_picture.motion_val[1][xy];
    if(s->mb_intra){
	S32STD(xr0,cur_mv0,0x0);
	S32STD(xr0,cur_mv1,0x0);
        if(mv1) { /* duplicate motion data for 1-MV block */
	  S32STD(xr0,cur_mv0,0x4);
	  S32STD(xr0,cur_mv1,0x4);
	  S32STD(xr0,cur_mv0+wrap,0x0);
	  S32STD(xr0,cur_mv0+wrap,0x4);
	  S32STD(xr0,cur_mv1+wrap,0x0);
	  S32STD(xr0,cur_mv1+wrap,0x4);
        }
        return;
    }

    C = cur_mv0 - 1;
    A = cur_mv0 - wrap;
    if(mv1){
        off = (s->mb_x == (s->mb_width - 1)) ? -1 : 2;	
    }
    else {
        //in 4-MV mode different blocks have different B predictor position
        switch(n){
        case 0:
            off = (s->mb_x > 0) ? -1 : 1;
            break;
        case 1:
            off = (s->mb_x == (s->mb_width - 1)) ? -1 : 1;
            break;
        case 2:
            off = 1;
            break;
        case 3:
            off = -1;
        }
    }
    B = cur_mv0 - wrap + off;

    if(!s->first_slice_line || (n==2 || n==3)) { // predictor A is not out of bounds
        if(s->mb_width == 1) {
	  S32LDD(xr14,A,0x0);
        } else {
	  S32LDD(xr14,A,0x0);
	  S32LDD(xr13,B,0x0);
	  S32LDD(xr12,C,0x0);
	  
	  D16MAX(xr4,xr14,xr13);//xr4=max of (A,B)
	  D16MIN(xr5,xr14,xr13);//xr5=min of (A,B)
	  D16MIN(xr4,xr4,xr12);//xr4:comp max(A,B) and C, get rid of the MAX of (A,B,C)
	  D16MAX(xr14,xr4,xr5);
        }
    } else if(s->mb_x || (n==1 || n==3)) { // predictor C is not out of bounds
      S32LDD(xr14,C,0x0);
    } else {
      S32I2M(xr14,0x0);
    }
    /* Pullback MV as specified in 8.3.5.3.4 */
    {
      int16_t qx, qy, X, Y, Z;      
      Z = mv1 ? -60 : -28;	
      int fg=0;	
	i_movn(fg,0x20,n==1);
	i_movn(fg,0x200000,n==2);
	i_movn(fg,0x200020,n==3);
	S32I2M(xr15,0x4);
	S32I2M(xr13,s->mb_x);
	S32I2M(xr12,s->mb_y);
	S32SFL(xr0,xr15,xr15,xr15,3);  //xr15:4,4
	S32SFL(xr0,xr12,xr13,xr13,3);  //xr13:{s->mb_y,s->mb_x}
	
	S32I2M(xr10,s->mb_width);
	S32I2M(xr9,s->mb_height);
	S32SFL(xr0,xr9,xr10,xr10,3);  //xr10:{s->mb_height,s->mb_width}
	Q16SLL(xr10,xr10,xr13,xr13,6);//xr13={s->mb_y<<6,s->mb_x<<6}, xr10 = ()

	S32I2M(xr11,fg);
	Q16ADD_AA_WW(xr13,xr13,xr11,xr0); //xr13:qy qx
	Q16ADD_SS_WW(xr12,xr10,xr15,xr0); //xr12:y   X
	
	S32I2M(xr11,Z);
	S32SFL(xr0,xr11,xr11,xr11,3); //xr11:z z
	Q16ADD_AA_WW(xr10,xr13,xr14,xr0); //xr10:
	D16SLT(xr9,xr10,xr11);            //xr9 :con1
	Q16ADD_SS_WW(xr8,xr11,xr13,xr0);  //xr8 :z-qy z-qx
	Q16ADD_SS_WW(xr7,xr12,xr13,xr0);  //xr7 :Y-qy X-qx
	D16MOVN(xr14,xr9,xr8);
	Q16ADD_AA_WW(xr10,xr13,xr14,xr0);
	D16SLT(xr6,xr12,xr10);
	D16MOVN(xr14,xr6,xr7);
    }
    /* Calculate hybrid prediction as specified in 8.3.5.3.5 */
    if((!s->first_slice_line || (n==2 || n==3)) && (s->mb_x || (n==1 || n==3))) { 
      if(is_intra[xy - wrap]){
	D16CPS(xr11,xr14,xr14);
	Q16ADD_AA_XW(xr10,xr11,xr11,xr0);
      }
      else{
	S32LDD(xr15,A,0x0);
	Q16ADD_SS_WW(xr13,xr14,xr15,xr0);
	D16CPS(xr11,xr13,xr13);
	Q16ADD_AA_XW(xr10,xr11,xr11,xr0);
      }
      
      sum = S32M2I(xr10) >> 16;
        if(sum > 32) {
            if(get_bits1(&s->gb)) {
	      S32LDD(xr14,A,0x0);
            } else {
		S32LDD(xr14,C,0x0);
            }
        } else {
	  if(is_intra[xy - 1]){
	    D16CPS(xr11,xr14,xr14);
	    Q16ADD_AA_XW(xr10,xr11,xr11,xr0);
	  }
	  else{
	    S32LDD(xr15,C,0x0);	    
	    Q16ADD_SS_WW(xr13,xr14,xr15,xr0);
	    D16CPS(xr11,xr13,xr13);
	    Q16ADD_AA_XW(xr10,xr11,xr11,xr0);
	  }
	  sum = S32M2I(xr10) >> 16;
	  if(sum > 32) {
                if(get_bits1(&s->gb)) {
		  S32LDD(xr14,A,0x0);
                } else {
		  S32LDD(xr14,C,0x0);
                }
            }
        }
    }
    px = S32M2I(xr14)<<16>>16;
    py = S32M2I(xr14) >> 16;
    /* store MV using signed modulus of MV range defined in 4.11 */
    dMB->vc1_mv[n][0] = ((short *)cur_mv0)[0] = ((px + dmv_x + r_x) & ((r_x << 1) - 1)) - r_x;
    dMB->vc1_mv[n][1] = ((short *)cur_mv0)[1] = ((py + dmv_y + r_y) & ((r_y << 1) - 1)) - r_y;

    if(mv1) { /* duplicate motion data for 1-MV block */
      S32LDD(xr14,cur_mv0,0x0);      
      S32STD(xr14,cur_mv0,0x4);
      S32STD(xr14,cur_mv0 + wrap,0x0);
      S32STD(xr14,cur_mv0 + wrap,0x4);
    }
}
#else
static inline void vc1_pred_mv(MpegEncContext *s, int n, int dmv_x, int dmv_y, int mv1, int r_x, int r_y, uint8_t* is_intra)
{
  int xy, wrap, off = 0;
  int16_t *A, *B, *C;
  int px, py;
  int sum;

  /* scale MV difference to be quad-pel */
  dmv_x <<= 1 - s->quarter_sample;
  dmv_y <<= 1 - s->quarter_sample;

  wrap = s->b8_stride;
  xy = s->block_index[n];

  if(s->mb_intra){
    s->mv[0][n][0] = s->current_picture.motion_val[0][xy][0] = 0;
    s->mv[0][n][1] = s->current_picture.motion_val[0][xy][1] = 0;
    s->current_picture.motion_val[1][xy][0] = 0;
    s->current_picture.motion_val[1][xy][1] = 0;
    if(mv1) { /* duplicate motion data for 1-MV block */
      s->current_picture.motion_val[0][xy + 1][0] = 0;
      s->current_picture.motion_val[0][xy + 1][1] = 0;
      s->current_picture.motion_val[0][xy + wrap][0] = 0;
      s->current_picture.motion_val[0][xy + wrap][1] = 0;
      s->current_picture.motion_val[0][xy + wrap + 1][0] = 0;
      s->current_picture.motion_val[0][xy + wrap + 1][1] = 0;
      s->current_picture.motion_val[1][xy + 1][0] = 0;
      s->current_picture.motion_val[1][xy + 1][1] = 0;
      s->current_picture.motion_val[1][xy + wrap][0] = 0;
      s->current_picture.motion_val[1][xy + wrap][1] = 0;
      s->current_picture.motion_val[1][xy + wrap + 1][0] = 0;
      s->current_picture.motion_val[1][xy + wrap + 1][1] = 0;
    }
    return;
  }

  C = s->current_picture.motion_val[0][xy - 1];
  A = s->current_picture.motion_val[0][xy - wrap];
  if(mv1)
    off = (s->mb_x == (s->mb_width - 1)) ? -1 : 2;
  else {
    //in 4-MV mode different blocks have different B predictor position
    switch(n){
    case 0:
      off = (s->mb_x > 0) ? -1 : 1;
      break;
    case 1:
      off = (s->mb_x == (s->mb_width - 1)) ? -1 : 1;
      break;
    case 2:
      off = 1;
      break;
    case 3:
      off = -1;
    }
  }
  B = s->current_picture.motion_val[0][xy - wrap + off];

  if(!s->first_slice_line || (n==2 || n==3)) { // predictor A is not out of bounds
    if(s->mb_width == 1) {
      px = A[0];
      py = A[1];
    } else {
      px = mid_pred(A[0], B[0], C[0]);
      py = mid_pred(A[1], B[1], C[1]);
    }
  } else if(s->mb_x || (n==1 || n==3)) { // predictor C is not out of bounds
    px = C[0];
    py = C[1];
  } else {
    px = py = 0;
  }
  /* Pullback MV as specified in 8.3.5.3.4 */
  {
    int qx, qy, X, Y;
    qx = (s->mb_x << 6) + ((n==1 || n==3) ? 32 : 0);
    qy = (s->mb_y << 6) + ((n==2 || n==3) ? 32 : 0);
    X = (s->mb_width << 6) - 4;
    Y = (s->mb_height << 6) - 4;
    if(mv1) {
      if(qx + px < -60) px = -60 - qx;
      if(qy + py < -60) py = -60 - qy;
    } else {
      if(qx + px < -28) px = -28 - qx;
      if(qy + py < -28) py = -28 - qy;
    }
    if(qx + px > X) px = X - qx;
    if(qy + py > Y) py = Y - qy;
  }
  /* Calculate hybrid prediction as specified in 8.3.5.3.5 */
  if((!s->first_slice_line || (n==2 || n==3)) && (s->mb_x || (n==1 || n==3))) {
    if(is_intra[xy - wrap])
      sum = FFABS(px) + FFABS(py);
    else
      sum = FFABS(px - A[0]) + FFABS(py - A[1]);
    if(sum > 32) {
      if(get_bits1(&s->gb)) {
	px = A[0];
	py = A[1];
      } else {
	px = C[0];
	py = C[1];
      }
    } else {
      if(is_intra[xy - 1])
	sum = FFABS(px) + FFABS(py);
      else
	sum = FFABS(px - C[0]) + FFABS(py - C[1]);
      if(sum > 32) {
	if(get_bits1(&s->gb)) {
	  px = A[0];
	  py = A[1];
	} else {
	  px = C[0];
	  py = C[1];
	}
      }
    }
  }
  /* store MV using signed modulus of MV range defined in 4.11 */
  s->mv[0][n][0] = s->current_picture.motion_val[0][xy][0] = ((px + dmv_x + r_x) & ((r_x << 1) - 1)) - r_x;
  s->mv[0][n][1] = s->current_picture.motion_val[0][xy][1] = ((py + dmv_y + r_y) & ((r_y << 1) - 1)) - r_y;
  if(mv1) { /* duplicate motion data for 1-MV block */
    s->current_picture.motion_val[0][xy + 1][0] = s->current_picture.motion_val[0][xy][0];
    s->current_picture.motion_val[0][xy + 1][1] = s->current_picture.motion_val[0][xy][1];
    s->current_picture.motion_val[0][xy + wrap][0] = s->current_picture.motion_val[0][xy][0];
    s->current_picture.motion_val[0][xy + wrap][1] = s->current_picture.motion_val[0][xy][1];
    s->current_picture.motion_val[0][xy + wrap + 1][0] = s->current_picture.motion_val[0][xy][0];
    s->current_picture.motion_val[0][xy + wrap + 1][1] = s->current_picture.motion_val[0][xy][1];
  }
}
#endif
/** Motion compensation for direct or interpolated blocks in B-frames
 */
#ifdef JZC_DCORE_OPTn
static void vc1_interp_mc(VC1Context *v)
{
  MpegEncContext *s = &v->s;
  DSPContext *dsp = &v->s.dsp;
  uint8_t *srcY, *srcU, *srcV;
  int dxy, mx, my, uvmx, uvmy, src_x, src_y, uvsrc_x, uvsrc_y;

  if(!dFRM->next_data[0])return;
  mx = dMB->vc1_mv[1][0];
  my = dMB->vc1_mv[1][1];

  uvmx = (mx + ((mx & 3) == 3)) >> 1;
  uvmy = (my + ((my & 3) == 3)) >> 1;
  if(dFRM->fastuvmc) {
    uvmx = uvmx + ((uvmx<0)?-(uvmx&1):(uvmx&1));
    uvmy = uvmy + ((uvmy<0)?-(uvmy&1):(uvmy&1));
  }

  srcY = dFRM->next_data[0];
  srcU = dFRM->next_data[1];
  srcV = dFRM->next_data[2];

  src_x = dMB->mb_x * 16 + (mx >> 2);
  src_y = dMB->mb_y * 16 + (my >> 2);
  uvsrc_x = dMB->mb_x * 8 + (uvmx >> 2);
  uvsrc_y = dMB->mb_y * 8 + (uvmy >> 2);

  if(dFRM->profile != PROFILE_ADVANCED){
    src_x   = av_clip(  src_x, -16, dFRM->mb_width  * 16);
    src_y   = av_clip(  src_y, -16, dFRM->mb_height * 16);
    uvsrc_x = av_clip(uvsrc_x,  -8, dFRM->mb_width  *  8);
    uvsrc_y = av_clip(uvsrc_y,  -8, dFRM->mb_height *  8);
  }else{
    src_x   = av_clip(  src_x, -17, dFRM->coded_width);
    src_y   = av_clip(  src_y, -18, dFRM->coded_height + 1);
    uvsrc_x = av_clip(uvsrc_x,  -8, dFRM->coded_width  >> 1);
    uvsrc_y = av_clip(uvsrc_y,  -8, dFRM->coded_height >> 1);
  }

  srcY += src_y * dFRM->linesize + src_x;
  srcU += uvsrc_y * dFRM->uvlinesize + uvsrc_x;
  srcV += uvsrc_y * dFRM->uvlinesize + uvsrc_x;

  /* for grayscale we should not try to read from unknown area */
  if(dFRM->flags & CODEC_FLAG_GRAY) {
    srcU = dFRM->edge_emu_buffer + 18 * dFRM->linesize;
    srcV = dFRM->edge_emu_buffer + 18 * dFRM->linesize;
  }

  if(dFRM->rangeredfrm
     || (unsigned)(src_x -dFRM->mspel)> dFRM->h_edge_pos - (mx&3) - 16 - dFRM->mspel*3
     || (unsigned)(src_y - dFRM->mspel)> dFRM->v_edge_pos - (my&3) - 16 - dFRM->mspel*3){
    uint8_t *uvbuf= dFRM->edge_emu_buffer + 19 * dFRM->linesize;

    srcY -= dFRM->mspel * (1 + dFRM->linesize);
    ff_emulated_edge_mc(dFRM->edge_emu_buffer, srcY, dFRM->linesize, 17+dFRM->mspel*2, 17+dFRM->mspel*2,
			src_x - dFRM->mspel, src_y - dFRM->mspel, dFRM->h_edge_pos, dFRM->v_edge_pos);
    srcY = dFRM->edge_emu_buffer;
    ff_emulated_edge_mc(uvbuf     , srcU, dFRM->uvlinesize, 8+1, 8+1,
			uvsrc_x, uvsrc_y, dFRM->h_edge_pos >> 1, dFRM->v_edge_pos >> 1);
    ff_emulated_edge_mc(uvbuf + 16, srcV, dFRM->uvlinesize, 8+1, 8+1,
			uvsrc_x, uvsrc_y, dFRM->h_edge_pos >> 1, dFRM->v_edge_pos >> 1);
    srcU = uvbuf;
    srcV = uvbuf + 16;
    /* if we deal with range reduction we need to scale source blocks */
    if(dFRM->rangeredfrm) {
      int i, j;
      uint8_t *src, *src2;

      src = srcY;
      for(j = 0; j < 17 + dFRM->mspel*2; j++) {
	for(i = 0; i < 17 + dFRM->mspel*2; i++) src[i] = ((src[i] - 128) >> 1) + 128;
	src += dFRM->linesize;
      }
      src = srcU; src2 = srcV;
      for(j = 0; j < 9; j++) {
	for(i = 0; i < 9; i++) {
	  src[i] = ((src[i] - 128) >> 1) + 128;
	  src2[i] = ((src2[i] - 128) >> 1) + 128;
	}
	src += dFRM->uvlinesize;
	src2 += dFRM->uvlinesize;
      }
    }
    srcY += dFRM->mspel * (1 + dFRM->linesize);
  }

  if(dFRM->mspel) {
    dxy = ((my & 3) << 2) | (mx & 3);
    dsp->avg_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0]    , srcY    , dFRM->linesize, v->rnd);
    dsp->avg_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0] + 8, srcY + 8, dFRM->linesize, v->rnd);
    srcY += dFRM->linesize * 8;
    dsp->avg_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0] + 8 * dFRM->linesize    , srcY    , dFRM->linesize, v->rnd);
    dsp->avg_vc1_mspel_pixels_tab[dxy](current_picture_ptr[0] + 8 * dFRM->linesize + 8, srcY + 8, dFRM->linesize, v->rnd);
  } else { // hpel mc
    dxy = (my & 2) | ((mx & 2) >> 1);

    if(!v->rnd)
      dsp->avg_pixels_tab[0][dxy](s->dest[0], srcY, dFRM->linesize, 16);
    else
      dsp->avg_no_rnd_pixels_tab[0][dxy](s->dest[0], srcY, dFRM->linesize, 16);
  }

  if(dFRM->flags & CODEC_FLAG_GRAY) return;
  /* Chroma MC always uses qpel blilinear */
  uvmx = (uvmx&3)<<1;
  uvmy = (uvmy&3)<<1;
  if(!v->rnd){
    dsp->avg_h264_chroma_pixels_tab[0](current_picture_ptr[1], srcU, dFRM->uvlinesize, 8, uvmx, uvmy);
    dsp->avg_h264_chroma_pixels_tab[0](current_picture_ptr[2], srcV, dFRM->uvlinesize, 8, uvmx, uvmy);
  }else{
    dsp->avg_no_rnd_vc1_chroma_pixels_tab[0](current_picture_ptr[1], srcU, dFRM->uvlinesize, 8, uvmx, uvmy);
    dsp->avg_no_rnd_vc1_chroma_pixels_tab[0](current_picture_ptr[2], srcV, dFRM->uvlinesize, 8, uvmx, uvmy);
  }
}
#endif

static av_always_inline int scale_mv(int value, int bfrac, int inv, int qs)
{
  int n = bfrac;

#if B_FRACTION_DEN==256
  if(inv)
    n -= 256;
  if(!qs)
    return 2 * ((value * n + 255) >> 9);
  return (value * n + 128) >> 8;
#else
  if(inv)
    n -= B_FRACTION_DEN;
  if(!qs)
    return 2 * ((value * n + B_FRACTION_DEN - 1) / (2 * B_FRACTION_DEN));
  return (value * n + B_FRACTION_DEN/2) / B_FRACTION_DEN;
#endif
}

static inline void vc1_pred_b_mv(VC1Context *v, int dmv_x[2], int dmv_y[2], int direct, int mvtype)
{
  MpegEncContext *s = &v->s;
  int xy, wrap, off = 0;
  int16_t *A, *B, *C;
  int px, py;
  int sum;
  int r_x, r_y;
  const uint8_t *is_intra = v->mb_type[0];

  r_x = v->range_x;
  r_y = v->range_y;
  /* scale MV difference to be quad-pel */
  dmv_x[0] <<= 1 - s->quarter_sample;
  dmv_y[0] <<= 1 - s->quarter_sample;
  dmv_x[1] <<= 1 - s->quarter_sample;
  dmv_y[1] <<= 1 - s->quarter_sample;

  wrap = s->b8_stride;
  xy = s->block_index[0];

  if(s->mb_intra) {
    s->current_picture.motion_val[0][xy][0] =
      s->current_picture.motion_val[0][xy][1] =
      s->current_picture.motion_val[1][xy][0] =
      s->current_picture.motion_val[1][xy][1] = 0;
    return;
  }
  s->mv[0][0][0] = scale_mv(s->next_picture.motion_val[1][xy][0], v->bfraction, 0, s->quarter_sample);
  s->mv[0][0][1] = scale_mv(s->next_picture.motion_val[1][xy][1], v->bfraction, 0, s->quarter_sample);
  s->mv[1][0][0] = scale_mv(s->next_picture.motion_val[1][xy][0], v->bfraction, 1, s->quarter_sample);
  s->mv[1][0][1] = scale_mv(s->next_picture.motion_val[1][xy][1], v->bfraction, 1, s->quarter_sample);

  /* Pullback predicted motion vectors as specified in 8.4.5.4 */
  s->mv[0][0][0] = av_clip(s->mv[0][0][0], -60 - (s->mb_x << 6), (s->mb_width  << 6) - 4 - (s->mb_x << 6));
  s->mv[0][0][1] = av_clip(s->mv[0][0][1], -60 - (s->mb_y << 6), (s->mb_height << 6) - 4 - (s->mb_y << 6));
  s->mv[1][0][0] = av_clip(s->mv[1][0][0], -60 - (s->mb_x << 6), (s->mb_width  << 6) - 4 - (s->mb_x << 6));
  s->mv[1][0][1] = av_clip(s->mv[1][0][1], -60 - (s->mb_y << 6), (s->mb_height << 6) - 4 - (s->mb_y << 6));
  if(direct) {
    s->current_picture.motion_val[0][xy][0] = s->mv[0][0][0];
    s->current_picture.motion_val[0][xy][1] = s->mv[0][0][1];
    s->current_picture.motion_val[1][xy][0] = s->mv[1][0][0];
    s->current_picture.motion_val[1][xy][1] = s->mv[1][0][1];
    return;
  }

  if((mvtype == BMV_TYPE_FORWARD) || (mvtype == BMV_TYPE_INTERPOLATED)) {
    C = s->current_picture.motion_val[0][xy - 2];
    A = s->current_picture.motion_val[0][xy - wrap*2];
    off = (s->mb_x == (s->mb_width - 1)) ? -2 : 2;
    B = s->current_picture.motion_val[0][xy - wrap*2 + off];

    if(!s->mb_x) C[0] = C[1] = 0;
    if(!s->first_slice_line) { // predictor A is not out of bounds
      if(s->mb_width == 1) {
	px = A[0];
	py = A[1];
      } else {
	px = mid_pred(A[0], B[0], C[0]);
	py = mid_pred(A[1], B[1], C[1]);
      }
    } else if(s->mb_x) { // predictor C is not out of bounds
      px = C[0];
      py = C[1];
    } else {
      px = py = 0;
    }
    /* Pullback MV as specified in 8.3.5.3.4 */
    {
      int qx, qy, X, Y;
      if(v->profile < PROFILE_ADVANCED) {
	qx = (s->mb_x << 5);
	qy = (s->mb_y << 5);
	X = (s->mb_width << 5) - 4;
	Y = (s->mb_height << 5) - 4;
	if(qx + px < -28) px = -28 - qx;
	if(qy + py < -28) py = -28 - qy;
	if(qx + px > X) px = X - qx;
	if(qy + py > Y) py = Y - qy;
      } else {
	qx = (s->mb_x << 6);
	qy = (s->mb_y << 6);
	X = (s->mb_width << 6) - 4;
	Y = (s->mb_height << 6) - 4;
	if(qx + px < -60) px = -60 - qx;
	if(qy + py < -60) py = -60 - qy;
	if(qx + px > X) px = X - qx;
	if(qy + py > Y) py = Y - qy;
      }
    }
    /* Calculate hybrid prediction as specified in 8.3.5.3.5 */
    if(0 && !s->first_slice_line && s->mb_x) {
      if(is_intra[xy - wrap])
	sum = FFABS(px) + FFABS(py);
      else
	sum = FFABS(px - A[0]) + FFABS(py - A[1]);
      if(sum > 32) {
	if(get_bits1(&s->gb)) {
	  px = A[0];
	  py = A[1];
	} else {
	  px = C[0];
	  py = C[1];
	}
      } else {
	if(is_intra[xy - 2])
	  sum = FFABS(px) + FFABS(py);
	else
	  sum = FFABS(px - C[0]) + FFABS(py - C[1]);
	if(sum > 32) {
	  if(get_bits1(&s->gb)) {
	    px = A[0];
	    py = A[1];
	  } else {
	    px = C[0];
	    py = C[1];
	  }
	}
      }
    }
    /* store MV using signed modulus of MV range defined in 4.11 */
    s->mv[0][0][0] = ((px + dmv_x[0] + r_x) & ((r_x << 1) - 1)) - r_x;
    s->mv[0][0][1] = ((py + dmv_y[0] + r_y) & ((r_y << 1) - 1)) - r_y;
  }
  if((mvtype == BMV_TYPE_BACKWARD) || (mvtype == BMV_TYPE_INTERPOLATED)) {
    C = s->current_picture.motion_val[1][xy - 2];
    A = s->current_picture.motion_val[1][xy - wrap*2];
    off = (s->mb_x == (s->mb_width - 1)) ? -2 : 2;
    B = s->current_picture.motion_val[1][xy - wrap*2 + off];

    if(!s->mb_x) C[0] = C[1] = 0;
    if(!s->first_slice_line) { // predictor A is not out of bounds
      if(s->mb_width == 1) {
	px = A[0];
	py = A[1];
      } else {
	px = mid_pred(A[0], B[0], C[0]);
	py = mid_pred(A[1], B[1], C[1]);
      }
    } else if(s->mb_x) { // predictor C is not out of bounds
      px = C[0];
      py = C[1];
    } else {
      px = py = 0;
    }
    /* Pullback MV as specified in 8.3.5.3.4 */
    {
      int qx, qy, X, Y;
      if(v->profile < PROFILE_ADVANCED) {
	qx = (s->mb_x << 5);
	qy = (s->mb_y << 5);
	X = (s->mb_width << 5) - 4;
	Y = (s->mb_height << 5) - 4;
	if(qx + px < -28) px = -28 - qx;
	if(qy + py < -28) py = -28 - qy;
	if(qx + px > X) px = X - qx;
	if(qy + py > Y) py = Y - qy;
      } else {
	qx = (s->mb_x << 6);
	qy = (s->mb_y << 6);
	X = (s->mb_width << 6) - 4;
	Y = (s->mb_height << 6) - 4;
	if(qx + px < -60) px = -60 - qx;
	if(qy + py < -60) py = -60 - qy;
	if(qx + px > X) px = X - qx;
	if(qy + py > Y) py = Y - qy;
      }
    }
    /* Calculate hybrid prediction as specified in 8.3.5.3.5 */
    if(0 && !s->first_slice_line && s->mb_x) {
      if(is_intra[xy - wrap])
	sum = FFABS(px) + FFABS(py);
      else
	sum = FFABS(px - A[0]) + FFABS(py - A[1]);
      if(sum > 32) {
	if(get_bits1(&s->gb)) {
	  px = A[0];
	  py = A[1];
	} else {
	  px = C[0];
	  py = C[1];
	}
      } else {
	if(is_intra[xy - 2])
	  sum = FFABS(px) + FFABS(py);
	else
	  sum = FFABS(px - C[0]) + FFABS(py - C[1]);
	if(sum > 32) {
	  if(get_bits1(&s->gb)) {
	    px = A[0];
	    py = A[1];
	  } else {
	    px = C[0];
	    py = C[1];
	  }
	}
      }
    }
    /* store MV using signed modulus of MV range defined in 4.11 */

    s->mv[1][0][0] = ((px + dmv_x[1] + r_x) & ((r_x << 1) - 1)) - r_x;
    s->mv[1][0][1] = ((py + dmv_y[1] + r_y) & ((r_y << 1) - 1)) - r_y;
  }
  s->current_picture.motion_val[0][xy][0] = s->mv[0][0][0];
  s->current_picture.motion_val[0][xy][1] = s->mv[0][0][1];
  s->current_picture.motion_val[1][xy][0] = s->mv[1][0][0];
  s->current_picture.motion_val[1][xy][1] = s->mv[1][0][1];
}

/** Get predicted DC value for I-frames only
 * prediction dir: left=0, top=1
 * @param s MpegEncContext
 * @param overlap flag indicating that overlap filtering is used
 * @param pq integer part of picture quantizer
 * @param[in] n block index in the current MB
 * @param dc_val_ptr Pointer to DC predictor
 * @param dir_ptr Prediction direction for use in AC prediction
 */
#ifdef JZC_MXU_OPT
static inline int vc1_i_pred_dc(MpegEncContext *s, int overlap, int pq, int n,
                              int16_t **dc_val_ptr, int *dir_ptr)
{
    int wrap, pred, scale;
    int16_t *dc_val;
    static const uint16_t dcpred[32] = {
    -1, 1024,  512,  341,  256,  205,  171,  146,  128,
         114,  102,   93,   85,   79,   73,   68,   64,
          60,   57,   54,   51,   49,   47,   45,   43,
          41,   39,   38,   37,   35,   34,   33
    };

    /* find prediction - wmv3_dc_scale always used here in fact */
    if (n < 4)     scale = s->y_dc_scale;
    else           scale = s->c_dc_scale;

    wrap = s->block_wrap[n];
    dc_val= s->dc_val[0] + s->block_index[n];

    /* B A
     * C X
     */
    S16LDD(xr3,dc_val,-2,2);
    S16LDD(xr1,&dc_val[-wrap],0,2);
    S16LDD(xr2,&dc_val[-wrap-1],0,2);

    if (s->first_slice_line)
      if(!(n&2)){
	S32I2M(xr1,(pq < 9 || !overlap)?dcpred[scale]:0);
	S32MOVZ(xr2,xr0,xr1);
      }
    if (s->mb_x == 0)
      if(n!=1 && n!=3){
	S32I2M(xr2,(pq < 9 || !overlap)?dcpred[scale]:0);
	S32MOVZ(xr3,xr0,xr2);
      }

    D32ADD_SS(xr4,xr1,xr2,xr0);
    D32ADD_SS(xr5,xr3,xr2,xr0);
    S32CPS(xr4,xr4,xr4);
    S32CPS(xr5,xr5,xr5);
    S32SLT(xr5,xr5,xr4);
    S32MOVN(xr3,xr5,xr1);
    pred=S32M2I(xr3);
    *dir_ptr=S32M2I(xr5)?0:1;

    /* update predictor */
    *dc_val_ptr = &dc_val[0];
    return pred;
}
#else
static inline int vc1_i_pred_dc(MpegEncContext *s, int overlap, int pq, int n,
				int16_t **dc_val_ptr, int *dir_ptr)
{
  int a, b, c, wrap, pred, scale;
  int16_t *dc_val;
  static const uint16_t dcpred[32] = {
    -1, 1024,  512,  341,  256,  205,  171,  146,  128,
    114,  102,   93,   85,   79,   73,   68,   64,
    60,   57,   54,   51,   49,   47,   45,   43,
    41,   39,   38,   37,   35,   34,   33
  };

  /* find prediction - wmv3_dc_scale always used here in fact */
  if (n < 4)     scale = s->y_dc_scale;
  else           scale = s->c_dc_scale;

  wrap = s->block_wrap[n];
  dc_val= s->dc_val[0] + s->block_index[n];

  /* B A
   * C X
   */
  c = dc_val[ - 1];
  b = dc_val[ - 1 - wrap];
  a = dc_val[ - wrap];

  if (pq < 9 || !overlap)
    {
      /* Set outer values */
      if (s->first_slice_line && (n!=2 && n!=3)) b=a=dcpred[scale];
      if (s->mb_x == 0 && (n!=1 && n!=3)) b=c=dcpred[scale];
    }
  else
    {
      /* Set outer values */
      if (s->first_slice_line && (n!=2 && n!=3)) b=a=0;
      if (s->mb_x == 0 && (n!=1 && n!=3)) b=c=0;
    }

  if (abs(a - b) <= abs(b - c)) {
    pred = c;
    *dir_ptr = 1;//left
  } else {
    pred = a;
    *dir_ptr = 0;//top
  }

  /* update predictor */
  *dc_val_ptr = &dc_val[0];
  return pred;
}
#endif

/** Get predicted DC value
 * prediction dir: left=0, top=1
 * @param s MpegEncContext
 * @param overlap flag indicating that overlap filtering is used
 * @param pq integer part of picture quantizer
 * @param[in] n block index in the current MB
 * @param a_avail flag indicating top block availability
 * @param c_avail flag indicating left block availability
 * @param dc_val_ptr Pointer to DC predictor
 * @param dir_ptr Prediction direction for use in AC prediction
 */
static inline int vc1_pred_dc(MpegEncContext *s, int overlap, int pq, int n,
                              int a_avail, int c_avail,
                              int16_t **dc_val_ptr, int *dir_ptr)
{
  int a, b, c, wrap, pred;
  int16_t *dc_val;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  int q1, q2 = 0;

  wrap = s->block_wrap[n];
  dc_val= s->dc_val[0] + s->block_index[n];

  /* B A
   * C X
   */
  c = dc_val[ - 1];
  b = dc_val[ - 1 - wrap];
  a = dc_val[ - wrap];
  /* scale predictors if needed */
  q1 = s->current_picture.qscale_table[mb_pos];
  if(c_avail && (n!= 1 && n!=3)) {
    q2 = s->current_picture.qscale_table[mb_pos - 1];
    if(q2 && q2 != q1)
      c = (c * s->y_dc_scale_table[q2] * ff_vc1_dqscale[s->y_dc_scale_table[q1] - 1] + 0x20000) >> 18;
  }
  if(a_avail && (n!= 2 && n!=3)) {
    q2 = s->current_picture.qscale_table[mb_pos - s->mb_stride];
    if(q2 && q2 != q1)
      a = (a * s->y_dc_scale_table[q2] * ff_vc1_dqscale[s->y_dc_scale_table[q1] - 1] + 0x20000) >> 18;
  }
  if(a_avail && c_avail && (n!=3)) {
    int off = mb_pos;
    if(n != 1) off--;
    if(n != 2) off -= s->mb_stride;
    q2 = s->current_picture.qscale_table[off];
    if(q2 && q2 != q1)
      b = (b * s->y_dc_scale_table[q2] * ff_vc1_dqscale[s->y_dc_scale_table[q1] - 1] + 0x20000) >> 18;
  }

  if(a_avail && c_avail) {
    if(abs(a - b) <= abs(b - c)) {
      pred = c;
      *dir_ptr = 1;//left
    } else {
      pred = a;
      *dir_ptr = 0;//top
    }
  } else if(a_avail) {
    pred = a;
    *dir_ptr = 0;//top
  } else if(c_avail) {
    pred = c;
    *dir_ptr = 1;//left
  } else {
    pred = 0;
    *dir_ptr = 1;//left
  }

  /* update predictor */
  *dc_val_ptr = &dc_val[0];
  return pred;
}

/** @} */ // Block group

/**
 * @defgroup vc1_std_mb VC1 Macroblock-level functions in Simple/Main Profiles
 * @see 7.1.4, p91 and 8.1.1.7, p(1)04
 * @{
 */
static inline int vc1_coded_block_pred(MpegEncContext * s, int n, uint8_t **coded_block_ptr)
{
  int xy, wrap, pred, a, b, c;

  xy = s->block_index[n];
  wrap = s->b8_stride;

  /* B C
   * A X
   */
  a = s->coded_block[xy - 1       ];
  b = s->coded_block[xy - 1 - wrap];
  c = s->coded_block[xy     - wrap];

  if (b == c) {
    pred = a;
  } else {
    pred = c;
  }

  /* store value */
  *coded_block_ptr = &s->coded_block[xy];

  return pred;
}

#ifdef JZC_VLC_HW_OPT
uint32_t residual_init_table[8][7];
static inline void vc1_decode_residual_i(VC1Context *v, DCTELEM block[64], const uint8_t *zz_table, int codingset)
{
  GetBitContext *gb = &v->s.gb;
  int index, escape, last,code_set;
  int skip, value, idx, bits;
  int i=1;
  int mq = (v->pq < 8 || v->dquantfrm);
  uint32_t *init_data=residual_init_table[codingset];
  code_set = init_data[0];
  int8_t (*p)[2] = init_data[1];
  uint8_t (*last_delta) = init_data[2];
  uint8_t (*delta) = init_data[3];    
  uint8_t (*last_run) = init_data[4];
  uint8_t (*run) = init_data[5];
  //int vc1_ac_size=init_data[6];
  int vlc_ctrl=init_data[6];
  CPU_SET_CTRL(vlc_ctrl);
  index = CPU_GET_RSLT();

  skip = 0;
  value = 0;
  index = CPU_GET_RSLT();
  while (1){
    if (likely(!(index&0x0400))) {
      //SET_GET_BITS(1);
      skip = p[index][0];
      last = index >= code_set;
      value = p[index][1];
      //bits = CPU_GET_RSLT();
      //i_movn(value, -value, bits);
    } else {
      escape = (index&1) ? 0 : (2 - get_bits(gb,1));
      if (likely(escape != 2)) {
	CPU_SET_CTRL(vlc_ctrl);
	index = CPU_GET_RSLT();
	index = CPU_GET_RSLT();
	bits=index&1;
	index&=0xfffffbfe;
	skip = p[index][0];
	value = p[index][1];
	last = index >= code_set;
	if(escape == 0) {
	  uint8_t del = last ? last_delta[skip] : delta[skip];
	  value += del;
	} else {
	  uint8_t ru = last ? (last_run[value] + 1):(run[value] + 1);
	  skip += ru;
	}
	//bits = get_bits1(gb);
	i_movn(value, -value, bits);
      } else {	  
	int sign,lev_len1;
	int lev_len = v->s.esc3_level_length;
	int run_len = v->s.esc3_run_length ;
	last = get_bits(gb,1);
	if(lev_len == 0) {
	  if(mq){
	    lev_len = get_bits(gb, 3);
	    if(!lev_len)
	      lev_len = get_bits(gb, 2) + 8;
	  } else { //table 60
	    lev_len = get_unary(gb, 1, 6) + 2;
	  }
	  run_len = 3 + get_bits(gb, 2);
	}
	skip = get_bits(gb, run_len);
	sign = get_bits(gb, 1);
	value = get_bits(gb, lev_len);
	i_movn(value, -value, sign);
	v->s.esc3_level_length = lev_len;
	v->s.esc3_run_length = run_len;
      }
    }

    i += skip;
    if(i > 63||last)
      break;
    CPU_SET_CTRL(vlc_ctrl);
    index = CPU_GET_RSLT();
    idx = zz_table[i++];
    index = CPU_GET_RSLT();
    S32I2M(xr10,idx);
    block[idx] = value;
    S32MAX(xr11, xr11, xr10);
 }
  if(i<=63){
    idx = zz_table[i++];
    S32I2M(xr10,idx);
    S32MAX(xr11, xr11, xr10);

    block[idx] = value;
  }
}

static inline void vc1_decode_residual_p(VC1Context *v, DCTELEM *block, int numb, int scale, int mquant, const uint8_t *zz_table)
{
  GetBitContext *gb = &v->s.gb;
  int index, escape, last,code_set;
  int skip, value, idx, bits;
  int i=0;
  int mq = (v->pq < 8 || v->dquantfrm);
  uint32_t *init_data=residual_init_table[vc1_hw_codingset2];
  code_set = init_data[0];
  int8_t (*p)[2] = init_data[1];
  uint8_t (*last_delta) = init_data[2];
  uint8_t (*delta) = init_data[3];    
  uint8_t (*last_run) = init_data[4];
  uint8_t (*run) = init_data[5];
  //int vc1_ac_size=init_data[6];
  int vlc_ctrl=init_data[6];
  CPU_SET_CTRL(vlc_ctrl);
  index = CPU_GET_RSLT();

  skip = 0;
  value = 0;
  index = CPU_GET_RSLT();
  while (1){
    if (likely(!(index&0x0400))) {
      //SET_GET_BITS(1);
      skip = p[index][0];
      last = index >= code_set;
      value = p[index][1];
      //bits = CPU_GET_RSLT();
      //i_movn(value, -value, bits);
    } else {
      escape = (index&1) ? 0 : (2 - get_bits(gb,1));
      if (likely(escape != 2)) {
	CPU_SET_CTRL(vlc_ctrl);
	index = CPU_GET_RSLT();
	index = CPU_GET_RSLT();
	bits=index&1;
	index&=0xfffffbfe;
	skip = p[index][0];
	value = p[index][1];
	last = index >= code_set;
	if(escape == 0) {
	  uint8_t del = last ? last_delta[skip] : delta[skip];	    
	  value += del;
	} else {
	  uint8_t ru = last ? (last_run[value] + 1):(run[value] + 1);
	  skip += ru;
	}
	//bits = get_bits1(gb);
	i_movn(value, -value, bits);
      } else {
	int sign,lev_len1;
	int lev_len = v->s.esc3_level_length;
	int run_len = v->s.esc3_run_length ;
	last = get_bits(gb,1);
	if(lev_len == 0) {
	  if(mq){
	    lev_len = get_bits(gb, 3);
	    if(!lev_len)
	      lev_len = get_bits(gb, 2) + 8;
	  } else { //table 60
	    lev_len = get_unary(gb, 1, 6) + 2;
	  }
	  run_len = 3 + get_bits(gb, 2);
	}
	skip = get_bits(gb, run_len);
	sign = get_bits(gb, 1);
	value = get_bits(gb, lev_len);
	i_movn(value, -value, sign);
	v->s.esc3_level_length = lev_len;
	v->s.esc3_run_length = run_len;
      }
    }

    i += skip;
    if(i > numb||last)
      break;
    CPU_SET_CTRL(vlc_ctrl);
    idx = zz_table[i++];
    index = CPU_GET_RSLT();
    S32I2M(xr10,idx);
    S32MAX(xr11, xr11, xr10);
    block[idx] = value * scale;
    index = CPU_GET_RSLT();
    if(!v->pquantizer)
      block[idx] += (block[idx] < 0) ? -mquant : mquant;
  }
  if(i<=numb){
    idx = zz_table[i++];
    S32I2M(xr10,idx);
    S32MAX(xr11, xr11, xr10);
    block[idx] = value * scale;
    if(!v->pquantizer)
      block[idx] += (block[idx] < 0) ? -mquant : mquant;
  }
}
#else
void vc1_decode_residual(VC1Context *v, DCTELEM block[64], int numb, int scale, int mquant, int last, int off, const int8_t *zz_table, int inter, int codingset)
{
  GetBitContext *gb = &v->s.gb;
  int index, escape, lst,code_set;
  int skip, value, idx, bits;
  int i=(inter) ? 0 : 1;
  int mq = (v->pq < 8 || v->dquantfrm);
  code_set = vc1_last_decode_table[codingset];
  uint8_t (*p)[2] = vc1_index_decode_table[codingset];
  uint8_t (*last_delta) = vc1_last_delta_level_table[codingset];
  uint8_t (*delta) = vc1_delta_level_table[codingset];
    
  uint8_t (*last_run) = vc1_last_delta_run_table[codingset];
  uint8_t (*run) = vc1_delta_run_table[codingset];

  skip = 0;
  value = 0;
  lst = 0;
  while (!last){
    index = get_vlc2(gb, ff_vc1_ac_coeff_table[codingset].table,AC_VLC_BITS, 3);
    if (index != vc1_ac_sizes[codingset] - 1) {
      skip = p[index][0];
      value = p[index][1];
      lst = index >= code_set;
      bits = get_bits(gb,1);
      i_movn(value, -value, bits);
    } else {
      escape = (get_bits(gb,1)) ? 0 : (2 - get_bits(gb,1));
      if (escape != 2) {
	index = get_vlc2(gb, ff_vc1_ac_coeff_table[codingset].table, AC_VLC_BITS, 3);
	skip = p[index][0];
	value = p[index][1];
	lst = index >= code_set;
	if(escape == 0) {
	  uint8_t del = lst ? last_delta[skip] : delta[skip];	    
	  value += del;
	} else {
	  uint8_t ru = lst ? (last_run[value] + 1):(run[value] + 1);
	  skip += ru;
	}
	bits = get_bits1(gb);
	i_movn(value, -value, bits);
      } else {	  
	int sign,lev_len1;
	int lev_len = v->s.esc3_level_length;
	int run_len = v->s.esc3_run_length ;
	lst = get_bits(gb,1);
	if(lev_len == 0) {
	  if(mq){
	    lev_len = get_bits(gb, 3);
	    if(!lev_len)
	      lev_len = get_bits(gb, 2) + 8;
	  } else { //table 60
	    lev_len = get_unary(gb, 1, 6) + 2;
	  }
	  run_len = 3 + get_bits(gb, 2);
	}
	skip = get_bits(gb, run_len);
	sign = get_bits(gb, 1);
	value = get_bits(gb, lev_len);
	i_movn(value, -value, sign);
	v->s.esc3_level_length = lev_len;
	v->s.esc3_run_length = run_len;
      }
    }

    last = lst;
       
    i += skip;
    if(i > numb)
      break;
    idx = zz_table[i++];
    S32I2M(xr10,idx);
    S32MAX(xr11, xr11, xr10);

    if (inter){
      block[idx+off] = value * scale;
      if(!v->pquantizer)
	block[idx+off] += (block[idx+off] < 0) ? -mquant : mquant;
    }else{
      block[idx] = value;
    }
  }
}
#endif
/** Decode intra block in intra frames - should be faster than decode_intra_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock index
 * @param coded are AC coeffs present or not
 * @param codingset set of VLC to decode data
 */
int16_t *left_buf;
static inline int vc1_decode_i_block(VC1Context *v, DCTELEM block[64], int n, int coded, int codingset)
{
  GetBitContext *gb = &v->s.gb;
  MpegEncContext *s = &v->s;
  int dc_pred_dir = 0; /* Direction of the DC prediction used */
  int i;
  int16_t *dc_val;
  int16_t *ac_val, *ac_val2;
  int dcdiff, index, val;
  S32CPS(xr11, xr0, xr0);
  /* Get DC differential */
  VLC *table;
  table = (n < 4) ? ff_msmp4_dc_luma_vlc[s->dc_table_index].table:
    ff_msmp4_dc_chroma_vlc[s->dc_table_index].table;
#ifdef JZC_VLC_HW_OPT
  dcdiff = get_vlc2_hw_m4(table);
#else
  dcdiff = get_vlc2(gb, table, DC_VLC_BITS, 4);
#endif
  if (dcdiff < 0){
    av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
    return -1;
  }
  if (dcdiff)
    {
      if (dcdiff == 119 /* ESC index value */)
        {
	  /* TODO: Optimize */
	  dcdiff = (v->pq == 1)? (get_bits(gb, 10)) : ((v->pq == 2) ?  get_bits(gb, 9): get_bits(gb, 8));
        }
      else
        {
	  dcdiff = (v->pq == 1)?((dcdiff<<2) + get_bits(gb, 2) - 3) : ((v->pq == 2) ?(dcdiff<<1) + get_bits1(gb)   - 1 : dcdiff);
	    
        }
      i_movn(dcdiff, -dcdiff, get_bits(gb,1));	
    }

  /* Prediction */
  dcdiff += vc1_i_pred_dc(s, v->overlap, v->pq, n, &dc_val, &dc_pred_dir);
  *dc_val = dcdiff;

  /* Store the quantized DC coeff, used for prediction */
  int dc_scale = (n<4) ? s->y_dc_scale:s->c_dc_scale;
  block[0] = dcdiff * dc_scale;

  /* Skip ? */
  if (!coded) {
    goto not_coded;
  }

  //AC Decoding
  i = 1;

  {

    int skip, value;
    const uint8_t *zz_table;
    int16_t scale;
    int k;

    scale = v->pq * 2 + v->halfpq;

    if(v->s.ac_pred) {
      if(!dc_pred_dir)
	zz_table = wmv1_scantable[2];
      else
	zz_table = wmv1_scantable[3];
    } else
      zz_table = wmv1_scantable[1];

#ifdef JZC_VLC_HW_OPT
    vc1_decode_residual_i(v, block, zz_table, codingset);
#else
    int last=0;
    vc1_decode_residual(v, block, 63, 0, 0, last, 0, zz_table, 0, codingset);
#endif
    ac_val = left_buf+((n>3?n:n>>1)<<3);
    ac_val2 = s->ac_val[0][0] + (s->mb_x*48) + ((n&5)<<3);
    /* apply AC prediction if needed */
    if(s->ac_pred) {
      if(dc_pred_dir) { //left
#ifdef JZC_MXU_OPT
	block[8] += ac_val[1];
	S16LDD(xr6,block,0x20,0);
	S16LDD(xr6,block,0x30,1);
	S16LDD(xr7,block,0x40,0);
	S16LDD(xr7,block,0x50,1);
	S16LDD(xr8,block,0x60,0);
	S16LDD(xr8,block,0x70,1);

	S32LDD(xr1,ac_val,0x4);
	S32LDD(xr2,ac_val,0x8);
	S32LDD(xr3,ac_val,0xc);

	Q16ADD_AA_WW(xr9,xr6,xr1,xr0);
	Q16ADD_AA_WW(xr10,xr7,xr2,xr0);
	Q16ADD_AA_WW(xr4,xr8,xr3,xr0);

	S32I2M(xr5,0x00080000);
	D32SLL(xr6,xr5,xr0,xr0,1);
	S32I2M(xr12,0x00080000);
	S16STD(xr9,block,0x20,0);
	S16STD(xr9,block,0x30,1);
	Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	D16MOVN(xr12,xr9,xr5);

	S16STD(xr10,block,0x40,0);
	S16STD(xr10,block,0x50,1);
	Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	D16MOVN(xr12,xr10,xr5);

	S16STD(xr4,block,0x60,0);
	S16STD(xr4,block,0x70,1);		    
	Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	D16MOVN(xr12,xr4,xr5);
	S32SFL(xr12,xr0,xr12,xr10,3);
	S32MAX(xr12, xr12, xr10);
	S32MAX(xr11, xr11, xr12);
#else
	for(k = 1; k < 8; k++){
	  block[k << 3] += ac_val[k];
	  if(block[k << 3]){
	    S32I2M(xr10,(k << 3));
	    S32MAX(xr11, xr11, xr10);
	  }
	}
#endif
      } else { //top
#ifdef JZC_MXU_OPT
	block[1] += ac_val2[1];
	S32LDD(xr6,ac_val2,0x4);
	S32LDD(xr7,ac_val2,0x8);
	S32LDD(xr8,ac_val2,0xc);

	S32LDD(xr2,block,0x4);
	S32LDD(xr3,block,0x8);
	S32LDD(xr4,block,0xc);
		
	Q16ADD_AA_WW(xr2,xr2,xr6,xr0);
	Q16ADD_AA_WW(xr3,xr3,xr7,xr0);
	Q16ADD_AA_WW(xr4,xr4,xr8,xr0);

	S32STD(xr2,block,0x4);
	S32STD(xr3,block,0x8);
	S32STD(xr4,block,0xc);

#else
	for(k = 1; k < 8; k++)
	  block[k] += ac_val[k + 8];
#endif
      }
    }
    /* save AC coeffs for further prediction */
#ifdef JZC_MXU_OPT
    ac_val[1] = block[8];
    ac_val2[1] = block[1];
    S16LDD(xr6,block,0x20,0);
    S16LDD(xr6,block,0x30,1);
    S16LDD(xr7,block,0x40,0);
    S16LDD(xr7,block,0x50,1);
    S16LDD(xr8,block,0x60,0);
    S16LDD(xr8,block,0x70,1);

    S32STD(xr6,ac_val,0x4);
    S32STD(xr7,ac_val,0x8);
    S32STD(xr8,ac_val,0xc);

    S32LDD(xr2,block,0x4);
    S32LDD(xr3,block,0x8);
    S32LDD(xr4,block,0xc);

    S32STD(xr2,ac_val2,0x4);
    S32STD(xr3,ac_val2,0x8);
    S32STD(xr4,ac_val2,0xc);
#else
    for(k = 1; k < 8; k++) {
      ac_val2[k] = block[k << 3];
      ac_val2[k + 8] = block[k];
    }
#endif
    dMB->mq = v->pq;
    if(s->ac_pred) i = 63;
  }

 not_coded:
  if(!coded) {
    int k, scale;
    scale = v->pq * 2 + v->halfpq;

    ac_val = left_buf+((n>3?n:n>>1)<<3);
    ac_val2 = s->ac_val[0][0] + (s->mb_x*48) + ((n&5)<<3);
    /* apply AC prediction if needed */
    if(s->ac_pred) {
      if(dc_pred_dir) { //left
#ifdef JZC_MXU_OPT
	block[8] =  ac_val[1] * scale;
	S32I2M(xr15,scale);
	S32LDD(xr1,ac_val,0x4);
	S32LDD(xr2,ac_val,0x8);
	S32LDD(xr3,ac_val,0xc);

	D16MUL_LW(xr4,xr15,xr1,xr5);
	D16MUL_LW(xr6,xr15,xr2,xr7);
	D16MUL_LW(xr8,xr15,xr3,xr9);

	S16STD(xr5,block,0x20,0);
	S16STD(xr4,block,0x30,0);
	S16STD(xr7,block,0x40,0);
	S16STD(xr6,block,0x50,0);
	S16STD(xr9,block,0x60,0);
	S16STD(xr8,block,0x70,0);
	for(k = 1; k < 8; k++) {
	  i_movn(block[k << 3],block[k << 3] + ((block[k << 3] < 0)? -v->pq : v->pq),(!v->pquantizer && block[k << 3]));
	  if(block[k << 3]){		   
	    S32I2M(xr10,(k << 3));
	    S32MAX(xr11, xr11, xr10);
	  }
	}
	S32STD(xr0,ac_val2,0x0);
	S32STD(xr0,ac_val2,0x4);
	S32STD(xr0,ac_val2,0x8);
	S32STD(xr0,ac_val2,0xc);
#else
	for(k = 1; k < 8; k++) {
	  block[k << 3] = ac_val[k] * scale;
	  if(!v->pquantizer && block[k << 3])
	    block[k << 3] += (block[k << 3] < 0) ? -v->pq : v->pq;
	  if(block[k << 3]){
	    S32I2M(xr10,(k << 3));
	    S32MAX(xr11, xr11, xr10);
	  }
	}
#endif
      } else { //top
#ifdef JZC_MXU_OPT
	block[1] = ac_val2[1] * scale;
	S32I2M(xr15,scale);  
	S32LDD(xr6,ac_val2,0x4);
	S32LDD(xr7,ac_val2,0x8);
	S32LDD(xr8,ac_val2,0xc);

	D16MUL_LW(xr1,xr15,xr6,xr2);
	D16MUL_LW(xr3,xr15,xr7,xr4);
	D16MUL_LW(xr5,xr15,xr8,xr6);

	S32SFL(xr0,xr1,xr2,xr1,ptn3);
	S32SFL(xr0,xr3,xr4,xr3,ptn3);
	S32SFL(xr0,xr5,xr6,xr5,ptn3);
		
	S32STD(xr1,block,0x4);
	S32STD(xr3,block,0x8);
	S32STD(xr5,block,0xc);

	for(k = 1; k < 8; k++) {		  
	  i_movn(block[k], block[k]+((block[k] < 0) ? -v->pq : v->pq),
		 !v->pquantizer && block[k]);
	}
	S32STD(xr0,ac_val,0x0);
	S32STD(xr0,ac_val,0x4);
	S32STD(xr0,ac_val,0x8);
	S32STD(xr0,ac_val,0xc);
#else
	for(k = 1; k < 8; k++) {
	  block[k] = ac_val[k + 8] * scale;
	  if(!v->pquantizer && block[k])
	    block[k] += (block[k] < 0) ? -v->pq : v->pq;
	}
#endif
      }
      i = 63;
    }else{
      S32STD(xr0,ac_val,0);
      S32STD(xr0,ac_val,4);
      S32STD(xr0,ac_val,8);
      S32STD(xr0,ac_val,0xc);

      S32STD(xr0,ac_val2,0x0);
      S32STD(xr0,ac_val2,0x4);
      S32STD(xr0,ac_val2,0x8);
      S32STD(xr0,ac_val2,0xc);
    }
  }

  dMB->idct_row[n] = (S32M2I(xr11) >> 3) + 1;
  s->block_last_index[n] = i;
  return 0;
}

/** Decode intra block in intra frames - should be faster than decode_intra_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock number
 * @param coded are AC coeffs present or not
 * @param codingset set of VLC to decode data
 * @param mquant quantizer value for this macroblock
 */
static int vc1_decode_i_block_adv(VC1Context *v, DCTELEM block[64], int n, int coded, int codingset, int mquant)
{
  GetBitContext *gb = &v->s.gb;
  MpegEncContext *s = &v->s;
  int dc_pred_dir = 0; /* Direction of the DC prediction used */
  int i;
  int16_t *dc_val;
  int16_t *ac_val, *ac_val2;
  int dcdiff;
  int a_avail = v->a_avail, c_avail = v->c_avail;
  int use_pred = s->ac_pred;
  int scale, index, val;
  int q1, q2 = 0;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;

  S32CPS(xr11, xr0, xr0);
  /* Get DC differential */
  VLC *table;
  if (n < 4) {
    table = ff_msmp4_dc_luma_vlc[s->dc_table_index].table;
  } else {
    table = ff_msmp4_dc_chroma_vlc[s->dc_table_index].table;
  }
  dcdiff = get_vlc2(gb, table, DC_VLC_BITS, 4);

  if (dcdiff < 0){
    av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
    return -1;
  }
  if (dcdiff)
    {
      if (dcdiff == 119 /* ESC index value */)
        {
	  /* TODO: Optimize */
	  if (mquant == 1) dcdiff = get_bits(gb, 10);
	  else if (mquant == 2) dcdiff = get_bits(gb, 9);
	  else dcdiff = get_bits(gb, 8);
        }
      else
        {
	  if (mquant == 1)
	    dcdiff = (dcdiff<<2) + get_bits(gb, 2) - 3;
	  else if (mquant == 2)
	    dcdiff = (dcdiff<<1) + get_bits1(gb)   - 1;
        }
      if (get_bits1(gb))
	dcdiff = -dcdiff;
    }

  /* Prediction */
  dcdiff += vc1_pred_dc(s, v->overlap, mquant, n, a_avail, c_avail, &dc_val, &dc_pred_dir);
  *dc_val = dcdiff;

  /* Store the quantized DC coeff, used for prediction */
  int dc_scale = (n<4)?s->y_dc_scale:s->c_dc_scale;
  block[0] = dcdiff * dc_scale;

  //AC Decoding
  i = 1;

  /* check if AC is needed at all */
  if(!a_avail && !c_avail) use_pred = 0;
  ac_val = left_buf+((n>3?n:n>>1)<<3);
  ac_val2 = s->ac_val[0][0] + (s->mb_x*48) + ((n&5)<<3);

  scale = mquant * 2 + ((mquant == v->pq) ? v->halfpq : 0);

  q1 = s->current_picture.qscale_table[mb_pos];
  if(dc_pred_dir && c_avail && mb_pos) q2 = s->current_picture.qscale_table[mb_pos - 1];
  if(!dc_pred_dir && a_avail && mb_pos >= s->mb_stride) q2 = s->current_picture.qscale_table[mb_pos - s->mb_stride];
  if(dc_pred_dir && n==1) q2 = q1;
  if(!dc_pred_dir && n==2) q2 = q1;
  if(n==3) q2 = q1;

  if(coded) {
    int skip, value;
    const uint8_t *zz_table;
    int k;

    if(v->s.ac_pred) {
      if(!dc_pred_dir)
	zz_table = wmv1_scantable[2];
      else
	zz_table = wmv1_scantable[3];
    } else
      zz_table = wmv1_scantable[1];

#ifdef JZC_VLC_HW_OPT
    vc1_decode_residual_i(v, block, zz_table, codingset);
#else
    int last = 0;
    vc1_decode_residual(v, block, 63, 0, 0, last, 0,zz_table, 0, codingset);
#endif

    /* apply AC prediction if needed */
    if(use_pred) {
      /* scale predictors if needed*/
      if(q2 && q1!=q2) {
	q1 = q1 * 2 + ((q1 == v->pq) ? v->halfpq : 0) - 1;
	q2 = q2 * 2 + ((q2 == v->pq) ? v->halfpq : 0) - 1;

	if(dc_pred_dir) { //left
	  for(k = 1; k < 8; k++){
	    block[k << 3] += (ac_val[k] * q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
	    if(block[k << 3]){
	      S32I2M(xr10,(k << 3));
	      S32MAX(xr11, xr11, xr10);
	    }
	  }
	} else { //top
	  for(k = 1; k < 8; k++)
	    block[k] += (ac_val2[k] * q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
	}
      } else {
	if(dc_pred_dir) { //left
#ifdef JZC_MXU_OPT
	  block[8] += ac_val[1];
	  S16LDD(xr6,block,0x20,0);
	  S16LDD(xr6,block,0x30,1);
	  S16LDD(xr7,block,0x40,0);
	  S16LDD(xr7,block,0x50,1);
	  S16LDD(xr8,block,0x60,0);
	  S16LDD(xr8,block,0x70,1);

	  S32LDD(xr1,ac_val,0x4);
	  S32LDD(xr2,ac_val,0x8);
	  S32LDD(xr3,ac_val,0xc);

	  Q16ADD_AA_WW(xr9,xr6,xr1,xr0);
	  Q16ADD_AA_WW(xr10,xr7,xr2,xr0);
	  Q16ADD_AA_WW(xr4,xr8,xr3,xr0);

	  S32I2M(xr5,0x00080000);
	  D32SLL(xr6,xr5,xr0,xr0,1);
	  S32I2M(xr12,block[8]?0x8:0);
	  S16STD(xr9,block,0x20,0);
	  S16STD(xr9,block,0x30,1);
	  Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	  D16MOVN(xr12,xr9,xr5);

	  S16STD(xr10,block,0x40,0);
	  S16STD(xr10,block,0x50,1);
	  Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	  D16MOVN(xr12,xr10,xr5);

	  S16STD(xr4,block,0x60,0);
	  S16STD(xr4,block,0x70,1);		    
	  Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	  D16MOVN(xr12,xr4,xr5);
	  S32SFL(xr12,xr0,xr12,xr10,3);
	  S32MAX(xr12, xr12, xr10);
	  S32MAX(xr11, xr11, xr12);
#else
	  for(k = 1; k < 8; k++){
	    block[k << 3] += ac_val[k];
	    if(block[k << 3]){
	      S32I2M(xr10,(k << 3));
	      S32MAX(xr11, xr11, xr10);
	    }
	  }
#endif
	} else { //top
#ifdef JZC_MXU_OPT
	  block[1] += ac_val2[1];
	  S32LDD(xr6,ac_val2,0x4);
	  S32LDD(xr7,ac_val2,0x8);
	  S32LDD(xr8,ac_val2,0xc);

	  S32LDD(xr2,block,0x4);
	  S32LDD(xr3,block,0x8);
	  S32LDD(xr4,block,0xc);

	  Q16ADD_AA_WW(xr2,xr2,xr6,xr0);
	  Q16ADD_AA_WW(xr3,xr3,xr7,xr0);
	  Q16ADD_AA_WW(xr4,xr4,xr8,xr0);

	  S32STD(xr2,block,0x4);
	  S32STD(xr3,block,0x8);
	  S32STD(xr4,block,0xc);
#else
	  for(k = 1; k < 8; k++)
	    block[k] += ac_val[k + 8];
#endif
	}
      }
    }
    /* save AC coeffs for further prediction */
#ifdef JZC_MXU_OPT
    ac_val[1] = block[8];
    ac_val2[1] = block[1];
    S16LDD(xr6,block,0x20,0);
    S16LDD(xr6,block,0x30,1);
    S16LDD(xr7,block,0x40,0);
    S16LDD(xr7,block,0x50,1);
    S16LDD(xr8,block,0x60,0);
    S16LDD(xr8,block,0x70,1);

    S32STD(xr6,ac_val,0x4);
    S32STD(xr7,ac_val,0x8);
    S32STD(xr8,ac_val,0xc);

    S32LDD(xr2,block,0x4);
    S32LDD(xr3,block,0x8);
    S32LDD(xr4,block,0xc);

    S32STD(xr2,ac_val2,0x4);
    S32STD(xr3,ac_val2,0x8);
    S32STD(xr4,ac_val2,0xc);
#else
    for(k = 1; k < 8; k++) {
      ac_val2[k] = block[k << 3];
      ac_val2[k + 8] = block[k];
    }
#endif

    /* scale AC coeffs */
#if 0
#ifdef JZC_MXU_OPT
    val = (S32M2I(xr11) >> 3) + 1;
    S16LDD(xr15,&mquant,0,3);
    S16LDD(xr14,&scale,0,3);
    if(v->pquantizer){
      if(block[1]) {
	block[1] *= scale;
      }
      DCTELEM *blk = block;
      for(k = 0; k < val*8; k++)
	{
	  S32LDI(xr13,blk,0x4);
	  D16MUL_WW(xr12,xr13,xr14,xr10);
	  S32SFL(xr0,xr12,xr10,xr12,ptn3);
	  S32STD(xr12,blk,0x0);
	}
    }else{
      if(block[1]) {
	block[1] *= scale;
	block[1] += (block[1] < 0) ? -mquant : mquant;
      }
      DCTELEM *blk = block;
      for(k = 0; k < val*8; k++)
	{
	  S32LDI(xr13,blk,0x4);
	  if(S32M2I(xr13)==0)
	    continue;
	  D16MUL_WW(xr12,xr13,xr14,xr10);
	  S32SFL(xr0,xr12,xr10,xr12,ptn3);
	  D16CPS(xr9,xr15,xr12);
	  D16MOVZ(xr9,xr12,xr0);
	  Q16ADD_AA_WW(xr12,xr12,xr9,xr0);
	  S32STD(xr12,blk,0x0);
	}
    }
#else
    for(k = 1; k < 64; k++)
      if(block[k]) {
	block[k] *= scale;
	if(!v->pquantizer)
	  block[k] += (block[k] < 0) ? -mquant : mquant;
      }
#endif
#endif
    dMB->mq = mquant;
    if(use_pred) i = 63;
  } else { // no AC coeffs
    int k;
    /* apply AC prediction if needed */
    if(use_pred) {
      if(q2 && q1!=q2) {
	int16_t *ac_v=dc_pred_dir?ac_val:ac_val2;
	q1 = q1 * 2 + ((q1 == v->pq) ? v->halfpq : 0) - 1;
	q2 = (q2 * 2 + ((q2 == v->pq) ? v->halfpq : 0) - 1) * ff_vc1_dqscale[q1 - 1] ;

	for(k = 1; k < 8; k++)
	  ac_v[k] = (ac_v[k] * q2 + 0x20000) >> 18;
      }
      if(dc_pred_dir) { //left
#ifdef JZC_MXU_OPT
	block[8] =  ac_val[1] * scale;		  
	S32I2M(xr15,scale);  
	S32LDD(xr1,ac_val,0x4);
	S32LDD(xr2,ac_val,0x8);
	S32LDD(xr3,ac_val,0xc);

	D16MUL_LW(xr4,xr15,xr1,xr5);
	D16MUL_LW(xr6,xr15,xr2,xr7);
	D16MUL_LW(xr8,xr15,xr3,xr9);
		
	S16STD(xr5,block,0x20,0);
	S16STD(xr4,block,0x30,0);
	S16STD(xr7,block,0x40,0);
	S16STD(xr6,block,0x50,0);
	S16STD(xr9,block,0x60,0);
	S16STD(xr8,block,0x70,0);
	for(k = 1; k < 8; k++) {
	  if(!v->pquantizer && block[k << 3])
	    block[k << 3] += (block[k << 3] < 0) ? -mquant : mquant;
	  if(block[k << 3]){
	    S32I2M(xr10,(k << 3));
	    S32MAX(xr11, xr11, xr10);
	  }
	}
	S32STD(xr0,ac_val2,0x0);
	S32STD(xr0,ac_val2,0x4);
	S32STD(xr0,ac_val2,0x8);
	S32STD(xr0,ac_val2,0xc);
#else
	for(k = 1; k < 8; k++) {
	  block[k << 3] = ac_val2[k] * scale;
	  if(!v->pquantizer && block[k << 3])
	    block[k << 3] += (block[k << 3] < 0) ? -mquant : mquant;
	  if(block[k << 3]){
	    S32I2M(xr10,(k << 3));
	    S32MAX(xr11, xr11, xr10);
	  }
	}
#endif
      } else { //top
#ifdef JZC_MXU_OPT
	block[1] = ac_val2[1] * scale;
	S32I2M(xr15,scale);
	S32LDD(xr6,ac_val2,0x4);
	S32LDD(xr7,ac_val2,0x8);
	S32LDD(xr8,ac_val2,0xc);

	D16MUL_LW(xr1,xr15,xr6,xr2);
	D16MUL_LW(xr3,xr15,xr7,xr4);
	D16MUL_LW(xr5,xr15,xr8,xr6);

	S32SFL(xr0,xr1,xr2,xr1,ptn3);
	S32SFL(xr0,xr3,xr4,xr3,ptn3);
	S32SFL(xr0,xr5,xr6,xr5,ptn3);

	S32STD(xr1,block,0x4);
	S32STD(xr3,block,0x8);
	S32STD(xr5,block,0xc);

	for(k = 1; k < 8; k++) {
	  if(!v->pquantizer && block[k])
	    block[k] += (block[k] < 0) ? -mquant : mquant;
	}
	S32STD(xr0,ac_val,0x0);
	S32STD(xr0,ac_val,0x4);
	S32STD(xr0,ac_val,0x8);
	S32STD(xr0,ac_val,0xc);
#else
	for(k = 1; k < 8; k++) {
	  block[k] = ac_val2[k + 8] * scale;
	  if(!v->pquantizer && block[k])
	    block[k] += (block[k] < 0) ? -mquant : mquant;
	}
#endif
      }
      i = 63;
    }else{
      S32STD(xr0,ac_val,0);
      S32STD(xr0,ac_val,4);
      S32STD(xr0,ac_val,8);
      S32STD(xr0,ac_val,0xc);

      S32STD(xr0,ac_val2,0x0);
      S32STD(xr0,ac_val2,0x4);
      S32STD(xr0,ac_val2,0x8);
      S32STD(xr0,ac_val2,0xc);
    }
  }
  dMB->idct_row[n] = (S32M2I(xr11) >> 3) + 1;
  s->block_last_index[n] = i;
  return 0;
}

/** Decode intra block in inter frames - more generic version than vc1_decode_i_block
 * @param v VC1Context
 * @param block block to decode
 * @param[in] n subblock index
 * @param coded are AC coeffs present or not
 * @param mquant block quantizer
 * @param codingset set of VLC to decode data
 */
static int vc1_decode_intra_block(VC1Context *v, DCTELEM block[64], int n, int coded, int mquant, int codingset)
{
  GetBitContext *gb = &v->s.gb;
  MpegEncContext *s = &v->s;
  int dc_pred_dir = 0; /* Direction of the DC prediction used */
  int i;
  int16_t *dc_val;
  int16_t *ac_val, *ac_val2;
  int dcdiff;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  int a_avail = v->a_avail, c_avail = v->c_avail;
  int use_pred = s->ac_pred;
  int scale, index, val;
  int q1, q2 = 0;
  dMB->idct_row[n] = 1;
  S32CPS(xr11, xr0, xr0);
  /* XXX: Guard against dumb values of mquant */
  mquant = (mquant < 1) ? 0 : ( (mquant>31) ? 31 : mquant );

  /* Set DC scale - y and c use the same */
  s->y_dc_scale = s->y_dc_scale_table[mquant];
  s->c_dc_scale = s->c_dc_scale_table[mquant];

  VLC *table;
  table = (n < 4) ? ff_msmp4_dc_luma_vlc[s->dc_table_index].table:
    ff_msmp4_dc_chroma_vlc[s->dc_table_index].table;

  dcdiff = get_vlc2(gb, table, DC_VLC_BITS, 4);

  if (dcdiff < 0){
    av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
    return -1;
  }
  if (dcdiff)
    {
      if (dcdiff == 119 /* ESC index value */)
        {
	  /* TODO: Optimize */
	  dcdiff = (mquant == 1)? (get_bits(gb, 10)) : ((mquant == 2) ?  get_bits(gb, 9): get_bits(gb, 8));
        }
      else
        {
	  dcdiff = (mquant == 1)?((dcdiff<<2) + get_bits(gb, 2) - 3) : ((mquant == 2) ?(dcdiff<<1) + get_bits1(gb)   - 1 : dcdiff);

        }
      i_movn(dcdiff, -dcdiff, get_bits(gb,1));
    }

  /* Prediction */
  dcdiff += vc1_pred_dc(s, v->overlap, mquant, n, a_avail, c_avail, &dc_val, &dc_pred_dir);
  *dc_val = dcdiff;

  /* Store the quantized DC coeff, used for prediction */
  int dc_scale = (n<4)?s->y_dc_scale:s->c_dc_scale;
  block[0] = dcdiff * dc_scale;
  /* Skip ? */
  //AC Decoding
  i = 1;

  /* check if AC is needed at all and adjust direction if needed */
  i_movn(dc_pred_dir, 1, !a_avail);
  i_movn(dc_pred_dir, 0, !c_avail);
  i_movn(use_pred, 0, (!a_avail && !c_avail));

  ac_val = left_buf+((n>3?n:n>>1)<<3);
  ac_val2 = s->ac_val[0][0] + (s->mb_x*48) + ((n&5)<<3);
  int16_t *ac_v=dc_pred_dir?ac_val:ac_val2;

  scale = mquant * 2 + v->halfpq;

  q1 = s->current_picture.qscale_table[mb_pos];
  i_movn(q2,s->current_picture.qscale_table[mb_pos-1],dc_pred_dir && c_avail && mb_pos);
  i_movn(q2,s->current_picture.qscale_table[mb_pos - s->mb_stride],!dc_pred_dir && a_avail && mb_pos >= s->mb_stride);
  i_movn(q2, q1, (dc_pred_dir && n==1)||(!dc_pred_dir && n==2)||(n==3));    

  if(coded) {
    int skip, value;
    const uint8_t *zz_table;
    int k;

    zz_table = wmv1_scantable[0];
#ifdef JZC_VLC_HW_OPT
    vc1_decode_residual_i(v, block, zz_table, codingset);
#else
    int last = 0;
    vc1_decode_residual(v, block, 63, 0, 0, last, 0, zz_table, 0, codingset);
#endif
    dMB->idct_row[n] = (S32M2I(xr11) >> 3) + 1;
    /* apply AC prediction if needed */
    if(use_pred) {
      /* scale predictors if needed*/
      if(q2 && q1!=q2) {
#ifdef JZC_MXU_OPT
	int dc_fg = (dc_pred_dir)? 3  : 0;
	q1 = q1 * 2 + ((q1 == v->pq) ? v->halfpq : 0) - 1;
	q2 = (q2 * 2 + ((q2 == v->pq) ? v->halfpq : 0) - 1) * ff_vc1_dqscale[q1 - 1] ;

	for(k = 1; k < 8; k++){
	  block[k << dc_fg] += (ac_v[k] * q2 + 0x20000) >> 18;
	  if(block[k << dc_fg]){
	    S32I2M(xr10,(k << dc_fg));
	    S32MAX(xr11, xr11, xr10);
	  }
	}                
#else
	q1 = q1 * 2 + ((q1 == v->pq) ? v->halfpq : 0) - 1;
	q2 = q2 * 2 + ((q2 == v->pq) ? v->halfpq : 0) - 1;

	if(dc_pred_dir) { //left
	  for(k = 1; k < 8; k++){
	    block[k << 3] += (ac_val[k] * q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
	    if(block[k << 3]){
	      S32I2M(xr10,(k << 3));
	      S32MAX(xr11, xr11, xr10);
	    }
	  }
	} else { //top
	  for(k = 1; k < 8; k++)
	    block[k] += (ac_val[k + 8] * q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;		    
	}

#endif
	    
      } else {
	if(dc_pred_dir) { //left
#ifdef JZC_MXU_OPT
	  block[8] += ac_val[1];
	  S16LDD(xr6,block,0x20,0);
	  S16LDD(xr6,block,0x30,1);
	  S16LDD(xr7,block,0x40,0);
	  S16LDD(xr7,block,0x50,1);
	  S16LDD(xr8,block,0x60,0);
	  S16LDD(xr8,block,0x70,1);

	  S32LDD(xr1,ac_val,0x4);
	  S32LDD(xr2,ac_val,0x8);
	  S32LDD(xr3,ac_val,0xc);

	  Q16ADD_AA_WW(xr9,xr6,xr1,xr0);
	  Q16ADD_AA_WW(xr10,xr7,xr2,xr0);
	  Q16ADD_AA_WW(xr4,xr8,xr3,xr0);

	  S32I2M(xr5,0x00080000);
	  D32SLL(xr6,xr5,xr0,xr0,1);
	  S32I2M(xr12,block[8]?0x8:0);
	  S16STD(xr9,block,0x20,0);
	  S16STD(xr9,block,0x30,1);
	  Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	  D16MOVN(xr12,xr9,xr5);

	  S16STD(xr10,block,0x40,0);
	  S16STD(xr10,block,0x50,1);
	  Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	  D16MOVN(xr12,xr10,xr5);

	  S16STD(xr4,block,0x60,0);
	  S16STD(xr4,block,0x70,1);		    
	  Q16ADD_AA_HW(xr5,xr6,xr5,xr0);
	  D16MOVN(xr12,xr4,xr5);
	  S32SFL(xr12,xr0,xr12,xr10,3);
	  S32MAX(xr12, xr12, xr10);
	  S32MAX(xr11, xr11, xr12);
#else
	  for(k = 1; k < 8; k++){
	    block[k << 3] += ac_val[k];
	    if(block[k << 3]) {
	      S32I2M(xr10,(k << 3));
	      S32MAX(xr11, xr11, xr10);
	    }
	  }
#endif
	} else { //top
#ifdef JZC_MXU_OPT
	  block[1] += ac_val2[1];
	  S32LDD(xr6,ac_val2,0x4);
	  S32LDD(xr7,ac_val2,0x8);
	  S32LDD(xr8,ac_val2,0xc);
		  
	  S32LDD(xr2,block,0x4);
	  S32LDD(xr3,block,0x8);
	  S32LDD(xr4,block,0xc);
		
	  Q16ADD_AA_WW(xr2,xr2,xr6,xr0);
	  Q16ADD_AA_WW(xr3,xr3,xr7,xr0);
	  Q16ADD_AA_WW(xr4,xr4,xr8,xr0);
		  
	  S32STD(xr2,block,0x4);
	  S32STD(xr3,block,0x8);
	  S32STD(xr4,block,0xc);

	  S32I2M(xr12,0x7);
	  S32MAX(xr11, xr11, xr12);
#else
	  for(k = 1; k < 8; k++)
	    block[k] += ac_val[k + 8];
#endif
	}
      }
    }
    /* save AC coeffs for further prediction */
#ifdef JZC_MXU_OPT
    ac_val[1] = block[8];
    ac_val2[1] = block[1];
    S16LDD(xr6,block,0x20,0);
    S16LDD(xr6,block,0x30,1);
    S16LDD(xr7,block,0x40,0);
    S16LDD(xr7,block,0x50,1);
    S16LDD(xr8,block,0x60,0);
    S16LDD(xr8,block,0x70,1);

    S32STD(xr6,ac_val,0x4);
    S32STD(xr7,ac_val,0x8);
    S32STD(xr8,ac_val,0xc);

    S32LDD(xr2,block,0x4);
    S32LDD(xr3,block,0x8);
    S32LDD(xr4,block,0xc);

    S32STD(xr2,ac_val2,0x4);
    S32STD(xr3,ac_val2,0x8);
    S32STD(xr4,ac_val2,0xc);
#else
    for(k = 1; k < 8; k++) {
      ac_val2[k] = block[k << 3];
      ac_val2[k + 8] = block[k];
    }
#endif

    dMB->mq = mquant;
    if(use_pred) i = 63;
  } else { // no AC coeffs
    int k;

    /* apply AC prediction if needed */
    if(use_pred) {
      if(q2 && q1!=q2) {
	q1 = q1 * 2 + ((q1 == v->pq) ? v->halfpq : 0) - 1;
	q2 = (q2 * 2 + ((q2 == v->pq) ? v->halfpq : 0) - 1) * ff_vc1_dqscale[q1 - 1] ;

	for(k = 1; k < 8; k++)
	  ac_v[k] = (ac_v[k] * q2 + 0x20000) >> 18;
      }
      if(dc_pred_dir) { //left
#ifdef JZC_MXU_OPT
	block[8] =  ac_val[1] * scale;
	S32I2M(xr15,scale);
	S32LDD(xr1,ac_val,0x4);
	S32LDD(xr2,ac_val,0x8);
	S32LDD(xr3,ac_val,0xc);

	D16MUL_LW(xr4,xr15,xr1,xr5);
	D16MUL_LW(xr6,xr15,xr2,xr7);
	D16MUL_LW(xr8,xr15,xr3,xr9);

	S16STD(xr5,block,0x20,0);
	S16STD(xr4,block,0x30,0);
	S16STD(xr7,block,0x40,0);
	S16STD(xr6,block,0x50,0);
	S16STD(xr9,block,0x60,0);
	S16STD(xr8,block,0x70,0);
	for(k = 1; k < 8; k++) {
	  i_movn(block[k<<3],block[k<<3] + ((block[k<<3] < 0)? -mquant : mquant),(!v->pquantizer && block[k<<3]));
	  if(block[k<<3]){
	    S32I2M(xr10,(k << 3));
	    S32MAX(xr11, xr11, xr10);
	  }
	}
	S32STD(xr0,ac_val2,0x0);
	S32STD(xr0,ac_val2,0x4);
	S32STD(xr0,ac_val2,0x8);
	S32STD(xr0,ac_val2,0xc);
#else
	for(k = 1; k < 8; k++) {
	  block[k << 3] = ac_val2[k] * scale;
	  if(!v->pquantizer && block[k << 3])
	    block[k << 3] += (block[k << 3] < 0) ? -mquant : mquant;
	  if(block[k << 3]){
	    S32I2M(xr10,(k << 3));
	    S32MAX(xr11, xr11, xr10);
	  }
	}
#endif
      } else { //top
#ifdef JZC_MXU_OPT
	block[1] = ac_val2[1] * scale;
	S32I2M(xr15,scale);  
	S32LDD(xr6,ac_val2,0x4);
	S32LDD(xr7,ac_val2,0x8);
	S32LDD(xr8,ac_val2,0xc);

	D16MUL_LW(xr1,xr15,xr6,xr2);
	D16MUL_LW(xr3,xr15,xr7,xr4);
	D16MUL_LW(xr5,xr15,xr8,xr6);

	S32SFL(xr0,xr1,xr2,xr1,ptn3);
	S32SFL(xr0,xr3,xr4,xr3,ptn3);
	S32SFL(xr0,xr5,xr6,xr5,ptn3);

	S32STD(xr1,block,0x4);
	S32STD(xr3,block,0x8);
	S32STD(xr5,block,0xc);

	for(k = 1; k < 8; k++) {
	  i_movn(block[k], block[k] + ((block[k] < 0) ? -mquant : mquant),
		 !v->pquantizer && block[k]);
	}
	S32STD(xr0,ac_val,0x0);
	S32STD(xr0,ac_val,0x4);
	S32STD(xr0,ac_val,0x8);
	S32STD(xr0,ac_val,0xc);
#else
	for(k = 1; k < 8; k++) {
	  block[k] = ac_val2[k + 8] * scale;
	  if(!v->pquantizer && block[k])
	    block[k] += (block[k] < 0) ? -mquant : mquant;
	}
#endif
      }
      i = 63;
    }else{
      S32STD(xr0,ac_val,0);
      S32STD(xr0,ac_val,4);
      S32STD(xr0,ac_val,8);
      S32STD(xr0,ac_val,0xc);

      S32STD(xr0,ac_val2,0x0);
      S32STD(xr0,ac_val2,0x4);
      S32STD(xr0,ac_val2,0x8);
      S32STD(xr0,ac_val2,0xc);
    }
  }
  dMB->idct_row[n] = (S32M2I(xr11) >> 3) + 1;
  s->block_last_index[n] = i;
  return 0;
}

static int vc1_decode_p_block_vlc(VC1Context *v, DCTELEM block[64], int n, int mquant, int ttmb, int first_block)
{
  MpegEncContext *s = &v->s;
  GetBitContext *gb = &s->gb;
  int i, j;
  int subblkpat = 0;
  int scale, off, idx, last, skip, value;
  int tt[4];
  uint8_t ttblk = ttmb & 7;
  int pat = 0;
  const uint8_t *zz_table;

  S32CPS(xr11, xr0, xr0);
  //s->dsp.clear_block(block);
  if(ttmb == -1) {
    ttblk = ff_vc1_ttblk_to_tt[v->tt_index][get_vlc2(gb, ff_vc1_ttblk_vlc[v->tt_index].table, VC1_TTBLK_VLC_BITS, 1)];
  }
  if(ttblk == TT_4X4) {
    subblkpat = ~(get_vlc2(gb, ff_vc1_subblkpat_vlc[v->tt_index].table, VC1_SUBBLKPAT_VLC_BITS, 1) + 1);
  }
  if((ttblk != TT_8X8 && ttblk != TT_4X4)
     && ((v->ttmbf || (ttmb != -1 && (ttmb & 8) && !first_block))
	 || (!v->res_rtm_flag && !first_block))) {
    subblkpat = decode012(gb);
    if(subblkpat) subblkpat ^= 3; //swap decoded pattern bits
    if(ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM) ttblk = TT_8X4;
    if(ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT) ttblk = TT_4X8;
  }
  scale = 2 * mquant + ((v->pq == mquant) ? v->halfpq : 0);

  // convert transforms like 8X4_TOP to generic TT and SUBBLKPAT
  if(ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM) {
    subblkpat = 2 - (ttblk == TT_8X4_TOP);
    ttblk = TT_8X4;
  }
  if(ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT) {
    subblkpat = 2 - (ttblk == TT_4X8_LEFT);
    ttblk = TT_4X8;
  }
  dMB->ttblock[n] = ttblk;
  dMB->subblockpat[n] = subblkpat;     
  switch(ttblk) {
  case TT_8X8:
    pat = 0xF;
    zz_table=wmv1_scantable[0];
#ifdef JZC_VLC_HW_OPT
    vc1_decode_residual_p(v,block, 63, scale, mquant, zz_table);
#else
    vc1_decode_residual(v, block, 63, scale, mquant, 0, 0, zz_table, 1, v->codingset2);
#endif
    dMB->idct_row[n] =  (S32M2I(xr11) >> 3) + 1;
    dma_len += dMB->idct_row[n];
    break;
  case TT_4X4:
    pat = ~subblkpat & 0xF;
    zz_table=ff_vc1_simple_progressive_4x4_zz;
    for(j = 0; j < 4; j++) {
      last = subblkpat & (1 << (3 - j));
      off = (j & 1) * 4 + (j & 2) * 16;
#ifdef JZC_VLC_HW_OPT
      if(!last)
	vc1_decode_residual_p(v,block+off, 15, scale, mquant, zz_table);
#else
      vc1_decode_residual(v, block, 15,scale,mquant, last, off, zz_table, 1, v->codingset2);
#endif
      dMB->idct_row_4x4[n*4+j] =  (S32M2I(xr11) >> 3) + 1; 
      tt[j] =  (S32M2I(xr11) >> 3) + 1;
    }
    dma_len += (FFMAX(tt[2], tt[3]) + 4);
    break;
  case TT_8X4:
    pat = ~((subblkpat & 2)*6 + (subblkpat & 1)*3) & 0xF;
    zz_table=v->zz_8x4;
    for(j = 0; j < 2; j++) {
      last = subblkpat & (1 << (1 - j));
      off = j * 32;
#ifdef JZC_VLC_HW_OPT
      if(!last)
	vc1_decode_residual_p(v,block+off, 31, scale, mquant, zz_table);
#else
      vc1_decode_residual(v, block, 31,scale, mquant, last, off, zz_table, 1, v->codingset2);
#endif
      dMB->idct_row_8x4[n*2+j] =  (S32M2I(xr11) >> 3) + 1;  
    }
    dma_len += (dMB->idct_row_8x4[n*2+1] + 4);
    break;
  case TT_4X8:
    pat = ~(subblkpat*5) & 0xF;
    zz_table=v->zz_4x8;
    for(j = 0; j < 2; j++) {
      last = subblkpat & (1 << (1 - j));
      off = j * 4;
#ifdef JZC_VLC_HW_OPT
      if(!last)
	vc1_decode_residual_p(v,block+off, 31, scale, mquant, zz_table);
#else
      vc1_decode_residual(v, block, 31, scale, mquant, last, off, zz_table, 1, v->codingset2);
#endif
      tt[j] = (S32M2I(xr11) >> 3) + 1;
      dMB->idct_row_8x4[n*2+j] =  (S32M2I(xr11) >> 3) + 1;  
    }
    dma_len += FFMAX(tt[0], tt[1]);
    break;
  }
  return pat;
}

/***********************************************************************/
/**
 * @defgroup vc1block VC-1 Block-level functions
 * @see 7.1.4, p91 and 8.1.1.7, p(1)04
 * @{
 */

/**
 * @def GET_MQUANT
 * @brief Get macroblock-level quantizer scale
 */
#define GET_MQUANT()						\
  if (v->dquantfrm)						\
    {								\
      int edges = 0;						\
      if (v->dqprofile == DQPROFILE_ALL_MBS)			\
	{							\
	  if (v->dqbilevel)					\
	    {							\
	      mquant = (get_bits1(gb)) ? v->altpq : v->pq;	\
	    }							\
	  else							\
	    {							\
	      mqdiff = get_bits(gb, 3);				\
	      if (mqdiff != 7) mquant = v->pq + mqdiff;		\
	      else mquant = get_bits(gb, 5);			\
	    }							\
	}							\
      if(v->dqprofile == DQPROFILE_SINGLE_EDGE)			\
        edges = 1 << v->dqsbedge;				\
      else if(v->dqprofile == DQPROFILE_DOUBLE_EDGES)		\
        edges = (3 << v->dqsbedge) % 15;			\
      else if(v->dqprofile == DQPROFILE_FOUR_EDGES)		\
        edges = 15;						\
      if((edges&1) && !s->mb_x)					\
        mquant = v->altpq;					\
      if((edges&2) && s->first_slice_line)			\
        mquant = v->altpq;					\
      if((edges&4) && s->mb_x == (s->mb_width - 1))		\
        mquant = v->altpq;					\
      if((edges&8) && s->mb_y == (s->mb_height - 1))		\
        mquant = v->altpq;					\
    }

/**
 * @def GET_MVDATA(_dmv_x, _dmv_y)
 * @brief Get MV differentials
 * @see MVDATA decoding from 8.3.5.2, p(1)20
 * @param _dmv_x Horizontal differential for decoded MV
 * @param _dmv_y Vertical differential for decoded MV
 */
#define GET_MVDATA(_dmv_x, _dmv_y)					\
  index = 1 + get_vlc2(gb, ff_vc1_mv_diff_vlc[s->mv_table_index].table,	\
                       VC1_MV_DIFF_VLC_BITS, 2);			\
  if (index > 36)							\
    {									\
      mb_has_coeffs = 1;						\
      index -= 37;							\
    }									\
  else mb_has_coeffs = 0;						\
  s->mb_intra = 0;							\
  if (!index) { _dmv_x = _dmv_y = 0; }					\
  else if (index == 35)							\
    {									\
      _dmv_x = get_bits(gb, v->k_x - 1 + s->quarter_sample);		\
      _dmv_y = get_bits(gb, v->k_y - 1 + s->quarter_sample);		\
    }									\
  else if (index == 36)							\
    {									\
      _dmv_x = 0;							\
      _dmv_y = 0;							\
      s->mb_intra = 1;							\
    }									\
  else									\
    {									\
      index1 = index%6;							\
      if (!s->quarter_sample && index1 == 5) val = 1;			\
      else                                   val = 0;			\
      if(size_table[index1] - val > 0)					\
        val = get_bits(gb, size_table[index1] - val);			\
      else                                   val = 0;			\
      sign = 0 - (val&1);						\
      _dmv_x = (sign ^ ((val>>1) + offset_table[index1])) - sign;	\
									\
      index1 = index/6;							\
      if (!s->quarter_sample && index1 == 5) val = 1;			\
      else                                   val = 0;			\
      if(size_table[index1] - val > 0)					\
        val = get_bits(gb, size_table[index1] - val);			\
      else                                   val = 0;			\
      sign = 0 - (val&1);						\
      _dmv_y = (sign ^ ((val>>1) + offset_table[index1])) - sign;	\
    }

/** @} */ // Macroblock group
static const int size_table  [6] = { 0, 2, 3, 4,  5,  8 };
static const int offset_table[6] = { 0, 1, 3, 7, 15, 31 };

static int vc1_decode_p_mb_vlc(VC1Context *v)
{
  MpegEncContext *s = &v->s;
  GetBitContext *gb = &s->gb;
  int i, j;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  int cbp; /* cbp decoding stuff */
  int mqdiff, mquant; /* MB quantization */
  int ttmb = v->ttfrm; /* MB Transform type */

  int mb_has_coeffs = 1; /* last_flag */
  int dmv_x, dmv_y; /* Differential MV components */
  int index, index1; /* LUT indices */
  int val, sign; /* temp values */
  int first_block = 1;
  int off;
  uint8_t skipped, fourmv, dst_idx;
  int block_cbp = 0, pat;

  mquant = v->pq; /* Loosy initialization */

  if (v->mv_type_is_raw)
    fourmv = get_bits1(gb);
  else
    fourmv = v->mv_type_mb_plane[mb_pos];
  if (v->skip_is_raw)
    skipped = get_bits1(gb);
  else
    skipped = v->s.mbskip_table[mb_pos];

  dMB->vc1_fourmv = fourmv;
  dMB->vc1_skipped = skipped;

  if (likely(!skipped)){
    MXU_MEMSET(dMB->mb, 0, sizeof(DCTELEM)*6*64);
    if (!fourmv) /* 1MV mode */
      {
	GET_MVDATA(dmv_x, dmv_y);

	if (s->mb_intra) {
	  s->current_picture.motion_val[1][s->block_index[0]][0] = 0;
	  s->current_picture.motion_val[1][s->block_index[0]][1] = 0;
	  s->current_picture.mb_type[mb_pos] = MB_TYPE_INTRA;
	}else
	  s->current_picture.mb_type[mb_pos] = MB_TYPE_16x16;
	vc1_pred_mv(s, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type[0]);

	/* FIXME Set DC val for inter block ? */
	if (s->mb_intra && !mb_has_coeffs)
	  {
	    GET_MQUANT();
	    s->ac_pred = get_bits1(gb);
	    cbp = 0;
	  }
	else if (mb_has_coeffs)
	  {
	    if (s->mb_intra) s->ac_pred = get_bits1(gb);
	    cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
	    GET_MQUANT();
	  }
	else
	  {
	    mquant = v->pq;
	    cbp = 0;
	  }
	s->current_picture.qscale_table[mb_pos] = mquant;

	if (!v->ttmbf && !s->mb_intra && mb_has_coeffs)
	  ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table,
			  VC1_TTMB_VLC_BITS, 2);
	for (i=0; i<6; i++){
	  dMB->vc1_intra[i] = s->mb_intra;
	  s->dc_val[0][s->block_index[i]] = 0;
	  val = ((cbp >> (5 - i)) & 1);
	  dMB->pb_val[i] = val;
	  v->mb_type[0][s->block_index[i]] = s->mb_intra;
	  if(s->mb_intra) {
	    /* check if prediction blocks A and C are available */
	    v->a_avail = v->c_avail = 0;
	    if(i == 2 || i == 3 || !s->first_slice_line)
	      v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
	    if(i == 1 || i == 3 || s->mb_x)
	      v->c_avail = v->mb_type[0][s->block_index[i] - 1];
	    vc1_decode_intra_block(v, dMB->mb + dma_len * 8, i, val, mquant, (i&4)?v->codingset2:v->codingset);
	    dma_len += dMB->idct_row[i];
	    block_cbp |= 0xF << (i << 2);
	  }else if(val) {
	    pat = vc1_decode_p_block_vlc(v, dMB->mb + dma_len * 8, i, mquant, ttmb, first_block);
	    block_cbp |= pat << (i << 2);

	    if(!v->ttmbf && ttmb < 8) ttmb = -1;
	    first_block = 0;
	  }
	}
	return 0;
      }
    else//4MV mode
      {
	int intra_count = 0, coded_inter = 0;
	uint8_t is_intra[6], is_coded[6];
	/* Get CBPCY */
	cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
	for (i=0; i<6; i++)
	  {
	    val = ((cbp >> (5 - i)) & 1);
	    s->dc_val[0][s->block_index[i]] = 0;
	    s->mb_intra = 0;
	    if(i < 4) {
	      dmv_x = dmv_y = 0;
	      s->mb_intra = 0;
                   
	      mb_has_coeffs = 0;
	      if(val) {
		GET_MVDATA(dmv_x, dmv_y);
	      }
	      vc1_pred_mv(s, i, dmv_x, dmv_y, 0, v->range_x, v->range_y, v->mb_type[0]);
	      //dMB->sintra[i] = s->mb_intra;      

	      intra_count += s->mb_intra;
	      is_intra[i] = s->mb_intra;
	      is_coded[i] = mb_has_coeffs;
	      dMB->vc1_intra[i] = s->mb_intra;
	      dMB->pb_val[i] = mb_has_coeffs; 
	    }
	    if(i&4){
	      is_intra[i] = (intra_count >= 3);
	      is_coded[i] = val;
	      dMB->vc1_intra[i] = (intra_count >= 3);;
	      dMB->pb_val[i] = val;
	    }

	    v->mb_type[0][s->block_index[i]] = is_intra[i];
	    //if (i<4) dMB->vc1_blk_index[i] = v->mb_type[0][s->block_index[i]];
	    if(!coded_inter) coded_inter = !is_intra[i] & is_coded[i];
	  }
	// if there are no coded blocks then don't do anything more
	if(!intra_count && !coded_inter) return 0;
	GET_MQUANT();
	s->current_picture.qscale_table[mb_pos] = mquant;
	/* test if block is intra and has pred */
	{
	  int intrapred = 0;
	  for(i=0; i<6; i++)
	    if(is_intra[i]) {
	      if(((!s->first_slice_line || (i==2 || i==3)) && v->mb_type[0][s->block_index[i] - s->block_wrap[i]])
		 || ((s->mb_x || (i==1 || i==3)) && v->mb_type[0][s->block_index[i] - 1])) {
		intrapred = 1;
		break;
	      }
	    }
	  if(intrapred)s->ac_pred = get_bits1(gb);
	  else s->ac_pred = 0;
	}
	if (!v->ttmbf && coded_inter)
	  ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
	for (i=0; i<6; i++)
	  {
	    s->mb_intra = is_intra[i];
	    if (is_intra[i]) {
	      /* check if prediction blocks A and C are available */
	      v->a_avail = v->c_avail = 0;
	      if(i == 2 || i == 3 || !s->first_slice_line)
		v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
	      if(i == 1 || i == 3 || s->mb_x)
		v->c_avail = v->mb_type[0][s->block_index[i] - 1];                  
	      vc1_decode_intra_block(v,dMB->mb+dma_len*8, i, is_coded[i], mquant, (i&4)?v->codingset2:v->codingset);
	      dma_len += dMB->idct_row[i];
	      block_cbp |= 0xF << (i << 2);
	    } else if(is_coded[i]) {
	      pat = vc1_decode_p_block_vlc(v,dMB->mb+dma_len*8, i, mquant, ttmb, first_block);
	      block_cbp |= pat << (i << 2);
	      if(!v->ttmbf && ttmb < 8) ttmb = -1;
	      first_block = 0;
	    }
	  }
	return 0;
      }
  }
  else //Skipped
    {
      if (!fourmv) /* 1MV mode */
        {
	  s->mb_intra = 0;
	  for(i = 0; i < 6; i++) {
	    v->mb_type[0][s->block_index[i]] = 0;
	    s->dc_val[0][s->block_index[i]] = 0;
	  }
	  s->current_picture.mb_type[mb_pos] = MB_TYPE_SKIP;
	  s->current_picture.qscale_table[mb_pos] = 0;
	  vc1_pred_mv(s, 0, 0, 0, 1, v->range_x, v->range_y, v->mb_type[0]);
	  return 0;
        }
      else //4MV mode
        {
	  s->mb_intra = 0;
	  s->current_picture.qscale_table[mb_pos] = 0;
	  for (i=0; i<6; i++) {
	    v->mb_type[0][s->block_index[i]] = 0;
	    s->dc_val[0][s->block_index[i]] = 0;
	  }
	  for (i=0; i<4; i++)
            {
	      vc1_pred_mv(s, i, 0, 0, 0, v->range_x, v->range_y, v->mb_type[0]);
            }

	  s->current_picture.qscale_table[mb_pos] = 0;
	  return 0;
        }
    }
  v->cbp[s->mb_x] = block_cbp;
}

static void vc1_decode_b_mb_vlc(VC1Context *v)
{
  MpegEncContext *s = &v->s;
  GetBitContext *gb = &s->gb;
  int i, j;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  int cbp = 0; /* cbp decoding stuff */
  int mqdiff, mquant; /* MB quantization */
  int ttmb = v->ttfrm; /* MB Transform type */

  int mb_has_coeffs = 0; /* last_flag */
  int index, index1; /* LUT indices */
  int val, sign; /* temp values */
  int first_block = 1;
  int off;
  uint8_t skipped, direct, dst_idx;
  int dmv_x[2], dmv_y[2];
  int bmvtype = BMV_TYPE_BACKWARD;

  mquant = v->pq; /* Loosy initialization */
  s->mb_intra = 0;
  dma_len = 0;
  if (v->dmb_is_raw)
    direct = get_bits1(gb);
  else
    direct = v->direct_mb_plane[mb_pos];
  if (v->skip_is_raw)
    skipped = get_bits1(gb);
  else
    skipped = v->s.mbskip_table[mb_pos];

  dmv_x[0] = dmv_x[1] = dmv_y[0] = dmv_y[1] = 0;
  for(i = 0; i < 6; i++) {
    v->mb_type[0][s->block_index[i]] = 0;
    s->dc_val[0][s->block_index[i]] = 0;
  }
  s->current_picture.qscale_table[mb_pos] = 0;

  MXU_MEMSET(dMB->mb, 0, sizeof(DCTELEM)*6*64); 
  dMB->vc1_direct = direct;
  dMB->vc1_skipped = skipped;
  dMB->vc1_b_mc_skipped = 0;

  if (!direct) {
    if (!skipped) {
      GET_MVDATA(dmv_x[0], dmv_y[0]);
      dmv_x[1] = dmv_x[0];
      dmv_y[1] = dmv_y[0];
    }
    if(skipped || !s->mb_intra) {
      bmvtype = decode012(gb);
      switch(bmvtype) {
      case 0:
	bmvtype = (v->bfraction >= (B_FRACTION_DEN/2)) ? BMV_TYPE_BACKWARD : BMV_TYPE_FORWARD;
	break;
      case 1:
	bmvtype = (v->bfraction >= (B_FRACTION_DEN/2)) ? BMV_TYPE_FORWARD : BMV_TYPE_BACKWARD;
	break;
      case 2:
	bmvtype = BMV_TYPE_INTERPOLATED;
	dmv_x[0] = dmv_y[0] = 0;
      }
    }
  }
  for(i = 0; i < 6; i++)
    v->mb_type[0][s->block_index[i]] = s->mb_intra;

  if (skipped) {
    if(direct) bmvtype = BMV_TYPE_INTERPOLATED;
    vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
    dMB->bfmv_type = bmvtype;
    return;
  }
  if (direct) {
    cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
    GET_MQUANT();
    s->mb_intra = 0;
    s->current_picture.qscale_table[mb_pos] = mquant;
    if(!v->ttmbf)
      ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
    dmv_x[0] = dmv_y[0] = dmv_x[1] = dmv_y[1] = 0;
    vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);

    dMB->bfmv_type = bmvtype;
  } else {
    dMB->vc1_mb_has_coeffs[0] = mb_has_coeffs;
    dMB->bintra = s->mb_intra;
    if(!mb_has_coeffs && !s->mb_intra) {
      /* no coded blocks - effectively skipped */
      vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
      dMB->bfmv_type = bmvtype;
      return;
    }
    if(s->mb_intra && !mb_has_coeffs) {
      GET_MQUANT();
      s->current_picture.qscale_table[mb_pos] = mquant;
      s->ac_pred = get_bits1(gb);
      cbp = 0;
      vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
      dMB->vc1_b_mc_skipped = 1;
    } else {
      if(bmvtype == BMV_TYPE_INTERPOLATED) {
	GET_MVDATA(dmv_x[0], dmv_y[0]);
	dMB->vc1_mb_has_coeffs[1] = mb_has_coeffs;
	if(!mb_has_coeffs) {
	  /* interpolated skipped block */
	  vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
	  dMB->bfmv_type = bmvtype;
	  return;
	}
      }
      vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);

      if (s->mb_intra){
	dMB->vc1_b_mc_skipped = 1;
      }

      //dMB->pintra[1] = s->mb_intra;
      dMB->bfmv_type = bmvtype;
      if(s->mb_intra)
	s->ac_pred = get_bits1(gb);
      cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
      GET_MQUANT();
      s->current_picture.qscale_table[mb_pos] = mquant;
      if(!v->ttmbf && !s->mb_intra && mb_has_coeffs)
	ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
    }
  }

  for (i=0; i<6; i++)
    {
      s->dc_val[0][s->block_index[i]] = 0;
      val = ((cbp >> (5 - i)) & 1);
      dMB->pb_val[i] = val;
      v->mb_type[0][s->block_index[i]] = s->mb_intra;
      dMB->vc1_intra[i] = s->mb_intra;
      if(s->mb_intra) {
	/* check if prediction blocks A and C are available */
	v->a_avail = v->c_avail = 0;
	if(i == 2 || i == 3 || !s->first_slice_line)
	  v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
	if(i == 1 || i == 3 || s->mb_x)
	  v->c_avail = v->mb_type[0][s->block_index[i] - 1];

	vc1_decode_intra_block(v, dMB->mb+dma_len*8, i, val, mquant, (i&4)?v->codingset2:v->codingset);
	dma_len += dMB->idct_row[i];
      } else if(val) {
	vc1_decode_p_block_vlc(v, dMB->mb+dma_len*8, i, mquant, ttmb, first_block);
	if(!v->ttmbf && ttmb < 8) ttmb = -1;
	first_block = 0;
      }
    }
}

static void vc1_decode_i_mb_vlc(VC1Context *v)
{
  int k, j;
  MpegEncContext *s = &v->s;
  int cbp, val;
  uint8_t *coded_val;
  int mb_pos;
  int mquant = v->pq;
  int mqdiff;
  int overlap;
  DCTELEM *mb;
  mb = task_fifo_wp;
  GetBitContext *gb = &s->gb;
  mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  
  s->current_picture.mb_type[mb_pos] = MB_TYPE_INTRA;
  s->current_picture.motion_val[1][s->block_index[0]][0] = 0;
  s->current_picture.motion_val[1][s->block_index[0]][1] = 0;

  cbp = get_vlc2(&v->s.gb, ff_msmp4_mb_i_vlc.table, MB_INTRA_VLC_BITS, 2);

  if(v->profile != PROFILE_ADVANCED){
    v->s.ac_pred = get_bits1(&v->s.gb);
  }else{
    if(v->acpred_is_raw)
      v->s.ac_pred = get_bits1(&v->s.gb);
    else
      v->s.ac_pred = v->acpred_plane[mb_pos];

    if(v->condover == CONDOVER_SELECT) {
      if(v->overflg_is_raw)
	overlap = get_bits1(&v->s.gb);
      else
	overlap = v->over_flags_plane[mb_pos];
    }else
      overlap = (v->condover == CONDOVER_ALL);

    GET_MQUANT();

    s->current_picture.qscale_table[mb_pos] = mquant;
    /* Set DC scale - y and c use the same */
    s->y_dc_scale = s->y_dc_scale_table[mquant];
    s->c_dc_scale = s->c_dc_scale_table[mquant];
  }
  MXU_MEMSET(mb, 0, sizeof(DCTELEM)*6*64);
  for(k = 0; k < 6; k++) {
    val = ((cbp >> (5 - k)) & 1);

    if (k < 4) {
      int pred = vc1_coded_block_pred(&v->s, k, &coded_val);
      val = val ^ pred;
      *coded_val = val;
    }
    dMB->pb_val[k] = val;
    cbp |= val << (5 - k);
              
    if(v->profile == PROFILE_ADVANCED){
      v->a_avail = !s->first_slice_line || (k==2 || k==3);
      v->c_avail = !!s->mb_x || (k==1 || k==3);                 
    }

    if(v->profile == PROFILE_ADVANCED){
      vc1_decode_i_block_adv(v, mb + dma_len * 8, k, val, (k<4)? v->codingset : v->codingset2, mquant);
      dma_len += dMB->idct_row[k];
    }else{
      vc1_decode_i_block(v, mb + dma_len * 8, k, val, (k<4)? v->codingset : v->codingset2);
      dma_len += dMB->idct_row[k];
    }

  }
}
#ifdef JZC_VLC_HW_OPT
#undef get_bits
#undef get_bits1
#undef decode210
#undef decode012
#undef get_unary
#undef get_vlc2
#endif

#if 0
void prt_square_test(void * start_ptr,int size,int h,int w, int stride){
  unsigned int* start_int=(int*)start_ptr;
  unsigned short* start_short=(short*)start_ptr;
  unsigned char* start_byte=(char*)start_ptr;
  int i, j;
  if(size==4){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%8x,",start_int[i*stride+j]);
      }
      printf("\n");
    }
  }
  if(size==2){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%4x,",start_short[i*stride+j]);
      }
      printf("\n");
    }
  }
  if(size==1){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%2x,",start_byte[i*stride+j]);
      }
      printf("\n");
    }
  }
}
#endif

/** Initialize a VC1/WMV3 decoder
 * @todo TODO: Handle VC-1 IDUs (Transport level?)
 * @todo TODO: Decypher remaining bits in extra_data
 */
static av_cold int vc1_decode_init(AVCodecContext *avctx)
{
  VC1Context *v = avctx->priv_data;
  MpegEncContext *s = &v->s;
  GetBitContext gb;
  int i;
#ifdef JZC_ROTA90_OPT
    crc_code_rota = 0;
#endif

#ifdef JZC_TLB_OPT
  int ll;
  tlb_i = 0;
  for(ll=0; ll<8; ll++)
    SET_VPU_TLB(ll, 0, 0, 0, 0);
#endif

#ifdef JZC_MC_OPT
  motion_init(VC1_QPEL, H264_EPEL);
#endif

#ifdef JZC_MXU_OPT
  S32I2M(xr16, 0x3);
#endif
#ifdef JZC_TCSM_OPT
  tcsm_init();
#endif

#ifdef JZC_DCORE_OPT
  int * tmp_hm_buf = jz4740_alloc_frame(32, SPACE_HALF_MILLION_BYTE);
  if (tmp_hm_buf < 0)
    printf("JZ4740 ALLOC tmp_hm_buf ERROR !! \n");
#endif

#ifdef JZC_DCORE_OPTn
#define TCSM0_FUNC_LEN (0x2000)
#define TCSM0_FUNC_START (0xf4001000)
      FILE *fp_text;
      int  len, *tcsm;
#if 1//def JZ_LINUX_OS
      int *load_buf = (int *)jz4740_alloc_frame(32, TCSM0_FUNC_LEN);
#else
      int *load_buf = av_malloc(32+TCSM0_FUNC_LEN);
      load_buf=((unsigned)load_buf+31)&0xffffffe0;
#endif
      fp_text = fopen("vc1_tcsm0_text0.bin", "r+b");
      if (!fp_text){
	printf(" error while open vc1_tcsm0_text0.bin \n");
	exit_player_with_rc();
      }else {
	len = fread(load_buf, 4, TCSM0_FUNC_LEN>>2, fp_text);
	printf(" vc1 len of vc1_tcsm0_text0 insn = %d\n",len);
	tcsm = (int *)TCSM0_FUNC_START;
	for(i=0; i<len; i++)
	  tcsm[i] = load_buf[i];
	fclose(fp_text);
      }
#endif

#ifdef JZC_DCORE_OPTn
#define TCSM0_FUNC_START1 (0xf4000020)
     fp_text = fopen("vc1_tcsm0_text1.bin", "r+b");
      if (!fp_text){
	printf(" error while open vc1_tcsm0_text1.bin \n");
	exit_player_with_rc();
      }else {
	len = fread(load_buf, 4, TCSM0_FUNC_LEN>>2, fp_text);
	printf(" vc1 len of vc1_tcsm0_text1 insn = %d\n",len);
	tcsm = (int *)TCSM0_FUNC_START1;
	for(i=0; i<len; i++)
	  tcsm[i] = load_buf[i];
	fclose(fp_text);
      }
#endif

#ifdef JZC_DCORE_OPT
  {
    AUX_RESET();
    int tmp;
    int len, *reserved_mem;
    volatile int *src, *dst;

    *((volatile int *)(TCSM0_P1_TASK_DONE)) = 0;

    //several setup instructions direct p1 to execute p1_boot
    // load p1 insn and data to reserved mem
#ifdef ANDROID
    if(loadfile("vc1_p1.bin",(int *)TCSM1_VUCADDR(VC1_P1_MAIN),SPACE_HALF_MILLION_BYTE,1) == -1){
        printf("LOAD VC1_P1_BIN ERROR.....................\n");
        return -1;
    }
#else
#if 1 //#ifdef JZ_LINUX_OS
      printf("vc1 len of aux task = %d\n", vc1_auxcodes_len);
      reserved_mem = (int *)TCSM1_VCADDR(VC1_P1_MAIN);
      for (i=0; i< vc1_auxcodes_len/4; i++)
        reserved_mem[i] = vc1_aux_task_codes[i];
#else
    FILE *fp_text;
    fp_text = fopen("./vc1_p1.bin", "r+b");
    if (!fp_text)
      printf(" error while open vc1_p1.bin \n");
    int *load_buf = tmp_hm_buf;
    len = fread(load_buf, 4, SPACE_HALF_MILLION_BYTE, fp_text);
    printf(" vc1 len of p1 task = %d\n",len);

    reserved_mem = (int *)TCSM1_VCADDR(VC1_P1_MAIN);
    for(i=0; i<len; i++)
      reserved_mem[i] = load_buf[i];

    fclose(fp_text);
#endif
#endif
    jz_dcache_wb(); /*flush cache into reserved mem*/
    i_sync();
  }
#endif // JZC_DCORE_OPT

#ifdef JZC_PMON_P0
  i_mtc0_2(0, 16, 7);
#endif

#ifdef JZC_VLC_HW_OPT

  // cabac / vlc global enable
  SET_GLOBAL_CTRL(0x10);

  for(i=0;i<8;i++){
    residual_init_table[i][0] = vc1_last_decode_table[i]<<1;
    residual_init_table[i][1] = vc1_index_decode_table[i];
    residual_init_table[i][2] = vc1_last_delta_level_table[i];
    residual_init_table[i][3] = vc1_delta_level_table[i];
    residual_init_table[i][4] = vc1_last_delta_run_table[i];
    residual_init_table[i][5] = vc1_delta_run_table[i];
    residual_init_table[i][6] = CALC_VLC2_CTRL(i);
  }
#endif
  left_buf=jz4740_alloc_frame(32,48*sizeof(int16_t));
  buf2_pre_alloc_buf = jz4740_alloc_frame(4,0x10000);

  if (!avctx->extradata_size || !avctx->extradata) return -1;
  if (!(avctx->flags & CODEC_FLAG_GRAY))
    avctx->pix_fmt = avctx->get_format(avctx, avctx->codec->pix_fmts);
  else
    avctx->pix_fmt = PIX_FMT_GRAY8;
  avctx->hwaccel = ff_find_hwaccel(avctx->codec->id, avctx->pix_fmt);
  v->s.avctx = avctx;
  //avctx->flags |= CODEC_FLAG_EMU_EDGE;
  //v->s.flags |= CODEC_FLAG_EMU_EDGE;

  if(avctx->idct_algo==FF_IDCT_AUTO){
    avctx->idct_algo=FF_IDCT_WMV2;
  }

  if(ff_msmpeg4_vc1_init(avctx) < 0)
    return -1;
  if (vc1_init_common(v) < 0) return -1;

  avctx->coded_width = avctx->width;
  avctx->coded_height = avctx->height;
  if (avctx->codec_id == CODEC_ID_WMV3)
    {
      int count = 0;

      // looks like WMV3 has a sequence header stored in the extradata
      // advanced sequence header may be before the first frame
      // the last byte of the extradata is a version number, 1 for the
      // samples we can decode

      init_get_bits(&gb, avctx->extradata, avctx->extradata_size*8);

      if (vc1_decode_sequence_header(avctx, v, &gb) < 0)
	return -1;

      count = avctx->extradata_size*8 - get_bits_count(&gb);
      if (count>0)
        {
	  av_log(avctx, AV_LOG_INFO, "Extra data: %i bits left, value: %X\n",
		 count, get_bits(&gb, count));
        }
      else if (count < 0)
        {
	  av_log(avctx, AV_LOG_INFO, "Read %i bits in overflow\n", -count);
        }
    } else { // VC1/WVC1
    const uint8_t *start = avctx->extradata;
    uint8_t *end = avctx->extradata + avctx->extradata_size;
    const uint8_t *next;
    int size, buf2_size;
    uint8_t *buf2 = NULL;
    int seq_initialized = 0, ep_initialized = 0;

    if(avctx->extradata_size < 16) {
      av_log(avctx, AV_LOG_ERROR, "Extradata size too small: %i\n", avctx->extradata_size);
      return -1;
    }

    buf2 = av_mallocz(avctx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
    start = find_next_marker(start, end); // in WVC1 extradata first byte is its size, but can be 0 in mkv
    next = start;
    for(; next < end; start = next){
      next = find_next_marker(start + 4, end);
      size = next - start - 4;
      if(size <= 0) continue;
      buf2_size = vc1_unescape_buffer(start + 4, size, buf2);
      init_get_bits(&gb, buf2, buf2_size * 8);
      switch(AV_RB32(start)){
      case VC1_CODE_SEQHDR:
	if(vc1_decode_sequence_header(avctx, v, &gb) < 0){
	  av_free(buf2);
	  return -1;
	}
	seq_initialized = 1;
	break;
      case VC1_CODE_ENTRYPOINT:
	if(vc1_decode_entry_point(avctx, v, &gb) < 0){
	  av_free(buf2);
	  return -1;
	}
	ep_initialized = 1;
	break;
      }
    }
    av_free(buf2);
    if(!seq_initialized || !ep_initialized){
      av_log(avctx, AV_LOG_ERROR, "Incomplete extradata\n");
      return -1;
    }
  }
  avctx->has_b_frames= !!(avctx->max_b_frames);
  s->low_delay = !avctx->has_b_frames;

  s->mb_width = (avctx->coded_width+15)>>4;
  s->mb_height = (avctx->coded_height+15)>>4;

  /* Allocate mb bitplanes */
  v->mv_type_mb_plane = av_malloc(s->mb_stride * s->mb_height);
  v->direct_mb_plane = av_malloc(s->mb_stride * s->mb_height);
  v->acpred_plane = av_malloc(s->mb_stride * s->mb_height);
  v->over_flags_plane = av_malloc(s->mb_stride * s->mb_height);

  v->cbp_base = av_malloc(sizeof(v->cbp_base[0]) * 2 * s->mb_stride);
  v->cbp = v->cbp_base + s->mb_stride;

  /* allocate block type info in that way so it could be used with s->block_index[] */
  v->mb_type_base = av_malloc(s->b8_stride * (s->mb_height * 2 + 1) + s->mb_stride * (s->mb_height + 1) * 2);
  v->mb_type[0] = v->mb_type_base + s->b8_stride + 1;
  v->mb_type[1] = v->mb_type_base + s->b8_stride * (s->mb_height * 2 + 1) + s->mb_stride + 1;
  v->mb_type[2] = v->mb_type[1] + s->mb_stride * (s->mb_height + 1);

  ff_intrax8_common_init(&v->x8,s);
  use_jz_buf=1;
  return 0;
}

static inline void vc1_decode_save_mb_mv(VC1Context *v)
{
  MpegEncContext *s = &v->s;
 
  dMB->mv_mode = v->mv_mode; 
  if (!dMB->vc1_fourmv) {
    if (!dMB->vc1_skipped) {
      if(!dMB->vc1_intra[0]) {
	if(v->s.last_picture.data[0]){
	  s->current_picture.motion_val[1][s->block_index[0]][0] = dMB->vc1_mv[0][0];
	  s->current_picture.motion_val[1][s->block_index[0]][1] = dMB->vc1_mv[0][1];	
	}
      }
    }else{
      if(v->s.last_picture.data[0]){
	s->current_picture.motion_val[1][s->block_index[0]][0] = dMB->vc1_mv[0][0];
	s->current_picture.motion_val[1][s->block_index[0]][1] = dMB->vc1_mv[0][1];	
      }
    }
  }
  else {
    int i,j, idx, tx = 0, ty = 0;
    int mvx[4], mvy[4], intra[4];
    static const int count[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
    dMB->chroma_ret = 0;
    if(v->s.last_picture.data[0]) {
      if( !(s->flags & CODEC_FLAG_GRAY) ) {
	for(i = 0; i < 4; i++) {
	  mvx[i] = dMB->vc1_mv[i][0];
	  mvy[i] = dMB->vc1_mv[i][1];
	  intra[i] = v->mb_type[0][s->block_index[i]];
	}
	/* calculate chroma MV vector from four luma MVs */
	idx = (intra[3] << 3) | (intra[2] << 2) | (intra[1] << 1) | intra[0];
	if(!idx) { // all blocks are inter
	  S32LDD(xr1,dMB->vc1_mv[0],0x0);
	  S32LDD(xr2,dMB->vc1_mv[1],0x0);
	  S32LDD(xr3,dMB->vc1_mv[2],0x0);
	  S32LDD(xr4,dMB->vc1_mv[3],0x0);
	  D16MAX(xr5,xr1,xr2);
	  D16MAX(xr6,xr3,xr4);
	  
	  D16MIN(xr7,xr1,xr2);
	  D16MIN(xr8,xr3,xr4);
	  
	  D16MIN(xr1,xr5,xr6);
	  D16MAX(xr2,xr7,xr8);
	  
	  Q16ADD_AS_WW(xr3,xr1,xr2,xr0); 
	  tx = (S32M2I(xr3)<<16>>16)/2;
	  ty = (S32M2I(xr3)>>16)/2;
	} else if(count[idx] == 1) { // 3 inter blocks
	  switch(idx) {
	  case 0x1:
	    S32LDD(xr14,dMB->vc1_mv[1],0x0);
	    S32LDD(xr13,dMB->vc1_mv[2],0x0);
	    S32LDD(xr12,dMB->vc1_mv[3],0x0);
	    break;
	  case 0x2:
	    S32LDD(xr14,dMB->vc1_mv[0],0x0);
	    S32LDD(xr13,dMB->vc1_mv[2],0x0);
	    S32LDD(xr12,dMB->vc1_mv[3],0x0);	    
	    break;
	  case 0x4:
	    S32LDD(xr14,dMB->vc1_mv[0],0x0);
	    S32LDD(xr13,dMB->vc1_mv[1],0x0);
	    S32LDD(xr12,dMB->vc1_mv[3],0x0);
	    break;
	  case 0x8:
	    S32LDD(xr14,dMB->vc1_mv[0],0x0);
	    S32LDD(xr13,dMB->vc1_mv[1],0x0);
	    S32LDD(xr12,dMB->vc1_mv[2],0x0);
	    break;
	  }
	  D16MAX(xr4,xr14,xr13);//xr4=max of (A,B)
	  D16MIN(xr5,xr14,xr13);//xr5=min of (A,B)
	  D16MIN(xr4,xr4,xr12);//xr4:comp max(A,B) and C, get rid of the MAX of (A,B,C)
	  D16MAX(xr4,xr4,xr5);
	  tx = (S32M2I(xr4)<<16>>16);
	  ty = (S32M2I(xr4)>>16);
	} else if(count[idx] == 2) {
	  int t1 = 0, t2 = 0;
	  for(i=0; i<3;i++) if(!intra[i]) {t1 = i; break;}
	  for(i= t1+1; i<4; i++)if(!intra[i]) {t2 = i; break;}
	  tx = (mvx[t1] + mvx[t2]) / 2;
	  ty = (mvy[t1] + mvy[t2]) / 2;
	} else {
	  dMB->chroma_ret = 1;	  
	}
	s->current_picture.motion_val[1][s->block_index[0]][0] = tx;
	s->current_picture.motion_val[1][s->block_index[0]][1] = ty;	
	 
        dMB->tx = tx;
        dMB->ty = ty;
      }
    }
  }
}

static void vc1_decode_blocks_header(VC1Context *v)
{
  MpegEncContext *s = &v->s;
  if (s->pict_type == FF_I_TYPE || (s->pict_type == FF_B_TYPE && v->bi_type)){
    switch(v->y_ac_table_index){
    case 0:
      v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
      break;
    case 1:
      v->codingset = CS_HIGH_MOT_INTRA;
      break;
    case 2:
      v->codingset = CS_MID_RATE_INTRA;
      break;
    }

    if(v->profile != PROFILE_ADVANCED){
      s->y_dc_scale = s->y_dc_scale_table[v->pq];
      s->c_dc_scale = s->c_dc_scale_table[v->pq];
    }

    s->mb_x = s->mb_y = 0;
    s->mb_intra = 1;
  }else{
    switch(v->c_ac_table_index){
    case 0:
      v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
      break;
    case 1:
      v->codingset = CS_HIGH_MOT_INTRA;
      break;
    case 2:
      v->codingset = CS_MID_RATE_INTRA;
      break;
    }
  }
 
  switch(v->c_ac_table_index){
  case 0:
    v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
    break;
  case 1:
    v->codingset2 = CS_HIGH_MOT_INTER;
    break;
  case 2:
    v->codingset2 = CS_MID_RATE_INTER;
    break;
  }
  s->first_slice_line = 1;
}

static enum AVDiscard last_skip_frame = AVDISCARD_DEFAULT;

/** Decode a VC1/WMV3 frame
 * @todo TODO: Handle VC-1 IDUs (Transport level?)
 */
static int vc1_decode_frame(AVCodecContext *avctx,
                            void *data, int *data_size,
                            AVPacket *avpkt)
{
  const uint8_t *buf = avpkt->data;
  int buf_size = avpkt->size;
  VC1Context *v = avctx->priv_data;
  MpegEncContext *s = &v->s;
  AVFrame *pict = data;
  uint8_t *buf2 = NULL;
  const uint8_t *buf_start = buf;
  //s->flags= avctx->flags;
  int count,realnum = 0;

#ifdef JZC_DCORE_OPT
  unsigned int tcsm0_fifo_rp_lh;
  unsigned int task_fifo_wp_lh;
  unsigned int task_fifo_wp_lh_overlap;
#endif
#ifdef JZ-IN-MODIFY
  static unsigned char * dyy;
  static unsigned char * duv;
  int dump_i, dump_j;
  int dump_mb_x, dump_mb_y;
  unsigned char *dump_ptr;
  unsigned char *dump_frm_ptr,*dy,*dc;    
  int linesize = s->linesize;
  int uvlinesize=s->uvlinesize;
  int mb_height = (avctx->height + 15)>> 4;
  int mb_width = (avctx->width + 15) >> 4;
#endif

  /* no supplementary picture */
  if (buf_size == 0) {
    /* special case for last picture */
    if (s->low_delay==0 && s->next_picture_ptr) {
      *pict= *(AVFrame*)s->next_picture_ptr;
      s->next_picture_ptr= NULL;

      *data_size = sizeof(AVFrame);
    }

    return 0;
  }

  /* We need to set current_picture_ptr before reading the header,
   * otherwise we cannot store anything in there. */
  if(s->current_picture_ptr==NULL || s->current_picture_ptr->data[0]){
    int i= ff_find_unused_picture(s, 0);
    s->current_picture_ptr= &s->picture[i];
  }

  if (s->avctx->codec->capabilities&CODEC_CAP_HWACCEL_VDPAU){
    if (v->profile < PROFILE_ADVANCED)
      avctx->pix_fmt = PIX_FMT_VDPAU_WMV3;
    else
      avctx->pix_fmt = PIX_FMT_VDPAU_VC1;
  }

  //for advanced profile we may need to parse and unescape data
  if (avctx->codec_id == CODEC_ID_VC1) {
    int buf_size2 = 0;
    //buf2 = av_mallocz(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);
    buf2 = buf2_pre_alloc_buf;

    if(IS_MARKER(AV_RB32(buf))){ /* frame starts with marker and needs to be parsed */
      const uint8_t *start, *end, *next;
      int size;

      next = buf;
      for(start = buf, end = buf + buf_size; next < end; start = next){
	next = find_next_marker(start + 4, end);
	size = next - start - 4;
	if(size <= 0) continue;
	switch(AV_RB32(start)){
	case VC1_CODE_FRAME:
	  if (avctx->hwaccel ||
	      s->avctx->codec->capabilities&CODEC_CAP_HWACCEL_VDPAU)
	    buf_start = start;
	  buf_size2 = vc1_unescape_buffer(start + 4, size, buf2);
	  break;
	case VC1_CODE_ENTRYPOINT: /* it should be before frame data */
	  buf_size2 = vc1_unescape_buffer(start + 4, size, buf2);
	  init_get_bits(&s->gb, buf2, buf_size2*8);
	  vc1_decode_entry_point(avctx, v, &s->gb);
	  break;
	case VC1_CODE_SLICE:
	  av_log(avctx, AV_LOG_ERROR, "Sliced decoding is not implemented (yet)\n");
	  //av_free(buf2);
	  return -1;
	}
      }
    }else if(v->interlace && ((buf[0] & 0xC0) == 0xC0)){ /* WVC1 interlaced stores both fields divided by marker */
      const uint8_t *divider;

      divider = find_next_marker(buf, buf + buf_size);
      if((divider == (buf + buf_size)) || AV_RB32(divider) != VC1_CODE_FIELD){
	av_log(avctx, AV_LOG_ERROR, "Error in WVC1 interlaced frame\n");
	//av_free(buf2);
	return -1;
      }

      buf_size2 = vc1_unescape_buffer(buf, divider - buf, buf2);
      // TODO
      if(!v->warn_interlaced++)
	av_log(v->s.avctx, AV_LOG_ERROR, "Interlaced WVC1 support is not implemented\n");
      //av_free(buf2);
      return -1;
    }else{
      buf_size2 = vc1_unescape_buffer(buf, buf_size, buf2);
    }
    init_get_bits(&s->gb, buf2, buf_size2*8);
  } else
    init_get_bits(&s->gb, buf, buf_size*8);
  // do parse frame header
  if(v->profile < PROFILE_ADVANCED) {
    if(vc1_parse_frame_header(v, &s->gb) == -1) {
      //av_free(buf2);
      return -1;
    }
  } else {
    if(vc1_parse_frame_header_adv(v, &s->gb) == -1) {
      //av_free(buf2);
      return -1;
    }
  }

  // for hurry_up==5
  s->current_picture.pict_type= s->pict_type;
  s->current_picture.key_frame= s->pict_type == FF_I_TYPE;

  /* skip B-frames if we don't have reference frames */
  if(s->last_picture_ptr==NULL && (s->pict_type==FF_B_TYPE || s->dropable)){
    //av_free(buf2);
    return -1;//buf_size;
  }
  /* skip b frames if we are in a hurry */
  if(avctx->hurry_up && s->pict_type==FF_B_TYPE) {
    //av_free(buf2);
    return -1;//buf_size;
  }
  if(   (avctx->skip_frame >= AVDISCARD_NONREF && s->pict_type==FF_B_TYPE)
	|| (avctx->skip_frame >= AVDISCARD_NONKEY && s->pict_type!=FF_I_TYPE)
	||  avctx->skip_frame >= AVDISCARD_ALL) {
      last_skip_frame = avctx->skip_frame;
      //av_free(buf2);
      return buf_size;
  }
  /* skip everything if we are in a hurry>=5 */
  if(avctx->hurry_up>=5) {
    //av_free(buf2);
    return -1;//buf_size;
  }
  if( (last_skip_frame >= AVDISCARD_NONKEY && avctx->skip_frame == AVDISCARD_DEFAULT && s->pict_type != FF_I_TYPE)) {
    //av_free(buf2);
      return buf_size;
  }
  last_skip_frame = avctx->skip_frame;

  if(s->next_p_frame_damaged){
    if(s->pict_type==FF_B_TYPE){
      //av_free(buf2);
      return buf_size;
    }
    else
      s->next_p_frame_damaged=0;
  }

  if(MPV_frame_start(s, avctx) < 0) {
    //av_free(buf2);
    return -1;
  }

#ifdef JZ-IN-MODIFY
  dyy = s->current_picture.data[0];
  duv = s->current_picture.data[1];
#ifdef JZC_ROTA90_OPT	
  rota_y = s->current_picture.data[2];
  rota_c = s->current_picture.data[3];
#endif
#endif

  s->me.qpel_put= s->dsp.put_qpel_pixels_tab;
  s->me.qpel_avg= s->dsp.avg_qpel_pixels_tab;

  if ((CONFIG_VC1_VDPAU_DECODER)
      &&s->avctx->codec->capabilities&CODEC_CAP_HWACCEL_VDPAU)
    ff_vdpau_vc1_decode_picture(s, buf_start, (buf + buf_size) - buf_start);
  else if (avctx->hwaccel) {
    if (avctx->hwaccel->start_frame(avctx, buf, buf_size) < 0){
      //av_free(buf2);
      return -1;
    }
    if (avctx->hwaccel->decode_slice(avctx, buf_start, (buf + buf_size) - buf_start) < 0)
      {
	//av_free(buf2);
	return -1;
      }
    if (avctx->hwaccel->end_frame(avctx) < 0)
      {
	//av_free(buf2);
	return -1;
      }
  } else {
    ff_er_frame_start(s);
    v->bits = buf_size * 8;
#ifdef JZC_DCORE_OPT
    v->s.esc3_level_length = 0;
    vc1_decode_blocks_header(v);
    //dFRM=dcore_sh_buf;
    dFRM=TCSM1_VCADDR(TCSM1_DFRM_BUF);
#ifdef JZC_MC_OPT	
    motion_config_vc1(v);
#endif  
    tcsm1_fifo_wp = (int *)TCSM1_VUCADDR(TCSM1_MBNUM_WP);
    tcsm0_fifo_rp = (int *)TCSM0_P1_FIFO_RP;

    task_fifo_wp = (int *)TCSM0_TASK_FIFO; // used by P0
    task_fifo_wp_d1 = (int *)TCSM0_TASK_FIFO; // wp delay 1 MB
    task_fifo_wp_d2 = (int *)TCSM0_TASK_FIFO; // wp delay 2 MB
    *tcsm1_fifo_wp = 0; // write by P0, used by P1
    *tcsm0_fifo_rp = 0; // write once before p1 start

    dFRM->mpFrame = mpFrame;
    dFRM->mvmode = v->mv_mode;
    dFRM->mb_width = s->mb_width;
    dFRM->mb_height = s->mb_height;
    dFRM->mb_stride = s->mb_stride;   
    dFRM->linesize = s->linesize;
    dFRM->uvlinesize = s->uvlinesize;
    dFRM->res_fasttx = v->res_fasttx;
    dFRM->res_x8 = v->res_x8;
    dFRM->overlap = v->overlap;
    dFRM->pq = v->pq;
    dFRM->flags = s->flags;
    dFRM->pict_type = s->pict_type;
    dFRM->rnd = 0;//v->rnd;
    dFRM->bi_type = v->bi_type;
    dFRM->profile = v->profile;
    dFRM->rangeredfrm = v->rangeredfrm;
    dFRM->fastuvmc = v->fastuvmc;
    dFRM->mspel = s->mspel;
    dFRM->use_ic = v->use_ic;

    dFRM->halfpq = v->halfpq;
    dFRM->pquantizer = v->pquantizer;

    dFRM->chroma_y_shift = s->chroma_y_shift;
    dFRM->chroma_x_shift = s->chroma_x_shift;
    dFRM->lowres = s->avctx->lowres;
    dFRM->picture_structure = s->picture_structure;
    dFRM->draw_horiz_band = s->avctx->draw_horiz_band != NULL;

    dFRM->h_edge_pos = s->h_edge_pos;
    dFRM->v_edge_pos = s->v_edge_pos;

    dFRM->coded_width = s->avctx->coded_width;
    dFRM->coded_height = s->avctx->coded_height;
 
    dFRM->slinesize[0] = v->s.current_picture.linesize[0];  
    dFRM->slinesize[1] = v->s.current_picture.linesize[1];

    dFRM->last_data[0] = get_phy_addr(v->s.last_picture.data[0]);
    dFRM->last_data[1] = get_phy_addr(v->s.last_picture.data[1]);
	
    dFRM->next_data[0] = get_phy_addr(v->s.next_picture.data[0]);
    dFRM->next_data[1] = get_phy_addr(v->s.next_picture.data[1]);

    dFRM->edge_emu_buffer = s->edge_emu_buffer;  

    dFRM->current_picture_save.ptr[0] =  get_phy_addr(s->current_picture.data[0]);
    dFRM->current_picture_save.ptr[1] =  get_phy_addr(s->current_picture.data[1]);	
#ifdef JZC_ROTA90_OPT	
    dFRM->rota_current_picture.ptr[0] = get_phy_addr(rota_y);
    dFRM->rota_current_picture.ptr[1] = get_phy_addr(rota_c);
#endif

    int n;int temp=dFRM;
    for(n=0;n<FRAME_T_CC_LINE;n++){
      i_cache(0x19,temp,0);
      temp +=32;
    }

    if(unlikely(v->x8_type)){
      ff_intrax8_decode_picture(&v->x8, 2*v->pq+v->halfpq, v->pq*(!v->pquantizer) );
    }
    else if (!(s->pict_type == FF_P_TYPE && v->p_frame_skipped)){

      AUX_RESET(); 
      *((volatile int *)(TCSM0_P1_TASK_DONE)) = 0;
      AUX_START();

      //if(s->pict_type == FF_P_TYPE)
	//memset(v->cbp_base, 0, sizeof(v->cbp_base[0])*2*s->mb_stride);
#ifdef JZC_VLC_HW_OPT
      { // set table data
	int i;
	int tbl_sel;
	unsigned int * vlc_table_base;
	unsigned int * hw_table_base;
	// set 1st table
	if(vc1_hw_codingset1 != v->codingset){
	  tbl_sel = vc1_hw_codingset1 =  v->codingset;
	  vlc_table_base = (unsigned int *)(vlc_base + VLC_TBL_OFST);
	  hw_table_base  = (unsigned int *)&hw_all_table[tbl_sel][0];
	  for (i=0; i<(cfg_all_table[tbl_sel].ram_size + 1)/2; i++)
	    vlc_table_base[i] = hw_table_base[i];
	}
	// set 2nd table
	if(vc1_hw_codingset2 != v->codingset2){
	  tbl_sel = vc1_hw_codingset2 =  v->codingset2;
	  vlc_table_base = (unsigned int *)(vlc_base + VLC_TBL_OFST + 0x800);
	  hw_table_base  = (unsigned int *)&hw_all_table[tbl_sel][0];
	  for (i=0; i<(cfg_all_table[tbl_sel].ram_size + 1)/2; i++)
	    vlc_table_base[i] = hw_table_base[i];
	}
	jz_dcache_wb();
      }

      { // bs
	unsigned int sw_addr, sw_ofst, sw_acc_pos;
	sw_addr = get_phy_addr(s->gb.buffer);
	vc1_hw_bs_buffer = sw_addr;
	sw_ofst = s->gb.index;
	sw_ofst+=(sw_addr&3)<<3;
	unsigned int bs_addr, bs_ofst;
	bs_addr = (sw_addr + (sw_ofst >> 3)) & 0xFFFFFFFC;
	bs_ofst = sw_ofst & 0x1F;

	// bs addr
	CPU_SET_BS_ADDR(bs_addr);
	unsigned int bs_ofst_val = (1 << 16) | (bs_ofst << 5);
	CPU_SET_BS_OFST(bs_ofst_val);
	unsigned int bs_init_done;
	do {
	  bs_init_done = (CPU_GET_BS_OFST() & (1<<16));
	} while (bs_init_done);
      }
#endif
      for(s->mb_y = 0; s->mb_y < s->mb_height; s->mb_y++) {
        s->mb_x = 0;
        jz_init_block_index(s);
	MXU_SETZERO(left_buf,3);
        for(; s->mb_x < s->mb_width; s->mb_x++) {
	  jz_update_block_index(s);
#ifdef JZC_PMON_P0
	  PMON_ON(vc1wait);
#endif
	  do{
	    tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
	    task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
	    task_fifo_wp_lh_overlap = ((unsigned int)(task_fifo_wp + (TASK_BUF_LEN>>2))) & 0x0000FFFF;
	    //task_fifo_wp_lh_overlap = task_fifo_wp_lh + TASK_BUF_LEN;
	  } while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );
#ifdef JZC_PMON_P0
	  PMON_OFF(vc1wait);
#endif

	  dMB = task_fifo_wp;
	  dMB->real_num = 1;
	  dma_len = 0;

	  dMB->mb_x = s->mb_x;
	  dMB->mb_y = s->mb_y;

#ifdef JZC_PMON_P0
	  PMON_ON(vc1vlc);
#endif
	  if (s->pict_type == FF_I_TYPE || (s->pict_type == FF_B_TYPE && v->bi_type)){
	    task_fifo_wp += 5;
	    vc1_decode_i_mb_vlc(v);
	  }else if (s->pict_type == FF_P_TYPE){
	    vc1_decode_p_mb_vlc(v);
	    vc1_decode_save_mb_mv(v);
	  }else{
	    vc1_decode_b_mb_vlc(v);
	    dMB->mv_mode = v->mv_mode; 
	    dMB->vc1_mv[0][0]= s->mv[0][0][0];
	    dMB->vc1_mv[0][1]= s->mv[0][0][1];
	    dMB->vc1_mv[1][0]= s->mv[1][0][0];
	    dMB->vc1_mv[1][1]= s->mv[1][0][1];
	  }
#ifdef JZC_PMON_P0
	  PMON_OFF(vc1vlc);
#endif
#ifdef JZC_VLC_HW_OPT
	  {// check bs
	    unsigned int bs_addr, bs_ofst;
	    bs_ofst = (CPU_GET_BS_OFST() >> 5) & 0x1F;
	    bs_addr = CPU_GET_BS_ADDR() + (bs_ofst >> 3);
	    bs_ofst &= 0x7;
	    //printf(" bs_addr : 0x%x;  bs_ofst : 0x%x \n",bs_addr,bs_ofst);
	    unsigned int sw_addr, sw_ofst, sw_acc_pos;
	    //sw_addr = s->buffer;
	    sw_addr = vc1_hw_bs_buffer;
	    sw_ofst = ((bs_addr - sw_addr) << 3) + bs_ofst ;
	    //printf(" sw_addr : 0x%x; sw_ofst : 0x%x \n",sw_addr,sw_ofst);
	    //if(sw_ofst != s->gb.index) printf(" -------- hw bs error !! sw_ofst: %d; s->gb.index: %d \n",sw_ofst,s->gb.index);
	    s->gb.index = sw_ofst;
	  }
#endif

	  if (s->pict_type == FF_I_TYPE || (s->pict_type == FF_B_TYPE && v->bi_type)){
	    task_fifo_wp += (dma_len * 16) >> 2;
	  }else{
	    task_fifo_wp += (TASK_BUF_LEN-(384<<1) + dma_len * 16) >> 2;
	  }

          int current_mb_len = (unsigned int)(task_fifo_wp - task_fifo_wp_d1) << 2;
	  *(uint16_t *)task_fifo_wp_d2 = current_mb_len;
	  
	  if ( s->mb_x == 0 && s->mb_y == 0 )
	    *(int *)(TCSM1_VUCADDR(TCSM1_FIRST_MBLEN)) = current_mb_len;
	  
	  int reach_tcsm0_end = (((unsigned int)task_fifo_wp & 0x0000FFFF)+TASK_BUF_LEN) >= 0x4000;
	  if (reach_tcsm0_end)
	    task_fifo_wp = (int *)TCSM0_TASK_FIFO;
	  
	  task_fifo_wp_d2 = task_fifo_wp_d1;
	  task_fifo_wp_d1 = task_fifo_wp;

          (*tcsm1_fifo_wp)++;

	  if (s->pict_type == FF_I_TYPE || (s->pict_type == FF_B_TYPE && v->bi_type)){
	    if(get_bits_count(&s->gb) > v->bits) {
	      ff_er_add_slice(s, 0, 0, s->mb_x, s->mb_y, (AC_END|DC_END|MV_END));
	      av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i\n", get_bits_count(&s->gb), v->bits);
	      //av_free(buf2);
	      return;
	    }
	  }else{
	    if (get_bits_count(&s->gb) > v->bits || get_bits_count(&s->gb) < 0) {
	      ff_er_add_slice(s, 0, 0, s->mb_x, s->mb_y, (AC_END|DC_END|MV_END));
	      av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i at %ix%i\n", get_bits_count(&s->gb), v->bits,s->mb_x,s->mb_y);
	      //av_free(buf2);
	      return;
	    }
	  }
	}
	//if(s->pict_type == FF_P_TYPE)
	//memmove(v->cbp_base, v->cbp, sizeof(v->cbp_base[0])*s->mb_stride);
	//	    ff_draw_horiz_band(s, s->mb_y * 16, 16);
	s->first_slice_line = 0;
      }
      int cnt, add_num = 3;
      for(cnt = 0; cnt < add_num + 2; cnt++){

	do{
	  tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
	  task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
	  task_fifo_wp_lh_overlap = task_fifo_wp_lh + TASK_BUF_LEN;
	} while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );

	dMB = task_fifo_wp;
	if (cnt == 1)
	  dMB->real_num = -1;
	else
	  dMB->real_num = 1;
	task_fifo_wp += (sizeof(VC1_MB_DecARGs)-(384<<1) + 3)>> 2;
	*(uint16_t *)task_fifo_wp_d2 = (sizeof(VC1_MB_DecARGs)-(384<<1) + 3) & 0xFFFC;
   
	int reach_tcsm0_end = (((unsigned int)task_fifo_wp & 0x0000FFFF)+TASK_BUF_LEN) >= 0x4000 ;
	if (reach_tcsm0_end)
	  task_fifo_wp = (int *)TCSM0_TASK_FIFO;   
	task_fifo_wp_d2 = task_fifo_wp_d1;
	task_fifo_wp_d1 = task_fifo_wp;
	(*tcsm1_fifo_wp)++;
      }

      (*tcsm1_fifo_wp)++;
      (*tcsm1_fifo_wp)++;
      (*tcsm1_fifo_wp)++;

      int tmp;
      do{
	tmp = *((volatile int *)(TCSM0_P1_TASK_DONE));
      }while (tmp == 0);
      AUX_RESET();

      ff_er_add_slice(s, 0, 0, s->mb_width - 1, s->mb_height - 1, (AC_END|DC_END|MV_END));
    }else{
      ff_er_add_slice(s, 0, 0, s->mb_width - 1, s->mb_height - 1, (AC_END|DC_END|MV_END));
      //s->first_slice_line = 1;
      for(s->mb_y = 0; s->mb_y < s->mb_height; s->mb_y++) {
	//s->mb_x = 0;
	//ff_init_block_index(s);
	//ff_update_block_index(s);
	s->dest[0] = s->current_picture.data[0] + s->mb_y * s->linesize;
	s->dest[1] = s->current_picture.data[1] + s->mb_y * s->uvlinesize;
	memcpy(s->dest[0], s->last_picture.data[0] + s->mb_y * s->linesize, s->linesize);
	memcpy(s->dest[1], s->last_picture.data[1] + s->mb_y * s->uvlinesize, s->uvlinesize);
	//memcpy(s->dest[2], s->last_picture.data[2] + s->mb_y * s->uvlinesize, s->uvlinesize);
	//	    ff_draw_horiz_band(s, s->mb_y * 16, 16);
	//s->first_slice_line = 0;
      }
      s->pict_type = FF_P_TYPE;
    }
#else
    vc1_decode_blocks(v);
#endif
    //av_log(s->avctx, AV_LOG_INFO, "Consumed %i/%i bits\n", get_bits_count(&s->gb), buf_size*8);
    //  if(get_bits_count(&s->gb) > buf_size * 8)
    //      return -1;
    ff_er_frame_end(s);
  }
  MPV_frame_end(s);

  ////////////draw edge//////////
  volatile uint8_t *tile_y, *tile_c;
  tile_y = s->current_picture.data[0];
  tile_c = s->current_picture.data[1];
#if 1
  uint32_t src_ptr,dst_ptr,src_cptr,dst_cptr,i,j,k;
  src_ptr=tile_y;
  dst_ptr=tile_y-s->linesize*2-512-32;
  src_cptr=tile_c;
  dst_cptr=tile_c-s->linesize-256-16;
  for(k=0;k<2;k++){
    S8LDD(xr5,src_ptr,0,7);
    S8LDD(xr6,src_cptr,0,7);
    S8LDD(xr7,src_cptr,8,7);

    S32LDD(xr8,src_ptr,0);
    S32LDD(xr9,src_ptr,4);
    S32LDD(xr10,src_ptr,8);
    S32LDD(xr11,src_ptr,12);
    S32LDD(xr12,src_cptr,0);
    S32LDD(xr13,src_cptr,4);
    S32LDD(xr14,src_cptr,8);
    S32LDD(xr15,src_cptr,12);
    for(i=0;i<2;i++){
      for(j=0;j<16;j++){
	S32SDI(xr5,dst_ptr,32);
	S32STD(xr5,dst_ptr,4);
	S32STD(xr5,dst_ptr,8);
	S32STD(xr5,dst_ptr,12);
	S32STD(xr5,dst_ptr,16);
	S32STD(xr5,dst_ptr,20);
	S32STD(xr5,dst_ptr,24);
	S32STD(xr5,dst_ptr,28);
      }
      for(j=0;j<8;j++){
	S32SDI(xr8,dst_ptr,32);
	S32STD(xr9,dst_ptr,4);
	S32STD(xr10,dst_ptr,8);
	S32STD(xr11,dst_ptr,12);
	S32STD(xr8,dst_ptr,16);
	S32STD(xr9,dst_ptr,20);
	S32STD(xr10,dst_ptr,24);
	S32STD(xr11,dst_ptr,28);
      }
      for(j=0;j<16;j++){
	S32SDI(xr6,dst_cptr,16);
	S32STD(xr6,dst_cptr,4);
	S32STD(xr7,dst_cptr,8);
	S32STD(xr7,dst_cptr,12);
      }
      for(j=0;j<8;j++){
	S32SDI(xr12,dst_cptr,16);
	S32STD(xr13,dst_cptr,4);
	S32STD(xr14,dst_cptr,8);
	S32STD(xr15,dst_cptr,12);
      }
      dst_ptr+=s->linesize-768;
      dst_cptr+=s->uvlinesize-384;
    }
    src_ptr=tile_y+(s->mb_height-1)*s->linesize+240;
    src_cptr=tile_c+(s->mb_height-1)*s->uvlinesize+112;
    dst_ptr=tile_y+(s->mb_height)*s->linesize-512-32;
    dst_cptr=tile_c+(s->mb_height)*s->uvlinesize-256-16;
  }
  src_ptr=tile_y+(s->mb_width-1)*256;
  dst_ptr=tile_y+(s->mb_width-1)*256-s->linesize*2-32;
  src_cptr=tile_c+(s->mb_width-1)*128;
  dst_cptr=tile_c+(s->mb_width-1)*128-s->linesize-16;
  for(k=0;k<2;k++){
    S8LDD(xr5,src_ptr,15,7);
    S8LDD(xr6,src_cptr,7,7);
    S8LDD(xr7,src_cptr,15,7);

    S32LDD(xr8,src_ptr,0);
    S32LDD(xr9,src_ptr,4);
    S32LDD(xr10,src_ptr,8);
    S32LDD(xr11,src_ptr,12);
    S32LDD(xr12,src_cptr,0);
    S32LDD(xr13,src_cptr,4);
    S32LDD(xr14,src_cptr,8);
    S32LDD(xr15,src_cptr,12);
    for(i=0;i<2;i++){
      for(j=0;j<8;j++){
	S32SDI(xr8,dst_ptr,32);
	S32STD(xr9,dst_ptr,4);
	S32STD(xr10,dst_ptr,8);
	S32STD(xr11,dst_ptr,12);
	S32STD(xr8,dst_ptr,16);
	S32STD(xr9,dst_ptr,20);
	S32STD(xr10,dst_ptr,24);
	S32STD(xr11,dst_ptr,28);
      }
      for(j=0;j<16;j++){
	S32SDI(xr5,dst_ptr,32);
	S32STD(xr5,dst_ptr,4);
	S32STD(xr5,dst_ptr,8);
	S32STD(xr5,dst_ptr,12);
	S32STD(xr5,dst_ptr,16);
	S32STD(xr5,dst_ptr,20);
	S32STD(xr5,dst_ptr,24);
	S32STD(xr5,dst_ptr,28);
      }
      for(j=0;j<8;j++){
	S32SDI(xr12,dst_cptr,16);
	S32STD(xr13,dst_cptr,4);
	S32STD(xr14,dst_cptr,8);
	S32STD(xr15,dst_cptr,12);
      }
      for(j=0;j<16;j++){
	S32SDI(xr6,dst_cptr,16);
	S32STD(xr6,dst_cptr,4);
	S32STD(xr7,dst_cptr,8);
	S32STD(xr7,dst_cptr,12);
      }
      dst_ptr+=s->linesize-768;
      dst_cptr+=s->uvlinesize-384;
    }
    src_ptr=tile_y+(s->mb_height-1)*s->linesize+(s->mb_width-1)*256+240;
    src_cptr=tile_c+(s->mb_height-1)*s->uvlinesize+(s->mb_width-1)*128+112;
    dst_ptr=tile_y+(s->mb_height)*s->linesize+(s->mb_width-1)*256-32;
    dst_cptr=tile_c+(s->mb_height)*s->uvlinesize+(s->mb_width-1)*128-16;
  }
#else
  volatile uint8_t *tile_y, *tile_c, *pxl, *edge, *tile, *dest;
  int mb_i, mb_j, i, j;
  /********************* top ***********************/    
  for(mb_i=0; mb_i<s->mb_width; mb_i++){
    // Y
    pxl = tile_y + mb_i*16*16;
    edge = pxl - s->linesize*(EDGE_WIDTH/16);
    for(j=0; j<16; j++){
      memcpy(edge, pxl, 16);
      memcpy(edge+s->linesize, pxl, 16);
      edge += 16;
    }
    // C
    pxl = tile_c + mb_i*16*8;
    edge = pxl - s->linesize;
    for(j=0; j<8; j++){
      memcpy(edge, pxl, 16);
      memcpy(edge+s->linesize/2, pxl, 16);
      edge += 16;
    }
  }
  /********************* bottom ***********************/
  for(mb_i=0; mb_i<s->mb_width; mb_i++){
    // Y
    edge = tile_y + s->mb_height*s->linesize + mb_i*16*16;
    pxl = edge - s->linesize + 16*15;
    for(j=0; j<16; j++){
      memcpy(edge, pxl, 16);
      memcpy(edge+s->linesize, pxl, 16);
      edge += 16;
    }
    // C
    edge = tile_c + s->mb_height*s->uvlinesize + mb_i*16*8;
    pxl = edge - s->uvlinesize + 16*7;
    for(j=0; j<8; j++){
      memcpy(edge, pxl, 16);
      memcpy(edge+s->uvlinesize, pxl, 16);
      edge += 16;
    }
  }

  /********************* left ***********************/
  for(mb_i=-2; mb_i<s->mb_height+2; mb_i++){
    // Y
    for(j=0; j<16; j++){
      pxl = tile_y + mb_i*s->linesize + 16*j;
      edge = pxl - 16*(EDGE_WIDTH);
      memset(edge, pxl[0], 16);
      memset(edge+16*16, pxl[0], 16);
    }
    // C
    for(j=0; j<8; j++){
      pxl = tile_c + mb_i*s->uvlinesize + 16*j;
      edge = pxl - 8*(EDGE_WIDTH);
      memset(edge, pxl[0], 8);
      memset(edge+8, pxl[8], 8);
      memset(edge+8*16, pxl[0], 8);
      memset(edge+8*16+8, pxl[8], 8);
    }
  }
  /********************* right ***********************/
  for(mb_i=-2; mb_i<s->mb_height+2; mb_i++){
    // Y
    for(j=0; j<16; j++){
      pxl = tile_y + (s->mb_width-1)*16*16 + 15 + mb_i*s->linesize + 16*j;
      edge = pxl - 15 + 16*16;
      memset(edge, pxl[0], 16);
      memset(edge+16*16, pxl[0], 16);
    }
    // C
    for(j=0; j<8; j++){
      pxl = tile_c + (s->mb_width-1)*16*8 + 7 + mb_i*s->uvlinesize + 16*j;
      edge = pxl - 7 + 16*8;
      memset(edge, pxl[0], 8);
      memset(edge+8, pxl[8], 8);
      memset(edge+8*16, pxl[0], 8);
      memset(edge+8*16+8, pxl[8], 8);
    }
  }
#endif
  ////////////draw edge end///////

  assert(s->current_picture.pict_type == s->current_picture_ptr->pict_type);
  assert(s->current_picture.pict_type == s->pict_type);
  if (s->pict_type == FF_B_TYPE || s->low_delay) {
    *pict= *(AVFrame*)s->current_picture_ptr;
  } else if (s->last_picture_ptr != NULL) {
    *pict= *(AVFrame*)s->last_picture_ptr;
  }

  if(s->last_picture_ptr || s->low_delay){
    *data_size = sizeof(AVFrame);
    ff_print_debug_info(s, pict);
  }

  //av_free(buf2);

  //------------------------JZ test area------------------------//
#ifdef JZ-IN-MODIFY
#ifdef JZC_CRC_VER
  {
    int crc_tlb;
    for(crc_tlb=0;crc_tlb<(s->mb_height);crc_tlb++)
      crc_code = crc(s->current_picture.data[0]+ crc_tlb * (s->linesize), s->mb_width*256, crc_code);
    for(crc_tlb=0;crc_tlb<(s->mb_height);crc_tlb++){
      crc_code = crc(s->current_picture.data[1] + crc_tlb * (s->uvlinesize), s->mb_width*128, crc_code);
    }
    mp_msg(NULL,NULL,"frame: %d,line mb crc_code: 0x%x@#\n", mpFrame, crc_code);

#if 0//mb print
    //dyy = s->current_picture.data[0] - 256*2 - s->linesize*2;
    //duv = s->current_picture.data[1] - 256   - s->linesize;      
    dyy = s->current_picture.data[0];
    duv = s->current_picture.data[1];
    if(mpFrame == 0){
      dy = dyy;
      dc = duv;
      for (dump_mb_y = 0; dump_mb_y < mb_height ; dump_mb_y ++ ) {
	for (dump_mb_x = 0; dump_mb_x < mb_width ; dump_mb_x ++ ) {
	  if((dump_mb_y<1||dump_mb_y<-1)&&(dump_mb_x>6||dump_mb_x<-1))
	  {
	    printf("\ndump_mb_y %d dump_mb_x %d\n",dump_mb_y,dump_mb_x);
	    for (dump_i=0; dump_i<16; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	    printf("chroma pixels\n");
	    for (dump_i=0; dump_i<8; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	  }
	  dy+=256;
	  dc+=128;
	}
	dy += 1024;
	dc += 512;
      }
    }
#endif

#ifdef JZC_ROTA90_OPTn
      int crc_90;
      linesize=mb_height*256;
      uvlinesize=linesize>>1;

      for(crc_90=0;crc_90<(mb_width);crc_90++){
	crc_code_rota = crc(rota_y + crc_90 * (linesize), linesize, crc_code_rota);
      }
      for(crc_90=0;crc_90<(mb_width);crc_90++){
	crc_code_rota = crc(rota_c + crc_90 * (uvlinesize), uvlinesize, crc_code_rota);
      }
      mp_msg(NULL,NULL,"frame: %d,rota line mb crc_code_rota: 0x%x@#\n", mpFrame, crc_code_rota);
#endif

#if 0//mb print
      if(mpFrame==0){
      dy = rota_y;
      dc = rota_c;
      for (dump_mb_y = 0; dump_mb_y < mb_width; dump_mb_y ++ ) {
	for (dump_mb_x = 0; dump_mb_x < mb_height; dump_mb_x ++ ) {
	  //if(dump_mb_y==0&&dump_mb_x==0)
	  {
	    printf("\ndump_mb_y %d dump_mb_x %d\n",dump_mb_y,dump_mb_x);
	    for (dump_i=0; dump_i<16; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	    dy+=256;
	    printf("chroma pixels\n");
	    for (dump_i=0; dump_i<8; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	    dc+=128;
	  }
	}
      }
      }
#endif

#if 0//mb crc
    dy = dyy;
    dc = duv;
    for (dump_mb_y = 0; dump_mb_y < mb_height; dump_mb_y ++ ) {
      for (dump_mb_x = 0; dump_mb_x < mb_width; dump_mb_x ++ ) {
	crc_code=0;
	crc_code = crc(dy,256, crc_code);
	printf("dump_mb_y %d dump_mb_x %d Ycrc 0x%x \n",dump_mb_y,dump_mb_x,crc_code);
	dy+=256;
      }
      dy+=1024;
    }
    for (dump_mb_y = 0; dump_mb_y < mb_height; dump_mb_y ++ ) {
      for (dump_mb_x = 0; dump_mb_x < mb_width; dump_mb_x ++ ) {
	crc_code=0;
	crc_code = crc(dc,128, crc_code);
	printf("dump_mb_y %d dump_mb_x %d Ccrc 0x%x \n",dump_mb_y,dump_mb_x,crc_code);
	dc+=128;
      }
      dc+=512;
    }
#endif
#if 0//left edge
    printf("-----------------left edge Y--------------------\n");
    for(dump_mb_y = 0; dump_mb_y < mb_height; dump_mb_y ++){
      dy=dyy-512;
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	}
	printf("edge 0x%2x\n",*(dyy+dump_i*16));
      }
      dy=dyy-256;
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      dyy+=dFRM->linesize;
      printf("-----------------\n");
    }
    printf("-----------------left edge C--------------------\n");
    for(dump_mb_y = 0; dump_mb_y < mb_height; dump_mb_y ++){
      dc=duv-256;
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	}
	printf("edge 0x%2x 0x%2x\n",*(duv+dump_i*16),*(duv+dump_i*16+8));
      }
      dc=duv-128;
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      duv+=dFRM->uvlinesize;
      printf("-----------------\n");
    }
#endif
#if 0//top edge
    dy=dyy-dFRM->linesize*2;
    printf("-----------------top edge Y--------------------\n");
    for(dump_mb_y = 0; dump_mb_y < mb_width; dump_mb_y ++){
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy + dump_mb_y*256 + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("--------------------edge-----------------------\n");
      for (dump_j = 0; dump_j < 16; dump_j ++ )
	printf("0x%2x ",*(dyy+dump_mb_y*256+dump_j));
      printf("\n");
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy +dFRM->linesize+ dump_mb_y*256 +dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("-----------------\n");
    }
    dc=duv-dFRM->linesize;
    printf("-----------------top edge C--------------------\n");
    for(dump_mb_y = 0; dump_mb_y < mb_width; dump_mb_y ++){
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc + dump_mb_y*128 + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("--------------------edge-----------------------\n");
      for (dump_j = 0; dump_j < 16; dump_j ++ )
	printf("0x%2x ",*(duv+dump_mb_y*128+dump_j));
      printf("\n");
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc +dFRM->uvlinesize+ dump_mb_y*128 +dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("-----------------\n");
    }
#endif
#if 0//right edge
    printf("-----------------right edge Y--------------------\n");
    for(dump_mb_y = 0; dump_mb_y < mb_height; dump_mb_y ++){
      dy=dyy+dFRM->linesize-1024;
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	}
	printf("edge 0x%2x\n",*(dyy+dFRM->linesize-1024-256+15+dump_i*16));
      }
      dy=dyy+dFRM->linesize-768;
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      dyy+=dFRM->linesize;
      printf("-----------------\n");
    }
    printf("-----------------right edge C--------------------\n");
    for(dump_mb_y = 0; dump_mb_y < mb_height; dump_mb_y ++){
      dc=duv+dFRM->uvlinesize-512;
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	}
	printf("edge 0x%2x 0x%2x\n",*(duv+dFRM->uvlinesize-512-128+7+dump_i*16),*(duv+dFRM->uvlinesize-512-128+15+dump_i*16));
      }
      dc=duv+dFRM->uvlinesize-384;
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      duv+=dFRM->uvlinesize;
      printf("-----------------\n");
    }
#endif
#if 0//bottom edge
    printf("-----------------bottom edge Y--------------------\n");
    dy=dyy+dFRM->linesize*mb_height;
    for(dump_mb_y = 0; dump_mb_y < mb_width; dump_mb_y ++){
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy + dump_mb_y*256 + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("--------------------edge-----------------------\n");
      for (dump_j = 0; dump_j < 16; dump_j ++ )
	printf("0x%2x ",*(dyy+dFRM->linesize*(mb_height-1)+dump_mb_y*256+240+dump_j));
      printf("\n");
      for(dump_i = 0; dump_i < 16;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dy +dFRM->linesize+ dump_mb_y*256 +dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("-----------------\n");
    }
    dc=duv+dFRM->uvlinesize*mb_height;
    printf("-----------------bottom edge C--------------------\n");
    for(dump_mb_y = 0; dump_mb_y < mb_width; dump_mb_y ++){
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc + dump_mb_y*128 + dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("--------------------edge-----------------------\n");
      for (dump_j = 0; dump_j < 16; dump_j ++ )
	printf("0x%2x ",*(duv+dFRM->uvlinesize*(mb_height-1)+dump_mb_y*128+112+dump_j));
      printf("\n");
      for(dump_i = 0; dump_i < 8;dump_i++){
	for (dump_j = 0; dump_j < 16; dump_j ++ ){
	  printf("0x%2x ",*(dc +dFRM->uvlinesize+ dump_mb_y*128 +dump_i * 16 + dump_j));
	}
	printf("\n");
      }
      printf("-----------------\n");
    }
#endif
  }
#endif//JZC_CRC_VER
#ifdef JZC_PMON_P0
    {
      int mb_num = ((s->width+15)/16)*((s->height+15)/16);
      if (vc1_pmon_p0_frmcnt==0){
	pmon_p0_fp=fopen("jz4760e_p0.pmon","aw+");
        fprintf(pmon_p0_fp, "PMON nfl:%s\t(size: %d x %d; mb_num: %d) \n",filename,s->width,s->height,mb_num);
      }

      fprintf(pmon_p0_fp,"PMON frame num: %d\n",vc1_pmon_p0_frmcnt);
      fprintf(pmon_p0_fp,"PMON VLC  -D: %d; I:%d\n",
	      vc1vlc_pmon_val/mb_num, vc1vlc_pmon_val_ex/mb_num);
      fprintf(pmon_p0_fp,"PMON WAIT -D: %d; I:%d\n",
	      vc1wait_pmon_val/mb_num, vc1wait_pmon_val_ex/mb_num);
      fprintf(pmon_p0_fp,"PMON WHILE -D: %d; I:%d\n",
	      taskwhile_pmon_val/mb_num, taskwhile_pmon_val_ex/mb_num);
      vc1vlc_pmon_val=0; vc1vlc_pmon_val_ex=0;
      vc1wait_pmon_val=0; vc1wait_pmon_val_ex=0;
      taskwhile_pmon_val=0; taskwhile_pmon_val_ex=0;
      vc1_pmon_p0_frmcnt++;
    }
#endif //JZC_PMON_P0

#ifdef JZC_PMON_P1
    {
      int mb_num = ((s->width+15)/16)*((s->height+15)/16);
      if (pmon_p1_frmcnt==0){
	pmon_p1_fp=fopen("jz4760e_p1.pmon","aw+");
	//fprintf(pmon_p1_fp, " %s\t(size: %d x %d; mb_num: %d) \n",filename,s->width,s->height,mb_num);
        fprintf(pmon_p1_fp, "PMON nfl:%s\t(size: %d x %d; mb_num: %d) \n",filename,s->width,s->height,mb_num);
	//printf(" %s\t(size: %d x %d; mb_num: %d) \n",filename,s->width,s->height,mb_num);
      }

      fprintf(pmon_p1_fp,"PMON frame num: %d  - - -\n",pmon_p1_frmcnt);
      
      fprintf(pmon_p1_fp,"PMON     MC_CFG -- cclk:%6d  piperdy:%6d  useless:%6d\n",
	      pmon_p1_ptr[12]/mb_num,pmon_p1_ptr[6]/mb_num,pmon_p1_ptr[0]/mb_num);
      fprintf(pmon_p1_fp,"PMON    MC_POLL -- cclk:%6d  piperdy:%6d  useless:%6d\n",
	      pmon_p1_ptr[13]/mb_num, pmon_p1_ptr[7]/mb_num, pmon_p1_ptr[1]/mb_num);
      fprintf(pmon_p1_fp,"PMON      IDCT  -- cclk:%6d  piperdy:%6d  useless:%6d\n",
	      pmon_p1_ptr[14]/mb_num, pmon_p1_ptr[8]/mb_num, pmon_p1_ptr[2]/mb_num);
      fprintf(pmon_p1_fp,"PMON       GP0  -- cclk:%6d  piperdy:%6d  useless:%6d\n",
	      pmon_p1_ptr[15]/mb_num, pmon_p1_ptr[9]/mb_num, pmon_p1_ptr[3]/mb_num);
      fprintf(pmon_p1_fp,"PMON       GP1  -- cclk:%6d  piperdy:%6d  useless:%6d\n",
	      pmon_p1_ptr[16]/mb_num, pmon_p1_ptr[10]/mb_num, pmon_p1_ptr[4]/mb_num);      
      fprintf(pmon_p1_fp,"PMON       GP2  -- cclk:%6d  piperdy:%6d  useless:%6d\n",
	      pmon_p1_ptr[17]/mb_num, pmon_p1_ptr[11]/mb_num, pmon_p1_ptr[5]/mb_num);
      pmon_p1_frmcnt ++;

    }
#endif //JZC_PMON_P1


  mpFrame++;
#endif//JZ-IN-MODIFY
  jz_dcache_wb();
  //exit_player_with_rc();
  return buf_size;
}


/** Close a VC1/WMV3 decoder
 * @warning Initial try at using MpegEncContext stuff
 */
static av_cold int vc1_decode_end(AVCodecContext *avctx)
{
#ifdef JZC_PMON_P1
  if(pmon_p1_fp!=0){
    fprintf(pmon_p1_fp,"PMON frame num: %d  - - -\n",pmon_p1_frmcnt);
    fclose(pmon_p1_fp);
  }
#endif

  VC1Context *v = avctx->priv_data;
  av_freep(&v->hrd_rate);
  av_freep(&v->hrd_buffer);
  MPV_common_end(&v->s);
#ifdef JZ_LINUX_OS
#define av_freep(a)
#endif
  av_freep(&v->mv_type_mb_plane);
  av_freep(&v->direct_mb_plane);
  av_freep(&v->acpred_plane);
  av_freep(&v->over_flags_plane);
  av_freep(&v->mb_type_base);
  av_freep(&v->cbp_base);
#ifdef JZ_LINUX_OS
#undef av_freep(a)
#endif
  ff_intrax8_common_end(&v->x8);
  use_jz_buf=0;
  return 0;
}

void vc1_fluash_buffer(MpegEncContext *s)
{
  int i;
    
  for(i=0; i<MAX_PICTURE_COUNT ; i++){
    if(s->picture[i].data[0])
      s->avctx->release_buffer(s->avctx, (AVFrame*)&s->picture[i]);
  }
}

static void flush_dpb(AVCodecContext *avctx){
  VC1Context *h= avctx->priv_data;
  MpegEncContext *s = &h->s;
  int i;
  if(s->current_picture_ptr)
    s->current_picture_ptr = NULL;
  if(s->last_picture_ptr)
    s->last_picture_ptr = NULL;
  if(s->next_picture_ptr)
    s->next_picture_ptr = NULL;
   
  vc1_fluash_buffer(s);
  for(i=0; i<MAX_PICTURE_COUNT; i++)
    s->picture[i].reference = 0;
    
}


AVCodec vc1_decoder = {
  "vc1",
  AVMEDIA_TYPE_VIDEO,
  CODEC_ID_VC1,
  sizeof(VC1Context),
  vc1_decode_init,
  NULL,
  vc1_decode_end,
  vc1_decode_frame,
  CODEC_CAP_DR1 | CODEC_CAP_DELAY,
  //NULL,
  .flush = flush_dpb,
  .long_name = NULL_IF_CONFIG_SMALL("SMPTE VC-1"),
  .pix_fmts = ff_hwaccel_pixfmt_list_420
};

#if CONFIG_WMV3_DECODER
AVCodec wmv3_decoder = {
  "wmv3",
  AVMEDIA_TYPE_VIDEO,
  CODEC_ID_WMV3,
  sizeof(VC1Context),
  vc1_decode_init,
  NULL,
  vc1_decode_end,
  vc1_decode_frame,
  CODEC_CAP_DR1 | CODEC_CAP_DELAY,
  //NULL,
  .flush = flush_dpb,
  .long_name = NULL_IF_CONFIG_SMALL("Windows Media Video 9"),
  .pix_fmts = ff_hwaccel_pixfmt_list_420
};
#endif

#if CONFIG_WMV3_VDPAU_DECODER
AVCodec wmv3_vdpau_decoder = {
  "wmv3_vdpau",
  AVMEDIA_TYPE_VIDEO,
  CODEC_ID_WMV3,
  sizeof(VC1Context),
  vc1_decode_init,
  NULL,
  vc1_decode_end,
  vc1_decode_frame,
  CODEC_CAP_DR1 | CODEC_CAP_DELAY | CODEC_CAP_HWACCEL_VDPAU,
  //NULL,
  .flush = flush_dpb,
  .long_name = NULL_IF_CONFIG_SMALL("Windows Media Video 9 VDPAU"),
  .pix_fmts = (const enum PixelFormat[]){PIX_FMT_VDPAU_WMV3, PIX_FMT_NONE}
};
#endif

#if CONFIG_VC1_VDPAU_DECODER
AVCodec vc1_vdpau_decoder = {
  "vc1_vdpau",
  AVMEDIA_TYPE_VIDEO,
  CODEC_ID_VC1,
  sizeof(VC1Context),
  vc1_decode_init,
  NULL,
  vc1_decode_end,
  vc1_decode_frame,
  CODEC_CAP_DR1 | CODEC_CAP_DELAY | CODEC_CAP_HWACCEL_VDPAU,
  //NULL,
  .flush = flush_dpb,
  .long_name = NULL_IF_CONFIG_SMALL("SMPTE VC-1 VDPAU"),
  .pix_fmts = (const enum PixelFormat[]){PIX_FMT_VDPAU_VC1, PIX_FMT_NONE}
};
#endif
