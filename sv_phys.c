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
// sv_phys.c

#include "quakedef.h"

/*

pushmove objects do not obey gravity, and do not interact with each other or
trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.
it is set for steping or walking objects 

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/

cvar_t	sv_friction    = {"sv_friction",       "4", CVAR_FLAG_SERVER};
cvar_t	sv_stopspeed   = {"sv_stopspeed",    "100"};
cvar_t	sv_gravity     = {"sv_gravity",      "800", CVAR_FLAG_SERVER};
cvar_t	sv_maxvelocity = {"sv_maxvelocity", "2000"};
cvar_t	sv_nostep      = {"sv_nostep",         "0"};

#define	MOVE_EPSILON	0.01

extern	cvar_t		/*sv_oldprotocol,*/ host_cutscenehack, sv_fishfix, sv_imp12hack;
extern	qboolean	pr_handles_imp12;
extern	int			eval_attack_finished;

void SV_Physics_Toss (edict_t *ent);

/*
================
SV_CheckAllEnts
================
*/
void SV_CheckAllEnts (void)
{
	int		e;
	edict_t		*check;

// see if any solid entities are inside the final position
	check = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
		|| check->v.movetype == MOVETYPE_NONE
		|| check->v.movetype == MOVETYPE_NOCLIP)
			continue;

		if (SV_TestEntityPosition (check))
			Con_Print ("entity in invalid position\n");
	}
}

/*
================
SV_CheckVelocity
================
*/
void SV_CheckVelocity (edict_t *ent)
{
	int	i;

// bound velocity
	for (i=0 ; i<3 ; i++)
	{
		if (IS_NAN(ent->v.velocity[i]))
		{
			Con_Printf ("Got a NaN velocity on %s\n", pr_strings + ent->v.classname);
			ent->v.velocity[i] = 0;
		}
		if (IS_NAN(ent->v.origin[i]))
		{
			Con_Printf ("Got a NaN origin on %s\n", pr_strings + ent->v.classname);
			ent->v.origin[i] = 0;
		}
		if (ent->v.velocity[i] > sv_maxvelocity.value)
			ent->v.velocity[i] = sv_maxvelocity.value;
		else if (ent->v.velocity[i] < -sv_maxvelocity.value)
			ent->v.velocity[i] = -sv_maxvelocity.value;
	}
}

/*
=============
SV_RunThink

Runs thinking code if time.  There is some play in the exact time the think
function will be called, because it is called before any movement is done
in a frame.  Not used for pushmove objects, because they must be exact.
Returns false if the entity removed itself.
=============
*/
qboolean SV_RunThink (edict_t *ent)
{
	float	thinktime;
	int		oldcount;

	thinktime = ent->v.nextthink;
	if (thinktime <= 0 || thinktime > sv.time + host_frametime)
		return true;
		
	if (thinktime < sv.time)
		thinktime = sv.time;	// don't let things stay in the past.
								// it is possible to start that way
								// by a trigger with a local time.
	ent->v.nextthink = 0;

	PR_GLOBAL(time) = thinktime;
	PR_GLOBAL(self) = EDICT_TO_PROG(ent);
	PR_GLOBAL(other) = EDICT_TO_PROG(sv.edicts);

// JDH: the extra 1 added to total_monsters by monster_fish happens 
//      in swimmonster_start_go, during the first frame (sv.time = 1). 
//      But fix it only if total_monsters was increased when fish were 
//      spawned (ie. if sv.fish_counted is true)

	if ((sv.time == 1.0) && sv_fishfix.value && sv.fish_counted && 
		!strcmp(pr_strings + ent->v.classname, "monster_fish") &&
		!strcmp(pr_functions[ent->v.think].s_name, "swimmonster_start_go"))
	{
		oldcount = PR_GLOBAL(total_monsters);		
	}
	else oldcount = -1;
	
	PR_ExecuteProgram (ent->v.think);
	
	if (oldcount != -1)
	{
		if ((int)PR_GLOBAL(total_monsters) - oldcount == 1)
		{
			PR_GLOBAL(total_monsters) -= 1;
			if (sv.fish_counted == 1)
			{
				Con_Print ("Detected fish-count bug in progs.dat; monster count has been adjusted\n");
				sv.fish_counted++;
			}
		}
	}

	return !ent->free;
}

/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
void SV_Impact (edict_t *e1, edict_t *e2)
{
	int	old_self, old_other;
	
	old_self = PR_GLOBAL(self);
	old_other = PR_GLOBAL(other);
	
	PR_GLOBAL(time) = sv.time;
	if (e1->v.touch && e1->v.solid != SOLID_NOT)
	{
		PR_GLOBAL(self) = EDICT_TO_PROG(e1);
		PR_GLOBAL(other) = EDICT_TO_PROG(e2);
		PR_ExecuteProgram (e1->v.touch);
	}
	
	if (e2->v.touch && e2->v.solid != SOLID_NOT)
	{
		PR_GLOBAL(self) = EDICT_TO_PROG(e2);
		PR_GLOBAL(other) = EDICT_TO_PROG(e1);
		PR_ExecuteProgram (e2->v.touch);
	}

	PR_GLOBAL(self) = old_self;
	PR_GLOBAL(other) = old_other;
}


/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1

int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff, change;
	int	i, blocked;
	
	blocked = 0;
	if (normal[2] > 0)
		blocked |= 1;		// floor
	if (!normal[2])
		blocked |= 2;		// step
	
	backoff = DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	
	return blocked;
}


/*
============
SV_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
If steptrace is not NULL, the trace of any vertical wall hit will be stored
============
*/
#define	MAX_CLIP_PLANES	5
int SV_FlyMove (edict_t *ent, float time, trace_t *steptrace)
{
	int		i, j, bumpcount, numbumps, numplanes, blocked;
	vec3_t		dir, planes[MAX_CLIP_PLANES], primal_velocity, original_velocity, new_velocity, end;
	float		d, time_left;
	trace_t		trace;
	
	numbumps = 4;
	
	blocked = 0;
	VectorCopy (ent->v.velocity, original_velocity);
	VectorCopy (ent->v.velocity, primal_velocity);
	numplanes = 0;
	
	time_left = time;

	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		if (!ent->v.velocity[0] && !ent->v.velocity[1] && !ent->v.velocity[2])
			break;

		for (i=0 ; i<3 ; i++)
			end[i] = ent->v.origin[i] + time_left * ent->v.velocity[i];

		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

		if (trace.allsolid)
		{	// entity is trapped in another solid
			VectorCopy (vec3_origin, ent->v.velocity);
			return 3;
		}

		if (trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy (trace.endpos, ent->v.origin);
			VectorCopy (ent->v.velocity, original_velocity);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			 break;		// moved the entire distance

		if (!trace.ent)
			Sys_Error ("SV_FlyMove: !trace.ent");

		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
			if (trace.ent->v.solid == SOLID_BSP)
			{
				ent->v.flags =	(int)ent->v.flags | FL_ONGROUND;
				ent->v.groundentity = EDICT_TO_PROG(trace.ent);
			}
		}
		if (!trace.plane.normal[2])
		{
			blocked |= 2;		// step
			if (steptrace)
				*steptrace = trace;	// save for player extrafriction
		}

// run the impact function
		SV_Impact (ent, trace.ent);
		if (ent->free)
			break;		// removed by the impact function

		time_left -= time_left * trace.fraction;
		
	// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			VectorCopy (vec3_origin, ent->v.velocity);
			return 3;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

// modify original_velocity so it parallels all of the clip planes
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity (original_velocity, planes[i], new_velocity, 1);
			for (j=0 ; j<numplanes ; j++)
				if (j != i)
				{
					if (DotProduct (new_velocity, planes[j]) < 0)
						break;	// not ok
				}
			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
		{	// go along this plane
			VectorCopy (new_velocity, ent->v.velocity);
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
//				Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
				VectorCopy (vec3_origin, ent->v.velocity);
				return 7;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, ent->v.velocity);
			VectorScale (dir, d, ent->v.velocity);
		}

// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
		if (DotProduct (ent->v.velocity, primal_velocity) <= 0)
		{
			VectorCopy (vec3_origin, ent->v.velocity);
			return blocked;
		}
	}

	return blocked;
}

#ifdef HEXEN2_SUPPORT
/*
============
SV_FlyExtras
============
*/
void SV_FlyExtras (edict_t *ent, float time, trace_t *steptrace)
{
const float hoverinc = 0.4;

	ent->v.flags = (int) ent->v.flags | FL_ONGROUND;  // Jumping makes you loose this flag so reset it

	if ((ent->v.velocity[2]<=6) && (ent->v.velocity[2]>=-6))
	{
		ent->v.velocity[2] += ent->v.hoverz;

		if (ent->v.velocity[2] >= 6)
		{
			ent->v.hoverz = -hoverinc;
			ent->v.velocity[2] += ent->v.hoverz;
		}
		else if (ent->v.velocity[2] <= -6)
		{
			ent->v.hoverz = hoverinc;
			ent->v.velocity[2] += ent->v.hoverz;
		}
	}
	else  // friction for upward or downward progress once key is released
	{
		ent->v.velocity[2]-=sv_player->v.velocity[2] * .1;
	}

}
#endif	// #ifdef HEXEN2_SUPPORT

/*
============
SV_AddGravity

============
*/
void SV_AddGravity (edict_t *ent)
{
	float	ent_gravity;

	eval_t	*val;

	val = GETEDICTFIELD(ent, eval_gravity);
	if (val && val->_float)
		ent_gravity = val->_float;
	else
		ent_gravity = 1.0;
	ent->v.velocity[2] -= ent_gravity * sv_gravity.value * host_frametime;
}


/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entity's velocity at all
============
*/
trace_t SV_PushEntity (edict_t *ent, vec3_t push)
{
	trace_t	trace;
	vec3_t	end;
	int movetype;
		
	VectorAdd (ent->v.origin, push, end);

	if (ent->v.movetype == MOVETYPE_FLYMISSILE)
		movetype = MOVE_MISSILE;

#ifdef HEXEN2_SUPPORT
	else if (hexen2 && (ent->v.movetype == MOVETYPE_BOUNCEMISSILE))
		movetype = MOVE_MISSILE;
#endif
	else if (ent->v.solid == SOLID_TRIGGER || ent->v.solid == SOLID_NOT)
		movetype = MOVE_NOMONSTERS;		// only clip against bmodels

#ifdef HEXEN2_SUPPORT
	else if (hexen2 && (ent->v.movetype == MOVETYPE_SWIM))
		movetype = MOVE_WATER;
#endif
	else
		movetype = MOVE_NORMAL;
	
	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, movetype, ent);	
	
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (ent->v.solid != SOLID_PHASE)
		{
			if ((ent->v.movetype == MOVETYPE_BOUNCE) && !(trace.allsolid == 0 && trace.startsolid == 0))
			{
				trace.fraction = 0;
				return trace;
			}
		}
		else	// Entity is PHASED so bounce off walls and other entities, go through monsters and players
		{
			if (trace.ent)
			{	// Go through MONSTERS and PLAYERS, can't use FL_CLIENT cause rotating brushes do
				if (((int) trace.ent->v.flags & FL_MONSTER) || (trace.ent->v.movetype == MOVETYPE_WALK))
				{
					vec3_t  impact;
					edict_t *impact_e;
					
					VectorCopy (trace.endpos, impact);
					impact_e = trace.ent;

					trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_PHASE, ent);

					VectorCopy (impact, ent->v.origin);
					SV_Impact (ent, impact_e);
				}
			}
		}
	}
#endif

	VectorCopy (trace.endpos, ent->v.origin);
	
#ifdef _DEBUG
	if (!VectorCompare(trace.endpos, end))
		movetype *= 1;
#endif
	
	SV_LinkEdict (ent, true);

	if (trace.ent)
		SV_Impact (ent, trace.ent);		

	return trace;
}					


/*
============
SV_PushMove

============
*/
void SV_PushMove (edict_t *pusher, float movetime)
{
	int			i, e;
	edict_t		*check, *block;
	vec3_t		mins, maxs, move;
	vec3_t		entorig, pushorig;
	int			num_moved;
	edict_t		*moved_edict[MAX_EDICTS];
	vec3_t		moved_from[MAX_EDICTS];

	if (!pusher->v.velocity[0] && !pusher->v.velocity[1] && !pusher->v.velocity[2])
	{
		pusher->v.ltime += movetime;
		return;
	}

	for (i=0 ; i<3 ; i++)
	{
		move[i] = pusher->v.velocity[i] * movetime;
		mins[i] = pusher->v.absmin[i] + move[i];
		maxs[i] = pusher->v.absmax[i] + move[i];
	}

	VectorCopy (pusher->v.origin, pushorig);
	
// move the pusher to it's final position

	VectorAdd (pusher->v.origin, move, pusher->v.origin);
	pusher->v.ltime += movetime;
	SV_LinkEdict (pusher, false);


// see if any solid entities are inside the final position
	num_moved = 0;
	check = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
		|| check->v.movetype == MOVETYPE_NONE
		|| check->v.movetype == MOVETYPE_NOCLIP)
			continue;

	// if the entity is standing on the pusher, it will definately be moved
		if (!(((int)check->v.flags & FL_ONGROUND)
		&& PROG_TO_EDICT(check->v.groundentity) == pusher))
		{
			if (check->v.absmin[0] >= maxs[0]
			|| check->v.absmin[1] >= maxs[1]
			|| check->v.absmin[2] >= maxs[2]
			|| check->v.absmax[0] <= mins[0]
			|| check->v.absmax[1] <= mins[1]
			|| check->v.absmax[2] <= mins[2])
				continue;

		// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

	// remove the onground flag for non-players
		if (check->v.movetype != MOVETYPE_WALK)
			check->v.flags = (int)check->v.flags & ~FL_ONGROUND;
		
		VectorCopy (check->v.origin, entorig);
		VectorCopy (check->v.origin, moved_from[num_moved]);
		moved_edict[num_moved] = check;
		num_moved++;

		// try moving the contacted entity 
		pusher->v.solid = SOLID_NOT;
		SV_PushEntity (check, move);
		pusher->v.solid = SOLID_BSP;

	// if it is still inside the pusher, block
		block = SV_TestEntityPosition (check);
		if (block)
		{	// fail the move
			if (check->v.mins[0] == check->v.maxs[0])
				continue;
			if (check->v.solid == SOLID_NOT || check->v.solid == SOLID_TRIGGER)
			{	// corpse
				check->v.mins[0] = check->v.mins[1] = 0;
				VectorCopy (check->v.mins, check->v.maxs);
				continue;
			}
			
			VectorCopy (entorig, check->v.origin);
			SV_LinkEdict (check, true);

			VectorCopy (pushorig, pusher->v.origin);
			SV_LinkEdict (pusher, false);
			pusher->v.ltime -= movetime;

			// if the pusher has a "blocked" function, call it
			// otherwise, just stay in place until the obstacle is gone
			if (pusher->v.blocked)
			{
				PR_GLOBAL(self) = EDICT_TO_PROG(pusher);
				PR_GLOBAL(other) = EDICT_TO_PROG(check);

				PR_ExecuteProgram (pusher->v.blocked);
			}
			
		// move back any entities we already moved
			for (i=0 ; i<num_moved ; i++)
			{
				VectorCopy (moved_from[i], moved_edict[i]->v.origin);
				SV_LinkEdict (moved_edict[i], false);
			}
			return;
		}	
	}	
}

#ifdef HEXEN2_SUPPORT
/*
============
SV_PushRotate
============
*/
void SV_PushRotate (edict_t *pusher, float movetime)
{
	int			i, e, t, num_moved, slaves_moved;
	edict_t		*check, *block, *ground, *master, *slave;
	edict_t		*moved_edict[MAX_EDICTS];
	vec3_t		moved_from[MAX_EDICTS];
	vec3_t		move, a, amove, mins, maxs, move2, move3, testmove;
	vec3_t		entorig, pushorig, pushorigangles;
	vec3_t		org, org2, check_center;
	vec3_t		forward, right, up;
	qboolean	moveit;

	for (i=0 ; i<3 ; i++)
	{
		amove[i] = pusher->v.avelocity[i] * movetime;
		move[i] = pusher->v.velocity[i] * movetime;
		mins[i] = pusher->v.absmin[i] + move[i];
		maxs[i] = pusher->v.absmax[i] + move[i];
	}

	VectorSubtract (vec3_origin, amove, a);
	AngleVectors (a, forward, right, up);

	VectorCopy (pusher->v.origin, pushorig);
	VectorCopy (pusher->v.angles, pushorigangles);
	
// move the pusher to it's final position

	VectorAdd (pusher->v.origin, move, pusher->v.origin);
	VectorAdd (pusher->v.angles, amove, pusher->v.angles);

	pusher->v.ltime += movetime;
	SV_LinkEdict (pusher, false);

	master = pusher;
	slaves_moved = 0;

// see if any solid entities are inside the final position
	num_moved = 0;
	check = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH || check->v.movetype == MOVETYPE_NONE || 
		    check->v.movetype == MOVETYPE_FOLLOW || check->v.movetype == MOVETYPE_NOCLIP)
			continue;

		// if the entity is standing on the pusher, it will definitely be moved
		moveit = false;
		ground = PROG_TO_EDICT(check->v.groundentity);
		if ((int)check->v.flags & FL_ONGROUND)
		{
			if (ground == pusher)
			{
				moveit = true;
			}
			else
			{
				for (i=0; i<slaves_moved; i++)
				{
					if (ground == moved_edict[MAX_EDICTS - i - 1])
					{
						moveit = true;
						break;
					}
				}
			}
		}

		if (!moveit)
		{
			if ( check->v.absmin[0] >= maxs[0] || check->v.absmin[1] >= maxs[1] ||
				 check->v.absmin[2] >= maxs[2] || check->v.absmax[0] <= mins[0] ||
			     check->v.absmax[1] <= mins[1] || check->v.absmax[2] <= mins[2] )
			{
				for (i=0; i<slaves_moved; i++)
				{
					slave = moved_edict[MAX_EDICTS - i - 1];
					if ( check->v.absmin[0] >= slave->v.absmax[0]
					|| check->v.absmin[1] >= slave->v.absmax[1]
					|| check->v.absmin[2] >= slave->v.absmax[2]
					|| check->v.absmax[0] <= slave->v.absmin[0]
					|| check->v.absmax[1] <= slave->v.absmin[1]
					|| check->v.absmax[2] <= slave->v.absmin[2] )
						continue;
				}
				if (i == slaves_moved)
					continue;
			}

		// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

		// remove the onground flag for non-players
		if (check->v.movetype != MOVETYPE_WALK)
			check->v.flags = (int)check->v.flags & ~FL_ONGROUND;
		
		VectorCopy (check->v.origin, entorig);
		VectorCopy (check->v.origin, moved_from[num_moved]);
		moved_edict[num_moved] = check;
		num_moved++;

//put check in first move spot
		VectorAdd (check->v.origin, move, check->v.origin);
//Use center of model, like in QUAKE!!!!  Our origins are on the bottom!!!
		for (i=0 ; i<3 ; i++)
			check_center[i] = (check->v.absmin[i] + check->v.absmax[i])/2;
// calculate destination position
		VectorSubtract (check_center, pusher->v.origin, org);
//put check back
		VectorSubtract (check->v.origin, move, check->v.origin);
		org2[0] = DotProduct (org, forward);
		org2[1] = -DotProduct (org, right);
		org2[2] = DotProduct (org, up);
		VectorSubtract (org2, org, move2);

		//Add all moves together
		VectorAdd(move,move2,move3);

		// try moving the contacted entity 
		for( t = 0; t < 13; t++)
		{
			switch(t)
			{
				case 0:
				//try x, y and z
					VectorCopy(move3,testmove);
					break;
				case 1:
				//Try xy only
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=move3[0];
					testmove[1]=move3[1];
					testmove[2]=0;
					break;
				case 2:
				//Try z only
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=0;
					testmove[1]=0;
					testmove[2]=move3[2];
					break;
				case 3:
				//Try none
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=0;
					testmove[1]=0;
					testmove[2]=0;
					break;
				case 4:
				//Try xy in opposite dir
					testmove[0]=move3[0]*-1;
					testmove[1]=move3[1]*-1;
					testmove[2]=move3[2];
					break;
				case 5:
				//Try z in opposite dir
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=move3[0];
					testmove[1]=move3[1];
					testmove[2]=move3[2]*-1;
					break;
				case 6:
				//Try xyz in opposite dir
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=move3[0]*-1;
					testmove[1]=move3[1]*-1;
					testmove[2]=move3[2]*-1;
					break;
				case 7:
				//Try move3 times 2
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					VectorScale(move3,2,testmove);
					break;
				case 8:
				//Try normalized org
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					VectorScale(org,movetime,org);//movetime*20?
					VectorCopy(org,testmove);
					break;
				case 9:
				//Try normalized org z * 3 only
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=0;
					testmove[1]=0;
					testmove[2]=org[2]*3;//was: +org[2]*(fastfabs(org[1])+fastfabs(org[2]));
					break;
				case 10:
				//Try normalized org xy * 2 only
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=org[0]*2;//was: +org[0]*fastfabs(org[2]);
					testmove[1]=org[1]*2;//was: +org[1]*fastfabs(org[2]);
					testmove[2]=0;
					break;
				case 11:
				//Try xy in opposite org dir
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=org[0]*-2;
					testmove[1]=org[1]*-2;
					testmove[2]=org[2];
					break;
				case 12:
				//Try z in opposite dir
					VectorSubtract(check->v.origin,testmove,check->v.origin);
					testmove[0]=org[0];
					testmove[1]=org[1];
					testmove[2]=org[2]*-3;
					break;
			}

			if(t!=3)
			{
				//THIS IS VERY BAD BAD HACK...
				pusher->v.solid = SOLID_NOT;
				SV_PushEntity (check, move3);
				//@@TODO: do we ever want to do anybody's angles?  maybe just yaw???
				//		if (!((int)check->v.flags & (FL_CLIENT | FL_MONSTER)))
				//			VectorAdd (check->v.angles, amove, check->v.angles);
				check->v.angles[YAW] += amove[YAW];
				pusher->v.solid = SOLID_BSP;
			}
			// if it is still inside the pusher, block
			block = SV_TestEntityPosition (check);
			if(!block)
				break;
		}

		if (block)
		{	// fail the move
			//			Con_DPrintf("Check blocked\n");
			if (check->v.mins[0] == check->v.maxs[0])
				continue;
			if (check->v.solid == SOLID_NOT || check->v.solid == SOLID_TRIGGER)
			{	// corpse
				check->v.mins[0] = check->v.mins[1] = 0;
				VectorCopy (check->v.mins, check->v.maxs);
				continue;
			}
			
			VectorCopy (entorig, check->v.origin);
			SV_LinkEdict (check, true);
			
			VectorCopy (pushorig, pusher->v.origin);
			VectorCopy (pushorigangles, pusher->v.angles);
			SV_LinkEdict (pusher, false);
			pusher->v.ltime -= movetime;
			
			for (i=0; i<slaves_moved; i++)
			{
				slave = moved_edict[MAX_EDICTS - i - 1];
				VectorCopy (moved_from[MAX_EDICTS - i - 1], slave->v.angles);
				SV_LinkEdict (slave, false);
				slave->v.ltime -= movetime;
			}
			
			// if the pusher has a "blocked" function, call it
			// otherwise, just stay in place until the obstacle is gone
			if (pusher->v.blocked)
			{
				*pr_global_ptrs.self = EDICT_TO_PROG(pusher);
				*pr_global_ptrs.other = EDICT_TO_PROG(check);
				PR_ExecuteProgram (pusher->v.blocked);
			}
			
			// move back any entities we already moved
			for (i=0 ; i<num_moved ; i++)
			{
				VectorCopy (moved_from[i], moved_edict[i]->v.origin);
				//@@TODO:: see above
				//				if (!((int)moved_edict[i]->v.flags & (FL_CLIENT | FL_MONSTER)))
				//					VectorSubtract (moved_edict[i]->v.angles, amove, moved_edict[i]->v.angles);
				moved_edict[i]->v.angles[YAW] -= amove[YAW];
				
				SV_LinkEdict (moved_edict[i], false);
			}
			return;
		}
	}
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
================
SV_Physics_Pusher

================
*/
void SV_Physics_Pusher (edict_t *ent)
{
	float	thinktime;
	float	oldltime;
	float	movetime;

	oldltime = ent->v.ltime;
	
#ifdef _DEBUG
	if (!ent->v.classname && ent->v.think)
		movetime = 0;
#endif
	
	thinktime = ent->v.nextthink;
	if (thinktime < ent->v.ltime + host_frametime)
	{
		movetime = thinktime - ent->v.ltime;
		if (movetime < 0)
			movetime = 0;
	}
	else
		movetime = host_frametime;

	if (movetime)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2 && (ent->v.avelocity[0] || ent->v.avelocity[1] || ent->v.avelocity[2]))
		{
			SV_PushRotate (ent, movetime);
		}
		else
	#endif
		SV_PushMove (ent, movetime);	// advances ent->v.ltime if not blocked
	}
		
	if (thinktime > oldltime && thinktime <= ent->v.ltime)
	{
		ent->v.nextthink = 0;

		PR_GLOBAL(time) = sv.time;
		PR_GLOBAL(self) = EDICT_TO_PROG(ent);
		PR_GLOBAL(other) = EDICT_TO_PROG(sv.edicts);

		PR_ExecuteProgram (ent->v.think);
		if (ent->free)
			return;
	}
}


/*
===============================================================================

CLIENT MOVEMENT

===============================================================================
*/

/*
=============
SV_CheckStuck

This is a big hack to try and fix the rare case of getting stuck in the world
clipping hull.
=============
*/
void SV_CheckStuck (edict_t *ent)
{
	int		i, j;
	int		z;
	vec3_t	org;

	if (!SV_TestEntityPosition(ent))
	{
		VectorCopy (ent->v.origin, ent->v.oldorigin);
		return;
	}

	VectorCopy (ent->v.origin, org);
	VectorCopy (ent->v.oldorigin, ent->v.origin);
	if (!SV_TestEntityPosition(ent))
	{
		Con_DPrintf ("Unstuck.\n");
		SV_LinkEdict (ent, true);
		return;
	}
	
	for (z=0 ; z< 18 ; z++)
		for (i=-1 ; i <= 1 ; i++)
			for (j=-1 ; j <= 1 ; j++)
			{
				ent->v.origin[0] = org[0] + i;
				ent->v.origin[1] = org[1] + j;
				ent->v.origin[2] = org[2] + z;
				if (!SV_TestEntityPosition(ent))
				{
					Con_DPrintf ("Unstuck.\n");
					SV_LinkEdict (ent, true);
					return;
				}
			}
			
	VectorCopy (org, ent->v.origin);
	Con_DPrintf ("player is stuck.\n");
}


/*
=============
SV_CheckWater
=============
*/
qboolean SV_CheckWater (edict_t *ent)
{
	vec3_t	point;
	int		cont;

	point[0] = ent->v.origin[0];
	point[1] = ent->v.origin[1];
	point[2] = ent->v.origin[2] + ent->v.mins[2] + 1;	
	
	ent->v.waterlevel = 0;
	ent->v.watertype = CONTENTS_EMPTY;
	cont = SV_PointContents (point);
	if (cont <= CONTENTS_WATER)
	{
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
		point[2] = ent->v.origin[2] + (ent->v.mins[2] + ent->v.maxs[2])*0.5;
		cont = SV_PointContents (point);
		if (cont <= CONTENTS_WATER)
		{
			ent->v.waterlevel = 2;
			point[2] = ent->v.origin[2] + ent->v.view_ofs[2];
			cont = SV_PointContents (point);
			if (cont <= CONTENTS_WATER)
				ent->v.waterlevel = 3;
		}
	}
	
	return ent->v.waterlevel > 1;
}

/*
============
SV_WallFriction

============
*/
void SV_WallFriction (edict_t *ent, trace_t *trace)
{
	vec3_t		forward, right, up;
	float		d, i;
	vec3_t		into, side;
	
	AngleVectors (ent->v.v_angle, forward, right, up);
	d = DotProduct (trace->plane.normal, forward);
	
	d += 0.5;
	if (d >= 0)
		return;
		
// cut the tangential velocity
	i = DotProduct (trace->plane.normal, ent->v.velocity);
	VectorScale (trace->plane.normal, i, into);
	VectorSubtract (ent->v.velocity, into, side);
	
	ent->v.velocity[0] = side[0] * (1 + d);
	ent->v.velocity[1] = side[1] * (1 + d);
}

/*
=====================
SV_TryUnstick

Player has come to a dead stop, possibly due to the problem with limited
float precision at some angle joins in the BSP hull.

Try fixing by pushing one pixel in each direction.

This is a hack, but in the interest of good gameplay...
======================
*/
int SV_TryUnstick (edict_t *ent, vec3_t oldvel)
{
	int		i;
	vec3_t	oldorg;
	vec3_t	dir;
	int		clip;
	trace_t	steptrace;
	
	VectorCopy (ent->v.origin, oldorg);
	VectorCopy (vec3_origin, dir);

	for (i=0 ; i<8 ; i++)
	{
// try pushing a little in an axial direction
		switch (i)
		{
			case 0:	dir[0] = 2; dir[1] = 0; break;
			case 1:	dir[0] = 0; dir[1] = 2; break;
			case 2:	dir[0] = -2; dir[1] = 0; break;
			case 3:	dir[0] = 0; dir[1] = -2; break;
			case 4:	dir[0] = 2; dir[1] = 2; break;
			case 5:	dir[0] = -2; dir[1] = 2; break;
			case 6:	dir[0] = 2; dir[1] = -2; break;
			case 7:	dir[0] = -2; dir[1] = -2; break;
		}
		
		SV_PushEntity (ent, dir);

// retry the original move
		ent->v.velocity[0] = oldvel[0];
		ent->v. velocity[1] = oldvel[1];
		ent->v. velocity[2] = 0;
		clip = SV_FlyMove (ent, 0.1, &steptrace);

		if (fabs(oldorg[1] - ent->v.origin[1]) > 4 || fabs(oldorg[0] - ent->v.origin[0]) > 4)
		{
//Con_DPrintf ("unstuck!\n");
			return clip;
		}
			
// go back to the original pos and try again
		VectorCopy (oldorg, ent->v.origin);
	}
	
	VectorCopy (vec3_origin, ent->v.velocity);
	return 7;		// still not moving
}

/*
=====================
SV_WalkMove

Only used by players
======================
*/
#define	STEPSIZE	18
void SV_WalkMove (edict_t *ent)
{
	vec3_t		upmove, downmove, oldorg, oldvel, nosteporg, nostepvel;
	int		clip, oldonground;
	trace_t		steptrace, downtrace;
	
// do a regular slide move unless it looks like you ran into a step
	oldonground = (int)ent->v.flags & FL_ONGROUND;
	ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;
	
	VectorCopy (ent->v.origin, oldorg);
	VectorCopy (ent->v.velocity, oldvel);
	
	clip = SV_FlyMove (ent, host_frametime, &steptrace);

	if (!(clip & 2))
		return;		// move didn't block on a step

	if (!oldonground && ent->v.waterlevel == 0)
		return;		// don't stair up while jumping
	
	if (ent->v.movetype != MOVETYPE_WALK)
		return;		// gibbed by a trigger
	
	if (sv_nostep.value)
		return;
	
	if ((int)sv_player->v.flags & FL_WATERJUMP)
		return;

	VectorCopy (ent->v.origin, nosteporg);
	VectorCopy (ent->v.velocity, nostepvel);

// try moving up and forward to go up a step
	VectorCopy (oldorg, ent->v.origin);	// back to start pos

	VectorCopy (vec3_origin, upmove);
	VectorCopy (vec3_origin, downmove);
	upmove[2] = STEPSIZE;
	downmove[2] = -STEPSIZE + oldvel[2]*host_frametime;

// move up
	SV_PushEntity (ent, upmove);	// FIXME: don't link?

// move forward
	ent->v.velocity[0] = oldvel[0];
	ent->v. velocity[1] = oldvel[1];
	ent->v. velocity[2] = 0;
	clip = SV_FlyMove (ent, host_frametime, &steptrace);

// check for stuckness, possibly due to the limited precision of floats
// in the clipping hulls
	if (clip)
	{
		if (fabs(oldorg[1] - ent->v.origin[1]) < 0.03125 && fabs(oldorg[0] - ent->v.origin[0]) < 0.03125)
		{	// stepping up didn't make any progress
			clip = SV_TryUnstick (ent, oldvel);
		}
	}
	
// extra friction based on view angle
	if (clip & 2)
		SV_WallFriction (ent, &steptrace);

// move down
	downtrace = SV_PushEntity (ent, downmove);	// FIXME: don't link?

	if (downtrace.plane.normal[2] > 0.7)
	{
		if (ent->v.solid == SOLID_BSP)
		{
			ent->v.flags =	(int)ent->v.flags | FL_ONGROUND;
			ent->v.groundentity = EDICT_TO_PROG(downtrace.ent);
		}
	}
	else
	{
// if the push down didn't end up on good ground, use the move without
// the step up.  This happens near wall / slope combinations, and can
// cause the player to hop up higher on a slope too steep to climb	
		VectorCopy (nosteporg, ent->v.origin);
		VectorCopy (nostepvel, ent->v.velocity);
	}
}

/*
============
SV_CycleWeaponReverse

(JDH: copy of weapons.qc function) 
============
*/
void SV_CycleWeaponReverse (edict_t *ent)
{
	int			it, weapon;
	qboolean	has_ammo;
	dfunction_t	*func;
	
	it = (int) ent->v.items;
	weapon = (int) ent->v.weapon;
	ent->v.impulse = 0;

	while (1)
	{
		has_ammo = true;

		switch (weapon)
		{
		case IT_LIGHTNING:
			weapon = IT_ROCKET_LAUNCHER;
			if (ent->v.ammo_rockets < 1)
				has_ammo = false;
			break;
		case IT_ROCKET_LAUNCHER:
			weapon = IT_GRENADE_LAUNCHER;
			if (ent->v.ammo_rockets < 1)
				has_ammo = false;
			break;
		case IT_GRENADE_LAUNCHER:
			weapon = IT_SUPER_NAILGUN;
			if (ent->v.ammo_nails < 2)
				has_ammo = false;
			break;
		case IT_SUPER_NAILGUN:
			weapon = IT_NAILGUN;
			if (ent->v.ammo_nails < 1)
				has_ammo = false;
			break;
		case IT_NAILGUN:
			weapon = IT_SUPER_SHOTGUN;
			if (ent->v.ammo_shells < 2)
				has_ammo = false;
			break;
		case IT_SUPER_SHOTGUN:
			weapon = IT_SHOTGUN;
			if (ent->v.ammo_shells < 1)
				has_ammo = false;
			break;
		case IT_SHOTGUN:
			weapon = IT_AXE;
			break;
		case IT_AXE:
			weapon = IT_LIGHTNING;
			if (ent->v.ammo_cells < 1)
				has_ammo = false;
			break;
		}
		
		if ((it & weapon) && has_ammo)
		{
			func = PR_FindFunction ("W_SetCurrentAmmo", PRFF_NOBUILTINS);
			if (func)
			{
			// W_SetCurrentAmmo usually has no params, but for lthsp2-lthsp5
			// it expects "self" as an argument
				if (func->numparms == 1)
					((int *)pr_globals)[OFS_PARM0] = PR_GLOBAL(self);
				ent->v.weapon = weapon;
				PR_ExecuteProgram (func - pr_functions);
			}
			return;
		}
	}
};

/*
================
SV_Physics_Client

Player character actions
================
*/
void SV_Physics_Client (edict_t	*ent, int num)
{
	client_t	*cl;
	vec3_t		v;
	qboolean	was_angle_set;
	
	cl = &svs.clients[num-1];
	if (!cl->active)
		return;		// unconnected slot

// call standard client pre-think
	PR_GLOBAL(time) = sv.time;
	PR_GLOBAL(self) = EDICT_TO_PROG(ent);
	PR_ExecuteProgram (PR_GLOBAL(PlayerPreThink));
	
	// for cutscene hack (see below)
	if (isDedicated || (num != 1))
		was_angle_set = false;		// do it on local client only
	else
		was_angle_set = (ent->v.fixangle != 0);
	
// do a move
	SV_CheckVelocity (ent);

// decide which move function to call
	switch ((int)ent->v.movetype)
	{
	case MOVETYPE_NONE:
		if (!SV_RunThink(ent))
			return;
		break;

	case MOVETYPE_WALK:
		if (!SV_RunThink(ent))
			return;
		if (!SV_CheckWater(ent) && !((int)ent->v.flags & FL_WATERJUMP))
			SV_AddGravity (ent);
		SV_CheckStuck (ent);
		SV_WalkMove (ent);

		break;
		
	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		SV_Physics_Toss (ent);
		break;

	case MOVETYPE_FLY:
#ifdef HEXEN2_SUPPORT
	case MOVETYPE_SWIM:
#endif
		if (!SV_RunThink(ent))
			return;
		SV_FlyMove (ent, host_frametime, NULL);
		break;
		
	case MOVETYPE_NOCLIP:
		if (!SV_RunThink(ent))
			return;
		VectorMA (ent->v.origin, host_frametime, ent->v.velocity, ent->v.origin);
		break;
		
	default:
		Sys_Error ("SV_Physics_client: bad movetype %i", (int)ent->v.movetype);
	}


	// JDH: hack for cutscenes made by Darin McNeil's Cutscene Construction Kit:
	//  (note that the extra precision is noticeable only if the viewangles 
	//  are sent from server to client as 2-byte values; hence the addition 
	//  of the new svc_setpreciseangle message code)

	if (was_angle_set && (ent->v.view_ofs[2] == 0) && host_cutscenehack.value 
		&& !strcmp (pr_strings + ent->v.classname, "camera"))
	{
		// - when camera changes back to player, classname remains "camera" for
		//   1 frame, but movedir is no longer valid.  So as an  additional check, 
		//   I verify that view_ofs[2] is still 0
		// - early version(s?) of Cutscene Construction Kit don't move the camera, 
		//   so movedir is not used.  I determine the version by checking *when* 
		//   the viewangle is set: early version does it in the .think function; 
		//   later ones in PlayerPreThink.	was_angle_set will be true only if
		//   it was changed in PlayerPreThink
		
		//if (!sv_oldprotocol.value)
		{
			v[0] = ent->v.movedir[0] - ent->v.origin[0];
			v[1] = ent->v.movedir[1] - ent->v.origin[1];
			v[2] = ent->v.origin[2] - ent->v.movedir[2];
			//vectoangles (v, ent->v.angles);
			vectoangles (v, cl->cutscene_viewangles);
		}
		
		if (!cl->in_cutscene)
		{
			int i;
			edict_t *ed;
			
			// by this time, the player's viewangles have already been changed.
			// But the dummy entity spawned in place of the player has the values
			
			for (i = 1 ; i < sv.num_edicts ; i++)
			{
			// get the current server version
				ed = EDICT_NUM(i);
				if (ed->free)
					continue;

				if (!strcmp(pr_strings + ed->v.classname, "dummy"))
				{
					VectorCopy (ed->v.angles, cl->prev_viewangles);
					break;
				}
			}

			cl->in_cutscene = true;
		}
		//sv.found_cutscene = true;
	}
	else 
	{
		if (cl->in_cutscene)
		{
		// I'm not sure why, but last viewangle while in_cutscene isn't final angle
			ent->v.fixangle = 1;
			VectorCopy (cl->prev_viewangles, ent->v.angles);
			cl->in_cutscene = false;
		}
	}
	
	SV_LinkEdict (ent, true);
	
	PR_GLOBAL(time) = sv.time;
	PR_GLOBAL(self) = EDICT_TO_PROG(ent);

// JDH: another hack, this time for progs that lack CycleWeaponReverse
	if ((ent->v.impulse == 12.0) && ((sv_imp12hack.value >= 2) || (sv_imp12hack.value && !pr_handles_imp12)) && 
		!ent->v.deadflag && (ent->v.view_ofs[0] || ent->v.view_ofs[1] || ent->v.view_ofs[2]))
	{
		eval_t  *val = GETEDICTFIELD(ent, eval_attack_finished);
		if (val && (sv.time >= val->_float))
		{
			SV_CycleWeaponReverse (ent);
		}
	}

// call standard player post-think
	PR_ExecuteProgram (PR_GLOBAL(PlayerPostThink));
}

//============================================================================

/*
=============
SV_Physics_None

Non moving objects can only think
=============
*/
void SV_Physics_None (edict_t *ent)
{
// regular thinking
	SV_RunThink (ent);
}

/*
=============
SV_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
void SV_Physics_Noclip (edict_t *ent)
{
// regular thinking
	if (!SV_RunThink (ent))
		return;
	
#ifdef _DEBUG
	if (ent->v.velocity[0] || ent->v.velocity[1] || ent->v.velocity[2])
		ent->v.velocity[0] *= 1;
#endif

	VectorMA (ent->v.angles, host_frametime, ent->v.avelocity, ent->v.angles);
	VectorMA (ent->v.origin, host_frametime, ent->v.velocity, ent->v.origin);

	SV_LinkEdict (ent, false);
}

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
SV_CheckWaterTransition

=============
*/
void SV_CheckWaterTransition (edict_t *ent)
{
	int	cont;

	cont = SV_PointContents (ent->v.origin);
	if (!ent->v.watertype)
	{	// just spawned here
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
		return;
	}
	
	if (cont <= CONTENTS_WATER)
	{
		if (ent->v.watertype == CONTENTS_EMPTY)
		{	// just crossed into water
		#ifdef HEXEN2_SUPPORT
			if ( hexen2 )
				SV_StartSound (ent, 0, "misc/hith2o.wav", 255, 1);
			else
		#endif
			SV_StartSound (ent, 0, "misc/h2ohit1.wav", 255, 1);
		}		
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
	}
	else
	{
		if (ent->v.watertype != CONTENTS_EMPTY)
		{	// just crossed into water
		#ifdef HEXEN2_SUPPORT
			if ( hexen2 )
				SV_StartSound (ent, 0, "misc/hith2o.wav", 255, 1);
			else
		#endif
			SV_StartSound (ent, 0, "misc/h2ohit1.wav", 255, 1);
		}		
		ent->v.watertype = CONTENTS_EMPTY;
		ent->v.waterlevel = cont;
	}
}

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void SV_Physics_Toss (edict_t *ent)
{
	trace_t	trace;
	vec3_t	move;
	float	backoff;

	// regular thinking
	if (!SV_RunThink (ent))
		return;

// if onground, return without moving
	if (((int)ent->v.flags & FL_ONGROUND))
		return;

	SV_CheckVelocity (ent);

// add gravity
	if (ent->v.movetype != MOVETYPE_FLY && ent->v.movetype != MOVETYPE_FLYMISSILE)
#ifdef HEXEN2_SUPPORT
		if ( (!hexen2) || (ent->v.movetype != MOVETYPE_BOUNCEMISSILE && ent->v.movetype != MOVETYPE_SWIM))
#endif
			SV_AddGravity (ent);

// move anglesy
	VectorMA (ent->v.angles, host_frametime, ent->v.avelocity, ent->v.angles);

// move origin
	VectorScale (ent->v.velocity, host_frametime, move);
	trace = SV_PushEntity (ent, move);
	if (trace.fraction == 1)
		return;
	if (ent->free)
		return;
	
	if (ent->v.movetype == MOVETYPE_BOUNCE)
		backoff = 1.5;
#ifdef HEXEN2_SUPPORT
	else if (hexen2 && (ent->v.movetype == MOVETYPE_BOUNCEMISSILE))
	{	
		if ((ent->v.solid == SOLID_PHASE) && 
			(((int) trace.ent->v.flags & FL_MONSTER) || ((int) trace.ent->v.movetype == MOVETYPE_WALK)))
		{
			return;		// Solid phased missiles don't bounce on monsters or players
		}
		backoff = 2.0;
	}
#endif
	else
		backoff = 1;

	ClipVelocity (ent->v.velocity, trace.plane.normal, ent->v.velocity, backoff);

// stop if on ground
	if (trace.plane.normal[2] > 0.7)
	{		
#ifdef HEXEN2_SUPPORT
		if ((!hexen2) || (ent->v.movetype != MOVETYPE_BOUNCEMISSILE))
#endif
		if (ent->v.velocity[2] < 60 || ent->v.movetype != MOVETYPE_BOUNCE)
		{
			ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
			ent->v.groundentity = EDICT_TO_PROG(trace.ent);
			VectorCopy (vec3_origin, ent->v.velocity);
			VectorCopy (vec3_origin, ent->v.avelocity);
		}
	}
	
// check for in water
	SV_CheckWaterTransition (ent);
}

/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

/*
=============
SV_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
=============
*/
void SV_Physics_Step (edict_t *ent)
{
	qboolean	hitsound;
	char		*snd;

// freefall if not onground
	if (!((int)ent->v.flags & (FL_ONGROUND | FL_FLY | FL_SWIM)))
	{
		if (ent->v.velocity[2] < sv_gravity.value*-0.1)
			hitsound = true;
		else
			hitsound = false;

		SV_AddGravity (ent);
		SV_CheckVelocity (ent);
		SV_FlyMove (ent, host_frametime, NULL);
		SV_LinkEdict (ent, true);

		if ((int)ent->v.flags & FL_ONGROUND)	// just hit ground
#ifdef HEXEN2_SUPPORT
		if (!hexen2 || (!ent->v.flags & FL_MONSTER))
#endif
		{
			if (hitsound)
			{
			#ifdef HEXEN2_SUPPORT
				if (hexen2)
					snd = "fx/thngland.wav";
				else
			#endif
				snd = "demon/dland2.wav";
				SV_StartSound (ent, 0, snd, 255, 1);
			}
		}
	}

// regular thinking
	SV_RunThink (ent);
	
	SV_CheckWaterTransition (ent);
}

//============================================================================

/*
================
SV_Physics

================
*/
void SV_Physics (void)
{
	int			i;
	edict_t		*ent;

// let the progs know that a new frame has started
#ifdef HEXEN2_SUPPORT
	edict_t		*ent2;
	vec3_t		oldOrigin, oldAngle;
	int			originMoved, c;
#endif

	PR_GLOBAL(self) = EDICT_TO_PROG(sv.edicts);
	PR_GLOBAL(other) = EDICT_TO_PROG(sv.edicts);
	PR_GLOBAL(time) = sv.time;
	PR_ExecuteProgram (PR_GLOBAL(StartFrame));

//SV_CheckAllEnts ();

// treat each object in turn
	ent = sv.edicts;
	for (i=0 ; i<sv.num_edicts ; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;

	#ifndef RQM_SV_ONLY
		if (!isDedicated && ((i+1) % 100 == 0))
			S_ExtraUpdateTime (); // BJP: Improve sound when many entities
	#endif

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			ent2 = PROG_TO_EDICT (ent->v.movechain);
			if (ent2 != sv.edicts)
			{
				VectorCopy (ent->v.origin, oldOrigin);
				VectorCopy (ent->v.angles, oldAngle);
			}
		}
	#endif

		if (pr_global_ptrs.force_retouch && *pr_global_ptrs.force_retouch)
			SV_LinkEdict (ent, true);	// force retouch even for stationary

		if (i > 0 && i <= svs.maxclients)
			SV_Physics_Client (ent, i);
		else if (ent->v.movetype == MOVETYPE_PUSH)
			SV_Physics_Pusher (ent);
		else if (ent->v.movetype == MOVETYPE_NONE)
			SV_Physics_None (ent);
		else if (ent->v.movetype == MOVETYPE_NOCLIP)
			SV_Physics_Noclip (ent);
		else if (ent->v.movetype == MOVETYPE_STEP)
			SV_Physics_Step (ent);
	#ifdef HEXEN2_SUPPORT
		else if	((hexen2) && (ent->v.movetype == MOVETYPE_PUSHPULL))
			SV_Physics_Step (ent);
	#endif
		else if (ent->v.movetype == MOVETYPE_TOSS 
		|| ent->v.movetype == MOVETYPE_BOUNCE
	#ifdef HEXEN2_SUPPORT
		|| ((hexen2) && ((ent->v.movetype == MOVETYPE_BOUNCEMISSILE) || (ent->v.movetype == MOVETYPE_SWIM)))
	#endif
		|| ent->v.movetype == MOVETYPE_FLY
		|| ent->v.movetype == MOVETYPE_FLYMISSILE)
			SV_Physics_Toss (ent);
		else
			Sys_Error ("SV_Physics: bad movetype %i", (int)ent->v.movetype);			

	#ifdef HEXEN2_SUPPORT
		if ((hexen2) && (ent2 != sv.edicts))
		{
			originMoved = !VectorCompare(ent->v.origin, oldOrigin);
			if (originMoved || !VectorCompare(ent->v.angles, oldAngle))
			{
				VectorSubtract(ent->v.origin, oldOrigin, oldOrigin);
				VectorSubtract(ent->v.angles, oldAngle, oldAngle);

				for (c=0; c<10; c++)
				{   // chain a max of 10 objects
					if (ent2->free) break;

					VectorAdd(oldOrigin, ent2->v.origin, ent2->v.origin);
					if ((int) ent2->v.flags & FL_MOVECHAIN_ANGLE)
					{
						VectorAdd(oldAngle, ent2->v.angles, ent2->v.angles);
					}

					if (originMoved && ent2->v.chainmoved)
					{	// callback function
						*pr_global_ptrs.self = EDICT_TO_PROG(ent2);
						*pr_global_ptrs.other = EDICT_TO_PROG(ent);
						PR_ExecuteProgram(ent2->v.chainmoved);
					}

					ent2 = PROG_TO_EDICT( ent2->v.movechain );
					if (ent2 == sv.edicts) break;

				}
			}
		}
	#endif
	}
	
	if (pr_global_ptrs.force_retouch && *pr_global_ptrs.force_retouch)
		PR_GLOBAL(force_retouch)--;

	sv.time += host_frametime;
}

trace_t SV_Trace_Toss (edict_t *ent, edict_t *ignore)
{
	edict_t	tempent, *tent;
	trace_t	trace;
	vec3_t	move, end;
	double	save_frametime;

	save_frametime = host_frametime;
	host_frametime = 0.05;

	memcpy (&tempent, ent, sizeof(edict_t));
	tent = &tempent;

	while (1)
	{
		SV_CheckVelocity (tent);
		SV_AddGravity (tent);
		VectorMA (tent->v.angles, host_frametime, tent->v.avelocity, tent->v.angles);
		VectorScale (tent->v.velocity, host_frametime, move);
		VectorAdd (tent->v.origin, move, end);
		trace = SV_Move (tent->v.origin, tent->v.mins, tent->v.maxs, end, MOVE_NORMAL, tent);	
		VectorCopy (trace.endpos, tent->v.origin);

		if (trace.ent)
			if (trace.ent != ignore)
				break;
	}
	host_frametime = save_frametime;

	return trace;
}

