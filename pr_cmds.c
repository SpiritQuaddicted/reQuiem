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

#include "quakedef.h"

#define	RETURN_EDICT(e) (((int *)pr_globals)[OFS_RETURN] = EDICT_TO_PROG(e))

#ifdef HEXEN2_SUPPORT
  extern int *pr_field_map;
#endif

qboolean allow_postcache = false;

static const char *pr_extensionlist[] =
{
	"DP_CL_LOADSKY",
	"DP_EF_BLUE",
	"DP_EF_NODRAW",
	"DP_EF_RED",
	"DP_EF_ADDITIVE",
	"DP_ENT_ALPHA",
	"DP_GFX_EXTERNALTEXTURES",			// not all pathname permutations though
	"DP_GFX_EXTERNALTEXTURES_PERMAPTEXTURES",
	"DP_GFX_FOG",
	"DP_GFX_SKYBOX",
	"DP_LITSUPPORT",
	"DP_QC_ETOS",
	"DP_QC_MINMAXBOUND",
	"DP_QC_SINCOSSQRTPOW",
	"DP_QC_TRACETOSS",
	"DP_QC_TRACEBOX",
	"DP_QUAKE2_MODEL",
	"DP_QUAKE3_MODEL",		// sort of...
	"DP_SND_FAKETRACKS",	// mp3 & ogg, but not wav
	"DP_SPRITE32",
//	"DP_SV_ROTATINGBMODEL",		// currently only for Hexen2
	"DP_TE_BLOOD",
	"NEH_CMD_PLAY2",
	"NEH_RESTOREGAME",
	NULL
};

/*
===============================================================================

				BUILT-IN FUNCTIONS

===============================================================================
*/

char *PF_VarString (int	first)
{
	int		len, i;
	static char out[MAXPRINTMSG];

	if (pr_argc == first+1)
		return G_STRING((OFS_PARM0+first*3));

	len = 0;
//	out[0] = 0;

	for (i=first ; i<pr_argc ; i++)
	{
		len += Q_strcpy (out+len, G_STRING((OFS_PARM0+i*3)), sizeof(out)-len);
		//strcat (out, G_STRING((OFS_PARM0+i*3)));
	}
	return out;
}


/*
=================
PF_error

This is a TERMINAL error, which will kill off the entire server.
Dumps self.

error(value)
=================
*/
void PF_error (void)
{
	char	*s;
	edict_t	*ed;

	s = PF_VarString (0);
	Con_Printf ("======SERVER ERROR in %s:\n%s\n", /*pr_strings +*/ pr_xfunction->s_name, s);
	ed = PROG_TO_EDICT(PR_GLOBAL(self));
	ED_Print (ed);

	Host_Error ("Program error");
}

/*
=================
PF_objerror

Dumps out self, then an error message.  The program is aborted and self is
removed, but the level can continue.

objerror(value)
=================
*/
void PF_objerror (void)
{
	char	*s;
	edict_t	*ed;

	s = PF_VarString (0);
	Con_Printf ("======OBJECT ERROR in %s:\n%s\n", /*pr_strings +*/ pr_xfunction->s_name, s);
	ed = PROG_TO_EDICT(PR_GLOBAL(self));
	ED_Print (ed);
	ED_Free (ed);

	//Host_Error ("Program error"); //johnfitz -- by design, this should not be fatal
}


/*
==============
PF_makevectors

Writes new values for v_forward, v_up, and v_right based on angles
makevectors(vector)
==============
*/
void PF_makevectors (void)
{
	AngleVectors (G_VECTOR(OFS_PARM0), PR_GLOBAL(v_forward), PR_GLOBAL(v_right), PR_GLOBAL(v_up));
}

/*
=================
PF_setorigin

This is the only valid way to move an object without using the physics of the world
(setting velocity and waiting).  Directly changing origin will not set internal
links correctly, so clipping would be messed up.  This should be called when an
object is spawned, and then only if it is teleported.

setorigin (entity, origin)
=================
*/
void PF_setorigin (void)
{
	edict_t	*e;
	float	*org;

	e = G_EDICT(OFS_PARM0);
	org = G_VECTOR(OFS_PARM1);
	VectorCopy (org, e->v.origin);
	SV_LinkEdict (e, false);
}


void SetMinMaxSize (edict_t *e, float *min, float *max, qboolean rotate)
{
	float	*angles;
	vec3_t	rmin, rmax;
	float	bounds[2][3];
	float	xvector[2], yvector[2];
	float	a;
	vec3_t	base, transformed;
	int		i, j, k, l;

	for (i=0 ; i<3 ; i++)
		if (min[i] > max[i])
			PR_RunError ("backwards mins/maxs");

	rotate = false;		// FIXME: implement rotation properly again

	if (!rotate)
	{
		VectorCopy (min, rmin);
		VectorCopy (max, rmax);
	}
	else
	{
	// find min / max for rotations
		angles = e->v.angles;

		a = angles[1]/180 * M_PI;

		xvector[0] = cos(a);
		xvector[1] = sin(a);
		yvector[0] = -sin(a);
		yvector[1] = cos(a);

		VectorCopy (min, bounds[0]);
		VectorCopy (max, bounds[1]);

		rmin[0] = rmin[1] = rmin[2] = 9999;
		rmax[0] = rmax[1] = rmax[2] = -9999;

		for (i=0 ; i<= 1 ; i++)
		{
			base[0] = bounds[i][0];
			for (j=0 ; j<= 1 ; j++)
			{
				base[1] = bounds[j][1];
				for (k=0 ; k<= 1 ; k++)
				{
					base[2] = bounds[k][2];

				// transform the point
					transformed[0] = xvector[0]*base[0] + yvector[0]*base[1];
					transformed[1] = xvector[1]*base[0] + yvector[1]*base[1];
					transformed[2] = base[2];

					for (l=0 ; l<3 ; l++)
					{
						if (transformed[l] < rmin[l])
							rmin[l] = transformed[l];
						if (transformed[l] > rmax[l])
							rmax[l] = transformed[l];
					}
				}
			}
		}
	}

// set derived values
	VectorCopy (rmin, e->v.mins);
	VectorCopy (rmax, e->v.maxs);
	VectorSubtract (max, min, e->v.size);

	SV_LinkEdict (e, false);
}

/*
=================
PF_setsize

the size box is rotated by the current angle

setsize (entity, minvector, maxvector)
=================
*/
void PF_setsize (void)
{
	edict_t	*e;
	float	*min, *max;

	e = G_EDICT(OFS_PARM0);
	min = G_VECTOR(OFS_PARM1);
	max = G_VECTOR(OFS_PARM2);
	SetMinMaxSize (e, min, max, false);
}
/*
#ifdef _DEBUG
// JDH: the next 3 procs are just me testing out post-compile BSP modification,
//   with the goal of making rotating entities more mapper-friendly.  It doesn't
//   belong in the engine; it just saved me the effort of creating a new project

// FindTexinfo - Returns a global texinfo number
mtexinfo_t * FindTexinfo (model_t *mod, const mtexinfo_t *t)
{
	int			i, j;
	mtexinfo_t	*tex;

	tex = mod->texinfo;
	for (i=0 ; i<mod->numtexinfo;i++, tex++)
	{
		if (t->texture != tex->texture)
			continue;
		if (t->flags != tex->flags)
			continue;

		for (j=0 ; j<8 ; j++)
			if (t->vecs[0][j] != tex->vecs[0][j])
				break;
		if (j != 8)
			continue;

		return tex;
	}

// allocate a new texture
	tex = malloc (sizeof(mtexinfo_t));			// ********* FIXME: LEAK!!!! ************
	*tex = *t;
	return tex;
}

mplane_t * FindPlane (model_t *mod, const mplane_t *p, float dist)
{
	int			i, j;
	mplane_t	*plane;

	plane = mod->planes;
	for (i=0 ; i<mod->numplanes;i++, plane++)
	{
		if (p->type != plane->type)
			continue;
		if (p->signbits != plane->signbits)
			continue;
		if (dist != plane->dist)
			continue;

		for (j=0 ; j<3 ; j++)
			if (p->normal[j] != plane->normal[j])
				break;
		if (j != 3)
			continue;

		return plane;
	}

// allocate a new plane
	plane = malloc (sizeof(mplane_t));			// ********* FIXME: LEAK!!!! ************
	*plane = *p;
	plane->dist = dist;
	return plane;
}

extern void CalcSurfaceExtents (model_t *mod, msurface_t *s);

void OffsetBrushModel (edict_t *ed, model_t *mod)
{
	msurface_t	*psurf;
	int			i, j, lindex;
	unsigned	vnum;
	medge_t		*r_pedge;
	float		s, t, dist, *ofs, *vert;
//	vec3_t		t1, t2, t3;
	byte		done[MAX_MAP_VERTS];
	mtexinfo_t	tx;

	for (i = 0; i < mod->numvertexes; i++)
		done[i] = 0;

	ofs = ed->v.v_angle;
	psurf = &mod->surfaces[mod->firstmodelsurface];

	for (i=0 ; i<mod->nummodelsurfaces ; i++, psurf++)
	{
		if (!strcmp(psurf->texinfo->texture->name, "blackdot"))
			j = 0;

		for (j = 0; j < psurf->numedges; j++)
		{
			lindex = mod->surfedges[psurf->firstedge + j];

			if (lindex > 0)
			{
				vnum = 0;
			}
			else
			{
				lindex = -lindex;
				vnum = 1;
			}

			r_pedge = &mod->edges[lindex];
			vnum = r_pedge->v[vnum];

			vert = mod->vertexes[vnum].position;		// last value for "vert" is used outside of j-loop

		// make sure each vertex is offset only once
			if (done[vnum])
				continue;

			done[vnum] = 1;

		// calc s and t values before offset
			s = DotProduct(vert, psurf->texinfo->vecs[0]) + psurf->texinfo->vecs[0][3];
			t = DotProduct(vert, psurf->texinfo->vecs[1]) + psurf->texinfo->vecs[1][3];

			VectorSubtract (vert, ofs, vert);

		// modify texture offsets (vecs[0][3] and vecs[1][3]) so that s and t are unchanged
			tx = *psurf->texinfo;

			tx.vecs[0][3] = s - DotProduct(vert, psurf->texinfo->vecs[0]);
			tx.vecs[1][3] = t - DotProduct(vert, psurf->texinfo->vecs[1]);

//			if ((tx.vecs[0][3] != psurf->texinfo->vecs[0][3]) || (tx.vecs[1][3] != psurf->texinfo->vecs[1][3]))
//				psurf->texinfo = FindTexinfo (mod, &tx);

		#if 0
			if (psurf->polys)
			{
				vert = psurf->polys->verts[j];
				VectorSubtract (vert, ofs, vert);
			}
		#endif
		}

			if ((psurf->plane->normal[0] && psurf->plane->normal[1]) ||
				(psurf->plane->normal[0] && psurf->plane->normal[2]) ||
				(psurf->plane->normal[1] && psurf->plane->normal[2]))
				dist = 23423;


		dist = DotProduct (vert, psurf->plane->normal);
		if (dist != psurf->plane->dist)
			psurf->plane = FindPlane (mod, psurf->plane, dist);

		tx = *psurf->texinfo;

		tx.vecs[0][3] += DotProduct(ofs, psurf->texinfo->vecs[0]);
		tx.vecs[1][3] += DotProduct(ofs, psurf->texinfo->vecs[1]);

		if ((tx.vecs[0][3] != psurf->texinfo->vecs[0][3]) || (tx.vecs[1][3] != psurf->texinfo->vecs[1][3]))
			psurf->texinfo = FindTexinfo (mod, &tx);

		CalcSurfaceExtents (mod, psurf);

	// convert to a vector / dist plane
#if 0
		for (j=0 ; j<3 ; j++)
		{
			t1[j] = planepts[0][j] - planepts[1][j];
			t2[j] = planepts[2][j] - planepts[1][j];
			t3[j] = planepts[1][j];
		}

		CrossProduct(t1,t2, f->plane.normal);
		if (VectorCompare (f->plane.normal, vec3_origin))
		{
			Message (MSGWARN, "Brush plane with no normal on line %d", scriptline - 1);
			FreeOther (f);
			continue;
		}
		VectorNormalize (f->plane.normal);
		f->plane.dist = DotProduct (t3, f->plane.normal);
#endif
	}

	VectorAdd (ed->v.origin, ofs, ed->v.origin);
	VectorSubtract (mod->mins, ofs, mod->mins);
	VectorSubtract (mod->maxs, ofs, mod->maxs);
}
#endif
*/
/*
=================
PF_setmodel

setmodel(entity, model)
=================
*/
void PF_setmodel (void)
{
	edict_t	*e;
	char	*m, **check;
	model_t	*mod;
	int		i;

	e = G_EDICT(OFS_PARM0);
	m = G_STRING(OFS_PARM1);

// check to see if model was properly precached
	for (i=0, check = sv.model_precache ; *check ; i++, check++)
		if (COM_FilenamesEqual(*check, m))
			break;

/*****JDH******/
	if (!*check)
		//PR_RunError ("no precache: %s\n", m);
		return;
/*****JDH******/

	e->v.model = m - pr_strings;
	e->v.modelindex = i; //SV_ModelIndex (m);

	mod = sv.models[i];  // Mod_ForName (m, true);
/*
#ifdef _DEBUG
	if (!strcmp(pr_strings + e->v.classname, "func_door_hinged"))
	{
		if ((mod->type == mod_brush) && VectorCompare(e->v.origin, vec3_origin))
		{
			OffsetBrushModel (e, mod);
		}
	}
#endif
*/
	if (mod)
		SetMinMaxSize (e, mod->mins, mod->maxs, true);
	else
		SetMinMaxSize (e, vec3_origin, vec3_origin, true);
}

/*
=================
PF_bprint

broadcast print to everyone on server

bprint(value)
=================
*/
void PF_bprint (void)
{
	char		*s;

	s = PF_VarString (0);
	SV_BroadcastPrintf ("%s", s);
}

/*
=================
PF_sprint

single print to a specific client

sprint(clientent, value)
=================
*/
void PF_sprint (void)
{
	char		*s;
	client_t	*client;
	int			entnum;

	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString (1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Print ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteCmd (&client->message, svc_print);
	MSG_WriteString (&client->message, s );
}


/*
=================
PF_centerprint

single print to a specific client

centerprint(clientent, value)
=================
*/
void PF_centerprint (void)
{
	char		*s;
	client_t	*client;
	int			entnum;

	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString (1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Print ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteCmd (&client->message, svc_centerprint);
	MSG_WriteString (&client->message, s );
}


/*
=================
PF_normalize

vector normalize(vector)
=================
*/
void PF_normalize (void)
{
	float	*value1;
	vec3_t	newvalue;
	float	new;

	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);

	if (new == 0)
		newvalue[0] = newvalue[1] = newvalue[2] = 0;
	else
	{
		new = 1/new;
		newvalue[0] = value1[0] * new;
		newvalue[1] = value1[1] * new;
		newvalue[2] = value1[2] * new;
	}

	VectorCopy (newvalue, G_VECTOR(OFS_RETURN));
}

/*
=================
PF_vlen

scalar vlen(vector)
=================
*/
void PF_vlen (void)
{
	float	*value1;
	float	new;

	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);

	G_FLOAT(OFS_RETURN) = new;
}

/*
=================
PF_vectoyaw

float vectoyaw(vector)
=================
*/
void PF_vectoyaw (void)
{
	float	*value1;
	float	yaw;

	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	G_FLOAT(OFS_RETURN) = yaw;
}


/*
=================
PF_vectoangles

vector vectoangles(vector)
=================
*/
void PF_vectoangles (void)
{
	float	*value1, forward, yaw, pitch;

	value1 = G_VECTOR(OFS_PARM0);

	if (!value1[1] && !value1[0])
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	G_FLOAT(OFS_RETURN+0) = pitch;
	G_FLOAT(OFS_RETURN+1) = yaw;
	G_FLOAT(OFS_RETURN+2) = 0;
}

/*
=================
PF_Random

Returns a number from 0<= num < 1

random()
=================
*/
void PF_random (void)
{
	float		num;

	num = (rand ()&0x7fff) / ((float)0x7fff);

	G_FLOAT(OFS_RETURN) = num;
}

/*
=================
PF_particle

particle(origin, color, count)
=================
*/
void PF_particle (void)
{
	float	*org, *dir, color, count;

	org = G_VECTOR(OFS_PARM0);
	dir = G_VECTOR(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	count = G_FLOAT(OFS_PARM3);
	SV_StartParticle (org, dir, color, count);
}

/*
=================
PF_te_blood
=================
*/
void PF_te_blood (void)
{
	int count = G_FLOAT(OFS_PARM2);

	if (count < 1)
		return;
	MSG_WriteCmd(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_BLOOD);
	// origin
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
	// velocity
	MSG_WriteChar(&sv.datagram, bound(-128, (int) G_VECTOR(OFS_PARM1)[0], 127));
	MSG_WriteChar(&sv.datagram, bound(-128, (int) G_VECTOR(OFS_PARM1)[1], 127));
	MSG_WriteChar(&sv.datagram, bound(-128, (int) G_VECTOR(OFS_PARM1)[2], 127));
	// count
	MSG_WriteByte(&sv.datagram, bound(0, (int) count, 255));
}

/*
=================
PF_te_customflash
=================
*/
/*void PF_te_customflash (void)
{
	float rad, life;

	rad = G_FLOAT(OFS_PARM1);
	life = G_FLOAT(OFS_PARM2);

	if (rad < 8 || life < (1.0 / 256.0))
		return;
	MSG_WriteByte(&sv.datagram, svc_temp_entity);
	MSG_WriteByte(&sv.datagram, TE_CUSTOMFLASH);
	// origin
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[0]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[1]);
	MSG_WriteCoord(&sv.datagram, G_VECTOR(OFS_PARM0)[2]);
	// radius
	MSG_WriteByte(&sv.datagram, (int)bound(0, rad/8 - 1, 255));
	// lifetime
	MSG_WriteByte(&sv.datagram, (int)bound(0, life*256 - 1, 255));
	// color
	MSG_WriteByte(&sv.datagram, (int)bound(0, G_VECTOR(OFS_PARM3)[0] * 255, 255));
	MSG_WriteByte(&sv.datagram, (int)bound(0, G_VECTOR(OFS_PARM3)[1] * 255, 255));
	MSG_WriteByte(&sv.datagram, (int)bound(0, G_VECTOR(OFS_PARM3)[2] * 255, 255));
}
*/
/*
=================
PF_ambientsound
=================
*/
void PF_ambientsound (void)
{
	char		**check, *samp;
	float		*pos, vol, attenuation;
	int		i, soundnum;

	pos = G_VECTOR (OFS_PARM0);
	samp = G_STRING(OFS_PARM1);
	vol = G_FLOAT(OFS_PARM2);
	attenuation = G_FLOAT(OFS_PARM3);

// check to see if samp was properly precached
	for (soundnum=0, check = sv.sound_precache ; *check ; check++, soundnum++)
		if (!strcmp(*check,samp))
			break;

	if (!*check)
	{
		Con_Printf ("no precache: %s\n", samp);
		return;
	}

// add an svc_spawnambient command to the level signon packet

	MSG_WriteCmd (&sv.signon, svc_spawnstaticsound);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.signon, pos[i]);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		MSG_WriteShort (&sv.signon, soundnum);
	else
#endif
	MSG_WriteByte (&sv.signon, soundnum);

	MSG_WriteByte (&sv.signon, vol*255);
	MSG_WriteByte (&sv.signon, attenuation*64);

}

/*
=================
PF_sound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
allready running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.

=================
*/
void PF_sound (void)
{
	char		*sample;
	int			channel, volume;
	edict_t		*entity;
	float		attenuation;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);
	sample = G_STRING(OFS_PARM2);
	volume = G_FLOAT(OFS_PARM3) * 255;
	attenuation = G_FLOAT(OFS_PARM4);

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	SV_StartSound (entity, channel, sample, volume, attenuation);
}

/*
=================
PF_break

break()
=================
*/
void PF_break (void)
{
	Con_Print ("break statement\n");
	*(int *)-4 = 0;	// dump to debugger
//	PR_RunError ("break statement");
}

/*
=================
PR_Trace
  internal function shared by PF_traceline and PF_tracebox
=================
*/
void PR_Trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int type, edict_t *passedict)
{
	trace_t	trace;
#ifdef HEXEN2_SUPPORT
	float	save_hull;

	if (hexen2)
	{
		save_hull = passedict->v.hull;
		passedict->v.hull = 0;
	}
#endif

	trace = SV_Move (start, mins, maxs, end, type, passedict);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		passedict->v.hull = save_hull;
#endif

	PR_GLOBAL(trace_allsolid) = trace.allsolid;
	PR_GLOBAL(trace_startsolid) = trace.startsolid;
	PR_GLOBAL(trace_fraction) = trace.fraction;
	PR_GLOBAL(trace_inwater) = trace.inwater;
	PR_GLOBAL(trace_inopen) = trace.inopen;
	VectorCopy (trace.endpos, PR_GLOBAL(trace_endpos));
	VectorCopy (trace.plane.normal, PR_GLOBAL(trace_plane_normal));
	PR_GLOBAL(trace_plane_dist) =  trace.plane.dist;
	if (trace.ent)
		PR_GLOBAL(trace_ent) = EDICT_TO_PROG(trace.ent);
	else
		PR_GLOBAL(trace_ent) = EDICT_TO_PROG(sv.edicts);

}

/*
=================
PF_traceline

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entities, and also slide box entities
if the tryents flag is set.

traceline (start, end, type, ent)
=================
*/
void PF_traceline (void)
{
	float	*v1, *v2;
	int		type;
	edict_t	*ent;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	type = G_FLOAT(OFS_PARM2);
	ent = G_EDICT(OFS_PARM3);

	PR_Trace (v1, vec3_origin, vec3_origin, v2, type, ent);
}

/*
=================
PF_tracebox (from DarkPlaces)

tracebox (start, mins, maxs, end, type, ent)
=================
*/
void PF_tracebox (void)
{
	float	*v1, *v2, *mins, *maxs;
	int		type;
	edict_t	*ent;

	v1 = G_VECTOR(OFS_PARM0);
	mins = G_VECTOR(OFS_PARM1);
	maxs = G_VECTOR(OFS_PARM2);
	v2 = G_VECTOR(OFS_PARM3);
	type = G_FLOAT(OFS_PARM4);
	ent = G_EDICT(OFS_PARM5);

	PR_Trace (v1, mins, maxs, v2, type, ent);
}

extern trace_t SV_Trace_Toss (edict_t *ent, edict_t *ignore);

void PF_tracetoss (void)
{
	trace_t	trace;
	edict_t	*ent, *ignore;

	ent = G_EDICT(OFS_PARM0);
	ignore = G_EDICT(OFS_PARM1);

	trace = SV_Trace_Toss (ent, ignore);

	PR_GLOBAL(trace_allsolid) = trace.allsolid;
	PR_GLOBAL(trace_startsolid) = trace.startsolid;
	PR_GLOBAL(trace_fraction) = trace.fraction;
	PR_GLOBAL(trace_inwater) = trace.inwater;
	PR_GLOBAL(trace_inopen) = trace.inopen;
	VectorCopy (trace.endpos, PR_GLOBAL(trace_endpos));
	VectorCopy (trace.plane.normal, PR_GLOBAL(trace_plane_normal));
	PR_GLOBAL(trace_plane_dist) =  trace.plane.dist;
	if (trace.ent)
		PR_GLOBAL(trace_ent) = EDICT_TO_PROG(trace.ent);
	else
		PR_GLOBAL(trace_ent) = EDICT_TO_PROG(sv.edicts);
}

/*
=================
PF_checkpos

Returns true if the given entity can move to the given position from it's
current position by walking or rolling.
FIXME: make work...
scalar checkpos (entity, vector)
=================
*/
void PF_checkpos (void)
{
}

//============================================================================

byte	checkpvs[MAX_MAP_LEAFS/8];

int PF_newcheckclient (int check)
{
	int		i;
	byte	*pvs;
	edict_t	*ent;
	mleaf_t	*leaf;
	vec3_t	org;

// cycle to the next one

	if (check < 1)
		check = 1;
	if (check > svs.maxclients)
		check = svs.maxclients;

	if (check == svs.maxclients)
		i = 1;
	else
		i = check + 1;

	for ( ; ; i++)
	{
		if (i == svs.maxclients+1)
			i = 1;

		ent = EDICT_NUM(i);

		if (i == check)
			break;	// didn't find anything else

		if (ent->free)
			continue;
		if (ent->v.health <= 0)
			continue;
		if ((int)ent->v.flags & FL_NOTARGET)
			continue;

	// anything that is a client, or has a client as an enemy
		break;
	}

// get the PVS for the entity
	VectorAdd (ent->v.origin, ent->v.view_ofs, org);
	leaf = Mod_PointInLeaf (org, sv.worldmodel);
	pvs = Mod_LeafPVS (leaf, sv.worldmodel);
	memcpy (checkpvs, pvs, (sv.worldmodel->numleafs+7)>>3 );

	return i;
}

/*
=================
PF_checkclient

Returns a client (or object that has a client enemy) that would be a
valid target.

If there are more than one valid options, they are cycled each frame

If (self.origin + self.viewofs) is not in the PVS of the current target,
it is not returned at all.

name checkclient ()
=================
*/
#define	MAX_CHECK	16
int c_invis, c_notvis;
void PF_checkclient (void)
{
	edict_t	*ent, *self;
	mleaf_t	*leaf;
	int		l;
	vec3_t	view;

// find a new check if on a new frame
	if (sv.time - sv.lastchecktime >= 0.1)
	{
		sv.lastcheck = PF_newcheckclient (sv.lastcheck);
		sv.lastchecktime = sv.time;
	}

// return check if it might be visible
	ent = EDICT_NUM(sv.lastcheck);
	if (ent->free || ent->v.health <= 0)
	{
		RETURN_EDICT(sv.edicts);
		return;
	}

// if current entity can't possibly see the check entity, return 0
	self = PROG_TO_EDICT(PR_GLOBAL(self));

	VectorAdd (self->v.origin, self->v.view_ofs, view);
	leaf = Mod_PointInLeaf (view, sv.worldmodel);
	l = (leaf - sv.worldmodel->leafs) - 1;
	if ((l < 0) || !(checkpvs[l>>3] & (1 << (l & 7))))
	{
c_notvis++;
		RETURN_EDICT(sv.edicts);
		return;
	}

// might be able to see it
c_invis++;
	RETURN_EDICT(ent);
}

//============================================================================


/*
=================
PF_stuffcmd

Sends text over to the client's execution buffer

stuffcmd (clientent, value)
=================
*/
void PF_stuffcmd (void)
{
	int			entnum;
	char		*str;
	client_t	*old;

	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("Parm 0 not a client");
	str = G_STRING(OFS_PARM1);

	old = host_client;
	host_client = &svs.clients[entnum-1];
	Host_ClientCommands ("%s", str);
	host_client = old;
}

/*
=================
PF_localcmd

Sends text over to the client's execution buffer

localcmd (string)
=================
*/
void PF_localcmd (void)
{
	char	*str;

	str = G_STRING(OFS_PARM0);
	Cbuf_AddText (str, SRC_COMMAND);
}

/*
=================
PF_cvar

float cvar (string)
=================
*/
void PF_cvar (void)
{
	char	*str;

	str = G_STRING(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = Cvar_VariableValue (str);
}

/*
=================
PF_cvar_set

cvar (string, string)
=================
*/
void PF_cvar_set (void)
{
	char	*var, *val;

	var = G_STRING(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

	Cvar_Set (var, val);
}

/*
=================
PF_findradius

Returns a chain of entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
void PF_findradius (void)
{
	edict_t	*ent, *chain;
	float	rad;
	float	*org;
	vec3_t	eorg;
	int		i, j;

	chain = (edict_t *)sv.edicts;

	org = G_VECTOR(OFS_PARM0);
	rad = G_FLOAT(OFS_PARM1);

	ent = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		if (ent->v.solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (ent->v.origin[j] + (ent->v.mins[j] + ent->v.maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;

		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}

	RETURN_EDICT(chain);
}


/*
=========
PF_dprint
=========
*/
void PF_dprint (void)
{
	Con_DPrintf ("%s", PF_VarString(0));
}

//char	pr_string_temp[128];
char	pr_string_temp[1024];	//JDH

void PF_ftos (void)
{
	float	v;
	v = G_FLOAT(OFS_PARM0);

	if (v == (int)v)
		Q_snprintfz (pr_string_temp, sizeof(pr_string_temp), "%d",(int)v);
/*****JDH*****/
	//else
	//	Q_snprintfz (pr_string_temp, sizeof(pr_string_temp), "%5.1f",v);

	else if ((int)(v * 100) % 10 == 0)
		Q_snprintfz (pr_string_temp, sizeof(pr_string_temp), "%5.1f",v);
	else
	{
		int i = Q_snprintfz (pr_string_temp, sizeof(pr_string_temp), "%f", v);

		for (i = i - 1; i > 0 && pr_string_temp[i] == '0'; i--)
			pr_string_temp[i] = 0;
	}
/*****JDH*****/

	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

void PF_fabs (void)
{
	float	v;
	v = G_FLOAT(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = fabs(v);
}

void PF_vtos (void)
{
	Q_snprintfz (pr_string_temp, sizeof(pr_string_temp), "'%5.1f %5.1f %5.1f'", G_VECTOR(OFS_PARM0)[0], G_VECTOR(OFS_PARM0)[1], G_VECTOR(OFS_PARM0)[2]);
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

void PF_etos (void)
{
	Q_snprintfz (pr_string_temp, sizeof(pr_string_temp), "entity %i", G_EDICTNUM(OFS_PARM0));
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

void PF_Spawn (void)
{
	edict_t	*ed;
	ed = ED_Alloc();
	RETURN_EDICT(ed);
}

void PF_Remove (void)
{
	edict_t	*ed;
#ifdef HEXEN2_SUPPORT
	int i;
#endif

	ed = G_EDICT(OFS_PARM0);

#ifdef HEXEN2_SUPPORT
	if (ed == sv.edicts)
	{
		Con_DPrintf("Tried to remove the world at %s in %s!\n",
			pr_xfunction->s_name /*+ pr_strings*/, pr_xfunction->s_file /*+ pr_strings*/);
		return;
	}

	i = NUM_FOR_EDICT(ed);
	if (i <= svs.maxclients)
	{
		Con_DPrintf("Tried to remove a client at %s in %s!\n",
			pr_xfunction->s_name /*+ pr_strings*/, pr_xfunction->s_file /*+ pr_strings*/);
		return;
	}
#endif

	ED_Free (ed);
}


// entity (entity start, .string field, string match) find = #5;
void PF_Find (void)
{
	int		e;
	int		f;
	char	*s, *t;
	edict_t	*ed;

	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("PF_Find: bad search string");

	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
	/****JDH****/
		//t = E_STRING(ed,f);
	#ifdef HEXEN2_SUPPORT
		t = pr_strings + *(string_t *)&((float*)&ed->v)[pr_field_map[f]];
	#else
		t = pr_strings + *(string_t *)&((float*)&ed->v)[f];
	#endif
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			RETURN_EDICT(ed);
			return;
		}
	}

	RETURN_EDICT(sv.edicts);
}

void PR_CheckEmptyString (const char *s)
{
	if (s[0] <= ' ')
		PR_RunError ("Bad string");
}

void PF_precache_file (void)
{	// precache_file is only used to copy files with qcc, it does nothing
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
}

void PF_precache_sound (void)
{
	char	*s;
	int		i, maxsounds;

	if ((sv.state != ss_loading) && !allow_postcache)
		PR_RunError ("PF_Precache_*: Precache can be done only in spawn functions");

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		maxsounds = MAX_SOUNDS_H2;
	else
#endif
	maxsounds = MAX_SOUNDS;

	for (i=0 ; i < maxsounds ; i++)
	{
		if (!sv.sound_precache[i])
		{
			sv.sound_precache[i] = s;
			return;
		}
		if (COM_FilenamesEqual(sv.sound_precache[i], s))
			return;
	}
	PR_RunError ("PF_precache_sound: overflow");
}

void PF_precache_model (void)
{
	char	*s;
	int		i;

	if ((sv.state != ss_loading) && !allow_postcache)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
		{
			sv.model_precache[i] = s;
		/*******JDH*******/
			//sv.models[i] = Mod_ForName (s, true);

			sv.models[i] = Mod_ForName (s, false);
			if (!sv.models[i])
			{
				PR_LogMissingModel (s);
				sv.model_precache[i] = 0;
			}
		/*******JDH*******/
			return;
		}
		if (COM_FilenamesEqual(sv.model_precache[i], s))
			return;
	}
	PR_RunError ("PF_precache_model: overflow");
}


void PF_coredump (void)
{
	PR_PrintEdicts_f (SRC_SERVER);
}

void PF_traceon (void)
{
	pr_trace = true;
}

void PF_traceoff (void)
{
	pr_trace = false;
}

void PF_eprint (void)
{
	ED_PrintNum (G_EDICTNUM(OFS_PARM0));
}

/*
===============
PF_walkmove

float(float yaw, float dist) walkmove
===============
*/
void PF_walkmove (void)
{
	edict_t	*ent;
	float	yaw, dist;
	vec3_t	move;
	dfunction_t	*oldf;
	int 	oldself;

#ifdef HEXEN2_SUPPORT
	qboolean set_trace = (hexen2 ? G_FLOAT(OFS_PARM2) : false);
#endif

	ent = PROG_TO_EDICT(PR_GLOBAL(self));

	yaw = G_FLOAT(OFS_PARM0);
	dist = G_FLOAT(OFS_PARM1);

	if ( !( (int)ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM) ) )
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	yaw = yaw*M_PI*2 / 360;

	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

// save program state, because SV_movestep may call other progs
	oldf = pr_xfunction;

	oldself = PR_GLOBAL(self);

#ifdef HEXEN2_SUPPORT
	G_FLOAT(OFS_RETURN) = SV_movestep(ent, move, true, (hexen2 ? true : false), set_trace);
#else
	G_FLOAT(OFS_RETURN) = SV_movestep(ent, move, true);
#endif

// restore program state
	pr_xfunction = oldf;

	PR_GLOBAL(self) = oldself;
}

/*
===============
PF_droptofloor

void() droptofloor
===============
*/
void PF_droptofloor (void)
{
	edict_t		*ent;
	vec3_t		end;
	trace_t		trace;

	ent = PROG_TO_EDICT(PR_GLOBAL(self));

	VectorCopy (ent->v.origin, end);
	end[2] -= 256;

	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

	if (trace.fraction == 1 || trace.allsolid)
		G_FLOAT(OFS_RETURN) = 0;
	else
	{
		VectorCopy (trace.endpos, ent->v.origin);
		SV_LinkEdict (ent, false);
		ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
		ent->v.groundentity = EDICT_TO_PROG(trace.ent);
		G_FLOAT(OFS_RETURN) = 1;
	}
}

/*
===============
PF_lightstyle

void(float style, string value) lightstyle
===============
*/
void PF_lightstyle (void)
{
	int		style;
	char	*val;
	client_t	*client;
	int			j;

	style = G_FLOAT(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

// change the string in sv
	sv.lightstyles[style] = val;

// send message to all clients on this server
	if (sv.state != ss_active)
		return;

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
		if (client->active || client->spawned)
		{
			MSG_WriteCmd (&client->message, svc_lightstyle);
			MSG_WriteChar (&client->message, style);
			MSG_WriteString (&client->message, val);
		}
}

void PF_rint (void)
{
	float	f;
	f = G_FLOAT(OFS_PARM0);
	if (f > 0)
		G_FLOAT(OFS_RETURN) = (int)(f + 0.5);
	else
		G_FLOAT(OFS_RETURN) = (int)(f - 0.5);
}
void PF_floor (void)
{
	G_FLOAT(OFS_RETURN) = floor(G_FLOAT(OFS_PARM0));
}
void PF_ceil (void)
{
	G_FLOAT(OFS_RETURN) = ceil(G_FLOAT(OFS_PARM0));
}


/*
=============
PF_checkbottom
=============
*/
void PF_checkbottom (void)
{
	edict_t	*ent;

	ent = G_EDICT(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_CheckBottom (ent);
}

/*
=============
PF_pointcontents
=============
*/
void PF_pointcontents (void)
{
	float	*v;

	v = G_VECTOR(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_PointContents (v);
}

/*
=============
PF_nextent

entity nextent(entity)
=============
*/
void PF_nextent (void)
{
	int		i;
	edict_t	*ent;

	i = G_EDICTNUM(OFS_PARM0);
	while (1)
	{
		i++;
		if (i == sv.num_edicts)
		{
			RETURN_EDICT(sv.edicts);
			return;
		}
		ent = EDICT_NUM(i);
		if (!ent->free)
		{
			RETURN_EDICT(ent);
			return;
		}
	}
}

/*
=============
PF_aim

Pick a vector for the player to shoot along
vector aim(entity, missilespeed)
=============
*/
void PF_aim (void)
{
	edict_t	*ent, *check, *bestent;
	vec3_t	start, dir, end, bestdir;
	int		i, j, dmgflag;
	trace_t	tr;
	float	dist, bestdist, speed;

#ifdef HEXEN2_SUPPORT
	float	*shot_org;
	float	save_hull;

	if (hexen2)
	{
		//vector aim(entity, vector, speed)
		ent = G_EDICT(OFS_PARM0);
		shot_org = G_VECTOR(OFS_PARM1);
		speed = G_FLOAT(OFS_PARM2);

		VectorCopy (shot_org, start);
		dmgflag = DAMAGE_YES;
	}
	else
#endif
	{
		ent = G_EDICT(OFS_PARM0);
		speed = G_FLOAT(OFS_PARM1);

		VectorCopy (ent->v.origin, start);
		dmgflag = DAMAGE_AIM;
	}
	start[2] += 20;

// try sending a trace straight
	VectorCopy (PR_GLOBAL(v_forward), dir);

	VectorMA (start, 2048, dir, end);
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		save_hull = ent->v.hull;
		ent->v.hull = 0;
		tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
		ent->v.hull = save_hull;
	}
	else
#endif
	tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);

	if (tr.ent && tr.ent->v.takedamage == dmgflag && (!teamplay.value || ent->v.team <= 0 || ent->v.team != tr.ent->v.team))
	{
		VectorCopy (PR_GLOBAL(v_forward), G_VECTOR(OFS_RETURN));
		return;
	}

// try all possible entities
	VectorCopy (dir, bestdir);
	bestdist = sv_aim.value;
	bestent = NULL;

	check = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, check = NEXT_EDICT(check))
	{
		if (check->v.takedamage != dmgflag)
			continue;
		if (check == ent)
			continue;
		if (teamplay.value && ent->v.team > 0 && ent->v.team == check->v.team)
			continue;	// don't aim at teammate
		for (j=0 ; j<3 ; j++)
			end[j] = check->v.origin[j] + 0.5*(check->v.mins[j] + check->v.maxs[j]);
		VectorSubtract (end, start, dir);
		VectorNormalize (dir);

		dist = DotProduct (dir, PR_GLOBAL(v_forward));

		if (dist < bestdist)
			continue;	// too far to turn

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			save_hull = ent->v.hull;
			ent->v.hull = 0;
			tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
			ent->v.hull = save_hull;
		}
		else
	#endif
		tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);

		if (tr.ent == check)
		{	// can shoot at this one
			bestdist = dist;
			bestent = check;
		}
	}

	if (bestent)
	{
	#ifdef HEXEN2_SUPPORT
		// Since all origins are at the base, move the point to the middle of the victim model
		if (hexen2)
		{
			vec3_t hold_org;

			VectorCopy (bestent->v.origin, hold_org);
			hold_org[2] += 0.5 * bestent->v.maxs[2];
			VectorSubtract (hold_org, shot_org, dir);
		}
		else
	#endif
		VectorSubtract (bestent->v.origin, ent->v.origin, dir);

		dist = DotProduct (dir, PR_GLOBAL(v_forward));
		VectorScale (PR_GLOBAL(v_forward), dist, end);

		end[2] = dir[2];
		VectorNormalize (end);
		VectorCopy (end, G_VECTOR(OFS_RETURN));
	}
	else
	{
		VectorCopy (bestdir, G_VECTOR(OFS_RETURN));
	}
}

/*
==============
PF_changeyaw

This was a major timewaster in progs, so it was converted to C
==============
*/
void PF_changeyaw (void)
{
	edict_t	*ent;
	float	ideal, current, move, speed;

	ent = PROG_TO_EDICT(PR_GLOBAL(self));

	current = anglemod( ent->v.angles[1] );
	ideal = ent->v.ideal_yaw;
	speed = ent->v.yaw_speed;

	if (current == ideal)
	{
	#ifdef HEXEN2_SUPPORT
		G_FLOAT(OFS_RETURN) = 0;
	#endif
		return;
	}
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}

#ifdef HEXEN2_SUPPORT
	G_FLOAT(OFS_RETURN) = move;
#endif

	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->v.angles[1] = anglemod (current + move);
}

/*
==============
PF_changepitch
==============
*/
void PF_changepitch (void)
{
	edict_t	*ent;
	float	ideal, current, move, speed;
	eval_t	*val;

	ent = G_EDICT(OFS_PARM0);
	current = anglemod(ent->v.angles[0]);

/*	if ((val = GETEDICTFIELD(ent, eval_idealpitch)))
	{
		ideal = val->_float;
	}
	else
	{
		PR_RunError ("PF_changepitch: .float idealpitch and .float pitch_speed must be defined to use changepitch");
		return;
	}
*/
	ideal = ent->v.idealpitch;

	if ((val = GETEDICTFIELD(ent, eval_pitch_speed)))
	{
		speed = val->_float;
	}
	else
	{
		PR_RunError ("PF_changepitch: .float idealpitch and .float pitch_speed must be defined to use changepitch");
		return;
	}

	if (current == ideal)
		return;
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->v.angles[0] = anglemod (current + move);
}

/*
===============================================================================

MESSAGE WRITING

===============================================================================
*/

#define	MSG_BROADCAST	0	// unreliable to all
#define	MSG_ONE			1	// reliable to one (msg_entity)
#define	MSG_ALL			2	// reliable to all
#define	MSG_INIT		3	// write to the init string

sizebuf_t *WriteDest (void)
{
	int	entnum, dest;
	edict_t	*ent;

	dest = G_FLOAT(OFS_PARM0);
	switch (dest)
	{
	case MSG_BROADCAST:
		return &sv.datagram;

	case MSG_ONE:
		ent = PROG_TO_EDICT(PR_GLOBAL(msg_entity));
		entnum = NUM_FOR_EDICT(ent);
		if (entnum < 1 || entnum > svs.maxclients)
			PR_RunError ("WriteDest: not a client");
		return &svs.clients[entnum-1].message;

	case MSG_ALL:
		return &sv.reliable_datagram;

	case MSG_INIT:
		return &sv.signon;

	default:
		PR_RunError ("WriteDest: bad destination");
		break;
	}

	return NULL;
}

void PF_WriteByte (void)
{
	MSG_WriteByte (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteChar (void)
{
	MSG_WriteChar (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteShort (void)
{
	MSG_WriteShort (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteLong (void)
{
	MSG_WriteLong (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteAngle (void)
{
	MSG_WriteAngle (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteCoord (void)
{
	MSG_WriteCoord (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteString (void)
{
	MSG_WriteString (WriteDest(), G_STRING(OFS_PARM1));
}

void PF_WriteEntity (void)
{
	MSG_WriteShort (WriteDest(), G_EDICTNUM(OFS_PARM1));
}

// JDH: old Write functions from beta Quake (first arg is target client):

sizebuf_t *WriteDestEnt (void)
{
	int entnum = G_EDICTNUM(OFS_PARM0);

	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("WriteDestEnt: not a client");
	return &svs.clients[entnum-1].message;
}

void PF_WriteByteToEnt (void)
{
	MSG_WriteByte (WriteDestEnt(), G_FLOAT(OFS_PARM1));
}

void PF_WriteCharToEnt (void)
{
	MSG_WriteChar (WriteDestEnt(), G_FLOAT(OFS_PARM1));
}

void PF_WriteShortToEnt (void)
{
	MSG_WriteShort (WriteDestEnt(), G_FLOAT(OFS_PARM1));
}

void PF_WriteLongToEnt (void)
{
	MSG_WriteLong (WriteDestEnt(), G_FLOAT(OFS_PARM1));
}

void PF_WriteAngleToEnt (void)
{
	MSG_WriteAngle (WriteDestEnt(), G_FLOAT(OFS_PARM1));
}

void PF_WriteCoordToEnt (void)
{
	MSG_WriteCoord (WriteDestEnt(), G_FLOAT(OFS_PARM1));
}

void PF_WriteStringToEnt (void)
{
	MSG_WriteString (WriteDestEnt(), G_STRING(OFS_PARM1));
}

// JDH: bWrite functions from beta Quake:
//     (I'm guessing that bWrite is equivalent to modern Write with MSG_BROADCAST)
#define BWRITEDEST &sv.datagram

void PF_bWriteByte (void)
{
	MSG_WriteByte (BWRITEDEST, G_FLOAT(OFS_PARM0));
}

void PF_bWriteChar (void)
{
	MSG_WriteChar (BWRITEDEST, G_FLOAT(OFS_PARM0));
}

void PF_bWriteShort (void)
{
	MSG_WriteShort (BWRITEDEST, G_FLOAT(OFS_PARM0));
}

void PF_bWriteLong (void)
{
	MSG_WriteLong (BWRITEDEST, G_FLOAT(OFS_PARM0));
}

void PF_bWriteAngle (void)
{
	MSG_WriteAngle (BWRITEDEST, G_FLOAT(OFS_PARM0));
}

void PF_bWriteCoord (void)
{
	MSG_WriteCoord (BWRITEDEST, G_FLOAT(OFS_PARM0));
}

void PF_bWriteString (void)
{
	MSG_WriteString (BWRITEDEST, G_STRING(OFS_PARM0));
}

void PF_bWriteEntity (void)
{
	MSG_WriteShort (BWRITEDEST, G_EDICTNUM(OFS_PARM0));
}

//=============================================================================

extern qboolean MSG_WriteModelIndex (sizebuf_t *sb, int index, const client_t *client);

void MSG_WriteSpawnstatic (sizebuf_t *dest, edict_t	*ent)
{
	int	i;

	MSG_WriteCmd (dest, svc_spawnstatic);

	if (MSG_WriteModelIndex (dest, SV_ModelIndex(pr_strings + ent->v.model, true), NULL))
	{

		MSG_WriteByte (dest, ent->v.frame);
		MSG_WriteByte (dest, ent->v.colormap);
		MSG_WriteByte (dest, ent->v.skin);

		#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			MSG_WriteByte (dest, (int)(ent->v.scale*100.0)&255);
			MSG_WriteByte (dest, ent->v.drawflags);
			MSG_WriteByte (dest, (int)(ent->v.abslight*255.0)&255);
		}
		#endif

		for (i=0 ; i<3 ; i++)
		{
			MSG_WriteCoord (dest, ent->v.origin[i]);
			MSG_WriteAngle (dest, ent->v.angles[i]);
		}
	}
	else
	{
		// undo the WriteByte
		dest->cursize--;
	}
}

void PF_makestatic (void)
{
	edict_t *ent = G_EDICT(OFS_PARM0);

	MSG_WriteSpawnstatic (&sv.signon, ent);

// JDH: if there's a local client running, send it a message
	if (svs.clients[0].spawned && svs.clients[0].netconnection && !svs.clients[0].netconnection->socket)
		MSG_WriteSpawnstatic (&svs.clients[0].message, ent);

// throw the entity away now
	ED_Free (ent);
}

//=============================================================================

/*
==============
PF_setspawnparms
==============
*/
void PF_setspawnparms (void)
{
	edict_t		*ent;
	int		i;
	client_t	*client;

	ent = G_EDICT(OFS_PARM0);
	i = NUM_FOR_EDICT(ent);
	if (i < 1 || i > svs.maxclients)
		PR_RunError ("Entity is not a client");

	// copy spawn parms out of the client_t
	client = svs.clients + (i-1);

	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		(pr_global_ptrs.parm1)[i] = client->spawn_parms[i];
}

/*
==============
PF_changelevel
==============
*/
void PF_changelevel (void)
{
	char	*s;

// make sure we don't issue two changelevels
	if (svs.changelevel_issued)
		return;
	svs.changelevel_issued = true;

	s = G_STRING(OFS_PARM0);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		char *cmd, *s2 = G_STRING(OFS_PARM1);

		if ((int)*pr_global_ptrs.serverflags & (SFL_NEW_UNIT | SFL_NEW_EPISODE))
			cmd = "changelevel";
		else
			cmd = "changelevel2";

		Cbuf_AddText (va("%s %s %s\n", cmd, s, s2), SRC_COMMAND);
	}
	else
#endif
	Cbuf_AddText (va("changelevel %s\n", s), SRC_COMMAND);
}

void PF_sin (void)
{
	G_FLOAT(OFS_RETURN) = sin(G_FLOAT(OFS_PARM0));
}

void PF_cos (void)
{
	G_FLOAT(OFS_RETURN) = cos(G_FLOAT(OFS_PARM0));
}

void PF_sqrt (void)
{
	G_FLOAT(OFS_RETURN) = sqrt(G_FLOAT(OFS_PARM0));
}

/*
=================
VM_min

returns the minimum of two (or more) supplied floats

float min(float a, float b, ...[float])
=================
*/
void PF_min (void)
{
	int i;
	float minval;

	minval = G_FLOAT(OFS_PARM0);
	for (i=1 ; i<pr_argc ; i++)
	{
		minval = min(G_FLOAT(OFS_PARM0 + i*3), minval);
	}

	G_FLOAT(OFS_RETURN) = minval;
}

/*
=================
VM_max

returns the maximum of two (or more) supplied floats

float max(float a, float b, ...[float])
=================
*/
void PF_max (void)
{
	int i;
	float maxval;

	maxval = G_FLOAT(OFS_PARM0);
	for (i=1 ; i<pr_argc ; i++)
	{
		maxval = max(G_FLOAT(OFS_PARM0 + i*3), maxval);
	}

	G_FLOAT(OFS_RETURN) = maxval;
}

/*
=================
VM_bound

returns number bounded by supplied range

float	bound(float min, float value, float max)
=================
*/
void PF_bound (void)
{
	G_FLOAT(OFS_RETURN) = bound(G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1), G_FLOAT(OFS_PARM2));
}

void PF_pow (void)
{
	G_FLOAT(OFS_RETURN) = pow(G_FLOAT(OFS_PARM0), G_FLOAT(OFS_PARM1));
}

void PF_Fixme (void)
{
	PR_RunError ("unimplemented builtin");
}

/*
==============
PF_checkextension

returns true if the extension is supported by the server

checkextension(extensionname)
==============
*/
void PF_checkextension (void)
{
	char *name;
	int i;

	name = G_STRING(OFS_PARM0);
	for (i = 0; pr_extensionlist[i]; i++)
	{
		if (!Q_strcasecmp(name, pr_extensionlist[i]))
		{
			G_FLOAT(OFS_RETURN) = 1;
			return;
		}
	}

	G_FLOAT(OFS_RETURN) = 0;		// not supported
}

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start

/*

=================
PF_builtin_find
float builtin_find (string)
=================
*/
void PF_builtin_find (void)
{
	int		j;
	float	funcno;
	char	*funcname;

	funcno = 0;
	funcname = G_STRING(OFS_PARM0);
	// search function name
	for ( j=1 ; j < pr_ebfs_numbuiltins ; j++)
	{
		if ((pr_ebfs_builtins[j].funcname) && (!(Q_strcasecmp(funcname,pr_ebfs_builtins[j].funcname))))
		{
			break;	// found
		}
	}

	if (j < pr_ebfs_numbuiltins)
	{
		funcno = pr_ebfs_builtins[j].funcno;
	}
	G_FLOAT(OFS_RETURN) = funcno;
}

#ifdef ETWAR_SUPPORT
/*
=================
PF_strlen

float PF_strlen(string)
=================
*/
void PF_strlen (void)
{
	G_FLOAT(OFS_RETURN) = strlen( G_STRING(OFS_PARM0) );
}

/*
=================
PF_strcat

string PF_strcat(str1, str2)
=================
*/
void PF_strcat (void)
{
	Q_snprintfz (pr_string_temp, sizeof(pr_string_temp), "%s%s", G_STRING(OFS_PARM0), G_STRING(OFS_PARM1));
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

/*
=================
PF_stof

float PF_stof(string)
=================
*/
void PF_stof (void)
{
	G_FLOAT(OFS_RETURN) = Q_atof (G_STRING(OFS_PARM0));
}

/*
=================
PF_stov

vector PF_stov(string)
=================
*/
void PF_stov (void)
{
	char	string[128];
	char	*a, *b;
	int		i;
	vec3_t	v;

	Q_strcpy(string, G_STRING(OFS_PARM0), sizeof(string));
	a = b = string;
	for (i = 0; i < 3; i++)
	{
		while (*a && *a != ' ')
			a++;
		*a = 0;
		v[i] = Q_atof (b);
		b = a = a+1;
	}

	VectorCopy (v, G_VECTOR(OFS_RETURN));
}

/*
=================
PF_open

float PF_open(string, float)
=================
*/
void PF_open (void)
{
	float	mode;
	char	s[4];
	FILE	*f;

	mode = G_FLOAT(OFS_PARM1);
	s[2] = 0;
	s[1] = 'b';
	s[0] = (mode == 2) ? 'w' : (mode == 1) ? 'a' : 'r';

	f = fopen( va("%s/%s", com_gamedir, G_STRING(OFS_PARM0)), s );
	G_FLOAT(OFS_RETURN) = (f ? (int) f : -1);
}

/*
=================
PF_close

void PF_close(float)
=================
*/
void PF_close (void)
{
	fclose( (FILE *)(int)G_FLOAT(OFS_PARM0));
}

/*
=================
PF_read

string PF_read(float)
=================
*/
void PF_read (void)
{
	// JDH: not sure if this is how EW expects this to work...
	if (fgets (pr_string_temp, sizeof(pr_string_temp), (FILE *)(int)G_FLOAT(OFS_PARM0)))
	{
		int len = strlen(pr_string_temp);
		if ((len > 0) && (pr_string_temp[len-1] == '\n'))
		{
			pr_string_temp[--len] = 0;
			if ((len > 0) && (pr_string_temp[len-1] == '\r'))
				pr_string_temp[--len] = 0;
		}
	}

	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

/*
=================
PF_update_wmode

float PF_update_wmode(float)
=================
*/
void PF_update_wmode (void)
{
	G_FLOAT(OFS_RETURN) = G_FLOAT(OFS_PARM0);
}

/*
=================
PF_get_numbots

float PF_get_numbots(float min, float val, float max)
=================
*/
void PF_get_numbots (void)
{
	G_FLOAT(OFS_RETURN) = 0;
}

#endif	// #ifdef ETWAR_SUPPORT


// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end

/*
builtin_t pr_builtin[] =
{
PF_Fixme,
PF_makevectors,	// void(entity e)	makevectors 		= #1;
PF_setorigin,	// void(entity e, vector o) setorigin	= #2;
PF_setmodel,	// void(entity e, string m) setmodel	= #3;
PF_setsize,	// void(entity e, vector min, vector max) setsize = #4;
PF_Fixme,	// void(entity e, vector min, vector max) setabssize = #5;
PF_break,	// void() break						= #6;
PF_random,	// float() random						= #7;
PF_sound,	// void(entity e, float chan, string samp) sound = #8;
PF_normalize,	// vector(vector v) normalize			= #9;
PF_error,	// void(string e) error				= #10;
PF_objerror,	// void(string e) objerror				= #11;
PF_vlen,	// float(vector v) vlen				= #12;
PF_vectoyaw,	// float(vector v) vectoyaw		= #13;
PF_Spawn,	// entity() spawn						= #14;
PF_Remove,	// void(entity e) remove				= #15;
PF_traceline,	// float(vector v1, vector v2, float tryents) traceline = #16;
PF_checkclient,	// entity() clientlist					= #17;
PF_Find,	// entity(entity start, .string fld, string match) find = #18;
PF_precache_sound,	// void(string s) precache_sound		= #19;
PF_precache_model,	// void(string s) precache_model		= #20;
PF_stuffcmd,	// void(entity client, string s)stuffcmd = #21;
PF_findradius,	// entity(vector org, float rad) findradius = #22;
PF_bprint,	// void(string s) bprint				= #23;
PF_sprint,	// void(entity client, string s) sprint = #24;
PF_dprint,	// void(string s) dprint				= #25;
PF_ftos,	// void(string s) ftos				= #26;
PF_vtos,	// void(string s) vtos				= #27;
PF_coredump,
PF_traceon,
PF_traceoff,
PF_eprint,	// void(entity e) debug print an entire entity
PF_walkmove, // float(float yaw, float dist) walkmove
PF_Fixme, // float(float yaw, float dist) walkmove
PF_droptofloor,
PF_lightstyle,
PF_rint,
PF_floor,
PF_ceil,
PF_Fixme,
PF_checkbottom,
PF_pointcontents,
PF_Fixme,
PF_fabs,
PF_aim,
PF_cvar,
PF_localcmd,
PF_nextent,
PF_particle,
PF_changeyaw,
PF_Fixme,
PF_vectoangles,

PF_WriteByte,
PF_WriteChar,
PF_WriteShort,
PF_WriteLong,
PF_WriteCoord,
PF_WriteAngle,
PF_WriteString,
PF_WriteEntity,

PF_sin,
PF_cos,
PF_sqrt,
PF_changepitch,
PF_TraceToss, // 65?
PF_etos,
PF_Fixme,

SV_MoveToGoal,
PF_precache_file,
PF_makestatic,

PF_changelevel,
PF_Fixme,

PF_cvar_set,
PF_centerprint,

PF_ambientsound,

PF_precache_model,
PF_precache_sound,		// precache_sound2 is different only for qcc
PF_precache_file,

PF_setspawnparms
};
*/

builtin_t *pr_builtins;
int pr_numbuiltins;

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start
// for builtin function definitions see Quake Standards Group at http://www.quakesrc.org/

ebfs_builtin_t pr_ebfs_builtins[] =
{
	{   0, NULL, PF_Fixme },		// has to be first entry as it is needed for initialization in PR_LoadProgs()
	{   1, "makevectors", PF_makevectors },	// void(vector v)	makevectors 		= #1;
	{   2, "setorigin", PF_setorigin },		// void(entity e, vector o) setorigin	= #2;
	{   3, "setmodel", PF_setmodel },		// void(entity e, string m) setmodel	= #3;
	{   4, "setsize", PF_setsize },			// void(entity e, vector min, vector max) setsize = #4;
//	{   5, "fixme", PF_Fixme },				// void(entity e, vector min, vector max) setabssize = #5;
	{   6, "break", PF_break },				// void() break						= #6;
	{   7, "random", PF_random },			// float() random						= #7;
	{   8, "sound", PF_sound },				// void(entity e, float chan, string samp, float vol, float atten) sound = #8;
	{   9, "normalize", PF_normalize },		// vector(vector v) normalize			= #9;
	{  10, "error", PF_error },				// void(string e) error				= #10;
	{  11, "objerror", PF_objerror },		// void(string e) objerror				= #11;
	{  12, "vlen", PF_vlen },				// float(vector v) vlen				= #12;
	{  13, "vectoyaw", PF_vectoyaw },		// float(vector v) vectoyaw		= #13;
	{  14, "spawn", PF_Spawn },				// entity() spawn						= #14;
	{  15, "remove", PF_Remove },			// void(entity e) remove				= #15;
	{  16, "traceline", PF_traceline },		// void(vector v1, vector v2, float, entity) traceline = #16;
	{  17, "checkclient", PF_checkclient },	// entity() clientlist					= #17;
	{  18, "find", PF_Find },				// entity(entity start, .string fld, string match) find = #18;
	{  19, "precache_sound", PF_precache_sound },	// string(string s) precache_sound		= #19;
	{  20, "precache_model", PF_precache_model },	// string(string s) precache_model		= #20;
	{  21, "stuffcmd", PF_stuffcmd },		// void(entity client, string s)stuffcmd = #21;
	{  22, "findradius", PF_findradius },	// entity(vector org, float rad) findradius = #22;
	{  23, "bprint", PF_bprint },			// void(string s) bprint				= #23;
	{  24, "sprint", PF_sprint },			// void(entity client, string s) sprint = #24;
	{  25, "dprint", PF_dprint },			// void(string s) dprint				= #25;
	{  26, "ftos", PF_ftos },				// string(float f) ftos				= #26;
	{  27, "vtos", PF_vtos },				// string(vector v) vtos				= #27;
	{  28, "coredump", PF_coredump },
	{  29, "traceon", PF_traceon },
	{  30, "traceoff", PF_traceoff },
	{  31, "eprint", PF_eprint },			// void(entity e) eprint - debug print an entire entity
	{  32, "walkmove", PF_walkmove },		// float(float yaw, float dist) walkmove
//	{  33, "fixme", PF_Fixme },
	{  34, "droptofloor", PF_droptofloor },
	{  35, "lightstyle", PF_lightstyle },
	{  36, "rint", PF_rint },
	{  37, "floor", PF_floor },
	{  38, "ceil", PF_ceil },
//	{  39, "fixme", PF_Fixme },
	{  40, "checkbottom", PF_checkbottom },
	{  41, "pointcontents", PF_pointcontents },
//	{  42, "fixme", PF_Fixme },
	{  43, "fabs", PF_fabs },
	{  44, "aim", PF_aim },
	{  45, "cvar", PF_cvar },
	{  46, "localcmd", PF_localcmd },
	{  47, "nextent", PF_nextent },
	{  48, "particle", PF_particle },
	{  49, "ChangeYaw", PF_changeyaw },
//	{  50, "fixme", PF_Fixme },
	{  51, "vectoangles", PF_vectoangles },
	{  52, "WriteByte", PF_WriteByte },
	{  53, "WriteChar", PF_WriteChar },
	{  54, "WriteShort", PF_WriteShort },
	{  55, "WriteLong", PF_WriteLong },
	{  56, "WriteCoord", PF_WriteCoord },
	{  57, "WriteAngle", PF_WriteAngle },
	{  58, "WriteString", PF_WriteString },
	{  59, "WriteEntity", PF_WriteEntity },

//#ifdef QUAKE2

	{  60, "sin", PF_sin },
	{  61, "cos", PF_cos },
	{  62, "sqrt", PF_sqrt },
	{  63, "changepitch", PF_changepitch },
	{  64, "tracetoss", PF_tracetoss },
	{  65, "etos", PF_etos },
//	{  66, "WaterMove", PF_WaterMove },

//#endif

	{  67, "movetogoal", SV_MoveToGoal },
	{  68, "precache_file", PF_precache_file },
	{  69, "makestatic", PF_makestatic },
	{  70, "changelevel", PF_changelevel },
//	{  71, "fixme", PF_Fixme },
	{  72, "cvar_set", PF_cvar_set },
	{  73, "centerprint", PF_centerprint },
	{  74, "ambientsound", PF_ambientsound },
	{  75, "precache_model2", PF_precache_model },
	{  76, "precache_sound2", PF_precache_sound },	// precache_sound2 is different only for qcc
	{  77, "precache_file2", PF_precache_file },
	{  78, "setspawnparms", PF_setspawnparms },
//	{  81, "stof", PF_stof },	// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes

#ifdef ETWAR_SUPPORT
	{  81, "strlen", PF_strlen },
	{  82, "strcat", PF_strcat },
//	{  83, "substring", PF_substring },
	{  84, "stof", PF_stof },
	{  85, "stov", PF_stov },
	{  86, "open", PF_open },
	{  87, "close", PF_close },
//	{  88, "read", PF_read },
//	{  89, "write", PF_write },
//	{  90, "init_draw", PF_init_draw },
//	{  91, "stop_draw", PF_stop_draw },
//	{  92, "draw_coord", PF_draw_coord },
//	{  93, "draw_center", PF_draw_center },
//	{  94, "CoordPrint", PF_CoordPrint },
	{  95, "update_wmode", PF_update_wmode },
	{  96, "get_numbots", PF_get_numbots },
//	{  97, "set_numbots", PF_set_numbots },
#endif

// 2001-11-15 DarkPlaces general builtin functions by Lord Havoc  start

// not implemented yet


	{  90, "tracebox", PF_tracebox },
/*	{  91, "randomvec", PF_randomvec },
	{  92, "getlight", PF_GetLight },	// not implemented yet
	{  93, "cvar_create", PF_cvar_create },		// 2001-09-18 New BuiltIn Function: cvar_create() by Maddes
*/
	{  94, "min", PF_min },
	{  95, "max", PF_max },
	{  96, "fbound", PF_bound },
	{  97, "pow", PF_pow },
/*	{  98, "findfloat", PF_FindFloat },
	{ PR_DEFAULT_FUNCNO_EXTENSION_FIND, "extension_find", PF_extension_find },	// 2001-10-20 Extension System by Lord Havoc/Maddes
	{   0, "registercvar", PF_cvar_create },	// 0 indicates that this entry is just for remapping (because of name change)
	{   0, "checkextension", PF_extension_find },
*/
	{ PR_DEFAULT_FUNCNO_EXTENSION_FIND, "checkextension", PF_checkextension },		// #99 float(string s) checkextension (the basis of the extension system)

// 2001-11-15 DarkPlaces general builtin functions by Lord Havoc  end

	{ PR_DEFAULT_FUNCNO_BUILTIN_FIND, "builtin_find", PF_builtin_find },	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes

// not implemented yet

/*
	{ 101, "cmd_find", PF_cmd_find },		// 2001-09-16 New BuiltIn Function: cmd_find() by Maddes
	{ 102, "cvar_find", PF_cvar_find },		// 2001-09-16 New BuiltIn Function: cvar_find() by Maddes
	{ 103, "cvar_string", PF_cvar_string },	// 2001-09-16 New BuiltIn Function: cvar_string() by Maddes
	{ 105, "cvar_free", PF_cvar_free },		// 2001-09-18 New BuiltIn Function: cvar_free() by Maddes
	{ 106, "NVS_InitSVCMsg", PF_NVS_InitSVCMsg },	// 2000-05-02 NVS SVC by Maddes
	{ 107, "WriteFloat", PF_WriteFloat },	// 2001-09-16 New BuiltIn Function: WriteFloat() by Maddes
	{ 108, "etof", PF_etof },	// 2001-09-25 New BuiltIn Function: etof() by Maddes
	{ 109, "ftoe", PF_ftoe },	// 2001-09-25 New BuiltIn Function: ftoe() by Maddes
*/

// 2001-09-20 QuakeC file access by FrikaC/Maddes  start

// not implemented yet

/*
	{ 110, "fopen", PF_fopen },			// float(string filename, float mode) fopen (FRIK_FILE)
	{ 111, "fclose", PF_fclose },		// void(float fhandle) fclose (FRIK_FILE)
	{ 112, "fgets", PF_fgets },			// string(float fhandle) fgets (FRIK_FILE)
	{ 113, "fputs", PF_fputs },			// void(float fhandle, string s) fputs (FRIK_FILE)
	{ 114, "strlen", PF_strlen },		// float(string s) strlen (FRIK_FILE)
	{ 115, "strcat", PF_strcat },		// string(string s1, string s2) strcat (FRIK_FILE)
	{ 116, "substring", PF_substring },	// string(string s, float start, float length) substring (FRIK_FILE)
	{ 117, "stov", PF_stov },			// vector(string) stov (FRIK_FILE)
	{ 118, "strzone", PF_strzone },		// string(string s) strzone (FRIK_FILE)
	{ 118, "strunzone", PF_strunzone }, // void(string s) strunzone (FRIK_FILE)
*/

// 2001-09-20 QuakeC file access by FrikaC/Maddes  end

// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes  start

// not implemented yet

// 2001-09-20 QuakeC string manipulation by FrikaC/Maddes  end

// 2001-11-15 DarkPlaces general builtin functions by Lord Havoc  start

// not implemented yet

/*
	{ 400, "copyentity", PF_copyentity },							// #400 void(entity from, entity to) copyentity (DP_QC_COPYENTITY)
	{ 401, "setcolor", PF_setcolor },								// #401 void(entity ent, float colors) setcolor (DP_QC_SETCOLOR)
	{ 402, "findchain", PF_findchain },								// #402 entity(.string fld, string match) findchain (DP_QC_FINDCHAIN)
	{ 403, "findchainfloat", PF_findchainfloat },					// #403 entity(.float fld, float match) findchainfloat (DP_QC_FINDCHAINFLOAT)
	{ 404, "effect", PF_effect },									// #404 void(vector org, string modelname, float startframe, float endframe, float framerate) effect (DP_SV_EFFECT)
*/
	{ 405, "te_blood", PF_te_blood },								// #405 void(vector org, vector velocity, float howmany) te_blood (DP_TE_BLOOD)
/*	{ 406, "te_bloodshower", PF_te_bloodshower },					// #406 void(vector mincorner, vector maxcorner, float explosionspeed, float howmany) te_bloodshower (DP_TE_BLOODSHOWER)
	{ 407, "te_explosionrgb", PF_te_explosionrgb },					// #407 void(vector org, vector color) te_explosionrgb (DP_TE_EXPLOSIONRGB)
	{ 408, "te_particlecube", PF_te_particlecube },					// #408 void(vector mincorner, vector maxcorner, vector vel, float howmany, float color, float gravityflag, float randomveljitter) te_particlecube (DP_TE_PARTICLECUBE)
	{ 409, "te_particlerain", PF_te_particlerain },					// #409 void(vector mincorner, vector maxcorner, vector vel, float howmany, float color) te_particlerain (DP_TE_PARTICLERAIN)
	{ 410, "te_particlesnow", PF_te_particlesnow },					// #410 void(vector mincorner, vector maxcorner, vector vel, float howmany, float color) te_particlesnow (DP_TE_PARTICLESNOW)
	{ 411, "te_spark", PF_te_spark },								// #411 void(vector org, vector vel, float howmany) te_spark (DP_TE_SPARK)
	{ 412, "te_gunshotquad", PF_te_gunshotquad },					// #412 void(vector org) te_gunshotquad (DP_QUADEFFECTS1)
	{ 413, "te_spikequad", PF_te_spikequad },						// #413 void(vector org) te_spikequad (DP_QUADEFFECTS1)
	{ 414, "te_superspikequad", PF_te_superspikequad },				// #414 void(vector org) te_superspikequad (DP_QUADEFFECTS1)
	{ 415, "te_explosionquad", PF_te_explosionquad },				// #415 void(vector org) te_explosionquad (DP_QUADEFFECTS1)
	{ 416, "te_smallflash", PF_te_smallflash },						// #416 void(vector org) te_smallflash (DP_TE_SMALLFLASH)
	{ 417, "te_customflash", PF_te_customflash },					// #417 void(vector org, float radius, float lifetime, vector color) te_customflash (DP_TE_CUSTOMFLASH)
	{ 418, "te_gunshot", PF_te_gunshot },							// #418 void(vector org) te_gunshot (DP_TE_STANDARDEFFECTBUILTINS)
	{ 419, "te_spike", PF_te_spike },								// #419 void(vector org) te_spike (DP_TE_STANDARDEFFECTBUILTINS)
	{ 420, "te_superspike", PF_te_superspike },						// #420 void(vector org) te_superspike (DP_TE_STANDARDEFFECTBUILTINS)
	{ 421, "te_explosion", PF_te_explosion },						// #421 void(vector org) te_explosion (DP_TE_STANDARDEFFECTBUILTINS)
	{ 422, "te_tarexplosion", PF_te_tarexplosion },					// #422 void(vector org) te_tarexplosion (DP_TE_STANDARDEFFECTBUILTINS)
	{ 423, "te_wizspike", PF_te_wizspike },							// #423 void(vector org) te_wizspike (DP_TE_STANDARDEFFECTBUILTINS)
	{ 424, "te_knightspike", PF_te_knightspike },					// #424 void(vector org) te_knightspike (DP_TE_STANDARDEFFECTBUILTINS)
	{ 425, "te_lavasplash", PF_te_lavasplash },						// #425 void(vector org) te_lavasplash (DP_TE_STANDARDEFFECTBUILTINS)
	{ 426, "te_teleport", PF_te_teleport },							// #426 void(vector org) te_teleport (DP_TE_STANDARDEFFECTBUILTINS)
	{ 427, "te_explosion2", PF_te_explosion2 },						// #427 void(vector org, float color) te_explosion2 (DP_TE_STANDARDEFFECTBUILTINS)
	{ 428, "te_lightning1", PF_te_lightning1 },						// #428 void(entity own, vector start, vector end) te_lightning1 (DP_TE_STANDARDEFFECTBUILTINS)
	{ 429, "te_lightning2", PF_te_lightning2 },						// #429 void(entity own, vector start, vector end) te_lightning2 (DP_TE_STANDARDEFFECTBUILTINS)
	{ 430, "te_lightning3", PF_te_lightning3 },						// #430 void(entity own, vector start, vector end) te_lightning3 (DP_TE_STANDARDEFFECTBUILTINS)
	{ 431, "te_beam", PF_te_beam },									// #431 void(entity own, vector start, vector end) te_beam (DP_TE_STANDARDEFFECTBUILTINS)
	{ 432, "vectorvectors", PF_vectorvectors },						// #432 void(vector dir) vectorvectors (DP_QC_VECTORVECTORS)
	{ 433, "plasmaburn", PF_te_plasmaburn },						// #433 void(vector org) te_plasmaburn (DP_TE_PLASMABURN)
	{ 434, "getsurfacenumpoints", PF_getsurfacenumpoints },			// #434 float(entity e, float s) getsurfacenumpoints (DP_QC_GETSURFACE)
	{ 435, "getsurfacepoint", PF_getsurfacepoint },					// #435 vector(entity e, float s, float n) getsurfacepoint (DP_QC_GETSURFACE)
	{ 436, "getsurfacenormal", PF_getsurfacenormal },				// #436 vector(entity e, float s) getsurfacenormal (DP_QC_GETSURFACE)
	{ 437, "getsurfacetexture", PF_getsurfacetexture },				// #437 string(entity e, float s) getsurfacetexture (DP_QC_GETSURFACE)
	{ 438, "getsurfacenearpoint", PF_getsurfacenearpoint },			// #438 float(entity e, vector p) getsurfacenearpoint (DP_QC_GETSURFACE)
	{ 439, "getsurfaceclippedpoint", PF_getsurfaceclippedpoint },	// #439 vector(entity e, float s, vector p) getsurfaceclippedpoint (DP_QC_GETSURFACE)
	{ 440, "clientcommand", PF_clientcommand },						// #440 void(entity e, string s) clientcommand (KRIMZON_SV_PARSECLIENTCOMMAND)
	{ 441, "tokensize", PF_tokenize },								// #441 float(string s) tokenize (KRIMZON_SV_PARSECLIENTCOMMAND)
	{ 442, "argv", PF_argv },										// #442 string(float n) argv (KRIMZON_SV_PARSECLIENTCOMMAND)
	{ 443, "setattachment", PF_setattachment },						// #443 void(entity e, entity tagentity, string tagname) setattachment (DP_GFX_QUAKE3MODELTAGS)
	{ 444, "search_begin", PF_search_begin },						// #444 float(string pattern, float caseinsensitive, float quiet) search_begin (DP_FS_SEARCH)
	{ 445, "search_end", PF_search_end },							// #445 void(float handle) search_end (DP_FS_SEARCH)
	{ 446, "search_getsize", PF_search_getsize },					// #446 float(float handle) search_getsize (DP_FS_SEARCH)
	{ 447, "search_getfilename", PF_search_getfilename },			// #447 string(float handle, float num) search_getfilename (DP_FS_SEARCH)
	{ 448, "cvar_string", PF_cvar_string },							// #448 string(string s) cvar_string (DP_QC_CVAR_STRING)
	{ 449, "findflags", PF_findflags },								// #449 entity(entity start, .float fld, float match) findflags (DP_QC_FINDFLAGS)
	{ 450, "findchainflags", PF_findchainflags },					// #450 entity(.float fld, float match) findchainflags (DP_QC_FINDCHAINFLAGS)
	{ 451, "gettagindex", PF_gettagindex },							// #451 float(entity ent, string tagname) gettagindex (DP_QC_GETTAGINFO)
	{ 452, "gettaginfo", PF_gettaginfo },							// #452 vector(entity ent, float tagindex) gettaginfo (DP_QC_GETTAGINFO)
	{ 453, "dropclient", PF_dropclient },							// #453 void(entity clent) dropclient (DP_SV_DROPCLIENT)
	{ 454, "spawnclient", PF_spawnclient },							// #454 entity() spawnclient (DP_SV_BOTCLIENT)
	{ 455, "clienttype", PF_clienttype },							// #455 float(entity clent) clienttype (DP_SV_BOTCLIENT)
*/

// 2001-11-15 DarkPlaces general builtin functions by Lord Havoc  end

};

int pr_ebfs_numbuiltins = sizeof(pr_ebfs_builtins)/sizeof(pr_ebfs_builtins[0]);

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end


// JDH: the builtins from beta Quake that differ from release Quake:
ebfs_builtin_t pr_builtins_beta[] =
{
	{ 52,	"WriteByte",	PF_WriteByteToEnt },		// void (entity to, float f) bWriteByte = #52
	{ 53,	"WriteChar",	PF_WriteCharToEnt },		// void (entity to, float f) bWriteChar = #53
	{ 54,	"WriteShort",	PF_WriteShortToEnt },		// void (entity to, float f) bWriteShort = #54
	{ 55,	"WriteLong",	PF_WriteLongToEnt },		// void (entity to, float f) bWriteLong = #55
	{ 56,	"WriteCoord",	PF_WriteCoordToEnt },		// void (entity to, float f) bWriteCoord = #56
	{ 57,	"WriteAngle",	PF_WriteAngleToEnt },		// void (entity to, float f) bWriteAngle = #57
	{ 58,	"WriteString",	PF_WriteStringToEnt },		// void (entity to, string s) bWriteString = #58
	{ 59,	"bWriteByte",	PF_bWriteByte },		// void (float f) bWriteByte = #59
	{ 60,	"bWriteChar",	PF_bWriteChar },		// void (float f) bWriteChar = #60
	{ 61,	"bWriteShort",	PF_bWriteShort },		// void (float f) bWriteShort = #61
	{ 62,	"bWriteLong",	PF_bWriteLong },		// void (float f) bWriteLong = #62
	{ 63,	"bWriteCoord",	PF_bWriteCoord },		// void (float f) bWriteCoord = #63
	{ 64,	"bWriteAngle",	PF_bWriteAngle },		// void (float f) bWriteAngle = #64
	{ 65,	"bWriteString",	PF_bWriteString },		// void (string s) bWriteString = #65
	{ 66,	"bWriteEntity",	PF_bWriteEntity }		// void (entity e) bWriteEntity = #66
};

int pr_numbuiltins_beta = sizeof(pr_builtins_beta)/sizeof(pr_builtins_beta[0]);

