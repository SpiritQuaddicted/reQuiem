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
// client.h

#ifdef HEXEN2_SUPPORT
#include "cl_effect_H2.h"
#endif

typedef struct lightstyle_s
{
	int		length;
	char	map[MAX_STYLESTRING];
	char	average, peak;		// JDH: for r_flatlightstyles (from Fitz)
	struct lightstyle_s *prev;		// JDH: linked list of previous styles, used for demo rewind
} lightstyle_t;

typedef struct
{
	char	name[MAX_SCOREBOARDNAME];
	float	entertime;
	int		frags;
	int		colors;			// two 4 bit fields
	byte	translations[VID_GRADES*256];
#ifdef HEXEN2_SUPPORT
	float	playerclass;
#endif
} scoreboard_t;

typedef struct
{
	int	destcolor[3];
	int	percent;		// 0-256
} cshift_t;

#define	CSHIFT_CONTENTS	0
#define	CSHIFT_DAMAGE	1
#define	CSHIFT_BONUS	2
#define	CSHIFT_POWERUP	3
#define	NUM_CSHIFTS		4


// client_state_t should hold all pieces of the client state

#define	SIGNONS		4		// signon messages to receive before connected

#define	MAX_DLIGHTS	64		// joe: doubled
							// JDH: NOTE: msurface_t.dlightbits must be at least this wide!
							//    same for "bit" param to R_MarkLights

// by joe: the last 2 are own creations, they handle color mapped explosions
typedef enum
{
	lt_default, lt_muzzleflash, lt_explosion, lt_rocket,
	lt_red, lt_blue, lt_redblue, NUM_DLIGHTTYPES,
	lt_explosion2, lt_explosion3
} dlighttype_t;

//extern	float	ExploColor[3];		// joe: for color mapped explosions

#define DLIGHT_INACTIVE(dl) ((cl.time < (dl)->starttime) || (cl.time > (dl)->endtime))

typedef struct
{
	int			key;		// so entities can reuse same entry
	vec3_t		origin;
	float		radius;
	float		starttime;
	float		endtime;	// stop lighting after this time
	float		decay;		// drop this each second
	float		minlight;	// don't add when contributing less
//	int			type;		// color
	float		color[3];
#ifdef HEXEN2_SUPPORT
	qboolean	dark;			// subtracts light instead of adding
#endif
} dlight_t;


//#define	MAX_EFRAGS		640
#define	MAX_EFRAGS		2048

#define	MAX_MAPSTRING	2048
#define	MAX_DEMOS		32
#define	MAX_DEMONAME	64

typedef enum
{
	ca_dedicated, 		// a dedicated server with no ability to start a client
	ca_disconnected, 	// full screen console with no connection
	ca_connected		// valid netcon, talking to a server
} cactive_t;

// the client_static_t structure is persistant through an arbitrary number
// of server connections
typedef struct
{
	cactive_t	state;

// personalization data sent to server
	char		mapstring[MAX_QPATH];
	char		spawnparms[MAX_MAPSTRING];	// to restart a level

// demo loop control
	int			demonum;			// -1 = don't play demos
	char		demos[MAX_DEMOS][MAX_DEMONAME];	// when not playing

// demo recording info must be here, because record is started before
// entering a map (and clearing client_state_t)
	qboolean	demorecording;
	qboolean	demoplayback;
	qboolean	timedemo;
	int			forcetrack;			// -1 = use normal cd track
	FILE		*demofile;
	int			td_lastframe;			// to meter out one message a frame
	int			td_startframe;			// host_framecount at start
	float		td_starttime;			// realtime at second frame of timedemo


// connection information
	int			signon;			// 0 to SIGNONS
	struct qsocket_s	*netcon;
	sizebuf_t	message;		// writing buffer to send to server

	qboolean	capturedemo;
} client_static_t;

extern	client_static_t	cls;

// the client_state_t structure is wiped completely at every
// server signon
typedef struct
{
	int			movemessages;		// since connecting to this server
						// throw out the first couple, so the player
						// doesn't accidentally do something the
						// first frame
	usercmd_t	cmd;			// last command sent to the server

// information for local display
	int			stats[MAX_CL_STATS];	// health, etc
	int			items;			// inventory bit flags
	float		item_gettime[32];	// cl.time of aquiring item, for blinking
	float		faceanim_starttime, faceanim_endtime;		// use anim frame if cl.time is in this range

	cshift_t	cshifts[NUM_CSHIFTS];	// color shifts for damage, powerups, and content types
//	cshift_t	prev_cshifts[NUM_CSHIFTS];

// the client maintains its own idea of view angles, which are
// sent to the server each frame.  The server sets punchangle when
// the view is temporarliy offset, and an angle reset commands at the start
// of each level and after teleporting.
	vec3_t		mviewangles, mviewangles_prev;		// during demo playback viewangles is lerped
						// between these
	vec3_t		viewangles;

	vec3_t		mvelocity, mvelocity_prev;		// update by server, used for lean+bob
						// (0 is newest)
	vec3_t		velocity;		// lerped between mvelocity and mvelocity_prev

	vec3_t		punchangle;		// temporary offset

// pitch drifting vars
	float		idealpitch;
	float		pitchvel;
	qboolean	nodrift;
	float		driftmove;
	double		laststop;

	float		viewheight;
	float		crouch;			// local amount for smoothing stepups

	qboolean	paused;			// send over by server
	qboolean	onground;
	qboolean	inwater;

	int			intermission;		// don't change view angle, full screen, etc
	int			completed_time;		// latched at intermission start

	double		mtime, mtime_prev;		// the timestamp of last two messages
	double		time;			// clients view of time, should be between
						// servertime and oldservertime to generate
						// a lerp point for other data
	double		oldtime;		// previous cl.time, time-oldtime is used
						// to decay light values and smooth step ups
	double		ctime;			// joe: copy of cl.time, to avoid incidents caused by rewind


	double		last_received_message;	// (realtime) for net trouble icon

// information that is static for the entire time connected to a server
	struct model_s	*model_precache[MAX_MODELS];
	struct sfx_s	*sound_precache[MAX_SOUNDS];

	char		levelname[128];		// for display on solo scoreboard  (JDH: increased size from 40)
	int			viewentity;			// cl_entities[cl.viewentity] = player
	int			maxclients;
	int			gametype;

// refresh related state
	struct model_s	*worldmodel;		// cl_entities[0].model
	struct efrag_s	*free_efrags;
	int				num_entities;		// held in cl_entities array
	int				num_statics;		// held in cl_staticentities array
	entity_t		viewent;		// the gun model

	int				cdtrack, looptrack;	// cd audio

#ifdef HEXEN2_SUPPORT
	byte			current_frame, last_frame, reference_frame;
	byte			current_sequence, last_sequence;
	byte			need_build;
	client_frames2_t frames[3]; // 0 = base, 1 = building, 2 = 0 & 1 merged
	short			RemoveList[MAX_CLIENT_STATES], NumToRemove;

	struct effect_t	effects[MAX_EFFECTS];
	int				light_level;
	float			idealroll;
	float			rollvel;
	unsigned		info_mask, info_mask2;
	char			midi_name[128];			// midi file name
	char			puzzle_pieces[8][10];	// puzzle piece names
	entvars_t		v; // NOTE: not every field will be updated - you must specifically add
	                   // them in functions SV_SendUpdateInv() and CL_ParseUpdateInv()
#endif

// frag scoreboard
	scoreboard_t	*scores;		// [cl.maxclients]
	int				protocol;		// JDH
} client_state_t;

extern	client_state_t	cl;

// cvars
extern	cvar_t	cl_name;
extern	cvar_t	cl_color;

extern	cvar_t	cl_upspeed;
extern	cvar_t	cl_forwardspeed;
extern	cvar_t	cl_backspeed;
extern	cvar_t	cl_sidespeed;

extern	cvar_t	cl_movespeedkey;

extern	cvar_t	cl_yawspeed;
extern	cvar_t	cl_pitchspeed;

extern	cvar_t	cl_anglespeedkey;

extern	cvar_t	cl_shownet;
extern	cvar_t	cl_nolerp;

extern	cvar_t	cl_pitchdriftspeed;
extern	cvar_t	lookspring;
extern	cvar_t	lookstrafe;
extern	cvar_t	sensitivity;

extern	cvar_t	m_pitch;
extern	cvar_t	m_yaw;
extern	cvar_t	m_forward;
extern	cvar_t	m_side;

// by joe
extern	cvar_t	cl_truelightning;
extern	cvar_t	cl_sbar;
extern	cvar_t	cl_rocket2grenade;
extern	cvar_t	vid_mode;
extern	cvar_t	cl_demorewind;

extern	cvar_t	r_explosiontype;
extern	cvar_t	r_explosionlight;
extern	cvar_t	r_rocketlight;
//#ifdef GLQUAKE
extern	cvar_t	r_explosionlightcolor;
extern	cvar_t	r_rocketlightcolor;
//#endif
extern	cvar_t	r_rockettrail;
extern	cvar_t	r_grenadetrail;

extern	cvar_t	cl_bobbing;
extern	cvar_t	cl_demospeed;
extern	cvar_t	cl_advancedcompletion;

//extern	cvar_t	cl_maxfps;	// JDH: now host_maxfps

#define	MAX_TEMP_ENTITIES	(MAX_EDICTS/8)		// lightning bolts, etc  (JDH: was 64)
#define	MAX_STATIC_ENTITIES	(MAX_EDICTS/4)		// torches, etc  (JDH: was 128)

// FIXME, allocate dynamically
extern	efrag_t			cl_efrags[MAX_EFRAGS];
extern	entity_t		cl_entities[MAX_EDICTS];
extern	entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
extern	lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
extern	dlight_t		cl_dlights[MAX_DLIGHTS];
//extern	entity_t		cl_temp_entities[MAX_TEMP_ENTITIES];
//extern	beam_t			cl_beams[MAX_BEAMS];

//=============================================================================

// cl_main.c
dlight_t *CL_AllocDlight (int key);
dlight_t * CL_NewDlight (int key, vec3_t origin, float radius, float time, dlighttype_t type);
void CL_DecayLights (void);
void CL_MuzzleFlash (entity_t *ent, int entnum);

void CL_Init (void);

void CL_EstablishConnection (const char *host);
void CL_Signon1 (void);
void CL_Signon2 (void);
void CL_Signon3 (void);
void CL_Signon4 (void);

void CL_Disconnect (qboolean stoprecord);
void CL_Disconnect_f (cmd_source_t src);
int CL_NextDemo (void);

void CL_ResetState (void);
void CL_RelinkPlayer (float frac);

void CL_ClearTEnts (void);
void CL_LinkProjectiles (void);		// JDH: QW demos

//#define	MAX_VISEDICTS	256
#define	MAX_VISEDICTS	(MAX_EDICTS/2)
extern	int			cl_numvisedicts;
extern	entity_t	*cl_visedicts[MAX_VISEDICTS];

// model indexes
typedef	enum modelindex_s
{
	mi_player, mi_eyes, mi_rocket, mi_grenade, mi_spike, mi_flame0, mi_flame1, mi_flame2,
	mi_explo1, mi_explo2, mi_bubble, mi_sng,
	mi_fish, mi_dog, mi_soldier, mi_enforcer, mi_knight, mi_hknight,
	mi_scrag, mi_ogre, mi_fiend, mi_vore, mi_shambler,
	/*mi_h_dog, mi_h_soldier, mi_h_enforcer, mi_h_knight, mi_h_hknight, mi_h_scrag,
	mi_h_ogre, mi_h_fiend, mi_h_vore, mi_h_shambler, mi_h_zombie,*/ mi_h_player,
	mi_gib1, mi_gib2, mi_gib3, NUM_MODELINDEX
} modelindex_t;

extern	modelindex_t	cl_modelindex[NUM_MODELINDEX];
extern	char			*cl_modelnames[NUM_MODELINDEX];

// cl_input.c
typedef struct
{
	int		down[2];		// key nums holding it down
	int		state;			// low bit is down state
} kbutton_t;

extern	kbutton_t	in_mlook, in_klook;
extern 	kbutton_t 	in_strafe;
extern 	kbutton_t 	in_speed;

void CL_InitInput (void);
void CL_SendCmd (void);
void CL_SendMove (const usercmd_t *cmd);
void CL_SendLagMove (void);	// joe: synthetic lag, from ProQuake

void CL_ClearState (void);

int  CL_ReadFromServer (void);
void CL_BaseMove (usercmd_t *cmd);

float CL_KeyState (kbutton_t *key);
char *Key_KeynumToString (int keynum);

#ifdef HEXEN2_SUPPORT
void IN_CrouchDown (cmd_source_t src);
void IN_CrouchUp (cmd_source_t src);
void IN_infoPlaqueDown (cmd_source_t src);
void IN_infoPlaqueUp (cmd_source_t src);
#endif

// cl_demo.c
extern qboolean	cl_demoseek;

void CL_StartPlayback (const char *filename);
void CL_StopPlayback (void);
int  CL_GetMessage (void);
void CL_DemoSeek (float);
void CL_WriteDemoMessage (FILE *demofile, const vec3_t viewangles);
void CL_StopRecord (qboolean cl_disconnecting);
void CL_Stop_f (cmd_source_t src);
void CL_Record_f (cmd_source_t src);
void CL_PlayDemo_f (cmd_source_t src);
void CL_TimeDemo_f (cmd_source_t src);
qboolean CL_CheckExistingFile (const char *filepath);
void CL_MakeRecordingName (char *buf, int bufsize);


// cl_parse.c
void CL_ParseServerMessage (void);
void CL_NewTranslation (int slot);
entity_t *CL_EntityNum (int num);

// for QuakeWorld demos:
void CL_ParsePlayerinfo (void);
void CL_UpdateUserinfo (void);
qboolean CL_ParsePacketEntities (qboolean delta);
void CL_SetInfo (void);
void CL_ClearProjectiles (void);
void CL_ParseProjectiles (void);

#ifdef HEXEN2_SUPPORT
qboolean CL_ParseUpdate_H2 (entity_t *ent, int num, int bits);
void CL_ParseMIDI (void);
void CL_ParseUpdateClass (void);
void CL_ParseRainEffect(void);
void CL_Plaque (void);
void CL_ParticleExplosion (void);
qboolean CL_ParseReference (void);
qboolean CL_ClearEdicts (void);
void CL_ParseUpdateInv (void);
void CL_ParseAngleInterpolate (void);
void CL_ParseSoundPos (void);
void R_ParseParticleEffect2 (void);
void R_ParseParticleEffect3 (void);
void R_ParseParticleEffect4 (void);
void CL_ParseEffect (void);
void CL_EndEffect (void);
#endif


// cl_tent.c
void CL_InitTEnts (void);
void CL_InitStrings (void);
qboolean CL_ParseTEnt (qboolean parse_only);
void CL_UpdateTEnts (void);
void CL_SignonReply (void);

//dlighttype_t SetDlightColor (float f, dlighttype_t def, qboolean random);
void SetDlightColor (dlight_t *dl, int f, dlighttype_t def, qboolean random);

void R_TranslatePlayerSkin (int playernum);
