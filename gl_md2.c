
#ifndef RQM_SV_ONLY

#include "quakedef.h"

#define MD2ALIAS_VERSION	8

typedef struct md2_s
{
	int			ident;
	int			version;

	int			skinwidth;
	int			skinheight;
	int			framesize;		// byte size of each frame

	int			num_skins;
	int			num_xyz;
	int			num_st;			// greater than num_xyz for seams
	int			num_tris;
	int			num_glcmds;		// dwords in strip/fan command list
	int			num_frames;

	int			ofs_skins;		// each skin is a MAX_QPATH string
	int			ofs_st;			// byte offset from start for stverts
	int			ofs_tris;		// offset for dtriangles
	int			ofs_frames;		// offset for first frame
	int			ofs_glcmds;
	int			ofs_end;		// end of file
} md2_t;

/*typedef struct
{
	short	index_xyz[3];
	short	index_st[3];
} dtriangle_md2_t;
*/

typedef struct
{
	float		scale[3];	// multiply byte verts by this
	float		translate[3];	// then add this
	char		name[16];	// frame name from grabbing
	trivertx_t	verts[1];	// variable sized
} dframe_md2_t;

/*
=================
Mod_LoadQ2Skins
=================
*/
void Mod_LoadQ2Skins (aliashdr_t *pheader, const char *skinname, const model_t *mod)
{
	int			tex_flags, i, j, texnum;
	skingroup_t	*skingroup;

	if (pheader->numskins < 1 || pheader->numskins > 256)
		Sys_Error ("Mod_LoadQ2Skins: Invalid # of skins in %s: %d\n", pheader->numskins, mod->name);

#ifndef RQM_SV_ONLY
	tex_flags = (gl_picmip_all.value ? TEX_MIPMAP : 0) | TEX_BLEND;
#else
	tex_flags = 0;
#endif

	for (i = 0; i < pheader->numskins; i++)
	{
	#ifndef RQM_SV_ONLY
		texnum = GL_LoadTextureImage ("", skinname, NULL, tex_flags, mod_loadpath->dir_level);
		if (!texnum)
		{
			Con_Printf("Mod_LoadQ2Skins: %s  Texture missing: %s\n", mod->name, skinname);
		}
	#else
		texnum = 0;
	#endif
		skingroup = &pheader->skins[i];
		skingroup->texels = 0;

		for (j = 0; j < 4; j++)
		{
			skingroup->skins[j].gl_texturenum = texnum;
			skingroup->skins[j].fb_texturenum = 0;
			skingroup->skins[j].h_value = 1.0;
			skingroup->skins[j].v_value = 1.0;
		}

		skinname += MAX_QPATH;
	}
}

/*
=================
Mod_LoadQ2Commands
=================
*/
int Mod_LoadQ2Commands (const md2_t *pinmodel, aliashdr_t *pheader)
{
	int		numcommands, i;
	int		*cmds, *pincmd;

	numcommands = LittleLong(pinmodel->num_glcmds);
	cmds = Hunk_Alloc (numcommands * 4);

	pincmd = (int *) ((byte *)pinmodel + LittleLong(pinmodel->ofs_glcmds));
	for (i=0 ; i<numcommands ; i++)
		cmds[i] = LittleLong (pincmd[i]);

/*
	int		numcommands, count;
	int		*cmds, *pincmd, *poutcmd, *lastcmd;

	numcommands = LittleLong(pinmodel->num_glcmds);
	cmds = Hunk_Alloc (numcommands * 4);

	poutcmd = cmds;
	pincmd = (int *) ((byte *)pinmodel + LittleLong(pinmodel->ofs_glcmds));
	lastcmd = pincmd + numcommands;

	while (pincmd < lastcmd)
	{
		*poutcmd++ = count = LittleLong(*pincmd++);

		if (!count)
			break;

		if (count < 0)
			count = -count;

		do
		{
			*poutcmd++ = LittleLong(*pincmd++);
			*poutcmd++ = LittleLong(*pincmd++);
			pincmd++;		// extra array index not used in Quake 1
		} while (--count);
	}
*/
	return (byte *)cmds - (byte *)pheader;
}

/*
=================
Mod_LoadQ2Frames
=================
*/
void Mod_LoadQ2Frames (const dframe_md2_t *pframe, int framesize, aliashdr_t *pheader, trivertx_t *verts)
{
	int i, j;
	maliasframedesc_t	*poutframe;

	for (i=0 ; i < pheader->numframes ; i++)
	{
		poutframe = &pheader->frames[i];

		poutframe->firstpose = i;
		poutframe->numposes = 1;
		poutframe->interval = 0.1;		// this doesn't get used if numposes is 1
		Q_strcpy (poutframe->name, pframe->name, sizeof(poutframe->name));

		for (j=0 ; j<3 ; j++)
		{
			poutframe->scale[j] = LittleFloat (pframe->scale[j]);
			poutframe->translate[j] = LittleFloat (pframe->translate[j]);
		}

//	trivertx_t	poutframe->bboxmin;
//	trivertx_t	poutframe->bboxmax;

		// verts are all 8 bit, so no swapping needed
		memcpy (verts, pframe->verts, pheader->numverts * sizeof(trivertx_t));
		verts += pheader->numverts;

		pframe = (dframe_md2_t *) ((byte *) pframe + framesize);
	}
}

/*
=================
Mod_LoadQ2Header
=================
*/
void Mod_LoadQ2Header (const md2_t *pinmodel, aliashdr_t *pheader, const char *modname)
{
	int numframes;

// endian-adjust and copy the data, starting with the alias model header
//	pheader->boundingradius = 0;
	pheader->numskins = LittleLong (pinmodel->num_skins);
	pheader->skinwidth = LittleLong (pinmodel->skinwidth);
	pheader->skinheight = LittleLong (pinmodel->skinheight);

/************JDH***********/
	//if (pheader->skinheight > MAX_LBM_HEIGHT)
	//	Sys_Error ("Mod_LoadAliasHeader: model %s has a skin taller than %d", modname, MAX_LBM_HEIGHT);
/************JDH***********/

	pheader->numverts = LittleLong (pinmodel->num_xyz);

	if (pheader->numverts <= 0)
		Sys_Error ("Mod_LoadQ2Header: model %s has no vertices", modname);

	if (pheader->numverts > 65536)
		Sys_Error ("Mod_LoadQ2Header: model %s has too many vertices", modname);

	pheader->numstverts = LittleLong (pinmodel->num_st);

	pheader->numtris = LittleLong (pinmodel->num_tris);

	if (pheader->numtris <= 0)
		Sys_Error ("Mod_LoadQ2Header: model %s has no triangles", modname);

	numframes = pheader->numframes = LittleLong (pinmodel->num_frames);
	if (numframes < 1)
		Sys_Error ("Mod_LoadQ2Header: Invalid # of frames: %d\n", numframes);

	pheader->numposes = numframes;
	// as far as I can tell, pheader->size is not used anywhere, so I'll store framesize here
	pheader->size = LittleFloat (pinmodel->framesize);

/*	for (i=0 ; i<3 ; i++)
	{
		pheader->scale[i] = 1;
		pheader->scale_origin[i] = 0;
		pheader->eyeposition[i] = 0;
	}
*/
}

/*
=================
Mod_LoadQ2Model
=================
*/
void Mod_LoadQ2Model (model_t *mod, const void *buffer)
{
	md2_t		*pinmodel;
	int			version, start, size, end, total;
	aliashdr_t	*pheader;
	trivertx_t	*verts;

	pinmodel = (md2_t *) buffer;

	version = LittleLong (pinmodel->version);

	if (version != MD2ALIAS_VERSION)
	{
		Sys_Error ("Mod_LoadQ2Model: %s has wrong version number (%i should be %i)",
					mod->name, version, MD2ALIAS_VERSION);
		return;
	}

//	mod->modhint = MOD_NORMAL;
	mod->modhint = Mod_GetAliasHint (mod->name);

// allocate space for a working header, plus all the data except the frames,
// skin and group info
	start = Hunk_LowMark ();
	size = sizeof(aliashdr_t) + (LittleLong(pinmodel->num_frames) - 1) * sizeof(maliasframedesc_t);
	pheader = Hunk_AllocName (size, mod_loadname);

	Mod_LoadQ2Header (pinmodel, pheader, mod->name);

	mod->flags = 0;
	mod->synctype = ST_RAND;
	mod->numframes = pheader->numframes;
	mod->type = mod_alias;

	Mod_LoadQ2Skins (pheader, (char *)((byte *) pinmodel + LittleLong (pinmodel->ofs_skins)), mod);

	// load base s and t vertices
/*	pinstverts = (stvert_t *)((byte *) pinmodel + LittleLong (pinmodel->ofs_st));
	for (i=0 ; i < pheader->numstverts ; i++)
	{
		stverts[i].onseam = LittleLong (pinstverts[i].onseam);
		stverts[i].s = LittleLong (pinstverts[i].s);
		stverts[i].t = LittleLong (pinstverts[i].t);
	}
*/
	verts = Hunk_Alloc (pheader->numframes * pheader->numverts * sizeof(trivertx_t) );
	pheader->posedata = (byte *)verts - (byte *)pheader;
	pheader->poseverts = pheader->numverts;

	Mod_LoadQ2Frames ((dframe_md2_t *) ((byte *) pinmodel + LittleLong (pinmodel->ofs_frames)),
						LittleLong(pinmodel->framesize), pheader, verts);

	pheader->commands = Mod_LoadQ2Commands (pinmodel, pheader);

	mod->mins[0] = -32;
	mod->mins[1] = -32;
	mod->mins[2] = -32;
	mod->maxs[0] = 32;
	mod->maxs[1] = 32;
	mod->maxs[2] = 32;

	mod->radius = RadiusFromBounds (mod->mins, mod->maxs);

// move the complete, relocatable alias model to the cache
	end = Hunk_LowMark ();
	total = end - start;

	Cache_Alloc (&mod->cache, total, mod_loadname);
	if (mod->cache.data)
		memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);
}

#endif		// #ifndef RQM_SV_ONLY
