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
// sys_linux.c

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>
#include <dirent.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>		// JDH: for clipboard
#include "quakedef.h"

extern Display	*dpy;

qboolean isDedicated = false;

qboolean	nostdout = true;

char	*basedir = ".";

//cvar_t  sys_linerefresh = {"sys_linerefresh", "0"};	// set for entity display

// =======================================================================
// General routines
// =======================================================================

void Sys_DebugNumber(int y, int val)
{
}

/*
============
Sys_Printf
============
*/
void Sys_Printf (const char *fmt, ...)
{
	va_list		argptr;
	char		text[2048];
	unsigned char	*p;

	if (nostdout)
		return;

	va_start (argptr, fmt);
	vsnprintf (text, sizeof(text), fmt, argptr);
	va_end (argptr);

	if (strlen(text) > sizeof(text))
		Sys_Error ("memory overwrite in Sys_Printf");

	for (p=(unsigned char *)text ; *p ; p++)
	{
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf ("[%02x]", *p);
		else
			putc (*p, stdout);
	}

	// joe, from ProQuake: rcon (64 doesn't mean anything special, but we need some extra space because NET_MAXMESSAGE == RCON_BUFF_SIZE)
	if (rcon_active && (rcon_message.cursize < rcon_message.maxsize - strlen(text) - 64))
	{
		rcon_message.cursize--;
		MSG_WriteString (&rcon_message, text);
	}
}

/*
============
Sys_Quit
============
*/
void Sys_Quit (void)
{
	Host_Shutdown ();
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	fflush (stdout);

	exit (0);
}

/*
============
Sys_Error
============
*/
void Sys_Error (const char *error, ...)
{
	va_list	argptr;
	char	string[1024];

// change stdin to non blocking
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

	va_start (argptr, error);
	vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);
	fprintf (stderr, "Error: %s\n", string);

	Host_Shutdown ();
	exit (1);
}

/*
void Sys_Warn (const char *warning, ...)
{
	va_list	argptr;
	char	string[1024];

	va_start (argptr, warning);
	vsnprintf (string, sizeof(string), warning, argptr);
	va_end (argptr);
	fprintf (stderr, "Warning: %s", string);
}
*/

/*
============
Sys_FileTime

returns NULL if not present
============
*/
qtime_t * Sys_FileTime (const char *path)
{
	struct	stat	buf;
	struct	tm		localtm;
	static	qtime_t	qt;

	if (stat(path, &buf) == -1)
		return NULL;

	if (!localtime_r (&buf.st_mtime, &localtm))
		return NULL;

	qt.wYear = localtm.tm_year + 1900;
	qt.wMonth = localtm.tm_mon + 1;
	qt.wDayOfWeek = localtm.tm_wday;
	qt.wDay = localtm.tm_mday;
	qt.wHour = localtm.tm_hour;
	qt.wMinute = localtm.tm_min;
	qt.wSecond = localtm.tm_sec;
	return &qt;
}

/*
================
Sys_FolderExists
================
*/
qboolean Sys_FolderExists (const char *path)
{
	struct	stat	filestat;

	if (stat(path, &filestat) == -1)
		return false;

	return (S_ISDIR(filestat.st_mode) ? true : false);
}

/*
============
Sys_mkdir
============
*/
void Sys_mkdir (const char *path)
{
	mkdir (path, 0777);
}


/*
int Sys_FileOpenRead (const char *path, int *handle)
{
	int		h;
	struct stat	fileinfo;

	h = open (path, O_RDONLY, 0666);
	*handle = h;
	if (h == -1)
		return -1;

	if (fstat (h,&fileinfo) == -1)
		Sys_Error ("Error fstating %s", path);

	return fileinfo.st_size;
}

int Sys_FileOpenWrite (const char *path)
{
	int     handle;

	umask (0);

	handle = open (path, O_RDWR | O_CREAT | O_TRUNC, 0666);

	if (handle == -1)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));

	return handle;
}

int Sys_FileWrite (int handle, const void *src, int count)
{
	return write (handle, src, count);
}

void Sys_FileClose (int handle)
{
	close (handle);
}

void Sys_FileSeek (int handle, int position)
{
	lseek (handle, position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return read (handle, dest, count);
}

void Sys_DebugLog (char *file, char *fmt, ...)
{
	va_list		argptr;
	static	char	data[1024];
	int		fd;

	va_start (argptr, fmt);
	vsnprintf (data, sizeof(data), fmt, argptr);
	va_end (argptr);
//	fd = open(file, O_WRONLY | O_BINARY | O_CREAT | O_APPEND, 0666);
	fd = open (file, O_WRONLY | O_CREAT | O_APPEND, 0666);
	write (fd, data, strlen(data));
	close (fd);
}

void Sys_EditFile (char *filename)
{
	char	cmd[256], *term, *editor;

	term = getenv("TERM");
	if (term && !strcmp(term, "xterm"))
	{
		editor = getenv("VISUAL");
		if (!editor)
			editor = getenv("EDITOR");
		if (!editor)
			editor = getenv("EDIT");
		if (!editor)
			editor = "vi";
		sprintf (cmd, "xterm -e %s %s", editor, filename);
		system (cmd);
	}
}
*/

/*
========================
Sys_ListFilesInDir - JDH
========================
*/

/*  -- this version does only case-sensitive matching --

int Sys_ListFilesInDir (char *path, searchpath_t *search, int *count,
						FFCALLBACK callproc, unsigned int callparam)
{
	int				h, i, result = 0;
	glob_t			fd;
	char			*p;
	struct	stat	filestat;
	packfile_t		fileinfo;

	h = glob (path, 0, NULL, &fd);
	if (h != 0)
	{
		globfree (&fd);
		return 0;		// return non-zero only if callback returned non-zero
	}

//	Con_Printf("%cFound %d files in path \"%s\" (h=%d)\n", 2, fd.gl_pathc, path, h);

	for (i = 0; i < fd.gl_pathc; i++)
	{
		if (!(p = strrchr(fd.gl_pathv[i], '/')))
			p = fd.gl_pathv[i];
		else
			p++;
		stat (fd.gl_pathv[i], &filestat);

		if (S_ISDIR(filestat.st_mode))
		{
			fileinfo.filelen = 0;
		}
		else
		{
			fileinfo.filelen = filestat.st_size;
		}

		fileinfo.filepos = 0;
		Q_strcpy (fileinfo.name, p, sizeof(fileinfo.name));

//		result = callproc (&fileinfo, search, 0, i+(*count)++, callparam);
		result = callproc (&fileinfo, search, 0, (*count)++, callparam);
		if (result)
			break;
	}

	globfree (&fd);
	return result;
}
*/

int Sys_ListFilesInDir (const char *path, const searchpath_t *search, qboolean dirs_only,
						int *count, FFCALLBACK callproc, unsigned int callparam)
{
	int				wildcard, result = 0;		// return non-zero only if callback returned non-zero
	char			*p, *fname, fullname[MAX_OSPATH];
	DIR				*dirp;
	struct dirent	*dp;
	struct stat		filestat;
	com_fileinfo_t	fileinfo;

	fname = strrchr (path, '/');
	if (fname)
	{
		p = strchr (fname+1, '*');
		wildcard = (p ? p-(fname+1) : -1);
		*fname = 0;
	}
	else
	{
		wildcard = -1;
	}

	dirp = opendir (path);
//	Con_DPrintf ("Sys_ListFilesInDir: opendir (%s) --> $%08X\n", path, dirp);
	p = fname;

	while (dirp)
	{
		dp = readdir (dirp);
		if (!dp)
			break;

		if (!p || COM_FilenameMatches (dp->d_name, p+1, wildcard, 0))
		{
			Q_snprintfz (fullname, sizeof(fullname), "%s/%s", path, dp->d_name);
			stat (fullname, &filestat);

//			Con_DPrintf ("Sys_ListFilesInDir: stat (\"%s\")\n", fullname);

			if (S_ISDIR (filestat.st_mode))
			{
				if (!dirs_only)
					continue;
				fileinfo.filelen = 0;
				fileinfo.isdir = true;
			}
			else
			{
				if (dirs_only)
					continue;
				fileinfo.filelen = filestat.st_size;
				fileinfo.isdir = false;
			}

			if (!strcmp (dp->d_name, ".") || !strcmp(dp->d_name, ".."))
				continue;

			/*if (!(fname = strrchr(dp->d_name, '/')))
				fname = dp->d_name;
			else
				fname++;
			Q_strcpy (fileinfo.name, fname, sizeof(fileinfo.name));*/

			Q_strcpy (fileinfo.name, dp->d_name, sizeof(fileinfo.name));
			fileinfo.filepos = 0;
			fileinfo.searchpath = search;
			fileinfo.index = -1;

			result = callproc (&fileinfo, (*count)++, callparam);
			if (result)
				break;
		}
//		else Con_DPrintf ("  no match: dp->d_name = %s\n", dp->d_name);
	}

	if (p)
		*p = '/';
	closedir (dirp);
	return result;
}

/*
=============
Sys_LoadLibrary
=============
*/
void * Sys_LoadLibrary (const char *name)
{
	return dlopen (name, RTLD_NOW);
}

/*
=============
Sys_UnloadLibrary
=============
*/
void Sys_UnloadLibrary (void *lib_handle)
{
	dlclose (lib_handle);
}

/*
=============
Sys_DoubleTime
=============
*/
double Sys_DoubleTime (void)
{
	struct	timeval		tp;
	struct	timezone	tzp;
	static	int		secbase;

	gettimeofday (&tp, &tzp);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
}

// =======================================================================
// Sleeps for microseconds
// =======================================================================

/*
static	volatile int	oktogo;

void alarm_handler (int x)
{
	oktogo = 1;
}

void Sys_LineRefresh (void)
{
}
*/

void floating_point_exception_handler (int whatever)
{
//	Sys_Warn("floating point exception\n");
	signal (SIGFPE, floating_point_exception_handler);
}

/*
=============
Sys_ConsoleInput
=============
*/
char *Sys_ConsoleInput (void)
{
	static	char	text[256];
	int     	len;
	fd_set		fdset;
	struct timeval	timeout;

	if (cls.state == ca_dedicated)
	{
		FD_ZERO(&fdset);
		FD_SET(0, &fdset); // stdin
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if (select(1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
			return NULL;

		len = read (0, text, sizeof(text));
		if (len < 1)
			return NULL;
		text[len-1] = 0;    // rip off the /n and terminate

		return text;
	}

	return NULL;
}

/*
#if !id386
void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}
#endif
*/


/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
	int		r;
	unsigned long	addr;
	int		psize = getpagesize ();

	addr = (startaddr & ~(psize-1)) - psize;
	r = mprotect ((char*)addr, length + startaddr - addr + psize, 7);

	if (r < 0)
    		Sys_Error ("Protection change failed\n");
}
/*
#define SYS_CLIPBOARD_SIZE	256
static	char	clipboard_buffer[SYS_CLIPBOARD_SIZE] = {0};
*/
/*
=============
Sys_GetClipboardData
=============
*/
char *Sys_GetClipboardData (void)
{
	Window owner;
	extern Window win;
	extern Atom	vid_clipatom;

//	Con_DPrintf ("Sys_GetClipboardData called\n");

	owner = XGetSelectionOwner (dpy, XA_PRIMARY);
	if (owner == None)
		owner = XGetSelectionOwner (dpy, XA_SECONDARY);

	if (owner != None)
	{
		/*char *name;
		if (XFetchName (dpy, owner, &name))
			XFree (name);*/

		XConvertSelection (dpy, XA_PRIMARY, XA_STRING, /*None*/ vid_clipatom, win, CurrentTime);
		XFlush (dpy);
	}
	else
	{
		/*int count;
		byte *data;

		Con_DPrintf ("Sys_GetClipboardData: no owner\n");

		data = (byte *) XFetchBytes (dpy, &count);
		if (data)
		{
			count = min (count, sizeof(clipboard_buffer)-1);
			memcpy (clipboard_buffer, data, count);
			clipboard_buffer[count] = 0;
			XFree (data);
			return clipboard_buffer;
		}*/
	}

	return NULL;
}

/*
=============
main
=============
*/
int main (int argc, const char **argv)
{
	double		time, oldtime, newtime;
	quakeparms_t	parms;
	extern	FILE	*vcrFile;
	extern	int	recording;
	int		j;

//	static char cwd[1024];

//	signal (SIGFPE, floating_point_exception_handler);
	signal (SIGFPE, SIG_IGN);

	memset (&parms, 0, sizeof(parms));

	COM_InitArgv (argc, argv);
	parms.argc = com_argc;
	parms.argv = com_argv;

//	isDedicated = COM_CheckParm("-dedicated");
	if (COM_CheckParm("-stdout"))
		nostdout = false;

	parms.memsize = 64 * 1024 * 1024;		// JDH: was 32MB

	if ((j = COM_CheckParm("-heapsize")) != 0 && j + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[j+1]) * 1024;

	if ((j = COM_CheckParm("-mem")) && j + 1 < com_argc)
		parms.memsize = Q_atoi(com_argv[j+1]) * 1024 * 1024;

	parms.membase = Q_malloc (parms.memsize);

	parms.basedir = basedir;

	fcntl (0, F_SETFL, fcntl(0, F_GETFL, 0) | FNDELAY);

	Host_Init (&parms);

#if id386
	Sys_SetFPCW ();
#endif

	oldtime = Sys_DoubleTime () - 0.1;
	while (1)
	{
// find time spent rendering last frame
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		if (cls.state == ca_dedicated)
		{	// play vcrfiles at max speed
			if (time < sys_ticrate.value && (!vcrFile || recording))
			{
				usleep (1);
				continue;       // not time to run a server only tic yet
			}
			time = sys_ticrate.value;
		}

		if (time > sys_ticrate.value*2)
			oldtime = newtime;
		else
			oldtime += time;

		Host_Frame (time);

// graphic debugging aids
//		if (sys_linerefresh.value)
//			Sys_LineRefresh ();
	}
}
