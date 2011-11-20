/*

 TiMidity -- Experimental MIDI to WAVE converter
 Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>
 
  Suddenly, you realize that this program is free software; you get
  an overwhelming urge to redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
    You should have received another copy of the GNU General Public
    License along with this program; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    I bet they'll be amazed.
	
mix.c */

#include <config.h>
#include <kernel/kernel.h>
#include <direct/medialib/MidiCfg.h>
#include <direct/medialib/MidiDecodec.h>

#ifdef CONFIG_DECODE_MIDI_ENABLE

/* Returns 1 if envelope runs out */
int recompute_envelope(PMIDI_DECODEC hMidi,int v)
{
	int stage;
	
	stage = hMidi->voice[v].envelope_stage;
	
	if (stage>5)
    {
		/* Envelope ran out. */
		int tmp;
		tmp = (hMidi->voice[v].status == VOICE_DIE); /* Already displayed as dead */
		hMidi->voice[v].status = VOICE_FREE;
		return 1;
    }
	
	if (hMidi->voice[v].sample->modes & MODES_ENVELOPE)
    {
		if (hMidi->voice[v].status==VOICE_ON || hMidi->voice[v].status==VOICE_SUSTAINED)
		{
			if (stage>2)
			{
				/* Freeze envelope until note turns off. Trumpets want this. */
				hMidi->voice[v].envelope_increment=0;
				return 0;
			}
		}
    }
	hMidi->voice[v].envelope_stage=stage+1;
	
	if (hMidi->voice[v].envelope_volume==hMidi->voice[v].sample->envelope_offset[stage])
		return recompute_envelope(hMidi,v);
	hMidi->voice[v].envelope_target=hMidi->voice[v].sample->envelope_offset[stage];
	hMidi->voice[v].envelope_increment = hMidi->voice[v].sample->envelope_rate[stage];
	if (hMidi->voice[v].envelope_target<hMidi->voice[v].envelope_volume)
		hMidi->voice[v].envelope_increment = -hMidi->voice[v].envelope_increment;
	return 0;
}

void apply_envelope_to_amp(PMIDI_DECODEC hMidi,int v)
{
	float lamp=hMidi->voice[v].left_amp, ramp;
	int32 la,ra;
	if (hMidi->voice[v].panned == PANNED_MYSTERY)
    {
		ramp=hMidi->voice[v].right_amp;
		if (hMidi->voice[v].tremolo_phase_increment)
		{
			lamp *= hMidi->voice[v].tremolo_volume;
			ramp *= hMidi->voice[v].tremolo_volume;
		}
		if (hMidi->voice[v].sample->modes & MODES_ENVELOPE)
		{
			lamp *= vol_table[hMidi->voice[v].envelope_volume>>23];
			ramp *= vol_table[hMidi->voice[v].envelope_volume>>23];
		}
		
		la = FSCALE(lamp,AMP_BITS);
		
		if (la>MAX_AMP_VALUE)
			la=MAX_AMP_VALUE;
		
		ra = FSCALE(ramp,AMP_BITS);
		if (ra>MAX_AMP_VALUE)
			ra=MAX_AMP_VALUE;
		
		
		hMidi->voice[v].left_mix=FINAL_VOLUME(la);
		hMidi->voice[v].right_mix=FINAL_VOLUME(ra);
    }
	else
    {
		if (hMidi->voice[v].tremolo_phase_increment)
			lamp *= hMidi->voice[v].tremolo_volume;
		if (hMidi->voice[v].sample->modes & MODES_ENVELOPE)
			lamp *= vol_table[hMidi->voice[v].envelope_volume>>23];
		
		la = FSCALE(lamp,AMP_BITS);
		
		if (la>MAX_AMP_VALUE)
			la=MAX_AMP_VALUE;
		
		hMidi->voice[v].left_mix=FINAL_VOLUME(la);
    }
}

static int update_envelope(PMIDI_DECODEC hMidi ,int v)
{
	hMidi->voice[v].envelope_volume += hMidi->voice[v].envelope_increment;
	/* Why is there no ^^ operator?? */
	if (((hMidi->voice[v].envelope_increment < 0) &&
		(hMidi->voice[v].envelope_volume <= hMidi->voice[v].envelope_target)) ||
		((hMidi->voice[v].envelope_increment > 0) &&
		(hMidi->voice[v].envelope_volume >= hMidi->voice[v].envelope_target)))
    {
		hMidi->voice[v].envelope_volume = hMidi->voice[v].envelope_target;
		if (recompute_envelope(hMidi,v))
			return 1;
    }
	return 0;
}

static void update_tremolo(PMIDI_DECODEC hMidi ,int v)
{
	int32 depth=hMidi->voice[v].sample->tremolo_depth<<7;
	
	if (hMidi->voice[v].tremolo_sweep)
    {
		/* Update sweep position */
		
		hMidi->voice[v].tremolo_sweep_position += hMidi->voice[v].tremolo_sweep;
		if (hMidi->voice[v].tremolo_sweep_position>=(1<<SWEEP_SHIFT))
			hMidi->voice[v].tremolo_sweep=0; /* Swept to max amplitude */
		else
		{
			/* Need to adjust depth */
			depth *= hMidi->voice[v].tremolo_sweep_position;
			depth >>= SWEEP_SHIFT;
		}
    }
	
	hMidi->voice[v].tremolo_phase += hMidi->voice[v].tremolo_phase_increment;
	
	/* if (voice[v].tremolo_phase >= (SINE_CYCLE_LENGTH<<RATE_SHIFT))
	hMidi->voice[v].tremolo_phase -= SINE_CYCLE_LENGTH<<RATE_SHIFT;  */
	
	hMidi->voice[v].tremolo_volume = 
		1.0 - FSCALENEG((sine(hMidi->voice[v].tremolo_phase >> RATE_SHIFT) + 1.0)
		* depth * TREMOLO_AMPLITUDE_TUNING,
		17);
	
		/* I'm not sure about the +1.0 there -- it makes tremoloed voices'
	volumes on average the lower the higher the tremolo amplitude. */
}

/* Returns 1 if the note died */
static int update_signal(PMIDI_DECODEC hMidi ,int v)
{
	if (hMidi->voice[v].envelope_increment && update_envelope(hMidi,v))
		return 1;
	
	if (hMidi->voice[v].tremolo_phase_increment)
		update_tremolo(hMidi,v);
	
	apply_envelope_to_amp(hMidi,v);
	return 0;
}

#define MIXATION(a)	*lp++ += (a)*s;

static void mix_mystery_signal(PMIDI_DECODEC hMidi ,sample_t *sp, int32 *lp, int v, int count)
{
	Voice *vp = hMidi->voice + v;
	final_volume_t 
		left=vp->left_mix, 
		right=vp->right_mix;
	int cc;
	sample_t s;
	
	cc = vp->control_counter;
	if (!cc)
    {
		cc = hMidi->control_ratio;
		if (update_signal(hMidi,v))
			return;	/* Envelope ran out */
		left = vp->left_mix;
		right = vp->right_mix;
    }
	
	while (count)
	{
		if (cc < count)
		{
			count -= cc;
			while (cc--)
			{
				s = *sp++;
				MIXATION(left);
				MIXATION(right);
			}
			cc = hMidi->control_ratio;
			if (update_signal(hMidi,v))
				return;	/* Envelope ran out */
			left = vp->left_mix;
			right = vp->right_mix;
		}
		else
		{
			vp->control_counter = cc - count;
			while (count--)
			{
				s = *sp++;
				MIXATION(left);
				MIXATION(right);
			}
			return;
		}
	}
}

static void mix_center_signal(PMIDI_DECODEC hMidi ,sample_t *sp, int32 *lp, int v, int count)
{
	Voice *vp = hMidi->voice + v;
	final_volume_t 
		left=vp->left_mix;
	int cc;
	sample_t s;
	
	cc = vp->control_counter;
	if (!cc)
    {
		cc = hMidi->control_ratio;
		if (update_signal(hMidi,v))
			return;	/* Envelope ran out */
		left = vp->left_mix;
    }
	
	while (count)
	{
		if (cc < count)
		{
			count -= cc;
			while (cc--)
			{
				s = *sp++;
				MIXATION(left);
				MIXATION(left);
			}
			cc = hMidi->control_ratio;
			if (update_signal(hMidi,v))
				return;	/* Envelope ran out */
			left = vp->left_mix;
		}
		else
		{
			vp->control_counter = cc - count;
			while (count--)
			{
				s = *sp++;
				MIXATION(left);
				MIXATION(left);
			}
			return;
		}
	}
}

static void mix_single_signal(PMIDI_DECODEC hMidi ,sample_t *sp, int32 *lp, int v, int count)
{
	Voice *vp = hMidi->voice + v;
	final_volume_t 
		left=vp->left_mix;
	int cc;
	sample_t s;
	
	cc = vp->control_counter;
	if (!cc)
    {
		cc = hMidi->control_ratio;
		if (update_signal(hMidi,v))
			return;	/* Envelope ran out */
		left = vp->left_mix;
    }
	
	while (count)
	{
		if (cc < count)
		{
			count -= cc;
			while (cc--)
			{
				s = *sp++;
				MIXATION(left);
				lp++;
			}
			cc = hMidi->control_ratio;
			if (update_signal(hMidi,v))
				return;	/* Envelope ran out */
			left = vp->left_mix;
		}
		else
		{
			vp->control_counter = cc - count;
			while (count--)
			{
				s = *sp++;
				MIXATION(left);
				lp++;
			}
			return;
		}
	}
}


static void mix_mystery(PMIDI_DECODEC hMidi,sample_t *sp, int32 *lp, int v, int count)
{
	final_volume_t 
		left=hMidi->voice[v].left_mix, 
		right=hMidi->voice[v].right_mix;
	sample_t s;
	
	while (count--)
    {
		s = *sp++;
		MIXATION(left);
		MIXATION(right);
    }
}

static void mix_center(PMIDI_DECODEC hMidi,sample_t *sp, int32 *lp, int v, int count)
{
	final_volume_t 
		left=hMidi->voice[v].left_mix;
	sample_t s;
	
	while (count--)
    {
		s = *sp++;
		MIXATION(left);
		MIXATION(left);
    }
}

static void mix_single(PMIDI_DECODEC hMidi ,sample_t *sp, int32 *lp, int v, int count)
{
	final_volume_t 
		left=hMidi->voice[v].left_mix;
	sample_t s;
	
	while (count--)
    {
		s = *sp++;
		MIXATION(left);
		lp++;
    }
}


/* Ramp a note out in c samples */
static void ramp_out(PMIDI_DECODEC hMidi ,sample_t *sp, int32 *lp, int v, int32 c)
{
	
	/* should be final_volume_t, but uint8 gives trouble. */
	int32 left, right, li, ri;
	
	sample_t s=0; /* silly warning about uninitialized s */
	
	left=hMidi->voice[v].left_mix;
	li=-(left/c);
	if (!li) li=-1;
	
	/* printf("Ramping out: left=%d, c=%d, li=%d\n", left, c, li); */
	if (hMidi->voice[v].panned==PANNED_MYSTERY)
	{
		right=hMidi->voice[v].right_mix;
		ri=-(right/c);
		while (c--)
		{
			left += li;
			if (left<0)
				left=0;
			right += ri;
			if (right<0)
				right=0;
			s=*sp++;
			MIXATION(left);
			MIXATION(right);
		}
	}
	else if (hMidi->voice[v].panned==PANNED_CENTER)
	{
		while (c--)
		{
			left += li;
			if (left<0)
				return;
			s=*sp++;	
			MIXATION(left);
			MIXATION(left);
		}
	}
	else if (hMidi->voice[v].panned==PANNED_LEFT)
	{
		while (c--)
		{
			left += li;
			if (left<0)
				return;
			s=*sp++;
			MIXATION(left);
			lp++;
		}
	}
	else if (hMidi->voice[v].panned==PANNED_RIGHT)
	{
		while (c--)
		{
			left += li;
			if (left<0)
				return;
			s=*sp++;
			lp++;
			MIXATION(left);
		}
	}
}


/**************** interface function ******************/

void mix_voice(PMIDI_DECODEC hMidi, int32 *buf, int v, int32 c)
{
	Voice *vp=hMidi->voice+v;
	sample_t *sp;
	if (vp->status==VOICE_DIE)
    {
		if (c>=MAX_DIE_TIME)
			c=MAX_DIE_TIME;
		sp=resample_voice(hMidi,v, &c);
		ramp_out(hMidi,sp, buf, v, c);
		vp->status=VOICE_FREE;
    }
	else
    {
		sp=resample_voice(hMidi,v, &c);
		if (vp->panned == PANNED_MYSTERY)
		{
			if (vp->envelope_increment || vp->tremolo_phase_increment)
				mix_mystery_signal(hMidi,sp, buf, v, c);
			else
				mix_mystery(hMidi,sp, buf, v, c);
		}
		else if (vp->panned == PANNED_CENTER)
		{
			if (vp->envelope_increment || vp->tremolo_phase_increment)
				mix_center_signal(hMidi,sp, buf, v, c);
			else
				mix_center(hMidi,sp, buf, v, c);
		}
		else
		{ 
		/* It's either full left or full right. In either case,
			every other sample is 0. Just get the offset right: */
			if (vp->panned == PANNED_RIGHT) buf++;
			
			if (vp->envelope_increment || vp->tremolo_phase_increment)
				mix_single_signal(hMidi,sp, buf, v, c);
			else 
				mix_single(hMidi,sp, buf, v, c);
		}
    }
}

#endif // DECODE_MIDI_ENABLE
