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
// wad.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

//===============
//   TYPES
//===============

#define	CMP_NONE		0
#define	CMP_LZSS		1

#define	TYP_NONE		0
#define	TYP_LABEL		1

#define	TYP_LUMPY		64		// 64 + grab command number
#define	TYP_PALETTE		64
#define	TYP_QTEX		65
#define	TYP_QPIC		66
#define	TYP_SOUND		67
#define	TYP_MIPTEX		68


typedef struct
{
	char		identification[4];	// should be WAD2 or 2DAW
	int		numlumps;
	int		infotableofs;
} wadinfo_t;

typedef struct
{
	int		filepos;
	int		disksize;
	int		size;			// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];		// must be null terminated
} lumpinfo_t;


int		wad_numlumps;
lumpinfo_t	*wad_lumps;
byte		*wad_base = NULL;

void SwapPic (qpic_t *pic);

/*
==================
W_CleanupName

Lowercases name and pads with spaces and a terminating 0 to the length of
lumpinfo_t->name.
Used so lumpname lookups can proceed rapidly by comparing 4 chars at a time
Space padding is so names can be printed nicely in tables.
Can safely be performed in place.
==================
*/
void W_CleanupName (const char *in, char *out)
{
	int	i, c;

	for (i=0 ; i<16 ; i++)
	{
		c = in[i];
		if (!c)
			break;

		if (c >= 'A' && c <= 'Z')
			c += ('a' - 'A');
		out[i] = c;
	}

	for ( ; i< 16 ; i++)
		out[i] = 0;
}

/*
====================
W_LoadWadFile
====================
*/
qboolean W_LoadWadFile (const char *filename)
{
	byte		*old_wad;
	lumpinfo_t	*lump_p;
	wadinfo_t	*header;
	unsigned	i;
	int			infotableofs;

	old_wad = wad_base;		// JDH: for gamedir change

	if (!(wad_base = COM_LoadMallocFile(filename, 0)))		// JDH: originally loaded to hunk
	{
		if (old_wad)
		{
			wad_base = old_wad;
			return false;
		}
		Sys_Error ("W_LoadWadFile: couldn't load %s", filename);
	}

	header = (wadinfo_t *)wad_base;

	if (memcmp(header->identification, "WAD2", 4))
	{
		if (old_wad)
		{
			wad_base = old_wad;
			return false;
		}
		Sys_Error ("Wad file %s doesn't have WAD2 id\n", filename);
	}

	if (old_wad)
		free (old_wad);
	
	wad_numlumps = LittleLong (header->numlumps);
	infotableofs = LittleLong (header->infotableofs);
	wad_lumps = (lumpinfo_t *)(wad_base + infotableofs);

	for (i=0, lump_p = wad_lumps ; i < wad_numlumps ; i++, lump_p++)
	{
		lump_p->filepos = LittleLong (lump_p->filepos);
		lump_p->size = LittleLong (lump_p->size);
		W_CleanupName (lump_p->name, lump_p->name);
		if (lump_p->type == TYP_QPIC)
			SwapPic ((qpic_t *)(wad_base + lump_p->filepos));
	}

	return true;
}

/*
=============
W_GetLumpinfo
=============
*/
lumpinfo_t *W_GetLumpinfo (const char *name)
{
	int		i;
	lumpinfo_t	*lump_p;
	char		clean[16];

	W_CleanupName (name, clean);

	for (lump_p = wad_lumps, i=0 ; i<wad_numlumps ; i++, lump_p++)
	{
		if (!strcmp(clean, lump_p->name))
			return lump_p;
	}

	Con_Printf ("\x02""WARNING: lump %s not found!\n", name);		// JDH: was Sys_Error
	return NULL;
}

void *W_GetLumpByName (const char *name, int *size_out)
{
	lumpinfo_t	*lump;

	lump = W_GetLumpinfo (name);
	if (!lump)
		return NULL;
	
	if (size_out)
		*size_out = lump->disksize;
	
	return (void *)(wad_base + lump->filepos);
}

/*void *W_GetLumpByNum (int num)
{
	lumpinfo_t	*lump;

	if (num < 0 || num > wad_numlumps)
		Sys_Error ("W_GetLumpByNum: bad number: %i", num);

	lump = wad_lumps + num;

	return (void *)(wad_base + lump->filepos);
}
*/

byte * Wad_LoadPalette (void)
{
	lumpinfo_t	*lump;

	lump = W_GetLumpinfo ("palette");
	if (lump && (lump->type == TYP_PALETTE) && (lump->disksize == 768))
		return wad_base + lump->filepos;

	return NULL;
}

byte * Wad_LoadColormap (void)
{
	lumpinfo_t	*lump;

	lump = W_GetLumpinfo ("colormap");
	if (lump && (lump->type == TYP_QTEX) && (lump->disksize == 8196))
		return wad_base + lump->filepos;

	return NULL;
}

/*
=============================================================================

automatic byte swapping

=============================================================================
*/

void SwapPic (qpic_t *pic)
{
	pic->width = LittleLong (pic->width);
	pic->height = LittleLong (pic->height);
}

#endif		//#ifndef RQM_SV_ONLY
