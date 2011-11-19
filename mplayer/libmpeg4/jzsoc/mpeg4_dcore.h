/*****************************************************************************
 *
 * JZC VP6 Decoder Architecture
 *
 ****************************************************************************/

#ifndef __JZC_MPEG4_ARCH_H__
#define __JZC_MPEG4_ARCH_H__

#ifdef __MINIOS__
#include <mplaylib.h>
#else
#include <stdio.h>
#endif

#ifdef JZC_P1_OPT
typedef struct VP6_Frame_Ptr{  
  int key_frame;    
  uint8_t *ptr[3];
}MPEG4_Frame_Ptr;

typedef struct _MPEG4_Frame_GlbARGs{
  uint32_t flags;
  uint8_t intra_only;
  int8_t pict_type;
  uint8_t mb_decision;
  uint8_t lowres;
  uint32_t draw_horiz_band;
  int8_t no_rounding;
  uint8_t quarter_sample;
  int8_t out_format;
  int8_t codec_id;
  int8_t chroma_x_shift,chroma_y_shift;
  uint16_t width, height;
  uint8_t mb_width, mb_height;
  uint16_t linesize;
  uint16_t uvlinesize;
  uint8_t mspel;
  int8_t h263_aic;
  int8_t hurry_up;
  int8_t h263_msmpeg4;
  int32_t workaround_bugs;
  uint16_t intra_matrix[64];
  //uint8_t permutated[64];
  uint8_t raster_end[64];
  uint16_t inter_matrix[64];
  uint8_t *last_picture_data[4];
  uint8_t *next_picture_data[4];
  uint8_t *b_scratchpad;
  uint8_t *current_picture_data[2];
  uint8_t *rota_picture_data[2];
  uint32_t mpFrame;
  int8_t alternate_scan;
  int8_t mpeg_quant;
  uint8_t edge;
}MPEG4_Frame_GlbARGs;

#define FRAME_T_CC_LINE ((sizeof(MPEG4_Frame_GlbARGs)+31)/32)

typedef struct _MPEG4_MB_DecARGs{
  int8_t real_num;
  uint8_t val[6]; 
  uint8_t interlaced_dct; //no use
  uint8_t mb_intra;
  uint8_t mv_dir;  
  uint8_t mv_type; 
  uint8_t y_dc_scale;
  //uint8_t *dest[3];
  int32_t mv[16];
  uint8_t mb_x,mb_y;
  uint8_t qscale;
  uint8_t chroma_qscale;
  int32_t block_last_index[6];
  uint8_t c_dc_scale;
  uint8_t ac_pred;
  uint8_t skip_idct;
  //uint16_t next_mb_len;
  short block[6][64];
}MPEG4_MB_DecARGs;

#define MPEG4_MB_CTRL_INFO (sizeof(MPEG4_MB_DecARGs)-(384<<1))
#endif
#endif /*__JZC_VP6_ARCH_H__*/

