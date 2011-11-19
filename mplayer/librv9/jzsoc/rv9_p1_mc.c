/*******************************************************
 Motion Test Center
 ******************************************************/
#define __place_k0_data__
#undef printf
#undef fprintf
#include "t_motion.h"
//#include "t_intpid.h"
#include "rv9_tcsm1.h"
#include "rv9_dcore.h"

#define MPEG_HPEL  0
#define MPEG_QPEL  1
#define H264_QPEL  2
#define H264_EPEL  3
#define RV8_TPEL   4
#define RV9_QPEL   5
#define RV9_CPEL   6 
#define WMV2_QPEL  7
#define VC1_QPEL   8
#define AVS_QPEL   9
#define VP6_QPEL   10
#define VP8_QPEL   11

enum SPelSFT {
  HPEL = 1,
  QPEL,
  EPEL,
};

#define IS_EC     1
#define IS_ILUT0  0
#define IS_ILUT1  2

static char SubPel[] = {HPEL, QPEL, QPEL, EPEL, 
			QPEL, QPEL, QPEL, QPEL,
			QPEL, QPEL, QPEL, QPEL};

//volatile uint8_t *motion_buf, *motion_dha, *motion_dsa, *motion_douty, *motion_doutc;
//extern uint32_t current_picture_ptr[3];
static void rv40_mc_2mv_hw(RV9_Slice_GlbARGs *dSlice, RV9_MB_DecARGs *dMB,const int block_type,
				const int xoff, const int yoff, int mv_off, int *tdd, int *tkn, int blkh, int blkw, int boy, int box,
			   const int width, const int height, int dir0, int dir1)
{
  int mx, my;
  int is16x16 = 1;    
  int i;
  int cx, cy;
  int smvx, smvy;
  int dir[2];
  dir[0] = dir0;
  dir[1] = dir1;
  int is_bidir = dir0 && dir1;

  for(i=0; i<2; i++){
    if(dir[i]){
    smvx = dMB->motion_val[i][mv_off][0];
    smvy = dMB->motion_val[i][mv_off][1];
    mx = smvx >> 2;
    my = smvy >> 2;
    int mvx, mvy, full_mx, full_my;
    mvx = smvx; mvy = smvy;
    full_mx = dMB->mb_x*16 + (mx); full_my = dMB->mb_y*16 + (my);
    if(full_mx < -EDGE_WIDTH + 2){
      mvx += (-EDGE_WIDTH+2-full_mx)<<2;
    }
    if(full_mx + 16 > 16*dSlice->mb_width + EDGE_WIDTH - 3){
      mvx -= (full_mx+16-16*dSlice->mb_width-EDGE_WIDTH+3)<<2;
    }
    if(full_my < -EDGE_WIDTH + 2){
      mvy += (-EDGE_WIDTH+2-full_my)<<2;
    }
    if(full_my + 16 > 16*dSlice->mb_height + EDGE_WIDTH - 3){
      mvy -= (full_my+16-16*dSlice->mb_height-EDGE_WIDTH+3)<<2;
    }
    tdd[ 2*tkn[0]   ] = TDD_MV(mvy, mvx);
    tdd[ 2*tkn[0]+1 ] = TDD_CMD(is_bidir,/*bidir*/
				i,/*refdir*/
				0,/*fld*/
				0,/*fldsel*/
				0,/*rgr*/
				0,/*its*/
				0,/*doe*/
				0,/*cflo*/
				mvy & 0x7,/*ypos*/
				IS_ILUT0,/*lilmd*/
				IS_EC,/*cilmd*/
				0, /*list*/     
				boy,/*boy*/
				box,/*box*/
				blkh,/*bh*/
				blkw,/*bw*/
				mvx & 0x7/*xpos*/);
    (*tkn)++;
    }
  }
}

static int rv40_decode_mv_aux(RV9_Slice_GlbARGs *dSlice,RV9_MB_DecARGs *dMB, uint8_t *dout_ptr, uint8_t *motion_dha, uint8_t *motion_dsa)
{
  int i, j, k, l;
  int next_bt;
  int pbdir;    
  uint8_t *motion_douty, *motion_doutc;
  uint8_t *Y, *U, *V;
  int block_type = dMB->mbtype;
  motion_douty = dout_ptr;
  motion_doutc = dout_ptr+PREVIOUS_OFFSET_U;
  SET_REG1_DSTA(TCSM1_PADDR((int)motion_douty));
  SET_REG2_DSTA(TCSM1_PADDR((int)motion_doutc));
  SET_REG1_DSA(TCSM1_PADDR((int)motion_dsa));
  SET_REG2_DSA(TCSM1_PADDR((int)motion_dsa));  

  volatile int *tdd = (int *)motion_dha;
  int tkn = 0;
  motion_dsa[0] = 0x0;
  tdd++;	  	  

  switch(block_type){
  case RV34_MB_SKIP:
    if(dSlice->pict_type == FF_P_TYPE){
      rv40_mc_2mv_hw (dSlice, dMB, block_type, 0, 0, 0,tdd, &tkn, 3, 3, 0, 0, 2, 2, 1,0);
      mc_flag = 1;
      break;
    }
  case RV34_MB_B_DIRECT:
    //surprisingly, it uses motion scheme from next reference frame
    if(!(IS_16X8(dMB->next_bt) || IS_8X16(dMB->next_bt) || IS_8X8(dMB->next_bt))) 
      rv40_mc_2mv_hw(dSlice, dMB, block_type,0, 0, 0, tdd,&tkn,3,3,0,0,2,2,1,1);
    else
      for(i=0;i< 4;i++){	
	rv40_mc_2mv_hw(dSlice, dMB, RV34_MB_P_8x8, (i&1)<<3, (i&2)<<2, i,tdd,&tkn,2,2,(i&0x2),(i&0x1)*2, 1, 1, 1, 1);
      }
    mc_flag = 1;
    break;
  case RV34_MB_P_16x16:
  case RV34_MB_P_MIX16x16:
    rv40_mc_2mv_hw (dSlice, dMB,block_type, 0, 0, 0,tdd, &tkn, 3, 3, 0, 0, 2, 2, 1, 0);
    mc_flag = 1;
    break;
  case RV34_MB_B_FORWARD:
  case RV34_MB_B_BACKWARD:
    rv40_mc_2mv_hw (dSlice, dMB,block_type, 0, 0, 0,tdd, &tkn, 3, 3, 0, 0, 2, 2, block_type == RV34_MB_B_FORWARD,block_type == RV34_MB_B_BACKWARD);
    mc_flag = 1;
    break;
  case RV34_MB_P_16x8:
  case RV34_MB_P_8x16:
    if(block_type == RV34_MB_P_16x8){
      rv40_mc_2mv_hw(dSlice, dMB,block_type, 0, 0, 0, tdd, &tkn, 2, 3, 0, 0, 2, 1, 1, 0);
      rv40_mc_2mv_hw(dSlice, dMB,block_type, 0, 8, 2, tdd, &tkn, 2, 3, 2, 0, 2, 1, 1, 0);
    }
    if(block_type == RV34_MB_P_8x16){
      rv40_mc_2mv_hw(dSlice, dMB,block_type, 0, 0, 0, tdd, &tkn, 3, 2, 0, 0, 1, 2, 1, 0);
      rv40_mc_2mv_hw(dSlice, dMB,block_type, 8, 0, 1, tdd, &tkn, 3, 2, 0, 2, 1, 2, 1, 0);
    }
    mc_flag = 1;
    break;
  case RV34_MB_B_BIDIR:
    rv40_mc_2mv_hw(dSlice, dMB,block_type,0, 0, 0, tdd,&tkn,3,3,0,0,2,2,1,1);
     mc_flag = 1;
    break;
  case RV34_MB_P_8x8:
    for(i=0;i< 4;i++){	
      rv40_mc_2mv_hw(dSlice, dMB,block_type, (i&1)<<3, (i&2)<<2, i,tdd,&tkn,2,2,(i&0x2),(i&0x1)*2, 1, 1, 1, 0);
    }
    mc_flag = 1;
    break;
  }

  tdd[2*tkn-1] |= 0x1<<TDD_DOE_SFT;
  tdd[-1] = TDD_HEAD(1,/*vld*/
		     1,/*lk*/
		     0,/*sync*/
		     1,/*ch1pel*/
		     2,/*ch2pel*/ 
		     TDD_POS_SPEC,/*posmd*/
		     TDD_MV_AUTO,/*mvmd*/ 
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
			TDD_MV_AUTO,/*mvmd*/ 
			1,/*ch2en*/
			0,/*tkn*/
			0xFF,/*mby*/
			0xFF/*mbx*/);

}

