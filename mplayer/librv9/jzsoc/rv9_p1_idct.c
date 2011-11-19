
#define MXU_SETZERO(addr,cline)   \
    do {                            \
       int32_t mxu_i;               \
       int32_t local = (int32_t)(addr)-4;  \    
       for (mxu_i=0; mxu_i < cline; mxu_i++) \
       {                                     \
	 i_pref(30,local,4);		     \
	   S32SDI(xr0,local,4);              \
           S32SDI(xr0,local,4);              \
           S32SDI(xr0,local,4);              \
           S32SDI(xr0,local,4);              \
           S32SDI(xr0,local,4);              \
           S32SDI(xr0,local,4);              \
           S32SDI(xr0,local,4);              \
           S32SDI(xr0,local,4);              \
       }                                     \
    }while(0)

#ifdef JZC_IDCT_OPT
#else
static void rv40_row_transform(int temp[16], uint32_t *block)
{
    int i;

    for(i=0; i<4; i++){
        const int z0= 13*(block[i+4*0] +    block[i+4*2]);
        const int z1= 13*(block[i+4*0] -    block[i+4*2]);
        const int z2=  7* block[i+4*1] - 17*block[i+4*3];
        const int z3= 17* block[i+4*1] +  7*block[i+4*3];

        temp[4*i+0]= z0+z3;
        temp[4*i+1]= z1+z2;
        temp[4*i+2]= z1-z2;
        temp[4*i+3]= z0-z3;
    }
}

/**
 * Real Video 3.0/4.0 inverse transform
 * Code is almost the same as in SVQ3, only scaling is different.
 */
static void rv40_inv_transform(DCTELEM *block){
    int temp[16];
    int i;

    rv40_row_transform(temp, (uint32_t *)block);

    for(i=0; i<4; i++){
        const int z0= 13*(temp[4*0+i] +    temp[4*2+i]) + 0x200;
        const int z1= 13*(temp[4*0+i] -    temp[4*2+i]) + 0x200;
        const int z2=  7* temp[4*1+i] - 17*temp[4*3+i];
        const int z3= 17* temp[4*1+i] +  7*temp[4*3+i];
#if 1
        block[i*4+0]= (z0 + z3)>>10;
        block[i*4+1]= (z1 + z2)>>10;
        block[i*4+2]= (z1 - z2)>>10;
        block[i*4+3]= (z0 - z3)>>10;
#else
	dst[i*8+0]= (z0 + z3)>>10;
        dst[i*8+1]= (z1 + z2)>>10;
        dst[i*8+2]= (z1 - z2)>>10;
        dst[i*8+3]= (z0 - z3)>>10;
#endif
    }

}
#endif
static void rv40_add_4x4_block(uint8_t *dst, int stride, DCTELEM block[64], int off)
{
#if 0
  int y;
  uint32_t src=block+off-4;
  dst -= stride;
  for(y = 0; y < 4; y++){
    S32LDI(xr1,src,0x8);
    S32LDIV(xr3,dst,stride,0);
    S32LDD(xr2,src,0x4);
    Q8ACCE_AA(xr2,xr3,xr0,xr1);
    Q16SAT(xr4,xr2,xr1);
    S32STD(xr4,dst,0);
  }
#else
    DCTELEM *blk = block+off;
    uint8_t *dst_mid = dst;
    int strd = stride;

    S32LDD(xr1, dst_mid, 0); //d3,d2,d1,d0
    S32LDD(xr2,blk,0); //b1,b0
    S32LDD(xr3,blk,4);   //b3,b2        
    Q8ACCE_AA(xr3,xr1,xr0,xr2);

    S32LDIV(xr11, dst_mid, strd, 0); //d3,d2,d1,d0
    S32LDI(xr12,blk,8); //b1,b0
    S32LDD(xr13,blk,4);   //b3,b2        
    Q16SAT(xr4,xr3,xr2);
    Q8ACCE_AA(xr13,xr11,xr0,xr12);

    S32LDIV(xr1, dst_mid, strd, 0); //d3,d2,d1,d0
    S32LDI(xr2,blk,8); //b1,b0
    S32LDD(xr3,blk,4);   //b3,b2        
    Q16SAT(xr5,xr13,xr12);
    Q8ACCE_AA(xr3,xr1,xr0,xr2);

    S32LDIV(xr11, dst_mid, strd, 0); //d3,d2,d1,d0
    S32LDI(xr12,blk,8); //b1,b0
    S32LDD(xr13,blk,4);   //b3,b2        
    Q16SAT(xr6,xr3,xr2);
    Q8ACCE_AA(xr13,xr11,xr0,xr12);

    S32STD(xr4,dst,0);
    S32SDIV(xr5, dst, strd, 0);
    Q16SAT(xr7,xr13,xr12);
    S32SDIV(xr6, dst, strd, 0); //d3,d2,d1,d0
    S32SDIV(xr7, dst, strd, 0); //d3,d2,d1,d0
#endif
}
