
#include "quakedef.h"

#define	SAVEGAME_VERSION	5

int	current_skill;


#ifdef HEXEN2_SUPPORT

#include <time.h>
extern cvar_t	cl_playerclass, randomclass;

//extern unsigned int	info_mask, info_mask2;

/*
=====================
Host_DeleteFile
- callback for COM_FindDirFiles in procedure below
=====================
*/
int Host_DeleteFile (com_fileinfo_t *fileinfo, int count, unsigned int tempdir)
{
	char	name[MAX_OSPATH];

	Q_snprintfz (name, sizeof(name), "%s%s", (char *) tempdir, fileinfo->name);
	COM_DeleteFile (name);

	return 0;		// continue searching
}

/*
=====================
Host_RemoveGIPFiles
=====================
*/
void Host_RemoveGIPFiles (const char *path)
{
	char	name[MAX_OSPATH], tempdir[MAX_OSPATH];
	int i;
//	HANDLE handle;
//	WIN32_FIND_DATA filedata;
//	BOOL retval;

	if (path)
	{
		Q_snprintfz(tempdir, sizeof(tempdir), "%s/", path);
	}
	else
	{
		i = COM_GetTempPath (tempdir, sizeof(tempdir));
		if (!i)
		{
			Q_snprintfz (tempdir, sizeof(tempdir), "%s/", com_gamedir);
		}
	}

	Q_snprintfz (name, sizeof(name), "%s*.gip", tempdir);

	COM_FindDirFiles (name, NULL, 0, Host_DeleteFile, (unsigned int) tempdir);

/*	handle = FindFirstFile(name,&filedata);
	retval = TRUE;

	while (handle != INVALID_HANDLE_VALUE && retval)
	{
		sprintf(name,"%s%s", tempdir,filedata.cFileName);
		DeleteFile(name);

		retval = FindNextFile(handle,&filedata);
	}

	if (handle != INVALID_HANDLE_VALUE)
		FindClose(handle);
*/
}

qboolean host_copyfileerror = false;

typedef struct copyinfo_s
{
	const char *source;
	const char *dest;
} copyinfo_t;

/*
=====================
Host_CopyFile
- callback for COM_FindDirFiles in procedure below
=====================
*/
int Host_CopyFile (com_fileinfo_t *fileinfo, int count, unsigned int copyinfo)
{
	char	name[MAX_OSPATH], tempdir[MAX_OSPATH];

	Q_snprintfz (name, sizeof(name), "%s%s", ((copyinfo_t *) copyinfo)->source, fileinfo->name);
	Q_snprintfz (tempdir, sizeof(tempdir), "%s%s", ((copyinfo_t *) copyinfo)->dest, fileinfo->name);

#ifdef _WIN32
	if (!CopyFileA (name, tempdir, FALSE))
#else
	if (!COM_CopyFile (name, tempdir))
#endif
		host_copyfileerror = true;

	return 0;		// continue searching
}

/*
=====================
Host_CopyFiles
=====================
*/
qboolean Host_CopyFiles (const char *source, const char *pat, const char *dest)
{
	copyinfo_t copyinfo;

	copyinfo.source = source;
	copyinfo.dest = dest;

	host_copyfileerror = false;
	COM_FindDirFiles (pat, NULL, 0, Host_CopyFile, (unsigned int) &copyinfo);
	return host_copyfileerror;

/*	char	name[MAX_OSPATH],tempdir[MAX_OSPATH];
	HANDLE handle;
	WIN32_FIND_DATA filedata;
	BOOL retval,error;

	handle = FindFirstFile(pat,&filedata);
	retval = TRUE;
	error = false;

	while (handle != INVALID_HANDLE_VALUE && retval)
	{
		sprintf(name,"%s%s", source, filedata.cFileName);
		sprintf(tempdir,"%s%s", dest, filedata.cFileName);
		if (!CopyFile(name,tempdir,FALSE))
			error = true;

		retval = FindNextFile(handle,&filedata);
	}

	if (handle != INVALID_HANDLE_VALUE)
		FindClose(handle);

	return error;
*/
}

#endif	//#ifdef HEXEN2_SUPPORT

/*
===============
Host_SavegameComment

Writes a SAVEGAME_COMMENT_LENGTH character comment describing the current
===============
*/
void Host_SavegameComment (char *text)
{
	int		i, len;
	const char *p;
#ifndef RQM_SV_ONLY
	char	kills[20];
#else
	extern const char * SV_MapTitle (void);
#endif

	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		text[i] = ' ';

#ifdef RQM_SV_ONLY
	p = SV_MapTitle ();
#else
	p = cl.levelname;
#endif

// JDH: skip CR/LF and truncate map titles

	//memcpy (text, cl.levelname, strlen(cl.levelname));

	for (len = 0; len < 22 && *p; p++)
	{
		if ((*p != '\n') && (*p != '\r'))
		{
			text[len++] = *p;
		}
	}

#ifndef RQM_SV_ONLY

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		#define ShortTime "%m/%d/%Y %H:%M"
		struct tm	*tblock;
		time_t		TempTime;

		TempTime = time(NULL);
		tblock = localtime(&TempTime);
		strftime(kills, sizeof(kills), ShortTime, tblock);

		memcpy (text+21, kills, strlen(kills));
	}
	else
#endif
	{
		sprintf (kills, "kills:%3i/%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
		memcpy (text+22, kills, strlen(kills));
	}
#endif

// convert space to _ to make stdio happy
	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		if (text[i] == ' ')
			text[i] = '_';
	text[SAVEGAME_COMMENT_LENGTH] = '\0';
}

/*
=====================
Host_SaveLights
=====================
*/
void Host_SaveLights (FILE *f)
{
	int i;

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		if (sv.lightstyles[i])
			fprintf (f, "%s\n", sv.lightstyles[i]);
		else
			fprintf (f,"m\n");
	}
}

#ifdef HEXEN2_SUPPORT
/*
=====================
Host_SaveGamestate
=====================
*/
void Host_SaveGamestate (qboolean ClientsOnly)
{
	char	name[MAX_OSPATH], tempdir[MAX_OSPATH];
	FILE	*f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];
	edict_t	*ent;
	int		start,end;

	i = COM_GetTempPath(tempdir, sizeof(tempdir));
	if (!i)
	{
		Q_snprintfz(tempdir, sizeof(tempdir), "%s/", com_savedir);
	}

	if (ClientsOnly)
	{
		start = 1;
		end = svs.maxclients+1;

		Q_snprintfz (name, sizeof(name), "%sclients.gip", tempdir);
	}
	else
	{
		start = 1;
		end = sv.num_edicts;

		Q_snprintfz (name, sizeof(name), "%s%s.gip", tempdir, sv.name);

//		Con_Printf ("Saving game to %s...\n", name);
	}

	f = fopen (name, "w");
	if (!f)
	{
		Con_Print ("ERROR: couldn't open.\n");
		return;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);

	if (!ClientsOnly)
	{
		Host_SavegameComment (comment);
		fprintf (f, "%s\n", comment);
		fprintf (f, "%f\n", skill.value);
		fprintf (f, "%s\n", sv.name);
		fprintf (f, "%f\n", sv.time);

		Host_SaveLights(f);
		SV_SaveEffects(f);

		fprintf(f, "-1\n");
		PR_WriteGlobals (f);
	}

	host_client = svs.clients;

//  to save the client states
	for (i=start ; i<end ; i++)
	{
		ent = EDICT_NUM(i);
		if ((int)ent->v.flags & FL_ARCHIVE_OVERRIDE)
			continue;
		if (ClientsOnly)
		{
			if (host_client->active)
			{
				fprintf (f, "%i\n",i);
				ED_Write (f, ent);
				fflush (f);
			}
			host_client++;
		}
		else
		{
			fprintf (f, "%i\n",i);
			ED_Write (f, ent);
			fflush (f);
		}
	}

	fclose (f);
}
#endif	// #ifdef HEXEN2_SUPPORT


#ifndef RQM_SV_ONLY
/*
===============
Host_Savegame_f
===============
*/
void Host_Savegame_f (cmd_source_t src)
{
	char	name[MAX_OSPATH];
	FILE	*f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];

	if (src == SRC_CLIENT)
		return;

	if (!sv.active)
	{
		Con_Print ("Not playing a local game.\n");
		return;
	}

	if (cl.intermission)
	{
		Con_Print ("Can't save in intermission.\n");
		return;
	}

	if (svs.maxclients != 1)
	{
		Con_Print ("Can't save multiplayer games.\n");
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Con_Print ("save <savename> : save a game\n");
		return;
	}

	if (strstr(Cmd_Argv(1), ".."))
	{
		Con_Print ("Relative pathnames are not allowed.\n");
		return;
	}

	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.health <= 0) )
		{
			Con_Print ("Can't savegame with a dead player\n");
			return;
		}
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		char	dest[MAX_OSPATH], tempdir[MAX_OSPATH];

		Host_SaveGamestate(false);

		Q_snprintfz (name, sizeof(name), "%s/%s", com_savedir, Cmd_Argv(1));
		Sys_mkdir (name);

		Host_RemoveGIPFiles(name);

		i = COM_GetTempPath(tempdir, sizeof(tempdir));
		if (!i)
		{
			Q_snprintfz(tempdir, sizeof(tempdir), "%s/", com_savedir);
		}

		Q_snprintfz (name, sizeof(name), "%sclients.gip", tempdir);
//		DeleteFile(name);
		COM_DeleteFile(name);

		Q_snprintfz (name, sizeof(name), "%s*.gip", tempdir);
		Q_snprintfz (dest, sizeof(dest), "%s/%s/", com_savedir, Cmd_Argv(1));
		Con_Printf ("Saving game to %s...\n", dest);

		Host_CopyFiles(tempdir, name, dest);

		Q_snprintfz(dest, sizeof(dest), "%s/%s/info.dat", com_savedir, Cmd_Argv(1));
		f = fopen (dest, "w");
	}
	else
#endif
	{
		Q_snprintfz (name, sizeof(name), "%s/%s", com_gamedir, Cmd_Argv(1));
		COM_ForceExtension (name, ".sav", sizeof(name));		// joe: force to ".sav"

		Con_Printf ("Saving game to %s...\n", name);
		f = fopen(name, "w");
	}

	if (!f)
	{
		Con_Print ("ERROR: couldn't open\n");
		return;
	}

	fprintf (f, "%i\n", SAVEGAME_VERSION);
	Host_SavegameComment (comment);
	fprintf (f, "%s\n", comment);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
	fprintf (f, "%d\n", current_skill);
	fprintf (f, "%s\n", sv.name);
	fprintf (f, "%f\n", sv.time);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		fprintf (f, "%d\n", svs.maxclients);
		fprintf (f, "%f\n", deathmatch.value);
		fprintf (f, "%f\n", coop.value);
		fprintf (f, "%f\n", teamplay.value);
		fprintf (f, "%f\n", randomclass.value);
		fprintf (f, "%f\n", cl_playerclass.value);
		fprintf (f, "%u\n", sv.info_mask);
		fprintf (f, "%u\n", sv.info_mask2);
	}
	else
#endif
	{
	// write the light styles
		Host_SaveLights(f);

		PR_WriteGlobals (f);
		for (i=0 ; i<sv.num_edicts ; i++)
		{
			ED_Write (f, EDICT_NUM(i));
			fflush (f);
		}
	}

	fclose (f);
	Con_Print ("done.\n");
}

#endif		//#ifndef RQM_SV_ONLY

/*
===============
Host_LoadLights
===============
*/
void Host_LoadLights (FILE *f, char *buf)
{
	int i;

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		fscanf (f, "%s\n", buf);
		sv.lightstyles[i] = Hunk_Alloc (strlen(buf)+1);
		strcpy (sv.lightstyles[i], buf);
	}
}

/*
===============
Host_LoadEdicts
===============
*/
int Host_LoadEdicts (FILE *f, char *buf, int bufsize, qboolean set_sr_flag)
{
	int		entnum, i, r;
	const char	*start;
	edict_t *ent;
#ifdef HEXEN2_SUPPORT
	qboolean auto_correct = false;
#endif

	entnum = -1;		// -1 is the globals

	while (!feof(f))
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			fscanf (f, "%i\n",&entnum);
	#endif

		for (i=0 ; i<bufsize-1 ; i++)
		{
			r = fgetc (f);
			if (r == EOF || !r)
				break;
			buf[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i == bufsize-1)
			Sys_Error ("Loadgame buffer overflow");
		buf[i] = 0;
//		start = buf;
		start = COM_Parse (buf);
		if (!com_token[0])
			break;		// end of file
		if (strcmp(com_token, "{"))
			Sys_Error ("First token isn't a brace");

		if (entnum == -1)
		{	// parse the global vars
			PR_ParseGlobals (start);

		#ifdef HEXEN2_SUPPORT
			// Need to restore this
			if (hexen2)
				*pr_global_ptrs.startspot = sv.startspot - pr_strings;
		#endif
		}
		else
		{	// parse an edict
			ent = EDICT_NUM(entnum);
			memset (&ent->v, 0, progs->entityfields * 4);

		#ifdef HEXEN2_SUPPORT
			if (!hexen2)
		#endif
				ent->free = false;

			ED_ParseEdict (start, ent);

		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				if (set_sr_flag)
					ent->v.stats_restored = true;
			}
		#endif

			// link it into the bsp tree
			if (!ent->free)
			{
				SV_LinkEdict (ent, false);

			#ifdef HEXEN2_SUPPORT
				if (hexen2)
				{
					if (ent->v.modelindex && ent->v.model)
					{
						i = SV_ModelIndex(ent->v.model + pr_strings, false);
						if (i != ent->v.modelindex)
						{
							ent->v.modelindex = i;
							auto_correct = true;
						}
					}
				}
			#endif
			}
		}

		entnum++;
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		return auto_correct;
#endif

	return entnum;
}

#ifdef HEXEN2_SUPPORT
/*
===============
Host_LoadGamestate
===============
*/
qboolean Host_LoadGamestate (const char *level, const char *startspot, int ClientsMode)
{
	char	name[MAX_OSPATH], tempdir[MAX_OSPATH];
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	time, sk;
	char	str[32768];
	int		i;
	int		version;
	qboolean auto_correct = false;

	i = COM_GetTempPath (tempdir, sizeof(tempdir));
	if (!i)
	{
		Q_snprintfz (tempdir, sizeof(tempdir), "%s/", com_savedir);
	}

	if (ClientsMode == 1)
	{
		Q_snprintfz (name, sizeof(name), "%sclients.gip", tempdir);
	}
	else
	{
		Q_snprintfz (name, sizeof(name), "%s%s.gip", tempdir, level);

		if (ClientsMode != 2 && ClientsMode != 3)
			Con_Printf ("Loading game from %s...\n", name);
	}

	f = fopen (name, "r");
	if (!f)
	{
		if (ClientsMode == 2)
			Con_Print ("ERROR: couldn't open.\n");

		return false;
	}

	fscanf (f, "%i\n", &version);

	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return false;
	}

	if (ClientsMode != 1)
	{
		fscanf (f, "%s\n", str);
		fscanf (f, "%f\n", &sk);
		Cvar_SetValueDirect (&skill, sk);

		fscanf (f, "%s\n",mapname);
		fscanf (f, "%f\n",&time);

		SV_SpawnServer (mapname, startspot);

		if (!sv.active)
		{
			Con_Print ("Couldn't load map\n");
			return false;
		}

		Host_LoadLights(f, str);
		SV_LoadEffects(f);
	}
	else time = 0;		// shut up compiler warning

// load the edicts out of the savegame file
	auto_correct = Host_LoadEdicts(f, str, sizeof(str), ClientsMode);

	fclose (f);

	switch (ClientsMode)
	{
		case 0:
			sv.paused = true;
		case 3:
			sv.time = time;
			*pr_global_ptrs.serverflags = svs.serverflags;
			Host_RestoreClients();
		case 2:
			if (ClientsMode == 2)		// since cases 0 and 3 also lead here
				sv.time = time;
			if (auto_correct)
				Con_DPrintf("*** Auto-corrected model indexes!\n");
	}

/*** JDH: original code below (I changed it to switch block above)
	if (ClientsMode == 0)
	{
		sv.time = time;
		sv.paused = true;

		pr_global_struct->serverflags = svs.serverflags;

		RestoreClients();
	}
	else if (ClientsMode == 2)
	{
		sv.time = time;
	}
	else if (ClientsMode == 3)
	{
		sv.time = time;

		pr_global_struct->serverflags = svs.serverflags;

		RestoreClients();
	}

	if (ClientsMode != 1 && auto_correct)
	{
		Con_DPrintf("*** Auto-corrected model indexes!\n");
	}
*/

	return true;
}

/*
===============
Host_LoadCvars_H2
===============
*/
void Host_LoadCvars_H2 (FILE *f)
{
	float	tempf;
	int		tempi;

	tempi = -1;
	fscanf (f, "%d\n", &tempi);
	if (tempi >= 1)
		svs.maxclients = tempi;

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	Cvar_SetValueDirect (&deathmatch, (tempf >= 0 ? tempf : 0));

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	Cvar_SetValueDirect (&coop, (tempf >= 0 ? tempf : 0));

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	Cvar_SetValueDirect (&teamplay, (tempf >= 0 ? tempf : 0));

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	Cvar_SetValueDirect (&randomclass, (tempf >= 0 ? tempf : 0));

	tempf = -1;
	fscanf (f, "%f\n", &tempf);
	if (tempf >= 0)
		Cvar_SetValueDirect (&cl_playerclass, tempf);
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
===============
Host_Loadgame_f
===============
*/
void Host_Loadgame_f (cmd_source_t src)
{
	char	name[MAX_OSPATH], mapname[MAX_QPATH], str[32768];
	float	time, tfloat, spawn_parms[NUM_SPAWN_PARMS];
	int		i, version;
	FILE	*f;
#ifdef HEXEN2_SUPPORT
	edict_t	*ent;
	char	dest[MAX_OSPATH], tempdir[MAX_OSPATH];
#endif

	if (src == SRC_CLIENT)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Print ("load <savename> : load a game\n");
		return;
	}

#ifndef RQM_SV_ONLY
	cls.demonum = -1;		// stop demo loop in case this fails
#endif

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
	#ifndef RQM_SV_ONLY
		CL_Disconnect (false);
	#endif
		Host_RemoveGIPFiles (NULL);

		Q_snprintfz (name, sizeof(name), "%s/%s", com_savedir, Cmd_Argv(1));
		Con_Printf ("Loading game from %s...\n", name);

		i = COM_GetTempPath (tempdir, sizeof(tempdir));
		if (!i)
		{
			Q_snprintfz (tempdir, sizeof(tempdir), "%s/", com_savedir);
		}

		Q_snprintfz (dest, sizeof(dest), "%s/info.dat", name);
		f = fopen (dest, "r");
	}
	else
#endif
	{
		Q_snprintfz (name, sizeof(name), "%s/%s", com_gamedir, Cmd_Argv(1));
		COM_DefaultExtension (name, ".sav", sizeof(name));

	// we can't call SCR_BeginLoadingPlaque, because too much stack space has
	// been used. The menu calls it before stuffing loadgame command
	//	SCR_BeginLoadingPlaque ();

		Con_Printf ("Loading game from %s...\n", name);
		f = fopen(name, "r");
	}

	if (!f)
	{
		Con_Print ("ERROR: couldn't open.\n");
		return;
	}

	fscanf (f, "%i\n", &version);
	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return;
	}

	fscanf (f, "%s\n", str);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fscanf (f, "%f\n", &spawn_parms[i]);

// this silliness is so we can load 1.06 save files, which have float skill values
	fscanf (f, "%f\n", &tfloat);
	current_skill = (int)(tfloat + 0.1);
	Cvar_SetValueDirect (&skill, (float)current_skill);

	fscanf (f, "%s\n", mapname);
	fscanf (f, "%f\n", &time);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		float playerclass;

		Host_LoadCvars_H2 (f);

		fscanf (f, "%u\n", &sv.info_mask);
		fscanf (f, "%u\n", &sv.info_mask2);
		fclose (f);

		Host_RemoveGIPFiles (tempdir);

		Q_snprintfz (name, sizeof(name), "%s/%s/*.gip", com_savedir, Cmd_Argv(1));
		Q_snprintfz (dest, sizeof(dest), "%s/%s/", com_savedir, Cmd_Argv(1));
		i = strlen (tempdir);
		Q_strcpy (tempdir+i, "/", sizeof(tempdir)-i);

		Host_CopyFiles (dest, name, tempdir);

		if (!Host_LoadGamestate (mapname, NULL, 2))
			return;

		SV_SaveSpawnparms ();

		ent = EDICT_NUM(1);
		playerclass = ent->v.playerclass;

		Cvar_SetValueDirect (&cl_playerclass, playerclass);	//this better be the same as above...

		// this may be rudundant with the setting in PR_LoadProgs, but not sure so its here too
		*pr_global_ptrs.cl_playerclass = playerclass;

		svs.clients->playerclass = playerclass;

		sv.paused = true;		// pause until all clients connect
		sv.loadgame = true;
	}
	else
#endif
	{
	#ifndef RQM_SV_ONLY
//		CL_Disconnect_f (src);		JDH: 2011/05/16: changed so demo recording continues
		
		CL_Disconnect (false);
		if (sv.active)
			Host_ShutdownServer (false);		// this may be redundant
	#endif

	#ifdef HEXEN2_SUPPORT
		SV_SpawnServer (mapname, NULL);
	#else
		SV_SpawnServer (mapname);
	#endif

		if (!sv.active)
		{
			Con_Print ("Couldn't load map\n");
			fclose (f);
			return;
		}
		sv.paused = true;		// pause until all clients connect
		sv.loadgame = true;

	// load the light styles
		Host_LoadLights (f, str);

	// load the edicts out of the savegame file
		sv.num_edicts = Host_LoadEdicts (f, str, sizeof(str), false);

		sv.time = time;
		fclose (f);

		for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
			svs.clients->spawn_parms[i] = spawn_parms[i];
	}

#ifndef RQM_SV_ONLY
	if (cls.state != ca_dedicated)
	{
		CL_EstablishConnection ("local");
		Host_Reconnect_f (src);
	}
#endif
}

