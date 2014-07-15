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
// cd_win.c

#include <windows.h>
#include "quakedef.h"

#ifndef RQM_SV_ONLY

extern qboolean cdValid;
extern qboolean	playing;
extern byte		playTrack;
extern qboolean playLooping;

extern	HWND	mainwindow;
extern	cvar_t	bgmvolume;

static UINT		wDeviceID;
static float	cd_initialvol = 1.0;

/*
============
CDAudio_Eject
============
*/
void CDAudio_Eject (void)
{
	DWORD	dwReturn;

	if ((dwReturn = mciSendCommandA(wDeviceID, MCI_SET, MCI_SET_DOOR_OPEN, (DWORD)NULL)))
		Con_DPrintf ("MCI_SET_DOOR_OPEN failed (%i)\n", dwReturn);
}

/*
============
CDAudio_CloseDoor
============
*/
void CDAudio_CloseDoor (void)
{
	DWORD	dwReturn;

	if ((dwReturn = mciSendCommandA(wDeviceID, MCI_SET, MCI_SET_DOOR_CLOSED, (DWORD)NULL)))
		Con_DPrintf ("MCI_SET_DOOR_CLOSED failed (%i)\n", dwReturn);
}

/*
============
CDAudio_GetNumTracks
============
*/
int CDAudio_GetNumTracks (void)
{
	DWORD			dwReturn;
	MCI_STATUS_PARMS	mciStatusParms;

	mciStatusParms.dwItem = MCI_STATUS_READY;
	dwReturn = mciSendCommandA (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD)(LPVOID)&mciStatusParms);
	if (dwReturn)
	{
		Con_DPrintf ("CDAudio: drive ready test - get status failed\n");
		return -1;
	}
	if (!mciStatusParms.dwReturn)
	{
		Con_DPrintf ("CDAudio: drive not ready\n");
		return -1;
	}

	mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	dwReturn = mciSendCommandA (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD)(LPVOID)&mciStatusParms);
	if (dwReturn)
	{
		Con_DPrintf ("CDAudio: get tracks - status failed\n");
		return -1;
	}
	if (mciStatusParms.dwReturn < 1)
	{
		Con_DPrintf ("CDAudio: no music tracks\n");
		return -1;
	}

	return mciStatusParms.dwReturn;
}

/*
============
CDAudio_GetTrackLength
============
*/
int CDAudio_GetTrackLength (byte track)
{
	DWORD				dwReturn;
	MCI_STATUS_PARMS	mciStatusParms;

	mciStatusParms.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
	mciStatusParms.dwTrack = track;
	dwReturn = mciSendCommandA (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD)(LPVOID)&mciStatusParms);
	if (dwReturn)
	{
		Con_DPrintf ("MCI_STATUS failed (%i)\n", dwReturn);
		return -1;
	}
	if (mciStatusParms.dwReturn != MCI_CDA_TRACK_AUDIO)
	{
		Con_Printf ("CDAudio: track %i is not audio\n", track);
		return -1;
	}

	// get the length of the track to be played
	mciStatusParms.dwItem = MCI_STATUS_LENGTH;
	mciStatusParms.dwTrack = track;
	dwReturn = mciSendCommandA (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD)(LPVOID)&mciStatusParms);
	if (dwReturn)
	{
		Con_DPrintf ("MCI_STATUS failed (%i)\n", dwReturn);
		return -1;
	}

	return mciStatusParms.dwReturn;
}

/*
============
CDAudio_PlayTrack
============
*/
qboolean CDAudio_PlayTrack (byte track, int length)
{
	DWORD				dwReturn;
	MCI_PLAY_PARMS		mciPlayParms;

	mciPlayParms.dwFrom = MCI_MAKE_TMSF(track, 0, 0, 0);
	mciPlayParms.dwTo = (length << 8) | track;
	mciPlayParms.dwCallback = (DWORD)mainwindow;
	dwReturn = mciSendCommandA (wDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_FROM | MCI_TO, (DWORD)(LPVOID)&mciPlayParms);
	if (dwReturn)
	{
		Con_DPrintf ("CDAudio: MCI_PLAY failed (%i)\n", dwReturn);
		return false;
	}

	return true;
}

/*
============
CDAudio_StopDevice
============
*/
qboolean CDAudio_StopDevice (void)
{
	DWORD	dwReturn;

	if ((dwReturn = mciSendCommandA(wDeviceID, MCI_STOP, 0, (DWORD)NULL)))
	{
		Con_DPrintf ("MCI_STOP failed (%i)", dwReturn);
		return false;
	}

	return true;
}

/*
============
CDAudio_PauseDevice
============
*/
qboolean CDAudio_PauseDevice (void)
{
	DWORD			dwReturn;
	MCI_GENERIC_PARMS	mciGenericParms;

	mciGenericParms.dwCallback = (DWORD)mainwindow;
	if ((dwReturn = mciSendCommandA(wDeviceID, MCI_PAUSE, 0, (DWORD)(LPVOID)&mciGenericParms)))
	{
		Con_DPrintf ("MCI_PAUSE failed (%i)", dwReturn);
		return false;
	}

	return true;
}

/*
============
CDAudio_ResumeDevice
============
*/
qboolean CDAudio_ResumeDevice (void)
{
	DWORD		dwReturn;
	MCI_PLAY_PARMS	mciPlayParms;
	
	mciPlayParms.dwFrom = MCI_MAKE_TMSF(playTrack, 0, 0, 0);
	mciPlayParms.dwTo = MCI_MAKE_TMSF(playTrack + 1, 0, 0, 0);
	mciPlayParms.dwCallback = (DWORD)mainwindow;
	dwReturn = mciSendCommandA (wDeviceID, MCI_PLAY, MCI_TO | MCI_NOTIFY, (DWORD)(LPVOID)&mciPlayParms);
	if (dwReturn)
	{
		Con_DPrintf ("CDAudio: MCI_PLAY failed (%i)\n", dwReturn);
		return false;
	}
	
	return true;
}

typedef enum {CDDA_GETVOL, CDDA_SETVOL} CDDA_OP;

/*
============
CDAudio_AccessVolume
  JDH: get/set CD volume
       (code adpated from cd_player.c in avp package at icculus.org)
============
*/
void CDAudio_AccessVolume (float *vol, CDDA_OP op)
{
	int					numDevs, i;
	HMIXEROBJ			hMixer;
	MIXERLINEA			line;
	MIXERLINECONTROLSA	lineControls;
	MIXERCONTROL		control;
	MIXERCONTROLDETAILS details;
	MIXERCONTROLDETAILS_UNSIGNED detailValue;
	DWORD				range;
	
	numDevs = mixerGetNumDevs ();
	for (i = 0; i < numDevs; i++)
	{
		if (mixerOpen ((HMIXER *)&hMixer, i, 0, 0, MIXER_OBJECTF_MIXER) != MMSYSERR_NOERROR)
			continue;

		line.cbStruct = sizeof(MIXERLINE);
		line.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

		if (mixerGetLineInfoA (hMixer, &line, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
		{
			control.cbStruct = sizeof(MIXERCONTROL);
			
			lineControls.cbStruct = sizeof(MIXERLINECONTROLS);
			lineControls.dwLineID = line.dwLineID;
			lineControls.pamxctrl = &control;
			lineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			lineControls.cControls = 1;
			lineControls.cbmxctrl = sizeof(MIXERCONTROL);
			
			if (mixerGetLineControlsA (hMixer, &lineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR)
			{
				details.cbStruct = sizeof(MIXERCONTROLDETAILS);
				details.dwControlID = control.dwControlID;
				details.cChannels = 1;
				details.cMultipleItems = 0;
				details.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
				details.paDetails = &detailValue;

				range = control.Bounds.dwMaximum - control.Bounds.dwMinimum;
				if (op == CDDA_SETVOL)
				{
					detailValue.dwValue = control.Bounds.dwMinimum + (*vol * range);
				
					mixerSetControlDetails (hMixer, &details, MIXER_SETCONTROLDETAILSF_VALUE);
				}
				else
				{
					mixerGetControlDetails (hMixer, &details, MIXER_GETCONTROLDETAILSF_VALUE);

					*vol = (float)(detailValue.dwValue - control.Bounds.dwMinimum) / range;
				}
				
				mixerClose ((HMIXER) hMixer);
				return;
			}
		}
		
		mixerClose ((HMIXER) hMixer);
	}
}

void CDAudio_UpdateVolume (float vol)
{
	CDAudio_AccessVolume (&vol, CDDA_SETVOL);
}

/*
============
CDAudio_MessageHandler
============
*/
LONG CDAudio_MessageHandler (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (lParam != wDeviceID)
		return 1;

	switch (wParam)
	{
		case MCI_NOTIFY_SUCCESSFUL:
			if (playing)
			{
				playing = false;
				if (playLooping)
					CDAudio_Play (SRC_COMMAND, playTrack, true);
			}
			break;

		case MCI_NOTIFY_ABORTED:
		case MCI_NOTIFY_SUPERSEDED:
			break;

		case MCI_NOTIFY_FAILURE:
			Con_DPrintf ("MCI_NOTIFY_FAILURE\n");
			CDAudio_Stop (SRC_COMMAND);
			cdValid = false;
			break;

		default:
			Con_DPrintf ("Unexpected MM_MCINOTIFY type (%i)\n", wParam);
			return 1;
	}

	return 0;
}

/*
============
CDAudio_InitDevice
============
*/
qboolean CDAudio_InitDevice (void)
{
	DWORD		dwReturn;
	MCI_OPEN_PARMS	mciOpenParms;
	MCI_SET_PARMS	mciSetParms;

	mciOpenParms.lpstrDeviceType = "cdaudio";
	if ((dwReturn = mciSendCommandA (0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE, (DWORD)(LPVOID) &mciOpenParms)))
	{
		Con_Printf ("CDAudio_Init: MCI_OPEN failed (%i)\n", dwReturn);
		return false;
	}
	wDeviceID = mciOpenParms.wDeviceID;

	// Set the time format to track/minute/second/frame (TMSF).
	mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
	if ((dwReturn = mciSendCommandA (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID) &mciSetParms)))
	{
		Con_Printf ("MCI_SET_TIME_FORMAT failed (%i)\n", dwReturn);

		mciSendCommandA (wDeviceID, MCI_CLOSE, 0, 0);
		return false;
	}

// JDH: store original volume
	CDAudio_AccessVolume (&cd_initialvol, CDDA_GETVOL);
	
	return true;
}

/*
============
CDAudio_CloseDevice
============
*/
void CDAudio_CloseDevice (void)
{
// JDH: restore original volume
	CDAudio_AccessVolume (&cd_initialvol, CDDA_SETVOL);
	
	if (mciSendCommandA (wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD) NULL))
		Con_DPrintf ("CDAudio_Shutdown: MCI_CLOSE failed\n");
}

#endif		//#ifndef RQM_SV_ONLY
