/*
Copyright (C) 2002 Quake done Quick

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
// movie.c -- video capturing

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "movie.h"
#include "movie_avi.h"

extern	float	scr_con_current;
extern qboolean	scr_drawloading;
extern	short	*snd_out;
extern	int	snd_linear_count;

// Variables for buffering audio
//short	capture_audio_samples[44100];	// big enough buffer for 1fps at 44100Hz
static short	*capture_audio_samples;		// JDH: now dynamically allocated
static int		captured_audio_samples;

//static	int	out_size, ssize, outbuf_size;
//static	byte	*outbuf, *picture_buf;
//static	FILE	*moviefile;

qboolean OnChange_capturevar (cvar_t *var, const char *string);
//cvar_t	capture_dir	     = {"capture_dir",         "", CVAR_FLAG_ARCHIVE, OnChange_capturevar};
cvar_t	capture_fps	     = {"capture_fps",     "30.0", CVAR_FLAG_ARCHIVE, OnChange_capturevar};
cvar_t	capture_width    = {"capture_width",      "0", CVAR_FLAG_ARCHIVE, OnChange_capturevar};
cvar_t	capture_height   = {"capture_height",     "0", CVAR_FLAG_ARCHIVE, OnChange_capturevar};
cvar_t	capture_gameonly = {"capture_gameonly",   "1", CVAR_FLAG_ARCHIVE};

static qboolean movie_is_capturing = false;

void Movie_Start_f (cmd_source_t src);
void Movie_Stop_f (cmd_source_t src);
void Movie_CaptureDemo_f (cmd_source_t src);
void Movie_CaptureInfo_f (cmd_source_t src);
void Movie_CaptureConfig_f (cmd_source_t src);

void Movie_Init (void)
{
	if (Capture_InitAVI ())
	{
		Capture_InitACM ();

		Cmd_AddCommand ("capture_start", Movie_Start_f, 0);
		Cmd_AddCommand ("capture_stop", Movie_Stop_f, 0);
		Cmd_AddCommand ("capturedemo", Movie_CaptureDemo_f, 0);
		Cmd_AddCommand ("capture_info", Movie_CaptureInfo_f, 0);
		Cmd_AddCommand ("capcfg", Movie_CaptureConfig_f, 0);
	}

	captured_audio_samples = 0;

// Add cvars even if init failed, so any values loaded from cfg files
// will be preserved:

	Cvar_Register (&capture_fps);
//	Cvar_RegisterString (&capture_dir);
	Cvar_RegisterInt (&capture_width, 0, vid.width);
	Cvar_RegisterInt (&capture_height, 0, vid.height);
	Cvar_RegisterBool (&capture_gameonly);

//	Cvar_SetDirect (&capture_dir, va("%s/%s", com_basedir, capture_dir.string));

}

qboolean Movie_IsActive (void)
{
	if (capture_gameonly.value)
	{
		// don't output while console is down or 'loading' is displayed
//		if (scr_con_current > 0 || scr_drawloading)
		if (scr_con_current > 0 || scr_drawloading || (key_dest != key_game) ||
			(cls.demoplayback && (cl.paused & 2)) || (!cls.demoplayback && cl.paused))		// JDH
			return false;
	}

	// otherwise output if a file is open to write to
	return movie_is_capturing;
}

void Movie_CheckAspect (int width, int height)
{
	if ((float)width/vid.width != (float)height/vid.height)
	{
		Con_Print ("\x02""Warning:");
		Con_Print (" movie's width:height ratio does not match screen's\n\n");
	}
}

void Movie_CalcParams (int *width, int *height, float *fps)
{
	*width = capture_width.value;
	*height = capture_height.value;

	if (*width == 0 && *height != 0)
		*width = (int) (*height * (double)vid.width / ((double)vid.height/* * vid_pixelheight.value*/));
	if (*width != 0 && *height == 0)
		*height = (int) (*width * ((double)vid.height/* * vid_pixelheight.value*/) / (double)vid.width);

	if (*width < 2 || *width > vid.width) // can't scale up
		*width = vid.width;
	if (*height < 2 || *height > vid.height) // can't scale up
		*height = vid.height;

	// ensure it's all even; if not, scale down a little
/*	if (*width % 1)
		(*width)--;
	if (*height % 1)
		(*height)--;
*/
	*width &= ~1;
	*height &= ~1;

	*fps = bound(1, capture_fps.value, 1000);
}

void Movie_BeginCapture (const char *basename)
{
	char	name[MAX_QPATH], path[MAX_OSPATH];
	FILE	*moviefile;
	int		width, height;
	float	fps;

	Q_strcpy (name, basename, sizeof(name));
//	COM_AddExtension (name, ".avi", sizeof(name));
	COM_ForceExtension (name, ".avi", sizeof(name));		// 2011/05/21: so we don't get .dem.avi

	COM_ExpandPath (name, path, sizeof(path));

//	dir = (capture_dir.string[0] ? capture_dir.string : com_gamedir);
//	Q_snprintfz (path, sizeof(path), "%s/%s", com_gamedir, name);

	// JDH: check if the file already exists:
	if (CL_CheckExistingFile (path))
	{
		Con_Print ("Capture aborted.\n");
		return;
	}

	if (!(moviefile = fopen(path, "wb")))
	{
		COM_CreatePath (path);
		if (!(moviefile = fopen(path, "wb")))
		{
			Con_Printf ("ERROR: Couldn't open %s\n", name);
			return;
		}
	}

	fclose (moviefile);		// JDH: Capture_Open does its own fopen
	COM_DeleteFile (path);

	Movie_CalcParams (&width, &height, &fps);
	Movie_CheckAspect (width, height);

	movie_is_capturing = Capture_Open (path, width, height, fps);

	if (movie_is_capturing)
	{
		int rate = S_GetSoundRate();
		if (rate)
		{
			assert ((capture_audio_samples == NULL));
			capture_audio_samples = Q_malloc (rate * sizeof(short));
		}

		Con_Printf ("Capturing movie as %s\n", path);
	}
}

void Movie_Start_f (cmd_source_t src)
{
	char fname[MAX_OSPATH];

/*	if (Cmd_Argc() != 2)
	{
		Con_Print ("capture_start <filename> : Start capturing a movie to named file\n");
		return;
	}

	Movie_BeginCapture (Cmd_Argv(1));
*/
	if (Cmd_Argc() >= 2)
		Q_strcpy (fname, Cmd_Argv(1), sizeof(fname));
	else
		CL_MakeRecordingName (fname, sizeof(fname));

	Movie_BeginCapture (fname);
}

void Movie_Stop (void)
{
	if (movie_is_capturing)
	{
		if (cls.capturedemo)
			Movie_StopDemoCapture ();
		else
			Capture_Close ();

		movie_is_capturing = false;
		free (capture_audio_samples);
		capture_audio_samples = NULL;

	//	fclose (moviefile);		// JDH
	}
}

void Movie_Stop_f (cmd_source_t src)
{
	if (!movie_is_capturing)
	{
		Con_Print ("Not capturing\n");
		return;
	}

	Movie_Stop ();

	Con_Print ("Stopped capturing\n\n");
}

extern const char * CL_GetDemoFilepath (void);

void Movie_CaptureDemo_f (cmd_source_t src)
{
	int  argc;
	const char *movie_name;
#if 1
	char demopath[MAX_OSPATH];
	FILE *demofile;
	extern FILE * CL_OpenDemo (const char *name, char *filepathbuf, int bufsize);
#endif

	argc = Cmd_Argc ();

	if ((argc > 3) || ((argc == 1) && !cls.demoplayback))
	{
		Con_Print ("capturedemo <demo_name> [[path/]movie_name[.avi]]: creates a AVI movie of given demo\n");
		return;
	}

	if (argc > 1)
	{
//		Con_Printf ("Capturing %s.dem\n", Cmd_Argv(1));

#if 0		// 2011/05/23
		CL_StartPlayback (Cmd_Argv(1));
		if (!cls.demoplayback)
			return;
#else
		demofile = CL_OpenDemo (Cmd_Argv(1), demopath, sizeof(demopath));
		if (demofile)
			fclose (demofile);
		else
		{
			Con_Printf ("ERROR: couldn't open %s\n", Cmd_Argv(1));
			return;
		}
#endif
	}

	if (argc == 3)
		movie_name = Cmd_Argv (2);
	else if (argc == 2)
		movie_name = COM_SkipPath (demopath);
	else
		movie_name = COM_SkipPath (CL_GetDemoFilepath());

	Movie_BeginCapture (movie_name);

#if 0		// 2011/05/23
	cls.capturedemo = true;

	if (!movie_is_capturing)
		Movie_StopDemoCapture ();
#else
	if (movie_is_capturing)
	{
		if (argc > 1)
			CL_StartPlayback (Cmd_Argv(1));

		if (cls.demoplayback)
		{
			cls.capturedemo = true;
			return;
		}
	}

	Movie_Stop ();
#endif
}

typedef struct movie_cfg_s
{
	int width, height;
	float fps;
} movie_cfg_t;


int Movie_TestFPS (const movie_cfg_t *config)
{
	int		len, i;
	char	path[MAX_OSPATH];
	double	time1;
	float	olddev;
#ifdef _WIN32
	extern	int vid_refreshrate;		// FIXME: should use actual refresh
	extern	cvar_t vid_vsync;
#endif

	if (movie_is_capturing)
		return 0;

	len = Q_snprintfz (path, sizeof(path), "%s/RQMAVI", com_gamedir);
	for (i = 0; i < 1000; i++)
	{
		Q_snprintfz (path+len, sizeof(path)-len, "%04d", i);
		if (!COM_FileExists (path))
			break;
	}
	if (i == 1000)
		return 0;

	movie_is_capturing = Capture_Open (path, config->width, config->height, config->fps);
	if (!movie_is_capturing)
		return 0;

	movie_is_capturing = false;		// so frame capture doesn't happen in SCR_UpdateScreen
	time1 = Sys_DoubleTime ();

	for (i = 0; i < 5; i++)
	{
		SCR_UpdateScreen ();
		if (!Capture_WriteVideo ())
			break;
	}
	if (i)
		time1 = (1.0 / (Sys_DoubleTime () - time1)) * (float) i;
	else
		time1 = 0;

//	movie_is_capturing = true;

	olddev = developer.value;
	developer.value = 0;		// otherwise we get status report
	Capture_Close ();
	developer.value = olddev;

//	movie_is_capturing = false;
	COM_DeleteFile (path);

#ifdef _WIN32
	if (vid_vsync.value && (time1 > vid_refreshrate))
		time1 = vid_refreshrate;
#endif

	return (int)time1;
}

movie_cfg_t * Movie_PrintSettings (void)
{
	static const cvar_t *cvars[] = {&capture_width, &capture_height, &capture_fps, &capture_gameonly};
	static movie_cfg_t config;
	int i;

	Con_Print ("\nCurrent settings:\n");
	for (i = 0; i < sizeof(cvars)/sizeof(cvars[0]); i++)
		Con_Printf ("  %-18s: %6s\n", cvars[i]->name, cvars[i]->string);

	Movie_CalcParams (&config.width, &config.height, &config.fps);

	Con_Print ("\n");
	if ((config.width != capture_width.value) || (config.height != capture_height.value) ||
		(config.fps != capture_fps.value))
	{
		Con_Printf ("Movie will be captured at %dx%d @ %.1f fps\n\n",
						config.width, config.height, config.fps);
	}

	Movie_CheckAspect (config.width, config.height);
	return &config;
}

void Movie_CaptureInfo_f (cmd_source_t src)
{
	movie_cfg_t *config;
	float size_per_frame, test_fps;

	config = Movie_PrintSettings ();

// FIXME: 1.5 multiplier assumes i420 codec

	size_per_frame = ((config->width * config->height * 1.5) * config->fps) / (1024*1024);
		// (audio data doesn't affect size much)

	Con_Print ("Disk space needed: about ");
	if (size_per_frame < 10)
		Con_Printf ("%.1f", size_per_frame);
	else
		Con_Printf ("%d", (int)(size_per_frame + 0.5));
	Con_Print (" MB per second (");

	size_per_frame *= 60.0;
	if (size_per_frame >= 1024)
		Con_Printf ("%.1f GB", size_per_frame/1024);
	else
		Con_Printf ("%d MB", (int)(size_per_frame + 0.5));
	Con_Print (" per minute).\n");

	if (capture_gameonly.value)
		Con_Print ("Capture will pause when in menu or console, and when game is paused.\n");
	Con_Print ("\n");

	test_fps = Movie_TestFPS (config);
	if (test_fps > 0)
		Con_Printf ("With these settings, your system can generate about %d frames per second.\n\n", (int)test_fps);

	if (test_fps < 30)
	{
		Con_Print ("To improve performance:\n");
		Con_Print ("  - run the game at a lower resolution\n");
		Con_Print ("  - reduce the movie's width & height\n");
		Con_Print ("  - save the movie to a faster (or less full) drive\n\n");
	}
}

void Movie_CaptureConfig_f (cmd_source_t src)
{
	int argc;

	argc = Cmd_Argc ();
	switch (argc)
	{
		default:
		case 4:
			Cvar_SetDirect (&capture_fps, Cmd_Argv(3));
		case 3:
			Cvar_SetDirect (&capture_height, Cmd_Argv(2));
		case 2:
			Cvar_SetDirect (&capture_width, Cmd_Argv(1));
			break;

		case 1:
			Con_Printf ("Usage: %s width [height [fps]]\n", Cmd_Argv(0));
			Movie_PrintSettings ();
			Con_Print ("For more detailed information, enter \"capture_info\"\n" );
			return;
	}
}

void Movie_StopDemoCapture (void)
{
	if (!cls.capturedemo)
		return;

	cls.capturedemo = false;
	Capture_Close ();
	movie_is_capturing = false;
	CL_Disconnect (false);
}

void Movie_UpdateScreen (void)
{
#ifdef _DEBUG
	double time1;
#endif

	if (!Movie_IsActive())
		return;

#ifdef _DEBUG
	time1 = Sys_DoubleTime ();
#endif

	if (!Capture_WriteVideo ())
		Movie_Stop ();

#ifdef _DEBUG
	time1 = Sys_DoubleTime () - time1;
	return;
#endif
}

byte * Movie_ScaleDownBGR (const byte *in, int inw, int inh, byte *out, int outw, int outh)
{
	// TODO optimize this function

	int x, y;
	float area;

	if (inw == outw && inh == outh)
	{
		return (byte *) in;
	}

	// otherwise: a box filter
	area = (float)outw * (float)outh / (float)inw / (float)inh;
	for(y = 0; y < outh; ++y)
	{
		float iny0 =  y    / (float)outh * inh; int iny0_i = floor(iny0);
		float iny1 = (y+1) / (float)outh * inh; int iny1_i = ceil(iny1);
		for(x = 0; x < outw; ++x)
		{
			float inx0 =  x    / (float)outw * inw; int inx0_i = floor(inx0);
			float inx1 = (x+1) / (float)outw * inw; int inx1_i = ceil(inx1);
			float b = 0, g = 0, r = 0;
			int xx, yy;

			for(yy = iny0_i; yy < iny1_i; ++yy)
			{
				float ya = min(yy+1, iny1) - max(iny0, yy);
				for(xx = inx0_i; xx < inx1_i; ++xx)
				{
					float a = ya * (min(xx+1, inx1) - max(inx0, xx));
					b += a * in[3*(xx + inw * yy)+0];
					g += a * in[3*(xx + inw * yy)+1];
					r += a * in[3*(xx + inw * yy)+2];
				}
			}

			out[3*(x + outw * y)+0] = b * area;
			out[3*(x + outw * y)+1] = g * area;
			out[3*(x + outw * y)+2] = r * area;
		}
	}

	return out;
}

double Movie_FrameTime (void)
{
	if (!movie_is_capturing)
		return 0;

	return Capture_FrameTime ();
}

qboolean Movie_GetSoundtime (void)
{
	if (!Movie_IsActive())
		return false;

	return Capture_GetSoundtime ();
}

void Movie_TransferStereo16 (short *data, int len)
{
	if (!Movie_IsActive())
		return;

	assert (((captured_audio_samples << 1) + len * shm->channels <= shm->speed * sizeof(short)));

	// Copy last audio chunk written into our temporary buffer
	memcpy (capture_audio_samples + (captured_audio_samples << 1), data, len * shm->channels);
	captured_audio_samples += (len >> 1);

	// JDH: since host_frametime is now affected by cl_demospeed, it no longer
	//      is equal to the movie frametime
//	if (captured_audio_samples >= (int)(0.5 + host_frametime * shm->speed))
	if (captured_audio_samples >= (int)(0.5 + Capture_FrameTime() * shm->speed))
	{
		// We have enough audio samples to match one frame of video
		if (!Capture_WriteAudio ((byte *)capture_audio_samples, captured_audio_samples))
			Movie_Stop ();

		captured_audio_samples = 0;
	}
}

qboolean OnChange_capturevar (cvar_t *var, const char *string)
{
	if (movie_is_capturing)
	{
		//Con_Print ("Cannot change capture_dir whilst capturing. Use 'capture_stop' to cease capturing first.\n");
		//return true;
		Con_Printf ("Change to %s will not affect movie currently being recorded.\n", var->name);
	}

	return false;
}

void Movie_UpdateColorRamps (void)
{
	if (movie_is_capturing)
		Capture_UpdateRamps ();
}

#endif		//#ifndef RQM_SV_ONLY
