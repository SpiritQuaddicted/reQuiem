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
// protocol.h -- communications protocols

#define	PROTOCOL_VERSION_STD	  15
#define PROTOCOL_VERSION_BETA     12
#define PROTOCOL_VERSION_QW		  28
#define	PROTOCOL_VERSION_FITZ    666

#define	PROTOCOL_VERSION_BJP	10000	// Extended protocol (models > 256 etc), hopefully no conflict
#define	PROTOCOL_VERSION_BJP2	10001	// Extended protocol (sounds > 256), problems with Marcher
#define	PROTOCOL_VERSION_BJP3	10002	// Extended protocol (sounds > 256), more compatible, but less functional

// in progress...
#define PROTOCOL_VERSION_DP7	3504
#define PROTOCOL_VERSION_FTE	(('F'<<0) + ('T'<<8) + ('E'<<16) + ('X' << 24))	//fte extensions.

#ifdef HEXEN2_SUPPORT
  #define	PROTOCOL_VERSION_H2_111		18
  #define	PROTOCOL_VERSION_H2_112		19
#endif

// if the high bit of the servercmd is set, the low bits are fast update flags:
#define	U_MOREBITS	0x0001	//(1<<0)
#define	U_ORIGIN1	0x0002	//(1<<1)
#define	U_ORIGIN2	0x0004	//(1<<2)
#define	U_ORIGIN3	0x0008	//(1<<3)
#define	U_ANGLE2	0x0010	//(1<<4)
#define	U_NOLERP	0x0020	//(1<<5)		// don't interpolate movement
#define	U_FRAME		0x0040	//(1<<6)
#define U_SIGNAL	0x0080	//(1<<7)		// just differentiates from other updates

// svc_update can pass all of the fast update bits, plus more
#define	U_ANGLE1	0x0100	//(1<<8)
#define	U_ANGLE3	0x0200	//(1<<9)
#define	U_MODEL		0x0400	//(1<<10)
#define	U_COLORMAP	0x0800	//(1<<11)
#define	U_SKIN		0x1000	//(1<<12)
#define	U_EFFECTS	0x2000	//(1<<13)
#define	U_LONGENTITY	0x4000	//(1<<14)
// nehahra support
#define	U_TRANS		0x8000	//(1<<15)
// Fitz protocol:
#define U_EXTEND1		(1<<15)
#define U_ALPHA			(1<<16) // 1 byte, uses ENTALPHA_ENCODE, not sent if equal to baseline
#define U_FRAME2		(1<<17) // 1 byte, this is .frame & 0xFF00 (second byte)
#define U_MODEL2		(1<<18) // 1 byte, this is .modelindex & 0xFF00 (second byte)
#define U_LERPFINISH	(1<<19) // 1 byte, 0.0-1.0 maps to 0-255, not sent if exactly 0.1, this is ent->v.nextthink - sv.time, used for lerping
#define U_EXTEND2		(1<<23) // another byte to follow, future expansion

#ifdef HEXEN2_SUPPORT
  #define	U_CLEAR_ENT		0x000800	//(1<<11)
  //#define	U_ENT_OFF       0x002000	//(1<<13) - JDH: not used
  #define	U_MOREBITS2     0x008000	//(1<<15)
  #define	U_SKIN_H2		0x010000	//(1<<16)
  #define	U_EFFECTS_H2	0x020000	//(1<<17)
  #define	U_SCALE			0x040000	//(1<<18)
  #define	U_COLORMAP_H2	0x080000	//(1<<19)
#endif

#define	SU_VIEWHEIGHT	0x0001	//(1<<0)
#define	SU_IDEALPITCH	0x0002	//(1<<1)
#define	SU_PUNCH1		0x0004	//(1<<2)
#define	SU_PUNCH2		0x0008	//(1<<3)
#define	SU_PUNCH3		0x0010	//(1<<4)
#define	SU_VELOCITY1	0x0020	//(1<<5)
#define	SU_VELOCITY2	0x0040	//(1<<6)
#define	SU_VELOCITY3	0x0080	//(1<<7)
#ifdef HEXEN2_SUPPORT
  #define SU_IDEALROLL	0x0100	//(1<<8)
#endif
#define	SU_ITEMS		0x0200	//(1<<9)
#define	SU_ONGROUND		0x0400	//(1<<10)		// no data follows, the bit is it
#define	SU_INWATER		0x0800	//(1<<11)		// no data follows, the bit is it
#define	SU_WEAPONFRAME	0x1000	//(1<<12)
#define	SU_ARMOR		0x2000	//(1<<13)
#define	SU_WEAPON		0x4000	//(1<<14)

// Fitz protocol:
#define SU_EXTEND1		(1<<15) // another byte to follow
#define SU_WEAPON2		(1<<16) // 1 byte, this is .weaponmodel & 0xFF00 (second byte)
#define SU_ARMOR2		(1<<17) // 1 byte, this is .armorvalue & 0xFF00 (second byte)
#define SU_AMMO2		(1<<18) // 1 byte, this is .currentammo & 0xFF00 (second byte)
#define SU_SHELLS2		(1<<19) // 1 byte, this is .ammo_shells & 0xFF00 (second byte)
#define SU_NAILS2		(1<<20) // 1 byte, this is .ammo_nails & 0xFF00 (second byte)
#define SU_ROCKETS2		(1<<21) // 1 byte, this is .ammo_rockets & 0xFF00 (second byte)
#define SU_CELLS2		(1<<22) // 1 byte, this is .ammo_cells & 0xFF00 (second byte)
#define SU_EXTEND2		(1<<23) // another byte to follow
#define SU_WEAPONFRAME2	(1<<24) // 1 byte, this is .weaponframe & 0xFF00 (second byte)
#define SU_WEAPONALPHA	(1<<25) // 1 byte, this is alpha for weaponmodel, uses ENTALPHA_ENCODE, not sent if ENTALPHA_DEFAULT

// a sound with no channel is a local only sound
#define	SND_VOLUME		0x0001	//(1<<0)		// a byte
#define	SND_ATTENUATION	0x0002	//(1<<1)		// a byte
//#define	SND_LOOPING		0x0004	//(1<<2)		// a long
#ifdef HEXEN2_SUPPORT
  #define	SND_OVERFLOW	0x0004			// add 255 to snd num
#endif
//johnfitz -- PROTOCOL_FITZQUAKE -- new bits
#define	SND_LARGEENTITY	0x0008	// a short + byte (instead of just a short)
#define	SND_LARGESOUND	0x0010	// a short soundindex (instead of a byte)
//johnfitz

#define DEFAULT_SOUND_PACKET_VOLUME 255
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

// defaults for clientinfo messages
#define	DEFAULT_VIEWHEIGHT	22


// game types sent by serverinfo
// these determine which intermission screen plays
#define	GAME_COOP		0
#define	GAME_DEATHMATCH		1

//==================
// note that there are some defs.qc that mirror to these numbers
// also related to svc_strings[] in cl_parse
//==================

//
// server to client
//
#define	svc_bad					0
#define	svc_nop					1
#define	svc_disconnect			2
#define	svc_updatestat			3	// [byte] [long]
#define	svc_version				4	// [long] server version
#define	svc_setview				5	// [short] entity number
#define	svc_sound				6	// <see code>
#define	svc_time				7	// [float] server time
#define	svc_print				8	// [string] null terminated string
#define	svc_stufftext			9	// [string] stuffed into client's console buffer
									// the string should be \n terminated
#define	svc_setangle			10	// [angle3] set the view angle to this absolute value
	
#define	svc_serverinfo			11	// [long] version
									// [byte] max # clients
									// [byte] is deathmatch?
									// [string] signon string
									// [string]..[0]model cache
									// [string]...[0]sounds cache
#define	svc_lightstyle			12	// [byte] [string]
#define	svc_updatename			13	// [byte] [string]
#define	svc_updatefrags			14	// [byte] [short]
#define	svc_clientdata			15	// <shortbits + data>
#define	svc_stopsound			16	// [short]
#define	svc_updatecolors		17	// [byte] [byte]
#define	svc_particle			18	// [coord3] [angle3] [byte] [byte]
#define	svc_damage				19	// [byte] [byte] [coord3]
	
#define	svc_spawnstatic			20	// [byte/short] [byte] [byte] [byte] [coord] [angle] [coord] [angle] [coord] [angle]
//	svc_spawnbinary				21
#define	svc_spawnbaseline		22	// [short] [<svc_spawnstatic as above>]
	
#define	svc_temp_entity			23	// <see code>

#define	svc_setpause			24	// [byte] on / off
#define	svc_signonnum			25	// [byte]  used for the signon sequence

#define	svc_centerprint			26	// [string] to put in center of the screen

#define	svc_killedmonster		27	// <no additional data>
#define	svc_foundsecret			28	// <no additional data>

#define	svc_spawnstaticsound	29	// [coord3] [byte/short] samp [byte] vol [byte] aten

#define	svc_intermission		30	// <no data>
#define	svc_finale				31	// [string] text

#define	svc_cdtrack				32	// [byte] track [byte] looptrack
#define svc_sellscreen			33	// <no data>

#define svc_cutscene			34	// [string]

// nehahra support
#define	svc_showlmp				35	// [string] slotname [string] lmpfilename [angle] x [angle] y
#define	svc_hidelmp				36	// [string] slotname
#define	svc_skybox				37	// [string] skyname

// JDH: FitzQuake protocol support:
#define svc_bf					40
#define svc_fog_fitz			41	// [byte] dens [byte] r [byte] g [byte] b [short] time
#define svc_spawnbaseline2		42  // support for large modelindex, large framenum, alpha, using flags
#define svc_spawnstatic2		43	// support for large modelindex, large framenum, alpha, using flags
#define	svc_spawnstaticsound2	44	// [coord3] [short] samp [byte] vol [byte] aten

// nehahra support
#define svc_skyboxsize			50  // [coord] size (default is 4096)
#define svc_fog_neh				51	// [byte] enable <optional past this point, only included if enable is true> [float] density [byte] red [byte] green [byte] blue

// JDH: Quakeworld (demo) support:

#define	svc_smallkick			34		// set client punchangle to 2
#define	svc_bigkick				35		// set client punchangle to 4

#define	svc_updateping			36		// [byte] [short]
#define	svc_updateentertime		37		// [byte] [float]

#define	svc_updatestatlong		38		// [byte] [long]

#define	svc_muzzleflash			39		// [short] entity

#define	svc_updateuserinfo		40		// [byte] slot [long] uid
										// [string] userinfo

#define	svc_download			41		// [short] size [size bytes]
#define	svc_playerinfo			42		// variable
#define	svc_nails				43		// [byte] num [48 bits] xyzpy 12 12 12 4 8 
#define	svc_chokecount			44		// [byte] packets choked
#define	svc_modellist			45		// [strings]
#define	svc_soundlist			46		// [strings]
#define	svc_packetentities		47		// [...]
#define	svc_deltapacketentities	48		// [...]
#define svc_maxspeed			49		// maxspeed change, for prediction
#define svc_entgravity			50		// gravity change, for prediction
#define svc_setinfo				51		// setinfo on a client
#define svc_serverinfo_qw		52		// serverinfo
#define svc_updatepl			53		// [byte] [byte]


#ifdef HEXEN2_SUPPORT

#define	svc_raineffect				21

#define svc_particle2				34		// [vec3] <variable>
#define svc_cutscene_H2				35
#define svc_midi_name				36		// [string] name
#define svc_updateclass				37		// [byte] [byte]
#define	svc_particle3				38	
#define	svc_particle4				39	
#define svc_set_view_flags			40
#define svc_clear_view_flags		41
#define svc_start_effect			42
#define svc_end_effect				43
#define svc_plaque					44
#define svc_particle_explosion		45
#define svc_set_view_tint			46
#define svc_reference				47
#define svc_clear_edicts			48
#define svc_update_inv				49

#define	svc_setangle_interpolate	50
#define svc_update_kingofhill		51
#define svc_toggle_statbar			52
#define svc_sound_update_pos		53	//[short] ent+channel [coord3] pos

// Bits to help send server info about the client's edict variables
#define SC1_HEALTH				(1<<0)		// changes stat bar
#define SC1_LEVEL				(1<<1)		// changes stat bar
#define SC1_INTELLIGENCE		(1<<2)		// changes stat bar
#define SC1_WISDOM				(1<<3)		// changes stat bar
#define SC1_STRENGTH			(1<<4)		// changes stat bar
#define SC1_DEXTERITY			(1<<5)		// changes stat bar
#define SC1_WEAPON				(1<<6)		// changes stat bar
#define SC1_BLUEMANA			(1<<7)		// changes stat bar
#define SC1_GREENMANA			(1<<8)		// changes stat bar
#define SC1_EXPERIENCE			(1<<9)		// changes stat bar
#define SC1_CNT_TORCH			(1<<10)		// changes stat bar
#define SC1_CNT_H_BOOST			(1<<11)		// changes stat bar
#define SC1_CNT_SH_BOOST		(1<<12)		// changes stat bar
#define SC1_CNT_MANA_BOOST		(1<<13)		// changes stat bar
#define SC1_CNT_TELEPORT		(1<<14)		// changes stat bar
#define SC1_CNT_TOME			(1<<15)		// changes stat bar
#define SC1_CNT_SUMMON			(1<<16)		// changes stat bar
#define SC1_CNT_INVISIBILITY	(1<<17)		// changes stat bar
#define SC1_CNT_GLYPH			(1<<18)		// changes stat bar
#define SC1_CNT_HASTE			(1<<19)		// changes stat bar
#define SC1_CNT_BLAST			(1<<20)		// changes stat bar
#define SC1_CNT_POLYMORPH		(1<<21)		// changes stat bar
#define SC1_CNT_FLIGHT			(1<<22)		// changes stat bar
#define SC1_CNT_CUBEOFFORCE		(1<<23)		// changes stat bar
#define SC1_CNT_INVINCIBILITY	(1<<24)		// changes stat bar
#define SC1_ARTIFACT_ACTIVE		(1<<25)
#define SC1_ARTIFACT_LOW		(1<<26)
#define SC1_MOVETYPE			(1<<27)
#define SC1_CAMERAMODE			(1<<28)
#define SC1_HASTED				(1<<29)
#define SC1_INVENTORY			(1<<30)
#define SC1_RINGS_ACTIVE		(1<<31)

#define SC2_RINGS_LOW			(1<<0)
#define SC2_AMULET				(1<<1)
#define SC2_BRACER				(1<<2)
#define SC2_BREASTPLATE			(1<<3)
#define SC2_HELMET				(1<<4)
#define SC2_FLIGHT_T			(1<<5)
#define SC2_WATER_T				(1<<6)
#define SC2_TURNING_T			(1<<7)
#define SC2_REGEN_T				(1<<8)
#define SC2_HASTE_T				(1<<9)
#define SC2_TOME_T				(1<<10)
#define SC2_PUZZLE1				(1<<11)
#define SC2_PUZZLE2				(1<<12)
#define SC2_PUZZLE3				(1<<13)
#define SC2_PUZZLE4				(1<<14)
#define SC2_PUZZLE5				(1<<15)
#define SC2_PUZZLE6				(1<<16)
#define SC2_PUZZLE7				(1<<17)
#define SC2_PUZZLE8				(1<<18)
#define SC2_MAXHEALTH			(1<<19)
#define SC2_MAXMANA				(1<<20)
#define SC2_FLAGS				(1<<21)
#define SC2_OBJ					(1<<22)
#define SC2_OBJ2				(1<<23)


// This is to mask out those items that need to generate a stat bar change
#define SC1_STAT_BAR   0x01ffffff
#define SC2_STAT_BAR   0x0

// This is to mask out those items in the inventory (for inventory changes)
#define SC1_INV 0x01fffc00
#define SC2_INV 0x00000000

#define TE_STREAM_LIGHTNING_SMALL	24
#define TE_STREAM_CHAIN				25
#define TE_STREAM_SUNSTAFF1			26
#define TE_STREAM_SUNSTAFF2			27
#define TE_STREAM_LIGHTNING			28
#define TE_STREAM_COLORBEAM			29
#define TE_STREAM_ICECHUNKS			30
#define TE_STREAM_GAZE				31
#define TE_STREAM_FAMINE			32

#endif		// #ifdef HEXEN2_SUPPORT


// client to server
#define	clc_bad				0
#define	clc_nop 			1
#define	clc_disconnect		2
#define	clc_move			3	// [usercmd_t]
#define	clc_stringcmd		4	// [string] message
#ifdef HEXEN2_SUPPORT
  #define clc_inv_select	5
  #define clc_frame			6
#endif

// temp entity events
#define	TE_SPIKE			0
#define	TE_SUPERSPIKE		1
#define	TE_GUNSHOT			2
#define	TE_EXPLOSION		3
#define	TE_TAREXPLOSION		4
#define	TE_LIGHTNING1		5
#define	TE_LIGHTNING2		6
#define	TE_WIZSPIKE			7
#define	TE_KNIGHTSPIKE		8
#define	TE_LIGHTNING3		9
#define	TE_LAVASPLASH		10
#define	TE_TELEPORT			11
#define TE_EXPLOSION2		12

// PGM 01/21/97 
#define TE_BEAM				13
// PGM 01/21/97 

// nehahra support
#define	TE_EXPLOSION3		16
#define	TE_LIGHTNING4NEH	17
#define TE_SMOKE            18
#define TE_NEW1             19
#define TE_NEW2             20

// JT042105 - adding DP extensions.  These were taken from DP20050420Beta1, and compared to
// DP105.  In order to get basic functionality quickest, DP105 functions will be implemented.
// FIXME: each is disabled until functional

// LordHavoc: added some TE_ codes (block1 - 50-60)
#define	TE_BLOOD			50 // [vector] origin [byte] xvel [byte] yvel [byte] zvel [byte] count
//#define	TE_SPARK			51 // [vector] origin [byte] xvel [byte] yvel [byte] zvel [byte] count
//#define	TE_BLOODSHOWER		52 // [vector] min [vector] max [coord] explosionspeed [short] count
//#define	TE_EXPLOSIONRGB		53 // [vector] origin [byte] red [byte] green [byte] blue
//#define TE_PARTICLECUBE		54 // [vector] min [vector] max [vector] dir [short] count [byte] color [byte] gravity [coord] randomvel
//#define TE_PARTICLERAIN		55 // [vector] min [vector] max [vector] dir [short] count [byte] color
//#define TE_PARTICLESNOW		56 // [vector] min [vector] max [vector] dir [short] count [byte] color
//#define TE_GUNSHOTQUAD		57 // [vector] origin
//#define TE_SPIKEQUAD		58 // [vector] origin
//#define TE_SUPERSPIKEQUAD	59 // [vector] origin
#define TE_CUSTOMFLASH		73 // [vector] origin [byte] radius / 8 - 1 [byte] lifetime / 256 - 1 [byte] red [byte] green [byte] blue
