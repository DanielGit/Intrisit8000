/*****************************************************************************
 *
 * JZ4760 HardWare Core Accelerate Init
 *
 * $Id: h264_vae_hwinit.c,v 1.7 2011/02/25 10:01:15 xqliang Exp $
 *
 ****************************************************************************/
#include "h264_dcore.h"
#include "h264_tcsm1.h"
#include "h264_sram.h"
#include "h264_dblk_coef_tables.h"
#include "../../libjzcommon/jz4760e_dblk_hw.h"
#include "../../libjzcommon/jz4760e_idct_hw.h"
#include "../../libjzcommon/jz4760e_spu.h"
#undef printf

/********************************** DBLK  Init ********************************/
void p1_dblk_frame_cabac_init(MpegEncContext *s){
  volatile int * dblk_base=(int*)DBLK_BASE;

  POLLING_DBLK_END_FLAG();
  
  dblk_base[1]=1|(1<<15);//FMT:row_end | h.264 to clear the rowmb_cnt reg
  dblk_base[0]=((1 << 31) |//update_addr
		(0 << 30) |//TRAN_EVER
		(0 << 29) |//MBOUT_MNTN
		(0 << 28) |//MBOUT_XCHG
		(0 << 26) |//MBIN_MNTN
		(0 << 25) |//MBIN_XCHG
		(0 << 23) |//UP_OUT_MNTN
		(0 << 22) |//UP_OUT_INCR
		(1 << 21) |//UP_IN_MNTN
		(0 << 20) |//UP_IN_INCR
		1//DBLK_EN
		);
  POLLING_DBLK_END_FLAG();

  //v_xchg_mbout_addr
  dblk_base[24]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U1+4+TCSM1_DBLK_MBOUT_UV_OFFSET);
  //u_xchg_mbout_addr
  dblk_base[23]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U1+4);
  //y_xchg_mbout_addr
  dblk_base[22]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y1+4);

  //v_mbout_addr
  dblk_base[18]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U0+4+TCSM1_DBLK_MBOUT_UV_OFFSET);
  //u_mbout_addr
  dblk_base[17]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U0+4);
  //y_mbout_addr
  dblk_base[16]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y0+4);

  //y_upi_stride
  dblk_base[25] = (s->mb_width<<16)|SRAM_DBLKUP_STRD_Y;

  //v_upin_addr
  dblk_base[15]=SRAM_PADDR(SRAM_DBLKUP_V);
  //u_upin_addr
  dblk_base[14]=SRAM_PADDR(SRAM_DBLKUP_U);
  //y_upin_addr
  dblk_base[13]=SRAM_PADDR(SRAM_DBLKUP_Y);
}

#if 1 //p1_dblk_frame_cavlc_init
void p1_dblk_frame_cavlc_init(MpegEncContext *s){
  volatile int * dblk_base=(int*)DBLK_BASE;

  POLLING_DBLK_END_FLAG();
  
  dblk_base[1]=1|(1<<15);//FMT:row_end | h.264 to clear the rowmb_cnt reg
  dblk_base[0]=((1 << 31) |//update_addr
		(0 << 30) |//TRAN_EVER
		(0 << 29) |//MBOUT_MNTN
		(0 << 28) |//MBOUT_XCHG
		(0 << 26) |//MBIN_MNTN
		(0 << 25) |//MBIN_XCHG
		(0 << 23) |//UP_OUT_MNTN
		(0 << 22) |//UP_OUT_INCR
		(1 << 21) |//UP_IN_MNTN
		(0 << 20) |//UP_IN_INCR
		1//DBLK_EN
		);
  POLLING_DBLK_END_FLAG();

  //v_xchg_mbout_addr
  dblk_base[24]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U1+4+TCSM1_DBLK_MBOUT_UV_OFFSET);
  //u_xchg_mbout_addr
  dblk_base[23]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U1+4);
  //y_xchg_mbout_addr
  dblk_base[22]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y1+4);

  //v_mbout_addr
  dblk_base[18]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U0+4+TCSM1_DBLK_MBOUT_UV_OFFSET);
  //u_mbout_addr
  dblk_base[17]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U0+4);
  //y_mbout_addr
  dblk_base[16]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y0+4);

  //y_upi_stride
  dblk_base[25] = (s->mb_width<<16)|SRAM_DBLKUP_STRD_Y;

  //v_upin_addr
  dblk_base[15]=SRAM_PADDR(SRAM_DBLKUP_V);
  //u_upin_addr
  dblk_base[14]=SRAM_PADDR(SRAM_DBLKUP_U);
  //y_upin_addr
  dblk_base[13]=SRAM_PADDR(SRAM_DBLKUP_Y);
}
#else //p1_dblk_frame_cavlc_init
void p1_dblk_frame_cavlc_init(MpegEncContext *s){
  volatile int * dblk_base=(int*)DBLK_BASE;

  POLLING_DBLK_END_FLAG();
  dblk_base[1]=2|(1<<15);//FMT:row_end | h.264 to clear the rowmb_cnt reg
  dblk_base[0]=((1 << 31) |//update_addr
		(1 << 30) |//TRAN_EVER
		(0 << 29) |//MBOUT_MNTN
		(0 << 28) |//MBOUT_XCHG
		(0 << 26) |//MBIN_MNTN
		(0 << 25) |//MBIN_XCHG
		(1 << 23) |//UP_OUT_MNTN
		(0 << 22) |//UP_OUT_INCR
		(1 << 21) |//UP_IN_MNTN
		(0 << 20) |//UP_IN_INCR
		1//DBLK_EN
		);

  POLLING_DBLK_END_FLAG();

  //v_xchg_mbout_addr
  dblk_base[24]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U1+4+TCSM1_DBLK_MBOUT_UV_OFFSET);
  //u_xchg_mbout_addr
  dblk_base[23]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U1+4);
  //y_xchg_mbout_addr
  dblk_base[22]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y1+4);

  //v_mbout_addr
  dblk_base[18]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U0+4+TCSM1_DBLK_MBOUT_UV_OFFSET);
  //u_mbout_addr
  dblk_base[17]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U0+4);
  //y_mbout_addr
  dblk_base[16]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y0+4);

  //y_upi_stride
  dblk_base[25] = (s->mb_width<<16)|SRAM_DBLKUP_STRD_Y;

  //v_upin_addr
  dblk_base[15]=SRAM_PADDR(SRAM_DBLKUP_V);
  //u_upin_addr
  dblk_base[14]=SRAM_PADDR(SRAM_DBLKUP_U);
  //y_upin_addr
  dblk_base[13]=SRAM_PADDR(SRAM_DBLKUP_Y);

  //v_upout_addr
  dblk_base[12]=TCSM1_PADDR(TCSM1_DBLK_UPOUT_U0+TCSM1_DBLK_UPOUT_UV_OFFSET);
  //u_upout_addr
  dblk_base[11]=TCSM1_PADDR(TCSM1_DBLK_UPOUT_U0);
  //y_upout_addr
  dblk_base[10]=TCSM1_PADDR(TCSM1_DBLK_UPOUT_Y0);

}
#endif

void p1_dblk_decode_cabac_init(){
  unsigned int * node;
  volatile int * dblk_base=(int*)DBLK_BASE;
  dblk_base[0] = 0x4;           //set end_flag for first polling
  dblk_base[32] = TCSM1_DBLK_MBOUT_STRD_C;              //uv_out_stride
  dblk_base[31] = (1<<16)|TCSM1_DBLK_MBOUT_STRD_Y;      //y_out_stride
  dblk_base[30] = PREVIOUS_CHROMA_STRIDE;               //uv_in_stride
  dblk_base[29] = PREVIOUS_LUMA_STRIDE;                 //y_in_stride
  dblk_base[28] = TCSM1_DBLK_UPOUT_STRD_C;              //uv_upo_stride
  dblk_base[27] = (1<<16)|TCSM1_DBLK_UPOUT_STRD_Y;      //y_upo_stride
  dblk_base[26] = SRAM_DBLKUP_STRD_C;                   //uv_upi_stride
  dblk_base[25] = SRAM_DBLKUP_STRD_Y;                   //y_upi_stride

  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN0);

  node[0] = 0x80000000;
  node[1] = 0x300d0048;    
  //31~24:CSA 23~16:chain_word_len 6:nd_dn 5~4: nd_type 3:hsk 2:ckg 1:cy 0:lk

  node[14] = ( (0<<31) | /*only maintain the addr, no transfer and filter*/
	       (1<<30) | /*transfer for ever*/
	       (1<<29) | /*maintain mb_out addr*/
	       (1<<28) | /*mb_out exchange*/
	       (0<<27) | /*mb_out wrap*/
	       (0<<26) | /*maintain mb_in addr*/
	       (0<<25) | /*mb_in exchange*/
	       (0<<24) | /*mb_in wrap*/
	       (0<<23) | /*maintain mb_up_out addr*/
	       (0<<22) | /*mb_up_out incremental*/
	       (1<<21) | /*maintain mb_up_in addr*/
	       (0<<20) | /*mb_up_in incremental*/
	       (1) );    /*dblk enable*/

  node[15] = 0x80000000;
  node[16] = 0x0001006C;//address node
  node[17] = TCSM1_PADDR(DBLK_END_FLAG);
  node[18] = DBLK_SYN_VALUE;

  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN1);

  node[0] = 0x80000000;
  node[1] = 0x300d0048;
  //31~24:CSA 23~16:chain_word_len 6:nd_dn 5~4: nd_type 3:hsk 2:ckg 1:cy 0:lk

  node[14] = ( (0<<31) | /*only maintain the addr, no transfer and filter*/
	       (1<<30) | /*transfer for ever*/
	       (1<<29) | /*maintain mb_out addr*/
	       (1<<28) | /*mb_out exchange*/
	       (0<<27) | /*mb_out wrap*/
	       (0<<26) | /*maintain mb_in addr*/
	       (0<<25) | /*mb_in exchange*/
	       (0<<24) | /*mb_in wrap*/
	       (0<<23) | /*maintain mb_up_out addr*/
	       (0<<22) | /*mb_up_out incremental*/
	       (1<<21) | /*maintain mb_up_in addr*/
	       (0<<20) | /*mb_up_in incremental*/
	       (1) );    /*dblk enable*/

  node[15] = 0x80000000;
  node[16] = 0x0001006C;//address node
  node[17] = TCSM1_PADDR(DBLK_END_FLAG);
  node[18] = DBLK_SYN_VALUE;
}

#if 1 //p1_dblk_decode_cavlc_init
void p1_dblk_decode_cavlc_init(){
  unsigned int * node;
  volatile int * dblk_base=(int*)DBLK_BASE;
  dblk_base[0] = 0x4;           //set end_flag for first polling
  dblk_base[32] = TCSM1_DBLK_MBOUT_STRD_C;              //uv_out_stride
  dblk_base[31] = (1<<16)|TCSM1_DBLK_MBOUT_STRD_Y;      //y_out_stride
  dblk_base[30] = PREVIOUS_CHROMA_STRIDE;               //uv_in_stride
  dblk_base[29] = PREVIOUS_LUMA_STRIDE;                 //y_in_stride
  dblk_base[28] = TCSM1_DBLK_UPOUT_STRD_C;              //uv_upo_stride
  dblk_base[27] = (1<<16)|TCSM1_DBLK_UPOUT_STRD_Y;      //y_upo_stride
  dblk_base[26] = SRAM_DBLKUP_STRD_C;                   //uv_upi_stride
  dblk_base[25] = SRAM_DBLKUP_STRD_Y;                   //y_upi_stride

  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN0);

  node[0] = 0x80000000;
  node[1] = 0x300d0048;    
  //31~24:CSA 23~16:chain_word_len 6:nd_dn 5~4: nd_type 3:hsk 2:ckg 1:cy 0:lk

  node[14] = ( (0<<31) | /*only maintain the addr, no transfer and filter*/
	       (1<<30) | /*transfer for ever*/
	       (1<<29) | /*maintain mb_out addr*/
	       (1<<28) | /*mb_out exchange*/
	       (0<<27) | /*mb_out wrap*/
	       (0<<26) | /*maintain mb_in addr*/
	       (0<<25) | /*mb_in exchange*/
	       (0<<24) | /*mb_in wrap*/
	       (0<<23) | /*maintain mb_up_out addr*/
	       (0<<22) | /*mb_up_out incremental*/
	       (1<<21) | /*maintain mb_up_in addr*/
	       (0<<20) | /*mb_up_in incremental*/
	       (1) );    /*dblk enable*/

  node[15] = 0x80000000;
  node[16] = 0x0001006C;//address node
  node[17] = TCSM1_PADDR(DBLK_END_FLAG);
  node[18] = DBLK_SYN_VALUE;

  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN1);

  node[0] = 0x80000000;
  node[1] = 0x300d0048;
  //31~24:CSA 23~16:chain_word_len 6:nd_dn 5~4: nd_type 3:hsk 2:ckg 1:cy 0:lk

  node[14] = ( (0<<31) | /*only maintain the addr, no transfer and filter*/
	       (1<<30) | /*transfer for ever*/
	       (1<<29) | /*maintain mb_out addr*/
	       (1<<28) | /*mb_out exchange*/
	       (0<<27) | /*mb_out wrap*/
	       (0<<26) | /*maintain mb_in addr*/
	       (0<<25) | /*mb_in exchange*/
	       (0<<24) | /*mb_in wrap*/
	       (0<<23) | /*maintain mb_up_out addr*/
	       (0<<22) | /*mb_up_out incremental*/
	       (1<<21) | /*maintain mb_up_in addr*/
	       (0<<20) | /*mb_up_in incremental*/
	       (1) );    /*dblk enable*/

  node[15] = 0x80000000;
  node[16] = 0x0001006C;//address node
  node[17] = TCSM1_PADDR(DBLK_END_FLAG);
  node[18] = DBLK_SYN_VALUE;
}
#else //p1_dblk_decode_cavlc_init
void p1_dblk_decode_cavlc_init(){
  unsigned int * node;
  volatile int * dblk_base=(int*)DBLK_BASE;
  dblk_base[0] = 0x4;           //set end_flag for first polling
  dblk_base[32] = TCSM1_DBLK_MBOUT_STRD_C;              //uv_out_stride
  dblk_base[31] = (1<<16)|TCSM1_DBLK_MBOUT_STRD_Y;      //y_out_stride
  dblk_base[30] = PREVIOUS_CHROMA_STRIDE;               //uv_in_stride
  dblk_base[29] = PREVIOUS_LUMA_STRIDE;                 //y_in_stride
  dblk_base[28] = TCSM1_DBLK_UPOUT_STRD_C;              //uv_upo_stride
  dblk_base[27] = (2<<16)|TCSM1_DBLK_UPOUT_STRD_Y;      //y_upo_stride
  dblk_base[26] = SRAM_DBLKUP_STRD_C;                   //uv_upi_stride
  dblk_base[25] = SRAM_DBLKUP_STRD_Y;                   //y_upi_stride

  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN0);

  node[0] = 0x80000000;
  node[1] = 0x240a0048;    
  //31~24:CSA 23~16:chain_word_len 6:nd_dn 5~4: nd_type 3:hsk 2:ckg 1:cy 0:lk

  node[11] = ( (0<<31) | /*only maintain the addr, no transfer and filter*/
	       (1<<30) | /*transfer for ever*/
	       (1<<29) | /*maintain mb_out addr*/
	       (1<<28) | /*mb_out exchange*/
	       (0<<27) | /*mb_out wrap*/
	       (0<<26) | /*maintain mb_in addr*/
	       (0<<25) | /*mb_in exchange*/
	       (0<<24) | /*mb_in wrap*/
	       (1<<23) | /*maintain mb_up_out addr*/
	       (0<<22) | /*mb_up_out incremental*/
	       (1<<21) | /*maintain mb_up_in addr*/
	       (0<<20) | /*mb_up_in incremental*/
	       (1) );    /*dblk enable*/

  node[12] = 0x80000000;
  node[13] = 0x0001006C;//address node
  node[14] = TCSM1_PADDR(DBLK_END_FLAG);
  node[15] = DBLK_SYN_VALUE;

  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN1);

  node[0] = 0x80000000;
  node[1] = 0x240a0048;
  //31~24:CSA 23~16:chain_word_len 6:nd_dn 5~4: nd_type 3:hsk 2:ckg 1:cy 0:lk

  node[11] = ( (0<<31) | /*only maintain the addr, no transfer and filter*/
	       (1<<30) | /*transfer for ever*/
	       (1<<29) | /*maintain mb_out addr*/
	       (1<<28) | /*mb_out exchange*/
	       (0<<27) | /*mb_out wrap*/
	       (0<<26) | /*maintain mb_in addr*/
	       (0<<25) | /*mb_in exchange*/
	       (0<<24) | /*mb_in wrap*/
	       (1<<23) | /*maintain mb_up_out addr*/
	       (0<<22) | /*mb_up_out incremental*/
	       (1<<21) | /*maintain mb_up_in addr*/
	       (0<<20) | /*mb_up_in incremental*/
	       (1) );    /*dblk enable*/

  node[12] = 0x80000000;
  node[13] = 0x0001006C;//address node
  node[14] = TCSM1_PADDR(DBLK_END_FLAG);
  node[15] = DBLK_SYN_VALUE;

}
#endif //p1_dblk_decode_cavlc_init

void h264_dblk_tab_init() {
  int tmp=0;
  int i;
  unsigned char *alpha_p = (unsigned char *)(alpha_table + 52);
  unsigned char *beta_p  = (unsigned char *)(beta_table  + 52);
  unsigned char (*tc0_p)[3*52][3] = (unsigned char *)(tc0_table + 52);

  // alpha beta lut
  tmp = (beta_p[17] << 24) | (alpha_p[17] << 16);
  SET_AB_LUT(0, tmp);
  for (i = 1; i < 18; i++) { 
    tmp = ( (beta_p[17 + i*2] << 24)  | (alpha_p[17 + i*2] << 16) |
	    (beta_p[16 + i*2] << 8)   | alpha_p[16 + i*2] ); 
    SET_AB_LUT((i*4), tmp);
  }

  // tc0 lut
  for (i = 0; i < 18; i++) {
    tmp = ( ((*tc0_p)[17+i*2][2] << 25) |
	    ((*tc0_p)[17+i*2][1] << 20) |
	    ((*tc0_p)[17+i*2][0] << 16) |
	    ((*tc0_p)[16+i*2][2] << 9)  |
	    ((*tc0_p)[16+i*2][1] << 4)  |
	    ((*tc0_p)[16+i*2][0]     )  );
    SET_TC0_LUT((i*4), tmp);
  }
}

/********************************** IDCT  Init ********************************/
#define hw_h264_lowres_idct_initial() \
({\
  enable_idct() ;\
  fresh_idct(1) ;\
  fresh_idct(0) ;\
  set_idct_type_video(1, \
		 2   \
		 ) ;\
  set_idct_stride(2*4, \
	     2*4  \
	     );\
  set_idct_block_width(32, 32);\
})

#define hw_h264_idct_add_initial()\
({\
  enable_idct() ;\
  fresh_idct(1) ;\
  fresh_idct(0) ;\
  set_idct_type_video(1, \
		 1 \
		 ) ;\
  set_idct_stride(2*4,\
	     2*4 \
	     );\
  set_idct_block_width(32, 32);  \
})

#define hw_h264_idct8_add_initial()\
({\
  enable_idct() ;\
  fresh_idct(1) ;\
  fresh_idct(0) ;\
  set_idct_type_video(1, \
		 1  \
		 ) ;\
  set_idct_stride(2*4, \
	     2*4  \
	     );\
  set_idct_block_width(32, 32);\
})

void h264_idct_cabac_init() {
  unsigned int * idct_chain_ptr= TCSM1_VUCADDR(IDCT_DES_CHAIN);
  idct_chain_ptr[0] = 0x80000000;
  idct_chain_ptr[1] = INTRA_8x8_NOD_HEAD;

  idct_chain_ptr[19] = 0x80000000;
  idct_chain_ptr[20] = INTER_NOD_HEAD;

  idct_chain_ptr[24]=0x80000000;
  idct_chain_ptr[25]=0x00010024;//address node
  idct_chain_ptr[26]=TCSM1_PADDR(IDCT_END_FLAG);
  idct_chain_ptr[27]=IDCT_SYN_VALUE;

  idct_chain_ptr= TCSM1_VUCADDR(IDCT_DES_CHAIN1);
  idct_chain_ptr[0] = 0x80000000;
  idct_chain_ptr[1] = INTRA_8x8_NOD_HEAD;

  idct_chain_ptr[19] = 0x80000000;
  idct_chain_ptr[20] = INTER_NOD_HEAD;

  idct_chain_ptr[24]=0x80000000;
  idct_chain_ptr[25]=0x00010024;//address node
  idct_chain_ptr[26]=TCSM1_PADDR(IDCT_END_FLAG);
  idct_chain_ptr[27]=IDCT_SYN_VALUE;


  hw_h264_idct_add_initial();
  desp_enable_idct();
}

void h264_idct_cavlc_init() {
  unsigned int * idct_chain_ptr= TCSM1_VUCADDR(IDCT_DES_CHAIN);
  idct_chain_ptr[0] = 0x80000000;
  idct_chain_ptr[1] = INTRA_8x8_NOD_HEAD;

  idct_chain_ptr[19] = 0x80000000;
  idct_chain_ptr[20] = INTER_NOD_HEAD;

  idct_chain_ptr[24]=0x80000000;
  idct_chain_ptr[25]=0x00010024;//address node
  idct_chain_ptr[26]=TCSM1_PADDR(IDCT_END_FLAG);
  idct_chain_ptr[27]=IDCT_SYN_VALUE;

  hw_h264_idct_add_initial();
  desp_enable_idct();
}
