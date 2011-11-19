/*****************************************************************************
 *
 * JZ4760 Deblock Filter Accelerate
 *
 * $Id: h264_p1_dblk.c,v 1.8 2011/03/24 02:36:26 xqliang Exp $
 *
 ****************************************************************************/

static av_always_inline int hw_nnz_cache_get(H264_MB_Ctrl_DecARGs *dmb, char *sw_nnz_addr) {
  int nnz_reg;

  if (dmb->cbp || IS_INTRA16x16(dmb->mb_type)) {

    int tmp_addr = sw_nnz_addr;
    int tmp_val;
    S32I2M(xr5, 0x01010101);
    S32LDD(xr1, tmp_addr, 0);
    S32LDI(xr2, tmp_addr, 4);
    S32LDI(xr3, tmp_addr, 4);
    S32LDI(xr4, tmp_addr, 4);

    Q8MOVN(xr1, xr1, xr5);
    Q8MOVN(xr2, xr2, xr5);
    Q8MOVN(xr3, xr3, xr5);
    Q8MOVN(xr4, xr4, xr5);

    S32SFL(xr2, xr2, xr1, xr1, ptn0);
    S32SFL(xr4, xr4, xr3, xr3, ptn0);
    S32SFL(xr3, xr3, xr1, xr1, ptn3);
    S32SFL(xr4, xr4, xr2, xr2, ptn3);

    D32SLL(xr4, xr4, xr3, xr3, 1);
    S32OR(xr1, xr1, xr3);
    S32OR(xr2, xr2, xr4);
    D32SLL(xr2, xr2, xr0, xr10, 2);//xr10 used for s8ldd(xr10,sw_nnz_addr,27,0)
    S32OR(xr1, xr1, xr2);

    //S8LDD(xr10,sw_nnz_addr,27,0);
    S8LDD(xr10,&dmb->nnz_tl_8bits,0,0);

    D32SLR(xr2, xr1, xr0, xr0, 4);
    S32OR(xr1, xr1, xr2);

    S32SFL(xr0, xr10, xr1, xr1, ptn1);
    return S32M2I(xr1);
  }else{
    //nnz_reg = ( ((int)sw_nnz_addr[0]) & 0x000000FF ) << 16;
    //return nnz_reg;
    return (( (dmb->nnz_tl_8bits) & 0x000000FF ) << 16);
  }
}

static inline void filter_mb_hw(H264_Slice_GlbARGs *SLICE_T, H264_XCH2_T *XCH2_T0, H264_MB_Ctrl_DecARGs *dmb, uint8_t *in_addr_yuv,
		  uint8_t *dblk_upout_addr) {
  // ==================================================================
  //     Gather Variable 
  // ==================================================================
  //reg 0x10
  //STOP_P1_();
  char *sw_nnz_addr = (char *)((unsigned int)dmb + dmb->ctrl_len);
  int nnz = hw_nnz_cache_get(dmb,sw_nnz_addr);


  int mb_curr_type = dmb->mb_type;
  int mb_left_type = dmb->left_mb_type;
  int mb_up_type   = dmb->top_mb_type;
  int mv_msk = 0;
  if (mb_curr_type & MB_TYPE_16x16) {
    mv_msk = 0xee;
  } else if (mb_curr_type & MB_TYPE_16x8) {
    mv_msk = 0xae;
  } else if (mb_curr_type & MB_TYPE_8x16) {
    mv_msk = 0xea;
  }

  int v0_smth = ( ( (mb_curr_type & MB_TYPE_16x16) | (mb_curr_type & MB_TYPE_8x16) ) &
		  (mb_left_type & (MB_TYPE_16x16 | MB_TYPE_8x16)) ) ? 0x1 : 0;
  int h0_smth = ( (mb_curr_type & MB_TYPE_16x16) &
		  ( (mb_up_type & (MB_TYPE_16x16 | MB_TYPE_16x8)) |
		    (mb_up_type & (MB_TYPE_16x16 | MB_TYPE_16x8)) ) ) ? 0x10 : 0;
  int v2_smth = ( mb_curr_type & MB_TYPE_8x16 ) ? 0x4 : 0;
  int h2_smth = ( mb_curr_type & MB_TYPE_16x8 ) ? 0x40 : 0;
  int mv_smth = (v0_smth | v2_smth | h0_smth | h2_smth);

  int edg_msk;
  if (!SLICE_T->deblocking_filter) {
    edg_msk = 0xFFFF;
  } else {
    if ( (mb_curr_type & (MB_TYPE_16x16|MB_TYPE_SKIP)) == (MB_TYPE_16x16|MB_TYPE_SKIP) )
      edg_msk = 0xEE;
    else if ( IS_8x8DCT(mb_curr_type) )
      edg_msk = 0xAA;
    else
      edg_msk = 0;
    edg_msk |= dmb->dblk_start;
  }


  // ==================================================================
  //     Produce Descriptor Node
  // ==================================================================
  char *DBLK_Y_IN  = (char *)(TCSM1_PADDR(in_addr_yuv));
  char *DBLK_U_IN  = (char *)(TCSM1_PADDR(in_addr_yuv + PREVIOUS_OFFSET_U));
  char *DBLK_V_IN  = (char *)(TCSM1_PADDR(in_addr_yuv + PREVIOUS_OFFSET_V));

  unsigned int * node = (unsigned int *)XCH2_T0->dblk_des_ptr;

  char *DBLK_Y_UPOUT  = (char *)TCSM1_PADDR(dblk_upout_addr);
  char *DBLK_U_UPOUT  = (char *)TCSM1_PADDR(dblk_upout_addr + 128);
#ifdef JZC_DBLKLI_OPT
  char *DBLK_V_UPOUT  = (char *)TCSM1_PADDR(dblk_upout_addr + 136);
#else
  char *DBLK_V_UPOUT  = (char *)TCSM1_PADDR(dblk_upout_addr + 128 + TCSM1_DBLK_UPOUT_UV_OFFSET);
#endif  

  node[2] = (unsigned int)DBLK_V_UPOUT;
  node[3] = (unsigned int)DBLK_U_UPOUT;
  node[4] = (unsigned int)DBLK_Y_UPOUT;

  node[5] = (unsigned int)DBLK_V_IN;
  node[6] = (unsigned int)DBLK_U_IN;
  node[7] = (unsigned int)DBLK_Y_IN;

  node[9] = TCSM1_PADDR(XCH2_T0->dblk_mv_ptr);
  node[10] = DBLK_REG10(mv_msk, nnz);

  //reg 0x0C,0x08
  int qp_y_curr = dmb->qscale & 0x3f;
  int qp_y_left = ((dmb->qscale + dmb->qp_left + 1) >> 1) & 0x3f;
  int qp_y_up   = ((dmb->qscale + dmb->qp_top  + 1) >> 1) & 0x3f;;
  int qp_u_curr = dmb->chroma_qp[0] & 0x3f;
  int qp_u_left = ((dmb->chroma_qp[0] + dmb->chroma_qp_left[0] + 1) >> 1) & 0x3f;
  int qp_u_up   = ((dmb->chroma_qp[0] + dmb->chroma_qp_top[0]  + 1) >> 1) & 0x3f;
  int qp_v_curr = dmb->chroma_qp[1] & 0x3f;
  int qp_v_left = ((dmb->chroma_qp[1] + dmb->chroma_qp_left[1] + 1) >> 1) & 0x3f;
  int qp_v_up   = ((dmb->chroma_qp[1] + dmb->chroma_qp_top[1]  + 1) >> 1) & 0x3f;

  node[11] = DBLK_REG0C(qp_v_left,qp_v_curr,qp_v_up,qp_u_left,qp_u_curr);

  int mv_reduce_len;
  if(IS_INTRA(mb_curr_type)){
    mv_reduce_len = 0;
  }else if(SLICE_T->slice_type == B_TYPE){
    H264_MB_InterB_DecARGs * dmbB = (uint32_t)dmb + ((sizeof(H264_MB_Ctrl_DecARGs)+3) & 0xFFFFFFFC);
    mv_reduce_len = (dmbB->mv_num << 1) + 21;
  }else{
    H264_MB_InterB_DecARGs * dmbP = (uint32_t)dmb + ((sizeof(H264_MB_Ctrl_DecARGs)+3) & 0xFFFFFFFC);
    mv_reduce_len = dmbP->mv_num + 11;
  }

  node[12] = DBLK_REG08(mv_reduce_len,qp_u_up,qp_y_left,qp_y_curr,qp_y_up);

  //reg 0x04
  int intra_curr = (IS_INTRA(mb_curr_type)) != 0;
  int intra_left = (IS_INTRA(mb_left_type)) != 0;
  int intra_up   = (IS_INTRA(mb_up_type)  ) != 0;
  /* mv_smooth,edg_msk,row_end,yuv_flag,dma_cfg_bs,mv_reduce,
     intra_left,intra_curr,intra_up,hw_bs_h264,video_fmt */
  node[13] = DBLK_REG04(mv_smth,edg_msk,0,0x7,0,1,intra_left,intra_curr,intra_up,1,1);
}

