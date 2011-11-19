/*******************************************************
 Motion Test Center
 ******************************************************/
#define __place_k0_data__
#undef printf
#undef fprintf
#include "t_motion.h"
//#include "t_intpid.h"
#include "t_vputlb.h"
#include "vc1_tcsm1.h"

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

volatile uint8_t *motion_buf, *motion_dha, *motion_dsa, *motion_douty, *motion_doutc, *motion_iwta;
volatile int total_tran, total_work;
extern VC1_MB_DecARGs *dMB;
extern VC1_Frame_GlbARGs *dFRM;
extern uint32_t current_picture_ptr[2];

static void vc1_mc_1mv_hw(int bidir,int dir,int *tdd, int *tkn)
{
    int dxy, mx, my, uvmx, uvmy;
    int mv[4], mvx, mvy, full_mx, full_my;
    int cmv_x, cmv_y;
    if((dir == 0 && !dFRM->last_data[0]))
    return;

    mx = dMB->vc1_mv[dir][0];
    my = dMB->vc1_mv[dir][1];

    uvmx = (mx + ((mx & 3) == 3)) >> 1;
    uvmy = (my + ((my & 3) == 3)) >> 1;
    if(dFRM->fastuvmc) {
        uvmx = uvmx + ((uvmx<0)?(uvmx&1):-(uvmx&1));
        uvmy = uvmy + ((uvmy<0)?(uvmy&1):-(uvmy&1));
   }
    cmv_x = uvmx;
    cmv_y = uvmy;
    
    if(dFRM->mspel)
      dxy = ((my & 3) << 2) | (mx & 3);
    else
      dxy = ((my & 2)<<2) | (mx & 2);
    mvx = mx; mvy = my;
    full_mx = dMB->mb_x*16 + (mx>>2); full_my = dMB->mb_y*16 + (my>>2);
    if(full_mx < -EDGE_WIDTH + dFRM->mspel){
      mvx += (-EDGE_WIDTH+dFRM->mspel-full_mx)<<2;
    }
    if(full_mx + 16 > 16*dFRM->mb_width + EDGE_WIDTH - 1 - dFRM->mspel){
      mvx -= (full_mx+16-16*dFRM->mb_width-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    if(full_my < -EDGE_WIDTH + dFRM->mspel){
      mvy += (-EDGE_WIDTH+dFRM->mspel-full_my)<<2;
    }
    if(full_my + 16 > 16*dFRM->mb_height + EDGE_WIDTH - 1 - dFRM->mspel){
      mvy -= (full_my+16-16*dFRM->mb_height-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    mv[0] = TDD_MV(mvy, mvx);

    full_mx = dMB->mb_x*8 + (cmv_x>>2); full_my = dMB->mb_y*8 + (cmv_y>>2);

    if(full_mx < -(EDGE_WIDTH/2)){
      cmv_x += (-(EDGE_WIDTH/2)-full_mx)<<2;
    }
    if(full_mx + 8 > 8*dFRM->mb_width + EDGE_WIDTH/2 - 1){
      cmv_x -= (full_mx+8-8*dFRM->mb_width-EDGE_WIDTH/2+1)<<2;
    }
    if(full_my < -(EDGE_WIDTH/2)){
      cmv_y += (-(EDGE_WIDTH/2)-full_my)<<2;
    }
    if(full_my + 8 > 8*dFRM->mb_height + EDGE_WIDTH/2 - 1){
      cmv_y -= (full_my+8-8*dFRM->mb_height-EDGE_WIDTH/2+1)<<2;
    }
    //motion_execute_vc1(bidir, dir, 0, &mv[0], &dxy, cmv_x, cmv_y);     

    //SET_REG1_BINFO(0,0,0,0,dFRM->mspel? (IS_ILUT0): (IS_ILUT1),0,0,0,0,0,0,0,0);

    tdd[2*tkn[0]]= TDD_MV(mv[0]>>16, mv[0]);

    tdd[2*tkn[0]+1]= TDD_CMD(bidir,/*bidir*/
		     dir,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     dFRM->rangeredfrm,/*rgr*/
		     (dMB->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     1,/*cflo*/
		     0,/*ypos*/
		     dFRM->mspel? (IS_ILUT0): (IS_ILUT1 | 1),/*lilmd*/		    
		     0,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     dxy/*xpos*/
		     );
    (*tkn)++;
    tdd[2*tkn[0]] = TDD_MV(cmv_y, cmv_x);            

	    tdd[2*tkn[0]+1] = TDD_CMD(bidir,/*bidir*/
		     dir,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     dFRM->rangeredfrm,/*rgr*/
		     (dMB->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     0,/*cflo*/
		     (cmv_y&3)<<1,/*ypos*/
		     0,/*lilmd*/
		     IS_EC,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     (cmv_x&3)<<1/*xpos*/);

    (*tkn)++; 

}


static void vc1_mc_4mv_luma_hw(int n,int *tdd, int *tkn)
{
    int dxy, mx, my, src_x, src_y;
    int off;

    if(!dFRM->last_data[0]) return;
    mx = dMB->vc1_mv[n][0];
    my = dMB->vc1_mv[n][1];

    int mv[4], mvx, mvy, full_mx, full_my;

    if(dFRM->mspel)
      dxy = ((my & 3) << 2) | (mx & 3);
    else
      dxy = ((my & 2)<<2) | (mx & 2);

    mvx = mx; mvy = my;
  
    full_mx = dMB->mb_x*16 + (mx>>2); full_my = dMB->mb_y*16 + (my>>2);

    if(full_mx < -EDGE_WIDTH + dFRM->mspel){
      mvx += (-EDGE_WIDTH+dFRM->mspel-full_mx)<<2;
    }
    if(full_mx + 16 > 16*dFRM->mb_width + EDGE_WIDTH - 1 - dFRM->mspel){
      mvx -= (full_mx+16-16*dFRM->mb_width-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    if(full_my < -EDGE_WIDTH + dFRM->mspel){
      mvy += (-EDGE_WIDTH+dFRM->mspel-full_my)<<2;
    }
    if(full_my + 16 > 16*dFRM->mb_height + EDGE_WIDTH - 1 - dFRM->mspel){
      mvy -= (full_my+16-16*dFRM->mb_height-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    mv[0] = TDD_MV(mvy, mvx);

    //SET_REG1_BINFO(0,0,0,0,dFRM->mspel? (IS_ILUT0): (IS_ILUT1 | 1),0,0,0,0,0,0,0,0);

    tdd[2*tkn[0]] = TDD_MV(mv[0]>>16, mv[0]);    
    tdd[2*tkn[0]+1] = TDD_CMD(0,/*bidir*/
		     0,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     dFRM->rangeredfrm,/*rgr*/
		     (dMB->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     0,/*cflo*/
		     0, /*mvy & 0x7,*//*ypos*/
		     dFRM->mspel? (IS_ILUT0): (IS_ILUT1 | 1),/*lilmd*/
		     0,/*cilmd*/
		     0,/*list*/
		     (n & 0x2),/*boy*/
		     (n & 0x1)*2,/*box*/
		     BLK_H8,/*bh*/
		     BLK_W8,/*bw*/
		     dxy/*xpos*/);    
    (*tkn)++;
}

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

/** Do motion compensation for 4-MV macroblock - both chroma blocks
 */
static void vc1_mc_4mv_chroma_hw(int *tdd, int *tkn)
{
    int uvmx, uvmy, uvsrc_x, uvsrc_y;
    int i, idx, tx = 0, ty = 0;
    int mvx[4], mvy[4], intra[4];

    if(!dFRM->last_data[0])return;
    if(dFRM->flags & CODEC_FLAG_GRAY) return;
    if (dMB->chroma_ret) return;

    uvmx = (dMB->tx + ((dMB->tx&3) == 3)) >> 1;
    uvmy = (dMB->ty + ((dMB->ty&3) == 3)) >> 1;

    if(dFRM->fastuvmc) {
      uvmx = uvmx + ((uvmx<0)?(uvmx&1):-(uvmx&1));
      uvmy = uvmy + ((uvmy<0)?(uvmy&1):-(uvmy&1));
    }

    /* Chroma MC always uses qpel bilinear */
    int cmv_x = uvmx;
    int cmv_y = uvmy;

    int mv[4], full_mx, full_my;
    full_mx = dMB->mb_x*8 + (cmv_x>>2); full_my = dMB->mb_y*8 + (cmv_y>>2);

    if(full_mx < -(EDGE_WIDTH/2)){
      cmv_x += (-(EDGE_WIDTH/2)-full_mx)<<2;
    }
    if(full_mx + 8 > 8*dFRM->mb_width + EDGE_WIDTH/2 - 1){
      cmv_x -= (full_mx+8-8*dFRM->mb_width-EDGE_WIDTH/2+1)<<2;
    }
    if(full_my < -(EDGE_WIDTH/2)){
      cmv_y += (-(EDGE_WIDTH/2)-full_my)<<2;
    }
    if(full_my + 8 > 8*dFRM->mb_height + EDGE_WIDTH/2 - 1){
      cmv_y -= (full_my+8-8*dFRM->mb_height-EDGE_WIDTH/2+1)<<2;
    }

    {
      tdd[2*tkn[0]-1] = tdd[2*tkn[0]-1] | (1 << 24);
      tdd[2*tkn[0]]   = TDD_MV(cmv_y, cmv_x);
      tdd[2*tkn[0]+1] = TDD_CMD(0,/*bidir*/
				0,/*refdir*/
				0,/*fld*/
				0,/*fldsel*/
				dFRM->rangeredfrm,/*rgr*/
				(dMB->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
				1,/*doe*/
				0,/*cflo*/
				(cmv_y&3)<<1,/*ypos*/
				0,/*lilmd*/
				IS_EC,/*cilmd*/
				0,/*list*/
				0,/*boy*/
				0,/*box*/
				BLK_H16,/*bh*/
				BLK_W16,/*bw*/
				(cmv_x&3)<<1/*xpos*/);
      
    }    
    (*tkn)++;
}

static void vc1_mc_1mv_hw_b(int bidir,int dir,int *tdd, int *tkn)
{
    int dxy, mx, my, uvmx, uvmy;
    int mv[4], mvx, mvy, full_mx, full_my;
    int cmv_x, cmv_y;
    int cmv_x0, cmv_y0;

    if(bidir==1)
      if((!dFRM->last_data[0]) && (!dFRM->next_data[0]))
	return;
    mx = dMB->vc1_mv[0][0];
    my = dMB->vc1_mv[0][1];

    uvmx = (mx + ((mx & 3) == 3)) >> 1;
    uvmy = (my + ((my & 3) == 3)) >> 1;
    if(dFRM->fastuvmc) {
        uvmx = uvmx + ((uvmx<0)?(uvmx&1):-(uvmx&1));
        uvmy = uvmy + ((uvmy<0)?(uvmy&1):-(uvmy&1));
   }
    cmv_x = uvmx;
    cmv_y = uvmy;
    
    if(dFRM->mspel)
      dxy = ((my & 3) << 2) | (mx & 3);
    else
      dxy = ((my & 2)<<2) | (mx & 2);
    mvx = mx; mvy = my;
    full_mx = dMB->mb_x*16 + (mx>>2); full_my = dMB->mb_y*16 + (my>>2);
    if(full_mx < -EDGE_WIDTH + dFRM->mspel){
      mvx += (-EDGE_WIDTH+dFRM->mspel-full_mx)<<2;
    }
    if(full_mx + 16 > 16*dFRM->mb_width + EDGE_WIDTH - 1 - dFRM->mspel){
      mvx -= (full_mx+16-16*dFRM->mb_width-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    if(full_my < -EDGE_WIDTH + dFRM->mspel){
      mvy += (-EDGE_WIDTH+dFRM->mspel-full_my)<<2;
    }
    if(full_my + 16 > 16*dFRM->mb_height + EDGE_WIDTH - 1 - dFRM->mspel){
      mvy -= (full_my+16-16*dFRM->mb_height-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    mv[0] = TDD_MV(mvy, mvx);

    full_mx = dMB->mb_x*8 + (cmv_x>>2); full_my = dMB->mb_y*8 + (cmv_y>>2);

    if(full_mx < -(EDGE_WIDTH/2)){
      cmv_x += (-(EDGE_WIDTH/2)-full_mx)<<2;
    }
    if(full_mx + 8 > 8*dFRM->mb_width + EDGE_WIDTH/2 - 1){
      cmv_x -= (full_mx+8-8*dFRM->mb_width-EDGE_WIDTH/2+1)<<2;
    }
    if(full_my < -(EDGE_WIDTH/2)){
      cmv_y += (-(EDGE_WIDTH/2)-full_my)<<2;
    }
    if(full_my + 8 > 8*dFRM->mb_height + EDGE_WIDTH/2 - 1){
      cmv_y -= (full_my+8-8*dFRM->mb_height-EDGE_WIDTH/2+1)<<2;
    }
    cmv_y0 = cmv_y;
    cmv_x0 = cmv_x;
    //motion_execute_vc1(bidir, dir, 0, &mv[0], &dxy, cmv_x, cmv_y);     

    //SET_REG1_BINFO(0,0,0,0,dFRM->mspel? (IS_ILUT0): (IS_ILUT1),0,0,0,0,0,0,0,0);

    tdd[2*tkn[0]]= TDD_MV(mv[0]>>16, mv[0]);

    tdd[2*tkn[0]+1]= TDD_CMD( 1,/*bidir*/
		     0,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     dFRM->rangeredfrm,/*rgr*/
		     (dMB->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     0,/*doe*/
		     0,/*cflo*/
		     0,/*ypos*/
		     dFRM->mspel? (IS_ILUT0): (IS_ILUT1 | 1),/*lilmd*/		    
		     0,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     dxy/*xpos*/
		     );
    (*tkn)++;

    mx = dMB->vc1_mv[1][0];
    my = dMB->vc1_mv[1][1];

    uvmx = (mx + ((mx & 3) == 3)) >> 1;
    uvmy = (my + ((my & 3) == 3)) >> 1;
    if(dFRM->fastuvmc) {
        uvmx = uvmx + ((uvmx<0)?-(uvmx&1):(uvmx&1));
        uvmy = uvmy + ((uvmy<0)?-(uvmy&1):(uvmy&1));
    }

    cmv_x = uvmx;
    cmv_y = uvmy;
    
    if(dFRM->mspel)
      dxy = ((my & 3) << 2) | (mx & 3);
    else
      dxy = ((my & 2)<<2) | (mx & 2);
    mvx = mx; mvy = my;
    full_mx = dMB->mb_x*16 + (mx>>2); full_my = dMB->mb_y*16 + (my>>2);
    if(full_mx < -EDGE_WIDTH + dFRM->mspel){
      mvx += (-EDGE_WIDTH+dFRM->mspel-full_mx)<<2;
    }
    if(full_mx + 16 > 16*dFRM->mb_width + EDGE_WIDTH - 1 - dFRM->mspel){
      mvx -= (full_mx+16-16*dFRM->mb_width-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    if(full_my < -EDGE_WIDTH + dFRM->mspel){
      mvy += (-EDGE_WIDTH+dFRM->mspel-full_my)<<2;
    }
    if(full_my + 16 > 16*dFRM->mb_height + EDGE_WIDTH - 1 - dFRM->mspel){
      mvy -= (full_my+16-16*dFRM->mb_height-EDGE_WIDTH+1+dFRM->mspel)<<2;
    }
    mv[0] = TDD_MV(mvy, mvx);

    full_mx = dMB->mb_x*8 + (cmv_x>>2); full_my = dMB->mb_y*8 + (cmv_y>>2);

    if(full_mx < -(EDGE_WIDTH/2)){
      cmv_x += (-(EDGE_WIDTH/2)-full_mx)<<2;
    }
    if(full_mx + 8 > 8*dFRM->mb_width + EDGE_WIDTH/2 - 1){
      cmv_x -= (full_mx+8-8*dFRM->mb_width-EDGE_WIDTH/2+1)<<2;
    }
    if(full_my < -(EDGE_WIDTH/2)){
      cmv_y += (-(EDGE_WIDTH/2)-full_my)<<2;
    }
    if(full_my + 8 > 8*dFRM->mb_height + EDGE_WIDTH/2 - 1){
      cmv_y -= (full_my+8-8*dFRM->mb_height-EDGE_WIDTH/2+1)<<2;
    }
    //motion_execute_vc1(bidir, dir, 0, &mv[0], &dxy, cmv_x, cmv_y);     

    //SET_REG1_BINFO(0,0,0,0,dFRM->mspel? (IS_ILUT0): (IS_ILUT1),0,0,0,0,0,0,0,0);

    tdd[2*tkn[0]]= TDD_MV(mv[0]>>16, mv[0]);

    tdd[2*tkn[0]+1]= TDD_CMD(1,/*bidir*/
		     1,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     dFRM->rangeredfrm,/*rgr*/
		     (dMB->mv_mode == MV_PMODE_INTENSITY_COMP),/*its*/
		     1,/*doe*/
		     1,/*cflo*/
		     0,/*ypos*/
		     dFRM->mspel? (IS_ILUT0): (IS_ILUT1 | 1),/*lilmd*/		    
		     0,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     dxy/*xpos*/
		     );
    (*tkn)++;

    tdd[2*tkn[0]] = TDD_MV(cmv_y0, cmv_x0);            

    tdd[2*tkn[0]+1] = TDD_CMD(1,/*bidir*/
		     0,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     dFRM->rangeredfrm,/*rgr*/
		     0,/*its*/
		     0,/*doe*/
		     0,/*cflo*/
		     (cmv_y0&3)<<1,/*ypos*/
		     0,/*lilmd*/
		     IS_EC,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     (cmv_x0&3)<<1/*xpos*/);
    (*tkn)++;


    tdd[2*tkn[0]] = TDD_MV(cmv_y, cmv_x);            

    tdd[2*tkn[0]+1] = TDD_CMD(1,/*bidir*/
		     1,/*refdir*/
		     0,/*fld*/
		     0,/*fldsel*/
		     dFRM->rangeredfrm,/*rgr*/
		     0,/*its*/
		     1,/*doe*/
		     0,/*cflo*/
		     (cmv_y&3)<<1,/*ypos*/
		     0,/*lilmd*/
		     IS_EC,/*cilmd*/
		     0,/*list*/
		     0,/*boy*/
		     0,/*box*/
		     BLK_H16,/*bh*/
		     BLK_W16,/*bw*/
		     (cmv_x&3)<<1/*xpos*/);

    (*tkn)++;

}
