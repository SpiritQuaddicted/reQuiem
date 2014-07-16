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

#define MAX_KEYS 256

// these are the key numbers that should be passed to Key_Event
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define	K_CAPSLOCK		153

// JDH: added next 13
#define K_NUMLOCK		154
#define K_SCRLOCK		155
#define K_PRINTSCR		156
#define K_LALT			157
#define K_RALT			158
#define K_LCTRL			159
#define K_RCTRL			160
#define K_LSHIFT		161
#define K_RSHIFT		162
#define K_WIN			163
#define K_LWIN			164
#define K_RWIN			165
#define K_MENU			166

// JDH: keyboard scancodes that don't map to Quake-recognized keys are
//    given the following names (assigned to the first 8 such scancodes)
#define K_EX1			167
#define K_EX2			168
#define K_EX3			169
#define K_EX4			170
#define K_EX5			171
#define K_EX6			172
#define K_EX7			173
#define K_EX8			174

#define K_PAUSE			255

// mouse buttons generate virtual keys
#define	K_MOUSE1		200
#define	K_MOUSE2		201
#define	K_MOUSE3		202

// joystick buttons
#define	K_JOY1			203
#define	K_JOY2			204
#define	K_JOY3			205
#define	K_JOY4			206

// aux keys are for multi-buttoned joysticks to generate so they can use
// the normal binding process
#define	K_AUX1			207
#define	K_AUX2			208
#define	K_AUX3			209
#define	K_AUX4			210
#define	K_AUX5			211
#define	K_AUX6			212
#define	K_AUX7			213
#define	K_AUX8			214
#define	K_AUX9			215
#define	K_AUX10			216
#define	K_AUX11			217
#define	K_AUX12			218
#define	K_AUX13			219
#define	K_AUX14			220
#define	K_AUX15			221
#define	K_AUX16			222
#define	K_AUX17			223
#define	K_AUX18			224
#define	K_AUX19			225
#define	K_AUX20			226
#define	K_AUX21			227
#define	K_AUX22			228
#define	K_AUX23			229
#define	K_AUX24			230
#define	K_AUX25			231
#define	K_AUX26			232
#define	K_AUX27			233
#define	K_AUX28			234
#define	K_AUX29			235
#define	K_AUX30			236
#define	K_AUX31			237
#define	K_AUX32			238

// JACK: Intellimouse(c) Mouse Wheel Support

#define K_MWHEELUP		239
#define K_MWHEELDOWN	240

#define MAX_EXKEYS		8			/* K_EX1 to K_EX8 */


#ifndef RQM_SV_ONLY

#define	MAXCMDLINELEN		256

typedef enum {key_game, key_console, key_message, key_menu} keydest_t;

extern keydest_t	key_dest;
extern	int		key_repeats[MAX_KEYS];
extern	int		key_count;			// incremented every key event
extern	int		key_lastpress;
extern qboolean keydown[256];

#define Key_Event(key, down) Key_Event2((key), 0, (down))
void Key_Event2 (int key, int keyshift, qboolean down);		// JDH: added 2nd key param
void Key_Init (void);
void Key_ClearStates (void);

#endif		//#ifndef RQM_SV_ONLY


extern char *keybindings[MAX_KEYS];

int  Key_WriteBindings (FILE *f);
void Key_SetBinding (int keynum, const char *binding);
//char * Key_SwapBinding (int keynum, char *binding);
void Key_Unbindall_f (cmd_source_t src);
void Key_ResetAll (void);
int Key_StringToKeynum (const char *str);
