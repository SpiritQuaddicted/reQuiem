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
// vid_glx.c -- Linux GLX driver

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
//#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>

#include "quakedef.h"

#include <GL/glx.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>		// JDH: for clipboard

#ifdef MACOSX
#  define Q_NODGA
#endif

//#define Q_NODGA			/* XF86DGADirectVideo causes key combos to quit working! */

#ifndef Q_NODGA
#  include <X11/extensions/xf86dga.h>
#endif

#include <X11/extensions/xf86vmode.h>

#define	WARP_WIDTH	320
#define	WARP_HEIGHT	200

#define X_CLIPATOM "reQuiemCLIP"

Display		*dpy = NULL;

Window		win;
Atom		vid_clipatom;
static	GLXContext	ctx = NULL;

static	float	mouse_x, mouse_y, old_mouse_x, old_mouse_y;
static	int		old_windowed_mouse = 0;		// JDH: used to be a float

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | PointerMotionMask)
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | FocusChangeMask)

unsigned	short	*currentgammaramp = NULL;
static unsigned	short	systemgammaramp[3][256];

qboolean vid_gammaworks = false;
qboolean vid_hwgamma_enabled = false;
qboolean customgamma = false;
qboolean fullsbardraw = false;
qboolean fullscreen = true;		// JDH: changed default to true (same as win32)

#ifndef Q_NODGA
// JDH: these are set in VID_Init, rather than checking every time install_grabs is called:
qboolean vid_nomdga = false;
qboolean vid_nokdga = false;
qboolean vid_nodga = false;
#endif

qboolean mouseactive = false;		// JDH: whether reQuiem is currently handling mouse events
Cursor	vid_nullcursor;

static	int	scr_width, scr_height, scrnum;

#ifndef Q_NODGA
static qboolean dgamouse = false, dgakeyb = false;
#endif

static	qboolean	vidmode_ext = false;
static	XF86VidModeModeInfo **vidmodes;
static	int			num_vidmodes;
//static	qboolean	vidmode_active = false;

static	int			vid_xerrornum;		// JDH: set by XErrorHandler if an error occurs
static	char		*vid_xerrorstr;

cvar_t	vid_mode = {"vid_mode","0"};
qboolean OnChange_windowed_mouse (cvar_t *var, const char *value);
cvar_t	_windowed_mouse = {"_windowed_mouse", "1", 0, OnChange_windowed_mouse};
cvar_t	m_filter = {"m_filter", "0"};
cvar_t	vid_hwgammacontrol = {"vid_hwgammacontrol", "1"};

//===========================================

/*
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}

void VID_LockBuffer (void)
{
}

void VID_UnlockBuffer (void)
{
}
*/
extern char		*key_symbolchars;
extern byte		key_fromcode[256];

static KeySym	keymap[256];		// keycodes to keysyms
static int		firstkeycode;

// keycodes that map to K_EX1-8 (assigned to first 8 keycodes that don't translate to a Quake key)
static byte		exkeys[MAX_EXKEYS];

/*
=================
XLateKey
  converts KeySym to Quake keynum
=================
*/
static int XLateKey (KeySym keysym, unsigned state)
{
	if ((keysym >= XK_F1) && (keysym <= XK_F12))
		return K_F1 + (keysym-XK_F1);

//XK_KP_0-9, XK_Clear ???

	switch (keysym)
	{
	case XK_ISO_Left_Tab:
	case XK_Tab:
		return K_TAB;

	case XK_KP_Enter:
	case XK_Return:
		return K_ENTER;

	case XK_Escape:
		return K_ESCAPE;

	case XK_BackSpace:
		return K_BACKSPACE;

	case XK_KP_Up:
		if (state & Mod2Mask)		// numlock is ON
			return '8';
	case XK_Up:
		return K_UPARROW;

	case XK_KP_Down:
		if (state & Mod2Mask)
			return '2';
	case XK_Down:
		return K_DOWNARROW;

	case XK_KP_Left:
		if (state & Mod2Mask)
			return '4';
	case XK_Left:
		return K_LEFTARROW;

	case XK_KP_Right:
		if (state & Mod2Mask)
			return '6';
	case XK_Right:
		return K_RIGHTARROW;

	case XK_Alt_L:
		return K_LALT;
	case XK_Alt_R:
		return K_RALT;

	case XK_Execute:
	case XK_ISO_Level3_Shift:
		return K_CTRL;
	case XK_Control_L:
		return K_LCTRL;
	case XK_Control_R:
		return K_RCTRL;

	case XK_Shift_L:
		return K_LSHIFT;
	case XK_Shift_R:
		return K_RSHIFT;

	case XK_KP_Insert:
		if (state & Mod2Mask)
			return '0';
	case XK_Insert:
		return K_INS;

	case XK_KP_Delete:
		if (state & Mod2Mask)
			return '.';
	case XK_Delete:
		return K_DEL;

	case XK_KP_Page_Up:
		if (state & Mod2Mask)
			return '9';
	case XK_Page_Up:
		return K_PGUP;

	case XK_KP_Page_Down:
		if (state & Mod2Mask)
			return '3';
	case XK_Page_Down:
		return K_PGDN;

	case XK_KP_Home:
		if (state & Mod2Mask)
			return '7';
	case XK_Home:
		return K_HOME;

	case XK_KP_End:
		if (state & Mod2Mask)
			return '1';
	case XK_End:
		return K_END;

	case XK_KP_Begin:
		return '5';

	case XK_Caps_Lock:
		return K_CAPSLOCK;

	case XK_Num_Lock:
		return K_NUMLOCK;

	case XK_Scroll_Lock:
		return K_SCRLOCK;

	case XK_Sys_Req:
		return K_PRINTSCR;

	case XK_Super_L:
		return K_LWIN;
	case XK_Super_R:
		return K_RWIN;

	case XK_Menu:
		return K_MENU;

	case XK_Pause:
		return K_PAUSE;

//	case XK_KP_Begin:
	//	return K_AUX30;

	case XK_KP_Multiply:
		return '*';

	case XK_KP_Add:
		return '+';

	case XK_KP_Subtract:
		return '-';

	case XK_KP_Divide:
		return '/';

	case XK_dead_circumflex:
		return '^';

	case XK_dead_grave:
		return '`';

/*	case XK_dead_tilde:
		return '~';
*/
	default:
//		Con_Printf ("XLateKey: unknown key %d\n", keysym);
		if (!(keysym & 0xFF00) && (keysym >= K_SPACE) && (keysym < K_BACKSPACE))
		{
			// 2009-07-24 - removed this - it caused shift+letter to remain lowercase (what was the point anyway?)
			//if (keysym >= 'A' && keysym <= 'Z')
			//	keysym += 'a' - 'A';
			return keysym;
		}
	}

	return 0;
}

void IN_SetupKeyMap (void)		// **requires dpy to be initialized**
{
	int		maxcode, syms_per_code, numexkeys, i, qkey;
	KeySym *keys;
#ifdef _DEBUG
//	int j;
//	FILE *f = fopen("./keymapDE.txt", "w");
#endif

	XDisplayKeycodes (dpy, &firstkeycode, &maxcode);

	keys = XGetKeyboardMapping (dpy, firstkeycode, maxcode-firstkeycode+1, &syms_per_code);
	numexkeys = 0;

	for (i = firstkeycode; i <= maxcode; i++)
	{
		keymap[i] = keys[(i-firstkeycode)*syms_per_code];

		if (keymap[i] && (numexkeys < MAX_EXKEYS))
		{
		// check whether keysym maps to a quake key.  If not, assign keycode to next available K_EXn
			qkey = XLateKey (keymap[i], 0);
			if (!qkey)
				exkeys[numexkeys++] = i;
		}

#ifdef _DEBUG
//		fprintf (f, "%03d:", i);
		//for (j = 0; j < syms_per_code /*&& keys[(i-firstkeycode)*syms_per_code + j]*/; j++)
		//	fprintf (f, "   0x%04X", (int)keys[(i-firstkeycode)*syms_per_code + j]);

//		fprintf (f, "   0x%04X (%s)", (int)keymap[i], Key_KeynumToString(XLateKey(keymap[i], 0)));
//		fprintf (f, "\n");
#endif
	}

#ifdef _DEBUG
//	fclose (f);
#endif
}

int CodeToExKey (unsigned code)
{
	int i;

	// if a keycode can't be translated to any standard Quake keynum, see if it
	// corresponds to one of the K_EXn keys
	for (i = 0; i < MAX_EXKEYS; i++)
	{
		if (code == exkeys[i])
			return K_EX1 + i;
	}

	return 0;
}

extern byte		key_shifttable[256];
extern qboolean	key_forceUSlayout;

static int IN_MapKey (XKeyEvent *ev, int *keyshift)
{
	int		key;
	byte	buf[64];
	KeySym	keysym;

	*keyshift = 0;

	if (key_forceUSlayout)
	{
		key = key_fromcode[ev->keycode-firstkeycode];
		if (key && keydown[K_SHIFT])
			*keyshift = key_shifttable[key];
#ifdef _DEBUG
//		if (ev->type == KeyPress)
//			Con_DPrintf ("keycode = %d (0x%02X) --> %s\n", ev->keycode, ev->keycode, Key_KeynumToString(key));
#endif
		return key;
	}

#ifdef _DEBUG
//		Con_DPrintf ("key%s: code=0x%02X, time = %d, serial = %08X (keys down: %d)\n", (ev->type == KeyPress ? "down" : "up  "),
//						ev->keycode, ev->time, ev->serial, keydowncount);
//		if (ev->type == KeyPress)
//			Con_DPrintf ("keydown: code=0x%02X, window=%08X, root=%08X, subwin=%08X, same_screen = %d\n",
//							ev->keycode, ev->window, ev->root, ev->subwindow, ev->same_screen);
#endif

	if (keydown[K_SHIFT] || keydown[K_CTRL] || keydown[K_ALT] ||
		(ev->state & (ShiftMask | ControlMask | Mod1Mask | Mod5Mask | LockMask)))		// Mod1=alt, Mod5=altgr, Lock=capslock
	{
		int count = XLookupString (ev, (char *)buf, sizeof(buf), &keysym, 0);

		*keyshift = XLateKey (keysym, ev->state);
	#ifdef _DEBUG
		//if (ev->type == KeyPress)
		//	Con_DPrintf ("XLateKey : lookup on \"%s\" (%d) returned $%04X (XLate to %d [$%04X])\n",
		//		buf, *buf, keysym, *keyshift, *keyshift);
	#endif

		if (!*keyshift && count && (buf[0] > K_SPACE) && (buf[0] < K_BACKSPACE))
			*keyshift = buf[0];
	}

//	if (key && keydown[K_SHIFT])
//		*keyshift = key_shifttable[key];		/**** FIX THIS!!! ****/

	if (ev->keycode > 255)
		return 0;

	key = XLateKey (keymap[ev->keycode], ev->state);
	if (!key)
	{
		key = CodeToExKey (ev->keycode);
	#ifdef _DEBUG
//		Con_DPrintf ("CodeToExKey: mapped for 0x%02X to %d\n", ev->keycode, key);
	#endif
	}


#ifdef _DEBUG
//	if (ev->type == KeyPress)
//		Con_DPrintf ("IN_MapKey: keycode 0x%04X --> 0x%04x --> %d [%c] (mods = 0x%04X)\n",
//					ev->keycode, keymap[ev->keycode], key, key, ev->state);
#endif

	return key;
}
/*
static void uninstall_mousegrab (void)
{
	if (dgamouse)
	{
		XF86DGADirectVideo (dpy, DefaultScreen(dpy), (dgakeyb ? XF86DGADirectKeyb : 0));
		dgamouse = false;
	}

	mouseactive = false;
	XUngrabPointer (dpy, CurrentTime);
	XDefineCursor (dpy, win, None);
}

static void install_grabs (void)
{
	int	DGAflags = 0;

	XGrabPointer (dpy, win, True, 0, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
	mouseactive = true;
	XDefineCursor (dpy, win, vid_nullcursor);

	if (!vid_nomdga)
		DGAflags |= XF86DGADirectMouse;
	if (!vid_nokdga)
		DGAflags |= XF86DGADirectKeyb;

	if (!vid_nodga && DGAflags)
	{
		XF86DGADirectVideo (dpy, DefaultScreen(dpy), DGAflags);
		if (!vid_nomdga)
			dgamouse = true;
		if (!vid_nokdga)
			dgakeyb = true;
	}
	else XWarpPointer (dpy, None, win, 0, 0, 0, 0, vid.width / 2, vid.height / 2);

	XGrabKeyboard (dpy, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
}

static void uninstall_grabs (void)
{
	if (!dgamouse && dgakeyb)
	{
		XF86DGADirectVideo (dpy, DefaultScreen(dpy), 0);
	}

	dgakeyb = false;
	uninstall_mousegrab ();
	XUngrabKeyboard (dpy, CurrentTime);
}
*/
static void install_grabs (void)
{
#ifndef Q_NODGA
	int	DGAflags = 0;
#endif

	XGrabPointer (dpy, win, True, 0, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
	mouseactive = true;
	XDefineCursor (dpy, win, vid_nullcursor);

#ifndef Q_NODGA
	if (!vid_nomdga)
		DGAflags |= XF86DGADirectMouse;
//	if (!vid_nokdga)
//		DGAflags |= XF86DGADirectKeyb;		// 2009/08/07 - removed; requires different keyboard translation code

	if (!vid_nodga && DGAflags)
	{
		XF86DGADirectVideo (dpy, scrnum, DGAflags);
		if (DGAflags & XF86DGADirectMouse)
			dgamouse = true;
		if (DGAflags & XF86DGADirectKeyb)
			dgakeyb = true;
	}
	else
#endif
	{
		XWarpPointer (dpy, None, win, 0, 0, 0, 0, vid.width / 2, vid.height / 2);
	}

	XGrabKeyboard (dpy, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
//	XGrabKeyboard (dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
}

static void uninstall_grabs (void)
{
#ifndef Q_NODGA
	if (dgamouse || dgakeyb)
	{
		XF86DGADirectVideo (dpy, scrnum, 0);
		dgamouse = dgakeyb = false;
	}
#endif

	mouseactive = false;
	XUngrabPointer (dpy, CurrentTime);
	XDefineCursor (dpy, win, None);

	XUngrabKeyboard (dpy, CurrentTime);
}

qboolean OnChange_windowed_mouse (cvar_t *var, const char *value)
{
	if (fullscreen/*vidmode_active*/ && !Q_atof(value))
	{
		Con_Printf ("Cannot turn %s off when using fullscreen mode\n", var->name);
		return true;
	}

	return false;
}

void XPasteSelection (XSelectionEvent *sel)			// JDH
{
	Atom type;
	int format, result;
	unsigned long numitems, len;
	byte *data;

	if ((key_dest != key_console) && ((key_dest != key_game) || !con_forcedup))
		return;

//	Con_DPrintf ("XPasteSelection called\n");

	if (sel->property == None)
		return;

	result = XGetWindowProperty (dpy, win, sel->property, 0, MAXCMDLINELEN/4, False, AnyPropertyType,
									&type, &format, &numitems, &len, &data);
	if ((result == Success) && (type == XA_STRING) && (format == 8) && numitems && data)
	{
		//Con_Printf ("XGetWindowProperty: numitems=%d, len=%d, data=%s\n", numitems, len, data);
		Con_InsertText ((char *)data);
		XFree (data);
	}
//	else Con_DPrintf ("XPasteSelection: bad format\n");
}


static void GetEvent (void)
{
	XEvent	event;
	int		key, keyshift;
	static int keydowncount = 0;

	if (!dpy)
		return;

	XNextEvent (dpy, &event);

	switch (event.type)
	{
	case KeyPress:
#ifdef _DEBUG
//		Con_Printf ("Keydown: %d\n", event.xkey.keycode);
#endif
		keydowncount += 2;		// since it follows through to decrement
	case KeyRelease:
		keydowncount--;
		key = IN_MapKey (&event.xkey, &keyshift);
		Key_Event2 (key, keyshift, (event.type == KeyPress));
		if (keymap[event.xkey.keycode] == XK_ISO_Level3_Shift)
			Key_Event2 (K_RALT, 0, (event.type == KeyPress));		// ALT-GR sends CTRL+RALT
		break;

	case MotionNotify:
		if (_windowed_mouse.value && mouseactive)
//		if (_windowed_mouse.value)
		{
#ifndef Q_NODGA
			if (dgamouse)
			{
				mouse_x += event.xmotion.x_root;
				mouse_y += event.xmotion.y_root;
			}
			else
#endif
			{
				mouse_x = ((int)event.xmotion.x - (int)(vid.width / 2));
				mouse_y = ((int)event.xmotion.y - (int)(vid.height / 2));

				// move the mouse to the window center again
				XSelectInput (dpy, win, X_MASK & ~PointerMotionMask);
				XWarpPointer (dpy, None, win, 0, 0, 0, 0, (vid.width / 2), (vid.height / 2));
				XSelectInput (dpy, win, X_MASK);
			}
		}
		break;

	case ButtonPress:
	case ButtonRelease:
//		if (mouseactive)		//removed 2009/06/18
		{
			switch (event.xbutton.button)
			{
			case 1:
				Key_Event (K_MOUSE1, event.type == ButtonPress);
				break;

			case 2:
				Key_Event (K_MOUSE3, event.type == ButtonPress);
				break;

			case 3:
				Key_Event (K_MOUSE2, event.type == ButtonPress);
				break;

			case 4:
				Key_Event (K_MWHEELUP, event.type == ButtonPress);
				break;

			case 5:
				Key_Event (K_MWHEELDOWN, event.type == ButtonPress);
				break;
			}
		}
		break;

	case FocusIn:
		Key_ClearStates ();
		keydowncount = 0;
		break;

	case SelectionNotify:
		XPasteSelection (&event.xselection);
		break;
	}

	if (keydowncount)		// this seems to fix stuck key when grabbing keyboard
		return;

//	if (event.type == KeyPress)
//		return;			// reduce chance of "stuck" key when grabbing keyboard

/*	if (old_windowed_mouse != _windowed_mouse.value)
	{
		old_windowed_mouse = _windowed_mouse.value;

		if (!_windowed_mouse.value)
			uninstall_grabs ();
		else
			install_grabs ();
	}
*/

// JDH: changed this to mirror Windows version
//      (when _windowed_mouse is 1, mouse is grabbed only when playing map)

	if (!fullscreen)
	{
		if (!_windowed_mouse.value)
		{
			if (old_windowed_mouse)
			{
//				Con_DPrintf ("releasing mouse - _windowed_mouse changed\n");
				uninstall_grabs ();
//				uninstall_mousegrab ();
				old_windowed_mouse = 0;
			}
		}
		else
		{
			if (old_windowed_mouse != (int)_windowed_mouse.value)
			{
				install_grabs ();
				old_windowed_mouse = (int)_windowed_mouse.value;
			}

			if (old_windowed_mouse == 1)
			{
				if (mouseactive && !GAME_ACTIVE())
				{
//					Con_DPrintf ("releasing mouse - game not active\n");
//					uninstall_mousegrab ();
					uninstall_grabs ();
				}
				else if (!mouseactive && GAME_ACTIVE())
				{
//					Con_DPrintf ("grabbing mouse - game active\n");
					install_grabs ();
				}
			}
		}
	}
}

void signal_handler (int sig)
{
	printf ("Received signal %d, exiting...\n", sig);
	Sys_Quit ();
	exit (0);
}

void InitSig (void)
{
	signal (SIGHUP, signal_handler);
	signal (SIGINT, signal_handler);
	signal (SIGQUIT, signal_handler);
	signal (SIGILL, signal_handler);
	signal (SIGTRAP, signal_handler);
	signal (SIGIOT, signal_handler);
	signal (SIGBUS, signal_handler);
	signal (SIGFPE, signal_handler);
	signal (SIGSEGV, signal_handler);
	signal (SIGTERM, signal_handler);
}

void VID_ShiftPalette (unsigned char *p)
{
}

void InitHWGamma (void)
{
	int	xf86vm_gammaramp_size;

	if (COM_CheckParm("-nohwgamma"))
		return;

	XF86VidModeGetGammaRampSize (dpy, scrnum, &xf86vm_gammaramp_size);

	vid_gammaworks = (xf86vm_gammaramp_size == 256);
	if (vid_gammaworks)
		XF86VidModeGetGammaRamp (dpy, scrnum, xf86vm_gammaramp_size, systemgammaramp[0], systemgammaramp[1], systemgammaramp[2]);
}

void VID_SetDeviceGammaRamp (unsigned short *ramps)
{
	if (vid_gammaworks)
	{
		currentgammaramp = ramps;
		if (vid_hwgamma_enabled)
		{
			XF86VidModeSetGammaRamp (dpy, scrnum, 256, ramps, ramps + 256, ramps + 512);
			customgamma = true;
		}
	}
}

void RestoreHWGamma (void)
{
	if (vid_gammaworks && customgamma)
	{
		customgamma = false;
		XF86VidModeSetGammaRamp (dpy, scrnum, 256, systemgammaramp[0], systemgammaramp[1], systemgammaramp[2]);
	}
}

/*
=================
GL_BeginRendering
=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;

	vid_hwgamma_enabled = vid_hwgammacontrol.value && vid_gammaworks;
	vid_hwgamma_enabled = vid_hwgamma_enabled && (fullscreen || vid_hwgammacontrol.value == 2);
}

/*
=================
GL_EndRendering
=================
*/
void GL_EndRendering (void)
{
	static qboolean old_hwgamma_enabled;

//	vid_hwgamma_enabled = vid_hwgammacontrol.value && vid_gammaworks;
//	vid_hwgamma_enabled = vid_hwgamma_enabled && (fullscreen || vid_hwgammacontrol.value == 2);
	if (vid_hwgamma_enabled != old_hwgamma_enabled)
	{
		old_hwgamma_enabled = vid_hwgamma_enabled;
		if (vid_hwgamma_enabled && currentgammaramp)
			VID_SetDeviceGammaRamp (currentgammaramp);
		else
			RestoreHWGamma ();
	}

	glFlush ();
	glXSwapBuffers (dpy, win);

	if (fullsbardraw)
		Sbar_Changed ();
}

void VID_Shutdown (void)
{
	if (!ctx)
		return;

//	printf ("VID_Shutdown is happening!\n");

	uninstall_grabs ();

	RestoreHWGamma ();

	if (dpy)
	{
		glXDestroyContext (dpy, ctx);
		if (win)
			XDestroyWindow (dpy, win);
		if (fullscreen/*vidmode_active*/)
			XF86VidModeSwitchToMode (dpy, scrnum, vidmodes[0]);
		XCloseDisplay (dpy);
		fullscreen/*vidmode_active*/ = false;
	}
}

static Cursor CreateNullCursor (Display *display, Window root)
{
	Pixmap		cursormask;
	XGCValues	xgc;
	GC		gc;
	XColor		dummycolour;
	Cursor		cursor;

	cursormask = XCreatePixmap (display, root, 1, 1, 1);
	xgc.function = GXclear;
	gc = XCreateGC (display, cursormask, GCFunction, &xgc);
	XFillRectangle (display, cursormask, gc, 0, 0, 1, 1);
	dummycolour.pixel = 0;
	dummycolour.red = 0;
	dummycolour.flags = 04;
	cursor = XCreatePixmapCursor (display, cursormask, cursormask, &dummycolour, &dummycolour, 0, 0);
	XFreePixmap (display, cursormask);
	XFreeGC (display, gc);

	return cursor;
}

int VID_XErrorHandler (Display *dpy, XErrorEvent *error)
{
	vid_xerrornum = error->error_code;
	return 0;
}
void VID_XErrorHandler_Start (void)
{
	vid_xerrornum = 0;
	vid_xerrorstr = NULL;
	XSynchronize (dpy, True);		// so error is handled immediately
	XSetErrorHandler (VID_XErrorHandler);
}

void VID_XErrorHandler_End (void)
{
	static char errstr[256];

	XSetErrorHandler (NULL);
	XSynchronize (dpy, False);

	if (vid_xerrornum)
	{
		XGetErrorText (dpy, vid_xerrornum, errstr, sizeof(errstr));
		vid_xerrorstr = errstr;
	}
}

void VID_Init (unsigned char *palette)
{
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};
//	int		i, width = 640, height = 480;
	int		i, width = 800, height = 600;
	XSetWindowAttributes attr;
	unsigned long	mask;
	Window		root;
	XVisualInfo	*visinfo;
	int		MajorVersion, MinorVersion/*, actualWidth, actualHeight*/;

	Cvar_Register (&vid_mode);
	Cvar_RegisterInt (&_windowed_mouse, 0, 2);
//	Cvar_RegisterInt (&_windowed_mouse, 0, 1);
	Cvar_RegisterBool (&m_filter);
	Cvar_RegisterInt (&vid_hwgammacontrol, 0, 2);

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
//	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	if (!(dpy = XOpenDisplay(NULL)))
		Sys_Error ("Error: couldn't open the X display");

	scrnum = DefaultScreen (dpy);		// 2009.08.25 - moved up from below
	if (!(visinfo = glXChooseVisual(dpy, scrnum, attrib)))
		Sys_Error ("Error: couldn't get an RGB, Double-buffered, Depth visual");

	IN_SetupKeyMap ();		// JDH: *after* XOpenDisplay()

//	scrnum = DefaultScreen (dpy);
	root = RootWindow (dpy, scrnum);

//	if (COM_CheckParm("-fullscreen"))
//		fullscreen = true;

	if (COM_CheckParm("-window"))
		fullscreen = false;

#ifndef Q_NODGA
	if (COM_CheckParm("-nomdga"))
		vid_nomdga = true;
	if (COM_CheckParm("-nokdga"))
		vid_nokdga = true;
	if (COM_CheckParm("-nodga"))
		vid_nodga = true;
#endif

	// set vid parameters
	if (COM_CheckParm("-current"))
	{
		width = DisplayWidth (dpy, scrnum);
		height = DisplayHeight (dpy, scrnum);
	}
	else
	{
		if ((i = COM_CheckParm("-width")) && i + 1 < com_argc)
			width = Q_atoi(com_argv[i+1]);

		if ((i = COM_CheckParm("-height")) && i + 1 < com_argc)
			height = Q_atoi(com_argv[i+1]);
		else
			height = width*0.75;
	}

	// 2010/04/11: moved conwidth/conheight calcs lower

	// Get video mode list
	MajorVersion = MinorVersion = 0;
	if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion))
	{
		vidmode_ext = false;
	}
	else
	{
		Con_Printf ("Using XFree86-VidModeExtension Version %d.%d\n", MajorVersion, MinorVersion);
		vidmode_ext = true;
	}

	if (!vidmode_ext)
		fullscreen = false;

	// Are we going fullscreen? If so, let's change video mode
	if (fullscreen)
	{
		int	best_fit, best_dist, dist, x, y;
		//XF86VidModeMonitor vidmon;

		fullscreen = false;		// assume the worst
		XF86VidModeGetAllModeLines (dpy, scrnum, &num_vidmodes, &vidmodes);
/*
		for (i=0 ; i<num_vidmodes ; i++)
		{
			Con_Printf ("mode %d: %dx%d $%08X %d-%d\n", i, vidmodes[i]->hdisplay, vidmodes[i]->vdisplay,
				vidmodes[i]->flags, vidmodes[i]->vsyncstart, vidmodes[i]->vsyncend);
		}
		XF86VidModeGetMonitor (dpy, scrnum, &vidmon);
		Con_Printf ("monitor: %d vsyncs (0: %d-%dHz)\n", (int)vidmon.nvsync, (int)vidmon.vsync[0].lo, (int)vidmon.vsync[0].hi);
*/
		best_dist = 9999999;
		best_fit = -1;

		for (i=0 ; i<num_vidmodes ; i++)
		{
			if (width > vidmodes[i]->hdisplay || height > vidmodes[i]->vdisplay)
				continue;

			x = width - vidmodes[i]->hdisplay;
			y = height - vidmodes[i]->vdisplay;
			dist = (x * x) + (y * y);
			if (dist < best_dist)
			{
				best_dist = dist;
				best_fit = i;
			}
		}

		if (best_fit != -1)
		{
			width = vidmodes[best_fit]->hdisplay;		// was actualWidth =
			height = vidmodes[best_fit]->vdisplay;		// was actualHeight =

			// change to the mode
			VID_XErrorHandler_Start ();
			XF86VidModeSwitchToMode (dpy, scrnum, vidmodes[best_fit]);
			VID_XErrorHandler_End ();
	//		vidmode_active = true;

			if (vid_xerrorstr)
			{
				Con_Printf ("\x02""Warning: unable to set fullscreen mode %d (%dx%d)\n", best_fit,
								vidmodes[best_fit]->hdisplay, vidmodes[best_fit]->vdisplay);
			}
			else
			{
				// Move the viewport to top left
				XF86VidModeSetViewPort (dpy, scrnum, 0, 0);
				fullscreen = true;
			}
		}
	}

	if ((i = COM_CheckParm("-conwidth")) && i + 1 < com_argc)
		vid.conwidth = Q_atoi(com_argv[i+1]);
	else
		vid.conwidth = width;		// JDH: was 640

	vid.conwidth &= 0xfff8;	// make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	// JDH: get aspect ratio from current resolution
//	vid.conheight = vid.conwidth * 3 / 4;
	vid.conheight = vid.conwidth * height / width;

	if ((i = COM_CheckParm("-conheight")) && i + 1 < com_argc)
		vid.conheight = Q_atoi(com_argv[i+1]);
	if (vid.conheight < 200)
		vid.conheight = 200;

	// window attributes
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap (dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = X_MASK;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	if (fullscreen/*vidmode_active*/)
	{
//		mask = CWBackPixel | CWColormap | CWEventMask | CWSaveUnder | CWBackingStore | CWOverrideRedirect;
		mask |= CWSaveUnder | CWBackingStore | CWOverrideRedirect;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
	}

	VID_XErrorHandler_Start ();		// so error in XCreateWindow is handled immediately
	win = XCreateWindow (dpy, root, 0, 0, width, height, 0, visinfo->depth, InputOutput, visinfo->visual, mask, &attr);
	VID_XErrorHandler_End ();

#define REORDER20100607
	if (vid_xerrorstr)
	{
		if (fullscreen/*vidmode_active*/)
			XF86VidModeSwitchToMode (dpy, scrnum, vidmodes[0]);		// JDH: graceful exit if fullscreen fails
		XCloseDisplay (dpy);
		Sys_Error ("XCreateWindow returned %s", vid_xerrorstr);
	}

	vid_nullcursor = CreateNullCursor (dpy, win);
//	XDefineCursor (dpy, win, vid_nullcursor);
#ifndef REORDER20100607
	XMapWindow (dpy, win);
#endif
	XStoreName (dpy, win, "reQuiem");		// JDH

#ifndef REORDER20100607
	if (fullscreen)	//vidmode_active
	{
		XMoveWindow (dpy, win, 0, 0);
		XRaiseWindow (dpy, win);
		XWarpPointer (dpy, None, win, 0, 0, 0, 0, 0, 0);
	//	XFlush (dpy);
		// Move the viewport to top left
	//	XF86VidModeSetViewPort (dpy, scrnum, 0, 0);
	}
	else	// JDH: center window
	{
		XWindowChanges wc;
		wc.x = (DisplayWidth (dpy, scrnum) - width) / 2;
		wc.x = max (wc.x, 0);
		wc.y = (DisplayHeight (dpy, scrnum) - height) / 2;
		wc.y = max (wc.y, 0);
		XConfigureWindow (dpy, win, CWX | CWY, &wc);
	}
#endif

	XFlush (dpy);

	ctx = glXCreateContext (dpy, visinfo, NULL, True);

	glXMakeCurrent (dpy, win, ctx);

	vid_clipatom = XInternAtom (dpy, X_CLIPATOM, False);		// False --> create atom if it doesn't exist

	scr_width = width;
	scr_height = height;

	if (vid.conheight > height)
		vid.conheight = height;
	if (vid.conwidth > width)
		vid.conwidth = width;

// JDH: this is done so that HUD/menu/etc have same scale as console
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

//	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.numpages = 2;

	InitSig ();	// trap evil signals

	GL_Init ();

	VID_BuildGammaTable ();
	VID_SetPalette (palette);

#ifndef REORDER20100607
	if (fullscreen)
	{
//		mouseactive = true;
		install_grabs ();
	}
	else
	{
//		if (_windowed_mouse.value && (key_dest == key_game))
//		if ((((int)_windowed_mouse.value == 1) && GAME_ACTIVE()) || ((int)_windowed_mouse.value > 1))
//		if ((int)_windowed_mouse.value > 1)
//		{
//			Con_Print ("VID_Init: grabbing mouse\n");
//			install_grabs ();
//		}
//		else
		XDefineCursor (dpy, win, vid_nullcursor);
		{
			mouseactive = false;
		}
	}
#endif

	InitHWGamma ();

//----------- 2010/06/07 - moved from above ----------------
#ifdef REORDER20100607
	XMapWindow (dpy, win);

	if (fullscreen)	//vidmode_active
	{
		XMoveWindow (dpy, win, 0, 0);
		XRaiseWindow (dpy, win);
		XWarpPointer (dpy, None, win, 0, 0, 0, 0, 0, 0);
		install_grabs ();
	//	XFlush (dpy);
		// Move the viewport to top left
	//	XF86VidModeSetViewPort (dpy, scrnum, 0, 0);
	}
	else	// JDH: center window
	{
		XWindowChanges wc;
		wc.x = (DisplayWidth (dpy, scrnum) - width) / 2;
		wc.x = max (wc.x, 0);
		wc.y = (DisplayHeight (dpy, scrnum) - height) / 2;
		wc.y = max (wc.y, 0);
		XConfigureWindow (dpy, win, CWX | CWY, &wc);
		mouseactive = false;
	}
#endif
//----------- 2010/06/07 - moved from above ----------------

	Con_SafePrintf ("Video mode %dx%dx%d initialized.\n", width, height, visinfo->depth);

	if (fullscreen)
		vid_windowedmouse = false;

	vid.recalc_refdef = 1;			// force a surface cache flush

	if (COM_CheckParm("-fullsbar"))
		fullsbardraw = true;
}

void Sys_SendKeyEvents (void)
{
	if (dpy)
	{
		while (XPending(dpy))
			GetEvent ();

/*		if (!fullscreen)
		{
			if (!_windowed_mouse.value)
			{
				if (old_windowed_mouse)
				{
	//				Con_DPrintf ("releasing mouse - _windowed_mouse changed\n");
					uninstall_grabs ();
	//				uninstall_mousegrab ();
					old_windowed_mouse = 0;
				}
			}
			else
			{
				if (old_windowed_mouse != (int)_windowed_mouse.value)
				{
					install_grabs ();
					old_windowed_mouse = (int)_windowed_mouse.value;
				}

				if (old_windowed_mouse == 1)
				{
					if (mouseactive && !GAME_ACTIVE())
					{
	//					Con_DPrintf ("releasing mouse - game not active\n");
	//					uninstall_mousegrab ();
						uninstall_grabs ();
					}
					else if (!mouseactive && GAME_ACTIVE())
					{
	//					Con_DPrintf ("grabbing mouse - game active\n");
						install_grabs ();
					}
				}
			}
		}*/
	}
}

void Force_CenterView_f (cmd_source_t src)
{
	cl.viewangles[PITCH] = 0;
}

void IN_Init (void)
{
	Cmd_AddCommand ("force_centerview", Force_CenterView_f, 0);
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
}

void IN_MouseMove (usercmd_t *cmd)
{
	float	tx, ty;
	extern double scr_demo_overlay_time;

	tx = mouse_x;
	ty = mouse_y;

	if (m_filter.value)
	{
		mouse_x = (tx + old_mouse_x) * 0.5;
		mouse_y = (ty + old_mouse_y) * 0.5;
	}

	old_mouse_x = tx;
	old_mouse_y = ty;

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

	// add mouse X/Y movement to cmd
	if ((in_strafe.state & 1) || (lookstrafe.value && mouselook))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;

	if (mouselook)
		V_StopPitchDrift ();

	if (mouselook && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		cl.viewangles[PITCH] = bound(-70, cl.viewangles[PITCH], 80);
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (cl.idealroll == 0)) // Did keyboard set it already??
	{
		float movetype = cl.v.movetype;

		if ((mouse_x < 0) && (movetype == MOVETYPE_FLY))
			cl.idealroll = -10;
		else if ((mouse_x > 0) && (movetype == MOVETYPE_FLY))
			cl.idealroll = 10;
	}
#endif

	if (mouse_x || mouse_y)
	{
		mouse_x = mouse_y = 0.0;

		if (cls.demoplayback && (key_dest == key_game))
			scr_demo_overlay_time = realtime;
	}
}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove (cmd);
}
