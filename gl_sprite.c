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
// gl_sprite.c -- sprite model loading and caching (see gl_drawsprite.c for draw routines)

// models are the only shared resource between a client and server running
// on the same machine.

#ifndef RQM_SV_ONLY

#include "quakedef.h"

extern cvar_t		nospr32;

static int			spr_version;

#ifndef RQM_SV_ONLY

static const char *spr_paths[] = {"textures/sprites/", "textures/", NULL, NULL};
	// 3rd path set dynamically based on sprite's folder

/*
=================
Mod_Load24bitSprite
=================
*/
int Mod_Load24bitSprite (const char *sprite_name, const char *identifier, int framenum, int numingroup, int flags)
{
	char	basename[MAX_QPATH], filename[MAX_QPATH], path[MAX_QPATH];
	char	*name;
	const char *namelist[3] = {filename, COM_SkipPath(identifier), NULL};
	int		texturenum;

	COM_StripExtension (sprite_name, basename, sizeof(basename));

	name = COM_SkipPath (basename);
	if (name != basename)
	{
		Q_strncpy (path, sizeof(path), basename, name-basename);
		spr_paths[2] = path;
	}
	else
		spr_paths[2] = NULL;

	if (numingroup > 1)
		Q_snprintfz (filename, sizeof(filename), "%s_%i_%i", name, framenum/100, framenum%100);
	else
		Q_snprintfz (filename, sizeof(filename), "%s_%i", name, framenum);

	texturenum = GL_LoadTextureImage_MultiSource (spr_paths, namelist, identifier, flags, mod_loadpath->dir_level);



/*	Q_snprintfz (loadpath, sizeof(loadpath), "%s_%i", basename, framenum);
	texturenum = GL_LoadTextureImage ("textures/sprites/", loadpath, identifier, flags, mod_loadpath->dir_level);

	if (texturenum == 0)
	{
		texturenum = GL_LoadTextureImage ("textures/", loadpath, identifier, flags, mod_loadpath->dir_level);

		if ( texturenum == 0 )
		{
			Q_snprintfz (loadpath, sizeof(loadpath), "%s", identifier);
			texturenum = GL_LoadTextureImage ("textures/", loadpath, identifier, flags, mod_loadpath->dir_level);
		}
	}
*/
	return texturenum;
}

/*
=================
Mod_LoadSpriteModelTexture
=================
*/
void Mod_LoadSpriteModelTexture (const char *sprite_name, const dspriteframe_t *pinframe,
								 mspriteframe_t *pspriteframe, int framenum, int numingroup, int texture_flag)
{
	char	identifier[MAX_QPATH];
	int		bytes;

	if (numingroup > 1)
		Q_snprintfz (identifier, sizeof(identifier), "%s_%i_%i", sprite_name, framenum/100, framenum%100);
	else
		Q_snprintfz (identifier, sizeof(identifier), "%s_%i", sprite_name, framenum);

	if (!no24bit && !isDedicated)		// JDH: added no24bit check here
	{
		pspriteframe->gl_texturenum = Mod_Load24bitSprite (sprite_name, identifier, framenum, numingroup, texture_flag);
		if (pspriteframe->gl_texturenum)
		{
			pspriteframe->is_tex_external = true;
			return;
		}
	}

	bytes = (spr_version == SPRITE32_VERSION) ? 4 : 1;
	pspriteframe->gl_texturenum = GL_LoadTexture (identifier, pspriteframe->width, pspriteframe->height,
														(byte *)(pinframe + 1), texture_flag, bytes);
}

#endif		//#ifndef RQM_SV_ONLY

/*
=================
Mod_LoadSpriteFrame
=================
*/
void *Mod_LoadSpriteFrame (model_t *mod, void *pin, mspriteframe_t **ppframe, int framenum, int numingroup)
{
	dspriteframe_t	*pinframe;
	mspriteframe_t	*pspriteframe;
	int				width, height, size, origin[2];
#ifndef RQM_SV_ONLY
	int				texture_flag;
#endif

	pinframe = (dspriteframe_t *) pin;
	width = LittleLong (pinframe->width);
	height = LittleLong (pinframe->height);
	size = width * height * ((mod->modhint == MOD_SPR32) ? 4 : 1);

	pspriteframe = Hunk_AllocName (sizeof(mspriteframe_t), mod_loadname);
	memset (pspriteframe, 0, sizeof(mspriteframe_t));

	*ppframe = pspriteframe;

	pspriteframe->width = width;
	pspriteframe->height = height;
	origin[0] = LittleLong (pinframe->origin[0]);
	origin[1] = LittleLong (pinframe->origin[1]);

	pspriteframe->up = origin[1];
	pspriteframe->down = origin[1] - height;
	pspriteframe->left = origin[0];
	pspriteframe->right = width + origin[0];
	pspriteframe->is_tex_external = false;

#ifndef RQM_SV_ONLY
//	texture_flag = (gl_picmip_all.value ? TEX_MIPMAP : 0) | TEX_ALPHA | TEX_NOTILE;
	texture_flag = TEX_MIPMAP | TEX_ALPHA | TEX_NOTILE;
	Mod_LoadSpriteModelTexture (mod->name, pinframe, pspriteframe, framenum, numingroup, texture_flag);
#else
	pspriteframe->gl_texturenum = 0;
#endif

	return (byte *)(pinframe + 1) + size;
}

/*
=================
Mod_LoadSpriteGroup
=================
*/
void *Mod_LoadSpriteGroup (model_t *mod, void *pin, mspriteframe_t **ppframe, int framenum)
{
	dspritegroup_t		*pingroup;
	mspritegroup_t		*pspritegroup;
	int				i, numframes;
	dspriteinterval_t	*pin_intervals;
	float				*poutintervals;
	void				*ptemp;

	pingroup = (dspritegroup_t *)pin;

	numframes = LittleLong (pingroup->numframes);
	pspritegroup = Hunk_AllocName (sizeof(mspritegroup_t) + (numframes - 1) * sizeof(mspriteframe_t), mod_loadname);

	pspritegroup->numframes = numframes;
	*ppframe = (mspriteframe_t *)pspritegroup;

	pin_intervals = (dspriteinterval_t *)(pingroup + 1);
	poutintervals = Hunk_AllocName (numframes * sizeof(float), mod_loadname);
	pspritegroup->intervals = poutintervals;

	for (i=0 ; i<numframes ; i++)
	{
		*poutintervals = LittleFloat (pin_intervals->interval);
		if (*poutintervals <= 0.0)
			Sys_Error ("Mod_LoadSpriteGroup: interval <= 0");

		poutintervals++;
		pin_intervals++;
	}

	ptemp = (void *)pin_intervals;

	for (i=0 ; i<numframes ; i++)
		ptemp = Mod_LoadSpriteFrame (mod, ptemp, &pspritegroup->frames[i], framenum * 100 + i, numframes);

	return ptemp;
}

/*
=================
Mod_LoadSpriteFrames
=================
*/
void Mod_LoadSpriteFrames (model_t *mod, dspriteframetype_t *pinframe, msprite_t *psprite)
{
	int i;
	spriteframetype_t	frametype;

	for (i=0 ; i<psprite->numframes ; i++)
	{
		frametype = LittleLong (pinframe->type);
		psprite->frames[i].type = frametype;

		if (frametype == SPR_SINGLE)
		{
			pinframe = (dspriteframetype_t *)Mod_LoadSpriteFrame (mod, pinframe + 1, &psprite->frames[i].frameptr, i, 1);
		}
		else
		{
			pinframe = (dspriteframetype_t *)Mod_LoadSpriteGroup (mod, pinframe + 1, &psprite->frames[i].frameptr, i);
		}
	}
}

/*
=================
Mod_LoadSpriteHeader
=================
*/
void Mod_LoadSpriteHeader (const dsprite_t *pin, msprite_t *psprite)
{
	psprite->type = LittleLong (pin->type);
	psprite->maxwidth = LittleLong (pin->width);
	psprite->maxheight = LittleLong (pin->height);
	psprite->beamlength = LittleFloat (pin->beamlength);
	psprite->numframes = LittleLong (pin->numframes);
}

/*
=================
Mod_LoadSpriteModel
=================
*/
void Mod_LoadSpriteModel (model_t *mod, const void *buffer)
{
	int					numframes, size;
	dsprite_t			*pin;
	msprite_t			*psprite;
	dspriteframetype_t	*pframetype;

	pin = (dsprite_t *)buffer;

	spr_version = LittleLong (pin->version);

	if (spr_version != SPRITE_VERSION && spr_version != SPRITE32_VERSION)
		Sys_Error ("%s has wrong version number (%i should be %i or %i)", mod->name, spr_version, SPRITE_VERSION, SPRITE32_VERSION);

	if (nospr32.value && spr_version == SPRITE32_VERSION)
		return;

	// joe: indicating whether it's a 32bit sprite or not
	mod->modhint = (spr_version == SPRITE32_VERSION) ? MOD_SPR32 : MOD_SPR;

	numframes = LittleLong (pin->numframes);

// load the frames
	if (numframes < 1)
		Sys_Error ("Mod_LoadSpriteModel: Invalid # of frames: %d\n", numframes);

	size = sizeof(msprite_t) + (numframes - 1) * sizeof(mspriteframedesc_t);
	psprite = Hunk_AllocName (size, mod_loadname);

	Mod_LoadSpriteHeader (pin, psprite);

	mod->cache.data = psprite;
	mod->synctype = LittleLong (pin->synctype);
	mod->numframes = numframes;
	mod->type = mod_sprite;

	mod->mins[0] = mod->mins[1] = -psprite->maxwidth/2;
	mod->maxs[0] = mod->maxs[1] = psprite->maxwidth/2;
	mod->mins[2] = -psprite->maxheight/2;
	mod->maxs[2] = psprite->maxheight/2;

	pframetype = (dspriteframetype_t *)(pin + 1);
	Mod_LoadSpriteFrames (mod, pframetype, psprite);
}

#endif		// #ifndef RQM_SV_ONLY
