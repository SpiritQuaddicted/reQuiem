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
// common.h  -- general definitions


// Create our own define for Mac OS X
#if defined(__APPLE__) && defined(__MACH__)
# define MACOSX
#endif

#if !defined BYTE_DEFINED
typedef unsigned char 		byte;
#define BYTE_DEFINED 1
#endif

#if defined(_WIN32) && defined(_MSC_VER)
#  define QINT64 __int64
#else
#  define QINT64 __S64_TYPE
#endif

#undef	true
#undef	false

typedef enum {false, true}	qboolean;

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

// by joe
#define bound(a, b, c) ((a) >= (c) ? (a) : (b) < (a) ? (a) : (b) > (c) ? (c) : (b))

//============================================================================

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte		*data;
	int		maxsize;
	int		cursize;
	int		lastcmdpos;			// JDH: position of last command (server-side only)
} sizebuf_t;

void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void SZ_CheckSpace (sizebuf_t *buf, int length);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, const char *data);	// appends string to the sizebuf

//============================================================================

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);

// (type *)STRUCT_FROM_LINK(link_t *link, type, member)
// ent = STRUCT_FROM_LINK(link,entity_t,order)
// FIXME: remove this mess!
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (int)&(((t *)0)->m)))

//============================================================================

#define Q_MAXCHAR	((char)0x7f)
#define Q_MAXSHORT	((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG	((int)0x7fffffff)
#define Q_MAXFLOAT	((int)0x7fffffff)

#define Q_MINCHAR	((char)0x80)
#define Q_MINSHORT	((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG	((int)0x80000000)
#define Q_MINFLOAT	((int)0x7fffffff)

//============================================================================

/****************** JDH ****************/
//extern	qboolean	bigendien;

extern	short	(*BigShort) (short l);
extern	int		(*BigLong) (int l);
extern	float	(*BigFloat) (float l);

//extern	short			(*LittleShort) (short l);
//extern	int				(*LittleLong) (int l);
//extern	float			(*LittleFloat) (float l);


#define LittleLong(l)   (l)
#define LittleShort(l)  (l)
#define LittleFloat(l)  (l)
/****************** JDH ****************/

//============================================================================

#define	MAXPRINTMSG	4096	/* MSG_ReadString, Con_Print */

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteCmd (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);
void MSG_WritePreciseAngle (sizebuf_t *sb, float f);	// precise aim from [sons]Quake

#define MSG_WriteFloatCoord MSG_WriteFloat		// taken from DP for extensions

extern	int			msg_readcount;
extern	qboolean	msg_badread;		// set if a read goes beyond end of message

void  MSG_BeginReading (void);
int   MSG_ReadChar (void);
int   MSG_ReadByte (void);
int   MSG_ReadShort (void);
int   MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);
float MSG_ReadPreciseAngle (void);	// precise aim from [sons]Quake


//============================================================================

#ifdef _WIN32

#define	vsnprintf _vsnprintf

#define Q_strcasecmp  _stricmp
#define Q_strncasecmp _strnicmp

#else

#define Q_strcasecmp  strcasecmp
#define Q_strncasecmp strncasecmp

#endif

//int Q_strcpy (char *dest, const char *src, int bufsize);
//int Q_strncpy (char *dest, const char *src, int count);		/* NO null-termination */
//int Q_strncpyz (char *dest, const char *src, int size);

#define Q_strcpy(dest, src, size) Q_strncpy((dest), (size), (src), Q_MAXINT)
int Q_strncpy (char *dest, int bufsize, const char *src, int count);
int Q_snprintfz (char *dest, int size, const char *fmt, ...);

int Q_atoi (const char *str);
float Q_atof (const char *str);

char *va (const char *format, ...);
// does a varargs printf into a temp buffer

char *CopyString (const char *s);

//============================================================================

extern	char		com_token[1024];
extern	qboolean	com_eof;

const char *COM_Parse (const char *data);


extern	int	com_argc;
extern	const char	**com_argv;

#define COM_CheckParm(p) COM_FindNextParm((p), 1)

int   COM_FindNextParm (const char *parm, int start);
void  COM_Init (void);
void  COM_InitArgv (int argc, const char **argv, const char *cmdline);
void  COM_InitFilesystem (void);
void *COM_LoadLibrary (const char *name);
void  COM_UnloadLibrary (void *);
void  COM_Shutdown (void);

char *COM_SkipPath (const char *pathname);
int   COM_StripExtension (const char *in, char *out, int bufsize);
char *COM_FileExtension (const char *in);
void  COM_FileBase (const char *in, char *out, int bufsize);
void COM_ForceExtension (char *path, const char *extension, int bufsize);	// by joe
void  COM_DefaultExtension (char *path, const char *extension, int bufsize);
void  COM_AddExtension (char *path, const char *extension, int bufsize);
qboolean COM_ExpandPath (const char *fname, char *buf, int bufsize);

qboolean COM_IsRealBSP (const char *bspname);

qboolean COM_FilenamesEqualn (const char *f1, const char *f2, int len);
#define COM_FilenamesEqual(f1, f2) COM_FilenamesEqualn((f1), (f2), -1)

qboolean COM_FilenameMatches (const char *filename, const char *testname, int wildcardpos, int flags);


// these deal with files independent of the Quake file system:
int COM_FileLength (FILE *f);				//
void COM_CreatePath (const char *path);			//
qboolean COM_FileExists (const char *path);
qboolean COM_DzipIsMounted (const char *qpath, const char *dzname);


//============================================================================

/****************** JDH ****************/
// flags for COM_FOpenFile/COM_LoadFile/COM_FindFile/COM_FindAllFiles/COM_FindDirFiles/COM_LoadHunkFile
// NOTE: low-byte is reserved and must be 0

#define FILE_NO_PAKS       0x8000		/* used for dzips */
#define FILE_NO_DZIPS      0x4000		/* used for mp3/ogg */
#define FILE_DIRS_ONLY     0x1000		/* return directories only */
#define FILE_ANY_IMG       0x0100		/* match given filename with any image extension */
#define FILE_ANY_DEMO      0x0200		/* match given filename with any demo extension */
#define FILE_ANY_MDL       0x0400		/* match given filename with any alias model extension (mdl/md2/md3) */
#define FILE_ANY_MUS       0x0800		/* match given filename with any music extension (mp3/ogg) */
/****************** JDH ****************/

extern int com_filesize;
extern char	com_netpath[MAX_OSPATH];

struct cache_user_s;

extern	char	com_gamedir[MAX_OSPATH];
extern	char	com_basedir[MAX_OSPATH];	// by joe
extern	char	com_gamedirname[MAX_QPATH];		// JDH

#ifdef HEXEN2_SUPPORT
  extern  char	com_savedir[MAX_OSPATH];
#endif

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen;
} packfile_t;

typedef struct pack_s
{
	char		filename[MAX_OSPATH];
	FILE		*handle;	// only if pak
	void		*dzhandle;	// only if dzip
	int			numfiles;
	packfile_t	*files;
} pack_t;

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;          // may be NULL
	int		dir_level;		// JDH: to quickly compare paths; 0 = ID1, 1 = <next path>, ...
	char	dir_name[MAX_QPATH];
	struct searchpath_s *next;
} searchpath_t;

// JDH: struct used to return search results
typedef struct
{
	char				name[MAX_QPATH];
	const searchpath_t	*searchpath;
	int					index;			// -1 for files not in paks/dzips
	int					filepos, filelen;
	qboolean			isdir;
} com_fileinfo_t;

extern searchpath_t	*com_searchpaths;
extern const searchpath_t	*com_filepath;		// JDH: to store path of last loaded file

// JDH: for COM_FindMultifile/COM_FOpen_MultiSource: max number of items per array
#define MAX_MULTISOURCE_NAMES 4

int COM_FileOpenRead (const char *path, FILE **hndl);

qboolean COM_FindMultifile (const char *paths[], const char *names[], int flags, com_fileinfo_t *info_out);
qboolean COM_FindFile (const char *filename, int flags, com_fileinfo_t *info_out);
qboolean COM_DeleteFile (const char *path);
qboolean COM_CopyFile (const char *netpath, const char *cachepath);

#ifdef HEXEN2_SUPPORT
  extern qboolean hexen2;

  int COM_GetTempPath (char *buf, unsigned int buflen);
#endif

typedef int (*FFCALLBACK) (com_fileinfo_t *, int count, unsigned int param);

int COM_FindAllFiles (const char *paths[], const char *name, int flags, FFCALLBACK, unsigned int param);
int COM_FindDirFiles (const char *path, const searchpath_t *, int flags, FFCALLBACK, unsigned int param);

qboolean COM_WriteFile (const char *filename, const void *data, int len);
FILE * COM_FOpenFile (const char *filename, int flags, com_fileinfo_t *fi_out);
FILE * COM_FOpen_MultiSource (const char *paths[], const char *names[], int flags, com_fileinfo_t *fi_out);
FILE * COM_FOpenFromInfo (const com_fileinfo_t *fileinfo);

typedef enum {memtype_zone, memtype_hunk, memtype_temp, memtype_cache, memtype_stack, memtype_malloc} memtype_t;

byte *COM_LoadStackFile (const char *path, void *buffer, int bufsize, int flags);
byte *COM_LoadTempFile (const char *path, int flags);
byte *COM_LoadHunkFile (const char *path, int flags);
//void COM_LoadCacheFile (const char *path, struct cache_user_s *cu);
byte *COM_LoadMallocFile (const char *path, int flags);
byte * COM_LoadFromFile (FILE *h, const com_fileinfo_t *fi, memtype_t memtype, byte *buffer, int bufsize);

extern	struct	cvar_s	registered;

extern qboolean hipnotic, rogue, nehahra;

#define NUM_MODEL_EXTS 3
extern const char * com_mdl_exts[NUM_MODEL_EXTS];
