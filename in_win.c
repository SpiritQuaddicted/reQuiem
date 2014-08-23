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
// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include <InitGuid.h>

#define DIRECTINPUT_VERSION	0x0700

#include <dinput.h>
#include "quakedef.h"
#include "winquake.h"

#ifndef RQM_SV_ONLY

#define DINPUT_BUFFERSIZE	16
#define iDirectInputCreate(a,b,c,d) pDirectInputCreate(a,b,c,d)

HRESULT (WINAPI *pDirectInputCreate)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT * lplpDirectInput, LPUNKNOWN punkOuter);

// mouse variables
cvar_t	m_filter = {"m_filter", "0", CVAR_FLAG_ARCHIVE};

int		mouse_buttons;
int		mouse_oldbuttonstate;
POINT		current_pos;
double		mouse_x, mouse_y;
int		old_mouse_x, old_mouse_y, mx_accum, my_accum;

static qboolean	restore_spi;
static	int	originalmouseparms[3], newmouseparms[3] = {0, 0, 1};

unsigned int	uiWheelMessage;
qboolean	mouseactive;
qboolean	mouseinitialized;
static qboolean	mouseparmsvalid, mouseactivatetoggle;
static qboolean	mouseshowtoggle = 1;
static qboolean	dinput_acquired;

static unsigned int	mstate_di;

// joystick defines and variables
// where should defines be moved?
#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball
#define	JOY_MAX_AXES		6			// X, Y, Z, R, U, V
#define JOY_AXIS_X		0
#define JOY_AXIS_Y		1
#define JOY_AXIS_Z		2
#define JOY_AXIS_R		3
#define JOY_AXIS_U		4
#define JOY_AXIS_V		5

enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn
};

DWORD dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};

DWORD	dwAxisMap[JOY_MAX_AXES];
DWORD	dwControlMap[JOY_MAX_AXES];
PDWORD	pdwRawValue[JOY_MAX_AXES];

// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.
cvar_t	in_joystick = {"joystick","0", CVAR_FLAG_ARCHIVE};
cvar_t	joy_name = {"joyname", "joystick"};
cvar_t	joy_advanced = {"joyadvanced", "0"};
cvar_t	joy_advaxisx = {"joyadvaxisx", "0"};
cvar_t	joy_advaxisy = {"joyadvaxisy", "0"};
cvar_t	joy_advaxisz = {"joyadvaxisz", "0"};
cvar_t	joy_advaxisr = {"joyadvaxisr", "0"};
cvar_t	joy_advaxisu = {"joyadvaxisu", "0"};
cvar_t	joy_advaxisv = {"joyadvaxisv", "0"};
cvar_t	joy_forwardthreshold = {"joyforwardthreshold", "0.15"};
cvar_t	joy_sidethreshold = {"joysidethreshold", "0.15"};
cvar_t	joy_pitchthreshold = {"joypitchthreshold", "0.15"};
cvar_t	joy_yawthreshold = {"joyyawthreshold", "0.15"};
cvar_t	joy_forwardsensitivity = {"joyforwardsensitivity", "-1.0"};
cvar_t	joy_sidesensitivity = {"joysidesensitivity", "-1.0"};
cvar_t	joy_pitchsensitivity = {"joypitchsensitivity", "1.0"};
cvar_t	joy_yawsensitivity = {"joyyawsensitivity", "-1.0"};
cvar_t	joy_wwhack1 = {"joywwhack1", "0.0"};
cvar_t	joy_wwhack2 = {"joywwhack2", "0.0"};

qboolean	joy_avail, joy_advancedinit, joy_haspov;
DWORD		joy_oldbuttonstate, joy_oldpovstate;

int		joy_id;
DWORD		joy_flags;
DWORD		joy_numbuttons;

static	LPDIRECTINPUT		g_pdi;
static	LPDIRECTINPUTDEVICE	g_pMouse;

static	JOYINFOEX	ji;

static	HINSTANCE	hInstDI;

static qboolean	dinput;

typedef struct MYDATA {
	LONG  lX;                   // X axis goes here
	LONG  lY;                   // Y axis goes here
	LONG  lZ;                   // Z axis goes here
	BYTE  bButtonA;             // One button goes here
	BYTE  bButtonB;             // Another button goes here
	BYTE  bButtonC;             // Another button goes here
	BYTE  bButtonD;             // Another button goes here
} MYDATA;

static DIOBJECTDATAFORMAT rgodf[] = {
  { &GUID_XAxis,    FIELD_OFFSET(MYDATA, lX),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_YAxis,    FIELD_OFFSET(MYDATA, lY),       DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { &GUID_ZAxis,    FIELD_OFFSET(MYDATA, lZ),       0x80000000 | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonA), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonB), DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonC), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
  { 0,              FIELD_OFFSET(MYDATA, bButtonD), 0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0,},
};

#define NUM_OBJECTS (sizeof(rgodf) / sizeof(rgodf[0]))

static DIDATAFORMAT	df = {
	sizeof(DIDATAFORMAT),       // this structure
	sizeof(DIOBJECTDATAFORMAT), // size of object data format
	DIDF_RELAXIS,               // absolute axis coordinates
	sizeof(MYDATA),             // device data size
	NUM_OBJECTS,                // number of objects
	rgodf,                      // and here they are
};

// forward-referenced functions
void IN_StartupJoystick (void);
void Joy_AdvancedUpdate_f (cmd_source_t src);
void IN_JoyMove (usercmd_t *cmd);

cvar_t	m_rate		= {"m_rate",	"60"};
cvar_t	m_showrate	= {"m_showrate", "0"};

qboolean	use_m_smooth;
HANDLE		m_event;

#define	 M_HIST_SIZE  64
#define	 M_HIST_MASK  (M_HIST_SIZE - 1)

typedef	struct msnap_s {
	long   data;	// data (relative axis pos)
	double time;	// timestamp
} msnap_t;

msnap_t	m_history_x[M_HIST_SIZE];	// history
msnap_t	m_history_y[M_HIST_SIZE];
int	m_history_x_wseq = 0;		// write sequence
int	m_history_y_wseq = 0;
int	m_history_x_rseq = 0;		// read	sequence
int	m_history_y_rseq = 0;
int	wheel_up_count = 0;
int	wheel_dn_count = 0;

#define INPUT_CASE_DIMOFS_BUTTON(NUM)			\
	case (DIMOFS_BUTTON0 + NUM):			\
		if (od.dwData &	0x80)			\
			mstate_di |= (1	<< NUM);	\
		else					\
			mstate_di &= ~(1 << NUM);	\
		break;

#define INPUT_CASE_DINPUT_MOUSE_BUTTONS			\
		INPUT_CASE_DIMOFS_BUTTON(0);		\
		INPUT_CASE_DIMOFS_BUTTON(1);		\
		INPUT_CASE_DIMOFS_BUTTON(2);		\

DWORD WINAPI IN_SMouseProc (void *lpParameter)
{
	// read	mouse events and generate history tables
	DWORD	ret;

	while (1)
	{
		if ((ret = WaitForSingleObject(m_event,	INFINITE)) == WAIT_OBJECT_0)
		{
			int			mx = 0,	my = 0;
			DIDEVICEOBJECTDATA	od;
			HRESULT	 		hr;
			double 			time;

			if (!ActiveApp || Minimized || !mouseactive || !dinput_acquired)
			{
				Sleep (50);
				continue;
			}

			time = Sys_DoubleTime ();

			while (1)
			{
				DWORD	dwElements = 1;

				hr = IDirectInputDevice_GetDeviceData (g_pMouse, sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0);

				if ((hr	== DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
				{
					dinput_acquired	= false;
					break;
				}

				// Unable to read data or no data available
				if (FAILED(hr) || !dwElements)
					break;

				// Look at the element to see what happened
				switch (od.dwOfs)
				{
					case DIMOFS_X:
						m_history_x[m_history_x_wseq & M_HIST_MASK].time = time;
						m_history_x[m_history_x_wseq & M_HIST_MASK].data = od.dwData;
						m_history_x_wseq++;
						break;

					case DIMOFS_Y:
						m_history_y[m_history_y_wseq & M_HIST_MASK].time = time;
						m_history_y[m_history_y_wseq & M_HIST_MASK].data = od.dwData;
						m_history_y_wseq++;
						break;

					INPUT_CASE_DINPUT_MOUSE_BUTTONS

					case DIMOFS_Z:
						if (od.dwData &	0x80)
							wheel_dn_count++;
						else
							wheel_up_count++;
						break;
				}
			}
		}
	}
}

void IN_SMouseRead (int *mx, int *my)
{
	static	acc_x, acc_y;
	int	x = 0, y = 0;
	double	t1, t2, maxtime, mintime;

	// acquire device
	IDirectInputDevice_Acquire (g_pMouse);
	dinput_acquired	= true;

	// gather data from last read seq to now
	for ( ; m_history_x_rseq < m_history_x_wseq ; m_history_x_rseq++)
		x += m_history_x[m_history_x_rseq&M_HIST_MASK].data;
	for ( ; m_history_y_rseq < m_history_y_wseq ; m_history_y_rseq++)
		y += m_history_y[m_history_y_rseq&M_HIST_MASK].data;

	x -= acc_x;
	y -= acc_y;

	acc_x =	acc_y =	0;

	// show	rate if	requested
	if (m_showrate.value)
	{
		static	last_wseq_printed;

		if (m_history_x_wseq > last_wseq_printed)
		{
			double t = m_history_x[(m_history_x_rseq - 1) &	M_HIST_MASK].time -
					   m_history_x[(m_history_x_rseq - 2) &	M_HIST_MASK].time;

			if (t >	0.001)
				Con_Printf ("mouse rate: %3d\n", (int)(1 / t));

			last_wseq_printed = m_history_x_wseq;
		}
	}

	// smoothing goes here
	mintime	= maxtime = 1.0 / max(m_rate.value, 10);
	maxtime	*= 1.2;
	mintime	*= 0.7;

	// X axis
	t1 = m_history_x[(m_history_x_rseq - 2)	& M_HIST_MASK].time;
	t2 = m_history_x[(m_history_x_rseq - 1)	& M_HIST_MASK].time;

	if (t2 - t1 > mintime && t2 - t1 < maxtime)
	{
		double vel = m_history_x[(m_history_x_rseq - 1)	& M_HIST_MASK].data / (t2 - t1);

		t1 = t2;
		t2 = Sys_DoubleTime ();

		if (t2 - t1 < maxtime)
			acc_x =	vel * (t2 - t1);
	}

	// Y axis
	t1 = m_history_y[(m_history_y_rseq - 2)	& M_HIST_MASK].time;
	t2 = m_history_y[(m_history_y_rseq - 1)	& M_HIST_MASK].time;

	if (t2 - t1 > mintime && t2 - t1 < maxtime)
	{
		double vel = m_history_y[(m_history_y_rseq-1) &	M_HIST_MASK].data / (t2 - t1);

		t1 = t2;
		t2 = Sys_DoubleTime ();

		if (t2 - t1 < maxtime)
			acc_y =	vel * (t2 - t1);
	}

	x += acc_x;
	y += acc_y;

	// return data
	*mx = x;
	*my = y;

	// serve wheel
	bound(0, wheel_dn_count, 10);
	bound(0, wheel_up_count, 10);

	while (wheel_dn_count >	0)
	{
		Key_Event (K_MWHEELDOWN, true);
		Key_Event (K_MWHEELDOWN, false);
		wheel_dn_count--;
	}
	while (wheel_up_count >	0)
	{
		Key_Event (K_MWHEELUP, true);
		Key_Event (K_MWHEELUP, false);
		wheel_up_count--;
	}
}

void IN_SMouseInit (void)
{
	HRESULT	res;
	DWORD	threadid;
	HANDLE	thread;

	use_m_smooth = false;
	if (!COM_CheckParm("-m_smooth"))
		return;

	// create event	object
	m_event	= CreateEventA(
		NULL,			// NULL secutity attributes
		FALSE,			// automatic reset
		FALSE,			// initial state = nonsignaled
		NULL			// NULL name
	);
	if (m_event == NULL)
		return;

	// enable di notification
	if ((res = IDirectInputDevice_SetEventNotification(g_pMouse, m_event)) != DI_OK	&& res != DI_POLLEDDEVICE)
		return;

	// create thread
	thread = CreateThread (
		NULL,			// pointer to security attributes
		0,			// initial thread stack	size
		IN_SMouseProc,		// pointer to thread function
		NULL,			// argument for new thread
		CREATE_SUSPENDED,	// creation flags
		&threadid		// pointer to receive thread ID
	);
	if (!thread)
		return;

	SetThreadPriority (thread, THREAD_PRIORITY_HIGHEST);
	ResumeThread (thread);

	Cvar_Register (&m_rate);
	Cvar_RegisterBool (&m_showrate);

	use_m_smooth = true;
}

/*
===========
Force_CenterView_f
===========
*/
void Force_CenterView_f (cmd_source_t src)
{
	cl.viewangles[PITCH] = 0;
}

/*
===========
IN_UpdateClipCursor
===========
*/
void IN_UpdateClipCursor (void)
{
	if (mouseinitialized && mouseactive && !dinput)
		ClipCursor (&window_rect);
}

/*
===========
IN_ShowMouse
===========
*/
void IN_ShowMouse (void)
{
	if (!mouseshowtoggle)
	{
		ShowCursor (TRUE);
		mouseshowtoggle = 1;
	}
}

/*
===========
IN_HideMouse
===========
*/
void IN_HideMouse (void)
{
	if (mouseshowtoggle)
	{
		ShowCursor (FALSE);
		mouseshowtoggle = 0;
	}
}

/*
===========
IN_ActivateMouse
===========
*/
void IN_ActivateMouse (void)
{
	mouseactivatetoggle = true;

	if (mouseinitialized)
	{
		if (dinput)
		{
			if (g_pMouse)
			{
				if (!dinput_acquired)
				{
					IDirectInputDevice_Acquire (g_pMouse);
					dinput_acquired = true;
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			if (mouseparmsvalid)
				restore_spi = SystemParametersInfoA (SPI_SETMOUSE, 0, newmouseparms, 0);

			SetCursorPos (window_center_x, window_center_y);
			SetCapture (mainwindow);
			ClipCursor (&window_rect);
		}

		mouseactive = true;
	}
}

/*
===========
IN_SetQuakeMouseState
===========
*/
void IN_SetQuakeMouseState (void)
{
	if (mouseactivatetoggle)
		IN_ActivateMouse ();
}

/*
===========
IN_DeactivateMouse
===========
*/
void IN_DeactivateMouse (void)
{
	mouseactivatetoggle = false;

	if (mouseinitialized)
	{
		if (dinput)
		{
			if (g_pMouse)
			{
				if (dinput_acquired)
				{
					IDirectInputDevice_Unacquire(g_pMouse);
					dinput_acquired = false;
				}
			}
		}
		else
		{
			if (restore_spi)
				SystemParametersInfoA (SPI_SETMOUSE, 0, originalmouseparms, 0);

			ClipCursor (NULL);
			ReleaseCapture ();
		}

		mouseactive = false;
	}
}

/*
===========
IN_RestoreOriginalMouseState
===========
*/
void IN_RestoreOriginalMouseState (void)
{
	if (mouseactivatetoggle)
	{
		IN_DeactivateMouse ();
		mouseactivatetoggle = true;
	}

// try to redraw the cursor so it gets reinitialized, because sometimes it
// has garbage after the mode switch
	ShowCursor (TRUE);
	ShowCursor (FALSE);
}

/*
===========
IN_InitDInput
===========
*/
qboolean IN_InitDInput (void)
{
	HRESULT		hr;
	DIPROPDWORD	dipdw = {
		{
			sizeof(DIPROPDWORD),        // diph.dwSize
			sizeof(DIPROPHEADER),       // diph.dwHeaderSize
			0,                          // diph.dwObj
			DIPH_DEVICE,                // diph.dwHow
		},
		DINPUT_BUFFERSIZE,              // dwData
	};

	if (!hInstDI)
	{
		hInstDI = LoadLibrary ("dinput.dll");

		if (hInstDI == NULL)
		{
			Con_SafePrintf ("Couldn't load dinput.dll\n");
			return false;
		}
	}

	if (!pDirectInputCreate)
	{
		pDirectInputCreate = (void *)GetProcAddress(hInstDI,"DirectInputCreateA");

		if (!pDirectInputCreate)
		{
			Con_SafePrintf ("Couldn't get DI proc addr\n");
			return false;
		}
	}

// register with DirectInput and get an IDirectInput to play with.
	hr = iDirectInputCreate(global_hInstance, DIRECTINPUT_VERSION, &g_pdi, NULL);

	if (FAILED(hr))
	{
		return false;
	}

// obtain an interface to the system mouse device.
	hr = IDirectInput_CreateDevice(g_pdi, &GUID_SysMouse, &g_pMouse, NULL);

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't open DI mouse device\n");
		return false;
	}

// set the data format to "mouse format".
	hr = IDirectInputDevice_SetDataFormat(g_pMouse, &df);

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't set DI mouse format\n");
		return false;
	}

// set the cooperativity level.
	hr = IDirectInputDevice_SetCooperativeLevel(g_pMouse, mainwindow,
			DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't set DI coop level\n");
		return false;
	}


// set the buffer size to DINPUT_BUFFERSIZE elements.
// the buffer size is a DWORD property associated with the device
	hr = IDirectInputDevice_SetProperty(g_pMouse, DIPROP_BUFFERSIZE, &dipdw.diph);

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't set DI buffersize\n");
		return false;
	}

	IN_SMouseInit ();

	return true;
}

/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void)
{
	if (COM_CheckParm ("-nomouse"))
		return;

	mouseinitialized = true;

	if (COM_CheckParm ("-dinput"))
	{
		dinput = IN_InitDInput ();

		if (dinput)
		{
			Con_SafePrintf ("DirectInput initialized\n");
			if (use_m_smooth)
				Con_SafePrintf ("Mouse smoothing initialized\n");
		}
		else
			Con_SafePrintf ("DirectInput not initialized\n");
	}

	if (!dinput)
	{
		mouseparmsvalid = SystemParametersInfoA (SPI_GETMOUSE, 0, originalmouseparms, 0);

		if (mouseparmsvalid)
		{
			if (COM_CheckParm ("-noforcemspd"))
				newmouseparms[2] = originalmouseparms[2];

			if (COM_CheckParm ("-noforcemaccel"))
			{
				newmouseparms[0] = originalmouseparms[0];
				newmouseparms[1] = originalmouseparms[1];
			}

			if (COM_CheckParm ("-noforcemparms"))
			{
				newmouseparms[0] = originalmouseparms[0];
				newmouseparms[1] = originalmouseparms[1];
				newmouseparms[2] = originalmouseparms[2];
			}
		}
	}

	mouse_buttons = 3;

// if a fullscreen video mode was set before the mouse was initialized,
// set the mouse state appropriately
	if (mouseactivatetoggle)
		IN_ActivateMouse ();
}

#if 0
void IN_CharToScan (char c, byte *scan)
{
	short vkey;
	UINT code;

	vkey = VkKeyScanA (c);
	if ((vkey & 0xFF00) == 0)		// no modifier keys
	{
		code = MapVirtualKeyA (vkey & 0x00FF, 0);		// 0 --> vkey to scan code
		if (code && (code < 128))
			scantokey[code] = c;
	}

}

/*
===========
IN_InitKeymap (JDH)

  Construct the table of scan codes to Quake key codes
===========
*/
void IN_InitKeymap (void)
{
	short vkey;
	UINT code;
	byte vtoqkey[128];

	for (i = 'a'; i <= 'z'; i++)
		IN_CharToScan (i);
	{
		vkey = VkKeyScanA (i);
		if ((vkey & 0xFF00) == 0)		// no modifier keys
		{
			code = MapVirtualKeyA (vkey & 0x00FF, 0);		// 0 --> vkey to scan code
			if (code && (code < 128))
				scantokey[code] = i;
		}
	}

	for (i = 0; i < 128; i++)
		vtoqkey[i] = 0;

	for (i = 'a'; i <= 'z'; i++)
		vtoqkey[i] = i;

	for (i = '0'; i <= '9'; i++)
		vtoqkey[i] = i;

	for (i = 0; i < 12; i++)
		vtoqkey[VK_F1 + i] = K_F1 + i;

	vtoqkey[VK_ESCAPE] = K_ESCAPE;
	vtoqkey[VK_TAB] = K_TAB;
	vtoqkey[VK_BACK] = K_BACKSPACE;		/*** ??? ***/
	vtoqkey[VK_TAB] = K_TAB;
	vtoqkey[VK_TAB] = K_TAB;

	for (i = 'a'; i <= 'z'; i++)
	{
		code = MapVirtualKeyA (i, 0);		// 0 --> vkey to scan code
		if (code && (code < 128))
			scantokey[code] = i;
	}

	for (i = '0'; i <= '9'; i++)
	{
		code = MapVirtualKeyA (i, 0);		// 0 --> vkey to scan code
		if (code && (code < 128))
			scantokey[code] = i;
	}

	for (i = 0; i < 12; i++)
	{
		code = MapVirtualKeyA (VK_F1 + i, 0);		// 0 --> vkey to scan code
		if (code && (code < 128))
			scantokey[code] = K_F1 + i;
	}

	for (i = 0; i < 128; i++)
	{
		code = MapVirtualKeyA (i, 2);		// 2 --> vkey to character value
		vkey = VkKeyScanA (c);
		if ((vkey & 0xFF00) == 0)		// no modifier keys
	{
		if (code && (code < 128) && (scantokey[code] != c))
			scantokey[code] = c;
	}
}
#endif

/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	// mouse variables
	Cvar_RegisterBool (&m_filter);

	// joystick variables
	Cvar_Register (&in_joystick);
	Cvar_Register (&joy_name);
	Cvar_Register (&joy_advanced);
	Cvar_Register (&joy_advaxisx);
	Cvar_Register (&joy_advaxisy);
	Cvar_Register (&joy_advaxisz);
	Cvar_Register (&joy_advaxisr);
	Cvar_Register (&joy_advaxisu);
	Cvar_Register (&joy_advaxisv);
	Cvar_Register (&joy_forwardthreshold);
	Cvar_Register (&joy_sidethreshold);
	Cvar_Register (&joy_pitchthreshold);
	Cvar_Register (&joy_yawthreshold);
	Cvar_Register (&joy_forwardsensitivity);
	Cvar_Register (&joy_sidesensitivity);
	Cvar_Register (&joy_pitchsensitivity);
	Cvar_Register (&joy_yawsensitivity);
	Cvar_Register (&joy_wwhack1);
	Cvar_Register (&joy_wwhack2);

	Cmd_AddCommand ("force_centerview", Force_CenterView_f, 0);
	Cmd_AddCommand ("joyadvancedupdate", Joy_AdvancedUpdate_f, 0);

	uiWheelMessage = RegisterWindowMessageA ("MSWHEEL_ROLLMSG");

	IN_StartupMouse ();
	IN_StartupJoystick ();

	IN_SetupKeyMap ();
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{
	IN_DeactivateMouse ();
	IN_ShowMouse ();

	if (g_pMouse)
	{
		IDirectInputDevice_Release (g_pMouse);
		g_pMouse = NULL;
	}

	if (g_pdi)
	{
		IDirectInput_Release (g_pdi);
		g_pdi = NULL;
	}
}

/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate)
{
	int	i;

	if (mouseactive && !dinput)
	{
	// perform button actions
		for (i=0 ; i<mouse_buttons ; i++)
		{
			if ((mstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)))
				Key_Event (K_MOUSE1 + i, true);

			if (!(mstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)))
				Key_Event (K_MOUSE1 + i, false);
		}

		mouse_oldbuttonstate = mstate;
	}
}

/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	int			mx, my, i;
	DIDEVICEOBJECTDATA	od;
	DWORD			dwElements;
	HRESULT			hr;
	extern double scr_demo_overlay_time;

	if (!mouseactive)
		return;

	if (dinput)
	{
		mx = my = 0;

		if (use_m_smooth)
			IN_SMouseRead (&mx, &my);
		else
		{
			while (1)
			{
				dwElements = 1;

				hr = IDirectInputDevice_GetDeviceData (g_pMouse,
					sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0);

				if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
				{
					dinput_acquired = true;
					IDirectInputDevice_Acquire (g_pMouse);
					break;
				}

				// Unable to read data or no data available
				if (FAILED(hr) || dwElements == 0)
					break;

				// Look at the element to see what happened
				switch (od.dwOfs)
				{
					case DIMOFS_X:
						mx += od.dwData;
						break;

					case DIMOFS_Y:
						my += od.dwData;
						break;

					INPUT_CASE_DINPUT_MOUSE_BUTTONS

					case DIMOFS_Z:
						if (od.dwData & 0x80)
						{
							Key_Event (K_MWHEELDOWN, true);
							Key_Event (K_MWHEELDOWN, false);
						}
						else
						{
							Key_Event (K_MWHEELUP, true);
							Key_Event (K_MWHEELUP, false);
						}
				}
			}
		}

	// perform button actions
		for (i=0 ; i<mouse_buttons ; i++)
		{
			if ((mstate_di & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)))
				Key_Event (K_MOUSE1 + i, true);

			if (!(mstate_di & (1<<i)) && (mouse_oldbuttonstate & (1<<i)))
				Key_Event (K_MOUSE1 + i, false);
		}

		mouse_oldbuttonstate = mstate_di;
	}
	else
	{
		GetCursorPos (&current_pos);
		mx = current_pos.x - window_center_x + mx_accum;
		my = current_pos.y - window_center_y + my_accum;
		mx_accum = my_accum = 0;
	}

	if (m_filter.value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

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
		cl.viewangles[PITCH] = bound (-70, cl.viewangles[PITCH], 80);
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

// if the mouse has moved, force it to the center, so there's room to move
	if (mx || my)
	{
		SetCursorPos (window_center_x, window_center_y);

		if (cls.demoplayback && (key_dest == key_game))
			scr_demo_overlay_time = realtime;
	}
}

/*
===========
IN_Move
===========
*/

void IN_Move (usercmd_t *cmd)
{
	if (ActiveApp && !Minimized)
	{
		IN_MouseMove (cmd);
		IN_JoyMove (cmd);
	}
}

/*
===========
IN_Accumulate
===========
*/
void IN_Accumulate (void)
{
	if (mouseactive)
	{
		if (!dinput)
		{
			GetCursorPos (&current_pos);

			mx_accum += current_pos.x - window_center_x;
			my_accum += current_pos.y - window_center_y;

		// force the mouse to the center, so there's room to move
			SetCursorPos (window_center_x, window_center_y);
		}
	}
}

/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void)
{
	if (mouseactive)
		mx_accum = my_accum = mouse_oldbuttonstate = 0;
}

/*
===============
IN_StartupJoystick
===============
*/
void IN_StartupJoystick (void)
{
	int		numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;

 	// assume no joystick
	joy_avail = false;

	// abort startup if user requests no joystick
	if (COM_CheckParm("-nojoy"))
		return;

	// verify joystick driver is present
	if (!(numdevs = joyGetNumDevs()))
	{
		Con_Print ("joystick not found -- driver not present\n");
		return;
	}

	// cycle through the joystick ids for the first valid one
	for (joy_id = 0 ; joy_id < numdevs ; joy_id++)
	{
		memset (&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;

		if ((mmr = joyGetPosEx(joy_id, &ji)) == JOYERR_NOERROR)
			break;
	}

	// abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR)
	{
		Con_DPrintf ("joystick not found -- no valid joysticks (%x)\n", mmr);
		return;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof(jc));
	if ((mmr = joyGetDevCapsA(joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		Con_Printf ("joystick not found -- invalid joystick capabilities (%x)\n", mmr);
		return;
	}

	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization

	joy_avail = true;
	joy_advancedinit = false;

	Con_Print ("joystick detected\n");
}

/*
===========
RawValuePointer
===========
*/
PDWORD RawValuePointer (int axis)
{
	switch (axis)
	{
		case JOY_AXIS_X:
			return &ji.dwXpos;
		case JOY_AXIS_Y:
			return &ji.dwYpos;
		case JOY_AXIS_Z:
			return &ji.dwZpos;
		case JOY_AXIS_R:
			return &ji.dwRpos;
		case JOY_AXIS_U:
			return &ji.dwUpos;
		case JOY_AXIS_V:
			return &ji.dwVpos;
	}

	return NULL;	// shut up compiler
}

/*
===========
Joy_AdvancedUpdate_f
===========
*/
void Joy_AdvancedUpdate_f (cmd_source_t src)
{

	// called once by IN_ReadJoystick and by user whenever an update is needed
	// cvars are now available
	int	i;
	DWORD	dwTemp;

	// initialize all the maps
	for (i=0 ; i<JOY_MAX_AXES ; i++)
	{
		dwAxisMap[i] = AxisNada;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer(i);
	}

	if (joy_advanced.value == 0.0)
	{
		// default joystick initialization
		// 2 axes only with joystick control
		dwAxisMap[JOY_AXIS_X] = AxisTurn;
		// dwControlMap[JOY_AXIS_X] = JOY_ABSOLUTE_AXIS;
		dwAxisMap[JOY_AXIS_Y] = AxisForward;
		// dwControlMap[JOY_AXIS_Y] = JOY_ABSOLUTE_AXIS;
	}
	else
	{
		if (strcmp(joy_name.string, "joystick"))
		{
			// notify user of advanced controller
			Con_Printf ("\n%s configured\n\n", joy_name.string);
		}

		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = (DWORD) joy_advaxisx.value;
		dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisy.value;
		dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisz.value;
		dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisr.value;
		dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisu.value;
		dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisv.value;
		dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}

	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for (i=0 ; i<JOY_MAX_AXES ; i++)
		if (dwAxisMap[i] != AxisNada)
			joy_flags |= dwAxisFlags[i];
}

/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int	i, key_index;
	DWORD	buttonstate, povstate;

	if (!joy_avail)
		return;

	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for (i=0 ; i<joy_numbuttons ; i++)
	{
		if ((buttonstate & (1<<i)) && !(joy_oldbuttonstate & (1<<i)))
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, true);
		}

		if (!(buttonstate & (1<<i)) && (joy_oldbuttonstate & (1<<i)))
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, false);
		}
	}
	joy_oldbuttonstate = buttonstate;

	if (joy_haspov)
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if(ji.dwPOV != JOY_POVCENTERED)
		{
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;
			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;
			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;
			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for each change
		for (i=0 ; i<4 ; i++)
		{
			if ((povstate & (1<<i)) && !(joy_oldpovstate & (1<<i)))
				Key_Event (K_AUX29 + i, true);
			if (!(povstate & (1<<i)) && (joy_oldpovstate & (1<<i)))
				Key_Event (K_AUX29 + i, false);
		}
		joy_oldpovstate = povstate;
	}
}

/*
===============
IN_ReadJoystick
===============
*/
qboolean IN_ReadJoystick (void)
{
	memset (&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx(joy_id, &ji) == JOYERR_NOERROR)
	{
		// this is a hack -- there is a bug in the Logitech WingMan Warrior DirectInput Driver
		// rather than having 32768 be the zero point, they have the zero point at 32668
		// go figure -- anyway, now we get the full resolution out of the device
		if (joy_wwhack1.value != 0.0)
			ji.dwUpos += 100;
		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error,\
		// but what should be done?
		// Con_Print ("IN_ReadJoystick: no response\n");
		// joy_avail = false;
		return false;
	}
}

/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove (usercmd_t *cmd)
{
	float	speed, aspeed;
	float	fAxisValue, fTemp;
	int	i;

	// complete initialization if first time in
	// this is needed as cvars are not available at initialization time
	if (joy_advancedinit != true)
	{
		Joy_AdvancedUpdate_f (SRC_COMMAND);
		joy_advancedinit = true;
	}

	// verify joystick is available and that the user wants to use it
	if (!joy_avail || !in_joystick.value)
		return;

	// collect the joystick data, if possible
	if (IN_ReadJoystick () != true)
		return;

	speed = (in_speed.state & 1) ? cl_movespeedkey.value : 1;
	aspeed = speed * host_frametime;

	// loop through the axes
	for (i=0 ; i<JOY_MAX_AXES ; i++)
	{
		// get the floating point zero-centered, potentially-inverted data for the current axis
		fAxisValue = (float)*pdwRawValue[i];
		// move centerpoint to zero
		fAxisValue -= 32768.0;

		if (joy_wwhack2.value != 0.0)
		{
			if (dwAxisMap[i] == AxisTurn)
			{
				// this is a special formula for the Logitech WingMan Warrior
				// y=ax^b; where a = 300 and b = 1.3
				// also x values are in increments of 800 (so this is factored out)
				// then bounds check result to level out excessively high spin rates
				fTemp = 300.0 * pow(abs(fAxisValue) / 800.0, 1.3);
				if (fTemp > 14000.0)
					fTemp = 14000.0;
				// restore direction information
				fAxisValue = (fAxisValue > 0.0) ? fTemp : -fTemp;
			}
		}

		// convert range from -32768..32767 to -1..1
		fAxisValue /= 32768.0;

		switch (dwAxisMap[i])
		{
		case AxisForward:
			if ((joy_advanced.value == 0.0) && mouselook)
			{
				// user wants forward control to become look control
				if (fabs(fAxisValue) > joy_pitchthreshold.value)
				{
					// if mouse invert is on, invert the joystick pitch value
					// only absolute control support here (joy_advanced is false)
					if (m_pitch.value < 0.0)
						cl.viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					else
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					V_StopPitchDrift ();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if (lookspring.value == 0.0)
						V_StopPitchDrift ();
				}
			}
			else
			{
				// user wants forward control to be forward control
				if (fabs(fAxisValue) > joy_forwardthreshold.value)
					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity.value) * speed * cl_forwardspeed.value;
			}
			break;

		case AxisSide:
			if (fabs(fAxisValue) > joy_sidethreshold.value)
				cmd->sidemove += (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
			break;

		case AxisTurn:
			if ((in_strafe.state & 1) || (lookstrafe.value && mouselook))
			{
				// user wants turn control to become side control
				if (fabs(fAxisValue) > joy_sidethreshold.value)
					cmd->sidemove -= (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
			}
			else
			{
				// user wants turn control to be turn control
				if (fabs(fAxisValue) > joy_yawthreshold.value)
				{
					if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity.value) * aspeed * cl_yawspeed.value;
					else
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity.value) * speed * 180.0;
				}
			}
			break;

		case AxisLook:
			if (mouselook)
			{
				if (fabs(fAxisValue) > joy_pitchthreshold.value)
				{
					// pitch movement detected and pitch movement desired by user
					if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					else
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * speed * 180.0;
					V_StopPitchDrift ();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if (lookspring.value == 0.0)
						V_StopPitchDrift ();
				}
			}
			break;

		default:
			break;
		}
	}

	// bounds check pitch
	cl.viewangles[PITCH] = bound (-70, cl.viewangles[PITCH], 80);
}

//==========================================================================


/*static byte scantokey[128] =
{
//      0       1        2       3       4       5       6       7
//      8       9        A       B       C       D       E       F
	0  ,   K_ESCAPE,'1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',     0,      0, K_BACKSPACE, K_TAB,   // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',     0,      0,   K_ENTER, K_CTRL,  'a',    's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',     0,
	 0,      0,    K_SHIFT,  0,     'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',     0,      0,      0,    K_SHIFT, '*',
	K_ALT, K_SPACE, K_CAPSLOCK, K_F1,  K_F2,   K_F3,   K_F4,   K_F5,  // 3
	K_F6,   K_F7,   K_F8,   K_F9,   K_F10,  K_PAUSE, 0,     K_HOME,
	K_UPARROW,K_PGUP,'-',   K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END,   // 4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL, 0,      0,      0,      K_F11,
	K_F12,  0,      0,      0,      0,      0,      0,      0,        // 5
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,        // 6
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0         // 7
};
*/
static byte vtoqkey[256];
static byte kbstate[256];

extern byte		key_fromcode[256];
extern byte		key_shifttable[256];
extern qboolean	key_forceUSlayout;

#ifndef VK_OEM_PLUS
#  define VK_OEM_PLUS       0xBB   // '+' any country
#  define VK_OEM_COMMA      0xBC   // ',' any country
#  define VK_OEM_MINUS      0xBD   // '-' any country
#  define VK_OEM_PERIOD     0xBE   // '.' any country
#endif
/*
#ifndef VK_OEM_1
#  define VK_OEM_1          0xBA   // ';:' for US
#  define VK_OEM_2          0xBF   // '/?' for US
#  define VK_OEM_3          0xC0   // '`~' for US
#  define VK_OEM_4          0xDB  //  '[{' for US
#  define VK_OEM_5          0xDC  //  '\|' for US
#  define VK_OEM_6          0xDD  //  ']}' for US
#  define VK_OEM_7          0xDE  //  ''"' for US
#  define VK_OEM_8          0xDF
#endif
*/
#ifndef VK_OEM_RESET
#  define VK_OEM_RESET      0xE9		// start of Nokia/Ericsson keys
#endif

/*
===========
IN_SetupKeyMap (JDH)

Sets up the keyboard translation table.  Each character in the given string is checked;
if it requires NO modifier keys, it is entered into the vtoqkey table.
===========
*/
void IN_SetupKeyMap (void)
{
	short vkey;
//	UINT code;
	int i, numexkeys;
	char *s;
	extern char key_symbolchars[];


#ifdef _DEBUG
//	LoadKeyboardLayout ("00000407", KLF_ACTIVATE);			//**** TEMP TEST!!! ****
#endif
/*
	for (; *s; s++)
	{
		vkey = VkKeyScanA (*s);
		if ((vkey & 0xFF00) == 0)		// no modifier bits set
		{
			code = MapVirtualKeyA (vkey & 0x00FF, 0);		// 0 --> get scan code
			if (code && (code < 128) && !scantokey[code])
				scantokey[code] = *s;
		}
	}
*/
	for (i = 0; i < 26; i++)
		vtoqkey['A'+ i] = 'a'+ i;

	for (i = '0'; i <= '9'; i++)
	{
		vtoqkey[i] = i;
		vtoqkey[VK_NUMPAD0+i-'0'] = i;			// sent only if numlock is ON
	}

	for (i = 0; i < 12; i++)
		vtoqkey[VK_F1+i] = K_F1+i;

// NOTE: VK_OEM_ constants are non-contiguous!
// NOTE2: on US keyboards, most of the K_EXn values get overwritten by the loop at the end
/*	vtoqkey[VK_OEM_1] = K_EX1;
	vtoqkey[VK_OEM_2] = K_EX2;
	vtoqkey[VK_OEM_3] = K_EX3;
	for (i = 0; i < 5; i++)
		vtoqkey[VK_OEM_4+i] = K_EX4+i;*/

	vtoqkey[VK_TAB] = K_TAB;
	vtoqkey[VK_RETURN] = K_ENTER;
	vtoqkey[VK_ESCAPE] = K_ESCAPE;
	vtoqkey[VK_SPACE] = K_SPACE;

	vtoqkey[VK_BACK] = K_BACKSPACE;
	vtoqkey[VK_UP] = K_UPARROW;
	vtoqkey[VK_DOWN] = K_DOWNARROW;
	vtoqkey[VK_LEFT] = K_LEFTARROW;
	vtoqkey[VK_RIGHT] = K_RIGHTARROW;

	vtoqkey[VK_MENU] = K_ALT;
	vtoqkey[VK_CONTROL] = K_CTRL;
	vtoqkey[VK_SHIFT] = K_SHIFT;

	vtoqkey[VK_INSERT] = K_INS;
	vtoqkey[VK_DELETE] = K_DEL;
	vtoqkey[VK_NEXT] = K_PGDN;
	vtoqkey[VK_PRIOR] = K_PGUP;
	vtoqkey[VK_HOME] = K_HOME;
	vtoqkey[VK_END] = K_END;

	vtoqkey[VK_CAPITAL] = K_CAPSLOCK;
	vtoqkey[VK_NUMLOCK] = K_NUMLOCK;
	vtoqkey[VK_SCROLL] = K_SCRLOCK;
	vtoqkey[VK_SNAPSHOT] = K_PRINTSCR;
	vtoqkey[VK_LMENU] = K_LALT;
	vtoqkey[VK_RMENU] = K_RALT;
	vtoqkey[VK_LCONTROL] = K_LCTRL;
	vtoqkey[VK_RCONTROL] = K_RCTRL;
	vtoqkey[VK_LSHIFT] = K_LSHIFT;
	vtoqkey[VK_RSHIFT] = K_RSHIFT;
	vtoqkey[VK_LWIN] = K_LWIN;
	vtoqkey[VK_RWIN] = K_RWIN;
	vtoqkey[VK_APPS] = K_MENU;
	vtoqkey[VK_PAUSE] = K_PAUSE;

	vtoqkey[VK_MULTIPLY] = '*';
	vtoqkey[VK_ADD] = '+';
//	vtoqkey[VK_SEPARATOR] = K_ENTER;		// numpad enter, I think?
	vtoqkey[VK_SUBTRACT] = '-';
	vtoqkey[VK_DECIMAL] = '.';
	vtoqkey[VK_DIVIDE] = '/';
	vtoqkey[VK_CLEAR] = '5';		// numpad 5 when numlock is OFF

	vtoqkey[VK_OEM_PLUS] = '+';
	vtoqkey[VK_OEM_COMMA] = ',';
	vtoqkey[VK_OEM_MINUS] = '-';
	vtoqkey[VK_OEM_PERIOD] = '.';

	for (s = key_symbolchars; *s; s++)
	{
		vkey = VkKeyScanA (*s);
		if ((vkey & 0xFF00) == 0)		// no modifier bits set
		{
			vtoqkey[vkey] = *s;
		}
	}

	numexkeys = 0;
	for (i = 0; i < 256; i++)
	{
		vkey = MapVirtualKeyA (i, 1);		// scancode to vkeycode
		if (vkey && (vkey < VK_OEM_RESET) && !vtoqkey[vkey])		// above OEM_RESET are cellphone keys
		{
			vtoqkey[vkey] = K_EX1 + numexkeys++;
			if (numexkeys == MAX_EXKEYS)
				break;
		}
	}
}

/*
===========
IN_MapKey

Map from Windows notification codes to quake keynums
===========
*/
//int IN_MapKey (int key)
int IN_MapKey (WPARAM wParam, LPARAM lParam, int *keyshift)
{
	byte charbuf[2];
	UINT code;
	int count, key;
	qboolean capsdown;

	*keyshift = 0;

#ifdef _DEBUG
	if (wParam == VK_SEPARATOR)
		count = 1;

	if ((wParam != VK_SHIFT) && (wParam != VK_CONTROL) && (wParam != VK_MENU))
		*keyshift = 0;
//	MapVirtualKey (0x0D, 1);
#endif

	if (key_forceUSlayout)
	{
		if ((lParam & 0x01FF0000) == 0x01450000)
			key = K_NUMLOCK;
		else
		{
			code = (lParam >> 16) & 255;
			key = key_fromcode[code];
			if (lParam & 0x01000000)
			{
				if (key == K_LALT)
					key = K_RALT;
				else if (key == K_LCTRL)
					key = K_RCTRL;
			}
			
			if (key && keydown[K_SHIFT])
				*keyshift = key_shifttable[key];
		}
	}
	else
	{
		if (wParam == VK_MENU)
			wParam = (lParam & 0x01000000) ? VK_RMENU : VK_LMENU;

		else if (wParam == VK_CONTROL)
			wParam = (lParam & 0x01000000) ? VK_RCONTROL : VK_LCONTROL;

		else if (wParam == VK_SHIFT)
			wParam = (((lParam >> 16) & 255) == 0x36) ? VK_RSHIFT : VK_LSHIFT;

		capsdown = ((wParam != VK_CAPITAL) && GetKeyState (VK_CAPITAL)) ? true : false;

		if (keydown[K_SHIFT] || keydown[K_CTRL] || keydown[K_ALT] || capsdown)
		{
			code = (lParam >> 16) & 255;

			kbstate[VK_SHIFT] = keydown[K_SHIFT] ? 0x80 : 0;
			kbstate[VK_CONTROL] = keydown[K_CTRL] ? 0x80 : 0;
			kbstate[VK_MENU] = keydown[K_ALT] ? 0x80 : 0;
			kbstate[VK_CAPITAL] = capsdown ? 0x01 : 0;		// low-bit for toggle keys

		// this allows ctrl+shift+key in Con_MapColoredChars to work
			if (keydown[K_SHIFT] && keydown[K_CTRL] && !keydown[K_ALT])
				kbstate[VK_CONTROL] = 0;
			
			count = ToAscii (wParam, code, kbstate, (WORD *)charbuf, 0);
			if (count && (charbuf[0] < K_BACKSPACE) && (charbuf[0] > K_SPACE))
				*keyshift = charbuf[0];
		}

		if (wParam > 255)
			return 0;

		code = wParam;
		key = vtoqkey[code];
	}

	if (key == 0)
		Con_DPrintf ("key 0x%02x has no translation\n", code);

	return key;
}

/*
===========
IN_GetUnshiftedKey (JDH)

If the given character requires a shift-key combo, returns the Quake key value of the
"base" key (the one used in conjunction with shift).  Otherwise, returns 0.
===========
*/
/*int IN_GetUnshiftedKey (char c)
{
	short vkey;
	UINT code;

	vkey = VkKeyScanA (c);
	if ((vkey & 0xFF00) == 0x0100)		// if only shift-key bit is set
	{
		code = MapVirtualKeyA (vkey & 0x00FF, 0);		// 0 --> get scan code
		if (code && (code < 128) && scantokey[code])
			return scantokey[code];
	}

	return 0;
}
*/
#endif		//#ifndef RQM_SV_ONLY
