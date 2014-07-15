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
// cl.input.c  -- builds an intended movement command to send to the server

// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include "quakedef.h"

#ifndef RQM_SV_ONLY

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition

===============================================================================
*/


kbutton_t	in_mlook, in_klook;
kbutton_t	in_left, in_right, in_forward, in_back;
kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t	in_strafe, in_speed, in_use, in_jump, in_attack;
kbutton_t	in_up, in_down;

int		in_impulse;


#ifdef HEXEN2_SUPPORT
  extern qboolean info_up;

  kbutton_t  in_crouch, in_infoplaque;
#endif

void KeyDown (kbutton_t *b)
{
	int	k;
	const char	*c;

	c = Cmd_Argv(1);
	if (c[0])
		k = atoi(c);
	else
		k = -1;		// typed manually at the console for continuous down

	if (k == b->down[0] || k == b->down[1])
		return;		// repeating key

	if (!b->down[0])
	{
		b->down[0] = k;
	}
	else if (!b->down[1])
	{
		b->down[1] = k;
	}
	else
	{
		Con_Print ("Three keys down for a button!\n");
		return;
	}

	if (b->state & 1)
		return;		// still down
	b->state |= 1 + 2;	// down + impulse down
}

void KeyUp (kbutton_t *b)
{
	int		k;
	const char	*c;

	c = Cmd_Argv(1);
	if (c[0])
		k = atoi(c);
	else
	{ // typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->state = 4;	// impulse up
		return;
	}

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return;		// key up without coresponding down (menu pass through)
	if (b->down[0] || b->down[1])
		return;		// some other key is still holding it down

	if (!(b->state & 1))
		return;		// still up (this should not happen)
	b->state &= ~1;		// now up
	b->state |= 4; 		// impulse up
}

void IN_KLookDown (cmd_source_t src)    {KeyDown(&in_klook);}
void IN_KLookUp (cmd_source_t src)      {KeyUp(&in_klook);}
void IN_MLookDown (cmd_source_t src)    {KeyDown(&in_mlook);}
void IN_MLookUp (cmd_source_t src)
{
	KeyUp(&in_mlook);
	if (!(in_mlook.state & 1) && lookspring.value)
		V_StartPitchDrift ();
}
void IN_UpDown(cmd_source_t src)        {KeyDown(&in_up);}
void IN_UpUp(cmd_source_t src)          {KeyUp(&in_up);}
void IN_DownDown(cmd_source_t src)      {KeyDown(&in_down);}
void IN_DownUp(cmd_source_t src)        {KeyUp(&in_down);}
void IN_LeftDown(cmd_source_t src)      {KeyDown(&in_left);}
void IN_LeftUp(cmd_source_t src)        {KeyUp(&in_left);}
void IN_RightDown(cmd_source_t src)     {KeyDown(&in_right);}
void IN_RightUp(cmd_source_t src)       {KeyUp(&in_right);}
void IN_ForwardDown(cmd_source_t src)   {KeyDown(&in_forward);}
void IN_ForwardUp(cmd_source_t src)     {KeyUp(&in_forward);}
void IN_BackDown(cmd_source_t src)      {KeyDown(&in_back);}
void IN_BackUp(cmd_source_t src)        {KeyUp(&in_back);}
void IN_LookupDown(cmd_source_t src)    {KeyDown(&in_lookup);}
void IN_LookupUp(cmd_source_t src)      {KeyUp(&in_lookup);}
void IN_LookdownDown(cmd_source_t src)  {KeyDown(&in_lookdown);}
void IN_LookdownUp(cmd_source_t src)    {KeyUp(&in_lookdown);}
void IN_MoveleftDown(cmd_source_t src)  {KeyDown(&in_moveleft);}
void IN_MoveleftUp(cmd_source_t src)    {KeyUp(&in_moveleft);}
void IN_MoverightDown(cmd_source_t src) {KeyDown(&in_moveright);}
void IN_MoverightUp(cmd_source_t src)   {KeyUp(&in_moveright);}

void IN_SpeedDown(cmd_source_t src)     {KeyDown(&in_speed);}
void IN_SpeedUp(cmd_source_t src)       {KeyUp(&in_speed);}
void IN_StrafeDown(cmd_source_t src)    {KeyDown(&in_strafe);}
void IN_StrafeUp(cmd_source_t src)      {KeyUp(&in_strafe);}

void IN_AttackDown(cmd_source_t src)    {KeyDown(&in_attack);}
void IN_AttackUp(cmd_source_t src)      {KeyUp(&in_attack);}

void IN_UseDown (cmd_source_t src)      {KeyDown(&in_use);}
void IN_UseUp (cmd_source_t src)        {KeyUp(&in_use);}
void IN_JumpDown (cmd_source_t src)     {KeyDown(&in_jump);}
void IN_JumpUp (cmd_source_t src)       {KeyUp(&in_jump);}

void IN_Impulse (cmd_source_t src)      {in_impulse = Q_atoi(Cmd_Argv(1));}

/*
===============
CL_KeyState

Returns 0.25 if a key was pressed and released during the frame,
0.5 if it was pressed and held
0 if held then released, and
1.0 if held for the entire time
===============
*/
float CL_KeyState (kbutton_t *key)
{
	float		val;
	qboolean	impulsedown, impulseup, down;

	impulsedown = key->state & 2;
	impulseup = key->state & 4;
	down = key->state & 1;
	val = 0;

	if (impulsedown && !impulseup)
		val = down ? 0.5 : 0;
	if (impulseup && !impulsedown)
		val = 0;
	if (!impulsedown && !impulseup)
		val = down ? 1.0 : 0;
	if (impulsedown && impulseup)
		val = down ? 0.75 : 0.25;

	key->state &= 1;		// clear impulses

	return val;
}

//==========================================================================

cvar_t	cl_upspeed = {"cl_upspeed", "200", CVAR_FLAG_ARCHIVE};
cvar_t	cl_forwardspeed = {"cl_forwardspeed", "200", CVAR_FLAG_ARCHIVE};
cvar_t	cl_backspeed = {"cl_backspeed", "200", CVAR_FLAG_ARCHIVE};
cvar_t	cl_sidespeed = {"cl_sidespeed", "350", CVAR_FLAG_ARCHIVE};

cvar_t	cl_movespeedkey = {"cl_movespeedkey", "2.0", CVAR_FLAG_ARCHIVE};

cvar_t	cl_yawspeed = {"cl_yawspeed", "140", CVAR_FLAG_ARCHIVE};
cvar_t	cl_pitchspeed = {"cl_pitchspeed", "150", CVAR_FLAG_ARCHIVE};

cvar_t	cl_anglespeedkey = {"cl_anglespeedkey", "1.5", CVAR_FLAG_ARCHIVE};

// joe: synthetic lag, from ProQuake
cvar_t	pq_lag = {"pq_lag", "0", CVAR_FLAG_ARCHIVE};

/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
void CL_AdjustAngles (void)
{
	float	speed, up, down;

	speed = (in_speed.state & 1) ? host_frametime * cl_anglespeedkey.value : host_frametime;

	if (!(in_strafe.state & 1))
	{
		cl.viewangles[YAW] -= speed * cl_yawspeed.value * CL_KeyState (&in_right);
		cl.viewangles[YAW] += speed * cl_yawspeed.value * CL_KeyState (&in_left);
		cl.viewangles[YAW] = anglemod(cl.viewangles[YAW]);
	}
	if (in_klook.state & 1)
	{
		V_StopPitchDrift ();
		cl.viewangles[PITCH] -= speed * cl_pitchspeed.value * CL_KeyState (&in_forward);
		cl.viewangles[PITCH] += speed * cl_pitchspeed.value * CL_KeyState (&in_back);
	}

#ifdef HEXEN2_SUPPORT
	// FIXME: This is a cheap way of doing this, it belongs in V_CalcViewRoll
	// but I don't see where I can get the yaw velocity, I have to get on to other things so here it is
	if (hexen2)
	{
		float movetype = cl.v.movetype;

		if (CL_KeyState (&in_left) && (movetype == MOVETYPE_FLY))
			cl.idealroll = -10;
		else if (CL_KeyState (&in_right) && (movetype == MOVETYPE_FLY))
			cl.idealroll = 10;
		else
			cl.idealroll = 0;
	}
#endif

	up = CL_KeyState (&in_lookup);
	down = CL_KeyState (&in_lookdown);

	cl.viewangles[PITCH] -= speed * cl_pitchspeed.value * up;
	cl.viewangles[PITCH] += speed * cl_pitchspeed.value * down;

	if (up || down)
		V_StopPitchDrift ();

	cl.viewangles[PITCH] = bound (-70, cl.viewangles[PITCH], 80);

	cl.viewangles[ROLL] = bound (-50, cl.viewangles[ROLL], 50);
}

/*
================
CL_BaseMove

Send the intended movement message to the server
================
*/
void CL_BaseMove (usercmd_t *cmd)
{
	if (cls.signon != SIGNONS)
		return;

	memset (cmd, 0, sizeof(*cmd));

#ifdef HEXEN2_SUPPORT
	if (hexen2 && cl.v.cameramode)
	{
		return;		// Stuck in a different camera so don't move
	}
#endif

	CL_AdjustAngles ();

	if (in_strafe.state & 1)
	{
		cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_right);
		cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_left);
	}

	cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_moveright);
	cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_moveleft);

	cmd->upmove += cl_upspeed.value * CL_KeyState (&in_up);
	cmd->upmove -= cl_upspeed.value * CL_KeyState (&in_down);

	if (!(in_klook.state & 1))
	{
		cmd->forwardmove += cl_forwardspeed.value * CL_KeyState (&in_forward);
		cmd->forwardmove -= cl_backspeed.value * CL_KeyState (&in_back);
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		float hasted = cl.v.hasted;

		if ((/*cl_forwardspeed.value > 200 ||*/ in_speed.state & 1) && (hasted <= 1))
		{
			cmd->forwardmove *= cl_movespeedkey.value;
			cmd->sidemove *= cl_movespeedkey.value;
			cmd->upmove *= cl_movespeedkey.value;
		}

		// Hasted player?
		if (hasted)
		{
			cmd->forwardmove *= hasted;
			cmd->sidemove *= hasted;
			cmd->upmove *= hasted;
		}

		cmd->lightlevel = cl.light_level;
	}
	else
#endif
// adjust for speed key
	if (in_speed.state & 1)
	{
		cmd->forwardmove *= cl_movespeedkey.value;
		cmd->sidemove *= cl_movespeedkey.value;
		cmd->upmove *= cl_movespeedkey.value;
	}
}

// joe: support for synthetic lag, from ProQuake
sizebuf_t	lag_buff[32];
byte		lag_data[32][128];
unsigned int	lag_head, lag_tail;
double		lag_sendtime[32];

/*
==============
CL_SendLagMove
==============
*/
void CL_SendLagMove (void)
{
	if (cls.state != ca_connected || cls.signon != SIGNONS)
		return;

	while ((lag_tail < lag_head) && (lag_sendtime[lag_tail&31] <= realtime))
	{
		lag_tail++;
		if (++cl.movemessages <= 2)
		{
			lag_head = lag_tail = 0;  // JPG - hack: if cl.movemessages has been reset, we should reset these too
			continue;	// return -> continue
		}

		if (NET_SendUnreliableMessage (cls.netcon, &lag_buff[(lag_tail-1)&31]) == -1)
		{
			Con_Print ("CL_SendMove: lost server connection\n");
			CL_Disconnect (false);
		}
	}
}

/*
==============
CL_SendMove
==============
*/
void CL_SendMove (const usercmd_t *cmd)
{
	int		i, bits;
	sizebuf_t	*buf;

	buf = &lag_buff[lag_head&31];
	buf->maxsize = 128;
	buf->cursize = 0;
	buf->data = lag_data[lag_head&31];
	lag_sendtime[lag_head++&31] = realtime + (pq_lag.value / 1000.0);

	cl.cmd = *cmd;

// send the movement message
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		MSG_WriteByte (buf, clc_frame);
		MSG_WriteByte (buf, cl.reference_frame);
		MSG_WriteByte (buf, cl.current_sequence);
	}
#endif

	MSG_WriteByte (buf, clc_move);

	MSG_WriteFloat (buf, cl.mtime);	// so server can get ping times

// precise aim from [sons]Quake
	if (!cls.demoplayback && ((cls.netcon->mod == MOD_JOEQUAKE) || (cl.protocol == PROTOCOL_VERSION_FITZ)))
	{
		for (i=0 ; i<3 ; i++)
			MSG_WritePreciseAngle (buf, cl.viewangles[i]);
	}
	else
	{
		for (i=0 ; i<3 ; i++)
			MSG_WriteAngle (buf, cl.viewangles[i]);
	}

	MSG_WriteShort (buf, cmd->forwardmove);
	MSG_WriteShort (buf, cmd->sidemove);
	MSG_WriteShort (buf, cmd->upmove);

// send button bits
	bits = 0;

	if (in_attack.state & 3)
		bits |= 1;
	in_attack.state &= ~2;

	if (in_jump.state & 3)
		bits |= 2;
	in_jump.state &= ~2;

#ifdef HEXEN2_SUPPORT
	if ((hexen2) && (in_crouch.state & 1))
		bits |= 4;
#endif

	MSG_WriteByte (buf, bits);

	MSG_WriteByte (buf, in_impulse);
	in_impulse = 0;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		MSG_WriteByte (buf, cmd->lightlevel);
#endif

// deliver the message
	if (cls.demoplayback)
		return;

	CL_SendLagMove ();
}

#ifdef HEXEN2_SUPPORT
void IN_CrouchDown (cmd_source_t src)
{
	int state;

	if (key_dest == key_game)
	{
		state = in_crouch.state;
		KeyDown (&in_crouch);

//		if (!(state & 1) && (in_crouch.state & 1))
//			in_impulse = 22;
	}
}

void IN_CrouchUp (cmd_source_t src)
{
//	int state;

	if (key_dest == key_game)
	{
//		state = in_crouch.state;

		KeyUp (&in_crouch);
//		if ((state & 1) && !(in_crouch.state & 1))
//			in_impulse = 22;
	}
}

void IN_infoPlaqueUp (cmd_source_t src)
{
	if (key_dest == key_game)
	{
		//They want to lower the plaque
		/*if (!infomessage)
		{
			infomessage=Z_Malloc(1028);//"Objectives:@";
		}*/

		info_up = 0;
		KeyUp (&in_infoplaque);
	}
}

void IN_infoPlaqueDown(cmd_source_t src)
{
	if (key_dest == key_game)
	{
		//They want to see the plaque
		/*if (infomessage[0] == '\0')
			strcpy(infomessage, "Objectives:");*/

		info_up = 1;
		KeyDown (&in_infoplaque);
	}
}
#endif	// #ifdef HEXEN2_SUPPORT

/*
============
CL_InitInput
============
*/
void CL_InitInput (void)
{
	Cmd_AddCommand ("+moveup", IN_UpDown, 0);
	Cmd_AddCommand ("-moveup", IN_UpUp, 0);
	Cmd_AddCommand ("+movedown", IN_DownDown, 0);
	Cmd_AddCommand ("-movedown", IN_DownUp, 0);
	Cmd_AddCommand ("+left", IN_LeftDown, 0);
	Cmd_AddCommand ("-left", IN_LeftUp, 0);
	Cmd_AddCommand ("+right", IN_RightDown, 0);
	Cmd_AddCommand ("-right", IN_RightUp, 0);
	Cmd_AddCommand ("+forward", IN_ForwardDown, 0);
	Cmd_AddCommand ("-forward", IN_ForwardUp, 0);
	Cmd_AddCommand ("+back", IN_BackDown, 0);
	Cmd_AddCommand ("-back", IN_BackUp, 0);
	Cmd_AddCommand ("+lookup", IN_LookupDown, 0);
	Cmd_AddCommand ("-lookup", IN_LookupUp, 0);
	Cmd_AddCommand ("+lookdown", IN_LookdownDown, 0);
	Cmd_AddCommand ("-lookdown", IN_LookdownUp, 0);
	Cmd_AddCommand ("+strafe", IN_StrafeDown, 0);
	Cmd_AddCommand ("-strafe", IN_StrafeUp, 0);
	Cmd_AddCommand ("+moveleft", IN_MoveleftDown, 0);
	Cmd_AddCommand ("-moveleft", IN_MoveleftUp, 0);
	Cmd_AddCommand ("+moveright", IN_MoverightDown, 0);
	Cmd_AddCommand ("-moveright", IN_MoverightUp, 0);
	Cmd_AddCommand ("+speed", IN_SpeedDown, 0);
	Cmd_AddCommand ("-speed", IN_SpeedUp, 0);
	Cmd_AddCommand ("+attack", IN_AttackDown, 0);
	Cmd_AddCommand ("-attack", IN_AttackUp, 0);
	Cmd_AddCommand ("+use", IN_UseDown, 0);
	Cmd_AddCommand ("-use", IN_UseUp, 0);
	Cmd_AddCommand ("+jump", IN_JumpDown, 0);
	Cmd_AddCommand ("-jump", IN_JumpUp, 0);
	Cmd_AddCommand ("impulse", IN_Impulse, 0);
	Cmd_AddCommand ("+klook", IN_KLookDown, 0);
	Cmd_AddCommand ("-klook", IN_KLookUp, 0);
	Cmd_AddCommand ("+mlook", IN_MLookDown, 0);
	Cmd_AddCommand ("-mlook", IN_MLookUp, 0);

	Cvar_Register (&pq_lag);	// joe: synthetic lag, from ProQuake
}

#endif		//#ifndef RQM_SV_ONLY
