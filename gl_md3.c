
#ifndef RQM_SV_ONLY

#include "quakedef.h"

#define MD3_MAX_TRIANGLES   3
#define MD3_IDP3			4
#define MD3_3DS				16

extern vec3_t   lightcolor;

typedef struct
{
   char         id[MD3_IDP3];
   int            version;			/**** JDH - this field comes BEFORE filename (DarkPlaces) ****/
   char         filename[MAX_QPATH];
   int            flags;            //not used
   int            num_frames;
   int            num_tags;
   int            num_surfaces;
   int            num_skins;         //not used, older format thing?
   int            frame_offs;       //offset to frame
   int            tag_offs;          //offset to tags
   int            surface_offs;      //offset to surfaces
   int			  end_offs;			/**** JDH - added this field (DarkPlaces) ****/
} md3header_t;

typedef struct
{
   char     id[MD3_IDP3];
   char     name[MAX_QPATH];
   int      flags;            //not used
   int      num_surf_frames;
   int      num_surf_shaders;
   int      num_surf_verts;
   int      num_surf_tris;
   int      tris_offs;
   int      shader_offs;
   int      tc_offs;
   int      vert_offs;
   int      end_offs;
} md3surface_t;				// this corresponds to DP's md3_mesh_t

typedef struct
{
   byte   v[3];         // scaled byte to fit in frame mins/maxs
   byte   lightnormalindex;
} md3trivertx_t;

//**** md3 disk format ****//
typedef struct
{
   short         vec[3];	 // The vertex for this face (scale down by 64.0f)
   short         normal;     // high byte is pitch, low byte is yaw
} md3vert_t;

typedef struct
{
   char    name[MAX_QPATH];
   int      index;               // This stores the indices into the vertex and texture coordinate arrays
} md3shader_t;

typedef struct
{
   float   s;
   float   t;
} md3st_t;

typedef struct
{
   int      tri[3];
} md3tri_t;

typedef struct
{
   char     name[MAX_QPATH];
   vec3_t   pos;
   vec3_t   rot[MD3_MAX_TRIANGLES];
} md3tag_t;

typedef struct
{
   vec3_t         mins;
   vec3_t         maxs;
   vec3_t         pos;
   float         radius;
   char         name[MD3_3DS];
/****JDH****/
//   md3trivertx_t   verts[1];         //variable sized
/****JDH****/
} md3frame_t;


//**** md3 memory format ****//

//different size
/*****JDH*****
typedef struct
{
   vec3_t         vec;              // The vertex for this face (scale down by 64.0f)
   vec3_t         normal;           // This stores some crazy normal values (not sure...)
} md3vert_mem_t;

//same
typedef struct
{
   float   s;
   float   t;
} md3st_mem_t;

//same
typedef struct
{
   int      tri[MD3_MAX_TRIANGLES];
} md3tri_mem_t;

//extra
typedef struct
{
   char    name[MAX_QPATH];
   int      index;            //not used
   int      texnum;
} md3shader_mem_t;

//diffrent
typedef struct md3surface_mem_s
{
   char     id[MD3_IDP3];      //should be IDP3
   char     name[MAX_QPATH];   //name of this surface
   int      flags;            //not used yet
   int      num_surf_frames;   //number of frames in this surface
   int      num_surf_shaders;  //number of shaders in this surface
   int      num_surf_verts;   //number of vertices in this surface
   int      num_surf_tris;      //number of triangles in this surface

   //offsets to the verious parts
   int      offs_shaders;
   int      offs_tris;
   int      offs_tc;
   int      offs_verts;
   int      offs_end;
} md3surface_mem_t;

//same - not used yet
typedef struct
{
   char     name[MAX_QPATH];
   vec3_t   pos;
   vec3_t   rot[MD3_MAX_TRIANGLES];
} md3tag_mem_t;

//same
typedef struct
{
   vec3_t         mins;
   vec3_t         maxs;
   vec3_t         pos;
   float         radius;
   char         name[MD3_3DS];          // The modeler used to create the model (I.E. "3DS Max")
   md3trivertx_t   verts[1];            //variable sized
} md3frame_mem_t;

//same
typedef struct
{
   char         id[MD3_IDP3];         //should be IDP3
   int            version;            //should be 15
   char         filename[MAX_QPATH];   //the name inside md3
   int            flags;               //not used (will be for particle trails etc)
   int            num_frames;            //number of frames in file
   int            num_tags;            //number of tags
   int            num_surfaces;         //number of surfaces
   int            num_skins;            //old model format leftovers, not used

   //convert these to new offsets
   int            offs_frames;         //offset to frame
   int            offs_tags;            //offset to tags
   int            offs_surfaces;         //offset to surfaces
} md3header_mem_t;
*****JDH*****/

//qboolean   r_md3fullbright_draw;

// precalculated dot products for quantized angles
extern float   r_avertexnormal_dots[16][256];

float   *shadedots_md3   = r_avertexnormal_dots[0];
float   *shadedots2_md3  = r_avertexnormal_dots[0];

#ifndef RQM_SV_ONLY

// make these seperate "works"
// only lightcolor is shared but thats k
//float   shadelightmd3, ambientlightmd3;
//float   apitchmd3, ayawmd3;
float   lightlerpoffsetmd3;

/*
=============
GL_CalcQ3LightLerp

Calculates lightning between two angles
=============
*/
/*float GL_CalcQ3LightLerp (float l1, float l2)
{
   float l, diff;

   if (l1 != l2) {
      if (l1 > l2) {
         diff = (l1 - l2) * -1;
      } else {
         diff = l2 - l1;
      }
      diff *= lightlerpoffsetmd3;
      l = l1 + diff;
   } else {
      l = l1;
   }
   // add contrast - this is a nasty hack from Pox's tut which simulates specular lighting
   // mainly introduced cos at the time he didn't know the norms.  however, my engine does
   // calculate the norms at model load time (although it doesn't really use them) so i
   // should be able to do proper specular lighting... if only i knew how :-(
   l *= l;

   return l;
}
*/
/*
=================
MD3_ConvertNormal
=================
*/
/*
void MD3_ConvertNormal( short normal, vec3_t v )
{
	float lat, lng;

	lat = (float) ((normal >> 8) & 255) * (2.0 * M_PI) / 255.0;
	lng = (float) (normal & 255) * (2.0 * M_PI) / 255.0;
	v[0] = cos ( lat ) * sin ( lng );
	v[1] = sin ( lat ) * sin ( lng );
	v[2] = cos ( lng );
}
*/
void GL_SetupMD3Light (const md3vert_t *currvert, const md3vert_t *lastvert, float alpha)
{
	float			 lightval;
	vec3_t			 l_v;
//	int				 i;
//	md3frame_t      *light, *lastlight;
//	md3trivertx_t   *lightverts, *lastlightverts;
//	float            d1, d2, l, l1, l2;

/*	// now this gets hackish
      light = (md3frame_t *)((byte *) header + header->frame_offs + (header->num_frames * e->lastpose));
      lastlight = (md3frame_t *)((byte *) header + header->frame_offs + (header->num_frames * pose));

      // damn this was a bitch to get going !!
      lightverts = light->verts;
      lastlightverts = lastlight->verts;

      // normals and vertexes come from the frame list
      // blend the light intensity from the two frames together
      if (gl_vertexlights.value && !r_md3fullbright_draw)
	  {
         lightval = R_LerpVertexLight (lightverts->lightnormalindex, lastlightverts->lightnormalindex, blend, apitchmd3, ayawmd3);

         lightval = min(lightval, 1);
      }
	  else
	  {
         d1 = (shadedots_md3[lastlightverts->lightnormalindex] * shadelightmd3 + ambientlightmd3) - (shadedots2_md3[lightverts->lightnormalindex] * shadelightmd3 + ambientlightmd3);
         d2 = (shadedots_md3[lastlightverts->lightnormalindex] * shadelightmd3 + ambientlightmd3) - (shadedots2_md3[lightverts->lightnormalindex] * shadelightmd3 + ambientlightmd3);

         l1 = min(1, (shadedots_md3[lightverts->lightnormalindex] * shadelightmd3 + ambientlightmd3) + (blend * d1) / 256);
         l2 = min(1, (shadedots_md3[lightverts->lightnormalindex] * shadelightmd3 + ambientlightmd3) + (blend * d2) / 256);

         lightval = GL_CalcQ3LightLerp (l1, l2);
      }

	if (!r_md3fullbright_draw)
	{
		for (i=0 ; i<3 ; i++)
		{
			l_v[i] = lightcolor[i] / 256 + lightval;
		}
		glColor4f (l_v[0], l_v[1], l_v[2], alpha);
	}
	else
	{
		glColor4f (lightval, lightval, lightval, alpha);
	}
*/

	// JDH: above code is very broken, so for now let's just use constant lightval:
	lightval = 0.5f;

	VectorScale(lightcolor, lightval, l_v);
	glColor4f (l_v[0], l_v[1], l_v[2], alpha);
}

/*
=================
GL_DrawQ3AliasFrame

Please fix this ANYONE!!!
=================
*/
//extern cvar_t	gl_ringalpha;

/*******JDH*******/
//void GL_DrawQ3AliasFrame (md3header_mem_t *header, int lastpose, int pose, float blend, qboolean mtex)
void GL_DrawQ3AliasFrame (const md3header_t *header, const entity_t *e, int pose, float blend, qboolean mtex)
/*******JDH*******/
{
   int               i, j, k, index;
   int               frame;
   int               lastframe;
   int               vertframeoffset;
   int               lastvertframeoffset;
   float            /*alpha, */iblend;
   md3surface_t   *surf;
   md3st_t         *tc, *xycoord;
   md3tri_t      *tris;
   md3vert_t      *verts, *vertslast, *currvert, *lastvert;
   md3shader_t		*shaders;
//  vec3_t			currnorm, lastnorm;

	r_modelalpha = GL_GetAliasAlpha (e);

	if (r_modelalpha < 1.0)
		glEnable (GL_BLEND);

	blend = bound(0, blend, 1);
	iblend = 1.0 - blend;

	surf = (md3surface_t *)((byte *)header + header->surface_offs);

	for (i = 0; i < header->num_surfaces; i++)
	{
		if (surf->num_surf_frames == 0)
		{
			surf = (md3surface_t *)((byte *)surf + surf->end_offs);
			continue;   //shouldn't ever do this, each surface should have at least one frame
		}

		//get pointer to shaders (ie. textures)
		shaders = (md3shader_t *)((byte *)surf + surf->shader_offs);
		index = shaders[e->skinnum % surf->num_surf_shaders].index;

		GL_Bind (index);

		frame = pose % surf->num_surf_frames;   //cap the frame inside the list of frames in the model
		vertframeoffset = frame * surf->num_surf_verts * sizeof(md3vert_t);
		/*******JDH*******/
		//lastframe = lastpose%surf->num_surf_frames;
		lastframe = e->lastpose % surf->num_surf_frames;
		/*******JDH*******/
		lastvertframeoffset = lastframe*surf->num_surf_verts * sizeof(md3vert_t);

		tc = (md3st_t *)((byte *)surf + surf->tc_offs);
		tris = (md3tri_t *)((byte *)surf + surf->tris_offs);
		verts = (md3vert_t *)((byte *)surf + surf->vert_offs + vertframeoffset);
		vertslast = (md3vert_t *)((byte *)surf + surf->vert_offs + lastvertframeoffset);

		/*if (blend >=1)
		{
		//glNormalPointer(GL_FLOAT, 6 * sizeof(float), (float *)verts->normal);
		//glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_SHORT, sizeof( md3vert_t ), verts->vec);
		glEnableClientState(GL_VERTEX_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, (float *)tc);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glDrawElements(GL_TRIANGLES, surf->num_surf_tris*3, GL_UNSIGNED_INT, tris);

		//glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		else*/
		{
		//interpolated

			glBegin(GL_TRIANGLES);
			//for each triangle
			for (j = 0; j < surf->num_surf_tris; j++)
			{
				//draw the poly
				for (k=0; k < 3; k++)
				{
					index = tris[j].tri[k];
					xycoord = &tc[index];

					if (mtex)
					{
						qglMultiTexCoord2f (GL_TEXTURE0_ARB, xycoord->s, xycoord->t);
						qglMultiTexCoord2f (GL_TEXTURE1_ARB, xycoord->s, xycoord->t);
					}
					else
					{
						glTexCoord2f (xycoord->s, xycoord->t);
					}

					currvert = &verts[index];
					lastvert = &vertslast[index];

					GL_SetupMD3Light (currvert, lastvert, r_modelalpha);

					glVertex3f ((currvert->vec[0] * blend + lastvert->vec[0] * iblend) / 64.0,
								(currvert->vec[1] * blend + lastvert->vec[1] * iblend) / 64.0,
								(currvert->vec[2] * blend + lastvert->vec[2] * iblend) / 64.0);

					/*MD3_ConvertNormal (currvert->normal, currnorm);
					MD3_ConvertNormal (lastvert->normal, lastnorm);

					glNormal3f (currnorm[0] * blend + lastnorm[0] * iblend,
								currnorm[1] * blend + lastnorm[1] * iblend,
								currnorm[2] * blend + lastnorm[2] * iblend);*/
				}
			}

			glEnd();
		}

		surf = (md3surface_t *)((byte *)surf + surf->end_offs);
	}

	if (r_modelalpha < 1.0)
		glDisable (GL_BLEND);

	glColor3f (1, 1, 1);
}


/*
=================
R_SetupQ3AliasFrame

From TQ148 thx Tomaz
=================
*/
void R_SetupQ3AliasFrame (int frame, const md3header_t *header, entity_t *e, qboolean mtex)
{
	float   blend;

	// im getting confirms on non existing frames here
	// but dunno if its a bug with the md3 models, so far
	// it only seems to adhere to the rocketlauncher ?
	if ((frame >= header->num_frames) || (frame < 0))
	{
		Con_DPrintf ("R_SetupQ3AliasFrame: no such frame %d\n", frame);
		frame = 0;
	}

	blend = R_SetupAliasBlend (e, frame);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	GL_DrawQ3AliasFrame (header, e, frame, blend, mtex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
=================
R_CalcMD3Lighting
=================
*/
void R_CalcMD3Lighting (entity_t *e, const model_t *clmodel)
{
	qboolean noshadow;
	float	ang_ceil, ang_floor, shadelightmd3;

	R_CalcAliasLighting (e, clmodel, &noshadow);

	// get lighting information
   // make thunderbolt and torches full light
/*   if (clmodel->modhint & MOD_THUNDERBOLT)
   {
      ambientlightmd3 = 210;
      shadelightmd3 = 0;
      r_md3fullbright_draw = true;
   } else if (clmodel->modhint & MOD_FLAME)
   {
      ambientlightmd3 = 255;
      shadelightmd3 = 0;
      r_md3fullbright_draw = true;
   } else
   {
      // normal lighting
      r_md3fullbright_draw = false;
      ambientlightmd3 = shadelightmd3 = R_LightPoint (e->origin);
      for (lnum = 0 ; lnum < MAX_DLIGHTS ; lnum++)
	  {
         if (cl_dlights[lnum].die < cl.time || !cl_dlights[lnum].radius)   continue;
         VectorSubtract (e->origin, cl_dlights[lnum].origin, dist);
         add = cl_dlights[lnum].radius - VectorLength (dist);
         if (add > 0)
		 {
            if (gl_vertexlights.value)
			{
               if (!radiusmax || cl_dlights[lnum].radius > radiusmax)
			   {
                  radiusmax = cl_dlights[lnum].radius;
                  VectorCopy (cl_dlights[lnum].origin, vertexlightmd3);
               }
            }

            VectorCopy (bubblecolor[cl_dlights[lnum].type], dlight_color);
//			VectorCopy (dlightcolor[cl_dlights[lnum].type], dlight_color);
           for (i=0 ; i<3 ; i++)
			{
//               lightcolor[i] = lightcolor[i] + (dlight_color[i] * add) * 2;
//               lightcolor[i] += (dlight_color[i] * add) / 255.0;
                 lightcolor[i] += dlight_color[i] * add;
              if (lightcolor[i] > 256)
			   {
					for (j = 0; j < 3; j++)
					{
						if (j != i)
							lightcolor[j] *= 2.0/3.0;
					}
			#if 0
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
#endif
               }
            }
         }
      }

      // calculate pitch and yaw for vertex lighting
      if (gl_vertexlights.value)
	  {
         apitchmd3 = e->angles[0];
         ayawmd3 = e->angles[1];
         if (!radiusmax)
		 {
            vlight_pitch = 45;
            vlight_yaw = 45;
         } else
		 {
            VectorSubtract (vertexlightmd3, e->origin, dist);
            vectoangles (dist, ang);
            vlight_pitch = ang[0];
            vlight_yaw = ang[1];
         }
      }

      // clamp lighting so it doesn't overbright as much
      ambientlightmd3 = min(128, ambientlightmd3);

      if (ambientlightmd3 + shadelightmd3 > 192)
	  {
         shadelightmd3 = 192 - ambientlightmd3;
      }

      // always give the gun some light
      if (e == &cl.viewent && ambientlightmd3 < 24)
	  {
         ambientlightmd3 = shadelightmd3 = 24;
      }

      // never allow players to go totally black
      if (clmodel->modhint & MOD_PLAYER)
	  {
         if (ambientlightmd3 < 8)
		 {
            ambientlightmd3 = shadelightmd3 = 8;
            r_md3fullbright_draw = false;
         }
      }
   }

   if (r_fullbrightskins.value && (clmodel->modhint & MOD_PLAYER))
   {
      ambientlightmd3 = shadelightmd3 = 128;
      r_md3fullbright_draw = true;
   }
*/
   if (gl_vertexlights.value)
   {
      // YUCK YUCK YUCK
      shadedots_md3= r_avertexnormal_dots[((int)(e->angles[1])) & (16 - 1)];
   } else
   {
      // add pitch angle so lighting changes when looking up/down (mainly for viewmodel)
      lightlerpoffsetmd3 = e->angles[1] * (16 / 360.0);

      ang_ceil = ceil(lightlerpoffsetmd3);
      ang_floor = floor(lightlerpoffsetmd3);

      lightlerpoffsetmd3 = ang_ceil - lightlerpoffsetmd3;

      // it gives me the crepaas !!
      shadedots_md3  = r_avertexnormal_dots[(int)ang_ceil & (16 - 1)];
      shadedots2_md3 = r_avertexnormal_dots[(int)ang_floor & (16 - 1)];

      shadelightmd3 = (lightcolor[0] + lightcolor[1] + lightcolor[2]) / 3.0;

	if (e->lastShadeLight)
	  {
         shadelightmd3 = (shadelightmd3 + e->lastShadeLight) / 2;
      }

      if (shadelightmd3)
	  {
         e->lastShadeLight = shadelightmd3;
      }
	  else
	  {
         e->lastShadeLight = 1;
      }
   }
}

/*
=================
R_DrawQ3Model

Modified Dr Labmans
md3 code splitting it
up to make it easier
to add dynamic lighting code
=================
*/
void R_DrawQ3Model (entity_t *e)
{
   md3header_t      *header;
   //md3shader_t      *shader;
   md3surface_t   *surf;

/*	if (!gl_notrans.value)	// always true if not -nehahra
	{
		r_modelalpha = e->transparency;
		if (r_modelalpha == 0)
			r_modelalpha = 1.0;
	}
	else
	{
		r_modelalpha = 1.0;
	}
*/
/*************** FIXME: combine with alias model code! ***************/


   R_CalcMD3Lighting (e, e->model);

   // Get the model data
   header = (md3header_t *)Mod_Extradata (e->model);

   // locate the proper data "huh surface related"!
   surf = (md3surface_t *)((byte *)header + header->surface_offs);

   c_alias_polys += surf->num_surf_tris;

/*******JDH*******
   //get pointer to shaders
   shader = (md3shader_t *)((byte *)surf + surf->shader_offs);

	//shaders = shader[(currententity->skinnum%surf->num_surf_shaders)].texnum;
	shaders = shader[ e->skinnum % surf->num_surf_shaders ].index;

   if (shaders) {
      GL_Bind(shaders);
   } else {
      GL_Bind(0);
   }
*******JDH*******/

   glPushMatrix();

   //interpolate unless its the viewmodel
   if (gl_interpolate_animation.value && (e != &cl.viewent))
   {
      R_BlendedRotateForEntity (e);
   }
   else
   {
      R_RotateForEntity (e);
   }

   if (gl_smoothmodels.value)
      glShadeModel (GL_SMOOTH);

   if (gl_affinemodels.value)
      glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

   R_SetupQ3AliasFrame (e->frame, header, e, gl_mtexable);
   glShadeModel (GL_FLAT);

   if (gl_affinemodels.value)
      glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

   glPopMatrix();
}

#endif		//#ifndef RQM_SV_ONLY

/*
=================
RadiusFromMD3Bounds
=================
*/
/*float RadiusFromMD3Bounds (vec3_t mins, vec3_t maxs)
{
   int         i;
   vec3_t      corner;

   for (i=0 ; i<3 ; i++) {
      corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);
   }
   return VectorLength (corner);
}
*/
/*
typedef struct modflags_s
{
	const char *name;
	int  flags;
} modflags_t;

//Hack to give .md3 files renamed to .mdl rotate effects - Eradicator
modflags_t mod_flags[] =
{
	{"g_shot",   EF_ROTATE},
	{"g_nail",   EF_ROTATE},
	{"g_nail2",  EF_ROTATE},
	{"g_rock",   EF_ROTATE},
	{"g_rock2",  EF_ROTATE},
	{"g_light",  EF_ROTATE},
	{"armor",    EF_ROTATE},
	{"backpack", EF_ROTATE},
	{"w_g_key",  EF_ROTATE},
	{"w_s_key",  EF_ROTATE},
	{"m_g_key",  EF_ROTATE},
	{"m_s_key",  EF_ROTATE},
	{"b_g_key",  EF_ROTATE},
	{"b_s_key",  EF_ROTATE},
	{"quaddama", EF_ROTATE},
	{"invisibl", EF_ROTATE},
	{"invulner", EF_ROTATE},
	{"suit",     EF_ROTATE},
	{"end1",     EF_ROTATE},
	{"end2",     EF_ROTATE},
	{"end3",     EF_ROTATE},
	{"end4",     EF_ROTATE},
	{"jetpack",  EF_ROTATE},
	{"cube",     EF_ROTATE},
	{"boots",    EF_ROTATE},
	{"missile",  EF_ROCKET},
	{"gib1",     EF_GIB},
	{"gib2",     EF_GIB},
	{"gib3",     EF_GIB},
	{"h_player", EF_GIB},
	{"h_dog",    EF_GIB},
	{"h_mega",   EF_GIB},
	{"h_guard",  EF_GIB},
	{"h_wizard", EF_GIB},
	{"h_knight", EF_GIB},
	{"h_hellkn", EF_GIB},
	{"h_zombie", EF_GIB},
	{"h_shams",  EF_GIB},
	{"h_shal",   EF_GIB},
	{"h_ogre",   EF_GIB},
//	{"armor",    EF_GIB},
	{"h_demon",  EF_GIB},
	{"grenade",  EF_GRENADE},
	{"w_spike",  EF_TRACER},
	{"k_spike",  EF_TRACER2},
	{"v_spike",  EF_TRACER3},
	{"zom_gib",  EF_ZOMGIB}
};

#define NUM_MOD_FLAGS (sizeof(mod_flags) / sizeof(mod_flags[0]))
*/

//Hack to give .md3 files renamed to .mdl the right effects - Eradicator

static const char *models_rotate[] =
{
	"g_shot", "g_nail", "g_nail2", "g_rock", "g_rock2", "g_light",
	"armor", "backpack", "quaddama", "invisibl", "invulner", "suit",
	"w_g_key", "w_s_key", "m_g_key", "m_s_key", "b_g_key", "b_s_key",
	"end1", "end2", "end3", "end4", "jetpack", "cube", "boots", NULL
};
static const char *models_gib[] =
{
	"gib1", "gib2", "gib3"/*, "h_player", "h_dog", "h_mega", "h_guard",
	"h_wizard", "h_knight", "h_hellkn", "h_zombie", "h_shams", "h_shal",
	"h_ogre", "h_demon", "armor"*/, NULL
};
static const char *models_rocket[]  = {"missile", NULL};
static const char *models_grenade[] = {"grenade", NULL};
static const char *models_tracer[]  = {"w_spike", NULL};
static const char *models_zomgib[]  = {"zom_gib", NULL};
static const char *models_tracer2[] = {"k_spike", NULL};
static const char *models_tracer3[] = {"v_spike", NULL};

static aliastype_t mod_flags[] =
{
	{EF_ROCKET,  models_rocket},
	{EF_GRENADE, models_grenade},
	{EF_GIB,     models_gib},
	{EF_ROTATE,  models_rotate},
	{EF_TRACER,  models_tracer},
	{EF_ZOMGIB,  models_zomgib},
	{EF_TRACER2, models_tracer2},
	{EF_TRACER3, models_tracer3}
};

#define NUM_EF_FLAGS (sizeof(mod_flags)/sizeof(mod_flags[0]))

vec3_t md3bboxmins, md3bboxmaxs;

/*
=================
Mod_GetQ3ModelFlags
=================
*/

int Mod_GetQ3ModelFlags (const char *modname)
{
	int flags = Mod_GetAliasType (modname, "progs/", mod_flags, NUM_EF_FLAGS);

	return (flags < 0) ? 0 : flags;

/*	char *ext, shortname[MAX_QPATH];
	int  i, j;

	if (!COM_FilenamesEqualn(modname, "progs/", 6))
		return 0;		// anything not in progs folder has no flags

	modname += 6;		// skip over folder prefix

	ext = COM_FileExtension (modname);
	if (!COM_FilenamesEqual(ext, "mdl"))
		return 0;

	Q_strncpy (shortname, sizeof(shortname), modname, ext-1-modname);

	for (i = 0; i < NUM_EF_FLAGS; i++)
	{
		for (j = 0; mod_flags[i][j]; j++)
		{
			if (COM_FilenamesEqual (shortname, mod_flags[i][j]))
				return (1 << i);
		}
	}

	return 0;
*/
}

static const char *md3_paths[] = {NULL, "textures/", NULL};		// first one is dynamically set

/*
=================
Mod_LoadQ3Model
=================
*/
void Mod_LoadQ3Model (model_t *mod, const void *buffer)
{
	md3header_t		*header;
	md3surface_t	*surf;
	int				 i, j, picmip_flag;
	md3shader_t		*shader;
	md3frame_t		*frame;
//	md3ver_t		*vert;
	byte			*modeldata;
	const char		**pathlist;
	const char		*namelist[2];
	char			path[MAX_OSPATH];

	header = (md3header_t *) buffer;
	if (header->version != 15)
	{
		Con_Printf ("\x02""ERROR: incorrect version of MD3 file %s\n", mod->name);
		return;
	}

	Con_DPrintf ("Loading md3 model...%s (%s)\n", header->filename, mod->name);

	frame = (md3frame_t *) ((byte *) header + header->frame_offs);
	for (i = 0; i < header->num_frames; i++, frame++)
	{
		VectorCopy (frame->pos, md3bboxmins);
		VectorCopy (frame->pos, md3bboxmaxs);

		for (j = 0; j < 3; j++)
		{
			if (frame->mins[j] < md3bboxmins[j])
				md3bboxmins[j] = frame->mins[j];
			if (frame->maxs[j] > md3bboxmaxs[j])
				md3bboxmaxs[j] = frame->maxs[j];
		}

	}

#ifndef RQM_SV_ONLY
	picmip_flag = gl_picmip_all.value ? TEX_MIPMAP : 0;
#else
	picmip_flag = 0;
#endif

	surf = (md3surface_t *) ((byte *)header + header->surface_offs);
	for (i=0; i < header->num_surfaces; i++)
	{
		if (*(long *)surf->id != MD3IDHEADER)
		{
			Con_Printf ("\x02""MD3 bad surface for: %s\n", header->filename);
			break;
		}

		/*vert = (md3vert_t *) ((byte *) surf + surf->vert_offs);
		for (j=0; j < surf->num_surf_verts * surf->num_surf_frames; j++, vert++)
		{
			float lat;
			float lng;

			//convert verts from shorts to floats
			md3vert_mem_t   *mem_vert = (md3vert_mem_t *)((byte *)mem_head + posn);
			md3vert_t      *disk_vert = (md3vert_t *)((byte *)surf + surf->vert_offs + j * sizeof(md3vert_t));

			mem_vert->vec[0] = (float)disk_vert->vec[0] / 64.0f;
			mem_vert->vec[1] = (float)disk_vert->vec[1] / 64.0f;
			mem_vert->vec[2] = (float)disk_vert->vec[2] / 64.0f;

			//work out normals
			lat = (disk_vert->normal + 255) * (2 * 3.141592654) / 255;
			lng = ((disk_vert->normal >> 8) & 255) * (2 * 3.141592654) / 255;

			mem_vert->normal[0] = cos (lat) * sin (lng);
			mem_vert->normal[1] = sin (lat) * sin (lng);
			mem_vert->normal[2] = cos (lng);
		} */

		// load all the external textures:
		shader = (md3shader_t *)((byte *)surf + surf->shader_offs);
		for (j=0; j < surf->num_surf_shaders; j++, shader++)
		{
		#ifndef RQM_SV_ONLY
			//shader->index = GL_LoadTextureImage ("", shader->name, NULL, picmip_flag | TEX_BLEND, mod_loadpath->dir_level);
			namelist[0] = COM_SkipPath (shader->name);
			namelist[1] = NULL;
			if (namelist[0] != shader->name)
			{
				Q_strncpy (path, sizeof(path), shader->name, namelist[0]-shader->name);
				md3_paths[0] = path;
				pathlist = &md3_paths[0];
			}
			else
			{
				pathlist = &md3_paths[1];		// no custom path
			}

			shader->index = GL_LoadTextureImage_MultiSource (pathlist, namelist, NULL, picmip_flag | TEX_BLEND, mod_loadpath->dir_level);
			if (!shader->index)
			{
				Con_Printf ("\x02""Model: %s  Texture missing: %s\n", mod->name, shader->name);
			}
		#else
			shader->index = 0;
		#endif
		}

		surf = (md3surface_t *)((byte *)surf + surf->end_offs);
	}

	modeldata = Cache_Alloc (&mod->cache, header->end_offs, mod->name);
	if (!modeldata)
	{
		Con_DPrintf ("\x02""cache alloc failed...%s (%s)\n", header->filename, mod->name);
		return;   //cache alloc failed
	}

	memcpy (modeldata, buffer, header->end_offs);

	mod->type = mod_md3;
//	mod->aliastype = MD3IDHEADER;
	mod->numframes = header->num_frames;

	mod->modhint = Mod_GetAliasHint (mod->name);
	mod->flags = header->flags;

	if (mod->modhint == MOD_HEAD)
		mod->flags |= EF_GIB;
	else
		mod->flags |= Mod_GetQ3ModelFlags (mod->name);

	VectorScale (md3bboxmaxs, 1.0/64.0, mod->maxs);
	VectorScale (md3bboxmins, 1.0/64.0, mod->mins);
}

#endif		// #ifndef RQM_SV_ONLY
