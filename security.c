//
// security.c
//
// Created by JPG for ProQuake v3.20
//

#include "quakedef.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

qboolean	pq_cheatfreeEnabled = false;
qboolean	pq_cheatfree = false;
unsigned long	qsmackAddr;
qboolean	qsmackActive = false;		// only allow one qsmack connection
unsigned long	net_seed;
cvar_t		pq_cvar_cheatfree = {"pq_cheatfree", "0"};

Security_InitCRC_t Security_InitCRC = NULL;
Security_SetSeed_t Security_SetSeed = NULL;
Security_Decode_t Security_Decode = NULL;
Security_Encode_t Security_Encode = NULL;
Security_CRC_t Security_CRC = NULL;
Security_CRC_File_t Security_CRC_File = NULL;
Security_Verify_t Security_Verify = NULL;

#ifdef _WIN32
#define PROCADDRESS GetProcAddress
#else
#define PROCADDRESS dlsym
#endif

#define	GETFUNC(f) if (!(Security_##f = (Security_##f##_t) PROCADDRESS(h, "Security_" #f))) goto error

void Security_Init (void)
{
#ifdef _WIN32
	HINSTANCE	h = LoadLibrary ("qsecurity.dll");
#else
	void		*h = dlopen ("./qsecurity.so", RTLD_LAZY);
#endif

	if (!h)
		goto error;

	GETFUNC(InitCRC);
	GETFUNC(SetSeed);
	GETFUNC(Decode);
	GETFUNC(Encode);
	GETFUNC(CRC);
	GETFUNC(CRC_File);
	GETFUNC(Verify);

	Security_InitCRC (0x3915c28b);

	pq_cheatfreeEnabled = true;

	Cvar_RegisterBool (&pq_cvar_cheatfree);
	Con_Print ("Security module initialized\n");

	return;

error:
	Con_Print ("\x02" "Security module not found\n");
}
