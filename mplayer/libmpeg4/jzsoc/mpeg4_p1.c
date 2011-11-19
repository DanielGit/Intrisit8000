#define VP6_P1_USE_PADDR
#define MPEG4_P1_USE_PADDR

#include "jzsys.h"
#include "jzasm.h"
#include "jzmedia.h"
#include "mpeg4_p1_type.h"
//#include "../../libjzcommon/jz4760e_2ddma_hw.h"
#include "jz4760_2ddma_hw.h"
#include "mpeg4_dcore.h"
#include "mpeg4_tcsm0.h"
#include "mpeg4_tcsm1.h"
//include "jz4760_mc_hw.h"
#include "../../libjzcommon/jz4760e_dcsc.h"

#ifdef JZC_PMON_P1
#include "../libjzcommon/jz4760e_aux_pmon.h"
#endif

#ifdef JZC_PMON_P1MB
PMON_CREAT(p1mc);
int p1mc;
#endif
#include "mpeg4_p1_mc.c"

#define mpeg4_STOP_P1()						\
  ({								\
    ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=0;	\
    *((volatile int *)(TCSM0_PADDR(TCSM0_P1_TASK_DONE))) = 0x1;	\
    i_nop;							\
    i_nop;							\
    i_wait();							\
  })

#include "mpeg4_aux_idct.c"
#define CODEC_FLAG_PSNR            0x8000
#define FF_MB_DECISION_RD     2
#define CODEC_FLAG_GRAY            0x2000
#define ENABLE_GRAY 1

#define __p1_text __attribute__ ((__section__ (".p1_text")))
#define __p1_main __attribute__ ((__section__ (".p1_main")))
#define __p1_data __attribute__ ((__section__ (".p1_data")))

#ifdef JZC_PMON_P1MB
PMON_CREAT(p1test);
PMON_CREAT(ckmc);
PMON_CREAT(stmc);
PMON_CREAT(idct);
PMON_CREAT(plp0);
PMON_CREAT(plp1);
PMON_CREAT(edge);
PMON_CREAT(wait);
int ckmc,stmc,idct,plp0,plp1,edge,wait,p1test;
int wtct;
#endif

#ifdef JZC_PMON_P1FRM
PMON_CREAT(p1frm1);
PMON_CREAT(p1frm2);
#endif

#ifdef JZC_ROTA90_OPT
int rota_mx;
uint32_t rota_picture_ptr[2];
#endif

extern int _gp;

uint32_t current_picture_ptr[3];
MPEG4_Frame_GlbARGs *dFRM;
MPEG4_MB_DecARGs *dMB, *dMB_L, *dMB_N;
//volatile int *dbg_ptr = TCSM1_DBG_BUF;

__p1_main int main() {
  //int tmp_gp;
  S32I2M(xr16, 0x3);
  //tmp_gp= i_la(_gp);

  int xchg_tmp, count, i, j;
  int *gp0_chain_ptr, *gp1_chain_ptr;
  uint8_t *dest_y;
  uint8_t *dest_c;
  uint8_t *yuv_dest = RECON_YBUF1;
  uint32_t *buf_use = RECON_BUF_USE;
  const uint16_t off_tab[6] = {0, 8, 8 * RECON_BUF_STRD, 8 * RECON_BUF_STRD + 8, 16*RECON_BUF_STRD, 16*RECON_BUF_STRD+8};

  volatile int *mbnum_wp=TCSM1_MBNUM_WP;
  volatile int *mbnum_rp=TCSM1_MBNUM_RP;
  volatile int *addr_rp=TCSM1_ADDR_RP;

  volatile int *dbg_ptr = TCSM1_DBG_BUF;

  uint32_t src_ptr,dst_ptr,src_cptr,dst_cptr;
  uint16_t edge_type=1;
  uint16_t mulslice = 0;
  count = 0;

#ifdef JZC_ROTA90_OPT
  uint8_t *rota_buf1;
  uint8_t *rota_buf2;
  uint8_t *rota_buf3;
  uint8_t *rota_buf4;

  rota_buf1 = ROTA_Y_BUF;
  rota_buf2 = ROTA_C_BUF;

  rota_buf3 = ROTA_Y_BUF1;
  rota_buf4 = ROTA_C_BUF1;
  int strd = 16;
  uint8_t *t1,*t2,*t3,*t4;
#endif

  *mbnum_rp=0;
  *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);
  dFRM=TCSM1_DFRM_BUF;

  dMB = TASK_BUF1;//curr mb
  dMB_L = TASK_BUF0;
  dMB_N = TASK_BUF2;
  gp0_chain_ptr = DDMA_GP0_DES_CHAIN;
  gp0_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
  set_gp0_dha(TCSM1_PADDR(DDMA_GP0_DES_CHAIN));
  gp1_chain_ptr = DDMA_GP1_DES_CHAIN;

  while(*mbnum_wp<=*mbnum_rp+2);//wait until the first two mb is ready

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dMB);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,64,TASK_BUF_LEN);
  set_gp0_dcs();
  poll_gp0_end();

  (*mbnum_rp)++;
  *addr_rp+= (TASK_BUF_LEN);
  if((int)(*addr_rp)>(0x132B4000-TASK_BUF_LEN))
    *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);
  //*addr_rp=0x132B2000;    
  ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dMB_N);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,64,TASK_BUF_LEN);
  set_gp0_dcs();
  poll_gp0_end();

  *((volatile int *)(TCSM1_P0_POLL)) = 0;
  *((volatile int *)(TCSM1_P1_POLL)) = 1;

  (*mbnum_rp)++;
  *addr_rp += (TASK_BUF_LEN);
  if((int)(*addr_rp)>(0x132B4000-TASK_BUF_LEN))
    *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);
  ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;

  gp1_chain_ptr[2]=GP_STRD(RECON_BUF_STRD,GP_FRM_NML,16);   
  gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,16,16*16);	

  gp1_chain_ptr[6]=GP_STRD(RECON_BUF_STRD,GP_FRM_NML,16);   
  gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,16,16*8);

#ifdef JZC_ROTA90_OPT
  gp1_chain_ptr[10]=GP_STRD(RECON_BUF_STRD,GP_FRM_NML,16);
  gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,16,16*16);	

  gp1_chain_ptr[14]=GP_STRD(RECON_BUF_STRD,GP_FRM_NML,16);   
  gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,16,16*8);
#else
  gp1_chain_ptr[10] = GP_STRD(4,GP_FRM_NML,4);
  gp1_chain_ptr[11] = GP_UNIT(GP_TAG_LK,4,4);
  gp1_chain_ptr[14] = GP_STRD(4,GP_FRM_NML,4);
  gp1_chain_ptr[15] = GP_UNIT(GP_TAG_LK,4,4);
#endif
  gp1_chain_ptr[16]=TCSM1_PADDR(EDGE_YOUT_BUF);
  gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
  gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);

  gp1_chain_ptr[20]=TCSM1_PADDR(EDGE_YOUT_BUF);
  gp1_chain_ptr[22]=GP_STRD(256,GP_FRM_NML,256);
  gp1_chain_ptr[23]=GP_UNIT(GP_TAG_LK,256,256);

  gp1_chain_ptr[24]=TCSM1_PADDR(EDGE_COUT_BUF);
  gp1_chain_ptr[26]=GP_STRD(128,GP_FRM_NML,128);
  gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,128,128);

  gp1_chain_ptr[28]=TCSM1_PADDR(EDGE_COUT_BUF);
  gp1_chain_ptr[30]=GP_STRD(128,GP_FRM_NML,128);
  gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,128,128);

  gp1_chain_ptr[32] = TCSM0_PADDR(TCSM0_P1_POLL);
  gp1_chain_ptr[33] = TCSM1_PADDR(TCSM1_P1_POLL);
  gp1_chain_ptr[34] = GP_STRD(4,GP_FRM_NML,4);
  gp1_chain_ptr[35] = GP_UNIT(GP_TAG_UL,4,4);

#ifdef JZC_PMON_P1MB
  PMON_CLEAR(ckmc);
  PMON_CLEAR(stmc);
  PMON_CLEAR(idct);
  PMON_CLEAR(plp0);
  PMON_CLEAR(plp1);
  PMON_CLEAR(edge);
  PMON_CLEAR(p1test);
  PMON_CLEAR(wait);
  PMON_CLEAR(p1mc);
  ckmc=0;stmc=0;idct=0;plp0=0;plp1=0;edge=0;p1test=0;wait=0;
  wtct=0;
  p1mc=0;
#endif

  dest_y = dFRM->current_picture_data[0] - 1280;
  dest_c = dFRM->current_picture_data[1] - 640;

  if (dMB->mb_x != 0 || dMB->mb_y != 0){//for mul-slice
    dest_y += (dMB->mb_y * dFRM->mb_width * 256 + dMB->mb_x * 256 + dMB->mb_y * 1024 + 1024 - 256);
    //+1024 is correct of -1280, -256 is twice dest cal in while
    dest_c += (dMB->mb_y * dFRM->mb_width * 128 + dMB->mb_x * 128 + dMB->mb_y * 512 + 512 - 128);
    if (dMB->mb_x == 0 || dMB->mb_x == 1){//insure step over the edge buf
      dest_y -= 1024;
      dest_c -= 512;
    }
    count += (dMB->mb_y * dFRM->mb_width + dMB->mb_x);
    mulslice=1;

    int lx,ly;
    lx = dMB->mb_x;
    ly = dMB->mb_y;
    if (lx == 0)
      edge_type = 1;
    else if (lx == (dFRM->mb_width-1))
      edge_type = 3;
    else if (ly == 0)
      edge_type = 2;
    else if (ly == (dFRM->mb_height-1))
      edge_type = 5;
    else
      edge_type = 4;
  }

#ifdef JZC_PMON_P1FRM
  PMON_CLEAR(p1frm1);
  PMON_CLEAR(p1frm2);
#endif

  while (dMB->real_num > 0){
#ifdef JZC_PMON_P1FRM
    PMON_ON(p1frm1);
#endif

#if 1
    int mb_x, mb_y;
    mb_x = dMB->mb_x;
    mb_y = dMB->mb_y;

    if (count > 0){
      if (dFRM->draw_horiz_band){
      	dest_y += ((count-1)%dFRM->mb_width)==0?1280:256;
      	dest_c += ((count-1)%dFRM->mb_width)==0?640:128;
      }else{
	dest_y = dFRM->current_picture_data[0] + dMB_L->mb_x * 256;
	dest_c = dFRM->current_picture_data[1] + dMB_L->mb_x * 128;
      }
    }

#if 1

    if ((mb_x == 0 && mb_y == 0) || mulslice == 1){
      yuv_dest = RECON_YBUF0;
      *buf_use = 0;

      if (!dMB->mb_intra){//nees start mc
	MPV_motion_p1(dMB->mv_type, dFRM->quarter_sample, dMB->mv_dir, dFRM->no_rounding,
		      mb_x, mb_y, dMB->mv, dFRM->width, dFRM->height, dFRM->workaround_bugs);
      }

      if (mulslice == 1){
	mulslice = 0;
	goto mulslice;
      }
      goto skip_all;
    }else{
      if (!dMB_L->mb_intra){//need check mc result
#ifdef JZC_PMON_P1MB
	PMON_ON(ckmc);
#endif	
	check_mc_result();
#ifdef JZC_PMON_P1MB
	PMON_OFF(ckmc);
	ckmc++;
#endif
      }
      if (*buf_use == 0){
	yuv_dest = RECON_YBUF0;//this is last buf for idct now.
	*buf_use = 1;
      }else{
	yuv_dest = RECON_YBUF1;
	*buf_use = 0;
      }

      if (!dMB->mb_intra){//nees start mc
#ifdef JZC_PMON_P1MB
      PMON_ON(stmc);
#endif
	MPV_motion_p1(dMB->mv_type, dFRM->quarter_sample, dMB->mv_dir, dFRM->no_rounding,
		      mb_x, mb_y, dMB->mv, dFRM->width, dFRM->height, dFRM->workaround_bugs);
#ifdef JZC_PMON_P1MB
	PMON_OFF(stmc);
	stmc++;
#endif
      }
    }
#endif
    //mbnum++;
    { //FIXME precal
#ifdef JZC_PMON_P1MB
      PMON_ON(p1test);
#endif
    //uint8_t *dest_y, *dest_cb, *dest_cr;
      int dct_linesize, dct_offset;
      //op_pixels_func (*op_pix)[4];
      //qpel_mc_func (*op_qpix)[16];
      const int linesize= 16; //not s->linesize as this would be wrong for field pics
      const int uvlinesize= 16;
      const int readable= dFRM->pict_type != B_TYPE || dFRM->draw_horiz_band; //|| lowres_flag;
      const int block_size= 8;
      /* avoid copy if macroblock skipped in last frame too */
      /* skip only during decoding as we might trash the buffers during encoding a bit */

      dct_linesize = linesize << dMB->interlaced_dct;
      dct_offset =(dMB->interlaced_dct)? linesize : linesize*block_size;
      //if(readable){
      //dest_y=  dMB_L->dest[0];
      //dest_cb= dMB_L->dest[1];
	//dest_cr= dMB->dest[2];
	//dest_cr = dMB_L->dest[1] + 8;
      //}else{
      //dest_y = dFRM->b_scratchpad;
      //dest_cb= dFRM->b_scratchpad+16*linesize;
      //dest_cr= dFRM->b_scratchpad+32*linesize;
      //}
#ifdef JZC_PMON_P1MB
      PMON_OFF(p1test);
      p1test++;
#endif
      if (!dMB_L->mb_intra) {
	/* skip dequant / idct if we are really late ;) */
#if 0
	if(dFRM->hurry_up>1) goto skip_idct;
	if(dMB->skip_idct){
	  if(  (dMB->skip_idct >= 8 && dFRM->pict_type == B_TYPE)
	       ||(dMB->skip_idct >= 32 && dFRM->pict_type != I_TYPE)
	       || dMB->skip_idct >= 48)
	    goto skip_idct;
	}
#endif

	//goto skip_dbg;
	/* add dct residue */
#ifdef JZC_PMON_P1MB
	PMON_ON(idct);
#endif
	if(!(dFRM->h263_msmpeg4 || dFRM->codec_id==1 || dFRM->codec_id==2 || (dFRM->codec_id==13 && !dFRM->mpeg_quant))){
	  add_dequant_dct_opt(dMB_L->block[0], 0, yuv_dest + off_tab[0], RECON_BUF_STRD, dMB_L->qscale);
	  add_dequant_dct_opt(dMB_L->block[1], 1, yuv_dest + off_tab[1], RECON_BUF_STRD, dMB_L->qscale);
	  add_dequant_dct_opt(dMB_L->block[2], 2, yuv_dest + off_tab[2], RECON_BUF_STRD, dMB_L->qscale);
	  add_dequant_dct_opt(dMB_L->block[3], 3, yuv_dest + off_tab[3], RECON_BUF_STRD, dMB_L->qscale);

	  add_dequant_dct_opt(dMB_L->block[4], 4, yuv_dest + off_tab[4], RECON_BUF_STRD, dMB_L->chroma_qscale);
	  add_dequant_dct_opt(dMB_L->block[5], 5, yuv_dest + off_tab[5], RECON_BUF_STRD, dMB_L->chroma_qscale);
	} else if(dFRM->codec_id != 19){
	  add_dct_opt(dMB_L->block[0], 0, yuv_dest + off_tab[0], RECON_BUF_STRD);
	  add_dct_opt(dMB_L->block[1], 1, yuv_dest + off_tab[1], RECON_BUF_STRD);
	  add_dct_opt(dMB_L->block[2], 2, yuv_dest + off_tab[2], RECON_BUF_STRD);
	  add_dct_opt(dMB_L->block[3], 3, yuv_dest + off_tab[3], RECON_BUF_STRD);

	  add_dct_opt(dMB_L->block[4], 4, yuv_dest + off_tab[4], RECON_BUF_STRD);
	  add_dct_opt(dMB_L->block[5], 5, yuv_dest + off_tab[5], RECON_BUF_STRD);
	}
#ifdef JZC_PMON_P1MB
	PMON_OFF(idct);
	idct++;
#endif
      } else {
#ifdef JZC_PMON_P1MB
	PMON_ON(idct);
#endif
	/* dct only in intra block */
	if(!(dFRM->codec_id==1 || dFRM->codec_id==2)){
	  put_dct_opt(dMB_L->block[0], 0, yuv_dest + off_tab[0], dMB_L->qscale);
	  put_dct_opt(dMB_L->block[1], 1, yuv_dest + off_tab[1], dMB_L->qscale);
	  put_dct_opt(dMB_L->block[2], 2, yuv_dest + off_tab[2], dMB_L->qscale);
	  put_dct_opt(dMB_L->block[3], 3, yuv_dest + off_tab[3], dMB_L->qscale);

	  put_dct_opt(dMB_L->block[4], 4, yuv_dest + off_tab[4], dMB_L->chroma_qscale);
	  put_dct_opt(dMB_L->block[5], 5, yuv_dest + off_tab[5], dMB_L->chroma_qscale);
	}else{
          int i,yuv_len[6];
	  for(i=0;i<6;i++)
            yuv_len[i] = (dMB->val[i]>>3) + 1;    
	  ff_simple_idct_put_mxu(yuv_dest + off_tab[0],yuv_len[0], dct_linesize, dMB->block[0]);
	  ff_simple_idct_put_mxu(yuv_dest + off_tab[1],yuv_len[1], dct_linesize, dMB->block[1]);
	  ff_simple_idct_put_mxu(yuv_dest + off_tab[2],yuv_len[2], dct_linesize, dMB->block[2]);
	  ff_simple_idct_put_mxu(yuv_dest + off_tab[3],yuv_len[3], dct_linesize, dMB->block[3]);

	  ff_simple_idct_put_mxu(yuv_dest + off_tab[4],yuv_len[4], uvlinesize, dMB->block[4]);                        
	  ff_simple_idct_put_mxu(yuv_dest + off_tab[5],yuv_len[5], uvlinesize, dMB->block[5]);                        
	}
#ifdef JZC_PMON_P1MB
	PMON_OFF(idct);
	idct++;
#endif
      }
#if 0
    skip_idct:
      if(!readable){
	dFRM->put_pixels_tab[0][0](dMB->dest[0], dest_y ,   linesize,16);
	dFRM->put_pixels_tab[dFRM->chroma_x_shift][0](dMB->dest[1], dest_cb, uvlinesize,16 >> dFRM->chroma_y_shift);
	dFRM->put_pixels_tab[dFRM->chroma_x_shift][0](dMB->dest[2], dest_cr, uvlinesize,16 >> dFRM->chroma_y_shift);
      }
#endif
    }

  skip_dbg:
#ifdef JZC_ROTA90_OPT
    for(i = 0;i < 4;i++){
      t1 = yuv_dest + 16*i*4 - 4;      
      t2 = rota_buf1 + 12 - 4*i - strd; 
      for(j = 0; j < 4; j++){	
	S32LDI (xr1, t1, 0x4);       //xr1: x3  ,x2  ,x1  ,x0
	S32LDD (xr2, t1, 0x10);      //xr2: x19 ,x18 ,x17 ,x16
	S32LDD (xr3, t1, 0x20);      //xr3: x35 ,x34 ,x33 ,x32
	S32LDD (xr4, t1, 0x30);      //xr4: x51 ,x50 ,x49 ,x48
	   
	S32SFL(xr2,xr2,xr1,xr1,ptn0); //xr2:x19,x3,x18,x2 xr1:x17,x1,x16,x0
	S32SFL(xr4,xr4,xr3,xr3,ptn0); //xr4:x51,x35,x50,x34 xr3:x49,x33,x48,x32	
	S32SFL(xr4,xr4,xr2,xr2,ptn3); //xr4:x51,x35,x19,x3 xr2:x50,x34,x18,x2
	S32SFL(xr3,xr3,xr1,xr1,ptn3); //xr3:x49,x33,x17,x1 xr1:x48,x32,x16,x0	    
	S32SDIVR(xr1, t2, strd, 0x0);
	S32SDIVR(xr3, t2, strd, 0x0);
	S32SDIVR(xr2, t2, strd, 0x0);
	S32SDIVR(xr4, t2, strd, 0x0);	                
      }  
    }
    strd = 16;
    for(i = 0;i < 2;i++){
      t1 = yuv_dest + off_tab[4] + 16*i*4 - 4;      
      t2 = rota_buf2 + 4 - 4*i - strd;
      t3 = t1 + 8;
      t4 = t2 + 8;
      for(j = 0; j < 2; j++){	
	S32LDI (xr1, t1, 0x4);       //xr1: x3  ,x2  ,x1  ,x0
	S32LDD (xr2, t1, 0x10);      //xr2: x19 ,x18 ,x17 ,x16
	S32LDD (xr3, t1, 0x20);      //xr3: x35 ,x34 ,x33 ,x32
	S32LDD (xr4, t1, 0x30);      //xr4: x51 ,x50 ,x49 ,x48

	S32LDI (xr5, t3, 0x4);       //xr5: x3  ,x2  ,x1  ,x0
	S32LDD (xr6, t3, 0x10);      //xr6: x19 ,x18 ,x17 ,x16
	S32LDD (xr7, t3, 0x20);      //xr7: x35 ,x34 ,x33 ,x32
	S32LDD (xr8, t3, 0x30);      //xr8: x51 ,x50 ,x49 ,x48
	    
	S32SFL(xr2,xr2,xr1,xr1,ptn0); //xr2:x19,x3,x18,x2 xr1:x17,x1,x16,x0
	S32SFL(xr4,xr4,xr3,xr3,ptn0); //xr4:x51,x35,x50,x34 xr3:x49,x33,x48,x32	
	S32SFL(xr4,xr4,xr2,xr2,ptn3); //xr4:x51,x35,x19,x3 xr2:x50,x34,x18,x2
	S32SFL(xr3,xr3,xr1,xr1,ptn3); //xr3:x49,x33,x17,x1 xr1:x48,x32,x16,x0

	S32SFL(xr6,xr6,xr5,xr5,ptn0); //xr6:x19,x3,x18,x2 xr5:x17,x1,x16,x0
	S32SFL(xr8,xr8,xr7,xr7,ptn0); //xr8:x51,x35,x50,x34 xr7:x49,x33,x48,x32	
	S32SFL(xr8,xr8,xr6,xr6,ptn3); //xr8:x51,x35,x19,x3 xr6:x50,x34,x18,x2
	S32SFL(xr7,xr7,xr5,xr5,ptn3); //xr7:x49,x33,x17,x1 xr5:x48,x32,x16,x0
	    
	S32SDIVR(xr1, t2, strd, 0x0);
	S32SDIVR(xr3, t2, strd, 0x0);
	S32SDIVR(xr2, t2, strd, 0x0);
	S32SDIVR(xr4, t2, strd, 0x0);

	S32SDIVR(xr5, t4, strd, 0x0);
	S32SDIVR(xr7, t4, strd, 0x0);
	S32SDIVR(xr6, t4, strd, 0x0);
	S32SDIVR(xr8, t4, strd, 0x0);	                
      }  
    }
    
    rota_mx = dFRM->mb_height - 1 - dMB_L->mb_y;
    rota_picture_ptr[0] = dFRM->rota_picture_data[0] + (dMB_L->mb_x*dFRM->mb_height*256 + rota_mx*256);
    rota_picture_ptr[1] = dFRM->rota_picture_data[1] + (dMB_L->mb_x*dFRM->mb_height*128 + rota_mx*128);
#endif

#ifdef JZC_PMON_P1MB
    PMON_ON(plp1);
#endif
    //poll_gp1_end();
    while(*((volatile int *)(TCSM1_P1_POLL)) == 0);
    *((volatile int *)(TCSM1_P1_POLL)) = 0;
#ifdef JZC_PMON_P1MB
    PMON_OFF(plp1);
    plp1++;
#endif

    gp1_chain_ptr[0]=TCSM1_PADDR(yuv_dest);
    gp1_chain_ptr[1]=(dest_y);
    gp1_chain_ptr[4]=TCSM1_PADDR(yuv_dest + off_tab[4]);
    gp1_chain_ptr[5]=(dest_c);
#ifdef JZC_ROTA90_OPT
    gp1_chain_ptr[8]=TCSM1_PADDR(rota_buf1);
    gp1_chain_ptr[9]=(rota_picture_ptr[0]);
    gp1_chain_ptr[12]=TCSM1_PADDR(rota_buf2);
    gp1_chain_ptr[13]=(rota_picture_ptr[1]);
#else
    gp1_chain_ptr[8] = TCSM0_PADDR(TCSM0_P1_POLL);
    gp1_chain_ptr[9] = TCSM1_PADDR(TCSM1_DBG_BUF+256);
    gp1_chain_ptr[12] = TCSM0_PADDR(TCSM0_P1_POLL);
    gp1_chain_ptr[13] = TCSM1_PADDR(TCSM1_DBG_BUF+256);
#endif
    if (dFRM->edge){
#ifdef JZC_PMON_P1MB
      PMON_ON(edge);
#endif
      switch(edge_type){
      case 4://middle
	//gp1_chain_ptr[7]=GP_UNIT(GP_TAG_UL,128,128);
	gp1_chain_ptr[16] = TCSM0_PADDR(TCSM0_P1_POLL);
	gp1_chain_ptr[17] = TCSM1_PADDR(TCSM1_P1_POLL);
	gp1_chain_ptr[18] = GP_STRD(4,GP_FRM_NML,4);
	gp1_chain_ptr[19] = GP_UNIT(GP_TAG_UL,4,4);
	edge_type=(count+1)%dFRM->mb_width==0?3:edge_type;
	break;
      case 1://left edge
	edge_type=(count+1)>dFRM->mb_width*(dFRM->mb_height-1)?5:(count+1)>dFRM->mb_width?4:2;
      case 3://right edge
	src_ptr=yuv_dest-(edge_type!=3?16:1);
	dst_ptr=EDGE_YOUT_BUF-16;
	src_cptr=yuv_dest + off_tab[4] - (edge_type!=3?8:1);
	dst_cptr=EDGE_COUT_BUF-8;
	for(i=0;i<16;i++){
	  S8LDI(xr5,src_ptr,16,7);
	  S32SDI(xr5,dst_ptr,16);
	  S32STD(xr5,dst_ptr,4);
	  S32STD(xr5,dst_ptr,8);
	  S32STD(xr5,dst_ptr,12);
	  S8LDI(xr6,src_cptr,8,7);
	  S32SDI(xr6,dst_cptr,8);
	  S32STD(xr6,dst_cptr,4);
	}
	gp1_chain_ptr[16]=TCSM1_PADDR(EDGE_YOUT_BUF);
	gp1_chain_ptr[17]=(dest_y+(edge_type!=3?-512:512));
	gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);
	gp1_chain_ptr[21]=(dest_y+(edge_type!=3?-256:256));
	gp1_chain_ptr[25]=(dest_c+(edge_type!=3?-256:256));
	gp1_chain_ptr[29]=(dest_c+(edge_type!=3?-128:128));

	edge_type=(edge_type!=3?edge_type:1);
	break;
      case 2://top edge
      case 5://bottom edge
	src_ptr=yuv_dest+(edge_type==2?0:240);
	dst_ptr=EDGE_YOUT_BUF-16;
	src_cptr=yuv_dest + off_tab[4] + (edge_type==2?0:112);
	dst_cptr=EDGE_COUT_BUF-16;
	S32LDD(xr5,src_ptr,0);
	S32LDD(xr6,src_ptr,4);
	S32LDD(xr7,src_ptr,8);
	S32LDD(xr8,src_ptr,12);
	S32LDD(xr9,src_cptr,0);
	S32LDD(xr10,src_cptr,4);
	S32LDD(xr11,src_cptr,8);
	S32LDD(xr12,src_cptr,12);
	for(i=0;i<16;i++){
	  S32SDI(xr5,dst_ptr,16);
	  S32STD(xr6,dst_ptr,4);
	  S32STD(xr7,dst_ptr,8);
	  S32STD(xr8,dst_ptr,12);
	}
	for(i=0;i<8;i++){
	  S32SDI(xr9,dst_cptr,16);
	  S32STD(xr10,dst_cptr,4);
	  S32STD(xr11,dst_cptr,8);
	  S32STD(xr12,dst_cptr,12);
	}
	gp1_chain_ptr[16]=TCSM1_PADDR(EDGE_YOUT_BUF);
	gp1_chain_ptr[17]=(dest_y+((edge_type==2?-dFRM->linesize:dFRM->linesize)<<1));
	gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
	gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);
	gp1_chain_ptr[21]=(dest_y+(edge_type==2?-dFRM->linesize:dFRM->linesize));
	gp1_chain_ptr[25]=(dest_c+(edge_type==2?-dFRM->linesize:dFRM->linesize));
	gp1_chain_ptr[29]=(dest_c+(edge_type==2?-dFRM->uvlinesize:dFRM->uvlinesize));

	edge_type=(count + 1)%dFRM->mb_width==0?3:edge_type;
	break;
      }
#ifdef JZC_PMON_P1MB
      PMON_OFF(edge);
      edge++;
#endif
    }

    set_gp1_dha(TCSM1_PADDR(gp1_chain_ptr));
    set_gp1_dcs();
    //poll_gp1_end();
#endif

  skip_all:

#ifdef JZC_PMON_P1FRM
    PMON_OFF(p1frm1);
#endif

#ifdef JZC_PMON_P1MB
    PMON_ON(wait);
#endif
    while(*mbnum_wp<=*mbnum_rp+2){
#ifdef JZC_PMON_P1MB
      wtct++;
#endif
    }//wait until the next next mb is ready
#ifdef JZC_PMON_P1MB
    PMON_OFF(wait);
    wait++;
#endif

#ifdef JZC_PMON_P1FRM
    PMON_ON(p1frm2);
#endif

    if (count > 0){
#ifdef JZC_PMON_P1MB
      PMON_ON(plp0);
#endif
      //poll_gp0_end();
      while(*((volatile int *)(TCSM1_P0_POLL)) == 0);
      *((volatile int *)(TCSM1_P0_POLL)) = 0;
#ifdef JZC_PMON_P1MB
      PMON_OFF(plp0);
      plp0++;
#endif
      *addr_rp+=(TASK_BUF_LEN);
      if((int)(*addr_rp)>=(TCSM0_PADDR(TCSM0_END)-TASK_BUF_LEN))
	*addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);
      ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;
      
      (*mbnum_rp)++;
    }

  mulslice:
    XCHG3(dMB_L,dMB,dMB_N,xchg_tmp);
#ifdef JZC_ROTA90_OPT
    XCHG2(rota_buf1,rota_buf3,xchg_tmp);
    XCHG2(rota_buf2,rota_buf4,xchg_tmp);
#endif
    gp0_chain_ptr[0]=addr_rp[0];
    gp0_chain_ptr[1]=TCSM1_PADDR(dMB_N);
    gp0_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,TASK_BUF_LEN);

    gp0_chain_ptr[4] = TCSM0_PADDR(TCSM0_P0_POLL);
    gp0_chain_ptr[5] = TCSM1_PADDR(TCSM1_P0_POLL);
    gp0_chain_ptr[6] = GP_STRD(4,GP_FRM_NML,4);
    gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,4,4);

    set_gp0_dcs(); 

    count++;
#ifdef JZC_PMON_P1FRM
    PMON_OFF(p1frm2);
#endif
  }

#ifdef JZC_PMON_P1MB
  if (ckmc > 0){
    dbg_ptr[0] = ckmc_pmon_val_cclk/ckmc;
    dbg_ptr[10] = ckmc_pmon_val_piperdy/ckmc;
  }
  if (stmc > 0){
    dbg_ptr[1] = stmc_pmon_val_cclk/stmc;
    dbg_ptr[11] = stmc_pmon_val_piperdy/stmc;
  }
  if (idct > 0){
    dbg_ptr[2] = idct_pmon_val_cclk/idct;
    dbg_ptr[12] = idct_pmon_val_piperdy/idct;
  }
  if (edge > 0){
    dbg_ptr[3] = edge_pmon_val_cclk/edge;
    dbg_ptr[13] = edge_pmon_val_piperdy/edge;
  }
  if (plp0 > 0){
    dbg_ptr[4] = plp0_pmon_val_cclk/plp0;
    dbg_ptr[14] = plp0_pmon_val_piperdy/plp0;
  }
  if (plp1 > 0){
    dbg_ptr[5] = plp1_pmon_val_cclk/plp1;
    dbg_ptr[15] = plp1_pmon_val_piperdy/plp1;
  }
  if (wait > 0){
    dbg_ptr[6] = wait_pmon_val_cclk/wait;
    dbg_ptr[16] = wait_pmon_val_piperdy/wait;
  }
  if (p1test > 0){
    dbg_ptr[7] = p1test_pmon_val_cclk/p1test;
    dbg_ptr[17] = p1test_pmon_val_piperdy/p1test;
  }
  if (p1mc > 0){
    dbg_ptr[8] = p1mc_pmon_val_cclk/p1mc;
    dbg_ptr[18] = p1mc_pmon_val_piperdy/p1mc;
  }
  dbg_ptr[20] = wtct;
#endif

#ifdef JZC_PMON_P1FRM
  dbg_ptr[22] = p1frm1_pmon_val_cclk + p1frm2_pmon_val_cclk;
#endif

  poll_gp1_end();

  *((volatile int *)TCSM0_PADDR(TCSM0_P1_TASK_DONE)) = 0x1;
  i_nop;  
  i_nop;    
  i_nop;      
  i_nop;  
  i_wait();

  return 0;
}
