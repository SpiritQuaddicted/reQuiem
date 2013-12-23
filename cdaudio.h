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
// cdaudio.h

int  CDAudio_Init (void);
void CDAudio_Play (cmd_source_t src, byte track, qboolean looping);
void CDAudio_Stop (cmd_source_t src);
void CDAudio_Pause (cmd_source_t src);
void CDAudio_Resume (cmd_source_t src);
void CDAudio_Shutdown (void);
void CDAudio_Update (void);

void CDAudio_Eject (void);
void CDAudio_CloseDoor (void);
int  CDAudio_GetNumTracks (void);

extern qboolean CDAudio_InitDevice (void);
extern void     CDAudio_CloseDevice (void);
extern qboolean CDAudio_PlayTrack (byte track, int length);
extern int      CDAudio_GetTrackLength (byte track);
extern qboolean CDAudio_StopDevice (void);
extern qboolean CDAudio_PauseDevice (void);
extern qboolean CDAudio_ResumeDevice (void);
extern void		CDAudio_UpdateVolume (float vol);
#ifndef _WIN32
extern void     CDAudio_Update_Linux (void);
#endif

