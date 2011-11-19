/*****************************************************************************
 *
 * JZC RV9 Decoder Architecture
 *
 ****************************************************************************/

#ifndef __JZC_RV9_ARCH_H__
#define __JZC_RV9_ARCH_H__

typedef struct RV9_Slice_Ptr{
  uint8_t *ptr[3];
}RV9_Slice_Ptr;

typedef struct RV9_Frame_LPtr{
  uint8_t * y_ptr;
  uint8_t * uv_ptr;
  uint8_t * rota_y_ptr;
  uint8_t * rota_uv_ptr;
}RV9_Frame_LPtr;

typedef struct RV9_Slice_GlbARGs{
  uint8_t pict_type;
  uint8_t si_type;
  uint8_t mb_width;
  uint8_t mb_height;

  uint8_t *last_data[3];
  uint8_t *next_data[3];
  uint8_t refqp; 
  uint16_t linesize;
  uint16_t uvlinesize;
  RV9_Slice_Ptr current_picture;
  RV9_Frame_LPtr line_current_picture;
}RV9_Slice_GlbARGs;

typedef struct RV9_MB_DecARGs{
  int cbp;
  int next_bt;
  uint32_t mb_type;

  int dcbp_above;
  int dcbp_left;
  int mvd;
  int mvd_above;
  int mvd_left;
  
  int8_t mbtype;
  int8_t mbtype_above;
  int8_t mbtype_left;
  uint8_t  is16;

  uint8_t qscale;
  uint8_t mb_x;
  uint8_t mb_y; 
  int8_t  er_slice;

  uint8_t itype4[20];
  uint8_t rup[20];

  int8_t new_slice;
  uint16_t yqtable;  
  int8_t dq_len[24];

  DCTELEM block16[16];
  uint16_t cqtable[2];
  short motion_val[2][4][2]; //
  DCTELEM block[6][64]; // 144+192=336 words
}RV9_MB_DecARGs;

#define SLICE_T_CC_LINE ((sizeof(struct RV9_Slice_GlbARGs)+31)/32)//=58

typedef struct RV9_XCH2_T{
  uint8_t * dblk_des_ptr;
  uint8_t * dblk_out_addr;
  uint8_t * dblk_out_addr_c;
  uint8_t * dblk_upout_addr_c;
}RV9_XCH2_T;
#endif
