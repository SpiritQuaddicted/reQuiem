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
// gl_sky.c -- sky polygons

#include "quakedef.h"

#ifndef RQM_SV_ONLY

extern	msurface_t	*skychain;
//extern	msurface_t	**skychain_tail;

static	int	solidskytexture, alphaskytexture;
static	float	speedscale_x, speedscale_y;		// for top sky layer
static	float	speedscale2_x, speedscale2_y;	// for bottom sky layer


qboolean	r_skyboxloaded;
int			skybox_widths[6]  = {0,0,0,0,0,0};
int			skybox_heights[6] = {0,0,0,0,0,0};
unsigned	r_skytex_width, r_skytex_height;		// JDH


extern	cvar_t	gl_skyclip;

//extern qboolean noclip_anglehack;
extern cvar_t	r_oldsky;

#ifdef FITZWORLD
extern cvar_t	r_fastworld;
#endif

/*
=====================================================================================
MH IMPROVED SKY WARP BEGIN
=====================================================================================
*/
float r_skycolor_avg[4];
int spherelist = -1;
/*
=====================================================================================
MH IMPROVED SKY WARP END
=====================================================================================
*/

/*
=============
EmitSkyPolys
=============
*/
void EmitSkyPolys (msurface_t *fa, qboolean mtex)
{
	glpoly_t	*p;
	float		*v, s, t, ss, tt, length;
	int			i;
	vec3_t		dir;

	for (p = fa->polys ; p ; p = p->next)
	{
		glBegin (GL_POLYGON);
		for (i = 0, v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
		{
			VectorSubtract (v, r_origin, dir);
			dir[2] *= 3;	// flatten the sphere

			length = VectorLength (dir);

			/*length = 6 * 63 / length;
			dir[0] *= length;
			dir[1] *= length;*/

			dir[0] *= 6 * (r_skytex_width/2 - 1) / length;
			dir[1] *= 6 * (r_skytex_height/2 - 1) / length;

			s = (speedscale_x + dir[0]) / (float) r_skytex_width;
			t = (speedscale_y + dir[1]) / (float) r_skytex_height;

			if (mtex)
			{
				ss = (speedscale2_x + dir[0]) / (float) r_skytex_width;
				tt = (speedscale2_y + dir[1]) / (float) r_skytex_height;

				qglMultiTexCoord2f (GL_TEXTURE0_ARB, s, t);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, ss, tt);
			}
			else
			{
				glTexCoord2f (s, t);
			}

			glVertex3fv (v);
		}
		glEnd ();
	}
}

void EmitFlatPoly (msurface_t *fa)
{
	glpoly_t	*p;
	float		*v;
	int		i;

// JDH: fa->samples stores the undivided poly
	if (fa->samples)
		p = (glpoly_t *)fa->samples;
	else
		p = fa->polys;

	for ( ; p ; p = p->next)
	{
		glBegin (GL_POLYGON);
		for (i = 0, v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
			glVertex3fv (v);
		glEnd ();
	}
}

void R_DrawSolidSkyChain (void)
{
	byte		col[4];
	msurface_t	*fa;

	if (Cvar_IsDefaultValue(&r_skycolor))		// "auto"
	{
		glColor3fv (r_skycolor_avg);
	}
	else
	{
		StringToRGB (r_skycolor.string, col);
		glColor3ubv (col);
	}

	GL_DisableMultitexture ();
	glDisable (GL_TEXTURE_2D);

	for (fa = skychain ; fa ; fa = fa->texturechain)
	{
	#ifdef FITZWORLD
		if (!r_fastworld.value || !fa->culled)
	#endif
			EmitFlatPoly (fa);
	}

	glEnable (GL_TEXTURE_2D);
	glColor3f (1, 1, 1);
}

/*
=================
R_DrawSkyChainOld
=================
*/
void R_DrawSkyChainOld (void)
{
	msurface_t	*fa;
	float		speed;

	if (!skychain)
		return;

	if (r_skytype.value == 2)
	{
		R_DrawSolidSkyChain ();
	}
	else
	{
/*		if ((r_skytex_width == 128) && (r_skytex_height == 128))
		{
			speedscale_x = cl.time * 8;
			speedscale_x -= (int)speedscale_x & ~127;
			speedscale_y = speedscale_x;

			speedscale2_x = cl.time * 16;
			speedscale2_x -= (int)speedscale2_x & ~127;
			speedscale2_y = speedscale2_x;
		}
		else
*/		{
			speed = cl.time * (r_skytex_width/16.0);
			speedscale_x = speed - ((int)speed / r_skytex_width) * r_skytex_width;

			speed = cl.time * (r_skytex_height/16.0);
			speedscale_y = speed - ((int)speed / r_skytex_height) * r_skytex_height;

			speed = cl.time * (r_skytex_width/8.0);
			speedscale2_x = speed - ((int)speed / r_skytex_width) * r_skytex_width;

			speed = cl.time * (r_skytex_height/8.0);
			speedscale2_y = speed - ((int)speed / r_skytex_height) * r_skytex_height;
		}


		if (gl_mtexable)
		{
			GL_DisableMultitexture ();
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			GL_Bind (solidskytexture);

			GL_EnableMultitexture ();
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
			GL_Bind (alphaskytexture);

			for (fa = skychain ; fa ; fa = fa->texturechain)
			{
			#ifdef FITZWORLD
				if (!r_fastworld.value || !fa->culled)
			#endif
					EmitSkyPolys (fa, true);
			}

			GL_DisableMultitexture ();
		}
		else
		{
			GL_Bind (solidskytexture);

			for (fa = skychain ; fa ; fa = fa->texturechain)
			{
			#ifdef FITZWORLD
				if (!r_fastworld.value || !fa->culled)
			#endif
					EmitSkyPolys (fa, false);
			}

			glEnable (GL_BLEND);
			GL_Bind (alphaskytexture);
			speedscale_x = speedscale2_x;
			speedscale_y = speedscale2_y;

			for (fa = skychain ; fa ; fa = fa->texturechain)
			{
			#ifdef FITZWORLD
				if (!r_fastworld.value || !fa->culled)
			#endif
					EmitSkyPolys (fa, false);
			}

			glDisable (GL_BLEND);
		}
	}

//	skychain = NULL;
//	skychain_tail = &skychain;
}

/****************JDH****************/

/*
=====================================================================================
MH IMPROVED SKY WARP BEGIN
=====================================================================================
*/
/*
=================
R_DrawSkyChain

Hot damn Jethro, this sho' is purty!!!
=================
*/
void R_DrawSkyChain (void)
{
	// sky scaling
	float fullscale, halfscale, reducedfull, reducedhalf;
	float rotateBack, rotateFore;
	qboolean map_has_fog;
	extern qboolean submerged;
//	extern cvar_t gl_skyfog;

	if (!skychain)
		return;

//	skychain = NULL;
	// JoeQuake specific!!!
//	skychain_tail = &skychain;

	fullscale = bound (4096, r_farclip.value, 16384) / 4096.0;
	halfscale = fullscale * 0.6;		// JDH: was *0.5, but seemed too low overhead
	reducedfull = fullscale * 0.9;
	reducedhalf = halfscale * 0.9;

// JDH: changed realtime to cl.time so it obeys pause and cl_demospeed
	rotateBack = anglemod (cl.time * 6);		// JT022605 - changed from 12 to slow sky
	rotateFore = anglemod (cl.time * 10);		// JT022605 - changed from 20 to slow sky

	map_has_fog = (Fog_GetDensity() > 0) || submerged;
	if (!map_has_fog /*&& gl_skyfog.value*/)			 // JDH: don't mess with fog if it's already active
	{
		// turn on fogging.  fog looks good on the skies - it gives them a more
		// "airy" far-away look, and has the knock-on effect of preventing the
		// old "texture distortion at the poles" problem.
//		glFogi (GL_FOG_MODE, GL_LINEAR);
//		glFogf (GL_FOG_START, 512);		// JDH: increased from 5
//		glFogf (GL_FOG_END, r_farclip.value * 0.60);		// changed from 0.75

	// JDH: switched to exponential fog so it's more evenly distributed across sky:
		glFogi (GL_FOG_MODE, GL_EXP2);			// EXP2 obscures the sky "sides" a bit more than EXP
		glFogf (GL_FOG_DENSITY, 0.0005/fullscale);
//		glFogf (GL_FOG_DENSITY, 0.001*gl_skyfog.value/fullscale);
//		glFogfv (GL_FOG_COLOR, r_skycolor_avg);
		{
//			vec3_t col;
//			VectorScale (r_skycolor_avg, 1.0, col);
//			glFogfv (GL_FOG_COLOR, col);
			glFogfv (GL_FOG_COLOR, r_skycolor_avg);
		}
		glEnable (GL_FOG);
	}

	// sky texture scaling
	// previous releases made a tiled version of the sky texture.  here i just shrink it using the
	// texture matrix, for much the same effect
	glMatrixMode (GL_TEXTURE);
	glLoadIdentity ();
	glScalef (4, 2, 1);			// JDH: was 2,1,1 but a larger scale makes sky look further away
	glMatrixMode (GL_MODELVIEW);

	// background
	// ==========
	// go to a new matrix
	glPushMatrix ();

	// center it on the players position
	glTranslatef (r_origin[0], r_origin[1], r_origin[2]);

	// flatten the sphere
	glScalef (fullscale, fullscale, halfscale);

	// orient it so that the poles are unobtrusive
	glRotatef (-90, 1, 0, 0);

	// make it not always at right angles to the player
	glRotatef (-22, 0, 1, 0);

	// rotate it around the poles
	glRotatef (-rotateBack, 0, 0, 1);

	// solid sky texture
	glBindTexture (GL_TEXTURE_2D, solidskytexture);

	// draw the sphere
	glCallList (spherelist);

	// restore the previous matrix
	glPopMatrix ();

	// foreground
	// ==========
	// go to a new matrix
	glPushMatrix ();

	// center it on the players position
	glTranslatef (r_origin[0], r_origin[1], r_origin[2]);

	// flatten the sphere and shrink it a little - the reduced scale prevents artifacts appearing when
	// corners on the skysphere may potentially overlap
	glScalef (reducedfull, reducedfull, reducedhalf);

	// orient it so that the poles are unobtrusive
	glRotatef (-90, 1, 0, 0);

	// make it not always at right angles to the player
	glRotatef (-22, 0, 1, 0);

	// rotate it around the poles
	glRotatef (-rotateFore, 0, 0, 1);

	// blend mode
	glEnable (GL_BLEND);

	// alpha sky texture
	glBindTexture (GL_TEXTURE_2D, alphaskytexture);

	// draw the sphere
	glCallList (spherelist);

	// back to normal mode
	glDisable (GL_BLEND);

	// restore the previous matrix
	glPopMatrix ();

	// turn off fog
	if (!map_has_fog /*&& gl_skyfog.value*/)
		glDisable (GL_FOG);

	glMatrixMode (GL_TEXTURE);
	glLoadIdentity ();
	glMatrixMode (GL_MODELVIEW);
}
/*
=====================================================================================
MH IMPROVED SKY WARP END
=====================================================================================
*/

//===============================================================


/*
==============
R_DrawSphere

Draw a sphere!!!  The verts and texcoords are precalculated for extra efficiency.  The sphere is put into
a display list to reduce overhead even further.
==============
*/
float skytexes[440];
float skyverts[660];


void R_DrawSphere (void)
{
	if (spherelist == -1)
	{
		int i;
		int j;

		int vertspos = 0;
		int texespos = 0;

		// build the sphere display list
		spherelist = glGenLists (1);

		glNewList (spherelist, GL_COMPILE);

		for (i = 0; i < 10; i++)
		{
			glBegin (GL_TRIANGLE_STRIP);

			for (j = 0; j <= 10; j++)
			{
				glTexCoord2fv (&skytexes[texespos]);
				glVertex3fv (&skyverts[vertspos]);

				texespos += 2;
				vertspos += 3;

				glTexCoord2fv (&skytexes[texespos]);
				glVertex3fv (&skyverts[vertspos]);

				texespos += 2;
				vertspos += 3;
			}

			glEnd ();
		}

		glEndList ();
	}
}


/*
==============
R_InitSkyChain

Initialize the sky chain arrays
==============
*/
void R_InitSkyChain (void)
{
	float drho = 0.3141592653589;
	float dtheta = 0.6283185307180;

	float ds = 0.1;
	float dt = 0.1;

	float t = 1.0f;
	float s = 0.0f;

	int i;
	int j;

	int vertspos = 0;
	int texespos = 0;

	for (i = 0; i < 10; i++)
	{
		float rho = (float) i * drho;
		float srho = (float) (sin (rho));
		float crho = (float) (cos (rho));
		float srhodrho = (float) (sin (rho + drho));
		float crhodrho = (float) (cos (rho + drho));

		s = 0.0f;

		for (j = 0; j <= 10; j++)
		{
			float theta = (j == 10) ? 0.0f : j * dtheta;
			float stheta = (float) (-sin( theta));
			float ctheta = (float) (cos (theta));

			skytexes[texespos++] = s * 2;
			skytexes[texespos++] = t * 2;

			skyverts[vertspos++] = stheta * srho * 4096.0;
			skyverts[vertspos++] = ctheta * srho * 4096.0;
			skyverts[vertspos++] = crho * 4096.0;

			skytexes[texespos++] = s * 2;
			skytexes[texespos++] = (t - dt) * 2;

			skyverts[vertspos++] = stheta * srhodrho * 4096.0;
			skyverts[vertspos++] = ctheta * srhodrho * 4096.0;
			skyverts[vertspos++] = crhodrho * 4096.0;

			s += ds;
		}

		t -= dt;
	}
}


/*
=============
R_InitSky

A sky texture is 256*128, with the right side being a masked overlay
==============
*/
/*
void R_InitSky (miptex_t *mt)
{
	int		i, j, p, r, g, b;
	byte		*src;
	unsigned	trans[128*128], transpix, *rgba;

	src = (byte *)mt + mt->offsets[0];

	// make an average value for the back to avoid a fringe on the top level
	r = g = b = 0;
	for (i=0 ; i<128 ; i++)
		for (j=0 ; j<128 ; j++)
		{
			p = src[i*256 + j + 128];
			rgba = &d_8to24table[p];
			trans[(i*128) + j] = *rgba;
			r += ((byte *)rgba)[0];
			g += ((byte *)rgba)[1];
			b += ((byte *)rgba)[2];
		}

	((byte *)&transpix)[0] = r / (128 * 128);
	((byte *)&transpix)[1] = g / (128 * 128);
	((byte *)&transpix)[2] = b / (128 * 128);
	((byte *)&transpix)[3] = 0;

	if (!solidskytexture)
		solidskytexture = texture_extension_number++;

	GL_Bind (solidskytexture);
	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (i=0 ; i<128 ; i++)
		for (j=0 ; j<128 ; j++)
		{
			p = src[i*256 + j];
			trans[(i * 128) + j] = p ? d_8to24table[p] : transpix;
		}

	if (!alphaskytexture)
		alphaskytexture = texture_extension_number++;

	GL_Bind (alphaskytexture);
	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
*/

/*
==================
R_LoadExtSkyTex (JDH)
==================
*/
unsigned * R_LoadExtSkyTex (const char *name)
{
	char	mappath[MAX_QPATH];
	const char *namelist[2], *pathlist[3];

	if (no24bit || isDedicated || !gl_externaltextures_world.value)
		return NULL;

	namelist[0] = name;
	namelist[1] = NULL;

	Q_snprintfz (mappath, sizeof(mappath), "textures/%s/", Host_MapName ());
	pathlist[0] = mappath;
	pathlist[1] = "textures/";
	pathlist[2] = NULL;

	return (unsigned *) GL_LoadImage_MultiSource (pathlist, namelist, 0, 0);
}

/*
=====================================================================================
MH IMPROVED SKY WARP BEGIN
=====================================================================================
*/
void R_InitSky (miptex_t *mt)
{
	int			width, height, size, i, j, p;
	byte		*src;
	unsigned	*data_24bit, *trans;
	unsigned	transpix;
	int			r, g, b;
	unsigned	*rgba;
	unsigned	topalpha;
	int			div;

	data_24bit = R_LoadExtSkyTex (mt->name);		// JDH
	if (data_24bit)
	{
		width = image_width;
		height = image_height;
	}
	else
	{
		width = mt->width;
		height = mt->height;
		src = (byte *)mt + mt->offsets[0];
	}

	if (width != 256 || height != 128)
		Con_DPrintf ("\x02""WARNING: non-standard sky texture size will cause strange effects in most engines!\n");

// -----------------
// solid sky texture (right-hand half of texture)
// -----------------

	// make an average value for the back to avoid
	// a fringe on the top level
	r = g = b = 0;

// JDH: changed to handle non-256x128 textures
	r_skytex_width = width/2;
	r_skytex_height = height;
	size = r_skytex_height * r_skytex_width;

	// initialize our pointers
	trans = Q_malloc (size * 4);

	for (i=0 ; i<r_skytex_height ; i++)
	{
		for (j=0 ; j<r_skytex_width ; j++)
		{
			if (data_24bit)
			{
				rgba = &data_24bit[i*width + r_skytex_width + j];
			}
			else
			{
				//p = src[i*256 + j + 128];
				p = src[i*width + r_skytex_width + j];

				rgba = &d_8to24table[p];
			}

			trans[i*r_skytex_width + j] = *rgba;

			r += ((byte *)rgba)[0];
			g += ((byte *)rgba)[1];
			b += ((byte *)rgba)[2];
		}
	}

	((byte *)&transpix)[0] = r/size;
	((byte *)&transpix)[1] = g/size;
	((byte *)&transpix)[2] = b/size;
	((byte *)&transpix)[3] = 0;

	if (!solidskytexture)
		solidskytexture = texture_extension_number++;

	glBindTexture (GL_TEXTURE_2D, solidskytexture);

	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, r_skytex_width, r_skytex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
//	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, r_skytex_width, r_skytex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// -----------------
// alpha sky texture
// -----------------
	// MH: get another average cos the bottom layer average can be too dark
	div = r = g = b = 0;

	for (i=0 ; i<r_skytex_height ; i++)
	{
		for (j=0 ; j<r_skytex_width ; j++)
		{
			if (data_24bit)
			{
				// JDH: DarkPlaces uses alpha channel from image, rather than
				//      replacing a certain colour

				rgba = &data_24bit[i*width + j];
				/*if (!*rgba)
					topalpha = transpix;
				else
				{
					r += ((byte *)rgba)[0];
					g += ((byte *)rgba)[1];
					b += ((byte *)rgba)[2];

					div++;

					topalpha = *rgba;
					((byte *)&topalpha)[3] = 0xD0;		// 0.8 instead of 0.5
				}*/
				topalpha = *rgba;
			}
			else
			{
	//			p = src[i*256 + j];
				p = src[i*width + j];

				if (p == 0)
					topalpha = transpix;
				else
				{
					rgba = &d_8to24table[p];

					r += ((byte *)rgba)[0];
					g += ((byte *)rgba)[1];
					b += ((byte *)rgba)[2];

					div++;

					topalpha = *rgba;

				// JDH: reduced transparency to improve distinction between layers
				//      in MH sky (although pixellation is more apparent now...)

					//((byte *)&topalpha)[3] = ((byte *)&topalpha)[3] / 2;
//					((byte *)&topalpha)[3] = 0xD0;		// 0.8 instead of 0.5
					((byte *)&topalpha)[3] = 255;
				}
			}

			trans[i*r_skytex_width + j] = topalpha;
		}
	}

	if (!alphaskytexture) alphaskytexture = texture_extension_number++;

	glBindTexture (GL_TEXTURE_2D, alphaskytexture);

	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, r_skytex_width, r_skytex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
//	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, r_skytex_width, r_skytex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// get fog colours for the sky - we halve these
	r_skycolor_avg[0] = ((float)r / (float)div) / 512.0;
	r_skycolor_avg[1] = ((float)g / (float)div) / 512.0;
	r_skycolor_avg[2] = ((float)b / (float)div) / 512.0;
	r_skycolor_avg[3] = 0.1;

	// free the used memory
	free (trans);
	if (data_24bit)
		free (data_24bit);

	// recalc the sphere display list - delete the original one first!!!
	if (spherelist != -1) glDeleteLists (spherelist, 1);
	spherelist = -1;

	R_InitSkyChain ();
	R_DrawSphere ();
}
/*
=====================================================================================
MH IMPROVED SKY WARP END
=====================================================================================
*/

/*
==================
R_SetSky
==================
*/
static const char	*skybox_ext[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
static const char	*skybox_paths[] = {"env/", "gfx/env/", NULL};

qboolean R_SetSkybox (const char *skyname)
{
	int			namelen, i, j;
	byte		*data[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	char		fname1[MAX_QPATH], fname2[MAX_QPATH];
	const char	*namelist[3] = {fname1, fname2, NULL};

	if (!skyname || !*skyname)
		return false;

	namelen = strlen (skyname);
	if (skyname[namelen-1] == '_')
	{
		Q_strcpy (fname2, skyname, sizeof(fname2));
		Q_strcpy (fname1, skyname, sizeof(fname1));
		fname1[--namelen] = 0;
	}
	else
	{
		Q_strcpy (fname1, skyname, sizeof(fname1));
		Q_snprintfz (fname2, sizeof(fname2), "%s_", skyname);
	}

	for (i=0 ; i<6 ; i++)
	{
	// append suffix for current face:
		Q_strcpy (fname1 + namelen, skybox_ext[i], sizeof(fname1) - namelen);
		Q_strcpy (fname2 + namelen + 1, skybox_ext[i], sizeof(fname2) - namelen - 1);

		data[i] = GL_LoadImage_MultiSource (skybox_paths, namelist, 0, 0);

		/*if (!(data[i] = GL_LoadImage("env/",     va("%s%s", skyname, skybox_ext[i]), 0, 0)) &&
		    !(data[i] = GL_LoadImage("gfx/env/", va("%s%s", skyname, skybox_ext[i]), 0, 0)) &&
		    !(data[i] = GL_LoadImage("env/",     va("%s_%s", skyname, skybox_ext[i]), 0, 0)) &&
		    !(data[i] = GL_LoadImage("gfx/env/", va("%s_%s", skyname, skybox_ext[i]), 0, 0)))
		*/
		if (!data[i])
		{
			Con_Printf ("Couldn't load skybox \"%s\"\n", skyname);
			for (j = 0; j < i; j++)
			{
				free (data[j]);
				skybox_widths[j] = 0;
				skybox_heights[j] = 0;
			}

			r_skyboxloaded = false;
			return false;
		}

		skybox_widths[i] = image_width;
		skybox_heights[i] = image_height;
	}

	for (i=0 ; i<6 ; i++)
	{
		GL_Bind (skyboxtextures + i);
		GL_Upload32 (NULL, (unsigned int *)data[i], &skybox_widths[i], &skybox_heights[i], 0);
		free (data[i]);
	}

	r_skyboxloaded = true;
	return true;
}

qboolean OnChange_r_skybox (cvar_t *var, const char *string)
{
	int i;

	if (!string[0])
	{
		r_skyboxloaded = false;
		for (i = 0; i < 6; i++)
		{
			skybox_widths[i] = 0;
			skybox_heights[i] = 0;
		}

		return false;
	}

	if (nehahra)
	{
		Cvar_SetDirect (&r_oldsky, "0");
		Q_strcpy (prev_skybox, string, sizeof(prev_skybox));
	}

	return !R_SetSkybox (string);
}

qboolean Sky_GetBaseName (char *filename)
{
	int len, i;

	len = strlen (filename);
	if (len < 3)
		return false;		// not long enough to have suffix

	for (i = 0; i < 6; i++)
	{
		if (!Q_strcasecmp(filename+len-2, skybox_ext[i]))
			break;
	}

	if (i == 6)
		return false;		// suffix doesn't match any of the 6

	if ((len > 3) && (filename[len-3] == '_'))
		len -= 3;
	else
		len -= 2;

	filename[len] = 0;	// cut off skybox_ext
	return true;
}

/*
=============
Sky_InitWorldspawn (JDH)

called on map load, if "sky" key is found
=============
*/
qboolean Sky_InitWorldspawn (const char *value)
{
	if (no24bit)
		return false;

	Cvar_SetDirect (&r_skybox, value);
	return r_skyboxloaded;
}


static	vec3_t	skyclip[6] = {
	{1, 1, 0},
	{1, -1, 0},
	{0, -1, 1},
	{0, 1, 1},
	{1, 0, 1},
	{-1, 0, 1}
};

// 1 = s, 2 = t, 3 = 2048
static	int	st_to_vec[6][3] = {
	{3, -1, 2},
	{-3, 1, 2},

	{1, 3, 2},
	{-1, -3, 2},

	{-2, -1, 3},		// 0 degrees yaw, look straight up
	{2, -1, -3}		// look straight down
};

// s = [0]/[2], t = [1]/[2]
static	int	vec_to_st[6][3] = {
	{-2, 3, 1},
	{2, 3, -1},

	{1, 3, 2},
	{-1, 3, -2},

	{-2, -1, 3},
	{-2, 1, -3}
};

static	float	skymins[2][6], skymaxs[2][6];

void DrawSkyPolygon (int nump, vec3_t vecs)
{
	int	i, j, axis;
	vec3_t	v, av;
	float	s, t, dv, *vp;

	// decide which face it maps to
	VectorClear (v);
	for (i = 0, vp = vecs ; i < nump ; i++, vp += 3)
		VectorAdd (vp, v, v);

	av[0] = fabs(v[0]);
	av[1] = fabs(v[1]);
	av[2] = fabs(v[2]);

	if (av[0] > av[1] && av[0] > av[2])
		axis = (v[0] < 0) ? 1 : 0;
	else if (av[1] > av[2] && av[1] > av[0])
		axis = (v[1] < 0) ? 3 : 2;
	else
		axis = (v[2] < 0) ? 5 : 4;

	// project new texture coords
	for (i=0 ; i<nump ; i++, vecs+=3)
	{
		j = vec_to_st[axis][2];
		dv = (j > 0) ? vecs[j - 1] : -vecs[-j - 1];

		j = vec_to_st[axis][0];
		s = (j < 0) ? -vecs[-j -1] / dv : vecs[j-1] / dv;

		j = vec_to_st[axis][1];
		t = (j < 0) ? -vecs[-j -1] / dv : vecs[j-1] / dv;

		if (s < skymins[0][axis])
			skymins[0][axis] = s;
		if (t < skymins[1][axis])
			skymins[1][axis] = t;
		if (s > skymaxs[0][axis])
			skymaxs[0][axis] = s;
		if (t > skymaxs[1][axis])
			skymaxs[1][axis] = t;
	}
}

#define	MAX_CLIP_VERTS	64
void ClipSkyPolygon (int nump, vec3_t vecs, int stage)
{
	float		*norm, *v, d, e, dists[MAX_CLIP_VERTS];
	qboolean	front, back;
	int		sides[MAX_CLIP_VERTS], newc[2], i, j;
	vec3_t		newv[2][MAX_CLIP_VERTS];

	if (nump > MAX_CLIP_VERTS-2)
		Sys_Error ("ClipSkyPolygon: nump > MAX_CLIP_VERTS - 2");

	if (stage == 6)
	{	// fully clipped, so draw it
		DrawSkyPolygon (nump, vecs);
		return;
	}

	front = back = false;
	norm = skyclip[stage];
	for (i = 0, v = vecs ; i < nump ; i++, v += 3)
	{
		d = DotProduct (v, norm);
		if (d > ON_EPSILON)
		{
			front = true;
			sides[i] = SIDE_FRONT;
		}
		else if (d < -ON_EPSILON)
		{
			back = true;
			sides[i] = SIDE_BACK;
		}
		else
			sides[i] = SIDE_ON;
		dists[i] = d;
	}

	if (!front || !back)
	{	// not clipped
		ClipSkyPolygon (nump, vecs, stage+1);
		return;
	}

	// clip it
	sides[i] = sides[0];
	dists[i] = dists[0];
	VectorCopy (vecs, (vecs + (i*3)));
	newc[0] = newc[1] = 0;

	for (i=0, v=vecs ; i<nump ; i++, v+=3)
	{
		switch (sides[i])
		{
		case SIDE_FRONT:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			break;

		case SIDE_BACK:
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;

		case SIDE_ON:
			VectorCopy (v, newv[0][newc[0]]);
			newc[0]++;
			VectorCopy (v, newv[1][newc[1]]);
			newc[1]++;
			break;
		}

		if (sides[i] == SIDE_ON || sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

		d = dists[i] / (dists[i] - dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{
			e = v[j] + d*(v[j+3] - v[j]);
			newv[0][newc[0]][j] = e;
			newv[1][newc[1]][j] = e;
		}
		newc[0]++;
		newc[1]++;
	}

	// continue
	ClipSkyPolygon (newc[0], newv[0][0], stage+1);
	ClipSkyPolygon (newc[1], newv[1][0], stage+1);
}

/*
=================
R_AddSkyBoxSurface
=================
*/
void R_AddSkyBoxSurface (msurface_t *fa)
{
	int		i;
	vec3_t		verts[MAX_CLIP_VERTS];
	glpoly_t	*p;

	// calculate vertex values for sky box
	for (p = fa->polys ; p ; p = p->next)
	{
		for (i=0 ; i<p->numverts ; i++)
			VectorSubtract (p->verts[i], r_origin, verts[i]);
		ClipSkyPolygon (p->numverts, verts[0], 0);
	}
}

/*
==============
R_ClearSkyBox
==============
*/
void R_ClearSkyBox (void)
{
	int	i;

	for (i=0 ; i<6 ; i++)
	{
		skymins[0][i] = skymins[1][i] = 9999;
		skymaxs[0][i] = skymaxs[1][i] = -9999;
	}
}

void MakeSkyVec (float s, float t, int axis)
{
	vec3_t	v, b;
	int		j, k, farclip;

	if (/*nehahra &&*/ Fog_GetDensity ())
		farclip = gl_skyclip.value;
	else
		farclip = (int) r_farclip.value;

	farclip = bound (4096, farclip, 16384);

	b[0] = s * (farclip >> 1);
	b[1] = t * (farclip >> 1);
	b[2] = (farclip >> 1);

//	Fitz uses this:
/*	float dist = r_farclip.value / sqrt(3.0);
	b[0] = s * dist;
	b[1] = t * dist;
	b[2] = dist;
*/
	for (j=0 ; j<3 ; j++)
	{
		k = st_to_vec[axis][j];
		v[j] = (k < 0) ? -b[-k-1] : b[k-1];
		v[j] += r_origin[j];
	}

	// avoid bilerp seam
	s = (s+1) * 0.5;
	t = (t+1) * 0.5;

	j = skybox_widths[axis];
	k = skybox_heights[axis];

	if (s < 1.0/j)
		s = 1.0 / j;
	else if (s > (float)(j-1)/j)
		s = (float)(j-1) / j;

	if (t < 1.0/k)
		t = 1.0 / k;
	else if (t > (float)(k-1)/k)
		t = (float)(k-1) / k;

//	Fitz uses this:
/*	s = s * (float)(j-1)/j + 0.5/j;
	t = t * (float)(k-1)/k + 0.5/k;
*/
	t = 1.0 - t;
	glTexCoord2f (s, t);
	glVertex3fv (v);
}

/*
==============
R_DrawSkyBox
==============
*/
static	int	skytexorder[6] = {0, 2, 1, 3, 4, 5};
void R_DrawSkyBox (void)
{
	int		i;
	msurface_t	*fa;

//	if (!skychain)
//		return;

	R_ClearSkyBox ();

	for (fa = skychain ; fa ; fa = fa->texturechain)
	{
	#ifdef FITZWORLD
		if (!r_fastworld.value || !fa->culled)
	#endif
			R_AddSkyBoxSurface (fa);
	}

	GL_DisableMultitexture ();

	for (i=0 ; i<6 ; i++)
	{
		if (skymins[0][i] >= skymaxs[0][i] || skymins[1][i] >= skymaxs[1][i])
			continue;

		GL_Bind (skyboxtextures + skytexorder[i]);

		glBegin (GL_QUADS);
		MakeSkyVec (skymins[0][i], skymins[1][i], i);
		MakeSkyVec (skymins[0][i], skymaxs[1][i], i);
		MakeSkyVec (skymaxs[0][i], skymaxs[1][i], i);
		MakeSkyVec (skymaxs[0][i], skymins[1][i], i);
		glEnd ();
	}


// this pass sets the depth buffer properly:
	if (gl_skyhack.value < 1)
	{
		glDisable (GL_TEXTURE_2D);
		glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//glEnable (GL_BLEND);
		//glBlendFunc (GL_ZERO, GL_ONE);

		for (fa = skychain ; fa ; fa = fa->texturechain)
		{
		#ifdef FITZWORLD
			if (!r_fastworld.value || !fa->culled)
		#endif
				EmitFlatPoly (fa);
		}

		glEnable (GL_TEXTURE_2D);
		glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//glDisable (GL_BLEND);
		//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

//	skychain = NULL;
//	skychain_tail = &skychain;
}

void R_DrawSky (void)
{
#ifdef HEXEN2_SUPPORT
	if (!hexen2 || !r_skyboxloaded || (r_skyboxloaded && r_oldsky.value))
#endif
	if (!skychain)
		return;

	if (gl_skyhack.value >= 0)		// this check will be removed once old code is purged
	{
	// JDH: scene renders MUCH faster if sky surfs are excluded from depth checks via glDepthMask(FALSE)
	//      (this doesn't work with gl_ztrick, so that cvar is history)

		if (r_skyboxloaded && !r_oldsky.value)
		{
			if (gl_skyhack.value)
				glDepthMask (GL_FALSE);
		}
		else
		{
			if (r_skytype.value == 0)
			{
				if (gl_skyhack.value <= 1)
				{
					R_DrawSolidSkyChain ();		// fast-draw the surfs to initialize the depth buffer
					glDepthFunc (GL_GEQUAL);
				}
			}
			glDepthMask (GL_FALSE);
		}
	}


	if (r_skyboxloaded && !r_oldsky.value)
	{
		R_DrawSkyBox ();
	}
	else if (r_skytype.value == 0)
	{
		R_DrawSkyChain ();
	}
	else
		R_DrawSkyChainOld ();


	if (gl_skyhack.value >= 0)
	{
		glDepthFunc (GL_LEQUAL);
		glDepthMask (GL_TRUE);
	}
}

#endif		//#ifndef RQM_SV_ONLY
