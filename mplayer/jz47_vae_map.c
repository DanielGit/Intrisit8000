#ifdef __MINIOS__
#include <mplaylib.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include <sys/ioctl.h>
#ifndef NOAH_OS
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"
#define IPU__OFFSET 0x13080000
#define ME__OFFSET 0x130A0000
#define MC__OFFSET 0x13090000
#define DBLK__OFFSET 0x130B0000
#define IDCT__OFFSET 0x130C0000
#define CPM__OFFSET 0x10000000


#define IPU__SIZE   0x00001000
#define ME__SIZE   0x00001000
#define MC__SIZE   0x00001000
#define DBLK__SIZE   0x00001000
#define IDCT__SIZE   0x00001000
#define CPM__SIZE 0x00001000

#ifdef __MINIOS__
#define MINIOS_VADDR(addr) (((addr) & 0x1FFFFFFF) | 0xA0000000)
#else
int vae_fd;
int tcsm_fd;
#endif
volatile unsigned char *mc_base;
volatile unsigned char *me_base;
volatile unsigned char *dblk_base;
volatile unsigned char *idct_base;
volatile unsigned char *cpm_base;

static unsigned int cpm_gate0 = 0, cpm_gate1 = 0;

void VAE_map() {
#ifdef __MINIOS__
	mc_base   = MINIOS_VADDR (MC__OFFSET  );
	me_base   = MINIOS_VADDR (ME__OFFSET  );
	dblk_base = MINIOS_VADDR (DBLK__OFFSET);
	idct_base = MINIOS_VADDR (IDCT__OFFSET);	
	cpm_base  = MINIOS_VADDR (CPM__OFFSET );
	
    unsigned int regval;
    __asm__ __volatile ("lui  %0, %1" :    "=d"(regval) : "K"(0xa900)); 
    __asm__ __volatile ("mtc0 %0, $5, 4" ::"d" (regval));
#else
	/* open and map flash device */
	vae_fd = open("/dev/mem", O_RDWR);
	// tricky appoach to use TCSM
	tcsm_fd = open("/dev/tcsm", O_RDWR);
	if (vae_fd < 0) {
	  printf("open /dev/mem error.\n");
	  exit(1);
	}
	if (tcsm_fd < 0) {
	  printf("open /dev/tcsm error.\n");
	  exit(1);
	}
	mc_base = mmap((void *)0, MC__SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vae_fd, 
			MC__OFFSET);
	me_base = mmap((void *)0, ME__SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vae_fd, 
			ME__OFFSET);
	dblk_base = mmap((void *)0, DBLK__SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vae_fd, 
			DBLK__OFFSET);
	idct_base = mmap((void *)0, IDCT__SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vae_fd, 
			IDCT__OFFSET);	
	cpm_base = mmap((void *)0, CPM__SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vae_fd, 
			CPM__OFFSET);	
#endif
        cpm_gate0 = *(volatile unsigned int *)(cpm_base + 0x20);
        cpm_gate1 = *(volatile unsigned int *)(cpm_base + 0x28);
        *(volatile unsigned int *)(cpm_base + 0x20) = 0;
		*(volatile unsigned int *)(cpm_base + 0x20) = 
	(1 << 27|1 << 26|1 << 23|1 << 22|1 << 21|1 << 18|1 << 17|1 << 9);
        *(volatile unsigned int *)(cpm_base + 0x28) = 0;
		*(volatile unsigned int *)(cpm_base + 0x28) = (1 << 9);
	printf("VAE mmap successfully done!\n");
		int arb0 = 0;
	#define AHB_CIM(n)    (((n) & 0x3) << 0)
#define AHB_LCD(n)    (((n) & 0x3) << 2)
#define AHB_IPU(n)    (((n) & 0x3) << 4)
#define AHB_AXI(n)    (((n) & 0x3) << 6)
#define AHB_DMA(n)    (((n) & 0x3) << 8)
#define AHB_CORE0(n)  (((n) & 0x3) << 12)
#define AHB_VPU(n)    (((n) & 0x3) << 14)
#define AHB_AOSD(n)   (((n) & 0x3) << 18)
#define AHB_AHB2(n)   (((n) & 0x3) << 20)
#define AHB_ALL(n)   (((n) & 0x3) << 30)
 
 
  if ((*(volatile unsigned int *)0xb3070048) & 1)
     *(volatile unsigned int *)0xb3070048 &= ~(1 << 0);
    
   if ((*(volatile unsigned int *)0xb3060050) & 1)
     *(volatile unsigned int *)0xb3060050 &= ~(1 << 0);
 
   if (*(volatile unsigned int *)0xb30502c0 != 0x70000000)
    *(volatile unsigned int *)0xb30502c0 = 0x70000000;
    
    arb0 = AHB_CIM(0)   | AHB_LCD(2) | AHB_IPU(1)  | AHB_AXI(1) | AHB_DMA(2) | \
           AHB_CORE0(1) | AHB_VPU(1) | AHB_AOSD(1) | AHB_AHB2(1)| AHB_ALL(0);
          
    if (*(volatile unsigned int *)0xb3000000 != arb0)
    *(volatile unsigned int *)0xb3000000 = arb0;
}

void VAE_unmap() {
  *(volatile unsigned int *)(cpm_base + 0x20) = cpm_gate0;
  *(volatile unsigned int *)(cpm_base + 0x28) = cpm_gate1;
#ifndef __MINIOS__
  munmap(ipu_base, IPU__SIZE);
  munmap(mc_base, MC__SIZE);
  munmap(me_base, ME__SIZE);
  munmap(dblk_base, DBLK__SIZE);
  munmap(idct_base, IDCT__SIZE);
  munmap(cpm_base, CPM__SIZE);
  close(vae_fd);
  close(tcsm_fd);
#endif
}
