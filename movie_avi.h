/*
Copyright (C) 2002 Quake done Quick

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
// movie_avi.h

// if QWINAVI is defined, the Windows API code in movie_win.c is used.
// Otherwise, LordHavoc's direct AVI creation code is used (movie_avi.c)

//#define QWINAVI

qboolean Capture_InitAVI (void);
qboolean Capture_InitACM (void);
qboolean Capture_Open (const char *filename, int width, int height, float fps);
qboolean Capture_WriteVideo (void);
qboolean Capture_WriteAudio (const byte *sample_buffer, int samples);
void Capture_Close (void);
double Capture_FrameTime (void);
qboolean Capture_GetSoundtime (void);
void Capture_UpdateRamps (void);

extern byte * Movie_ScaleDownBGR (const byte *in, int inw, int inh, byte *out, int outw, int outh);
