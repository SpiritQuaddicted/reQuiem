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
// common_file.c -- functions used in client and server file management

#include "quakedef.h"

#ifdef _WIN32
#  include "io.h"
#else
#  include <sys/stat.h>
#endif

#include "dzip.h"

#define MAX_FILES_IN_PACK	2048


// on disk
typedef struct
{
	char	name[56];
	int	filepos, filelen;
} dpackfile_t;

typedef struct
{
	char	id[4];
	int	dirofs;
	int	dirlen;
} dpackheader_t;

cvar_t  com_matchfilecase = {"host_matchfilecase", "0", CVAR_FLAG_ARCHIVE};

searchpath_t		*com_searchpaths;

int					com_filesize = -1;
char				com_netpath[MAX_OSPATH];
const searchpath_t	*com_filepath = NULL;		// JDH: to store path of last loaded file

char	com_gamedir[MAX_OSPATH];
char	com_basedir[MAX_OSPATH];
char	com_gamedirname[MAX_QPATH];

#ifdef HEXEN2_SUPPORT
  char    com_savedir[MAX_OSPATH];
#endif

searchpath_t	*com_base_searchpaths;	// without id1 and its packs
searchpath_t	*com_game_searchpaths;	// without -hipnotic, -rogue, -nehahra, -quoth

int   num_img_exts;
const char *com_img_exts[4];		// tga, jpg, pcx, and png (png and jpg iff library available)

#define NUM_DEMO_EXTS 3
const char * com_demo_exts[NUM_DEMO_EXTS] = {".dem", ".qwd", ".dz"};
const char * com_mdl_exts[NUM_MODEL_EXTS] = {".mdl", ".md2", ".md3"};
const char * com_music_exts[2] = {".mp3", ".ogg"};

qboolean hipnotic = false;
qboolean rogue = false;
qboolean nehahra = false;

#ifdef HEXEN2_SUPPORT
  qboolean hexen2 = false;
#endif

qboolean dzlib_loaded = false;

void COM_InitFilesystem (void);
void COM_Path_f (cmd_source_t src);
void COM_Gamedir_f (cmd_source_t src);

extern qboolean png_available, jpg_available;

/*

All of Quake's data access is through a hierchal file system, but the contents
of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all
game directories. The sys_* files pass this to host_init in quakeparms_t->basedir.
This can be overridden with the "-basedir" command line parm to allow code
debugging in a different directory. The base directory is only used during
filesystem initialization.

The "game directory" is the first tree on the search path and directory that all
generated files (savegames, screenshots, demos, config files) will be saved to.
This can be overridden with the "-game" command line parameter.
The game directory can never be changed while quake is executing.
This is a precacution against having a malicious server instruct clients to
write files over areas they shouldn't.

The "cache directory" is only used during development to save network bandwidth,
especially over ISDN / T1 lines.  If there is a cache directory specified, when
a file is found by the normal search path, it will be mirrored into the cache
directory, then opened there.


FIXME/TODO:
The file "parms.txt" will be read out of the game directory and appended to the
current command line arguments to allow different games to initialize startup
parms differently. This could be used to add a "-sspeed 22050" for the high
quality sound edition. Because they are added at the end, they will not override
an explicit setting on the original command line.

*/

/*
============
COM_SkipPath
============
*/
char *COM_SkipPath (const char *pathname)
{
	const char *last;

	last = pathname;
	while (*pathname)
	{
		if ((*pathname == '/') || (*pathname == '\\'))			// 2010-03-26: added backslash
			last = pathname+1;
		pathname++;
	}

	return (char *) last;
}

/*
============
COM_StripExtension
============
*/
int COM_StripExtension (const char *in, char *out, int bufsize)
{
	const char	*dot;
	int		len;

	/*if (!(dot = strrchr(in, '.')))
	{
		Q_strcpy (out, in, bufsize);
		return;
	}*/

	if (bufsize <= 0)
		return 0;

	len = strlen (in);
	if (len > 0)
	{
		dot = in + len - 1;
		while (*dot != '.')
		{
			if (--dot <= in+len-6)
			{
				//Q_strncpyz (out, in, len + 1);
				return Q_strcpy (out, in, bufsize);
			}
		}

		len = 0;
		while (*in && in != dot && --bufsize)
		{
			*out++ = *in++;
			len++;
		}
	}

	*out = 0;
	return len;
}

/*
============
COM_FileExtension
============
*/
char *COM_FileExtension (const char *in)
{
//	static	char	exten[8];
//	int		i;

	if (!(in = strrchr(in, '.')))
		return "";

/*	in++;

	for (i=0 ; i<7 && *in ; i++, in++)
		exten[i] = *in;
	exten[i] = 0;

	return exten;
*/
	return (char *)in + 1;
}

/*
============
COM_FileBase
============
*/
void COM_FileBase (const char *in, char *out, int bufsize)
{
	const char	*s, *s2;

	s = in + strlen(in) - 1;

	while (s != in && *s != '.')
		s--;

	for (s2 = s ; *s2 && *s2 != '/' ; s2--)
	;

	if (s-s2 < 2)
		Q_strcpy (out, "?model?", bufsize);
	else
	{
		s--;
		Q_strncpy (out, bufsize, s2+1, s-s2);
		//out[s-s2] = 0;
	}
}


/*
==================
COM_ForceExtension

If path doesn't have an extension or has a different extension, append specified extension
Extension should include the '.'
==================
*/
void COM_ForceExtension (char *path, const char *extension, int bufsize)
{
	int		len;
	char    *src;

	len = strlen (path);
	src = path + len - 1;

	while (*src != '/' && src != path)
	{
		if (*src-- == '.')
		{
			if (strcmp(src+1, extension))
			{
				Q_strcpy (src+1, extension, bufsize-(src+1-path));
			}
			return;
		}
	}

	Q_strcpy (path + len, extension, bufsize-len);
}

/*
==================
COM_DefaultExtension

If path doesn't have an extension, append extension
Extension should include the '.'
==================
*/
void COM_DefaultExtension (char *path, const char *extension, int bufsize)
{
	int		len;
	char    *src;

	len = strlen (path);
	src = path + len - 1;

	while (*src != '/' && *src != '\\' && src != path)
		if (*src-- == '.')
			return;			// it has an extension

	Q_strcpy (path+len, extension, bufsize-len);
}

/*
==================
COM_AddExtension

If path doesn't have the specified extension, append it
Extension should include the '.'
==================
*/
void COM_AddExtension (char *path, const char *extension, int bufsize)
{
	int		len;
	char    *src;

	len = strlen (path);
	src = path + len - 1;

	while (*src != '/' && *src != '\\' && src != path)
	{
		if (*src == '.')
		{
			if (!Q_strcasecmp(src, extension))
				return;			// it has the correct extension
			break;
		}
		src--;
	}

	Q_strcpy (path+len, extension, bufsize-len);
}


/*
==================
COM_ExpandPath (JDH)
  Takes a filename or partial/relative path and returns a fully resolved path.
  Uses com_gamedir as base for relative paths.
==================
*/
#define IS_LETTER(c) ((((c) >= 'A') && ((c) <= 'Z')) || (((c) >= 'a') && ((c) <= 'z')))

qboolean COM_ExpandPath (const char *fname, char *buf, int bufsize)
{
	int			len;
	char		*barename, *p/*, path[MAX_QPATH]*/;
//	qboolean	fullpath;

	len = Q_strcpy (buf, com_gamedir, bufsize);

	barename = COM_SkipPath (fname);
	if (barename == fname)
	{
		Q_snprintfz (buf+len, bufsize-len, "/%s", fname);
		return true;
	}

	if (*fname == '.')
	{
		do
		{
			fname++;
			if (*fname == '.')
			{
				p = COM_SkipPath (buf);
				if (p == buf)
				{
					*buf = 0;
					return false;
				}

				p[-1] = 0;
				len = p-buf-1;
				fname++;
			}

			if (*fname != '/' && *fname != '\\')
			{
				*buf = 0;
				return false;
			}
			fname++;
		}
		while (*fname == '.');

		Q_snprintfz (buf+len, bufsize-len, "/%s", fname);
		return true;
	}

#ifdef _WIN32
	if ((fname[0] == '/') || (fname[0] == '\\') || ((fname[1] == ':') && IS_LETTER(fname[0])))
#else
	if (fname[0] == '/')
#endif
	{
		Q_strcpy (buf, fname, bufsize);
	}
	else
	{
		Q_snprintfz (buf+len, bufsize-len, "/%s", fname);
	}

	return true;

/*	Q_strncpy (path, sizeof(path), fname, barename-fname-1);
	if (Sys_FolderExists (path))
	{
		Q_strcpy (buf, fname, bufsize);
		return true;
	}

	return false;
*/
}

/*
==================
COM_IsRealBSP
==================
*/
static const char *bsp_models[] =
{
	"batt0", "batt1", "bh10", "bh100", "bh25", "explob", "nail0",
	"nail1", "rock0", "rock1", "shell0", "shell1", "exbox2", NULL
};

qboolean COM_IsRealBSP (const char *bspname)
{
	int len, i;

	if (!COM_FilenamesEqualn (bspname, "b_", 2))
		return true;

	len = strlen(bspname);
	if ((len < 5) || !COM_FilenamesEqual (bspname + len - 4, ".bsp"))
		return true;

	bspname += 2;
	len -= 6;
	for (i = 0; bsp_models[i]; i++)
	{
		if (COM_FilenamesEqualn(bspname, bsp_models[i], len))
			return false;
	}

	return true;
}

/*
================
COM_FreeSearchpaths
================
*/
void COM_FreeSearchpaths (searchpath_t *lastsp)
{
	searchpath_t	*next;
	pack_t			*pak;
	int				curr_level = 99999;

	while (com_searchpaths != lastsp)
	{
		if ((pak = com_searchpaths->pack))
		{
			if (pak->handle)
				fclose (pak->handle);
			else if (pak->dzhandle)
				Dzip_Close (pak->dzhandle);

			free (pak->files);
			free (pak);
		}

	// each time we hit a new directory, check if it's a known mission pack
		if (curr_level != com_searchpaths->dir_level)
		{
			curr_level = com_searchpaths->dir_level;

		// if current gamedir is one of the recognized mission packs,
		//  and its switch isn't on the command-line (or if searchpaths
		//  are being reset to base), set its var to false

			if (COM_FilenamesEqual(com_searchpaths->dir_name, "rogue"))
			{
				if ((lastsp == com_base_searchpaths) || !COM_CheckParm("-rogue"))
					rogue = false;
			}
			else if (COM_FilenamesEqual(com_searchpaths->dir_name, "hipnotic"))
			{
				if ((lastsp == com_base_searchpaths) || !COM_CheckParm("-hipnotic"))
					hipnotic = false;
			}
			else if (COM_FilenamesEqual(com_searchpaths->dir_name, "quoth"))
			{
				if ((lastsp == com_base_searchpaths) || (!COM_CheckParm("-quoth") && !COM_CheckParm("-hipnotic")))
					hipnotic = false;
			}
			else if (COM_FilenamesEqual(com_searchpaths->dir_name, "nehahra"))
			{
				if (nehahra && ((lastsp == com_base_searchpaths) || !COM_CheckParm("-nehahra")))
				{
					Neh_UninitEnv ();
					nehahra = false;
				}
			}
		#ifdef HEXEN2_SUPPORT
			else if (COM_FilenamesEqual(com_searchpaths->dir_name, "hexen2"))
			{
				if (hexen2 && ((lastsp == com_base_searchpaths) || !COM_CheckParm("-hexen2")))
				{
					Hexen2_UninitEnv ();
					hexen2 = false;
				}
			}
		#endif
		}

		next = com_searchpaths->next;
		if (com_game_searchpaths == com_searchpaths)
			com_game_searchpaths = next;

		free (com_searchpaths);
		com_searchpaths = next;
	}
}

/*
================
COM_FindSearchpath (JDH)
================
*/
qboolean COM_FindSearchpath (const char *dir)
{
	searchpath_t *search;

	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (COM_FilenamesEqual (search->dir_name, dir))
			return true;
	}

	return false;
}

/*
================
COM_DzipIsMounted (JDH)
================
*/
qboolean COM_DzipIsMounted (const char *qpath, const char *dzname)
{
	searchpath_t *search;

	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (search->pack && search->pack->dzhandle)
		{
			if (COM_FilenamesEqual (search->dir_name, qpath) &&
				COM_FilenamesEqual (COM_SkipPath(search->pack->filename), dzname))
				return true;
		}
	}

	return false;
}

/*
=============
COM_LoadLibrary (JDH)
=============
*/
void * COM_LoadLibrary (const char *name)
{
	void *lib_handle;

	if ((lib_handle = Sys_LoadLibrary (va("./%s", name))))
		return lib_handle;

	if ((lib_handle = Sys_LoadLibrary (va("%s/%s", com_basedir, name))))
		return lib_handle;

	return Sys_LoadLibrary (name);
}

/*
=============
COM_UnloadLibrary (JDH)
=============
*/
void COM_UnloadLibrary (void *lib_handle)
{
	Sys_UnloadLibrary (lib_handle);
}

/*
================
COM_DeleteFile (JDH)
================
*/
qboolean COM_DeleteFile (const char *path)
{
	return (remove(path) == 0);
}

#ifdef HEXEN2_SUPPORT
/*
================
COM_GetTempPath (JDH)
- copies the temporary directory path to buf (includes trailing /)
- returns the length of the path string
================
*/
int COM_GetTempPath (char *buf, unsigned int buflen)
{
#ifdef _WIN32
	return GetTempPathA (buflen, buf);
#else
	int len;

	/*** DOES THIS WORK??? ***/
	len = Q_strcpy (buf, P_tmpdir, buflen);
	if (len && buf[len-1] != '/')
		buf[len++] = '/';
	return len;
#endif
}
#endif

/*
================
COM_FileLength
================
*/
int COM_FileLength (FILE *f)
{
	int	pos, end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

/*
=================
COM_FileOpenRead

Use this instead of Sys_FileOpenRead
=================
*/
int COM_FileOpenRead (const char *path, FILE **hndl)
{
	FILE	*f;

	if (!(f = fopen(path, "rb")))
	{
		*hndl = NULL;
		return -1;
	}
	*hndl = f;

	return COM_FileLength (f);
}

/*
=============
COM_FileExists
=============
*/
qboolean COM_FileExists (const char *path)
{
	FILE		*f;
	qboolean	retval;

/*#ifndef GLQUAKE
	int	t;

	t = VID_ForceUnlockedAndReturnState ();
#endif*/

	if ((f = fopen(path, "rb")))
	{
		fclose (f);
		retval = true;
	}
	else
	{
		retval = false;
	}

/*#ifndef GLQUAKE
	VID_ForceLockState (t);
#endif*/
	return retval;
}

/*
=============
COM_FilenamesEqual
=============
*/
qboolean COM_FilenamesEqualn (const char *f1, const char *f2, int len)
{
	if (len < 0)
	{
		if (com_matchfilecase.value)
			return !strcmp (f1, f2);

		return !Q_strcasecmp (f1, f2);
	}

	if (com_matchfilecase.value)
		return !strncmp (f1, f2, len);

	return !Q_strncasecmp (f1, f2, len);
}
/*
qboolean COM_PakFileMatches (char *pakfile, char *testfile, int flags)
{
	return COM_FilenameMatches (pakfile, testfile, -1, flags);
}
*/
/*
=================
COM_PakFileMatches

helper function for COM_FindFile
- checks whether the given filename from a pak matches the test filename
- if flags contains FILE_ANY_IMG, FILE_ANY_DEMO, FILE_ANY_MDL, or FILE_ANY_MUS,
   pakfile is compared to testfile with each known extension.
=================
*/
qboolean COM_PakFileMatches (const char *pakfile, const char *testfile, int flags)
{
	int  i, len, num_exts;
	char basename[MAX_OSPATH];
	const char **ext_list;

	if (!(flags & (FILE_ANY_IMG | FILE_ANY_DEMO | FILE_ANY_MDL | FILE_ANY_MUS)))
		return COM_FilenamesEqual (pakfile, testfile);

	if (flags & FILE_ANY_IMG)
	{
		ext_list = com_img_exts;
		num_exts = num_img_exts;
	}
	else if (flags & FILE_ANY_DEMO)
	{
		// if dzlib is not loaded, include files with .dz extension
		ext_list = com_demo_exts;
		num_exts = (dzlib_loaded ? NUM_DEMO_EXTS-1 : NUM_DEMO_EXTS);
	}
	else if (flags & FILE_ANY_MDL)
	{
		ext_list = com_mdl_exts;
		num_exts = NUM_MODEL_EXTS;
	}
	else
	{
		ext_list = com_music_exts;
		num_exts = 2;
	}

	COM_StripExtension (testfile, basename, sizeof(basename));
	len = strlen (basename);

	if (!COM_FilenamesEqualn (pakfile, basename, len))
		return false;

	for (i = 0; i < num_exts; i++)
	{
//		Q_strcpy (basename+len, ext_list[i], sizeof(basename)-len);
		if (COM_FilenamesEqual (pakfile+len, ext_list[i]))
		{
		//	Q_snprintfz (testfile, MAX_QPATH, "%s%s", basename, ext_list[i]);		removed 2010/03/03
			return true;
		}
	}

	return false;
}

// callback for COM_FindDirFiles in COM_FindInDir
//   - does nothing but copy the filename & halt searching after first match

int COM_DummyCallback (com_fileinfo_t *fileinfo, int count, unsigned param)
{
	Q_strcpy ((char *) param, fileinfo->name, MAX_QPATH);		// sizeof(foundfile) in COM_FindInDir
	return 1;		// stop searching
}

qboolean COM_FindInDir (searchpath_t *search, char *filename, int flags)		/*** modifies filename arg ***/
{
	char fullpath[MAX_OSPATH], foundfile[MAX_QPATH], *name;

	Q_snprintfz (fullpath, sizeof(fullpath), "%s/%s", search->filename, filename);
	if (COM_FindDirFiles (fullpath, search, flags, COM_DummyCallback, (unsigned) foundfile))
	{
//		if (flags & (FILE_ANY_IMG | FILE_ANY_DEMO | FILE_ANY_MDL | FILE_ANY_MUS))
			name = COM_SkipPath (filename);
			Q_strcpy (name, foundfile, MAX_QPATH-(name-filename));		// so file case & extension are correct

		return true;
	}

	return false;
}

/*
=============
COM_FindInDir
=============
*/
/*qboolean COM_FindInDir (searchpath_t *search, char *filename, int flags)
{
	char	netpath[MAX_OSPATH], **ext_list;
	int		i, len1, len2, num_exts;

// **** FIXME: should check host_matchfilecase ****


	if (!(flags & (FILE_ANY_IMG | FILE_ANY_DEMO | FILE_ANY_MDL | FILE_ANY_MUS)))
	{
		Q_snprintfz (netpath, sizeof(netpath), "%s/%s", search->filename, filename);
		return COM_FileExists(netpath);
	}

	if (flags & FILE_ANY_IMG)
	{
		ext_list = com_img_exts;
		num_exts = num_img_exts;
	}
	else if (flags & FILE_ANY_DEMO)
	{
		ext_list = com_demo_exts;
		num_exts = (dzlib_loaded ? NUM_DEMO_EXTS-1 : NUM_DEMO_EXTS);
	}
	else
	{
		ext_list = com_music_exts;
		num_exts = 2;
	}

	len1 = Q_snprintfz (netpath, sizeof(netpath), "%s/", search->filename);
	COM_StripExtension (filename, netpath+len1);
	len2 = strlen (netpath);

	for (i = 0; i < num_exts; i++)
	{
		Q_strcpy (netpath+len2, ext_list[i], sizeof(netpath)-len2);
		if (COM_FileExists(netpath))
		{
			strcpy (filename, netpath+len1);
			return true;
		}
	}

	return false;
}
*/

int COM_FindNamesInDir (searchpath_t *search, char *path, int pathlen, const char *names[], int flags)
		/**** call to COM_FindInDir modifies path arg ****/
{
	int i;

	for (i=0 ; names[i] ; i++)
	{
		Q_strcpy (path + pathlen, names[i], MAX_QPATH - pathlen);
		if (COM_FindInDir (search, path, flags))
		{
			return i;
		}
	}

	return -1;
}

/*
==============
COM_FindMultifile                            - FIXME: merge with COM_FindAllFiles
 - non-empty paths must have trailing '/'
 - last item in paths and names arrays must be NULL
 - number of items may not exceed MAX_PATTERNS per array
 - fileinfo may be NULL if file info is not needed
==============
*/
qboolean COM_FindMultifile (const char *paths[], const char *names[], int flags, com_fileinfo_t *info_out)
{
	int				pathlens[MAX_MULTISOURCE_NAMES], i, j, k;
	searchpath_t	*search;
	pack_t			*pak;
	char			qname[MAX_QPATH];

#if 0
//#if defined(_DEBUG) && defined(_WIN32)
	extern HWND mainwindow;
	
	for (i = 0; names[i]; i++)
		if (strchr(names[i], '/'))
			MessageBoxA (mainwindow, "COM_FindMultifile: Path separator found in filename", "Debug warning", MB_OK);
#endif

	// calculate these just once to save time
	if (paths && paths[0])
	{
		for (i=0 ; paths[i] ; i++)
			pathlens[i] = strlen (paths[i]);
	}
	else pathlens[0] = 0;

	// search through the path, one element at a time
	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (search->dir_level < (flags & 0x00FF))
			break;

		if ((pak = search->pack))
		{
			if (pak->handle && (flags & FILE_NO_PAKS))
				continue;

			if (pak->dzhandle && (flags & FILE_NO_DZIPS))
				continue;

			for (i=0 ; i<pak->numfiles ; i++)
			{
				// make sure the prefix matches a path:
				j = 0;
				if (paths && paths[0])
				{
					for ( ; paths[j] ; j++)
					{
						if (COM_FilenamesEqualn (pak->files[i].name, paths[j], pathlens[j]))
							break;
					}

					if (!paths[j])
						continue;		// doesn't match any of the paths
				}

				for (k=0 ; names[k] ; k++)
				{
					if (COM_PakFileMatches (pak->files[i].name + pathlens[j], names[k], flags))
					{
						if (info_out)
						{
							Q_strcpy (info_out->name, pak->files[i].name, MAX_QPATH),
							info_out->searchpath = search;
							info_out->index = i;
							info_out->filepos = pak->files[i].filepos;
							info_out->filelen = pak->files[i].filelen;
							info_out->isdir = false;
						}
						return true;
					}
				}
			}
		}
		else
		{
			if (paths && paths[0])
			{
				i = 0;		// shut up compiler warning
				for (j=0 ; paths[j] ; j++)
				{
					Q_strcpy (qname, paths[j], MAX_QPATH);

					i = COM_FindNamesInDir (search, qname, pathlens[j], names, flags);
					if (i >= 0)
						break;
				}
			}
			else
			{
				i = COM_FindNamesInDir (search, qname, 0, names, flags);
			}

			if (i >= 0)
			{
				if (info_out)
				{
					Q_strcpy (info_out->name, qname, MAX_QPATH),
					info_out->searchpath = search;
					info_out->index = -1;
					info_out->filepos = 0;
					info_out->filelen = 0;		// FIXME
					info_out->isdir = false;
				}

				return true;
			}
		}
	}

	return false;
}

/*
=================
COM_FindFile

Searches the folder hierarchy for the given file (searches inside paks as well)

JDH: flags can be used to restrict where the file is looked for
    - the low byte specifies the minimum directory level ID
	   (0 --> any folder; 1 --> all but base "ID1" folder; etc.)
	- this can be OR'ed with any of the FILE_xxxx flags
	- if FILE_ANY_IMG, FILE_ANY_DEMO, FILE_ANY_MDL or FILE_ANY_MUS is used,
	  and a match is found, filename will contain it on return
=================
*/
qboolean COM_FindFile (const char *filename, int flags, com_fileinfo_t *info_out)
{
	const char *namelist[2];
	
	namelist[0] = filename;
	namelist[1] = NULL;

	return COM_FindMultifile (NULL, namelist, flags, info_out);
}

/*
================
COM_FilenameMatches
================
*/
qboolean COM_FilenameMatches (const char *filename, const char *testname, int wildcardpos, int flags)
{
	int		num_exts, len, suffixlen, i;
	const char **ext_list;

	if (wildcardpos >= 0)
	{
		if (wildcardpos)
		{
			// compare the strings up to the wildcard
			if (!COM_FilenamesEqualn (filename, testname, wildcardpos))
				return false;
		}

		filename += wildcardpos + 1;
		testname += wildcardpos + 1;
	}

	len = strlen (filename);

	if (!(flags & (FILE_ANY_IMG | FILE_ANY_DEMO | FILE_ANY_MDL | FILE_ANY_MUS)))
	{
		if (wildcardpos >= 0)
		{
			suffixlen = strlen (testname);
			if (len < suffixlen)
				return false;

			filename += len-suffixlen;
		}
		return COM_FilenamesEqual (filename, testname);
	}

	if (flags & FILE_ANY_IMG)
	{
		ext_list = com_img_exts;
		num_exts = num_img_exts;
	}
	else if (flags & FILE_ANY_DEMO)
	{
		// include files with .dz extension only if dzlib is not loaded
		ext_list = com_demo_exts;
		num_exts = (dzlib_loaded ? NUM_DEMO_EXTS-1 : NUM_DEMO_EXTS);
	}
	else if (flags & FILE_ANY_MDL)
	{
		ext_list = com_mdl_exts;
		num_exts = NUM_MODEL_EXTS;
	}
	else
	{
		ext_list = com_music_exts;
		num_exts = 2;
	}

	// this assumes that any wildcard immediately precedes the suffix
	for (i = 0; i < num_exts; i++)
	{
		suffixlen = strlen (ext_list[i]);
		if (len < suffixlen)
			continue;

		if (COM_FilenamesEqual (filename + len - suffixlen, ext_list[i]))
			return true;
	}

	return false;
}

/*
================
COM_FindDirFiles
================
*/
int COM_FindDirFiles (const char *path, const searchpath_t *search, int flags,
						   FFCALLBACK callproc, unsigned int callparam)
{
	int			num_exts, len, i, result, count = 0;
	char		netpath[MAX_OSPATH];
	const char	**ext_list;
	qboolean	dirs_only = (flags & FILE_DIRS_ONLY) ? true : false;

	if (!(flags & (FILE_ANY_IMG | FILE_ANY_DEMO | FILE_ANY_MDL | FILE_ANY_MUS)))
	{
		return Sys_ListFilesInDir (path, search, dirs_only, &count, callproc, callparam);
	}

	if (flags & FILE_ANY_IMG)
	{
		ext_list = com_img_exts;
		num_exts = num_img_exts;
	}
	else if (flags & FILE_ANY_DEMO)
	{
		// if dzlib is loaded and we are in a known searchpath,
		//  don't include files with .dz extension
		ext_list = com_demo_exts;
		num_exts = ((search && dzlib_loaded) ? NUM_DEMO_EXTS-1 : NUM_DEMO_EXTS);
	}
	else if (flags & FILE_ANY_MDL)
	{
		ext_list = com_mdl_exts;
		num_exts = NUM_MODEL_EXTS;
	}
	else
	{
		ext_list = com_music_exts;
		num_exts = 2;
	}

	COM_StripExtension (path, netpath, sizeof(netpath));
	len = strlen (netpath);

	for (i = 0; i < num_exts; i++)
	{
		Q_strcpy (netpath+len, ext_list[i], sizeof(netpath)-len);
		result = Sys_ListFilesInDir (netpath, search, dirs_only, &count, callproc, callparam);
		if (result)
			return result;
	}

	return 0;
}

/*
============
COM_FindAllFiles - FIXME: merge with COM_FindMultifile
============
*/

int COM_FindAllFiles (const char *paths[], const char *name, int flags, FFCALLBACK callproc, unsigned int callparam)
{
	int				i, j, count, result, wildcard;
	int				pathlens[MAX_MULTISOURCE_NAMES];
	searchpath_t	*search;
	char			netpath[MAX_OSPATH], *p;
	pack_t			*pak;
	com_fileinfo_t	fileinfo;

#if 0
//#if defined(_DEBUG) && defined(_WIN32)
	extern HWND mainwindow;
	
	if (strchr(name, '/'))
		MessageBoxA (mainwindow, "COM_FindAllFiles: Path separator found in filename", "Debug warning", MB_OK);
#endif
	
	p = strchr (name, '*');
	wildcard = (p ? p-name : -1);

	// calculate these just once to save time
	if (paths && paths[0])
	{
		for (i=0 ; paths[i] ; i++)
			pathlens[i] = strlen (paths[i]);
	}
	else pathlens[0] = 0;

	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (search->dir_level < (flags & 0x00FF))
			break;

		count = 0;

		if ((pak = search->pack))
		{
			if (pak->handle && (flags & FILE_NO_PAKS))
				continue;

			if (pak->dzhandle && (flags & FILE_NO_DZIPS))
				continue;

			for (i=0 ; i<pak->numfiles ; i++)
			{
				p = pak->files[i].name;

				// make sure the prefix matches a path:
				j = 0;
				if (paths && paths[0])
				{
					for ( ; paths[j] ; j++)
					{
						if (COM_FilenamesEqualn (p, paths[j], pathlens[j]))
							break;
					}

					if (!paths[j])
						continue;		// doesn't match any of the paths
				}

				if (COM_FilenameMatches (p + pathlens[j], name, wildcard, flags))
				{
					Q_strcpy (fileinfo.name, COM_SkipPath (p), sizeof(fileinfo.name));
					//fileinfo.name[ strlen( fileinfo.name ) - suffixlen ] = 0;
					fileinfo.filelen = pak->files[i].filelen;
					fileinfo.filepos = pak->files[i].filepos;
					fileinfo.searchpath = search;
					fileinfo.index = i;
					fileinfo.isdir = false;

					result = callproc (&fileinfo, count++, callparam);
					if (result)
						return result;
				}
			}
		}
		else
		{
			if (paths && paths[0])
			{
				for (j=0 ; paths[j] ; j++)
				{
					Q_snprintfz (netpath, sizeof(netpath), "%s/%s%s", search->filename, paths[j], name);

					result = COM_FindDirFiles (netpath, search, flags, callproc, callparam);
					if (result)
						return result;
				}
			}
			else
			{
				Q_snprintfz (netpath, sizeof(netpath), "%s/%s", search->filename, name);

				result = COM_FindDirFiles (netpath, search, flags, callproc, callparam);
				if (result)
					return result;
			}
		}
	}

	return 0;
}

/*
============
COM_Path_f
============
*/
void COM_Path_f (cmd_source_t src)
{
	searchpath_t	*s;

	Con_Print ("Current search path (in the order they are searched):\n");
	for (s = com_searchpaths ; s ; s = s->next)
	{
		if ((s == com_base_searchpaths) || (s == com_game_searchpaths))
			Con_Print ("------------\n");
		if (s->pack)
			Con_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Con_Printf ("%s\n", s->filename);
	}
}

/*
============
COM_WriteFile

The filename will be prefixed by the current game directory
============
*/
qboolean COM_WriteFile (const char *filename, const void *data, int len)
{
	FILE	*f;
	char	name[MAX_OSPATH];

	Q_snprintfz (name, MAX_OSPATH, "%s/%s", com_gamedir, filename);

	if (!(f = fopen(name, "wb")))
	{
		COM_CreatePath (name);
		if (!(f = fopen(name, "wb")))
			return false;
	}

	Sys_Printf ("COM_WriteFile: %s\n", name);
	fwrite (data, 1, len, f);
	fclose (f);

	return true;
}

/*
============
COM_CreatePath

Only used for CopyFile
============
*/
void COM_CreatePath (const char *path)
{
	char	*ofs;

	for (ofs = (char *)path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{       // create the directory
			*ofs = 0;
			Sys_mkdir (path);
			*ofs = '/';
		}
	}
}

/*
===========
COM_CopyFile

Copies a file over from the net to the local cache, creating any directories
needed. This is for the convenience of developers using ISDN from home.
===========
*/
qboolean COM_CopyFile (const char *netpath, const char *cachepath)
{
	FILE	*in, *out;
	int	remaining, count;
	char	buf[4096];

	remaining = COM_FileOpenRead (netpath, &in);
	if (remaining < 0)
		return false;

	COM_CreatePath (cachepath);	// create directories up to the cache file
	if (!(out = fopen(cachepath, "wb")))
	{
		//Sys_Error ("Error opening %s", cachepath);
		fclose (in);
		return false;
	}

	while (remaining)
	{
		if (remaining < sizeof(buf))
			count = remaining;
		else
			count = sizeof(buf);
		fread (buf, 1, count, in);
		fwrite (buf, 1, count, out);
		remaining -= count;
	}

	fclose (in);
	fclose (out);
	return true;
}

/*
=================
COM_FOpenFromInfo
 - opens for reading the file specified by fileinfo
   (fileinfo should be obtained via a COM_Find___ call)
=================
*/
FILE * COM_FOpenFromInfo (const com_fileinfo_t *fileinfo)
{
	pack_t	*pak;
	FILE    *file;

	pak = fileinfo->searchpath->pack;
	if (pak)
	{
		if (developer.value)
			Sys_Printf ("PackFile: %s : %s\n", pak->filename, fileinfo->name);

		// open a new file on the pakfile
		if (pak->handle)
		{
			file = fopen (pak->filename, "rb");
		}
		else if (pak->dzhandle)
		{
			file = Dzip_ExtractFile (pak->dzhandle, fileinfo->index);
		}
		else
			file = NULL;

		if (!file)
			Sys_Error ("Couldn't reopen %s", pak->filename);

		fseek (file, fileinfo->filepos, SEEK_SET);
		com_filesize = fileinfo->filelen;
		Q_snprintfz (com_netpath, sizeof(com_netpath), "%s#%i", pak->filename, fileinfo->index);
	}
	else
	{
		Q_snprintfz (com_netpath, sizeof(com_netpath), "%s/%s", fileinfo->searchpath->filename, fileinfo->name);
		if (!(file = fopen(com_netpath, "rb")))
			Sys_Error ("Couldn't open %s", com_netpath);

		if (developer.value)
			Sys_Printf ("COM_FOpenFromInfo: %s\n", com_netpath);

		com_filesize = COM_FileLength (file);
	}

	com_filepath = fileinfo->searchpath;
	return file;
}

/*
=================
COM_FOpen_MultiSource
 - see COM_FindMultifile and COM_FOpenFile for usage
=================
*/
FILE * COM_FOpen_MultiSource (const char *paths[], const char *names[], int flags, com_fileinfo_t *fi_out)
{
	FILE *f;

	if (COM_FindMultifile (paths, names, flags, fi_out))
	{
		f = COM_FOpenFromInfo (fi_out);
		if (f)
			fi_out->filelen = com_filesize;
		return f;
	}

	com_filesize = -1;
	com_netpath[0] = 0;
	com_filepath = NULL;	// JDH
	return NULL;
}

/*
=================
COM_FOpenFile

Finds the file in the search path.
If found, and filelen_out is non-null, sets it to file's length
=================
*/
FILE * COM_FOpenFile (const char *filename, int flags, com_fileinfo_t *fi_out)
{
	const char		*namelist[2];
//	char			*pathlist[2] = {"", NULL};
	com_fileinfo_t	fi;
	FILE			*file;

	namelist[0] = filename;
	namelist[1] = NULL;

	if (!fi_out)
		fi_out = &fi;

//	filesize = COM_FOpen_MultiSource (pathlist, namelist, flags, fi_out, &file);
	file = COM_FOpen_MultiSource (NULL, namelist, flags, fi_out);
	if (!file)
	{
		if (developer.value && !(flags & FILE_ANY_IMG))
			Sys_Printf ("COM_FOpenFile: can't find %s\n", filename);
		return NULL;
	}

	return file;
}

/*
=================
COM_LoadFile

Filename are relative to the quake directory.
Always appends a 0 byte.
=================
*/
cache_user_t 	*loadcache;
byte    		*loadbuf;
int             loadsize;

byte * COM_LoadFromFile (FILE *h, const com_fileinfo_t *fi, memtype_t memtype, byte *buffer, int bufsize)
{
	char	base[32];
	byte	*buf = NULL;     // quiet compiler warning

	// extract the filename base name for hunk tag
	COM_FileBase (fi->name, base, sizeof(base));

	if (memtype == memtype_hunk)
	{
		buf = Hunk_AllocName (fi->filelen + 1, base);
	}
	else if (memtype == memtype_temp)
	{
		buf = Hunk_TempAlloc (fi->filelen + 1);
	}
	else if (memtype == memtype_zone)
	{
		buf = Z_Malloc (fi->filelen + 1);
	}
/*	else if (memtype == memtype_cache)
	{
		buf = Cache_Alloc (loadcache, fi->filelen + 1, base);
	}
*/	else if (memtype == memtype_stack)
	{
		if (fi->filelen > bufsize)
			buf = Hunk_TempAlloc (fi->filelen + 1);
		else
			buf = buffer;
	}
	else if (memtype == memtype_malloc)		// JDH: added
	{
		buf = Q_malloc (fi->filelen + 1);
	}
	else
	{
		Sys_Error ("COM_LoadFile: bad memtype");
	}

	if (!buf)
		Sys_Error ("COM_LoadFile: not enough space for %s", fi->name);

	if (buf != loadbuf)		// JDH: otherwise this trashes mem!
		buf[fi->filelen] = 0;

#ifndef RQM_SV_ONLY
	Draw_BeginDisc ();
#endif

	fread (buf, 1, fi->filelen, h);

#ifndef RQM_SV_ONLY
	Draw_EndDisc ();
#endif

	return buf;
}

byte *COM_LoadFile (const char *path, int memtype, int flags)
{
	FILE			*h;
	byte			*buf;
	com_fileinfo_t	fi;

	// look for it in the filesystem or pack files
	h = COM_FOpenFile (path, flags, &fi);
	if (!h)
		return NULL;

	buf = COM_LoadFromFile (h, &fi, memtype, loadbuf, loadsize);

	fclose (h);
	return buf;
}

byte *COM_LoadHunkFile (const char *path, int flags)
{
	return COM_LoadFile (path, memtype_hunk, flags);
}

byte *COM_LoadTempFile (const char *path, int flags)
{
	return COM_LoadFile (path, memtype_temp, flags);
}

/*void COM_LoadCacheFile (const char *path, struct cache_user_s *cu)
{
	loadcache = cu;
	COM_LoadFile (path, memtype_cache, 0);
}
*/
// uses temp hunk if larger than bufsize
byte *COM_LoadStackFile (const char *path, void *buffer, int bufsize, int flags)
{
	byte	*buf;

	loadbuf = (byte *)buffer;
	loadsize = bufsize;
	buf = COM_LoadFile (path, memtype_stack, flags);

	return buf;
}

// JDH:
byte *COM_LoadMallocFile (const char *path, int flags)
{
	return COM_LoadFile (path, memtype_malloc, flags);
}

/*
=================
COM_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
pack_t *COM_LoadPackFile (const char *packfile)
{
	dpackheader_t	header;
	packfile_t	*newfiles;
	int		i, numpackfiles;
	pack_t		*pack;
	FILE		*packhandle;
	dpackfile_t	info[MAX_FILES_IN_PACK];

	if (COM_FileOpenRead(packfile, &packhandle) == -1)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (memcmp(header.id, "PACK", 4))
		Sys_Error ("%s is not a packfile", packfile);
	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error ("%s has %i files", packfile, numpackfiles);

	newfiles = Q_malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (&info, 1, header.dirlen, packhandle);

	// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		Q_strcpy (newfiles[i].name, info[i].name, sizeof(newfiles[i].name));
		newfiles[i].filepos = LittleLong (info[i].filepos);
		newfiles[i].filelen = LittleLong (info[i].filelen);
	}

	pack = Q_malloc (sizeof(pack_t));
	Q_strcpy (pack->filename, packfile, sizeof(pack->filename));
	pack->handle = packhandle;
	pack->dzhandle = NULL;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;

	if (con_initialized)
		Con_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);

	return pack;
}

/*
================
COM_AddNewSearchpath
================
*/
void COM_AddNewSearchpath (pack_t *pack)
{
	searchpath_t	*search;

	search = Q_malloc (sizeof(searchpath_t));

	// JDH: record path to paks as well as folders
	Q_strcpy (search->filename, com_gamedir, sizeof(search->filename));
	Q_strcpy (search->dir_name, com_gamedirname, sizeof(search->dir_name));
	search->pack = pack;
	if (pack)
	{
		// same level for all paks + files in a folder
		// (com_searchpaths is never null because /ID1 folder is added first)
		search->dir_level = com_searchpaths->dir_level;
	}
	else
	{
		// each new folder gets 1 + dir_level of previous folder
		search->dir_level = (com_searchpaths ? com_searchpaths->dir_level+1 : 0);
	}

	search->next = com_searchpaths;
	com_searchpaths = search;
}

/*
================
COM_AddDzip
- callback for COM_FindDirFiles in COM_AddDirectoryFiles
================
*/
int COM_AddDzip (com_fileinfo_t *file, int count, unsigned int param)
{
	return COM_AddDzipByName (file->name);
}

/*
================
COM_AddDzipByName
- used by callback above, and also when adding newly created dzip
================
*/
int COM_AddDzipByName (char *name)
{
	char	filepath[MAX_OSPATH];
	pack_t	*pak;

	Q_snprintfz (filepath, sizeof(filepath), "%s/%s", com_gamedir, name);

	pak = Dzip_LoadFileList (filepath);
	if (pak)
		COM_AddNewSearchpath (pak);

	return 0;
}

/*
================
COM_AddPak
- callback for COM_FindDirFiles in COM_AddDirectoryFiles
================
*/
int COM_AddPak (com_fileinfo_t *fileinfo, int count, unsigned int numpaks)
{
	int		i;
	char	pakfile[MAX_OSPATH];
	pack_t	*pak;

//	if (file->name[0] == '.')
//		return;

// JDH: check that it's not one of the standard pak names (already added)
	for (i = 0; i < numpaks; i++)
	{
		if (COM_FilenamesEqual (fileinfo->name, va("pak%i.pak", i)))
			return 0;
	}

	Q_snprintfz (pakfile, sizeof(pakfile), "%s/%s", com_gamedir, fileinfo->name);

	pak = COM_LoadPackFile (pakfile);
	if (pak)
		COM_AddNewSearchpath (pak);

	return 0;
}

/*
================
COM_AddDirectoryFiles

JDH: internal function combining common code of COM_AddGameDirectory and COM_SetGameDir
     Requires that com_gamedir already be set to desired path
================
*/
void COM_AddDirectoryFiles (void)
{
//	searchpath_t	*sp;
	pack_t			*pack;
	char			pakfile[MAX_OSPATH];
	int				i, numpaks;

	// make sure dir is not already in searchpaths
	if (COM_FindSearchpath (com_gamedirname))
		return;

// add the directory to the search path
#ifdef HEXEN2_SUPPORT
	if (!COM_FilenamesEqual (com_gamedirname, "hexen2"))		// H2 adds dir _after_ paks
#endif
		COM_AddNewSearchpath (NULL);


// add any pak files in the format pak0.pak pak1.pak, ...
/***JDH - added back in to maintain proper search order - JDH***/
	for (i=0 ; ; i++)
	{
		Q_snprintfz (pakfile, sizeof(pakfile), "%s/pak%i.pak", com_gamedir, i);
		if (!(pack = COM_LoadPackFile(pakfile)))
			break;

		COM_AddNewSearchpath (pack);
	}
	numpaks = i;
/***JDH - added back in to maintain proper search order - JDH***/


// add ANY pak file - MrG (biteme@telefragged.com)
	Q_snprintfz (pakfile, sizeof(pakfile), "%s/*.pak", com_gamedir);
	COM_FindDirFiles (pakfile, NULL, 0, COM_AddPak, numpaks);
// end of PAK code update

	if (dzlib_loaded)
	{
		i = Q_snprintfz (pakfile, sizeof(pakfile)-1, "%s/*.dz", com_gamedir);
		COM_FindDirFiles (pakfile, NULL, 0, COM_AddDzip, 0);

		strcpy (pakfile+i-2, "zip");
		COM_FindDirFiles (pakfile, NULL, 0, COM_AddDzip, 0);

		strcpy (pakfile+i-2, "pk3");
		COM_FindDirFiles (pakfile, NULL, 0, COM_AddDzip, 0);
	}


// if gamedir is one of the recognized missionpacks, set its variable to true
	if (COM_FilenamesEqual (com_gamedirname, "rogue"))
		rogue = true;
	else if (COM_FilenamesEqual (com_gamedirname, "hipnotic"))
		hipnotic = true;
	else if (COM_FilenamesEqual (com_gamedirname, "quoth"))
		hipnotic = true;
	else if (COM_FilenamesEqual (com_gamedirname, "nehahra"))
	{
		if (!nehahra)
		{
			nehahra = true;
			Neh_InitEnv ();
		}
	}
#ifdef HEXEN2_SUPPORT
	else if (COM_FilenamesEqual (com_gamedirname, "hexen2"))
	{
		COM_AddNewSearchpath (NULL);		// directory itself
		if (!hexen2)
		{
			hexen2 = true;
			Hexen2_InitEnv ();
		}
		/*rogue = hipnotic = false;
		if (nehahra)
		{
			if (host_initialized)
				Neh_UninitEnv ();
			nehahra = false;
		}*/
	}
#endif
}


/*
================
COM_AddGameDirectory

Sets com_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ...
================
*/
void COM_AddGameDirectory (const char *gamedir)
{
	char fullpath[MAX_OSPATH];

	Q_snprintfz (fullpath, sizeof(fullpath), "%s/%s", com_basedir, gamedir);
	if (!Sys_FolderExists (fullpath))
	{
		Con_Printf ("  Invalid gamedir: %s\n", gamedir);
		return;
	}

	Q_strcpy (com_gamedirname, gamedir, sizeof(com_gamedirname));
	Q_strcpy (com_gamedir, fullpath, sizeof(com_gamedir));

/****JDH****/
	COM_AddDirectoryFiles();
/****JDH****/

	// initializing demodir
//	Q_snprintfz (demodir, sizeof(demodir), "/%s", com_gamedirname);
}

/*
================
COM_SetGameDir

Sets the gamedir and path to a different directory.
================
*/
void COM_SetGameDir (const char *dir)
{
	Q_strcpy (com_gamedirname, dir, sizeof(com_gamedirname));
	Q_snprintfz (com_gamedir, sizeof(com_gamedir), "%s/%s", com_basedir, dir);

// special case: "gamedir id1" clears all game switches
	if (COM_FilenamesEqual(dir, GAMENAME))
	{
		COM_FreeSearchpaths (com_base_searchpaths);
		com_game_searchpaths = com_base_searchpaths;
		return;
	}

	COM_AddDirectoryFiles();
}

#define GAMEDIR_WARNING \
  "Changing gamedir requires you to\n" \
  "quit your current game. Continue?\n" \
  "(\x02Y/\x02N)"

/*
================
COM_Gamedir_f

Sets the gamedir and path to a different directory.
  JDH: now supports more than 1 argument
================
*/
void COM_Gamedir_f (cmd_source_t src)
{
	int			argc, i;
	const char	*dir;
	char		fullpath[MAX_OSPATH];

	if (src == SRC_CLIENT)
		return;

	argc = Cmd_Argc ();
	if (argc == 1)
	{
		Con_Print ("  Usage: gamedir <newdir> [newdir2] ...\n");
		Con_Printf ("  Current gamedir: %s\n", com_gamedirname);
		return;
	}

// Verify each directory before doing anything:
	for (i = 1; i < argc; i++)
	{
		dir = Cmd_Argv (i);

		if (strstr(dir, "..") || strstr(dir, "/") || strstr(dir, "\\") || strstr(dir, ":"))
		{
			Con_Print ("  Gamedir should be a single filename, not a path\n");
			return;
		}

		//if (COM_FilenamesEqual (com_gamedirname, dir))
		//	return;		// still the same

		Q_snprintfz (fullpath, sizeof(fullpath), "%s/%s", com_basedir, dir);
		if (!Sys_FolderExists (fullpath))
		{
			Con_Printf ("  Invalid gamedir: %s\n", dir);
			return;
		}
	}
#ifndef RQM_SV_ONLY
//	if (cls.state != ca_disconnected)
	if (cls.state == ca_connected)
	{
//		Con_Print ("you must disconnect before changing gamedir\n");

		int result = SCR_ModalMessage (GAMEDIR_WARNING, "YN");
		if (result == 0)
		{
			Cbuf_AddText ("disconnect\n", SRC_COMMAND);
			Cbuf_AddText ("gamedir", SRC_COMMAND);
			for (i = 1; i < argc; i++)
				Cbuf_AddText (va(" %s", Cmd_Argv (i)), SRC_COMMAND);
			Cbuf_AddText ("\n", SRC_COMMAND);
		}
		return;
	}
#endif

	Host_WriteConfiguration ();

	// free up any current game dir info
	COM_FreeSearchpaths (com_game_searchpaths);

	// flush all data, so it will be forced to reload
	Cache_Flush (src);

	Q_strcpy (com_gamedirname, com_game_searchpaths->dir_name, sizeof(com_gamedirname));
	Q_strcpy (com_gamedir, com_game_searchpaths->filename, sizeof(com_gamedir));

#ifndef RQM_SV_ONLY
	// important to do this BEFORE COM_SetGameDir, since loading files can result
	//  in screen updates or pic loading
	if (cls.state != ca_dedicated)
		scr_disabled_for_loading = true;
#endif

	for (i = 1; i < argc; i++)
		COM_SetGameDir (Cmd_Argv (i));

#ifdef HEXEN2_SUPPORT
	Q_strcpy (com_savedir, com_gamedir, sizeof(com_savedir));
#endif

// 2010/05/30 - moved these 3 before palette/wad load (so image cvars are current)
	Key_ResetAll ();
	Cvar_ResetAll ();
	Host_ExecCfgs ();

#ifndef RQM_SV_ONLY
// disable screen while we're reloading graphics
	if (cls.state != ca_dedicated)
	{
		Host_ReloadPalette ();
		Draw_ReloadWadFile ();

		scr_disabled_for_loading = false;
	}
#endif		//#ifndef RQM_SV_ONLY

}

/*
============
COM_InitImageLibs
============
*/
#ifndef RQM_SV_ONLY
void COM_InitImageLibs (void)
{
	int i = 0;

	Image_Init ();

	com_img_exts[i++] = ".tga";
	if (png_available)
		com_img_exts[i++] = ".png";
	if (jpg_available)
		com_img_exts[i++] = ".jpg";
	com_img_exts[i++] = ".pcx";
	num_img_exts = i;
}
#endif		//#ifndef RQM_SV_ONLY

/*
================
COM_InitFilesystem
================
*/
void COM_InitFilesystem (void)
{
	int	i, len;
	char *subdir;
	int rogue_parm, hipnotic_parm, nehahra_parm, quoth_parm;
#ifdef HEXEN2_SUPPORT
	int hexen2_parm;
#endif
/*
#ifdef _DEBUG
	char buf[32];
	//COM_StripExtension ("abcdefg.hij", buf, 5);
	Q_strcpy (buf, "abcd", 5);
	Q_strcpy (buf, "abcdefg", 4);
	Q_strncpy (buf, 5, "abdefg", 4);
#endif
*/
	Cmd_AddCommand ("path", COM_Path_f, 0);
	Cmd_AddCommand ("gamedir", COM_Gamedir_f, 0);
	Cmd_AddLegacyCommand ("game", "gamedir");		// for FitzQuake people

	Cvar_RegisterBool (&com_matchfilecase);

// -basedir <path>
// Overrides the system supplied base directory (under GAMENAME)
	if ((i = COM_CheckParm ("-basedir")) && i < com_argc-1)
		Q_strcpy (com_basedir, com_argv[i+1], sizeof(com_basedir));
	else
		Q_strcpy (com_basedir, host_parms.basedir, sizeof(com_basedir));

	len = strlen (com_basedir);
	for (i=0 ; i < len; i++)
		if (com_basedir[i] == '\\')
			com_basedir[i] = '/';

	if (i > 0 && com_basedir[i-1] == '/')
		com_basedir[i-1] = 0;

// load helper libs:
	if (!COM_CheckParm ("-nodzlib"))
		dzlib_loaded = Dzip_Init();

#ifndef RQM_SV_ONLY
	COM_InitImageLibs ();
#endif

	if (Sys_FolderExists (va("%s/%s", com_basedir, RQMDIR)))
		COM_AddGameDirectory (RQMDIR);

	// start up with GAMENAME by default (id1)
	COM_AddGameDirectory (GAMENAME);
	com_base_searchpaths = com_searchpaths;

/*********JDH**********/
	//COM_AddGameDirectory ("base");	// JT021305 - use base as default GAMENAME

/******JDH - moved from COM_InitArgv: ******/
	rogue_parm = COM_CheckParm("-rogue");
	hipnotic_parm = COM_CheckParm("-hipnotic");
	nehahra_parm = COM_CheckParm("-nehahra");
	quoth_parm = COM_CheckParm("-quoth");

#ifdef HEXEN2_SUPPORT
	hexen2_parm = COM_CheckParm("-hexen2");
	if (hexen2_parm)
	{
		if (rogue_parm || hipnotic_parm || nehahra_parm || quoth_parm)
		{
			Con_Print ("WARNING: hexen2 cannot be combined with other mission packs\n");
			rogue_parm = hipnotic_parm = nehahra_parm = quoth_parm = 0;
		}
	}
#endif

	// add any mission pack switches in the order they appear:
	for (i = 1; i < com_argc; i++)
	{
		if (i == rogue_parm) subdir = "rogue";
		else if (i == hipnotic_parm) subdir = "hipnotic";
		else if (i == nehahra_parm) subdir = "nehahra";
		else if (i == quoth_parm)
		{
			//COM_AddGameDirectory ("hipnotic");
			hipnotic = true;
			subdir = "quoth";
		}
	#ifdef HEXEN2_SUPPORT
		else if (i == hexen2_parm) subdir = "hexen2";
	#endif
		else continue;

		COM_AddGameDirectory (subdir);
	}
/*********JDH**********/

	// any set gamedirs will be freed up to here
	com_game_searchpaths = com_searchpaths;

// -game <gamedir>
// Adds basedir/gamedir as an override game
	i = -1;
	while ((i = COM_FindNextParm("-game", i+2)) && (i < com_argc-1))
	{
       	COM_AddGameDirectory (com_argv[i+1]);
	}

#ifdef HEXEN2_SUPPORT
/*	i = COM_CheckParm ("-savedir");
	if (i)
	{
		Q_snprintfz (com_savedir, sizeof(com_savedir), "%s/"GAMENAME, basedir);
	}
	else*/
	{
		Q_strcpy (com_savedir, com_gamedir, sizeof(com_savedir));
	}
#endif
}

/*
================
COM_Shutdown
================
*/
void COM_Shutdown (void)
{
	COM_FreeSearchpaths (NULL);

	if (dzlib_loaded)
	{
		Dzip_Shutdown();
		dzlib_loaded = false;
	}
}
