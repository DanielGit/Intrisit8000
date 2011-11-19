#if 0
void rv40_dequant4x4(DCTELEM *block, int Qdc, int Q, uint32_t *dst, int n)
{
  int i, j;

  dst[0] = (block[0] * Qdc + 8) >> 4;
  for(i = 0; i < 4; i++)
    for(j = !i; j < 4; j++)
      {
	if(block[j + i*4])
	  dst[j + i*4] = (block[j + i*4] * Q + 8) >> 4;
      }
}
#else
#include"../../libjzcommon/jzmedia.h"
static void rv40_dequant4x4(DCTELEM *block,uint32_t *dst, int n)
{
  int i;
  uint32_t src=block-4;
  uint32_t dst_t = dst-4;
#if 0
  for(i = 0; i < n; i++){   
    S32LDI(xr1,src,0x8);
    S32LDD(xr2,src,0x4);
    S32LDI(xr7,src,0x8);
    S32LDD(xr8,src,0x4);

    D16MUL_LW(xr4,xr12,xr1,xr3);
    D16MUL_LW(xr10,xr12,xr7,xr9);
    D16MUL_LW(xr14,xr12,xr8,xr15);
    D32ASUM_AA(xr3,xr13,xr13,xr4);
    D16MUL_LW(xr6,xr12,xr2,xr5);
    D32SLR(xr3,xr3,xr4,xr4,4);
    D32ASUM_AA(xr5,xr13,xr13,xr6);
    D32ASUM_AA(xr9,xr13,xr13,xr10);
    D32SLR(xr5,xr5,xr6,xr6,4);
    D32SLR(xr9,xr9,xr10,xr10,4);
    D32ASUM_AA(xr15,xr13,xr13,xr14);

    S32SDI(xr3,dst_t,0x10);
    S32STD(xr4,dst_t,0x4);
    S32STD(xr5,dst_t,0x8);
    S32STD(xr6,dst_t,0xc);

    D32SLR(xr15,xr15,xr14,xr14,4);
    S32SDI(xr9,dst_t,0x10);
    S32STD(xr10,dst_t,0x4);
    S32STD(xr15,dst_t,0x8);
    S32STD(xr14,dst_t,0xc);      
  }
#else
  /////////////////////     
  if(n == 1){
    S32LDI(xr1,src,0x8);
    S32LDI(xr2,src,0x8);
    S32LDI(xr7,src,0x8);
    S32LDI(xr8,src,0x8);
	  
    D16MUL_XW(xr4,xr12,xr1,xr3);
    D16MUL_LW(xr10,xr12,xr7,xr9);
    D16MUL_LW(xr14,xr12,xr8,xr15);
    D32ASUM_AA(xr3,xr13,xr13,xr4);
    D16MUL_LW(xr6,xr12,xr2,xr5);
    D32SLR(xr3,xr3,xr4,xr4,4);
    D32ASUM_AA(xr5,xr13,xr13,xr6);
    D32ASUM_AA(xr9,xr13,xr13,xr10);
    D32SLR(xr5,xr5,xr6,xr6,4);
    D32SLR(xr9,xr9,xr10,xr10,4);
    D32ASUM_AA(xr15,xr13,xr13,xr14);

    S32SDI(xr3,dst_t,0x10);
    S32STD(xr4,dst_t,0x4);
    S32STD(xr0,dst_t,0x8);
    S32STD(xr0,dst_t,0xc);

    S32SDI(xr5,dst_t,0x10);
    S32STD(xr6,dst_t,0x4);
    S32STD(xr0,dst_t,0x8);
    S32STD(xr0,dst_t,0xc);

    D32SLR(xr15,xr15,xr14,xr14,4);
    S32SDI(xr9,dst_t,0x10);
    S32STD(xr10,dst_t,0x4);
    S32STD(xr0,dst_t,0x8);
    S32STD(xr0,dst_t,0xc);
    //S32STD(xr15,dst_t,0x8);
    //S32STD(xr14,dst_t,0xc);
    S32SDI(xr15,dst_t,0x10);
    S32STD(xr14,dst_t,0x4);
    S32STD(xr0,dst_t,0x8);
    S32STD(xr0,dst_t,0xc);
  }
  else if(n==2)
    {
      S32LDI(xr1,src,0x8);
      S32LDD(xr2,src,0x4);
      S32LDI(xr7,src,0x8);
      S32LDD(xr8,src,0x4);
	  
      D16MUL_XW(xr4,xr12,xr1,xr3);
      D16MUL_LW(xr10,xr12,xr7,xr9);
      D16MUL_LW(xr14,xr12,xr8,xr15);
      D32ASUM_AA(xr3,xr13,xr13,xr4);
      D16MUL_LW(xr6,xr12,xr2,xr5);
      D32SLR(xr3,xr3,xr4,xr4,4);
      D32ASUM_AA(xr5,xr13,xr13,xr6);
      D32ASUM_AA(xr9,xr13,xr13,xr10);
      D32SLR(xr5,xr5,xr6,xr6,4);
      D32SLR(xr9,xr9,xr10,xr10,4);
      D32ASUM_AA(xr15,xr13,xr13,xr14);

      S32SDI(xr3,dst_t,0x10);
      S32STD(xr4,dst_t,0x4);
      S32STD(xr5,dst_t,0x8);
      S32STD(xr6,dst_t,0xc);

      D32SLR(xr15,xr15,xr14,xr14,4);
      S32SDI(xr9,dst_t,0x10);
      S32STD(xr10,dst_t,0x4);
      S32STD(xr15,dst_t,0x8);
      S32STD(xr14,dst_t,0xc);

      S32SDI(xr0,dst_t,0x10);
      S32STD(xr0,dst_t,0x4);
      S32STD(xr0,dst_t,0x8);
      S32STD(xr0,dst_t,0xc);
	  
      S32SDI(xr0,dst_t,0x10);
      S32STD(xr0,dst_t,0x4);
      S32STD(xr0,dst_t,0x8);
      S32STD(xr0,dst_t,0xc);	  
    }

  else
    {
      S32LDI(xr1,src,0x8);
      S32LDD(xr2,src,0x4);
      S32LDI(xr7,src,0x8);
      S32LDD(xr8,src,0x4);

      D16MUL_XW(xr4,xr12,xr1,xr3);
      D16MUL_LW(xr10,xr12,xr7,xr9);
      D16MUL_LW(xr14,xr12,xr8,xr15);
      D32ASUM_AA(xr3,xr13,xr13,xr4);
      D16MUL_LW(xr6,xr12,xr2,xr5);
      D32SLR(xr3,xr3,xr4,xr4,4);
      D32ASUM_AA(xr5,xr13,xr13,xr6);
      D32ASUM_AA(xr9,xr13,xr13,xr10);
      D32SLR(xr5,xr5,xr6,xr6,4);
      D32SLR(xr9,xr9,xr10,xr10,4);
      D32ASUM_AA(xr15,xr13,xr13,xr14);

      S32SDI(xr3,dst_t,0x10);
      S32STD(xr4,dst_t,0x4);
      S32STD(xr5,dst_t,0x8);
      S32STD(xr6,dst_t,0xc);

      D32SLR(xr15,xr15,xr14,xr14,4);
      S32SDI(xr9,dst_t,0x10);
      S32STD(xr10,dst_t,0x4);
      S32STD(xr15,dst_t,0x8);
      S32STD(xr14,dst_t,0xc);

      S32LDI(xr1,src,0x8);
      S32LDD(xr2,src,0x4);
      S32LDI(xr7,src,0x8);
      S32LDD(xr8,src,0x4);

      D16MUL_LW(xr4,xr12,xr1,xr3);
      D16MUL_LW(xr10,xr12,xr7,xr9);
      D16MUL_LW(xr14,xr12,xr8,xr15);
      D32ASUM_AA(xr3,xr13,xr13,xr4);
      D16MUL_LW(xr6,xr12,xr2,xr5);
      D32SLR(xr3,xr3,xr4,xr4,4);
      D32ASUM_AA(xr5,xr13,xr13,xr6);
      D32ASUM_AA(xr9,xr13,xr13,xr10);
      D32SLR(xr5,xr5,xr6,xr6,4);
      D32SLR(xr9,xr9,xr10,xr10,4);
      D32ASUM_AA(xr15,xr13,xr13,xr14);

      S32SDI(xr3,dst_t,0x10);
      S32STD(xr4,dst_t,0x4);
      S32STD(xr5,dst_t,0x8);
      S32STD(xr6,dst_t,0xc);

      D32SLR(xr15,xr15,xr14,xr14,4);
      S32SDI(xr9,dst_t,0x10);
      S32STD(xr10,dst_t,0x4);
      S32STD(xr15,dst_t,0x8);
      S32STD(xr14,dst_t,0xc);      
    }
#endif

}
#endif
