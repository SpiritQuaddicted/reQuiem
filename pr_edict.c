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
// sv_edict.c -- entity dictionary

#include "quakedef.h"

dprograms_t		*progs;
dfunction_t		*pr_functions;
char			*pr_strings;
ddef_t			*pr_fielddefs;
ddef_t			*pr_globaldefs;
dstatement_t	*pr_statements;
float			*pr_globals;			// same as pr_global_struct
int				pr_edict_size;			// in bytes

extern cvar_t	sv_fishfix;


#ifdef HEXEN2_SUPPORT

#include "progdefs_H2.h"

// Built-in Spawn Flags
#define SF_H2_NOT_PALADIN       0x00000100
#define SF_H2_NOT_CLERIC		0x00000200
#define SF_H2_NOT_NECROMANCER	0x00000400
#define SF_H2_NOT_THEIF			0x00000800
#define	SF_H2_NOT_EASY			0x00001000
#define	SF_H2_NOT_MEDIUM		0x00002000
#define	SF_H2_NOT_HARD		    0x00004000
#define	SF_H2_NOT_DEATHMATCH	0x00008000
#define SF_H2_NOT_COOP			0x00010000
#define SF_H2_NOT_SINGLE		0x00020000
#define SF_H2_NOT_DEMON			0x00040000

extern cvar_t			max_temp_edicts, cl_playerclass;

#ifndef RQM_SV_ONLY
	extern int	current_loading_size, entity_file_size;
#endif

#endif		// #ifdef HEXEN2_SUPPORT


typedef struct fieldoffset_t
{
	char	*name;
	int		ofs;
} fieldoffset_t;

static fieldoffset_t STD_FIELD_OFFSETS[] =
{
	{ "modelindex",		 0 },
	{ "absmin",			 1 },
	{ "absmin_x",		 1 },
	{ "absmin_y",		 2 },
	{ "absmin_z",		 3 },
	{ "absmax",			 4 },
	{ "absmax_x",		 4 },
	{ "absmax_y",		 5 },
	{ "absmax_z",		 6 },
	{ "ltime",			 7 },
	{ "movetype",		 8 },
	{ "solid", 			 9 },
	{ "origin",			10 },
	{ "origin_x",		10 },
	{ "origin_y",		11 },
	{ "origin_z",		12 },
	{ "oldorigin",	 	13 },
	{ "oldorigin_x", 	13 },
	{ "oldorigin_y", 	14 },
	{ "oldorigin_z", 	15 },
	{ "velocity", 		16 },
	{ "velocity_x", 	16 },
	{ "velocity_y", 	17 },
	{ "velocity_z", 	18 },
	{ "angles", 		19 },
	{ "angles_x", 		19 },
	{ "angles_y", 		20 },
	{ "angles_z", 		21 },
	{ "avelocity", 		22 },
	{ "avelocity_x", 	22 },
	{ "avelocity_y", 	23 },
	{ "avelocity_z", 	24 },
	{ "punchangle", 	25 },
	{ "punchangle_x", 	25 },
	{ "punchangle_y", 	26 },
	{ "punchangle_z", 	27 },
	{ "classname", 		28 },
	{ "model", 			29 },
	{ "frame", 			30 },
	{ "skin", 			31 },
	{ "effects", 		32 },
	{ "mins", 			33 },
	{ "mins_x", 		33 },
	{ "mins_y", 		34 },
	{ "mins_z", 		35 },
	{ "maxs", 			36 },
	{ "maxs_x", 		36 },
	{ "maxs_y", 		37 },
	{ "maxs_z", 		38 },
	{ "size", 			39 },
	{ "size_x", 		39 },
	{ "size_y", 		40 },
	{ "size_z", 		41 },
	{ "touch", 			42 },
	{ "use", 			43 },
	{ "think", 			44 },
	{ "blocked", 		45 },
	{ "nextthink", 		46 },
	{ "groundentity", 	47 },
	{ "health", 		48 },
	{ "frags", 			49 },
	{ "weapon", 		50 },
	{ "weaponmodel", 	51 },
	{ "weaponframe", 	52 },
	{ "currentammo",	53 },
	{ "ammo_shells", 	54 },
	{ "ammo_nails", 	55 },
	{ "ammo_rockets", 	56 },
	{ "ammo_cells", 	57 },
	{ "items", 			58 },
	{ "takedamage", 	59 },
	{ "chain", 			60 },
	{ "deadflag", 		61 },
	{ "view_ofs", 		62 },
	{ "view_ofs_x", 	62 },
	{ "view_ofs_y", 	63 },
	{ "view_ofs_z", 	64 },
	{ "button0", 		65 },
	{ "button1", 		66 },
	{ "button2", 		67 },
	{ "impulse", 		68 },
	{ "fixangle", 		69 },
	{ "v_angle", 		70 },
	{ "v_angle_x", 		70 },
	{ "v_angle_y", 		71 },
	{ "v_angle_z", 		72 },
	{ "idealpitch", 	73 },
	{ "netname", 		74 },
	{ "enemy",			75 },
	{ "flags", 			76 },
	{ "colormap", 		77 },
	{ "team",			78 },
	{ "max_health", 	79 },
	{ "teleport_time", 	80 },
	{ "armortype",		81 },
	{ "armorvalue", 	82 },
	{ "waterlevel", 	83 },
	{ "watertype", 		84 },
	{ "ideal_yaw", 		85 },
	{ "yaw_speed", 		86 },
	{ "aiment", 		87 },
	{ "goalentity", 	88 },
	{ "spawnflags", 	89 },
	{ "target", 		90 },
	{ "targetname", 	91 },
	{ "dmg_take", 		92 },
	{ "dmg_save",		93 },
	{ "dmg_inflictor", 	94 },
	{ "owner",			95 },
	{ "movedir", 		96 },
	{ "movedir_x", 		96 },
	{ "movedir_y", 		97 },
	{ "movedir_z", 		98 },
	{ "message", 		99 },
	{ "sounds", 		100 },
	{ "noise", 			101 },
	{ "noise1",			102 },
	{ "noise2",			103 },
	{ "noise3", 		104 },
#ifdef HEXEN2_SUPPORT
// Hexen II-specific fields:
	{ "scale",				105 },
	{ "drawflags",			106 },
	{ "abslight",			107 },
	{ "hull", 				108 },
	{ "stats_restored",		109 },
	{ "playerclass", 		110 },
	{ "bluemana", 			111 },
	{ "greenmana",	 		112 },
	{ "max_mana", 			113 },
	{ "armor_amulet", 		114 },
	{ "armor_bracer",		115 },
	{ "armor_breastplate", 	116 },
	{ "armor_helmet", 		117 },
	{ "level", 				118 },
	{ "intelligence",		119 },
	{ "wisdom",	 			120 },
	{ "dexterity",	 		121 },
	{ "strength",	 		122 },
	{ "experience",	 		123 },
	{ "ring_flight", 		124 },
	{ "ring_water",	 		125 },
	{ "ring_turning",		126 },
	{ "ring_regeneration", 	127 },
	{ "haste_time",			128 },
	{ "tome_time",			129 },
	{ "puzzle_inv1", 		130 },
	{ "puzzle_inv2", 		131 },
	{ "puzzle_inv3", 		132 },
	{ "puzzle_inv4", 		133 },
	{ "puzzle_inv5", 		134 },
	{ "puzzle_inv6", 		135 },
	{ "puzzle_inv7", 		136 },
	{ "puzzle_inv8", 		137 },
	{ "idealroll", 			138 },
	{ "hoverz", 			139 },
	{ "flags2", 			140 },
	{ "light_level", 		141 },
	{ "friction",	 		142 },
	{ "rings_active", 		143 },
	{ "rings_low",			144 },
	{ "artifact_active",	145 },
	{ "artifact_low",		146 },
	{ "hasted",		 		147 },
	{ "inventory",	 		148 },
	{ "cnt_torch",	 		149 },
	{ "cnt_h_boost", 		150 },
	{ "cnt_sh_boost",		151 },
	{ "cnt_mana_boost",		152 },
	{ "cnt_teleport",		153 },
	{ "cnt_tome",	 		154 },
	{ "cnt_summon",	 		155 },
	{ "cnt_invisibility",	156 },
	{ "cnt_glyph",	 		157 },
	{ "cnt_haste",	 		158 },
	{ "cnt_blast",	 		159 },
	{ "cnt_polymorph",		160 },
	{ "cnt_flight",			161 },
	{ "cnt_cubeofforce",	162 },
	{ "cnt_invincibility",	163 },
	{ "cameramode",	 		164 },
	{ "movechain",			165 },
	{ "chainmoved",			166 }
#endif		// #ifdef HEXEN2_SUPPORT
};

#define NUM_STD_FIELD_DEFS sizeof(STD_FIELD_OFFSETS)/sizeof(fieldoffset_t)


globalptrs_t	pr_global_ptrs;
int				*pr_field_map = NULL;


unsigned short	pr_crc;
qboolean pr_handles_imp12;		// JDH

int		type_size[8] = {1, sizeof(string_t) / 4, 1, 3, 1, 1, sizeof(func_t) / 4, sizeof(void *) / 4};

//ddef_t *ED_FieldAtOfs (int ofs);
qboolean ED_ParseEpair (void *base, const ddef_t *key, const char *s);

cvar_t	nomonsters   = {"nomonsters",   "0"};
cvar_t	gamecfg      = {"gamecfg",      "0"};
cvar_t	scratch1     = {"scratch1",     "0"};
cvar_t	scratch2     = {"scratch2",     "0"};
cvar_t	scratch3     = {"scratch3",     "0"};
cvar_t	scratch4     = {"scratch4",     "0"};
cvar_t	savedgamecfg = {"savedgamecfg", "0", CVAR_FLAG_ARCHIVE};
cvar_t	saved1       = {"saved1",       "0", CVAR_FLAG_ARCHIVE};
cvar_t	saved2       = {"saved2",       "0", CVAR_FLAG_ARCHIVE};
cvar_t	saved3       = {"saved3",       "0", CVAR_FLAG_ARCHIVE};
cvar_t	saved4       = {"saved4",       "0", CVAR_FLAG_ARCHIVE};

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start
cvar_t	pr_builtin_find  = {"pr_builtin_find",  "0"};
cvar_t	pr_builtin_remap = {"pr_builtin_remap", "0"};
// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end

qboolean OnChange_pr_checkextension (cvar_t *var, const char *string);
cvar_t	pr_checkextension = {"pr_checkextension", "1", CVAR_FLAG_ARCHIVE, OnChange_pr_checkextension};		// JDH, from DP

#define	MAX_FIELD_LEN	64
#define GEFV_CACHESIZE	2

typedef struct {
	ddef_t	*pcache;
	char	field[MAX_FIELD_LEN];
} gefv_cache;

static	gefv_cache	gefvCache[GEFV_CACHESIZE] = {{NULL, ""}, {NULL, ""}};

// evaluation shortcuts
int	eval_gravity, eval_items2, eval_attack_finished;

int eval_ammo_shells1, eval_ammo_nails1, eval_ammo_lava_nails;
int eval_ammo_rockets1, eval_ammo_multi_rockets;
int	eval_ammo_cells1, eval_ammo_plasma;

// nehahra specific
int	eval_alpha, eval_fullbright, /*eval_idealpitch, */eval_pitch_speed;

// JDH: support for alpha field w/o progs
ddef_t pr_alpha_def = {ev_float, 0, "alpha"};


char *ED_ValueString (etype_t type, const eval_t *val);
char *ED_UglyValueString (etype_t type, const eval_t *val);

void ED_PrintEdict_f (cmd_source_t src);
void ED_Count_f (cmd_source_t src);

qboolean ED_IsInhibited (int spawnflags);

//===========================================================================

/*
============
PR_GlobalAtOfs
============
*/
ddef_t *PR_GlobalAtOfs (int ofs)
{
	ddef_t	*def;
	int	i;

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (def->ofs == ofs)
			return def;
	}

	return NULL;
}

/*
============
PR_FieldAtOfs
============
*/
ddef_t *PR_FieldAtOfs (int ofs)
{
	ddef_t	*def;
	int	i;

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (def->ofs == ofs)
			return def;
	}

	return NULL;
}

/*
============
PR_FindField
============
*/
ddef_t *PR_FindField (const char *name)
{
	ddef_t	*def;
	int	i;

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (!strcmp(/*pr_strings +*/ def->s_name, name))
			return def;
	}

	if (!strcmp (name, "alpha"))
		return &pr_alpha_def;

	return NULL;
}

int PR_FindFieldOffset (const char *name)
{
	ddef_t	*d = PR_FindField (name);

	return (d ? d->ofs*4 : 0);
}

void PR_FindFieldOffsets (void)
{
	eval_gravity = PR_FindFieldOffset ("gravity");
	eval_items2 = PR_FindFieldOffset ("items2");
	eval_attack_finished = PR_FindFieldOffset ("attack_finished");

// Rogue:
	eval_ammo_shells1 = PR_FindFieldOffset ("ammo_shells1");
	eval_ammo_nails1 = PR_FindFieldOffset ("ammo_nails1");
	eval_ammo_lava_nails = PR_FindFieldOffset ("ammo_lava_nails");
	eval_ammo_rockets1 = PR_FindFieldOffset ("ammo_rockets1");
	eval_ammo_multi_rockets = PR_FindFieldOffset ("ammo_multi_rockets");
	eval_ammo_cells1 = PR_FindFieldOffset ("ammo_cells1");
	eval_ammo_plasma = PR_FindFieldOffset ("ammo_plasma");

// Nehahra:
	eval_alpha = PR_FindFieldOffset ("alpha");
	eval_fullbright = PR_FindFieldOffset ("fullbright");
	eval_pitch_speed = PR_FindFieldOffset ("pitch_speed");
}

/*
============
PR_FindGlobal
============
*/
ddef_t *PR_FindGlobal (const char *name)
{
	ddef_t	*def;
	int	i;

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (!strcmp(/*pr_strings +*/ def->s_name, name))
			return def;
	}

	return NULL;
}


/*
============
PR_FindFunction
============
*/
dfunction_t *PR_FindNextFunction (int *start, const char *name, int flags)
{
	dfunction_t	*func;
	int			len, i;
	int			(*cmpfunc)(const char *, const char *, size_t);

	cmpfunc = (flags & PRFF_IGNORECASE ? Q_strncasecmp : strncmp);

	len = strlen (name);

	i = (start ? max(*start, 0) : 0);
	for ( ; i<progs->numfunctions ; i++)
	{
		func = &pr_functions[i];
		if ((flags & PRFF_NOBUILTINS) && (func->first_statement < 0))
			continue;

		if ((flags & PRFF_NOPARAMS) && func->numparms)
			continue;

		if (!cmpfunc(/*pr_strings +*/ func->s_name, name, len))
		{
			if ((flags & PRFF_NOPARTIALS) && func->s_name[len])
				continue;
			if (start)
				*start = i;
			return func;
		}
	}

	return NULL;
}

/*
============
PR_GlobalString

Returns a string with a description and the contents of a global,
padded to 20 field width
============
*/
char *PR_GlobalString (int ofs)
{
	char	*s;
	int		len;
	ddef_t	*def;
	void	*val;
	static char	line[128];

	val = (void *)&pr_globals[ofs];
	if (!(def = PR_GlobalAtOfs(ofs)))
	{
		len = Q_snprintfz (line, sizeof(line), "%i(%c%c%c)", ofs, '?', '?', '?');		// JDH: "???" was giving a trigaph warning on Linux
	}
	else
	{
		s = ED_ValueString (def->type, val);
		len = Q_snprintfz (line, sizeof(line), "%i(%s)%s", ofs, /*pr_strings +*/ def->s_name, s);
	}

	for ( ; len<20 ; len++)
		Q_strcpy (line+len, " ", sizeof(line)-len);
	Q_strcpy (line+len, " ", sizeof(line)-len);

	return line;
}

char *PR_GlobalStringNoContents (int ofs)
{
	int		len;
	ddef_t	*def;
	static	char	line[128];

	if (!(def = PR_GlobalAtOfs(ofs)))
		len = Q_snprintfz (line, sizeof(line), "%i(%c%c%c)", ofs, '?', '?', '?');
	else
		len = Q_snprintfz (line, sizeof(line), "%i(%s)", ofs, /*pr_strings +*/ def->s_name);

	for ( ; len<20 ; len++)
		Q_strcpy (line+len, " ", sizeof(line)-len);
	Q_strcpy (line+len, " ", sizeof(line)-len);

	return line;
}

/*
=============
PR_PrintEdicts_f

For debugging, prints all the entities in the current server
  JDH: added Preach's filtering options
=============
*/
void PR_PrintEdicts_f (cmd_source_t src)
{
	int		i, len, count;
	edict_t	*ed;
	const char	*name;
	ddef_t	*field;
	eval_t	*val;

	Con_PagedOutput_Begin ();

	switch (Cmd_Argc())
	{
		case 1:
			for (i=0 ; i<sv.num_edicts ; i++)
				ED_PrintNum (i);
			Con_Printf ("%i server-side entities\n", sv.num_edicts);
			break;

		case 2:		// match against classname
			name = Cmd_Argv (1);
			len = strlen (name);
			count = 0;
			for (i=0 ; i<sv.num_edicts ; i++)
			{
				ed = EDICT_NUM(i);
				if (!strncmp(name, pr_strings + ed->v.classname, len))
				{
					ED_Print (ed);
					count++;
				}
			}
			Con_Printf ("%i entities matching classname \"%s\"\n", count, name);
			break;

		default:	// match against given entity field
			field = PR_FindField (Cmd_Argv(1));
			if (!field)
			{
				Con_Printf ("Bad field name \"%s\"\n", Cmd_Argv(1));
				break;
			}

			count = 0;
			name = Cmd_Argv (2);
			len = strlen (name);
			for (i=0 ; i<sv.num_edicts ; i++)
			{
				ed = EDICT_NUM(i);
				val = (eval_t *)((int *)&ed->v + field->ofs);
				if (!strncmp (name, ED_UglyValueString(field->type, val), len)) // match initial substring
				{
					ED_Print (ed);
					count++;
				}
			}
			Con_Printf ("%i entities with %s matching \"%s\"\n", count, field->s_name, name);
			break;
	}


	Con_PagedOutput_End ();
}

/*
==============================================================================

		ARCHIVING GLOBALS

FIXME: need to tag constants, doesn't really work
==============================================================================
*/

/*
=============
PR_WriteGlobals
=============
*/
void PR_WriteGlobals (FILE *f)
{
	ddef_t		*def;
	int		i, type;
	char		*name;

	fprintf (f, "{\n");
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		type = def->type;
		if (!(def->type & DEF_SAVEGLOBAL))
			continue;
		type &= ~DEF_SAVEGLOBAL;

		if (type != ev_string && type != ev_float && type != ev_entity)
			continue;

		name = /*pr_strings +*/ def->s_name;
		fprintf (f, "\"%s\" ", name);
		fprintf (f, "\"%s\"\n", ED_UglyValueString(type, (eval_t *)&pr_globals[def->ofs]));
	}
	fprintf (f, "}\n");
}

/*
=============
PR_ParseGlobals
=============
*/
void PR_ParseGlobals (const char *data)
{
	char	keyname[64];
	ddef_t	*key;

	while (1)
	{
	// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Sys_Error ("PR_ParseGlobals: EOF without closing brace");

		Q_strcpy (keyname, com_token, sizeof(keyname));

	// parse value
		if (!(data = COM_Parse (data)))
			Sys_Error ("PR_ParseGlobals: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("PR_ParseGlobals: closing brace without data");

#ifdef _DEBUG
		if (!strcmp(keyname, "serverflags"))
			key = NULL;
#endif

		if (!(key = PR_FindGlobal (keyname)))
		{
			Con_Printf ("'%s' is not a global\n", keyname);
			continue;
		}

		if (!ED_ParseEpair ((void *)pr_globals, key, com_token))
			Host_Error ("PR_ParseGlobals: parse error");
	}
}

//============================================================================


/*
=============
PR_NewString
=============
*/
char *PR_NewString (const char *string)
{
	char	*new, *new_p;
	int	i, l;

	l = strlen(string) + 1;
	new = Hunk_Alloc (l);
	new_p = new;

	for (i=0 ; i<l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			*new_p++ = (string[i] == 'n') ? '\n' : '\\';
		}
		else
			*new_p++ = string[i];
	}

	return new;
}


// JDH: keep track of unknown fields, and display only one warning per name
#define MAX_MISSING_FIELDS 32
char pr_missing_fields[MAX_MISSING_FIELDS][256];

void PR_LogMissingField (const char *name)
{
	int i;

	// JDH: check if we've already shown a warning for this field name:
	for (i = 0; i < MAX_MISSING_FIELDS; i++)
	{
		if (pr_missing_fields[i][0])
		{
			if (!strcmp(pr_missing_fields[i], name))
				return;
		}
		else
		{
			Q_strcpy (pr_missing_fields[i], name, sizeof(pr_missing_fields[i]));
			break;
		}
	}

	Con_Printf ("'%s' is not a field\n", name);
}

// JDH: keep track of entities that couldn't be spawned,
//      and display only one warning per classname
#define MAX_MISSING_ENTS 64
static const char	*pr_missing_ents[MAX_MISSING_ENTS];

#define NOSPAWN_WARNING "\x02""Warning: no spawn function for"

void PR_LogMissingEntity (const char *classname)
{
	int i;

	for (i = 0; i < MAX_MISSING_ENTS; i++)
	{
		if (pr_missing_ents[i])
		{
			if (!strcmp(pr_missing_ents[i], classname))
				return;
		}
		else
		{
			pr_missing_ents[i] = classname;
			break;
		}
	}

	Con_Printf (NOSPAWN_WARNING" %s\n", classname);
}

// JDH: for consolidating list of models not found during precache:
#define MAX_MISSING_MODELS 64
static const char *pr_missing_models[MAX_MISSING_MODELS];

void PR_LogMissingModel (const char *name)
{
	int	i;

	for (i = 0; i < MAX_MISSING_MODELS; i++)
	{
		if (pr_missing_models[i])
		{
			if (!strcmp(pr_missing_models[i], name))
				return;
		}
		else
		{
			pr_missing_models[i] = name;
			break;
		}
	}

	Con_Printf ("\x02""WARNING: unable to precache model %s\n", name);
}

void PR_ClearWarningLists (void)
{
	int i;

	for (i = 0; i < MAX_MISSING_ENTS; i++)
		pr_missing_ents[i] = NULL;

	for (i = 0; i < MAX_MISSING_FIELDS; i++)
		pr_missing_fields[i][0] = 0;

	for (i = 0; i < MAX_MISSING_MODELS; i++)
		pr_missing_models[i] = NULL;
}

/*
================
PR_LoadEdicts

The entities are directly placed in the array, rather than allocated with
ED_Alloc, because otherwise an error loading the map would have entity
number references out of order.

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.

Used for both fresh maps and savegame loads.  A fresh map would also need
to call ED_CallSpawnFunctions () to let the objects initialize themselves.
================
*/
void PR_LoadEdicts (const char *data)
{
	edict_t		*ent;
	int			inhibit;
	dfunction_t	*func;
	int			num_badents = 0;		// JDH: entities whose spawn function was not found
	int			oldcount;
#if defined(HEXEN2_SUPPORT) && !defined(RQM_SV_ONLY)
	int			start_amount;
	const char	*orig;
#endif

	ent = NULL;
	inhibit = 0;
	PR_GLOBAL(time) = sv.time;
	sv.fish_counted = false;

	PR_ClearWarningLists ();

#if defined(HEXEN2_SUPPORT) && !defined(RQM_SV_ONLY)
	start_amount = current_loading_size;
	orig = data;
#endif

// parse ents
	while (1)
	{
	// parse the opening brace
		data = COM_Parse (data);
		if (!data)
			break;

	#if defined(HEXEN2_SUPPORT) && !defined(RQM_SV_ONLY)
		if (entity_file_size)
		{
			current_loading_size = start_amount + ((data-orig)*80/entity_file_size);
			if (!isDedicated)
				SCR_ShowLoadingSize ();
		}
	#endif

		if (com_token[0] != '{')
			Sys_Error ("PR_LoadEdicts: found %s when expecting {",com_token);

		ent = (!ent) ? EDICT_NUM(0) : ED_Alloc ();
		data = ED_ParseEdict (data, ent);

	// remove things from different skill levels or deathmatch
		if (ED_IsInhibited (ent->v.spawnflags))
		{
			ED_Free (ent);
			inhibit++;
			continue;
		}

#ifdef _DEBUG
		if (!strcmp(pr_strings + ent->v.classname, "func_bossgate"))
			inhibit *= 1;
#endif

		// immediately call spawn function
		if (!ent->v.classname)
		{
			Con_Print ("No classname for:\n");
			ED_PrintWithOffset (ent, inhibit + num_badents);
			ED_Free (ent);
			num_badents++;
			continue;
		}

		// look for the spawn function
		func = PR_FindFunction (pr_strings + ent->v.classname, PRFF_NOBUILTINS);

		if (!func)
		{
			if (!developer.value)
			{
			// JDH: check if we've already shown a warning for this entity type:
				PR_LogMissingEntity (pr_strings + ent->v.classname);
			}
			else
			{
				Con_Print (NOSPAWN_WARNING":\n");
				ED_PrintWithOffset (ent, inhibit + num_badents);
			}
			ED_Free (ent);
			num_badents++;
			continue;
		}

		PR_GLOBAL(self) = EDICT_TO_PROG(ent);

		//dwTicks = GetTickCount();

		if (!sv.fish_counted && sv_fishfix.value && !strcmp(func->s_name, "monster_fish"))
		{
			oldcount = PR_GLOBAL(total_monsters);
		}
		else oldcount = -1;

//Con_DPrintf ("calling spawnfunc %s\n", func->s_name);

		PR_ExecuteProgram (func - pr_functions);

		if (oldcount != -1)
		{
			if ((int)PR_GLOBAL(total_monsters) > oldcount)
				sv.fish_counted = true;
		}

		//dwTicks = GetTickCount() - dwTicks;

		//Con_DPrintf ("PR_ExecuteProgram took %i milliseconds\n", dwTicks);
	}

	Con_DPrintf ("%i entities inhibited%s\n", inhibit, (num_badents ? va("; %i could not be spawned", num_badents) : ""));
}

/*
===============
PR_CheckRevCycle (JDH)
  checks whether progs handles impulse 12 (usually used for CycleWeaponReverseCommand)
===============
*/
qboolean PR_CheckRevCycle (void)
{
	char			*name;
	dfunction_t		*func;
	dstatement_t	*st;
	int				ofs_impulse, temp_impulse;
	eval_t			*arg2;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		return true;		// assume it already has it
#endif

	name = (progs->crc == PROGHEADER_BETA_CRC) ? "W_WeaponFrame" : "ImpulseCommands";
	func = PR_FindFunction (name, PRFF_NOBUILTINS);
	if (!func)
		return true;		// if there's no ImpulseCommands func, it's customized code that I won't mess with

	ofs_impulse = PR_FindFieldOffset ("impulse") / 4;
	temp_impulse = 0;

	for (st = &pr_statements[func->first_statement]; st->op; st++)
	{
		arg2 = (eval_t *) &pr_globals[(unsigned short)st->b];

		if (st->op == OP_LOAD_F)
		{
			//eval_t *arg1 = (eval_t *) &pr_globals[(unsigned short)st->a];
			//edict_t *ed = PROG_TO_EDICT(arg1->edict);

			if (arg2->_int == ofs_impulse)
				temp_impulse = st->c;		// local var that self.impulse is loaded into
		}
		else if (st->op == OP_EQ_F)
		{
			if ((st->a == temp_impulse) && (arg2->_float == 12.0))
				return true;
		}
	}

	return false;
}

int PR_GetStdFieldOffset (const char *name)
{
	int i;

	for (i = 0; i < NUM_STD_FIELD_DEFS; i++)
	{
		if (!strcmp (STD_FIELD_OFFSETS[i].name, name))
			return STD_FIELD_OFFSETS[i].ofs;
	}

	return -1;
}

/*
==================
PR_MapEntityFields
  JDH: for compatibility with alternate progs.dat layouts (Quake, Hexen II, Quake beta);
       maps field offsets used by the QC to offsets in the standard entvars_t structure
==================
*/
int PR_MapEntityFields (void)
{
	int		nonstdcount = 0, i, ofs, len;
	ddef_t	*def;
	char	*name;

	len = progs->numfielddefs;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		len += sizeof(entvars_t)/4;		// max field entries in the worst case scenario
										//	(all non-standard)
#endif

	pr_field_map = Hunk_AllocName (len * sizeof(int), "pr_field_map");

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	if (progs->crc == PROGHEADER_CRC)		// standard progs.dat
	{
		for (i=0; i < len; i++)
		{
			pr_field_map[i] = i;
		}
		return len;
	}

	for (i = 0; i < len; i++)
		pr_field_map[i] = -1;

	def = &pr_fielddefs[1];
	for (i=1; i < progs->numfielddefs; i++, def++)
	{
		name = /*pr_strings +*/ def->s_name;

		ofs = PR_GetStdFieldOffset (name);
		if (ofs < 0)
		{
			// check if we've already seen this offset (eg. vector & vector_x, or HexenC unions)
			if (pr_field_map[def->ofs] != -1)
			{
				ofs = pr_field_map[def->ofs];
			}
			else
			{
				ofs = (sizeof(entvars_t) / 4) + nonstdcount;
				nonstdcount++;
			}
		}

		assert (def->ofs < len);

		pr_field_map[def->ofs] = ofs;
		if (ofs != def->ofs)
			def->ofs = (unsigned short) ofs;
	}

	for (i = 0; i < len; i++)
	{
		if (pr_field_map[i] < 0)
			pr_field_map[i] = 0;
	}

	return (sizeof(entvars_t) / 4) + nonstdcount;
}


ebfs_builtin_t * PR_GetEBFSByName (const char *name)
{
	int i;

	// search function name
	for ( i=1 ; i < pr_ebfs_numbuiltins ; i++)
	{
		if (!(Q_strcasecmp(name, pr_ebfs_builtins[i].funcname)))
		{
			return &pr_ebfs_builtins[i];
		}
	}

	return NULL;
}

ebfs_builtin_t * PR_GetEBFSByNum (int num)
{
	int i;

	for ( i=1 ; i < pr_ebfs_numbuiltins ; i++)
	{
		if (pr_ebfs_builtins[i].funcno == num)
		{
			return &pr_ebfs_builtins[i];
		}
	}

	return NULL;
}

/*
===============
PR_MapFunctions
===============
*/
void PR_MapFunctions (void)
{
	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes/Firestorm  start
	int 	i;
	int		funcno;
	char	*funcname;
	ebfs_builtin_t *ft;
// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes/Firestorm  end

	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes/Firestorm  start

	// initialize function numbers for PROGS.DAT

	pr_numbuiltins = 0;
	pr_builtins = NULL;

	if (pr_builtin_remap.value)
	{
		// remove all previous assigned function numbers
		for (i=1 ; i < pr_ebfs_numbuiltins; i++)
		{
			pr_ebfs_builtins[i].funcno = 0;
		}

		for (i=0 ; i<progs->numfunctions; i++)
		{
			if (pr_functions[i].first_statement < 0)	// builtin function
			{
				funcno = -pr_functions[i].first_statement;
				funcname = /*pr_strings +*/ pr_functions[i].s_name;

				ft = PR_GetEBFSByName( funcname );
				if ( ft )
				{
					ft->funcno = funcno;
				}
				else	// not found
				{
					Con_DPrintf("Can not assign builtin #%i to %s - function unknown\n", funcno, funcname);
				}
			}
		}

		// check for unassigned functions and try to assign their default function number
		for ( i=1 ; i < pr_ebfs_numbuiltins; i++)
		{
			if ((!pr_ebfs_builtins[i].funcno) && (pr_ebfs_builtins[i].default_funcno))	// unassigned and has a default number
			{
				// check if default number is already assigned to another function
				ft = PR_GetEBFSByNum( pr_ebfs_builtins[i].default_funcno );
				if ( ft && strcmp( pr_ebfs_builtins[i].funcname, ft->funcname))
				{
					Con_DPrintf("Can not assign default builtin #%i to %s "
								"(already assigned to %s)\n", pr_ebfs_builtins[i].default_funcno,
								pr_ebfs_builtins[i].funcname, ft->funcname);
				}
				else
				{
					pr_ebfs_builtins[i].funcno = pr_ebfs_builtins[i].default_funcno;
				}
			}

			// determine highest builtin number (when remapped)
			if (pr_ebfs_builtins[i].funcno > pr_numbuiltins)
			{
				pr_numbuiltins = pr_ebfs_builtins[i].funcno;
			}
		}
	}
	else
	{
		// use default function numbers
		for (i=1 ; i < pr_ebfs_numbuiltins; i++)
		{
			ft = &pr_ebfs_builtins[i];
			ft->funcno = ft->default_funcno;

			// determine highest builtin number (when NOT remapped)
			if (ft->funcno > pr_numbuiltins)
			{
				pr_numbuiltins = ft->funcno;
			}
		}

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			funcno = pr_builtins_H2[pr_numbuiltins_H2 - 1].funcno;
			if (funcno > pr_numbuiltins)
				pr_numbuiltins = funcno;
		}
	#endif
	}


	// allocate and initialize builtin list for execution time
	pr_numbuiltins++;
	pr_builtins = Hunk_AllocName (pr_numbuiltins*sizeof(builtin_t), "builtins");

	ft = &pr_ebfs_builtins[0];
	for (i=0 ; i < pr_numbuiltins ; i++)
	{
		pr_builtins[i] = ft->function;
	}

	// create builtin list for execution time and set cvars accordingly

	Cvar_SetDirect(&pr_builtin_find, "0");
//	Cvar_SetDirect(&pf_checkextension, "0");	// 2001-10-20 Extension System by Lord Havoc/Maddes (DP compatibility)

	for (i=1 ; i < pr_ebfs_numbuiltins ; i++)
	{
		ft = &pr_ebfs_builtins[i];
		if (ft->funcno)		// only put assigned functions into builtin list
		{
			pr_builtins[ft->funcno] = ft->function;
		}

		if (ft->default_funcno == PR_DEFAULT_FUNCNO_BUILTIN_FIND)
		{
			Cvar_SetValueDirect (&pr_builtin_find, ft->funcno);
		}

// 2001-10-20 Extension System by Lord Havoc/Maddes (DP compatibility)  start

// not implemented yet

/*
		if (ft->default_funcno == PR_DEFAULT_FUNCNO_EXTENSION_FIND)
		{
			Cvar_SetValueDirect (&pf_checkextension, ft->funcno);
		}
*/

// 2001-10-20 Extension System by Lord Havoc/Maddes (DP compatibility)  end
	}
// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes/Firestorm  end

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		// override some standard functions with Hexen II ones
		for (i=0; i < pr_numbuiltins_H2; i++)
		{
			ft = &pr_builtins_H2[i];
			pr_builtins[ ft->default_funcno ] = ft->function;
		}
	}
	else
#endif
	if (progs->crc == PROGHEADER_BETA_CRC)
	{
		// override some standard functions with ones from beta Quake
		for (i=0; i < pr_numbuiltins_beta; i++)
		{
			ft = &pr_builtins_beta[i];
			pr_builtins[ ft->default_funcno ] = ft->function;
		}
	}
}


#define PR_SETGLOBALPTRS(v) \
	pr_global_ptrs.self = &(v)->self;	\
	pr_global_ptrs.other = &(v)->other;	\
	pr_global_ptrs.time = &(v)->time;	\
	pr_global_ptrs.frametime = &(v)->frametime;	\
	pr_global_ptrs.mapname = &(v)->mapname;	\
	pr_global_ptrs.deathmatch = &(v)->deathmatch;	\
	pr_global_ptrs.coop = &(v)->coop;	\
	pr_global_ptrs.serverflags = &(v)->serverflags;	\
	pr_global_ptrs.total_secrets = &(v)->total_secrets;	\
	pr_global_ptrs.total_monsters = &(v)->total_monsters;	\
	pr_global_ptrs.found_secrets = &(v)->found_secrets;	\
	pr_global_ptrs.killed_monsters = &(v)->killed_monsters;	\
	pr_global_ptrs.parm1 = &(v)->parm1;	\
	\
	pr_global_ptrs.v_forward = &(v)->v_forward;	\
	pr_global_ptrs.v_up = &(v)->v_up;	\
	pr_global_ptrs.v_right = &(v)->v_right;	\
	\
	pr_global_ptrs.trace_allsolid = &(v)->trace_allsolid;	\
	pr_global_ptrs.trace_startsolid = &(v)->trace_startsolid;	\
	pr_global_ptrs.trace_fraction = &(v)->trace_fraction;	\
	pr_global_ptrs.trace_endpos = &(v)->trace_endpos;	\
	pr_global_ptrs.trace_plane_normal = &(v)->trace_plane_normal;	\
	pr_global_ptrs.trace_plane_dist = &(v)->trace_plane_dist;	\
	pr_global_ptrs.trace_ent = &(v)->trace_ent;	\
	pr_global_ptrs.trace_inopen = &(v)->trace_inopen;	\
	pr_global_ptrs.trace_inwater = &(v)->trace_inwater;	\
	\
	pr_global_ptrs.StartFrame = &(v)->StartFrame;	\
	pr_global_ptrs.PlayerPreThink = &(v)->PlayerPreThink;	\
	pr_global_ptrs.PlayerPostThink = &(v)->PlayerPostThink;	\
	pr_global_ptrs.ClientKill = &(v)->ClientKill;	\
	pr_global_ptrs.ClientConnect = &(v)->ClientConnect;	\
	pr_global_ptrs.PutClientInServer = &(v)->PutClientInServer;	\
	pr_global_ptrs.ClientDisconnect = &(v)->ClientDisconnect;

void PR_SetGlobalPtrs (globalvars_t *v)
{
	PR_SETGLOBALPTRS(v);
	pr_global_ptrs.force_retouch = &v->force_retouch;
	pr_global_ptrs.msg_entity = &v->msg_entity;
	pr_global_ptrs.SetNewParms = &v->SetNewParms;
	pr_global_ptrs.SetChangeParms = &v->SetChangeParms;
}

void PR_SetGlobalPtrsBeta (globalvars_beta_t *v)
{
	static float dummy_f;
	static int   dummy_e;

	PR_SETGLOBALPTRS(v);

	dummy_f = 0;
	pr_global_ptrs.force_retouch = &dummy_f;
	dummy_e = 0;
	pr_global_ptrs.msg_entity = &dummy_e;

	pr_global_ptrs.SetNewParms = &v->SetNewParms;
	pr_global_ptrs.SetChangeParms = &v->SetChangeParms;
}

#ifdef HEXEN2_SUPPORT

void PR_SetGlobalPtrsH2 (globalvars_H2_t *v)
{
	static func_t dummy_func;

	PR_SETGLOBALPTRS(v);
	pr_global_ptrs.force_retouch = &v->force_retouch;
	pr_global_ptrs.msg_entity = &v->msg_entity;

	dummy_func = 0;		// the empty function
	pr_global_ptrs.SetNewParms = &dummy_func;
	pr_global_ptrs.SetChangeParms = &dummy_func;

	pr_global_ptrs.cycle_wrapped = &v->cycle_wrapped;
	pr_global_ptrs.cl_playerclass = &v->cl_playerclass;
	pr_global_ptrs.startspot = &v->startspot;
	pr_global_ptrs.randomclass = &v->randomclass;
	pr_global_ptrs.ClassChangeWeapon = &v->ClassChangeWeapon;
	pr_global_ptrs.ClientReEnter = &v->ClientReEnter;
}

#endif

/*
===============
PR_LoadProgs
===============
*/
void PR_LoadProgs (void)
{
	int	i, filesize;
	qboolean has_alpha;

// flush the non-C variable lookup cache
	for (i=0 ; i<GEFV_CACHESIZE ; i++)
		gefvCache[i].field[0] = 0;

	if (!(progs = (dprograms_t *)COM_LoadHunkFile("progs.dat", FILE_NO_DZIPS)))
		Sys_Error ("PR_LoadProgs: couldn't load progs.dat");

	filesize = com_filesize;		// JDH: since call to Con_DPrintf may overwrite
	Con_DPrintf ("Programs occupy %iK.\n", filesize / 1024);

	pr_crc = CRC_Block ((byte *)progs, filesize);		// used only in SV_SendServerinfo

// byte swap the header
	for (i=0 ; i<sizeof(*progs)/4 ; i++)
		((int *)progs)[i] = LittleLong (((int *)progs)[i]);

	if (progs->version != PROG_VERSION)
		Host_Error ("progs.dat has wrong version number %i (should be %i)", progs->version, PROG_VERSION);	// was Sys_Error

	if (progs->crc != PROGHEADER_CRC)
	{
		if (progs->crc != PROGHEADER_BETA_CRC)
	#ifdef HEXEN2_SUPPORT
		if (!hexen2 || (progs->crc != PROGHEADER_CRC_H2))
	#endif
//			Sys_Error ("progs.dat system vars have been modified, progdefs.h is out of date");
			Host_Error ("progs.dat does not match expected format!");
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	if (progs->numglobals > 32767)
		Con_DPrintf ("\x02""Warning: progs.dat has %d globals (standard limit is 32767)\n", progs->numglobals);

	pr_functions = (dfunction_t *)((byte *)progs + progs->ofs_functions);
	pr_strings = (char *)progs + progs->ofs_strings;
	pr_globaldefs = (ddef_t *)((byte *)progs + progs->ofs_globaldefs);
	pr_fielddefs = (ddef_t *)((byte *)progs + progs->ofs_fielddefs);
	pr_statements = (dstatement_t *)((byte *)progs + progs->ofs_statements);
	pr_globals = (float *)((byte *)progs + progs->ofs_globals);

// byte swap the lumps
#ifndef _WIN32
	for (i=0 ; i<progs->numstatements ; i++)
	{
		pr_statements[i].op = LittleShort (pr_statements[i].op);
		pr_statements[i].a = LittleShort (pr_statements[i].a);
		pr_statements[i].b = LittleShort (pr_statements[i].b);
		pr_statements[i].c = LittleShort (pr_statements[i].c);
	}

	for (i=0 ; i<progs->numglobals ; i++)
	{
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);
	}
#endif

	for (i=0 ; i<progs->numfunctions ; i++)
	{
	#ifndef _WIN32
		pr_functions[i].first_statement = LittleLong (pr_functions[i].first_statement);
		pr_functions[i].parm_start = LittleLong (pr_functions[i].parm_start);
		pr_functions[i].numparms = LittleLong (pr_functions[i].numparms);
		pr_functions[i].locals = LittleLong (pr_functions[i].locals);
	#endif
	// JDH: convert offsets to strings
		pr_functions[i].s_name = pr_strings + LittleLong ((long) pr_functions[i].s_name);
		pr_functions[i].s_file = pr_strings + LittleLong ((long) pr_functions[i].s_file);
	}

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		pr_globaldefs[i].type = LittleShort (pr_globaldefs[i].type);
		pr_globaldefs[i].ofs = LittleShort (pr_globaldefs[i].ofs);
		pr_globaldefs[i].s_name = pr_strings + LittleLong ((long) pr_globaldefs[i].s_name);
	}

	has_alpha = false;
	for (i=0 ; i<progs->numfielddefs ; i++)
	{
	#ifndef _WIN32
		pr_fielddefs[i].type = LittleShort (pr_fielddefs[i].type);
		pr_fielddefs[i].ofs = LittleShort (pr_fielddefs[i].ofs);
	#endif

		pr_fielddefs[i].s_name = pr_strings + LittleLong ((long) pr_fielddefs[i].s_name);

		if (!has_alpha && !strcmp (pr_fielddefs[i].s_name, "alpha"))
			has_alpha = true;

		if (pr_fielddefs[i].type & DEF_SAVEGLOBAL)
		{
		//	Sys_Error ("PR_LoadProgs: pr_fielddefs[i].type & DEF_SAVEGLOBAL");
			Con_DPrintf ("WARNING (PR_LoadProgs): field %d has DEF_SAVEGLOBAL set.\n", i);
			pr_fielddefs[i].type &= ~DEF_SAVEGLOBAL;
		}
	}


#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		PR_SetGlobalPtrsH2 ((globalvars_H2_t *) pr_globals);
		*pr_global_ptrs.cl_playerclass = cl_playerclass.value;
	}
	else
#endif
	if (progs->crc == PROGHEADER_BETA_CRC)
	{
		PR_SetGlobalPtrsBeta ((globalvars_beta_t *) pr_globals);
	}
	else
	{
		PR_SetGlobalPtrs ((globalvars_t *) pr_globals);
	}

	progs->entityfields = PR_MapEntityFields();

	pr_handles_imp12 = PR_CheckRevCycle ();

// JDH: add room for alpha field, if not already present
	pr_alpha_def.ofs = (has_alpha ? 0 : progs->entityfields++);

	pr_edict_size = progs->entityfields * 4 + sizeof(edict_t) - sizeof(entvars_t);
	PR_MapFunctions();
	PR_FindFieldOffsets ();
}

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start

/*
=============
PR_BuiltInList_f
For debugging, prints all builtin functions with assigned and default number
=============
*/
void PR_BuiltInList_f (cmd_source_t src)
{
	int		i;
	const char	*partial;
	int		len;
	int		count;

	if (Cmd_Argc() > 1)
	{
		partial = Cmd_Argv (1);
		len = strlen (partial);
	}

	else
	{
		partial = NULL;
		len = 0;
	}

	count=0;
	Con_PagedOutput_Begin ();

	for (i=1; i < pr_ebfs_numbuiltins; i++)
	{
		if (partial && Q_strncasecmp (partial, pr_ebfs_builtins[i].funcname, len))
		{
			continue;
		}

		count++;
		Con_Printf ("%i(%i): %s\n", pr_ebfs_builtins[i].funcno, pr_ebfs_builtins[i].default_funcno, pr_ebfs_builtins[i].funcname);
	}

	Con_Print ("------------\n");

	if (partial)
	{
		Con_Printf ("%i beginning with \"%s\" out of ", count, partial);
	}
	Con_Printf ("%i builtin functions\n", i);

	Con_PagedOutput_End ();
}

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end

qboolean OnChange_pr_checkextension (cvar_t *var, const char *string)
{
	if (sv.active)
		Con_Printf ("Change to %s will be applied the next time a map is loaded.\n", var->name);
	return false;		// allow change
}

#ifdef PR_DEBUGGER
extern void PR_Debugger_f (cmd_source_t src);
#endif

/*
===============
PR_Init
===============
*/
void PR_Init (void)
{
	Cmd_AddCommand ("edict", ED_PrintEdict_f, 0);
	Cmd_AddCommand ("edicts", PR_PrintEdicts_f, 0);
	Cmd_AddCommand ("edictcount", ED_Count_f, 0);
	Cmd_AddCommand ("profile", PR_Profile_f, 0);
	Cmd_AddCommand ("builtinlist", PR_BuiltInList_f, 0);	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes

#ifdef PR_DEBUGGER
	Cmd_AddCommand ("prdb", PR_Debugger_f, 0);
#endif

	Cvar_RegisterBool (&nomonsters);
	Cvar_Register (&gamecfg);
	Cvar_Register (&scratch1);
	Cvar_Register (&scratch2);
	Cvar_Register (&scratch3);
	Cvar_Register (&scratch4);
	Cvar_Register (&savedgamecfg);
	Cvar_Register (&saved1);
	Cvar_Register (&saved2);
	Cvar_Register (&saved3);
	Cvar_Register (&saved4);

	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start
	Cvar_Register (&pr_builtin_find);
	Cvar_RegisterBool (&pr_builtin_remap);
	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end

	Cvar_RegisterBool (&pr_checkextension);

/*  2008/09/18 - moved to Hexen2_InitEnv
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		PR_LoadStrings ();
		PR_LoadInfoStrings ();
	}
#endif
*/
}

/*
===============
EDICT_NUM
===============
*/
edict_t *EDICT_NUM (int n)
{
	if (n < 0 || n >= sv.max_edicts)
		Sys_Error ("EDICT_NUM: bad number %i", n);

	return (edict_t *)((byte *)sv.edicts + (n)*pr_edict_size);
}

/*
===============
NUM_FOR_EDICT
===============
*/
int NUM_FOR_EDICT (const edict_t *e)
{
	int		b;

	b = (byte *)e - (byte *)sv.edicts;
	b = b / pr_edict_size;

/*** FIXME: it would be nice to do a PR_RunError instead, but how to check if running QC?? ***/

	if (b < 0 || b >= sv.num_edicts)
		Host_Error ("NUM_FOR_EDICT: bad pointer");		// JDH: was Sys_Error

	return b;
}

/*
=================
ED_ClearEdict

Sets everything to NULL
=================
*/
void ED_ClearEdict (edict_t *e)
{
	memset (&e->v, 0, progs->entityfields * 4);
	e->free = false;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		memset (&e->baseline, 0, sizeof(e->baseline));
#endif
}

/*
=================
ED_Alloc

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *ED_Alloc (void)
{
	int		i;
	edict_t		*e;

	i = svs.maxclients + 1;
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		i += max_temp_edicts.value;
#endif

	for (; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && (e->freetime < 2 || sv.time - e->freetime > 0.5))
		{
			ED_ClearEdict (e);
		#ifdef HEXEN2_SUPPORT
			e->alloctime = sv.time;
		#endif
			return e;
		}
	}

	if (i == MAX_EDICTS)
		Sys_Error ("ED_Alloc: no free edicts");

	sv.num_edicts++;
	e = EDICT_NUM(i);
	ED_ClearEdict (e);
#ifdef HEXEN2_SUPPORT
	e->alloctime = sv.time;
#endif

	return e;
}

#ifdef HEXEN2_SUPPORT

edict_t *ED_Alloc_Temp (void)
{
	int			i,j,Found;
	edict_t		*e,*Least;
	float		LeastTime;
	qboolean	LeastSet;

	LeastTime = -1;
	LeastSet = false;
	for ( i=svs.maxclients+1,j=0 ; j < max_temp_edicts.value ; i++,j++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && ( e->freetime < 2 || sv.time - e->freetime > 0.5 ) )
		{
			ED_ClearEdict (e);
			e->alloctime = sv.time;

			return e;
		}
		else if (e->alloctime < LeastTime || !LeastSet)
		{
			Least = e;
			LeastTime = e->alloctime;
			Found = j;
			LeastSet = true;
		}
	}

	ED_Free(Least);
	ED_ClearEdict (Least);
	Least->alloctime = sv.time;

	return Least;
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
=================
ED_Free

Marks the edict as free
FIXME: walk all entities and NULL out references to this entity
=================
*/
void ED_Free (edict_t *ed)
{
	SV_UnlinkEdict (ed);		// unlink from world bsp

	ed->free = true;
	ed->v.model = 0;
	ed->v.takedamage = 0;
	ed->v.modelindex = 0;
	ed->v.colormap = 0;
	ed->v.skin = 0;
	ed->v.frame = 0;
	VectorCopy (vec3_origin, ed->v.origin);
	VectorCopy (vec3_origin, ed->v.angles);
	ed->v.nextthink = -1;
	ed->v.solid = 0;

	ed->freetime = sv.time;
#ifdef HEXEN2_SUPPORT
	ed->alloctime = -1;
#endif
}

/*
============
ED_GetFieldValue
=============
*/
float ED_GetFieldValue (const edict_t *ed, const char *name)
{
	ddef_t	*d = PR_FindField (name);

	return (d ? GETFIELDFLOAT(ed, d->ofs) : 0);
}

/*
============
ED_SetFieldValue
=============
*/
void ED_SetFieldValue (edict_t *ed, const char *name, float val)
{
	ddef_t	*d = PR_FindField (name);

	if (d)
		GETFIELDFLOAT(ed, d->ofs) = 0;
}

/*
============
ED_ValueString

Returns a string describing *data in a type specific manner
=============
*/
char *ED_ValueString (etype_t type, const eval_t *val)
{
	static char	line[256];
	ddef_t		*def;
	dfunction_t	*f;

	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		Q_snprintfz (line, sizeof(line), "%s", pr_strings + val->string);
		break;

	case ev_entity:
		Q_snprintfz (line, sizeof(line), "entity %i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)));
		break;

	case ev_function:
		f = pr_functions + val->function;
		Q_snprintfz (line, sizeof(line), "%s()", /*pr_strings +*/ f->s_name);
		break;

	case ev_field:
		def = PR_FieldAtOfs (val->_int);
		Q_snprintfz (line, sizeof(line), ".%s", /*pr_strings +*/ def->s_name);
		break;

	case ev_void:
		Q_snprintfz (line, sizeof(line), "void");
		break;

	case ev_float:
		Q_snprintfz (line, sizeof(line), "%5.1f", val->_float);
		break;

	case ev_vector:
		Q_snprintfz (line, sizeof(line), "'%5.1f %5.1f %5.1f'", val->vector[0], val->vector[1], val->vector[2]);
		break;

	case ev_pointer:
		Q_snprintfz (line, sizeof(line), "pointer");
		break;

	default:
		Q_snprintfz (line, sizeof(line), "bad type %i", type);
		break;
	}

	return line;
}

/*
============
ED_UglyValueString

Returns a string describing *data in a type specific manner
Easier to parse than ED_ValueString
=============
*/
char *ED_UglyValueString (etype_t type, const eval_t *val)
{
	static char	line[256];
	ddef_t		*def;
	dfunction_t	*f;

	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		Q_snprintfz (line, sizeof(line), "%s", pr_strings + val->string);
		break;

	case ev_entity:
		Q_snprintfz (line, sizeof(line), "%i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)));
		break;

	case ev_function:
		f = pr_functions + val->function;
		Q_snprintfz (line, sizeof(line), "%s", /*pr_strings +*/ f->s_name);
		break;

	case ev_field:
		def = PR_FieldAtOfs ( val->_int );
		Q_snprintfz (line, sizeof(line), "%s", /*pr_strings +*/ def->s_name);
		break;

	case ev_void:
		Q_snprintfz (line, sizeof(line), "void");
		break;

	case ev_float:
		Q_snprintfz (line, sizeof(line), "%f", val->_float);
		break;

	case ev_vector:
		Q_snprintfz (line, sizeof(line), "%f %f %f", val->vector[0], val->vector[1], val->vector[2]);
		break;

	default:
		Q_snprintfz (line, sizeof(line), "bad type %i", type);
		break;
	}

	return line;
}

/*
=============
ED_Print

For debugging
=============
*/
void ED_PrintWithOffset (const edict_t *ed, int numoffset)
{
	int	len, *v, i, j, type;
	ddef_t	*d;
	char	*name;
/****JDH****/
	char    buf[128];
/****JDH****/

	if (ed->free)
	{
		Con_Print ("FREE\n");
		return;
	}

	Con_Printf ("EDICT %i:\n", NUM_FOR_EDICT(ed) + numoffset);
	d = &pr_fielddefs[1];
	for (i=1 ; i<progs->numfielddefs ; i++, d++)
	{
		//d = &pr_fielddefs[i];
		name = /*pr_strings +*/ d->s_name;
		len = strlen (name);
		if ((len > 2) && (name[len-2] == '_') && (name[len-1] >= 'x') && (name[len-1] <= 'z'))
			continue;	// skip _x, _y, _z vars

	/****JDH****/
		//v = (int *)((char *)&ed->v + d->ofs*4);
		v = (int *) &ed->v + d->ofs;
	/****JDH****/

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;

		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;

	/****JDH****/
		/*len = sprintf( buf, "  %s", name );
		while (len < 17)
			buf[len++] = ' ';

		buf[len] = 0;
		Con_Print( buf );*/

		j = !strcmp (name, "classname");
		Q_snprintfz (buf, sizeof(buf), "%s  %-16s%s\n", (j ? "\x02" : ""), name,
						ED_ValueString(d->type, (eval_t *)v));
		Con_Print (buf);


		//Con_Printf ("%s", name);
		//l = strlen (name);
		//while (l++ < 15)
		//	Con_Print (" ");
	/****JDH****/

		//Con_Printf ("%s\n", PR_ValueString(d->type, (eval_t *)v));
	}

	if (pr_alpha_def.ofs)
	{
		v = (int *) &ed->v + pr_alpha_def.ofs;
		if (*v)
			Con_Printf ("  %-16s%s\n", pr_alpha_def.s_name, ED_ValueString(pr_alpha_def.type, (eval_t *)v));
	}

	Con_Print ("\n");
}

/*
=============
ED_Write

For savegames
=============
*/
void ED_Write (FILE *f, const edict_t *ed)
{
	ddef_t	*d;
	int	*v, i, j, type;
	char	*name;

	fprintf (f, "{\n");

	if (ed->free)
	{
		fprintf (f, "}\n");
		return;
	}

	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = /*pr_strings +*/ d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars

	/******JDH******/
		//v = (int *)((char *)&ed->v + d->ofs*4);
		v = (int *) &ed->v + d->ofs;
	/******JDH******/

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;
		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;

		fprintf (f, "\"%s\" ", name);
		fprintf (f, "\"%s\"\n", ED_UglyValueString(d->type, (eval_t *)v));
	}

	if (pr_alpha_def.ofs)
	{
		v = (int *) &ed->v + pr_alpha_def.ofs;
		if (*v)
			fprintf (f, "\"%s\" \"%s\"\n", pr_alpha_def.s_name, ED_UglyValueString(pr_alpha_def.type, (eval_t *)v));
	}

	fprintf (f, "}\n");
}

void ED_PrintNum (int ent)
{
	ED_Print (EDICT_NUM(ent));
}


/*
=============
ED_PrintEdict_f

For debugging, prints a single edict
=============
*/
void ED_PrintEdict_f (cmd_source_t src)
{
	int	i;

	i = Q_atoi (Cmd_Argv(1));		// if no arg1, empty string is returned, which converts to 0
	if (i >= sv.num_edicts)
	{
		Con_Print ("Bad edict number\n");
		return;
	}
	ED_PrintNum (i);
}

/*
=============
ED_Count_f

For debugging
=============
*/
void ED_Count_f (cmd_source_t src)
{
	int		i, active, models, solid, step;
	edict_t		*ent;

	active = models = solid = step = 0;
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ent = EDICT_NUM(i);
		if (ent->free)
			continue;
		active++;
		if (ent->v.solid)
			solid++;
		if (ent->v.model)
			models++;
		if (ent->v.movetype == MOVETYPE_STEP)
			step++;
	}

	Con_Printf ("num_edicts:%3i\n", sv.num_edicts);
	Con_Printf ("active    :%3i\n", active);
	Con_Printf ("view      :%3i\n", models);
	Con_Printf ("touch     :%3i\n", solid);
	Con_Printf ("step      :%3i\n", step);
}


/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
Used for initial level load and for savegames.
====================
*/
const char *ED_ParseEdict (const char *data, edict_t *ent)
{
	ddef_t		*key;
	qboolean	anglehack;
	qboolean	init;
	char		keyname[256];
	int		n;

	init = false;

// clear it
	if (ent != sv.edicts)	// hack
		memset (&ent->v, 0, progs->entityfields * 4);

// go through all the dictionary pairs
	while (1)
	{
	// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Sys_Error ("ED_ParseEdict: EOF without closing brace");

	// anglehack is to allow QuakeEd to write single scalar angles
	// and allow them to be turned into vectors. (FIXME...)
		if (!strcmp(com_token, "angle"))
		{
			strcpy (com_token, "angles");
			anglehack = true;
		}
		else
			anglehack = false;

	// FIXME: change light to _light to get rid of this hack
		if (!strcmp(com_token, "light"))
			strcpy (com_token, "light_lev");	// hack for single light def

		Q_strcpy (keyname, com_token, sizeof(keyname));

	// another hack to fix keynames with trailing spaces
		n = strlen (keyname);
		while (n && keyname[n-1] == ' ')
		{
			keyname[n-1] = 0;
			n--;
		}

	// parse value
		if (!(data = COM_Parse (data)))
			Sys_Error ("ED_ParseEdict: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEdict: closing brace without data");

		init = true;

	// keynames with a leading underscore are used for utility comments,
	// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			if (Q_strcasecmp(keyname, "MIDI") == 0)
			{
				Q_strcpy (sv.midi_name, com_token, sizeof(sv.midi_name));
				continue;
			}
			else if (Q_strcasecmp(keyname, "CD") == 0)
			{
				sv.edicts[0].v.sounds = atol(com_token);
				continue;
			}
		}
	#endif

		if (!(key = PR_FindField (keyname)))
		{
			//johnfitz -- HACK -- suppress error becuase fog/sky fields might not be mentioned in defs.qc
			if (strncmp(keyname, "sky", 3) && strncmp(keyname, "fog", 3))
			{
				PR_LogMissingField (keyname);
			}
			continue;
		}

		if (anglehack)
		{
			char	temp[32];

			Q_strcpy (temp, com_token, sizeof(temp));
			Q_snprintfz (com_token, sizeof(com_token), "0 %s 0", temp);
		}

		if (!ED_ParseEpair ((void *)&ent->v, key, com_token))
			Host_Error ("ED_ParseEdict: parse error");
	}

	if (!init)
		ent->free = true;

	return data;
}

/*
================
ED_IsInhibited
================
*/

qboolean ED_IsInhibited (int spawnflags)
{
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (deathmatch.value)
		{
			if (spawnflags & SF_H2_NOT_DEATHMATCH)
				return true;
		}
		else if (coop.value)
		{
			if (spawnflags & SF_H2_NOT_COOP)
				return true;
		}
		else
		{ // Gotta be single player
			if (spawnflags & SF_H2_NOT_SINGLE)
				return true;

			switch ((int)cl_playerclass.value)
			{
				case CLASS_PALADIN:
					if (spawnflags & SF_H2_NOT_PALADIN)
						return true;
					break;

				case CLASS_CLERIC:
					if (spawnflags & SF_H2_NOT_CLERIC)
						return true;
					break;

				case CLASS_DEMON:
				case CLASS_NECROMANCER:
					if (spawnflags & SF_H2_NOT_NECROMANCER)
						return true;
					break;

				case CLASS_THEIF:
					if (spawnflags & SF_H2_NOT_THEIF)
						return true;
					break;
			}
		}

		if ((current_skill == 0 && (spawnflags & SF_H2_NOT_EASY))
			|| (current_skill == 1 && (spawnflags & SF_H2_NOT_MEDIUM))
			|| (current_skill >= 2 && (spawnflags & SF_H2_NOT_HARD)) )
		{
			return true;
		}

		return false;
	}
#endif

	if (deathmatch.value)
	{
		if (spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
		{
			return true;
		}
	}
	else if ((current_skill == 0 && (spawnflags & SPAWNFLAG_NOT_EASY))
			|| (current_skill == 1 && (spawnflags & SPAWNFLAG_NOT_MEDIUM))
			|| (current_skill >= 2 && (spawnflags & SPAWNFLAG_NOT_HARD)))
	{
		return true;
	}

	return false;
}


/*
==============
ED_ParseEpair

Can parse either fields or globals
returns false if error
==============
*/
qboolean ED_ParseEpair (void *base, const ddef_t *key, const char *s)
{
	int		i;
	char		string[128];
	ddef_t		*def;
	char		*v, *w;
	void		*d;
	dfunction_t	*func;

	d = (void *)((int *)base + key->ofs);

	switch (key->type & ~DEF_SAVEGLOBAL)
	{
	case ev_string:
		*(string_t *)d = PR_NewString (s) - pr_strings;
		break;

	case ev_float:
		*(float *)d = Q_atof (s);
		break;

	case ev_vector:
		Q_strcpy (string, s, sizeof(string));
		v = string;
		w = string;
		for (i=0 ; i<3 ; i++)
		{
			while (*v && *v != ' ')
				v++;
			*v = 0;
			((float *)d)[i] = Q_atof (w);
			w = v = v+1;
		}
		break;

	case ev_entity:
		*(int *)d = EDICT_TO_PROG(EDICT_NUM(atoi (s)));
		break;

	case ev_field:
		if (!(def = PR_FindField(s)))
		{
			// LordHavoc: don't warn about worldspawn sky/fog fields
			// because they don't require mod support
			if (strncmp(s, "sky", 3) && strncmp(s, "fog", 3))
				Con_Printf ("Can't find field %s\n", s);
			return false;
		}
		*(int *)d = G_INT(def->ofs);
		break;

	case ev_function:
		if (!(func = PR_FindFunction(s, 0)))
		{
			Con_Printf ("Can't find function %s\n", s);
			return false;
		}
		*(func_t *)d = func - pr_functions;
		break;

	default:
		break;
	}

	return true;
}

