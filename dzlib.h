
/***********************************************************************************
dzlib (c) 2009 jdhack@hotmail.com
based on dzip 2.9

Notes:
- this library is NOT thread-safe
- big-endian support is present, but untested
- zip support is very limited:
   - compression method must be either deflate or store
   - no passwords, encryption, or zip64 extensions
   - DZ_AddFile works only if the DZHANDLE was obtained via DZ_CreateZip
     (ie. only if the zip was created by dzlib, and the handle has not been closed)
***********************************************************************************/


#define DZLIB_VERSION		0x00000904		/* 0.94 */

#define DZ_ERR_NONE         0x00000000
#define DZ_ERR_OUTOFMEM     0x00000001
#define DZ_ERR_TOOMANYOPEN  0x00000002
#define DZ_ERR_BADHANDLE    0x00000003
#define DZ_ERR_BADPARAM     0x00000004
#define DZ_ERR_BUFTOOSMALL  0x00000005
#define DZ_ERR_UNSUPPORTED  0x00000006

#define DZ_ERR_FILENOTFOUND 0x00000010
#define DZ_ERR_FILEWRITE    0x00000011
#define DZ_ERR_FILEREAD     0x00000012
#define DZ_ERR_FILECREATE   0x00000013
#define DZ_ERR_FILETOOBIG   0x00000014
#define DZ_ERR_FILESEEK     0x00000015

#define DZ_ERR_BADHEADER    0x00000020
#define DZ_ERR_BADTABLE     0x00000021
#define DZ_ERR_BADFILEENTRY 0x00000022

typedef int DZBOOL;
typedef unsigned int DZUINT;
typedef unsigned int DZHANDLE;

/* these must match the TYPE_xxxx enums in dzip.h: */
typedef enum 
{ 
	DZ_FTYPE_NORMAL, 
	DZ_FTYPE_DEMV1, 
	DZ_FTYPE_TXT, 
	DZ_FTYPE_PAK, 
	DZ_FTYPE_DZ, 
	DZ_FTYPE_DEM,
	DZ_FTYPE_NEHAHRA, 
	DZ_FTYPE_DIR, 
	DZ_FTYPE_STORE 
} DZFTYPE;

typedef struct 
{
    unsigned short year;	/* full year */
    unsigned short month;	/* 1 to 12 */
    unsigned short day;		/* 1 to 31 */
    unsigned short hour;	/* 0 to 23 */
    unsigned short minute;
    unsigned short second;
} DZTIME;

typedef struct
{
	char     name[260];
	DZFTYPE  type;
	DZUINT   size;
	DZTIME   time;
} DZENTRY;

typedef struct
{
	int      struct_size;
	char     name[260];
	DZFTYPE  type;
	DZUINT   size;
	DZTIME   time;
	DZUINT   crc;
	DZUINT   size_compressed;
} DZENTRYEX;


#ifndef DZIP_DLL_API
#ifdef _WIN32
#define DZIP_DLL_API __declspec(dllimport)
#else
#define DZIP_DLL_API
#endif
#endif

/***********************************************************************************/
/* DZ_GetVersion: the value returned is to be interpreted as follows:              */
/*     In hex: 0x00MMmmbb, where MM is the major version, mm is the minor version, */
/*     and bb is the build number.  So, for example, 0x00020104 would be v2.1.4    */
/*                                                                                 */
/* DZ_GetFileInfoEx: before calling, be sure to set struct_size field of DZENTRYEX */
/*     to sizeof(DZENTRYEX)                                                        */
/***********************************************************************************/


// function prototypes for dynamic loading:
typedef DZHANDLE (*DZ_OPEN_PROC)(const char *);
typedef DZBOOL   (*DZ_CLOSE_PROC)(DZHANDLE);
typedef DZBOOL   (*DZ_SETFILEPATH_PROC)(DZHANDLE, const char *path);
typedef int      (*DZ_GETNUMFILES_PROC)(DZHANDLE);
typedef DZBOOL   (*DZ_GETFILEINFO_PROC)(DZHANDLE, DZUINT index, DZENTRY *entry);
typedef DZBOOL   (*DZ_GETFILEINFOEX_PROC)(DZHANDLE, DZUINT index, DZENTRYEX *entry);
typedef DZBOOL   (*DZ_VERIFY_PROC)(DZHANDLE, DZUINT index);
typedef DZBOOL   (*DZ_VERIFYALL_PROC)(DZHANDLE);
typedef DZBOOL   (*DZ_EXTRACT_PROC)(DZHANDLE, DZUINT index);
typedef int      (*DZ_EXTRACTTEMP_PROC)(DZHANDLE, DZUINT index, char *pathbuf, int bufsize);
typedef DZBOOL   (*DZ_EXTRACTALL_PROC)(DZHANDLE);
typedef int      (*DZ_GETLASTERROR_PROC)(void);
typedef DZUINT   (*DZ_GETVERSION_PROC)(void);

#ifndef UNDZIP_ONLY
typedef DZHANDLE (*DZ_CREATE_PROC) (const char *filepath);
typedef DZHANDLE (*DZ_CREATEZIP_PROC) (const char *filepath);
typedef DZBOOL   (*DZ_ADDFILE_PROC) (DZHANDLE, const char *filepath, DZFTYPE);
#endif

// function prototypes for static linking:
DZIP_DLL_API DZHANDLE DZ_Open (const char *filepath);
DZIP_DLL_API DZBOOL   DZ_Close (DZHANDLE);
DZIP_DLL_API DZBOOL   DZ_SetFilePath (DZHANDLE, const char *path);
DZIP_DLL_API int      DZ_GetNumFiles (DZHANDLE);
DZIP_DLL_API DZBOOL   DZ_GetFileInfo (DZHANDLE, DZUINT index, DZENTRY *entry);
DZIP_DLL_API DZBOOL   DZ_GetFileInfoEx (DZHANDLE, DZUINT index, DZENTRYEX *entry);
DZIP_DLL_API DZBOOL   DZ_Verify (DZHANDLE, DZUINT index);
DZIP_DLL_API DZBOOL   DZ_VerifyAll (DZHANDLE);
DZIP_DLL_API DZBOOL   DZ_Extract (DZHANDLE, DZUINT index);
DZIP_DLL_API int      DZ_ExtractTemp (DZHANDLE, DZUINT index, char *pathbuf, int bufsize);
DZIP_DLL_API DZBOOL   DZ_ExtractAll (DZHANDLE);
DZIP_DLL_API int      DZ_GetLastError (void);
DZIP_DLL_API DZUINT   DZ_GetVersion (void);

#ifndef UNDZIP_ONLY
DZIP_DLL_API DZHANDLE DZ_Create (const char *filepath);
DZIP_DLL_API DZHANDLE DZ_CreateZip (const char *filepath);
DZIP_DLL_API DZBOOL   DZ_AddFile (DZHANDLE, const char *filepath, DZFTYPE);
#endif

