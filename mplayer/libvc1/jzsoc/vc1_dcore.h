/*****************************************************************************
 *
 * JZC VC1 Decoder Architecture
 *
 ****************************************************************************/

#ifndef __JZC_VC1_ARCH_H__
#define __JZC_VC1_ARCH_H__

typedef struct VC1_Frame_Ptr{
  uint8_t *ptr[3];
}VC1_Frame_Ptr;

typedef struct VC1_Frame_GlbARGs{
  uint8_t mb_width;
  uint8_t mb_height;
  uint8_t mb_stride;
  uint8_t pq;
  uint16_t pquantizer;
  uint8_t halfpq;

  uint8_t res_fasttx;   //
  uint8_t overlap;      // 
  uint8_t res_x8;       //
  uint8_t pict_type;    //
  uint8_t profile;      //
  uint8_t bi_type;      //
  uint8_t rangeredfrm;  //
  uint8_t rnd;
  uint8_t fastuvmc;
  uint8_t mspel;
  uint8_t use_ic;
  uint8_t flags;

  uint8_t chroma_y_shift;
  uint8_t chroma_x_shift;
  uint8_t lowres;
  uint8_t draw_horiz_band;
  uint8_t picture_structure;

  uint8_t *last_data[3];
  uint8_t *next_data[3];

  uint8_t *edge_emu_buffer;

  uint16_t h_edge_pos;
  uint16_t v_edge_pos;

  uint16_t coded_width;
  uint16_t coded_height;  

  uint16_t slinesize[3];  //
  uint16_t linesize;
  uint16_t uvlinesize;
  VC1_Frame_Ptr current_picture_save;
  VC1_Frame_Ptr rota_current_picture;
  uint8_t mvmode;
  int mpFrame;
}VC1_Frame_GlbARGs;

#define FRAME_T_CC_LINE ((sizeof(struct VC1_Frame_GlbARGs)+31)/32)

typedef struct VC1_MB_DecARGs{
  uint16_t next_dma_len;
  uint8_t mb_x;     
  uint8_t mb_y; 
  
  int16_t real_num;
  uint8_t idct_row[6];   //intra:12

  uint8_t pb_val[6];
  int16_t mq;

  uint8_t vc1_fourmv;  
  uint8_t mv_mode;
  uint8_t idct_row_4x4[24];
  uint8_t idct_row_8x4[12];

  uint8_t vc1_intra[6];  
  uint8_t ttblock[6];    

  char subblockpat[6];    
  uint8_t vc1_skipped;  
  uint8_t vc1_direct;

  uint8_t bfmv_type;
  //uint8_t pb_val[6];
  uint8_t vc1_mb_has_coeffs[2];   
  uint8_t vc1_b_mc_skipped;
  uint8_t chroma_ret;
  uint8_t bintra;
  //uint8_t dump[2];
  short tx, ty;
  short vc1_mv[4][2]; 
  DCTELEM mb[6*64]; 
}VC1_MB_DecARGs;

#if 0
typedef struct VC1_AUX_T{
  // linesize for mc
  unsigned char mc_des_dirty;
  int mb_linesize;
  int mb_uvlinesize;

  // reconstruction buffer
  uint8_t *h264_yrecon[2];
  uint8_t *h264_urecon[2];
  uint8_t *h264_vrecon[2];

  // for mc Bi-direction average/weight
  uint8_t *h264_ymcavg[2];
  uint8_t *h264_umcavg[2];
  uint8_t *h264_vmcavg[2];

  // dblk dst buffer
  uint8_t *h264_ydblk[2];

  // des pointers
  int h264_dblk_des_ptr[2];

  //weight back up for next mb
  uint16_t sub_mb_type[4];

  // back-up-bottom buffer
  uint8_t (*BackupMBbottom_Y)[16];
  uint8_t (*BackupMBbottom_U)[8];
  uint8_t (*BackupMBbottom_V)[8];

}VC1_AUX_T;
#endif
#endif /*__JZC_H264_ARCH_H__*/

