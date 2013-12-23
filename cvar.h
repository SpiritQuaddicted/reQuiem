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
// cvar.h

/*

cvar_t variables are used to hold scalar or string variables that can be changed
or displayed at the console or prog code as well as accessed directly in C code.

it is sufficient to initialize a cvar_t with just the first two fields, or
you can add a ,true flag for variables that you want saved to the configuration
file when the game is quit:

cvar_t	r_draworder = {"r_draworder","1"};
cvar_t	scr_screensize = {"screensize","1",true};

Cvars must be registered before use, or they will have a 0 value instead of the
float interpretation of the string. Generally, all cvar_t declarations should be
registered in the apropriate init function before any console commands are executed:
Cvar_RegisterVariable (&host_framerate);

C code usually just references a cvar in place:
if (r_draworder.value)

It could optionally ask for the value to be looked up for a string name:
if (Cvar_VariableValue ("r_draworder"))

Interpreted prog code can access cvars with the cvar(name) or
cvar_set (name, value) internal functions:
teamplay = cvar("teamplay");
cvar_set ("registered", "1");

The user can access cvars from the console in two ways:
r_draworder		prints the current value
r_draworder 0		sets the current value to 0
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/

typedef enum {CVAR_STRING, CVAR_INT, CVAR_FLOAT} cvartype_t;

#define CVAR_FLAG_ARCHIVE    0x0001		/* if set, var is saved to reQuiem.cfg */
#define CVAR_FLAG_SERVER     0x0002		/* if set, notifies players when changed */
#define CVAR_FLAG_NOCASE     0x0004		/* if set, string value is not case-sensitive */
#define CVAR_FLAG_READONLY   0x0008

typedef struct cvar_s
{
	const char	*name;
	char		*string;
//	qboolean	archive;		
//	qboolean	server;
	int			flags;
	qboolean	(*OnChange)(struct cvar_s *var, const char *value);
	float		value;
	char		*defaultvalue;
	cvartype_t	type;				// JDH
	float		minvalue, maxvalue;	// JDH
	char		*savedvalue;		// JDH
	struct cvar_s *next;
} cvar_t;

#define CVAR_UNKNOWN_MAX 0x7FFFFFFF

/*
registers a cvar that already has the name, string, and optionally the
archive elements set.
Adds a freestanding variable to the variable list.
============		
*/
void 	Cvar_RegisterTypeBounds (cvar_t *, cvartype_t, float min, float max);

#define Cvar_RegisterString(var)        Cvar_RegisterTypeBounds((var), CVAR_STRING, 0, 0)
#define	Cvar_RegisterBool(var)          Cvar_RegisterTypeBounds((var), CVAR_INT, 0, 1)
#define	Cvar_RegisterFloat(var, min, max) Cvar_RegisterTypeBounds((var), CVAR_FLOAT, (min), (max))
#define	Cvar_RegisterInt(var, min, max)   Cvar_RegisterTypeBounds((var), CVAR_INT, (min), (max))

#define Cvar_Register(var) Cvar_RegisterString((var))

void Cvar_Unregister (cvar_t *var);

void Cvar_ResetAll (void);

// JDH: added SetDirect calls for better performance
qboolean 	Cvar_SetDirect (cvar_t *cvar, const char *value);
qboolean 	Cvar_SetValueDirect (cvar_t *cvar, float value);

qboolean 	Cvar_Set (const char *var_name, const char *value);
// equivelant to "<name> <variable>" typed at the console

//void	Cvar_SetValue (char *var_name, float value);
// expands value to a string and calls Cvar_Set

qboolean Cvar_CycleValue (cvar_t *var, qboolean reverse, qboolean wrap);
qboolean Cvar_ToggleValue (cvar_t *var);

float	Cvar_VariableValue (const char *var_name);
// returns 0 if not defined or non numeric

char	*Cvar_VariableString (const char *var_name);
// returns an empty string if not defined

int Cvar_CompleteCountPossible (const char *partial);		// by joe

//char 	*Cvar_CompleteVariable (const char *partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

qboolean Cvar_Command (void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

int		Cvar_WriteVariables (FILE *f);
// Writes lines containing "set variable value" for all variables
// with the archive flag set to true.

qboolean Cvar_IsDefaultValue (const cvar_t *var);

cvar_t *Cvar_FindVar (const char *var_name);
void Cvar_Init (void);

extern cvar_t	*cvar_vars;
