
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
// gl_texture.c -- texture loading procs originally in gl_draw.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY


typedef struct gltexture_s
{
	int			texnum;
	char		identifier[MAX_QPATH];
	char		*pathname;
	int			width, height;
	int			scaled_width, scaled_height;
	int			flags;
	unsigned 	crc;
	int			bpp;
	unsigned	average_color;		// JDH: calculated only for turbulent textures
	byte		*data8;			// JDH: for non-fullbright bmodel textures only
} gltexture_t;



gltexture_t	gltextures[MAX_GLTEXTURES];

int			numgltextures;
int			currenttexture = -1;		// to avoid unnecessary texture sets
qboolean	no24bit;


static	int	gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
static	int	gl_filter_max = GL_LINEAR;

int		gl_max_size_default;

const glmode_t gl_texmodes[GL_NUM_TEXMODES] =
{
	{"GL_NEAREST",                GL_NEAREST, GL_NEAREST},			// point-sampled, no mips
	{"GL_LINEAR",                 GL_LINEAR, GL_LINEAR},			// linear, no mips
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},	// point-sampled w. mips
	{"GL_LINEAR_MIPMAP_NEAREST",  GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},		// linear w. mips ("bilinear")
	{"GL_NEAREST_MIPMAP_LINEAR",  GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},	// point-sampled w. linear mips
	{"GL_LINEAR_MIPMAP_LINEAR",   GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}		// linear w. linear mips ("trilinear")
};

// some cards have low quality of alpha pics, so load the pics
// without transparent pixels into a different scrap block.
// scrap 0 is solid pics, 1 is transparent
#define	MAX_SCRAPS	2
int	scrap_texnum;

cvar_t	gl_picmip = {"gl_picmip", "0", CVAR_FLAG_ARCHIVE};
cvar_t	gl_picmip_all = {"gl_picmip_all", "1", CVAR_FLAG_ARCHIVE};

qboolean OnChange_gl_max_size (cvar_t *var, const char *string);
cvar_t	gl_max_size = {"gl_max_size", "1024", CVAR_FLAG_ARCHIVE, OnChange_gl_max_size};

qboolean OnChange_gl_texturemode (cvar_t *var, const char *string);
cvar_t	gl_texturemode = {"gl_texturemode", "GL_LINEAR_MIPMAP_NEAREST", CVAR_FLAG_ARCHIVE, OnChange_gl_texturemode};

qboolean OnChange_gl_texbrighten (cvar_t *var, const char *string);
cvar_t	gl_texbrighten = {"gl_texbrighten", "0", CVAR_FLAG_ARCHIVE, OnChange_gl_texbrighten};

extern	unsigned d_8to24table2[256];
extern	byte	vid_gamma_table[256];
extern	float	vid_gamma;

void Scrap_Init (void);
void GL_Upload8 (gltexture_t *glt, byte *data, int width, int height, int texflags);

/*
===============
GL_TextureInit
===============
*/
void GL_TextureInit (void)
{
	Cvar_RegisterInt (&gl_picmip, 0, 4);
	Cvar_RegisterBool (&gl_picmip_all);
	Cvar_RegisterInt (&gl_max_size, 32, 32768);
	Cvar_RegisterString (&gl_texturemode);
	Cvar_RegisterBool (&gl_texbrighten);

	no24bit = COM_CheckParm("-no24bit") ? true : false;

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &gl_max_size_default);
	Cvar_SetValueDirect (&gl_max_size, gl_max_size_default);

	// 3dfx can only handle 256 wide textures  ** FIXME: what about Voodooo 4/5 ?? **
	if (!Q_strncasecmp(gl_renderer, "3dfx", 4) || strstr(gl_renderer, "Glide"))
		Cvar_SetDirect (&gl_max_size, "256");

	Scrap_Init ();
}

/*
===============
GL_Bind
===============
*/
void GL_Bind (int texnum)
{
	if (currenttexture == texnum)
		return;

	currenttexture = texnum;

	if (!isDedicated)	// JDH
		glBindTexture (GL_TEXTURE_2D, texnum);
}


/*
===============
OnChange_gl_max_size
===============
*/
qboolean OnChange_gl_max_size (cvar_t *var, const char *string)
{
	int	i;
	int	newval = (int) Q_atof(string);

	if (newval > gl_max_size_default)
	{
		Con_Printf ("Your hardware doesn't support texture sizes bigger than %dx%d\n", gl_max_size_default, gl_max_size_default);
		return true;
	}

	for (i = 1 ; i < newval ; i <<= 1)
		;

	if (i != newval)
	{
		Con_Printf ("Valid values for %s are powers of 2 only\n", var->name);
		return true;
	}

	return false;
}

/*
===============
OnChange_gl_texturemode
===============
*/
qboolean OnChange_gl_texturemode (cvar_t *var, const char *string)
{
	int		i;
	gltexture_t	*glt;

	for (i=0 ; i<GL_NUM_TEXMODES ; i++)
	{
		if (!Q_strcasecmp (gl_texmodes[i].name, string))
			break;
	}

	if (i == GL_NUM_TEXMODES)
	{
		Con_Printf ("unknown texture mode: %s\n", string);
		return true;
	}

	gl_filter_min = gl_texmodes[i].minimize;
	gl_filter_max = gl_texmodes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->flags & TEX_NOFILTER)
			continue;

		GL_Bind (glt->texnum);
		if (glt->flags & TEX_MIPMAP)
		{
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		}
		else
		{
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		}
	}

	for (i = 0; i < MAX_SCRAPS; i++)
	{
		GL_Bind (scrap_texnum+i);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	return false;		// allow change
}

/*
===============
OnChange_gl_texbrighten
===============
*/
qboolean OnChange_gl_texbrighten (cvar_t *var, const char *string)
{
	int		i;
	gltexture_t	*glt;

	gl_texbrighten.value = atof (string);

	// reload all the existing bmodel textures using the new palette
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if ((glt->flags & TEX_BMODEL) && glt->data8)
		{
			GL_Bind (glt->texnum);
			GL_Upload8 (glt, glt->data8, glt->width, glt->height, glt->flags);
		}
	}

	return false;		// allow change
}

unsigned GL_GetTextureColor (int texnum)
{
	int i;
	gltexture_t	*glt;

	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->texnum == texnum)
			return glt->average_color;
	}

	return 0;
}

/*
=============================================================================

  scrap allocation

  Allocate all the little status bar objects into a single texture
  to crutch up stupid hardware / drivers

=============================================================================
*/

#define	BLOCK_WIDTH	256
#define	BLOCK_HEIGHT	256

int	scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte	scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT*4];
int	scrap_dirty = 0;	// bit mask


void Scrap_Init (void)
{
	// save slots for scraps
	scrap_texnum = texture_extension_number;
	texture_extension_number += MAX_SCRAPS;
}

// returns false if allocation failed
qboolean Scrap_AllocBlock (int scrapnum, int w, int h, int *x, int *y)
{
	int	i, j, best, best2;

	best = BLOCK_HEIGHT;

	for (i=0 ; i < BLOCK_WIDTH - w ; i++)
	{
		best2 = 0;

		for (j=0 ; j<w ; j++)
		{
			if (scrap_allocated[scrapnum][i+j] >= best)
				break;
			if (scrap_allocated[scrapnum][i+j] > best2)
				best2 = scrap_allocated[scrapnum][i+j];
		}
		if (j == w)
		{	// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if (best + h > BLOCK_HEIGHT)
		return false;

	for (i=0 ; i<w ; i++)
		scrap_allocated[scrapnum][*x+i] = best + h;

	scrap_dirty |= (1 << scrapnum);

	return true;
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	int	i;

	scrap_uploads++;
	for (i=0 ; i<MAX_SCRAPS ; i++)
	{
		if (!(scrap_dirty & (1 << i)))
			continue;

		scrap_dirty &= ~(1 << i);
		GL_Bind (scrap_texnum + i);
		GL_Upload8 (NULL, scrap_texels[i], BLOCK_WIDTH, BLOCK_HEIGHT, TEX_ALPHA);
	}
}

/*
=============================================================================
*/

/*
===============
GL_PadTexture8
===============
*/
byte * GL_PadTexture8 (const byte *data, int width, int height, int glwidth, int glheight)
{
	byte *data_out, *dest;
	const byte *src;
	int i;

	data_out = Q_malloc (glwidth * glheight);

	src = data;
	dest = data_out;
	for (i = 0 ; i < height ; i++)
	{
		memcpy (dest, src, width);
		memset (dest+width, dest[width-1], glwidth - width);		// repeat last pixel of each row
		src += width;
		dest += glwidth;
	}
	for ( ; i < glheight; i++)
	{
		memcpy (dest, dest-glwidth, glwidth);		// repeat last row
		dest += glwidth;
	}

// if there are at least 2 extra rows, make the last one a duplicate of the very first row
// (prevents some artifacts caused by bilinear filtering)
	if (glheight - height > 1)
		memcpy (data_out + (glheight-1)*glwidth, data_out, glwidth);

// same for last column
	if (glwidth - width > 1)
	{
		for (i = 0; i < glheight; i++)
			data_out[(i+1)*glwidth - 1] = data_out[i*glwidth];
	}

	return data_out;
}

static const char *wad_pathlist[3] = {"textures/wad/", "gfx/", NULL};

#define PADDEDSCRAP

/*
================
Draw_PicFromWad
================
*/
mpic_t *Draw_PicFromWad (const char *name)
{
	qpic_t		*p;
	mpic_t		*pic, *pic_24bit;
	int			glwidth, glheight;
	const char	*namelist[2] = {name, NULL};

	p = W_GetLumpByName (name, NULL);
	if (!p)
		return NULL;

	pic = (mpic_t *)p;

	if ((pic_24bit = GL_LoadPicImage_MultiSource (wad_pathlist, namelist, name, TEX_ALPHA, 0)))

//	if ((pic_24bit = GL_LoadPicImage("textures/wad/", name, name, TEX_ALPHA, 0)) ||
//	    (pic_24bit = GL_LoadPicImage("gfx/", name, name, TEX_ALPHA, 0)))
	{
		//memcpy (&pic->texnum, &pic_24bit->texnum, sizeof(mpic_t) - 8);
		pic_24bit->width = pic->width;
		pic_24bit->height = pic->height;
		*pic = *pic_24bit;
		return pic;
	}

#ifdef PADDEDSCRAP
	for (glwidth = 1 ; glwidth < p->width ; glwidth <<= 1)
		;
	for (glheight = 1 ; glheight < p->height ; glheight <<= 1)
		;

	// load little ones into the scrap
	if (glwidth < 64 && glheight < 64)
	{
		int	x, y, i, j, k, texnum;
		byte *buf, *data;

		if ((p->width == glwidth) && (p->height == glheight))
		{
			buf = NULL;
			data = p->data;
		}
		else
		{
			buf = GL_PadTexture8 (p->data, p->width, p->height, glwidth, glheight);
			data = buf;
		}

		texnum = memchr (p->data, 255, p->width*p->height) != NULL;
		if (Scrap_AllocBlock (texnum, glwidth, glheight, &x, &y))
		{
			k = 0;
			for (i=0 ; i<glheight ; i++)
				for (j=0 ; j<glwidth ; j++, k++)
					scrap_texels[texnum][(y+i)*BLOCK_WIDTH+x+j] = data[k];
			texnum += scrap_texnum;
			pic->texnum = texnum;
			pic->sl = x / (float)BLOCK_WIDTH;
			pic->sh = (x + p->width) / (float)BLOCK_WIDTH;
			pic->tl = y / (float)BLOCK_HEIGHT;
			pic->th = (y + p->height) / (float)BLOCK_HEIGHT;
		}
		else
		{
			GL_LoadPicTexture (name, pic, p->data, TEX_ALPHA, 1);
		}

		if (buf)
			free (buf);

//		pic_count++;
//		pic_texels += p->width * p->height;
	}
#else
		// load little ones into the scrap
	if (p->width < 64 && p->height < 64)
	{
		int	x, y, i, j, k, texnum;

		texnum = memchr (p->data, 255, p->width*p->height) != NULL;
		if (!Scrap_AllocBlock (texnum, p->width, p->height, &x, &y))
		{
			GL_LoadPicTexture (name, pic, p->data, TEX_ALPHA, 1);
			return pic;
		}
		k = 0;
		for (i=0 ; i<p->height ; i++)
			for (j=0 ; j<p->width ; j++, k++)
				scrap_texels[texnum][(y+i)*BLOCK_WIDTH+x+j] = p->data[k];
		texnum += scrap_texnum;
		pic->texnum = texnum;
		pic->sl = (x + 0.01) / (float)BLOCK_WIDTH;
		pic->sh = (x + p->width - 0.01) / (float)BLOCK_WIDTH;
		pic->tl = (y + 0.01) / (float)BLOCK_HEIGHT;
		pic->th = (y + p->height - 0.01) / (float)BLOCK_HEIGHT;

//		pic_count++;
//		pic_texels += p->width * p->height;
	}
#endif
	else
	{
		GL_LoadPicTexture (name, pic, p->data, TEX_ALPHA, 1);
	}

	return pic;
}

/*
================
ResampleTextureLerpLine
================
*/
void ResampleTextureLerpLine (byte *in, byte *out, int inwidth, int outwidth)
{
	int xi, oldx = 0, f, fstep, endx, lerp;

	fstep = (inwidth << 16) / outwidth;
	endx = (inwidth - 1);

/*****JDH*****/
	//for (j = 0, f = 0 ; j < outwidth ; j++, f += fstep)
	for (f = 0 ; ; f += fstep)
/*****JDH*****/
	{
		xi = f >> 16;
		if (xi != oldx)
		{
			in += ((xi - oldx) << 2);
			oldx = xi;
		}
		if (xi < endx)
		{
			lerp = f & 0xFFFF;

			*out++ = (byte)((((in[4] - in[0]) * lerp) >> 16) + in[0]);
			*out++ = (byte)((((in[5] - in[1]) * lerp) >> 16) + in[1]);
			*out++ = (byte)((((in[6] - in[2]) * lerp) >> 16) + in[2]);
			*out++ = (byte)((((in[7] - in[3]) * lerp) >> 16) + in[3]);
		}
		else	// last pixel of the line has no pixel to lerp to
		{
		/*****JDH*****/
			//*out++ = in[0];
			//*out++ = in[1];
			//*out++ = in[2];
			//*out++ = in[3];

			*(int *) out = *(int *) in;
			break;
		/*****JDH*****/
		}
	}
}

/*
================
ResampleTexture
================
*/
void ResampleTexture (const unsigned *indata, int inwidth, int inheight, unsigned *outdata, int outwidth, int outheight)
{
	int	i, j, yi, oldy, f, fstep, outrowwidth, inrowwidth, lerp, endy = (inheight-1);
	byte	*inrow, *out;
	byte	*row1, *row2;

	out = (byte *)outdata;
	fstep = (inheight << 16) / outheight;
	inrowwidth = inwidth << 2;
	outrowwidth = outwidth << 2;		// 4 bytes per pixel

	row1 = Q_malloc( outrowwidth << 1 );		// allocate for 2 rows
	row2 = row1 + outrowwidth;

	inrow = (byte *)indata;
	oldy = 0;

	ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
	ResampleTextureLerpLine (inrow + inrowwidth, row2, inwidth, outwidth);

	for (i = 0, f = 0 ; i < outheight ; i++, f += fstep)
	{
		yi = f >> 16;
		if (yi < endy)
		{
			lerp = f & 0xFFFF;

			if (yi != oldy)
			{
				inrow = (byte *)indata + inrowwidth * yi;
				if (yi == oldy+1)
					memcpy (row1, row2, outrowwidth);
				else
					ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
				ResampleTextureLerpLine (inrow + inrowwidth, row2, inwidth, outwidth);
				oldy = yi;
			}

			for (j = outwidth ; j ; j--)
			{
				out[0] = (byte)((((row2[0] - row1[0]) * lerp) >> 16) + row1[0]);
				out[1] = (byte)((((row2[1] - row1[1]) * lerp) >> 16) + row1[1]);
				out[2] = (byte)((((row2[2] - row1[2]) * lerp) >> 16) + row1[2]);
				out[3] = (byte)((((row2[3] - row1[3]) * lerp) >> 16) + row1[3]);
				out += 4;
				row1 += 4;
				row2 += 4;
			}
			row1 -= outrowwidth;
			row2 -= outrowwidth;
		}
		else
		{
			if (yi != oldy)
			{
				inrow = (byte *)indata + inrowwidth * yi;
				if (yi == oldy+1)
					memcpy (row1, row2, outrowwidth);
				else
					ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
				oldy = yi;
			}
			memcpy (out, row1, outrowwidth);
		}
	}

	free (row1);
}

/*
================
MipMap

Operates in place, quartering the size of the texture
================
*/
void MipMap (byte *in, int *width, int *height)
{
	int	i, j;
	byte	*out;

/*****JDH*****/
	//int nextrow = *width << 2;
	int rowsize;
	byte *nextrow;

	rowsize = *width << 2;
	nextrow = in + rowsize;
/*****JDH*****/

	out = in;
	if (*width > 1)
	{
		*width >>= 1;
		if (*height > 1)
		{
			*height >>= 1;
		/*****JDH*****/
			/*for (i = *height ; i ; i--, in += nextrow)
			{
				for (j = *width ; j  ; j--, in += 8)
				{
					*out++ = (in[0] + in[4] + in[nextrow+0] + in[nextrow+4]) >> 2;
					*out++ = (in[1] + in[5] + in[nextrow+1] + in[nextrow+5]) >> 2;
					*out++ = (in[2] + in[6] + in[nextrow+2] + in[nextrow+6]) >> 2;
					*out++ = (in[3] + in[7] + in[nextrow+3] + in[nextrow+7]) >> 2;
				}
			}*/

			for (i = *height ; i ; i--, in += rowsize, nextrow += rowsize)
			{
				for (j = *width ; j  ; j--, in += 8, nextrow += 8)
				{
					*out++ = (in[0] + in[4] + nextrow[0] + nextrow[4]) >> 2;
					*out++ = (in[1] + in[5] + nextrow[1] + nextrow[5]) >> 2;
					*out++ = (in[2] + in[6] + nextrow[2] + nextrow[6]) >> 2;
					*out++ = (in[3] + in[7] + nextrow[3] + nextrow[7]) >> 2;
				}
			}
		/*****JDH*****/
		}
		else
		{
			for (i = *height ; i ; i--)
			{
				for (j = *width ; j ; j--, in += 8)
				{
					*out++ = (in[0] + in[4]) >> 1;
					*out++ = (in[1] + in[5]) >> 1;
					*out++ = (in[2] + in[6]) >> 1;
					*out++ = (in[3] + in[7]) >> 1;
				}
			}
		}
	}
	else if (*height > 1)
	{
		*height >>= 1;
	/*****JDH*****/
		/*for (i = *height ; i ; i--, in += nextrow)
		{
			for (j = *width ; j ; j--, in += 4)
			{
				*out++ = (in[0] + in[nextrow+0]) >> 1;
				*out++ = (in[1] + in[nextrow+1]) >> 1;
				*out++ = (in[2] + in[nextrow+2]) >> 1;
				*out++ = (in[3] + in[nextrow+3]) >> 1;
			}
		}*/

		for (i = *height ; i ; i--, in += rowsize, nextrow += rowsize)
		{
			for (j = *width ; j ; j--, in += 4, nextrow += 4)
			{
				*out++ = (in[0] + nextrow[0]) >> 1;
				*out++ = (in[1] + nextrow[1]) >> 1;
				*out++ = (in[2] + nextrow[2]) >> 1;
				*out++ = (in[3] + nextrow[3]) >> 1;
			}
		}
	/*****JDH*****/
	}
}

/*
===============
GL_ReduceDimensions
===============
*/
void GL_ReduceDimensions (int *width, int *height, int texflags)
{
	int maxsize;

	if (texflags & TEX_PLAYER)
	{
		*width >>= max(0, (int)gl_playermip.value);		// JDH - changed 1 to 0
		*height >>= max(0, (int)gl_playermip.value);
	}
	else if (texflags & TEX_MIPMAP)
	{
		*width >>= max(0, (int) gl_picmip.value);
		*height >>= max(0, (int) gl_picmip.value);
	}

	maxsize = (texflags & TEX_MIPMAP) ? gl_max_size.value : gl_max_size_default;

	*width = bound(1, *width, maxsize);
	*height = bound(1, *height, maxsize);
}

/*
===============
GL_ScaleDimensions
===============
*/
void GL_ScaleDimensions (int width, int height, int *scaled_width, int *scaled_height, int texflags)
{
	for (*scaled_width = 1 ; *scaled_width < width ; *scaled_width <<= 1)
		;
	for (*scaled_height = 1 ; *scaled_height < height ; *scaled_height <<= 1)
		;

	GL_ReduceDimensions (scaled_width, scaled_height, texflags);
}

/*
===============
GL_PadTexture32
===============
*/
void GL_PadTexture32 (const unsigned *dataIn, int width, int height, unsigned *dataOut,
					  int scaled_width, int scaled_height)
{
	const unsigned	*src;
	unsigned	*dest, pad;
	int			x, y;

	src = dataIn;
	dest = dataOut;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
			*dest++ = *src++;

		// fill in remainder of row with last pixel's color (for mipping)
		pad = *(dest-1);
		for (; x < scaled_width; x++)
			*dest++ = pad;
	}

	// fill in remaining rows with a copy of the last row:
	src = dataOut + (height-1)*scaled_width;		// last row of scaled image
	for (; y < scaled_height; y++)
	{
		for (x = 0; x < scaled_width; x++)
			*dest++ = src[x];
	}
}

/*
===============
GL_Upload32
  assumes current texture is already bound
===============
*/
void GL_Upload32 (gltexture_t *glt, const unsigned *data, int *width, int *height, int texflags)
{
	int		internal_format, scaled_width, scaled_height, miplevel, texval;
	unsigned int	*scaled;		// JDH: was static array

#ifdef WAITFORTEXPTR		// see corresponding code & notes in R_MarkLights
	if (glt /*&& (glt->identifier[0] == '*')*/)
#else
	if (glt && (glt->identifier[0] == '*'))
#endif
	{
	// JDH: calculate average color (for waterfog)
		unsigned r = 0, g = 0, b = 0;
		int i, size = (*width) * (*height);

		for (i = 0; i < size; i++)
		{
			r += (data[i] & 0x000000FF);
			g += (data[i] & 0x0000FF00) >> 8;
			b += (data[i] & 0x00FF0000) >> 16;
		}

		r /= size;
		g /= size;
		b /= size;
		glt->average_color = (r | (g << 8) | (b << 16));
	}

	for (scaled_width = 1 ; scaled_width < *width ; scaled_width <<= 1)
		;
	for (scaled_height = 1 ; scaled_height < *height ; scaled_height <<= 1)
		;

	//if (scaled_width * scaled_height * 4 > sizeof(scaled))
	//	Sys_Error ("GL_LoadTexture: too big");

	if (*width < scaled_width || *height < scaled_height)
	{
		scaled = Q_malloc (scaled_width * scaled_height * 4);

		if ((texflags & TEX_NOSTRETCH) && (*width <= scaled_width) && (*height <= scaled_height))
		{
			GL_PadTexture32 (data, *width, *height, scaled, scaled_width, scaled_height);
		}
		else ResampleTexture (data, *width, *height, scaled, scaled_width, scaled_height);

		*width = scaled_width;
		*height = scaled_height;

		data = scaled;
	}
	else
	{
		scaled = NULL;
	}


	GL_ReduceDimensions (&scaled_width, &scaled_height, texflags);

	while (*width > scaled_width || *height > scaled_height)
		MipMap ((byte *) data, width, height);

	internal_format = (texflags & TEX_ALPHA) ? gl_alpha_format : gl_solid_format;
	glTexImage2D (GL_TEXTURE_2D, 0, internal_format, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (texflags & TEX_MIPMAP)
	{
		miplevel = 0;

		while (*width > 1 || *height > 1)
		{
			miplevel++;

			MipMap ((byte *) data, width, height);
			glTexImage2D (GL_TEXTURE_2D, miplevel, internal_format, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		texval = gl_filter_min;
	}
	else
	{
		texval = gl_filter_max;
	}

	if (!(texflags & TEX_NOFILTER))
	{
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texval);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	texval = (texflags & TEX_NOTILE) ? GL_CLAMP : GL_REPEAT;		// JDH
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texval);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texval);

	if (scaled)
		free (scaled);
}

#ifdef HEXEN2_SUPPORT

extern int ColorIndex[16];
extern unsigned ColorPercent[16];
/*
===============
GL_TranslateAlpha_H2
===============
*/
void GL_TranslateAlpha_H2 (const byte *data, int width, int height, int *texflags, unsigned *trans)
{
	int size, i, p;

	size = width * height;

	// JDH: this routine was very buggy (it accesses memory outside of image range,
	//	which makes it difficult to produce the same output as it does in Hexen II).
	//  My attempt to fix it is in the #if 0 blocks, but it's hard to tell exactly
	//  what they were trying to accomplish (some sort of edge smoothing, I think).  
	//  So I just removed it all.

/*	for (i=0 ; i<size ; i++)
	{
		int n;
		int r = 0, g = 0, b = 0;

		p = data[i];
		if (p == 255)
		{
#if 0
			unsigned long neighbors[8];			//JDH
#else
			unsigned long neighbors[9];
#endif
			int num_neighbors_valid = 0;
			int neighbor_u, neighbor_v;

			int u, v;
#if 0
			u = i % width;		//JDH
			v = i / width;		//JDH
#else
			u = size % width;
			v = size / width;
#endif

#if 0
			for (neighbor_u = (u ? u-1 : 0); neighbor_u <= u + 1; neighbor_u++)
			{
				for (neighbor_v = (v ? v-1 : 0); neighbor_v <= v + 1; neighbor_v++)
				{
					if (neighbor_u >= width || neighbor_v >= height)
						continue;
#else
			for (neighbor_u = u - 1; neighbor_u <= u + 1; neighbor_u++)
			{
				for (neighbor_v = v - 1; neighbor_v <= v + 1; neighbor_v++)
				{
					if (neighbor_u == neighbor_v)
						continue;
					// Make sure  that we are accessing a texel in the image, not out of range.
					if (neighbor_u < 0 || neighbor_u > width || neighbor_v < 0 || neighbor_v > height)
						continue;
#endif
//					assert(neighbor_u + neighbor_v*width < size);

					if (data[neighbor_u + neighbor_v * width] == 255)
						continue;

					assert(num_neighbors_valid < sizeof(neighbors)/4);

					neighbors[num_neighbors_valid++] = trans[neighbor_u + neighbor_v * width];
				}
			}

			if (num_neighbors_valid == 0)
				continue;

			for (n = 0; n < num_neighbors_valid; n++)
			{
				r += neighbors[n] & 0xff;
				g += (neighbors[n] & 0xff00) >> 8;
				b += (neighbors[n] & 0xff0000) >> 16;
			}

			r /= num_neighbors_valid;
			g /= num_neighbors_valid;
			b /= num_neighbors_valid;

			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;

			trans[i] = (b << 16) | (g << 8) | r;
		}
	}
*/

	if (*texflags & TEX_ALPHA_MODE1)
	{
		*texflags |= TEX_ALPHA;
		for (i=0 ; i<size ; i++)
		{
			p = data[i];
			if (p == 0)
				trans[i] &= 0x00ffffff;
			else if (p & 1)
			{
				trans[i] &= 0x00ffffff;
				trans[i] |= ((int)(255 * r_wateralpha.value)) << 24;
			}
			else
			{
				trans[i] |= 0xff000000;
			}
		}
	}
	else if (*texflags & TEX_ALPHA_MODE2)
	{
		*texflags |= TEX_ALPHA;
		for (i=0 ; i<size ; i++)
		{
			p = data[i];
			if (p == 0)
				trans[i] &= 0x00ffffff;
		}
	}
	else if (*texflags & TEX_ALPHA_MODE3)
	{
		*texflags |= TEX_ALPHA;
		for (i=0 ; i<size ; i++)
		{
			p = data[i];
			trans[i] = d_8to24table[ColorIndex[p>>4]] & 0x00ffffff;
			trans[i] |= (int) ColorPercent[p&15] << 24;
		}
	}
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
================
GL_SmudgeEdges
  JDH: for alpha textures, replaces transparent pixels next to non-transparent ones
       with an appropriate color (fixes fringes when texture is filtered, eg. bilinear)
================
*/
/*
void GL_SmudgeEdges (unsigned *data, int width, int height)
{
	int y, x, prevcol, nextcol, r, g, b, count, i;
	unsigned *curr, *row_prev, *row_curr, *row_next, neighbors[8];

	curr = data;
	
	for (y = 0; y < height; y++)
	{
		row_prev = data + ((y+height-1) % height) * width;
		row_curr = curr;
		row_next = data + ((y+1) % height) * width;

		for (x = 0; x < width; x++, curr++)
		{
			if ((*curr >> 24) == 0xFF)
				continue;		// leave solid pixels alone

			prevcol = (x+width-1) % width;
			nextcol = (x+1) % width;

			neighbors[0] = row_prev[prevcol];
			neighbors[1] = row_prev[x];
			neighbors[2] = row_prev[nextcol];
			neighbors[3] = row_curr[prevcol];
			neighbors[4] = row_curr[nextcol];
			neighbors[5] = row_next[prevcol];
			neighbors[6] = row_next[x];
			neighbors[7] = row_next[nextcol];

			r = g = b = 0;
			count = 0;
			
			for (i = 0; i < 8; i++)
			{
				if (neighbors[i] >> 24)
				{
					r += neighbors[i] & 0xFF;
					g += (neighbors[i] >> 8) & 0xFF;
					b += (neighbors[i] >> 16) & 0xFF;
					count++;
				}
			}

			if (count)
			{
				r /= count;
				g /= count;
				b /= count;
				*curr = (*curr & 0xFF000000) + r + (g<<8) + (b<<16);
			}
		}
	}
}
*/
/*
================
GL_FindTexture
================
*/
gltexture_t *GL_FindTexture (const char *identifier)
{
	int	i;

	if (identifier[0])
	{
		for (i=0 ; i<numgltextures ; i++)
		{
			if (!strcmp(identifier, gltextures[i].identifier))
				return &gltextures[i];
		}
	}

	return NULL;
}

/*
===============
GL_Upload8
===============
*/
void GL_Upload8 (gltexture_t *glt, byte *data, int width, int height, int texflags)
{
	unsigned	*trans;		// JDH: dynamically allocated now
	int			i, size, p;
	const unsigned	*table;

//	table = (texflags & TEX_BRIGHTEN) ? d_8to24table2 : d_8to24table;
	if ((texflags & TEX_BMODEL) && gl_texbrighten.value)
		table = d_8to24table2;
	else
		table = d_8to24table;

	size = width * height;
	trans = Q_malloc (size*4);

	if (texflags & TEX_FULLBRIGHT)
	{
	// this is a fullbright mask, so make all non-fullbright
	// colors transparent
		texflags |= TEX_ALPHA;
		for (i=0 ; i<size ; i++)
		{
			p = data[i];
			if (p < 224)
				trans[i] = table[p] & 0x00FFFFFF;	// transparent
			else
			{
		#ifdef _DEBUG
				if (p != 245 && p != 246)
					table = d_8to24table;
#endif
				trans[i] = table[p];			// fullbright
			}
		}
	}
	else if ((texflags & TEX_ALPHA)
#ifdef HEXEN2_SUPPORT
		|| (texflags & (TEX_ALPHA_MODE1 | TEX_ALPHA_MODE2 | TEX_ALPHA_MODE3))
#endif
		)
	{
	// if there are no transparent pixels, make it a 3 component
	// texture even if it was specified as otherwise
		texflags &= ~TEX_ALPHA;
		for (i=0 ; i<size ; i++)
		{
			p = data[i];
			if (p == 255)
				texflags |= TEX_ALPHA;
			trans[i] = table[p];
		}

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			GL_TranslateAlpha_H2 (data, width, height, &texflags, trans);
		}
	#endif
	}
	else
	{
	//	if (size & 3)		// removed check 2010.02.06
	//		Sys_Error ("GL_Upload8: size & 3");

		for (i = 0; i < size; i++)
			trans[i] = table[data[i]];
	}

//	if (texflags & TEX_ALPHA)
//		GL_SmudgeEdges (trans, width, height);
	
	GL_Upload32 (glt, trans, &width, &height, texflags);

	free (trans);
}

/*
================
GL_LoadTexture
================
*/
int GL_LoadTexture (const char *identifier, int width, int height, void *data, int texflags, int bytesperpixel)
{
	int		scaled_width, scaled_height, crc = 0;
	gltexture_t	*glt;

	GL_ScaleDimensions (width, height, &scaled_width, &scaled_height, texflags);

	// see if the texture is already present
	if (identifier[0])
	{
		crc = CRC_Block (data, width * height * bytesperpixel);

		glt = GL_FindTexture (identifier);
		if (glt)
		{
			if ((width == glt->width) && (height == glt->height) &&
				(scaled_width == glt->scaled_width) && (scaled_height == glt->scaled_height) &&
				(crc == glt->crc) && (bytesperpixel == glt->bpp) &&
				(texflags & ~TEX_COMPLAIN) == (glt->flags & ~TEX_COMPLAIN))
			{
				GL_Bind (glt->texnum);
				return glt->texnum;		// texture cached
			}
			else
			{
				goto GL_LoadTexture_setup;	// reload the texture into the same slot
			}

		}
		/*for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
		{
			if (!strncmp(identifier, glt->identifier, sizeof(glt->identifier)-1))
			{
				if (width == glt->width && height == glt->height &&
					scaled_width == glt->scaled_width && scaled_height == glt->scaled_height &&
					crc == glt->crc && bpp == glt->bpp &&
					(texflags & ~TEX_COMPLAIN) == (glt->texmode & ~TEX_COMPLAIN))
				{
					GL_Bind (gltextures[i].texnum);
					return gltextures + i;	// texture cached
				}
				else
				{
					goto GL_LoadTexture_setup;	// reload the texture into the same slot
				}
			}
		}*/
	}


	if (numgltextures == MAX_GLTEXTURES)
		Sys_Error ("GL_LoadTexture: numgltextures == MAX_GLTEXTURES");

	glt = &gltextures[numgltextures];
	numgltextures++;

	Q_strcpy (glt->identifier, identifier, sizeof(glt->identifier));
	glt->texnum = texture_extension_number;
	texture_extension_number++;

GL_LoadTexture_setup:
	glt->width = width;
	glt->height = height;
	glt->scaled_width = scaled_width;
	glt->scaled_height = scaled_height;
	glt->flags = texflags;
	glt->crc = crc;
	glt->bpp = bytesperpixel;
	if (glt->pathname)
	{
		Z_Free (glt->pathname);
		glt->pathname = NULL;
	}

	if (bytesperpixel == 4 && com_netpath[0])
		glt->pathname = CopyString (com_netpath);

	if (glt->data8)
		free (glt->data8);

// JDH: save raw data so it can be reloaded if gl_texbrighten changes
	if ((texflags & TEX_BMODEL) && (bytesperpixel == 1))
	{
		glt->data8 = malloc (width*height);
		if (glt->data8)
			memcpy (glt->data8, data, width*height);
	}
	else glt->data8 = NULL;

	GL_Bind (glt->texnum);

	if (!isDedicated)		// JDH
	{
		switch (bytesperpixel)
		{
		case 1:
			GL_Upload8 (glt, data, width, height, texflags);
			break;

		case 4:
			GL_Upload32 (glt, (void *)data, &width, &height, texflags);
			break;

		default:
			Sys_Error ("GL_LoadTexture: unknown bpp\n");
			break;
		}
	}

	return glt->texnum;
}

#ifdef TEXLOADTEST
/*************************JDH*************************

struct TexInfo
{
	char identifier[ MAX_QPATH ];
	int width, height, bpp;
	int scaled_width, scaled_height;
	int flags;
	unsigned crc;
	int texnum, fb_texnum;
	qboolean hasfb;
};

gltexture_t * GL_LocateTexture( TexInfo *info, byte *data )
{
	gltexture_t *currtex;

	// see if the texture is already present

	currtex = GL_FindTexture( info->identifier );
	if ( currtex )
	{
		info->crc = CRC_Block( data, info->width * info->height * info->bpp );

		if ((currtex->width == info->width) && (currtex->height == info->height) &&
			(currtex->scaled_width == info->scaled_width) &&
			(currtex->scaled_height == info->scaled_height) &&
			(currtex->crc == info->crc) && (currtex->bpp == info->bpp) &&
			((currtex->flags & ~TEX_COMPLAIN) == (info->flags & ~TEX_COMPLAIN)))
		{
			goto ExitPoint;		// texture cached
		}
	}
	else
	{
		if (numgltextures == MAX_GLTEXTURES)
			Sys_Error ("GL_LoadTexture: numgltextures == MAX_GLTEXTURES");

		currtex = &gltextures[ numgltextures++ ];

		Q_strcpy( currtex->identifier, info->identifier, sizeof(currtex->identifier));
		currtex->texnum = texture_extension_number++;
	}

	currtex->width = info->width;
	currtex->height = info->height;
	currtex->scaled_width = info->scaled_width;
	currtex->scaled_height = info->scaled_height;
	currtex->flags = info->flags;
	currtex->crc = info->crc;
	currtex->bpp = info->bpp;
	if (currtex->pathname)
	{
		Z_Free (currtex->pathname);
		currtex->pathname = NULL;
	}

	if ((info->bpp == 4) && com_netpath[0])
		currtex->pathname = CopyString (com_netpath);

ExitPoint:
	GL_Bind (currtex->texnum);
	return currtex;
}
*/
// GL_LoadTextureAndFB - loads the texture, and if fullbright pixels are found,
//						  also loads the fullbright mask texture

/*int GL_LoadTextureAndFB( char *identifier, int width, int height, byte *data, int texflags)
{
	TexInfo info;
	gltexture_t *tex;

	Q_strcpy( info.identifier, identifier, sizeof(info.identifier) );
	info.width = width;
	info.height = height;
	info.bpp = 1;
	info.flags = texflags & ~TEX_FULLBRIGHT;
	info.crc = 0;
	info.texnum = 0;
	info.fb_texnum = 0;
	info.hasfb = false;

	GL_ScaleDimensions( width, height, &info.scaled_width, &info.scaled_height, texflags);

	tex = GL_LocateTexture( &info, data );
	GL_Upload8( data, tex );

	Q_strcpy( info.identifier, va( "@fb_%s", identifier ), sizeof(info.identifier) );
	info.flags = texflags & TEX_FULLBRIGHT;
	tex = GL_LocateTexture( &info, data );
	GL_Upload8( data, tex );

	return tex->texnum;
}*/
#endif
/*************************JDH*************************/

/*
================
GL_LoadPicTexture
	** requires pic->width and pic->height to be set **
================
*/
int GL_LoadPicTexture (const char *name, mpic_t *pic, void *data, int texflags, int bytesperpixel)
{
	int		glwidth, glheight;
	char	identifier[MAX_QPATH] = "pic:";
	byte	*buf;

	for (glwidth = 1 ; glwidth < pic->width ; glwidth <<= 1)
		;
	for (glheight = 1 ; glheight < pic->height ; glheight <<= 1)
		;

	Q_strcpy (identifier + 4, name, sizeof(identifier) - 4);

	if ((glwidth == pic->width) && (glheight == pic->height))
	{
	//	pic->texnum = GL_LoadTexture (identifier, glwidth, glheight, data, texflags, bytesperpixel);
		pic->sh = 1;
		pic->th = 1;
		buf = NULL;
	}
	else
	{
		if (bytesperpixel == 1)
			buf = GL_PadTexture8 (data, pic->width, pic->height, glwidth, glheight);
		else
		{
			byte *src, *dest;
			int i;

			buf = Q_malloc (glwidth * glheight * 4);

			src = data;
			dest = buf;
			for (i=0 ; i<pic->height ; i++)
			{
				memcpy (dest, src, pic->width * 4);
				src += pic->width * 4;
				dest += glwidth * 4;
			}
		}

// JDH: any reason why this was passing an empty name?
//		pic->texnum = GL_LoadTexture ("", glwidth, glheight, buf, texflags, 1);
		
		pic->sh = (float)pic->width / glwidth;
		pic->th = (float)pic->height / glheight;
		data = buf;
	}

	pic->texnum = GL_LoadTexture (identifier, glwidth, glheight, data, texflags, bytesperpixel);
	pic->sl = 0;
	pic->tl = 0;

	if (buf)
		free (buf);
	return pic->texnum;
}

/*
================
GL_UpdatePicTexture (JDH)
  - currently used only for player lmp in menu
================
*/
void GL_UpdatePicTexture (mpic_t *pic, const unsigned *data)
{
	int width = pic->width;
	int height = pic->height;

	GL_Bind (pic->texnum);
	GL_Upload32 (NULL, data, &width, &height, TEX_NOSTRETCH | TEX_ALPHA | TEX_NOTILE);

// recalc these in case texture prefs have changed (eg. gl_max_size, gl_picmip):
	pic->sh = (float)pic->width / width;
	pic->th = (float)pic->height / height;
}

static	gltexture_t	*current_texture = NULL;


/*
===============
CheckTextureLoaded
===============
*/
static qboolean CheckTextureLoaded (int texflags, FILE *f)
{
	int	scaled_width, scaled_height;

	if (current_texture && current_texture->pathname && COM_FilenamesEqual(com_netpath, current_texture->pathname))
	{
		GL_ScaleDimensions (current_texture->width, current_texture->height, &scaled_width, &scaled_height, texflags);
		if (current_texture->scaled_width == scaled_width && current_texture->scaled_height == scaled_height)
		{
			if (current_texture)
			{
				image_width = current_texture->width;
				image_height = current_texture->height;
				current_texture = NULL;
			}
			fclose (f);
			return true;
		}
	}

	return false;
}

/*
===============
GL_LoadImage_MultiSource
   JDH: item_dirlevel is the ID of the directory from which the original item
      (model/sprite/lmp/wad ) was loaded.  The external texture will be loaded
      only from a directory whose ID is higher (that is, one that appears
      earlier in the searchpath).  This should prevent custom low-res graphics
      being overridden by the generic hi-res ones.
===============
*/
byte *GL_LoadImage_MultiSource (const char *paths[], const char *filenames[], int texflags, int item_dirlevel)
{
	char			basenames[MAX_MULTISOURCE_NAMES][MAX_QPATH];
	const char		*filelist[MAX_MULTISOURCE_NAMES];
	int				i;
	char			*c;
	byte			*data;
	com_fileinfo_t	fileinfo;
	FILE			*f;

	for (i = 0; filenames[i]; i++)
	{
		COM_StripExtension (filenames[i], basenames[i], sizeof(basenames[i]));
		for (c = basenames[i] ; *c ; c++)
		{
			if (*c == '*')
				*c = '#';
		}

		filelist[i] = basenames[i];
	}

	filelist[i] = NULL;

	f = COM_FOpen_MultiSource (paths, filelist, item_dirlevel | FILE_ANY_IMG, &fileinfo);
	if (f)
	{
		if (CheckTextureLoaded (texflags, f))
			return NULL;

		data = Image_LoadFile (f, fileinfo.filelen, fileinfo.name);
		if (data)
		{
		//	c = (byte *) (com_filepath->pack ? com_filepath->pack->filename : com_filepath->filename);
		//	c += strlen (com_basedir) + 1;		// just the relative path
		//	Con_Printf ("Loaded texture \"%s/%s\"\n", c, fileinfo.filename);
			if (fileinfo.searchpath->pack)
				c = fileinfo.searchpath->pack->filename + strlen(com_basedir)+1;	// just gamedir+pak
			else
				c = (char *) fileinfo.searchpath->dir_name;
			Con_Printf ("Loaded %s from %s\n", COM_SkipPath (fileinfo.name), c);
			fclose (f);
			return data;
		}
		fclose (f);
	}

	return NULL;
}

/*
===============
GL_LoadImage
===============
*/
byte *GL_LoadImage (const char *path, const char *filename, int texflags, int item_dirlevel)
{
	const char	*pathlist[2] = {path, NULL};
	const char	*namelist[2] = {filename, NULL};
	byte	*data;

	data = GL_LoadImage_MultiSource (pathlist, namelist, texflags, item_dirlevel);

	if (!data && (texflags & TEX_COMPLAIN))
	{
		if (!no24bit)
			Con_Printf ("Couldn't load %s image\n", COM_SkipPath(filename));
	}

	return data;
}

/*
===============
GL_LoadTexturePixels
===============
*/
int GL_LoadTexturePixels (byte *data, const char *identifier, int width, int height, int texflags)
{
	int		i, j, image_size;
	qboolean	gamma;

	image_size = width * height;
	gamma = (vid_gamma != 1);

	if (texflags & TEX_LUMA)
	{
		gamma = false;
	}
	else if (texflags & TEX_ALPHA)
	{
		texflags &= ~TEX_ALPHA;
		for (j=0 ; j<image_size ; j++)
		{
			if (((((unsigned *)data)[j] >> 24) & 0xFF) < 255)
			{
				texflags |= TEX_ALPHA;
				break;
			}
		}
	}

	if (gamma)
	{
		for (i=0 ; i<image_size ; i++)
		{
			data[4*i] = vid_gamma_table[data[4*i]];
			data[4*i+1] = vid_gamma_table[data[4*i+1]];
			data[4*i+2] = vid_gamma_table[data[4*i+2]];
		}
	}

	return GL_LoadTexture (identifier, width, height, data, texflags, 4);
}

/*
===============
GL_LoadTextureImage_MultiSource
===============
*/
int GL_LoadTextureImage_MultiSource (const char *paths[], const char *filenames[],
								const char *identifier, int texflags, int item_dirlevel)
{
	int			texnum;
	byte		*data;
	gltexture_t	*gltexture;

// JDH: don't check no24bit here; MD3 and MD2 models must be allowed to load external textures

	if (!identifier)
		identifier = filenames[0];

	gltexture = current_texture = GL_FindTexture (identifier);

	if (!(data = GL_LoadImage_MultiSource (paths, filenames, texflags, item_dirlevel)))
	{
		texnum = (gltexture && !current_texture) ? gltexture->texnum : 0;
					// FIXME: what the hell is this about???
	}
	else
	{
		texnum = GL_LoadTexturePixels (data, identifier, image_width, image_height, texflags);
		free (data);
	}

	current_texture = NULL;
	return texnum;
}

/*
===============
GL_LoadTextureImage
===============
*/
int GL_LoadTextureImage (const char *path, const char *filename, const char *identifier,
						 int texflags, int item_dirlevel)
{
	const char *pathlist[2] = {path, NULL};
	const char *namelist[2] = {filename, NULL};

	return GL_LoadTextureImage_MultiSource (pathlist, namelist, identifier, texflags, item_dirlevel);
}

/*
===============
GL_LoadPicImage_MultiSource
===============
*/
mpic_t *GL_LoadPicImage_MultiSource (const char *paths[], const char *filenames[], const char *id,
							  int texflags, int item_dirlevel)
{
	int			i;
	byte		*data;
	static		mpic_t	pic;
#if 0
	int			glwidth, glheight;
	char		identifier[MAX_QPATH] = "pic:";
	byte		*src, *dest, *buf;
#endif

	if (no24bit)
		return NULL;

	if (!(data = GL_LoadImage_MultiSource (paths, filenames, texflags, item_dirlevel)))
		return NULL;

	pic.width = image_width;
	pic.height = image_height;

	if (texflags & TEX_ALPHA)
	{
		texflags &= ~TEX_ALPHA;
		for (i=0 ; i < image_width * image_height ; i++)
		{
			if (((((unsigned *)data)[i] >> 24) & 0xFF) < 255)
			{
				texflags |= TEX_ALPHA;
				break;
			}
		}
	}


#if 0
/* JDH: this duplicated a lot of the code in GL_LoadPicTexture, so I merged it:
	for (glwidth = 1 ; glwidth < pic.width ; glwidth <<= 1)
		;
	for (glheight = 1 ; glheight < pic.height ; glheight <<= 1)
		;

	Q_strcpy (identifier + 4, id ? id : filenames[0], sizeof(identifier) - 4);
	if (glwidth == pic.width && glheight == pic.height)
	{
		pic.texnum = GL_LoadTexture (identifier, pic.width, pic.height, data, texflags, 4);
		pic.sl = 0;
		pic.sh = 1;
		pic.tl = 0;
		pic.th = 1;
	}
	else
	{
		buf = Q_calloc (glwidth * glheight, 4);

		src = data;
		dest = buf;
		for (i=0 ; i<pic.height ; i++)
		{
			memcpy (dest, src, pic.width * 4);
			src += pic.width * 4;
			dest += glwidth * 4;
		}
		pic.texnum = GL_LoadTexture (identifier, glwidth, glheight, buf, texflags & ~TEX_MIPMAP, 4);
		pic.sl = 0;
		pic.sh = (float)pic.width / glwidth;
		pic.tl = 0;
		pic.th = (float)pic.height / glheight;
		free (buf);
	}
*/
#else
	GL_LoadPicTexture ((id ? id : filenames[0]), &pic, data, texflags, 4);
#endif
	
	free (data);
	return &pic;
}

/*
===============
GL_LoadPicImage
===============
*/
mpic_t *GL_LoadPicImage (const char *path, const char *filename, const char *id, int texflags, int item_dirlevel)
{
	const char	*pathlist[2] = {path, NULL};
	const char	*namelist[2] = {filename, NULL};

	return GL_LoadPicImage_MultiSource (pathlist, namelist, id, texflags, item_dirlevel);
}


/*
===============
Multitexturing functions
===============
*/
static	GLenum	oldtarget = GL_TEXTURE0_ARB;		// active TMU
static	int	cnttextures[MAX_TMUS] = {-1, -1, -1, -1};		// active texture for each TMU
static qboolean	mtexenabled = false;

void GL_SelectTMUTexture (GLenum target)
{
	if (target == oldtarget)
		return;

	qglActiveTexture (target);

	cnttextures[oldtarget-GL_TEXTURE0_ARB] = currenttexture;
	currenttexture = cnttextures[target-GL_TEXTURE0_ARB];
	oldtarget = target;
}

void GL_DisableMultitexture (void)
{
	if (mtexenabled)
	{
		glDisable (GL_TEXTURE_2D);
		GL_SelectTMUTexture (GL_TEXTURE0_ARB);
		mtexenabled = false;
	}
}

void GL_EnableMultitexture (void)
{
	if (gl_mtexable)
	{
		GL_SelectTMUTexture (GL_TEXTURE1_ARB);
		glEnable (GL_TEXTURE_2D);
		mtexenabled = true;
	}
}

void GL_EnableTMU (GLenum target)
{
	GL_SelectTMUTexture (target);
	if (target != GL_TEXTURE0_ARB)		// JDH: added this check (1st TMU will already have tex2d enabled)
		glEnable (GL_TEXTURE_2D);
}

void GL_DisableTMU (GLenum target)
{
	GL_SelectTMUTexture (target);
	if (target != GL_TEXTURE0_ARB)		// JDH: added this check
		glDisable (GL_TEXTURE_2D);
}

#endif		//#ifndef RQM_SV_ONLY
