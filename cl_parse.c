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
// cl_parse.c -- parse a message received from the server

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#define NUM_SVC (svc_updatepl+1)

static const char *svc_strings[NUM_SVC] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",			// [long] server version
	"svc_setview",			// [short] entity number
	"svc_sound",			// <see code>
	"svc_time",				// [float] server time
	"svc_print",			// [string] null terminated string
	"svc_stufftext",		// [string] stuffed into client's console buffer
							//		the string should be \n terminated
	"svc_setangle",			// [vec3] set the view angle to this absolute value

	"svc_serverinfo",		// [long] version
							//		[string] signon string
							//		[string]..[0]model cache [string]...[0]sounds cache
							//		[string]..[0]item cache
	"svc_lightstyle",		// [byte] [string]
	"svc_updatename",		// [byte] [string]
	"svc_updatefrags",		// [byte] [short]
	"svc_clientdata",		// <shortbits + data>
	"svc_stopsound",		// <see code>
	"svc_updatecolors",		// [byte] [byte]
	"svc_particle",			// [vec3] <variable>
	"svc_damage",			// [byte] impact [byte] blood [vec3] from
	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",
	"svc_temp_entity",		// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",			// [string] music [string] text
	"svc_cdtrack",			// [byte] track [byte] looptrack
	"svc_sellscreen",
	"svc_cutscene",
// nehahra support begin
	"svc_showlmp",			// [string] iconlabel [string] lmpfile [byte] x [byte] y
	"svc_hidelmp",			// [string] iconlabel
	"svc_skybox",			// [string] skyname
// nehahra support end
	"?",
	"?",
	"?",
	"svc_fog_fitz",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"?",
	"svc_skyboxsize",
	"svc_fog_neh",
	"?",
	"?"
};

static const char *svc_strings_QW[NUM_SVC];

#ifdef HEXEN2_SUPPORT
static const char *svc_strings_H2[NUM_SVC];

extern qboolean intro_playing;
extern cvar_t	bgmtype, sv_flypitch, sv_walkpitch;
extern int		sv_kingofhill;
extern int		total_loading_size, current_loading_size, loading_stage;
#endif

extern cvar_t	host_cutscenehack;
/*
const char *hipnotic_models[] =
{
	"progs/armabody.mdl",
	"progs/armalegs.mdl",
	"progs/empathy.mdl",
	"progs/g_hammer.mdl",
	"progs/g_laserg.mdl",
	"progs/g_prox.mdl",
	"progs/grem.mdl",
	"progs/h_grem.mdl",
	"progs/h_scourg.mdl",
	"progs/horn.mdl",
	"progs/lasrspik.mdl",
	"progs/lavarock.mdl",
	"progs/playham.mdl",
	"progs/proxbomb.mdl",
	"progs/rubble1.mdl",
	"progs/rubble2.mdl",
	"progs/rubble3.mdl",
	"progs/scor.mdl",
	"progs/spikmine.mdl",
	"progs/v_hammer.mdl",
	"progs/v_laserg.mdl",
	"progs/v_prox.mdl",
	"progs/wetsuit.mdl",
	NULL
};
*/
//=============================================================================

void CL_InitModelnames (void)
{
	int	i;

	memset (cl_modelnames, 0, sizeof(cl_modelnames));

	cl_modelnames[mi_player] = "progs/player.mdl";
	cl_modelnames[mi_h_player] = "progs/h_player.mdl";
	cl_modelnames[mi_eyes] = "progs/eyes.mdl";
	cl_modelnames[mi_rocket] = "progs/missile.mdl";
	cl_modelnames[mi_grenade] = "progs/grenade.mdl";
	cl_modelnames[mi_spike] = "progs/spike.mdl";
	cl_modelnames[mi_explo1] = "progs/s_expl.spr";
	cl_modelnames[mi_explo2] = "progs/s_explod.spr";
	cl_modelnames[mi_bubble] = "progs/s_bubble.spr";
	cl_modelnames[mi_sng] = "progs/v_nail2.mdl";		//JDH

	cl_modelnames[mi_flame0] = "progs/flame0.mdl";
	cl_modelnames[mi_flame1] = "progs/flame.mdl";
	cl_modelnames[mi_flame2] = "progs/flame2.mdl";

	cl_modelnames[mi_gib1] = "progs/gib1.mdl";
	cl_modelnames[mi_gib2] = "progs/gib2.mdl";
	cl_modelnames[mi_gib3] = "progs/gib3.mdl";

	cl_modelnames[mi_fish] = "progs/fish.mdl";
	cl_modelnames[mi_dog] = "progs/dog.mdl";
	cl_modelnames[mi_soldier] = "progs/soldier.mdl";
	cl_modelnames[mi_enforcer] = "progs/enforcer.mdl";
	cl_modelnames[mi_knight] = "progs/knight.mdl";
	cl_modelnames[mi_hknight] = "progs/hknight.mdl";
	cl_modelnames[mi_scrag] = "progs/wizard.mdl";
	cl_modelnames[mi_ogre] = "progs/ogre.mdl";
	cl_modelnames[mi_fiend] = "progs/demon.mdl";
	cl_modelnames[mi_vore] = "progs/shalrath.mdl";
	cl_modelnames[mi_shambler] = "progs/shambler.mdl";

/* moved to alias loading code
	cl_modelnames[mi_h_dog] = "progs/h_dog.mdl";
	cl_modelnames[mi_h_soldier] = "progs/h_guard.mdl";
	cl_modelnames[mi_h_enforcer] = "progs/h_mega.mdl";
	cl_modelnames[mi_h_knight] = "progs/h_knight.mdl";
//	cl_modelnames[mi_h_hknight] = "progs/h_hknight.mdl";
	cl_modelnames[mi_h_hknight] = "progs/h_hellkn.mdl";
	cl_modelnames[mi_h_scrag] = "progs/h_wizard.mdl";
	cl_modelnames[mi_h_ogre] = "progs/h_ogre.mdl";
	cl_modelnames[mi_h_fiend] = "progs/h_demon.mdl";
	cl_modelnames[mi_h_vore] = "progs/h_shal.mdl";
	cl_modelnames[mi_h_shambler] = "progs/h_shams.mdl";
	cl_modelnames[mi_h_zombie] = "progs/h_zombie.mdl";
*/
	for (i=0 ; i<NUM_MODELINDEX ; i++)
	{
		if (!cl_modelnames[i])
			Host_Error ("cl_modelnames[%d] not initialized", i);		// JDH: was Sys_Error
	}
}

void CL_InitMessageStrings (void)
{
	int i;

	for (i=0; i < NUM_SVC; i++)
	{
		svc_strings_QW[i] = svc_strings[i];
#ifdef HEXEN2_SUPPORT
		svc_strings_H2[i] = svc_strings[i];
#endif
	}

	svc_strings_QW[svc_smallkick] = "svc_smallkick";
	svc_strings_QW[svc_bigkick] = "svc_bigkick";
	svc_strings_QW[svc_updateping] = "svc_updateping";
	svc_strings_QW[svc_updateentertime] = "svc_updateentertime";
	svc_strings_QW[svc_updatestatlong] = "svc_updatestatlong";
	svc_strings_QW[svc_muzzleflash] = "svc_muzzleflash";
	svc_strings_QW[svc_updateuserinfo] = "svc_updateuserinfo";
	svc_strings_QW[svc_download] = "svc_download";
	svc_strings_QW[svc_playerinfo] = "svc_playerinfo";
	svc_strings_QW[svc_nails] = "svc_nails";
	svc_strings_QW[svc_chokecount] = "svc_chokecount";
	svc_strings_QW[svc_modellist] = "svc_modellist";
	svc_strings_QW[svc_soundlist] = "svc_soundlist";
	svc_strings_QW[svc_packetentities] = "svc_packetentities";
	svc_strings_QW[svc_deltapacketentities] = "svc_deltapacketentities";
	svc_strings_QW[svc_maxspeed] = "svc_maxspeed";
	svc_strings_QW[svc_entgravity] = "svc_entgravity";
	svc_strings_QW[svc_setinfo] = "svc_setinfo";
	svc_strings_QW[svc_serverinfo_qw] = "svc_serverinfo_qw";
	svc_strings_QW[svc_updatepl] = "svc_updatepl";

#ifdef HEXEN2_SUPPORT
	svc_strings_H2[svc_raineffect] = "svc_raineffect";
	svc_strings_H2[svc_particle2] = "svc_particle2";
	svc_strings_H2[svc_cutscene_H2] = "svc_cutscene_H2";
	svc_strings_H2[svc_midi_name] = "svc_midi_name";
	svc_strings_H2[svc_updateclass] = "svc_updateclass";
	svc_strings_H2[svc_particle3] = "svc_particle3";
	svc_strings_H2[svc_particle4] = "svc_particle4";
	svc_strings_H2[svc_set_view_flags] = "svc_set_view_flags";
	svc_strings_H2[svc_clear_view_flags] = "svc_clear_view_flags";
	svc_strings_H2[svc_start_effect] = "svc_start_effect";
	svc_strings_H2[svc_end_effect] = "svc_end_effect";
	svc_strings_H2[svc_plaque] = "svc_plaque";
	svc_strings_H2[svc_particle_explosion] = "svc_particle_explosion";
	svc_strings_H2[svc_set_view_tint] = "svc_set_view_tint";
	svc_strings_H2[svc_reference] = "svc_reference";
	svc_strings_H2[svc_clear_edicts] = "svc_clear_edicts";
	svc_strings_H2[svc_update_inv] = "svc_update_inv";
	svc_strings_H2[svc_setangle_interpolate] = "svc_setangle_interpolate";
	svc_strings_H2[svc_update_kingofhill] = "svc_update_kingofhill";
	svc_strings_H2[svc_toggle_statbar] = "svc_toggle_statbar";
	svc_strings_H2[svc_sound_update_pos] = "svc_sound_update_pos";
#endif
}

void CL_InitStrings (void)
{
	CL_InitModelnames ();
	CL_InitMessageStrings ();
}

/*
===============
CL_EntityNum

This error checks and tracks the total number of entities
===============
*/
entity_t *CL_EntityNum (int num)
{
	if (num < 0)		// JDH
		goto BADENTNUM;

	if (num >= cl.num_entities)
	{
		if (num >= MAX_EDICTS)
			goto BADENTNUM;

		while (cl.num_entities <= num)
		{
			cl_entities[cl.num_entities].colormap = vid.colormap;
			cl.num_entities++;
		}
	}

	return &cl_entities[num];

BADENTNUM:
	if (!cls.demoplayback)
		Host_Error ("CL_EntityNum: %i is an invalid number", num);

	Con_DPrintf ("\x02""Warning: invalid entity number %i; skipping to next message\n", num);
	return NULL;
}

/*
==================
CL_ParseStartSoundPacket
==================
*/
qboolean CL_ParseStartSoundPacket (qboolean parse_only)
{
	vec3_t	pos;
	int	i, channel, ent, sound_num, volume, field_mask;
	float	attenuation;

	if (cl.protocol != PROTOCOL_VERSION_QW)
	{
		field_mask = MSG_ReadByte ();

		volume = (field_mask & SND_VOLUME) ? MSG_ReadByte() : DEFAULT_SOUND_PACKET_VOLUME;
		attenuation = (field_mask & SND_ATTENUATION) ? MSG_ReadByte() / 64.0 : DEFAULT_SOUND_PACKET_ATTENUATION;
	}

	if ((cl.protocol == PROTOCOL_VERSION_FITZ) && (field_mask & SND_LARGEENTITY))
	{
		ent = (unsigned short) MSG_ReadShort ();
		channel = MSG_ReadByte ();
	}
	else
	{
		channel = (unsigned short) MSG_ReadShort ();
		ent = channel >> 3;

		if (cl.protocol == PROTOCOL_VERSION_QW)
		{
			ent &= 0x03FF;
			if (channel & 0x8000)
				volume = MSG_ReadByte ();
			else
				volume = DEFAULT_SOUND_PACKET_VOLUME;

			if (channel & 0x4000)
				attenuation = MSG_ReadByte () / 64.0;
			else
				attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;
		}

		channel &= 7;
	}

	if (ent > MAX_EDICTS)
	{
		if (cls.demoplayback)		// JDH
		{
			Con_DPrintf ("\x02""Warning: invalid entity %i for svc_sound\n", ent);
			return false;
		}

		Host_Error ("CL_ParseStartSoundPacket: ent = %i", ent);
	}

	if (((cl.protocol > PROTOCOL_VERSION_BJP) && (cl.protocol <= PROTOCOL_VERSION_BJP3)) ||
		((cl.protocol == PROTOCOL_VERSION_FITZ) && (field_mask & SND_LARGESOUND)))
		sound_num = (unsigned short) MSG_ReadShort();
	else
		sound_num = MSG_ReadByte ();

#ifdef HEXEN2_SUPPORT
    if (hexen2 && (field_mask & SND_OVERFLOW))
		sound_num += 255;
#endif

	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();

	if (!parse_only)
		S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation);

	return true;
}

/*
==================
CL_KeepaliveMessage

When the client is taking a long time to load stuff, send keepalive messages
so the server doesn't disconnect.
==================
*/
void CL_KeepaliveMessage (void)
{
	float		time;
	static float	lastmsg;
	int			ret;
	sizebuf_t	old;
	byte		olddata[NET_MAXMESSAGE];		// JDH: was [8192]

	if (sv.active)
		return;		// no need if server is local
	if (cls.demoplayback)
		return;

// read messages from server, should just be nops
	old = net_message;
	memcpy (olddata, net_message.data, net_message.cursize);

	do {
		ret = CL_GetMessage ();
		switch (ret)
		{
		default:
			Host_Error ("CL_KeepaliveMessage: CL_GetMessage failed");

		case 0:
			break;	// nothing waiting

		case 1:
			Host_Error ("CL_KeepaliveMessage: received a message");
			break;

		case 2:
			if (MSG_ReadByte() != svc_nop)
				Host_Error ("CL_KeepaliveMessage: datagram wasn't a nop");
			break;
		}
	} while (ret);

	net_message = old;
	memcpy (net_message.data, olddata, net_message.cursize);

// check time
	time = Sys_DoubleTime ();
	if (time - lastmsg < 5)
		return;
	lastmsg = time;

// write out a nop
	Con_Print ("--> client to server keepalive\n");

	MSG_WriteByte (&cls.message, clc_nop);
	NET_SendMessage (cls.netcon, &cls.message);
	SZ_Clear (&cls.message);
}

/*
==================
CL_IsKnownProtocol
==================
*/
qboolean CL_IsKnownProtocol (int *prot)
{
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
	// Hexen II v1.03, v1.07 use same protocol version as Quake, but with 2-byte models
		return ((*prot == PROTOCOL_VERSION_STD) || (*prot == PROTOCOL_VERSION_H2_111) ||
				(*prot == PROTOCOL_VERSION_H2_112));
	}
#endif

	if ((*prot == PROTOCOL_VERSION_STD) || (*prot == PROTOCOL_VERSION_FITZ) ||
		   ((*prot >= PROTOCOL_VERSION_BJP) && (*prot <= PROTOCOL_VERSION_BJP3)))
		   return true;

//	if (*prot == PROTOCOL_VERSION_DP7)
//		return true;

	if (cls.demoplayback)
	{
		if ((*prot >= PROTOCOL_VERSION_QW-2) && (*prot <= PROTOCOL_VERSION_QW))
		{
			*prot = PROTOCOL_VERSION_QW;
			return true;
		}

		if (*prot == PROTOCOL_VERSION_BETA)
			return true;
	}

	return false;
}

qboolean cl_precache_changed;
char	sound_precache[MAX_SOUNDS][MAX_QPATH];

/*
==================
CL_ParseSoundlist
==================
*/
int CL_ParseSoundlist (int startnum)
{
	int numsounds;
	const char *str;

	for (numsounds=startnum ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;

		//if (!cl_demoseek)
		{
			if (numsounds == MAX_SOUNDS)
				Host_Error ("Server sent too many sound precaches");

			if (cl_demoseek && !cl_precache_changed && sound_precache[numsounds][0])
				cl_precache_changed = !COM_FilenamesEqual (sound_precache[numsounds], str);

			Q_strcpy (sound_precache[numsounds], str, sizeof(sound_precache[numsounds]));
			S_TouchSound (str);
		}
	}

	return numsounds;
}

/*
==================
CL_PrecacheSounds
==================
*/
void CL_PrecacheSounds (int startnum, int numsounds)
{
	int i;

/*******JDH*******/
//	Con_DPrintf( "  client: loading sounds...\n" );
/*******JDH*******/

	S_BeginPrecaching ();
	for (i=startnum ; i<numsounds ; i++)
	{
		cl.sound_precache[i] = S_PrecacheSound (sound_precache[i]);
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			current_loading_size++;
			SCR_ShowLoadingSize ();
		}
	#endif
		CL_KeepaliveMessage ();
	}
	S_EndPrecaching ();
}

char	model_precache[MAX_MODELS][MAX_QPATH];
/*
==================
CL_ParseModellist
==================
*/
int CL_ParseModellist (int startnum)
{
	int nummodels, i;
	const char *str;

// first we go through and touch all of the precache data that still
// happens to be in the cache, so precaching something else doesn't
// needlessly purge it

	for (nummodels=startnum ; ; nummodels++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;

		//if (!cl_demoseek)
		{
			if (nummodels == MAX_MODELS)
				Host_Error ("Server sent too many model precaches");

			if (cl_demoseek && !cl_precache_changed && model_precache[nummodels][0])
				cl_precache_changed = !COM_FilenamesEqual (model_precache[nummodels], str);

			Q_strcpy (model_precache[nummodels], str, sizeof(model_precache[nummodels]));
			Mod_TouchModel (str);

			for (i=0 ; i<NUM_MODELINDEX ; i++)
			{
				if (!strcmp(cl_modelnames[i], str))
				{
					cl_modelindex[i] = nummodels;
					break;
				}
			}
		}
	}

	return nummodels;
}

extern qboolean COM_FindSearchpath (const char *dir);
extern qboolean Mod_LoadModelFile (const char *name, void *buffer, int bufsize);

/*
==================
CL_ModelInList
==================
*/
qboolean CL_ModelInList (const char *name, const char *mlist[])
{
	int i;

	for (i = 0; mlist[i]; i++)
	{
		if (COM_FilenamesEqual(name, mlist[i]))
			return true;
	}

	return false;
}

/*
==================
CL_LocateModels
==================
*/
/*void CL_LocateModels (int startnum, int nummodels)
{
	qboolean hipnotic_canload, quoth_canload;
	qboolean hipnotic_needed = false;
	int i;

	hipnotic_canload = (!COM_FindSearchpath ("hipnotic") && Sys_FolderExists (va("%s/hipnotic", com_basedir)));
	quoth_canload = (!COM_FindSearchpath ("quoth") && Sys_FolderExists (va("%s/quoth", com_basedir)));

	if (!hipnotic_canload && !quoth_canload)
		return;

	startnum = max(startnum, 2);		// skip bsp
	for (i=startnum ; i<nummodels ; i++)
	{
		if (model_precache[i][0] == '*')
			continue;
		if (Mod_LoadModelFile (model_precache[i], NULL, 0))
			continue;

		if (hipnotic_canload)
		{
			if (CL_ModelInList(model_precache[i], hipnotic_models))
				hipnotic_needed = true;
		}
	}
}
*/
/*
==================
CL_PrecacheModels
==================
*/
void CL_PrecacheModels (int startnum, int nummodels)
{
	char	mapname[MAX_QPATH];
	int i;

	if (startnum == 1)
	{
		// by joe
		COM_StripExtension (COM_SkipPath(model_precache[1]), mapname, sizeof(mapname));
		Host_SetMapName (mapname);

	/*******JDH*******/
	//	Con_DPrintf( "  client: loading models...\n" );
	/*******JDH*******/
	}

//	CL_LocateModels (startnum, nummodels);

	// now we try to load everything else until a cache allocation fails
	for (i=startnum ; i<nummodels ; i++)
	{
		cl.model_precache[i] = Mod_ForName (model_precache[i], false);
		if (!cl.model_precache[i])
		{
			Con_Printf ("\x02""WARNING: Couldn't load %s\n", model_precache[i]);
			if ((i == 1) || (model_precache[i][0] == '*'))
			{
				Host_EndGame ("Map load failed\n");
				return;
			}
		}

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			current_loading_size++;
			SCR_ShowLoadingSize ();
		}
	#endif

		CL_KeepaliveMessage ();
	}

	if (cl.protocol == PROTOCOL_VERSION_QW)
	{
		if (MSG_ReadByte())
			return;			// more models coming
	}

// local state
	cl_entities[0].model = cl.worldmodel = cl.model_precache[1];
	cl_entities[0].fullbright = 0;

	noclip_anglehack = false;	// noclip is turned off at start

	if (cl.protocol == PROTOCOL_VERSION_QW)
	{
		R_NewMap ();
		Hunk_Check ();			// make sure nothing is hurt
	}
}

extern float cl_demo_starttime, cl_demo_endtime;

/*
==================
CL_ParseServerInfo
==================
*/
void CL_ParseServerInfo (void)
{
	char	*str;
	int		i, vers, nummodels, numsounds/*, maxsounds*/;

	if (!cl_demoseek)
	{
		if (!sv.active)
			Con_Printf ("\n");		// kludge for demos/servers that don't include \n after server version string
		Con_DPrintf ("Serverinfo packet received\n");

	// wipe the client_state_t struct
		CL_ClearState ();
	}

// parse protocol version number
	vers = MSG_ReadLong ();
	if ((vers == PROTOCOL_VERSION_FTE) && cls.demoplayback)
	{
		MSG_ReadLong ();		// extensions
		vers = MSG_ReadLong ();
	}

	if (!CL_IsKnownProtocol (&vers))
	{
		Host_Error ("CL_ParseServerInfo: Server is using unknown protocol %i", vers);
//		Con_Printf ("Server is using unknown protocol %i\n", vers);
//		msg_badread = true;
		return;
	}

	cl.protocol = vers;

	if (!cl_demoseek)
	{
	#ifdef HEXEN2_SUPPORT
		if (!hexen2)
	#endif
		if (vers != PROTOCOL_VERSION_STD)
		{
			if (cls.demoplayback)
				Con_Printf ("Playing protocol %d demo ", vers);
			else
				Con_Printf ("Using protocol %d ", vers);

			switch (vers)
			{
			case PROTOCOL_VERSION_QW:
				Con_Print ("(QuakeWorld)");
				break;
			case PROTOCOL_VERSION_BJP:
			case PROTOCOL_VERSION_BJP2:
			case PROTOCOL_VERSION_BJP3:
				Con_Printf ("(BJP%d)", vers-PROTOCOL_VERSION_BJP+1);
				break;
			case PROTOCOL_VERSION_FITZ:
				Con_Print ("(Fitz)");
				break;
			case PROTOCOL_VERSION_BETA:
				Con_Print ("(Quake BETA)");
				break;
			}

			Con_Print ("\n");
		}

		cl_demo_starttime = 0;
		cl_demo_endtime = 0;
	}

	if (cl.protocol == PROTOCOL_VERSION_QW)
	{
		MSG_ReadLong();		// server count
		MSG_ReadString();	// gamedir
		cl.viewentity = (MSG_ReadByte() & 0x7F) + 1;		// playernum (hi-bit indicates spectator)

		// these are not explicitly sent by QW server:
		cl.maxclients = 32;		// MAX_CLIENTS
		cl.viewheight = DEFAULT_VIEWHEIGHT;
	}
	else
	{
	// parse maxclients
		cl.maxclients = MSG_ReadByte ();
		if (cl.maxclients < 1 || cl.maxclients > MAX_SCOREBOARD)
		{
			Con_Printf ("Bad maxclients (%u) from server\n", cl.maxclients);
			return;
		}

		if (cl.protocol != PROTOCOL_VERSION_BETA)
		{
		// parse gametype
			cl.gametype = MSG_ReadByte ();

		#ifdef HEXEN2_SUPPORT
			if (hexen2 && (cl.gametype == GAME_DEATHMATCH))
				sv_kingofhill = MSG_ReadShort ();
		#endif
		}
	}

	// parse signon message
	str = MSG_ReadString ();
	Q_strcpy (cl.levelname, str, sizeof(cl.levelname));

	if (!cl_demoseek)
	{
		// seperate the printf's so the server message can have a color
		Con_Print ("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
		Con_Printf ("\x02%s\n", cl.levelname);

		// JDH: these get cleared by CL_ClearState
	//	memset (cl.sound_precache, 0, sizeof(cl.sound_precache));
	//	memset (cl.model_precache, 0, sizeof(cl.model_precache));

		cl_precache_changed = true;
	}
	else
		cl_precache_changed = false;

	for (i=0 ; i<NUM_MODELINDEX ; i++)
		cl_modelindex[i] = -1;

	if (cl.protocol == PROTOCOL_VERSION_QW)
	{
		for (i = 0; i < 10; i++)
			MSG_ReadFloat();		// physics values
		cls.signon = SIGNONS-1;
		return;		// sounds & models sent via separate messages
	}

	nummodels = CL_ParseModellist (1);
	numsounds = CL_ParseSoundlist (1);

	if (!cl_precache_changed)
	{
		memset (cl.scores, 0, cl.maxclients * sizeof(*cl.scores));
		return;
	}

	if (cl_demoseek)
	{
		extern void CL_ClearMapData (void);
		extern int host_hunklevel;

	// this is equivalent to CL_ClearState minus the CL_ClearDynamic, which was
	// already done via CL_ResetState
		Mod_ClearAll ();
		if (host_hunklevel)
			Hunk_FreeToLowMark (host_hunklevel);
		CL_ClearMapData ();
	}

	cl.scores = Hunk_AllocName (cl.maxclients * sizeof(*cl.scores), "scores");

//	if (!cl_demoseek)
	{
		SCR_UpdateLoadCaption (COM_SkipPath(model_precache[1]));

	// load the extra "no-flamed-torch" model
	// NOTE: this is an ugly hack
	#ifdef HEXEN2_SUPPORT
		if (!hexen2)
	#endif
		{
			if (nummodels == MAX_MODELS)
			{
				Con_DPrintf ("Server sent too many model precaches -> replacing flame0.mdl with flame.mdl\n");
				cl_modelindex[mi_flame0] = cl_modelindex[mi_flame1];
			}
			else
			{
				Q_strcpy (model_precache[nummodels], cl_modelnames[mi_flame0], sizeof(model_precache[nummodels]));
				cl_modelindex[mi_flame0] = nummodels++;
			}
		}

	#ifdef HEXEN2_SUPPORT
		if (hexen2 /*&& precache.value*/)
		{
			total_loading_size = nummodels + numsounds;
			current_loading_size = 1;
			loading_stage = 2;
		}
	#endif

	// precache models
		CL_PrecacheModels (1, nummodels);
		CL_PrecacheSounds (1, numsounds);

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			total_loading_size = 0;
			loading_stage = 0;
		}
	#endif

		R_NewMap ();
		Hunk_Check ();			// make sure nothing is hurt
	}
}

/*
==================
CL_ParseUpdate

Parse an entity update message from the server
If an entity's model or origin changes from frame to frame, it must be
relinked. Other attributes can change without relinking.
==================
*/
//int	bitcounts[16];

extern qboolean mod_oversized;

qboolean CL_ParseUpdate (int bits)
{
	int			startcount, num;
	model_t		*model;
	qboolean	forcelink;
	entity_t	*ent;
	int			skin, i, colornum;
//	float		(*readcoord)(void);

	if (cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		cls.signon = SIGNONS;
		CL_SignonReply ();
	}

	startcount = msg_readcount;

	if (bits & U_MOREBITS)
		bits |= (MSG_ReadByte() << 8);

	if (cl.protocol == PROTOCOL_VERSION_FITZ)
	{
		if (bits & U_EXTEND1)
			bits |= MSG_ReadByte() << 16;
		if (bits & U_EXTEND2)
			bits |= MSG_ReadByte() << 24;
	}
#ifdef HEXEN2_SUPPORT
	else if (hexen2 && (bits & U_MOREBITS2))
		bits |= (MSG_ReadByte () << 16);
#endif

	num = (bits & U_LONGENTITY) ? MSG_ReadShort() : MSG_ReadByte();
	ent = CL_EntityNum (num);
	if (!ent)
		return false;

#ifdef _DEBUG
	if (!ent->model && !(bits & U_MODEL))
		if (!ent->baseline.modelindex || (ent->modelindex && (ent->baseline.modelindex != ent->modelindex)))
			num *= 1;

	if (cl_shownet.value == 2)
		Con_Printf ("  ent #%d\n", num);

	if (num == 70)
		num *= 1;

	if (num+1 == cl.num_entities)		// allocated a new entity
		num *= 1;
#endif
//	for (i=0 ; i<16 ; i++)
//		if (bits & (1 << i))
//			bitcounts[i]++;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		forcelink = CL_ParseUpdate_H2 (ent, num, bits);
	}
	else
#endif
	{
		forcelink = (ent->msgtime != cl.mtime_prev) ? true : false;

		ent->msgtime = cl.mtime;

		if (bits & U_MODEL)
		{
		/***************** JDH ******************/
			if ((cl.protocol <= PROTOCOL_VERSION_STD) || (cl.protocol == PROTOCOL_VERSION_FITZ))
				ent->modelindex = MSG_ReadByte ();
			else
				ent->modelindex = MSG_ReadShort ();
		/***************** JDH ******************/

			//if (ent->modelindex >= MAX_MODELS)
			//	Host_Error ("CL_ParseUpdate: bad modelindex");
		}
		else
		{
			ent->modelindex = ent->baseline.modelindex;
		}

		/*model = cl.model_precache[ent->modelindex];
		if (model != ent->model)
		{
			ent->model = model;
		// automatic animation (torches, etc) can be either all together or randomized
			if (model)
				ent->syncbase = (model->synctype == ST_RAND) ? (float)(rand() & 0x7fff) / 0x7fff : 0.0;
			else
				forcelink = true;	// hack to make null model players work

			if (num > 0 && num <= cl.maxclients)
				R_TranslatePlayerSkin (num - 1);
		}*/

		ent->frame = (bits & U_FRAME) ? MSG_ReadByte() : ent->baseline.frame;

		/*i =*/colornum = (bits & U_COLORMAP) ? MSG_ReadByte() : ent->baseline.colormap;
		/*if (i && (i <= cl.maxclients) && ent->model && (ent->model->modhint == MOD_PLAYER))
			ent->colormap = cl.scores[i-1].translations;
		else
			ent->colormap = vid.colormap;*/

		skin = (bits & U_SKIN) ? MSG_ReadByte() : ent->baseline.skin;
		if (skin != ent->skinnum)
		{
			ent->skinnum = skin;
			if (num > 0 && num <= cl.maxclients)
				R_TranslatePlayerSkin (num - 1);
		}

		ent->effects = (bits & U_EFFECTS) ? MSG_ReadByte() : ent->baseline.effects;


	// shift the known values for interpolation
		VectorCopy (ent->msg_origin, ent->msg_origin_prev);
		VectorCopy (ent->msg_angles, ent->msg_angles_prev);

#ifdef _DEBUG
		if (ent->model && (ent->model->type == mod_brush))
		{
			if (bits & (U_ANGLE1 | U_ANGLE2 | U_ANGLE3))
				Con_DPrintf ("new angle for %s\n", ent->model->name);
		}
#endif
		/*if (mod_oversized && sv.active)
			readcoord = MSG_ReadFloat;
		else
			readcoord = MSG_ReadCoord;*/

		ent->msg_origin[0] = (bits & U_ORIGIN1) ? MSG_ReadCoord() : ent->baseline.origin[0];
		ent->msg_angles[0] = (bits & U_ANGLE1) ? MSG_ReadAngle() : ent->baseline.angles[0];

		ent->msg_origin[1] = (bits & U_ORIGIN2) ? MSG_ReadCoord() : ent->baseline.origin[1];
		ent->msg_angles[1] = (bits & U_ANGLE2) ? MSG_ReadAngle() : ent->baseline.angles[1];

		ent->msg_origin[2] = (bits & U_ORIGIN3) ? MSG_ReadCoord() : ent->baseline.origin[2];
		ent->msg_angles[2] = (bits & U_ANGLE3) ? MSG_ReadAngle() : ent->baseline.angles[2];

	// JDH: blatant hack to smooth rotating entities and deal with coords > +/-4096
	//	(but I figured it was better than introducing yet another customized protocol)

		if (sv.active && ((bits & (U_ANGLE1 | U_ANGLE2 | U_ANGLE3)) ||
			(mod_oversized && (bits & (U_ORIGIN1 | U_ORIGIN2 | U_ORIGIN3)))))
		{
			edict_t *ed = EDICT_NUM(num);

			if (ed)
			{
				if (bits & (U_ANGLE1 | U_ANGLE2 | U_ANGLE3))
					VectorCopy (ed->v.angles, ent->msg_angles);
				if (bits & (U_ORIGIN1 | U_ORIGIN2 | U_ORIGIN3))
					VectorCopy (ed->v.origin, ent->msg_origin);
			}
		}

		if (cl.protocol == PROTOCOL_VERSION_FITZ)
		{
			if (bits & U_ALPHA)
				ent->transparency = MSG_ReadByte() / 255.0;
			else
				ent->transparency = ent->baseline.transparency;

			if (bits & U_FRAME2)
				ent->frame = (ent->frame & 0x00FF) | (MSG_ReadByte() << 8);
			if (bits & U_MODEL2)
				ent->modelindex = (ent->modelindex & 0x00FF) | (MSG_ReadByte() << 8);
			if (bits & U_LERPFINISH)
				MSG_ReadByte();
		}
		else if (bits & U_TRANS)		// Nehahra
		{
			int	floatcount, bytecount;

			floatcount = MSG_ReadFloat ();
			ent->transparency = MSG_ReadFloat ();
			if (floatcount == 2)
				ent->fullbright = MSG_ReadFloat ();

		// JDH: remove alpha & fullbright from message if recording demo
			if (cls.demorecording && !nehahra)
			{
				net_message.data[startcount] &= ~(U_TRANS >> 8);
				bytecount = 4*(floatcount+1);		// # bytes to remove

				for (i = msg_readcount; i < net_message.cursize; i++)
					net_message.data[i-bytecount] = net_message.data[i];

				net_message.cursize -= bytecount;
				msg_readcount -= bytecount;
			}
		}
		else
		{
			ent->transparency = 1.0;
			ent->fullbright = 0;
		}

		if (ent->modelindex >= MAX_MODELS)
			Host_Error ("CL_ParseUpdate: bad modelindex");

		model = cl.model_precache[ent->modelindex];
		if (model != ent->model)
		{
#ifdef _DEBUG
			if (cls.demoplayback && /*cl_demorewind.value &&*/ model &&
				(!strcmp(model->name, "progs/shambler.mdl") || !strcmp(model->name, "progs/h_shams.mdl")))
				ent->model = model;
#endif
			ent->model = model;
		// automatic animation (torches, etc) can be either all together or randomized
			if (model)
				ent->syncbase = (model->synctype == ST_RAND) ? (float)(rand() & 0x7fff) / 0x7fff : 0.0;
			else
				forcelink = true;	// hack to make null model players work

			if (num > 0 && num <= cl.maxclients)
				R_TranslatePlayerSkin (num - 1);

		// I'm not sure whether I want this or not:
		//	ent->frame_start_time = ent->translate_start_time = ent->rotate_start_time = 0;
		}

		if (colornum && (colornum <= cl.maxclients) && ent->model && (ent->model->modhint == MOD_PLAYER))
			ent->colormap = cl.scores[colornum-1].translations;
		else
			ent->colormap = vid.colormap;
	}

#ifdef _DEBUG
/*	if (cls.demoplayback && (bits & (U_ORIGIN1 | U_ORIGIN2 | U_ORIGIN3)))
		if ((num == 236))//ent->model && !strcmp(ent->model->name, "progs/wizard.mdl"))
				Con_Printf("scrag %d moved to (%f,%f,%f); time=%lf\n", num,
					ent->msg_origin[0], ent->msg_origin[1], ent->msg_origin[2],
					ent->msgtime);
	if (cls.demoplayback && ent->model && !strcmp(ent->model->name, "progs/wizard.mdl") && !(bits & U_NOLERP))
		num = 1;*/
#endif

	if (cl_demoseek)
		ent->frame_start_time = 0;		// don't want morph between non-consecutive poses

/********TEST!!!! - TEMP HACK **********/
	/*if ((bits & U_NOLERP) && (!cls.demoplayback || (ent->modelindex != cl_modelindex[mi_scrag]) || !Monster_isDying(ent->modelindex, ent->frame)))
		ent->forcelink = true;
	else
		ent->forcelink = false;*/		// JDH: set here instead of in CL_RelinkEntities

	if ((bits & U_NOLERP) && !cls.demoplayback)
		ent->forcelink = true;
/********TEST!!!!**********/

	if (forcelink)
	{	// didn't have an update last message
		VectorCopy (ent->msg_origin, ent->msg_origin_prev);
		VectorCopy (ent->msg_origin, ent->origin);
		VectorCopy (ent->msg_angles, ent->msg_angles_prev);
		VectorCopy (ent->msg_angles, ent->angles);
		ent->forcelink = true;
	}

	return true;
}

// JDH: support for Fitz protocol:
#define B_LARGEMODEL	(1<<0)	// modelindex is short instead of byte
#define B_LARGEFRAME	(1<<1)	// frame is short instead of byte
#define B_ALPHA			(1<<2)	// 1 byte, uses ENTALPHA_ENCODE, not sent if ENTALPHA_DEFAULT

/*
==================
CL_ParseBaseline
==================
*/
void CL_ParseBaseline (entity_t *ent, int version)
{
	entity_state_t *es = &ent->baseline;
	int	bits, i;

	bits = (version == 2) ? MSG_ReadByte() : 0;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		es->modelindex = MSG_ReadShort ();		// even if cl.protocol = 15
	else
#endif
	if (((cl.protocol >= PROTOCOL_VERSION_BJP) && (cl.protocol <= PROTOCOL_VERSION_BJP3)) ||
		(bits & B_LARGEMODEL))	// Fitz protocol
		es->modelindex = MSG_ReadShort ();
	else
		es->modelindex = MSG_ReadByte ();

// Fitz protocol:
	if (bits & B_LARGEFRAME)
		es->frame = MSG_ReadShort ();
	else
		es->frame = MSG_ReadByte ();

	es->colormap = MSG_ReadByte ();
	es->skin = MSG_ReadByte ();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		es->scale = MSG_ReadByte();
		es->drawflags = MSG_ReadByte();
		es->abslight = MSG_ReadByte();
	}
#endif

	for (i=0 ; i<3 ; i++)
	{
		es->origin[i] = MSG_ReadCoord ();
		es->angles[i] = MSG_ReadAngle ();
	}

// JDH: part 2 of blatant hack (part 1 is in CL_ParseUpdate)
//   - doesn't work for svc_spawnstatic since server-side edict no longer exists
	if (sv.active && (ent != &cl_static_entities[cl.num_statics-1]))
	{
		edict_t *ed = EDICT_NUM (ent-cl_entities);

		if (ed)
		{
			VectorCopy (ed->baseline.angles, es->angles);
			if (mod_oversized)
				VectorCopy (ed->baseline.origin, es->origin);
		}
	}

// Fitz protocol:
	if (bits & B_ALPHA)
		es->transparency = MSG_ReadByte () / 255.0;
	else
		es->transparency = 0;
}

/*
==================
CL_ParseClientdata

Server information pertaining to this client only
==================
*/
void CL_ParseClientdata (int bits)
{
	int	i, j;

	if (cl.protocol == PROTOCOL_VERSION_FITZ)
	{
		if (bits & SU_EXTEND1)
			bits |= (MSG_ReadByte() << 16);
		if (bits & SU_EXTEND2)
			bits |= (MSG_ReadByte() << 24);
	}

	if (bits & SU_VIEWHEIGHT)
		cl.viewheight = MSG_ReadChar();

	if (bits & SU_IDEALPITCH)
		cl.idealpitch = MSG_ReadChar();

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (bits & SU_IDEALROLL))
		cl.idealroll = MSG_ReadChar ();
#endif

	VectorCopy (cl.mvelocity, cl.mvelocity_prev);
	if (cl.protocol == PROTOCOL_VERSION_DP7)
	{
		for (i=0 ; i<3 ; i++)
		{
			if (bits & (SU_PUNCH1 << i))
				cl.punchangle[i] = MSG_ReadPreciseAngle ();

			if (bits & (SU_VELOCITY1 << i))
				cl.mvelocity[i] = MSG_ReadFloat ();
		}
	}
	else
	{
		for (i=0 ; i<3 ; i++)
		{
			if (bits & (SU_PUNCH1 << i))
				cl.punchangle[i] = MSG_ReadChar();

			if (bits & (SU_VELOCITY1 << i))
				cl.mvelocity[i] = MSG_ReadChar()*16;
		}
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	{
		// don't check for SU_ITEMS flag unless protocol is DP; some demos
		//  and servers don't set it, even though items field gets sent

		if ((cl.protocol != PROTOCOL_VERSION_DP7) || (bits & SU_ITEMS))
		{
			i = MSG_ReadLong ();
			if (cl.items != i)
			{	// set flash times
				Sbar_Changed ();
				for (j=0 ; j<32 ; j++)
					if ((i & (1 << j)) && !(cl.items & (1 << j)))
						cl.item_gettime[j] = cl.time;
				cl.items = i;
			}
		}
	}

	cl.onground = (bits & SU_ONGROUND) != 0;
	cl.inwater = (bits & SU_INWATER) != 0;

	if (cl.protocol == PROTOCOL_VERSION_DP7)
		return;

	if (bits & SU_WEAPONFRAME)
		cl.stats[STAT_WEAPONFRAME] = MSG_ReadByte();

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
	#ifdef _DEBUG
		if (bits & 0x0200)
			Con_Print("CL_ParseClientdata: got SU_SC1\n");

		if (bits & 0x8000)
			Con_Print("CL_ParseClientdata: got SU_SC2\n");
	#endif

		if (bits & SU_ARMOR)
		{
			cl.stats[STAT_ARMOR] = MSG_ReadByte();
			Sbar_Changed ();
		}

		if (bits & SU_WEAPON)
		{
			cl.stats[STAT_WEAPON] = MSG_ReadShort ();
			Sbar_Changed();
		}

		return;		// default values are NOT set for non-updated values
	}
	else
#endif
	{
		i = (bits & SU_ARMOR) ? MSG_ReadByte() : 0;
		if (cl.stats[STAT_ARMOR] != i)
		{
			cl.stats[STAT_ARMOR] = i;
			Sbar_Changed ();
		}

	/******************JDH******************/
		//i = (bits & SU_WEAPON) ? MSG_ReadByte() : 0;
		if (bits & SU_WEAPON)
		{
			if ((cl.protocol <= PROTOCOL_VERSION_STD) || (cl.protocol == PROTOCOL_VERSION_FITZ))
				i = MSG_ReadByte();
			else
				i = MSG_ReadShort();
		}
		else i=0;
	/******************JDH******************/
		if (cl.stats[STAT_WEAPON] != i)
		{
			cl.stats[STAT_WEAPON] = i;
			Sbar_Changed ();
		// JDH: 2009/05/28 - reset these so interpolation gets a clean start
			cl.viewent.frame_start_time = 0;
			cl.viewent.translate_start_time = 0;
			cl.viewent.rotate_start_time = 0;
		}
	}


// if the following bits are NOT set, the value gets set to a default
// (in Quake, but not Hexen II):

	if (!(bits & SU_VIEWHEIGHT))
		cl.viewheight = DEFAULT_VIEWHEIGHT;

	if (!(bits & SU_IDEALPITCH))
		cl.idealpitch = 0;

	for (i=0 ; i<3 ; i++)
	{
		if (!(bits & (SU_PUNCH1 << i)))
			cl.punchangle[i] = 0;

		if (!(bits & (SU_VELOCITY1 << i)))
			cl.mvelocity[i] = 0;
	}

	if (!(bits & SU_WEAPONFRAME))
	{
	// JDH: hack to fix	weapon deforming when SNG stops firing
		if ((cl.stats[STAT_WEAPON] == cl_modelindex[mi_sng]) && (cl.stats[STAT_WEAPONFRAME] != 8) &&
			(cl.stats[STAT_WEAPONFRAME] != 0))
			cl.viewent.frame_start_time = 0;

		cl.stats[STAT_WEAPONFRAME] = 0;
	}

	i = MSG_ReadShort ();
	if (cl.stats[STAT_HEALTH] != i)
	{
		cl.stats[STAT_HEALTH] = i;
		Sbar_Changed ();
	}

	i = MSG_ReadByte ();
	if (cl.stats[STAT_AMMO] != i)
	{
		cl.stats[STAT_AMMO] = i;
		Sbar_Changed ();
	}

	for (i=0 ; i<4 ; i++)
	{
		j = MSG_ReadByte ();
		if (cl.stats[STAT_SHELLS+i] != j)
		{
			cl.stats[STAT_SHELLS+i] = j;
			Sbar_Changed ();
		}
	}

	i = MSG_ReadByte ();

	if (hipnotic || rogue)
	{
		if (cl.stats[STAT_ACTIVEWEAPON] != (1 << i))
		{
			cl.stats[STAT_ACTIVEWEAPON] = (1 << i);
			Sbar_Changed ();
		}
	}
	else
	{
		if (cl.stats[STAT_ACTIVEWEAPON] != i)
		{
			cl.stats[STAT_ACTIVEWEAPON] = i;
			Sbar_Changed ();
		}
	}

	if (cl.protocol == PROTOCOL_VERSION_FITZ)
	{
		if (bits & SU_WEAPON2)
			cl.stats[STAT_WEAPON] |= (MSG_ReadByte() << 8);
		if (bits & SU_ARMOR2)
			cl.stats[STAT_ARMOR] |= (MSG_ReadByte() << 8);
		if (bits & SU_AMMO2)
			cl.stats[STAT_AMMO] |= (MSG_ReadByte() << 8);
		if (bits & SU_SHELLS2)
			cl.stats[STAT_SHELLS] |= (MSG_ReadByte() << 8);
		if (bits & SU_NAILS2)
			cl.stats[STAT_NAILS] |= (MSG_ReadByte() << 8);
		if (bits & SU_ROCKETS2)
			cl.stats[STAT_ROCKETS] |= (MSG_ReadByte() << 8);
		if (bits & SU_CELLS2)
			cl.stats[STAT_CELLS] |= (MSG_ReadByte() << 8);
		if (bits & SU_WEAPONFRAME2)
			cl.stats[STAT_WEAPONFRAME] |= (MSG_ReadByte() << 8);

		if (bits & SU_WEAPONALPHA)
			cl.viewent.transparency = MSG_ReadByte() / 255.0;
		else
			cl.viewent.transparency = 0;
	}
}

/*
==================
CL_ClientdataSize - parse svc_clientdata and calculate the data size
==================
*/
/*int CL_ClientdataSize (int bits)
{
	int	i, count = 0;

	if (bits & SU_VIEWHEIGHT)
		count++;
	if (bits & SU_IDEALPITCH)
		count++;

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (bits & SU_IDEALROLL))
		count++;
#endif

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1 << i))
			count += ((cl.protocol == PROTOCOL_VERSION_DP7) ? 2 : 1);

		if (bits & (SU_VELOCITY1 << i))
			count += ((cl.protocol == PROTOCOL_VERSION_DP7) ? 4 : 1);
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
		if ((cl.protocol != PROTOCOL_VERSION_DP7) || (bits & SU_ITEMS))
			count += 4;

	if (cl.protocol != PROTOCOL_VERSION_DP7)
	{
		if (bits & SU_WEAPONFRAME)
			count++;
		if (bits & SU_ARMOR)
			count++;

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			if (bits & SU_WEAPON)
				count += 2;
		}
		else
	#endif
		{
			if (bits & SU_WEAPON)
				count += ((cl.protocol == PROTOCOL_VERSION_STD) ? 1 : 2);

			count += 8;		// health, ammo, active weapon
		}
	}

	return count;
}
*/
/*
=====================
CL_NewTranslation
=====================
*/
void CL_NewTranslation (int slot)
{
	int	i, j, top, bottom;
	byte	*dest, *source;

	if (slot > cl.maxclients)
		Host_Error ("CL_NewTranslation: slot > cl.maxclients");

#ifdef HEXEN2_SUPPORT
	if (hexen2 && !cl.scores[slot].playerclass)
		return;
#endif

	dest = cl.scores[slot].translations;
	source = vid.colormap;
	memcpy (dest, vid.colormap, sizeof(cl.scores[slot].translations));
	top = cl.scores[slot].colors & 0xf0;
	bottom = (cl.scores[slot].colors & 15) << 4;

	R_TranslatePlayerSkin (slot);

	for (i = 0 ; i < VID_GRADES ; i++, dest += 256, source += 256)
	{
		if (top < 128)	// the artists made some backward ranges. sigh.
			memcpy (dest + TOP_RANGE, source + top, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[TOP_RANGE+j] = source[top+15-j];

		if (bottom < 128)
			memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[BOTTOM_RANGE+j] = source[bottom+15-j];
	}
}

/*
==============
CL_UpdateStat
==============
*/
void CL_UpdateStat (int stat, int value)
{
	int j;

	if (stat < 0 || stat >= MAX_CL_STATS)
		Host_Error ("svc_updatestat: %i is invalid", stat);		// JDH: was Sys_Error

	if ((cl.protocol == PROTOCOL_VERSION_QW) && (stat == STAT_ITEMS))
	{	// set flash times
		Sbar_Changed ();
		for (j=0 ; j<32 ; j++)
			if ((value & (1<<j)) && !(cl.items & (1<<j)))
				cl.item_gettime[j] = cl.time;
		cl.items = value;
	}

	cl.stats[stat] = value;
}

/*
=====================
CL_ParseStatic
=====================
*/
void CL_ParseStatic (int version)
{
	entity_t	*ent;

	if (cl.num_statics >= MAX_STATIC_ENTITIES)
		Host_Error ("Too many static entities");
	ent = &cl_static_entities[cl.num_statics];
	cl.num_statics++;
	CL_ParseBaseline (ent, version);

// copy it to the current state
	ent->model = cl.model_precache[ent->baseline.modelindex];
	ent->frame = ent->baseline.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = ent->baseline.skin;
	ent->effects = ent->baseline.effects;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		ent->scale = ent->baseline.scale;
		ent->drawflags = ent->baseline.drawflags;
		ent->abslight = ent->baseline.abslight;
	}
#endif

	VectorCopy (ent->baseline.origin, ent->origin);
	VectorCopy (ent->baseline.angles, ent->angles);

	R_AddEfrags (ent);
}

/*
===================
CL_ParseStaticSound
===================
*/
void CL_ParseStaticSound (int version)
{
	float (*read_coord)();
	vec3_t	org;
	int	i, sound_num, vol, atten;

	if (cl.protocol == PROTOCOL_VERSION_DP7)
		read_coord = MSG_ReadFloat;
	else
		read_coord = MSG_ReadCoord;

	for (i=0 ; i<3 ; i++)
		org[i] = read_coord ();

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (cl.protocol == PROTOCOL_VERSION_H2_112))
		sound_num = MSG_ReadShort ();
	else
#endif
	if ((cl.protocol == PROTOCOL_VERSION_BJP2) || (version == 2))
		sound_num = MSG_ReadShort ();
	else
		sound_num = MSG_ReadByte ();

	vol = MSG_ReadByte ();
	atten = MSG_ReadByte ();

	S_StaticSound (cl.sound_precache[sound_num], org, vol, atten);
}

/*
===================
CL_ParseLightstyle
===================
*/
void CL_ParseLightstyle (void)
{
	int				i, total, length;
	char			*str;
	lightstyle_t	*ls, *oldls;

	i = MSG_ReadByte ();
	str = MSG_ReadString ();

	if (i >= MAX_LIGHTSTYLES)
	{
		//Sys_Error ("svc_lightstyle > MAX_LIGHTSTYLES");
		Con_Printf ("\x02""Warning: received svc_lightstyle with invalid index %d\n", i);
		return;
	}

	ls = &cl_lightstyle[i];
	if (cls.demoplayback)		// JDH
	{
		if (cl_demorewind.value)
		{
		// re-activate previous lightstyle when rewinding
			oldls = ls->prev;
			if (oldls)
			{
				Q_strcpy (ls->map, oldls->map, MAX_STYLESTRING);
				ls->length = oldls->length;
				ls->prev = oldls->prev;
				ls->average = oldls->average;
				ls->peak = oldls->peak;
				free (oldls);
			}
			return;
		}

		if (ls->length)
		{
		// create copy of current lightstyle:
			oldls = Q_malloc (sizeof(lightstyle_t));
			Q_strcpy (oldls->map, ls->map, MAX_STYLESTRING);
			oldls->length = ls->length;
			oldls->average = ls->average;
			oldls->peak = ls->peak;

		// add it to the head of the list
			oldls->prev = ls->prev;
			ls->prev = oldls;
		}
	}

	Q_strcpy (ls->map, str, MAX_STYLESTRING);
	ls->length = length = strlen (ls->map);

	// JDH: calc extra info for r_flatlightstyles (from Fitz)
	if (length)
	{
		i = 0;		// index of first value
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			int k = ls->map[0];
			if (k == '1' || k == '2' || k == '3')		// Explicit anim rate
			{
				if (!--length)
				{
					ls->average = ls->peak = 'm';
					return;
				}
				i++;
			}
		}
	#endif

		total = 0;
		ls->peak = 'a';
		for (; i < length; i++)
		{
			total += ls->map[i] - 'a';
			ls->peak = max(ls->peak, ls->map[i]);
		}
		ls->average = (total / length) + 'a';
	}
	else
		ls->average = ls->peak = 'm';
}

/*
===================
CL_ParseIntermission
===================
*/
void CL_ParseIntermission (void)
{
	int i;
	entity_t *ent;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		cl.intermission = MSG_ReadByte();
	else
#endif
		cl.intermission = 1;

	// intermission bugfix -- by joe
	cl.completed_time = cl.mtime;
	vid.recalc_refdef = true;	// go to full screen

	if (cl.protocol == PROTOCOL_VERSION_QW)
	{
		ent = &cl_entities[cl.viewentity];

		for (i=0 ; i<3 ; i++)
			ent->msg_origin[i] = MSG_ReadCoord ();
		for (i=0 ; i<3 ; i++)
			ent->msg_angles[i] = MSG_ReadAngle ();

		VectorCopy (ent->msg_origin, ent->msg_origin_prev);
		VectorCopy (ent->msg_origin, ent->origin);
		VectorCopy (ent->msg_angles, ent->msg_angles_prev);
		VectorCopy (ent->msg_angles, ent->angles);
		VectorClear (cl.mvelocity);
		VectorClear (cl.mvelocity_prev);
		VectorClear (cl.velocity);
	}
}

/*
===================
CL_ParseViewangles
===================
*/
void CL_ParseViewangles (void)
{
	float (*read_angle)();
	int i;

	if (cl.protocol == PROTOCOL_VERSION_DP7)
		read_angle = MSG_ReadPreciseAngle;
	else
		read_angle = MSG_ReadAngle;

	for (i=0 ; i<3 ; i++)
		cl.viewangles[i] = read_angle ();
	//Con_DPrintf ("viewangles = (%.2f, %.2f, %.2f)\n", cl.viewangles[0], cl.viewangles[1], cl.viewangles[2]);
	if (sv.active && host_cutscenehack.value)
	{
		if (svs.clients[0].in_cutscene)
			VectorCopy (svs.clients[0].cutscene_viewangles, cl.viewangles);
	}
}

#define SHOWNET(x)						\
	if (cl_shownet.value == 2)				\
		Con_Printf ("%3i:%s\n", msg_readcount - 1, (x))

#ifdef HEXEN2_SUPPORT
	#define SVC_STRING(n)	\
		((((n) >= NUM_SVC) || ((n) < 0)) ? "?" : (hexen2) ? svc_strings_H2[(n)] : \
		 (cl.protocol == PROTOCOL_VERSION_QW) ? svc_strings_QW[(n)] : svc_strings[(n)])
#else
	#define SVC_STRING(n)	\
		((((n) >= NUM_SVC) || ((n) < 0)) ? "?" : \
		(cl.protocol == PROTOCOL_VERSION_QW) ? svc_strings_QW[(n)] : svc_strings[(n)])
#endif

/*
=====================
CL_ParseServerMessage
=====================
*/
void CL_ParseServerMessage (void)
{
	int		cmd, i, vers, count;
	qboolean		cmd_processed;
	extern	float	printstats_limit;		//
	extern	cvar_t	scr_printstats;			// by joe
	extern	cvar_t	scr_printstats_length;	//
	extern  qboolean CL_Demo_EOF (void);
	const char	*str;
	entity_t	*ent;
	int		lastcmd = -1;		// JDH

	if (cl.protocol == PROTOCOL_VERSION_QW)
		CL_ClearProjectiles ();

// if recording demos, copy the message out
	if (cl_shownet.value == 1)
		Con_Printf ("%i ", net_message.cursize);
	else if (cl_shownet.value == 2)
		Con_Print ("------------------\n");

	cl.onground = false;	// unless the server says otherwise

// parse the message
	MSG_BeginReading ();

	while (1)
	{
		if (msg_badread)
		{
			if (cls.demoplayback)		// JDH
			{
				Con_DPrintf ("\x02""Warning: unexpected end of message\n");
				return;
			}
			str = SVC_STRING(cmd);
			Host_Error ("CL_ParseServerMessage: Bad server message; last cmd was %s (%i)", str, cmd);
		}

		cmd = MSG_ReadByte ();
		if (cmd == -1)
		{
			SHOWNET("END OF MESSAGE");
			return;		// end of message
		}

	// if the high bit of the command byte is set, it is a fast update
		if (cmd & U_SIGNAL)
		{
			SHOWNET("fast update");
			if (!CL_ParseUpdate (cmd & 127))
				return;
			lastcmd = cmd;
			continue;
		}

		SHOWNET(SVC_STRING(cmd));
		cmd_processed = true;		// until we find out otherwise

		// other commands
		switch (cmd)
		{
		case svc_nop:
			break;

		case svc_time:
			cl.mtime_prev = cl.mtime;
			cl.mtime = MSG_ReadFloat ();
#ifdef _DEBUG
			if (cl.mtime < cl.mtime_prev)
				cmd *= 1;
#endif
			/*if (cls.demoplayback && !cl_demo_starttime)
			{
				cl_demo_starttime = cl.mtime;			// JDH: store first time update of demo
				cl_demo_endtime = 0;		// not known yet
			}*/
			break;

		case svc_clientdata:
			i = (unsigned short) MSG_ReadShort ();
			CL_ParseClientdata (i);
			break;

		case svc_version:
			vers = MSG_ReadLong ();
			if (!CL_IsKnownProtocol (&vers))
				Host_Error ("CL_ParseServerMessage: Server is using unknown protocol %i", vers);

			if (!sv.active && !cls.demoplayback && (vers != cl.protocol))
				Con_Printf ("Switching to protocol %d at request of server\n", vers);
			cl.protocol = vers;
			break;

		case svc_disconnect:
			if (cl.protocol == PROTOCOL_VERSION_QW)
				str = MSG_ReadString ();
			else str = "Server disconnected\n";
			if (!cls.demoplayback || CL_Demo_EOF())		// JDH: to handle demo corruption
				Host_EndGame (str);
			Con_DPrintf ("\x02""Warning: ignoring mid-demo svc_disconnect\n");
			break;

		case svc_print:
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				i = MSG_ReadByte ();
				str = MSG_ReadString();
				if (i == 3)		// PRINT_CHAT
					str = va("\x01%s", str);		// colored text
			}
			else
				str = MSG_ReadString();
	#ifdef HEXEN2_SUPPORT
			if (!hexen2 || !intro_playing)
	#endif
			if (!cl_demoseek)		// JDH
			{
				Con_Print (str);
				if (scr_printstats.value == 3 || scr_printstats.value == 4)
					printstats_limit = cl.time + scr_printstats_length.value;
			}
			break;

		case svc_centerprint:
			//SCR_CenterPrint (MSG_ReadString ());
			str = MSG_ReadString();
			if (!cl_demoseek)		// JDH
			{
				SCR_CenterPrint (str);
				Con_LogCenterPrint (str);		// JDH, from Fitz
			}
			break;

		case svc_stufftext:
			Cbuf_AddText (MSG_ReadString(), SRC_SERVER);
			break;

		case svc_damage:
			V_ParseDamage (cl_demoseek);
			if (scr_printstats.value == 3 || scr_printstats.value == 4)
				printstats_limit = cl.time + scr_printstats_length.value;
			break;

		case svc_serverinfo:
			CL_ParseServerInfo ();
			vid.recalc_refdef = true;	// leave intermission full screen
			break;

		case svc_setangle:
			CL_ParseViewangles ();
			break;

	// JDH: 2-byte angles for cutscene viewangles
		/*case svc_setpreciseangle:
			for (i=0 ; i<3 ; i++)
				cl.viewangles[i] = MSG_ReadPreciseAngle ();
			break;*/

		case svc_setview:
			cl.viewentity = MSG_ReadShort ();
			break;

		case svc_lightstyle:
			CL_ParseLightstyle ();
			break;

		case svc_sound:
			if (!CL_ParseStartSoundPacket (cl_demoseek))
				return;
			break;

		case svc_stopsound:
			i = MSG_ReadShort ();
			S_StopSound (i>>3, i&7);
			break;

		case svc_updatename:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatename > MAX_SCOREBOARD");
			Q_strcpy (cl.scores[i].name, MSG_ReadString (), sizeof(cl.scores[i].name));
			break;

		case svc_updatefrags:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatefrags > MAX_SCOREBOARD");
			cl.scores[i].frags = MSG_ReadShort ();
			break;

		case svc_updatecolors:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatecolors > MAX_SCOREBOARD");
			cl.scores[i].colors = MSG_ReadByte ();
			CL_NewTranslation (i);
			break;

		case svc_particle:
			R_ParseParticleEffect (cl_demoseek);
			break;

		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
			ent = CL_EntityNum(i);
			if (!ent)
				return;
			CL_ParseBaseline (ent, 1);
			break;

		case svc_spawnstatic:
			CL_ParseStatic (1);
			break;

		case svc_temp_entity:
			CL_ParseTEnt (cl_demoseek);
			break;

		case svc_setpause:
			if ((cl.paused = MSG_ReadByte()))
			{
				CDAudio_Pause (SRC_SERVER);
			}
			else
			{
				CDAudio_Resume (SRC_SERVER);
			}
			break;

		case svc_signonnum:
			i = MSG_ReadByte ();
			if (i > cls.signon)
			{
				cls.signon = i;
				CL_SignonReply ();
			}
			else if (cls.demoplayback)		// JDH
			{
				Con_DPrintf ("\x02""Warning: invalid signon number\n");
				return;
			}
			else
				Host_Error ("Received signon %i when at %i", i, cls.signon);
			break;

		case svc_killedmonster:
			if (cls.demoplayback && cl_demorewind.value)
				cl.stats[STAT_MONSTERS]--;
			else
				cl.stats[STAT_MONSTERS]++;
			if (scr_printstats.value == 3 || scr_printstats.value == 4)
				printstats_limit = cl.time + scr_printstats_length.value;
			break;

		case svc_foundsecret:
			if (cls.demoplayback && cl_demorewind.value)
				cl.stats[STAT_SECRETS]--;
			else
				cl.stats[STAT_SECRETS]++;
			if (scr_printstats.value == 3 || scr_printstats.value == 4)
				printstats_limit = cl.time + scr_printstats_length.value;
			break;

		case svc_updatestat:
			i = MSG_ReadByte ();
			count = (cl.protocol == PROTOCOL_VERSION_QW) ? MSG_ReadByte () : MSG_ReadLong ();
			CL_UpdateStat (i, count);
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound (1);
			break;

		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			if (cl.protocol != PROTOCOL_VERSION_QW)
				cl.looptrack = MSG_ReadByte ();
		#ifdef HEXEN2_SUPPORT
			if (hexen2 && Q_strcasecmp(bgmtype.string, "cd"))
			{
				CDAudio_Stop (SRC_SERVER);
				break;
			}
		#endif
			if ((cls.demoplayback || cls.demorecording) && (cls.forcetrack != -1))
				CDAudio_Play (SRC_SERVER, (byte)cls.forcetrack, true);
			else
				CDAudio_Play (SRC_SERVER, (byte)cl.cdtrack, true);
			break;

		case svc_intermission:
			CL_ParseIntermission ();
			break;

		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString());
			break;

		case svc_cutscene:		// svc_smallkick for QW; svc_particle2 for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				R_ParseParticleEffect2 ();
				break;
			}
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				cl.punchangle[PITCH] = -2;
				break;
			}
			cl.intermission = 3;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString());
			break;

		case svc_sellscreen:
			Cmd_ExecuteString ("help", SRC_SERVER);
			break;

		// nehahra support
		case svc_showlmp:		// svc_bigkick for QW; svc_cutscene for Hexen II (never implemented)
			if (cl.protocol == PROTOCOL_VERSION_QW)
				cl.punchangle[PITCH] = -4;
			else
				SHOWLMP_decodeshow ();
			break;

		case svc_hidelmp:		// svc_updateping for QW; svc_midi_name for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				CL_ParseMIDI ();
			}
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				MSG_ReadByte ();		// client num
				MSG_ReadShort ();	// ping value
			}
			else SHOWLMP_decodehide ();
			break;

		case svc_skybox:		// svc_updateentertime for QW; svc_updateclass for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				CL_ParseUpdateClass ();
			}
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				i = MSG_ReadByte ();
				if (i >= cl.maxclients)
					Host_Error ("CL_ParseServerMessage: svc_updateentertime > MAX_SCOREBOARD");
				cl.scores[i].entertime = realtime - MSG_ReadFloat ();
			}
			else
			{
				str = MSG_ReadString();
				if (!no24bit)
					Cvar_Set ("r_skybox", str);
			}
			break;

		case svc_skyboxsize:		// svc_entgravity for QW; svc_setangle_interpolate for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				CL_ParseAngleInterpolate();
			}
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				MSG_ReadFloat();
			}
			else MSG_ReadCoord();
			break;

		case svc_fog_neh:		// svc_setinfo for QW; svc_update_kingofhill for Hexen II; svc_updatestatubyte for DP
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				sv_kingofhill = MSG_ReadShort() - 1;
			}
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				CL_SetInfo ();
			}
			else if (cl.protocol == PROTOCOL_VERSION_DP7)
			{
				i = MSG_ReadByte ();
				CL_UpdateStat (i, MSG_ReadByte ());
			}
			else Fog_ParseNehMessage();
			break;

		case svc_bf:				// from Fitz; svc_updateuserinfo for QW; svc_set_view_flags for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
				cl.viewent.drawflags |= MSG_ReadByte();
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				CL_UpdateUserinfo ();
			}
			else Cmd_ExecuteString ("bf", SRC_SERVER);
			break;

		case svc_fog_fitz:			// svc_fog from Fitz; svc_download for QW; svc_clear_view_flags for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
				cl.viewent.drawflags &= ~MSG_ReadByte();
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				MSG_ReadShort ();		// size
				MSG_ReadByte ();		// percent
			}
			else Fog_ParseFitzMessage ();
			break;

		case svc_spawnbaseline2:	// PROTOCOL_VERSION_FITZ; svc_playerinfo for QW; svc_start_effect for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
				CL_ParseEffect ();
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				CL_ParsePlayerinfo ();
			}
			else
			{
				i = MSG_ReadShort ();
				ent = CL_EntityNum (i);		// must use CL_EntityNum() to force cl.num_entities up
				if (!ent)
					return;
				CL_ParseBaseline (ent, 2);
			}
			break;

		case svc_spawnstatic2:		// PROTOCOL_VERSION_FITZ; svc_nails for QW; svc_end_effect for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
				CL_EndEffect ();
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				CL_ParseProjectiles ();
			}
			else CL_ParseStatic (2);
			break;

		case svc_spawnstaticsound2: // PROTOCOL_VERSION_FITZ; svc_chokecount for QW; svc_plaque for Hexen II
	#ifdef HEXEN2_SUPPORT
			if (hexen2)
				CL_Plaque ();
			else
	#endif
			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				MSG_ReadByte ();		// ignored
			}
			else CL_ParseStaticSound (2);
			break;

		default:
			cmd_processed = false;
			break;
		}


		if (!cmd_processed)
		{
			cmd_processed = true;		// until we find out otherwise

			if (cl.protocol == PROTOCOL_VERSION_QW)
			{
				switch (cmd)
				{
				case svc_updatestatlong:
					i = MSG_ReadByte ();
					count = MSG_ReadLong ();
					CL_UpdateStat (i, count);
					break;

				case svc_muzzleflash:
					i = MSG_ReadShort ();
					if (!cl_demoseek && ((unsigned)(i-1) < MAX_SCOREBOARD))
						CL_MuzzleFlash (CL_EntityNum (i), i);
					break;

				case svc_modellist:
					i = MSG_ReadByte();
					count = CL_ParseModellist (i+1);
					CL_PrecacheModels (i+1, count);
					break;

				case svc_soundlist:
					i = MSG_ReadByte();
					count = CL_ParseSoundlist (i+1);
					CL_PrecacheSounds (i+1, count);
					MSG_ReadByte();		// # of sounds, or 0 ??
					break;

				case svc_packetentities:
					if (!CL_ParsePacketEntities (false))
						return;
					break;

				case svc_deltapacketentities:
					if (!CL_ParsePacketEntities (true))		// FIXME: NOT WORKING YET
						return;
					break;

				case svc_maxspeed:
					MSG_ReadFloat();
					break;

				case svc_serverinfo_qw:
					MSG_ReadString();	// key name
					MSG_ReadString();	// key value
					break;

				case svc_updatepl:
					MSG_ReadByte();		// client num
					MSG_ReadByte();		// "pl" - what is this?
					break;

				default:
					cmd_processed = false;
					break;
				}

			}
		#ifdef HEXEN2_SUPPORT
			else if (hexen2)
			{
				switch (cmd)
				{
				case svc_raineffect:
					CL_ParseRainEffect ();
					break;

				case svc_particle3:
					R_ParseParticleEffect3 ();
					break;

				case svc_particle4:
					R_ParseParticleEffect4 ();
					break;

				case svc_particle_explosion:
					CL_ParticleExplosion ();
					break;

				case svc_set_view_tint:
					i = MSG_ReadByte ();
					cl.viewent.colorshade = i;
					break;

				case svc_reference:
					if (!CL_ParseReference ())
						return;
					break;

				case svc_clear_edicts:
					if (!CL_ClearEdicts ())
						return;
					break;

				case svc_update_inv:
					CL_ParseUpdateInv ();
					break;

				case svc_toggle_statbar:
					break;		// wasn't implemented in Hexen II source

				case svc_sound_update_pos:
					CL_ParseSoundPos ();
					break;

				default:
					cmd_processed = false;
					break;
				}
			}
		#endif		// #ifdef HEXEN2_SUPPORT
			else cmd_processed = false;
		}

		if (!cmd_processed)
		{
			if (cls.demoplayback)		// JDH
			{
				Con_DPrintf ("\x02""Warning: unknown command %d; skipping to next message\n", cmd);
				return;
			}
			if (lastcmd & U_SIGNAL)
				str = "a fast update";
			else
				str = SVC_STRING(lastcmd);

			Host_Error ("CL_ParseServerMessage: Unknown server command %i\n  Last cmd was %s (%i)", cmd, str, lastcmd);
		}

		lastcmd = cmd;
	}
}

#endif		//#ifndef RQM_SV_ONLY
