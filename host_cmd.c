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
// host_cmd.c

#include <stdarg.h>
#include "quakedef.h"

extern	cvar_t		pausable, host_mapname;
extern	qboolean	allow_postcache;
//extern  cvar_t		sv_protocol;

// these two are not intended to be set directly
cvar_t	cl_name = {"_cl_name", "player", CVAR_FLAG_ARCHIVE};
cvar_t	cl_color = {"_cl_color", "0", CVAR_FLAG_ARCHIVE};

#ifndef RQM_SV_ONLY
cvar_t	cl_confirmquit = {"cl_confirmquit", "1", CVAR_FLAG_ARCHIVE};
#endif

#ifdef HEXEN2_SUPPORT
	extern cvar_t	cl_playerclass;
  #ifndef RQM_SV_ONLY
	extern int		loading_stage;
  #endif

//	unsigned int	info_mask, info_mask2;
	static double	old_time;
#endif

/*
==================
Host_Quit
==================
*/
void Host_Quit (void)
{
//	host_quitting = true;

#ifndef RQM_SV_ONLY
	CL_Disconnect (true);
#endif
	Host_ShutdownServer (false);

	Sys_Quit ();
}

/*
==================
Host_Quit_f
==================
*/
void Host_Quit_f (cmd_source_t src)
{
#ifndef RQM_SV_ONLY
	if (cl_confirmquit.value)
	{
		// JDH: added ca_connected case
		//if ((key_dest != key_console && cls.state != ca_dedicated) || (cls.state == ca_connected))
		if (cls.state != ca_dedicated)
		{
			M_Menu_Quit_f (src);
			return;
		}
	}
#endif

	Host_Quit ();
}

/*
==================
Host_Status_f
==================
*/
void Host_Status_f (cmd_source_t src)
{
	client_t	*client;
	int			seconds, minutes, hours = 0, j;
	qboolean	(*print)(const char *msg);

	if (src != SRC_CLIENT)
	{
#ifndef RQM_SV_ONLY
		if (!sv.active)
		{
			Cmd_ForwardToServer (src);
			return;
		}
#endif
		print = Con_Print;
	}
	else
	{
		print = Host_PrintToClient;
	}

	print (va("host:    %s\n", Cvar_VariableString("hostname")));
	print (va("version: reQuiem %s\n", REQUIEM_VERSION));
	if (tcpipAvailable)
		print (va("tcp/ip:  %s\n", my_tcpip_address));
	if (ipxAvailable)
		print (va("ipx:     %s\n", my_ipx_address));
	print (va("map:     %s\n", sv.name));
	print (va("players: %i active (%i max)\n\n", net_activeconnections, svs.maxclients));
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = (int)(net_time - client->netconnection->connecttime);
		minutes = seconds / 60;
		if (minutes)
		{
			seconds -= (minutes * 60);
			hours = minutes / 60;
			if (hours)
				minutes -= (hours * 60);
		}
		else
		{
			hours = 0;
		}
		print (va("#%-2u %-16.16s  %3i  %2i:%02i:%02i\n", j+1, client->name, 
					(int)client->edict->v.frags, hours, minutes, seconds));
		print (va("   %s\n", client->netconnection->address));
	}
}

/*
==================
Host_God_f

Sets client to godmode
==================
*/
void Host_God_f (cmd_source_t src)
{
	qboolean on;
	
#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (PR_GLOBAL(deathmatch))
		return;

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (PR_GLOBAL(coop) || skill.value > 2))
		return;
#endif

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_GODMODE;
	on = (int)sv_player->v.flags & FL_GODMODE;

	SV_ClientPrintf (host_client, "godmode %s\n", on ? "ON" : "OFF");
}

void Host_Notarget_f (cmd_source_t src)
{
	qboolean on;
	
#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (PR_GLOBAL(deathmatch))
		return;

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (skill.value > 2))
		return;
#endif

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_NOTARGET;
	on = (int)sv_player->v.flags & FL_NOTARGET;
	
	SV_ClientPrintf (host_client, "notarget %s\n", on ? "ON" : "OFF");
}

qboolean noclip_anglehack;

void Host_Noclip_f (cmd_source_t src)
{
#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (PR_GLOBAL(deathmatch))
		return;

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (PR_GLOBAL(coop) || skill.value > 2))
		return;
#endif

	if (sv_player->v.movetype != MOVETYPE_NOCLIP)
	{
		sv_player->v.movetype = MOVETYPE_NOCLIP;
		noclip_anglehack = true;
	}
	else
	{
		sv_player->v.movetype = MOVETYPE_WALK;
		noclip_anglehack = false;
	}

	SV_ClientPrintf (host_client, "noclip %s\n", noclip_anglehack ? "ON" : "OFF");
}

/*
==================
Host_Fly_f

Sets client to flymode
==================
*/
void Host_Fly_f (cmd_source_t src)
{
	qboolean flyon;
	
#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (PR_GLOBAL(deathmatch))
		return;

	if (sv_player->v.movetype != MOVETYPE_FLY)
	{
		sv_player->v.movetype = MOVETYPE_FLY;
		flyon = true;
	}
	else
	{
		sv_player->v.movetype = MOVETYPE_WALK;
		flyon = false;
	}

	SV_ClientPrintf (host_client, "flymode %s\n", flyon ? "ON" : "OFF");
}

/*
==================
Host_Ping_f
==================
*/
void Host_Ping_f (cmd_source_t src)
{
	int		i, j;
	float		total;
	client_t	*client;

#ifndef RQM_SV_ONLY
	const char		*n;	// joe, from ProQuake: for ping +N
	
	if (src != SRC_CLIENT)
	{
		// JPG - check for ping +N
		if (Cmd_Argc() == 2)
		{
			if (cls.state != ca_connected)
				return;

			n = Cmd_Argv (1);
			if (*n == '+')
			{
				Cvar_Set ("pq_lag", n+1);
				return;
			}
		}
//		cl.console_ping = true;		// joe: FIXME

		Cmd_ForwardToServer (src);
		return;
	}
#endif

	SV_ClientPrint (host_client, "Client ping times:\n");
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		total = 0;
		for (j=0 ; j<NUM_PING_TIMES ; j++)
			total += client->ping_times[j];
		total /= NUM_PING_TIMES;
		SV_ClientPrintf (host_client, "%4i %s\n", (int)(total*1000), client->name);
	}
}

/*
===============================================================================

				SERVER TRANSITIONS

===============================================================================
*/

/*
======================
Host_Map_f

handle a
map <servername>
command from the console. Active clients are kicked off.
======================
*/
void Host_Map_f (cmd_source_t src)
{
	int		i, len;
	char	name[MAX_QPATH];

	if (src == SRC_CLIENT)
		return;		// client can't tell server to change maps

#ifndef RQM_SV_ONLY
	if (nehahra && (NehGameType == NEH_TYPE_DEMO))
	{
		M_Menu_Main_f (src);
		return;
	}
#endif

	if (Cmd_Argc() < 2)		// no map name given
	{
		Con_Print ("map <levelname>: start a new server\n");
#ifndef RQM_SV_ONLY
		if (cl.levelname[0])
			Con_Printf ("  Currently on: \"%s\" (%s)\n", cl.levelname, host_mapname.string);
		//Con_Printf ("%s\n", cls.mapstring);
#endif
		return;
	}

#ifndef RQM_SV_ONLY
	cls.demonum = -1;		// stop demo loop in case this fails
	CL_Disconnect (false);
#endif

	Host_ShutdownServer (false);

#ifndef RQM_SV_ONLY
	key_dest = key_game;			// remove console or menu
	SCR_BeginLoadingPlaque (Cmd_Argv(1));

	cls.mapstring[0] = 0;
	len = 0;
	for (i=0 ; i < Cmd_Argc() ; i++)
	{
		len += Q_strcpy (cls.mapstring + len, Cmd_Argv(i), sizeof(cls.mapstring)-len);
		len += Q_strcpy (cls.mapstring + len, " ", sizeof(cls.mapstring)-len);
	}
	Q_strcpy (cls.mapstring + len, "\n", sizeof(cls.mapstring)-len);
#endif

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		sv.info_mask = 0;
		if (!coop.value && deathmatch.value)
			sv.info_mask2 = 0x80000000;
		else
			sv.info_mask2 = 0;
	}
#endif

	svs.serverflags = 0;			// haven't completed an episode yet
	len = Q_strcpy (name, Cmd_Argv(1), sizeof(name));
	if ((len > 4) && COM_FilenamesEqual(name+len-4, ".bsp"))
		name[len-4] = 0;

#ifdef HEXEN2_SUPPORT
	SV_SpawnServer (name, NULL);
#else
	SV_SpawnServer (name);
#endif
	if (!sv.active)
		return;

#ifndef RQM_SV_ONLY
	if (cls.state != ca_dedicated)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			loading_stage = 2;
	#endif

		cls.spawnparms[0] = 0;
		len = 0;

		for (i=2 ; i<Cmd_Argc() ; i++)
		{
			len += Q_strcpy (cls.spawnparms + len, Cmd_Argv(i), sizeof(cls.spawnparms)-len);
			len += Q_strcpy (cls.spawnparms + len , " ", sizeof(cls.spawnparms)-len);
		}

		Cmd_ExecuteString ("connect local", SRC_COMMAND);
	}
#endif
}

/*
==================
Host_Changelevel_f

Goes to a new map, taking all clients along
==================
*/
void Host_Changelevel_f (cmd_source_t src)
{
	char		level[MAX_QPATH];
	qboolean	spawned;
#ifdef HEXEN2_SUPPORT
	char		_startspot[MAX_QPATH];
	char		*startspot;
#endif

	if (Cmd_Argc() != 2)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			if (Cmd_Argc() != 3)
			{
				Con_Print ("changelevel <levelname> [<startspot>]: continue game on a new level in the unit\n");
				return;
			}
		}
		else
	#endif
		{
			Con_Print ("changelevel <levelname> : continue game on a new level\n");
			return;
		}
	}

	if (!sv.active /*|| cls.demoplayback*/)
	{
		Con_Print ("Only the server may changelevel\n");
		return;
	}

	SV_SaveSpawnparms ();
	Q_strcpy (level, Cmd_Argv(1), sizeof(level));

#ifdef HEXEN2_SUPPORT
	if (!hexen2 || (Cmd_Argc() == 2))
		startspot = NULL;
	else
	{
		Q_strcpy (_startspot, Cmd_Argv(2), sizeof(_startspot));
		startspot = _startspot;
	}
	spawned = SV_SpawnServer (level, startspot);
#else
	spawned = SV_SpawnServer (level);
#endif

#ifndef RQM_SV_ONLY
	if (!spawned)
		CL_Disconnect_f (SRC_COMMAND);
#endif
}

#ifdef HEXEN2_SUPPORT
/*
==================
Host_RestoreClients
==================
*/
void Host_RestoreClients (void)
{
	int i,j;
	edict_t	*ent;
	double time_diff;

	if (!Host_LoadGamestate (NULL, NULL, 1))
		return;

	time_diff = sv.time - old_time;

	for (i=0,host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (host_client->active)
		{
			ent = host_client->edict;

			ent->v.team = (host_client->colors & 15) + 1;
			ent->v.netname = host_client->name - pr_strings;
			ent->v.playerclass = host_client->playerclass;

			// copy spawn parms out of the client_t

			for (j=0 ; j< NUM_SPAWN_PARMS ; j++)
				(pr_global_ptrs.parm1)[j] = host_client->spawn_parms[j];

			// call the spawn function

			*pr_global_ptrs.time = sv.time;
			*pr_global_ptrs.self = EDICT_TO_PROG(ent);
			G_FLOAT(OFS_PARM0) = time_diff;
			PR_ExecuteProgram (*pr_global_ptrs.ClientReEnter);
		}
	}

	Host_SaveGamestate(true);
}

/*
==================
Host_Changelevel2_f
  - changing levels within a unit
==================
*/
void Host_Changelevel2_f (cmd_source_t src)
{
	char	level[MAX_QPATH];
	char	_startspot[MAX_QPATH];
	char	*startspot;

	if (Cmd_Argc() < 3)
	{
		Con_Print ("changelevel2 <levelname> <startspot>: continue game on a new level in the unit\n");
		return;
	}
	if (!sv.active /*|| cls.demoplayback*/)
	{
		Con_Print ("Only the server may changelevel2\n");
		return;
	}

	Q_strcpy (level, Cmd_Argv(1), sizeof(level));
	if (Cmd_Argc() == 2)
		startspot = NULL;
	else
	{
		Q_strcpy (_startspot, Cmd_Argv(2), sizeof(_startspot));
		startspot = _startspot;
	}

	SV_SaveSpawnparms ();

	// save the current level's state
	old_time = sv.time;
	Host_SaveGamestate (false);

	// try to restore the new level
	if (!Host_LoadGamestate (level, startspot, 0))
	{
		SV_SpawnServer (level, startspot);
		Host_RestoreClients ();
	}
}
#endif
/*
==================
Host_Restart_f

Restarts the current server for a dead player
==================
*/
void Host_Restart_f (cmd_source_t src)
{
	char	mapname[MAX_QPATH];
#ifdef HEXEN2_SUPPORT
	char	startspot[MAX_QPATH];
#endif

	if (!sv.active /*||cls.demoplayback*/)
		return;

	if (src == SRC_CLIENT)
		return;
	Q_strcpy (mapname, sv.name, sizeof(mapname));	// must copy out, because it gets cleared in sv_spawnserver

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if ((Cmd_Argc() == 2) && !Q_strcasecmp(Cmd_Argv(1),"restore"))
		{
			if (!Host_LoadGamestate (mapname, startspot, 3))
			{
				SV_SpawnServer (mapname, startspot);
				Host_RestoreClients ();
			}
		}
		else
		{
			SV_SpawnServer (mapname, startspot);
		}
	}
	else SV_SpawnServer (mapname, NULL);
#else
	SV_SpawnServer (mapname);
#endif
}

#ifndef RQM_SV_ONLY
/*
==================
Host_Reconnect_f

This command causes the client to wait for the signon messages again.
This is sent just before a server changes levels
==================
*/
void Host_Reconnect_f (cmd_source_t src)
{
	const char *mapname;
	extern	float	scr_centertime_off;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		R_ClearParticles ();	//jfm: for restarts which didn't use to clear parts.
		if (/*oem.value &&*/ cl.intermission == 9)
		{
			CL_Disconnect (false);
			return;
		}
	}
#endif

	scr_centertime_off = 0;
	mapname = (Cmd_Argc() > 1 ? Cmd_Argv(1) : NULL);		// JDH
	SCR_BeginLoadingPlaque (mapname);
	cls.signon = 0;		// need new connection messages
}

extern char server_name[MAX_QPATH];

/*
=====================
Host_Connect_f

User command to connect to server
=====================
*/
void Host_Connect_f (cmd_source_t src)
{
	char	name[MAX_QPATH];
	extern  cmd_arglist_t cmd_arglist;

	cls.demonum = -1;		// stop demo loop in case this fails
	if (cls.demoplayback)
	{
		CL_StopPlayback ();
		CL_Disconnect (false);
	}
	Q_strcpy (name, Cmd_Argv(1), sizeof(name));
	CL_EstablishConnection (name);
	cmd_arglist.argc = 1;		// JDH: don't want Reconnect mistaking server name for map name
	Host_Reconnect_f (src);

	Q_strcpy (server_name, name, sizeof(server_name));	// added from ProQuake
}

#endif		//#ifndef RQM_SV_ONLY

//============================================================================

/*
======================
Host_Name_f
======================
*/
void Host_Name_f (cmd_source_t src)
{
//	char	*newName;
	const char	*arg;
	char	newName[16];
#ifdef HEXEN2_SUPPORT
	char	*pdest;
#endif

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"name\" is \"%s\"\n", cl_name.string);
		return;
	}

	arg = (Cmd_Argc() == 2) ? Cmd_Argv (1) : Cmd_Args (1);
	Q_strcpy (newName, arg, sizeof(newName));
	newName[15] = 0;


#ifdef HEXEN2_SUPPORT
	//this is for the fuckers who put braces in the name causing loadgame to crash.
	if (hexen2)
	{
		pdest = strchr(newName,'{');
		if (pdest)
		{
			*pdest = 0;	//zap the brace
			Con_Print ("Illegal char in name removed!\n");
		}
	}
#endif

#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		if (!strcmp(cl_name.string, newName))
			return;
		Cvar_SetDirect (&cl_name, newName);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (host_client->name[0] && strcmp(host_client->name, "unconnected"))
		if (strcmp(host_client->name, newName))
			Con_Printf ("%s renamed to %s\n", host_client->name, newName);

	Q_strcpy (host_client->name, newName, sizeof(host_client->name));
	host_client->edict->v.netname = host_client->name - pr_strings;

// send notification to all clients

	MSG_WriteCmd (&sv.reliable_datagram, svc_updatename);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteString (&sv.reliable_datagram, host_client->name);
}

#ifdef HEXEN2_SUPPORT

extern const char *ClassNames[NUM_CLASSES];
/*
======================
Host_Class_f
======================
*/
void Host_Class_f (cmd_source_t src)
{
	float	newClass;

	if (Cmd_Argc () == 1)
	{
		if (!(int)cl_playerclass.value)
			Con_Printf ("\"playerclass\" is %d (\"unknown\")\n", (int)cl_playerclass.value);
		else
			Con_Printf ("\"playerclass\" is %d (\"%s\")\n", (int)cl_playerclass.value, ClassNames[(int)cl_playerclass.value-1]);
		return;
	}

//	if (Cmd_Argc () == 2)
		newClass = Q_atof(Cmd_Argv(1));
//	else
//		newClass = Q_atof(Cmd_Args(1));

//	if (!registered.value && !oem.value && (newClass == CLASS_CLERIC || newClass == CLASS_NECROMANCER))
//	{
//		Con_Print("That class is not available in the demo version.\n");
//		return;
//	}

#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cvar_SetValueDirect (&cl_playerclass, newClass);

		// when classes changes after map load, update cl_playerclass, cl_playerclass should
		// probably only be used in worldspawn, though
		if (pr_global_ptrs.cl_playerclass)
			*pr_global_ptrs.cl_playerclass = newClass;

		if (cls.state == ca_connected)
			Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (sv.loadgame || host_client->playerclass)
	{
		if (host_client->edict->v.playerclass)
			newClass = host_client->edict->v.playerclass;
		else if (host_client->playerclass)
			newClass = host_client->playerclass;
	}

	host_client->playerclass = newClass;
	host_client->edict->v.playerclass = newClass;

	// Change the weapon model used
	*pr_global_ptrs.self = EDICT_TO_PROG(host_client->edict);
	PR_ExecuteProgram (*pr_global_ptrs.ClassChangeWeapon);

// send notification to all clients

	MSG_WriteCmd (&sv.reliable_datagram, svc_updateclass);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, (byte)newClass);
}
#endif	// #ifdef HEXEN2_SUPPORT


/*
=========
Host_Say
=========
*/
void Host_Say (cmd_source_t src, qboolean teamonly)
{
	client_t	*client;
	int			j, len;
	char		*p;
    char		text[64], args[64];
	qboolean	fromServer = false;

#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		if (cls.state == ca_dedicated)
		{
			fromServer = true;
			teamonly = false;
		}
		else
		{
			Cmd_ForwardToServer (src);
			return;
		}
	}
#else
	fromServer = true;
	teamonly = false;
#endif

	if (Cmd_Argc () < 2)
		return;

//	p = Cmd_Args ();
	len = Q_strcpy (args, Cmd_Args (1), sizeof(args));
	p = args;
// remove quotes if present
	if (*p == '"')
	{
		p++;
//		p[strlen(p)-1] = 0;
		if (p[len-2] == '"')
			p[len-2] = 0;
	}

// turn on color set 1
	if (!fromServer)
		len = Q_snprintfz (text, sizeof(text), "\x01""%s: ", host_client->name);
	else
		len = Q_snprintfz (text, sizeof(text), "\x01""<%s> ", hostname.string);

	j = sizeof(text) - 2 - len;  // -2 for \n and null terminator
	if (strlen(p) > j)
		p[j] = 0;

	len += Q_strcpy (text+len, p, sizeof(text)-len);
	Q_strcpy (text+len, "\n", sizeof(text)-len);

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client || !client->active || !client->spawned)
			continue;
		if (teamplay.value && teamonly && client->edict->v.team != host_client->edict->v.team)
			continue;
		SV_ClientPrint (client, text);
	}

	Sys_Printf ("%s", &text[1]);
}


void Host_Say_f (cmd_source_t src)
{
	Host_Say (src, false);
}


void Host_Say_Team_f (cmd_source_t src)
{
	Host_Say (src, true);
}


void Host_Tell_f (cmd_source_t src)
{
	client_t	*client;
	int			j, len;
	char		text[64];
	const char	*p, *name;

#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (Cmd_Argc () < 3)
		return;

	p = Cmd_Args (2);
	j = strlen (p);

// remove quotes if present
	if (*p == '"')
	{
		p++;
		j--;
		if (p[j-1] == '"')
			j--;
	}

//	len = Q_strcpy (text, host_client->name, sizeof(text));
//	len += Q_strcpy (text+len, ": ", sizeof(text)-len);
	name = Cmd_Argv (1);
	len = Q_snprintfz (text, sizeof(text), "%s [to %s]: ", host_client->name, name);
	len += Q_strncpy (text+len, sizeof(text)-len, p, j);

// check length & truncate if necessary
	j = sizeof(text) - 2;  // -2 for \n and null terminator
	if (len > j)
	{
		text[j] = 0;
		len = j;
	}

	Q_strcpy (text+len, "\n", sizeof(text)-len);

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active || !client->spawned)
			continue;
		if (Q_strcasecmp(client->name, name))
			continue;
		SV_ClientPrint (client, text);
		break;
	}
}


/*
==================
Host_Color_f
==================
*/
void Host_Color_f (cmd_source_t src)
{
	int		top, bottom, playercolor;

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("player color is currently \"%i %i\"  (%s = %d)\n", ((int)cl_color.value) >> 4, 
						((int)cl_color.value) & 0x0f, cl_color.name, (int)cl_color.value);
//		Con_Printf ("color <0-13> [0-13]\n");
		Con_Printf ("usage: color <top> [bottom]\n  (values must be 0-13)\n");

		return;
	}

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else
	{
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}

	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;

	playercolor = top*16 + bottom;

#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cvar_SetValueDirect (&cl_color, playercolor);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer (src);
		return;
	}
#endif

	host_client->colors = playercolor;
	host_client->edict->v.team = bottom + 1;

// send notification to all clients
	MSG_WriteCmd (&sv.reliable_datagram, svc_updatecolors);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, host_client->colors);
}

/*
==================
Host_Kill_f
==================
*/
void Host_Kill_f (cmd_source_t src)
{
#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (sv_player->v.health <= 0)
	{
		SV_ClientPrint (host_client, "Can't suicide --- already dead!\n");
		return;
	}

	PR_GLOBAL(time) = sv.time;
	PR_GLOBAL(self) = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (PR_GLOBAL(ClientKill));
}


/*
==================
Host_Pause_f
==================
*/
void Host_Pause_f (cmd_source_t src)
{
#ifndef RQM_SV_ONLY
	cl.paused ^= 2;		// by joe: to handle demo-pause

	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (!pausable.value)
	{
		SV_ClientPrint (host_client, "Pause not allowed.\n");
	}
	else
	{
		sv.paused ^= 1;

		if (sv.paused)
			SV_BroadcastPrintf ("%s paused the game\n", pr_strings + sv_player->v.netname);
		else
			SV_BroadcastPrintf ("%s unpaused the game\n",pr_strings + sv_player->v.netname);

	// send notification to all clients
		MSG_WriteCmd (&sv.reliable_datagram, svc_setpause);
		MSG_WriteByte (&sv.reliable_datagram, sv.paused);
	}
}

//===========================================================================

/*
==================
Host_PreSpawn_f
==================
*/
void Host_PreSpawn_f (cmd_source_t src)
{
	if (src != SRC_CLIENT)
	{
		Con_Print ("prespawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Print ("prespawn not valid -- already spawned\n");
		return;
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	{
//		if (host_client->protocol == PROTOCOL_VERSION_BJP3)
		if (sv.protocol == PROTOCOL_VERSION_BJP3)
		{
		// JDH: unless desired_protocol is set to PROTOCOL_VERSION_BJP3, sv.signon message
		//      initially adheres to old protocol, so we need to temporarily downgrade
		 	if (sv.desired_protocol != PROTOCOL_VERSION_BJP3)
			{
				MSG_WriteCmd (&host_client->message, svc_version);
				MSG_WriteLong (&host_client->message, PROTOCOL_VERSION_STD);
			}
		}
		else if ((host_client->message.maxsize - host_client->message.cursize < sv.signon.cursize+2) &&
//					(sv_protocol.value != PROTOCOL_VERSION_STD)/*!sv_oldprotocol.value*/)
					(sv.desired_protocol != PROTOCOL_VERSION_STD))
		{
//			host_client->protocol = PROTOCOL_VERSION_BJP3;
			host_client->message.maxsize = sizeof(host_client->msgbuf);
		}
	}

	SZ_Write (&host_client->message, sv.signon.data, sv.signon.cursize);
	MSG_WriteCmd (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 2);
	host_client->sendsignon = true;


// go back to extended protocol, if necessary
// (and if sv.signon message didn't already change it)

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
//	if ((host_client->protocol == PROTOCOL_VERSION_BJP3) && (sv.serverinfo_protocol != PROTOCOL_VERSION_BJP3))
	if ((sv.protocol == PROTOCOL_VERSION_BJP3) && (sv.desired_protocol != PROTOCOL_VERSION_BJP3))
	{
		MSG_WriteCmd (&host_client->message, svc_version);
		MSG_WriteLong (&host_client->message, PROTOCOL_VERSION_BJP3);
	}
}

/*
==================
Host_Spawn_f
==================
*/
void Host_Spawn_f (cmd_source_t src)
{
	int			i;
	client_t	*client;
	edict_t		*ent;
	func_t		RestoreGame;
	dfunction_t	*f;

	if (src != SRC_CLIENT)
	{
		Con_Print ("spawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Print ("Spawn not valid -- already spawned\n");
		return;
	}

	if (pq_cheatfree)
	{
		unsigned	pcrc, ecrc;

		pcrc = MSG_ReadLong ();
		ecrc = MSG_ReadLong ();

		if (sv.player_model_crc != pcrc || sv.eyes_model_crc != ecrc)
			SV_BroadcastPrintf ("%s (%s) WARNING: non standard player/eyes model detected\n", host_client->name, host_client->netconnection->address);
	}

// run the entrance script
	if (sv.loadgame)
	{	// loaded games are fully inited allready
		// if this is the last client to be connected, unpause
		sv.paused = false;

		// nehahra stuff
		if ((f = PR_FindFunction("RestoreGame", PRFF_NOBUILTINS)))
		{
			if ((RestoreGame = (func_t)(f - pr_functions)))
			{
				Con_DPrintf ("Calling RestoreGame\n");
				PR_GLOBAL(time) = sv.time;
				PR_GLOBAL(self) = EDICT_TO_PROG(sv_player);
				PR_ExecuteProgram (RestoreGame);
			}
		}
	}
	else
	{
		// set up the edict
		ent = host_client->edict;

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			sv.paused = false;

		if (!hexen2 || !ent->v.stats_restored || deathmatch.value)
	#endif
		{
			memset (&ent->v, 0, progs->entityfields * 4);
			ent->v.colormap = NUM_FOR_EDICT(ent);
			ent->v.team = (host_client->colors & 15) + 1;
			ent->v.netname = host_client->name - pr_strings;

			// copy spawn parms out of the client_t
		/*****JDH*****/
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				ent->v.playerclass = host_client->playerclass;
			}
		#endif

			for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
				(pr_global_ptrs.parm1)[i] = host_client->spawn_parms[i];

			// call the spawn function
			PR_GLOBAL(time) = sv.time;
			PR_GLOBAL(self) = EDICT_TO_PROG(sv_player);
			PR_ExecuteProgram (PR_GLOBAL(ClientConnect));

			if ((Sys_DoubleTime() - host_client->netconnection->connecttime) <= sv.time)
				Sys_Printf ("%s entered the game\n", host_client->name);

			PR_ExecuteProgram (PR_GLOBAL(PutClientInServer));
		}
	/*****JDH*****/
	}


// send all current names, colors, and frag counts
	SZ_Clear (&host_client->message);

// send time of update
	MSG_WriteCmd (&host_client->message, svc_time);
	MSG_WriteFloat (&host_client->message, sv.time);

	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		MSG_WriteCmd (&host_client->message, svc_updatename);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteString (&host_client->message, client->name);

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			MSG_WriteCmd (&host_client->message, svc_updateclass);
			MSG_WriteByte (&host_client->message, i);
			MSG_WriteByte (&host_client->message, client->playerclass);
		}
	#endif

		MSG_WriteCmd (&host_client->message, svc_updatefrags);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteShort (&host_client->message, client->old_frags);

		MSG_WriteCmd (&host_client->message, svc_updatecolors);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteByte (&host_client->message, client->colors);
	}

// send all current light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		MSG_WriteCmd (&host_client->message, svc_lightstyle);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteString (&host_client->message, sv.lightstyles[i]);
	}

// send some stats
	MSG_WriteCmd (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALSECRETS);
	MSG_WriteLong (&host_client->message, PR_GLOBAL(total_secrets));

	MSG_WriteCmd (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->message, PR_GLOBAL(total_monsters));

	MSG_WriteCmd (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_SECRETS);
	MSG_WriteLong (&host_client->message, PR_GLOBAL(found_secrets));

	MSG_WriteCmd (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_MONSTERS);
	MSG_WriteLong (&host_client->message, PR_GLOBAL(killed_monsters));

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		SV_UpdateEffects (&host_client->message);
#endif

// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM(1 + (host_client - svs.clients));
/****JDH****/
/*	if (sv_currentprotocol == PROTOCOL_VERSION_PLUS)
	{
		MSG_WriteCmd (&host_client->message, svc_setpreciseangle);
		for (i=0 ; i < 2 ; i++)
			MSG_WritePreciseAngle (&host_client->message, ent->v.angles[i]);
		MSG_WritePreciseAngle (&host_client->message, 0);
	}
	else*/
/****JDH****/
	{
		MSG_WriteCmd (&host_client->message, svc_setangle);
		for (i=0 ; i < 2 ; i++)
			MSG_WriteAngle (&host_client->message, ent->v.angles[i]);
		MSG_WriteAngle (&host_client->message, 0);

	//JDH:
		host_client->prev_viewangles[0] = ent->v.angles[0];
		host_client->prev_viewangles[1] = ent->v.angles[1];
		host_client->prev_viewangles[2] = 0;
	}

	SV_WriteClientdataToMessage (host_client, sv_player, &host_client->message);

	MSG_WriteCmd (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 3);
	host_client->sendsignon = true;
}

#ifndef RQM_SV_ONLY

/*
==================
Host_Create_f
- allows user to spawn an arbitrary entity (from Hexen II)
==================
*/
void Host_Create_f (cmd_source_t src)
{
	const char	*name;
	dfunction_t	*func;
	edict_t		*ent;
	int			i, j, sv_modelcount, cl_modelcount, sv_soundcount, cl_soundcount;
	ddef_t		*def;
	float		framecount;
//	dfunctions_t *Search;
//	int			Length,NumFound,Diff,NewDiff;
//	char		temp[256];
	qboolean	hasflame;
	model_t		*mod;
	msurface_t	*surf;

	if (!sv.active)
	{
		Con_Print ("Error: 'create' command requires local server to be active.\n");
		return;
	}

	if ((svs.maxclients != 1) || (skill.value > 2))
	{
		Con_Print ("Error: can't use 'create' command in multiplayer mode!\n");
		return;
	}

	if (Cmd_Argc () == 1)
	{
		Con_Print ("create <classname>\n");
		return;
	}

	name = Cmd_Argv (1);

	func = PR_FindFunction (name, PRFF_IGNORECASE | PRFF_NOBUILTINS | PRFF_NOPARAMS);

	if (!func)
	{
		Con_Printf ("Could not find spawn function for \"%s\"\n", name);
		return;
	}

/*	if (!func)
	{
		Length = strlen(FindName);
		NumFound = 0;

		Diff = 999;

		for (i=0 ; i<progs->numfunctions ; i++)
		{
			Search = &pr_functions[i];
			if (!Q_strncasecmp(pr_strings + Search->s_name,FindName,Length) )
			{
				if (NumFound == 1)
				{
					Con_Printf("   %s\n",pr_strings+func->s_name);
				}
				if (NumFound)
				{
					Con_Printf("   %s\n",pr_strings+Search->s_name);
					NewDiff = strdiff(pr_strings+Search->s_name,pr_strings+func->s_name);
					if (NewDiff < Diff) Diff = NewDiff;
				}

				func = Search;
				NumFound++;
			}
		}

		if (!NumFound)
		{
			Con_Print("Could not find spawn function\n");
			return;
		}

		if (NumFound != 1)
		{
			sprintf(key_lines[edit_line],">create %s",func->s_name+pr_strings);
			key_lines[edit_line][Diff+8] = 0;
			key_linepos = strlen(key_lines[edit_line]);
			return;
		}
	}
*/
	Con_Printf ("Executing %s...\n", /*pr_strings +*/ func->s_name);

	ent = ED_Alloc ();

	ent->v.classname = func->s_name - pr_strings;
	VectorCopy (r_origin, ent->v.origin);
	ent->v.origin[0] += vpn[0] * 80;
	ent->v.origin[1] += vpn[1] * 80;
	ent->v.origin[2] += vpn[2] * 80;
	VectorCopy (ent->v.origin, ent->v.absmin);
	VectorCopy (ent->v.origin, ent->v.absmax);
	VectorOffset (ent->v.absmin, -16);
	VectorOffset (ent->v.absmax, 16);

	for (i=1 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
			break;
	}
	sv_modelcount = i;

	for (i=1 ; i<MAX_MODELS ; i++)
	{
		if (!cl.model_precache[i])
			break;
	}
	cl_modelcount = i;

// UGLY HACK (to deal with ugly hack in CL_ParseServerInfo - loading of alt flame model)
	if ((i > 0) && COM_FilenamesEqual (cl.model_precache[i-1]->name, cl_modelnames[mi_flame0]))
	{
		cl_modelcount--;
		hasflame = true;
	}
	else hasflame = false;

	for (i=1 ; i<MAX_SOUNDS ; i++)
	{
		if (!sv.sound_precache[i])
			break;
	}
	sv_soundcount = i;

	for (i=1 ; i<MAX_SOUNDS ; i++)
	{
		if (!cl.sound_precache[i])
			break;
	}
	cl_soundcount = i;

// Quoth2 blocks precache if framecount != 0
	def = PR_FindGlobal ("framecount");
	if (def)
	{
		framecount = G_FLOAT(def->ofs);
		G_FLOAT(def->ofs) = 0;
	}
	else framecount = 0;		// shut up compiler warning

	PR_GLOBAL(self) = EDICT_TO_PROG(ent);
	
	allow_postcache = true;
	PR_ExecuteProgram (func - pr_functions);
	allow_postcache = false;

	if (def)
		G_FLOAT(def->ofs) = framecount;

	// JDH: some entities aren't fully spawned until their think func is called:
/*	ent->v.nextthink = 0;
	PR_GLOBAL(other) = EDICT_TO_PROG(sv.edicts);
	PR_ExecuteProgram (ent->v.think);
*/
	// JDH: if any models were loaded during the spawn function,
	//  load them on the client-side as well:
	for (i=sv_modelcount ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
			break;

		for (j=0 ; j<NUM_MODELINDEX ; j++)
		{
			if (!strcmp(cl_modelnames[j], sv.model_precache[i]))
			{
				cl_modelindex[j] = i;
				break;
			}
		}

		mod = Mod_ForName (sv.model_precache[i], false);
		if (mod)
		{
			if (hasflame && (cl_modelcount+1 < MAX_MODELS))
			{
				cl.model_precache[cl_modelcount+1] = cl.model_precache[cl_modelcount];
				cl_modelindex[mi_flame0] = cl_modelcount+1;
			}

			cl.model_precache[cl_modelcount] = mod;
			cl_modelcount++;

		// this usually happens in GL_BuildLightmaps when map is loaded:
			if (mod->type == mod_brush)
			{
				surf = mod->surfaces;
				for (j=0; j < mod->numsurfaces; j++, surf++)
				{
					if (surf->flags & (SURF_DRAWTURB | SURF_DRAWSKY))
						continue;

					GL_CreateSurfaceLightmap (mod, surf);
					GL_BuildSurfaceDisplayList (mod, surf);
					GL_ForceLightMapReload (surf);
				}
			}
		}
	}

	for (i=sv_soundcount ; i<MAX_SOUNDS ; i++)
	{
		name = sv.sound_precache[i];
		if (!name)
			break;

		S_TouchSound (name);
		cl.sound_precache[cl_soundcount] = S_PrecacheSound (name);
		cl_soundcount++;
	}

	cl.stats[STAT_TOTALMONSTERS] = (int) PR_GLOBAL(total_monsters);
}

#endif		//#ifndef RQM_SV_ONLY

/*
==================
Host_Begin_f
==================
*/
void Host_Begin_f (cmd_source_t src)
{
	if (src != SRC_CLIENT)
	{
		Con_Print ("begin is not valid from the console\n");
		return;
	}

	host_client->spawned = true;
}

//===========================================================================


/*
==================
Host_Kick_f

Kicks a user off of the server
==================
*/
void Host_Kick_f (cmd_source_t src)
{
	char		*who;
	const char	*message = NULL;
	client_t	*save;
	int			i;
	qboolean	byNumber = false;

#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer (src);
			return;
		}
	}
	else
#endif
	if (PR_GLOBAL(deathmatch))
	{
		return;
	}

	save = host_client;

	if (Cmd_Argc() > 2 && !strcmp(Cmd_Argv(1), "#"))
	{
		i = Q_atof(Cmd_Argv(2)) - 1;
		if (i < 0 || i >= svs.maxclients)
			return;
		if (!svs.clients[i].active)
			return;
		host_client = &svs.clients[i];
		byNumber = true;
	}
	else
	{
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if (!host_client->active)
				continue;
			if (Q_strcasecmp(host_client->name, Cmd_Argv(1)) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (src != SRC_CLIENT)
		{
		#ifndef RQM_SV_ONLY
			if (cls.state == ca_dedicated)
				who = "Console";
			else
				who = cl_name.string;
		#else
				who = "Console";
		#endif
		}
		else
		{
			who = save->name;
		}

		// can't kick yourself!
		if (host_client == save)
			return;

		if (Cmd_Argc() > 2)
		{
			message = COM_Parse (Cmd_Args(1));
			if (byNumber)
			{
				message++;							// skip the #
				while (*message == ' ')				// skip white space
					message++;
				message += strlen (Cmd_Argv(2));	// skip the number
			}
			while (*message && *message == ' ')
				message++;
		}
		if (message)
			SV_ClientPrintf (host_client, "Kicked by %s: %s\n", who, message);
		else
			SV_ClientPrintf (host_client, "Kicked by %s\n", who);
		SV_DropClient (false);
	}

	host_client = save;
}

/*
===============================================================================

				DEBUGGING TOOLS

===============================================================================
*/

int PR_FindString (const char *str)
{
	int i;
	char *s;

	for (i = 1; i < progs->numstrings; i++)
	{
		s = pr_strings+i;
		if (!strcmp(s, str))
			return i;
		i += strlen(s);
	}

	return 0;
}

/*
==================
Host_ResurrectPlayer (JDH)
  extension of the "give health" cheat that works on a dead player
==================
*/
void Host_ResurrectPlayer (void)
{
// Note: this is all based on assumptions about how the code works in standard progs.dat 
//     (esp. PlayerDie, PutClientInServer, and Killed)
	dfunction_t	*func;

	sv_player->v.view_ofs[2] = DEFAULT_VIEWHEIGHT;
	sv_player->v.deadflag = DEAD_NO;
	sv_player->v.solid = SOLID_SLIDEBOX;
	sv_player->v.nextthink = sv.time;
	sv_player->v.takedamage = DAMAGE_AIM;

	ED_SetFieldValue (sv_player, "invisible_finished", 0);
	ED_SetFieldValue (sv_player, "invincible_finished", 0);
	ED_SetFieldValue (sv_player, "super_damage_finished", 0);
	ED_SetFieldValue (sv_player, "radsuit_finished", 0);
	
	sv_player->v.items = (int)sv_player->v.items & ~(IT_INVISIBILITY | IT_INVULNERABILITY | IT_QUAD | IT_SUIT);
	sv_player->v.effects = (int)sv_player->v.effects & ~EF_DIMLIGHT;

// if player was gibbed, model is currently h_player, so change it back:
	if (sv_player->v.movetype == MOVETYPE_BOUNCE)
	{
		sv_player->v.origin[2] += 24;
		sv_player->v.modelindex = sv_player->baseline.modelindex;		// original player model
		sv_player->v.model = PR_FindString (sv.model_precache[sv_player->baseline.modelindex]);
		SetMinMaxSize (sv_player, vec3_hull_min, vec3_hull_max, false);
	}

	sv_player->v.movetype = MOVETYPE_WALK;
	func = PR_FindFunction ("W_SetCurrentAmmo", PRFF_NOBUILTINS);
	if (func)
	{
		PR_GLOBAL(time) = sv.time;
		PR_GLOBAL(self) = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (func - pr_functions);		// restore weaponmodel
	}

	// TODO: angles??
}

/*
==================
Host_Give_f
==================
*/
void Host_Give_f (cmd_source_t src)
{
	const char	*t;
	int		v;
	eval_t	*val;
	int		flag;
	float	type;

#ifndef RQM_SV_ONLY
	if (src != SRC_CLIENT)
	{
		Cmd_ForwardToServer (src);
		return;
	}
#endif

	if (PR_GLOBAL(deathmatch))
		return;

#ifdef HEXEN2_SUPPORT
//	if (hexen2 && (skill.value > 2))
//		return;
#endif

	t = Cmd_Argv(1);
	v = atoi (Cmd_Argv(2));

	switch (t[0])
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		// MED 01/04/97 added hipnotic give stuff
		if (hipnotic)
		{
			if (t[0] == '6')
			{
				if (t[1] == 'a')
					sv_player->v.items = (int)sv_player->v.items | HIT_PROXIMITY_GUN;
				else
					sv_player->v.items = (int)sv_player->v.items | IT_GRENADE_LAUNCHER;
			}
			else if (t[0] == '9')
				sv_player->v.items = (int)sv_player->v.items | HIT_LASER_CANNON;
			else if (t[0] == '0')
				sv_player->v.items = (int)sv_player->v.items | HIT_MJOLNIR;
			else if (t[0] >= '2')
				sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
		}
		else if (rogue)			// JDH: added codes for Rogue weapons (4a/5a/6a/7a/8a)
		{
			if (t[0] >= '2')
			{
				if ((t[0] >= '4') && (t[0] <= '8') && (t[1] == 'a'))
					sv_player->v.items = (int)sv_player->v.items | (RIT_LAVA_NAILGUN << (t[0] - '4'));
				else
					sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
			}
		}
		else
		{
			if (t[0] >= '2')
				sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
		}
		break;

	case 's':
		if (rogue)
		{
		    if ((val = GETEDICTFIELD(sv_player, eval_ammo_shells1)))
			    val->_float = v;
		}
		sv_player->v.ammo_shells = v;
		if ((sv_player->v.weapon == IT_SHOTGUN) || (sv_player->v.weapon == IT_SUPER_SHOTGUN))
			sv_player->v.currentammo = v;
		break;

	case 'n':
		if (rogue)
		{
			if ((val = GETEDICTFIELD(sv_player, eval_ammo_nails1)))
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
		else
		{
			sv_player->v.ammo_nails = v;
		}
		if ((sv_player->v.weapon == IT_NAILGUN) || (sv_player->v.weapon == IT_SUPER_NAILGUN))
			sv_player->v.currentammo = v;
		break;

	case 'l':
		if (rogue)
		{
			if ((val = GETEDICTFIELD(sv_player, eval_ammo_lava_nails)))
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
				{
					sv_player->v.ammo_nails = v;
					if ((sv_player->v.weapon == RIT_LAVA_NAILGUN) || (sv_player->v.weapon == RIT_LAVA_SUPER_NAILGUN))
						sv_player->v.currentammo = v;
				}
			}
		}
		break;

	case 'r':
		if (rogue)
		{
			if ((val = GETEDICTFIELD(sv_player, eval_ammo_rockets1)))
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
		else
		{
			sv_player->v.ammo_rockets = v;
		}
		if ((sv_player->v.weapon == IT_GRENADE_LAUNCHER) || (sv_player->v.weapon == IT_ROCKET_LAUNCHER))
			sv_player->v.currentammo = v;
		break;

	case 'm':
		if (rogue)
		{
			if ((val = GETEDICTFIELD(sv_player, eval_ammo_multi_rockets)))
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
				{
					sv_player->v.ammo_rockets = v;
					if ((sv_player->v.weapon == RIT_MULTI_GRENADE) || (sv_player->v.weapon == RIT_MULTI_ROCKET))
						sv_player->v.currentammo = v;
				}
			}
		}
		break;

	case 'h':
		if (sv_player->v.health <= 0)
		{
			Host_ResurrectPlayer ();
		}
		sv_player->v.health = v;
		break;

	case 'a':		// JDH: armor
		if (v >= 200)
		{
			v = 200;
			flag = IT_ARMOR3;
			type = 0.8;
		}
		else if (v >= 150)
		{
			flag = IT_ARMOR2;
			type = 0.6;
		}
		else
		{
			flag = IT_ARMOR1;
			type = 0.3;
		}
		sv_player->v.items = (int) sv_player->v.items | flag;
		sv_player->v.armorvalue = v;
		sv_player->v.armortype = type;
		break;

	case 'c':
		if (rogue)
		{
			if ((val = GETEDICTFIELD(sv_player, eval_ammo_cells1)))
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
		else
		{
			sv_player->v.ammo_cells = v;
		}
		if (sv_player->v.weapon == IT_LIGHTNING)
			sv_player->v.currentammo = v;
		break;

	case 'p':
		if (rogue)
		{
			if ((val = GETEDICTFIELD(sv_player, eval_ammo_plasma)))
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
				{
					sv_player->v.ammo_cells = v;
					if (sv_player->v.weapon == RIT_PLASMA_GUN)
						sv_player->v.currentammo = v;
				}
			}
		}
		break;
    }
}

#ifndef RQM_SV_ONLY

edict_t	*FindViewthing (void)
{
	int		i;
	edict_t	*e;

	for (i=0 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		if (!strcmp (pr_strings + e->v.classname, "viewthing"))
			return e;
	}
	Con_Print ("No viewthing on map\n");
	return NULL;
}

/*
==================
Host_Viewmodel_f
==================
*/
void Host_Viewmodel_f (cmd_source_t src)
{
	edict_t	*e;
	model_t	*m;

	if (!(e = FindViewthing ()))
		return;

	if (!(m = Mod_ForName(Cmd_Argv(1), false)))
	{
		Con_Printf ("Can't load %s\n", Cmd_Argv(1));
		return;
	}

	e->v.frame = 0;
	cl.model_precache[(int)e->v.modelindex] = m;
}

/*
==================
Host_Viewframe_f
==================
*/
void Host_Viewframe_f (cmd_source_t src)
{
	edict_t	*e;
	int		f;
	model_t	*m;

	if (!(e = FindViewthing ()))
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	f = atoi(Cmd_Argv(1));
	if (f >= m->numframes)
		f = m->numframes-1;

	e->v.frame = f;
}


void PrintFrameName (model_t *m, int frame)
{
	aliashdr_t 			*hdr;
	maliasframedesc_t	*pframedesc;

	if (!(hdr = (aliashdr_t *)Mod_Extradata (m)))
		return;
	pframedesc = &hdr->frames[frame];

	Con_Printf ("frame %i: %s\n", frame, pframedesc->name);
}

/*
==================
Host_Viewnext_f
==================
*/
void Host_Viewnext_f (cmd_source_t src)
{
	edict_t	*e;
	model_t	*m;

	if (!(e = FindViewthing ()))
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame + 1;
	if (e->v.frame >= m->numframes)
		e->v.frame = m->numframes - 1;

	PrintFrameName (m, e->v.frame);
}

/*
==================
Host_Viewprev_f
==================
*/
void Host_Viewprev_f (cmd_source_t src)
{
	edict_t	*e;
	model_t	*m;

	if (!(e = FindViewthing ()))
		return;

	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame - 1;
	if (e->v.frame < 0)
		e->v.frame = 0;

	PrintFrameName (m, e->v.frame);
}

#endif		//#ifndef RQM_SV_ONLY

/*
===============================================================================

				DEMO LOOP CONTROL

===============================================================================
*/

#ifndef RQM_SV_ONLY

/*
==================
Host_Startdemos_f
==================
*/
void Host_Startdemos_f (cmd_source_t src)
{
	int	i, c;

	if (cls.state == ca_dedicated)
	{
		if (!sv.active)
			Cbuf_AddText ("map start\n", SRC_COMMAND);
		return;
	}

	c = Cmd_Argc() - 1;
	if (c > MAX_DEMOS)
	{
		Con_Printf ("Max %i demos in demoloop\n", MAX_DEMOS);
		c = MAX_DEMOS;
	}
	Con_Printf ("%i demo(s) in loop\n", c);

	for (i=1 ; i<c+1 ; i++)
		Q_strcpy (cls.demos[i-1], Cmd_Argv(i), sizeof(cls.demos[0]));

	if (!sv.active && cls.demonum != -1 && !cls.demoplayback)
	{
		cls.demonum = 0;
		CL_NextDemo ();
	}
	else
	{
		cls.demonum = -1;
	}
}

/*
==================
Host_Demos_f

Return to looping demos
==================
*/
void Host_Demos_f (cmd_source_t src)
{
	if (cls.state == ca_dedicated)
		return;

	if (cls.demonum == -1)
		cls.demonum = 1;

	CL_Disconnect_f (src);
	CL_NextDemo ();
}

/*
==================
Host_Stopdemo_f

Return to looping demos
==================
*/
void Host_Stopdemo_f (cmd_source_t src)
{
	if (cls.state == ca_dedicated || !cls.demoplayback)
	{
		Con_Printf ("No demo is currently playing!\n");
		return;
	}

	CL_StopPlayback ();
	CL_Disconnect (false);
}

void Host_GetCoords_f (cmd_source_t src)
{
	char	name[MAX_OSPATH], cmdname[MAX_QPATH] = "";
	FILE	*f;

	if (src == SRC_CLIENT)
		return;

	if (cls.state != ca_connected)
	{
		Con_Print ("ERROR: You must be connected\n");
		return;
	}

	if (Cmd_Argc() == 3)
	{
		Q_strcpy (name, Cmd_Argv(1), sizeof(name));
		Q_strcpy (cmdname, Cmd_Argv(2), sizeof(cmdname));
	}
	else if (Cmd_Argc() == 2)
	{
		Q_strcpy (name, Cmd_Argv(1), sizeof(name));
	}
	else if (Cmd_Argc() == 1)
	{
		Q_strcpy (name, "camfile.cam", sizeof(name));
	}
	else
	{
		Con_Printf ("Usage: %s [filename] [command]\n", Cmd_Argv(0));
		return;
	}

	COM_DefaultExtension (name, ".cam", sizeof(name));

	if (!(f = fopen(va("%s/%s", com_gamedir, name), "a")))
	{
		Con_Printf ("Couldn't write %s\n", name);
		return;
	}

	fprintf (f, " %s %3.1f %3.1f %3.1f\n", cmdname, sv_player->v.origin[0], sv_player->v.origin[1], sv_player->v.origin[2]);
	fclose (f);
}

#endif		//#ifndef RQM_SV_ONLY


//=============================================================================

/*
==================
Host_InitCommands
==================
*/
void Host_InitCommands (void)
{
	Cmd_AddCommand ("status", Host_Status_f, 0);
	Cmd_AddCommand ("quit", Host_Quit_f, 0);
	Cmd_AddCommand ("map", Host_Map_f, 0);
	Cmd_AddCommand ("restart", Host_Restart_f, 0);
	Cmd_AddCommand ("changelevel", Host_Changelevel_f, 0);
	Cmd_AddCommand ("name", Host_Name_f, 0);
	Cmd_AddCommand ("version", Host_Version_f, 0);
#ifdef IDGODS
	Cmd_AddCommand ("please", Host_Please_f, 0);
#endif
	Cmd_AddCommand ("say", Host_Say_f, 0);
	Cmd_AddCommand ("say_team", Host_Say_Team_f, 0);
	Cmd_AddCommand ("tell", Host_Tell_f, 0);
	Cmd_AddCommand ("color", Host_Color_f, 0);
	Cmd_AddCommand ("kill", Host_Kill_f, 0);
	Cmd_AddCommand ("pause", Host_Pause_f, 0);
	Cmd_AddCommand ("spawn", Host_Spawn_f, 0);
	Cmd_AddCommand ("begin", Host_Begin_f, 0);
	Cmd_AddCommand ("prespawn", Host_PreSpawn_f, 0);
	Cmd_AddCommand ("kick", Host_Kick_f, 0);
	Cmd_AddCommand ("ping", Host_Ping_f, 0);
	Cmd_AddCommand ("load", Host_Loadgame_f, 0);

	Cmd_AddCommand ("give", Host_Give_f, 0);
	Cmd_AddCommand ("god", Host_God_f, 0);
	Cmd_AddCommand ("notarget", Host_Notarget_f, 0);
	Cmd_AddCommand ("noclip", Host_Noclip_f, 0);
	Cmd_AddCommand ("fly", Host_Fly_f, 0);

#ifndef RQM_SV_ONLY
	Cmd_AddCommand ("save", Host_Savegame_f, 0);
	Cmd_AddCommand ("connect", Host_Connect_f, 0);
	Cmd_AddCommand ("reconnect", Host_Reconnect_f, 0);

	Cmd_AddCommand ("startdemos", Host_Startdemos_f, 0);
	Cmd_AddCommand ("demos", Host_Demos_f, 0);
	Cmd_AddCommand ("stopdemo", Host_Stopdemo_f, 0);

	Cmd_AddCommand ("viewmodel", Host_Viewmodel_f, 0);
	Cmd_AddCommand ("viewframe", Host_Viewframe_f, 0);
	Cmd_AddCommand ("viewnext", Host_Viewnext_f, 0);
	Cmd_AddCommand ("viewprev", Host_Viewprev_f, 0);
	Cmd_AddCommand ("create", Host_Create_f, 0);
	Cmd_AddCommand ("getcoords", Host_GetCoords_f, 0);

	Cvar_RegisterBool (&cl_confirmquit);
#endif

	Cvar_RegisterString (&cl_name);
	Cvar_RegisterInt (&cl_color, 0, (13 << 4) | 13);

	Cmd_AddCommand ("mcache", Mod_Print_f, 0);
}
