#define __place_k0_data__

#include "t_motion.h"
#include "t_intpid.h"
#include "h264_tcsm0.h"

#define DOUT_Y_STRD 20
#define DOUT_C_STRD 12

#define H264_QPEL 2
#define H264_EPEL 3

void h264_motion_init(int intpid, int cintpid)
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

void h264_motion_config(H264Context *h)
{
  int i, j;
  MpegEncContext * const s = &h->s;

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


#if 0
  for(i=0; i<16; i++){ 
    SET_TAB1_RLUT(i,get_phy_addr((int)(h->ref_list[0][i].data[0])),  
 		  h->luma_weight[i][0][0], h->luma_weight[i][0][1]); 
    SET_TAB1_RLUT(16+i, get_phy_addr((int)(h->ref_list[1][i].data[0])), 
 		  h->luma_weight[i][1][0], h->luma_weight[i][1][1]); 
    SET_TAB2_RLUT(i,get_phy_addr((int)(h->ref_list[0][i].data[1])),  
 		  h->chroma_weight[i][0][1][0], h->chroma_weight[i][0][1][1], 
 		  h->chroma_weight[i][0][0][0], h->chroma_weight[i][0][0][1]); 
    SET_TAB2_RLUT(16+i, get_phy_addr((int)(h->ref_list[1][i].data[1])),  
 		  h->chroma_weight[i][1][1][0], h->chroma_weight[i][1][1][1], 
 		  h->chroma_weight[i][1][0][0], h->chroma_weight[i][1][0][1]); 
  } 
#else
  for(i=0; i<16; i++){ 
    SET_TAB1_RLUT_ROA(i,get_phy_addr((int)(h->ref_list[0][i].data[0]))); 
    SET_TAB1_RLUT_ROA(16+i, get_phy_addr((int)(h->ref_list[1][i].data[0]))); 
    SET_TAB2_RLUT_ROA(i,get_phy_addr((int)(h->ref_list[0][i].data[1]))); 
    SET_TAB2_RLUT_ROA(16+i, get_phy_addr((int)(h->ref_list[1][i].data[1]))); 
  }

  for(i=0; i<16; i++){ 
    SET_TAB1_RLUT_WCOEF(i, h->luma_weight[i][0][0], h->luma_weight[i][0][1]); 
    SET_TAB1_RLUT_WCOEF(16+i, h->luma_weight[i][1][0], h->luma_weight[i][1][1]); 
    SET_TAB2_RLUT_WCOEF(i,h->chroma_weight[i][0][1][0], h->chroma_weight[i][0][1][1], 
 		  h->chroma_weight[i][0][0][0], h->chroma_weight[i][0][0][1]); 
    SET_TAB2_RLUT_WCOEF(16+i, h->chroma_weight[i][1][1][0], h->chroma_weight[i][1][1][1], 
 		  h->chroma_weight[i][1][0][0], h->chroma_weight[i][1][0][1]); 
  }
#endif

  for(j=0; j<16; j++)
    for(i=0; i<16; i++)
      iwta_buf0[j*16+i] = h->implicit_weight[j][i][0];

  SET_REG1_PINFO(0,/*rgr*/
		 0,/*its*/
		 0,/*its_sft*/
		 0,/*its_scale*/
		 0/*its_rnd*/);
  SET_REG2_PINFO(0,/*rgr*/
		 0,/*its*/
		 0,/*its_sft*/
		 0,/*its_scale*/
		 0/*its_rnd*/);

  SET_REG1_WINFO(0,/*wt*/
		 (h->use_weight == IS_WT1), /*wtpd*/
		 h->use_weight,/*wtmd*/
		 1,/*biavg_rnd*/
		 h->luma_log2_weight_denom,/*wt_denom*/
		 5,/*wt_sft*/
		 0,/*wt_lcoef*/
		 0/*wt_rcoef*/);
  SET_REG1_WTRND(1<<5);

  SET_REG2_WINFO1(0,/*wt*/
		  h->use_weight_chroma && (h->use_weight == IS_WT1), /*wtpd*/
		  h->use_weight,/*wtmd*/
		  1,/*biavg_rnd*/
		  h->chroma_log2_weight_denom,/*wt_denom*/
		  5,/*wt_sft*/
		  0,/*wt_lcoef*/
		  0/*wt_rcoef*/);
  SET_REG2_WINFO2(5,/*wt_sft*/
		  0,/*wt_lcoef*/
		  0/*wt_rcoef*/);
  SET_REG2_WTRND(1<<5, 1<<5);
  SET_REG1_IWTA(TCSM0_PADDR((int)iwta_buf0));
  SET_REG1_STRD(s->linesize/16,0,DOUT_Y_STRD);
  SET_REG1_GEOM(s->mb_height*16,s->mb_width*16);
  SET_REG2_STRD(s->linesize/16,0,DOUT_C_STRD);
}


