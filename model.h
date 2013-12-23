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

#ifndef __MODEL__
#define __MODEL__

#include "modelgen.h"
#include "spritegn.h"

//#define FITZWORLD

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/

#ifdef HEXEN2_SUPPORT
  #define MAX_EXTRA_TEXTURES 156   // 255-100+1

  typedef struct
  {
	  int texnum;
	  float s, t;
  } skintex_t;

  extern skintex_t gl_extra_textures[MAX_EXTRA_TEXTURES];	// in gl_drawalias.c
#endif

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


// in memory representation
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vec3_t		position;
} mvertex_t;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2



// plane_t structure
// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct mplane_s
{
	vec3_t		normal;
	float		dist;
	byte		type;		// for texture axis selection and fast side tests
	byte		signbits;	// signx + signy<<1 + signz<<1
	byte		pad[2];
} mplane_t;

typedef struct texture_s
{
	char			name[16];
	unsigned		width, height;
	int				gl_texturenum;
	int				fb_texturenum;		// index of fullbright mask or 0
//	struct msurface_s *texturechain[2];
//	struct msurface_s **texturechain_tail[2];
	struct msurface_s *texturechain;
	int				anim_total;		// total tenths in sequence (0 = no)
	int				anim_min, anim_max;	// time for this frame min <=time< max
	struct texture_s *anim_next;		// in the animation sequence
	struct texture_s *alternate_anims;	// bmodels in frame 1 use these
	unsigned		offsets[MIPLEVELS];	// four mip maps stored
	int				isLumaTexture;		// for textures from external files, indicates fb_texturenum
										//   gets drawn via GL_ADD instead of GL_DECAL
} texture_t;


#define	SURF_PLANEBACK		0x002
#define	SURF_DRAWSKY		0x004
#define SURF_DRAWSPRITE		0x008
#define SURF_DRAWTURB		0x010
#define SURF_DRAWTILED		0x020
#define SURF_DRAWBACKGROUND	0x040
#define SURF_UNDERWATER		0x080

#ifdef HEXEN2_SUPPORT
  #define SURF_TRANSLUCENT	0x100
  #define SURF_DRAWBLACK	0x200
#endif

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct
{
	float		vecs[2][4];
	texture_t	*texture;
	int			flags;
} mtexinfo_t;

#define	VERTEXSIZE	9		/* increased from 7 (added st coords for detail texture) */

typedef struct glpoly_s
{
	struct glpoly_s	*next;
	struct glpoly_s	*chain;
	struct glpoly_s	*fb_chain;
	struct glpoly_s *luma_chain;		// next luma poly in chain
	struct glpoly_s	*caustics_chain;	// next caustic poly in chain
	struct glpoly_s	*detail_chain;		// next detail poly in chain
	int		dist;						// distance from player to poly (for detail textures)  - NOT USED YET
	int		numverts;
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;

typedef struct msurface_s
{
	int			visframe;	// should be drawn when node is crossed

	mplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges

	int/*short*/		texturemins[2];
	int/*short*/		extents[2];

	int			light_s, light_t;	// gl lightmap coordinates

	glpoly_t	*polys;			// multiple if warped
	struct	msurface_s *texturechain;

	mtexinfo_t	*texinfo;

// lighting info
	int			dlightframe;
	QINT64		dlightbits;			// must correspond with MAX_DLIGHTS

	int			lightmaptexturenum;
	byte		styles[MAXLIGHTMAPS];
	int			cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	qboolean	cached_dlight;			// true if dynamic light in cache
	byte		*samples;		// [numstyles*surfsize]
	qboolean	lightcolored;		// JDH: true only if lightmaps are 24-bit and actually
									//      contain colors where rgb channels are different
#ifdef FITZWORLD
	qboolean	culled;			// johnfitz -- for frustum culling
#endif
	float		mins[3];		// johnfitz -- for frustum culling
	float		maxs[3];		// johnfitz -- for frustum culling
//#endif
} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int				contents;		// 0, to differentiate from leafs
	int				visframe;		// node needs to be traversed if current

	float			minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// node specific
	mplane_t		*plane;
	struct mnode_s	*children[2];

	unsigned short	firstsurface;
	unsigned short	numsurfaces;
} mnode_t;

typedef struct mclipnode_s
{
	int			planenum;
	int			children[2];	// negative numbers are contents
} mclipnode_t;


typedef struct efrag_s
{
	struct mleaf_s		*leaf;
	struct efrag_s		*leafnext;
	struct entity_s		*entity;
	struct efrag_s		*entnext;
} efrag_t;


typedef struct mleaf_s
{
// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
/*******JDH*******/
	byte		*uncompressed_vis;
/*******JDH*******/

	efrag_t		*efrags;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
	byte		ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
	mclipnode_t	*clipnodes;
	mplane_t	*planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;

extern vec3_t vec3_hull_min, vec3_hull_max;

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/


// FIXME: shorten these?
typedef struct mspriteframe_s
{
	int			width;
	int			height;
	float		up, down, left, right;
	int			gl_texturenum;
	qboolean	is_tex_external;
} mspriteframe_t;

typedef struct
{
	int				numframes;
	float			*intervals;
	mspriteframe_t	*frames[1];
} mspritegroup_t;

typedef struct
{
	spriteframetype_t	type;
	mspriteframe_t		*frameptr;
} mspriteframedesc_t;

typedef struct
{
	int					type;
	int					maxwidth;
	int					maxheight;
	int					numframes;
	float				beamlength;		// remove?
	void				*cachespot;		// remove?
	mspriteframedesc_t	frames[1];
} msprite_t;


/*
==============================================================================

ALIAS MODELS

Alias models are position independent, so the cache manager can move them.
==============================================================================
*/

#define	MAXALIASVERTS_OLD	1024
#define	MAXALIASTRIS_OLD	2048

#define	MAXALIASVERTS	4096
#define	MAXALIASFRAMES	256
#define	MAXALIASTRIS	4096

typedef struct
{
	int			firstpose;
	int			numposes;
	float		interval;
	trivertx_t	bboxmin;
	trivertx_t	bboxmax;
//	int			frame;		//JDH - not used anywhere
	char		name[16];
// JDH: per-frame scale & translate for MD2:
	float		scale[3];	// multiply byte verts by this
	float		translate[3];	// then add this
} maliasframedesc_t;

typedef struct
{
	trivertx_t	bboxmin;
	trivertx_t	bboxmax;
	int			frame;
} maliasgroupframedesc_t;

typedef struct
{
	int						numframes;
	int						intervals;
	maliasgroupframedesc_t	frames[1];
} maliasgroup_t;

typedef struct mtriangle_s
 {
	int					facesfront;
	int					vertindex[3];
#ifdef HEXEN2_SUPPORT
	unsigned short		stindex[3];
#endif
} mtriangle_t;


// JDH: h_value and v_value hold the horizontal & vertical scale to be applied to
// the ST coordinate list (computed by dividing the image dimensions by the
// GL texture dimensions).  For original MDL files, h_value and v_value hold
// the skin width & height until meshing, after which they hold the scale.
typedef struct skin_s
{
	float	h_value;
	float	v_value;
	int		gl_texturenum;
	int		fb_texturenum;
	int		fb_isLuma;
} skin_t;

typedef struct skingroup_s
{
	skin_t	skins[4];
	int		texels;			// only for player skins
} skingroup_t;

//#define	MAX_SKINS	32
#define	MAX_SKINS	256		//JDH - for MD2

typedef struct
{
//	int			ident;
//	int			version;
//	vec3_t		scale;				//JDH - each frame has scale now
//	vec3_t		scale_origin;		//JDH - each frame has translate now
//	float		boundingradius;		//JDH - not used
//	vec3_t		eyeposition;		//JDH - not used
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numstverts;		//JDH - for MD2 and Raven mdl
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;

	int			numposes;
	int			poseverts;
	int			posedata;		// numposes*poseverts trivert_t
	int			commands;		// gl command list with embedded s/t
	skingroup_t		skins[MAX_SKINS];
	maliasframedesc_t	frames[1];		// variable sized
} aliashdr_t;

#define INTERP_WEAP_MINDIST		 5000
#define INTERP_WEAP_MAXDIST		95000

// JDH: a structure used to associate an integer (eg. flags, type) with a list of models
typedef struct aliastype_s
{
	int type;
	const char **names;		/*** IMPORTANT: array must be NULL-terminated !! ***/
} aliastype_t;

/*****JDH*****/
//extern	aliashdr_t	*pheader;
/*****JDH*****/
extern	stvert_t	stverts[MAXALIASVERTS];
extern	mtriangle_t	triangles[MAXALIASTRIS];
extern	trivertx_t	*poseverts[MAXALIASFRAMES];

//===================================================================

// Whole model

typedef enum {mod_brush, mod_sprite, mod_alias, mod_md3 /*this is added for md3*/} modtype_t;
//typedef enum {mod_brush, mod_sprite, mod_alias} modtype_t;

// some models are special
typedef enum {MOD_NORMAL, MOD_PLAYER, MOD_EYES, MOD_FLAME, MOD_THUNDERBOLT, MOD_WEAPON,
		MOD_LAVABALL, MOD_SPIKE, MOD_SHAMBLER, MOD_HEAD, MOD_SPR, MOD_SPR32} modhint_t;

// entity flags:
#define	EF_ROCKET	1			// leave a trail
#define	EF_GRENADE	2			// leave a trail
#define	EF_GIB		4			// leave a trail
#define	EF_ROTATE	8			// rotate (bonus items)
#define	EF_TRACER	16			// green split trail
#define	EF_ZOMGIB	32			// small blood trail
#define	EF_TRACER2	64			// orange split trail + rotate
#define	EF_TRACER3	128			// purple trail

#ifdef HEXEN2_SUPPORT
  #define  EF_FIREBALL			0x00000100		// Yellow transparent trail in all directions
  #define  EF_ICE				0x00000200		// Blue-white transparent trail, with gravity
  #define  EF_MIP_MAP			0x00000400		// This model has mip-maps
  #define  EF_SPIT				0x00000800		// Black transparent trail with negative light
  #define  EF_TRANSPARENT		0x00001000		// Transparent sprite
  #define  EF_SPELL				0x00002000		// Vertical spray of particles
  #define  EF_HOLEY				0x00004000		// Solid model with color 0
  #define  EF_SPECIAL_TRANS		0x00008000		// Translucency through the particle table
  #define  EF_FACE_VIEW			0x00010000		// Poly Model always faces you
  #define  EF_VORP_MISSILE		0x00020000		// leave a trail at top and bottom of model
  #define  EF_SET_STAFF			0x00040000		// slowly move up and left/right
  #define  EF_MAGICMISSILE		0x00080000		// a trickle of blue/white particles with gravity
  #define  EF_BONESHARD			0x00100000		// a trickle of brown particles with gravity
  #define  EF_SCARAB			0x00200000		// white transparent particles with little gravity
  #define  EF_ACIDBALL			0x00400000		// Green drippy acid shit
  #define  EF_BLOODSHOT			0x00800000		// Blood rain shot trail
  #define  EF_MIP_MAP_FAR		0x01000000		// Set per frame, this model will use the far mip map
#endif

typedef struct
{
	float		mins[3], maxs[3];
	float		origin[3];
#ifdef HEXEN2_SUPPORT
	int			headnode[MAX_MAP_HULLS_H2];
#else
	int			headnode[MAX_MAP_HULLS];
#endif
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} mmodel_t;

typedef struct model_s
{
	char		name[MAX_QPATH];
	qboolean	needload;	// bmodels and sprites don't cache normally
	modtype_t	type;

// volume occupied by the model graphics
	vec3_t		mins, maxs;

#ifndef RQM_SV_ONLY
	modhint_t	modhint;	// by joe

	int			numframes;
	synctype_t	synctype;

	int			flags;
	float		radius;
#endif

// brush model
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	mmodel_t	*submodels;

	int			numplanes;
	mplane_t	*planes;

#ifndef RQM_SV_ONLY
	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numtextures;
	texture_t	**textures;

	byte		*lightdata;
	int			lightdatadepth;		// JDH: either 1 (8-bit) or 3 (24-bit)

	int			numsurfaces;
	msurface_t	*surfaces;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	int			numsurfedges;
	int			*surfedges;
#endif

	int			numleafs;	// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numnodes;
	mnode_t		*nodes;

	int			numclipnodes;
	mclipnode_t	*clipnodes;

#ifdef HEXEN2_SUPPORT
	hull_t		hulls[MAX_MAP_HULLS_H2];
#else
	hull_t		hulls[MAX_MAP_HULLS];
#endif

	byte		*visdata;
	char		*entities;		// worldmodel only

	qboolean	isworldmodel;

/*********** JDH **************/
	byte		*unpackedvis;
	byte		*currvisrow;
/*********** JDH **************/

// additional model data  (***JDH NOTE: it seems this field has to be last in model_t ?!? ***)
	cache_user_t	cache;		// only access through Mod_Extradata

} model_t;

//============================================================================

extern model_t				*loadmodel;
extern char				    mod_loadname[32];
extern const searchpath_t	*mod_loadpath;

void Mod_Init (void);
void Mod_ClearAll (void);
model_t *Mod_ForName (const char *name, qboolean crash);
void *Mod_Extradata (model_t *mod);	// handles caching
void Mod_TouchModel (const char *name);
unsigned Mod_CalcCRC (const char *mdl);
void Mod_Print_f (cmd_source_t src);


void Mod_LoadSpriteModel (model_t *mod, const void *buffer);
void Mod_LoadBrushModel (model_t *mod, void *buffer);

void Mod_LoadAliasModel (model_t *mod, const void *buffer);
void Mod_LoadRavenModel (model_t *mod, const void *buffer);
void Mod_LoadQ2Model (model_t *mod, const void *buffer);
void Mod_LoadQ3Model(model_t *mod, const void *buffer);
int Mod_GetAliasType (const char *modname, const char *reqprefix, const aliastype_t typelists[], int numlists);
modhint_t Mod_GetAliasHint (const char *modname);

mleaf_t *Mod_PointInLeaf (const vec3_t p, const model_t *model);
byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model);

float RadiusFromBounds (const vec3_t mins, const vec3_t maxs);

#endif	// __MODEL__
