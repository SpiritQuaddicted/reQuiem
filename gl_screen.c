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
// gl_screen.c -- master for refresh, status bar, console, chat, notify, etc

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include <time.h>	// cl_clock
#include "movie.h"

#ifndef _WIN32
#include <ctype.h>
#endif

/*

background clear
rendering
turtle/net/ram icons
sbar
centerprint / slow centerprint
notify lines
intermission / finale overlay
loading plaque
console
menu

required background clears
required update regions


syncronous draw mode or async
One off screen buffer, with updates either copied or xblited
Need to double buffer?


async draw will require the refresh area to be cleared, because it will be
xblited, but sync draw can just ignore it.

sync
draw

CenterPrint ()
SlowPrint ()
Screen_Update ();
Con_Printf ();

net
turn off messages option

the refresh is always rendered, unless the console is full screen


console is:
	notify lines
	half
	full

*/


int		glx, gly, glwidth, glheight;

// only the refresh window will be updated unless these variables are flagged
// JDH: remnants from SW renderer?
//int		scr_copytop;
//int		scr_copyeverything;
//int		scr_fullupdate;


float		scr_con_current;
float		scr_conlines;		// lines of console to display

const char	*scr_notifystring;
qboolean	scr_drawdialog;

//float		oldscreensize, oldfov;
qboolean	OnChange_screenvar (cvar_t *var, const char *value);

cvar_t		scr_viewsize     = {"viewsize",         "100", CVAR_FLAG_ARCHIVE, OnChange_screenvar};
cvar_t		scr_fov          = {"fov",               "90", CVAR_FLAG_ARCHIVE, OnChange_screenvar};	// 10 - 170
cvar_t		scr_fov_adapt    = {"fov_adapt",          "1", CVAR_FLAG_ARCHIVE, OnChange_screenvar};
cvar_t		scr_consize      = {"scr_consize",      "0.8", CVAR_FLAG_ARCHIVE};		// by joe (JDH: changed default from 0.5)
cvar_t		scr_conspeed     = {"scr_conspeed",     "600", CVAR_FLAG_ARCHIVE};		// JDH: changed default from 1000
cvar_t		scr_centertime   = {"scr_centertime",     "2", CVAR_FLAG_ARCHIVE};
cvar_t		scr_showram      = {"showram",            "1", CVAR_FLAG_ARCHIVE};
cvar_t		scr_showturtle   = {"showturtle",         "0", CVAR_FLAG_ARCHIVE};
cvar_t		scr_showpause    = {"showpause",          "1", CVAR_FLAG_ARCHIVE};
cvar_t		scr_printspeed   = {"scr_printspeed",     "8", CVAR_FLAG_ARCHIVE};
cvar_t		gl_triplebuffer  = {"gl_triplebuffer",    "1"};

cvar_t		scr_sshot_format = {"scr_sshot_format", "jpg", CVAR_FLAG_ARCHIVE};

cvar_t		scr_clock        = {"cl_clock",           "0", CVAR_FLAG_ARCHIVE};
cvar_t		scr_clock_x      = {"cl_clock_x",         "0", CVAR_FLAG_ARCHIVE};
cvar_t		scr_clock_y      = {"cl_clock_y",        "-1", CVAR_FLAG_ARCHIVE};
cvar_t		scr_showspeed    = {"scr_showspeed",      "0", CVAR_FLAG_ARCHIVE};
cvar_t		scr_showfps      = {"scr_showfps",        "0", CVAR_FLAG_ARCHIVE};
cvar_t		scr_showorigin   = {"scr_showorigin",     "0", CVAR_FLAG_ARCHIVE};		// JDH

cvar_t		scr_printstats        = {"scr_printstats",          "0", CVAR_FLAG_ARCHIVE};	// JT022405 - default off for timer
cvar_t		scr_printstats_style  = {"scr_printstats_style",    "0"};
cvar_t		scr_printstats_length = {"scr_printstats_length", "0.5"};

cvar_t		cl_sbar          = {"cl_sbar",            "0", CVAR_FLAG_ARCHIVE, OnChange_screenvar};
cvar_t		scr_sbarsize     = {"scr_sbarsize",       "0", CVAR_FLAG_ARCHIVE, OnChange_screenvar};		// JDH
cvar_t		scr_hudscale     = {"scr_hudscale",       "1", CVAR_FLAG_ARCHIVE};		// JDH
cvar_t		scr_menusize     = {"scr_menusize",       "0", CVAR_FLAG_ARCHIVE};			// JDH
cvar_t		scr_centermenu   = {"scr_centermenu",     "1", CVAR_FLAG_ARCHIVE};


qboolean	scr_initialized;		// ready to draw

mpic_t          *scr_ram;
mpic_t          *scr_net;
mpic_t          *scr_turtle;

int		clearconsole;
int		clearnotify;

viddef_t	vid;				// global video state

vrect_t		scr_vrect;

qboolean	scr_disabled_for_loading;
qboolean	scr_drawloading;
float		scr_disabled_time;
double		scr_demo_overlay_time = 0;		// JDH: at what time demo overlay should disappear
const char	*scr_loadcaption = NULL;		// JDH: displayed below LOADING plaque
double		scr_sshot_time = 0;				// JDH: at what time screenshot should be taken
char		scr_sshot_filename[MAX_OSPATH];	// JDH: for time-delayed screenshots 

qboolean	block_drawing;

void SCR_ScreenShot_f (cmd_source_t src);
void SCR_SizeUp_f (cmd_source_t src);
void SCR_SizeDown_f (cmd_source_t src);


#ifdef HEXEN2_SUPPORT

  #define PLAQUE_WIDTH 26
  #define MAXLINES 27
  #define MAX_INFO 1024

  extern double	introTime;

  extern int	*pr_string_index;
  extern char	*pr_global_strings;
  extern int	pr_string_count;

  extern int	*pr_info_string_index;
  extern char	*pr_global_info_strings;
  extern int	 pr_info_string_count;
  extern char	*plaquemessage;
  extern cvar_t	scr_hudspeed;

  char			infomessage[MAX_INFO];
  static int	StartC[MAXLINES], EndC[MAXLINES];
  static int	lines;

  int			total_loading_size, current_loading_size, loading_stage;

  qboolean info_up = false;
  qboolean intro_playing = false;
  void SCR_Bottom_Plaque_Draw (const char *message);

#endif	// #ifdef HEXEN2_SUPPORT


/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	Cvar_RegisterInt (&scr_fov, 10, 170);
	Cvar_RegisterInt (&scr_fov_adapt, 0, 1);
	Cvar_RegisterInt (&scr_viewsize, 30, 120);
	Cvar_RegisterFloat (&scr_consize, 0, 1);	// by joe
	Cvar_RegisterInt (&scr_conspeed, 100, CVAR_UNKNOWN_MAX);
	Cvar_RegisterBool (&scr_showram);
	Cvar_RegisterBool (&scr_showturtle);
	Cvar_RegisterBool (&scr_showpause);
	Cvar_RegisterFloat (&scr_centertime, 0, 5);		// 5 as max is arbitrary
	Cvar_Register (&scr_printspeed);
	Cvar_RegisterBool (&gl_triplebuffer);
	Cvar_RegisterString (&scr_sshot_format);

	Cvar_RegisterInt (&scr_clock, 0, 4);
	Cvar_RegisterInt (&scr_clock_x, 0, vid.width/8 - 8);
	Cvar_RegisterInt (&scr_clock_y, -(int)vid.height/8, vid.height/8);		// neg val gives offset from bottom
	Cvar_RegisterBool (&scr_showspeed);
	Cvar_RegisterBool (&scr_showfps);
	Cvar_RegisterBool (&scr_showorigin);		// JDH

	Cvar_RegisterInt (&scr_printstats, 0, 4);
	Cvar_RegisterBool (&scr_printstats_style);
	Cvar_Register (&scr_printstats_length);

	Cvar_RegisterBool (&cl_sbar);		// by joe
	Cvar_RegisterFloat (&scr_sbarsize, 0, 100);
	Cvar_RegisterFloat (&scr_hudscale, 1, 3);		// JDH (range is somewhat arbitrary, used by menu only)
	Cvar_RegisterBool (&scr_centermenu);
	Cvar_RegisterFloat (&scr_menusize, 0, 100);

// register our commands
	Cmd_AddCommand ("screenshot", SCR_ScreenShot_f, 0);
	Cmd_AddCommand ("sizeup", SCR_SizeUp_f, 0);
	Cmd_AddCommand ("sizedown", SCR_SizeDown_f, 0);

//	SCR_LoadWadPics ();		// JDH: now done in Draw_Init

	Movie_Init ();

	vid.recalc_refdef = true;
	scr_initialized = true;
}

/*
=====================
OnChange_screenvar
  (viewsize, fov, fov_adapt, cl_sbar)
=====================
*/
qboolean OnChange_screenvar (cvar_t *var, const char *value)
{
#ifdef HEXEN2_SUPPORT
	if (hexen2 && (var == &scr_viewsize))
	{
		float oldsize = scr_viewsize.value;

		scr_viewsize.value = Q_atof (value);
		Sbar_ViewSizeChanged ();
		scr_viewsize.value = oldsize;
	}
#endif

	vid.recalc_refdef = true;
	return false;		// allow change
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


#ifdef HEXEN2_SUPPORT

/*
=====================
SCR_DrawTextBox2
=====================
*/
void SCR_DrawTextBox2 (int x, int y, int width, int lines, qboolean bottom)
{
	mpic_t	*p,*tm,*bm;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_GetCachePic ("gfx/box_tl.lmp", false);
	if (bottom)
		M_DrawTransPic (cx, cy, p);
	else
		M_DrawTransPic2 (cx, cy, p);

	p = Draw_GetCachePic ("gfx/box_ml.lmp", false);
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		if(bottom)
			M_DrawTransPic (cx, cy, p);
		else
			M_DrawTransPic2 (cx, cy, p);
	}

	p = Draw_GetCachePic ("gfx/box_bl.lmp", false);
	if (bottom)
		M_DrawTransPic (cx, cy+8, p);
	else
		M_DrawTransPic2 (cx, cy+8, p);

	// draw middle
	cx += 8;
	tm = Draw_GetCachePic ("gfx/box_tm.lmp", false);
	bm = Draw_GetCachePic ("gfx/box_bm.lmp", false);
	while (width > 0)
	{
		cy = y;

		if (bottom)
			M_DrawTransPic (cx, cy, tm);
		else
			M_DrawTransPic2 (cx, cy, tm);
		p = Draw_GetCachePic ("gfx/box_mm.lmp", false);
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_GetCachePic ("gfx/box_mm2.lmp", false);
			if(bottom)
				M_DrawTransPic (cx, cy, p);
			else
				M_DrawTransPic2 (cx, cy, p);
		}
		if (bottom)
			M_DrawTransPic (cx, cy+8, bm);
		else
			M_DrawTransPic2 (cx, cy+8, bm);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_GetCachePic ("gfx/box_tr.lmp", false);
	if (bottom)
		M_DrawTransPic (cx, cy, p);
	else
		M_DrawTransPic2 (cx, cy, p);

	p = Draw_GetCachePic ("gfx/box_mr.lmp", false);
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		if (bottom)
			M_DrawTransPic (cx, cy, p);
		else
			M_DrawTransPic2 (cx, cy, p);
	}

	p = Draw_GetCachePic ("gfx/box_br.lmp", false);
	if (bottom)
		M_DrawTransPic (cx, cy+8, p);
	else
		M_DrawTransPic2 (cx, cy+8, p);
}

/*
=====================
FindTextBreaks
=====================
*/
void FindTextBreaks (const char *message, int width)
{
	int length, pos, start, lastspace, oldlast;

	length = strlen (message);
	lines = pos = start = 0;
	lastspace = -1;

	while(1)
	{
		if (pos-start >= width || message[pos] == '@' || message[pos] == 0)
		{
			oldlast = lastspace;
			if (message[pos] == '@' || lastspace == -1 || message[pos] == 0)
				lastspace = pos;

			StartC[lines] = start;
			EndC[lines] = lastspace;
			lines++;
			if (lines == MAXLINES)
				return;
			if (message[pos] == '@')
				start = pos + 1;
			else if (oldlast == -1)
				start = lastspace;
			else
				start = lastspace + 1;

			lastspace = -1;
		}

		if (message[pos] == 32) lastspace = pos;
		else if (message[pos] == 0)
		{
			break;
		}

		pos++;
	}
}

/*
============
ReformatText
 - replaces '@' with '\n' and inserts '\n' when word-wrap occurs
============
*/
void ReformatText (char *text, int width)
{
	int		len, i;
	char	buf[MAXPRINTMSG];

	FindTextBreaks (text, width);
	len = 0;
	for (i=0; i < lines; i++)
	{
		//strncpy(buf + len, text + StartC[i], EndC[i] - StartC[i]);
		//len += EndC[i] - StartC[i];
		len += Q_strncpy (buf + len, sizeof(buf)-len, text + StartC[i], EndC[i] - StartC[i]);
		buf[len++] = '\n';
	}

	//strncpy(text, buf, len);
	//text[len-1] = 0;		// skip last '\n'
	Q_strncpy (text, MAXPRINTMSG, buf, len-1);		// skip last '\n'
}

/*
=====================
SCR_Print2
=====================
*/
void SCR_Print2 (int cx, int cy, const char *str)
{
	while (*str)
	{
		Draw_Character (cx + ((vid.width - 320)>>1), cy + ((vid.height - 200)>>1), ((unsigned char)(*str))+256);
		str++;
		cx += 8;
	}
}

#endif	// #ifdef HEXEN2_SUPPORT


char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int		scr_center_lines;
int		scr_erase_lines;
int		scr_erase_center;

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (const char *str)
{
	Q_strcpy (scr_centerstring, str, sizeof(scr_centerstring));
	scr_centertime_off = scr_centertime.value;
	scr_centertime_start = cl.time;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		FindTextBreaks(scr_centerstring, 38);
		scr_center_lines = lines;
		return;
	}
#endif

	// count the number of lines for centering
	scr_center_lines = 1;
	while (*str)
	{
		if (*str == '\n')
			scr_center_lines++;
		str++;
	}
}


/*
=====================
SCR_DrawCenterString
=====================
*/
void SCR_DrawCenterString (void)
{
	char	*start;
	int		len, j, x, y, remaining;

// the finale prints the characters one at a time
	if (cl.intermission)
		remaining = scr_printspeed.value * (cl.time - scr_centertime_start);
	else
		remaining = 9999;

	scr_erase_center = 0;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		char	temp[80];

		FindTextBreaks (scr_centerstring, 38);
		y = ((25-lines) * 8) / 2;

		for (j=0; j<lines; j++, y+=8)
		{
			len = Q_strncpy (temp, sizeof(temp), &scr_centerstring[StartC[j]], EndC[j]-StartC[j]);
			//temp[EndC[j]-StartC[j]] = 0;
			//x = ((40-strlen(temp)) * 8) / 2;
			x = ((40-len)*8)/2;
	  		SCR_Print2 (x, y, temp);
		}

		return;
	}
#endif

	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = vid.height*0.35;
	else
		y = 48;

	do
	{
	// scan the width of the line
		for (len=0 ; len<40 ; len++)
			if (start[len] == '\n' || !start[len])
				break;
		x = (vid.width - len*8) / 2;
		for (j=0 ; j<len ; j++, x+=8)
		{
			Draw_Character (x, y, start[j]);
			if (!remaining--)
				return;
		}

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

/*
=====================
SCR_CheckDrawCenterString
=====================
*/
void SCR_CheckDrawCenterString (void)
{
//	scr_copytop = 1;
	if (scr_center_lines > scr_erase_lines)
		scr_erase_lines = scr_center_lines;

	scr_centertime_off -= host_frametime;

	if (scr_centertime_off <= 0 && !cl.intermission)
		return;
	if (key_dest != key_game)
		return;

#ifdef HEXEN2_SUPPORT
	if (intro_playing)
		SCR_Bottom_Plaque_Draw (scr_centerstring);
	else
#endif
		SCR_DrawCenterString ();
}

#ifdef HEXEN2_SUPPORT

/*
=====================
SCR_Plaque_Draw
=====================
*/
void SCR_Plaque_Draw (const char *message, qboolean AlwaysDraw)
{
	int		i, len;
	char	temp[80];
	int		bx, by;

	if (scr_con_current == vid.height && !AlwaysDraw)
		return;		// console is full screen

	if (!*message)
		return;

	M_SetScale (true);		// set up menu vars, since DrawTextBox2 uses menu funcs
	FindTextBreaks (message, PLAQUE_WIDTH);

	by = (200 - lines*DRAW_CHARHEIGHT) / 2;
	SCR_DrawTextBox2 (32, by-2*DRAW_CHARHEIGHT, 30, lines+2, false);

	for (i=0; i<lines; i++, by+=DRAW_CHARHEIGHT)
	{
		//strncpy (temp,&message[StartC[i]],EndC[i]-StartC[i]);
		//temp[EndC[i]-StartC[i]] = 0;
		//bx = ((40-strlen(temp)) * 8) / 2;
		len = Q_strncpy (temp, sizeof(temp), &message[StartC[i]], EndC[i]-StartC[i]);
		bx = (320 - len*DRAW_CHARWIDTH) / 2;
	  	SCR_Print2 (bx, by, temp);
	}

	M_SetScale (false);
}

/*
=====================
SCR_Bottom_Plaque_Draw
=====================
*/
void SCR_Bottom_Plaque_Draw (const char *message)
{
	int		i, len;
	char	temp[80];
	int		bx, by;

	if (!*message)
		return;

//	scr_needfull = true;

	M_SetScale (true);		// set up menu vars, since DrawTextBox2 uses menu funcs
	FindTextBreaks (message, PLAQUE_WIDTH);

	by = vid.height - (lines+2)*DRAW_CHARHEIGHT;

	SCR_DrawTextBox2 (32, by-2*DRAW_CHARHEIGHT, 30, lines+2, true);

	for (i=0; i<lines; i++, by+=DRAW_CHARHEIGHT)
	{
		//strncpy (temp,&message[StartC[i]],EndC[i]-StartC[i]);
		//temp[EndC[i]-StartC[i]] = 0;
		//bx = ((40-strlen(temp)) * 8) / 2;
		len = Q_strncpy (temp, sizeof(temp), &message[StartC[i]], EndC[i]-StartC[i]);
		bx = (320 - len*DRAW_CHARWIDTH) / 2;
	  	M_Print (bx, by, temp);
	}

	M_SetScale (false);
}

/*
=====================
SCR_Info_Plaque_Draw
=====================
*/
void SCR_Info_Plaque_Draw (const char *message)
{
	int		i, len;
	char	temp[80];
	int		bx, by;

	if (scr_con_current == vid.height)
		return;		// console is full screen

	if (!*message)
		return;

//	scr_needfull = true;

	M_SetScale (true);		// set up menu vars, since DrawTextBox2 uses menu funcs
	FindTextBreaks (message, PLAQUE_WIDTH+4);

	if (lines == MAXLINES)
	{
		Con_DPrintf ("Info_Plaque_Draw: line overflow error\n");
		lines = MAXLINES-1;
	}

	by = (200 - lines*DRAW_CHARHEIGHT) / 2;
	SCR_DrawTextBox2 (15, by-2*DRAW_CHARHEIGHT, PLAQUE_WIDTH+4+4, lines+2,false);

	for (i=0; i<lines; i++, by+=DRAW_CHARHEIGHT)
	{
		//strncpy(temp,&message[StartC[i]],EndC[i]-StartC[i]);
		//temp[EndC[i]-StartC[i]] = 0;
		//bx = ((40-strlen(temp)) * 8) / 2;
		len = Q_strncpy (temp, sizeof(temp), &message[StartC[i]], EndC[i]-StartC[i]);
	  	bx = (320 - len*DRAW_CHARWIDTH) / 2;
		SCR_Print2 (bx, by, temp);
	}

	M_SetScale (false);
}

/*
=====================
UpdateInfoMessage
=====================
*/
void UpdateInfoMessage (void)
{
	unsigned int len, i, check;
	char *newmessage;

	len = Q_strcpy (infomessage, "Objectives:", sizeof(infomessage));

	if (!pr_global_info_strings)
		return;

	for (i = 0; i < 32; i++)
	{
		check = (1 << i);

		if (cl.info_mask & check)
		{
			newmessage = &pr_global_info_strings[pr_info_string_index[i]];
			len += Q_strcpy (infomessage+len, "@@", sizeof(infomessage)-len);
			len += Q_strcpy (infomessage+len, newmessage, sizeof(infomessage)-len);
		}
	}

	for (i = 0; i < 32; i++)
	{
		check = (1 << i);

		if (cl.info_mask2 & check)
		{
			newmessage = &pr_global_info_strings[pr_info_string_index[i+32]];
			len += Q_strcpy (infomessage+len, "@@", sizeof(infomessage)-len);
			len += Q_strcpy (infomessage+len, newmessage, sizeof(infomessage)-len);
		}
	}
}

#endif	// #ifdef HEXEN2_SUPPORT

//=============================================================================

/*
====================
CalcFov

Given an FOV value for a reference dimension, calculate the FOV value for
another dimension that would preserve the subtended angles of objects. 
====================
*/
float CalcFov (float fov_ref, float dim_ref, float dim_other)
{
        float   a;

        if (fov_ref < 1 || fov_ref > 179)
                Sys_Error ("Bad fov: %f", fov_ref);

        a = atan(dim_other / dim_ref * tan(fov_ref / 360 * M_PI));
        a = a * 360 / M_PI;
        return a;
}

/*
=================
SCR_CalcRefdef

Must be called whenever vid changes
Internal use only
=================
*/
static void SCR_CalcRefdef (void)
{
	float		size;
	int			sbar_height;
	qboolean	full = false;

//	scr_fullupdate = 0;		// force a background redraw
	vid.recalc_refdef = 0;

// force the status bar to redraw
	Sbar_Changed ();

//========================================

// bound viewsize
	if (scr_viewsize.value < scr_viewsize.minvalue)
		Cvar_SetValueDirect (&scr_viewsize, scr_viewsize.minvalue);
	else if (scr_viewsize.value > scr_viewsize.maxvalue)
		Cvar_SetValueDirect (&scr_viewsize, scr_viewsize.maxvalue);

// bound field of view
	if (scr_fov.value < scr_fov.minvalue)
		Cvar_SetValueDirect (&scr_fov, scr_fov.minvalue);
	else if (scr_fov.value > scr_fov.maxvalue)
		Cvar_SetValueDirect (&scr_fov, scr_fov.maxvalue);

// intermission is always full screen
	if (cl.intermission)
	{
		full = true;
		size = 100.0;
//		sb_lines = 0;
	}
	else
	{
		size = scr_viewsize.value;
/*
		if (size >= scr_viewsize.maxvalue)
			sb_lines = 0;		// no status bar at all
		else if (size >= scr_viewsize.maxvalue-10)
			sb_lines = 1;		// no inventory
		else
			sb_lines = 2;
*/
		if (size >= 100.0)
		{
			full = true;
			size = 100.0;
		}
	}

	size /= 100.0;

	r_refdef.vrect.width = vid.width * size;
	if (r_refdef.vrect.width < 96)
	{
		size = 96.0 / r_refdef.vrect.width;
		r_refdef.vrect.width = 96;	// min for icons
	}

	/*r_refdef.vrect.height = (vid.height * size) - Sbar_Height();

	if (cl_sbar.value || !full)
	{
		if (r_refdef.vrect.height > vid.height - sb_lines)
			r_refdef.vrect.height = vid.height - sb_lines;
	}
	else if (r_refdef.vrect.height > vid.height)
	{
		r_refdef.vrect.height = vid.height;
	}*/

	sbar_height = ((cl_sbar.value || scr_viewsize.value < 100) ? Sbar_Height() : 0);
		
	r_refdef.vrect.height = (vid.height - sbar_height) * size;
	
	r_refdef.vrect.x = (vid.width - r_refdef.vrect.width) / 2;

	if (full)
		r_refdef.vrect.y = 0;
	else
//		r_refdef.vrect.y = (vid.height - sb_lines - r_refdef.vrect.height) / 2;
		r_refdef.vrect.y = (vid.height - sbar_height - r_refdef.vrect.height) / 2;

	if (scr_fov_adapt.value)
	{
		// "auto" FOV: treat fov value as horizontal FOV for 4:3 aspect ratio
		// 1) Find what the width dimension would be for the current height,
		//    if the aspect ratio were 4:3.
		float width_4_3 = r_refdef.vrect.height * 4.0 / 3.0;
		// 2) Calculate vertical FOV from the fov value and the "4:3 width".
		r_refdef.fov_y = CalcFov (scr_fov.value, width_4_3, r_refdef.vrect.height);
		// 3) Calculate actual horizontal FOV from vertical FOV.
		r_refdef.fov_x = CalcFov (r_refdef.fov_y, r_refdef.vrect.height, r_refdef.vrect.width);	
	}
	else
	{
		// "manual" FOV: treat fov value as horizontal FOV.
		// 1) Calculate vertical FOV from the fov value and the width.
		r_refdef.fov_y = CalcFov (scr_fov.value, r_refdef.vrect.width, r_refdef.vrect.height);
		// 2) Set horizontal FOV directly.
		r_refdef.fov_x = scr_fov.value;
	}

	scr_vrect = r_refdef.vrect;
}

/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (cmd_source_t src)
{
	Cvar_SetValueDirect (&scr_viewsize, scr_viewsize.value + 10);
	vid.recalc_refdef = 1;
}

/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (cmd_source_t src)
{
	Cvar_SetValueDirect (&scr_viewsize, scr_viewsize.value - 10);
	vid.recalc_refdef = 1;
}

//============================================================================

/*
==================
SCR_ClearWadPics
==================
*/
void SCR_ClearWadPics (void)
{
	scr_ram = scr_net = scr_turtle = NULL;
}

/*
==================
SCR_LoadWadPics
==================
*/
void SCR_LoadWadPics (void)
{
	scr_ram = Draw_PicFromWad ("ram");
	scr_net = Draw_PicFromWad ("net");
	scr_turtle = Draw_PicFromWad ("turtle");
}

/*
==============
SCR_DrawRam
==============
*/
void SCR_DrawRam (void)
{
	if (!scr_showram.value)
		return;

	if (!r_cache_thrash)
		return;

	if (scr_ram)
		Draw_Pic (scr_vrect.x+32, scr_vrect.y, scr_ram);
}

/*
==============
SCR_DrawTurtle
==============
*/
void SCR_DrawTurtle (void)
{
	static	int	count;

	if (!scr_showturtle.value)
		return;

	if (host_frametime < 0.1)
	{
		count = 0;
		return;
	}

	count++;
	if (count < 3)
		return;

	if (scr_turtle)
		Draw_Pic (scr_vrect.x, scr_vrect.y, scr_turtle);
}

/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void)
{
	if (realtime - cl.last_received_message < 0.3)
		return;

	if (cls.demoplayback)
		return;

	if (scr_net)
		Draw_Pic (scr_vrect.x+64, scr_vrect.y, scr_net);
}

/*
==============
DrawPause
==============
*/

void SCR_DrawPause (void)
{
	mpic_t	*pic;

	if (!scr_showpause.value)		// turn off for screenshots
		return;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		static float pause_percent = 0;
		float val;

		if (!cl.paused)
		{
			pause_percent = 0;
			return;
		}

		if (pause_percent < 1)
		{
			if (scr_hudspeed.value > 0)
			{
				val = ((1-pause_percent) * scr_hudspeed.value) * host_frametime;
				val = bound( 0.004, val, 1 );
				pause_percent += val;
			}
			else pause_percent = 1;
		}

		pic = Draw_GetCachePic ("gfx/menu/paused.lmp", false);
		if (pic)
		{
			val = ((float)pic->height * pause_percent) - pic->height;
			Draw_Pic ( (vid.width - pic->width) / 2, val, pic);
			return;
		}
	}
	else
#endif
	{
		if (!cl.paused)
			return;

		pic = Draw_GetCachePic ("gfx/pause.lmp", false);
		if (pic)
		{
			Draw_Pic ((vid.width - pic->width) / 2, (vid.height - 48 - pic->height) / 2, pic);
			return;
		}
	}

// last resort if no pic found:
	M_Print ((vid.width - 48) / 2, (vid.height - 54) / 2, "Paused");
}

#ifdef HEXEN2_SUPPORT
/*
==============
SCR_UpdateLoadProgress
==============
*/
void SCR_UpdateLoadProgress (int x)
{
	int size, count;

	if (total_loading_size)
		size = current_loading_size * 106 / total_loading_size;
	else
		size = 0;

	if (loading_stage == 1)
		count = size;
	else
		count = 106;

	Draw_Fill (x+42, 87,   count, 1, 136);
	Draw_Fill (x+42, 87+1, count, 4, 138);
	Draw_Fill (x+42, 87+5, count, 1, 136);

	if (loading_stage == 2)
		count = size;
	else
		count = 0;

	Draw_Fill (x+42, 97,   count, 1, 168);
	Draw_Fill (x+42, 97+1, count, 4, 170);
	Draw_Fill (x+42, 97+5, count, 1, 168);
}

#endif	// #ifdef HEXEN2_SUPPORT

/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
	mpic_t	*pic;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		int		x;

		pic = Draw_GetCachePic ("gfx/menu/loading.lmp", false);
		if (pic)
		{
			x = (vid.width - pic->width) / 2;
			Draw_Pic (x, 0, pic);

			if (loading_stage > 0)
				SCR_UpdateLoadProgress (x);
		}

		return;
	}
#endif

	if (!scr_drawloading)
		return;

	pic = Draw_GetCachePic ("gfx/loading.lmp", false);
	if (pic)
	{
		Draw_Pic ((vid.width - pic->width) / 2, (vid.height - 48 - pic->height) / 2, pic);

		if (scr_loadcaption)
			Draw_String ((vid.width - strlen(scr_loadcaption)*8)/2, (vid.height - 40 + pic->height)/2, scr_loadcaption);
	}
	else
		M_Print ((vid.width - 48) / 2, (vid.height - 54) / 2, "Loading...");
}

void SCR_UpdateLoadCaption (const char *caption)
{
#ifdef FIXME
	mpic_t	*pic;
	int width, left, top;

	if (!vid_hwgamma_enabled && (v_contrast.value > 1.0))
		return;

	if (!scr_disabled_for_loading || !caption)
		return;
	
	if (scr_loadcaption && !strcmp(scr_loadcaption, caption))
		return;

	scr_loadcaption = caption;
	pic = Draw_GetCachePic ("gfx/loading.lmp", false);
	if (pic)
	{
		GL_Set2D ();

		width = strlen(scr_loadcaption)*8;
		left = (vid.width - width)/2;
		top = (vid.height - 40 + pic->height)/2;
		Draw_String (left, top, scr_loadcaption);

		R_BrightenRect (left, top, width, 8);
		
		GL_EndRendering ();
	}
#endif
}

#ifdef HEXEN2_SUPPORT
void SCR_ShowLoadingSize (void)
{
	if (!scr_initialized)
		return;

	if (!scr_drawloading && (loading_stage == 0))
		return;

	glDrawBuffer  (GL_FRONT);
	SCR_DrawLoading();
	glDrawBuffer  (GL_BACK);
}
#endif

//=============================================================================

extern qboolean M_Quit_OverConsole (void);

/*
==================
SCR_SetUpToDrawConsole
==================
*/
void SCR_SetUpToDrawConsole (void)
{
	double	frametime;
	int		lines;

	Con_CheckResize ();

	if (scr_drawloading)
		return;		// never a console with loading plaque

// decide on the height of the console
	con_forcedup = !cl.worldmodel || cls.signon != SIGNONS;

	if (con_forcedup)
	{
		scr_conlines = vid.height;		// full screen
		scr_con_current = scr_conlines;
	}
	else if ((key_dest == key_console)		// by joe
		|| M_Quit_OverConsole ())		// JDH
	{
		scr_conlines = vid.height * scr_consize.value;
		if (scr_conlines < 30)
			scr_conlines = 30;
		if (scr_conlines > vid.height - 10)
			scr_conlines = vid.height - 10;
	}
	else
	{
		scr_conlines = 0;			// none visible
	}

	if (scr_conlines != scr_con_current)
	{
		frametime = host_frametime;
		if (cls.demoplayback)
			frametime /= (double)cl_demospeed.value;

		lines = scr_conspeed.value * frametime * vid.height / 320;

		if (scr_conlines < scr_con_current)
		{
			scr_con_current -= lines;
			if (scr_conlines > scr_con_current)
				scr_con_current = scr_conlines;
		}
		else
		{
			scr_con_current += lines;
			if (scr_conlines < scr_con_current)
				scr_con_current = scr_conlines;
		}
	}

	if (clearconsole++ < vid.numpages)
		Sbar_Changed ();
	else if (clearnotify++ >= vid.numpages)
		con_notifylines = 0;
}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void)
{
	if (scr_con_current)
	{
//		scr_copyeverything = 1;
		Con_DrawConsole (scr_con_current, (!scr_drawdialog || scr_notifystring));
		clearconsole = 0;
	}
	else
	{
		if (key_dest == key_game || key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}

static const char *time_formats[] =
{
	"%I:%M:%S %p",			// eg. 08:25:07 PM
	"%H:%M:%S",				// eg. 20:25:07
	"%b %d %I:%M:%S %p",	// eg. Nov 04 08:25:07 PM
	"%b %d %H:%M:%S"		// eg. Nov 04 20:25:07
};

/*
==============
SCR_DrawLocalTime
==============
*/
void SCR_DrawLocalTime (void)
{
	int		pos;
	time_t	ltime;
	char	str[64];

	pos = bound(0, (int) scr_clock.value, 4);
	if (!pos)
		return;

	time (&ltime);
	strftime (str, sizeof(str)-1, time_formats[pos-1], localtime(&ltime));

	if (scr_clock_y.value < 0)
	{
		//pos = vid.height - sb_lines + 8*scr_clock_y.value;
		pos = vid.height - 4 + 8*scr_clock_y.value;
	}
	else
		pos = 8*scr_clock_y.value;

	if ((pos >= vid.height-12) && (scr_clock_x.value > vid.width/8 - 28))
	{
		if (scr_showfps.value)
			pos -= 12;
		if (scr_showorigin.value)
			pos -= 12;
	}

	Draw_String (8*scr_clock_x.value, pos, str);
}

/*
==============
SCR_DrawOrigin (JDH)
==============
*/
void SCR_DrawOrigin (void)
{
	float		*org;
	char		str[32];
	int			x, y;

	if (!scr_showorigin.value)
		return;

	org = cl_entities[cl.viewentity].msg_origin;
	Q_snprintfz (str, sizeof(str), "%.0f, %.0f, %.0f", org[0], org[1], org[2]);

	x = vid.width - strlen(str)*8 - 8;
	y =  vid.height - 12;
	if (scr_showfps.value)
		y -= 12;

	Draw_String (x, y, str);
}

/*
==============
SCR_DrawFPS
==============
*/
// joe: from FuhQuake
void SCR_DrawFPS (void)
{
	static	float	lastframetime;
	float		t;
	extern	int	fps_count;
	static	float	lastfps;
	int		x, y;
	char		st[80];

	if (!scr_showfps.value)
		return;

	t = Sys_DoubleTime ();
	if ((t - lastframetime) >= 1.0)
	{
		lastfps = fps_count / (t - lastframetime);
		fps_count = 0;
		lastframetime = t;
	}

	sprintf (st, "%3.1f FPS", lastfps + 0.05);
	x = vid.width - strlen(st) * 8 - 8;
	//y = vid.height - sb_lines - 8;
//	y = vid.height - sb_lines;	// JT021305 - lower fps a little to avoid overlap
	y = vid.height - 12;
	Draw_String (x, y, st);
}

/*
==============
SCR_DrawSpeed
==============
*/
#define SPEEDBAR_WIDTH   160
#define SPEEDBAR_HEIGHT   13
// joe: from [sons]Quake
void SCR_DrawSpeed (void)
{
	int			x, y;
	char		st[8];
	vec3_t		vel;
	float		speed, speedunits;
	static	float	maxspeed = 0;
	static	float	display_speed = -1;
	static	double	lastrealtime = 0;

	if (!scr_showspeed.value)
		return;

	if (lastrealtime > realtime)
	{
		lastrealtime = 0;
		display_speed = -1;
		maxspeed = 0;
	}

	VectorCopy (cl.velocity, vel);
	vel[2] = 0;
	speed = VectorLength (vel);

	if (speed > maxspeed)
		maxspeed = speed;

	if (display_speed >= 0)
	{
		sprintf (st, "%3d", (int)display_speed);

		x = (vid.width - SPEEDBAR_WIDTH) / 2;
		y = vid.height - Sbar_Height() - SPEEDBAR_HEIGHT - 3;

		/*if ((scr_viewsize.value >= scr_viewsize.maxvalue) || cl.intermission)
			y = vid.height - 16;

		else if ((scr_viewsize.value >= scr_viewsize.maxvalue-10) ||
				((scr_viewsize.value >= scr_viewsize.maxvalue-20) && !cl_sbar.value))
			y = vid.height - 8*5;
		else
			y = vid.height - 8*8;*/

		Draw_Fill (x, y-1, SPEEDBAR_WIDTH, SPEEDBAR_HEIGHT-2, 10);		// light grey frame
//		Draw_Fill (x, y+9, SPEEDBAR_WIDTH, 1, 10);
		Draw_Fill (x +   SPEEDBAR_WIDTH/5, y-2, 1, SPEEDBAR_HEIGHT, 10);		// mark for each 100
		Draw_Fill (x + 2*SPEEDBAR_WIDTH/5, y-2, 1, SPEEDBAR_HEIGHT, 10);
		Draw_Fill (x + 3*SPEEDBAR_WIDTH/5, y-2, 1, SPEEDBAR_HEIGHT, 10);
		Draw_Fill (x + 4*SPEEDBAR_WIDTH/5, y-2, 1, SPEEDBAR_HEIGHT, 10);

		Draw_Fill (x, y, SPEEDBAR_WIDTH, SPEEDBAR_HEIGHT-4, 52);		// dark green background

		speedunits = display_speed;
		if (display_speed <= 500)
			Draw_Fill (x, y, (int)((display_speed/500.0)*SPEEDBAR_WIDTH), SPEEDBAR_HEIGHT-4, 100);		// orange
		else
		{
			while (speedunits > 500)
				speedunits -= 500;
			Draw_Fill (x, y, (int)((speedunits/500.0)*SPEEDBAR_WIDTH), SPEEDBAR_HEIGHT-4, 68);		// red
		}
		Draw_String (x + 36 - strlen(st) * 8, y, st);
	}

	if (realtime - lastrealtime >= 0.1)
	{
		lastrealtime = realtime;
		display_speed = maxspeed;
		maxspeed = 0;
	}
}


// added by joe
float	printstats_limit;

/*
===============
SCR_DrawStats
  time, secrets & kills drawing to the top right corner -- joe
===============
*/
void SCR_DrawStats (void)
{
	const char *str;
//extern void Sbar_IntermissionNumber (int x, int y, int num, int digits, qboolean altcolor);

/*	int	mins, secs, tens;

extern mpic_t		*sb_nums[2][STAT_MINUS+1];
extern mpic_t		*sb_colon;
*/
	if (!scr_printstats.value)
		return;

	if ((scr_printstats.value == 3 || scr_printstats.value == 4) && printstats_limit < cl.time)
		return;

	str = Sbar_FormatTime (cl.ctime, true);
/*
	mins = cl.ctime / 60;
	secs = cl.ctime - 60*mins;
	tens = (int)(cl.ctime * 10) % 10;
*/
	if (!scr_printstats_style.value)
	{
		Draw_BigNumString (vid.width-2, 0, str, DRAWNUM_TIGHT | DRAWNUM_ALIGNRIGHT);
/*
		Sbar_IntermissionNumber (vid.width - 140, 0, mins, 2, false);

		Draw_TransPic (vid.width - 92, 0, sb_colon);
		Draw_TransPic (vid.width - 80, 0, sb_nums[0][secs/10]);
		Draw_TransPic (vid.width - 58, 0, sb_nums[0][secs%10]);

		Draw_TransPic (vid.width - 36, 0, sb_colon);
		Draw_TransPic (vid.width - 24, 0, sb_nums[0][tens]);
*/
		if (scr_printstats.value == 2 || scr_printstats.value == 4)
		{
			str = va("%i", cl.stats[STAT_SECRETS]);
			Draw_BigNumString (vid.width-2, DRAW_BIGCHARHEIGHT, str, DRAWNUM_TIGHT | DRAWNUM_ALIGNRIGHT);

			str = va("%i", cl.stats[STAT_MONSTERS]);
			Draw_BigNumString (vid.width-2, 2*DRAW_BIGCHARHEIGHT, str, DRAWNUM_TIGHT | DRAWNUM_ALIGNRIGHT);

//			Sbar_IntermissionNumber (vid.width - 2*DRAW_BIGNUMWIDTH, DRAW_BIGNUMWIDTH, cl.stats[STAT_SECRETS], 2, false);
//			Sbar_IntermissionNumber (vid.width - 3*DRAW_BIGNUMWIDTH, 2*DRAW_BIGNUMWIDTH, cl.stats[STAT_MONSTERS], 3, false);

//			Sbar_IntermissionNumber (vid.width - 48, 24, cl.stats[STAT_SECRETS], 2, false);
//			Sbar_IntermissionNumber (vid.width - 72, 48, cl.stats[STAT_MONSTERS], 3, false);
		}
	}
	else
	{
		Draw_String (vid.width - strlen(str)*DRAW_CHARWIDTH, 1, str);
//		Draw_String (vid.width - 56, 0, va("%2i:%02i:%i", mins, secs, tens));
		if (scr_printstats.value == 2 || scr_printstats.value == 4)
		{
			Draw_String (vid.width - 2*DRAW_CHARWIDTH, DRAW_CHARHEIGHT + 1, va("%2i", cl.stats[STAT_SECRETS]));
			Draw_String (vid.width - 3*DRAW_CHARWIDTH, 2*DRAW_CHARHEIGHT + 1, va("%3i", cl.stats[STAT_MONSTERS]));

//			Draw_String (vid.width - 16, 8, va("%2i", cl.stats[STAT_SECRETS]));
//			Draw_String (vid.width - 24, 16, va("%3i", cl.stats[STAT_MONSTERS]));
		}
	}
}

extern float CL_GetDemoProgress (void);

#define SCR_DEMO_HELPSTR "Press    to view keyboard controls for demo"

/*
=========================
SCR_DrawDemoOverlay (JDH)
=========================
*/
void SCR_DrawDemoOverlay (void)
{
	double	elapsed;
	char	*name, str[16];
	mpic_t	*p;
	int		width, x;
	float	progress;

	if (!cls.demoplayback || (scr_demo_overlay_time == 0) || cls.capturedemo)
		return;

	elapsed = realtime - scr_demo_overlay_time;

	// show overlay only during 2s after keypress
	if (elapsed > 2)
		return;

	if (cl_demorewind.value)
		name = "gfx/demo_rw.lmp";
	else
		name = "gfx/demo_fw.lmp";

	// solid for first 1.5s, then fade out
	if (elapsed > 1.5)
	{
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glColor4f (1, 1, 1, 2*(2-elapsed));
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	p = Draw_GetCachePic (name, false);
	if (p)
	{
		width = p->width;
		Draw_TransPic (vid.width - 16 - width, 12, p);
	}
	else width = 32;

	x = Q_snprintfz (str, sizeof(str), "%d%%", (int)(cl_demospeed.value * 100));
	Draw_String (vid.width - 16 - (width + x*8)/2, 4, str);

	p = Draw_GetCachePic ("gfx/demo_bar.lmp", false);
	if (p)
	{
		progress = CL_GetDemoProgress ();
		x = vid.width - 16 - p->width;
		//Draw_Fill (x+1, 45, (int)(progress*(p->width-2)), p->height, 249);
		//Draw_TransPic (x, 45, p);
		Draw_SubPic (x+1, 45, p, 1, p->height/2, (int)(progress*(p->width-2)), p->height/2);
		Draw_SubPic (x, 45, p, 0, 0, p->width, p->height/2);
	}
/*
	x = (vid.width - strlen(SCR_DEMO_HELPSTR)*8) / 2;
	Draw_Alt_String (x, 10, SCR_DEMO_HELPSTR);
	Draw_String (x + 6*8, 10, "F1");
*/
	if (elapsed > 1.5)
	{
		glEnable (GL_ALPHA_TEST);
		glDisable (GL_BLEND);
		glColor4f (1, 1, 1, 1);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
}

/*
==============================================================================

				SCREEN SHOTS

==============================================================================
*/

// stuff added from FuhQuake - joe
extern	unsigned short	ramps[3][256];

void ApplyGamma (byte *buffer, int size)
{
	int	i;

	if (!vid_hwgamma_enabled)
		return;

	for (i=0 ; i<size ; i+=3)
	{
		buffer[i+0] = ramps[0][buffer[i+0]] >> 8;
		buffer[i+1] = ramps[1][buffer[i+1]] >> 8;
		buffer[i+2] = ramps[2][buffer[i+2]] >> 8;
	}
}

void SCR_ScreenShot (char *name)
{
	int			buffersize = glwidth * glheight * 3;
	byte		*buffer;
	qboolean	ok;

	buffer = Q_malloc (buffersize);
	glReadPixels (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	ApplyGamma (buffer, buffersize);

	ok = Image_WriteFile (name, buffer, glwidth, glheight);

	free (buffer);

	Con_Printf ("%s %s\n", ok ? "Wrote" : "Couldn't write", name);
}

const char *scr_sshot_exts[SCR_NUM_SSHOT_EXTS] = {"tga", "png", "jpg"};

/*
==================
SCR_ScreenShot_f
  screenshot <filename> <delay>
==================
*/
void SCR_ScreenShot_f (cmd_source_t src)
{
	int		namelen, i, j;
	float	delay;
	char	fname[MAX_OSPATH], *ext;

	i = Cmd_Argc ();
	if (i > 3)
	{
		Con_Printf ("Usage: %s [filename] [delay]\n", Cmd_Argv(0));
		return;
	}

	if (i == 3)
		delay = atof (Cmd_Argv(2));
	else
		delay = 0;
	
	if (i > 1)
	{
		namelen = Q_strcpy (fname, Cmd_Argv(1), sizeof(fname));
		ext = COM_FileExtension (fname);
	}
	else
	{
	// find a file name to save it to

		char fullname[MAX_OSPATH];
		
		if (cl.worldmodel && (cls.signon == SIGNONS))
			Q_snprintfz (fname, sizeof(fname), "%s_", Host_MapName ());
		else
			Q_strcpy (fname, "reQuiem", sizeof(fname));

		namelen = Q_snprintfz (fullname, sizeof(fullname), "%s/%s", com_gamedir, fname);

		for (i=0 ; i<1000 ; i++)
		{
			// check if the current sshot # exists with any known extension
			for (j = 0; j < SCR_NUM_SSHOT_EXTS; j++)
			{
				Q_snprintfz (fullname+namelen, sizeof(fullname)-namelen, "%03i.%s", i, scr_sshot_exts[j]);
				if (COM_FileExists(fullname))
					break;	// file already exists with this extension
			}

			if (j == SCR_NUM_SSHOT_EXTS)
				break;		// found an open slot
		}

		if (i == 1000)
		{
			Con_Printf ("\x02""Error: Cannot create more than 1000 screenshots\n");
			return;
		}

		namelen = strlen (fname);
		namelen += Q_snprintfz (fname + namelen, sizeof(fname)-namelen, "%03i", i);
		ext = NULL;
	}

	if (!ext || !*ext)
	{
		if (!Q_strcasecmp(scr_sshot_format.string, "jpg") || !Q_strcasecmp(scr_sshot_format.string, "jpeg"))
			ext = "jpg";
		else if (!Q_strcasecmp(scr_sshot_format.string, "png"))
			ext = "png";
		else
			ext = "tga";

		Q_snprintfz (fname + namelen, sizeof(fname) - namelen, ".%s", ext);
	}
	
	if (delay > 0)
	{
		scr_sshot_time = realtime + delay;
		Q_strcpy (scr_sshot_filename, fname, sizeof(scr_sshot_filename));
	}
	else
		SCR_ScreenShot (fname);
}

//=============================================================================

/*
================
SCR_BeginLoadingPlaque
================
*/
void SCR_BeginLoadingPlaque (const char *caption)
{
	S_StopAllSounds (true);

	if (cls.state != ca_connected || cls.signon != SIGNONS)
		return;

// redraw with no console and the loading plaque
	Con_ClearNotify ();
	scr_centertime_off = 0;
	scr_con_current = 0;
	scr_loadcaption = caption;

	scr_drawloading = true;
//	scr_fullupdate = 0;
	Sbar_Changed ();
	SCR_UpdateScreen ();
	scr_drawloading = false;

	scr_disabled_for_loading = true;
	scr_disabled_time = realtime;
//	scr_fullupdate = 0;
}

/*
================
SCR_EndLoadingPlaque
================
*/
void SCR_EndLoadingPlaque (void)
{
	scr_disabled_for_loading = false;
//	scr_fullupdate = 0;
	Con_ClearNotify ();
}

//=============================================================================

void SCR_DrawNotifyString (void)
{
	const char	*start;
	int			mask, maxwidth, l, j, x, y, len;

	start = scr_notifystring;

	mask = 0;
	y = vid.height*0.35;

	maxwidth = vid.width/8;
	
	do
	{
	// scan the width of the line
		for (l=0, len=0 ; l<maxwidth ; l++)
		{
			if (start[l] == '\n' || !start[l])
				break;
			if (start[l] != 2)		// don't include format char in length
				len++;
		}

		x = (vid.width - len*8)/2;
		for (j=0 ; j<l ; j++)
		{
		// JDH: a character preceded by 0x02 is drawn colored
			if (start[j] == 2)
			{
			#ifdef HEXEN2_SUPPORT
				if (hexen2)
					mask = 256;
				else
			#endif
				mask = 128;
				continue;
			}

			Draw_Character (x, y, start[j] | mask);
			mask = 0;
			x += 8;
		}

		y += 8;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

/*
==================
SCR_ModalMessage

Displays a text string in the center of the screen and waits for
one of the input keys (or esc) to be pressed.  Returns the index
of the pressed key in input_keys (or -1 for escape).

If NULL is passed for the text string, the screen is not dimmed
and no message is displayed.
==================
*/
int SCR_ModalMessage (const char *text, const char *input_keys)
{
	const char	*key;
	int		key_index = -1;
	double	oldtime, newtime;

	if (cls.state == ca_dedicated)
		return -1;	//return true;

	scr_notifystring = text;
	oldtime = 0;

// draw a fresh screen
//	scr_fullupdate = 0;
	scr_drawdialog = true;

	S_ClearBuffer ();		// so dma doesn't loop current sound

	key_lastpress = -1;		// JDH: so previous keypresses don't count
	while (1)
	{
	// JDH: now updates screen regularly (otherwise screen is blank if you alt-tab out)
		newtime = Sys_DoubleTime ();
		if (newtime-oldtime >= 0.1)
		{
			SCR_UpdateScreen ();
			oldtime = newtime;
		}

		key_count = -3;		// purge keyup events, then wait for a key down
		Sys_SendKeyEvents ();

		if (key_lastpress == K_ESCAPE)
			break;

		for (key = input_keys; *key; key++)
		{
			if (toupper(key_lastpress) == toupper(*key))
			{
				key_index = key-input_keys;
				goto MM_EXIT;
			}
		}
	}

MM_EXIT:
//	scr_fullupdate = 0;
	SCR_UpdateScreen ();
	scr_drawdialog = false;

	return key_index;
}


//=============================================================================

void SCR_TileClear (void)
{
	int viewheight;
	
	viewheight = vid.height - (cl_sbar.value ? Sbar_Height () : 0);

	if (r_refdef.vrect.x > 0)
	{
	//	viewheight = vid.height - sb_lines;

		// left
		Draw_TileClear (0, 0, r_refdef.vrect.x, viewheight);
		// right
		Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0,
			vid.width - r_refdef.vrect.x + r_refdef.vrect.width, viewheight);
	}
	if (r_refdef.vrect.y > 0)
	{
		// top
		Draw_TileClear (r_refdef.vrect.x, 0,
			r_refdef.vrect.x + r_refdef.vrect.width,
			r_refdef.vrect.y);
		// bottom
		Draw_TileClear (r_refdef.vrect.x,
			r_refdef.vrect.y + r_refdef.vrect.height,
			r_refdef.vrect.width,
		//	vid.height - sb_lines -
			viewheight -
			(r_refdef.vrect.height + r_refdef.vrect.y));
	}
}

#ifdef HEXEN2_SUPPORT

#define MAX_INTERMISSION_NUM 12

const char *cl_intermission_lmps[MAX_INTERMISSION_NUM] = 
{
	"meso", "egypt", "roman", "castle", "castle", "end-1", 
	"end-2", "end-3", "castle", "mpend", "mpmid", "end-3"
};

const int cl_intermission_strs[MAX_INTERMISSION_NUM] =
{
	395, 396, 397, 398, ABILITIES_STR_INDEX+NUM_CLASSES*2+1, 392, 
	393, 394, 391, 538, 545, 561
};

/*
==================
SCR_IntermissionOverlay_H2
  JDH: rewritten to use above arrays
==================
*/
void SCR_IntermissionOverlay_H2 (void)
{
	mpic_t		*pic;
	int			elapsed, size, bx, by, i;
	const char	*message;
	char		temp[80];

//	scr_copyeverything = 1;
//	scr_fullupdate = 0;

	if (cl.gametype == GAME_DEATHMATCH)
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	if ((cl.intermission < 1) || (cl.intermission > MAX_INTERMISSION_NUM))
		Sys_Error ("SCR_IntermissionOverlay: Bad episode");
	
	message = (char *) cl_intermission_lmps[cl.intermission-1];

	pic = Draw_GetCachePic (va("gfx/%s.lmp", message), true);
	M_SetScale (true);
	M_DrawPic (0, 0, pic);
//	Draw_Pic((vid.width - 320)>>1),((vid.height - 200)>>1), pic);

	if (cl.intermission >= 6 && cl.intermission <= 8)
	{
		elapsed = (cl.time - cl.completed_time) * 20;
		elapsed -= 50;
		if (elapsed < 0) elapsed = 0;
	}
	else if (cl.intermission == 12)
	{
		elapsed = introTime;
		if (introTime < 500)
			introTime += 0.25;
	}
	else
	{
		elapsed = (cl.time - cl.completed_time) * 20;
	}

	i = cl_intermission_strs[cl.intermission-1];
	if (i > pr_string_count)
		message = "";
	else
		message = &pr_global_strings[pr_string_index[i]];

	FindTextBreaks (message, 38);

	if (cl.intermission == 8)
		by = 16;
	else
		by = ((25-lines) * 8) / 2;

	for (i=0; i<lines; i++, by+=8)
	{
		size = EndC[i]-StartC[i];

		/*strncpy(temp,&message[StartC[i]],size);

		if (size > elapsed) size = elapsed;
		temp[size] = 0;

		bx = ((40-strlen(temp)) * 8) / 2;*/

		if (size > elapsed) size = elapsed;
		Q_strncpy (temp, sizeof(temp), &message[StartC[i]], size);
	  	bx = ((40-size) * 8) / 2;

		if (cl.intermission < 6 || cl.intermission > 9)
			M_IPrint (bx, by, temp);
		else
			M_PrintWhite (bx, by, temp);

		elapsed -= size;
		if (elapsed <= 0) break;
	}

	if (i == lines && elapsed >= 300 && cl.intermission >= 6 && cl.intermission <= 7)
	{
		cl.intermission++;
		cl.completed_time = cl.time;
	}

	M_SetScale (false);
//	Con_Printf("Time is %10.2f\n",elapsed);
}
#endif	// #ifdef HEXEN2_SUPPORT


void SCR_SetHUDScale (qboolean enable)
{
	static unsigned orig_vidwidth, orig_vidheight;

	if ((scr_hudscale.value == 1.0) || (scr_hudscale.value <= 0))
		return;

	assert (!!orig_vidwidth != enable);
	
	if (enable)
	{
		orig_vidwidth = vid.width;
		orig_vidheight = vid.height;
		vid.width = ceil(vid.width / scr_hudscale.value);
		vid.height = ceil(vid.height / scr_hudscale.value);
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, vid.width, vid.height, 0, -99999, 99999);
	}
	else
	{
		// restore standard projection
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, orig_vidwidth, orig_vidheight, 0, -99999, 99999);
		vid.width = orig_vidwidth;
		vid.height = orig_vidheight;
		orig_vidwidth = orig_vidheight = 0;
	}
}

float	oldsbar = 0;

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen (void)
{
	qboolean force_refdef_recalc;
	static qboolean in_update = false;

	if (in_update)
		return;

// JDH: time-delayed screenshots:
	if (scr_sshot_time && (realtime >= scr_sshot_time))
	{
		SCR_ScreenShot (scr_sshot_filename);
		scr_sshot_filename[0] = 0;
		scr_sshot_time = 0;
	}
	
	if (block_drawing)
		return;

	if (scr_disabled_for_loading)
	{
		if (realtime - scr_disabled_time > 30)		// JDH: was 60
		{
			scr_disabled_for_loading = false;
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
			{
				total_loading_size = 0;
				loading_stage = 0;
				Con_Print ("load failed.\n");
			}
		#endif
		}
		else
			return;
	}

	if (!scr_initialized || !con_initialized)
		return;				// not initialized yet

#ifdef _WIN32
	{	// don't suck up any cpu if minimized
		extern	int	Minimized;

		if (Minimized && !cls.capturedemo)
			return;
	}
#endif

	in_update = true;
	vid.numpages = (gl_triplebuffer.value ? 3 : 2);		/* NOTE: this is never used! */

//	scr_copytop = 0;
//	scr_copyeverything = 0;

/*	if (oldsbar != cl_sbar.value)
	{
		oldsbar = cl_sbar.value;
		vid.recalc_refdef = true;
	}

	// determine size of refresh window
	if (oldfov != scr_fov.value)
	{
		oldfov = scr_fov.value;
		vid.recalc_refdef = true;
	}

	if (oldscreensize != scr_viewsize.value)
	{
		oldscreensize = scr_viewsize.value;
		vid.recalc_refdef = true;
	}
*/
	force_refdef_recalc = vid.recalc_refdef;
	if (vid.recalc_refdef)
		SCR_CalcRefdef ();

	if ((v_contrast.value > 1 && !vid_hwgamma_enabled) || gl_clear.value)
		Sbar_Changed ();

// do 3D refresh drawing, and then update the screen
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);


	SCR_SetUpToDrawConsole ();

	V_RenderView (force_refdef_recalc);
	
	GL_Set2D ();

// added by joe - IMPORTANT: this _must_ be here so that
//			     palette flashes take effect in windowed mode too.
	R_PolyBlend ();

	// draw any areas not covered by the refresh
	SCR_TileClear ();

	/*if (scr_drawdialog)
	{
		Sbar_Draw ();
		Draw_DimScreen ();
		SCR_SetHUDScale (true);
		SCR_DrawNotifyString ();
		SCR_SetHUDScale (false);
//		scr_copyeverything = true;
	}
	else*/ if (scr_drawloading)
	{
	#ifdef HEXEN2_SUPPORT
		if (!hexen2)
	#endif
			SCR_DrawLoading ();
		Sbar_Draw ();
	}
#ifdef HEXEN2_SUPPORT
	else if (hexen2 && (cl.intermission >= 1) && (cl.intermission <= MAX_INTERMISSION_NUM))
	{
		if (cl.intermission == 12)
			Draw_ConsoleBackground (vid.height);		// JDH: not originally in Hexen 2

		SCR_IntermissionOverlay_H2();
		if (cl.intermission < 12)
		{
			SCR_DrawConsole();
			M_Draw();
		}
	}
#endif
	else if (cl.intermission == 1 && key_dest == key_game)
	{
		Sbar_IntermissionOverlay ();
	}
	else if (cl.intermission == 2 && key_dest == key_game)
	{
		Sbar_FinaleOverlay ();

	#ifdef HEXEN2_SUPPORT
		if (hexen2 && intro_playing)
			SCR_CheckDrawCenterString ();		// uses scr_menusize for scale
		else
	#endif
		{
			SCR_SetHUDScale (true);
			SCR_CheckDrawCenterString ();
			SCR_SetHUDScale (false);
		}
	}
	else
	{
		if (cls.state == ca_connected)
			Draw_Crosshair ();
		
		SCR_DrawRam ();
		SCR_DrawNet ();
		SCR_DrawTurtle ();
		SCR_DrawPause ();
		if (nehahra)
			SHOWLMP_drawall ();
		
		Sbar_Draw ();

		SCR_DrawStats ();

	#ifdef HEXEN2_SUPPORT
		if (hexen2 && intro_playing)
		{
			SCR_CheckDrawCenterString ();
			SCR_SetHUDScale (true);
		}
		else
	#endif
		{
			SCR_SetHUDScale (true);
			SCR_CheckDrawCenterString ();
		}

	// JDH: moved these after Sbar_Draw so they are visible even on shrunken screen:
		SCR_DrawFPS ();

		if (cls.state == ca_connected)
		{
			SCR_DrawLocalTime ();
			SCR_DrawOrigin ();
			SCR_DrawSpeed ();
			SCR_DrawDemoOverlay ();
		}

		SCR_SetHUDScale (false);

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			SCR_Plaque_Draw (plaquemessage, 0);
			if (info_up)
			{
				UpdateInfoMessage ();
				SCR_Info_Plaque_Draw (infomessage);
			}
		}
	#endif

		if (scr_drawdialog && scr_notifystring)
		{
			if (key_dest == key_menu)
				M_Draw ();
			else
				Draw_ConbackSolid ();
		}
		else
		{
			SCR_DrawConsole ();
			M_Draw ();
		}
	}


	if (scr_drawdialog && scr_notifystring)
	{
		Draw_DimScreen ();
		SCR_SetHUDScale (true);
		SCR_DrawNotifyString ();
		SCR_SetHUDScale (false);
//		scr_copyeverything = true;
	}

	R_BrightenScreen ();

	V_UpdatePalette ();

#ifdef HEXEN2_SUPPORT
	if (hexen2 && (loading_stage || scr_drawloading))
		SCR_DrawLoading ();		// AFTER brightening, otherwise it'll look different
#endif							//  when it's called outside of SCR_UpdateScreen

	Movie_UpdateScreen ();

	GL_EndRendering ();
	in_update = false;
}

#endif		//#ifndef RQM_SV_ONLY
