/*

 TiMidity -- Experimental MIDI to WAVE converter
 Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
	
	 instrum.c 
	 
	  Code to load and unload GUS-compatible instrument patches.
	  
*/

#include <config.h>
#include <kernel/kernel.h>
#include <direct/medialib/MidiCfg.h>
#include <direct/medialib/MidiDecodec.h>

#ifdef CONFIG_DECODE_MIDI_ENABLE

//#define KPRINTF_DT(x,s...) {}	//{kprintf("%s:%d  ",__FILE__,__LINE__); kprintf(x,##s); kprintf("\n");}

static Instrument *load_instrument(PMIDI_DECODEC hMidi,char *name, int drum, int bank, int tone);
Instrument *load_instrumentex(PMIDI_DECODEC hMidi, FILE *file, int drum, int bank, int tone);

static Instrument *midi_load_instrument(PMIDI_DECODEC hMidi,FHND file, int drum, int bank, int tone)
{
	int i, index, data_length;
	Instrument *ip;

	// 跳过文件头
	FSEEK(file, 0, SEEK_SET);
	if(FREAD(&index, sizeof(int), 1, file) != 1)
		return NULL;

	// 获取Bank索引
	index += bank * sizeof(int);
	if(drum)
		index += 128 * sizeof(int);

	// 获取Tone索引
	FSEEK(file, index, SEEK_SET);
	if(FREAD(&index, sizeof(int), 1, file) != 1)
		return NULL;
	if(index == 0)
		return NULL;
	index += tone * sizeof(int);

	// 获取乐器索引
	FSEEK(file, index, SEEK_SET);
	if(FREAD(&index, sizeof(int), 1, file) != 1)
		return NULL;
	if(index == 0)
		return NULL;

	// 申请乐器波表信息空间
	ip = (Instrument*)hMidi->MidiMalloc(sizeof(Instrument));
//	KPRINTF_DT("malloc ip = %x\n",ip);

	if(ip == NULL)
		return NULL;
	FSEEK(file, index, SEEK_SET);
	if(FREAD(&index, sizeof(int), 1, file) != 1)
	{
		hMidi->MidiFree(ip);
//		KPRINTF_DT("free ip = %x\n",ip);
		return NULL;
	}
	ip->samples = index;
	ip->sample = hMidi->MidiMalloc(sizeof(Sample)*index);
//	KPRINTF_DT("malloc ip->sample = %x\n",ip->sample);

	if(ip->sample == NULL)
	{
		hMidi->MidiFree(ip);
//		KPRINTF_DT("free ip = %x\n",ip);
		return NULL;
	}
	
	// 读取波表信息
	if(FREAD(ip->sample, sizeof(Sample), index, file) != index)
	{
		hMidi->MidiFree(ip->sample);
		hMidi->MidiFree(ip);
// 		KPRINTF_DT("free ip = %x\n",ip);
// 		KPRINTF_DT("free ip->sample = %x\n",ip->sample);
		return NULL;
	}

	// 读取采样数据信息
	for(i=0; i<ip->samples; i++)
	{
		index = (int)ip->sample[i].data;
		data_length = (ip->sample[i].data_length >> FRACTION_BITS) * 2;
		ip->sample[i].data = (sample_t*)hMidi->MidiMalloc(data_length);
//		KPRINTF_DT("malloc ip->sample[%d].data = %x\n",i,ip->sample[i].data);

		if(ip->sample[i].data == NULL)
		{
			while(--i >= 0)
			{
				hMidi->MidiFree(ip->sample[i].data);
//				KPRINTF_DT("free ip->sample[%d].data = %x\n",i,ip->sample[i].data);
			}
			hMidi->MidiFree(ip->sample);
			hMidi->MidiFree(ip);
// 			KPRINTF_DT("free ip = %x\n",ip);
// 			KPRINTF_DT("free ip->sample = %x\n",ip->sample);
			return NULL;
		}
		FSEEK(file, index, SEEK_SET);
		if(FREAD(ip->sample[i].data, sizeof(char), data_length, file) != data_length)
		{
			while(--i >= 0)
			{
				hMidi->MidiFree(ip->sample[i].data);
//				KPRINTF_DT("free ip->sample[%d].data = %x\n",i,ip->sample[i].data);
			}
			hMidi->MidiFree(ip->sample);
			hMidi->MidiFree(ip);
// 			KPRINTF_DT("free ip = %x\n",ip);
// 			KPRINTF_DT("free ip->sample = %x\n",ip->sample);
			return NULL;
		}
	}
	
	return ip;
}

static Instrument *load_instrument(PMIDI_DECODEC hMidi,char *name, int drum, int bank, int tone)
{
	FHND file;
	Instrument *ip;

	file = FOPEN(name, "rb");
	if(file == 0)
		return NULL;
	ip = midi_load_instrument(hMidi,file, drum, bank, tone);
	FCLOSE(file);
	return ip;
}


static void free_instrument(PMIDI_DECODEC hMidi,Instrument *ip)
{
	Sample *sp;
	int i;
	if (!ip) return;
	for (i=0; i<ip->samples; i++)
    {
		sp=&(ip->sample[i]);
		hMidi->MidiFree(sp->data);
//		KPRINTF_DT("free sp->data = %x\n",sp->data);
    }
	hMidi->MidiFree(ip->sample);
	hMidi->MidiFree(ip);
// 	KPRINTF_DT("free ip->sample = %x\n",ip->sample);
// 	KPRINTF_DT("free ip = %x\n",ip);
}

static void free_bank(PMIDI_DECODEC hMidi,int dr, int b)
{
	int i;
	ToneBank *bank=((dr) ? hMidi->drumset[b] : hMidi->tonebank[b]);
	for (i=0; i<128; i++)
	{
		if (bank->instrument[i] != NULL)
		{
			/* Not that this could ever happen, of course */
			if (bank->instrument[i] != MAGIC_LOAD_INSTRUMENT)
				free_instrument(hMidi,bank->instrument[i]);
			bank->instrument[i]=0;
		}
	}
}

static int fill_bank(PMIDI_DECODEC hMidi,char *wtb, int dr, int b)
{
	int i, errors=0;
	ToneBank *bank=((dr) ? hMidi->drumset[b] : hMidi->tonebank[b]);
	for (i=0; i<128; i++)
    {
		if (bank->instrument[i]==MAGIC_LOAD_INSTRUMENT)
		{
			if(dr)
			{
				bank->instrument[i]=
					load_instrument(hMidi,wtb, 1, b, i); 
			}
			else
			{
				bank->instrument[i]=
					load_instrument(hMidi,wtb, 0, b, i); 
			}
			
			if (!bank->instrument[i])
			{
				MidiMsg("Couldn't load instrument %s %d, %d\n",
					(dr)? "drum set" : "tone bank", b, i);
				errors++;
			}				
		}
    }
	return errors;
}


int init_instruments(PMIDI_DECODEC hMidi,char *wtb)
{
	int index;
	FHND file;
	
	file = FOPEN(wtb, "rb");
	if(file == 0)
		return -1;
	
	FSEEK(file, 0, SEEK_SET);
	if(FREAD(&index, sizeof(int), 1, file) != 1)
	{
		FCLOSE(file);
		return -1;
	}
	FSEEK(file, index, SEEK_SET);
	if(FREAD(hMidi->tonebank, sizeof(int), 128, file) != 128)
	{
		kmemset(hMidi->tonebank, 0, sizeof(int)*128);
		FCLOSE(file);
		return -1;
	}
	if(FREAD(hMidi->drumset, sizeof(int), 128, file) != 128)
	{
		kmemset(hMidi->tonebank, 0, sizeof(int)*128);
		kmemset(hMidi->drumset, 0, sizeof(int)*128);
		FCLOSE(file);
		return -1;
	}
	for(index=0; index<128; index++)
	{
		if(hMidi->tonebank[index] != NULL)
		{
			hMidi->tonebank[index] = hMidi->MidiMalloc(sizeof(ToneBank));
//			KPRINTF_DT("malloc hMidi->tonebank[%d] = %x\n",index,hMidi->tonebank[index]);
			if(hMidi->tonebank[index] == NULL)
			{
				while(index-- >= 0 )
				{
					if(hMidi->tonebank[index])
					{
						hMidi->MidiFree(hMidi->tonebank[index]);
//						KPRINTF_DT("free hMidi->tonebank[%d] = %x\n",index,hMidi->tonebank[index]);

					}
					if(hMidi->drumset[index])
					{
//						KPRINTF_DT("free hMidi->drumset[%d] = %x\n",index,hMidi->drumset[index]);
						hMidi->MidiFree(hMidi->drumset[index]);
					}
				}
				FCLOSE(file);
				return -1;
			}
			kmemset(hMidi->tonebank[index], 0x00, sizeof(ToneBank));
		}
		if(hMidi->drumset[index] != NULL)
		{
			hMidi->drumset[index] = hMidi->MidiMalloc(sizeof(ToneBank));
//			KPRINTF_DT("malloc hMidi->drumset[%d] = %x\n",index,hMidi->drumset[index]);
			if(hMidi->drumset[index] == NULL)
			{
				while(index-- >= 0)
				{
					if(hMidi->tonebank[index])
					{
						hMidi->MidiFree(hMidi->tonebank[index]);
//						KPRINTF_DT("free hMidi->tonebank[%d] = %x\n",index,hMidi->tonebank[index]);
					}
					if(hMidi->drumset[index])
					{
						hMidi->MidiFree(hMidi->drumset[index]);
//						KPRINTF_DT("free hMidi->drumset[%d] = %x\n",index,hMidi->drumset[index]);
					}
				}
				FCLOSE(file);
				return -1;
			}
			kmemset(hMidi->drumset[index], 0x00, sizeof(ToneBank));
		}
	}
	FCLOSE(file);
	return 0;
}

int load_usr_instruments(PMIDI_DECODEC hMidi,int drum, int bank, int tone)
{
	if((bank >= 128) || (tone >= 128))
		return -1;
	if((bank < 0) || (tone < 0))
		return -1;
	if(drum)
	{
		if(hMidi->drumset[bank] == NULL)
			return -1;
		hMidi->drumset[bank]->instrument[tone] = MAGIC_LOAD_INSTRUMENT;
	}
	else
	{
		if(hMidi->tonebank[bank] == NULL)
			return -1;
		hMidi->tonebank[bank]->instrument[tone] = MAGIC_LOAD_INSTRUMENT;
	}
	return 0;
}

int load_instruments(PMIDI_DECODEC hMidi,char *wtb)
{
	int i=128,errors=0;
	for( i = 0 ; i < 128 ; i++ )
	{
		if (hMidi->tonebank[127-i])
			errors+=fill_bank(hMidi,wtb, 0,127-i);
		if (hMidi->drumset[127-i])
			errors+=fill_bank(hMidi,wtb, 1,127-i);
	}
	return errors;
}

void free_instruments(PMIDI_DECODEC hMidi)
{
	int i=128;
	for( i = 0 ; i < 128 ; i++ )
	{
		if (hMidi->tonebank[127 - i])
		{
			free_bank(hMidi,0,127 - i);
			hMidi->MidiFree(hMidi->tonebank[127 - i]);
//			KPRINTF_DT("free hMidi->tonebank[%d] = %x\n",127-i,hMidi->tonebank[127-i]);
			hMidi->tonebank[127 - i] = NULL;
		}
		if (hMidi->drumset[127 - i])
		{
			free_bank(hMidi,1,127 - i);
			hMidi->MidiFree(hMidi->drumset[127 - i]);
//			KPRINTF_DT("free hMidi->drumset[%d] = %x\n",127-i,hMidi->drumset[127-i]);
			hMidi->drumset[127 - i] = NULL;
		}
	}

	if(hMidi->default_instrument)
	{
		free_instrument(hMidi,hMidi->default_instrument);
		hMidi->default_instrument = NULL;
	}
	hMidi->default_program=DEFAULT_PROGRAM;
}


int set_default_instrument(PMIDI_DECODEC hMidi,char *wtb, int dr, int bank, int tone)
{
	Instrument *ip;

	ip=load_instrument(hMidi,wtb, dr, bank, tone);
	if (!ip)
		return -1;
	if(hMidi->default_instrument)
		free_instrument(hMidi,hMidi->default_instrument);
	hMidi->default_instrument=ip;
	hMidi->default_program=SPECIAL_PROGRAM;
	return 0;
}

#endif // DECODE_MIDI_ENABLE
