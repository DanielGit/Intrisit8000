/*
 * RV30/40 decoder common data
 * Copyright (c) 2007 Mike Melanson, Konstantin Shishkov
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * RV30/40 decoder common data
 */
#include "libavcore/imgutils.h"
#include "avcodec.h"
#include "dsputil.h"
#include "mpegvideo.h"
#include "golomb.h"
#include "rv34.h"
#include "rv34vlc.h"
#include "rv34data.h"
#include "rv40vlc2.h"
#include "rv40data.h"

#include "dsputil.h"
#include "mpegvideo.h"
#include "mathops.h"
#include "rectangle.h"

#ifdef JZC_MXU_OPT
#include "../libjzcommon/com_config.h"
#include "../libjzcommon/jzmedia.h"
#include "../libjzcommon/jzasm.h"
#include "../libjzcommon/jz4760e_dcsc.h"
#include "../libjzcommon/jz4760e_tcsm_init.c"
#include "../libjzcommon/jz4760e_2ddma_hw.h"
#include "rv40mxu.h"
#endif
#ifdef JZC_CRC_VER
#include "../libjzcommon/crc.c"
short crc_code_line;
#ifdef JZC_ROTA90_OPT
uint8_t *rota_y, *rota_c;
short crc_code_rota;
#endif
int rvFrame = 0;
#undef printf
#endif//JZC_CRC_VER

#ifdef JZC_TLB_OPT
volatile int tlb_i;
#include "../libjzcommon/t_vputlb.h"
//#include "jzsoc/t_intpid.h"
#endif

#if 1 //#ifdef JZ_LINUX_OS
extern unsigned int rv9_auxcodes_len, rv9_aux_task_codes[];
#endif

#ifdef JZC_MC_OPT
#include "jzsoc/rv9_p0_mc.c"
#endif

#ifdef JZC_DBLK_OPT
int do_dblk = 0;
#include "jzsoc/rv9_sram.h"
#include "jzsoc/jz4760e_rv_dblk.h"
#endif

#ifdef JZC_DCORE_OPT
uint32_t current_picture_ptr[3];
#include "jzsoc/rv9_dcore.h"
#include "jzsoc/rv9_tcsm1.h"
#include "jzsoc/rv9_tcsm0.h"
RV9_MB_DecARGs *task_fifo_mem;
int *frame_info_mem,*frame_info_mem_ex;
RV9_Slice_GlbARGs * dSlice;
RV9_Slice_GlbARGs * dSlice_ex;
RV9_MB_DecARGs *dMB;
int * task_fifo_wp;
volatile int * tcsm1_fifo_wp;
volatile int * tcsm0_fifo_rp;
#endif //JZC_DCORE_OPT

#ifdef JZC_IDCT_OPT
#include "../libjzcommon/jz4760e_idct_hw.h"
#endif

#ifdef JZC_VLC_HW_OPT
volatile char * rv9_hw_bs_buffer;
#include "vlc_tables.h"
#include "vlc_bs.c"
#endif

#ifdef JZC_PMON_P0
#include "../libjzcommon/jz4760e_pmon.h"
FILE *pmon_p0_fp;
extern char* filename;
int rv9_pmon_p0_frmcnt = 0;
PMON_CREAT(rv9vlc);
PMON_CREAT(rv9wait);
PMON_CREAT(rv9while);
#endif//JZC_PMON_P0

#define XCHG2(a,b,t)   t=a;a=b;b=t
#define JZ-IN-MODIFY
#ifdef JZ-IN-MODIFY
static unsigned char * dyy;
static unsigned char * duv;
#endif

#ifdef JZC_TEST_OPT
uint8_t *ydd, *udd, *vdd;
#endif
extern int use_jz_buf;

//#define DEBUG
static VLC aic_top_vlc;
static VLC aic_mode1_vlc[AIC_MODE1_NUM], aic_mode2_vlc[AIC_MODE2_NUM];
static VLC ptype_vlc[NUM_PTYPE_VLCS], btype_vlc[NUM_BTYPE_VLCS];

#if 0
static const int16_t mode2_offs[] = {
  0,  614, 1222, 1794, 2410,  3014,  3586,  4202,  4792, 5382, 5966, 6542,
  7138, 7716, 8292, 8864, 9444, 10030, 10642, 11212, 11814
};
#else
static const int16_t mode2_offs[] = {
      0,  426 , 856, 1208, 1646, 2064, 2414, 2856, 3224, 3588, 3950, 4310,
   4662, 5024, 5376, 5732, 6086, 6454, 6890, 7256, 7682
};
#endif

static av_always_inline void ZERO8x2(void* dst, int stride)
{
  fill_rectangle(dst,                 1, 2, stride, 0, 4);
  fill_rectangle(((uint8_t*)(dst))+4, 1, 2, stride, 0, 4);
}

/** translation of RV30/40 macroblock types to lavc ones */
static const int rv40_mb_type_to_lavc[12] = {
  MB_TYPE_INTRA,
  MB_TYPE_INTRA16x16              | MB_TYPE_SEPARATE_DC,
  MB_TYPE_16x16   | MB_TYPE_L0,
  MB_TYPE_8x8     | MB_TYPE_L0,
  MB_TYPE_16x16   | MB_TYPE_L0,
  MB_TYPE_16x16   | MB_TYPE_L1,
  MB_TYPE_SKIP,
  MB_TYPE_DIRECT2 | MB_TYPE_16x16,
  MB_TYPE_16x8    | MB_TYPE_L0,
  MB_TYPE_8x16    | MB_TYPE_L0,
  MB_TYPE_16x16   | MB_TYPE_L0L1,
  MB_TYPE_16x16   | MB_TYPE_L0    | MB_TYPE_SEPARATE_DC
};


static RV34VLC intra_vlcs[NUM_INTRA_TABLES], inter_vlcs[NUM_INTER_TABLES];

/**
 * @defgroup vlc RV30/40 VLC generating functions
 * @{
 */

#if 0
static const int table_offs[] = {
  0,   1818,   3622,   4144,   4698,   5234,   5804,   5868,   5900,   5932,
  5996,   6252,   6316,   6348,   6380,   7674,   8944,  10274,  11668,  12250,
  14060,  15846,  16372,  16962,  17512,  18148,  18180,  18212,  18244,  18308,
  18564,  18628,  18660,  18692,  20036,  21314,  22648,  23968,  24614,  26384,
  28190,  28736,  29366,  29938,  30608,  30640,  30672,  30704,  30768,  31024,
  31088,  31120,  31184,  32570,  33898,  35236,  36644,  37286,  39020,  40802,
  41368,  42052,  42692,  43348,  43380,  43412,  43444,  43476,  43604,  43668,
  43700,  43732,  45100,  46430,  47778,  49160,  49802,  51550,  53340,  53972,
  54648,  55348,  55994,  56122,  56154,  56186,  56218,  56346,  56410,  56442,
  56474,  57878,  59290,  60636,  62036,  62682,  64460,  64524,  64588,  64716,
  64844,  66076,  67466,  67978,  68542,  69064,  69648,  70296,  72010,  72074,
  72138,  72202,  72330,  73572,  74936,  75454,  76030,  76566,  77176,  77822,
  79582,  79646,  79678,  79742,  79870,  81180,  82536,  83064,  83672,  84242,
  84934,  85576,  87384,  87448,  87480,  87544,  87672,  88982,  90340,  90902,
  91598,  92182,  92846,  93488,  95246,  95278,  95310,  95374,  95502,  96878,
  98266,  98848,  99542, 100234, 100884, 101524, 103320, 103352, 103384, 103416,
  103480, 104874, 106222, 106910, 107584, 108258, 108902, 109544, 111366, 111398,
  111430, 111462, 111494, 112878, 114320, 114988, 115660, 116310, 116950, 117592
};
#else
static const int table_offs[] = {
     0,  1634,  3298,  3586,  3914,  4218,  4558,  4622,  4654,  4686, 
  4750,  5006,  5070,  5102,  5134,  6320,  7546,  8644,  9884, 10276, 
 11920, 13566, 13860, 14220, 14538, 14974, 15006, 15038, 15070, 15134,
 15390, 15454, 15486, 15518, 16696, 17904, 19014, 20222, 20738, 22324,
 23964, 24276, 24712, 25048, 25584, 25616, 25648, 25680, 25744, 26000,
 26064, 26096, 26160, 27328, 28534, 29650, 30878, 31392, 33000, 34570,
 34904, 35456, 35892, 36416, 36448, 36480, 36512, 36544, 36672, 36736,
 36768, 36800, 38028, 39208, 40328, 41522, 42036, 43698, 45284, 45718,
 46260, 46822, 47338, 47466, 47498, 47530, 47562, 47690, 47754, 47786,
 47818, 48984, 50232, 51342, 52624, 53140, 54744, 54808, 54872, 55000,
 55128, 56306, 57510, 57786, 58116, 58426, 58768, 59286, 60898, 60962,
 61026, 61090, 61218, 62408, 63604, 63888, 64226, 64550, 64932, 65450,
 67082, 67146, 67178, 67242, 67370, 68570, 69762, 70076, 70454, 70796,
 71352, 71866, 73456, 73520, 73552, 73616, 73744, 74906, 76138, 76470,
 77034, 77394, 77930, 78444, 79982, 80014, 80046, 80110, 80238, 81452,
 82598, 82954, 83516, 84078, 84596, 85108, 86788, 86820, 86852, 86884,
 86948, 88180, 89284, 89842, 90380, 90922, 91438, 91950, 93558, 93590,
 93622, 93654, 93686, 94940, 96256, 96800, 97338, 97862, 98374, 98886,
};
#endif

static VLC_TYPE table_data[98886][2];

/**
 * Generate VLC from codeword lengths.
 * @param bits   codeword lengths (zeroes are accepted)
 * @param size   length of input data
 * @param vlc    output VLC
 * @param insyms symbols for input codes (NULL for default ones)
 * @param num    VLC table number (for static initialization)
 */
static void rv40_gen_vlc(const uint8_t *bits, int size, VLC *vlc, const uint8_t *insyms,
                         const int num)
{
  int i;
  int counts[17] = {0}, codes[17];
  uint16_t cw[MAX_VLC_SIZE], syms[MAX_VLC_SIZE];
  uint8_t bits2[MAX_VLC_SIZE];
  int maxbits = 0, realsize = 0;

  for(i = 0; i < size; i++){
    if(bits[i]){
      bits2[realsize] = bits[i];
      syms[realsize] = insyms ? insyms[i] : i;
      realsize++;
      maxbits = FFMAX(maxbits, bits[i]);
      counts[bits[i]]++;
    }
  }

  codes[0] = 0;
  for(i = 0; i < 16; i++)
    codes[i+1] = (codes[i] + counts[i]) << 1;
  for(i = 0; i < realsize; i++)
    cw[i] = codes[bits2[i]]++;

  vlc->table = &table_data[table_offs[num]];
  vlc->table_allocated = table_offs[num + 1] - table_offs[num];
  init_vlc_sparse(vlc, FFMIN(maxbits, 8), realsize,
		  bits2, 1, 1,
		  cw,    2, 2,
		  syms,  2, 2, INIT_VLC_USE_NEW_STATIC);
}

/**
 * Initialize all tables.
 */
static av_cold void rv34_init_tables(void)
{
  int i, j, k;

  for(i = 0; i < NUM_INTRA_TABLES; i++){
    for(j = 0; j < 2; j++){
      rv40_gen_vlc(rv34_table_intra_cbppat   [i][j], CBPPAT_VLC_SIZE,   &intra_vlcs[i].cbppattern[j],     NULL, 19*i + 0 + j);
      rv40_gen_vlc(rv34_table_intra_secondpat[i][j], OTHERBLK_VLC_SIZE, &intra_vlcs[i].second_pattern[j], NULL, 19*i + 2 + j);
      rv40_gen_vlc(rv34_table_intra_thirdpat [i][j], OTHERBLK_VLC_SIZE, &intra_vlcs[i].third_pattern[j],  NULL, 19*i + 4 + j);
      for(k = 0; k < 4; k++){
	rv40_gen_vlc(rv34_table_intra_cbp[i][j+k*2],  CBP_VLC_SIZE,   &intra_vlcs[i].cbp[j][k],         rv34_cbp_code, 19*i + 6 + j*4 + k);
      }
    }
    for(j = 0; j < 4; j++){
      rv40_gen_vlc(rv34_table_intra_firstpat[i][j], FIRSTBLK_VLC_SIZE, &intra_vlcs[i].first_pattern[j], NULL, 19*i + 14 + j);
    }
    rv40_gen_vlc(rv34_intra_coeff[i], COEFF_VLC_SIZE, &intra_vlcs[i].coefficient, NULL, 19*i + 18);
  }

  for(i = 0; i < NUM_INTER_TABLES; i++){
    rv40_gen_vlc(rv34_inter_cbppat[i], CBPPAT_VLC_SIZE, &inter_vlcs[i].cbppattern[0], NULL, i*12 + 95);
    for(j = 0; j < 4; j++){
      rv40_gen_vlc(rv34_inter_cbp[i][j], CBP_VLC_SIZE, &inter_vlcs[i].cbp[0][j], rv34_cbp_code, i*12 + 96 + j);
    }
    for(j = 0; j < 2; j++){
      rv40_gen_vlc(rv34_table_inter_firstpat [i][j], FIRSTBLK_VLC_SIZE, &inter_vlcs[i].first_pattern[j],  NULL, i*12 + 100 + j);
      rv40_gen_vlc(rv34_table_inter_secondpat[i][j], OTHERBLK_VLC_SIZE, &inter_vlcs[i].second_pattern[j], NULL, i*12 + 102 + j);
      rv40_gen_vlc(rv34_table_inter_thirdpat [i][j], OTHERBLK_VLC_SIZE, &inter_vlcs[i].third_pattern[j],  NULL, i*12 + 104 + j);
    }
    rv40_gen_vlc(rv34_inter_coeff[i], COEFF_VLC_SIZE, &inter_vlcs[i].coefficient, NULL, i*12 + 106);
  }
}

/** @} */ // vlc group
/**
 * Initialize all tables.
 */
static av_cold void rv40_init_tables(void)
{
  int i,j;
#ifdef JZC_VLC_HW_OPT
  VLC_TYPE_8B (*cp_ptr)[2];
#endif
  static VLC_TYPE aic_table[1 << AIC_TOP_BITS][2];
  static VLC_TYPE aic_mode1_table[AIC_MODE1_NUM << AIC_MODE1_BITS][2];
  static VLC_TYPE aic_mode2_table[7682][2];
  static VLC_TYPE ptype_table[NUM_PTYPE_VLCS << PTYPE_VLC_BITS][2];
  static VLC_TYPE btype_table[NUM_BTYPE_VLCS << BTYPE_VLC_BITS][2];

  aic_top_vlc.table = aic_table;
  aic_top_vlc.table_allocated = 1 << AIC_TOP_BITS;
  init_vlc(&aic_top_vlc, AIC_TOP_BITS, AIC_TOP_SIZE,
	   rv40_aic_top_vlc_bits,  1, 1,
	   rv40_aic_top_vlc_codes, 1, 1, INIT_VLC_USE_NEW_STATIC);
  for(i = 0; i < AIC_MODE1_NUM; i++){
    // Every tenth VLC table is empty
    if((i % 10) == 9) continue;
    aic_mode1_vlc[i].table = &aic_mode1_table[i << AIC_MODE1_BITS];
    aic_mode1_vlc[i].table_allocated = 1 << AIC_MODE1_BITS;
    init_vlc(&aic_mode1_vlc[i], AIC_MODE1_BITS, AIC_MODE1_SIZE,
	     aic_mode1_vlc_bits[i],  1, 1,
	     aic_mode1_vlc_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
#ifdef JZC_VLC_HW_OPT
    cp_ptr=aic_mode1_vlc[i].table;
    for(j=0;j<(1 << AIC_MODE1_BITS);j++){
      cp_ptr[j][0]=aic_mode1_vlc[i].table[j][0];
      cp_ptr[j][1]=(aic_mode1_vlc[i].table[j][1]-1<< 4) | 0xd;
    }
#endif
  }
  for(i = 0; i < AIC_MODE2_NUM; i++){
    aic_mode2_vlc[i].table = &aic_mode2_table[mode2_offs[i]];
    aic_mode2_vlc[i].table_allocated = mode2_offs[i + 1] - mode2_offs[i];
    init_vlc(&aic_mode2_vlc[i], AIC_MODE2_BITS, AIC_MODE2_SIZE,
	     aic_mode2_vlc_bits[i],  1, 1,
	     aic_mode2_vlc_codes[i], 2, 2, INIT_VLC_USE_NEW_STATIC);
  }
  for(i = 0; i < NUM_PTYPE_VLCS; i++){
    ptype_vlc[i].table = &ptype_table[i << PTYPE_VLC_BITS];
    ptype_vlc[i].table_allocated = 1 << PTYPE_VLC_BITS;
    init_vlc_sparse(&ptype_vlc[i], PTYPE_VLC_BITS, PTYPE_VLC_SIZE,
		    ptype_vlc_bits[i],  1, 1,
		    ptype_vlc_codes[i], 1, 1,
		    ptype_vlc_syms,     1, 1, INIT_VLC_USE_NEW_STATIC);
#ifdef JZC_VLC_HW_OPT
    cp_ptr=ptype_vlc[i].table;
    for(j=0;j<(1 << PTYPE_VLC_BITS);j++){
      cp_ptr[j][0]=ptype_vlc[i].table[j][0];
      cp_ptr[j][1]=(ptype_vlc[i].table[j][1]-1<< 4) | 0xd;
    }
#endif
  }
  for(i = 0; i < NUM_BTYPE_VLCS; i++){
    btype_vlc[i].table = &btype_table[i << BTYPE_VLC_BITS];
    btype_vlc[i].table_allocated = 1 << BTYPE_VLC_BITS;
    init_vlc_sparse(&btype_vlc[i], BTYPE_VLC_BITS, BTYPE_VLC_SIZE,
		    btype_vlc_bits[i],  1, 1,
		    btype_vlc_codes[i], 1, 1,
		    btype_vlc_syms,     1, 1, INIT_VLC_USE_NEW_STATIC);
#ifdef JZC_VLC_HW_OPT
    cp_ptr=btype_vlc[i].table;
    for(j=0;j<(1 << BTYPE_VLC_BITS);j++){
      cp_ptr[j][0]=btype_vlc[i].table[j][0];
      cp_ptr[j][1]=(btype_vlc[i].table[j][1]-1<< 4) | 0xd;
    }
#endif
  }
}

/**
 * Get stored dimension from bitstream.
 *
 * If the width/height is the standard one then it's coded as a 3-bit index.
 * Otherwise it is coded as escaped 8-bit portions.
 */
static int get_dimension(GetBitContext *gb, const int *dim)
{
  int t   = get_bits(gb, 3);
  int val = dim[t];
  if(val < 0)
    val = dim[get_bits1(gb) - val];
  if(!val){
    do{
      t = get_bits(gb, 8);
      val += t << 2;
    }while(t == 0xFF);
  }
  return val;
}

/**
 * Get encoded picture size - usually this is called from rv40_parse_slice_header.
 */
static void rv40_parse_picture_size(GetBitContext *gb, int *w, int *h)
{
  *w = get_dimension(gb, rv40_standard_widths);
  *h = get_dimension(gb, rv40_standard_heights);
}

static int rv40_parse_slice_header(RV34DecContext *r, GetBitContext *gb, SliceInfo *si)
{
  int mb_bits;
  int w = r->s.width, h = r->s.height;
  int mb_size;

  memset(si, 0, sizeof(SliceInfo));
  if(get_bits1(gb))
    return -1;
  si->type = get_bits(gb, 2);
  if(si->type == 1) si->type = 0;
  si->quant = get_bits(gb, 5);
  if(get_bits(gb, 2))
    return -1;
  si->vlc_set = get_bits(gb, 2);
  skip_bits1(gb);
  si->pts = get_bits(gb, 13);
  if(!si->type || !get_bits1(gb))
    rv40_parse_picture_size(gb, &w, &h);
  if(av_image_check_size(w, h, 0, r->s.avctx) < 0)
    return -1;
  si->width  = w;
  si->height = h;
  mb_size = ((w + 15) >> 4) * ((h + 15) >> 4);
  mb_bits = ff_rv40_get_start_offset(gb, mb_size);
  si->start = get_bits(gb, mb_bits);

  return 0;
}

/**
 * Decode 4x4 intra types array.
 */
#ifdef JZC_VLC_HW_OPT
#define get_bits(s,n) get_bits_hw(n)
#define get_bits1(s) get_bits1_hw()
#define skip_bits(s,n) skip_bits_hw(n)
#define show_bits(s,n) show_ubits_hw(n)
#define get_vlc2(s,table,bits,max_depth) get_vlc2_hw(table,bits,max_depth)
#define svq3_get_ue_golomb svq3_get_ue_golombl
#define svq3_get_se_golomb svq3_get_se_golombl
#endif

#ifdef DIV9_TABLE_OPT
uint8_t * div9_tcsm0=TCSM0_DIV9_TABLE;
uint8_t div9[81]={
  0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,
  0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71,0x81,
  0x02,0x12,0x22,0x32,0x42,0x52,0x62,0x72,0x82,
  0x03,0x13,0x23,0x33,0x43,0x53,0x63,0x73,0x83,
  0x04,0x14,0x24,0x34,0x44,0x54,0x64,0x74,0x84,
  0x05,0x15,0x25,0x35,0x45,0x55,0x65,0x75,0x85,
  0x06,0x16,0x26,0x36,0x46,0x56,0x66,0x76,0x86,
  0x07,0x17,0x27,0x37,0x47,0x57,0x67,0x77,0x87,
  0x08,0x18,0x28,0x38,0x48,0x58,0x68,0x78,0x88,
};
#endif

#ifdef HUGE_AIC_TABLE_OPT
char * huge_rv40_aic_table_index=TCSM0_HUGE_AIC_TABLE;
#endif

static av_always_inline int rv40_decode_intra_types(RV34DecContext *r, GetBitContext *gb, int8_t *dst)
{
  MpegEncContext *s = &r->s;
  int i, j, k, v;
  int A, B, C;
  int pattern;
  int8_t *ptr;

  for(i = 0; i < 4; i++, dst += r->intra_types_stride){
    if(!i && s->first_slice_line){
      pattern = get_vlc2(gb, aic_top_vlc.table, AIC_TOP_BITS, 1);
      dst[0] = (pattern >> 2) & 2;
      dst[1] = (pattern >> 1) & 2;
      dst[2] =  pattern       & 2;
      dst[3] = (pattern << 1) & 2;
      continue;
    }
    ptr = dst;
    for(j = 0; j < 4; j++){
      /* Coefficients are read using VLC chosen by the prediction pattern
       * The first one (used for retrieving a pair of coefficients) is
       * constructed from the top, top right and left coefficients
       * The second one (used for retrieving only one coefficient) is
       * top + 10 * left.
       */
      A = ptr[-r->intra_types_stride + 1]; // it won't be used for the last coefficient in a row
      B = ptr[-r->intra_types_stride];
      C = ptr[-1];
      pattern = A + (B << 4) + (C << 8);

#ifdef HUGE_AIC_TABLE_OPT
      k=huge_rv40_aic_table_index[pattern&0xfff];
#else
      if(A!=B&&B!=C&&A!=C)
	k=MODE2_PATTERNS_NUM;
      else
      for(k = 0; k < MODE2_PATTERNS_NUM; k++)
	if(pattern == rv40_aic_table_index[k])
	  break;
#endif

      if(k < MODE2_PATTERNS_NUM && j < 3){ //pattern is found, decoding 2 coefficients
	v = get_vlc2(gb, aic_mode2_vlc[k].table, AIC_MODE2_BITS, 2);
#ifdef DIV9_TABLE_OPT
	uint32_t a=div9_tcsm0[v];
	*ptr++ = a&0xf;
	*ptr++ = a>>4;
#else
	*ptr++ = v/9;
	*ptr++ = v%9;
#endif
	j++;
      }else{
#ifdef JZC_VLC_HW_OPT
	CPU_SET_CTRL((AIC_MODE1_BITS-1 << 4) | (1 << 3) | (3 << 1) | 1);
#endif
	if((B|C)>=0){
#ifdef JZC_VLC_HW_OPT
	  VLC_TYPE_8B (*table)[2]=aic_mode1_vlc[B + C*10].table;
	  int index=CPU_GET_RSLT();
	  v = table[index][0];
	  int ctrl = table[index][1];
	  CPU_SET_CTRL(ctrl);
#else
	  v = get_vlc2(gb, aic_mode1_vlc[B + C*10].table, AIC_MODE1_BITS, 1);
#endif
	}
	else{ // tricky decoding
	  v = 0;
	  switch(C){
	  case -1: // code 0 -> 1, 1 -> 0
	    if(B < 2)
	      v = get_bits1(gb) ^ 1;
	    break;
	  case  0:
	  case  2: // code 0 -> 2, 1 -> 0
	    v = (get_bits1(gb) ^ 1) << 1;
	    break;
	  }
	}
	*ptr++ = v;
      }

    }
  }

  return 0;
}

/**
 * Decode macroblock information.
 */
static av_always_inline int svq3_get_ue_golombl(GetBitContext *gb){
  uint32_t buf,t;

  t=get_bits1(gb);
  buf=show_bits(gb,8);
  buf|=(t<<8);
  if(buf&0x155){
    buf >>= 1;

    if(ff_interleaved_golomb_vlc_len[buf]-1) skip_bits(gb, ff_interleaved_golomb_vlc_len[buf]-1);
    return ff_interleaved_ue_golomb_vlc_code[buf];
  }else{
    int ret = 1;
    buf >>= 1;
    if(ff_interleaved_golomb_vlc_len[buf]-1) skip_bits(gb,FFMIN(ff_interleaved_golomb_vlc_len[buf], 8)-1);

    while (1) {
      if (ff_interleaved_golomb_vlc_len[buf] != 9){
	ret <<= (ff_interleaved_golomb_vlc_len[buf] - 1) >> 1;
	ret |= ff_interleaved_dirac_golomb_vlc_code[buf];
	break;
      }
      ret = (ret << 4) | ff_interleaved_dirac_golomb_vlc_code[buf];
      buf=show_bits(gb,8);
      skip_bits(gb,FFMIN(ff_interleaved_golomb_vlc_len[buf], 8));
    }
    return ret - 1;
  }
}
static av_always_inline int rv40_decode_mb_info(RV34DecContext *r,int bk_mb_type)
{
  MpegEncContext *s = &r->s;
  GetBitContext *gb = &s->gb;
  int q, i;
  int prev_type = 0;
  int mb_pos = s->mb_x ;
  int blocks[RV34_MB_TYPES] = {0};
  int count = 0;

  if(!r->s.mb_skip_run)
    r->s.mb_skip_run = svq3_get_ue_golomb(gb) + 1;

  if(--r->s.mb_skip_run)
    return RV34_MB_SKIP;

  if(r->avail_cache[6-1])
    blocks[r->mb_type[mb_pos - 1]]++;
  if(r->avail_cache[6-4]){
    blocks[r->mb_type[mb_pos ]]++;
    if(r->avail_cache[6-2])
      blocks[r->mb_type[mb_pos + 1]]++;
    if(r->avail_cache[6-5])
      blocks[bk_mb_type]++;
  }

  for(i = 0; i < RV34_MB_TYPES; i++){
    if(blocks[i] > count){
      count = blocks[i];
      prev_type = i;
    }
  }
#ifdef JZC_VLC_HW_OPT
  uint32_t ctrl_val;
  int index,ctrl;
  VLC_TYPE_8B (*table)[2];
  ctrl_val = ((PTYPE_VLC_BITS-1 << 4) | (1 << 3) | (3 << 1) | 1 );
  CPU_SET_CTRL(ctrl_val);
#endif
  if(s->pict_type == FF_P_TYPE){
#ifdef JZC_VLC_HW_OPT
    prev_type = block_num_to_ptype_vlc_num[prev_type];
    table=ptype_vlc[prev_type].table;
    index= CPU_GET_RSLT();
    q = table[index][0];
    ctrl = table[index][1];
    CPU_SET_CTRL(ctrl);
#else
    prev_type = block_num_to_ptype_vlc_num[prev_type];
    q = get_vlc2(gb, ptype_vlc[prev_type].table, PTYPE_VLC_BITS, 1);
#endif
    if(q < PBTYPE_ESCAPE)
      return q;
#ifdef JZC_VLC_HW_OPT
    CPU_SET_CTRL(ctrl_val);
    index= CPU_GET_RSLT();
    index= CPU_GET_RSLT();
    q = table[index][0];
    ctrl = table[index][1];
    CPU_SET_CTRL(ctrl);
#else
    q = get_vlc2(gb, ptype_vlc[prev_type].table, PTYPE_VLC_BITS, 1);
#endif
    av_log(s->avctx, AV_LOG_ERROR, "Dquant for P-frame\n");
  }else{
#ifdef JZC_VLC_HW_OPT
    prev_type = block_num_to_btype_vlc_num[prev_type];
    table=btype_vlc[prev_type].table;
    index= CPU_GET_RSLT()>>1;
    q = table[index][0];
    ctrl = table[index][1];
    CPU_SET_CTRL(ctrl);
#else
    prev_type = block_num_to_btype_vlc_num[prev_type];
    q = get_vlc2(gb, btype_vlc[prev_type].table, BTYPE_VLC_BITS, 1);
#endif
    if(q < PBTYPE_ESCAPE)
      return q;
#ifdef JZC_VLC_HW_OPT
    CPU_SET_CTRL((BTYPE_VLC_BITS-1 << 4) | (1 << 3) | (3 << 1) | 1);
    index= CPU_GET_RSLT();
    index= CPU_GET_RSLT();
    q = table[index][0];
    ctrl = table[index][1];
    CPU_SET_CTRL(ctrl);
#else
    q = get_vlc2(gb, btype_vlc[prev_type].table, BTYPE_VLC_BITS, 1);
#endif
    av_log(s->avctx, AV_LOG_ERROR, "Dquant for B-frame\n");
  }
  return 0;
}

#define CLIP_SYMM(a, b) av_clip(a, -(b), b)
/**
 * weaker deblocking very similar to the one described in 4.4.2 of JVT-A003r1
 */
#if 0
static inline void rv40_weak_loop_filter(uint8_t *src, const int step,
                                         const int filter_p1, const int filter_q1,
                                         const int alpha, const int beta,
                                         const int lim_p0q0,
                                         const int lim_q1, const int lim_p1,
                                         const int diff_p1p0, const int diff_q1q0,
                                         const int diff_p1p2, const int diff_q1q2)
{
  uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;
  int t, u, diff;

  t = src[0*step] - src[-1*step];
  if(!t)
    return;
  u = (alpha * FFABS(t)) >> 7;
  if(u > 3 - (filter_p1 && filter_q1))
    return;

  t <<= 2;
  if(filter_p1 && filter_q1)
    t += src[-2*step] - src[1*step];
  diff = CLIP_SYMM((t + 4) >> 3, lim_p0q0);
  src[-1*step] = cm[src[-1*step] + diff];
  src[ 0*step] = cm[src[ 0*step] - diff];
  if(FFABS(diff_p1p2) <= beta && filter_p1){
    t = (diff_p1p0 + diff_p1p2 - diff) >> 1;
    src[-2*step] = cm[src[-2*step] - CLIP_SYMM(t, lim_p1)];
  }
  if(FFABS(diff_q1q2) <= beta && filter_q1){
    t = (diff_q1q0 + diff_q1q2 + diff) >> 1;
    src[ 1*step] = cm[src[ 1*step] - CLIP_SYMM(t, lim_q1)];
  }
}

static av_always_inline void rv40_adaptive_loop_filter(uint8_t *src, const int step,
						       const int stride, const int dmode,
						       const int lim_q1, const int lim_p1,
						       const int alpha,
						       const int beta, const int beta2,
						       const int chroma, const int edge)
{
  int diff_p1p0[4], diff_q1q0[4], diff_p1p2[4], diff_q1q2[4];
  int sum_p1p0 = 0, sum_q1q0 = 0, sum_p1p2 = 0, sum_q1q2 = 0;
  uint8_t *ptr;
  int flag_strong0 = 1, flag_strong1 = 1;
  int filter_p1, filter_q1;
  int i;
  int lims;

  for(i = 0, ptr = src; i < 4; i++, ptr += stride){
    diff_p1p0[i] = ptr[-2*step] - ptr[-1*step];
    diff_q1q0[i] = ptr[ 1*step] - ptr[ 0*step];
    sum_p1p0 += diff_p1p0[i];
    sum_q1q0 += diff_q1q0[i];
  }
  filter_p1 = FFABS(sum_p1p0) < (beta<<2);
  filter_q1 = FFABS(sum_q1q0) < (beta<<2);
  if(!filter_p1 && !filter_q1)
    return;

  for(i = 0, ptr = src; i < 4; i++, ptr += stride){
    diff_p1p2[i] = ptr[-2*step] - ptr[-3*step];
    diff_q1q2[i] = ptr[ 1*step] - ptr[ 2*step];
    sum_p1p2 += diff_p1p2[i];
    sum_q1q2 += diff_q1q2[i];
  }

  if(edge){
    flag_strong0 = filter_p1 && (FFABS(sum_p1p2) < beta2);
    flag_strong1 = filter_q1 && (FFABS(sum_q1q2) < beta2);
  }else{
    flag_strong0 = flag_strong1 = 0;
  }

  lims = filter_p1 + filter_q1 + ((lim_q1 + lim_p1) >> 1) + 1;
  if(flag_strong0 && flag_strong1){ /* strong filtering */
    for(i = 0; i < 4; i++, src += stride){
      int sflag, p0, q0, p1, q1;
      int t = src[0*step] - src[-1*step];

      if(!t) continue;
      sflag = (alpha * FFABS(t)) >> 7;
      if(sflag > 1) continue;

      p0 = (25*src[-3*step] + 26*src[-2*step]
	    + 26*src[-1*step]
	    + 26*src[ 0*step] + 25*src[ 1*step] + rv40_dither_l[dmode + i]) >> 7;
      q0 = (25*src[-2*step] + 26*src[-1*step]
	    + 26*src[ 0*step]
	    + 26*src[ 1*step] + 25*src[ 2*step] + rv40_dither_r[dmode + i]) >> 7;
      if(sflag){
	p0 = av_clip(p0, src[-1*step] - lims, src[-1*step] + lims);
	q0 = av_clip(q0, src[ 0*step] - lims, src[ 0*step] + lims);
      }
      p1 = (25*src[-4*step] + 26*src[-3*step]
	    + 26*src[-2*step]
	    + 26*p0           + 25*src[ 0*step] + rv40_dither_l[dmode + i]) >> 7;
      q1 = (25*src[-1*step] + 26*q0
	    + 26*src[ 1*step]
	    + 26*src[ 2*step] + 25*src[ 3*step] + rv40_dither_r[dmode + i]) >> 7;
      if(sflag){
	p1 = av_clip(p1, src[-2*step] - lims, src[-2*step] + lims);
	q1 = av_clip(q1, src[ 1*step] - lims, src[ 1*step] + lims);
      }
      src[-2*step] = p1;
      src[-1*step] = p0;
      src[ 0*step] = q0;
      src[ 1*step] = q1;
      if(!chroma){
	src[-3*step] = (25*src[-1*step] + 26*src[-2*step] + 51*src[-3*step] + 26*src[-4*step] + 64) >> 7;
	src[ 2*step] = (25*src[ 0*step] + 26*src[ 1*step] + 51*src[ 2*step] + 26*src[ 3*step] + 64) >> 7;
      }
    }
  }else if(filter_p1 && filter_q1){
    for(i = 0; i < 4; i++, src += stride)
      rv40_weak_loop_filter(src, step, 1, 1, alpha, beta, lims, lim_q1, lim_p1,
			    diff_p1p0[i], diff_q1q0[i], diff_p1p2[i], diff_q1q2[i]);
  }else{
    for(i = 0; i < 4; i++, src += stride)
      rv40_weak_loop_filter(src, step, filter_p1, filter_q1,
			    alpha, beta, lims>>1, lim_q1>>1, lim_p1>>1,
			    diff_p1p0[i], diff_q1q0[i], diff_p1p2[i], diff_q1q2[i]);
  }
}

static void rv40_v_loop_filter(uint8_t *src, int stride, int dmode,
                               int lim_q1, int lim_p1,
                               int alpha, int beta, int beta2, int chroma, int edge){
  rv40_adaptive_loop_filter(src, 1, stride, dmode, lim_q1, lim_p1,
			    alpha, beta, beta2, chroma, edge);
}
static void rv40_h_loop_filter(uint8_t *src, int stride, int dmode,
                               int lim_q1, int lim_p1,
                               int alpha, int beta, int beta2, int chroma, int edge){
  rv40_adaptive_loop_filter(src, stride, 1, dmode, lim_q1, lim_p1,
			    alpha, beta, beta2, chroma, edge);
}

enum RV40BlockPos{
  POS_CUR,
  POS_TOP,
  POS_LEFT,
  POS_BOTTOM,
};

#define MASK_CUR          0x0001
#define MASK_RIGHT        0x0008
#define MASK_BOTTOM       0x0010
#define MASK_TOP          0x1000
#define MASK_Y_TOP_ROW    0x000F
#define MASK_Y_LAST_ROW   0xF000
#define MASK_Y_LEFT_COL   0x1111
#define MASK_Y_RIGHT_COL  0x8888
#define MASK_C_TOP_ROW    0x0003
#define MASK_C_LAST_ROW   0x000C
#define MASK_C_LEFT_COL   0x0005
#define MASK_C_RIGHT_COL  0x000A

static const int neighbour_offs_x[4] = { 0,  0, -1, 0 };
static const int neighbour_offs_y[4] = { 0, -1,  0, 1 };

/**
 * RV40 loop filtering function
 */
static void rv40_loop_filter(RV34DecContext *r, int row)
{
  MpegEncContext *s = &r->s;
  int mb_pos, mb_x;
  int i, j, k;
  uint8_t *Y, *C;
  int alpha, beta, betaY, betaC;
  int q;
  int mbtype[4];   ///< current macroblock and its neighbours types
  /**
   * flags indicating that macroblock can be filtered with strong filter
   * it is set only for intra coded MB and MB with DCs coded separately
   */
  int mb_strong[4];
  int clip[4];     ///< MB filter clipping value calculated from filtering strength
  /**
   * coded block patterns for luma part of current macroblock and its neighbours
   * Format:
   * LSB corresponds to the top left block,
   * each nibble represents one row of subblocks.
   */
  int cbp[4];
  /**
   * coded block patterns for chroma part of current macroblock and its neighbours
   * Format is the same as for luma with two subblocks in a row.
   */
  int uvcbp[4][2];
  /**
   * This mask represents the pattern of luma subblocks that should be filtered
   * in addition to the coded ones because because they lie at the edge of
   * 8x8 block with different enough motion vectors
   */
  int mvmasks[4];

  mb_pos = row * s->mb_stride;
  for(mb_x = 0; mb_x < s->mb_width; mb_x++, mb_pos++){
    int mbtype = s->current_picture_ptr->mb_type[mb_pos];
    if(IS_INTRA(mbtype) || IS_SEPARATE_DC(mbtype))
      r->cbp_luma  [mb_pos] = r->deblock_coefs[mb_pos] = 0xFFFF;
    if(IS_INTRA(mbtype))
      r->cbp_chroma[mb_pos] = 0xFF;
  }
  mb_pos = row * s->mb_stride;
  for(mb_x = 0; mb_x < s->mb_width; mb_x++, mb_pos++){
    int y_h_deblock, y_v_deblock;
    int c_v_deblock[2], c_h_deblock[2];
    int clip_left;
    int avail[4];
    int y_to_deblock, c_to_deblock[2];

    q = s->current_picture_ptr->qscale_table[mb_pos];
    alpha = rv40_alpha_tab[q];
    beta  = rv40_beta_tab [q];
    betaY = betaC = beta * 3;
    if(s->width * s->height <= 176*144)
      betaY += beta;

    avail[0] = 1;
    avail[1] = row;
    avail[2] = mb_x;
    avail[3] = row < s->mb_height - 1;
    for(i = 0; i < 4; i++){
      if(avail[i]){
	int pos = mb_pos + neighbour_offs_x[i] + neighbour_offs_y[i]*s->mb_stride;
	mvmasks[i] = r->deblock_coefs[pos];
	mbtype [i] = s->current_picture_ptr->mb_type[pos];
	cbp    [i] = r->cbp_luma[pos];
	uvcbp[i][0] = r->cbp_chroma[pos] & 0xF;
	uvcbp[i][1] = r->cbp_chroma[pos] >> 4;
      }else{
	mvmasks[i] = 0;
	mbtype [i] = mbtype[0];
	cbp    [i] = 0;
	uvcbp[i][0] = uvcbp[i][1] = 0;
      }
      mb_strong[i] = IS_INTRA(mbtype[i]) || IS_SEPARATE_DC(mbtype[i]);
      clip[i] = rv40_filter_clip_tbl[mb_strong[i] + 1][q];
    }
    y_to_deblock =  mvmasks[POS_CUR]
      | (mvmasks[POS_BOTTOM] << 16);
    /* This pattern contains bits signalling that horizontal edges of
     * the current block can be filtered.
     * That happens when either of adjacent subblocks is coded or lies on
     * the edge of 8x8 blocks with motion vectors differing by more than
     * 3/4 pel in any component (any edge orientation for some reason).
     */
    y_h_deblock =   y_to_deblock
      | ((cbp[POS_CUR]                           <<  4) & ~MASK_Y_TOP_ROW)
      | ((cbp[POS_TOP]        & MASK_Y_LAST_ROW) >> 12);
    /* This pattern contains bits signalling that vertical edges of
     * the current block can be filtered.
     * That happens when either of adjacent subblocks is coded or lies on
     * the edge of 8x8 blocks with motion vectors differing by more than
     * 3/4 pel in any component (any edge orientation for some reason).
     */
    y_v_deblock =   y_to_deblock
      | ((cbp[POS_CUR]                      << 1) & ~MASK_Y_LEFT_COL)
      | ((cbp[POS_LEFT] & MASK_Y_RIGHT_COL) >> 3);
    if(!mb_x)
      y_v_deblock &= ~MASK_Y_LEFT_COL;
    if(!row)
      y_h_deblock &= ~MASK_Y_TOP_ROW;
    if(row == s->mb_height - 1 || (mb_strong[POS_CUR] || mb_strong[POS_BOTTOM]))
      y_h_deblock &= ~(MASK_Y_TOP_ROW << 16);
    /* Calculating chroma patterns is similar and easier since there is
     * no motion vector pattern for them.
     */
    for(i = 0; i < 2; i++){
      c_to_deblock[i] = (uvcbp[POS_BOTTOM][i] << 4) | uvcbp[POS_CUR][i];
      c_v_deblock[i] =   c_to_deblock[i]
	| ((uvcbp[POS_CUR] [i]                       << 1) & ~MASK_C_LEFT_COL)
	| ((uvcbp[POS_LEFT][i]   & MASK_C_RIGHT_COL) >> 1);
      c_h_deblock[i] =   c_to_deblock[i]
	| ((uvcbp[POS_TOP][i]    & MASK_C_LAST_ROW)  >> 2)
	|  (uvcbp[POS_CUR][i]                        << 2);
      if(!mb_x)
	c_v_deblock[i] &= ~MASK_C_LEFT_COL;
      if(!row)
	c_h_deblock[i] &= ~MASK_C_TOP_ROW;
      if(row == s->mb_height - 1 || mb_strong[POS_CUR] || mb_strong[POS_BOTTOM])
	c_h_deblock[i] &= ~(MASK_C_TOP_ROW << 4);
    }

    for(j = 0; j < 16; j += 4){
      Y = s->current_picture_ptr->data[0] + mb_x*16 + (row*16 + j) * s->linesize;
      for(i = 0; i < 4; i++, Y += 4){
	int ij = i + j;
	int clip_cur = y_to_deblock & (MASK_CUR << ij) ? clip[POS_CUR] : 0;
	int dither = j ? ij : i*4;

	// if bottom block is coded then we can filter its top edge
	// (or bottom edge of this block, which is the same)
	if(y_h_deblock & (MASK_BOTTOM << ij)){
	  rv40_h_loop_filter(Y+4*s->linesize, s->linesize, dither,
			     y_to_deblock & (MASK_BOTTOM << ij) ? clip[POS_CUR] : 0,
			     clip_cur,
			     alpha, beta, betaY, 0, 0);
	}
	// filter left block edge in ordinary mode (with low filtering strength)
	if(y_v_deblock & (MASK_CUR << ij) && (i || !(mb_strong[POS_CUR] || mb_strong[POS_LEFT]))){
	  if(!i)
	    clip_left = mvmasks[POS_LEFT] & (MASK_RIGHT << j) ? clip[POS_LEFT] : 0;
	  else
	    clip_left = y_to_deblock & (MASK_CUR << (ij-1)) ? clip[POS_CUR] : 0;
	  rv40_v_loop_filter(Y, s->linesize, dither,
			     clip_cur,
			     clip_left,
			     alpha, beta, betaY, 0, 0);
	}
	// filter top edge of the current macroblock when filtering strength is high
	if(!j && y_h_deblock & (MASK_CUR << i) && (mb_strong[POS_CUR] || mb_strong[POS_TOP])){
	  rv40_h_loop_filter(Y, s->linesize, dither,
			     clip_cur,
			     mvmasks[POS_TOP] & (MASK_TOP << i) ? clip[POS_TOP] : 0,
			     alpha, beta, betaY, 0, 1);
	}
	// filter left block edge in edge mode (with high filtering strength)
	if(y_v_deblock & (MASK_CUR << ij) && !i && (mb_strong[POS_CUR] || mb_strong[POS_LEFT])){
	  clip_left = mvmasks[POS_LEFT] & (MASK_RIGHT << j) ? clip[POS_LEFT] : 0;
	  rv40_v_loop_filter(Y, s->linesize, dither,
			     clip_cur,
			     clip_left,
			     alpha, beta, betaY, 0, 1);
	}
      }
    }
    for(k = 0; k < 2; k++){
      for(j = 0; j < 2; j++){
	C = s->current_picture_ptr->data[k+1] + mb_x*8 + (row*8 + j*4) * s->uvlinesize;
	for(i = 0; i < 2; i++, C += 4){
	  int ij = i + j*2;
	  int clip_cur = c_to_deblock[k] & (MASK_CUR << ij) ? clip[POS_CUR] : 0;
	  if(c_h_deblock[k] & (MASK_CUR << (ij+2))){
	    int clip_bot = c_to_deblock[k] & (MASK_CUR << (ij+2)) ? clip[POS_CUR] : 0;
	    rv40_h_loop_filter(C+4*s->uvlinesize, s->uvlinesize, i*8,
			       clip_bot,
			       clip_cur,
			       alpha, beta, betaC, 1, 0);
	  }
	  if((c_v_deblock[k] & (MASK_CUR << ij)) && (i || !(mb_strong[POS_CUR] || mb_strong[POS_LEFT]))){
	    if(!i)
	      clip_left = uvcbp[POS_LEFT][k] & (MASK_CUR << (2*j+1)) ? clip[POS_LEFT] : 0;
	    else
	      clip_left = c_to_deblock[k]    & (MASK_CUR << (ij-1))  ? clip[POS_CUR]  : 0;
	    rv40_v_loop_filter(C, s->uvlinesize, j*8,
			       clip_cur,
			       clip_left,
			       alpha, beta, betaC, 1, 0);
	  }
	  if(!j && c_h_deblock[k] & (MASK_CUR << ij) && (mb_strong[POS_CUR] || mb_strong[POS_TOP])){
	    int clip_top = uvcbp[POS_TOP][k] & (MASK_CUR << (ij+2)) ? clip[POS_TOP] : 0;
	    rv40_h_loop_filter(C, s->uvlinesize, i*8,
			       clip_cur,
			       clip_top,
			       alpha, beta, betaC, 1, 1);
	  }
	  if(c_v_deblock[k] & (MASK_CUR << ij) && !i && (mb_strong[POS_CUR] || mb_strong[POS_LEFT])){
	    clip_left = uvcbp[POS_LEFT][k] & (MASK_CUR << (2*j+1)) ? clip[POS_LEFT] : 0;
	    rv40_v_loop_filter(C, s->uvlinesize, j*8,
			       clip_cur,
			       clip_left,
			       alpha, beta, betaC, 1, 1);
	  }
	}
      }
    }
  }
}
#endif

/**
 * RealVideo 3.0/4.0 inverse transform for DC block
 *
 * Code is almost the same as rv34_inv_transform()
 * but final coefficients are multiplied by 1.5 and have no rounding.
 */
static av_always_inline void rv40_inv_transform_noround(DCTELEM *block){
  int temp[16];
  int i;

  S32I2M(xr11,13);
  S32I2M(xr12,((7<<16)|17));
  uint32_t src=block-1;
  uint32_t dst=temp-4;
  for(i=0; i<4; i++){
#if 1
    S16LDI(xr1,src,0x2,2);
    S16LDD(xr2,src,0x10,2);
    S16LDD(xr3,src,0x8,3);
    D32ADD_AS(xr1,xr1,xr2,xr2);
    S32SFL(xr0,xr1,xr2,xr2,3);
    D16MUL_LW(xr1,xr11,xr2,xr2);
    S16LDD(xr5,src,0x18,3);

    D16MUL_WW(xr3,xr3,xr12,xr4);
    D16MUL_WW(xr5,xr5,xr12,xr6);
    D32ASUM_SA(xr3,xr6,xr5,xr4);

    D32ADD_AS(xr1,xr1,xr4,xr4);
    D32ADD_AS(xr2,xr2,xr3,xr3);
    S32SDI(xr1,dst,0x10);
    S32STD(xr2,dst,0x4);
    S32STD(xr3,dst,0x8);
    S32STD(xr4,dst,0xc);
#else
    const int z0= 13*(block[i+4*0] +    block[i+4*2]);
    const int z1= 13*(block[i+4*0] -    block[i+4*2]);
    const int z2=  7* block[i+4*1] - 17*block[i+4*3];
    const int z3= 17* block[i+4*1] +  7*block[i+4*3];

    temp[4*i+0]= z0+z3;
    temp[4*i+1]= z1+z2;
    temp[4*i+2]= z1-z2;
    temp[4*i+3]= z0-z3;
#endif
  }
  for(i=0; i<4; i++){
    const int z0= 13*(temp[4*0+i] +    temp[4*2+i]);
    const int z1= 13*(temp[4*0+i] -    temp[4*2+i]);
    const int z2=  7* temp[4*1+i] - 17*temp[4*3+i];
    const int z3= 17* temp[4*1+i] +  7*temp[4*3+i];
    block[i*4+0]= ((z0 + z3)*3)>>11;
    block[i*4+1]= ((z1 + z2)*3)>>11;
    block[i*4+2]= ((z1 - z2)*3)>>11;
    block[i*4+3]= ((z0 - z3)*3)>>11;
  }

}

/** @} */ // transform


/**
 * @defgroup block RV30/40 4x4 block decoding functions
 * @{
 */

/**
 * Decode coded block pattern.
 */

#ifdef JZC_CCBP_TBL_OPT
typedef struct
{
unsigned char dsc_to_ccbp1;
unsigned char dsc_to_cmask1;
unsigned char dsc_to_clen1;
} CCBP_TBL1;

CCBP_TBL1 dsc_lookup[81];
unsigned char getb_lookup[256];
#endif//JZC_CCBP_TBL_OPT

static av_always_inline int rv40_decode_cbp(GetBitContext *gb, RV34VLC *vlc, int table)
{

  int pattern, code, cbp=0;
  int ones;
#ifdef JZC_VLC_HW_OPT
  CPU_SET_CTRL(0x7F);
#endif
  static const int cbp_masks[3] = {0x100000, 0x010000, 0x110000};
  static const int shifts[4] = { 0, 2, 8, 10 };
  const int *curshift = shifts;
  int i, t, mask;

#ifdef JZC_VLC_HW_OPT
  code = get_vlc2_fast(vlc->cbppattern[table].table, 8, 2);
#else
  code = get_vlc2(gb, vlc->cbppattern[table].table, 8, 2);
#endif
  pattern = code & 0xF;
  code >>= 4;

  ones = rv34_count_ones[pattern];
#ifdef JZC_VLC_HW_OPT
  int ctrl=(vlc->cbp[table][ones].bits-1<<4)|0xf;
  CPU_SET_CTRL(ctrl);
#endif
  for(mask = 8; mask; mask >>= 1, curshift++){
    if(pattern & mask){
#ifdef JZC_VLC_HW_OPT
      cbp |= get_vlc2_fast(vlc->cbp[table][ones].table, vlc->cbp[table][ones].bits, 1) << curshift[0];
      CPU_SET_CTRL(ctrl);
#else
      cbp |= get_vlc2(gb, vlc->cbp[table][ones].table, vlc->cbp[table][ones].bits, 1) << curshift[0];
#endif
    }
  }

#ifdef JZC_CCBP_TBL_OPT
    uint32_t ccbp_part1=dsc_lookup[code].dsc_to_ccbp1;
    uint32_t getb_low4=dsc_lookup[code].dsc_to_cmask1;
    uint32_t getb_len=dsc_lookup[code].dsc_to_clen1;
    /*
    fprintf(ccbp_tab,"d=%d\nccbp_part1=0x%x, getb_low4=0x%x, getb_len=0x%x\n",
	   d, ccbp_part1, getb_low4, getb_len);
    */
    uint32_t cc_bs=getb_len?get_bits(gb,getb_len):0;
    
    getb_low4|=cc_bs<<4;
    ccbp_part1|=getb_lookup[getb_low4];
    cbp|=ccbp_part1<<16;
    /*    fprintf(ccbp_tab,"cbp=0x%x\nc=0x%x\t getb_low4=0x%x\t ccbp_part1=0x%x\n",
	  cbp,c, getb_low4, ccbp_part1);*/
#else
  for(i = 0; i < 4; i++){
    t = modulo_three_table[code][i];
    if(t == 1)
      cbp |= cbp_masks[get_bits1(gb)] << i;
    if(t == 2)
      cbp |= cbp_masks[2] << i;
  }
#endif//#ifdef JZC_CCBP_TBL_OPT

  return cbp;
}

/**
 * Get one coefficient value from the bistream and store it.
 */
#ifdef JZC_VLC_HW_OPT
static uint32_t vlc_ctrl;
#endif
static av_always_inline void decode_coeff(DCTELEM *dst, int coef, int esc, GetBitContext *gb, VLC* vlc)
{
  if(coef){
    if(coef == esc){
#ifdef JZC_VLC_HW_OPT
      CPU_SET_CTRL(vlc_ctrl);
      coef=CPU_GET_RSLT();
      coef=CPU_GET_RSLT();
#else
      coef = get_vlc2(gb, vlc->table, 8, 2);
#endif
      if(coef > 23){
	coef -= 23;
	coef = 22 + ((1 << coef) | get_bits(gb, coef));
      }
      coef += esc;
    }
    if(get_bits1(gb))
      coef = -coef;
    *dst = coef;
  }
}

/**
 * Decode 2x2 subblock of coefficients.
 */
static av_always_inline void decode_subblock(DCTELEM *dst, int code, const int is_block2, GetBitContext *gb, VLC *vlc)
{
  int coeffs[4];

  coeffs[0] = modulo_three_table[code][0];
  coeffs[1] = modulo_three_table[code][1];
  coeffs[2] = modulo_three_table[code][2];
  coeffs[3] = modulo_three_table[code][3];
  decode_coeff(dst  , coeffs[0], 3, gb, vlc);
  if(is_block2){
    decode_coeff(dst+4, coeffs[1], 2, gb, vlc);
    decode_coeff(dst+1, coeffs[2], 2, gb, vlc);
  }else{
    decode_coeff(dst+1, coeffs[1], 2, gb, vlc);
    decode_coeff(dst+4, coeffs[2], 2, gb, vlc);
  }
  decode_coeff(dst+5, coeffs[3], 2, gb, vlc);
}

/**
 * Decode coefficients for 4x4 block.
 *
 * This is done by filling 2x2 subblocks with decoded coefficients
 * in this order (the same for subblocks and subblock coefficients):
 *  o--o
 *    /
 *   /
 *  o--o
 */
#ifdef JZC_VLC_HW_OPT
uint32_t *sec_pat_ctrl;
uint32_t sec_pat_ctrl_intra[2],sec_pat_ctrl_inter[2];
#endif
static av_always_inline void rv40_decode_block(DCTELEM *dst, GetBitContext *gb, RV34VLC *rvlc, int fc, int sc, int n)
{
  int code, pattern;

#ifdef JZC_VLC_HW_OPT
  code = get_vlc2_fast(rvlc->first_pattern[fc].table, 8, 2);
#else
  code = get_vlc2(gb, rvlc->first_pattern[fc].table, 8, 2);
#endif

  pattern = code & 0x7;
#if 0
  if(!pattern & 3)
    dMB->dq_len[n] = 1;
  else  
    dMB->dq_len[n] = 2;
#else  
  if(!(pattern & 3))
    dMB->dq_len[n] = 2;
  else if(!(pattern & 5) && (pattern & 2))
    dMB->dq_len[n] = 1;
  else
    dMB->dq_len[n] = 3;
#endif
  code >>= 3;
  decode_subblock(dst, code, 0, gb, &rvlc->coefficient);

  if(pattern & 4){
#ifdef JZC_VLC_HW_OPT
    CPU_SET_CTRL(sec_pat_ctrl[sc]);
    code=CPU_GET_RSLT();
    code=CPU_GET_RSLT();
#else
    code = get_vlc2(gb, rvlc->second_pattern[sc].table, 8, 2);
#endif
    decode_subblock(dst + 2, code, 0, gb, &rvlc->coefficient);
  }
  if(pattern & 2){ // Looks like coefficients 1 and 2 are swapped for this block
#ifdef JZC_VLC_HW_OPT
    CPU_SET_CTRL(sec_pat_ctrl[sc]);
    code=CPU_GET_RSLT();
    code=CPU_GET_RSLT();
#else
    code = get_vlc2(gb, rvlc->second_pattern[sc].table, 8, 2);
#endif
    decode_subblock(dst + 4*2, code, 1, gb, &rvlc->coefficient);
  }
  if(pattern & 1){
    code = get_vlc2(gb, rvlc->third_pattern[sc].table, 8, 2);
    decode_subblock(dst + 4*2+2, code, 0, gb, &rvlc->coefficient);
  }

}

/**
 * Dequantize 4x4 block of DC values for 16x16 macroblock.
 * @todo optimize
 */
static av_always_inline void rv40_dequant4x4_16x16(DCTELEM *block, int Qdc, int Q)
{
  int i;
#if 0
  const uint8_t rv40_dezigzag[16] = {
    0,  1,  4,  8,
    5,  2,  3,  6,
    9, 12, 13, 10,
    7, 11, 14, 15
  };
  for(i = 0; i < 3; i++)
    block[rv40_dezigzag[i]] = (block[rv40_dezigzag[i]] * Qdc + 8) >> 4;
  for(; i < 16; i++)
    block[rv40_dezigzag[i]] = (block[rv40_dezigzag[i]] * Q + 8) >> 4;
#else
  S16LDD(xr11,&Q,0,0);
  S16LDD(xr11,&Qdc,0,1);
  S32I2M(xr12,0x8);
  S32LDD(xr1,block,0x0);
  D16MUL_HW(xr3,xr11,xr1,xr4);
  S32LDD(xr2,block,0x4);
  D32ASUM_AA(xr3,xr12,xr12,xr4);
  D16MUL_LW(xr5,xr11,xr2,xr6);
  D32SARL(xr3,xr3,xr4,4);
  S32LDD(xr1,block,0x8);
  D32ASUM_AA(xr5,xr12,xr12,xr6);
  S32STD(xr3,block,0x0);

  D16MUL_XW(xr3,xr11,xr1,xr4);
  S32LDD(xr2,block,0xc);
  D32SARL(xr5,xr5,xr6,4);
  S32STD(xr5,block,0x4);
  D32ASUM_AA(xr3,xr12,xr12,xr4);

  D16MUL_LW(xr5,xr11,xr2,xr6);
  S32LDD(xr1,block,0x10);
  D32SARL(xr3,xr3,xr4,4);
  D32ASUM_AA(xr5,xr12,xr12,xr6);
  S32STD(xr3,block,0x8);

  D16MUL_LW(xr3,xr11,xr1,xr4);
  S32LDD(xr2,block,0x14);
  D32SARL(xr5,xr5,xr6,4);
  D32ASUM_AA(xr3,xr12,xr12,xr4);
  S32STD(xr5,block,0x0c);

  D16MUL_LW(xr5,xr11,xr2,xr6);
  S32LDD(xr1,block,0x18);
  D32SARL(xr3,xr3,xr4,4);
  D32ASUM_AA(xr5,xr12,xr12,xr6);
  S32STD(xr3,block,0x10);

  D16MUL_LW(xr3,xr11,xr1,xr4);
  S32LDD(xr2,block,0x1c);
  D32SARL(xr5,xr5,xr6,4);
  D32ASUM_AA(xr3,xr12,xr12,xr4);
  S32STD(xr5,block,0x14);

  D16MUL_LW(xr5,xr11,xr2,xr6);
  D32SARL(xr3,xr3,xr4,4);
  D32ASUM_AA(xr5,xr12,xr12,xr6);
  S32STD(xr3,block,0x18);

  D32SARL(xr5,xr5,xr6,4);
  S32STD(xr5,block,0x1c);
#endif
}

/**
 * Decode starting slice position.
 * @todo Maybe replace with ff_h263_decode_mba() ?
 */
int inline ff_rv40_get_start_offset(GetBitContext *gb, int mb_size)
{
  int i;
  for(i = 0; i < 5; i++)
    if(rv34_mb_max_sizes[i] >= mb_size - 1)
      break;
  return rv34_mb_bits_sizes[i];
}

/**
 * Select VLC set for decoding from current quantizer, modifier and frame type.
 */
#ifdef JZC_VLC_HW_OPT
#else
static inline RV34VLC* choose_vlc_set(int quant, int mod, int type)
{
  if(mod == 2 && quant < 19) quant += 10;
  else if(mod && quant < 26) quant += 5;
  return type ? &inter_vlcs[rv34_quant_to_vlc_set[1][av_clip(quant, 0, 30)]]
    : &intra_vlcs[rv34_quant_to_vlc_set[0][av_clip(quant, 0, 30)]];
}
/**
 * Decode quantizer difference and return modified quantizer.
 */
static inline int rv40_decode_dquant(GetBitContext *gb, int quant)
{
  if(get_bits1(gb))
    return rv34_dquant_tab[get_bits1(gb)][quant];
  else
    return get_bits(gb, 5);
}
#endif

/** macroblock partition width in 8x8 blocks */
static const uint8_t part_sizes_w[RV34_MB_TYPES] = { 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2 };

/** macroblock partition height in 8x8 blocks */
static const uint8_t part_sizes_h[RV34_MB_TYPES] = { 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2 };

/** availability index for subblocks */
static const uint8_t avail_indexes[4] = { 6, 7, 10, 11 };

/**
 * motion vector prediction
 *
 * Motion prediction performed for the block by using median prediction of
 * motion vectors from the left, top and right top blocks but in corner cases
 * some other vectors may be used instead.
 */
static av_always_inline void rv40_pred_mv(RV34DecContext *r, int block_type, int subblock_no, int dmv_no)
{
  MpegEncContext *s = &r->s;
  int mv_pos = s->mb_x * 2 + s->mb_y * 2 * s->b8_stride;
  int A[2] = {0}, B[2], C[2];
  int i, j;
  int mx, my;
  int avail_index = avail_indexes[subblock_no];
  int c_off = part_sizes_w[block_type];

  mv_pos += (subblock_no & 1) + (subblock_no >> 1)*s->b8_stride;
  if(subblock_no == 3)
    c_off = -1;

  int *cur_mv=s->current_picture_ptr->motion_val[0][mv_pos];
#if 0
  if(r->avail_cache[avail_index - 1]){
    A[0] = cur_mv[-1][0];//s->current_picture_ptr->motion_val[0][mv_pos-1][0];
    A[1] = cur_mv[-1][1];//s->current_picture_ptr->motion_val[0][mv_pos-1][1];
  }
  if(r->avail_cache[avail_index - 4]){
    B[0] = cur_mv[-s->b8_stride][0];//s->current_picture_ptr->motion_val[0][mv_pos-s->b8_stride][0];
    B[1] = cur_mv[-s->b8_stride][1];//s->current_picture_ptr->motion_val[0][mv_pos-s->b8_stride][1];
  }else{
    B[0] = A[0];
    B[1] = A[1];
  }
  if(!r->avail_cache[avail_index - 4 + c_off]){
    if(r->avail_cache[avail_index - 4] && (r->avail_cache[avail_index - 1])){
      C[0] = cur_mv[-s->b8_stride-1][0];//s->current_picture_ptr->motion_val[0][mv_pos-s->b8_stride-1][0];
      C[1] = cur_mv[-s->b8_stride-1][1];//s->current_picture_ptr->motion_val[0][mv_pos-s->b8_stride-1][1];
    }else{
      C[0] = A[0];
      C[1] = A[1];
    }
  }else{
    C[0] = cur_mv[-s->b8_stride+c_off][0];//s->current_picture_ptr->motion_val[0][mv_pos-s->b8_stride+c_off][0];
    C[1] = cur_mv[-s->b8_stride+c_off][1];//s->current_picture_ptr->motion_val[0][mv_pos-s->b8_stride+c_off][1];
  }
  mx = mid_pred(A[0], B[0], C[0]);
  my = mid_pred(A[1], B[1], C[1]);
  mx += r->dmv[dmv_no][0];
  my += r->dmv[dmv_no][1];
  int aa=mx;
  int bb=my;
  for(j = 0; j < part_sizes_h[block_type]; j++){
    for(i = 0; i < part_sizes_w[block_type]; i++){
      //      s->current_picture_ptr->motion_val[0][mv_pos + i + j*s->b8_stride][0] = mx;
      //      s->current_picture_ptr->motion_val[0][mv_pos + i + j*s->b8_stride][1] = my;
      cur_mv[i + j*s->b8_stride][0]=mx;
      cur_mv[i + j*s->b8_stride][1]=my;
    }
  }
#else
  D32SLL(xr1,xr0,xr0,xr0,1);//xr1=A=0;
  if(r->avail_cache[avail_index - 1]){
    S32LDD(xr1,cur_mv,-4);//xr1=mv_left
  }
  S32OR(xr2,xr1,xr0);
  S32OR(xr3,xr1,xr0);
  if(r->avail_cache[avail_index - 4]){
    S32LDDV(xr2,cur_mv,-s->b8_stride,2);//xr2=mv_above
  }else{
  }
  if(!r->avail_cache[avail_index - 4 + c_off]){
    if(r->avail_cache[avail_index - 4] && (r->avail_cache[avail_index - 1])){
      S32LDDV(xr3,cur_mv,-s->b8_stride-1,2);//xr3=mv_above
    }else{
    }
  }else{
    S32LDDV(xr3,cur_mv,-s->b8_stride+c_off,2);//xr3=mv_above
  }

  D16MAX(xr4,xr1,xr2);//xr4=max of (A,B)
  D16MIN(xr5,xr1,xr2);//xr5=min of (A,B)
  D16MIN(xr4,xr4,xr3);//xr4:comp max(A,B) and C, get rid of the MAX of (A,B,C)
  D16MAX(xr4,xr4,xr5);//xr4:get rid of the MIN of (A,B,C), xr4=mid

  S32LDDV(xr6,r->dmv,dmv_no,2);
  Q16ADD_AA_WW(xr9,xr4,xr6,xr0);

#if 0
  if(((aa&0xffff)|((bb&0xffff)<<16)) != S32M2I(xr9)){
    printf("r->dmv[dmv_no][0]=0x%x, r->dmv[dmv_no][1]=0x%x, xr7=0x%x,xr8=0x%x,xr9=0x%x,xr6=0x%x\n",
	   r->dmv[dmv_no][0],r->dmv[dmv_no][1],S32M2I(xr7),S32M2I(xr8),S32M2I(xr9),S32M2I(xr6));

    printf("((aa&0xffff)|((bb&0xffff)<<16))=0x%x, aa=0x%x, bb=0x%x, xr4=0x%x, xr5=0x%x\n",
	   ((aa&0xffff)|((bb&0xffff)<<16)),aa,bb,S32M2I(xr4),S32M2I(xr5));
  }
#endif
  /*
  for(j = 0; j < part_sizes_h[block_type]; j++){
    for(i = 0; i < part_sizes_w[block_type]; i++){
      S32STDV(xr9,cur_mv,i + j*s->b8_stride,2);
    }
  }
  */
#endif
}

/**
 * Calculate motion vector component that should be added for direct blocks.
 */

#define GET_PTS_DIFF(a, b) ((a - b + 8192) & 0x1FFF)
/**
 * Predict motion vector for B-frame macroblock.
 */
static av_always_inline void rv40_pred_b_vector(int A[2], int B[2], int C[2],
                                      int A_avail, int B_avail, int C_avail,
                                      int *mx, int *my)
{
  if(A_avail + B_avail + C_avail != 3){
    *mx = A[0] + B[0] + C[0];
    *my = A[1] + B[1] + C[1];
    if(A_avail + B_avail + C_avail == 2){
      *mx /= 2;
      *my /= 2;
    }
  }else{
    *mx = mid_pred(A[0], B[0], C[0]);
    *my = mid_pred(A[1], B[1], C[1]);
  }
}

/**
 * motion vector prediction for B-frames
 */
static inline void rv40_pred_mv_b(RV34DecContext *r, int block_type, int dir)
{
  MpegEncContext *s = &r->s;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  int mv_pos = s->mb_x * 2 + s->mb_y * 2 * s->b8_stride;
  int has_A = 0, has_B = 0, has_C = 0;
  int i, j;
  Picture *cur_pic = s->current_picture_ptr;
  const int mask = dir ? MB_TYPE_L1 : MB_TYPE_L0;
  int type = cur_pic->mb_type[mb_pos];

  int *cur_mv=cur_pic->motion_val[dir][mv_pos];

  D32SLL(xr1,xr0,xr0,xr2,1);//xr1=A,xr2=B
  D32SLL(xr3,xr0,xr0,xr0,1);//xr3=C
  if((r->avail_cache[6-1] & type) & mask){
    S32LDD(xr1,cur_mv,-4);//xr1=mv_left
    has_A = 1;
  }
  if((r->avail_cache[6-4] & type) & mask){
    S32LDDV(xr2,cur_mv,-s->b8_stride,2);//xr2=mv_above
    has_B = 1;
  }
  if(r->avail_cache[6-4] && (r->avail_cache[6-2] & type) & mask){
    S32LDDV(xr3,cur_mv,-s->b8_stride+2,2);//xr3=mv_dia
    has_C = 1;
  }else if((s->mb_x+1) == s->mb_width && (r->avail_cache[6-5] & type) & mask){
    S32LDDV(xr3,cur_mv,-s->b8_stride-1,2);//xr3=mv_dia
    has_C = 1;
  }

  if(has_A + has_B + has_C != 3){
    Q16ACC_AA(xr1,xr2,xr3,xr0);//xr1=A+B+C
    if(has_A + has_B + has_C == 2){
      Q16SLR(xr4,xr1,xr0,xr0,15);//xr4=sign
      S32AND(xr5,xr1,xr4);//xr5=minus&odds
      D16AVG(xr1,xr5,xr1);//xr1=xr1/2
    }
  }else{
    D16MAX(xr4,xr1,xr2);//xr4=max of (A,B)
    D16MIN(xr5,xr1,xr2);//xr5=min of (A,B)
    D16MIN(xr4,xr4,xr3);//xr4:comp max(A,B) and C, get rid of the MAX of (A,B,C)
    D16MAX(xr1,xr4,xr5);//xr1:get rid of the MIN of (A,B,C), xr1=mid
  }

  S32LDDV(xr6,r->dmv,dir,2);
  Q16ADD_AA_WW(xr9,xr1,xr6,xr0);

  S32STD(xr9,cur_mv,0);
  S32STD(xr9,cur_mv,4);
  S32STDV(xr9,cur_mv,s->b8_stride,2);
  S32STDV(xr9,cur_mv,1 + s->b8_stride,2);
  
}

static const int chroma_coeffs[3] = { 0, 3, 5 };
/**
 * generic motion compensation function
 *
 * @param r decoder context
 * @param block_type type of the current block
 * @param xoff horizontal offset from the start of the current block
 * @param yoff vertical offset from the start of the current block
 * @param mv_off offset to the motion vector information
 * @param width width of the current partition in 8x8 blocks
 * @param height height of the current partition in 8x8 blocks
 * @param dir motion compensation direction (i.e. from the last or the next reference frame)
 * @param thirdpel motion vectors are specified in 1/3 of pixel
 * @param qpel_mc a set of functions used to perform luma motion compensation
 * @param chroma_mc a set of functions used to perform chroma motion compensation
 */
#if 0
static inline void rv40_mc_aux(RV34DecContext *r, const int block_type,
			       const int xoff, const int yoff, int mv_off,
			       const int width, const int height, int dir,           
			       qpel_mc_func (*qpel_mc)[16],
			       h264_chroma_mc_func (*chroma_mc))
{
  MpegEncContext *s = &r->s;
  uint8_t *Y, *U, *V, *srcY, *srcU, *srcV;
  int dxy, mx, my, umx, umy, lx, ly, uvmx, uvmy, src_x, src_y, uvsrc_x, uvsrc_y;
  int is16x16 = 1;

  int cx, cy;
  mx = dMB->motion_val[dir][mv_off][0] >> 2;
  my = dMB->motion_val[dir][mv_off][1] >> 2;
  lx = dMB->motion_val[dir][mv_off][0] & 3;
  ly = dMB->motion_val[dir][mv_off][1] & 3;
  cx = dMB->motion_val[dir][mv_off][0] / 2;
  cy = dMB->motion_val[dir][mv_off][1] / 2;	
  //printf("222mx = %x, my = %x\n",dMB->motion_val[dir][mv_off][0],dMB->motion_val[dir][mv_off][1]);

  umx = cx >> 2;
  umy = cy >> 2;
  uvmx = (cx & 3) << 1;
  uvmy = (cy & 3) << 1;
  //due to some flaw RV40 uses the same MC compensation routine for H2V2 and H3V3
  if(uvmx == 6 && uvmy == 6)
    uvmx = uvmy = 4;
    
  dxy = ly*4 + lx;
#if 0
  srcY = dir?dSlice->next_data[0]:dSlice->last_data[0];
  srcU = dir?dSlice->next_data[1]:dSlice->last_data[1];
  srcV = dir?dSlice->next_data[2]:dSlice->last_data[2];
#else
  srcY = dir ? s->next_picture_ptr->data[0] : s->last_picture_ptr->data[0];
  srcU = dir ? s->next_picture_ptr->data[1] : s->last_picture_ptr->data[1];
  srcV = dir ? s->next_picture_ptr->data[2] : s->last_picture_ptr->data[2];
#endif
  src_x = dMB->mb_x * 16 + xoff + mx;
  src_y = dMB->mb_y * 16 + yoff + my;
  uvsrc_x = dMB->mb_x * 8 + (xoff >> 1) + umx;
  uvsrc_y = dMB->mb_y * 8 + (yoff >> 1) + umy;
  srcY += src_y * dSlice->linesize + src_x;
  srcU += uvsrc_y * dSlice->uvlinesize + uvsrc_x;
  srcV += uvsrc_y * dSlice->uvlinesize + uvsrc_x;
  if(   (unsigned)(src_x - !!lx*2) > dSlice->h_edge_pos - !!lx*2 - (width <<3) - 4
	|| (unsigned)(src_y - !!ly*2) > dSlice->v_edge_pos - !!ly*2 - (height<<3) - 4){
    uint8_t *uvbuf= dSlice->edge_emu_buffer + 22 * dSlice->linesize;

    srcY -= 2 + 2*dSlice->linesize;
    ff_emulated_edge_mc(dSlice->edge_emu_buffer, srcY, dSlice->linesize, (width<<3)+6, (height<<3)+6,
			src_x - 2, src_y - 2, dSlice->h_edge_pos, dSlice->v_edge_pos);
    srcY = dSlice->edge_emu_buffer + 2 + 2*dSlice->linesize;
    ff_emulated_edge_mc(uvbuf     , srcU, dSlice->uvlinesize, (width<<2)+1, (height<<2)+1,
			uvsrc_x, uvsrc_y, dSlice->h_edge_pos >> 1, dSlice->v_edge_pos >> 1);
    ff_emulated_edge_mc(uvbuf + 16, srcV, dSlice->uvlinesize, (width<<2)+1, (height<<2)+1,
			uvsrc_x, uvsrc_y, dSlice->h_edge_pos >> 1, dSlice->v_edge_pos >> 1);
    srcU = uvbuf;
    srcV = uvbuf + 16;
  }
  Y = s->dest[0] + xoff      + yoff     *s->linesize;
  U = s->dest[1] + (xoff>>1) + (yoff>>1)*s->uvlinesize;
  V = s->dest[2] + (xoff>>1) + (yoff>>1)*s->uvlinesize;

  if(block_type == RV34_MB_P_16x8){
    qpel_mc[1][dxy](Y, srcY, dSlice->linesize);
    Y    += 8;
    srcY += 8;
  }else if(block_type == RV34_MB_P_8x16){
    qpel_mc[1][dxy](Y, srcY, dSlice->linesize);
    Y    += 8 * dSlice->linesize;
    srcY += 8 * dSlice->linesize;
  }
  is16x16 = (block_type != RV34_MB_P_8x8) && (block_type != RV34_MB_P_16x8) && (block_type != RV34_MB_P_8x16);
  qpel_mc[!is16x16][dxy](Y, srcY, dSlice->linesize);
  chroma_mc[2-width]   (U, srcU, dSlice->uvlinesize, height*4, uvmx, uvmy);
  chroma_mc[2-width]   (V, srcV, dSlice->uvlinesize, height*4, uvmx, uvmy);
}

static void rv40_mc_1mv_aux(RV34DecContext *r, const int block_type,
			    const int xoff, const int yoff, int mv_off,
			    const int width, const int height, int dir)
{
  rv40_mc_aux(r, block_type, xoff, yoff, mv_off, width, height, dir,
	      r->s.dsp.put_rv40_qpel_pixels_tab,
	      r->s.dsp.put_rv40_chroma_pixels_tab);
}

static void rv40_mc_2mv_aux(RV34DecContext *r, const int block_type)
{
  rv40_mc_aux(r, block_type, 0, 0, 0, 2, 2, 0,
	      r->s.dsp.put_rv40_qpel_pixels_tab,
	      r->s.dsp.put_rv40_chroma_pixels_tab);
  rv40_mc_aux(r, block_type, 0, 0, 0, 2, 2, 1,
	      r->s.dsp.avg_rv40_qpel_pixels_tab,
	      r->s.dsp.avg_rv40_chroma_pixels_tab);
}

static void rv40_mc_2mv_skip_aux(RV34DecContext *r)
{
  int i, j;
  for(j = 0; j < 2; j++)
    for(i = 0; i < 2; i++){
      rv40_mc_aux(r, RV34_MB_P_8x8, i*8, j*8, i+j*2, 1, 1, 0,
		  r->s.dsp.put_rv40_qpel_pixels_tab,
		  r->s.dsp.put_rv40_chroma_pixels_tab);
      rv40_mc_aux(r, RV34_MB_P_8x8, i*8, j*8, i+j*2, 1, 1, 1,
		  r->s.dsp.avg_rv40_qpel_pixels_tab,
		  r->s.dsp.avg_rv40_chroma_pixels_tab);
    }
}

/**
 * Decode motion vector differences
 * and perform motion vector reconstruction and motion compensation.
 */
static int rv40_decode_mv_aux(RV34DecContext *r, int block_type)
{
  MpegEncContext *s = &r->s;
  GetBitContext *gb = &s->gb;
  int i, j, k, l;
  int next_bt;
  int pbdir;
  switch(block_type){
  case RV34_MB_SKIP:
    if(dSlice->pict_type == FF_P_TYPE){
      rv40_mc_1mv_aux (r, block_type, 0, 0, 0, 2, 2, 0);
      break;
    }
  case RV34_MB_B_DIRECT:
    //surprisingly, it uses motion scheme from next reference frame
    if(!(IS_16X8(dMB->next_bt) || IS_8X16(dMB->next_bt) || IS_8X8(dMB->next_bt))) //we can use whole macroblock MCv
      rv40_mc_2mv_aux(r, block_type);
    else
      rv40_mc_2mv_skip_aux(r);
    break;
  case RV34_MB_P_16x16:
  case RV34_MB_P_MIX16x16:
    rv40_mc_1mv_aux (r, block_type, 0, 0, 0, 2, 2, 0);
    break;
  case RV34_MB_B_FORWARD:
  case RV34_MB_B_BACKWARD:
    rv40_mc_1mv_aux(r, block_type, 0, 0, 0, 2, 2, block_type == RV34_MB_B_BACKWARD);
    break;
  case RV34_MB_P_16x8:
  case RV34_MB_P_8x16:
    if(block_type == RV34_MB_P_16x8){
      rv40_mc_1mv_aux(r, block_type, 0, 0, 0,            2, 1, 0);
      rv40_mc_1mv_aux(r, block_type, 0, 8, 2, 2, 1, 0);
    }
    if(block_type == RV34_MB_P_8x16){
      rv40_mc_1mv_aux(r, block_type, 0, 0, 0, 1, 2, 0);
      rv40_mc_1mv_aux(r, block_type, 8, 0, 1, 1, 2, 0);
    }
    break;
  case RV34_MB_B_BIDIR:
    rv40_mc_2mv_aux(r, block_type);
    break;
  case RV34_MB_P_8x8:
    for(i=0;i< 4;i++){	
      rv40_mc_1mv_aux(r, block_type, (i&1)<<3, (i&2)<<2, i, 1, 1, 0);
    }
    break;
  }    
  return 0;
}
#endif
/** number of motion vectors in each macroblock type */
static const int num_mvs[RV34_MB_TYPES] = { 0, 0, 1, 4, 1, 1, 0, 0, 2, 2, 2, 1 };

static av_always_inline int svq3_get_se_golombl(GetBitContext *gb){
  unsigned int buf,t;
  int log;

  t=get_bits1(gb);
  buf=show_bits(gb,8);
  buf|=(t<<8);

  if(buf&0x155){
    buf >>= 1;
    if(ff_interleaved_golomb_vlc_len[buf]-1) skip_bits(gb,ff_interleaved_golomb_vlc_len[buf]-1);

    return ff_interleaved_se_golomb_vlc_code[buf];
  }else{
    skip_bits(gb,7);
    buf=(buf<<23)| 1 ;

    for(log=31; (buf & 0x80000000) == 0; log--){
      buf = (buf << 2) - ((buf << log) >> (log - 1)) + (buf >> 30);
      log--;
      if(buf & 0x80000000) break;
      buf = (buf << 2) - ((buf << log) >> (log - 1)) + (buf >> 30);
      log--;
      if(buf & 0x80000000) break;
      buf = (buf << 2) - ((buf << log) >> (log - 1)) + (buf >> 30);
      log--;
      if(buf & 0x80000000) break;
      if(log==16) return INVALID_VLC;
      buf = (buf << 2) - ((buf << log) >> (log - 1)) + (buf >> 30);
      if(log<28) skip_bits(gb,8);
      buf|=(show_bits(gb,8)<<24);
    }

    if((63 - 2*log)%8) {skip_bits(gb,(63 - 2*log)%8);}

    return (signed) (((((buf << log) >> log) - 1) ^ -(buf & 0x1)) + 1) >> 1;
  }
}

static av_always_inline int rv40_decode_mv(RV34DecContext *r, int block_type)
{
  MpegEncContext *s = &r->s;
  GetBitContext *gb = &s->gb;
  int i, j, k, l;
  int mv_pos = s->mb_x * 2 + s->mb_y * 2 * s->b8_stride;
  int next_bt;
  int pbdir;

  int midx = mv_pos;
  memset(r->dmv, 0, sizeof(r->dmv));

  for(i = 0; i < num_mvs[block_type]; i++){
    r->dmv[i][0] = svq3_get_se_golomb(gb);
    r->dmv[i][1] = svq3_get_se_golomb(gb);
  }

  int *dmb_mv_fw=(int*)dMB->motion_val[0];
  int *dmb_mv_bw=(int*)dMB->motion_val[1];
  int *mv_fw=(int*)(s->current_picture_ptr->motion_val[0][mv_pos]);
  int *mv_bw=(int*)(s->current_picture_ptr->motion_val[1][mv_pos]);
  short (*ref_mv)[2]=s->next_picture_ptr->motion_val[0][mv_pos];
  switch(block_type){
  case RV34_MB_TYPE_INTRA:
  case RV34_MB_TYPE_INTRA16x16:
    mv_fw[0]=mv_fw[1]=0;
    mv_fw[s->b8_stride]=mv_fw[s->b8_stride+1]=0;
    dmb_mv_fw[0]=0;
    return 0;
  case RV34_MB_SKIP:
    if(s->pict_type == FF_P_TYPE){
      mv_fw[0]=mv_fw[1]=0;
      mv_fw[s->b8_stride]=mv_fw[s->b8_stride+1]=0;
      dmb_mv_fw[0]=0;
      break;
    }
  case RV34_MB_B_DIRECT:
    //surprisingly, it uses motion scheme from next reference frame
    next_bt = s->next_picture_ptr->mb_type[s->mb_x + s->mb_y * s->mb_stride];
    dMB->next_bt = next_bt;
    int refdist = GET_PTS_DIFF(r->next_pts, r->last_pts);

    if(IS_INTRA(next_bt) || IS_SKIP(next_bt) || !refdist){
      mv_bw[0]=mv_bw[1]=0;
      mv_bw[s->b8_stride]=mv_bw[s->b8_stride+1]=0;
      dmb_mv_fw[0] = 0;
      dmb_mv_bw[0] = 0;
    }else{
      for(l = 0; l < 2; l++){
	int dist = l ? -GET_PTS_DIFF(r->next_pts, r->cur_pts) : GET_PTS_DIFF(r->cur_pts, r->last_pts);
	int mul=((dist << 15) / refdist);
	S32I2M(xr1,mul);
	S32LDD(xr2,ref_mv,0);//xr2=refmv[0]
	D16MULF_LW(xr3,xr1,xr2);//xr3=((refmv[0] * mul + 0x4000) >> 15)
	S32LDD(xr4,ref_mv,4);//xr4=refmv[1]

	S32STD(xr3,dmb_mv_fw,0);//store to dmb_mv[l][0]
	D16MULF_LW(xr5,xr1,xr4);//xr5=((refmv[1] * mul + 0x4000) >> 15)
	S32LDDV(xr6,ref_mv,s->b8_stride,2);//xr6=refmv[s->b8_stride]

	S32STD(xr5,dmb_mv_fw,4);//store to dmb_mv[l][1]
	D16MULF_LW(xr7,xr1,xr6);//xr7=((refmv[s->b8_stride] * mul + 0x4000) >> 15)
	S32LDDV(xr8,ref_mv,s->b8_stride+1,2);//xr8=refmv[s->b8_stride+1]

	S32STD(xr7,dmb_mv_fw,8);//store to dmb_mv[l][2]
	D16MULF_LW(xr9,xr1,xr8);//xr9=((refmv[s->b8_stride+1] * mul + 0x4000) >> 15)
	S32STD(xr9,dmb_mv_fw,12);//store to dmb_mv[l][3]

	dmb_mv_fw+=4;
      }
      S32STD(xr3,mv_bw,0);//store to mv_bw[l][0]
      S32STD(xr5,mv_bw,4);//store to mv_bw[l][1]
      S32STDV(xr7,mv_bw,s->b8_stride,2);//store to mv_bw[l][s->b8_stride]
      S32STDV(xr9,mv_bw,s->b8_stride+1,2);//store to mv_bw[l][s->b8_stride+1]
    }
    mv_fw[0]=mv_fw[1]=0;
    mv_fw[s->b8_stride]=mv_fw[s->b8_stride+1]=0;
    break;
  case RV34_MB_P_16x16:
  case RV34_MB_P_MIX16x16:
    rv40_pred_mv(r, block_type, 0, 0);
    S32STD(xr9,mv_fw,0);
    S32STD(xr9,mv_fw,4);
    S32STDV(xr9,mv_fw,s->b8_stride,2);
    S32STDV(xr9,mv_fw,s->b8_stride+1,2);
    S32STD(xr9,dmb_mv_fw,0);
    break;
  case RV34_MB_B_FORWARD:
  case RV34_MB_B_BACKWARD:
    ((int*)r->dmv[1])[0] = ((int*)r->dmv[0])[0];
    pbdir = (block_type == RV34_MB_B_BACKWARD);
    rv40_pred_mv_b  (r, block_type, pbdir);
    dmb_mv_fw[0]=mv_fw[0];
    dmb_mv_bw[0]=mv_bw[0];
    i_movz(mv_fw,mv_bw,pbdir);
    mv_fw[0]=mv_fw[1]=0;
    mv_fw[s->b8_stride]=mv_fw[s->b8_stride+1]=0;
    break;
  case RV34_MB_P_16x8:
  case RV34_MB_P_8x16:
    rv40_pred_mv(r, block_type, 0, 0);
    S32STD(xr9,mv_fw,0);
    S32STD(xr9,dmb_mv_fw,0);
    rv40_pred_mv(r, block_type, 1 + (block_type == RV34_MB_P_16x8), 1);
    S32STDV(xr9,mv_fw,s->b8_stride+1,2);
    if(block_type == RV34_MB_P_16x8){
      mv_fw[1]=mv_fw[0];
      dmb_mv_fw[2]=mv_fw[s->b8_stride]=mv_fw[s->b8_stride+1];
    }
    if(block_type == RV34_MB_P_8x16){
      dmb_mv_fw[1]=mv_fw[1]=mv_fw[s->b8_stride+1];
      mv_fw[s->b8_stride]=mv_fw[0];
    }	
    break;
  case RV34_MB_B_BIDIR:
    rv40_pred_mv_b  (r, block_type, 0);
    rv40_pred_mv_b  (r, block_type, 1);
    dmb_mv_fw[0]=mv_fw[0];
    dmb_mv_bw[0]=mv_bw[0];
    break;
  case RV34_MB_P_8x8:
    for(i=0;i< 4;i++){
      rv40_pred_mv(r, block_type, i, i);
      S32STDV(xr9,mv_fw,s->b8_stride*(i>>1)+(i&1),2);
      S32STDV(xr9,dmb_mv_fw,i,2);
    }
    break;
  }
  // rv34_decode_mv_aux(r,block_type);
  return 0;
}


/** @} */ // mv group

/**
 * @defgroup recons Macroblock reconstruction functions
 * @{
 */
/** mapping of RV30/40 intra prediction types to standard H.264 types */
static const uint8_t ittrans_list[8][9] = {
  {
    DC_128_PRED,DC_128_PRED,DC_128_PRED,DC_128_PRED,DC_128_PRED,
    DC_128_PRED,DC_128_PRED,DC_128_PRED,DC_128_PRED,
  },
  {
    LEFT_DC_PRED, HOR_PRED, HOR_PRED, DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_LEFT_PRED_RV40_NODOWN,
    VERT_RIGHT_PRED, VERT_LEFT_PRED_RV40_NODOWN, HOR_UP_PRED_RV40_NODOWN, HOR_DOWN_PRED,
  },
  {
    TOP_DC_PRED, VERT_PRED, VERT_PRED, DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_LEFT_PRED_RV40_NODOWN,
    VERT_RIGHT_PRED, VERT_LEFT_PRED_RV40_NODOWN, HOR_UP_PRED_RV40_NODOWN, HOR_DOWN_PRED,
  },
  {
    DC_PRED, VERT_PRED, HOR_PRED, DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_LEFT_PRED_RV40_NODOWN,
    VERT_RIGHT_PRED, VERT_LEFT_PRED_RV40_NODOWN, HOR_UP_PRED_RV40_NODOWN, HOR_DOWN_PRED,
  },
  {
    DC_128_PRED,DC_128_PRED,DC_128_PRED,DC_128_PRED,DC_128_PRED,
    DC_128_PRED,DC_128_PRED,DC_128_PRED,DC_128_PRED,
  },
  {
    LEFT_DC_PRED, HOR_PRED, HOR_PRED, DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_LEFT_PRED,
    VERT_RIGHT_PRED, VERT_LEFT_PRED, HOR_UP_PRED, HOR_DOWN_PRED,
  },
  {
    TOP_DC_PRED, VERT_PRED, VERT_PRED, DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_LEFT_PRED_RV40_NODOWN,
    VERT_RIGHT_PRED, VERT_LEFT_PRED, HOR_UP_PRED, HOR_DOWN_PRED,
  },
  {
    DC_PRED, VERT_PRED, HOR_PRED, DIAG_DOWN_RIGHT_PRED, DIAG_DOWN_LEFT_PRED,
    VERT_RIGHT_PRED, VERT_LEFT_PRED, HOR_UP_PRED, HOR_DOWN_PRED,
  },

};
/** mapping of RV30/40 intra 16x16 prediction types to standard H.264 types */
static const uint8_t ittrans16[4][4] = {
  {DC_128_PRED8x8, DC_128_PRED8x8, DC_128_PRED8x8, DC_128_PRED8x8},
  {LEFT_DC_PRED8x8, HOR_PRED8x8, HOR_PRED8x8, HOR_PRED8x8},

  {TOP_DC_PRED8x8, VERT_PRED8x8, VERT_PRED8x8, VERT_PRED8x8},
  {DC_PRED8x8, VERT_PRED8x8, HOR_PRED8x8, PLANE_PRED8x8},
};
static const uint8_t ittrans16_2[4][4] = {
  {DC_128_PRED8x8, DC_128_PRED8x8, DC_128_PRED8x8, DC_128_PRED8x8},
  {LEFT_DC_PRED8x8, HOR_PRED8x8, HOR_PRED8x8, LEFT_DC_PRED8x8},

  {TOP_DC_PRED8x8, VERT_PRED8x8, VERT_PRED8x8, TOP_DC_PRED8x8},
  {DC_PRED8x8, VERT_PRED8x8, HOR_PRED8x8, DC_PRED8x8},
};
/**
 * Perform 4x4 intra prediction.
 */
#if 0
static inline void rv40_pred_4x4_block(RV34DecContext *r,int itype, int up, int left, int down, int right, int num)
{
  int rup = 0;
  if(!up && !left)
    itype = DC_128_PRED;
  else if(!up){
    if(itype == VERT_PRED) itype = HOR_PRED;
    if(itype == DC_PRED)   itype = LEFT_DC_PRED;
  }else if(!left){
    if(itype == HOR_PRED)  itype = VERT_PRED;
    if(itype == DC_PRED)   itype = TOP_DC_PRED;
    if(itype == DIAG_DOWN_LEFT_PRED) itype = DIAG_DOWN_LEFT_PRED_RV40_NODOWN;
  }
  if(!down){
    if(itype == DIAG_DOWN_LEFT_PRED) itype = DIAG_DOWN_LEFT_PRED_RV40_NODOWN;
    if(itype == HOR_UP_PRED) itype = HOR_UP_PRED_RV40_NODOWN;
    if(itype == VERT_LEFT_PRED) itype = VERT_LEFT_PRED_RV40_NODOWN;
  }
  if(!right && up){
    rup = 1;
  }
  dMB->itype4[num] = itype;
  dMB->rup[num] = rup;
}
static inline int adjust_pred16(int itype, int up, int left)
{
  if(!up && !left)
    itype = DC_128_PRED8x8;
  else if(!up){
    if(itype == PLANE_PRED8x8)itype = HOR_PRED8x8;
    if(itype == VERT_PRED8x8) itype = HOR_PRED8x8;
    if(itype == DC_PRED8x8)   itype = LEFT_DC_PRED8x8;
  }else if(!left){
    if(itype == PLANE_PRED8x8)itype = VERT_PRED8x8;
    if(itype == HOR_PRED8x8)  itype = VERT_PRED8x8;
    if(itype == DC_PRED8x8)   itype = TOP_DC_PRED8x8;
  }
  return itype;
}
#endif

static av_always_inline void rv40_output_macroblock(RV34DecContext *r, int8_t *intra_types, int cbp, int is16)
{
  MpegEncContext *s = &r->s;
  DSPContext *dsp = &s->dsp;
  int itype;
  int idx;
  int i,j,num;

  uint8_t *ittrans_ptr;
  
  if(!is16){
    uint8_t avail[6*8] = {0};
    // Set neighbour information.
    if(r->avail_cache[1])
      avail[0] = 1;
    if(r->avail_cache[2])
      avail[1] = avail[2] = 1;
    if(r->avail_cache[3])
      avail[3] = avail[4] = 1;
    if(r->avail_cache[4])
      avail[5] = 1;
    if(r->avail_cache[5])
      avail[8] = avail[16] = 1;
    if(r->avail_cache[9])
      avail[24] = avail[32] = 1;

    fill_rectangle(r->avail_cache + 6, 2, 2, 4, 0, 4);
    idx=5;
    for(j = 0; j < 4; j++){
      idx+=4;
      //idx = 9 + j*8;
      for(i = 0; i < 4; i++,idx++){
	num = (j<<2) + i;
	dMB->itype4[num]=ittrans_list[(avail[idx+7]<<2)|(avail[idx-8]<<1)|(avail[idx-1])][intra_types[i]];
	dMB->rup[num] =(!avail[idx-7]&&avail[idx-8]);
	avail[idx] = 1;
      }
      intra_types += r->intra_types_stride;
    }
    intra_types -= r->intra_types_stride<<2;
    idx=4;
    for(j = 0; j < 2; j++){
      idx+=2;
      //idx = 6 + j*4;
      for(i = 0; i < 2; i++, idx++){
	num = 16 + (j<<1) + i;
	int up=!!r->avail_cache[idx-4];
	int right=!!r->avail_cache[idx-3];
	int left=!!r->avail_cache[idx-1];
	dMB->itype4[num]=ittrans_list[(!(i|j)<<2)|(up<<1)|(left)][intra_types[(i+(j?r->intra_types_stride:0))<<1]];
	dMB->rup[num] = (!right && up);
	r->avail_cache[idx] = 1;
      }
    }
  }else{
    int tbl_i=(!!r->avail_cache[6-4]<<1)|!!r->avail_cache[6-1];
    itype = intra_types[0];
    if(itype!=3)
      dMB->itype4[1] = dMB->itype4[0] = ittrans16[tbl_i][itype];
    else{
      dMB->itype4[0] = ittrans16[tbl_i][itype];
      dMB->itype4[1] =ittrans16_2[tbl_i][itype];
    }
  }
}

/**
 * @addtogroup bitstream
 * Decode macroblock header and return CBP in case of success, -1 otherwise.
 */
#ifdef JZC_VLC_HW_OPT
uint32_t vlc_intra_tbls,vlc_inter_tbls;
#endif
static av_always_inline int rv40_decode_mb_header(RV34DecContext *r, int8_t *intra_types)
{
  MpegEncContext *s = &r->s;
  GetBitContext *gb = &s->gb;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  int i, t;
  int cur_mb_type;
  static int bk_mb_type;

  if(!r->si.type){
    dMB->is16 = get_bits1(gb);
    if(!dMB->is16){
      if(!get_bits1(gb))
	av_log(s->avctx, AV_LOG_ERROR, "Need DQUANT\n");
    }
    dMB->mb_type = s->current_picture_ptr->mb_type[mb_pos]=dMB->is16 ? MB_TYPE_INTRA16x16 : MB_TYPE_INTRA;
    cur_mb_type = r->block_type = dMB->is16 ? RV34_MB_TYPE_INTRA16x16 : RV34_MB_TYPE_INTRA;
#ifdef JZC_DBLK_OPT
    r->mbtype[s->mb_x] = cur_mb_type;
#endif
  }else{
    r->block_type = rv40_decode_mb_info(r,bk_mb_type);
    cur_mb_type = r->block_type;
#ifdef JZC_DBLK_OPT
    r->mbtype[s->mb_x] = cur_mb_type;
#endif
    if(cur_mb_type == -1)
      return -1;
    dMB->mb_type=s->current_picture_ptr->mb_type[mb_pos] = rv40_mb_type_to_lavc[cur_mb_type];

    if(cur_mb_type == RV34_MB_SKIP){
      if(s->pict_type == FF_P_TYPE)
	cur_mb_type = RV34_MB_P_16x16;
      if(s->pict_type == FF_B_TYPE)
	cur_mb_type = RV34_MB_B_DIRECT;
    }
    bk_mb_type=r->mb_type[s->mb_x];
    r->mb_type[s->mb_x]=cur_mb_type;

    dMB->is16 = !!IS_INTRA16x16(dMB->mb_type);
#ifdef JZC_PMON_P0ed
  PMON_ON(rv9vlc);
#endif
    rv40_decode_mv(r, r->block_type);
#ifdef JZC_PMON_P0ed
  PMON_OFF(rv9vlc);
#endif

    if(r->block_type == RV34_MB_SKIP){
      fill_rectangle(intra_types, 4, 4, r->intra_types_stride, 0, sizeof(intra_types[0]));
      //rv40_decode_mv_aux(r,dMB->block_type);
      return 0;
    }
    //rv40_decode_mv_aux(r,dMB->block_type);
    r->chroma_vlc = 1;
    r->luma_vlc   = 0;
  }

  if(IS_INTRA(dMB->mb_type)){
    if(dMB->is16){
      t = get_bits(gb, 2);
      fill_rectangle(intra_types, 4, 4, r->intra_types_stride, t, sizeof(intra_types[0]));
      r->luma_vlc   = 2;
    }else{
      if(rv40_decode_intra_types(r, gb, intra_types) < 0)
	return -1;
      r->luma_vlc   = 1;
    }
    r->chroma_vlc = 0;
#ifdef JZC_VLC_HW_OPT
    vlc_ctrl=CALC_VLC2_CTRL_INTRA();
    sec_pat_ctrl=sec_pat_ctrl_intra;
    r->cur_vlcs = vlc_intra_tbls;
#else
    r->cur_vlcs = choose_vlc_set(r->si.quant, r->si.vlc_set, 0);
#endif
  }else{
    for(i = 0; i < 16; i++)
      intra_types[(i & 3) + (i>>2) * r->intra_types_stride] = 0;
#ifdef JZC_VLC_HW_OPT
    vlc_ctrl=CALC_VLC2_CTRL_INTER();
    sec_pat_ctrl=sec_pat_ctrl_inter;
    r->cur_vlcs = vlc_inter_tbls;
#else
    r->cur_vlcs = choose_vlc_set(r->si.quant, r->si.vlc_set, 1);
#endif
    if(cur_mb_type == RV34_MB_P_MIX16x16){
      dMB->is16 = 1;
      r->chroma_vlc = 1;
      r->luma_vlc   = 2;
#ifdef JZC_VLC_HW_OPT
      vlc_ctrl=CALC_VLC2_CTRL_INTRA();
      sec_pat_ctrl=sec_pat_ctrl_intra;
      r->cur_vlcs = vlc_intra_tbls;
#else
      r->cur_vlcs = choose_vlc_set(r->si.quant, r->si.vlc_set, 0);
#endif
    }
  }
  return rv40_decode_cbp(gb, r->cur_vlcs, dMB->is16);
}

/**
 * @addtogroup recons
 * @{
 */
/**
 * mask for retrieving all bits in coded block pattern
 * corresponding to one 8x8 block
 */
#define LUMA_CBP_BLOCK_MASK 0x33

#define U_CBP_MASK 0x0F0000
#define V_CBP_MASK 0xF00000

static av_always_inline int is_mv_diff_gt_3(int16_t (*motion_val)[2], int step)
{
  int d;
  d = motion_val[0][0] - motion_val[-step][0];
  if(d < -3 || d > 3)
    return 1;
  d = motion_val[0][1] - motion_val[-step][1];
  if(d < -3 || d > 3)
    return 1;
  return 0;
}

#ifdef JZC_DBLK_OPT
static av_always_inline int rv40_set_deblock_coef(RV34DecContext *r)
{
  MpegEncContext *s = &r->s;
  int hmvmask = 0, vmvmask = 0;
  int midx = s->mb_x * 2 + s->mb_y * 2 * s->b8_stride;
  int16_t (*motion_val)[2];  
  int tmp1, tmp2, tmp3, tmp4;

  if (s->pict_type == FF_B_TYPE && ((r->block_type == RV34_MB_SKIP) || (r->block_type == RV34_MB_B_DIRECT))){
    motion_val = &s->next_picture_ptr->motion_val[0][midx];
  }else{
    motion_val = &s->current_picture_ptr->motion_val[0][midx];
  }

  int *mval = motion_val + s->b8_stride; 
  S32LUI(xr15,3,ptn4);         
  S32LDD(xr1,motion_val,-0x4); 
  S32LDD(xr2,motion_val,0x0);  
  S32LDD(xr3,motion_val,0x4);  

  S32LDD(xr7,mval,-0x4); 
  S32LDD(xr8,mval,0x0);  
  S32LDD(xr9,mval,0x4);  
  
  Q16ADD_SS_WW(xr5,xr2,xr1,xr0); 
  Q16ADD_SS_WW(xr6,xr3,xr2,xr0); 
  Q16ADD_SS_WW(xr12,xr8,xr7,xr0); 
  Q16ADD_SS_WW(xr13,xr9,xr8,xr0); 

  D16CPS(xr5,xr5,xr5);   
  D16CPS(xr6,xr6,xr6);   
  D16SLT(xr5,xr15,xr5);
  D16SLT(xr6,xr15,xr6);
  
  D16CPS(xr12,xr12,xr12);   
  D16CPS(xr13,xr13,xr13);   
  D16SLT(xr12,xr15,xr12);
  D16SLT(xr13,xr15,xr13);

  tmp1 = S32M2I(xr5);
  tmp2 = S32M2I(xr6);  
  tmp3 = S32M2I(xr12);
  tmp4 = S32M2I(xr13);  

  if (tmp1) vmvmask |= 0x11;
  if (tmp2) vmvmask |= 0x44;
  if (tmp3) vmvmask |= 0x1100;
  if (tmp4) vmvmask |= 0x4400;

  if (s->mb_y){
    int *mval1 = motion_val - s->b8_stride;    
    S32LDD(xr4,mval1,0x0);
    S32LDD(xr10,mval1,0x4);
    Q16ADD_SS_WW(xr5,xr2,xr4,xr0); 
    Q16ADD_SS_WW(xr6,xr3,xr10,xr0); 
    
    D16CPS(xr5,xr5,xr5);   
    D16CPS(xr6,xr6,xr6);   
    D16SLT(xr5,xr15,xr5);
    D16SLT(xr6,xr15,xr6);
    tmp1 = S32M2I(xr5);
    tmp2 = S32M2I(xr6);  
    if (tmp1) hmvmask |= 0x03;
    if (tmp2) hmvmask |= 0x0c;
  }

  Q16ADD_SS_WW(xr5,xr8,xr2,xr0); 
  Q16ADD_SS_WW(xr6,xr9,xr3,xr0); 

  D16CPS(xr5,xr5,xr5);   
  D16CPS(xr6,xr6,xr6);   
  D16SLT(xr5,xr15,xr5);
  D16SLT(xr6,xr15,xr6);
  tmp2 = S32M2I(xr5);
  tmp3 = S32M2I(xr6);  
  if (tmp2) hmvmask |= 0x300;
  if (tmp3) hmvmask |= 0xc00;

  if(s->first_slice_line)
    hmvmask &= ~0x000F;
  if(!s->mb_x)
    vmvmask &= ~0x1111;
  return hmvmask | vmvmask;
}
#else
static inline int rv40_set_deblock_coef(RV34DecContext *r)
{
  MpegEncContext *s = &r->s;
  int hmvmask = 0, vmvmask = 0, i, j;
  int midx = s->mb_x * 2 + s->mb_y * 2 * s->b8_stride;
  int16_t (*motion_val)[2] = &s->current_picture_ptr->motion_val[0][midx];
  for(j = 0; j < 16; j += 8){
    for(i = 0; i < 2; i++){
      if(is_mv_diff_gt_3(motion_val + i, 1))
	vmvmask |= 0x11 << (j + i*2);
      if((j || s->mb_y) && is_mv_diff_gt_3(motion_val + i, s->b8_stride))
	hmvmask |= 0x03 << (j + i*2);
    }
    motion_val += s->b8_stride;
  }

  if(s->first_slice_line)
    hmvmask &= ~0x000F;
  if(!s->mb_x)
    vmvmask &= ~0x1111;
  return hmvmask | vmvmask;
}
#endif

#ifdef  JZC_DCORE_OPT
static av_always_inline int rv40_decode_macroblock(RV34DecContext *r, int8_t *intra_types)
{
  MpegEncContext *s = &r->s;
  GetBitContext *gb = &s->gb;
  int cbp, cbp2;
  int i, k;
  //DCTELEM block16[16];
  int luma_dc_quant;
  int dist, pos;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
  // Calculate which neighbours are available. Maybe it's worth optimizing too.
  uint32_t *pavail = r->avail_cache - 1;
  for(i=0; i<3; i++){
    S32SDI(xr0,pavail,0x4);
    S32SDI(xr0,pavail,0x4);
    S32SDI(xr0,pavail,0x4);
    S32SDI(xr0,pavail,0x4);
  }

  S32I2M(xr11,1);
  pavail = r->avail_cache;
  S32STD(xr11,pavail,24);
  S32STD(xr11,pavail,28);
  S32STD(xr11,pavail,40);
  S32STD(xr11,pavail,44);

  uint32_t *mtype = s->current_picture_ptr->mb_type + mb_pos;
  dist = (s->mb_x - s->resync_mb_x) + (s->mb_y - s->resync_mb_y) * s->mb_width;
  S32LDD(xr1,mtype,-4);
  mtype-=s->mb_stride;
  S32LDD(xr2,mtype,-4);
  S32LDD(xr3,mtype,0);
  S32LDD(xr4,mtype,4);
  if(s->mb_x && dist){
    S32STD(xr1,pavail,20);
    S32STD(xr1,pavail,36);
  }
  if(dist >= s->mb_width){
    S32STD(xr3,pavail,8);
    S32STD(xr3,pavail,12);
  }
  if(((s->mb_x+1) < s->mb_width) && dist >= s->mb_width - 1)
    S32STD(xr4,pavail,16);
  if(s->mb_x && dist > s->mb_width)
    S32STD(xr2,pavail,4);

  s->qscale = r->si.quant;
  dMB->qscale = s->qscale;
  dist=r->mbtype[s->mb_x];
  cbp = cbp2 = rv40_decode_mb_header(r, intra_types);

#ifdef JZC_DBLK_OPT
  pos = s->mb_x;

  dMB->dcbp_above = r->dcbp[pos];
  dMB->dcbp_left = r->dcbp[pos - 1];
  dMB->mvd_above = r->mvd[pos];
  dMB->mvd_left = r->mvd[pos - 1];
  dMB->mbtype_above = dist;
  dMB->mbtype_left = r->mbtype[pos - 1];

  r->dcbp[pos] = cbp;
  if(s->pict_type == FF_I_TYPE){
      r->mvd[pos] = 0x0;
  }else{
    r->mvd[pos] = rv40_set_deblock_coef(r);
  }
  dMB->mvd = r->mvd[pos];
  dMB->mbtype = r->mbtype[pos];
#endif
  dMB->cbp = cbp;
  s->current_picture_ptr->qscale_table[mb_pos] = s->qscale;

  dMB->yqtable = rv34_qscale_tab[s->qscale];
  dMB->cqtable[0] = rv34_qscale_tab[rv34_chroma_quant[1][s->qscale]];
  dMB->cqtable[1] = rv34_qscale_tab[rv34_chroma_quant[0][s->qscale]];

  if(cbp == -1)
    return -1;

  luma_dc_quant = r->block_type == RV34_MB_P_MIX16x16 ? r->luma_dc_quant_p[s->qscale] : r->luma_dc_quant_i[s->qscale];
  if(dMB->is16){
    //memset(dMB->block16, 0, 32);
#ifdef JZC_VLC_HW_OPT
    CPU_SET_CTRL(0x7F);
#endif
    MXU_SETZERO(dMB->block16,1);
    rv40_decode_block(dMB->block16, gb, r->cur_vlcs, 3, 0, 0);
    rv40_dequant4x4_16x16(dMB->block16, rv34_qscale_tab[luma_dc_quant],rv34_qscale_tab[s->qscale]);
    rv40_inv_transform_noround(dMB->block16);
    /*for(i = 0; i < 16; i++)
      dMB->block16[i] = block16[(i & 3) | ((i & 0xC) << 1)];*/
  }

  for(i = 0; i < 16; i++, cbp >>= 1){
    if(cbp & 1){
#ifdef JZC_VLC_HW_OPT
      CPU_SET_CTRL(0x7F);
#endif
      MXU_SETZERO(dMB->block[0] + (i<<4),1);
      rv40_decode_block(dMB->block[0] + (i<<4), gb, r->cur_vlcs, r->luma_vlc, 0, i);
    }
  }

  if(r->block_type == RV34_MB_P_MIX16x16){
#ifdef JZC_VLC_HW_OPT
    vlc_ctrl=CALC_VLC2_CTRL_INTER();
    sec_pat_ctrl=sec_pat_ctrl_inter;
    r->cur_vlcs = vlc_inter_tbls;
#else
    r->cur_vlcs = choose_vlc_set(r->si.quant, r->si.vlc_set, 1);
#endif
  }
  for(; i < 24; i++, cbp >>= 1){
    if(cbp & 1){
#ifdef JZC_VLC_HW_OPT
      CPU_SET_CTRL(0x7F);
#endif
      MXU_SETZERO(dMB->block[0] + (i<<4),1);
      rv40_decode_block(dMB->block[0] + (i<<4), gb, r->cur_vlcs, r->chroma_vlc, 1,i);
    }
  }
  if(IS_INTRA(dMB->mb_type))
    rv40_output_macroblock(r, intra_types, cbp2, dMB->is16);
  return 0;
}

#if 0
int rv34_internal()
{
  int i, k, blknum, blkoff;
    
  if(dSlice->si_type){     
    //rv40_decode_mv_aux(r, dMB->block_type);
    //if (dMB->block_type == RV34_MB_SKIP)
    //return 0;
  }
  for(k = 0; k < 16; k++, dMB->cbp >>= 1){
    if(!dMB->is16 && !(dMB->cbp & 1)) continue;
    blknum = ((k & 2) >> 1) + ((k & 8) >> 2);
    blkoff = ((k & 1) << 2) + ((k & 4) << 3);
    rv40_dequant4x4(dMB->block[blknum] + blkoff, rv34_qscale_tab[dMB->qscale],rv34_qscale_tab[dMB->qscale]);
    if(dMB->is16) //FIXME: optimize
      dMB->block[blknum][blkoff] = dMB->block16[k];
    rv40_inv_transform(dMB->block[blknum] + blkoff);
  }

  for(; k < 24; k++, dMB->cbp >>= 1){
    if(!(dMB->cbp & 1)) continue;
    blknum = ((k & 4) >> 2) + 4;
    blkoff = ((k & 1) << 2) + ((k & 2) << 4);
    rv40_dequant4x4(dMB->block[blknum] + blkoff, rv34_qscale_tab[rv34_chroma_quant[1][dMB->qscale]],rv34_qscale_tab[rv34_chroma_quant[0][dMB->qscale]]);
    rv40_inv_transform(dMB->block[blknum] + blkoff);
  }
       
  if(IS_INTRA(dMB->mb_type))
    rv40_output_macroblock_aux();
  else
    rv40_apply_differences();    
}
#endif
#else
static int rv34_decode_macroblock(RV34DecContext *r, int8_t *intra_types)
{
  MpegEncContext *s = &r->s;
  GetBitContext *gb = &s->gb;
  int cbp, cbp2;
  int i, blknum, blkoff;
  DCTELEM block16[64];
  int luma_dc_quant;
  int dist;
  int mb_pos = s->mb_x + s->mb_y * s->mb_stride;

  // Calculate which neighbours are available. Maybe it's worth optimizing too.
  memset(r->avail_cache, 0, sizeof(r->avail_cache));
  fill_rectangle(r->avail_cache + 6, 2, 2, 4, 1, 4);
  dist = (s->mb_x - s->resync_mb_x) + (s->mb_y - s->resync_mb_y) * s->mb_width;
  if(s->mb_x && dist)
    r->avail_cache[5] =
      r->avail_cache[9] = s->current_picture_ptr->mb_type[mb_pos - 1];
  if(dist >= s->mb_width)
    r->avail_cache[2] =
      r->avail_cache[3] = s->current_picture_ptr->mb_type[mb_pos - s->mb_stride];
  if(((s->mb_x+1) < s->mb_width) && dist >= s->mb_width - 1)
    r->avail_cache[4] = s->current_picture_ptr->mb_type[mb_pos - s->mb_stride + 1];
  if(s->mb_x && dist > s->mb_width)
    r->avail_cache[1] = s->current_picture_ptr->mb_type[mb_pos - s->mb_stride - 1];

  s->qscale = r->si.quant;
  cbp = cbp2 = rv34_decode_mb_header(r, intra_types);
  r->cbp_luma  [mb_pos] = cbp;
  r->cbp_chroma[mb_pos] = cbp >> 16;
  if(s->pict_type == FF_I_TYPE)
    r->deblock_coefs[mb_pos] = 0xFFFF;
  else
    r->deblock_coefs[mb_pos] = rv34_set_deblock_coef(r) | r->cbp_luma[mb_pos];
  s->current_picture_ptr->qscale_table[mb_pos] = s->qscale;

  if(cbp == -1)
    return -1;

  luma_dc_quant = r->block_type == RV34_MB_P_MIX16x16 ? r->luma_dc_quant_p[s->qscale] : r->luma_dc_quant_i[s->qscale];
  if(dMB->is16){
    memset(block16, 0, sizeof(block16));
    rv34_decode_block(block16, gb, r->cur_vlcs, 3, 0);
    rv34_dequant4x4_16x16(block16, rv34_qscale_tab[luma_dc_quant],rv34_qscale_tab[s->qscale]);
    rv34_inv_transform_noround(block16);
  }

  for(i = 0; i < 16; i++, cbp >>= 1){
    if(!dMB->is16 && !(cbp & 1)) continue;
    blknum = ((i & 2) >> 1) + ((i & 8) >> 2);
    blkoff = ((i & 1) << 2) + ((i & 4) << 3);
    if(cbp & 1)
      rv34_decode_block(s->block[blknum] + blkoff, gb, r->cur_vlcs, r->luma_vlc, 0);
    rv34_dequant4x4(s->block[blknum] + blkoff, rv34_qscale_tab[s->qscale],rv34_qscale_tab[s->qscale]);
    if(dMB->is16) //FIXME: optimize
      s->block[blknum][blkoff] = block16[(i & 3) | ((i & 0xC) << 1)];
    rv34_inv_transform(s->block[blknum] + blkoff);
  }

  int m,n,l;
  for(m = 0; m < 4; m++)
    {
      int16_t *src = s->block[m];
      int16_t *dst = s->block[m];
      for(n = 0; n < 8; n++)
	{
	  for(l = 0; l < 8; l++){
	    //dst[n*8+l] = src[n*8+l];	    
	    printf("%x ",dst[n*8+l]);
	  }
	  printf("\n");
	}
      printf("\n");
    }
  printf("\n");

  if(r->block_type == RV34_MB_P_MIX16x16)
    r->cur_vlcs = choose_vlc_set(r->si.quant, r->si.vlc_set, 1);
  for(; i < 24; i++, cbp >>= 1){
    if(!(cbp & 1)) continue;
    blknum = ((i & 4) >> 2) + 4;
    blkoff = ((i & 1) << 2) + ((i & 2) << 4);
    rv34_decode_block(s->block[blknum] + blkoff, gb, r->cur_vlcs, r->chroma_vlc, 1);
    rv34_dequant4x4(s->block[blknum] + blkoff, rv34_qscale_tab[rv34_chroma_quant[1][s->qscale]],rv34_qscale_tab[rv34_chroma_quant[0][s->qscale]]);
    rv34_inv_transform(s->block[blknum] + blkoff);
  }
  if(IS_INTRA(s->current_picture_ptr->mb_type[mb_pos]))
    rv34_output_macroblock(r, intra_types, cbp2, dMB->is16);
  else
    rv34_apply_differences(r, cbp2);

  return 0;
}
#endif
static av_always_inline int check_slice_end(RV34DecContext *r, MpegEncContext *s)
{
  int bits;
  if(s->mb_y >= s->mb_height){
    return 1;
  }
  if(!s->mb_num_left){
    return 1;
  }
  if(r->s.mb_skip_run > 1){
    return 0;
  }
#ifdef JZC_VLC_HW_OPT
  bits = r->bits - get_bs_index();
#else
  bits = r->bits - get_bits_count(&s->gb);
#endif
#ifdef JZC_VLC_HW_OPT
  if(bits < 0 )
    return 1;
  if(bits < 8)
    if(!show_bits(&s->gb,bits))
      return 1;
#else
  if(bits < 0 || (bits < 8 && !show_bits(&s->gb, bits)))
    {
      return 1;
    }
#endif
  return 0;
}
#ifdef JZC_VLC_HW_OPT
#undef get_bits
#undef get_bits1
#undef show_bits
#undef skip_bits
#undef get_vlc2
#undef svq3_get_ue_golomb
#undef svq3_get_se_golomb
#endif


#ifdef JZC_VLC_HW_OPT
int last_quant,last_vlc_set;
uint8_t last_intra_tbl_sel,last_inter_tbl_sel;
#endif
static int rv40_decode_slice(RV34DecContext *r, int end, const uint8_t* buf, int buf_size)
{
  MpegEncContext *s = &r->s;
  GetBitContext *gb = &s->gb;
  int mb_pos;
  int res;

#ifdef JZC_DCORE_OPT
  unsigned int tcsm0_fifo_rp_lh;
  unsigned int task_fifo_wp_lh;
  unsigned int task_fifo_wp_lh_overlap;
  int tcsm1_fifo_value=*tcsm1_fifo_wp;
#endif

  init_get_bits(&r->s.gb, buf, buf_size*8);
  res = rv40_parse_slice_header(r, gb, &r->si);
  if(res < 0){
    av_log(s->avctx, AV_LOG_ERROR, "Incorrect or unknown slice header\n");
    return -1;
  }

  if ((s->mb_x == 0 && s->mb_y == 0) || s->current_picture_ptr==NULL) {
    if(s->width != r->si.width || s->height != r->si.height){
      av_log(s->avctx, AV_LOG_DEBUG, "Changing dimensions to %dx%d\n", r->si.width,r->si.height);
      MPV_common_end(s);
      s->width  = r->si.width;
      s->height = r->si.height;
      avcodec_set_dimensions(s->avctx, s->width, s->height);
      if(MPV_common_init(s) < 0)
	return -1;
      r->intra_types_stride = s->mb_width*4 + 4;
#if 1//def JZ_LINUX_OS
      r->intra_types_hist = jz4740_alloc_frame_k0(16, r->intra_types_stride * 5 * sizeof(*r->intra_types_hist));
      r->mb_type = jz4740_alloc_frame_k0(16, r->s.mb_stride * r->s.mb_height * sizeof(*r->mb_type)); 
#else
      r->intra_types_hist = av_realloc(r->intra_types_hist, r->intra_types_stride * 5 * sizeof(*r->intra_types_hist));
      r->mb_type = av_realloc(r->mb_type, r->s.mb_stride * r->s.mb_height * sizeof(*r->mb_type));
#endif
      r->intra_types = r->intra_types_hist + r->intra_types_stride;
      
      r->cbp_luma   = av_realloc(r->cbp_luma,   r->s.mb_stride * sizeof(*r->cbp_luma));
      r->cbp_chroma = av_realloc(r->cbp_chroma, r->s.mb_stride * sizeof(*r->cbp_chroma));
      r->deblock_coefs = av_realloc(r->deblock_coefs, r->s.mb_stride * r->s.mb_height * sizeof(*r->deblock_coefs));
#ifdef JZC_DBLK_OPT
#if 1//def JZ_LINUX_OS
      r->mbtype = jz4740_alloc_frame_k0(16, r->s.mb_stride * sizeof(*r->mbtype));
      *r->mbtype++=0;
      r->mvd = jz4740_alloc_frame_k0(16, (r->s.mb_stride+1) * sizeof(*r->mvd));
      *r->mvd++=0;
      r->dcbp = jz4740_alloc_frame_k0(16, (r->s.mb_stride+1) * sizeof(*r->dcbp));
      *r->dcbp++=0;
#else
      r->mbtype = av_realloc(r->mbtype, r->s.mb_stride * sizeof(*r->mbtype));
      r->mvd = av_realloc(r->mvd, r->s.mb_stride * sizeof(*r->mvd));
      r->dcbp = av_realloc(r->dcbp, r->s.mb_stride * sizeof(*r->dcbp)); 
#endif
#endif
    }
    s->pict_type = r->si.type ? r->si.type : FF_I_TYPE;
    if(MPV_frame_start(s, s->avctx) < 0)
      return -1;
    ff_er_frame_start(s);
    r->cur_pts = r->si.pts;
    if(s->pict_type != FF_B_TYPE){
      r->last_pts = r->next_pts;
      r->next_pts = r->cur_pts;
    }
    s->mb_x = s->mb_y = 0;
  }

  r->si.end = end;
  s->qscale = r->si.quant;

#ifdef JZC_DBLK_OPT
  if (do_dblk == 0){
    uint8_t qcif_beta2 = (((s->mb_width<<4)*(s->mb_height<<4))<=(176*144));
    unsigned int tmp;
    unsigned int * node;
    volatile int * dblk_base=(int*)DBLK_BASE;
    //dblk_base[0] = 0x4;           //set end_flag for first polling

    node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN0);
    node[14] = ((0 << 31) |
		(1 << 30) |//TRAN_EVER
		(1 << 29) |//MBOUT_MNTN
		(1 << 28) |//MBOUT_XCHG
		(1 << 27) |//MBOUT_WRAP
		(0 << 26) |//MBIN_MNTN
		(0 << 25) |//MBIN_XCHG
		(0 << 24) |//MBIN_WRAP
		(0 << 23) |//UP_OUT_MNTN
		(0 << 22) |//UP_OUT_INCR
		(1 << 21) |//UP_IN_MNTN
		(0 << 20) |//UP_IN_INCR
		(qcif_beta2 << 3) |
		1//DBLK_EN
		);

    dblk_polling_end();
    dblk_base[1]=2|(1<<15);
    
    dblk_base[0]=((1 << 31) |//update_addr
		  (1 << 30) |//TRAN_EVER
		  (1 << 29) |//MBOUT_MNTN
		  (1 << 28) |//MBOUT_XCHG
		  (0 << 26) |//MBIN_MNTN
		  (0 << 25) |//MBIN_XCHG
		  (0 << 23) |//UP_OUT_MNTN
		  (0 << 22) |//UP_OUT_INCR
		  (1 << 21) |//UP_IN_MNTN
		  (0 << 20) |//UP_IN_INCR
		  1//DBLK_EN
		);
    dblk_polling_end();

    dblk_base[25] = (s->mb_width<<16)|SRAM_DBLKUP_STRD_Y; //y_upi_stride
    dblk_base[24]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_V1+4);
    dblk_base[23]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U1+4);
    dblk_base[22]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y1+4);
    
    dblk_base[18]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_V0+4);
    dblk_base[17]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_U0+4);
    dblk_base[16]=TCSM1_PADDR(TCSM1_DBLK_MBOUT_Y0+4);
    
    dblk_base[15]=SRAM_PADDR(SRAM_DBLKUP_V);
    dblk_base[14]=SRAM_PADDR(SRAM_DBLKUP_U);
    dblk_base[13]=SRAM_PADDR(SRAM_DBLKUP_Y);
    
    node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN1);
    node[14] = ((0 << 31) |
		(1 << 30) |//TRAN_EVER
		(1 << 29) |//MBOUT_MNTN
		(1 << 28) |//MBOUT_XCHG
		(1 << 27) |//MBOUT_WRAP
		(0 << 26) |//MBIN_MNTN
		(0 << 25) |//MBIN_XCHG
		(0 << 24) |//MBIN_WRAP
		(0 << 23) |//UP_OUT_MNTN
		(0 << 22) |//UP_OUT_INCR
		(1 << 21) |//UP_IN_MNTN
		(0 << 20) |//UP_IN_INCR
		(qcif_beta2 << 3) |
		1//DBLK_EN
		);
  }
  do_dblk++;

  if (s->pict_type != FF_B_TYPE){
    dSlice->refqp = r->si.quant;
  }
#endif
  r->bits = buf_size*8;
  s->mb_num_left = r->si.end - r->si.start;
  r->s.mb_skip_run = 0;

  mb_pos = s->mb_x + s->mb_y * s->mb_width;
  if(r->si.start != mb_pos){
    av_log(s->avctx, AV_LOG_ERROR, "Slice indicates MB offset %d, got %d\n", r->si.start, mb_pos);
    s->mb_x = r->si.start % s->mb_width;
    s->mb_y = r->si.start / s->mb_width;
  }
  memset(r->intra_types_hist, -1, r->intra_types_stride * 5 * sizeof(*r->intra_types_hist));
  s->first_slice_line = 1;
  s->resync_mb_x= s->mb_x;
  s->resync_mb_y= s->mb_y;

  dSlice->pict_type = s->pict_type;
  dSlice->si_type = r->si.type;
  dSlice->mb_width = s->mb_width;
  dSlice->mb_height = s->mb_height;
  dSlice->linesize = s->linesize;
  dSlice->uvlinesize = s->uvlinesize;

  dSlice->last_data[0] = get_phy_addr(s->last_picture.data[0]);
  dSlice->last_data[1] = get_phy_addr(s->last_picture.data[1]);
  dSlice->last_data[2] = get_phy_addr(s->last_picture.data[2]);
    
  dSlice->next_data[0] = get_phy_addr(s->next_picture.data[0]);
  dSlice->next_data[1] = get_phy_addr(s->next_picture.data[1]);
  dSlice->next_data[2] = get_phy_addr(s->next_picture.data[2]);

#ifdef JZC_MC_OPT
  if (r->new_slice_p0 == 0){
    SET_TAB1_RLUT(0, ((int)(dSlice->last_data[0])), 0, 0);
    SET_TAB1_RLUT(16,((int)(dSlice->next_data[0])), 0, 0);
    SET_TAB2_RLUT(0, ((int)(dSlice->last_data[1])), 0, 0, 0, 0);
    SET_TAB2_RLUT(16,((int)(dSlice->next_data[1])), 0, 0, 0, 0);
  }
#endif

#ifdef JZC_TEST_OPT
  ydd = s->current_picture.data[2];
  udd = s->current_picture.data[3];
  vdd = udd + (s->mb_height * 8 + EDGE_WIDTH/2) * s->uvlinesize + s->uvlinesize*(EDGE_WIDTH>>1) + (EDGE_WIDTH>>1) + 32;

  dSlice->current_picture.ptr[0] = get_phy_addr(ydd);
  dSlice->current_picture.ptr[1] = get_phy_addr(udd);
  dSlice->current_picture.ptr[2] = get_phy_addr(vdd);
#else
#if 0
  dSlice->current_picture.ptr[0] = get_phy_addr(s->current_picture.data[0]);
  dSlice->current_picture.ptr[1] = get_phy_addr(s->current_picture.data[1]);
  dSlice->current_picture.ptr[2] = get_phy_addr(s->current_picture.data[2]);
#endif
#endif
  dSlice->line_current_picture.y_ptr = get_phy_addr(s->current_picture.data[0]);
  dSlice->line_current_picture.uv_ptr = get_phy_addr(s->current_picture.data[1]);
#ifdef JZC_ROTA90_OPT
  dSlice->line_current_picture.rota_y_ptr = get_phy_addr(s->current_picture.data[2]);
  dSlice->line_current_picture.rota_uv_ptr = get_phy_addr(s->current_picture.data[3]);
#endif

  int i;int ttmp=dSlice;
  for(i=0;i<SLICE_T_CC_LINE;i++){
    i_cache(0x19,ttmp,0);
    ttmp +=32;
  }
#ifdef JZC_VLC_HW_OPT
  int quant=r->si.quant;
  int mod=r->si.vlc_set;
  if(quant!=last_quant||mod!=last_vlc_set)
    { // set table data
      int i;
      uint8_t tbl_sel;
      uint32_t * vlc_table_base;
      uint32_t * hw_table_base;
      last_quant=quant;
      last_vlc_set=mod;
      if(mod == 2 && quant < 19) quant += 10;
      else if(mod && quant < 26) quant += 5;
      // set 1st table
      tbl_sel = rv34_quant_to_vlc_set[0][av_clip(quant, 0, 30)];
      if(tbl_sel!=last_intra_tbl_sel){
	last_intra_tbl_sel=tbl_sel;
	vlc_intra_tbls=&intra_vlcs[tbl_sel];
	hw_table_base = (vlc_base + VLC_TBL_OFST);
	vlc_table_base  = &hw_coef_table[tbl_sel][0];
	for (i=0; i<(cfg_coef_table[tbl_sel].ram_size + 1)/2; i++)
	  hw_table_base[i] = vlc_table_base[i];

	hw_table_base = (vlc_base + VLC_TBL_OFST+0x400);
	vlc_table_base  = &second_pattern_intra_table[tbl_sel<<1][0];
	for (i=0; i<(cfg_sp_intra_table[tbl_sel<<1].ram_size + 1)/2; i++)
	  hw_table_base[i] = vlc_table_base[i];
	sec_pat_ctrl_intra[0]=CALC_SEC_PAT_CTRL(cfg_sp_intra_table[tbl_sel<<1].lvl0_len,0x200);

	hw_table_base = (vlc_base + VLC_TBL_OFST+0x600);
	vlc_table_base  = &second_pattern_intra_table[(tbl_sel<<1)+1][0];
	for (i=0; i<(cfg_sp_intra_table[(tbl_sel<<1)+1].ram_size + 1)/2; i++)
	  hw_table_base[i] = vlc_table_base[i];
	sec_pat_ctrl_intra[1]=CALC_SEC_PAT_CTRL(cfg_sp_intra_table[(tbl_sel<<1)+1].lvl0_len,0x300);
      }
      // set 2nd table
      tbl_sel = rv34_quant_to_vlc_set[1][av_clip(quant, 0, 30)];
      if(tbl_sel!=last_inter_tbl_sel){
	last_inter_tbl_sel=tbl_sel;
	vlc_inter_tbls=&inter_vlcs[tbl_sel];
	hw_table_base = (vlc_base + VLC_TBL_OFST + 0x200);
	vlc_table_base  = &hw_coef_table[tbl_sel+5][0];
	for (i=0; i<(cfg_coef_table[tbl_sel+5].ram_size + 1)/2; i++)
	  hw_table_base[i] = vlc_table_base[i];

	hw_table_base = (vlc_base + VLC_TBL_OFST+0x800);
	vlc_table_base  = &second_pattern_inter_table[tbl_sel<<1][0];
	for (i=0; i<(cfg_sp_inter_table[tbl_sel<<1].ram_size + 1)/2; i++)
	  hw_table_base[i] = vlc_table_base[i];
	sec_pat_ctrl_inter[0]=CALC_SEC_PAT_CTRL(cfg_sp_inter_table[tbl_sel<<1].lvl0_len,0x400);

	hw_table_base = (vlc_base + VLC_TBL_OFST+0xC00);
	vlc_table_base  = &second_pattern_inter_table[(tbl_sel<<1)+1][0];
	for (i=0; i<(cfg_sp_inter_table[(tbl_sel<<1)+1].ram_size + 1)/2; i++)
	  hw_table_base[i] = vlc_table_base[i];
	sec_pat_ctrl_inter[1]=CALC_SEC_PAT_CTRL(cfg_sp_inter_table[(tbl_sel<<1)+1].lvl0_len,0x600);
      }
    }
  sync_hw_bits(gb);
#endif

  while(!check_slice_end(r, s)) {

#ifdef JZC_DCORE_OPT
#ifdef JZC_PMON_P0
  PMON_ON(rv9while);
#endif
    do{
      tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
      task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
      task_fifo_wp_lh_overlap = task_fifo_wp_lh + TASK_BUF_LEN;
    } while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );
#ifdef JZC_PMON_P0
  PMON_OFF(rv9while);
#endif
    dMB = task_fifo_wp;

    if(r->new_slice_p0){
      dMB->new_slice = 1;
      r->new_slice_p0 = 0;
    }else{
      dMB->new_slice = 0;
    }
#endif
    dMB->mb_x = s->mb_x;
    dMB->mb_y = s->mb_y;

#ifdef JZC_PMON_P0
  PMON_ON(rv9vlc);
#endif
    dMB->er_slice = rv40_decode_macroblock(r, r->intra_types + s->mb_x * 4 + 4);
#ifdef JZC_PMON_P0
  PMON_OFF(rv9vlc);
#endif

    if(dMB->er_slice < 0){
      ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, AC_ERROR|DC_ERROR|MV_ERROR);
      return -1;
    }

#ifdef JZC_DBLK_OPT
    int mbpos = s->mb_y * s->mb_stride + s->mb_x;
    int mbtype = s->current_picture_ptr->mb_type[mbpos];
    if(IS_INTRA(mbtype) || IS_SEPARATE_DC(mbtype)){
      r->dcbp[s->mb_x] |=  0xFFFF;
    }
    if(IS_INTRA(mbtype)){
      r->dcbp[s->mb_x] |= (0xFF<<16);
    }
#endif

    if (++s->mb_x == s->mb_width) {
      s->mb_x = 0;
      s->mb_y++;
      memcpy(r->intra_types_hist, r->intra_types+r->intra_types_stride * 3, r->intra_types_stride * sizeof(*r->intra_types_hist));
      //memset(r->intra_types, -1, r->intra_types_stride * 4 * sizeof(*r->intra_types_hist));
    }

#ifdef JZC_DCORE_OPT
    task_fifo_wp += (TASK_BUF_LEN) >> 2;
    //int current_mb_len = (unsigned int)(task_fifo_wp - task_fifo_wp_d1) << 2;
    //*(uint16_t *)task_fifo_wp_d2 = current_mb_len;

    //if ( s->mb_y == 0 && s->mb_x == 0 )
    // *(int *)(TCSM1_VUCADDR(TCSM1_FIRST_MBLEN)) = current_mb_len;

    int reach_tcsm0_end = (((unsigned int)task_fifo_wp & 0x0000FFFF)) > 0x4000-TASK_BUF_LEN;
    if (reach_tcsm0_end)
      task_fifo_wp = (int *)TCSM0_TASK_FIFO;

    //(*tcsm1_fifo_wp)++;
    *tcsm1_fifo_wp=++tcsm1_fifo_value;
#endif
    if(s->mb_x == s->resync_mb_x)
      s->first_slice_line=0;
    s->mb_num_left--;
  }
#ifdef JZC_VLC_HW_OPT
  sync_sw_bits(gb);
#endif

  ff_er_add_slice(s, s->resync_mb_x, s->resync_mb_y, s->mb_x-1, s->mb_y, AC_END|DC_END|MV_END);

  return s->mb_y == s->mb_height;
}

unsigned int abc_table[32]=
{
  0x800000,//0
  0x800000,//1
  0x800000,//2
  0x800000,//3
  0x800000,//4
  0x800000,//5
  0x800000,//6
  0x800000,//7
  0x3800000,//8
  0x3800100,//9
  0x37a0100,//10
  0x4600101,//11
  0x44b0101,//12
  0x43b0101,//13
  0x62f0101,//14
  0x6250101,//15
  0x61d0101,//16
  0x7170201,//17
  0x8120201,//18
  0x80f0201,//19
  0x90d0201,//20
  0x90b0302,//21
  0xa0a0302,//22
  0xa090302,//23
  0xb080402,//24
  0xb070403,//25
  0xc060503,//26
  0xd050503,//27
  0xe040503,//28
  0xf030704,//29
  0x10020805,//30
  0x11010905,//31
};


/** @} */ // recons group end

/**
 * Initialize decoder.
 */
static av_cold int rv40_decode_init(AVCodecContext *avctx)
{

#ifdef JZC_CCBP_TBL_OPT
  {int i;
    for(i=0; i<81; i++) {
      int cc[4];
      int bn;
      dsc_lookup[i].dsc_to_cmask1=0;
      dsc_lookup[i].dsc_to_clen1=0;
      cc[0]=i/27;
      cc[1]=(i%27)/9;
      cc[2]=(i%9)/3;
      cc[3]=i%3;
      dsc_lookup[i].dsc_to_ccbp1=
	((cc[0]==2)?0x11:0)
	|((cc[1]==2)?0x22:0)
	|((cc[2]==2)?0x44:0)
	|((cc[3]==2)?0x88:0);
      for(bn=0;bn<4;bn++){
	if(cc[bn]==1){
	  dsc_lookup[i].dsc_to_cmask1 |= 0x8>>bn;
	  dsc_lookup[i].dsc_to_clen1++;
	}
      }
      /*      fprintf(ccbp_tab,"i=%d\ndsc_to_ccbp1=0x%x, dsc_to_cmask1=0x%x, dsc_to_clen1=0x%x\n",
	     i,
	     dsc_lookup[i].dsc_to_ccbp1,
	     dsc_lookup[i].dsc_to_cmask1,
	     dsc_lookup[i].dsc_to_clen1);
      */
    }

    printf("dsc_lookup[0].dsc_to_ccbp1=0x%x, dsc_to_cmask1=0x%x, dsc_to_clen1=0x%x\n",
	   dsc_lookup[i].dsc_to_ccbp1, dsc_lookup[i].dsc_to_cmask1,dsc_lookup[i].dsc_to_clen1);

    for(i=0; i<256; i++){
      U8 bof;//bit offset
      U8 low4=i&0xf;
      U8 high4=(i>>4)&0xf;
      U8 new4=0;
      U8 sof=0;
      getb_lookup[i]=0;
      for(bof=0;bof<4;bof++){
	if(low4&(1<<bof)){
	  U8 raw=((high4 & (1<<sof))<<(bof-sof)) | ((~high4 & (1<<sof))<<(4+bof-sof));
	  getb_lookup[i] |= (bof==0?raw<<3:(bof==1?raw<<1:(bof==2?raw>>1:(bof==3?raw>>3:0))));
	  sof++;
	}
      }
      //      fprintf(ccbp_tab,"getb_lookup[i]=0x%x\n",getb_lookup[i]);
    }
  }

#endif//#ifdef JZC_CCBP_TBL_OPT

  RV34DecContext *r = avctx->priv_data;

  r->rv30 = 0;
  MpegEncContext *s = &r->s;

#ifdef JZC_CRC_VER
  crc_code_line = 0;
  rvFrame = 0;
#ifdef JZC_ROTA90_OPT
    crc_code_rota = 0;
#endif
#endif //JZC_CRC_VER 

#ifdef JZC_MXU_OPT
  S32I2M(xr16, 0x7);
#endif

#ifdef JZC_TCSM_OPT
  AUX_RESET();
  tcsm_init();
#endif

#ifdef JZC_TLB_OPT
  int ll;
  tlb_i = 0;
  for(ll=0; ll<8; ll++)
    SET_VPU_TLB(ll, 0, 0, 0, 0);
#endif

#ifdef JZC_MC_OPT
  motion_init_rv9(RV9_QPEL, H264_EPEL);
  *(volatile int *)TCSM1_VCADDR(TCSM1_MOTION_DSA) = 0x8000FFFF;
#endif

  printf("tcsm1 end %x\n",TCSM1_MC_BUG_SPACE);
#ifdef JZC_DCORE_OPTn
  task_fifo_mem = av_malloc(3601 * sizeof(RV9_MB_DecARGs));
  frame_info_mem = av_malloc(32 * sizeof(int));
  frame_info_mem_ex = av_malloc(32 * sizeof(int));
  //emu_edge_buf_aux = av_malloc(3601 * 2*EMU_BUF_STRD);    

  if (task_fifo_mem < 0){
    printf("JZ4740 alloc task_fifo_mem failed!\n");
    return -1;
  }

  if (frame_info_mem < 0){
    printf("JZ4740 alloc frame_info_mem failed!\n");
    return -1;
  }
#if 0
  if (emu_edge_buf_aux < 0){
    printf("JZ4740 alloc emu_edge_buf_aux failed!\n");
    return -1;
  }
#endif
#endif

#ifdef HUGE_AIC_TABLE_OPT
  {int i;
    for(i=0;i<4096;i++)
      huge_rv40_aic_table_index[i]=20;
    for(i=0;i<20;i++)
      huge_rv40_aic_table_index[rv40_aic_table_index[i]]=i;
  }
#endif

#ifdef DIV9_TABLE_OPT
  {int i;
    for(i=0;i<81;i++)
      div9_tcsm0[i]=div9[i];
  }
#endif

#ifdef JZC_DCORE_OPT
  int * tmp_hm_buf = jz4740_alloc_frame(32, SPACE_HALF_MILLION_BYTE);
  if (tmp_hm_buf < 0)
    printf("JZ4740 ALLOC tmp_hm_buf ERROR !! \n");
#endif

#ifdef JZC_DCORE_OPT
  {
    //several setup instructions direct p1 to execute p1_boot
    int i;
    volatile int *src, *dst;
    // load p1 insn and data to reserved mem
    FILE *fp_text;
    int len, *reserved_mem;
    int *load_buf;	
#if 1 //#ifdef JZ_LINUX_OS
      printf("rv9 len of aux task = %d\n", rv9_auxcodes_len);
      reserved_mem = (int *)TCSM1_VCADDR(P1_MAIN_ADDR);      
      for (i=0; i< rv9_auxcodes_len/4; i++)
        reserved_mem[i] = rv9_aux_task_codes[i];
#else
    fp_text = fopen("./rv9_p1.bin", "r+b");
    if (!fp_text){
      printf(" error while open rv9_p1.bin \n");
      exit_player_with_rc();
    }
    load_buf = tmp_hm_buf;
    len = fread(load_buf, 4, SPACE_HALF_MILLION_BYTE, fp_text);
    reserved_mem = (int *)TCSM1_VCADDR(P1_MAIN_ADDR);      
    //printf("----------------------rv9_p1.bin len = %d\n", len);
    for(i=0; i<len; i++)
      reserved_mem[i] = load_buf[i];
    fclose(fp_text);
#endif
    jz_dcache_wb(); /*flush cache into reserved mem*/
    i_sync();
  }
#endif
#ifdef JZC_IDCT_OPT
  enable_idct() ;
  fresh_idct(1) ;
  fresh_idct(0) ;
  set_idct_type_video(REAL, //video
		      1    //type
		      ) ;
  set_idct_stride(16, // in_stride
		  8  // out_stride
		  );
  set_idct_block_width(64, 64); //
  desp_enable_idct();
#endif//JZC_IDCT_OPT

#ifdef JZC_PMON_P0
  i_mtc0_2(0, 16, 7);
#endif

#ifdef JZC_DBLK_OPT
  volatile int * dblk_base=(int*)DBLK_BASE;
  dblk_base[0] = 0x4;           //set end_flag for first polling
  FILL_DBLK_ABC_TABLE(abc_table);
  dblk_base[32] = TCSM1_DBLK_MBOUT_STRD_C;              //uv_out_stride
  dblk_base[31] = (1<<16)|TCSM1_DBLK_MBOUT_STRD_Y;      //y_out_stride
  dblk_base[30] = PREVIOUS_CHROMA_STRIDE;               //uv_in_stride
  dblk_base[29] = (1<<16)|PREVIOUS_LUMA_STRIDE;         //y_in_stride 
  dblk_base[28] = TCSM1_DBLK_UPOUT_STRD_C;              //uv_upo_stride
  dblk_base[27] = (1<<16)|TCSM1_DBLK_UPOUT_STRD_Y;      //y_upo_stride
  dblk_base[26] = SRAM_DBLKUP_STRD_C;                   //uv_upi_stride

  unsigned int * node;
  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN0);
  node[0] = 0x80000000;
  node[1] = 0x300d0048;    
  node[15] = 0x80000000;
  node[16] = 0x0001006C;//address node
  node[17] = TCSM1_PADDR(DBLK_END_FLAG);
  node[18] = DBLK_SYN_VALUE;
 
  node = (unsigned int *)TCSM1_VUCADDR(DBLK_DES_CHAIN1);
  node[0] = 0x80000000;
  node[1] = 0x300d0048;    
  node[15] = 0x80000000;
  node[16] = 0x0001006C;//address node
  node[17] = TCSM1_PADDR(DBLK_END_FLAG);
  node[18] = DBLK_SYN_VALUE;
#endif


  RV9_XCH2_T *RV_XCH_T0, *RV_XCH_T1;

  RV_XCH_T0 = TCSM1_VUCADDR(TCSM1_XCH2_T_BUF0);
  RV_XCH_T1 = TCSM1_VUCADDR(TCSM1_XCH2_T_BUF1);

  RV_XCH_T0->dblk_des_ptr = DBLK_DES_CHAIN0;
  RV_XCH_T0->dblk_out_addr = TCSM1_DBLK_MBOUT_Y0;
  RV_XCH_T0->dblk_out_addr_c = TCSM1_DBLK_MBOUT_U0;
  RV_XCH_T0->dblk_upout_addr_c = TCSM1_PADDR(TCSM1_DBLK_UPOUT_U0);

  RV_XCH_T1->dblk_des_ptr = DBLK_DES_CHAIN1;
  RV_XCH_T1->dblk_out_addr = TCSM1_DBLK_MBOUT_Y1;
  RV_XCH_T1->dblk_out_addr_c = TCSM1_DBLK_MBOUT_U1;
  RV_XCH_T1->dblk_upout_addr_c = TCSM1_PADDR(TCSM1_DBLK_UPOUT_U1);

  MPV_decode_defaults(s);
  s->avctx= avctx;
  s->out_format = FMT_H263;
  s->codec_id= avctx->codec_id;

  s->width = avctx->width;
  s->height = avctx->height;

  r->s.avctx = avctx;
  avctx->flags |= CODEC_FLAG_EMU_EDGE;
  r->s.flags |= CODEC_FLAG_EMU_EDGE;
  avctx->pix_fmt = PIX_FMT_YUV420P;
  avctx->has_b_frames = 1;
  s->low_delay = 0;

  if (MPV_common_init(s) < 0)
    return -1;

  ff_h264_pred_init(&r->h, CODEC_ID_RV40);

  r->intra_types_stride = 4*s->mb_stride + 4;
#if 1//def JZ_LINUX_OS
  r->intra_types_hist = jz4740_alloc_frame_k0(16, r->intra_types_stride * 5 * sizeof(*r->intra_types_hist));
  r->mb_type = jz4740_alloc_frame_k0(16, r->s.mb_stride * r->s.mb_height * sizeof(*r->mb_type)); 
  memset(r->mb_type, 0, r->s.mb_stride * r->s.mb_height * sizeof(*r->mb_type));
#else
  r->intra_types_hist = av_malloc(r->intra_types_stride * 5 * sizeof(*r->intra_types_hist));
  r->mb_type = av_mallocz(r->s.mb_stride * r->s.mb_height * sizeof(*r->mb_type));
#endif
  r->intra_types = r->intra_types_hist + r->intra_types_stride;
  
  r->cbp_luma   = av_malloc(r->s.mb_stride * r->s.mb_height * sizeof(*r->cbp_luma));
  r->cbp_chroma = av_malloc(r->s.mb_stride * r->s.mb_height * sizeof(*r->cbp_chroma));
  r->deblock_coefs = av_malloc(r->s.mb_stride * r->s.mb_height * sizeof(*r->deblock_coefs));

#ifdef JZC_DBLK_OPT
#if 1//def JZ_LINUX_OS
  r->mbtype = jz4740_alloc_frame_k0(16, (r->s.mb_stride+1) * sizeof(*r->mbtype));
  //memset(r->mbtype, 0, (r->s.mb_stride+1) * sizeof(*r->mbtype));
  *r->mbtype++=0;
  r->mvd = jz4740_alloc_frame_k0(16, (r->s.mb_stride+1) * sizeof(*r->mvd));
  //memset(r->mvd, 0, (r->s.mb_stride+1) * sizeof(*r->mvd));
  *r->mvd++=0;
  r->dcbp = jz4740_alloc_frame_k0(16, (r->s.mb_stride+1) * sizeof(*r->dcbp));
  //memset(r->dcbp, 0, (r->s.mb_stride+1) * sizeof(*r->dcbp));
  *r->dcbp++=0;
#else
  r->mbtype = av_malloc(r->s.mb_stride * r->s.mb_height * sizeof(*r->mbtype));
  r->mvd = av_malloc(r->s.mb_stride * r->s.mb_height * sizeof(*r->mvd));
  r->dcbp = av_malloc(r->s.mb_stride * r->s.mb_height * sizeof(*r->dcbp));
#endif
#endif

  if(!intra_vlcs[0].cbppattern[0].bits)
    rv34_init_tables();

  if(!aic_top_vlc.bits)
    rv40_init_tables();
#ifdef JZC_VLC_HW_OPT
    SET_GLOBAL_CTRL(0x10);
    last_quant=last_vlc_set=0xffabcdef;
    last_intra_tbl_sel=last_inter_tbl_sel=0xfe;
#endif
  //r->parse_slice_header = rv40_parse_slice_header;
  //r->decode_intra_types = rv40_decode_intra_types;
  //r->decode_mb_info     = rv40_decode_mb_info;
  //r->loop_filter        = rv40_loop_filter;
  r->luma_dc_quant_i = rv40_luma_dc_quant[0];
  r->luma_dc_quant_p = rv40_luma_dc_quant[1];
  use_jz_buf=1;
  return 0;
}

static int get_slice_offset(AVCodecContext *avctx, const uint8_t *buf, int n)
{
  if(avctx->slice_count) return avctx->slice_offset[n];
  else                   return AV_RL32(buf + n*8 - 4) == 1 ? AV_RL32(buf + n*8) :  AV_RB32(buf + n*8);
}
#undef printf


#if 1
void prt_square_rv(void * start_ptr,int size,int h,int w, int stride){
  unsigned int* start_int=(int*)start_ptr;
  unsigned short* start_short=(short*)start_ptr;
  unsigned char* start_byte=(char*)start_ptr;
  int i, j;
  if(size==4){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%8x,",start_int[i*stride+j]);
      }
      printf("\n");
    }
  }
  if(size==2){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%4x,",start_short[i*stride+j]);
      }
      printf("\n");
    }
  }
  if(size==1){
    for(i=0;i<h;i++){
      for(j=0;j<w;j++){
	printf("0x%2x,",start_byte[i*stride+j]);
      }
      printf("\n");
    }
  }
}
#endif


int ff_rv40_decode_frame(AVCodecContext *avctx,
			 void *data, int *data_size,
			 AVPacket *avpkt)
{
  const uint8_t *buf = avpkt->data;
  int buf_size = avpkt->size;
  RV34DecContext *r = avctx->priv_data;
  MpegEncContext *s = &r->s;
  AVFrame *pict = data;
  SliceInfo si;
  int i;
  int slice_count;
  const uint8_t *slices_hdr = NULL;
  int last = 0;
  int xch_tmp;
#ifdef JZC_DBLK_OPT
  do_dblk = 0;
#endif
  //printf("TCSM1_MC_BUG_SPACE = %x\n",TCSM1_MC_BUG_SPACE);
  /* no supplementary picture */
  if (buf_size == 0) {
    /* special case for last picture */
    if (s->low_delay==0 && s->next_picture_ptr) {
      *pict= *(AVFrame*)s->next_picture_ptr;
      s->next_picture_ptr= NULL;

      *data_size = sizeof(AVFrame);
    }
    return 0;
  }

  if(!avctx->slice_count){
    slice_count = (*buf++) + 1;
    slices_hdr = buf + 4;
    buf += 8 * slice_count;
  }else
    slice_count = avctx->slice_count;

  //parse first slice header to check whether this frame can be decoded
  if(get_slice_offset(avctx, slices_hdr, 0) > buf_size){
    av_log(avctx, AV_LOG_ERROR, "Slice offset is greater than frame size\n");
    return -1;
  }
  init_get_bits(&s->gb, buf+get_slice_offset(avctx, slices_hdr, 0), buf_size-get_slice_offset(avctx, slices_hdr, 0));
  if(rv40_parse_slice_header(r, &r->s.gb, &si) < 0 || si.start){
    av_log(avctx, AV_LOG_ERROR, "First slice header is incorrect\n");
    return -1;
  }
  if((!s->last_picture_ptr || !s->last_picture_ptr->data[0]) && si.type == FF_B_TYPE)
    return -1;
  /* skip b frames if we are in a hurry */
  if(avctx->hurry_up && si.type==FF_B_TYPE) return buf_size;
  if(   (avctx->skip_frame >= AVDISCARD_NONREF && si.type==FF_B_TYPE)
	|| (avctx->skip_frame >= AVDISCARD_NONKEY && si.type!=FF_I_TYPE)
	||  avctx->skip_frame >= AVDISCARD_ALL)
    return buf_size;
  /* skip everything if we are in a hurry>=5 */
  if(avctx->hurry_up>=5)
    return buf_size;

#ifdef JZC_VLC_HW_OPT
  jz_dcache_wb();
#endif
  for(i=0; i<slice_count; i++){
#ifdef JZC_DCORE_OPT
    if (!i){
      dSlice=(int*)TCSM1_VCADDR(TCSM1_SLICE_BUF0);
      dSlice_ex=(int*)TCSM1_VCADDR(TCSM1_SLICE_BUF1);
      tcsm1_fifo_wp = (int *)TCSM1_VUCADDR(TCSM1_MBNUM_WP);
      tcsm0_fifo_rp = (int *)TCSM0_P1_FIFO_RP;

      task_fifo_wp = (int *)TCSM0_TASK_FIFO; // used by P0
      *tcsm1_fifo_wp = 0; // write by P0, used by P1
      *tcsm0_fifo_rp = 0; // write once before p1 start

      r->new_slice_p0 = 0;
      AUX_RESET();
      *((volatile int *)(TCSM0_P1_TASK_DONE)) = 0;
      AUX_START();

#ifdef JZC_MC_OPT
      motion_config_rv9(s);
#endif
       
    }else{
      r->new_slice_p0 = 1;
      XCHG2(dSlice,dSlice_ex,xch_tmp);
    }
#endif

    int offset= get_slice_offset(avctx, slices_hdr, i);
    int size;
    if(i+1 == slice_count)
      size= buf_size - offset;
    else
      size= get_slice_offset(avctx, slices_hdr, i+1) - offset;

    if(offset > buf_size){
      av_log(avctx, AV_LOG_ERROR, "Slice offset is greater than frame size\n");
      break;
    }

    r->si.end = s->mb_width * s->mb_height;
    if(i+1 < slice_count){
      init_get_bits(&s->gb, buf+get_slice_offset(avctx, slices_hdr, i+1), (buf_size-get_slice_offset(avctx, slices_hdr, i+1))*8);
      if(rv40_parse_slice_header(r, &r->s.gb, &si) < 0){
	if(i+2 < slice_count)
	  size = get_slice_offset(avctx, slices_hdr, i+2) - offset;
	else
	  size = buf_size - offset;
      }else
	r->si.end = si.start;
    }

    last = rv40_decode_slice(r, r->si.end, buf + offset, size);
    s->mb_num_left = r->s.mb_x + r->s.mb_y*r->s.mb_width - r->si.start;
    if(last < 0)
      {
	((volatile int *)(TCSM0_P1_FIFO_RP))[0]=0;
	*((volatile int *)(TCSM0_P1_TASK_DONE)) = 0x1;	
	AUX_RESET();       
	return -1;
      }
    else if(last)
      break;
  }

  if(last){
    //if(r->loop_filter)
    //    r->loop_filter(r, s->mb_height - 1);
    
#ifdef JZC_DCORE_OPT
    {
      int ai;
      int add_num = 3;
      for (ai=0; ai<add_num+2; ai++) {
	unsigned int tcsm0_fifo_rp_lh, task_fifo_wp_lh, task_fifo_wp_lh_overlap;

	do{
	  tcsm0_fifo_rp_lh = (unsigned int)*tcsm0_fifo_rp & 0x0000FFFF;
	  task_fifo_wp_lh = (unsigned int)task_fifo_wp & 0x0000FFFF;
	  task_fifo_wp_lh_overlap = ((unsigned int)task_fifo_wp + TASK_BUF_LEN) & 0x0000FFFF;
	} while ( !( (task_fifo_wp_lh_overlap <= tcsm0_fifo_rp_lh) || (task_fifo_wp_lh > tcsm0_fifo_rp_lh) ) );

	RV9_MB_DecARGs * dMB = task_fifo_wp;
        if (ai == 3)
	  dMB->new_slice = -1;
	else
	  dMB->new_slice = 0;
	//dMB->ctrl_len = sizeof(RV9_MB_DecARGs);
	dMB->cbp = 0;
	dMB->mb_type = MB_TYPE_INTRA;

	dMB->dcbp_above = 0;
	dMB->dcbp_left = 0;
	dMB->mvd_above = 0;
	dMB->mvd_left = 0;
	dMB->mvd = 0;
	dMB->mbtype = -1;
	dMB->mbtype_above = 0;
	dMB->mbtype_left = 0;
	dMB->is16=0;
        dMB->qscale = s->qscale;
        dMB->er_slice = 0;
	dMB->mb_y = s->mb_height;
        dMB->mb_x = s->mb_width;

	task_fifo_wp += TASK_BUF_LEN >> 2;
 	//*(uint16_t *)task_fifo_wp_d2 = (sizeof(RV9_MB_DecARGs)) & 0xFFFC;

	int reach_tcsm0_end = ((unsigned int)task_fifo_wp + TASK_BUF_LEN) > 0xF4004000;
	if (reach_tcsm0_end)
	  task_fifo_wp = (int *)TCSM0_TASK_FIFO;
	(*tcsm1_fifo_wp)++;
      }


      (*tcsm1_fifo_wp) += 5;
#ifdef JZC_PMON_P0
      PMON_ON(rv9wait);
#endif
      int tmp;
      do {
	tmp = *((volatile int *)(TCSM0_P1_TASK_DONE));
      } while (tmp == 0);

      AUX_RESET();
#ifdef JZC_PMON_P0
      PMON_OFF(rv9wait);
#endif

    }
#endif // JZC_DCORE_OPT

#ifdef JZC_TEST_OPT
    int dump_i, dump_j, m, n;
    int dump_mb_x, dump_mb_y;
    uint8_t *dump_ptr;
    uint8_t *dump_frm_ptr;

    unsigned char *dy, *dc;
    unsigned char *dy_tmp, *dc_tmp;
    unsigned char *dy_test, *dc_test;
#if 0
    dy_tmp = s->current_picture.data[0];
    dc_tmp = s->current_picture.data[1];

    dy = s->current_picture.data[0];
    dc = s->current_picture.data[1];

    dump_frm_ptr = ydd;
    for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
      dy = dy_tmp;
      for (dump_mb_x = 0; dump_mb_x < s->mb_width; dump_mb_x ++ ) {
	dump_ptr = dump_frm_ptr + (dump_mb_y*16*s->linesize) + dump_mb_x*16;
	for (dump_i=0; dump_i<16; dump_i++) {
	  for (dump_j=0; dump_j<16; dump_j++) {
            *(dy + dump_i * 16 + dump_j) = *(dump_ptr + dump_i * s->linesize + dump_j);
	  }
	}
        dy += 256;
      }
      dy_tmp += (s->mb_width +4) * 256;
    }

    dump_frm_ptr = udd;
    dc_tmp = s->current_picture.data[1];
    for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
      dc = dc_tmp;
      for (dump_mb_x = 0; dump_mb_x < s->mb_width; dump_mb_x ++ ) {
	dump_ptr = dump_frm_ptr + (dump_mb_y*8*s->uvlinesize) + dump_mb_x*8;
	for (dump_i=0; dump_i<8; dump_i++) {
	  for (dump_j=0; dump_j<8; dump_j++) {
	    *(dc + dump_i * 16 + dump_j) = *(dump_ptr + dump_i * s->uvlinesize + dump_j);
	  }
	}
        dc += 128;
      }

      dc_tmp += (s->mb_width*2 + 8) * 64;
    }

    dump_frm_ptr = vdd;
    dc_tmp = s->current_picture.data[1] + 8;
   
    for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
      dc = dc_tmp;
      for (dump_mb_x = 0; dump_mb_x < s->mb_width; dump_mb_x ++ ) {
	dump_ptr = dump_frm_ptr + (dump_mb_y*8*s->uvlinesize) + dump_mb_x*8;
	for (dump_i=0; dump_i<8; dump_i++) {
	  for (dump_j=0; dump_j<8; dump_j++) {
	    *(dc + dump_i * 16 + dump_j) = *(dump_ptr + dump_i * s->uvlinesize + dump_j);
	  }
	}
        dc += 128;
      }
      dc_tmp += (s->mb_width*2 + 8) * 64;
    }
#endif
#endif

#ifdef JZC_CRC_VER
    {
      int dump_mb_y;
      int linelen = (s->mb_width +4) * 256;
      for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
	  crc_code_line = crc(s->current_picture.data[0] + dump_mb_y * linelen, s->mb_width * 256, crc_code_line);
      }

      linelen = (s->mb_width*2 + 8) * 64;
      for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
	  crc_code_line = crc(s->current_picture.data[1] + dump_mb_y * linelen, s->mb_width * 128, crc_code_line);
      }
      mp_msg(NULL,NULL,"frame: %d, crc_code_line: 0x%x\n", rvFrame, crc_code_line);
      //printf("frame: %d, crc_code_line: 0x%x\n", rvFrame, crc_code_line);
    }
#endif
#ifdef JZC_PMON_P0
    {
      int mb_num = ((s->width+15)/16)*((s->height+15)/16);
      if (rv9_pmon_p0_frmcnt==0){
	pmon_p0_fp=fopen("jz4760e_p0.pmon","aw+");
        fprintf(pmon_p0_fp, "PMON nfl:%s\t(size: %d x %d; mb_num: %d) \n",filename,s->width,s->height,mb_num);
      }

      fprintf(pmon_p0_fp,"PMON frame num: %d\n",rv9_pmon_p0_frmcnt);
      fprintf(pmon_p0_fp,"PMON VLC  -D: %d; I:%d\n",
	      rv9vlc_pmon_val/mb_num, rv9vlc_pmon_val_ex/mb_num);
      fprintf(pmon_p0_fp,"PMON WAIT -D: %d; I:%d\n",
	      rv9wait_pmon_val/mb_num, rv9wait_pmon_val_ex/mb_num);
      fprintf(pmon_p0_fp,"PMON WHILE -D: %d; I:%d\n",
	      rv9while_pmon_val/mb_num, rv9while_pmon_val_ex/mb_num);
      rv9vlc_pmon_val=0; rv9vlc_pmon_val_ex=0;
      rv9wait_pmon_val=0; rv9wait_pmon_val_ex=0;
      rv9while_pmon_val=0; rv9while_pmon_val_ex=0;
      rv9_pmon_p0_frmcnt++;
    }
#endif //JZC_PMON_P0

    ff_er_frame_end(s);
    MPV_frame_end(s);

#define PW_DBG
#define DUMP_FRAME_STR 0
#define DUMP_FRAME_END 0

#ifdef PW_DBGn
    if ( (rvFrame >= DUMP_FRAME_STR) && (rvFrame <= DUMP_FRAME_END) )
    //if(rvFrame <= 54 && crc_code != tt[rvFrame])
      {
	int dump_i, dump_j;
	int dump_mb_x, dump_mb_y;
	uint8_t *dump_ptr;
	uint8_t *dump_frm_ptr;
#if 1
	printf(" ++++++  y buffer, frame:%d ++++++ PW_DBG\n",rvFrame);
#ifdef JZC_TEST_OPT
        dump_frm_ptr = ydd;
#else
	dump_frm_ptr = s->current_picture.data[0];
#endif
        //for (dump_mb_y = 34; dump_mb_y < 35; dump_mb_y ++ ) {
	for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
	  for (dump_mb_x = 0; dump_mb_x < s->mb_width; dump_mb_x ++ ) {
	    dump_ptr = dump_frm_ptr + (dump_mb_y*16*s->linesize) + dump_mb_x*16;
	    printf(" ---- mb_x:%d, mb_y:%dPW_DBG\n",dump_mb_x,dump_mb_y);
	    for (dump_i=0; dump_i<16; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x, ",dump_ptr[dump_j]);
	      }
              printf("\n");
	      dump_ptr += s->linesize;
	    }

	  }
	}
#endif

#if 1
	printf(" ++++++  u buffer, frame:%d ++++++ PW_DBG\n",rvFrame);
#ifdef JZC_TEST_OPT
        dump_frm_ptr = udd;
#else
	dump_frm_ptr = s->current_picture.data[1];
#endif
	for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
	  for (dump_mb_x = 0; dump_mb_x < s->mb_width; dump_mb_x ++ ) {
	    dump_ptr = dump_frm_ptr + (dump_mb_y*8*s->uvlinesize) + dump_mb_x*8;
	    printf(" ---- mb_x:%d, mb_y:%dPW_DBG\n",dump_mb_x,dump_mb_y);
	    for (dump_i=0; dump_i<8; dump_i++) {
	      for (dump_j=0; dump_j<8; dump_j++) {
		printf("0x%2x, ",dump_ptr[dump_j]);
	      }
	      printf("\n");
	      dump_ptr += s->uvlinesize;
	    }
	  }
	}
#endif
	printf(" ++++++  v buffer, frame:%d ++++++ PW_DBG\n",rvFrame);
#ifdef JZC_TEST_OPT
        dump_frm_ptr = vdd;
#else
	dump_frm_ptr = s->current_picture.data[2];
#endif
	for (dump_mb_y = 0; dump_mb_y < s->mb_height; dump_mb_y ++ ) {
	  for (dump_mb_x = 0; dump_mb_x < s->mb_width; dump_mb_x ++ ) {
	    dump_ptr = dump_frm_ptr + (dump_mb_y*8*s->uvlinesize) + dump_mb_x*8;
	    printf(" ---- mb_x:%d, mb_y:%dPW_DBG\n",dump_mb_x,dump_mb_y);
	    for (dump_i=0; dump_i<8; dump_i++) {
	      for (dump_j=0; dump_j<8; dump_j++) {
		printf("0x%2x, ",dump_ptr[dump_j]);
	      }
	      printf("\n");
	      dump_ptr += s->uvlinesize;
	    }
	  }
	}
      }
#endif //PW_DBG

    volatile uint8_t *tile_y, *tile_c, *pxl, *edge, *tile, *dest;
    int mb_i, mb_j, i, j;
    uint8_t *tt2, *pxl1, *pxl2, *edge1,*edge2,*edge3;
    int nn1, nn2;

    tile_y = s->current_picture.data[0];
    tile_c = s->current_picture.data[1];
#if 1
    /********************* top ***********************/
    pxl = tile_y + (s->mb_width - 1) * 256;
    pxl1 = tile_c + (s->mb_width - 1) * 128;
    edge = pxl - s->linesize*2 - 16;
    edge1 = pxl - s->linesize - 16;
    S32LDD(xr1,pxl,0x0);
    S32LDD(xr2,pxl,0x4);
    S32LDD(xr3,pxl,0x8);
    S32LDD(xr4,pxl,0xC);
    for(j=0; j<16; j++){
      S32SDI(xr1,edge,0x10);
      S32STD(xr2,edge,0x4);
      S32STD(xr3,edge,0x8);
      S32STD(xr4,edge,0xC);
      S32SDI(xr1,edge1,0x10);
      S32STD(xr2,edge1,0x4);
      S32STD(xr3,edge1,0x8);
      S32STD(xr4,edge1,0xC); 
    }
    edge = pxl1 - s->linesize - 16;
    edge1 = pxl1 - s->uvlinesize - 16;
    S32LDD(xr1,pxl1,0x0);
    S32LDD(xr2,pxl1,0x4);
    S32LDD(xr3,pxl1,0x8);
    S32LDD(xr4,pxl1,0xC);
    for(j=0; j<8; j++){
      S32SDI(xr1,edge,0x10);
      S32STD(xr2,edge,0x4);
      S32STD(xr3,edge,0x8);
      S32STD(xr4,edge,0xC);
      S32SDI(xr1,edge1,0x10);
      S32STD(xr2,edge1,0x4);
      S32STD(xr3,edge1,0x8);
      S32STD(xr4,edge1,0xC); 
    }
    
    /********************* bottom ***********************/
    for(i=0; i<3; i++){
      if (!i){
	edge = tile_y + s->mb_height*s->linesize;
        edge2 = tile_c + s->mb_height*s->uvlinesize;
      }else{
        edge = tile_y + s->mb_height*s->linesize + (s->mb_width - i) * 256;
        edge2 = tile_c + s->mb_height*s->uvlinesize + (s->mb_width -i) * 128;
      }
      edge1 = edge + s->linesize - 16;
      edge3 = edge2 + s->uvlinesize - 16;
      pxl = edge - s->linesize + 16*15;
      pxl1 = edge2 - s->uvlinesize + 16*7;
      edge -= 16;
      edge2 -= 16;
      S32LDD(xr1,pxl,0x0);
      S32LDD(xr2,pxl,0x4);
      S32LDD(xr3,pxl,0x8);
      S32LDD(xr4,pxl,0xC);
      for(j=0; j<16; j++){
	S32SDI(xr1,edge,0x10);
	S32STD(xr2,edge,0x4);
	S32STD(xr3,edge,0x8);
	S32STD(xr4,edge,0xC);
	S32SDI(xr1,edge1,0x10);
	S32STD(xr2,edge1,0x4);
	S32STD(xr3,edge1,0x8);
	S32STD(xr4,edge1,0xC); 
      }
      S32LDD(xr1,pxl1,0x0);
      S32LDD(xr2,pxl1,0x4);
      S32LDD(xr3,pxl1,0x8);
      S32LDD(xr4,pxl1,0xC);
      for(j=0; j<8; j++){
	S32SDI(xr1,edge2,0x10);
	S32STD(xr2,edge2,0x4);
	S32STD(xr3,edge2,0x8);
	S32STD(xr4,edge2,0xC);
	S32SDI(xr1,edge3,0x10);
	S32STD(xr2,edge3,0x4);
	S32STD(xr3,edge3,0x8);
	S32STD(xr4,edge3,0xC); 
      }
    }


    /********************* left***********************/
    for(mb_i=-3; mb_i<3; mb_i++){
      if (mb_i<0) {
        pxl = tile_y + (mb_i+1)*s->linesize - 16;
        pxl1 = tile_c + (mb_i+1)*s->uvlinesize - 16;
	pxl2 = pxl1 + 8;
      }else{
        pxl = tile_y + (s->mb_height - 1 + mb_i)*s->linesize - 16;
	pxl1 = tile_c + (s->mb_height - 1 + mb_i)*s->uvlinesize - 16;
	pxl2 = pxl1 + 8;
      }     
      edge = pxl - 16*32;
      edge1 = pxl - 256;
      for(i=0; i<8; i++){
	S8LDI(xr1,pxl,0x10,ptn7);
	S8LDI(xr2,pxl,0x10,ptn7);
        S32SDI(xr1,edge,0x10);
        S32STD(xr1,edge,0x4);
        S32STD(xr1,edge,0x8);
        S32STD(xr1,edge,0xC);
        S32SDI(xr1,edge1,0x10);
        S32STD(xr1,edge1,0x4);
        S32STD(xr1,edge1,0x8);
        S32STD(xr1,edge1,0xC);
        S32SDI(xr2,edge,0x10);
        S32STD(xr2,edge,0x4);
        S32STD(xr2,edge,0x8);
        S32STD(xr2,edge,0xC);
        S32SDI(xr2,edge1,0x10);
        S32STD(xr2,edge1,0x4);
        S32STD(xr2,edge1,0x8);
        S32STD(xr2,edge1,0xC);
      }
      edge2 = pxl1 - 16*16;
      edge3 = pxl1 - 128;
      for(i=0; i<8; i++){
	S8LDI(xr1,pxl1,0x10,ptn7);
	S8LDI(xr3,pxl2,0x10,ptn7);
        S32SDI(xr1,edge2,0x10);
        S32STD(xr1,edge2,0x4);
	S32STD(xr3,edge2,0x8);
	S32STD(xr3,edge2,0xC);
        S32SDI(xr1,edge3,0x10);
        S32STD(xr1,edge3,0x4);
	S32STD(xr3,edge3,0x8);
	S32STD(xr3,edge3,0xC);
      }
    }
 
    /********************* right ***********************/
    for(mb_i=-2; mb_i<3; mb_i++){
      if (mb_i<0){
	pxl = tile_y + (s->mb_width-1)*16*16 + 15 + mb_i*s->linesize - 16;
        pxl1 = tile_c + (s->mb_width-1)*16*8 + 7 + mb_i*s->uvlinesize - 16;
	pxl2 = pxl1 + 8;
      }else{
	pxl = tile_y + (s->mb_width-1)*16*16 + 15 + (s->mb_height - 1 + mb_i)*s->linesize - 16;
	pxl1 = tile_c + (s->mb_width-1)*16*8 + 7 + (s->mb_height - 1 + mb_i)*s->uvlinesize - 16;
	pxl2 = pxl1 + 8;
      }
      edge = pxl - 15 + 256;
      edge1 = edge + 256;
      for(i=0; i<8; i++){
        S8LDI(xr1,pxl,0x10,ptn7);
        S8LDI(xr2,pxl,0x10,ptn7);
        S32SDI(xr1,edge,0x10);
        S32STD(xr1,edge,0x4);
	S32STD(xr1,edge,0x8);
        S32STD(xr1,edge,0xC);
        S32SDI(xr1,edge1,0x10);
        S32STD(xr1,edge1,0x4);
	S32STD(xr1,edge1,0x8);
        S32STD(xr1,edge1,0xC);
        S32SDI(xr2,edge,0x10);
        S32STD(xr2,edge,0x4);
	S32STD(xr2,edge,0x8);
        S32STD(xr2,edge,0xC);
        S32SDI(xr2,edge1,0x10);
        S32STD(xr2,edge1,0x4);
	S32STD(xr2,edge1,0x8);
        S32STD(xr2,edge1,0xC);
      }
      edge2 = pxl1 - 7 + 16*8;
      edge3 = edge2 + 128;
      for(i=0; i<8; i++){
        S8LDI(xr1,pxl1,0x10,ptn7);
	S8LDI(xr2,pxl2,0x10,ptn7);
        S32SDI(xr1,edge2,0x10);
        S32STD(xr1,edge2,0x4);
	S32STD(xr2,edge2,0x8);
	S32STD(xr2,edge2,0xC);
        S32SDI(xr1,edge3,0x10);
        S32STD(xr1,edge3,0x4);
	S32STD(xr2,edge3,0x8);
	S32STD(xr2,edge3,0xC);
      } 
    }
#else
    ////////////draw edge//////////   
    // int mb_i, mb_j, i, j;
    int linesize = s->linesize / 16;
    /********************* top ***********************/
    for(mb_i=0; mb_i<s->mb_width; mb_i++){
      // Y
      pxl = tile_y + mb_i*16*16;
      edge = pxl - linesize*32;
      for(j=0; j<16; j++){
	memcpy(edge, pxl, 16);
	memcpy(edge+linesize*16, pxl, 16);
	edge += 16;
      }

      // C
      pxl = tile_c + mb_i*16*8;
      edge = pxl - linesize*16;
      for(j=0; j<8; j++){
	memcpy(edge, pxl, 16);
	memcpy(edge+linesize*8, pxl, 16);
	edge += 16;
      }
    }


    /********************* bottom ***********************/
    for(mb_i=0; mb_i<s->mb_width; mb_i++){
      // Y
      edge = tile_y + s->mb_height*16*linesize + mb_i*16*16;
      pxl = edge - linesize*16 + 16*15;
      for(j=0; j<16; j++){
	memcpy(edge, pxl, 16);
	memcpy(edge+linesize*16, pxl, 16);
	edge += 16;
      }
      // C
      edge = tile_c + s->mb_height*8*linesize + mb_i*16*8;
      pxl = edge - linesize*8 + 16*7;
      for(j=0; j<8; j++){
	memcpy(edge, pxl, 16);
	memcpy(edge+linesize*8, pxl, 16);
	edge += 16;
      }
    }

    /********************* left ***********************/
    for(mb_i=-2; mb_i<s->mb_height+2; mb_i++){
      // Y
      for(j=0; j<16; j++){
	pxl = tile_y + mb_i*16*linesize + 16*j;
	edge = pxl - 16*32;
	memset(edge, pxl[0], 16);
	memset(edge+16*16, pxl[0], 16);
      }

      // C
      for(j=0; j<8; j++){
	pxl = tile_c + mb_i*8*linesize + 16*j;
	edge = pxl - 8*32;
	memset(edge, pxl[0], 8);
	memset(edge+8, pxl[8], 8);
	memset(edge+8*16, pxl[0], 8);
	memset(edge+8*16+8, pxl[8], 8);
      }
    }

    /********************* right ***********************/
    for(mb_i=-2; mb_i<s->mb_height+2; mb_i++){
      // Y
      for(j=0; j<16; j++){
	pxl = tile_y + (s->mb_width-1)*16*16 + 15 + mb_i*16*linesize + 16*j;
	edge = pxl - 15 + 16*16;
	memset(edge, pxl[0], 16);
	memset(edge+16*16, pxl[0], 16);
      }

      // C
      for(j=0; j<8; j++){
	pxl = tile_c + (s->mb_width-1)*16*8 + 7 + mb_i*8*linesize + 16*j;
	edge = pxl - 7 + 16*8;
	memset(edge, pxl[0], 8);
	memset(edge+8, pxl[8], 8);
	memset(edge+8*16, pxl[0], 8);
	memset(edge+8*16+8, pxl[8], 8);
      }
    }
#endif

#if 0//mb print
    dyy = s->current_picture.data[0] - 256*2 - s->linesize*2;
    duv = s->current_picture.data[1] - 256   - s->linesize;      
    //dyy = s->current_picture.data[0];
    //duv = s->current_picture.data[1];
    if(rvFrame == 0){
      dy = dyy;
      dc = duv;
      for (dump_mb_y = 0; dump_mb_y < s->mb_height+4; dump_mb_y ++ ) {
	for (dump_mb_x = 0; dump_mb_x < s->mb_width+4; dump_mb_x ++ ) {
	  //if((dump_mb_y<1||dump_mb_y<-1)&&(dump_mb_x>6||dump_mb_x<-1))
	  {
	    printf("\ndump_mb_y %d dump_mb_x %d\n",dump_mb_y,dump_mb_x);
	    for (dump_i=0; dump_i<16; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	    printf("chroma pixels\n");
	    for (dump_i=0; dump_i<8; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	  }
	  dy+=256;
	  dc+=128;
	}
	//dy += 1024;
	//dc += 512;
      }
    }
#endif

#ifdef JZC_ROTA90_OPT
      int k1, k2, l1, l2;
      unsigned char  *du, *dv;
      unsigned char  *rota_y1, *rota_c1;
      //int rota_linesize, rota_uvlinesize;
      //rota_linesize = ((avctx->height + 15) >> 4);   
      //rota_uvlinesize = ((rota_linesize + 1) >> 1);       
      rota_y1 = s->current_picture.data[2];
      rota_c1 = s->current_picture.data[3];
   
#if 0
      dyy = s->current_picture.data[0];
      duv = s->current_picture.data[1];
      for(k2=0; k2<s->mb_height; k2++){     
      for(k1=0; k1<s->mb_width; k1++){       
	dy = dyy + k2 * (s->mb_width+4) * 256 + k1 * 256;
	rota_y = rota_y1 +  k1 * s->mb_height*256 + (s->mb_height-1-k2)*256;
	for(l1=0;l1<16;l1++){
	  for(l2=0;l2<16;l2++){
	    rota_y[l2*16+15-l1] = dy[l1*16+l2];	   
	  }	  
	}
      }
    }
         
    for(k2=0; k2<s->mb_height; k2++){     
      for(k1=0; k1<s->mb_width; k1++){       
	du = duv + k2 * (s->mb_width+4) * 128 + k1 * 128;
	rota_c = rota_c1 +  k1 * s->mb_height*128 + (s->mb_height-1-k2)*128;
	for(l1=0;l1<8;l1++){
	  for(l2=0;l2<8;l2++){
	    rota_c[l2*16+7-l1] = du[l1*16+l2];
	    rota_c[l2*16+15-l1] = du[8 + l1*16+l2];
	  }
	}       
      }
    }
#endif

#ifdef JZC_CRC_VERN//rota crc code
      int crc_90;
      int linesize, uvlinesize;
      linesize=s->mb_height*256;
      uvlinesize=linesize>>1;
      for(crc_90=0;crc_90<(s->mb_width);crc_90++){
	crc_code_rota = crc(rota_y1 + crc_90 * (linesize), linesize, crc_code_rota);
      }
      for(crc_90=0;crc_90<(s->mb_width);crc_90++){
	crc_code_rota = crc(rota_c1 + crc_90 * (uvlinesize), uvlinesize, crc_code_rota);
      }
      mp_msg(NULL,NULL,"frame: %d,rota line mb crc_code_rota: 0x%x@#\n", rvFrame, crc_code_rota);
#endif

#if 0//mb print
      if(rvFrame==0){
	uint8_t *dy, *dc;
	//rota_y = 
	//rota_c = s->current_picture.data[3];
      dy = s->current_picture.data[2];
      dc = s->current_picture.data[3];
      for (dump_mb_y = 0; dump_mb_y < s->mb_width; dump_mb_y ++ ) {
	for (dump_mb_x = 0; dump_mb_x < s->mb_height; dump_mb_x ++ ) {
	  //  if(dump_mb_y==0&&dump_mb_x==0)
	  {
	    printf("\ndump_mb_y %d dump_mb_x %d\n",dump_mb_y,dump_mb_x);
	    for (dump_i=0; dump_i<16; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dy + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	    dy+=256;
	    printf("chroma pixels\n");
	    for (dump_i=0; dump_i<8; dump_i++) {
	      for (dump_j=0; dump_j<16; dump_j++) {
		printf("0x%2x ",*(dc + dump_i * 16 + dump_j));
	      }
	      printf("\n");
	    }
	    dc+=128;
	  }
	}
      }
      }
#endif

#endif
    if (s->pict_type == FF_B_TYPE || s->low_delay) {
      *pict= *(AVFrame*)s->current_picture_ptr;
    } else if (s->last_picture_ptr != NULL) {
      *pict= *(AVFrame*)s->last_picture_ptr;
    }

    if(s->last_picture_ptr || s->low_delay){
      *data_size = sizeof(AVFrame);
      ff_print_debug_info(s, pict);
    }
    s->current_picture_ptr= NULL; //so we can detect if frame_end wasnt called (find some nicer solution...)
  }

#ifdef JZC_CRC_VER
  rvFrame++;
#endif
  return buf_size;
}

av_cold int ff_rv40_decode_end(AVCodecContext *avctx)
{
  RV34DecContext *r = avctx->priv_data;

  MPV_common_end(&r->s);
#ifdef JZC_DCORE_OPTN
  av_free(task_fifo_mem);
  av_free(frame_info_mem);
  //av_free(emu_edge_buf_aux);
#endif
#if 1//#ifdef JZ_LINUX_OS
#else
  av_freep(&r->intra_types_hist);
  av_freep(&r->mb_type);
#endif
  r->intra_types = NULL;
  av_freep(&r->cbp_luma);
  av_freep(&r->cbp_chroma);
  av_freep(&r->deblock_coefs);

#ifdef JZC_DBLK_OPT
#if 1//#ifdef JZ_LINUX_OS
#else
  av_freep(&r->mbtype);
  av_freep(&r->mvd);
  av_freep(&r->dcbp);
#endif
#endif
  use_jz_buf=0;

  return 0;
}

AVCodec rv40_decoder = {
  "rv40",
  AVMEDIA_TYPE_VIDEO,
  CODEC_ID_RV40,
  sizeof(RV34DecContext),
  rv40_decode_init,
  NULL,
  ff_rv40_decode_end,
  ff_rv40_decode_frame,
  CODEC_CAP_DR1 | CODEC_CAP_DELAY,
  .flush = ff_mpeg_flush,
  .long_name = NULL_IF_CONFIG_SMALL("RealVideo 4.0"),
  .pix_fmts= ff_pixfmt_list_420,
};
