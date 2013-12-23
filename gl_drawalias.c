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
// gl_drawalias.c -- alias model drawing

// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#ifdef HEXEN2_SUPPORT
  extern float RTint[256], GTint[256], BTint[256];

  float	model_alpha_H2;
  skintex_t	gl_extra_textures[MAX_EXTRA_TEXTURES];   // generic textures for models
#endif

#define NUMVERTEXNORMALS	162

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

extern	int	player_skins[MAX_SCOREBOARD];
extern	int	player_skins_fb[MAX_SCOREBOARD];

vec3_t	shadevector;

//qboolean full_light;
float	/*shadelight,*/ ambientlight;

// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anorm_dots.h"
;

float	*shadedots = r_avertexnormal_dots[0];

// fenix@io.com: model animation interpolation
//int	r_lastpose, r_currpose;

vec3_t	vertexlight;

extern cvar_t	gl_ringalpha;
extern cvar_t	gl_glows;


#define INTERP_WEAP_MAXNUM		24

typedef struct interpolated_weapon
{
	char	name[MAX_QPATH];
	int	maxDistance;
} interp_weapon_t;

static	interp_weapon_t	interpolated_weapons[INTERP_WEAP_MAXNUM];
static	int		interp_weap_num = 0;

/*
=============
DoWeaponInterpolation
=============
*/
int DoWeaponInterpolation (entity_t *currententity)
{
	int	i, extpos, distance = -1;
	char *modelname, *currname;

	if (currententity != &cl.viewent)
		return -1;

	modelname = currententity->model->name;
	if (COM_FilenamesEqualn (modelname, "progs/", 6 ))
		modelname += 6;

	i = strlen (modelname);
	if (( i > 4 ) && COM_FilenamesEqual (modelname + i - 4, ".mdl"))
	{
		extpos = i-4;
		modelname[ extpos ] = 0;
	}
	else extpos = -1;

	for (i=0 ; i<interp_weap_num ; i++)
	{
		currname = interpolated_weapons[i].name;
		if (!currname[0])
			break;

		if (COM_FilenamesEqual (modelname, currname))
		{
			distance = interpolated_weapons[i].maxDistance;
			break;
		}
	}

	if (extpos > 0) modelname[ extpos ] = '.';
	return distance;
}

// joe: from FuhQuake, but this is less configurable
void Set_Interpolated_Weapon_f (cmd_source_t src)
{
	int	i;
	char	str[MAX_QPATH];

	if (src == SRC_CLIENT)
		return;

	if (Cmd_Argc() == 2)
	{
		for (i=0 ; i<interp_weap_num ; i++)
			if (COM_FilenamesEqual(Cmd_Argv(1), interpolated_weapons[i].name))
			{
				Con_Printf ("%s`s distance is %d\n", Cmd_Argv(1), interpolated_weapons[i].maxDistance);
				return;
			}
		Con_Printf ("%s`s distance is default (%d)\n", Cmd_Argv(1), (int)gl_interdist.value);
		return;
	}

	if (Cmd_Argc() != 3)
	{
		Con_Print ("Usage: set_interpolated_weapon <model> <distance>\n");
		return;
	}

	Q_strcpy (str, Cmd_Argv(1), sizeof(str));
	for (i=0 ; i<interp_weap_num ; i++)
		if (COM_FilenamesEqual(str, interpolated_weapons[i].name))
			break;
	if (i == interp_weap_num)
	{
		if (interp_weap_num == 24)
		{
			Con_Print ("interp_weap_num == INTERP_WEAP_MAXNUM\n");
			return;
		}
		else
		{
			interp_weap_num++;
		}
	}

	Q_strcpy (interpolated_weapons[i].name, str, sizeof(interpolated_weapons[0].name));
	interpolated_weapons[i].maxDistance = (int)Q_atof(Cmd_Argv(2));
}

/*
=============
GL_DrawAliasGlow (JDH: from nehahra)
=============
*/
void GL_DrawAliasGlow (entity_t *currententity, model_t *clmodel)
{
	const int TORCH_STYLE = 1;      // Flicker.
    vec3_t    lightorigin;    // Origin of glow.
 	vec3_t    v;                      // Vector to torch.
	float     radius;         // Radius of torch flare.
	float     distance;               // Vector distance to torch.
	float     intensity;              // Intensity of torch flare.
	int	      i, j;

	if (clmodel->modhint != MOD_THUNDERBOLT) return;
		// JDH: nehahra also did glows for quad, pent & rocket; I don't.

	VectorCopy(currententity->origin, lightorigin);

	radius = 30.0f;

	VectorSubtract(lightorigin, r_origin, v);

	// See if view is outside the light.
    distance = VectorLength(v);
	if (distance > radius)
	{
		glDepthMask (0);
		glDisable (GL_TEXTURE_2D);
		glShadeModel (GL_SMOOTH);
		glEnable (GL_BLEND);
		glBlendFunc (GL_ONE, GL_ONE);

		glPushMatrix();
		glBegin(GL_TRIANGLE_FAN);

		intensity = 0.2f;

		// Now modulate with flicker.
        i = (int)(cl.time*10);
		if (!cl_lightstyle[TORCH_STYLE].length) {
			j = 256;
		}
		else {
			j = i % cl_lightstyle[TORCH_STYLE].length;
			j = cl_lightstyle[TORCH_STYLE].map[j] - 'a';
			j = j*22;
		}
		intensity *= ((float)j / 255.0f);

		// Set yellow intensity

		glColor3f(0.2f*intensity, 0.2f*intensity, 0.8f*intensity);

		for (i=0 ; i<3 ; i++)
			v[i] = lightorigin[i] - vpn[i]*radius;
		glVertex3fv(v);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i=16; i>=0; i--)
		{
			float a = i/16.0f * M_PI*2;
			for (j=0; j<3; j++)
				v[j] = lightorigin[j] + vright[j]*cos(a)*radius + vup[j]*sin(a)*radius;

			glVertex3fv(v);
		}
		glEnd();

		// Restore previous matrix! KH
		glPopMatrix();

		glColor3f (1,1,1);
		glDisable (GL_BLEND);
		glEnable (GL_TEXTURE_2D);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask (1);
	}
}

/*
=============
GL_GetAliasAlpha
=============
*/
float GL_GetAliasAlpha (const entity_t *ent)
{
	float alpha;
	
	if (gl_notrans.value)
		return 1.0f;

	if (ent == &cl.viewent)
	{
		if (cl.items & IT_INVISIBILITY)
			return gl_ringalpha.value;

		return bound(0, r_drawviewmodel.value, 1);
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2) 
		return model_alpha_H2;
#endif

	alpha = ent->transparency;
	if (alpha <= 0)
		alpha = 1.0;

	return alpha;
}

/*
=============
GL_DrawAliasFrame
=============
*/
void GL_DrawAliasFrame (aliashdr_t *paliashdr, entity_t *ent, skin_t *skin,
						/*int posenum,*/ qboolean mtex)
{
	float		light, /*alpha,*/ hscale, vscale, s, t;
	trivertx_t	*verts, *v1;
	int			*order, count, index;
	vec3_t		l_v;
#ifdef HEXEN2_SUPPORT
	float		r, g, b;
#endif

//	alpha = GL_GetAliasAlpha (ent);

//	r_currpose = posenum;

	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
//	verts += posenum * paliashdr->poseverts;
	verts += ent->currpose * paliashdr->poseverts;

	hscale = skin->h_value;
	vscale = skin->v_value;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (ent->colorshade)
		{
			r = RTint[ent->colorshade];
			g = GTint[ent->colorshade];
			b = BTint[ent->colorshade];
		}
		else
			r = g = b = 1;
	}
	else
#endif
	if (r_modelalpha < 1)
		glEnable (GL_BLEND);

	while ((count = *order++))
	{
		// get the vertex count and primitive type
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
		{
			glBegin (GL_TRIANGLE_STRIP);
		}

		do {
			// texture coordinates come from the draw list
			s = hscale * (*(float *)order++);
			t = vscale * (*(float *)order++);
			if (mtex)
			{
				qglMultiTexCoord2f (GL_TEXTURE0_ARB, s, t);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, s, t);
			}
			else
			{
				glTexCoord2f (s, t);
			}

			index = *order++;
			//order += 2;
			v1 = &verts[index];

			// normals and vertexes come from the frame list
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
//				light = shadedots[v1->lightnormalindex] * shadelight;
//				glColor4f (r*light, g*light, b*light, alpha);
				light = shadedots[v1->lightnormalindex];
				VectorScale(lightcolor, light, l_v);
				glColor4f (r*l_v[0], g*l_v[1], b*l_v[2], r_modelalpha);
			}
			else
		#endif
			{
				if (gl_vertexlights.value /*&& !full_light*/)
				{
					light = R_GetVertexLightValue (v1->lightnormalindex, ent->angles[0], ent->angles[1]);
					light += ambientlight/255;
					//light *= 2.0f;		// JDH: so values are in same range as shadedots
				}
				else
					//light = (shadedots[v1->lightnormalindex] * shadelight + ambientlight) / 256.0;
					light = shadedots[v1->lightnormalindex];
				//light = min(light, 1);

				//if (!full_light)
				{
				//	for (i=0 ; i<3 ; i++)
				//		l_v[i] = lightcolor[i] / 256 + light;
					VectorScale(lightcolor, light, l_v);
					glColor4f (l_v[0], l_v[1], l_v[2], r_modelalpha);
				}
				/*else
				{
					glColor4f (light, light, light, alpha);
				}*/
			}

			glVertex3f (v1->v[0], v1->v[1], v1->v[2]);

		} while (--count);

		glEnd ();
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	if (r_modelalpha < 1)
		glDisable (GL_BLEND);
}

/*
=============
GL_DrawAliasBlendedFrame

fenix@io.com: model animation interpolation
=============
*/
void GL_DrawAliasBlendedFrame (aliashdr_t *paliashdr, entity_t *ent, skin_t *skin,
								float blend, int distance, qboolean mtex)
{
	float		light, /*alpha, */hscale, vscale, s, t;
	trivertx_t	*verts1, *verts2, *v1, *v2;
	int			*order, count, index;
	vec3_t		d, l_v;
#ifdef HEXEN2_SUPPORT
	float		r, g, b;
#endif

//	alpha = GL_GetAliasAlpha (ent);

	distance = bound(INTERP_WEAP_MINDIST, distance, INTERP_WEAP_MAXDIST);

//	r_lastpose = ent->lastpose;
//	r_currpose = ent->currpose;

	verts1 = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts2 = verts1;

	verts1 += ent->lastpose * paliashdr->poseverts;
	verts2 += ent->currpose * paliashdr->poseverts;

	hscale = skin->h_value;
	vscale = skin->v_value;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (ent->colorshade)
		{
			r = RTint[ent->colorshade];
			g = GTint[ent->colorshade];
			b = BTint[ent->colorshade];
		}
		else
			r = g = b = 1;
	}
	else
#endif
	if (r_modelalpha < 1)
		glEnable (GL_BLEND);

	while ((count = *order++))
	{
		// get the vertex count and primitive type
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
		{
			glBegin (GL_TRIANGLE_STRIP);
		}

		do {
			// texture coordinates come from the draw list
			s = hscale * (*(float *)order++);
			t = vscale * (*(float *)order++);
			if (mtex)
			{
				qglMultiTexCoord2f (GL_TEXTURE0_ARB, s, t);
				qglMultiTexCoord2f (GL_TEXTURE1_ARB, s, t);
			}
			else
			{
				glTexCoord2f (s, t);
			}

			index = *order++;
			//order += 2;
			v1 = &verts1[index];
			v2 = &verts2[index];

			// normals and vertexes come from the frame list
			// blend the light intensity from the two frames together
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
			/*	d[0] = (shadedots[v2->lightnormalindex] * shadelight)
						- (shadedots[v1->lightnormalindex] * shadelight);
				light = ((shadedots[v1->lightnormalindex] * shadelight) + (blend * d[0]));
				glColor4f (r*light, g*light, b*light, alpha);
			*/
				light = shadedots[v1->lightnormalindex];
				d[0] = shadedots[v2->lightnormalindex] - light;
				light += (blend * d[0]);
				VectorScale(lightcolor, light, l_v);
				glColor4f (r*l_v[0], g*l_v[1], b*l_v[2], r_modelalpha);
			}
			else
		#endif
			{
				if (gl_vertexlights.value /*&& !full_light*/)
				{
					light = R_LerpVertexLight (v1->lightnormalindex, v2->lightnormalindex, 
												blend, ent->angles[0], ent->angles[1]);
					light = light*4 + ambientlight/255;
					//light *= 2.0f;		// JDH: so values are in same range as shadedots
				}
				else
				{
				/*	d[0] = (shadedots[v2->lightnormalindex] * shadelight + ambientlight)
						- (shadedots[v1->lightnormalindex] * shadelight + ambientlight);
					light = ((shadedots[v1->lightnormalindex] * shadelight + ambientlight) + (blend * d[0])) / 256.0;
				*/
					light = shadedots[v1->lightnormalindex];
					d[0] = shadedots[v2->lightnormalindex] - light;
					light += (blend * d[0]);
				}
			//	light = min(light, 1);

				//if (!full_light)
				{
				//	for (i=0 ; i<3 ; i++)
				//		l_v[i] = lightcolor[i] / 256 + light;
					VectorScale(lightcolor, light, l_v);
					glColor4f (l_v[0], l_v[1], l_v[2], r_modelalpha);
#ifdef _DEBUG
					if (!strcmp(ent->model->name, "progs/armor.mdl"))
						light = 4;
#endif
				}
				/*else
				{
					glColor4f (light, light, light, r_modelalpha);
				}*/
			}

			VectorSubtract (v2->v, v1->v, d);

			if ((ent == &cl.viewent) && (DotProduct(d, d) >= distance))
			{
			// This is to handle the flame that appears at the muzzle of many weapons when fired.
			// Besides 1 or 2 firing frames, the flame usually is located at the near end of the
			// weapon, behind the camera.  If interpolation occurred here, the flame would be seen
			// moving through the entire length of the barrel.
				glVertex3f (v2->v[0], v2->v[1], v2->v[2]);
			}
			else
			{
			// blend the vertex positions from each frame together
				glVertex3f (v1->v[0] + (blend * d[0]), v1->v[1] + (blend * d[1]), v1->v[2] + (blend * d[2]));
			}

		} while (--count);

		glEnd ();
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	if (r_modelalpha < 1)
		glDisable (GL_BLEND);
}

/*
=============
GL_DrawAliasShadow
=============
*/
void GL_DrawAliasShadow (aliashdr_t *paliashdr, maliasframedesc_t *currframe, /*int posenum,*/ entity_t *ent)
{
	trivertx_t	*verts;
	int			*order, count, index;
	vec3_t		point;
	float		height, lheight;

	lheight = ent->origin[2] - lightspot[2];
	height = 1 - lheight;

	verts = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
//	verts += posenum * paliashdr->poseverts;
	verts += ent->currpose * paliashdr->poseverts;
	order = (int *)((byte *)paliashdr + paliashdr->commands);

	while ((count = *order++))
	{
		// get the vertex count and primitive type
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
		{
			glBegin (GL_TRIANGLE_STRIP);
		}

		do {
			// texture coordinates come from the draw list
			// (skipped for shadows) glTexCoord2fv ((float *)order);
			index = order[2];
			order += 3;

			// normals and vertexes come from the frame list
			point[0] = verts[index].v[0] * currframe->scale[0] + currframe->translate[0];
			point[1] = verts[index].v[1] * currframe->scale[1] + currframe->translate[1];
			point[2] = verts[index].v[2] * currframe->scale[2] + currframe->translate[2];

			point[0] -= shadevector[0] * (point[2] + lheight);
			point[1] -= shadevector[1] * (point[2] + lheight);
			point[2] = height;
			glVertex3fv (point);

			//verts++;
		} while (--count);

		glEnd ();
	}
}

/*
=============
GL_DrawAliasBlendedShadow

fenix@io.com: model animation interpolation
=============
*/
void GL_DrawAliasBlendedShadow (aliashdr_t *paliashdr, maliasframedesc_t *currframe, /*int lastpose, int nextpose,*/ entity_t *ent)
{
	trivertx_t	*verts1, *verts2;
	int			*order, count, index;
	vec3_t		point1, point2, d;
	float		height, lheight, blend;
	
	blend = fabs(cl.time - ent->frame_start_time) / ent->frame_interval;
	blend = min(1, blend);

	lheight = ent->origin[2] - lightspot[2];
	height  = 1 - lheight;

	verts1 = (trivertx_t *)((byte *)paliashdr + paliashdr->posedata);
	verts2 = verts1;

//	verts1 += lastpose * paliashdr->poseverts;
//	verts2 += nextpose * paliashdr->poseverts;
	verts1 += ent->lastpose * paliashdr->poseverts;
	verts2 += ent->currpose * paliashdr->poseverts;

	order = (int *)((byte *)paliashdr + paliashdr->commands);

	while ((count = *order++))
	{
		// get the vertex count and primitive type
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
		{
			glBegin (GL_TRIANGLE_STRIP);
		}

		do {
			index = order[2];
			order += 3;

			point1[0] = verts1[index].v[0] * currframe->scale[0] + currframe->translate[0];
			point1[1] = verts1[index].v[1] * currframe->scale[1] + currframe->translate[1];
			point1[2] = verts1[index].v[2] * currframe->scale[2] + currframe->translate[2];

			point1[0] -= shadevector[0]*(point1[2]+lheight);
			point1[1] -= shadevector[1]*(point1[2]+lheight);

			point2[0] = verts2[index].v[0] * currframe->scale[0] + currframe->translate[0];
			point2[1] = verts2[index].v[1] * currframe->scale[1] + currframe->translate[1];
			point2[2] = verts2[index].v[2] * currframe->scale[2] + currframe->translate[2];

			point2[0] -= shadevector[0]*(point2[2]+lheight);
			point2[1] -= shadevector[1]*(point2[2]+lheight);

			VectorSubtract (point2, point1, d);

			glVertex3f (point1[0] + (blend * d[0]), point1[1] + (blend * d[1]), height);

			//verts1++;
			//verts2++;
		} while (--count);

		glEnd ();
	}
}

/*
=================
R_DrawAliasShadow
=================
*/
void R_DrawAliasShadow (entity_t *ent, aliashdr_t *paliashdr, maliasframedesc_t *currframe)
{
	float		an, alpha;
	static	float	shadescale = 0;

	if (r_refdef.vieworg[2] + 1 <= lightspot[2])
		return;		// no shadows on ents above player

#ifdef HEXEN2_SUPPORT
	if (hexen2 && ((ent->model->flags & EF_ROTATE) || (ent->drawflags & MLS_MASKIN)))
	{
		return;		// in this case, R_LightPoint isn't called when calculating lighting,
					// so vector "lightspot" is not set.  Hence shadow position is not right.
	}
#endif

	if (!shadescale)
		shadescale = 1 / sqrt(2);
	an = -ent->angles[1] / 180 * M_PI;		// yaw

	VectorSet (shadevector, cos(an) * shadescale, sin(an) * shadescale, shadescale);

	glPushMatrix ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		R_RotateForEntity_H2 (ent);
	else
#endif
	{
		glTranslatef (ent->origin[0], ent->origin[1], ent->origin[2]);
		glRotatef (ent->angles[1], 0, 0, 1);
	}

	glDisable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	if ((r_shadows.value > 0) && (r_shadows.value < 1))
		alpha = r_shadows.value;
	else
		alpha = ambientlight/200;

	glColor4f (0, 0, 0, alpha*r_modelalpha);
	// Pa3PyX: prevent Z fighting
	glDepthMask(0);

	// fenix@io.com: model animation interpolation
	if (gl_interpolate_animation.value)
		GL_DrawAliasBlendedShadow (paliashdr, currframe, /*r_lastpose, r_currpose,*/ ent);
	else
		GL_DrawAliasShadow (paliashdr, currframe, /*r_currpose,*/ ent);

	glDepthMask(1);

	glEnable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
	glPopMatrix ();
}

/*
=================
R_SetupAliasFrame
=================
*/
void R_SetupAliasFrame (entity_t *ent, aliashdr_t *paliashdr, maliasframedesc_t *currframe,
						skin_t *skin, qboolean mtex)
{
	int		pose, numposes;
	float	interval;

	pose = currframe->firstpose;
	numposes = currframe->numposes;

	if (numposes > 1)
	{
		interval = currframe->interval;
		pose += (int)(cl.time / interval) % numposes;
	}

//	GL_DrawAliasFrame (paliashdr, ent, skin, pose, mtex);
	ent->currpose = pose;
	GL_DrawAliasFrame (paliashdr, ent, skin, mtex);
}

/*
=================
R_SetupAliasBlend
=================
*/
float R_SetupAliasBlend (entity_t *ent, int pose)
{
	float blend;
	
	if (ent->frame_start_time == 0)		// JDH: 2009/05/26 - prevent model morphing into first pose
	{
		ent->currpose = ent->lastpose = pose;
		ent->frame_start_time = cl.time;
		blend = 0;
	}
	else if (ent->currpose != pose)
	{
		ent->lastpose = ent->currpose;
		ent->currpose = pose;
		ent->frame_start_time = cl.time;
		blend = 0;
	}
	else
	{
#if 0
		blend = (cl.time - ent->frame_start_time) / ent->frame_interval;

		// weird things start happening if blend passes 1
		//  (blend < 0 can occur when rewinding a demo)
		if (/*cl.paused ||*/ blend > 1 || blend < 0)
			blend = 1;
#else
		blend = fabs(cl.time - ent->frame_start_time) / ent->frame_interval;
			// negative time interval occurs during demo rewind
		
		// weird things start happening if blend passes 1
		if (/*cl.paused ||*/ blend > 1)
			blend = 1;
#endif
	}

	return blend;
}

/*
=================
R_SetupAliasBlendedFrame

fenix@io.com: model animation interpolation
=================
*/
void R_SetupAliasBlendedFrame (entity_t *ent, aliashdr_t *paliashdr, maliasframedesc_t *currframe,
								skin_t *skin, qboolean mtex, int distance)
{
	int		pose, numposes;
	float	blend;

	pose = currframe->firstpose;
	numposes = currframe->numposes;

#if defined(_DEBUG) && defined(HEXEN2_SUPPORT)
//	if (hexen2 && !strcmp(ent->model->name, "models/golem_i.mdl"))
//		blend = 124;
	if (cls.demoplayback && (ent-cl_entities == 236))
		blend = 1523;
#endif

	if (numposes > 1)
	{
		ent->frame_interval = currframe->interval;
		pose += (int)(cl.time / ent->frame_interval) % numposes;
	}
	else
	{
		// One tenth of a second is a good for most Quake animations.
		// If the nextthink is longer then the animation is usually meant to pause
		// (e.g. check out the shambler magic animation in shambler.qc). If its
		// shorter then things will still be smoothed partly, and the jumps will be
		// less noticable because of the shorter time. So, this is probably a good assumption.

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			ent->frame_interval = HX_FRAME_TIME;
		else
	#endif
			ent->frame_interval = 0.1;
	}

	blend = R_SetupAliasBlend (ent, pose);

	GL_DrawAliasBlendedFrame (paliashdr, ent, skin, blend, distance, mtex);
}

extern cvar_t r_modelbrightness;

/*
=================
R_CalcStdAliasLighting
=================
*/
void R_CalcStdAliasLighting (const entity_t *ent, const vec3_t entorigin)
{
	int			lnum;
	vec3_t		dist/*, dlight_color*/;
	float		add, level, radiusmax = 0.0;

	//full_light = false;
	ambientlight = /*shadelight =*/ R_LightPoint (entorigin);

#if 0
	for (lnum = 0 ; lnum < MAX_DLIGHTS ; lnum++)
	{
		if (cl_dlights[lnum].die < cl.time || !cl_dlights[lnum].radius)
			continue;

		VectorSubtract (entorigin, cl_dlights[lnum].origin, dist);

//			add = cl_dlights[lnum].radius - VectorLength(dist);
		add = 1 - VectorLength(dist)/cl_dlights[lnum].radius;

		if (add > 0)
		{
			if (gl_vertexlights.value)
			{
				if (!radiusmax || cl_dlights[lnum].radius > radiusmax)
				{
					radiusmax = cl_dlights[lnum].radius;
					VectorCopy (cl_dlights[lnum].origin, vertexlight);
				}
			}
		// joe: only allow colorlight affection if dynamic lights are on
			if (r_dynamic.value)
			{
//					VectorCopy (dlightcolor[cl_dlights[lnum].type], dlight_color);
				if ((cl_dlights[lnum].type == lt_explosion2) || (cl_dlights[lnum].type == lt_explosion3))
					VectorCopy (ExploColor, dlight_color);
				else
					VectorCopy (bubblecolor[cl_dlights[lnum].type], dlight_color);
				
//					VectorMA (lightcolor, ambientlight * add * 0.7, dlight_color, lightcolor);
				VectorMA (lightcolor, add * 0.5, dlight_color, lightcolor);

/*					for (i=0 ; i<3 ; i++)
				{
//						lightcolor[i] += (dlight_color[i] * add) * 2;
//						lightcolor[i] += (dlight_color[i] / 255.0) * add;
//						lightcolor[i] += dlight_color[i] * add * 0.4;
					lightcolor[i] += ambientlight * dlight_color[i] * add * 0.7;
					if (lightcolor[i] > 256)
					{
						switch (i)
						{
						case 0:
							lightcolor[1] = lightcolor[1] - (1 * lightcolor[1]/3);
							lightcolor[2] = lightcolor[2] - (1 * lightcolor[2]/3);
							break;

						case 1:
							lightcolor[0] = lightcolor[0] - (1 * lightcolor[0]/3);
							lightcolor[2] = lightcolor[2] - (1 * lightcolor[2]/3);
							break;

						case 2:
							lightcolor[1] = lightcolor[1] - (1 * lightcolor[1]/3);
							lightcolor[0] = lightcolor[0] - (1 * lightcolor[0]/3);
							break;
						}
					}
				//	ambientlight += (add*0.75);//<--add this here
				//	shadelight += (add*0.70);
				}
*/
			}

//				ambientlight += add * ambientlight * 0.7;
			ambientlight += add * 0.5;
		}
	}
#else
	if (r_dynamic.value)
	{
		int numdlights = 0;
		//float scale = 0;
		vec3_t dlights = {0,0,0};

		for (lnum = 0 ; lnum < MAX_DLIGHTS ; lnum++)
		{
			if (DLIGHT_INACTIVE(&cl_dlights[lnum]) || !cl_dlights[lnum].radius)
				continue;

			VectorSubtract (entorigin, cl_dlights[lnum].origin, dist);
			add = cl_dlights[lnum].radius - VectorLength(dist);

			if (add > 0)
			{
			#ifdef ALIAS_COLOR_DLIGHT
				float *dlight_color = cl_dlights[lnum].color;
				VectorMA (dlights, add, dlight_color, dlights);		// dlights[i] += add*dlight_color[i]

				// tone back the red channel a bit:
				if (dlight_color[0] > dlight_color[1])
					dlights[0] -= 0.25f * add * dlight_color[0];
			#else
				dlights[0] += add/2.0f;
				dlights[1] = dlights[2] = dlights[0];
			#endif

				if (gl_vertexlights.value)
				{
					if (!radiusmax || cl_dlights[lnum].radius > radiusmax)
					{
						radiusmax = cl_dlights[lnum].radius;
						VectorCopy (cl_dlights[lnum].origin, vertexlight);
					}
				}

				numdlights++;
			}
		}

		if (numdlights)
		{
		// divide by numdlights to get the average color, then double the level
			add = 2.0f/numdlights;
			VectorMA (lightcolor, add, dlights, lightcolor);
			ambientlight += add*(dlights[0] + dlights[1] + dlights[2])/3;
		}
	}
#endif

	// calculate pitch and yaw for vertex lighting
	if (gl_vertexlights.value)
	{
		vec3_t	dist, ang;

		if (!radiusmax)
		{
			vlight_pitch = 45;
			vlight_yaw = 45;
		}
		else
		{
			VectorSubtract (vertexlight, entorigin, dist);

			vectoangles (dist, ang);
			vlight_pitch = ang[0];
			vlight_yaw = ang[1];
		}
	}

	if (ent->model->modhint == MOD_PLAYER && r_fullbrightskins.value)
	{
		ambientlight = /*shadelight =*/ 128.0f;
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 255.0f;
		//full_light = true;
	}
	else
	{
		// clamp lighting so it doesn't overbright as much
		ambientlight = min(128.0f, ambientlight);
		/*if (ambientlight + shadelight > 192)
		{
			shadelight = 192 - ambientlight;
		}*/

		level = (lightcolor[0] + lightcolor[1] + lightcolor[2]) / 3.0f;
		/*add = (192.0f - ambientlight) / level;
		if (add < 1.0f)
			VectorScale (lightcolor, add, lightcolor);
*/
		// always give the gun some light
		if (ent == &cl.viewent)
		{
			ambientlight = max(24.0f, ambientlight);
			if (level)
			{
				add = 24.0f / level;
				if (add > 1.0f)
					VectorScale (lightcolor, add, lightcolor);
			}
			else
				lightcolor[0] = lightcolor[1] = lightcolor[2] = 24.0f;
		}

#ifdef HEXEN2_SUPPORT
		if (!hexen2)
#endif
		// never allow players to go totally black
		if (ent->model->modhint == MOD_PLAYER)
		{
			//if (ambientlight < 8)
			//	ambientlight = shadelight = 8;
			ambientlight = max(8.0f, ambientlight);
			if (level)
			{
				add = 8.0f / level;
				if (add > 1.0f)
					VectorScale (lightcolor, add, lightcolor);
			}
			else
				lightcolor[0] = lightcolor[1] = lightcolor[2] = 8.0f;

			/*add = 8 - light;
			if (add > 0)
			{
				lightcolor[0] += add;
				lightcolor[1] += add;
				lightcolor[2] += add;
			}*/
		}
	}
}

/*
=================
R_CalcAliasLighting
=================
*/
void R_CalcAliasLighting (const entity_t *ent, const model_t *clmodel, qboolean *noshadow)
{
	float		level;

	// make thunderbolt and torches full light
	if (clmodel->modhint == MOD_THUNDERBOLT)
	{
		//ambientlight = 210;
		//shadelight = 0;
		ambientlight = lightcolor[0] = lightcolor[1] = lightcolor[2] = 210;
		//full_light = true;
		*noshadow = true;
	}
/*	else if (clmodel->modhint == MOD_FLAME)		// removed 2010/03/02 (for base of Quoth's brazier)
	{
		//ambientlight = 255;
		//shadelight = 0;
		ambientlight = lightcolor[0] = lightcolor[1] = lightcolor[2] = 255;
		//full_light = true;
		*noshadow = true;
	}
*/	else
	{
		if (clmodel->modhint == MOD_FLAME)
			*noshadow = true;

	// normal lighting
 		R_CalcStdAliasLighting (ent, ent->origin);
	}

	shadedots = r_avertexnormal_dots[((int)(ent->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];

	level = max(0, r_modelbrightness.value);
	level /= 180.0f/*200.0f*/;
	VectorScale (lightcolor, level, lightcolor);
}

#ifdef HEXEN2_SUPPORT
/*
=================
R_CalcAliasLighting_H2
=================
*/
//#define OLDH2LIGHT

void R_CalcAliasLighting_H2 (entity_t *ent, model_t *clmodel, qboolean *noshadow)
{
	vec3_t	adjust_origin;
	int		mls;
#ifdef OLDH2LIGHT
	vec3_t	dist;
	int		lnum;
	float	/*shadelight, */add;
#endif

	mls = ent->drawflags & MLS_MASKIN;
	if (ent->model->flags & EF_ROTATE)
	{
		ambientlight = /*shadelight = */lightcolor[0] = lightcolor[1] = lightcolor[2] = 
			60 + 34 + sin(ent->origin[0] + ent->origin[1] + (cl.time*3.8)) * 34;
	}
	else if (mls == MLS_ABSLIGHT)
	{
		ambientlight = /*shadelight = */lightcolor[0] = lightcolor[1] = lightcolor[2] = ent->abslight;
	}
	else if (mls != MLS_NONE)
	{ // Use a model light style (25-30)
		ambientlight = /*shadelight = */lightcolor[0] = lightcolor[1] = lightcolor[2] = d_lightstylevalue[24+mls]/2;
	}
	else
	{
		VectorCopy(ent->origin, adjust_origin);
		adjust_origin[2] += (ent->model->mins[2] + ent->model->maxs[2]) / 2;

#ifdef OLDH2LIGHT
		ambientlight = /*shadelight = */lightcolor[0] = lightcolor[1] = lightcolor[2] = R_LightPoint (adjust_origin);

		// allways give the gun some light
		if (ent == &cl.viewent && ambientlight < 24)
			ambientlight = /*shadelight =*/ 24;

		for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
		{
			if (!DLIGHT_INACTIVE(&cl_dlights[lnum]))
			{
				VectorSubtract (ent->origin, cl_dlights[lnum].origin, dist);
				add = cl_dlights[lnum].radius - VectorLength(dist);

				if (add > 0)
					ambientlight += add;
			}
		}

		// clamp lighting so it doesn't overbright as much
		if (ambientlight > 128)
			ambientlight = 128;
		/*if (ambientlight + shadelight > 192)
			shadelight = 192 - ambientlight;*/
#else		
		R_CalcStdAliasLighting (ent, adjust_origin);
#endif
	}

	// JDH: setting client's light_level is very important, since monsters use it
	//  to determine if player is visible (and should therefore be attacked)
	if (ent == &cl.viewent)
		cl.light_level = ambientlight;

	shadedots = r_avertexnormal_dots[((int)(ent->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
//	shadelight /= 200.0;

#ifdef OLDH2LIGHT
	VectorScale (lightcolor, 1/200.0, lightcolor);
#endif

/*	DONE IN DRAWSHADOW
	an = ent->angles[1]/180*M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);
*/
	*noshadow = false;
	//full_light = true;		// no fullbright texture
}
#endif	// #ifdef HEXEN2_SUPPORT


/*
=================
R_GetAliasSkin
=================
*/
skin_t * R_GetAliasSkin (entity_t *ent, aliashdr_t *paliashdr)
{
	int		anim, skinnum, index;
	skin_t	*skin;
	static skin_t dummy;

	anim = (int)(cl.time*10) & 3;
	skinnum = ent->skinnum;

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (skinnum >= 100) && (skinnum < 100+MAX_EXTRA_TEXTURES))
	{
		if (gl_extra_textures[skinnum-100].texnum == -1)  // Need to load it in
		{
			mpic_t	*skinpic = Draw_GetCachePic (va ("gfx/skin%d.lmp", skinnum), false);
			if (skinpic)
			{
				gl_extra_textures[skinnum-100].texnum = skinpic->texnum;
				gl_extra_textures[skinnum-100].s = skinpic->sh;
				gl_extra_textures[skinnum-100].t = skinpic->th;
			}
		}

		dummy.gl_texturenum = gl_extra_textures[skinnum-100].texnum;
		dummy.fb_texturenum = 0;
		dummy.fb_isLuma = false;
		dummy.h_value = gl_extra_textures[skinnum-100].s;
		dummy.v_value = gl_extra_textures[skinnum-100].t;
		return &dummy;
	}
#endif

	if ((skinnum >= paliashdr->numskins) || (skinnum < 0))
	{
		Con_DPrintf ("R_GetAliasSkin: no such skin #%d for %s\n", skinnum, ent->model->name);
		ent->skinnum = skinnum = 0;
	}

	skin = &paliashdr->skins[skinnum].skins[anim];

	// we can't dynamically colormap textures, so they are cached
	// seperately for the players. Heads are just uncolored.
	if ((ent->colormap != vid.colormap) && !gl_nocolors.value)
	{
		index = ent - cl_entities;

		if ((index > 0) && (index <= cl.maxclients))
		{
//JDH			dummy.gl_texturenum = playertextures - 1 + index;
			dummy.gl_texturenum = player_skins[ index-1 ];
			if (gl_fb_models.value)
			{
				dummy.fb_texturenum = player_skins_fb[ index-1 ];
				dummy.fb_isLuma = skin->fb_isLuma;
			}
			else
			{
				dummy.fb_texturenum = 0;
				dummy.fb_isLuma = false;
			}
			dummy.h_value = skin->h_value;
			dummy.v_value = skin->v_value;
			return &dummy;
		}
	}

	if (/*full_light ||*/ !gl_fb_models.value)
	{
		dummy.gl_texturenum = skin->gl_texturenum;
		dummy.fb_texturenum = 0;
		dummy.fb_isLuma = false;
		dummy.h_value = skin->h_value;
		dummy.v_value = skin->v_value;
		return &dummy;
	}

	return skin;
}


#ifdef HEXEN2_SUPPORT
/*
=================
R_ScaleAliasModel_H2
=================
*/
void R_ScaleAliasModel_H2 (entity_t *ent, maliasframedesc_t *currframe)
{
	float			entScale, xyfact, zfact;
	int				i;
	static float	tmatrix[3][4];

	if ((ent->scale != 0) && (ent->scale != 100))
	{
		entScale = (float)ent->scale / 100.0;
		for ( i = 0; i < 3; i++ )
			tmatrix[i][i] = currframe->scale[i];

		switch (ent->drawflags & SCALE_TYPE_MASKIN)
		{
		case SCALE_TYPE_UNIFORM:
			tmatrix[0][0] *= entScale;
			tmatrix[1][1] *= entScale;
			tmatrix[2][2] *= entScale;
			xyfact = zfact = (entScale-1.0)*127.95;
			break;
		case SCALE_TYPE_XYONLY:
			tmatrix[0][0] *= entScale;
			tmatrix[1][1] *= entScale;
			xyfact = (entScale-1.0)*127.95;
			zfact = 1.0;
			break;
		case SCALE_TYPE_ZONLY:
			tmatrix[2][2] *= entScale;
			xyfact = 1.0;
			zfact = (entScale-1.0)*127.95;
			break;
		}

		tmatrix[0][3] = currframe->translate[0] - currframe->scale[0]*xyfact;
		tmatrix[1][3] = currframe->translate[1] - currframe->scale[1]*xyfact;
		tmatrix[2][3] = currframe->translate[2];

		switch (ent->drawflags & SCALE_ORIGIN_MASKIN)
		{
		case SCALE_ORIGIN_CENTER:
			tmatrix[2][3] -= currframe->scale[2]*zfact;
			break;
		//case SCALE_ORIGIN_BOTTOM:
		//	break;
		case SCALE_ORIGIN_TOP:
			tmatrix[2][3] -= currframe->scale[2]*zfact*2.0;
			break;
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			tmatrix[i][i] = currframe->scale[i];
			tmatrix[i][3] = currframe->translate[i];
		}
	}

	if (ent->model->flags & EF_ROTATE)
	{ // Floating motion
		tmatrix[2][3] += sin(ent->origin[0] + ent->origin[1] + (cl.time*3))*5.5;
	}

	glTranslatef (tmatrix[0][3], tmatrix[1][3], tmatrix[2][3]);
	glScalef (tmatrix[0][0], tmatrix[1][1], tmatrix[2][2]);
}

/*
=================
R_EnableAliasBlend_H2
=================
*/
void R_EnableAliasBlend_H2 (entity_t *ent, model_t *clmodel)
{
	if (clmodel->flags & EF_SPECIAL_TRANS)
	{
		// rjr
		glEnable (GL_BLEND);
		glBlendFunc (GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
//		glColor3f (1,1,1);
		model_alpha_H2 = 1.0f;
		glDisable (GL_CULL_FACE);
	}
	else if (ent->drawflags & DRF_TRANSLUCENT)
	{
		// rjr
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glColor4f (1,1,1,r_wateralpha.value);
		model_alpha_H2 = r_wateralpha.value;
	}
	else if (clmodel->flags & EF_TRANSPARENT)
	{
		// rjr
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glColor3f (1,1,1);
		model_alpha_H2 = 1.0f;
	}
	else if (clmodel->flags & EF_HOLEY)
	{
		// rjr
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//		glColor3f (1,1,1);
		model_alpha_H2 = 1.0f;
	}
	else
	{
		// rjr
//		glColor3f (1,1,1);
		model_alpha_H2 = 1.0f;
	}
}

/*
=================
R_DisableAliasBlend_H2
=================
*/
void R_DisableAliasBlend_H2 (entity_t *ent, model_t *clmodel)
{
	if (clmodel->flags & EF_SPECIAL_TRANS)
	{
		glDisable (GL_BLEND);
		glEnable (GL_CULL_FACE);
	}

	if (ent->drawflags & DRF_TRANSLUCENT)
		glDisable (GL_BLEND);

	if (clmodel->flags & (EF_TRANSPARENT | EF_HOLEY))
		glDisable (GL_BLEND);
}

#endif	// #ifdef HEXEN2_SUPPORT

/*
=================
R_PositionAliasModel
=================
*/
void R_PositionAliasModel (entity_t *ent, model_t *clmodel, maliasframedesc_t *currframe)
{
	float scale_x, scale_y, scale_z, trans_z;

	// fenix@io.com: model transform interpolation
	// joe: don't blend flame/fire model frames
	if (gl_interpolate_transform.value && clmodel->modhint != MOD_FLAME)
	{
#ifdef HEXEN2_SUPPORT
		if (hexen2)
			R_BlendedRotateForEntity_H2 (ent);
		else
#endif
		R_BlendedRotateForEntity (ent);
	}
	else
	{
#ifdef HEXEN2_SUPPORT
		if (hexen2)
			R_RotateForEntity_H2 (ent);
		else
#endif
		R_RotateForEntity (ent);
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		R_ScaleAliasModel_H2 (ent, currframe);
		return;
	}
#endif

/*	scale_x = paliashdr->scale[0];
	scale_y = paliashdr->scale[1];
	scale_z = paliashdr->scale[2];
	trans_z = paliashdr->scale_origin[2];
*/
	scale_x = currframe->scale[0];
	scale_y = currframe->scale[1];
	scale_z = currframe->scale[2];
	trans_z = currframe->translate[2];

	if ((clmodel->modhint == MOD_EYES) && gl_doubleeyes.value)
	{
		// double size of eyes, since they are really hard to see in gl
		scale_x *= 2;
		scale_y *= 2;
		scale_z *= 2;
		trans_z -= 30;		// 22 + 8
	}
	else if (ent == &cl.viewent)
	{
		scale_x *= (0.5 + bound(0, r_viewmodelsize.value, 1) / 2);
	}

//	glTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], trans_z);
	glTranslatef (currframe->translate[0], currframe->translate[1], trans_z);
	glScalef (scale_x, scale_y, scale_z);
}

/*
=================
R_DrawAliasFrame
=================
*/
void R_DrawAliasFrame (entity_t *ent, aliashdr_t *paliashdr, maliasframedesc_t *currframe,
						skin_t *skin, modhint_t modhint, qboolean mtex)
{
	int distance;

	// fenix@io.com: model animation interpolation
	if (gl_interpolate_animation.value && (modhint != MOD_FLAME))
	{
		// if model's on list, use the given value
		distance = DoWeaponInterpolation (ent);

		if (distance == -1)
		{
			// if model's not on list, but is on JQ's weapon list, use gl_interdist
			if (modhint == MOD_WEAPON)
				distance = (int) gl_interdist.value;

			// else use the max distance
			else
				distance = INTERP_WEAP_MAXDIST;
		}

		R_SetupAliasBlendedFrame (ent, paliashdr, currframe, skin, mtex, distance);
	}
	else
	{
		R_SetupAliasFrame (ent, paliashdr, currframe, skin, mtex);
	}
}

/*
=================
R_DrawCurrAliasFrame
=================
*/
void R_DrawCurrAliasFrame (entity_t *ent, aliashdr_t *paliashdr, maliasframedesc_t *currframe,
							modhint_t modhint, skin_t *skin)
{
	qboolean mtex = false;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		R_EnableAliasBlend_H2 (ent, ent->model);
	}
#endif

	r_modelalpha = GL_GetAliasAlpha (ent);
	
	if (gl_smoothmodels.value)
		glShadeModel (GL_SMOOTH);

	if (gl_affinemodels.value)
		glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	GL_DisableMultitexture ();
	GL_Bind (skin->gl_texturenum);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (skin->fb_texturenum && gl_mtexable)
	{
		if (!skin->fb_isLuma || gl_add_ext)
		{
			mtex = true;
			GL_EnableMultitexture ();
			GL_Bind (skin->fb_texturenum);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (skin->fb_isLuma ? GL_ADD : GL_DECAL));
		}
	}


	R_DrawAliasFrame (ent, paliashdr, currframe, skin, modhint, mtex);


	if (mtex)
	{
		GL_DisableMultitexture ();
	}
	else
	{
		if (skin->fb_texturenum)
		{
			GL_Bind (skin->fb_texturenum);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			if (skin->fb_isLuma)
			{
				glDepthMask (GL_FALSE);	// don't bother writing Z
				glEnable (GL_BLEND);
				glBlendFunc (GL_ONE, GL_ONE);

				R_DrawAliasFrame (ent, paliashdr, currframe, skin, modhint, false);

				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDisable (GL_BLEND);
				glDepthMask (GL_TRUE);
			}
			else
			{
				glEnable (GL_ALPHA_TEST);

				R_DrawAliasFrame (ent, paliashdr, currframe, skin, modhint, false);

				glDisable (GL_ALPHA_TEST);
			}
		}
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		R_DisableAliasBlend_H2 (ent, ent->model);
	}
#endif

	glShadeModel (GL_FLAT);
	if (gl_affinemodels.value)
		glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

/*
=================
R_DrawAliasModel
=================
*/
void R_DrawAliasModel (entity_t *ent)
{
	vec3_t				mins, maxs;
	aliashdr_t			*paliashdr;
	model_t				*clmodel = ent->model;
	qboolean			noshadow = false;
	int					framenum;
	maliasframedesc_t	*currframe;
	skin_t				*skin;

	/*if (!gl_notrans.value)	// always true if not -nehahra
	{
		r_modelalpha = ent->transparency;
		if (r_modelalpha == 0)
			r_modelalpha = 1.0;
	}
	else r_modelalpha = 1.0;*/


	if (ent->angles[0] || ent->angles[1] || ent->angles[2])
	{
#ifdef _WIN32
// JDH: R_CullSphere doesn't seem to work reliably on Linux
		if (R_CullSphere(ent->origin, clmodel->radius))
		{
//			Con_DPrintf ("R_DrawAliasModel: R_CullSphere of %s returned TRUE\n", clmodel->name);
			return;
		}
#else
		int i;
		for (i=0 ; i<3 ; i++)
		{
			mins[i] = ent->origin[i] - clmodel->radius;
			maxs[i] = ent->origin[i] + clmodel->radius;
		}

		if (R_CullBox(mins, maxs))
		{
//			Con_DPrintf ("R_DrawAliasModel: R_CullBox of %s returned TRUE (angles[1] = %.2f)\n", clmodel->name, ent->angles[1]);
			return;
		}
#endif
	}
	else
	{
		VectorAdd (ent->origin, clmodel->mins, mins);
		VectorAdd (ent->origin, clmodel->maxs, maxs);

		if (R_CullBox(mins, maxs))
		{
//			Con_DPrintf ("R_DrawAliasModel: R_CullBox of %s returned TRUE\n", clmodel->name);
			return;
		}
	}

#ifdef _DEBUG
//	if (cls.demoplayback && /*cl_demorewind.value &&*/ (ent->modelindex == cl_modelindex[mi_scrag]) && (ent-cl_entities == 236))
		//framenum = 0;
		//Con_Printf ("mins = %.0f,%.0f,%.0f;  maxs = %.0f,%.0f,%.0f\n", 
		//	mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]);
//		Con_Printf ("scrag: origin = %.0f,%.0f,%.0f; translate_start_time = %lf\n", 
//					ent->origin[0], ent->origin[1], ent->origin[2], ent->translate_start_time);
#endif
	
	VectorCopy (ent->origin, r_entorigin);
	VectorSubtract (r_origin, r_entorigin, modelorg);

	// get lighting information
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		R_CalcAliasLighting_H2 (ent, clmodel, &noshadow);
	else
#endif
		R_CalcAliasLighting (ent, clmodel, &noshadow);


	// locate the proper data
	paliashdr = (aliashdr_t *)Mod_Extradata (clmodel);
	c_alias_polys += paliashdr->numtris;

#ifdef _DEBUG
	if (ent-cl_entities == 236)
	{
		framenum = 3;
	}
#endif
	
	framenum = ent->frame;
	if ((framenum >= paliashdr->numframes) || (framenum < 0))
	{
		Con_DPrintf ("\x02""R_DrawAliasModel(%s): no such frame %d\n", clmodel->name, framenum);
		framenum = ent->frame = 0;
	}

	currframe = &paliashdr->frames[framenum];

// draw all the triangles

	glPushMatrix ();

	R_PositionAliasModel (ent, clmodel, currframe);
	skin = R_GetAliasSkin (ent, paliashdr);
	R_DrawCurrAliasFrame (ent, paliashdr, currframe, clmodel->modhint, skin);

	glPopMatrix ();


// JDH: from nehBJP
	if (gl_glows.value)
		GL_DrawAliasGlow (ent, clmodel);

	if (r_shadows.value && !noshadow && (ent != &cl.viewent))
	{
		R_DrawAliasShadow (ent, paliashdr, currframe);
	}

	glColor3f (1, 1, 1);
}

#endif		//#ifndef RQM_SV_ONLY

