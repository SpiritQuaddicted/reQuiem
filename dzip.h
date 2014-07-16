
qboolean Dzip_Init (void);
void Dzip_Shutdown (void);

pack_t *Dzip_LoadFileList (const char *dzpath);
void Dzip_Close (void *dzhandle);

FILE * Dzip_ExtractFile (void *dzhandle, int index);
FILE * Dzip_OpenFromArchive (const char *dzpath, const char *filename);

qboolean Dzip_CompressFile (const char *path, const char *filename, const char *dzname, qboolean stdzip);
qboolean Dzip_Verify (const char *dzpath);
