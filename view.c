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
// view.c -- player eye positioning

#include "quakedef.h"

cvar_t	cl_rollspeed = {"cl_rollspeed", "200"};
cvar_t	cl_rollangle = {"cl_rollangle", "2.0"};

#ifndef RQM_SV_ONLY

#include "movie.h"

/*

The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.

*/


/*#ifndef GLQUAKE
cvar_t	lcd_x = {"lcd_x","0"};
cvar_t	lcd_yaw = {"lcd_yaw","0"};
#endif*/

cvar_t	scr_ofsx = {"scr_ofsx","0"};
cvar_t	scr_ofsy = {"scr_ofsy","0"};
cvar_t	scr_ofsz = {"scr_ofsz","0"};

cvar_t	cl_bob = {"cl_bob","0.02"};
cvar_t	cl_bobcycle = {"cl_bobcycle","0.6"};
cvar_t	cl_bobup = {"cl_bobup","0.5"};

cvar_t	v_kicktime = {"v_kicktime", "0.5"};
cvar_t	v_kickroll = {"v_kickroll", "0.6"};
cvar_t	v_kickpitch = {"v_kickpitch", "0.6"};
cvar_t	v_gunkick = {"v_gunkick", "0", CVAR_FLAG_ARCHIVE};		// by joe

cvar_t	v_iyaw_cycle = {"v_iyaw_cycle", "2"};
cvar_t	v_iroll_cycle = {"v_iroll_cycle", "0.5"};
cvar_t	v_ipitch_cycle = {"v_ipitch_cycle", "1"};
cvar_t	v_iyaw_level = {"v_iyaw_level", "0.3"};
cvar_t	v_iroll_level = {"v_iroll_level", "0.1"};
cvar_t	v_ipitch_level = {"v_ipitch_level", "0.3"};

cvar_t	v_idlescale = {"v_idlescale", "0"};

cvar_t	v_centermove     = {"v_centermove",    "0.15"};
cvar_t	v_centerspeed    = {"v_centerspeed",    "500"};

cvar_t  v_contentblend   = {"v_contentblend",     "1", CVAR_FLAG_ARCHIVE};
cvar_t	v_damagecshift   = {"v_damagecshift",     "1", CVAR_FLAG_ARCHIVE};
cvar_t	v_quadcshift     = {"v_quadcshift",       "1", CVAR_FLAG_ARCHIVE};
cvar_t	v_suitcshift     = {"v_suitcshift",       "1", CVAR_FLAG_ARCHIVE};
cvar_t	v_ringcshift     = {"v_ringcshift",       "1", CVAR_FLAG_ARCHIVE};
cvar_t	v_pentcshift     = {"v_pentcshift",       "1", CVAR_FLAG_ARCHIVE};

//#ifdef GLQUAKE
cvar_t	v_dlightcshift   = {"v_dlightcshift",     "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_cshiftpercent = {"gl_cshiftpercent", "100", CVAR_FLAG_ARCHIVE};
cvar_t	gl_hwblend       = {"gl_hwblend",         "1", CVAR_FLAG_ARCHIVE};
cvar_t	v_gamma          = {"gl_gamma",           "1", CVAR_FLAG_ARCHIVE};
cvar_t	v_contrast       = {"gl_contrast",        "1", CVAR_FLAG_ARCHIVE};
//#endif

cvar_t	v_bonusflash     = {"cl_bonusflash",      "1", CVAR_FLAG_ARCHIVE};

float	v_dmg_time, v_dmg_roll, v_dmg_pitch;

#ifdef HEXEN2_SUPPORT
  extern cvar_t v_centerrollspeed;
#endif

#endif		//#ifndef RQM_SV_ONLY

/*
===============
V_CalcRoll

Used by view and sv_user
===============
*/
vec3_t	right;

float V_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign, side;

	AngleVectors (angles, NULL, right, NULL);
	side = DotProduct(velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	side = (side < cl_rollspeed.value) ? side * cl_rollangle.value / cl_rollspeed.value : cl_rollangle.value;

	return side * sign;
}

#ifndef RQM_SV_ONLY
/*
===============
V_CalcBob
===============
*/
float V_CalcBob (void)
{
	/*static*/	float	bob;
	float		cycle;

	if (cl_bobcycle.value <= 0)
		return 0;

	cycle = cl.time - (int)(cl.time / cl_bobcycle.value) * cl_bobcycle.value;
	cycle /= cl_bobcycle.value;
	if (cycle < cl_bobup.value)
		cycle = M_PI * cycle / cl_bobup.value;
	else
		cycle = M_PI + M_PI * (cycle - cl_bobup.value) / (1.0 - cl_bobup.value);

// bob is proportional to velocity in the xy plane
// (don't count Z, or jumping messes it up)
	bob = sqrt(cl.velocity[0]*cl.velocity[0] + cl.velocity[1]*cl.velocity[1]) * cl_bob.value;
	bob = bob * 0.3 + bob * 0.7 * sin(cycle);
	bob = bound(-7, bob, 4);

	return bob;
}

//=============================================================================

void V_StartPitchDrift (void)
{
	if (cl.laststop == cl.time)
		return;		// something else is keeping it from drifting

	if (cl.nodrift || !cl.pitchvel)
	{
		cl.pitchvel = v_centerspeed.value;
		cl.nodrift = false;
		cl.driftmove = 0;
	}
}

void V_StartPitchDrift_f (cmd_source_t src)
{
	V_StartPitchDrift();
}

void V_StopPitchDrift (void)
{
	cl.laststop = cl.time;
	cl.nodrift = true;
	cl.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards cl.idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.

Drifting is enabled when the center view key is hit, mlook is released and
lookspring is non 0, or when
===============
*/
void V_DriftPitch (void)
{
	float	delta, move;

	if (noclip_anglehack || !cl.onground || cls.demoplayback)
	{
		cl.driftmove = 0;
		cl.pitchvel = 0;
		return;
	}

// don't count small mouse motion
	if (cl.nodrift)
	{
		float maxspeed = cl_forwardspeed.value;

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			maxspeed = (maxspeed * cl.v.hasted) - 10;
	#endif

		if (fabs(cl.cmd.forwardmove) < maxspeed)
			cl.driftmove = 0;
		else
			cl.driftmove += host_frametime;

		if (cl.driftmove > v_centermove.value)
			V_StartPitchDrift ();

		return;
	}

	delta = cl.idealpitch - cl.viewangles[PITCH];

	if (!delta)
	{
		cl.pitchvel = 0;
		return;
	}

	move = host_frametime * cl.pitchvel;
	cl.pitchvel += host_frametime * v_centerspeed.value;

//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			cl.pitchvel = 0;
			move = delta;
		}
		cl.viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			cl.pitchvel = 0;
			move = -delta;
		}
		cl.viewangles[PITCH] -= move;
	}
}

#ifdef HEXEN2_SUPPORT
/*
===============
V_DriftRoll

Moves the client pitch angle towards cl.idealroll sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.

===============
*/
void V_DriftRoll (void)
{
	float		delta, move;

	if (noclip_anglehack || cls.demoplayback)
		return;

	delta = cl.idealroll - cl.viewangles[ROLL];
	if (!delta)
	{
		cl.rollvel = 0;
		return;
	}

	move = host_frametime * cl.rollvel;
	cl.rollvel += host_frametime * v_centerrollspeed.value;

	if (delta > 0)
	{
		if (move > delta)
		{
			cl.rollvel = 0;
			move = delta;
		}
		cl.viewangles[ROLL] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			cl.rollvel = 0;
			move = -delta;
		}
		cl.viewangles[ROLL] -= move;
	}
}
#endif

/*
==============================================================================

				PALETTE FLASHES

==============================================================================
*/

// JDH: linked list of cshifts, used for demo rewind
typedef struct cshift_list_s 
{
	cshift_t cshift;
	int rep_count;			// merge duplicate cshifts into a single entry
	struct cshift_list_s *prev;
} cshift_list_t;

cshift_list_t	cshift_empty = {{{130, 80, 50}, 0}, 0, NULL};
cshift_t		cshift_water = {{130, 80, 50}, 128};
cshift_t		cshift_slime = {{0, 25, 5}, 150};
cshift_t		cshift_lava  = {{255, 80, 0}, 150};

float		v_blend[4];		// rgba 0.0 - 1.0
unsigned short	ramps[3][256];


/*
===============
V_ParseDamage
===============
*/
void V_ParseDamage (qboolean parse_only)
{
	int			i, armor, blood;
	vec3_t		from, forward, right, up;
	entity_t	*ent;
	float		side, count, fraction;

	armor = MSG_ReadByte ();
	blood = MSG_ReadByte ();
	for (i=0 ; i<3 ; i++)
		from[i] = MSG_ReadCoord ();

	if (parse_only)
		return;			// JDH: don't want cshifts when doing demo seek
	
	count = blood*0.5 + armor*0.5;
	if (count < 10)
		count = 10;

// put sbar face into pain frame
	if (cl.faceanim_endtime < cl.time)
		cl.faceanim_starttime = cl.time;		// otherwise just extend current interval
	cl.faceanim_endtime = cl.time + 0.2;

	cl.cshifts[CSHIFT_DAMAGE].percent += 3*count;
	if (cl.cshifts[CSHIFT_DAMAGE].percent < 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;
	if (cl.cshifts[CSHIFT_DAMAGE].percent > 150)
		cl.cshifts[CSHIFT_DAMAGE].percent = 150;

	fraction = bound(0, v_damagecshift.value, 1);
	cl.cshifts[CSHIFT_DAMAGE].percent *= fraction;

	if (armor > blood)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}

// calculate view angle kicks
	ent = &cl_entities[cl.viewentity];

	VectorSubtract (from, ent->origin, from);
	VectorNormalize (from);

	AngleVectors (ent->angles, forward, right, up);

	side = DotProduct (from, right);
	v_dmg_roll = count*side*v_kickroll.value;

	side = DotProduct (from, forward);
	v_dmg_pitch = count*side*v_kickpitch.value;

	v_dmg_time = v_kicktime.value;
}

/*
==================
V_cshift_f
==================
*/
void V_cshift_f (cmd_source_t src)
{
	cshift_list_t *oldcs;
	cshift_t newcs;
	
// JDH: this is so cshifts still work properly when rewinding a demo (eg. Quoth's trinity)
	if ((src == SRC_SERVER) && cls.demoplayback)
	{
		if (cl_demorewind.value)
		{
			assert (((cshift_empty.prev != NULL) || (cshift_empty.rep_count != 0)));			

			// restore previous colors when rewinding
			if (cshift_empty.rep_count)
				cshift_empty.rep_count--;
			else
			{
				oldcs = cshift_empty.prev;
				if (oldcs)
				{
					cshift_empty = *oldcs;
					free (oldcs);
				}
			}
		}
		else
		{
			newcs.destcolor[0] = atoi(Cmd_Argv(1));
			newcs.destcolor[1] = atoi(Cmd_Argv(2));
			newcs.destcolor[2] = atoi(Cmd_Argv(3));
			newcs.percent = atoi(Cmd_Argv(4));

			if ((newcs.destcolor[0] == cshift_empty.cshift.destcolor[0]) &&
				(newcs.destcolor[1] == cshift_empty.cshift.destcolor[1]) &&
				(newcs.destcolor[2] == cshift_empty.cshift.destcolor[2]) &&
				(newcs.percent == cshift_empty.cshift.percent))
			{
			// if new cshift is the same as the current one, just increment
			// the rep_count instead of allocating a new copy:
				cshift_empty.rep_count++;
			}
			else
			{
			// create copy of current cshift, and make the new one current:
				oldcs = Q_malloc (sizeof(cshift_list_t));
				*oldcs = cshift_empty;
				cshift_empty.prev = oldcs;
				cshift_empty.cshift = newcs;
			}
		}

		return;
	}

	cshift_empty.cshift.destcolor[0] = atoi(Cmd_Argv(1));
	cshift_empty.cshift.destcolor[1] = atoi(Cmd_Argv(2));
	cshift_empty.cshift.destcolor[2] = atoi(Cmd_Argv(3));
	cshift_empty.cshift.percent = atoi(Cmd_Argv(4));
}

/*
==================
V_ResetCshifts (JDH)
==================
*/
void V_ResetCshifts (void)
{
	cshift_list_t *cs, *prev;
	
// free dynamically allocated list of previous cshifts, 
// and set current cshift to its original value
	cs = cshift_empty.prev;
	while (cs)
	{
		prev = cs->prev;
		if (!prev)
			cshift_empty = *cs;
		
		free (cs);
		cs = prev;
	}

	cshift_empty.rep_count = 0;

// JDH: fixes tint from Quoth's cross remaining after loading a saved game:
	cshift_empty.cshift.percent = 0;
}
		
/*
==================
V_BonusFlash_f

When you run over an item, the server sends this command
==================
*/
void V_BonusFlash_f (cmd_source_t src)
{
	if (!v_bonusflash.value || cl_demoseek)
		return;

	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	cl.cshifts[CSHIFT_BONUS].percent = 50;
}

#ifdef HEXEN2_SUPPORT

void V_DarkFlash_f (cmd_source_t src)
{
	if (cl_demoseek)	// JDH: don't want flashes accumulating while we're
		return;			//      jumping to a new demo position

	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 0;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 0;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 0;
	cl.cshifts[CSHIFT_BONUS].percent = 255;
}

void V_WhiteFlash_f (cmd_source_t src)
{
	if (cl_demoseek)
		return;

	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 255;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 255;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 255;
	cl.cshifts[CSHIFT_BONUS].percent = 255;
}

#endif	// #ifdef HEXEN2_SUPPORT

/*
=============
V_SetContentsColor

Underwater, lava, etc each has a color shift
=============
*/
void V_SetContentsColor (int contents)
{
	if (!v_contentblend.value)
	{
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty.cshift;
		cl.cshifts[CSHIFT_CONTENTS].percent *= 100;
		return;
	}

	switch (contents)
	{
	case CONTENTS_EMPTY:
	case CONTENTS_SOLID:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty.cshift;
		break;
	case CONTENTS_LAVA:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case CONTENTS_SLIME:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	default:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}

	if (v_contentblend.value > 0 && v_contentblend.value < 1 && contents != CONTENTS_EMPTY)
		cl.cshifts[CSHIFT_CONTENTS].percent *= v_contentblend.value;

	if (contents != CONTENTS_EMPTY)
	{
		if (!gl_polyblend.value)
			cl.cshifts[CSHIFT_CONTENTS].percent = 0;
		else
			cl.cshifts[CSHIFT_CONTENTS].percent *= gl_cshiftpercent.value;
	}
	else
	{
		cl.cshifts[CSHIFT_CONTENTS].percent *= 100;
	}
}

/*
=============
V_CalcPowerupCshift
=============
*/
void V_CalcPowerupCshift (void)
{
	float	fraction;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		int art = cl.v.artifact_active;

		if (art & ARTFLAG_DIVINE_INTERVENTION)
		{
			cl.cshifts[CSHIFT_BONUS].destcolor[0] = 255;
			cl.cshifts[CSHIFT_BONUS].destcolor[1] = 255;
			cl.cshifts[CSHIFT_BONUS].destcolor[2] = 255;
			cl.cshifts[CSHIFT_BONUS].percent = 256;
		}

		if( art & ARTFLAG_FROZEN)
		{
			cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 20;
			cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 70;
			cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
			cl.cshifts[CSHIFT_POWERUP].percent = 65;
		}
		else if (art & ARTFLAG_STONED)
		{
			cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 205;
			cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 205;
			cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 205;
			cl.cshifts[CSHIFT_POWERUP].percent = 80;
		//	cl.cshifts[CSHIFT_POWERUP].percent = 11000;
		}
		else if (art & ART_INVISIBILITY)
		{
			cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
			cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
			cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
			cl.cshifts[CSHIFT_POWERUP].percent = 100;
		}
		else if (art & ART_INVINCIBILITY)
		{
			cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
			cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
			cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
			cl.cshifts[CSHIFT_POWERUP].percent = 30;
		}
		else
		{
			cl.cshifts[CSHIFT_POWERUP].percent = 0;
		}

		return;
	}
#endif	// #ifdef HEXEN2_SUPPORT

	if (cl.items & IT_QUAD)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		fraction = bound(0, v_quadcshift.value, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = 30 * fraction;
	}
	else if (cl.items & IT_SUIT)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		fraction = bound(0, v_suitcshift.value, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = 20 * fraction;
	}
	else if (cl.items & IT_INVISIBILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		fraction = bound(0, v_ringcshift.value, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = 100 * fraction;
	}
	else if (cl.items & IT_INVULNERABILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		fraction = bound(0, v_pentcshift.value, 1);
		cl.cshifts[CSHIFT_POWERUP].percent = 30 * fraction;
	}
	else
	{
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
	}
}

/*
=============
V_CalcBlend
=============
*/
void V_CalcBlend (void)
{
	float	r, g, b, a, a2;
	int	j;

	r = g = b = a = 0;

	if (cls.state != ca_connected)
	{
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty.cshift;
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;
		cl.cshifts[CSHIFT_BONUS].percent = 0;
	}
	else
	{
		V_CalcPowerupCshift ();
	
		// drop the damage value
		cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime * 150;
		if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
			cl.cshifts[CSHIFT_DAMAGE].percent = 0;

		// drop the bonus value
		cl.cshifts[CSHIFT_BONUS].percent -= host_frametime * 100;
		if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
			cl.cshifts[CSHIFT_BONUS].percent = 0;
	}

	for (j=0 ; j<NUM_CSHIFTS ; j++)
	{
		if ((!gl_cshiftpercent.value || !gl_polyblend.value) && j != CSHIFT_CONTENTS)
			continue;

		a2 = cl.cshifts[j].percent / 100.0 / 255.0;
		if (j != CSHIFT_CONTENTS)
			a2 *= gl_cshiftpercent.value;

		if (!a2)
			continue;
		a += a2 * (1 - a);

		a2 /= a;
		r = r * (1 - a2) + cl.cshifts[j].destcolor[0] * a2;
		g = g * (1 - a2) + cl.cshifts[j].destcolor[1] * a2;
		b = b * (1 - a2) + cl.cshifts[j].destcolor[2] * a2;
	}

	v_blend[0] = r / 255.0;
	v_blend[1] = g / 255.0;
	v_blend[2] = b / 255.0;
	v_blend[3] = bound(0, a, 1);
}

void V_AddLightBlend (float r, float g, float b, float a2)
{
	float	a;

	if (!gl_polyblend.value || !gl_cshiftpercent.value || !v_dlightcshift.value)
		return;

	a2 = a2 * bound(0, v_dlightcshift.value, 1) * gl_cshiftpercent.value / 100.0;

	v_blend[3] = a = v_blend[3] + a2 * (1 - v_blend[3]);

	if (!a)
		return;

	a2 = a2 / a;

	v_blend[0] = v_blend[0] * (1 - a2) + r * a2;
	v_blend[1] = v_blend[1] * (1 - a2) + g * a2;
	v_blend[2] = v_blend[2] * (1 - a2) + b * a2;
}

/*
=============
V_UpdatePalette
=============
*/
void V_UpdatePalette (void)
{
	int				i, j, c;
	qboolean		newramp;
#ifdef NEWHWBLEND
	qboolean		wanted;
	static qboolean prev_wanted;
#endif
	static	float	prev_blend[4];
	float			a, rgb[3];
	float			gamma, contrast;
	static	float	old_gamma, old_contrast, old_hwblend;
	extern	float	vid_gamma;

	newramp = false;

#ifdef NEWHWBLEND
// 2010/03/30: now does cshifts here (via hw blending) only if console/menu aren't open
//			   (otherwise they get tinted too) - see also R_PolyBlend

	wanted = V_WANT_HWBLEND();

	if (wanted != prev_wanted)
	{
		prev_wanted = wanted;
		newramp = true;
	}
	
//	if (wanted)
#endif
	{
		for (i=0 ; i<4 ; i++)
		{
			if (v_blend[i] != prev_blend[i])
			{
				newramp = true;
				prev_blend[i] = v_blend[i];
			}
		}
	}

	gamma = bound(0.3, v_gamma.value, 3);
	if (gamma != old_gamma)
	{
		old_gamma = gamma;
		newramp = true;
	}

	contrast = bound(1, v_contrast.value, 3);
	if (contrast != old_contrast)
	{
		old_contrast = contrast;
		newramp = true;
	}

	if (gl_hwblend.value != old_hwblend)
	{
		newramp = true;
		old_hwblend = gl_hwblend.value;
	}

	if (!newramp)
		return;

	if (V_USING_HWRAMPS())
	{
//		Con_Printf ("Applying v_blend %.02f, %.02f, %.02f, %.02f\n", v_blend[0], v_blend[1], v_blend[2], v_blend[3]);
		
		a = v_blend[3];

		rgb[0] = 255 * v_blend[0] * a;
		rgb[1] = 255 * v_blend[1] * a;
		rgb[2] = 255 * v_blend[2] * a;

		a = 1 - a;
	}
	else
	{
		rgb[0] = rgb[1] = rgb[2] = 0;
		a = 1;
#ifdef NEWHWBLEND
//		contrast = gamma = 1;
#endif
	}

	if (vid_gamma != 1.0)
	{
		contrast = pow (contrast, vid_gamma);
		gamma /= vid_gamma;
	}

	for (i=0 ; i<256 ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			// apply blend and contrast
			c = (i*a + rgb[j]) * contrast;
			if (c > 255)
				c = 255;
			// apply gamma
			c = 255 * pow((c + 0.5)/255.5, gamma) + 0.5;
			c = bound(0, c, 255);
			ramps[j][i] = c << 8;
		}
	}

	VID_SetDeviceGammaRamp ((unsigned short *)ramps);
	Movie_UpdateColorRamps ();
}

/*
==============================================================================

				VIEW RENDERING

==============================================================================
*/

float angledelta (float a)
{
	a = anglemod(a);
	if (a > 180)
		a -= 360;
	return a;
}

/*
==================
CalcGunAngle
==================
*/
void CalcGunAngle (void)
{
	float		yaw, pitch, move;
	static	float	oldyaw = 0;
	static	float	oldpitch = 0;

	yaw = r_refdef.viewangles[YAW];
	pitch = -r_refdef.viewangles[PITCH];

	yaw = angledelta(yaw - r_refdef.viewangles[YAW]) * 0.4;
	yaw = bound(-10, yaw, 10);

	pitch = angledelta(-pitch - r_refdef.viewangles[PITCH]) * 0.4;
	pitch = bound(-10, pitch, 10);

	move = host_frametime * 20;
	if (yaw > oldyaw)
	{
		if (oldyaw + move < yaw)
			yaw = oldyaw + move;
	}
	else
	{
		if (oldyaw - move > yaw)
			yaw = oldyaw - move;
	}

	if (pitch > oldpitch)
	{
		if (oldpitch + move < pitch)
			pitch = oldpitch + move;
	}
	else
	{
		if (oldpitch - move > pitch)
			pitch = oldpitch - move;
	}

	oldyaw = yaw;
	oldpitch = pitch;

	cl.viewent.angles[YAW] = r_refdef.viewangles[YAW] + yaw;
	cl.viewent.angles[PITCH] = -(r_refdef.viewangles[PITCH] + pitch);
// joe: this makes it fix when strafing
	cl.viewent.angles[ROLL] = r_refdef.viewangles[ROLL];

	cl.viewent.angles[ROLL] -= v_idlescale.value * sin(cl.time*v_iroll_cycle.value) * v_iroll_level.value;
	cl.viewent.angles[PITCH] -= v_idlescale.value * sin(cl.time*v_ipitch_cycle.value) * v_ipitch_level.value;
	cl.viewent.angles[YAW] -= v_idlescale.value * sin(cl.time*v_iyaw_cycle.value) * v_iyaw_level.value;
}

/*
==============
V_BoundOffsets
==============
*/
void V_BoundOffsets (void)
{
	entity_t	*ent;

	ent = &cl_entities[cl.viewentity];

	// absolutely bound refresh reletive to entity clipping hull
	// so the view can never be inside a solid wall
	r_refdef.vieworg[0] = max(r_refdef.vieworg[0], ent->origin[0] - 14);
	r_refdef.vieworg[0] = min(r_refdef.vieworg[0], ent->origin[0] + 14);
	r_refdef.vieworg[1] = max(r_refdef.vieworg[1], ent->origin[1] - 14);
	r_refdef.vieworg[1] = min(r_refdef.vieworg[1], ent->origin[1] + 14);

#ifdef HEXEN2_SUPPORT
	if ( hexen2 )
	{
		r_refdef.vieworg[2] = max(r_refdef.vieworg[2], ent->origin[2]);
		r_refdef.vieworg[2] = min(r_refdef.vieworg[2], ent->origin[2] + 86);
		return;
	}
#endif

	r_refdef.vieworg[2] = max(r_refdef.vieworg[2], ent->origin[2] - 22);
	r_refdef.vieworg[2] = min(r_refdef.vieworg[2], ent->origin[2] + 30);
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle (void)
{
	r_refdef.viewangles[ROLL] += v_idlescale.value * sin(cl.time*v_iroll_cycle.value) * v_iroll_level.value;
	r_refdef.viewangles[PITCH] += v_idlescale.value * sin(cl.time*v_ipitch_cycle.value) * v_ipitch_level.value;
	r_refdef.viewangles[YAW] += v_idlescale.value * sin(cl.time*v_iyaw_cycle.value) * v_iyaw_level.value;
}

/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll (void)
{
	float	side;

#ifdef _DEBUG
	float *angles;

	if (cls.demoplayback && (cl.protocol == PROTOCOL_VERSION_QW))
		angles = cl.viewangles;
	else
		angles = cl_entities[cl.viewentity].angles;
	side = V_CalcRoll (angles, cl.velocity);
#else
	side = V_CalcRoll (cl_entities[cl.viewentity].angles, cl.velocity);
#endif
	
	r_refdef.viewangles[ROLL] += side;

	if (v_dmg_time > 0)
	{
		r_refdef.viewangles[ROLL] += v_dmg_time / v_kicktime.value * v_dmg_roll;
		r_refdef.viewangles[PITCH] += v_dmg_time / v_kicktime.value * v_dmg_pitch;
		v_dmg_time -= host_frametime;
	}

#ifdef HEXEN2_SUPPORT
	if ((hexen2) && (cl.v.health <= 0))
	{
		r_refdef.viewangles[ROLL] = 80;	// dead view angle
	}
#endif
}

#ifdef HEXEN2_SUPPORT

/*
===============
V_CalcRefdef_H2
===============
*/
void V_CalcRefdef_H2 (entity_t *ent, entity_t *view)
{
	static float oldz = 0;

	// Place weapon in powered up mode
	if ((ent->drawflags & MLS_MASKIN) == MLS_POWERMODE)
		view->drawflags = (view->drawflags & MLS_MASKOUT) | MLS_POWERMODE;
	else
		view->drawflags = (view->drawflags & MLS_MASKOUT) | 0;

// set up the refresh position
	VectorAdd (r_refdef.viewangles, cl.punchangle, r_refdef.viewangles);

// smooth out stair step ups
	if (cl.onground && ent->origin[2] - oldz > 0)
	{
		float steptime;

		steptime = cl.time - cl.oldtime;
		if (steptime < 0)
	//FIXME		I_Error ("steptime < 0");
			steptime = 0;

		oldz += steptime * 80;
		if (oldz > ent->origin[2])
			oldz = ent->origin[2];
		if (ent->origin[2] - oldz > 12)
			oldz = ent->origin[2] - 12;
		r_refdef.vieworg[2] += oldz - ent->origin[2];
		view->origin[2] += oldz - ent->origin[2];
	}
	else
		oldz = ent->origin[2];
}

#endif	// #ifdef HEXEN2_SUPPORT

/*
===============
V_AddViewWeapon
  (ent is player, view is weapon)
===============
*/
void V_AddViewWeapon (entity_t *ent, entity_t *view, float bob)
{
	int		i;
	vec3_t	forward, right, up;

	// angles
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		view->angles[YAW] = ent->angles[YAW];
		view->angles[PITCH] = -ent->angles[PITCH];
		view->angles[ROLL] = ent->angles[ROLL];

		AngleVectors (view->angles, forward, right, up);
	}
	else
#endif
	{
		view->angles[YAW] = r_refdef.viewangles[YAW];
		view->angles[PITCH] = -r_refdef.viewangles[PITCH];
		view->angles[ROLL] = r_refdef.viewangles[ROLL];

		AngleVectors (r_refdef.viewangles, forward, right, up);
	}

	// origin
	for (i=0 ; i<3 ; i++)
		r_refdef.vieworg[i] += scr_ofsx.value * forward[i] + scr_ofsy.value * right[i] + scr_ofsz.value * up[i];

	V_BoundOffsets ();

	// set up gun position
	VectorCopy (cl.viewangles, view->angles);
	CalcGunAngle ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		VectorCopy (ent->origin, view->origin);
		view->origin[2] += cl.viewheight;
		VectorMA (view->origin, bob * 0.4, forward, view->origin);
		view->origin[2] += bob;
	}
	else
#endif
	{
		VectorCopy (r_refdef.vieworg, view->origin);
		VectorMA (view->origin, bob * 0.4, forward, view->origin);
	}

	// fudge position around to keep amount of weapon visible roughly equal with different FOV
/*	if (scr_viewsize.value == 110)
		view->origin[2] += 1;
	else if (scr_viewsize.value == 100)
		view->origin[2] += 2;
	else if (scr_viewsize.value == 90)
		view->origin[2] += 1;
	else if (scr_viewsize.value == 80)
		view->origin[2] += 0.5;
*/
// JDH: this seems better to me:
/*	if (scr_viewsize.value >= 90)
		view->origin[2] += 1;
	else if (scr_viewsize.value == 80)
		view->origin[2] += 0.5;
*/
	view->model = cl.model_precache[cl.stats[STAT_WEAPON]];
	view->frame = cl.stats[STAT_WEAPONFRAME];
	view->colormap = vid.colormap;
}

/*
========================
V_CalcIntermissionRefdef
========================
*/
void V_CalcIntermissionRefdef (void)
{
	entity_t	*ent, *view;
	float		old;

	// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
	// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	VectorCopy (ent->origin, r_refdef.vieworg);
	VectorCopy (ent->angles, r_refdef.viewangles);
	view->model = NULL;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		r_refdef.vieworg[2] += cl.viewheight;
#endif

	// always idle in intermission
	old = v_idlescale.value;
	v_idlescale.value = 1;
	V_AddIdle ();
	v_idlescale.value = old;
}


float	punchangle = 0;
/*
==================
V_CalcRefdef
==================
*/
void V_CalcRefdef (void)
{
	entity_t	*ent, *view;
	vec3_t		forward;
	float		bob;

			/****** TEST ONLY!! REMOVE!! ******/
#ifdef _DEBUG
/*	if (cl.protocol == PROTOCOL_VERSION_QW)
	{
		ent = &cl_entities[cl.viewentity];
		view = &cl.viewent;

		//VectorClear (cl.viewangles);
		ent->angles[YAW] = cl.viewangles[YAW];		// the model should face the view dir
		ent->angles[PITCH] = -cl.viewangles[PITCH];	//

		VectorClear (r_refdef.vieworg);
	//	VectorCopy (ent->origin, r_refdef.vieworg);
	//	r_refdef.vieworg[2] += cl.viewheight;
	//	r_refdef.vieworg[2] = 0;
		VectorCopy (cl.viewangles, r_refdef.viewangles);
		return;
	}
*/
#endif
			/****** TEST ONLY!! REMOVE!! ******/




#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (!cl.v.cameramode)
		{
			V_DriftPitch ();
			V_DriftRoll ();
		}
	}
	else
#endif
	V_DriftPitch ();

	// ent is the player model (visible when out of body)
	ent = &cl_entities[cl.viewentity];
	// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	// transform the view offset by the model's matrix to get the offset from
	// model origin for the view
	ent->angles[YAW] = cl.viewangles[YAW];		// the model should face the view dir
	ent->angles[PITCH] = -cl.viewangles[PITCH];	//

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (cl.v.movetype == MOVETYPE_FLY))
		bob = 1;	// no bobbing when you fly
	else
#endif
	bob = V_CalcBob ();

	// set up the refresh position
	VectorCopy (ent->origin, r_refdef.vieworg);
	r_refdef.vieworg[2] += cl.viewheight + bob;

	// never let it sit exactly on a node line, because a water plane can
	// dissapear when viewed with the eye exactly on it.
	// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis
	r_refdef.vieworg[0] += 1.0 / 32;
	r_refdef.vieworg[1] += 1.0 / 32;
	r_refdef.vieworg[2] += 1.0 / 32;

	// add view height
#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
		r_refdef.vieworg[2] += cl.crouch;	// smooth out stair step ups

	// set up refresh view angles
	VectorCopy (cl.viewangles, r_refdef.viewangles);
	V_CalcViewRoll ();
	V_AddIdle ();

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	{
		if (v_gunkick.value)
		{
			// add weapon kick offset
			AngleVectors (r_refdef.viewangles, forward, NULL, NULL);
			VectorMA (r_refdef.vieworg, punchangle, forward, r_refdef.vieworg);

			// add weapon kick angle
			r_refdef.viewangles[PITCH] += punchangle * 0.5;
		}

		if (cl.stats[STAT_HEALTH] <= 0)
			r_refdef.viewangles[ROLL] = 80;	// dead view angle
	}

	V_AddViewWeapon (ent, view, bob);


#ifdef HEXEN2_SUPPORT
	if (hexen2)
		V_CalcRefdef_H2 (ent, view);
#endif

	if (chase_active.value)
		Chase_Update ();
}

void DropPunchAngle (void)
{
			/****** TEST ONLY!! REMOVE!! ******/
#ifdef _DEBUG
/*	if (cl.protocol == PROTOCOL_VERSION_QW)
	{
		VectorClear (cl.punchangle);
		return;
	}
*/
#endif
			/****** TEST ONLY!! REMOVE!! ******/


	
	if (cl.punchangle[0] < punchangle)
	{
		if (cl.punchangle[0] == -2)		// small kick
			punchangle -= 20 * host_frametime;
		else					// big kick
			punchangle -= 40 * host_frametime;

		if (punchangle < cl.punchangle[0])
		{
			punchangle = cl.punchangle[0];
			cl.punchangle[0] = 0;
		}
	}
	else
	{
		punchangle += 20 * host_frametime;
		if (punchangle > 0)
			punchangle = 0;
	}
}

/*
==================
V_RenderView

The player's clipping box goes from (-16 -16 -24) to (16 16 32) from
the entity origin, so any view position inside that will be valid
==================
*/
void V_RenderView (qboolean force_refdef_recalc)
{
	if (cls.state != ca_connected)
	{
		V_CalcBlend ();
		return;
	}

	if (con_forcedup)
		return;

// don't allow cheats in multiplayer
	if (cl.maxclients > 1)
	{
		Cvar_SetDirect (&scr_ofsx, "0");
		Cvar_SetDirect (&scr_ofsy, "0");
		Cvar_SetDirect (&scr_ofsz, "0");
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	{
/*		if (cls.state != ca_connected)
		{
			V_CalcBlend ();
			return;
		}
*/
		DropPunchAngle ();
	}

	if (cl.intermission)	// intermission / finale rendering
	{
		V_CalcIntermissionRefdef ();
	}
	else
	{
		// JDH: added recalc check for demos that contain svc_pause in 1st frame
		if (!cl.paused || force_refdef_recalc)
			V_CalcRefdef ();
	}

	R_PushDlights ();

	R_RenderView ();
}

#endif		//#ifndef RQM_SV_ONLY

//============================================================================

/*
=============
V_Init
=============
*/
void V_Init (void)
{
	Cvar_Register (&cl_rollspeed);
	Cvar_Register (&cl_rollangle);

#ifndef RQM_SV_ONLY
	Cmd_AddCommand ("v_cshift", V_cshift_f, 0);
	Cmd_AddCommand ("bf", V_BonusFlash_f, 0);
	Cmd_AddCommand ("centerview", V_StartPitchDrift_f, 0);

//#ifndef GLQUAKE
//	Cvar_RegisterVariable (&lcd_x);
//	Cvar_RegisterVariable (&lcd_yaw);
//#endif

	Cvar_Register (&v_centermove);
	Cvar_Register (&v_centerspeed);

	Cvar_Register (&v_iyaw_cycle);
	Cvar_Register (&v_iroll_cycle);
	Cvar_Register (&v_ipitch_cycle);
	Cvar_Register (&v_iyaw_level);
	Cvar_Register (&v_iroll_level);
	Cvar_Register (&v_ipitch_level);
	Cvar_Register (&v_idlescale);

	Cvar_Register (&scr_ofsx);
	Cvar_Register (&scr_ofsy);
	Cvar_Register (&scr_ofsz);

	Cvar_Register (&cl_bob);
	Cvar_Register (&cl_bobcycle);
	Cvar_Register (&cl_bobup);

	Cvar_Register (&v_kicktime);
	Cvar_Register (&v_kickroll);
	Cvar_Register (&v_kickpitch);

	Cvar_RegisterBool (&v_gunkick);

	Cvar_RegisterBool (&v_bonusflash);
	Cvar_RegisterFloat (&v_contentblend, 0, 1);
	Cvar_RegisterFloat (&v_damagecshift, 0, 1);
	Cvar_RegisterFloat (&v_quadcshift, 0, 1);
	Cvar_RegisterFloat (&v_suitcshift, 0, 1);
	Cvar_RegisterFloat (&v_ringcshift, 0, 1);
	Cvar_RegisterFloat (&v_pentcshift, 0, 1);
	Cvar_RegisterFloat (&v_dlightcshift, 0, 1);
	Cvar_Register (&gl_cshiftpercent);
#ifdef NEWHWBLEND
	Cvar_RegisterInt (&gl_hwblend, 0, 2);
#else
	Cvar_RegisterInt (&gl_hwblend, 0, 1);
#endif

	Cvar_RegisterFloat (&v_gamma, 0.3, 3);
	Cvar_RegisterFloat (&v_contrast, 1, 3);

	Cmd_AddLegacyCommand ("gamma", v_gamma.name);
	Cmd_AddLegacyCommand ("contrast", v_contrast.name);
#endif
}
