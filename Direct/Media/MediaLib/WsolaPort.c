#include "wsola.h"


void* WsolaPortMalloc(int size)
{
	return kmalloc(size);
}


void WsolaPortFree(void *ptr)
{
	if(ptr)
		kfree(ptr);
}

void *WsolaPortMemset(void *ptr, int c, int size)
{
	return kmemset(ptr, c, size); 
}

void *WsolaPortMemcpy(void *dst, void *src, int size)
{
	return kmemcpy(dst, src, size); 
}


void WsolaPortCopySamples(short *dst, const short *src,
									 DWORD count)
{
	DWORD i;
	for (i=0; i<count; ++i) 
		dst[i] = src[i];
}


void WsolaPortMoveSamples(short *dst, const short *src, DWORD count)
{
	DWORD i;
	for (i=0; i<count; ++i) 
		dst[i] = src[i];
}

void WsolaPortZeroSamples(short *samples, DWORD count)
{
	DWORD i;
	for (i=0; i<count; ++i) 
		samples[i] = 0;
}



