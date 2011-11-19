/*****************************************************************************
 *
 * JZ4760 SRAM Space Seperate
 *
 * $Id: rv9_sram.h,v 1.1 2010/12/22 11:45:58 xqliang Exp $
 *
 ****************************************************************************/

#ifndef __RV9_SRAM_H__
#define __RV9_SRAM_H__


#define SRAM_BANK0  0x132D0000
#define SRAM_BANK1  0x132D1000
#define SRAM_BANK2  0x132D2000
#define SRAM_BANK3  0x132D3000
/*
  XXXX_PADDR:       physical address
  XXXX_VCADDR:      virtual cache-able address
  XXXX_VUCADDR:     virtual un-cache-able address 
*/
#define SRAM_PADDR(a)         ((((unsigned)(a)) & 0xFFFF) | 0x132D0000) 
#define SRAM_VCADDR(a)        ((((unsigned)(a)) & 0xFFFF) | 0xB32D0000) 
#define SRAM_VUCADDR(a)       ((((unsigned)(a)) & 0xFFFF) | 0xB32D0000) 


#define SRAM_DBLKUP_STRD_Y               (1280+4)
#define SRAM_DBLKUP_STRD_C               (640+4)
#define SRAM_DBLKUP_Y                    (SRAM_BANK0 + 4)
#define SRAM_DBLKUP_U                    (SRAM_DBLKUP_Y+(SRAM_DBLKUP_STRD_Y<<2) + 4)
#define SRAM_DBLKUP_V                    (SRAM_DBLKUP_U+(SRAM_DBLKUP_STRD_C<<2) + 4)
#endif /*__RV9_SRAM_H__*/
