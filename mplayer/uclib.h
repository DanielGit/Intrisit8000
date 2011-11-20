#define DEF_NOT_PRINT 1
#if DEF_NOT_PRINT
#undef printf
#define printf(x,y...) ({x;})
#endif //DEF_NOT_PRINT
#ifndef __UCLIB_H__
#define __UCLIB_H__

//extern "C"
//{
//	
//	unsigned int alloc(unsigned int nbytes);
//	unsigned int Drv_realloc(unsigned int address,unsigned int nbytes);
//	void deAlloc(unsigned int address);
//  unsigned int Drv_calloc(unsigned int size,unsigned int n);
//}
#if DEF_NOT_PRINT
#undef printf
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>
#ifdef __cplusplus
extern "C"{
#endif
//#define	RAND_MAX	2147483647	
#include "jz_mxu.h"

#ifndef __cplusplus

int printf(const char *fmt, ...);
#if DEF_NOT_PRINT

#undef printf
#define printf(x,y...) ({x;})
#endif
#endif
//typedef struct
//{
//  int quot;			// Quotient.  
//  int rem;			// Remainder.  
//} div_t;
//
#undef div
  
#define div(a, b)                       \
({                                      \
  div_t result;                         \
  result.quot = a / b;                  \
  result.rem = a - (result.quot * b);   \
  result;                               \
})

#define off_t int 

#define abs(a)	   (((a) < 0) ? -(a) : (a))

#undef memset
//#define DEBUG_MEMSET
#ifdef DEBUG_MEMSET
#define memset(x,y,z) do{printf("%s:%d->%s\n",__FILE__,__LINE__,__FUNCTION__);uc_memset(x,y,z);}while(0)
#else
#define memset(x,y,z) uc_memset(x,y,z)
#endif
//#undef memmove
//#define memmove(x,y,z) uc_memcpy(x,y,z)
#undef memcpy
#define memcpy(x,y,z) uc_memcpy(x,y,z)

void *uc_memcpy(void *to,const void *from,unsigned int size);
void uc_memset_tmp(void *to,unsigned char v,unsigned int size);
void uc_memset(void *to,unsigned char v,unsigned int size);

//-------------------------------------------------------------------
extern unsigned alloc(unsigned int nbytes);
extern unsigned int alignAlloc(unsigned int align,unsigned int size);
extern void deAlloc(unsigned int address);
extern unsigned int Drv_realloc(unsigned int address,unsigned int nbytes);
extern unsigned int Drv_calloc(unsigned int size,unsigned int n);
//-------------------------------------------------------------------

//---------------------------------------------------------------------------------
extern void *Module_Alloc(unsigned int nbyte);
extern void *Module_Realloc(void* address,unsigned int nbytes);
extern void *Module_CAlloc(unsigned int size,unsigned int n);
extern void *Module_alignAlloc(unsigned int align,unsigned int n);
//----------------------------------------------------------------
extern void GM_Dealloc(void* addr);
//-------------------------------------------------------------------

void *uc_malloc(unsigned int size);
void uc_free(void *addr);
void *uc_calloc(unsigned int x,unsigned int n);
void *uc_realloc(void *addr,unsigned int size);
void *uc_memalign(unsigned int x,unsigned int size);
char * uc_strdup(const char *str);

//#define MEMDEBUG

#undef malloc
#ifdef MEMDEBUG
#define malloc(x) ({void * mem;printf("malloc:%s %d",__FILE__,__LINE__);mem = uc_malloc(x);printf(" addr = %08x len = %d\n",mem,x);mem;})
#else
#define malloc(x) uc_malloc(x)
#endif

#undef free
#ifdef MEMDEBUG
#define free(x) ({printf("free:%s %d addr = %08x\n",__FILE__,__LINE__,x);uc_free(x);})
#else
#define free(x) uc_free(x)
#endif

#undef callloc
#ifdef MEMDEBUG
#define calloc(x,y) ({void *mem;printf("calloc:%s %d",__FILE__,__LINE__);mem = uc_calloc(x,y);printf(" addr = %08x len = %d\n",mem,x,y);mem;})
#else
#define calloc(x,y) uc_calloc(x,y)
#endif

#undef realloc
#ifdef MEMDEBUG
#define realloc(x,y) ({void *mem;printf("realloc:%s %d",__FILE__,__LINE__);mem = uc_realloc(x,y);printf(" addr = %08x %d\n",mem,y);mem;})
#else
#define realloc(x,y) uc_realloc(x,y)
#endif

#undef memalign
#ifdef MEMDEBUG
 #define memalign(x,y) ({void *mem;printf("memalign:%s %d",__FILE__,__LINE__); mem = uc_memalign(x,y);printf(" addr = %08x %d\n",mem,y);mem;})
#else
 #define memalign(x,y) uc_memalign(x,y)
#endif

#undef strdup
#ifdef MEMDEBUG
#define strdup(x) ({char * mem;printf("strdup:%s %d",__FILE__,__LINE__);mem = uc_strdup(x);printf(" addr = %08x\n",mem);mem;})
#else
#define strdup(x) uc_strdup(x)
#endif

#define F(x,s...) {printf("%s:%d  ",__FILE__,__LINE__); printf(x,##s);printf("\n");}
#ifndef __cplusplus
#undef stderr
#define stderr 1
#undef stdout
#define stdout 2
#undef fprintf
#define fprintf(x,y,c...) ({printf("%s %d",__FILE__,__LINE__); printf(y,##c);})
#undef snprintf
#define snprintf(x,y,s,a...) sprintf(x,s,##a)
#undef vsnprintf
#define vsnprintf(x,y,s,a...) vsprintf(x,s,##a)
#undef vfprintf
#define vfprintf fprintf

#define MY_THROW() asm("syscall")
#undef exit
#define exit(x) do{ printf("=======================%s %d exit\n",__FILE__,__LINE__); return x;}while(0)
#undef perror
#define perror printf
#undef fflush
#define fflush(x) 
#endif	

#define ShowAddress()          \
do                             \
{			unsigned int dra;        \
			__asm__ __volatile__(    \
			"sw $31,0x00(%0)\n\t"    \
            :                  \
			: "r" (&dra));           \
		F("%x\n",dra);             \
}while(0)
#ifdef __cplusplus
}
#endif
#endif

