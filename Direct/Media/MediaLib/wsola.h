#ifndef __WSOLA_H__
#define __WSOLA_H__

#include <kernel/kernel.h>

#define RC_SUCCESS		0
#define RC_EUNKNOWN	    (-1)
#define RC_EPENDING	    (-2)
#define RC_ETOOMANYCONN	(-3)
#define RC_EINVAL	    (-4)
#define RC_ENAMETOOLONG	(-5)
#define RC_ENOTFOUND	(-6)
#define RC_ENOMEM	    (-7)
#define RC_EBUG         (-8)
#define RC_ETIMEDOUT    (-9)
#define RC_ETOOMANY     (-10)
#define RC_EBUSY        (-11)
#define RC_ENOTSUP	    (-12)
#define RC_EINVALIDOP	(-13)
#define RC_ECANCELLED	(-14)
#define RC_EEXISTS      (-15)
#define RC_EEOF		    (-16)
#define RC_ETOOBIG	    (-17)
#define RC_ERESOLVE	    (-18)
#define RC_ETOOSMALL	(-19)
#define RC_EIGNORED	    (-20)
#define RC_EIPV6NOTSUP	(-21)
#define RC_EAFNOTSUP	(-22)

typedef struct _AUDIO_FILTER
{
	short *iBuf;
	short *oBuf;
	int iSize;
	int oSize;
}AUDIO_FILTER;
typedef AUDIO_FILTER *PAUDIO_FILTER;


typedef struct _WSOLA_BUF 
{
    short	    *buf;	    // The buffer
    DWORD	     capacity;	// Buffer capacity, in samples

    short	    *start;	    // Pointer to the first sample
    DWORD	     len;	    // Audio samples length,  in samples
}WSOLA_BUF;
typedef WSOLA_BUF *PWSOLA_BUF;

typedef struct _CONV_BUF
{
	char		*frame_buf;
	char		*cache_buf;
	int			filter_off;
	int			frame_size;
	int			cache_size;
}CONV_BUF;
typedef CONV_BUF *PCONV_BUF;


typedef struct _WSOLA_OBJECT
{
	DWORD		 sample_rate;		// Sampling rate.		
	WORD		 samples_per_frame; // Samples per frame (const)	
	WORD		 channel_count;		// Channel countt (const)		
	WORD		 options;			// Options.				
	short		 rate;
	int			 iframes;
	int			 oframes;

	WSOLA_BUF	*buf;				// The buffer.			
	short		*erase_buf;			// Temporary erase buffer.		
	short		*merge_buf;			// Temporary merge buffer.		

	WORD		 buf_size;			// Total buffer size (const)	
	WORD		 hanning_size;		// Hanning window size (const)  
	WORD		 templ_size;		// Template size (const)		
	WORD		 hist_size;			// History size (const)		

	WORD		 min_extra;			// Minimum extra (const)		
	DWORD		 max_expand_cnt;	// Max # of synthetic samples   
	DWORD		 fade_out_pos;		// Last fade-out position		
	WORD		 expand_sr_min_dist; // Minimum distance from template for find_pitch() on expansion (const)				
	WORD		 expand_sr_max_dist; // Maximum distance from template for find_pitch() on expansion (const)				

	WORD		*hanning;			// Hanning window.			
	
	CONV_BUF	*conv_buf;
}WSOLA_OBJECT;
typedef WSOLA_OBJECT *PWSOLA_OBJECT;


// Disable Hanning window to conserve memory.
#define WSOLA_OPTION_NO_HANNING	1

// Specify that the WSOLA will not be used for PLC.
#define WSOLA_OPTION_NO_PLC		2

// Specify that the WSOLA will not be used to discard frames in non-contiguous buffer.
#define WSOLA_OPTION_NO_DISCARD	4

// Disable fade-in and fade-out feature in the transition between
// actual and synthetic frames in WSOLA. With fade feature enabled, 
// WSOLA will only generate a limited number of synthetic frames 
// (configurable with #Wsolaset_max_expand()), fading out 
// the volume on every more samples it generates, and when it reaches
// the limit it will only generate silence.
#define WSOLA_OPTION_NO_FADING	8
    
#define WSOLA_OPTION_LITE		16
    
#define WSOLA_OPTION_LINER_WIN	32


HANDLE WsolaCreate(DWORD sample_rate, DWORD channel_count, DWORD options, int rate);
int WsolaDestroy(HANDLE hwsola);
int WsolaReset(HANDLE hwsola, DWORD options);
int WsolaSave(HANDLE hwsola, short frm[], int prev_lost);
int WsolaGenerate(HANDLE hwsola, short frm[]);
int WsolaDiscard(HANDLE hwsola,  short buf1[], DWORD buf1_cnt, 
					   short buf2[], DWORD buf2_cnt, DWORD *erase_cnt);



void* WsolaPortMalloc(int size);
void WsolaPortFree(void *ptr);
void *WsolaPortMemset(void *ptr, int c, int size);
void *WsolaPortMemcpy(void *dst, void *src, int size);

void WsolaPortCopySamples(short *dst, const short *src, DWORD count);
void WsolaPortMoveSamples(short *dst, const short *src, DWORD count);
void WsolaPortZeroSamples(short *samples, DWORD count);

int WsolaConvCreate(PWSOLA_OBJECT wsola);
void WsolaConvDestroy(PWSOLA_OBJECT wsola);
int WsolaConvert(HANDLE hwsola, PAUDIO_FILTER filter);

int WsolaBufCreate(DWORD capacity, WSOLA_BUF **circbuf);
void WsolaBufDestroy(WSOLA_BUF *circbuf);
int WsolaBufReset(WSOLA_BUF *circbuf);
int WsolaBufGetLen(WSOLA_BUF *circbuf);
void WsolaBufSetLen(WSOLA_BUF *circbuf, DWORD len);
int WsolaBufReadPtr(WSOLA_BUF *circbuf, DWORD count);
int WsolaBufWritePtr(WSOLA_BUF *circbuf, DWORD count);
void WsolaBufReadRegions(WSOLA_BUF *circbuf, 
						  short **reg1, 
						  DWORD *reg1_len, 
						  short **reg2, 
						  DWORD *reg2_len);
void WsolaBufWriteRegions(WSOLA_BUF *circbuf, 
						   short **reg1, 
						   DWORD *reg1_len, 
						   short **reg2, 
						   DWORD *reg2_len);
int WsolaBufRead(WSOLA_BUF *circbuf, short *data, DWORD count);
int WsolaBufWrite(WSOLA_BUF *circbuf, short *data, DWORD count);
int WsolaBufCopy(WSOLA_BUF *circbuf, 
					     DWORD start_idx,
					     short *data, 
					     DWORD count);
int WsolaBufPack(WSOLA_BUF *circbuf);


#endif	// __WSOLA_H__

