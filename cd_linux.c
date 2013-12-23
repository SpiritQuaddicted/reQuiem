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
// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <linux/cdrom.h>

#include "quakedef.h"

extern qboolean	playing;
extern qboolean	enabled;
extern qboolean playLooping;
extern byte		playTrack;

static	int		cdfile = -1;
static	char		cd_dev[64] = "/dev/cdrom";
static	struct cdrom_volctrl cd_initialvol;

/*
============
CDAudio_Eject
============
*/
void CDAudio_Eject (void)
{
	if (cdfile == -1 || !enabled)
		return;	// no cd init'd

	if (ioctl(cdfile, CDROMEJECT) == -1)
		Con_DPrintf ("ioctl cdromeject failed\n");
}

/*
============
CDAudio_CloseDoor
============
*/
void CDAudio_CloseDoor (void)
{
	if (cdfile == -1 || !enabled)
		return;	// no cd init'd

	if (ioctl(cdfile, CDROMCLOSETRAY) == -1)
		Con_DPrintf ("ioctl cdromclosetray failed\n");
}

/*
============
CDAudio_GetNumTracks
============
*/
int CDAudio_GetNumTracks (void)
{
	struct	cdrom_tochdr	tochdr;

	if (ioctl(cdfile, CDROMREADTOCHDR, &tochdr) == -1)
	{
		Con_DPrintf ("ioctl cdromreadtochdr failed\n");
		return -1;
	}

	if (tochdr.cdth_trk0 < 1)
	{
		Con_DPrintf ("CDAudio: no music tracks\n");
		return -1;
	}

	return tochdr.cdth_trk1;
}

/*
============
CDAudio_GetTrackLength
============
*/
int CDAudio_GetTrackLength (byte track)
{
	struct	cdrom_tocentry	entry;

	if (cdfile == -1)
		return -1;

	entry.cdte_track = track;
	entry.cdte_format = CDROM_MSF;
	if (ioctl(cdfile, CDROMREADTOCENTRY, &entry) == -1)
	{
		Con_DPrintf ("ioctl cdromreadtocentry failed\n");
		return -1;
	}
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
	{
		Con_Printf ("CDAudio: track %i is not audio\n", track);
		return -1;
	}
	
	return 1;
}

/*
============
CDAudio_PlayTrack
============
*/
qboolean CDAudio_PlayTrack (byte track, int length)
{
	struct	cdrom_ti	ti;

	ti.cdti_trk0 = track;
	ti.cdti_trk1 = track;
	ti.cdti_ind0 = 1;
	ti.cdti_ind1 = 99;

	if (ioctl(cdfile, CDROMPLAYTRKIND, &ti) == -1)
	{
		Con_DPrintf ("ioctl cdromplaytrkind failed\n");
		return false;
	}

	if (ioctl(cdfile, CDROMRESUME) == -1)
		Con_DPrintf ("ioctl cdromresume failed\n");

	return true;
}

/*
============
CDAudio_StopDevice
============
*/
qboolean CDAudio_StopDevice (void)
{
	if (cdfile == -1)
		return false;

	if (ioctl(cdfile, CDROMSTOP) == -1)
	{
		Con_DPrintf ("ioctl cdromstop failed (%d)\n", errno);
		return false;
	}

	return true;
}

/*
============
CDAudio_PauseDevice
============
*/
qboolean CDAudio_PauseDevice (void)
{
	if (cdfile == -1)
		return false;

	if (ioctl(cdfile, CDROMPAUSE) == -1)
	{
		Con_DPrintf ("ioctl cdrompause failed\n");
		return false;
	}

	return true;
}

/*
============
CDAudio_ResumeDevice
============
*/
qboolean CDAudio_ResumeDevice (void)
{
	if (cdfile == -1)
		return false;
	
	if (ioctl(cdfile, CDROMRESUME) == -1)
	{
		Con_DPrintf ("ioctl cdromresume failed\n");
		return false;
	}

	return true;
}

/*
============
CDAudio_Update_Linux
============
*/
void CDAudio_Update_Linux (void)
{
	struct	cdrom_subchnl	subchnl;
	static	time_t		lastchk;

	if (playing && lastchk < time(NULL))
	{
		lastchk = time(NULL) + 2;	// two seconds between chks
		subchnl.cdsc_format = CDROM_MSF;
		if (ioctl(cdfile, CDROMSUBCHNL, &subchnl) == -1)
		{
			Con_DPrintf ("ioctl cdromsubchnl failed\n");
			playing = false;
			return;
		}
		if (subchnl.cdsc_audiostatus != CDROM_AUDIO_PLAY && subchnl.cdsc_audiostatus != CDROM_AUDIO_PAUSED)
		{
			playing = false;
			if (playLooping)
				CDAudio_Play (SRC_COMMAND, playTrack, true);
		}
	}
}

/*
============
CDAudio_UpdateVolume
============
*/
void CDAudio_UpdateVolume (float vol)
{
	struct cdrom_volctrl	vc;

	if (cdfile == -1)
		return;

	vc.channel0 = vc.channel1 = vc.channel2 = vc.channel3 = 255*vol;

	if (ioctl(cdfile, CDROMVOLCTRL, &vc) == -1)
	{
		Con_DPrintf ("ioctl cdromvolctrl failed\n");
	}
}

/*
============
CDAudio_InitDevice
============
*/
qboolean CDAudio_InitDevice (void)
{
	int	i;

	if ((i = COM_CheckParm("-cddev")) != 0 && i < com_argc - 1)
	{
		Q_strcpy (cd_dev, com_argv[i + 1], sizeof(cd_dev));
	}

	if ((cdfile = open(cd_dev, O_RDONLY)) == -1)
	{
		if (errno != ENOMEDIUM)
			Con_Printf ("%cCDAudio_Init: open of \"%s\" failed (%i)\n", 2, cd_dev, errno);
		cdfile = -1;
		return false;
	}

	if (ioctl(cdfile, CDROMVOLREAD, &cd_initialvol) == -1)
	{
		Con_DPrintf ("ioctl cdromvolread failed\n");
	}

	return true;
}

/*
============
CDAudio_CloseDevice
============
*/
void CDAudio_CloseDevice (void)
{
// JDH: restore original volume
	if (ioctl(cdfile, CDROMVOLCTRL, &cd_initialvol) == -1)
	{
		Con_DPrintf ("ioctl cdromvolctrl failed\n");
	}

	close (cdfile);
	cdfile = -1;
}

