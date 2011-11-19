
#include <mplaylib.h>
#include "fcntl.h"
#include "unistd.h"
#include "sys/ioctl.h"

#include "libavcodec/avcodec.h"
#include "libmpcodecs/img_format.h"
#include "stream/stream.h"
#include "libmpdemux/demuxer.h"
#include "libmpdemux/stheader.h"

#include "libmpcodecs/img_format.h"
#include "libmpcodecs/mp_image.h"
#include "libmpcodecs/vf.h"
#include "libswscale/swscale.h"
#include "libswscale/swscale_internal.h"
#include "libmpcodecs/vf_scale.h"

#ifdef JZ4750_OPT
#include "jz4750_ipu_regops.h"
#include "jz4750_lcd.h"
struct jz4750lcd_info jzlcd_info;
#else
#include "jz4740_ipu_regops.h"
#endif
#include "jz47_iputype.h"

//#define  JZ47_IPU_CRC_VERIFY
//#define OUTPUT_BLACK_NOKEYFRAME
#ifdef JZ47_IPU_CRC_VERIFY
extern unsigned int image_crc32;
extern unsigned int crc32_verify(unsigned char *buf, int line, int col, int stride);
#endif

typedef struct {
    AVCodecContext *avctx;
    AVFrame *pic;
    enum PixelFormat pix_fmt;
    int do_slices;
    int do_dr1;
    int vo_initialized;
    int best_csp;
    int b_age;
    int ip_age[2];
    int qp_stat[32];
    double qp_sum;
    double inv_qp_sum;
    int ip_count;
    int b_count;
    AVRational last_sample_aspect_ratio;
} vd_ffmpeg_ctx;

static struct vf_priv_s {
    int w,h;
    int v_chr_drop;
    double param[2];
    unsigned int fmt;
    struct SwsContext *ctx;
    struct SwsContext *ctx2; //for interlaced slices only
    unsigned char* palette;
    int interlaced;
    int noup;
    int accurate_rnd;
};

/* ================================================================================ */
extern int xpos, ypos, tv_mode;
extern int jz47_calc_resize_para ();
/* ================================================================================ */
#define IPU_OUT_FB        0
#define IPU_OUT_LCD       1
#define IPU_OUT_PAL_TV    2
#define IPU_OUT_NTSC_TV   3
#define IPU_OUT_MEM       8

#define DEBUG_LEVEL  1  /* 1 dump resize message, 2 dump register for every frame.  */
/* ================================================================================ */
int scale_outW=0;  // -1, calculate the w,h base the panel w,h; 0, use the input w,h
int scale_outH=0;

#ifdef JZ4760_OPT
  #define CPU_TYPE 4760
  #define OUTPUT_MODE IPU_OUT_FB
#else
#ifdef JZ4755_OPT
  #define CPU_TYPE 4755
  #define OUTPUT_MODE IPU_OUT_LCD
#else
#ifdef JZ4750_OPT
  #define CPU_TYPE 4750
  #define OUTPUT_MODE IPU_OUT_FB
#else
  #define CPU_TYPE 4740
  #define OUTPUT_MODE IPU_OUT_FB
#endif /* JZ4750_OPT  */
#endif /* JZ4755_OPT  */
#endif /* JZ4760_OPT  */

/* struct IPU module to recorde some info */
struct JZ47_IPU_MOD jz47_ipu_module = {
    .output_mode = OUTPUT_MODE, /* Use the frame for the default */ 
    .rsize_algorithm = 1,      /* 0: liner, 1: bicube, 2: biliner.  */
    .rsize_bicube_level = -2,     /*-2, -1, -0.75, -0.5 */
  };

/* Flag to indicate the module init status */
int jz47_ipu_module_init = 0; 

/* CPU type: 4740, 4750, 4755, 4760 */
int jz47_cpu_type = CPU_TYPE;

/* flush the dcache.  */
unsigned int dcache[4096];

unsigned char *jz47_ipu_output_mem_ptr = NULL;
static int ipu_fbfd = 0;
int ipu_image_completed = 0;

unsigned int get_phy_addr (unsigned int vaddr);
/* ipu virtual address.   */
volatile unsigned char *ipu_vbase = (unsigned char *)(IPU__OFFSET | 0xa0000000);

static char *ipu_regs_name[] = {
    "REG_CTRL",         /* 0x0 */ 
    "REG_STATUS",       /* 0x4 */ 
    "REG_D_FMT",        /* 0x8 */ 
    "REG_Y_ADDR",       /* 0xc */ 
    "REG_U_ADDR",       /* 0x10 */ 
    "REG_V_ADDR",       /* 0x14 */ 
    "REG_IN_FM_GS",     /* 0x18 */ 
    "REG_Y_STRIDE",     /* 0x1c */ 
    "REG_UV_STRIDE",    /* 0x20 */ 
    "REG_OUT_ADDR",     /* 0x24 */ 
    "REG_OUT_GS",       /* 0x28 */ 
    "REG_OUT_STRIDE",   /* 0x2c */ 
    "RSZ_COEF_INDEX",   /* 0x30 */ 
    "REG_CSC_C0_COEF",  /* 0x34 */ 
    "REG_CSC_C1_COEF",  /* 0x38 */ 
    "REG_CSC_C2_COEF",  /* 0x3c */ 
    "REG_CSC_C3_COEF",  /* 0x40 */ 
    "REG_CSC_C4_COEF",  /* 0x44 */
    "REG_H_LUT",        /* 0x48 */ 
    "REG_V_LUT",        /* 0x4c */ 
    "REG_CSC_OFFPARA",  /* 0x50 */
};

static int jz47_dump_ipu_regs(int num)
{
  int i, total;
  if (num >= 0)
  {
    noah_kprintf ("ipu_reg: %s: 0x%x\n", ipu_regs_name[num >> 2], REG32(ipu_vbase + num));
  	return 1;
  }
  if (num == -1)
  {
    total = sizeof (ipu_regs_name) / sizeof (char *);
    for (i = 0; i < total; i++)
      noah_kprintf ("ipu_reg: %s: 0x%x\n", ipu_regs_name[i], REG32(ipu_vbase + (i << 2)));
  }
  return 1;
}

/* ================================================================================ */

static int jz47_get_output_panel_info (void)
{
  int output_mode = jz47_ipu_module.output_mode;

  /* For JZ4740 cpu, We haven't TV output and IPU through */
  if (jz47_cpu_type == 4740 && output_mode != IPU_OUT_FB && output_mode != IPU_OUT_MEM)
    jz47_ipu_module.output_mode = output_mode = IPU_OUT_FB;

  switch (output_mode)
  {
    case IPU_OUT_MEM:
      jz47_ipu_module.out_panel.w = 480;
      jz47_ipu_module.out_panel.h = 272;
      jz47_ipu_module.out_panel.bpp_byte = 4;
      jz47_ipu_module.out_panel.bytes_per_line = 480 * 4;
      jz47_ipu_module.out_panel.output_phy = get_phy_addr ((unsigned int)jz47_ipu_output_mem_ptr);
      break;

    case IPU_OUT_FB:
    default:
      /* set the output panel info */
      jz47_ipu_module.out_panel.w = lcd_get_width ();
      jz47_ipu_module.out_panel.h = lcd_get_height ();
      jz47_ipu_module.out_panel.bytes_per_line = lcd_get_line_length ();
      jz47_ipu_module.out_panel.bpp_byte = lcd_get_line_length () / lcd_get_width ();
      jz47_ipu_module.out_panel.output_phy = get_phy_addr ((unsigned int)lcd_get_frame ());
      
      noah_kprintf ("+++ w = %d, h = %d, line = %d, bpp = %d, phy=0x%08x\n",
                jz47_ipu_module.out_panel.w, jz47_ipu_module.out_panel.h, 
                jz47_ipu_module.out_panel.bytes_per_line, jz47_ipu_module.out_panel.bpp_byte,
                jz47_ipu_module.out_panel.output_phy);
      
      break;
  }
  return 1;
}

/* ================================================================================ */
/*
  x = -1, y = -1 is center display
  w = -1, h = -1 is orignal w,h
  w = -2, h = -2 is auto fit
  other: specify  by user
*/

static int jz47_calc_ipu_outsize_and_position (int x, int y, int w, int h)
{
  int dispscr_w, dispscr_h;
  int orignal_w = jz47_ipu_module.srcW;
  int orignal_h = jz47_ipu_module.srcH;

  /* record the orignal setting */
  jz47_ipu_module.out_x = x;
  jz47_ipu_module.out_y = y;
  jz47_ipu_module.out_w = w;
  jz47_ipu_module.out_h = h;
  // The MAX display area which can be used by ipu 
  dispscr_w = (x < 0) ? jz47_ipu_module.out_panel.w : (jz47_ipu_module.out_panel.w - x);
  dispscr_h = (y < 0) ? jz47_ipu_module.out_panel.h : (jz47_ipu_module.out_panel.h - y);

  // Orignal size playing or auto fit screen playing mode
  if ((w == -1 && h == -1 && (orignal_w > dispscr_w ||  orignal_h > dispscr_h)) || (w == -2 || h == -2))
  {
    float scale_h = (float)orignal_h / dispscr_h;
    float scale_w = (float)orignal_w / dispscr_w;
    if (scale_w > scale_h)
    {
      w = dispscr_w;
      h = (dispscr_w * orignal_h) / orignal_w;
    }
    else
    {
      h = dispscr_h;
      w = (dispscr_h * orignal_w) / orignal_h;
    }
  }
  
  // w,h is orignal w,h
  w = (w == -1)? orignal_w : w;
  h = (h == -1)? orignal_h : h;

  // w,h must < dispscr_w,dispscr_h
  w = (w > dispscr_w)? dispscr_w : w;
  h = (h > dispscr_h)? dispscr_h : h;

  // w,h must <= 31*(orignal_w, orignal_h)
  w = (w > 31 * orignal_w) ? (31 * orignal_w) : w;
  h = (h > 31 * orignal_h) ? (31 * orignal_h) : h;

  // calc output position out_x, out_y
  jz47_ipu_module.act_x = (x == -1) ? ((jz47_ipu_module.out_panel.w - w) / 2) : x;
  jz47_ipu_module.act_y = (y == -1) ? ((jz47_ipu_module.out_panel.h - h) / 2) : y;

  // set the resize_w, resize_h
  jz47_ipu_module.act_w = w;
  jz47_ipu_module.act_h = h;

  jz47_ipu_module.need_config_resize = 1;
  jz47_ipu_module.need_config_outputpara = 1;
  return 1;
}

/* ================================================================================ */
static void jz47_config_ipu_input_para (SwsContext *c, mp_image_t *mpi)
{
  unsigned int in_fmt;
  unsigned int srcFormat = c->srcFormat;

  in_fmt = INFMT_YCbCr420; // default value
  if (jz47_ipu_module.need_config_inputpara)
  {
    /* Set input Data format.  */
    switch (srcFormat)
    {
      case PIX_FMT_YUV420P:
        in_fmt = INFMT_YCbCr420;
      break;

      case PIX_FMT_YUV422P:
        in_fmt = INFMT_YCbCr422;
      break;

      case PIX_FMT_YUV444P:
        in_fmt = INFMT_YCbCr444;
      break;

      case PIX_FMT_YUV411P:
        in_fmt = INFMT_YCbCr411;
      break;
    }
    REG32 (ipu_vbase + REG_D_FMT) &= ~(IN_FMT_MASK);
    REG32 (ipu_vbase + REG_D_FMT) |= in_fmt;

    /* Set input width and height.  */
    REG32(ipu_vbase + REG_IN_FM_GS) = IN_FM_W(jz47_ipu_module.srcW) | IN_FM_H (jz47_ipu_module.srcH & ~1);

    /* Set the CSC COEF */
#ifdef JZ4765_OPT
    REG32(ipu_vbase + REG_CSC_C0_COEF) = YUV_CSC_C0;
    REG32(ipu_vbase + REG_CSC_C4_COEF) = YUV_CSC_C1;
    REG32(ipu_vbase + REG_CSC_C3_COEF) = YUV_CSC_C2;
    REG32(ipu_vbase + REG_CSC_C2_COEF) = YUV_CSC_C3;
    REG32(ipu_vbase + REG_CSC_C1_COEF) = YUV_CSC_C4;
#else
    REG32(ipu_vbase + REG_CSC_C0_COEF) = YUV_CSC_C0;
    REG32(ipu_vbase + REG_CSC_C1_COEF) = YUV_CSC_C1;
    REG32(ipu_vbase + REG_CSC_C2_COEF) = YUV_CSC_C2;
    REG32(ipu_vbase + REG_CSC_C3_COEF) = YUV_CSC_C3;
    REG32(ipu_vbase + REG_CSC_C4_COEF) = YUV_CSC_C4;
#endif

    if (jz47_cpu_type != 4740)
      REG32(ipu_vbase + REG_CSC_OFFPARA) = YUV_CSC_OFFPARA;

    /* Configure the stride for YUV.  */

    REG32(ipu_vbase + REG_Y_STRIDE) = mpi->stride[0];
#ifdef JZ4765_OPT
    if (mpi->ipu_line){
      REG32(ipu_vbase + REG_UV_STRIDE) = U_STRIDE(mpi->stride[1]) | V_STRIDE(mpi->stride[1]);
    }else{
      REG32(ipu_vbase + REG_UV_STRIDE) = U_STRIDE(mpi->stride[1]) | V_STRIDE(mpi->stride[2]);
    }
#else
    REG32(ipu_vbase + REG_UV_STRIDE) = U_STRIDE(mpi->stride[1]) | V_STRIDE(mpi->stride[2]);
#endif
  }

  /* Set the YUV addr.  */
#ifdef JZ4765_OPT
  if (mpi->ipu_line){
    REG32(ipu_vbase + REG_Y_ADDR) = get_phy_addr ((unsigned int)mpi->planes[0]);
    REG32(ipu_vbase + REG_U_ADDR) = get_phy_addr ((unsigned int)mpi->planes[1]);
    REG32(ipu_vbase + REG_V_ADDR) = get_phy_addr ((unsigned int)mpi->planes[1]);
  }else{
    REG32(ipu_vbase + REG_Y_ADDR) = get_phy_addr ((unsigned int)mpi->planes[0]);
    REG32(ipu_vbase + REG_U_ADDR) = get_phy_addr ((unsigned int)mpi->planes[1]);
    REG32(ipu_vbase + REG_V_ADDR) = get_phy_addr ((unsigned int)mpi->planes[2]);
  }
#else
  REG32(ipu_vbase + REG_Y_ADDR) = get_phy_addr ((unsigned int)mpi->planes[0]);
  REG32(ipu_vbase + REG_U_ADDR) = get_phy_addr ((unsigned int)mpi->planes[1]);
  REG32(ipu_vbase + REG_V_ADDR) = get_phy_addr ((unsigned int)mpi->planes[2]);
#endif

}

/* ================================================================================ */
static void jz47_config_ipu_resize_para ()
{
  int i, width_resize_enable, height_resize_enable;
  int width_up, height_up, width_lut_size, height_lut_size;

  width_resize_enable  = jz47_ipu_module.resize_para.width_resize_enable;
  height_resize_enable = jz47_ipu_module.resize_para.height_resize_enable;
  width_up  =  jz47_ipu_module.resize_para.width_up;
  height_up =  jz47_ipu_module.resize_para.height_up;
  width_lut_size  = jz47_ipu_module.resize_para.width_lut_size;
  height_lut_size = jz47_ipu_module.resize_para.height_lut_size;

  /* Enable the rsize configure.  */
  disable_rsize (ipu_vbase);

  if (width_resize_enable)
    enable_hrsize (ipu_vbase);

  if (height_resize_enable)
    enable_vrsize (ipu_vbase);

  /* select the resize algorithm.  */
#ifdef JZ4750_OPT
  if (jz47_cpu_type == 4760 && jz47_ipu_module.rsize_algorithm)     /* 0: liner, 1: bicube, 2: biliner.  */
    enable_ipu_bicubic(ipu_vbase);
  else
    disable_ipu_bicubic(ipu_vbase);
#endif

  /* Enable the scale configure.  */
  REG32 (ipu_vbase + REG_CTRL) &= ~((1 << V_SCALE_SHIFT) | (1 << H_SCALE_SHIFT));
  REG32 (ipu_vbase + REG_CTRL) |= (height_up << V_SCALE_SHIFT) | (width_up << H_SCALE_SHIFT);

  /* Set the LUT index.  */
  REG32 (ipu_vbase + REG_RSZ_COEF_INDEX) = (((height_lut_size - 1) << VE_IDX_SFT) 
                                            | ((width_lut_size - 1) << HE_IDX_SFT));

 /* set lut. */
  if (jz47_cpu_type == 4740)
  {
    if (height_resize_enable)
    {
      for (i = 0; i < height_lut_size; i++)
        REG32 (ipu_vbase + VRSZ_LUT_BASE + i * 4) = jz47_ipu_module.resize_para.height_lut_coef[i];
    }
    else
      REG32 (ipu_vbase + VRSZ_LUT_BASE) = ((128 << 2)| 0x3);

    if (width_resize_enable)
    {
      for (i = 0; i < width_lut_size; i++)
        REG32 (ipu_vbase + HRSZ_LUT_BASE + i * 4) = jz47_ipu_module.resize_para.width_lut_coef[i];
    }
    else
      REG32 (ipu_vbase + HRSZ_LUT_BASE) = ((128 << 2)| 0x3);
  }
  else
  {
    REG32 (ipu_vbase + VRSZ_LUT_BASE) = (1 << START_N_SFT);
    for (i = 0; i < height_lut_size; i++)
      if (jz47_cpu_type == 4760 && jz47_ipu_module.rsize_algorithm)     /* 0: liner, 1: bicube, 2: biliner.  */
      {
        REG32 (ipu_vbase + VRSZ_LUT_BASE) = jz47_ipu_module.resize_para.height_bicube_lut_coef[2*i + 0];
        REG32 (ipu_vbase + VRSZ_LUT_BASE) = jz47_ipu_module.resize_para.height_bicube_lut_coef[2*i + 1];
      }
      else
        REG32 (ipu_vbase + VRSZ_LUT_BASE) = jz47_ipu_module.resize_para.height_lut_coef[i];

    REG32 (ipu_vbase + HRSZ_LUT_BASE) = (1 << START_N_SFT);
    for (i = 0; i < width_lut_size; i++)
      if (jz47_cpu_type == 4760 && jz47_ipu_module.rsize_algorithm)     /* 0: liner, 1: bicube, 2: biliner.  */
      {
        REG32 (ipu_vbase + HRSZ_LUT_BASE) = jz47_ipu_module.resize_para.width_bicube_lut_coef[2*i + 0];
        REG32 (ipu_vbase + HRSZ_LUT_BASE) = jz47_ipu_module.resize_para.width_bicube_lut_coef[2*i + 1];
      }
      else
        REG32 (ipu_vbase + HRSZ_LUT_BASE) = jz47_ipu_module.resize_para.width_lut_coef[i];
  }
 
  /* dump the resize messages.  */
  if (DEBUG_LEVEL > 0)
  {
		noah_kprintf ("panel_w = %d, panel_h = %d, srcW = %d, srcH = %d\n",
            jz47_ipu_module.out_panel.w, jz47_ipu_module.out_panel.h, 
            jz47_ipu_module.srcW, jz47_ipu_module.srcH);
		noah_kprintf ("out_x = %d, out_y = %d, out_w = %d, out_h = %d\n", 
            jz47_ipu_module.out_x, jz47_ipu_module.out_y, 
            jz47_ipu_module.out_w, jz47_ipu_module.out_h);
		noah_kprintf ("act_x = %d, act_y = %d, act_w = %d, act_h = %d, outW = %d, outH = %d\n", 
            jz47_ipu_module.act_x, jz47_ipu_module.act_y, 
            jz47_ipu_module.act_w, jz47_ipu_module.act_h,
            jz47_ipu_module.resize_para.outW, jz47_ipu_module.resize_para.outH);
  }

  noah_open_osd(jz47_ipu_module.resize_para.outW,jz47_ipu_module.resize_para.outH);
}

/* ================================================================================ */
static void  jz47_config_ipu_output_para (SwsContext *c, int ipu_line)
{
  int frame_offset, out_x, out_y;
  int rsize_w, rsize_h, outW, outH;
  int output_mode = jz47_ipu_module.output_mode;
  unsigned int out_fmt, dstFormat = c->dstFormat;
   
  /* Get the output parameter from struct.  */
  outW = jz47_ipu_module.resize_para.outW;
  outH = jz47_ipu_module.resize_para.outH;
  out_x = jz47_ipu_module.act_x;
  out_y = jz47_ipu_module.act_y;
  rsize_w = jz47_ipu_module.act_w;
  rsize_h = jz47_ipu_module.act_h;

  /* outW must < resize_w and outH must < resize_h.  */
  outW = (outW <= rsize_w) ? outW : rsize_w;
  outH = (outH <= rsize_h) ? outH : rsize_h;

  /* calc the offset for output.  */
  frame_offset = (out_x + out_y * jz47_ipu_module.out_panel.w) * jz47_ipu_module.out_panel.bpp_byte;
  out_fmt = OUTFMT_RGB565;  // default value
  dstFormat = noah_get_out_format();

  /* clear some output control bits.  */
  disable_ipu_direct (ipu_vbase);
  disable_ipu_field  (ipu_vbase);

  switch (dstFormat)
  { 
    case PIX_FMT_RGB32:
    case PIX_FMT_RGB32_1:
      out_fmt = OUTFMT_RGB888;
      outW = outW << 2;
      break;

    case PIX_FMT_RGB555:
      out_fmt = OUTFMT_RGB555;
      outW = outW << 1;
      break;

    case PIX_FMT_RGB565:
    case PIX_FMT_BGR565:
      out_fmt = OUTFMT_RGB565;
      outW = outW << 1;
      break;
  }

  /* clear the OUT_DATA_FMT control bits.  */
  REG32 (ipu_vbase + REG_D_FMT) &= ~(OUT_FMT_MASK);

  switch (output_mode)
  {
    case IPU_OUT_FB:
    default:
      REG32(ipu_vbase + REG_OUT_ADDR) = jz47_ipu_module.out_panel.output_phy + frame_offset;
      REG32(ipu_vbase + REG_OUT_STRIDE) = jz47_ipu_module.out_panel.bytes_per_line;
      break;
  }

  REG32(ipu_vbase + REG_OUT_GS) = OUT_FM_W (outW) | OUT_FM_H (outH);
  REG32 (ipu_vbase + REG_D_FMT) |= out_fmt;
  REG32 (ipu_vbase + REG_CTRL) |= CSC_EN;

  if (ipu_line){
    REG32 (ipu_vbase + REG_D_FMT) |= BLK_SEL;
    REG32 (ipu_vbase + REG_D_FMT) |= OUTFMT_OFTBGR;
  }
}

static int jz47_config_stop_ipu ()
{
  int switch_mode, runing_mode;

  /* Get the runing mode class.  */
  if (jz47_cpu_type == 4740
      || jz47_ipu_module.ipu_working_mode == IPU_OUT_FB
      || jz47_ipu_module.ipu_working_mode == IPU_OUT_MEM)
    runing_mode = 'A';
  else 
    runing_mode = 'B';
  
  /* Get the switch mode class.  */
  if (jz47_cpu_type == 4740
      || jz47_ipu_module.output_mode == IPU_OUT_FB
      || jz47_ipu_module.output_mode == IPU_OUT_MEM)
    switch_mode = 'A';
  else 
    switch_mode = 'B';

  /* Base on the runing_mode and switch_mode, disable lcd or stop ipu.  */
  if (runing_mode == 'A' && switch_mode == 'A')
  {
    stop_ipu(ipu_vbase);
    while (ipu_is_enable(ipu_vbase) && (!polling_end_flag(ipu_vbase)))
      ;
    //clear_end_flag(ipu_vbase);
  }
  return 1;
}

/* ================================================================================ */

static int  jz47_config_run_ipu ()
{
  int output_mode = jz47_ipu_module.output_mode;

  /* set the ipu working mode.  */
  jz47_ipu_module.ipu_working_mode = output_mode;

  if (jz47_cpu_type == 4740)
  {
    clear_end_flag(ipu_vbase);
    run_ipu (ipu_vbase);
    return 1;
  }

  /* run ipu for different output mode.  */
  switch (output_mode)
  {
    case IPU_OUT_FB:
    case IPU_OUT_MEM:
    default:
      clear_end_flag(ipu_vbase);
      run_ipu (ipu_vbase);
      break;
  }

  return 1;
}

/* ================================================================================ */
int jz47_put_image_with_ipu (struct vf_instance *vf, mp_image_t *mpi, double pts)
{
  SwsContext *c = vf->priv->ctx;
  int image_outW, image_outH;
  /* Get the output panel information, calc the output position and shapes */
  if (!jz47_ipu_module_init)
  {
    if (tv_mode != -1)
      jz47_ipu_module.output_mode = tv_mode;

    if (! jz47_get_output_panel_info ())
      return 0;
    /* input size */
    jz47_ipu_module.srcW = c->srcW;
    jz47_ipu_module.srcH = c->srcH;
    /* output size */
    image_outW = (!scale_outW) ? c->dstW : scale_outW;
    image_outH = (!scale_outH) ? c->dstH : scale_outH;
    /* calculate input/output size. */
    jz47_calc_ipu_outsize_and_position (xpos, ypos, image_outW, image_outH);

    jz47_ipu_module.need_config_resize = 1;
    jz47_ipu_module.need_config_inputpara = 1;
    jz47_ipu_module.need_config_outputpara = 1;
    jz47_ipu_module_init = 1;
    reset_ipu (ipu_vbase);
  }

  if( noah_get_ipu_status() )
  {
	/* set some flag for normal.  */
	jz47_ipu_module.need_config_resize = 0;
	jz47_ipu_module.need_config_inputpara = 0;
	jz47_ipu_module.need_config_outputpara = 0;
	ipu_image_completed = 1;
	return 1;
  }
	
  /* calculate resize parameter.  */
  if (jz47_ipu_module.need_config_resize)
    jz47_calc_resize_para ();

  /* Following codes is used to configure IPU, so we need stop the ipu.  */
  jz47_config_stop_ipu ();

  /* configure the input parameter.  */
  jz47_config_ipu_input_para (c, mpi);

  /* configure the resize parameter.  */
  if (jz47_ipu_module.need_config_resize)
    jz47_config_ipu_resize_para ();

  /* configure the output parameter.  */
  if (jz47_ipu_module.need_config_outputpara)
    jz47_config_ipu_output_para (c, mpi->ipu_line);

  /* flush dcache if need.  */
  {
    __dcache_writeback_all();
#ifdef JZ47_IPU_CRC_VERIFY
    crc32_verify(mpi->planes[0], 1, img_area, img_area);
    crc32_verify(mpi->planes[1], 1, img_area/2, img_area/2);
    printf ("image_crc32 = 0x%08x\n", image_crc32);    
#endif
  }

  /* run ipu */
  jz47_config_run_ipu ();
//  while(ipu_is_enable(ipu_vbase) && (!polling_end_flag(ipu_vbase)));

  if (DEBUG_LEVEL > 1)
    jz47_dump_ipu_regs(-1);

  /* set some flag for normal.  */
  jz47_ipu_module.need_config_resize = 0;
  jz47_ipu_module.need_config_inputpara = 0;
  jz47_ipu_module.need_config_outputpara = 0;
  ipu_image_completed = 1;
  return 1;
}

/* Following function is the interface.  */
/* ================================================================================ */


void  ipu_image_start()
{
  ipu_image_completed = 0;
  jz47_ipu_module_init = 0;
}

static int visual = 0;
static int first_keyframe = 0;
void drop_image ()
{
  visual = 0;
}

int jz47_put_image (struct vf_instance *vf, mp_image_t *mpi, double pts)
{
  mp_image_t *dmpi=mpi->priv;
  SwsContext *c = vf->priv->ctx;

  if (!(mpi->flags & MP_IMGFLAG_DRAW_CALLBACK && dmpi))
  {
    if (mpi->pict_type == 1)  /* P_type */
    {
      visual = 1;
      first_keyframe = 1;
    }
    if (visual || (mpi->pict_type == 0)) /* I_type */
      jz47_put_image_with_ipu (vf, mpi, pts);
#ifdef OUTPUT_BLACK_NOKEYFRAME
    else if (!visual && !first_keyframe)
    {
      int i, j;
      unsigned char *p1, *p2, *p3;
      /* output the black screen.  */
      /* clear y buffer */
      p1 = (unsigned char *)mpi->planes[0];
      if (mpi->ipu_line)
        memset (p1, 0x0, (mpi->stride[0] * ((c->srcH + 15)/16)));
      else
        memset (p1, 0x0, mpi->stride[0] * c->srcH);

      /* clear uv buffer */
      p2 = (unsigned char *)mpi->planes[1];
      if (mpi->ipu_line)
       memset (p2, 0x80, (mpi->stride[1] * ((c->srcH + 15)/16)));
      else
      {
        memset (p2, 0x80, mpi->stride[1] * ((c->srcH + 1) / 2));
        p3 = (unsigned char *)mpi->planes[2];
        memset (p3, 0x80, mpi->stride[2] * ((c->srcH + 1) / 2));
      }

      /* flush cache. */
      memset (dcache, 0x2, 0x4000);
      jz47_put_image_with_ipu (vf, mpi, pts);	
    }
#endif

  }

  return 1;
}

void  IpuOutmodeChanged(int x,int y,int w, int h)
{
  /* caculate the x, y, w, h */
  jz47_calc_ipu_outsize_and_position(x, y,  w, h);
  jz47_ipu_module.need_config_resize = 1;
  jz47_ipu_module.need_config_outputpara = 1;
}

void ipu_init_lcd_size(int x,int y,int w, int h)
{
	xpos = x;
	ypos = y;
	scale_outW = w;  // 0,original value,else,user's value
	scale_outH = h;
}

unsigned int get_phy_addr (unsigned int vaddr)
{
	return vaddr&0x1FFFFFFF;
}

void *jz4740_alloc_frame (int align, int size)
{
	void *addr;
	if (align <= 0)
		align = 1;

	addr = uc_memalign(align, size);
	if ((unsigned int)addr & (align - 1))
	{
		kprintf("^^^^^^^^^^^^^^^ jz4760_alloc_frame align error! addr:%p align:%d size:%d\n", addr, align, size);
	}

	unsigned int offset = (unsigned int)addr & 0x3fffff;
	unsigned int max_size = 0x400000 - offset;
	if (size > max_size)
	{
		kprintf("^^^^^^^^^^^^^^^ jz4760_alloc_frame boundary error! addr:%p align:%d size:%d\n", addr, align, size);
		while(1);
	}

	return addr;
}

void *jz4740_alloc_frame_k0 (int align, int size)
{
	return (void *)(get_phy_addr(jz4740_alloc_frame(align, size)) & 0x1fffffff | 0x80000000);
}

#ifdef JZ47_IPU_CRC_VERIFY
#define CRC_32 0x04c11db7
unsigned int image_crc32;
unsigned int crc32_tab[256]={
         0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
         0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
         0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
         0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
         0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
         0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
         0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
         0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
         0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
         0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
         0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
         0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
         0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
         0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
         0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
         0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
         0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
         0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
         0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
         0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
         0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
         0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
         0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
         0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
         0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
         0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
         0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
         0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
         0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
         0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
         0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
         0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
         0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
         0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
         0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
         0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
         0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
         0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
         0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
         0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
         0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
         0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
         0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
         0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
         0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
         0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
         0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
         0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
         0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
         0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
         0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
         0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
         0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
         0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
         0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
         0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
         0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
         0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
         0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
         0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
         0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
         0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
         0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
         0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4, };

unsigned int frame_no=0;
unsigned int crc32_verify(unsigned char *buf, int line, int col, int stride)
{
  int i, j, counter = 0;

  for (j = 0; j < line; j++)
  {
    for (i = 0; i < col; i++){
      image_crc32 = ((image_crc32 << 8) | buf[i]) ^ (crc32_tab[(image_crc32 >> 24) & 0xff]);
    }
    buf += stride;
  }
}
#endif

