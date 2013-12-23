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
// quakedef.h -- primary header for client

#define	QUAKE_GAME			// as opposed to utilities

//define	PARANOID		// speed sapping error checking

#define	GAMENAME	"id1"		// directory to look in by default
#define RQMDIR  "reQuiem_base"		// JDH: dir for engine-specific files


/******JDH******/
#define HEXEN2_SUPPORT
#define ETWAR_SUPPORT
/******JDH******/

// disable data conversion warnings
#ifdef _WIN32
#  ifdef _MSC_VER
#    pragma warning (disable : 4244)     // MIPS
#    pragma warning (disable : 4136)     // X86
#    pragma warning (disable : 4051)     // ALPHA
//#    pragma warning (disable : 4244 4127 4201 4214 4514 4305 4115 4018)
#    pragma warning (disable : 4127 4201 4305 4115 4018 4100 4706)
	// 4018: signed/unsigned mismatch (on comparison)
	// 4100: unreferenced parameter
	// 4115: named type definition in parentheses
	// 4127: constant conditional expression (usu. while(1))
	// 4201: non-standard expression (nameless union)
	// 4210: non-standard expression (extern function declared locally)
	// 4305: truncation (eg. from double to float; from int to float)
	// 4706: assignment within conditional expression [ if ((x = y))...]
#  endif
#endif

#include <math.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WIN32
#  define __USE_LARGEFILE64		/* JDH: for movie capturing */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>

#if defined(_WIN32) && !defined(WINDED)

void	VID_LockBuffer (void);
void	VID_UnlockBuffer (void);

#endif

#if id386
#define UNALIGNED_OK	1	// set to 0 if unaligned accesses are not supported
#else
#define UNALIGNED_OK	0
#endif

#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings

#define	MINIMUM_MEMORY			0x550000
#define	MINIMUM_MEMORY_LEVELPAK		(MINIMUM_MEMORY + 0x100000)

#define MAX_NUM_ARGVS	50

// up / down
#define	PITCH		0

// left / right
#define	YAW		1

// fall over
#define	ROLL		2


#define	MAX_QPATH		64			// max length of a quake game pathname
//#define	MAX_OSPATH		128			// JDH: increased
#ifdef _WIN32
#define	MAX_OSPATH		260			// max length of a filesystem pathname (MAX_PATH)
#else
#define	MAX_OSPATH		256			// max length of a filesystem pathname
#endif

#define	ON_EPSILON		0.1			// point on plane side epsilon

/**********************JDH********************/
#define	MAX_MSGLEN_OLD		 8000		// max length of a reliable message
#define	MAX_MSGLEN			(NETFLAG_DATA - 1 - NET_HEADERSIZE)		// max length of a reliable message (65527)

#define	MAX_DATAGRAM_OLD	 1024			// max length of unreliable message
#define	MAX_DATAGRAM		 MAX_MSGLEN		// max length of unreliable message

#ifdef HEXEN2_SUPPORT
  #define	MAX_MSGLEN_H2		20000		// for mission pack tibet2
  #define	MAX_DATAGRAM_H2		 1024		// max length of unreliable message
#endif
/**********************JDH********************/


// per-level limits
#define	MAX_EDICTS_OLD		 600
#define	MAX_EDICTS			4096

#define	MAX_LIGHTSTYLES		64

/**********************JDH********************/
#define	MAX_MODELS_OLD		 256			// these are sent over the net as bytes
#define	MAX_SOUNDS_OLD		 256			// so they cannot be blindly increased

#define	MAX_MODELS			1024			// these are sent over the net as shorts
#define	MAX_SOUNDS			1024
/**********************JDH********************/


#ifdef HEXEN2_SUPPORT
  #define MAX_MODELS_H2		512			// Sent over the net as a word
  #define MAX_SOUNDS_H2		512			// byte + overflow bit
#endif

#define	SAVEGAME_COMMENT_LENGTH	39

#define	MAX_STYLESTRING		64

// stats are integers communicated to the client by the server
#define	MAX_CL_STATS		32
#define	STAT_HEALTH			0
#define	STAT_FRAGS			1
#define	STAT_WEAPON			2
#define	STAT_AMMO			3
#define	STAT_ARMOR			4
#define	STAT_WEAPONFRAME	5
#define	STAT_SHELLS			6
#define	STAT_NAILS			7
#define	STAT_ROCKETS		8
#define	STAT_CELLS			9
#define	STAT_ACTIVEWEAPON	10
#define	STAT_TOTALSECRETS	11
#define	STAT_TOTALMONSTERS	12
#define	STAT_SECRETS		13		// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14		// bumped by svc_killedmonster
#define	STAT_ITEMS			15		// for QW demos

// stock defines

#define	IT_SHOTGUN				1
#define	IT_SUPER_SHOTGUN		2
#define	IT_NAILGUN				4
#define	IT_SUPER_NAILGUN		8
#define	IT_GRENADE_LAUNCHER		16
#define	IT_ROCKET_LAUNCHER		32
#define	IT_LIGHTNING			64
#define IT_SUPER_LIGHTNING      128
#define IT_SHELLS               256
#define IT_NAILS                512
#define IT_ROCKETS              1024
#define IT_CELLS                2048
#define IT_AXE                  4096
#define IT_ARMOR1               8192
#define IT_ARMOR2               16384
#define IT_ARMOR3               32768
#define IT_SUPERHEALTH          65536
#define IT_KEY1                 131072
#define IT_KEY2                 262144
#define	IT_INVISIBILITY			524288
#define	IT_INVULNERABILITY		1048576
#define	IT_SUIT					2097152
#define	IT_QUAD					4194304
#define IT_SIGIL1               (1<<28)
#define IT_SIGIL2               (1<<29)
#define IT_SIGIL3               (1<<30)
#define IT_SIGIL4               (1<<31)

//===========================================
//rogue changed and added defines

#define RIT_SHELLS              128
#define RIT_NAILS               256
#define RIT_ROCKETS             512
#define RIT_CELLS               1024
#define RIT_AXE                 2048
#define RIT_LAVA_NAILGUN        4096
#define RIT_LAVA_SUPER_NAILGUN  8192
#define RIT_MULTI_GRENADE       16384
#define RIT_MULTI_ROCKET        32768
#define RIT_PLASMA_GUN          65536
#define RIT_ARMOR1              8388608
#define RIT_ARMOR2              16777216
#define RIT_ARMOR3              33554432
#define RIT_LAVA_NAILS          67108864
#define RIT_PLASMA_AMMO         134217728
#define RIT_MULTI_ROCKETS       268435456
#define RIT_SHIELD              536870912
#define RIT_ANTIGRAV            1073741824
#define RIT_SUPERHEALTH         2147483648

//===========================================
//MED 01/04/97 added hipnotic defines

#define	HIT_PROXIMITY_GUN_BIT	16
#define	HIT_MJOLNIR_BIT			7
#define	HIT_LASER_CANNON_BIT	23
#define	HIT_PROXIMITY_GUN		(1<<HIT_PROXIMITY_GUN_BIT)
#define	HIT_MJOLNIR				(1<<HIT_MJOLNIR_BIT)
#define	HIT_LASER_CANNON		(1<<HIT_LASER_CANNON_BIT)
#define	HIT_WETSUIT				(1<<(23+2))
#define	HIT_EMPATHY_SHIELDS		(1<<(23+3))

//===========================================

#ifdef HEXEN2_SUPPORT
  #define ART_HASTE						1
  #define ART_INVINCIBILITY				2
  #define ART_TOMEOFPOWER				4
  #define ART_INVISIBILITY				8
  #define ARTFLAG_FROZEN				128
  #define ARTFLAG_STONED				256
  #define ARTFLAG_DIVINE_INTERVENTION	512

  #define NUM_CLASSES					5
  #define NUM_DIFFLEVELS				4
#endif

//#define	MAX_SCOREBOARD		16		//JDH: --> 32 to match MAX_CLIENTS for QW demos
#define	MAX_SCOREBOARD		32
#define	MAX_SCOREBOARDNAME	32

#define	SOUND_CHANNELS		8

/*
Commands can come from various sources, but the handler functions may choose
to disallow the action or forward it to a remote server if the source is
not appropriate.
*/
typedef enum
{
	SRC_CLIENT,		// came in over a net connection as a clc_stringcmd
					//  (host_client will be valid during this state)
	SRC_COMMAND,	// from the command buffer

// JDH: added extra values to distinguish source of commands in cbuf
	SRC_CFG,		// from standard configs (quake.rc, default.cfg, autoexec.cfg or config.cfg)
	SRC_CFG_RQM,	// from reQuiem.cfg
	SRC_CMDLINE,	// from command-line
	SRC_CONSOLE,	// from console
	SRC_SERVER		// from server, via svc_stufftext
} cmd_source_t;


#include "common.h"
#include "bspfile.h"
#include "vid.h"
#include "sys.h"
#include "zone.h"
#include "mathlib.h"

typedef struct
{
	vec3_t	origin;
	vec3_t	angles;
	int		modelindex;
	int		frame;
	int		colormap;
	int		skin;
	int		effects;
	float	transparency;		// for Fitz protocol
#ifdef HEXEN2_SUPPORT
	byte	scale;
	byte	drawflags;
	byte	abslight;
	byte	flags;
	short	index;
	byte	ClearCount[32];
#endif
} entity_state_t;


#ifdef HEXEN2_SUPPORT
  #define ENT_STATE_ON		1
  #define ENT_CLEARED		2

  #define MAX_CLIENT_STATES 150
  #define MAX_FRAMES		5

  typedef struct
  {
	entity_state_t states[MAX_CLIENT_STATES];
	int count;
  } client_frames_t;

  typedef struct
  {
	  entity_state_t states[MAX_CLIENT_STATES*2];
	  int count;
  } client_frames2_t;

  typedef struct
  {
	client_frames_t frames[MAX_FRAMES+2]; // 0 = base, 1-max = proposed, max+1 = too late
  } client_state2_t;
#endif


// disable data conversion warnings
#ifdef _WIN32
#  include <windows.h>
#else
#  define WORD  unsigned short
#  define DWORD unsigned long
#endif

#include "cvar.h"

#include "net.h"
#include "protocol.h"
#include "cmd.h"
#include "progs.h"
#include "server.h"

#include "model.h"
#include "world.h"
#include "keys.h"
#include "console.h"
#include "view.h"
#include "crc.h"
#include "version.h"

#ifndef RQM_SV_ONLY
#include "wad.h"
#include "draw.h"
#include "screen.h"
#include "sbar.h"
#include "sound.h"
#include "render.h"
#include "client.h"
#include "input.h"
#include "menu.h"
#include "cdaudio.h"
#include "glquake.h"
#endif

#include "nehahra.h"

//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct
{
	const char	*basedir;
	int			argc;
	const char	**argv;
	void		*membase;
	int			memsize;
} quakeparms_t;

//=============================================================================

extern	qboolean	noclip_anglehack;

// host
extern	quakeparms_t	host_parms;

extern	cvar_t		sys_ticrate;
//extern	cvar_t		sys_nostdout;
extern	cvar_t		developer;

extern	qboolean	host_initialized;	// true if into command execution
extern	double		host_frametime;
extern	byte		*host_basepal;
extern	byte		*host_colormap;
extern	int			host_framecount;	// incremented every frame, never reset
extern	double		realtime;		// not bounded in any way, changed at
						// start of every frame, never reset

void Host_ClearMemory (void);
void Host_ServerFrame (void);
void Host_InitCommands (void);
void Host_Init (quakeparms_t *parms);
void Host_Shutdown (void);
void Host_Error (const char *error, ...);
void Host_EndGame (const char *message, ...);
qboolean Host_PrintToClient (const char *msg);
void Host_Frame (double time);
void Host_Quit (void);
void Host_Quit_f (cmd_source_t src);
void Host_Reconnect_f (cmd_source_t src);
void Host_ClientCommands (const char *fmt, ...);
void Host_ExecCfgs (void);
void Host_ShutdownServer (qboolean crash);

void Host_SetMapName (const char *name);
const char *Host_MapName (void);
void Host_ReloadPalette (void);
void Host_WriteConfiguration (void);

#ifdef HEXEN2_SUPPORT
  void Host_Class_f (cmd_source_t src);
  void Host_Changelevel2_f (cmd_source_t src);
  void Host_RestoreClients (void);
  void Host_SaveGamestate (qboolean ClientsOnly);
  qboolean  Host_LoadGamestate (const char *level, const char *startspot, int ClientsMode);
  void Host_RemoveGIPFiles (const char *path);
#endif

void Host_Savegame_f (cmd_source_t src);
void Host_Loadgame_f (cmd_source_t src);


extern qboolean		msg_suppress_1;		// suppresses resolution and cache size console output
						// an fullscreen DIB focus gain/loss
extern int		current_skill;		// skill level for currently loaded level (in case
						// the user changes the cvar while the level is
						// running, this reflects the level actually in use)

extern qboolean		isDedicated;

extern int		minimum_memory;

#ifdef HEXEN2_SUPPORT
  void Hexen2_InitEnv (void);
  void Hexen2_UninitEnv (void);
  void Hexen2_InitVars (void);
#endif

// chase
extern	cvar_t	chase_active;

void Chase_Init (void);
void Chase_Reset (void);
void Chase_Update (void);
