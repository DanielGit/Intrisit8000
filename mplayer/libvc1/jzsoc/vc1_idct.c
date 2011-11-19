#include "vc1_p1_type.h"
#include "vc1_dcore.h"
#include "jzmedia.h"
#define JZC_MXU_OPT

#define W1 0x4 //4
#define W2 0x6 //6
#define W3 0x9 //9
#define W4 0xC //12
#define W5 0xF //15
#define W6 0x10 //16
#define W7 0x11 //17
#define W8 0x16 //22
#define W9 0xA  //10

extern VC1_MB_DecARGs *dMB;
extern VC1_Frame_GlbARGs *dFRM;
extern uint32_t current_picture_ptr[2];

int OFFTAB[4] = {0, 4, 32 , 36};
static void add_pixels_clamped_aux(DCTELEM *block, uint8_t *pixels, int line_size)
{
  int i;
  DCTELEM *b = block - 2;
  uint8_t *dst = pixels - line_size;
  
  for(i=0;i<8;i++) {
    S32LDIV(xr1,dst,line_size,0);   // xr1 <- dst[3,2,1,0]
    S32LDI(xr2,b,4);
    S32LDI(xr3,b,4);
    
    Q8ADDE_AA(xr4,xr1,xr0,xr5);  //xr4<-sign_ext16(dst[3], dst[2]),xr5 <- sign_ext16(dst[1], dst[0])
    Q16ACCM_AA(xr4,xr3,xr2,xr5);//xr4<-(dst[3]+block[3],dst[2]+block[2]),xr5<-(dst[1]+block[1],dst[0]+block[0])
    Q16SAT(xr6,xr4,xr5);//xr6<-sat8(dst[3]+block[3]),sat8(dst[2]+block[2]),sat8(dst[1]+block[1]),sat8(dst[0]+block[0]);
    S32STD(xr6,dst,0);  // xr6 -> dst[3,2,1,0]

    S32LDD(xr1,dst,4);   // xr1 <- dst[7,6,5,4]
    S32LDI(xr2,b,4);
    S32LDI(xr3,b,4);
    
    Q8ADDE_AA(xr4,xr1,xr0,xr5);  // xr4 <- sign_ext16(dst[7], dst[6]),xr5 <- sign_ext16(dst[5], dst[4])
    Q16ACCM_AA(xr4,xr3,xr2,xr5);  // xr4 <- (dst[7]+block[7],dst[6]+block[6]), xr5 <- (dst[5]+block[5],dst[4]+block[4])
    Q16SAT(xr6,xr4,xr5);   // xr6 <- sat8(dst[7]+block[7]),sat8(dst[6]+block[6]),sat8(dst[5]+block[5]),sat8(dst[4]+block[4]);
    S32STD(xr6,dst,4);  // xr6 -> dst[7,6,5,4]
  }
}

static void put_pixels_clamped_aux(DCTELEM *block, uint8_t *pixels, int line_size)
{
  int i;
  DCTELEM *src = block;  
  uint8_t *dst = pixels;
  src -= 8;
  pixels -= line_size;
  
  for(i=0;i<8;i++) {
    S32LDI(xr2,src,0x10);
    S32LDD(xr3,src,4);
    
    S32LDD(xr8,src,8);
    S32LDD(xr9,src,12);
    
    Q16SAT(xr6,xr3,xr2);   // xr6 <- sat8(src[3]),sat8(src[2]),sat8(src[1]),sat8(src[0]);
    S32SDIV(xr6,pixels,line_size,0);  // xr6 -> pixels[3,2,1,0]
    
    Q16SAT(xr10,xr9,xr8);   // xr6 <- sat8(src[7]),sat8(src[6]),sat8(src[5]),sat8(src[4]);
    S32STD(xr10,pixels,4);  // xr6 -> pixels[7,6,5,4]
  }
}

static void vc1_inv_trans_8x8_aux(DCTELEM block[64],DCTELEM *dst,uint8_t *pixels,uint8_t idct_row)
{
  int i;
  DCTELEM *src,*blk;
  src = block - 8;
  blk = dst - 8; 

  S32I2M(xr15,W4<<16|W4);  //xr15:12|12
  S32I2M(xr14,W6<<16|W2);  //xr14:16|6
  S32I2M(xr13,W6<<16|W5);  //xr13:16|15
  S32I2M(xr12,W3<<16|W1);  //xr12:9 |4

  for(i=0;i<idct_row;i++){
    S32LDI(xr1,src,0x10); //xr1:src[1] src[0]
    S32LDD(xr2,src,0x4); //xr2:src[3] src[2]
    S32LDD(xr3,src,0x8); //xr3:src[5] src[4]
    S32LDD(xr4,src,0xc); //xr4:src[7] src[6]
    D16MUL_LW(xr5,xr1,xr15,xr6); //xr5:12*src[0] xr6:12*src[0]
    D16MAC_AS_LW(xr5,xr3,xr15,xr6); //xr5:t1 12*src[0]+12*src[4] xr6:t2 12*src[0]-12*src[4]
    D16MUL_LW(xr7,xr2,xr14,xr8);      //xr7:16*src[2] xr8:6*src[2]
    D16MAC_SA_LW(xr8,xr4,xr14,xr7);    //xr8:t4 6*src[2]-16*src[6] xr7:t3 16*src[2]+6*src[6]

    S32I2M(xr9,4);      
    D32ADD_AS(xr5,xr5,xr7,xr7);     //xr5:t5 t1+t3  xr7:t8 t1-t3
    D32ADD_AS(xr6,xr6,xr8,xr8);     //xr6:t6 t2+t4  xr8:t7 t2-t4

    D32ACC_AS(xr5,xr9,xr0,xr7);
    D32ACC_AS(xr6,xr9,xr0,xr8);

    D16MUL_HW(xr9,xr1,xr13,xr10);   //xr9:16*src[1] xr10:15*src[1]
    D16MUL_HW(xr11,xr1,xr12,xr1);   //xr11:9*src[1] xr1:4*src[1]
    D16MAC_SA_HW(xr11,xr2,xr13,xr9);//xr11:9*src[1]-16*src[3] xr9:16*src[1]+15*src[3]
    D16MAC_SS_HW(xr1,xr2,xr12,xr10);//xr1:4*src[1]-9*src[3]   xr10:15*src[1]-4*src[3]
    
    D16MAC_SA_HW(xr10,xr3,xr13,xr1);//xr10:15*src[1]-4*src[3]-16*src[5] xr1:4*src[1]-9*src[3]+15*src[5]
    D16MAC_AA_HW(xr9,xr3,xr12,xr11);//xr9:16*src[1]+15*src[3]+ 9*src[5] xr11:9*src[1]-16*src[3]+4*src[5]
    
    D16MAC_SA_HW(xr1,xr4,xr13,xr11);//xr1:t4 4*src[1]-9*src[3]+15*src[5]-16*src[7] xr11:t3 9*src[1]-16*src[3]+4*src[5]+15*src[7]
    D16MAC_SA_HW(xr10,xr4,xr12,xr9);//xr10:t2 15*src[1]-4*src[3]-16*src[5]-9*src[7] xr9:t1 16*src[1]+15*src[3]+ 9*src[5]+4*src[7]
 
    D32ADD_AS(xr6,xr6,xr10,xr10);   //xr6:t6+t2 xr10:t6-t2
    D32ADD_AS(xr5,xr5,xr9,xr9);     //xr5:t5+t1 xr9:t5-t1
    D32ADD_AS(xr8,xr8,xr11,xr11);   //xr8:t7+t3 xr11:t7-t3
    D32ADD_AS(xr7,xr7,xr1,xr1);     //xr7:t8+t4 xr1:t8-t4
    D32SARL(xr5,xr6,xr5,3);
    D32SARL(xr6,xr7,xr8,3);
    D32SARL(xr7,xr11,xr1,3);
    D32SARL(xr8,xr9,xr10,3);

    S32SDI(xr5, blk, 0x10);
    S32STD(xr6, blk, 0x4);
    S32STD(xr7, blk, 0x8);
    S32STD(xr8, blk, 0xc);
  } 

  for(; i<8; i++){
    S32SDI(xr0,blk,0x10);
    S32STD(xr0,blk,0x4);
    S32STD(xr0,blk,0x8);
    S32STD(xr0,blk,0xc);
  }
  uint8_t *pixs = pixels;
  if(idct_row == 1){
    src = dst - 4;    
    S32I2M(xr4,64);
    S32I2M(xr5,1);
    pixs = pixels - 4;
    for(i = 0; i < 2; i++){
      S32LDI(xr1,src,0x8);  //xr1: src[1] src[0]    
      S32LDD(xr11,src,0x4); //xr11:src[3] src[2]
      D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t5 12*src[1]  xr3:pt5 12*src[0]
      D16MUL_HW(xr10,xr15,xr11,xr9); //xr10:t5 12*src[3] xr9:pt5 12*src[2]

      D32ASUM_AA(xr2,xr4,xr4,xr3);
      D32ASUM_AA(xr10,xr4,xr4,xr9);

      D32SARL(xr6,xr2,xr3,7);
      D32SARL(xr7,xr10,xr9,7);
      Q16SAT(xr6,xr7,xr6);

      S32SDI(xr6,pixs,0x4); 
      S32STD(xr6,pixs,0x10); 
      S32STD(xr6,pixs,0x20); 
      S32STD(xr6,pixs,0x30); 

      D32ASUM_AA(xr2,xr5,xr5,xr3);
      D32ASUM_AA(xr10,xr5,xr5,xr9);     
      D32SARL(xr6,xr2,xr3,7);
      D32SARL(xr7,xr10,xr9,7);
      Q16SAT(xr6,xr7,xr6);
      S32STD(xr6,pixs,0x40); 
      S32STD(xr6,pixs,0x50); 
      S32STD(xr6,pixs,0x60); 
      S32STD(xr6,pixs,0x70);
    }  
  } 
  else{
    src = dst - 2;
    pixs = pixels - 2;
    for(i = 0; i < 4; i++){
      S32I2M(xr15,W4<<16|W2);  //xr15:12|6
      S32I2M(xr14,W5<<16|W6);  //xr14:15|16 
      S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]      
      S32LDD(xr2,src,0x20);  //xr2:src[17] src[16]
      S32LDD(xr3,src,0x40);  //xr3:src[33] src[32]
      S32LDD(xr4,src,0x60);  //xr4:src[49] src[48]
     
      D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
      D16MAC_AA_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]+12*src[33] xr6:12*src[0]+12*src[32]
      D16MAC_AA_LW(xr5,xr14,xr2,xr6); //xr5:12*src[1]+12*src[33]+16*src[17] xr6:12*src[0]+12*src[32]+16*src[16]
      D16MAC_AA_LW(xr5,xr15,xr4,xr6); //xr5:pt5  xr6:t5
     
      S32LDD(xr7,src,0x10);  //xr7:src[9] src[8]
      S32LDD(xr8,src,0x30);  //xr8:src[25] src[24]     
      S32LDD(xr9,src,0x50);  //xr9:src[41] src[40]
      S32LDD(xr10,src,0x70);  //xr10:src[57] src[56]

      D16MUL_LW(xr11,xr14,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
      D16MAC_AA_HW(xr11,xr14,xr8,xr13);   //xr11:16 * src[ 9] + 15 * src[25] xr13:16 * src[ 8]+15 * src[24]
      D16MAC_AA_HW(xr11,xr12,xr9,xr13);   //xr11:16 * src[ 9] + 15 * src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
      D16MAC_AA_LW(xr11,xr12,xr10,xr13);   //xr11:pt1   xr13:t1
     
      D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
      D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
      S32I2M(xr15,64);
      S32I2M(xr14,65);

      D32ACC_AA(xr5,xr15,xr0,xr6);
      D32ACC_AA(xr11,xr14,xr0,xr13);
      D32SARL(xr5,xr5,xr6,7);
      D32SARL(xr11,xr11,xr13,7);

      Q16SAT(xr5,xr11,xr5);
      S16SDI(xr5,pixs,2,0);
      S16STD(xr5,pixs,0x70,1);
    	
      S32I2M(xr15,W4<<16|W2);  //xr15:12|6
      S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
      D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
      D16MAC_SS_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
      D16MAC_AA_LW(xr5,xr15,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
      D16MAC_SS_LW(xr5,xr14,xr4,xr6); //xr5:pt6  xr6:t6

      D16MUL_HW(xr11,xr14,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
      D16MAC_SS_LW(xr11,xr12,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
      D16MAC_SS_LW(xr11,xr14,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
      D16MAC_SS_HW(xr11,xr12,xr10,xr13);   //xr11:pt1   xr13:t1
     
      D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
      D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
      S32I2M(xr15,64);
      S32I2M(xr14,65);
      D32ACC_AA(xr5,xr15,xr0,xr6);
      D32ACC_AA(xr11,xr14,xr0,xr13);
      D32SARL(xr5,xr5,xr6,7);
      D32SARL(xr11,xr11,xr13,7);

      Q16SAT(xr5,xr11,xr5);
      S16STD(xr5,pixs,0x10,0);
      S16STD(xr5,pixs,0x60,1);
    
      S32I2M(xr15,W4<<16|W2);  //xr15:12|6
      S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
      D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
      D16MAC_SS_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
      D16MAC_SS_LW(xr5,xr15,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
      D16MAC_AA_LW(xr5,xr14,xr4,xr6); //xr5:pt6  xr6:t6

      D16MUL_HW(xr11,xr12,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
      D16MAC_SS_LW(xr11,xr14,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
      D16MAC_AA_LW(xr11,xr12,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
      D16MAC_AA_HW(xr11,xr14,xr10,xr13);   //xr11:pt1   xr13:t1
     
      D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
      D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
      S32I2M(xr15,64);
      S32I2M(xr14,65);
      D32ACC_AA(xr5,xr15,xr0,xr6);
      D32ACC_AA(xr11,xr14,xr0,xr13);
      D32SARL(xr5,xr5,xr6,7);
      D32SARL(xr11,xr11,xr13,7);

      Q16SAT(xr5,xr11,xr5);
      S16STD(xr5,pixs,0x20,0);
      S16STD(xr5,pixs,0x50,1);

      S32I2M(xr15,W4<<16|W2);  //xr15:12|6
      S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
      D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
      D16MAC_AA_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
      D16MAC_SS_LW(xr5,xr14,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
      D16MAC_SS_LW(xr5,xr15,xr4,xr6); //xr5:pt6  xr6:t6

      D16MUL_LW(xr11,xr12,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
      D16MAC_SS_HW(xr11,xr12,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
      D16MAC_AA_HW(xr11,xr14,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
      D16MAC_SS_LW(xr11,xr14,xr10,xr13);   //xr11:pt1   xr13:t1
     
      D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
      D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
      S32I2M(xr15,64);
      S32I2M(xr14,65);
      D32ACC_AA(xr5,xr15,xr0,xr6);
      D32ACC_AA(xr11,xr14,xr0,xr13);
      D32SARL(xr5,xr5,xr6,7);
      D32SARL(xr11,xr11,xr13,7);

      Q16SAT(xr5,xr11,xr5);
      S16STD(xr5,pixs,0x30,0);
      S16STD(xr5,pixs,0x40,1);

    }   
  }
}

static void vc1_inv_trans_8x8_aux_test(DCTELEM block[64],DCTELEM *dst,uint8_t idct_row)
{
  int i;
  DCTELEM *src,*blk;
  src = block - 8;
  blk = dst - 8; 

  S32I2M(xr15,W4<<16|W4);  //xr15:12|12
  S32I2M(xr14,W6<<16|W2);  //xr14:16|6
  S32I2M(xr13,W6<<16|W5);  //xr13:16|15
  S32I2M(xr12,W3<<16|W1);  //xr12:9 |4

  for(i=0;i<idct_row;i++){
    S32LDI(xr1,src,0x10); //xr1:src[1] src[0]
    S32LDD(xr2,src,0x4); //xr2:src[3] src[2]
    S32LDD(xr3,src,0x8); //xr3:src[5] src[4]
    S32LDD(xr4,src,0xc); //xr4:src[7] src[6]
    D16MUL_LW(xr5,xr1,xr15,xr6); //xr5:12*src[0] xr6:12*src[0]
    D16MAC_AS_LW(xr5,xr3,xr15,xr6); //xr5:t1 12*src[0]+12*src[4] xr6:t2 12*src[0]-12*src[4]
    D16MUL_LW(xr7,xr2,xr14,xr8);      //xr7:16*src[2] xr8:6*src[2]
    D16MAC_SA_LW(xr8,xr4,xr14,xr7);    //xr8:t4 6*src[2]-16*src[6] xr7:t3 16*src[2]+6*src[6]

    S32I2M(xr9,4);      
    D32ADD_AS(xr5,xr5,xr7,xr7);     //xr5:t5 t1+t3  xr7:t8 t1-t3
    D32ADD_AS(xr6,xr6,xr8,xr8);     //xr6:t6 t2+t4  xr8:t7 t2-t4

    D32ACC_AS(xr5,xr9,xr0,xr7);
    D32ACC_AS(xr6,xr9,xr0,xr8);

    D16MUL_HW(xr9,xr1,xr13,xr10);   //xr9:16*src[1] xr10:15*src[1]
    D16MUL_HW(xr11,xr1,xr12,xr1);   //xr11:9*src[1] xr1:4*src[1]
    D16MAC_SA_HW(xr11,xr2,xr13,xr9);//xr11:9*src[1]-16*src[3] xr9:16*src[1]+15*src[3]
    D16MAC_SS_HW(xr1,xr2,xr12,xr10);//xr1:4*src[1]-9*src[3]   xr10:15*src[1]-4*src[3]
    
    D16MAC_SA_HW(xr10,xr3,xr13,xr1);//xr10:15*src[1]-4*src[3]-16*src[5] xr1:4*src[1]-9*src[3]+15*src[5]
    D16MAC_AA_HW(xr9,xr3,xr12,xr11);//xr9:16*src[1]+15*src[3]+ 9*src[5] xr11:9*src[1]-16*src[3]+4*src[5]
    
    D16MAC_SA_HW(xr1,xr4,xr13,xr11);//xr1:t4 4*src[1]-9*src[3]+15*src[5]-16*src[7] xr11:t3 9*src[1]-16*src[3]+4*src[5]+15*src[7]
    D16MAC_SA_HW(xr10,xr4,xr12,xr9);//xr10:t2 15*src[1]-4*src[3]-16*src[5]-9*src[7] xr9:t1 16*src[1]+15*src[3]+ 9*src[5]+4*src[7]
 
    D32ADD_AS(xr6,xr6,xr10,xr10);   //xr6:t6+t2 xr10:t6-t2
    D32ADD_AS(xr5,xr5,xr9,xr9);     //xr5:t5+t1 xr9:t5-t1
    D32ADD_AS(xr8,xr8,xr11,xr11);   //xr8:t7+t3 xr11:t7-t3
    D32ADD_AS(xr7,xr7,xr1,xr1);     //xr7:t8+t4 xr1:t8-t4
    D32SARL(xr5,xr6,xr5,3);
    D32SARL(xr6,xr7,xr8,3);
    D32SARL(xr7,xr11,xr1,3);
    D32SARL(xr8,xr9,xr10,3);

    S32SDI(xr5, blk, 0x10);
    S32STD(xr6, blk, 0x4);
    S32STD(xr7, blk, 0x8);
    S32STD(xr8, blk, 0xc);
  } 

  for(; i<8; i++){
    S32SDI(xr0,blk,0x10);
    S32STD(xr0,blk,0x4);
    S32STD(xr0,blk,0x8);
    S32STD(xr0,blk,0xc);
  }
  if(idct_row == 1){
    src = dst - 4;    
    S32I2M(xr4,64);
    S32I2M(xr5,1);
    for(i = 0; i < 2; i++){
      S32LDI(xr1,src,0x8);  //xr1: src[1] src[0]    
      S32LDD(xr11,src,0x4); //xr11:src[3] src[2]
      D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t5 12*src[1]  xr3:pt5 12*src[0]
      D16MUL_HW(xr10,xr15,xr11,xr9); //xr10:t5 12*src[3] xr9:pt5 12*src[2]
      D32ASUM_AA(xr2,xr4,xr4,xr3);
      D32ASUM_AA(xr10,xr4,xr4,xr9);

      D32SARL(xr6,xr2,xr3,7);
      S32STD (xr6,src,0x00);      
      S32STD (xr6,src,0x10);      
      S32STD (xr6,src,0x20);      
      S32STD (xr6,src,0x30);

      D32SARL(xr7,xr10,xr9,7);
      S32STD (xr7,src,0x04);      
      S32STD (xr7,src,0x14);      
      S32STD (xr7,src,0x24);      
      S32STD (xr7,src,0x34);  

      D32ASUM_AA(xr2,xr5,xr5,xr3);
      D32ASUM_AA(xr10,xr5,xr5,xr9);     
      D32SARL(xr6,xr2,xr3,7);
      S32STD (xr6,src,0x40);      
      S32STD (xr6,src,0x50);      
      S32STD (xr6,src,0x60);      
      S32STD (xr6,src,0x70); 

      D32SARL(xr7,xr10,xr9,7);
      S32STD (xr7,src,0x44);      
      S32STD (xr7,src,0x54);      
      S32STD (xr7,src,0x64);      
      S32STD (xr7,src,0x74); 
    }
  }
  else if(idct_row == 2){
    src = dst - 2 ;
    S32I2M(xr4,64);
    S32I2M(xr5,65);
    S32I2M(xr11,15<<16|9);
    S32I2M(xr10,16<<16|4);
    for(i = 0; i < 4; i++){
	S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]     
	S32LDD(xr6,src,0x10);  //xr6:src[9] src[8]  	
	D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t5 12*src[1]  xr3:pt5 12*src[0]
	D16MUL_HW(xr7,xr10,xr6,xr8); //xr7:t1  xr8:pt1

	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t1   xr9:t5-t1
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt1 xr8:pt5-pt1
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x0);
	S32STD(xr9,src,0x70);

	D16MUL_HW(xr7,xr11,xr6,xr8); //xr7:t2  xr8:pt2

	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t2   xr9:t5-t2
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt2 xr8:pt5-pt2
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x10);
	S32STD(xr9,src,0x60);

	D16MUL_LW(xr7,xr11,xr6,xr8); //xr7:t3  xr8:pt3

	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t3   xr9:t5-t3
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt3 xr8:pt5-pt3
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x20);
	S32STD(xr9,src,0x50);

	D16MUL_LW(xr7,xr10,xr6,xr8); //xr7:t1  xr8:pt1
	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t1   xr9:t5-t1
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt1 xr8:pt5-pt1
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x30);
	S32STD(xr9,src,0x40);
    }
  }
  else{
  src = dst - 2;
  for(i = 0; i < 4; i++){
    S32I2M(xr15,W4<<16|W2);  //xr15:12|6
    S32I2M(xr14,W5<<16|W6);  //xr14:15|16 
    S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]      
    S32LDD(xr2,src,0x20);  //xr2:src[17] src[16]
    S32LDD(xr3,src,0x40);  //xr3:src[33] src[32]
    S32LDD(xr4,src,0x60);  //xr4:src[49] src[48]
     
    D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
    D16MAC_AA_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]+12*src[33] xr6:12*src[0]+12*src[32]
    D16MAC_AA_LW(xr5,xr14,xr2,xr6); //xr5:12*src[1]+12*src[33]+16*src[17] xr6:12*src[0]+12*src[32]+16*src[16]
    D16MAC_AA_LW(xr5,xr15,xr4,xr6); //xr5:pt5  xr6:t5
     
    S32LDD(xr7,src,0x10);  //xr7:src[9] src[8]
    S32LDD(xr8,src,0x30);  //xr8:src[25] src[24]     
    S32LDD(xr9,src,0x50);  //xr9:src[41] src[40]
    S32LDD(xr10,src,0x70);  //xr10:src[57] src[56]

    D16MUL_LW(xr11,xr14,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
    D16MAC_AA_HW(xr11,xr14,xr8,xr13);   //xr11:16 * src[ 9] + 15 * src[25] xr13:16 * src[ 8]+15 * src[24]
    D16MAC_AA_HW(xr11,xr12,xr9,xr13);   //xr11:16 * src[ 9] + 15 * src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
    D16MAC_AA_LW(xr11,xr12,xr10,xr13);   //xr11:pt1   xr13:t1
     
    D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
    D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
    S32I2M(xr15,64);
    S32I2M(xr14,65);

    D32ACC_AA(xr5,xr15,xr0,xr6);
    D32ACC_AA(xr11,xr14,xr0,xr13);
    D32SARL(xr5,xr5,xr6,7);
    D32SARL(xr11,xr11,xr13,7);

    S32STD(xr5, src, 0x00);
    S32STD(xr11,src, 0x70);

    S32I2M(xr15,W4<<16|W2);  //xr15:12|6
    S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
    D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
    D16MAC_SS_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
    D16MAC_AA_LW(xr5,xr15,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
    D16MAC_SS_LW(xr5,xr14,xr4,xr6); //xr5:pt6  xr6:t6

    D16MUL_HW(xr11,xr14,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
    D16MAC_SS_LW(xr11,xr12,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
    D16MAC_SS_LW(xr11,xr14,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
    D16MAC_SS_HW(xr11,xr12,xr10,xr13);   //xr11:pt1   xr13:t1
     
    D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
    D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
    S32I2M(xr15,64);
    S32I2M(xr14,65);
    D32ACC_AA(xr5,xr15,xr0,xr6);
    D32ACC_AA(xr11,xr14,xr0,xr13);
    D32SARL(xr5,xr5,xr6,7);
    D32SARL(xr11,xr11,xr13,7);

    S32STD(xr5, src, 0x10);
    S32STD(xr11,src, 0x60);

    S32I2M(xr15,W4<<16|W2);  //xr15:12|6
    S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
    D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
    D16MAC_SS_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
    D16MAC_SS_LW(xr5,xr15,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
    D16MAC_AA_LW(xr5,xr14,xr4,xr6); //xr5:pt6  xr6:t6

    D16MUL_HW(xr11,xr12,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
    D16MAC_SS_LW(xr11,xr14,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
    D16MAC_AA_LW(xr11,xr12,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
    D16MAC_AA_HW(xr11,xr14,xr10,xr13);   //xr11:pt1   xr13:t1
     
    D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
    D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
    S32I2M(xr15,64);
    S32I2M(xr14,65);
    D32ACC_AA(xr5,xr15,xr0,xr6);
    D32ACC_AA(xr11,xr14,xr0,xr13);
    D32SARL(xr5,xr5,xr6,7);
    D32SARL(xr11,xr11,xr13,7);

    S32STD(xr5, src, 0x20);
    S32STD(xr11,src, 0x50);

    S32I2M(xr15,W4<<16|W2);  //xr15:12|6
    S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
    D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
    D16MAC_AA_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
    D16MAC_SS_LW(xr5,xr14,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
    D16MAC_SS_LW(xr5,xr15,xr4,xr6); //xr5:pt6  xr6:t6

    D16MUL_LW(xr11,xr12,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
    D16MAC_SS_HW(xr11,xr12,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
    D16MAC_AA_HW(xr11,xr14,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
    D16MAC_SS_LW(xr11,xr14,xr10,xr13);   //xr11:pt1   xr13:t1
     
    D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
    D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
    S32I2M(xr15,64);
    S32I2M(xr14,65);
    D32ACC_AA(xr5,xr15,xr0,xr6);
    D32ACC_AA(xr11,xr14,xr0,xr13);
    D32SARL(xr5,xr5,xr6,7);
    D32SARL(xr11,xr11,xr13,7);

    S32STD(xr5, src, 0x30);
    S32STD(xr11,src, 0x40);
  }
  }
}

static void vc1_inv_trans_4x8_aux(DCTELEM block[64], int n, DCTELEM *dst,uint8_t idct_row)
{
   int i;
   DCTELEM *src, *blk;
   int off;
   off = n << 2;
   src = block + off - 8;
   blk = dst + off - 8;
   S32I2M(xr15,W7<<16|W7);  //xr15:17|17
   S32I2M(xr14,W8<<16|W9);  //xr14:22|10
   S32I2M(xr13,4);
   for(i = 0; i < idct_row; i++){
     S32LDI(xr1,src,0x10); //xr1:src[1] src[0]
     S32LDD(xr2,src,0x4); //xr2:src[3] src[2]
     D16MUL_LW(xr3,xr1,xr15,xr4); //xr3:17*src[0] xr4:17*src[0]
     D16MAC_AS_LW(xr3,xr2,xr15,xr4); //xr3:t1  17*src[0]+src[2]*17 //xr4:t2   17*src[0]-17*src[2]
     D32ACC_AS(xr3,xr13,xr0,xr4);    //xr3:t1+4  xr4:t2+4
     D16MUL_HW(xr5,xr1,xr14,xr6);    //xr5:t3  22*src[1]  xr6:t5  10*src[1]
     D16MUL_HW(xr7,xr2,xr14,xr8);    //xr7:t4  22*src[3]  xr8:t6  10*src[3]
     D32ADD_AS(xr5,xr5,xr8,xr0);      //xr5:t3+t6
     D32ADD_AS(xr0,xr7,xr6,xr6);      //xr6:t4-t5
     D32ADD_AS(xr3,xr3,xr5,xr5);      //xr3:t1+4+t3+t6 xr5:t1+4-t3-t6
     D32ADD_AS(xr4,xr4,xr6,xr6);      //xr4:t2+4+t4-t5 xr6:t2+4-t4+t5
     D32SARL(xr3,xr6,xr3,3);
     D32SARL(xr5,xr5,xr4,3);

     S32SDI(xr3,blk,0x10);
     S32STD(xr5,blk,0x4);
   }

   for(; i<8; i++){
      S32SDI(xr0,blk,0x10);
      S32STD(xr0,blk,0x4);
   }
   if(idct_row == 1){
     S32I2M(xr15,W4<<16|W4);  //xr15:12|12
     src = dst + off;
     S32I2M(xr4,64);
     S32I2M(xr5,1);

     S32LDD(xr1,src,0x0);  //xr1: src[1] src[0]    
      S32LDD(xr11,src,0x4); //xr11:src[3] src[2]
      D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t5 12*src[1]  xr3:pt5 12*src[0]
      D16MUL_HW(xr10,xr15,xr11,xr9); //xr10:t5 12*src[3] xr9:pt5 12*src[2]
      D32ASUM_AA(xr2,xr4,xr4,xr3);
      D32ASUM_AA(xr10,xr4,xr4,xr9);

      D32SARL(xr6,xr2,xr3,7);
      S32STD (xr6,src,0x00);      
      S32STD (xr6,src,0x10);      
      S32STD (xr6,src,0x20);      
      S32STD (xr6,src,0x30);

      D32SARL(xr7,xr10,xr9,7);
      S32STD (xr7,src,0x04);      
      S32STD (xr7,src,0x14);      
      S32STD (xr7,src,0x24);      
      S32STD (xr7,src,0x34);  

      D32ASUM_AA(xr2,xr5,xr5,xr3);
      D32ASUM_AA(xr10,xr5,xr5,xr9);     
      D32SARL(xr6,xr2,xr3,7);
      S32STD (xr6,src,0x40);      
      S32STD (xr6,src,0x50);      
      S32STD (xr6,src,0x60);      
      S32STD (xr6,src,0x70); 

      D32SARL(xr7,xr10,xr9,7);
      S32STD (xr7,src,0x44);      
      S32STD (xr7,src,0x54);      
      S32STD (xr7,src,0x64);      
      S32STD (xr7,src,0x74); 
#if 0
    for(i = 0; i < 2; i++){
      S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]     
      D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t5 12*src[1]  xr3:pt5 12*src[0]
      D32ADD_AA(xr6,xr4,xr2,xr7);  //xr6:t5 + 64   xr7: t5 + 64  
      D32ADD_AA(xr8,xr4,xr3,xr9);  //xr8:pt5 + 64  xr9: pt5 + 64  
      D32SARL(xr6,xr6,xr8,7);
      S32STD (xr6,src,0x00);      
      S32STD (xr6,src,0x10);      
      S32STD (xr6,src,0x20);      
      S32STD (xr6,src,0x30);
      
      D32ADD_AA(xr6,xr5,xr2,xr7);  //xr6:t5 + 65   xr7: t5 + 65 
      D32ADD_AA(xr8,xr5,xr3,xr9);  //xr8:pt5 + 65  xr9: pt5 + 65  
      D32SARL(xr6,xr6,xr8,7);
      S32STD (xr6,src,0x40);      
      S32STD (xr6,src,0x50);      
      S32STD (xr6,src,0x60);      
      S32STD (xr6,src,0x70); 
    }
#endif
   }
  else if(idct_row == 2){
   src = dst + off - 2;
   S32I2M(xr15,W4<<16|W4);  //xr15:12|12
    S32I2M(xr4,64);
    S32I2M(xr5,65);
    S32I2M(xr11,15<<16|9);
    S32I2M(xr10,16<<16|4);
    for(i = 0; i < 2; i++){
	S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]     
	D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t5 12*src[1]  xr3:pt5 12*src[0]
	S32LDD(xr6,src,0x10);  //xr6:src[9] src[8]  	
	D16MUL_HW(xr7,xr10,xr6,xr8); //xr7:t1  xr8:pt1

	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t1   xr9:t5-t1
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt1 xr8:pt5-pt1
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x0);
	S32STD(xr9,src,0x70);

	D16MUL_HW(xr7,xr11,xr6,xr8); //xr7:t2  xr8:pt2

	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t2   xr9:t5-t2
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt2 xr8:pt5-pt2
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x10);
	S32STD(xr9,src,0x60);

	D16MUL_LW(xr7,xr11,xr6,xr8); //xr7:t3  xr8:pt3

	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t3   xr9:t5-t3
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt3 xr8:pt5-pt3
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x20);
	S32STD(xr9,src,0x50);

	D16MUL_LW(xr7,xr10,xr6,xr8); //xr7:t1  xr8:pt1
	D32ADD_AS(xr1,xr2,xr7,xr9);  //xr1:t5+t1   xr9:t5-t1
	D32ADD_AS(xr7,xr3,xr8,xr8);  //xr7:pt5+pt1 xr8:pt5-pt1
	
	D32ACC_AA(xr1,xr4,xr0,xr7);
	D32ACC_AA(xr9,xr5,xr0,xr8);
	D32SARL(xr1,xr1,xr7,7);
	D32SARL(xr9,xr9,xr8,7);
	S32STD(xr1,src,0x30);
	S32STD(xr9,src,0x40);
    }
  }
   else{
   src = dst + off - 2; 
   S32I2M(xr12,W3<<16|W1);  //xr12:9 |4
   for(i = 0; i < 2; i++){
     S32I2M(xr15,W4<<16|W2);  //xr15:12|6
     S32I2M(xr14,W5<<16|W6);  //xr14:15|16 
     S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]      
     S32LDD(xr2,src,0x20);  //xr2:src[17] src[16]
     S32LDD(xr3,src,0x40);  //xr3:src[33] src[32]
     S32LDD(xr4,src,0x60);  //xr4:src[49] src[48]

     D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
     D16MAC_AA_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]+12*src[33] xr6:12*src[0]+12*src[32]
     D16MAC_AA_LW(xr5,xr14,xr2,xr6); //xr5:12*src[1]+12*src[33]+16*src[17] xr6:12*src[0]+12*src[32]+16*src[16]
     D16MAC_AA_LW(xr5,xr15,xr4,xr6); //xr5:pt5  xr6:t5
     
     S32LDD(xr7,src,0x10);  //xr7:src[9] src[8]
     S32LDD(xr8,src,0x30);  //xr8:src[25] src[24]     
     S32LDD(xr9,src,0x50);  //xr9:src[41] src[40]
     S32LDD(xr10,src,0x70);  //xr10:src[57] src[56]

     D16MUL_LW(xr11,xr14,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
     D16MAC_AA_HW(xr11,xr14,xr8,xr13);   //xr11:16 * src[ 9] + 15 * src[25] xr13:16 * src[ 8]+15 * src[24]
     D16MAC_AA_HW(xr11,xr12,xr9,xr13);   //xr11:16 * src[ 9] + 15 * src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
     D16MAC_AA_LW(xr11,xr12,xr10,xr13);   //xr11:pt1   xr13:t1
     
     D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
     D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
     S32I2M(xr15,64);
     S32I2M(xr14,65);

     D32ACC_AA(xr5,xr15,xr0,xr6);
     D32ACC_AA(xr11,xr14,xr0,xr13);
     D32SARL(xr5,xr5,xr6,7);
     D32SARL(xr11,xr11,xr13,7);

     S32STD(xr5, src, 0x00);
     S32STD(xr11,src, 0x70);

     S32I2M(xr15,W4<<16|W2);  //xr15:12|6
     S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
     D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
     D16MAC_SS_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
     D16MAC_AA_LW(xr5,xr15,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
     D16MAC_SS_LW(xr5,xr14,xr4,xr6); //xr5:pt6  xr6:t6

     D16MUL_HW(xr11,xr14,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
     D16MAC_SS_LW(xr11,xr12,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
     D16MAC_SS_LW(xr11,xr14,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
     D16MAC_SS_HW(xr11,xr12,xr10,xr13);   //xr11:pt1   xr13:t1
     
     D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
     D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
     S32I2M(xr15,64);
     S32I2M(xr14,65);
     D32ACC_AA(xr5,xr15,xr0,xr6);
     D32ACC_AA(xr11,xr14,xr0,xr13);
     D32SARL(xr5,xr5,xr6,7);
     D32SARL(xr11,xr11,xr13,7);
     
     S32STD(xr5, src, 0x10);
     S32STD(xr11,src, 0x60);

     S32I2M(xr15,W4<<16|W2);  //xr15:12|6
     S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
     D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
     D16MAC_SS_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
     D16MAC_SS_LW(xr5,xr15,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
     D16MAC_AA_LW(xr5,xr14,xr4,xr6); //xr5:pt6  xr6:t6

     D16MUL_HW(xr11,xr12,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
     D16MAC_SS_LW(xr11,xr14,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
     D16MAC_AA_LW(xr11,xr12,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
     D16MAC_AA_HW(xr11,xr14,xr10,xr13);   //xr11:pt1   xr13:t1
     
     D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
     D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
     S32I2M(xr15,64);
     S32I2M(xr14,65);
     D32ACC_AA(xr5,xr15,xr0,xr6);
     D32ACC_AA(xr11,xr14,xr0,xr13);
     D32SARL(xr5,xr5,xr6,7);
     D32SARL(xr11,xr11,xr13,7);
     
     S32STD(xr5, src, 0x20);
     S32STD(xr11,src, 0x50);

     S32I2M(xr15,W4<<16|W2);  //xr15:12|6
     S32I2M(xr14,W5<<16|W6);  //xr14:15|16      
     D16MUL_HW(xr5,xr15,xr1,xr6);    //xr5:12*src[1]   xr6:12*src[0]
     D16MAC_AA_HW(xr5,xr15,xr3,xr6); //xr5:12*src[1]-12*src[33] xr6:12*src[0]-12*src[32]
     D16MAC_SS_LW(xr5,xr14,xr2,xr6); //xr5:12*src[1]+12*src[33]+6*src[17] xr6:12*src[0]+12*src[32]+6*src[16]
     D16MAC_SS_LW(xr5,xr15,xr4,xr6); //xr5:pt6  xr6:t6

     D16MUL_LW(xr11,xr12,xr7,xr13);   //xr11:16 * src[ 9] xr13:16 * src[ 8]
     D16MAC_SS_HW(xr11,xr12,xr8,xr13);   //xr11:16 * src[ 9] + 15*src[25] xr13:16 * src[ 8]+15 * src[24]
     D16MAC_AA_HW(xr11,xr14,xr9,xr13);   //xr11:16 * src[ 9] + 15*src[25]+9*src[41] xr13:16*src[8]+15*src[24]+9*src[40]
     D16MAC_SS_LW(xr11,xr14,xr10,xr13);   //xr11:pt1   xr13:t1
     
     D32ADD_AS(xr5,xr5,xr11,xr11);        //xr5:pt5 + pt1  xr11:pt5 - pt1
     D32ADD_AS(xr6,xr6,xr13,xr13);        //xr6:t5 + t1    xr13:t5 - t1
 
     S32I2M(xr15,64);
     S32I2M(xr14,65);
     D32ACC_AA(xr5,xr15,xr0,xr6);
     D32ACC_AA(xr11,xr14,xr0,xr13);
     D32SARL(xr5,xr5,xr6,7);
     D32SARL(xr11,xr11,xr13,7);
     
     S32STD(xr5, src, 0x30);
     S32STD(xr11,src, 0x40);
   }
   }
}

/** Do inverse transform on 4x4 part of block
*/
static void vc1_inv_trans_4x4_aux(DCTELEM block[64], int n, DCTELEM *dst,uint8_t idct_row)
{
  int i;
  DCTELEM *src, *blk;
  int off;
  off = OFFTAB[n];
  src = block + off - 8;
  blk = dst + off - 8;
  S32I2M(xr15,W7<<16|W7);  //xr15:17|17
  S32I2M(xr14,W8<<16|W9);  //xr14:22|10
  S32I2M(xr13,4);
  S32I2M(xr11,64<<16|64);
  for(i = 0; i < idct_row; i++){
    S32LDI(xr1,src,0x10); //xr1:src[1] src[0]
    S32LDD(xr2,src,0x4); //xr2:src[3] src[2]
    D16MUL_LW(xr3,xr1,xr15,xr4); //xr3:17*src[0] xr4:17*src[0]
    D16MAC_AS_LW(xr3,xr2,xr15,xr4); //xr3:t1  17*src[0]+src[2]*17 //xr4:t2   17*src[0]-17*src[2]  
    D32ACC_AS(xr3,xr13,xr0,xr4);    //xr3:t1+4  xr4:t2+4
    D16MUL_HW(xr5,xr1,xr14,xr6);    //xr5:t3  22*src[1]  xr6:t5  10*src[1]
    D16MUL_HW(xr7,xr2,xr14,xr8);    //xr7:t4  22*src[3]  xr8:t6  10*src[3]
    D32ADD_AS(xr5,xr5,xr8,xr0);      //xr5:t3+t6
    D32ADD_AS(xr0,xr7,xr6,xr6);      //xr6:t4-t5
    D32ADD_AS(xr3,xr3,xr5,xr5);      //xr3:t1+4+t3+t6 xr5:t1+4-t3-t6
    D32ADD_AS(xr4,xr4,xr6,xr6);      //xr4:t2+4+t4-t5 xr6:t2+4-t4+t5
    D32SARL(xr3,xr6,xr3,3);
    D32SARL(xr5,xr5,xr4,3);

    S32SDI(xr3,blk,0x10);
    S32STD(xr5,blk,0x4);
  }

  for(; i<4; i++){
    S32SDI(xr0,blk,0x10);
    S32STD(xr0,blk,0x4);
  }
  if(idct_row == 1){
    src = dst + off - 2;
    S32I2M(xr4,64);
    for(i = 0; i < 2; i++){
      S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]     
      D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t1 17*src[1]  xr3:pt1 17*src[0]
      D32ACC_AA(xr2,xr4,xr0,xr3);
      D32SARL(xr2,xr2,xr3,7);
      S32STD(xr2,src,0x0);
      S32STD(xr2,src,0x10);
      S32STD(xr2,src,0x20);
      S32STD(xr2,src,0x30);
    }
  }
  else if(idct_row == 2){
    S32I2M(xr12,64);
    src = dst + off - 2;
    for(i = 0; i < 2; i++){
      S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]
      S32LDD(xr2,src,0x10);  //xr2:src[9] src[8]
      D16MUL_LW(xr3,xr15,xr1,xr4); //xr3:t1 17*src[1] xr4:pt1 17*src[0]  
      D16MUL_HW(xr5,xr14,xr2,xr6); //xr5:t3 22*src[9] xr6:pt3 22*src[8]
      D16MUL_LW(xr7,xr14,xr2,xr8); //xr7:t5 10*src[9] xr8:pt5 10*src[8]

      D32ADD_AS(xr9,xr3,xr5,xr5);//xr9:t1+t3 xr5: t1-t3
      D32ADD_AS(xr10,xr4,xr6,xr6);//xr10:pt1+pt3 xr6:pt1-pt3
      D32ACC_AA(xr9,xr12,xr0,xr10);
      D32ACC_AA(xr5,xr12,xr0,xr6);
      
      D32SARL(xr9,xr9,xr10,7);
      D32SARL(xr5,xr5,xr6,7);
      S32STD(xr9,src,0x00);
      S32STD(xr5,src,0x30);      
      
      D32ADD_AS(xr3,xr3,xr7,xr7); 
      D32ADD_AS(xr4,xr4,xr8,xr8);
      D32ACC_AA(xr3,xr12,xr0,xr4);
      D32ACC_AA(xr7,xr12,xr0,xr8);

      D32SARL(xr3,xr3,xr4,7);
      D32SARL(xr7,xr7,xr8,7);
      S32STD(xr3,src,0x10);
      S32STD(xr7,src,0x20); 
    }
  }
  else{
  src = dst + off - 2;
  for(i = 0; i < 2; i++){
    S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]
    S32LDD(xr2,src,0x10);  //xr2:src[9] src[8]
    S32LDD(xr3,src,0x20);  //xr3:src[17] src[16]
    S32LDD(xr4,src,0x30);  //xr4:src[25] src[24]
    Q16ADD_AS_WW(xr5,xr1,xr3,xr6); //xr5:src[1]+src[17] src[0]+src[16] xr6:src[1]-src[17] src[0]-src[16]
    Q16SLL(xr7,xr5,xr6,xr8,4);     //xr7:(src[1]+src[17])<<4 (src[0]+src[16])<<4 xr8:(src[1]-src[17])<<4 (src[0]-src[16])<<4
    Q16ADD_AS_WW(xr5,xr7,xr5,xr0); //xr5:pt1 t1 
    Q16ADD_AS_WW(xr6,xr8,xr6,xr0); //xr6:pt2 t2
    Q16ACC_AS(xr5,xr11,xr0,xr6);
    Q16SLL(xr1,xr2,xr4,xr3,3);     //xr1:src[9]<<3 src[8]<<3  xr3:src[25]<<3 src[24]<<3
    Q16SLL(xr7,xr2,xr4,xr8,1);     //xr7:src[9]<<1 src[8]<<1  xr8:src[25]<<1 src[24]<<1
    Q16ADD_AS_WW(xr9,xr1,xr7,xr0); //xr9:pt5 t5 
    Q16ADD_AS_WW(xr10,xr3,xr8,xr0);//xr10:pt6 t6
    Q16SLL(xr1,xr1,xr3,xr3,1);     //xr1:src[9]<<16 src[8]<<16  xr3:src[25]<<16 src[24]<<16
    Q16SLL(xr2,xr2,xr4,xr4,2);     //xr2:src[9]<<2 src[8]<<2    xr4:src[25]<<2  src[24]<<2

    Q16ADD_AS_WW(xr1,xr1,xr2,xr0); 
    Q16ADD_AS_WW(xr3,xr3,xr4,xr0);
    Q16ADD_AS_WW(xr1,xr1,xr7,xr0); //xr1:pt3 t3
    Q16ADD_AS_WW(xr3,xr3,xr8,xr0); //xr3:pt4 t4
    Q16ADD_AS_WW(xr5,xr5,xr1,xr1); //xr5:pt1+pt3 t1+t3 xr1:pt1-pt3 t1-t3
    Q16ADD_AS_WW(xr6,xr6,xr3,xr3); //xr6:pt2+pt4 t2+t4 xr3:pt2-pt4 t2-t4
    Q16ADD_AS_WW(xr5,xr5,xr10,xr0); //xr5:pt1+pt3+pt6 t1+t3+t6
    Q16ADD_AS_WW(xr0,xr1,xr10,xr1); //xr1:pt1-pt3-pt6 t1-t3-t6
    Q16ADD_AS_WW(xr3,xr3,xr9,xr0);  //xr3:pt2-pt4+pt5 t2-t4+t5
    Q16ADD_AS_WW(xr0,xr6,xr9,xr6);  //xr6:pt2+pt4-pt5 t2+t4-t5
    Q16SAR(xr5,xr5,xr1,xr1,7);
    Q16SAR(xr3,xr3,xr6,xr6,7);
    S32STD(xr5,src,0x00);
    S32STD(xr3,src,0x10);
    S32STD(xr6,src,0x20);
    S32STD(xr1,src,0x30);
  }
  }
}

static void vc1_inv_trans_8x4_aux(DCTELEM block[64], int n,DCTELEM *dst,uint8_t idct_row)
{
  int i;
  DCTELEM *src, *blk;
  int off;
  off = n << 5;
  src = block + off - 8;
  blk = dst + off - 8;
  S32I2M(xr15,W4<<16|W4);  //xr15:12|12
  S32I2M(xr14,W6<<16|W2);  //xr14:16|6
  S32I2M(xr13,W6<<16|W5);  //xr13:16|15
  S32I2M(xr12,W3<<16|W1);  //xr12:9 |4
  for(i=0;i<idct_row;i++){
    S32LDI(xr1,src,0x10); //xr1:src[1] src[0]
    S32LDD(xr2,src,0x4); //xr2:src[3] src[2]
    S32LDD(xr3,src,0x8); //xr3:src[5] src[4]
    S32LDD(xr4,src,0xc); //xr4:src[7] src[6]
    D16MUL_LW(xr5,xr1,xr15,xr6); //xr5:12*src[0] xr6:12*src[0]
    D16MAC_AS_LW(xr5,xr3,xr15,xr6); //xr5:t1 12*src[0]+12*src[4] xr6:t2 12*src[0]-12*src[4]
    D16MUL_LW(xr7,xr2,xr14,xr8);      //xr7:16*src[2] xr8:6*src[2]
    D16MAC_SA_LW(xr8,xr4,xr14,xr7);    //xr8:t4 6*src[2]-16*src[6] xr7:t3 16*src[2]+6*src[6]
      
    D32ADD_AS(xr5,xr5,xr7,xr7);     //xr5:t5 t1+t3  xr7:t8 t1-t3
    D32ADD_AS(xr6,xr6,xr8,xr8);     //xr6:t6 t2+t4  xr8:t7 t2-t4
    S32I2M(xr9,4);
    D32ACC_AS(xr5,xr9,xr0,xr7);
    D32ACC_AS(xr6,xr9,xr0,xr8);
    D16MUL_HW(xr9,xr1,xr13,xr10);   //xr9:16*src[1] xr10:15*src[1]
    D16MUL_HW(xr11,xr1,xr12,xr1);   //xr11:9*src[1] xr1:4*src[1]
    D16MAC_SA_HW(xr11,xr2,xr13,xr9);//xr11:9*src[1]-16*src[3] xr9:16*src[1]+15*src[3]
    D16MAC_SS_HW(xr1,xr2,xr12,xr10);//xr1:4*src[1]-9*src[3]   xr10:15*src[1]-4*src[3]
    
    D16MAC_SA_HW(xr10,xr3,xr13,xr1);//xr10:15*src[1]-4*src[3]-16*src[5] xr1:4*src[1]-9*src[3]+15*src[5]
    D16MAC_AA_HW(xr9,xr3,xr12,xr11);//xr9:16*src[1]+15*src[3]+ 9*src[5] xr11:9*src[1]-16*src[3]+4*src[5]
    
    D16MAC_SA_HW(xr1,xr4,xr13,xr11);//xr1:t4 4*src[1]-9*src[3]+15*src[5]-16*src[7] xr11:t3 9*src[1]-16*src[3]+4*src[5]+15*src[7]
    D16MAC_SA_HW(xr10,xr4,xr12,xr9);//xr10:t2 15*src[1]-4*src[3]-16*src[5]-9*src[7] xr9:t1 16*src[1]+15*src[3]+ 9*src[5]+4*src[7]
    D32ADD_AS(xr5,xr5,xr9,xr9);     //xr5:t5+t1 xr9:t5-t1
    D32ADD_AS(xr6,xr6,xr10,xr10);   //xr6:t6+t2 xr10:t6-t2
    D32ADD_AS(xr8,xr8,xr11,xr11);   //xr8:t7+t3 xr11:t7-t3
    D32ADD_AS(xr7,xr7,xr1,xr1);     //xr7:t8+t4 xr1:t8-t4
    D32SARL(xr5,xr6,xr5,3);
    D32SARL(xr6,xr7,xr8,3);
    D32SARL(xr7,xr11,xr1,3);
    D32SARL(xr8,xr9,xr10,3);

    S32SDI(xr5,blk,0x10);
    S32STD(xr6,blk,0x4);
    S32STD(xr7,blk,0x8);
    S32STD(xr8,blk,0xc);
  }

  for(; i<4; i++){
    S32SDI(xr0,blk,0x10);
    S32STD(xr0,blk,0x4);
    S32STD(xr0,blk,0x8);
    S32STD(xr0,blk,0xc);
  }

  if(idct_row == 1){
    src = dst + off - 2;
    S32I2M(xr4,64);
    S32I2M(xr15,W7<<16|W7);  //xr15:17|17
    for(i = 0; i < 4; i++){
      S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]     
      D16MUL_HW(xr2,xr15,xr1,xr3); //xr2:t1 17*src[1]  xr3:pt1 17*src[0]
      D32ACC_AA(xr2,xr4,xr0,xr3);
      D32SARL(xr2,xr2,xr3,7);
      S32STD(xr2,src,0x0);
      S32STD(xr2,src,0x10);
      S32STD(xr2,src,0x20);
      S32STD(xr2,src,0x30);
    }
  }
  else if(idct_row == 2){
    S32I2M(xr12,64);
    S32I2M(xr15,W7<<16|W7);  //xr15:17|17
    S32I2M(xr14,W8<<16|W9);  //xr14:22|10  
    src = dst + off - 2;
    for(i = 0; i < 4; i++){
      S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]
      S32LDD(xr2,src,0x10);  //xr2:src[9] src[8]
      D16MUL_LW(xr3,xr15,xr1,xr4); //xr3:t1 17*src[1] xr4:pt1 17*src[0]  
      D16MUL_HW(xr5,xr14,xr2,xr6); //xr5:t3 22*src[9] xr6:pt3 22*src[8]
      D16MUL_LW(xr7,xr14,xr2,xr8); //xr7:t5 10*src[9] xr8:pt5 10*src[8]

      D32ADD_AS(xr9,xr3,xr5,xr5);//xr9:t1+t3 xr5: t1-t3
      D32ADD_AS(xr10,xr4,xr6,xr6);//xr10:pt1+pt3 xr6:pt1-pt3
      D32ACC_AA(xr9,xr12,xr0,xr10);
      D32ACC_AA(xr5,xr12,xr0,xr6);
      
      D32SARL(xr9,xr9,xr10,7);
      D32SARL(xr5,xr5,xr6,7);
      S32STD(xr9,src,0x00);
      S32STD(xr5,src,0x30);      
      
      D32ADD_AS(xr3,xr3,xr7,xr7); 
      D32ADD_AS(xr4,xr4,xr8,xr8);
      D32ACC_AA(xr3,xr12,xr0,xr4);
      D32ACC_AA(xr7,xr12,xr0,xr8);

      D32SARL(xr3,xr3,xr4,7);
      D32SARL(xr7,xr7,xr8,7);
      S32STD(xr3,src,0x10);
      S32STD(xr7,src,0x20); 
    }
  }
  else{
  src = dst + off - 2;
  for(i = 0; i < 4; i++){
    S32LDI(xr1,src,0x4);  //xr1:src[1] src[0]
    S32LDD(xr2,src,0x10);  //xr2:src[9] src[8]
    S32LDD(xr3,src,0x20);  //xr3:src[17] src[16]
    S32LDD(xr4,src,0x30);  //xr4:src[25] src[24]
    Q16ADD_AS_WW(xr5,xr1,xr3,xr6); //xr5:src[1]+src[17] src[0]+src[16] xr6:src[1]-src[17] src[0]-src[16]
    Q16SLL(xr7,xr5,xr6,xr8,4);     //xr7:(src[1]+src[17])<<4 (src[0]+src[16])<<4 xr8:(src[1]-src[17])<<4 (src[0]-src[16])<<4
    Q16ADD_AS_WW(xr5,xr7,xr5,xr0); //xr5:pt1 t1 
    Q16ADD_AS_WW(xr6,xr8,xr6,xr0); //xr6:pt2 t2
    S32I2M(xr11,64<<16|64);
    Q16ACC_AS(xr5,xr11,xr0,xr6);
    Q16SLL(xr1,xr2,xr4,xr3,3);     //xr1:src[9]<<3 src[8]<<3  xr3:src[25]<<3 src[24]<<3
    Q16SLL(xr7,xr2,xr4,xr8,1);     //xr7:src[9]<<1 src[8]<<1  xr8:src[25]<<1 src[24]<<1
    Q16ADD_AS_WW(xr9,xr1,xr7,xr0); //xr9:pt5 t5 
    Q16ADD_AS_WW(xr10,xr3,xr8,xr0);//xr10:pt6 t6
    Q16SLL(xr1,xr1,xr3,xr3,1);     //xr1:src[9]<<16 src[8]<<16  xr3:src[25]<<16 src[24]<<16
    Q16SLL(xr2,xr2,xr4,xr4,2);     //xr2:src[9]<<2 src[8]<<2    xr4:src[25]<<2  src[24]<<2

    Q16ADD_AS_WW(xr1,xr1,xr2,xr0); 
    Q16ADD_AS_WW(xr3,xr3,xr4,xr0);
    Q16ADD_AS_WW(xr1,xr1,xr7,xr0); //xr1:pt3 t3
    Q16ADD_AS_WW(xr3,xr3,xr8,xr0); //xr3:pt4 t4
    Q16ADD_AS_WW(xr5,xr5,xr1,xr1); //xr5:pt1+pt3 t1+t3 xr1:pt1-pt3 t1-t3
    Q16ADD_AS_WW(xr6,xr6,xr3,xr3); //xr6:pt2+pt4 t2+t4 xr3:pt2-pt4 t2-t4
    Q16ADD_AS_WW(xr5,xr5,xr10,xr0); //xr5:pt1+pt3+pt6 t1+t3+t6
    Q16ADD_AS_WW(xr0,xr1,xr10,xr1); //xr1:pt1-pt3-pt6 t1-t3-t6
    Q16ADD_AS_WW(xr3,xr3,xr9,xr0);  //xr3:pt2-pt4+pt5 t2-t4+t5
    Q16ADD_AS_WW(xr0,xr6,xr9,xr6);  //xr6:pt2+pt4-pt5 t2+t4-t5
    Q16SAR(xr5,xr5,xr1,xr1,7);
    Q16SAR(xr3,xr3,xr6,xr6,7);
    S32STD(xr5,src,0x00);
    S32STD(xr3,src,0x10);
    S32STD(xr6,src,0x20);
    S32STD(xr1,src,0x30);
  }
  }
}

static void add_pixels_aux(DCTELEM *block, DCTELEM n)
{
  int i;
  DCTELEM *src = block - 8;
  S32I2M(xr15, n<<16|n);    //xr15:n     n
  for(i=0; i<8; i++){
    S32LDI(xr1,src,0x10);   //xr1:src[1] src[0]
    S32LDD(xr2,src,0x4);    //xr2:src[3] src[2]
    S32LDD(xr3,src,0x8);    //xr3:src[5] src[4]
    S32LDD(xr4,src,0xc);    //xr4:src[7] src[6]

    Q16ADD_AA_WW(xr5,xr1,xr15,xr0);   //xr5:src[1] + n  src[0] + n
    Q16ADD_AA_WW(xr6,xr2,xr15,xr0);   //xr6:src[3] + n  src[2] + n
    Q16ADD_AA_WW(xr7,xr3,xr15,xr0);   //xr7:src[5] + n  src[4] + n
    Q16ADD_AA_WW(xr8,xr4,xr15,xr0);   //xr8:src[7] + n  src[6] + n
 
    S32STD(xr5,src,0);
    S32STD(xr6,src,4);
    S32STD(xr7,src,8);
    S32STD(xr8,src,0xc);
  }
}

static void vc1_dq_aux(int vpq,int16_t scale,DCTELEM * residual,int val,int mblen)
{

  int16_t tmp;    	 	
  int m;
  S16LDD(xr15,&vpq,0,3);
  S16LDD(xr14,&scale,0,3);
  if(dFRM->pquantizer){
    DCTELEM *blk1,*blkt;
    blk1=residual + mblen * 8;
    blkt=blk1-8;
    //val = dMB_L->idct_row[i];
    S32LDI(xr1,blkt,16);
    tmp=blk1[0];
    for(m = 0; m < val; m++)
      {
	D16MUL_WW(xr4,xr1,xr14,xr5);
	S32LDD(xr2,blkt,4);
	S32LDD(xr3,blkt,8);
	S32SFL(xr0,xr4,xr5,xr4,ptn3);
		    
	D16MUL_WW(xr6,xr2,xr14,xr7);
	S32LDD(xr9,blkt,12);
	S32LDI(xr1,blkt,16);
	S32SFL(xr0,xr6,xr7,xr6,ptn3);
  
	D16MUL_WW(xr8,xr3,xr14,xr5);
	S32STD(xr4,blkt,-16);
	S32STD(xr6,blkt,-12);
	S32SFL(xr0,xr8,xr5,xr8,ptn3);

	D16MUL_WW(xr12,xr9,xr14,xr10);
	S32STD(xr8,blkt,-8);
	S32SFL(xr0,xr12,xr10,xr12,ptn3);

	S32STD(xr12,blkt,-4);
      }
    blk1[0]=tmp;

  }else{
    val = (val*8) >> 1 ;
    DCTELEM *blk1 =  residual + mblen * 8;
    if(blk1[1]) {
      blk1[1] *= scale;
      blk1[1] += (blk1[1] < 0) ? -vpq : vpq;
    }
    for(m = 0; m < val-1; m++)
      {
	S32LDI(xr13,blk1,0x4);
	if(S32M2I(xr13)==0)
	  continue;
	D16MUL_WW(xr12,xr13,xr14,xr10);
	S32SFL(xr0,xr12,xr10,xr12,ptn3);
	D16CPS(xr9,xr15,xr12);
	D16MOVZ(xr9,xr12,xr0);
	Q16ADD_AA_WW(xr12,xr12,xr9,xr0);
	S32STD(xr12,blk1,0x0);
      }
  }

}
