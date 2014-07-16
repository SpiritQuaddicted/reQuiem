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

// the status bar is only redrawn if something has changed, but if anything
// does, the entire thing will be redrawn for the next vid.numpages frames.

void Sbar_Init (void);
void Sbar_Hipnotic_Init (void);
void Sbar_Rogue_Init (void);
void Sbar_ClearWadPics (void);
void Sbar_LoadWadPics (void);

void Sbar_Changed (void);
// call whenever any of the client stats represented on the sbar changes

int Sbar_Height (void);
void Sbar_SetScale (qboolean enable);		// JDH

void Sbar_Draw (void);
// called every frame by screen

void Sbar_IntermissionOverlay (void);
// called each frame after the level has been completed

void Sbar_FinaleOverlay (void);

const char * Sbar_FormatTime (double time, qboolean use_tenths);

#ifdef HEXEN2_SUPPORT
  #define ABILITIES_STR_INDEX	400

  int  Sbar_itoa (int num, char *buf);
  void Sbar_DeathmatchOverlay (void);

  void Sbar_Hexen2_Init (void);
  void Sbar_LoadWadPics_H2 (void);
  void Sbar_Draw_H2 (void);
  void Sbar_InvReset (void);
  void Sbar_InvChanged (void);
  void Sbar_ViewSizeChanged (void);

  void Sbar_InvLeft_f (cmd_source_t src);
  void Sbar_InvRight_f (cmd_source_t src);
  void Sbar_InvUse_f (cmd_source_t src);
  void Sbar_InvOff_f (cmd_source_t src);
  void Sbar_ShowInfoDown_f (cmd_source_t src);
  void Sbar_ShowInfoUp_f (cmd_source_t src);
  void Sbar_ShowDMDown_f (cmd_source_t src);
  void Sbar_ShowDMUp_f (cmd_source_t src);
  void Sbar_ToggleDM_f (cmd_source_t src);
#endif
