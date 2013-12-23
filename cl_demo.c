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
// cl_demo.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#define OLDSEEK

#include "winquake.h"
#include "dzip.h"
#include "movie.h"
#include <time.h>	// easyrecord stats

#ifndef _WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif

// JDH: I took Joe's idea of a framepos stack, and reduced the overhead of
//      doing a malloc/free for each framepos.  I did this by allocating
//      large blocks, then chaining them together as a linked list.

// 8000 frames is good for ~2-10 minutes of recorded demo (depending on demo's framerate)
#define FRAMEPOS_BLOCKSIZE 8000
typedef struct framepos_entry_s
{
	long		inpos/*, outpos*/;
} framepos_entry_t;

// added by joe
typedef struct framepos_s
{
	struct framepos_s *next;
	framepos_entry_t	entries[FRAMEPOS_BLOCKSIZE];
} framepos_t;

framepos_t	*dem_framepos = NULL;
int			dem_framecount = 0;

long		cl_demo_filestart, cl_demo_filesize;
double		cl_demo_starttime, cl_demo_endtime;		// JDH: during playback, cl.time at beginning and end
qboolean	cl_demoseek;			// JDH: true if jumping to new position in demo

cvar_t		cl_demorewind = {"cl_demorewind", "0"};

// .dz playback

#define DZ_APP_NOERR   0
#define DZ_APP_ERROR   1

typedef enum {DZ_OP_NONE, DZ_OP_EXTRACT, DZ_OP_COMPRESS, DZ_OP_VERIFY} dz_op_t;

#ifdef _WIN32
#define DZIP_APPNAME	"dzip.exe"
static	HANDLE	hDZipProcess = NULL;
static	HANDLE  hDZipThread = NULL;		// just so we can CloseHandle when done
#else
#define DZIP_APPNAME	"dzip-linux"
static	qboolean hDZipProcess = false;
#endif

static qboolean	dz_playback = false;
static dz_op_t	dz_app_op = DZ_OP_NONE;		// what operation dzip app is currently doing
static int      dz_exitcode = DZ_APP_ERROR;
static char		dz_filename[MAX_OSPATH];	// used when compressing; doesn't include path


static	char	cl_demo_filepath[MAX_OSPATH] = "";	// full path of demo currently being played/recorded
//static	char	dem_basedir[256] = "";

static	void CheckDZipCompletion ();
static	void StopDZPlayback ();

#define DZ_APPEXISTS() (COM_FileExists(va("%s/"DZIP_APPNAME, com_basedir)))

// joe: support for recording demos after connecting to the server
byte	demo_head[3][NET_MAXMESSAGE];
int		demo_head_size[2];

#ifdef HEXEN2_SUPPORT
  extern qboolean intro_playing;
#endif

extern qboolean dzlib_loaded;

void CL_FinishTimeDemo (void);

// JDH: added overlay, QWD support, quickjump navigation, demo compression
extern double		scr_demo_overlay_time;
extern cvar_t		cl_demo_compress, cl_demo_compress_fmt;
extern qboolean		dz_canzip;

qboolean is_qwd = false;

/*
// JDH: struct used to record the state of several client vars
//      once signon is complete
typedef struct demo_state_s
{
	int	stats[MAX_CL_STATS];
	int viewentity;
	int viewheight;		// for QWD
#ifdef HEXEN2_SUPPORT
	entvars_t entvars;
	long info_mask;
	long info_mask2;
#endif
} demo_state_t;

demo_state_t demo_state_orig;
*/
/*
==============================================================================
DEMO CODE

When a demo is playing back, all NET_SendMessages are skipped, and
NET_GetMessages are read from the demo file.

Whenever cl.time gets past the last received message, another message is
read from the demo file.
==============================================================================
*/

void PushFrameposEntry (long inpos)
{
	framepos_t	*newf;

#ifdef _DEBUG
	if (inpos == 0x04a112a8)
		inpos *= 1;
#endif

	if (!dem_framepos)
	{
		dem_framepos = Q_malloc (sizeof(framepos_t));
		dem_framepos->next = NULL;
		dem_framecount = 0;
	}
	else if (dem_framecount >= FRAMEPOS_BLOCKSIZE)
	{
		newf = Q_malloc (sizeof(framepos_t));
		newf->next = dem_framepos;
		dem_framepos = newf;
		dem_framecount = 0;
	}

	dem_framepos->entries[dem_framecount].inpos = inpos;
	dem_framecount++;
}

void EraseTopEntry (void)
{
	framepos_t *next;

	if (dem_framepos)
	{
		if (dem_framecount > 1)
			dem_framecount--;
		else
		{
			next = dem_framepos->next;
			free (dem_framepos);
			dem_framepos = next;
			dem_framecount = (dem_framepos ? FRAMEPOS_BLOCKSIZE : 0);
		}
	}
}

long ClearFrameposStack (void)
{
	framepos_t *next;
	long	startpos = 0;

	while (dem_framepos)
	{
		startpos = dem_framepos->entries[0].inpos;
		next = dem_framepos->next;
		free (dem_framepos);
		dem_framepos = next;
	}

	dem_framecount = 0;
	return startpos;
}

const char * CL_GetDemoFilepath (void)
{
	return cl_demo_filepath;
}

/*
==============
CL_StopPlayback

Called when a demo file runs out, or the user starts a game
==============
*/
void CL_StopPlayback (void)
{
	if (!cls.demoplayback)
		return;

#ifdef HEXEN2_SUPPORT
	if (hexen2 && intro_playing)
	{
		M_ToggleMenu_f (SRC_COMMAND);
		intro_playing = false;
	}
#endif

	fclose (cls.demofile);
	cls.demofile = NULL;
	cls.demoplayback = false;
	cl_demo_filepath[0] = 0;
	cls.state = ca_disconnected;

	ClearFrameposStack ();

	if (dz_playback)
		StopDZPlayback ();

	if (cls.timedemo)
		CL_FinishTimeDemo ();

	Movie_StopDemoCapture ();
}

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
void CL_WriteDemoMessage (FILE *demofile, const vec3_t viewangles)
{
	int	i, len;
	float	f;

	len = LittleLong (net_message.cursize);
	fwrite (&len, 4, 1, demofile);
	for (i=0 ; i<3 ; i++)
	{
		f = LittleFloat (viewangles[i]);
		fwrite (&f, 4, 1, demofile);
	}
	fwrite (net_message.data, net_message.cursize, 1, demofile);
	fflush (demofile);
}

/*
=================
CL_DemoSaveState
  used during demo playback to record the initial state of some variables
=================
*/
void CL_DemoSaveState (void)
{
/*	int i;

	for (i = 0; i < MAX_CL_STATS; i++)
		demo_state_orig.stats[i] = cl.stats[i];

	demo_state_orig.viewentity = cl.viewentity;
	demo_state_orig.viewheight = cl.viewheight;
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		demo_state_orig.entvars = cl.v;
		demo_state_orig.info_mask = cl.info_mask;
		demo_state_orig.info_mask2 = cl.info_mask2;
	}
#endif
*/
}

// QW demo message types:
#define dem_cmd			0
#define dem_read		1
#define dem_set			2
/*
#define dem_multiple	3 // MVD ONLY. This message is directed to several clients.
#define	dem_single		4 // MVD ONLY. This message is directed to a single client.
#define dem_stats		5 // MVD ONLY. Stats update for a player.
#define dem_all			6 // MVD ONLY. This message is directed to all clients.
*/

/*
====================
CL_GetDemoMessage

  JDH: split off from CL_GetMessage
====================
*/
int CL_GetDemoMessage (void)
{
	int			r, i;
	float		f;
	double		demotime;
	byte		c;
	qboolean	connectionless = false;

//		if (start_of_demo && cl_demorewind.value)
	if (!dem_framepos && cl_demorewind.value)
		return 0;

	// if there are pending commands, run them now
	//  (this is needed so Hexen II demos can switch maps properly)
	if (!Cbuf_IsEmpty())		// --> 2011/05/18: moved from below, execute directly
	{
		Cbuf_Execute ();
		if (!cls.demoplayback)		// in case command in cbuf caused demo to stop
			return 0;
	}

	if ((cls.signon < SIGNONS) && dem_framepos)		// clear stack if new demo (or new run within same demo)
		ClearFrameposStack ();

#if 0		// --> changed 2011/05/18 (fixes speed issues in Rubicon2 demos)
	if (!Cbuf_IsEmpty())	// if there are pending commands, return 0 so they can be run
		return 0;			//  (this is needed so Hexen II demos can switch maps properly)
#endif

	/*if (is_qwd && !cl_demorewind.value)		--> removed 2010/01/20
	{
		// read the time from the packet
		fread(&f, sizeof(f), 1, cls.demofile);
		demotime = LittleFloat(f);
		fseek (cls.demofile, ftell(cls.demofile)-4, SEEK_SET);		// 'undo' read
	}
	else*/ demotime = cl.mtime;

	// decide if it is time to grab the next message
	if (cls.signon == SIGNONS)	// only after fully connected
	{
#ifndef OLDSEEK
		if (!cl_demoseek)
#endif
		{
		if (cls.timedemo)
		{
			if (host_framecount == cls.td_lastframe)
				return 0;		// already read this frame's message
			cls.td_lastframe = host_framecount;
			// if this is the second frame, grab the real td_starttime
			// so the bogus time on the first frame doesn't count
			if (host_framecount == cls.td_startframe + 1)
				cls.td_starttime = realtime;
		}
		// modified by joe to handle rewind playing
		else if (!cl_demorewind.value && (cl.ctime <= demotime))
		{
#ifdef _DEBUG
//			Con_Printf("delaying before next message (ctime = %.3lf, demotime = %.3lf)\n", cl.ctime, demotime);
#endif
			return 0;		// don't need another message yet
		}
		else if (cl_demorewind.value && (cl.ctime >= demotime))
		{
#ifdef _DEBUG
//			Con_Printf("delaying before next message (ctime = %.3lf, demotime = %.3lf)\n", cl.ctime, demotime);
#endif
			return 0;
		}
		}

#ifdef _DEBUG
//		Con_Printf("grabbing next message (ctime = %.3lf, demotime = %.3lf). realtime = %.3lf\n", 
//					cl.ctime, demotime, realtime);
#endif
	
		// JDH: show OSD at start of demo, for 1sec longer than usual
		//  (have to do it here because realtime is halted until
		//   the 2nd frame)
		if (scr_demo_overlay_time == 0)
			scr_demo_overlay_time = realtime+1;

		// joe: fill in the stack of frames' positions
		// enable on intermission or not...?
		// NOTE: it can't handle fixed intermission views!
		if (!cl_demorewind.value /*&& !cl.intermission*/)
		{
			if (!cl_demo_starttime)
			{
				cl_demo_starttime = cl.mtime;		// store time of first message once connected
				CL_DemoSaveState ();
			}

			PushFrameposEntry (ftell(cls.demofile));
		}
	}

	if (is_qwd)
	{
		while (1)
		{
			fread (&demotime, 4, 1, cls.demofile);		// packet time
			demotime = LittleFloat(demotime);

			//cl.mtime_prev = cl.mtime;		--> moved below 2010/01/20
			//cl.mtime = demotime;

			fread (&c, 1, 1, cls.demofile);		// packet type

#ifdef _DEBUG
//				Con_DPrintf ("Packet type %d; time = %f\n", c, demotime);
#endif

			if (c == dem_set)
			{
				// JDH: just skip next 8 bytes & read next packet (FIXME?)
				fread (&r, 4, 1, cls.demofile);		// outgoing_sequence
				fread (&r, 4, 1, cls.demofile);		// incoming_sequence
			}
			else if (c == dem_cmd)
			{
				fseek (cls.demofile, 24, SEEK_CUR);		// sizeof(qw's usercmd_t) - is this info needed??
				VectorCopy (cl.mviewangles, cl.mviewangles_prev);
				for (i=0 ; i<3 ; i++)
				{
					fread (&f, 4, 1, cls.demofile);
					cl.mviewangles[i] = LittleFloat (f);
				}
			/*
				i = ftell (cls.demofile);
				fseek (cls.demofile, 4, SEEK_CUR);
				VectorCopy (cl.mviewangles, cl.mviewangles_prev);
				for (i=0 ; i<3 ; i++)
				{
					fread (&f, 4, 1, cls.demofile);
					cl.mviewangles[i] = LittleFloat (f);
				}
				fseek (cls.demofile, 20, SEEK_CUR);		// remainder of usercmd_t + viewangles
			*/
			}
			else if (c == dem_read)
			{
				if (demotime != cl.mtime)
				{
				// if it's not a single update spread over multiple packets
					cl.mtime_prev = cl.mtime;
					cl.mtime = demotime;
				}
				break;
			}
			else
			{
			//	Host_Error ("QuakeWorld Demo ERROR");
				Con_Printf ("ERROR: invalid message type %d\n", c);
				CL_StopPlayback ();
				return 0;
			}
		}
	}

	// get the next message's size
	fread (&net_message.cursize, 4, 1, cls.demofile);
	net_message.cursize = LittleLong (net_message.cursize);

#ifdef _DEBUG
	if (net_message.cursize == 1)
		i = 0;
#endif
	
	if (net_message.cursize > MAX_MSGLEN)
	{
	// JDH: try ignoring top 2 bytes
		if ((net_message.cursize & 0xFFFF) > MAX_MSGLEN)
			Host_Error ("Demo message > MAX_MSGLEN");		// JDH: was Sys_Error

		Con_DPrintf ("\x02""Warning: invalid message length %u\n", net_message.cursize);
		net_message.cursize &= 0xFFFF;
	}

	if (is_qwd)
	{
		fread (&r, 4, 1, cls.demofile);		// sequence
		net_message.cursize -= 4;
		if (r == -1)
		{
			// -1 signals a "connectionless packet" (no ACK)
			fread (&c, 1, 1, cls.demofile);		// message id
			fseek (cls.demofile, -1, SEEK_CUR);		// 'undo' read
			if (c != svc_disconnect)		// disconnect gets handled normally (in CL_ParseServerMessage)
			{
			//	Host_EndGame ("======== End of demo ========");
				connectionless = true;
			}
		}
		else
		{
			fread (&r, 4, 1, cls.demofile);		// sequence ACK
			net_message.cursize -= 4;
		}
	}
	else
	{
		VectorCopy (cl.mviewangles, cl.mviewangles_prev);
		for (i=0 ; i<3 ; i++)
		{
			r = fread (&f, 4, 1, cls.demofile);
			cl.mviewangles[i] = LittleFloat (f);
		}
	}

	r = fread (net_message.data, net_message.cursize, 1, cls.demofile);
	if (r != 1)
	{
		CL_StopPlayback ();
		return 0;
	}

#ifdef _DEBUG
	if (hexen2 && !Q_strcasecmp(COM_SkipPath(cl_demo_filepath), "walk1.dem"))
		if (ftell(cls.demofile) == 189819)		// message with "reconnect" cmd
			r = 1;
#endif
		
	// joe: get out framestack's top entry
	if (cl_demorewind.value && dem_framepos && dem_framecount/*&& !cl.intermission*/)
	{
		//fseek (cls.demofile, dem_framepos->inpos, SEEK_SET);
		fseek (cls.demofile, dem_framepos->entries[dem_framecount-1].inpos, SEEK_SET);
		EraseTopEntry ();
		//if (!dem_framepos)
		//	start_of_demo = true;
	}

	if (connectionless)					// **** UNTESTED (never encountered) ****
	{
		// TODO: handle various commands
		return CL_GetDemoMessage ();
	}

	return 1;
}

/*
====================
CL_GetMessage

Handles recording and playback of demos, on top of NET_ code
====================
*/
int CL_GetMessage (void)
{
	int		r;

	// JDH: pause if in menu/console when capturing a demo to movie
	if (cls.capturedemo)
	{
		if ((cls.signon == SIGNONS) && !Movie_IsActive())
			return 0;
	}

	// by joe: pause during demo
	if (cl.paused & 2)
		return 0;

	CheckDZipCompletion ();
	if (dz_app_op)
		return 0;

	if (cls.demoplayback)
		return CL_GetDemoMessage ();

	while (1)
	{
		r = NET_GetMessage (cls.netcon);
		if (r != 1 && r != 2)
			return r;

		// discard nop keepalive message
		if (net_message.cursize == 1 && net_message.data[0] == svc_nop)
			Con_Print ("<-- server to client keepalive\n");
		else
			break;
	}

// JDH: moved to after CL_ParseServerMessage
//	if (cls.demorecording)
//		CL_WriteDemoMessage (cls.demofile, cl.viewangles);

	// joe: support for recording demos after connecting
	if (cls.signon < 2)
	{
		memcpy (demo_head[cls.signon], net_message.data, net_message.cursize);
		demo_head_size[cls.signon] = net_message.cursize;
	}

	return r;
}

/*
void CL_SkipEntityUpdate (int bits)
{
	int	i, num, count = 0;

	if (bits & U_MOREBITS)
		bits |= (MSG_ReadByte() << 8);

	count += (bits & U_LONGENTITY) ? 2 : 1;

	if (bits & U_FRAME)
		count++;

	for (i = 0; i < 3; i++)
	{
		if (bits & (U_ORIGIN1 << i))
			count += 2;

		num = (i == 0) ? U_ANGLE1 : (i == 1) ? U_ANGLE2 : U_ANGLE3;
		if (bits & num)
			count++;
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (bits & U_MOREBITS2)
			bits |= (MSG_ReadByte () << 16);

		if (bits & U_MODEL)
			count += 2;
		if (bits & U_COLORMAP_H2)
			count++;
		if (bits & U_SKIN_H2)
			count += 2;		// skinnum & drawflags
		if (bits & U_EFFECTS_H2)
			count++;
		if (bits & U_SCALE)
			count += 2;		// scale & abslight

		msg_readcount += count;
		return;
	}
#endif

	if (bits & U_MODEL)
		count += ((cl.protocol == PROTOCOL_VERSION_STD) ? 1 : 2);
	if (bits & U_COLORMAP)
		count++;
	if (bits & U_SKIN)
		count++;
	if (bits & U_EFFECTS)
		count++;

	msg_readcount += count;

	if (bits & U_TRANS)
	{
		i = MSG_ReadFloat ();
		MSG_ReadFloat ();
		if (i == 2)
			MSG_ReadFloat ();
	}
}

// size of messages using standard protocol (-1 means special handling is needed, -9 means bad code)
int cl_msgsizes[53] =
{
	0, 0, 0, 5, 4, 2, -1, -1, -1, -1, 3, -1, -1, -1, 3, -1, 2, 2,			// svc_bad to svc_updatecolors (0 to 17)
	11, 8, -1, -1, -1, -1, 1, 1, -1, 0, 0, -1, 0, -1, 2, 0, -1,				// svc_particle to svc_cutscene (18 to 34)
	-1, -1,	-1, -9, -9, -9, 6, -9, -9, -9, -9, -9, -9, -9, -9, 2, -1 		// svc_showlmp to svc_fog_neh (35 to 51)
};

#define MSG_SKIPSTRING()	\
	while (1) {		\
		if (msg_readcount >= net_message.cursize)	\
		{	\
			msg_badread = true;	\
			break;	\
		}	\
		if (!net_message.data[msg_readcount++])	\
			break;	\
	}

#define MSG_SKIPSTRINGLIST() \
	do {	\
		MSG_SKIPSTRING();	\
	if (!net_message.data[msg_readcount++])	\
		break;	\
	} while (!msg_badread);
*/

/*
====================
CL_ParseDemoMessage (JDH)
====================
*/
/*qboolean CL_ParseDemoMessage (void)
{
	int				cmd, val;
	entity_state_t	entstate;
	float			time;

	MSG_BeginReading ();

	while (1)
	{
		if (msg_badread)
			break;

		cmd = MSG_ReadByte ();
		if (cmd == -1)
			break;			// end of message

		if (cmd & U_SIGNAL)
		{
			CL_SkipEntityUpdate (cmd & 127);
			continue;
		}

		if ((cmd > 51) || (cmd == svc_bad) || (cmd == svc_disconnect))
			return false;

		val = cl_msgsizes[cmd];
		if (val == -9)
			return false;		// unknown command

		if (val >= 0)
		{
			msg_readcount += val;
			continue;
		}

		switch (cmd)
		{
		case svc_time:
		//	time doesn't always increase (eg. map restarted in same demo)
			time = MSG_ReadFloat ();
			if (time < cl_demo_endtime)
				return false;		// restart of level, or new level
			cl_demo_endtime = time;
			return true;		// assume only 1 svc_time per message

		case svc_sound:
			CL_ParseStartSoundPacket (true);
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound (true);
			break;

		case svc_serverinfo:
			val = MSG_ReadLong ();
			if (!CL_IsKnownProtocol (&val))
				return false;
			msg_readcount += 2;
			MSG_SKIPSTRING();			// server greeting
			MSG_SKIPSTRINGLIST();		// model precache
			MSG_SKIPSTRINGLIST();		// sound precache
			break;

		case svc_lightstyle:
		case svc_updatename:
			MSG_ReadByte ();		// (and follow through to MSG_ReadString)
		case svc_print:
		case svc_stufftext:
		case svc_centerprint:
		case svc_finale:
		case svc_cutscene:
		case svc_hidelmp:
		case svc_skybox:
			MSG_SKIPSTRING ();
			break;

		case svc_spawnbaseline:
			MSG_ReadShort ();		// (and follow through to CL_ParseBaseline)
		case svc_spawnstatic:
			CL_ParseBaseline (&entstate);
			break;

		case svc_clientdata:
			msg_readcount += CL_ClientdataSize (MSG_ReadShort ());
			break;

		case svc_temp_entity:
			CL_ParseTEnt (true);
			break;

		case svc_showlmp:
			MSG_SKIPSTRING ();
			MSG_SKIPSTRING ();
			msg_readcount += 2;
			break;

		case svc_fog_fitz:
			msg_readcount += 6;
			break;

		case svc_fog_neh:
			if (MSG_ReadByte ())
				msg_readcount += 7;
			break;
		}
	}

	return true;
}
*/

/*
====================
CL_FindDemoLength (JDH)
 - parses all the demo messages from the current point forward,
   and records the largest svc_time value
====================
*/
/*void CL_FindDemoEndTime (void)
{
	long	orig_pos, orig_protocol;
	int		r;
	float	time;

	orig_pos = ftell (cls.demofile);
	orig_protocol = cl.protocol;

	while (1)
	{
		r = fread (&net_message.cursize, 4, 1, cls.demofile);
		if (r != 1)
			break;
		if (fseek (cls.demofile, 12, SEEK_CUR))		// viewangles
			break;

		net_message.cursize = LittleLong (net_message.cursize);
		if (net_message.cursize <= MAX_MSGLEN)
		{
			if (net_message.cursize >= 5)
			{
				// messages usually start with time command, so try that first:
				r = fread (net_message.data, 5, 1, cls.demofile);
				if (r != 1)
					break;
				if (net_message.data[0] == svc_time)
				{
					time = LittleFloat (*(float *) (net_message.data + 1));
					//if (time > cl_demo_endtime)
					//	cl_demo_endtime = time;
					if (time < cl_demo_endtime)
						break;		// restart of level, or new level
					cl_demo_endtime = time;
					fseek (cls.demofile, net_message.cursize-5, SEEK_CUR);
				}
				else
				{
					if (net_message.cursize > 5)
					{
						r = fread (net_message.data+5, net_message.cursize-5, 1, cls.demofile);
						if (r != 1)
							break;
					}

					if (!CL_ParseDemoMessage ())
						break;
				}

				continue;
			}
		}

		if (fseek (cls.demofile, net_message.cursize, SEEK_CUR))
			break;
	}

	fseek (cls.demofile, orig_pos, SEEK_SET);
	cl.protocol = orig_protocol;
}
*/

/*
====================
CL_FindDemoEndTime (JDH)
 - parses all the demo messages from the current point forward,
   and records the largest svc_time value
   (FIXME? - uses svc_time only if it's the first command in a message
====================
*/
void CL_FindDemoEndTime (void)
{
	long	orig_pos, /*orig_protocol,*/ bytes_read = 0;
	int		size;
	size_t	read_size;
	float	time;
	byte	buf[24];

	orig_pos = ftell (cls.demofile);
//	orig_protocol = cl.protocol;

	if (is_qwd)
		read_size = 5;		// packet time (4) + packet type (1)
	else
		read_size = 21;		// msg size (4) + viewangles (12) + first 5 bytes of message

	while (1)
	{
	//  (bounds is checked before doing fread, since a demo in a pak
	//   may give a successful fread even though eof has been reached)

		if (orig_pos + bytes_read + read_size > cl_demo_filestart + cl_demo_filesize)
			break;

		if (fread (buf, read_size, 1, cls.demofile) != 1)
			break;

		bytes_read += read_size;

		if (is_qwd)
		{
			time = LittleFloat(*(float *) buf);
			if (time < cl_demo_endtime)
				break;			// restarting level, or new level
			cl_demo_endtime = time;

			if (buf[4] == dem_set)
			{
				if (fseek (cls.demofile, 8, SEEK_CUR))		// sequence numbers
					break;
				bytes_read += 8;
				continue;
			}
			else if (buf[4] == dem_cmd)
			{
				if (fseek (cls.demofile, 36, SEEK_CUR))		// usercmd_t (qw) + viewangles
					break;
				bytes_read += 36;
				continue;
			}
			else if (buf[4] == dem_read)
			{
				if (fread (&size, 4, 1, cls.demofile) != 1)		// message size
					break;
				bytes_read += 4;
				size = LittleLong (size) & 0xFFFF;
			}
			else break;
		}
		else
		{
			size = LittleLong (*(int *) buf) & 0xFFFF;
			if (size >= 5)
			{
				// messages usually start with time command:
				if (buf[16] == svc_time)
				{
					time = LittleFloat (*(float *) (buf + 17));
					//if (time > cl_demo_endtime)
					//	cl_demo_endtime = time;
					if (time < cl_demo_endtime)
						break;			// restarting level, or new level
					cl_demo_endtime = time;
				}
			}

			size -= 5;
		}

		if (fseek (cls.demofile, size, SEEK_CUR))
			break;
		bytes_read += size;

#ifdef _DEBUG
		if (orig_pos + bytes_read != ftell(cls.demofile))
			bytes_read *= 1;
#endif
	}

	fseek (cls.demofile, orig_pos, SEEK_SET);
//	cl.protocol = orig_protocol;
}

extern float scr_centertime_off;

/*
====================
CL_DemoSeek (JDH)
   jumps to given position [0..1] in demo
====================
*/
void CL_DemoSeek (float pos)
{
	int			r, waspaused;
	entity_t	*ent;
#ifdef OLDSEEK
	double		desttime;
#else
	long		destpos;
#endif

#ifdef HEXEN2_SUPPORT
#ifndef _DEBUG			// ***WIP***
	if (hexen2)
	{
		SCR_CenterPrint ("Sorry!  Quick-seek doesn't work\nfor Hexen II demos yet...");
		return;		// **** FIXME ****
	}
#endif
#endif

#ifdef OLDSEEK
	if (!cl_demo_endtime)
	{
	// figure out how long the demo is (time-wise)
		CL_FindDemoEndTime ();
		if (!cl_demo_endtime)
			return;			// parsing failed
	}

	desttime = cl_demo_starttime + pos*(cl_demo_endtime - cl_demo_starttime);
	if (desttime == cl.ctime)
		return;

	if (desttime < cl.ctime)
#else
	destpos = cl_demo_filestart + pos*cl_demo_filesize;
	if (destpos < ftell(cls.demofile))
#endif
	{
		// clear the stack and go back to the beginning of demo
		// (actually, to after cls.signon = SIGNONS)
		r = ClearFrameposStack ();

//		if (r)
//			fseek (cls.demofile, r, SEEK_SET);
		fseek (cls.demofile, cl_demo_filestart, SEEK_SET);

		CL_ResetState ();
/*		
		memcpy (cl.stats, demo_state_orig.stats, sizeof(cl.stats));		// needed for STAT_MONSTERS, maybe others
		cl.viewentity = demo_state_orig.viewentity;
		cl.viewheight = demo_state_orig.viewheight;
		
#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			cl.v = demo_state_orig.entvars;
			cl.info_mask = demo_state_orig.info_mask;
			cl.info_mask2 = demo_state_orig.info_mask2;
		}
		else
#endif
*/
		scr_centertime_off = 0;

		/** FIXME: some more values may need to be reset to their original values,
		    as set during signon (maybe name, frags, colors) **/
	/*
	//	cl.time = desttime;
		cl.mtime = 0;
	//	cl.mtime = cl_demo_starttime;
	//	cl.mtime_prev = 0;
		VectorClear (cl.mvelocity);
		VectorClear (cl.mviewangles);
	*/
#ifndef OLDSEEK
//		cl.ctime = 0;
#endif
	}

#ifdef OLDSEEK
	cl.ctime = desttime;
#endif
//	cl.paused &= ~2;
	waspaused = cl.paused;
	cl.paused = 0;
	cl_demoseek = true;
	Cvar_SetDirect (&cl_demorewind, "0");

	do {
		r = CL_GetMessage ();
		if (r == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (r)
		{
			//cl.last_received_message = realtime;
			CL_ParseServerMessage ();
			cl.time = cl.mtime;		// cl.time must be kept current for particles, lights, models, etc.

			/*if (cl_demorewind.value)
			{
			//	if (start_of_demo || (cl.mtime <= desttime))
				if (!dem_framepos || (cl.mtime <= desttime))
					break;
			}
			else*/ 
#ifdef OLDSEEK
			if ((cls.signon == SIGNONS) && (cl.mtime >= desttime))
#else
			if ((cls.signon == SIGNONS) && (ftell(cls.demofile) >= destpos))
#endif
				break;
		}
		else
		{
			Cbuf_Execute ();
		}

	} while (cls.state == ca_connected);


//	Cvar_SetDirect (&cl_demorewind, "0");
	cl_demoseek = false;
#ifdef OLDSEEK
	cl.time = cl.ctime;
#else
	cl.ctime = cl.time;
#endif
	//	cl.mtime_prev = cl.mtime;		// so previous update is not used in interpolation (CL_LerpPoint) - removed 2010/04/30
	cl.paused = waspaused;
	if (waspaused)
	{
	// normally, refdef is not recalculated when paused, so we have to force it
		CL_RelinkPlayer (1.0);
		ent = &cl_entities[cl.viewentity];
		VectorCopy (ent->msg_origin, ent->origin);
		VectorCopy (ent->msg_angles, ent->angles);
		V_CalcRefdef ();
	}

// Note: This function was called in response to a keypress, which was checked by the
// Sys_SendKeyEvents call in _Host_Frame.  So we now (indirectly) return to that point.
}

/*
====================
CL_GetDemoProgress (JDH)
  returns percentage of demo playback completed, 0 to 1
====================
*/
float CL_GetDemoProgress (void)
{
	if (cl_demo_endtime)
		return (cl.time - cl_demo_starttime)/(cl_demo_endtime - cl_demo_starttime);

	return (ftell(cls.demofile) - cl_demo_filestart) / (float)cl_demo_filesize;
}

/*
====================
StartDZProcess
====================
*/
#ifdef _WIN32
static HANDLE DZStartProcess (dz_op_t op, const char *dzname, const char *path, const char *filename)
{
	char				*args, cmdline[512];
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	memset (&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	if (op == DZ_OP_EXTRACT)
		args = "-x -f";
	else if (op == DZ_OP_VERIFY)
		args = "-e -v";
	else
		args = va("%s -e -o", filename);		// -e: quit on first error

	Q_snprintfz (cmdline, sizeof(cmdline), "\"%s/"DZIP_APPNAME"\" %s \"%s\"", com_basedir, args, dzname);

	if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, path, &si, &pi))
	{
		Con_Printf ("\x02""Couldn't execute %s/"DZIP_APPNAME"\n", com_basedir);
		return NULL;
	}

	dz_app_op = op;
	dz_exitcode = DZ_APP_ERROR;		// until we find out otherwise
	hDZipThread = pi.hThread;
	return pi.hProcess;
}
#else
static qboolean DZStartProcess (dz_op_t op, const char *dzname, const char *path, const char *filename)
{
	const char	*args[6];
	char	dzipAppPath[MAX_OSPATH], fullpath[MAX_OSPATH]/*, dz_filepath[MAX_OSPATH], demo[MAX_OSPATH]*/;
	pid_t	pid;
	int		status;

	dz_exitcode = DZ_APP_ERROR;
	if (!realpath(com_basedir, dzipAppPath))
	{
		Con_Printf ("Couldn't realpath '%s'\n", com_basedir);
		return false;
	}

	if (!realpath(path, fullpath))
	{
		Con_Printf ("Couldn't realpath '%s'\n", path);
		return false;
	}

	if (chdir(fullpath) == -1)
	{
		Con_Printf ("Couldn't chdir to '%s'\n", fullpath);
		return false;
	}

//	Q_snprintfz (dz_filepath, sizeof(dz_filepath), "%s/%s", fullpath, dzname);

	switch (pid = fork())
	{
	case -1:
		Con_Print ("Couldn't create subprocess\n");
		return false;

	case 0:
		args[0] = DZIP_APPNAME;
		if (op == DZ_OP_EXTRACT)
		{
			args[1] = "-x";
			args[2] = "-f";
			args[3] = dzname/*dz_filepath*/;
			args[4] = NULL;
		}
		else if (op == DZ_OP_VERIFY)
		{
			args[1] = "-v";
			args[2] = "-e";		// halt on first error
			args[3] = dzname/*dz_filepath*/;
			args[4] = NULL;
		}
		else	// compress
		{
			//Q_snprintfz (demo, sizeof(demo), "%s/%s", fullpath, filename);
			args[1] = filename/*demo*/;
			args[2] = "-e";
			args[3] = "-o";
			args[4] = dzname;
			args[5] = NULL;
		}

		if (execvp(va("%s/"DZIP_APPNAME, dzipAppPath), (char **)args) == -1)
		{
			Con_Printf ("Couldn't execute %s/"DZIP_APPNAME"\n", com_basedir);
			exit (-1);
		}

	default:
		if (waitpid(pid, &status, 0) == -1)
		{
			Con_Print ("waitpid failed\n");
			return false;
		}
		break;
	}

	if (chdir(dzipAppPath) == -1)
	{
		Con_Printf ("Couldn't chdir to '%s'\n", dzipAppPath);
		return false;
	}

	dz_app_op = op;
	dz_exitcode = status;
	return true;
}
#endif

void DZEndProcess (void)
{
#ifdef _WIN32
	CloseHandle (hDZipThread);
	CloseHandle (hDZipProcess);
	hDZipProcess = NULL;
#else
	hDZipProcess = false;
#endif
}

void DZCompressError (void)
{
	Con_Print ("\n");
	Con_Print ("\x02""Warning: demo compression failed\n");
	dz_app_op = DZ_OP_NONE;
}

void DZCompressFinish (void)
{
	char	dempath[MAX_OSPATH];

	Con_Printf ("Deleting %s... ", cl_demo_filepath);

	Q_snprintfz (dempath, sizeof(dempath), "%s/%s", com_gamedir, cl_demo_filepath);
	remove (dempath);
	Con_Print ("Done!\n");
}

/*
====================
CL_VerifyCompressedDemo
  assumes compressed demo is in com_gamedir
====================
*/
static qboolean CL_VerifyCompressedDemo (const char *dzname)
{
	char	dzpath[MAX_OSPATH];

	Con_Printf ("Verifying %s... ", dzname);

	if (dzlib_loaded)
	{
		// don't need to realpath com_gamedir here, since dzlib uses reQuiem's environment
		Q_snprintfz (dzpath, sizeof(dzpath), "%s/%s", com_gamedir, dzname);

		dz_app_op = DZ_OP_NONE;		// in case compression step was done by dz app
		if (Dzip_Verify (dzpath))
		{
			DZCompressFinish ();
			return true;
		}
		remove (dzpath);
		return false;
	}
	else
	{
		hDZipProcess = DZStartProcess (DZ_OP_VERIFY, dzname, com_gamedir, NULL);
		// CheckDZipCompletion will handle finish

		return (hDZipProcess ? true : false);
	}
}

qboolean DZWaitForApp (void)
{
#ifdef _WIN32
	DWORD exitcode;
#endif

	if (!hDZipProcess)
		return false;

#ifdef _WIN32
	while (1)
	{
		if (!GetExitCodeProcess(hDZipProcess, &exitcode))
		{
			exitcode = DZ_APP_ERROR;
			break;
		}
		if (exitcode != STILL_ACTIVE)
			break;
	}

	dz_exitcode = exitcode;
#endif

	DZEndProcess ();
	return true;
}

const char DZ_OVERWRITE_PROMPT[] =
{
	"Overwrite existing file %s?\n"
	"\n"
	"(\x02Yes/\x02No)"
};
/*
====================
CL_CompressDemo (JDH)
  assumes cl_demo_filepath is set to filename of .dem, and demo is in com_gamedir
====================
*/
void CL_CompressDemo (qboolean disconnecting, qboolean usezip)
{
	int len, i;
	char prompt[MAX_OSPATH];

	len = COM_StripExtension (cl_demo_filepath, dz_filename, sizeof(dz_filename));
//	len = strlen (dz_filename);
	Q_strcpy (dz_filename+len, (usezip ? ".zip" : ".dz"), sizeof(dz_filename)-len);

	if (COM_FileExists (va("%s/%s", com_gamedir, dz_filename)))
	{
		if (!COM_DzipIsMounted (COM_SkipPath(com_gamedir), dz_filename))
		{
			Q_snprintfz (prompt, sizeof(prompt), DZ_OVERWRITE_PROMPT, dz_filename);
			if (SCR_ModalMessage (prompt, "YN") == 0)
			{
				remove (va("%s/%s", com_gamedir, dz_filename));
				goto DOCOMPRESS;
			}
		}

		// try to generate a unique filename:
		Q_snprintfz (dz_filename+len, sizeof(dz_filename)-len, "_xxx%s", (usezip ? ".zip" : ".dz"));

		for (i = 1; i < 1000; i++)
		{
			strncpy (dz_filename+len+1, va("%03d", i), 3);		// use strncpy so extension remains
			if (!COM_FileExists (va("%s/%s", com_gamedir, dz_filename)))
				break;
		}
	}

DOCOMPRESS:
	Con_Print ("Compressing demo... ");

// use app if available, unless we're in the process of quitting
	if ((disconnecting && dzlib_loaded) || !DZ_APPEXISTS() || usezip)
	{
		if (Dzip_CompressFile (com_gamedir, cl_demo_filepath, dz_filename, usezip))
		{
			if (CL_VerifyCompressedDemo (dz_filename))
				return;
		}
	}
	else
	{
		hDZipProcess = DZStartProcess (DZ_OP_COMPRESS, dz_filename, com_gamedir, cl_demo_filepath);

		if (hDZipProcess && disconnecting)		// can't rely on message loop to monitor progress
		{
			if (!DZWaitForApp () || (dz_exitcode != DZ_APP_NOERR))
				goto ERROR_EXIT;

			if (!CL_VerifyCompressedDemo (dz_filename))
				goto ERROR_EXIT;

			if (hDZipProcess)
			{
				if (!DZWaitForApp () || (dz_exitcode != DZ_APP_NOERR))
					goto ERROR_EXIT;

				DZCompressFinish ();
				dz_app_op = DZ_OP_NONE;
			}
		}

		return;		// CheckDZipCompletion will handle verification
	}

ERROR_EXIT:
	DZCompressError ();
}

const char DZ_COMPRESS_PROMPT[] =
{
	"Would you like your demo\n"
	"to be compressed via %s?\n"
	"\n"
	"(\x02Yes/\x02No/\x02""Always/Ne\x02ver)"
};

/*
====================
CL_StopRecord

stop recording a demo
====================
*/
void CL_StopRecord (qboolean disconnecting)
{
	int 		result;
	char		str[MAX_OSPATH];
	qboolean	usezip;

#ifdef HEXEN2_SUPPORT
	intro_playing = false;
#endif

// write a disconnect message to the demo file
	SZ_Clear (&net_message);
	MSG_WriteByte (&net_message, svc_disconnect);
	CL_WriteDemoMessage (cls.demofile, cl.viewangles);

// finish up
	result = COM_FileLength (cls.demofile);
	fclose (cls.demofile);
	cls.demofile = NULL;
	cls.demorecording = false;
	if (result <= 22)			// nothing but cdtrack, viewangles, and svc_disconnect
	{
		Q_snprintfz (str, sizeof(str), "%s/%s", com_gamedir, cl_demo_filepath);
		remove (str);
		Con_Printf ("Removed empty demo file \"%s\"\n", cl_demo_filepath);
		cl_demo_filepath[0] = 0;
		return;
	}
	else
		Con_Print ("Completed demo\n");

// JDH: compress the demo, if desired:
	if (!cl_demo_compress.value)
		return;

	if (!dzlib_loaded && !DZ_APPEXISTS())
		return;

	usezip = (dzlib_loaded && dz_canzip && (cl_demo_compress_fmt.value == 1));

	if (cl_demo_compress.value < 2)
	{
		Q_snprintfz (str, sizeof(str), DZ_COMPRESS_PROMPT, (usezip ? "zip" : "dzip"));
		result = SCR_ModalMessage (str, "YNAV");

		if (result == 2)	// 'A' - always compress
		{
			Cvar_SetValueDirect (&cl_demo_compress, 2);
			Con_Print ("cl_demo_compress set to 2 (always compress)\n");
		}
		else if (result == 3)	// 'V' - never compress
		{
			Cvar_SetValueDirect (&cl_demo_compress, 0);
			Con_Print ("cl_demo_compress set to 0 (never compress)\n");
		}

		if ((result != 0) && (result != 2))
			return;
	}

	CL_CompressDemo (disconnecting, usezip);
}

/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void CL_Stop_f (cmd_source_t src)
{
	if (src == SRC_CLIENT)
		return;

	if (!cls.demorecording)
	{
		Con_Print ("Not recording a demo\n");
		return;
	}

	CL_StopRecord (false);
}

/*const char DEMO_OVERWRITE_PROMPT[] =
{
	"There's already a file named\n"
	"%s (%d-%02d-%02d).\n"
	"Overwrite?\n"
	"\n"
	"(\x02Yes/\x02No)"
};*/

// NOTE: the following 2 functions are also used by the movie code,
//  so wording should be applicable to demos & movies
static const char DEMO_OVERWRITE_PROMPT[] =
{
	"%s already exists\n"
	"(recorded %d-%02d-%02d)\n"
	"Overwrite?\n"
	"\n"
	"(\x02Yes/\x02No)"
};

qboolean CL_CheckExistingFile (const char *filepath)
{
	qtime_t	*qtime;
	char prompt[MAX_QPATH+128];

	if ((qtime = Sys_FileTime (filepath)))
	{
		Q_snprintfz (prompt, sizeof(prompt), DEMO_OVERWRITE_PROMPT, COM_SkipPath(filepath),
						qtime->wYear, qtime->wMonth, qtime->wDay);
		if (SCR_ModalMessage (prompt, "YN") != 0)
			return true;
	}

	return false;
}

void CL_MakeRecordingName (char *buf, int bufsize)
{
	time_t		ltime;
	const char	*name;
	int			len;

	time (&ltime);

	if (cls.state == ca_connected)
	{
		name = Host_MapName ();
		if (name && *name)
		{
			len = Q_snprintfz (buf, bufsize, "%s-", name);
			buf += len;
			bufsize -= len;
		}
	}

	len = strftime (buf, bufsize, "%b-%d-%Y-%H%M%S", localtime(&ltime));
	buf += len;
	bufsize -= len;

	name = cl_name.string;
	if ( name[0] && Q_strcasecmp (name, cl_name.defaultvalue))
		Q_snprintfz (buf, bufsize, "-%s", name);
}

/*
====================
CL_Record_f

record <demoname> <map> [cd track]
====================
*/
void CL_Record_f (cmd_source_t src)
{
	int	c, track;
	char	name[MAX_OSPATH], easyname[MAX_OSPATH] = ""/*, mapname[MAX_QPATH]*/;

	if (src == SRC_CLIENT)
		return;

	c = Cmd_Argc ();

	if (c > 4)
	{
		Con_Print ("Usage: record <demoname> [<map> [cd track]]\n");
		return;
	}

	if (cls.demoplayback)		// JDH
	{
		Con_Print ("Cannot record while a demo is playing.\n");
		return;
	}

	if (c == 1 || c == 2)
	{
	//	if (cls.state != ca_connected)
	//	{
	//		Con_Print ("You must be connected for recording\n");
	//		return;
	//	}

		if (c == 1)
		{
			CL_MakeRecordingName (easyname, sizeof(easyname));
		}
		else if (c == 2)
		{
			if (strstr(Cmd_Argv(1), ".."))
			{
				Con_Print ("Relative pathnames are not allowed\n");
				return;
			}
			if (cls.state == ca_connected && cls.signon < 2)
			{
				Con_Print ("Can't record - try again when connected\n");
				return;
			}
		}
	}

	if (cls.demorecording)
		CL_StopRecord (true);		// true so that process completes before returning

// write the forced cd track number, or -1
	if (c == 4)
	{
		track = atoi(Cmd_Argv(3));
		Con_Printf ("Forcing CD track to %i\n", cls.forcetrack);
	}
	else
	{
		track = -1;
	}

	if (easyname[0])
		Q_strcpy (cl_demo_filepath, easyname, sizeof(cl_demo_filepath));
	else
		Q_strcpy (cl_demo_filepath, Cmd_Argv(1), sizeof(cl_demo_filepath));

	COM_ForceExtension (cl_demo_filepath, ".dem", sizeof(cl_demo_filepath));
	Q_snprintfz (name, sizeof(name), "%s/%s", com_gamedir, cl_demo_filepath);

//	if (c > 2)
//		Q_strcpy (mapname, Cmd_Argv(2), sizeof(mapname));		// SCR_ModalMessage may overwrite cmd args

// JDH: check if the file already exists:
	if (CL_CheckExistingFile (name))
		return;

// start the map up
	if (c > 2)
	{
//		Cmd_ExecuteString (va("map %s", mapname), src_command);
		Cmd_ExecuteString (va("map %s", Cmd_Argv(2)), SRC_COMMAND);
	// joe: if couldn't find the map, don't start recording
		if (cls.state != ca_connected)
			return;
	}

// open the demo file
//	COM_ForceExtension (name, ".dem");

	if (!(cls.demofile = fopen(name, "wb")))
	{
		Con_Printf ("ERROR: couldn't open %s\n", name);
		return;
	}

	Con_Printf ("recording to %s\n", name);
	cls.forcetrack = track;
	fprintf (cls.demofile, "%i\n", cls.forcetrack);

	cls.demorecording = true;

	// joe: initialize the demo file if we're already connected
	if (c < 3 && cls.state == ca_connected)
	{
		byte	*data = net_message.data;
		int	i, cursize = net_message.cursize;

		for (i=0 ; i<2 ; i++)
		{
			net_message.data = demo_head[i];
			net_message.cursize = demo_head_size[i];
			CL_WriteDemoMessage (cls.demofile, cl.viewangles);
		}

		net_message.data = demo_head[2];
		SZ_Clear (&net_message);

		// current names, colors, and frag counts
		for (i=0 ; i<cl.maxclients ; i++)
		{
			MSG_WriteByte (&net_message, svc_updatename);
			MSG_WriteByte (&net_message, i);
			MSG_WriteString (&net_message, cl.scores[i].name);
			MSG_WriteByte (&net_message, svc_updatefrags);
			MSG_WriteByte (&net_message, i);
			MSG_WriteShort (&net_message, cl.scores[i].frags);
			MSG_WriteByte (&net_message, svc_updatecolors);
			MSG_WriteByte (&net_message, i);
			MSG_WriteByte (&net_message, cl.scores[i].colors);
		}

		// send all current light styles
		for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
		{
			MSG_WriteByte (&net_message, svc_lightstyle);
			MSG_WriteByte (&net_message, i);
			MSG_WriteString (&net_message, cl_lightstyle[i].map);
		}

		// JDH: monster & secret counts (plus others just in case)
		for (i = 0; i < MAX_CL_STATS; i++)
		{
			MSG_WriteByte (&net_message, svc_updatestat);
			MSG_WriteByte (&net_message, i);
			MSG_WriteLong (&net_message, cl.stats[i]);
		}

		// view entity
		MSG_WriteByte (&net_message, svc_setview);
		MSG_WriteShort (&net_message, cl.viewentity);

		// signon
		MSG_WriteByte (&net_message, svc_signonnum);
		MSG_WriteByte (&net_message, 3);

		// JDH: in case protocol has changed since serverinfo
		MSG_WriteByte (&net_message, svc_version);
		MSG_WriteLong (&net_message, cl.protocol);

		CL_WriteDemoMessage (cls.demofile, cl.viewangles);

		// restore net_message
		net_message.data = data;
		net_message.cursize = cursize;
	}
}

/*void BindDemoKeys (void)
{
	Key_SwapBinding (K_LEFTARROW, "cl_demorewind 1");
	Key_SwapBinding (K_RIGHTARROW, "cl_demorewind 0");

}
*/
/*
====================
CL_Demo_EOF
====================
*/
qboolean CL_Demo_EOF (void)
{
	if (!cls.demofile)
		return true;

	if (cl_demorewind.value)
		return false;

	return (ftell(cls.demofile) - cl_demo_filestart >= cl_demo_filesize);
}

/*
====================
StartPlayingOpenedDemo
====================
*/
//void StartPlayingOpenedDemo (char *path, char *fname)
void StartPlayingOpenedDemo (void)
{
	int			track, count, c;
	qboolean	neg = false;

	cls.demoplayback = true;
	cls.state = ca_connected;
	cls.forcetrack = 0;
	
	cl_demo_starttime = 0;
	cl_demo_endtime = 0;
	
	cl_demo_filestart = ftell(cls.demofile);
	if (!cl_demo_filesize)
		cl_demo_filesize = COM_FileLength (cls.demofile);

	cl.paused &= ~2;		// JDH: in case a demo was playing, but paused
	scr_demo_overlay_time = 0;
//	Cvar_SetDirect (&cl_demospeed, "1.0");

	if (is_qwd)
		return;

	track = 0;
	for (count = 0; count < 8; )
	{
		c = getc (cls.demofile);
		if (c == '\n')
		{
			cls.forcetrack = (neg ? -track : track);
			cl_demo_filestart += count+1;		// so we don't have to parse CD track again if demo is reset
			cl_demo_filesize -= count+1;
			return;
		}

		if ((count++ == 0) && (c == '-'))
		{
			neg = true;
		}
		else
		{
			if (c < '0' || c > '9')
				break;
			track = track * 10 + (c - '0');
		}
	}

// JDH: found invalid chars in track string, so undo read
//    (demos from pre-release Quake don't start with track number)
	fseek (cls.demofile, -count, SEEK_CUR);
}

/*
====================
CheckDZipCompletion
  joe: playing demos from .dz files
  JDH: added demo compression
====================
*/
static void CheckDZipCompletion (void)
{
#ifdef _WIN32
	DWORD	ExitCode;
#endif

	if (!hDZipProcess)
		return;

#ifdef _WIN32
	if (!GetExitCodeProcess(hDZipProcess, &ExitCode))
	{
		Con_Print ("WARNING: GetExitCodeProcess failed\n");
		DZEndProcess ();
		dz_app_op = DZ_OP_NONE;
		if (dz_playback)
		{
			dz_playback = cls.demoplayback = false;
			StopDZPlayback ();
		}
		return;
	}

	if (ExitCode == STILL_ACTIVE)
		return;

	dz_exitcode = ExitCode;
#endif

	DZEndProcess ();

	if (dz_playback)
	{
		if (!dz_app_op || !cls.demoplayback)
		{
			StopDZPlayback ();
			return;
		}

		dz_app_op = DZ_OP_NONE;

		if (!(cls.demofile = fopen(cl_demo_filepath, "rb")))
		{
			Con_Printf ("ERROR: couldn't open %s\n", cl_demo_filepath);
			dz_playback = cls.demoplayback = false;
			cls.demonum = -1;
			return;
		}

		// start playback
		StartPlayingOpenedDemo ();//(NULL, cl_demo_filepath);
		return;
	}

	if (dz_app_op == DZ_OP_COMPRESS)
	{
		if (dz_exitcode == DZ_APP_NOERR)
		{
			if (CL_VerifyCompressedDemo (dz_filename))
				return;
		}
	}
	else if (dz_app_op == DZ_OP_VERIFY)
	{
		if (dz_exitcode == DZ_APP_NOERR)
		{
			dz_app_op = DZ_OP_NONE;
			DZCompressFinish ();
			return;
		}
	}
	else return;

	DZCompressError ();
}

/*
====================
StopDZPlayback
====================
*/
static void StopDZPlayback (void)
{
	if (!hDZipProcess && cl_demo_filepath[0])
	{
		char	temptxt_name[MAX_OSPATH];
		int		len;

		len = COM_StripExtension (cl_demo_filepath, temptxt_name, sizeof(temptxt_name));
		//strcat (temptxt_name, ".txt");
		Q_strcpy (temptxt_name + len, ".txt", sizeof(temptxt_name)-len);
		remove (temptxt_name);
		if (remove(cl_demo_filepath) != 0)
			Con_Printf ("Couldn't delete %s\n", cl_demo_filepath);
		cl_demo_filepath[0] = '\0';
	}
	dz_playback = false;
}

/*
====================
PlayDZDemo
====================
*/
static void PlayDZDemo (const char *name)
{
	char	dem_basedir[MAX_OSPATH], *p;
	int		len;

#ifdef _WIN32
	if (hDZipProcess)
	{
		Con_Printf ("\x02""Cannot unpack -- DZip is still running!\n");
		return;
	}
#endif

	/*if (strstr(name, "..") == name)
	{
		len = Q_snprintfz (dem_basedir, sizeof(dem_basedir), "%s%s", com_basedir, name + 2);
		while (--len > 0)
		{
			if ((dem_basedir[len] == '/') || (dem_basedir[len] == '\\'))
			{
				dem_basedir[len] = 0;
				break;
			}
		}
		//p = strrchr (dem_basedir, '/');
		// *p = 0;
		p = strrchr (name, '/');	// we have to cut off the path for the name
		if (!p)
			p = strrchr (name, '\\');
		if (p)
			name = p+1;
		else
			name += 2;
	}
	else
	{
		sp = COM_FindFile (name, FILE_NO_PAKS);
		if (!sp)
		{
			Con_Printf ("\x02""ERROR: couldn't open %s\n", name);
			return;
		}

		//Q_strcpy (dem_basedir, com_gamedir, sizeof(dem_basedir));
		Q_strcpy (dem_basedir, sp->filename, sizeof(dem_basedir));
	}*/

	// check if the file exists
	if (!COM_FileExists(name))
	{
		Con_Printf ("\x02""ERROR: couldn't open %s\n", name);
		return;
	}

// JDH: make sure dzip app exists:
	if (!DZ_APPEXISTS())
	{
		Con_Printf ("\x02""ERROR: %s/"DZIP_APPNAME" not found\n", com_basedir);
		return;
	}

	len = COM_StripExtension (name, cl_demo_filepath, sizeof(cl_demo_filepath));
	//strcat (cl_demo_filepath, ".dem");
	Q_strcpy (cl_demo_filepath+len, ".dem", sizeof(cl_demo_filepath)-len);

	if ((cls.demofile = fopen(cl_demo_filepath, "rb")))
	{
		// .dem already exists, so just play it
		Con_Printf ("Playing demo from %s\n", cl_demo_filepath);
		StartPlayingOpenedDemo ();//(dem_basedir, cl_demo_filepath);
		return;
	}

	p = strrchr (name, '/');	// we have to cut off the path for the name
	if (!p && !(p = strrchr (name, '\\')))
		return;

	Q_strncpy (dem_basedir, sizeof(dem_basedir), name, p-name+1);
	name = p+1;

/*
#ifndef _WIN32
	if (!realpath(dem_basedir, tmppath))
	{
		Con_Printf ("Couldn't realpath '%s'\n", dem_basedir);
		return;
	}
	Q_strcpy (dem_basedir, tmppath, sizeof(dem_basedir));
#endif
*/

	Con_Print ("\x02" "\nunpacking demo. please wait...\n\n");
	key_dest = key_game;

#ifndef _WIN32
//	name = dz_name;		// Linux version of StartDZProcess uses full path+file
#endif

	// start DZip to unpack the demo
	hDZipProcess = DZStartProcess (DZ_OP_EXTRACT, name, dem_basedir, NULL);
	if (!hDZipProcess)
		return;

	dz_playback = true;

	// demo playback doesn't actually start yet, we just set cls.demoplayback
	// so that CL_StopPlayback() will be called if CL_Disconnect() is issued
	cls.demoplayback = true;
	cls.demofile = NULL;
	cls.state = ca_connected;
}

/*
====================
CL_OpenDemo (JDH)
  - if successful, filepathbuf will contain full path to demo/dzip/zip
====================
*/
FILE * CL_OpenDemo (const char *name, char *filepathbuf, int bufsize)
{
	char testname[MAX_OSPATH];
	FILE *f;
//	int filesize;
	com_fileinfo_t fi;

	if (!strncmp(name, "../", 3) || !strncmp(name, "..\\", 3))
	{
		Q_snprintfz (testname, sizeof(testname), "%s/%s", com_basedir, name + 3);
		COM_DefaultExtension (testname, ".dem", sizeof(testname));

		f = fopen (testname, "rb");
		if (!f)
		{
			COM_ForceExtension (testname, ".dz", sizeof(testname));
			f = fopen (testname, "rb");
			if (!f && dzlib_loaded)
			{
				COM_ForceExtension (testname, ".zip", sizeof(testname));
				f = fopen (testname, "rb");
			}
		}

		if (f)
			Q_strcpy (filepathbuf, testname, bufsize);
	}
	else
	{
		Q_strcpy (testname, name, sizeof(testname));
		COM_DefaultExtension (testname, ".dem", sizeof(testname));

		f = COM_FOpenFile (testname, FILE_ANY_DEMO, &fi);
		if (f)
		{
			cl_demo_filesize = fi.filelen;		// can't use COM_FileLength since demo may be in pak
			if (fi.searchpath->pack)
				Q_snprintfz (filepathbuf, bufsize, "%s/%s", fi.searchpath->pack->filename, fi.name);
			else
				Q_strcpy (filepathbuf, com_netpath, bufsize);
		}
		else if (dzlib_loaded)
		{
		// JDH: this is to handle dzips/zips that were added after initialization,
		//      and therefore weren't mounted by dzlib
			COM_ForceExtension (testname, ".dz", sizeof(testname));
			f = COM_FOpenFile (testname, 0, NULL);
			if (!f)
			{
				COM_ForceExtension (testname, ".zip", sizeof(testname));
				f = COM_FOpenFile (testname, 0, NULL);
			}

			if (f)
				Q_strcpy (filepathbuf, com_netpath, bufsize);
		}
	}

	return f;
}

/*
====================
CL_StartPlayback
====================
*/
void CL_StartPlayback (const char *filename)
{
	char	name[MAX_OSPATH], demopath[MAX_OSPATH], testname[MAX_OSPATH], *ext;
//	int		namelen;

// disconnect from server
	CL_Disconnect (true);

	Cvar_SetValueDirect (&cl_demorewind, 0);
	Cvar_SetValueDirect (&cl_demospeed, 1);

	is_qwd = false;
	cl_demo_filestart = cl_demo_filesize = 0;

// open the demo file
	Q_strcpy (name, filename, sizeof(name));

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		intro_playing = !Q_strcasecmp (name, "t9");
#endif

	/*namelen = strlen(name);
	if ((namelen > 3) && !Q_strcasecmp(name + namelen - 3, ".dz"))
	{
		PlayDZDemo (name);
		return;
	}*/

//	cls.demofile = CL_OpenDemo (name, ".dem");
	cls.demofile = CL_OpenDemo (name, demopath, sizeof(demopath));
	if (!cls.demofile)
	{
		/*cls.demofile = CL_OpenDemo (name, ".dz");
		if (cls.demofile)
		{
			fclose (cls.demofile);
			COM_DefaultExtension (name, ".dz", sizeof(name));
			PlayDZDemo (name);
			return;
		}*/

		goto ERROR_EXIT;
	}

// JDH: check if CL_OpenDemo found a dzip instead of a dem:
	/*if (!strncmp(name, "../", 3) || !strncmp(name, "..\\", 3))
	{
		ext = COM_FileExtension (name);
		if (!strcmp(ext, "dz"))
		{
			fclose (cls.demofile);
			PlayDZDemo (name);
			return;
		}
	}
	if (!dzlib_loaded && !strcmp(COM_FileExtension(com_netpath), "dz"))
	{
		fclose (cls.demofile);
		Q_strcpy (name, COM_SkipPath(com_netpath), sizeof(name));
		PlayDZDemo (name);
		return;
	}*/

	ext = COM_FileExtension (demopath);
	if (dzlib_loaded && (!Q_strcasecmp(ext, "dz") || !Q_strcasecmp(ext, "zip")))
	{
	// from dz/zip that is not mounted
		fclose (cls.demofile);
		Q_strcpy (testname, COM_SkipPath(demopath), sizeof(testname));
		COM_ForceExtension (testname, ".dem", sizeof(testname));

		cls.demofile = Dzip_OpenFromArchive (demopath, testname);
		if (!cls.demofile)
		{
			cls.demofile = Dzip_OpenFromArchive (demopath, "*.dem");
			if (!cls.demofile)
				goto ERROR_EXIT;
		}
	}
	else if (!Q_strcasecmp(ext, "dz"))
	{
		fclose (cls.demofile);
		PlayDZDemo (demopath);
		return;
	}


	Q_strcpy (cl_demo_filepath, demopath, sizeof(cl_demo_filepath));
	Con_Printf ("Playing demo from %s\n", COM_SkipPath(demopath));

/*	namelen = strlen(name);		// calc length again (extension may have been added)
	if (namelen > 3)
	{
		if ((namelen > 4) && !Q_strcasecmp(name + namelen - 4, ".qwd"))
		{
			is_qwd = true;
		}
		else if (!Q_strcasecmp(name + namelen - 3, ".dz"))
		{
			PlayDZDemo (name);
			return;
		}
	}
*/

	if (!Q_strcasecmp(ext, "qwd"))
		is_qwd = true;

	StartPlayingOpenedDemo ();//(com_filepath->filename, name);
	return;

ERROR_EXIT:
	Con_Printf ("ERROR: couldn't open %s\n", name);
	cls.demonum = -1;		// stop demo loop
	cl_demo_filepath[0] = 0;
}

/*
====================
CL_PlayDemo_f

playdemo [demoname]
====================
*/
void CL_PlayDemo_f (cmd_source_t src)
{
	if (src == SRC_CLIENT)
		return;

	if (Cmd_Argc() == 1)
	{
		Cbuf_AddText ("menu_demos\n", SRC_COMMAND);
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Con_Print ("playdemo <demoname> : plays a demo\n");
		return;
	}

	CL_StartPlayback (Cmd_Argv(1));
}

/*
====================
CL_FinishTimeDemo
====================
*/
void CL_FinishTimeDemo (void)
{
	int	frames;
	float	time;

	cls.timedemo = false;

// the first frame didn't count
	frames = (host_framecount - cls.td_startframe) - 1;
	time = realtime - cls.td_starttime;
	if (!time)
		time = 1;
	Con_Printf ("%i frames %5.1f seconds %5.1f fps\n", frames, time, frames/time);
}

/*
====================
CL_TimeDemo_f

timedemo [demoname]
====================
*/
void CL_TimeDemo_f (cmd_source_t src)
{
	if (src == SRC_CLIENT)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Print ("timedemo <demoname> : gets demo speeds\n");
		return;
	}

	CL_PlayDemo_f (src);

// cls.td_starttime will be grabbed at the second frame of the demo, so
// all the loading time doesn't get counted
	cls.timedemo = true;
	cls.td_startframe = host_framecount;
	cls.td_lastframe = -1;		// get a new message this frame
}

#endif		//#ifndef RQM_SV_ONLY
