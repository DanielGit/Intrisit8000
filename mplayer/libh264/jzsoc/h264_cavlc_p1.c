#define P1_USE_PADDR

#define av_always_inline __attribute__((always_inline)) inline
#define __p1_text __attribute__ ((__section__ (".p1_text")))
#define __p1_main __attribute__ ((__section__ (".p1_main")))
#define __p1_data __attribute__ ((__section__ (".p1_data")))

#include "h264_p1_types.h"
#include "h264_dcore.h"
#include "h264_tcsm0.h"
#include "h264_tcsm1.h"
#include "h264_sram.h"
#include "../../libjzcommon/jzsys.h"
#include "../../libjzcommon/jzasm.h"
#include "../../libjzcommon/jzmedia.h"
#include "../../libjzcommon/jz4760e_2ddma_hw.h"
#include "../../libjzcommon/jz4760e_idct_hw.h"
#include "../../libjzcommon/jz4760e_aux_pmon.h"
#include "../../libjzcommon/jz4760e_spu.h"
#include "../../libjzcommon/jz4760e_dblk_hw.h"
#ifdef JZC_PMON_P1
#include "../../libjzcommon/jz4760e_harb.h" 
#endif

#include "h264_cavlc_p1_idct.c"
#include "h264_p1_intra.c"

#include "h264_mc_hw.c"
#include "h264_p1_dblk.c"


/*
              dmbA     dmbB     dmbZ    dmbA     dmbB      dmbZ       dmbA     dmbB      dmbZ

ini            -------------
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
0              /\
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
1                       /\        fetching
                               -.-.-.-.-.-.-
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
2                              -------------
                               /\
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
3                                         /\          fetching
                                                  -.-.-.-.-.-.-   
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
4                                                 --------------
                                                   /\
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
5                                                             /\      fetching
                                                                      -.-.-.-.-.-.-   
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 */

#if 0
#define STOP_P1_() \
  ({	*(int*)TCSM1_ADDR_RP=0;						\
	((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*(int*)TCSM1_ADDR_RP;\
	*((volatile int *)(TCSM0_PADDR(TCSM0_P1_TASK_DONE))) = 0x1;\
	i_nop;  \
	i_nop;  \ 
	i_nop;  \   
	i_nop;  \
	i_wait();\
})
#endif

#ifdef JZC_PMON_P1
PMON_CREAT(total);
#define JZC_PMON_P1_TOTAL
//#define JZC_PMON_P1_PART
//#define JZC_PMON_P1_ROTA
//"You only use one macro among p1_part and p1_total!"
#define AHB1_ARB_PMON
#endif //JZC_PMON_P1


//#ifdef JZC_DCORE_SYN_DEBUG
//volatile int* mpframe;
//volatile int *debug_ptr;
//#endif

//volatile int *dbg_ptr;
//int mbnum;
volatile int *mbnum_wp;
volatile int *mbnum_rp;//FIXME: may use variable instead of pointer? gjwang
volatile int *addr_rp;
#define gp0_end_flag  (*(volatile int *)TCSM1_GP0_END_FLAG)
#define gp1_end_flag  (*(volatile int *)TCSM1_GP1_END_FLAG)


/*
 *NOTE: gp0 can NOT been polling more than once in a loop;
 *clear gp0_end_flag zero  before start gp0 can fix this
 */
//#define poll_gp0_end() do{while(!gp0_end_flag); gp0_end_flag = 0;}while(0)

#define POLL_MC_END()        do{}while(*(volatile int *)motion_mc_dsa1 != (0x80000000 | 0xFFFF) )
#define POLL_IDCT_END()      do{}while(*(volatile int*)IDCT_END_FLAG != IDCT_SYN_VALUE)
#define POLL_DBLK_END()      do{}while(*(volatile int*)DBLK_END_FLAG != DBLK_SYN_VALUE)
#define POLL_FIFO_IS_READY() do{}while(*mbnum_wp<=*mbnum_rp + 1)//wait until the next mb is ready


//#define INTRAUP_STRD_Y (1284)
#define INTRAUP_STRD_Y SRAM_INTRAUP_STRD_Y
#define INTRAUP_STRD_C (644)
//unsigned char INTRAUP_Y[INTRAUP_STRD_Y];
unsigned char *INTRAUP_Y = (unsigned char *)SRAM_PADDR(SRAM_INTRAUP_Y);

unsigned char INTRAUP_U[INTRAUP_STRD_C];
unsigned char INTRAUP_V[INTRAUP_STRD_C];

__p1_main void main() {
  //enable MXU 
  S32I2M(xr16, 0x3);

  /*---------------------------------------------------------------------
   * P1 Decode Task Begin
   *---------------------------------------------------------------------*/
  void *xchg_tmp;
  int mbnum;
  uint8_t *ydst, *udst;
  uint8_t *up_ydst, *up_udst;
  uint8_t *edge_tb_ydst, *edge_tb_cdst, *edge_ydst, *edge_cdst;
  uint8_t *src1, *dst1, *src2, *dst2, *src3, *dst3, *src4, *dst4;

  int y_mb_line, uv_mb_line, edge_type; 
#ifdef JZC_ROTA90_OPT
  uint8_t *rota_ydst, *rota_cdst, *rota_up_ydst, *rota_up_udst;
  int rota_y_mb_line, rota_uv_mb_line;  
#endif
  
  H264_Frame_LPtr *Lpicture;

  H264_Slice_GlbARGs *SLICE_T0=(H264_Slice_GlbARGs *)TCSM1_SLICE_BUF0;
  H264_Slice_GlbARGs *SLICE_T1=(H264_Slice_GlbARGs *)TCSM1_SLICE_BUF1;
  H264_MB_Ctrl_DecARGs *dmb_Z=(H264_MB_Ctrl_DecARGs *)TASK_BUF0;//last mb
  H264_MB_Ctrl_DecARGs *dmb_A=(H264_MB_Ctrl_DecARGs *)TASK_BUF1;//curr mb
  H264_MB_Ctrl_DecARGs *dmb_B=(H264_MB_Ctrl_DecARGs *)TASK_BUF2;//next mb

  uint8_t *motion_mc_dha1 = (uint8_t *)TCSM1_MC_DHA1;
  uint8_t *motion_mc_dsa1 = (uint8_t *)(TCSM1_MC_DHA1 + 0x108);

#ifdef JZC_ROTA90_OPT
  uint8_t *rota_mb_ybuf1 = (uint8_t *)TCSM1_ROTA_MB_YBUF1;
  uint8_t *rota_mb_ybuf2 = (uint8_t *)TCSM1_ROTA_MB_YBUF2;
  uint8_t *rota_mb_ybuf3 = (uint8_t *)TCSM1_ROTA_MB_YBUF3;
  uint8_t *rota_mb_cbuf1 = (uint8_t *)TCSM1_ROTA_MB_CBUF1;
  uint8_t *rota_mb_cbuf2 = (uint8_t *)TCSM1_ROTA_MB_CBUF2;
  uint8_t *rota_mb_cbuf3 = (uint8_t *)TCSM1_ROTA_MB_CBUF3;

  uint8_t *dblk_mb_ydst1 = (uint8_t *)TCSM1_DBLK_MBOUT_YDES;
  uint8_t *dblk_mb_ydst2 = (uint8_t *)TCSM1_DBLK_MBOUT_YDES1;
  uint8_t *dblk_mb_ydst3 = (uint8_t *)TCSM1_DBLK_MBOUT_YDES2;

  uint8_t *dblk_mb_udst1 = (uint8_t *)TCSM1_DBLK_MBOUT_UDES;
  uint8_t *dblk_mb_udst2 = (uint8_t *)TCSM1_DBLK_MBOUT_UDES1;
  uint8_t *dblk_mb_udst3 = (uint8_t *)TCSM1_DBLK_MBOUT_UDES2;

  uint8_t *dblk_upout_yaddr1 = (uint8_t *)TCSM1_DBLK_UPOUT_Y0;
  uint8_t *dblk_upout_yaddr2 = (uint8_t *)TCSM1_DBLK_UPOUT_Y1;
  
  uint8_t *dblk_upout_caddr1 = (uint8_t *)TCSM1_DBLK_UPOUT_U0;
  uint8_t *dblk_upout_caddr2 = (uint8_t *)TCSM1_DBLK_UPOUT_U1;
#endif

  uint8_t *edge_ybuf1 = (uint8_t *)TCSM1_EDGE_YBUF1;
  uint8_t *edge_cbuf1 = (uint8_t *)TCSM1_EDGE_CBUF1;
  uint8_t *edge_bak_ybuf1 = (uint8_t *)TCSM1_EDGE_BAK_YBUF1; 
  uint8_t *edge_bak_cbuf1 = (uint8_t *)TCSM1_EDGE_BAK_CBUF1; 

#ifdef JZC_PMON_P1
  int *pmon_ptr=(int*)P1_PMON_BUF;
  PMON_CREAT(total);
#endif//#ifdef JZC_PMON_P1

  //#ifdef JZC_DCORE_SYN_DEBUG
  //mpframe=TCSM1_DBG_RESERVE;
  //debug_ptr=TCSM1_DBG_RESERVE+4;
  //#endif

  *(volatile int*)IDCT_END_FLAG = IDCT_SYN_VALUE;
  *(volatile int*)DBLK_END_FLAG = DBLK_SYN_VALUE;
  *(volatile int*)motion_mc_dsa1 = (0x80000000 | 0xFFFF);
  
  mbnum = 0;
  mbnum_wp = (int *)TCSM1_MBNUM_WP;
  mbnum_rp = (int *)TCSM1_MBNUM_RP;
  addr_rp = (int *)TCSM1_ADDR_RP;
  *mbnum_rp = 0;
  *addr_rp = (int)TCSM0_PADDR(TCSM0_TASK_FIFO);

  //int kk;
  //dbg_ptr = TCSM1_MC_BUG_SPACE;
  //for(kk=0; kk<10; kk++) {dbg_ptr[kk] = 0;}

  H264_XCH2_T *XCH2_T0, *XCH2_T1;
  XCH2_T0 = (H264_XCH2_T *)TCSM1_XCH2_T_BUF0;
  XCH2_T1 = (H264_XCH2_T *)TCSM1_XCH2_T_BUF1;

  int i = 0 ;
  int j = 0 ;
  int rota_mx, rota_my, rota_up_mx, rota_up_my, tag1, tag2;

  unsigned int error_add_cbp = 0 ;
  unsigned char *recon_ptr0 = (unsigned char *)RECON_PREVIOUS_YBUF0;
  unsigned char *recon_ptr1 = (unsigned char *)RECON_PREVIOUS_YBUF1;
  unsigned char *recon_ptr2 = (unsigned char *)RECON_PREVIOUS_YBUF2;
  unsigned int *intra_top_y = (unsigned int *)(INTRAUP_Y+INTRAUP_STRD_Y-20);
  unsigned int *intra_top_u = (unsigned int *)(INTRAUP_U+INTRAUP_STRD_C-12);
  unsigned int *intra_top_v = (unsigned int *)(INTRAUP_V+INTRAUP_STRD_C-12);

  int *gp0_chain_ptr = (int *)DDMA_GP0_DES_CHAIN;
  int *gp1_chain_ptr = (int *)TCSM1_DDMA_GP1_DES_CHAIN1;
  int *gp1_chain_ptr2 = (int *)TCSM1_DDMA_GP1_DES_CHAIN2;
 
  unsigned intra_topleft_tmp[3];
  uint8_t  mb_x = 0;
  uint8_t  mb_x_d1=0, mb_x_d2=0, mb_x_d3=0;
  uint8_t  mb_y=0;
  uint8_t  mb_y_d1=0, mb_y_d2=0, mb_y_d3=0;

  gp0_end_flag = 0;//1: transmission finish; 0: transmission NOT finish
  gp1_end_flag = 0;//1: transmission finish; 0: transmission NOT finish

  POLL_FIFO_IS_READY();

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dmb_A);
  //gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,64,*(int*)TCSM1_FIRST_MBLEN);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,*(int*)TCSM1_FIRST_MBLEN);

  gp0_chain_ptr[5] = addr_rp[0];
  gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,64,*(int*)TCSM1_FIRST_MBLEN);

  set_gp0_dcs();

  (*mbnum_rp)++;
  *addr_rp+=*(int*)TCSM1_FIRST_MBLEN;
  if((int)(*addr_rp)>(0x132B4000-MAX_TASK_LEN))
    *addr_rp=TCSM0_PADDR(TCSM0_TASK_FIFO);

  poll_gp0_end();

  ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;

  gp0_chain_ptr[0]=addr_rp[0];
  gp0_chain_ptr[1]=TCSM1_PADDR(dmb_B);
  //gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,64,dmb_A->next_mb_len);
  gp0_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,dmb_A->next_mb_len);

  gp0_chain_ptr[5] = addr_rp[0];
  gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,64,dmb_A->next_mb_len);

  POLL_FIFO_IS_READY();
  set_gp0_dcs();

  *addr_rp+=dmb_A->next_mb_len;


  y_mb_line = (SLICE_T0->mb_width +4) * 256;
  uv_mb_line = (SLICE_T0->mb_width*2 + 8) * 64;
#ifdef JZC_ROTA90_OPT
  rota_y_mb_line = SLICE_T0->mb_height * 256;
  rota_uv_mb_line = SLICE_T0->mb_height * 128;
#endif
  Lpicture = &SLICE_T0->line_current_picture;

#ifdef AHB1_ARB_PMON

    #define AHB1_CHN0_MID_ AHB1_ID_MDMA1
    #define AHB1_CHN1_MID_ AHB1_ID_ALL

    #define AHB0_CHN0_MID_ AHB0_ID_ALL
    #define AHB0_CHN1_MID_ AHB0_ID_CORE0

    #define AHB1_BUS_MID 15
    #define AHB0_BUS_MID 15    

#if ( AHB1_BUS_MID == 0 )
#define AHB1_CHN0_MID_  AHB1_ID_CORE0  //2
#define AHB1_CHN1_MID_  AHB1_ID_IDCT  //184
#endif

#if ( AHB1_BUS_MID == 1 )
#define AHB1_CHN0_MID_ AHB1_ID_MC    // 528 // 570
#define AHB1_CHN1_MID_  AHB1_ID_DBLK // 447 // 447
#endif

#if ( AHB1_BUS_MID == 2 )
#define AHB1_CHN0_MID_ AHB1_ID_MDMA2 // 309 
#define AHB1_CHN1_MID_  AHB1_ID_MDMA1 // 179
#endif

#if ( AHB1_BUS_MID == 3 )
#define AHB1_CHN0_MID_ AHB1_ID_MDMA0 // 74
#define AHB1_CHN1_MID_  AHB1_ID_CABAC // 49
#endif

#if ( AHB1_BUS_MID == 4 )
#define AHB1_CHN0_MID_ AHB1_ID_CORE1 //28   //27 
#define AHB1_CHN1_MID_  AHB1_ID_ALL // 1847 //1844
#endif


#if ( AHB0_BUS_MID == 0 )
#define AHB0_CHN0_MID_ AHB0_ID_CIM//0
#define AHB0_CHN1_MID_ AHB0_ID_LCD //773 
#endif


#if ( AHB0_BUS_MID == 1 )
#define AHB0_CHN0_MID_ AHB0_ID_IPU   //0   //0
#define AHB0_CHN1_MID_ AHB0_ID_CORE1 //101 //67   
#endif


#if ( AHB0_BUS_MID == 2 )
#define AHB0_CHN0_MID_ AHB0_ID_DMA//0
#define AHB0_CHN1_MID_ AHB0_ID_CABAC //11   
#endif

#if ( AHB0_BUS_MID == 3 )
#define AHB0_CHN0_MID_ AHB0_ID_CORE0//610 //213
#define AHB0_CHN1_MID_ AHB0_ID_MDMA0//0   //0
#endif

#if ( AHB0_BUS_MID == 4 )
#define AHB0_CHN0_MID_ AHB0_ID_MDMA1//118 //118
#define AHB0_CHN1_MID_ AHB0_ID_MDMA2//1278 //1285
#endif

#if ( AHB0_BUS_MID == 5 )
#define AHB0_CHN0_MID_ AHB0_ID_AHB0_AHB2_BRI //0    //0
#define AHB0_CHN1_MID_ AHB0_ID_ALL           //2948 //2471
#endif
    
    arb_ch1_clk_wr(AHB1, 0);
    arb_ch0_evenl_wr(AHB1, 0);
    arb_ch1_evenl_wr(AHB1, 0);    
    arb_eveclk_h_wr(AHB1, 0);
	
    arb_clk_ctrl(AHB1, 0);
    arb_chn0_even_ctrl(AHB1, AHB1_CHN0_MID_, ABME_BTC, 0);
    arb_chn1_even_ctrl(AHB1, AHB1_CHN1_MID_, ABME_BTC, 0);

    arb_ch1_clk_wr(AHB0, 0);
    arb_ch0_evenl_wr(AHB0, 0);
    arb_ch1_evenl_wr(AHB0, 0);    
    arb_eveclk_h_wr(AHB0, 0);
    arb_clk_ctrl(AHB0, 0);
    arb_chn0_even_ctrl(AHB0, AHB0_CHN0_MID_, ABME_BTC,0);
    arb_chn1_even_ctrl(AHB0, AHB0_CHN1_MID_, ABME_BTC,0);    
    
    arb_reg(AHB0, ARB_CTRL) = arb_reg(AHB0, ARB_CTRL) | 0x7;
    arb_reg(AHB1, ARB_CTRL) = arb_reg(AHB1, ARB_CTRL) | 0x7;    
#endif

    gp1_chain_ptr[10]=GP_STRD(192,GP_FRM_NML,192);
    gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,192,192);
	
    gp1_chain_ptr[14]=GP_STRD(64,GP_FRM_NML,64);
    gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,64,64);

    gp1_chain_ptr2[10]=GP_STRD(192,GP_FRM_NML,192);
    gp1_chain_ptr2[11]=GP_UNIT(GP_TAG_LK,192,192);
	
    gp1_chain_ptr2[14]=GP_STRD(64,GP_FRM_NML,64);
    gp1_chain_ptr2[15]=GP_UNIT(GP_TAG_LK,64,64);

  /*---------------------------------------------------------------------
   * P1 Decode Task Main Loop
   *---------------------------------------------------------------------*/

  while(dmb_A->new_slice>=0){
#ifdef JZC_PMON_P1_TOTAL
    PMON_ON(total);
#endif

    mb_x_d3=mb_x_d2;
    mb_x_d2=mb_x_d1;
    mb_x_d1=mb_x;
    mb_x= dmb_A->mb_x;
    mb_y_d3=mb_y_d2;
    mb_y_d2=mb_y_d1;
    mb_y_d1=mb_y;
    mb_y= dmb_A->mb_y;
    edge_type = 1;
    
    POLL_MC_END();

    if(dmb_A->new_slice==1){
      XCHG2(SLICE_T0,SLICE_T1,xchg_tmp);

      if(SLICE_T0->slice_type != I_TYPE){
	SET_REG1_WINFO(0,(SLICE_T0->use_weight == IS_WT1),\
		       SLICE_T0->use_weight,1,SLICE_T0->luma_log2_weight_denom,5,0,0);
	SET_REG2_WINFO1(0,SLICE_T0->use_weight_chroma && (SLICE_T0->use_weight == IS_WT1), \
			SLICE_T0->use_weight, 1,SLICE_T0->chroma_log2_weight_denom,5,0,0);

	for(i=0; i<16; i++){ 
	  SET_TAB1_RLUT_WCOEF(i, SLICE_T0->luma_weight[i][0][0], SLICE_T0->luma_weight[i][0][1]); 
	  SET_TAB1_RLUT_WCOEF(16+i, SLICE_T0->luma_weight[i][1][0], SLICE_T0->luma_weight[i][1][1]); 
	  SET_TAB2_RLUT_WCOEF(i,SLICE_T0->chroma_weight[i][0][1][0], SLICE_T0->chroma_weight[i][0][1][1], 
			    SLICE_T0->chroma_weight[i][0][0][0], SLICE_T0->chroma_weight[i][0][0][1]); 
	  SET_TAB2_RLUT_WCOEF(16+i, SLICE_T0->chroma_weight[i][1][1][0], SLICE_T0->chroma_weight[i][1][1][1], 
			    SLICE_T0->chroma_weight[i][1][0][0], SLICE_T0->chroma_weight[i][1][0][1]); 
	}
      }
    }

    if(!IS_INTRA(dmb_A->mb_type)){
      motion_execute(SLICE_T0, dmb_A, recon_ptr0,motion_mc_dha1);
      SET_REG1_DDC(TCSM1_PADDR((int)motion_mc_dha1) | 0x1);
    }

    /*---------------------------------------------------------------------
     * add error
     *---------------------------------------------------------------------*/
    POLL_IDCT_END();
    func_add_error(error_add_cbp,recon_ptr2,XCH2_T0->add_error_buf);

    /*---------------------------------------------------------------------
     * back up intra bottom
     *---------------------------------------------------------------------*/
    intra_top_y[0] = intra_topleft_tmp[0];
    intra_top_y[1] = ((unsigned int *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[0];
    intra_top_y[2] = ((unsigned int *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[1];
    intra_top_y[3] = ((unsigned int *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[2];
    intra_top_u[0] = intra_topleft_tmp[1];
    intra_top_u[1] = ((unsigned int *)(recon_ptr2+PREVIOUS_OFFSET_U+7*PREVIOUS_CHROMA_STRIDE))[0];
    intra_top_v[0] = intra_topleft_tmp[2];
    intra_top_v[1] = ((unsigned int *)(recon_ptr2+PREVIOUS_OFFSET_V+7*PREVIOUS_CHROMA_STRIDE))[0];

    if(mb_x){
      intra_top_y+=4;
      intra_top_u+=2;
      intra_top_v+=2;
      intra_topleft_tmp[0] = ((unsigned int *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[3];
      intra_topleft_tmp[1] = ((unsigned int *)(recon_ptr2+PREVIOUS_OFFSET_U+7*PREVIOUS_CHROMA_STRIDE))[1];
      intra_topleft_tmp[2] = ((unsigned int *)(recon_ptr2+PREVIOUS_OFFSET_V+7*PREVIOUS_CHROMA_STRIDE))[1];
    }else{
      intra_top_y[4] = ((unsigned int *)(recon_ptr2+15*PREVIOUS_LUMA_STRIDE))[3];
      intra_top_u[2] = ((unsigned int *)(recon_ptr2+PREVIOUS_OFFSET_U+7*PREVIOUS_CHROMA_STRIDE))[1];
      intra_top_v[2] = ((unsigned int *)(recon_ptr2+PREVIOUS_OFFSET_V+7*PREVIOUS_CHROMA_STRIDE))[1];
      intra_top_y = (unsigned int *)INTRAUP_Y;
      intra_top_u = (unsigned int *)INTRAUP_U;
      intra_top_v = (unsigned int *)INTRAUP_V;
    }

    error_add_cbp = hw_idct_cavlc_config(dmb_A,XCH2_T0->add_error_buf,intra_top_y,recon_ptr2);

    *(volatile int*)IDCT_END_FLAG=0;
    run_idct_ddma();

    /*---------------------------------------------------------------------
     * back up mv for dblk_configure
     *---------------------------------------------------------------------*/
    if((!IS_INTRA(dmb_Z->mb_type)) && mbnum){
      unsigned int *Inter_Dec_base = (unsigned int *)(((uint32_t)dmb_Z)+sizeof(struct H264_MB_Ctrl_DecARGs)+4);
      int is_bframe=SLICE_T0->slice_type-2;
      int mv_num=Inter_Dec_base[-1]<<is_bframe;
      int fix_len=is_bframe?21:11;
      unsigned int *tmp_dblk_mv_ptr = (unsigned int *)(XCH2_T1->dblk_mv_ptr - 4);
      uint32_t tmp_Inter_Dec_base = (uint32_t)Inter_Dec_base - 4;
      int packed_num = (fix_len + mv_num + 3) >> 2;
      for(i=0;i<packed_num;i++){
	S32LDI(xr1, tmp_Inter_Dec_base, 4);
	S32LDI(xr2, tmp_Inter_Dec_base, 4);
	S32LDI(xr3, tmp_Inter_Dec_base, 4);
	S32LDI(xr4, tmp_Inter_Dec_base, 4);
	S32SDI(xr1, tmp_dblk_mv_ptr, 4);
	S32SDI(xr2, tmp_dblk_mv_ptr, 4);
	S32SDI(xr3, tmp_dblk_mv_ptr, 4);
	S32SDI(xr4, tmp_dblk_mv_ptr, 4);
      }
    }

    (*mbnum_rp)++;

    if((int)(*addr_rp)>(0x132B4000-MAX_TASK_LEN))
      *addr_rp = TCSM0_PADDR(TCSM0_TASK_FIFO);

    poll_gp0_end();

    ((volatile int *)TCSM0_PADDR(TCSM0_P1_FIFO_RP))[0]=*addr_rp;

    gp0_chain_ptr[0]=addr_rp[0];
    gp0_chain_ptr[1]=TCSM1_PADDR(dmb_Z);
    //gp0_chain_ptr[3]=GP_UNIT(GP_TAG_UL,64,dmb_B->next_mb_len);
    gp0_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,dmb_B->next_mb_len);

    gp0_chain_ptr[5] = addr_rp[0];
    gp0_chain_ptr[7] = GP_UNIT(GP_TAG_UL,64,dmb_B->next_mb_len);

    
#ifdef AHB1_ARB_PMON
      {
	arb_reg(AHB1, ARB_CTRL) = arb_reg(AHB1, ARB_CTRL) & 0xFFFFFFF0;
      }	
#endif
    POLL_FIFO_IS_READY(); 
#ifdef AHB1_ARB_PMON
      {
	arb_reg(AHB1, ARB_CTRL) = arb_reg(AHB1, ARB_CTRL) | 0x7;    
      }	
#endif

    set_gp0_dcs();//after add_error and backup dblk's mv 

    *addr_rp+=dmb_B->next_mb_len;


    /*---------------------------------------------------------------------
     * Intra/Inter Prediction
     *---------------------------------------------------------------------*/
    if(IS_INTRA(dmb_A->mb_type)){

      H264_MB_Intra_DecARGs *INTRA_T=(H264_MB_Intra_DecARGs*)((unsigned)(dmb_A)+sizeof(struct H264_MB_Ctrl_DecARGs));
      if(!IS_INTRA4x4(dmb_A->mb_type))
	Intra_pred_luma_16x16(INTRA_T->intra16x16_pred_mode,recon_ptr0,recon_ptr2+16,(uint8_t *)(intra_top_y+1),dmb_A);

      Intra_pred_chroma(INTRA_T->chroma_pred_mode, recon_ptr0+PREVIOUS_OFFSET_U, 
			recon_ptr2+PREVIOUS_OFFSET_U+8, (uint8_t *)(intra_top_u+1));
      Intra_pred_chroma(INTRA_T->chroma_pred_mode, recon_ptr0+PREVIOUS_OFFSET_V, 
  			recon_ptr2+PREVIOUS_OFFSET_V+8, (uint8_t *)(intra_top_v+1));
    }

    /*---------------------------------------------------------------------
     * back up dblk left; dblk hw configure
     *---------------------------------------------------------------------*/
    if( mbnum ){ /*LEFT copy*/
	src1 = (uint8_t *)(XCH2_T0->dblk_out_addr + 16 - TCSM1_DBLK_MBOUT_STRD_Y);
	dst1 = (uint8_t *)(recon_ptr2 - 4 - PREVIOUS_LUMA_STRIDE);
	src2 = (uint8_t *)(XCH2_T0->dblk_out_addr_c + 8 - TCSM1_DBLK_MBOUT_STRD_C);
	dst2 = (uint8_t *)(recon_ptr2+PREVIOUS_OFFSET_U - 4 - PREVIOUS_CHROMA_STRIDE);

        POLL_DBLK_END();

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

	SET_DBLK_DHA(TCSM1_PADDR(XCH2_T1->dblk_des_ptr));
	*(volatile int*)DBLK_END_FLAG=0;
	SET_DBLK_DCS(0x1);
    }

    //config dblk, must be after dblk polling
    filter_mb_hw(SLICE_T0, XCH2_T0,dmb_A, recon_ptr0,dblk_upout_yaddr1);
   
    if (mbnum>1){
      tag1 = (mb_y_d2+1>=SLICE_T0->mb_height) ? 1 : 0; 
      tag2 = tag1 ? 0 : 4; 

      for(i=0; i<4; i++){
        src1 = XCH2_T0->dblk_out_addr + 20*(16-tag2) + i*4;
        if (i){
	  dst2 = dblk_mb_ydst1 + 16*(16-tag2) + (i - 1)*4;
	  dst1 = rota_mb_ybuf1 + (i-1)*64 - 4*tag1;
        }else{
	  dst2 = dblk_mb_ydst3 + 16*(16-tag2) + 12;
	  dst1 = rota_mb_ybuf3 + 192 - 4*tag1 ;
        }

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
      }

      for(i=0; i<2; i++){
        src3 = XCH2_T0->dblk_out_addr_c + 12*(8-tag2) + i*4;
	src4 = src3 + TCSM1_DBLK_MBOUT_UV_OFFSET;
        if (i){
	  dst3 = dblk_mb_udst1 + 16*(8-tag2);
          dst4 = dst3 + 8;
          dst1 = rota_mb_cbuf1 - 4*tag1;
          dst2 = dst1 + 8;
        }else{
          dst3 = dblk_mb_udst3 + 16*(8-tag2) + 4;
          dst4 = dst3 + 8;
          dst1 = rota_mb_cbuf3 + 64 - 4*tag1;
          dst2 = dst1 + 8;
        }

        for(j=0; j<1+tag1; j++){
	  S32LDI(xr1,src3,-TCSM1_DBLK_MBOUT_STRD_C);
          S32LDI(xr2,src3,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr3,src3,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr4,src3,-TCSM1_DBLK_MBOUT_STRD_C);

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

	  S32LDI(xr1,src4,-TCSM1_DBLK_MBOUT_STRD_C);
          S32LDI(xr2,src4,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr3,src4,-TCSM1_DBLK_MBOUT_STRD_C);
	  S32LDI(xr4,src4,-TCSM1_DBLK_MBOUT_STRD_C);

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


#ifdef JZC_ROTA90_OPT
      if (mb_y_d2){
        //luma
	for(i=0; i<4; i++){
          src1 = dblk_upout_yaddr1 + 64 + i*4;      
	  dst1 = XCH2_T0->rota_upmb_ybuf + i*16 - 4; 
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
  
        //chroma
        for(i=0; i<2; i++){
          src1 = dblk_upout_caddr1 + 64 + i*4;      
	  dst1 = XCH2_T0->rota_upmb_ubuf + i*16 - 4; 
	  src2 = src1 + 8;      
	  dst2 = dst1 + 64; 

	  S32LDI (xr1,src1,-0x10);       //xr1: x15,x14,x13,x12
	  S32LDI (xr2,src1,-0x10);       //xr2: x11,x10,x9,x8
	  S32LDI (xr3,src1,-0x10);       //xr3: x7,x6,x5,x4
	  S32LDI (xr4,src1,-0x10);       //xr4: x3,x2,x1,x0
	  
          S32LDI (xr5,src2,-0x10);       //xr1: x15,x14,x13,x12
	  S32LDI (xr6,src2,-0x10);       //xr2: x11,x10,x9,x8
	  S32LDI (xr7,src2,-0x10);       //xr3: x7,x6,x5,x4
	  S32LDI (xr8,src2,-0x10);       //xr4: x3,x2,x1,x0

	  S32SFL(xr9,xr2,xr1,xr10,ptn2);  //xr9:x11,x15,x9,x13 xr10:x10,x14,x8,x12
	  S32SFL(xr11,xr4,xr3,xr12,ptn2); //xr11:x3,x7,x1,x5 xr12:x2,x6,x0,x4	
	  S32SFL(xr1,xr12,xr10,xr2,ptn3); //xr1:x2,x6,x10,x14 xr2:x0,x4,x8,x12	    
	  S32SFL(xr3,xr11,xr9,xr4,ptn3);  //xr3:x3,x7,x11,x15 xr4:x1,x5,x9,x13
	  
          S32SFL(xr9,xr6,xr5,xr10,ptn2);  //xr9:x11,x15,x9,x13 xr10:x10,x14,x8,x12
	  S32SFL(xr11,xr8,xr7,xr12,ptn2); //xr11:x3,x7,x1,x5 xr12:x2,x6,x0,x4	
	  S32SFL(xr5,xr12,xr10,xr6,ptn3); //xr5:x2,x6,x10,x14 xr6:x0,x4,x8,x12	    
	  S32SFL(xr7,xr11,xr9,xr8,ptn3);  //xr7:x3,x7,x11,x15 xr8:x1,x5,x9,x13

	  S32SDI(xr2,dst1,0x4);
	  S32STD(xr4,dst1,0x4);
	  S32STD(xr1,dst1,0x8);
	  S32STD(xr3,dst1,0xC);
	  
	  S32SDI(xr6,dst2,0x4);
	  S32STD(xr8,dst2,0x4);
	  S32STD(xr5,dst2,0x8);
	  S32STD(xr7,dst2,0xC);
	}
      }
#endif

      //poll_gp1_end();  
      if (mbnum > 2){
        uint8_t *src5, *src6, *dst5, *dst6;
        if (mb_y_d2){
	  up_ydst = Lpicture->y_ptr + (mb_y_d2 - 1) * y_mb_line + mb_x_d2 * 256;
	  up_udst = Lpicture->uv_ptr + (mb_y_d2 - 1) * uv_mb_line + mb_x_d2 * 128;
        }else{
          up_ydst = Lpicture->y_ptr - 2 * y_mb_line - 512;
	  up_udst = Lpicture->uv_ptr - 2 * uv_mb_line - 256;
        }
	ydst = Lpicture->y_ptr + mb_y_d3 * y_mb_line + mb_x_d3 * 256;
	udst = Lpicture->uv_ptr + mb_y_d3 * uv_mb_line + mb_x_d3 * 128;

	rota_mx = SLICE_T0->mb_height - 1 - mb_y_d3;
	rota_my = mb_x_d3;
	rota_ydst = Lpicture->rota_y_ptr + rota_my*rota_y_mb_line + rota_mx*256;
	rota_cdst = Lpicture->rota_uv_ptr + rota_my*rota_uv_mb_line + rota_mx * 128;

        if (mb_y_d2){
	  rota_up_mx = SLICE_T0->mb_height - mb_y_d2;
	  rota_up_my = mb_x_d2;
        }else{
          rota_up_mx = 0;
          rota_up_my = -1;
        }
	rota_up_ydst = Lpicture->rota_y_ptr + rota_up_my*rota_y_mb_line + rota_up_mx*256;
	rota_up_udst = Lpicture->rota_uv_ptr + rota_up_my*rota_uv_mb_line + rota_up_mx * 128;

        gp1_chain_ptr[0]=TCSM1_PADDR(dblk_upout_yaddr1);
	gp1_chain_ptr[1]=(up_ydst + 16*12);
	//gp1_chain_ptr[2]=GP_STRD(64,GP_FRM_NML,64);
	//gp1_chain_ptr[3]=GP_UNIT(GP_TAG_LK,64,64);

        gp1_chain_ptr[4]=TCSM1_PADDR(dblk_upout_caddr1);
	gp1_chain_ptr[5]=(up_udst + 16*4);
	//gp1_chain_ptr[6]=GP_STRD(64,GP_FRM_NML,64);
	//gp1_chain_ptr[7]=GP_UNIT(GP_TAG_LK,64,64);
	
	gp1_chain_ptr[8]=TCSM1_PADDR(dblk_mb_ydst3);
	gp1_chain_ptr[9]=(ydst);
        //gp1_chain_ptr[10]=GP_STRD(192,GP_FRM_NML,192);
	//gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,192,192);
	
	gp1_chain_ptr[12]=TCSM1_PADDR(dblk_mb_udst3);
	gp1_chain_ptr[13]=(udst);
        //gp1_chain_ptr[14]=GP_STRD(64,GP_FRM_NML,64);
	//gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,64,64);
	
	if ((mb_y_d3+1)==SLICE_T0->mb_height) {
	  gp1_chain_ptr[10]=GP_STRD(256,GP_FRM_NML,256);
	  gp1_chain_ptr[11]=GP_UNIT(GP_TAG_LK,256,256);
	  gp1_chain_ptr[14]=GP_STRD(128,GP_FRM_NML,128);
	  gp1_chain_ptr[15]=GP_UNIT(GP_TAG_LK,128,128);
	}

       	gp1_chain_ptr[16]=TCSM1_PADDR(rota_mb_ybuf3);
	gp1_chain_ptr[17]=(rota_ydst);
        //gp1_chain_ptr[18]=GP_STRD(256,GP_FRM_NML,256);
	//gp1_chain_ptr[19]=GP_UNIT(GP_TAG_LK,256,256);

        gp1_chain_ptr[20]=TCSM1_PADDR(rota_mb_cbuf3);
	gp1_chain_ptr[21]=(rota_cdst);
	//gp1_chain_ptr[22]=GP_STRD(128,GP_FRM_NML,128);
	//gp1_chain_ptr[23]=GP_UNIT(GP_TAG_LK,128,128);

	gp1_chain_ptr[24]=TCSM1_PADDR(XCH2_T0->rota_upmb_ybuf);
	gp1_chain_ptr[25]=(rota_up_ydst);
	//gp1_chain_ptr[26]=GP_STRD(4,GP_FRM_NML,16);
	//gp1_chain_ptr[27]=GP_UNIT(GP_TAG_LK,4,4*16);
	
	gp1_chain_ptr[28]=TCSM1_PADDR(XCH2_T0->rota_upmb_ubuf);
	gp1_chain_ptr[29]=(rota_up_udst);
        //gp1_chain_ptr[30]=GP_STRD(4,GP_FRM_NML,16);
	//gp1_chain_ptr[31]=GP_UNIT(GP_TAG_LK,4,4*8);
	
	gp1_chain_ptr[32]=TCSM1_PADDR(XCH2_T0->rota_upmb_vbuf);
	gp1_chain_ptr[33]=(rota_up_udst + 8);
	//gp1_chain_ptr[34]=GP_STRD(4,GP_FRM_NML,16);
	gp1_chain_ptr[35]=GP_UNIT(GP_TAG_UL,4,4*8);

        if (!mb_x_d3){  //left
	  for(i=0; i<12; i++){
	    edge_bak_ybuf1[i] = dblk_mb_ydst3[16*i];
          }
          for(i=0; i<4; i++){
            edge_bak_cbuf1[i] = dblk_mb_udst3[16*i]; //u
            edge_bak_cbuf1[4+i] = dblk_mb_udst3[16*i + 8];  //v
          }
        }
       
        if (mb_x_d3+1 == SLICE_T0->mb_width){  //right
	  for(i=0; i<12; i++){
	    edge_bak_ybuf1[12+i] = dblk_mb_ydst3[16*i + 15];
          }
          for(i=0; i<4; i++){
            edge_bak_cbuf1[8+i] = dblk_mb_udst3[16*i + 7];    //u
            edge_bak_cbuf1[12+i] = dblk_mb_udst3[16*i + 15];  //v
          }
        }

        if (!mb_y_d2 && mb_x_d3){ //top
          edge_type = 4;
          src1 = dblk_mb_ydst3;
          src2 = dblk_mb_udst3;
          edge_tb_ydst = Lpicture->y_ptr - 2*y_mb_line + mb_x_d3  * 256;
	  edge_tb_cdst = Lpicture->uv_ptr - 2*uv_mb_line + mb_x_d3  * 128;
        }

        if ((mb_y_d2+1==SLICE_T0->mb_height) && mb_x_d3 &&(mb_x_d3+2!=SLICE_T0->mb_width)){ //bottom
	  edge_type = 244; 
          src1 = dblk_mb_ydst3 + 240;
          src2 = dblk_mb_udst3 + 112;
          edge_tb_ydst = Lpicture->y_ptr + SLICE_T0->mb_height * y_mb_line + mb_x_d3  * 256;
	  edge_tb_cdst = Lpicture->uv_ptr + SLICE_T0->mb_height * uv_mb_line + mb_x_d3  * 128;
        }
  
	if (edge_type == 4 || edge_type == 244){  //top and bottom
	  gp1_chain_ptr[35]=GP_UNIT(GP_TAG_LK,4,4*8);
	  gp1_chain_ptr[36]=TCSM1_PADDR(edge_ybuf1);
	  gp1_chain_ptr[37]=(edge_tb_ydst);
	  gp1_chain_ptr[38]=GP_STRD(256,GP_FRM_NML,256);
	  gp1_chain_ptr[39]=GP_UNIT(GP_TAG_LK,256,256);
	  
	  gp1_chain_ptr[40]=TCSM1_PADDR(edge_ybuf1);
	  gp1_chain_ptr[41]=(edge_tb_ydst + y_mb_line);
	  gp1_chain_ptr[42]=GP_STRD(256,GP_FRM_NML,256);
	  gp1_chain_ptr[43]=GP_UNIT(GP_TAG_LK,256,256);

          gp1_chain_ptr[44]=TCSM1_PADDR(edge_cbuf1);
	  gp1_chain_ptr[45]=(edge_tb_cdst);
	  gp1_chain_ptr[46]=GP_STRD(128,GP_FRM_NML,128);
	  gp1_chain_ptr[47]=GP_UNIT(GP_TAG_LK,128,128);
	  
	  gp1_chain_ptr[48]=TCSM1_PADDR(edge_cbuf1);
	  gp1_chain_ptr[49]=(edge_tb_cdst + uv_mb_line);
	  gp1_chain_ptr[50]=GP_STRD(128,GP_FRM_NML,128);
	  gp1_chain_ptr[51]=GP_UNIT(GP_TAG_UL,128,128);

	  dst1 = edge_ybuf1 - 16;
	  dst2 = edge_cbuf1 - 16;
	  S32LDD(xr1,src1,0x0);
	  S32LDD(xr2,src1,0x4);
	  S32LDD(xr3,src1,0x8);
	  S32LDD(xr4,src1,0xC);
          S32LDD(xr5,src2,0x0);
	  S32LDD(xr6,src2,0x4);
	  S32LDD(xr7,src2,0x8);
	  S32LDD(xr8,src2,0xC);

	  poll_gp1_end();  //because we have only a edge_buf

	  for(i=0; i<16; i++){ //luma
	    S32SDI(xr1,dst1,0x10);
	    S32STD(xr2,dst1,0x4);
	    S32STD(xr3,dst1,0x8);
	    S32STD(xr4,dst1,0xC);
	  }
          for(i=0; i<8; i++){  //chroma
	    S32SDI(xr5,dst2,0x10);
	    S32STD(xr6,dst2,0x4);
	    S32STD(xr7,dst2,0x8);
	    S32STD(xr8,dst2,0xC);
	  }
        }

	if (mb_y_d2 && ((!mb_x_d2)|| (mb_x_d2+1 == SLICE_T0->mb_width))){
          if (!mb_x_d2){  //left
            src1 = edge_bak_ybuf1 - 1;
            src2 = dblk_upout_yaddr1 - 16;
            src3 = edge_bak_cbuf1 - 1;  // up u
            src4 = dblk_upout_caddr1 - 16; //bottom u
            src5 = src3 + 4;            //up v
            src6 = src4 + 8;            //bottom v
	    edge_ydst = Lpicture->y_ptr + (mb_y_d2-1) * y_mb_line - 2 * 256;
	    edge_cdst = Lpicture->uv_ptr + (mb_y_d2-1) * uv_mb_line - 2 * 128;
          }else{
            src1 = edge_bak_ybuf1 + 12 - 1;
            src2 = dblk_upout_yaddr1 - 16 + 15; 
            src3 = edge_bak_cbuf1 - 1 + 8;// up u
            src4 = dblk_upout_caddr1 - 16 + 7; //bottom u
            src5 = src3 + 4;  //up v
            src6 = src4 + 8;
            edge_ydst = Lpicture->y_ptr + (mb_y_d2-1) * y_mb_line + SLICE_T0->mb_width * 256;
	    edge_cdst = Lpicture->uv_ptr + (mb_y_d2-1) * uv_mb_line + SLICE_T0->mb_width * 128;
          }

	  gp1_chain_ptr[35]=GP_UNIT(GP_TAG_LK,4,4*8);
	  gp1_chain_ptr[36]=TCSM1_PADDR(edge_ybuf1);
	  gp1_chain_ptr[37]=(edge_ydst);
	  gp1_chain_ptr[38]=GP_STRD(256,GP_FRM_NML,256);
	  gp1_chain_ptr[39]=GP_UNIT(GP_TAG_LK,256,256);
	  
	  gp1_chain_ptr[40]=TCSM1_PADDR(edge_ybuf1);
	  gp1_chain_ptr[41]=(edge_ydst + 256);
	  gp1_chain_ptr[42]=GP_STRD(256,GP_FRM_NML,256);
	  gp1_chain_ptr[43]=GP_UNIT(GP_TAG_LK,256,256);

          gp1_chain_ptr[44]=TCSM1_PADDR(edge_cbuf1);
	  gp1_chain_ptr[45]=(edge_cdst);
	  gp1_chain_ptr[46]=GP_STRD(128,GP_FRM_NML,128);
	  gp1_chain_ptr[47]=GP_UNIT(GP_TAG_LK,128,128);
	  
	  gp1_chain_ptr[48]=TCSM1_PADDR(edge_cbuf1);
	  gp1_chain_ptr[49]=(edge_cdst + 128);
	  gp1_chain_ptr[50]=GP_STRD(128,GP_FRM_NML,128);
	  gp1_chain_ptr[51]=GP_UNIT(GP_TAG_UL,128,128);


	  dst1 = edge_ybuf1 - 16;
	  dst2 = edge_ybuf1 - 16 + 192;
	  dst3 = edge_cbuf1 - 16;
          dst4 = dst3 + 16*4;
          dst5 = dst3 + 8;
          dst6 = dst4 + 8;

	  poll_gp1_end();  //because we have only a edge_buf 

	  for(i=0; i<6; i++){ //16*12
	    S8LDI(xr1,src1,0x1,ptn7);  //xr1:x0,x0,x0,x0
	    S8LDI(xr2,src1,0x1,ptn7);  //xr2:x16,x16,x16,x16
	    S32SDI(xr1,dst1,0x10);
	    S32STD(xr1,dst1,0x4);
	    S32STD(xr1,dst1,0x8);
	    S32STD(xr1,dst1,0xC);
	    S32SDI(xr2,dst1,0x10);
	    S32STD(xr2,dst1,0x4);
	    S32STD(xr2,dst1,0x8);
	    S32STD(xr2,dst1,0xC);
	  }
	  for(i=0; i<2; i++){//16*4
	    S8LDI(xr1,src2,0x10,ptn7);  //xr1:x0,x0,x0,x0
	    S8LDI(xr2,src2,0x10,ptn7);  //xr2:x16,x16,x16,x16
	    S32SDI(xr1,dst2,0x10);
	    S32STD(xr1,dst2,0x4);
	    S32STD(xr1,dst2,0x8);
	    S32STD(xr1,dst2,0xC);
	    S32SDI(xr2,dst2,0x10);
	    S32STD(xr2,dst2,0x4);
	    S32STD(xr2,dst2,0x8);
	    S32STD(xr2,dst2,0xC);
	  }

          for(i=0; i<2; i++){    //up uv left:right
            S8LDI(xr1,src3,0x1,ptn7);
	    S8LDI(xr2,src3,0x1,ptn7);
            S8LDI(xr3,src5,0x1,ptn7);   
            S8LDI(xr4,src5,0x1,ptn7);
            S32SDI(xr1,dst3,0x10);
            S32STD(xr1,dst3,0x4);         
            S32SDI(xr2,dst3,0x10);
            S32STD(xr2,dst3,0x4);
            S32SDI(xr3,dst5,0x10);
            S32STD(xr3,dst5,0x4);
	    S32SDI(xr4,dst5,0x10);
	    S32STD(xr4,dst5,0x4);
          }
  
          for(i=0; i<2; i++){    //bottom  uv left:right
            S8LDI(xr1,src4,0x10,ptn7);
	    S8LDI(xr2,src4,0x10,ptn7);   
	    S8LDI(xr3,src6,0x10,ptn7);
	    S8LDI(xr4,src6,0x10,ptn7);   
            S32SDI(xr1,dst4,0x10);
            S32STD(xr1,dst4,0x4);         
            S32SDI(xr2,dst4,0x10);
            S32STD(xr2,dst4,0x4);
	    S32SDI(xr3,dst6,0x10);
            S32STD(xr3,dst6,0x4);
	    S32SDI(xr4,dst6,0x10);
            S32STD(xr4,dst6,0x4);
          }
	}

	poll_gp1_end();   
	set_gp1_dha(TCSM1_PADDR(gp1_chain_ptr));
 	set_gp1_dcs();
      }
    }
    /*---------------------------------------------------------------------
     * PTRs exchange
     *---------------------------------------------------------------------*/

    XCHG3(dmb_Z,dmb_A,dmb_B,xchg_tmp);
    XCHG3(recon_ptr0,recon_ptr1,recon_ptr2,xchg_tmp);
#ifdef JZC_ROTA90_OPT
    XCHG3(rota_mb_ybuf3,rota_mb_ybuf1,rota_mb_ybuf2,xchg_tmp);
    XCHG3(rota_mb_cbuf3,rota_mb_cbuf1,rota_mb_cbuf2,xchg_tmp);

    XCHG3(dblk_mb_ydst3,dblk_mb_ydst1,dblk_mb_ydst2,xchg_tmp);
    XCHG3(dblk_mb_udst3,dblk_mb_udst1,dblk_mb_udst2,xchg_tmp);
    XCHG2(dblk_upout_yaddr1,dblk_upout_yaddr2,xchg_tmp);
    XCHG2(dblk_upout_caddr1,dblk_upout_caddr2,xchg_tmp);
    XCHG2(gp1_chain_ptr, gp1_chain_ptr2,xchg_tmp);
#endif    
    XCHG2(XCH2_T0,XCH2_T1,xchg_tmp);
    mbnum++;

#ifdef JZC_PMON_P1_TOTAL
  PMON_OFF(total);
#endif
  }

#ifdef AHB1_ARB_PMON
      {
	arb_reg(AHB1, ARB_CTRL) = arb_reg(AHB1, ARB_CTRL) & 0xFFFFFFF0;
      }	
#endif

  poll_gp1_end();
#ifdef JZC_PMON_P1
  pmon_ptr[0]=total_pmon_val_useless_insn;
  pmon_ptr[1]=total_pmon_val_piperdy;
  pmon_ptr[2]=total_pmon_val_cclk;
#endif//#ifdef JZC_PMON_P1

  /*---------------------------------------------------------------------------
   * Task done
   ---------------------------------------------------------------------------*/

  *((volatile int *)(TCSM0_PADDR(TCSM0_P1_TASK_DONE))) = 0x1;

  i_nop;  
  i_nop;    
  i_nop;      
  i_nop;  
  i_wait();
}
