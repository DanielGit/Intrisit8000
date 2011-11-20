
#include "wsola.h"

int WsolaConvCreate(PWSOLA_OBJECT wsola)
{
	PCONV_BUF cbuf;
	int frame_size;
	
	if(!wsola->rate)
	{
		wsola->conv_buf = NULL;
		return 0;
	}

	cbuf = WsolaPortMalloc(sizeof(CONV_BUF));
	wsola->conv_buf = cbuf;
	if(cbuf == NULL)
		return -1;
	WsolaPortMemset(cbuf, 0, sizeof(CONV_BUF));

	if(wsola->rate > 0)
		frame_size = wsola->samples_per_frame * sizeof(short);
	else
		frame_size = wsola->samples_per_frame * sizeof(short) * 10;

	cbuf->frame_buf = WsolaPortMalloc(frame_size);
	if(cbuf->frame_buf == NULL)
	{
		WsolaPortFree(wsola->conv_buf);
		wsola->conv_buf = NULL;
		return -1;
	}
	cbuf->frame_size = frame_size;
	
	cbuf->cache_buf = WsolaPortMalloc(frame_size);
	if(cbuf->cache_buf == NULL)
	{
		WsolaPortFree(cbuf->frame_buf);
		WsolaPortFree(wsola->conv_buf);
		wsola->conv_buf = NULL;
		return -1;
	}
	cbuf->cache_size = 0;
	return 0;
}

void WsolaConvDestroy(PWSOLA_OBJECT wsola)
{
	if(wsola->conv_buf)
	{
		if(wsola->conv_buf->frame_buf)
			WsolaPortFree(wsola->conv_buf->frame_buf);
		if(wsola->conv_buf->cache_buf)
			WsolaPortFree(wsola->conv_buf->cache_buf);
		WsolaPortFree(wsola->conv_buf);
	}
}


static int WsolaConvReadFrame(PWSOLA_OBJECT wsola, PAUDIO_FILTER filter)
{
	PCONV_BUF cbuf;
	int frame_size;
	int rsize;

	cbuf = wsola->conv_buf;
	frame_size = cbuf->frame_size;

	// 检查cache中和Filter中的数据是否足够
	rsize = cbuf->cache_size + filter->iSize;
	if(rsize < frame_size)
	{
		// 检查是否最后一帧
		if(rsize && !filter->iSize)
		{
			WsolaPortMemcpy(cbuf->frame_buf, cbuf->cache_buf, cbuf->cache_size);
			WsolaPortMemset(cbuf->frame_buf + cbuf->cache_size, 0x00, frame_size - cbuf->cache_size);
			cbuf->cache_size = 0;
			cbuf->filter_off = 0;
			filter->iSize = 0;
			return 1;
		}
		WsolaPortMemcpy(cbuf->cache_buf+cbuf->cache_size, (char*)filter->iBuf+cbuf->filter_off, filter->iSize);
		cbuf->cache_size += filter->iSize;
		cbuf->filter_off = 0;
		filter->iSize = 0;
		return 0;
	}	
	
	// 复制Cache中的数据到frame_buf
	if(cbuf->cache_size)
		WsolaPortMemcpy(cbuf->frame_buf, cbuf->cache_buf, cbuf->cache_size);

	// 复制filter中的数据到frame_buf
	rsize = frame_size - cbuf->cache_size;
	WsolaPortMemcpy(cbuf->frame_buf+cbuf->cache_size, (char*)filter->iBuf+cbuf->filter_off, rsize);
	cbuf->cache_size = 0;
	filter->iSize -= rsize;
	if(filter->iSize)
		cbuf->filter_off += rsize;
	else
		cbuf->filter_off = 0;
	return 1;
}


int WsolaConvert(HANDLE hwsola, PAUDIO_FILTER filter)
{
	int rate;
	int osize;
	int frame_size;
	short *frame;
	char *opcm;
	WSOLA_OBJECT *wsola;

	wsola= (WSOLA_OBJECT*)hwsola;
	if(!wsola)
		return -1;
	frame_size = wsola->conv_buf->frame_size;
	frame = (short*)wsola->conv_buf->frame_buf;
	opcm = (char*)filter->oBuf;
	osize = 0;
	
	// 不变速处理
	if(!wsola->rate)
	{
		osize = (filter->iSize > filter->oSize) ? filter->oSize : filter->iSize;
		WsolaPortMemcpy(opcm, filter->iBuf, osize);
		filter->oSize = osize;
		return 0;
	}
	
	// 速度变慢处理
	if(wsola->rate > 0)
	{
		while(WsolaConvReadFrame(wsola, filter)) 
		{
			WsolaSave(wsola, frame, 0);
			WsolaPortMemcpy(opcm, frame, frame_size);
			
			osize += frame_size;
			opcm += frame_size;
			wsola->iframes++;
			wsola->oframes++;
			rate = ((wsola->oframes - wsola->iframes) * 10) / wsola->iframes;
			if(rate < wsola->rate)
			{
				WsolaGenerate(wsola, frame);
				WsolaPortMemcpy(opcm, frame, frame_size);
				opcm += frame_size;
				osize += frame_size;
				wsola->oframes++;
			} 
		}
		filter->oSize = osize;
		return 0;
	}
	
	// 速度变快处理
	rate = -wsola->rate;
    while(WsolaConvReadFrame(wsola, filter)) 
	{
		DWORD count;
		int i;
		
		count = frame_size / sizeof(short);
		for (i=0; i<rate; i++) 
		{
			DWORD to_del = wsola->samples_per_frame / 2;
			WsolaDiscard(wsola, frame, count, NULL, 0, &to_del);
			count -= to_del;
		}
		WsolaPortMemcpy(opcm, frame, count*sizeof(short));
		osize += count*sizeof(short);
		opcm += count*sizeof(short);
    }
	filter->oSize = osize;
	return 0;
}

