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
// gl_draw.c -- this is the only file outside the refresh that touches the vid buffer

#include "quakedef.h"

#ifndef RQM_SV_ONLY

int	texture_extension_number = 1;

cvar_t	gl_conalpha = {"gl_conalpha", "0.6", CVAR_FLAG_ARCHIVE};	// by joe

cvar_t	gl_externaltextures_world   = {"gl_externaltextures_world",   "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_externaltextures_bmodels = {"gl_externaltextures_bmodels", "1", CVAR_FLAG_ARCHIVE};
cvar_t	gl_externaltextures_models  = {"gl_externaltextures_models",  "1", CVAR_FLAG_ARCHIVE};

qboolean OnChange_gl_consolefont (cvar_t *var, const char *string);
cvar_t	gl_consolefont = {"gl_consolefont", "original", CVAR_FLAG_ARCHIVE, OnChange_gl_consolefont};

qboolean OnChange_gl_smoothfont (cvar_t *var, const char *string);
cvar_t gl_smoothfont = {"gl_smoothfont", "0", CVAR_FLAG_ARCHIVE, OnChange_gl_smoothfont};

qboolean OnChange_gl_crosshairimage (cvar_t *var, const char *string);
cvar_t	gl_crosshairimage = {"crosshairimage", "", CVAR_FLAG_ARCHIVE, OnChange_gl_crosshairimage};

cvar_t	gl_crosshairalpha = {"crosshairalpha", "1", CVAR_FLAG_ARCHIVE};

cvar_t	crosshair = {"crosshair", "1", CVAR_FLAG_ARCHIVE};		// JDH: default was 2
cvar_t	crosshaircolor = {"crosshaircolor", "10", CVAR_FLAG_ARCHIVE};		// JDH: changed default from red (79) to light grey
cvar_t	crosshairsize	= {"crosshairsize", "1", CVAR_FLAG_ARCHIVE};
cvar_t	cl_crossx = {"cl_crossx", "0", CVAR_FLAG_ARCHIVE};
cvar_t	cl_crossy = {"cl_crossy", "0", CVAR_FLAG_ARCHIVE};

//byte		*draw_chars;			// 8*8 graphic characters
mpic_t		*draw_backtile;
mpic_t		*draw_disc;

#define STAT_MINUS		10		// num frame for '-' stats digit

mpic_t		*draw_bignums[2][STAT_MINUS+1];		// from sbar.c
mpic_t		*draw_bigcolon, *draw_bigslash;			// ditto

#ifdef HEXEN2_SUPPORT
  #define MAX_DISC 18

  extern int	loading_stage;

  mpic_t		*draw_disc_H2[MAX_DISC];
  int			char_smalltexture;
  char			BigCharWidth[27][27];
#endif

//int		translate_texture;
int		char_texture;
int		char_texture_bright;
float	charset_hspacing, charset_vspacing;		// distance from one char to the next, as a
												// multiple of character's width or height

// JDH: some extra characters for menu & console:
//int		char_texture2;
//int		char_menufonttexture = 0;
static char	MenuFontWidths[64][64];

extern	cvar_t	crosshair, cl_crossx, cl_crossy, crosshaircolor, crosshairsize;

mpic_t		crosshairpic;
static qboolean	crosshairimage_loaded = false;

//mpic_t		conback_data;
//mpic_t		*conback = &conback_data;
mpic_t		draw_conback;
mpic_t		draw_menufont;

int		gl_lightmap_format = 3, gl_solid_format = 3, gl_alpha_format = 4;

#define		NUMCROSSHAIRS	6

int		crosshairtextures[NUMCROSSHAIRS];

// JDH: made these palette-independent (0 --> transparent, 1 to 255 --> greyscale)
static byte crosshairdata[NUMCROSSHAIRS][64] =
{
{	0x00, 0x00, 0x00, 0xEF, 0xEF, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xEF, 0xCF, 0x20, 0x00, 0x00,
	0x00, 0xEF, 0xEF, 0xCF, 0xEF, 0xCF, 0xEF, 0x00,
	0x00, 0x00, 0x20, 0xEF, 0xCF, 0x20, 0x20, 0x20,
	0x00, 0x00, 0x00, 0xDF, 0xFF, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
{	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
},
{	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
	0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF
},
{	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}
};

//=============================================================================
/* Support Routines */

typedef struct cachepic_s
{
	char	name[MAX_QPATH];
	mpic_t	pic;
} cachepic_t;

#define MAX_CACHED_PICS 	256			// JDH: was 128
cachepic_t	cachepics[MAX_CACHED_PICS];
int		numcachepics;
/*
#ifdef HEXEN2_SUPPORT
byte		*menuplyr_pixels[NUM_CLASSES];
#else
byte		*menuplyr_pixels[1];
#endif
*/
//int		pic_texels;
//int		pic_count;

mpic_t * Draw_FindCachePic (const char *filepath)
{
	cachepic_t	*pic;
	int			i;

	for (pic = cachepics, i = 0 ; i < numcachepics ; pic++, i++)
		if (!strcmp(filepath, pic->name))
			return &pic->pic;

	return NULL;
}

/*
================
Draw_AddCachePic
================
*/
mpic_t *Draw_AddCachePic (const char *filepath, byte *data, int width, int height)
{
//	int			player_idx = -1;
	cachepic_t	*pic;
	mpic_t		*pic_24bit;

	if (numcachepics == MAX_CACHED_PICS)
		Sys_Error ("numcachepics == MAX_CACHED_PICS");
/*
	// HACK HACK HACK --- we need to keep the bytes for
	// the translatable player picture just for the menu
	// configuration dialog
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (COM_FilenamesEqual (filepath, "gfx/menu/netp1.lmp"))
			player_idx = 0;
		else if (COM_FilenamesEqual (filepath, "gfx/menu/netp2.lmp"))
			player_idx = 1;
		else if (COM_FilenamesEqual (filepath, "gfx/menu/netp3.lmp"))
			player_idx = 2;
		else if (COM_FilenamesEqual (filepath, "gfx/menu/netp4.lmp"))
			player_idx = 3;
		else if (COM_FilenamesEqual (filepath, "gfx/menu/netp5.lmp"))
			player_idx = 4;
	}
	else
#endif
	if (COM_FilenamesEqual(filepath, "gfx/menuplyr.lmp"))
	{
		player_idx = 0;
	}

	if (player_idx >= 0)
	{
	//	assert (width*height <= sizeof(menuplyr_pixels[player_idx]));
		if (menuplyr_pixels[player_idx])
			free (menuplyr_pixels[player_idx]);
		menuplyr_pixels[player_idx] = malloc (width*height);
		memcpy (menuplyr_pixels[player_idx], data, width*height);
	}
*/
	pic = &cachepics[numcachepics++];
	Q_strcpy (pic->name, filepath, sizeof(pic->name));

	if (!no24bit)
	{
		char *name, path[MAX_QPATH];

		name = COM_SkipPath (filepath);
		Q_strncpy (path, sizeof(path), filepath, name-filepath);
		//path[name-filepath] = 0;

		pic_24bit = GL_LoadPicImage (path, name, NULL, TEX_ALPHA | TEX_NOTILE, 0);
		if (pic_24bit)
		{
			//memcpy (&pic->pic.texnum, &pic_24bit->texnum, sizeof(mpic_t) - 8);
			pic->pic = *pic_24bit;
		}
	}
	else pic_24bit = NULL;

	pic->pic.width = width;
	pic->pic.height = height;

	if (!pic_24bit)
		GL_LoadPicTexture (filepath, &pic->pic, data, TEX_ALPHA | TEX_NOTILE, 1);

	return &pic->pic;
}

/*
================
Draw_GetCachePic
  (JDH: originally named Draw_CachePic)
================
*/
mpic_t *Draw_GetCachePic (const char *filepath, qboolean crash)
{
	mpic_t	*pic;
	qpic_t	*dat;

	pic = Draw_FindCachePic (filepath);
	if (pic)
		return pic;

	if (!(dat = (qpic_t *) COM_LoadTempFile (filepath, 0)))
	{
		if (crash)
			Sys_Error ("Draw_GetCachePic: failed to load %s", filepath);
		return NULL;
	}

	SwapPic (dat);
	return Draw_AddCachePic (filepath, dat->data, dat->width, dat->height);
}

/*
===============
Draw_CopyPicToPic
===============
*/
void Draw_CopyPicToPic (const qpic_t *pic_src, qpic_t *pic_dest, int x, int y)
{
	int width, height, i;
	const byte *src;
	byte *dest;

	src = pic_src->data;
	width = pic_src->width;
	height = pic_src->height;

	if (x < 0)
	{
		width += x;
		src -= x;
		x = 0;
	}

	if (y < 0)
	{
		height += y;
		src -= y*pic_src->width;
		y = 0;
	}

	width = min(width, pic_dest->width - x);
	height = min(height, pic_dest->height - y);

	dest = pic_dest->data + y*pic_dest->width + x;
	for (i = 0; i < height; i++)
	{
		memcpy (dest, src, width);
		dest += pic_dest->width;
		src += pic_src->width;
	}
}

/*void Draw_CharToConback (int num, byte *dest)
{
	int	row, col, drawline, x;
	byte	*source;

	row = num >> 4;
	col = num & 15;
	source = draw_chars + (row<<10) + (col<<3);

	drawline = 8;

	while (drawline--)
	{
		for (x=0 ; x<8 ; x++)
			if (source[x] != 255)
				dest[x] = 0x60 + source[x];
		source += 128;
		dest += 320;
	}
}
*/
void Draw_InitConback (void)
{
	qpic_t		*cb;
	int			start;
	mpic_t		*pic_24bit;
	const char	*path;
#if 0
	int			x, y;
	byte		*dest;
	char		ver[40];
#endif

	start = Hunk_LowMark ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		path = "gfx/menu/";
	else
#endif
	path = "gfx/";

	cb = (qpic_t *) COM_LoadHunkFile (va ("%sconback.lmp", path), 0);
	if (!cb)
		Sys_Error (va("Couldn't load %sconback.lmp", path));

	SwapPic (cb);

/*****JDH******/
//	if (cb->width != 320 || cb->height != 200)
//		Sys_Error ("Draw_InitConback: conback.lmp size is not 320x200");
/*****JDH******/

	/*
#if 0	// hack the version number directly into the pic
	sprintf (ver, "j0e-%4.2fbeta", (float)JOEQUAKE_VERSION);

	dest = cb->data + 320*186 + 320 - 11 - 8*strlen(ver);
	y = strlen(ver);
	for (x=0 ; x<y ; x++)
		Draw_CharToConback (ver[x], dest + (x<<3));
#endif
		*/

	if ((pic_24bit = GL_LoadPicImage(path, "conback", "conback", 0, com_filepath->dir_level)))
	{
//		memcpy (&conback->texnum, &pic_24bit->texnum, sizeof(mpic_t) - 8);		// this line bothers me...
	//	memcpy (&draw_conback, pic_24bit, sizeof(mpic_t));
		draw_conback = *pic_24bit;
	}
	else
	{
		draw_conback.width = cb->width;
		draw_conback.height = cb->height;
		GL_LoadPicTexture ("conback", &draw_conback, cb->data, 0, 1);
	}

	draw_conback.width = vid.conwidth;
	draw_conback.height = vid.conheight;

	// free loaded console
	Hunk_FreeToLowMark (start);
}

/*
===============
GL_LoadCharsetImage
===============
*/
int GL_LoadCharsetImage (const char *filename, const char *identifier)
{
	int			i, j, texnum, image_size;
	byte		*data, *buf, *dest, *src;
	qboolean	transparent = false;

	if (no24bit)
		return 0;

	if (!(data = GL_LoadImage("textures/charsets/", filename, 0, 0)))
		return 0;

	if (!identifier)
		identifier = filename;

	image_size = image_width * image_height;

	for (j=0 ; j<image_size ; j++)
	{
		if (((((unsigned *)data)[j] >> 24) & 0xFF) < 255)
		{
			transparent = true;
			break;
		}
	}
	if (!transparent)
	{
		for (i=0 ; i < image_width * image_height ; i++)
		{
			if (data[i*4] == data[0] && data[i*4+1] == data[1] && data[i*4+2] == data[2])
				data[i*4+3] = 0;
		}
	}


	buf = dest = Q_calloc (image_size * 2, 4);
	src = data;
	for (i=0 ; i<16 ; i++)
	{
		memcpy (dest, src, image_size >> 2);
		src += image_size >> 2;
		dest += image_size >> 1;
	}

	texnum = GL_LoadTexture (identifier, image_width, image_height * 2, buf, TEX_ALPHA, 4);

	if (texnum)
	{
		charset_hspacing = 1.0;
		charset_vspacing = 2.0;
	}

	free (buf);
	free (data);

	return texnum;
}

void GL_SetFontFilter (float filter)
{
// assumes charset texture is currently bound
	if (filter)
	{
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

qboolean OnChange_gl_smoothfont (cvar_t *var, const char *string)
{
	float	newval = Q_atof (string);

	if (char_texture)
	{
		GL_Bind (char_texture);
		GL_SetFontFilter (newval);
	}

	if (char_texture_bright)
	{
		GL_Bind (char_texture_bright);
		GL_SetFontFilter (newval);
	}

	M_OnChange_gl_smoothfont (newval);
	return false;		// allow change
}

static qboolean Draw_LoadConchars (const byte *data, int width, int height, int rows, int cols)
{
	int			charwidth, charheight, newsize, i, j;
	const byte	*src;
	byte		*buf, *dest;
	unsigned	*buf32, r, g, b;

	charwidth = width/cols;
	charheight = height/rows;

	if ((width*2 <= gl_max_size.value) && (height*2 <= gl_max_size.value))
	{
	// Double the texture's width and height, leaving empty space between chars
	// so that they don't stumble on each other because of texture smoothing.
	// This hack costs us an extra 192K of GL texture memory

		newsize = width*height*4;
		buf = Q_malloc (newsize);
		memset (buf, 255, newsize);

		src = data;
		dest = buf;
		for (i=0 ; i<rows ; i++)
		{
			for (j = 0; j < (width/charwidth)*charheight; j++)
			{
				memcpy (dest, src, charwidth);
				dest[charwidth] = dest[charwidth-1];		// duplicate last pixel of each row in char
				src += charwidth;
				dest += charwidth*2;
				dest[-1] = *src;		// precede first pixel of each row in char with duplicate
			}
			memcpy (dest, dest-width*2, width*2);		// JDH: duplicate bottom row of pixels
			dest += width*2*charheight;
		}

		width *= 2;
		height *= 2;
		charset_hspacing = charset_vspacing = 2.0;
	}
	else
	{
		newsize = width*height;
		buf = (byte *) data;
		charset_hspacing = charset_vspacing = 1.0;
	}


	char_texture = GL_LoadTexture ("pic:charset", width, height, buf, TEX_ALPHA | TEX_NOFILTER, 1);
		// TEX_NOFILTER since they obey gl_smoothfont cvar, not gl_texturemode
	GL_SetFontFilter (gl_smoothfont.value);

// JDH: create a brighter version for auto-complete text (colors get inverted when drawn)
	buf32 = Q_malloc (newsize*4);		// 32-bit color

	for (i = 0; i < newsize; i++)
	{
		r =   ((d_8to24table[buf[i]] & 0x000000FF)        * 1.5);
		r = min(r, 255);

		g =  (((d_8to24table[buf[i]] & 0x0000FF00) >>  8) * 1.5);
		g = min(g, 255);

		b =  (((d_8to24table[buf[i]] & 0x00FF0000) >> 16) * 1.5);
		b = min(b, 255);

		buf32[i] = buf[i] == 255 ? 0 : 0xFF000000;
		buf32[i] |= ((b << 16) | (g << 8) | r);
	}

	char_texture_bright = GL_LoadTexture ("pic:charset2", width, height, buf32, TEX_ALPHA | TEX_NOFILTER, 4);
	GL_SetFontFilter (gl_smoothfont.value);

	free (buf32);
	if (buf != data)
		free (buf);
	return true;
}

#define CONCHARS_NUMROWS 16
#define CONCHARS_NUMCOLS 16

#ifdef HEXEN2_SUPPORT
#  define CONCHARS_NUMROWS_H2  16
#  define CONCHARS_NUMCOLS_H2  32
#endif

qboolean Draw_LoadCharsetData (const char *name, byte *data, int size)
{
	int i, width, height, rows, cols, texnum;

	if (data && !Q_strcasecmp(name, gl_consolefont.defaultvalue))
	{
		for (i=0 ; i<size ; i++)
			if (data[i] == 0)
				data[i] = 255;	// proper transparent color

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			height = sqrt(size/2);
			width = 2*height;
			rows = CONCHARS_NUMROWS_H2;
			cols = CONCHARS_NUMCOLS_H2;

		//	char_texture = GL_LoadTexture ("charset", width, height, data, TEX_ALPHA, 1);
		}
		else
	#endif
		{
			width = height = sqrt(size);
			rows = CONCHARS_NUMROWS;
			cols = CONCHARS_NUMCOLS;
		}

		return Draw_LoadConchars (data, width, height, rows, cols);
	}

	texnum = GL_LoadCharsetImage (name, "pic:charset");
	if (!texnum)
	{
		Con_Printf ("Couldn't load charset \"%s\"\n", name);
		return false;
	}

	char_texture = texnum;
	return true;
}

qboolean Draw_LoadCharset (const char *name)
{
	byte		*data;
	int			size;
	qboolean	result;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		data = COM_LoadMallocFile ("gfx/menu/conchars.lmp", 0);
		if (data)
		{
			result = Draw_LoadCharsetData (name, data, com_filesize);
			free (data);
		}
	}
	else
#endif
	{
		data = W_GetLumpByName ("conchars", &size);
		result = Draw_LoadCharsetData (name, data, size);
	}

	return result;
}

qboolean OnChange_gl_consolefont (cvar_t *var, const char *string)
{
	return !Draw_LoadCharset (string);
}

void Draw_LoadCharset_f (cmd_source_t src)
{
	switch (Cmd_Argc())
	{
	case 1:
		Con_Printf ("Current charset is \"%s\"\n", gl_consolefont.string);
		break;

	case 2:
		Cvar_SetDirect (&gl_consolefont, Cmd_Argv(1));
		break;

	default:
		Con_Printf ("Usage: %s <charset>\n", Cmd_Argv(0));
	}
}

void Draw_InitCharset (void)
{
	int    size, i, j;
	qpic_t *pic;
	mpic_t *pic_24bit;
	byte   *data;

	Draw_LoadCharset (gl_consolefont.string);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		data = W_GetLumpByName ("tinyfont", &size);
		if (data)
		{
			for (i=0 ; i<128*32 ; i++)
				if (data[i] == 0)
					data[i] = 255;	// proper transparent color

			// now turn them into textures
			char_smalltexture = GL_LoadTexture ("smallcharset", 128, 32, data, TEX_ALPHA, 1);
		}
		else char_smalltexture = 0;

		pic = (qpic_t *) COM_LoadTempFile ("gfx/menu/bigfont2.lmp", 0);
		if (pic)
		{
			SwapPic (pic);
			for (i=0 ; i<pic->width*pic->height ; i++)
				if (pic->data[i] == 0)
					pic->data[i] = 255;	// proper transparent color

			draw_menufont.width = pic->width;
			draw_menufont.height = pic->height;
			GL_LoadPicTexture ("menufont", &draw_menufont, pic->data, TEX_ALPHA, 1);

			//char_menufonttexture = GL_LoadTexture ("menufont", pic->width, pic->height, pic->data, TEX_ALPHA, 1);
		}
		else memset (&draw_menufont, 0, sizeof(draw_menufont));

		// Load the table of character widths:
		COM_LoadStackFile ("gfx/menu/fontsize.lmp", BigCharWidth, sizeof(BigCharWidth), 0);
	}
	else
#endif
	{
//		Draw_LoadCharset (gl_consolefont.string, width, height);

		if (!char_texture)
			Cvar_SetDirect (&gl_consolefont, "original");

		if ((pic_24bit = GL_LoadPicImage ("gfx/", "mcharset", "menufont", TEX_ALPHA, 0)))
		{
			draw_menufont = *pic_24bit;
			draw_menufont.width = DRAW_BIGCHARWIDTH*8;		// so each char occupies 24 pixels onscreen
			draw_menufont.height = DRAW_BIGCHARWIDTH*8;

			for (i = 0; i < 64; i++)
				for (j = 0; j < 64; j++)
					MenuFontWidths[i][j] = DRAW_BIGCHARWIDTH;
		}
		else
		{
			pic = (qpic_t *) COM_LoadTempFile ("gfx/menufont.lmp", 0);
			if (!pic)
				pic = (qpic_t *) COM_LoadTempFile ("gfx/bigfont.lmp", 0);

			if (pic)
			{
				SwapPic (pic);
				for (i=0 ; i<pic->width*pic->height ; i++)
					if (pic->data[i] == 254)
						pic->data[i] = 255;	// convert white bounding boxes to transparent  (TEMP FIX!!)

				draw_menufont.width = pic->width;
				draw_menufont.height = pic->height;
				GL_LoadPicTexture ("menufont", &draw_menufont, pic->data, TEX_ALPHA, 1);

				// char_menufonttexture = GL_LoadTexture ("menufont", pic->width, pic->height, pic->data, TEX_ALPHA, 1);

				// Load the table of character widths:
				COM_LoadStackFile ("gfx/fontsize.lmp", MenuFontWidths, sizeof(MenuFontWidths), 0);

				for (i = 0; i < 64; i++)
					for (j = 0; j < 64; j++)
						if (MenuFontWidths[i][j] == 0)
							MenuFontWidths[i][j] = pic->width/10.0;		// **TEMP** (until I finish font)
			}
			else memset (&draw_menufont, 0, sizeof(draw_menufont));
		}
	}

	if (!char_texture)
		Sys_Error ("Draw_InitCharset: Couldn't load charset");

// JDH: load extra characters
/*	pic = (qpic_t *) COM_LoadTempFile ("gfx/conchars2.lmp", 0);
	if (pic)
	{
		SwapPic (pic);
		for (i=0 ; i<pic->width*pic->height ; i++)
			if (pic->data[i] == 0)
				pic->data[i] = 255;	// proper transparent color

		char_texture2 = GL_LoadTexture ("conchars2", pic->width, pic->height, pic->data, TEX_ALPHA, 1);
	}
*/
}

static const char *crosshair_pathlist[3] = {"crosshairs/", "textures/crosshairs/", NULL};

qboolean OnChange_gl_crosshairimage (cvar_t *var, const char *string)
{
	mpic_t	*pic;
	const char	*namelist[2];

	if (!string[0])
	{
		crosshairimage_loaded = false;
		return false;
	}

	namelist[0] = string;
	namelist[1] = NULL;

	if (!(pic = GL_LoadPicImage_MultiSource (crosshair_pathlist, namelist, "crosshair", TEX_ALPHA, 0)))
	{
		crosshairimage_loaded = false;
		if (key_dest != key_menu)
			Con_Printf ("Couldn't load crosshair \"%s\"\n", string);

		return false;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	crosshairpic = *pic;
	crosshairimage_loaded = true;

	return false;
}

/*
===============
Draw_LoadWadPics
===============
*/
void Draw_LoadWadPics (void)
{
#ifdef HEXEN2_SUPPORT
	int i;

	// get the other pics we need
	if (hexen2)
	{
		for (i = 0; i < MAX_DISC; i++)
		{
			draw_disc_H2[i] = Draw_GetCachePic (va ("gfx/menu/skull%d.lmp", i), true);
		}

		draw_disc = draw_disc_H2[0];		// just in case - but draw_disc shouldn't get used
		draw_backtile = Draw_GetCachePic ("gfx/menu/backtile.lmp", true);
	}
	else
#endif
	{
	#ifdef HEXEN2_SUPPORT
		for (i = 0; i < MAX_DISC; i++)
		{
			draw_disc_H2[i] = NULL;
		}
	#endif
		draw_disc = Draw_PicFromWad ("disc");
		draw_backtile = Draw_PicFromWad ("backtile");
	}

	for (i = 0; i < STAT_MINUS; i++)
	{
		draw_bignums[0][i] = Draw_PicFromWad (va("num_%i", i));
	}

	draw_bignums[0][STAT_MINUS] = Draw_PicFromWad ("num_minus");
	draw_bigcolon = Draw_PicFromWad ("num_colon");
	draw_bigslash = Draw_PicFromWad ("num_slash");

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
	{
#endif
		for (i=0 ; i<STAT_MINUS ; i++)
		{
			draw_bignums[1][i] = Draw_PicFromWad (va("anum_%i", i));
		}

		draw_bignums[1][STAT_MINUS] = Draw_PicFromWad ("anum_minus");
	}

	Sbar_LoadWadPics ();
	SCR_LoadWadPics ();
}

/*
==================
Draw_ReloadWadFile
==================
*/
void Draw_ReloadWadFile (void)
{
	int i;

	if (cls.state == ca_dedicated)
		return;

	if (!W_LoadWadFile ("gfx.wad"))
		return;

	draw_disc = NULL;
	draw_backtile = NULL;

#ifdef HEXEN2_SUPPORT
	for (i = 0; i < MAX_DISC; i++)
	{
		draw_disc_H2[i] = NULL;
	}
#endif

// JDH: moved big numbers here from sbar.c
	for (i = 0; i <= STAT_MINUS; i++)
	{
		draw_bignums[0][i] = draw_bignums[1][i] = NULL;
	}

	draw_bigcolon = draw_bigslash = NULL;

	SCR_ClearWadPics ();
	Sbar_ClearWadPics ();

	Draw_LoadWadPics ();

	Draw_InitCharset ();
	Draw_InitConback ();
}

/*
===============
Draw_Init
===============
*/
void Draw_Init (void)
{
	int	i, j;
	unsigned data24[8*8];

#define MAKEGREY(c) (0xFF000000 | ((int)(c) << 16) | ((int)(c) << 8) | (c))

	Cmd_AddCommand ("loadcharset", Draw_LoadCharset_f, 0);

	Cvar_RegisterBool (&gl_externaltextures_world);
	Cvar_RegisterBool (&gl_externaltextures_bmodels);
	Cvar_RegisterBool (&gl_externaltextures_models);

	Cvar_RegisterString (&gl_consolefont);
	Cvar_RegisterFloat (&gl_conalpha, 0, 1);
	Cvar_RegisterBool (&gl_smoothfont);

	Cvar_RegisterFloat (&gl_crosshairalpha, 0, 1);
	Cvar_RegisterString (&gl_crosshairimage);
	Cvar_RegisterInt (&crosshair, 0, NUMCROSSHAIRS+1);
	Cvar_RegisterString (&crosshaircolor);
	Cvar_RegisterFloat (&crosshairsize, 0.5, 4);
	Cvar_RegisterInt (&cl_crossx, -(int)vid.width/8, vid.width/8);
	Cvar_RegisterInt (&cl_crossy, -(int)vid.height/8, vid.height/8);

	Cmd_AddLegacyCommand ("con_alpha", gl_conalpha.name);
	
	GL_TextureInit ();

	// load the console background and the charset by hand, because we need to write the version
	// string into the background before turning it into a texture
	Draw_InitCharset ();
	Draw_InitConback ();
/*
	// save a texture slot for translated picture
	translate_texture = texture_extension_number++;		--> to M_Init
#ifdef HEXEN2_SUPPORT
	texture_extension_number += NUM_CLASSES-1;
#endif
*/
	// save slots for scraps
//	scrap_texnum = texture_extension_number;	// --> to GL_TextureInit
//	texture_extension_number += MAX_SCRAPS;

	// Load the crosshair pics
	for (i=0 ; i<NUMCROSSHAIRS ; i++)
	{
		for (j = 0; j < 8*8; j++)
			data24[j] = crosshairdata[i][j] ? MAKEGREY(crosshairdata[i][j]) : 0;

		crosshairtextures[i] = GL_LoadTexture ("", 8, 8, (byte *)data24, TEX_ALPHA, 4);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	Draw_LoadWadPics ();
}

/*
================
Draw_Character

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Character_Scaled (int x, int y, int num, float scale)
{
	int		numrows, numcols;
	float	frow, fcol, xsize, ysize;

	if (y <= -DRAW_CHARWIDTH)
		return;			// totally off screen

	if (num == 32)
		return;		// space

#ifdef HEXEN2_SUPPORT
	if (hexen2)			// charset is 256x128 (16 rows of 32 chars)
	{
		numrows = CONCHARS_NUMROWS_H2;
		numcols = CONCHARS_NUMCOLS_H2;
/*
		num &= 511;

	//	xsize = 1/32.0;
	//	ysize = 1/16.0;
		xsize = 1/64.0;
		ysize = 1/32.0;

		fcol = (num & 31) * xsize * 2;
		frow = (num >> 5) * ysize * 2;
*/
	}
	else
#endif
	{
		numrows = CONCHARS_NUMROWS;
		numcols = CONCHARS_NUMCOLS;

/*		num &= 255;

		xsize = 1/32.0;
		ysize = 1/32.0;		// since space is inserted between rows when texture is loaded
		fcol = (num & 15) * xsize * 2;
		frow = (num >> 4) * ysize * 2;
*/
	}

	num &= (numcols * numrows)-1;
	xsize = 1.0 / (numcols * charset_hspacing);
	ysize = 1.0 / (numrows * charset_vspacing);
	fcol = (num % numcols) / (float) numcols;	//* xsize * charset_hspacing;
	frow = (num / numcols) / (float) numrows;	//* ysize * charset_vspacing;

	GL_Bind (char_texture);

	glBegin (GL_QUADS);
	glTexCoord2f (fcol, frow);
	glVertex2f (x, y);
	glTexCoord2f (fcol + xsize, frow);
	glVertex2f (x + DRAW_CHARWIDTH*scale, y);
	glTexCoord2f (fcol + xsize, frow + ysize);
	glVertex2f (x + DRAW_CHARWIDTH*scale, y + DRAW_CHARHEIGHT*scale);
	glTexCoord2f (fcol, frow + ysize);
	glVertex2f (x, y + DRAW_CHARHEIGHT*scale);
	glEnd ();
}

/*
================
Draw_Character2 (JDH)
  - from the extended character set conchars2.lmp (assumed to be 64x16, ie. 2 rows of 8 chars)
================
*/
/*
void Draw_Character2 (int x, int y, int num)
{
	float	frow, fcol;

	if (!char_texture2 || (y <= -8))
		return;			// totally off screen

	num &= 0x0F;
	frow = (num >> 3) / 2.0;
	fcol = (num & 7) / 8.0;

	GL_Bind (char_texture2);

	glBegin (GL_QUADS);
	glTexCoord2f (fcol, frow);
	glVertex2f (x, y);
	glTexCoord2f (fcol + 1/8.0, frow);
	glVertex2f (x + 8, y);
	glTexCoord2f (fcol + 1/8.0, frow + 1/2.0);
	glVertex2f (x + 8, y + 8);
	glTexCoord2f (fcol, frow + 1/2.0);
	glVertex2f (x, y + 8);
	glEnd ();
}
*/
/*
================
Draw_String
================
*/
void Draw_String (int x, int y, const char *str)
{
//	float		frow, fcol, xsize, ysize;
	int		num;

	if (y <= -DRAW_CHARHEIGHT)
		return;			// totally off screen

	if (!*str)
		return;
/*
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		xsize = 1/64.0;
		ysize = 1/32.0;
	}
	else
#endif
	{
		xsize = 1/32.0;
		ysize = 1/32.0;
	}

	GL_Bind (char_texture);

	glBegin (GL_QUADS);
*/
	while (*str)		// stop rendering when out of characters
	{
		if ((num = *str++) != 32)	// skip spaces
		{
			Draw_Character (x, y, num);
		/*
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				fcol = (float)(num & 31) * xsize * 2;
				frow = (float)(num >> 5) * ysize * 2;
			}
			else
		#endif
			{
				fcol = (float)(num & 15) * xsize * 2;
				frow = (float)(num >> 4) * ysize * 2;
			}
			glTexCoord2f (fcol, frow);
			glVertex2f (x, y);
			glTexCoord2f (fcol + xsize, frow);
			glVertex2f (x+8, y);
			glTexCoord2f (fcol + xsize, frow + ysize);
			glVertex2f (x+8, y+8);
			glTexCoord2f (fcol, frow + ysize);
			glVertex2f (x, y+8);
		*/
		}
		x += DRAW_CHARWIDTH;
	}

//	glEnd ();
}

/*
================
Draw_Alt_String
================
*/
void Draw_Alt_String (int x, int y, const char *str)
{
/*	char buf[1024];
	int i;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		//Draw_String( x, y, str );
		while (*str)
		{
			Draw_Character (x, y, (int)*str | 0x0100);
			str++;
			x += 8;
		}
		return;
	}
#endif

	for (i=0; i < sizeof(buf)-1 && *str; i++, str++)
	{
		buf[i] = (*str == 32) ? 32 : *str | 0x80;
	}

	buf[i] = 0;
	Draw_String (x, y, buf);
*/
	int mask;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		mask = 0x0100;
	else
#endif
		mask = 0x80;

	while (*str)
	{
		Draw_Character (x, y, (int)*str | mask);
		str++;
		x += DRAW_CHARWIDTH;
	}
}

#define MENUFONT_ROWS 8
#define MENUFONT_COLS 8

#ifdef HEXEN2_SUPPORT
#  define MENUFONT_ROWS_H2 4
#endif

#define BIGCHARINDEX(c) \
	if ((c) == '/') (c) = 63;				\
	else if ((c) == ':') (c) = 62;			\
	else if (((c) >= '0') && ((c) <= '9'))	\
		(c) = (c) - '0' + 51;				\
	else if (((c) >= 'a') && ((c) <= 'z'))	\
		(c) = (c) - 'a' + 26;				\
	else if (((c) >= 'A') && ((c) <= 'Z'))	\
		(c) -= 'A';							\
	else									\
		(c) = -1;

/*
================
Draw_BigCharacter
================
*/
int Draw_BigCharacter (int x, int y, int num, int numNext)
{
	int		size;
	float	frow, fcol, xfraction, yfraction;

	if (!draw_menufont.texnum)
		return 0;

	if (num == ' ') return 32;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (num == '/') num = 26;
		else num -= 65;

		if (num < 0 || num >= 27)  // only a-z and /
			return 0;

		if (numNext == '/') numNext = 26;
		else numNext -= 65;

		yfraction = 1.0 / MENUFONT_ROWS_H2;
//		size = 20;
	}
	else
#endif
	{
		BIGCHARINDEX(num);
		if (num < 0)
			return 0;

		BIGCHARINDEX(numNext);

		yfraction = 1.0 / MENUFONT_ROWS;
//		size = 32;
	}

	xfraction = 1.0 / MENUFONT_COLS;
	size = draw_menufont.width * xfraction;		// onscreen size

	xfraction *= draw_menufont.sh;
	yfraction *= draw_menufont.th;
	fcol = (num % MENUFONT_COLS) * xfraction;
	frow = (num / MENUFONT_COLS) * yfraction;

//	GL_Bind (char_menufonttexture);
	GL_Bind (draw_menufont.texnum);

	glBegin (GL_QUADS);
	glTexCoord2f (fcol, frow);
	glVertex2f (x, y);
	glTexCoord2f (fcol + xfraction, frow);
	glVertex2f (x+size, y);
	glTexCoord2f (fcol + xfraction, frow + yfraction);
	glVertex2f (x+size, y+size);
	glTexCoord2f (fcol, frow + yfraction);
	glVertex2f (x, y+size);
	glEnd ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		int add;

		if (numNext < 0 || numNext >= 27) return 0;

		add = 0;
		if ((num == 'C'-65) && (numNext == 'P'-65))
			add = 3;

		return BigCharWidth[num][numNext] + add;
	}
#endif

	if (numNext < 0)
		return 0;

	return MenuFontWidths[num][numNext];
}

/*
================
Draw_BigString
================
*/
void Draw_BigString (int x, int y, const char *string)
{
	int c;

	for (c=0; string[c]; c++)
	{
		x += Draw_BigCharacter (x, y, string[c], string[c+1]);
	}
}

/*
================
Draw_BigNumString
   string must contain *only* 0-9, dash, forward slash, colon
================
*/
void Draw_BigNumString (int x, int y, const char *str, int flags)
{
	int len, charwidth, color, index, width;
	mpic_t *pic;
	float scale;

	len = strlen (str);
	if (flags & DRAWNUM_ALIGNRIGHT)
		str += len-1;

	color = (flags & DRAWNUM_ALTCOLOR) ? 1 : 0;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		charwidth = DRAW_BIGCHARWIDTH_H2;
		flags &= ~DRAWNUM_TIGHT;
	}
	else
#endif
	charwidth = DRAW_BIGCHARWIDTH;

//	(flags & DRAWNUM_TIGHT) ? DRAW_BIGCHARWIDTH-2 : DRAW_BIGCHARWIDTH;

	while (len--)
	{
		if (*str == ':')
		{
			pic = draw_bigcolon;
			width = DRAW_BIGCHARWIDTH/2;
		}
		else
		{
			if (*str == '/')
			{
				pic = draw_bigslash;
				width = (DRAW_BIGCHARWIDTH*2)/3;
			}
			else
			{
				index = (*str == '-') ? STAT_MINUS : *str -'0';
				pic = draw_bignums[color][index];
				width = (flags & DRAWNUM_TIGHT) ? charwidth-2 : charwidth;
			}
		}

	#ifdef HEXEN2_SUPPORT
		// Hexen II has full-size colon & slash, but smaller numbers
		if (hexen2)
			scale = (float)charwidth/pic->width;
		else
	#endif
			scale = 1.0;


		if (flags & DRAWNUM_ALIGNRIGHT)
		{
			x -= width;
			Draw_TransPic_Scaled (x, y, pic, scale);
			str--;
		}
		else
		{
			Draw_TransPic_Scaled (x, y, pic, scale);
			x += width;
			str++;
		}
	}
}

#ifdef HEXEN2_SUPPORT

/*
==========================================================
Draw_SmallString
 - draws a string (using small characters) that is clipped
   at the bottom edge of the screen
==========================================================
*/
void Draw_SmallString (int x, int y, const char *str)
{
	float	xsize, ysize, frow, fcol;
	int		num, row, col;

	if (y <= -DRAW_CHARHEIGHT)
		return; 		// totally off screen

	if (y >= vid.height)
		return;			// totally off screen

	xsize = 0.0625;
	ysize = 0.25;

	GL_Bind (char_smalltexture);

	for ( ; *str; str++, x += 6 )
	{
		num = *str;
		if (num < 32)
			continue;

		if (num >= 'a' && num <= 'z')
			num -= 64;
		else if (num > '_')
			continue;
		else
			num -= 32;

		row = num >> 4;
		col = num & 15;

		fcol = col*xsize;
		frow = row*ysize;


		glBegin (GL_QUADS);
		glTexCoord2f (fcol, frow);
		glVertex2f (x, y);
		glTexCoord2f (fcol + xsize, frow);
		glVertex2f (x+DRAW_CHARWIDTH, y);
		glTexCoord2f (fcol + xsize, frow + ysize);
		glVertex2f (x+DRAW_CHARWIDTH, y+DRAW_CHARHEIGHT);
		glTexCoord2f (fcol, frow + ysize);
		glVertex2f (x, y+DRAW_CHARHEIGHT);
		glEnd ();
	}
}

#endif	// #ifdef HEXEN2_SUPPORT

int StringToRGB (const char *s, byte rgb[4])
{
	byte		*col;
	cmd_arglist_t args;

	Cmd_TokenizeString (s, &args);
	if (args.argc == 3)
	{
		rgb[0] = (byte)Q_atoi (args.argv[0]);
		rgb[1] = (byte)Q_atoi (args.argv[1]);
		rgb[2] = (byte)Q_atoi (args.argv[2]);
	}
	else
	{
		col = (byte *)&d_8to24table[(byte)Q_atoi (s)];

		rgb[0] = col[0];
		rgb[1] = col[1];
		rgb[2] = col[2];
	}
	rgb[3] = 255;
	return args.argc;
}

/*
================
Draw_Crosshair		-- joe, from FuhQuake
================
*/
/*void Draw_Crosshair (void)
{
	float		x, y, ofs1, ofs2, sh, th, sl, tl;
	byte		col[4];
	extern vrect_t	scr_vrect;

//	if ((crosshair.value >= 2 && crosshair.value <= NUMCROSSHAIRS + 1) || crosshairimage_loaded)
	if ((crosshair.value >= 1 && crosshair.value <= NUMCROSSHAIRS + 1) || crosshairimage_loaded)
	{
		x = scr_vrect.x + scr_vrect.width / 2 + cl_crossx.value;
		y = scr_vrect.y + scr_vrect.height / 2 + cl_crossy.value;

		if (crosshairimage_loaded || (crosshair.value >= 2))
		{
			if (!gl_crosshairalpha.value)
				return;

			StringToRGB (crosshaircolor.string, col);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			if (gl_crosshairalpha.value)
			{
				glDisable (GL_ALPHA_TEST);
				glEnable (GL_BLEND);
				col[3] = bound(0, gl_crosshairalpha.value, 1) * 255;
				glColor4ubv (col);
			}
			else
			{
				glColor3ubv (col);
			}

			if (crosshairimage_loaded)
			{
				GL_Bind (crosshairpic.texnum);
				ofs1 = 4 - 4.0 / crosshairpic.width;
				ofs2 = 4 + 4.0 / crosshairpic.width;
				sh = crosshairpic.sh;
				sl = crosshairpic.sl;
				th = crosshairpic.th;
				tl = crosshairpic.tl;
			}
			else
			{
				GL_Bind (crosshairtextures[(int)crosshair.value-2]);
				ofs1 = 3.5;
				ofs2 = 4.5;
				tl = sl = 0;
				sh = th = 1;
			}

			ofs1 *= (vid.width / 320) * bound(0, crosshairsize.value, 20);
			ofs2 *= (vid.width / 320) * bound(0, crosshairsize.value, 20);

			glBegin (GL_QUADS);
			glTexCoord2f (sl, tl);
			glVertex2f (x - ofs1, y - ofs1);
			glTexCoord2f (sh, tl);
			glVertex2f (x + ofs2, y - ofs1);
			glTexCoord2f (sh, th);
			glVertex2f (x + ofs2, y + ofs2);
			glTexCoord2f (sl, th);
			glVertex2f (x - ofs1, y + ofs2);
			glEnd ();

			glColor3f (1, 1, 1);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			if (gl_crosshairalpha.value)
			{
				glDisable (GL_BLEND);
				glEnable (GL_ALPHA_TEST);
			}
		}
		else
		{
			sl = bound(0, crosshairsize.value, 20);
			Draw_CharacterScaled (x - 4*sl, y - 4*sl, '+', sl);
		}
	}
//	else if (crosshair.value)
//	{
//		Draw_Character (scr_vrect.x + scr_vrect.width / 2 - 4 + cl_crossx.value, scr_vrect.y + scr_vrect.height / 2 - 4 + cl_crossy.value, '+');
//	}
}
*/
void Draw_Crosshair (void)
{
	float		x, y, ofs1, ofs2, sh, th, sl, tl;
	byte		col[4];
	extern vrect_t	scr_vrect;

	if (!gl_crosshairalpha.value)
		return;

//	if ((crosshair.value >= 2 && crosshair.value <= NUMCROSSHAIRS + 1) || crosshairimage_loaded)
	if ((crosshair.value >= 1 && crosshair.value <= NUMCROSSHAIRS + 1) || crosshairimage_loaded)
	{
		x = scr_vrect.x + scr_vrect.width / 2 + cl_crossx.value;
		y = scr_vrect.y + scr_vrect.height / 2 + cl_crossy.value;

		StringToRGB (crosshaircolor.string, col);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (gl_crosshairalpha.value)
		{
			glDisable (GL_ALPHA_TEST);
			glEnable (GL_BLEND);
			col[3] = bound(0, gl_crosshairalpha.value, 1) * 255;
			glColor4ubv (col);
		}
		else
		{
			glColor3ubv (col);
		}


		//if (crosshairimage_loaded || (crosshair.value >= 2))
		{
			if (crosshairimage_loaded)
			{
				GL_Bind (crosshairpic.texnum);
				ofs1 = 4 - 4.0 / crosshairpic.width;
				ofs2 = 4 + 4.0 / crosshairpic.width;
				sh = crosshairpic.sh;
				sl = crosshairpic.sl;
				th = crosshairpic.th;
				tl = crosshairpic.tl;
			}
			else
			{
				GL_Bind (crosshairtextures[(int)crosshair.value-1]);
				ofs1 = 3.5;
				ofs2 = 4.5;
				tl = sl = 0;
				sh = th = 1;
			}

			ofs1 *= /*(vid.width / 320) * */bound(0, crosshairsize.value, 20);
			ofs2 *= /*(vid.width / 320) * */bound(0, crosshairsize.value, 20);

			glBegin (GL_QUADS);
			glTexCoord2f (sl, tl);
			glVertex2f (x - ofs1, y - ofs1);
			glTexCoord2f (sh, tl);
			glVertex2f (x + ofs2, y - ofs1);
			glTexCoord2f (sh, th);
			glVertex2f (x + ofs2, y + ofs2);
			glTexCoord2f (sl, th);
			glVertex2f (x - ofs1, y + ofs2);
			glEnd ();
		}
		/*else
		{
			sl = bound(0, crosshairsize.value, 20);
			Draw_CharacterScaled (x - 4*sl, y - 4*sl, '+', sl);
		}*/

		glColor3f (1, 1, 1);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		if (gl_crosshairalpha.value)
		{
			glDisable (GL_BLEND);
			glEnable (GL_ALPHA_TEST);
		}
	}
}

typedef enum {DRAW_BOXPICS_LEFT, DRAW_BOXPICS_MIDDLE, DRAW_BOXPICS_RIGHT} draw_boxpicscol_t;

static const char * const draw_boxpics[3][4] =
{
	{"gfx/box_tl.lmp", "gfx/box_ml.lmp", "gfx/box_ml.lmp",  "gfx/box_bl.lmp"},		// left
	{"gfx/box_tm.lmp", "gfx/box_mm.lmp", "gfx/box_mm2.lmp", "gfx/box_bm.lmp"},		// middle
	{"gfx/box_tr.lmp", "gfx/box_mr.lmp", "gfx/box_mr.lmp",  "gfx/box_br.lmp"}		// right
};

/*
================
Draw_TextBoxColumn
================
*/
mpic_t * Draw_GetTextBoxPic (draw_boxpicscol_t col, int index)
{
/*** HACK!! these pics look better with GL_REPEAT (instead of GL_CLAMP) ***/
	static mpic_t *pics[12];
	mpic_t **mpic;
	qpic_t *pic;

	mpic = &pics[col*4 + index];
	if (*mpic)
		return *mpic;

	pic = (qpic_t *) COM_LoadTempFile (draw_boxpics[col][index], 0);
	if (!pic)
		return NULL;

	*mpic = Draw_AddCachePic (draw_boxpics[col][index], pic->data, pic->width, pic->height);
	if (*mpic)
	{
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	return *mpic;

//	return Draw_GetCachePic (draw_boxpics[col][index], false);
}

/*
================
Draw_TextBoxColumn
================
*/
void Draw_TextBoxColumn (int x, int y, int width, int height, int piclist)
{
	mpic_t	*pics[4], *p;
	int		h;

	for (h = 0; h < 4; h++)
		pics[h] = Draw_GetTextBoxPic (piclist, h);

	Draw_SubPic (x, y, pics[0], 0, 0, width, 8);
	y += 8;

	p = pics[1];
	height++;		// see comment in Draw_TextBox
	while (height > 0)
	{
		h = min(height, 8);
		Draw_SubPic (x, y, p, 0, 0, width, h);
		p = pics[2];		// only topmost one uses pics[1]
		height -= h;
		y += h;
	}

	Draw_SubPic (x, y-1, pics[3], 0, 0, width, 8);
}

/*
================
Draw_TextBox
  JDH: modified to handle arbitrary sizes (not strict multiples of 8 and 16)
================
*/
void Draw_TextBox (int x, int y, int width, int height)
{
	int	w;

	// draw left side
	Draw_TextBoxColumn (x, y, 8, height, DRAW_BOXPICS_LEFT);
	x += 8;

	// draw middle
	width++;		// draw background part a bit wider, otherwise there can be gaps with
					// certain glOrtho values (if not, frame will be drawn over anyway)
	while (width > 0)
	{
		w = min(16, width);
		Draw_TextBoxColumn (x, y, w, height, DRAW_BOXPICS_MIDDLE);
		width -= w;
		x += w;
	}

	// draw right side
	Draw_TextBoxColumn (x-1, y, 8, height, DRAW_BOXPICS_RIGHT);
}

/*
================
Draw_DebugChar

Draws a single character directly to the upper right corner of the screen.
This is for debugging lockups by drawing different chars in different parts
of the code.
================
*/
void Draw_DebugChar (char num)
{
}

extern int	scrap_dirty;

/*
=============
Draw_Pic
=============
*/
void Draw_Pic_Scaled (int x, int y, const mpic_t *pic, float scale)
{
	if (!pic)
		return;

	if (scrap_dirty)
		Scrap_Upload ();

	GL_Bind (pic->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (pic->sl, pic->tl);
	glVertex2f (x, y);
	glTexCoord2f (pic->sh, pic->tl);
	glVertex2f (x + pic->width*scale, y);
	glTexCoord2f (pic->sh, pic->th);
	glVertex2f (x + pic->width*scale, y + pic->height*scale);
	glTexCoord2f (pic->sl, pic->th);
	glVertex2f (x, y + pic->height*scale);
	glEnd ();
}

/*
=============
Draw_AlphaPic
=============
*/
void Draw_AlphaPic (int x, int y, const mpic_t *pic, float alpha)
{
	if (!pic)
		return;

	glDisable(GL_ALPHA_TEST);
	glEnable (GL_BLEND);
	glCullFace (GL_FRONT);
	glColor4f (1, 1, 1, alpha);

	Draw_Pic (x, y, pic);

	glColor3f (1, 1, 1);
	glEnable(GL_ALPHA_TEST);
	glDisable (GL_BLEND);
}


void Draw_SubPic (int x, int y, const mpic_t *pic, int s, int t, int width, int height)
{
	float	/*newsl, newtl, newsh, newth,*/ oldglwidth, oldglheight;
	mpic_t	newpic;

	if (!pic)
		return;

	if (!s && !t && (width == pic->width) && (height == pic->height))
	{
		Draw_Pic (x, y, pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	oldglwidth = pic->sh - pic->sl;
	oldglheight = pic->th - pic->tl;
/*
	newsl = pic->sl + (srcx * oldglwidth) / pic->width;
	newsh = newsl + (width * oldglwidth) / pic->width;

	newtl = pic->tl + (srcy * oldglheight) / pic->height;
	newth = newtl + (height * oldglheight) / pic->height;

	GL_Bind (pic->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (newsl, newtl);
	glVertex2f (x, y);
	glTexCoord2f (newsh, newtl);
	glVertex2f (x+width, y);
	glTexCoord2f (newsh, newth);
	glVertex2f (x+width, y+height);
	glTexCoord2f (newsl, newth);
	glVertex2f (x, y+height);
	glEnd ();
*/
	newpic.sl = pic->sl + (s * oldglwidth) / pic->width;
	newpic.sh = newpic.sl + (width * oldglwidth) / pic->width;

	newpic.tl = pic->tl + (t * oldglheight) / pic->height;
	newpic.th = newpic.tl + (height * oldglheight) / pic->height;

	newpic.width = width;
	newpic.height = height;
	newpic.texnum = pic->texnum;

	Draw_Pic (x, y, &newpic);
}

/*
=============
Draw_TransPic
  JDH: changed to a macro, calling Draw_Pic directly
=============
*/
/*void Draw_TransPic (int x, int y, const mpic_t *pic)
{
//	if (x < 0 || (unsigned)(x + pic->width) > vid.width || y < 0 || (unsigned)(y + pic->height) > vid.height)
//		Sys_Error ("Draw_TransPic: bad coordinates");

	Draw_Pic (x, y, pic);
}
*/

/*
=============
Draw_TransPicTranslate

Only used for the player color selection menu
=============
*/
/*
void Draw_TransPicTranslate (int x, int y, mpic_t *pic, const byte *translation, int playertype)
{
#if 0
	int			v, u, p, oldtex;
	unsigned	trans[64*64], *dest;
	byte		*src;
#else
	byte		*src;
	unsigned	*trans;
	int			width, height, i;
#endif

	if (!pic)
		return;

#if 0
	dest = trans;
	for (v=0 ; v<64 ; v++, dest += 64)
	{
		src = &menuplyr_pixels[0][((v*pic->height)>>6)*pic->width];
		for (u=0 ; u<64 ; u++)
		{
			p = src[(u*pic->width)>>6];
			dest[u] = (p == 255) ? p : d_8to24table[translation[p]];
		}
	}

	GL_Bind (translate_texture);
	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else

#ifdef HEXEN2_SUPPORT
	src = menuplyr_pixels[playertype];
#else
	src = menuplyr_pixels[0];
#endif
	if (!src)
		return;

	width = pic->width;
	height = pic->height;

	trans = malloc (width*height*4);
	if (!trans)
		return;

	for (i = width*height - 1; i >= 0; i--)
		trans[i] = d_8to24table[translation[src[i]]];

#ifdef HEXEN2_SUPPORT
	pic->texnum = translate_texture + playertype;
#else
	pic->texnum = translate_texture;
#endif

	GL_Bind (pic->texnum);
	GL_Upload32 (NULL, trans, &width, &height, TEX_NOSTRETCH | TEX_ALPHA);
	free (trans);
#endif

//	oldtex = ((mpic_t *)pic)->texnum;
	Draw_Pic (x, y, pic);
//	((mpic_t *)pic)->texnum = oldtex;
}
*/

/*
================
Draw_ConsoleBackground
================
*/
/*
void Draw_ConsoleBackground (int lines)
{
	char	ver[80];

	if (!gl_conalpha.value && lines < vid.height)
		goto end;

	if (lines == vid.height)
		Draw_Pic (0, lines - vid.height, conback);
	else
		Draw_AlphaPic (0, lines - vid.height, conback, bound(0, gl_conalpha.value, 1));

end:
	sprintf (ver, "Tremor %s", JOEQUAKE_VERSION);
	Draw_Alt_String (vid.conwidth - strlen(ver) * 8 - 8, lines - 10, ver);
}
*/

void Draw_ConsoleBackground (int lines)
{
	char ver[80];
	float alpha;

	if (lines == vid.height)
	{
		Draw_Pic (0, 0, &draw_conback);
	}
	else
	{
		alpha = gl_conalpha.value;

		if (!alpha && (lines < vid.height))
			goto end;

		if (alpha > 1) // Entar - 3/9/05 and 3/10/05
			alpha -= 1;

		alpha = bound(0, alpha, 1);

		if (alpha == 1)
			Draw_Pic (0, lines - vid.height, &draw_conback);
		else
			Draw_AlphaPic (0, lines - vid.height, &draw_conback, alpha);
	}

/*	if (gl_conalpha.value > 1)
	{
		Draw_AlphaPic (0, lines - (vid.height - 5), conback, gl_conalpha.value - 1);
		//Draw_AlphaPic (0, lines - (vid.height + 5), conback, gl_conalpha.value - 1);
	}
*/
end:
	sprintf (ver, "reQuiem %s", REQUIEM_VERSION);
	Draw_Alt_String (vid.conwidth - (strlen(ver) + 1) * DRAW_CHARWIDTH, lines - DRAW_CHARHEIGHT - 2, ver);
}

void Draw_ConbackSolid (void)
{
	float wasalpha;

	if (scr_conlines)
	{
		wasalpha = gl_conalpha.value;
		if (scr_con_current != vid.height)
			gl_conalpha.value = 1;
		Draw_ConsoleBackground (scr_con_current);
		S_ExtraUpdate ();
		gl_conalpha.value = wasalpha;
	}
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h)
{
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		glColor3f (1,1,1);
#endif

	if (!draw_backtile)
		return;

	GL_Bind (draw_backtile->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (x/64.0, y/64.0);
	glVertex2f (x, y);
	glTexCoord2f ((x+w)/64.0, y/64.0);
	glVertex2f (x+w, y);
	glTexCoord2f ((x+w)/64.0, (y+h)/64.0);
	glVertex2f (x+w, y+h);
	glTexCoord2f (x/64.0, (y+h)/64.0);
	glVertex2f (x, y+h);
	glEnd ();
}


/*
=============
Draw_FillRGB

Fills a box of pixels with a single color
=============
*/
void Draw_FillRGB (int x, int y, int w, int h, const byte rgb[3])
{
	glDisable (GL_TEXTURE_2D);
	glColor3f (rgb[0] / 255.0, rgb[1] / 255.0, rgb[2] / 255.0);

	glBegin (GL_QUADS);

	glVertex2f (x, y);
	glVertex2f (x+w, y);
	glVertex2f (x+w, y+h);
	glVertex2f (x, y+h);

	glEnd ();
	glColor3f (1, 1, 1);
	glEnable (GL_TEXTURE_2D);
}

/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	byte rgb[3];

	rgb[0] = host_basepal[c*3];
	rgb[1] = host_basepal[c*3+1];
	rgb[2] = host_basepal[c*3+2];

	Draw_FillRGB (x, y, w, h, rgb);
}

//=============================================================================

/*
================
Draw_DimScreen
================
*/
void Draw_DimScreen (void)
{
	glEnable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
//	glColor4f (0, 0, 0, 0.7f);
	glColor4f (0, 0, 0, 0.82f);
	glBegin (GL_QUADS);

	glVertex2f (0,0);
	glVertex2f (vid.width, 0);
	glVertex2f (vid.width, vid.height);
	glVertex2f (0, vid.height);

	glEnd ();
	glColor4f (1, 1, 1, 1);
	glEnable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);

	Sbar_Changed ();
}

//=============================================================================

/*
================
Draw_BeginDisc

Draws the little blue disc in the corner of the screen.
Call before beginning any disc IO.
================
*/
void Draw_BeginDisc (void)
{
	mpic_t *disc;

#ifdef HEXEN2_SUPPORT
	static int index = 0;

	if (loading_stage) return;

	disc = draw_disc_H2[index++];
	if (index >= MAX_DISC) index = 0;

	if (!disc)
#endif
	disc = draw_disc;

	if (!disc)
		return;
	glDrawBuffer  (GL_FRONT);
	Draw_Pic (vid.width - 24, 0,disc);
	glDrawBuffer  (GL_BACK);
}


/*
================
Draw_EndDisc

Erases the disc icon.
Call after completing any disc IO
================
*/
void Draw_EndDisc (void)
{
}

/*
================
GL_Set2D

Setup as if the screen was 320*200
================
*/
void GL_Set2D (void)
{
	glViewport (glx, gly, glwidth, glheight);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, vid.width, vid.height, 0, -99999, 99999);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glDisable (GL_BLEND);
	glEnable (GL_ALPHA_TEST);

	glColor3f (1, 1, 1);
}

//====================================================================
#endif		// #ifndef RQM_SV_ONLY

