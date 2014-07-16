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
// cmd.c -- Quake script command processing module

#include "quakedef.h"
#include "winquake.h"

#ifndef _WIN32
#include <glob.h>
#include <sys/stat.h>
#endif

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char		name[MAX_ALIAS_NAME];
	char		*value;
} cmdalias_t;

cmdalias_t	*cmd_alias;

//#define	MAX_ARGS	80

//static	int	cmd_argc;
//int				cmd_argc;
//static	char	*cmd_argv[MAX_ARGS];
cmd_arglist_t cmd_arglist;

qboolean	cmd_wait;
qboolean	config_exec = false;		// true only when exec'ing the contents of a cfg file
qboolean	config_exec_rq = false;		// true only when exec'ing reQuiem.cfg

extern cvar_t com_matchfilecase;
extern qboolean dzlib_loaded;

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
void Cmd_Wait_f (cmd_source_t src)
{
	cmd_wait = true;
}

/*
=============================================================================

				COMMAND BUFFER

=============================================================================
*/

sizebuf_t	cmd_text;

// JDH: special character sequence that may follow a line in the cbuf:
//   (this is followed by another byte, representing the cmd_source_t value)
#define CBUF_SRCMARKER "\xFD\xCE"

/*
============
Cbuf_Init
============
*/
void Cbuf_Init (void)
{
	SZ_Alloc (&cmd_text, 32768);		// space for commands and script files
		// JDH: was originally 8192 bytes
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddText (const char *text, cmd_source_t src)
{
/*	int	len;

	len = strlen (text);
	if (!len) return;

	if (cmd_text.cursize + len >= cmd_text.maxsize)
	{
		Con_Print ("Cbuf_AddText: overflow\n");
		return;
	}

	SZ_Write (&cmd_text, text, len);
*/

	int quotes, len;
	const char *line;
	char marker[4] = CBUF_SRCMARKER;

// JDH: rewritten to add text line-by-line, so each command gets tagged with its source
	quotes = 0;

	for (line = text; ; text++)
	{
		if (*text == '"')
			quotes++;

		if (!*text || (*text == '\n') || (!(quotes & 1) && (*text == ';')))	// don't break on ; if inside a quoted string
		{
			len = text - line;
			if (*text) len++;		// don't include null char, if that's what we've hit
			if (len)
			{
				if (cmd_text.cursize + len >= cmd_text.maxsize)
				{
					Con_Print ("Cbuf_AddText: overflow\n");
					return;
				}

				SZ_Write (&cmd_text, line, len);

				if (*text && (cmd_text.cursize + 3 < cmd_text.maxsize))
				{
				// add cmd_source marker only if this is the end of the line
					marker[2] = src | 0x80;
					SZ_Write (&cmd_text, marker, 3);
				}
			}

			if (!*text)
				break;

			quotes = 0;
			line = text+1;
		}
	}
}

/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
FIXME: actually change the command buffer to do less copying
============
*/
void Cbuf_InsertText (const char *text, cmd_source_t src)
{
	char	*temp;
	int	templen;

// copy off any commands still remaining in the exec buffer
	if ((templen = cmd_text.cursize))
	{
		temp = Z_Malloc (templen);
		memcpy (temp, cmd_text.data, templen);
		SZ_Clear (&cmd_text);
	}
	else
	{
		temp = NULL;	// shut up compiler
	}

// add the entire text of the file
	Cbuf_AddText (text, src);
	Cbuf_AddText ("\n", src);		// JDH: in case file is missing newline at end

// add the copied off data
	if (templen)
	{
		SZ_Write (&cmd_text, temp, templen);
		Z_Free (temp);
	}
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute (void)
{
	int	i, quotes;
	char	*text, line[1024];
	cmd_source_t src;

	while (cmd_text.cursize)
	{
// find a \n or ; line break
		text = (char *)cmd_text.data;

		quotes = 0;
		for (i=0 ; i<cmd_text.cursize ; i++)
		{
			if (text[i] == '"')
				quotes++;
			if (!(quotes & 1) &&  text[i] == ';')
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
			
			if (text[i] == '\r')		// JDH
			{
				if (text[i+1] == '\n')
					text[i++] = 0;
				break;
			}
		}

		Q_strncpy (line, sizeof(line), text, i);

	// JDH: check if line is followed by a cmd_source marker:
		if ((cmd_text.cursize-i >= 3) && (text[i+1] == CBUF_SRCMARKER[0]) && (text[i+2] == CBUF_SRCMARKER[1]))
		{
			i += 3;
			src = text[i] & 0x7F;
		}
		else
		{
			src = SRC_COMMAND;
		}

// delete the text from the command buffer and move remaining commands down
// this is necessary because commands (exec, alias) can insert data at the
// beginning of the text buffer

		if (i == cmd_text.cursize)
		{
			cmd_text.cursize = 0;
		}
		else
		{
			i++;
			cmd_text.cursize -= i;
			memcpy (text, text + i, cmd_text.cursize);
		}

// execute the command line
		Cmd_ExecuteString (line, src);

		if (cmd_wait)
		{	// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
}

/*
============
Cbuf_IsEmpty
============
*/
qboolean Cbuf_IsEmpty (void)
{
	return (cmd_text.cursize == 0);
}

/*
==============================================================================

				SCRIPT COMMANDS

==============================================================================
*/

/*
===============
Cmd_StuffCmds_f

Adds command line parameters as script statements
Commands lead with a +, and continue until a - or another +
quake +prog jctest.qp +cmd amlev1
quake -nosound +cmd amlev1
===============
*/
void Cmd_StuffCmds_f (cmd_source_t src)
{
	int	i, j, s;
	char	*text, *build, c;

// build the combined string to parse from
	s = 0;
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP nulls out -NXHost
		s += strlen (com_argv[i]) + 1;
	}
	if (!s)
		return;

// strcat's here should be safe, since buffer size is precalculated
	text = Z_Malloc (s + 1);
	text[0] = 0;
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP nulls out -NXHost
		strcat (text, com_argv[i]);
		if (i != com_argc-1)
			strcat (text, " ");
	}

// pull out the commands
	build = Z_Malloc (s + 1);
	build[0] = 0;

	for (i=0 ; i<s-1 ; i++)
	{
		if (text[i] == '+')
		{
			i++;

			for (j=i; ; j++)
			{
				if (!text[j]) break;
				if ((text[j] == '+') || (text[j] == '-'))
				{
					if (((byte *) text)[j-1] <= 32)
						break;			// JDH: break on '+' or '-' only if preceded by whitespace
				}
			}

			c = text[j];
			text[j] = 0;

			strcat (build, text + i);
			strcat (build, "\n");
			text[j] = c;
			i = j - 1;
		}
	}

	if (build[0])
//		Cbuf_AddText (build, SRC_CMDLINE);		// JDH: was Cbuf_InsertText when in Cmd_StuffCmds_f
		Cbuf_InsertText (build, SRC_CMDLINE);

	Z_Free (text);
	Z_Free (build);
}

/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f (cmd_source_t src)
{
	char	*f, *f2, name[MAX_OSPATH], *p;
	int		len, mark;
	static	qboolean id1_autoexec_done = false;
		// JDH: keep track of whether id1/autoexec.cfg has been exec'd

	if (cmd_arglist.argc != 2)
	{
		Con_Print ("exec <filename> : execute a script file\n");
		return;
	}

	len = Q_strcpy (name, cmd_arglist.argv[1], sizeof(name));

	for (p = name; *p; p++)
		if (*p == '\\') *p = '/';

	p = COM_SkipPath (name);

	mark = Hunk_LowMark ();
	if (!(f = (char *) COM_LoadHunkFile(name, 0)))
	{
		if (!strchr(p, '.'))
		{	// no extension, so try the default (.cfg)
			Q_strcpy (name+len, ".cfg", sizeof(name)-len);
			f = (char *)COM_LoadHunkFile (name, 0);
		}

		if (!f)
		{
			Con_Printf ("couldn't exec %s\n", name);
			// Normally requiem.cfg is executed after autoexec.cfg (see below). If
			// this was an attempt to exec autoexec.cfg, set the "done that" flag
			// and exec requiem.cfg now.
			if (!Q_strcasecmp(p, "autoexec.cfg") && !id1_autoexec_done)
			{
				Cbuf_InsertText ("exec reQuiem.cfg\n", SRC_COMMAND);
				id1_autoexec_done = true;
			}
			return;
		}
	}
	Con_Printf ("execing %s\n", name);

// JDH: I tag the contents of cfg files so I can track any cvars or keybindings loaded

	if (!Q_strcasecmp(p, "quake.rc")
#ifdef HEXEN2_SUPPORT
		|| (hexen2 && !Q_strcasecmp(p, "hexen.rc"))
#endif
		)
	{
		// JDH: since quake.rc is the first to exec when starting
		//      or changing gamedir, I'll reset this var here
		id1_autoexec_done = false;
		Cbuf_InsertText (f, SRC_CFG);
	}
	else if (!Q_strcasecmp(p, "autoexec.cfg"))
	{
		if (id1_autoexec_done)
		{
		// make sure it's not exec'd more than once
			/*if (!Q_strcasecmp(com_filepath->dir_name, GAMENAME))
			{
				Cbuf_InsertText (f, SRC_CFG);
			}*/
		}
		else
		{
			// JDH: I exec reQuiem.cfg after autoexec so that settings changed in-game
			//      will hold their value on the next run
			Cbuf_InsertText ("exec reQuiem.cfg\n", SRC_COMMAND);
		
			if (Q_strcasecmp(com_filepath->dir_name, GAMENAME))
			{
			// force id1/autoexec.cfg to be loaded first
				f2 = (char *)COM_LoadHunkFile ("../id1/autoexec.cfg", 0);
				if (f2)
					Cbuf_InsertText (f2, SRC_CFG);
			}
			id1_autoexec_done = true;
		}

		Cbuf_InsertText (f, SRC_CFG);
	}
	else if (!Q_strcasecmp(p, "config.cfg") || !Q_strcasecmp(p, "default.cfg"))
	{
		Cbuf_InsertText (f, SRC_CFG);
	}
	else if (!Q_strcasecmp(p, "reQuiem.cfg"))
	{
		//if (!Q_strcasecmp(com_filepath->dir_name, com_gamedirname))	removed 2009/06/28
		{
		// any setting loaded from reQuiem.cfg has to get written back there;
		//  that's why it gets this special tag. (This is true even if the file isn't
		//  loaded from the current gamedir, because the reQuiem.cfg that gets written
		//  on quit will override the other)

			Cbuf_InsertText (f, SRC_CFG_RQM);
		}
		//else
		//	Cbuf_InsertText (f, SRC_CFG);
	}
	else
		Cbuf_InsertText (f, SRC_COMMAND);

	Hunk_FreeToLowMark (mark);
}

/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f (cmd_source_t src)
{
	qboolean was_in_exec = config_exec;
	int	i;

	config_exec = false;		// so echo commands from cfg/rc files don't get blocked when changing gamedir
	
	for (i=1 ; i<cmd_arglist.argc ; i++)
		Con_Printf ("%s ", cmd_arglist.argv[i]);
	Con_Print ("\n");
	
	config_exec = was_in_exec;
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/
void Cmd_Alias_f (cmd_source_t src)
{
	cmdalias_t	*a;
	const char	*s;
	char		cmd_name[1024];
	int			len, i;

	if (cmd_arglist.argc == 1)
	{
		Con_Print ("Current alias commands:\n");
		for (a = cmd_alias ; a ; a = a->next)
			Con_Printf ("%s : %s\n", a->name, a->value);
		return;
	}

	s = Cmd_Argv (1);
	if (strlen(s) >= MAX_ALIAS_NAME)
	{
		Con_Print ("Alias name is too long\n");
		return;
	}

// if the alias already exists, reuse it
	for (a = cmd_alias ; a ; a = a->next)
	{
		if (!strcmp(s, a->name))
		{
			Z_Free (a->value);
			break;
		}
	}

	if (!a)
	{
		a = Z_Malloc (sizeof(cmdalias_t));
		a->next = cmd_alias;
		cmd_alias = a;
	}
	Q_strcpy (a->name, s, sizeof(a->name));

// copy the rest of the command line
/*	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i<cmd_arglist.argc ; i++)
	{
		strcat (cmd, cmd_arglist.argv[i]);
		if (i != cmd_arglist.argc)
			strcat (cmd, " ");
	}
	strcat (cmd, "\n");
*/
	len = 0;
	for (i=2 ; i<cmd_arglist.argc ; i++)
	{
		len += Q_strcpy (cmd_name+len, cmd_arglist.argv[i], sizeof(cmd_name)-len);
		if (i != cmd_arglist.argc-1)
			len += Q_strcpy (cmd_name+len, " ", sizeof(cmd_name)-len);
	}
	Q_strcpy (cmd_name+len, "\n", sizeof(cmd_name)-len);

	a->value = CopyString (cmd_name);
}

/*
=============================================================================

				COMMAND ADDITION/REMOVAL

=============================================================================
*/

static	cmd_function_t	*cmd_functions;		// possible commands to execute

// joe: legacy commands, from FuhQuake
typedef struct legacycmd_s
{
	char	*oldname, *newname;
	struct legacycmd_s *next;
} legacycmd_t;

static	legacycmd_t	*legacycmds = NULL;

void Cmd_AddLegacyCommand (const char *oldname, const char *newname)
{
	legacycmd_t	*cmd;

	cmd = (legacycmd_t *)Q_malloc (sizeof(legacycmd_t));
	cmd->next = legacycmds;
	legacycmds = cmd;

	cmd->oldname = CopyString (oldname);
	cmd->newname = CopyString (newname);
}

void Cmd_RemoveLegacyCommand (const char *oldname)
{
	legacycmd_t	*cmd, *prev = NULL;

	for (cmd = legacycmds ; cmd ; prev = cmd, cmd = cmd->next)
		if (!Q_strcasecmp(cmd->oldname, oldname))
			break;

	if (!cmd)
		return;

	Z_Free (cmd->oldname);
	Z_Free (cmd->newname);
	if (prev)
		prev->next = cmd->next;
	else
		legacycmds = cmd->next;
	free (cmd);
}

#define MAX_LEGACY_RECURSION 2

static qboolean Cmd_LegacyCommand (cmd_source_t src)
{
	static int	recursive = 0;
	legacycmd_t	*cmd;
	char		text[MAX_ARGBUF];

	for (cmd = legacycmds ; cmd ; cmd = cmd->next)
		if (!Q_strcasecmp(cmd->oldname, cmd_arglist.argv[0]))
			break;

	if (!cmd)
		return false;

	if (!cmd->newname[0])
		return true;		// just ignore this command

	// build new command string
	Q_snprintfz (text, sizeof(text), "%s %s", cmd->newname, Cmd_Args(1));

	assert (recursive < MAX_LEGACY_RECURSION);
	recursive++;
//	Cmd_ExecuteString (text, src_command);
// JDH: src should be the same as in original call to Cmd_ExecuteString:
	Cmd_ExecuteString (text, src);
	recursive--;

	return true;
}

/*
============
Cmd_AddCommand_internal
============
*/
void Cmd_AddCommand_internal (const char *cmd_name, xcommand_t function, int flags)
{
	cmd_function_t	*cmd;
	cmd_function_t	*cursor, *prev; //johnfitz -- sorted list insert

// fail if the command is a variable name
	if (Cvar_VariableString(cmd_name)[0])
	{
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}

// fail if the command already exists
	if (Cmd_Exists(cmd_name))
	{
		Con_Printf ("Cmd_AddCommand: %s already defined\n", cmd_name);
		return;
	}

	if (flags & CMD_TEMPORARY)
		cmd = Q_malloc (sizeof(cmd_function_t));
	else
		cmd = Hunk_AllocName (sizeof(cmd_function_t), "cmd");
	
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->flags = flags;

	//johnfitz -- insert each entry in alphabetical order
    if (cmd_functions == NULL || strcmp(cmd->name, cmd_functions->name) < 0) //insert at front
	{
        cmd->next = cmd_functions;
        cmd_functions = cmd;
    }
    else //insert later
	{
        prev = cmd_functions;
        cursor = cmd_functions->next;
        while ((cursor != NULL) && (strcmp(cmd->name, cursor->name) > 0))
		{
            prev = cursor;
            cursor = cursor->next;
        }
        cmd->next = prev->next;
        prev->next = cmd;
    }
	//johnfitz
}

/*
============
Cmd_AddCommand
============
*/
void Cmd_AddCommand (const char *cmd_name, xcommand_t function, int flags)
{
	if (host_initialized)	// because hunk allocation would get stomped
		Sys_Error ("Cmd_AddCommand after host_initialized");

	Cmd_AddCommand_internal (cmd_name, function, flags);
}


#ifdef HEXEN2_SUPPORT
/*
============
Cmd_AddGameCommand
  - allows adding of game-specific commands after host is initialized
============
*/
void Cmd_AddGameCommand (const char *cmd_name, xcommand_t function, int flags)
{
	Cmd_AddCommand_internal (cmd_name, function, flags | CMD_TEMPORARY);
}

/*
============
Cmd_RemoveGameCommand
  WARNING: command must have been added via Cmd_AddGameCommand (NOT Cmd_AddCommand)
============
*/
void Cmd_RemoveGameCommand (const char *cmd_name)
{
	cmd_function_t *curr, *prev;

	curr = cmd_functions;
	prev = NULL;

	while (curr)
	{
		if (!strcmp(cmd_name, curr->name))
		{
			if (!(curr->flags & CMD_TEMPORARY))
			{
			#ifdef _DEBUG
				Sys_Error ("Cmd_RemoveGameCommand: %s is not a game command!", cmd_name);
			#endif
				return;
			}

			if (prev)
				prev->next = curr->next;
			else
				cmd_functions = curr->next;

			free (curr);
			return;
		}

		prev = curr;
		curr = curr->next;
	}

	Con_DPrintf ("Cmd_RemoveCommand: %s not found\n", cmd_name);
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
============
Cmd_Exists
============
*/
qboolean Cmd_Exists (const char *cmd_name)
{
	cmd_function_t	*cmd;

	for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
	{
		if (!strcmp(cmd_name, cmd->name))
			return true;
	}

	return false;
}

#define	COLUMNWIDTH	20

extern	int	con_x;

/*
==================
PaddedPrint
==================
*/
static void PaddedPrint (const char *s)
{
	Con_ColumnPrint (s, COLUMNWIDTH);
}

/*
=============================================================================

				COMMAND PARSING/EXECUTION

=============================================================================
*/


//static	char	*cmd_null_string = "";
//static	char	*cmd_args = NULL;

//cmd_source_t	cmd_source;

/*
============
Cmd_Argc
============
*/
int Cmd_Argc (void)
{
	return cmd_arglist.argc;
}

/*
============
Cmd_Argv
============
*/
const char *Cmd_Argv (int arg)
{
	if ((unsigned)arg >= cmd_arglist.argc)
		return "";
	return cmd_arglist.argv[arg];
}

/*
============
Cmd_Args
============
*/
const char *Cmd_Args (int startarg)
{
//	if (!cmd_arglist.args)
//		return "";
//	return cmd_arglist.args;

	if ((unsigned)startarg >= cmd_arglist.argc)
		return "";
	
//	return cmd_arglist.args + cmd_arglist.argindex[startarg];
	return cmd_arglist.cmdline + cmd_arglist.args_ofs[startarg];
	
/*	int len, i;
	
	if (bufsize <= 0)
		return 0;
	
	if (cmd_arglist.argc <= 1)		// just command, no args
	{
		buf[0] = 0;
		return 0;
	}
	
	i = cmd_arglist.argv[1] - cmd_arglist.buf;		// length of command string + \0
	len = min(bufsize-1, cmd_arglist.buf_used-i);

	memcpy (buf, cmd_arglist.buf+i, len);

// replace all the null terminators with spaces (except the last one)
	for (i = 0; i < len-1; i++)
	{
		if (!buf[i])
			buf[i] = ' ';
	}
	
	buf[bufsize-1] = 0;
	return len;
*/
}

/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
  JDH: reworked to return result in a struct instead of globals
============
*/
void Cmd_TokenizeString (const char *text, cmd_arglist_t *out)
{
	int	idx = 0, len;
	const char *args = NULL;

	out->argc = 0;
//	out->args = NULL;
//	out->args[0] = 0;
	memset (out->args_ofs, 0, sizeof(out->args_ofs));
	out->buf[0] = 0;
//	out->buf_used = 0;
	memset (out->argv, 0, sizeof(out->argv));
	out->cmdline = NULL;
	args = text;

	while (1)
	{
	// skip whitespace up to a /n
		while (*text == ' ' || *text == '\t' || *text == '\r')
			text++;

		if (*text == '\n')	// a newline seperates commands in the buffer
			break;

		if (!*text)
			break;

		//if (out->argc == 1)
		//	 args = (char *) text;	//out->args = (char *)text;

		out->args_ofs[out->argc] = text-args;

		if (!(text = COM_Parse(text)))
			break;

		if (out->argc < MAX_ARGS)
		{
			out->argv[out->argc] = out->buf + idx;
			//strcpy (out->argv[out->argc], com_token);
			//idx += strlen(com_token) + 1;
			len = Q_strcpy (out->buf + idx, com_token, sizeof(out->buf)-idx);
			idx += len+1;
		//	out->buf_used = idx;
			out->argc++;
		}
	}

// shift offsets to after tokenized args
//	for (i = 0; i < out->argc; i++)
//	{
///		out->args_ofs[i] += idx;
//	}

// append the original command line
	if (text)
		len = text - args;
	else
		len = Q_MAXINT;

	Q_strncpy (out->buf + idx, sizeof(out->buf) - idx, args, len);
	out->cmdline = out->buf + idx;
}

/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
void Cmd_ExecuteString (const char *text, cmd_source_t src)
{
	cmd_function_t	*cmd;
	cmdalias_t	*a;

//	cmd_source = src;
	Cmd_TokenizeString (text, &cmd_arglist);

#ifdef _DEBUG
	if (!strncmp(text, "echo ", 5))
		a = NULL;
#endif
	
	// execute the command line
	if (!cmd_arglist.argc)
		return;		// no tokens

// JDH: check for special internal markers:
	/*if (!strncmp (cmd_arglist.argv[0], "CFG_START", 9))
	{
		config_exec++;
		if (!strcmp(cmd_arglist.argv[0]+9, "_RQ"))
			config_exec_rq = true;
		return;
	}
	else if (!strncmp (cmd_arglist.argv[0], "CFG_END", 7))
	{
		config_exec--;
		if (!strcmp(cmd_arglist.argv[0]+7, "_RQ"))
			config_exec_rq = false;
		return;
	}*/

	if (src == SRC_CFG_RQM)
	{
		config_exec = config_exec_rq = true;
	}
	else if (src == SRC_CFG)
	{
		config_exec = true;
		config_exec_rq = false;
	}
	else
	{
		config_exec = config_exec_rq = false;
	}

// check functions
	for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
	{
		if (!Q_strcasecmp(cmd_arglist.argv[0], cmd->name))
		{
		// JDH: this check is to prevent command-line commands from being re-executed on gamedir change
			if ((src != SRC_CMDLINE) || (host_time < 1.5))
			{
				if (cmd->flags & CMD_DISABLED)	// JDH
				{
					if (src == SRC_CONSOLE)
						Con_Printf ("The command \"%s\" is not currently active.\n", cmd->name);
				}
				else
					cmd->function (src);
			}
			return;
		}
	}

// check alias
	for (a = cmd_alias ; a ; a = a->next)
	{
		if (!Q_strcasecmp(cmd_arglist.argv[0], a->name))
		{
			Cbuf_InsertText (a->value, src);
			return;
		}
	}

// check cvars
	if (Cvar_Command())
		return;

// joe: check legacy commands
	if (Cmd_LegacyCommand(src))
		return;

	Con_Printf ("Unknown command \"%s\"\n", Cmd_Argv(0));
}

#ifndef RQM_SV_ONLY
/*
===================
Cmd_ForwardToServer

Sends the entire command line over to the server
===================
*/
void Cmd_ForwardToServer (cmd_source_t src)
{
	const char *cmd_name;

	if (cls.state != ca_connected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	if (cls.demoplayback)
		return;		// not really connected

	MSG_WriteByte (&cls.message, clc_stringcmd);
	cmd_name = Cmd_Argv(0);
	if (Q_strcasecmp(cmd_name, "cmd"))
	{
		if (nehahra)
		{
			// JDH: convert cheat cmds to nehahra format:
			if (!Q_strcasecmp(cmd_name, "god"))
				cmd_name = "max";
			else if (!Q_strcasecmp(cmd_name, "notarget"))
				cmd_name = "monster";
			else if (!Q_strcasecmp(cmd_name, "fly"))
				cmd_name = "scrag";
			else if (!Q_strcasecmp(cmd_name, "give"))
				cmd_name = "gimme";
			else if (!Q_strcasecmp(cmd_name, "noclip"))
				cmd_name = "wraith";
		}

		SZ_Print (&cls.message, cmd_name);
		SZ_Print (&cls.message, " ");
	}

	if (cmd_arglist.argc > 1)
		SZ_Print (&cls.message, Cmd_Args(1));
	else
		SZ_Print (&cls.message, "\n");
}
#endif		//#ifndef RQM_SV_ONLY

/*
================
Cmd_CheckParm

Returns the position (1 to argc-1) in the command's argument list
where the given parameter apears, or 0 if not present
================
*/
int Cmd_CheckParm (const char *parm)
{
	int	i;

	if (!parm)
		Sys_Error ("Cmd_CheckParm: NULL");

	for (i=1 ; i<cmd_arglist.argc ; i++)
		if (!Q_strcasecmp(parm, cmd_arglist.argv[i]))
			return i;

	return 0;
}

/*
=============================================================================

				FILELIST MANAGEMENT

=============================================================================
*/


//int		RDFlags = 0;
file_entry_t	*filelist = NULL;
int				num_files = 0;

//file_entry_t  *param_filelist = NULL;
//int			param_num_files = 0;

//static	char	filetype[8] = "file";

#ifdef _WIN32
static void toLower (char *str)		// for strings
{
	char	*s;

	for (s = str; *s; s++)
	{
		if (*s >= 'A' && *s <= 'Z')
			*s += 32;
	}
}
#endif

int Cmd_AddFilelistEntry (const char *fname, cmd_ftype_t ftype, long fsize, int flags)
{
	char *namecopy;
	int	i, pos;
	int (*cmp_proc)(const char *, const char *);

	filelist = Q_realloc (filelist, (num_files + 1) * sizeof(file_entry_t));
	namecopy = Q_strdup (fname);

#ifdef _WIN32
	if (!(flags & AFE_KEEPCASE))
		toLower (namecopy);
	// else don't convert, linux is case sensitive
#endif

	if (!(flags & (AFE_NO_NAMESORT | AFE_NO_TYPESORT)))
	{
		cmp_proc = (com_matchfilecase.value) ? strcmp : Q_strcasecmp;

		// inclusion sort
		for (i=0 ; i<num_files ; i++)
		{
			if (!(flags & AFE_NO_TYPESORT))
			{
				if (ftype < filelist[i].type)
					continue;
				else if (ftype > filelist[i].type)
					break;
			}

			if (!(flags & AFE_NO_NAMESORT))
			{
				if (cmp_proc(namecopy, filelist[i].name) < 0)
					break;
			}
		}

		pos = i;
		for (i=num_files ; i>pos ; i--)
			filelist[i] = filelist[i-1];
	}
	else pos = num_files;

    filelist[pos].name = namecopy;
	filelist[pos].type = ftype;
	filelist[pos].size = fsize;

	num_files++;
	return pos;
}

/*
static void AddNewEntry_unsorted (const char *fname, int ftype, long fsize)
{
	Cmd_AddFilelistEntry (fname, ftype, fsize, AFE_NO_NAMESORT | AFE_NO_TYPESORT);
}
*/

void Cmd_ClearFilelist (void)
{
	int i;

	if (filelist)
	{
		// JDH: fixed leak:
		for (i=0 ; i<num_files ; i++)
		{
			if (filelist[i].name)
				free (filelist[i].name);		// allocated via strdup
		}

		free (filelist);
		filelist = NULL;
		num_files = 0;
	}
}

qboolean Cmd_CheckEntryName (const char *fname)
{
	int	i;

	for (i=0 ; i<num_files ; i++)
		if (COM_FilenamesEqual (fname, filelist[i].name))
			return true;

	return false;
}

static void PrintEntries (int start, int end)
{
	int	i;

//	int filectr;
//	filectr = pak_files ? (num_files - pak_files) : 0;

	if (start < 0)
		start = 0;
	if (end > num_files)
		end = num_files;
	
	for (i=start ; i<end ; i++)
	{
		/*if (!filectr-- && pak_files)
		{
			if (con_x)
				Con_Print ("\n");

			Con_Print ("\x02" "inside pack file:\n");
		}*/

		PaddedPrint (filelist[i].name);
	}

	if (con_x)
		Con_Print ("\n");
}

/*
#define SLASHJMP(x, y)			\
	if (!(x = strrchr(y, '/')))	\
		x = y;			\
	else				\
		x++		// JDH: was *++x
*/
/*
=================
FindFilesInPak

Search for files inside a PAK file		-- by joe
=================
*/
/*
int	pak_files = 0;
void FindFilesInPak (char *the_arg)
{
	int		i;
	searchpath_t	*search;
	pack_t		*pak;
	char		*myarg;

	SLASHJMP(myarg, the_arg);		// set myarg to point to filename (excluding path)
	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (search->pack)
		{
			char	*s, *p, ext[8], filename[MAX_FILELENGTH];

			// look through all the pak file elements
			pak = search->pack;
			for (i=0 ; i<pak->numfiles ; i++)
			{
				s = pak->files[i].name;
				Q_strcpy (ext, COM_FileExtension(s), sizeof(ext));
				if (COM_FilenamesEqual(ext, COM_FileExtension(myarg)))
				{
					SLASHJMP(p, s);
					if (COM_FilenamesEqual(ext, "bsp") && !COM_IsRealBSP(p))
						continue;
					if (!Q_strncasecmp(s, the_arg, strlen(the_arg)-5) ||
					    (*myarg == '*' && !Q_strncasecmp(s, the_arg, strlen(the_arg)-5-compl_len)))
					{
						COM_StripExtension (p, filename);
						if (Cmd_CheckEntryName(filename))
							continue;
						AddNewEntry_unsorted (filename, 0, pak->files[i].filelen);
						pak_files++;
					}
				}
			}
		}
	}
}
*/

/*
#define	READDIR_ALL_PATH(p)							\
	for (search = com_searchpaths ; search ; search = search->next)		\
	{									\
		if (!search->pack)						\
		{								\
			RDFlags |= (RD_STRIPEXT | RD_NOERASE);			\
			if (skybox)						\
				RDFlags |= RD_SKYBOX;				\
			ReadDir (va("%s/%s", search->filename, subdir), p);	\
		}								\
	}
*/


#if 0
// JDH: replaced ReadDir with COM_FindXXXXX functions
/*
=================
ReadDir			-- by joe
=================
*/
void ReadDir (char *path, char *the_arg)
{
#ifdef _WIN32
	HANDLE		h;
	WIN32_FIND_DATAA	fd;
#else
	int		h, i = 0;
	glob_t		fd;
	char		*p;
	struct	stat	fileinfo;
#endif

	if (path[strlen(path)-1] == '/')
		path[strlen(path)-1] = 0;

	if (!(RDFlags & RD_NOERASE))
		Cmd_ClearFilelist ();

#ifdef _WIN32
	h = FindFirstFileA (va("%s/%s", path, the_arg), &fd);
	if (h == INVALID_HANDLE_VALUE)
#else
	h = glob (va("%s/%s", path, the_arg), 0, NULL, &fd);
	if (h == GLOB_ABORTED)
#endif
	{
		/*if (RDFlags & RD_MENU_DEMOS)
		{
			Cmd_AddFilelistEntry ("Error reading directory", 3, 0);
			num_files = 1;
		}
		else*/ if (RDFlags & RD_COMPLAIN)
		{
			Con_Print ("No such file\n");
		}
		goto end;
	}

	/*if (RDFlags & RD_MENU_DEMOS && !(RDFlags & RD_MENU_DEMOS_MAIN))
	{
		Cmd_AddFilelistEntry ("..", 2, 0);
		num_files = 1;
	}*/

	do {
		int	fdtype;
		long	fdsize;
		char	filename[MAX_FILELENGTH];

#ifdef _WIN32
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!(RDFlags & (RD_MENU_DEMOS | RD_GAMEDIR)) || !strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
				continue;

			fdtype = 1;
			fdsize = 0;
			Q_strcpy (filename, fd.cFileName, sizeof(filename));
		}
		else
		{
			char	ext[8];

			if (RDFlags & RD_GAMEDIR)
				continue;

			Q_strcpy (ext, COM_FileExtension(fd.cFileName), sizeof(ext));

			/*if ((RDFlags & RD_MENU_DEMOS) && Q_strcasecmp(ext, "dem", -1) && Q_strcasecmp(ext, "dz", -1))
				continue;*/

			fdtype = 0;
			fdsize = fd.nFileSizeLow;
			if (Q_strcasecmp(ext, "dz") && (RDFlags & (RD_STRIPEXT | RD_MENU_DEMOS)))
			{
				COM_StripExtension (fd.cFileName, filename, sizeof(filename));
				/*if (RDFlags & RD_SKYBOX)
				{
					int	idx = strlen(filename)-3;

					filename[filename[idx] == '_' ? idx : idx+1] = 0;	// cut off skybox_ext
				}*/
			}
			else
			{
				Q_strcpy (filename, fd.cFileName, sizeof(filename));
			}

			if (Cmd_CheckEntryName (filename))
				continue;	// file already on list
		}
#else
		if (h == GLOB_NOMATCH || !fd.gl_pathc)
			break;

		SLASHJMP(p, fd.gl_pathv[i]);
		stat (fd.gl_pathv[i], &fileinfo);

		if (S_ISDIR(fileinfo.st_mode))
		{
			if (!(RDFlags & (RD_MENU_DEMOS | RD_GAMEDIR)))
				continue;

			fdtype = 1;
			fdsize = 0;
			Q_strcpy (filename, p, sizeof(filename));
		}
		else
		{
			char	ext[8];

			if (RDFlags & RD_GAMEDIR)
				continue;

			Q_strcpy (ext, COM_FileExtension(p), sizeof(ext));

			/*if ((RDFlags & RD_MENU_DEMOS) && Q_strcasecmp(ext, "dem") && Q_strcasecmp(ext, "dz"))
				continue;*/

			fdtype = 0;
			fdsize = fileinfo.st_size;
			if (Q_strcasecmp(ext, "dz") && (RDFlags & (RD_STRIPEXT | RD_MENU_DEMOS)))
			{
				COM_StripExtension (p, filename, sizeof(filename));
				/*if (RDFlags & RD_SKYBOX)
				{
					int	idx = strlen(filename)-3;

					filename[filename[idx] == '_' ? idx : idx+1] = 0;	// cut off skybox_ext
				}*/
			}
			else
			{
				Q_strcpy (filename, p, sizeof(filename));
			}

			if (Cmd_CheckEntryName(filename))
				continue;	// file already on list
		}
#endif
		Cmd_AddFilelistEntry (filename, fdtype, fdsize, 0);
	}
#ifdef _WIN32
	while (FindNextFileA(h, &fd));
	FindClose (h);
#else
	while (++i < fd.gl_pathc);
	globfree (&fd);
#endif

	if (!num_files)
	{
		/*if (RDFlags & RD_MENU_DEMOS)
		{
			Cmd_AddFilelistEntry ("[ no files ]", 3, 0);
			num_files = 1;
		}
		else*/ if (RDFlags & RD_COMPLAIN)
		{
			Con_Print ("No such file\n");
		}
	}

end:
	RDFlags = 0;
}
#endif

/*
============
Cmd_PrintFileList
============
*/
void Cmd_PrintFileList (int colwidth)
{
	int i;
	extern int con_linewidth;

	colwidth = bound(28, colwidth, con_linewidth-8);

	for (i=0 ; i<num_files ; i++)
	{
		Con_Printf ("%-*s  ", colwidth, filelist[i].name);
		Con_Printf ("\x02%5i", filelist[i].size >> 10);
		Con_Print ("k\n");
	}

	Con_Print ("\n");
	Cmd_ClearFilelist();
}

/*
=============================================================================

				COMMAND-LINE COMPLETION

=============================================================================
*/

#ifndef RQM_SV_ONLY

static char  **completion_list = NULL;
static int     completion_count = 0;

/*
============
Cmd_FindCommonStart
   uses global filelist
============
*/
const char * Cmd_FindCommonStart (void)
{
	static char compl_common[MAX_QPATH];
	int	i, j;

	if (num_files)
	{
		Q_strcpy (compl_common, filelist[0].name, sizeof(compl_common));

		for (i=1 ; i<num_files ; i++)
		{
			// note: original code used Q_strncasecmp
			for (j = 0; compl_common[j]; j++)
			{
				if (compl_common[j] != filelist[i].name[j])
				{
					compl_common[j] = 0;
					break;
				}
			}
		}
	}
	else
		compl_common[0] = 0;

	return compl_common;
}

/*
============
Cmd_AllocCompletionList
============
*/
void Cmd_AllocCompletionList (int count)
{
	int i;

	if (completion_list)
	{
		// free the strings
		for (i = 0; i < completion_count; i++)
			free (completion_list[i]);

		// and the list itself
		free (completion_list);
	}

	completion_count = count;
	if (count)
	{
		completion_list = Q_malloc (count * sizeof(char *));

		for (i = 0; i < count; i++)
			completion_list[i] = NULL;
	}
	else completion_list = NULL;
}

/*
============
Cmd_CompleteCommand
============
*/
/*const char *Cmd_CompleteCommand (const char *partial)
{
	cmd_function_t	*cmd;
	legacycmd_t	*lcmd;
	int		len;

	if (!(len = strlen(partial)))
		return NULL;

// check functions
	for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
		if (!Q_strncasecmp(partial, cmd->name, len))
			return cmd->name;

	for (lcmd = legacycmds ; lcmd ; lcmd = lcmd->next)
		if (!Q_strncasecmp(partial, lcmd->oldname, len))
			return lcmd->oldname;

	return NULL;
}
*/
/*
============
Cmd_CompleteCountPossible
============
*/
int Cmd_CompleteCountPossible (const char *partial)
{
	cmd_function_t	*cmd;
	legacycmd_t	*lcmd;
	int		len, c = 0;

	if (!(len = strlen(partial)))
		return 0;

	for (cmd = cmd_functions ; cmd ; cmd = cmd->next)
	{
		if (!(cmd->flags & CMD_DISABLED) && !Q_strncasecmp(partial, cmd->name, len))
			c++;
	}

	for (lcmd = legacycmds ; lcmd ; lcmd = lcmd->next)
		if (!Q_strncasecmp(partial, lcmd->oldname, len))
			c++;

	return c;
}

/*
============
Cmd_FindPRFunctions (JDH)
- populates the global filelist with matching functions from progs.dat
============
*/
void Cmd_FindPRFunctions (const char *partial)
{
	dfunction_t	*func;
	int		i;

	if (!sv.active)
		return;

	for (i = 0; ; i++)
	{
		func = PR_FindNextFunction (&i, partial, PRFF_IGNORECASE | PRFF_NOBUILTINS | PRFF_NOPARAMS);
		if (!func)
			break;
		
		Cmd_AddFilelistEntry (func->s_name, 0, 0, AFE_KEEPCASE);
	}
}

/*
============
Cmd_FindTexmodes (JDH)
- populates the global filelist with matching texture filters from gl_texmodes
============
*/
void Cmd_FindTexmodes (const char *partial)
{
	int len, i;
	
	len = strlen (partial);
	for (i = 0; i < GL_NUM_TEXMODES; i++)
	{
		if (!Q_strncasecmp (partial, gl_texmodes[i].name, len))
			Cmd_AddFilelistEntry (gl_texmodes[i].name, 0, 0, AFE_KEEPCASE);
	}
}

/*
============
Cmd_CompleteCvarParam (JDH)
- for commands that take a cvar as a parameter (eg. toggle)
============
*/
void Cmd_CompleteCvarParam (const char *partial)
{
	int		len;
	cvar_t *var;
/*
	count = Cvar_CompleteCountPossible (partial);
	Cmd_AllocCompletionList (count);

	if (!count) return;
*/
	len = strlen (partial);
//	count = 0;

	// find matches
	for (var = cvar_vars ; var ; var = var->next)
	{
		if (!Q_strncasecmp(partial, var->name, len))
		{
			//completion_list[count++] = Q_strdup (var->name);
			Cmd_AddFilelistEntry (var->name, 0, 0, AFE_NO_NAMESORT | AFE_KEEPCASE);
				// NO_NAMESORT since they're already alphabetized
		}
	}
/*
	if (completion_count > 1)
	{
		Con_Printf ("]%s\n", partial);

		for (i=0 ; i<completion_count ; i++)
		{
			PaddedPrint (completion_list[i]);
		}

		if (con_x)
			Con_Print ("\n");
	}
*/
}
/*
extern	char	key_lines[64][MAXCMDLINE];
extern	int	edit_line;
extern	int	key_linepos;
*/
/*
============
Cmd_FillInParam
============
*/
qboolean Cmd_FillInParam (char *buf, int bufsize, const char *filename, int num_matches)
{
	qboolean	has_space = false;
	int			i, len;

	for (i = 0; filename[i]; i++)
	{
		if (filename[i] == ' ')
			has_space = true;
	}

	if (has_space)
		i += 2;			// quotes
	if (num_matches == 1)
		i++;			// space at end
	
	if (i >= bufsize)
		return false;
	
	if (has_space)
		*buf++ = '"';		// put quotes around filenames with spaces

	len = Q_strcpy (buf, filename, Q_MAXINT);		// already checked bufsize
	if (has_space)
		buf[len++] = '"';
	if (num_matches == 1)
		buf[len++] = ' ';

	buf[len] = 0;
	return true;
}

/*
============
Cmd_AddFoundFile
- callback for COM_FindAllFiles/COM_FindDirFiles in Cmd_CompleteParameter
============
*/
int Cmd_AddFoundFile (com_fileinfo_t *fileinfo, int count, unsigned skybox)
{
	char	filename[MAX_QPATH];

	COM_StripExtension (fileinfo->name, filename, sizeof(filename));

	if (skybox)
	{
		if (!Sky_GetBaseName (filename))
			return 0;
	}
	else if (!COM_IsRealBSP (fileinfo->name))
		return 0;

	if (!Cmd_CheckEntryName (filename))
	{
		Cmd_AddFilelistEntry (filename, CMD_FTYPE_FILE, fileinfo->filelen, 0);
	}

	return 0;		// continue searching
}

/*
============
Cmd_AddDemoArg
- callback for COM_FindDirFiles in Cmd_CompleteParameter
  (used when param for playdemo cmd starts with "../")
============
*/
int Cmd_AddDemoArg (com_fileinfo_t *fileinfo, int count, unsigned param)
{
	char	*p, filename[MAX_QPATH];
	int		len;

// find the path prefix:
	p = strrchr ((char *)param, '/');
	len = p - (char *)param + 1;

	Q_strncpy (filename, sizeof(filename), (char *) param, len);
//	Q_snprintfz (filename+len, sizeof(filename)-len, "%s%c", fileinfo->name, (fileinfo->isdir ? '/' : 0));
	Q_strcpy (filename+len, fileinfo->name, sizeof(filename)-len);

	if (!Cmd_CheckEntryName (filename))
	{
		// note: ftype arg is intentionally reversed, so demos appear *before* folders
		Cmd_AddFilelistEntry (filename, (fileinfo->isdir ? CMD_FTYPE_FILE : CMD_FTYPE_DIR), fileinfo->filelen, 0);
	}

	return 0;		// continue searching
}

/*
============
Cmd_AddDZArg
- callback for COM_FindAllFiles in Cmd_CompleteParameter
  (used to filter dzips/zips that are already mounted by dzlib)
============
*/
int Cmd_AddDZArg (com_fileinfo_t *fileinfo, int count, unsigned param)
{
	if (COM_DzipIsMounted(fileinfo->searchpath->dir_name, fileinfo->name))
		return 0;

	if (!Cmd_CheckEntryName (fileinfo->name))
	{
		Cmd_AddFilelistEntry (fileinfo->name, CMD_FTYPE_FILE, fileinfo->filelen, 0);
	}

	return 0;		// continue searching
}

/*
============
Cmd_GetLastParameter (JDH)
  takes a line from the console, and returns a pointer to the last argument
============
*/
/*char * Cmd_GetLastParameter (const char *line, int *argc_out)
{
	const char *param, *s;
	
	param = line;
	*argc_out = 1;
	while (1)
	{
		s = COM_Parse (param);
		if (!s || !*s)
			break;
		while (*s == ' ')
			s++;
		param = s;
		(*argc_out)++;
	}
	return (char *)param;
}
*/

typedef enum 
{
	ARG_BSP, ARG_DEM, ARG_SAV, ARG_SUBDIR, ARG_IMG, ARG_IMG_SKY, ARG_IMG_FONT, 
		ARG_IMG_CROSSHAIR, ARG_TXT, ARG_CFG, ARG_CVAR, ARG_FUNC, ARG_TEXMODE
} argtype_t;

/*
============
Cmd_FindParameters	-- by joe

parameter completion for various commands
JDH: added cycling through matches
============
*/
//void Cmd_CompleteParameter (char *partial, const char *attachment, qboolean reverse_cycle)
void Cmd_FindParameters (argtype_t argtype, const char *param/*, const char *cmdline*/)
{
//	char		*s, stay[MAXCMDLINE];
	char		path[MAX_OSPATH], qpath[MAX_QPATH];
	char		*subdir1 = NULL, *subdir2 = NULL;
	const char	*s, *pathlist[3];
	qboolean	/*full_fillin,*/ skybox = false;
	int			/*file_index,*/ flags;

//	full_fillin = ((int)cl_advancedcompletion.value == 1);
/*
	if (!full_fillin && !*param)		// no parameter was written in, so quit
	{
		last_fillin[0] = 0;
		return;
	}
	if (*param == '"')
		param++;

	len = strlen (param);

	if (full_fillin)
	{
	// JDH: if command line is the same as how we last completed it,
	//      cycle through existing file list
		if ((completion_count > 1) && !strcmp (stay, last_fillin))
		{
			if (param[len-1] == '"')
				param[len-1] = 0;

			for (file_index = 0; file_index < completion_count; file_index++)
			{
				if (!strcmp (completion_list[file_index], param))
				{
					if (reverse_cycle)
					{
						if (file_index == 0)
							file_index = completion_count - 1;
						else
							file_index--;
					}
					else
						file_index = (file_index+1) % completion_count;		// next in list
					goto EXITPOINT;
				}
			}
		}
	}
*/
//	strcat (param, attachment);
//	Q_strcpy (param+len, attachment, sizeof(stay)-(param-stay)-len);

	Cmd_ClearFilelist ();

	if (argtype == ARG_SUBDIR)
	{
	// these commands work for folders in com_basedir only, so COM_FindAllFiles can't be used
		Q_snprintfz (path, sizeof(path), "%s/%s", com_basedir, param);
		COM_FindDirFiles (path, NULL, FILE_DIRS_ONLY, Cmd_AddFoundFile, false);
		goto PRINTFILES;
	}
	else if (argtype == ARG_SAV)
	{
	// these commands work for files in com_gamedir only, so COM_FindAllFiles can't be used
		Q_snprintfz (path, sizeof(path), "%s/%s", com_gamedir, param);
		COM_FindDirFiles (path, NULL, 0, Cmd_AddFoundFile, false);
		goto PRINTFILES;
	}
	else if (argtype == ARG_CVAR)		// JDH
	{
		Cmd_CompleteCvarParam (param);
		goto PRINTFILES;
/*		if (!completion_count) 
			return NULL;
//		file_index = 0;
//		goto EXITPOINT;		// since completion_list is already filled in
		return completion_list[0];
*/
	}
	else if (argtype == ARG_FUNC)
	{
		Cmd_FindPRFunctions (param);
		goto PRINTFILES;
	}
	else if (argtype == ARG_TEXMODE)
	{
		Cmd_FindTexmodes (param);
		goto PRINTFILES;
	}
	

	flags = 0;
	
	if (argtype == ARG_BSP)
	{
		subdir1 = "maps/";
	}
	else if ((argtype >= ARG_IMG) && (argtype <= ARG_IMG_CROSSHAIR))
	{
		if (argtype == ARG_IMG_SKY)
		{
			subdir1 = "env/";
			subdir2 = "gfx/env/";
			skybox = true;
		}
		else if (argtype == ARG_IMG_FONT)
		{
			subdir1 = "textures/charsets/";
		}
		else if (argtype == ARG_IMG_CROSSHAIR)
		{
			subdir1 = "crosshairs/";
			subdir2 = "textures/crosshairs/";
		}

		flags = FILE_ANY_IMG;
	}
	else if (argtype == ARG_DEM)
	{
		int i;

		for (i = 0; param[i] && (i < sizeof(qpath)-1); i++)
		{
			qpath[i] = (param[i] == '\\') ? '/' : param[i];
		}
		qpath[i] = 0;
		
		if (/*!Q_strncasecmp(stay, "playdemo ", 9) &&*/ !strncmp(qpath, "../", 3))
		{
			//param[2] = '/';

			Q_snprintfz (path, sizeof(path), "%s/%s", com_basedir, qpath+3);
			COM_FindDirFiles (path, NULL, FILE_ANY_DEMO, Cmd_AddDemoArg, (unsigned)qpath);

			if (dzlib_loaded)
			{
				// if dzlib is loaded, COM_FindDirFiles won't find dzips or zips
				COM_ForceExtension (path, ".dz", sizeof(path));
				COM_FindDirFiles (path, NULL, 0, Cmd_AddDemoArg, (unsigned)qpath);

				COM_ForceExtension (path, ".zip", sizeof(path));
				COM_FindDirFiles (path, NULL, 0, Cmd_AddDemoArg, (unsigned)qpath);
			}

			s = strchr (qpath+3, '/');
			if (!s)
			{
				// if there's no path separator after the initial "../",
				//  then offer a list of gamedirs as well as demos
				qpath[strlen(qpath)-4] = 0;		// remove the ".dem" we added (leave the * though)
				Q_snprintfz (path, sizeof(path), "%s/%s", com_basedir, qpath+3);
				COM_FindDirFiles (path, NULL, FILE_DIRS_ONLY, Cmd_AddDemoArg, (unsigned)qpath);
			}
			goto PRINTFILES;
		}

		flags = FILE_ANY_DEMO;
	}

	pathlist[0] = subdir1;
	pathlist[1] = subdir2;
	pathlist[2] = NULL;

	COM_FindAllFiles (pathlist, param, flags, Cmd_AddFoundFile, skybox);

	// check for any dzips/zips that aren't mounted (added after game init)
	if ((flags & FILE_ANY_DEMO) && dzlib_loaded)
	{
		COM_ForceExtension (qpath, ".dz", sizeof(qpath));
		COM_FindAllFiles (pathlist, qpath, 0, Cmd_AddDZArg, 0);

		COM_ForceExtension (qpath, ".zip", sizeof(qpath));
		COM_FindAllFiles (pathlist, qpath, 0, Cmd_AddDZArg, 0);
	}

PRINTFILES:
	if ((num_files > 1) && cl_advancedcompletion.value)
	{
	//	Con_Printf ("]%s\n", cmdline);
		Con_Print ("\n");
		PrintEntries (0, num_files);
	}
	
/*
LISTFILLED:
	if (!filelist)
	{
		// should I be freeing completion_list here?

		last_fillin[0] = 0;
		return NULL;
	}

	if (num_files > 1)
	{
		if (cl_advancedcompletion.value)
		{
			Con_Printf ("]%s\n", partial);
			PrintEntries ();

			if (!full_fillin)
			{
				CompareParams (0);		// stores shared prefix to compl_common
				Cmd_ClearFilelist ();
				return compl_common;
			}
		}
	}

	Cmd_AllocCompletionList (num_files);

	for (file_index = 0; file_index < num_files; file_index++)
	{
		completion_list[file_index] = filelist[file_index].name;
		filelist[file_index].name = NULL;
	}

	Cmd_ClearFilelist ();
	return completion_list[0];
*/
}

/*
==================
Cmd_CompleteCmdOrCvar

Advanced command completion

Main body and many features imported from ZQuake	-- joe
==================
*/
#if 1
void Cmd_FindCmdsAndCvars (const char *partial)
{
	int				len, num_cmds;
	cmd_function_t	*cmdf;
	legacycmd_t		*lcmd;
	cvar_t			*var;

	Cmd_ClearFilelist ();

	if (!(len = strlen(partial)))
		return;

// check commands
	for (cmdf = cmd_functions ; cmdf ; cmdf = cmdf->next)
	{
		if (!(cmdf->flags & CMD_DISABLED) && !Q_strncasecmp(partial, cmdf->name, len))
		{
			Cmd_AddFilelistEntry (cmdf->name, 0, 0, AFE_NO_NAMESORT | AFE_KEEPCASE);
				// NO_NAMESORT since they're already alphabetized
		}
	}

// joe: check for legacy commands also
	for (lcmd = legacycmds ; lcmd ; lcmd = lcmd->next)
	{
		if (!Q_strncasecmp(partial, lcmd->oldname, len))
		{
			Cmd_AddFilelistEntry (lcmd->oldname, 0, 0, AFE_KEEPCASE);
		}
	}

	num_cmds = num_files;

// check variables
	for (var = cvar_vars ; var ; var = var->next)
	{
		if (!Q_strncasecmp(partial, var->name, len))
		{
			Cmd_AddFilelistEntry (var->name, 0, 0, AFE_NO_NAMESORT | AFE_KEEPCASE);
		}
	}

	if ((num_files > 1) && cl_advancedcompletion.value)
	{
		Con_Print ("\n");

		if (num_cmds)
		{
			Con_Print ("\x02""commands:\n");
			
			PrintEntries (0, num_cmds);
		}

		if (num_files - num_cmds)
		{
			Con_Print ("\x02""variables:\n");
			
			PrintEntries (num_cmds, num_files);
		}
	}
}
#else
const char * Cmd_CompleteCmdOrCvar (const char *partial, qboolean reverse_cycle, int *num_matches)
{
	qboolean	full_fillin;
	int			c, v, index, len;
	const char	*cmd;

	if (!(len = strlen(partial)))
		return NULL;

	full_fillin = ((int)cl_advancedcompletion.value == 1);
/*
	if (full_fillin)
	{
		// JDH: if command line is the same as how we last completed it,
		//      cycle through existing command/cvar list
		if (!strcmp (partial, last_fillin))
		{
			if (completion_count <= 1)
				return NULL;

			for (index = 0; index < completion_count; index++)
			{
				if (!Q_strcasecmp (completion_list[index], partial))
				{
					if (reverse_cycle)
					{
						if (index == 0)
							index = completion_count-1;
						else
							index--;
					}
					else
						index = (index+1) % completion_count;		// next in list
					
					c = v = 0;		// doesn't matter; all that's checked is if c+v==1
					cmd = completion_list[index];
					goto EXITPOINT;
				}
			}
		}
	}
	else*/
		compl_clen = 0;

	c = Cmd_CompleteCountPossible (partial);
	v = Cvar_CompleteCountPossible (partial);

	if (c+v == 0)
	{
		// should I be freeing completion_list here?

		last_fillin[0] = 0;
		return NULL;
	}

	if (c + v == 1)
	{
		if (!(cmd = Cmd_CompleteCommand(partial)))
			cmd = Cvar_CompleteVariable (partial);
		goto EXITPOINT;
	}
	
	if (full_fillin)
	{
		Cmd_AllocCompletionList (c+v);
		index = 0;
	}


	Con_Print ("\n");

	if (c)
	{
		cmd_function_t	*cmdf;
		legacycmd_t	*lcmd;

		Con_Print ("\x02""commands:\n");

		// check commands
		for (cmdf = cmd_functions ; cmdf ; cmdf = cmdf->next)
		{
			if (!(cmdf->flags & CMD_DISABLED) && !Q_strncasecmp(partial, cmdf->name, len))
			{
				PaddedPrint (cmdf->name);

				if (full_fillin)
					completion_list[index++] = Q_strdup (cmdf->name);
				else
					FindCommonSubString (cmdf->name, len);
			}
		}

		// joe: check for legacy commands also
		for (lcmd = legacycmds ; lcmd ; lcmd = lcmd->next)
		{
			if (!Q_strncasecmp(partial, lcmd->oldname, len))
			{
				PaddedPrint (lcmd->oldname);

				if (full_fillin)
					completion_list[index++] = Q_strdup (lcmd->oldname);
				else
					FindCommonSubString (lcmd->oldname, len);
			}
		}

		if (con_x)
			Con_Print ("\n");
	}

	if (v)
	{
		cvar_t		*var;

		Con_Print ("\x02""variables:\n");

		// check variables
		for (var = cvar_vars ; var ; var = var->next)
		{
			if (!Q_strncasecmp(partial, var->name, len))
			{
				PaddedPrint (var->name);

				if (full_fillin)
					completion_list[index++] = Q_strdup (var->name);
				else
					FindCommonSubString (var->name, len);
			}
		}

		if (con_x)
			Con_Print ("\n");
	}

	if (full_fillin)
	{
		cmd = completion_list[0];
	}
	else if (compl_clen)
	{
		compl_common[compl_clen] = 0;
		cmd = compl_common;
	}
	else return NULL;

EXITPOINT:
	*num_matches = c + v;
	return cmd;

#if 0	
	Q_strcpy (key_lines[edit_line]+1, cmd, sizeof(key_lines[edit_line])-1);
	key_linepos = strlen(cmd) + 1;
	if (/*!full_fillin &&*/ (c + v == 1))
		key_lines[edit_line][key_linepos++] = ' ';
	key_lines[edit_line][key_linepos] = 0;
	Q_strcpy (last_fillin, key_lines[edit_line]+1, sizeof(last_fillin));
#endif
}
#endif

const char * Cmd_CycleCompletion (const char *curr, qboolean cycle_rev)
{
	int i;
	
	for (i = 0; i < completion_count; i++)
	{
		if (!strcmp (completion_list[i], curr))
		{
			if (cycle_rev)
			{
				if (i == 0)
					i = completion_count - 1;
				else
					i--;
			}
			else
				i = (i+1) % completion_count;		// next in list
			return completion_list[i];
		}
	}
	
	return NULL;
}

int Cmd_AdjustArgc (cmd_arglist_t *args)
{
	int argc, len;
	const char *p;
	
// check if there is anything (whitespace) following the last arg.
// If so, it's the next arg that we want to complete.
	argc = args->argc;
	len = strlen (args->argv[argc-1]);
	
	p = args->cmdline + args->args_ofs[argc-1];
	if (*p == '"')
		p++;
	
	p += len;
	if (*p)
	{
		argc++;
		args->argc++;
		args->argv[argc-1] = args->argv[argc-2] + len;		// terminating \0 of previous arg
		while (*p++);		// find end of cmdline
		args->args_ofs[argc-1] = p - args->cmdline - 1;
	}

	return argc;
}

const char * Cmd_StoreCompletions (/*const char *cmdline,*/ int *num_matches)
{
	int i;
	
	if (cl_advancedcompletion.value)
		*num_matches = num_files;		// list all matches, fill in first/next match
	else
		*num_matches = 1;			// just fill in first match

	if ((num_files > 1) && ((unsigned int)cl_advancedcompletion.value > 1))
	{
		const char *str = Cmd_FindCommonStart ();
		Cmd_ClearFilelist ();
		return str;
	}

	Cmd_AllocCompletionList (num_files);

	for (i = 0; i < num_files; i++)
	{
		completion_list[i] = filelist[i].name;
		filelist[i].name = NULL;
	}

	Cmd_ClearFilelist ();
	return completion_list[0];
}

#define MAX_ARGCMDS 4

typedef struct
{
	argtype_t argtype;
	const char *pattern;
	const char *cmds[MAX_ARGCMDS];
} cmdarg_t;

extern cvar_t gl_texturemode;

cmdarg_t cmd_argtypes[] = 
{
	{ARG_BSP,           "*.bsp", {"record", "map", "changelevel", "capture_start"}},
	{ARG_BSP,           "*.bsp", {"record"}},
	{ARG_DEM,           "*.dem", {"playdemo", "timedemo", "capturedemo"}},
	{ARG_SAV,           "*.sav", {"load"}},
	{ARG_SUBDIR,          "*",   {"gamedir", "game"}},
	{ARG_IMG_SKY,       "*.tga", {"loadsky", "r_skybox"}},
	{ARG_IMG_FONT,      "*.tga", {"loadcharset", "gl_consolefont"}},
	{ARG_IMG_CROSSHAIR, "*.tga", {"crosshairimage"}},
	{ARG_TXT,           "*.txt", {"printtxt"}},
	{ARG_CFG,           "*.cfg", {"exec"}},
	{ARG_CVAR,            "",    {"toggle", "cycle", "inc", "dec"}},
	{ARG_FUNC,            "",    {"create"}},
	{ARG_TEXMODE,         "",    {"gl_texturemode"}}
};

#define NUM_CMDARGS (sizeof(cmd_argtypes)/sizeof(cmd_argtypes[0]))

const cmdarg_t * Cmd_GetArgtype (const char *cmd)
{
	int i, j;
	const char *testcmd;

	for (i = 0; i < NUM_CMDARGS; i++)
	{		
		for (j = 0; j < MAX_ARGCMDS; j++)
		{
			testcmd = cmd_argtypes[i].cmds[j];
			if (!testcmd)
				break;

			if (!Q_strcasecmp(testcmd, cmd))
				return &cmd_argtypes[i];
		}
	}

	return NULL;
}

/*
==================
Cmd_GetFinalCommand (JDH)
==================
*/
const char * Cmd_GetFinalCommand (const char *cmdline)
{
// JDH: check if the line contains >1 command, separated by semicolons.
//      If so, we want to complete the _last_ command.

	int cmdstart = 0, quotes = 0;
	const char *p;
	
	for ( p = cmdline; *p ; p++)
	{
		if (*p == '"')
			quotes++;
		if (!(quotes & 1) && (*p == ';'))	// don't break if inside a quoted string
			cmdstart = p - cmdline + 1;
	}
		
	return cmdline + cmdstart;
}

static char last_fillin[MAXCMDLINELEN] = "";
/*
==================
Cmd_TabComplete (JDH)
==================
*/
const char * Cmd_TabComplete (const char *cmdline, qboolean cycle_rev)
{
	const char		*currcmd, *result = NULL;
	cmd_arglist_t	args;
	int				i, argc, maxargs, num_matches = 0;
	
	currcmd = Cmd_GetFinalCommand (cmdline);
	
	Cmd_TokenizeString (currcmd, &args);
	if (args.argc < 1)
		return NULL;

	if ((int)cl_advancedcompletion.value == 1)
	{
	// JDH: if command line is the same as how we last completed it,
	//      cycle through existing file list
		if ((completion_count > 1) && !strcmp (cmdline, last_fillin))
		{
			//if (param[len-1] == '"')
			//	param[len-1] = 0;

			result = Cmd_CycleCompletion (args.argv[args.argc-1], cycle_rev);
			num_matches = completion_count;
			goto FILLIN;
		}
	}
	
	argc = Cmd_AdjustArgc (&args);
	
// for parameter completion, we need argc > 1
	if (argc > 1)
	{
		const cmdarg_t *arg_info;
		char param[MAX_QPATH];
		
		arg_info = Cmd_GetArgtype (args.argv[0]);
		if (!arg_info)
			return NULL;		// command doesn't support tab-completion of args
			
		if (arg_info->argtype == ARG_SUBDIR)
			maxargs = 10;		// game/gamedir cmd: no hard limit, so this is arbitrary
		else
			maxargs = (!Q_strcasecmp(args.argv[0], "record") ? 3 : 2);
			// for record command, the map & demo name args get completed
		if (argc > maxargs)
			return NULL;

		Q_snprintfz (param, sizeof(param), "%s%s", args.argv[args.argc-1], arg_info->pattern);
		
		Cmd_FindParameters (arg_info->argtype, param/*, cmdline*/);
	//	if (argc == maxargs)
	//		num_matches = 999;		// so space is not appended
	}
	else
	{
		Cmd_FindCmdsAndCvars (args.argv[0]);
		maxargs = 2;		// including command/cvar
	}
/*		
	// command completion
	if (cl_advancedcompletion.value)		// list all matches, fill in first/next match
	{
		result = Cmd_FindCmdsAndCvars (args.argv[0], cycle_rev, &num_matches);
	}
	else		// just fill in first match
	{
		if (!(result = Cmd_CompleteCommand(args.argv[0])))
			result = Cvar_CompleteVariable (args.argv[0]);

		num_matches = 1;
	}
*/

	if (!filelist)
	{
		// should I be freeing completion_list here?

		last_fillin[0] = 0;
		return NULL;
	}

	result = Cmd_StoreCompletions (/*cmdline,*/ &num_matches);
	
	if ((num_matches == 1) && (argc == maxargs))
		num_matches = 999;		// so space is not appended

FILLIN:
	if (!result)
		return NULL;
	
	Q_strcpy (last_fillin, cmdline, sizeof(last_fillin));
//	i = args.args_ofs[args.argc-1];		// position of last arg in command line
	i = currcmd - cmdline + args.args_ofs[args.argc-1];		// position of last arg in command line
	
	Cmd_FillInParam (last_fillin + i, sizeof(last_fillin) - i, result, num_matches);
/*	
	if (num_matches == 1)
		Q_snprintfz (last_fillin + i, sizeof(last_fillin) - i, "%s ", result);
	else
		Q_strcpy (last_fillin + i, result, sizeof(last_fillin) - i);
*/	
	return last_fillin;
}

#endif		//#ifndef RQM_SV_ONLY

/*
=============================================================================

				LIST COMMANDS

=============================================================================
*/

/*
====================
Cmd_CmdList_f

List all console commands
 JDH: partial listing option from Fitz
====================
*/
void Cmd_CmdList_f (cmd_source_t src)
{
	cmd_function_t	*curr_cmd;
	int		len, counter;
	char	*partial/*, buf[256]*/;

	if (src == SRC_CLIENT)
		return;

	if (cmd_arglist.argc > 1)
	{
		partial = cmd_arglist.argv[1];
		len = strlen (partial);
	}
	else
	{
		partial = NULL;
		len = 0;
	}

	Con_PagedOutput_Begin ();

	counter = 0;
	for (curr_cmd = cmd_functions; curr_cmd; curr_cmd = curr_cmd->next)
	{
		if (curr_cmd->flags & CMD_DISABLED)
			continue;
		if (partial && Q_strncasecmp (partial, curr_cmd->name, len))
			continue;

		if (!Con_Printf ("  %s\n", curr_cmd->name))
		{
			Con_PagedOutput_End ();
			return;
		}

		/*Q_snprintfz (buf, sizeof(buf), "  %s\n", curr_cmd->name);
		if (!Con_PagedOutput (CON_OP_PRINT, buf))
			return;*/

		//Con_Printf ("  %s\n", curr_cmd->name);
		counter++;
	}

	Con_PagedOutput_End ();

	Con_Printf ("%i commands", counter);
	if (partial)
	{
		Con_Printf (" beginning with \"%s\"\n", partial);
	}
	else Con_Print ("\n");

	/*for (counter = 0, curr_cmd = cmd_functions ; curr_cmd ; curr_cmd = curr_cmd->next, counter++)
		Con_Printf ("%s\n", curr_cmd->name);

	Con_Printf ("------------\n%d commands\n", counter);*/
}

/*
============
Cmd_AddDirFile
- callback for COM_FindDirFiles in Cmd_Dir_f
============
*/
int Cmd_AddDirFile (com_fileinfo_t *fileinfo, int count, unsigned int param)
{

	Cmd_AddFilelistEntry (fileinfo->name, CMD_FTYPE_FILE, fileinfo->filelen, 0);
	return 0;		// continue searching
}

/*
====================
Cmd_Dir_f

List all files in the mod's directory	-- by joe
====================
*/
/*void Cmd_Dir_f (void)
{
	char	myarg[MAX_FILELENGTH];

	if (cmd_source != src_command)
		return;

	if (!strcmp(Cmd_Argv(1), cmd_null_string))
	{
		Q_strcpy (myarg, "*", sizeof(myarg));
		Q_strcpy (filetype, "file", sizeof(filetype));
	}
	else
	{
		Q_strcpy (myarg, Cmd_Argv(1), sizeof(myarg));
		// first two are exceptional cases
		if (strstr(myarg, "*"))
			Q_strcpy (filetype, "file", sizeof(filetype));
		else if (strstr(myarg, "*.dem"))
			Q_strcpy (filetype, "demo", sizeof(filetype));
		else
		{
			if (strchr(myarg, '.'))
			{
				Q_strcpy (filetype, COM_FileExtension(myarg), sizeof(filetype));
				filetype[strlen(filetype)] = 0x60;	// right-shadowed apostrophe
			}
			else
			{
				strcat (myarg, "*");
				Q_strcpy (filetype, "file", sizeof(filetype));
			}
		}
	}

	RDFlags |= RD_COMPLAIN;
	ReadDir (com_gamedir, myarg);
	if (!filelist)
		return;

	Con_Printf ("\x02" "%ss in current folder are:\n", filetype);
	PrintEntries ();
}
*/
void Cmd_Dir_f (cmd_source_t src)
{
	char	myarg[MAX_FILELENGTH], path[MAX_OSPATH];
	int		flags = 0;

	if (src == SRC_CLIENT)
		return;

	if (cmd_arglist.argc < 2)
	{
		Q_strcpy (myarg, "*", sizeof(myarg));
	}
	else
	{
		int len = Q_strcpy (myarg, cmd_arglist.argv[1], sizeof(myarg));
		if (strstr(myarg, "*"))
		{
			if (len >= 5)
			{
				if (COM_FilenamesEqual (myarg+len-5, "*.dem"))
					flags = FILE_ANY_DEMO;
			}
		}
		else Q_strcpy (myarg+len, "*", sizeof(myarg)-len);
	}

	Cmd_ClearFilelist ();
	Q_snprintfz (path, sizeof(path), "%s/%s", com_gamedir, myarg);
	COM_FindDirFiles (path, NULL, flags, Cmd_AddDirFile, 0);

	if (num_files > 0)
	{
		Con_Printf ("\x02" "%s in current folder are:\n", (flags & FILE_ANY_DEMO) ? "Demos" : "Files");
		PrintEntries (0, num_files);
		Cmd_ClearFilelist ();
	}
	else Con_Print ("<no files found>\n");
}

/*******JDH-new*******/

int	cmd_maxnamelen;

/*
============
Cmd_ListDemo
- callback for COM_FindAllFiles in Cmd_DemoList_f
============
*/
int Cmd_ListDemo (com_fileinfo_t *fileinfo, int count, unsigned int prefix)
{
	int			len, len2;
	char		dir[MAX_QPATH];

	if (count == 0)
	{
		Cmd_PrintFileList (cmd_maxnamelen);		// for previous searchpath
		cmd_maxnamelen = 0;

		Q_strcpy (dir, COM_SkipPath (fileinfo->searchpath->filename), sizeof(dir));
		len = strlen (dir);

		if (fileinfo->searchpath->pack)
		{
			len += Q_snprintfz (dir+len, sizeof(dir)-len, "/%s", COM_SkipPath (fileinfo->searchpath->pack->filename));
		}

		dir[len++] = '/';
		len2 = strlen ((char *)prefix)-4;			// exclude ".dem"
		Q_strncpy (dir+len, sizeof(dir)-len, (char *) prefix, len2);
		len += len2;
		//dir[len] = 0;

		Con_Printf ("\n%s\n", dir);
		Con_Printf ("%s\n", Con_Quakebar (len));
	}

	Cmd_AddFilelistEntry (fileinfo->name, CMD_FTYPE_FILE, fileinfo->filelen, 0);
	len = strlen (fileinfo->name);
	if (len > cmd_maxnamelen)
		cmd_maxnamelen = len;

	return 0;		// continue searching
}

/*
====================
Cmd_DemoList_f

List all demo files		-- by joe
  JDH: modified to list demos in all searchpaths, instead of a particular dir
====================
*/
void Cmd_DemoList_f (cmd_source_t src)
{
	char	myarg[MAX_FILELENGTH];

	if (src == SRC_CLIENT)
		return;

//	if (!strcmp(Cmd_Argv(1), cmd_null_string))
	if (cmd_arglist.argc < 2)
	{
		Q_strcpy (myarg, "*.dem", sizeof(myarg));
	}
	else
	{
		int len = Q_strcpy (myarg, cmd_arglist.argv[1], sizeof(myarg));
		if (strchr(myarg, '.'))
		{
			Con_Print ("You needn't use dots in demolist parameters\n");
			if (Q_strcasecmp (COM_FileExtension(myarg), "dem"))
			{
				Con_Print ("demolist is for demo files only\n");
				return;
			}
		}
		else
		{
			Q_strcpy (myarg+len, "*.dem", sizeof(myarg)-len);
		}
	}

	Con_PagedOutput_Begin ();

	Cmd_ClearFilelist ();
	COM_FindAllFiles (NULL, myarg, FILE_ANY_DEMO, Cmd_ListDemo, (unsigned int) myarg);
	Cmd_PrintFileList (cmd_maxnamelen);

	Con_PagedOutput_End ();

/*	Q_strcpy (filetype, "demo", sizeof(filetype));

	RDFlags |= (RD_STRIPEXT | RD_COMPLAIN);
	ReadDir (com_gamedir, myarg);
	if (!filelist)
		return;

	Con_Printf ("\x02" "%ss in current folder are:\n", filetype);
	PrintEntries ();
*/
}

/*
====================
AddTabs

Replaces nasty tab character with spaces	-- by joe
====================
*/
#define TABWIDTH 8

static void AddTabs (char *buf, int bufsize)
{
	char	*s, tmp[256], tabs[TABWIDTH+1];
	int		i, j;

	for (s = buf, i = 0 ; *s ; s++, i++)
	{
		switch (*(unsigned char *)s)
		{
		case 0xb4:
		case 0x27:
			*s = 0x60;
			break;

		case '\t':
			Q_strcpy (tmp, s + 1, sizeof(tmp));		// store remainder of line
			/*while (i++ < TABWIDTH)
				*s++ = ' ';
			*s-- = '\0';
			strcat (buf, tmp);*/
			for (j = 0; i < TABWIDTH ; j++, i++)
				tabs[j] = ' ';
			tabs[j] = '\0';
			Q_strcpy (s, tabs, bufsize-(s-buf));
			s += j;
			Q_strcpy (s, tmp, bufsize-(s-buf));
			s--;
			break;
		}

		if (i >= TABWIDTH-1)
			i = -1;
	}
}

/*
====================
Cmd_PrintTxt_f

Prints a text file into the console	-- by joe
====================
*/
void Cmd_PrintTxt_f (cmd_source_t src)
{
	char	name[MAX_FILELENGTH], buf[256] = {0};
	FILE	*f;
//	int		pagestart, pageheight, key;
//	extern int con_current;

	if (src == SRC_CLIENT)
		return;

	if (cmd_arglist.argc != 2)
	{
		Con_Print ("printtxt <txtfile> : displays a text file\n");
		return;
	}

	Q_strcpy (name, cmd_arglist.argv[1], sizeof(name));

//	COM_DefaultExtension (name, ".txt", sizeof(name));

//	Q_strcpy (buf, va("%s/%s", com_gamedir, name), sizeof(buf));
//	if (!(f = fopen(buf, "rt")))
	f = COM_FOpenFile (name, 0, NULL);
	if (!f)
	{
		COM_DefaultExtension (name, ".txt", sizeof(name));
		f = COM_FOpenFile (name, 0, NULL);
		if (!f)
		{
			Con_Printf ("ERROR: couldn't open %s\n", name);
			return;
		}
	}

	Con_Print ("\n");

	Con_PagedOutput_Begin ();

	/*pagestart = con_current;
	pageheight = ((int)scr_conlines / CON_LINEHEIGHT) - 4;
	scr_disabled_for_loading = true;		// don't update screen after every line*/

	while (fgets(buf, sizeof(buf), f))
	{
		AddTabs (buf, sizeof(buf));
//		if (!Con_PagedOutput (CON_OP_PRINT, buf))
		if (!Con_Print (buf))
			goto PRINTTXT_EXIT;

		/*Con_Printf ("%s", buf);

		if (con_current - pagestart >= pageheight)
		{
			Con_Print ("--------------------- (more) ---------------------");
			scr_disabled_for_loading = false;
			pagestart = con_current;
			key = SCR_ModalMessage (NULL, " ");
			con_x = 0;
			con_current--;
			if (key < 0)
				goto PRINTTXT_EXIT;
			scr_disabled_for_loading = true;
		}*/

		memset (buf, 0, sizeof(buf));
	}

	Con_Print ("\n\n");

PRINTTXT_EXIT:
	//scr_disabled_for_loading = false;
	Con_PagedOutput_End ();
	fclose (f);
}

/*
============
Cmd_ListMap
- callback for COM_FindAllFiles in Cmd_Maplist_f
============
*/
int Cmd_ListMap (com_fileinfo_t *fileinfo, int count, unsigned int prefix)
{
	int len;
	char dir[MAX_QPATH];

	if (count == 0)
	{
		Cmd_PrintFileList(cmd_maxnamelen);		// for previous searchpath
		cmd_maxnamelen = 0;

		len = Q_strcpy (dir, COM_SkipPath (fileinfo->searchpath->filename), sizeof(dir));
		if (fileinfo->searchpath->pack)
			len += Q_strcpy (dir+len, va ("/%s", COM_SkipPath (fileinfo->searchpath->pack->filename)), sizeof(dir)-len);
		len += Q_strcpy (dir+len, va ("/%s", (char *) prefix), sizeof(dir)-len);

		Con_Printf ("\n%s\n", dir);
		Con_Printf ("%s\n", Con_Quakebar (len));
	}

	if (COM_IsRealBSP (fileinfo->name))
	{
		len = strlen (fileinfo->name) - 4;
		fileinfo->name[len] = 0;		// remove .bsp suffix
		Cmd_AddFilelistEntry (fileinfo->name, CMD_FTYPE_FILE, fileinfo->filelen, 0);
		if (len > cmd_maxnamelen)
			cmd_maxnamelen = len;
	}

	return 0;		// continue searching
}

/*
============
Cmd_Maplist_f
============
*/
void Cmd_Maplist_f (cmd_source_t src)
{
	char prefix[MAX_QPATH];
	int len;

	if (src == SRC_CLIENT)
		return;

	len = Q_strcpy (prefix, "maps/", sizeof(prefix));
	if (cmd_arglist.argc > 1)
	{
		len += Q_strcpy (prefix+len, cmd_arglist.argv[1], sizeof(prefix)-len);
		if (prefix[len-1] == '*')
			prefix[--len] = 0;
	}
	Q_strcpy (prefix+len, "*.bsp", sizeof(prefix)-len);

	Con_PagedOutput_Begin ();

	Cmd_ClearFilelist ();
	COM_FindAllFiles (NULL, prefix, 0, Cmd_ListMap, (unsigned int) prefix);
	Cmd_PrintFileList (cmd_maxnamelen);

	Con_PagedOutput_End ();
}

/*******JDH-new*******/

/*
============
Cmd_Init
============
*/
void Cmd_Init (void)
{
// register our commands
	Cmd_AddCommand ("stuffcmds", Cmd_StuffCmds_f, 0);
	Cmd_AddCommand ("exec", Cmd_Exec_f, 0);
	Cmd_AddCommand ("echo", Cmd_Echo_f, 0);
	Cmd_AddCommand ("alias", Cmd_Alias_f, 0);
	Cmd_AddCommand ("wait", Cmd_Wait_f, 0);
#ifndef RQM_SV_ONLY
	Cmd_AddCommand ("cmd", Cmd_ForwardToServer, 0);
#endif

	// by joe
	Cmd_AddCommand ("cmdlist", Cmd_CmdList_f, 0);
	Cmd_AddCommand ("dir", Cmd_Dir_f, 0);
	Cmd_AddCommand ("demolist", Cmd_DemoList_f, 0);
	Cmd_AddCommand ("printtxt", Cmd_PrintTxt_f, 0);

	Cmd_AddCommand ("maplist", Cmd_Maplist_f, 0);		// JDH
}
