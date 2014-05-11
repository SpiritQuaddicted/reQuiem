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
// sys_win.c -- Win32 system interface code

#include "quakedef.h"
#include "winquake.h"
#include "resource.h"
#include "conproc.h"
#include <limits.h>
#include <errno.h>
#include <direct.h>		// _mkdir
#include <conio.h>		// _putch

#define MINIMUM_WIN_MEMORY	0x1000000	// JT: increased
#define MAXIMUM_WIN_MEMORY	0x8000000	// JT: increased to 128MB

#define CONSOLE_ERROR_TIMEOUT	60.0	// # of seconds to wait on Sys_Error running
					// dedicated before exiting
#define PAUSE_SLEEP		50	// sleep time on pause or minimization
#define NOT_FOCUS_SLEEP		20	// sleep time when not focus

int		starttime;
qboolean	ActiveApp, Minimized;
qboolean	WinNT;
qboolean	DDActive;
qboolean	scr_skipupdate;

#ifdef RQM_SV_ONLY
qboolean block_drawing;
#endif

static	double		pfreq;
static	double		curtime = 0.0;
static	double		lastcurtime = 0.0;
static	int		lowshift;
qboolean		isDedicated;
static	qboolean	sc_return_on_enter = false;
HANDLE			hinput, houtput;

static char		*tracking_tag = "Clams & Mooses";

static HANDLE	tevent;
static HANDLE	hFile;
static HANDLE	heventParent;
static HANDLE	heventChild;

void MaskExceptions (void);
void Sys_InitDoubleTime (void);

//void Sys_PushFPCW_SetHigh (void);
//void Sys_PopFPCW (void);

/*
===============================================================================

FILE IO

===============================================================================
*/

/*
// joe: these functions are outdated
#if 0
#define	MAX_HANDLES	10
FILE	*sys_handles[MAX_HANDLES];

int findhandle (void)
{
	int	i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

int Sys_FileOpenRead (const char *path, int *hndl)
{
	FILE	*f;
	int	i, retval, t;

	t = VID_ForceUnlockedAndReturnState ();

	i = findhandle ();

	f = fopen (path, "rb");

	if (!f)
	{
		*hndl = -1;
		retval = -1;
	}
	else
	{
		sys_handles[i] = f;
		*hndl = i;
		retval = COM_FileLength (f);
	}

	VID_ForceLockState (t);

	return retval;
}

int Sys_FileOpenWrite (const char *path)
{
	FILE	*f;
	int	i, t;

	t = VID_ForceUnlockedAndReturnState ();

	i = findhandle ();

	if (!(f = fopen(path, "wb")))
	{
		COM_CreatePath (path);
		if (!(f = fopen(path, "wb")))
			Sys_Error ("Error opening %s: %s", path, strerror(errno));
	}
	sys_handles[i] = f;

	VID_ForceLockState (t);

	return i;
}

void Sys_FileClose (int handle)
{
	int	t;

	t = VID_ForceUnlockedAndReturnState ();
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
	VID_ForceLockState (t);
}

void Sys_FileSeek (int handle, int position)
{
	int	t;

	t = VID_ForceUnlockedAndReturnState ();
	fseek (sys_handles[handle], position, SEEK_SET);
	VID_ForceLockState (t);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	int	t, x;

	t = VID_ForceUnlockedAndReturnState ();
	x = fread (dest, 1, count, sys_handles[handle]);
	VID_ForceLockState (t);

	return x;
}

int Sys_FileWrite (int handle, const void *data, int count)
{
	int	t, x;

	t = VID_ForceUnlockedAndReturnState ();
	x = fwrite (data, 1, count, sys_handles[handle]);
	VID_ForceLockState (t);

	return x;
}
#endif
*/

/*
================
Sys_FileTime
================
*/
qtime_t * Sys_FileTime (const char *path)
{
	HANDLE			h;
	WIN32_FIND_DATA	fd;
	FILETIME		localtime;
	SYSTEMTIME		systime;
	static qtime_t	qt;
	
	h = FindFirstFileA (path, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return NULL;

	FindClose (h);

	FileTimeToLocalFileTime (&fd.ftLastWriteTime, &localtime);
	FileTimeToSystemTime (&localtime, &systime);
	
	qt.wYear = systime.wYear;
	qt.wMonth = systime.wMonth;
	qt.wDayOfWeek = systime.wDayOfWeek;
	qt.wDay = systime.wDay;
	qt.wHour = systime.wHour;
	qt.wMinute = systime.wMinute;
	qt.wSecond = systime.wSecond;
	return &qt;
}

/*
================
Sys_FolderExists
================
*/
qboolean Sys_FolderExists (const char *path)
{
	HANDLE			h;
	WIN32_FIND_DATA	fd;
	
	h = FindFirstFileA (path, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	FindClose (h);
	return (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
}

/*
================
Sys_mkdir
================
*/
void Sys_mkdir (const char *path)
{
	_mkdir (path);
}

/*
========================
Sys_ListFilesInDir - JDH
========================
*/
int Sys_ListFilesInDir (const char *path, const searchpath_t *search, qboolean dirs_only, 
						int *count, FFCALLBACK callproc, unsigned int callparam)
{
	HANDLE			h;
	WIN32_FIND_DATA	fd;
	int				result;
	com_fileinfo_t	fileinfo;

	h = FindFirstFileA (path, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return 0;		// return non-zero only if callback returned non-zero
	
	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!dirs_only)
				continue;
		}
		else if (dirs_only)
			continue;

		if (!strcmp (fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
			continue;
		
		Q_strcpy (fileinfo.name, fd.cFileName, sizeof(fileinfo.name));
		//fileinfo.name[ strlen( fileinfo.name ) - suffixlen ] = 0;
		fileinfo.filelen = fd.nFileSizeLow;
		fileinfo.filepos = 0;
		fileinfo.searchpath = search;
		fileinfo.index = -1;
		fileinfo.isdir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
		
		result = callproc (&fileinfo, (*count)++, callparam);
		if (result)
			break;
	}
	while (FindNextFileA (h, &fd));

	FindClose (h);
	return result;
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	DWORD  flOldProtect;

	if (!VirtualProtect ((LPVOID)startaddr, length, PAGE_READWRITE, &flOldProtect))
		Sys_Error ("Protection change failed\n");
}

/*
================
Sys_Init
================
*/
void Sys_Init (void)
{
	OSVERSIONINFOA	vinfo;

	MaskExceptions ();
	Sys_SetFPCW ();

	Sys_InitDoubleTime ();

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionExA(&vinfo))
		Sys_Error ("Couldn't get OS info");

	if ((vinfo.dwMajorVersion < 4) || (vinfo.dwPlatformId == VER_PLATFORM_WIN32s))
		Sys_Error ("Quake requires at least Win95 or NT 4.0");

	WinNT = (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) ? true : false;
}

/*
================
Sys_Error
================
*/
void Sys_Error (const char *error, ...)
{
	va_list		argptr;
	char		text[1024], text2[1024];
	char		*text3 = "Press Enter to exit\n";
	char		*text4 = "***********************************\n";
	char		*text5 = "\n";
	DWORD		dummy;
	double		starttime;
	static	int	in_sys_error0 = 0;
	static	int	in_sys_error1 = 0;
	static	int	in_sys_error2 = 0;
	static	int	in_sys_error3 = 0;

	if (!in_sys_error3)
	{
		in_sys_error3 = 1;
//#ifndef GLQUAKE
//		VID_ForceUnlockedAndReturnState ();
//#endif
	}

	va_start (argptr, error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	if (isDedicated)
	{
		va_start (argptr, error);
		vsnprintf (text, sizeof(text), error, argptr);
		va_end (argptr);

		Q_snprintfz (text2, sizeof(text2), "ERROR: %s\n", text);
		WriteFile (houtput, text5, strlen(text5), &dummy, NULL);
		WriteFile (houtput, text4, strlen(text4), &dummy, NULL);
		WriteFile (houtput, text2, strlen(text2), &dummy, NULL);
		WriteFile (houtput, text3, strlen(text3), &dummy, NULL);
		WriteFile (houtput, text4, strlen(text4), &dummy, NULL);

		starttime = Sys_DoubleTime ();
		sc_return_on_enter = true;	// so Enter will get us out of here
	}
	else
	{
	// switch to windowed so the message box is visible, unless we already
	// tried that and failed
		if (!in_sys_error0)
		{
			in_sys_error0 = 1;
		#ifndef RQM_SV_ONLY
			VID_SetDefaultMode ();
		#endif
			MessageBoxA (NULL, text, "reQuiem Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
		}
		else
		{
			MessageBoxA (NULL, text, "Double reQuiem Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
		}
	}

	if (!in_sys_error1)
	{
		in_sys_error1 = 1;
		Host_Shutdown ();
	}

	// shut down QHOST hooks if necessary
	if (!in_sys_error2)
	{
		in_sys_error2 = 1;
		DeinitConProc ();
	}

	exit (1);
}

/*
================
Sys_Printf
================
*/
void Sys_Printf (const char *fmt, ...)
{
	va_list	argptr;
	char	buf[2048], *text;
	DWORD	dummy;
	
	if (isDedicated)
	{
		va_start (argptr, fmt);
		vsnprintf (buf, sizeof(buf), fmt, argptr);
		va_end (argptr);

		text = buf;
		
		// ignore colored text marker:
		if ((buf[0] == 1) || (buf[0] == 2))
			text++;
		
		WriteFile (houtput, text, strlen(text), &dummy, NULL);

		// joe, from ProQuake: rcon (64 doesn't mean anything special, but we need some extra space because NET_MAXMESSAGE == RCON_BUFF_SIZE)
		if (rcon_active  && (rcon_message.cursize < rcon_message.maxsize - strlen(buf) - 64))
		{
			rcon_message.cursize--;
			MSG_WriteString (&rcon_message, buf);
		}
	}
}

/*
================
Sys_Quit
================
*/
void Sys_Quit (void)
{
//#ifndef GLQUAKE
//	VID_ForceUnlockedAndReturnState ();
//#endif

	Host_Shutdown ();

	if (tevent)
		CloseHandle (tevent);

	if (isDedicated)
		FreeConsole ();

// shut down QHOST hooks if necessary
	DeinitConProc ();

	exit (0);
}

// joe: not using just float for timing any more,
// this is copied from ZQuake source to fix overspeeding.

static	double	pfreq;
static qboolean	hwtimer = false;

/*
================
Sys_InitDoubleTime
================
*/
void Sys_InitDoubleTime (void)
{
	__int64	freq;

	if (!COM_CheckParm("-nohwtimer") && QueryPerformanceFrequency ((LARGE_INTEGER *)&freq) && freq > 0)
	{
		// hardware timer available
		pfreq = (double)freq;
		hwtimer = true;
	}
	/*else*/		// JDH: timeBeginPeriod also affects sleep/wait calls, 
	{				//      so I call it even if we're using the hw timer
		// make sure the timer is high precision, otherwise
		// NT gets 18ms resolution
		timeBeginPeriod (1);
	}
}

/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	__int64		pcount;
	static	__int64	startcount;
	static	DWORD	starttime;
	static qboolean	first = true;
	DWORD	now;

	if (hwtimer)
	{
		QueryPerformanceCounter ((LARGE_INTEGER *)&pcount);
		if (first)
		{
			first = false;
			startcount = pcount;
			return 0.0;
		}
		// TODO: check for wrapping
		return (pcount - startcount) / pfreq;
	}

	now = timeGetTime ();

	if (first)
	{
		first = false;
		starttime = now;
		return 0.0;
	}
	
	if (now < starttime) // wrapped?
		return (now / 1000.0) + (LONG_MAX - starttime / 1000.0);

	if (now - starttime == 0)
		return 0.0;

	return (now - starttime) / 1000.0;
}

/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	static char	text[256];
	static int	len;
	INPUT_RECORD	recs[1024];
	int		ch;
	DWORD	numevents, numread, dummy;

	if (!isDedicated)
		return NULL;

	for ( ; ; )
	{
		if (!GetNumberOfConsoleInputEvents(hinput, &numevents))
			Sys_Error ("Error getting # of console events");

		if (numevents <= 0)
			break;

		if (!ReadConsoleInputA (hinput, recs, 1, &numread))
			Sys_Error ("Error reading console input");

		if (numread != 1)
			Sys_Error ("Couldn't read console input");

		if (recs[0].EventType == KEY_EVENT)
		{
			if (!recs[0].Event.KeyEvent.bKeyDown)
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;

				switch (ch)
				{
					case '\r':
						WriteFile (houtput, "\r\n", 2, &dummy, NULL);

						if (len)
						{
							text[len] = 0;
							len = 0;
							return text;
						}
						else if (sc_return_on_enter)
						{
						// special case to allow exiting from the error handler on Enter
							text[0] = '\r';
							len = 0;
							return text;
						}

						break;

					case '\b':
						WriteFile (houtput, "\b \b", 3, &dummy, NULL);
						if (len)
							len--;
						break;

					default:
						if (ch >= ' ')
						{
							WriteFile (houtput, &ch, 1, &dummy, NULL);
							text[len] = ch;
							len = (len + 1) & 0xff;
						}

						break;
				}
			}
		}
	}

	return NULL;
}

/*
================
Sys_Sleep
================
*/
void Sys_Sleep (void)
{
	Sleep (1);
}


/*
================
Sys_SendKeyEvents
================
*/
void Sys_SendKeyEvents (void)
{
	MSG	msg;

	while (PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
	// we always update if there are any event, even if we're paused
		scr_skipupdate = 0;

		if (!GetMessageA(&msg, NULL, 0, 0))
			Sys_Quit ();

		TranslateMessage (&msg);
		DispatchMessageA (&msg);
	}
}


#define	SYS_CLIPBOARD_SIZE		256

/*
================
Sys_GetClipboardData
================
*/
char *Sys_GetClipboardData (void)
{
	HANDLE		th;
	char		*cliptext, *s, *t;
	static	char	clipboard[SYS_CLIPBOARD_SIZE];

	if (!OpenClipboard(NULL))
		return NULL;

	if (!(th = GetClipboardData(CF_TEXT)))
	{
		CloseClipboard ();
		return NULL;
	}

	if (!(cliptext = GlobalLock(th)))
	{
		CloseClipboard ();
		return NULL;
	}

	s = cliptext;
	t = clipboard;
	while (*s && t - clipboard < SYS_CLIPBOARD_SIZE - 1 && *s != '\n' && *s != '\r' && *s != '\b')
		*t++ = *s++;
	*t = 0;

	GlobalUnlock (th);
	CloseClipboard ();

	return clipboard;
}

/*
=============
Sys_LoadLibrary
=============
*/
void * Sys_LoadLibrary (const char *name)
{
	return LoadLibrary (name);
}

/*
=============
Sys_UnloadLibrary
=============
*/
void Sys_UnloadLibrary (void *lib_handle)
{
	FreeLibrary (lib_handle);
}


/*
==============================================================================

 WINDOWS CRAP

==============================================================================
*/


void SleepUntilInput (int time)
{
	MsgWaitForMultipleObjects (1, &tevent, FALSE, time, QS_ALLINPUT);
}


/*
==================
WinMain
==================
*/
HINSTANCE	global_hInstance;
int			global_nCmdShow;
static char	*empty_string = "";
//HWND		hwnd_dialog;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	quakeparms_t	parms;
	double		time, oldtime, newtime;
	MEMORYSTATUS	lpBuffer;
	static	char	cwd[1024];
	LPWSTR		*wargv;
	int		arg_num;
	int		t;
//	RECT		rect;

	/* previous instances do not exist in Win32 */
	if (hPrevInstance)
		return 0;

	global_hInstance = hInstance;
	global_nCmdShow = nCmdShow;

	lpBuffer.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus (&lpBuffer);

	if (!GetCurrentDirectoryA(sizeof(cwd), cwd))
		Sys_Error ("Couldn't determine current directory");

	if (cwd[strlen(cwd)-1] == '/')
		cwd[strlen(cwd)-1] = 0;

	parms.basedir = cwd;

	// We want to preserve spaces in the command-line arguments, which are not
	// uncommon in path values for args (like -basedir value). A few ways we
	// could do this:
	// - Link with the VC++ runtime and use the generated __argc and __argv
	//   variables.
	// - Parse command line ourselves, using the standard Microsoft rules for
	//   C/C++ programs.
	// - Get the unicode command line with GetCommandLineW and parse it with
	//   CommandLineToArgvW. But then we need convert everything to ANSI to
	//   stay compatible with the rest of the engine.

	// Let's do that last one. So:

	// Get the args in unicode.
	wargv = CommandLineToArgvW(GetCommandLineW(), &(parms.argc));
	// Allocate space pointers to ANSI args.
	parms.argv = Q_malloc(sizeof(char*) * parms.argc);
	// First arg is always just empty-string.
	parms.argv[0] = empty_string;
	// Loop through remaining args and convert them to ANSI.
	for (arg_num = 1; arg_num < parms.argc; arg_num++)
	{
		// Find the size for the ANSI arg.
		int arg_size = WideCharToMultiByte(
			CP_ACP, // ANSI
			0, // no special handling of unmapped chars
			wargv[arg_num], // arg to process
			-1, // arg is null-terminated
			NULL, 0, // no output yet, just calculating size
			NULL, NULL); // use default for unrepresented char
		// Allocate space for the ANSI arg.
		parms.argv[arg_num] = Q_malloc(arg_size);
		// Fill the ANSI arg.
		WideCharToMultiByte(CP_ACP, 0, wargv[arg_num], -1,
			parms.argv[arg_num], arg_size, NULL, NULL);
	}
	// Discard the unicode args.
	LocalFree(wargv);

	// Discard args past MAX_NUM_ARGVS, construct the "cmdline" console var,
	// and handle safe-mode switches.
	COM_InitArgv (parms.argc, parms.argv, lpCmdLine);

	parms.argc = com_argc;
	parms.argv = com_argv;

#ifndef RQM_SV_ONLY
	if (!(isDedicated = (COM_CheckParm("-dedicated"))))
	{
	/*	hwnd_dialog = CreateDialog (hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);

		if (hwnd_dialog)
		{
			if (GetWindowRect (hwnd_dialog, &rect))
			{
				if (rect.left > (rect.top * 2))
				{
					SetWindowPos (hwnd_dialog, 0,
						(rect.left / 2) - ((rect.right - rect.left) / 2),
						rect.top, 0, 0,
						SWP_NOZORDER | SWP_NOSIZE);
				}
			}

			ShowWindow (hwnd_dialog, SW_SHOWDEFAULT);
			UpdateWindow (hwnd_dialog);
			SetForegroundWindow (hwnd_dialog);
		}*/
	}
#else
	isDedicated = true;
#endif

// take the greater of all the available memory or half the total memory,
// but at least 16 Mb and no more than 128 Mb, unless they explicitly request otherwise
	parms.memsize = lpBuffer.dwAvailPhys;
	
	if (parms.memsize < MINIMUM_WIN_MEMORY)
		parms.memsize = MINIMUM_WIN_MEMORY;

	if (parms.memsize < (lpBuffer.dwTotalPhys >> 1))
		parms.memsize = lpBuffer.dwTotalPhys >> 1;

	if (parms.memsize > MAXIMUM_WIN_MEMORY)
		parms.memsize = MAXIMUM_WIN_MEMORY;

	if ((t = COM_CheckParm("-heapsize")) != 0 && t + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[t+1]) * 1024;

	if ((t = COM_CheckParm("-mem")) != 0 && t + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[t+1]) * 1024 * 1024;
	
	parms.membase = Q_malloc (parms.memsize);
	
	if (!(tevent = CreateEventA(NULL, FALSE, FALSE, NULL)))
		Sys_Error ("Couldn't create event");

	if (isDedicated)
	{
		if (!AllocConsole())
			Sys_Error ("Couldn't create dedicated server console");

		hinput = GetStdHandle (STD_INPUT_HANDLE);
		houtput = GetStdHandle (STD_OUTPUT_HANDLE);

	// give QHOST a chance to hook into the console
		if ((t = COM_CheckParm("-HFILE")) > 0)
		{
			if (t < com_argc-1)
				hFile = (HANDLE)Q_atoi (com_argv[t+1]);
		}
			
		if ((t = COM_CheckParm("-HPARENT")) > 0)
		{
			if (t < com_argc-1)
				heventParent = (HANDLE)Q_atoi (com_argv[t+1]);
		}
			
		if ((t = COM_CheckParm("-HCHILD")) > 0)
		{
			if (t < com_argc-1)
				heventChild = (HANDLE)Q_atoi (com_argv[t+1]);
		}

		InitConProc (hFile, heventParent, heventChild);
	}

	Sys_Init ();

#ifndef RQM_SV_ONLY
	// because sound is off until we become active
	S_BlockSound ();
#endif

	Sys_Printf ("Host_Init\n");
	Host_Init (&parms);
	
	oldtime = Sys_DoubleTime ();

	/* main window message loop */
	while (1)
	{
		if (isDedicated)
		{
			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;

			while (time < sys_ticrate.value)
			{
				Sys_Sleep ();
				newtime = Sys_DoubleTime ();
				time = newtime - oldtime;
			}
		}
#ifndef RQM_SV_ONLY
		else
		{
		// yield the CPU for a little while when paused, minimized, or not the focus
			if (!cls.capturedemo)
			{
				if ((cl.paused && (!ActiveApp && !DDActive)) || Minimized || block_drawing)
				{
					SleepUntilInput (PAUSE_SLEEP);
					scr_skipupdate = 1;		// no point in bothering to draw
				}
				else if (!ActiveApp && !DDActive)
				{
					SleepUntilInput (NOT_FOCUS_SLEEP);
				}
			}

			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;
		}
#endif

		Host_Frame (time);
		oldtime = newtime;
	}

	// return success of application
	return TRUE;
}

