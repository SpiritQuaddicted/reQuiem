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
// sv_main.c -- server main program

#include "quakedef.h"

#ifdef HEXEN2_SUPPORT

  extern cvar_t	max_temp_edicts;

  extern int		*pr_string_index;
  extern char		*pr_global_strings;
  extern int		pr_string_count;

  extern cvar_t		randomclass;

#ifndef RQM_SV_ONLY
  extern int		total_loading_size, current_loading_size, loading_stage;
#endif

//  unsigned int		info_mask, info_mask2;		JDH: moved to server_t (sv)
  int				sv_kingofhill;
#endif

extern cvar_t sv_altnoclip;

#define SV_NUM_PROTOCOLS 3
name_value_t sv_valid_protocols[SV_NUM_PROTOCOLS] =
{
	{"Auto",     0},
	{"Standard", PROTOCOL_VERSION_STD},
	{"Extended", PROTOCOL_VERSION_BJP3}
};

qboolean OnChange_sv_protocol (cvar_t *var, const char *value);

server_t		sv;
server_static_t	svs;

// JDH: new cvars:
cvar_t	sv_protocol       = {"sv_protocol",       "0", 0, OnChange_sv_protocol};
cvar_t	sv_fishfix        = {"sv_fishfix",        "1", CVAR_FLAG_ARCHIVE};
cvar_t	sv_entpatch       = {"sv_entpatch",       "1", CVAR_FLAG_ARCHIVE};
cvar_t	sv_imp12hack      = {"sv_imp12hack",      "1", CVAR_FLAG_ARCHIVE};
cvar_t	host_cutscenehack = {"host_cutscenehack", "1", CVAR_FLAG_ARCHIVE};

char	localmodels[MAX_MODELS][6];	// inline model names for precache (eg "*5", "*1018")

qboolean	pq_cheatfree;

//============================================================================

/*
===============
SV_Init
===============
*/
void SV_Init (void)
{
	int	i;

	Cvar_Register (&sv_maxvelocity);
	Cvar_Register (&sv_gravity);
	Cvar_Register (&sv_friction);
	Cvar_Register (&sv_edgefriction);
	Cvar_Register (&sv_stopspeed);
	Cvar_Register (&sv_maxspeed);
	Cvar_Register (&sv_accelerate);
	Cvar_Register (&sv_idealpitchscale);
	Cvar_RegisterFloat (&sv_aim, 0, 1);
	Cvar_RegisterBool (&sv_nostep);
	Cvar_RegisterBool (&sv_altnoclip);

// JDH: new cvars:
	Cvar_RegisterInt  (&sv_protocol, 0, CVAR_UNKNOWN_MAX);
	Cvar_RegisterBool (&host_cutscenehack);
	Cvar_RegisterInt  (&sv_imp12hack, 0, 2);
	Cvar_RegisterBool (&sv_fishfix);
	Cvar_RegisterBool (&sv_entpatch);

	if ((i = COM_CheckParm("-cheatfree")))
		pq_cheatfree = true;
	else
		pq_cheatfree = false;

	for (i=0 ; i<MAX_MODELS ; i++)
		sprintf (localmodels[i], "*%i", i);

#ifdef HEXEN2_SUPPORT
	sv_kingofhill = 0;//Initialize King of Hill to world
#endif
}

qboolean SV_IsValidProtocol (int prot)
{
	int i;

	for (i = 0; i < SV_NUM_PROTOCOLS; i++)
	{
		if (prot == sv_valid_protocols[i].value)
			return true;
	}

	return false;
}

/*
================
OnChange_sv_protocol
================
*/
qboolean OnChange_sv_protocol (cvar_t *var, const char *value)
{
	int i;

	if (!SV_IsValidProtocol ((int) Q_atof(value)))
	{
		Con_Printf ("Valid values for %s are %d", var->name, sv_valid_protocols[0].value);
		for (i = 1; i < SV_NUM_PROTOCOLS; i++)
			Con_Printf (", %d", sv_valid_protocols[i].value);
		Con_Print ("\n");
		return true;	// don't allow change
	}

	if (sv.active)
		Con_Printf ("Change to %s will have no effect until a new map is started.\n", var->name);

	return false;		// allow change
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

qboolean SV_CheckDatagramSpace (sizebuf_t *msg, int bytes_needed)
{
	if (msg->cursize > msg->maxsize - bytes_needed)
	{
	// switch to larger buffer, unless user has specifically requested PROTOCOL_VERSION_STD
//		if ((sv.datagram.maxsize == MAX_DATAGRAM_OLD) && (sv_protocol.value != PROTOCOL_VERSION_STD)/*!sv_oldprotocol.value*/)
		if ((msg->maxsize == MAX_DATAGRAM_OLD) && (sv.desired_protocol != PROTOCOL_VERSION_STD))
		{
			msg->maxsize = MAX_DATAGRAM;
			if (msg->cursize > msg->maxsize - bytes_needed)
				return false;
		}
		else return false;
	}

	return true;
}

/*
================
SV_ChangeToProtocol
================
*/
//void SV_ChangeToProtocol (sizebuf_t *sb, client_t *client, int protocol)
void SV_ChangeToProtocol (sizebuf_t *sb, int protocol)
{
//	MSG_WriteCmd (sb, svc_version);
//	MSG_WriteLong (sb, protocol);
//	Con_DPrintf ("Switching to protocol %d\n", protocol);

	/*if (client)
	{
		client->message.maxsize = sizeof(client->msgbuf);
		client->protocol = protocol;
	}
	else*/
	{
		sv.datagram.maxsize = sizeof(sv.datagram_buf);
		sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
		sv.signon.maxsize = sizeof(sv.signon_buf);
		sv.protocol = protocol;
	}

	MSG_WriteCmd (sb, svc_version);
	MSG_WriteLong (sb, protocol);
	Con_DPrintf ("Switching to protocol %d\n", protocol);
}

/*
==================
SV_StartParticle

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, v;

	if (!SV_CheckDatagramSpace (&sv.datagram, 16))
		return;

	MSG_WriteCmd (&sv.datagram, svc_particle);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	for (i=0 ; i<3 ; i++)
	{
		v = dir[i]*16;
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		MSG_WriteChar (&sv.datagram, v);
	}
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, color);
}


#ifdef HEXEN2_SUPPORT

/*
==================
SV_StartParticle2

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, int effect, int count)
{
	if (!SV_CheckDatagramSpace (&sv.datagram, 36))
		return;

	MSG_WriteCmd (&sv.datagram, svc_particle2);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteFloat (&sv.datagram, dmin[0]);
	MSG_WriteFloat (&sv.datagram, dmin[1]);
	MSG_WriteFloat (&sv.datagram, dmin[2]);
	MSG_WriteFloat (&sv.datagram, dmax[0]);
	MSG_WriteFloat (&sv.datagram, dmax[1]);
	MSG_WriteFloat (&sv.datagram, dmax[2]);

	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, effect);
}

/*
==================
SV_StartParticle3

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle3 (vec3_t org, vec3_t box, int color, int effect, int count)
{
	if (!SV_CheckDatagramSpace (&sv.datagram, 15))
		return;

	MSG_WriteCmd (&sv.datagram, svc_particle3);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteByte (&sv.datagram, box[0]);
	MSG_WriteByte (&sv.datagram, box[1]);
	MSG_WriteByte (&sv.datagram, box[2]);

	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, effect);
}

/*
==================
SV_StartParticle4

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle4 (vec3_t org, float radius, int color, int effect, int count)
{
	if (!SV_CheckDatagramSpace (&sv.datagram, 13))
		return;

	MSG_WriteCmd (&sv.datagram, svc_particle4);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteByte (&sv.datagram, radius);

	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, effect);
}

/*
==================
SV_StopSound
==================
*/
void SV_StopSound (edict_t *entity, int channel)
{
	int			ent;

	if (!SV_CheckDatagramSpace (&sv.datagram, 4))
		return;

	ent = NUM_FOR_EDICT(entity);
	channel = (ent<<3) | channel;

	MSG_WriteCmd (&sv.datagram, svc_stopsound);
	MSG_WriteShort (&sv.datagram, channel);
}

/*
==================
SV_UpdateSoundPos
==================
*/
void SV_UpdateSoundPos (edict_t *entity, int channel)
{
	int			ent;
    int			i;

	if (!SV_CheckDatagramSpace (&sv.datagram, 4))
		return;

	ent = NUM_FOR_EDICT(entity);
	channel = (ent<<3) | channel;

	MSG_WriteCmd (&sv.datagram, svc_sound_update_pos);
	MSG_WriteShort (&sv.datagram, channel);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.datagram, entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i]));
}

#endif		// #ifdef HEXEN2_SUPPORT

/*
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
allready running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)
==================
*/
void SV_StartSound (edict_t *entity, int channel, const char *sample, int volume, float attenuation)
{
	int		sound_num, field_mask, i, ent, maxsounds;

	if (volume < 0 || volume > 255)
		Sys_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	if (!SV_CheckDatagramSpace (&sv.datagram, 16))
	{
		Con_DPrintf ("SV_StartSound: packet overflow\n");
		return;
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		maxsounds = MAX_SOUNDS_H2;
	else
#endif
	maxsounds = MAX_SOUNDS;

// find precache number for sound
	for (sound_num=1 ; sound_num < maxsounds && sv.sound_precache[sound_num] ; sound_num++)
		if (COM_FilenamesEqual(sample, sv.sound_precache[sound_num]))
			break;

	if (sound_num == maxsounds || !sv.sound_precache[sound_num])
	{
		Con_Printf ("SV_StartSound: %s not precacheed\n", sample);
		return;
	}

	ent = NUM_FOR_EDICT(entity);

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;

	if (sound_num > 255)
	{
#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			field_mask |= SND_OVERFLOW;
			sound_num -= 255;
		}
//		else
#endif
	// JDH: until I figure out a way to update client->protocol
	/*	if ((sv.serverinfo_protocol == PROTOCOL_VERSION_STD) && !sv_oldprotocol.value)
		{
			SV_ChangeToProtocol (&sv.datagram, NULL, PROTOCOL_VERSION_BJP3);
		}
	*/
	}

// directed messages go only to the entity the are targeted on
	MSG_WriteCmd (&sv.datagram, svc_sound);
	MSG_WriteByte (&sv.datagram, field_mask);
	if (field_mask & SND_VOLUME)
		MSG_WriteByte (&sv.datagram, volume);
	if (field_mask & SND_ATTENUATION)
		MSG_WriteByte (&sv.datagram, attenuation*64);
	MSG_WriteShort (&sv.datagram, channel);

	if (sv.protocol == PROTOCOL_VERSION_BJP3)
		MSG_WriteShort (&sv.datagram, sound_num);
	else
		MSG_WriteByte (&sv.datagram, sound_num);

	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.datagram, entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i]));
}

/*
==============================================================================

CLIENT SPAWNING

==============================================================================
*/

extern qboolean mod_oversized;

/*
================
SV_InitClient
================
*/
void SV_InitClient (client_t *client)
{
//	int num_models, num_sounds;

	client->in_cutscene = false;

#ifdef HEXEN2_SUPPORT
//	if (sv_currentprotocol == PROTOCOL_VERSION_H2_112)
	if (hexen2)
	{
		//client->protocol = PROTOCOL_VERSION_H2_112;
		client->message.maxsize = MAX_MSGLEN_H2;
	}
	else
#endif
	{
		client->message.maxsize = (sv.protocol == PROTOCOL_VERSION_BJP3) ? sizeof(client->msgbuf) : MAX_MSGLEN_OLD;

		/*if (!sv_oldprotocol.value)
		{
		// count the number of precached models & sounds
			for (num_models = 1; sv.model_precache[num_models]; num_models++) ;
			for (num_sounds = 1; sv.sound_precache[num_sounds]; num_sounds++) ;

			if ((num_models > MAX_MODELS_OLD) || (num_sounds > MAX_SOUNDS_OLD))
			{
				client->protocol = PROTOCOL_VERSION_BJP3;
				client->message.maxsize = sizeof(client->msgbuf);
				return;
			}
		}

		client->protocol = PROTOCOL_VERSION_STD;
		client->message.maxsize = MAX_MSGLEN_OLD;*/
	}
}

const char * SV_MapTitle (void)
{
	if (sv.state != ss_active)
		return NULL;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		int index = (int) *(float *) &sv.edicts->v.message;
			// Hexen II stores message index as float

		if ((index > 0) && (index <= pr_string_count))
		{
			index = pr_string_index[index-1];
			return pr_global_strings + index;
		}
		return pr_strings + sv.edicts->v.netname;
	}
#endif

	return pr_strings + sv.edicts->v.message;
}

/*
================
SV_SendServerinfo

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void SV_SendServerinfo (client_t *client)
{
	char	message[2048], *gamename;
	float	server_vers;
	int		max_models, max_sounds, i;

	SV_InitClient (client);

#ifdef HEXEN2_SUPPORT
//	if (sv_currentprotocol == PROTOCOL_VERSION_H2_112)
	if (hexen2)
	{
		gamename = "Hexen2";
		server_vers = SERVER_VERSION_H2;
		max_models = MAX_MODELS_H2;
		max_sounds = MAX_SOUNDS_H2;		// 512
	}
	else
#endif
	{
		gamename = "Quake";
		server_vers = SERVER_VERSION;

//		if (client->protocol == PROTOCOL_VERSION_BJP3)
		if (sv.protocol == PROTOCOL_VERSION_BJP3)
		{
			max_models = MAX_MODELS;
			max_sounds = MAX_SOUNDS;
		}
		else
		{
			max_models = MAX_MODELS_OLD;
			max_sounds = MAX_SOUNDS_OLD;
		}
	}

	MSG_WriteCmd (&client->message, svc_print);
	sprintf (message, "\x02""\nreQuiem "REQUIEM_VERSION_SHORT" Server (%s %4.2f): %i CRC\n",
				gamename, server_vers, pr_crc);
	MSG_WriteString (&client->message, message);

	MSG_WriteCmd (&client->message, svc_serverinfo);

//	MSG_WriteLong (&client->message, client->protocol);
	MSG_WriteLong (&client->message, sv.protocol);
/*
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		MSG_WriteLong (&client->message, PROTOCOL_VERSION_H2_112);
	else
#endif
	MSG_WriteLong (&client->message, PROTOCOL_VERSION_STD);
*/
	MSG_WriteByte (&client->message, svs.maxclients);

	if (!coop.value && deathmatch.value)
	{
		MSG_WriteByte (&client->message, GAME_DEATHMATCH);
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			MSG_WriteShort (&client->message, sv_kingofhill);
	#endif
	}
	else
		MSG_WriteByte (&client->message, GAME_COOP);

	MSG_WriteString (&client->message, SV_MapTitle());

	for (i = 1; i < max_models && sv.model_precache[i]; i++)
		MSG_WriteString (&client->message, sv.model_precache[i]);
	MSG_WriteByte (&client->message, 0);

	for (i = 1; i < max_sounds && sv.sound_precache[i]; i++)
		MSG_WriteString (&client->message, sv.sound_precache[i]);
	MSG_WriteByte (&client->message, 0);

// send music
	MSG_WriteCmd (&client->message, svc_cdtrack);
	MSG_WriteByte (&client->message, sv.edicts->v.sounds);
	MSG_WriteByte (&client->message, sv.edicts->v.sounds);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		MSG_WriteCmd (&client->message, svc_midi_name);
		MSG_WriteString (&client->message, sv.midi_name);
	}
#endif

// set view
	MSG_WriteCmd (&client->message, svc_setview);
	MSG_WriteShort (&client->message, NUM_FOR_EDICT(client->edict));

	MSG_WriteCmd (&client->message, svc_signonnum);
	MSG_WriteByte (&client->message, 1);

	client->sendsignon = true;
	client->spawned = false;		// need prespawn, spawn, etc
}

/*
================
SV_ConnectClient

Initializes a client_t for a new net connection. This will only be called
once for a player each game, not once for each level change.
================
*/
void SV_ConnectClient (int clientnum)
{
	edict_t			*ent;
	client_t		*client;
	int			i, edictnum;
	struct qsocket_s	*netconnection;
	float			spawn_parms[NUM_SPAWN_PARMS];

	client = svs.clients + clientnum;

	if (client->netconnection->mod == MOD_JOEQUAKE)
		Con_DPrintf ("JoeQuake-compatible client %s connected\n", client->netconnection->address);
	else
		Con_DPrintf ("Client %s connected\n", client->netconnection->address);

	edictnum = clientnum + 1;

	ent = EDICT_NUM(edictnum);

// set up the client_t
	netconnection = client->netconnection;

	if (sv.loadgame)
		memcpy (spawn_parms, client->spawn_parms, sizeof(spawn_parms));
	memset (client, 0, sizeof(*client));
	client->netconnection = netconnection;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		client->send_all_v = true;
		memset(&sv.states[clientnum], 0, sizeof(client_state2_t));
	}
#endif

	strcpy (client->name, "unconnected");
	client->active = true;
	client->spawned = false;
	client->edict = ent;
	client->message.data = client->msgbuf;
	client->message.allowoverflow = true;		// we can catch it

/* 2008/08/29 - moved to SV_SendServerinfo

	if (sv.serverinfo_protocol == PROTOCOL_VERSION_STD)
		client->message.maxsize = MAX_MSGLEN_OLD;
	else
		client->message.maxsize = sizeof(client->msgbuf);

// JDH:
	client->protocol = PROTOCOL_VERSION_STD;
	client->in_cutscene = false;
*/

	if (sv.loadgame)
	{
		memcpy (client->spawn_parms, spawn_parms, sizeof(spawn_parms));
	}
	else
	{
	// call the progs to get default spawn parms for the new client
	#ifdef HEXEN2_SUPPORT
		if (!hexen2)
	#endif
		{
			PR_ExecuteProgram (*pr_global_ptrs.SetNewParms);
			for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
				client->spawn_parms[i] = (pr_global_ptrs.parm1)[i];
		}
	}

	SV_SendServerinfo (client);
}


/*
===================
SV_CheckForNewClients

===================
*/
void SV_CheckForNewClients (void)
{
	struct qsocket_s	*ret;
	int			i;

// check for new connections
	while (1)
	{
		if (!(ret = NET_CheckNewConnections()))
			break;

	// init a new client structure
		for (i=0 ; i<svs.maxclients ; i++)
			if (!svs.clients[i].active)
				break;
		if (i == svs.maxclients)
			Sys_Error ("Host_CheckForNewClients: no free clients");

		svs.clients[i].netconnection = ret;
		SV_ConnectClient (i);

		net_activeconnections++;
	}
}


/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

/*
==================
SV_ClearDatagram

==================
*/
void SV_ClearDatagram (void)
{
	SZ_Clear (&sv.datagram);
}

/*
=============================================================================

The PVS must include a small area around the client to allow head bobbing
or other small motion on the client side.  Otherwise, a bob might cause an
entity that should be visible to not show up, especially when the bob
crosses a waterline.

=============================================================================
*/

int		fatbytes;
byte	fatpvs[MAX_MAP_LEAFS/8];

void SV_AddToFatPVS (const vec3_t org, const mnode_t *node)
{
	int		i;
	byte		*pvs;
	mplane_t	*plane;
	float		d;

	while (1)
	{
	// if this is a leaf, accumulate the pvs bits
		if (node->contents < 0)
		{
			if (node->contents != CONTENTS_SOLID)
			{
				pvs = Mod_LeafPVS ((mleaf_t *)node, sv.worldmodel);
				for (i=0 ; i<fatbytes ; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}

		plane = node->plane;
		d = PlaneDiff (org, plane);
		if (d > 8)
			node = node->children[0];
		else if (d < -8)
			node = node->children[1];
		else
		{	// go down both
			SV_AddToFatPVS (org, node->children[0]);
			node = node->children[1];
		}
	}
}

/*
=============
SV_FatPVS

Calculates a PVS that is the inclusive or of all leafs within 8 pixels of the
given point.
=============
*/
byte *SV_FatPVS (const vec3_t org)
{
	fatbytes = (sv.worldmodel->numleafs+31)>>3;
	memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVS (org, sv.worldmodel->nodes);
	return fatpvs;
}


//=============================================================================

/*
================
MSG_WriteModelIndex
  returns false only if forcing original protocol, and modelindex >= MAX_MODELS_OLD
================
*/
qboolean MSG_WriteModelIndex (sizebuf_t *sb, int index, const client_t *client)
{
//	sizebuf_t	tempmsg;
//	byte		buf[256];
//	int			protocol, cmdlen, i;
	int			cmdlen, i;

/*
#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
*/
//	protocol = client ? client->protocol : sv.serverinfo_protocol;
//	if (protocol == PROTOCOL_VERSION_STD)
	if (sv.protocol == PROTOCOL_VERSION_STD)
	{
		if (index < MAX_MODELS_OLD)
		{
			MSG_WriteByte (sb, index);
			return true;
		}

//		if (sv_oldprotocol.value)
		if (client || (sv.desired_protocol == PROTOCOL_VERSION_STD)/*sv_oldprotocol.value*/)
			//Con_Printf ("WARNING: server found model with index %d, but protocol is 15!\n", index);
			return false;

		//SZ_CheckSpace (sb, 5);		// make sure buffer has 5 free bytes

		// shift current command & data forward 5 bytes:
		for (i = sb->cursize; i > sb->lastcmdpos; i--)
			sb->data[i+4] = sb->data[i-1];

		// insert svc_version command:
		cmdlen = sb->cursize - sb->lastcmdpos;
		sb->cursize = sb->lastcmdpos;

//		SV_ChangeToProtocol (sb, client, PROTOCOL_VERSION_BJP3);
		SV_ChangeToProtocol (sb, PROTOCOL_VERSION_BJP3);

		sb->lastcmdpos = sb->cursize;
		sb->cursize += cmdlen;
	}

	MSG_WriteShort (sb, index);
	return true;
}

/*
=============
SV_WriteEntityToClient
=============
*/
qboolean SV_WriteEntityToClient (client_t *client, edict_t *ent, int entnum,
								 sizebuf_t *msg, int bits, int modelindex)
{
	int msg_origsize = msg->cursize;
//	void (*writecoord)(sizebuf_t *, float);

	MSG_WriteByte (msg, bits | U_SIGNAL);

	if (bits & U_MOREBITS)
		MSG_WriteByte (msg, bits >> 8);

#ifdef HEXEN2_SUPPORT
	if ((hexen2) && (bits & U_MOREBITS2))
		MSG_WriteByte (msg, bits>>16);
#endif

	if (bits & U_LONGENTITY)
		MSG_WriteShort (msg, entnum);
	else
		MSG_WriteByte (msg, entnum);

	if (bits & U_MODEL)
	{
		if (!MSG_WriteModelIndex (msg, modelindex, client))
		{
			// undo what's been written so far
			msg->cursize = msg_origsize;
			return false;
		}
	}

	if (bits & U_FRAME)
		MSG_WriteByte (msg, ent->v.frame);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (bits & U_COLORMAP_H2)
			MSG_WriteByte (msg, ent->v.colormap);

		if (bits & U_SKIN_H2)
		{
			MSG_WriteByte (msg, ent->v.skin);
			MSG_WriteByte (msg, ent->v.drawflags);
		}
		if (bits & U_EFFECTS_H2)
			MSG_WriteByte (msg, ent->v.effects);
	}
	else
#endif
	{
		if (bits & U_COLORMAP)
			MSG_WriteByte (msg, ent->v.colormap);
		if (bits & U_SKIN)
			MSG_WriteByte (msg, ent->v.skin);
		if (bits & U_EFFECTS)
			MSG_WriteByte (msg, ent->v.effects);
	}

// JDH: use coordinate hack only if client is local
	/*if (mod_oversized && !client->netconnection->driver)
		writecoord = MSG_WriteFloat;
	else
		writecoord = MSG_WriteCoord;*/

#ifdef _DEBUG
/*	if (!strcmp(pr_strings + ent->v.model, "*306"))
	{
		if (bits & U_ANGLE2)
			Con_DPrintf ("rotated to %f\n", ent->v.angles[1]);
	}
*/
#endif

	if (bits & U_ORIGIN1)
		MSG_WriteCoord (msg, ent->v.origin[0]);
	if (bits & U_ANGLE1)
		MSG_WriteAngle (msg, ent->v.angles[0]);
	if (bits & U_ORIGIN2)
		MSG_WriteCoord (msg, ent->v.origin[1]);
	if (bits & U_ANGLE2)
		MSG_WriteAngle (msg, ent->v.angles[1]);
	if (bits & U_ORIGIN3)
		MSG_WriteCoord (msg, ent->v.origin[2]);
	if (bits & U_ANGLE3)
		MSG_WriteAngle (msg, ent->v.angles[2]);

#ifdef HEXEN2_SUPPORT
	if (bits & U_SCALE)
	{
		MSG_WriteByte (msg, (int)(ent->v.scale*100.0) & 255);
		MSG_WriteByte (msg, (int)(ent->v.abslight*255.0) & 255);
	}
#endif

	return true;
}

#ifdef HEXEN2_SUPPORT

#define CLIENT_FRAME_INIT	255
#define CLIENT_FRAME_RESET	254
#define CLEAR_LIMIT			 2

/*
============================
SV_WriteEntitiesToClient_H2
============================
*/
void SV_WriteEntitiesToClient_H2 (client_t *client, edict_t *clent, sizebuf_t *msg)
{
	int		e, i;
	int		bits;
	byte	*pvs;
	vec3_t	org;
	float	miss;
	edict_t	*ent;
	int		temp_index;
	char	NewName[MAX_QPATH];
	long	flagtest;
	int				position = 0;
	int				client_num;
	unsigned long   client_bit;
	client_frames_t *reference, *build;
	client_state2_t	*state;
	entity_state_t	*ref_ent, *set_ent;
	qboolean		FoundInList, DoRemove;
	short			RemoveList[MAX_CLIENT_STATES], NumToRemove;
//	qboolean		DoPlayer,DoMonsters,DoMissiles,DoMisc,IgnoreEnt;


	client_num = client-svs.clients;
	client_bit = 1<<client_num;
	state = &sv.states[client_num];
	reference = &state->frames[0];

	if (client->last_sequence != client->current_sequence)
	{
		// Old sequence
//		Con_Printf("SV: Old sequence SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		client->current_frame++;
		if (client->current_frame > MAX_FRAMES+1)
			client->current_frame = MAX_FRAMES+1;
	}
	else if ((client->last_frame == CLIENT_FRAME_INIT) || (client->last_frame == 0) ||
			 (client->last_frame == MAX_FRAMES+1))
	{
		// Reference expired in current sequence
//		Con_Printf("SV: Expired SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		client->current_frame = 1;
		client->current_sequence++;
	}
	else if ((client->last_frame >= 1) && (client->last_frame <= client->current_frame))
	{
		// Got a valid frame
//		Con_Printf("SV: Valid SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		*reference = state->frames[client->last_frame];

		for (i=0; i < reference->count; i++)
		{
			if (reference->states[i].flags & ENT_CLEARED)
			{
				e = reference->states[i].index;
				ent = EDICT_NUM(e);
				if (ent->baseline.ClearCount[client_num] < CLEAR_LIMIT)
				{
					ent->baseline.ClearCount[client_num]++;
				}
				else if (ent->baseline.ClearCount[client_num] == CLEAR_LIMIT)
				{
					ent->baseline.ClearCount[client_num] = 3;
					reference->states[i].flags &= ~ENT_CLEARED;
				}
			}
		}

		client->current_frame = 1;
		client->current_sequence++;
	}
	else
	{
		// Normal frame advance
//		Con_Printf("SV: Normal SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		client->current_frame++;
		if (client->current_frame > MAX_FRAMES+1)
			client->current_frame = MAX_FRAMES+1;
	}

/*	DoPlayer = DoMonsters = DoMissiles = DoMisc = false;

	if ((int)sv_update_player.value)
		DoPlayer = (client->current_sequence % ((int)sv_update_player.value)) == 0;
	if ((int)sv_update_monsters.value)
		DoMonsters = (client->current_sequence % ((int)sv_update_monsters.value)) == 0;
	if ((int)sv_update_missiles.value)
		DoMissiles = (client->current_sequence % ((int)sv_update_missiles.value)) == 0;
	if ((int)sv_update_misc.value)
		DoMisc = (client->current_sequence % ((int)sv_update_misc.value)) == 0;
*/
	build = &state->frames[client->current_frame];
	memset(build, 0, sizeof(*build));
	client->last_frame = CLIENT_FRAME_RESET;

	NumToRemove = 0;
	MSG_WriteCmd (msg, svc_reference);
	MSG_WriteByte (msg, client->current_frame);
	MSG_WriteByte (msg, client->current_sequence);

	// find the client's PVS
	if (clent->v.cameramode)
	{
		ent = PROG_TO_EDICT(clent->v.cameramode);
		VectorCopy(ent->v.origin, org);
	}
	else
		VectorAdd (clent->v.origin, clent->v.view_ofs, org);

	pvs = SV_FatPVS (org);

	// send over all entities (excpet the client) that touch the pvs
	ent = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
	{
		DoRemove = false;
		// don't send if flagged for NODRAW and there are no lighting effects
		if (ent->v.effects == EF_NODRAW)
		{
			DoRemove = true;
			goto skipA;
		}

		// ignore if not touching a PV leaf
		if (ent != clent)	// clent is ALWAYS sent
		{	// ignore ents without visible models
			if (!ent->v.modelindex || !pr_strings[ent->v.model])
			{
				DoRemove = true;
				goto skipA;
			}

			for (i=0 ; i < ent->num_leafs ; i++)
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i]&7) ))
					break;

			if (i == ent->num_leafs)
			{
				DoRemove = true;
				goto skipA;
			}
		}

skipA:
		flagtest = (long) ent->v.flags;

		/*IgnoreEnt = false;
		if (!DoRemove)
		{
			if (flagtest & FL_CLIENT)
			{
				if (!DoPlayer)
					IgnoreEnt = true;
			}
			else if (flagtest & FL_MONSTER)
			{
				if (!DoMonsters)
					IgnoreEnt = true;
			}
			else if (ent->v.movetype == MOVETYPE_FLYMISSILE ||
					 ent->v.movetype == MOVETYPE_BOUNCEMISSILE ||
					 ent->v.movetype == MOVETYPE_BOUNCE)
			{
				if (!DoMissiles)
					IgnoreEnt = true;
			}
			else
			{
				if (!DoMisc)
					IgnoreEnt = true;
			}
		}*/

		bits = 0;

		while (position < reference->count &&
			   e > reference->states[position].index)
			position++;

		if (position < reference->count && reference->states[position].index == e)
		{
			FoundInList = true;
			if (DoRemove)
			{
				RemoveList[NumToRemove] = e;
				NumToRemove++;
				continue;
			}

			ref_ent = &reference->states[position];
		}
		else
		{
			if (DoRemove /*|| IgnoreEnt*/)
				continue;

			ref_ent = &ent->baseline;

			ref_ent->index = e;
			ref_ent->flags = 0;

			FoundInList = false;
		}

		set_ent = &build->states[build->count];
		build->count++;
		if (ent->baseline.ClearCount[client_num] < CLEAR_LIMIT)
		{
			memset(ref_ent, 0, sizeof(*ref_ent));
			ref_ent->index = e;
		}
		*set_ent = *ref_ent;

//		if (IgnoreEnt)
//			continue;

		// send an update
		for (i=0 ; i<3 ; i++)
		{
			miss = ent->v.origin[i] - ref_ent->origin[i];
			if ( miss < -0.1 || miss > 0.1 )
			{
				bits |= U_ORIGIN1<<i;
				set_ent->origin[i] = ent->v.origin[i];
			}
		}

		if ( ent->v.angles[0] != ref_ent->angles[0] )
		{
			bits |= U_ANGLE1;
			set_ent->angles[0] = ent->v.angles[0];
		}

		if ( ent->v.angles[1] != ref_ent->angles[1] )
		{
			bits |= U_ANGLE2;
			set_ent->angles[1] = ent->v.angles[1];
		}

		if ( ent->v.angles[2] != ref_ent->angles[2] )
		{
			bits |= U_ANGLE3;
			set_ent->angles[2] = ent->v.angles[2];
		}

		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_NOLERP;	// don't mess up the step animation

		if (ref_ent->colormap != ent->v.colormap)
		{
			bits |= U_COLORMAP_H2;
			set_ent->colormap = ent->v.colormap;
		}

		if (ref_ent->skin != ent->v.skin
			|| ref_ent->drawflags != ent->v.drawflags)
		{
			bits |= U_SKIN_H2;
			set_ent->skin = ent->v.skin;
			set_ent->drawflags = ent->v.drawflags;
		}

		if (ref_ent->frame != ent->v.frame)
		{
			bits |= U_FRAME;
			set_ent->frame = ent->v.frame;
		}

		if (ref_ent->effects != ent->v.effects)
		{
			bits |= U_EFFECTS_H2;
			set_ent->effects = ent->v.effects;
		}

//		flagtest = (long)ent->v.flags;
		if (flagtest & 0xff000000)
		{
			Host_Error("Invalid flags setting for class %s",ent->v.classname + pr_strings);
			return;
		}

		temp_index = ent->v.modelindex;
		if (((int)ent->v.flags & FL_CLASS_DEPENDENT) && ent->v.model)
		{
			Q_strcpy (NewName, ent->v.model + pr_strings, sizeof(NewName));
			NewName[strlen(NewName)-5] = client->playerclass + '0';
			temp_index = SV_ModelIndex (NewName, false);
		}

		if (ref_ent->modelindex != temp_index)
		{
			bits |= U_MODEL;
			set_ent->modelindex = temp_index;
		}

		if ((ref_ent->scale != ((int)(ent->v.scale*100.0) & 255)) ||
			(ref_ent->abslight != ((int)(ent->v.abslight*255.0) & 255)))
		{
			bits |= U_SCALE;
			set_ent->scale = ((int)(ent->v.scale*100.0)&255);
			set_ent->abslight = (int)(ent->v.abslight*255.0)&255;
		}

		if (ent->baseline.ClearCount[client_num] < CLEAR_LIMIT)
		{
			bits |= U_CLEAR_ENT;
			set_ent->flags |= ENT_CLEARED;
		}

		if (!bits && FoundInList)
		{
			if (build->count >= MAX_CLIENT_STATES)
				break;

			continue;
		}

		if (e >= 256)
			bits |= U_LONGENTITY;

		if (bits >= 256)
			bits |= U_MOREBITS;

		if (bits >= 65536)
			bits |= U_MOREBITS2;

	//
	// write the message
	//
		SV_WriteEntityToClient (client, ent, e, msg, bits, temp_index);

		if (build->count >= MAX_CLIENT_STATES)
			break;
	}

	MSG_WriteCmd (msg, svc_clear_edicts);
	MSG_WriteByte (msg, NumToRemove);
	for (i=0; i < NumToRemove; i++)
		MSG_WriteShort (msg, RemoveList[i]);
}
#endif		// #ifdef HEXEN2_SUPPORT

// JDH: since entity update's size can vary (moreso than in orig Quake), I'll
//      write the message to this temp buf, then check if client's buf has space

byte		ent_buf[32];		// up to 19 bytes for regular flags, plus 12 more for Nehahra
sizebuf_t	ent_msg = {false, false, ent_buf, sizeof(ent_buf), 0, 0};

/*
=============
SV_WriteEntitiesToClient
=============
*/
void SV_WriteEntitiesToClient (client_t *client, edict_t *clent, sizebuf_t *msg)
{
	int		e, i, bits;
	byte	*pvs;
	vec3_t	org;
	float	miss;
	edict_t	*ent;
	eval_t  *val;
	float	alpha, fullbright;

// find the client's PVS
	VectorAdd (clent->v.origin, clent->v.view_ofs, org);
	pvs = SV_FatPVS (org);

// send over all entities (except the client) that touch the pvs
	ent = NEXT_EDICT(sv.edicts);
	for (e = 1 ; e < sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
	{
// ignore if not touching a PV leaf
		if (ent != clent)	// clent is ALWAYS sent
		{
// ignore ents without visible models
			if (!ent->v.modelindex || !pr_strings[ent->v.model])
				continue;

			for (i=0 ; i < ent->num_leafs ; i++)
				if (pvs[ent->leafnums[i]>>3] & (1 << (ent->leafnums[i] & 7)))
					break;

			if (i == ent->num_leafs)
				continue;		// not visible
		}

/*		if (msg->maxsize - msg->cursize < 16)
		{
			Con_Printf ("packet overflow (edict %i of %i)\n", e, sv.num_edicts);
			return;
		}
*/
// send an update
		bits = 0;

		for (i=0 ; i<3 ; i++)
		{
			miss = ent->v.origin[i] - ent->baseline.origin[i];
			if (miss < -0.1 || miss > 0.1)
				bits |= U_ORIGIN1 << i;
		}

		if (ent->v.angles[0] != ent->baseline.angles[0])
			bits |= U_ANGLE1;

		if (ent->v.angles[1] != ent->baseline.angles[1])
			bits |= U_ANGLE2;

		if (ent->v.angles[2] != ent->baseline.angles[2])
			bits |= U_ANGLE3;

		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_NOLERP;	// don't mess up the step animation

		if (ent->baseline.colormap != ent->v.colormap)
			bits |= U_COLORMAP;

		if (ent->baseline.skin != ent->v.skin)
			bits |= U_SKIN;

		if (ent->baseline.effects != ent->v.effects)
			bits |= U_EFFECTS;

		if (ent->baseline.frame != ent->v.frame)
			bits |= U_FRAME;

		if (ent->baseline.modelindex != ent->v.modelindex)
			bits |= U_MODEL;

	// nehahra: model alpha
	// JDH: send alpha info unless PROTOCOL_VERSION_STD is specified
//		if (nehahra || (client->protocol != PROTOCOL_VERSION_STD) || !sv_oldprotocol.value)
		if (nehahra || (sv.protocol != PROTOCOL_VERSION_STD) || (sv.desired_protocol != PROTOCOL_VERSION_STD)/*!sv_oldprotocol.value*/)
		{
			if ((val = GETEDICTFIELD(ent, eval_alpha)))
				alpha = val->_float;
			else
				alpha = 1;

			if ((val = GETEDICTFIELD(ent, eval_fullbright)))
				fullbright = val->_float;
			else
				fullbright = 0;

			if ((alpha < 1 && alpha > 0) || fullbright)
				bits |= U_TRANS;
		}

		if (e >= 256)
			bits |= U_LONGENTITY;

		if (bits >= 256)
			bits |= U_MOREBITS;

		SZ_Clear (&ent_msg);

	// write the message
		if (SV_WriteEntityToClient(client, ent, e, &ent_msg /*msg*/, bits, ent->v.modelindex))
		{
			if (/*nehahra &&*/ (bits & U_TRANS))
			{
				MSG_WriteFloat (&ent_msg /*msg*/, 2);
				MSG_WriteFloat (&ent_msg /*msg*/, alpha);
				MSG_WriteFloat (&ent_msg /*msg*/, fullbright);
			}
		}

	// JDH: moved this check to after the update message is built,
	//      and added support for larger buffer:

		if (!SV_CheckDatagramSpace (msg, ent_msg.cursize))
		{
			Con_Printf ("packet overflow (edict %i of %i)\n", e, sv.num_edicts);
			return;
		}

		/*if (msg->maxsize - msg->cursize < ent_msg.cursize)
		{
		//	if ((msg->maxsize == MAX_DATAGRAM_OLD) && !sv_oldprotocol.value)
			if ((msg->maxsize == MAX_DATAGRAM_OLD) && (sv.desired_protocol != PROTOCOL_VERSION_STD))
			{
				msg->maxsize = MAX_DATAGRAM;
			}
			else
			{
				Con_Printf ("packet overflow (edict %i of %i)\n", e, sv.num_edicts);
				return;
			}
		}*/

		SZ_Write (msg, ent_msg.data, ent_msg.cursize);
	}
}

/*
=============
SV_CleanupEnts
=============
*/
void SV_CleanupEnts (void)
{
	int	e;
	edict_t	*ent;

	ent = NEXT_EDICT(sv.edicts);
	for (e = 1 ; e < sv.num_edicts ; e++, ent = NEXT_EDICT(ent))
		ent->v.effects = (int)ent->v.effects & ~EF_MUZZLEFLASH;
}

#ifdef HEXEN2_SUPPORT
/*
=============
SV_SendUpdateInv
=============
*/
void SV_SendUpdateInv (client_t *client, edict_t *ent)
{
	int		sc1, sc2;
	byte	test;

	if (host_client->send_all_v)
	{
		sc1 = sc2 = 0xffffffff;
		host_client->send_all_v = false;
	}
	else
	{
		sc1 = sc2 = 0;

		if (ent->v.health != host_client->old_v.health)
			sc1 |= SC1_HEALTH;
		if (ent->v.level != host_client->old_v.level)
			sc1 |= SC1_LEVEL;
		if (ent->v.intelligence != host_client->old_v.intelligence)
			sc1 |= SC1_INTELLIGENCE;
		if (ent->v.wisdom != host_client->old_v.wisdom)
			sc1 |= SC1_WISDOM;
		if (ent->v.strength != host_client->old_v.strength)
			sc1 |= SC1_STRENGTH;
		if (ent->v.dexterity != host_client->old_v.dexterity)
			sc1 |= SC1_DEXTERITY;
		if (ent->v.weapon != host_client->old_v.weapon)
			sc1 |= SC1_WEAPON;
		if (ent->v.bluemana != host_client->old_v.bluemana)
			sc1 |= SC1_BLUEMANA;
		if (ent->v.greenmana != host_client->old_v.greenmana)
			sc1 |= SC1_GREENMANA;
		if (ent->v.experience != host_client->old_v.experience)
			sc1 |= SC1_EXPERIENCE;
		if (ent->v.cnt_torch != host_client->old_v.cnt_torch)
			sc1 |= SC1_CNT_TORCH;
		if (ent->v.cnt_h_boost != host_client->old_v.cnt_h_boost)
			sc1 |= SC1_CNT_H_BOOST;
		if (ent->v.cnt_sh_boost != host_client->old_v.cnt_sh_boost)
			sc1 |= SC1_CNT_SH_BOOST;
		if (ent->v.cnt_mana_boost != host_client->old_v.cnt_mana_boost)
			sc1 |= SC1_CNT_MANA_BOOST;
		if (ent->v.cnt_teleport != host_client->old_v.cnt_teleport)
			sc1 |= SC1_CNT_TELEPORT;
		if (ent->v.cnt_tome != host_client->old_v.cnt_tome)
			sc1 |= SC1_CNT_TOME;
		if (ent->v.cnt_summon != host_client->old_v.cnt_summon)
			sc1 |= SC1_CNT_SUMMON;
		if (ent->v.cnt_invisibility != host_client->old_v.cnt_invisibility)
			sc1 |= SC1_CNT_INVISIBILITY;
		if (ent->v.cnt_glyph != host_client->old_v.cnt_glyph)
			sc1 |= SC1_CNT_GLYPH;
		if (ent->v.cnt_haste != host_client->old_v.cnt_haste)
			sc1 |= SC1_CNT_HASTE;
		if (ent->v.cnt_blast != host_client->old_v.cnt_blast)
			sc1 |= SC1_CNT_BLAST;
		if (ent->v.cnt_polymorph != host_client->old_v.cnt_polymorph)
			sc1 |= SC1_CNT_POLYMORPH;
		if (ent->v.cnt_flight != host_client->old_v.cnt_flight)
			sc1 |= SC1_CNT_FLIGHT;
		if (ent->v.cnt_cubeofforce != host_client->old_v.cnt_cubeofforce)
			sc1 |= SC1_CNT_CUBEOFFORCE;
		if (ent->v.cnt_invincibility != host_client->old_v.cnt_invincibility)
			sc1 |= SC1_CNT_INVINCIBILITY;
		if (ent->v.artifact_active != host_client->old_v.artifact_active)
			sc1 |= SC1_ARTIFACT_ACTIVE;
		if (ent->v.artifact_low != host_client->old_v.artifact_low)
			sc1 |= SC1_ARTIFACT_LOW;
		if (ent->v.movetype != host_client->old_v.movetype)
			sc1 |= SC1_MOVETYPE;
		if (ent->v.cameramode != host_client->old_v.cameramode)
			sc1 |= SC1_CAMERAMODE;
		if (ent->v.hasted != host_client->old_v.hasted)
			sc1 |= SC1_HASTED;
		if (ent->v.inventory != host_client->old_v.inventory)
			sc1 |= SC1_INVENTORY;
		if (ent->v.rings_active != host_client->old_v.rings_active)
			sc1 |= SC1_RINGS_ACTIVE;

		if (ent->v.rings_low != host_client->old_v.rings_low)
			sc2 |= SC2_RINGS_LOW;
		if (ent->v.armor_amulet != host_client->old_v.armor_amulet)
			sc2 |= SC2_AMULET;
		if (ent->v.armor_bracer != host_client->old_v.armor_bracer)
			sc2 |= SC2_BRACER;
		if (ent->v.armor_breastplate != host_client->old_v.armor_breastplate)
			sc2 |= SC2_BREASTPLATE;
		if (ent->v.armor_helmet != host_client->old_v.armor_helmet)
			sc2 |= SC2_HELMET;
		if (ent->v.ring_flight != host_client->old_v.ring_flight)
			sc2 |= SC2_FLIGHT_T;
		if (ent->v.ring_water != host_client->old_v.ring_water)
			sc2 |= SC2_WATER_T;
		if (ent->v.ring_turning != host_client->old_v.ring_turning)
			sc2 |= SC2_TURNING_T;
		if (ent->v.ring_regeneration != host_client->old_v.ring_regeneration)
			sc2 |= SC2_REGEN_T;
		if (ent->v.haste_time != host_client->old_v.haste_time)
			sc2 |= SC2_HASTE_T;
		if (ent->v.tome_time != host_client->old_v.tome_time)
			sc2 |= SC2_TOME_T;
		if (ent->v.puzzle_inv1 != host_client->old_v.puzzle_inv1)
			sc2 |= SC2_PUZZLE1;
		if (ent->v.puzzle_inv2 != host_client->old_v.puzzle_inv2)
			sc2 |= SC2_PUZZLE2;
		if (ent->v.puzzle_inv3 != host_client->old_v.puzzle_inv3)
			sc2 |= SC2_PUZZLE3;
		if (ent->v.puzzle_inv4 != host_client->old_v.puzzle_inv4)
			sc2 |= SC2_PUZZLE4;
		if (ent->v.puzzle_inv5 != host_client->old_v.puzzle_inv5)
			sc2 |= SC2_PUZZLE5;
		if (ent->v.puzzle_inv6 != host_client->old_v.puzzle_inv6)
			sc2 |= SC2_PUZZLE6;
		if (ent->v.puzzle_inv7 != host_client->old_v.puzzle_inv7)
			sc2 |= SC2_PUZZLE7;
		if (ent->v.puzzle_inv8 != host_client->old_v.puzzle_inv8)
			sc2 |= SC2_PUZZLE8;
		if (ent->v.max_health != host_client->old_v.max_health)
			sc2 |= SC2_MAXHEALTH;
		if (ent->v.max_mana != host_client->old_v.max_mana)
			sc2 |= SC2_MAXMANA;
		if (ent->v.flags != host_client->old_v.flags)
			sc2 |= SC2_FLAGS;

		if (sv.info_mask != client->info_mask)
			sc2 |= SC2_OBJ;
		if (sv.info_mask2 != client->info_mask2)
			sc2 |= SC2_OBJ2;

		if (!sc1 && !sc2)
			return;
	}

	MSG_WriteCmd (&host_client->message, svc_update_inv);
	test = 0;
	if (sc1 & 0x000000ff)
		test |= 1;
	if (sc1 & 0x0000ff00)
		test |= 2;
	if (sc1 & 0x00ff0000)
		test |= 4;
	if (sc1 & 0xff000000)
		test |= 8;
	if (sc2 & 0x000000ff)
		test |= 16;
	if (sc2 & 0x0000ff00)
		test |= 32;
	if (sc2 & 0x00ff0000)
		test |= 64;
	if (sc2 & 0xff000000)
		test |= 128;

	MSG_WriteByte (&host_client->message, test);

	if (test & 1)
		MSG_WriteByte (&host_client->message, sc1 & 0xff);
	if (test & 2)
		MSG_WriteByte (&host_client->message, (sc1 >> 8) & 0xff);
	if (test & 4)
		MSG_WriteByte (&host_client->message, (sc1 >> 16) & 0xff);
	if (test & 8)
		MSG_WriteByte (&host_client->message, (sc1 >> 24) & 0xff);
	if (test & 16)
		MSG_WriteByte (&host_client->message, sc2 & 0xff);
	if (test & 32)
		MSG_WriteByte (&host_client->message, (sc2 >> 8) & 0xff);
	if (test & 64)
		MSG_WriteByte (&host_client->message, (sc2 >> 16) & 0xff);
	if (test & 128)
		MSG_WriteByte (&host_client->message, (sc2 >> 24) & 0xff);

	if (sc1 & SC1_HEALTH)
		MSG_WriteShort (&host_client->message, ent->v.health);
	if (sc1 & SC1_LEVEL)
		MSG_WriteByte(&host_client->message, ent->v.level);
	if (sc1 & SC1_INTELLIGENCE)
		MSG_WriteByte(&host_client->message, ent->v.intelligence);
	if (sc1 & SC1_WISDOM)
		MSG_WriteByte(&host_client->message, ent->v.wisdom);
	if (sc1 & SC1_STRENGTH)
		MSG_WriteByte(&host_client->message, ent->v.strength);
	if (sc1 & SC1_DEXTERITY)
		MSG_WriteByte(&host_client->message, ent->v.dexterity);
	if (sc1 & SC1_WEAPON)
		MSG_WriteByte (&host_client->message, ent->v.weapon);
	if (sc1 & SC1_BLUEMANA)
		MSG_WriteByte (&host_client->message, ent->v.bluemana);
	if (sc1 & SC1_GREENMANA)
		MSG_WriteByte (&host_client->message, ent->v.greenmana);
	if (sc1 & SC1_EXPERIENCE)
		MSG_WriteLong (&host_client->message, ent->v.experience);
	if (sc1 & SC1_CNT_TORCH)
		MSG_WriteByte (&host_client->message, ent->v.cnt_torch);
	if (sc1 & SC1_CNT_H_BOOST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_h_boost);
	if (sc1 & SC1_CNT_SH_BOOST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_sh_boost);
	if (sc1 & SC1_CNT_MANA_BOOST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_mana_boost);
	if (sc1 & SC1_CNT_TELEPORT)
		MSG_WriteByte (&host_client->message, ent->v.cnt_teleport);
	if (sc1 & SC1_CNT_TOME)
		MSG_WriteByte (&host_client->message, ent->v.cnt_tome);
	if (sc1 & SC1_CNT_SUMMON)
		MSG_WriteByte (&host_client->message, ent->v.cnt_summon);
	if (sc1 & SC1_CNT_INVISIBILITY)
		MSG_WriteByte (&host_client->message, ent->v.cnt_invisibility);
	if (sc1 & SC1_CNT_GLYPH)
		MSG_WriteByte (&host_client->message, ent->v.cnt_glyph);
	if (sc1 & SC1_CNT_HASTE)
		MSG_WriteByte (&host_client->message, ent->v.cnt_haste);
	if (sc1 & SC1_CNT_BLAST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_blast);
	if (sc1 & SC1_CNT_POLYMORPH)
		MSG_WriteByte (&host_client->message, ent->v.cnt_polymorph);
	if (sc1 & SC1_CNT_FLIGHT)
		MSG_WriteByte (&host_client->message, ent->v.cnt_flight);
	if (sc1 & SC1_CNT_CUBEOFFORCE)
		MSG_WriteByte (&host_client->message, ent->v.cnt_cubeofforce);
	if (sc1 & SC1_CNT_INVINCIBILITY)
		MSG_WriteByte (&host_client->message, ent->v.cnt_invincibility);
	if (sc1 & SC1_ARTIFACT_ACTIVE)
		MSG_WriteFloat (&host_client->message, ent->v.artifact_active);
	if (sc1 & SC1_ARTIFACT_LOW)
		MSG_WriteFloat (&host_client->message, ent->v.artifact_low);
	if (sc1 & SC1_MOVETYPE)
		MSG_WriteByte (&host_client->message, ent->v.movetype);
	if (sc1 & SC1_CAMERAMODE)
		MSG_WriteByte (&host_client->message, ent->v.cameramode);
	if (sc1 & SC1_HASTED)
		MSG_WriteFloat (&host_client->message, ent->v.hasted);
	if (sc1 & SC1_INVENTORY)
		MSG_WriteByte (&host_client->message, ent->v.inventory);
	if (sc1 & SC1_RINGS_ACTIVE)
		MSG_WriteFloat (&host_client->message, ent->v.rings_active);

	if (sc2 & SC2_RINGS_LOW)
		MSG_WriteFloat (&host_client->message, ent->v.rings_low);
	if (sc2 & SC2_AMULET)
		MSG_WriteByte(&host_client->message, ent->v.armor_amulet);
	if (sc2 & SC2_BRACER)
		MSG_WriteByte(&host_client->message, ent->v.armor_bracer);
	if (sc2 & SC2_BREASTPLATE)
		MSG_WriteByte(&host_client->message, ent->v.armor_breastplate);
	if (sc2 & SC2_HELMET)
		MSG_WriteByte(&host_client->message, ent->v.armor_helmet);
	if (sc2 & SC2_FLIGHT_T)
		MSG_WriteByte(&host_client->message, ent->v.ring_flight);
	if (sc2 & SC2_WATER_T)
		MSG_WriteByte(&host_client->message, ent->v.ring_water);
	if (sc2 & SC2_TURNING_T)
		MSG_WriteByte(&host_client->message, ent->v.ring_turning);
	if (sc2 & SC2_REGEN_T)
		MSG_WriteByte(&host_client->message, ent->v.ring_regeneration);
	if (sc2 & SC2_HASTE_T)
		MSG_WriteFloat(&host_client->message, ent->v.haste_time);
	if (sc2 & SC2_TOME_T)
		MSG_WriteFloat(&host_client->message, ent->v.tome_time);
	if (sc2 & SC2_PUZZLE1)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv1);
	if (sc2 & SC2_PUZZLE2)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv2);
	if (sc2 & SC2_PUZZLE3)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv3);
	if (sc2 & SC2_PUZZLE4)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv4);
	if (sc2 & SC2_PUZZLE5)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv5);
	if (sc2 & SC2_PUZZLE6)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv6);
	if (sc2 & SC2_PUZZLE7)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv7);
	if (sc2 & SC2_PUZZLE8)
		MSG_WriteString(&host_client->message, pr_strings+ent->v.puzzle_inv8);
	if (sc2 & SC2_MAXHEALTH)
		MSG_WriteShort(&host_client->message, ent->v.max_health);
	if (sc2 & SC2_MAXMANA)
		MSG_WriteByte(&host_client->message, ent->v.max_mana);
	if (sc2 & SC2_FLAGS)
		MSG_WriteFloat(&host_client->message, ent->v.flags);

	if (sc2 & SC2_OBJ)
	{
		MSG_WriteLong (&host_client->message, sv.info_mask);
		client->info_mask = sv.info_mask;
	}
	if (sc2 & SC2_OBJ2)
	{
		MSG_WriteLong (&host_client->message, sv.info_mask2);
		client->info_mask2 = sv.info_mask2;
	}
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
==================
SV_WriteClientdataToMessage
==================
*/
void SV_WriteClientdataToMessage (client_t *client, edict_t *ent, sizebuf_t *msg)
{
	int	bits, i, items;
	edict_t	*other;
	eval_t	*val;

// send a damage message
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = PROG_TO_EDICT(ent->v.dmg_inflictor);
		MSG_WriteCmd (msg, svc_damage);
		MSG_WriteByte (msg, ent->v.dmg_save);
		MSG_WriteByte (msg, ent->v.dmg_take);
		for (i=0 ; i<3 ; i++)
			MSG_WriteCoord (msg, other->v.origin[i] + 0.5 * (other->v.mins[i] + other->v.maxs[i]));

		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

// send the current viewpos offset from the view entity
	SV_SetIdealPitch ();		// how much to look up / down ideally

// a fixangle might get lost in a dropped packet. Oh well.
	if (ent->v.fixangle)
	{
/****JDH****/
		/*if (sv.found_cutscene && (client->protocol != PROTOCOL_VERSION_PLUS) && !sv_oldprotocol.value)
		{
			// step up to protocol+ so we can use 16-bit angles
			SV_ChangeToProtocol (msg, client, PROTOCOL_VERSION_PLUS);
		}

		if (client->protocol == PROTOCOL_VERSION_PLUS)
		{
			MSG_WriteCmd (msg, svc_setpreciseangle);
			for (i=0 ; i<3 ; i++)
				MSG_WritePreciseAngle (msg, ent->v.angles[i]);
		}
		else*/
/****JDH****/
		{
			MSG_WriteCmd (msg, svc_setangle);
			for (i=0 ; i<3 ; i++)
				MSG_WriteAngle (msg, ent->v.angles[i]);

		// JDH: store most recent viewangle before cutscene
			if (!client->in_cutscene)
				VectorCopy (ent->v.angles, client->prev_viewangles);
		}
		ent->v.fixangle = 0;
	}

	bits = 0;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (client->send_all_v)
		{
			bits = SU_VIEWHEIGHT | SU_IDEALPITCH | SU_IDEALROLL |
				   SU_VELOCITY1 | SU_VELOCITY2 | SU_VELOCITY3 |
				   SU_PUNCH1 | SU_PUNCH2 | SU_PUNCH3 | SU_WEAPONFRAME |
				   SU_ARMOR | SU_WEAPON | SU_ONGROUND;
		}
		else
		{
			if (ent->v.idealroll != client->old_v.idealroll)
				bits |= SU_IDEALROLL;

			if (ent->v.view_ofs[2] != client->old_v.view_ofs[2])
				bits |= SU_VIEWHEIGHT;
		}
	}
	else
#endif
	{
		if (ent->v.view_ofs[2] != DEFAULT_VIEWHEIGHT)
			bits |= SU_VIEWHEIGHT;

	// stuff the sigil bits into the high bits of items for sbar, or else mix in items2
		if ((val = GETEDICTFIELD(ent, eval_items2)))
			items = (int)ent->v.items | ((int)val->_float << 23);
		else
			items = (int)ent->v.items | ((int)PR_GLOBAL(serverflags) << 28);

		bits |= SU_ITEMS;
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (!client->send_all_v)
		{
			if (ent->v.idealpitch != client->old_v.idealpitch)
				bits |= SU_IDEALPITCH;

			for (i=0 ; i<3 ; i++)
			{
				if (ent->v.punchangle[i] != client->old_v.punchangle[i])
					bits |= (SU_PUNCH1<<i);

				if (ent->v.velocity[i] != client->old_v.velocity[i])
					bits |= (SU_VELOCITY1<<i);
			}

			if (ent->v.weaponframe != client->old_v.weaponframe)
				bits |= SU_WEAPONFRAME;

			if (ent->v.armorvalue != client->old_v.armorvalue)
				bits |= SU_ARMOR;

			if (ent->v.weaponmodel != client->old_v.weaponmodel)
				bits |= SU_WEAPON;
		}
	}
	else
#endif
	{
		if (ent->v.idealpitch)
			bits |= SU_IDEALPITCH;

		for (i=0 ; i<3 ; i++)
		{
			if (ent->v.punchangle[i])
				bits |= (SU_PUNCH1 << i);
			if (ent->v.velocity[i])
				bits |= (SU_VELOCITY1 << i);
		}

		if (ent->v.weaponframe)
			bits |= SU_WEAPONFRAME;

		if (ent->v.armorvalue)
			bits |= SU_ARMOR;

	//	if (ent->v.weapon)
			bits |= SU_WEAPON;
	}

	if (ent->v.waterlevel >= 2)
		bits |= SU_INWATER;

	if ((int)ent->v.flags & FL_ONGROUND)
		bits |= SU_ONGROUND;

// send the data
WRITEDATA_START:
	MSG_WriteCmd (msg, svc_clientdata);
	MSG_WriteShort (msg, bits);

	if (bits & SU_VIEWHEIGHT)
		MSG_WriteChar (msg, ent->v.view_ofs[2]);

	if (bits & SU_IDEALPITCH)
		MSG_WriteChar (msg, ent->v.idealpitch);

#ifdef HEXEN2_SUPPORT
	if (bits & SU_IDEALROLL)
		MSG_WriteChar (msg, ent->v.idealroll);
#endif

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1 << i))
			MSG_WriteChar (msg, ent->v.punchangle[i]);
		if (bits & (SU_VELOCITY1 << i))
			MSG_WriteChar (msg, ent->v.velocity[i]/16);
	}

	if (bits & SU_ITEMS)
		MSG_WriteLong (msg, items);

	if (bits & SU_WEAPONFRAME)
		MSG_WriteByte (msg, ent->v.weaponframe);

	if (bits & SU_ARMOR)
		MSG_WriteByte (msg, ent->v.armorvalue);

	if (bits & SU_WEAPON)
	{
		i = SV_ModelIndex (pr_strings + ent->v.weaponmodel, true);
		if (!MSG_WriteModelIndex (msg, i, client))
		{
			// if model index > 255 and sv.protocol is 15, all we can do
			//  is omit the weapon update
			msg->cursize = msg->lastcmdpos;
			bits &= ~SU_WEAPON;
			goto WRITEDATA_START;
		}
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		SV_SendUpdateInv (client, ent);
		memcpy (&client->old_v, &ent->v, sizeof(client->old_v));
		return;
	}
#endif

	MSG_WriteShort (msg, ent->v.health);
	MSG_WriteByte (msg, ent->v.currentammo);
	MSG_WriteByte (msg, ent->v.ammo_shells);
	MSG_WriteByte (msg, ent->v.ammo_nails);
	MSG_WriteByte (msg, ent->v.ammo_rockets);
	MSG_WriteByte (msg, ent->v.ammo_cells);

	if (hipnotic || rogue)
	{
		for (i=0 ; i<32 ; i++)
		{
			if (((int)ent->v.weapon) & (1<<i))
			{
				MSG_WriteByte (msg, i);
				break;
			}
		}
	}
	else
	{
		MSG_WriteByte (msg, ent->v.weapon);
	}
}

/*
=======================
SV_SendClientDatagram
=======================
*/
qboolean SV_SendClientDatagram (client_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;

	msg.data = buf;
	msg.cursize = 0;
	msg.lastcmdpos = 0;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		msg.maxsize = NET_MAXMESSAGE_H2;
	else
#endif
//	if (client->protocol == PROTOCOL_VERSION_STD)
	if (sv.protocol == PROTOCOL_VERSION_STD)
		msg.maxsize = MAX_DATAGRAM_OLD;
	else
		msg.maxsize = sizeof(buf);

	MSG_WriteCmd (&msg, svc_time);
	MSG_WriteFloat (&msg, sv.time);

// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client, client->edict, &msg);

// JDH: in case protocol has changed:
//	if (client->protocol != PROTOCOL_VERSION_STD)
//		msg.maxsize = sizeof(buf);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		SV_WriteEntitiesToClient_H2 (client, client->edict, &msg);
	else
#endif
		SV_WriteEntitiesToClient (client, client->edict, &msg);

// copy the server datagram if there is space
	if (SV_CheckDatagramSpace (&msg, sv.datagram.cursize))
		SZ_Write (&msg, sv.datagram.data, sv.datagram.cursize);

/*	if (msg.cursize + sv.datagram.cursize < msg.maxsize)
		SZ_Write (&msg, sv.datagram.data, sv.datagram.cursize);
	else
	{
	//	if ((msg.maxsize == MAX_DATAGRAM_OLD) && !sv_oldprotocol.value)
		if ((msg.maxsize == MAX_DATAGRAM_OLD) && (sv_protocol.value != PROTOCOL_VERSION_STD))
		{
		// set the buffer to its full size and try again:
			msg.maxsize = sizeof(buf);
			if (msg.cursize + sv.datagram.cursize < msg.maxsize)
				SZ_Write (&msg, sv.datagram.data, sv.datagram.cursize);
		}
	}
*/
// JDH: in case protocol has changed inside sv.datagram (via SV_StartSound):
//	if (sv.serverinfo_protocol == PROTOCOL_VERSION_BJP3)
//		client->protocol = PROTOCOL_VERSION_BJP3;

// send the datagram
	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
	{
		SV_DropClient (true);	// if the message couldn't send, kick off
		return false;
	}

	return true;
}

/*
=======================
SV_UpdateToReliableMessages
=======================
*/
void SV_UpdateToReliableMessages (void)
{
	int		i, j;
	client_t	*client;

// check for changes to be sent over the reliable streams
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (host_client->old_frags != host_client->edict->v.frags)
		{
			for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
			{
				if (!client->active)
					continue;
				MSG_WriteCmd (&client->message, svc_updatefrags);
				MSG_WriteByte (&client->message, i);
				MSG_WriteShort (&client->message, host_client->edict->v.frags);
			}

			host_client->old_frags = host_client->edict->v.frags;
		}
	}

	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		SZ_Write (&client->message, sv.reliable_datagram.data, sv.reliable_datagram.cursize);
	}

	SZ_Clear (&sv.reliable_datagram);
}


/*
=======================
SV_SendNop

Send a nop message without trashing or sending the accumulated client
message buffer
=======================
*/
void SV_SendNop (client_t *client)
{
	sizebuf_t	msg;
	byte		buf[4];

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;
	msg.lastcmdpos = 0;

	MSG_WriteCmd (&msg, svc_nop);

	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
		SV_DropClient (true);	// if the message couldn't send, kick off
	client->last_message = realtime;
}

/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	int			i;

// update frags, names, etc
	SV_UpdateToReliableMessages ();

// build individual updates
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

		if (host_client->spawned)
		{
			if (!SV_SendClientDatagram (host_client))
				continue;
		}
		else
		{
		// the player isn't totally in the game yet
		// send small keepalive messages if too much time has passed
		// send a full message when the next signon stage has been requested
		// some other message data (name changes, etc) may accumulate
		// between signon stages
			if (!host_client->sendsignon)
			{
				if (realtime - host_client->last_message > 5)
					SV_SendNop (host_client);
				continue;	// don't send out non-signon messages
			}
		}

		// joe: NAT fix from ProQuake
		if (host_client->netconnection->net_wait)
			continue;

		// check for an overflowed message. Should only happen on a very
		// fucked up connection that backs up a lot, then changes level
		if (host_client->message.overflowed)
		{
			SV_DropClient (true);
			host_client->message.overflowed = false;
			continue;
		}

		if (host_client->message.cursize || host_client->dropasap)
		{
			if (!NET_CanSendMessage (host_client->netconnection))
			{
//				I_Printf ("can't write\n");
				continue;
			}

			if (host_client->dropasap)
				SV_DropClient (false);	// went to another level
			else
			{
				if (NET_SendMessage (host_client->netconnection, &host_client->message) == -1)
					SV_DropClient (true);	// if the message couldn't send, kick off
				SZ_Clear (&host_client->message);
				host_client->last_message = realtime;
				host_client->sendsignon = false;
			}
		}
	}

// clear muzzle flashes
	SV_CleanupEnts ();
}


/*
==============================================================================

SERVER SPAWNING

==============================================================================
*/

/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (const char *name, qboolean crash)
{
	int	i;

	if (!name || !name[0])
		return 0;

	for (i=0 ; i<MAX_MODELS && sv.model_precache[i] ; i++)
		if (COM_FilenamesEqual(sv.model_precache[i], name))
			return i;

	if (i == MAX_MODELS || !sv.model_precache[i])
	{
		if (crash)
			Host_Error ("SV_ModelIndex: model %s not precached", name);
		return 0;
	}

	return i;
}

/*
================
SV_CreateBaseline
================
*/
void SV_CreateBaseline (void)
{
	int			msg_size, i, entnum;
	edict_t		*svent;

	for (entnum = 0 ; entnum < sv.num_edicts ; entnum++)
	{
	// get the current server version
		svent = EDICT_NUM(entnum);
		if (svent->free)
			continue;
		if (entnum > svs.maxclients && !svent->v.modelindex)
			continue;

	// create entity baseline
		VectorCopy (svent->v.origin, svent->baseline.origin);
		VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = svent->v.frame;
		svent->baseline.skin = svent->v.skin;

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			svent->baseline.scale = (int)(svent->v.scale*100.0) & 255;
			svent->baseline.drawflags = svent->v.drawflags;
			svent->baseline.abslight = (int)(svent->v.abslight*255.0)&255;
		}
	#endif

		if ((entnum > 0) && (entnum <= svs.maxclients))
		{
			svent->baseline.colormap = entnum;
			svent->baseline.modelindex = SV_ModelIndex ("progs/player.mdl", false);
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex = SV_ModelIndex (pr_strings + svent->v.model, true);
		}

		msg_size = (sv.protocol == PROTOCOL_VERSION_BJP3) ? 17 : 16;
				// BJP3 protocol uses an extra byte for the model index
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			msg_size += 3;
	#endif

		if (sv.signon.maxsize - sv.signon.cursize < msg_size)
		{
//			if ((sv.protocol == PROTOCOL_VERSION_STD) && !sv_oldprotocol.value)
			if ((sv.protocol == PROTOCOL_VERSION_STD) && (sv.desired_protocol != PROTOCOL_VERSION_STD))
			{
//				SV_ChangeToProtocol (&sv.signon, NULL, PROTOCOL_VERSION_BJP3);
				SV_ChangeToProtocol (&sv.signon, PROTOCOL_VERSION_BJP3);
			}
			else continue;
		}


	// add to the message
		MSG_WriteCmd (&sv.signon, svc_spawnbaseline);
		MSG_WriteShort (&sv.signon, entnum);

		if (MSG_WriteModelIndex (&sv.signon, svent->baseline.modelindex, NULL))
		{
			MSG_WriteByte (&sv.signon, svent->baseline.frame);
			MSG_WriteByte (&sv.signon, svent->baseline.colormap);
			MSG_WriteByte (&sv.signon, svent->baseline.skin);

		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				MSG_WriteByte (&sv.signon, svent->baseline.scale);
				MSG_WriteByte (&sv.signon, svent->baseline.drawflags);
				MSG_WriteByte (&sv.signon, svent->baseline.abslight);
			}
		#endif

			for (i=0 ; i<3 ; i++)
			{
				MSG_WriteCoord (&sv.signon, svent->baseline.origin[i]);
				MSG_WriteAngle (&sv.signon, svent->baseline.angles[i]);
			}
		}
		else
		{
			// undo the WriteCmd & WriteShort
			sv.signon.cursize -= 3;
		}
	}
}

/*
================
SV_SendReconnect

Tell all the clients that the server is changing levels
================
*/
void SV_SendReconnect (const char *newmap)
{
	byte		data[128];
	sizebuf_t	msg;
	char		cmd[MAX_QPATH+16];

	msg.data = data;
	msg.maxsize = sizeof(data);
	msg.cursize = 0;
	msg.lastcmdpos = 0;

	// JDH: added name of new map to reconnect command:
	Q_snprintfz (cmd, sizeof(cmd), "reconnect %s.bsp\n", newmap);

	MSG_WriteCmd (&msg, svc_stufftext);
	MSG_WriteString (&msg, cmd);
	NET_SendToAll (&msg, 5);

//	if (cls.state != ca_dedicated)
	if (!isDedicated)
		Cmd_ExecuteString (cmd, SRC_COMMAND);
}

/*
================
SV_SaveSpawnparms

Grabs the current state of each client for saving across the
transition to another level
================
*/
void SV_SaveSpawnparms (void)
{
	int		i, j;

	svs.serverflags = PR_GLOBAL(serverflags);

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

	// call the progs to get default spawn parms for the new client
	#ifdef HEXEN2_SUPPORT
		if (!hexen2)
	#endif
		{
			PR_GLOBAL(self) = EDICT_TO_PROG(host_client->edict);

			PR_ExecuteProgram (*pr_global_ptrs.SetChangeParms);

			for (j=0 ; j<NUM_SPAWN_PARMS ; j++)
				host_client->spawn_parms[j] = (pr_global_ptrs.parm1)[j];
		}
	}
}

/*
================
SV_SpawnServer

This is called at the start of each level
================
*/
//extern	float	scr_centertime_off;

#ifdef HEXEN2_SUPPORT
qboolean SV_SpawnServer (const char *server, const char *startspot)
#else
qboolean SV_SpawnServer (const char *server)
#endif
{
	edict_t		*ent;
	int			i;
	char		mapname[MAX_QPATH];
	byte		*entdata;

// JDH: check for existence of map file before doing anything else:
	Q_snprintfz (mapname, sizeof(mapname), "maps/%s.bsp", server);
	if (!COM_FindFile (mapname, 0, NULL))
	{
		Con_Printf ("Couldn't spawn server %s\n", mapname);
		return false;
	}

#ifndef RQM_SV_ONLY
	if (nehahra && (NehGameType == NEH_TYPE_DEMO))		// JDH: FIXME - should be client-side!
	{
		M_Menu_Main_f (SRC_COMMAND);
		return false;
	}
#endif

	// let's not have any servers with no name
	if (hostname.string[0] == 0)
		Cvar_SetDirect (&hostname, "UNNAMED");
//	scr_centertime_off = 0;			// JDH: moved to Host_Reconnect_f (client-side)

	Con_DPrintf ("SpawnServer: %s\n", server);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (svs.changelevel_issued)
		{
			Host_SaveGamestate (true);
		}
	}
	else
#endif
	svs.changelevel_issued = false;		// now safe to issue another

// tell all connected clients that we are going to a new level
	if (sv.active)
		SV_SendReconnect (server);

// make cvars consistant
	if (coop.value)
		Cvar_SetValueDirect (&deathmatch, 0);
	current_skill = (int)(skill.value + 0.5);
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		current_skill = bound(0, current_skill, 4);
	else
#endif
		current_skill = bound(0, current_skill, 3);

	Cvar_SetValueDirect (&skill, (float)current_skill);

// set up the new server
	Host_ClearMemory ();

	Q_strcpy (sv.name, server, sizeof(sv.name));
#ifdef HEXEN2_SUPPORT
	if (startspot)
		Q_strcpy (sv.startspot, startspot, sizeof(sv.startspot));
#endif

// load progs to get entity field count
#if defined(HEXEN2_SUPPORT) && !defined(RQM_SV_ONLY)
	if (hexen2)
	{
		total_loading_size = 100;
		current_loading_size = 0;
		loading_stage = 1;
	}
#endif

	PR_LoadProgs ();

#if defined(HEXEN2_SUPPORT) && !defined(RQM_SV_ONLY)
	if (hexen2 && !isDedicated)
	{
		current_loading_size += 15;
		SCR_ShowLoadingSize ();
	}
#endif

// allocate server memory
	sv.max_edicts = MAX_EDICTS;
	sv.edicts = Hunk_AllocName (sv.max_edicts * pr_edict_size, "edicts");


/*
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		sv_currentprotocol = PROTOCOL_VERSION_H2_112;
	else
#endif
	if (sv_oldprotocol.value)
		sv_currentprotocol = PROTOCOL_VERSION_STD;
	else
		sv_currentprotocol = PROTOCOL_VERSION_PLUS;

	if (sv_oldprotocol.value)
	{
		// use original (smaller) buffer sizes, for compatibility
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			sv.datagram.maxsize = 16384;
			sv.reliable_datagram.maxsize = 16384;
			sv.signon.maxsize = 16384;
		}
		else
	#endif
		{
			sv.datagram.maxsize = MAX_DATAGRAM_OLD;
			sv.reliable_datagram.maxsize = MAX_DATAGRAM_OLD;
			sv.signon.maxsize = NET_MAXMESSAGE_OLD;
		}
	}
	else
	{
		sv.datagram.maxsize = sizeof(sv.datagram_buf);
		sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
		sv.signon.maxsize = sizeof(sv.signon_buf);
	}
*/

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		sv.protocol = PROTOCOL_VERSION_H2_112;
		sv.desired_protocol = sv.protocol;

		sv.datagram.maxsize = NET_MAXMESSAGE_H2;
		sv.reliable_datagram.maxsize = NET_MAXMESSAGE_H2;
		sv.signon.maxsize = NET_MAXMESSAGE_H2;
	}
	else
#endif
	{
		sv.desired_protocol = sv_protocol.value;
		if (sv_protocol.value == PROTOCOL_VERSION_BJP3)
		{
			sv.protocol = PROTOCOL_VERSION_BJP3;

			sv.datagram.maxsize = sizeof(sv.datagram_buf);
			sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
			sv.signon.maxsize = sizeof(sv.signon_buf);
		}
		else
		{
			sv.protocol = PROTOCOL_VERSION_STD;

			sv.datagram.maxsize = MAX_DATAGRAM_OLD;
			sv.reliable_datagram.maxsize = MAX_DATAGRAM_OLD;
			sv.signon.maxsize = NET_MAXMESSAGE_OLD;
		}
	}

	sv.datagram.cursize = 0;
	sv.datagram.lastcmdpos = 0;
	sv.datagram.data = sv.datagram_buf;

	sv.reliable_datagram.cursize = 0;
	sv.reliable_datagram.lastcmdpos = 0;
	sv.reliable_datagram.data = sv.reliable_datagram_buf;

	sv.signon.cursize = 0;
	sv.signon.lastcmdpos = 0;
	sv.signon.data = sv.signon_buf;

// leave slots at start for clients only
	sv.num_edicts = svs.maxclients + 1;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		sv.num_edicts += max_temp_edicts.value;

		memset(sv.effects, 0, sizeof(sv.effects));

		sv.states = Hunk_AllocName (svs.maxclients * sizeof(client_state2_t), "states");
		memset(sv.states, 0, svs.maxclients * sizeof(client_state2_t));

		for (i=0 ; i < max_temp_edicts.value ; i++)
		{
			ent = EDICT_NUM(i+svs.maxclients+1);
			ED_ClearEdict(ent);

			ent->free = true;
			ent->freetime = -999;
		}
	}
#endif

	for (i=0 ; i<svs.maxclients ; i++)
	{
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;
	#ifdef HEXEN2_SUPPORT
		svs.clients[i].send_all_v = true;
	#endif
	}

	sv.state = ss_loading;
	sv.paused = false;

	sv.time = 1.0;
//	sv.found_cutscene = false;

//JDH: server shouldn't be calling client code directly!
//	R_PreMapLoad (server);		// joe
	Host_SetMapName (server);

/*******JDH*******/
	Con_DPrintf ("  server: loading world model...\n");
/*******JDH*******/

	Q_strcpy (sv.name, server, sizeof(sv.name));
	Q_snprintfz (sv.modelname, sizeof(sv.modelname), "maps/%s.bsp", server);
	if (!(sv.worldmodel = Mod_ForName(sv.modelname, false)))
	{
		Con_Printf ("Couldn't spawn server %s\n", sv.modelname);
		sv.active = false;
	#if defined(HEXEN2_SUPPORT) && !defined(RQM_SV_ONLY)
		total_loading_size = 0;
		loading_stage = 0;
	#endif
		return false;
	}

	if (sv.worldmodel->numsubmodels == -1)	// HACK - signifies incorrect BSP version
	{
		Con_Printf ("Couldn't spawn server %s\n", sv.modelname);
		sv.active = false;
		return false;
	}

	sv.models[1] = sv.worldmodel;

// clear world interaction links
	SV_ClearWorld ();

	sv.sound_precache[0] = pr_strings;

	Con_DPrintf ("  server: loading models...\n");		// JDH

	sv.model_precache[0] = pr_strings;
	sv.model_precache[1] = sv.modelname;
	for (i=1 ; i<sv.worldmodel->numsubmodels ; i++)
	{
		sv.model_precache[i+1] = localmodels[i];
		sv.models[i+1] = Mod_ForName (localmodels[i], false);
	}

// check player/eyes models for hacks
	if (pq_cheatfree)		// JDH: added condition
	{
		sv.player_model_crc = Mod_CalcCRC ("progs/player.mdl");
		sv.eyes_model_crc = Mod_CalcCRC ("progs/eyes.mdl");
	}

// load the rest of the entities
	ent = EDICT_NUM(0);
	memset (&ent->v, 0, progs->entityfields * 4);
	ent->free = false;
	ent->v.model = sv.worldmodel->name - pr_strings;
	ent->v.modelindex = 1;		// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop.value)
		PR_GLOBAL(coop) = coop.value;
	else
		PR_GLOBAL(deathmatch) = deathmatch.value;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		*pr_global_ptrs.randomclass = randomclass.value;
		*pr_global_ptrs.startspot = sv.startspot - pr_strings;

	#ifndef RQM_SV_ONLY
		if (!isDedicated)
		{
			current_loading_size += 5;
			SCR_ShowLoadingSize ();
		}
	#endif
	}
#endif

	PR_GLOBAL(mapname) = sv.name - pr_strings;

	// serverflags are for cross level information (sigils)
	PR_GLOBAL(serverflags) = svs.serverflags;

	if (sv_entpatch.value && (entdata = COM_LoadMallocFile(va("maps/%s.ent", sv.name), 0)))
	{
		Con_Printf ("  Loading entities from maps/%s.ent\n", sv.name);
		PR_LoadEdicts ((const char *) entdata);
		free (entdata);
	}
	else
	{
		Con_DPrintf( "  Loading entities...\n" );
		PR_LoadEdicts (sv.worldmodel->entities);
	}

/*
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		sv_currentprotocol = PROTOCOL_VERSION_H2_112;
	else
#endif
	{
		for (i = 0; i < MAX_MODELS; i++)
			if (!sv.model_precache[i])
				break;

		if (sv_oldprotocol.value)
		{
			sv_currentprotocol = PROTOCOL_VERSION_STD;
			if (i-1 >= MAX_MODELS_OLD)
				Con_DPrintf ("MAX_MODELS exceeded; using standard protocol (sv_oldprotocol is 1)\n");
		}
		else
		{
			if (i-1 < MAX_MODELS_OLD)
				sv_currentprotocol = PROTOCOL_VERSION_STD;
			else
			{
				Con_DPrintf ("MAX_MODELS exceeded; using extended protocol\n");
				sv_currentprotocol = PROTOCOL_VERSION_PLUS;
			}
		}
	}
*/
/*
#ifdef HEXEN2_SUPPORT
	if (hexen2 && sv_oldprotocol.value)
	{
		// use original (smaller) buffer sizes, for compatibility
		sv.datagram.maxsize = 16384;
		sv.reliable_datagram.maxsize = 16384;
		sv.signon.maxsize = 16384;
	}
	else
#endif
	if (sv_currentprotocol == PROTOCOL_VERSION_STD)
	{
		// use original (smaller) buffer sizes, for compatibility
		sv.datagram.maxsize = MAX_DATAGRAM_OLD;
		sv.reliable_datagram.maxsize = MAX_DATAGRAM_OLD;
		sv.signon.maxsize = NET_MAXMESSAGE_OLD;
	}
	else
	{
		sv.datagram.maxsize = sizeof(sv.datagram_buf);
		sv.reliable_datagram.maxsize = sizeof(sv.reliable_datagram_buf);
		sv.signon.maxsize = sizeof(sv.signon_buf);
	}
*/
	sv.active = true;

// all setup is completed, any further precache statements are errors
	sv.state = ss_active;

// run two frames to allow everything to settle
	host_frametime = 0.1;
	SV_Physics ();
	SV_Physics ();

// create a baseline for more efficient communications
	SV_CreateBaseline ();

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	{
	//	if (!sv_oldprotocol.value)
		if ((sv.protocol != PROTOCOL_VERSION_BJP3) && (sv.desired_protocol != PROTOCOL_VERSION_STD))
		{
			int num_models, num_sounds;

		// count the number of precached models & sounds
			for (num_models = 1; sv.model_precache[num_models]; num_models++) ;
			for (num_sounds = 1; sv.sound_precache[num_sounds]; num_sounds++) ;

			if ((num_models > MAX_MODELS_OLD) || (num_sounds > MAX_SOUNDS_OLD))
			{
				sv.protocol = PROTOCOL_VERSION_BJP3;
			}
		}
	}

 // send serverinfo to all connected clients
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		if (host_client->active)
			SV_SendServerinfo (host_client);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		svs.changelevel_issued = false;		// now safe to issue another
#endif

	Con_DPrintf ("Server spawned\n");

#if defined(HEXEN2_SUPPORT) && !defined(RQM_SV_ONLY)
	total_loading_size = 0;
	loading_stage = 0;
#endif

	return true;
}
