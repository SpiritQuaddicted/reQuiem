/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_mix.c -- portable code to mix sounds for snd_dma.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#ifdef _WIN32
#include "winquake.h"
#else
#define DWORD	unsigned long
#endif

#include "movie.h"

#define	PAINTBUFFER_SIZE	512
portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
int		snd_scaletable[32][256];
int 	*snd_p, snd_linear_count, snd_vol;
short	*snd_out;

#if id386
void Snd_WriteLinearBlastStereo16 (void);
#else
void Snd_WriteLinearBlastStereo16 (void)
{
	int	i, val;

	for (i=0 ; i<snd_linear_count ; i+=2)
	{
		val = (snd_p[i]*snd_vol)>>8;
		snd_out[i] = bound(-32768, val, 32767);
		val = (snd_p[i+1]*snd_vol)>>8;
		snd_out[i+1] = bound(-32768, val, 32767);
	}
}
#endif

void S_TransferStereo16 (int endtime)
{
	int			lpos, lpaintedtime;
	byte		*pbuf;
	unsigned	dwSize;

	snd_vol = volume.value * 256;

	snd_p = (int *)paintbuffer;
	lpaintedtime = paintedtime;

	pbuf = SNDDMA_LockBuffer (&dwSize);
	if (!pbuf)
		return;

	while (lpaintedtime < endtime)
	{
	// handle recirculating buffer issues
		lpos = lpaintedtime & ((shm->samples>>1)-1);

		snd_out = (short *)pbuf + (lpos<<1);

		snd_linear_count = (shm->samples>>1) - lpos;
		if (lpaintedtime + snd_linear_count > endtime)
			snd_linear_count = endtime - lpaintedtime;

		snd_linear_count <<= 1;

	// write a linear blast of samples
		Snd_WriteLinearBlastStereo16 ();

		snd_p += snd_linear_count;
		lpaintedtime += (snd_linear_count>>1);

		Movie_TransferStereo16 (snd_out, snd_linear_count);
	}

	SNDDMA_UnlockBuffer (pbuf, dwSize);
}

void S_TransferPaintBuffer (int endtime)
{
	int 		out_idx, count, out_mask, *p, step, val, snd_vol;
	byte		*pbuf;
	unsigned	dwSize;

	if (shm->samplebits == 16 && shm->channels == 2)
	{
		S_TransferStereo16 (endtime);
		return;
	}

	p = (int *) paintbuffer;
	count = (endtime - paintedtime) * shm->channels;
	out_mask = shm->samples - 1;
	out_idx = paintedtime * shm->channels & out_mask;
	step = 3 - shm->channels;
	snd_vol = volume.value*256;

	pbuf = SNDDMA_LockBuffer (&dwSize);
	if (!pbuf)
		return;

	if (shm->samplebits == 16)
	{
		short *out = (short *) pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = val;
			out_idx = (out_idx + 1) & out_mask;
		}
	}
	else if (shm->samplebits == 8)
	{
		byte *out = pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = (val>>8) + 128;
			out_idx = (out_idx + 1) & out_mask;
		}
	}

	SNDDMA_UnlockBuffer (pbuf, dwSize);
}


/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

void SND_PaintChannelFrom8 (channel_t *ch, sfxcache_t *sc, int endtime);
void SND_PaintChannelFrom16 (channel_t *ch, sfxcache_t *sc, int endtime);

void S_PaintChannels (int endtime)
{
	int		i, end, ltime, count;
	channel_t	*ch;
	sfxcache_t	*sc;

	while (paintedtime < endtime)
	{
	// if paintbuffer is smaller than DMA buffer
		end = endtime;
		if (endtime - paintedtime > PAINTBUFFER_SIZE)
			end = paintedtime + PAINTBUFFER_SIZE;

	// clear the paint buffer
		memset (paintbuffer, 0, (end - paintedtime) * sizeof(portable_samplepair_t));

	// paint in the channels.
		ch = channels;
		for (i = 0 ; i < total_channels ; i++, ch++)
		{
			if (!ch->sfx)
				continue;
			if (!ch->leftvol && !ch->rightvol)
				continue;
			if (!(sc = S_LoadSound(ch->sfx)))
				continue;

			ltime = paintedtime;

			while (ltime < end)
			{	// paint up to end
				if (ch->end < end)
					count = ch->end - ltime;
				else
					count = end - ltime;

				if (count > 0)
				{
					if (sc->width == 1)
						SND_PaintChannelFrom8(ch, sc, count);
					else
						SND_PaintChannelFrom16(ch, sc, count);

					ltime += count;
				}

			// if at end of loop, restart
				if (ltime >= ch->end)
				{
					if (sc->loopstart >= 0)
					{
						ch->pos = sc->loopstart;
						ch->end = ltime + sc->length - ch->pos;
					}
					else
					{	// channel just stopped
						ch->sfx = NULL;
						break;
					}
				}
			}

		}

	// transfer out according to DMA format
		S_TransferPaintBuffer (end);
		paintedtime = end;
	}
}

void SND_InitScaletable (void)
{
	int		i, j;

	for (i=0 ; i<32 ; i++)
		for (j=0 ; j<256 ; j++)
			snd_scaletable[i][j] = ((signed char)j) * i * 8;
}


#if !id386

void SND_PaintChannelFrom8 (channel_t *ch, sfxcache_t *sc, int count)
{
	int 		i, data, *lscale, *rscale;
	unsigned char	*sfx;

	if (ch->leftvol > 255)
		ch->leftvol = 255;
	if (ch->rightvol > 255)
		ch->rightvol = 255;

	lscale = snd_scaletable[ch->leftvol >> 3];
	rscale = snd_scaletable[ch->rightvol >> 3];
//	sfx = (signed char *)sc->data + ch->pos;
	sfx = (byte *)sc->data + ch->pos;

	for (i=0 ; i<count ; i++)
	{
		data = sfx[i];
		paintbuffer[i].left += lscale[data];
		paintbuffer[i].right += rscale[data];
	}

	ch->pos += count;
}

#endif	// !id386


void SND_PaintChannelFrom16 (channel_t *ch, sfxcache_t *sc, int count)
{
	int		data, left, right, leftvol, rightvol;
	signed	short	*sfx;
	int		i;

	leftvol = ch->leftvol;
	rightvol = ch->rightvol;
	sfx = (signed short *)sc->data + ch->pos;

	for (i=0 ; i<count ; i++)
	{
		data = sfx[i];
		left = (data * leftvol) >> 8;
		right = (data * rightvol) >> 8;
		paintbuffer[i].left += left;
		paintbuffer[i].right += right;
	}

	ch->pos += count;
}

#endif		//#ifndef RQM_SV_ONLY
