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
// menu.h

// menus
void M_Init (void);
qboolean M_HandleKey (int key, qboolean down);
void M_Draw (void);
void M_ToggleMenu_f (cmd_source_t src);
void M_Menu_Main_f (cmd_source_t src);
void M_Menu_Quit_f (cmd_source_t src);

void M_Print (int cx, int cy, const char *str);
void M_PrintWhite (int cx, int cy, const char *str);
void M_PrintBig (int cx, int cy, const char *str);

void M_DrawPic (int x, int y, const mpic_t *pic);
void M_DrawTransPic (int x, int y, const mpic_t *pic);

#ifdef HEXEN2_SUPPORT
  void M_SetScale (qboolean enable);		/*** call this before any other M_xxx calls ***/
  void M_DrawSpinningCursor (int x, int y);

  void M_DrawTransPic2 (int x, int y, const mpic_t *pic);
  void M_IPrint (int cx, int cy, const char *str);
#endif

int  M_GetParticlePreset (void);
void M_SetParticlePreset (int preset);
void M_OnChange_gl_smoothfont (float newval);
