#define VC1_P1_USE_PADDR

#include "jzsys.h"
#include "jzasm.h"
#include "jzmedia.h"
#include "vc1_p1_type.h"
#include "jz4760_2ddma_hw.h"
#include "vc1_dcore.h"
#include "vc1_tcsm0.h"
#include "vc1_tcsm1.h"
#include "../../libjzcommon/jz4760e_aux_pmon.h"

#define VC1_STOP_P1()						\
  ({								\
    ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=0;	\
    *((volatile int *)(TCSM0_PADDR(TCSM0_P1_TASK_DONE))) = 0x1;	\
    i_nop;							\
    i_nop;							\
    i_wait();							\
  })

#include "vc1_idct.c"
#include "vc1_p1_mc.c"

#define __p1_text __attribute__ ((__section__ (".p1_text")))
#define __p1_main __attribute__ ((__section__ (".p1_main")))
#define __p1_data __attribute__ ((__section__ (".p1_data")))

extern int _gp;

#ifdef JZC_PMON_P1
PMON_CREAT(mc_polling);
PMON_CREAT(mc_cfg);
PMON_CREAT(idct);
PMON_CREAT(gp0);
PMON_CREAT(gp1);
PMON_CREAT(gp2);
#endif

uint32_t current_picture_ptr[2];
VC1_Frame_GlbARGs *dFRM;
VC1_MB_DecARGs *dMB;
uint8_t *recon_yptr, *recon_uptr, *recon_vptr;
uint8_t *rota_buf1, *rota_buf2,*rota_buf3, *rota_buf4;
uint8_t luty[256],lutuv[256];
int mc_flag,last_mc_flag;
#define XCHG2(a,b,t)   t=a;a=b;b=t
#define XCHG3(a,b,c,t)   t=a;a=b;b=c;c=t
extern int _gp;

__p1_main int main() {

  //enable MXU 
  S32I2M(xr16, 0x3);
  int xchg_tmp;
  int i, j, k, count, off, dMBs, mblen;
  int linesize, uvlinesize, mb_size, block_size;
  uint8_t *dout_ptr0[2], *dout_ptr1[2], *dout_ptr2[3];
#ifdef JZC_ROTA90_OPT
  int rota_mx;
  uint32_t rota_picture_ptr[2];
  uint32_t rota_picture_ptr1[2];
#endif
  DCTELEM *idct, *blk;
  uint8_t *mc_uv;
  int *gp0_chain_ptr, *gp1_chain_ptr, *gp1_chain_ptr1,*pmon_ptr;
  VC1_MB_DecARGs *dMB_L, *dMB_N;
  uint8_t mv_mode2;
  uint16_t edge_type=1;
  uint16_t edge_type_config=1;
  volatile uint8_t *mc_dha[2], *mc_dsa[2];
  volatile int *tcsm0_gp0_poll_end ,*tcsm0_gp1_poll_end;
  volatile int *tcsm1_gp0_poll_end ,*tcsm1_gp1_poll_end;

  uint8_t *edge_bufy[2],*edge_bufc[2];
  mc_dha[0] = (volatile uint8_t *)TCSM1_MOTION_DHA0;
  mc_dha[1] = (volatile uint8_t *)TCSM1_MOTION_DHA1;
  mc_dsa[0] = TCSM1_MOTION_DSA0;
  mc_dsa[1] = TCSM1_MOTION_DSA1;

  tcsm1_gp0_poll_end  = (volatile int*)(TCSM1_GP0_POLL_END);
  *tcsm1_gp0_poll_end  = 0;

  tcsm0_gp0_poll_end  = (volatile int*)TCSM0_PADDR(TCSM0_GP0_POLL_END);
  *tcsm0_gp0_poll_end  = 0;

  tcsm1_gp1_poll_end  = (volatile int*)(TCSM1_GP1_POLL_END);
  *tcsm1_gp1_poll_end  = 0;

  tcsm0_gp1_poll_end  = (volatile int*)TCSM0_PADDR(TCSM0_GP1_POLL_END);
  *tcsm0_gp1_poll_end  = 0;
  
  count = 0;
  volatile int *mbnum_wp=TCSM1_MBNUM_WP;
  volatile int *mbnum_rp=TCSM1_MBNUM_RP;
  volatile int *addr_rp=TCSM1_ADDR_RP;

  *mbnum_rp=0;
  *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);

  dFRM=TCSM1_DFRM_BUF;
  linesize= dFRM->slinesize[0]; 
  uvlinesize= dFRM->slinesize[1];
  mb_size= 4 - dFRM->lowres;
  block_size = 8>>dFRM->lowres;
  last_mc_flag = 0;

  dMB = TASK_BUF1;//curr mb
  dMB_L = TASK_BUF0;
  dMB_N = TASK_BUF2;
  gp0_chain_ptr = DDMA_GP0_DES_CHAIN;
  gp0_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
  set_gp0_dha(TCSM1_PADDR(DDMA_GP0_DES_CHAIN));

  dout_ptr0[0] = DOUT_YBUF0;
  dout_ptr0[1] = DOUT_CBUF0;

  dout_ptr1[0] = DOUT_YBUF1;
  dout_ptr1[1] = DOUT_CBUF1;

  dout_ptr2[0] = DOUT_YBUF2;
  dout_ptr2[1] = DOUT_CBUF2;
  dout_ptr2[2] = DOUT_CBUF3;

  edge_bufy[0] = EDGE_YOUT_BUF;
  edge_bufy[1] = EDGE_YOUT_BUF1;

  edge_bufc[0] = EDGE_COUT_BUF;
  edge_bufc[1] = EDGE_COUT_BUF1;

  idct = TCSM1_IDCT_BUF;
#ifdef JZC_ROTA90_OPT
  rota_buf1 = ROTA_Y_BUF;
  rota_buf2 = ROTA_C_BUF;

  rota_buf3 = ROTA_Y_BUF1;
  rota_buf4 = ROTA_C_BUF1;
#endif

  gp1_chain_ptr = DDMA_GP1_DES_CHAIN;
  gp1_chain_ptr1 = DDMA_GP1_DES_CHAIN1;

  gp1_chain_ptr[2]=GP_STRD(256,GP_FRM_NML,256);
  gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,256,256);
  
  gp1_chain_ptr[6]=GP_STRD(128,GP_FRM_NML,128);       
  gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,128,128);       	
  
  gp1_chain_ptr[22]=GP_STRD(128,GP_FRM_NML,128);
  gp1_chain_ptr[23]=GP_UNIT(GP_TAG_LK,128,128);	    	
 
  gp1_chain_ptr1[2]=GP_STRD(256,GP_FRM_NML,256);
  gp1_chain_ptr1[3]=GP_UNIT(GP_TAG_LK,256,256);
  
  gp1_chain_ptr1[6]=GP_STRD(128,GP_FRM_NML,128);       
  gp1_chain_ptr1[7]=GP_UNIT(GP_TAG_LK,128,128);       	
  
  gp1_chain_ptr1[22]=GP_STRD(128,GP_FRM_NML,128);
  gp1_chain_ptr1[23]=GP_UNIT(GP_TAG_LK,128,128);	    	
    
#ifdef JZC_PMON_P1
  pmon_ptr=(int*)TCSM1_PMON_BUF;
  PMON_CLEAR(mc_polling);
  PMON_CLEAR(mc_cfg);
  PMON_CLEAR(gp1);
  PMON_CLEAR(gp0);
  PMON_CLEAR(gp2);
  PMON_CLEAR(idct);
#endif
  while(*mbnum_wp<=*mbnum_rp+2);//wait until the first two mb is ready

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dMB);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,64,(*(int*)TCSM1_FIRST_MBLEN));
  set_gp0_dcs();
  poll_gp0_end();

  (*mbnum_rp)++;
  *addr_rp+=*(int*)TCSM1_FIRST_MBLEN;
  ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dMB_N);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,dMB->next_dma_len);

  gp0_chain_ptr[4] = TCSM0_PADDR(TCSM0_GP0_POLL_END);
  gp0_chain_ptr[5] = TCSM1_PADDR(TCSM1_GP0_POLL_END);
  gp0_chain_ptr[6] = GP_STRD(4,GP_FRM_NML,4);
  gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,4,4);

  set_gp0_dcs();

  current_picture_ptr[0] = dFRM->current_picture_save.ptr[0]-1280;
  current_picture_ptr[1] = dFRM->current_picture_save.ptr[1]-640;
  int loop_i=(dFRM->flags & CODEC_FLAG_GRAY)?4:6;
  while (dMB->real_num > 0){

    if(count > 0){
      current_picture_ptr[0]+=((count-1)%dFRM->mb_width)==0?1280:256;
      current_picture_ptr[1]+=((count-1)%dFRM->mb_width)==0?640:128;
    }
    mblen = 0;
    mc_flag = 0;

    motion_dha = mc_dha[0];
    motion_dsa = mc_dsa[0];
    motion_douty = dout_ptr0[0];
    motion_doutc = dout_ptr0[1];
 
    volatile int *tdd = (int *)motion_dha;
    int tkn = 0;
    motion_dsa[0] = 0x0;
    tdd ++;
#ifdef JZC_PMON_P1
    PMON_ON(mc_cfg);
#endif
    
    if (dFRM->pict_type == P_TYPE){
      if(!dMB->vc1_fourmv) {
	if (((!dMB->vc1_skipped) && (!dMB->vc1_intra[0])) || (dMB->vc1_skipped)){
	  vc1_mc_1mv_hw(0,0,tdd, &tkn);
	  mc_flag = 1;
	}
      }else{
	for (i=0; i<4; i++) {
	  if (((!dMB->vc1_skipped) && (!dMB->vc1_intra[i])) || (dMB->vc1_skipped)) {
	    vc1_mc_4mv_luma_hw(i, tdd, &tkn);
	  }
	}
	vc1_mc_4mv_chroma_hw(tdd, &tkn);
	mc_flag = 1;
      }
    }
    else if (dFRM->pict_type == B_TYPE && (!(dFRM->bi_type))){
      if (!dMB->vc1_b_mc_skipped){
	if(dFRM->use_ic) {
	  mv_mode2 = dMB->mv_mode;
	  dMB->mv_mode = MV_PMODE_INTENSITY_COMP;
	}
	if(dMB->vc1_direct || (dMB->bfmv_type == BMV_TYPE_INTERPOLATED)) {
	  vc1_mc_1mv_hw_b(1, 0, tdd, &tkn);
          mc_flag = 1;
	} else {
	  if(dFRM->use_ic && (dMB->bfmv_type == BMV_TYPE_BACKWARD)) dMB->mv_mode = mv_mode2;
	  vc1_mc_1mv_hw(0, (dMB->bfmv_type == BMV_TYPE_BACKWARD),tdd, &tkn);
          mc_flag = 1;
	}
	if(dFRM->use_ic) dMB->mv_mode = mv_mode2;
      }
    }

    tdd[-1] = TDD_HEAD(1,/*vld*/
		       1,/*lk*/
		       0,/*sync*/
		       SubPel[VC1_QPEL]-1,/*ch1pel*/
		       SubPel[VC1_QPEL]-1,/*ch2pel*/ 
		       TDD_POS_SPEC,/*posmd*/
		       TDD_MV_SPEC,/*mvmd*/ 
		       1,/*ch2en*/
		       tkn,/*tkn*/
		       dMB->mb_y,/*mby*/
		       dMB->mb_x/*mbx*/);

    tdd[2*tkn] = TDD_HEAD(1,/*vld*/
			  0,/*lk*/
			  1,/*sync*/
			  0,/*ch1pel*/
			  0,/*ch2pel*/ 
			  TDD_POS_SPEC,/*posmd*/
			  TDD_MV_SPEC,/*mvmd*/ 
			  1,/*ch2en*/
			  0,/*tkn*/
			  0xFF,/*mby*/
			  0xFF/*mbx*/);
#ifdef JZC_PMON_P1
    PMON_OFF(mc_cfg);
#endif

    if (last_mc_flag)
      {
	volatile uint8_t *dsa = mc_dsa[1];
#ifdef JZC_PMON_P1
	PMON_ON(mc_polling);
#endif
	while(*(volatile int *)dsa != (0x80000000 | 0xFFFF) )	 { };
#ifdef JZC_PMON_P1
	PMON_OFF(mc_polling);
#endif  
	dsa[0] = 0;	
      }
   
    SET_REG1_DSTA(TCSM1_PADDR((int)motion_douty));
    SET_REG2_DSTA(TCSM1_PADDR((int)motion_doutc));
    SET_REG1_DSA(TCSM1_PADDR((int)motion_dsa));
    SET_REG2_DSA(TCSM1_PADDR((int)motion_dsa));  
    SET_REG1_BINFO(0,0,0,0,dFRM->mspel? (IS_ILUT0): (IS_ILUT1 | 1),0,0,0,0,0,0,0,0);
    if (mc_flag)
      SET_REG1_DDC(TCSM1_PADDR((int)motion_dha) | 0x1);           	

    if (last_mc_flag)
      {
	int i,j;
	uint8_t *dest_c,*dest_c1;
	dest_c  = dout_ptr2[2] - 16;
	dest_c1 = dout_ptr1[1] - 8;
	
	for(i=0;i<8;i++) {
	  S32LDI(xr1, dest_c1,0x8);
	  S32LDD(xr2, dest_c1,0x4);

	  S32LDD(xr3, dest_c1,64);
	  S32LDD(xr4, dest_c1,64+4);

	  S32SDI(xr1, dest_c, 0x10);
	  S32STD(xr2, dest_c, 0x4);

	  S32STD(xr3, dest_c, 0x8);
	  S32STD(xr4, dest_c, 0xc);
	}	
	last_mc_flag = 0;
      }
    
    if(count > 0)
      {
#ifdef JZC_PMON_P1
	PMON_ON(idct);
#endif

	if (dFRM->pict_type == B_TYPE && (!(dFRM->bi_type))){
	  if (!dMB_L->vc1_b_mc_skipped){
	    if (dMB_L->vc1_skipped) {
	      goto end;
	    } else if (!dMB_L->vc1_direct) {
	      if(!dMB_L->vc1_mb_has_coeffs[0] && !dMB_L->bintra)
		goto end;
	      if(!(dMB_L->bintra && !dMB_L->vc1_mb_has_coeffs[0]))
		if(dMB_L->bfmv_type == BMV_TYPE_INTERPOLATED)
		  if(!dMB_L->vc1_mb_has_coeffs[1])
		    goto end;
	    }
	  } 
	}   
 
	//begin idct and add residual
	if(dFRM->pict_type == I_TYPE || (dFRM->pict_type == B_TYPE && dFRM->bi_type)){
	  DCTELEM *residual = ((unsigned)dMB_L) + 20;
	  for(i = 0; i < loop_i; i++) {
	    DCTELEM *block = idct + i * 64;
	    off = (((i & 1)?8:0) + ((i&2)?128:0));
	    
	    if(dMB_L->pb_val[i])
	      {
		int vpq = dMB_L->mq;
		int16_t scale,tmp;
		int m;
		int val;
		scale = dFRM->pq * 2 + dFRM->halfpq;
		val = dMB_L->idct_row[i];
		vc1_dq_aux(dMB_L->mq,scale,residual,dMB_L->idct_row[i],mblen);
	      }

	    if((dFRM->profile == PROFILE_ADVANCED)||(dFRM->pq >= 9 && dFRM->overlap))
	      {
		vc1_inv_trans_8x8_aux_test(residual + mblen * 8,block,dMB_L->idct_row[i]);	
		add_pixels_aux(block, 128);
		put_pixels_clamped_aux(block,(i < 4) ?dout_ptr1[0]+off: dout_ptr2[2] + off,16);
	      }
	    else
	      {
		vc1_inv_trans_8x8_aux(residual + mblen * 8,block,(i < 4) ?dout_ptr1[0]+off: dout_ptr2[2] + off,dMB_L->idct_row[i]);	
	      }
	    
	    mblen += dMB_L->idct_row[i];
	  }
	}else{
	  if( !((dFRM->pict_type == P_TYPE) && (dMB_L->vc1_skipped))) {
	    for (i=0; i<6; i++){
	      off = (((i & 1)?8:0) + ((i&2)?128:0));
	      DCTELEM *block = idct + i * 64;
	      if (dMB_L->vc1_intra[i]){
		if((i>3) && (dFRM->flags & CODEC_FLAG_GRAY)) continue;

		if(dMB_L->pb_val[i])
		  {
		    int vpq = dMB_L->mq;
		    int16_t scale,tmp;
		    int m;
		    int val;
		    scale = dMB_L->mq * 2 + dFRM->halfpq;
		    val = dMB_L->idct_row[i];
		    vc1_dq_aux(dMB_L->mq,scale,dMB_L->mb,dMB_L->idct_row[i],mblen);
		  }
		
		vc1_inv_trans_8x8_aux_test(dMB_L->mb + mblen * 8, block, dMB_L->idct_row[i]);
		if(dFRM->rangeredfrm){
		  DCTELEM *src = block - 8;
		  for(j=0; j<8; j++){
		    S32LDI(xr1,src,0x10);
		    S32LDD(xr2,src,0x4);
		    S32LDD(xr3,src,0x8);
		    S32LDD(xr4,src,0xc);
		    Q16SLL(xr5,xr1,xr2,xr6,1);
		    Q16SLL(xr7,xr3,xr4,xr8,1);
		    S32STD(xr5,src,0x0);
		    S32STD(xr6,src,0x4);
		    S32STD(xr7,src,0x8);
		    S32STD(xr8,src,0xc);
		  }
		}
		add_pixels_aux(block, 128);		
		put_pixels_clamped_aux(block,(i < 4) ? dout_ptr1[0]+off: dout_ptr2[2] + off,16);		 
		mblen += dMB_L->idct_row[i];
	      }else if(dMB_L->pb_val[i]) {
		switch(dMB_L->ttblock[i]) {
		case TT_8X8: 
		  vc1_inv_trans_8x8_aux_test(dMB_L->mb + mblen * 8, block, dMB_L->idct_row[i]);
		  mblen += dMB_L->idct_row[i];
		  break;
		case TT_4X4:
		  for(j = 0; j < 4; j++) {
		    if(!(dMB_L->subblockpat[i] & (1 << (3 - j)))){
		      vc1_inv_trans_4x4_aux(dMB_L->mb + mblen * 8, j, block, dMB_L->idct_row_4x4[i*4+j]);
		    }else{
		      blk = block + OFFTAB[j] - 8;
		      for(k = 0; k < 4; k++){
			S32SDI(xr0,blk,0x10);
			S32STD(xr0,blk,0x4);
		      }
		    }
		  }
		  mblen += (FFMAX(dMB_L->idct_row_4x4[i*4+2], dMB_L->idct_row_4x4[i*4+3])+4);
		  break;
		case TT_8X4:
		  for(j = 0; j < 2; j++) {
		    if(!(dMB_L->subblockpat[i] & (1 << (1 - j)))){
		      vc1_inv_trans_8x4_aux(dMB_L->mb + mblen * 8, j, block, dMB_L->idct_row_8x4[i*2+j]);
		    }else{		  
		      blk = block + (j<<5) - 8;
		      for(k = 0; k < 4; k++){
			S32SDI(xr0,blk,0x10);
			S32STD(xr0,blk,0x4);
			S32STD(xr0,blk,0x8);
			S32STD(xr0,blk,0xc);
		      }
		  
		    }
		  }
		  mblen += (dMB_L->idct_row_8x4[i*2+1] + 4);
		  break;
		case TT_4X8:
		  for(j = 0; j < 2; j++) {
		    if(!(dMB_L->subblockpat[i] & (1 << (1 - j)))){
		      vc1_inv_trans_4x8_aux(dMB_L->mb + mblen * 8, j, block, dMB_L->idct_row_8x4[i*2+j]);
		    }else{
		      blk = block + (j<<2) - 8;
		      for(k = 0; k < 8; k++){
			S32SDI(xr0,blk,0x10);
			S32STD(xr0,blk,0x4);
		      }
		    }
		  }
		  mblen += FFMAX(dMB_L->idct_row_8x4[i*2+0], dMB_L->idct_row_8x4[i*2+1]);
		  break;
		}

		if((i<4) || !(dFRM->flags & CODEC_FLAG_GRAY))
		  add_pixels_clamped_aux(block,(i < 4) ? dout_ptr1[0]+off: dout_ptr2[2] + off,16);		 
	      }
	    }
	  }
	}
#ifdef JZC_PMON_P1
	PMON_OFF(idct);
#endif

      }
  end:
    if(count > 0)
      {
#ifdef JZC_PMON_P1
	PMON_ON(gp2);
#endif

#ifdef JZC_ROTA90_OPT
	int strd = 16;
	uint8_t *t1,*t2,*t3,*t4;
	for(i = 0;i < 4;i++){
	  t1 = dout_ptr1[0] + 16*i*4 - 4;      
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
	  t1 = dout_ptr2[2] + 16*i*4 - 4;      
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
#endif	
#ifdef JZC_PMON_P1
	PMON_OFF(gp2);
#endif
	gp1_chain_ptr[0]=TCSM1_PADDR(dout_ptr1[0]);
	gp1_chain_ptr[1]=(current_picture_ptr[0]);
	gp1_chain_ptr[4]=TCSM1_PADDR(dout_ptr2[2]);
	gp1_chain_ptr[5]=(current_picture_ptr[1]);	

	gp1_chain_ptr[20]=TCSM1_PADDR(edge_bufc[1]);
	
#ifdef JZC_ROTA90_OPT
	rota_mx = dFRM->mb_height - 1 - dMB_L->mb_y;
	rota_picture_ptr[0] = dFRM->rota_current_picture.ptr[0] + (dMB_L->mb_x*dFRM->mb_height*256 + rota_mx*256);
	rota_picture_ptr[1] = dFRM->rota_current_picture.ptr[1] + (dMB_L->mb_x*dFRM->mb_height*128 + rota_mx*128);
#endif

   	uint32_t src_ptr,dst_ptr,src_cptr,dst_cptr;
	switch(edge_type)
	  {
	  case 4://middle
	    	   
#ifdef JZC_ROTA90_OPT
	    gp1_chain_ptr[8]=TCSM1_PADDR(rota_buf1);
	    gp1_chain_ptr[9]=(rota_picture_ptr[0]);
	    gp1_chain_ptr[10]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,256,256);
	    
	    gp1_chain_ptr[12]=TCSM1_PADDR(rota_buf2);
	    gp1_chain_ptr[13]=(rota_picture_ptr[1]);
	    gp1_chain_ptr[14]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[16] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[17] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[18] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[19] = GP_UNIT(GP_TAG_UL,4,4);
#else
	    gp1_chain_ptr[8] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[9] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[10] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[11] = GP_UNIT(GP_TAG_UL,4,4);	   
#endif
	    edge_type=(count+1)%dFRM->mb_width==0?3:edge_type;
	    break;
	  case 1://left edge
	    edge_type=(count+1)>dFRM->mb_width*(dFRM->mb_height-1)?5:(count+1)>dFRM->mb_width?4:2;
	  case 3://right edge
	    src_ptr=dout_ptr1[0]-(edge_type!=3?16:1);
	    dst_ptr=edge_bufy[1]-16;
	    src_cptr=dout_ptr2[2]-(edge_type!=3?8:1);
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
	    gp1_chain_ptr[8]=TCSM1_PADDR(edge_bufy[1]);
	    gp1_chain_ptr[9]=(current_picture_ptr[0]+(edge_type!=3?-512:512));
	    gp1_chain_ptr[10]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,256,256);	    

	    gp1_chain_ptr[12]=TCSM1_PADDR(edge_bufy[1]);
	    gp1_chain_ptr[13]=(current_picture_ptr[0]+(edge_type!=3?-256:256));
	    gp1_chain_ptr[14]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,256,256);

	    gp1_chain_ptr[16]=TCSM1_PADDR(edge_bufc[1]);
	    gp1_chain_ptr[17]=(current_picture_ptr[1]+(edge_type!=3?-256:256));
	    gp1_chain_ptr[18]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[21]=(current_picture_ptr[1]+(edge_type!=3?-128:128)); 

#ifdef JZC_ROTA90_OPT
	    gp1_chain_ptr[24]=TCSM1_PADDR(rota_buf1);
	    gp1_chain_ptr[25]=(rota_picture_ptr[0]);
	    gp1_chain_ptr[26]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,256,256);
	    
	    gp1_chain_ptr[28]=TCSM1_PADDR(rota_buf2);
	    gp1_chain_ptr[29]=(rota_picture_ptr[1]);
	    gp1_chain_ptr[30]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[32] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[33] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[34] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[35] = GP_UNIT(GP_TAG_UL,4,4);
#else
	    gp1_chain_ptr[24] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[25] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[26] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[27] = GP_UNIT(GP_TAG_UL,4,4);
#endif

	    edge_type=(edge_type!=3?edge_type:1);
	    break;
	  case 2://top edge
	  case 5://bottom edge
	    src_ptr=dout_ptr1[0]+(edge_type==2?0:240);
	    dst_ptr=edge_bufy[1]-16;
	    src_cptr=dout_ptr2[2]+(edge_type==2?0:112);
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
	    gp1_chain_ptr[8]=TCSM1_PADDR(edge_bufy[1]);
	    gp1_chain_ptr[9]=(current_picture_ptr[0]+((edge_type==2?-dFRM->linesize:dFRM->linesize)<<1));
	    gp1_chain_ptr[10]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,256,256);

	    gp1_chain_ptr[12]=TCSM1_PADDR(edge_bufy[1]);
	    gp1_chain_ptr[13]=(current_picture_ptr[0]+(edge_type==2?-dFRM->linesize:dFRM->linesize));
	    gp1_chain_ptr[14]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,256,256);

	    gp1_chain_ptr[16]=TCSM1_PADDR(edge_bufc[1]);
	    gp1_chain_ptr[17]=(current_picture_ptr[1]+(edge_type==2?-dFRM->linesize:dFRM->linesize));
	    gp1_chain_ptr[18]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[21]=(current_picture_ptr[1]+(edge_type==2?-dFRM->uvlinesize:dFRM->uvlinesize));	    

#ifdef JZC_ROTA90_OPT
	    gp1_chain_ptr[24]=TCSM1_PADDR(rota_buf1);
	    gp1_chain_ptr[25]=(rota_picture_ptr[0]);
	    gp1_chain_ptr[26]=GP_STRD(256,GP_FRM_NML,256);
	    gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,256,256);
	    
	    gp1_chain_ptr[28]=TCSM1_PADDR(rota_buf2);
	    gp1_chain_ptr[29]=(rota_picture_ptr[1]);
	    gp1_chain_ptr[30]=GP_STRD(128,GP_FRM_NML,128);
	    gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,128,128);

	    gp1_chain_ptr[32] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[33] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[34] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[35] = GP_UNIT(GP_TAG_UL,4,4);
#else
	    gp1_chain_ptr[24] = TCSM0_PADDR((int)tcsm0_gp1_poll_end);
	    gp1_chain_ptr[25] = TCSM1_PADDR((int)tcsm1_gp1_poll_end);
	    gp1_chain_ptr[26] = GP_STRD(4,GP_FRM_NML,4);
	    gp1_chain_ptr[27] = GP_UNIT(GP_TAG_UL,4,4);
#endif
	    
	    edge_type=(count + 1)%dFRM->mb_width==0?3:edge_type;
	    break;
	  }
	set_gp1_dha(TCSM1_PADDR(gp1_chain_ptr));   	
#ifdef JZC_PMON_P1
	PMON_ON(gp1);
#endif
	//poll_gp1_end();
	while(*tcsm1_gp1_poll_end);
	*tcsm1_gp1_poll_end = 1;     

#ifdef JZC_PMON_P1
	PMON_OFF(gp1);
#endif
	set_gp1_dcs();
      }//if(count)

    //poll_gp0_end();
    while(*tcsm1_gp0_poll_end);
    *tcsm1_gp0_poll_end = 1;     
        
    (*mbnum_rp)++;
    *addr_rp += dMB->next_dma_len;

    if((int)(*addr_rp)>=(TCSM0_PADDR(TCSM0_END)-TASK_BUF_LEN))
      *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);
    ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;

#ifdef JZC_PMON_P1
    PMON_ON(gp0);
#endif
    while(*mbnum_wp<=*mbnum_rp+2);//wait until the next next mb is ready

#ifdef JZC_PMON_P1
    PMON_OFF(gp0);
#endif

    gp0_chain_ptr[0]=addr_rp[0];
    gp0_chain_ptr[1]=TCSM1_PADDR(dMB_L);
    gp0_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,dMB_N->next_dma_len);

    gp0_chain_ptr[4] = TCSM0_PADDR(TCSM0_GP0_POLL_END);
    gp0_chain_ptr[5] = TCSM1_PADDR(TCSM1_GP0_POLL_END);
    gp0_chain_ptr[6] = GP_STRD(4,GP_FRM_NML,4);
    gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,4,4);
    
    set_gp0_dcs(); 
    count ++;

    last_mc_flag = mc_flag;
    XCHG3(dMB_L,dMB,dMB_N,xchg_tmp);
    XCHG2(dout_ptr0[0],dout_ptr1[0],xchg_tmp);
    XCHG2(dout_ptr0[1],dout_ptr1[1],xchg_tmp);
    XCHG2(dout_ptr2[1],dout_ptr2[2],xchg_tmp);

    XCHG2(mc_dsa[0],mc_dsa[1],xchg_tmp);
    XCHG2(mc_dha[0],mc_dha[1],xchg_tmp);

#ifdef JZC_ROTA90_OPT
    XCHG2(rota_buf1,rota_buf3,xchg_tmp);
    XCHG2(rota_buf2,rota_buf4,xchg_tmp);
#endif
    XCHG2(edge_bufy[0],edge_bufy[1],xchg_tmp);
    XCHG2(edge_bufc[0],edge_bufc[1],xchg_tmp);
    XCHG2(gp1_chain_ptr,gp1_chain_ptr1,xchg_tmp);
  }
  poll_gp1_end();
#ifdef JZC_PMON_P1
  pmon_ptr[0]=mc_cfg_pmon_val_useless_insn;
  pmon_ptr[1]=mc_polling_pmon_val_useless_insn;
  pmon_ptr[2]=idct_pmon_val_useless_insn;
  pmon_ptr[3]=gp0_pmon_val_useless_insn;
  pmon_ptr[4]=gp1_pmon_val_useless_insn;
  pmon_ptr[5]=gp2_pmon_val_useless_insn;
 
  pmon_ptr[6]=mc_cfg_pmon_val_piperdy;
  pmon_ptr[7]=mc_polling_pmon_val_piperdy;
  pmon_ptr[8]=idct_pmon_val_piperdy;
  pmon_ptr[9]=gp0_pmon_val_piperdy;
  pmon_ptr[10]=gp1_pmon_val_piperdy;
  pmon_ptr[11]=gp2_pmon_val_piperdy;

  pmon_ptr[12]=mc_cfg_pmon_val_cclk;
  pmon_ptr[13]=mc_polling_pmon_val_cclk;
  pmon_ptr[14]=idct_pmon_val_cclk;
  pmon_ptr[15]=gp0_pmon_val_cclk;
  pmon_ptr[16]=gp1_pmon_val_cclk;
  pmon_ptr[17]=gp2_pmon_val_cclk;
#endif

  
  *((volatile int *)TCSM0_PADDR(TCSM0_P1_TASK_DONE)) = 0x1;
  i_nop;  
  i_nop;    
  i_wait();
}
