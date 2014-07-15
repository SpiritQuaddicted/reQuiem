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

// refresh.h -- public interface to refresh functions

#ifndef __RENDER_H__
#define __RENDER_H__

#define	TOP_RANGE	16			// soldier uniform colors
#define	BOTTOM_RANGE	96

//=============================================================================

typedef struct entity_s
{
	qboolean		forcelink;		// model changed

//	int				update_type;

	entity_state_t	baseline;		// to fill in defaults in updates

	double			msgtime;		// time of last update
	vec3_t			msg_origin, msg_origin_prev;	// last two updates
	vec3_t			origin;
	vec3_t			msg_angles, msg_angles_prev;		// last two updates
	vec3_t			angles;	

	struct model_s	*model;			// NULL = no model
	struct efrag_s	*efrag;			// linked list of efrags
	int				modelindex;
	int				frame;
	float			syncbase;		// for client-side animations
	byte			*colormap;
	int				effects;		// light, particals, etc
	int				skinnum;		// for Alias models
	int				visframe;		// last frame this entity was found in an active leaf
											
//	int				dlightframe;		// dynamic lighting
//	int				dlightbits;
	
	vec3_t			trail_origin;	
	qboolean		traildrawn;
	
//	int				trivial_accept;
//	struct mnode_s	*topnode;		// for bmodels, first world node
									// that splits bmodel, or NULL if not split

	// fenix@io.com: model animation interpolation
	float			frame_start_time;
	float			frame_interval;
	int				lastpose, currpose;
	
	// fenix@io.com: model transform interpolation
	float			translate_start_time;
	vec3_t			origin1, origin2;
	
	float			rotate_start_time;
	vec3_t			angles1, angles2;

	// nehahra support
	qboolean		transignore;
	float			transparency, smokepuff_time;
	float			fullbright;			// if non-zero, lighting is not applied to entity 

	// MD3 support (interpolation)
	float			lastShadeLight;

#ifdef HEXEN2_SUPPORT
	int				scale;			// for Alias models
	int				drawflags;		// for Alias models
	int				abslight;		// for Alias models
	byte			colorshade;
#endif
} entity_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vrect_t		vrect;				// subwindow in video for refresh
									// FIXME: not need vrect next field here?
	vrect_t		aliasvrect;			// scaled Alias version
	int			vrectright, vrectbottom;	// right & bottom screen coords
	int			aliasvrectright, aliasvrectbottom; // scaled Alias versions
	float		vrectrightedge;			// rightmost right edge we care about,
										//  for use in edge list
	float		fvrectx, fvrecty;		// for floating-point compares
	float		fvrectx_adj, fvrecty_adj;	// left and top edges, for clamping
	int			vrect_x_adj_shift20;		// (vrect.x + 0.5 - epsilon) << 20
	int			vrectright_adj_shift20;		// (vrectright + 0.5 - epsilon) << 20
	float		fvrectright_adj, fvrectbottom_adj; // right and bottom edges, for clamping
	float		fvrectright;			// rightmost edge, for Alias clamping
	float		fvrectbottom;			// bottommost edge, for Alias clamping
	float		horizontalFieldOfView;		// at Z = 1.0, this many X is visible 
										// 2.0 = 90 degrees
	float		xOrigin;			// should probably always be 0.5
	float		yOrigin;			// between be around 0.3 to 0.5

	vec3_t		vieworg;
	vec3_t		viewangles;
	
	float		fov_x, fov_y;

	int		ambientlight;
} refdef_t;


// refresh
extern	refdef_t	r_refdef;
extern	vec3_t		r_origin, vpn, vright, vup;

extern	struct texture_s *r_notexture_mip;

extern	entity_t	r_worldentity;

void R_Init (void);
void R_InitTextures (void);
void R_InitEfrags (void);
void R_RenderView (void);		// must set r_refdef first
//void R_ViewChanged (vrect_t *pvrect, int lineadj, float aspect); // called whenever r_refdef or vid change
void R_InitSky (miptex_t *mt);		// called at level load

void R_AddEfrags (entity_t *ent);
void R_RemoveEfrags (entity_t *ent);

qboolean Img_HasFullbrights (byte *pixels, int size);

void R_DrawSprites (void);
#ifdef HEXEN2_SUPPORT
	void R_DrawSprite (entity_t *);
#endif

void R_DrawAliasModel (entity_t *);
void R_CalcAliasLighting (const entity_t *ent, const model_t *clmodel, qboolean *noshadow);
void R_DrawQ3Model (entity_t *e); 

float GL_GetAliasAlpha (const entity_t *ent);
float R_SetupAliasBlend (entity_t *ent, int pose);

void Set_Interpolated_Weapon_f (cmd_source_t src);

void R_NewMap (void);

// particles

typedef enum trail_type_s
{
	ROCKET_TRAIL, GRENADE_TRAIL, BLOOD_TRAIL, TRACER1_TRAIL, SLIGHT_BLOOD_TRAIL,
	TRACER2_TRAIL, VOOR_TRAIL, LAVA_TRAIL, BUBBLE_TRAIL, NEHAHRA_SMOKE,

#ifdef HEXEN2_SUPPORT
	ICE_TRAIL, SPIT_TRAIL, SPELL_TRAIL, VORPAL_TRAIL, STAFF_TRAIL, MAGICMISSILE_TRAIL,
	BONESHARD_TRAIL, SCARAB_TRAIL, ACID_TRAIL, BLOODSHOT_TRAIL
#endif
} trail_type_t;


typedef enum
{
	pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2, 
#ifdef HEXEN2_SUPPORT
	pt_fastgrav,
	pt_rain,
	pt_c_explode,
	pt_c_explode2,
	pt_spit,
	pt_fireball,
	pt_ice,
	pt_spell,
	pt_test,
	pt_quake,
	pt_rd,			// rider's death
	pt_vorpal,
	pt_setstaff,
	pt_magicmissile,
	pt_boneshard,
	pt_scarab,
	pt_acidball,
	pt_darken,
	pt_snow,
	pt_gravwell,
	pt_redfire
#endif
} ptype_t;

typedef struct particle_s
{
	vec3_t		org;
	float		color;
	vec3_t		vel;
	float		ramp;
	float		starttime;	// JDH: added for demo rewind
	float		endtime;
	ptype_t		type;
	struct particle_s *next;
#ifdef HEXEN2_SUPPORT
	vec3_t		min_org;	// snow
	vec3_t		max_org;	// snow
	byte		flags;		// snow
	byte		count;		// snow
#endif
} particle_t;


#define PARTICLE_INACTIVE(p) ((cl.time < (p)->starttime) || (cl.time > (p)->endtime)) 


void R_ParseParticleEffect (qboolean parse_only);
void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void R_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type);

void R_EntityParticles (entity_t *ent);
void R_BlobExplosion (vec3_t org);
void R_ParticleExplosion (vec3_t org);
void R_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength);
void R_LavaSplash (vec3_t org);
void R_TeleportSplash (vec3_t org);

#ifdef HEXEN2_SUPPORT
  void Hexen2_LoadParticleTexture (void);
  void R_UpdateParticles (void);
  void Hexen2_DrawParticles (void);
  void Hexen2_RocketTrail (vec3_t start, vec3_t end, int type);

  void R_RainEffect (vec3_t org,vec3_t e_size,int x_dir, int y_dir,int color,int count);
  void R_SnowEffect (vec3_t org1,vec3_t org2,int flags,vec3_t alldir,int count);
  void R_ColoredParticleExplosion (vec3_t org,int color,int radius,int counter);
  void R_RunParticleEffect2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, ptype_t effect, int count);
  void R_RunParticleEffect3 (vec3_t org, vec3_t box, int color, ptype_t effect, int count);
  void R_RunParticleEffect4 (vec3_t org, float radius, int color, ptype_t effect, int count);
  void R_RunQuakeEffect (vec3_t org, float distance);
  void R_RiderParticle( int count, vec3_t origin);
  void R_GravityWellParticle (int count, vec3_t origin, int color);
  void R_DarkFieldParticles (entity_t *ent);
  void R_SunStaffTrail (vec3_t source, vec3_t dest);
#endif

void R_InitParticles (void);
void R_ClearParticles (void);
void R_DrawParticles (void);
void R_ToggleParticles_f (cmd_source_t src);

void R_PushDlights (void);
void R_DrawWaterSurfaces (void);


// surface cache related
extern qboolean	r_cache_thrash;	// set if thrashing the surface cache
void D_FlushCaches (void);

#ifndef GLQUAKE
int D_SurfaceCacheForRes (int width, int height);
void D_DeleteSurfaceCache (void);
void D_InitCaches (void *buffer, int size);
void R_SetVrect (vrect_t *pvrect, vrect_t *pvrectin, int lineadj);
#endif

#endif
