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
// cl_tent.c -- client side temporary entities

#include "quakedef.h"

#ifndef RQM_SV_ONLY

//#define	MAX_BEAMS	24
#define	MAX_BEAMS	64
typedef struct
{
	int		entity;
	struct model_s	*model;
	float		starttime, endtime;
	vec3_t		start, end;
} beam_t;


int		num_temp_entities;
entity_t	cl_temp_entities[MAX_TEMP_ENTITIES];
beam_t		cl_beams[MAX_BEAMS];

static	vec3_t	playerbeam_end;		// added by joe

#define BEAM_INACTIVE(b) ((cl.time < (b)->starttime) || (cl.time > (b)->endtime)) 

//float		ExploColor[3];		// joe: for color mapped explosions

model_t		*cl_bolt1_mod, *cl_bolt2_mod, *cl_bolt3_mod;

sfx_t		*cl_sfx_wizhit;
sfx_t		*cl_sfx_knighthit;
sfx_t		*cl_sfx_tink1;
sfx_t		*cl_sfx_ric1;
sfx_t		*cl_sfx_ric2;
sfx_t		*cl_sfx_ric3;
sfx_t		*cl_sfx_r_exp3;


#ifdef HEXEN2_SUPPORT

#define MAX_STREAMS				32
#define MAX_STREAM_ENTITIES		128
#define STREAM_ATTACHED			16

typedef struct
{
	int type;
	int entity;
	int tag;
	int flags;
	int skin;
	struct model_s *models[4];
	vec3_t source;
	vec3_t dest;
	vec3_t offset;
	float startTime;		// JDH
	float endTime;
	float lastTrailTime;
} stream_t;

#define STREAM_INACTIVE(s) ((cl.time < (s)->startTime) || (cl.time > (s)->endTime))

static stream_t cl_Streams[MAX_STREAMS];
static entity_t StreamEntities[MAX_STREAM_ENTITIES];
static int StreamEntityCount;

#endif	// #ifdef HEXEN2_SUPPORT


#ifdef _DEBUG

typedef struct demobeam_s
{
	beam_t beam;
	struct demobeam_s *next;
} demobeam_t;

demobeam_t *cl_demobeams;

void CL_PushBeam (beam_t *b)
{
	demobeam_t *curr, *prev, *db;
	
	curr = cl_demobeams;
	prev = NULL;
	while (curr)
	{
		if (b->endtime > curr->beam.endtime)
			break;
		prev = curr;
		curr = curr->next;
	}
	
	db = Q_malloc (sizeof(demobeam_t));
	db->beam = *b;
	db->next = curr;
	if (prev)
		prev->next = db;
	else
		cl_demobeams = db;
}
#endif

/*
=================
CL_InitTEnts
=================
*/
void CL_InitTEnts (void)
{
	cl_sfx_wizhit = S_PrecacheSound ("wizard/hit.wav");
	cl_sfx_knighthit = S_PrecacheSound ("hknight/hit.wav");
	cl_sfx_tink1 = S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1 = S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2 = S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3 = S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3 = S_PrecacheSound ("weapons/r_exp3.wav");
}

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void)
{
	memset (cl_temp_entities, 0, sizeof(cl_temp_entities));
	memset (&cl_beams, 0, sizeof(cl_beams));

#ifdef HEXEN2_SUPPORT
	memset(cl_Streams, 0, sizeof(cl_Streams));
#endif
}
	
/*
=================
CL_ParseBeam
=================
*/
void CL_ParseBeam (const char *modelname, qboolean parse_only)
{
	int		i, ent, index;
	vec3_t	start, end;
	beam_t	*b;
	
	ent = MSG_ReadShort ();
	
	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();
	
	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

	if (parse_only)
		return;		// JDH: parse message only, don't do anything
	
	
	if (ent == cl.viewentity)
		VectorCopy (end, playerbeam_end);	// for cl_truelightning

	index = MAX_BEAMS;

	for (i = 0, b = cl_beams ; i < MAX_BEAMS ; i++, b++)
	{
		// override any beam with the same entity
		if (b->entity == ent)
		{
			index = i;
			break;
		}
		
		// make note of first available slot, but continue checking for same ent:
//		if ((index == MAX_BEAMS) && (!b->model || (b->endtime < cl.time)))
		if ((index == MAX_BEAMS) && (!b->model || BEAM_INACTIVE(b)))
		{
			index = i;
		}
	}

	if (index < MAX_BEAMS)
	{
		b = cl_beams + index;
		b->entity = ent;
		b->model = Mod_ForName (modelname, true);
		b->starttime = cl.time - 0.2;			// JDH: for demo rewind (see note in AllocParticle)
		b->endtime = cl.time + 0.2;
		VectorCopy (start, b->start);
		VectorCopy (end, b->end);
#ifdef _DEBUG
//		if (cls.demoplayback && !cl_demorewind.value)
//			CL_PushBeam (b);
#endif
		return;	
	}

	Con_Print ("beam list overflow!\n");	
}

#ifdef HEXEN2_SUPPORT

//==========================================================================
//
// NewStream
//
//==========================================================================

static stream_t *CL_NewStream (int ent, int tag)
{
	int i;
	stream_t *stream;

	// Search for a stream with matching entity and tag
	for (i = 0, stream = cl_Streams; i < MAX_STREAMS; i++, stream++)
	{
		if (stream->entity == ent && stream->tag == tag)
		{
			return stream;
		}
	}
	// Search for a free stream
	for (i = 0, stream = cl_Streams; i < MAX_STREAMS; i++, stream++)
	{
		if (!stream->models[0] || STREAM_INACTIVE(stream))
		{
			return stream;
		}
	}
	return NULL;
}

//==========================================================================
//
// NewStreamEntity
//
//==========================================================================

static entity_t *NewStreamEntity (void)
{
	entity_t	*ent;

	if(cl_numvisedicts == MAX_VISEDICTS)
	{
		return NULL;
	}
	if(StreamEntityCount == MAX_STREAM_ENTITIES)
	{
		return NULL;
	}
	ent = &StreamEntities[StreamEntityCount++];
	memset(ent, 0, sizeof(*ent));
	cl_visedicts[cl_numvisedicts++] = ent;
	ent->colormap = vid.colormap;
	return ent;
}

//==========================================================================
//
// ParseStream
//
//==========================================================================

static void CL_ParseStream (int type, qboolean parse_only)
{
	int ent;
	int tag;
	int flags;
	int skin;
	vec3_t source;
	vec3_t dest;
	stream_t *stream;
	float duration;
	model_t *models[4];

	ent = MSG_ReadShort();
	flags = MSG_ReadByte();
	tag = flags&15;
	duration = (float)MSG_ReadByte()*HX_FRAME_TIME;
	skin = 0;
	if(type == TE_STREAM_COLORBEAM)
	{
		skin = MSG_ReadByte();
	}
	source[0] = MSG_ReadCoord();
	source[1] = MSG_ReadCoord();
	source[2] = MSG_ReadCoord();
	dest[0] = MSG_ReadCoord();
	dest[1] = MSG_ReadCoord();
	dest[2] = MSG_ReadCoord();

	if (parse_only)
		return;
	
	models[1] = models[2] = models[3] = NULL;
	switch(type)
	{
	case TE_STREAM_CHAIN:
		models[0] = Mod_ForName("models/stchain.mdl", true);
		break;
	case TE_STREAM_SUNSTAFF1:
		models[0] = Mod_ForName("models/stsunsf1.mdl", true);
		models[1] = Mod_ForName("models/stsunsf2.mdl", true);
		models[2] = Mod_ForName("models/stsunsf3.mdl", true);
		models[3] = Mod_ForName("models/stsunsf4.mdl", true);
		break;
	case TE_STREAM_SUNSTAFF2:
		models[0] = Mod_ForName("models/stsunsf5.mdl", true);
		models[2] = Mod_ForName("models/stsunsf3.mdl", true);
		models[3] = Mod_ForName("models/stsunsf4.mdl", true);
		break;
	case TE_STREAM_LIGHTNING:
		models[0] = Mod_ForName("models/stlghtng.mdl", true);
//		duration*=2;
		break;
	case TE_STREAM_LIGHTNING_SMALL:
		models[0] = Mod_ForName("models/stltng2.mdl", true);
//		duration*=2;
		break;
	case TE_STREAM_FAMINE:
		models[0] = Mod_ForName("models/fambeam.mdl", true);
		break;
	case TE_STREAM_COLORBEAM:
		models[0] = Mod_ForName("models/stclrbm.mdl", true);
		break;
	case TE_STREAM_ICECHUNKS:
		models[0] = Mod_ForName("models/stice.mdl", true);
		break;
	case TE_STREAM_GAZE:
		models[0] = Mod_ForName("models/stmedgaz.mdl", true);
		break;
	default:
		Sys_Error("ParseStream: bad type");
	}

	if((stream = CL_NewStream(ent, tag)) == NULL)
	{
		Con_Print("stream list overflow\n");
		return;
	}
	stream->type = type;
	stream->tag = tag;
	stream->flags = flags;
	stream->entity = ent;
	stream->skin = skin;
	stream->models[0] = models[0];
	stream->models[1] = models[1];
	stream->models[2] = models[2];
	stream->models[3] = models[3];
	stream->endTime = cl.time + duration;
	stream->startTime = cl.time - duration;		// JDH
	stream->lastTrailTime = 0;
	VectorCopy(source, stream->source);
	VectorCopy(dest, stream->dest);
	if(flags & STREAM_ATTACHED)
	{
		VectorSubtract(source, cl_entities[ent].origin, stream->offset);
	}
}

#endif	// #ifdef HEXEN2_SUPPORT


#define	SetCommonExploStuff						\
	dl = CL_AllocDlight (0);					\
	VectorCopy (pos, dl->origin);				\
	dl->radius = 150 + 200 * bound(0, r_explosionlight.value, 1);	\
	dl->starttime = cl.time - 0.5;					\
	dl->endtime = cl.time + 0.5;				\
	dl->decay = 300

/*
=================
CL_ParseTEnt
=================
*/
qboolean CL_ParseTEnt (qboolean parse_only)
{
	int			type, rnd, colorStart, colorLength, cnt;
	vec3_t		pos, dir;
	dlight_t	*dl;
	byte		*colorByte;
	float		ExploColor[3];

	type = MSG_ReadByte ();
	switch (type)
	{
	
	case TE_WIZSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
		{
			R_RunParticleEffect (pos, vec3_origin, 20, 30);
		#ifdef HEXEN2_SUPPORT
			if (!hexen2)
		#endif
				S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		}
		break;

	case TE_KNIGHTSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
		{
			R_RunParticleEffect (pos, vec3_origin, 226, 20);
		#ifdef HEXEN2_SUPPORT
			if (!hexen2)
		#endif
				S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		}
		break;

	case TE_SPIKE:				// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
	// joe: they put the ventillator's wind effect to "10" in Nehahra. sigh.
		if (!parse_only)
		{
			if (nehahra)
				R_RunParticleEffect (pos, vec3_origin, 0, 9);
			else
				R_RunParticleEffect (pos, vec3_origin, 0, 10);

			if (rand() % 5)
			{
				S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
			}
			else
			{
				rnd = rand() & 3;
				if (rnd == 1)
					S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
				else if (rnd == 2)
					S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
				else
					S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
			}
		}
		break;

	case TE_SUPERSPIKE:			// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
		{
			R_RunParticleEffect (pos, vec3_origin, 0, 20);

			if (rand() % 5)
			{
				S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
			}
			else
			{
				rnd = rand() & 3;
				if (rnd == 1)
					S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
				else if (rnd == 2)
					S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
				else
					S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
			}
		}
		break;

		// DP105 version
		/* 
	case TE_BLOOD:	// blood puff
		MSG_ReadVector(pos);
		dir[0] = MSG_ReadChar ();
		dir[1] = MSG_ReadChar ();
		dir[2] = MSG_ReadChar ();
		count = MSG_ReadByte (); // amount of particles
		if (!parse_only)
			R_BloodPuff(pos, dir, count);
		break;
		*/

	case TE_BLOOD:	// blood puff hitting flesh
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		dir[0] = MSG_ReadChar ();
		dir[1] = MSG_ReadChar ();
		dir[2] = MSG_ReadChar ();
		cnt = MSG_ReadByte (); // amount of particles
		if (!parse_only)
		{
			if (!qmb_initialized || !gl_part_blood.value)		// JDH: fix for Marcher/Bastion
			{
				VectorScale (dir, 0.1f, dir);		// 0.1 and 2 (below) come from standard SpawnBlood
			}
			R_RunParticleEffect (pos, dir, 73, cnt*2);
		}
		break;

/*	case TE_CUSTOMFLASH:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		radius = (MSG_ReadByte() + 1) * 8;
		velspeed = (MSG_ReadByte() + 1) / 256.0;
		color[0] = MSG_ReadByte() * (2.0f / 255.0f);
		color[1] = MSG_ReadByte() * (2.0f / 255.0f);
		color[2] = MSG_ReadByte() * (2.0f / 255.0f);
	//	Matrix4x4_CreateTranslate(&tempmatrix, pos[0], pos[1], pos[2]);
	//	CL_AllocDlight(NULL, &tempmatrix, radius, color[0], color[1], color[2], radius / velspeed, velspeed, 0, -1, true, 1, 0.25, 1, 1, 1, LIGHTFLAG_NORMALMODE | LIGHTFLAG_REALTIMEMODE);
		if (!parse_only)
			CL_NewDlight (0, pos, radius, );
		break;
*/
	case TE_GUNSHOT:			// bullet hitting wall
		cnt = (cl.protocol == PROTOCOL_VERSION_QW) ? MSG_ReadByte () : 1;
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
		{
			/*if (nehahra)
				R_SparkShower (pos, vec3_origin, 15, 0);
			else*/
				R_RunParticleEffect (pos, vec3_origin, 0, 21*cnt);
		}
		break;

	case TE_EXPLOSION:			// rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
		{
			if (r_explosiontype.value == 3)
				R_RunParticleEffect (pos, vec3_origin, 225, 50);
			else
				R_ParticleExplosion (pos);

			if (r_explosionlight.value)
			{
				SetCommonExploStuff;
//				dl->type = SetDlightColor (r_explosionlightcolor.value, lt_explosion, true);
				SetDlightColor (dl, r_explosionlightcolor.value, lt_explosion, true);
			}

			S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		}
		break;

	case TE_TAREXPLOSION:			// tarbaby explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
		{
			R_BlobExplosion (pos);

			S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		}
		break;

	case TE_LIGHTNING1:				// lightning bolts
		CL_ParseBeam ("progs/bolt.mdl", parse_only);
		break;

	case TE_LIGHTNING2:				// lightning bolts
		CL_ParseBeam ("progs/bolt2.mdl", parse_only);
		break;

	case TE_LIGHTNING3:				// lightning bolts
		CL_ParseBeam ("progs/bolt3.mdl", parse_only);
		break;

	// nehahra support
	case TE_LIGHTNING4NEH:                             // lightning bolts
		CL_ParseBeam (MSG_ReadString(), parse_only);
		break;

// PGM 01/21/97 
	case TE_BEAM:				// grappling hook beam
		if (cl.protocol == PROTOCOL_VERSION_QW)
		{
			// temp ent #13 in QW is TE_LIGHTNINGBLOOD
			pos[0] = MSG_ReadCoord ();
			pos[1] = MSG_ReadCoord ();
			pos[2] = MSG_ReadCoord ();
			if (!parse_only)
				R_RunParticleEffect (pos, vec3_origin, 225, 50);
		}
		else
			CL_ParseBeam ("progs/beam.mdl", parse_only);
		break;
// PGM 01/21/97

	case TE_LAVASPLASH:	
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
			R_LavaSplash (pos);
		break;

	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		if (!parse_only)
			R_TeleportSplash (pos);
		break;

	case TE_EXPLOSION2:			// color mapped explosion
		if (cl.protocol == PROTOCOL_VERSION_QW)
		{
			// JDH: temp ent #12 in QW is TE_BLOOD
			cnt = MSG_ReadByte ();
			pos[0] = MSG_ReadCoord ();
			pos[1] = MSG_ReadCoord ();
			pos[2] = MSG_ReadCoord ();
			if (!parse_only)
				R_RunParticleEffect (pos, vec3_origin, 73, 20*cnt);
			break;
		}
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		colorStart = MSG_ReadByte ();
		colorLength = MSG_ReadByte ();

		if (!parse_only)
		{
			if (r_explosiontype.value == 3)
				R_RunParticleEffect (pos, vec3_origin, 225, 50);
			else
				R_ColorMappedExplosion (pos, colorStart, colorLength);

			if (r_explosionlight.value)
			{
				SetCommonExploStuff;
	//#ifdef GLQUAKE
				colorByte = (byte *)&d_8to24table[colorStart];
				/*ExploColor[0] = ((float)colorByte[0]) / (2.0 * 255.0);
				ExploColor[1] = ((float)colorByte[1]) / (2.0 * 255.0);
				ExploColor[2] = ((float)colorByte[2]) / (2.0 * 255.0);
				dl->type = lt_explosion2;*/
				dl->color[0] = ((float)colorByte[0]) / (2.0 * 255.0);
				dl->color[1] = ((float)colorByte[1]) / (2.0 * 255.0);
				dl->color[2] = ((float)colorByte[2]) / (2.0 * 255.0);	
	//#endif
			}

			S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		}
		break;

	// nehahra support
	case TE_SMOKE:                      // rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		MSG_ReadByte();		//R_Smoke(pos, MSG_ReadByte());
	case TE_EXPLOSION3:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		ExploColor[0] = MSG_ReadCoord () / 2.0;
		ExploColor[1] = MSG_ReadCoord () / 2.0;
		ExploColor[2] = MSG_ReadCoord () / 2.0;

		if (!parse_only)
		{
			if (r_explosiontype.value == 3)
				R_RunParticleEffect (pos, vec3_origin, 225, 50);
			else
				R_ParticleExplosion (pos);

			if (r_explosionlight.value)
			{
				SetCommonExploStuff;
//				dl->type = lt_explosion3;
				VectorCopy (ExploColor, dl->color);
			}

			S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		}
		break;

#ifdef HEXEN2_SUPPORT
	case TE_STREAM_CHAIN:
	case TE_STREAM_SUNSTAFF1:
	case TE_STREAM_SUNSTAFF2:
	case TE_STREAM_LIGHTNING:
	case TE_STREAM_LIGHTNING_SMALL:
	case TE_STREAM_COLORBEAM:
	case TE_STREAM_ICECHUNKS:
	case TE_STREAM_GAZE:
	case TE_STREAM_FAMINE:
		CL_ParseStream (type, parse_only);
		break;

#endif
	
	default:
		if (!parse_only)
			Sys_Error ("CL_ParseTEnt: bad type");
		return false;
	}

	return true;
}

/*
=================
CL_NewTempEntity
=================
*/
entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

	if (cl_numvisedicts == MAX_VISEDICTS)
	{
		Con_DPrintf("max vis edicts reached!\n");
		return NULL;
	}

	if (num_temp_entities == MAX_TEMP_ENTITIES)
	{
		Con_DPrintf("max temp entities reached!\n");
		return NULL;
	}

	ent = &cl_temp_entities[num_temp_entities];
	memset (ent, 0, sizeof(*ent));
	num_temp_entities++;
	cl_visedicts[cl_numvisedicts] = ent;
	cl_numvisedicts++;

	ent->colormap = vid.colormap;
	ent->fullbright = 1;		// JDH
	return ent;
}

/*
=================
CL_UpdateLightning
=================
*/
void CL_UpdateLightning (void)
{
	int			i, j;
	beam_t		*b;
	vec3_t		dist, org, beamstart, beamend;
	float		d, yaw, pitch, forward;
	entity_t	*ent;
//	qboolean	sparks = false;

	num_temp_entities = 0;

	// update lightning
	for (i = 0, b = cl_beams ; i < MAX_BEAMS ; i++, b++)
	{
		if (!b->model || BEAM_INACTIVE(b))
			continue;

		// if coming from the player, update the start position
		if (b->entity == cl.viewentity)
		{
			VectorCopy (cl_entities[cl.viewentity].origin, b->start);
			// joe: using koval's [sons]Quake code
			b->start[2] += cl.crouch;
			if (!cls.demorecording && cl_truelightning.value)
			{
				vec3_t	forward, v, org, ang;
				float	f, delta;
				trace_t	trace;

				f = bound(0, cl_truelightning.value, 1);

				VectorSubtract (playerbeam_end, cl_entities[cl.viewentity].origin, v);
		//		v[2] -= 22;		// adjust for view height
				v[2] -= cl.viewheight;		// adjust for view height
				vectoangles (v, ang);

				// lerp pitch
				ang[0] = -ang[0];
				if (ang[0] < -180)
					ang[0] += 360;
				ang[0] += (cl.viewangles[0] - ang[0]) * f;

				// lerp yaw
				delta = cl.viewangles[1] - ang[1];
				if (delta > 180)
					delta -= 360;
				if (delta < -180)
					delta += 360;
				ang[1] += delta * f;
				ang[2] = 0;

				AngleVectors (ang, forward, NULL, NULL);
				VectorScale (forward, 600, forward);
				VectorCopy (cl_entities[cl.viewentity].origin, org);
				org[2] += 16;
				VectorAdd(org, forward, b->end);

				memset (&trace, 0, sizeof(trace_t));
				if (!SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, org, b->end, &trace))
					VectorCopy (trace.endpos, b->end);
			}
		}

		// calculate pitch and yaw
		VectorSubtract (b->end, b->start, dist);

		if (!dist[1] && !dist[0])
		{
			yaw = 0;
			pitch = (dist[2] > 0) ? 90 : 270;
		}
		else
		{
			yaw = atan2 (dist[1], dist[0]) * 180 / M_PI;
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = atan2 (dist[2], forward) * 180 / M_PI;
			if (pitch < 0)
				pitch += 360;
		}

		// add new entities for the lightning
		VectorCopy (b->start, org);
		VectorCopy (b->start, beamstart);
		d = VectorNormalize (dist);
		VectorScale (dist, 30, dist);

		for ( ; d > 0 ; d -= 30)
		{
			if (qmb_initialized && gl_part_lightning.value)
			{
				VectorAdd(org, dist, beamend);
				for (j=0 ; j<3 ; j++)
					beamend[j] += (b->entity != cl.viewentity) ? (rand() % 40) - 20 : (rand() % 16) - 8;
				QMB_LightningBeam (beamstart, beamend);

				// JT040905 - glowing lightning.
				CL_NewDlight (i, beamend, 200 + (rand() & 31), 0.1, lt_blue);
				CL_NewDlight (i, beamstart, 200 + (rand() & 31), 0.1, lt_blue);
				// end

				VectorCopy (beamend, beamstart);
			}
			else
			{
				if (!(ent = CL_NewTempEntity()))
					return;
				VectorCopy (org, ent->origin);
				ent->model = b->model;
				ent->angles[0] = pitch;
				ent->angles[1] = yaw;
				ent->angles[2] = rand() % 360;
			}

#if 0
			if (qmb_initialized && gl_part_lightning.value && !sparks)
			{
				trace_t	trace;

				memset (&trace, 0, sizeof(trace_t));
				if (!SV_RecursiveHullCheck(cl.worldmodel->hulls, 0, 0, 1, org, beamend, &trace))
				{
					byte	col[3] = {240, 150, 0};	// change color here 60, 100, 240

					QMB_GenSparks (trace.endpos, col, 3, 300, 0.25);	// 

					// JT 041105 - add second spark color
					/*
					byte	col[3] = {255, 204, 0};	// change color here

					QMB_GenSparks (trace.endpos, col, 2, 300, 0.25);
					*/
					// end

					QMB_Lightning_Splash (trace.endpos);
					sparks = true;
				}
			}
#endif
			VectorAdd(org, dist, org);
		}
	}
}

#ifdef HEXEN2_SUPPORT
/*
=================
CL_UpdateStreams
=================
*/
void CL_UpdateStreams (void)
{
	int i;
	stream_t *stream;
	vec3_t dist;
	vec3_t org;
	float d;
	entity_t *ent;
	float yaw, pitch;
	float forward;
	int segmentCount;
	int offset;

	// Update streams
	StreamEntityCount = 0;
	for (i = 0, stream = cl_Streams; i < MAX_STREAMS; i++, stream++)
	{
		if (!stream->models[0])// || stream->endTime < cl.time)
		{ // Inactive
			continue;
		}
		if (STREAM_INACTIVE(stream))
		{ // Inactive
			if ((stream->type != TE_STREAM_LIGHTNING) && (stream->type != TE_STREAM_LIGHTNING_SMALL))
				continue;
			else if ((stream->endTime + 0.25 < cl.time) || (stream->startTime - 0.25 > cl.time))
				continue;		// extra 1/4 second for fade
		}

		if ((stream->flags & STREAM_ATTACHED) && !STREAM_INACTIVE(stream))
		{ // Attach the start position to owner
			VectorAdd(cl_entities[stream->entity].origin, stream->offset, stream->source);
		}

		VectorSubtract(stream->dest, stream->source, dist);
		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
			{
				pitch = 90;
			}
			else
			{
				pitch = 270;
			}
		}
		else
		{
			yaw = (int)(atan2(dist[1], dist[0])*180/M_PI);
			if (yaw < 0)
			{
				yaw += 360;
			}

			forward = sqrt(dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int)(atan2(dist[2], forward)*180/M_PI);
			if (pitch < 0)
			{
				pitch += 360;
			}
		}

		VectorCopy(stream->source, org);
		d = VectorNormalize(dist);
		segmentCount = 0;
		if (stream->type == TE_STREAM_ICECHUNKS)
		{
			offset = (int)(cl.time*40)%30;
			for (i = 0; i < 3; i++)
			{
				org[i] += dist[i]*offset;
			}
		}

		while (d > 0)
		{
			ent = NewStreamEntity();
			if (!ent)
			{
				return;
			}

			VectorCopy(org, ent->origin);
			ent->model = stream->models[0];
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;

			switch (stream->type)
			{
				case TE_STREAM_CHAIN:
					ent->angles[2] = 0;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					break;

				case TE_STREAM_SUNSTAFF1:
					ent->angles[2] = (int)(cl.time*10)%360;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					//ent->frame = (int)(cl.time*20)%20;

					ent = NewStreamEntity();
					if (!ent)
						return;
					VectorCopy(org, ent->origin);
					ent->model = stream->models[1];
					ent->angles[0] = pitch;
					ent->angles[1] = yaw;
					ent->angles[2] = (int)(cl.time*50)%360;
					ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
					ent->abslight = 128;
					break;
				case TE_STREAM_SUNSTAFF2:
					ent->angles[2] = (int)(cl.time*10)%360;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					ent->frame = (int)(cl.time*10)%8;
					break;

				case TE_STREAM_LIGHTNING:
				case TE_STREAM_LIGHTNING_SMALL:
					if (stream->endTime < cl.time)
					{//fixme: keep last non-translucent frame and angle
						ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
						ent->abslight = 128 + (stream->endTime - cl.time)*192;
					}
					else if (cl.time < stream->startTime)
					{
						ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
						ent->abslight = 128 + (cl.time - stream->startTime)*192;						
					}
					else
					{
						ent->angles[2] = rand()%360;
						ent->drawflags = MLS_ABSLIGHT;
						ent->abslight = 128;
						ent->frame = rand()%6;
					}
					break;

			/*	case TE_STREAM_LIGHTNING_SMALL:
					if(stream->endTime < cl.time)
					{
						ent->drawflags = MLS_ABSLIGHT|DRF_TRANSLUCENT;
						ent->abslight = 128 + (stream->endTime - cl.time)*192;
					}
					else
					{
						ent->angles[2] = rand()%360;
						ent->frame = rand()%6;
						ent->drawflags = MLS_ABSLIGHT;
						ent->abslight = 128;
					}
					break;*/

				case TE_STREAM_FAMINE:
					ent->angles[2] = rand()%360;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					ent->frame = 0;
					break;

				case TE_STREAM_COLORBEAM:
					ent->angles[2] = 0;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					ent->skinnum = stream->skin;
					break;

				case TE_STREAM_GAZE:
					ent->angles[2] = 0;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					ent->frame = (int)(cl.time*40)%36;
					break;

				case TE_STREAM_ICECHUNKS:
					ent->angles[2] = rand()%360;
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 128;
					ent->frame = rand()%5;
					break;

				default:
					ent->angles[2] = 0;
			}

			for (i = 0; i < 3; i++)
			{
				org[i] += dist[i]*30;
			}

			d -= 30;
			segmentCount++;
		}

		if ((stream->type == TE_STREAM_SUNSTAFF1) || (stream->type == TE_STREAM_SUNSTAFF2))
		{
			if (stream->lastTrailTime+0.2 < cl.time)
			{
				stream->lastTrailTime = cl.time;
				R_SunStaffTrail(stream->source, stream->dest);
			}

			ent = NewStreamEntity();
			if (ent == NULL)
			{
				return;
			}

			VectorCopy(stream->dest, ent->origin);
			ent->model = stream->models[2];
			ent->drawflags = MLS_ABSLIGHT;
			ent->abslight = 128;
			ent->scale = 80 + (rand()&15);
			//ent->frame = (int)(cl.time*20)%20;

			ent = NewStreamEntity();
			if (ent == NULL)
			{
				return;
			}
			VectorCopy(stream->dest, ent->origin);
			ent->model = stream->models[3];
			ent->drawflags = MLS_ABSLIGHT | DRF_TRANSLUCENT;
			ent->abslight = 128;
			ent->scale = 150 + (rand()&15);
		}
	}
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
=================
CL_UpdateTEnts
=================
*/
void CL_UpdateTEnts (void)
{
	CL_UpdateLightning ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		CL_UpdateStreams ();
#endif
}

#endif		//#ifndef RQM_SV_ONLY
