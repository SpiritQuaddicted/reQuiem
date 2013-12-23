
#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "music.h"
#include "fmod.h"
#include "fmod_errors.h"

extern int sound_started;

static float           (F_API *qFSOUND_GetVersion)(void);
static signed char     (F_API *qFSOUND_Init)(int, int, unsigned int);
static int             (F_API *qFSOUND_GetError)(void);
static void            (F_API *qFSOUND_Close)(void);
static FMUSIC_MODULE * (F_API *qFMUSIC_LoadSongEx)(const char *, int, int, unsigned int, const int *, int);
static signed char     (F_API *qFMUSIC_FreeSong)(FMUSIC_MODULE *);
static signed char     (F_API *qFMUSIC_PlaySong)(FMUSIC_MODULE *);
static signed char     (F_API *qFMUSIC_StopSong)(FMUSIC_MODULE *);
static signed char     (F_API *qFMUSIC_SetPaused)(FMUSIC_MODULE *, signed char);

/*******JDH*******/
//static signed char     (F_API *qFSOUND_SetBufferSize)(int);
//static int             (F_API *qFSOUND_GetMixer)(void);
//static signed char     (F_API *qFSOUND_SetMixer)(int);

//static signed char     (F_API *qFSOUND_SetOutput)(int outputtype);
//static int             (F_API *qFMUSIC_GetMasterVolume)(FMUSIC_MODULE *);

// fallback if LoadSongEx is not found:
static FMUSIC_MODULE *   (F_API *qFMUSIC_LoadSongMemory)(const char *, int);

static signed char       (F_API *qFMUSIC_SetMasterVolume)(FMUSIC_MODULE *, int);
static void              (F_API *qFSOUND_SetSFXMasterVolume)(int volume);

static FSOUND_STREAM *   (F_API *qFSOUND_Stream_Open)(const char *name_or_data, unsigned int mode, int offset, int length);

//fallback if Stream_Open@16 is not found
static FSOUND_STREAM *   (F_API *qFSOUND_Stream_OpenFile)(const char *name_or_data, unsigned int mode, int length);

static signed char       (F_API *qFSOUND_Stream_Close)(FSOUND_STREAM *stream);
static int               (F_API *qFSOUND_Stream_Play)(int channel, FSOUND_STREAM *stream);
static signed char       (F_API *qFSOUND_Stream_Stop)(FSOUND_STREAM *stream);
static signed char       (F_API *qFSOUND_Stream_SetPosition)(FSOUND_STREAM *stream, unsigned int position);
static unsigned int      (F_API *qFSOUND_Stream_GetPosition)(FSOUND_STREAM *stream);

#ifdef HEXEN2_SUPPORT
  static signed char (F_API *qFMUSIC_SetLooping)(FMUSIC_MODULE *, signed char);

  static qboolean music_looped = true;
#endif
/*******JDH*******/

#ifdef _WIN32
  static	HINSTANCE fmod_handle = NULL;
#else
  #include <dlfcn.h>
  static	void	*fmod_handle = NULL;
#endif

static qboolean       music_fmod_loaded = false;
static qboolean       music_paused = false;
static FMUSIC_MODULE *music_mod = NULL;
static FSOUND_STREAM *music_stream = NULL;
static unsigned int	  music_stream_pos = 0;

#ifdef _WIN32
#  define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)GetProcAddress(fmod_handle, "_FSOUND_" #f #g))
#  define FMUSIC_GETFUNC(f, g) (qFMUSIC_##f = (void *)GetProcAddress(fmod_handle, "_FMUSIC_" #f #g))
#  define FMOD_LIBNAME1	"fmod375.dll"
#  define FMOD_LIBNAME2	"fmod.dll"
#else
#  define FSOUND_GETFUNC(f, g) (qFSOUND_##f = (void *)dlsym(fmod_handle, "FSOUND_" #f))
#  define FMUSIC_GETFUNC(f, g) (qFMUSIC_##f = (void *)dlsym(fmod_handle, "FMUSIC_" #f))
//#  ifdef MACOSX
//#    define FMOD_LIBNAME1	""
//#    define FMOD_LIBNAME2	""
//#  else
#    define FMOD_LIBNAME1	"libfmod-3.75.so"
#    define FMOD_LIBNAME2	"libfmod-3.73.so"
//#  endif
#endif

void Music_PlayMOD_f (cmd_source_t src);
void Music_PlayMP3_f (cmd_source_t src);

/*
==============
Music_UnloadFMOD
==============
*/
void Music_UnloadFMOD (void)
{
	if (fmod_handle)
	{
		COM_UnloadLibrary (fmod_handle);
		fmod_handle = NULL;
	}

	music_fmod_loaded = false;
}

/*
==============
Music_LoadFMOD
==============
*/
void Music_LoadFMOD (void)
{
	float fVersion = 0;

	music_fmod_loaded = false;

#ifdef MACOSX
	return;			// FIXME!!
#endif
	
	if (!(fmod_handle = COM_LoadLibrary(FMOD_LIBNAME1)) &&
	    !(fmod_handle = COM_LoadLibrary(FMOD_LIBNAME2)))
	{
		Con_Print ("\x02" "Failed to load FMOD! (not found)\n");
		goto fail;
	}

	FSOUND_GETFUNC(GetVersion, @0);
	FSOUND_GETFUNC(Init, @12);
	FSOUND_GETFUNC(GetError, @0);
	FSOUND_GETFUNC(Close, @0);

	FMUSIC_GETFUNC(LoadSongEx, @24);
	if (!qFMUSIC_LoadSongEx)
	{
		FMUSIC_GETFUNC(LoadSongMemory, @8);
	}

	FMUSIC_GETFUNC(FreeSong, @4);
	FMUSIC_GETFUNC(PlaySong, @4);
	FMUSIC_GETFUNC(StopSong, @4);
	FMUSIC_GETFUNC(SetPaused, @8);

/*******JDH*******/
//	FMUSIC_GETFUNC(GetMasterVolume, @4);
	FMUSIC_GETFUNC(SetMasterVolume, @8);
	FSOUND_GETFUNC(SetSFXMasterVolume, @4);

//	FSOUND_GETFUNC(SetBufferSize, @4);
//	FSOUND_GETFUNC(GetMixer, @0);
//	FSOUND_GETFUNC(SetMixer, @4);

	FSOUND_GETFUNC(Stream_Open, @16);
	if (!qFSOUND_Stream_Open)
	{
		FSOUND_GETFUNC(Stream_OpenFile, @12);
	}

	FSOUND_GETFUNC(Stream_Close, @4);
	FSOUND_GETFUNC(Stream_Play, @8);
	FSOUND_GETFUNC(Stream_Stop, @4);
	FSOUND_GETFUNC(Stream_SetPosition, @8);
	FSOUND_GETFUNC(Stream_GetPosition, @4);

#ifdef HEXEN2_SUPPORT
	FMUSIC_GETFUNC(SetLooping, @8);
#endif
	/*******JDH*******/

	music_fmod_loaded = qFSOUND_Init && /*qFSOUND_SetBufferSize && qFSOUND_GetMixer &&
			qFSOUND_SetMixer &&*/ qFSOUND_GetError && qFSOUND_Close && qFMUSIC_FreeSong &&
			(qFMUSIC_LoadSongEx || qFMUSIC_LoadSongMemory) && qFMUSIC_PlaySong &&
			qFMUSIC_StopSong && qFMUSIC_SetMasterVolume && qFSOUND_SetSFXMasterVolume &&
			(qFSOUND_Stream_Open || qFSOUND_Stream_OpenFile) && qFSOUND_Stream_Close &&
			qFSOUND_Stream_Play && qFMUSIC_SetPaused && qFSOUND_Stream_Stop &&
			qFSOUND_Stream_SetPosition && qFSOUND_Stream_GetPosition;
#ifdef HEXEN2_SUPPORT
//	music_fmod_loaded = music_fmod_loaded && qFMUSIC_SetLooping;		// no longer "fatal"
#endif

	if (!music_fmod_loaded)
	{
		if (qFSOUND_GetVersion)
			fVersion = qFSOUND_GetVersion();

		Con_Printf ("\x02" "Failed to load FMOD! (unsupported version%s)\n",
					(fVersion ? va(" %.2f", fVersion) : ""));
		goto fail;
	}

	music_mod = NULL;
	music_stream = NULL;
	return;

fail:
	if (fmod_handle)
		Music_UnloadFMOD();
}


/*
==============
Music_Init
==============
*/
qboolean Music_Init (void)
{
	float fVersion;
	int cmd_flags;

	if (music_fmod_loaded) return true;		// already initialized

	if (sound_started && !COM_CheckParm("-nomusic"))
	{
		Music_LoadFMOD ();
		if (music_fmod_loaded)
		{
		//	qFSOUND_SetBufferSize (300);

		#ifdef _WIN32
			if (!qFSOUND_Init(44100, 32, FSOUND_INIT_USEDEFAULTMIDISYNTH ))
		#else
			if (!qFSOUND_Init(44100, 32, 0 ))		// JDH: freq was 11025
		#endif
			{
				Con_Printf ("\x02" "Failed to load FMOD: %s\n", FMOD_ErrorString(qFSOUND_GetError()));
				qFSOUND_Close ();
				music_fmod_loaded = false;
			}
			else
			{
				fVersion = (qFSOUND_GetVersion ? qFSOUND_GetVersion() : 0);

				Con_Printf ("FMOD%s initialized\n", (fVersion ? va(" v%.2f", fVersion) : ""));
			}
		}
	}

// JDH: Add commands even if music isn't enabled, otherwise we'll get warnings
// if a map or demo tries to use them.

	cmd_flags = (music_fmod_loaded ? 0 : CMD_DISABLED);
// Nehahra
	Cmd_AddCommand ("stopmod", Music_Stop_f, cmd_flags);
	Cmd_AddCommand ("playmod", Music_PlayMOD_f, cmd_flags);

// ???? (used in nesp02)
	Cmd_AddCommand ("mp3", Music_PlayMP3_f, cmd_flags);
	
	return music_fmod_loaded;
}

/*
==============
Music_IsInitialized
==============
*/
qboolean Music_IsInitialized (void)
{
	return music_fmod_loaded;
}

/*
==============
Music_Close
==============
*/
void Music_Shutdown (void)
{
	if (music_fmod_loaded)
	{
		Music_Stop_f (SRC_COMMAND);
		qFSOUND_Close ();
		Music_UnloadFMOD ();
	}
}

/*
==============
OnChange_bgmvolume
==============
*/
qboolean OnChange_bgmvolume (cvar_t *var, const char *string)
{
	float value = Q_atof (string);

	Music_ChangeVolume (value);
	return false;		// allow change
}

/*
==============
Music_ChangeVolume
==============
*/
void Music_ChangeVolume (float value)
{
	if (music_mod)
		qFMUSIC_SetMasterVolume (music_mod, 256 * bound(0, value, 1));

	if (music_stream)
		qFSOUND_SetSFXMasterVolume (255 * bound(0, value, 1));
}

/*
==============
Music_Stop_f
==============
*/
void Music_Stop_f (cmd_source_t src)
{
	if (!music_fmod_loaded)
		return;
	
	// MOD/MIDI:
	if (music_mod)
	{
		qFMUSIC_StopSong (music_mod);
		qFMUSIC_FreeSong (music_mod);
		music_mod = NULL;
	}

	// MP3/OGG:
	if (music_stream)
	{
		qFSOUND_Stream_Close (music_stream);
		music_stream = NULL;
	}
}

/*
==============
Music_Pause_f
==============
*/
void Music_Pause_f (cmd_source_t src)
{
	if (music_mod)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			music_paused = !music_paused;		// in Hexen 2, "pause" is a toggle
		else
	#endif
			music_paused = true;

		qFMUSIC_SetPaused (music_mod, (char) music_paused);
	}

	if (music_stream && !music_paused)
	{
		music_paused = true;
		music_stream_pos = qFSOUND_Stream_GetPosition (music_stream);
		qFSOUND_Stream_Stop (music_stream);
	}
}

/*
==============
Music_Resume
==============
*/
void Music_Resume (void)
{
	if (!music_paused)
		return;

	if (music_mod)
	{
		music_paused = false;
		qFMUSIC_SetPaused (music_mod, (char) music_paused);
	}

	if (music_stream)
	{
		music_paused = false;
		qFSOUND_Stream_SetPosition (music_stream, music_stream_pos);
		qFSOUND_Stream_Play (FSOUND_FREE, music_stream);
	}
}


/*
==============
Music_LoadSong
==============
*/
FMUSIC_MODULE * Music_LoadSong (const char *buffer, int size)
{
	if (qFMUSIC_LoadSongEx)
		return qFMUSIC_LoadSongEx (buffer, 0, size, FSOUND_LOADMEMORY, NULL, 0);

	return qFMUSIC_LoadSongMemory (buffer, size);
}

/*
==============
Music_PlaySong - mod/midi
==============
*/
void Music_PlaySong (cmd_source_t src, const char *name)
{
	char *buffer;
	int	mark, err;

/*****JDH - note: this won't work if Quake is launched with "-primarysound" - JDH******/

	if (!music_fmod_loaded)
		return;

	Music_Stop_f (src);

	mark = Hunk_LowMark ();

	if (!(buffer = (char *)COM_LoadHunkFile(name, 0)))
	{
		Con_Printf ("ERROR: Couldn't open %s\n", name);
		return;
	}

	music_mod = Music_LoadSong (buffer, com_filesize);
	if (!music_mod)
	{
		// sometimes with MIDI, the first call to LoadSongEx fails
		err = qFSOUND_GetError();
		if (err == FMOD_ERR_FILE_FORMAT)
			music_mod = Music_LoadSong (buffer, com_filesize);
	}

	if (music_mod)
	{
		Music_ChangeVolume (bgmvolume.value);		// JDH

#ifdef HEXEN2_SUPPORT
		if (hexen2 && qFMUSIC_SetLooping)
			qFMUSIC_SetLooping (music_mod, (char) (music_looped ? 1 : 0));
		music_paused = false;
#endif

		qFMUSIC_PlaySong (music_mod);
		Con_Printf ("Playing audio track %s\n", name);
	}
	else
	{
		Con_Printf ("Music_PlaySong (%s): %s\n", name, FMOD_ErrorString(qFSOUND_GetError()));
	}

	Hunk_FreeToLowMark (mark);
}

static const char *music_paths[] = 
{
	"sound/cdtracks/",
	"music/",
	"music/cdtracks/",
	NULL
};

/*
==============
Music_PlaySound - mp3/ogg
==============
*/
qboolean Music_PlaySound (cmd_source_t src, const char *names[], qboolean loop)
{
	unsigned		mode;
	char			fpath[MAX_OSPATH];
	com_fileinfo_t	fileinfo;
	pack_t			*pak;
	int				filepos, filelen;
	const char		*dir;
	
	mode = FILE_ANY_MUS | FILE_NO_DZIPS;
	if (!qFSOUND_Stream_Open)
		mode |= FILE_NO_PAKS;
	
	if (!COM_FindMultifile (music_paths, names, mode, &fileinfo))
		return false;
	
	pak = fileinfo.searchpath->pack;
	if (pak)
	{
		Q_strcpy (fpath, pak->filename, sizeof(fpath));
		filepos = fileinfo.filepos;
		filelen = fileinfo.filelen;
	}
	else
	{
		Q_snprintfz (fpath, sizeof(fpath), "%s/%s", fileinfo.searchpath->filename, fileinfo.name);
		filepos = filelen = 0;
	}

	mode = (loop ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF);
	if (qFSOUND_Stream_Open)
	{
		music_stream = qFSOUND_Stream_Open (fpath, mode, filepos, filelen);
	}
	else if (filepos == 0)
	{
		music_stream = qFSOUND_Stream_OpenFile (fpath, mode, filelen);
	}
	else
		music_stream = NULL;

	if (!music_stream)
		return false;

	Music_ChangeVolume (bgmvolume.value);

	if (qFSOUND_Stream_Play (FSOUND_FREE, music_stream) == -1)
	{
		Music_Stop_f (src);
		return false;
	}

	if (pak)
		dir = pak->filename + strlen(com_basedir)+1;	// just gamedir+pak
	else
		dir = fileinfo.searchpath->dir_name;
	
	Con_Printf ("Playing audio track %s from %s\n", COM_SkipPath(fileinfo.name), dir);
	music_paused = false;
	return true;
}

/*
==============
Music_PlayMOD_f
==============
*/
void Music_PlayMOD_f (cmd_source_t src)
{
	static char	modname[MAX_OSPATH];

	if (!music_fmod_loaded)
		return;
	
	if (music_mod)
	{
		if (!Q_strcasecmp(modname, Cmd_Argv(1)))
			return;		// BJP: Ignore repeated play commands of the same song
	}

	Q_strcpy (modname, Cmd_Argv(1), sizeof(modname));

	if (strlen(modname) < 3)
	{
		Con_Printf ("Usage: %s <filename.ext>\n", Cmd_Argv(0));
		return;
	}

	Music_PlaySong (src, modname);
}

/*
==============
Music_PlayMP3_f
==============
*/
void Music_PlayMP3_f (cmd_source_t src)
{
	const char *command, *namelist[2];
	char filename[MAX_QPATH];

	if (!music_fmod_loaded)
		return;
	
	if (Cmd_Argc() >= 2)
	{
		command = Cmd_Argv (1);

		if (!Q_strcasecmp(command, "play"))
		{
			if (Cmd_Argc() >= 3)
			{
				//Q_snprintfz (filename, sizeof(filename), "music/%s", Cmd_Argv(2));
				Q_strcpy (filename, Cmd_Argv(2), sizeof(filename));
				COM_AddExtension (filename, ".mp3", sizeof(filename));
				namelist[0] = filename;
				namelist[1] = NULL;
				//Music_PlaySound (src, NULL, namelist, true);
				Music_PlaySound (src, namelist, true);
				return;
			}
		}

		if (!Q_strcasecmp(command, "stop"))
		{
			Music_Stop_f (src);
			return;
		}

		if (!Q_strcasecmp(command, "pause"))
		{
			Music_Pause_f (src);
			return;
		}

		if (!Q_strcasecmp(command, "resume"))
		{
			Music_Resume ();
			return;
		}
	}
	
	Con_Print ("Usage: mp3 <play/stop/pause/resume>\n");
}

/*
==============
Music_PlayTrack
  - plays an mp3/ogg instead of a CD audio track
==============
*/
/*static char *music_paths[] =
{
//	"music/T%02d.mp3",
//	"music/track%02d.mp3",
//	"cdtracks/T%02d.mp3",
	"sound/cdtracks/track%02d.mp3",		// DarkPlaces format
	"sound/cdtracks/track%03d.mp3",		// DarkPlaces format
//	"cdtracks/track%02d.mp3",			// DarkPlaces format
//	"cdtracks/track%03d.mp3",			// DarkPlaces format
	"sound/cdtracks/T%02d.mp3"		// Travail soundtrack
};


#define NUM_MUSIC_PATHS sizeof(music_paths)/sizeof(char *)


qboolean Music_PlayTrack (byte track, qboolean loop)
{
	unsigned int i, index, mode;
	char		 fname[MAX_QPATH];
	char		 fpath[MAX_OSPATH];
	searchpath_t *path;
	int			 filepos, filelen;

	if (!music_fmod_loaded)
		return false;

	Music_Stop();

	for (i = 0; i < NUM_MUSIC_PATHS; i++)
	{
		Q_snprintfz (fname, sizeof(fname), music_paths[i], (int) track);
		if ((path = COM_FindFile (fname, &index, FILE_ANY_MUS | FILE_NO_DZIPS)))
			break;
	}

	if (!path)
		return false;

	if (path->pack)
	{
		Q_snprintfz (fpath, sizeof(fpath), "%s", path->pack->filename);
		filepos = path->pack->files[index].filepos;
		filelen = path->pack->files[index].filelen;
	}
	else
	{
		Q_snprintfz (fpath, sizeof(fpath), "%s/%s", path->filename, fname);
		filepos = filelen = 0;
	}

	mode = (loop ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF);
	if (qFSOUND_Stream_Open)
	{
		music_stream = qFSOUND_Stream_Open (fpath, mode, filepos, filelen);
	}
	else if (filepos == 0)
	{
		music_stream = qFSOUND_Stream_OpenFile (fpath, mode, filelen);
	}
	else
		music_stream = NULL;

	if (!music_stream)
		return false;

	Music_ChangeVolume (bgmvolume.value);

	if (qFSOUND_Stream_Play (FSOUND_FREE, music_stream) == -1)
	{
		Music_Stop ();
		return false;
	}

	Con_Printf ("Playing audio track %s\n", fname);
	music_paused = false;
	return true;
}
*/

static const char *music_names[] =
{
	"track%02d",		// DarkPlaces format
	"track%03d",		// DarkPlaces format
	"T%02d"			// Travail soundtrack
};

#define NUM_MUSIC_NAMES sizeof(music_names)/sizeof(char *)

qboolean Music_PlayTrack (cmd_source_t src, byte track, qboolean loop)
{
	unsigned int	i;
	char			fnames[NUM_MUSIC_NAMES][MAX_QPATH];
	const char		*filelist[NUM_MUSIC_NAMES+1];

	if (!music_fmod_loaded)
		return false;

	Music_Stop_f (src);

	for (i = 0; i < NUM_MUSIC_NAMES; i++)
	{
		Q_snprintfz (fnames[i], sizeof(fnames[i]), music_names[i], (int) track);
		filelist[i] = fnames[i];
	}

	filelist[NUM_MUSIC_NAMES] = NULL;
	
	return Music_PlaySound (src, filelist, loop);
}

#ifdef HEXEN2_SUPPORT
/*
==============
Music_ChangeVolume_f
==============
*/
void Music_ChangeVolume_f (cmd_source_t src)
{
	if (Cmd_Argc () == 2)
	{
		Music_ChangeVolume ((float) atoi(Cmd_Argv(1)) / 100.0);
	}
	else
	{
//		Con_Printf("MIDI volume is %d\n", dwVolumePercent/(65535/100));
	}
}

/*
==============
Music_PlayMIDI
==============
*/
void Music_PlayMIDI (cmd_source_t src, const char *name)
{
	Music_PlaySong (src, va("midi/%s.mid", name));
}

/*
==============
Music_PlayMIDI_f
==============
*/
void Music_PlayMIDI_f (cmd_source_t src)
{
	if (Cmd_Argc() < 2)
	{
		Con_Printf ("Usage: %s <filename>\n", Cmd_Argv(0));
		return;
	}

	Music_PlayMIDI (src, Cmd_Argv(1));
}

/*
==============
Music_Loop
==============
*/
void Music_Loop (int newValue)
{
	if (newValue == 2)
		music_looped = !music_looped;
	else
		music_looped = (newValue ? 1 : 0);

	if (music_mod && qFMUSIC_SetLooping)
	{
		qFMUSIC_SetLooping (music_mod, (char) music_looped);
	}
}

/*
==============
Music_Loop_f
==============
*/
void Music_Loop_f (cmd_source_t src)
{
	if (Cmd_Argc () == 2)
	{
		if (!Q_strcasecmp (Cmd_Argv(1), "on") || !Q_strcasecmp (Cmd_Argv(1), "1"))
			Music_Loop(1);
		else if (!Q_strcasecmp (Cmd_Argv(1), "off") || !Q_strcasecmp (Cmd_Argv(1), "0"))
			Music_Loop(0);
		else if (!Q_strcasecmp (Cmd_Argv(1), "toggle"))
			Music_Loop(2);
	}

	Con_Printf ("Music will %sbe looped\n", (music_looped ? "" : "not "));
}

#endif	// #ifdef HEXEN2_SUPPORT

#endif		//#ifndef RQM_SV_ONLY
