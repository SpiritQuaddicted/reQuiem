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
// gl_warp.c -- water and other turbulent texture polygons

#include "quakedef.h"

#ifndef RQM_SV_ONLY

extern cvar_t gl_subdivide_size;

static	msurface_t	*warpface;

void BoundPoly (int numverts, float *verts, vec3_t mins, vec3_t maxs)
{
	int	i, j;
	float	*v;

	mins[0] = mins[1] = mins[2] = 9999;
	maxs[0] = maxs[1] = maxs[2] = -9999;
	v = verts;
	for (i=0 ; i<numverts ; i++)
	{
		for (j=0 ; j<3 ; j++, v++)
		{
			if (*v < mins[j])
				mins[j] = *v;
			if (*v > maxs[j])
				maxs[j] = *v;
		}
	}
}

void SubdividePolygon (int numverts, float *verts)
{
	int		i, j, k, f, b;
	vec3_t		mins, maxs, front[64], back[64];
	float		m, *v, dist[64], frac, s, t, subdivide_size;
	glpoly_t	*poly;

	if (numverts > 60)
		Sys_Error ("numverts = %i", numverts);

	subdivide_size = max(1, gl_subdivide_size.value);
	BoundPoly (numverts, verts, mins, maxs);

	for (i=0 ; i<3 ; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5;
		m = subdivide_size * floor (m/subdivide_size + 0.5);
		if (maxs[i] - m < 8)
			continue;
		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;
		for (j=0 ; j<numverts ; j++, v+=3)
			dist[j] = *v - m;

		// wrap cases
		dist[j] = dist[0];
		v-=i;
		VectorCopy (verts, v);

		f = b = 0;
		v = verts;
		for (j=0 ; j<numverts ; j++, v+=3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy (v, front[f]);
				f++;
			}
			if (dist[j] <= 0)
			{
				VectorCopy (v, back[b]);
				b++;
			}
			if (dist[j] == 0 || dist[j+1] == 0)
				continue;
			if ((dist[j] > 0) != (dist[j+1] > 0))
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);
				for (k=0 ; k<3 ; k++)
					front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);
				f++;
				b++;
			}
		}

		SubdividePolygon (f, front[0]);
		SubdividePolygon (b, back[0]);
		return;
	}

	poly = Hunk_Alloc (sizeof(glpoly_t) + (numverts - 4) * VERTEXSIZE * sizeof(float));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts;
	for (i = 0 ; i < numverts ; i++, verts += 3)
	{
		VectorCopy (verts, poly->verts[i]);
		s = DotProduct (verts, warpface->texinfo->vecs[0]);
		t = DotProduct (verts, warpface->texinfo->vecs[1]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;
	}
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface (model_t *mod, msurface_t *fa)
{
	vec3_t		verts[64];
	int			numverts, i, lindex;
	float		*vec;

	warpface = fa;

	// convert edges back to a normal polygon
	numverts = 0;
	for (i=0 ; i<fa->numedges ; i++)
	{
		lindex = mod->surfedges[fa->firstedge + i];

		if (lindex > 0)
			vec = mod->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = mod->vertexes[loadmodel->edges[-lindex].v[1]].position;
		VectorCopy (vec, verts[numverts]);
		numverts++;
	}

// JDH: I first build the un-subdivided poly, because this draws a bit faster.
//      It's used when drawing the sky as a solid color, or doing the invisible
//		pass for skybox & MH sky (the pass that sets the depth properly).
//		I save it to the "samples" field, since sky surfs have no lightmaps.
	
#ifdef _DEBUG
	if (fa->samples)
		i = 235;
#endif
	
	GL_BuildSurfaceDisplayList (mod, fa);
	fa->samples = (byte *) fa->polys;
	fa->polys = NULL;

	SubdividePolygon (numverts, verts[0]);
}

//=========================================================

#define	TURBSINSIZE	128
#define	TURBSCALE	((float)TURBSINSIZE / (2 * M_PI))

byte turbsin[TURBSINSIZE] =
{
	127, 133, 139, 146, 152, 158, 164, 170, 176, 182, 187, 193, 198, 203, 208, 213, 
	217, 221, 226, 229, 233, 236, 239, 242, 245, 247, 249, 251, 252, 253, 254, 254, 
	255, 254, 254, 253, 252, 251, 249, 247, 245, 242, 239, 236, 233, 229, 226, 221, 
	217, 213, 208, 203, 198, 193, 187, 182, 176, 170, 164, 158, 152, 146, 139, 133, 
	127, 121, 115, 108, 102, 96, 90, 84, 78, 72, 67, 61, 56, 51, 46, 41, 
	37, 33, 28, 25, 21, 18, 15, 12, 9, 7, 5, 3, 2, 1, 0, 0, 
	0, 0, 0, 1, 2, 3, 5, 7, 9, 12, 15, 18, 21, 25, 28, 33, 
	37, 41, 46, 51, 56, 61, 67, 72, 78, 84, 90, 96, 102, 108, 115, 121, 
};

__inline static float SINTABLE_APPROX (float time)
{
	float	sinlerpf, lerptime, lerp;
	int	sinlerp1, sinlerp2;

	sinlerpf = time * TURBSCALE;
	sinlerp1 = floor(sinlerpf);
	sinlerp2 = sinlerp1 + 1;
	lerptime = sinlerpf - sinlerp1;

	lerp =	turbsin[sinlerp1 & (TURBSINSIZE - 1)] * (1 - lerptime) + turbsin[sinlerp2 & (TURBSINSIZE - 1)] * lerptime;
	return -8 + 16 * lerp / 255.0;
}


/*
=============
EmitWaterPolys

Does a water warp on the pre-fragmented glpoly_t chain
=============
*/
/*
void EmitWaterPolys (msurface_t *fa)
{
	glpoly_t	*p;
	float		*v, s, t, os, ot;
	int		i;

	GL_DisableMultitexture ();

	GL_Bind (fa->texinfo->texture->gl_texturenum);
	for (p = fa->polys ; p ; p = p->next)
	{
		glBegin (GL_POLYGON);
		for (i = 0, v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			s = os + SINTABLE_APPROX(ot * 0.125 + cl.time);
			s *= (1.0 / 64);

			t = ot + SINTABLE_APPROX(os * 0.125 + cl.time);
			t *= (1.0 / 64);

			glTexCoord2f (s, t);
			glVertex3fv (v);
		}
		glEnd ();
	}
}
*/

/*
void EmitWaterPolys (msurface_t *fa)
{
        glpoly_t        *p;
        float                *v, s, t, os, ot;
        int                i;

        GL_DisableMultitexture ();

        GL_Bind (fa->texinfo->texture->gl_texturenum);
        for (p = fa->polys ; p ; p = p->next)
        {
                glBegin (GL_POLYGON);
                for (i = 0, v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
                {
                        os = v[3];
                        ot = v[4];
                        if (strstr(fa->texinfo->texture->name, "lava") || 
                                strstr (fa->texinfo->texture->name, "slime"))
                        {
                                s = os + SINTABLE_APPROX(ot * 0.115 + cl.time);
                                s *= (1.0 / 64);

                                t = ot + SINTABLE_APPROX(os * 0.115 + cl.time);
                                t *= (1.0 / 64);
                        }
                        else
                        {
                                s = os + SINTABLE_APPROX(ot * 0.035 + cl.time);
                                s *= (1.0 / 64);

                                t = ot + SINTABLE_APPROX(os * 0.035 + cl.time);
                                t *= (1.0 / 64);
                                
                        }
                        glTexCoord2f (s, t);
                        glVertex3fv (v);
                }
                glEnd ();
        }
}
*/

//extern void QMB_LiquidEffect (vec3_t org, float size, float time, int type);

void EmitWaterPolys (msurface_t *fa, qboolean doSetup) // QMB code, adapted and edited by Entar for Tremor on 3-3-05
{
	glpoly_t *p;
	float *v;
	int i;
	float s, t, os, ot;
	vec3_t nv; //qmb :water wave

	if (doSetup)
	{
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_BLEND);
		glColor4f (1, 1, 1, r_wateralpha.value);
	}

	for (p=fa->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			s = os + SINTABLE_APPROX(ot * 0.115 + cl.time);
			s *= (1.0 / 64);

			t = ot + SINTABLE_APPROX(os * 0.115 + cl.time);
			t *= (1.0 / 64);

			glTexCoord2f (s, t);

		// JDH: from nehahra:
			if (r_waterripple.value) 
			{
				nv[0] = v[0];
				nv[1] = v[1];
				nv[2] = v[2] + r_waterripple.value*sin(v[0]*0.05+realtime*3)*sin(v[2]*0.05+realtime*3);
				glVertex3fv (nv);
			}
			else glVertex3fv (v);
		}
		glEnd ();
	}
/**************** JDH ***************

	glColor4f(1, 1, 1, r_wateralpha.value);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (strstr(fa->texinfo->texture->name, "tele"))
	{

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glColor4f(1, 1, 1, r_wateralpha.value*0.5);

		for (p=fa->polys ; p; p=p->next)
		{
			glBegin (GL_POLYGON);
			for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
			{
				os = v[3];
				ot = v[4];

				s = os + turbsin[(int)((ot*0.25+(cl.time*2)) * TURBSCALE) & 255];
				s *= (0.5/64)*(-1);

				t = ot + turbsin[(int)((os*0.25+(cl.time*2)) * TURBSCALE) & 255];
				t *= (0.5/64)*(-1);

				VectorCopy(v, nv);
				
				//qmb :water wave
				//if (r_wave.value)
					nv[2] = v[2] + r_wave.value *sin(v[0]*0.02+cl.time)*sin(v[1]*0.02+cl.time)*sin(v[2]*0.02+cl.time);
				//
				glTexCoord2f (s, t);
				glVertex3fv (nv);
			}
			glEnd ();
		}

	}
**************** JDH ***************/

	//if (strstr(fa->texinfo->texture->name, "lava"))
	//	QMB_LiquidEffect (nv, 12, 0.8, 1);

	if (doSetup)
	{
		glDisable (GL_BLEND);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor4f (1, 1, 1, 1);
	}
}

/*
void EmitWaterPolys2 (msurface_t *fa)
{
	glpoly_t *p, *bp;
	float *v;
	int i;
	float s, t;
	float os, ot;
	float scroll;
	float rdt = r_refdef.time;

	glEnable(GL_BLEND);
	glColor4f (1,1,1,0.25);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glShadeModel(GL_SMOOTH);

	glNormal3fv(fa->plane->normal);

	scroll = 0;

	for (bp=fa->polys ; bp ; bp=bp->next) {
		p = bp;
		glBegin (GL_TRIANGLE_FAN);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE) {       

			os = v[3];
			ot = v[4];

			s = os + turbsin[(int)((ot*0.25+(cl.time*2)) * TURBSCALE) & 255];
			s *= (0.5/64)*(-1);

			t = ot + turbsin[(int)((os*0.25+(cl.time*2)) * TURBSCALE) & 255];
			t *= (0.5/64)*(-1);

			s += scroll;
			s *= 0.015625; // divide by empatpuluh enam
			t *= 0.015625; // ditto     
   
     
			glTexCoord2f (s,t);
			glVertex3fv (v);
		}

		glEnd ();
	}
}
*/

void EmitWaterPolysReflection (msurface_t *fa) // Thanks reckless
{
#ifdef SHINYWATER
	extern cvar_t gl_shinywater;
	glpoly_t	*p;
	float		*v;
	int			i;
	float		os, ot;
//	float		color[4];
	vec3_t		nv;
	vec3_t		xv;

	if (!chrometexture2 || !gl_shinywater.value || strstr(fa->texinfo->texture->name, "lava"))
		return;
	
	glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glEnable (GL_TEXTURE_GEN_S);
	glEnable (GL_TEXTURE_GEN_T);

	//    if (r_wateralpha.value == 1)  // changed by entar - fix for transparent water
	glEnable (GL_BLEND);

	glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR); 
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, 0.5);
	
// JDH: brief attempt to fix
/*	VectorSet (color, 1, 1, 1);
	color[3] = bound(0, gl_shinywater.value, 1);        
	glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
*/
	GL_Bind(chrometexture2);

	for (p = fa->polys ; p ; p = p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			nv[0] = v[0];
			nv[1] = v[1];
			nv[2] = v[2];

			xv[0] = (nv[0] * r_world_matrix[0]) + (nv[1] * r_world_matrix[4]) +
				(nv[2] * r_world_matrix[8] ) + r_world_matrix[12];
			xv[1] = (nv[0] * r_world_matrix[1]) + (nv[1] * r_world_matrix[5]) + (nv[2] *
				r_world_matrix[9] ) + r_world_matrix[13];
			xv[2] = (nv[0] * r_world_matrix[2]) + (nv[1] * r_world_matrix[6]) + (nv[2] *
				r_world_matrix[10]) + r_world_matrix[14];

			xv[0] += xv[2];
			xv[1] += xv[2];

			VectorNormalize (xv);

			glTexCoord2f (xv[0], xv[1]);
			glVertex3fv (nv);
		}
		glEnd ();
	}

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//    if (r_wateralpha.value == 1)
	glDisable(GL_BLEND);

#else	
	return;		// JDH: this procedure never worked!
#endif
}

void CalcCausticTexCoords (float *v, float *s, float *t)
{
	float	os, ot;

	os = v[3];
	ot = v[4];

	*s = os + SINTABLE_APPROX(0.465 * (cl.time + ot));
	*s *= -3 * (0.5 / 64);

	*t = ot + SINTABLE_APPROX(0.465 * (cl.time + os));
	*t *= -3 * (0.5 / 64);
}

void EmitCausticsPolys (void)
{
	glpoly_t	*p;
	int		i;
	float		s, t, *v;
	extern	glpoly_t	*caustics_polys;

	if (!caustics_polys)
		return;
	
	GL_Bind (underwatertexture);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
	glEnable (GL_BLEND);

	for (p = caustics_polys ; p ; p = p->caustics_chain)
	{
		glBegin (GL_POLYGON);
		for (i = 0, v = p->verts[0] ; i < p->numverts ; i++, v += VERTEXSIZE)
		{
			CalcCausticTexCoords (v, &s, &t);

			glTexCoord2f (s, t);
			glVertex3fv (v);
		}
		glEnd ();
	}

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable (GL_BLEND);

	caustics_polys = NULL;
}

#endif		//#ifndef RQM_SV_ONLY
