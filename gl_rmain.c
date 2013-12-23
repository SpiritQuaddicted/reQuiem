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
// gl_rmain.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

entity_t	r_worldentity;

qboolean	r_cache_thrash;		// compatibility

vec3_t		modelorg, r_entorigin;

qboolean OnChange_lightmaps (cvar_t *var, const char *value);		// JDH

int		r_visframecount;	// bumped when going to a new PVS
int		r_framecount;		// used for dlight push checking

mplane_t	frustum[4];

int		c_brush_polys, c_alias_polys;

int		particletexture;	// little dot for particles
int		playertextures;		// base number for up to MAX_SCOREBOARD color translated skins
int		skyboxtextures;		// by joe
int		underwatertexture = 0;
//int		detailtexture = 0;
int		detailtexture[NUM_DETAIL_LEVELS];

// view origin
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

#ifdef SHINYWATER
  int	chrometexture2 = 0;  // JT added chrometexture2
  float	r_world_matrix[16];
#endif

// screen size info
refdef_t	r_refdef;

mleaf_t		*r_viewleaf, *r_oldviewleaf;
mleaf_t		*r_viewleaf2, *r_oldviewleaf2;	// for watervis hack

texture_t	*r_notexture_mip;

int		d_lightstylevalue[256];	// 8.8 fraction of base light value

cvar_t	r_drawentities    = {"r_drawentities",    "1"};
cvar_t	r_drawviewmodel   = {"r_drawviewmodel",   "1"};
cvar_t	r_viewmodelsize   = {"r_viewmodelsize",   "1", CVAR_FLAG_ARCHIVE};
cvar_t	r_speeds          = {"r_speeds",          "0"};
cvar_t	r_fullbright      = {"r_fullbright",      "0", 0, OnChange_lightmaps};
cvar_t	r_lightmap        = {"r_lightmap",        "0"};
cvar_t	r_shadows         = {"r_shadows",         "0", CVAR_FLAG_ARCHIVE};
cvar_t	r_wateralpha      = {"r_wateralpha",      "1", CVAR_FLAG_ARCHIVE};
cvar_t	r_dynamic         = {"r_dynamic",         "1", CVAR_FLAG_ARCHIVE};
cvar_t	r_flatlightstyles = {"r_flatlightstyles", "0"};			// Fitz
cvar_t	r_fullbrightskins = {"r_fullbrightskins", "0", CVAR_FLAG_ARCHIVE};
cvar_t	r_skytype         = {"r_skytype",         "1", CVAR_FLAG_ARCHIVE};
cvar_t	r_skycolor        = {"r_skycolor",     "auto", CVAR_FLAG_ARCHIVE | CVAR_FLAG_NOCASE};
cvar_t	r_modelbrightness = {"r_modelbrightness", "1", CVAR_FLAG_ARCHIVE};

cvar_t	r_farclip         = {"r_farclip",     "16384", CVAR_FLAG_ARCHIVE};
cvar_t	gl_skyclip        = {"gl_skyclip",     "4608", CVAR_FLAG_ARCHIVE};	// from aguirRe's nehquake (original neh used 4096)
//cvar_t	gl_skyfog         = {"gl_skyfog",       "0.5", CVAR_FLAG_ARCHIVE};

qboolean OnChange_r_skybox (cvar_t *var, const char *string);
cvar_t	r_skybox = {"r_skybox", "", 0, OnChange_r_skybox};

// fenix@io.com: model interpolation
cvar_t  gl_interpolate_animation = {"gl_interpolate_animation", "1", CVAR_FLAG_ARCHIVE};
cvar_t  gl_interpolate_transform = {"gl_interpolate_transform", "1", CVAR_FLAG_ARCHIVE};

cvar_t	gl_clear            = {"gl_clear",                      "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_cull             = {"gl_cull",                       "1", CVAR_FLAG_ARCHIVE};
//cvar_t	gl_ztrick           = {"gl_ztrick",                     "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_smoothmodels     = {"gl_smoothmodels",               "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_affinemodels     = {"gl_affinemodels",               "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_polyblend        = {"gl_polyblend",                  "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_flashblend       = {"gl_flashblend",                 "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_playermip        = {"gl_playermip",                  "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_nocolors         = {"gl_nocolors",                   "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_finish           = {"gl_finish",                     "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_loadlitfiles     = {"gl_loadlitfiles",               "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_doubleeyes       = {"gl_doubleeyes",                 "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_interdist        = {"gl_interpolate_distance",   "17000", CVAR_FLAG_ARCHIVE};
cvar_t  gl_waterfog         = {"gl_waterfog",                   "0", CVAR_FLAG_ARCHIVE};
cvar_t  gl_waterfog_density = {"gl_waterfog_density",         "0.5", CVAR_FLAG_ARCHIVE};
cvar_t	gl_detail           = {"gl_detail",                     "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_caustics         = {"gl_caustics",                   "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_ringalpha        = {"gl_ringalpha",                "0.4", CVAR_FLAG_ARCHIVE};
cvar_t	gl_fb_world         = {"gl_fb_world",                   "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_fb_bmodels       = {"gl_fb_bmodels",                 "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_fb_models        = {"gl_fb_models",                  "1", CVAR_FLAG_ARCHIVE};
cvar_t  gl_solidparticles   = {"gl_solidparticles",             "0", CVAR_FLAG_ARCHIVE};
cvar_t  gl_vertexlights     = {"gl_vertexlights",               "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_subdivide_size   = {"gl_subdivide_size",           "128", CVAR_FLAG_ARCHIVE};

cvar_t	gl_part_explosions  = {"gl_part_explosions",  "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_trails      = {"gl_part_trails",      "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_spikes      = {"gl_part_spikes",      "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_gunshots    = {"gl_part_gunshots",    "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_blood       = {"gl_part_blood",       "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_telesplash  = {"gl_part_telesplash",  "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_blobs       = {"gl_part_blobs",       "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_lavasplash  = {"gl_part_lavasplash",  "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_inferno     = {"gl_part_inferno",     "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_flames      = {"gl_part_flames",      "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_lightning   = {"gl_part_lightning",   "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_part_spiketrails = {"gl_part_spiketrails", "0", CVAR_FLAG_ARCHIVE};

#ifdef SHINYWATER
  cvar_t  gl_shinywater     = {"gl_shinywater",       "0", CVAR_FLAG_ARCHIVE};			// JT030105 - add reflections
#endif

cvar_t	gl_skyhack = {"gl_skyhack", "1", CVAR_FLAG_ARCHIVE};
	// less than 0 --> use old, slow code		**TEMP**
	// 0 --> use new, faster code
	// 1 --> use very fast, but buggy code on skyboxes only
	// 2 --> use very fast, but buggy code on skyboxes & MH
/***********************JDH*********************/

#ifdef FITZWORLD
qboolean vis_changed = false;

qboolean OnChange_r_novis (cvar_t *var, const char *value)
{
	vis_changed = true;
	return false;		// allow change
}

qboolean OnChange_r_fastworld (cvar_t *var, const char *string)
{
	r_oldviewleaf = r_oldviewleaf2 = NULL;		// force rebuilding of texture chains
	return false;		// allow change
}

cvar_t	r_fastworld  = {"r_fastworld",  "0", 0, OnChange_r_fastworld};		// instead of RecursiveWorldNode
cvar_t	r_novis      = {"r_novis",      "0", 0, OnChange_r_novis};
#else
cvar_t	r_novis      = {"r_novis",      "0"};
#endif

// JDH
cvar_t	r_showbboxes = {"r_showbboxes", "0"};			// from Fitz
cvar_t	r_oldsky     = {"r_oldsky",     "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_glows     = {"gl_glows",     "1", CVAR_FLAG_ARCHIVE};				// from nehahra
cvar_t	gl_cache_ms2 = {"gl_cache_ms2", "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_lightmode = {"gl_lightmode", "1", CVAR_FLAG_ARCHIVE, OnChange_lightmaps};
cvar_t	gl_zfightfix = {"gl_zfightfix", "0", CVAR_FLAG_ARCHIVE};

qboolean r_drawskylast;
// JDH

//johnfitz -- polygon offset
#define OFFSET_BMODEL 1
#define OFFSET_NONE 0
#define OFFSET_DECAL -1
#define OFFSET_FOG -2
#define OFFSET_SHOWTRIS -3

/***********************JDH*********************/

void R_InitBubble (void);

/*
=================
R_CullBox

Returns true if the box is completely outside the frustum
=================
*/
qboolean R_CullBox (const vec3_t mins, const vec3_t maxs)
{
	int		i;

//	JDH: code from DarkPlaces
/*	mplane_t *p;
	for (i = 0;i < 4;i++)
	{
		p = frustum + i;
		switch(p->signbits)
		{
		default:
		case 0:
			if (p->normal[0]*maxs[0] + p->normal[1]*maxs[1] + p->normal[2]*maxs[2] < p->dist)
				return true;
			break;
		case 1:
			if (p->normal[0]*mins[0] + p->normal[1]*maxs[1] + p->normal[2]*maxs[2] < p->dist)
				return true;
			break;
		case 2:
			if (p->normal[0]*maxs[0] + p->normal[1]*mins[1] + p->normal[2]*maxs[2] < p->dist)
				return true;
			break;
		case 3:
			if (p->normal[0]*mins[0] + p->normal[1]*mins[1] + p->normal[2]*maxs[2] < p->dist)
				return true;
			break;
		case 4:
			if (p->normal[0]*maxs[0] + p->normal[1]*maxs[1] + p->normal[2]*mins[2] < p->dist)
				return true;
			break;
		case 5:
			if (p->normal[0]*mins[0] + p->normal[1]*maxs[1] + p->normal[2]*mins[2] < p->dist)
				return true;
			break;
		case 6:
			if (p->normal[0]*maxs[0] + p->normal[1]*mins[1] + p->normal[2]*mins[2] < p->dist)
				return true;
			break;
		case 7:
			if (p->normal[0]*mins[0] + p->normal[1]*mins[1] + p->normal[2]*mins[2] < p->dist)
				return true;
			break;
		}
	}
	return false;
*/
	for (i=0 ; i<4 ; i++)
		if (BoxOnPlaneSide(mins, maxs, &frustum[i]) == 2)
			return true;

	return false;
}

#ifdef _WIN32
// JDH: this doesn't seem to work reliably on Linux when fov != 90
/*
=================
R_CullSphere

Returns true if the sphere is completely outside the frustum
=================
*/
qboolean R_CullSphere (const vec3_t centre, float radius)
{
	int			i;
	mplane_t	*p;

	for (i=0, p=frustum ; i<4 ; i++, p++)
	{
		if (PlaneDiff(centre, p) <= -radius)
			return true;
	}

	return false;
}
#endif

/*
=============
R_GLRotate
=============
*/
void R_RotateEntity (const entity_t *e, float yaw, float pitch, float rot)
{
	// scale back pitch for player (otherwise when chase_active, model can be
	//  leaning forward all the way to the floor)

	/******JDH-from Fitz*******/
	if (e == &cl_entities[cl.viewentity])
	{
		pitch *= 0.3;
	}
	/******JDH-from Fitz*******/

	glRotatef ( yaw,   0, 0, 1);
	glRotatef (-pitch, 0, 1, 0);
	glRotatef ( rot,   1, 0, 0);
}

/*
=============
R_RotateForEntity
=============
*/
void R_RotateForEntity (const entity_t *e)
{
	glTranslatef (e->origin[0], e->origin[1], e->origin[2]);

	R_RotateEntity (e, e->angles[1], e->angles[0], e->angles[2]);
}

/*
=============
R_CalcBlendedRotateVector
=============
*/
void R_CalcBlendedRotateVector (entity_t *e, vec3_t v)
{
	float	blend, timepassed;
	int		i;

	timepassed = fabs(cl.time - e->rotate_start_time);

	if (e->rotate_start_time == 0 || timepassed > 1)
	{
		e->rotate_start_time = cl.time;
		VectorCopy (e->angles, e->angles1);
		VectorCopy (e->angles, e->angles2);
		blend = 1;
	}
	else if (!VectorCompare (e->angles, e->angles2))
	{
		e->rotate_start_time = cl.time;
		VectorCopy (e->angles2, e->angles1);
		VectorCopy (e->angles,  e->angles2);
		blend = 0;
	}
	else
	{
		blend = timepassed / 0.1;
		if (/*cl.paused ||*/ blend > 1)
			blend = 1;
	}

	VectorSubtract (e->angles2, e->angles1, v);

	// always interpolate along the shortest path
	for (i=0 ; i<3 ; i++)
	{
		if (v[i] > 180)
			v[i] -= 360;
		else if (v[i] < -180)
			v[i] += 360;
	}

	v[0] *= blend;
	v[1] *= blend;
	v[2] *= blend;
}

/*
=============
R_DoBlendedTranslate
=============
*/
void R_DoBlendedTranslate (entity_t *e)
{
	float	blend, timepassed;
	vec3_t	d;

	timepassed = fabs(cl.time - e->translate_start_time);

	if (e->translate_start_time == 0 || timepassed > 1)
	{
		e->translate_start_time = cl.time;
		VectorCopy (e->origin, e->origin1);
		VectorCopy (e->origin, e->origin2);
		blend = 1;
	}
	else if (!VectorCompare (e->origin, e->origin2))
	{
#ifdef _DEBUG
		if (cls.demoplayback && /*cl_demorewind.value &&*/ e->modelindex == cl_modelindex[mi_scrag])
			blend = 0;
#endif
		
		e->translate_start_time = cl.time;
		VectorCopy (e->origin2, e->origin1);
		VectorCopy (e->origin,  e->origin2);
		blend = 0;
	}
	else
	{
#if 1
		blend = timepassed / 0.1;
		if (/*cl.paused ||*/ blend > 1)
			blend = 1;
#else		
	// 2010/04/30 - demo scrags
		if (cl.mtime == cl.mtime_prev)
			blend = 1;
		//else if (cl.mtime - cl.mtime_prev > 0.1)
		//	blend = timepassed / 0.1;
		else 
			blend = timepassed / (cl.mtime - cl.mtime_prev);
		
		if (/*cl.paused ||*/ blend > 1)
			blend = 1;
#endif
	}

	VectorSubtract (e->origin2, e->origin1, d);
	glTranslatef (e->origin1[0] + (blend * d[0]), e->origin1[1] + (blend * d[1]), e->origin1[2] + (blend * d[2]));
}


#ifdef HEXEN2_SUPPORT
/*
=============
R_DoEntityRotate
=============
*/
void R_DoEntityRotate (entity_t *e, vec3_t angles)
{
	vec3_t v;
	float	yaw, pitch, forward;

	if (e->model->flags & EF_FACE_VIEW)
	{
		VectorSubtract(r_origin,e->origin,v);
		VectorNormalize(v);

		if (v[1] == 0 && v[0] == 0)
		{
			yaw = 0;
			if (v[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(v[1], v[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			forward = sqrt (v[0]*v[0] + v[1]*v[1]);
			pitch = (int) (atan2(v[2], forward) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		glRotatef(-pitch,     0, 1, 0);
		glRotatef( yaw,       0, 0, 1);
		glRotatef(-angles[2], 1, 0, 0);
	}
	else
	{
		if (e->model->flags & EF_ROTATE)
		{
			yaw = anglemod((e->origin[0] + e->origin[1])*0.8 + (108*cl.time));
		}
		else
		{
			yaw = angles[1];
		}

		R_RotateEntity( e, yaw, angles[0], -angles[2] );
	}
}

//==========================================================================
//
// R_RotateForEntity_H2
//
// Same as R_RotateForEntity(), but checks for EF_ROTATE and modifies
// yaw appropriately.
//
//==========================================================================

void R_RotateForEntity_H2 (entity_t *e)
{
	glTranslatef (e->origin[0], e->origin[1], e->origin[2]);

	R_DoEntityRotate (e, e->angles);
}


//==========================================================================
//
// R_BlendedRotateForEntity_H2  ****JDH****   MAY HAVE BUGS!!!!
//
// Same as R_BlendedRotateForEntity(), but checks for EF_ROTATE and modifies
// yaw appropriately.
//
//==========================================================================

void R_BlendedRotateForEntity_H2 (entity_t *e)
{
	vec3_t	d;

	// positional interpolation
	R_DoBlendedTranslate(e);

	// orientation interpolation
	R_CalcBlendedRotateVector(e, d);

	d[0] += e->angles1[0];
	d[1] += e->angles1[1];
	d[2] += e->angles1[2];

	R_DoEntityRotate(e, d);
}

#endif	// #ifdef HEXEN2_SUPPORT

/*
=============
R_BlendedRotateForEntity

fenix@io.com: model transform interpolation
=============
*/
void R_BlendedRotateForEntity (entity_t *e)
{
	vec3_t	d;

	// positional interpolation
	R_DoBlendedTranslate (e);

	// orientation interpolation (Euler angles, yuck!)
	R_CalcBlendedRotateVector (e, d);

	R_RotateEntity (e, e->angles1[1] + d[1], e->angles1[0] + d[0], e->angles1[2] + d[2]);
}


//==================================================================================


#ifdef HEXEN2_SUPPORT

typedef struct sortedent_s
{
	entity_t *ent;
	vec_t len;
} sortedent_t;

sortedent_t     cl_transvisedicts[MAX_VISEDICTS];
sortedent_t		cl_transwateredicts[MAX_VISEDICTS];

int				cl_numtransvisedicts;
int				cl_numtranswateredicts;
#endif

/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawEntitiesOnList (void)
{
	int	i;
	vec3_t	liteorg;
/*******JDH*******/
	entity_t *currententity;
#ifdef HEXEN2_SUPPORT
	qboolean item_trans;

	if (hexen2)
	{
		cl_numtransvisedicts = 0;
		cl_numtranswateredicts = 0;
	}
#endif
/*******JDH*******/

	if (!r_drawentities.value)
		return;

	// draw sprites seperately, because of alpha blending
	for (i=0 ; i<cl_numvisedicts ; i++)
	{
	/******JDH-from nehBJP*******/
		if ((i + 1) % 100 == 0)
			S_ExtraUpdateTime ();	// don't let sound get messed up if going slow
	/******JDH-from nehBJP*******/

		currententity = cl_visedicts[i];

		if (currententity->transparency != 1 && currententity->transparency != 0 && !gl_notrans.value)
		{
			currententity->transignore = false;
			continue;
		}

		switch (currententity->model->type)
		{
		case mod_alias:
			if (qmb_initialized)
			{
				if (!gl_part_flames.value && COM_FilenamesEqual(currententity->model->name, "progs/flame0.mdl"))
				{
					currententity->model = cl.model_precache[cl_modelindex[mi_flame1]];
				}
				else if (gl_part_flames.value)
				{
					VectorCopy (currententity->origin, liteorg);
					if (currententity->baseline.modelindex == cl_modelindex[mi_flame0])
					{
						liteorg[2] += 5.5;
						QMB_TorchFlame (liteorg, 8, 0.8);		// changed from 7 to 8
					}
					else if (currententity->baseline.modelindex == cl_modelindex[mi_flame1])
					{
						liteorg[2] += 5.5;
						QMB_TorchFlame (liteorg, 8, 0.8);		// changed from 7 to 8
						currententity->model = cl.model_precache[cl_modelindex[mi_flame0]];
					}
					else if (currententity->baseline.modelindex == cl_modelindex[mi_flame2])
					{
						liteorg[2] -= 1;
						QMB_TorchFlame (liteorg, 15, 1);		// changed from 12 to 15
						continue;
					}
				}
			}

		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				item_trans = ((currententity->drawflags & DRF_TRANSLUCENT) ||
					(currententity->model->flags & (EF_TRANSPARENT|EF_HOLEY|EF_SPECIAL_TRANS))) != 0;
				if (item_trans) break;
			}
		#endif
			R_DrawAliasModel (currententity);
			break;

		case mod_brush:
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				item_trans = ((currententity->drawflags & DRF_TRANSLUCENT) != 0);
				if (item_trans) break;
			}
		#endif
			R_DrawBrushModel (currententity);
			break;

		case mod_md3:
		#ifdef HEXEN2_SUPPORT
			item_trans = false;
		#endif
			R_DrawQ3Model (currententity);
			break;

		#ifdef HEXEN2_SUPPORT
		case mod_sprite:
			item_trans = true;
			break;
		#endif

		default:
		#ifdef HEXEN2_SUPPORT
			item_trans = false;
		#endif
			break;
		}

	#ifdef HEXEN2_SUPPORT
		if (hexen2 && item_trans)
		{
			mleaf_t *pLeaf = Mod_PointInLeaf (currententity->origin, cl.worldmodel);
			if (pLeaf->contents != CONTENTS_WATER)
				cl_transvisedicts[cl_numtransvisedicts++].ent = currententity;
			else
				cl_transwateredicts[cl_numtranswateredicts++].ent = currententity;
		}
	#endif
	}
}

/*
=============
R_DrawTransEntities
=============
*/
void R_DrawTransEntities (void)
{
	// need to draw back to front
	// fixme: this isn't my favorite option
	int		i;
	float		bestdist, dist;
	entity_t	*bestent;
	vec3_t		start, test;
/*******JDH*******/
	entity_t *currententity;
/*******JDH*******/

	VectorCopy (r_refdef.vieworg, start);

	if (!r_drawentities.value)
		return;

transgetent:
	bestdist = 0;
	bestent = NULL;
	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = cl_visedicts[i];

		if (currententity->transignore || currententity->transparency == 1 || currententity->transparency == 0)
			continue;

		VectorCopy (currententity->origin, test);
		if (currententity->model->type == mod_brush)
		{
			test[0] += currententity->model->mins[0];
			test[1] += currententity->model->mins[1];
			test[2] += currententity->model->mins[2];
		}
		dist = (((test[0] - start[0]) * (test[0] - start[0])) +
			((test[1] - start[1]) * (test[1] - start[1])) +
			((test[2] - start[2]) * (test[2] - start[2])));

		if (dist > bestdist)
		{
			bestdist = dist;
			bestent = currententity;
		}
	}
	if (bestdist == 0)
		return;
	bestent->transignore = true;

	currententity = bestent;
	switch (currententity->model->type)
	{
	case mod_alias:
		R_DrawAliasModel (currententity);
		break;

	case mod_brush:
		R_DrawBrushModel (currententity);
		break;

	default:
		break;
	}

	goto transgetent;
}


#ifdef HEXEN2_SUPPORT

int transCompare(const void *arg1, const void *arg2 )
{
	const sortedent_t *a1, *a2;
	a1 = (sortedent_t *) arg1;
	a2 = (sortedent_t *) arg2;
	return (a2->len - a1->len); // Sorted in reverse order.  Neat, huh?
}

void R_DrawTransEntitiesOnList (qboolean inwater)
{
	int i;
	int numents;
	sortedent_t *theents;
	entity_t *currententity;
	int depthMaskWrite = 0;
	vec3_t result;

	theents = (inwater) ? cl_transwateredicts : cl_transvisedicts;
	numents = (inwater) ? cl_numtranswateredicts : cl_numtransvisedicts;

	if ( numents == 0 ) return;

	for (i=0; i<numents; i++)
	{
		VectorSubtract( theents[i].ent->origin, r_origin, result);
		theents[i].len = DotProduct( result, result );
	}

	qsort((void *) theents, numents, sizeof(sortedent_t), transCompare);
	// Add in BETTER sorting here

	glDepthMask(0);
	for (i=0; i<numents; i++)
	{
		currententity = theents[i].ent;

		switch (currententity->model->type)
		{
		case mod_alias:
			if (!depthMaskWrite)
			{
				depthMaskWrite = 1;
				glDepthMask(1);
			}
			R_DrawAliasModel (currententity);
			break;

		case mod_brush:
			if (!depthMaskWrite)
			{
				depthMaskWrite = 1;
				glDepthMask(1);
			}
			R_DrawBrushModel (currententity);
			break;

		case mod_sprite:
			if (depthMaskWrite)
			{
				depthMaskWrite = 0;
				glDepthMask(0);
			}
			R_DrawSprite (currententity);
			break;

#ifndef _WIN32
        case mod_md3:
            break;          // shut up compiler warning
#endif
		}
	}
	if (!depthMaskWrite)
		glDepthMask(1);
}
#endif	// #ifdef HEXEN2_SUPPORT


/*
=============
R_DrawViewModel
=============
*/
void R_DrawViewModel (void)
{
	// fenix@io.com: model transform interpolation
	float		old_interpolate_transform;
/*******JDH*******/
	entity_t *currententity;
/*******JDH*******/

	currententity = &cl.viewent;

	if (!r_drawviewmodel.value || chase_active.value || !r_drawentities.value  || !currententity->model)
		return;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (cl.v.health <= 0)
			return;
	}
	else
#endif
	if (cl.stats[STAT_HEALTH] <= 0) return;

	// LordHavoc: if the player is transparent, so is his gun
	currententity->transparency = r_modelalpha = cl_entities[cl.viewentity].transparency;

	// hack the depth range to prevent view model from poking into walls
//	glDepthRange (gldepthmin, gldepthmin + 0.3 * (gldepthmax - gldepthmin));
	glDepthRange (0, 0.3);

	// fenix@io.com: model transform interpolation
	old_interpolate_transform = gl_interpolate_transform.value;
	gl_interpolate_transform.value = false;

/******JDH******/
	//R_DrawAliasModel (currententity);
/******JDH******/

	switch (currententity->model->type)
	{
		case mod_alias:
			R_DrawAliasModel (currententity);
			break;

		case mod_md3:
			R_DrawQ3Model (currententity);
			break;

#ifndef _WIN32
        case mod_brush:
        case mod_sprite:
            break;          // shut up compiler warning
#endif
	}


	gl_interpolate_transform.value = old_interpolate_transform;

//	glDepthRange (gldepthmin, gldepthmax);
	glDepthRange (0, 1);
}

/*************************JDH**************************/

/*
=============
VisibleToClient -- johnfitz

PVS test encapsulated in a nice function
=============
*/
qboolean VisibleToClient (const edict_t *client, const edict_t *test)
{
	byte	*pvs;
	vec3_t	org;
	int		i;

	VectorAdd (client->v.origin, client->v.view_ofs, org);
	pvs = SV_FatPVS (org);

	for (i=0 ; i < test->num_leafs ; i++)
		if (pvs[test->leafnums[i] >> 3] & (1 << (test->leafnums[i]&7) ))
			return true;

	return false;
}

/*
=============
GL_PolygonOffset -- johnfitz

negative offset moves polygon closer to camera
=============
*/
void GL_PolygonOffset (int offset)
{
	if (offset > 0)
	{
		glEnable (GL_POLYGON_OFFSET_FILL);
		glEnable (GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(1, offset);
	}
	else if (offset < 0)
	{
		glEnable (GL_POLYGON_OFFSET_FILL);
		glEnable (GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-1, offset);
	}
	else
	{
		glDisable (GL_POLYGON_OFFSET_FILL);
		glDisable (GL_POLYGON_OFFSET_LINE);
	}
}

/*
================
R_EmitWireBox -- johnfitz -- draws one axis aligned bounding box
================
*/
void R_EmitWireBox (const vec3_t mins, const vec3_t maxs)
{
	glBegin (GL_QUAD_STRIP);
	glVertex3f (mins[0], mins[1], mins[2]);
	glVertex3f (mins[0], mins[1], maxs[2]);
	glVertex3f (maxs[0], mins[1], mins[2]);
	glVertex3f (maxs[0], mins[1], maxs[2]);
	glVertex3f (maxs[0], maxs[1], mins[2]);
	glVertex3f (maxs[0], maxs[1], maxs[2]);
	glVertex3f (mins[0], maxs[1], mins[2]);
	glVertex3f (mins[0], maxs[1], maxs[2]);
	glVertex3f (mins[0], mins[1], mins[2]);
	glVertex3f (mins[0], mins[1], maxs[2]);
	glEnd ();
}

/*
================
R_ShowBoundingBoxes -- johnfitz

draw bounding boxes -- the server-side boxes, not the renderer cullboxes
================
*/
void R_ShowBoundingBoxes (void)
{
	extern		edict_t *sv_player;
	vec3_t		mins, maxs;
	edict_t		*ed;
	entity_t	*ent;
	int			i;
	model_t		*model;

	if (!r_showbboxes.value || !r_drawentities.value)
		return;

	if (sv.active)
	{
		if (cl.maxclients > 1)
			return;
	}
	else if (!cls.demoplayback)
		return;

	glDisable (GL_DEPTH_TEST);
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	GL_PolygonOffset (OFFSET_SHOWTRIS);
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_CULL_FACE);
	glColor3f (1,1,1);

	if (sv.active)
	{
		for (i=0, ed=NEXT_EDICT(sv.edicts) ; i<sv.num_edicts ; i++, ed=NEXT_EDICT(ed))
		{
			if (ed == sv_player)
				continue; //don't draw player's own bbox

			if (r_showbboxes.value == 1)
			{
				model = cl.model_precache[ed->baseline.modelindex];
				if (model && (model->type != mod_brush))
					continue;
			}

			if ((r_showbboxes.value < 3) && !VisibleToClient (sv_player, ed))
				continue; //don't draw if not in pvs

#ifdef _DEBUG
			if (!strcmp(pr_strings + ed->v.classname, "rotate_object"))
				glColor3f (1,0,0);
			else
				glColor3f (1,1,1);
#endif
		#if 1
			VectorAdd (ed->v.mins, ed->v.origin, mins);
			VectorAdd (ed->v.maxs, ed->v.origin, maxs);
			R_EmitWireBox (mins, maxs);
		#else
			R_EmitWireBox (ed->v.absmin, ed->v.absmax);
		#endif
		}
	}
	else		// JDH: demo playback
	{
		for (i=0; i < cl_numvisedicts; i++)
		{
			ent = cl_visedicts[i];
			model = ent->model;
			if (!model) continue;

			if ((r_showbboxes.value == 1) && (model->type != mod_brush))
				continue;

#ifdef _DEBUG
			if (!strcmp(model->name, "progs/shambler.mdl"))
				glColor3f (1,0,0);
			else
				glColor3f (1,1,1);
#endif
			VectorAdd (model->mins, ent->origin, mins);
			VectorAdd (model->maxs, ent->origin, maxs);
			R_EmitWireBox (mins, maxs);
		}
	}

//	glColor3f (1,1,1);
	glEnable (GL_TEXTURE_2D);
	if (gl_cull.value)
		glEnable (GL_CULL_FACE);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	GL_PolygonOffset (OFFSET_NONE);
	glEnable (GL_DEPTH_TEST);

	Sbar_Changed (); // so we don't get dots collecting on the statusbar
}
/*************************JDH**************************/

/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	extern cvar_t	gl_hwblend;

#ifdef NEWHWBLEND
// 2010/03/30: changed to do view cshifts here not only when hw blending is disabled, 
//			   but also when console/menu are open - see also V_UpdatePalette
	if (!v_blend[3])
		return;
	
	if (V_USING_HWRAMPS())
		return;
#else
	if ((vid_hwgamma_enabled && gl_hwblend.value) || !v_blend[3])
		return;
#endif

	glDisable (GL_ALPHA_TEST);
	glEnable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);

	glColor4fv (v_blend);

	glBegin (GL_QUADS);
	glVertex2f (r_refdef.vrect.x, r_refdef.vrect.y);
	glVertex2f (r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y);
	glVertex2f (r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y + r_refdef.vrect.height);
	glVertex2f (r_refdef.vrect.x, r_refdef.vrect.y + r_refdef.vrect.height);
	glEnd ();

	glDisable (GL_BLEND);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_ALPHA_TEST);

	glColor3f (1, 1, 1);
}

/*
================
R_BrightenRect
================
*/
void R_BrightenRect (int left, int top, int width, int height)
{
#define GAMMA_SCALE 0.2			// determines the weight gamma is given vs. contrast
	extern	float	vid_gamma;
	float		contrast, gamma, f;

#ifdef NEWHWBLEND
	extern cvar_t	gl_hwblend;

	if (V_USING_HWRAMPS())
		return;
#else
	if (vid_hwgamma_enabled /*|| (v_contrast.value <= 1.0)*/)
		return;
#endif

//	f = min(v_contrast.value, 3);
//	f = pow (f, vid_gamma);
	gamma = bound(0.3, v_gamma.value, 3);
	contrast = bound(1, v_contrast.value, 3);

	if (vid_gamma != 1.0)
	{
//		contrast = pow (contrast, vid_gamma);
		gamma /= vid_gamma;
	}

// note: this isn't a proper gamma calculation, but at least it gets brighter
//       as gamma decreases.  (Proper gamma is exponential, and I don't know
//       if that can be done via simple GL)

	f = pow (GAMMA_SCALE, gamma);
	f *= contrast / GAMMA_SCALE;


	glDisable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	glBlendFunc (GL_DST_COLOR, GL_ONE);		// dst*src + dst

	glBegin (GL_QUADS);
	while (f > 1)
	{
		if (f >= 2)
			glColor3f (1, 1, 1);
		else
			glColor3f (f - 1, f - 1, f - 1);
		glVertex2f (left, top);
		glVertex2f (left+width, top);
		glVertex2f (left+width, top+height);
		glVertex2f (left, top+height);
		f *= 0.5;
	}
	glEnd ();
	glColor3f (1, 1, 1);
	
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_BLEND);
	glEnable (GL_TEXTURE_2D);
}

int SignbitsForPlane (const mplane_t *plane)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (plane->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}

void R_SetFrustum (void)
{
	int	i;

// JDH: Quake2 code says doing special case for 90 is incorrect

/*	if (r_refdef.fov_x == 90.0)
	{
		VectorAdd (vpn, vright, frustum[0].normal);
		VectorSubtract (vpn, vright, frustum[1].normal);
	}
	else
*/	{
		// rotate VPN right by FOV_X/2 degrees
		RotatePointAroundVector (frustum[0].normal, vup, vpn, -(90 - r_refdef.fov_x / 2));
		// rotate VPN left by FOV_X/2 degrees
		RotatePointAroundVector (frustum[1].normal, vup, vpn, 90 - r_refdef.fov_x / 2);
	}

/*	if (r_refdef.fov_y == 90.0)
	{
		VectorAdd (vpn, vup, frustum[2].normal);
		VectorSubtract (vpn, vup, frustum[3].normal);
	}
	else
*/	{
		// rotate VPN up by FOV_X/2 degrees
		RotatePointAroundVector (frustum[2].normal, vright, vpn, 90 - r_refdef.fov_y / 2);
		// rotate VPN down by FOV_X/2 degrees
		RotatePointAroundVector (frustum[3].normal, vright, vpn, -(90 - r_refdef.fov_y / 2));
	}

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	vec3_t		testorigin;
	mleaf_t		*leaf;
	extern	float	wateralpha;
//	extern qboolean submerged;

	if (nehahra)
	{
		if (r_oldsky.value && r_skybox.string[0])
			Cvar_SetDirect (&r_skybox, "");
		if (!r_oldsky.value && !r_skybox.string[0])
			Cvar_SetDirect (&r_skybox, prev_skybox);
	}

	wateralpha = r_wateralpha.value;

	R_AnimateLight ();

	r_framecount++;

// build the transformation matrix for the given view angles
	VectorCopy (r_refdef.vieworg, r_origin);
	AngleVectors (r_refdef.viewangles, vpn, vright, vup);

// current viewleaf
/*******JDH - moved to R_MarkLeaves *******/
	//r_oldviewleaf = r_viewleaf;
	//r_oldviewleaf2 = r_viewleaf2;
/*******JDH*******/

	r_viewleaf = Mod_PointInLeaf (r_origin, cl.worldmodel);
	r_viewleaf2 = NULL;

	// check above and below so crossing solid water doesn't draw wrong
	if (r_viewleaf->contents <= CONTENTS_WATER && r_viewleaf->contents >= CONTENTS_LAVA)
	{
		// look up a bit
		VectorCopy (r_origin, testorigin);
		testorigin[2] += 10;
		leaf = Mod_PointInLeaf (testorigin, cl.worldmodel);

		if (leaf->contents == CONTENTS_EMPTY)
			r_viewleaf2 = leaf;
	}
	else if (r_viewleaf->contents == CONTENTS_EMPTY)
	{
		// look down a bit
		VectorCopy (r_origin, testorigin);
		testorigin[2] -= 10;
		leaf = Mod_PointInLeaf (testorigin, cl.worldmodel);

		if (leaf->contents <= CONTENTS_WATER &&	leaf->contents >= CONTENTS_LAVA)
			r_viewleaf2 = leaf;
	}

	V_SetContentsColor (r_viewleaf->contents);

/*** moved to R_DrawWorld ***
	submerged = (gl_waterfog.value && (r_viewleaf->contents != CONTENTS_EMPTY) && (r_viewleaf->contents != CONTENTS_SOLID));
	if (!submerged)
		Fog_SetupFrame ();
*/
	V_CalcBlend ();

	r_cache_thrash = false;

	c_brush_polys = 0;
	c_alias_polys = 0;
}


void MYgluPerspective (GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble	xmax, ymax;

	ymax = zNear * tan(fovy * M_PI / 360.0);
	xmax = ymax * aspect;

	glFrustum (-xmax, xmax, -ymax, ymax, zNear, zFar);
}

/*
=============
R_SetupGL
=============
*/
void R_SetupGL (void)
{
	float	screenaspect;
	int	x, x2, y2, y, w, h, farclip;

	// set up viewpoint
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	x = r_refdef.vrect.x * glwidth/vid.width;
	x2 = (r_refdef.vrect.x + r_refdef.vrect.width) * glwidth/vid.width;
	y = (vid.height-r_refdef.vrect.y) * glheight/vid.height;
	y2 = (vid.height - (r_refdef.vrect.y + r_refdef.vrect.height)) * glheight/vid.height;

	w = x2 - x;
	h = y - y2;

	glViewport (glx + x, gly + y2, w, h);

	screenaspect = (float)r_refdef.vrect.width/r_refdef.vrect.height;
	farclip = max((int)r_farclip.value, 4096);
	MYgluPerspective (r_refdef.fov_y, screenaspect, 4, farclip);

	glCullFace (GL_FRONT);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glRotatef (-90, 1, 0, 0);	    // put Z going up
	glRotatef (90, 0, 0, 1);	    // put Z going up
	glRotatef (-r_refdef.viewangles[2], 1, 0, 0);
	glRotatef (-r_refdef.viewangles[0], 0, 1, 0);
	glRotatef (-r_refdef.viewangles[1], 0, 0, 1);
	glTranslatef (-r_refdef.vieworg[0], -r_refdef.vieworg[1], -r_refdef.vieworg[2]);

#ifdef SHINYWATER		// put this back in if water reflections is ever fixed!
	glGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);
#endif

	// set drawing parms
	if (gl_cull.value)
		glEnable (GL_CULL_FACE);
	else
		glDisable (GL_CULL_FACE);

	glDisable (GL_BLEND);
	glDisable (GL_ALPHA_TEST);
	glEnable (GL_DEPTH_TEST);
}

/*
===============
R_SetFullbright_f
===============
*/
void R_SetFullbright_f (cmd_source_t src)
{
	float val;
	
	if (Cmd_Argc() != 2)
	{
		Con_Print ("Usage: gl_fbr [0|1] - disable/enable fullbright colors in textures\n");
		return;
	}

	val = atof(Cmd_Argv(1));
	
	Cvar_SetValueDirect (&gl_fb_world, val);
	Cvar_SetValueDirect (&gl_fb_bmodels, val);
	Cvar_SetValueDirect (&gl_fb_models, val);
}

extern qboolean	gl_combine_support;

/*
===============
R_Init
===============
*/
void R_Init (void)
{
	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f, 0);
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f, 0);
	Cmd_AddCommand ("toggleparticles", R_ToggleParticles_f, 0);
	Cmd_AddCommand ("set_interpolated_weapon", Set_Interpolated_Weapon_f, 0);
	Cmd_AddCommand ("gl_fbr", R_SetFullbright_f, 0);		// TomazQuake

	Cvar_RegisterBool(&r_lightmap);
	Cvar_RegisterBool (&r_fullbright);
	Cvar_RegisterBool (&r_drawentities);
	Cvar_RegisterFloat (&r_drawviewmodel, 0, 1);
	Cvar_RegisterFloat (&r_viewmodelsize, 0, 1);
	Cvar_RegisterFloat (&r_shadows, 0, 1);
	Cvar_RegisterFloat (&r_wateralpha, 0, 1);
	Cvar_RegisterBool (&r_dynamic);
	Cvar_RegisterInt (&r_flatlightstyles, 0, 2);
	Cvar_RegisterBool (&r_novis);
	Cvar_RegisterBool (&r_speeds);
	Cvar_RegisterBool (&r_fullbrightskins);
	Cvar_RegisterInt (&r_skytype, 0, 2);
	Cvar_RegisterString (&r_skycolor);
	Cvar_RegisterString (&r_skybox);
	Cvar_RegisterInt (&r_farclip, 4096, 16384);		// JDH: max distance in a 8192-unit cube is 14189

	// fenix@io.com: register new cvar for model interpolation
	Cvar_RegisterBool (&gl_interpolate_animation);
	Cvar_RegisterBool (&gl_interpolate_transform);

	Cvar_RegisterBool (&gl_finish);
	Cvar_RegisterBool (&gl_clear);

	Cvar_RegisterBool  (&gl_cull);
//	Cvar_RegisterBool  (&gl_ztrick);
	Cvar_RegisterBool  (&gl_smoothmodels);
	Cvar_RegisterBool  (&gl_affinemodels);
	Cvar_RegisterBool  (&gl_polyblend);
	Cvar_RegisterBool  (&gl_flashblend);
	Cvar_RegisterInt   (&gl_playermip, 0, 4);
	Cvar_RegisterBool  (&gl_nocolors);
	Cvar_RegisterBool  (&gl_loadlitfiles);
	Cvar_RegisterBool  (&gl_doubleeyes);
	Cvar_RegisterInt   (&gl_interdist, INTERP_WEAP_MINDIST, INTERP_WEAP_MAXDIST);
	Cvar_RegisterInt   (&gl_waterfog, 0, 2);
	Cvar_RegisterFloat (&gl_waterfog_density, 0, 1);
	Cvar_RegisterBool  (&gl_detail);
	Cvar_RegisterBool  (&gl_caustics);
	Cvar_RegisterFloat (&gl_ringalpha, 0, 1);
	Cvar_RegisterBool  (&gl_fb_world);
	Cvar_RegisterBool  (&gl_fb_bmodels);
	Cvar_RegisterBool  (&gl_fb_models);
	Cvar_RegisterBool  (&gl_solidparticles);
	Cvar_RegisterBool  (&gl_vertexlights);
	Cvar_Register      (&gl_subdivide_size);
	Cvar_RegisterFloat (&r_modelbrightness, 0, 2);

	Cvar_RegisterInt   (&gl_part_explosions, 0, 2);
	Cvar_RegisterBool  (&gl_part_trails);
	Cvar_RegisterBool  (&gl_part_spikes);
	Cvar_RegisterBool  (&gl_part_gunshots);
	Cvar_RegisterBool  (&gl_part_blood);
	Cvar_RegisterBool  (&gl_part_telesplash);
	Cvar_RegisterBool  (&gl_part_blobs);
	Cvar_RegisterBool  (&gl_part_lavasplash);
	Cvar_RegisterBool  (&gl_part_inferno);
	Cvar_RegisterBool  (&gl_part_flames);
	Cvar_RegisterBool  (&gl_part_lightning);
	Cvar_RegisterBool  (&gl_part_spiketrails);

#ifdef SHINYWATER
	Cvar_RegisterFloat (&gl_shinywater, 0, 1);		// JT030105 - reflections
#endif

// JDH
#ifdef FITZWORLD
	Cvar_RegisterBool  (&r_fastworld);
#endif

	Cvar_RegisterInt   (&r_showbboxes, 0, 3);
	Cvar_RegisterBool  (&r_oldsky);
	Cvar_RegisterBool  (&gl_glows);
	Cvar_RegisterBool  (&gl_cache_ms2);
	Cvar_RegisterInt   (&gl_lightmode, 0, 3);
	Cvar_RegisterInt   (&gl_skyclip, 4096, 16384);
//	Cvar_RegisterFloat (&gl_skyfog, 0, 1);
	Cvar_RegisterBool  (&gl_zfightfix);

/****** TEMP!!! (until I can get fast sky code working in all situations ******/
	Cvar_RegisterInt (&gl_skyhack, 0, 2);
/****** TEMP!!! (until I can get fast sky code working in all situations ******/
// JDH

	Cmd_AddLegacyCommand ("loadsky", r_skybox.name);

	if (!strcmp(gl_vendor, "METABYTE/WICKED3D"))
		Cvar_SetDirect (&gl_solidparticles, "1");

	R_InitTextures ();
	R_InitBubble ();
	R_InitParticles ();
	R_InitVertexLights ();

/*******JDH*******/
	Fog_Init (); //johnfitz
/*******JDH*******/

	playertextures = texture_extension_number;
	texture_extension_number += MAX_SCOREBOARD;

	// fullbright skins
	texture_extension_number += MAX_SCOREBOARD;

	// by joe
	skyboxtextures = texture_extension_number;
	texture_extension_number += 6;

	R_InitOtherTextures ();

// JDH: this assumes 4th TMU is used exclusively for detail texture
	if (detailtexture[0] && gl_combine_support && (gl_textureunits >= 4))
	{
		GL_SelectTMUTexture (GL_TEXTURE3_ARB);
		GL_Bind (detailtexture[0]);

		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
		//glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
		//glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi (GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
		GL_SelectTMUTexture (GL_TEXTURE0_ARB);
	}
}

double r_time4, r_time5;
#ifdef _DEBUG
double r_time3_9;
#endif

/*
================
R_RenderScene

r_refdef must be set before the first call
================
*/
void R_RenderScene (void)
{
	extern qboolean	r_skyboxloaded;
	
//	r_drawskylast = (gl_skyhack.value > 0) || ((!r_skyboxloaded || r_oldsky.value) && (r_skytype.value != 0));
	if (r_skyboxloaded && !r_oldsky.value)
		r_drawskylast = (gl_skyhack.value > 0);
	else if (r_skytype.value == 0)
		r_drawskylast = (gl_skyhack.value > 1);		// MH sky
	else
		r_drawskylast = true;			// solid or classic sky
		
	R_SetupFrame ();

	R_SetFrustum ();

	R_SetupGL ();

	R_MarkLeaves ();	// done here so we know if we're in water

#ifdef _DEBUG
	if (r_speeds.value > 1)
		r_time3_9 = Sys_DoubleTime ();
#endif

	R_DrawWorld ();		// adds static entities to the list

	S_ExtraUpdate ();	// don't let sound get messed up if going slow

	if (r_speeds.value > 1)
		r_time4 = Sys_DoubleTime ();

	R_DrawEntitiesOnList ();

	if (r_speeds.value > 1)
		r_time5 = Sys_DoubleTime ();

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
		R_DrawWaterSurfaces ();		// 2009-10-14: moved here from after R_DrawSprites (sm156_zwiffle)

	if (r_drawskylast)		// 2009-07-20: moved here from below so EF_ADDITIVE sprites work
		R_DrawSky ();
							
#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
		R_DrawSprites();

//	if (r_drawskylast)		// 2009-04-22: moved here from R_DrawWorld
//		R_DrawSky ();		//  (also requires that R_DrawBrushModel NOT clear or draw skychain)		
	
	GL_DisableMultitexture ();
}

/*
=============
R_Clear
=============
*/
//int	gl_ztrickframe = 0;

void R_Clear (void)
{
	static qboolean	cleartogray;
	qboolean	clear = false;

	if (gl_clear.value)
	{
		clear = true;
		if (cleartogray)
		{
			glClearColor (1, 0, 0, 0);
			cleartogray = false;
		}
	}
	else if (!vid_hwgamma_enabled && v_contrast.value > 1)
	{
		clear = true;
		if (!cleartogray)
		{
			glClearColor (0.1, 0.1, 0.1, 0);
			cleartogray = true;
		}
	}

/*	if (gl_ztrick.value)
	{
		if (clear)
			glClear (GL_COLOR_BUFFER_BIT);

		gl_ztrickframe = !gl_ztrickframe;
		if (gl_ztrickframe)
		{
			gldepthmin = 0;
			gldepthmax = 0.49999;
			glDepthFunc (GL_LEQUAL);
		}
		else
		{
			gldepthmin = 1;
			gldepthmax = 0.5;
			glDepthFunc (GL_GEQUAL);
		}
	}
	else
	{
		gldepthmin = 0;
		gldepthmax = 1;
		glDepthFunc (GL_LEQUAL);
	}

	glDepthRange (gldepthmin, gldepthmax);
*/
	if (clear)
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else
		glClear (GL_DEPTH_BUFFER_BIT);
}

/*
================
R_RenderView

r_refdef must be set before the first call
================
*/
void R_RenderView (void)
{
	double	time1 = 0, time2, time3;
	extern qboolean submerged;

#ifdef _DEBUG
//	double time0;
#endif

#ifdef _DEBUG
//	time0 = Sys_DoubleTime ();
#endif

	if (!r_worldentity.model || !cl.worldmodel)
		Sys_Error ("R_RenderView: NULL worldmodel");

	if (r_speeds.value)
	{
		time1 = Sys_DoubleTime ();
		glFinish ();
		c_brush_polys = 0;
		c_alias_polys = 0;
	}
	else if (gl_finish.value)
		glFinish ();

	R_Clear ();

	// render normal view

	R_RenderScene ();
	R_RenderDlights ();

	if (r_speeds.value > 1)
		time2 = Sys_DoubleTime ();

	R_DrawParticles ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		R_DrawTransEntitiesOnList (r_viewleaf->contents == CONTENTS_EMPTY); // This restores the depth mask
		R_DrawWaterSurfaces ();
		R_DrawTransEntitiesOnList (r_viewleaf->contents != CONTENTS_EMPTY);
	}
#endif

	if (submerged)
		glDisable (GL_FOG);
	else
		Fog_DisableGFog (); // JDH: from Fitz

	R_DrawViewModel ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
//		R_Mirror ();			// render mirror view
	}
	else
#endif
	if (!gl_notrans.value)	// always true if not -nehahra
		R_DrawTransEntities ();

	R_ShowBoundingBoxes (); // JDH: from Fitz

#ifdef _DEBUG
//	Con_Printf ("%3i ms\n", (int)((Sys_DoubleTime () - time0)*1000));
#endif

	if (r_speeds.value && !cl.paused)
	{
		time3 = Sys_DoubleTime ();
		if (r_speeds.value > 1)
		{
			Con_Printf ("%.1f ms  (%.1f world, %.1f entities)\n", (float)((time3 - time1) * 1000),
						(float)((time2 - time1 - (r_time5 - r_time4)) * 1000),
						(float)((time3 - time2 + (r_time5 - r_time4)) * 1000));
#ifdef _DEBUG
			Con_Printf ("  %.1f for R_DrawWorld\n", (float)((r_time4 - r_time3_9)*1000));
#endif
		}
		else
			Con_Printf ("%3i ms  %4i wpoly %4i epoly\n", (int)((time3 - time1) * 1000), c_brush_polys, c_alias_polys);
	}
}

#endif		//#ifndef RQM_SV_ONLY
