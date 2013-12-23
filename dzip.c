
#include "quakedef.h"
#include "dzlib.h"

#ifdef _WIN32
#  include <windows.h>
#  define DZ_GETFUNC(f,fapi) (qDZ_##f = (fapi) GetProcAddress(dz_lib_handle, "DZ_"#f))
#  define DZ_LIBNAME	"dzlib.dll"
#else
#  include <dlfcn.h>
#  define DZ_GETFUNC(f,fapi) (qDZ_##f = (fapi) dlsym(dz_lib_handle, "DZ_"#f))
#  ifdef MACOSX
#    define DZ_LIBNAME  "dzlib.dylib"
#  else
#    define DZ_LIBNAME	"dzlib.so"
#  endif
#endif

static void *dz_lib_handle = NULL;

static DZ_OPEN_PROC          qDZ_Open = NULL;
static DZ_CLOSE_PROC         qDZ_Close = NULL;
static DZ_GETNUMFILES_PROC   qDZ_GetNumFiles = NULL;
static DZ_GETFILEINFO_PROC   qDZ_GetFileInfo = NULL;
static DZ_EXTRACTTEMP_PROC   qDZ_ExtractTemp = NULL;

static DZ_GETVERSION_PROC    qDZ_GetVersion = NULL;
static DZ_CREATE_PROC        qDZ_Create = NULL;
static DZ_CREATEZIP_PROC     qDZ_CreateZip = NULL;
static DZ_ADDFILE_PROC       qDZ_AddFile = NULL;
static DZ_VERIFYALL_PROC     qDZ_VerifyAll = NULL;
static DZ_GETLASTERROR_PROC  qDZ_GetLastError = NULL;
//static DZ_GETFILEINFOEX_PROC qDZ_GetFileInfoEx = NULL;

#define DZ_MAX_TEMPFILES 512

typedef struct dz_temp_entry
{
	DZHANDLE dzhandle;
	int   index;
	char *filepath;
} dz_temp_entry;

static dz_temp_entry dz_tempfiles[DZ_MAX_TEMPFILES];
static int dz_num_tempfiles = 0;

qboolean dz_canzip = false;

/*
================
Dzip_Init
================
*/
qboolean Dzip_Init (void)
{
	int i;
	char *vers;

	for (i = 0; i < DZ_MAX_TEMPFILES; i++)
	{
		dz_tempfiles[i].dzhandle = 0;
		dz_tempfiles[i].index = 0;
		dz_tempfiles[i].filepath = NULL;
	}

	if (!(dz_lib_handle = COM_LoadLibrary(DZ_LIBNAME)))
	{
	#ifdef _WIN32
		vers = NULL;
	#else
		vers = dlerror();
	#endif
		Con_Printf ("\x02""Failed to load dzlib!%s\n", (vers ? va(" (%s)", vers) : ""));
		return false;
	}

	DZ_GETFUNC (Open, DZ_OPEN_PROC);
	DZ_GETFUNC (Close, DZ_CLOSE_PROC);
	DZ_GETFUNC (GetNumFiles, DZ_GETNUMFILES_PROC);
	DZ_GETFUNC (GetFileInfo, DZ_GETFILEINFO_PROC);
	DZ_GETFUNC (ExtractTemp, DZ_EXTRACTTEMP_PROC);

	if (!qDZ_Open || !qDZ_Close || !qDZ_GetNumFiles || !qDZ_GetFileInfo || !qDZ_ExtractTemp)
	{
		COM_UnloadLibrary (dz_lib_handle);
		dz_lib_handle = NULL;
		Con_Print ("\x02""Failed to load dzlib!\n");
		return false;
	}

// It's not fatal if these funcs are missing (but demo compression will be disabled)
	DZ_GETFUNC (GetVersion, DZ_GETVERSION_PROC);
	DZ_GETFUNC (Create, DZ_CREATE_PROC);
	DZ_GETFUNC (CreateZip, DZ_CREATEZIP_PROC);
	DZ_GETFUNC (AddFile, DZ_ADDFILE_PROC);
	DZ_GETFUNC (VerifyAll, DZ_VERIFYALL_PROC);
	DZ_GETFUNC (GetLastError, DZ_GETLASTERROR_PROC);
//	DZ_GETFUNC (GetFileInfoEx, DZ_GETFILEINFOEX_PROC);

	dz_canzip = (qDZ_CreateZip != NULL);

	if (qDZ_GetVersion)
	{
		i = qDZ_GetVersion ();
		vers = va (" v%d.%d%d", i >> 16, (i >> 8) & 0xFF, i & 0xFF);
	}
	else vers = NULL;

	Con_Printf ("Successfully loaded dzlib%s\n", (vers ? vers : ""));
	return true;
}

/*
================
Dzip_Shutdown
================
*/
void Dzip_Shutdown (void)
{
	int i;

	for (i = 0; i < dz_num_tempfiles; i++)
	{
		if (dz_tempfiles[i].filepath)
		{
			remove(dz_tempfiles[i].filepath);
			free(dz_tempfiles[i].filepath);
			dz_tempfiles[i].dzhandle = 0;
			dz_tempfiles[i].index = 0;
			dz_tempfiles[i].filepath = NULL;
		}
	}

	dz_num_tempfiles = 0;
	dz_canzip = false;

	qDZ_Open = NULL;
	qDZ_Close = NULL;
	qDZ_GetNumFiles = NULL;
	qDZ_GetFileInfo = NULL;
	qDZ_ExtractTemp = NULL;

	qDZ_GetVersion = NULL;
	qDZ_Create = NULL;
	qDZ_CreateZip = NULL;
	qDZ_AddFile = NULL;
	qDZ_VerifyAll = NULL;
	qDZ_GetLastError = NULL;
//	qDZ_GetFileInfoEx = NULL;

	if (dz_lib_handle)
	{
		COM_UnloadLibrary (dz_lib_handle);
		dz_lib_handle = NULL;
	}
}

/*
=================
Dzip_LoadFileList

Takes an explicit (not game tree related) path to a dzip file.
Returns the contents (file listing) in the form of a pack_t pointer.
=================
*/
pack_t *Dzip_LoadFileList (const char *dzpath)
{
	DZHANDLE	dzhandle;
	int			i, numfiles;
	DZENTRY		entry;
	packfile_t	*newfiles;
	pack_t		*pack;

	dzhandle = qDZ_Open (dzpath);
	if (!dzhandle)
		return NULL;

	numfiles = qDZ_GetNumFiles (dzhandle);
	if (numfiles <= 0)
	{
		qDZ_Close (dzhandle);
		return NULL;
	}

	newfiles = Q_malloc (numfiles * sizeof(packfile_t));

	for (i = 0; i < numfiles; i++)
	{
		if (!qDZ_GetFileInfo (dzhandle, i, &entry))
			break;

		if (strlen (entry.name) >= sizeof (newfiles->name))
		{
			Con_DPrintf ("WARNING: file #%d in dzip %s skipped (name too long)\n", i, COM_SkipPath(dzpath));
			newfiles[i].name[0] = 0;
			newfiles[i].filepos = 0;
			newfiles[i].filelen = 0;
			continue;
		}

		Q_strcpy (newfiles[i].name, entry.name, sizeof(newfiles[i].name));
		newfiles[i].filepos = 0;
		newfiles[i].filelen = entry.size;
	}


	pack = Q_malloc (sizeof(pack_t));
	Q_strcpy (pack->filename, dzpath, sizeof(pack->filename));

	pack->dzhandle = (void *) dzhandle;
	pack->handle = NULL;
	pack->numfiles = numfiles;
	pack->files = newfiles;

	if (con_initialized)
		Con_Printf ("Added dzip %s (%i files)\n", dzpath, numfiles);

//	qDZ_Close (dzip);
	return pack;
}

/*
==========
Dzip_Close
==========
*/
void Dzip_Close (void *dzhandle)
{
	if (dzhandle)
	{
		qDZ_Close ((DZHANDLE)dzhandle);
		dzhandle = NULL;
	}
}

/*
================
Dzip_ExtractFile
================
*/
FILE * Dzip_ExtractFile (void *dzhandle, int index)
{
	int			i, tempindex = -1;
	FILE		*pf;
	DZENTRY		entry;
	char		filepath[MAX_OSPATH], *pathcopy;

	// check if it's already been extracted, and still exists:
	for (i = 0; i < dz_num_tempfiles; i++)
	{
		if ((dz_tempfiles[i].dzhandle == (DZHANDLE) dzhandle) &&
			(dz_tempfiles[i].index == index))
		{
			if (dz_tempfiles[i].filepath)
			{
				pf = fopen (dz_tempfiles[i].filepath, "rb");
				if (pf) return pf;
			}

			// if temp file no longer exists, reuse this slot
			free (dz_tempfiles[i].filepath);
			dz_tempfiles[i].filepath = NULL;
			tempindex = i;
			break;
		}
	}

	if ((tempindex < 0) && (dz_num_tempfiles == DZ_MAX_TEMPFILES))
		return NULL;

	if (qDZ_GetFileInfo ((DZHANDLE) dzhandle, index, &entry) && (entry.size > (1<<20)))
		Con_Printf ("Decompressing %s...\n", entry.name);

	i = qDZ_ExtractTemp ((DZHANDLE) dzhandle, index, filepath, MAX_OSPATH);
	if (i)
	{
		pf = fopen (filepath, "rb");
		if (pf)
		{
			pathcopy = Q_malloc (i+1);
			strcpy (pathcopy, filepath);

			if (tempindex < 0)
			{
				dz_tempfiles[dz_num_tempfiles].dzhandle = (DZHANDLE)dzhandle;
				dz_tempfiles[dz_num_tempfiles].index = index;
				dz_tempfiles[dz_num_tempfiles++].filepath = pathcopy;
			}
			else
				dz_tempfiles[tempindex].filepath = pathcopy;
		}
	}
	else pf = NULL;

	return pf;
}

/*
================
Dzip_OpenFromArchive
================
*/
extern cvar_t com_matchfilecase;

FILE * Dzip_OpenFromArchive (const char *dzpath, const char *filename)
{
	DZHANDLE	dzhandle;
	int			i, numfiles, wildcardpos;
	const char	*p;
	float		oldcase;
	DZENTRY		entry;
	FILE		*f = NULL;

	dzhandle = qDZ_Open (dzpath);
	if (!dzhandle)
		return NULL;

	numfiles = qDZ_GetNumFiles (dzhandle);
	if (numfiles <= 0)
		goto EXITPOINT;

	p = strchr (filename, '*');
	wildcardpos = (p ? p-filename : -1);

	oldcase = com_matchfilecase.value;
	com_matchfilecase.value = 0;

	for (i = 0; i < numfiles; i++)
	{
		if (!qDZ_GetFileInfo (dzhandle, i, &entry))
			break;

		if (COM_FilenameMatches (entry.name, filename, wildcardpos, 0))
		//if (!Q_strcasecmp (entry.name, filename))
		{
			f = Dzip_ExtractFile ((void *)dzhandle, i);
			break;
		}
	}

	com_matchfilecase.value = oldcase;

EXITPOINT:
	qDZ_Close (dzhandle);
	return f;
}

/*
================
Dzip_CompressFile
================
*/
qboolean Dzip_CompressFile (const char *path, const char *filename, const char *dzname, qboolean stdzip)
{
	DZHANDLE	(*createproc) (const char *);
	char		dzpath[MAX_OSPATH], filepath[MAX_OSPATH];
	DZHANDLE	dzh;
	qboolean	result;

	createproc = (stdzip ? qDZ_CreateZip : qDZ_Create);
	if (!createproc || !qDZ_AddFile)
		return false;

	Q_snprintfz (dzpath, sizeof(dzpath), "%s/%s", path, dzname);

	dzh = createproc (dzpath);
	if (!dzh)
		return false;

	Q_snprintfz (filepath, sizeof(filepath), "%s/%s", path, filename);
	result = qDZ_AddFile (dzh, filepath, DZ_FTYPE_DEM);

	qDZ_Close (dzh);
	if (!result)
		remove (dzname);

	return result;
}

/*
================
Dzip_Verify
================
*/
qboolean Dzip_Verify (const char *dzpath)
{
	DZHANDLE	dzh;
	qboolean	result;

	if (!qDZ_VerifyAll)
		return false;

	dzh = qDZ_Open (dzpath);
	if (!dzh)
		return false;

	result = qDZ_VerifyAll (dzh);

#ifdef _DEBUG
	if (!result && qDZ_GetLastError)
	{
		result = qDZ_GetLastError ();
		result = false;
	}
#endif

	qDZ_Close (dzh);
	return result;
}
