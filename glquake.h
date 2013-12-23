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

#ifndef _WIN32
#  define RGB(r,g,b) ((DWORD)(((byte)(r)|((WORD)((byte)(g))<<8))|(((DWORD)(byte)(b))<<16)))
#  define APIENTRY
#endif

#include <GL/gl.h>

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

extern	int	texture_extension_number;

//extern	float	gldepthmin, gldepthmax;

// added by joe
#define	TEX_COMPLAIN		0x0001
#define TEX_MIPMAP			0x0002
#define TEX_ALPHA			0x0004
#define TEX_LUMA			0x0008
#define TEX_FULLBRIGHT		0x0010
//#define	TEX_BRIGHTEN		0x0020
#define	TEX_BMODEL			0x0020
#define TEX_PLAYER			0x0040
#define	TEX_BLEND			0x0080
#define TEX_NOSTRETCH		0x0100
#define TEX_NOTILE			0x0200
#define TEX_NOFILTER		0x0400		// doesn't obey gl_texturemode

#ifdef HEXEN2_SUPPORT
#  define TEX_ALPHA_MODE1	0x1000
#  define TEX_ALPHA_MODE2	0x2000
#  define TEX_ALPHA_MODE3	0x4000
#endif

#define	MAX_GLTEXTURES	2048		// JDH: was 1024
#define MAX_TMUS 4

#ifndef GL_COMBINE
#  define GL_COMBINE                        0x8570
#  define GL_COMBINE_RGB                    0x8571
#  define GL_RGB_SCALE                      0x8573
#  define GL_CONSTANT                       0x8576
#  define GL_PREVIOUS                       0x8578
#  define GL_SOURCE0_RGB                    0x8580
#  define GL_SOURCE1_RGB                    0x8581
//#  define GL_OPERAND0_RGB                   0x8590
//#  define GL_OPERAND1_RGB                   0x8591
#endif

struct gltexture_s;

// filtering modes:
typedef struct
{
	const char	*name;
	int		minimize, maximize;
} glmode_t;

#define GL_NUM_TEXMODES 6

extern const glmode_t gl_texmodes[GL_NUM_TEXMODES];

void GL_TextureInit (void);
void GL_SelectTMUTexture (GLenum target);
void GL_DisableMultitexture (void);
void GL_EnableMultitexture (void);
void GL_EnableTMU (GLenum target);
void GL_DisableTMU (GLenum target);

void GL_Upload32 (struct gltexture_s *glt, const unsigned *data, int *width, int *height, int texflags);

int GL_LoadTexture (const char *identifier, int width, int height, void *data, int texflags, int bytesperpixel);
int GL_LoadPicTexture (const char *name, mpic_t *pic, void *data, int texflags, int bytesperpixel);
void GL_UpdatePicTexture (mpic_t *pic, const unsigned *data);

byte *GL_LoadImage (const char *path, const char *filename, int texflags, int item_dirlevel);
byte *GL_LoadImage_MultiSource (const char *paths[], const char *filenames[], int texflags, int item_dirlevel);

int GL_LoadTextureImage (const char *path, const char *filename, const char *identifier, int texflags, int item_dirlevel);
int GL_LoadTextureImage_MultiSource (const char *paths[], const char *filenames[], const char *identifier, int texflags, int item_dirlevel);

mpic_t *GL_LoadPicImage (const char *path, const char *filename, const char *id, int texflags, int item_dirlevel);
mpic_t *GL_LoadPicImage_MultiSource (const char *paths[], const char *filenames[], const char *id, int texflags, int item_dirlevel);

void GL_ScaleDimensions (int w, int h, int *scaled_w, int *scaled_h, int texflags);

void Scrap_Upload (void);

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;

extern glvert_t	glv;

extern	int	glx, gly, glwidth, glheight;

extern	const char *gl_vendor;
extern	const char *gl_renderer;
extern	const char *gl_version;
extern	const char *gl_extensions;

void GL_Bind (int texnum);

// Multitexture
#define	GL_TEXTURE0_ARB 		0x84C0
#define	GL_TEXTURE1_ARB 		0x84C1
#define	GL_TEXTURE2_ARB 		0x84C2
#define	GL_TEXTURE3_ARB 		0x84C3
#define GL_MAX_TEXTURE_UNITS_ARB	0x84E2

typedef void (APIENTRY *lpMTexFUNC)(GLenum, GLfloat, GLfloat);
typedef void (APIENTRY *lpSelTexFUNC)(GLenum);

extern lpMTexFUNC qglMultiTexCoord2f;
extern lpSelTexFUNC qglActiveTexture;

extern qboolean gl_mtexable;
extern int gl_textureunits;

// normalizing factor so player model works out to about 1 pixel per triangle
#define ALIAS_BASE_SIZE_RATIO	(1.0 / 11.0)

//#define	MAX_LBM_HEIGHT		480

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)

#define BACKFACE_EPSILON	0.01


void R_TimeRefresh_f (cmd_source_t src);
void R_ReadPointFile_f (cmd_source_t src);

//texture_t *R_TextureAnimation (texture_t *base);

//====================================================

void QMB_InitParticles (void);
void QMB_ClearParticles (void);
void QMB_DrawParticles (void);

void QMB_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void QMB_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type);
void QMB_BlobExplosion (vec3_t org);
void QMB_ParticleExplosion (vec3_t org);
void QMB_LavaSplash (vec3_t org);
void QMB_TeleportSplash (vec3_t org);
void QMB_InfernoFlame (vec3_t org);
void QMB_StaticBubble (entity_t *ent);
void QMB_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength);
void QMB_TorchFlame (vec3_t org, float size, float time);
void QMB_MissileFire (vec3_t org, vec3_t start, vec3_t end);
void QMB_ShamblerCharge (vec3_t org);
void QMB_LightningBeam (vec3_t start, vec3_t end);
void QMB_Lightning_Splash (vec3_t org);
void QMB_GenSparks (vec3_t org, byte col[3], float count, float size, float life);

extern	qboolean	qmb_initialized;

//int CheckParticles (void);

//====================================================


extern	entity_t	r_worldentity;
extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
//extern	entity_t	*currententity;
extern	int		r_visframecount;	// ??? what difs?
extern	int		r_framecount;
extern	mplane_t	frustum[4];
extern	int		c_brush_polys, c_alias_polys;


// view origin
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

// screen size info
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	mleaf_t		*r_viewleaf2, *r_oldviewleaf2;	// for watervis hack
extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

#define NUM_DETAIL_LEVELS 16

extern	int	currenttexture;
extern	int	particletexture;
extern	int	playertextures;
extern	int	skyboxtextures;
extern	int	underwatertexture, /*detailtexture,*/ chrometexture2;	// JT added chrometexture2
extern	int detailtexture[NUM_DETAIL_LEVELS];

extern	cvar_t	r_drawentities;
extern	cvar_t	r_drawworld;
extern	cvar_t	r_drawviewmodel;
extern	cvar_t	r_viewmodelsize;
extern	cvar_t	r_speeds;
extern	cvar_t	r_waterwarp;
extern	cvar_t	r_fullbright;
extern	cvar_t	r_lightmap;
extern	cvar_t	r_shadows;
extern	cvar_t	r_wateralpha;
extern	cvar_t	r_dynamic;
extern	cvar_t	r_novis;
extern	cvar_t	r_fullbrightskins;
extern	cvar_t	r_skytype;
extern	cvar_t	r_skycolor;
extern	cvar_t	r_skybox;
extern	cvar_t	r_farclip;

// fenix@io.com: model interpolation
extern  cvar_t  gl_interpolate_animation;
extern  cvar_t  gl_interpolate_transform;

extern	cvar_t	gl_clear;
extern	cvar_t	gl_cull;
extern	cvar_t	gl_poly;
//extern	cvar_t	gl_ztrick;
extern	cvar_t	gl_smoothmodels;
extern	cvar_t	gl_affinemodels;
extern	cvar_t	gl_polyblend;
extern	cvar_t	gl_flashblend;
extern	cvar_t	gl_nocolors;
extern	cvar_t	gl_loadlitfiles;
extern	cvar_t	gl_doubleeyes;
extern	cvar_t	gl_interdist;
extern  cvar_t  gl_waterfog;
extern  cvar_t  gl_waterfog_density;
extern  cvar_t  gl_detail;
extern  cvar_t  gl_caustics;
extern	cvar_t	gl_fb_world;
extern	cvar_t	gl_fb_bmodels;
extern	cvar_t	gl_fb_models;
extern  cvar_t  gl_solidparticles;
extern  cvar_t  gl_vertexlights;
extern	cvar_t	gl_lightmode;

extern  cvar_t	gl_part_explosions;
extern  cvar_t	gl_part_trails;
extern  cvar_t	gl_part_spikes;
extern  cvar_t	gl_part_gunshots;
extern  cvar_t	gl_part_blood;
extern  cvar_t	gl_part_telesplash;
extern  cvar_t	gl_part_blobs;
extern  cvar_t	gl_part_lavasplash;
extern  cvar_t	gl_part_inferno;
extern	cvar_t	gl_part_flames;
extern	cvar_t	gl_part_lightning;
extern	cvar_t	gl_part_spiketrails;

extern	cvar_t	gl_bounceparticles;
extern	cvar_t	gl_clipparticles;

extern	qboolean	no24bit;
extern	cvar_t		gl_externaltextures_world;
extern	cvar_t		gl_externaltextures_bmodels;
extern	cvar_t		gl_externaltextures_models;

extern	int	gl_lightmap_format;
extern	int	gl_solid_format;
extern	int	gl_alpha_format;

extern	cvar_t	gl_max_size;
extern	cvar_t	gl_playermip;
extern	cvar_t	gl_picmip;
extern	cvar_t	gl_picmip_all;

extern	cvar_t	gl_skyhack;		/****** TEMP!!! *******/

extern	int			mirrortexturenum;	// quake texturenum, not gltexturenum
extern	qboolean	mirror;
extern	mplane_t	*mirror_plane;

extern	float	r_world_matrix[16];

// vid_common_gl.c
void VID_BuildGammaTable (void);
void VID_SetPalette (unsigned char *palette);
void GL_Init (void);
extern qboolean	gl_add_ext;

// image.c
extern	int	image_width, image_height;

void Image_Init (void);
byte *Image_LoadFile (FILE *f, int filelen, const char *filename);

qboolean Image_WriteFile (char *filename, const byte *data, int width, int height);

// gl_warp.c
void GL_SubdivideSurface (model_t *mod, msurface_t *fa);
void EmitWaterPolys (msurface_t *fa, qboolean doSetup);
void CalcCausticTexCoords(float *v, float *s, float *t);
void EmitCausticsPolys (void);
void EmitWaterPolysReflection (msurface_t *fa);
//void EmitWaterPolys2 (msurface_t *fa);

// gl_sky.c
void EmitSkyPolys (msurface_t *fa, qboolean mtex);
//void R_DrawSkyChain (void);
void R_AddSkyBoxSurface (msurface_t *fa);
void R_ClearSkyBox (void);
//void R_DrawSkyBox (void);
void R_DrawSky (void);
void R_DrawSkyChainOld (void);
//qboolean R_SetSky (char *skyname);
qboolean Sky_GetBaseName (char *filename);
qboolean Sky_InitWorldspawn (const char *);		// JDH

// gl_draw.c
void GL_Set2D (void);
int StringToRGB (const char *s, byte rgb[4]);

// gl_rmain.c
qboolean R_CullBox (const vec3_t mins, const vec3_t maxs);
#ifdef _WIN32
qboolean R_CullSphere (const vec3_t centre, float radius);
#endif

void R_RotateForEntity (const entity_t *e);
void R_BlendedRotateForEntity (entity_t *e);

#ifdef HEXEN2_SUPPORT
  void R_RotateForEntity_H2 (entity_t *e);
  void R_BlendedRotateForEntity_H2 (entity_t *e);
#endif
  
void R_PolyBlend (void);
void R_BrightenRect (int left, int top, int width, int height);

#define R_BrightenScreen() R_BrightenRect(0, 0, vid.width, vid.height)

// gl_rlight.c
void R_MarkLights (dlight_t *light, QINT64 bit, mnode_t *node);
void R_AnimateLight (void);
void R_RenderDlights (void);
int R_LightPoint (const vec3_t p);
float R_GetVertexLightValue (int index, float apitch, float ayaw);
float R_LerpVertexLight (int index1, int index2, float ilerp, float apitch, float ayaw);
void R_InitVertexLights (void);
extern	float	bubblecolor[NUM_DLIGHTTYPES][3];
extern	float	vlight_pitch, vlight_yaw;
extern	vec3_t	lightspot, lightcolor;

// gl_refrag.c
void R_StoreEfrags (efrag_t **ppefrag);

// gl_mesh.c
void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr);

// gl_rsurf.c
//extern int dlightcolor[NUM_DLIGHTTYPES][3];

void EmitDetailPolys (void);
void R_DrawBrushModel (entity_t *e);
void R_DrawWorld (void);
void R_MarkLeaves (void);
void GL_BuildLightmaps (void);
void GL_ForceLightMapReload (msurface_t *);
void GL_CreateSurfaceLightmap (model_t *, msurface_t *);
void GL_BuildSurfaceDisplayList (model_t *, msurface_t *);

// gl_rmisc
void R_InitOtherTextures (void);

/*******JDH*******
void Mod_LoadQ3Model(model_t *mod, void *buffer);
void R_DrawQ3Model(entity_t *e);
*******JDH*******/

// gl_fog.c (JDH)
void Fog_Init (void);
void Fog_InitWorldspawn (const char *);
void Fog_NewMap (void);
void Fog_Reset (void);

void Fog_ParseFitzMessage (void);
void Fog_ParseNehMessage (void);

void Fog_SetupFrame (void);
void Fog_AddWaterfog (unsigned color, int contents);

void Fog_EnableGFog (void);
void  Fog_DisableGFog (void);
float Fog_GetDensity (void);
