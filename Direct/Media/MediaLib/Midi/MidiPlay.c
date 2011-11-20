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
	
	 playmidi.c -- random stuff in need of rearrangement
	 
*/

#include <config.h>
#include <kernel/kernel.h>
#include <direct/medialib/MidiCfg.h>
#include <direct/medialib/MidiDecodec.h>

#ifdef CONFIG_DECODE_MIDI_ENABLE

static void s32tos16(int32 *lp, int32 c)
{
	int16 *sp=(int16 *)(lp);
	int32 l;
	while (c--)
    {
		l=(*lp++)>>(32-16-GUARD_BITS);
		if (l > 32767) 
			l=32767;
		else if (l<-32768) 
			l=-32768;
		*sp++ = (int16)(l);
    }
}

static void output_data(PMIDI_DECODEC hMidi,int32 *buf, int32 count)
{
	if(hMidi->midi_pcm_callback)
	{
		count*=2; /* Stereo samples */
		s32tos16(buf, count); /* Little-endian data */
		hMidi->midi_pcm_callback((HANDLE)hMidi,NULL, buf, count*2);
	}
}

static void ctl_current_time(int ct)
{
	int mins, secs;
	secs=ct/DEFAULT_RATE;
	mins=secs/60;
	secs-=mins*60;
	MidiMsg("\r%3d:%02d", mins, secs);
}


static void adjust_amplification(PMIDI_DECODEC hMidi)
{ 
	hMidi->master_volume = (double)(hMidi->amplification) / 100.0L;
}

static void reset_voices(PMIDI_DECODEC hMidi)
{
	int i;
	for (i=0; i<MAX_VOICES; i++)
		hMidi->voice[i].status=VOICE_FREE;
}

/* Process the Reset All Controllers event */
static void reset_controllers(PMIDI_DECODEC hMidi ,int c)
{
	hMidi->channel[c].volume=90; /* Some standard says, although the SCC docs say 0. */
	hMidi->channel[c].expression=127; /* SCC-1 does this. */
	hMidi->channel[c].sustain=0;
	hMidi->channel[c].pitchbend=0x2000;
	hMidi->channel[c].pitchfactor=0; /* to be computed */
}


static void reset_midi(PMIDI_DECODEC hMidi)
{
	int i;
	for (i=0; i<16; i++)
    {
		reset_controllers(hMidi,i);
		/* The rest of these are unaffected by the Reset All Controllers event */
		hMidi->channel[i].program=hMidi->default_program;
		hMidi->channel[i].panning=NO_PANNING;
		hMidi->channel[i].pitchsens=2;
		hMidi->channel[i].bank=0; /* tone bank or drum set */
    }
	reset_voices(hMidi);
}

static void select_sample(PMIDI_DECODEC hMidi ,int v, Instrument *ip)
{
	int32 f, cdiff, diff;
	int s,i;
	Sample *sp, *closest;
	
	s=ip->samples;
	sp=ip->sample;
	
	if (s==1)
    {
		hMidi->voice[v].sample=sp;
		return;
    }
	
	f=hMidi->voice[v].orig_frequency;
	for (i=0; i<s; i++)
    {
		if (sp->low_freq <= f && sp->high_freq >= f)
		{
			hMidi->voice[v].sample=sp;
			return;
		}
		sp++;
    }
	
	/* 
	No suitable sample found! We'll select the sample whose root
	frequency is closest to the one we want. (Actually we should
	probably convert the low, high, and root frequencies to MIDI note
	values and compare those.) */
	
	cdiff=0x7FFFFFFF;
	closest=sp=ip->sample;
	for(i=0; i<s; i++)
    {
		diff=sp->root_freq - f;
		if (diff<0) diff=-diff;
		if (diff<cdiff)
		{
			cdiff=diff;
			closest=sp;
		}
		sp++;
    }
	hMidi->voice[v].sample=closest;
	return;
}

static void recompute_freq(PMIDI_DECODEC hMidi ,int v)
{
	int 
		sign=(hMidi->voice[v].sample_increment < 0), /* for bidirectional loops */
		pb=hMidi->channel[hMidi->voice[v].channel].pitchbend;
	double a;
	
	if (!hMidi->voice[v].sample->sample_rate)
		return;
	
	if (hMidi->voice[v].vibrato_control_ratio)
    {
	/* This instrument has vibrato. Invalidate any precomputed
		sample_increments. */
		
		int i=VIBRATO_SAMPLE_INCREMENTS;
		while (i--)
			hMidi->voice[v].vibrato_sample_increment[i]=0;
    }
	
	if (pb==0x2000 || pb<0 || pb>0x3FFF)
		hMidi->voice[v].frequency=hMidi->voice[v].orig_frequency;
	else
    {
		pb-=0x2000;
		if (!(hMidi->channel[hMidi->voice[v].channel].pitchfactor))
		{
			/* Damn. Somebody bent the pitch. */
			int32 i=pb*hMidi->channel[hMidi->voice[v].channel].pitchsens;
			if (pb<0)
				i=-i;
			hMidi->channel[hMidi->voice[v].channel].pitchfactor=
				bend_fine[(i>>5) & 0xFF] * bend_coarse[i>>13];
		}
		if (pb>0)
			hMidi->voice[v].frequency=
			(int32)(hMidi->channel[hMidi->voice[v].channel].pitchfactor *
			(double)(hMidi->voice[v].orig_frequency));
		else
			hMidi->voice[v].frequency=
			(int32)((double)(hMidi->voice[v].orig_frequency) /
			hMidi->channel[hMidi->voice[v].channel].pitchfactor);
    }
	
	a = FSCALE(((double)(hMidi->voice[v].sample->sample_rate) *
		(double)(hMidi->voice[v].frequency)) /
		((double)(hMidi->voice[v].sample->root_freq) *
		(double)DEFAULT_RATE),
		FRACTION_BITS);
	
	if (sign) 
		a = -a; /* need to preserve the loop direction */
	
	hMidi->voice[v].sample_increment = (int32)(a);
}

static void recompute_amp(PMIDI_DECODEC hMidi ,int v)
{
	int32 tempamp;
	
	/* TODO: use fscale */
	
	tempamp= (hMidi->voice[v].velocity *
		hMidi->channel[hMidi->voice[v].channel].volume * 
		hMidi->channel[hMidi->voice[v].channel].expression); /* 21 bits */
	
		if (hMidi->voice[v].panning > 60 && hMidi->voice[v].panning < 68)
		{
			hMidi->voice[v].panned=PANNED_CENTER;
			
			hMidi->voice[v].left_amp=
				FSCALENEG((double)(tempamp) * hMidi->voice[v].sample->volume * hMidi->master_volume,
				21);
		}
		else if (hMidi->voice[v].panning<5)
		{
			hMidi->voice[v].panned = PANNED_LEFT;
			
			hMidi->voice[v].left_amp=
				FSCALENEG((double)(tempamp) * hMidi->voice[v].sample->volume * hMidi->master_volume,
				20);
		}
		else if (hMidi->voice[v].panning>123)
		{
			hMidi->voice[v].panned = PANNED_RIGHT;
			
			hMidi->voice[v].left_amp= /* left_amp will be used */
				FSCALENEG((double)(tempamp) * hMidi->voice[v].sample->volume * hMidi->master_volume,
				20);
		}
		else
		{
			hMidi->voice[v].panned = PANNED_MYSTERY;
			
			hMidi->voice[v].left_amp=
				FSCALENEG((double)(tempamp) * hMidi->voice[v].sample->volume * hMidi->master_volume,
				27);
			hMidi->voice[v].right_amp=hMidi->voice[v].left_amp * (hMidi->voice[v].panning);
			hMidi->voice[v].left_amp *= (double)(127-hMidi->voice[v].panning);
		}
}

static void start_note(PMIDI_DECODEC hMidi ,MidiEvent *e, int i)
{
	Instrument *ip;
	int j;
	
	if ( (hMidi->drumchannels & (1<<(e->channel))) )
    {
    	ip=hMidi->drumset[hMidi->channel[e->channel].bank]->instrument[e->a];
		if (!ip)
		{
			ip=hMidi->drumset[0]->instrument[e->a];
			if (!ip)
				return; /* No instrument? Then we can't play. */
		}
		if (ip->samples != 1)
		{
			MidiMsg("Strange: percussion instrument with %d samples!\n", ip->samples);
		}
		
		if (ip->sample->note_to_use) /* Do we have a fixed pitch? */
			hMidi->voice[i].orig_frequency=freq_table[(int)(ip->sample->note_to_use)];
		else
			hMidi->voice[i].orig_frequency=freq_table[e->a & 0x7F];
		
		/* drums are supposed to have only one sample */
		hMidi->voice[i].sample=ip->sample;
    }
	else
    {
		if (hMidi->channel[e->channel].program==SPECIAL_PROGRAM)
			ip=hMidi->default_instrument;
		else 
		{
			ip=hMidi->tonebank[hMidi->channel[e->channel].bank]->instrument[hMidi->channel[e->channel].program];
			if (!ip)
			{
				ip=hMidi->tonebank[0]->instrument[hMidi->channel[e->channel].program];
				if (!ip)
					return; /* No instrument? Then we can't play. */
			}
		}
		if (ip->sample->note_to_use) /* Fixed-pitch instrument? */
			hMidi->voice[i].orig_frequency=freq_table[(int)(ip->sample->note_to_use)];
		else
			hMidi->voice[i].orig_frequency=freq_table[e->a & 0x7F];
		select_sample(hMidi,i, ip);
    }
	
	hMidi->voice[i].status=VOICE_ON;
	hMidi->voice[i].channel=e->channel;
	hMidi->voice[i].note=e->a;
	hMidi->voice[i].velocity=e->b;
	hMidi->voice[i].sample_offset=0;
	hMidi->voice[i].sample_increment=0; /* make sure it isn't negative */
	
	hMidi->voice[i].tremolo_phase=0;
	hMidi->voice[i].tremolo_phase_increment=hMidi->voice[i].sample->tremolo_phase_increment;
	hMidi->voice[i].tremolo_sweep=hMidi->voice[i].sample->tremolo_sweep_increment;
	hMidi->voice[i].tremolo_sweep_position=0;
	
	hMidi->voice[i].vibrato_sweep=hMidi->voice[i].sample->vibrato_sweep_increment;
	hMidi->voice[i].vibrato_sweep_position=0;
	hMidi->voice[i].vibrato_control_ratio=hMidi->voice[i].sample->vibrato_control_ratio;
	hMidi->voice[i].vibrato_control_counter=hMidi->voice[i].vibrato_phase=0;
	for (j=0; j<VIBRATO_SAMPLE_INCREMENTS; j++)
		hMidi->voice[i].vibrato_sample_increment[j]=0;
	
	if (hMidi->channel[e->channel].panning != NO_PANNING)
		hMidi->voice[i].panning=hMidi->channel[e->channel].panning;
	else
		hMidi->voice[i].panning=hMidi->voice[i].sample->panning;
	
	recompute_freq(hMidi,i);
	recompute_amp(hMidi,i);
	if (hMidi->voice[i].sample->modes & MODES_ENVELOPE)
    {
		/* Ramp up from 0 */
		hMidi->voice[i].envelope_stage=0;
		hMidi->voice[i].envelope_volume=0;
		hMidi->voice[i].control_counter=0;
		recompute_envelope(hMidi,i);
		apply_envelope_to_amp(hMidi,i);
    }
	else
    {
		hMidi->voice[i].envelope_increment=0;
		apply_envelope_to_amp(hMidi,i);
    }
}

static void kill_note(PMIDI_DECODEC hMidi ,int i)
{
	hMidi->voice[i].status=VOICE_DIE;
}

/* Only one instance of a note can be playing on a single channel. */
static void note_on(PMIDI_DECODEC hMidi ,MidiEvent *e)
{
	int i=hMidi->voices, lowest=-1; 
	int32 lv=0x7FFFFFFF, v;
	
	while (i--)
    {
		if (hMidi->voice[i].status == VOICE_FREE)
			lowest=i; /* Can't get a lower volume than silence */
		else if (hMidi->voice[i].channel==e->channel && 
			(hMidi->voice[i].note==e->a || hMidi->channel[hMidi->voice[i].channel].mono))
			kill_note(hMidi,i);
    }
	
	if (lowest != -1)
    {
		/* Found a free voice. */
		start_note(hMidi,e,lowest);
		return;
    }
	
	/* Look for the decaying note with the lowest volume */
	i=hMidi->voices;
	while (i--)
    {
		if ((hMidi->voice[i].status!=VOICE_ON) &&
			(hMidi->voice[i].status!=VOICE_DIE))
		{
			v=hMidi->voice[i].left_mix;
			if ((hMidi->voice[i].panned==PANNED_MYSTERY) && (hMidi->voice[i].right_mix>v))
				v=hMidi->voice[i].right_mix;
			if (v<lv)
			{
				lv=v;
				lowest=i;
			}
		}
    }
	
	if (lowest != -1)
    {
	/* This can still cause a click, but if we had a free voice to
	spare for ramping down this note, we wouldn't need to kill it
	in the first place... Still, this needs to be fixed. Perhaps
		we could use a reserve of voices to play dying notes only. */
		
		hMidi->cut_notes++;
		hMidi->voice[lowest].status=VOICE_FREE;
		start_note(hMidi,e,lowest);
    }
	else
		hMidi->lost_notes++;
}

static void finish_note(PMIDI_DECODEC hMidi ,int i)
{
	if (hMidi->voice[i].sample->modes & MODES_ENVELOPE)
    {
		/* We need to get the envelope out of Sustain stage */
		hMidi->voice[i].envelope_stage=3;
		hMidi->voice[i].status=VOICE_OFF;
		recompute_envelope(hMidi,i);
		apply_envelope_to_amp(hMidi,i);
    }
	else
    {
	/* Set status to OFF so resample_voice() will let this voice out
	of its loop, if any. In any case, this voice dies when it
		hits the end of its data (ofs>=data_length). */
		hMidi->voice[i].status=VOICE_OFF;
    }
}

static void note_off(PMIDI_DECODEC hMidi ,MidiEvent *e)
{
	int i=hMidi->voices;
	while (i--)
		if (hMidi->voice[i].status==VOICE_ON &&
			hMidi->voice[i].channel==e->channel &&
			hMidi->voice[i].note==e->a)
		{
			if (hMidi->channel[e->channel].sustain)
			{
				hMidi->voice[i].status=VOICE_SUSTAINED;
			}
			else
				finish_note(hMidi,i);
			return;
		}
}

/* Process the All Notes Off event */
static void all_notes_off(PMIDI_DECODEC hMidi ,int c)
{
	int i=hMidi->voices;
	MidiMsg("All notes off on channel %d\n", c);
	while (i--)
	{
		if (hMidi->voice[i].status==VOICE_ON &&
			hMidi->voice[i].channel==c)
		{
			if (hMidi->channel[c].sustain) 
			{
				hMidi->voice[i].status=VOICE_SUSTAINED;
			}
			else
				finish_note(hMidi,i);
		}
	}
}

/* Process the All Sounds Off event */
static void all_sounds_off(PMIDI_DECODEC hMidi ,int c)
{
	int i=hMidi->voices;
	while (i--)
		if (hMidi->voice[i].channel==c && 
			hMidi->voice[i].status != VOICE_FREE &&
			hMidi->voice[i].status != VOICE_DIE)
		{
			kill_note(hMidi,i);
		}
}

static void adjust_pressure(PMIDI_DECODEC hMidi ,MidiEvent *e)
{
	int i=hMidi->voices;
	while (i--)
		if (hMidi->voice[i].status==VOICE_ON &&
			hMidi->voice[i].channel==e->channel &&
			hMidi->voice[i].note==e->a)
		{
			hMidi->voice[i].velocity=e->b;
			recompute_amp(hMidi,i);
			apply_envelope_to_amp(hMidi,i);
			return;
		}
}

static void adjust_panning(PMIDI_DECODEC hMidi ,int c)
{
	int i=hMidi->voices;
	while (i--)
		if ((hMidi->voice[i].channel==c) &&
			(hMidi->voice[i].status==VOICE_ON || hMidi->voice[i].status==VOICE_SUSTAINED))
		{
			hMidi->voice[i].panning=hMidi->channel[c].panning;
			recompute_amp(hMidi,i);
			apply_envelope_to_amp(hMidi,i);
		}
}

static void drop_sustain(PMIDI_DECODEC hMidi ,int c)
{
	int i=hMidi->voices;
	while (i--)
		if (hMidi->voice[i].status==VOICE_SUSTAINED && hMidi->voice[i].channel==c)
			finish_note(hMidi,i);
}

static void adjust_pitchbend(PMIDI_DECODEC hMidi ,int c)
{
	int i=hMidi->voices;
	while (i--)
		if (hMidi->voice[i].status!=VOICE_FREE && hMidi->voice[i].channel==c)
		{
			recompute_freq(hMidi,i);
		}
}

static void adjust_volume(PMIDI_DECODEC hMidi ,int c)
{
	int i=hMidi->voices;
	while (i--)
		if (hMidi->voice[i].channel==c &&
			(hMidi->voice[i].status==VOICE_ON || hMidi->voice[i].status==VOICE_SUSTAINED))
		{
			recompute_amp(hMidi,i);
			apply_envelope_to_amp(hMidi,i);
		}
}

static void seek_forward(PMIDI_DECODEC hMidi ,int32 until_time)
{
	reset_voices(hMidi);
	while (hMidi->current_event->time < until_time)
    {
		switch(hMidi->current_event->type)
		{
			/* All notes stay off. Just handle the parameter changes. */
			
		case ME_PITCH_SENS:
			hMidi->channel[hMidi->current_event->channel].pitchsens=
				hMidi->current_event->a;
			hMidi->channel[hMidi->current_event->channel].pitchfactor=0;
			break;
			
		case ME_PITCHWHEEL:
			hMidi->channel[hMidi->current_event->channel].pitchbend=
				hMidi->current_event->a + hMidi->current_event->b * 128;
			hMidi->channel[hMidi->current_event->channel].pitchfactor=0;
			break;
			
		case ME_MAINVOLUME:
			hMidi->channel[hMidi->current_event->channel].volume=hMidi->current_event->a;
			break;
			
		case ME_PAN:
			hMidi->channel[hMidi->current_event->channel].panning=hMidi->current_event->a;
			break;
			
		case ME_EXPRESSION:
			hMidi->channel[hMidi->current_event->channel].expression=hMidi->current_event->a;
			break;
			
		case ME_PROGRAM:
			if ( (hMidi->drumchannels & (1<<(hMidi->current_event->channel))) )
				/* Change drum set */
				hMidi->channel[hMidi->current_event->channel].bank=hMidi->current_event->a;
			else
				hMidi->channel[hMidi->current_event->channel].program=hMidi->current_event->a;
			break;
			
		case ME_SUSTAIN:
			hMidi->channel[hMidi->current_event->channel].sustain=hMidi->current_event->a;
			break;
			
		case ME_RESET_CONTROLLERS:
			reset_controllers(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_TONE_BANK:
			hMidi->channel[hMidi->current_event->channel].bank=hMidi->current_event->a;
			break;
			
		case ME_EOT:
			hMidi->current_sample=hMidi->current_event->time;
			return;
		}
		hMidi->current_event++;
    }
	/*current_sample=hMidi->current_event->time;*/
	if (hMidi->current_event != hMidi->event_list)
		hMidi->current_event--;
	hMidi->current_sample=until_time;
}

static void skip_to(PMIDI_DECODEC hMidi ,int32 until_time)
{
	if (hMidi->current_sample > until_time)
		hMidi->current_sample=0;
	
	reset_midi(hMidi);
	hMidi->buffered_count=0;
	hMidi->buffer_pointer=hMidi->common_buffer;
	hMidi->current_event=hMidi->event_list;
	
	if (until_time)
		seek_forward(hMidi,until_time);
}


static void do_compute_data(PMIDI_DECODEC hMidi ,int32 count)
{
	int i;
	kmemset(hMidi->buffer_pointer, 0, (count * 8));
	for (i=0; i<hMidi->voices; i++)
    {
		if(hMidi->voice[i].status != VOICE_FREE)
			mix_voice(hMidi,hMidi->buffer_pointer, i, count);
    }
	hMidi->current_sample += count;
}

/* count=0 means flush remaining buffered data to output device, then
flush the device itself */
static int compute_data(PMIDI_DECODEC hMidi ,int32 count)
{
	int32 midi_size;

	hMidi->midi_count_flag = 0;
	if (!count)
    {
		if (hMidi->buffered_count)
			output_data(hMidi,hMidi->common_buffer, hMidi->buffered_count);
		hMidi->buffer_pointer=hMidi->common_buffer;
		hMidi->buffered_count=0;
		return 0;
    }

	midi_size = 0;
//	kprintf("compute_data: count = %d\n",count);
	while ((count+hMidi->buffered_count) >= AUDIO_BUFFER_SIZE)
    {
		do_compute_data(hMidi,AUDIO_BUFFER_SIZE-hMidi->buffered_count);
		count -= AUDIO_BUFFER_SIZE-hMidi->buffered_count;
		midi_size += AUDIO_BUFFER_SIZE-hMidi->buffered_count;

		output_data(hMidi,hMidi->common_buffer, AUDIO_BUFFER_SIZE);
		hMidi->buffer_pointer=hMidi->common_buffer;
		hMidi->buffered_count=0;
		
		ctl_current_time(hMidi->current_sample);

		if( midi_size >= 4096 )
		{
			hMidi->midi_count = count;
			hMidi->midi_count_flag = 1;
			return 1;
		}
    }

	if (count>0)
    {
		do_compute_data(hMidi,count);
		hMidi->buffered_count += count;
		hMidi->buffer_pointer += count*2;
    }
	return 0;
}

int play_midi(PMIDI_DECODEC hMidi ,MidiEvent *eventlist, int32 events, int32 samples)
{
	int rc;
	
	adjust_amplification(hMidi);
	
	if (!hMidi->control_ratio)
	{
		hMidi->control_ratio = DEFAULT_RATE / CONTROLS_PER_SECOND;
		if(hMidi->control_ratio<1)
			hMidi->control_ratio=1;
		else if (hMidi->control_ratio > MAX_CONTROL_RATIO)
			hMidi->control_ratio=MAX_CONTROL_RATIO;
	}

	hMidi->sample_count=samples;
	hMidi->event_list=eventlist;
	hMidi->lost_notes=hMidi->cut_notes=0;
	
	skip_to(hMidi,0);
	hMidi->midiquit = 0;
	for (;;)
    {
		if(hMidi->midiquit == 1)
			return 0;
		/* Handle all events that should happen at this time */
		while (hMidi->current_event->time <= hMidi->current_sample)
		{
			switch(hMidi->current_event->type)
			{
				
				/* Effects affecting a single note */
				
			case ME_NOTEON:
				if (!(hMidi->current_event->b)) /* Velocity 0? */
					note_off(hMidi,hMidi->current_event);
				else
					note_on(hMidi,hMidi->current_event);
				break;
				
			case ME_NOTEOFF:
				note_off(hMidi,hMidi->current_event);
				break;
				
			case ME_KEYPRESSURE:
				adjust_pressure(hMidi,hMidi->current_event);
				break;
				
				/* Effects affecting a single channel */
				
			case ME_PITCH_SENS:
				hMidi->channel[hMidi->current_event->channel].pitchsens=
					hMidi->current_event->a;
				hMidi->channel[hMidi->current_event->channel].pitchfactor=0;
				break;
				
			case ME_PITCHWHEEL:
				hMidi->channel[hMidi->current_event->channel].pitchbend=
					hMidi->current_event->a + hMidi->current_event->b * 128;
				hMidi->channel[hMidi->current_event->channel].pitchfactor=0;
				/* Adjust pitch for notes already playing */
				adjust_pitchbend(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_MAINVOLUME:
				hMidi->channel[hMidi->current_event->channel].volume=hMidi->current_event->a;
				adjust_volume(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_PAN:
				hMidi->channel[hMidi->current_event->channel].panning=hMidi->current_event->a;
				if (hMidi->adjust_panning_immediately)
					adjust_panning(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_EXPRESSION:
				hMidi->channel[hMidi->current_event->channel].expression=hMidi->current_event->a;
				adjust_volume(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_PROGRAM:
				if ( (hMidi->drumchannels & (1<<(hMidi->current_event->channel))) )
				{
					/* Change drum set */
					hMidi->channel[hMidi->current_event->channel].bank=hMidi->current_event->a;
				}
				else
				{
					hMidi->channel[hMidi->current_event->channel].program=hMidi->current_event->a;
				}
				break;
				
			case ME_SUSTAIN:
				hMidi->channel[hMidi->current_event->channel].sustain=hMidi->current_event->a;
				if (!hMidi->current_event->a)
					drop_sustain(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_RESET_CONTROLLERS:
				reset_controllers(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_ALL_NOTES_OFF:
				all_notes_off(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_ALL_SOUNDS_OFF:
				all_sounds_off(hMidi,hMidi->current_event->channel);
				break;
				
			case ME_TONE_BANK:
				hMidi->channel[hMidi->current_event->channel].bank=hMidi->current_event->a;
				break;
				
			case ME_EOT:
				/* Give the last notes a couple of seconds to decay  */
				compute_data(hMidi,DEFAULT_RATE * 2);
				compute_data(hMidi,0); /* flush buffer to device */
				MidiMsg("Playing time: ~%d seconds\n", hMidi->current_sample/DEFAULT_RATE+2);
				MidiMsg("Notes cut: %d\n", hMidi->cut_notes);
				MidiMsg("Notes lost totally: %d\n", hMidi->lost_notes);
				return 14;
			}
			hMidi->current_event++;
		}
		
		rc = compute_data(hMidi,hMidi->current_event->time - hMidi->current_sample);
		/*hMidi->current_sample=hMidi->current_event->time;*/
		if(rc!=0)
			return rc;
	}
}

void init_midi_play(PMIDI_DECODEC hMidi,MidiEvent *eventlist, int32 events, int32 samples)
{
	adjust_amplification(hMidi);
	
	if (!hMidi->control_ratio)
	{
		hMidi->control_ratio = DEFAULT_RATE / CONTROLS_PER_SECOND;
		if(hMidi->control_ratio<1)
			hMidi->control_ratio=1;
		else if (hMidi->control_ratio > MAX_CONTROL_RATIO)
			hMidi->control_ratio=MAX_CONTROL_RATIO;
	}

	hMidi->sample_count=samples;
	hMidi->event_list=eventlist;
	hMidi->lost_notes=hMidi->cut_notes=0;
	hMidi->midi_count      = 0;
	hMidi->midi_count_flag = 0;
	
	skip_to(hMidi,0);
	hMidi->midiquit = 0;
}


int get_midi_pcm(PMIDI_DECODEC hMidi)
{
	int rc;
	
	if(hMidi->midiquit == 1)
		return -1;

	if( hMidi->midi_count_flag )
	{
//		kprintf("hMidi->midi_count_flag = 1,hMidi->midiquit = %d, count = %d\n",hMidi->midiquit,hMidi->midi_count);
		compute_data(hMidi,hMidi->midi_count);
		if( hMidi->midiquit == 2 && hMidi->midi_count_flag )
		{
			compute_data(hMidi,0);
			MidiMsg("Playing time: ~%d seconds\n", hMidi->current_sample/DEFAULT_RATE+2);
			MidiMsg("Notes cut: %d\n", hMidi->cut_notes);
			MidiMsg("Notes lost totally: %d\n", hMidi->lost_notes);
			if( hMidi->midi_count_flag )
				return 0;
			else
				return -1;
		}
		return 0;
	}
	
	/* Handle all events that should happen at this time */
	while (hMidi->current_event->time <= hMidi->current_sample)
	{
		switch(hMidi->current_event->type)
		{
			
			/* Effects affecting a single note */
			
		case ME_NOTEON:
			if (!(hMidi->current_event->b)) /* Velocity 0? */
				note_off(hMidi,hMidi->current_event);
			else
				note_on(hMidi,hMidi->current_event);
			break;
			
		case ME_NOTEOFF:
			note_off(hMidi,hMidi->current_event);
			break;
			
		case ME_KEYPRESSURE:
			adjust_pressure(hMidi,hMidi->current_event);
			break;
			
			/* Effects affecting a single channel */
			
		case ME_PITCH_SENS:
			hMidi->channel[hMidi->current_event->channel].pitchsens=
				hMidi->current_event->a;
			hMidi->channel[hMidi->current_event->channel].pitchfactor=0;
			break;
			
		case ME_PITCHWHEEL:
			hMidi->channel[hMidi->current_event->channel].pitchbend=
				hMidi->current_event->a + hMidi->current_event->b * 128;
			hMidi->channel[hMidi->current_event->channel].pitchfactor=0;
			/* Adjust pitch for notes already playing */
			adjust_pitchbend(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_MAINVOLUME:
			hMidi->channel[hMidi->current_event->channel].volume=hMidi->current_event->a;
			adjust_volume(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_PAN:
			hMidi->channel[hMidi->current_event->channel].panning=hMidi->current_event->a;
			if (hMidi->adjust_panning_immediately)
				adjust_panning(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_EXPRESSION:
			hMidi->channel[hMidi->current_event->channel].expression=hMidi->current_event->a;
			adjust_volume(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_PROGRAM:
			if ( (hMidi->drumchannels & (1<<(hMidi->current_event->channel))) )
			{
				/* Change drum set */
				hMidi->channel[hMidi->current_event->channel].bank=hMidi->current_event->a;
			}
			else
			{
				hMidi->channel[hMidi->current_event->channel].program=hMidi->current_event->a;
			}
			break;
			
		case ME_SUSTAIN:
			hMidi->channel[hMidi->current_event->channel].sustain=hMidi->current_event->a;
			if (!hMidi->current_event->a)
				drop_sustain(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_RESET_CONTROLLERS:
			reset_controllers(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_ALL_NOTES_OFF:
			all_notes_off(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_ALL_SOUNDS_OFF:
			all_sounds_off(hMidi,hMidi->current_event->channel);
			break;
			
		case ME_TONE_BANK:
			hMidi->channel[hMidi->current_event->channel].bank=hMidi->current_event->a;
			break;
			
		case ME_EOT:
			/* Give the last notes a couple of seconds to decay  */
			hMidi->midiquit = 2;
			if( compute_data(hMidi,DEFAULT_RATE * 2) == 1 )
				return 0;
			compute_data(hMidi,0); /* flush buffer to device */
			MidiMsg("Playing time: ~%d seconds\n", hMidi->current_sample/DEFAULT_RATE+2);
			MidiMsg("Notes cut: %d\n", hMidi->cut_notes);
			MidiMsg("Notes lost totally: %d\n", hMidi->lost_notes);
			return -1;
		}
		hMidi->current_event++;
	}
	
	rc = compute_data(hMidi,hMidi->current_event->time - hMidi->current_sample);
	return 0;
}

static int get_midi_event(PMIDI_DECODEC hMidi ,MidiEvent *event)
{
	if(hMidi->midi_queue_head == hMidi->midi_queue_tail)
		return -1;
	*event = *hMidi->midi_queue_head++;
	if(hMidi->midi_queue_head == &hMidi->midi_queue[MAX_MIDI_QUEUE_EVENTS])
		hMidi->midi_queue_head = hMidi->midi_queue;
	return 0;
}

static void init_midi_event(PMIDI_DECODEC hMidi)
{
	hMidi->midi_queue_head = hMidi->midi_queue;
	hMidi->midi_queue_tail = hMidi->midi_queue;
}

int send_midi_event(PMIDI_DECODEC hMidi ,MidiEvent *event)
{
	MidiEvent *pnew;

	pnew = hMidi->midi_queue_tail+1;
	if(pnew == &hMidi->midi_queue[MAX_MIDI_QUEUE_EVENTS])
		pnew = hMidi->midi_queue;
	if(pnew == hMidi->midi_queue_head)
	{
		return -1;
	}
	*hMidi->midi_queue_tail = *event;
	hMidi->midi_queue_tail->time = hMidi->current_sample;
	hMidi->midi_queue_tail = pnew;
	return 0;
}


int run_midi(PMIDI_DECODEC hMidi ,char *program)
{
	MidiEvent event;
	int delta_time;
	
	adjust_amplification(hMidi);
	
	if (!hMidi->control_ratio)
	{
		hMidi->control_ratio = DEFAULT_RATE / CONTROLS_PER_SECOND;
		if(hMidi->control_ratio<1)
			hMidi->control_ratio=1;
		else if (hMidi->control_ratio > MAX_CONTROL_RATIO)
			hMidi->control_ratio=MAX_CONTROL_RATIO;
	}

	hMidi->lost_notes=hMidi->cut_notes=0;
	hMidi->midi_run_time = 0;
	delta_time = DEFAULT_RATE / 2 / 120;
	
	reset_midi(hMidi);
	init_midi_event(hMidi);
	hMidi->buffered_count=0;
	hMidi->buffer_pointer=hMidi->common_buffer;

	if(program)
	{
		int i;
		for (i=0; i<16; i++)
		{
			hMidi->channel[i].program=program[i];
		}
	}


	hMidi->midiquit = 0;
	event.time = 0;
	event.type = ME_LYRIC;
	for(;;)
    {
		if(hMidi->midiquit == 1)
			return 0;
		/* Handle all events that should happen at this time */
		while(event.time <= hMidi->current_sample)
		{
			if(get_midi_event(hMidi,&event) < 0)
			{
				event.time = hMidi->current_sample + delta_time;	//delta_time;
				break;
			}
			switch(event.type)
			{
			case ME_LYRIC:
				break;
				
				/* Effects affecting a single note */
			case ME_NOTEON:
				if (!event.b) /* Velocity 0? */
					note_off(hMidi,&event);
				else
					note_on(hMidi,&event);
				break;
				
			case ME_NOTEOFF:
				note_off(hMidi,&event);
				break;
				
			case ME_KEYPRESSURE:
				adjust_pressure(hMidi,&event);
				break;
				
				/* Effects affecting a single channel */
				
			case ME_PITCH_SENS:
				hMidi->channel[event.channel].pitchsens=event.a;
				hMidi->channel[event.channel].pitchfactor=0;
				break;
				
			case ME_PITCHWHEEL:
				hMidi->channel[event.channel].pitchbend=event.a + event.b * 128;
				hMidi->channel[event.channel].pitchfactor=0;
				/* Adjust pitch for notes already playing */
				adjust_pitchbend(hMidi,event.channel);
				break;
				
			case ME_MAINVOLUME:
				hMidi->channel[event.channel].volume=event.a;
				adjust_volume(hMidi,event.channel);
				break;
				
			case ME_PAN:
				hMidi->channel[event.channel].panning=event.a;
				if (hMidi->adjust_panning_immediately)
					adjust_panning(hMidi,event.channel);
				break;
				
			case ME_EXPRESSION:
				hMidi->channel[event.channel].expression=event.a;
				adjust_volume(hMidi,event.channel);
				break;
				
			case ME_PROGRAM:
					if ( (hMidi->drumchannels & (1<<(event.channel))) )
				{
					hMidi->channel[event.channel].bank=event.a;
				}
				else
				{
					hMidi->channel[event.channel].program=event.a;
				}
				break;
				
			case ME_SUSTAIN:
				hMidi->channel[event.channel].sustain=event.a;
				if (!event.a)
					drop_sustain(hMidi,event.channel);
				break;
				
			case ME_RESET_CONTROLLERS:
				reset_controllers(hMidi,event.channel);
				break;
				
			case ME_ALL_NOTES_OFF:
				all_notes_off(hMidi,event.channel);
				break;
				
			case ME_ALL_SOUNDS_OFF:
				all_sounds_off(hMidi,event.channel);
				break;
				
			case ME_TONE_BANK:
				hMidi->channel[event.channel].bank=event.a;
				break;
				
			case ME_EOT:
				/* Give the last notes a couple of seconds to decay  */
				compute_data(hMidi,DEFAULT_RATE * 2);
				compute_data(hMidi,0); /* flush buffer to device */
				MidiMsg("Playing time: ~%d seconds\n", hMidi->current_sample/DEFAULT_RATE+2);
				MidiMsg("Notes cut: %d\n", hMidi->cut_notes);
				MidiMsg("Notes lost totally: %d\n", hMidi->lost_notes);
				return 14;
			}
		}
		compute_data(hMidi,event.time - hMidi->current_sample);
	}
}

void midi_quit(PMIDI_DECODEC hMidi )
{
	hMidi->midiquit = 1;
}

int midi_msg(char *str, ...)
{
	return 0;
}

HANDLE MidiDrvCrteate()
{
	PMIDI_DECODEC pMidi;
	pMidi = (PMIDI_DECODEC)kmalloc(sizeof(MIDI_DECODEC));
	kmemset(pMidi,0,sizeof(MIDI_DECODEC));

	pMidi->default_instrument = 0;
	pMidi->default_program    = DEFAULT_PROGRAM;
	pMidi->quietchannels      = 0;
	pMidi->voices             = DEFAULT_VOICES;
	pMidi->control_ratio      = 0;
	pMidi->amplification      = DEFAULT_AMPLIFICATION;
	pMidi->midi_pcm_callback  = NULL;
	pMidi->midi_callback      = NULL;
	pMidi->drumchannels       = DEFAULT_DRUMCHANNELS;
	pMidi->adjust_panning_immediately = 0;

	return (HANDLE)pMidi;
}

void MidiDrvDestory(HANDLE hMidi)
{
	if( hMidi )
		kfree(hMidi);
}
#endif // DECODE_MIDI_ENABLE
