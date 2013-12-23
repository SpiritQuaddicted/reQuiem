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
// cd_mac.c  (UNIMPLEMENTED)

#include "quakedef.h"

extern qboolean cdValid;
extern qboolean	playing;
extern byte		playTrack;
extern qboolean playLooping;

extern	cvar_t	bgmvolume;


/*
============
CDAudio_Eject
============
*/
void CDAudio_Eject (void)
{
}

/*
============
CDAudio_CloseDoor
============
*/
void CDAudio_CloseDoor (void)
{
}

/*
============
CDAudio_GetNumTracks
============
*/
int CDAudio_GetNumTracks (void)
{
	return 0;
}

/*
============
CDAudio_GetTrackLength
============
*/
int CDAudio_GetTrackLength (byte track)
{
	return 0;
}

/*
============
CDAudio_PlayTrack
============
*/
qboolean CDAudio_PlayTrack (byte track, int length)
{
	return false;
}

/*
============
CDAudio_StopDevice
============
*/
qboolean CDAudio_StopDevice (void)
{
	return true;
}

/*
============
CDAudio_PauseDevice
============
*/
qboolean CDAudio_PauseDevice (void)
{
	return true;
}

/*
============
CDAudio_ResumeDevice
============
*/
qboolean CDAudio_ResumeDevice (void)
{
	return false;
}

/*
============
CDAudio_InitDevice
============
*/
qboolean CDAudio_InitDevice (void)
{	
	return false;
}

/*
============
CDAudio_CloseDevice
============
*/
void CDAudio_CloseDevice (void)
{
}
