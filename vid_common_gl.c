/*
Copyright (C) 2002-2003 A Nourai

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
// vid_common_gl.c -- Common code for vid_wgl.c and vid_glx.c

#include "quakedef.h"

#ifdef _WIN32
#  define qglGetProcAddress wglGetProcAddress
#else
#  include <GL/glx.h>
#  define qglGetProcAddress glXGetProcAddressARB
#endif

typedef int (APIENTRY * lpSwapFUNC) (int);
#ifndef _WIN32
  typedef void (APIENTRY * lpSwapFUNC2) (Display *, GLXDrawable, int);		// for glXSwapIntervalEXT
#endif

const	char	*gl_vendor;
const	char	*gl_renderer;
const	char	*gl_version;
const	char	*gl_extensions;

qboolean	gl_mtexable = false;
int			gl_textureunits = 1;

lpMTexFUNC		qglMultiTexCoord2f = NULL;
lpSelTexFUNC	qglActiveTexture = NULL;

qboolean	gl_add_ext = false;
qboolean	gl_combine_support = false;

//float		gldepthmin, gldepthmax;

float		vid_gamma = 1.0;
byte		vid_gamma_table[256];

unsigned	d_8to24table[256];
unsigned	d_8to24table2[256];

lpSwapFUNC	qglSwapInterval = NULL;
#ifndef _WIN32
	lpSwapFUNC2	qglSwapInterval2 = NULL;		// used only if other pointer is null
#endif

qboolean OnChange_vsync (cvar_t *var, const char *value);

cvar_t		vid_vsync = {"vid_vsync", "0", CVAR_FLAG_ARCHIVE, OnChange_vsync};

static const char *VSYNC_PROCS[] =
{
#ifdef _WIN32
	"wglSwapIntervalEXT",
	"wglSwapInterval",
	"wglSwapIntervalSGI",
#else
	"glXSwapInterval",
	"glXSwapIntervalSGI",
//	"glXSwapIntervalEXT",
#endif
	"glSwapIntervalEXT"
};

#define NUM_VSYNC_PROCS (sizeof(VSYNC_PROCS)/sizeof(char *))


#ifdef HEXEN2_SUPPORT
unsigned	d_8to24TranslucentTable[256];
float		RTint[256], GTint[256], BTint[256];

int ColorIndex[16] =
{
	0, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 199, 207, 223, 231
};

unsigned ColorPercent[16] =
{
	25, 51, 76, 102, 114, 127, 140, 153, 165, 178, 191, 204, 216, 229, 237, 247
};
#endif

extern qboolean	fullsbardraw;


qboolean GL_SetSwapInterval (int interval)
{
	if (qglSwapInterval)
		return qglSwapInterval (interval);

#ifndef _WIN32
	if (qglSwapInterval2)
	{
		GLXDrawable drawable = glXGetCurrentDrawable ();
		if (drawable)
		{
			extern Display *dpy;
			qglSwapInterval2 (dpy, drawable, interval);
			return true;
		}
	}
#endif

	return false;
}

qboolean CheckExtension (const char *extension)
{
	const	char	*start;
	char		*where, *terminator;

	if (!gl_extensions && !(gl_extensions = (char *) glGetString(GL_EXTENSIONS)))
		return false;

	if (!extension || *extension == 0 || strchr(extension, ' '))
		return false;

	for (start = gl_extensions ; (where = strstr(start, extension)) ; start = terminator)
	{
		terminator = where + strlen(extension);
		if ((where == start || *(where - 1) == ' ') && (*terminator == 0 || *terminator == ' '))
			return true;
	}

	return false;
}

void CheckMultiTextureExtensions (void)
{
	if (!COM_CheckParm("-nomtex") && CheckExtension("GL_ARB_multitexture"))
	{
		if (strstr(gl_renderer, "Savage"))
			return;
		qglMultiTexCoord2f = (void *)qglGetProcAddress ((byte *) "glMultiTexCoord2fARB");
		qglActiveTexture = (void *)qglGetProcAddress ((byte *) "glActiveTextureARB");
		if (!qglMultiTexCoord2f || !qglActiveTexture)
			return;
		Con_Print ("Using GL_ARB_multitexture\n");
		gl_mtexable = true;
	}

	glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &gl_textureunits);
	gl_textureunits = min(gl_textureunits, MAX_TMUS);

/************JDH************/
	//if (COM_CheckParm("-maxtmu2") || !strcmp(gl_vendor, "ATI Technologies Inc."))
	if (COM_CheckParm("-maxtmu2"))
/************JDH************/
		gl_textureunits = min(gl_textureunits, 2);

	if (gl_textureunits < 2)
		gl_mtexable = false;

	if (!gl_mtexable)
		gl_textureunits = 1;
	else
		Con_Printf ("  Enabled %i texture units\n", gl_textureunits);
//	Con_Print ("\n");
}

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	int i;

	Cvar_RegisterBool (&vid_vsync);

	Con_Print ("\n");
	gl_vendor = (char *) glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = (char *) glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);
	gl_version = (char *) glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = (char *) glGetString (GL_EXTENSIONS);
	if (COM_CheckParm("-gl_ext"))
		Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);		// JT030605 - prevent listing of all extensions on init

	if (!Q_strncasecmp(gl_renderer, "PowerVR", 7))
		fullsbardraw = true;

	glDepthRange (0, 1);		// JDH: these are fixed now that gl_ztrick is gone
	glDepthFunc (GL_LEQUAL);

	glCullFace (GL_FRONT);
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_ALPHA_TEST);
	glAlphaFunc (GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		--> this is default anyway
//	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	CheckMultiTextureExtensions ();

	gl_add_ext = CheckExtension ("GL_ARB_texture_env_add");
	if (gl_add_ext)
		Con_Print ("Using GL_ARB_texture_env_add\n");

	gl_combine_support = CheckExtension ("GL_ARB_texture_env_combine");
	if (gl_combine_support)
		Con_Print ("Using GL_ARB_texture_env_combine\n");
	else
	{
		gl_combine_support = CheckExtension ("GL_EXT_texture_env_combine");
		if (gl_combine_support)
			Con_Print ("Using GL_EXT_texture_env_combine\n");
	}

// JDH: vsync
#ifdef _WIN32
	if (CheckExtension ("GL_EXT_swap_control") || CheckExtension ("WGL_EXT_swap_control"))
		// note: Linux doesn't reliably return an extension, even though a proc might exist
#endif
	{
		for (i = 0; i < NUM_VSYNC_PROCS; i++)
		{
			qglSwapInterval = (lpSwapFUNC) qglGetProcAddress ((byte *) VSYNC_PROCS[i]);
			if (qglSwapInterval)
			{
				Con_Printf ("Using %s\n", VSYNC_PROCS[i]);
				break;
			}
		}
#ifndef _WIN32
		if (!qglSwapInterval)
		{
			qglSwapInterval2 = (lpSwapFUNC2) qglGetProcAddress ((byte *) "glXSwapIntervalEXT");
			if (qglSwapInterval2)
			{
				Con_Printf ("Using %s\n", "glXSwapIntervalEXT");
			}
		}
#endif
		GL_SetSwapInterval (vid_vsync.value);		// init to default value
	}

	glClearColor (0, 0, 0, 0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor (1, 0, 0, 0);
}

/*void VID_ApplyGammaToPalette (unsigned char *palette)
{
	unsigned char	pal[768];
	int i;

	for (i=0 ; i<768 ; i++)
		pal[i] = vid_gamma_table[palette[i]];

	memcpy (palette, pal, sizeof(palette));
}
*/

void VID_BuildGammaTable (void)
{
	float	inf;
	int		i;

	if ((i = COM_CheckParm("-gamma")) && (i+1 < com_argc))
		vid_gamma = bound(0.3, Q_atof(com_argv[i+1]), 1);
	else
		vid_gamma = 1;

	Cvar_SetValueDirect (&v_gamma, vid_gamma);

	if (vid_gamma != 1)
	{
		for (i=0 ; i<256 ; i++)
		{
			inf = min(255 * pow((i + 0.5) / 255.5, vid_gamma) + 0.5, 255);
			vid_gamma_table[i] = inf;
		}
	}
	else
	{
		for (i=0 ; i<256 ; i++)
			vid_gamma_table[i] = i;
	}

//	VID_ApplyGammaToPalette ();
}

void VID_SetPalette (unsigned char *palette)
{
	byte		*pal;
	int			i;
	unsigned	r, g, b, *table;
#ifdef HEXEN2_SUPPORT
	int			c, p, v;
#endif

	for (i=0 ; i<768 ; i++)
		palette[i] = vid_gamma_table[palette[i]];

// 8 8 8 encoding
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<255 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		*table++ = (255<<24) + (r<<0) + (g<<8) + (b<<16);
	}
	d_8to24table[255] = 0;	// 255 is transparent

// Tonik: create a brighter palette for bmodel textures
	pal = palette;
	table = d_8to24table2;
	for (i=0 ; i<255 ; i++)
	{
		r = min(pal[0] * (2.0 / 1.5), 255);
		g = min(pal[1] * (2.0 / 1.5), 255);
		b = min(pal[2] * (2.0 / 1.5), 255);
		pal += 3;
		*table++ = (255<<24) + (r<<0) + (g<<8) + (b<<16);
	}
	d_8to24table2[255] = 0;	// 255 is transparent

#ifdef HEXEN2_SUPPORT
	pal = palette;
	table = d_8to24TranslucentTable;

	for (i=0; i<16;i++)
	{
		c = ColorIndex[i]*3;

		r = pal[c];
		g = pal[c+1];
		b = pal[c+2];

		for(p=0;p<16;p++)
		{
			v = (ColorPercent[15-p]<<24) + (r<<0) + (g<<8) + (b<<16);
			//v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
			*table++ = v;

			RTint[i*16+p] = ((float)r) / ((float)ColorPercent[15-p]) ;
			GTint[i*16+p] = ((float)g) / ((float)ColorPercent[15-p]);
			BTint[i*16+p] = ((float)b) / ((float)ColorPercent[15-p]);
		}
	}
#endif
}

/*
================
OnChange_vsync
================
*/
qboolean OnChange_vsync (cvar_t *var, const char *value)
{
	int v;

	v = (int) Q_atof (value);
	if (GL_SetSwapInterval (v))
		return false;					// allow change

	Con_Print ("Your video card/drivers don't allow vsync to be changed\n");
	return false;			// allow cvar to change, even though it won't affect anything
}

