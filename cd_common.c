
#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "music.h"

qboolean	cdValid = false;
qboolean	playing = false;
qboolean	enabled = false;
qboolean	playLooping = false;
byte		playTrack;

static qboolean	wasPlaying = false;
static qboolean	initialized = false;
static float	cdvolume;
static byte 	remap[100];
static byte		maxTrack;

/*
============
CDAudio_GetAudioDiskInfo
============
*/
static int CDAudio_GetAudioDiskInfo (void)
{
	int numtracks;

	cdValid = false;

	numtracks = CDAudio_GetNumTracks ();
	if (numtracks >= 0)
	{
		cdValid = true;
		maxTrack = numtracks;
		return 0;
	}

	return -1;
}

/*
============
CDAudio_Play
============
*/
void CDAudio_Play (cmd_source_t src, byte track, qboolean looping)
{
	int length;

	if (track <= 99)
		track = remap[track];
		
/******JDH******/
	if (Music_PlayTrack (src, track, looping))
		return;
/******JDH******/

	if (!enabled)
		return;

	if (track > 99)
	{
		Con_DPrintf ("CDAudio_Play: Bad track number %u.\n", track);
		return;
	}

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo ();
		if (!cdValid)
			return;
	}

	if (track < 1 || track > maxTrack)
	{
		Con_DPrintf ("CDAudio: Bad track number %u.\n", track);
		return;
	}

	// don't try to play a non-audio track
	length = CDAudio_GetTrackLength (track);
	if (length < 0)
		return;

	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop (src);
	}

	if (CDAudio_PlayTrack (track, length))
	{
		playLooping = looping;
		playTrack = track;
		playing = true;

		if (cdvolume == 0.0)
			CDAudio_Pause (src);
	}
}

/*
============
CDAudio_Update
============
*/
void CDAudio_Update (void)
{
	if (!enabled)
		return;

	if (bgmvolume.value != cdvolume)
	{
// JDH: this old behavior doesn't make sense now that fmod uses the bgmvolume cvar too
/*		if (cdvolume)
		{
			Cvar_SetValueDirect (&bgmvolume, 0.0);
			cdvolume = bgmvolume.value;
			CDAudio_Pause ();
		}
		else
		{
			Cvar_SetValueDirect (&bgmvolume, 1.0);
			cdvolume = bgmvolume.value;
			CDAudio_Resume ();
		}
*/
		if (cdvolume)
		{
			if (!bgmvolume.value)
				CDAudio_Pause (SRC_COMMAND);
		}
		else
		{
			CDAudio_Resume (SRC_COMMAND);
		}

		cdvolume = bgmvolume.value;
		CDAudio_UpdateVolume (cdvolume);
	}

#ifndef _WIN32
	CDAudio_Update_Linux ();
#endif
}


/*
============
CDAudio_Stop
============
*/
void CDAudio_Stop (cmd_source_t src)
{
	Music_Stop_f (src);		// JDH

	if (!enabled)
		return;

	if (!playing)
		return;

	if (CDAudio_StopDevice ())
	{
		wasPlaying = false;
		playing = false;
	}
}


/*
============
CDAudio_Pause
============
*/
void CDAudio_Pause (cmd_source_t src)
{
	Music_Pause_f (src);		// JDH

	if (!enabled)
		return;

	if (!playing)
		return;

	if (CDAudio_PauseDevice ())
	{
		wasPlaying = playing;
		playing = false;
	}
}


/*
============
CDAudio_Resume
============
*/
void CDAudio_Resume (cmd_source_t src)
{
	Music_Resume();		// JDH

	if (!enabled)
		return;

	if (!cdValid)
		return;

	if (!wasPlaying)
		return;

	if (CDAudio_ResumeDevice ())
		playing = true;
}

/*
============
CD_f
============
*/
static void CD_f (cmd_source_t src)
{
	const char	*command;
	int	ret, n;

	if (Cmd_Argc() < 2)
	{
		Con_Print ("usage: cd <play/stop/pause/resume/info/loop/eject/close/reset/remap/on/off>\n");
		return;
	}

	command = Cmd_Argv (1);

	if (!Q_strcasecmp(command, "on"))
	{
		enabled = true;
		return;
	}

	if (!Q_strcasecmp(command, "off"))
	{
		if (playing)
			CDAudio_Stop (src);
		enabled = false;
		return;
	}

	if (!Q_strcasecmp(command, "reset"))
	{
		enabled = true;
		if (playing)
			CDAudio_Stop (src);
		for (n=0 ; n<100 ; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo ();
		return;
	}

	if (!Q_strcasecmp(command, "remap"))
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			Con_Print ("usage: cd remap <t1> <t2> .. <tn>, where tx is the track number\n");
			Con_Print ("       to be used in place of track x (x = 1 to 99)\n");

			for (n=1 ; n<100 ; n++)
				if (remap[n] != n)
					Con_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n=1 ; (n<=ret) && (n<100) ; n++)
			remap[n] = Q_atoi(Cmd_Argv (n+1));

		return;
	}

	if (!Q_strcasecmp(command, "close"))
	{
		CDAudio_CloseDoor ();
		return;
	}

	if (!Q_strcasecmp(command, "play"))
	{
		CDAudio_Play (src, (byte) Q_atoi(Cmd_Argv (2)), false);
		return;
	}

	if (!Q_strcasecmp(command, "loop"))
	{
		CDAudio_Play (src, (byte) Q_atoi(Cmd_Argv (2)), true);
		return;
	}

	if (!Q_strcasecmp(command, "stop"))
	{
		CDAudio_Stop (src);
		return;
	}

	if (!Q_strcasecmp(command, "pause"))
	{
		CDAudio_Pause (src);
		return;
	}

	if (!Q_strcasecmp(command, "resume"))
	{
		CDAudio_Resume (src);
		return;
	}

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo ();
		if (!cdValid)
		{
			Con_Print ("No CD in player.\n");
			return;
		}
	}

	if (!Q_strcasecmp(command, "eject"))
	{
		if (playing)
			CDAudio_Stop (src);
		CDAudio_Eject ();
		cdValid = false;
		return;
	}

	if (!Q_strcasecmp(command, "info"))
	{
		Con_Printf ("%u tracks\n", maxTrack);
		if (playing)
			Con_Printf ("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Con_Printf ("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Con_Printf ("Volume is %f\n", cdvolume);

		return;
	}
}

/*
============
CDAudio_Init
============
*/
int CDAudio_Init (void)
{
	qboolean	nocdaudio;
	int			n;

	for (n=0 ; n<100 ; n++)
		remap[n] = n;

	if (cls.state == ca_dedicated)
		return -1;

	nocdaudio = COM_CheckParm("-nocdaudio");
	if (nocdaudio && COM_CheckParm("-nomusic"))
		return -1;

	// JDH: moved this up here now, since it works on mp3/ogg tracks too
	Cmd_AddCommand ("cd", CD_f, 0);

	if (nocdaudio)
		return -1;

	if (!CDAudio_InitDevice())
	{
		return -1;
	}

	initialized = true;
	enabled = true;

	if (CDAudio_GetAudioDiskInfo())
	{
//joe		Con_Print("CDAudio_Init: No CD in player.\n");
		cdValid = false;
	}

//joe	Con_Print("CD Audio Initialized\n");

	return 0;
}

/*
============
CDAudio_Shutdown
============
*/
void CDAudio_Shutdown (void)
{
	if (!initialized)
		return;

	CDAudio_Stop (SRC_COMMAND);
	CDAudio_CloseDevice ();
}

#endif		//#ifndef RQM_SV_ONLY
