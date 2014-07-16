/*
Copyright (C) 2000	LordHavoc, Ender

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
// nehahra.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "music.h"

cvar_t	gl_notrans	= {"gl_notrans", "0"};
float	r_modelalpha;

char	prev_skybox[64];

cvar_t	r_waterripple = {"r_waterripple", "0"};		// JDH: default was 3 in nehahra

//extern cvar_t	gl_glows;
extern cvar_t	gl_hwblend;
extern int		sound_started;

int	num_sfxorig;

// JDH: vars for preserving cvar values when initializing Nehahra
float old_hwblend;
//float old_glows;
//char *old_fkey_binding[10];

#endif		//#ifndef RQM_SV_ONLY


neh_gametype_t	NehGameType = NEH_TYPE_NONE;

// cutscene demo usage
cvar_t  nehx00	= {"nehx00", "0"};
cvar_t	nehx01	= {"nehx01", "0"};
cvar_t  nehx02	= {"nehx02", "0"};
cvar_t	nehx03	= {"nehx03", "0"};
cvar_t  nehx04	= {"nehx04", "0"};
cvar_t	nehx05	= {"nehx05", "0"};
cvar_t  nehx06	= {"nehx06", "0"};
cvar_t	nehx07	= {"nehx07", "0"};
cvar_t  nehx08	= {"nehx08", "0"};
cvar_t	nehx09	= {"nehx09", "0"};
cvar_t  nehx10	= {"nehx10", "0"};
cvar_t	nehx11	= {"nehx11", "0"};
cvar_t  nehx12	= {"nehx12", "0"};
cvar_t	nehx13	= {"nehx13", "0"};
cvar_t  nehx14	= {"nehx14", "0"};
cvar_t	nehx15	= {"nehx15", "0"};
cvar_t  nehx16	= {"nehx16", "0"};
cvar_t	nehx17	= {"nehx17", "0"};
cvar_t  nehx18	= {"nehx18", "0"};
cvar_t	nehx19	= {"nehx19", "0"};
cvar_t  cutscene = {"cutscene", "1"};

cvar_t	nospr32	= {"nospr32", "0"};

qboolean Neh_CheckMode (void);

/*
================
Neh_Init
  Register cmds & cvars that are originally from Nehahra, 
  but are now always available
================
*/
void Neh_Init (void)
{
	Cvar_RegisterBool (&nospr32);

#ifndef RQM_SV_ONLY
	Cvar_RegisterBool (&gl_notrans);

	Cvar_Register (&r_waterripple);

/*	if (Music_IsInitialized())
	{
		Cmd_AddCommand ("stopmod", Music_Stop);
		Cmd_AddCommand ("playmod", Music_PlayMOD_f);
	}
*/
#endif
}

/*
// no longer needed, since arrow keys now control demo speed
void Neh_DoBindings (void)
{
	int		i, keynum = K_F1;
	float	speed;
	char	binding[32];

	for (i = 0; i < 10; i++)
	{
		speed = i/4 + 0.4 + 0.2*(i%4);
		Q_snprintfz (binding, sizeof(binding), "cl_demospeed %.1f", speed);
		old_fkey_binding[i] = Key_SwapBinding (keynum++, binding);
	}

#if 0
	Cbuf_AddText ("bind F1 \"cl_demospeed 0.4\"\n");
	Cbuf_AddText ("bind F2 \"cl_demospeed 0.6\"\n");
	Cbuf_AddText ("bind F3 \"cl_demospeed 0.8\"\n");
	Cbuf_AddText ("bind F4 \"cl_demospeed 1.0\"\n");

	Cbuf_AddText ("bind F5 \"cl_demospeed 1.4\"\n");
	Cbuf_AddText ("bind F6 \"cl_demospeed 1.6\"\n");
	Cbuf_AddText ("bind F7 \"cl_demospeed 1.8\"\n");
	Cbuf_AddText ("bind F8 \"cl_demospeed 2.0\"\n");

	Cbuf_AddText ("bind F9 \"cl_demospeed 2.4\"\n");
	Cbuf_AddText ("bind F10 \"cl_demospeed 2.6\"\n");
#endif
//	Cbuf_AddText ("bind PAUSE pausedemo\n");
}


void Neh_UndoBindings (void)
{
	int		i, keynum = K_F1;

	for (i = 0; i < 10; i++)
	{
		if (old_fkey_binding[i])
		{
			Key_SetBinding (keynum++, old_fkey_binding[i]);
			Z_Free (old_fkey_binding[i]);
			old_fkey_binding[i] = NULL;
		}
		else Key_SetBinding (keynum++, "");
	}
}
*/

/*
================
Neh_InitEnv
  Register cmds & cvars that are available only when Nehahra is active
================
*/
void Neh_InitEnv (void)
{
	if (!Neh_CheckMode ())
	{
		nehahra = false;
		return;
	}

	// Nehahra uses these to pass data around cutscene demos
	Cvar_Register (&nehx00);
	Cvar_Register (&nehx01);
	Cvar_Register (&nehx02);
	Cvar_Register (&nehx03);
	Cvar_Register (&nehx04);
	Cvar_Register (&nehx05);
	Cvar_Register (&nehx06);
	Cvar_Register (&nehx07);
	Cvar_Register (&nehx08);
	Cvar_Register (&nehx09);
	Cvar_Register (&nehx10);
	Cvar_Register (&nehx11);
	Cvar_Register (&nehx12);
	Cvar_Register (&nehx13);
	Cvar_Register (&nehx14);
	Cvar_Register (&nehx15);
	Cvar_Register (&nehx16);
	Cvar_Register (&nehx17);
	Cvar_Register (&nehx18);
	Cvar_Register (&nehx19);
	Cvar_Register (&cutscene);

	Cmd_AddLegacyCommand ("slowmo", "cl_demospeed");
	Cmd_AddLegacyCommand ("pausedemo", "pause");

	Cmd_AddLegacyCommand ("max", "god");
	Cmd_AddLegacyCommand ("monster", "notarget");
	Cmd_AddLegacyCommand ("scrag", "fly");
	Cmd_AddLegacyCommand ("wraith", "noclip");
	Cmd_AddLegacyCommand ("gimme", "give");
}

/*
================
Neh_InitEnv
  Initialize vars in preparation for running Nehahra
================
*/
void Neh_InitVars (void)
{
#ifndef RQM_SV_ONLY
	old_hwblend = gl_hwblend.value;
	Cvar_SetDirect (&gl_hwblend, "0");		// JDH: is this still necessary?

//	old_glows = gl_glows.value;
//	Cvar_SetDirect (&gl_glows, "1");

	if (COM_CheckParm("-matrox"))
		Cvar_SetDirect (&nospr32, "1");

//	if (NehGameType == NEH_TYPE_DEMO)
//		Neh_DoBindings ();
#endif
}

void Neh_UninitEnv (void)
{
#ifndef RQM_SV_ONLY
	Cvar_SetValueDirect (&gl_hwblend, old_hwblend);
//	Cvar_SetValueDirect (&gl_glows, old_glows);
#endif

	// Nehahra uses these to pass data around cutscene demos
	Cvar_Unregister (&nehx00);
	Cvar_Unregister (&nehx01);
	Cvar_Unregister (&nehx02);
	Cvar_Unregister (&nehx03);
	Cvar_Unregister (&nehx04);
	Cvar_Unregister (&nehx05);
	Cvar_Unregister (&nehx06);
	Cvar_Unregister (&nehx07);
	Cvar_Unregister (&nehx08);
	Cvar_Unregister (&nehx09);
	Cvar_Unregister (&nehx10);
	Cvar_Unregister (&nehx11);
	Cvar_Unregister (&nehx12);
	Cvar_Unregister (&nehx13);
	Cvar_Unregister (&nehx14);
	Cvar_Unregister (&nehx15);
	Cvar_Unregister (&nehx16);
	Cvar_Unregister (&nehx17);
	Cvar_Unregister (&nehx18);
	Cvar_Unregister (&nehx19);
	Cvar_Unregister (&cutscene);

	Cmd_RemoveLegacyCommand ("slowmo");
	Cmd_RemoveLegacyCommand ("pausedemo");

	Cmd_RemoveLegacyCommand ("max");
	Cmd_RemoveLegacyCommand ("monster");
	Cmd_RemoveLegacyCommand ("scrag");
	Cmd_RemoveLegacyCommand ("wraith");
	Cmd_RemoveLegacyCommand ("gimme");

//	if (NehGameType == NEH_TYPE_DEMO)
//		Neh_UndoBindings ();

	NehGameType = NEH_TYPE_NONE;
}


qboolean Neh_CheckMode (void)
{
	qboolean	movieinstalled = false, gameinstalled = false;

	Con_SafePrintf ("Beginning Nehahra check...\n");

// Check for movies
	if (COM_FindFile("hearing.dem", 0, NULL) || COM_FindFile("hearing.dz", 0, NULL))
	{
		movieinstalled = true;
		Con_SafePrintf ("  Found Movie...\n");
	}

// Check for game
	if (COM_FindFile("maps/neh1m4.bsp", 0, NULL))
	{
		gameinstalled = true;
		Con_SafePrintf ("  Found Game...\n");
	}

	if (gameinstalled && movieinstalled)
	{
		NehGameType = NEH_TYPE_BOTH;             // mainmenu.lmp
		Con_SafePrintf ("Running Both Movie and Game\n");
		return true;
	}

	if (gameinstalled && !movieinstalled)
	{
		NehGameType = NEH_TYPE_GAME;             // gamemenu.lmp
		Con_SafePrintf ("Running Game Nehahra\n");
		return true;
	}

	if (!gameinstalled && movieinstalled)
	{
		NehGameType = NEH_TYPE_DEMO;             // demomenu.lmp
		Con_SafePrintf ("Running Movie Nehahra\n");
		return true;
	}

//	Sys_Error ("Nehahra is not properly installed!");
	Con_SafePrintf ("\x02Nehahra is not properly installed!\n");
	return false;
}


#ifndef RQM_SV_ONLY

void Neh_ResetSFX (void)
{
	int	i;

/*****JDH*****/
	if (!sound_started)
		return;
/*****JDH*****/

	if (num_sfxorig == 0)
		num_sfxorig = num_sfx;

	num_sfx = num_sfxorig;
	Con_DPrintf ("Neh_ResetSFX: current # SFX: %d\n", num_sfx);

	for (i=num_sfx+1 ; i<MAX_SFX ; i++)
	{
		strcpy (known_sfx[i].name, "dfw3t23EWG#@T#@");
		if (known_sfx[i].cache.data)
			Cache_Free (&known_sfx[i].cache);
	}
}

#define SHOWLMP_MAXLABELS	256
typedef struct showlmp_s
{
	qboolean	isactive;
	float		x;
	float		y;
	char		label[32];
	char		pic[128];
} showlmp_t;

showlmp_t	showlmp[SHOWLMP_MAXLABELS];

void SHOWLMP_decodehide (void)
{
	int	    i;
	char	*lmplabel;

	lmplabel = MSG_ReadString ();
	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
	{
		if (showlmp[i].isactive && !strcmp(showlmp[i].label, lmplabel))
		{
			showlmp[i].isactive = false;
			return;
		}
	}
}

void SHOWLMP_decodeshow (void)
{
	int	    i, k;
	char	lmplabel[256], picname[256];
	float	x, y;

	Q_strcpy (lmplabel, MSG_ReadString(), sizeof(lmplabel));
	Q_strcpy (picname, MSG_ReadString(), sizeof(picname));
	x = MSG_ReadByte ();
	y = MSG_ReadByte ();
	k = -1;
	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
	{
		if (showlmp[i].isactive)
		{
			if (!strcmp(showlmp[i].label, lmplabel))
			{
				k = i;
				break;	// drop out to replace it
			}
		}
		else if (k < 0)	// find first empty one to replace
		{
			k = i;
		}
	}

	if (k < 0)
		return;	// none found to replace
	// change existing one
	showlmp[k].isactive = true;
	Q_strcpy (showlmp[k].label, lmplabel, sizeof(showlmp[k].label));
	Q_strcpy (showlmp[k].pic, picname, sizeof(showlmp[k].pic));
	showlmp[k].x = x;
	showlmp[k].y = y;
}

void SHOWLMP_drawall (void)
{
	int	i;

	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
		if (showlmp[i].isactive)
			Draw_TransPic (showlmp[i].x, showlmp[i].y, Draw_GetCachePic(showlmp[i].pic, true));
}

void SHOWLMP_clear (void)
{
	int	i;

	for (i=0 ; i<SHOWLMP_MAXLABELS ; i++)
		showlmp[i].isactive = false;
}

#endif	// #ifdef RQM_SV_ONLY
