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
// r_light.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

int	r_dlightframecount;

extern cvar_t r_flatlightstyles;

/*
==================
R_AnimateLight
==================
*/
void R_AnimateLight (void)
{
	int	defaultLocus = (int)(cl.time * 10);
	int j, k;
	
#ifdef HEXEN2_SUPPORT
	int locusHz[3];

	if (hexen2)
	{
		locusHz[0] = defaultLocus;
		locusHz[1] = (int)(cl.time*20);
		locusHz[2] = (int)(cl.time*30);
	}
#endif
	
// light animations
// 'm' is normal light, 'a' is no light, 'z' is double bright
	for (j=0 ; j<MAX_LIGHTSTYLES ; j++)
	{
		if (!cl_lightstyle[j].length)
		{
			d_lightstylevalue[j] = 256;
			continue;
		}

		// JDH: r_flatlightstyles (from Fitz)
		if (r_flatlightstyles.value == 2)
		{
			k = cl_lightstyle[j].peak - 'a';
		}
		else if (r_flatlightstyles.value == 1)
		{
			k = cl_lightstyle[j].average - 'a';
		}
		else
		{
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				k = cl_lightstyle[j].map[0];
				if (k == '1' || k == '2' || k == '3')		// Explicit anim rate
				{ 
					if (cl_lightstyle[j].length == 1)
					{ 
						d_lightstylevalue[j] = 256;
						continue;					// Bad style def
					}

					k = locusHz[k-'1'] % (cl_lightstyle[j].length-1);
					k = cl_lightstyle[j].map[k+1] - 'a';
					d_lightstylevalue[j] = k*22;
					continue;
				}
			}
		#endif

			// Default anim rate (10 Hz)
			k = defaultLocus % cl_lightstyle[j].length;		// current index
			k = cl_lightstyle[j].map[k] - 'a';		// current level
		}

		d_lightstylevalue[j] = k*22;		// 0 to 550
	}	
}

/*
=============================================================================

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/

float	bubble_sintable[17], bubble_costable[17];

void R_InitBubble (void)
{
	int	i;
	float	a, *bub_sin, *bub_cos;

	bub_sin = bubble_sintable;
	bub_cos = bubble_costable;

	for (i=16 ; i>=0 ; i--)
	{
		a = i/16.0 * M_PI*2;
		*bub_sin++ = sin(a);
		*bub_cos++ = cos(a);
	}
}

float bubblecolor[NUM_DLIGHTTYPES][3] = 
{
	{0.2,  0.1,  0.05},		// dimlight or brightlight (lt_default)
	{0.2,  0.1,  0.05},		// muzzleflash
	{0.2,  0.1,  0.05},		// explosion
	{0.2,  0.1,  0.05},		// rocket
	{0.5,  0.05, 0.05},		// red
	{0.05, 0.05, 0.3},		// blue
	{0.5,  0.05, 0.4}		// red + blue
};

void R_RenderDlight (dlight_t *light)
{
	int	i, j;
	vec3_t	v, v_right, v_up;
	float	length, rad, *bub_sin, *bub_cos;

// don't draw our own powerup glow - OUTDATED: see below, why
//	if (light->key == cl.viewentity)
//		return;

	rad = light->radius * 0.35;
	VectorSubtract (light->origin, r_origin, v);
	length = VectorNormalize (v);

	if (length < rad)
	{
		// view is inside the dlight
// joe: this looks ugly, so I decided NOT TO use it...
//		V_AddLightBlend (1, 0.5, 0, light->radius * 0.0003);
		return;
	}

	glBegin (GL_TRIANGLE_FAN);
#if 0
	if (light->type == lt_explosion2 || light->type == lt_explosion3)
		glColor3fv (ExploColor);
	else
	{
//		vec3_t color;
		glColor3fv (bubblecolor[light->type]);
//		VectorScale (dlightcolor[light->type], 1.0/255.0, color);
//		glColor3fv (color);
	}
#else
	glColor3fv (light->color);
#endif

	VectorVectors(v, v_right, v_up);

	if (length - rad > 8)
		VectorScale (v, rad, v);
	else
	// make sure the light bubble will not be clipped by near z clip plane
		VectorScale (v, length - 8, v);

	VectorSubtract (light->origin, v, v);

	glVertex3fv (v);
	glColor3f (0, 0, 0);

	bub_sin = bubble_sintable;
	bub_cos = bubble_costable;

	for (i=16; i>=0; i--)
	{
		for (j=0 ; j<3 ; j++)
			v[j] = light->origin[j] + (v_right[j]*(*bub_cos) + v_up[j]*(*bub_sin)) * rad;
		bub_sin++; 
		bub_cos++;
		glVertex3fv (v);
	}

	glEnd ();
}

/*
=============
R_RenderDlights
=============
*/
void R_RenderDlights (void)
{
	int		i;
	dlight_t	*l;

	if (!gl_flashblend.value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
						// advanced yet for this frame
	glDepthMask (GL_FALSE);
	glDisable (GL_TEXTURE_2D);
	glShadeModel (GL_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE);

	l = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (DLIGHT_INACTIVE(l) || !l->radius)
			continue;
		R_RenderDlight (l);
	}

	glColor3f (1, 1, 1);
	glDisable (GL_BLEND);
	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (GL_TRUE);
}

/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
/*
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	int		i;

	if (node->contents < 0)
		return;

	splitplane = node->plane;
	dist = PlaneDiff(light->origin, splitplane);

	if (dist > light->radius)
	{
		R_MarkLights (light, bit, node->children[0]);
		return;
	}
	if (dist < -light->radius)
	{
		R_MarkLights (light, bit, node->children[1]);
		return;
	}

// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->dlightframe != r_dlightframecount)
		{
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
	}

	R_MarkLights (light, bit, node->children[0]);
	R_MarkLights (light, bit, node->children[1]);
}
*/

// joe: this is said to be faster, so I use it instead
void R_MarkLights (dlight_t *light, QINT64 bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist, l, maxdist;
	msurface_t	*surf;
	int		i, j, s, t, sidebit;
	vec3_t		impact;

loc0:
	if (node->contents < 0)
		return;

	splitplane = node->plane;
	dist = PlaneDiff(light->origin, splitplane);
	
	if (dist > light->radius)
	{
		node = node->children[0];
		goto loc0;
	}
	if (dist < -light->radius)
	{
		node = node->children[1];
		goto loc0;
	}

	maxdist = light->radius * light->radius;
// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
#ifdef WAITFORTEXPTR
	// JDH: this allows lights to shine through bright surfaces (eg. hipnotic start map),
	//      but not through dark ones (eg. id1 start)
	//	** ENABLE ONCE THERE'S A FASTER WAY TO GET TEXTURE PTR - needs GL_Upload32 code too **

		j = GL_GetTextureColor (surf->texinfo->texture->gl_texturenum);
		j = ((j & 0x000000FF) + ((j & 0x0000FF00) >> 8) + ((j & 0x00FF0000) >> 16)) / 3;
		if (j < 100)
#endif
		{
			dist = DotProduct (light->origin, surf->plane->normal) - surf->plane->dist;		// JT030305 - fix light bleed through
			if (dist >= 0)
				sidebit = 0;
			else
				sidebit = SURF_PLANEBACK;

			if ( (surf->flags & SURF_PLANEBACK) != sidebit )				//Discoloda
				continue;								//Discoloda
		}

		for (j=0 ; j<3 ; j++)
			impact[j] = light->origin[j] - surf->plane->normal[j]*dist;

		// clamp center of light to corner and check brightness
		l = DotProduct(impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l + 0.5;
		s = bound(0, s, surf->extents[0]);
		s = l - s;
		l = DotProduct(impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l + 0.5;
		t = bound(0, t, surf->extents[1]);
		t = l - t;

		// compare to minimum light
		if ((s*s + t*t + dist*dist) < maxdist)
		{
			if (surf->dlightframe != r_dlightframecount)	// not dynamic until now
			{
				surf->dlightbits = bit;
				surf->dlightframe = r_dlightframecount;
			}
			else	// already dynamic
			{
				surf->dlightbits |= bit;
			}
		}
	}
	if (node->children[0]->contents >= 0)
		R_MarkLights (light, bit, node->children[0]);
	if (node->children[1]->contents >= 0)
		R_MarkLights (light, bit, node->children[1]);
}

/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int			i;
	dlight_t	*l;

	if (gl_flashblend.value)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
											// advanced yet for this frame
	l = cl_dlights;

	for (i=0 ; i<MAX_DLIGHTS ; i++, l++)
	{
		if (DLIGHT_INACTIVE(l) || !l->radius)
			continue;
//JDH	R_MarkLights (l, 1<<i, cl.worldmodel->nodes);
		R_MarkLights (l, ((QINT64) 1)<<i, cl.worldmodel->nodes);
	}
}

/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

//mplane_t	*lightplane;
vec3_t		lightspot, lightcolor;

qboolean RecursiveLightPoint (vec3_t color, const mnode_t *node, const vec3_t start, const vec3_t end)
{
	mplane_t	*plane;
	float		front, back, frac;
	vec3_t		mid;

loc0:
	if (node->contents < 0)
		return false;		// didn't hit anything
	
// calculate mid point
	plane = node->plane;
	if (plane->type < 3)
	{
		front = start[plane->type] - plane->dist;
		back = end[plane->type] - plane->dist;
	}
	else
	{
		front = DotProduct(start, plane->normal) - plane->dist;
		back = DotProduct(end, plane->normal) - plane->dist;
	}

	// optimized recursion
	if ((back < 0) == (front < 0))
	{
		node = node->children[front < 0];
		goto loc0;
	}
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0]) * frac;
	mid[1] = start[1] + (end[1] - start[1]) * frac;
	mid[2] = start[2] + (end[2] - start[2]) * frac;
	
// go down front side
	if (RecursiveLightPoint(color, node->children[front < 0], start, mid))
	{
		return true;	// hit something
	}
	else
	{
		int		depth, i, ds, dt;
		msurface_t	*surf;

	// check for impact on this node
		VectorCopy (mid, lightspot);
		//lightplane = node->plane;

		surf = cl.worldmodel->surfaces + node->firstsurface;
		depth = cl.worldmodel->lightdatadepth;

		for (i = 0 ; i < node->numsurfaces ; i++, surf++)
		{
			if (surf->flags & SURF_DRAWTILED)
				continue;	// no lightmaps

			ds = (int)((float)DotProduct (mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
			dt = (int)((float)DotProduct (mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);

			if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
				continue;
			
			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];
			
			if (ds > surf->extents[0] || dt > surf->extents[1])
				continue;

			if (surf->samples)
			{
				// enhanced to interpolate lighting
				byte	*lightmap;
				int		smax, tmax, maps, line, val;
				int		dsfrac = ds & 15, dtfrac = dt & 15;
				int		r00 = 0, g00 = 0, b00 = 0;
				int		r01 = 0, g01 = 0, b01 = 0;
				int		r10 = 0, g10 = 0, b10 = 0;
				int		r11 = 0, g11 = 0, b11 = 0;
				float	scale;

				smax = (surf->extents[0] >> 4) + 1;
				tmax = (surf->extents[1] >> 4) + 1;
				
				//line = ((surf->extents[0] >> 4) + 1) * 3;
				//lightmap = surf->samples + ((dt>>4) * ((surf->extents[0]>>4)+1) + (ds>>4))*3; // LordHavoc: *3 for color
				line = smax * depth;
				lightmap = surf->samples + ((dt>>4) * smax + (ds>>4))*depth;

				for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ; maps++)
				{
					scale = (float)d_lightstylevalue[surf->styles[maps]] / 256.0;
					if (depth == 3)		// JDH
					{
						r00 += (float)lightmap[0] * scale;
						g00 += (float)lightmap[1] * scale;
						b00 += (float)lightmap[2] * scale;

						r01 += (float)lightmap[3] * scale;
						g01 += (float)lightmap[4] * scale;
						b01 += (float)lightmap[5] * scale;

						r10 += (float)lightmap[line+0] * scale;
						g10 += (float)lightmap[line+1] * scale;
						b10 += (float)lightmap[line+2] * scale;

						r11 += (float)lightmap[line+3] * scale;
						g11 += (float)lightmap[line+4] * scale;
						b11 += (float)lightmap[line+5] * scale;
					}
					else
					{
						val = (float)lightmap[0] * scale;
						r00 += val;
						g00 += val;
						b00 += val;

						val = (float)lightmap[1] * scale;
						r01 += val;
						g01 += val;
						b01 += val;

						val = (float)lightmap[line+0] * scale;
						r10 += val;
						g10 += val;
						b10 += val;

						val = (float)lightmap[line+1] * scale;
						r11 += val;
						g11 += val;
						b11 += val;
					}

					//lightmap += ((surf->extents[0] >> 4) + 1) * ((surf->extents[1] >> 4) +1 ) * 3; // LordHavoc: *3 for colored lighting
					lightmap += smax * tmax * depth;
				}
				
				color[0] += (float)((int)((((((((r11 - r10) * dsfrac) >> 4) + r10) 
					- ((((r01 - r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) 
					+ ((((r01 - r00) * dsfrac) >> 4) + r00)));
				
				color[1] += (float)((int)((((((((g11 - g10) * dsfrac) >> 4) + g10) 
					- ((((g01 - g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) 
					+ ((((g01 - g00) * dsfrac) >> 4) + g00)));
				
				color[2] += (float)((int)((((((((b11 - b10) * dsfrac) >> 4) + b10) 
					- ((((b01 - b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) 
					+ ((((b01 - b00) * dsfrac) >> 4) + b00)));
			}

			return true;	// success
		}

	// go down back side
		return RecursiveLightPoint (color, node->children[front >= 0], mid, end);
	}
}

int R_LightPoint (const vec3_t p)
{
	vec3_t	end;
	
	if (r_fullbright.value || !cl.worldmodel->lightdata)
	{
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 255;
		return 255;
	}
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	
	VectorClear (lightcolor);
	RecursiveLightPoint (lightcolor, cl.worldmodel->nodes, p, end);

	return (lightcolor[0] + lightcolor[1] + lightcolor[2]) / 3.0;
}

/*
=============================================================================

VERTEX LIGHTING

=============================================================================
*/

float	vlight_pitch = 45;
float	vlight_yaw = 45;
float	vlight_highcut = 128;
float	vlight_lowcut = 60;

#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];

byte	anorm_pitch[NUMVERTEXNORMALS];
byte	anorm_yaw[NUMVERTEXNORMALS];

byte	vlighttable[256][256];

float R_GetVertexLightValue (int index, float apitch, float ayaw)
{
	int	pitchofs, yawofs;
	float	retval;

	pitchofs = anorm_pitch[index] + (apitch * 256 / 360);
	yawofs = anorm_yaw[index] + (ayaw * 256 / 360);
	while (pitchofs > 255)
		pitchofs -= 256;
	while (yawofs > 255)
		yawofs -= 256;
	while (pitchofs < 0)
		pitchofs += 256;
	while (yawofs < 0)
		yawofs += 256;

	retval = vlighttable[pitchofs][yawofs];

	return retval / 256;
}

float R_LerpVertexLight (int index1, int index2, float ilerp, float apitch, float ayaw)
{
	float	lightval1, lightval2, val;

	lightval1 = R_GetVertexLightValue (index1, apitch, ayaw);
	lightval2 = R_GetVertexLightValue (index2, apitch, ayaw);

	val = (lightval2*ilerp) + (lightval1*(1-ilerp));

	return val;
}

void R_ResetAnormTable (void)
{
	int	i, j;
	float	forward, yaw, pitch, angle, sp, sy, cp, cy, precut;
	vec3_t	normal, lightvec;

	// Define the light vector here
	angle = DEG2RAD(vlight_pitch);
	sy = sin(angle);
	cy = cos(angle);
	angle = DEG2RAD(-vlight_yaw);
	sp = sin(angle);
	cp = cos(angle);
	lightvec[0] = cp*cy;
	lightvec[1] = cp*sy;
	lightvec[2] = -sp;

	// First thing that needs to be done is the conversion of the
	// anorm table into a pitch/yaw table

	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		if (r_avertexnormals[i][1] == 0 && r_avertexnormals[i][0] == 0)
		{
			yaw = 0;
			if (r_avertexnormals[i][2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int)(atan2(r_avertexnormals[i][1], r_avertexnormals[i][0]) * 57.295779513082320);
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt(r_avertexnormals[i][0]*r_avertexnormals[i][0] + r_avertexnormals[i][1]*r_avertexnormals[i][1]);
			pitch = (int)(atan2(r_avertexnormals[i][2], forward) * 57.295779513082320);
			if (pitch < 0)
				pitch += 360;
		}
		anorm_pitch[i] = pitch * 256 / 360;
		anorm_yaw[i] = yaw * 256 / 360;
	}

	// Next, a light value table must be constructed for pitch/yaw offsets
	// DotProduct values

	// DotProduct values never go higher than 2, so store bytes as
	// (product * 127.5)

	for (i=0 ; i<256 ; i++)
	{
		angle = DEG2RAD(i * 360 / 256);
		sy = sin(angle);
		cy = cos(angle);
		for (j=0 ; j<256 ; j++)
		{
			angle = DEG2RAD(j * 360 / 256);
			sp = sin(angle);
			cp = cos(angle);

			normal[0] = cp*cy;
			normal[1] = cp*sy;
			normal[2] = -sp;

			precut = ((DotProduct(normal, lightvec) + 2) * 31.5);
			precut = (precut - (vlight_lowcut)) * 256 / (vlight_highcut - vlight_lowcut);
			if (precut > 255)
				precut = 255;
			if (precut < 0)
				precut = 0;
			vlighttable[i][j] = precut;
		}
	}
}

void R_InitVertexLights (void)
{
	R_ResetAnormTable ();
}

#endif		//#ifndef RQM_SV_ONLY
