//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典文件系统  *****
//        --------------------------------------
//                    处理器相关头文件
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
//  V0.1    2007-4      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]

#ifndef _INC_ARCH
#define _INC_ARCH

#if defined(__arm__)
#include "./arm/arch.h"
#elif defined (__i386__)
#include "./i386/arch.h"
#elif defined (__ppc__)
#include "./ppc/arch.h"
#elif defined (__mips__)
#include "./mips/arch.h"
#elif defined (__sh4__)
#include "./sh4/arch.h"
#elif defined (WIN32)
#include "./win32/arch.h"
#elif defined (__c33pe)
#include "./C33L27/arch.h"
#else
#error architecture not supported
#endif

#endif // _INC_ARCH
