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
// sys.h -- non-portable functions

// file IO

// returns the file size
// return -1 if file is not present
// the file should be in BINARY mode for stupid OSs that care
/*
int Sys_FileOpenRead (const char *path, int *hndl);

int Sys_FileOpenWrite (const char *path);
void Sys_FileClose (int handle);
void Sys_FileSeek (int handle, int position);
int Sys_FileRead (int handle, void *dest, int count);
int Sys_FileWrite (int handle, const void *data, int count);
*/

typedef struct 
{
    unsigned short wYear;		// 0000-20xx
    unsigned short wMonth;		// 1-12
    unsigned short wDayOfWeek;	// 0-6 (Sunday=0)
    unsigned short wDay;		// 1-31
    unsigned short wHour;		// 0-23
    unsigned short wMinute;
    unsigned short wSecond;
} qtime_t;

qtime_t * Sys_FileTime (const char *path);
qboolean Sys_FolderExists (const char *path);
void Sys_mkdir (const char *path);

//typedef enum {DF_NODIRS, DF_WITHDIRS, DF_ONLYDIRS} dirfilter_t;

int Sys_ListFilesInDir (const char *path, const searchpath_t *search, qboolean dirs_only, 
						int *count, FFCALLBACK callproc, unsigned int callparam);

void *Sys_LoadLibrary (const char *name);
void Sys_UnloadLibrary (void *lib_handle);

// memory protection
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length);

// system IO
//void Sys_DebugLog(char *file, char *fmt, ...);

void Sys_Error (const char *error, ...);
// an error will cause the entire program to exit

void Sys_Printf (const char *fmt, ...);
// send text to the console

void Sys_Quit (void);

double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

void Sys_Sleep (void);
// called to yield for a little bit so as
// not to hog cpu when paused or debugging

void Sys_SendKeyEvents (void);
// Perform Key_Event () callbacks until the input que is empty

void Sys_LowFPPrecision (void);
void Sys_HighFPPrecision (void);
void Sys_SetFPCW (void);

char *Sys_GetClipboardData (void);
