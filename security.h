//
// security.h
//
// Created by JPG for ProQuake v3.20
//

#ifndef SECURITY_H
#define SECURITY_H

extern qboolean		pq_cheatfreeEnabled;
extern qboolean		pq_cheatfree;
extern unsigned long	net_seed;
extern unsigned long	net_seed75;
extern unsigned long	qsmackAddr;
extern qboolean		qsmackActive;
extern cvar_t		pq_cvar_cheatfree;

// Functions exported by security module
typedef void (*Security_InitCRC_t) (unsigned k);
typedef void (*Security_SetSeed_t) (unsigned long seed, char *filename);
typedef void (*Security_Encode_t) (unsigned char *src, unsigned char *dst, int len, int to);
typedef void (*Security_Decode_t) (unsigned char *src, unsigned char *dst, int len, int from);
typedef unsigned long (*Security_CRC_t) (unsigned char *data, int len);
typedef unsigned (*Security_CRC_File_t) (char *filename);
typedef int (*Security_Verify_t) (unsigned a, unsigned b);

extern Security_InitCRC_t Security_InitCRC;
extern Security_SetSeed_t Security_SetSeed;
extern Security_Decode_t Security_Decode;
extern Security_Encode_t Security_Encode;
extern Security_CRC_t Security_CRC;
extern Security_CRC_File_t Security_CRC_File;
extern Security_Verify_t Security_Verify;

void Security_Init (void);

#endif
