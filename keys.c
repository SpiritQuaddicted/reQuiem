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
#include "quakedef.h"
/*

key up events are sent even if in console mode

*/

#ifndef RQM_SV_ONLY

int			key_lastpress;
int			key_count;		// incremented every key event

qboolean	key_forceUSlayout;		// JDH: true only if -uskbd is on command-line

qboolean	consolekeys[MAX_KEYS];		// if true, can't be rebound while in console
qboolean	menubound[MAX_KEYS];			// if true, can't be rebound while in Key Config menu
byte		key_shifttable[MAX_KEYS];	// key to map to if shift held down in console (JDH was int[])
int			key_repeats[MAX_KEYS];		// if > 1, it is autorepeating
qboolean	keydown[MAX_KEYS];
keydest_t	key_dest;

#endif		//#ifndef RQM_SV_ONLY


#define KEY_BINDING_EMPTY ((char *) -1)		// JDH: used by savedbindings[] to distinguish between a key
											//   whose binding hasn't changed from the saved one (NULL),
											//   and one that's changed from nothing to something
char		*keybindings[MAX_KEYS];
char		*savedbindings[MAX_KEYS];		// JDH: bindings loaded from cfg's (default/autoexec/config)
										//   (null until a key's binding is changed)

typedef struct
{
	int		keynum;
	char	*name;
} keyname_t;

keyname_t keynames[] =
{
	{K_TAB, "TAB"},
	{K_ENTER, "ENTER"},
	{K_ESCAPE, "ESCAPE"},
	{K_SPACE, "SPACE"},
	{K_BACKSPACE, "BACKSPACE"},

	{K_UPARROW, "UPARROW"},
	{K_DOWNARROW, "DOWNARROW"},
	{K_LEFTARROW, "LEFTARROW"},
	{K_RIGHTARROW, "RIGHTARROW"},

	{K_ALT, "ALT"},
	{K_CTRL, "CTRL"},
	{K_SHIFT, "SHIFT"},

	{K_F1, "F1"},
	{K_F2, "F2"},
	{K_F3, "F3"},
	{K_F4, "F4"},
	{K_F5, "F5"},
	{K_F6, "F6"},
	{K_F7, "F7"},
	{K_F8, "F8"},
	{K_F9, "F9"},
	{K_F10, "F10"},
	{K_F11, "F11"},
	{K_F12, "F12"},

	{K_INS, "INS"},
	{K_DEL, "DEL"},
	{K_PGDN, "PGDN"},
	{K_PGUP, "PGUP"},
	{K_HOME, "HOME"},
	{K_END, "END"},

	{K_CAPSLOCK, "CAPSLOCK"},
// the next two have alternate names - first one is what's used when writing cfg,
//    but all are accepted when loading cfg)
	{K_NUMLOCK, "NUMLOCK"}, {K_NUMLOCK, "KP_NUMLCK"}, {K_NUMLOCK, "KP_NUMLOCK"},
	{K_SCRLOCK, "SCRLOCK"}, {K_SCRLOCK, "SCRLCK"}, {K_SCRLOCK, "SCROLLOCK"},
	{K_PRINTSCR, "PRINTSCR"},

	{K_LALT, "LALT"},
	{K_RALT, "RALT"},
	{K_LCTRL, "LCTRL"},
	{K_RCTRL, "RCTRL"},
	{K_LSHIFT, "LSHIFT"},
	{K_RSHIFT, "RSHIFT"},
	{K_WIN, "WINKEY"},
	{K_LWIN, "LWINKEY"},
	{K_RWIN, "RWINKEY"},
	{K_MENU, "POPUPMENU"},

	{K_EX1, "EXKEY1"},
	{K_EX2, "EXKEY2"},
	{K_EX3, "EXKEY3"},
	{K_EX4, "EXKEY4"},
	{K_EX5, "EXKEY5"},
	{K_EX6, "EXKEY6"},
	{K_EX7, "EXKEY7"},
	{K_EX8, "EXKEY8"},

	{K_PAUSE, "PAUSE"},

	{K_MOUSE1, "MOUSE1"},
	{K_MOUSE2, "MOUSE2"},
	{K_MOUSE3, "MOUSE3"},

	{K_JOY1, "JOY1"},
	{K_JOY2, "JOY2"},
	{K_JOY3, "JOY3"},
	{K_JOY4, "JOY4"},

	{K_AUX1,  "AUX1"},
	{K_AUX2,  "AUX2"},
	{K_AUX3,  "AUX3"},
	{K_AUX4,  "AUX4"},
	{K_AUX5,  "AUX5"},
	{K_AUX6,  "AUX6"},
	{K_AUX7,  "AUX7"},
	{K_AUX8,  "AUX8"},
	{K_AUX9,  "AUX9"},
	{K_AUX10, "AUX10"},
	{K_AUX11, "AUX11"},
	{K_AUX12, "AUX12"},
	{K_AUX13, "AUX13"},
	{K_AUX14, "AUX14"},
	{K_AUX15, "AUX15"},
	{K_AUX16, "AUX16"},
	{K_AUX17, "AUX17"},
	{K_AUX18, "AUX18"},
	{K_AUX19, "AUX19"},
	{K_AUX20, "AUX20"},
	{K_AUX21, "AUX21"},
	{K_AUX22, "AUX22"},
	{K_AUX23, "AUX23"},
	{K_AUX24, "AUX24"},
	{K_AUX25, "AUX25"},
	{K_AUX26, "AUX26"},
	{K_AUX27, "AUX27"},
	{K_AUX28, "AUX28"},
	{K_AUX29, "AUX29"},
	{K_AUX30, "AUX30"},
	{K_AUX31, "AUX31"},
	{K_AUX32, "AUX32"},

	{K_MWHEELUP, "MWHEELUP"},
	{K_MWHEELDOWN, "MWHEELDOWN"},

	{';', "SEMICOLON"},	// because a raw semicolon seperates commands

	{0, NULL}
};

char key_symbolchars[] = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?";		// JDH: all Quake's "symbol" keys

// table to convert keyboard scan codes to Quake keys (used only if -uskbd)
byte key_fromcode[MAX_KEYS] =
{
//      0       1        2       3       4       5       6       7
//      8       9        A       B       C       D       E       F
	0  ,   K_ESCAPE,'1',    '2',    '3',     '4',    '5',      '6',
	'7',    '8',    '9',    '0',    '-',     '=', K_BACKSPACE, K_TAB,   // 0
	'q',    'w',    'e',    'r',    't',     'y',    'u',      'i',
	'o',    'p',    '[',    ']',   K_ENTER, K_LCTRL, 'a',      's',      // 1
	'd',    'f',    'g',    'h',    'j',     'k',    'l',      ';',
	'\'',   '`',  K_LSHIFT, '\\',   'z',     'x',    'c',      'v',      // 2
	'b',    'n',    'm',    ',',    '.',     '/',   K_RSHIFT,  '*',
	K_LALT, K_SPACE, K_CAPSLOCK,K_F1,  K_F2,   K_F3,   K_F4,    K_F5,     // 3
#ifdef _WIN32
	K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_PAUSE, K_SCRLOCK, K_HOME,
#else
	K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_NUMLOCK, K_SCRLOCK, K_HOME,
#endif
	K_UPARROW,  K_PGUP,  '-',   K_LEFTARROW,'5',K_RIGHTARROW,'+',  K_END,   // 4
	K_DOWNARROW,K_PGDN, K_INS,  K_DEL,       0,      0,       0,   K_F11,
#ifdef _WIN32
	K_F12,        0,      0,     K_LWIN,  K_RWIN,  K_MENU
#else
	K_F12,       K_HOME, K_UPARROW, K_PGUP, K_LEFTARROW,  0,   K_RIGHTARROW,  K_END,    // 5
	K_DOWNARROW, K_PGDN,  K_INS,    K_DEL,   K_ENTER,  K_RCTRL,  K_PAUSE,   K_PRINTSCR,
	'/',         K_RALT,   0,       K_LWIN,   K_RWIN,  K_MENU,      0,         0        // 6
#endif
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/
#ifndef RQM_SV_ONLY

const float cl_demospeed_presets[] = {0.1, 0.2, 0.4, 0.6, 0.8, 1.0, 1.5, 2, 3, 4, 6, 8, 10, 15, 20};
#define NUM_DEMOSPEED_PRESETS (sizeof(cl_demospeed_presets)/sizeof(float))

extern double scr_demo_overlay_time;
/*
================
Key_Demo (JDH)

called when a key is pressed during demo playback.  Returns true if key was handled.
  TODO: handle key repeat? (for demospeed only)
================
*/
qboolean Key_Demo (int key)
{
	int i;

	if ((key >= '0') && (key <= '9'))
	{
		CL_DemoSeek ((key - '0') / 10.0);
		goto KEY_DEMO_EXIT;
	}

	switch (key)
	{
		case K_ENTER:
		case K_MOUSE3:
			if (cl_demorewind.value)
				Cvar_SetDirect (&cl_demorewind, "0");
			if (cl_demospeed.value != 1.0)
				Cvar_SetDirect (&cl_demospeed, "1.0");
			break;

		case K_RIGHTARROW:
		case K_MOUSE2:
			if (cl_demorewind.value)
			{
				Cvar_SetDirect (&cl_demorewind, "0");
				//Con_Print ("Demo playback: forward\n");
			}
			break;

		case K_LEFTARROW:
		case K_MOUSE1:
			if (!cl_demorewind.value)
			{
				Cvar_SetDirect (&cl_demorewind, "1");
				//Con_Print ("Demo playback: reverse\n");
			}
			break;

		case K_UPARROW:
		case K_MWHEELUP:
			for (i = 0; i < NUM_DEMOSPEED_PRESETS; i++)
			{
				if (cl_demospeed_presets[i] > cl_demospeed.value)
					break;
			}

			i = min (i, NUM_DEMOSPEED_PRESETS-1);
			if (cl_demospeed.value != cl_demospeed_presets[i])
			{
				Cvar_SetDirect (&cl_demospeed, va("%.1f", cl_demospeed_presets[i]));
				//Con_Printf ("Demo speed: %s\n", cl_demospeed.string);
			}
			break;

		case K_DOWNARROW:
		case K_MWHEELDOWN:
			for (i = NUM_DEMOSPEED_PRESETS-1; i >= 0; i--)
			{
				if (cl_demospeed_presets[i] < cl_demospeed.value)
					break;
			}

			i = max (i, 0);
			if (cl_demospeed.value != cl_demospeed_presets[i])
			{
				Cvar_SetDirect (&cl_demospeed, va("%.1f", cl_demospeed_presets[i]));
				//Con_Printf ("Demo speed: %s\n", cl_demospeed.string);
			}
			break;

	/*	case K_F1:

			break;*/

		default:
			return false;
	}

KEY_DEMO_EXIT:
	scr_demo_overlay_time = realtime;
	return true;
}

//============================================================================

#define	MAX_CHAT_SIZE	45
char		chat_buffer[MAX_CHAT_SIZE];
qboolean	team_message = false;

void Key_Message (int key)
{
	static	int	chat_bufferlen = 0;

	if (key == K_ENTER)
	{
		if (team_message)
			Cbuf_AddText ("say_team \"", SRC_CONSOLE);
		else
			Cbuf_AddText ("say \"", SRC_CONSOLE);
		Cbuf_AddText (chat_buffer, SRC_CONSOLE);
		Cbuf_AddText ("\"\n", SRC_CONSOLE);

		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key == K_ESCAPE)
	{
		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
		{
			chat_bufferlen--;
			chat_buffer[chat_bufferlen] = 0;
		}
		return;
	}

	if (chat_bufferlen == MAX_CHAT_SIZE - (team_message ? 3 : 1))
		return;	// all full

	chat_buffer[chat_bufferlen++] = key;
	chat_buffer[chat_bufferlen] = 0;
}

#endif		//#ifndef RQM_SV_ONLY

//============================================================================

/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string. Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
int Key_StringToKeynum (const char *str)
{
	keyname_t	*kn;

	if (!str || !str[0])
		return -1;

	if (!str[1])
		return str[0];

	for (kn = keynames ; kn->keynum ; kn++)
	{
		if (!Q_strcasecmp(str, kn->name))
			return kn->keynum;
	}

// JDH: if key string has "K_" prefix, try search without the prefix
	if ((str[0] == 'K' || str[0] == 'k') && str[1] == '_')
	{
		str += 2;
		for (kn = keynames ; kn->keynum ; kn++)
		{
			if (!Q_strcasecmp(str, kn->name))
				return kn->keynum;
		}
	}

	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;
	static	char	tinystr[2];

	if (keynum == -1)
		return "<KEY NOT FOUND>";

	if ((keynum > K_SPACE) && (keynum < K_BACKSPACE))
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	for (kn = keynames ; kn->keynum ; kn++)
		if (keynum == kn->keynum)
			return kn->name;

	return "<UNKNOWN KEYNUM>";
}

/*
===================
Key_SetBinding_internal
  (assumes keynum is valid)
===================
*/
void Key_SetBinding_internal (int keynum, const char *binding)
{
	extern qboolean	config_exec, config_exec_rq;

	if (keybindings[keynum] && !strcmp(keybindings[keynum], binding))
		return;

/*	if (!Q_strcasecmp(binding, "toggleconsole"))
	{
		menubound[keynum] = true;
		consolekeys[keynum] = false;
	}
*/
// JDH: if this change occurs after execing quake.rc, store key's current binding to savedbindings
	if ((!config_exec || config_exec_rq) && !savedbindings[keynum])
	{
		if (keybindings[keynum])
		{
			savedbindings[keynum] = keybindings[keynum];
			keybindings[keynum] = NULL;
		}
		else savedbindings[keynum] = KEY_BINDING_EMPTY;
	}

// free old bindings
	if (keybindings[keynum])
	{
		Z_Free (keybindings[keynum]);
		keybindings[keynum] = NULL;
	}

	keybindings[keynum] = CopyString (binding);
}

/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, const char *binding)
{
//	int pairedkey, parentkey;

	if (keynum == -1)
		return;

/* REMOVED - too confusing for user

// JDH: this code checks if the right & left keys have the same binding.
//      If so, the binding is applied to the generic "parent" key.
	switch (keynum)
	{
		case K_LALT:   pairedkey = K_RALT;   parentkey = K_ALT;   break;
		case K_RALT:   pairedkey = K_LALT;   parentkey = K_ALT;   break;
		case K_LCTRL:  pairedkey = K_RCTRL;  parentkey = K_CTRL;  break;
		case K_RCTRL:  pairedkey = K_LCTRL;  parentkey = K_CTRL;  break;
		case K_LSHIFT: pairedkey = K_RSHIFT; parentkey = K_SHIFT; break;
		case K_RSHIFT: pairedkey = K_LSHIFT; parentkey = K_SHIFT; break;
		case K_LWIN:   pairedkey = K_RWIN;   parentkey = K_WIN;   break;
		case K_RWIN:   pairedkey = K_LWIN;   parentkey = K_WIN;   break;
		default:       pairedkey = 0;
	}

	if (pairedkey)
	{
		if (keybindings[pairedkey] && !Q_strcasecmp(keybindings[pairedkey], binding))
		{
			Key_SetBinding_internal (parentkey, binding);
			binding = "";
			Key_SetBinding_internal (pairedkey, binding);
		}
		else if (keybindings[parentkey] && !Q_strcasecmp(keybindings[parentkey], binding))
		{
		//  If the new binding is to the right or left key, and it's the same as
		//  the parent key's binding, the parent's binding is cleared.

			Key_SetBinding_internal (parentkey, "");
		}
	}
*/
	Key_SetBinding_internal (keynum, binding);
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (cmd_source_t src)
{
	int	b;

	if (Cmd_Argc() != 2)
	{
		Con_Print ("unbind <key> : remove commands from a key\n");
		return;
	}

	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b == -1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, "");
}

void Key_Unbindall_f (cmd_source_t src)
{
	int	i;

	for (i=0 ; i<MAX_KEYS ; i++)
		if (keybindings[i])
			Key_SetBinding (i, "");
}

/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (cmd_source_t src)
{
	int		i, c, b, len;
	char	cmd[1024];

	c = Cmd_Argc();

	if (c != 2 && c != 3)
	{
		Con_Print ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b == -1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Con_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b]);
		else
			Con_Printf ("\"%s\" is not bound\n", Cmd_Argv(1));
		return;
	}

// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	len = 0;
	for (i=2 ; i<c ; i++)
	{
		if (i > 2)
			len += Q_strcpy (cmd+len, " ", sizeof(cmd)-len);
		len += Q_strcpy (cmd+len, Cmd_Argv(i), sizeof(cmd)-len);
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_ResetAll (JDH)
  used when changing gamedir, before execing new cfgs
============
*/
void Key_ResetAll (void)
{
	int	i;

	for (i=0 ; i<MAX_KEYS ; i++)
	{
		if (keybindings[i])
		{
			Z_Free (keybindings[i]);
			keybindings[i] = NULL;
		}
		if (savedbindings[i])
		{
			if (savedbindings[i] != KEY_BINDING_EMPTY)
				Z_Free (savedbindings[i]);
			savedbindings[i] = NULL;
		}
	}
}

qboolean Key_BindingChanged (int key)
{
	if (keybindings[key] && *keybindings[key])
	{
		if ((savedbindings[key] == KEY_BINDING_EMPTY) || (savedbindings[key] && strcmp(keybindings[key], savedbindings[key])))
			return true;
	}
	else if (savedbindings[key] && (savedbindings[key] != KEY_BINDING_EMPTY))
		return true;

	return false;
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
  If file pointer is null, just returns count
============
*/
int Key_WriteBindings (FILE *f)
{
	int	i, count = 0;

// JDH: write a key binding only if it has changed from the binding originally loaded
	for (i=0 ; i<MAX_KEYS ; i++)
	{
		if (Key_BindingChanged(i))
		{
			count++;
			if (f)
			{
				if (count == 1)		// JDH: print section name only if there's at least 1 binding
					fprintf (f, "\n// Key bindings\n");

				fprintf (f, "bind \"%s\" \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
			}
		}
	}

	return count;
}

/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
#ifndef RQM_SV_ONLY

// joe: added stuff from [sons]Quake
	int		i;
//	char extras[] = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?";		// JDH: all Quake's "symbol" keys

//	Key_LoadHistory ();
	
	key_forceUSlayout = COM_CheckParm ("-uskbd") ? true: false;

// init ascii characters in console mode
	for (i=K_SPACE ; i<=K_BACKSPACE ; i++)
		consolekeys[i] = true;
// JDH: for non-US keyboards with keys not in the Quake charset
	for (i = K_EX1; i <= K_EX8; i++)
		consolekeys[i] = true;

	consolekeys[K_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_DEL] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_ALT] = true;
	consolekeys[K_LALT] = true;
	consolekeys[K_RALT] = true;
	consolekeys[K_CTRL] = true;
	consolekeys[K_LCTRL] = true;
	consolekeys[K_RCTRL] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_LSHIFT] = true;
	consolekeys[K_RSHIFT] = true;
	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;
//	consolekeys['`'] = false;
//	consolekeys['~'] = false;

// JDH: 2009/06/28 (for demo control)
	consolekeys[K_MOUSE1] = true;
	consolekeys[K_MOUSE2] = true;
	consolekeys[K_MOUSE3] = true;

#ifndef _WIN32
//	consolekeys[22] = true;		// JDH: on Linux, ctrl+v sends keypress of 22
#endif

//	IN_SetupKeyMap (extras);

// JDH: key_shifttable is now used only if -uskbd is specified when reQuiem is run
	for (i=0 ; i<MAX_KEYS ; i++)
		key_shifttable[i] = i;
	for (i='a' ; i<='z' ; i++)
		key_shifttable[i] = i - 'a' + 'A';
	key_shifttable['1'] = '!';
	key_shifttable['2'] = '@';
	key_shifttable['3'] = '#';
	key_shifttable['4'] = '$';
	key_shifttable['5'] = '%';
	key_shifttable['6'] = '^';
	key_shifttable['7'] = '&';
	key_shifttable['8'] = '*';
	key_shifttable['9'] = '(';
	key_shifttable['0'] = ')';
	key_shifttable['-'] = '_';
	key_shifttable['='] = '+';
	key_shifttable[','] = '<';
	key_shifttable['.'] = '>';
	key_shifttable['/'] = '?';
	key_shifttable[';'] = ':';
	key_shifttable['\''] = '"';
	key_shifttable['['] = '{';
	key_shifttable[']'] = '}';
	key_shifttable['`'] = '~';
	key_shifttable['\\'] = '|';

	menubound[K_ESCAPE] = true;
//	menubound['`'] = true;		// JDH
//	menubound['~'] = true;		// JDH

	for (i=0 ; i<12 ; i++)
		menubound[K_F1+i] = true;

#endif		//#ifndef RQM_SV_ONLY

// register our functions
	Cmd_AddCommand ("bind", Key_Bind_f, 0);
	Cmd_AddCommand ("unbind", Key_Unbind_f, 0);
	Cmd_AddCommand ("unbindall", Key_Unbindall_f, 0);
}

#ifndef RQM_SV_ONLY

/*qboolean Key_isSpecial (int key)
{
	if (key == K_INS || key == K_DEL || key == K_HOME ||
	    key == K_END || key == K_ALT || key == K_CTRL)
		return true;

	return false;
}
*/

#define ISTOGGLECONSOLE(kb) \
	(!Q_strncasecmp(kb, "toggleconsole", 13) && (!kb[13] || kb[13] == K_TAB || kb[13] == K_SPACE))

qboolean Key_RunBinding (int key, int keyshift)
{
	char	*kb, cmd[1024];

// JDH: added explicit check for toggleconsole so it'll work on all keyboard layouts
	kb = keybindings[key];
	if (kb && ISTOGGLECONSOLE(kb))
		goto RUNBINDING;

	if (keyshift)
	{
		kb = keybindings[keyshift];
		if (kb && ISTOGGLECONSOLE(kb))
			goto RUNBINDING;
	}

// if not a consolekey, send to the interpreter no matter what mode is
	if ((key_dest == key_menu && menubound[key]) ||
		(key_dest == key_console && !consolekeys[key]) ||
		(key_dest == key_game && (!con_forcedup || !consolekeys[key])))/* ||
		(Key_isSpecial(key) && keybindings[key] != NULL))*/
	{
		if ((kb = keybindings[key]))
			goto RUNBINDING;
		return true;
	}

	return false;

RUNBINDING:
	if (kb[0] == '+')
	{	// button commands add keynum as a parm
		sprintf (cmd, "%s %i\n", kb, key);
	}
	else
	{
		sprintf (cmd, "%s\n", kb);
	}
	Cbuf_AddText (cmd, SRC_COMMAND);
	return true;
}

typedef struct
{
	byte key;
	byte leftkey;
	byte rightkey;
} keygroup_t;

keygroup_t keygroups[] = 
{
	{K_ALT,   K_LALT,   K_RALT},
	{K_CTRL,  K_LCTRL,  K_RCTRL},
	{K_SHIFT, K_LSHIFT, K_RSHIFT},
	{K_WIN,   K_LWIN,   K_RWIN}
};

#define NUM_KEYGROUPS (sizeof(keygroups)/sizeof(keygroup_t))

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!

  JDH: if alt/ctrl/shift keys are down, and result in a printable character when combined
       with "key", the keyshift param will be that character.  Otherwise, it's 0.
	   (The keyshift param replaces the keyshift table that Quake originally used,
	   which didn't work too well on non-US keyboards)
===================
*/
void Key_Event2 (int key, int keyshift, qboolean down)
{
	char	*kb, cmd[1024];
	int		i;

#ifdef _DEBUG
	if (key == K_RCTRL && !down)
		kb = NULL;
#endif
	
// JDH: for non-US keyboards, it's possible that the lowercase key has no translation,
//      but shift+key does.  So we skip most of this if key = 0
	if (key)
	{
		keydown[key] = down;

		if (!down)
			key_repeats[key] = 0;

	// JDH: for SCR_ModalMessage - make sure pending keyup event doesn't count
		if (key_count == -3)
		{
			if (!down)
				return;
			key_count = -1;
		}

		key_lastpress = key;
		key_count++;
		if (key_count <= 0)
			return;		// just catching keys for Con_NotifyBox

	// update auto-repeat status
		if (down)
		{
			key_repeats[key]++;
			if (key_repeats[key] > 1)	// joe: modified to work as ZQuake
			{
				if ((key != K_BACKSPACE && key != K_DEL && key != K_LEFTARROW &&
					 key != K_RIGHTARROW && key != K_UPARROW && key != K_DOWNARROW &&
					 key != K_PGUP && key != K_PGDN && (key < K_SPACE || key >= K_BACKSPACE || key == '`')) ||
					(key_dest == key_game && cls.state == ca_connected))
					return;	// ignore most autorepeats
			}

		// joe: this was annoying, so I removed
		//	if (key >= 200 && !keybindings[key])
		//		Con_Printf ("%s is unbound, hit F4 to set.\n", Key_KeynumToString(key));
		}

#ifdef _DEBUG
//		Con_DPrintf ("Key_Event: key %s: $%02X (%s)\n", down ? "down" : "up", key, Key_KeynumToString(key));
#endif

	// JDH: first give menu a chance to handle it - needed when changing key bindings
		if ((key_dest == key_menu) && M_HandleKey(key, down))
			return;

	// handle escape specially, so the user can never unbind it
		if (key == K_ESCAPE)
		{
			if (!down)
				return;
			switch (key_dest)
			{
				case key_message:
					Key_Message (key);
					break;

				case key_menu:
				//	M_Keydown (key);
					break;

				case key_game:
				case key_console:
					M_ToggleMenu_f (SRC_COMMAND);
					break;

				default:
					Sys_Error ("Bad key_dest");
			}
			return;
		}

	// JDH: if the key is one of the L/R pairs, also send the generic keycode
	// (unless it's a keyup event and the other of the pair is still down)
		for (i = 0; i < NUM_KEYGROUPS; i++)
		{
			if (key == keygroups[i].leftkey)
			{
				if (down || !keydown[keygroups[i].rightkey])
					Key_Event (keygroups[i].key, down);
				break;
			}

			if (key == keygroups[i].rightkey)
			{
				if (down || !keydown[keygroups[i].leftkey])
					Key_Event (keygroups[i].key, down);
				break;
			}
		}
		
		/*if (key == K_LALT || key == K_RALT)
			Key_Event (K_ALT, down);
		else if (key == K_LCTRL || key == K_RCTRL)
			Key_Event (K_CTRL, down);
		else if (key == K_LSHIFT || key == K_RSHIFT)
			Key_Event (K_SHIFT, down);
		else if (key == K_LWIN || key == K_RWIN)
			Key_Event (K_WIN, down);*/

	// key up events only generate commands if the game key binding is
	// a button command (leading + sign). These will occur even in console mode,
	// to keep the character from continuing an action started before a console
	// switch. Button commands include the keynum as a parameter, so multiple
	// downs can be matched with ups
		if (!down)
		{
			kb = keybindings[key];
			if (kb && kb[0] == '+')
			{
				sprintf (cmd, "-%s %i\n", kb+1, key);
				Cbuf_AddText (cmd, SRC_COMMAND);
			}
			if (keyshift)
			{
				kb = keybindings[keyshift];
				if (kb && kb[0] == '+')
				{
					sprintf (cmd, "-%s %i\n", kb+1, key);
					Cbuf_AddText (cmd, SRC_COMMAND);
				}
			}
			return;
		}

	// during demo playback, most keys bring up the main menu
	// joe: no, thanks :)
		if (cls.demoplayback && down && consolekeys[key] && key_dest == key_game)
		{
	//		M_ToggleMenu_f ();
			if (Key_Demo (key))
				return;
		}

	#ifdef HEXEN2_SUPPORT
		if (hexen2 && (cl.intermission == 12) && down)
		{
			Cbuf_AddText ("map keep1\n", SRC_COMMAND);
		}
	#endif

		if (Key_RunBinding(key, keyshift))
			return;
	}

	if (!down)
		return;		// other systems only care about key down events

#ifdef _DEBUG
	if (key != K_SHIFT && key != K_LSHIFT && key != K_CTRL && key != K_LCTRL)
		key *= 1;
#endif
	
	if (keyshift)
	{
		key = keyshift;
#ifdef _DEBUG
//		Con_DPrintf ("Modifier key down - new key = %d ('%c')\n", key, key);
#endif
	}

	if (key)
	{
		switch (key_dest)
		{
			case key_message:
				Key_Message (key);
				break;

			case key_menu:
			//	M_Keydown (key);
				break;

			case key_game:
			case key_console:
				Con_Keydown (key);
				break;

			default:
				Sys_Error ("Bad key_dest");
		}
	}
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	for (i=0 ; i<MAX_KEYS ; i++)
	{
		// send an up event for each key, to make sure the server clears them all
		if (keydown[i])
			Key_Event (i, false);
//		keydown[i] = false;
		key_repeats[i] = 0;
	}
}

#endif		//#ifndef RQM_SV_ONLY

