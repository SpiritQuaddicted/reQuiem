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
// gl_alias.c -- alias model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

#ifndef RQM_SV_ONLY

#include "quakedef.h"

#define ALIAS_VERSION		 6
#define ALIAS_VERSION_RAVEN	50			// JDH: not really - just for internal reference

#define ALIAS_ONSEAM	0x0020

typedef enum {ALIAS_SINGLE=0, ALIAS_GROUP} aliasframetype_t;
typedef enum {ALIAS_SKIN_SINGLE=0, ALIAS_SKIN_GROUP} aliasskintype_t;

typedef struct
{
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;
} mdl_t;

// TODO: could be shorts

typedef struct dtriangle_s
{
	int		facesfront;
	int		vertindex[3];
} dtriangle_t;


typedef struct
{
	trivertx_t  min;	// lightnormal isn't used
	trivertx_t  max;	// lightnormal isn't used
} bboxsize_t;

typedef struct
{
	bboxsize_t	bbox;
	char		name[16];	// frame name from grabbing
} daliasframe_t;

typedef struct
{
	int			numframes;
	bboxsize_t  bbox;
} daliasgroup_t;

typedef struct
{
	int		numskins;
} daliasskingroup_t;

typedef struct
{
	float	interval;
} daliasinterval_t;

typedef struct
{
	float	interval;
} daliasskininterval_t;

typedef struct
{
	aliasframetype_t	type;
} daliasframetype_t;

typedef struct
{
	aliasskintype_t	type;
} daliasskintype_t;


#ifdef HEXEN2_SUPPORT

typedef struct
{
	mdl_t	mdl;
	int		num_st_verts;		// only difference from mdl_t
} ravenmdl_t;

typedef struct
{
	int facesfront;
	unsigned short vertindex[3];
	unsigned short stindex[3];
} draventri_t;

#endif	// #ifdef HEXEN2_SUPPORT

/*
==============================================================================

				ALIAS MODELS

==============================================================================
*/

stvert_t	stverts[MAXALIASVERTS];
mtriangle_t	triangles[MAXALIASTRIS];

// a pose is a single set of vertexes. a frame may be
// an animating sequence of poses
trivertx_t	*poseverts[MAXALIASFRAMES];
int			posenum;

byte		aliasbboxmins[3], aliasbboxmaxs[3];

#ifdef HEXEN2_SUPPORT

static vec3_t	mins_H2, maxs_H2;
static float	aliastransform[3][4];

/*
=================
Mod_AliasTransformVector
=================
*/
void Mod_AliasTransformVector (const vec3_t in, vec3_t out)
{
	out[0] = DotProduct(in, aliastransform[0]) + aliastransform[0][3];
	out[1] = DotProduct(in, aliastransform[1]) + aliastransform[1][3];
	out[2] = DotProduct(in, aliastransform[2]) + aliastransform[2][3];
}

#endif	// #ifdef HEXEN2_SUPPORT


/*
=================
Mod_LoadFrame
=================
*/
void * Mod_LoadAliasFrame (const daliasframe_t *pinframe, const bboxsize_t *bbox, int numposes, int numverts,
							maliasframedesc_t *frame)
{
	int i;
	trivertx_t *verts;

	frame->firstpose = posenum;
	frame->numposes = numposes;

	for (i=0 ; i<3 ; i++)
	{
	// these are byte values, so we don't have to worry about endianness
		frame->bboxmin.v[i] = bbox->min.v[i];
		frame->bboxmax.v[i] = bbox->max.v[i];

		aliasbboxmins[i] = min(aliasbboxmins[i], frame->bboxmin.v[i]);
		aliasbboxmaxs[i] = max(aliasbboxmaxs[i], frame->bboxmax.v[i]);
	}

	for (i=0; i<numposes; i++ )
	{
		verts = (trivertx_t *) (pinframe + 1);

	#ifdef HEXEN2_SUPPORT
		if ( hexen2 )
		{
			vec3_t	in, out;
			int		j, k;

			for (j=0; j<numverts; j++)
			{
				in[0] = verts[j].v[0];
				in[1] = verts[j].v[1];
				in[2] = verts[j].v[2];
				Mod_AliasTransformVector(in, out);
				for (k=0 ; k<3 ; k++)
				{
					if (mins_H2[k] > out[k])
						mins_H2[k] = out[k];
					if (maxs_H2[k] < out[k])
						maxs_H2[k] = out[k];
				}
			}
		}
	#endif

		poseverts[posenum] = verts;
		posenum++;
		pinframe = (daliasframe_t *) (verts + numverts);
	}

	return (void *)pinframe;
}

/*
=================
Mod_LoadAliasFrames
=================
*/
int Mod_LoadAliasFrames (const mdl_t *pinmodel, daliasframetype_t *pframetype, aliashdr_t *pheader)
{
	int i, j, numposes;
	maliasframedesc_t	*poutframe;
	daliasframe_t		*pinframe;
	daliasgroup_t		*pingroup;
	bboxsize_t			*bbox;
	daliasinterval_t	*pintervals;

	posenum = 0;
	for (i=0 ; i<pheader->numframes ; i++)
	{
		poutframe = &pheader->frames[i];

		for (j=0 ; j<3 ; j++)
		{
			poutframe->scale[j] = LittleFloat(pinmodel->scale[j]);
			poutframe->translate[j] = LittleFloat(pinmodel->scale_origin[j]);
		}

		if (LittleLong (pframetype->type) == ALIAS_SINGLE)
		{
			numposes = 1;
			pinframe = (daliasframe_t *) (pframetype + 1);
			strcpy (poutframe->name, pinframe->name);
			bbox = &pinframe->bbox;
		}
		else		// flames, w_spike
		{
			pingroup = (daliasgroup_t *)(pframetype + 1);
			numposes = LittleLong (pingroup->numframes);
			bbox = &pingroup->bbox;

			pintervals = (daliasinterval_t *)(pingroup + 1);
			poutframe->interval = LittleFloat (pintervals->interval);
			pinframe = (daliasframe_t *) (pintervals + numposes);
		}

		pframetype = Mod_LoadAliasFrame (pinframe, bbox, numposes, pheader->numverts, poutframe);
	}

	return posenum;
}

static const char * mod_eyes[] = {"eyes", NULL};
static const char * mod_flames[] =
{
	"flame0", "flame", "flame2", "brazshrt", "braztall", "longtrch", "fireball", "flame_pyre", NULL
};
static const char * mod_bolts[] = {"bolt", "bolt2", "bolt3", NULL};

// NOTE: v_axe and v_ham are intentionally omitted from the weapon list.
//		This is because they don't have a "firing" animation, and thus
//		don't need any special treatment when interpolating.  (In fact,
//		the special weapon interpolation code fails on them because
//		some vertices move a great distance between frames)  - JDH
static const char * mod_weapons[] =
{
	"v_shot", "v_shot2",  "v_nail", "v_nail2", "v_rock",  "v_rock2",  "v_laserg",
	"v_prox", "v_grpple", "v_lava", "v_lava2", "v_multi", "v_multi2", "v_plasma",
	"v_star", "v_bomb",   "v_lance", NULL
};
static const char * mod_lavaballs[] = {"lavaball", NULL};
static const char * mod_spikes[]    = {"spike", "s_spike", NULL};
static const char * mod_shamblers[] = {"shambler", NULL};

static const char * mod_heads[] =
{
	"h_dog",  "h_guard", "h_mega", "h_knight", "h_hellkn", "h_wizard",
	"h_ogre", "h_demon", "h_shal", "h_shams",  "h_zombie", "h_player", NULL
};

static const aliastype_t mod_hints[] =
{
	{MOD_EYES,        mod_eyes},
	{MOD_FLAME,       mod_flames},
	{MOD_THUNDERBOLT, mod_bolts},
	{MOD_WEAPON,      mod_weapons},
	{MOD_LAVABALL,    mod_lavaballs},
	{MOD_SPIKE,       mod_spikes},
	{MOD_SHAMBLER,    mod_shamblers},
	{MOD_HEAD,        mod_heads}
};
#define NUM_ALIAS_HINTS (sizeof(mod_hints)/sizeof(mod_hints[0]))

int Mod_GetAliasType (const char *modname, const char *reqprefix, const aliastype_t typelists[], int numlists)
{
// some models are special
	char *ext, shortname[MAX_QPATH];
	int  i, j;

	if (!COM_FilenamesEqualn(modname, reqprefix, strlen(reqprefix)))
		return -1;		// anything not in progs folder

	modname += 6;			// skip over folder prefix

	ext = COM_FileExtension (modname);
	for (i = 0; i < NUM_MODEL_EXTS; i++)
	{
		if (COM_FilenamesEqual(ext, com_mdl_exts[i]+1))		// +1 to skip the dot
			break;
	}
	if (i == NUM_MODEL_EXTS)
		return -1;

	/*if (!COM_FilenamesEqual(ext, "mdl") && !COM_FilenamesEqual(ext, "md3") && !COM_FilenamesEqual(ext, "md2"))
		return -1;*/

	Q_strncpy (shortname, sizeof(shortname), modname, ext-1-modname);

	for (i = 0; i < numlists; i++)
	{
		for (j = 0; typelists[i].names[j]; j++)
		{
			if (COM_FilenamesEqual (shortname, typelists[i].names[j]))
				return typelists[i].type;
		}
	}

	return -1;
}
/*
=================
Mod_GetAliasHint
=================
*/
modhint_t Mod_GetAliasHint (const char *modname)
{
// some models are special
	int type;

	// NOTE: comparing not only with player.mdl, but with all models
	// begin with "player" coz we need to support DME models as well!
	if (COM_FilenamesEqualn(modname, "progs/player", 12))
		return MOD_PLAYER;

	type = Mod_GetAliasType (modname, "progs/", mod_hints, NUM_ALIAS_HINTS);
	if (type < 0)
		type = MOD_NORMAL;

	return type;
/*
	char *ext, shortname[MAX_QPATH];
	int  i, j;

  if (!COM_FilenamesEqualn(modname, "progs/", 6))
		return MOD_NORMAL;		// anything not in progs folder

	modname += 6;			// skip over folder prefix

	ext = COM_FileExtension (modname);
	if (!COM_FilenamesEqual(ext, "mdl"))
		return MOD_NORMAL;

	// NOTE: comparing not only with player.mdl, but with all models
	// begin with "player" coz we need to support DME models as well!
	if (COM_FilenamesEqualn(modname, "progs/player", 12))
		return MOD_PLAYER;

	Q_strncpy (shortname, sizeof(shortname), modname, ext-1-modname);

	for (i = 0; i < NUM_ALIAS_HINTS; i++)
	{
		for (j = 0; mod_hints[i].names[j]; j++)
		{
			if (COM_FilenamesEqual (shortname, mod_hints[i].names[j]))
				return mod_hints[i].type;
		}
	}
*/
/*
	if (COM_FilenamesEqual(modname, "eyes.mdl"))
		return MOD_EYES;

	if (COM_FilenamesEqual(modname, "flame0.mdl")	||
		COM_FilenamesEqual(modname, "flame.mdl")	||
		COM_FilenamesEqual(modname, "flame2.mdl"))
		return MOD_FLAME;

	if (COM_FilenamesEqual(modname, "bolt.mdl")	||
		COM_FilenamesEqual(modname, "bolt2.mdl")	||
		COM_FilenamesEqual(modname, "bolt3.mdl"))
		return MOD_THUNDERBOLT;

	// NOTE: v_axe and v_ham are intentionally omitted from the weapon list.
	//		This is because they don't have a "firing" animation, and thus
	//		don't need any special treatment when interpolating.  (In fact,
	//		the special weapon interpolation code fails on them because
	//		some vertices move a great distance between frames)  - JDH

	if (COM_FilenamesEqual(modname, "v_shot.mdl")	||
		COM_FilenamesEqual(modname, "v_shot2.mdl")	||
		COM_FilenamesEqual(modname, "v_nail.mdl")	||
		COM_FilenamesEqual(modname, "v_nail2.mdl")	||
		COM_FilenamesEqual(modname, "v_rock.mdl")	||
		COM_FilenamesEqual(modname, "v_rock2.mdl")	||
	// hipnotic weapons
		COM_FilenamesEqual(modname, "v_laserg.mdl")||
		COM_FilenamesEqual(modname, "v_prox.mdl")	||
	// rogue weapons
		COM_FilenamesEqual(modname, "v_grpple.mdl")||	// ?
		COM_FilenamesEqual(modname, "v_lava.mdl")	||
		COM_FilenamesEqual(modname, "v_lava2.mdl")	||
		COM_FilenamesEqual(modname, "v_multi.mdl")	||
		COM_FilenamesEqual(modname, "v_multi2.mdl")||
		COM_FilenamesEqual(modname, "v_plasma.mdl")||	// ?
		COM_FilenamesEqual(modname, "v_star.mdl"))		// ?
		return MOD_WEAPON;

	if (COM_FilenamesEqual(modname, "lavaball.mdl"))
		return MOD_LAVABALL;

	if (COM_FilenamesEqual(modname, "spike.mdl")	||
		COM_FilenamesEqual(modname, "s_spike.mdl"))
		return MOD_SPIKE;

	if (COM_FilenamesEqual(modname, "shambler.mdl"))
		return MOD_SHAMBLER;

// JDH: Quoth support for flames & viewmodels
	if (COM_FilenamesEqual(modname, "brazshrt.mdl")	||
		COM_FilenamesEqual(modname, "braztall.mdl")	||
		COM_FilenamesEqual(modname, "longtrch.mdl") ||
		COM_FilenamesEqual(modname, "fireball.mdl") ||
		COM_FilenamesEqual(modname, "flame_pyre.mdl"))
		return MOD_FLAME;

	if (COM_FilenamesEqual(modname, "v_bomb.mdl")	||
//		COM_FilenamesEqual(modname, "v_ham.mdl")	||
		COM_FilenamesEqual(modname, "v_lance.mdl"))
		return MOD_WEAPON;
*/
	return MOD_NORMAL;
}

#ifdef HEXEN2_SUPPORT

static const char * mod_players_H2[] = {"paladin", "crusader", "necro", "assassin", "succubus", NULL};
static const char * mod_flames_H2[] =
{
	"flame", "flame1", "flame2", "cflmtrch", "flaming", "eflmtrch",
	"mflmtrch", "candle", "newfire", NULL
};

static const aliastype_t mod_hints_H2[] =
{
	{MOD_PLAYER, mod_players_H2},
	{MOD_FLAME,  mod_flames_H2}
};
#define NUM_ALIAS_HINTS_H2 (sizeof(mod_hints_H2)/sizeof(mod_hints_H2[0]))

/*
===================
Mod_GetAliasHint_H2
===================
*/
modhint_t Mod_GetAliasHint_H2 (const char *modname)
{
// some models are special
	int type = Mod_GetAliasType (modname, "models/", mod_hints_H2, NUM_ALIAS_HINTS_H2);
	if (type < 0)
		type = MOD_NORMAL;

	return type;

/*	char *ext, shortname[MAX_QPATH];
	int  i, j;

	if (!COM_FilenamesEqualn (modname, "models/", 7))
		return MOD_NORMAL;		// anything not in default folder

	modname += 7;			// skip over folder prefix

	ext = COM_FileExtension (modname);
	if (!COM_FilenamesEqual(ext, "mdl"))
		return MOD_NORMAL;

	Q_strncpy (shortname, sizeof(shortname), modname, ext-1-modname);

	for (i = 0; i < NUM_ALIAS_HINTS_H2; i++)
	{
		for (j = 0; mod_hints_H2[i].names[j]; j++)
		{
			if (COM_FilenamesEqual (shortname, mod_hints_H2[i].names[j]))
				return mod_hints_H2[i].type;
		}
	}
*/
/*	if (COM_FilenamesEqual (modname, "paladin.mdl") ||
		COM_FilenamesEqual (modname, "crusader.mdl") ||
		COM_FilenamesEqual (modname, "necro.mdl") ||
		COM_FilenamesEqual (modname, "assassin.mdl") ||
		COM_FilenamesEqual (modname, "succubus.mdl"))
	{
		return MOD_PLAYER;
	}

	if (COM_FilenamesEqual (modname, "flame.mdl") ||
		COM_FilenamesEqual (modname, "flame1.mdl") ||
		COM_FilenamesEqual (modname, "flame2.mdl") ||
		COM_FilenamesEqual (modname, "cflmtrch.mdl") ||
		COM_FilenamesEqual (modname, "flaming.mdl") ||
		COM_FilenamesEqual (modname, "eflmtrch.mdl") ||
		COM_FilenamesEqual (modname, "mflmtrch.mdl") ||
		COM_FilenamesEqual (modname, "candle.mdl") ||
		COM_FilenamesEqual (modname, "newfire.mdl"))
	{
		return MOD_FLAME;
	}
*/
	//****TODO: other models that maybe shouldn't be interpolated:
	//	- tornato (Portals pak3)

	return MOD_NORMAL;
}
#endif

//=========================================================

#ifndef RQM_SV_ONLY

/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes - Ed
=================
*/

#if 1
typedef struct
{
	short	x, y;
} floodfill_t;

// must be a power of 2
#define	FLOODFILL_FIFO_SIZE	0x1000
#define	FLOODFILL_FIFO_MASK	(FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP(offset, dx, dy)					\
{									\
	if (pos[(offset)] == fillcolor)					\
	{								\
		pos[(offset)] = 255;						\
		fifo[inpt].x = x + (dx);	\
		fifo[inpt].y = y + (dy);	\
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;		\
	}								\
	else if (pos[(offset)] != 255) fdc = pos[(offset)];			\
}

void Mod_FloodFillSkin (byte *skin, int skinwidth, int skinheight)
{
	byte		fillcolor = *skin;	// assume this is the pixel to fill
	floodfill_t	fifo[FLOODFILL_FIFO_SIZE];
	int		x, y, fdc, inpt, outpt = 0, filledcolor = 0;
	byte *pos;

	// can't fill to transparent color
	if (fillcolor == 255)
		return;

	// attempt to find opaque black
	for (x=0 ; x<256 ; x++)
	{
//		if (d_8to24table[x] == 255)	// alpha 1.0
		if (d_8to24table[x] == (255 << 24))	// JDH: isn't this what we want?
		{
			filledcolor = x;
			break;
		}
	}

	// can't fill to filled color (used as visited marker)
	if (fillcolor == filledcolor)
		return;

	fifo[0].x = 0;
	fifo[0].y = 0;
	inpt = 1;

	while (outpt != inpt)
	{
		x = fifo[outpt].x;
		y = fifo[outpt].y;
		fdc = filledcolor;
		pos = &skin[x + skinwidth*y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)
			FLOODFILL_STEP(-1, -1, 0);
		if (x < skinwidth - 1)
			FLOODFILL_STEP(1, 1, 0);
		if (y > 0)
			FLOODFILL_STEP(-skinwidth, 0, -1);
		if (y < skinheight - 1)
			FLOODFILL_STEP(skinwidth, 0, 1);
		*pos = fdc;
	}
}

#else		// WIP below

void Mod_FloodFillSkin (byte *skin, int skinwidth, int skinheight)
{
	byte		fillcolor = *skin;	// assume this is the pixel to fill
	int			filledcolor = 0, x, y, start;
	qboolean	bg;

	// can't fill to transparent color
	if (fillcolor == 255)
		return;

	// attempt to find opaque black
	for (x=0 ; x<256 ; x++)
	{
//		if (d_8to24table[x] == 255)	// alpha 1.0
		if (d_8to24table[x] == (255 << 24))	// JDH: isn't this what we want?
		{
			filledcolor = x;
			break;
		}
	}

	// can't fill to filled color (used as visited marker)
	if (fillcolor == filledcolor)
		return;

	for (y = 0; y < skinheight; y++)
	{
		start = 0;
		x = 0;
		bg = (skin[y*skinwidth] == fillcolor);
		while (x < skinwidth-1)
		{
			if (bg)
			{
				if (skin[y*skinwidth + x+1] != fillcolor)
					break;

				if ((y < skinheight-1) && (skin[(y+1)*skinwidth + x] != fillcolor))
					break;
			}

			bg = !bg;
		}
	}
}
#endif

static const char *alias_paths[] = {"textures/models/", "textures/", "", "", NULL};
	// 3rd & 4th paths set dynamically based on model's folder

/*
=================
Mod_TryLoadAliasTexture
=================
*/
/*void Mod_TryLoadAliasTexture (char *path, char *filename, char *identifier, int mode, skin_t *skin)
{
	skin->gl_texturenum = GL_LoadTextureImage (path, filename, identifier, mode, mod_loadpath->dir_level);
	if (skin->gl_texturenum)
	{
		// JDH - added r_fullbrightskins check
		if (r_fullbrightskins.value)
		{
			skin->fb_texturenum = GL_LoadTextureImage (path, va("%s_luma", filename), va("@fb_%s", identifier),
												mode | TEX_LUMA, mod_loadpath->dir_level);
			if (skin->fb_texturenum)
				skin->fb_isLuma = true;
		}
	}
}
*/

/*
=================
Mod_LoadAliasModelTexture
=================
*/
void Mod_LoadAliasModelTexture (const model_t *mod, const char *identifier, const char *basename, int mode, skin_t *skin)
{
	char		filename1[MAX_QPATH], filename2[MAX_QPATH], path1[MAX_QPATH], path2[MAX_QPATH], *name;
	const char	*namelist[3];
	int			len2;

	if (no24bit || isDedicated)		// JDH: added 24-bit test here
		return;

	if (!gl_externaltextures_models.value)
		return;

	namelist[0] = identifier;
	namelist[1] = filename2;
	namelist[2] = NULL;

	name = COM_SkipPath (mod->name);
	if (name != mod->name)
	{
		*(name-1) = 0;		// temp change of / to 0
		Q_snprintfz (path1, sizeof(path1), "%s/", mod->name);
		alias_paths[2] = path1;
		Q_snprintfz (path2, sizeof(path2), "textures/%s/", mod->name);
		alias_paths[3] = path2;
		*(name-1) = '/';
	}
	else alias_paths[2] = NULL;

	len2 = Q_snprintfz (filename2, sizeof(filename2), "%s%s", name, identifier + strlen(basename));
			// model name + suffix indicating skinnum or framenum

	skin->gl_texturenum = GL_LoadTextureImage_MultiSource (alias_paths, namelist, identifier, mode, mod_loadpath->dir_level);

	if (skin->gl_texturenum /*&& r_fullbrightskins.value*/)		// JDH: added r_fullbrightskins check
	{
		namelist[0] = filename1;
		Q_snprintfz (filename1, sizeof(filename1), "%s_luma", identifier);
		Q_strcpy (filename2 + len2, "_luma", sizeof(filename2)-len2);

		skin->fb_texturenum = GL_LoadTextureImage_MultiSource (alias_paths, namelist, va("@fb_%s", identifier),
											mode | TEX_LUMA, mod_loadpath->dir_level);
		if (skin->fb_texturenum)
			skin->fb_isLuma = true;
	}

/*	Mod_TryLoadAliasTexture ("textures/models/", identifier, identifier, mode, skin);
	if (skin->gl_texturenum) return;

	Mod_TryLoadAliasTexture ("textures/", identifier, identifier, mode, skin);
	if (skin->gl_texturenum) return;

	Q_snprintfz (loadpath, sizeof(loadpath), "%s%s", mod->name, identifier + strlen(basename));
		// suffix indicating skinnum or framenum
	Mod_TryLoadAliasTexture ("textures/", loadpath, identifier, mode, skin);
*/
}

#endif		//#ifndef RQM_SV_ONLY

/*
===============
Mod_LoadSkin
===============
*/
void Mod_LoadSkin (const model_t *mod, byte *data, const aliashdr_t *pheader, const char *identifier,
				   const char *basename, int tex_flags, skingroup_t *skingroup, int skinnum)
{
	skin_t *skin;

	skin = &skingroup->skins[skinnum];

	skin->gl_texturenum = 0;
	skin->fb_texturenum = 0;
	skin->fb_isLuma = false;

#ifndef RQM_SV_ONLY
	tex_flags |= TEX_NOSTRETCH;

	Mod_LoadAliasModelTexture (mod, identifier, basename, tex_flags, skin);

	if (skin->gl_texturenum)
	{
		// set new width & height so coordinate scaling works while meshing
		skin->h_value = image_width;
		skin->v_value = image_height;
		if (mod->modhint == MOD_PLAYER)
			skingroup->texels = 0;			// so that changes to _cl_color do not override 24-bit texture
		return;
	}

	skin->h_value = pheader->skinwidth;
	skin->v_value = pheader->skinheight;

	if (strcmp(identifier, "v_light_0"))	// JDH: **TEMPORARY** fix for blue fringe on laser gun
		Mod_FloodFillSkin (data, pheader->skinwidth, pheader->skinheight);

#ifdef _DEBUG
	if (!strcmp(identifier, "null_0"))
		skin->fb_isLuma = false;

/*	if (!strcmp(identifier, "v_light_0"))
	{
		int i, size = pheader->skinwidth*pheader->skinheight*3;
		byte *data24 = Q_malloc(size);

		for (i = 0; i < size/3; i++)
		{
			data24[i*3] = d_8to24table[data[i]] & 0x0000FF;
			data24[i*3+1] = (d_8to24table[data[i]] & 0x00FF00) >> 8;
			data24[i*3+2] = (d_8to24table[data[i]] & 0xFF0000) >> 16;
		}

		Image_WriteFile ("v_light.tga", data24, pheader->skinwidth, pheader->skinheight);
		free (data24);
	}*/
#endif

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (mod->flags & EF_HOLEY)
			tex_flags |= TEX_ALPHA_MODE2;

		else if (mod->flags & EF_TRANSPARENT)
			tex_flags |= TEX_ALPHA_MODE1;

		else if (mod->flags & EF_SPECIAL_TRANS)
			tex_flags |= TEX_ALPHA_MODE3;
	}
#endif

	skin->gl_texturenum = GL_LoadTexture (identifier, pheader->skinwidth, pheader->skinheight, data, tex_flags, 1);
//	if (r_fullbrightskins.value)
	{
		if (Img_HasFullbrights (data, pheader->skinwidth * pheader->skinheight))
		{
			skin->fb_texturenum = GL_LoadTexture (va("@fb_%s", identifier), pheader->skinwidth, pheader->skinheight,
												data, tex_flags | TEX_FULLBRIGHT, 1);
		}
	}
#endif		//#ifndef RQM_SV_ONLY
}


/*
===============
Mod_LoadSkinGroup
===============
*/
void Mod_LoadSkinGroup (byte **data, aliashdr_t *pheader, const char *basename, int skinnum,
							int numingroup, int tex_flags, const model_t *mod)
{
	skingroup_t	*skingroup;
	int			i, size;
	byte		*texels;
	char		identifier[128];
	skin_t		*skin_src, *skin_dst;

	skingroup = &pheader->skins[skinnum];

	// save 8 bit texels for the player model to remap
	size = pheader->skinwidth * pheader->skinheight;
	if (mod->modhint == MOD_PLAYER)
	{
		texels = Hunk_AllocName (size, mod_loadname);
		skingroup->texels = texels - (byte *)pheader;
		memcpy (texels, *data, size);
	}
	else skingroup->texels = 0;

	for (i=0 ; i<numingroup ; i++)
	{
		if (numingroup == 1)
		{
			Q_snprintfz (identifier, sizeof(identifier), "%s_%i", basename, skinnum);
		}
		else Q_snprintfz (identifier, sizeof(identifier), "%s_%i_%i", basename, skinnum, i);


		Mod_LoadSkin (mod, *data, pheader, identifier, basename, tex_flags, skingroup, i&3);
		*data += size;
	}

	for ( ; i<4 ; i++)
	{
		skin_src = &skingroup->skins[i-numingroup];
		skin_dst = &skingroup->skins[i];

		skin_dst->gl_texturenum = skin_src->gl_texturenum;
		skin_dst->fb_texturenum = skin_src->fb_texturenum;
		skin_dst->h_value = skin_src->h_value;
		skin_dst->v_value = skin_src->v_value;
		skin_dst->fb_isLuma = skin_src->fb_isLuma;
	}
}

/*
===============
Mod_LoadAliasSkins
===============
*/
void *Mod_LoadAliasSkins (aliashdr_t *pheader, daliasskintype_t *pskintype, const model_t *mod)
{
	int						i, groupskins, tex_flags;
	char					basename[MAX_QPATH];
	byte					*skin;
	daliasskingroup_t		*pinskingroup;
	daliasskininterval_t	*pinskinintervals;

	if (pheader->numskins < 1 || pheader->numskins > MAX_SKINS)
		Sys_Error ("Mod_LoadAliasSkins: Invalid # of skins: %d\n", pheader->numskins);

	COM_StripExtension (COM_SkipPath(mod->name), basename, sizeof(basename));

#ifndef RQM_SV_ONLY
	tex_flags = gl_picmip_all.value ? TEX_MIPMAP : 0;
#else
	tex_flags = 0;
#endif

	for (i=0; i < pheader->numskins; i++)
	{
		if (pskintype->type == ALIAS_SKIN_SINGLE)
		{
			groupskins = 1;
			skin = (byte *)(pskintype + 1);
		}
		else
		{
			// animating skin group. yuck.
			pskintype++;
			pinskingroup = (daliasskingroup_t *)pskintype;
			groupskins = LittleLong (pinskingroup->numskins);
			pinskinintervals = (daliasskininterval_t *)(pinskingroup + 1);

			skin = (byte *) (pinskinintervals + groupskins);
		}

		Mod_LoadSkinGroup (&skin, pheader, basename, i, groupskins, tex_flags, mod);
		pskintype = (daliasskintype_t *) skin;
	}

	return (void *)pskintype;
}

//=========================================================================

/*
=================
Mod_LoadAliasHeader
=================
*/
void Mod_LoadAliasHeader (const mdl_t *pinmodel, aliashdr_t *pheader, const char *modname, qboolean isRaven)
{
	int numframes;

// endian-adjust and copy the data, starting with the alias model header
//	pheader->boundingradius = LittleFloat (pinmodel->boundingradius);		// JDH: not used
	pheader->numskins = LittleLong (pinmodel->numskins);
	pheader->skinwidth = LittleLong (pinmodel->skinwidth);
	pheader->skinheight = LittleLong (pinmodel->skinheight);

/************JDH***********/
	//if (pheader->skinheight > MAX_LBM_HEIGHT)
	//	Sys_Error ("Mod_LoadAliasHeader: model %s has a skin taller than %d", modname, MAX_LBM_HEIGHT);
/************JDH***********/

	pheader->numverts = LittleLong (pinmodel->numverts);

	if (pheader->numverts <= 0)
		Sys_Error ("Mod_LoadAliasHeader: model %s has no vertices", modname);

	if (pheader->numverts > MAXALIASVERTS)
		Sys_Error ("Mod_LoadAliasHeader: model %s has too many vertices", modname);

	if (pheader->numverts > MAXALIASVERTS_OLD)
		Con_DPrintf ("\x02""Warning: model %s has %d vertices (standard limit is %d)\n",
						modname, pheader->numverts, MAXALIASVERTS_OLD);

#ifdef HEXEN2_SUPPORT
	if (isRaven)
		pheader->numstverts = LittleLong (((ravenmdl_t *)pinmodel)->num_st_verts);
	else
#endif
		pheader->numstverts = pheader->numverts;

	pheader->numtris = LittleLong (pinmodel->numtris);

	if (pheader->numtris <= 0)
		Sys_Error ("Mod_LoadAliasHeader: model %s has no triangles", modname);

	pheader->numframes = LittleLong (pinmodel->numframes);
	numframes = pheader->numframes;
	if (numframes < 1)
		Sys_Error ("Mod_LoadAliasHeader: Invalid # of frames: %d\n", numframes);

	pheader->size = LittleFloat (pinmodel->size) * ALIAS_BASE_SIZE_RATIO;

/*	for (i=0 ; i<3 ; i++)
	{
		pheader->scale[i] = LittleFloat (pinmodel->scale[i]);
		pheader->scale_origin[i] = LittleFloat (pinmodel->scale_origin[i]);
		pheader->eyeposition[i] = LittleFloat (pinmodel->eyeposition[i]);
	}
*/
}

//=========================================================================


/*
=================
Mod_LoadAliasCommon - shared loading code between IDP0 & RAP0
=================
*/
void Mod_LoadAliasCommon (const mdl_t *pinmodel, model_t *mod, int required_version)
{
	int					version, i, j, size, start, end, total;
	aliashdr_t			*pheader;
	stvert_t			*pinstverts;
	dtriangle_t			*pintriangles;
	daliasframetype_t	*pframetype;
	daliasskintype_t	*pskintype;

//	DWORD dwTicks = GetTickCount();

	version = LittleLong (pinmodel->version);

	if (version != required_version)
	{
		Sys_Error ("Mod_LoadAliasModel: %s has wrong version number (%i should be %i)",
					mod->name, version, required_version);
		return;
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		mod->modhint = Mod_GetAliasHint_H2 (mod->name);
	else
#endif
		mod->modhint = Mod_GetAliasHint (mod->name);

// allocate space for a working header, plus all the data except the frames,
// skin and group info
	start = Hunk_LowMark ();
	size = sizeof(aliashdr_t) + (LittleLong(pinmodel->numframes) - 1) * sizeof(maliasframedesc_t);
	pheader = Hunk_AllocName (size, mod_loadname);

	Mod_LoadAliasHeader (pinmodel, pheader, mod->name, (version == ALIAS_VERSION_RAVEN));

	mod->flags = LittleLong (pinmodel->flags);
	mod->synctype = LittleLong (pinmodel->synctype);
	mod->numframes = pheader->numframes;
	mod->type = mod_alias;


//	Con_DPrintf ("LoadAliasModel:1 took %i ms\n", GetTickCount() - dwTicks);

	// load the skins
#ifdef HEXEN2_SUPPORT
	if (version == ALIAS_VERSION_RAVEN)
		size = sizeof (ravenmdl_t);
	else
#endif
		size = sizeof (mdl_t);
	pskintype = (daliasskintype_t *) ((byte *) pinmodel + size);
	pskintype = Mod_LoadAliasSkins (pheader, pskintype, mod);

//	Con_DPrintf ("LoadAliasModel:2 took %i ms\n", GetTickCount() - dwTicks);


	// load base s and t vertices into temporary global storage
	//  (long-term memory is allocated from hunk in GL_MakeAliasModelDisplayLists)
	pinstverts = (stvert_t *)pskintype;
	for (i=0 ; i < pheader->numstverts ; i++)
	{
		stverts[i].onseam = LittleLong (pinstverts[i].onseam);
		stverts[i].s = LittleLong (pinstverts[i].s);
		stverts[i].t = LittleLong (pinstverts[i].t);
	}


// load triangle lists into temporary global storage (see above comment)
	pintriangles = (dtriangle_t *)&pinstverts[pheader->numstverts];

	for (i=0 ; i<pheader->numtris ; i++)
	{
		triangles[i].facesfront = LittleLong (pintriangles[i].facesfront);

		for (j=0 ; j<3 ; j++)
		{
		#ifdef HEXEN2_SUPPORT
			if (version == ALIAS_VERSION_RAVEN)
			{
				triangles[i].vertindex[j] = LittleShort (((draventri_t *)pintriangles)[i].vertindex[j]);
				triangles[i].stindex[j]	  = LittleShort (((draventri_t *)pintriangles)[i].stindex[j]);
			}
			else
			{
				triangles[i].vertindex[j] = LittleLong (pintriangles[i].vertindex[j]);
				triangles[i].stindex[j]	= triangles[i].vertindex[j];
			}
		#else
			triangles[i].vertindex[j] = LittleLong (pintriangles[i].vertindex[j]);
		#endif
		}
	}


// load the frames
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		mins_H2[0] = mins_H2[1] = mins_H2[2] = 32768;
		maxs_H2[0] = maxs_H2[1] = maxs_H2[2] = -32768;

		aliastransform[0][0] = LittleFloat(pinmodel->scale[0]);
		aliastransform[1][1] = LittleFloat(pinmodel->scale[1]);
		aliastransform[2][2] = LittleFloat(pinmodel->scale[2]);
		aliastransform[0][3] = LittleFloat(pinmodel->scale_origin[0]);
		aliastransform[1][3] = LittleFloat(pinmodel->scale_origin[1]);
		aliastransform[2][3] = LittleFloat(pinmodel->scale_origin[2]);
	}
#endif

	aliasbboxmins[0] = aliasbboxmins[1] = aliasbboxmins[2] = 0;
	aliasbboxmaxs[0] = aliasbboxmaxs[1] = aliasbboxmaxs[2] = 255;


	pframetype = (daliasframetype_t *) &pintriangles[pheader->numtris];
	pheader->numposes = Mod_LoadAliasFrames (pinmodel, pframetype, pheader);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		mod->mins[0] = mins_H2[0] - 10;
		mod->mins[1] = mins_H2[1] - 10;
		mod->mins[2] = mins_H2[2] - 10;
		mod->maxs[0] = maxs_H2[0] + 10;
		mod->maxs[1] = maxs_H2[1] + 10;
		mod->maxs[2] = maxs_H2[2] + 10;
	}
	else
#endif
	{
		for (i=0 ; i<3 ; i++)
		{
			mod->mins[i] = aliasbboxmins[i] * LittleFloat(pinmodel->scale[i]) + LittleFloat(pinmodel->scale_origin[i]);
			mod->maxs[i] = aliasbboxmaxs[i] * LittleFloat(pinmodel->scale[i]) + LittleFloat(pinmodel->scale_origin[i]);
		}
	}

	mod->radius = RadiusFromBounds (mod->mins, mod->maxs);

#ifndef RQM_SV_ONLY
	// build the draw lists
	GL_MakeAliasModelDisplayLists (mod, pheader);
#endif

//	Con_DPrintf ("LoadAliasModel:3 took %i ms\n", GetTickCount() - dwTicks);

	// move the complete, relocatable alias model to the cache
	end = Hunk_LowMark ();
	total = end - start;

	Cache_Alloc (&mod->cache, total, mod_loadname);
	if (mod->cache.data)
		memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);
}

/*
=================
Mod_LoadAliasModel
=================
*/
void Mod_LoadAliasModel (model_t *mod, const void *buffer)
{

	Mod_LoadAliasCommon ((mdl_t *) buffer, mod, ALIAS_VERSION);
}

#ifdef HEXEN2_SUPPORT

/*
=================
Mod_LoadRavenModel
- has extra field in header for num ST verts,
  and extra index of them in the triangle list
=================
*/
void Mod_LoadRavenModel (model_t *mod, const void *buffer)
{
//	Con_Printf("Loading NEW model %s\n",mod->name);

	Mod_LoadAliasCommon ((mdl_t *) buffer, mod, ALIAS_VERSION_RAVEN);
}

#endif

#endif		// #ifndef RQM_SV_ONLY
