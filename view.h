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
// view.h

extern	cvar_t	v_gamma;
extern	cvar_t	v_contrast;

//#ifdef GLQUAKE
extern	float	v_blend[4];
void V_AddLightBlend (float r, float g, float b, float a2);
//#endif

//extern	byte	gammatable[256];	// palette is sent through this
extern	byte	current_pal[768];
extern	cvar_t	gl_hwblend;
//#ifndef GLQUAKE
//extern	cvar_t	lcd_x, lcd_yaw;
//#endif

#ifdef NEWHWBLEND
#  define V_WANT_HWBLEND() ((cls.state == ca_disconnected) || !v_blend[3] || ((key_dest == key_game) && !scr_con_current))
#  define V_USING_HWRAMPS() (vid_hwgamma_enabled && ((gl_hwblend.value >= 2) || (gl_hwblend.value && V_WANT_HWBLEND())))
#else
#  define V_USING_HWRAMPS() (vid_hwgamma_enabled && gl_hwblend.value)
#endif

void V_Init (void);
void V_RenderView (qboolean force_refdef_recalc);

void V_UpdatePalette (void);
void V_CalcBlend (void);
void V_SetContentsColor (int contents);
void V_ResetCshifts (void);			// JDH
void V_CalcRefdef (void);

void V_StartPitchDrift (void);
void V_StopPitchDrift (void);
float V_CalcRoll (vec3_t angles, vec3_t velocity);

void V_ParseDamage (qboolean parse_only);

#ifdef HEXEN2_SUPPORT
  void V_DarkFlash_f (cmd_source_t src);
  void V_WhiteFlash_f (cmd_source_t src);
#endif
