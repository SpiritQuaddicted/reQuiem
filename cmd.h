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
// cmd.h -- Command buffer and command execution

//===========================================================================

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/

void Cbuf_Init (void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText (const char *text, cmd_source_t);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void Cbuf_InsertText (const char *text, cmd_source_t);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void Cbuf_Execute (void);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

qboolean Cbuf_IsEmpty (void);

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.
*/

typedef void (*xcommand_t) (cmd_source_t);

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	const char		*name;
	xcommand_t		function;
	int				flags;
} cmd_function_t;


//extern	cmd_source_t	cmd_source;

#define	MAX_ARGS	  80
#define MAX_ARGBUF  1024

typedef struct
{
	int   argc;
	char  buf[MAX_ARGBUF*2];	// tokenized args, followed by exact copy of original cmd+args
	char  *argv[MAX_ARGS];		// pointers to within buf
	char  *cmdline;				// pointer to copy of cmd+args in buf
	short args_ofs[MAX_ARGS];	// offset (in cmdline) for each arg, in original form
} cmd_arglist_t;

//extern	int	pak_files;
//void FindFilesInPak (char *the_arg);

// flags for Cmd_AddCommand:
#define CMD_TEMPORARY 0x0040
#define CMD_DISABLED  0x8000

void Cmd_Init (void);

void Cmd_AddCommand (const char *cmd_name, xcommand_t function, int flags);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory

#ifdef HEXEN2_SUPPORT
  void Cmd_AddGameCommand (const char *cmd_name, xcommand_t function, int flags);
  void Cmd_RemoveGameCommand (const char *cmd_name);
#endif

void Cmd_AddLegacyCommand (const char *oldname, const char *newname);
void Cmd_RemoveLegacyCommand (const char *oldname);

qboolean Cmd_Exists (const char *cmd_name);
// used by the cvar code to check for cvar / command name overlap

int Cmd_Argc (void);
const char *Cmd_Argv (int arg);
const char * Cmd_Args (int argstart);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are always safe.

int Cmd_CheckParm (const char *parm);
// Returns the position (1 to argc-1) in the command's argument list
// where the given parameter apears, or 0 if not present

void Cmd_TokenizeString (const char *text, cmd_arglist_t *out);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void Cmd_ExecuteString (const char *text, cmd_source_t src);
// Parses a single line of text into arguments and tries to execute it.
// The text can come from the command buffer, a remote client, or stdin.

void Cmd_ForwardToServer (cmd_source_t src);
// adds the current command line as a clc_stringcmd to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

void Cmd_Print (const char *text);
// used by command functions to send output to either the graphics console or
// passed as a print message to the client


#define	MAX_FILELENGTH	64

typedef enum {CMD_FTYPE_FILE, CMD_FTYPE_DIR, CMD_FTYPE_CDUP} cmd_ftype_t;

typedef struct file_entry_s
{
	cmd_ftype_t	type;
	char		*name;
	int			size;
//	struct file_entry_s *next;
} file_entry_t;

extern	file_entry_t	*filelist;
extern	int				num_files;

/*
extern	int		RDFlags;

#define	RD_MENU_DEMOS		1	// for demos menu printing
#define	RD_MENU_DEMOS_MAIN	2	// to avoid printing ".." in the main Quake folder
#define	RD_COMPLAIN		4	// to avoid printing "No such file"
#define	RD_STRIPEXT		8	// for stripping file's extension
#define	RD_NOERASE		16	// to avoid deleting the filelist
#define	RD_SKYBOX		32	// for skyboxes
#define	RD_GAMEDIR		64	// for the "gamedir" command

void ReadDir (char *path, char *the_arg);
*/

#define AFE_NO_NAMESORT		0x0001
#define AFE_NO_TYPESORT		0x0002
#define AFE_KEEPCASE		0x0004

int  Cmd_AddFilelistEntry (const char *fname, cmd_ftype_t ftype, long fsize, int flags);
void Cmd_ClearFilelist (void);
qboolean Cmd_CheckEntryName (const char *ename);
const char * Cmd_FindCommonStart (void);

const char * Cmd_TabComplete (const char *cmdline, qboolean cycle_rev);
/*
void Cmd_CompleteParameter (char *partial, const char *attachment, qboolean reverse_cycle);
void Cmd_CompleteCmdOrCvar (const char *partial, qboolean reverse_cycle);

const char *Cmd_CompleteCommand (const char *partial);
*/
// attempts to match a partial command for automatic command line completion
// returns NULL if nothing fits

