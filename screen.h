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
// screen.h

void SCR_Init (void);
void SCR_ClearWadPics (void);
void SCR_LoadWadPics (void);

void SCR_UpdateScreen (void);
//void SCR_UpdateWholeScreen (void);

void SCR_SizeUp (void);
void SCR_SizeDown (void);
void SCR_CenterPrint (const char *str);

void ApplyGamma (byte *buffer, int size);

void SCR_BeginLoadingPlaque (const char *caption);
void SCR_EndLoadingPlaque (void);
void SCR_UpdateLoadCaption (const char *caption);

int SCR_ModalMessage (const char *text, const char *input_keys);

#ifdef HEXEN2_SUPPORT
void SCR_ShowLoadingSize (void);
void ReformatText (char *text, int width);
#endif

extern	float		scr_con_current;
extern	float		scr_conlines;		// lines of console to display

extern	int			clearnotify;	// set to 0 whenever notify text is drawn
extern	qboolean	scr_disabled_for_loading;
extern	qboolean	scr_skipupdate;

extern	cvar_t		scr_viewsize;

// only the refresh window will be updated unless these variables are flagged 
//extern	int			scr_copytop;
//extern	int			scr_copyeverything;
//extern	int			scr_fullupdate;	// set to 0 to force full redraw

extern qboolean		block_drawing;

#define SCR_NUM_SSHOT_EXTS 3
extern const char *		scr_sshot_exts[SCR_NUM_SSHOT_EXTS];


