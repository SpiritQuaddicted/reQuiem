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
// r_surf.c: surface-related refresh code

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#define LIGHTMAPFIX			/* JDH: reduce lm uploads when mtexing */

#define	BLOCK_WIDTH			128
#define	BLOCK_HEIGHT		128

#define MAX_LIGHTMAP_SIZE	4096

// JDH: increased from 64
#define	MAX_LIGHTMAPS		256

#define MAX_DETAIL_DIST		255

float	wateralpha;

static	int	lightmap_textures;


typedef struct glRect_s
{
	unsigned char	l, t, w, h;
} glRect_t;

// the lightmap texture data needs to be kept in
// main memory so texsubimage can update properly

// JDH: combined several lightmap arrays into an array of structs:
typedef struct lightmap_s
{
	qboolean	modified;
	glpoly_t	*polys;
	glRect_t	rectchange;		// bounds of area that needs to be re-uploaded
	int			depth;			// JDH: either 1 (8-bit) or 3 (24-bit)
	int			allocated[BLOCK_WIDTH];
	byte		data[3*BLOCK_WIDTH*BLOCK_HEIGHT];		// large enough to hold 24-bit data
	byte		*data24;		// allocated if 8-bit data needs to be combined w/ colored light
} lightmap_t;

static lightmap_t lightmaps[MAX_LIGHTMAPS];

/************JDH************/

msurface_t  	*skychain = NULL;
msurface_t  	*waterchain = NULL;
//msurface_t	*alphachain = NULL;
//msurface_t	**skychain_tail = &skychain;
msurface_t	**waterchain_tail = &waterchain;
//msurface_t	**alphachain_tail = &alphachain;

#define CHAIN_SURF_F2B(surf, chain_tail)		\
	{						\
		*(chain_tail) = (surf);			\
		(chain_tail) = &(surf)->texturechain;	\
		(surf)->texturechain = NULL;		\
	}

#define CHAIN_SURF_B2F(surf, chain) 			\
	{						\
		(surf)->texturechain = (chain);		\
		(chain) = (surf);			\
	}

#define MAX_TEXNUM (MAX_GLTEXTURES+MAX_LIGHTMAPS+128)
		// JDH: added 128 to cover textures not allocated from main pool (sky, playerskins, etc.)

glpoly_t	*fullbright_polys[MAX_TEXNUM];
glpoly_t	*luma_polys[MAX_TEXNUM];
qboolean	drawfullbrights = false, drawlumas = false, render_lightmaps = false;
glpoly_t	*caustics_polys = NULL;
glpoly_t	*detail_polys = NULL;

#ifdef _DEBUG
int lm_uploadcount = 0, lm_uploadsize = 0;		// JDH: for tracking lightmaps uploaded to video card
#endif

/****************JDH****************/
#ifdef FITZWORLD
extern cvar_t r_fastworld;
extern qboolean vis_changed;
#endif

extern cvar_t r_oldsky;
extern cvar_t gl_zfightfix;
//extern qboolean	r_skyboxloaded;
extern qboolean	r_drawskylast;

/****************JDH****************/

/*
================
DrawGLPoly
================
*/
void DrawGLPoly (glpoly_t *p)
{
	int		i;
	float	*v;

	glBegin (GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[3], v[4]);
		glVertex3fv (v);
	}
	glEnd ();
}

void R_RenderFullbrights (void)
{
	int		i;
	glpoly_t	*p;

	if (!drawfullbrights)
		return;

	glDepthMask (GL_FALSE);	// don't bother writing Z
	glEnable (GL_ALPHA_TEST);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	for (i=1 ; i<MAX_TEXNUM ; i++)
	{
		if (!fullbright_polys[i])
			continue;
		GL_Bind (i);
		for (p = fullbright_polys[i] ; p ; p = p->fb_chain)
			DrawGLPoly (p);
		fullbright_polys[i] = NULL;
	}

	glDisable (GL_ALPHA_TEST);
	glDepthMask (GL_TRUE);

	drawfullbrights = false;
}

void R_RenderLumas (void)
{
	int		i;
	glpoly_t	*p;

	if (!drawlumas)
		return;

	glDepthMask (GL_FALSE);	// don't bother writing Z
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for (i=1 ; i<MAX_TEXNUM ; i++)
	{
		if (!luma_polys[i])
			continue;
		GL_Bind (i);
		for (p = luma_polys[i] ; p ; p = p->luma_chain)
			DrawGLPoly (p);
		luma_polys[i] = NULL;
	}

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (GL_TRUE);

	drawlumas = false;
}

void EmitDetailPolys (void)
{
	glpoly_t	*p;
	int			i;
//	int			level;
	float		*v;

	if (!detail_polys)
		return;

	GL_Bind (detailtexture[0]);
//	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

//	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	glEnable (GL_BLEND);

	for (p = detail_polys ; p ; p = p->detail_chain)
	{
//		level = NUM_DETAIL_LEVELS * ((float)p->dist / (MAX_DETAIL_DIST+1));
//		GL_Bind (detailtexture[level]);

		glBegin (GL_POLYGON);
		v = p->verts[0];
		for (i=0 ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			glTexCoord2f (v[7]*13, v[8]*13);	// 13 was 18
			glVertex3fv (v);
		}
		glEnd();
	}

//	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_BLEND);

	detail_polys = NULL;
}

//=============================================================
// Dynamic lights

typedef struct dlightinfo_s
{
	int	local[2];
	int	rad;
	int	minlight;	// rad - minlight
//	int	type;
	float color[3];
} dlightinfo_t;

static dlightinfo_t dlightlist[MAX_DLIGHTS];
static int	numdlights;

/*
===============
R_BuildDlightList
===============
*/
void R_BuildDlightList (msurface_t *surf)
{
	int				lnum, i, smax, tmax;
	float			dist;
	vec3_t			impact;
	mtexinfo_t		*tex;
	int				irad, iminlight, local[2], tdmin, sdmin, distmin;
	dlightinfo_t	*light;

	numdlights = 0;

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;
	tex = surf->texinfo;

	for (lnum = 0 ; lnum < MAX_DLIGHTS ; lnum++)
	{
		if (!(surf->dlightbits & (((QINT64) 1) << lnum)))
			continue;		// not lit by this light

		dist = PlaneDiff(cl_dlights[lnum].origin, surf->plane);
		irad = (cl_dlights[lnum].radius - fabs(dist)) * 256;
		iminlight = cl_dlights[lnum].minlight * 256;
		if (irad < iminlight)
			continue;

		iminlight = irad - iminlight;

		for (i=0 ; i<3 ; i++)
			impact[i] = cl_dlights[lnum].origin[i] - surf->plane->normal[i]*dist;

		local[0] = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		local[1] = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];

		// check if this dlight will touch the surface
		if (local[1] > 0)
		{
			tdmin = local[1] - (tmax << 4);
			if (tdmin < 0)
				tdmin = 0;
		}
		else
		{
			tdmin = -local[1];
		}

		if (local[0] > 0)
		{
			sdmin = local[0] - (smax<<4);
			if (sdmin < 0)
				sdmin = 0;
		}
		else
		{
			sdmin = -local[0];
		}

		distmin = (sdmin > tdmin) ? (sdmin << 8) + (tdmin << 7) : (tdmin << 8) + (sdmin << 7);

		if (distmin < iminlight)
		{
			// save dlight info
			light = &dlightlist[numdlights];
			light->minlight = iminlight;
			light->rad = irad;
			light->local[0] = local[0];
			light->local[1] = local[1];
//			light->type = cl_dlights[lnum].type;
			VectorCopy (cl_dlights[lnum].color, light->color);
			numdlights++;
		}
	}
}

/*int dlightcolor[NUM_DLIGHTTYPES][3] =
{
	{100, 90, 80},		// dimlight or brightlight
	{100, 50, 10},		// muzzleflash
	{100, 50, 10},		// explosion
	{90, 60, 7},		// rocket
	{128, 0, 0},		// red
	{0, 0, 128},		// blue
	{128, 0, 128}		// red + blue
};
*/
/*
===============
R_AddDynamicLights

NOTE: R_BuildDlightList must be called first!
===============
*/
//void R_AddDynamicLights (msurface_t *surf, unsigned *blocklights)
void R_AddDynamicLights (msurface_t *surf, unsigned *blocklights, int depth)
{
	int		i, j, smax, tmax, s, t, sd, td, _sd, _td;
	int		irad, idist, iminlight, color[3], tmp;
	dlightinfo_t	*light;
	unsigned	*dest;

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;

	for (i = 0, light = dlightlist ; i < numdlights ; i++, light++)
	{
		for (j=0 ; j<3 ; j++)
		{
/*			if (light->type == lt_explosion2 || light->type == lt_explosion3)
				color[j] = (int)(ExploColor[j] * 255);
			else
//				color[j] = dlightcolor[light->type][j];
				color[j] = bubblecolor[light->type][j] * 255;
*/
				color[j] = light->color[j] * 255;
		}

		if (depth == 1)
		{
			color[0] = (color[0] + color[1] + color[2]) / 3.0;
		}

		irad = light->rad;
		iminlight = light->minlight;

		_td = light->local[1];
		dest = blocklights;
		for (t=0 ; t<tmax ; t++)
		{
			td = _td;
			if (td < 0)
				td = -td;
			_td -= 16;
			_sd = light->local[0];

			for (s=0 ; s<smax ; s++)
			{
				sd = _sd < 0 ? -_sd : _sd;
				_sd -= 16;
				idist = (sd > td) ? (sd << 8) + (td << 7) : (td << 8) + (sd << 7);
				if (idist < iminlight)
				{
					tmp = (irad - idist) >> 7;
					//dest[0] += tmp * color[0];
					//dest[1] += tmp * color[1];
					//dest[2] += tmp * color[2];
					for (j = 0; j < depth; j++)
						dest[j] += tmp * color[j];
				}
				//dest += 3;
				dest += depth;
			}
		}
	}
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights
===============
*/
//static void R_BuildLightMap (msurface_t *surf, byte *dest, int stride)
static void R_BuildLightMap (model_t *mod, msurface_t *surf, lightmap_t *lm)
{
	int			smax, tmax, depth_src, depth_dst, t, i, j, maps, blocksize, stride, lightmode;
	byte		*lightmap, *dest;
	unsigned	scale, *bl;
	static unsigned	blocklights[MAX_LIGHTMAP_SIZE*3];		// JDH: this used to be global

	surf->cached_dlight = !!numdlights;

	if (lm->data24)
	{
		depth_dst = 3;
		dest = lm->data24;
	}
	else
	{
		depth_dst = lm->depth;
		dest = lm->data;
	}

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;
	blocksize = smax * tmax;

	// set to full bright if no light data
	if (r_fullbright.value || !cl.worldmodel->lightdata)
	{
		for (i=0 ; i<blocksize*depth_dst ; i++)
			blocklights[i] = 255 << 8;
		goto store;
	}

	lightmap = surf->samples;
//	depth_src = lm->depth;		// format of model's lightdata
	depth_src = mod->lightdatadepth;		// format of model's lightdata

// JDH: since dynamic lights are colored, we need to expand 8-bit lightmap to 24
	if (numdlights && (depth_dst == 1))
	{
		lm->data24 = malloc (BLOCK_WIDTH*BLOCK_HEIGHT*3);
		if (lm->data24)
		{
		// convert the current texture to 24-bit
			dest = lm->data24;
			for (i = 0; i < BLOCK_WIDTH*BLOCK_HEIGHT; i++)
			{
				dest[0] = dest[1] = dest[2] = lm->data[i];
				dest += 3;
			}

			depth_dst = 3;
			dest = lm->data24;
		}
	}

	// clear to no light
	memset (blocklights, 0, blocksize * depth_dst * sizeof(int));

	// add all the lightmaps
	if (lightmap)
	{
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++)
		{
			scale = d_lightstylevalue[surf->styles[maps]];
			surf->cached_light[maps] = scale;	// 8.8 fraction
			bl = blocklights;
			if (depth_src == depth_dst)
			{
				for (i=0 ; i<blocksize*depth_src ; i++)
					*bl++ += lightmap[i] * scale;
			}
			else if (depth_src == 1)		// src is 8-bit, dst is 24-bit
			{
				for (i=0 ; i<blocksize ; i++)
				{
					*bl++ += lightmap[i] * scale;
					*bl++ += lightmap[i] * scale;
					*bl++ += lightmap[i] * scale;
				}
			}
			else		// src is 24-bit, dst is 8-bit
			{
				for (i=0 ; i<blocksize*3 ; i+=3)
				{
					*bl++ += lightmap[i] * scale;		// we know that all 3 color channels (rgb) are equal;
														//  this was determined in GL_CreateSurfaceLightmap
				}
			}
			lightmap += blocksize*depth_src;	// skip to next lightmap
		}
	}

	// add all the dynamic lights
	if (numdlights)
		//R_AddDynamicLights (surf, blocklights);
		R_AddDynamicLights (surf, blocklights, depth_dst);

// bound, invert, and shift
store:
	stride = (BLOCK_WIDTH - smax) * depth_dst;
	dest += (surf->light_t * BLOCK_WIDTH + surf->light_s) * depth_dst;

	lightmode = (int)gl_lightmode.value;	
	bl = blocklights;
	for (i = 0 ; i < tmax ; i++, dest += stride)
	{
//		for (j = smax*3 ; j ; j--)
		for (j = smax*depth_dst ; j ; j--)
		{
			t = *bl++;
			if (lightmode >= 3)			// Fitz overbright
			{
				t >>= 8;
				*dest++ = min(t, 255);		// (not inverted)
			}
			else 
			{
				if (lightmode == 2)
					t >>= 8;					// high-contrast
				else if (lightmode == 1)
					t = (t >> 8) + (t >> 9);	// JoeQuake
				else
					t >>= 7;					// original GLQuake

				t = min(t, 255);
				*dest++ = 255 - t;
			}
		}
	}
}

/************JDH**********/
//void R_UploadLightMap (int lightmapnum)
void R_UploadLightMap (lightmap_t *lm)
/************JDH**********/
{
	glRect_t	*theRect;
	byte		*data;
	int			depth;

/************JDH**********/
	//lightmap_modified[lightmapnum] = false;
	//theRect = &lightmap_rectchange[lightmapnum];
	//glTexSubImage2D (GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, GL_RGB, GL_UNSIGNED_BYTE,
	//	lightmaps + (lightmapnum * BLOCK_HEIGHT + theRect->t) * BLOCK_WIDTH * 3);

	lm->modified = false;
	theRect = &lm->rectchange;

	if (lm->data24)
	{
		data = lm->data24;
		depth = 3;
	}
	else
	{
		data = lm->data;
		depth = lm->depth;
	}

#ifdef _DEBUG
	lm_uploadcount++;
	lm_uploadsize += BLOCK_WIDTH * theRect->h * depth;
#endif

	//glTexSubImage2D (GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, GL_RGB, GL_UNSIGNED_BYTE,
	//					lm->data + (theRect->t * BLOCK_WIDTH * 3));
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, (depth == 3 ? GL_RGB : GL_LUMINANCE),
						GL_UNSIGNED_BYTE, data + (theRect->t * BLOCK_WIDTH * depth));

// clear the changerect
	theRect->l = BLOCK_WIDTH;
	theRect->t = BLOCK_HEIGHT;
	theRect->h = 0;
	theRect->w = 0;
/************JDH**********/

	if (lm->data24)
	{
		int i;
	// convert the current 24-bit texture to 8-bit before freeing
		data = lm->data24;
		for (i = 0; i < BLOCK_WIDTH*BLOCK_HEIGHT; i++)
		{
			lm->data[i] = (data[0] + data[1] + data[2]) / 3;
			data += 3;
		}
		free (lm->data24);
		lm->data24 = NULL;
	}
}

/*
=================
R_UploadLightmaps
=================
*/
void R_UploadLightmaps (void)
{
	int i;

	for (i = 0; i < MAX_LIGHTMAPS; i++)
	{
		if (lightmaps[i].modified)
		{
			GL_Bind (lightmap_textures + i);
			R_UploadLightMap (&lightmaps[i]);
		}
	}
}

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
/*******JDH*******/
//texture_t *R_TextureAnimation (texture_t *base)
texture_t *R_TextureAnimation (entity_t *currententity, texture_t *base)
/*******JDH*******/
{
	int	relative, count;

#ifdef _DEBUG
//	if (!strcmp(currententity->model->name, "*87"))
//		count = 236534;
#endif

	if (currententity->frame)
	{
		if (base->alternate_anims)
			base = base->alternate_anims;
	}

	if (!base->anim_total)
		return base;

	relative = (int)(cl.time*10) % base->anim_total;

	count = 0;
	while (base->anim_min > relative || base->anim_max <= relative)
	{
		base = base->anim_next;
		if (!base)
			Sys_Error ("R_TextureAnimation: broken cycle");
		if (++count > 100)
			Sys_Error ("R_TextureAnimation: infinite cycle");
	}

	return base;
}

/*
===============================================================================

				BRUSH MODELS

===============================================================================
*/

/*
================
OnChange_lightmaps
  - common callback for r_fullbright and gl_lightmode
================
*/
qboolean OnChange_lightmaps (cvar_t *var, const char *value)
{
	int i, j;
	model_t *m;
	msurface_t *surf;

	// if no lightdata, map is always fullbright - no need to rebuild lightmaps
	if (cl.worldmodel && cl.worldmodel->lightdata && (cls.state == ca_connected))
	{
		//Con_Print( "Rebuilding lightmaps...\n" );
		//var->value = Q_atof (value);		// since it's not set globally until AFTER this function returns
		//GL_BuildLightmaps ();

		for (j=1 ; j<MAX_MODELS ; j++)
		{
			if (!(m = cl.model_precache[j]))
				break;

			if (m->name[0] == '*')
				continue;

			surf = m->surfaces;
			for (i=0; i < m->numsurfaces; i++, surf++)
			{
				if (!(surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY)))
					surf->cached_dlight = true;			// this forces surf's lightmap to be rebuilt (R_RenderDynamicLightmaps)
			}
		}
	}

	return false;		// allow change
}

/*
================
R_BlendLightmaps
================
*/
void R_BlendLightmaps (void)
{
	int			i, j;
	glpoly_t	*p;
	float		*v;
/************JDH**************/
	lightmap_t *lm;
	qboolean	wasblendon;
/************JDH**************/

	if (!render_lightmaps)
		return;

	glDepthMask (GL_FALSE);		// don't bother writing Z
	if (gl_lightmode.value >= 3)
		glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	else
		glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

	wasblendon = glIsEnabled (GL_BLEND);
	if (!r_lightmap.value)
	{
		if (!wasblendon)
			glEnable (GL_BLEND);
	}
	else if (wasblendon)
		glDisable (GL_BLEND);


/************JDH**************/
	/*for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		if (!(p = lightmap_polys[i]))
			continue;
		GL_Bind (lightmap_textures + i);
		if (lightmap_modified[i])
			R_UploadLightMap (i);

		for ( ; p ; p = p->chain)
		{
			glBegin (GL_POLYGON);
			v = p->verts[0];
			for (j=0 ; j<p->numverts ; j++, v += VERTEXSIZE)
			{
				glTexCoord2f (v[5], v[6]);
				glVertex3fv (v);
			}
			glEnd ();
		}
		lightmap_polys[i] = NULL;
	}*/

	lm = &lightmaps[0];
	for (i = 0; i < MAX_LIGHTMAPS; i++, lm++)
	{
		if (!(p = lm->polys)) continue;

		GL_Bind (lightmap_textures + i);

#ifndef LIGHTMAPFIX
		if (lm->modified)
			R_UploadLightMap (lm);
#endif
		for ( ; p; p = p->chain)
		{
			glBegin (GL_POLYGON);
			v = p->verts[0];
			for (j=0 ; j<p->numverts ; j++, v += VERTEXSIZE)
			{
				glTexCoord2f (v[5], v[6]);
				glVertex3fv (v);
			}
			glEnd ();
		}

		lm->polys = NULL;
	}

//	if (!r_lightmap.value)
//		glDisable (GL_BLEND);

	if (!r_lightmap.value)
	{
		if (!wasblendon)
			glDisable (GL_BLEND);
	}
	else if (wasblendon)
		glEnable (GL_BLEND);

/************JDH**************/

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (GL_TRUE);		// back to normal Z buffering

	render_lightmaps = false;
}

/*
================
GL_ForceLightMapReload
  - marks the surf's lightmap as modified, and updates its changerect
    (texture will be reupped when R_DrawBrushModel is next called)
================
*/
void GL_ForceLightMapReload (msurface_t *surf)
{
	lightmap_t	*lm;
	glRect_t	*theRect;
	int			smax, tmax;
	
	lm = &lightmaps[surf->lightmaptexturenum];
	theRect = &lm->rectchange;

	lm->modified = true;

// expand the changerect to include this lightmap
	if (surf->light_t < theRect->t)
	{
		if (theRect->h)
			theRect->h += theRect->t - surf->light_t;
		theRect->t = surf->light_t;
	}
	if (surf->light_s < theRect->l)
	{
		if (theRect->w)
			theRect->w += theRect->l - surf->light_s;
		theRect->l = surf->light_s;
	}

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;

	if (theRect->w + theRect->l < surf->light_s + smax)
		theRect->w = surf->light_s - theRect->l + smax;
	if (theRect->h + theRect->t < surf->light_t + tmax)
		theRect->h = surf->light_t - theRect->t + tmax;
}
		
/*
================
R_RenderDynamicLightmaps
================
*/
//void R_RenderDynamicLightmaps (msurface_t *fa)
void R_RenderDynamicLightmaps (model_t *mod, msurface_t *fa)
{
//	byte		*base;
	int			maps;
	qboolean	lightstyle_modified = false;
	lightmap_t	*lm;			// JDH

	c_brush_polys++;

	if (!r_dynamic.value)
		return;

	// check for lightmap modification
	for (maps = 0 ; maps < MAXLIGHTMAPS && fa->styles[maps] != 255 ; maps++)
	{
		if (d_lightstylevalue[fa->styles[maps]] != fa->cached_light[maps])
		{
			lightstyle_modified = true;
			break;
		}
	}

	if (fa->dlightframe == r_framecount)
		R_BuildDlightList (fa);
	else
		numdlights = 0;

	if (numdlights == 0 && !fa->cached_dlight && !lightstyle_modified)
		return;

/************JDH**************/
	//lightmap_modified[fa->lightmaptexturenum] = true;
	//theRect = &lightmap_rectchange[fa->lightmaptexturenum];

	lm = &lightmaps[fa->lightmaptexturenum];

//	base = lm->data;
//	base += (fa->light_t * BLOCK_WIDTH + fa->light_s) * depth;
	R_BuildLightMap (mod, fa, lm);

	GL_ForceLightMapReload (fa);
/*
	base = lm->data;
//	base += (fa->light_t * BLOCK_WIDTH + fa->light_s) * 3;
	base += (fa->light_t * BLOCK_WIDTH + fa->light_s) * depth;
//	R_BuildLightMap (fa, base, BLOCK_WIDTH * 3);
	R_BuildLightMap (fa, base, depth);
*/
}


/*
================
R_DrawWaterSurfaces
================
*/
void R_DrawWaterSurfaces (void)
{
	msurface_t	*s;
	int boundtex;

	if (!waterchain)
		return;

	GL_DisableMultitexture ();

	if (wateralpha < 1.0)
	{
		glEnable (GL_BLEND);
		glColor4f (1, 1, 1, wateralpha);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		if (wateralpha < 0.9)
			glDepthMask (GL_FALSE);
	}

	boundtex = 0;
	for (s = waterchain ; s ; s = s->texturechain)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			if (s->flags & SURF_TRANSLUCENT)
				glColor4f (1, 1, 1, wateralpha);
			else
				glColor4f (1, 1, 1, 1);
		}
	#endif

		if (s->texinfo->texture->gl_texturenum != boundtex)
		{
			boundtex = s->texinfo->texture->gl_texturenum;
			GL_Bind (boundtex);
		}

		EmitWaterPolys (s, false);
//		if (strstr(s->texinfo->texture->name, "tele"))
//			EmitWaterPolys2 (s);
		EmitWaterPolysReflection (s);		// JT030105 - reflections
	}
//	waterchain = NULL;
//	waterchain_tail = &waterchain;

	if (wateralpha < 1.0)
	{
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glColor3f (1, 1, 1);
		glDisable (GL_BLEND);
		if (wateralpha < 0.9)
			glDepthMask (GL_TRUE);
	}
}

/*void R_DrawAlphaChain (void)
{
	int		k;
	msurface_t	*s;
	texture_t	*t;
	float		*v;

	if (!alphachain)
		return;

	glEnable (GL_ALPHA_TEST);

	for (s = alphachain ; s ; s = s->texturechain)
	{
		t = s->texinfo->texture;
		R_RenderDynamicLightmaps (s);

		// bind the world texture
		GL_DisableMultitexture ();
		GL_Bind (t->gl_texturenum);

		if (gl_mtexable)
		{
			// bind the lightmap texture
			GL_EnableMultitexture ();
			GL_Bind (lightmap_textures + s->lightmaptexturenum);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
			// update lightmap if its modified by dynamic lights
			k = s->lightmaptexturenum;

			if ( lightmaps[k].modified )
				R_UploadLightMap( &lightmaps[k] );
		}

		glBegin (GL_POLYGON);
		v = s->polys->verts[0];
		for (k = 0 ; k < s->polys->numverts ; k++, v += VERTEXSIZE)
		{
			if (gl_mtexable)
			{
				qglMultiTexCoord2f (GL_TEXTURE0_ARB, v[3], v[4]);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, v[5], v[6]);
			}
			else
			{
				glTexCoord2f (v[3], v[4]);
			}
			glVertex3fv (v);
		}
		glEnd ();
	}

	alphachain = NULL;

	glDisable (GL_ALPHA_TEST);
	GL_DisableMultitexture ();
}
*/

static void R_ClearPolyLists (void)
{
	int i;

	//memset (lightmap_polys, 0, sizeof(lightmap_polys));

	for ( i = 0; i < MAX_LIGHTMAPS; i++ )
		lightmaps[i].polys = NULL;

	memset (fullbright_polys, 0, sizeof(fullbright_polys));
	memset (luma_polys, 0, sizeof(luma_polys));
	detail_polys = NULL;
	caustics_polys = NULL;
}

static void R_ClearTextureChains (model_t *clmodel)
{
	int		i/*, waterline*/;
	texture_t	*texture;

//	if (!r_fastworld.value)
//		R_ClearPolyLists();

	for (i=0 ; i<clmodel->numtextures ; i++)
	{
		//for (waterline=0 ; waterline<2 ; waterline++)
		{
			if ((texture = clmodel->textures[i]))
			{
				texture->texturechain = NULL;

				//texture->texturechain[waterline] = NULL;
				//texture->texturechain_tail[waterline] = &texture->texturechain[waterline];
			}
		}
	}

	r_notexture_mip->texturechain = NULL;
//	r_notexture_mip->texturechain[0] = NULL;
//	r_notexture_mip->texturechain_tail[0] = &r_notexture_mip->texturechain[0];
//	r_notexture_mip->texturechain[1] = NULL;
//	r_notexture_mip->texturechain_tail[1] = &r_notexture_mip->texturechain[1];

	if (!r_drawskylast || (clmodel == cl.worldmodel))
		skychain = NULL;

//	skychain_tail = &skychain;
	if (clmodel == cl.worldmodel)
	{
		waterchain = NULL;
		waterchain_tail = &waterchain;
	}
//	alphachain = NULL;
//	alphachain_tail = &alphachain;
}

/*
================
R_DistanceToSurf
================
*/
// TODO: implement properly by calculating min distance to surface's plane
//       (can't remember how to do this anymore...)
int R_DistanceToSurf (msurface_t *s)
{
	vec3_t tempv, sumv = {0,0,0};
	float *v = s->polys->verts[0];
	int i;
	float mindist = 99999, dist;

	for (i = 0 ; i < s->polys->numverts ; i++, v += VERTEXSIZE)
	{
		VectorAdd (v, sumv, sumv);
		VectorSubtract (r_refdef.vieworg, v, tempv);
		dist = VectorLength (tempv);
		if (dist < mindist)
			mindist = dist;
	}

	// calc the average of the verts, and check the distance to it:
	VectorScale (sumv, 1.0 / (float) s->polys->numverts, sumv);
	VectorSubtract (r_refdef.vieworg, v, tempv);
	dist = VectorLength (tempv);
	if (dist < mindist)
		mindist = dist;

#ifdef _DEBUG
//	if (!strcmp(s->texinfo->texture->name, "comp03_4"))
//		Con_Printf ("mindist = %d\n", (int) mindist);
#endif

	return (int)mindist;
}


/*#define GLR_DRAW_LM   0x0001
#define GLR_DRAW_FB   0x0002
#define GLR_DRAW_LUMA 0x0003*/

extern qboolean	gl_combine_support;

/*
================
R_DrawTextureChain
================
*/
//void R_DrawTextureChain (entity_t *ent, texture_t *t, int texnum, qboolean can_mtex_lightmaps,
//							qboolean can_mtex_fbs, qboolean can_mtex_detail)
void R_DrawTextureChain (entity_t *ent, texture_t *t, int texnum)
{
	int			k, /*waterline,*/ tmu_lightmap, tmu_fullbright, dist;
	msurface_t	*s;
	float		*v;
	int			curr_tmu, tmu_detail;
	qboolean	draw_detail, can_mtex_detail = false, can_mtex_light = false;
	qboolean	draw_lightmap = false, draw_fbs;

	// bind the world texture
	GL_SelectTMUTexture (GL_TEXTURE0_ARB);
	GL_Bind (t->gl_texturenum);

#ifdef HEXEN2_SUPPORT
	if (!hexen2 || ((ent->drawflags & MLS_ABSLIGHT) != MLS_ABSLIGHT))
#endif
	{
		if (/*!nehahra ||*/ !ent->fullbright)
			draw_lightmap = true;
	}

//	draw_fbs = (/*t->isLumaTexture ? true :*/ ent->model->isworldmodel ? gl_fb_world.value : gl_fb_bmodels.value);
	if (!t->fb_texturenum)
		draw_fbs = false;
	else
		draw_fbs = (!draw_lightmap ? false : ent->model->isworldmodel ? gl_fb_world.value : gl_fb_bmodels.value);

	draw_detail = (gl_detail.value && detailtexture[0] && (r_modelalpha == 1.0));
	tmu_lightmap = tmu_fullbright = tmu_detail = curr_tmu = 0;

	if (gl_mtexable && (r_modelalpha == 1))
	{
		// TODO: figure out proper combiner to do multi-texture with alpha bmodel

		/*if ((r_modelalpha != 1) && gl_combine_support)
		{
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
			glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
			glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA);
		}*/

		/* JDH: Drawing luma textures when fb is off doesn't have enough of an effect to make it worthwhile
		//if (t->isLumaTexture && !gl_fb_bmodels.value)
		if (t->isLumaTexture && !draw_fbs)
		{
			if (gl_add_ext)
			{
				tmu_fullbright = GL_TEXTURE0_ARB + (++curr_tmu);
				GL_EnableTMU (tmu_fullbright);
				glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
				GL_Bind (t->fb_texturenum);

				if (gl_textureunits > 2)
				{
					tmu_lightmap = GL_TEXTURE0_ARB + (++curr_tmu);
					GL_EnableTMU (tmu_lightmap);
					glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
				}
				else
				{
					render_lightmaps = true;		// can't apply lightmaps before luma
				}
			}
			else
			{
				GL_DisableTMU (GL_TEXTURE1_ARB);		// can't apply lightmaps before luma
				render_lightmaps = true;
			}
		}
		else*/
		{
			if (draw_lightmap)
			{
				if ((gl_lightmode.value < 3) || gl_combine_support)
				{
					can_mtex_light = true;
					tmu_lightmap = GL_TEXTURE0_ARB + (++curr_tmu);
					GL_EnableTMU (tmu_lightmap);
					
					if (gl_lightmode.value >= 3)		// overbright
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
						glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
					}
					else
						glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
				}
				else
					render_lightmaps = true;
			}

		//	if (t->fb_texturenum && draw_mtex_fbs)
			if (draw_fbs && (gl_textureunits > curr_tmu+1))
			{
				if (!t->isLumaTexture || gl_add_ext)
				{
					tmu_fullbright = GL_TEXTURE0_ARB + (++curr_tmu);
					GL_EnableTMU (tmu_fullbright);
					GL_Bind (t->fb_texturenum);
					glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, t->isLumaTexture ? GL_ADD : GL_DECAL);
				}
			}
		}

		if (draw_detail && (gl_textureunits >= 4) && gl_combine_support)
			can_mtex_detail = true;


		/*if (draw_detail && !render_lightmaps)
		{
			// cannot multitexture detail if lights still need rendering

			if (can_mtex_detail)
			{
				tmu_detail = GL_TEXTURE0_ARB + ++curr_tmu;
				GL_EnableTMU (tmu_detail);
				GL_Bind (detailtexture);

				glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
				glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
				glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

				glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
			}
		}*/
	}
	else		// no multitexture
	{
		if (draw_lightmap)
			render_lightmaps = true;
	}


//	for (waterline=0 ; waterline<2 ; waterline++)
	{
//		if (!(s = ent->model->textures[ texnum ]->texturechain[waterline]))
//			continue;

		s = ent->model->textures[texnum]->texturechain;

		for ( ; s ; s = s->texturechain)
		{
			if (!s->polys) continue;		// JDH
		#ifdef FITZWORLD
			if (r_fastworld.value && s->culled) continue;
		#endif

			k = s->lightmaptexturenum;

			if ((k >= 0) && draw_lightmap)
			{
			#ifndef LIGHTMAPFIX
				R_RenderDynamicLightmaps (ent->model, s);
			#endif
				//if (tmu_lightmap)
				if (can_mtex_light)
				{
					if (!tmu_lightmap)
					{
						tmu_lightmap = GL_TEXTURE1_ARB;
					}
					GL_EnableTMU (tmu_lightmap);
					GL_Bind (lightmap_textures + k);

				#ifndef LIGHTMAPFIX
					// JDH: this resulted in way more uploads than necessary, decreasing performance
					//      It's now done when building the texture chains.
					if (lightmaps[k].modified)
						R_UploadLightMap (&lightmaps[k]);
				#endif
				}
				else
				{
					s->polys->chain = lightmaps[k].polys;
					lightmaps[k].polys = s->polys;
				}
			}
			else if (can_mtex_light && tmu_lightmap)
			{
				GL_DisableTMU (tmu_lightmap);
				tmu_lightmap = 0;
			}

			if (draw_detail)
			{
				dist = R_DistanceToSurf (s);

				if (can_mtex_detail)
				{
					if (dist <= MAX_DETAIL_DIST)
					{
						if (!tmu_detail)
						{
							tmu_detail = GL_TEXTURE3_ARB;
							GL_EnableTMU (tmu_detail);
						}

						//k = NUM_DETAIL_LEVELS * ((float)dist / (MAX_DETAIL_DIST+1));
						//GL_Bind (detailtexture[k]);
					}
					else
					{
						if (tmu_detail)
						{
							GL_DisableTMU (tmu_detail);
							tmu_detail = 0;
						}
					}
				}
			}

			v = s->polys->verts[0];

			glBegin (GL_POLYGON);
			for (k = 0 ; k < s->polys->numverts ; k++, v += VERTEXSIZE)
			{
#ifdef _DEBUG
				if (v[3] < 0 || v[3] > 1 || v[4] < 0 || v[4] > 1)
					k *=1;
#endif				
				if ((curr_tmu > 0) || tmu_detail)
				{
					qglMultiTexCoord2f (GL_TEXTURE0_ARB, v[3], v[4]);

					if (tmu_lightmap)
						qglMultiTexCoord2f (tmu_lightmap, v[5], v[6]);

					if (tmu_fullbright)
						qglMultiTexCoord2f (tmu_fullbright, v[3], v[4]);

					if (tmu_detail)
						qglMultiTexCoord2f (tmu_detail, v[7]*13, v[8]*13);
				}
				else
				{
					glTexCoord2f (v[3], v[4]);
				}
				glVertex3fv (v);
			}
			glEnd ();

//			if (waterline && gl_caustics.value && underwatertexture)
			if ((s->flags & SURF_UNDERWATER) && gl_caustics.value && underwatertexture)
			{
				s->polys->caustics_chain = caustics_polys;
				caustics_polys = s->polys;
			}

//			if (!waterline && draw_detail && !tmu_detail && is_surf_near)
			if (/*!(s->flags & SURF_UNDERWATER) &&*/ draw_detail && !tmu_detail)
			{
				s->polys->detail_chain = detail_polys;
				detail_polys = s->polys;
				s->polys->dist = dist;
			}

			if (draw_fbs && !tmu_fullbright)
			{
				if (t->isLumaTexture)
				{
					s->polys->luma_chain = luma_polys[t->fb_texturenum];
					luma_polys[t->fb_texturenum] = s->polys;
					drawlumas = true;
				}
				else
				{
					s->polys->fb_chain = fullbright_polys[t->fb_texturenum];
					fullbright_polys[t->fb_texturenum] = s->polys;
					drawfullbrights = true;
				}
			}
		}
	}

	if (tmu_detail)
		GL_DisableTMU (tmu_detail);

	if (tmu_fullbright)
		GL_DisableTMU (tmu_fullbright);

	if (tmu_lightmap)
		GL_DisableTMU (tmu_lightmap);
}

/*
================
R_PreBrushDraw
================
*/
void R_PreBrushDraw (entity_t *ent)
{
	if (r_modelalpha == 1)
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		float intensity = 1.0f, alpha_val = 1.0f;

		if (ent->drawflags & DRF_TRANSLUCENT)
		{
			glEnable (GL_BLEND);
			alpha_val = r_modelalpha = r_wateralpha.value;
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			intensity = 1.0;
		}

		if ((ent->drawflags & MLS_ABSLIGHT) == MLS_ABSLIGHT)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			intensity = (float) ent->abslight / 255.0f;
		}

		glColor4f (intensity, intensity, intensity, alpha_val);
	}
#endif
}

/*
================
R_PostBrushDraw
================
*/
void R_PostBrushDraw (entity_t *ent)
{
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if ((ent->drawflags & MLS_ABSLIGHT) == MLS_ABSLIGHT ||
			(ent->drawflags & DRF_TRANSLUCENT))
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}

		if (ent->drawflags & DRF_TRANSLUCENT)
			glDisable (GL_BLEND);
	}
#endif
}

/*
================
DrawTextureChains
================
*/
void DrawTextureChains (entity_t *ent, model_t *model)
{
//	qboolean	can_mtex_lightmaps, can_mtex_fbs, can_mtex_detail;
	int			i;
	texture_t	*currtex, *t;
	msurface_t	*s;

	if (!model->textures)
		return;

/*	if (gl_fb_bmodels.value)
	{
		can_mtex_lightmaps = gl_mtexable;
		can_mtex_fbs = gl_textureunits >= 3;
	}
	else		// JDH: isn't this backwards?
	{
		can_mtex_lightmaps = gl_textureunits >= 3;
		can_mtex_fbs = gl_textureunits >= 3 && gl_add_ext;
	}

	can_mtex_detail = (gl_textureunits >= 4) && gl_combine_support;
*/
	GL_DisableMultitexture ();

	R_PreBrushDraw (ent);

	for (i = 0; i < model->numtextures; i++)
	{
		currtex = model->textures[i];
//		if (!currtex || (!currtex->texturechain[0] && !currtex->texturechain[1]))
		if (!currtex || !currtex->texturechain)
			continue;

		// make sure something is actually getting drawn:
		for (s = currtex->texturechain ; s ; s = s->texturechain)
		{
			if (!s->polys) continue;
		#ifdef FITZWORLD
			if (!r_fastworld.value || !s->culled)
		#endif
			{
				t = R_TextureAnimation (ent, currtex);
//				R_DrawTextureChain (ent, t, i, can_mtex_lightmaps, can_mtex_fbs, can_mtex_detail);
				R_DrawTextureChain (ent, t, i);
				break;
			}
		}
	}

	if (gl_mtexable)
		GL_SelectTMUTexture (GL_TEXTURE0_ARB);

	R_PostBrushDraw (ent);

/*	if ((model->isworldmodel && gl_fb_world.value) ||
		(!model->isworldmodel && gl_fb_bmodels.value))*/
	{
		if (render_lightmaps)
			R_BlendLightmaps ();
		if (drawfullbrights)
			R_RenderFullbrights ();
		if (drawlumas)
			R_RenderLumas ();
	}
/*	else
	{
		if (drawlumas)
			R_RenderLumas ();
		if (render_lightmaps)
			R_BlendLightmaps ();
		if (drawfullbrights)
			R_RenderFullbrights ();
	}
*/
	EmitCausticsPolys ();
	EmitDetailPolys ();
}

/*
=================
R_DrawSurface
- this needs to do all the same stuff as drawing 1 surf in DrawTextureChain

TODO: multitexturing
=================
*/
#ifdef FITZWORLD

void R_DrawSurface (entity_t *ent, msurface_t *psurf)
{
	msurface_t	*surfchain_old/*, **surfchain_tail_old*/, *texchain_old;
	texture_t	*t;
	int			underwater, k;

	if (psurf->flags & SURF_DRAWSKY)
	{
		// preserve existing chain built in R_MarkLeaves:
		surfchain_old = skychain;
//		surfchain_tail_old = skychain_tail;
		texchain_old = psurf->texturechain;

		psurf->texturechain = NULL;
		skychain = psurf;

		R_DrawSkyChainOld();

		// restore old chain:
		psurf->texturechain = texchain_old;
		skychain = surfchain_old;
//		skychain_tail = surfchain_tail_old;
	}
	else if (psurf->flags & SURF_DRAWTURB)
	{
		GL_Bind (psurf->texinfo->texture->gl_texturenum);

		//JDH: if model has no transparency value, tell EmitWaterPolys to use wateralpha;
		//     otherwise tell EmitWaterPolys to skip default setup since blend is already set
		EmitWaterPolys (psurf, (r_modelalpha == 1));
		EmitWaterPolysReflection (psurf);	// JT030105 - reflections
	}
	else
	{
		underwater = (psurf->flags & SURF_UNDERWATER) ? 1 : 0;
		R_PreBrushDraw (ent);

		t = R_TextureAnimation (ent, psurf->texinfo->texture);
		GL_Bind (t->gl_texturenum);
		DrawGLPoly (psurf->polys);

	#ifdef HEXEN2_SUPPORT
		if (!hexen2 || ((ent->drawflags & MLS_ABSLIGHT) != MLS_ABSLIGHT))
	#endif
			if (!ent->fullbright)		// nehahra flag to omit lighting
			{
				k = psurf->lightmaptexturenum;
				if (k >= 0)
				{
					render_lightmaps = true;
					R_RenderDynamicLightmaps (ent->model, psurf);
				#ifdef LIGHTMAPFIX
					if (lightmaps[k].modified)
						R_UploadLightMap (&lightmaps[k]);
				#endif
					lightmaps[psurf->lightmaptexturenum].polys = psurf->polys;
				}
			}

		if (underwater && gl_caustics.value && underwatertexture)
		{
			psurf->polys->caustics_chain = caustics_polys;
			caustics_polys = psurf->polys;
		}

		if (!underwater && gl_detail.value && detailtexture[0] && (r_modelalpha == 1.0))
		{
//			if (R_IsSurfNear(psurf))
			if (R_DistanceToSurf (psurf) <= MAX_DETAIL_DIST)
			{
				psurf->polys->detail_chain = detail_polys;
				detail_polys = psurf->polys;
			}
		}

		if (t->fb_texturenum && (t->isLumaTexture || gl_fb_bmodels.value))
		{
			if (t->isLumaTexture)
			{
				psurf->polys->luma_chain = luma_polys[t->fb_texturenum];
				luma_polys[t->fb_texturenum] = psurf->polys;
				drawlumas = true;
			}
			else
			{
				psurf->polys->fb_chain = fullbright_polys[t->fb_texturenum];
				fullbright_polys[t->fb_texturenum] = psurf->polys;
				drawfullbrights = true;
			}
		}

		R_PostBrushDraw (ent);

//		if (gl_fb_bmodels.value)
		{
			if (render_lightmaps)
				R_BlendLightmaps ();
			if (drawfullbrights)
				R_RenderFullbrights ();
			if (drawlumas)
				R_RenderLumas ();
		}
/*		else
		{
			if (drawlumas)
				R_RenderLumas ();
			if (render_lightmaps)
				R_BlendLightmaps ();
			if (drawfullbrights)
				R_RenderFullbrights ();
		}
*/
		EmitCausticsPolys ();
		EmitDetailPolys ();
	}
}
#endif

/*
=================
R_PositionBrushModel
=================
*/
void R_PositionBrushModel (entity_t *e, vec3_t mins, vec3_t maxs)
{
	int		i/*, axis*/;
//	float	width, minwidth;
	vec3_t	offset;

// JDH: my attempt to fix z-fighting

	if (gl_zfightfix.value)
	{
/*		minwidth = 99999;

		// find the axis with the smallest width:
		for (i = 0; i < 3; i++)
		{
			width = fastfabs (maxs[i]-mins[i]);
			if (width < minwidth)
			{
				minwidth = width;
				axis = i;
			}
		}

		// shift model away from player along this axis:
		VectorCopy (e->origin, offset);
		if (e->origin[axis] > r_refdef.vieworg[axis])
			offset[axis] += 0.01;
		else
			offset[axis] -= 0.01;

		glTranslatef (offset[0], offset[1], offset[2]);
*/
		VectorCopy (e->origin, offset);

		for (i = 0; i < 3; i++)
		{
			if (offset[i] > r_refdef.vieworg[i])
				offset[i] += 0.01;
			else
				offset[i] -= 0.01;
		}

		glTranslatef (offset[0], offset[1], offset[2]);
	}
	else
		glTranslatef (e->origin[0], e->origin[1], e->origin[2]);

#ifdef _DEBUG
	if (e->angles[0] || e->angles[1] || e->angles[2])
		i = 2134;
#endif
	
	glRotatef (e->angles[1], 0, 0, 1);
	glRotatef (e->angles[0], 0, 1, 0);
	glRotatef (e->angles[2], 1, 0, 0);
/*	{
		vec3_t	d;
		extern void R_CalcBlendedRotateVector (entity_t *e, vec3_t v);
		extern void R_RotateEntity (entity_t *e, float yaw, float pitch, float rot);

		// orientation interpolation (Euler angles, yuck!)
		R_CalcBlendedRotateVector (e, d);

		R_RotateEntity (e, e->angles1[1] + d[1], -(e->angles1[0] + d[0]), e->angles1[2] + d[2]);
	}
*/}

/*
=================
R_DrawBrushModel
=================
*/
void R_DrawBrushModel (entity_t *e)
{
	int		i, k/*, underwater*/;
	vec3_t		mins, maxs;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	model_t		*clmodel;
	qboolean	rotated;

/*******JDH*******/
	//currententity = e;
/*******JDH*******/
	currenttexture = -1;

#ifdef _DEBUG
//	if (!strcmp(e->model->name, "*30"))
//		e->fullbright = 1;
#endif

	if (!gl_notrans.value)
	{
	/*******JDH*******/
		//r_modelalpha = currententity->transparency;
		r_modelalpha = e->transparency;
	/*******JDH*******/
		if (r_modelalpha == 0)
			r_modelalpha = 1.0;
	}
	else
	{
		r_modelalpha = 1.0;
	}

	clmodel = e->model;

	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		rotated = true;
#ifdef _WIN32
// JDH: R_CullSphere doesn't seem to work reliably on Linux
		if (R_CullSphere(e->origin, clmodel->radius))
			return;
#else
		for (i=0 ; i<3 ; i++)
		{
			mins[i] = e->origin[i] - clmodel->radius;
			maxs[i] = e->origin[i] + clmodel->radius;
		}

		if (R_CullBox(mins, maxs))
			return;
#endif
	}
	else
	{
		rotated = false;
		VectorAdd (e->origin, clmodel->mins, mins);
		VectorAdd (e->origin, clmodel->maxs, maxs);

		if (R_CullBox(mins, maxs))
			return;
	}

/*****JDH*****/
	if (r_modelalpha != 1)
	{
/*****JDH*****/
		//if (!gl_mtexable || !gl_combine_support || r_fastworld.value)
		{
			glEnable (GL_BLEND);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// TNT Fix
		}
		glColor4f (1, 1, 1, r_modelalpha);
/*****JDH*****/
	}
	else glColor3f (1, 1, 1);
/*****JDH*****/

	VectorSubtract (r_refdef.vieworg, e->origin, modelorg);
	if (rotated)
	{
		vec3_t	temp, forward, right, up;

		VectorCopy (modelorg, temp);
		AngleVectors (e->angles, forward, right, up);
		modelorg[0] = DotProduct (temp, forward);
		modelorg[1] = -DotProduct (temp, right);
		modelorg[2] = DotProduct (temp, up);
	}

	// calculate dynamic lighting for bmodel if it's not an instanced model
	if (clmodel->firstmodelsurface != 0 && !gl_flashblend.value)
	{
		for (k=0 ; k<MAX_DLIGHTS ; k++)
		{
			if (DLIGHT_INACTIVE(&cl_dlights[k]) || !cl_dlights[k].radius)
				continue;

//			R_MarkLights (&cl_dlights[k], 1 << k, clmodel->nodes + clmodel->hulls[0].firstclipnode);
			R_MarkLights (&cl_dlights[k], ((QINT64) 1) << k, clmodel->nodes + clmodel->hulls[0].firstclipnode);
		}
	}

	glPushMatrix ();
	R_PositionBrushModel (e, mins, maxs);

#ifdef FITZWORLD
	if (!r_fastworld.value)
#endif
		R_ClearTextureChains (clmodel);

	R_ClearPolyLists();

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];

	// draw texture
	for (i=0 ; i<clmodel->nummodelsurfaces ; i++, psurf++)
	{
		// find which side of the node we are on
		pplane = psurf->plane;
		dot = PlaneDiff(modelorg, pplane);

		// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
		#ifdef FITZWORLD
			if (r_fastworld.value)
			{
				R_DrawSurface (e, psurf);
			}
			else
		#endif
			{
				if (psurf->flags & SURF_DRAWSKY)
				{
					CHAIN_SURF_B2F(psurf, skychain);
				}
				else if (psurf->flags & SURF_DRAWTURB)
				{
				/********************JDH******************/
					// fixes wrong texture on func_illusionary teleporters
					// *** does it have an effect on water?? ***

					GL_Bind (psurf->texinfo->texture->gl_texturenum);
				/********************JDH******************/

					//JDH: if model has a transparency value, use that; otherwise do default setup
					EmitWaterPolys (psurf, (r_modelalpha == 1));
	//				if (strstr(psurf->texinfo->texture->name, "tele"))
	//					EmitWaterPolys2 (psurf);
					EmitWaterPolysReflection (psurf);	// JT030105 - reflections
				}
				else
				{
//					underwater = (psurf->flags & SURF_UNDERWATER) ? 1 : 0;
//					CHAIN_SURF_B2F(psurf, psurf->texinfo->texture->texturechain[underwater]);
					CHAIN_SURF_B2F(psurf, psurf->texinfo->texture->texturechain);

				#ifdef LIGHTMAPFIX
				// JDH: lightmap calculations done here now (instead of when drawing)
					if (psurf->polys && (psurf->lightmaptexturenum >= 0))
					{
						R_RenderDynamicLightmaps (clmodel, psurf);
					}
				#endif
				}
			}
		}
	}

/********************JDH******************/
#ifdef FITZWORLD
	if (!r_fastworld.value)
#endif
	{
		//DrawTextureChains (clmodel);
		//R_DrawSkyChain ();

#ifdef LIGHTMAPFIX
		R_UploadLightmaps ();
#endif
		DrawTextureChains (e, clmodel);
		if (!r_drawskylast)
			R_DrawSkyChainOld ();

	/*	if ( skychain )
		{
			if ( r_oldsky.value )
			{
				R_DrawSkyChainOld();
				skychain = NULL;
			}
		}
		else R_DrawSkyChain ();*/
	/********************JDH******************/

//		R_DrawAlphaChain ();
	}

	glPopMatrix ();

/*****JDH*****/
#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	if (r_modelalpha != 1)
/*****JDH*****/
		glDisable (GL_BLEND);
}

/*
===============================================================================

				WORLD MODEL

===============================================================================
*/
qboolean submerged;
int r_watersurf_dist;
msurface_t *r_nearest_watersurf;
extern unsigned GL_GetTextureColor (int texnum);

#ifdef _DEBUG
int r_surfcount = 0, r_cullcount = 0;
#endif

/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode (mnode_t *node, int clipflags)
{
	int		c, side, clipped/*, underwater*/;
	mplane_t	*plane, *clipplane;
	msurface_t	*surf, **mark;
	mleaf_t		*pleaf;
	double		dot;

/****************JDH****************/
	mnode_t *nextnode;

	/*if (node->contents == CONTENTS_SOLID)
		return;		// solid

	//if (node->visframe != r_visframecount)
	if ( !r_novis.value && (node->visframe != r_visframecount) )
		return;*/
/****************JDH****************/

	for (c=0, clipplane=frustum ; c<4 ; c++, clipplane++)
	{
		if (!(clipflags & (1<<c)))
			continue;	// don't need to clip against it

		clipped = BOX_ON_PLANE_SIDE(node->minmaxs, node->minmaxs+3, clipplane);
		if (clipped == 2)
			return;
		else if (clipped == 1)
			clipflags &= ~(1<<c);	// node is entirely on screen
	}

// if a leaf node, draw stuff
	if (node->contents < 0)
	{
		pleaf = (mleaf_t *)node;

		//gTEMPhitleaves++;

		if (!r_novis.value)		// JDH
		{
			mark = pleaf->firstmarksurface;
			c = pleaf->nummarksurfaces;

			if (c)
			{
				do {
					(*mark)->visframe = r_framecount;
					mark++;

					//gTEMPvissurfs++;

				} while (--c);
			}
		}

	// deal with model fragments in this leaf
		if (pleaf->efrags)
			R_StoreEfrags (&pleaf->efrags);

		return;
	}

// node is just a decision point, so go down the apropriate sides

// find which side of the node we are on
	plane = node->plane;

	dot = PlaneDiff(modelorg, plane);
	side = (dot >= 0) ? 0 : 1;

// recurse down the children, front side first
/*******JDH*******/
	//R_RecursiveWorldNode (node->children[side], clipflags);

	nextnode = node->children[side];
	if (nextnode->contents != CONTENTS_SOLID)
	{
		if (r_novis.value || (nextnode->visframe == r_visframecount))
			R_RecursiveWorldNode (nextnode, clipflags);
	}
/*******JDH*******/

// draw stuff
	c = node->numsurfaces;

	if (c)
	{
		surf = cl.worldmodel->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;

		for ( ; c ; c--, surf++)
		{
			if ((dot < 0) ^ !!(surf->flags & SURF_PLANEBACK))
				continue;		// wrong side

		/****************JDH****************/
			//if (surf->visframe != r_framecount)
			if (!r_novis.value)
			{
				if (surf->visframe != r_framecount)
					continue;

				if (R_CullBox(surf->mins, surf->maxs))
				{
				#ifdef _DEBUG
					r_cullcount++;
				#endif
					continue;		// JDH
				}
			}

		/****************JDH****************/

			//gTEMPhitsurfs++;

		#ifdef _DEBUG
			r_surfcount++;
		#endif

			// if sorting by texture, just store it out
			if (surf->flags & SURF_DRAWSKY)
			{
			/****************JDH****************/
				//CHAIN_SURF_F2B(surf, skychain_tail);
				CHAIN_SURF_B2F(surf, skychain);
			/****************JDH****************/
			}
			else if (surf->flags & SURF_DRAWTURB)
			{
				CHAIN_SURF_F2B(surf, waterchain_tail);
			// JDH: for tinting of waterfog
				if (submerged)
				{
					int dist = R_DistanceToSurf (surf);
					if (dist < r_watersurf_dist)
					{
						r_watersurf_dist = dist;
						r_nearest_watersurf = surf;
					}
				}
			}
			else
			{
//				underwater = (surf->flags & SURF_UNDERWATER) ? 1 : 0;
//				CHAIN_SURF_F2B(surf, surf->texinfo->texture->texturechain_tail[underwater]);

				CHAIN_SURF_B2F(surf, surf->texinfo->texture->texturechain);

			#ifdef LIGHTMAPFIX
			// JDH: lightmap calculations done here now (instead of in R_DrawTextureChain)
				if (surf->polys && (surf->lightmaptexturenum >= 0))
				{
					R_RenderDynamicLightmaps (cl.worldmodel, surf);
				}
			#endif
			}
		}
	}

	// recurse down the back side
/*******JDH*******/
	//R_RecursiveWorldNode (node->children[!side], clipflags);

	nextnode = node->children[!side];
	if (nextnode->contents != CONTENTS_SOLID)
	{
		if (r_novis.value || (nextnode->visframe == r_visframecount))
			R_RecursiveWorldNode (nextnode, clipflags);
	}
/*******JDH*******/
}

#ifdef FITZWORLD
/*
=============
R_BuildTextureChains
=============
*/
void R_BuildTextureChains (model_t *m)
{
	int			i, j/*, underwater*/;
	mnode_t		*node;
	msurface_t	*surf;

	for (i=0, node = m->nodes ; i<m->numnodes ; i++, node++)
	{
		for (j=0, surf=&m->surfaces[node->firstsurface] ; j<node->numsurfaces ; j++, surf++)
		{
			if (surf->visframe == r_visframecount)
			{
				if (surf->flags & SURF_DRAWSKY)
				{
					CHAIN_SURF_B2F (surf, skychain);
				}
				else if (surf->flags & SURF_DRAWTURB)
				{
					CHAIN_SURF_F2B (surf, waterchain_tail);
				}
				else
				{
					//underwater = (surf->flags & SURF_UNDERWATER) ? 1 : 0;
					//CHAIN_SURF_F2B (surf, surf->texinfo->texture->texturechain_tail[underwater]);
					CHAIN_SURF_B2F (surf, surf->texinfo->texture->texturechain);

				#ifdef LIGHTMAPFIX
				// JDH: lightmap calculations done here now (instead of in R_DrawTextureChain)
					if (surf->polys && (surf->lightmaptexturenum >= 0))
					{
						R_RenderDynamicLightmaps (cl.worldmodel, surf);
					}
				#endif
				}
			}
		}
	}
}

/*
================
R_BackFaceCull -- johnfitz -- returns true if the surface is facing away from vieworg
================
*/
qboolean R_BackFaceCull (msurface_t *surf)
{
	double dot;

	switch (surf->plane->type)
	{
	case PLANE_X:
		dot = r_refdef.vieworg[0] - surf->plane->dist;
		break;
	case PLANE_Y:
		dot = r_refdef.vieworg[1] - surf->plane->dist;
		break;
	case PLANE_Z:
		dot = r_refdef.vieworg[2] - surf->plane->dist;
		break;
	default:
		dot = DotProduct (r_refdef.vieworg, surf->plane->normal) - surf->plane->dist;
		break;
	}

	if ((dot < 0) ^ !!(surf->flags & SURF_PLANEBACK))
		return true;

	return false;
}

/*
================
R_CullSurfaces -- johnfitz
================
*/
void R_CullSurfaces (void)
{
	msurface_t *s;
	int i;

	s = &cl.worldmodel->surfaces[cl.worldmodel->firstmodelsurface];
	for (i=0 ; i<cl.worldmodel->nummodelsurfaces ; i++, s++)
	{
		if (s->visframe == r_visframecount)
		{
			if (R_CullBox(s->mins, s->maxs) || R_BackFaceCull (s))
				s->culled = true;
			else
			{
				s->culled = false;
				//rs_brushpolys++; //count wpolys here
				//if (s->texinfo->texture->warpimage)
				//	s->texinfo->texture->update_warp = true;
			}
		}
	}
}
#endif		// #ifdef FITZWORLD


/*
=============
R_DrawWorld
=============
*/
void R_DrawWorld (void)
{
	unsigned color;
	msurface_t *last_watersurf;
/****************JDH****************/
	//entity_t	ent;

	//memset (&ent, 0, sizeof(ent));
	//ent.model = cl.worldmodel;
/****************JDH****************/

	R_ClearPolyLists();

	VectorCopy (r_refdef.vieworg, modelorg);

/****************JDH****************/
	//currententity = &ent;
/****************JDH****************/
	currenttexture = -1;

	submerged = (gl_waterfog.value && (r_viewleaf->contents != CONTENTS_EMPTY) && (r_viewleaf->contents != CONTENTS_SOLID));
	if (submerged)
	{
		last_watersurf = r_nearest_watersurf;		// in case there aren't any in the current fov
		r_watersurf_dist = 999999;
		r_nearest_watersurf = NULL;
	}

#ifdef FITZWORLD
	if (r_fastworld.value)
		R_CullSurfaces ();
	else
#endif
	{
		// set up texture chains for the world
		R_ClearTextureChains (cl.worldmodel);
		R_RecursiveWorldNode (cl.worldmodel->nodes, 15);

	#ifdef LIGHTMAPFIX
	// JDH: lightmaps are now calculated in RecursiveWorldNode
		R_UploadLightmaps ();
	#endif
	}

	if (submerged)
	{
		if (gl_waterfog.value == 2)
			color = 0;		// use preset colors based on contents
		else
		{
			if (!r_nearest_watersurf)	// r_nearest_watersurf determined in R_RecursiveWorldNode
				r_nearest_watersurf = last_watersurf;

			if (r_nearest_watersurf)
				color = GL_GetTextureColor (r_nearest_watersurf->texinfo->texture->gl_texturenum);
			else
				submerged = false;
		}
	}

	if (submerged)
		Fog_AddWaterfog (color, r_viewleaf->contents);
	else
		Fog_SetupFrame ();

/****************JDH****************/
	if (skychain && !r_drawskylast)
	{
		R_DrawSky ();
		
		/*if (r_skyboxloaded && !r_oldsky.value)
			R_DrawSkyBox ();
		else if (r_skytype.value == 0)
		{
			R_DrawSkyChain ();
		}*/
	}
/****************JDH****************/

/****************JDH****************/
	// draw the world
	//DrawTextureChains (cl.worldmodel);

	r_modelalpha = 1.0;
	DrawTextureChains (&cl_entities[0], cl.worldmodel);

/* 2009/04/09 - moved up with other sky calls (before DrawTextureChains)
	if (skychain && r_skytype.value && ((r_skyboxloaded && r_oldsky.value) || !r_skyboxloaded))
	{
		R_DrawSkyChainOld ();
	}
*/

	// draw the world alpha textures
//	R_DrawAlphaChain ();

#ifdef _DEBUG
//	if (!cl.paused)
//		Con_DPrintf ("Uploaded %d lightmap textures (%d bytes)\n", lm_uploadcount, lm_uploadsize);
//		Con_DPrintf ("Culled %d of %d surfaces\n", r_cullcount, r_cullcount+r_surfcount);
	lm_uploadcount = 0;
	lm_uploadsize = 0;
	r_cullcount = 0;
	r_surfcount = 0;
#endif
}

extern byte mod_novis[MAX_MAP_LEAFS/8];

/*
===============
R_MarkLeaves
===============
*/
void R_MarkLeaves (void)
{
	byte	*vis;
	int	i;
/*******JDH********/
	mleaf_t *leafs;
	int numleafs;
	mnode_t	*node;
#ifdef FITZWORLD
	mleaf_t *currleaf;
	msurface_t **mark;
	int j;
#endif

	static byte	 r_visbuf[MAX_MAP_LEAFS/8];
	static byte *r_oldvis = NULL;
/*******JDH********/

/*******JDH********/
	//if (!r_novis.value && r_oldviewleaf == r_viewleaf && r_oldviewleaf2 == r_viewleaf2)	// watervis hack
	//	return;

	//r_visframecount++;
	//r_oldviewleaf = r_viewleaf;
/*******JDH********/

	if (r_novis.value)
	{
	/*******JDH********/
		//vis = solid;
		//memset (solid, 0xff, (cl.worldmodel->numleafs+7)>>3);

		//vis = mod_novis;

	#ifdef FITZWORLD
		if (r_fastworld.value)
		{
			if ((r_oldviewleaf == r_viewleaf) && (r_oldviewleaf2 == r_viewleaf2) && !vis_changed)
			{
				currleaf = &cl.worldmodel->leafs[1];
				for (i=0 ; i<cl.worldmodel->numleafs ; i++, currleaf++)
					if (currleaf->efrags)
						R_StoreEfrags (&currleaf->efrags);
				return;
			}
			vis = mod_novis;
		}
		else
	#endif
		{
			r_oldviewleaf = NULL;		// ensure that leaves get marked when novis is turned off
			return;
		}
	/*******JDH********/
	}
	else
	{
	/*******JDH********/
		if ((r_oldviewleaf == r_viewleaf) && (r_oldviewleaf2 == r_viewleaf2))	// watervis hack
		{
		#ifdef FITZWORLD
			if (r_fastworld.value)
			{
				if (!vis_changed)
				{
					currleaf = &cl.worldmodel->leafs[1];
					for (i=0 ; i<cl.worldmodel->numleafs ; i++, currleaf++)
						if (currleaf->efrags)
							if (r_oldvis[i>>3] & (1<<(i&7)))
								R_StoreEfrags (&currleaf->efrags);
					return;
				}
			}
			else
		#endif
				return;
		}
	/*******JDH********/

		vis = Mod_LeafPVS (r_viewleaf, cl.worldmodel);

		if (r_viewleaf2)
		{
			int			count;
			unsigned	*src, *dest;

			// merge visibility data for two leafs
			count = (cl.worldmodel->numleafs+7)>>3;
			memcpy (r_visbuf, vis, count);
			src = (unsigned *) Mod_LeafPVS (r_viewleaf2, cl.worldmodel);
			dest = (unsigned *) r_visbuf;
			count = (count + 3)>>2;
			for (i=0 ; i<count ; i++)
				*dest++ |= *src++;
			vis = r_visbuf;
		}
	}

	r_visframecount++;
	r_oldviewleaf = r_viewleaf;
	r_oldviewleaf2 = r_viewleaf2;
	r_oldvis = vis;

	//gTEMPvisleaves = 0;
	//gTEMPhitleaves = 0;
	//gTEMPvissurfs = 0;
	//gTEMPhitsurfs = 0;

/*******JDH********/
	leafs = cl.worldmodel->leafs;
	numleafs = cl.worldmodel->numleafs;
	//for (i=0 ; i<cl.worldmodel->numleafs ; i++)
	for (i=0 ; i<numleafs ; i++)
/*******JDH********/
	{
		if (vis[i>>3] & (1 << (i & 7)))
		{
		#ifdef FITZWORLD
			if (r_fastworld.value)
			{
				currleaf = &leafs[i+1];
				if (currleaf->contents != CONTENTS_SKY)
				{
					for (j=0, mark = currleaf->firstmarksurface; j<currleaf->nummarksurfaces; j++, mark++)
						(*mark)->visframe = r_visframecount;
				}

				// add static models
				if (currleaf->efrags)
					R_StoreEfrags (&currleaf->efrags);
			}
			else
		#endif
			{
			/*******JDH********/
				//node = (mnode_t *)&cl.worldmodel->leafs[i+1];
				node = (mnode_t *)&leafs[i+1];
			/*******JDH********/
				//gTEMPvisleaves++;
				do
				{
					if (node->visframe == r_visframecount)
						break;
					node->visframe = r_visframecount;
					node = node->parent;
				} while (node);
			}
		}
	}

#ifdef FITZWORLD
	if (r_fastworld.value)
	{
		R_ClearTextureChains (cl.worldmodel);
		R_BuildTextureChains (cl.worldmodel);
		vis_changed = false;
	}
#endif
}

/*
===============================================================================

				LIGHTMAP ALLOCATION

===============================================================================
*/

// returns a texture number and the position inside it
//int AllocBlock (int w, int h, int *x, int *y)
int AllocBlock (int w, int h, int depth, int *x, int *y)
{
	int	i, j, best, best2, texnum;
/*******JDH*******/
	int *lmAlloc;
/*******JDH*******/

	for (texnum=0 ; texnum<MAX_LIGHTMAPS ; texnum++)
	{
	/*******JDH*******/
		if (lightmaps[texnum].depth && (lightmaps[texnum].depth != depth))
			continue;
		lmAlloc = lightmaps[texnum].allocated;
	/*******JDH*******/

		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
			/*******JDH*******/
				//if (allocated[texnum][i+j] >= best)
				//	break;
				//if (allocated[texnum][i+j] > best2)
				//	best2 = allocated[texnum][i+j];

				if (lmAlloc[i+j] >= best)
					break;
				if (lmAlloc[i+j] > best2)
					best2 = lmAlloc[i+j];
			/*******JDH*******/
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

	/*******JDH*******/
		//for (i=0 ; i<w ; i++)
		//	allocated[texnum][*x + i] = best + h;

		for (i=0 ; i<w ; i++)
			lmAlloc[*x + i] = best + h;
	/*******JDH*******/

		lightmaps[texnum].depth = depth;
		return texnum;
	}

	Sys_Error ("AllocBlock: full");
	return 0;
}

/*
================
GL_BuildSurfaceDisplayList
================
*/
void GL_BuildSurfaceDisplayList (model_t *currentmodel, msurface_t *fa)
{
	int		i, lindex, lnumverts, vertpage;
	medge_t		*pedges, *r_pedge;
	float		*vec, s, t;
	glpoly_t	*poly;
	mvertex_t	*r_pcurrentvertbase = currentmodel->vertexes;

	if (fa->polys) return;		// JDH: when rebuilding lighting after r_fullbright change

#ifdef _DEBUG
	if (sv.models[2] && (fa == &sv.models[2]->surfaces[sv.models[2]->firstmodelsurface]))
		fa->polys = NULL;
#endif
	
	// reconstruct the polygon
	pedges = currentmodel->edges;
	lnumverts = fa->numedges;
	vertpage = 0;

	// draw texture
	poly = Hunk_Alloc (sizeof(glpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float));
	poly->next = fa->polys;		// JDH: this seems to always be NULL
	fa->polys = poly;
	poly->numverts = lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = currentmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = r_pcurrentvertbase[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = r_pcurrentvertbase[r_pedge->v[1]].position;
		}

		VectorCopy (vec, poly->verts[i]);

		s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];

		poly->verts[i][3] = s / fa->texinfo->texture->width;
		poly->verts[i][4] = t / fa->texinfo->texture->height;

		// FIXME: does this mean detail texture coords are dependent on texture scale??
		poly->verts[i][7] = s / 128;
		poly->verts[i][8] = t / 128;

		if (fa->lightmaptexturenum < 0)		// JDH: for oversized lightmaps that weren't loaded
		{
			poly->verts[i][5] = 0;
			poly->verts[i][6] = 0;
		}
		else
		{
			// lightmap texture coordinates
			s -= fa->texturemins[0];
			s += fa->light_s * 16;
			s += 8;
			s /= BLOCK_WIDTH * 16;	//fa->texinfo->texture->width;

			t -= fa->texturemins[1];
			t += fa->light_t * 16;
			t += 8;
			t /= BLOCK_HEIGHT * 16;	//fa->texinfo->texture->height;

			poly->verts[i][5] = s;
			poly->verts[i][6] = t;
		}
	}

//	poly->numverts = lnumverts;
}

int numcolorsurfs = 0;
/*
========================
GL_CreateSurfaceLightmap
========================
*/
//void GL_CreateSurfaceLightmap (msurface_t *surf)
void GL_CreateSurfaceLightmap (model_t *mod, msurface_t *surf)
{
	int		smax, tmax;
//	byte	*base;
	int depth = mod->lightdatadepth;

/******* JDH - checked in GL_BuildLightmaps ********/
	//if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB))
	//	return;
/******* JDH - checked in GL_BuildLightmaps ********/

	smax = (surf->extents[0] >> 4) + 1;
	tmax = (surf->extents[1] >> 4) + 1;

/* JDH: now just ignores oversized lightmaps
	if (smax > BLOCK_WIDTH)
		Host_Error ("GL_CreateSurfaceLightmap: smax = %d > BLOCK_WIDTH", smax);
	if (tmax > BLOCK_HEIGHT)
		Host_Error ("GL_CreateSurfaceLightmap: tmax = %d > BLOCK_HEIGHT", tmax);
	if (smax * tmax > MAX_LIGHTMAP_SIZE)
		Host_Error ("GL_CreateSurfaceLightmap: smax * tmax = %d > MAX_LIGHTMAP_SIZE", smax * tmax);
*/
	surf->lightcolored = false;

	if ((smax > BLOCK_WIDTH) || (tmax > BLOCK_HEIGHT) || (smax * tmax > MAX_LIGHTMAP_SIZE))
	{
		surf->lightmaptexturenum = -1;
		return;
	}

	if (depth == 3)
	{
	// JDH: figure out whether the 24-bit lightdata for this surface
	//      actually has any colors (otherwise we can use 8-bit)

		int maps, i, blocksize = smax*tmax*3;
		byte *lightmap = surf->samples;

		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++)
		{
			for (i=0 ; i<blocksize ; i+=3)
			{
				if ((lightmap[i] != lightmap[i+1]) || (lightmap[i] != lightmap[i+2]))
				{
					surf->lightcolored = true;
					numcolorsurfs++;
					goto LMALLOC;
				}

			#ifdef _DEBUG
				if (lightmap[i+1] != lightmap[i+2])
				{
					surf->lightcolored = true;
					goto LMALLOC;
				}
			#endif
			}

			lightmap += blocksize;	// skip to next lightmap
		}

		depth = 1;		// no colored lights for this surf
	}

LMALLOC:
	//surf->lightmaptexturenum = AllocBlock (smax, tmax, &surf->light_s, &surf->light_t);
	surf->lightmaptexturenum = AllocBlock (smax, tmax, depth, &surf->light_s, &surf->light_t);

	//base = lightmaps[surf->lightmaptexturenum].data;
	//base += ((surf->light_t * BLOCK_WIDTH) + surf->light_s) * depth;
	numdlights = 0;
	//R_BuildLightMap (surf, base, BLOCK_WIDTH * 3);
	R_BuildLightMap (mod, surf, &lightmaps[surf->lightmaptexturenum]);
}

/*
==================
GL_BuildLightmaps

Builds the lightmap texture
with all the surfaces from all brush models
==================
*/
void GL_BuildLightmaps (void)
{
	int	i, j;
	model_t	*m;
	msurface_t *surf;
	lightmap_t *lm;
	glRect_t *theRect;

	for (i = 0; i < MAX_LIGHTMAPS; i++)
	{
		memset (lightmaps[i].allocated, 0, sizeof(lightmaps[i].allocated));
		lightmaps[i].depth = 0;
		lightmaps[i].data24 = NULL;
	}

	r_framecount = 1;		// no dlightcache

	if (!lightmap_textures)
	{
		lightmap_textures = texture_extension_number;
		texture_extension_number += MAX_LIGHTMAPS;
	}

	numcolorsurfs = 0;

	for (j=1 ; j<MAX_MODELS ; j++)
	{
		if (!(m = cl.model_precache[j]))
			break;

		if (m->name[0] == '*')
			continue;

	/************ JDH ***********/
		/*r_pcurrentvertbase = m->vertexes;
		currentmodel = m;
		for (i=0 ; i<m->numsurfaces ; i++)
		{
			GL_CreateSurfaceLightmap (m->surfaces + i);
			if (m->surfaces[i].flags & (SURF_DRAWTURB | SURF_DRAWSKY))
				continue;
			BuildSurfaceDisplayList (m->surfaces + i);
		}*/

		surf = m->surfaces;
		for (i=0; i < m->numsurfaces; i++, surf++)
		{
			if (surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY))
				continue;

			GL_CreateSurfaceLightmap (m, surf);
			GL_BuildSurfaceDisplayList (m, surf);
		}
	/************ JDH ***********/
	}

 	if (gl_mtexable)
 		GL_EnableMultitexture ();

// upload all lightmaps that were filled
// JDH: modified to use array of lightmap_t
	lm = &lightmaps[0];
	for (i = 0; i < MAX_LIGHTMAPS; i++, lm++)
	{
		if (!lm->allocated[0])
			break;		// no more used

		lm->modified = false;
		theRect = &lm->rectchange;
		theRect->l = BLOCK_WIDTH;
		theRect->t = BLOCK_HEIGHT;
		theRect->w = 0;
		theRect->h = 0;

		GL_Bind (lightmap_textures + i);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glTexImage2D (GL_TEXTURE_2D, 0, gl_lightmap_format, BLOCK_WIDTH, BLOCK_HEIGHT,
		//				0, GL_RGB, GL_UNSIGNED_BYTE, lm->data);
		glTexImage2D (GL_TEXTURE_2D, 0, lm->depth, BLOCK_WIDTH, BLOCK_HEIGHT, 0,
						(lm->depth == 3 ? GL_RGB : GL_LUMINANCE), GL_UNSIGNED_BYTE, lm->data);
	}

	if (i > 64)
		Con_Printf ("\x02""WARNING: lightmap count (%i) exceeds normal engine max (64)\n", i);

/************JDH**************/

	if (gl_mtexable)
 		GL_DisableMultitexture ();
}

#endif		//#ifndef RQM_SV_ONLY
