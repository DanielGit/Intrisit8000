#include "jzsoc/jzmedia.h"
#include "jzsoc/jzasm.h"
#include "jzsoc/vc1_dcore.h"
#define MXU_MEMSET(addr,data,len)   \
    do {                            \
       int32_t mxu_i;               \
       int32_t local = (int32_t)(addr)-4;  \
       int cline = ((len)>>5);		   \
       S32I2M(xr1, data);                  \
       for (mxu_i=0; mxu_i < cline; mxu_i++) \
       {                                     \
	   S32SDI(xr1,local,4);              \
           S32SDI(xr1,local,4);              \
           S32SDI(xr1,local,4);              \
           S32SDI(xr1,local,4);              \
           S32SDI(xr1,local,4);              \
           S32SDI(xr1,local,4);              \
           S32SDI(xr1,local,4);              \    
           S32SDI(xr1,local,4);              \
       }                                     \
    }while(0)

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
