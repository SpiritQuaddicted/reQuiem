/*
Copyright (C) 2001-2002       A Nourai

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// image.c -- handling images

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "png.h"

// On Linux, boolean is an int, not a uchar
#ifndef _WIN32
  #include <dlfcn.h>
  #define boolean int
  #define HAVE_BOOLEAN
  #define __RPCNDR_H__
#endif

#include "jpeglib.h"


// comment this out for static linking to libpng:
//  (you'll also need to add libpng to the link step)
#define QPNG_DYNAMIC

// ditto for libjpeg:
#define QJPG_DYNAMIC

#define	IMAGE_MAX_DIMENSIONS	4096

int	image_width, image_height;
qboolean png_available = false;
qboolean jpg_available = false;

cvar_t	png_compression_level  = {"png_compression_level",   "1", CVAR_FLAG_ARCHIVE};
cvar_t	jpeg_compression_level = {"jpeg_compression_level", "75", CVAR_FLAG_ARCHIVE};

/*
=========================================================

			Targa

=========================================================
*/

#define TGA_MAXCOLORS 16384

/* Definitions for image types. */
#define TGA_Null	0	/* no image data */
#define TGA_Map		1	/* Uncompressed, color-mapped images. */
#define TGA_RGB		2	/* Uncompressed, RGB images. */
#define TGA_Mono	3	/* Uncompressed, black and white images. */
#define TGA_RLEMap	9	/* Runlength encoded color-mapped images. */
#define TGA_RLERGB	10	/* Runlength encoded RGB images. */
#define TGA_RLEMono	11	/* Compressed, black and white images. */
#define TGA_CompMap	32	/* Compressed color-mapped data, using Huffman, Delta, and runlength encoding. */
#define TGA_CompMap4	33	/* Compressed color-mapped data, using Huffman, Delta, and runlength encoding. 4-pass quadtree-type process. */

/* Definitions for interleave flag. */
#define TGA_IL_None	0	/* non-interleaved. */
#define TGA_IL_Two	1	/* two-way (even/odd) interleaving */
#define TGA_IL_Four	2	/* four way interleaving */
#define TGA_IL_Reserved	3	/* reserved */

/* Definitions for origin flag */
#define TGA_O_UPPER	0	/* Origin in lower left-hand corner. */
#define TGA_O_LOWER	1	/* Origin in upper left-hand corner. */

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

#ifdef QPNG_DYNAMIC
static void *libpng_handle = NULL;
#endif

png_uint_32 (*qpng_access_version_number) (void) = NULL;

void (*qpng_read_end) (png_structp, png_infop) = NULL;
void (*qpng_read_image) (png_structp, png_bytepp) = NULL;
void (*qpng_read_info) (png_structp, png_infop) = NULL;
void (*qpng_read_update_info) (png_structp, png_infop) = NULL;

void (*qpng_write_end) (png_structp, png_infop) = NULL;
void (*qpng_write_image) (png_structp, png_bytepp) = NULL;
void (*qpng_write_info) (png_structp, png_infop) = NULL;

png_byte (*qpng_get_bit_depth) (png_structp, png_infop) = NULL;
png_byte (*qpng_get_channels) (png_structp, png_infop) = NULL;
png_uint_32 (*qpng_get_IHDR) (png_structp, png_infop, png_uint_32 *, png_uint_32 *,
									int *, int *, int *, int *, int *) = NULL;
png_voidp (*qpng_get_io_ptr) (png_structp) = NULL;
png_uint_32 (*qpng_get_rowbytes) (png_structp, png_infop) = NULL;
png_uint_32 (*qpng_get_valid) (png_structp, png_infop, png_uint_32) = NULL;

void (*qpng_set_compression_level) (png_structp, int) = NULL;
void (*qpng_set_expand) (png_structp) = NULL;
void (*qpng_set_filler) (png_structp, png_uint_32, int) = NULL;
void (*qpng_set_gray_1_2_4_to_8) (png_structp) = NULL;
void (*qpng_set_gray_to_rgb) (png_structp) = NULL;
void (*qpng_set_IHDR) (png_structp, png_infop, png_uint_32, png_uint_32,
							int, int, int, int, int) = NULL;
void (*qpng_set_palette_to_rgb) (png_structp) = NULL;
void (*qpng_set_read_fn) (png_structp, png_voidp, png_rw_ptr) = NULL;
void (*qpng_set_sig_bytes) (png_structp, int) = NULL;
void (*qpng_set_strip_16) (png_structp) = NULL;
void (*qpng_set_tRNS_to_alpha) (png_structp) = NULL;
void (*qpng_set_write_fn) (png_structp, png_voidp, png_rw_ptr, png_flush_ptr) = NULL;

png_infop (*qpng_create_info_struct) (png_structp) = NULL;
png_structp (*qpng_create_read_struct) (png_const_charp, png_voidp, png_error_ptr, png_error_ptr) = NULL;
png_structp (*qpng_create_write_struct) (png_const_charp, png_voidp, png_error_ptr, png_error_ptr) = NULL;
void (*qpng_destroy_read_struct) (png_structpp, png_infopp, png_infopp) = NULL;
void (*qpng_destroy_write_struct) (png_structpp, png_infopp) = NULL;
int (*qpng_sig_cmp) (png_bytep, png_size_t, png_size_t) = NULL;

#ifdef QPNG_DYNAMIC
#  ifdef _WIN32
#    define PNG_GETFUNC(f) (q##f = (void *)GetProcAddress(libpng_handle, #f)); \
				if (!q##f) {curr_proc = #f; goto PNG_LOAD_FAIL;}
#    define PNG_LIBNAME		"libpng12.dll"
#  else
#    define PNG_GETFUNC(f) (q##f = (void *)dlsym(libpng_handle, #f)); \
				if (!q##f) {curr_proc = #f; goto PNG_LOAD_FAIL;}
#    ifdef MACOSX
#      define PNG_LIBNAME	"libpng12.0.dylib"
#    else
#      define PNG_LIBNAME	"libpng12.so.0"
#    endif
#  endif
#else
#  define PNG_GETFUNC(f) (q##f = f)
#endif

#ifdef QJPG_DYNAMIC
static void *libjpg_handle = NULL;
#endif

#define qjpeg_create_compress(cinfo) \
    qjpeg_CreateCompress((cinfo), JPEG_LIB_VERSION, \
			(size_t) sizeof(struct jpeg_compress_struct))

#define qjpeg_create_decompress(cinfo) \
    qjpeg_CreateDecompress((cinfo), JPEG_LIB_VERSION, \
			  (size_t) sizeof(struct jpeg_decompress_struct))

struct jpeg_error_mgr * (*qjpeg_std_error) (struct jpeg_error_mgr * err);

// JPEG decompression:
void (*qjpeg_CreateDecompress) (j_decompress_ptr cinfo, int version, size_t structsize);
void (*qjpeg_stdio_src) (j_decompress_ptr cinfo, FILE * infile);
int (*qjpeg_read_header) (j_decompress_ptr cinfo, boolean require_image);
boolean (*qjpeg_start_decompress) (j_decompress_ptr cinfo);
JDIMENSION (*qjpeg_read_scanlines) (j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines);
boolean (*qjpeg_finish_decompress) (j_decompress_ptr cinfo);
void (*qjpeg_destroy_decompress) (j_decompress_ptr cinfo);

// JPEG compression:
void (*qjpeg_CreateCompress) (j_compress_ptr cinfo, int version, size_t structsize);
void (*qjpeg_stdio_dest) (j_compress_ptr cinfo, FILE * outfile);
void (*qjpeg_set_defaults) (j_compress_ptr cinfo);
void (*qjpeg_set_quality) (j_compress_ptr cinfo, int quality, boolean force_baseline);
void (*qjpeg_start_compress) (j_compress_ptr cinfo, boolean write_all_tables);
JDIMENSION (*qjpeg_write_scanlines) (j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines);
void (*qjpeg_finish_compress) (j_compress_ptr cinfo);
void (*qjpeg_destroy_compress) (j_compress_ptr cinfo);


#ifdef QJPG_DYNAMIC
  boolean (*qjpeg_resync_to_restart) (j_decompress_ptr cinfo, int desired);

#  ifdef _WIN32
#    define JPG_GETFUNC(f) (q##f = (void *)GetProcAddress(libjpg_handle, #f)); \
		if (!q##f) {curr_proc = #f; goto JPG_LOAD_FAIL;}
#    define JPG_LIBNAME1	"libjpeg.dll"
#    define JPG_LIBNAME2	"jpeg62.dll"
#  else
#    define JPG_GETFUNC(f) (q##f = (void *)dlsym(libjpg_handle, #f)); \
		if (!q##f) {curr_proc = #f; goto JPG_LOAD_FAIL;}
#    ifdef MACOSX
#      define JPG_LIBNAME1	"libjpeg.62.dylib"
#      define JPG_LIBNAME2	"libjpeg.dylib"
#    else
#      define JPG_LIBNAME1	"libjpeg.so.62"
#      define JPG_LIBNAME2	"libjpeg.so"
#    endif
#  endif
#else
#  define JPG_GETFUNC(f) (q##f = f)
#endif


/*
=============
Image_InitPNG
=============
*/
void Image_InitPNG (void)
{
	png_uint_32 png_version = 0;
#ifdef QPNG_DYNAMIC
	char *curr_proc = NULL;
#endif

#ifdef QPNG_DYNAMIC
	if (!(libpng_handle = COM_LoadLibrary(PNG_LIBNAME)))
	{
		Con_Print ("\x02""Failed to load libpng!\n");
		return;
	}
#endif

	PNG_GETFUNC (png_access_version_number);

	PNG_GETFUNC (png_read_end);
	PNG_GETFUNC (png_read_image);
	PNG_GETFUNC (png_read_info);
	PNG_GETFUNC (png_read_update_info);

	PNG_GETFUNC (png_write_end);
	PNG_GETFUNC (png_write_image);
	PNG_GETFUNC (png_write_info);

	PNG_GETFUNC (png_get_bit_depth);
	PNG_GETFUNC (png_get_channels);
	PNG_GETFUNC (png_get_IHDR);
	PNG_GETFUNC (png_get_io_ptr);
	PNG_GETFUNC (png_get_rowbytes);
	PNG_GETFUNC (png_get_valid);

	PNG_GETFUNC (png_set_compression_level);
	PNG_GETFUNC (png_set_expand);
	PNG_GETFUNC (png_set_filler);
	PNG_GETFUNC (png_set_gray_1_2_4_to_8);
	PNG_GETFUNC (png_set_gray_to_rgb);
	PNG_GETFUNC (png_set_IHDR);
	PNG_GETFUNC (png_set_palette_to_rgb);
	PNG_GETFUNC (png_set_read_fn);
	PNG_GETFUNC (png_set_sig_bytes);
	PNG_GETFUNC (png_set_strip_16);
	PNG_GETFUNC (png_set_tRNS_to_alpha);
	PNG_GETFUNC (png_set_write_fn);

	PNG_GETFUNC (png_create_info_struct);
	PNG_GETFUNC (png_create_read_struct);
	PNG_GETFUNC (png_create_write_struct);
	PNG_GETFUNC (png_destroy_read_struct);
	PNG_GETFUNC (png_destroy_write_struct);

	PNG_GETFUNC (png_sig_cmp);

	png_version = qpng_access_version_number ();

#ifdef QPNG_DYNAMIC
	Con_Print ("Successfully loaded ");
#else
	Con_Print ("Using ");
#endif
	Con_Printf ("libpng v%d.%d.%d\n", (png_version % 1000000)/10000,
				(png_version % 10000)/100, png_version % 100);
	png_available = true;
	return;

#ifdef QPNG_DYNAMIC
PNG_LOAD_FAIL:
	Con_Printf ("\x02""Failed to load libpng (missing function %s)\n", curr_proc);
#endif
}

int fgetLittleShort (FILE *f)
{
	byte	b1, b2;

	b1 = fgetc(f);
	b2 = fgetc(f);

	return (short)(b1 + b2*256);
}

/*
=============
Image_InitJPG
=============
*/
void Image_InitJPG (void)
{
#ifdef QJPG_DYNAMIC
	char *curr_proc = NULL;

	if (!(libjpg_handle = COM_LoadLibrary(JPG_LIBNAME1)) &&
	    !(libjpg_handle = COM_LoadLibrary(JPG_LIBNAME2)))
	{
		Con_Print ("\x02""Failed to load libjpeg!\n");
		return;
	}

	JPG_GETFUNC (jpeg_resync_to_restart);
#endif

	JPG_GETFUNC (jpeg_std_error);

	JPG_GETFUNC (jpeg_CreateDecompress);
	JPG_GETFUNC (jpeg_stdio_src);
	JPG_GETFUNC (jpeg_read_header);
	JPG_GETFUNC (jpeg_start_decompress);
	JPG_GETFUNC (jpeg_read_scanlines);
	JPG_GETFUNC (jpeg_finish_decompress);
	JPG_GETFUNC (jpeg_destroy_decompress);

	JPG_GETFUNC (jpeg_CreateCompress);
	JPG_GETFUNC (jpeg_stdio_dest);
	JPG_GETFUNC (jpeg_set_defaults);
	JPG_GETFUNC (jpeg_set_quality);
	JPG_GETFUNC (jpeg_start_compress);
	JPG_GETFUNC (jpeg_write_scanlines);
	JPG_GETFUNC (jpeg_finish_compress);
	JPG_GETFUNC (jpeg_destroy_compress);

	jpg_available = true;
	return;

#ifdef QJPG_DYNAMIC
JPG_LOAD_FAIL:
	Con_Printf ("\x02""Failed to load libjpeg (missing function %s)\n", curr_proc);
#endif

}

/*
=============
Image_Init
=============
*/
void Image_Init (void)
{
	Cvar_RegisterInt (&png_compression_level, 0, 9);	// Z_NO_COMPRESSION to Z_BEST_COMPRESSION (zlib.h)
	Cvar_RegisterInt (&jpeg_compression_level, 0, 100);

	Image_InitPNG ();
	Image_InitJPG ();
}

/*
=================
Image_ReadTGAData
=================
*/
byte *Image_ReadTGAData (FILE *fin, int dataLen)
{
	byte *data = Q_calloc (dataLen, 1);

	if (fread (data, 1, dataLen, fin) < dataLen)
	{
		free (data);
		return NULL;
	}

	return data;
}

/*
=================
Image_LoadTGAData
=================
*/
byte *Image_LoadTGAData (FILE *fin, int dataLen, TargaHeader *header, byte *ColorMap)
{
	byte *dataIn, *dataOut, *src;
	int w, h, RLE_count, RLE_flag, pixel_size, interleave;
	int truerow, baserow, y, x, map_idx;
	qboolean rlencoded, istopdown;
	byte r, g, b, a;
	WORD index;
	DWORD dwColor, *dst;

	dataIn = Image_ReadTGAData (fin, dataLen);
	if (!dataIn) return NULL;

	w = header->width;
	h = header->height;
	dataOut = Q_calloc (w * h * 4, 1);

	/* check run-length encoding. */
	rlencoded = (header->image_type == TGA_RLEMap || header->image_type == TGA_RLERGB || header->image_type == TGA_RLEMono);
	RLE_count = RLE_flag = 0;

	/* set global width & height values: */
	image_width = w;
	image_height = h;

	/* read the Targa file body and convert to portable format. */
	pixel_size = header->pixel_size;
	istopdown = (header->attributes & 0x20) >> 5;
	b = (header->attributes & 0xC0) >> 6;
	interleave = (1 << b);

	truerow = 0;
	baserow = 0;
	src = dataIn;

	for (y=0 ; y<h ; y++)
	{
		map_idx = (istopdown ? truerow : h - truerow - 1);
		dst = (DWORD *) (dataOut + map_idx * w * 4);

		for (x=0 ; x<w ; x++)
		{
			/* check if run length encoded. */
			if (rlencoded)
			{
				if (RLE_count)
				{
					/* have already read count & (at least) first pixel. */
					--RLE_count;
					if (RLE_flag)
						/* replicated pixels. */
						goto PixEncode;
				}
				else
				{
					/* have to restart run. */
					b = *src++;
					RLE_flag = (b & 0x80);
					if (RLE_flag)		// single pixel replicated
						RLE_count = b & 0x7F;
					else	// stream of unencoded pixels
						RLE_count = b;
				}
			}

			/* read appropriate number of bytes, break into RGB. */
			switch (pixel_size)
			{
			case 8:	/* grey scale - read and triplicate. */
				r = g = b = *src++;
				if (ColorMap)
					index = r;
				else
					dwColor = RGB(r,g,b) | 0xFF000000;
				break;

			case 15:	/* 5 bits each of red green and blue. */
						/* watch byte order. */
				index = *(WORD *) src;
				src += 2;
				r = (byte) ((index & 0x7C00) >> 7);
				g = (byte) ((index & 0x03E0) >> 2);
				b = (byte) ((index & 0x001F) << 3);
				if (!ColorMap)
					dwColor = RGB(r,g,b) | 0xFF000000;
				break;

			case 16:	/* 5 bits each of red green and blue, 1 alpha bit. */
						/* watch byte order. */
				index = *(WORD *) src;
				src += 2;
				r = (byte) ((index & 0x7C00) >> 7);
				g = (byte) ((index & 0x03E0) >> 2);
				b = (byte) ((index & 0x001F) << 3);
				if (!ColorMap)
				{
					a = (index & 0x8000) ? 255 : 0;
					dwColor = RGB(r,g,b) | (((DWORD) a) << 24);
				}
				break;

			case 24:	/* 8 bits each of blue, green and red. */
				dwColor = ((DWORD) *src++) << 16;	// blue
				dwColor |= ((WORD) *src++) << 8;	// green
				dwColor |= *src++;					// red
				dwColor |= 0xFF000000;				// alpha
				break;

			case 32:	/* 8 bits each of blue, green, red and alpha. */
				dwColor = ((DWORD) *src++) << 16;	// blue
				dwColor |= ((WORD) *src++) << 8;	// green
				dwColor |= *src++;					// red
				dwColor |= ((DWORD) *src++) << 24;	// alpha
				break;
			}

PixEncode:
			if (ColorMap)
			{
				*dst = *((DWORD *) ColorMap + index);
				dst++;
			}
			else
			{
				*dst = dwColor;
				dst++;
			}
		}

		truerow += interleave;
		if (truerow >= h)
			truerow = ++baserow;
	}

	free( dataIn );
	return dataOut;
}

/*
=============
Image_LoadTGA
=============
*/
byte *Image_LoadTGA (FILE *fin, int fileLen, const char *filename, int matchwidth, int matchheight)
{
	int		i, temp1, temp2, map_idx;
	qboolean	mapped;
	byte		*data, r, g, b, a, j, k, l, *ColorMap;
	TargaHeader	header;
	long		startPos;

	data = NULL;
	startPos = ftell( fin );

	header.id_length = fgetc (fin);
	header.colormap_type = fgetc (fin);
	header.image_type = fgetc (fin);

	header.colormap_index = fgetLittleShort (fin);
	header.colormap_length = fgetLittleShort (fin);
	header.colormap_size = fgetc (fin);
	header.x_origin = fgetLittleShort (fin);
	header.y_origin = fgetLittleShort (fin);
	header.width = fgetLittleShort (fin);
	header.height = fgetLittleShort (fin);
	header.pixel_size = fgetc (fin);
	header.attributes = fgetc (fin);

	if (header.width > IMAGE_MAX_DIMENSIONS || header.height > IMAGE_MAX_DIMENSIONS)
	{
		Con_DPrintf ("TGA image %s exceeds maximum supported dimensions\n", COM_SkipPath(filename));
		goto ExitPoint;
	}

	if ((matchwidth && header.width != matchwidth) || (matchheight && header.height != matchheight))
	{
		goto ExitPoint;
	}

	if (header.id_length != 0)
		fseek (fin, header.id_length, SEEK_CUR);

	/* validate TGA type */
	switch (header.image_type)
	{
	case TGA_Map:
	case TGA_RGB:
	case TGA_Mono:
	case TGA_RLEMap:
	case TGA_RLERGB:
	case TGA_RLEMono:
		break;

	default:
		Con_DPrintf ("Unsupported TGA image %s: Only type 1 (map), 2 (RGB), 3 (mono), 9 (RLEmap), 10 (RLERGB), 11 (RLEmono) TGA images supported\n", COM_SkipPath(filename));
		goto ExitPoint;
	}

	/* validate color depth */
	switch (header.pixel_size)
	{
	case 8:
	case 15:
	case 16:
	case 24:
	case 32:
		break;

	default:
		Con_DPrintf ("Unsupported TGA image %s: Only 8, 15, 16, 24 or 32 bit images (with colormaps) supported\n", COM_SkipPath(filename));
		goto ExitPoint;
	}

	r = g = b = a = l = 0;

	/* if required, read the color map information. */
	ColorMap = NULL;
	mapped = (header.image_type == TGA_Map || header.image_type == TGA_RLEMap) && header.colormap_type == 1;
	if (mapped)
	{
		/* validate colormap size */
		switch (header.colormap_size)
		{
		case 8:
		case 15:
		case 16:
		case 32:
		case 24:
			break;

		default:
			Con_DPrintf ("Unsupported TGA image %s: Only 8, 15, 16, 24 or 32 bit colormaps supported\n", COM_SkipPath(filename));
			goto ExitPoint;
		}

		temp1 = header.colormap_index;
		temp2 = header.colormap_length;
		if ((temp1 + temp2 + 1) >= TGA_MAXCOLORS)
		{
			goto ExitPoint;
		}

		ColorMap = Q_malloc (TGA_MAXCOLORS * 4);
		map_idx = 0;

		for (i = temp1 ; i < temp1 + temp2 ; ++i, map_idx += 4)
		{
			/* read appropriate number of bytes, break into rgb & put in map. */
			switch (header.colormap_size)
			{
			case 8:	/* grey scale, read and triplicate. */
				r = g = b = getc (fin);
				a = 255;
				break;

			case 15:	/* 5 bits each of red green and blue. */
						/* watch byte order. */
				j = getc (fin);
				k = getc (fin);
				l = ((unsigned int)k << 8) + j;
				r = (byte)(((k & 0x7C) >> 2) << 3);
				g = (byte)((((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3);
				b = (byte)((j & 0x1F) << 3);
				a = 255;
				break;

			case 16:	/* 5 bits each of red green and blue, 1 alpha bit. */
						/* watch byte order. */
				j = getc (fin);
				k = getc (fin);
				l = ((unsigned int)k << 8) + j;
				r = (byte)(((k & 0x7C) >> 2) << 3);
				g = (byte)((((k & 0x03) << 3) + ((j & 0xE0) >> 5)) << 3);
				b = (byte)((j & 0x1F) << 3);
				a = (k & 0x80) ? 255 : 0;
				break;

			case 24:	/* 8 bits each of blue, green and red. */
				b = getc (fin);
				g = getc (fin);
				r = getc (fin);
				a = 255;
				l = 0;
				break;

			case 32:	/* 8 bits each of blue, green, red and alpha. */
				b = getc (fin);
				g = getc (fin);
				r = getc (fin);
				a = getc (fin);
				l = 0;
				break;
			}
			ColorMap[map_idx+0] = r;
			ColorMap[map_idx+1] = g;
			ColorMap[map_idx+2] = b;
			ColorMap[map_idx+3] = a;
		}
	}

/*******JDH*********/
	switch (header.pixel_size)
	{
		case 8:
		case 15:
		case 16:
			break;

		case 24:
		case 32:
			if (mapped)
			{
				Con_DPrintf ("Malformed TGA image %s: Colormapped image cannot be %d-bit\n",
								COM_SkipPath(filename), header.pixel_size);
				free (ColorMap);
				goto ExitPoint;
			}
			break;

		default:
			Con_DPrintf ("Malformed TGA image %s: Illegal pixel_size '%d'\n",
							COM_SkipPath(filename), header.pixel_size);
			if (mapped)
				free (ColorMap);
			goto ExitPoint;
	}

	fileLen -= ftell(fin) - startPos;
	data = Image_LoadTGAData (fin, fileLen, &header, ColorMap);

	if (mapped)
		free (ColorMap);

ExitPoint:
//	fclose (fin);
	return data;
}

/*
=============
Image_WriteTGA
  note: compression param is so all Image_Write functions have the same interface.
        It is ignored
=============
*/
qboolean Image_WriteTGA (const char *filename, int compression, const byte *pixels, int width, int height)
{
	byte		*buffer, temp;
	int		i, size;
	qboolean	retval;

	size = width * height * 3;
	buffer = Q_malloc (size + 18);
	memset (buffer, 0, 18);
	buffer[2] = 2;
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;

	memcpy (buffer + 18, pixels, size);

	for (i = 18 ; i < size + 18 ; i += 3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	retval = COM_WriteFile (filename, buffer, size + 18);
	free (buffer);

	return retval;
}

/*
=========================================================

			PNG

=========================================================
*/

//#ifdef GLQUAKE

static void PNG_IO_user_read_data (png_structp png_ptr, png_bytep data, png_size_t length)
{
	FILE	*f = (FILE *)qpng_get_io_ptr(png_ptr);

	fread (data, 1, length, f);
}

static void PNG_IO_user_write_data (png_structp png_ptr, png_bytep data, png_size_t length)
{
	FILE	*f = (FILE *)qpng_get_io_ptr(png_ptr);

	fwrite (data, 1, length, f);
}

static void PNG_IO_user_flush_data (png_structp png_ptr)
{
	FILE	*f = (FILE *)qpng_get_io_ptr(png_ptr);

	fflush (f);
}

/*
=============
Image_LoadPNG
=============
*/
byte *Image_LoadPNG (FILE *fin, int filelen, const char *filename, int matchwidth, int matchheight)
{
	byte		header[8], **rowpointers, *data;
	png_structp	png_ptr;
	png_infop	pnginfo;
	int			y, width, height, bitdepth, colortype;
	int			interlace, compression, filter, bytesperpixel;
	unsigned long	rowbytes;

	if (!png_available)
		return NULL;

//	if (!fin && COM_FOpenFile(filename, 0, &fin) == -1)
//		return NULL;

	fread (header, 1, 8, fin);

	if (qpng_sig_cmp(header, 0, 8))
	{
		Con_DPrintf ("Invalid PNG image %s\n", COM_SkipPath(filename));
//		fclose (fin);
		return NULL;
	}

	if (!(png_ptr = qpng_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
	{
//		fclose (fin);
		return NULL;
	}

	if (!(pnginfo = qpng_create_info_struct(png_ptr)))
	{
		qpng_destroy_read_struct (&png_ptr, &pnginfo, NULL);
//		fclose (fin);
		return NULL;
	}

	if (setjmp(png_ptr->jmpbuf))
	{
		qpng_destroy_read_struct (&png_ptr, &pnginfo, NULL);
//		fclose (fin);
		return NULL;
	}

	qpng_set_read_fn (png_ptr, fin, PNG_IO_user_read_data);
	qpng_set_sig_bytes (png_ptr, 8);
	qpng_read_info (png_ptr, pnginfo);
	qpng_get_IHDR (png_ptr, pnginfo, (png_uint_32 *)&width, (png_uint_32 *)&height, &bitdepth, &colortype, &interlace, &compression, &filter);

	if (width > IMAGE_MAX_DIMENSIONS || height > IMAGE_MAX_DIMENSIONS)
	{
		Con_DPrintf ("PNG image %s exceeds maximum supported dimensions\n", COM_SkipPath(filename));
		qpng_destroy_read_struct (&png_ptr, &pnginfo, NULL);
//		fclose (fin);
		return NULL;
	}

	if ((matchwidth && width != matchwidth) || (matchheight && height != matchheight))
	{
		qpng_destroy_read_struct (&png_ptr, &pnginfo, NULL);
//		fclose (fin);
		return NULL;
	}

	if (colortype == PNG_COLOR_TYPE_PALETTE)
	{
		qpng_set_palette_to_rgb (png_ptr);
		qpng_set_filler (png_ptr, 255, PNG_FILLER_AFTER);
	}

	if (colortype == PNG_COLOR_TYPE_GRAY && bitdepth < 8)
		qpng_set_gray_1_2_4_to_8 (png_ptr);

	if (qpng_get_valid(png_ptr, pnginfo, PNG_INFO_tRNS))
		qpng_set_tRNS_to_alpha (png_ptr);

	if (colortype == PNG_COLOR_TYPE_GRAY || colortype == PNG_COLOR_TYPE_GRAY_ALPHA)
		qpng_set_gray_to_rgb (png_ptr);

	if (colortype != PNG_COLOR_TYPE_RGBA)
		qpng_set_filler (png_ptr, 255, PNG_FILLER_AFTER);

	if (bitdepth < 8)
		qpng_set_expand (png_ptr);
	else if (bitdepth == 16)
		qpng_set_strip_16 (png_ptr);

	qpng_read_update_info (png_ptr, pnginfo);
	rowbytes = qpng_get_rowbytes (png_ptr, pnginfo);
	bytesperpixel = qpng_get_channels (png_ptr, pnginfo);
	bitdepth = qpng_get_bit_depth (png_ptr, pnginfo);

	if (bitdepth != 8 || bytesperpixel != 4)
	{
		Con_DPrintf ("Unsupported PNG image %s: Bad color depth and/or bpp\n", COM_SkipPath(filename));
		qpng_destroy_read_struct (&png_ptr, &pnginfo, NULL);
//		fclose (fin);
		return NULL;
	}

	data = Q_malloc (height * rowbytes);
	rowpointers = Q_malloc (height * sizeof(*rowpointers));

	for (y=0 ; y<height ; y++)
		rowpointers[y] = data + y * rowbytes;

	qpng_read_image (png_ptr, rowpointers);
	qpng_read_end (png_ptr, NULL);

	qpng_destroy_read_struct (&png_ptr, &pnginfo, NULL);
	free (rowpointers);
//	fclose (fin);
	image_width = width;
	image_height = height;

	return data;
}

/*
=============
Image_WritePNG
=============
*/
qboolean Image_WritePNG (const char *filename, int compression, const byte *pixels, int width, int height)
{
	char		name[MAX_OSPATH];
	int		i, bpp = 3, pngformat, width_sign;
	FILE		*fp;
	png_structp	png_ptr;
	png_infop	info_ptr;
	png_byte	**rowpointers;

	if (!png_available)
		return false;

	Q_snprintfz (name, sizeof(name), "%s/%s", com_gamedir, filename);

	if (!(fp = fopen(name, "wb")))
	{
		COM_CreatePath (name);
		if (!(fp = fopen(name, "wb")))
			return false;
	}

	if (!(png_ptr = qpng_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
	{
		fclose (fp);
		return false;
	}

	if (!(info_ptr = qpng_create_info_struct(png_ptr)))
	{
		qpng_destroy_write_struct (&png_ptr, (png_infopp)NULL);
		fclose (fp);
		return false;
	}

	if (setjmp(png_ptr->jmpbuf))
	{
		qpng_destroy_write_struct (&png_ptr, &info_ptr);
		fclose (fp);
		return false;
	}

	width_sign = (width < 0) ? -1 : 1;
	width = abs(width);

	qpng_set_write_fn (png_ptr, fp, PNG_IO_user_write_data, PNG_IO_user_flush_data);
	qpng_set_compression_level (png_ptr, bound(Z_NO_COMPRESSION, compression, Z_BEST_COMPRESSION));

	pngformat = (bpp == 4) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
	qpng_set_IHDR (png_ptr, info_ptr, width, height, 8, pngformat, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	qpng_write_info (png_ptr, info_ptr);

	rowpointers = Q_malloc (height * sizeof(*rowpointers));
	for (i=0 ; i<height ; i++)
		rowpointers[i] = (byte *)pixels + i * width_sign * width * bpp;

	qpng_write_image (png_ptr, rowpointers);
	qpng_write_end (png_ptr, info_ptr);
	free (rowpointers);
	qpng_destroy_write_struct (&png_ptr, &info_ptr);
	fclose (fp);

	return true;
}

//#endif


/*
=========================================================

			JPEG

=========================================================
*/

static qboolean image_jpg_err;

// JDH: custom exit-error handler, otherwise game quits if there's an JPEG error!
static void Image_JPG_error_exit (j_common_ptr cinfo)
{
	image_jpg_err = true;
}

#ifdef QJPG_DYNAMIC

// If we're dynamically linking to the lib, passing a FILE*
// for the data source/dest is not likely to work.  So we
// need some custom routines to read/write the file.
// That's what this mess is about.  (But it does shave ~90k
// off the exe size compared to static linking).

static void Image_JPG_dummy (j_decompress_ptr cinfo) {}

static boolean Image_JPG_getdata (j_decompress_ptr cinfo)
{
	// this proc will get called only if file data is incomplete
	image_jpg_err = true;
	return FALSE;
}

static void Image_JPG_skipdata (j_decompress_ptr cinfo, long num_bytes)
{
	if (num_bytes > cinfo->src->bytes_in_buffer)
		num_bytes = cinfo->src->bytes_in_buffer;

	cinfo->src->bytes_in_buffer -= num_bytes;
	cinfo->src->next_input_byte += num_bytes;
}

static byte * Image_JPG_mem_src (j_decompress_ptr cinfo, FILE *f, int filelen)
{
	byte *buf = Q_malloc (filelen + sizeof (struct jpeg_source_mgr));

	if (fread (buf, 1, filelen, f) < filelen)
	{
		image_jpg_err = true;
		return buf;		// so it gets free'd
	}

	cinfo->src = (struct jpeg_source_mgr *) (buf + filelen);

	cinfo->src->next_input_byte = buf;
	cinfo->src->bytes_in_buffer = filelen;

	cinfo->src->init_source = Image_JPG_dummy;
	cinfo->src->fill_input_buffer = Image_JPG_getdata;
	cinfo->src->skip_input_data = Image_JPG_skipdata;
	cinfo->src->resync_to_restart = qjpeg_resync_to_restart; // use the default method
	cinfo->src->term_source = Image_JPG_dummy;

	return buf;
}

#endif		// #ifdef QJPG_DYNAMIC

/*
=============
Image_LoadJPG
=============
*/
byte *Image_LoadJPG (FILE *fin, int filelen, const char *filename, int matchwidth, int matchheight)
{
	int		i, row_stride, progress = 0;
	byte	*data = NULL, *scanline = NULL, *p;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
#ifdef QJPG_DYNAMIC
	byte	*filebuf = NULL;
#endif

	if (!jpg_available)
		return NULL;

//	if (!fin && COM_FOpenFile(filename, &fin) == -1)
//		return NULL;

	image_jpg_err = false;

	cinfo.err = qjpeg_std_error (&jerr);
	if (image_jpg_err)
		goto LOADJPG_EXIT;

	cinfo.err->error_exit = Image_JPG_error_exit;

	qjpeg_create_decompress (&cinfo);
	progress++;
	if (image_jpg_err)
		goto LOADJPG_EXIT;

#ifdef QJPG_DYNAMIC
	filebuf = Image_JPG_mem_src (&cinfo, fin, filelen);
#else
	qjpeg_stdio_src (&cinfo, fin);
#endif
	if (image_jpg_err)
		goto LOADJPG_EXIT;

	qjpeg_read_header (&cinfo, true);		// FAILS if QJPG_DYNAMIC
	if (image_jpg_err)
		goto LOADJPG_EXIT;

	if (cinfo.image_width > IMAGE_MAX_DIMENSIONS || cinfo.image_height > IMAGE_MAX_DIMENSIONS)
	{
		Con_DPrintf ("JPEG image %s exceeds maximum supported dimensions\n", COM_SkipPath(filename));
		goto LOADJPG_EXIT;
	}

	if ((matchwidth && cinfo.image_width != matchwidth) || (matchheight && cinfo.image_height != matchheight))
	{
		goto LOADJPG_EXIT;
	}

	qjpeg_start_decompress (&cinfo);
	progress++;
	if (image_jpg_err)
		goto LOADJPG_EXIT;

	data = Q_malloc (cinfo.image_width * cinfo.image_height * 4);
	row_stride = cinfo.output_width * cinfo.output_components;
	scanline = Q_malloc (row_stride);

	p = data;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		qjpeg_read_scanlines (&cinfo, &scanline, 1);
		if (image_jpg_err)
			goto LOADJPG_EXIT;

		// convert the image to RGBA
		switch (cinfo.output_components)
		{
		// RGB images
		case 3:
			for (i=0 ; i<row_stride ; )
			{
				*p++ = scanline[i++];
				*p++ = scanline[i++];
				*p++ = scanline[i++];
				*p++ = 255;
			}
			break;

		// greyscale images (default to it, just in case)
		case 1:
		default:
			for (i=0 ; i<row_stride ; i++)
			{
				*p++ = scanline[i];
				*p++ = scanline[i];
				*p++ = scanline[i];
				*p++ = 255;
			}
			break;
		}
	}

	image_width = cinfo.image_width;
	image_height = cinfo.image_height;

LOADJPG_EXIT:
	if (progress > 1)
		qjpeg_finish_decompress (&cinfo);
	if (progress > 0)
		qjpeg_destroy_decompress (&cinfo);
	if (scanline)
		free (scanline);
#ifdef QJPG_DYNAMIC
	if (filebuf)
		free (filebuf);
#endif
//	fclose (fin);

	if (image_jpg_err)
	{
		Con_Printf ("\x02""Error loading image %s\n", COM_SkipPath(filename));
		if (data)
			free (data);
		return NULL;
	}

	return data;
}

#ifdef QJPG_DYNAMIC

#define JPG_BUF_SIZE 16384
byte image_jpg_buf[JPG_BUF_SIZE];
FILE *image_jpg_fout;

void Image_JPG_init_dest (j_compress_ptr cinfo)
{
	cinfo->dest->free_in_buffer = JPG_BUF_SIZE;
	cinfo->dest->next_output_byte = image_jpg_buf;
}

boolean Image_JPG_write_data (j_compress_ptr cinfo)
{
	if (fwrite (image_jpg_buf, JPG_BUF_SIZE, 1, image_jpg_fout) != 1)
	{
		image_jpg_err = true;
		return FALSE;
	}

	cinfo->dest->free_in_buffer = JPG_BUF_SIZE;
	cinfo->dest->next_output_byte = image_jpg_buf;
	return TRUE;
}

void Image_JPG_term_dest (j_compress_ptr cinfo)
{
	size_t count = JPG_BUF_SIZE - cinfo->dest->free_in_buffer;

	if (count)
		fwrite (image_jpg_buf, count, 1, image_jpg_fout);
}

static void Image_JPG_mem_dest (j_compress_ptr cinfo, FILE *f)
{
	static struct jpeg_destination_mgr dest;

//	cinfo->dest = Q_malloc (sizeof (struct jpeg_destination_mgr));
	cinfo->dest = &dest;
	cinfo->dest->init_destination = Image_JPG_init_dest;
	cinfo->dest->empty_output_buffer = Image_JPG_write_data;
	cinfo->dest->term_destination = Image_JPG_term_dest;

	image_jpg_fout = f;
}
#endif		// #ifdef QJPG_DYNAMIC

/*
=============
Image_WriteJPG
=============
*/
qboolean Image_WriteJPG (const char *filename, int compression, const byte *pixels, int width, int height)
{
	char	name[MAX_OSPATH];
	byte	*scanline;
	FILE	*fout;
	int		progress = 0;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	Q_snprintfz (name, sizeof(name), "%s/%s", com_gamedir, filename);

	if (!(fout = fopen(name, "wb")))
	{
		COM_CreatePath (name);
		if (!(fout = fopen(name, "wb")))
			return false;
	}

	image_jpg_err = false;

	cinfo.err = qjpeg_std_error (&jerr);
	if (image_jpg_err)
		goto WRITEJPG_EXIT;

	cinfo.err->error_exit = Image_JPG_error_exit;

	qjpeg_create_compress (&cinfo);
	progress++;
	if (image_jpg_err)
		goto WRITEJPG_EXIT;

#ifdef QJPG_DYNAMIC
	Image_JPG_mem_dest (&cinfo, fout);
#else
	qjpeg_stdio_dest (&cinfo, fout);
#endif
	if (image_jpg_err)
		goto WRITEJPG_EXIT;

	cinfo.image_width = abs(width);
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	qjpeg_set_defaults (&cinfo);
	qjpeg_set_quality (&cinfo, bound(0, compression, 100), true);
	if (image_jpg_err)
		goto WRITEJPG_EXIT;

	qjpeg_start_compress (&cinfo, true);
	progress++;
	if (image_jpg_err)
		goto WRITEJPG_EXIT;

	while (cinfo.next_scanline < height)
	{
//		scanline = &pixels[cinfo.next_scanline*width*3];
		scanline = (byte *) pixels + (cinfo.next_scanline*width*3);
		qjpeg_write_scanlines (&cinfo, &scanline, 1);
		if (image_jpg_err)
			goto WRITEJPG_EXIT;
	}

WRITEJPG_EXIT:
	if (progress > 1)
		qjpeg_finish_compress (&cinfo);
	if (progress > 0)
		qjpeg_destroy_compress (&cinfo);
	fclose (fout);

	return !image_jpg_err;
}


/*
=========================================================

			PCX

=========================================================
*/

typedef struct
{
	char		manufacturer;
	char		version;
	char		encoding;
	char		bits_per_pixel;
	unsigned short	xmin,ymin,xmax,ymax;
	unsigned short	hres,vres;
	unsigned char	palette[48];
	char		reserved;
	char		color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;
	char		filler[58];
//	unsigned char	data;			// unbounded
} pcx_t;

/*
==============
WritePCX
==============
*/
/*qboolean Image_WritePCX (char *filename, byte *data, int width, int height, byte *palette)
{
	int	rowbytes = width;
	int	i, j, length;
	pcx_t	*pcx;
	byte	*pack;

	if (!(pcx = Q_malloc(width * height * 2 + 1000)))
		return false;

	pcx->manufacturer = 0x0a;	// PCX id
	pcx->version = 5;		// 256 color
 	pcx->encoding = 1;		// uncompressed
	pcx->bits_per_pixel = 8;	// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleShort((short)(width-1));
	pcx->ymax = LittleShort((short)(height-1));
	pcx->hres = LittleShort((short)width);
	pcx->vres = LittleShort((short)height);
	memset (pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleShort((short)width);
	pcx->palette_type = LittleShort(2);		// not a grey scale
	memset (pcx->filler, 0, sizeof(pcx->filler));

// pack the image
//	pack = &pcx->data;
	pack = (byte *) (pcx + 1);

	for (i=0 ; i<height ; i++)
	{
		for (j=0 ; j<width ; j++)
		{
			if ((*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}

		data += rowbytes - width;
	}

// write the palette
	*pack++ = 0x0c;	// palette ID byte
	for (i=0 ; i<768 ; i++)
		*pack++ = *palette++;

// write output file
	length = pack - (byte *)pcx;
	if (!COM_WriteFile(filename, pcx, length))
	{
		free (pcx);
		return false;
	}

	free (pcx);
	return true;
}
*/
/*
==============
Image_LoadPCX - adapted from Quake 2
==============
*/
byte *Image_LoadPCX (FILE *fin, int fileLen, const char *filename, int matchwidth, int matchheight)
{
	pcx_t		pcx;
	byte		*data, *palette, *raw;
	byte		*out, *pix;
	int			x, y;
	int			dataByte, runLength;

	data = NULL;
	out = NULL;

	if (fread (&pcx, 1, sizeof(pcx_t), fin ) < sizeof(pcx_t))
	{
		goto ExitPoint;
	}

	//
	// parse the PCX file
	//

    pcx.xmin = LittleShort(pcx.xmin);
    pcx.ymin = LittleShort(pcx.ymin);
    pcx.xmax = LittleShort(pcx.xmax);
    pcx.ymax = LittleShort(pcx.ymax);
    pcx.hres = LittleShort(pcx.hres);
    pcx.vres = LittleShort(pcx.vres);
    pcx.bytes_per_line = LittleShort(pcx.bytes_per_line);
    pcx.palette_type = LittleShort(pcx.palette_type);

	if ((pcx.manufacturer != 0x0a) || (pcx.version != 5) || (pcx.encoding != 1) ||
		(pcx.bits_per_pixel != 8) /*|| (pcx.xmax >= 640) || (pcx.ymax >= 480)*/)
	{
		goto ExitPoint;
	}

	if ((matchwidth && (pcx.xmax+1 != matchwidth)) || (matchheight && (pcx.ymax+1 != matchheight)))
	{
		goto ExitPoint;
	}

	fileLen -= sizeof(pcx_t);
	data = Q_malloc (fileLen);
	if (fread (data, 1, fileLen, fin ) < fileLen)
	{
		goto ExitPoint;
	}

	palette = data + fileLen - 768;
	out = Q_calloc ((pcx.ymax+1) * (pcx.xmax+1) * 4, 1);
	pix = out;
	raw = data;

	image_width = pcx.xmax+1;
	image_height = pcx.ymax+1;

	for (y=0 ; y<=pcx.ymax ; y++, pix += (pcx.xmax+1)*4)
	{
		for (x=0 ; x<=pcx.xmax ; )
		{
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while (runLength-- > 0)
			{
				pix[4*x] = palette[3*dataByte];
				pix[4*x + 1] = palette[3*dataByte + 1];
				pix[4*x + 2] = palette[3*dataByte + 2];
				pix[4*x + 3] = 255;		// alpha
				x++;
			}
		}
	}

ExitPoint:
//	fclose (fin);
	if (data)
		free (data);
	if (!out)
		Con_DPrintf ("Image_LoadPCX: %s is not a valid PCX file.\n", filename);
	return (byte *) out;
}


typedef byte * (*IMG_LOAD_FUNC) (FILE *fin, int filelen, const char *filename, int matchwidth, int matchheight);

/*
===============
Image_LoadFile
===============
*/
byte *Image_LoadFile (FILE *f, int filelen, const char *filename)
{
	const char		*ext;
	IMG_LOAD_FUNC	func;

	ext = filename + strlen(filename) - 4;

	if (!Q_strcasecmp (ext, ".tga"))
		func = Image_LoadTGA;

	else if (!Q_strcasecmp (ext, ".png"))
		func = Image_LoadPNG;

	else if (!Q_strcasecmp (ext, ".jpg"))
		func = Image_LoadJPG;

	else if (!Q_strcasecmp (ext, ".pcx"))
		func = Image_LoadPCX;
	else
		return NULL;

	return func (f, filelen, filename, 0, 0);
}

typedef qboolean (*IMG_WRITE_FUNC) (const char *filename, int compression, const byte *pixels, int width, int height);

/*
===============
Image_WriteFile
  - assumes data is 24-bit RGB (3 bytes/pixel)
  - filename must NOT include a path; com_gamedir is used
===============
*/
qboolean Image_WriteFile (char *filename, const byte *data, int width, int height)
{
	char			*ext;
	IMG_WRITE_FUNC	func;
	int				compression;
	qboolean		bottomup;

	ext = COM_FileExtension (filename);

	if (!Q_strcasecmp(ext, "jpg") && jpg_available)
	{
		func = Image_WriteJPG;
		compression = jpeg_compression_level.value;
		bottomup = true;
	}
	else if (!Q_strcasecmp(ext, "png") && png_available)
	{
		func = Image_WritePNG;
		compression = png_compression_level.value;
		bottomup = true;
	}
	else
	{
		COM_ForceExtension (filename, ".tga", 999);		/***** FIXME: use proper bufsize *****/
		func = Image_WriteTGA;
		compression = 0;
		bottomup = false;
	}

	if (bottomup)
	{
		data += 3 * width * (height - 1);		// start of last row
		width = -width;
	}

	return func (filename, compression, data, width, height);
}

#endif		//#ifndef RQM_SV_ONLY
