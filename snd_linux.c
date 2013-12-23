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
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <errno.h>
#include "quakedef.h"

#ifndef SNDCTL_DSP_COOKEDMODE
#  define SNDCTL_DSP_COOKEDMODE  _SIOW ('P', 30, int)
#endif

static qboolean snd_useoss = true;

int	audio_fd = -1;
qboolean snd_inited;

static const int tryrates[] = { 11025, 22050, 22051, 44100, 48000, 8000 };
static char snd_dev[64] = "/dev/dsp";

#ifdef RQM_ALSA_TEST
#include <alsa/asoundlib.h>
static snd_pcm_t* pcm_handle = NULL;
#endif

qboolean SNDDMA_InitOSS_Direct (char **errstr)
{
	int 	caps, tmp, rc;
	struct	audio_buf_info	info;

	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &caps) == -1)
	{
		perror (snd_dev);
		*errstr = "Cannot determine capabilities of soundcard\n";
		return false;
	}

	if (!(caps & DSP_CAP_TRIGGER) || !(caps & DSP_CAP_MMAP))
	{
		*errstr = "Soundcard doesn't support necessary features\n";
		return false;
	}
	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info) == -1)
	{
		perror ("GETOSPACE");
		*errstr = "Um, can't do GETOSPACE?\n";
		return false;
	}

	shm->samples = info.fragstotal * info.fragsize / (shm->samplebits / 8);
//	shm->submission_chunk = 1;

// memory map the dma buffer
	shm->buffer = (unsigned char *)mmap (NULL, info.fragstotal * info.fragsize, PROT_WRITE, MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (!shm->buffer || shm->buffer == (unsigned char *)MAP_FAILED)
	{
		perror (snd_dev);
		*errstr = va ("Could not mmap %s (error %d)\n", snd_dev, errno);
		return false;
	}

// toggle the trigger & start her up
	tmp = 0;
	rc  = ioctl (audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror (snd_dev);
		*errstr = "Could not toggle.\n";
		return false;
	}

	tmp = PCM_ENABLE_OUTPUT;
	rc = ioctl (audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror (snd_dev);
		*errstr = "Could not toggle.\n";
		return false;
	}

	return true;
}

#define NB_FRAGMENTS 4
static int old_osstime = 0;
static unsigned int osssoundtime;

qboolean SNDDMA_InitOSS_Indirect (char **errstr)
{
	int flags, ioctl_param;
	unsigned int fragmentsize, i, val;

	// Use non-blocking IOs if possible
	flags = fcntl(audio_fd, F_GETFL);
	if (flags != -1)
	{
		if (fcntl(audio_fd, F_SETFL, flags | O_NONBLOCK) == -1)
			Con_Print ("SndSys_Init : fcntl(F_SETFL, O_NONBLOCK) failed!\n");
	}
	else
		Con_Print ("SndSys_Init: fcntl(F_GETFL) failed!\n");

	// Set the fragment size (up to "NB_FRAGMENTS" fragments of "fragmentsize" bytes)
	fragmentsize = shm->speed * shm->channels * (shm->samplebits/8) / 10;
	fragmentsize = (unsigned int)ceilf((float)fragmentsize / (float)NB_FRAGMENTS);
	//fragmentsize = CeilPowerOf2(fragmentsize);
	//ioctl_param = (NB_FRAGMENTS << 16) | log2i(fragmentsize);
	for (i = 0, val = 1; val < fragmentsize; val <<= 1, i++);
	fragmentsize = val;
	ioctl_param = (NB_FRAGMENTS << 16) | i;

	if (ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &ioctl_param) == -1)
	{
		*errstr = "SndSys_Init: could not set the fragment size\n";
		return false;
	}

	Con_Printf ("SndSys_Init: using %u fragments of %u bytes\n",
				ioctl_param >> 16, 1 << (ioctl_param & 0xFFFF));

	old_osstime = 0;
	osssoundtime = 0;
//	shm->buffer = Snd_CreateRingBuffer(requested, 0, NULL);
	shm->samples = NB_FRAGMENTS * fragmentsize / (shm->samplebits / 8);
	return false;
}

qboolean SNDDMA_InitOSS (void)
{
	int		rc, fmt, tmp, i;
//	char	*s;
//	int 	caps;
//	struct	audio_buf_info	info;
	char	*errstr = NULL;

	snd_inited = false;

	if ((i = COM_CheckParm("-snddev")) && (i + 1 < com_argc))
	{
		Q_strcpy (snd_dev, com_argv[i + 1], sizeof(snd_dev));
	}
// open snd_dev, confirm capability to mmap, and get size of dma buffer

	audio_fd = open (snd_dev, O_RDWR);		// darkplaces uses O_WRONLY
	if (audio_fd < 0)
	{
		perror (snd_dev);
		errstr = va ("Could not open %s\n", snd_dev);
		goto SNDFAIL;
	}

// disable cooked mode (OSS 4.0+, but not harmful on previous)
	tmp = 0;
	rc = ioctl(audio_fd, SNDCTL_DSP_COOKEDMODE, &tmp);

	rc = ioctl (audio_fd, SNDCTL_DSP_RESET, 0);
	if (rc < 0)
	{
		perror (snd_dev);
		errstr = va ("Could not reset %s\n", snd_dev);
		goto SNDFAIL;
	}

/*	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &caps) == -1)
	{
		perror (snd_dev);
		errstr = "Cannot determine capabilities of soundcard\n";
		goto SNDFAIL;
	}

	if (!(caps & DSP_CAP_TRIGGER) || !(caps & DSP_CAP_MMAP))
	{
		errstr = "Soundcard doesn't support necessary features\n";
		goto SNDFAIL;
	}
*/
/*	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info) == -1)
	{
		perror ("GETOSPACE");
		errstr = "Um, can't do GETOSPACE?\n";
		goto SNDFAIL;
	}
 */
/*
	shm = &sn;
	shm->splitbuffer = 0;
*/
// set sample bits & speed
/*	if ((s = getenv("QUAKE_SOUND_SAMPLEBITS")))
		shm->samplebits = Q_atoi(s);
	else if ((i = COM_CheckParm("-sndbits")) && (i + 1 < com_argc))
		shm->samplebits = Q_atoi(com_argv[i+1]);
	else
		shm->samplebits = 0;
*/
	if (shm->samplebits != 16 && shm->samplebits != 8)
	{
		ioctl (audio_fd, SNDCTL_DSP_GETFMTS, &fmt);
		if (fmt & AFMT_S16_LE)
			shm->samplebits = 16;
		else if (fmt & AFMT_U8)
			shm->samplebits = 8;
		else
		{
			errstr = "Soundcard must support 8- or 16-bit sound!\n";
			goto SNDFAIL;
		}
	}

	fmt = (shm->samplebits == 16 ? AFMT_S16_LE : AFMT_U8);
	rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &fmt);
	if (rc < 0)
	{
		perror (snd_dev);
		errstr = va ("Soundcard does not support %d-bit data.\n", shm->samplebits);
		goto SNDFAIL;
	}
/*
	if ((s = getenv("QUAKE_SOUND_SPEED")))
		shm->speed = Q_atoi(s);
	else if ((i = COM_CheckParm("-sndspeed")) && (i + 1 < com_argc))
		shm->speed = Q_atoi(com_argv[i+1]);
	else if (COM_CheckParm("-44khz"))
		shm->speed = 44100;
	else if (COM_CheckParm("-22khz"))
		shm->speed = 22050;
	else
		shm->speed = 0;
*/
	if (shm->speed == 0)
	{
		for (i=0 ; i<sizeof(tryrates)/4 ; i++)
		{
			shm->speed = tryrates[i];
			if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &shm->speed) == -1)
			{
				perror(snd_dev);
			}
			else if (tryrates[i] == shm->speed)
			{
				goto SET_CHANNELS;
			}
		}

		errstr = "Soundcard does not support any known sample rates\n";
		goto SNDFAIL;
	}

	tmp = shm->speed;
	rc = ioctl (audio_fd, SNDCTL_DSP_SPEED, &tmp);
	if (rc < 0)
	{
		perror (snd_dev);
		errstr = va ("Could not set %s speed to %d\n", snd_dev, shm->speed);
		goto SNDFAIL;
	}
	shm->speed = tmp;

SET_CHANNELS:
/*	if ((s = getenv("QUAKE_SOUND_CHANNELS")))
		shm->channels = Q_atoi(s);
	else if ((i = COM_CheckParm("-sndmono")))
		shm->channels = 1;
	else if ((i = COM_CheckParm("-sndstereo")))
		shm->channels = 2;
	else
		shm->channels = 2;
*/
	tmp = shm->channels;
	rc = ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &tmp);
	if (rc < 0)
	{
		perror (snd_dev);
		errstr = va ("Soundcard does not support %d channels.\n", shm->channels);
		goto SNDFAIL;
	}
	shm->channels = tmp;

/*
	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info) == -1)
	{
		perror ("GETOSPACE");
		errstr = "Um, can't do GETOSPACE?\n";
		goto SNDFAIL;
	}

	shm->samples = info.fragstotal * info.fragsize / (shm->samplebits / 8);
//	shm->submission_chunk = 1;

// memory map the dma buffer
	shm->buffer = (unsigned char *)mmap (NULL, info.fragstotal * info.fragsize, PROT_WRITE, MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (!shm->buffer || shm->buffer == (unsigned char *)MAP_FAILED)
	{
		perror (snd_dev);
		errstr = va("Could not mmap %s (error %d)\n", snd_dev, errno);
		goto SNDFAIL;
	}

// toggle the trigger & start her up
	tmp = 0;
	rc  = ioctl (audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror (snd_dev);
		errstr = "Could not toggle.\n";
		goto SNDFAIL;
	}

	tmp = PCM_ENABLE_OUTPUT;
	rc = ioctl (audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror (snd_dev);
		errstr = "Could not toggle.\n";
		goto SNDFAIL;
	}
*/
//	shm->samplepos = 0;

	if (!SNDDMA_InitOSS_Direct (&errstr))
	{
//		if (!SNDDMA_InitOSS_Indirect (&errstr))
			goto SNDFAIL;
	}

	snd_inited = true;
	return true;


SNDFAIL:
	if (errstr)
		Con_Printf ("\x02%s", errstr);
	if (audio_fd >= 0)
		close (audio_fd);
	return false;
}

#ifdef RQM_ALSA_TEST

#define NB_PERIODS 2

const char* pcm_name = "default";

qboolean SNDDMA_InitALSA (void)
{
	int					err;
	snd_pcm_hw_params_t	*hw_params = NULL;
	snd_pcm_format_t 	snd_pcm_format;
	snd_pcm_uframes_t	buffer_size;

	err = snd_pcm_open (&pcm_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't open audio device \"%s\" (%s)\n",
					pcm_name, snd_strerror (err));
		return false;
	}

	// Allocate the hardware parameters
	err = snd_pcm_hw_params_malloc (&hw_params);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't allocate hardware parameters (%s)\n",
					snd_strerror (err));
		goto init_error;
	}
	err = snd_pcm_hw_params_any (pcm_handle, hw_params);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't initialize hardware parameters (%s)\n",
					snd_strerror (err));
		goto init_error;
	}

	// Set the access type
	err = snd_pcm_hw_params_set_access (pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't set access type (%s)\n",
					snd_strerror (err));
		goto init_error;
	}

	// Set the sound width
	if (shm->samplebits == 8)
		snd_pcm_format = SND_PCM_FORMAT_U8;
	else
	{
		snd_pcm_format = SND_PCM_FORMAT_S16;
		shm->samplebits = 16;
	}
	err = snd_pcm_hw_params_set_format (pcm_handle, hw_params, snd_pcm_format);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't set sound width to %hu (%s)\n",
					shm->samplebits, snd_strerror (err));
		goto init_error;
	}

	// Set the sound channels
	err = snd_pcm_hw_params_set_channels (pcm_handle, hw_params, shm->channels);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't set sound channels to %hu (%s)\n",
					shm->channels, snd_strerror (err));
		goto init_error;
	}

	// Set the sound speed
	if (shm->speed == 0)
		shm->speed = tryrates[0];

	err = snd_pcm_hw_params_set_rate (pcm_handle, hw_params, shm->speed, 0);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't set sound speed to %u (%s)\n",
					shm->speed, snd_strerror (err));
		goto init_error;
	}

	buffer_size = shm->speed / 5;
	err = snd_pcm_hw_params_set_buffer_size_near (pcm_handle, hw_params, &buffer_size);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't set sound buffer size to %lu (%s)\n",
					buffer_size, snd_strerror (err));
		goto init_error;
	}

	buffer_size /= NB_PERIODS;
	err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &buffer_size, 0);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't set sound period size to %lu (%s)\n",
					buffer_size, snd_strerror (err));
		goto init_error;
	}

	err = snd_pcm_hw_params (pcm_handle, hw_params);
	if (err != 0)
	{
		Con_Printf ("\x02""SNDDMA_InitALSA: can't set hardware parameters (%s)\n",
					snd_strerror (err));
		goto init_error;
	}

	snd_pcm_hw_params_free (hw_params);

	shm->buffer = Q_malloc (65536);
	shm->samples = 65536 / (shm->samplebits/8);
	snd_inited = true;
	return true;

	/*shm->buffer = Snd_CreateRingBuffer (requested, 0, NULL);
	if (shm->buffer)
		return true;*/


// It's not very clean, but it avoids a lot of duplicated code.
init_error:

	if (hw_params != NULL)
		snd_pcm_hw_params_free (hw_params);
	SNDDMA_Shutdown ();
	return false;
}
#endif

qboolean SNDDMA_Init (void)
{
	int		i;
	char	*s;
	qboolean result;

	shm = &sn;
	shm->splitbuffer = 0;

	if ((s = getenv("QUAKE_SOUND_SAMPLEBITS")))
		shm->samplebits = Q_atoi(s);
	else if ((i = COM_CheckParm("-sndbits")) && (i + 1 < com_argc))
		shm->samplebits = Q_atoi(com_argv[i+1]);
	else
		shm->samplebits = 0;

	if ((s = getenv("QUAKE_SOUND_SPEED")))
		shm->speed = Q_atoi(s);
	else if ((i = COM_CheckParm("-sndspeed")) && (i + 1 < com_argc))
		shm->speed = Q_atoi(com_argv[i+1]);
	else if (COM_CheckParm("-44khz"))
		shm->speed = 44100;
	else if (COM_CheckParm("-22khz"))
		shm->speed = 22050;
	else
		shm->speed = 0;

	if ((s = getenv("QUAKE_SOUND_CHANNELS")))
		shm->channels = Q_atoi(s);
	else if ((i = COM_CheckParm("-sndmono")))
		shm->channels = 1;
	else if ((i = COM_CheckParm("-sndstereo")))
		shm->channels = 2;
	else
		shm->channels = 2;

	if (snd_useoss)
		result = SNDDMA_InitOSS ();
#ifdef RQM_ALSA_TEST
	else
		result = SNDDMA_InitALSA ();
#endif

	if (result)
	{
		shm->submission_chunk = 1;
		shm->samplepos = 0;
	}

	return result;
}

int SNDDMA_GetDMAPos (void)
{
	struct count_info count;

	if (!snd_inited)
		return 0;

	if (snd_useoss)
	{
		if (ioctl(audio_fd, SNDCTL_DSP_GETOPTR, &count) == -1)
		{
			perror (snd_dev);
			Con_Print ("Uh, sound dead.\n");
			close (audio_fd);
			snd_inited = false;
			return 0;
		}
	//	shm->samplepos = (count.bytes / (shm->samplebits / 8)) & (shm->samples-1);
	//	fprintf(stderr, "%d    \r", count.ptr);
		shm->samplepos = count.ptr / (shm->samplebits / 8);
	}
	else
	{
		/****** FIXME ******/
	}

	return shm->samplepos;
}

void SNDDMA_Shutdown (void)
{
	if (snd_useoss)
	{
		if (snd_inited)
		{
			ioctl(audio_fd, SNDCTL_DSP_RESET, NULL);		// fixes hang when using aoss
			close (audio_fd);
			audio_fd = 0;
		}
	}
#ifdef RQM_ALSA_TEST
	else if (pcm_handle)
	{
		snd_pcm_close (pcm_handle);
		pcm_handle = NULL;
	}
#endif

	snd_inited = false;
}

byte * SNDDMA_LockBuffer (unsigned *size_out)
{
	// *size_out = ????
	return shm->buffer;
}

void SNDDMA_UnlockBuffer (byte *pbuf, unsigned dwSize)
{

}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit (void)
{
#ifdef RQM_ALSA_TEST
	snd_pcm_sframes_t count;

	if (snd_useoss)
		return;			// hardware accesses buffer via dma

//	count = snd_pcm_writei (pcm_handle, shm->buffer, nbframes);
#endif
}
