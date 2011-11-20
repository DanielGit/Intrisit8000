//[PROPERTY]===========================================[PROPERTY]
//            *****   电子词典平台GUI  *****
//        --------------------------------------
//         	          
//        --------------------------------------
//                 版权: 新诺亚舟科技
//             ---------------------------
//                  版   本   历   史
//        --------------------------------------
//  版本    日前		说明		
//  V0.1    2008-8      Init.             Hisway.Gao
//[PROPERTY]===========================================[PROPERTY]

#include <kernel/kernel.h>
#include <direct/media.h>
#include <direct/surface.h>
#include <direct/waveout.h>
#include <direct/osd.h>

typedef void (*sysfn_t)(void);

#define SYSENT(func)	(sysfn_t)(func)

const sysfn_t DirectCallTable[] = 
{
#if defined(STC_EXP)
	SYSENT(0x28),
	/* 0x00 */ SYSENT(sMediaSrvCreate),
	/* 0x01 */ SYSENT(sMediaSrvDestroy),
	/* 0x02 */ SYSENT(sMediaSrvCtrl),
#ifdef CONFIG_MCU_AK8802
	/* 0x03 */ SYSENT(sMediaSrvGetName),
#else
	/* 0x03 */ SYSENT(0),
#endif
	/* 0x04 */ SYSENT(0),
	/* 0x05 */ SYSENT(0),
	/* 0x06 */ SYSENT(0),
	/* 0x07 */ SYSENT(0),	
	/* 0x08 */ SYSENT(sMediaSrvMaster),
	/* 0x09 */ SYSENT(sMediaSrvInfo),
	/* 0x0A */ SYSENT(0),
	/* 0x0B */ SYSENT(0),
	/* 0x0C */ SYSENT(0),
	/* 0x0D */ SYSENT(0),
	/* 0x0E */ SYSENT(0),
	/* 0x0F */ SYSENT(0),

	/* 0x10 */ SYSENT(sSurfaceCreate),
	/* 0x11 */ SYSENT(sSurfaceDestroy),
	/* 0x12 */ SYSENT(sSurfaceInfo),
	/* 0x13 */ SYSENT(sSurfaceUpdate),
	/* 0x14 */ SYSENT(0),
	/* 0x15 */ SYSENT(0),
	/* 0x16 */ SYSENT(0),
	/* 0x17 */ SYSENT(0),	

#if defined(CONFIG_DIRECT_WAVEOUT)
	/* 0x18 */ SYSENT(sWaveOutOpen),
	/* 0x19 */ SYSENT(sWaveOutClose),
	/* 0x1A */ SYSENT(sWaveOutSetVolume),
	/* 0x1B */ SYSENT(sWaveOutWrite),
	/* 0x1C */ SYSENT(0),
	/* 0x1D */ SYSENT(0),
	/* 0x1E */ SYSENT(0),
	/* 0x1F */ SYSENT(0),
#else
	/* 0x18 */ SYSENT(0),
	/* 0x19 */ SYSENT(0),
	/* 0x1A */ SYSENT(0),
	/* 0x1B */ SYSENT(0),
	/* 0x1C */ SYSENT(0),
	/* 0x1D */ SYSENT(0),
	/* 0x1E */ SYSENT(0),
	/* 0x1F */ SYSENT(0),
#endif
	
#if defined(CONFIG_DIRECT_OSD)
	/* 0x20 */ SYSENT(sOsdOpen),
	/* 0x21 */ SYSENT(sOsdClose),
	/* 0x22 */ SYSENT(sOsdSet),
	/* 0x23 */ SYSENT(sOsdWrite),
	/* 0x24 */ SYSENT(0),
	/* 0x25 */ SYSENT(0),
	/* 0x26 */ SYSENT(0),
	/* 0x27 */ SYSENT(0)
#else
	/* 0x20 */ SYSENT(0),
	/* 0x21 */ SYSENT(0),
	/* 0x22 */ SYSENT(0),
	/* 0x23 */ SYSENT(0),
	/* 0x24 */ SYSENT(0),
	/* 0x25 */ SYSENT(0),
	/* 0x26 */ SYSENT(0),
	/* 0x27 */ SYSENT(0)
#endif
	
#else
	SYSENT(0),
#endif
};

