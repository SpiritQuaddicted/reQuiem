
#include "quakedef.h"

#ifdef HEXEN2_SUPPORT

#ifndef RQM_SV_ONLY

#include "music.h"


float oldfbmodels  = 1;
float oldfbbmodels = 1;
float oldfbworld = 1;
float oldsidespeed = 350;

cvar_t	bgmtype				= {"bgmtype", "midi", CVAR_FLAG_ARCHIVE};   // cd/midi/none
cvar_t	scr_hudspeed		= {"scr_hudspeed", "8", CVAR_FLAG_ARCHIVE};

#endif		//#ifndef RQM_SV_ONLY


const char *ClassNames[NUM_CLASSES] =
{
	"Paladin",
	"Crusader",
	"Necromancer",
	"Assassin",
	"Demoness"
};

cvar_t	cl_playerclass		= {"_cl_playerclass", "1"};
cvar_t	max_temp_edicts		= {"max_temp_edicts", "30"};
cvar_t	sv_sound_distance	= {"sv_sound_distance", "800"};
cvar_t	v_centerrollspeed	= {"v_centerrollspeed", "125"};
cvar_t	randomclass			= {"randomclass", "0"};		// 0, 1, or 2
cvar_t	sv_idealrollscale	= {"sv_idealrollscale", "0.8"};
cvar_t	sv_flypitch			= {"sv_flypitch", "20"};
cvar_t	sv_walkpitch		= {"sv_walkpitch", "0"};
cvar_t	sv_ce_scale			= {"sv_ce_scale", "0", CVAR_FLAG_ARCHIVE};
cvar_t	sv_ce_max_size		= {"sv_ce_max_size", "0", CVAR_FLAG_ARCHIVE};

/*
cvar_t	sv_update_player	= {"sv_update_player","1", CVAR_FLAG_ARCHIVE};
cvar_t	sv_update_monsters	= {"sv_update_monsters","1", CVAR_FLAG_ARCHIVE};
cvar_t	sv_update_missiles	= {"sv_update_missiles","1", CVAR_FLAG_ARCHIVE};
cvar_t	sv_update_misc		= {"sv_update_misc","1", CVAR_FLAG_ARCHIVE};
*/

qboolean has_portals = false;

int		*pr_string_index = NULL;
char	*pr_global_strings = NULL;
int		pr_string_count = 0;

int		*pr_info_string_index = NULL;
char	*pr_global_info_strings = NULL;
int		pr_info_string_count = 0;

/*
===============
PR_LoadInfoStrings - Hexen II support for infolist.txt
===============
*/
void PR_LoadInfoStrings(void)
{
	int i,count,start,Length;
	int NewLineChar;

//	pr_global_info_strings = (char *)COM_LoadHunkFile ("infolist.txt", 0);
	pr_global_info_strings = (char *)COM_LoadMallocFile ("infolist.txt", 0);
	if (!pr_global_info_strings)
		Sys_Error ("PR_LoadInfoStrings: couldn't load infolist.txt");

	NewLineChar = -1;

	for(i=count=0; pr_global_info_strings[i] != 0; i++)
	{
		if (pr_global_info_strings[i] == 13 || pr_global_info_strings[i] == 10)
		{
			if (NewLineChar == pr_global_info_strings[i] || NewLineChar == -1)
			{
				NewLineChar = pr_global_info_strings[i];
				count++;
			}
		}
	}
	Length = i;

	if (!count)
	{
		Sys_Error ("PR_LoadInfoStrings: no string lines found");
	}

//	pr_info_string_index = (int *)Hunk_AllocName ((count+1)*4, "info_string_index");
	pr_info_string_index = (int *)Q_malloc ((count+1)*4);

	for(i=count=start=0; pr_global_info_strings[i] != 0; i++)
	{
		if (pr_global_info_strings[i] == 13 || pr_global_info_strings[i] == 10)
		{
			if (NewLineChar == pr_global_info_strings[i])
			{
				pr_info_string_index[count] = start;
				start = i+1;
				count++;
			}
			else start++;

			pr_global_info_strings[i] = 0;
		}
	}

	pr_info_string_count = count;
	Con_Printf("Read in %d objectives\n",count);
}

/*
===============
PR_LoadStrings - Hexen II support for strings.txt
===============
*/
void PR_LoadStrings(void)
{
	int i, count, start, length;
	int nlChar;

//	pr_global_strings = (char *) COM_LoadHunkFile ("strings.txt", 0);
	pr_global_strings = (char *) COM_LoadMallocFile ("strings.txt", 0);
	if (!pr_global_strings)
		Sys_Error ("PR_LoadStrings: couldn't load strings.txt");

	nlChar = -1;

	// count the number of strings (using CR/LF as separator)
	for ( i = count = 0; pr_global_strings[i]; i++ )
	{
		if ( pr_global_strings[i] == 13 || pr_global_strings[i] == 10 )
		{
			if ( nlChar == pr_global_strings[i] || nlChar == -1 )
			{
				nlChar = pr_global_strings[i];
				count++;
			}
		}
	}
	length = i;

	if (!count)
	{
		Sys_Error ("PR_LoadStrings: no string lines found");
	}


	// build a table of offsets
//	pr_string_index = (int *) Hunk_AllocName ((count+1)*4, "string_index");
	pr_string_index = (int *) Q_malloc ((count+1)*4);

	for ( i = count = start = 0; pr_global_strings[i]; i++ )
	{
		if ( pr_global_strings[i] == 13 || pr_global_strings[i] == 10 )
		{
			if ( nlChar == pr_global_strings[i] )
			{
				pr_string_index[count] = start;
				start = i+1;
				count++;
			}
			else start++;

			pr_global_strings[i] = 0;
		}
	}

	pr_string_count = count;
	Con_Printf( "Read in %d string lines\n", count );
}

/*
===============
PR_UnloadStrings
===============
*/
void PR_UnloadStrings (void)
{
	if (pr_global_strings)
	{
		free (pr_global_strings);
		pr_global_strings = NULL;
	}

	if (pr_string_index)
	{
		free (pr_string_index);
		pr_string_index = NULL;
	}

	if (pr_global_info_strings)
	{
		free (pr_global_info_strings);
		pr_global_info_strings = NULL;
	}

	if (pr_info_string_index)
	{
		free (pr_info_string_index);
		pr_info_string_index = NULL;
	}
}

/*
==============
Hexen2_RegisterVars
==============
*/
void Hexen2_RegisterVars (void)
{
#ifndef RQM_SV_ONLY
	Cvar_RegisterString (&bgmtype);
	Cvar_Register (&scr_hudspeed);
#endif

	Cvar_RegisterInt (&cl_playerclass, 0, NUM_CLASSES);
	Cvar_Register (&max_temp_edicts);
	Cvar_Register (&v_centerrollspeed);
	Cvar_Register (&randomclass);
	Cvar_Register (&sv_sound_distance);
	Cvar_Register (&sv_idealrollscale);
	Cvar_Register (&sv_flypitch);
	Cvar_Register (&sv_walkpitch);
	Cvar_Register (&sv_ce_scale);
	Cvar_Register (&sv_ce_max_size);
}

/*
==============
Hexen2_UnregisterVars
==============
*/
void Hexen2_UnregisterVars (void)
{
#ifndef RQM_SV_ONLY
	Cvar_Unregister (&bgmtype);
	Cvar_Unregister (&scr_hudspeed);
#endif

	Cvar_Unregister (&cl_playerclass);
	Cvar_Unregister (&max_temp_edicts);
	Cvar_Unregister (&v_centerrollspeed);
	Cvar_Unregister (&randomclass);
	Cvar_Unregister (&sv_sound_distance);
	Cvar_Unregister (&sv_idealrollscale);
	Cvar_Unregister (&sv_flypitch);
	Cvar_Unregister (&sv_walkpitch);
	Cvar_Unregister (&sv_ce_scale);
	Cvar_Unregister (&sv_ce_max_size);
}

/*
==============
Hexen2_InitVars
==============
*/
void Hexen2_InitVars (void)
{
#ifndef RQM_SV_ONLY
	oldfbmodels = gl_fb_models.value;
	Cvar_SetDirect (&gl_fb_models, "0");

	oldfbbmodels = gl_fb_bmodels.value;
	Cvar_SetDirect (&gl_fb_bmodels, "0");

	oldfbworld = gl_fb_world.value;
	Cvar_SetDirect (&gl_fb_world, "0");

	oldsidespeed = cl_sidespeed.value;
	Cvar_SetDirect (&cl_sidespeed, "225");
#endif
}

/*
==============
Hexen2_InitEnv
==============
*/
void Hexen2_InitEnv (void)
{
#ifndef RQM_SV_ONLY
	Cmd_AddGameCommand ("+crouch", IN_CrouchDown, 0);
	Cmd_AddGameCommand ("-crouch", IN_CrouchUp, 0);

	Cmd_AddGameCommand ("+infoplaque", IN_infoPlaqueDown, 0);
	Cmd_AddGameCommand ("-infoplaque", IN_infoPlaqueUp, 0);

	Cmd_AddGameCommand ("df", V_DarkFlash_f, 0);
	Cmd_AddGameCommand ("wf", V_WhiteFlash_f, 0);

	Cmd_AddGameCommand ("+showinfo", Sbar_ShowInfoDown_f, 0);
	Cmd_AddGameCommand ("-showinfo", Sbar_ShowInfoUp_f, 0);
	Cmd_AddGameCommand ("+showdm", Sbar_ShowDMDown_f, 0);
	Cmd_AddGameCommand ("-showdm", Sbar_ShowDMUp_f, 0);
	Cmd_AddGameCommand ("toggle_dm", Sbar_ToggleDM_f, 0);

  	Cmd_AddGameCommand ("invleft", Sbar_InvLeft_f, 0);
	Cmd_AddGameCommand ("invright", Sbar_InvRight_f, 0);
	Cmd_AddGameCommand ("invuse", Sbar_InvUse_f, 0);
	Cmd_AddGameCommand ("invoff", Sbar_InvOff_f, 0);

	if (!COM_CheckParm("-nomidi"))
	{
		if (Music_IsInitialized())
		{
			Cmd_AddGameCommand ("midi_play",   Music_PlayMIDI_f, 0);
			Cmd_AddGameCommand ("midi_stop",   Music_Stop_f, 0);
			Cmd_AddGameCommand ("midi_pause",  Music_Pause_f, 0);
			Cmd_AddGameCommand ("midi_loop",   Music_Loop_f, 0);
			Cmd_AddGameCommand ("midi_volume", Music_ChangeVolume_f, 0);
		}
	}

	Con_UpdateChars_H2 (true);
#endif

	Cmd_AddGameCommand ("playerclass", Host_Class_f, 0);
	Cmd_AddGameCommand ("changelevel2", Host_Changelevel2_f, 0);
	
	has_portals = COM_FindFile ("gfx/cport5.lmp", 0, NULL);

	Hexen2_RegisterVars ();
	PR_LoadStrings ();
	PR_LoadInfoStrings ();
}

/*
==============
Hexen2_UninitEnv
==============
*/
void Hexen2_UninitEnv (void)
{
	Cmd_RemoveGameCommand ("+crouch");
	Cmd_RemoveGameCommand ("-crouch");

	Cmd_RemoveGameCommand ("+infoplaque");
	Cmd_RemoveGameCommand ("-infoplaque");

	Cmd_RemoveGameCommand ("df");
	Cmd_RemoveGameCommand ("wf");

	Cmd_RemoveGameCommand ("+showinfo");
	Cmd_RemoveGameCommand ("-showinfo");
	Cmd_RemoveGameCommand ("+showdm");
	Cmd_RemoveGameCommand ("-showdm");
	Cmd_RemoveGameCommand ("toggle_dm");

	Cmd_RemoveGameCommand ("playerclass");
	Cmd_RemoveGameCommand ("changelevel2");

  	Cmd_RemoveGameCommand ("invleft");
	Cmd_RemoveGameCommand ("invright");
	Cmd_RemoveGameCommand ("invuse");
	Cmd_RemoveGameCommand ("invoff");

#ifndef RQM_SV_ONLY
	if (Cmd_Exists ("midi_play"))
	{
		Cmd_RemoveGameCommand ("midi_play");
		Cmd_RemoveGameCommand ("midi_stop");
		Cmd_RemoveGameCommand ("midi_pause");
		Cmd_RemoveGameCommand ("midi_loop");
		Cmd_RemoveGameCommand ("midi_volume");
	}

	Cvar_SetValueDirect (&gl_fb_models,  oldfbmodels);
	Cvar_SetValueDirect (&gl_fb_bmodels, oldfbbmodels);
	Cvar_SetValueDirect (&gl_fb_world, oldfbworld);
	Cvar_SetValueDirect (&cl_sidespeed,  oldsidespeed);

	Con_UpdateChars_H2 (false);
#endif

	Hexen2_UnregisterVars ();
	PR_UnloadStrings ();
}

#endif		// #ifdef HEXEN2_SUPPORT
