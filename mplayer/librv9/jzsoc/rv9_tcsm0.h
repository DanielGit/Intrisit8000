/*****************************************************************************
 *
 * JZ4760 TCSM0 Space Seperate
 *
 ****************************************************************************/

#ifndef __RV9_TCSM0_H__
#define __RV9_TCSM0_H__


#define TCSM0_BANK0 0xF4000000
#define TCSM0_BANK1 0xF4001000
#define TCSM0_BANK2 0xF4002000
#define TCSM0_BANK3 0xF4003000
#define TCSM0_END   0xF4004000
/*
  XXXX_PADDR:       physical address
  XXXX_VCADDR:      virtual cache-able address
  XXXX_VUCADDR:     virtual un-cache-able address 
*/
#define TCSM0_PADDR(a)        (((a) & 0xFFFF) | 0x132B0000) 
#define TCSM0_VCADDR(a)       (((a) & 0xFFFF) | 0xB32B0000) 
#define TCSM0_VUCADDR(a)      (((a) & 0xFFFF) | 0xB32B0000) 


#define SPACE_TYPE_CHAR 0x0
#define SPACE_TYPE_SHORT 0x1
#define SPACE_TYPE_WORD 0x2


#define TCSM0_CPUBASE	0xF4000000
#define TCSM1_CPUBASE	0xF4000000
#define TCSM0_PBASE	0x132B0000
#define TCSM0_VBASE_UC	0xB32B0000
#define TCSM0_VBASE_CC	0x932B0000
#define TCSM1_PBASE	0x132C0000
#define TCSM1_VBASE_UC	0xB32C0000
#define TCSM1_VBASE_CC	0x932C0000
#define TCSM_BANK0_OFST	0x0
#define TCSM_BANK1_OFST	0x1000
#define TCSM_BANK2_OFST	0x2000
#define TCSM_BANK3_OFST	0x3000

#define SPACE_HALF_MILLION_BYTE 0x80000

#define JZC_CACHE_LINE 32
#define	ALIGN(p,n)     ((uint8_t*)(p)+((((uint8_t*)(p)-(uint8_t*)(0)) & ((n)-1)) ? ((n)-(((uint8_t*)(p)-(uint8_t*)(0)) & ((n)-1))):0))
#define TCSM1_ALN4(x)  ((x & 0x3)? (x + 4 - (x & 0x3)) : x)
/*--------------------------------------------------
 * P1 to P0 interaction signals,
 * P0 only read, P1 should only write
 --------------------------------------------------*/
#define TCSM0_COMD_BUF_LEN 32
#define TCSM0_P1_TASK_DONE TCSM0_CPUBASE
#define TCSM0_P1_FIFO_RP (TCSM0_P1_TASK_DONE + 4) //start ADDRESS of using task
#define TCSM0_GP1_POLL_END       (TCSM0_P1_FIFO_RP + 4)
#define TCSM0_GP0_POLL_END       (TCSM0_GP1_POLL_END + 4)
#define TCSM0_P1_DEBUG (TCSM0_GP0_POLL_END + 4)

#define TCSM0_HUGE_AIC_TABLE     (TCSM0_CPUBASE+TCSM0_COMD_BUF_LEN)
#define TCSM0_HUGE_AIC_TABLE_LEN (4096)
#define TCSM0_DIV9_TABLE (TCSM0_HUGE_AIC_TABLE+TCSM0_HUGE_AIC_TABLE_LEN)
#define TCSM0_DIV9_TABLE_LEN (81/4+4)
/*--------------------------------------------------
 * P1 TASK_FIFO
 --------------------------------------------------*/
#define TCSM0_TASK_FIFO (TCSM0_HUGE_AIC_TABLE+TCSM0_HUGE_AIC_TABLE_LEN)
#define MAX_TASK_LEN (sizeof(RV9_MB_DecARGs))
#endif /*__RV9_TCSM_H0__*/
