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
// model.c -- model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

#include "quakedef.h"

model_t				*loadmodel;
char				mod_loadname[32];	// for hunk tags
const searchpath_t	*mod_loadpath;		// JDH

model_t *Mod_LoadModel (model_t *mod, qboolean crash);

byte	mod_novis[MAX_MAP_LEAFS/8];

#ifdef HEXEN2_SUPPORT
  #define	MAX_MOD_KNOWN	1500
#else
  #define	MAX_MOD_KNOWN	512
#endif

model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	memset (mod_novis, 0xff, sizeof(mod_novis));
}

/*
==================
Mod_FindName
==================
*/
model_t *Mod_FindName (const char *name)
{
	int	i;
	model_t	*mod;

	if (!name[0])
		Sys_Error ("Mod_ForName: NULL name");

// search the currently loaded models
	for (i = 0, mod = mod_known ; i < mod_numknown ; i++, mod++)
		if (COM_FilenamesEqual(mod->name, name))
			break;

	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			Sys_Error ("mod_numknown == MAX_MOD_KNOWN");
		Q_strcpy (mod->name, name, sizeof(mod->name));
		mod->needload = true;
		mod_numknown++;
	}

	return mod;
}

#ifndef RQM_SV_ONLY
/*
===============
Mod_Extradata

Caches the data if needed
===============
*/
void *Mod_Extradata (model_t *mod)
{
	void	*r;

	if ((r = Cache_Check (&mod->cache)))
		return r;

	Mod_LoadModel (mod, true);

	if (!mod->cache.data)
		Sys_Error ("Mod_Extradata: caching failed");

	return mod->cache.data;
}

/*
==================
Mod_TouchModel
==================
*/
void Mod_TouchModel (const char *name)
{
	model_t	*mod;

	mod = Mod_FindName (name);

	if (!mod->needload)
	{
		if (mod->type == mod_alias)
			Cache_Check (&mod->cache);
	}
}

#endif		// #ifndef RQM_SV_ONLY

/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t *Mod_PointInLeaf (const vec3_t p, const model_t *model)
{
	mnode_t		*node;
	float		d;
	mplane_t	*plane;

	if (!model || !model->nodes)
		Sys_Error ("Mod_PointInLeaf: bad model");

	node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		plane = node->plane;
		d = PlaneDiff(p, plane);
		node = (d > 0) ? node->children[0] : node->children[1];
	}

	return NULL;	// never reached
}

/*
===================
Mod_DecompressVis
===================
*/
/*********JDH**********
byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static	byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte		*out;
	int		row;

	row = (model->numleafs + 7) >> 3;
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);

	return decompressed;
}

byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if (leaf == model->leafs)
		return mod_novis;
	return Mod_DecompressVis (leaf->compressed_vis, model);
}
**************/

byte *Mod_DecompressVis (const byte *in, model_t *model)
{
	int		rowsize, c;
	byte	*out, *end;

	rowsize = (model->numleafs + 7) >> 3;

	if (!model->unpackedvis)
	{
		//model->unpackedvis = malloc( rowsize*rowsize );
		model->unpackedvis = Hunk_AllocName (rowsize * model->numleafs, "vis"/*model->name*/);
		model->currvisrow = model->unpackedvis;
	}
	out = model->currvisrow;

	//out = Hunk_AllocName( rowsize, model->name );

	end = out + rowsize;

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out < end);

	model->currvisrow += rowsize;
	return end-rowsize;
}

byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if (leaf == model->leafs)
		return mod_novis;

	if (!leaf->compressed_vis)
		return mod_novis;		// no vis info, so make all visible

	if (leaf->uncompressed_vis)
		return leaf->uncompressed_vis;

	return (leaf->uncompressed_vis = Mod_DecompressVis (leaf->compressed_vis, model));
}

/*********JDH**********

void Mod_DecompressVisRow( byte *in, byte *out, int rowsize )
{
	byte	*end;
	int		c;

	end = out + rowsize;
	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out < end);
}

void Mod_DecompressVis( model_t *model )
{
	int		 rowsize, i;
	byte    *worldvis;
	mleaf_t *currleaf;

	rowsize = (model->numleafs + 7) >> 3;		// ceiling of numleafs/8.0
	worldvis = Hunk_AllocName( rowsize*rowsize, loadname );

	currleaf = model->leafs;
	for ( i = model->numleafs; i; i--, currleaf++ )
	{
		Mod_DecompressVisRow( currleaf->visdata, worldvis, rowsize );
		currleaf->visdata = worldvis;
		worldvis += rowsize;
	}
}

byte *Mod_LeafPVS( mleaf_t *leaf, model_t *model )
{
	if ( leaf == model->leafs )
		return mod_novis;

	if ( !leaf->visdata )
		return mod_novis;		// no vis info, so make all visible

	if ( (DWORD) leaf->visdata & 0x80000000 )
	{
		int rowsize = (model->numleafs + 7) >> 3;
		byte *row = malloc( rowsize );
		Mod_DecompressVisRow( (byte *) ((DWORD) leaf->visdata & 0x7FFFFFFF), row, rowsize );
		leaf->visdata = row;
	}

	return leaf->visdata;

}
*********JDH**********/

/*
===================
Mod_ClearAll
===================
*/
void Mod_ClearAll (void)
{
	int		i;
	model_t	*mod;
	char	name[MAX_QPATH];

	for (i = 0, mod = mod_known ; i < mod_numknown ; i++, mod++)
	{
		if ((mod->type != mod_alias) && (mod->type != mod_md3))		// JDH 2010/05/21: added md3 check
		{
		// JDH: added memory clear, otherwise a model that is of a different type than the
		//  previously loaded one (eg. bsp -> mdl) has invalid fields.  This can happen
		//  when the gamedir is changed, or if the file is replaced while game is running.

			Q_strcpy (name, mod->name, sizeof(name));
			memset (mod, 0, sizeof(model_t));
			mod->needload = true;
			Q_strcpy (mod->name, name, sizeof(mod->name));
		}
	}
}

FILE * Mod_OpenModelFile (const char *name, com_fileinfo_t *fi_out)
{
	int flags;

//	if (!Q_strcasecmp(COM_FileExtension(name), "mdl"))
	if (COM_FilenamesEqual(COM_FileExtension(name), "mdl"))
		flags = FILE_ANY_MDL;
	else
		flags = 0;

	return COM_FOpenFile (name, flags, fi_out);
}

byte * Mod_LoadModelFile (const char *name, byte *buffer, int bufsize)
{
	com_fileinfo_t fi;
	FILE *f;

	f = Mod_OpenModelFile (name, &fi);
	if (f)
	{
		buffer = COM_LoadFromFile (f, &fi, memtype_stack, buffer, bufsize);
		fclose (f);
		return buffer;
	}

	return NULL;
/*	int flags;

//	if (!Q_strcasecmp(COM_FileExtension(name), "mdl"))
	if (COM_FilenamesEqual(COM_FileExtension(name), "mdl"))
		flags = FILE_ANY_MDL;
	else
		flags = 0;

	if (buffer && bufsize)
		return COM_LoadStackFile (name, buffer, bufsize, flags);

// if buffer is NULL, just indicate whether file exists or not
	return (byte *) COM_FindFile (name, flags, NULL);
*/
}

#ifdef RQM_SV_ONLY

model_t *SV_LoadModel (model_t *mod, qboolean crash)
{
	com_fileinfo_t	fi;
	int				ident;
	FILE			*f;
	byte			*buf;

	if (!mod->needload)
		return mod;

	f = Mod_OpenModelFile (mod->name, &fi);
	if (!f)
	{
		if (crash)
			Sys_Error ("SV_LoadModel: %s not found", mod->name);
		return NULL;
	}

	if (fi.filelen < 4)
	{
		if (crash)
			Sys_Error ("SV_LoadModel: %s is not a valid model", mod->name);
		fclose (f);
		return NULL;
	}

	fread (&ident, 1, 4, f);

// call the apropriate loader
	mod->needload = false;

	switch (LittleLong(ident))
	{
		case IDPOLYHEADER:
		case MD2IDHEADER:
	#ifdef HEXEN2_SUPPORT
		case RAPOLYHEADER:
	#endif
			mod->type = mod_alias;
			break;

		case MD3IDHEADER:
			mod->type = mod_md3;
			break;

		case IDSPRITEHEADER:
			mod->type = mod_sprite;
			break;

		case BSPVERSION:
			fseek (f, -4, SEEK_CUR);
			buf = COM_LoadFromFile (f, &fi, 1, NULL, 0);
			loadmodel = mod;
			mod_loadpath = fi.searchpath;
			Mod_LoadBrushModel (mod, buf);
			break;

		default:
			Sys_Error ("SV_LoadModel: %s is not a valid model", mod->name);
	}

	fclose (f);
	return mod;
}
#endif		// #ifndef RQM_SV_ONLY


/*
==================
Mod_LoadModel

Loads a model into the cache
==================
*/
model_t *Mod_LoadModel (model_t *mod, qboolean crash)
{
#ifdef RQM_SV_ONLY
	return SV_LoadModel (mod, crash);
#else
	void		*d;
	unsigned	*buf;
	byte		stackbuf[1024];		// avoid dirtying the cache heap
//	DWORD dwTicks;
//	int iType;

#ifdef _DEBUG
	if (mod->name && !strcmp(mod->name, "maps/b_explob.bsp"))
		d = NULL;
#endif

	if (!mod->needload)
	{
		if ((mod->type == mod_alias) || (mod->type == mod_md3))
		{
			if ((d = Cache_Check (&mod->cache)))
				return mod;
		}
		else
		{
			return mod;		// not cached at all
		}
	}

// because the world is so huge, load it one piece at a time
	if (!crash)
	{

	}

	Con_DPrintf ("    loading model %s\n", mod->name);		// JDH

// load the file
	if (!(buf = (unsigned *)Mod_LoadModelFile(mod->name, stackbuf, sizeof(stackbuf))))
	{
		if (crash)
			Sys_Error ("Mod_NumForName: %s not found", mod->name);
		return NULL;
	}

// allocate a new model
	COM_FileBase (mod->name, mod_loadname, sizeof(mod_loadname));

	loadmodel = mod;
	mod_loadpath = com_filepath;

// fill it in

// JDH: since we store vis data after decompressing:
	mod->unpackedvis = NULL;
	mod->currvisrow = NULL;

// call the apropriate loader
	mod->needload = false;

//	dwTicks = GetTickCount();
//	iType = LittleLong(*(unsigned *)buf);

	switch (LittleLong(*buf))
	{
		case IDPOLYHEADER:
			Mod_LoadAliasModel (mod, buf);
			break;

		case MD3IDHEADER:
			Mod_LoadQ3Model (mod, buf);
			break;

	#ifdef HEXEN2_SUPPORT
		case RAPOLYHEADER:
			Mod_LoadRavenModel (mod, buf);
			break;
	#endif

		case MD2IDHEADER:
			Mod_LoadQ2Model (mod, buf);
			break;

		case IDSPRITEHEADER:
			Mod_LoadSpriteModel (mod, buf);
			break;

		default:
			Mod_LoadBrushModel (mod, buf);
			break;
	}

//	dwTicks = GetTickCount() - dwTicks;
//	Con_DPrintf ("LoadModel(%i) took %i ms\n", iType, dwTicks);

	return mod;
#endif
}

/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (const char *name, qboolean crash)
{
	model_t	*mod;

	mod = Mod_FindName (name);

	return Mod_LoadModel (mod, crash);
}

#ifndef RQM_SV_ONLY
qboolean Img_HasFullbrights (byte *pixels, int size)
{
	int	i;

	for (i=0 ; i<size ; i++)
		if (pixels[i] >= 224)		// last 2 rows of palette
			return true;

	return false;
}
#endif

/*
================
Mod_CalcCRC
  (for pq cheatfree; was CheckModel)
================
*/
unsigned Mod_CalcCRC (const char *mdl)
{
	byte	stackbuf[1024];		// avoid dirtying the cache heap
	byte	*buf;
	unsigned short	crc;

	if (!(buf = Mod_LoadModelFile(mdl, stackbuf, sizeof(stackbuf))))
		Host_Error ("Mod_CalcCRC: could not load %s", mdl);
	crc = CRC_Block (buf, com_filesize);

	return crc;
}

/*
================
Mod_Print_f
================
*/
void Mod_Print_f (cmd_source_t src)
{
	int		i;
	model_t		*mod;

	Con_PagedOutput_Begin ();

	Con_Print ("Cached models:\n");
	for (i = 0, mod = mod_known ; i < mod_numknown ; i++, mod++)
//		Con_Printf ("%4d: %s (%8p)\n", i, mod->name, mod->cache.data);
		Con_Printf ("%4d: %s\n", i, mod->name);

	Con_PagedOutput_End ();
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (const vec3_t mins, const vec3_t maxs)
{
	int		i;
	vec3_t		corner;

	for (i=0 ; i<3 ; i++)
		corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);

	return VectorLength(corner);
}
