
/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2005 John Fitzgibbons and others

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
//gl_fog.c -- global and volumetric fog

#include "quakedef.h"

#ifndef RQM_SV_ONLY

//==============================================================================
//
//  GLOBAL FOG
//
//==============================================================================

#define FOG_RGB_DEFAULT "0.3"

cvar_t	gl_fogenable	= {"gl_fogenable", "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_fogdensity	= {"gl_fogdensity", "0.05", CVAR_FLAG_ARCHIVE};
cvar_t	gl_fogred	= {"gl_fogred", FOG_RGB_DEFAULT, CVAR_FLAG_ARCHIVE};
cvar_t	gl_fogblue	= {"gl_fogblue", FOG_RGB_DEFAULT, CVAR_FLAG_ARCHIVE};
cvar_t	gl_foggreen	= {"gl_foggreen", FOG_RGB_DEFAULT, CVAR_FLAG_ARCHIVE};

float old_density;
float old_red;
float old_green;
float old_blue;

float fade_time; //duration of fade
float fade_done; //time when fade will be done

// these vars remember fog's original settings, so if one map changes it,
//  the next map will be back to user's prefs
float orig_fogenable, orig_fogdensity, orig_fogred, orig_fogblue, orig_foggreen;
qboolean orig_fogvalid = false;

//#define LINEAR_FOG

/*
=============
Fog_Update

update internal variables
=============
*/
void Fog_Update (float density, float red, float green, float blue, float time)
{
	//save previous settings for fade
	if (time > 0)
	{
		//check for a fade in progress
		if (fade_done > cl.time)
		{
			float f;

			f = (fade_done - cl.time) / fade_time;
			old_density = f * old_density + (1.0 - f) * gl_fogdensity.value;
			old_red = f * old_red + (1.0 - f) * gl_fogred.value;
			old_green = f * old_green + (1.0 - f) * gl_foggreen.value;
			old_blue = f * old_blue + (1.0 - f) * gl_fogblue.value;
		}
		else
		{
			old_density = gl_fogdensity.value;
			old_red = gl_fogred.value;
			old_green = gl_foggreen.value;
			old_blue = gl_fogblue.value;
		}
	}

	Cvar_SetValueDirect (&gl_fogdensity, density);
	Cvar_SetValueDirect (&gl_fogenable, (density == 0) ? 0 : 1);

	Cvar_SetValueDirect (&gl_fogred, red);
	Cvar_SetValueDirect (&gl_foggreen, green);
	Cvar_SetValueDirect (&gl_fogblue, blue);

	fade_time = time;
	fade_done = cl.time + time;
}

/*
=============
Fog_ParseFitzMessage

handle an svc_fog_fitz message (FitzQuake) from server
=============
*/
void Fog_ParseFitzMessage (void)
{
	float density, red, green, blue, time;

	density = MSG_ReadByte() / 255.0;
	red = MSG_ReadByte() / 255.0;
	green = MSG_ReadByte() / 255.0;
	blue = MSG_ReadByte() / 255.0;
	time = max(0.0, MSG_ReadShort() / 100.0);

	Fog_Update (density, red, green, blue, time);
}

/*
=============
Fog_ParseNehMessage

handle an svc_fog_neh message (Nehahra) from server
=============
*/
void Fog_ParseNehMessage (void)
{
	float density, red, green, blue;

	if (MSG_ReadByte())
	{
		density = MSG_ReadFloat();
		red = MSG_ReadByte() / 255.0;
		green = MSG_ReadByte() / 255.0;
		blue = MSG_ReadByte() / 255.0;
	}
	else
	{
		density = 0.0f;
		red = gl_fogred.value;
		green = gl_foggreen.value;
		blue = gl_fogblue.value;
	}

	Fog_Update (density, red, green, blue, 0);
}

/*
=============
Fog_FogCommand_f  (from FitzQuake)

handle the 'fog' console command
=============
*/
void Fog_FogCommand_f (cmd_source_t src)
{
	int argc = Cmd_Argc();
	float density, r, g, b, time;

	switch (argc)
	{
	case 2:
	case 3: //TEST
		density = max(0.0, Q_atof(Cmd_Argv(1)));
		time = (argc == 2) ? 0.0 : Q_atof(Cmd_Argv(2));
		r = gl_fogred.value;
		g = gl_foggreen.value;
		b = gl_fogblue.value;
		if ((density == 0) || (density == 1))
		{
		// JDH: so fog can be disabled/enabled without changing any other settings
			Fog_Update (gl_fogdensity.value, r, g, b, time);
			Cvar_SetValueDirect (&gl_fogenable, density);
			return;
		}
		break;
	case 4:
		density = gl_fogdensity.value;
		r = bound (0.0, Q_atof(Cmd_Argv(1)), 1.0);
		g = bound (0.0, Q_atof(Cmd_Argv(2)), 1.0);
		b = bound (0.0, Q_atof(Cmd_Argv(3)), 1.0);
		time = 0.0;
		break;
	case 5:
	case 6: //TEST
		density = max (0.0, Q_atof(Cmd_Argv(1)));
		r = bound (0.0, Q_atof(Cmd_Argv(2)), 1.0);
		g = bound (0.0, Q_atof(Cmd_Argv(3)), 1.0);
		b = bound (0.0, Q_atof(Cmd_Argv(4)), 1.0);
		time = (argc == 5) ? 0.0 : Q_atof(Cmd_Argv(5));
		break;
	default:
		Con_Print ("usage:\n");
		Con_Print ("   fog <density>\n");
		Con_Print ("   fog <red> <green> <blue>\n");
		Con_Print ("   fog <density> <red> <green> <blue>\n");
		Con_Print ("current values:\n");
		Con_Printf ("   gl_fogenable is %d\n", (gl_fogenable.value == 0.0 ? 0 : 1));
		Con_Printf ("   gl_fogdensity is %f\n", gl_fogdensity.value);
		Con_Printf ("   gl_fogred is %f\n", gl_fogred.value);
		Con_Printf ("   gl_foggreen is %f\n", gl_foggreen.value);
		Con_Printf ("   gl_fogblue is %f\n", gl_fogblue.value);
		return;
	}

	Fog_Update (density, r, g, b, time);
}

/*
=============
Fog_InitWorldspawn

called at map load, if "fog" key is found
=============
*/
void Fog_InitWorldspawn (const char *value)
{
	float fog_density, fog_red, fog_green, fog_blue;
	int argc;

	// back up current values:
/*	orig_fogenable = gl_fogenable.value;
	orig_fogdensity = fog_density = gl_fogdensity.value;
	orig_fogred = fog_red = gl_fogred.value;
	orig_foggreen = fog_green = gl_foggreen.value;
	orig_fogblue = fog_blue = gl_fogblue.value;
	orig_fogvalid = true;
*/
	fog_density = gl_fogdensity.value;
	fog_red = gl_fogred.value;
	fog_green = gl_foggreen.value;
	fog_blue = gl_fogblue.value;

	// parse & set new values:
	argc = sscanf (value, "%f %f %f %f", &fog_density, &fog_red, &fog_green, &fog_blue);
	if (argc > 0)
	{
		if (argc == 1)
		{
			fog_red = fog_blue = fog_green = Q_atof (FOG_RGB_DEFAULT);
		}

		Fog_Update (fog_density, fog_red, fog_green, fog_blue, 0);
	}
}


static float	v_lava_colors[4] = {1.0f, 0.314f, 0.0f, 0.5f};
static float	v_slime_colors[4] = {0.039f, 0.738f, 0.333f, 0.5f};
static float	v_water_colors[4] = {0.039f, 0.584f, 0.888f, 0.5f};

/*
=============
Fog_AddWaterFog

Fog in liquids, from FuhQuake
  JDH: modified to color fog based on liquid color
=============
*/
void Fog_AddWaterfog (unsigned color, int contents)
{
	float	*fogcolor, density;
	float	colors[4];

	if (color)
	{
		colors[0] = (color & 0x000000FF) / 255.0;
		colors[1] = ((color & 0x0000FF00) >> 8) / 255.0;
		colors[2] = ((color & 0x00FF0000) >> 16) / 255.0;
		colors[3] = 0.5;
		fogcolor = colors;
	}
	else
	{
		switch (contents)
		{
		case CONTENTS_LAVA:
			fogcolor = v_lava_colors;
			break;

		case CONTENTS_SLIME:
			fogcolor = v_slime_colors;
			break;

		default:
			fogcolor = v_water_colors;
			break;
		}
	}

	density = bound(0, gl_waterfog_density.value, 1);

#ifdef LINEAR_FOG

	glFogi (GL_FOG_MODE, GL_LINEAR);
	glFogf (GL_FOG_START, 150.0f);
	glFogf (GL_FOG_END, 4250.0f - (4250.0f - 1536.0f) * density);

#else
	if (color)
		density *= 0.004;
	else
		density = 0.0002 + (0.0009 - 0.0002) * density;

	glFogf (GL_FOG_DENSITY, density);
	glFogi (GL_FOG_MODE, GL_EXP);
#endif

	glFogfv (GL_FOG_COLOR, fogcolor);
	glEnable (GL_FOG);
	return;
}

/*
=============
Fog_SetupFrame

called at the beginning of each frame
=============
*/
void Fog_SetupFrame (void)
{
	float c[4];
	float f, d;

//	if (gl_fogenable.value)
//	{
		if (fade_done > cl.time)
		{
			f = (fade_done - cl.time) / fade_time;
			d = f * old_density + (1.0 - f) * gl_fogdensity.value;
			c[0] = f * old_red + (1.0 - f) * gl_fogred.value;
			c[1] = f * old_green + (1.0 - f) * gl_foggreen.value;
			c[2] = f * old_blue + (1.0 - f) * gl_fogblue.value;
		}
		else
		{
			d = gl_fogdensity.value;
			c[0] = gl_fogred.value;
			c[1] = gl_foggreen.value;
			c[2] = gl_fogblue.value;
		}

		c[3] = 1.0;

#ifdef LINEAR_FOG
		glFogi (GL_FOG_MODE, GL_LINEAR);
		glFogi (GL_FOG_START, 0);
		glFogf (GL_FOG_END, 96.0 / max(d, 0.000001)); //don't divide by zero
#else
		glFogi (GL_FOG_MODE, GL_EXP2);

//		d /= (nehahra ? 100.0 : 64.0);
		d /= 90.0;
		glFogf (GL_FOG_DENSITY, d);
#endif

		glFogfv (GL_FOG_COLOR, c);

		Fog_EnableGFog ();

/*		glEnable (GL_FOG);
	}
	else
	{
		glDisable (GL_FOG);
	}*/
}

/*
=============
Fog_GetDensity

returns current density of fog
=============
*/
float Fog_GetDensity (void)
{
	float f;

	if (!gl_fogenable.value)
		return 0;

	if (fade_done > cl.time)
	{
		f = (fade_done - cl.time) / fade_time;
		return f * old_density + (1.0 - f) * gl_fogdensity.value;
	}
	else
		return gl_fogdensity.value;
}

/*
=============
Fog_EnableGFog

called before drawing stuff that should be fogged
=============
*/
void Fog_EnableGFog (void)
{
	if (Fog_GetDensity() > 0)
		glEnable(GL_FOG);
}

/*
=============
Fog_DisableGFog

called after drawing stuff that should be fogged
=============
*/
void Fog_DisableGFog (void)
{
	if (Fog_GetDensity() > 0)
		glDisable(GL_FOG);
}

/*
=============
Fog_Reset

called when a map is loaded, or client disconnects
=============
*/
void Fog_Reset (void)
{
	if (orig_fogvalid)
	{
		Cvar_SetValueDirect (&gl_fogenable, orig_fogenable);
		Cvar_SetValueDirect (&gl_fogdensity, orig_fogdensity);
		Cvar_SetValueDirect (&gl_fogred, orig_fogred);
		Cvar_SetValueDirect (&gl_foggreen, orig_foggreen);
		Cvar_SetValueDirect (&gl_fogblue, orig_fogblue);
		orig_fogvalid = false;
	}
}
/*
=============
Fog_NewMap

called whenever a map is loaded
=============
*/
void Fog_NewMap (void)
{
	Fog_Reset ();

	// back up current values:		(2009.04.17: moved here from Fog_InitWorldspawn)
	orig_fogenable = gl_fogenable.value;
	orig_fogdensity = gl_fogdensity.value;
	orig_fogred = gl_fogred.value;
	orig_foggreen = gl_foggreen.value;
	orig_fogblue = gl_fogblue.value;
	orig_fogvalid = true;

	fade_time = 0.0;
	fade_done = 0.0;
}

/*
=============
Fog_Init

called when quake initializes
=============
*/
void Fog_Init (void)
{
	Cvar_RegisterBool (&gl_fogenable);
	Cvar_RegisterFloat (&gl_fogdensity, 0, 1);
	Cvar_RegisterFloat (&gl_fogred, 0, 1);
	Cvar_RegisterFloat (&gl_foggreen, 0, 1);
	Cvar_RegisterFloat (&gl_fogblue, 0, 1);

	//fog_density = 0.0;
	Cmd_AddCommand ("fog", Fog_FogCommand_f, 0);
}

#endif		//#ifndef RQM_SV_ONLY
