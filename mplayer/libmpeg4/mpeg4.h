#ifndef _MPEG4_H_
#define _MPEG4_H_

#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3

av_cold int mpeg4_decode_end(AVCodecContext *avctx);
int mpeg4_decode_frame(AVCodecContext *avctx, void *data, int *data_size, AVPacket *avpkt);
av_cold int mpeg4_decode_init(AVCodecContext *avctx);
int mpeg4_decode_mb_p0(MpegEncContext *s, DCTELEM block[6][64]);

#ifndef JZC_P1_OPT

typedef struct _MPEG4_MB_DecARGs{
  uint8_t interlaced_dct; //no use
  uint8_t mb_skipped;
  int8_t mb_intra;
  int8_t mv_dir;
  uint8_t mb_x,mb_y;
  int8_t mv_type;
  int8_t mcsel;
  uint8_t ac_pred;
  uint8_t y_dc_scale;
  uint8_t c_dc_scale;
  uint8_t *dest[3];
  int32_t mv[2][4][2];
  int8_t field_select[2][2];
  int8_t qscale;
  int8_t chroma_qscale;
  int32_t block_last_index[6];
  short block[6][64];
  enum AVDiscard skip_idct;
}MPEG4_MB_DecARGs;

typedef struct _MPEG4_Frame_GlbARGs{
  uint32_t flags;
  uint32_t flags2;
  uint8_t encoding;
  uint8_t intra_only;
  int8_t pict_type;
  uint8_t mb_decision;

  Picture *current_picture_ptr;
  uint32_t draw_horiz_band;
  uint8_t lowres; //no use0
  uint8_t *mbskip_table;
  qpel_mc_func (*qpel_put)[16];
  qpel_mc_func (*qpel_avg)[16];
  int8_t no_rounding;
  op_pixels_func put_pixels_tab[4][4];
  op_pixels_func put_no_rnd_pixels_tab[4][4];
  op_pixels_func avg_pixels_tab[4][4];
  h264_chroma_mc_func put_h264_chroma_pixels_tab[3];
  h264_chroma_mc_func avg_h264_chroma_pixels_tab[3];
  uint8_t quarter_sample;
  int8_t obmc;
  int8_t real_sprite_warping_points;
  int32_t sprite_offset[2][2];
  int32_t sprite_delta[2][2];
  int8_t sprite_warping_accuracy;
  int16_t h_edge_pos,v_edge_pos;
  int8_t *edge_emu_buffer;
  uint8_t out_format;
  int8_t chroma_x_shift,chroma_y_shift;
  uint8_t codec_id;
  uint16_t width, height;
  uint8_t mb_width, mb_height;
  uint8_t b8_stride;
  uint8_t mb_stride;
  uint16_t linesize;
  uint16_t uvlinesize;
  uint8_t mspel;
  uint8_t picture_structure;
  int8_t h263_pred;
  int8_t h263_aic;
  uint8_t *mbintra_table;
  int8_t hurry_up;
  int8_t h263_msmpeg4;
  int8_t mpeg_quant;
  int8_t unrestricted_mv;
  int32_t workaround_bugs;
  int8_t alternate_scan;
  uint16_t *intra_matrix;
  uint16_t *inter_matrix;
  uint8_t *permutated;
  uint8_t *raster_end;

  //current_picture
  int32_t cp_linesize[2];
  //int32_t cp_age;
  int8_t cp_reference;
  //end current_picture
  uint8_t *last_picture_data[3];
  uint8_t *next_picture_data[3];
  uint8_t *b_scratchpad;
}MPEG4_Frame_GlbARGs;
#endif

#endif
