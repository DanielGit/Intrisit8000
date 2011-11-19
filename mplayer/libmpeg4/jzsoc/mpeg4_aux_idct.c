#include "../../libjzcommon/jzmedia.h"
#include "mpeg4_dcore.h"
extern MPEG4_Frame_GlbARGs *dFRM;
extern MPEG4_MB_DecARGs *dMB_L;

#define  wxr5  0x5A827642
#define  wxr6  0x5A8230FC
#define  wxr7  0x7D876A6E
#define  wxr8  0x18F9471D
#define  wxr9  0x6A6E18F9
#define  wxr10  0x471D7D87

static int32_t whirl_idct[6] = {wxr5, wxr6, wxr7, wxr8, wxr9, wxr10};

static void dct_unquantize_mpeg2_intra_c_opt(DCTELEM *block, int n, int qscale);
static void dct_unquantize_mpeg2_inter_c_opt(DCTELEM *block, int n, int qscale);
static void dct_unquantize_h263_intra_c_opt(DCTELEM *block, int n, int qscale);
static void dct_unquantize_h263_inter_c_opt(DCTELEM *block, int n, int qscale);


static void ff_simple_idct_add_mxu (uint8_t *dest,int yuv_len, int line_size, DCTELEM *block)
{
  int i;
  int16_t  *blk;
  int32_t wf = (int32_t)whirl_idct;

  S32LDD(xr5, wf, 0x0);         // xr5(w7, w3)
  S32LDD(xr6, wf, 0x4);         // xr6(w9, w8)
  S32LDD(xr7, wf, 0x8);         // xr7(w11,w10)
  S32LDD(xr8, wf, 0xc);         // xr8(w13,w12)
  S32LDD(xr9, wf, 0x10);        // xr9(w6, w0)
  S32LDD(xr10,wf, 0x14);

  blk = block - 8;
  for(i=0; i<yuv_len; i++){
    S32LDI(xr1, blk, 0x10);        //  xr1 (x4, x0)
    S32LDD(xr2, blk, 0x4);        //  xr2 (x7, x3)
    S32LDD(xr3, blk, 0x8);        //  xr3 (x6, x1)
    S32LDD(xr4, blk, 0xc);        //  xr4 (x5, x2)
    
    S32SFL(xr1,xr1,xr2,xr2, ptn3);
    S32SFL(xr3,xr3,xr4,xr4, ptn3);
    
    D16MUL_WW(xr11, xr2, xr5, xr12);
    D16MAC_AA_WW(xr11,xr4,xr6,xr12);
    
    D16MUL_WW(xr13, xr2, xr6, xr14);
    D16MAC_SS_WW(xr13,xr4,xr5,xr14);
    
    D16MUL_HW(xr2, xr1, xr7, xr4);
    D16MAC_AS_LW(xr2,xr1,xr9,xr4);
    D16MAC_AS_HW(xr2,xr3,xr10,xr4);
    D16MAC_AS_LW(xr2,xr3,xr8,xr4);
    D16MACF_AA_WW(xr2, xr0, xr0, xr4);
    D16MACF_AA_WW(xr11, xr0, xr0, xr13);
    D16MACF_AA_WW(xr12, xr0, xr0, xr14);
    
    D16MUL_HW(xr4, xr1, xr8, xr15);
    D16MAC_SS_LW(xr4,xr1,xr10,xr15);
    D16MAC_AA_HW(xr4,xr3,xr9,xr15);
    D16MAC_SA_LW(xr4,xr3,xr7,xr15);
    Q16ADD_AS_WW(xr11,xr11,xr12,xr12); 
    D16MACF_AA_WW(xr15, xr0, xr0, xr4);
    
    Q16ADD_AS_WW(xr11, xr11, xr2, xr2);
    Q16ADD_AS_XW(xr12, xr12, xr15, xr15);
    
    S32SFL(xr11,xr11,xr12,xr12, ptn3);
    S32SFL(xr12,xr12,xr11,xr11, ptn3);
    
    S32STD(xr12, blk, 0x0);
    S32STD(xr11, blk, 0x4);
    S32STD(xr15, blk, 0x8);
    S32STD(xr2, blk, 0xc);
  } 

  blk  = block - 2;
  for (i = 0; i < 4; i++)               /* idct columns */
    {
      S32LDI(xr1, blk, 0x4);
      S32LDD(xr3, blk, 0x20);
      S32I2M(xr5,wxr5);
      S32LDD(xr11, blk, 0x40);
      S32LDD(xr13, blk, 0x60);

      D16MUL_HW(xr15, xr5, xr1, xr9);
      D16MAC_AA_HW(xr15,xr5,xr11,xr9);
      D16MACF_AA_WW(xr15, xr0, xr0, xr9);
      D16MUL_LW(xr10, xr5, xr3, xr9);
      D16MAC_AA_LW(xr10,xr6,xr13,xr9);
      D16MACF_AA_WW(xr10, xr0, xr0, xr9);
      S32LDD(xr2, blk, 0x10);
      S32LDD(xr4, blk, 0x30);
      Q16ADD_AS_WW(xr15,xr15,xr10,xr9);

      D16MUL_HW(xr10, xr5, xr1, xr1);
      D16MAC_SS_HW(xr10,xr5,xr11,xr1);
      D16MACF_AA_WW(xr10, xr0, xr0, xr1);
      D16MUL_LW(xr11, xr6, xr3, xr1);
      D16MAC_SS_LW(xr11,xr5,xr13,xr1);
      D16MACF_AA_WW(xr11, xr0, xr0, xr1);
      S32LDD(xr12, blk, 0x50);
      S32LDD(xr14, blk, 0x70);
      Q16ADD_AS_WW(xr10,xr10,xr11,xr1);

      D16MUL_HW(xr11, xr7, xr2, xr13);
      D16MAC_AA_LW(xr11,xr7,xr4,xr13);
      D16MAC_AA_LW(xr11,xr8,xr12,xr13);
      D16MAC_AA_HW(xr11,xr8,xr14,xr13);
      D16MACF_AA_WW(xr11, xr0, xr0, xr13);
     
       D16MUL_LW(xr3, xr7, xr2, xr13);
      D16MAC_SS_HW(xr3,xr8,xr4,xr13);
      D16MAC_SS_HW(xr3,xr7,xr12,xr13);
      D16MAC_SS_LW(xr3,xr8,xr14,xr13);
      D16MACF_AA_WW(xr3, xr0, xr0, xr13);

      D16MUL_LW(xr5, xr8, xr2, xr13);
      D16MAC_SS_HW(xr5,xr7,xr4,xr13);
      D16MAC_AA_HW(xr5,xr8,xr12,xr13);
      D16MAC_AA_LW(xr5,xr7,xr14,xr13);
      D16MACF_AA_WW(xr5, xr0, xr0, xr13);

      D16MUL_HW(xr2, xr8, xr2, xr13);
      D16MAC_SS_LW(xr2,xr8,xr4,xr13);
      D16MAC_AA_LW(xr2,xr7,xr12,xr13);
      D16MAC_SS_HW(xr2,xr7,xr14,xr13);
      D16MACF_AA_WW(xr2, xr0, xr0, xr13);

      Q16ADD_AS_WW(xr15,xr15,xr11,xr11);
      Q16ADD_AS_WW(xr10,xr10,xr3,xr3);
      Q16ADD_AS_WW(xr1,xr1,xr5,xr5);
      Q16ADD_AS_WW(xr9,xr9,xr2,xr2);

      S32STD(xr15, blk, 0x00);
      S32STD(xr10, blk, 0x10);
      S32STD(xr1, blk, 0x20);
      S32STD(xr9, blk, 0x30);
      S32STD(xr2, blk, 0x40);
      S32STD(xr5, blk, 0x50);
      S32STD(xr3, blk, 0x60);
      S32STD(xr11, blk, 0x70);
    }
  blk = block - 8;
  dest -= line_size;
  for (i=0; i< 8; i++) {
    S32LDIV(xr5, dest, line_size, 0x0);
    S32LDI(xr1, blk, 0x10);
    S32LDD(xr2, blk, 0x4);
    Q8ACCE_AA(xr2, xr5, xr0, xr1);

    S32LDD(xr6, dest, 0x4);
    S32LDD(xr3, blk, 0x8);
    S32LDD(xr4, blk, 0xc);
    Q8ACCE_AA(xr4, xr6, xr0, xr3);

    Q16SAT(xr5, xr2, xr1);
    S32STD(xr5, dest, 0x0);
    Q16SAT(xr6, xr4, xr3);
    S32STD(xr6, dest, 0x4);
  }
}

void ff_simple_idct_put_mxu(uint8_t *dest,int yuv_len, int line_size, DCTELEM *block)
{
  short *blk;
  unsigned int i;// mid0, mid1, tmp0, tmp1;
  uint8_t *dst_mid = dest;

  S32I2M(xr5,wxr5) ;         // xr5(w7, w3)
  S32I2M(xr6,wxr6) ;         // xr6(w9, w8)
  S32I2M(xr7,wxr7) ;         // xr7(w11,w10)
  S32I2M(xr8,wxr8) ;         // xr8(w13,w12)
  S32I2M(xr9,wxr9) ;         // xr9(w6, w0)  
  S32I2M(xr10,wxr10);       

  blk = block - 8;
  for (i = 0; i < yuv_len; i++)	/* idct rows */
    {
      int hi_b, lo_b, hi_c, lo_c;
       blk += 8;
      S32LDD(xr1, blk, 0x0);        //  xr1 (x4, x0)
      S32LDD(xr2, blk, 0x4);        //  xr2 (x7, x3)
      S32LDD(xr3, blk, 0x8);        //  xr3 (x6, x1)
      S32LDD(xr4, blk, 0xc);        //  xr4 (x5, x2)
      
      S32SFL(xr1,xr1,xr2,xr2, ptn3);  
      
      S32SFL(xr3,xr3,xr4,xr4, ptn3);  
      
      D16MUL_WW(xr11, xr2, xr5, xr12);         
      D16MAC_AA_WW(xr11,xr4,xr6,xr12);        
      D16MUL_WW(xr13, xr2, xr6, xr14);         
      D16MAC_SS_WW(xr13,xr4,xr5,xr14);        
      D16MUL_HW(xr2, xr1, xr7, xr4);         
      D16MAC_AS_LW(xr2,xr1,xr9,xr4);        
      D16MAC_AS_HW(xr2,xr3,xr10,xr4);        
      D16MAC_AS_LW(xr2,xr3,xr8,xr4);        

      D16MACF_AA_WW(xr2, xr0, xr0, xr4); 
      D16MACF_AA_WW(xr11, xr0, xr0, xr13);       
      D16MACF_AA_WW(xr12, xr0, xr0, xr14); 

      D16MUL_HW(xr4, xr1, xr8, xr15);         
      D16MAC_SS_LW(xr4,xr1,xr10,xr15);        
      D16MAC_AA_HW(xr4,xr3,xr9,xr15);        
      D16MAC_SA_LW(xr4,xr3,xr7,xr15);        
      Q16ADD_AS_WW(xr11,xr11,xr12,xr12);

      D16MACF_AA_WW(xr15, xr0, xr0, xr4); 
      Q16ADD_AS_WW(xr11, xr11, xr2, xr2);    
      Q16ADD_AS_XW(xr12, xr12, xr15, xr15);       


      S32SFL(xr11,xr11,xr12,xr12, ptn3);
      
      S32SFL(xr12,xr12,xr11,xr11, ptn3);
      
      S32STD(xr12, blk, 0x0);
      S32STD(xr11, blk, 0x4); 
      S32STD(xr15, blk, 0x8); 
      S32STD(xr2, blk, 0xc);       
    }

  blk = block - 2;

  if(yuv_len > 4)
    {
  for (i = 0; i < 4; i++)		/* idct columns */
    {
      int hi_b, lo_b, hi_c, lo_c;
       blk += 2;
      S32LDD(xr1, blk, 0x00);
      //S32LDI(xr1, blk, 0x04);
      S32LDD(xr3, blk, 0x20);
      S32I2M(xr5,wxr5);
      S32LDD(xr11, blk, 0x40);
      S32LDD(xr13, blk, 0x60);
       
      D16MUL_HW(xr15, xr5, xr1, xr9);
      D16MAC_AA_HW(xr15,xr5,xr11,xr9);        

      D16MACF_AA_WW(xr15, xr0, xr0, xr9); 

      D16MUL_LW(xr10, xr5, xr3, xr9);         
      D16MAC_AA_LW(xr10,xr6,xr13,xr9);        

      D16MACF_AA_WW(xr10, xr0, xr0, xr9); 

      S32LDD(xr2, blk, 0x10);
      S32LDD(xr4, blk, 0x30);
      Q16ADD_AS_WW(xr15,xr15,xr10,xr9);
      D16MUL_HW(xr10, xr5, xr1, xr1);         
      D16MAC_SS_HW(xr10,xr5,xr11,xr1);        

      D16MACF_AA_WW(xr10, xr0, xr0, xr1); 

      D16MUL_LW(xr11, xr6, xr3, xr1);         
      D16MAC_SS_LW(xr11,xr5,xr13,xr1);        

      D16MACF_AA_WW(xr11, xr0, xr0, xr1); 

      S32LDD(xr12, blk, 0x50);
      S32LDD(xr14, blk, 0x70);
      Q16ADD_AS_WW(xr10,xr10,xr11,xr1);
      D16MUL_HW(xr11, xr7, xr2, xr13);         
      D16MAC_AA_LW(xr11,xr7,xr4,xr13);        
      D16MAC_AA_LW(xr11,xr8,xr12,xr13);        
      D16MAC_AA_HW(xr11,xr8,xr14,xr13);        

      D16MACF_AA_WW(xr11, xr0, xr0, xr13); 

      D16MUL_LW(xr3, xr7, xr2, xr13);         
      D16MAC_SS_HW(xr3,xr8,xr4,xr13);        
      D16MAC_SS_HW(xr3,xr7,xr12,xr13);        
      D16MAC_SS_LW(xr3,xr8,xr14,xr13);        
      
      D16MACF_AA_WW(xr3, xr0, xr0, xr13); 

      D16MUL_LW(xr5, xr8, xr2, xr13);         
      D16MAC_SS_HW(xr5,xr7,xr4,xr13);        
      D16MAC_AA_HW(xr5,xr8,xr12,xr13);        
      D16MAC_AA_LW(xr5,xr7,xr14,xr13);        
      
      D16MACF_AA_WW(xr5, xr0, xr0, xr13); 

      D16MUL_HW(xr2, xr8, xr2, xr13);         
      D16MAC_SS_LW(xr2,xr8,xr4,xr13);        
      D16MAC_AA_LW(xr2,xr7,xr12,xr13);        
      D16MAC_SS_HW(xr2,xr7,xr14,xr13);        
      
      D16MACF_AA_WW(xr2, xr0, xr0, xr13); 

      Q16ADD_AS_WW(xr15,xr15,xr11,xr11);    
      Q16ADD_AS_WW(xr10,xr10,xr3,xr3);    
      Q16ADD_AS_WW(xr1,xr1,xr5,xr5);          
      Q16ADD_AS_WW(xr9,xr9,xr2,xr2);      
      // saturate
      Q16SAT(xr15, xr15, xr10);
      Q16SAT(xr1, xr1,  xr9);
      Q16SAT(xr2, xr2,  xr5);
      Q16SAT(xr3, xr3,  xr11);
  
      // store it
      //RCON BUFFER 
      dst_mid = dest + i * 2;
      S16STD(xr15, dst_mid, 0, 1);
      S16SDI(xr15, dst_mid, 16, 0);
      S16SDI(xr1, dst_mid, 16,  1);
      S16SDI(xr1, dst_mid, 16,  0);
      S16SDI(xr2, dst_mid, 16,  1);
      S16SDI(xr2, dst_mid, 16,  0);
      S16SDI(xr3, dst_mid, 16,  1);
      S16STD(xr3, dst_mid, 16,  0);
    }
    }
  else
    {
       	    for (i = 0; i < 4; i++)		/* idct columns */
	    {
	      int hi_b, lo_b, hi_c, lo_c;
	       blk += 2;
	      S32LDD(xr1, blk, 0x00);
	      //S32LDI(xr1, blk, 0x04);
	      S32LDD(xr3, blk, 0x20);
	      S32I2M(xr5,wxr5);
	      // S32LDD(xr11, blk, 0x40);
	      // S32LDD(xr13, blk, 0x60);

	      D16MUL_HW(xr15, xr5, xr1, xr9);
	      // D16MAC_AA_HW(xr15,xr5,xr11,xr9);        

	      D16MACF_AA_WW(xr15, xr0, xr0, xr9); 

	      D16MUL_LW(xr10, xr5, xr3, xr9);         
	      //  D16MAC_AA_LW(xr10,xr6,xr13,xr9);        

	      D16MACF_AA_WW(xr10, xr0, xr0, xr9); 

	      S32LDD(xr2, blk, 0x10);
	      S32LDD(xr4, blk, 0x30);
	      Q16ADD_AS_WW(xr15,xr15,xr10,xr9);
	      D16MUL_HW(xr10, xr5, xr1, xr1);         
	      // D16MAC_SS_HW(xr10,xr5,xr11,xr1);        

	      D16MACF_AA_WW(xr10, xr0, xr0, xr1); 

	      D16MUL_LW(xr11, xr6, xr3, xr1);         
	      // D16MAC_SS_LW(xr11,xr5,xr13,xr1);        

	      D16MACF_AA_WW(xr11, xr0, xr0, xr1); 

	      // S32LDD(xr12, blk, 0x50);
	      // S32LDD(xr14, blk, 0x70);
	      Q16ADD_AS_WW(xr10,xr10,xr11,xr1);
	      D16MUL_HW(xr11, xr7, xr2, xr13);         
	      D16MAC_AA_LW(xr11,xr7,xr4,xr13);        
	      //D16MAC_AA_LW(xr11,xr8,xr12,xr13);        
	      //D16MAC_AA_HW(xr11,xr8,xr14,xr13);        

	      D16MACF_AA_WW(xr11, xr0, xr0, xr13); 

	      D16MUL_LW(xr3, xr7, xr2, xr13);         
	      D16MAC_SS_HW(xr3,xr8,xr4,xr13);        
	      // D16MAC_SS_HW(xr3,xr7,xr12,xr13);        
	      //D16MAC_SS_LW(xr3,xr8,xr14,xr13);        

	      D16MACF_AA_WW(xr3, xr0, xr0, xr13); 

	      D16MUL_LW(xr5, xr8, xr2, xr13);         
	      D16MAC_SS_HW(xr5,xr7,xr4,xr13);        
	      //D16MAC_AA_HW(xr5,xr8,xr12,xr13);        
	      //D16MAC_AA_LW(xr5,xr7,xr14,xr13);        

	      D16MACF_AA_WW(xr5, xr0, xr0, xr13); 

	      D16MUL_HW(xr2, xr8, xr2, xr13);         
	      D16MAC_SS_LW(xr2,xr8,xr4,xr13);        
	      // D16MAC_AA_LW(xr2,xr7,xr12,xr13);        
	      //D16MAC_SS_HW(xr2,xr7,xr14,xr13);        

	      D16MACF_AA_WW(xr2, xr0, xr0, xr13); 

	      Q16ADD_AS_WW(xr15,xr15,xr11,xr11);    
	      Q16ADD_AS_WW(xr10,xr10,xr3,xr3);    
	      Q16ADD_AS_WW(xr1,xr1,xr5,xr5);          
	      Q16ADD_AS_WW(xr9,xr9,xr2,xr2);      
	      // saturate
	      Q16SAT(xr15, xr15, xr10);
	      Q16SAT(xr1, xr1,  xr9);
	      Q16SAT(xr2, xr2,  xr5);
	      Q16SAT(xr3, xr3,  xr11);

	      // store it
	      dst_mid = dest + i * 2;
	      S16STD(xr15, dst_mid, 0, 1);
	      S16SDI(xr15, dst_mid, 16, 0);
	      S16SDI(xr1, dst_mid, 16,  1);
	      S16SDI(xr1, dst_mid, 16,  0);
	      S16SDI(xr2, dst_mid, 16,  1);
	      S16SDI(xr2, dst_mid, 16,  0);
	      S16SDI(xr3, dst_mid, 16,  1);
	      S16STD(xr3, dst_mid, 16,  0);
	    } 
    }
}

static inline void add_dequant_dct_opt(DCTELEM *block, int i, uint8_t *dest, int line_size, int qscale)
{
    //int yuv_len;
    //yuv_len = (dMB->val[i]>>3) + 1;    
    int yuv_len = 8; 
    if (dMB_L->block_last_index[i] >= 0) {
      if (dFRM->mpeg_quant || dFRM->codec_id == 2){
        dct_unquantize_mpeg2_inter_c_opt(block, i, qscale);
      }
      else if (dFRM->out_format == 2 || dFRM->out_format == 1){
        dct_unquantize_h263_inter_c_opt(block, i, qscale);
      }
      ff_simple_idct_add_mxu(dest,yuv_len, line_size, block);
    }
}

static void dct_unquantize_mpeg2_inter_c_opt(DCTELEM *block, int n, int qscale)
{
#if 0
  int i, level, nCoeffs;
  const uint16_t *quant_matrix;
  int sum=-1;

  if(dFRM->alternate_scan) nCoeffs= 63;
  else nCoeffs= dMB_L->block_last_index[n];

  quant_matrix = dFRM->inter_matrix;
  for(i=0; i<=nCoeffs; i++) {
    int j= dFRM->permutated[i];
    level = block[j];
    if (level) {
      if (level < 0) {
	level = -level;
	level = (((level << 1) + 1) * qscale *
		 ((int) (quant_matrix[j]))) >> 4;
	level = -level;
      } else {
	level = (((level << 1) + 1) * qscale *
		 ((int) (quant_matrix[j]))) >> 4;
      }
      block[j] = level;
      sum+=level;
    }
  }
  block[63]^=sum&1;
#else
   int i, level, nCoeffs,sum;
   const uint16_t *quant_matrix;
   nCoeffs= dMB_L->val[n];
   quant_matrix = dFRM->inter_matrix;
   S32I2M(xr15,-1);
   S32I2M(xr5,1);
   for(i=0; i<=nCoeffs; i++) {
   level = block[i];  
   if (level) {
       S32I2M(xr1,level);                  
       S32CPS(xr2,xr1,xr1);
       S32MUL(xr0,xr3,qscale,quant_matrix[i]);         
       D32SLL(xr4,xr2,xr0,xr0,1);
       S32OR(xr6,xr4,xr5);
       D16MUL_WW(xr0,xr6,xr3,xr7);
       D32SLR(xr9,xr7,xr0,xr0,4);
       S32CPS(xr8,xr9,xr1); 
       block[i] = S32M2I(xr8);
       D32ASUM_AA(xr15,xr8,xr0,xr0);
    }
   }
   S32AND(xr13,xr15,xr5);
   S32I2M(xr14,block[63]);
   S32XOR(xr14,xr14,xr13);
   block[63] = S32M2I(xr14);
#endif
}

static void dct_unquantize_h263_inter_c_opt(DCTELEM *block, int n, int qscale)
{
  int i, level, qmul, qadd;
  int nCoeffs;

  //assert(dMB->block_last_index[n]>=0);
  qadd = (qscale - 1) | 1;
  qmul = qscale << 1;

  nCoeffs= dFRM->raster_end[ dMB_L->block_last_index[n] ];
  //nCoeffs= s->inter_scantable.raster_end[ s->block_last_index[n] ];

  for(i=0; i<=nCoeffs; i++) {
    level = block[i];
    if (level) {
      if (level < 0) {
	level = level * qmul - qadd;
      } else {
	level = level * qmul + qadd;
      }
      block[i] = level;
    }
  }
}

static inline void add_dct_opt(DCTELEM *block, int i, uint8_t *dest, int line_size)
{
  int yuv_len;
  yuv_len = (dMB_L->val[i]>>3) + 1;
  if (dMB_L->block_last_index[i] >= 0) {
    ff_simple_idct_add_mxu(dest,yuv_len,line_size, block);
  }
}

static inline void put_dct_opt(DCTELEM *block, int i, uint8_t *dest, int qscale)
{
  int yuv_len;
  yuv_len = (dMB_L->val[i]>>3) + 1;
  if (dFRM->mpeg_quant || dFRM->codec_id == 2){
    dct_unquantize_mpeg2_intra_c_opt(block, i, qscale);
  }
  else if (dFRM->out_format == 2 || dFRM->out_format == 1){
    dct_unquantize_h263_intra_c_opt(block, i, qscale);
  }
  ff_simple_idct_put_mxu(dest,yuv_len,16,block);
}

static void dct_unquantize_mpeg2_intra_c_opt(DCTELEM *block, int n, int qscale)
{
#if 1
  int i, level, nCoeffs;
  const uint16_t *quant_matrix;
  //volatile int *tmp_dbg = TCSM1_DBG_BUF;
#if 0
  if(dFRM->alternate_scan) nCoeffs= 63;
  else nCoeffs= dMB_L->block_last_index[n];
#else
  nCoeffs= dMB_L->val[n];
#endif
#if 1
  int dc_scale = (n<4) ? dMB_L->y_dc_scale:dMB_L->c_dc_scale; 
  block[0] = block[0] * dc_scale;
#else
  if (n < 4)
    block[0] = block[0] * dMB_L->y_dc_scale;
  else
    block[0] = block[0] * dMB_L->c_dc_scale;
#endif
  quant_matrix = dFRM->intra_matrix;
    
  for(i=1;i<=nCoeffs;i++) {
    //int j= dFRM->permutated[i];
    level = block[i];
    if (level) {
      if (level < 0) {
	level = -level;
	level = (int)(level * qscale * quant_matrix[i]) >> 3;
	level = -level;
      } else {
	level = (int)(level * qscale * quant_matrix[i]) >> 3;
      }
      block[i] = level;
    }
  }
#else
  int i, level, nCoeffs;
  const uint16_t *quant_matrix;
  nCoeffs=((dMB_L->val[n]>>1) +1);  
  int dc_scale = (n<4) ? dMB_L->y_dc_scale:dMB_L->c_dc_scale; 
  quant_matrix = dFRM->intra_matrix;
  S32I2M(xr5,qscale);
  S32LUI(xr9,1,0);
  S32MUL(xr0,xr6,block[0],dc_scale);
  //D16MUL_WW(xr0,xr6,xr9,xr6);
  block-=2;
  quant_matrix-=2;
  for(i=0;i<nCoeffs;i++) {
    S32LDI(xr1,block,4);
    S32LDI(xr2,quant_matrix,4);
    D16MUL_LW(xr13,xr9,xr1,xr14);
    D16CPS(xr1,xr1,xr1);
    D16MUL_LW(xr7,xr5,xr2,xr8);
    S32SFL(xr0,xr7,xr8,xr2,3);
    D16MUL_WW(xr7,xr1,xr2,xr8);
    D32SLR(xr7,xr7,xr8,xr8,3);
    S32CPS(xr10,xr7,xr13);
    S32CPS(xr11,xr8,xr14);
    S32SFL(xr0,xr10,xr11,xr12,3);
    S32STD(xr12,block,0);
  }	    
  S16STD(xr6,block-(nCoeffs*2-2),0,0);//xr6 to data[0]
#endif
}

static void dct_unquantize_h263_intra_c_opt(DCTELEM *block, int n, int qscale)
{
#if 0
  int i, level, qmul, qadd;
  int nCoeffs;

  qmul = qscale << 1;

  if (!dFRM->h263_aic) {
    if (n < 4)
      block[0] = block[0] * dMB_L->y_dc_scale;
    else
      block[0] = block[0] * dMB_L->c_dc_scale;
    qadd = (qscale - 1) | 1;
  }else{
    qadd = 0;
  }
  if(dMB_L->ac_pred)
    nCoeffs=63;
  else
    nCoeffs= dFRM->raster_end[ dMB_L->block_last_index[n] ];
  for(i=1; i<=nCoeffs; i++) {
    level = block[i];
    if (level) {
      if (level < 0) {
	level = level * qmul - qadd;
      } else {
	level = level * qmul + qadd;
      }
      block[i] = level;
    }
  }

#else
  int i, level, qmul, qadd;
  int nCoeffs;
  
  S32LUI(xr9,1,0);
    S32I2M(xr1,qscale);
    D32SLL(xr5,xr1,xr0,xr0,1);
    if (!dFRM->h263_aic) { 
      int dc_scale = (n<4) ? dMB_L->y_dc_scale:dMB_L->c_dc_scale;
      S32MUL(xr0,xr6,block[0],dc_scale);
      S32AND(xr15,xr1,xr9);
      //block[0] = S32M2I(xr6);  
      S32MOVN(xr2,xr15,xr1);
      D32ADD_SS(xr1,xr1,xr9,xr3);
      S32MOVZ(xr2,xr15,xr1);
    }else{
      S32I2M(xr2,0);
    }
    nCoeffs=(dMB_L->val[n] == 0)?0:((dMB_L->val[n])>>1)+1;
    S32SFL(xr0,xr2,xr2,xr2,3);  
    block-=2;
    for(i=0; i<nCoeffs; i++) {
      S32LDI(xr8,block,4);
      if (S32M2I(xr8)==0)
        continue;
      D16CPS(xr13,xr2,xr8); 
      D16MOVZ(xr13,xr8,xr0); 
      D16MADL_AA_LW(xr13,xr5,xr8,xr12);      
      S32STD(xr12,block,0x0); 
    }
    S16STD(xr6,block-(nCoeffs*2-2),0,0);//xr6 to data[0]
#endif

}
