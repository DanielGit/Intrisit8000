/*
 * H.263 decoder
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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
 * H.263 decoder.
 */

#include "libavutil/cpu.h"
#include "internal.h"
#include "avcodec.h"
#include "dsputil.h"
#include "mpegvideo.h"
#include "h263.h"
#include "h263_parser.h"
#include "mpeg4video_parser.h"
#include "msmpeg4.h"
#include "vdpau_internal.h"
#include "flv.h"
#include "mpeg4video.h"
#include "mpeg4.h"

#include "jzsoc/jzmedia.h"
#include "jzsoc/jzasm.h"

#ifdef JZC_P1_OPT
#include "jzsoc/jzmedia.h"
#include "jzsoc/mpeg4_dcore.h"

#include "jzsoc/jzasm.h"
#include "jzsoc/mpeg4_tcsm0.h"
#include "jzsoc/mpeg4_tcsm1.h"
#include "../libjzcommon/jz4760e_dcsc.h"
//#include "../libjzcommon/jz4760e_tcsm_init.c"
#include "jzsoc/jz4760_tcsm_init.c"

//mpeg4 motion
#include "jzsoc/t_motion.h"
#include "jzsoc/t_intpid.h"
#include "jzsoc/t_vputlb.h"

#ifdef JZC_PMON_P0
#include "../libjzcommon/jz4760e_pmon.h"
#endif
#include "../libjzcommon/jzasm.h"

MPEG4_MB_DecARGs *dMB;
MPEG4_Frame_GlbARGs *dFRM;
#endif

#ifdef  JZC_TCSM_OPT
#include "jzsoc/mpeg4_tcsm1.h"
extern unsigned char *frame_buffer;
extern unsigned int phy_fb;
#endif

#ifdef JZC_DCORE_OPT
uint32_t current_picture_ptr[3];
uint8_t *emu_edge_buf_aux; 
//#include "../libjzcommon/jz4760e_2ddma_hw.h"
#include "jzsoc/jz4760_2ddma_hw.h"
#endif //JZC_DCORE_OPT

#ifdef JZC_P1_OPT
extern char * dcore_sh_buf;
volatile int * task_fifo_wp;
volatile int * task_fifo_wp_d1;
volatile int * task_fifo_wp_d2;
volatile int * tcsm1_fifo_wp;
volatile int * tcsm0_fifo_rp;
#endif

#ifdef JZC_CRC_VER
# undef   fprintf
# undef   printf
# include "crc.c"
short crc_code;
short mpFrame;
#else
#undef printf
#endif

//extern uint8_t val[6];

#ifdef JZC_VLC_HW_OPT
volatile char * vc1_sw_bs_buffer;
volatile char * vc1_hw_bs_buffer;
//int vc1_hw_codingset1;
//int vc1_hw_codingset2;
#include "vlc_hw.h"
#include "mpeg4_table.h"
#endif
extern int use_jz_buf;

#ifdef JZC_PMON_P0MB
PMON_CREAT(p0wt);
PMON_CREAT(dcmb);
PMON_CREAT(bfds);
PMON_CREAT(afds);
PMON_CREAT(mcfg);
PMON_CREAT(axst);
PMON_CREAT(inbl);
PMON_CREAT(dhbd);
PMON_CREAT(upbl);
PMON_CREAT(upmv);
PMON_CREAT(jzmb);
PMON_CREAT(gtmb);
PMON_CREAT(tskf);
PMON_CREAT(lpft);
int p0wt,dcmb,inbl,dhbd,upbl,upmv,gtmb,jzmb,tskf,lpft;
#endif

#ifdef JZC_PMON_P0FRM
static long long pmon_val = 0;
static long long pmon_val1 = 0;
static int pmon_cnt = 0;
PMON_CREAT(p0frm1);
PMON_CREAT(p0frm2);
#endif

#ifdef JZC_PMON_P1FRM
static long long p1_pmon = 0;
static int p1_pmon_cnt = 0;;
#endif

#ifdef JZC_PMON_ALLFRM
PMON_CREAT(allfrm);
static long long pmon_all = 0;
static int all_pmon_cnt = 0;
#endif

#ifdef JZC_ROTA90_OPT
short rota_crc;
#endif

#ifdef JZC_P1_OPT
static void mpeg4_motion_set(int intpid, int cintpid){
  int i;

  for(i=0; i<16; i++){
    SET_TAB1_ILUT(i,/*idx*/
		  IntpFMT[intpid][i].intp[1],/*intp2*/
		  IntpFMT[intpid][i].intp_pkg[1],/*intp2_pkg*/
		  IntpFMT[intpid][i].hldgl,/*hldgl*/
		  IntpFMT[intpid][i].avsdgl,/*avsdgl*/
		  IntpFMT[intpid][i].intp_dir[1],/*intp2_dir*/
		  IntpFMT[intpid][i].intp_rnd[1],/*intp2_rnd*/
		  IntpFMT[intpid][i].intp_sft[1],/*intp2_sft*/
		  IntpFMT[intpid][i].intp_sintp[1],/*sintp2*/
		  IntpFMT[intpid][i].intp_srnd[1],/*sintp2_rnd*/
		  IntpFMT[intpid][i].intp_sbias[1],/*sintp2_bias*/
		  IntpFMT[intpid][i].intp[0],/*intp1*/
		  IntpFMT[intpid][i].tap,/*tap*/
		  IntpFMT[intpid][i].intp_pkg[0],/*intp1_pkg*/
		  IntpFMT[intpid][i].intp_dir[0],/*intp1_dir*/
		  IntpFMT[intpid][i].intp_rnd[0],/*intp1_rnd*/
		  IntpFMT[intpid][i].intp_sft[0],/*intp1_sft*/
		  IntpFMT[intpid][i].intp_sintp[0],/*sintp1*/
		  IntpFMT[intpid][i].intp_srnd[0],/*sintp1_rnd*/
		  IntpFMT[intpid][i].intp_sbias[0]/*sintp1_bias*/
		  );
    SET_TAB1_CLUT(i,/*idx*/
		  IntpFMT[intpid][i].intp_coef[0][7],/*coef8*/
		  IntpFMT[intpid][i].intp_coef[0][6],/*coef7*/
		  IntpFMT[intpid][i].intp_coef[0][5],/*coef6*/
		  IntpFMT[intpid][i].intp_coef[0][4],/*coef5*/
		  IntpFMT[intpid][i].intp_coef[0][3],/*coef4*/
		  IntpFMT[intpid][i].intp_coef[0][2],/*coef3*/
		  IntpFMT[intpid][i].intp_coef[0][1],/*coef2*/
		  IntpFMT[intpid][i].intp_coef[0][0] /*coef1*/
		  );
    SET_TAB1_CLUT(16+i,/*idx*/
		  IntpFMT[intpid][i].intp_coef[1][7],/*coef8*/
		  IntpFMT[intpid][i].intp_coef[1][6],/*coef7*/
		  IntpFMT[intpid][i].intp_coef[1][5],/*coef6*/
		  IntpFMT[intpid][i].intp_coef[1][4],/*coef5*/
		  IntpFMT[intpid][i].intp_coef[1][3],/*coef4*/
		  IntpFMT[intpid][i].intp_coef[1][2],/*coef3*/
		  IntpFMT[intpid][i].intp_coef[1][1],/*coef2*/
		  IntpFMT[intpid][i].intp_coef[1][0] /*coef1*/
		  );

    SET_TAB2_ILUT(i,/*idx*/
		  IntpFMT[cintpid][i].intp[1],/*intp2*/
		  IntpFMT[cintpid][i].intp_dir[1],/*intp2_dir*/
		  IntpFMT[cintpid][i].intp_sft[1],/*intp2_sft*/
		  IntpFMT[cintpid][i].intp_coef[1][0],/*intp2_lcoef*/
		  IntpFMT[cintpid][i].intp_coef[1][1],/*intp2_rcoef*/
		  IntpFMT[cintpid][i].intp_rnd[1],/*intp2_rnd*/
		  IntpFMT[cintpid][i].intp[0],/*intp1*/
		  IntpFMT[cintpid][i].intp_dir[0],/*intp1_dir*/
		  IntpFMT[cintpid][i].intp_sft[0],/*intp1_sft*/
		  IntpFMT[cintpid][i].intp_coef[0][0],/*intp1_lcoef*/
		  IntpFMT[cintpid][i].intp_coef[0][1],/*intp1_rcoef*/
		  IntpFMT[cintpid][i].intp_rnd[0]/*intp1_rnd*/
		  );
  }
}

static void mpeg4_motion_init(int intpid, int cintpid)
{
  int i;
  for(i=0; i<16; i++){
    SET_TAB1_ILUT(i,/*idx*/
		  IntpFMT[intpid][i].intp[1],/*intp2*/
		  IntpFMT[intpid][i].intp_pkg[1],/*intp2_pkg*/
		  IntpFMT[intpid][i].hldgl,/*hldgl*/
		  IntpFMT[intpid][i].avsdgl,/*avsdgl*/
		  IntpFMT[intpid][i].intp_dir[1],/*intp2_dir*/
		  IntpFMT[intpid][i].intp_rnd[1],/*intp2_rnd*/
		  IntpFMT[intpid][i].intp_sft[1],/*intp2_sft*/
		  IntpFMT[intpid][i].intp_sintp[1],/*sintp2*/
		  IntpFMT[intpid][i].intp_srnd[1],/*sintp2_rnd*/
		  IntpFMT[intpid][i].intp_sbias[1],/*sintp2_bias*/
		  IntpFMT[intpid][i].intp[0],/*intp1*/
		  IntpFMT[intpid][i].tap,/*tap*/
		  IntpFMT[intpid][i].intp_pkg[0],/*intp1_pkg*/
		  IntpFMT[intpid][i].intp_dir[0],/*intp1_dir*/
		  IntpFMT[intpid][i].intp_rnd[0],/*intp1_rnd*/
		  IntpFMT[intpid][i].intp_sft[0],/*intp1_sft*/
		  IntpFMT[intpid][i].intp_sintp[0],/*sintp1*/
		  IntpFMT[intpid][i].intp_srnd[0],/*sintp1_rnd*/
		  IntpFMT[intpid][i].intp_sbias[0]/*sintp1_bias*/
		  );
    SET_TAB1_CLUT(i,/*idx*/
		  IntpFMT[intpid][i].intp_coef[0][7],/*coef8*/
		  IntpFMT[intpid][i].intp_coef[0][6],/*coef7*/
		  IntpFMT[intpid][i].intp_coef[0][5],/*coef6*/
		  IntpFMT[intpid][i].intp_coef[0][4],/*coef5*/
		  IntpFMT[intpid][i].intp_coef[0][3],/*coef4*/
		  IntpFMT[intpid][i].intp_coef[0][2],/*coef3*/
		  IntpFMT[intpid][i].intp_coef[0][1],/*coef2*/
		  IntpFMT[intpid][i].intp_coef[0][0] /*coef1*/
		  );
    SET_TAB1_CLUT(16+i,/*idx*/
		  IntpFMT[intpid][i].intp_coef[1][7],/*coef8*/
		  IntpFMT[intpid][i].intp_coef[1][6],/*coef7*/
		  IntpFMT[intpid][i].intp_coef[1][5],/*coef6*/
		  IntpFMT[intpid][i].intp_coef[1][4],/*coef5*/
		  IntpFMT[intpid][i].intp_coef[1][3],/*coef4*/
		  IntpFMT[intpid][i].intp_coef[1][2],/*coef3*/
		  IntpFMT[intpid][i].intp_coef[1][1],/*coef2*/
		  IntpFMT[intpid][i].intp_coef[1][0] /*coef1*/
		  );

    SET_TAB2_ILUT(i,/*idx*/
		  IntpFMT[cintpid][i].intp[1],/*intp2*/
		  IntpFMT[cintpid][i].intp_dir[1],/*intp2_dir*/
		  IntpFMT[cintpid][i].intp_sft[1],/*intp2_sft*/
		  IntpFMT[cintpid][i].intp_coef[1][0],/*intp2_lcoef*/
		  IntpFMT[cintpid][i].intp_coef[1][1],/*intp2_rcoef*/
		  IntpFMT[cintpid][i].intp_rnd[1],/*intp2_rnd*/
		  IntpFMT[cintpid][i].intp[0],/*intp1*/
		  IntpFMT[cintpid][i].intp_dir[0],/*intp1_dir*/
		  IntpFMT[cintpid][i].intp_sft[0],/*intp1_sft*/
		  IntpFMT[cintpid][i].intp_coef[0][0],/*intp1_lcoef*/
		  IntpFMT[cintpid][i].intp_coef[0][1],/*intp1_rcoef*/
		  IntpFMT[cintpid][i].intp_rnd[0]/*intp1_rnd*/
		  );
  }

  SET_REG1_STAT(1,/*pfe*/
		1,/*lke*/
		1 /*tke*/);
  SET_REG2_STAT(1,/*pfe*/
		1,/*lke*/
		1 /*tke*/);
  SET_REG1_CTRL(0,/*esms*/
		0,/*esa*/
		0,/*esmd*/
		0,/*csf*/
		0,/*cara*/
		0,/*cae*/
		0,/*crpm*/
		0xF,/*pgc*/
		1, /*pri*/
		0,/*ckge*/
		0,/*ofa*/
		0,/*rot*/
		USE_TDD,/*ddm*/
		0,/*wm*/
		1,/*ccf*/
		0,/*csl*/
		0,/*rst*/
		1 /*en*/);

  SET_REG1_BINFO(AryFMT[intpid],/*ary*/
		 0,/*doe*/
		 0,/*expdy*/
		 0,/*expdx*/
		 0,/*ilmd*/
		 SubPel[intpid]-1,/*pel*/
		 0,/*fld*/
		 0,/*fldsel*/
		 0,/*boy*/
		 0,/*box*/
		 0,/*bh*/
		 0,/*bw*/
		 0/*pos*/);
  SET_REG2_BINFO(0,/*ary*/
		 0,/*doe*/
		 0,/*expdy*/
		 0,/*expdx*/
		 0,/*ilmd*/
		 0,/*pel*/
		 0,/*fld*/
		 0,/*fldsel*/
		 0,/*boy*/
		 0,/*box*/
		 0,/*bh*/
		 0,/*bw*/
		 0/*pos*/);
}

static void motion_config_mpeg4(MpegEncContext *s)
{
  int i, j;

  SET_REG1_CTRL(0,/*esms*/
		0,/*esa*/
		0,/*esmd*/
		0,/*csf*/
		0,/*cara*/
		0,/*cae*/
		0,/*crpm*/
		0xF,/*pgc*/
		1, /*pri*/
		0,/*ckge*/
		0,/*ofa*/
		0,/*rot*/
		USE_TDD,/*ddm*/
		0,/*wm*/
		1,/*ccf*/
		0,/*csl*/
		0,/*rst*/
		1 /*en*/);

  SET_TAB1_RLUT(0, get_phy_addr((int)(s->last_picture.data[0])), 0, 0);
  SET_TAB1_RLUT(16, get_phy_addr((int)(s->next_picture.data[0])), 0, 0);
  SET_TAB2_RLUT(0, get_phy_addr((int)(s->last_picture.data[1])), 0, 0, 0, 0);
  SET_TAB2_RLUT(16, get_phy_addr((int)(s->next_picture.data[1])), 0, 0, 0, 0);

  //0x20
  SET_REG1_PINFO(0,/*rgr*/
		 0,/*its*/
		 6,/*its_sft*/
		 0/*v->its_scale*/,/*its_scale*/
		 0/*v->its_rnd_y*//*its_rnd*/);
  SET_REG2_PINFO(0,/*rgr*/
		 0,/*its*/
		 6,/*its_sft*/
		 0/*v->its_scale*/,/*its_scale*/
		 0/*v->its_rnd_c*//*its_rnd*/);

  //0x24
  SET_REG1_WINFO(0,/*wt*/
		 0, /*wtpd*/
		 IS_BIAVG,/*wtmd*/
		 1,/*biavg_rnd*/
		 0,/*wt_denom*/
		 0,/*wt_sft*/
		 0,/*wt_lcoef*/
		 0/*wt_rcoef*/);
  //0x2c
  SET_REG1_WTRND(0);


  SET_REG2_WINFO1(0,/*wt*/
		  0, /*wtpd*/
		  IS_BIAVG,/*wtmd*/
		  1,/*biavg_rnd*/
		  0,/*wt_denom*/
		  0,/*wt_sft*/
		  0,/*wt_lcoef*/
		  0/*wt_rcoef*/);
  SET_REG2_WINFO2(0,/*wt_sft*/
		  0,/*wt_lcoef*/
		  0/*wt_rcoef*/);

  SET_REG2_WTRND(0, 0);
  SET_REG1_STRD(s->linesize/16,0,DOUT_Y_STRD);
  SET_REG1_GEOM(s->mb_height*16,s->mb_width*16);
  SET_REG2_STRD(s->linesize/16,0,DOUT_C_STRD);
}

void mpeg4_p1_jumpto_main()
{
  i_jr(MPEG4_P1_MAIN);
}
#endif

av_cold int mpeg4_decode_end(AVCodecContext *avctx)
{
  MpegEncContext *s = avctx->priv_data;

  MPV_common_end(s);
  use_jz_buf=0;
  return 0;
}

static void linear_crc_frame(MpegEncContext *s){
#ifdef JZC_CRC_VER
  int crc_i;
  int crc_j = 0;

  char *ptr_y = s->current_picture.data[0];
  char *ptr_c = s->current_picture.data[1];

  //last_crc_code = crc_code;

  for(crc_i=0; crc_i<s->mb_height; crc_i++){
    for (crc_j = 0; crc_j < s->mb_width; crc_j++){
      crc_code = crc(ptr_y, 256, crc_code);
      ptr_y += 256;
    }
    ptr_y += 1024;
  }

  for(crc_i=0; crc_i<s->mb_height; crc_i++){
    for (crc_j = 0; crc_j < s->mb_width; crc_j++){
      crc_code = crc(ptr_c, 128, crc_code);
      ptr_c += 128;
    }
    ptr_c += 512;
  }

  //mpFrame++;

  printf("frame: %d, crc_code: 0x%x\n", mpFrame, crc_code);
#endif
}

static void rota_crc_frame(MpegEncContext *s){
#ifdef JZC_CRC_VER
#ifdef JZC_ROTA90_OPT
  int crc_i;
  int crc_j = 0;

  char *ptr_y = s->current_picture.data[2];
  char *ptr_c = s->current_picture.data[3];

  for(crc_i=0;crc_i<s->mb_width;crc_i++){
    rota_crc = crc(ptr_y + crc_i * 256 * s->mb_height,
        256 * s->mb_height, rota_crc);
  }

  for (crc_i = 0; crc_i < s->mb_width; crc_i++){
    rota_crc = crc(ptr_c + crc_i * 128 * s->mb_height,
        128 * s->mb_height, rota_crc);
  }

  printf("frame: %d, rota_crc: 0x%x\n", mpFrame, rota_crc);
#endif
#endif
}

/**
 * returns the number of bytes consumed for building the current frame
 */
static int get_consumed_bytes(MpegEncContext *s, int buf_size){
  int pos= (get_bits_count(&s->gb)+7)>>3;

  if(s->divx_packed || s->avctx->hwaccel){
    //we would have to scan through the whole buf to handle the weird reordering ...
    return buf_size;
  }else if(s->flags&CODEC_FLAG_TRUNCATED){
    pos -= s->parse_context.last_index;
    if(pos<0) pos=0; // padding is not really read so this might be -1
    return pos;
  }else{
    if(pos==0) pos=1; //avoid infinite loops (i doubt that is needed but ...)
    if(pos+10>buf_size) pos=buf_size; // oops ;)

    return pos;
  }
}

#if 1 //#ifdef JZ_LINUX_OS
extern unsigned int mpeg4_auxcodes_len, mpeg4_aux_task_codes[];
#endif

av_cold int mpeg4_decode_init(AVCodecContext *avctx)
{       
  MpegEncContext *s = avctx->priv_data;

#ifdef JZC_CRC_VER
  mpFrame = 0;
  crc_code = 0;
#endif

#ifdef JZC_ROTA90_OPT
  rota_crc = 0;
#endif

#ifdef JZC_MXU_OPT
  S32I2M(xr16, 0x3);
#endif

#ifdef JZC_PMON_P0MB
  i_mtc0_2(0, 16, 7);
#endif

#ifdef JZC_P1_OPT
  mpeg4_motion_init(MPEG_QPEL, MPEG_QPEL);
  //printf("mpeg4_motion_init successful\n");
#endif

#ifdef JZC_TCSM_OPT
  AUX_RESET();  
  //printf("mpeg4_decode_init AUX_RESET end\n");
  tcsm_init();
#endif
  //printf("mpeg4_decode_init tcsm_init end\n");
#ifdef JZC_P1_OPT
  dFRM=TCSM1_VUCADDR(TCSM1_DFRM_BUF);
  int * tmp_hm_buf = jz4740_alloc_frame(32, SPACE_HALF_MILLION_BYTE);
  if (tmp_hm_buf < 0)
    printf("JZ4740 ALLOC tmp_hm_buf ERROR !! \n");

  {
    int i, tmp;
    int *src, *dst;
    int len, *reserved_mem;

    *((volatile int *)(TCSM0_P1_TASK_DONE)) = 0;

    //several setup instructions direct p1 to execute p1_boot
    src = (int *)mpeg4_p1_jumpto_main;
    dst=(int*)(TCSM1_BOOT_CODE);
    for (i=0; i<64; i++) dst[i] = src[i];

#if 1 //#ifdef JZ_LINUX_OS
      // load p1 insn and data to reserved mem
      printf("mpeg4 len of aux task = %d\n", mpeg4_auxcodes_len);
      reserved_mem = (int *)TCSM1_VCADDR(MPEG4_P1_MAIN);
      for (i=0; i< mpeg4_auxcodes_len/4; i++)
        reserved_mem[i] = mpeg4_aux_task_codes[i];
      printf("reserved_mem addr = 0x%08x, *reserved_mem = 0x%08x\n",reserved_mem, *reserved_mem);
#else
    // load p1 insn and data to reserved mem
    FILE *fp_text;
    fp_text = fopen("mpeg4_p1.bin", "r+b");
    if (!fp_text)
      printf(" error while open mpeg4_p1.bin \n");
    int *load_buf = tmp_hm_buf;
    len = fread(load_buf, 4, SPACE_HALF_MILLION_BYTE, fp_text);
    //printf(" mpeg4 len of p1 task = %d\n",len);

    reserved_mem = (int *)TCSM1_VCADDR(MPEG4_P1_MAIN);
    for(i=0; i<len; i++)
      reserved_mem[i] = load_buf[i];

    fclose(fp_text);
#endif
    jz_dcache_wb(); /*flush cache into reserved mem*/
    i_sync();
  }

  *((volatile int *)(TCSM0_P0_POLL)) = 1;
  *((volatile int *)(TCSM0_P1_POLL)) = 1;
#endif

#ifdef JZC_VLC_HW_OPT
  SET_GLOBAL_CTRL(0x10);
  { // malloc a fb for bs
#define VC1_HW_BS_SIZE (1<<20) // 4M BYTE
    vc1_sw_bs_buffer = jz4740_alloc_frame(0,VC1_HW_BS_SIZE);

    vc1_hw_bs_buffer = get_phy_addr((unsigned int)vc1_sw_bs_buffer);

    SET_VPU_TLB(4,1,PSIZE_8M,
		(((unsigned int)vc1_hw_bs_buffer) >> 22),
		(((unsigned int)vc1_hw_bs_buffer) >> 22)
		);
  }
#endif

  s->avctx = avctx;
  s->out_format = FMT_H263;

  s->width  = avctx->coded_width;
  s->height = avctx->coded_height;
  s->workaround_bugs= avctx->workaround_bugs;

  // set defaults
  MPV_decode_defaults(s);
  s->quant_precision=5;
  s->decode_mb= ff_h263_decode_mb;
  s->low_delay= 1;
  avctx->pix_fmt= avctx->get_format(avctx, avctx->codec->pix_fmts);
  s->unrestricted_mv= 1;

  /* select sub codec */
  switch(avctx->codec->id) {
  case CODEC_ID_H263:
    s->unrestricted_mv= 0;
    avctx->chroma_sample_location = AVCHROMA_LOC_CENTER;
    break;
  case CODEC_ID_MPEG4:
    break;
  case CODEC_ID_MSMPEG4V1:
    s->h263_msmpeg4 = 1;
    s->h263_pred = 1;
    s->msmpeg4_version=1;
    break;
  case CODEC_ID_MSMPEG4V2:
    s->h263_msmpeg4 = 1;
    s->h263_pred = 1;
    s->msmpeg4_version=2;
    break;
  case CODEC_ID_MSMPEG4V3:
    s->h263_msmpeg4 = 1;
    s->h263_pred = 1;
    s->msmpeg4_version=3;
    break;
  case CODEC_ID_WMV1:
    s->h263_msmpeg4 = 1;
    s->h263_pred = 1;
    s->msmpeg4_version=4;
    break;
  case CODEC_ID_WMV2:
    s->h263_msmpeg4 = 1;
    s->h263_pred = 1;
    s->msmpeg4_version=5;
    break;
  case CODEC_ID_VC1:
  case CODEC_ID_WMV3:
    s->h263_msmpeg4 = 1;
    s->h263_pred = 1;
    s->msmpeg4_version=6;
    avctx->chroma_sample_location = AVCHROMA_LOC_LEFT;
    break;
  case CODEC_ID_H263I:
    break;
  case CODEC_ID_FLV1:
    s->h263_flv = 1;
    break;
  default:
    return -1;
  }
  s->codec_id= avctx->codec->id;
  avctx->hwaccel= ff_find_hwaccel(avctx->codec->id, avctx->pix_fmt);

  /* for h263, we allocate the images after having read the header */
  if (avctx->codec->id != CODEC_ID_H263 && avctx->codec->id != CODEC_ID_MPEG4)
    if (MPV_common_init(s) < 0)
      return -1;

  mpeg4_decode_init_vlc(s);
  use_jz_buf=1;

  return 0;
}

static inline void JZC_clean_intra_table_entries(MpegEncContext *s)
{
  int wrap = s->b8_stride;
  int xy = s->block_index[0];
  int addr,i;

  s->dc_val[0][xy           ] =
    s->dc_val[0][xy + 1       ] =
    s->dc_val[0][xy     + wrap] =
    s->dc_val[0][xy + 1 + wrap] = 1024;
  /* ac pred */
#if 1
  addr = s->ac_val[0][xy];
  i_pref(30, addr, 0);
  S32STD(xr0, addr, 0);
  S32STD(xr0, addr, 4);
  S32STD(xr0, addr, 8);
  S32STD(xr0, addr, 12);
  S32STD(xr0, addr, 16);
  S32STD(xr0, addr, 20);
  S32STD(xr0, addr, 24);
  S32STD(xr0, addr, 28);
  addr = s->ac_val[0][xy+1];
  i_pref(30, addr, 0);
  S32STD(xr0, addr, 0);
  S32STD(xr0, addr, 4);
  S32STD(xr0, addr, 8);
  S32STD(xr0, addr, 12);
  S32STD(xr0, addr, 16);
  S32STD(xr0, addr, 20);
  S32STD(xr0, addr, 24);
  S32STD(xr0, addr, 28);

  addr = s->ac_val[0][xy + wrap];
  i_pref(30, addr, 0);
  S32STD(xr0, addr, 0);
  S32STD(xr0, addr, 4);
  S32STD(xr0, addr, 8);
  S32STD(xr0, addr, 12);
  S32STD(xr0, addr, 16);
  S32STD(xr0, addr, 20);
  S32STD(xr0, addr, 24);
  S32STD(xr0, addr, 28);
  addr = s->ac_val[0][xy + wrap + 1];
  i_pref(30, addr, 0);
  S32STD(xr0, addr, 0);
  S32STD(xr0, addr, 4);
  S32STD(xr0, addr, 8);
  S32STD(xr0, addr, 12);
  S32STD(xr0, addr, 16);
  S32STD(xr0, addr, 20);
  S32STD(xr0, addr, 24);
  S32STD(xr0, addr, 28);
#else
  memset(s->ac_val[0][xy       ], 0, 32 * sizeof(int16_t));
  memset(s->ac_val[0][xy + wrap], 0, 32 * sizeof(int16_t));
#endif
  if (s->msmpeg4_version>=3) {
    s->coded_block[xy           ] =
      s->coded_block[xy + 1       ] =
      s->coded_block[xy     + wrap] =
      s->coded_block[xy + 1 + wrap] = 0;
  }
  /* chroma */
  wrap = s->mb_stride;
  xy = s->mb_x + s->mb_y * wrap;
  s->dc_val[1][xy] =
    s->dc_val[2][xy] = 1024;
  /* ac pred */
#if 1
  addr = s->ac_val[1][xy];
  i_pref(30, addr, 0);
  S32STD(xr0, addr, 0);
  S32STD(xr0, addr, 4);
  S32STD(xr0, addr, 8);
  S32STD(xr0, addr, 12);
  S32STD(xr0, addr, 16);
  S32STD(xr0, addr, 20);
  S32STD(xr0, addr, 24);
  S32STD(xr0, addr, 28);

  addr = s->ac_val[2][xy];
  i_pref(30, addr, 0);
  S32STD(xr0, addr, 0);
  S32STD(xr0, addr, 4);
  S32STD(xr0, addr, 8);
  S32STD(xr0, addr, 12);
  S32STD(xr0, addr, 16);
  S32STD(xr0, addr, 20);
  S32STD(xr0, addr, 24);
  S32STD(xr0, addr, 28);
  //jz_dcache_wb();
#else
  memset(s->ac_val[1][xy], 0, 16 * sizeof(int16_t));
  memset(s->ac_val[2][xy], 0, 16 * sizeof(int16_t));
#endif

  s->mbintra_table[xy]= 0;
}

static inline void JZC_decode_mb(MpegEncContext *s){
  const int mb_xy = s->mb_y * s->mb_stride + s->mb_x;
  uint8_t *mbskip_ptr = &s->mbskip_table[mb_xy];
  const int age = s->current_picture.age;

  s->current_picture.qscale_table[mb_xy] = s->qscale;

  if (!s->mb_intra){
    if (s->h263_pred || s->h263_aic){
      if (s->mbintra_table[mb_xy]){
	JZC_clean_intra_table_entries(s);
	//s->mbintra_table[mb_xy]= 0;
      }
    }else{
      s->last_dc[0]=
	s->last_dc[1]=
	s->last_dc[2]= 128 << s->intra_dc_precision;
    }
  }
  else if (s->h263_pred || s->h263_aic){
    s->mbintra_table[mb_xy]=1;
  }

  if(1){
    assert(age);
    //printf("mb_skipped %d, reference %d\n", s->mb_skipped, s->current_picture.reference);
    if (s->mb_skipped) {
      s->mb_skipped= 0;
      assert(s->pict_type!=I_TYPE);

      (*mbskip_ptr) ++; /* indicate that this time we skipped it */
      if(*mbskip_ptr >99) 
	*mbskip_ptr= 99;

    } else if(!s->current_picture.reference){
      (*mbskip_ptr) ++; /* increase counter so the age can be compared cleanly */
      if(*mbskip_ptr >99) 
	*mbskip_ptr= 99;
    } else{
      *mbskip_ptr = 0; /* not skipped */
    }
  }
  return;
}

static int get_GlbARGs(MpegEncContext *s)
{
  int i = 0,j=0;
  dFRM->flags = s->avctx->flags;
  dFRM->intra_only = s->intra_only;
  dFRM->pict_type = s->pict_type;
  dFRM->mb_decision = s->avctx->mb_decision;
  dFRM->draw_horiz_band = !(s->pict_type==FF_B_TYPE && s->avctx->draw_horiz_band && s->picture_structure==PICT_FRAME);
  //dFRM->draw_horiz_band = 1;
  dFRM->edge = dFRM->draw_horiz_band;
  dFRM->lowres = s->avctx->lowres;
  dFRM->no_rounding = s->no_rounding;
  dFRM->quarter_sample = s->quarter_sample;
  dFRM->out_format = s->out_format;
  dFRM->chroma_x_shift = s->chroma_x_shift;
  dFRM->chroma_y_shift = s->chroma_y_shift;
  dFRM->codec_id = s->codec_id;
  dFRM->width = s->width;
  dFRM->height = s->height;
  dFRM->mb_width = s->mb_width;
  dFRM->mb_height = s->mb_height;
  dFRM->linesize = s->linesize;
  dFRM->uvlinesize = s->uvlinesize;
  dFRM->mspel = s->mspel;
  dFRM->h263_aic = s->h263_aic;
  dFRM->hurry_up = s->hurry_up;
  dFRM->h263_msmpeg4 = s->h263_msmpeg4;
  dFRM->mpeg_quant = s->mpeg_quant;
  memcpy(dFRM->raster_end, s->inter_scantable.raster_end, sizeof(char)*64);
  //memcpy(dFRM->permutated, s->intra_scantable.permutated, sizeof(char)*64);
  dFRM->workaround_bugs = s->workaround_bugs;
  //dFRM->b_scratchpad = get_phy_addr(s->b_scratchpad)+0x80000000;
  dFRM->alternate_scan = s->alternate_scan;
  memcpy(dFRM->intra_matrix, s->intra_matrix, sizeof(short)*64);
  memcpy(dFRM->inter_matrix, s->inter_matrix, sizeof(short)*64);
  dFRM->last_picture_data[0] = s->last_picture.data[0];
  dFRM->last_picture_data[1] = s->last_picture.data[1];
  dFRM->last_picture_data[2] = s->last_picture.data[2];
  dFRM->last_picture_data[3] = s->last_picture.data[3];
  dFRM->next_picture_data[0] = s->next_picture.data[0];
  dFRM->next_picture_data[1] = s->next_picture.data[1];
  dFRM->next_picture_data[2] = s->next_picture.data[2];
  dFRM->next_picture_data[3] = s->next_picture.data[3];

#ifdef JZC_ROTA90_OPT
  dFRM->rota_picture_data[0] = get_phy_addr(s->current_picture.data[2]);
  dFRM->rota_picture_data[1] = get_phy_addr(s->current_picture.data[3]);
#endif

  dFRM->current_picture_data[0] = get_phy_addr(s->current_picture.data[0]);
  dFRM->current_picture_data[1] = get_phy_addr(s->current_picture.data[1]);

  //dFRM->mpFrame = mpFrame;

  return 0;
}

static int get_MBARGs(MpegEncContext *s)
{
  //dMB->mb_skipped = s->mb_skipped;
  //dMB->dest[0] = get_phy_addr(s->dest[0]);
  //dMB->dest[1] = get_phy_addr(s->dest[1]);
  //dMB->dest[2] = get_phy_addr(s->dest[2]);
#if 0
  memcpy(dMB->mv, s->mv, sizeof(int)*2*4*2);
  dMB->mv[0] = s->mv[0][0][0];
  dMB->mv[1] = s->mv[0][0][1];
  dMB->mv[2] = s->mv[0][1][0];
  dMB->mv[3] = s->mv[0][1][1];
  dMB->mv[4] = s->mv[0][2][0];
  dMB->mv[5] = s->mv[0][2][1];
  dMB->mv[6] = s->mv[0][3][0];
  dMB->mv[7] = s->mv[0][3][1];
  dMB->mv[8] = s->mv[1][0][0];
  dMB->mv[9] = s->mv[1][0][1];
  dMB->mv[10] = s->mv[1][1][0];
  dMB->mv[11] = s->mv[1][1][1];
  dMB->mv[12] = s->mv[1][2][0];
  dMB->mv[13] = s->mv[1][2][1];
  dMB->mv[14] = s->mv[1][3][0];
  dMB->mv[15] = s->mv[1][3][1];
#endif
#ifdef JZC_PMON_P0MB
      PMON_ON(gtmb);
#endif      
  dMB->interlaced_dct = s->interlaced_dct;
  dMB->mb_intra = s->mb_intra;
  dMB->mv_dir = s->mv_dir;
  dMB->mv_type = s->mv_type;
  dMB->mb_x = s->mb_x;
  dMB->mb_y = s->mb_y;
  dMB->qscale = s->qscale;
  dMB->chroma_qscale = s->chroma_qscale;
  dMB->y_dc_scale = s->y_dc_scale;
  dMB->c_dc_scale = s->c_dc_scale;
  dMB->ac_pred = s->ac_pred;
  dMB->skip_idct = s->avctx->skip_idct;
#ifdef JZC_PMON_P0MB
      PMON_OFF(gtmb);
      gtmb++;
#endif
  memcpy(dMB->block_last_index, s->block_last_index, sizeof(int)*6);
#if 0
  dMB->val[0] = val[0];
  dMB->val[1] = val[1];
  dMB->val[2] = val[2];
  dMB->val[3] = val[3];
  dMB->val[4] = val[4];
  dMB->val[5] = val[5];
#endif
#if 0
  memcpy(dMB->block[0], s->block[0], sizeof(short)*64);
  memcpy(dMB->block[1], s->block[1], sizeof(short)*64);
  memcpy(dMB->block[2], s->block[2], sizeof(short)*64);
  memcpy(dMB->block[3], s->block[3], sizeof(short)*64);
  memcpy(dMB->block[4], s->block[4], sizeof(short)*64);
  memcpy(dMB->block[5], s->block[5], sizeof(short)*64);
#endif
  return 0;
}

static inline void ff_init_mbdest(MpegEncContext *s){
  int mb_size = 4; 
  
  s->dest[0] = s->current_picture.data[0] + ((s->mb_x - 1) << (mb_size + 4));
  s->dest[1] = s->current_picture.data[1] + ((s->mb_x - 1) << (mb_size + 3));
  if(!(s->pict_type==FF_B_TYPE && s->avctx->draw_horiz_band && s->picture_structure==PICT_FRAME)){
    s->dest[0] += ((s->mb_y * s->mb_width) << (mb_size + 4));
    if (s->mb_y != 0)
      s->dest[0] += (1024 * s->mb_y);
    s->dest[1] += ((s->mb_y * s->mb_width) << (mb_size + 3));
    if (s->mb_y != 0)
      s->dest[1] += (512 * s->mb_y);
  }
}

static inline void ff_update_mbdest(MpegEncContext *s){
  s->dest[0] += 256;
  s->dest[1] += 128;
}

static void ptr_square(void * start_ptr,int size,int h,int w, int stride){
  unsigned int* start_int=(int*)start_ptr;
  unsigned short* start_short=(short*)start_ptr;
  unsigned char* start_byte=(char*)start_ptr;
  int i, j;
  if(size==4){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%08x,",start_int[i*stride+j]);
      }
      printf("\n");
    }
  }
  if(size==2){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%04x,",start_short[i*stride+j]);
      }
      printf("\n");
    }
  }
  if(size==1){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%02x,",start_byte[i*stride+j]);
      }
      printf("\n");
    }
  }
}

static inline void JZC_h263_update_motion_val(MpegEncContext * s){
  const int mb_xy = s->mb_y * s->mb_stride + s->mb_x;
  //FIXME a lot of that is only needed for !low_delay
  const int wrap = s->b8_stride;
  const int xy = s->block_index[0];

  s->current_picture.mbskip_table[mb_xy]= s->mb_skipped;

  if (s->mv_type == MV_TYPE_16X16){
    int motion_x, motion_y;
    if (s->mb_intra){
      motion_x = 0;
      motion_y = 0;
    }else{
      motion_x = s->mv[0][0][0];
      motion_y = s->mv[0][0][1];
    }
    s->current_picture.motion_val[0][xy][0] = motion_x;
    s->current_picture.motion_val[0][xy][1] = motion_y;
    s->current_picture.motion_val[0][xy + 1][0] = motion_x;
    s->current_picture.motion_val[0][xy + 1][1] = motion_y;
    s->current_picture.motion_val[0][xy + wrap][0] = motion_x;
    s->current_picture.motion_val[0][xy + wrap][1] = motion_y;
    s->current_picture.motion_val[0][xy + 1 + wrap][0] = motion_x;
    s->current_picture.motion_val[0][xy + 1 + wrap][1] = motion_y;
  }
}

static int decode_slice(MpegEncContext *s){
  const int part_mask= s->partitioned_frame ? (AC_END|AC_ERROR) : 0x7F;
  const int mb_size= 16>>s->avctx->lowres;

#ifdef JZC_P1_OPT
  unsigned int tcsm0_fifo_rp_lh;
  unsigned int task_fifo_wp_lh;
  unsigned int task_fifo_wp_lh_overlap;

  tcsm1_fifo_wp = (int *)TCSM1_VUCADDR(TCSM1_MBNUM_WP);
  tcsm0_fifo_rp = (int *)TCSM0_P1_FIFO_RP;

  task_fifo_wp = (int *)TCSM0_TASK_FIFO; // used by P0
  task_fifo_wp_d1 = (int *)TCSM0_TASK_FIFO; // wp delay 1 MB
  task_fifo_wp_d2 = (int *)TCSM0_TASK_FIFO; // wp delay 2 MB
  *tcsm1_fifo_wp = 0; // write by P0, used by P1
  *tcsm0_fifo_rp = 0; // write once before p1 start

  int *ptr = (unsigned int*)TCSM1_VUCADDR(TCSM1_DBG_BUF);
#endif

  s->last_resync_gb= s->gb;
  s->first_slice_line= 1;

  //printf("decode_slice start\n");
  s->resync_mb_x= s->mb_x;
  s->resync_mb_y= s->mb_y;

#ifdef JZC_P1_OPT
  motion_config_mpeg4(s);
  //printf("motion_config_mpeg4 successful\n");
#endif

  ff_set_qscale(s, s->qscale);

#if 0
  if (s->avctx->hwaccel) {
    const uint8_t *start= s->gb.buffer + get_bits_count(&s->gb)/8;
    const uint8_t *end  = ff_h263_find_resync_marker(start + 1, s->gb.buffer_end);
    skip_bits_long(&s->gb, 8*(end - start));
    return s->avctx->hwaccel->decode_slice(s->avctx, start, end - start);
  }

  if(s->partitioned_frame){
    const int qscale= s->qscale;

    if(CONFIG_MPEG4_DECODER && s->codec_id==CODEC_ID_MPEG4){
      if(ff_mpeg4_decode_partitions(s) < 0)
	return -1;
    }

    /* restore variables which were modified */
    s->first_slice_line=1;
    s->mb_x= s->resync_mb_x;
    s->mb_y= s->resync_mb_y;
    ff_set_qscale(s, qscale);
  }
#endif
  get_GlbARGs(s);
  //printf("decode_slice get_GlbARGs end\n");

#ifdef JZC_P1_OPT
  int n;int temp=dFRM;
  for(n=0;n<FRAME_T_CC_LINE;n++){
    i_cache(0x19,temp,0);
    temp +=32;
  }
  jz_dcache_wb();
  AUX_RESET();
  *((volatile int *)(TCSM0_P1_TASK_DONE)) = 0;
  AUX_START();
#endif

#ifdef JZC_VLC_HW_OPT
      { // copy vc1 bitstream to framebuffer
	int ii;
	int bs_len = (s->gb.size_in_bits >> 3) + 64;
	//int * bs_src = ((unsigned int)s->gb.buffer) & 0xFFFFFFFC;
	uint8_t * bs_src = s->gb.buffer;
	//int * bs_dst = ((unsigned int)vc1_hw_bs_buffer) & 0xFFFFFFFC;
	uint8_t * bs_dst = (uint8_t *)vc1_sw_bs_buffer;
	//printf(" bs_len: %d; bs_src: 0x%x; bs_dst: 0x%x \n",bs_len,bs_src,bs_dst);
	for(ii=0; ii<bs_len; ii++)
	  bs_dst[ii] = bs_src[ii];
      }

      {//set table data
	int i;
	unsigned int * vlc_table_base;
	unsigned int * hw_table_base;

	vlc_table_base = (unsigned int *)(vlc_base + VLC_TBL_OFST);
	hw_table_base  = (unsigned int *)c_table;

	for (i = 0; i < 36; i++){
	  vlc_table_base[i] = hw_table_base[i];
	}

	vlc_table_base = (unsigned int *)(vlc_base + VLC_TBL_OFST + 512);
	hw_table_base = (unsigned int *)cbpy_table;
	for (i = 0; i < 32; i++){
	  vlc_table_base[i] = hw_table_base[i];
	}

	vlc_table_base = (unsigned int *)(vlc_base + VLC_TBL_OFST + 1024);
	hw_table_base = (unsigned int *)dc_lum_table;
	for (i = 0; i < 132; i++){
	  vlc_table_base[i] = hw_table_base[i];
	}

	vlc_table_base = (unsigned int *)(vlc_base + VLC_TBL_OFST + 1536);
	hw_table_base = (unsigned int *)dc_chrom_table;
	for (i = 0; i < 136; i++){
	  vlc_table_base[i] = hw_table_base[i];
	}

	vlc_table_base = (unsigned int *)(vlc_base + VLC_TBL_OFST + 2048);
	hw_table_base = (unsigned int *)rl_intra_table;
	for (i = 0; i < 200; i++){
	  vlc_table_base[i] = hw_table_base[i];
	}
      }

      jz_dcache_wb();
#if 1
      { // bs
	unsigned int sw_addr, sw_ofst, sw_acc_pos;
	//sw_addr = s->buffer;
	sw_addr = vc1_hw_bs_buffer;
	sw_ofst = s->gb.index;
	//printf(" sw_addr : 0x%x; sw_ofst : 0x%x \n",sw_addr,sw_ofst);
	unsigned int bs_addr, bs_ofst;
	bs_addr = (sw_addr + (sw_ofst >> 3)) & 0xFFFFFFFC;
	bs_ofst = sw_ofst & 0x1F;
	//printf(" bs_addr : 0x%x;  bs_ofst : 0x%x \n",bs_addr,bs_ofst);

	// bs addr
	CPU_SET_BS_ADDR(bs_addr);
	// hw bs init
	unsigned int bs_ofst_val = (1 << 16) | (bs_ofst << 5);
	CPU_SET_BS_OFST(bs_ofst_val);
	//printf(" CPU_GET_BS_OFST() : 0x%x \n",CPU_GET_BS_OFST());
	unsigned int bs_init_done;
	do {
	  bs_init_done = (CPU_GET_BS_OFST() & (1<<16)) >> 16;
	  //printf(" bs_init_done : %d \n",bs_init_done);
	} while (bs_init_done);
	//printf(" bs_init_done \n");
      }
#endif
#endif
  for(; s->mb_y < s->mb_height; s->mb_y++) {
    /* per-row end of slice checks */
    if(s->msmpeg4_version){
      if(s->resync_mb_y + s->slice_height == s->mb_y){
#if 1
	int cnt, add_num = 5;
	for(cnt = 0; cnt < add_num; cnt++){
	  do{
	    tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
	    task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
	    task_fifo_wp_lh_overlap = task_fifo_wp_lh + TASK_BUF_LEN;
	  } while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );

	  dMB = task_fifo_wp;
	  get_MBARGs(s);
	  if (cnt == 1){
	    dMB->real_num = -1;
	  }else if (cnt == 0){
	    dMB->real_num = 2;
	  }
	  else
	    dMB->real_num = 1;

	  task_fifo_wp += (TASK_BUF_LEN + 3)>>2;

	  int reach_tcsm0_end = ((unsigned int)(task_fifo_wp + (TASK_BUF_LEN>>2))) >= 0xF4004000;
	  if (reach_tcsm0_end)
	    task_fifo_wp = (int *)TCSM0_TASK_FIFO;      

	  task_fifo_wp_d2 = task_fifo_wp_d1;
	  task_fifo_wp_d1 = task_fifo_wp;
	  (*tcsm1_fifo_wp)++;
	}
	
	int nn, tmp;
	do{
	  tmp = *((volatile int *)(TCSM0_P1_TASK_DONE));
	}while (tmp == 0);
	AUX_RESET();
#endif
	ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, AC_END|DC_END|MV_END);
	return 0;
      }
    }

    if(s->msmpeg4_version==1){
      s->last_dc[0]=
	s->last_dc[1]=
	s->last_dc[2]= 128;
    }

    jz_init_block_index(s);
    //ff_init_mbdest(s);

    for(; s->mb_x < s->mb_width; s->mb_x++) {
      int ret;
#ifdef JZC_PMON_P0FRM
      PMON_ON(p0frm1);
#endif
      jz_update_block_index(s);
      //ff_update_mbdest(s);

      if(s->resync_mb_x == s->mb_x && s->resync_mb_y+1 == s->mb_y){
	s->first_slice_line=0;
      }

      /* DCT & quantize */

      s->mv_dir = MV_DIR_FORWARD;
      s->mv_type = MV_TYPE_16X16;

#ifdef JZC_PMON_P0FRM
      PMON_OFF(p0frm1);
#endif

#ifdef JZC_P1_OPT
#ifdef JZC_PMON_P0MB
      PMON_ON(p0wt);
#endif
      do{
	tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
	task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
	task_fifo_wp_lh_overlap = task_fifo_wp_lh + TASK_BUF_LEN;
	//printf("mbx %d, mby %d, rp_lh 0x%08x, wp_lh 0x%08x\n", s->mb_x, s->mb_y, tcsm0_fifo_rp_lh, task_fifo_wp_lh);
	//if (mpFrame > 1)
	//printf("mbx %d, mby %d, 22222 %d, %d, %d, %d, %d, %d, %d, %d\n", 
	//s->mb_x, s->mb_y, ptr[30], ptr[31], ptr[32], ptr[33], ptr[34], ptr[35], ptr[36], ptr[37]);
	//printf("mb_x:%d mb_y:%d\n", s->mb_x, s->mb_y);
	//ptr_square(rota_tmp_buf+256,1,8,16,16);
      } while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );
      dMB = task_fifo_wp;
      dMB->real_num = 1;
#ifdef JZC_PMON_P0MB
      PMON_OFF(p0wt);
      p0wt++;
#endif
#endif

#ifdef JZC_PMON_P0FRM
      PMON_ON(p0frm2);
#endif

#ifdef JZC_PMON_P0MB
      PMON_ON(dcmb);
#endif
      ret= s->decode_mb(s, dMB->block);
#ifdef JZC_PMON_P0MB
      PMON_OFF(dcmb);
      dcmb++;
#endif

      if (s->pict_type!=FF_B_TYPE)
	//ff_h263_update_motion_val(s);
	JZC_h263_update_motion_val(s);

#ifdef JZC_PMON_P0MB
      PMON_ON(jzmb);
#endif
      JZC_decode_mb(s);
#ifdef JZC_PMON_P0MB
      PMON_OFF(jzmb);
      jzmb++;
#endif

      get_MBARGs(s);

#ifdef JZC_P1_OPT
      task_fifo_wp += (TASK_BUF_LEN + 3) >> 2;
      int reach_tcsm0_end = ((unsigned int)(task_fifo_wp + (TASK_BUF_LEN>>2))) >= 0xF4004000;

      if (reach_tcsm0_end)
	task_fifo_wp = (int *)TCSM0_TASK_FIFO;

      task_fifo_wp_d2 = task_fifo_wp_d1;
      task_fifo_wp_d1 = task_fifo_wp;

      (*tcsm1_fifo_wp)++;
#endif

#ifdef JZC_PMON_P0FRM
      PMON_OFF(p0frm2);
#endif

      if(ret<0){
	const int xy= s->mb_x + s->mb_y*s->mb_stride;
	if(ret==SLICE_END){

#ifndef JZC_P1_OPT
	  {
	    int i,j;
	    for (j = 0; j < s->mb_height; j++)
	      for (i = 0; i < s->mb_width; i++){
		MPV_get_buf(j*s->mb_width+i);
		//MPV_decode_mb_opt(s, gnpMMD[j*s->mb_width+i].block);
		MPV_decode_mb_p1(gnpMMD[j*s->mb_width+i].block);
	      }
	  }
#endif

#ifdef JZC_P1_OPT
	  int cnt, add_num = 5;
	  for(cnt = 0; cnt < add_num; cnt++){

	    do{
	      //printf("333333333333333333333333333333333\n");
	      tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
	      task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
	      task_fifo_wp_lh_overlap = task_fifo_wp_lh + TASK_BUF_LEN;
	    } while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );

	    dMB = task_fifo_wp;
	    get_MBARGs(s);
	    if (cnt == 1){
	      //printf("88888888888888888888888888888888888\n");
	      //printf("real_num -1 %d %d\n", dMB->mb_x, dMB->mb_y);
	      dMB->real_num = -1;
	    }else if (cnt == 0){
	      dMB->real_num = 2;
	    }
	    else
	      dMB->real_num = 1;

	    task_fifo_wp += (TASK_BUF_LEN + 3)>>2;

	    //*(uint16_t *)task_fifo_wp_d2 = (MPEG4_MB_CTRL_INFO + 3) & 0xFFFC;

	    int reach_tcsm0_end = ((unsigned int)(task_fifo_wp + (TASK_BUF_LEN>>2))) >= 0xF4004000;
	    if (reach_tcsm0_end)
	      task_fifo_wp = (int *)TCSM0_TASK_FIFO;      

	    task_fifo_wp_d2 = task_fifo_wp_d1;
	    task_fifo_wp_d1 = task_fifo_wp;
	    (*tcsm1_fifo_wp)++;
	  }

	  //(*tcsm1_fifo_wp)++;
	  //(*tcsm1_fifo_wp)++;
	  //(*tcsm1_fifo_wp)++;

	  int nn, tmp;
	  int *prt = (unsigned int*)TCSM1_VUCADDR(TCSM1_DBG_BUF);
	  do{
	    //printf("777777 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", 
	    //prt[0], prt[1], prt[2], prt[3]);  
	    tmp = *((volatile int *)(TCSM0_P1_TASK_DONE));
	  }while (tmp == 0);
	  AUX_RESET();
#endif

	  if(s->loop_filter)
	    ff_h263_loop_filter(s);

	  //printf("%d %d %d %06X\n", s->mb_x, s->mb_y, s->gb.size*8 - get_bits_count(&s->gb), show_bits(&s->gb, 24));
	  ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y, (AC_END|DC_END|MV_END)&part_mask);

	  s->padding_bug_score--;

	  if(++s->mb_x >= s->mb_width){
	    s->mb_x=0;
	    ff_draw_horiz_band(s, s->mb_y*mb_size, mb_size);
	    s->mb_y++;
	  }
	  return 0;
	}else if(ret==SLICE_NOEND){
	  dMB = task_fifo_wp;
	  dMB->real_num = -1;
	  (*tcsm1_fifo_wp)++;
	  (*tcsm1_fifo_wp)++;
	  (*tcsm1_fifo_wp)++;
	  av_log(s->avctx, AV_LOG_ERROR, "Slice mismatch at MB: %d\n", xy);
	  ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x+1, s->mb_y, (AC_END|DC_END|MV_END)&part_mask);
	  return -1;
	}
	dMB = task_fifo_wp;
	dMB->real_num = -1;
	(*tcsm1_fifo_wp)++;
	(*tcsm1_fifo_wp)++;
	(*tcsm1_fifo_wp)++;
	av_log(s->avctx, AV_LOG_ERROR, "Error at MB: %d\n", xy);
	ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y, (AC_ERROR|DC_ERROR|MV_ERROR)&part_mask);

	return -1;
      }

      if(s->loop_filter)
	ff_h263_loop_filter(s);
    }

    ff_draw_horiz_band(s, s->mb_y*mb_size, mb_size);

    s->mb_x= 0;
  }

#ifdef JZC_P1_OPT
  int cnt, add_num = 5;
  for(cnt = 0; cnt < add_num; cnt++){

    do{
      //printf("333333333333333333333333333333333\n");
      tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
      task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
      task_fifo_wp_lh_overlap = task_fifo_wp_lh + TASK_BUF_LEN;
    } while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );

    dMB = task_fifo_wp;
    get_MBARGs(s);
    if (cnt == 1){
      //printf("88888888888888888888888888888888888\n");
      //printf("real_num -1 %d %d\n", dMB->mb_x, dMB->mb_y);
      dMB->real_num = -1;
    }else if (cnt == 0){
      dMB->real_num = 2;
    }else
      dMB->real_num = 1;

    task_fifo_wp += (TASK_BUF_LEN + 3)>>2;

    //*(uint16_t *)task_fifo_wp_d2 = (MPEG4_MB_CTRL_INFO + 3) & 0xFFFC;

    int reach_tcsm0_end = ((unsigned int)(task_fifo_wp + (TASK_BUF_LEN>>2))) >= 0xF4004000;
    if (reach_tcsm0_end)
      task_fifo_wp = (int *)TCSM0_TASK_FIFO;      

    task_fifo_wp_d2 = task_fifo_wp_d1;
    task_fifo_wp_d1 = task_fifo_wp;
    (*tcsm1_fifo_wp)++;
  }

  int nn, tmp;
  do{
    //printf("666666666666666666666666666666666666\n");
    tmp = *((volatile int *)(TCSM0_P1_TASK_DONE));
  }while (tmp == 0);
  AUX_RESET();

#endif// JZC_P1_OPT

  assert(s->mb_x==0 && s->mb_y==s->mb_height);

  /* try to detect the padding bug */
  if(      s->codec_id==CODEC_ID_MPEG4
	   &&   (s->workaround_bugs&FF_BUG_AUTODETECT)
	   &&    get_bits_left(&s->gb) >=0
	   &&    get_bits_left(&s->gb) < 48
	   //       &&   !s->resync_marker
	   &&   !s->data_partitioning){

    const int bits_count= get_bits_count(&s->gb);
    const int bits_left = s->gb.size_in_bits - bits_count;

    if(bits_left==0){
      s->padding_bug_score+=16;
    } else if(bits_left != 1){
      int v= show_bits(&s->gb, 8);
      v|= 0x7F >> (7-(bits_count&7));

      if(v==0x7F && bits_left<=8)
	s->padding_bug_score--;
      else if(v==0x7F && ((get_bits_count(&s->gb)+8)&8) && bits_left<=16)
	s->padding_bug_score+= 4;
      else
	s->padding_bug_score++;
    }
  }

  if(s->workaround_bugs&FF_BUG_AUTODETECT){
    if(s->padding_bug_score > -2 && !s->data_partitioning /*&& (s->divx_version>=0 || !s->resync_marker)*/)
      s->workaround_bugs |=  FF_BUG_NO_PADDING;
    else
      s->workaround_bugs &= ~FF_BUG_NO_PADDING;
  }

  // handle formats which don't have unique end markers
  if(s->msmpeg4_version || (s->workaround_bugs&FF_BUG_NO_PADDING)){ //FIXME perhaps solve this more cleanly
    int left= get_bits_left(&s->gb);
    int max_extra=7;

    /* no markers in M$ crap */
    if(s->msmpeg4_version && s->pict_type==FF_I_TYPE)
      max_extra+= 17;

    /* buggy padding but the frame should still end approximately at the bitstream end */
    if((s->workaround_bugs&FF_BUG_NO_PADDING) && s->error_recognition>=3)
      max_extra+= 48;
    else if((s->workaround_bugs&FF_BUG_NO_PADDING))
      max_extra+= 256*256*256*64;

    if(left>max_extra){
      av_log(s->avctx, AV_LOG_ERROR, "discarding %d junk bits at end, next would be %X\n", left, show_bits(&s->gb, 24));
    }
    else if(left<0){
      av_log(s->avctx, AV_LOG_ERROR, "overreading %d bits\n", -left);
    }else
      ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, AC_END|DC_END|MV_END);

    return 0;
  }

  av_log(s->avctx, AV_LOG_ERROR, "slice end not reached but screenspace end (%d left %06X, score= %d)\n",
	 get_bits_left(&s->gb),
	 show_bits(&s->gb, 24), s->padding_bug_score);

  ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x, s->mb_y, (AC_END|DC_END|MV_END)&part_mask);

  return -1;
}

static void print_rota_frm(MpegEncContext *s, int *yidx, int *uidx, int *vidx){
  int i,j;
  char *ptr = s->current_picture.data[2];
  //char *ptr = rota_buf1;

  if (yidx[0] >= 0){
    for (j = yidx[2]; j < yidx[3]; j++){ 
      for (i = yidx[0]; i < yidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*256 + j*s->mb_height*256), 1, 16, 16, 16);
      }
    }
  }

  ptr = s->current_picture.data[3];
  //ptr = rota_buf2;
  if (uidx[0] >= 0){
    for (j = uidx[2]; j < uidx[3]; j++){
      for (i = uidx[0]; i < uidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*128 + j*s->mb_height*128), 1, 8, 8, 16);
      }
    }
  }

  if (vidx[0] >= 0){
    for (j = vidx[2]; j < vidx[3]; j++){
      for (i = vidx[0]; i < vidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*128 + j*s->mb_height*128 + 8), 1, 8, 8, 16);
      }
    }
  }
}

static void print_frminfo(MpegEncContext *s, int *yidx, int *uidx, int *vidx){
  int i,j;
  char *ptr = s->current_picture.data[0];

  if (yidx[0] >= 0){
    for (j = yidx[2]; j < yidx[3]; j++){ 
      for (i = yidx[0]; i < yidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*256 + j*s->mb_width*256 + j*1024), 1, 16, 16, 16);
      }
    }
  }

  ptr = s->current_picture.data[1];
  if (uidx[0] >= 0){
    for (j = uidx[2]; j < uidx[3]; j++){
      for (i = uidx[0]; i < uidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*128 + j*s->mb_width*128 + j*512), 1, 8, 8, 16);
      }
    }
  }

  if (vidx[0] >= 0){
    for (j = vidx[2]; j < vidx[3]; j++){
      for (i = vidx[0]; i < vidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*128 + j*s->mb_width*128 + j*512 + 8), 1, 8, 8, 16);
      }
    }
  }
}

static void print_last_frminfo(MpegEncContext *s, int *yidx, int *uidx, int *vidx){
  int i,j;
  char *ptr = s->last_picture.data[0];

  printf("print last frame\n");
  if (yidx[0] >= 0){
    for (j = yidx[2]; j < yidx[3]; j++){
      for (i = yidx[0]; i < yidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*256 + j*s->mb_width*256 + j*1024), 1, 16, 16, 16);
      }
    }
  }

  ptr = s->last_picture.data[1];
  if (uidx[0] >= 0){
    for (j = uidx[2]; j < uidx[3]; j++){
      for (i = uidx[0]; i < uidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*128 + j*s->mb_width*128 + j*512), 1, 8, 8, 16);
      }
    }
  }

  if (vidx[0] >= 0){
    for (j = vidx[2]; j < vidx[3]; j++){
      for (i = vidx[0]; i < vidx[1]; i++){
        printf("mbx:%d mby%d\n", i, j);
        ptr_square((ptr + i*128 + j*s->mb_width*128 + j*512 + 8), 1, 8, 8, 16);
      }
    }
  }
}

static void mpeg4_tile_stuff(MpegEncContext *s){
#if 0
  volatile uint8_t *tile_y, *tile_c, *pxl, *edge, *tile, *dest;
  int mb_i, mb_j, i, j;
    
  tile_y = s->current_picture.data[0];
  tile_c = s->current_picture.data[1];
      
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
#else
  volatile uint8_t *tile_y, *tile_c;
  tile_y = s->current_picture.data[0];
  tile_c = s->current_picture.data[1];

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
#endif
}

int mpeg4_decode_frame(AVCodecContext *avctx,
		       void *data, int *data_size,
		       AVPacket *avpkt)
{
  const uint8_t *buf = avpkt->data;
  int buf_size = avpkt->size;
  MpegEncContext *s = avctx->priv_data;
  int ret;
  AVFrame *pict = data;

#ifdef PRINT_FRAME_TIME
  uint64_t time= rdtsc();
#endif
  s->flags= avctx->flags;
  s->flags2= avctx->flags2;

#ifdef JZC_CRC_VER
  mpFrame++;
#endif

#ifdef JZC_PMON_P0MB
  PMON_CLEAR(p0wt);
  PMON_CLEAR(dcmb);
  PMON_CLEAR(bfds);
  PMON_CLEAR(afds);
  PMON_CLEAR(mcfg);
  PMON_CLEAR(axst);
  PMON_CLEAR(inbl);
  PMON_CLEAR(dhbd);
  PMON_CLEAR(upbl);
  PMON_CLEAR(upmv);
  PMON_CLEAR(jzmb);
  PMON_CLEAR(gtmb);
  PMON_CLEAR(tskf);
  PMON_CLEAR(lpft);

  p0wt=0;dcmb=0;inbl=0;dhbd=0;upbl=0;upmv=0;gtmb=0;jzmb=0;tskf=0;lpft=0;
#endif

#ifdef JZC_PMON_ALLFRM
  PMON_CLEAR(allfrm);
  PMON_ON(allfrm);
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

  if(s->flags&CODEC_FLAG_TRUNCATED){
    int next;

    if(CONFIG_MPEG4_DECODER && s->codec_id==CODEC_ID_MPEG4){
      next= ff_mpeg4_find_frame_end(&s->parse_context, buf, buf_size);
    }else if(CONFIG_H263_DECODER && s->codec_id==CODEC_ID_H263){
      next= ff_h263_find_frame_end(&s->parse_context, buf, buf_size);
    }else{
      av_log(s->avctx, AV_LOG_ERROR, "this codec does not support truncated bitstreams\n");
      return -1;
    }

    if( ff_combine_frame(&s->parse_context, next, (const uint8_t **)&buf, &buf_size) < 0 )
      return buf_size;
  }


 retry:

  if(s->bitstream_buffer_size && (s->divx_packed || buf_size<20)){ //divx 5.01+/xvid frame reorder
    init_get_bits(&s->gb, s->bitstream_buffer, s->bitstream_buffer_size*8);
  }else
    init_get_bits(&s->gb, buf, buf_size*8);
  s->bitstream_buffer_size=0;

  if (!s->context_initialized) {
    if (MPV_common_init(s) < 0) //we need the idct permutaton for reading a custom matrix
      return -1;
  }

  /* We need to set current_picture_ptr before reading the header,
   * otherwise we cannot store anyting in there */
  if(s->current_picture_ptr==NULL || s->current_picture_ptr->data[0]){
    int i= ff_find_unused_picture(s, 0);
    s->current_picture_ptr= &s->picture[i];
  }

  /* let's go :-) */
  if (CONFIG_WMV2_DECODER && s->msmpeg4_version==5) {
    //printf("WMV2\n");
    ret= ff_wmv2_decode_picture_header(s);
  } else if (CONFIG_MSMPEG4_DECODER && s->msmpeg4_version) {
    //printf("decode_frame, msmpeg4\n");
    ret = msmpeg4_decode_picture_header(s);
  } else if (CONFIG_MPEG4_DECODER && s->h263_pred) {
    //printf("decode_frame, ff_mpeg4\n");
    if(s->avctx->extradata_size && s->picture_number==0){
      GetBitContext gb;

      init_get_bits(&gb, s->avctx->extradata, s->avctx->extradata_size*8);
      //ret = ff_mpeg4_decode_picture_header(s, &gb);
     ret = JZC_mpeg4_decode_picture_header(s, &gb);
    }
    //ret = ff_mpeg4_decode_picture_header(s, &s->gb);
    ret = JZC_mpeg4_decode_picture_header(s, &s->gb);
  } else if (CONFIG_H263I_DECODER && s->codec_id == CODEC_ID_H263I) {
    //printf("H263I\n");
    ret = ff_intel_h263_decode_picture_header(s);
  } else if (CONFIG_FLV_DECODER && s->h263_flv) {
    //printf("FLV\n");
    ret = ff_flv_decode_picture_header(s);
  } else {
    ret = h263_decode_picture_header(s);
  }

  //printf("decode_frame, decode_header end\n");

  if(ret==FRAME_SKIPPED) return get_consumed_bytes(s, buf_size);

  /* skip if the header was thrashed */
  if (ret < 0){
    //av_log(s->avctx, AV_LOG_ERROR, "header damaged\n");
    return -1;
  }

  avctx->has_b_frames= !s->low_delay;

  if(s->xvid_build==-1 && s->divx_version==-1 && s->lavc_build==-1){
    if(s->stream_codec_tag == AV_RL32("XVID") ||
       s->codec_tag == AV_RL32("XVID") || s->codec_tag == AV_RL32("XVIX") ||
       s->codec_tag == AV_RL32("RMP4") ||
       s->codec_tag == AV_RL32("SIPP")
       )
      s->xvid_build= 0;
#if 0
    if(s->codec_tag == AV_RL32("DIVX") && s->vo_type==0 && s->vol_control_parameters==1
       && s->padding_bug_score > 0 && s->low_delay) // XVID with modified fourcc
      s->xvid_build= 0;
#endif
  }

  if(s->xvid_build==-1 && s->divx_version==-1 && s->lavc_build==-1){
    if(s->codec_tag == AV_RL32("DIVX") && s->vo_type==0 && s->vol_control_parameters==0)
      s->divx_version= 400; //divx 4
  }

  if(s->xvid_build>=0 && s->divx_version>=0){
    s->divx_version=
      s->divx_build= -1;
  }

  if(s->workaround_bugs&FF_BUG_AUTODETECT){
    if(s->codec_tag == AV_RL32("XVIX"))
      s->workaround_bugs|= FF_BUG_XVID_ILACE;

    if(s->codec_tag == AV_RL32("UMP4")){
      s->workaround_bugs|= FF_BUG_UMP4;
    }

    if(s->divx_version>=500 && s->divx_build<1814){
      s->workaround_bugs|= FF_BUG_QPEL_CHROMA;
    }

    if(s->divx_version>502 && s->divx_build<1814){
      s->workaround_bugs|= FF_BUG_QPEL_CHROMA2;
    }

    if(s->xvid_build<=3U)
      s->padding_bug_score= 256*256*256*64;

    if(s->xvid_build<=1U)
      s->workaround_bugs|= FF_BUG_QPEL_CHROMA;

    if(s->xvid_build<=12U)
      s->workaround_bugs|= FF_BUG_EDGE;

    if(s->xvid_build<=32U)
      s->workaround_bugs|= FF_BUG_DC_CLIP;

#define SET_QPEL_FUNC(postfix1, postfix2)				\
    s->dsp.put_ ## postfix1 = ff_put_ ## postfix2;			\
    s->dsp.put_no_rnd_ ## postfix1 = ff_put_no_rnd_ ## postfix2;	\
    s->dsp.avg_ ## postfix1 = ff_avg_ ## postfix2;

    if(s->lavc_build<4653U)
      s->workaround_bugs|= FF_BUG_STD_QPEL;

    if(s->lavc_build<4655U)
      s->workaround_bugs|= FF_BUG_DIRECT_BLOCKSIZE;

    if(s->lavc_build<4670U){
      s->workaround_bugs|= FF_BUG_EDGE;
    }

    if(s->lavc_build<=4712U)
      s->workaround_bugs|= FF_BUG_DC_CLIP;

    if(s->divx_version>=0)
      s->workaround_bugs|= FF_BUG_DIRECT_BLOCKSIZE;
    //printf("padding_bug_score: %d\n", s->padding_bug_score);
    if(s->divx_version==501 && s->divx_build==20020416)
      s->padding_bug_score= 256*256*256*64;

    if(s->divx_version<500U){
      s->workaround_bugs|= FF_BUG_EDGE;
    }

    if(s->divx_version>=0)
      s->workaround_bugs|= FF_BUG_HPEL_CHROMA;
#if 0
    if(s->divx_version==500)
      s->padding_bug_score= 256*256*256*64;

    /* very ugly XVID padding bug detection FIXME/XXX solve this differently
     * Let us hope this at least works.
     */
    if(   s->resync_marker==0 && s->data_partitioning==0 && s->divx_version==-1
	  && s->codec_id==CODEC_ID_MPEG4 && s->vo_type==0)
      s->workaround_bugs|= FF_BUG_NO_PADDING;

    if(s->lavc_build<4609U) //FIXME not sure about the version num but a 4609 file seems ok
      s->workaround_bugs|= FF_BUG_NO_PADDING;
#endif
  }

  //printf("111111111\n");
  if(s->workaround_bugs& FF_BUG_STD_QPEL){
    SET_QPEL_FUNC(qpel_pixels_tab[0][ 5], qpel16_mc11_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[0][ 7], qpel16_mc31_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[0][ 9], qpel16_mc12_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[0][11], qpel16_mc32_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[0][13], qpel16_mc13_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[0][15], qpel16_mc33_old_c)

      SET_QPEL_FUNC(qpel_pixels_tab[1][ 5], qpel8_mc11_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[1][ 7], qpel8_mc31_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[1][ 9], qpel8_mc12_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[1][11], qpel8_mc32_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[1][13], qpel8_mc13_old_c)
      SET_QPEL_FUNC(qpel_pixels_tab[1][15], qpel8_mc33_old_c)
      }

  if(avctx->debug & FF_DEBUG_BUGS)
    av_log(s->avctx, AV_LOG_DEBUG, "bugs: %X lavc_build:%d xvid_build:%d divx_version:%d divx_build:%d %s\n",
	   s->workaround_bugs, s->lavc_build, s->xvid_build, s->divx_version, s->divx_build,
	   s->divx_packed ? "p" : "");

#if 0 // dump bits per frame / qp / complexity
  {
    static FILE *f=NULL;
    if(!f) f=fopen("rate_qp_cplx.txt", "w");
    fprintf(f, "%d %d %f\n", buf_size, s->qscale, buf_size*(double)s->qscale);
  }
#endif

#if HAVE_MMX
  if (s->codec_id == CODEC_ID_MPEG4 && s->xvid_build>=0 && avctx->idct_algo == FF_IDCT_AUTO && (av_get_cpu_flags() & AV_CPU_FLAG_MMX)) {
    avctx->idct_algo= FF_IDCT_XVIDMMX;
    avctx->coded_width= 0; // force reinit
    //        dsputil_init(&s->dsp, avctx);
    s->picture_number=0;
  }
#endif

  /* After H263 & mpeg4 header decode we have the height, width,*/
  /* and other parameters. So then we could init the picture   */
  /* FIXME: By the way H263 decoder is evolving it should have */
  /* an H263EncContext                                         */

  if (   s->width  != avctx->coded_width
	 || s->height != avctx->coded_height) {
    /* H.263 could change picture size any time */
    ParseContext pc= s->parse_context; //FIXME move these demuxng hack to avformat
    s->parse_context.buffer=0;
		if (s->width > 1920 || s->height > 1080)
			return -1;
    MPV_common_end(s);
    s->parse_context= pc;
  }
  if (!s->context_initialized) {
    avcodec_set_dimensions(avctx, s->width, s->height);

    goto retry;
  }
  if((s->codec_id==CODEC_ID_H263 || s->codec_id==CODEC_ID_H263P || s->codec_id == CODEC_ID_H263I))
    s->gob_index = ff_h263_get_gob_height(s);

  // for hurry_up==5
  s->current_picture.pict_type= s->pict_type;
  s->current_picture.key_frame= s->pict_type == FF_I_TYPE;

  /* skip B-frames if we don't have reference frames */
  if(s->last_picture_ptr==NULL && (s->pict_type==FF_B_TYPE || s->dropable)) return get_consumed_bytes(s, buf_size);
  /* skip b frames if we are in a hurry */
  if(avctx->hurry_up && s->pict_type==FF_B_TYPE) return get_consumed_bytes(s, buf_size);
  if(   (avctx->skip_frame >= AVDISCARD_NONREF && s->pict_type==FF_B_TYPE)
	|| (avctx->skip_frame >= AVDISCARD_NONKEY && s->pict_type!=FF_I_TYPE)
	||  avctx->skip_frame >= AVDISCARD_ALL)
    return get_consumed_bytes(s, buf_size);
  /* skip everything if we are in a hurry>=5 */
  if(avctx->hurry_up>=5) return get_consumed_bytes(s, buf_size);

  if(s->next_p_frame_damaged){
    if(s->pict_type==FF_B_TYPE)
      return get_consumed_bytes(s, buf_size);
    else
      s->next_p_frame_damaged=0;
  }

  if((s->avctx->flags2 & CODEC_FLAG2_FAST) && s->pict_type==FF_B_TYPE){
    s->me.qpel_put= s->dsp.put_2tap_qpel_pixels_tab;
    s->me.qpel_avg= s->dsp.avg_2tap_qpel_pixels_tab;
  }else if((!s->no_rounding) || s->pict_type==FF_B_TYPE){
    s->me.qpel_put= s->dsp.put_qpel_pixels_tab;
    s->me.qpel_avg= s->dsp.avg_qpel_pixels_tab;
    mpeg4_motion_set(MPEG_QPEL, MPEG_HPEL);
  }else{
    s->me.qpel_put= s->dsp.put_no_rnd_qpel_pixels_tab;
    s->me.qpel_avg= s->dsp.avg_qpel_pixels_tab;
    mpeg4_motion_set(MPEG_NRND, MPEG_HPEL);
  }

  if(MPV_frame_start(s, avctx) < 0)
    return -1;

  if (CONFIG_MPEG4_VDPAU_DECODER && (s->avctx->codec->capabilities & CODEC_CAP_HWACCEL_VDPAU)) {
    ff_vdpau_mpeg4_decode_picture(s, s->gb.buffer, s->gb.buffer_end - s->gb.buffer);
    goto frame_end;
  }

  if (avctx->hwaccel) {
    if (avctx->hwaccel->start_frame(avctx, s->gb.buffer, s->gb.buffer_end - s->gb.buffer) < 0)
      return -1;
  }

  ff_er_frame_start(s);

  //the second part of the wmv2 header contains the MB skip bits which are stored in current_picture->mb_type
  //which is not available before MPV_frame_start()
  if (CONFIG_WMV2_DECODER && s->msmpeg4_version==5){
    ret = ff_wmv2_decode_secondary_picture_header(s);
    if(ret<0) return ret;
    if(ret==1) goto intrax8_decoded;
  }

  /* decode each macroblock */
  s->mb_x=0;
  s->mb_y=0;

  if (0){
    //if (mpFrame == 44){
    int yidx[4];
    int uidx[4];
    int vidx[4];
    //printf("Frame35 la 0x%08x\n", s->last_picture.data[0]);

    yidx[0] = 2;
    yidx[1] = 3;
    yidx[2] = 0;
    //yidx[3] = s->mb_height;
    yidx[3] = 1;

    uidx[0] = -1;
    uidx[1] = 1;
    uidx[2] = 0;
    //uidx[3] = s->mb_height;
    uidx[3] = 1;

    vidx[0] = -1;
    vidx[1] = s->mb_width;
    vidx[2] = 0;
    vidx[3] = s->mb_height;

    print_last_frminfo(s, yidx, uidx, vidx);
    //print_next_frminfo(s, yidx, uidx, vidx);
  }

#ifdef JZC_PMON_P0FRM
  PMON_CLEAR(p0frm1);
  PMON_CLEAR(p0frm2);
#endif
  decode_slice(s);
  while(s->mb_y<s->mb_height){
    if(s->msmpeg4_version){
      if(s->slice_height==0 || s->mb_x!=0 || (s->mb_y%s->slice_height)!=0 || get_bits_count(&s->gb) > s->gb.size_in_bits)
	break;
    }else{
      if(ff_h263_resync(s)<0)
	break;
    }

    if(s->msmpeg4_version<4 && s->h263_pred)
      ff_mpeg4_clean_buffers(s);

    decode_slice(s);
  }

#ifdef JZC_PMON_P0FRM
  pmon_cnt++;
  pmon_val1 += (p0frm1_pmon_val + p0frm2_pmon_val)/s->mb_width/s->mb_height;
  printf("PMON_frm cfrm:%d cfrmavg:%d avg:%lld %d\n", 
	 p0frm1_pmon_val+p0frm2_pmon_val, (p0frm1_pmon_val+p0frm2_pmon_val)/s->mb_width/s->mb_height, pmon_val1, pmon_cnt);
#endif

#ifdef JZC_PMON_P1FRM
  int *ptr = (unsigned int*)TCSM1_VUCADDR(TCSM1_DBG_BUF);
  p1_pmon_cnt++;
  p1_pmon += ptr[22]/s->mb_width/s->mb_height;
  printf("PMON_P1 cfrm:%d cfrmavg:%d avg:%lld %d\n", 
	 ptr[22], ptr[22]/s->mb_width/s->mb_height, p1_pmon, p1_pmon_cnt);
#endif

#ifdef JZC_PMON_P0MB
  printf("gtmb:%d dcmb:%d p0wt:%d jzmb:%d\n", 
	 gtmb_pmon_val/gtmb, dcmb_pmon_val/(dcmb+1), p0wt_pmon_val/p0wt, jzmb_pmon_val/(jzmb+1));
  printf("gtmb:%d dcmb:%d p0wt:%d jzmb:%d\n",
	 gtmb_pmon_val_ex/gtmb, dcmb_pmon_val_ex/(dcmb+1), p0wt_pmon_val_ex/p0wt, jzmb_pmon_val_ex/(jzmb+1));
#endif

#ifdef JZC_PMON_P1MB
  int *ptr = (unsigned int*)TCSM1_VUCADDR(TCSM1_DBG_BUF);
  printf("p1_pmon: ckmc:%d stmc:%d idct:%d edge:%d plp0:%d plp1:%d wait:%d p1test:%d wtct:%d p1mc:%d\n",
	 ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[20], ptr[8]);
  printf("p1_pipe: ckmc:%d p1mc:%d\n", ptr[10], ptr[18]);
#endif

  if (s->h263_msmpeg4 && s->msmpeg4_version<4 && s->pict_type==FF_I_TYPE)
    if(!CONFIG_MSMPEG4_DECODER || msmpeg4_decode_ext_header(s, buf_size) < 0){
      s->error_status_table[s->mb_num-1]= AC_ERROR|DC_ERROR|MV_ERROR;
    }

  assert(s->bitstream_buffer_size==0);
 frame_end:
  /* divx 5.01+ bistream reorder stuff */
  if(s->codec_id==CODEC_ID_MPEG4 && s->divx_packed){
    int current_pos= get_bits_count(&s->gb)>>3;
    int startcode_found=0;

    if(buf_size - current_pos > 5){
      int i;
      for(i=current_pos; i<buf_size-3; i++){
	if(buf[i]==0 && buf[i+1]==0 && buf[i+2]==1 && buf[i+3]==0xB6){
	  startcode_found=1;
	  break;
	}
      }
    }
    if(s->gb.buffer == s->bitstream_buffer && buf_size>7 && s->xvid_build>=0){ //xvid style
      startcode_found=1;
      current_pos=0;
    }

    if(startcode_found){
      av_fast_malloc(
		     &s->bitstream_buffer,
		     &s->allocated_bitstream_buffer_size,
		     buf_size - current_pos + FF_INPUT_BUFFER_PADDING_SIZE);
      if (!s->bitstream_buffer)
	return AVERROR(ENOMEM);
      memcpy(s->bitstream_buffer, buf + current_pos, buf_size - current_pos);
      s->bitstream_buffer_size= buf_size - current_pos;
    }
  }

 intrax8_decoded:
  //ff_er_frame_end(s);//hpwang 20101119

  if (avctx->hwaccel) {
    if (avctx->hwaccel->end_frame(avctx) < 0)
      return -1;
  }

  MPV_frame_end(s);

  if (dFRM->edge)
    mpeg4_tile_stuff(s);

#ifdef JZC_PMON_ALLFRM
  PMON_OFF(allfrm);
  all_pmon_cnt++;
  pmon_all += allfrm_pmon_val/s->mb_width/s->mb_height;
  printf("pmon_all; cur:%d avg:%lld cnt:%d\n",
	 allfrm_pmon_val/s->mb_width/s->mb_height, pmon_all, all_pmon_cnt);
#endif

  linear_crc_frame(s);
  //rota_crc_frame(s);
  if (0){
  //if (mpFrame == 5){
    int yidx[4];
    int uidx[4];
    int vidx[4];
        
    //printf("Frame34 cu 0x%08x\n", s->current_picture.data[0]);
          
    yidx[0] = 0;
    yidx[1] = s->mb_width;
    //yidx[1] = 3;
    yidx[2] = 0;
    yidx[3] = s->mb_height;
    //yidx[3] = 1;

    uidx[0] = 0;
    uidx[1] = s->mb_width;
    uidx[2] = 0;
    uidx[3] = s->mb_height;
    //uidx[3] = 31;
            
    vidx[0] = 0;
    vidx[1] = s->mb_width;
    vidx[2] = 0;
    vidx[3] = s->mb_height;
        
    print_frminfo(s, yidx, uidx, vidx);

    yidx[0] = 0;
    yidx[1] = s->mb_height;
    //yidx[1] = 3;
    yidx[2] = 0;
    yidx[3] = s->mb_width;
    //yidx[3] = 1;

    uidx[0] = 0;
    uidx[1] = s->mb_height;
    uidx[2] = 0;
    uidx[3] = s->mb_width;
    //uidx[3] = 31;
            
    vidx[0] = 0;
    vidx[1] = s->mb_height;
    vidx[2] = 0;
    vidx[3] = s->mb_width;
    //print_rota_frm(s, yidx, uidx, vidx);
  } 
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

#ifdef PRINT_FRAME_TIME
  av_log(avctx, AV_LOG_DEBUG, "%"PRId64"\n", rdtsc()-time);
#endif

  return get_consumed_bytes(s, buf_size);
}
