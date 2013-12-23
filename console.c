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
// console.c

//#ifdef NeXT
//#include <libc.h>
//#endif
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>
#include "quakedef.h"
#include "movie.h"		// just for Movie_IsActive check in Con_Print

#define CON_CHARWIDTH  DRAW_CHARWIDTH
#define CON_LINEHEIGHT DRAW_CHARWIDTH

//#define CON_EMPTYCHAR ' '
#define CON_EMPTYCHAR     0
#define CON_STARTWIDTH   80		// JDH: was 38
#define	CON_CURSORSPEED 2.0		// JDH: originally 4

qboolean	con_initialized = false;
int 		con_linewidth;
int			con_x;			// offset in current line for next print

FILE		*con_debuglog = NULL;		// JDH: log is now kept open (no more performance hit)


#ifndef RQM_SV_ONLY


#define		CON_TEXTSIZE	65536
#define		CON_MINSIZE     16384 //johnfitz -- old default, now the minimum size
int			con_buffersize = 0; //johnfitz -- user can now override default

qboolean 	con_forcedup;		// because no entities to refresh

int		con_totallines;		// total lines in console scrollback
int		con_numlines = 0;	// number of non-blank text lines, used for backscrolling, added by joe
int		con_backscroll;		// lines up from bottom to display
int		con_current;		// where next message will be printed

#ifdef HEXEN2_SUPPORT
  typedef short conchar_t;	// 9-bit characters
#else
  typedef char  conchar_t;
#endif

conchar_t	*con_text = NULL;

// JDH: code assumes MAXCMDLINES is a power of 2
#define MAXCMDLINES 64		// 32 doubled -- joe


typedef struct linehist_s
{
	char text[MAXCMDLINELEN];
//	char *pre_completion;			// JDH: unfinished idea
//	char *post_completion;
} linehist_t;


// joe: stuff from [sons]Quake for saved history lines
typedef struct
{
	int		con_editline;
	int		con_histline;
	int		linepos;
	char	lines[MAXCMDLINES][MAXCMDLINELEN];
} cmdhistory_t;


qboolean On_Change_con_linespacing (cvar_t *var, const char *string);

cvar_t		con_notifytime   = {"con_notifytime",   "3", CVAR_FLAG_ARCHIVE};		//seconds
cvar_t		_con_notifylines = {"con_notifylines",  "4", CVAR_FLAG_ARCHIVE};
cvar_t		con_linespacing  = {"con_linespacing",  "0", CVAR_FLAG_ARCHIVE, On_Change_con_linespacing};		// JDH
cvar_t		con_autocomplete = {"con_autocomplete", "1", CVAR_FLAG_ARCHIVE};		// JDH

#define	NUM_CON_TIMES 16
float		con_times[NUM_CON_TIMES];	// realtime time the line was generated
						// for transparent notify lines

int		con_vislines;
int		con_notifylines;		// scan lines to clear for notify lines
int		con_lineheight = CON_LINEHEIGHT;		// note: change this if default value of con_linespacing is changed

linehist_t	con_keylines[MAXCMDLINES];
int			con_editpos;

int			con_editline = 0;
int			con_histline = 0;
int			con_insert = 1;		// joe: from [sons]Quake
int			con_selstart;		// JDH: for auto-complete


char		con_lastcenterstring[MAXPRINTMSG];		// JDH (from fitz)
cvar_t		con_logcenterprint = {"con_logcenterprint", "1", CVAR_FLAG_ARCHIVE};		// JDH (from fitz)

#define CON_CLEARSEL()									\
	if (con_selstart)									\
	{													\
		con_keylines[con_editline].text[con_selstart] = 0;	\
		con_selstart = 0;								\
	}

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (cmd_source_t src)
{
//	con_keylines[con_editline][1] = 0;	// clear any typing
//	con_editpos = 1;

	if ((key_dest == key_console) || ((key_dest == key_game) && (cls.state != ca_connected)))
	{
		if (cls.state == ca_connected)
			key_dest = key_game;
		else
//			M_Menu_Main_f ();
			key_dest = key_menu;
	}
	else
	{
		key_dest = key_console;
	}

	SCR_EndLoadingPlaque ();
	memset (con_times, 0, sizeof(con_times));
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (cmd_source_t src)
{
	if (con_text)
		memset (con_text, CON_EMPTYCHAR, con_buffersize*sizeof(conchar_t));
}

/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;

	for (i=0 ; i<NUM_CON_TIMES ; i++)
		con_times[i] = 0;
}

/*
================
Con_MessageMode_f
================
*/
extern qboolean team_message;

void Con_MessageMode_f (cmd_source_t src)
{
	key_dest = key_message;
	team_message = false;
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (cmd_source_t src)
{
	key_dest = key_message;
	team_message = true;
}

qboolean On_Change_con_linespacing (cvar_t *var, const char *string)
{
	float val = Q_atof (string);

	if ((val < var->minvalue) || (val > var->maxvalue) || (val != (int)val))
	{
		Con_Printf ("%s must be an integer between %d and %d\n", var->name, var->minvalue, var->maxvalue);
		return true;
	}

	con_lineheight = CON_LINEHEIGHT + val;
	return false;		// allow change
}

/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int			i, j, width, oldwidth, oldtotallines, numlines, numchars;
	conchar_t	*tbuf;

	width = (vid.conwidth / CON_CHARWIDTH) - 2;

	if (width == con_linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = CON_STARTWIDTH;
		con_linewidth = width;
		con_totallines = con_buffersize / con_linewidth;
		memset (con_text, CON_EMPTYCHAR, con_buffersize*sizeof(conchar_t));
	}
	else
	{
		oldwidth = con_linewidth;
		con_linewidth = width;
		oldtotallines = con_totallines;
		con_totallines = con_buffersize / con_linewidth;
		numlines = oldtotallines;

		if (con_totallines < numlines)
			numlines = con_totallines;

		numchars = oldwidth;

		if (con_linewidth < numchars)
			numchars = con_linewidth;

		tbuf = Q_malloc (con_buffersize*sizeof(conchar_t));
		memcpy (tbuf, con_text, con_buffersize*sizeof(conchar_t));
		memset (con_text, CON_EMPTYCHAR, con_buffersize*sizeof(conchar_t));

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con_text[(con_totallines - 1 - i) * con_linewidth + j] =
					tbuf[((con_current - i + oldtotallines) % oldtotallines) * oldwidth + j];
			}
		}

		free (tbuf);
		Con_ClearNotify ();
	}

	con_backscroll = 0;
	con_current = con_totallines - 1;
}


/*
================
Con_AdjustHeight
  by joe, from ZQuake;  JDH: fixed to use proper line height
================
*/
void Con_AdjustHeight (int delta)
{
	extern	cvar_t	scr_consize;
//	int		height;
	float	height;

	if (!cl.worldmodel || cls.signon != SIGNONS)
		return;

	height = (scr_consize.value * vid.height) + delta;
//	height = (scr_consize.value * vid.height + delta + 5) / 10;
//	height *= 10;
	if (delta < 0 && height < 30)
		height = 30;
	if (delta > 0 && height > vid.height - 10)
		height = vid.height - 10;
	Cvar_SetValueDirect (&scr_consize, (float)height / vid.height);
}

void Con_SaveHistory (void)
{
	// joe: stuff from [sons]Quake
	FILE		*cmdhist;
	cmdhistory_t	cmdhistory;
	int		i, j;

	cmdhist = fopen(va("%s/%s/cmdhist.dat", com_basedir, RQMDIR), "wb");
	if (!cmdhist)
		cmdhist = fopen(va("%s/cmdhist.dat", com_basedir), "wb");

	if (cmdhist)
	{
		CON_CLEARSEL();

		for (i=0 ; i<MAXCMDLINES ; i++)
			for (j=0 ; j<MAXCMDLINELEN ; j++)
				cmdhistory.lines[i][j] = con_keylines[i].text[j];
		cmdhistory.linepos = con_editpos;
		cmdhistory.con_histline = con_histline;
		cmdhistory.con_editline = con_editline;
		fwrite (&cmdhistory, sizeof(cmdhistory_t), 1, cmdhist);
		fclose (cmdhist);
	}
}

void Con_LoadHistory (void)
{
	FILE		*cmdhist;
	cmdhistory_t	cmdhistory;
	int		i, j;

	cmdhist = fopen(va("%s/%s/cmdhist.dat", com_basedir, RQMDIR), "rb");
	if (!cmdhist)
		cmdhist = fopen(va("%s/cmdhist.dat", com_basedir), "rb");
	if (cmdhist)
	{
		fread (&cmdhistory, sizeof(cmdhistory_t), 1, cmdhist);
		fclose (cmdhist);
		for (i=0 ; i<MAXCMDLINES ; i++)
			for (j=0 ; j<MAXCMDLINELEN ; j++)
				con_keylines[i].text[j] = cmdhistory.lines[i][j];
		con_editpos = cmdhistory.linepos;
		con_histline = cmdhistory.con_histline;
		con_editline = cmdhistory.con_editline;
	}
	else
	{
		for (i=0 ; i<MAXCMDLINES ; i++)
		{
			con_keylines[i].text[0] = ']';
			con_keylines[i].text[1] = 0;
		}
		con_editpos = 1;
	}
}

#endif		//#ifndef RQM_SV_ONLY

/*
================
Con_PreInit
  JDH: splitting this from Con_Init allows console to be active earlier
================
*/
void Con_PreInit (void)
{
#ifndef RQM_SV_ONLY
	int		parm;

	//johnfitz -- user settable console buffer size
	parm = COM_CheckParm ("-consize");
	if (parm && (parm+1 < com_argc))
		con_buffersize = max (CON_MINSIZE, Q_atoi (com_argv[ parm+1 ])*1024);
	else
		con_buffersize = CON_TEXTSIZE;

	con_text = Hunk_AllocName (con_buffersize*sizeof(conchar_t), "context");
	con_linewidth = CON_STARTWIDTH;
	con_totallines = con_buffersize / con_linewidth;
	memset (con_text, CON_EMPTYCHAR, con_buffersize*sizeof(conchar_t));
	con_backscroll = 0;
	con_current = con_totallines - 1;

// register our commands
	Cvar_RegisterFloat (&con_notifytime, 0, 5);
	Cvar_RegisterInt (&_con_notifylines, 0, NUM_CON_TIMES);
	Cvar_RegisterInt (&con_linespacing, 0, CON_LINEHEIGHT);
	Cvar_RegisterInt (&con_autocomplete, 0, 2);

	Cvar_RegisterInt (&con_logcenterprint, 0, 2); //johnfitz

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f, 0);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f, 0);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f, 0);
	Cmd_AddCommand ("clear", Con_Clear_f, 0);
#else
	con_linewidth = 80;
#endif
}

/*
================
Con_Init
  JDH: Con_PreInit *MUST* be called first!
================
*/
void Con_Init (void)
{
	char	temp[MAX_OSPATH];

	if ((COM_CheckParm("-condebug")))
	{
		Q_snprintfz (temp, sizeof(temp), "%s/qconsole.log", com_gamedir);
		unlink (temp);
		con_debuglog = fopen (temp, "a+t");
	}

#ifndef RQM_SV_ONLY
	Con_LoadHistory ();
#endif

	con_initialized = true;
//	Con_Print ("Console initialized\n");
}

/*
================
Con_Shutdown
================
*/
void Con_Shutdown (void)
{
	if (con_debuglog)
	{
		fflush (con_debuglog);
		fclose (con_debuglog);
		con_debuglog = NULL;
	}
}

/*
================
Con_DebugLog
================
*/
void Con_DebugLog (const char *msg)
{
	if (con_debuglog)
	{
		fputs (msg, con_debuglog);
	}
}

#ifndef RQM_SV_ONLY

#ifdef HEXEN2_SUPPORT
/*
===============
Con_UpdateChars_H2
  - changes the colored character bit when switching to/from hexen2
===============
*/
void Con_UpdateChars_H2 (qboolean isH2)
{
	int oldmask, newmask, i, j;
	conchar_t *text;

	if (isH2)
	{
		oldmask = 128;
		newmask = 256;
	}
	else
	{
		oldmask = 256;
		newmask = 128;
	}

	for (i = 0 ; i < con_numlines ; i++)
	{
		j = con_current - con_numlines + i;
		text = con_text + (j % con_totallines)*con_linewidth;

		for (j = 0 ; j < con_linewidth ; j++)
		{
			if (text[j] & oldmask)
				text[j] = (text[j] & ~oldmask) | newmask;
		}
	}
}
#endif

/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (void)
{
	con_x = 0;
	con_current++;
	if (con_numlines < con_totallines)	// by joe
		con_numlines++;
	memset (&con_text[(con_current%con_totallines)*con_linewidth], CON_EMPTYCHAR, con_linewidth*sizeof(conchar_t));

// mark time for transparent overlay
	if (con_current > 0)
		con_times[(con_current-1) % NUM_CON_TIMES] = realtime;
}

/*
===============
Con_InsertText (JDH: for pasting from clip)
===============
*/
void Con_InsertText (const char *text)
{
	int	len = strlen (text);

	if (len + strlen(con_keylines[con_editline].text) > MAXCMDLINELEN - 1)
		len = MAXCMDLINELEN - 1 - strlen(con_keylines[con_editline].text);
	if (len > 0)
	{	// insert the string
		memmove (con_keylines[con_editline].text + con_editpos + len,
				con_keylines[con_editline].text + con_editpos,
				strlen(con_keylines[con_editline].text) - con_editpos + 1);
		memcpy (con_keylines[con_editline].text + con_editpos, text, len);
		con_editpos += len;
	}
}

/*
================
Con_AddText (JDH: used to be called Con_Print)

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the notify window will pop up.
================
*/
void Con_AddText (const char *txt)
{
	int		y, c, l, mask = 0;
//	static int	cr;

	con_backscroll = 0;

	if (txt[0] == 1)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			mask = 256;
			S_LocalSound ("misc/comm.wav");
		}
		else
	#endif
		{
			mask = 128;		// go to colored text
			S_LocalSound ("misc/talk.wav");	// play talk wav
		}
		txt++;
	}
	else if (txt[0] == 2)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			mask = 256;
		else
	#endif
		mask = 128;		// go to colored text
		txt++;
	}

	while ((c = *txt))
	{
	// count word length
		for (l = 0 ; l < con_linewidth ; l++)
			if (txt[l] <= ' ')
				break;

	// word wrap
		if (l != con_linewidth && (con_x + l > con_linewidth))
			Con_Linefeed ();//con_x = 0;

		txt++;

		/*if (cr)
		{
			con_current--;
			cr = false;
		}*/

		/*if (!con_x)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (con_current >= 0)
				con_times[con_current % NUM_CON_TIMES] = realtime;
		}*/

		switch (c)
		{
		case '\r':
			if (*txt == '\n')
				txt++;
		case '\n':
			Con_Linefeed ();//con_x = 0;
			break;

		default:	// display character and advance
			y = con_current % con_totallines;
			con_text[y*con_linewidth+con_x] = c | mask;
			con_x++;
			if (con_x >= con_linewidth)
			{
				Con_Linefeed ();//con_x = 0;

			// JDH: skip newline if right after line break
				if (*txt == '\r')
					txt++;
				if (*txt == '\n')
					txt++;
			}
			break;
		}
	}
}

/*
==================
JDH: paged output - for printing large amounts of data; pauses after each screenful
==================
*/
int con_pageheight = Q_MAXINT;		// max # lines to output before breaking
int con_pagestart = 0;		// value of con_current when paged output is started

void Con_PagedOutput_Begin (void)
{
	if (scr_conlines)
	{
		con_pagestart = con_current;
		con_pageheight = ((int)scr_conlines / con_lineheight) - 5;
		scr_disabled_for_loading = true;		// don't update screen after every line
	}
}

void Con_PagedOutput_End (void)
{
	scr_disabled_for_loading = false;
	con_pageheight = Q_MAXINT;
}

extern qboolean config_exec;

/*
================
Con_Print
  JDH: identical to Con_Printf, but with a pre-formatted string
================
*/
qboolean Con_Print (const char *msg)
{
	int		was_current_line, key;

// JDH: check if user has cancelled paged output
	if (!con_pageheight)
		return false;

	// also echo to debugging console
	Sys_Printf ("%s", msg);

	// log all messages to file
	if (con_debuglog)
		Con_DebugLog (msg);

	if (!con_initialized)
	{
		if (con_text)			// JDH: if Con_PreInit has been called, we can buffer text
			Con_AddText (msg);
		return true;
	}

	if (cls.state == ca_dedicated)
		return true;		// no graphics mode

// JDH: added this to disable printing when changing gamedir (but not during initial loading of configs)
	if (config_exec && (host_time > 1.5))
		return true;

	// cut down on the number of SCR_UpdateScreen calls by doing it only once per line
	// (ie. only if con_current changes)
	was_current_line = con_current;

	// write it to the scrollable buffer
	Con_AddText (msg);

	if (con_current > was_current_line)
	{
		// JDH: paged output
		if (con_current - con_pagestart >= con_pageheight)
		{
			//Con_AddText ("--------------------- (more) ---------------------");
			Con_AddText ("--------[");
			Con_AddText ("\x02""SPACE");
			Con_AddText (": next page]----[");
			Con_AddText ("\x02""TAB");
			Con_AddText (": show all]----[");
			Con_AddText ("\x02""ESC");
			Con_AddText (": stop]--------");

			scr_disabled_for_loading = false;
			con_pagestart = con_current;
			key = SCR_ModalMessage (NULL, "q\t ");

			con_current--;
			Con_Linefeed ();
			if (key <= 0)		// esc or q cancels output
			{
				Con_AddText ("\n");
				con_pageheight = 0;		// block remainder of Con_Printf calls until Con_PagedOutput_End
				return false;
			}
			if (key == 1)
				con_pageheight = Q_MAXINT;		// pressing tab causes output to continue w/o page breaks

			scr_disabled_for_loading = true;	// don't print line-by-line - wait for a pagefull
			return true;
		}

		// update the screen manually if there's no map running
		//	(JDH: unless it's during initial loading or movie capture)
		if ((cls.signon != SIGNONS) && !scr_disabled_for_loading /*&& host_initialized*/ && !Movie_IsActive())
		{
			SCR_UpdateScreen ();
		}
	}

	return true;
}

#else
// server-only versions

void Con_PagedOutput_Begin (void) {}

void Con_PagedOutput_End (void) {}

qboolean Con_Print (const char *msg)
{
	// also echo to debugging console
	Sys_Printf ("%s", msg);

	// log all messages to file
	if (con_debuglog)
		Con_DebugLog (msg);

	return true;
}

#endif		//#ifdef RQM_SV_ONLY

/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
JDH: returns false only if paged output is being used, and user cancels
================
*/
qboolean Con_Printf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	return Con_Print (msg);
}

/*
================
Con_DPrintf

A Con_Printf that only shows up if the "developer" cvar is set
================
*/
void Con_DPrintf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!developer.value)
		return;			// don't confuse non-developers with techie stuff...

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Con_Print (msg);
}

/*
==================
Con_SafePrintf

Okay to call even when the screen can't be updated
==================
*/
void Con_SafePrintf (const char *fmt, ...)
{
	va_list		argptr;
	char		msg[1024];
#ifndef RQM_SV_ONLY
	int		temp;
#endif

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

#ifndef RQM_SV_ONLY
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	Con_Print (msg);
	scr_disabled_for_loading = temp;
#else
	Con_Print (msg);
#endif
}

/*
==================
Con_ColumnPrint

Prints the given string to the console, aligned to the next multiple of colwidth
==================
*/
void Con_ColumnPrint (const char *s, int colwidth)
{
	int		nextcolx = 0, pos = 0, x;
	char	line[256];

	if (con_x)
		nextcolx = (int)((con_x + colwidth) / colwidth) * colwidth;

	if ((nextcolx > con_linewidth - (colwidth-2))	// the last column may be slightly smaller
		|| (con_x && nextcolx + strlen(s) >= con_linewidth))
	{
		line[pos++] = '\n';
		x = 0;
	}
	else
	{
		x = con_x;
		if (x)
		{
			line[pos++] = ' ';
			x++;
		}
	}

	for ( ; x % colwidth; x++)
		line[pos++] = ' ';

	line[pos] = 0;
	Con_Printf ("%s%s", line, s);
}

/*
================
Con_Quakebar -- johnfitz -- returns a bar of the desired length, but never wider than the console

includes a newline, unless len >= con_linewidth.
================
*/
char *Con_Quakebar (int len)
{
	static char bar[42];
	int i;

	len = min(len, sizeof(bar) - 2);
	len = min(len, con_linewidth);

	bar[0] = '\35';
	for (i = 1; i < len - 1; i++)
		bar[i] = '\36';
	bar[len-1] = '\37';

	if (len < con_linewidth)
	{
		bar[len] = '\n';
		bar[len+1] = 0;
	}
	else
		bar[len] = 0;

	return bar;
}

#ifndef RQM_SV_ONLY

/*
================
Con_CenterPrintf -- johnfitz -- pad each line with spaces to make it appear centered
================
*/
void Con_CenterPrintf (int linewidth, const char *fmt, ...)
{
	va_list	argptr;
	char	msg[MAXPRINTMSG]; //the original message
	char	line[MAXPRINTMSG]; //one line from the message
	char	spaces[21]; //buffer for spaces
	char	*src, *dst;
	int		len, s;

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	linewidth = min (linewidth, con_linewidth);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		ReformatText(msg, linewidth);
		/*for (src = msg; *src; src++)
		{
			if (*src == '@')
				*src = '\n';
		}*/
	}
#endif

	for (src = msg; *src; )
	{
		dst = line;
		while (*src && *src != '\n')
			*dst++ = *src++;

		*dst = 0;
		if (*src == '\n')
			src++;

		len = strlen(line);
		if (len < linewidth)
		{
			s = (linewidth-len)/2;
			memset (spaces, ' ', s);
			spaces[s] = 0;
			Con_Printf ("%s%s\n", spaces, line);
		}
		else
			Con_Printf ("%s\n", line);
	}
}

/*==================
Con_LogCenterPrint -- johnfitz -- echo centerprint message to the console
==================
*/
void Con_LogCenterPrint (const char *str)
{
	if (!strcmp(str, con_lastcenterstring))
		return; //ignore duplicates

	if ((cl.gametype == GAME_DEATHMATCH) && (con_logcenterprint.value != 2))
		return; //don't log in deathmatch

	Q_strcpy (con_lastcenterstring, str, sizeof(con_lastcenterstring));

	if (con_logcenterprint.value)
	{
		Con_Print (Con_Quakebar(40));
		Con_CenterPrintf (40, "%s\n", str);
		Con_Print (Con_Quakebar(40));
		Con_ClearNotify ();
	}
}

/*
==============================================================================

DRAWING

==============================================================================
*/

/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
// modified by joe to work as ZQuake
void Con_DrawInput (void)
{
	int		i, sel_end;
	char	*text;
	char	temp[MAXCMDLINELEN];

	if (key_dest != key_console && !con_forcedup)
		return;		// don't draw anything

	i = Q_strcpy (temp, con_keylines[con_editline].text, sizeof(temp));
	text = temp;

	sel_end = (con_selstart ? i : 0);

	// fill out remainder with spaces
	for ( ; i < MAXCMDLINELEN ; i++)
		text[i] = ' ';

	if (!con_selstart)
	{
		// add the cursor frame
		if ((int)(realtime * CON_CURSORSPEED) & 1)
			text[con_editpos] = 11 + 84 * con_insert;
	}

	// prestep if horizontally scrolling
	if (con_editpos >= con_linewidth)
		text += 1 + con_editpos - con_linewidth;

	// draw it
//#ifdef GLQUAKE
	Draw_String (CON_CHARWIDTH, con_vislines - 2*con_lineheight, text);
//#else
//	for (i=0 ; i<con_linewidth ; i++)
//		Draw_Character ((i+1)*CON_CHARWIDTH, con_vislines - 2*con_lineheight, text[i]);
//#endif

//	for (i = 0; i < con_linewidth; i++)
//		Draw_Character ((i+1)*CON_CHARWIDTH, con_vislines - 2*con_lineheight, text[i]);

	if (con_selstart)
	{
		extern int char_texture, char_texture_bright;
		int waschars = char_texture;

//		Draw_Fill ((key_selstart+1)*CON_CHARWIDTH, con_vislines - 2*con_lineheight,
//						(sel_end - key_selstart)*CON_CHARWIDTH, con_lineheight+1, 7);

		for (i = 0; i < con_lineheight; i++)
		{
//			byte rgb[3];

//			rgb[0] = rgb[1] = rgb[2] = 80 + i*16;
//			Draw_FillRGB ((key_selstart+1)*CON_CHARWIDTH, con_vislines - con_lineheight - i,
//						(sel_end - key_selstart)*CON_CHARWIDTH, 1, rgb);
			Draw_Fill ((con_selstart+1)*CON_CHARWIDTH, con_vislines - con_lineheight - i,
						(sel_end - con_selstart)*CON_CHARWIDTH, 1, i+5);
		}

//		Draw_Fill ((con_selstart+1)*CON_CHARWIDTH, con_vislines - con_lineheight + 1,
//					(sel_end - con_selstart)*CON_CHARWIDTH, 1, 4);

	// This combiner is similar to inverting the character's colors, except any
	// black pixels effectively become transparent.  This works well for the
	// black-outlined character set I use.

		glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_COLOR);		// (1-src)*dst
		glEnable (GL_BLEND);
		char_texture = char_texture_bright;

		for (i = con_selstart; i < sel_end; i++)
		{
//			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//			Draw_Character ((i+1)*CON_CHARWIDTH, con_vislines - 2*con_lineheight, 11);

//			if (text[i] != 11 + 84 * key_insert)
			{
//				glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
				Draw_Character ((i+1)*CON_CHARWIDTH, con_vislines - 2*con_lineheight, text[i]);
			}
		}

		char_texture = waschars;
		glDisable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//		glColor3f (1,1,1);
	}
}

/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int			x, v, i, maxlines, lastline;
	conchar_t	*text;
	float		time;
	extern	char	chat_buffer[];

	maxlines = bound(0, _con_notifylines.value, NUM_CON_TIMES);
	lastline = (con_x ? con_current : con_current-1);

	v = 0;
	for (i = lastline - maxlines + 1 ; i <= lastline ; i++)
	{
		if (i < 0)
			continue;
		time = con_times[i%NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = realtime - time;
		if (time > con_notifytime.value)
			continue;
		text = con_text + (i % con_totallines) * con_linewidth;

		clearnotify = 0;
		//scr_copytop = 1;

		for (x = 0 ; (x < con_linewidth) && text[x] ; x++)
			Draw_Character ((x+1)*CON_CHARWIDTH, v, text[x]);

		v += con_lineheight;
	}

	if (key_dest == key_message)
	{
		clearnotify = 0;
//		scr_copytop = 1;

		i = 0;

		if (team_message)
		{
			Draw_String (CON_CHARWIDTH, v, "(say):");
			x = 7;
		}
		else
		{
			Draw_String (CON_CHARWIDTH, v, "say:");
			x = 5;
		}

		while (chat_buffer[i])
		{
			Draw_Character (x * CON_CHARWIDTH, v, chat_buffer[i]);
			x++;

			i++;
			if (x > con_linewidth)
			{
				x = team_message ? 7 : 5;
				v += con_lineheight;
			}
		}
		Draw_Character (x * CON_CHARWIDTH, v, 10 + ((int)(realtime * CON_CURSORSPEED) & 1));
		v += con_lineheight;
	}

	if (v > con_notifylines)
		con_notifylines = v;
}

/*
================
Con_DrawConsole

Draws the console with the solid background
The typing input line at the bottom should only be drawn if typing is allowed
================
*/
void Con_DrawConsole (int lines, qboolean drawinput)
{
	int			i, j, x, y, rows, lastline;
	conchar_t	*text;
//	extern cvar_t	gl_hwblend;

	if (lines <= 0)
		return;

/*	if (v_blend[3] && vid_hwgamma_enabled && gl_hwblend.value)
	{
		float a = v_blend[3]*v_blend[3];

		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor3f (1-(v_blend[0]*a), 1-(v_blend[1]*a), 1-(v_blend[2]*a));
	}
*/
// draw the background
	Draw_ConsoleBackground (lines);

// draw the text
	con_vislines = lines;

	rows = (lines / con_lineheight) - 2;		// rows of text to draw
//	y = lines - 2*CON_LINEHEIGHT - (rows * CON_LINEHEIGHT);	// may start slightly negative
	y = lines - (2+rows)*con_lineheight;	// may start slightly negative

	con_backscroll = bound(0, con_backscroll, con_numlines - rows);
	lastline = (con_x ? con_current : con_current-1);	// don't include empty line at bottom

	for (i = lastline - rows + 1 ; i <= lastline ; i++, y += con_lineheight)
	{
		// added by joe
		if (con_backscroll)
		{
			if (i == lastline)
			{
				for (x = 0 ; x < con_linewidth ; x += 4)
					Draw_Character ((x+1)*CON_CHARWIDTH, y, '^');
				continue;
			}

			j = i - con_backscroll;
			if (j < 0)
				j = 0;
		}
		else j = i;

		text = con_text + (j % con_totallines)*con_linewidth;

		for (x = 0 ; (x < con_linewidth) && text[x] ; x++)
			Draw_Character ((x+1)*CON_CHARWIDTH, y, text[x]);
	}

// draw the input prompt, user text, and cursor if desired
	if (drawinput)
		Con_DrawInput ();
/*
	if (v_blend[3])
	{
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor3f (1, 1, 1);
	}
*/
}

/*
==================
Con_NotifyBox
==================
*/
/*void Con_NotifyBox (char *text)
{
	double		t1, t2;

// during startup for sound / cd warnings
	Con_Print("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	Con_Print (text);

	Con_Print ("Press a key.\n");
	Con_Print("\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	key_count = -2;		// wait for a key down and up
	key_dest = key_console;

	do {
		t1 = Sys_DoubleTime ();
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();
		t2 = Sys_DoubleTime ();
		realtime += t2-t1;		// make the cursor blink
	} while (key_count < 0);

	Con_Print ("\n");
	key_dest = key_game;
	realtime = 0;				// put the cursor back to invisible
}
*/

/*
====================
Con_Keydown_Tab

various parameter completions -- by joe/JDH
====================
*/
#if 1
void Con_Keydown_Tab (void)
{
	linehist_t *currline, *histline;
	qboolean	cycle_rev;
	const char	*p;

	currline = &con_keylines[con_editline];
	histline = &con_keylines[con_histline];

	cycle_rev = keydown[K_SHIFT];
/*
	if (((int)cl_advancedcompletion.value == 1) &&
		histline->post_completion && histline->pre_completion &&
		!strcmp(histline->post_completion, currline->text))
	{
		extern char last_fillin[MAXCMDLINE];
		if (strcmp(last_fillin, histline->pre_completion + 1))
		{
			Cmd_TabComplete (histline->pre_completion + 1, cycle_rev);
			Q_strcpy (last_fillin, currline->text + 1, sizeof(last_fillin));
		}
	}
	else
	{
		if (currline->pre_completion)
		{
			free (currline->pre_completion);
			currline->pre_completion = NULL;
		}
	}
*/

	p = Cmd_TabComplete (currline->text + 1, cycle_rev);

	if (p)
	{
/*		if (!currline->pre_completion)
			currline->pre_completion = Q_strdup (currline->text);

		if (currline->post_completion)
			free (currline->post_completion);
*/
		Q_strcpy (currline->text+1, p, sizeof(currline->text)-1);
		con_editpos = 1 + strlen(p);
//		currline->post_completion = Q_strdup (currline->text);

	//	con_keylines[con_editline][con_editpos++] = ' ';
	//	con_keylines[con_editline][con_editpos] = 0;
	}
};

#else

void Con_Keydown_Tab (void)
{
	qboolean	cycle_rev;
	char		*cmd, *param;
	cmd_arglist_t args;
	const char	*p;

	extern void Cmd_CompleteParameter (char *partial, const char *attachment, qboolean reverse_cycle);
	extern void Cmd_CompleteCmdOrCvar (const char *partial, qboolean reverse_cycle);
	extern const char *Cmd_CompleteCommand (const char *partial);

	cycle_rev = keydown[K_SHIFT];
	cmd = con_keylines[con_editline].text+1;

	//  JDH: replaced strstr with Q_strncasecmp so it's not case-sensitive
	if (!Q_strncasecmp(cmd, "playdemo ", 9) || !Q_strncasecmp(cmd, "timedemo ", 9) ||
		!Q_strncasecmp(cmd, "capturedemo ", 12))
	{
		param = "*.dem";
	}
	else if (!Q_strncasecmp(cmd, "printtxt ", 9))
	{
		param = "*.txt";
	}
	else if (!Q_strncasecmp(cmd, "map ", 4) || !Q_strncasecmp(cmd, "changelevel ", 12) ||
			 !Q_strncasecmp(cmd, "record ", 7) || !Q_strncasecmp(cmd, "capture_start ", 14))
	{
		param = "*.bsp";
	}
	else if (!Q_strncasecmp(cmd, "exec ", 5))
	{
		param = "*.cfg";
	}
	else if (!Q_strncasecmp(cmd, "load ", 5))
	{
		param = "*.sav";
	}
	else if (!Q_strncasecmp(cmd, "loadsky ", 8) || !Q_strncasecmp(cmd, "r_skybox ", 9) ||
			 !Q_strncasecmp(cmd, "loadcharset ", 12) || !Q_strncasecmp(cmd, "gl_consolefont ", 15) ||
			 !Q_strncasecmp(cmd, "crosshairimage ", 15))
	{
		param = "*.tga";
	}
	else if (!Q_strncasecmp(cmd, "gamedir ", 8) || !Q_strncasecmp(cmd, "game ", 5))
	{
		param = "*";
	}
	else if (!Q_strncasecmp(cmd, "toggle ", 7) || !Q_strncasecmp(cmd, "cycle ", 6) ||
			 !Q_strncasecmp(cmd, "inc ", 4) || !Q_strncasecmp(cmd, "dec ", 4) ||
			 !Q_strncasecmp(cmd, "create ", 7))
	{
		param = "";		// JDH
	}
	else
	{
		// command completion
		/*if (cl_advancedcompletion.value)
		{
			Cmd_CompleteCmdOrCvar (cmd, cycle_rev);
		}
		else
		{
			if (!(p = Cmd_CompleteCommand(cmd)))
				p = Cvar_CompleteVariable (cmd);

			if (p)
			{
				Q_strcpy (con_keylines[con_editline].text+1, p, sizeof(con_keylines[con_editline].text)-1);
				con_editpos = strlen(p) + 1;
				con_keylines[con_editline].text[con_editpos++] = ' ';
				con_keylines[con_editline].text[con_editpos] = 0;
			}
		}*/
		return;
	}

	Cmd_CompleteParameter (cmd, param, cycle_rev);
}
#endif

int Con_MapColoredChars (int key)
{
	if (keydown[K_CTRL] && !keydown[K_ALT])
	{
		if (key >= '0' && key <= '9')
			return key - '0' + 0x12;	// yellow number

		switch (key)
		{
			case '[': return 0x10;
			case ']': return 0x11;
			case '<': return 0x1d;
			case '-': return 0x1e;
			case '>': return 0x1f;

#ifdef HEXEN2_SUPPORT
		}

// in the Hexen II charset these symbols start at 0x100,
// which is useless for console since input is stored as bytes
		if (!hexen2)
		{
			switch (key)
#endif
			{
				case 'g': return 0x86;
				case 'r': return 0x87;
				case 'y': return 0x88;
				case 'b': return 0x89;
				case '(': return 0x80;
				case '=': return 0x81;
				case ')': return 0x82;
				case 'a': return 0x83;
				case '.': return 0x9c;
				case 'B': return 0x8b;
				case 'C': return 0x8d;
				case ',': return 0x1c;		// this one's blank in H2 charset
			}
		}
	}

#ifdef HEXEN2_SUPPORT
	if (!hexen2)
#endif
	if (keydown[K_ALT] && !keydown[K_CTRL])		// JDH: 2nd case excludes ALT-GR key
		return key | 0x80;		// red char

	return key;
}

#define CON_KEYLINE(n) ((n) & (MAXCMDLINES-1))

/*
====================
Con_CheckAutoComplete (JDH)
====================
*/
void Con_CheckAutoComplete (void)
{
	char *currline = con_keylines[con_editline].text;
	cmd_arglist_t	tokens;
	int i;
	
	// auto-complete only if cursor is at end of line, and at least 3 chars have been entered
	
	if (!con_autocomplete.value || (currline[con_editpos] != 0) || (con_editpos <= 3))
		return;
	
	if (con_autocomplete.value < 2)
	{
		// Spirit's request: don't auto-complete "say" commands
		Cmd_TokenizeString (currline+1, &tokens);
		if (!tokens.argc) return;
		
		if (!Q_strcasecmp(tokens.argv[0], "say"))
			return;
	}
	
	for (i = CON_KEYLINE(con_editline-1); i != con_editline; i = CON_KEYLINE(i-1))
	{
		if (!con_keylines[i].text[1])
			continue;		// 2010/06/05 - was break

		if (!Q_strncasecmp (con_keylines[i].text, currline, con_editpos))
		{
			if (strlen(con_keylines[i].text) > con_editpos)
			{
				Q_strcpy (currline, con_keylines[i].text, MAXCMDLINELEN);
				con_selstart = con_editpos;
			}
			break;
		}
	}
}

/*
====================
Con_Keydown

Interactive line editing and console scrollback
(JDH: was in keys.c as Key_Console)
====================
*/
void Con_Keydown (int key)
{
	int len;

	switch (key)
	{
		case K_ENTER:
			Cbuf_AddText (con_keylines[con_editline].text+1, SRC_CONSOLE);	// skip the "]"
			Cbuf_AddText ("\n", SRC_CONSOLE);
			Con_Printf ("%s\n", con_keylines[con_editline].text);
		// joe: don't save same consecutive commands multiple times
			if (strcmp(con_keylines[con_editline-1].text, con_keylines[con_editline].text))
				con_editline = CON_KEYLINE(con_editline + 1);
			con_histline = con_editline;
			con_keylines[con_editline].text[0] = ']';
			con_keylines[con_editline].text[1] = 0;
			con_editpos = 1;
			con_selstart = 0;
			if (cls.state == ca_disconnected)
				SCR_UpdateScreen ();	// force an update, because the command may take some time
			return;

		case K_LEFTARROW:
			con_selstart = 0;
			if (keydown[K_CTRL])
			{
				// word left
				while (con_editpos > 1 && con_keylines[con_editline].text[con_editpos-1] == ' ')
					con_editpos--;
				while (con_editpos > 1 && con_keylines[con_editline].text[con_editpos-1] != ' ')
					con_editpos--;
				return;
			}
			if (con_editpos > 1)
				con_editpos--;
			return;

		case K_RIGHTARROW:
			con_selstart = 0;
			len = strlen (con_keylines[con_editline].text);
			if (keydown[K_CTRL])
			{
				// word right
				while (con_editpos < len && con_keylines[con_editline].text[con_editpos] != ' ')
					con_editpos++;
				while (con_editpos < len && con_keylines[con_editline].text[con_editpos] == ' ')
					con_editpos++;
				return;
			}
			if (con_editpos < len)
				con_editpos++;
			return;

		case K_HOME:
			if (keydown[K_CTRL])
			{
				con_backscroll = con_numlines;
//				con_backscroll = con_numlines - ((int)scr_conlines / CON_LINEHEIGHT) + 2;
//				con_backscroll = max(con_backscroll, 0);
			}
			else
			{
				con_editpos = 1;
				con_selstart = 0;
			}
			return;

		case K_END:
			if (keydown[K_CTRL])
				con_backscroll = 0;
			else
			{
				con_editpos = strlen (con_keylines[con_editline].text);
				con_selstart = 0;
			}
			return;

		case K_INS:
			con_insert ^= 1;
			return;

		case K_PGUP:
		case K_MWHEELUP:
			if (con_numlines > ((int)scr_conlines / con_lineheight) - 2)
			{
				if (keydown[K_CTRL] && key == K_PGUP)
					con_backscroll += ((int)scr_conlines / con_lineheight) - 3;
				else
					con_backscroll += 2;
//				con_backscroll = min(con_backscroll, con_numlines - ((int)scr_conlines / CON_LINEHEIGHT) + 2);
			}
			else con_backscroll = 0;
			return;

		case K_PGDN:
		case K_MWHEELDOWN:
			if (keydown[K_CTRL] && key == K_PGDN)
				con_backscroll -= ((int)scr_conlines / con_lineheight) - 3;
			else
				con_backscroll -= 2;
//			con_backscroll = max(con_backscroll, 0);
			return;

		case K_TAB:
			CON_CLEARSEL();
			Con_Keydown_Tab ();
			return;

		case K_BACKSPACE:
			CON_CLEARSEL();
			if (con_editpos > 1)
			{
				Q_strcpy (con_keylines[con_editline].text + con_editpos - 1, con_keylines[con_editline].text + con_editpos,
							sizeof(con_keylines[con_editline].text) - con_editpos + 1);
				con_editpos--;
			}
			return;

		case K_DEL:
			CON_CLEARSEL();
			if (con_editpos < strlen(con_keylines[con_editline].text))
			{
				Q_strcpy (con_keylines[con_editline].text + con_editpos, con_keylines[con_editline].text + con_editpos + 1,
							sizeof(con_keylines[con_editline].text) - con_editpos);
			}
			return;

		case K_UPARROW:
			if (keydown[K_CTRL])
			{
				Con_AdjustHeight (-con_lineheight);
			}
			else
			{
				CON_CLEARSEL();
				do {
					con_histline = CON_KEYLINE(con_histline - 1);
				} while (con_histline != con_editline && !con_keylines[con_histline].text[1]);
				if (con_histline == con_editline)
					con_histline = CON_KEYLINE(con_editline + 1);

				Q_strcpy (con_keylines[con_editline].text, con_keylines[con_histline].text, sizeof(con_keylines[con_editline].text));
				con_editpos = strlen (con_keylines[con_editline].text);
			}
			return;

		case K_DOWNARROW:
			if (keydown[K_CTRL])
			{
				Con_AdjustHeight (con_lineheight);
			}
			else
			{
				if (con_histline != con_editline)
				{
					CON_CLEARSEL();
					do {
						con_histline = CON_KEYLINE(con_histline + 1);
					} while (con_histline != con_editline && !con_keylines[con_histline].text[1]);

					if (con_histline == con_editline)
					{
						con_keylines[con_editline].text[0] = ']';
						con_keylines[con_editline].text[1] = 0;
						con_editpos = 1;
					}
					else
					{
						Q_strcpy (con_keylines[con_editline].text, con_keylines[con_histline].text, sizeof(con_keylines[con_editline].text));
						con_editpos = strlen (con_keylines[con_editline].text);
					}
				}
			}
			return;
	}

	if ((keydown[K_CTRL] && !keydown[K_SHIFT] && !keydown[K_ALT] && (key == 'V' || key == 'v')))
	{
		char	*cliptext;

		if ((cliptext = Sys_GetClipboardData()))		// XWindows will always return NULL, since it's asynchronous
		{
			CON_CLEARSEL();
			Con_InsertText (cliptext);
		}
		return;
	}

	if (key < K_SPACE || key > K_BACKSPACE)
	{
//		Con_Printf ("control key %d\n", key);
		return;	// non printable
	}

	CON_CLEARSEL();

	len = strlen (con_keylines[con_editline].text);
	if (len >= MAXCMDLINELEN-1)
		return;

	key = Con_MapColoredChars (key);

	// This also moves the ending \0
	memmove (con_keylines[con_editline].text + con_editpos + 1, con_keylines[con_editline].text + con_editpos, len-con_editpos+1);
	con_keylines[con_editline].text[con_editpos] = key;
	con_editpos++;

	Con_CheckAutoComplete();
}

#endif		//#ifndef RQM_SV_ONLY
