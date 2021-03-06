/*TCSM init is done by P0*/
#define TCSM0_BASE 0xF4000000
#define TCSM1_BASE 0xB32C0000
#define SRAM_BASE  0xB32D0000

#define TCSM0_SIZE 0x1000
#define TCSM1_SIZE 0x2000
#define SRAM_SIZE  0x0c00

#if 0//defined(_FPGA_TEST_) || defined(_RTL_SIM_)
#include <stdio.h>
#include <stdlib.h>
#undef time
#undef rand
#undef srand
#include <time.h>

static void tcsm_init(){
  int i, *tcsm_ptr;
  srand(time(0));

  tcsm_ptr = (int *)TCSM0_BASE;
  for(i=0;i<TCSM0_SIZE;i++)
    tcsm_ptr[i] = (int)rand();

  tcsm_ptr = (int *)TCSM1_BASE;
  for(i=0;i<TCSM1_SIZE;i++)
    tcsm_ptr[i] = (int)rand();

  tcsm_ptr = (int *)SRAM_BASE;
  for(i=0;i<SRAM_SIZE;i++)
    tcsm_ptr[i] = (int)rand();
}

#else  /*application OS*/

static void tcsm_init(){
  int i, *tcsm_ptr;

  tcsm_ptr = (int *)TCSM0_BASE;
  for(i=0;i<TCSM0_SIZE;i++)
    tcsm_ptr[i] = 0x0;

  tcsm_ptr = (int *)TCSM1_BASE;
  for(i=0;i<TCSM1_SIZE;i++)
    tcsm_ptr[i] = 0x0;

  tcsm_ptr = (int *)SRAM_BASE;
  for(i=0;i<SRAM_SIZE;i++)
    tcsm_ptr[i] = 0x0;
}

#endif /*(_FPGA_TEST_) || (_RTL_SIM_)*/
