/*
Copyright (C) 2000	LordHavoc, Ender

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
// nehahra.h

#define NEHAHRA_VERSION	2.54

// system types
typedef enum {NEH_TYPE_NONE, NEH_TYPE_DEMO, NEH_TYPE_GAME, NEH_TYPE_BOTH} neh_gametype_t;

extern	neh_gametype_t	NehGameType;

#ifndef RQM_SV_ONLY
extern	char	prev_skybox[64];

extern	cvar_t	gl_notrans, r_waterripple;

extern	float	r_modelalpha;

extern	sfx_t	*known_sfx;
extern	int		num_sfx, num_sfxorig;

void Neh_ResetSFX (void);
void Neh_ChangeMusicVol (float value);

void SHOWLMP_drawall (void);
void SHOWLMP_clear (void);
void SHOWLMP_decodehide (void);
void SHOWLMP_decodeshow (void);

#endif		//#ifndef RQM_SV_ONLY

void Neh_Init (void);
void Neh_InitEnv (void);
void Neh_InitVars (void);
void Neh_UninitEnv (void);

