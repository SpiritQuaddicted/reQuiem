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
// gl_brush.c -- brush model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

#ifdef RQM_SV_ONLY
#  define r_notexture_mip NULL
#endif

vec3_t vec3_hull_min = {-16, -16, -24};
vec3_t vec3_hull_max = { 16,  16,  32};


#ifdef HEXEN2_SUPPORT
	int entity_file_size;
#endif

#define ISTURBTEX(name)		((name)[0] == '*')

//#define ISSKYTEX(name)		((name)[0] == 's' && (name)[1] == 'k' && (name)[2] == 'y')
#define ISSKYTEX(name)		(!Q_strncasecmp((name), "sky", 3))		// whiteroom has "SKY4"

byte		*mod_base;
qboolean	mod_oversized;		// if any coords in worldmodel are > +/-4096

extern model_t *Mod_FindName (const char *name);

#ifdef _DEBUG
#define BSP23TEST
int mod_bspversion;
#endif

/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents < 0)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_TryLoadBrushTexture
=================
*/
/*void Mod_TryLoadBrushTexture (char *path, char *name, int flags, texture_t *tx)
{
	if ((tx->gl_texturenum = GL_LoadTextureImage(path, name, name, flags, 0)))
	{
		if (!ISTURBTEX(name))
		{
			tx->fb_texturenum = GL_LoadTextureImage (path, va("%s_luma", name), va("@fb_%s", name),
														flags | TEX_LUMA, 0);
		}
	}
}
*/

#ifndef RQM_SV_ONLY

/*
=================
Mod_LoadBrushModelTexture
=================
*/
int Mod_LoadBrushModelTexture (texture_t *tx, int flags)
{
	char	mappath[MAX_QPATH], luma_name[MAX_QPATH];
	const char	*pathlist[3], *namelist[2];

// NOTE: any changes to this code should also be applied to R_InitSky

	if (no24bit || isDedicated)			// JDH: added these checks
		return 0;

	if ((loadmodel->isworldmodel && !gl_externaltextures_world.value) ||
	     (!loadmodel->isworldmodel && !gl_externaltextures_bmodels.value))
		return 0;

	namelist[0] = tx->name;
	namelist[1] = NULL;

	if (loadmodel->isworldmodel)
	{
		Q_snprintfz (mappath, sizeof(mappath), "textures/%s/", Host_MapName ());
		pathlist[0] = mappath;
	}
	else pathlist[0] = "textures/bmodels/";

	pathlist[1] = "textures/";
	pathlist[2] = NULL;

	tx->gl_texturenum = GL_LoadTextureImage_MultiSource (pathlist, namelist, tx->name, flags, 0);

	if (tx->gl_texturenum && !ISTURBTEX(tx->name))
	{
		Q_snprintfz (luma_name, sizeof(luma_name), "%s_luma", tx->name);
		namelist[0] = luma_name;
		tx->fb_texturenum = GL_LoadTextureImage_MultiSource (pathlist, namelist, va("@fb_%s", tx->name), flags | TEX_LUMA, 0);
	}


/*	if (loadmodel->isworldmodel)
	{
		Mod_TryLoadBrushTexture (va("textures/%s/", mapname), name, flags, tx);
	}
	else
	{
		Mod_TryLoadBrushTexture ("textures/bmodels/", name, flags, tx);
	}

	if (!tx->gl_texturenum)
	{
		Mod_TryLoadBrushTexture ("textures/", name, flags, tx);
	}
*/
/*****JDH*****
	if ( tx->gl_texturenum )
		Con_Printf( "Using external texture \"%s\"\n", name );
*****JDH*****/

	if (tx->fb_texturenum)
		tx->isLumaTexture = true;

	return tx->gl_texturenum;
}

#endif		//#ifndef RQM_SV_ONLY

#ifndef RQM_SV_ONLY
/*
=================
Mod_LoadTextures
=================
*/
void Mod_LoadTextures (lump_t *l)
{
	int				i, j, num, max, altmax, texture_flag, brighten_flag;
	miptex_t		*mt;
	texture_t		*tx, *tx2, *txblock, *anims[10], *altanims[10];
	dmiptexlump_t	*m;
#ifndef RQM_SV_ONLY
	byte			*data;
#endif

	if (!l->filelen)
	{
		loadmodel->textures = NULL;
		return;
	}

	m = (dmiptexlump_t *)(mod_base + l->fileofs);
	m->nummiptex = LittleLong (m->nummiptex);
	loadmodel->numtextures = m->nummiptex;
	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures), mod_loadname);

	txblock = Hunk_AllocName (m->nummiptex * sizeof(**loadmodel->textures), mod_loadname);

#ifndef RQM_SV_ONLY
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		brighten_flag = 0;
	else
#endif
//	brighten_flag = (gl_lightmode.value == 1) ? TEX_BRIGHTEN : 0;
	brighten_flag = 0;

	texture_flag = (gl_picmip_all.value || loadmodel->isworldmodel) ? TEX_MIPMAP : 0;
#else
	brighten_flag = texture_flag = 0;
#endif

	for (i = 0 ; i < m->nummiptex ; i++)
	{
		m->dataofs[i] = LittleLong (m->dataofs[i]);
		if (m->dataofs[i] == -1)
			continue;

		mt = (miptex_t *)((byte *)m + m->dataofs[i]);
		loadmodel->textures[i] = tx = txblock + i;

		memcpy (tx->name, mt->name, sizeof(tx->name));
		if (!tx->name[0])
		{
			Q_snprintfz (tx->name, sizeof(tx->name), "unnamed%d", i);
			Con_DPrintf ("Warning: unnamed texture in %s, renaming to %s\n", loadmodel->name, tx->name);
		}

		tx->width = mt->width = LittleLong (mt->width);
		tx->height = mt->height = LittleLong (mt->height);
//		if ((mt->width & 15) || (mt->height & 15))
//			Host_Error ("Mod_LoadTextures: Texture %s is not 16 aligned", mt->name);	// was Sys_Error

		for (j=0 ; j<MIPLEVELS ; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);

		// HACK HACK HACK
		if (!strcmp(mt->name, "shot1sid") && mt->width == 32 && mt->height == 32
			&& CRC_Block((byte*)(mt+1), mt->width*mt->height) == 65393)
		{	// This texture in b_shell1.bsp has some of the first 32 pixels painted white.
			// They are invisible in software, but look really ugly in GL. So we just copy
			// 32 pixels from the bottom to make it look nice.
			memcpy (mt+1, (byte *)(mt+1) + 32*31, 32);
		}

#ifndef RQM_SV_ONLY
		if (loadmodel->isworldmodel && ISSKYTEX(tx->name))
		{
			R_InitSky (mt);
			continue;
		}

		if (Mod_LoadBrushModelTexture(tx, texture_flag))
			continue;

		if (mt->offsets[0])
		{
			data = (byte *)mt + mt->offsets[0];
			tx2 = tx;
		}
		else
		{
			data = (byte *)tx2 + tx2->offsets[0];		// JDH: what is tx2 equal to at this point ???
			tx2 = r_notexture_mip;
		}

#ifdef _DEBUG
//		for (j = tx2->width*tx2->height; j >= 0; j--)
//			if (data[j] == 255)
//				break;
#endif

//		tx->gl_texturenum = GL_LoadTexture (tx2->name, tx2->width, tx2->height, data, texture_flag | brighten_flag, 1);
		tx->gl_texturenum = GL_LoadTexture (tx2->name, tx2->width, tx2->height, data, texture_flag | TEX_BMODEL, 1);
		if (!ISTURBTEX(tx->name) && Img_HasFullbrights(data, tx2->width * tx2->height))
			tx->fb_texturenum = GL_LoadTexture (va("@fb_%s", tx2->name), tx2->width, tx2->height, data, texture_flag | TEX_FULLBRIGHT, 1);
	/************** JDH ***************/
		else tx->fb_texturenum = 0;
	/************** JDH ***************/
#endif
	}

// sequence the animations
	for (i=0 ; i<m->nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;
		if (tx->anim_next)
			continue;	// already sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
		{
			Host_Error ("Bad animating texture %s", tx->name);	// was Sys_Error
		}

		for (j=i+1 ; j<m->nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp(tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
			{
				Host_Error ("Bad animating texture %s", tx->name);	// was Sys_Error
			}
		}

#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			if (!tx2)
				Host_Error ("Mod_LoadTextures: Missing frame %i of %s", j, tx->name);	// was Sys_Error
			tx2->anim_total = max * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[(j+1)%max];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
				Host_Error ("Mod_LoadTextures: Missing frame %i of %s", j, tx->name);	// was Sys_Error
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = altanims[(j+1)%altmax];
			if (max)
				tx2->alternate_anims = anims[0];
		}
	}
}

#endif

#ifndef RQM_SV_ONLY

// joe: from FuhQuake
static byte *LoadColoredLighting (const char *name, char **litfilename)
{
	byte		*data;
	const char	*tmpname;
	extern	cvar_t	gl_loadlitfiles;

	if (!gl_loadlitfiles.value)
		return NULL;

	tmpname = Host_MapName ();

	if (!COM_FilenamesEqual(name, va("maps/%s.bsp", tmpname)))
		return NULL;

	*litfilename = va("maps/%s.lit", tmpname);
	data = COM_LoadHunkFile (*litfilename, 0);

	if (!data)
	{
		*litfilename = va("lits/%s.lit", tmpname);
		data = COM_LoadHunkFile (*litfilename, 0);
	}

	return data;
}

#endif

#ifndef RQM_SV_ONLY
/*
=================
Mod_LoadLighting
=================
*/
void Mod_LoadLighting (lump_t *l)
{
	int		mark;
#ifndef RQM_SV_ONLY
	int		i;
	byte	*in, *out/*, d*/;
	byte	*data;
	char	*litfilename;
	int		lit_ver;
#endif

	loadmodel->lightdata = NULL;
	loadmodel->lightdatadepth = 1;		// JDH

//	if (!l->filelen)
//		return;

	// check for a .lit file
	mark = Hunk_LowMark ();

#ifndef RQM_SV_ONLY
	data = LoadColoredLighting (loadmodel->name, &litfilename);
	if (data)
	{
		if (com_filesize < 8 || strncmp((char *)data, "QLIT", 4))
			Con_Printf ("\x02""Corrupt .lit file (%s)...ignoring\n", COM_SkipPath(litfilename));
		else if (l->filelen * 3 + 8 != com_filesize)
			Con_Printf ("\x02""Warning: .lit file (%s) has incorrect size\n", COM_SkipPath(litfilename));
		else if ((lit_ver = LittleLong(((int *)data)[1])) != 1)
			Con_Printf ("\x02""Unknown .lit file version (v%d)\n", lit_ver);
		else
		{
			Con_DPrintf ("Static coloured lighting loaded\n");

			loadmodel->lightdata = data + 8;
			loadmodel->lightdatadepth = 3;

			in = mod_base + l->fileofs;
			out = loadmodel->lightdata;
			for (i=0 ; i<l->filelen ; i++)
			{
				int	b = max(out[3*i], max(out[3*i+1], out[3*i+2]));

				if (!b)
					out[3*i] = out[3*i+1] = out[3*i+2] = in[i];
				else
				{	// too bright
					float	r = in[i] / (float)b;

					out[3*i+0] = (int)(r * out[3*i+0]);
					out[3*i+1] = (int)(r * out[3*i+1]);
					out[3*i+2] = (int)(r * out[3*i+2]);
				}
			}
			return;
		}
		Hunk_FreeToLowMark (mark);
	}
#endif

	if (!l->filelen)
	{
		//loadmodel->lightdata = NULL;
		return;
	}


// no .lit found, expand the white lighting data to color
#if 0
	loadmodel->lightdata = Hunk_AllocName (l->filelen * 3, va("%s_@lightdata", loadmodel->name));
	loadmodel->lightdatadepth = 3;
	// place the file at the end, so it will not be overwritten until the very last write
	//in = loadmodel->lightdata + l->filelen * 2;
	in = mod_base + l->fileofs;
	out = loadmodel->lightdata;
	//memcpy (in, mod_base + l->fileofs, l->filelen);
	for (i = 0 ; i < l->filelen ; i++, out += 3)
	{
		out[0] = out[1] = out[2] = *in++;
	}
#else
	loadmodel->lightdata = Hunk_AllocName (l->filelen, "lights");
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
#endif
}

#endif

/*
=================
Mod_LoadVisibility
=================
*/
void Mod_LoadVisibility (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->visdata = NULL;
		return;
	}

	loadmodel->visdata = Hunk_AllocName (l->filelen, mod_loadname);
	memcpy (loadmodel->visdata, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadEntities
=================
*/
void Mod_LoadEntities (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->entities = NULL;
		return;
	}
	loadmodel->entities = Hunk_AllocName (l->filelen, mod_loadname);
	memcpy (loadmodel->entities, mod_base + l->fileofs, l->filelen);

#ifdef HEXEN2_SUPPORT
	entity_file_size = (hexen2 ? l->filelen : 0);
#endif
}


#ifndef RQM_SV_ONLY
/*
=================
Mod_LoadVertexes
=================
*/
void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int		i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadVertexes: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		out->position[0] = LittleFloat (in->point[0]);
		out->position[1] = LittleFloat (in->point[1]);
		out->position[2] = LittleFloat (in->point[2]);
#ifdef _DEBUG
		{
			int j;
			for (j = 0; j < 3; j++)
				if (out->position[j] > 4096 || out->position[j] < -4096)
					break;
		}
#endif
	}
}
#endif

/*****JDH*****/
#ifdef HEXEN2_SUPPORT

typedef struct
{
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[MAX_MAP_HULLS_H2];
	int			visleafs;
	int			firstface, numfaces;
} dmodelH2_t;

#endif
/*****JDH*****/

/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	mmodel_t	*out;
	int			i, j, count;

/*****JDH*****/
#ifdef HEXEN2_SUPPORT
	dmodelH2_t *inH2;

	if (hexen2)
	{
		inH2 = (void *)(mod_base + l->fileofs);
		if (l->filelen % sizeof(*inH2))
			Host_Error ("Mod_LoadSubmodels: funny lump size in %s", loadmodel->name);	// was Sys_Error

		count = l->filelen / sizeof(*inH2);
		if (count > MAX_MODELS)
			Host_Error ("Mod_LoadSubmodels: count > MAX_MODELS");	// was Sys_Error

		out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

		loadmodel->submodels = out;
		loadmodel->numsubmodels = count;

		for (i=0 ; i<count ; i++, inH2++, out++)
		{
			for (j=0 ; j<3 ; j++)
			{	// spread the mins / maxs by a pixel
				out->mins[j] = LittleFloat (inH2->mins[j]) - 1;
				out->maxs[j] = LittleFloat (inH2->maxs[j]) + 1;
				out->origin[j] = LittleFloat (inH2->origin[j]);
			}
			for (j=0 ; j<MAX_MAP_HULLS_H2 ; j++)
				out->headnode[j] = LittleLong (inH2->headnode[j]);
			out->visleafs = LittleLong (inH2->visleafs);
			out->firstface = LittleLong (inH2->firstface);
			out->numfaces = LittleLong (inH2->numfaces);
		}

		return;
	}
#endif
/*****JDH*****/

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadSubmodels: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	if (count > MAX_MODELS)
		Host_Error ("Mod_LoadSubmodels: count > MAX_MODELS");	// was Sys_Error

	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		for (j=0 ; j<MAX_MAP_HULLS ; j++)
			out->headnode[j] = LittleLong (in->headnode[j]);
		out->visleafs = LittleLong (in->visleafs);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}
}

#ifndef RQM_SV_ONLY
/*
=================
Mod_LoadEdges
=================
*/
void Mod_LoadEdges (lump_t *l)
{
	dedge_t	*in;
	medge_t	*out;
	int 	i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadEdges: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ((count + 1) * sizeof(*out), mod_loadname);

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort (in->v[0]);
		out->v[1] = (unsigned short)LittleShort (in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t	*in;
	mtexinfo_t	*out;
	int 		i, j, count, miptex;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadTexinfo: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<4 ; j++) {
			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);
			out->vecs[1][j] = LittleFloat (in->vecs[1][j]);
		}

		miptex = LittleLong (in->miptex);
		out->flags = LittleLong (in->flags);

		if (!loadmodel->textures)
		{
			out->texture = r_notexture_mip;	// checkerboard texture
			out->flags = 0;
		}
		else
		{
			if (miptex >= loadmodel->numtextures)
				Host_Error ("Mod_LoadTexinfo: miptex >= loadmodel->numtextures");	// was Sys_Error
			out->texture = loadmodel->textures[miptex];
			if (!out->texture)
			{
				out->texture = r_notexture_mip;	// texture not found
				out->flags = 0;
			}
		}
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void CalcSurfaceExtents (model_t *mod, msurface_t *s)
{
	float		mins[2], maxs[2], val;
	int			i, j, e, bmins[2], bmaxs[2];
	mvertex_t	*v;
	mtexinfo_t	*tex;

	mins[0] = mins[1] =  999999;
	maxs[0] = maxs[1] = -999999;

//#ifdef FITZWORLD
	s->mins[0] = s->mins[1] = s->mins[2] = (int)0x7FFFFFFF;
	s->maxs[0] = s->maxs[1] = s->maxs[2] = (int)0x80000000;
//#endif

	tex = s->texinfo;

	for (i=0 ; i<s->numedges ; i++)
	{
		e = mod->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &mod->vertexes[mod->edges[e].v[0]];
		else
			v = &mod->vertexes[mod->edges[-e].v[1]];

		for (j=0 ; j<2 ; j++)
		{
//#ifdef FITZWORLD
			if (v->position[j] < s->mins[j])
				s->mins[j] = v->position[j];
			if (v->position[j] > s->maxs[j])
				s->maxs[j] = v->position[j];
//#endif
			// the following comment and definition of val are from
			// http://sourceforge.net/p/quakespasm/code/908/
			/* The following calculation is sensitive to floating-point
			 * precision.  It needs to produce the same result that the
			 * light compiler does, because R_BuildLightMap uses surf->
			 * extents to know the width/height of a surface's lightmap,
			 * and incorrect rounding here manifests itself as patches
			 * of "corrupted" looking lightmaps.
			 * Most light compilers are win32 executables, so they use
			 * x87 floating point.  This means the multiplies and adds
			 * are done at 80-bit precision, and the result is rounded
			 * down to 32-bits and stored in val.
			 * Adding the casts to double seems to be good enough to fix
			 * lighting glitches when Quakespasm is compiled as x86_64
			 * and using SSE2 floating-point.  A potential trouble spot
			 * is the hallway at the beginning of mfxsp17.  -- ericw
			 */
			val =	((double)v->position[0] * (double)tex->vecs[j][0]) +
				((double)v->position[1] * (double)tex->vecs[j][1]) +
				((double)v->position[2] * (double)tex->vecs[j][2]) +
				(double)tex->vecs[j][3];
				
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}

//#ifdef FITZWORLD
		if (v->position[2] < s->mins[2])
			s->mins[2] = v->position[2];
		if (v->position[2] > s->maxs[2])
			s->maxs[2] = v->position[2];
//#endif
	}

	for (i=0 ; i<2 ; i++)
	{
		bmins[i] = floor (mins[i] / 16);
		bmaxs[i] = ceil (maxs[i] / 16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;

	// JDH: this check doesn't serve any purpose
	//	if (!(tex->flags & TEX_SPECIAL) && s->extents[i] > 512)
	//		Host_Error ("CalcSurfaceExtents: Bad surface extents");
	}

	if (!mod_oversized)
	{
		for (i = 0; i < 3; i++)
		{
			if ((s->mins[i] < -4096) || (s->maxs[i] > 4096))
				mod_oversized = true;
		}
	}
}

#ifdef BSP23TEST
typedef struct
{
	short		planenum;
	short		side;
	int			unk;
	int			firstedge;		// we must support > 64k edges
	short		numedges;
	short		texinfo;

// lighting info
	byte		styles[MAXLIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
} dface23_t;
#endif

/*
=================
Mod_LoadFaces
=================
*/
void Mod_LoadFaces (lump_t *l)
{
	dface_t		*in;
	msurface_t 	*out;
	int		i, count, surfnum, planenum, side;

#ifdef BSP23TEST
	int size = (mod_bspversion == 23) ? 24 : sizeof(dface_t);

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % size)
		Host_Error ("Mod_LoadFaces: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / size;
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for (surfnum=0 ; surfnum<count ; surfnum++, out++)
	{
		out->flags = 0;

		planenum = LittleShort(in->planenum);
		if ((side = LittleShort(in->side)))
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;

		if (mod_bspversion == 23)
		{
			out->firstedge = LittleLong(((dface23_t *)in)->firstedge);
			out->numedges = LittleShort(((dface23_t *)in)->numedges);
			out->texinfo = NULL;			/******* FIXME *******/
		// lighting info
			for (i=0 ; i<MAXLIGHTMAPS ; i++)
				out->styles[i] = ((dface23_t *)in)->styles[i];
			i = LittleLong(((dface23_t *)in)->lightofs);
		}
		else
		{
			out->firstedge = LittleLong(in->firstedge);
			out->numedges = LittleShort(in->numedges);
			out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

			CalcSurfaceExtents (loadmodel, out);

		// lighting info
			for (i=0 ; i<MAXLIGHTMAPS ; i++)
				out->styles[i] = in->styles[i];
			i = LittleLong(in->lightofs);
		}
#else
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadFaces: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for (surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);
		out->flags = 0;

//		out->visframe = 0;		/************ JDH ************/

		planenum = LittleShort(in->planenum);
		if ((side = LittleShort(in->side)))
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;
		out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

		CalcSurfaceExtents (loadmodel, out);

	// lighting info
		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
#endif

		//out->samples = (i == -1) ? NULL : loadmodel->lightdata + i * 3;
		out->samples = (i == -1) ? NULL : loadmodel->lightdata + i * loadmodel->lightdatadepth;

	// set the drawing flags flag
		if (ISSKYTEX(out->texinfo->texture->name))	// sky
		{
			out->flags |= (SURF_DRAWSKY | SURF_DRAWTILED);
			GL_SubdivideSurface (loadmodel, out);	// cut up polygon for warps
		}
		else if (ISTURBTEX(out->texinfo->texture->name))	// turbulent
		{
			out->flags |= (SURF_DRAWTURB | SURF_DRAWTILED);
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			GL_SubdivideSurface (loadmodel, out);	// cut up polygon for warps

		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				if ((!Q_strncasecmp(out->texinfo->texture->name, "*rtex078",8)) ||
					(!Q_strncasecmp(out->texinfo->texture->name, "*lowlight",9)))
					out->flags |= SURF_TRANSLUCENT;
			}
		#endif
		}

#ifdef BSP23TEST
		in = (dface_t *) ((byte *) in + size);
#endif
	}
}
#endif		//#ifndef RQM_SV_ONLY


#ifdef BSP23TEST
typedef struct
{
	int				planenum;
	short			children[2];	// negative numbers are -(leafs+1), not nodes
	float			mins[3];		// for sphere culling
	float			maxs[3];
	unsigned short	firstface;
	unsigned short	numfaces;	// counting both sides
} dnode23_t;

/*
=================
Mod_LoadNodes23
=================
*/
void Mod_LoadNodes23 (lump_t *l)
{
	int			i, j, count, p;
	dnode23_t	*in;
	mnode_t 	*out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadNodes23: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleFloat (in->mins[j]);
			out->minmaxs[3+j] = LittleFloat (in->maxs[j]);
		}

		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = (unsigned short) LittleShort (in->firstface);
		out->numsurfaces = (unsigned short) LittleShort (in->numfaces);

//		out->visframe = 0;	/****** JDH ******/

		for (j=0 ; j<2 ; j++)
		{
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}

	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

#endif			// #ifdef BSP23TEST


/*
=================
Mod_LoadNodes
=================
*/
void Mod_LoadNodes (lump_t *l)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

#ifdef BSP23TEST
	if (mod_bspversion == 23)
	{
		Mod_LoadNodes23 (l);
		return;
	}
#endif

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadNodes: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			// JDH: since these are stored as shorts, they may not be correct
			//      if map exceeds normal Quake coordinate space (spcscr.bsp)
			if (mod_oversized)
			{
				out->minmaxs[j] = (int)0x7FFFFFFF;
				out->minmaxs[3+j] = (int)0x80000000;
			}
			else
			{
				out->minmaxs[j] = LittleShort (in->mins[j]);
				out->minmaxs[3+j] = LittleShort (in->maxs[j]);
			}
		}

		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = (unsigned short) LittleShort (in->firstface);
		out->numsurfaces = (unsigned short) LittleShort (in->numfaces);

//		out->visframe = 0;	/****** JDH ******/

#ifndef RQM_SV_ONLY
		if (mod_oversized)
		{
			msurface_t	*surf;

			for (j = 0, surf = loadmodel->surfaces + out->firstsurface; j < out->numsurfaces; j++, surf++)
			{
				for (p = 0; p < 3; p++)
				{
					if (surf->mins[p] < out->minmaxs[p])
						out->minmaxs[p] = surf->mins[p];

					if (surf->maxs[p] > out->minmaxs[3+p])
						out->minmaxs[3+p] = surf->maxs[p];
				}
			}
		}
#endif

		for (j=0 ; j<2 ; j++)
		{
#if 0
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
#else	
		/* 2011/05/15: fix for oms2_2 */
			p = (unsigned short) LittleShort (in->children[j]);
			if (p < count)
				out->children[j] = loadmodel->nodes + p;
			else
			{
				// JDH: hack for >32767 nodes (thanks DP/Fitz!)
				p = 65535 - p;
				if (p < loadmodel->numleafs)
					out->children[j] = (mnode_t *)(loadmodel->leafs + p);
				else
				{
					Con_Printf("Mod_LoadNodes: invalid leaf index %i (file has only %i leafs)\n", p, loadmodel->numleafs);
					out->children[j] = (mnode_t *)(loadmodel->leafs); //map it to the solid leaf
				}
			}
#endif
		}
	}

	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in;
	mleaf_t 	*out;
	int			i, j, count, p;
	msurface_t	*surf;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadLeafs: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for (i=0 ; i<count ; i++, in++, out++)
	{
		if (mod_oversized && out->nummarksurfaces)
		{
			for (j=0 ; j<3 ; j++)
			{
				out->minmaxs[j] = (int)0x7FFFFFFF;
				out->minmaxs[3+j] = (int)0x80000000;
			}
		}
		else
		{
			for (j=0 ; j<3 ; j++)
			{
				out->minmaxs[j] = LittleShort (in->mins[j]);
				out->minmaxs[3+j] = LittleShort (in->maxs[j]);
			}
		}

		p = LittleLong(in->contents);
		out->contents = p;

#ifdef RQM_SV_ONLY
		out->firstmarksurface = NULL;
		out->nummarksurfaces = 0;
#else
		out->firstmarksurface = loadmodel->marksurfaces + LittleShort (in->firstmarksurface);
		out->nummarksurfaces = LittleShort(in->nummarksurfaces);
#endif
		p = LittleLong(in->visofs);
		out->compressed_vis = (p == -1) ? NULL : loadmodel->visdata + p;

		if (mod_oversized)
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
			{
				surf = out->firstmarksurface[j];
				for (p = 0; p < 3; p++)
				{
					if (surf->mins[p] < out->minmaxs[p])
						out->minmaxs[p] = surf->mins[p];

					if (surf->maxs[p] > out->minmaxs[3+p])
						out->minmaxs[3+p] = surf->maxs[p];
				}
			}
		}

	/**********JDH************/
		out->uncompressed_vis = NULL;
//		out->visframe = 0;

		/*if ( ( p == -1 ) || !loadmodel->isworldmodel )
			out->visdata = NULL;
		else
			out->visdata = (byte *) ( 0x8000000 | (DWORD) (loadmodel->visdata + p) );*/
	/**********JDH************/

		out->efrags = NULL;

		for (j=0 ; j<4 ; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];

		if (out->contents != CONTENTS_EMPTY)
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
		}
	}
}

/*
=================
Mod_LoadClipnodes
=================
*/
void Mod_LoadClipnodes (lump_t *l)
{
	dclipnode_t	*in;
	mclipnode_t *out;
	int			i, count;
	hull_t		*hull;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadClipnodes: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->clipnodes = out;
	loadmodel->numclipnodes = count;

// player
	hull = &loadmodel->hulls[1];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	VectorCopy (vec3_hull_min, hull->clip_mins);
	VectorCopy (vec3_hull_max, hull->clip_maxs);
/*	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 32;
*/
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
	//scorpion
		hull = &loadmodel->hulls[2];
		hull->clipnodes = out;
		hull->firstclipnode = 0;
		hull->lastclipnode = count-1;
		hull->planes = loadmodel->planes;
		hull->clip_mins[0] = -24;
		hull->clip_mins[1] = -24;
		hull->clip_mins[2] = -20;
		hull->clip_maxs[0] = 24;
		hull->clip_maxs[1] = 24;
		hull->clip_maxs[2] = 20;

	//crouch
		hull = &loadmodel->hulls[3];
		hull->clipnodes = out;
		hull->firstclipnode = 0;
		hull->lastclipnode = count-1;
		hull->planes = loadmodel->planes;
		hull->clip_mins[0] = -16;
		hull->clip_mins[1] = -16;
		hull->clip_mins[2] = -12;
		hull->clip_maxs[0] = 16;
		hull->clip_maxs[1] = 16;
		hull->clip_maxs[2] = 16;

	//pentacles (was hydra (-40,40) before MP)
		hull = &loadmodel->hulls[4];
		hull->clipnodes = out;
		hull->firstclipnode = 0;
		hull->lastclipnode = count-1;
		hull->planes = loadmodel->planes;
		hull->clip_mins[0] = -8;
		hull->clip_mins[1] = -8;
		hull->clip_mins[2] = -8;
		hull->clip_maxs[0] = 8;
		hull->clip_maxs[1] = 8;
		hull->clip_maxs[2] = 8;

	//golem
		hull = &loadmodel->hulls[5];
		hull->clipnodes = out;
		hull->firstclipnode = 0;
		hull->lastclipnode = count-1;
		hull->planes = loadmodel->planes;
		hull->clip_mins[0] = -48;
		hull->clip_mins[1] = -48;
		hull->clip_mins[2] = -50;
		hull->clip_maxs[0] = 48;
		hull->clip_maxs[1] = 48;
		hull->clip_maxs[2] = 50;
	}
	else
#endif	// #ifdef HEXEN2_SUPPORT
	{
	// shambler
		hull = &loadmodel->hulls[2];
		hull->clipnodes = out;
		hull->firstclipnode = 0;
		hull->lastclipnode = count-1;
		hull->planes = loadmodel->planes;
		hull->clip_mins[0] = -32;
		hull->clip_mins[1] = -32;
		hull->clip_mins[2] = -24;
		hull->clip_maxs[0] = 32;
		hull->clip_maxs[1] = 32;
		hull->clip_maxs[2] = 64;
	}

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		//out->children[0] = LittleShort(in->children[0]);
		//out->children[1] = LittleShort(in->children[1]);

		// JDH: treat these as unsigned to allow > 32767 clipnodes
		out->children[0] = (unsigned short) LittleShort(in->children[0]);
		out->children[1] = (unsigned short) LittleShort(in->children[1]);

		// JDH: however, I leave -1 to -15 for CONTENTS_xxxx constants
		//     (although only -1 and -2 seem to be used for clipnodes?)
		if (out->children[0] > 0xFFF0)
			out->children[0] -= 0x10000;
		if (out->children[1] > 0xFFF0)
			out->children[1] -= 0x10000;
	}
}


/*
=================
Mod_MakeHull0

Deplicate the drawing hull structure as a clipping hull
=================
*/
void Mod_MakeHull0 (void)
{
	mnode_t		*in, *child;
	mclipnode_t	*out;
	int			i, j, count;
	hull_t		*hull;

	hull = &loadmodel->hulls[0];

	in = loadmodel->nodes;
	count = loadmodel->numnodes;
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = in->plane - loadmodel->planes;
		for (j=0 ; j<2 ; j++)
		{
			child = in->children[j];
			if (child->contents < 0)
				out->children[j] = child->contents;
			else
				out->children[j] = child - loadmodel->nodes;
		}
	}
}

#ifndef RQM_SV_ONLY
/*
=================
Mod_LoadMarksurfaces
=================
*/
void Mod_LoadMarksurfaces (lump_t *l)
{
	int				i, j, count;
	unsigned short	*in;		// JDH: was signed short
	msurface_t		**out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadMarksurfaces: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for (i=0 ; i<count ; i++)
	{
		j = (unsigned short) LittleShort(in[i]);
		if (j >= loadmodel->numsurfaces)
			Host_Error ("Mod_LoadMarksurfaces: bad surface number");	// was Sys_Error
		out[i] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Mod_LoadSurfedges (lump_t *l)
{
	int	i, count, *in, *out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadSurfedges: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * sizeof(*out), mod_loadname);

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for (i=0 ; i<count ; i++)
	{
		out[i] = LittleLong (in[i]);

//		assert (((out[i] < MAX_MAP_EDGES) && (out[i] > -MAX_MAP_EDGES)));
	}
}

#endif

/*
=================
Mod_LoadPlanes
=================
*/
void Mod_LoadPlanes (lump_t *l)
{
	int		i, j, count, bits;
	mplane_t	*out;
	dplane_t 	*in;

#ifdef BSP23TEST
	int size = (mod_bspversion == 23) ? 28 : sizeof(dplane_t);

	in = (void *)(mod_base + l->fileofs);

	if (l->filelen % size)
		Host_Error ("Mod_LoadPlanes: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / size;
	out = Hunk_AllocName (count * 2 * sizeof(*out), mod_loadname);

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for (i=0 ; i<count ; i++)
#else
	in = (void *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Host_Error ("Mod_LoadPlanes: funny lump size in %s", loadmodel->name);	// was Sys_Error

	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName (count * 2 * sizeof(*out), mod_loadname);

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for (i=0 ; i<count ; i++, in++, out++)
#endif
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;

		assert ((out->type <= 5));

#ifdef BSP23TEST
		in = (dplane_t *)((byte *) in + size);
		out++;
#endif
	}
}

void Mod_RecalcNodeBounds (void)
{
	mleaf_t *leaf;
	int		i, p;
	mnode_t	*node, *parent;

	leaf = loadmodel->leafs;

	for (i = 0; i < loadmodel->numleafs; i++, leaf++)
	{
		node = (mnode_t *) leaf;
		parent = node->parent;

		while (parent)
		{
			for (p = 0; p < 3; p++)
			{
				if (node->minmaxs[p] < parent->minmaxs[p])
					parent->minmaxs[p] = node->minmaxs[p];

				if (node->minmaxs[3+p] > parent->minmaxs[3+p])
					parent->minmaxs[3+p] = node->minmaxs[3+p];
			}
			node = parent;
			parent = parent->parent;
		}
	}
}

/*
=================
Mod_LoadBrushModel
=================
*/
void Mod_LoadBrushModel (model_t *mod, void *buffer)
{
	int			i, j, maxhulls;
	dheader_t	*header;
	mmodel_t 	*bm;
	extern cvar_t host_mapname;

	loadmodel->type = mod_brush;

	header = (dheader_t *)buffer;

#ifdef BSP23TEST
	mod_bspversion =
#endif
		i = LittleLong (header->version);

	if ((i != BSPVERSION) && (i != BSPVERSION-1))		// JDH: bsps from v0.8 beta work fine too
//		Sys_Error ("Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, i, BSPVERSION);
#ifdef BSP23TEST
	if (i != 23)
#endif
	{
		Con_Printf ("Mod_LoadBrushModel: %s has wrong version number %i (should be %i)\n", mod->name, i, BSPVERSION);
		mod->numsubmodels = -1;	// HACK - incorrect BSP version is no longer fatal
		return;
	}

	loadmodel->isworldmodel = COM_FilenamesEqual (loadmodel->name, va("maps/%s.bsp", host_mapname.string));
	if (loadmodel->isworldmodel)
		mod_oversized = false;

// swap all the lumps
	mod_base = (byte *)header;

#ifdef BSP23TEST
	if (mod_bspversion == 23)
	{
		for (i=0 ; i<sizeof(dheader_t)/4 - 8 ; i++)
			((int *)header)[i] = LittleLong (((int *)header)[i]);

		Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
#ifndef RQM_SV_ONLY
		Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
		Mod_LoadEdges (&header->lumps[LUMP_EDGES-1]);
		Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES-1]);
		Mod_LoadTextures (&header->lumps[LUMP_TEXTURES]);
		Mod_LoadLighting (&header->lumps[LUMP_LIGHTING-1]);
	//	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
		Mod_LoadFaces (&header->lumps[LUMP_FACES-1]);
		Mod_LoadMarksurfaces (&header->lumps[LUMP_MARKSURFACES-1]);
#endif
		Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
		Mod_LoadLeafs (&header->lumps[LUMP_LEAFS-1]);
		Mod_LoadNodes (&header->lumps[LUMP_NODES]);
		Mod_LoadClipnodes (&header->lumps[LUMP_CLIPNODES-1]);
		Mod_LoadEntities (&header->lumps[LUMP_ENTITIES]);
		Mod_LoadSubmodels (&header->lumps[LUMP_MODELS-1]);
	}
	else
#endif
	{
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong (((int *)header)[i]);

// load into heap
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
#ifndef RQM_SV_ONLY
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadTextures (&header->lumps[LUMP_TEXTURES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_MARKSURFACES]);
#endif
	Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadClipnodes (&header->lumps[LUMP_CLIPNODES]);
	Mod_LoadEntities (&header->lumps[LUMP_ENTITIES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);
	}

	Mod_MakeHull0 ();

	if (loadmodel->isworldmodel && mod_oversized)
		Mod_RecalcNodeBounds ();

#ifndef RQM_SV_ONLY
	mod->numframes = 2;		// regular and alternate animation
#endif

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		maxhulls = MAX_MAP_HULLS_H2;
	else
#endif
		maxhulls = MAX_MAP_HULLS;

	// set up the submodels (FIXME: this is confusing)
	for (i = 0 ; i < mod->numsubmodels ; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->headnode[0];
		for (j=1 ; j<maxhulls ; j++)
		{
			mod->hulls[j].firstclipnode = bm->headnode[j];
			mod->hulls[j].lastclipnode = mod->numclipnodes - 1;
		}

		mod->firstmodelsurface = bm->firstface;
		mod->nummodelsurfaces = bm->numfaces;

		VectorCopy (bm->maxs, mod->maxs);
		VectorCopy (bm->mins, mod->mins);

#ifndef RQM_SV_ONLY
		mod->radius = RadiusFromBounds (mod->mins, mod->maxs);
#endif

		mod->numleafs = bm->visleafs;

		if (i < mod->numsubmodels - 1)
		{	// duplicate the basic information
			char	name[10];

			sprintf (name, "*%i", i+1);
			loadmodel = Mod_FindName (name);
			*loadmodel = *mod;
			Q_strcpy (loadmodel->name, name, sizeof(loadmodel->name));
			mod = loadmodel;
		}
	}

/**********JDH************/
	// can't do this in Mod_LoadLeafs, since the "loadmodel" at that time
	// is not what becomes the worldmodel
	/*if ( origmod->isworldmodel )
	{
		Mod_DecompressVis( mod );
	}*/
/**********JDH************/
}



