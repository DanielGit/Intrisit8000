
#include "wsola.h"

/* Buffer content:
 *
 *  +---------+-----------+--------------------+
 *  | history | min_extra | more extra / empty |
 *  +---------+-----------+--------------------+
 *  ^		 ^		   ^					^
 * buf	hist_size   min_extra			buf_size
 * 
 * History size (hist_size) is a constant value, initialized upon creation.
 *
 * min_extra size is equal to HANNING_PTIME, this samples is useful for 
 * smoothening samples transition between generated frame & history 
 * (when PLC is invoked), or between generated samples & normal frame 
 * (after lost/PLC). Since min_extra samples need to be available at 
 * any time, this will introduce delay of HANNING_PTIME ms.
 *
 * More extra is excess samples produced by PLC (PLC frame generation may 
 * produce more than exact one frame).
 *
 * At any particular time, the buffer will contain at least (hist_size + 
 * min_extra) samples.
 *
 * A "save" operation will append the new frame to the end of the buffer,
 * return the frame from samples right after history and shift the buffer
 * by one frame.
 *
 */

 /**
 * Create the circular buffer.
 *
 * @param pool			Pool where the circular buffer will be allocated
 *				from.
 * @param capacity		Capacity of the buffer, in samples.
 * @param circbuf			Pointer to receive the circular buffer instance.
 *
 * @return			RC_SUCCESS if the circular buffer has been
 *				created successfully, otherwise the appropriate
 *				error will be returned.
 */
int WsolaBufCreate(DWORD capacity, WSOLA_BUF **circbuf)
{
	WSOLA_BUF *cbuf;

	cbuf = (WSOLA_BUF *)WsolaPortMalloc(sizeof(WSOLA_BUF));
	if(cbuf == NULL)
	{
		*circbuf = NULL;
		return RC_ENOMEM;
	}
	WsolaPortMemset(cbuf, 0x00, sizeof(WSOLA_BUF));
	cbuf->buf = (short*) WsolaPortMalloc(capacity * sizeof(short));
	if(cbuf->buf == NULL)
	{
		WsolaPortFree(cbuf);
		*circbuf = NULL;
		return RC_ENOMEM;
	}
	WsolaPortMemset(cbuf->buf, 0x00, capacity * sizeof(short));
	cbuf->capacity = capacity;
	cbuf->start = cbuf->buf;
	cbuf->len = 0;
	*circbuf = cbuf;
	return RC_SUCCESS;
}


void WsolaBufDestroy(WSOLA_BUF *circbuf)
{
	if(circbuf)
	{
		if(circbuf->buf)
			WsolaPortFree(circbuf->buf);
		WsolaPortFree(circbuf);
	}
}


/**
 * Reset the circular buffer.
 *
 * @param circbuf		The circular buffer.
 *
 * @return			RC_SUCCESS when successful.
 */
int WsolaBufReset(WSOLA_BUF *circbuf)
{
	circbuf->start = circbuf->buf;
	circbuf->len = 0;

	return RC_SUCCESS;
}


/**
 * Get the circular buffer length, it is number of samples buffered in the 
 * circular buffer.
 *
 * @param circbuf		The circular buffer.
 *
 * @return			The buffer length.
 */
int WsolaBufGetLen(WSOLA_BUF *circbuf)
{
	return circbuf->len;
}


/**
 * Set circular buffer length. This is useful when audio buffer is manually 
 * manipulated by the user, e.g: shrinked, expanded.
 *
 * @param circbuf		The circular buffer.
 * @param len			The new buffer length.
 */
void WsolaBufSetLen(WSOLA_BUF *circbuf, DWORD len)
{
	circbuf->len = len;
}


/**
 * Advance the read pointer of circular buffer. This function will discard
 * the skipped samples while advancing the read pointer, thus reducing 
 * the buffer length.
 *
 * @param circbuf		The circular buffer.
 * @param count			Distance from current read pointer, can only be
 *				possitive number, in samples.
 *
 * @return			RC_SUCCESS when successful, otherwise 
 *				the appropriate error will be returned.
 */
int WsolaBufReadPtr(WSOLA_BUF *circbuf, DWORD count)
{
	if (count >= circbuf->len)
	return WsolaBufReset(circbuf);

	circbuf->start += count;
	if (circbuf->start >= circbuf->buf + circbuf->capacity) 
	circbuf->start -= circbuf->capacity;
	circbuf->len -= count;

	return RC_SUCCESS;
}


/**
 * Advance the write pointer of circular buffer. Since write pointer is always
 * pointing to a sample after the end of sample, so this function also means
 * increasing the buffer length.
 *
 * @param circbuf		The circular buffer.
 * @param count			Distance from current write pointer, can only be
 *				possitive number, in samples.
 *
 * @return			RC_SUCCESS when successful, otherwise 
 *				the appropriate error will be returned.
 */
int WsolaBufWritePtr(WSOLA_BUF *circbuf, DWORD count)
{
	if (count + circbuf->len > circbuf->capacity)
		return RC_ETOOBIG;

	circbuf->len += count;
	return RC_SUCCESS;
}


/**
 * Get the real buffer addresses containing the audio samples.
 *
 * @param circbuf		The circular buffer.
 * @param reg1			Pointer to store the first buffer address.
 * @param reg1_len		Pointer to store the length of the first buffer, 
 *				in samples.
 * @param reg2			Pointer to store the second buffer address.
 * @param reg2_len		Pointer to store the length of the second buffer, 
 *				in samples.
 */
void WsolaBufReadRegions(WSOLA_BUF *circbuf, 
						  short **reg1, 
						  DWORD *reg1_len, 
						  short **reg2, 
						  DWORD *reg2_len)
{
	*reg1 = circbuf->start;
	*reg1_len = circbuf->len;
	if (*reg1 + *reg1_len > circbuf->buf + circbuf->capacity) 
	{
		*reg1_len = circbuf->buf + circbuf->capacity - circbuf->start;
		*reg2 = circbuf->buf;
		*reg2_len = circbuf->len - *reg1_len;
	} 
	else 
	{
		*reg2 = NULL;
		*reg2_len = 0;
	}
}


/**
 * Get the real buffer addresses that is empty or writeable.
 *
 * @param circbuf		The circular buffer.
 * @param reg1			Pointer to store the first buffer address.
 * @param reg1_len		Pointer to store the length of the first buffer, 
 *				in samples.
 * @param reg2			Pointer to store the second buffer address.
 * @param reg2_len		Pointer to store the length of the second buffer, 
 *				in samples.
 */
void WsolaBufWriteRegions(WSOLA_BUF *circbuf, 
						   short **reg1, 
						   DWORD *reg1_len, 
						   short **reg2, 
						   DWORD *reg2_len)
{
	*reg1 = circbuf->start + circbuf->len;
	if (*reg1 >= circbuf->buf + circbuf->capacity)
	*reg1 -= circbuf->capacity;
	*reg1_len = circbuf->capacity - circbuf->len;
	if (*reg1 + *reg1_len > circbuf->buf + circbuf->capacity)
	{
		*reg1_len = circbuf->buf + circbuf->capacity - *reg1;
		*reg2 = circbuf->buf;
		*reg2_len = circbuf->start - circbuf->buf;
	} 
	else
	{
		*reg2 = NULL;
		*reg2_len = 0;
	}
}


/**
 * Read audio samples from the circular buffer.
 *
 * @param circbuf		The circular buffer.
 * @param data			Buffer to store the read audio samples.
 * @param count			Number of samples being read.
 *
 * @return			RC_SUCCESS when successful, otherwise 
 *				the appropriate error will be returned.
 */
int WsolaBufRead(WSOLA_BUF *circbuf, short *data, DWORD count)
{
	short *reg1, *reg2;
	DWORD reg1cnt, reg2cnt;

	/* Data in the buffer is less than requested */
	if (count > circbuf->len)
		return RC_ETOOBIG;

	WsolaBufReadRegions(circbuf, &reg1, &reg1cnt, 
					  &reg2, &reg2cnt);
	if (reg1cnt >= count) 
	{
		WsolaPortCopySamples(data, reg1, count);
	}
	else 
	{
		WsolaPortCopySamples(data, reg1, reg1cnt);
		WsolaPortCopySamples(data + reg1cnt, reg2, count - reg1cnt);
	}

	return WsolaBufReadPtr(circbuf, count);
}


/**
 * Write audio samples to the circular buffer.
 *
 * @param circbuf		The circular buffer.
 * @param data			Audio samples to be written.
 * @param count			Number of samples being written.
 *
 * @return			RC_SUCCESS when successful, otherwise
 *				the appropriate error will be returned.
 */
int WsolaBufWrite(WSOLA_BUF *circbuf, short *data, DWORD count)
{
	short *reg1, *reg2;
	DWORD reg1cnt, reg2cnt;

	/* Data to write is larger than buffer can store */
	if (count > circbuf->capacity - circbuf->len)
		return RC_ETOOBIG;

	WsolaBufWriteRegions(circbuf, &reg1, &reg1cnt, 
					   &reg2, &reg2cnt);
	if (reg1cnt >= count) 
	{
		WsolaPortCopySamples(reg1, data, count);
	}
	else 
	{
		WsolaPortCopySamples(reg1, data, reg1cnt);
		WsolaPortCopySamples(reg2, data + reg1cnt, count - reg1cnt);
	}
	
	return WsolaBufWritePtr(circbuf, count);
}


/**
 * Copy audio samples from the circular buffer without changing its state. 
 *
 * @param circbuf		The circular buffer.
 * @param start_idx		Starting sample index to be copied.
 * @param data			Buffer to store the read audio samples.
 * @param count			Number of samples being read.
 *
 * @return			RC_SUCCESS when successful, otherwise 
 *				the appropriate error will be returned.
 */
int WsolaBufCopy(WSOLA_BUF *circbuf, 
						 DWORD start_idx,
						 short *data, 
						 DWORD count)
{
	short *reg1, *reg2;
	DWORD reg1cnt, reg2cnt;

	/* Data in the buffer is less than requested */
	if (count + start_idx > circbuf->len)
		return RC_ETOOBIG;
	
	WsolaBufReadRegions(circbuf, &reg1, &reg1cnt, 
		&reg2, &reg2cnt);
	if (reg1cnt > start_idx) 
	{
		DWORD tmp_len;
		tmp_len = reg1cnt - start_idx;
		if (tmp_len > count)
			tmp_len = count;
		WsolaPortCopySamples(data, reg1 + start_idx, tmp_len);
		if (tmp_len < count)
			WsolaPortCopySamples(data + tmp_len, reg2, count - tmp_len);
	}
	else
	{
		WsolaPortCopySamples(data, reg2 + start_idx - reg1cnt, count);
	}
	
	return RC_SUCCESS;
}


/**
 * Pack the buffer so the first sample will be in the beginning of the buffer.
 * This will also make the buffer contiguous.
 *
 * @param circbuf		The circular buffer.
 *
 * @return			RC_SUCCESS when successful, otherwise 
 *				the appropriate error will be returned.
 */
int WsolaBufPack(WSOLA_BUF *circbuf)
{
	short *reg1, *reg2;
	DWORD reg1cnt, reg2cnt;
	DWORD gap;

	WsolaBufReadRegions(circbuf, &reg1, &reg1cnt, 
		&reg2, &reg2cnt);
	
	/* Check if not contigue */
	if (reg2cnt != 0)
	{
	/* Check if no space left to roll the buffer 
	* (or should this function provide temporary buffer?)
		*/
		gap = circbuf->capacity - WsolaBufGetLen(circbuf);
		if (gap == 0)
			return RC_ETOOBIG;
		
		/* Roll buffer left using the gap until reg2cnt == 0 */
		do
		{
			if (gap > reg2cnt)
				gap = reg2cnt;
			WsolaPortMoveSamples(reg1 - gap, reg1, reg1cnt);
			WsolaPortCopySamples(reg1 + reg1cnt - gap, reg2, gap);
			if (gap < reg2cnt)
				WsolaPortMoveSamples(reg2, reg2 + gap, reg2cnt - gap);
			reg1 -= gap;
			reg1cnt += gap;
			reg2cnt -= gap;
		} while (reg2cnt > 0);
	}
	
	/* Finally, Shift samples to the left edge */
	if (reg1 != circbuf->buf)
		WsolaPortMoveSamples(circbuf->buf, reg1, 
		WsolaBufGetLen(circbuf));
	circbuf->start = circbuf->buf;
	
	return RC_SUCCESS;
}

