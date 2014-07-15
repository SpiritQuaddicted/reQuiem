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
// cl_main.c -- client main loop

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "music.h"

#define CONNECT_DEBUG		// tracking down elusive bug in CL_EstablishConnection


// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

cvar_t	cl_shownet  = {"cl_shownet",  "0"};	// can be 0, 1, or 2
cvar_t	cl_nolerp   = {"cl_nolerp",   "0"};

cvar_t	lookspring  = {"lookspring",  "0", CVAR_FLAG_ARCHIVE};
cvar_t	lookstrafe  = {"lookstrafe",  "0", CVAR_FLAG_ARCHIVE};
cvar_t	sensitivity = {"sensitivity", "3", CVAR_FLAG_ARCHIVE};

cvar_t	m_pitch     = {"m_pitch", "0.022", CVAR_FLAG_ARCHIVE};
cvar_t	m_yaw       = {"m_yaw",   "0.022", CVAR_FLAG_ARCHIVE};
cvar_t	m_forward   = {"m_forward", "1",   CVAR_FLAG_ARCHIVE};
cvar_t	m_side      = {"m_side",   "0.8",  CVAR_FLAG_ARCHIVE};

// by joe
cvar_t	cl_truelightning		= {"cl_truelightning", "0", CVAR_FLAG_ARCHIVE};
cvar_t	cl_rocket2grenade		= {"cl_r2g", "0", CVAR_FLAG_ARCHIVE};
cvar_t	cl_muzzleflash			= {"cl_muzzleflash", "1", CVAR_FLAG_ARCHIVE};

cvar_t	r_powerupglow			= {"r_powerupglow", "1", CVAR_FLAG_ARCHIVE};
cvar_t	r_explosiontype			= {"r_explosiontype", "0", CVAR_FLAG_ARCHIVE};
cvar_t	r_explosionlight		= {"r_explosionlight", "1", CVAR_FLAG_ARCHIVE};
cvar_t	r_rocketlight			= {"r_rocketlight", "1", CVAR_FLAG_ARCHIVE};
//#ifdef GLQUAKE
cvar_t	r_explosionlightcolor	= {"r_explosionlightcolor", "0", CVAR_FLAG_ARCHIVE};
cvar_t	r_rocketlightcolor		= {"r_rocketlightcolor", "0", CVAR_FLAG_ARCHIVE};
//#endif
cvar_t	r_rockettrail			= {"r_rockettrail", "1", CVAR_FLAG_ARCHIVE};
cvar_t	r_grenadetrail			= {"r_grenadetrail", "1", CVAR_FLAG_ARCHIVE};

cvar_t	cl_bobbing				= {"cl_bobbing", "0", CVAR_FLAG_ARCHIVE};
cvar_t	cl_deadbodyfilter		= {"cl_deadbodyfilter", "0", CVAR_FLAG_ARCHIVE};
cvar_t	cl_gibfilter			= {"cl_gibfilter", "0", CVAR_FLAG_ARCHIVE};
cvar_t	cl_advancedcompletion	= {"cl_advancedcompletion", "1", CVAR_FLAG_ARCHIVE};

cvar_t	cl_demospeed			= {"cl_demospeed", "1"};
cvar_t	cl_demo_compress		= {"cl_demo_compress", "1", CVAR_FLAG_ARCHIVE};		// JDH
cvar_t	cl_demo_compress_fmt	= {"cl_demo_compress_fmt", "0", CVAR_FLAG_ARCHIVE};		// JDH

// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
entity_t		cl_entities[MAX_EDICTS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];

int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];

modelindex_t	cl_modelindex[NUM_MODELINDEX];
char			*cl_modelnames[NUM_MODELINDEX];

#ifdef HEXEN2_SUPPORT
  extern char		*plaquemessage;
  extern cvar_t		cl_playerclass;
  extern qboolean	intro_playing;
  extern int		loading_stage;
#endif

extern int		sound_started;
extern cvar_t	host_mapname;

/*
=====================
CL_FreeMem
=====================
*/
void CL_FreeMem (void)
{
	int				i;
	lightstyle_t	*ls, *prev;

// JDH: free dynamically allocated list of previous lightstyles,
//      and set current lightstyle to its original value
	for (i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		ls = cl_lightstyle[i].prev;
		while (ls)
		{
			prev = ls->prev;
			if (!prev)
			{
				Q_strcpy (cl_lightstyle[i].map, ls->map, MAX_STYLESTRING);
				cl_lightstyle[i].length = ls->length;
				cl_lightstyle[i].average = ls->average;
				cl_lightstyle[i].peak = ls->peak;
			}

			free (ls);
			ls = prev;
		}
		cl_lightstyle[i].prev = NULL;
	}

	V_ResetCshifts ();
}

/*
=====================
CL_ClearDynamic
=====================
*/
void CL_ClearDynamic (void)
{
// clears only the data that has changed since signon completed
	SZ_Clear (&cls.message);
	cls.signon = 0;

	memset (cl_entities+1, 0, sizeof(cl_entities)-sizeof(cl_entities[0]));
	memset (cl_static_entities, 0, sizeof(cl_static_entities));		// JDH
	
	memset (cl_dlights, 0, sizeof(cl_dlights));

	CL_ClearTEnts ();
	CL_FreeMem ();

#ifdef HEXEN2_SUPPORT
	CL_ClearEffects ();

	plaquemessage = "";

	Sbar_InvReset ();
#endif
}

/*
=====================
CL_ResetState
=====================
*/
void CL_ResetState (void)
{
// resets vars to what they were when cls.signon first became SIGNONS
// (used when jumping back to beginning of demo)

	memset (cl.stats, 0, sizeof(cl.stats));

	cl.items = 0;
	cl.faceanim_endtime = cl.faceanim_starttime = 0;
	memset (cl.item_gettime, 0, sizeof(cl.item_gettime));

	memset (cl.cshifts, 0, sizeof(cl.cshifts));
//	memset (cl.prev_cshifts, 0, sizeof(cl.prev_cshifts));

	VectorClear (cl.mviewangles);
	VectorClear (cl.mviewangles_prev);
	VectorClear (cl.viewangles);

	VectorClear (cl.mvelocity);
	VectorClear (cl.mvelocity_prev);
	VectorClear (cl.velocity);

	VectorClear (cl.punchangle);

	cl.idealpitch = cl.pitchvel = cl.driftmove = cl.viewheight = cl.crouch = 0;
	cl.nodrift = cl.onground = cl.inwater = false;
	cl.laststop = 0;
	cl.intermission = cl.completed_time = 0;
	cl.mtime = cl.mtime_prev = 0;
	cl.num_statics = 0;

#ifdef HEXEN2_SUPPORT
	memset (&cl.v, 0, sizeof(cl.v));
	cl.current_frame = cl.current_sequence = 99;
	cl.reference_frame = cl.last_frame = cl.last_sequence = 199;
	cl.need_build = 2;
	cl.info_mask = cl.info_mask2 = 0;
#endif
/*
	scoreboard_t *scores = cl.scores;		// allocated from hunk
	memset (&cl, 0, sizeof(cl));
	cl.scores = scores;
*/	
	CL_ClearDynamic ();
}

/*
=====================
CL_ClearMapData
=====================
*/
void CL_ClearMapData (void)
{
	int	i;

	memset (&cl_entities[0], 0, sizeof(cl_entities[0]));

// clear other arrays
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	memset (cl_efrags, 0, sizeof(cl_efrags));

	// allocate the efrags and chain together into a free list
	cl.free_efrags = cl_efrags;
	for (i=0 ; i<MAX_EFRAGS-1 ; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;

	if (nehahra)
		SHOWLMP_clear ();
}

/*
=====================
CL_ClearState
=====================
*/
void CL_ClearState (void)
{
	if (!sv.active)
		Host_ClearMemory ();
	else
	// wipe the entire cl structure
		memset (&cl, 0, sizeof(cl));

	CL_ClearDynamic ();
	CL_ClearMapData ();
}

/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (qboolean stoprecord)
{
// stop sounds (especially looping!)
	S_StopAllSounds (true);
	Music_Stop_f (SRC_COMMAND);		// JDH

	if (nehahra)
		Neh_ResetSFX ();

	Fog_Reset();

#ifdef HEXEN2_SUPPORT
	if (hexen2 && !isDedicated)
	{
		R_ClearParticles ();	//jfm: need to clear parts because some now check world
		loading_stage = 0;
	}
#endif

// recording may be active even if state is not connected
	if (cls.demorecording && stoprecord)
		CL_StopRecord (true);

	if (cls.demoplayback)
	{
		CL_StopPlayback ();
	}
	else if (cls.state == ca_connected)
	{
	// if running a local server, shut it down
		
		//if (cls.demorecording)
		//	CL_StopRecord (true);

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer (false);

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			Host_RemoveGIPFiles(NULL);
	#endif
	}

	CL_FreeMem ();
	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
	cl.protocol = PROTOCOL_VERSION_STD;		// JDH: reset to default
	cl.intermission = 0;		// JDH: 2009/04/10
	cl_demoseek = false;		// JDH
	scr_disabled_for_loading = false;		// JDH: in case disconnect happens because of demo/server
											//      changing to a bsp that doesn't exist client-side
}

void CL_Disconnect_f (cmd_source_t src)
{
	CL_Disconnect (true);
	if (sv.active)
		Host_ShutdownServer (false);		// this may be redundant
}

#ifdef CONNECT_DEBUG		// tracking down elusive bug

void CL_DumpSizebuf (sizebuf_t *buf, const char *bufname)
{
	Con_DPrintf ("    %s:\n", bufname);
	Con_DPrintf ("      maxsize = %d\n", buf->maxsize);
	Con_DPrintf ("      cursize = %d\n", buf->cursize);
	Con_DPrintf ("      lastcmdpos = %d\n", buf->lastcmdpos);
	Con_DPrintf ("      allowoverflow = %d\n", buf->allowoverflow);	
	Con_DPrintf ("      overflowed = %d\n", buf->overflowed);	
	Con_DPrintf ("      data = $%08lX\n", buf->data);	
}

void CL_DumpState_cl (void)
{
	Con_DPrintf ("  cl struct:\n");
	Con_DPrintf ("    paused = %d\n", cl.paused);
	Con_DPrintf ("    intermission = %d\n", cl.intermission);
	Con_DPrintf ("    time = %lf\n", cl.time);
	Con_DPrintf ("    ctime = %lf\n", cl.ctime);
}

void CL_DumpState_cls (void)
{
	int i;
	
	Con_DPrintf ("  cls struct:\n");
	Con_DPrintf ("    state = %d\n", cls.state);
	Con_DPrintf ("    mapstring = \"%s\"\n", cls.mapstring);
	Con_DPrintf ("    spawnparms = \"%s\"\n", cls.spawnparms);
	Con_DPrintf ("    demonum = %d\n", cls.demonum);
	for (i = 0; i < MAX_DEMOS; i++)
		Con_DPrintf ("    demos[%d] = \"%s\"\n", i, cls.demos[i]);
	Con_DPrintf ("    demorecording = %d\n", cls.demorecording);
	Con_DPrintf ("    demoplayback = %d\n", cls.demoplayback);
	Con_DPrintf ("    timedemo = %d\n", cls.timedemo);
	Con_DPrintf ("    forcetrack = %d\n", cls.forcetrack);
	Con_DPrintf ("    demofile = $%08lX\n", cls.demofile);
	Con_DPrintf ("    td_lastframe = %d\n", cls.td_lastframe);
	Con_DPrintf ("    td_startframe = %d\n", cls.td_startframe);
	Con_DPrintf ("    td_starttime = %f\n", cls.td_starttime);
	Con_DPrintf ("    signon = %d\n", cls.signon);
	Con_DPrintf ("    netcon = $%08lX\n", cls.netcon);
	Con_DPrintf ("    capturedemo = %d\n", cls.capturedemo);
	CL_DumpSizebuf (&cls.message, "message");
}

void CL_DumpState_sv (void)
{
	int i;
	
	Con_DPrintf ("  sv struct:\n");
	Con_DPrintf ("    active = %d\n", sv.active);
	Con_DPrintf ("    paused = %d\n", sv.paused);
	Con_DPrintf ("    loadgame = %d\n", sv.loadgame);
	Con_DPrintf ("    time = %lf\n", sv.time);
	Con_DPrintf ("    name = \"%s\"\n", sv.name);
	Con_DPrintf ("    lightstyles:\n");
	Con_DPrintf ("    state = %d\n", sv.state);
	Con_DPrintf ("    protocol = %d\n", sv.protocol);
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
		Con_DPrintf ("      %d: \"%s\"\n", i, sv.lightstyles[i]);
}

void CL_DumpState_svs (void)
{
	int i;
	
	Con_DPrintf ("  svs struct:\n");
	Con_DPrintf ("    maxclients = %d\n", svs.maxclients);
	Con_DPrintf ("    maxclientslimit = %d\n", svs.maxclientslimit);
	Con_DPrintf ("    serverflags = %d\n", svs.serverflags);
	Con_DPrintf ("    changelevel_issued = %d\n", svs.changelevel_issued);
	Con_DPrintf ("    clients->spawnparms:\n");
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		Con_DPrintf ("      %d: %f\n", i, svs.clients->spawn_parms[i]);
}

void CL_DumpState (void)
{	
	CL_DumpState_cls();
	Con_DPrintf ("\n");

	CL_DumpState_cl();
	Con_DPrintf ("\n");

	CL_DumpState_sv();
	Con_DPrintf ("\n");

	CL_DumpState_svs();
	Con_DPrintf ("\n");
}
#endif

/*
=====================
CL_EstablishConnection

Host should be either "local" or a net address to be passed on
=====================
*/
void CL_EstablishConnection (const char *host)
{
	if (cls.state == ca_dedicated)
		return;

	if (nehahra)
	    num_sfxorig = num_sfx;

	if (cls.demoplayback)
		return;

	CL_Disconnect (false);

	if (!(cls.netcon = NET_Connect (host)))
	{
#ifdef CONNECT_DEBUG		// tracking down elusive bug
		Con_DPrintf ("CL_EstablishConnection failed.  Dumping client state:\n");
		CL_DumpState ();
#endif
//		Host_Error ("CL_Connect: connect failed");
		Host_Error ("CL_EstablishConnection: failed to connect to \"%s\"", host);
	}

	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host);

	if ((cls.netcon->mod == MOD_JOEQUAKE) && (deathmatch.value || coop.value))
		Con_DPrintf ("Connected to JoeQuake-compatible server\n");

	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing

	MSG_WriteByte (&cls.message, clc_nop);	// joe: fix for NAT from ProQuake
}

/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
//	extern void CL_DemoSaveState (void);

//	Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "prespawn");
		break;

	case 2:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("name \"%s\"\n", cl_name.string));

	#ifdef HEXEN2_SUPPORT
		if ( hexen2 )
		{
			MSG_WriteByte (&cls.message, clc_stringcmd);
			MSG_WriteString (&cls.message, va("playerclass %i\n", (int)cl_playerclass.value));
		}
	#endif

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", ((int)cl_color.value) >> 4, ((int)cl_color.value) & 15));

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("spawn %s", cls.spawnparms));

		if (pq_cheatfree)
		{
			MSG_WriteLong (&cls.message, Mod_CalcCRC("progs/player.mdl"));
			MSG_WriteLong (&cls.message, Mod_CalcCRC("progs/eyes.mdl"));
		}

		break;

	case 3:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory
//		if (cls.demoplayback)
//			CL_DemoSaveState ();
		break;

	case 4:
		SCR_EndLoadingPlaque ();	// allow normal screen updates
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/
int CL_NextDemo (void)
{
	char	str[128];

	if (cls.demonum == -1)
		return 0;		// don't play demos

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
			Con_Print ("No demos listed with startdemos\n");
			cls.demonum = -1;

			return 0;
		}
	}

	SCR_BeginLoadingPlaque (cls.demos[cls.demonum]);

	Q_snprintfz (str, sizeof(str), "playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str, SRC_COMMAND);
	cls.demonum++;

	return 1;
}

/*
==============
CL_PrintEntities_f
==============
*/
void CL_PrintEntities_f (cmd_source_t src)
{
	entity_t	*ent;
	int			i;
	char		*s;

	Con_PagedOutput_Begin ();

	for (i = 0, ent = cl_entities ; i < cl.num_entities ; i++, ent++)
	{
	//	Con_Printf ("%3i:", i);
		if (ent->model)
		{
			s = va ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]"/* fb=%i"*/, ent->model->name,
						ent->frame, ent->origin[0], ent->origin[1], ent->origin[2],
						ent->angles[0], ent->angles[1], ent->angles[2]/*, (int) ent->fullbright*/);
		}
		else
		{
			s = "EMPTY";
		}

		if (!Con_Printf ("%3i:%s\n", i, s))
			break;
	}

	Con_PagedOutput_End ();
}

// joe: from FuhQuake
/*dlighttype_t SetDlightColor (float f, dlighttype_t def, qboolean random)
{
	dlighttype_t	colors[NUM_DLIGHTTYPES-4] = {lt_red, lt_blue, lt_redblue};

	if ((int)f == 1)
		return lt_red;
	else if ((int)f == 2)
		return lt_blue;
	else if ((int)f == 3)
		return lt_redblue;
	else if (((int)f == NUM_DLIGHTTYPES - 3) && random)
		return colors[rand()%(NUM_DLIGHTTYPES-4)];
	else
		return def;
}*/

void SetDlightColor (dlight_t *dl, int f, dlighttype_t def, qboolean random)
{
	dlighttype_t type;

	if (f == 1)
		type = lt_red;
	else if (f == 2)
		type = lt_blue;
	else if (f == 3)
		type = lt_redblue;
	else if ((f == NUM_DLIGHTTYPES - 3) && random)
		type = lt_red + rand()%(NUM_DLIGHTTYPES-4);		// red/blue/redblue
	else
		type = def;

	VectorCopy (bubblecolor[type], dl->color);
}

/*
===============
CL_AllocDlight
===============
*/
dlight_t *CL_AllocDlight (int key)
{
	int		i;
	dlight_t	*dl;

	// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
				goto DLINIT;
		}
	}

	// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (DLIGHT_INACTIVE(dl))
		{
			dl->key = key;
			goto DLINIT;
		}
	}

	dl = &cl_dlights[0];

DLINIT:
	memset (dl, 0, sizeof(*dl));
	dl->color[0] = dl->color[1] = dl->color[2] = 1.0f;
	return dl;
}

/*
===============
CL_NewDlight
===============
*/
dlight_t * CL_NewDlight (int key, vec3_t origin, float radius, float time, dlighttype_t type)
{
	dlight_t	*dl;

	dl = CL_AllocDlight (key);
	VectorCopy (origin, dl->origin);
	dl->radius = radius;
	dl->starttime = cl.time - time;
	dl->endtime = cl.time + time;
//	dl->type = type;
	if (type < NUM_DLIGHTTYPES)
		VectorCopy (bubblecolor[type], dl->color);
	// remainder of fields are zero'd by AllocDlight
	return dl;
}

/*
===============
CL_DecayLights
===============
*/
void CL_DecayLights (void)
{
	int			i;
	dlight_t	*dl;
	double		time;

	if (cl.paused)		// JDH: keep lights from fading while game or demo is paused
		time = 0;
/*#ifdef HEXEN2_SUPPORT
	else if (hexen2)
		time = cl.time - cl.oldtime;
#endif*/
	else
		time = host_frametime;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (DLIGHT_INACTIVE(dl) || !dl->radius)
			continue;

	#ifdef HEXEN2_SUPPORT
		if ( hexen2 )
		{
			if (dl->radius < 0)
			{
				dl->radius += time * dl->decay;
				if (dl->radius > 0)
					dl->radius = 0;
				continue;
			}
		}
	#endif

		dl->radius -= time * dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}

/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
float CL_LerpPoint (void)
{
	float	f, frac;

	f = cl.mtime - cl.mtime_prev;

	if (!f || /*cl_nolerp.value ||*/ cls.timedemo || sv.active)
	{
		cl.time = cl.ctime = cl.mtime;
		return 1;
	}

	if (f > 0.1)
	{	// dropped packet, or start of demo
		cl.mtime_prev = cl.mtime - 0.1;
		f = 0.1;
	}
	frac = (cl.ctime - cl.mtime_prev) / f;
	if (frac < 0)
	{
		if (frac < -0.01)
			cl.time = cl.ctime = cl.mtime_prev;
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
			cl.time = cl.ctime = cl.mtime;
		frac = 1;
	}

	if (cl_nolerp.value)
		return 1;

	return frac;
}

/*
===============
Monster_isDead
  - for dead body removal
===============
*/
qboolean Monster_isDead (int modelindex, int frame)
{
	if ((modelindex == cl_modelindex[mi_fish] && frame == 38) ||
		(modelindex == cl_modelindex[mi_dog] && (frame == 16 || frame == 25)) ||
		(modelindex == cl_modelindex[mi_soldier] && (frame == 17 || frame == 28)) ||
		(modelindex == cl_modelindex[mi_enforcer] && (frame == 54 || frame == 65)) ||
		(modelindex == cl_modelindex[mi_knight] && (frame == 85 || frame == 96)) ||
		(modelindex == cl_modelindex[mi_hknight] && (frame == 53 || frame == 62)) ||
		(modelindex == cl_modelindex[mi_scrag] && frame == 53) ||
		(modelindex == cl_modelindex[mi_ogre] && (frame == 125 || frame == 135)) ||
		(modelindex == cl_modelindex[mi_fiend] && frame == 53) ||
		(modelindex == cl_modelindex[mi_vore] && frame == 22) ||
		(modelindex == cl_modelindex[mi_shambler] && frame == 93) ||
		(modelindex == cl_modelindex[mi_player] && (frame == 49 || frame == 60 || frame == 69 ||
		frame == 84 || frame == 93 || frame == 102)))
		return true;

	return false;
}

qboolean Monster_isDying (int modelindex, int frame)
{
	if ((modelindex == cl_modelindex[mi_fish] && frame >= 18 && frame <= 38) ||
		(modelindex == cl_modelindex[mi_dog] && frame >= 8 && frame <= 25) ||
		(modelindex == cl_modelindex[mi_soldier] && frame >= 8 && frame <= 28) ||
		(modelindex == cl_modelindex[mi_enforcer] && frame >= 41 && frame <= 65) ||
		(modelindex == cl_modelindex[mi_knight] && frame >= 76 && frame <= 96) ||
		(modelindex == cl_modelindex[mi_hknight] && frame >= 42 && frame <= 62) ||
		(modelindex == cl_modelindex[mi_scrag] && frame >= 46 && frame <= 53) ||
		(modelindex == cl_modelindex[mi_ogre] && frame >= 112 && frame <= 135) ||
		(modelindex == cl_modelindex[mi_fiend] && frame >= 45 && frame <= 53) ||
		(modelindex == cl_modelindex[mi_vore] && frame >= 16 && frame <= 22) ||
		(modelindex == cl_modelindex[mi_shambler] && frame >= 83 && frame <= 93) ||
		(modelindex == cl_modelindex[mi_player] && frame >= 41 && frame <= 102))
		return true;

	return false;
}

qboolean Monster_shouldRemove (entity_t *ent)
{
	if (!cl_deadbodyfilter.value || (ent->model->type != mod_alias))
		return false; 

	if (cl_deadbodyfilter.value == 2)
	{
		return Monster_isDying (ent->modelindex, ent->frame);
	}
	else
	{
		return Monster_isDead (ent->modelindex, ent->frame);
	}
}

/*
===============
Model_isHead
  - for gib removal
===============
*/
/*qboolean Model_isHead (int modelindex)
{
	if (modelindex == cl_modelindex[mi_h_dog] || modelindex == cl_modelindex[mi_h_soldier] ||
	    modelindex == cl_modelindex[mi_h_enforcer] || modelindex == cl_modelindex[mi_h_knight] ||
	    modelindex == cl_modelindex[mi_h_hknight] || modelindex == cl_modelindex[mi_h_scrag] ||
	    modelindex == cl_modelindex[mi_h_ogre] || modelindex == cl_modelindex[mi_h_fiend] ||
	    modelindex == cl_modelindex[mi_h_vore] || modelindex == cl_modelindex[mi_h_shambler] ||
	    modelindex == cl_modelindex[mi_h_zombie] || modelindex == cl_modelindex[mi_h_player])
		return true;

	return false;
}*/

/*
===============
CL_MuzzleFlash
===============
*/
void CL_MuzzleFlash (entity_t *ent, int entnum)
{
	dlight_t *dl;
	vec3_t	fv;

	if (!cl_muzzleflash.value)
		return;

	dl = CL_NewDlight (entnum, ent->origin, 200 + (rand()&31), 0.1, lt_muzzleflash);

	dl->origin[2] += 16;
	AngleVectors (ent->angles, fv, NULL, NULL);
	VectorMA (dl->origin, 18, fv, dl->origin);

	dl->minlight = 32;
	if (qmb_initialized && gl_part_lightning.value && (ent->modelindex == cl_modelindex[mi_shambler]))
//		dl->type = lt_blue;
		VectorCopy (bubblecolor[lt_blue], dl->color);
}

/*
===============
CL_CheckEntityEffects
===============
*/
void CL_CheckEntityEffects (entity_t *ent, int entnum)
{
	// EF_BRIGHTFIELD is not used by original progs
	if (ent->effects & EF_BRIGHTFIELD)
		R_EntityParticles (ent);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (ent->effects & EF_DARKFIELD)
			R_DarkFieldParticles (ent);
	}
#endif

	if (ent->effects & EF_MUZZLEFLASH)
	{
		CL_MuzzleFlash (ent, entnum);
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		dlight_t *dl;

		if (ent->effects & EF_BRIGHTLIGHT)
		{
			dl = CL_NewDlight (entnum, ent->origin, 400 + (rand()&31), 0.001, lt_default);
			dl->origin[2] += 16;
		}
		if (ent->effects & EF_DIMLIGHT)
		{
			CL_NewDlight (entnum, ent->origin, 200 + (rand()&31), 0.001, lt_default);
		}
		if (ent->effects & EF_DARKLIGHT)
		{
			dl = CL_NewDlight (entnum, ent->origin, 200 + (rand()&31), 0.001, lt_default);
			dl->dark = true;
		}
		if (ent->effects & EF_LIGHT)
		{
			CL_NewDlight (entnum, ent->origin, 200, 0.001, lt_default);
		}
	}
	else
#endif
	if (ent->modelindex != cl_modelindex[mi_eyes] &&
		((ent->modelindex != cl_modelindex[mi_player] && ent->model->modhint != MOD_PLAYER &&
		  ent->modelindex != cl_modelindex[mi_h_player]) || (r_powerupglow.value && cl.stats[STAT_HEALTH] > 0)))
	{
		if ((ent->effects & (EF_BLUE | EF_RED)) == (EF_BLUE | EF_RED))
			CL_NewDlight (entnum, ent->origin, 200 + (rand() & 31), 0.1, lt_redblue);

		else if (ent->effects & EF_BLUE)
			CL_NewDlight (entnum, ent->origin, 200 + (rand() & 31), 0.1, lt_blue);

		else if (ent->effects & EF_RED)
			CL_NewDlight (entnum, ent->origin, 200 + (rand() & 31), 0.1, lt_red);
	// EF_BRIGHTLIGHT is not used by original progs
		else if (ent->effects & EF_BRIGHTLIGHT)
		{
			vec3_t	tmp;

			VectorCopy (ent->origin, tmp);
			tmp[2] += 16;
			CL_NewDlight (entnum, tmp, 400 + (rand() & 31), 0.1, lt_default);
		}
	// EF_DIMLIGHT is for powerup glows and enforcer's laser
		else if (ent->effects & EF_DIMLIGHT)
			CL_NewDlight (entnum, ent->origin, 200 + (rand() & 31), 0.1, lt_default);
	}
}

/*
===============
CL_CheckEntityFlags
===============
*/
void CL_CheckEntityFlags (entity_t *ent, int entnum, int flags, modhint_t hint, vec3_t oldorg)
{
	dlight_t *dl;

	if (!ent->traildrawn || !VectorL2Compare(ent->trail_origin, ent->origin, 140))
	{
		VectorCopy (ent->origin, oldorg);	//not present last frame or too far away
		ent->traildrawn = true;
	}
	else
		VectorCopy (ent->trail_origin, oldorg);

	if (flags & EF_GIB)
		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BLOOD_TRAIL);

	else if (flags & EF_ZOMGIB)
		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, SLIGHT_BLOOD_TRAIL);

#ifdef HEXEN2_SUPPORT
	else if (hexen2 && (flags & EF_BLOODSHOT) )
		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BLOODSHOT_TRAIL);
#endif
	else if (flags & EF_TRACER)
		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, TRACER1_TRAIL);

	else if (flags & EF_TRACER2)
		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, TRACER2_TRAIL);

	else if (flags & EF_ROCKET)
	{
		if (hint == MOD_LAVABALL)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, LAVA_TRAIL);
			CL_NewDlight( entnum, ent->origin, 100 * (1 + bound(0, r_rocketlight.value, 1)), 0.1, lt_rocket );
		}
		else
		{
			if (r_rockettrail.value)
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ROCKET_TRAIL);

		#ifdef HEXEN2_SUPPORT
			if ( !hexen2 )
		#endif
			{
				if (r_rocketlight.value)
				{
					dl = CL_NewDlight( entnum, ent->origin, 100 * (1 + bound(0, r_rocketlight.value, 1)), 0.1, lt_rocket );
//					dl->type = SetDlightColor (r_rocketlightcolor.value, lt_rocket, false);
					SetDlightColor (dl, r_rocketlightcolor.value, lt_rocket, false);
				}

				if (qmb_initialized)
				{
					vec3_t	back;
					float	scale;

					VectorSubtract (oldorg, ent->origin, back);
					scale = 8.0 / VectorLength(back);
					VectorMA (ent->origin, scale, back, back);
					QMB_MissileFire (back, oldorg, ent->origin);
				}
			}
		}
	}
#ifdef HEXEN2_SUPPORT
	else if (hexen2)
	{
		if (flags & EF_FIREBALL)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, LAVA_TRAIL);
			CL_NewDlight (entnum, ent->origin, 120 - (rand() % 20), 0.01, lt_default);
		}
		else if (flags & EF_ACIDBALL)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ACID_TRAIL);
			CL_NewDlight (entnum, ent->origin, 120 - (rand() % 20), 0.01, lt_default);
		}
		else if (flags & EF_ICE)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ICE_TRAIL);
		}
		else if (flags & EF_SPIT)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, SPIT_TRAIL);
			CL_NewDlight (entnum, ent->origin, -120 - (rand() % 20), 0.05, lt_default);
		}
		else if (flags & EF_SPELL)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, SPELL_TRAIL);
		}
	}
#endif
	else if ((flags & EF_GRENADE) && r_grenadetrail.value)
	{
		// Nehahra dem compatibility
		if (ent->transparency == -1)
		{
			if (cl.time >= ent->smokepuff_time)
			{
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, NEHAHRA_SMOKE);
				ent->smokepuff_time = cl.time + 0.14;
			}
		}
		else
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, GRENADE_TRAIL);
	}
	else if (flags & EF_TRACER3)
		R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, VOOR_TRAIL);

#ifdef HEXEN2_SUPPORT
	else if (hexen2)
	{
		if (flags & EF_VORP_MISSILE)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, VORPAL_TRAIL);
		}
		else if (flags & EF_SET_STAFF)
		{
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, STAFF_TRAIL);
		}
		else if (flags & EF_MAGICMISSILE)
		{
			if ((rand() & 3) < 1)
				R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, MAGICMISSILE_TRAIL);
		}
		else if (flags & EF_BONESHARD)
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BONESHARD_TRAIL);

		else if (flags & EF_SCARAB)
			R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, SCARAB_TRAIL);
	}
#endif
}

/*
===============
CL_RelinkPlayer
===============
*/
void CL_RelinkPlayer (float frac)
{
	int		i;
	float	d;

	// interpolate player info
	for (i=0 ; i<3 ; i++)
		cl.velocity[i] = cl.mvelocity_prev[i] + frac * (cl.mvelocity[i] - cl.mvelocity_prev[i]);

	if (cls.demoplayback)
	{
#ifdef HEXEN2_SUPPORT
		if (!hexen2 || !intro_playing)
#endif
		// interpolate the angles
		for (i=0 ; i<3 ; i++)
		{
			d = cl.mviewangles[i] - cl.mviewangles_prev[i];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[i] = cl.mviewangles_prev[i] + frac*d;
		}
	}
}

vec3_t	player_origin[MAX_SCOREBOARD];
int	numplayers;

#ifdef HEXEN2_SUPPORT
#  define ENT_LINKFULL(e) ((e)->forcelink || (hexen2 && ((e)->msgtime != cl.mtime)))
#else
#  define ENT_LINKFULL(e) ((e)->forcelink)
#endif

/*
===============
CL_RelinkEntities
===============
*/
void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int			i, j;
	float		frac, f, d, bobjrotate;
	vec3_t		delta, oldorg;
	model_t		*model;

	// determine partial update time
	frac = CL_LerpPoint ();

	cl_numvisedicts = 0;

	CL_RelinkPlayer (frac);

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
		bobjrotate = anglemod (100 * cl.time);

	numplayers = 0;

	// start on the entity after the world
	for (i = 1, ent = cl_entities + 1 ; i < cl.num_entities ; i++, ent++)
	{
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				R_RemoveEfrags (ent);	// just became empty

			// fenix@io.com: model transform interpolation
			ent->frame_start_time = ent->translate_start_time = ent->rotate_start_time = 0;
			continue;
		}

#ifdef _DEBUG
		if (i == 72 /*&& ent->model->name && !strcmp(ent->model->name, "progs/s_light.mdl")*/)
			i *= 1;
#endif

		// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime)
	#ifdef HEXEN2_SUPPORT
		if (!hexen2 || !(ent->baseline.flags & ENT_STATE_ON))
		/*if (hexen2)
		{
			if (ent->baseline.flags & ENT_STATE_ON)
			{
				if (cls.demoplayback && cl_demorewind.value && (ent->msgtime > cl.mtime))
				{
					ent->baseline.flags &= ~ENT_STATE_ON;	// FIXME: ** just gets set again in CL_ParseReference **
					ent->model = NULL;
					continue;
				}
			}
			else
			{
				ent->model = NULL;
				continue;
			}
		}
		else*/
	#endif
		{
#ifdef _DEBUG
//			if (cl.mtime - ent->msgtime < 0.2)
//				continue;
#endif
			ent->model = NULL;
			continue;
		}

		VectorCopy (ent->origin, oldorg);

		if (ENT_LINKFULL(ent))
		{	// the entity was not updated in the last message so move to the final spot
			VectorCopy (ent->msg_origin, ent->origin);
			VectorCopy (ent->msg_angles, ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			VectorSubtract (ent->msg_origin, ent->msg_origin_prev, delta);
			f = frac;
			for (j=0 ; j<3 ; j++)
			{
				//delta[j] = ent->msg_origin[j] - ent->msg_origin_prev[j];
				if (delta[j] > 100 || delta[j] < -100)
				{
					f = 1;		// assume a teleportation, not a motion
					ent->translate_start_time = 0;
					ent->rotate_start_time = 0;
					break;
				}
			}

			/*if (f >= 1)
			{
#ifdef _DEBUG
				if (!strcmp(ent->model->name, "*306"))
					ent->model->name[0] = '*';
#endif
				ent->translate_start_time = 0;
				ent->rotate_start_time = 0;
			}*/

		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
			{
				ent->origin[j] = ent->msg_origin_prev[j] + f*delta[j];

				d = ent->msg_angles[j] - ent->msg_angles_prev[j];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[j] = ent->msg_angles_prev[j] + f*d;
			}
		}

#ifdef _DEBUG
		/*if (!VectorCompare(ent->origin, oldorg) && (ent->modelindex == cl_modelindex[mi_scrag]) && Monster_isDying(ent->modelindex, ent->frame))
			Con_Printf("scrag %d moved to (%f,%f,%f); time=%lf\n", i, 
					ent->origin[0], ent->origin[1], ent->origin[2],
					ent->msgtime);*/
#endif

		if (ent->model->modhint == MOD_PLAYER)
		{
			VectorCopy (ent->origin, player_origin[numplayers]);
			numplayers++;
		}

		model = ent->model;

	#ifdef HEXEN2_SUPPORT
		if (!hexen2)
	#endif
		{
			if (Monster_shouldRemove(ent))
				continue;

			if (cl_gibfilter.value && model->type == mod_alias &&
				(ent->modelindex == cl_modelindex[mi_gib1] || ent->modelindex == cl_modelindex[mi_gib2] ||
				 ent->modelindex == cl_modelindex[mi_gib3] || /*Model_isHead(ent->modelindex)*/model->modhint == MOD_HEAD))
				continue;

			if (ent->modelindex == cl_modelindex[mi_explo1] || ent->modelindex == cl_modelindex[mi_explo2])
			{
				// software removal of sprites
				if (r_explosiontype.value == 2 || r_explosiontype.value == 3 || gl_part_explosions.value)
					continue;
			}

			if (!(model = cl.model_precache[ent->modelindex]))
				Host_Error ("CL_RelinkEntities: bad modelindex");

			if (ent->modelindex == cl_modelindex[mi_rocket] &&
				cl_rocket2grenade.value && cl_modelindex[mi_grenade] != -1)
			{
				ent->model = cl.model_precache[cl_modelindex[mi_grenade]];
			}

			// rotate binary objects locally
			if (ent->model->flags & EF_ROTATE)
			{
				ent->angles[1] = bobjrotate;
				if (cl_bobbing.value)
					ent->origin[2] += sin(bobjrotate / 90 * M_PI) * 5 + 5;
			}
		}

		CL_CheckEntityEffects (ent, i);

		if (model->flags)
		{
			CL_CheckEntityFlags (ent, i, model->flags, model->modhint, oldorg);
		}

#if 0
	// JDH: forcelink is now cleared in CL_ParseUpdate.  This fixes the jerky movement
	//      of scrags during demo playback, which was caused by the lerping code above.
	//      (Even though the entity update had U_NOLERP set, forcelink was cleared 
	//      on the first pass through this func; consecutive frames WERE lerped)
		if (cl.mtime != cl.mtime_prev)
			ent->forcelink = false;
#else
		ent->forcelink = false;
#endif

		if (i == cl.viewentity && !chase_active.value)
			continue;

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			if (ent->effects & EF_NODRAW_H2)
				continue;
		}
		else
	#endif
		// nehahra support
		if (ent->effects & EF_NODRAW)
			continue;

		if (qmb_initialized)
		{
			if (ent->modelindex == cl_modelindex[mi_bubble])
			{
				if (!cl.paused && cl.oldtime != cl.time)
					QMB_StaticBubble (ent);
				continue;
			}
			else if (gl_part_lightning.value && ent->modelindex == cl_modelindex[mi_shambler] &&
				 ent->frame >= 65 && ent->frame <= 68)
			{
				vec3_t	liteorg;

				VectorCopy (ent->origin, liteorg);
				liteorg[2] += 32;
				QMB_ShamblerCharge (liteorg);
			}
			else if (gl_part_spiketrails.value && ent->model->modhint == MOD_SPIKE)
			{
				QMB_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BUBBLE_TRAIL);
			}
		}

		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}

/*
================
PM_PlayerMove
  JDH: adapted from QuakeWorld; needed for determining cl.onground during QWD playback
================
*/
trace_t PM_PlayerMove (entity_t *player, vec3_t end)
{
	trace_t		trace;
	hull_t		*hull;

	memset (&trace, 0, sizeof(trace_t));

	if (!cl.worldmodel)			// too early in demo
		return trace;

// fill in a default trace
	trace.fraction = 1;
	trace.allsolid = true;
//		trace.startsolid = true;
	VectorCopy (end, trace.endpos);

// trace a line through the apropriate clipping hull
	hull = &cl.worldmodel->hulls[1];
	SV_RecursiveHullCheck (hull, hull->firstclipnode, 0, 1, player->origin, end, &trace);

	if (trace.allsolid)
		trace.startsolid = true;
	if (trace.startsolid)
		trace.fraction = 0;

// did we clip the move?
	if (trace.fraction >= 1)
	{
		memset (&trace, 0, sizeof(trace_t));
		trace.fraction = 1;
		VectorCopy (end, trace.endpos);
	}

	return trace;
}

vec3_t	player_mins = {-16, -16, -24};
vec3_t	player_maxs = {16, 16, 32};

/*
=============
PM_CategorizePosition
  JDH: adapted from QuakeWorld; needed for determining cl.onground during QWD playback
=============
*/
void PM_CategorizePosition (entity_t *player)
{
	vec3_t		point;
	trace_t		tr;

	if (cl.velocity[2] > 180)
	{
		cl.onground = false;
	}
	else
	{
	// if the player hull point one unit down is solid, the player
	// is on ground
		point[0] = player->origin[0];
		point[1] = player->origin[1];
		point[2] = player->origin[2] - 1;
		
		tr = PM_PlayerMove (player, point);
		cl.onground = (tr.plane.normal[2] > 0.7);		// check steepness
	}

	// QW also determines waterlevel.  But cl.inwater isn't used anyway.
}

/*
===============
CL_CalcCrouch

Smooth out stair step ups
===============
*/
void CL_CalcCrouch (void)
{
	qboolean	teleported;
	entity_t	*ent;
	static	vec3_t	oldorigin = {0, 0, 0};
	static	float	oldz = 0, extracrouch = 0, crouchspeed = 100;

	ent = &cl_entities[cl.viewentity];

	teleported = !VectorL2Compare(ent->origin, oldorigin, 48);
	VectorCopy (ent->origin, oldorigin);

	if (teleported)
	{
		// possibly teleported or respawned
		oldz = ent->origin[2];
		extracrouch = 0;
		crouchspeed = 100;
		cl.crouch = 0;
		return;
	}

	if (cls.demoplayback && (cl.protocol == PROTOCOL_VERSION_QW))
	{
		// onground is calculated client-side in QW, therefore not stored in demo
		PM_CategorizePosition (ent);
	}
	
	if (cl.onground && ent->origin[2] - oldz > 0)
	{
		if (ent->origin[2] - oldz > 20)
		{
			// if on steep stairs, increase speed
			if (crouchspeed < 160)
			{
				extracrouch = ent->origin[2] - oldz - host_frametime * 200 - 15;
				extracrouch = min(extracrouch, 5);
			}
			crouchspeed = 160;
		}

		oldz += host_frametime * crouchspeed;
		if (oldz > ent->origin[2])
			oldz = ent->origin[2];

		if (ent->origin[2] - oldz > 15 + extracrouch)
			oldz = ent->origin[2] - 15 - extracrouch;
		extracrouch -= host_frametime * 200;
		extracrouch = max(extracrouch, 0);

		cl.crouch = oldz - ent->origin[2];
	}
	else
	{
		// in air or moving down
		oldz = ent->origin[2];
		cl.crouch += host_frametime * 150;
		if (cl.crouch > 0)
			cl.crouch = 0;
		crouchspeed = 100;
		extracrouch = 0;
	}
}

/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int	ret;

	cl.oldtime = cl.ctime;

// JDH: 2008-09-08: cl.time now updated in same way as cl.ctime
//	cl.time += host_frametime;

	if (cls.demoplayback)
	{
		if (!(cl.paused & 2))		// JDH
		{
			if (cl_demorewind.value)
			{
				cl.time -= host_frametime;
				cl.ctime -= host_frametime;
#ifdef _DEBUG
//				Con_Printf ("cl.ctime set to %lf\n", cl.ctime);
#endif
			}
			else
			{
				cl.time += host_frametime;
				cl.ctime += host_frametime;
			}
		}
	}
	else
	{
		cl.time += host_frametime;
		cl.ctime += host_frametime;
	}
/*
	if (!cl_demorewind.value || !cls.demoplayback)	// by joe
		cl.ctime += host_frametime;
	else
		cl.ctime -= host_frametime;
*/

	do {
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;

		cl.last_received_message = realtime;

		
#ifdef _DEBUG
		//if (!cl.paused)
		//	Con_DPrintf ("CL got message at %.2lf\n", realtime);
#endif

		CL_ParseServerMessage ();

	// JDH: moved this here so CL_ParseServerMessage can modify it (eg. remove alpha)
		if (cls.demorecording)
			CL_WriteDemoMessage (cls.demofile, cl.viewangles);

	} while (ret && (cls.state == ca_connected));

	if (cl_shownet.value && !cl.paused)
		Con_Print ("\n");

	CL_RelinkEntities ();
	if (cl.protocol == PROTOCOL_VERSION_QW)
		CL_LinkProjectiles ();

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	if (cls.signon == SIGNONS)
		CL_CalcCrouch ();

	CL_UpdateTEnts ();		// must be done AFTER linking entities

// bring the links up to date
	return 0;
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t	cmd;

	if (cls.state != ca_connected)
		return;

	if (cls.signon == SIGNONS)
	{
	// get basic movement from keyboard
		CL_BaseMove (&cmd);

	// allow mice or other external controllers to add to the move
		IN_Move (&cmd);

	// send the unreliable message
		CL_SendMove (&cmd);
	}

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

// send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all

	if (!NET_CanSendMessage(cls.netcon))
	{
		Con_DPrintf ("CL_SendCmd: can't send\n");
		return;
	}

	if (NET_SendMessage(cls.netcon, &cls.message) == -1)
		Host_Error ("CL_SendCmd: lost server connection");

	SZ_Clear (&cls.message);
}

/*
=================
CL_Init
=================
*/
void CL_Init (void)
{
//	SZ_Alloc (&cls.message, 1024);
	SZ_Alloc (&cls.message, 8192);

	CL_InitInput ();
	CL_InitStrings ();
	CL_InitTEnts ();

#ifdef HEXEN2_SUPPORT
//	if ( hexen2 )
//		CL_InitEffects();	// JDH: empty function anyway
#endif

// register our commands
	Cvar_RegisterFloat (&cl_upspeed, 0, CVAR_UNKNOWN_MAX);
	Cvar_RegisterFloat (&cl_forwardspeed, 0, CVAR_UNKNOWN_MAX);
	Cvar_RegisterFloat (&cl_backspeed, 0, CVAR_UNKNOWN_MAX);
	Cvar_RegisterFloat (&cl_sidespeed, 0, CVAR_UNKNOWN_MAX);
	Cvar_Register (&cl_movespeedkey);
	Cvar_Register (&cl_yawspeed);
	Cvar_Register (&cl_pitchspeed);
	Cvar_Register (&cl_anglespeedkey);
	Cvar_RegisterInt (&cl_shownet, 0, 2);
	Cvar_RegisterBool (&cl_nolerp);
	Cvar_RegisterBool (&lookspring);
	Cvar_RegisterBool (&lookstrafe);
	Cvar_RegisterFloat (&sensitivity, 1, 11);

	Cvar_RegisterFloat (&m_pitch, -1, 1);
	Cvar_RegisterFloat (&m_yaw, -1, 1);		// JDH: is range correct??
	Cvar_Register (&m_forward);
	Cvar_Register (&m_side);

	// by joe
	Cvar_RegisterFloat (&cl_truelightning, 0, 1);
	Cvar_RegisterBool (&cl_rocket2grenade);
	Cvar_RegisterBool (&cl_muzzleflash);

	Cvar_RegisterBool (&cl_demorewind);
	Cvar_RegisterInt  (&cl_demo_compress, 0, 2);		// JDH
	Cvar_RegisterInt  (&cl_demo_compress_fmt, 0, 1);	// JDH

	Cvar_RegisterBool (&r_powerupglow);
	Cvar_RegisterInt (&r_explosiontype, 0, 3);
	Cvar_RegisterFloat (&r_explosionlight, 0, 1);
	Cvar_RegisterFloat (&r_rocketlight, 0, 1);
//#ifdef GLQUAKE
	Cvar_RegisterInt (&r_explosionlightcolor, 0, 4);
	Cvar_RegisterInt (&r_rocketlightcolor, 0, 4);
//#endif
	Cvar_RegisterInt (&r_rockettrail, 0, 3);
	Cvar_RegisterInt (&r_grenadetrail, 0, 3);

	Cvar_RegisterBool (&cl_bobbing);
	Cvar_RegisterFloat (&cl_demospeed, 0.1, 20);
	Cvar_RegisterInt (&cl_deadbodyfilter, 0, 2);
	Cvar_RegisterBool (&cl_gibfilter);
	Cvar_RegisterInt (&cl_advancedcompletion, 0, 2);

	//Cvar_RegisterVariable (&cl_maxfps);		// moved to host.c as host_maxfps

	Cmd_AddCommand ("entities", CL_PrintEntities_f, 0);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f, 0);
	Cmd_AddCommand ("record", CL_Record_f, 0);
	Cmd_AddCommand ("stop", CL_Stop_f, 0);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f, 0);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f, 0);
}

#endif		//#ifndef RQM_SV_ONLY
