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
// modelgen.h: header file for model generation program

// *********************************************************
// * This file must be identical in the modelgen directory *
// * and in the Quake directory, because it's used to      *
// * pass data from one to the other via model files.      *
// *********************************************************

// must match definition in spritegn.h
#ifndef SYNCTYPE_T
#define SYNCTYPE_T
typedef enum {ST_SYNC=0, ST_RAND} synctype_t;
#endif


#define DT_FACES_FRONT	0x0010

// This mirrors trivert_t in trilib.h, is present so Quake knows how to
// load this data

typedef struct 
{
	byte	v[3];
	byte	lightnormalindex;
} trivertx_t;

typedef struct 
{
	int		onseam;
	int		s;
	int		t;
} stvert_t;

#define IDPOLYHEADER	(('O'<<24) + ('P'<<16) + ('D'<<8) + 'I')	// little-endian "IDPO"
#define MD2IDHEADER     (('2'<<24) + ('P'<<16) + ('D'<<8) + 'I')	// little-endian "IDP2"
#define MD3IDHEADER     (('3'<<24) + ('P'<<16) + ('D'<<8) + 'I')	// little-endian "IDP3"
#define RAPOLYHEADER	(('O'<<24) + ('P'<<16) + ('A'<<8) + 'R')	// little-endian "RAPO"
