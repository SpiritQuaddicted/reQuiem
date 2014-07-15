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
// cvar.c -- dynamic variable tracking

#include "quakedef.h"

cvar_t	*cvar_vars;
char	*cvar_null_string = "";

cvar_t	cfg_savevars = {"cfg_savevars", "0", CVAR_FLAG_ARCHIVE};

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (const char *var_name)
{
	cvar_t	*var;

	for (var = cvar_vars ; var ; var = var->next)
		if (!strcmp(var_name, var->name))
			return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue (const char *var_name)
{
	cvar_t	*var;

	if (!(var = Cvar_FindVar(var_name)))
		return 0;
	return Q_atof (var->string);
}

/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (const char *var_name)
{
	cvar_t	*var;

	if (!(var = Cvar_FindVar(var_name)))
		return cvar_null_string;
	return var->string;
}

/*
============
Cvar_CompleteVariable
============
*/
/*char *Cvar_CompleteVariable (const char *partial)
{
	cvar_t	*cvar;
	int	len;

	if (!(len = strlen(partial)))
		return NULL;

	// check functions
	for (cvar = cvar_vars ; cvar ; cvar = cvar->next)
		if (!Q_strncasecmp(partial, cvar->name, len))
			return cvar->name;

	return NULL;
}
*/
/*
============
Cvar_CompleteCountPossible
============
*/
int Cvar_CompleteCountPossible (const char *partial)
{
	cvar_t	*cvar;
	int	len, c = 0;

	if (!(len = strlen(partial)))
		return 0;

	// check partial match
	for (cvar = cvar_vars ; cvar ; cvar = cvar->next)
		if (!Q_strncasecmp(partial, cvar->name, len))
			c++;

	return c;
}

// JDH: maximum number of recursive OnChange calls (originally 1)
#define CVAR_MAX_ONCHANGE   2

/*
============
Cvar_SetDirect
 - returns true only if value is changed
============
*/
qboolean Cvar_SetDirect (cvar_t *var, const char *value)
{
	float		fValue;
	qboolean	changed;
	char		*p, *s;
	static int	change_depth = 0;
	extern qboolean	config_exec;
	extern qboolean	config_exec_rq;

	if (var->flags & CVAR_FLAG_READONLY)
	{
		Con_Printf ("Cvar_Set: variable \"%s\" is read-only\n", var->name);
		return false;
	}
	
	fValue = Q_atof (value);

	if (var->type == CVAR_STRING)
		changed = strcmp (var->string, value);
	else
		changed = (/*Q_atof(var->string)*/ var->value != fValue);

	if (changed)
	{
		if (var->OnChange && (change_depth < CVAR_MAX_ONCHANGE))
		{
			change_depth++;
			if (var->OnChange(var, value))
			{
				change_depth--;
				return false;
			}
			change_depth--;
		}
	}

	if (var->string)		// name is null only for non-registered cvars in menu.c
	{
		// JDH: any cvars loaded from reQuiem.cfg file will get saved back to reQuiem.cfg on quit
		if (changed && config_exec_rq)
			var->flags |= CVAR_FLAG_ARCHIVE;

		// JDH: the first time the value changes after execing quake.rc, backup original value
		if (changed && (var->flags & CVAR_FLAG_ARCHIVE) && (!config_exec || config_exec_rq) && !var->savedvalue)
		{
			var->savedvalue = var->string;
		}
		else
			Z_Free (var->string);	// free the old value string
		
		var->string = CopyString (value);

		if (var->type == CVAR_INT)
		{
			// JDH: remove the decimal and trailing zeros:
			p = strchr (var->string, '.');
			if (p)
			{
				for (s = p + strlen(p) - 1; (*s == '0') && (s != p); s--)
					*s = 0;

				if (*s == '.')		// nothing left after the decimal
					*s = 0;
			}
		}
		else if (var->type == CVAR_FLOAT)
		{
			// JDH: remove the trailing zeros after the first 2 decimal places:
			p = strchr (var->string, '.');
			if (p)
			{
				for (s = p + strlen(p) - 1; (*s == '0') && (s > p+2); s--)
					*s = 0;
			}
		}
	}

	var->value = fValue;

	if (changed)		// JDH: before, "changed" condition was present only on first if-statement below
	{
		if ((var->flags & CVAR_FLAG_SERVER) && sv.active)
		{
			//if (!(!strcmp(var_name, "cl_maxfps") && cl.gametype != GAME_DEATHMATCH && var->value > 72))
				SV_BroadcastPrintf ("\"%s\" changed to \"%s\"\n", var->name, var->string);
		}

		// joe, from ProQuake: rcon (64 doesn't mean anything special, but we need some extra space because NET_MAXMESSAGE == RCON_BUFF_SIZE)
		if (rcon_active && (rcon_message.cursize < rcon_message.maxsize - strlen(var->name) - strlen(var->string) - 64))
		{
			rcon_message.cursize--;
			MSG_WriteString (&rcon_message, va("\"%s\" set to \"%s\"\n", var->name, var->string));
		}

		if (var->name && !strcmp(var->name, "pq_lag"))
		{
			var->value = bound (0, var->value, 400);
			Cbuf_AddText (va("say \"ping +%d\"\n", (int)var->value), SRC_COMMAND);
		}

		return true;
	}

	return false;
}

/*
===================
Cvar_SetValueDirect
===================
*/
qboolean Cvar_SetValueDirect (cvar_t *var, float value)
{
	char	val[128];

	Q_snprintfz (val, sizeof(val), "%.6f", value);
		// Cvar_SetDirect will chop off any extra 0's
	
	return Cvar_SetDirect (var, val);
}

/*
============
Cvar_Set
============
*/
qboolean Cvar_Set (const char *var_name, const char *value)
{
	cvar_t		*var;

	if (!(var = Cvar_FindVar(var_name)))
	{	// there is an error in C code if this happens
		Con_Printf ("Cvar_Set: variable \"%s\" not found\n", var_name);
		return false;
	}

	return Cvar_SetDirect (var, value);
}

/*
============
Cvar_SetValue
============
*/
/*
void Cvar_SetValue (char *var_name, float value)
{
	cvar_t		*var;

	if (!(var = Cvar_FindVar(var_name)))
	{	// there is an error in C code if this happens
		Con_Printf ("Cvar_SetValue: variable \"%s\" not found\n", var_name);
		return;
	}

	Cvar_SetValueDirect (var, value);
}
*/
/*
============
Cvar_ToggleValue
============
*/
qboolean Cvar_ToggleValue (cvar_t *var)
{
	float newval;

	if (var->type == CVAR_STRING)
	{
		Con_DPrintf ("  Can't toggle string cvar %s\n", var->name);
		return false;
	}

	if (var->maxvalue == CVAR_UNKNOWN_MAX)
	{
		Con_DPrintf ("  Can't toggle cvar %s (range is not known)\n", var->name);
		return false;
	}

	if (var->value == var->maxvalue)
	{
		newval = var->minvalue;
	}
	else if (var->value == var->minvalue)
	{
		newval = var->maxvalue;
	}
	else
	{
		Con_DPrintf ("  Can't toggle cvar %s (current value must be either min or max)\n", var->name);
		return false;
	}

	Cvar_SetDirect (var, va("%f", newval));
	return true;
}

/*
============
Cvar_Cycle - JDH
============
*/
qboolean Cvar_Cycle (cvar_t *var, const char *cmd, qboolean reverse, qboolean wrap, float step)
{
	float newval;

	if (var->type == CVAR_STRING)
	{
		Con_DPrintf ("  Can't %s string cvar %s\n", cmd, var->name);
		return false;
	}

	if (step == 0)
	{
		if (var->type == CVAR_INT)
		{
		#ifndef RQM_SV_ONLY
			if (var == &scr_viewsize)
				step = 10;
			else
		#endif
			{
				if (/*(var->maxvalue - var->minvalue > 10) ||*/ (var->maxvalue == CVAR_UNKNOWN_MAX))
				{
					Con_DPrintf ("  Can't %s cvar %s (too many possible values)\n", cmd, var->name);
					return false;
				}

				step = 1;
			}
		}
		else
		{
			if ((var->maxvalue - var->minvalue > 1.0) || (var->maxvalue == CVAR_UNKNOWN_MAX))
			{
				Con_DPrintf ("  Can't %s cvar %s (too many possible values)\n", cmd, var->name);
				return false;
			}

			step = 0.1;
		}
	}

	if (reverse)
	{
		if (var->value <= var->minvalue)
		{
			if (!wrap) return false;
			newval = var->maxvalue;			// wraparound
		}
		else
		{
			if (var->value > var->maxvalue)
				var->value = var->maxvalue;

			newval = var->value - step;
			if (newval < var->minvalue)
				newval = var->minvalue;
		}
	}
	else
	{
		if (var->value >= var->maxvalue)
		{
			if (!wrap) return false;
			newval = var->minvalue;			// wraparound
		}
		else
		{
			if (var->value < var->minvalue)
				var->value = var->minvalue;

			newval = var->value + step;
			if (newval > var->maxvalue)
				newval = var->maxvalue;
		}
	}

	Cvar_SetDirect (var, va("%f", newval));
	return true;
}

/*
============
Cvar_CycleValue - JDH
============
*/
qboolean Cvar_CycleValue (cvar_t *var, qboolean reverse, qboolean wrap)
{
	return Cvar_Cycle (var, "cycle", reverse, wrap, 0);
}

/*
============
Cvar_RegisterTypeBounds

Adds a freestanding variable to the variable list.
============
*/
void Cvar_RegisterTypeBounds (cvar_t *var, cvartype_t type, float min, float max)
{
// first check to see if it has already been defined
	if (Cvar_FindVar(var->name))
	{
		Con_Printf ("Can't register variable %s, already defined\n", var->name);
		return;
	}

// check for overlap with a command
	if (Cmd_Exists(var->name))
	{
		Con_Printf ("Cvar_Register: %s is a command\n", var->name);
		return;
	}

	if (var->defaultvalue)
		var->string = var->defaultvalue;		// JDH: when re-registering a cvar that has been unregistered
	else
		var->defaultvalue = CopyString (var->string);

// copy the value off, because future sets will Z_Free it
	var->string = CopyString (var->string);
	var->type = type;
	var->minvalue = min;
	var->maxvalue = max;

	var->value = Q_atof (var->string);

// link the variable in
	//var->next = cvar_vars;
	//cvar_vars = var;

	//johnfitz -- insert each entry in alphabetical order
    if (cvar_vars == NULL || strcmp(var->name, cvar_vars->name) < 0) //insert at front
	{
        var->next = cvar_vars;
        cvar_vars = var;
    }
    else //insert later
	{
        cvar_t *prev = cvar_vars;
        cvar_t *cursor = cvar_vars->next;

        while (cursor && (strcmp(var->name, cursor->name) > 0))
		{
            prev = cursor;
            cursor = cursor->next;
        }
        var->next = prev->next;
        prev->next = var;
    }
	//johnfitz
}

/*
============
Cvar_Unregister

Removes a variable from the variable list.
============
*/
void Cvar_Unregister (cvar_t *var)
{
	cvar_t	*curr, *prev = NULL;

	for (curr = cvar_vars ; curr ; curr = curr->next)
	{
		//if (!strcmp(curr->name, var->name))
		if (curr == var)
		{
			if (curr->string)
			{
				Z_Free (curr->string);
				curr->string = NULL;
			}

			if (curr->savedvalue)
			{
				Z_Free (curr->savedvalue);
				curr->savedvalue = NULL;
			}
			
			// IMPORTANT: don't free var->defaultvalue - it's needed when re-registering

			if (prev)
				prev->next = curr->next;
			else
				cvar_vars = curr->next;

			return;
		}
		prev = curr;
	}

	Con_DPrintf ("Cvar_UnregisterVariable: %s not found\n", var->name);
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (void)
{
//	extern qboolean	config_exec_rq;
	cvar_t	*var;
	int i;

// check variables
	if (!(var = Cvar_FindVar(Cmd_Argv(0))))
		return false;

// perform a variable print or set
	if (Cmd_Argc() == 1)
	{
//		Con_Printf ("\"%s\" is \"%s\"\n", var->name, var->string);

		if (var->type == CVAR_INT)
		{
			Con_Print ("  Valid values: ");

			if (var->maxvalue == CVAR_UNKNOWN_MAX)
			{
				Con_Printf ("integers, %d and above\n", (int)var->minvalue);
			}
			else if (var->maxvalue - var->minvalue < 5)
			{
				for (i = (int)var->minvalue; i < (int)var->maxvalue; i++)
					Con_Printf ("%d/", i);
				Con_Printf ("%d\n", (int)var->maxvalue);
			}
			else
				Con_Printf ("integers, %d to %d\n", (int)var->minvalue, (int)var->maxvalue);

		//	Con_Printf (" (default is %d)\n", atoi(var->defaultvalue));
		}
		else if (var->type == CVAR_FLOAT)
		{
			Con_Print ("  Valid values: ");

			if (var->maxvalue == CVAR_UNKNOWN_MAX)
			{
				Con_Printf ("%.1f and higher\n", var->minvalue);
			}
			else Con_Printf ("%.1f to %.1f\n", var->minvalue, var->maxvalue);

		//	Con_Printf (" (default is %.1f)\n", atof(var->defaultvalue));
		}

//		Con_Printf ("  Current value of %s is \x02%s", var->name, var->string);
		Con_Printf ("  Current value: %s", var->type == CVAR_STRING ? va("\"%s\"", var->string) : var->string);
		/*if (var->type == CVAR_INT)
			Con_Printf ("\x02%d\n", (int)var->value);

		else if (var->type == CVAR_FLOAT)
			Con_Printf ("\x02%.1f\n", var->value);

		else */
		if (var->flags & CVAR_FLAG_READONLY)
		{
			Con_Print ("\n");
		}
		else if (!strcmp(var->string, var->defaultvalue))
		{
			Con_Print (" (default)\n");
		}
		else
		{
//			Con_Printf ("\x02\"%s\"\n", var->string);
			Con_Printf (" (default is %s)\n", var->type == CVAR_STRING ? va("\"%s\"", var->defaultvalue) : var->defaultvalue);
		}

		return true;
	}

	Cvar_SetDirect (var, Cmd_Argv(1));
	return true;
}

/*
====================
Cvar_List_f

List all console variables
  JDH: moved from cmd.c; updated with code from Fitz; pauses after screenful
====================
*/
void Cvar_List_f (cmd_source_t src)
{
	cvar_t	*var;
	int		part_len, counter, len;
	const char 	*partial;
	char	buf[256];

	if (src == SRC_CLIENT)
		return;

	if (Cmd_Argc() > 1)
	{
		partial = Cmd_Argv (1);
		part_len = strlen (partial);
	}
	else
	{
		partial = NULL;
		part_len = 0;
	}

	Con_PagedOutput_Begin ();
	counter = 0;
	for (var = cvar_vars; var; var = var->next)
	{
		if (partial && Q_strncasecmp (partial, var->name, part_len))
		{
			continue;
		}

		// gl_externaltextures_bmodels is longest cvar name (27 letters)

		Con_Printf ("  %-28s", var->name);

		if (var->type == CVAR_INT)
		{
			len = Q_snprintfz (buf, sizeof(buf), " %-16s", var->string);

			if ((var->minvalue == 0) && (var->maxvalue == 1))
				len += Q_snprintfz (buf+len, sizeof(buf)-len, "(bool)\n");
			else
			{
				len += Q_snprintfz (buf+len, sizeof(buf)-len, "(int %d ", (int)var->minvalue);

				if (var->maxvalue == CVAR_UNKNOWN_MAX)
					Q_snprintfz (buf+len, sizeof(buf)-len, "and higher)\n");
				else
					Q_snprintfz (buf+len, sizeof(buf)-len, "to %d)\n", (int)var->maxvalue);
			}
		}
		else if (var->type == CVAR_FLOAT)
		{
			len = Q_snprintfz (buf, sizeof(buf), " %-16s(float %.1f ", var->string, var->minvalue);

			if (var->maxvalue == CVAR_UNKNOWN_MAX)
				Q_snprintfz (buf+len, sizeof(buf)-len, "and higher)\n");
			else
				Q_snprintfz (buf+len, sizeof(buf)-len, "to %.1f)\n", var->maxvalue);
		}
		else
			Q_snprintfz (buf, sizeof(buf), " \"%s\"\n", var->string);

//		if (!Con_PagedOutput (CON_OP_PRINT, buf))
		if (!Con_Print (buf))
		{
			Con_PagedOutput_End ();
			return;
		}

		/*Con_Printf ("  %-28s", var->name);

		if (var->type == CVAR_INT)
		{
			Con_Printf (" %-16s", var->string);

			if ((var->minvalue == 0) && (var->maxvalue == 1))
				Con_Print ("(bool)\n");
			else
			{
				Con_Printf ("(int %d ", (int)var->minvalue);

				if (var->maxvalue == CVAR_UNKNOWN_MAX)
					Con_Print ("and higher)\n");
				else
					Con_Printf ("to %d)\n", (int)var->maxvalue);
			}
		}
		else if (var->type == CVAR_FLOAT)
		{
			Con_Printf (" %-16s(float %.1f ", var->string, var->minvalue);

			if (var->maxvalue == CVAR_UNKNOWN_MAX)
				Con_Print ("and higher)\n");
			else
				Con_Printf ("to %.1f)\n", var->maxvalue);
		}
		else
			Con_Printf (" \"%s\"\n", var->string);*/

		counter++;
	}

	Con_Printf ("%i cvars", counter);
	if (partial)
	{
		Con_Printf (" beginning with \"%s\"\n", partial);
	}
	else Con_Print ("\n");

	Con_PagedOutput_End ();

	/*for (counter = 0, var = cvar_vars ; var ; var = var->next, counter++)
		Con_Printf ("%s\n", var->name);

	Con_Printf ("------------\n%d variables\n", counter);*/
}

/*
====================
Cvar_Toggle_f (JDH; idea from Fitz)
====================
*/
void Cvar_Toggle_f (cmd_source_t src)
{
	const char *name;
	cvar_t *var;

	if (Cmd_Argc() != 2)
	{
		Con_Print ("  toggle <var> : switch the value of a cvar between its min and its max\n");
		return;
	}

	name = Cmd_Argv(1);
	if (!(var = Cvar_FindVar(name)))
	{
		Con_Printf ("\"%s\" is not a valid cvar\n", name);
		return;
	}

	if (Cvar_ToggleValue (var))
		Con_Printf ("  --> %s set to %s\n", name, var->string);
}

/*
====================
Cvar_Cycle_f (JDH; idea from Fitz)
====================
*/
void Cvar_Cycle_f (cmd_source_t src)
{
	const char *name;
	cvar_t *var;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("  cycle <var> : set a cvar to the next valid value\n");
		return;
	}

	name = Cmd_Argv(1);
	if (!(var = Cvar_FindVar(name)))
	{
		Con_Printf ("\"%s\" is not a valid cvar\n", name);
		return;
	}

	if (Cvar_CycleValue (var, false, true))
		Con_Printf ("  --> %s set to %s\n", name, var->string);
}

/*
====================
Cvar_IncDec_f (JDH)
====================
*/
void Cvar_IncDec_f (cmd_source_t src, qboolean should_dec)
{
	const char	*cmd_name, *name;
	cvar_t	*var;
	float	amt;

	cmd_name = Cmd_Argv (0);

	if (Cmd_Argc() < 2)
	{
		Con_Printf ("  %s <var> [amount]: %srease the value of a cvar\n", cmd_name, cmd_name);
		Con_Print ("  If [amount] is not given, the default amount will be used\n");
		return;
	}

	name = Cmd_Argv(1);
	if (!(var = Cvar_FindVar(name)))
	{
		Con_Printf ("\"%s\" is not a valid cvar\n", name);
		return;
	}

	if (Cmd_Argc() > 2)
		amt = Q_atof (Cmd_Argv(2));
	else
		amt = 0;

	if (Cvar_Cycle (var, cmd_name, should_dec, false, amt))
		Con_Printf ("  --> %s set to %s\n", name, var->string);
}

/*
=================
Cvar_Inc_f (JDH)
=================
*/
void Cvar_Inc_f (cmd_source_t src)
{
	Cvar_IncDec_f (src, false);
}

/*
=================
Cvar_Dec_f (JDH)
=================
*/
void Cvar_Dec_f (cmd_source_t src)
{
	Cvar_IncDec_f (src, true);
}

/*
====================
Cvar_ValueEquals (JDH)
====================
*/
qboolean Cvar_ValueEquals (const cvar_t *var, const char *testval)
{
	float val;

	if (var->type == CVAR_STRING)
	{
		if (var->flags & CVAR_FLAG_NOCASE)
			return !Q_strcasecmp(var->string, testval);
		return !strcmp (var->string, testval);
	}

	// weird stuff happens here with representation of float vs. double,
	//  hence the second check below
	val = Q_atof (testval);
	return ((var->value == val) || (*(int *)&var->value == *(int *)&val));
}

/*
====================
Cvar_IsDefaultValue (JDH)
====================
*/
qboolean Cvar_IsDefaultValue (const cvar_t *var)
{
	return Cvar_ValueEquals (var, var->defaultvalue);
}

/*
====================
Cvar_IsArchivedValue (JDH)
====================
*/
qboolean Cvar_IsArchivedValue (const cvar_t *var)
{
	if (!var->savedvalue)
		return true;		// it's allocated only when value changes
	
	return Cvar_ValueEquals (var, var->savedvalue);
}

/*
============
Cvar_WriteVariables

Writes lines containing "variable "value"" for all variables that should be archived
(according to the criteria determined by the value of cfg_savevars)

  If file pointer is null, just returns count.
============
*/
int Cvar_WriteVariables (FILE *f)
{
	cvar_t	*var;
	int		count = 0;

	for (var = cvar_vars ; var ; var = var->next)
	{
		if (cfg_savevars.value == 0)
		{
			if (!(var->flags & CVAR_FLAG_ARCHIVE) || Cvar_IsArchivedValue(var))
				continue;
		}
		else if (cfg_savevars.value == 1)
		{
			if (!(var->flags & CVAR_FLAG_ARCHIVE) || Cvar_IsDefaultValue(var))
				continue;
		}

		count++;
		if (f)
		{
			if (count == 1)		// JDH: print section name only if there's at least 1 var written
				fprintf (f, "\n// Variables\n");

			//	if ((var->flags & CVAR_FLAG_ARCHIVE) || (cfg_savevars.value == 1 && !Cvar_IsDefaultValue(var)) || cfg_savevars.value == 2)
			fprintf (f, "%s \"%s\"\n", var->name, var->string);
		}
	}

	return count;
}

/*
============
Cvar_ResetAll
============
*/
void Cvar_ResetAll (void)
{
	cvar_t	*var;
	int flags;

	for (var = cvar_vars ; var ; var = var->next)
	{
	// skip gamma & contrast, otherwise screen flashes when changing gamedir
		if (strcmp(var->name, "gl_gamma") && strcmp(var->name, "gl_contrast"))
		{
			flags = var->flags;
			var->flags &= ~CVAR_FLAG_READONLY;
			Cvar_SetDirect (var, var->defaultvalue);
			var->flags = flags;
		}

		if (var->savedvalue)
		{
			Z_Free (var->savedvalue);
			var->savedvalue = NULL;
		}
	}
}

void Cvar_Init (void)
{
	Cvar_RegisterInt (&cfg_savevars, 0, 2);

	Cmd_AddCommand ("cvarlist", Cvar_List_f, 0);		// JDH: was in Cmd_Init
	Cmd_AddCommand ("toggle", Cvar_Toggle_f, 0);
	Cmd_AddCommand ("cycle", Cvar_Cycle_f, 0);
	Cmd_AddCommand ("inc", Cvar_Inc_f, 0);
	Cmd_AddCommand ("dec", Cvar_Dec_f, 0);
}
