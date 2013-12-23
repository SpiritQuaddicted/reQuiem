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
// host.c -- coordinates spawning and killing of local servers

#include "quakedef.h"

#ifndef RQM_SV_ONLY
#include "movie.h"
#endif

#include "music.h"
/*

A server can always be started, even if the system started out as a client
to a remote system.

A client can NOT be started if the system started as a dedicated server.

Memory is cleared / released when a server or client begins, not when they end.

*/

quakeparms_t	host_parms;

qboolean	host_initialized;		// true if into command execution

double		host_frametime;
double		host_time;
double		realtime;			// without any filtering or bounding
double		oldrealtime;			// last frame run
int		host_framecount;

int		host_hunklevel;

int		minimum_memory;

client_t	*host_client;			// current client

#ifndef RQM_SV_ONLY
client_static_t	cls;
client_state_t	cl;
#endif

jmp_buf 	host_abortserver;

byte		*host_basepal;
byte		*host_colormap;

int		fps_count;

cvar_t	host_framerate = {"host_framerate", "0"};	// set for slow motion
cvar_t	host_speeds = {"host_speeds", "0"};		// set for running times
cvar_t	host_mapname = {"mapname", "", CVAR_FLAG_READONLY};		// JDH: moved from cl_main
cvar_t	host_maxfps = {"host_maxfps", "72", CVAR_FLAG_ARCHIVE}; //johnfitz

cvar_t	sys_ticrate = {"sys_ticrate", "0.05"};
cvar_t	serverprofile = {"serverprofile", "0"};

cvar_t	fraglimit = {"fraglimit", "20", CVAR_FLAG_SERVER};	// JT021105 - set default to 20
cvar_t	timelimit = {"timelimit", "0", CVAR_FLAG_SERVER};
cvar_t	teamplay = {"teamplay", "0", CVAR_FLAG_SERVER};

cvar_t	samelevel = {"samelevel", "0"};
cvar_t	noexit = {"noexit", "0", CVAR_FLAG_SERVER};

cvar_t	developer = {"developer", "0"};

cvar_t	skill = {"skill", "1"};			// 0 - 3
cvar_t	deathmatch = {"deathmatch", "0"};	// 0, 1, or 2
cvar_t	coop = {"coop", "0"};			// 0 or 1

cvar_t	pausable = {"pausable", "1"};

cvar_t	temp1 = {"temp1", "0"};

void Host_WriteConfig_f (cmd_source_t src);

/*
================
Host_EndGame
================
*/
void Host_EndGame (const char *message, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr, message);
	vsnprintf (string, sizeof(string), message, argptr);
	va_end (argptr);
	Con_DPrintf ("Host_EndGame: %s\n", string);

	if (sv.active)
		Host_ShutdownServer (false);

#ifndef RQM_SV_ONLY
	if (cls.state == ca_dedicated)
		Sys_Error ("Host_EndGame: %s\n",string);	// dedicated servers exit

	if (cls.capturedemo || (cls.demonum == -1) || !CL_NextDemo())
		CL_Disconnect (true);
#else
	Sys_Error ("Host_EndGame: %s\n",string);	// dedicated servers exit
#endif

	longjmp (host_abortserver, 1);
}

/*
================
Host_Error

This shuts down both the client and server
================
*/
void Host_Error (const char *error, ...)
{
	va_list			argptr;
	char			string[1024], border[256];
	int				len, maxlen, i;
	static qboolean inerror = false;
	extern int		con_linewidth;

	if (inerror)
		Sys_Error ("Host_Error: recursively entered");
	inerror = true;

#ifndef RQM_SV_ONLY
	SCR_EndLoadingPlaque ();		// reenable screen updates
#endif

	va_start (argptr, error);
	vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	maxlen = len = 0;
	for (i = 0; string[i]; i++)
	{
		if (string[i] == '\n')
			len = 0;
		else
		{
			len++;
			if (len > maxlen)
				maxlen = len;
		}
	}

	len = min (maxlen+12, con_linewidth-1);
	border[len] = 0;
	for (i=0; i<len; i++)
		border[i] = '=';

//	Con_Print ("\n===========================\n");
//	Con_Printf ("Host_Error: %s\n",string);
//	Con_Print ("===========================\n\n");

	Con_Printf ("\n%s\n", border);
	Con_Printf ("Host_Error: %s\n",string);
	Con_Printf ("%s\n\n", border);

	if (sv.active)
		Host_ShutdownServer (false);

#ifndef RQM_SV_ONLY
	if (cls.state == ca_dedicated)
		Sys_Error ("Host_Error: %s\n",string);	// dedicated servers exit

	CL_Disconnect (true);
	cls.demonum = -1;
	key_dest = key_console;		// JDH
#else
	Sys_Error ("Host_Error: %s\n",string);	// dedicated servers exit
#endif

	inerror = false;
	longjmp (host_abortserver, 1);
}

void Host_SetMapName (const char *name)
{
	int flags = host_mapname.flags;

	host_mapname.flags &= ~CVAR_FLAG_READONLY;
	Cvar_SetDirect (&host_mapname, name);
	host_mapname.flags = flags;
}

// by joe
const char *Host_MapName (void)
{
	return host_mapname.string;
}

/*
================
Host_FindMaxClients
================
*/
void Host_FindMaxClients (void)
{
	int	i;

#ifndef RQM_SV_ONLY
	svs.maxclients = 1;

	if ((i = COM_CheckParm ("-dedicated")))
	{
		cls.state = ca_dedicated;
		if (i != (com_argc - 1))
			svs.maxclients = Q_atoi (com_argv[i+1]);
		else
			svs.maxclients = 8;
	}
	else
	{
		cls.state = ca_disconnected;
	}
#else
	svs.maxclients = 8;
#endif

	if ((i = COM_CheckParm ("-listen")))
	{
#ifndef RQM_SV_ONLY
		if (cls.state == ca_dedicated)
#endif
			Sys_Error ("Only one of -dedicated or -listen can be specified");
		
		if (i != (com_argc - 1))
			svs.maxclients = Q_atoi (com_argv[i+1]);
		else
			svs.maxclients = 8;
	}

	if (svs.maxclients < 1)
		svs.maxclients = 8;
	else if (svs.maxclients > MAX_SCOREBOARD)
		svs.maxclients = MAX_SCOREBOARD;

	svs.maxclientslimit = max(8, svs.maxclients);
	svs.clients = Hunk_AllocName (svs.maxclientslimit * sizeof(client_t), "clients");

	if (svs.maxclients > 1)
		Cvar_SetValueDirect (&deathmatch, 1.0);
	else
		Cvar_SetValueDirect (&deathmatch, 0.0);
}

/*
=======================
Host_InitLocal
======================
*/
void Host_InitLocal (void)
{
	Host_InitCommands ();

	Cvar_Register (&host_framerate);
	Cvar_RegisterBool (&host_speeds);
/************JDH**************/
	Cvar_RegisterInt (&host_maxfps, 1, 200);	// JDH: max is 200 for purpose of Video Options menu
	Cvar_RegisterString (&host_mapname);		// JDH: moved from cl_main
/************JDH**************/

	Cvar_Register (&sys_ticrate);
	Cvar_RegisterBool (&serverprofile);

	Cvar_RegisterInt (&fraglimit, 0, 100);
	Cvar_RegisterInt (&timelimit, 0, 60);
	Cvar_RegisterInt (&teamplay, 0, 6);
	Cvar_RegisterBool (&samelevel);
	Cvar_RegisterInt (&noexit, 0, 2);
	Cvar_RegisterInt (&skill, 0, 3);
	Cvar_RegisterInt (&developer, 0, 2);
	Cvar_RegisterInt (&deathmatch, 0, 2);
	Cvar_RegisterInt (&coop, 0, 4);		// warpspasm supports 2,3,4

	Cvar_RegisterBool (&pausable);

	Cvar_Register (&temp1);

	Cmd_AddCommand ("writeconfig", Host_WriteConfig_f, 0);	// by joe

	Host_FindMaxClients ();

	host_time = 1.0;		// so a think at time 0 won't get called
}

/*
===============
Host_WriteConfig

Writes key bindings and archived cvars to given cfg
===============
*/
void Host_WriteConfig (const char *cfgname)
{
	char	filepath[MAX_OSPATH];
	int		numlines;
	FILE	*f;

	Q_snprintfz (filepath, sizeof(filepath), "%s/%s", com_gamedir, cfgname);

	numlines = Key_WriteBindings (NULL) + Cvar_WriteVariables (NULL);
	if (!numlines)
	{
		remove (filepath);		// instead of writing empty file, just remove it
		return;
	}

	if (!(f = fopen(filepath, "w")))
	{
		Con_Printf ("Couldn't write %s\n", cfgname);
		return;
	}

	fprintf (f, "// Generated by reQuiem\n");
	Key_WriteBindings (f);
	Cvar_WriteVariables (f);

	fclose (f);
}

/*
===============
Host_WriteConfiguration
===============
*/
void Host_WriteConfiguration (void)
{
// dedicated servers initialize the host but don't parse and set the config.cfg cvars
	if (host_initialized & !isDedicated)
		Host_WriteConfig ("reQuiem.cfg");		// JDH: was config.cfg
}

/*
===============
Host_WriteConfig_f

Writes key bindings and ONLY archived cvars to a custom config file
===============
*/
void Host_WriteConfig_f (cmd_source_t src)
{
	char	name[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Con_Print ("Usage: writeconfig <filename>\n");
		return;
	}

	Q_strcpy (name, Cmd_Argv(1), sizeof(name));
	COM_ForceExtension (name, ".cfg", sizeof(name));

	Con_Printf ("Writing %s\n", name);

	Host_WriteConfig (name);
}

/*
=================
Host_PrintToClient
  (same as SV_ClientPrintf, but with prototype matching Con_Print)
=================
*/
qboolean Host_PrintToClient (const char *msg)
{
	SV_ClientPrint (host_client, msg);
	return true;
}

/*
=================
SV_ClientPrintf

Sends text across to be displayed
FIXME: make this just a stuffed echo?
=================
*/
void SV_ClientPrint (client_t *cl, const char *msg)
{
	MSG_WriteCmd (&host_client->message, svc_print);
	MSG_WriteString (&host_client->message, msg);
}

void SV_ClientPrintf (client_t *cl, const char *fmt, ...)
{
	va_list	argptr;
	char	string[1024];

	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	SV_ClientPrint (cl, string);
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (const char *fmt, ...)
{
	va_list	argptr;
	char	string[1024];
	int	i;

	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && svs.clients[i].spawned)
		{
			MSG_WriteCmd (&svs.clients[i].message, svc_print);
			MSG_WriteString (&svs.clients[i].message, string);
		}
	}
}

/*
=================
Host_ClientCommands

Send text over to the client to be executed
=================
*/
void Host_ClientCommands (const char *fmt, ...)
{
	va_list	argptr;
	char	string[1024];

	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	MSG_WriteCmd (&host_client->message, svc_stufftext);
	MSG_WriteString (&host_client->message, string);
}

/*
=====================
SV_DropClient

Called when the player is getting totally kicked off the host
if (crash = true), don't bother sending signofs
=====================
*/
void SV_DropClient (qboolean crash)
{
	int		i, saveSelf;
	client_t	*client;

	// joe, ProQuake fix: don't drop a client that's already been dropped!
	if (!host_client->active)
		return;

	if (!crash)
	{
		// send any final messages (don't check for errors)
		if (NET_CanSendMessage (host_client->netconnection))
		{
			MSG_WriteCmd (&host_client->message, svc_disconnect);
			NET_SendMessage (host_client->netconnection, &host_client->message);
		}

		if (host_client->edict && host_client->spawned)
		{
		// call the prog function for removing a client
		// this will set the body to a dead frame, among other things
			saveSelf = PR_GLOBAL(self);
			PR_GLOBAL(self) = EDICT_TO_PROG(host_client->edict);
			PR_ExecuteProgram (PR_GLOBAL(ClientDisconnect));
			PR_GLOBAL(self) = saveSelf;
		}

		Sys_Printf ("Client %s removed\n", host_client->name);
	}

// break the net connection
	NET_Close (host_client->netconnection);
	host_client->netconnection = NULL;

// free the client (the body stays around)
	host_client->active = false;
	host_client->name[0] = 0;
	host_client->old_frags = -999999;
	net_activeconnections--;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		memset(&host_client->old_v, 0, sizeof(host_client->old_v));
		ED_ClearEdict(host_client->edict);
		host_client->send_all_v = true;
	}
#endif

// send notification to all clients
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		MSG_WriteCmd (&client->message, svc_updatename);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteString (&client->message, "");
		MSG_WriteCmd (&client->message, svc_updatefrags);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteShort (&client->message, 0);
		MSG_WriteCmd (&client->message, svc_updatecolors);
		MSG_WriteByte (&client->message, host_client - svs.clients);
		MSG_WriteByte (&client->message, 0);
	}
}

/*
==================
Host_ShutdownServer

This only happens at the end of a game, not between levels
==================
*/
void Host_ShutdownServer (qboolean crash)
{
	int		i, count;
	sizebuf_t	buf;
	char		message[4];
	double		start;

	if (!sv.active)
		return;

	sv.active = false;

#ifndef RQM_SV_ONLY
// stop all client sounds immediately
	if (cls.state == ca_connected)
		CL_Disconnect (true);
#endif

// flush any pending messages - like the score!!!
	start = Sys_DoubleTime ();
	do
	{
		count = 0;
		for (i=0, host_client=svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->active && host_client->message.cursize)
			{
				if (NET_CanSendMessage (host_client->netconnection))
				{
					NET_SendMessage (host_client->netconnection, &host_client->message);
					SZ_Clear (&host_client->message);
				}
				else
				{
					NET_GetMessage (host_client->netconnection);
					count++;
				}
			}
		}
		if ((Sys_DoubleTime() - start) > 3.0)
			break;
	}
	while (count);

// make sure all the clients know we're disconnecting
	buf.data = (byte *) message;
	buf.maxsize = 4;
	buf.cursize = 0;
	buf.lastcmdpos = 0;

	MSG_WriteCmd (&buf, svc_disconnect);
	if ((count = NET_SendToAll (&buf, 5)))
		Con_Printf ("Host_ShutdownServer: NET_SendToAll failed for %u clients\n", count);

	for (i=0, host_client=svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_DropClient (crash);

// clear structures
	memset (&sv, 0, sizeof(sv));
	memset (svs.clients, 0, svs.maxclientslimit * sizeof(client_t));
}

/*
================
Host_ClearMemory

This clears all the memory used by both the client and server, but does
not reinitialize anything.
================
*/
void Host_ClearMemory (void)
{
	Con_DPrintf ("Clearing memory\n");

#ifndef RQM_SV_ONLY
	D_FlushCaches ();
#endif

	Mod_ClearAll ();
	if (host_hunklevel)
		Hunk_FreeToLowMark (host_hunklevel);
	memset (&sv, 0, sizeof(sv));

#ifndef RQM_SV_ONLY
	cls.signon = 0;
	memset (&cl, 0, sizeof(cl));
#endif
}

//============================================================================

/*
===================
Host_FilterTime

Returns false if the time is too short to run a frame
===================
*/
qboolean Host_FilterTime (double time)
{
	double	fps;

	realtime += time;

	fps = bound(1.0, host_maxfps.value, 1000);		// JDH: added upper bound
	if (cls.demorecording && (fps > 72))
		fps = 72;		// don't allow higher than 72 fps during recording

	if (realtime - oldrealtime < 1.0 / fps)
	{
#ifndef RQM_SV_ONLY
		if (!cls.timedemo && (!cls.capturedemo || !Movie_IsActive()))		
#endif
			return false;
	}

#ifndef RQM_SV_ONLY
	if (Movie_IsActive())
		host_frametime = Movie_FrameTime ();
#endif
	else 
	{
		if (host_framerate.value > 0)
			host_frametime = host_framerate.value;
		else
		{
			host_frametime = realtime - oldrealtime;
		// don't allow really long or short frames
			host_frametime = bound(0.001, host_frametime, 0.1);
		}
	}

#ifndef RQM_SV_ONLY
	if (cls.demoplayback)
	{
	// JDH: moved this to _after_ bound(host_frametime) to allow faster rewind/ff
		host_frametime *= bound(cl_demospeed.minvalue, cl_demospeed.value, cl_demospeed.maxvalue);
	}
#endif
	
	oldrealtime = realtime;
	return true;
}

/*
===================
Host_GetConsoleCommands

Add them exactly as if they had been typed at the console
===================
*/
void Host_GetConsoleCommands (void)
{
	char	*cmd;

	while (1)
	{
		if (!(cmd = Sys_ConsoleInput()))
			break;
		Cbuf_AddText (cmd, SRC_CONSOLE);
	}
}

/*
==================
Host_ServerFrame
==================
*/
void Host_ServerFrame (void)
{
	// joe, from ProQuake: stuff the port number into the server console once every minute
	static	double	port_time = 0;

	if (port_time > sv.time + 1 || port_time < sv.time - 60)
	{
		port_time = sv.time;
		Cmd_ExecuteString (va("port %d\n", net_hostport), SRC_COMMAND);
	}

	// run the world state
	PR_GLOBAL(frametime) = host_frametime;

	// set the time and clear the general datagram
	SV_ClearDatagram ();

	// check for new clients
	SV_CheckForNewClients ();

	// read client messages
	SV_RunClients ();

	// move things around and think
	// always pause in single player if in console or menus
	if (!sv.paused)
#ifndef RQM_SV_ONLY
		if (svs.maxclients > 1 || key_dest == key_game)
#endif
			SV_Physics ();

	// send all messages to the clients
	SV_SendClientMessages ();
}

/*
==================
Host_Frame

Runs all active servers
==================
*/
void _Host_Frame (double time)
{
#ifndef RQM_SV_ONLY
	static	double	time1 = 0, time2 = 0, time3 = 0;
	int		pass1, pass2, pass3;
#endif

	if (setjmp(host_abortserver))
		return;		// something bad happened, or the server disconnected

	// keep the random time dependent
	rand ();

	// decide the simulation time
	if (!Host_FilterTime(time))
	{
	#ifndef RQM_SV_ONLY
		// joe, ProQuake fix: if we're not doing a frame, still check for lagged moves to send
		if (!sv.active && (cl.movemessages > 2))
			CL_SendLagMove ();
	#endif
		return;			// don't run too fast, or packets will flood out
	}

	// get new key events
	Sys_SendKeyEvents ();

#ifndef RQM_SV_ONLY
	// allow mice or other external controllers to add commands
	IN_Commands ();
#endif

	// process console commands
	Cbuf_Execute ();

	NET_Poll ();

#ifndef RQM_SV_ONLY
	// if running the server locally, make intentions now
	if (sv.active)
		CL_SendCmd ();
#endif

//-------------------
//
// server operations
//
//-------------------

	// check for commands typed to the host
	Host_GetConsoleCommands ();

	if (sv.active)
		Host_ServerFrame ();

//-------------------
//
// client operations
//
//-------------------

#ifndef RQM_SV_ONLY
	// if running the server remotely, send intentions now after
	// the incoming messages have been read
	if (!sv.active)
		CL_SendCmd ();

	host_time += host_frametime;

	// fetch results from server
	if (cls.state == ca_connected)
		CL_ReadFromServer ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		R_UpdateParticles ();
		CL_UpdateEffects ();
	}
#endif

	if (host_speeds.value)
		time1 = Sys_DoubleTime ();

	// update video
	SCR_UpdateScreen ();

	if (host_speeds.value)
		time2 = Sys_DoubleTime ();

	if (cls.signon == SIGNONS)
	{
		// update audio
		S_Update (r_origin, vpn, vright, vup);
		CL_DecayLights ();
	}
	else
	{
		S_Update (vec3_origin, vec3_origin, vec3_origin, vec3_origin);
	}

	CDAudio_Update ();

	if (host_speeds.value)
	{
		pass1 = (time1 - time3) * 1000;
		time3 = Sys_DoubleTime ();
		pass2 = (time2 - time1) * 1000;
		pass3 = (time3 - time2) * 1000;
		Con_Printf ("%3i tot %3i server %3i gfx %3i snd\n", pass1 + pass2 + pass3, pass1, pass2, pass3);
	}

	if (!cls.demoplayback && cl_demorewind.value)
	{
		Cvar_SetValueDirect (&cl_demorewind, 0);
		Con_Print ("Demorewind is only enabled during playback\n");
	}

	// don't allow cheats in multiplayer
	if (cl.gametype == GAME_DEATHMATCH)
	{
		if (!cls.demorecording)
			Cvar_SetValueDirect (&r_fullbright, 0);
		Cvar_SetValueDirect (&r_fullbrightskins, 0);
/*#ifndef GLQUAKE
		Cvar_SetDirect (&r_draworder, "0");
		Cvar_SetDirect (&r_ambient, "0");
		Cvar_SetDirect (&r_drawflat, "0");
#endif*/
	}
/*	else		// 2011-05-31: moved to where cvars are used
	{
		// don't allow cheats when recording a demo
		if (cls.demorecording)
		{
			Cvar_SetValueDirect (&cl_truelightning, 0);
		// don't allow higher than 72 fps during recording
			if (host_maxfps.value > 72)
				Cvar_SetValueDirect (&host_maxfps, 72);
		}
	}
*/

//	if (cls.state == ca_connected)
//		Con_DPrintf ("cl.viewangles = (%.2f, %.2f)\n", cl.viewangles[0], cl.viewangles[1]);

#endif		//#ifndef RQM_SV_ONLY

	host_framecount++;
	fps_count++;
}

void Host_Frame (double time)
{
	double		time1, time2;
	static	double	timetotal;
	static	int	timecount;
	int		i, c, m;

	if (!serverprofile.value)
	{
		_Host_Frame (time);
		return;
	}

	time1 = Sys_DoubleTime ();
	_Host_Frame (time);
	time2 = Sys_DoubleTime ();

	timetotal += time2 - time1;
	timecount++;

	if (timecount < 1000)
		return;

	m = timetotal * 1000 / timecount;
	timecount = timetotal = 0;
	c = 0;
	for (i=0 ; i<svs.maxclients ; i++)
		if (svs.clients[i].active)
			c++;

	Con_Printf ("serverprofile: %2i clients %2i msec\n", c, m);
}

//============================================================================

extern	FILE	*vcrFile;
#define	VCR_SIGNATURE	0x56435231
// "VCR1"

void Host_InitVCR (quakeparms_t *parms)
{
	int	i, len, n;
	char	*p;

	if (COM_CheckParm("-playback"))
	{
		if (com_argc != 2)
			Sys_Error("No other parameters allowed with -playback\n");

		COM_FileOpenRead ("quake.vcr", &vcrFile);
		if (!vcrFile)
			Sys_Error("playback file not found\n");

		fread (&i, 1, sizeof(int), vcrFile);
		if (i != VCR_SIGNATURE)
			Sys_Error("Invalid signature in vcr file\n");

		fread (&com_argc, 1, sizeof(int), vcrFile);
		com_argv = malloc(com_argc * sizeof(char *));
		com_argv[0] = parms->argv[0];
		for (i=0 ; i<com_argc ; i++)
		{
			fread (&len, 1, sizeof(int), vcrFile);
			p = Q_malloc (len);
			fread (p, 1, len, vcrFile);
			com_argv[i+1] = p;
		}
		com_argc++; /* add one for arg[0] */
		parms->argc = com_argc;
		parms->argv = com_argv;
	}

	if ((n = COM_CheckParm("-record")))
	{
		vcrFile = fopen ("quake.vcr", "wb");

		i = VCR_SIGNATURE;
		fwrite (&i, 1, sizeof(int), vcrFile);
		i = com_argc - 1;
		fwrite (&i, 1, sizeof(int), vcrFile);
		for (i = 1; i < com_argc; i++)
		{
			if (i == n)
			{
				len = 10;
				fwrite (&len, 1, sizeof(int), vcrFile);
				fwrite (&"-playback", 1, len, vcrFile);
				continue;
			}
			len = strlen (com_argv[i]) + 1;
			fwrite (&len, 1, sizeof(int), vcrFile);
			fwrite (&com_argv[i], 1, len, vcrFile);
		}
	}
}

#ifndef RQM_SV_ONLY

byte * Host_LoadColormap (void)
{
	byte *colormap = (byte *) COM_LoadHunkFile ("gfx/colormap.lmp", 0);

	if (!colormap)
		colormap = Wad_LoadColormap ();		// beta Quake

	return colormap;
}

byte * Host_LoadPalette (void)
{
	byte *pal = (byte *) COM_LoadMallocFile ("gfx/palette.lmp", 0);

	if (!pal)
		pal = Wad_LoadPalette ();		// beta Quake

	return pal;
}

/*
====================
Host_ReloadPalette
====================
*/
void Host_ReloadPalette (void)
{
	byte *oldpal;

	oldpal = host_basepal;
	host_basepal = Host_LoadPalette ();
	if (!host_basepal)
	{
		if (oldpal)
		{
			host_basepal = oldpal;
			return;
		}
		Sys_Error ("Couldn't load gfx/palette.lmp");
	}

	if (oldpal)
		free (oldpal);

	VID_SetPalette (host_basepal);
}
#endif

/*
====================
Host_ExecCfgs
====================
*/
void Host_ExecCfgs (void)
{
	int i;

// JDH: load engine-specific cfg, if present:  (2009/05/22 - moved to Cmd_Exec_f)
//	if (COM_FindFile("reQuiem.cfg", 0))
//		Cbuf_InsertText ("exec reQuiem.cfg\n", SRC_COMMAND);

	// InsertText means that anything in .rc will be executed *before* reQuiem.cfg
#ifdef HEXEN2_SUPPORT
	if (hexen2 && COM_FindFile ("hexen.rc", 0, NULL))
		Cbuf_InsertText ("exec hexen.rc\n", SRC_COMMAND);
	else
#endif
	Cbuf_InsertText ("exec quake.rc\n", SRC_COMMAND);

	if (!nehahra)
#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
		return;

// JDH: make sure all cfg's get executed before initializing Nehahra & Hexen II vars

	for (i = 10; i && !Cbuf_IsEmpty(); i--)
		Cbuf_Execute ();

	if (nehahra)
		Neh_InitVars ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		Hexen2_InitVars ();
#endif
}

/*
====================
Host_Init
====================
*/
void Host_Init (quakeparms_t *parms)
{
	minimum_memory = (hipnotic || rogue || nehahra) ?
						MINIMUM_MEMORY_LEVELPAK : MINIMUM_MEMORY;

	if (COM_CheckParm("-minmemory"))
		parms->memsize = minimum_memory;

	host_parms = *parms;

	if (parms->memsize < minimum_memory)
		Sys_Error ("Only %4.1f megs of memory available, can't execute game", parms->memsize / (float)0x100000);

	com_argc = parms->argc;
	com_argv = parms->argv;

	Memory_Init (parms->membase, parms->memsize);
	Cbuf_Init ();
	Con_PreInit ();		// JDH: get the console working sooner
	Cmd_Init ();
	Cvar_Init ();
	Neh_Init ();
	V_Init ();

#ifndef RQM_SV_ONLY
	Chase_Init ();
#endif

	Host_InitVCR (parms);
	COM_Init ();
	Host_InitLocal ();
#ifndef RQM_SV_ONLY
	W_LoadWadFile ("gfx.wad");
	Key_Init ();
#endif

	Con_Init ();
//	M_Init ();		// JDH: now called only if not dedicated
	PR_Init ();
	Mod_Init ();
	NET_Init ();
	SV_Init ();

#ifndef RQM_SV_ONLY
	if (cls.state != ca_dedicated)
	{
//		if (!(host_basepal = (byte *)COM_LoadHunkFile ("gfx/palette.lmp", 0)))
		if (!(host_basepal = Host_LoadPalette ()))
			Sys_Error ("Couldn't load gfx/palette.lmp");
		if (!(host_colormap = Host_LoadColormap ()))
			Sys_Error ("Couldn't load gfx/colormap.lmp");

#ifndef _WIN32 // on non win32, mouse comes before video for security reasons
		IN_Init ();
#endif
		VID_Init (host_basepal);

		Draw_Init ();
		M_Init ();		// JDH: *after* Draw_Init, since it now loads textures
		SCR_Init ();
		R_Init ();

		S_Init ();
		Music_Init();		// JDH
		CDAudio_Init ();

		Sbar_Init ();
		CL_Init ();
#ifdef _WIN32 // on non win32, mouse comes before video for security reasons
		IN_Init ();
#endif
	}
#endif

	Hunk_AllocName (0, "-HOST_HUNKLEV-");
	host_hunklevel = Hunk_LowMark ();

	host_initialized = true;

	Con_Print ("Exe: "__TIME__" "__DATE__"\n");
	Con_Printf ("Hunk allocation: %4.1f MB\n", (float)parms->memsize / 0x100000);

	Con_Printf ("\nreQuiem version %s\n\n", VersionString());

	Con_Print ("\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e reQuiem Initialized \x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n");

// JDH: removed these, as they were getting exec'd twice
	//if (COM_FindFile("config.cfg"))
	//	Cbuf_AddText ("exec config.cfg\n");
	//if (COM_FindFile("autoexec.cfg"))
	//	Cbuf_AddText ("exec autoexec.cfg\n");

	Host_ExecCfgs ();

//	CheckParticles ();
}

/*
===============
Host_Shutdown

FIXME: this is a callback from Sys_Quit and Sys_Error. It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void Host_Shutdown (void)
{
	static qboolean isdown = false;

	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}
	isdown = true;

#ifndef RQM_SV_ONLY
// keep Con_Printf from trying to update the screen
	scr_disabled_for_loading = true;
#endif

	if (nehahra)
	{
		Neh_UninitEnv ();		// restore original cvar values (gl_hwblend)
		nehahra = false;
	}

	Host_WriteConfiguration ();

#ifndef RQM_SV_ONLY
// joe: same here
	if (con_initialized)
	{
		Con_SaveHistory ();
	}
#endif

//	printf ("Host_Shutdown is happening!\n");

	NET_Shutdown ();

#ifndef RQM_SV_ONLY
	CDAudio_Shutdown ();
//	printf ("done CD shutdown!\n");
	S_Shutdown ();
//	printf ("done sound shutdown!\n");
	Music_Shutdown ();
//	printf ("done music shutdown!\n");
	IN_Shutdown ();
#endif

//	printf ("shut down sound & input!\n");

	Con_Shutdown ();
	COM_Shutdown ();

#ifndef RQM_SV_ONLY
	if (cls.state != ca_dedicated)
		VID_Shutdown ();
#endif
}
