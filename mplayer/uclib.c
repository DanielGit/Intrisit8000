#include "uclib.h"
#include "LocalMemMgr.h"
#undef perror
extern void noah_kprintf(const char *format, ... );

int perror(const char *fmt, ...)
{
  int ret;
  va_list ap;
  va_start (ap, fmt);
  ret = printf(fmt, ap);
  va_end (ap);
  return ret;
}
int please_use_av_log(const char *fmt, ...)
{
  int ret;
  va_list ap;
  va_start (ap, fmt);
  ret = printf(fmt, ap);
  va_end (ap);
  return ret;

}

static char *mp_memory = NULL;
static unsigned int mp_memory_size = 0;
unsigned int mp_memory_empty = 0;

#define MEM_STAT    0
#if MEM_STAT
#undef printf
static char *old_memory = NULL;
static void print_mem_use(void *mem)
{
	if((unsigned int ) mem > (unsigned int )old_memory)
	{
		printf("cur mem max using %x\n",(unsigned int)mem - (unsigned int)mp_memory);
		old_memory = mem;
	}
}
#else
#define print_mem_use(x)
#endif
void mplayer_memory_set(void *mp,unsigned int size)
{
	mp_memory_size = size;
	mp_memory = mp;
	#if MEM_STAT
	old_memory = mp_memory; 
	#endif
}

int mp_memory_init(int mode)
{
	static int init_mem = 0;
	if(init_mem == 0)
	{
		memset(mp_memory,0,mp_memory_size);
		Local_HeapInit((unsigned int)mp_memory,mp_memory_size);
		init_mem = 1;
		return 1;
	}
	return 0;
}

void mp_memory_deinit(int mode)
{
	mp_memory = NULL;
	mp_memory_size = 0;
}

#define MEM_ADDRESS(x) ((unsigned int) x - (unsigned int)*(char *)((unsigned int)x - 1))
#define isBuddy(address) (*(unsigned int *)MEM_ADDRESS(address) > (unsigned int)0x80000000)

#define BUDDY_MEM  1
void *uc_malloc(unsigned int size)
{
	void *ret = NULL;
	#if BUDDY_MEM
	if(size <= (BUDDY_BLOCK_SIZE / 4))
		ret = (void *)buddyAlloc((unsigned int)mp_memory,size);
	#endif
	
	if(ret == NULL)
		ret = (void *)Local_Alloc((unsigned int)mp_memory,size);
	print_mem_use(ret+size);
	
	if (!ret)
	{
		printf("++++++ mplayer %s error size: %d ++++++\n", __FUNCTION__, size);
		mp_memory_empty = 1;	
	 }
	return ret;
}

void uc_free(void *addr)
{
	if(addr != NULL)
	{
		#if BUDDY_MEM
			if(isBuddy(addr))
			{	
				//unsigned int *d = (unsigned int *)MEM_ADDRESS(addr);		
				buddyFree((unsigned int)mp_memory,addr);
			}
			else{
		#endif
				Local_Dealloc((unsigned int)mp_memory,(unsigned int)addr);
			#if BUDDY_MEM
			}
			#endif
	}
}
void *uc_calloc(unsigned int x,unsigned int n)
{
	void *ret = NULL;
	unsigned int size = x * n;
	#if BUDDY_MEM
	if(size <= (BUDDY_BLOCK_SIZE / 4))
		ret = (void *)buddyCalloc((unsigned int)mp_memory,x,n);
	#endif
	if(ret == NULL)
		ret = (void *)Local_Calloc((unsigned int)mp_memory,x,n);
	print_mem_use(ret+n*x);
	if (!ret)
	{
		printf("mplayer %s error size: %d\n", __FUNCTION__, n);
		mp_memory_empty = 1;	
	 }
	return ret;
}

void *uc_realloc(void *addr,unsigned int size)
{
	void *ret = NULL;
	if(addr == NULL)
	{
			addr = uc_malloc(size);
			return addr;
	}
	#if BUDDY_MEM
	if(isBuddy(addr))
		ret = (void *)buddyRealloc((unsigned int)mp_memory,addr,size);
	#endif
	if(ret == NULL)
		ret = (void *)Local_Realloc((unsigned int)mp_memory,(unsigned int)addr,size);
	print_mem_use(ret+size);
	if (!ret)
	{
		printf("mplayer %s error size: %d\n", __FUNCTION__, size);
		mp_memory_empty = 1;	
	 }
	return ret;
}

void *uc_memalign(unsigned int x,unsigned int size)
{
	void *ret = NULL;
	#if BUDDY_MEM
	if(size <= (BUDDY_BLOCK_SIZE / 4))
		ret = (void *)buddyAlignalloc((unsigned int)mp_memory,x,(unsigned int)size); 
	#endif
	if(ret == NULL)
		ret = (void *)Local_alignAlloc((unsigned int)mp_memory,x,(unsigned int)size); 
	print_mem_use(ret+size);
	if (!ret)
	{
		printf("mplayer %s error size: %d\n", __FUNCTION__, size);
		mp_memory_empty = 1;	
	 }
	return ret;
}

char * uc_strdup(const char *str)
{
   char *p;
   if (!str)
      return(NULL);
   if ((p = (char *)uc_malloc(strlen(str) + 1)) != 0)
      return(strcpy(p,str));
   return(NULL);
}

#ifndef NOAH_OS
#ifndef USE_16M_SDRAM
//-----------------------------drv mem--------------------------------------------
void *uc_malloc_static(unsigned int addr)
{
	return (void *)alloc((unsigned int)addr);
}
void uc_free_static(void *addr)
{
	if(addr != NULL)
		deAlloc((unsigned int)addr);
}
void *uc_calloc_static(unsigned int x,unsigned int n)
{
	return (void *)Drv_calloc(x,n);
}
void *uc_realloc_static(void *addr,unsigned int size)
{
	return (void *)Drv_realloc((unsigned int)addr,(unsigned int)size);
}
void *uc_memalign_static(unsigned int x,unsigned int size)
{
	return (void *)alignAlloc(x,(unsigned int)size);
}
//-------------------------------------------------------------------------
#else
//-----------------------------module mem--------------------------------------------
void *uc_malloc_static(unsigned int size)
{
	
	return uc_malloc(size);
}
void uc_free_static(void *addr)
{
	uc_free(addr);
}
void *uc_calloc_static(unsigned int x,unsigned int n)
{
	return uc_calloc(x,n);
}
void *uc_realloc_static(void *addr,unsigned int size)
{
	return uc_realloc(addr,size);
}
void *uc_memalign_static(unsigned int x,unsigned int size)
{
	return uc_memalign(x,size);
}
#endif
#endif

void dumpdatabuf(unsigned char *buf,unsigned int len)
{
	int i,j;
	printf("\n");
	for(i = 0;i < len / 16;i++)
	{
		printf("%04x: ",i);
		for(j = 0; j < 16; j++)
			printf("%02x ",buf[j + i * 16]);
		printf("\n");	
	}
	if((i & 15) > 0)
	{
			printf("%02x: ",j);
		for(j = 0; j < (i & 15); j++)
			printf("%02x ",buf[j + i * 16]);
		printf("\n");	
	}
}

void stat(const char *path, struct stat *buf)
{
  kprintf ("+++++++++ calling stat +++++++++\n");
  kpanic("stat()\n");
}

void __assert (const char *file, int line, const char *msg)
{
  kprintf ("ASSERT: %s(line: %d), %s\n", file, line, msg);
  kpanic("__assert()\n");
}

