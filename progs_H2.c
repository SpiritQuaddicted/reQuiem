
#include "quakedef.h"

#ifdef HEXEN2_SUPPORT

#define WF_NORMAL_ADVANCE 0
#define WF_CYCLE_STARTED  1
#define WF_CYCLE_WRAPPED  2
#define WF_LAST_FRAME     3

#define	RETURN_EDICT(e) (((int *)pr_globals)[OFS_RETURN] = EDICT_TO_PROG(e))

extern int			*pr_string_index;
extern char			*pr_global_strings;
extern int			pr_string_count;
//extern unsigned int	info_mask, info_mask2;		JDH: now in server_t (sv)

#ifndef RQM_SV_ONLY
extern char			*plaquemessage;
#endif

extern qboolean		allow_postcache;

extern edict_t		*ED_Alloc_Temp (void);
extern sizebuf_t	*WriteDest (void);

/*
==============
PF_lightstylestatic
==============
*/
void PF_lightstylestatic (void)
{
	int i, value, styleNumber;
	const char *styleString;
	client_t *client;
	static const char *styleDefs[] =
	{
		"a", "b", "c", "d", "e", "f", "g",
		"h", "i", "j", "k", "l", "m", "n",
		"o", "p", "q", "r", "s", "t", "u",
		"v", "w", "x", "y", "z"
	};

	styleNumber = G_FLOAT(OFS_PARM0);
	value = G_FLOAT(OFS_PARM1);

	value = bound(0, value, 'z'-'a');
	styleString = styleDefs[value];

	// Change the string in sv
	sv.lightstyles[styleNumber] = (char *)styleString;

	if (sv.state != ss_active)
	{
		return;
	}

	// Send message to all clients on this server
	for (i = 0, client = svs.clients; i < svs.maxclients; i++, client++)
	{
		if (client->active || client->spawned)
		{
			MSG_WriteCmd (&client->message, svc_lightstyle);
			MSG_WriteChar (&client->message, styleNumber);
			MSG_WriteString (&client->message, styleString);
		}
	}
}

/*
==============
PF_tracearea
==============
*/
void PF_tracearea (void)
{
	float	*v1, *v2, *mins, *maxs;
	trace_t	trace;
	int		nomonsters;
	edict_t	*ent;
	float	save_hull;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	mins = G_VECTOR(OFS_PARM2);
	maxs = G_VECTOR(OFS_PARM3);
	nomonsters = G_FLOAT(OFS_PARM4);
	ent = G_EDICT(OFS_PARM5);

	save_hull = ent->v.hull;
	ent->v.hull = 0;
	trace = SV_Move (v1, mins, maxs, v2, nomonsters, ent);
	ent->v.hull = save_hull;

	*pr_global_ptrs.trace_allsolid = trace.allsolid;
	*pr_global_ptrs.trace_startsolid = trace.startsolid;
	*pr_global_ptrs.trace_fraction = trace.fraction;
	*pr_global_ptrs.trace_inwater = trace.inwater;
	*pr_global_ptrs.trace_inopen = trace.inopen;

	VectorCopy (trace.endpos, *pr_global_ptrs.trace_endpos);
	VectorCopy (trace.plane.normal, *pr_global_ptrs.trace_plane_normal);

	*pr_global_ptrs.trace_plane_dist = trace.plane.dist;
	if (trace.ent)
		*pr_global_ptrs.trace_ent = EDICT_TO_PROG(trace.ent);
	else
		*pr_global_ptrs.trace_ent = EDICT_TO_PROG(sv.edicts);
}

/*
==============
PF_particle2
==============
*/
void PF_particle2 (void)
{
	float		*org, *dmin, *dmax;
	float		color;
	float		count;
	float		effect;

	org = G_VECTOR(OFS_PARM0);
	dmin = G_VECTOR(OFS_PARM1);
	dmax = G_VECTOR(OFS_PARM2);
	color = G_FLOAT(OFS_PARM3);
	effect = G_FLOAT(OFS_PARM4);
	count = G_FLOAT(OFS_PARM5);
	SV_StartParticle2 (org, dmin, dmax, color, effect, count);
}

/*
==============
PF_vhlen
==============
*/
void PF_vhlen (void)
{
	float	*value1;
	float	result;

	value1 = G_VECTOR(OFS_PARM0);

	result = value1[0] * value1[0] + value1[1] * value1[1];
	result = sqrt(result);

	G_FLOAT(OFS_RETURN) = result;
}

/*
==============
PF_dprintf
==============
*/
void PF_dprintf (void)
{
	char temp[256];
	float	v;

	v = G_FLOAT(OFS_PARM1);

	if (v == (int)v)
		sprintf (temp, "%d",(int)v);
	else
		sprintf (temp, "%5.1f",v);

	Con_DPrintf (G_STRING(OFS_PARM0),temp);
}

/*
==============
PF_AdvanceFrame
==============
*/
void PF_AdvanceFrame (void)
{
	edict_t *ent;
	float start, end, result;

	ent = PROG_TO_EDICT(*pr_global_ptrs.self);
	start = G_FLOAT(OFS_PARM0);
	end = G_FLOAT(OFS_PARM1);

	if (ent->v.frame < start || ent->v.frame > end)
	{ // Didn't start in the range
		ent->v.frame = start;
		result = 0;
	}
	else if (ent->v.frame == end)
	{  // Wrapping
		ent->v.frame = start;
		result = 1;
	}
	else
	{  // Regular Advance
		ent->v.frame++;
		result = (ent->v.frame == end) ? 2 : 0;
	}

	G_FLOAT(OFS_RETURN) = result;
}

/*
==============
PF_dprintv
==============
*/
void PF_dprintv (void)
{
	char temp[256];

	sprintf (temp, "'%5.1f %5.1f %5.1f'", G_VECTOR(OFS_PARM1)[0], G_VECTOR(OFS_PARM1)[1], G_VECTOR(OFS_PARM1)[2]);

	Con_DPrintf (G_STRING(OFS_PARM0),temp);
}

/*
==============
PF_RewindFrame
==============
*/
void PF_RewindFrame (void)
{
	edict_t *ent;
	float start, end, result;

	ent = PROG_TO_EDICT(*pr_global_ptrs.self);
	start = G_FLOAT(OFS_PARM0);
	end = G_FLOAT(OFS_PARM1);

	if (ent->v.frame > start || ent->v.frame < end)
	{ // Didn't start in the range
		ent->v.frame = start;
		result = 0;
	}
	else if (ent->v.frame == end)
	{  // Wrapping
		ent->v.frame = start;
		result = 1;
	}
	else
	{  // Regular Advance
		ent->v.frame--;
		result = (ent->v.frame == end) ? 2 : 0;
	}

	G_FLOAT(OFS_RETURN) = result;
}

/*
==============
PF_setclass
==============
*/
void PF_setclass (void)
{
	float		newClass;
	int			entnum;
	edict_t		*e;
	client_t	*client, *old;

	entnum = G_EDICTNUM(OFS_PARM0);
	e = G_EDICT(OFS_PARM0);
	newClass = G_FLOAT(OFS_PARM1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Print ("tried to setclass of a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	old = host_client;
	host_client = client;
	Host_ClientCommands ("playerclass %i\n", (int)newClass);
	host_client = old;

	// These will get set again after the message has filtered its way
	// but it wouldn't take affect right away

	e->v.playerclass = newClass;
	client->playerclass = newClass;
}

/*
==============
PF_lightstylevalue
==============
*/
void PF_lightstylevalue (void)
{
	int style;

	style = G_FLOAT(OFS_PARM0);
	if(style < 0 || style >= MAX_LIGHTSTYLES)
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

#ifndef RQM_SV_ONLY
	G_FLOAT(OFS_RETURN) = (int)((float)d_lightstylevalue[style]/22.0);
#else
	G_FLOAT(OFS_RETURN) = 0;			//**** FIXME!!! ****
#endif
}

/*
==============
PF_plaque_draw
==============
*/
void PF_plaque_draw (void)
{
	int index;

#ifndef RQM_SV_ONLY
	plaquemessage = G_STRING(OFS_PARM0);
#endif

	index = ((int)G_FLOAT(OFS_PARM1));

	if (index < 0)
		PR_RunError ("PF_plaque_draw: index(%d) < 1", index);

	if (index > pr_string_count)
		PR_RunError ("PF_plaque_draw: index(%d) >= pr_string_count(%d)", index, pr_string_count);

	MSG_WriteCmd (WriteDest(), svc_plaque);
	MSG_WriteShort (WriteDest(), index);
}

/*
==============
PF_rain_go
==============
*/
void PF_rain_go (void)
{
	float *min_org,*max_org,*e_size;
	float *dir;
	vec3_t	org,org2;
	int color,count,x_dir,y_dir;

	min_org = G_VECTOR (OFS_PARM0);
	max_org = G_VECTOR (OFS_PARM1);
	e_size  = G_VECTOR (OFS_PARM2);
	dir		= G_VECTOR (OFS_PARM3);
	color	= G_FLOAT (OFS_PARM4);
	count = G_FLOAT (OFS_PARM5);

	org[0] = min_org[0];
	org[1] = min_org[1];
	org[2] = max_org[2];

	org2[0] = e_size[0];
	org2[1] = e_size[1];
	org2[2] = e_size[2];

	x_dir = dir[0];
	y_dir = dir[1];

	MSG_WriteCmd (&sv.datagram, svc_raineffect);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteCoord (&sv.datagram, e_size[0]);
	MSG_WriteCoord (&sv.datagram, e_size[1]);
	MSG_WriteCoord (&sv.datagram, e_size[2]);
	MSG_WriteAngle (&sv.datagram, x_dir);
	MSG_WriteAngle (&sv.datagram, y_dir);
	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteShort (&sv.datagram, count);
}

/*
==============
PF_particleexplosion
==============
*/
void PF_particleexplosion (void)
{
	float *org;
	int color, radius, counter;

	org = G_VECTOR(OFS_PARM0);
	color = G_FLOAT(OFS_PARM1);
	radius = G_FLOAT(OFS_PARM2);
	counter = G_FLOAT(OFS_PARM3);

	MSG_WriteCmd(&sv.datagram, svc_particle_explosion);
	MSG_WriteCoord(&sv.datagram, org[0]);
	MSG_WriteCoord(&sv.datagram, org[1]);
	MSG_WriteCoord(&sv.datagram, org[2]);
	MSG_WriteShort(&sv.datagram, color);
	MSG_WriteShort(&sv.datagram, radius);
	MSG_WriteShort(&sv.datagram, counter);
}

/*
==============
PF_movestep
==============
*/
void PF_movestep (void)
{
	vec3_t v;
	edict_t	*ent;
	dfunction_t	*oldf;
	int 	oldself;
	qboolean set_trace;

	ent = PROG_TO_EDICT(*pr_global_ptrs.self);

	v[0] = G_FLOAT(OFS_PARM0);
	v[1] = G_FLOAT(OFS_PARM1);
	v[2] = G_FLOAT(OFS_PARM2);
	set_trace = G_FLOAT(OFS_PARM3);

// save program state, because SV_movestep may call other progs
	oldf = pr_xfunction;
	oldself = *pr_global_ptrs.self;

	G_INT(OFS_RETURN) = SV_movestep (ent, v, false, true, set_trace);

// restore program state
	pr_xfunction = oldf;
	*pr_global_ptrs.self = oldself;
}

/*
==============
PF_advanceweaponframe
==============
*/
void PF_advanceweaponframe (void)
{
	edict_t *ent;
	float startframe, endframe;
	float state;

	ent = PROG_TO_EDICT(*pr_global_ptrs.self);
	startframe = G_FLOAT(OFS_PARM0);
	endframe = G_FLOAT(OFS_PARM1);

	if ((endframe > startframe && (ent->v.weaponframe > endframe || ent->v.weaponframe < startframe)) ||
	(endframe < startframe && (ent->v.weaponframe < endframe || ent->v.weaponframe > startframe)) )
	{
		ent->v.weaponframe = startframe;
		state = WF_CYCLE_STARTED;
	}
	else if (ent->v.weaponframe == endframe)
	{
		ent->v.weaponframe = startframe;
		state = WF_CYCLE_WRAPPED;
	}
	else
	{
		if (startframe > endframe)
			ent->v.weaponframe--;
		else if (startframe < endframe)
			ent->v.weaponframe++;

		if (ent->v.weaponframe == endframe)
			state = WF_LAST_FRAME;
		else
			state = WF_NORMAL_ADVANCE;
	}

	G_FLOAT(OFS_RETURN) = state;
}

/*
==============
PF_particle3

particle3(origin, box, color, effect, count)
==============
*/
void PF_particle3 (void)
{
	float	*org, *box;
	float	color;
	float	count;
	float	effect;

	org = G_VECTOR(OFS_PARM0);
	box = G_VECTOR(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	effect = G_FLOAT(OFS_PARM3);
	count = G_FLOAT(OFS_PARM4);
	SV_StartParticle3 (org, box, color, effect, count);
}

/*
==============
PF_particle4

particle4(origin, radius, color, effect, count)
==============
*/
void PF_particle4 (void)
{
	float	*org;
	float	radius;
	float	color;
	float	count;
	float	effect;

	org = G_VECTOR(OFS_PARM0);
	radius = G_FLOAT(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	effect = G_FLOAT(OFS_PARM3);
	count = G_FLOAT(OFS_PARM4);
	SV_StartParticle4 (org, radius, color, effect, count);
}

/*
==============
PF_setpuzzlemodel
==============
*/
void PF_setpuzzlemodel (void)
{
	edict_t	*e;
	char	*m, **check;
	model_t	*mod;
	int		i;
	char	NewName[256];

	e = G_EDICT(OFS_PARM0);
	m = G_STRING(OFS_PARM1);

	Q_snprintfz (NewName, sizeof(NewName), "models/puzzle/%s.mdl", m);
// check to see if model was properly precached
	for (i=0, check = sv.model_precache ; *check ; i++, check++)
		if (COM_FilenamesEqual(*check, NewName))
			break;

	e->v.model = PR_NewString (NewName) - pr_strings;

	if (!*check)
	{
//		PR_RunError ("no precache: %s\n", NewName);
		Con_Print("**** NO PRECACHE FOR PUZZLE PIECE:");
		Con_Printf("**** %s\n",NewName);

		sv.model_precache[i] = e->v.model + pr_strings;
		sv.models[i] = Mod_ForName (NewName, true);
	}

	e->v.modelindex = i; //SV_ModelIndex (m);

	mod = sv.models[ (int)e->v.modelindex];  // Mod_ForName (m, true);

	if (mod)
		SetMinMaxSize (e, mod->mins, mod->maxs, true);
	else
		SetMinMaxSize (e, vec3_origin, vec3_origin, true);
}

/*
==============
PF_starteffect
==============
*/
void PF_starteffect (void)
{
	SV_ParseEffect(&sv.reliable_datagram);
}

/*
==============
PF_endeffect
==============
*/
void PF_endeffect (void)
{
	int index;

	index = G_FLOAT(OFS_PARM0);
	index = G_FLOAT(OFS_PARM1);

	if (!sv.effects[index].type) return;

	sv.effects[index].type = 0;
	MSG_WriteCmd (&sv.reliable_datagram, svc_end_effect);
	MSG_WriteByte (&sv.reliable_datagram, index);
}

/*
==============
PF_precache_puzzle_model
==============
*/
void PF_precache_puzzle_model (void)
{
	int		i;
	char	*s, *m, temp[256];

	if ((sv.state != ss_loading) && !allow_postcache)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");

	m = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);

	Q_snprintfz (temp, sizeof(temp), "models/puzzle/%s.mdl", m);
	s = PR_NewString (temp);

	PR_CheckEmptyString (s);

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
		{
			sv.model_precache[i] = s;
			sv.models[i] = Mod_ForName (s, false);
		/*******JDH*******/
			if ( !sv.models[i] )
			{
				Con_Printf( "WARNING: unable to precache puzzle model %s\n", m );
				sv.model_precache[i] = 0;
			}
		/*******JDH*******/
			return;
		}
		if (COM_FilenamesEqual(sv.model_precache[i], s))
			return;
	}

	PR_RunError ("PF_precache_puzzle_model: overflow");
}

/*
==============
PF_concatv
==============
*/
void PF_concatv (void)
{
	float *in, *range;
	vec3_t result;

	in = G_VECTOR(OFS_PARM0);
	range = G_VECTOR(OFS_PARM1);

	VectorCopy (in, result);
	if (result[0] < -range[0]) result[0] = -range[0];
	if (result[0] > range[0]) result[0] = range[0];
	if (result[1] < -range[1]) result[1] = -range[1];
	if (result[1] > range[1]) result[1] = range[1];
	if (result[2] < -range[2]) result[2] = -range[2];
	if (result[2] > range[2]) result[2] = range[2];

	VectorCopy (result, G_VECTOR(OFS_RETURN));
}

/*
==============
PF_getstring
==============
*/
void PF_getstring (void)
{
	int index;

	index = (int)G_FLOAT(OFS_PARM0) - 1;

	if (index < 0)
		PR_RunError ("PF_GetString: index(%d) < 1", index+1);

	if (index >= pr_string_count)
		PR_RunError ("PF_GetString: index(%d) >= pr_string_count(%d)", index, pr_string_count);

	G_INT(OFS_RETURN) = &pr_global_strings[pr_string_index[index]] - pr_strings;
}

/*
==============
PF_spawn_temp
==============
*/
void PF_spawn_temp (void)
{
	edict_t	*ed;

	ed = ED_Alloc_Temp();

	RETURN_EDICT(ed);
}

/*
==============
PF_v_factor
==============
*/
void PF_v_factor (void)
// returns (v_right * factor_x) + (v_forward * factor_y) + (v_up * factor_z)
{
	float *range;
	vec3_t result;

	range = G_VECTOR(OFS_PARM0);

	result[0] = ((*pr_global_ptrs.v_right)[0] * range[0]) +
				((*pr_global_ptrs.v_forward)[0] * range[1]) +
				((*pr_global_ptrs.v_up)[0] * range[2]);

	result[1] = ((*pr_global_ptrs.v_right)[1] * range[0]) +
				((*pr_global_ptrs.v_forward)[1] * range[1]) +
				((*pr_global_ptrs.v_up)[1] * range[2]);

	result[2] = ((*pr_global_ptrs.v_right)[2] * range[0]) +
				((*pr_global_ptrs.v_forward)[2] * range[1]) +
				((*pr_global_ptrs.v_up)[2] * range[2]);

	VectorCopy (result, G_VECTOR(OFS_RETURN));
}

/*
==============
PF_v_factorrange
==============
*/
void PF_v_factorrange (void)
// returns (v_right * factor_x) + (v_forward * factor_y) + (v_up * factor_z)
{
	float num,*minv,*maxv;
	vec3_t result,r2;

	minv = G_VECTOR(OFS_PARM0);
	maxv = G_VECTOR(OFS_PARM1);

	num = rand()*(1.0/RAND_MAX);//(rand ()&0x7fff) / ((float)0x7fff);
	result[0] = ((maxv[0]-minv[0]) * num) + minv[0];
	num = rand()*(1.0/RAND_MAX);//(rand ()&0x7fff) / ((float)0x7fff);
	result[1] = ((maxv[1]-minv[1]) * num) + minv[1];
	num = rand()*(1.0/RAND_MAX);//(rand ()&0x7fff) / ((float)0x7fff);
	result[2] = ((maxv[2]-minv[2]) * num) + minv[2];

	r2[0] = ((*pr_global_ptrs.v_right)[0] * result[0]) +
			((*pr_global_ptrs.v_forward)[0] * result[1]) +
			((*pr_global_ptrs.v_up)[0] * result[2]);

	r2[1] = ((*pr_global_ptrs.v_right)[1] * result[0]) +
			((*pr_global_ptrs.v_forward)[1] * result[1]) +
			((*pr_global_ptrs.v_up)[1] * result[2]);

	r2[2] = ((*pr_global_ptrs.v_right)[2] * result[0]) +
			((*pr_global_ptrs.v_forward)[2] * result[1]) +
			((*pr_global_ptrs.v_up)[2] * result[2]);

	VectorCopy (r2, G_VECTOR(OFS_RETURN));
}

/*
==============
PF_matchAngleToSlope
==============
*/
void PF_matchAngleToSlope (void)
{
	edict_t	*actor;
	vec3_t v_forward, old_forward, old_right, new_angles2 = { 0, 0, 0 };
	float pitch, mod, dot;

	// OFS_PARM0 is used by PF_vectoangles below
	actor = G_EDICT(OFS_PARM1);

	AngleVectors(actor->v.angles, old_forward, old_right, *pr_global_ptrs.v_up);

	PF_vectoangles();

	pitch = G_FLOAT(OFS_RETURN) - 90;

	new_angles2[1] = G_FLOAT(OFS_RETURN+1);

	AngleVectors(new_angles2, v_forward, *pr_global_ptrs.v_right, *pr_global_ptrs.v_up);

	mod = DotProduct(v_forward, old_right);

	if(mod<0)
		mod=1;
	else
		mod=-1;

	dot = DotProduct(v_forward, old_forward);

	actor->v.angles[0] = dot*pitch;
	actor->v.angles[2] = (1-fastfabs(dot))*pitch*mod;
}

/*
==============
PF_updateInfoPlaque
==============
*/
void PF_updateInfoPlaque (void)
{
	unsigned int check;
	unsigned int index, mode;
	unsigned *use;
	int	ofs = 0;

	index = G_FLOAT(OFS_PARM0);
	mode = G_FLOAT(OFS_PARM1);

	if (index > 31)
	{
		use = &sv.info_mask2;
		ofs = 32;
	}
	else
	{
		use = &sv.info_mask;
	}

//	check = (long) (1 << index - ofs);		// JDH - original - ambiguous
	check = (long) (1 << (index - ofs));

	if (((mode & 1) && ((*use) & check)) || ((mode & 2) && !((*use) & check)));
	else
	{
		(*use) ^= check;
	}
}

/*
==============
PF_doWhiteFlash
==============
*/
void PF_doWhiteFlash (void)
{
#ifndef RQM_SV_ONLY
	V_WhiteFlash_f (SRC_SERVER);
#endif
}

/*
==============
PF_updateSoundPos
==============
*/
void PF_updateSoundPos (void)
{
	int			channel;
	edict_t		*entity;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	SV_UpdateSoundPos (entity, channel);
}

/*
==============
PF_stopSound
==============
*/
void PF_stopSound (void)
{
	int			channel;
	edict_t		*entity;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);

	if (channel < 0 || channel > 7)
		Sys_Error ("SV_StartSound: channel = %i", channel);

	SV_StopSound (entity, channel);
}


// JDH: these are the Hexen II builtins that differ from Quake's:

ebfs_builtin_t pr_builtins_H2[] =
{
//	{ 6,	"debugbreak",		PF_break },				// void debugbreak(void)
	{ 5,	"lightstylestatic", PF_lightstylestatic },	// void lightstylestatic(float style, float value)
	{ 33,	"tracearea",		PF_tracearea },			// float tracearea(vector v1, vector v2, vector mins, vector maxs,
														//					float nomonsters, entity forent)
	{ 42,	"particle2",		PF_particle2 },			// void particle2(vector o, vector dmin, vector dmax, float color,
														//					float type, float count)
	{ 50,	"vhlen",			PF_vhlen },				// float vhlen(vector v)
	{ 60,	"dprintf",			PF_dprintf },			// void dprintf(string s, float num)
	{ 62,	"sin",				PF_sin },				// float sin(float angle)
	{ 63,	"AdvanceFrame",		PF_AdvanceFrame },		// float AdvanceFrame(float start, float end)
	{ 64,	"dprintv",			PF_dprintv },			// void dprintv(string s, vector vec)
	{ 65,	"RewindFrame",		PF_RewindFrame },		// float RewindFrame(float start, float end)
	{ 66,	"setclass",			PF_setclass },			// void setclass(entity e, float value)
	{ 71,	"lightstylevalue",	PF_lightstylevalue },	// float lightstylevalue(float style)
	{ 79,	"plaque_draw",		PF_plaque_draw },		// void plaque_draw(float to, float index)
	{ 80,	"rain_go",			PF_rain_go },			// void rain_go(vector v1, vector v2, vector e_size, vector dir,
														//				float color, float count)
	{ 81,	"particleexplosion", PF_particleexplosion },// void particleexplosion(vector v,float f,float c,float s)
	{ 82,	"movestep",			PF_movestep },			// float movestep(float x, float y, float z, float set_trace)
	{ 83,	"advanceweaponframe", PF_advanceweaponframe },	// float advanceweaponframe(float startframe, float endframe)
	{ 84,	"sqrt",				PF_sqrt },				// float sqrt(float num1)
	{ 85,	"particle3",		PF_particle3 },			// void particle3(vector o, vector box, float color, float type, float count)
	{ 86,	"particle4",		PF_particle4 },			// void particle4(vector o, float radius, float color, float type, float count)
	{ 87,	"setpuzzlemodel",	PF_setpuzzlemodel },	// void setpuzzlemodel(entity e, string m)
	{ 88,	"starteffect",		PF_starteffect },		// float starteffect(...)
	{ 89,	"endeffect",		PF_endeffect },			// float endeffect(float to, float effect_id)
	{ 90,	"precache_puzzle_model", PF_precache_puzzle_model },	// string precache_puzzle_model(string s)
	{ 91,	"concatv",			PF_concatv },			// vector concatv(vector in, vector limit)
	{ 92,	"getstring",		PF_getstring },			// string getstring(float id)
	{ 93,	"spawn_temp",		PF_spawn_temp },		// entity spawn_temp(void)
	{ 94,	"v_factor",			PF_v_factor },			// vector v_factor(vector factor)
	{ 95,	"v_factorrange",	PF_v_factorrange },		// vector v_factorrange(vector start, vector end)
	{ 96,	"precache_sound3",	PF_precache_sound },	// string precache_sound3(string s)
	{ 97,	"precache_model3",	PF_precache_model },	// string precache_model3(string s)
	{ 98,	"precache_file3",	PF_precache_file },		// string precache_file3(string s)
	{ 99,	"matchAngleToSlope", PF_matchAngleToSlope },	// void matchAngleToSlope(vector slope, entity who)
	{100,	"updateInfoPlaque",	PF_updateInfoPlaque },	// void updateInfoPlaque(float text_id, float mode)
	{101,	"precache_sound4",	PF_precache_sound },	// string precache_sound4(string s)
	{102,	"precache_model4",	PF_precache_model },	// string precache_model4(string s)
	{103,	"precache_file4",	PF_precache_file },		// string precache_file4(string s)
	{104,	"doWhiteFlash",		PF_doWhiteFlash },		// void doWhiteFlash(void)
	{105,	"updateSoundPos",	PF_updateSoundPos },	// void updateSoundPos(entity e, float chan)
	{106,	"stopSound",		PF_stopSound }			// void stopSound(entity e, float chan)
};

int pr_numbuiltins_H2 = sizeof(pr_builtins_H2)/sizeof(pr_builtins_H2[0]);

#endif		// #ifdef HEXEN2_SUPPORT
