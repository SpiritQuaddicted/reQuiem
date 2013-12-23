
#include "quakedef.h"

#ifndef RQM_SV_ONLY

#define	CM_ANGLE1 	(1<<0)
#define	CM_ANGLE3 	(1<<1)
#define	CM_FORWARD	(1<<2)
#define	CM_SIDE		(1<<3)
#define	CM_UP		(1<<4)
#define	CM_BUTTONS	(1<<5)
#define	CM_IMPULSE	(1<<6)
#define	CM_ANGLE2 	(1<<7)

#define MAX_MSGLEN_QW  1450

typedef struct
{
	int		modelindex;
	vec3_t	origin;
	vec3_t	angles;
} projectile_t;

#define	MAX_PROJECTILES	32
projectile_t	cl_projectiles[MAX_PROJECTILES];
int				cl_num_projectiles;


void CL_ClearProjectiles (void)
{
	cl_num_projectiles = 0;
}

/*
=====================
CL_ParseProjectiles

Nails are passed as efficient temporary entities
=====================
*/
void CL_ParseProjectiles (void)
{
	int		i, c, j;
	byte	bits[6];
	projectile_t	*pr;

	c = MSG_ReadByte ();
	for (i=0 ; i<c ; i++)
	{
		for (j=0 ; j<6 ; j++)
			bits[j] = MSG_ReadByte ();

		if (cl_num_projectiles == MAX_PROJECTILES)
			continue;

		pr = &cl_projectiles[cl_num_projectiles];
		cl_num_projectiles++;

		pr->modelindex = cl_modelindex[mi_spike];
		pr->origin[0] = ( ( bits[0] + ((bits[1]&15)<<8) ) <<1) - 4096;
		pr->origin[1] = ( ( (bits[1]>>4) + (bits[2]<<4) ) <<1) - 4096;
		pr->origin[2] = ( ( bits[3] + ((bits[4]&15)<<8) ) <<1) - 4096;
		pr->angles[0] = 360*(bits[4]>>4)/16;
		pr->angles[1] = 360*bits[5]/256;
		pr->angles[2] = 0;		// JDH: added this line
	}
}

/*
=============
CL_LinkProjectiles
=============
*/
void CL_LinkProjectiles (void)
{
	int		i;
	projectile_t	*pr;
	entity_t		*ent;

	for (i=0, pr=cl_projectiles ; i<cl_num_projectiles ; i++, pr++)
	{
		// grab an entity to fill in
		if (cl_numvisedicts == MAX_VISEDICTS)
			break;		// object list is full
		
		if (pr->modelindex < 1)
			continue;

		ent = &cl_entities[cl.num_entities + i];
		cl_visedicts[cl_numvisedicts] = ent;
		cl_numvisedicts++;

		ent->model = cl.model_precache[pr->modelindex];
		ent->skinnum = 0;
		ent->frame = 0;
		ent->colormap = vid.colormap;
		
		ent->transparency = 1.0;
		ent->fullbright = 0;
		ent->translate_start_time = 0;
		ent->rotate_start_time = 0;

		//JDH: ****** may have to initialize other fields of ent ******

		VectorCopy (pr->origin, ent->origin);
		VectorCopy (pr->angles, ent->angles);
	}
}

/*
=====================
CL_ReadDeltaUsercmd
=====================
*/
void CL_ReadDeltaUsercmd (entity_t *ent)
{
	int bits;

	bits = MSG_ReadByte ();
		
	VectorCopy (ent->msg_angles, ent->msg_angles_prev);

// read current angles
	if (bits & CM_ANGLE1)
		ent->msg_angles[0] = MSG_ReadPreciseAngle ();
	if (bits & CM_ANGLE2)
		ent->msg_angles[1] = MSG_ReadPreciseAngle ();
	if (bits & CM_ANGLE3)
		ent->msg_angles[2] = MSG_ReadPreciseAngle ();
		
// read movement
	if (bits & CM_FORWARD)
		MSG_ReadShort ();
	if (bits & CM_SIDE)
		MSG_ReadShort ();
	if (bits & CM_UP)
		MSG_ReadShort ();
	
// read buttons
	if (bits & CM_BUTTONS)
		MSG_ReadByte ();

	if (bits & CM_IMPULSE)
		MSG_ReadByte ();

// read time to run command
	MSG_ReadByte ();
}

// playerinfo flags from server

#define	PF_MSEC			(1<<0)
#define	PF_COMMAND		(1<<1)
#define	PF_VELOCITY1	(1<<2)
#define	PF_VELOCITY2	(1<<3)
#define	PF_VELOCITY3	(1<<4)
#define	PF_MODEL		(1<<5)
#define	PF_SKINNUM		(1<<6)
#define	PF_EFFECTS		(1<<7)
#define	PF_WEAPONFRAME	(1<<8)		// only sent for view player
#define	PF_DEAD			(1<<9)		// don't block movement any more
#define	PF_GIB			(1<<10)		// offset the view height differently
#define	PF_NOGRAV		(1<<11)		// don't apply gravity for prediction

/*
==============
CL_ParsePlayerinfo (for QW demos)
==============
*/
void CL_ParsePlayerinfo (void)
{
	int			clientnum, flags, i;
	entity_t	*ent;
	float		fval;

	clientnum = MSG_ReadByte ();
	if (clientnum > MAX_SCOREBOARD)
		Host_Error ("CL_ParsePlayerinfo: bad clientnum");

//	ent = &cl_entities[cl.viewentity + clientnum];
	ent = &cl_entities[1 + clientnum];
	ent->msgtime = cl.mtime;
	
	flags = MSG_ReadShort ();

// shift the known values for interpolation
	VectorCopy (ent->msg_origin, ent->msg_origin_prev);
	
	ent->msg_origin[0] = MSG_ReadCoord();
	ent->msg_origin[1] = MSG_ReadCoord();
	ent->msg_origin[2] = MSG_ReadCoord();

#ifdef _DEBUG
	for (i=0; i<3; i++)
	{
		if (ent->msg_origin[i] != ent->msg_origin_prev[i])
			fval = 3463;
	}
#endif
	
	ent->frame = MSG_ReadByte ();

	if (flags & PF_MSEC)
		MSG_ReadByte ();

	if (flags & PF_COMMAND)
		CL_ReadDeltaUsercmd (ent);

//	if (clientnum == 0)
	if (clientnum == cl.viewentity-1)
		VectorCopy (cl.mvelocity, cl.mvelocity_prev);
	for (i=0 ; i<3 ; i++)
	{
		fval = (flags & (PF_VELOCITY1 << i)) ? MSG_ReadShort() : 0;
//		if (clientnum == 0)
		if (clientnum == cl.viewentity-1)
			cl.mvelocity[i] = fval;
	}

	if (flags & PF_MODEL)
		ent->modelindex = MSG_ReadByte ();
	else
		ent->modelindex = cl_modelindex[mi_player];

	ent->model = cl.model_precache[ent->modelindex];
	ent->colormap = cl.scores[clientnum].translations;
	
	if (flags & PF_SKINNUM)
		ent->skinnum = MSG_ReadByte ();
	else
		ent->skinnum = 0;

	if (flags & PF_EFFECTS)
		ent->effects = MSG_ReadByte ();
	else
		ent->effects = 0;

#ifdef _DEBUG
	if (flags & PF_WEAPONFRAME)
		i = 23423;
#endif

	i = (flags & PF_WEAPONFRAME) ? MSG_ReadByte () : 0;
//	if (clientnum == 0)
	if (clientnum == cl.viewentity-1)
		cl.stats[STAT_WEAPONFRAME] = i;

/*	VectorCopy (ent->msg_origin, ent->msg_origin_prev);
	VectorCopy (ent->msg_origin, ent->origin);
	VectorCopy (ent->msg_angles, ent->msg_angles_prev);
	VectorCopy (ent->msg_angles, ent->angles);
	ent->forcelink = true;
*/
}

/*
===============
CL_GetUserinfoValue
(Info_ValueForKey in QW src)

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
char *CL_GetUserinfoValue (const char *s, const char *key)
{
	char	pkey[512];
	static char	value[512];
	char	*o;
	
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
			return value;

		if (!*s)
			return "";
		s++;
	}
}

/*
==============
CL_ProcessUserInfo (for QW demos)
==============
*/
void CL_ProcessUserInfo (int slot, scoreboard_t *player, const char *userinfo)
{
	Q_strcpy (player->name, CL_GetUserinfoValue (userinfo, "name"), sizeof(player->name));
	player->colors = (atoi(CL_GetUserinfoValue (userinfo, "topcolor")) & 0x0F) << 4;
	player->colors |= (atoi(CL_GetUserinfoValue (userinfo, "bottomcolor")) & 0x0F);
	
	Sbar_Changed ();
	CL_NewTranslation (slot);
}

/*
==============
CL_UpdateUserinfo (for QW demos)
==============
*/
void CL_UpdateUserinfo (void)
{
	int				slot;
	scoreboard_t	*player;
	char			*userinfo;

	slot = MSG_ReadByte ();
	if (slot >= cl.maxclients)
		Host_Error ("CL_UpdateUserinfo: svc_updateuserinfo > MAX_SCOREBOARD");

	player = &cl.scores[slot];
	MSG_ReadLong ();		// user ID
	userinfo = MSG_ReadString();

	CL_ProcessUserInfo (slot, player, userinfo);
}

/*
==============
CL_SetInfo
==============
*/
void CL_SetInfo (void)
{
	int				slot;
	scoreboard_t	*player;
	char			key[MAX_MSGLEN_QW];
	char			value[MAX_MSGLEN_QW];

	slot = MSG_ReadByte ();
	if (slot >= MAX_SCOREBOARD)
		Host_EndGame ("CL_ParseServerMessage: svc_setinfo > MAX_SCOREBOARD");

	player = &cl.scores[slot];

	Q_strcpy (key, MSG_ReadString(), sizeof(key));
	key[sizeof(key) - 1] = 0;
	Q_strcpy (value, MSG_ReadString(), sizeof(value));
	key[sizeof(value) - 1] = 0;

	Con_DPrintf("CL_SetInfo %s: %s=%s\n", player->name, key, value);

	/************ FINISH ME!!! *************/

//	Info_SetValueForKey (player->userinfo, key, value, MAX_INFO_STRING);

//	CL_ProcessUserInfo (slot, player);
}

// the first 16 bits of a packetentities update holds 9 bits
// of entity number and 7 bits of flags
#define	UQW_ORIGIN1	 (1<<9)
#define	UQW_ORIGIN2	 (1<<10)
#define	UQW_ORIGIN3	 (1<<11)
#define	UQW_ANGLE2	 (1<<12)
#define	UQW_FRAME	 (1<<13)
#define	UQW_REMOVE	 (1<<14)		// REMOVE this entity, don't add it
#define	UQW_MOREBITS (1<<15)

// if MOREBITS is set, these additional flags are read in next
#define	UQW_ANGLE1	 (1<<0)
#define	UQW_ANGLE3	 (1<<1)
#define	UQW_MODEL	 (1<<2)
#define	UQW_COLORMAP (1<<3)
#define	UQW_SKIN	 (1<<4)
#define	UQW_EFFECTS	 (1<<5)
#define	UQW_SOLID	 (1<<6)		// the entity should be solid for prediction

/*
==================
CL_ParseDelta
==================
*/
void CL_ParseDelta (entity_t *ent, int bits, entity_state_t *es)
{
	int			i;

	if (bits & UQW_MOREBITS)
	{	// read in the low order bits
		i = MSG_ReadByte ();
		bits |= i;
	}
	
	if (bits & UQW_MODEL)
		ent->modelindex = MSG_ReadByte ();
	else
		ent->modelindex = es->modelindex;

	if (ent->modelindex)
		ent->model = cl.model_precache[ent->modelindex];
	else
		ent->model = NULL;

	if (bits & UQW_FRAME)
		ent->frame = MSG_ReadByte ();
	else
		ent->frame = es->frame;

	if (bits & UQW_COLORMAP)
		i = MSG_ReadByte();
	else
		i = es->colormap;

	if (i && (i <= cl.maxclients) && ent->model && (ent->model->modhint == MOD_PLAYER))
		ent->colormap = cl.scores[i-1].translations;
	else
		ent->colormap = vid.colormap;

	if (bits & UQW_SKIN)
		ent->skinnum = MSG_ReadByte();
	else
		ent->skinnum = es->skin;

	if (bits & UQW_EFFECTS)
		ent->effects = MSG_ReadByte();
	else
		ent->effects = es->effects;

// shift the known values for interpolation
	VectorCopy (ent->msg_origin, ent->msg_origin_prev);
	VectorCopy (ent->msg_angles, ent->msg_angles_prev);

	if (bits & UQW_ORIGIN1)
		ent->msg_origin[0] = MSG_ReadCoord ();
	else
		ent->msg_origin[0] = es->origin[0];
		
	if (bits & UQW_ANGLE1)
		ent->msg_angles[0] = MSG_ReadAngle();
	else
		ent->msg_angles[0] = es->angles[0];

	if (bits & UQW_ORIGIN2)
		ent->msg_origin[1] = MSG_ReadCoord ();
	else
		ent->msg_origin[1] = es->origin[1];
		
	if (bits & UQW_ANGLE2)
		ent->msg_angles[1] = MSG_ReadAngle();
	else
		ent->msg_angles[1] = es->angles[1];

	if (bits & UQW_ORIGIN3)
		ent->msg_origin[2] = MSG_ReadCoord ();
	else
		ent->msg_origin[2] = es->origin[2];
		
	if (bits & UQW_ANGLE3)
		ent->msg_angles[2] = MSG_ReadAngle();
	else
		ent->msg_angles[2] = es->angles[2];

	if (bits & UQW_SOLID)
	{
		// QW doesn't use this...
	}


			/****** TEST ONLY!! REMOVE!! ******/
#ifdef _DEBUG
//	VectorCopy (ent->msg_origin, ent->msg_origin_prev);
//	VectorCopy (ent->msg_angles, ent->msg_angles_prev);
#endif
			/****** TEST ONLY!! REMOVE!! ******/
}

/*
==================
CL_ParseDeltaDummy (temporary way to skip over delta packets)
==================
*/
void CL_ParseDeltaDummy (int bits)
{
	int			i;

	if (bits & UQW_MOREBITS)
	{	// read in the low order bits
		i = MSG_ReadByte ();
		bits |= i;
	}
	
	if (bits & UQW_MODEL)
		MSG_ReadByte ();

	if (bits & UQW_FRAME)
		MSG_ReadByte ();

	if (bits & UQW_COLORMAP)
		MSG_ReadByte();

	if (bits & UQW_SKIN)
		MSG_ReadByte();

	if (bits & UQW_EFFECTS)
		MSG_ReadByte();

	if (bits & UQW_ORIGIN1)
		MSG_ReadCoord ();
		
	if (bits & UQW_ANGLE1)
		MSG_ReadAngle();

	if (bits & UQW_ORIGIN2)
		MSG_ReadCoord ();
		
	if (bits & UQW_ANGLE2)
		MSG_ReadAngle();

	if (bits & UQW_ORIGIN3)
		MSG_ReadCoord ();
		
	if (bits & UQW_ANGLE3)
		MSG_ReadAngle();
}

/*
==============
CL_ParsePacketEntities (for QW demos)		**** FIXME: delta packet support not implemented yet ****
==============
*/

qboolean CL_ParsePacketEntities (qboolean delta)
{
	int			word;
	entity_t	*ent;
	static		qboolean delta_warned = false;

	if (delta)
	{
		MSG_ReadByte();
		Con_Print ("\x02""WARNING: ");
		Con_Print ("demo contains delta packets, which are not supported yet!\n");
		delta_warned = true;
	}
	
	if (cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		cls.signon = SIGNONS;
		delta_warned = false;
	}

	while (1)
	{
		word = (unsigned short) MSG_ReadShort ();
		if (!word)
			break;

		if (word & UQW_REMOVE)
			Con_Print ("WARNING: svc_packetentities: U_REMOVE not supported!\n");		// FIXME!
		
		ent = CL_EntityNum (word & 0x01FF);
		if (!ent)
			return false;
		ent->msgtime = cl.mtime;
		
		if (delta)
			CL_ParseDeltaDummy (word & 0xFE00);
		else
			CL_ParseDelta (ent, word & 0xFE00, &ent->baseline);
	}

	return true;
}

#endif		//#ifndef RQM_SV_ONLY
