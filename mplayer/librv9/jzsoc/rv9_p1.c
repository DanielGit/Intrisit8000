#define P1_USE_PADDR

#include "../../libjzcommon/jzsys.h"
#include "../../libjzcommon/jzasm.h"
#include "../../libjzcommon/jzmedia.h"
#include "rv9_p1_type.h"
#include "../../libjzcommon/jz4760e_2ddma_hw.h"
#include "rv9_dcore.h"
#include "rv9_tcsm0.h"
#include "rv9_tcsm1.h"
#include "jz4760e_rv_dblk.h"
#include "../../libjzcommon/jz4760e_dcsc.h"
#include "../../libjzcommon/jz4760e_idct_hw.h"

#if 0
#define RV9_STOP_P1()						\
  ({								\
    ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=0;	\
    *((volatile int *)(TCSM0_PADDR(TCSM0_P1_TASK_DONE))) = 0x1;	\
    i_nop;							\
    i_nop;							\
    i_wait();							\
  })
#endif
//volatile int *dbg_ptr;
//volatile unsigned char intraup_y[1284];
//volatile unsigned char intraup_u[644];
//volatile unsigned char intraup_v[644];
unsigned char intraup_y[1284];
unsigned char intraup_u[644];
unsigned char intraup_v[644];
int mc_flag=0,last_mc_flag=0;
#include "rv9_p1_idct.c"
#include "rv9_p1_pred.c"
#include "rv9_p1_dq.c"
#include "rv9_p1_mc.c"

#define __p1_text __attribute__ ((__section__ (".p1_text")))
#define __p1_main __attribute__ ((__section__ (".p1_main")))
#define __p1_data __attribute__ ((__section__ (".p1_data")))

__p1_main int main() {
  S32I2M(xr16, 0x3);
  uint8_t top_left_tmp[3];
  int *gp0_chain_ptr, *gp1_chain_ptr, *gp1_chain_ptr1;
  uint8_t *edge_tb_ydst, *edge_tb_cdst, *edge_ydst, *edge_cdst;
  int i,j, k, count, blknum, blkoff, chain;
  RV9_MB_DecARGs *dMB, *dMB_L, *dMB_N;
  RV9_Slice_GlbARGs *dSlice, *dSlice_ex;
  int y_mb_line, uv_mb_line, tag3, lastline = 0;
  uint32_t src_ptr,dst_ptr,src_cptr,dst_cptr;
  uint16_t edge_type=2;
  RV9_Frame_LPtr *Lpicture;
  uint8_t *src1, *dst1, *src2, *dst2, *src3, *dst3, *src4, *dst4;
  RV9_XCH2_T *XCH2_T0, *XCH2_T1;

#ifdef JZC_ROTA90_OPT
  int tag1, tag2;
  int rota_mx, rota_my, rota_up_mx, rota_up_my;
  uint8_t *rota_ydst, *rota_cdst, *rota_up_ydst;
  int rota_y_mb_line, rota_uv_mb_line;  
#endif

  uint8_t *dblk_upout_addr0 = TCSM1_DBLK_UPOUT_Y0;
  uint8_t *dblk_upout_addr1 = TCSM1_DBLK_UPOUT_Y1;

  *(volatile int*)DBLK_END_FLAG = DBLK_SYN_VALUE;
  *(volatile int*)TCSM1_MOTION_DSA = (0x80000000 | 0xFFFF);
  uint8_t *motion_dha = (volatile uint8_t *)TCSM1_MOTION_DHA;
  uint8_t *motion_dsa = TCSM1_MOTION_DSA;

  int xchg_tmp; 
  volatile int *mbnum_wp=TCSM1_MBNUM_WP;
  volatile int *mbnum_rp=TCSM1_MBNUM_RP;
  volatile int *addr_rp=TCSM1_ADDR_RP;
  unsigned char *intra_top_y=intraup_y;
  unsigned char *intra_top_u=intraup_u;
  unsigned char *intra_top_v=intraup_v;

  unsigned char * recon_ptr0=RECON_PREVIOUS_YBUF0;
  unsigned char * recon_ptr1=RECON_PREVIOUS_YBUF1;
  unsigned char * recon_ptr2=RECON_PREVIOUS_YBUF2;

#ifdef JZC_ROTA90_OPT
  uint8_t *dblk_mb_ydst1 = TCSM1_DBLK_MBOUT_YDES;
  uint8_t *dblk_mb_ydst2 = TCSM1_DBLK_MBOUT_YDES1;
  uint8_t *dblk_mb_ydst3 = TCSM1_DBLK_MBOUT_YDES2;

  uint8_t *dblk_mb_udst1 = TCSM1_DBLK_MBOUT_UDES;
  uint8_t *dblk_mb_udst2 = TCSM1_DBLK_MBOUT_UDES1;
  uint8_t *dblk_mb_udst3 = TCSM1_DBLK_MBOUT_UDES2;
#else
  uint8_t *dblk_mb_ydst = TCSM1_DBLK_MBOUT_YDES;
  uint8_t *dblk_mb_udst = TCSM1_DBLK_MBOUT_UDES;
  uint8_t *dblk_mb_vdst = TCSM1_DBLK_MBOUT_VDES;
  uint8_t *dblk_mb_ydst1 = TCSM1_DBLK_MBOUT_YDES1;
  uint8_t *dblk_mb_udst1 = TCSM1_DBLK_MBOUT_UDES1;
  uint8_t *dblk_mb_vdst1 = TCSM1_DBLK_MBOUT_VDES1;
#endif

#ifdef JZC_ROTA90_OPT
  uint8_t *rota_mb_ybuf1 = TCSM1_ROTA_MB_YBUF1;
  uint8_t *rota_mb_ybuf2 = TCSM1_ROTA_MB_YBUF2;
  uint8_t *rota_mb_ybuf3 = TCSM1_ROTA_MB_YBUF3;
  uint8_t *rota_mb_cbuf1 = TCSM1_ROTA_MB_CBUF1;
  uint8_t *rota_mb_cbuf2 = TCSM1_ROTA_MB_CBUF2;
  uint8_t *rota_mb_cbuf3 = TCSM1_ROTA_MB_CBUF3;
  uint8_t *rota_upmb_ybuf1 = TCSM1_UP_ROTA_YBUF1;
  uint8_t *rota_upmb_ybuf2 = TCSM1_UP_ROTA_YBUF2;
#endif
  uint8_t *edge_bufy[2],*edge_bufc[2];
  uint8_t *up_ydst;
  edge_bufy[0] = TCSM1_EDGE_YBUF;
  edge_bufy[1] = TCSM1_EDGE_YBUF1;
  
  edge_bufc[0] = TCSM1_EDGE_CBUF;
  edge_bufc[1] = TCSM1_EDGE_CBUF1;
  uint8_t *up_edge_ydst1 = TCSM1_UP_EDGE_YBUF;
  uint8_t *up_edge_ydst2 = TCSM1_UP_EDGE_YBUF1;

  uint8_t  mb_x = 0;
  uint8_t  mb_x_d1=0, mb_x_d2=0, mb_x_d3=0;
  uint8_t  mb_y=0;
  uint8_t  mb_y_d1=0, mb_y_d2=0, mb_y_d3=0;

  volatile int *tcsm0_gp0_poll_end ,*tcsm0_gp1_poll_end;
  volatile int *tcsm1_gp0_poll_end ,*tcsm1_gp1_poll_end;

  tcsm1_gp0_poll_end  = (volatile int*)(TCSM1_GP0_POLL_END);
  *tcsm1_gp0_poll_end  = 0;
  tcsm0_gp0_poll_end  = (volatile int*)TCSM0_PADDR(TCSM0_GP0_POLL_END);
  *tcsm0_gp0_poll_end  = 0;

  tcsm1_gp1_poll_end  = (volatile int*)(TCSM1_GP1_POLL_END);
  *tcsm1_gp1_poll_end  = 0;
  tcsm0_gp1_poll_end  = (volatile int*)TCSM0_PADDR(TCSM0_GP1_POLL_END);
  *tcsm0_gp1_poll_end  = 0;

  last_mc_flag = 0;
  mc_flag = 0;
  //*(int*)DBLK_END_FLAG = DBLK_SYN_VALUE;
  uint32_t *idct  = TCSM1_IDCT_BUF;
  uint32_t *idct1 = TCSM1_IDCT_BUF1;

  *mbnum_rp=0;
  *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);

  dSlice    = (int*)TCSM1_SLICE_BUF0;
  dSlice_ex = (int*)TCSM1_SLICE_BUF1;

#ifdef JZC_DBLK_OPT
  int intra, inter16x16;
  XCH2_T0 = TCSM1_XCH2_T_BUF0;
  XCH2_T1 = TCSM1_XCH2_T_BUF1;
#endif

  dMB   = TASK_BUF1;//curr mb
  dMB_L = TASK_BUF0;
  dMB_N = TASK_BUF2;
  gp0_chain_ptr = DDMA_GP0_DES_CHAIN;
  gp1_chain_ptr = DDMA_GP1_DES_CHAIN;
  gp1_chain_ptr1= DDMA_GP1_DES_CHAIN1;

  gp1_chain_ptr[6]=GP_STRD(128,GP_FRM_NML,128);
  gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,128,128);
  gp1_chain_ptr[10]=GP_STRD(64,GP_FRM_NML,64);
  gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,64,64);

  gp1_chain_ptr1[6]=GP_STRD(128,GP_FRM_NML,128);
  gp1_chain_ptr1[7]=GP_UNIT(GP_TAG_LK,128,128);
  gp1_chain_ptr1[10]=GP_STRD(64,GP_FRM_NML,64);
  gp1_chain_ptr1[11]=GP_UNIT(GP_TAG_LK,64,64);

  gp0_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
  set_gp0_dha(TCSM1_PADDR(DDMA_GP0_DES_CHAIN));
#ifdef JZC_IDCT_OPT
  uint32_t *idct_chain_tab =IDCT_DES_CHAIN;
  uint32_t *idct_chain_tab1=IDCT_DES_CHAIN1;
  idct_chain_tab[0]  = 0x80000000;
  idct_chain_tab[1]  = 0x1403004c;//IDCT_UNLINK_HEAD;
  idct_chain_tab1[0] = 0x80000000;
  idct_chain_tab1[1] = 0x1403004c;//IDCT_UNLINK_HEAD;
#endif

  while(*mbnum_wp<=*mbnum_rp+2);//wait until the first two mb is ready

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dMB);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,TASK_BUF_LEN,TASK_BUF_LEN);
  set_gp0_dcs();
  poll_gp0_end();

  (*mbnum_rp)++;
  *addr_rp += TASK_BUF_LEN;

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dMB_N);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,TASK_BUF_LEN,TASK_BUF_LEN);
  set_gp0_dcs();
  poll_gp0_end();

  count = 0;

  y_mb_line = (dSlice->mb_width +4) * 256;
  uv_mb_line = (dSlice->mb_width*2 + 8) * 64;
#ifdef JZC_ROTA90_OPT
  rota_y_mb_line = dSlice->mb_height * 256;
  rota_uv_mb_line = dSlice->mb_height * 128;
#endif
  Lpicture = &dSlice->line_current_picture;
  while((dMB->new_slice>=0) && (dMB->er_slice >=0)) {
    if(dMB->new_slice==1){
      XCHG2(dSlice,dSlice_ex,xchg_tmp);
    }
    mc_flag = 0;
#ifdef JZC_DBLK_OPT
    intra      = (int)(IS_INTRA_MBTYPE(dMB->mbtype)); // intra means intra4x4 and intra16x16
    inter16x16 = (dMB->mbtype == RV34_MB_P_MIX16x16);
#endif
    chain = 0;
    mb_x_d3=mb_x_d2;
    mb_x_d2=mb_x_d1;
    mb_x_d1=mb_x;
    mb_x= dMB->mb_x;
    mb_y_d3=mb_y_d2;
    mb_y_d2=mb_y_d1;
    mb_y_d1=mb_y;
    mb_y= dMB->mb_y;

    if(last_mc_flag){     
      while(*(volatile int *)motion_dsa != (0x80000000 | 0xFFFF) );
      last_mc_flag = 0;
    }
    if(dSlice->si_type){
      if(dMB->mbtype > RV34_MB_TYPE_INTRA16x16)
	{
	  rv40_decode_mv_aux(dSlice, dMB, recon_ptr0, motion_dha, motion_dsa);
	}
    }

    if(mc_flag){     
      SET_REG1_DDC(TCSM1_PADDR((int)motion_dha) | 0x1);
    }

    uint32_t cbp=dMB->cbp;
    uint32_t cbpmask=1;
    S16LDD(xr12,&dMB->yqtable,0,3);
    S32I2M(xr13,0x8);

    for(k = 0; k < 16; k++, cbpmask <<= 1){
      if(cbp & cbpmask){
	rv40_dequant4x4(dMB->block[0]+(k<<4),idct+(k<<4),dMB->dq_len[k]);
	if(dMB->is16)
	  *(idct+(k<<4)) = dMB->block16[k];
      }
      else
	{
	  if(dMB->is16) //FIXME: optimize
	    {
	      MXU_SETZERO(idct + (k<<4),2);
	      *(idct+(k<<4)) = dMB->block16[k];
	    }
	}
    }
    S16LDD(xr12,dMB->cqtable,2,0);
    S16LDD(xr12,dMB->cqtable,0,1);
    for(; k < 24; k++, cbpmask <<= 1){
      if(cbp & cbpmask)
	rv40_dequant4x4(dMB->block[0]+(k<<4),idct+(k<<4),dMB->dq_len[k]);
    }
    if(dMB->is16) cbp|=0xffff;
#ifdef JZC_IDCT_OPT
    idct_chain_tab[2] = cbp &((1 << 24) - 1);
    idct_chain_tab[3] = TCSM1_PADDR(idct);
    idct_chain_tab[4] = TCSM1_PADDR(idct);
    set_idct_ddma_dha(TCSM1_PADDR(idct_chain_tab));
    idct_polling_end_flag();
    clean_idct_end_flag();
    run_idct_ddma();
#else
    for(k = 0; k < 24; k++, cbp >>= 1)
      if(cbp &1)
	rv40_inv_transform(idct+(k<<4));
#endif

    if(count){
      if (dMB_L->mb_x){
	intra_top_y += 16;
	intra_top_u += 8;
	intra_top_v += 8;
      }else{
	intra_top_y=intraup_y;
	intra_top_u=intraup_u;
	intra_top_v=intraup_v;
      }

      if(IS_INTRA(dMB_L->mb_type)){
	rv40_output_macroblock_aux(dMB_L, recon_ptr2,intra_top_y, intra_top_u, intra_top_v, recon_ptr1,idct1, top_left_tmp);
      }else{
	rv40_apply_differences(dMB_L, idct1, recon_ptr2);
      }
      top_left_tmp[0] = intra_top_y[15];
      top_left_tmp[1] = intra_top_u[7];
      top_left_tmp[2] = intra_top_v[7];
      ((uint32_t*)(intra_top_y))[0]=((uint32_t *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[0];
      ((uint32_t*)(intra_top_y))[1]=((uint32_t *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[1];
      ((uint32_t*)(intra_top_y))[2]=((uint32_t *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[2];
      ((uint32_t*)(intra_top_y))[3]=((uint32_t *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[3];
 
      ((uint32_t*)(intra_top_u))[0]=((uint32_t *)(recon_ptr2+PREVIOUS_OFFSET_U+7*PREVIOUS_CHROMA_STRIDE))[0];
      ((uint32_t*)(intra_top_u))[1]=((uint32_t *)(recon_ptr2+PREVIOUS_OFFSET_U+7*PREVIOUS_CHROMA_STRIDE))[1];
      ((uint32_t*)(intra_top_v))[0]=((uint32_t *)(recon_ptr2+PREVIOUS_OFFSET_V+7*PREVIOUS_CHROMA_STRIDE))[0];
      ((uint32_t*)(intra_top_v))[1]=((uint32_t *)(recon_ptr2+PREVIOUS_OFFSET_V+7*PREVIOUS_CHROMA_STRIDE))[1];
    }

    if (count > 1){ /*LEFT copy*/
      while (*(volatile int*)DBLK_END_FLAG != DBLK_SYN_VALUE);
    }

    if (count>1){
#ifdef JZC_ROTA90_OPT
      tag1 = (mb_y_d2+1>=dSlice->mb_height) ? 1 : 0;
      tag2 = tag1 ? 0 : 4;

      dst2 = dblk_mb_ydst3 + 16*(16-tag2) + 12;
      dst1 = rota_mb_ybuf3 + 192 - 4*tag1 ;
      for(i=0; i<4; i++){
        src1 = XCH2_T0->dblk_out_addr + 20*(16-tag2) + i*4;

        for(j=0; j<3+tag1; j++){
	  S32LDI(xr1,src1,-TCSM1_DBLK_MBOUT_STRD_Y);  //xr1:x15,x14,x13,x12
          S32LDI(xr2,src1,-TCSM1_DBLK_MBOUT_STRD_Y);  //xr2:x11,x10,x9,x8
	  S32LDI(xr3,src1,-TCSM1_DBLK_MBOUT_STRD_Y);  //xr3:x7,x6,x5,x4
          S32LDI(xr4,src1,-TCSM1_DBLK_MBOUT_STRD_Y);  //xr4:x3,x2,x1,x0

	  S32SFL(xr5,xr2,xr1,xr6,ptn2); //xr5:x11,x15,x9,x13 xr6:x10,x14,x8,x12
	  S32SFL(xr7,xr4,xr3,xr8,ptn2); //xr7:x3,x7,x1,x5 xr8:x2,x6,x0,x4	
	  S32SFL(xr9,xr8,xr6,xr10,ptn3); //xr9:x2,x6,x10,x14 xr10:x0,x4,x8,x12	    
	  S32SFL(xr11,xr7,xr5,xr12,ptn3); //xr11:x3,x7,x11,x15 xr12:x1,x5,x9,x13

          S32SDI(xr1,dst2,-0x10);
          S32SDI(xr2,dst2,-0x10);
          S32SDI(xr3,dst2,-0x10);
	  S32SDI(xr4,dst2,-0x10);

          S32SDI(xr10,dst1,0x4);
	  S32STD(xr12,dst1,0x10);
	  S32STD(xr9, dst1,0x20);
	  S32STD(xr11,dst1,0x30); 
        }
	dst2 = dblk_mb_ydst1 + 16*(16-tag2) + i*4;
	dst1 = rota_mb_ybuf1 + i*64 - 4*tag1;
      }

      for(i=0; i<2; i++){
        src1 = XCH2_T0->dblk_out_addr_c + 96 + i*4;
	src2 = src1 + TCSM1_DBLK_MBOUT_UV_OFFSET;
        if (i){
	  dst3 = dblk_mb_udst1 + 128;
          dst4 = dst3 + 8;
          dst1 = rota_mb_cbuf1 - 4;
          dst2 = dst1 + 8;
        }else{
          dst3 = dblk_mb_udst3 + 132;
          dst4 = dst3 + 8;
          dst1 = rota_mb_cbuf3 + 60;
          dst2 = dst1 + 8;
        }

        for(j=0; j<2; j++){
	  S32LDI(xr1,src1,-TCSM1_DBLK_MBOUT_STRD_C);
          S32LDI(xr2,src1,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr3,src1,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr4,src1,-TCSM1_DBLK_MBOUT_STRD_C);

	  S32SFL(xr5,xr2,xr1,xr6,ptn2); //xr5:x11,x15,x9,x13 xr6:x10,x14,x8,x12
	  S32SFL(xr7,xr4,xr3,xr8,ptn2); //xr7:x3,x7,x1,x5 xr8:x2,x6,x0,x4	
	  S32SFL(xr9,xr8,xr6,xr10,ptn3); //xr9:x2,x6,x10,x14 xr10:x0,x4,x8,x12	    
	  S32SFL(xr11,xr7,xr5,xr12,ptn3); //xr11:x3,x7,x11,x15 xr12:x1,x5,x9,x13
          
          S32SDI(xr1,dst3,-0x10);
	  S32SDI(xr2,dst3,-0x10);
	  S32SDI(xr3,dst3,-0x10);
	  S32SDI(xr4,dst3,-0x10);

	  S32SDI(xr10,dst1,0x4);
	  S32STD(xr12,dst1,0x10);
	  S32STD(xr9, dst1,0x20);
	  S32STD(xr11,dst1,0x30);          

	  S32LDI(xr1,src2,-TCSM1_DBLK_MBOUT_STRD_C);
          S32LDI(xr2,src2,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr3,src2,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr4,src2,-TCSM1_DBLK_MBOUT_STRD_C);

	  S32SFL(xr5,xr2,xr1,xr6,ptn2); //xr5:x11,x15,x9,x13 xr6:x10,x14,x8,x12
	  S32SFL(xr7,xr4,xr3,xr8,ptn2); //xr7:x3,x7,x1,x5 xr8:x2,x6,x0,x4	
	  S32SFL(xr9,xr8,xr6,xr10,ptn3); //xr9:x2,x6,x10,x14 xr10:x0,x4,x8,x12	    
	  S32SFL(xr11,xr7,xr5,xr12,ptn3); //xr11:x3,x7,x11,x15 xr12:x1,x5,x9,x13
          
          S32SDI(xr1,dst4,-0x10);
	  S32SDI(xr2,dst4,-0x10);
	  S32SDI(xr3,dst4,-0x10);
	  S32SDI(xr4,dst4,-0x10);

	  S32SDI(xr10,dst2,0x4);
	  S32STD(xr12,dst2,0x10);
	  S32STD(xr9, dst2,0x20);
	  S32STD(xr11,dst2,0x30);          
        }
      }
#else
      src1 = XCH2_T0->dblk_out_addr + 4 - TCSM1_DBLK_MBOUT_STRD_Y;
      src2 = XCH2_T0->dblk_out_addr - TCSM1_DBLK_MBOUT_STRD_Y;
      dst1 = dblk_mb_ydst - 16;
      dst2 = dblk_mb_ydst1 - 4;
      for(kk=0; kk<16;kk++){
	S32LDI(xr1,src1,TCSM1_DBLK_MBOUT_STRD_Y);
	S32LDD(xr2,src1,0x4);
	S32LDD(xr3,src1,0x8);
	S32LDI(xr4,src2,TCSM1_DBLK_MBOUT_STRD_Y);
        S32SDI(xr1,dst1,0x10);
        S32STD(xr2,dst1,0x4);
        S32STD(xr3,dst1,0x8);
        S32SDI(xr4,dst2,0x10);
      }

      src1 = XCH2_T0->dblk_out_addr_c + 4 - TCSM1_DBLK_MBOUT_STRD_C;
      src2 = XCH2_T0->dblk_out_addr_c - TCSM1_DBLK_MBOUT_STRD_C;
      dst1 = dblk_mb_udst - 16;
      dst2 = dblk_mb_udst1 - 12;

      src3 = XCH2_T0->dblk_out_addr_c + 4 + TCSM1_DBLK_MBOUT_UV_OFFSET - TCSM1_DBLK_MBOUT_STRD_C;
      src4 = XCH2_T0->dblk_out_addr_c + TCSM1_DBLK_MBOUT_UV_OFFSET - TCSM1_DBLK_MBOUT_STRD_C;
      dst3 = dblk_mb_vdst - 16;
      dst4 = dblk_mb_vdst1 - 12;
      for(kk=0; kk<8;kk++){
	S32LDI(xr1,src1,TCSM1_DBLK_MBOUT_STRD_C);
	S32LDI(xr2,src3,TCSM1_DBLK_MBOUT_STRD_C);
        S32LDI(xr3,src2,TCSM1_DBLK_MBOUT_STRD_C);
	S32LDI(xr4,src4,TCSM1_DBLK_MBOUT_STRD_C);
        S32SDI(xr1,dst1,0x10);
        S32SDI(xr3,dst2,0x10);
        S32SDI(xr2,dst3,0x10);
        S32SDI(xr4,dst4,0x10);
      }
#endif

#ifdef JZC_ROTA90_OPT
      if (mb_y_d2){
	for(i=0; i<4; i++){
          src1 = dblk_upout_addr1 + 64 + i*4;      
	  dst1 = rota_upmb_ybuf1 + i*16 - 4; 
	  S32LDI (xr1,src1,-0x10);       //xr1: x15,x14,x13,x12
	  S32LDI (xr2,src1,-0x10);       //xr2: x11,x10,x9,x8
	  S32LDI (xr3,src1,-0x10);       //xr3: x7,x6,x5,x4
	  S32LDI (xr4,src1,-0x10);       //xr4: x3,x2,x1,x0
	  
	  S32SFL(xr5,xr2,xr1,xr6,ptn2); //xr5:x11,x15,x9,x13 xr6:x10,x14,x8,x12
	  S32SFL(xr7,xr4,xr3,xr8,ptn2); //xr7:x3,x7,x1,x5 xr8:x2,x6,x0,x4	
	  S32SFL(xr3,xr8,xr6,xr4,ptn3); //xr3:x2,x6,x10,x14 xr4:x0,x4,x8,x12	    
	  S32SFL(xr1,xr7,xr5,xr2,ptn3); //xr1:x3,x7,x11,x15 xr2:x1,x5,x9,x13
	  
	  S32SDI(xr4,dst1,0x4);
	  S32STD(xr2,dst1,0x4);
	  S32STD(xr3,dst1,0x8);
	  S32STD(xr1,dst1,0xC);
	}
      }
#endif
      
      if (count > 2){
	tag3 = (mb_y_d3 + 1 == dSlice->mb_height) ? 1 : 0;
#ifdef JZC_ROTA90_OPT
	rota_mx = dSlice->mb_height - 1 - mb_y_d3;
	rota_my = mb_x_d3;
	rota_ydst = Lpicture->rota_y_ptr + rota_my*rota_y_mb_line + rota_mx*256;
	rota_cdst = Lpicture->rota_uv_ptr + rota_my*rota_uv_mb_line + rota_mx * 128;
#endif
	dst3 = Lpicture->y_ptr + mb_y_d3 * y_mb_line + mb_x_d3 * 256;
	dst4 = Lpicture->uv_ptr + mb_y_d3 * uv_mb_line + mb_x_d3 * 128;

	if (mb_y_d2){
	  up_ydst = Lpicture->y_ptr + (mb_y_d2 - 1) * y_mb_line + mb_x_d2 * 256;
#ifdef JZC_ROTA90_OPT
	  rota_up_mx = dSlice->mb_height - mb_y_d2;
	  rota_up_my = mb_x_d2;
          rota_up_ydst = Lpicture->rota_y_ptr + rota_up_my * rota_y_mb_line + rota_up_mx*256;
#endif
        }else{
          up_ydst = Lpicture->y_ptr - 2 * y_mb_line - 512;
#ifdef JZC_ROTA90_OPT
          rota_up_ydst = up_ydst;
#endif
        }

	gp1_chain_ptr[0]=TCSM1_PADDR(dblk_mb_ydst3);
	gp1_chain_ptr[1]=(dst3);
        gp1_chain_ptr[2]=GP_STRD(192+tag3*64,GP_FRM_NML,tag3*64);
	gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,192+tag3*64,192+tag3*64);  

	gp1_chain_ptr[4]=TCSM1_PADDR(dblk_mb_udst3);
	gp1_chain_ptr[5]=(dst4);	    
	gp1_chain_ptr[8]=TCSM1_PADDR(dblk_upout_addr1);
	gp1_chain_ptr[9]=(up_ydst + 16*12);


        if (mb_y_d2 && ((!mb_x_d2)|| (mb_x_d2+1 == dSlice->mb_width))){ //up 16x4
	  uint8_t *edge_up_ydst = Lpicture->y_ptr + (mb_y_d2 - 1) * y_mb_line + mb_x_d2 * 256;
          src1 = dblk_upout_addr1 - 16 + (!!mb_x_d2)*15;
          dst1 = up_edge_ydst1-16;
	  for(i=0; i<2; i++){
	    S8LDI(xr1,src1,0x10,ptn7);  //xr1:x0,x0,x0,x0
	    S8LDI(xr2,src1,0x10,ptn7);  //xr2:x16,x16,x16,x16
	    S32SDI(xr1,dst1,0x10);
	    S32STD(xr1,dst1,0x4);
	    S32STD(xr1,dst1,0x8);
	    S32STD(xr1,dst1,0xC);
	    S32SDI(xr2,dst1,0x10);
	    S32STD(xr2,dst1,0x4);
	    S32STD(xr2,dst1,0x8);
	    S32STD(xr2,dst1,0xC);
	  }

          gp1_chain_ptr[12]=TCSM1_PADDR(up_edge_ydst1);
	  gp1_chain_ptr[13]=(edge_up_ydst + 192 + (!mb_x_d2?-512:512));
	  gp1_chain_ptr[14]=GP_STRD(64,GP_FRM_NML,64);
	  gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,64,64); 

          gp1_chain_ptr[16]=TCSM1_PADDR(up_edge_ydst1);
	  gp1_chain_ptr[17]=(edge_up_ydst + 192 +(!mb_x_d2?-256:256));
	  gp1_chain_ptr[18]=GP_STRD(64,GP_FRM_NML,64);
	  gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,64,64); 
	  chain += 2;
        }
	
	switch(edge_type)
	  {
	  case 4://middle	  	    	   	   	    
#ifdef JZC_ROTA90_OPT
	    gp1_chain_ptr[12+4*chain + 0]=TCSM1_PADDR(rota_mb_ybuf3);
	    gp1_chain_ptr[12+4*chain + 1]=(rota_ydst);
	    gp1_chain_ptr[12+4*chain + 2]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[12+4*chain + 3]=GP_UNIT(GP_TAG_LK,256,256);
	    
	    gp1_chain_ptr[12+4*chain + 4]=TCSM1_PADDR(rota_mb_cbuf3);
	    gp1_chain_ptr[12+4*chain + 5]=(rota_cdst);
	    gp1_chain_ptr[12+4*chain + 6]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[12+4*chain + 7]=GP_UNIT(GP_TAG_LK,128,128);
         
            gp1_chain_ptr[12+4*chain + 8]=TCSM1_PADDR(rota_upmb_ybuf1);
	    gp1_chain_ptr[12+4*chain + 9]=(rota_up_ydst);
	    gp1_chain_ptr[12+4*chain + 10]=GP_STRD(4,GP_FRM_NML,16);
	    gp1_chain_ptr[12+4*chain + 11]=GP_UNIT(GP_TAG_LK,4,4*16);

	    gp1_chain_ptr[12+4*chain + 12] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 13] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 14] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[12+4*chain + 15] = GP_UNIT(GP_TAG_UL,4,4);
#else
	    gp1_chain_ptr[12+4*chain + 0] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 1] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 2] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[12+4*chain + 3] = GP_UNIT(GP_TAG_UL,4,4);
#endif
	    edge_type=(count-1)%dSlice->mb_width==0?3:edge_type;
	    break;
	  case 1://left edge
	    edge_type=(count-1)>dSlice->mb_width*(dSlice->mb_height-1)?5:(count-1)>dSlice->mb_width?4:2;
	  case 3://right edge
            lastline = (mb_x_d3 + 1 == dSlice->mb_width);
	    src_ptr=dblk_mb_ydst3-(edge_type!=3?16:1);
	    dst_ptr=edge_bufy[1]-16;
	    src_cptr=dblk_mb_udst3-(edge_type!=3?8:1);
	    dst_cptr=edge_bufc[1]-8;
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
	    gp1_chain_ptr[12+4*chain + 0]=TCSM1_PADDR(edge_bufy[1]);	    
	    gp1_chain_ptr[12+4*chain + 1]=(dst3+(edge_type!=3?-512:512));
	    gp1_chain_ptr[12+4*chain + 2]=GP_STRD(192+64*lastline,GP_FRM_NML,192+64*lastline);
	    gp1_chain_ptr[12+4*chain + 3]=GP_UNIT(GP_TAG_LK,192+64*lastline,192+64*lastline);

	    gp1_chain_ptr[12+4*chain + 4]=TCSM1_PADDR(edge_bufy[1]);	    
	    gp1_chain_ptr[12+4*chain + 5]=(dst3+(edge_type!=3?-256:256));
	    gp1_chain_ptr[12+4*chain + 6]=GP_STRD(192+64*lastline,GP_FRM_NML,192+64*lastline);
	    gp1_chain_ptr[12+4*chain + 7]=GP_UNIT(GP_TAG_LK,192+64*lastline,192+64*lastline);

	    gp1_chain_ptr[12+4*chain + 8]=TCSM1_PADDR(edge_bufc[1]);	    
	    gp1_chain_ptr[12+4*chain + 9]=(dst4+(edge_type!=3?-256:256));
	    gp1_chain_ptr[12+4*chain + 10]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[12+4*chain + 11]=GP_UNIT(GP_TAG_LK,128,128);

            gp1_chain_ptr[12+4*chain + 12]=TCSM1_PADDR(edge_bufc[1]);
	    gp1_chain_ptr[12+4*chain + 13]=(dst4+(edge_type!=3?-128:128));
            gp1_chain_ptr[12+4*chain + 14]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[12+4*chain + 15]=GP_UNIT(GP_TAG_LK,128,128); 
#ifdef JZC_ROTA90_OPT
	    gp1_chain_ptr[12+4*chain + 16]=TCSM1_PADDR(rota_mb_ybuf3);
	    gp1_chain_ptr[12+4*chain + 17]=(rota_ydst);
	    gp1_chain_ptr[12+4*chain + 18]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[12+4*chain + 19]=GP_UNIT(GP_TAG_LK,256,256);
	    
	    gp1_chain_ptr[12+4*chain + 20]=TCSM1_PADDR(rota_mb_cbuf3);
	    gp1_chain_ptr[12+4*chain + 21]=(rota_cdst);
	    gp1_chain_ptr[12+4*chain + 22]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[12+4*chain + 23]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[12+4*chain + 24]=TCSM1_PADDR(rota_upmb_ybuf1);
	    gp1_chain_ptr[12+4*chain + 25]=(rota_up_ydst);
	    gp1_chain_ptr[12+4*chain + 26]=GP_STRD(4,GP_FRM_NML,16);
	    gp1_chain_ptr[12+4*chain + 27]=GP_UNIT(GP_TAG_LK,4,4*16);
             
	    gp1_chain_ptr[12+4*chain + 28] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 29] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 30] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[12+4*chain + 31] = GP_UNIT(GP_TAG_UL,4,4);
#else
	    gp1_chain_ptr[12+4*chain + 16] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 17] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 18] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[12+4*chain + 19] = GP_UNIT(GP_TAG_UL,4,4);
#endif
	    edge_type=(edge_type!=3?edge_type:1);
	    break;
	  case 2://top edge
	  case 5://bottom edge
	    src_ptr=dblk_mb_ydst3+(edge_type==2?0:240);
	    dst_ptr=edge_bufy[1]-16;
	    src_cptr=dblk_mb_udst3+(edge_type==2?0:112);
	    dst_cptr=edge_bufc[1]-16;
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
	    gp1_chain_ptr[12+4*chain + 0]=TCSM1_PADDR(edge_bufy[1]);
	    gp1_chain_ptr[12+4*chain + 1]=(dst3+((edge_type==2?-dSlice->linesize:dSlice->linesize)<<1));
	    gp1_chain_ptr[12+4*chain + 2]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[12+4*chain + 3]=GP_UNIT(GP_TAG_LK,256,256);
	    gp1_chain_ptr[12+4*chain + 4]=TCSM1_PADDR(edge_bufy[1]);
	    gp1_chain_ptr[12+4*chain + 5]=(dst3+(edge_type==2?-dSlice->linesize:dSlice->linesize));
	    gp1_chain_ptr[12+4*chain + 6]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[12+4*chain + 7]=GP_UNIT(GP_TAG_LK,256,256);
	    gp1_chain_ptr[12+4*chain + 8]=TCSM1_PADDR(edge_bufc[1]);
	    gp1_chain_ptr[12+4*chain + 9]=(dst4+(edge_type==2?-dSlice->linesize:dSlice->linesize));
	    gp1_chain_ptr[12+4*chain + 10]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[12+4*chain + 11]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[12+4*chain + 12]=TCSM1_PADDR(edge_bufc[1]);
	    gp1_chain_ptr[12+4*chain + 13]=(dst4+(edge_type==2?-dSlice->uvlinesize:dSlice->uvlinesize));
            gp1_chain_ptr[12+4*chain + 14]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[12+4*chain + 15]=GP_UNIT(GP_TAG_LK,128,128);
#ifdef JZC_ROTA90_OPT
	    gp1_chain_ptr[12+4*chain + 16]=TCSM1_PADDR(rota_mb_ybuf3);
	    gp1_chain_ptr[12+4*chain + 17]=(rota_ydst);
	    gp1_chain_ptr[12+4*chain + 18]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[12+4*chain + 19]=GP_UNIT(GP_TAG_LK,256,256);

	    gp1_chain_ptr[12+4*chain + 20]=TCSM1_PADDR(rota_mb_cbuf3);
	    gp1_chain_ptr[12+4*chain + 21]=(rota_cdst);
	    gp1_chain_ptr[12+4*chain + 22]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[12+4*chain + 23]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[12+4*chain + 24]=TCSM1_PADDR(rota_upmb_ybuf1);
	    gp1_chain_ptr[12+4*chain + 25]=(rota_up_ydst);
	    gp1_chain_ptr[12+4*chain + 26]=GP_STRD(4,GP_FRM_NML,16);
	    gp1_chain_ptr[12+4*chain + 27]=GP_UNIT(GP_TAG_LK,4,4*16);

	    gp1_chain_ptr[12+4*chain + 28] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 29] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 30] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[12+4*chain + 31] = GP_UNIT(GP_TAG_UL,4,4);
#else
	    gp1_chain_ptr[12+4*chain + 16] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 17] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[12+4*chain + 18] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[12+4*chain + 19] = GP_UNIT(GP_TAG_UL,4,4);
#endif

	    edge_type=(count - 1)%dSlice->mb_width==0?3:edge_type;
	    break;
	  }

	set_gp1_dha(TCSM1_PADDR(gp1_chain_ptr));
	while(*tcsm1_gp1_poll_end);
	*tcsm1_gp1_poll_end = 1;
	set_gp1_dcs();
      }
    }

    if (count){ /*LEFT copy*/
      src1 = (uint8_t *)(XCH2_T0->dblk_out_addr + 16 - TCSM1_DBLK_MBOUT_STRD_Y);
      dst1 = (uint8_t *)(recon_ptr2 - 4 - PREVIOUS_LUMA_STRIDE);
      src2 = (uint8_t *)(XCH2_T0->dblk_out_addr_c + 8 - TCSM1_DBLK_MBOUT_STRD_C);
      dst2 = (uint8_t *)(recon_ptr2 + PREVIOUS_OFFSET_U - 4 - PREVIOUS_CHROMA_STRIDE);
      for(i=0;i<4;i++){
	S32LDI(xr1, src1, TCSM1_DBLK_MBOUT_STRD_Y);
	S32LDI(xr2, src2, TCSM1_DBLK_MBOUT_STRD_C);
	S32LDI(xr3, src1, TCSM1_DBLK_MBOUT_STRD_Y);
	S32LDI(xr4, src2, TCSM1_DBLK_MBOUT_STRD_C);
	S32LDI(xr5, src1, TCSM1_DBLK_MBOUT_STRD_Y);
	S32LDI(xr6, src2, TCSM1_DBLK_MBOUT_STRD_C);
	S32LDI(xr7, src1, TCSM1_DBLK_MBOUT_STRD_Y);
	S32LDI(xr8, src2, TCSM1_DBLK_MBOUT_STRD_C);

	S32SDI(xr1, dst1, PREVIOUS_LUMA_STRIDE);
	S32SDI(xr2, dst2, PREVIOUS_CHROMA_STRIDE);
	S32SDI(xr3, dst1, PREVIOUS_LUMA_STRIDE);
	S32SDI(xr4, dst2, PREVIOUS_CHROMA_STRIDE);
	S32SDI(xr5, dst1, PREVIOUS_LUMA_STRIDE);
	S32SDI(xr6, dst2, PREVIOUS_CHROMA_STRIDE);
	S32SDI(xr7, dst1, PREVIOUS_LUMA_STRIDE);
	S32SDI(xr8, dst2, PREVIOUS_CHROMA_STRIDE);
      }
      SET_DBLK_DDMA_DHA(TCSM1_PADDR(XCH2_T1->dblk_des_ptr));
      *(volatile int*)DBLK_END_FLAG=0;
      SET_DBLK_DDMA_DCS(0x1);
    }

    //dblk config
    {
      unsigned char left_mbs = (dMB->mb_x == 0) || (dMB->qscale == 100); //qp==100 for row_parity pseudo MB
      unsigned char top_mbs = (dMB->mb_y == 0);

      unsigned int * node = (unsigned int *)XCH2_T0->dblk_des_ptr;
      unsigned  dblk_bp = dMB->cbp;
      i_movn(dblk_bp, (dMB->cbp | 0xffff), inter16x16);
      i_movn(dblk_bp, (dMB->cbp | 0xffffff), intra);
      node[2] = TCSM1_PADDR((unsigned int)dblk_upout_addr1 + 136);
      node[3] = TCSM1_PADDR((unsigned int)dblk_upout_addr1 + 128);
      node[4] = TCSM1_PADDR((unsigned int)dblk_upout_addr1);
      
      node[5] = TCSM1_PADDR((unsigned int)recon_ptr0 + PREVIOUS_OFFSET_V);
      node[6] = TCSM1_PADDR((unsigned int)recon_ptr0 + PREVIOUS_OFFSET_U);
      node[7] = TCSM1_PADDR((unsigned int)recon_ptr0);
      
      node[8] = dMB->mvd_left | (dMB->mvd_above<<16);
      node[9] = dMB->mvd;
      node[10] = dMB->dcbp_above | (dMB->mbtype_above<<24);

      node[11] = dMB->dcbp_left | (dMB->mbtype_left<<24);
      node[12] = dblk_bp | (dMB->mbtype << 24);
      node[13] = (2 | (left_mbs<<2)|(top_mbs<<3)|((dSlice->pict_type-1)<<4) | (dMB->qscale<<16)|(dSlice->refqp<<24));
    }

    while(*tcsm1_gp0_poll_end);
    *tcsm1_gp0_poll_end = 1;     
    //poll_gp0_end();
    (*mbnum_rp)++;
    *addr_rp += TASK_BUF_LEN;
    if((int)(*addr_rp)>(0x132B4000-TASK_BUF_LEN))
      *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);
    ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;

    while(*mbnum_wp<=*mbnum_rp+2);//wait until the next next mb is ready
    gp0_chain_ptr[0]=addr_rp[0];
    gp0_chain_ptr[1]=TCSM1_PADDR(dMB_L);
    gp0_chain_ptr[3]=GP_UNIT(GP_TAG_LK,TASK_BUF_LEN,TASK_BUF_LEN);

    gp0_chain_ptr[4] = TCSM0_PADDR(TCSM0_GP0_POLL_END);
    gp0_chain_ptr[5] = TCSM1_PADDR(TCSM1_GP0_POLL_END);
    gp0_chain_ptr[6] = GP_STRD(4,GP_FRM_NML,4);
    gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,4,4);
    set_gp0_dcs();

    count++;
#ifdef JZC_DBLK_OPT
    XCHG2(XCH2_T0,XCH2_T1,xchg_tmp);
#endif

    last_mc_flag = mc_flag;

    XCHG3(recon_ptr2,recon_ptr0,recon_ptr1,xchg_tmp);
    XCHG3(dMB_L,dMB,dMB_N,xchg_tmp);
    XCHG2(idct,idct1,xchg_tmp);
    XCHG2(idct_chain_tab,idct_chain_tab1,xchg_tmp);
    XCHG2(gp1_chain_ptr, gp1_chain_ptr1,xchg_tmp);
    XCHG2(edge_bufy[0],edge_bufy[1],xchg_tmp);
    XCHG2(edge_bufc[0],edge_bufc[1],xchg_tmp);
    XCHG2(up_edge_ydst1,up_edge_ydst2,xchg_tmp);
    XCHG2(dblk_upout_addr0,dblk_upout_addr1,xchg_tmp);

#ifdef JZC_ROTA90_OPT
    XCHG3(rota_mb_ybuf3,rota_mb_ybuf1,rota_mb_ybuf2,xchg_tmp);
    XCHG3(rota_mb_cbuf3,rota_mb_cbuf1,rota_mb_cbuf2,xchg_tmp);
    XCHG3(dblk_mb_ydst3,dblk_mb_ydst1,dblk_mb_ydst2,xchg_tmp);
    XCHG3(dblk_mb_udst3,dblk_mb_udst1,dblk_mb_udst2,xchg_tmp);
    XCHG2(rota_upmb_ybuf1,rota_upmb_ybuf2,xchg_tmp);
#else
    XCHG2(dblk_mb_ydst,dblk_mb_ydst1,xchg_tmp);
    XCHG2(dblk_mb_udst,dblk_mb_udst1,xchg_tmp);
    XCHG2(dblk_mb_vdst,dblk_mb_vdst1,xchg_tmp);
#endif

  }
  poll_gp0_end(); 
  poll_gp1_end();  
  
  *((volatile int *)TCSM0_PADDR(TCSM0_P1_TASK_DONE)) = 0x1;
  i_nop;
  i_nop;
  i_nop;
  i_nop;
  i_wait();

}
