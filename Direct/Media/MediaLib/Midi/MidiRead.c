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
	
*/

#include <config.h>
#include <kernel/kernel.h>
#include <direct/medialib/MidiCfg.h>
#include <direct/medialib/MidiDecodec.h>

#ifdef CONFIG_DECODE_MIDI_ENABLE

//#define KPRINTF_DT(x,s...) {}	//{kprintf("%s:%d  ",__FILE__,__LINE__); kprintf(x,##s); kprintf("\n");}

/* These would both fit into 32 bits, but they are often added in
large multiples, so it's simpler to have two roomy ints */
static int32 sample_increment, sample_correction; /*samples per MIDI delta-t*/

static void skip_midi(PMIDI_DECODEC hMidi ,int size)
{
	int getbytes;
	
	while(size > 0)
	{
		if(hMidi->midi_buf_remain == 0)
		{
			hMidi->midi_buf_remain = hMidi->midi_callback(hMidi->midi_file,hMidi->midi_buf, READ_MIDI_FILE_SIZE);
			hMidi->pmidi_buf = hMidi->midi_buf;
			if(hMidi->midi_buf_remain == 0)
				break;
		}
		getbytes = (size > hMidi->midi_buf_remain) ? hMidi->midi_buf_remain : size;
		hMidi->midi_buf_remain -= getbytes;
		hMidi->pmidi_buf += getbytes;
		size -= getbytes;
	}
}

static int read_midi(PMIDI_DECODEC hMidi ,void *pbuf, int size, int count)
{
	int rsize;
	int getbytes;
	char *buf;
	
	if(hMidi->midi_callback == NULL)
		return 0;
	buf =(char*)pbuf;
	rsize = size * count;
	while(rsize > 0)
	{
		if(hMidi->midi_buf_remain == 0)
		{
			hMidi->midi_buf_remain = hMidi->midi_callback(hMidi->midi_file,hMidi->midi_buf, READ_MIDI_FILE_SIZE);
			hMidi->pmidi_buf = hMidi->midi_buf;
			if(hMidi->midi_buf_remain == 0)
				break;
		}
		getbytes = (rsize > hMidi->midi_buf_remain) ? hMidi->midi_buf_remain : rsize;
		kmemcpy(buf, hMidi->pmidi_buf, getbytes);
		buf += getbytes;
		hMidi->pmidi_buf += getbytes;
		hMidi->midi_buf_remain -= getbytes;
		rsize -= getbytes;
	}
	if(rsize == 0)
		return count;
	return (size * count - rsize) / size;
}

/* Computes how many (fractional) samples one MIDI delta-time unit contains */
static void compute_sample_increment(int32 tempo, int32 divisions)
{
	double a;
	a = (double) (tempo) * (double) DEFAULT_RATE * (65536.0/1000000.0) /
		(double)(divisions);
	
	sample_correction = (int32)(a) & 0xFFFF;
	sample_increment = (int32)(a) >> 16;
	
	MidiMsg("Samples per delta-t: %d (correction %d)\n", sample_increment, sample_correction);
}

/* Read variable-length number (7 bits per byte, MSB first) */
static int32 getvl(PMIDI_DECODEC hMidi)
{
	int32 l=0;
	uint8 c;
	for (;;)
    {
		read_midi(hMidi,&c,1,1);
		l += (c & 0x7f);
		if (!(c & 0x80)) return l;
		l<<=7;
    }
}

/* Print a string from the file, followed by a newline. Any non-ASCII
or unprintable characters will be converted to periods. */
static int dumpstring(PMIDI_DECODEC hMidi ,int32 len, char *label)
{
	signed char *s=hMidi->MidiMalloc(len+1);
//	KPRINTF_DT("malloc s = %x\n",s);

	if (len != (int)read_midi(hMidi,s, 1, len))
    {
		hMidi->MidiFree(s);
//		KPRINTF_DT("free s = %x\n",s);
		return -1;
    }
	s[len]='\0';
	while (len--)
    {
		if (s[len]<32)
			s[len]='.';
    }
	MidiMsg("%s%s\n", label, s);
	hMidi->MidiFree(s);
//	KPRINTF_DT("free s = %x\n",s);
	return 0;
}

#define MIDIEVENT(hMidi,at,t,ch,pa,pb) \
	new=hMidi->MidiMalloc(sizeof(MidiEventList)); \
	new->event.time=at; new->event.type=t; new->event.channel=ch; \
	new->event.a=pa; new->event.b=pb; new->next=0;\
return new;

#define MAGIC_EOT ((MidiEventList *)(-1))

/* Read a MIDI event, returning a freshly allocated element that can
be linked to the event list */
static MidiEventList *read_midi_event(PMIDI_DECODEC hMidi)
{
	static uint8 laststatus, lastchan;
	static uint8 nrpn=0, rpn_msb[16], rpn_lsb[16]; /* one per channel */
	uint8 me, type, a,b,c;
	int32 len;
	MidiEventList *new;
	
	for (;;)
    {
		hMidi->at+=getvl(hMidi);
		if (read_midi(hMidi,&me,1,1)!=1)
		{
			MidiMsg("read_midi_event\n");
			return 0;
		}
		
		if(me==0xF0 || me == 0xF7) /* SysEx event */
		{
			len=getvl(hMidi);
			skip_midi(hMidi,len);
		}
		else if(me==0xFF) /* Meta event */
		{
			read_midi(hMidi,&type,1,1);
			len=getvl(hMidi);
			if (type>0 && type<16)
			{
				static char *label[]={
					"Text event: ", "Text: ", "Copyright: ", "Track name: ",
						"Instrument: ", "Lyric: ", "Marker: ", "Cue point: "};
					dumpstring(hMidi,len, label[(type>7) ? 0 : type]);
			}
			else
			{
				switch(type)
				{
				case 0x2F: /* End of Track */
					return MAGIC_EOT;
					
				case 0x51: /* Tempo */
					read_midi(hMidi,&a,1,1); read_midi(hMidi,&b,1,1); read_midi(hMidi,&c,1,1);
					MIDIEVENT(hMidi,hMidi->at, ME_TEMPO, c, a, b);
					
				default:
					MidiMsg("(Meta event type 0x%02x, length %ld)\n", type, len);
					skip_midi(hMidi,len);
					break;
				}
			}
		}
		else
		{
			a=me;
			if (a & 0x80) /* status byte */
			{
				lastchan=a & 0x0F;
				laststatus=(a>>4) & 0x07;
				read_midi(hMidi,&a, 1,1);
				a &= 0x7F;
			}
			switch(laststatus)
			{
			case 0: /* Note off */
				read_midi(hMidi,&b, 1,1);
				b &= 0x7F;
				MIDIEVENT(hMidi,hMidi->at, ME_NOTEOFF, lastchan, a,b);
				
			case 1: /* Note on */
				read_midi(hMidi,&b, 1,1);
				b &= 0x7F;
				MIDIEVENT(hMidi,hMidi->at, ME_NOTEON, lastchan, a,b);
				
			case 2: /* Key Pressure */
				read_midi(hMidi,&b, 1,1);
				b &= 0x7F;
				MIDIEVENT(hMidi,hMidi->at, ME_KEYPRESSURE, lastchan, a, b);
				
			case 3: /* Control change */
				read_midi(hMidi,&b, 1,1);
				b &= 0x7F;
				{
					int control=255;
					switch(a)
					{
					case 7: control=ME_MAINVOLUME; break;
					case 10: control=ME_PAN; break;
					case 11: control=ME_EXPRESSION; break;
					case 64: control=ME_SUSTAIN; break;
					case 120: control=ME_ALL_SOUNDS_OFF; break;
					case 121: control=ME_RESET_CONTROLLERS; break;
					case 123: control=ME_ALL_NOTES_OFF; break;
						
					/* These should be the SCC-1 tone bank switch
					commands. I don't know why there are two, or
					why the latter only allows switching to bank 0.
					Also, some MIDI files use 0 as some sort of
					continuous controller. This will cause lots of
						warnings about undefined tone banks. */
					case 0: control=ME_TONE_BANK; break;
					case 32: 
						if (b!=0)
							MidiMsg("(Strange: tone bank change 0x20%02x)\n", b);
						else
							control=ME_TONE_BANK;
						break;
						
					case 100: nrpn=0; rpn_msb[lastchan]=b; break;
					case 101: nrpn=0; rpn_lsb[lastchan]=b; break;
					case 99: nrpn=1; rpn_msb[lastchan]=b; break;
					case 98: nrpn=1; rpn_lsb[lastchan]=b; break;
						
					case 6:
						if (nrpn)
						{
							MidiMsg("(Data entry (MSB) for NRPN %02x,%02x: %ld)\n",
								rpn_msb[lastchan], rpn_lsb[lastchan], b);
							break;
						}
						
						switch((rpn_msb[lastchan]<<8) | rpn_lsb[lastchan])
						{
						case 0x0000: /* Pitch bend sensitivity */
							control=ME_PITCH_SENS;
							break;
							
						case 0x7F7F: /* RPN reset */
							/* reset pitch bend sensitivity to 2 */
							MIDIEVENT(hMidi,hMidi->at, ME_PITCH_SENS, lastchan, 2, 0);
							
						default:
							MidiMsg("(Data entry (MSB) for RPN %02x,%02x: %ld)\n",
								rpn_msb[lastchan], rpn_lsb[lastchan], b);
							break;
						}
						break;
						
						default:
							MidiMsg("(Control %d: %d)\n", a, b);
							break;
					}
					if (control != 255)
					{ 
						MIDIEVENT(hMidi,hMidi->at, control, lastchan, b, 0); 
					}
				}
				break;
				
			case 4: /* Program change */
				a &= 0x7f;
				MIDIEVENT(hMidi,hMidi->at, ME_PROGRAM, lastchan, a, 0);
				
			case 5: /* Channel pressure - NOT IMPLEMENTED */
				break;
				
			case 6: /* Pitch wheel */
				read_midi(hMidi,&b, 1,1);
				b &= 0x7F;
				MIDIEVENT(hMidi,hMidi->at, ME_PITCHWHEEL, lastchan, a, b);
				
			default: 
				MidiMsg("*** Can't happen: status 0x%02X, channel 0x%02X\n",
					laststatus, lastchan);
				break;
			}
		}
    }
//	return new;
}

#undef MIDIEVENT

/* Read a midi track into the linked list, either merging with any previous
tracks or appending to them. */
static int read_track(PMIDI_DECODEC hMidi ,int append)
{
	MidiEventList *meep;
	MidiEventList *next, *new;
	int32 len;
	char tmp[4];
	
	meep=hMidi->evlist;
	if (append && meep)
    {
		/* find the last event in the list */
		for (; meep->next; meep=meep->next)
			;
		hMidi->at=meep->event.time;
    }
	else
		hMidi->at=0;
	
	/* Check the formalities */
	
	if ((read_midi(hMidi,tmp,1,4) != 4) || (read_midi(hMidi,&len,4,1) != 1))
    {
		MidiMsg("Can't read track header.\n");
		return -1;
    }
	len=BE_LONG(len);
	if (kmemcmp(tmp, "MTrk", 4))
    {
		MidiMsg("Corrupt MIDI file.\n");
		return -2;
    }
	
	for (;;)
    {
    	new = read_midi_event(hMidi);
		if (!new) /* Some kind of error  */
			return -2;
		
		if (new==MAGIC_EOT) /* End-of-track Hack. */
		{
			return 0;
		}
		
		next=meep->next;
		while (next && (next->event.time < new->event.time))
		{
			meep=next;
			next=meep->next;
		}
		
		new->next=next;
		meep->next=new;
		
		hMidi->event_count++; /* Count the event. (About one?) */
		meep=new;
    }
}

/* Free the linked event list from memory. */
static void free_midi_list(PMIDI_DECODEC hMidi)
{
	MidiEventList *meep, *next;
	meep = hMidi->evlist;
	if (!meep)
	 return;
	while (meep)
    {
		next=meep->next;
		hMidi->MidiFree(meep);
//		KPRINTF_DT("free meep = %x\n",meep);
		meep=next;
    }
	hMidi->evlist=0;
}

/* Allocate an array of MidiEvents and fill it from the linked list of
events, marking used instruments for loading. Convert event times to
samples: handle tempo changes. Strip unnecessary events from the list.
Free the linked list. */
static MidiEvent *groom_list(PMIDI_DECODEC hMidi ,int32 divisions,int32 *eventsp,int32 *samplesp)
{
	MidiEvent *groomed_list, *lp;
	MidiEventList *meep;
	int32 i, our_event_count, tempo, skip_this_event, new_value;
	int32 sample_cum, samples_to_do, at, st, dt, counting_time;
	
	int current_bank[16], current_set[16], current_program[16]; 
	/* Or should each bank have its own current program? */
	
	for (i=0; i<16; i++)
    {
		current_bank[i]=0;
		current_set[i]=0;
		current_program[i]=hMidi->default_program;
    }
	
	tempo=500000;
	compute_sample_increment(tempo, divisions);
	
	/* This may allocate a bit more than we need */
	groomed_list=lp=hMidi->MidiMalloc(sizeof(MidiEvent) * (hMidi->event_count+1));
//	KPRINTF_DT("malloc groomed_list = %x\n",groomed_list);

	meep=hMidi->evlist;
	
	our_event_count=0;
	st=at=sample_cum=0;
	counting_time=2; /* We strip any silence before the first NOTE ON. */
	
	for (i=0; i<hMidi->event_count; i++)
    {
		skip_this_event=0;
/*		MidiMsg("%6d: ch %2d: event %d (%d,%d)",
			meep->event.time, meep->event.channel + 1,
			meep->event.type, meep->event.a, meep->event.b);
*/		
		if (meep->event.type==ME_TEMPO)
		{
			tempo=
				meep->event.channel + meep->event.b * 256 + meep->event.a * 65536;
			compute_sample_increment(tempo, divisions);
			skip_this_event=1;
		}
		else if ((hMidi->quietchannels & (1<<meep->event.channel)))
		{
			skip_this_event=1;
		}
		else 
		{
			switch (meep->event.type)
			{
			case ME_PROGRAM:
				if (ISDRUMCHANNEL(hMidi,meep->event.channel))
				{
					if (hMidi->drumset[meep->event.a]) /* Is this a defined drumset? */
						new_value=meep->event.a;
					else
					{
						MidiMsg("Drum set %d is undefined\n", meep->event.a);
						new_value=meep->event.a=0;
					}
					if (current_set[meep->event.channel] != new_value)
						current_set[meep->event.channel]=new_value;
					else 
						skip_this_event=1;
				}
				else
				{
					new_value=meep->event.a;
					if ((current_program[meep->event.channel] != SPECIAL_PROGRAM)
						&& (current_program[meep->event.channel] != new_value))
						current_program[meep->event.channel] = new_value;
					else
						skip_this_event=1;
				}
				break;
				
			case ME_NOTEON:
				if (counting_time)
					counting_time=1;
				if (ISDRUMCHANNEL(hMidi,meep->event.channel))
				{
					/* Mark this instrument to be loaded */
					if (!(hMidi->drumset[current_set[meep->event.channel]]
						->instrument[meep->event.a]))
						hMidi->drumset[current_set[meep->event.channel]]
						->instrument[meep->event.a]=
						MAGIC_LOAD_INSTRUMENT;
				}
				else
				{
					if (current_program[meep->event.channel]==SPECIAL_PROGRAM)
						break;
					/* Mark this instrument to be loaded */
					if (!(hMidi->tonebank[current_bank[meep->event.channel]]
						->instrument[current_program[meep->event.channel]]))
						hMidi->tonebank[current_bank[meep->event.channel]]
						->instrument[current_program[meep->event.channel]]=
						MAGIC_LOAD_INSTRUMENT;
				}
				break;
				
			case ME_TONE_BANK:
				if (ISDRUMCHANNEL(hMidi,meep->event.channel))
				{
					skip_this_event=1;
					break;
				}
				if (hMidi->tonebank[meep->event.a]) /* Is this a defined tone bank? */
					new_value=meep->event.a;
				else 
				{
					MidiMsg("Tone bank %d is undefined\n", meep->event.a);
					new_value=meep->event.a=0;
				}
				if (current_bank[meep->event.channel]!=new_value)
					current_bank[meep->event.channel]=new_value;
				else
					skip_this_event=1;
				break;
			}
		}
		
		/* Recompute time in samples*/
		dt=meep->event.time - at;
		if (dt && !counting_time)
		{
			samples_to_do=sample_increment * dt;
			sample_cum += sample_correction * dt;
			if (sample_cum & 0xFFFF0000)
			{
				samples_to_do += ((sample_cum >> 16) & 0xFFFF);
				sample_cum &= 0x0000FFFF;
			}
			st += samples_to_do;
		}
		else if (counting_time==1) counting_time=0;
		if (!skip_this_event)
		{
			/* Add the event to the list */
			*lp=meep->event;
			lp->time=st;
			lp++;
			our_event_count++;
		}
		at=meep->event.time;
		meep=meep->next;
    }
	/* Add an End-of-Track event */
	lp->time=st;
	lp->type=ME_EOT;
	our_event_count++;
	free_midi_list(hMidi);
	
	*eventsp=our_event_count;
	*samplesp=st;
	return groomed_list;
}

MidiEvent *read_midi_file(PMIDI_DECODEC hMidi ,int32 *count, int32 *sp)
{
	int32 len, divisions;
	int16 format, tracks, divisions_tmp;
	int i;
	char tmp[4];

	hMidi->event_count=0;
	hMidi->at=0;
	hMidi->evlist=0;
	
	if ((read_midi(hMidi,tmp,1,4) != 4) || (read_midi(hMidi,&len,4,1) != 1))
    {
		MidiMsg("Not a MIDI file!\n");
		return 0;
    }
	len=BE_LONG(len);
	if (kmemcmp(tmp, "MThd", 4) || len < 6)
    {
		MidiMsg("Not a MIDI file!\n");
		return 0;
    }
	
	read_midi(hMidi,&format, 2, 1);
	read_midi(hMidi,&tracks, 2, 1);
	read_midi(hMidi,&divisions_tmp, 2, 1);
	format=BE_SHORT(format);
	tracks=BE_SHORT(tracks);
	divisions_tmp=BE_SHORT(divisions_tmp);
	
	if (divisions_tmp<0)
    {
		/* SMPTE time -- totally untested. Got a MIDI file that uses this? */
		divisions=
			(int32)(-(divisions_tmp/256)) * (int32)(divisions_tmp & 0xFF);
    }
	else divisions=(int32)(divisions_tmp);
	
	if (len > 6)
    {
		MidiMsg("MIDI file header size %ld bytes\n", len);
		skip_midi(hMidi,len-6); /* skip the excess */
    }
	if (format<0 || format >2)
    {
		MidiMsg("Unknown MIDI file format %d\n", format);
		return 0;
    }
	MidiMsg("Format: %d  Tracks: %d  Divisions: %d\n", format, tracks, divisions);
	
	/* Put a do-nothing event first in the list for easier processing */
	hMidi->evlist=hMidi->MidiMalloc(sizeof(MidiEventList));
//	KPRINTF_DT("malloc hMidi->evlist = %x\n",hMidi->evlist);
	hMidi->evlist->event.time=0;
	hMidi->evlist->event.type=ME_NONE;
	hMidi->evlist->next=0;
	hMidi->event_count++;
	
	switch(format)
    {
    case 0:
		if (read_track(hMidi,0))
		{
			free_midi_list(hMidi);
			return 0;
		}
		break;
		
    case 1:
		for (i=0; i<tracks; i++)
		{
			if (read_track(hMidi,0))
			{
				free_midi_list(hMidi);
				return 0;
			}
		}
		break;
		
    case 2: /* We simply play the tracks sequentially */
		for (i=0; i<tracks; i++)
		{
			if (read_track(hMidi,1))
			{
				free_midi_list(hMidi);
				return 0;
			}
		}
		break;
    }
	return groom_list(hMidi,divisions, count, sp);
}


void free_midi_file(PMIDI_DECODEC hMidi ,MidiEvent *evlist)
{
	hMidi->MidiFree(evlist);
//	KPRINTF_DT("free evlist = %x\n",evlist);
}

int get_midi_file_time(PMIDI_DECODEC hMidi ,FHND file, void* cb)
{
	int32 events;
	int32 samples;
	int secs;
	MidiEvent *event;

	hMidi->midi_callback   = cb;
	hMidi->midi_file       = file;
	hMidi->midi_buf_remain = 0;
		
	event = read_midi_file(hMidi,&events, &samples);
	secs=samples/DEFAULT_RATE;
	free_midi_file(hMidi,event);
	
	return secs;
}

#endif // DECODE_MIDI_ENABLE
