/* JDH: split off from progdefs.h */

	int			pad[28];
	int			self;
	int			other;
	int			world;
	float		time;
	float		frametime;
#ifndef PRGLOBALS_BETA
	float		force_retouch;		// not in beta QC
#endif
	string_t	mapname;
	float		deathmatch;
	float		coop;
	float		teamplay;
	float		serverflags;
	float		total_secrets;
	float		total_monsters;
	float		found_secrets;
	float		killed_monsters;
	float		parm1;
	float		parm2;
	float		parm3;
	float		parm4;
	float		parm5;
	float		parm6;
	float		parm7;
	float		parm8;
	float		parm9;
	float		parm10;
	float		parm11;
	float		parm12;
	float		parm13;
	float		parm14;
	float		parm15;
	float		parm16;
	vec3_t		v_forward;
	vec3_t		v_up;
	vec3_t		v_right;
	float		trace_allsolid;
	float		trace_startsolid;
	float		trace_fraction;
	vec3_t		trace_endpos;
	vec3_t		trace_plane_normal;
	float		trace_plane_dist;
	int			trace_ent;
	float		trace_inopen;
	float		trace_inwater;
#ifndef PRGLOBALS_BETA
	int			msg_entity;			// not in beta QC
#endif
	func_t		main;
	func_t		StartFrame;
	func_t		PlayerPreThink;
	func_t		PlayerPostThink;
	func_t		ClientKill;
	func_t		ClientConnect;
	func_t		PutClientInServer;
	func_t		ClientDisconnect;
	func_t		SetNewParms;
	func_t		SetChangeParms;
