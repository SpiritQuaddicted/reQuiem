
#include "music.h"

#ifndef RQM_SV_ONLY

#ifdef HEXEN2_SUPPORT

extern int		*pr_string_index;
extern char		*pr_global_strings;
extern int		pr_string_count;
extern cvar_t	bgmtype;

char	*plaquemessage = NULL;   // Pointer to current plaque message

int particle_remap[] = 
{ 
	pt_static, pt_grav, pt_fastgrav, pt_slowgrav, pt_fire, pt_explode, pt_explode2,
	pt_blob, pt_blob2, pt_rain, pt_c_explode, pt_c_explode2, pt_spit, pt_fireball,
	pt_ice, pt_spell, pt_test, pt_quake, pt_rd, pt_vorpal, pt_setstaff, pt_magicmissile,
	pt_boneshard, pt_scarab, pt_acidball, pt_darken, pt_snow, pt_gravwell, pt_redfire
};

/*
=======================
CL_ParseMIDI (Hexen II)
=======================
*/
void CL_ParseMIDI (void)
{
	Q_strcpy (cl.midi_name, MSG_ReadString(), sizeof(cl.midi_name));

	if (!Q_strcasecmp (bgmtype.string, "midi"))
		Music_PlayMIDI (SRC_SERVER, cl.midi_name);
	else 
		Music_Stop_f (SRC_SERVER);
}

/*
=======================
CL_ParseUpdateClass (Hexen II)
=======================
*/
void CL_ParseUpdateClass (void)
{
	int i;
	
	Sbar_Changed();
	i = MSG_ReadByte ();
	if (i >= cl.maxclients)
		Host_Error ("CL_ParseServerMessage: svc_updateclass > MAX_SCOREBOARD");
	cl.scores[i].playerclass = (float) MSG_ReadByte();
	CL_NewTranslation(i); // update the color
}

/*
=======================
CL_ParseRainEffect (Hexen II)
=======================
*/
void CL_ParseRainEffect(void)
{
	vec3_t		org, e_size;
	short		color,count;
	int			x_dir, y_dir;

	org[0] = MSG_ReadCoord();
	org[1] = MSG_ReadCoord();
	org[2] = MSG_ReadCoord();
	e_size[0] = MSG_ReadCoord();
	e_size[1] = MSG_ReadCoord();
	e_size[2] = MSG_ReadCoord();
	x_dir = MSG_ReadAngle();
	y_dir = MSG_ReadAngle();
	color = MSG_ReadShort();
	count = MSG_ReadShort();

	R_RainEffect(org, e_size, x_dir, y_dir, color, count);
}

/*
=======================
CL_Plaque (Hexen II)
=======================
*/
void CL_Plaque (void)
{
	int index;

	index = MSG_ReadShort ();

	if (index > 0 && index <= pr_string_count)
	{
		plaquemessage = &pr_global_strings[pr_string_index[index-1]];
		Con_LogCenterPrint (plaquemessage);
	}
	else
		plaquemessage = "";
}

/*
=======================
CL_ParticleExplosion (Hexen II)
=======================
*/
void CL_ParticleExplosion(void)
{
	vec3_t org;
	short color, radius, counter;

	org[0] = MSG_ReadCoord();
	org[1] = MSG_ReadCoord();
	org[2] = MSG_ReadCoord();
	color = MSG_ReadShort();
	radius = MSG_ReadShort();
	counter = MSG_ReadShort();

	R_ColoredParticleExplosion(org,color,radius,counter);
}

/*
=======================
CL_ParseReference (Hexen II)
=======================
*/
qboolean CL_ParseReference (void)
{
	int			i;
	short		RemovePlace, OrigPlace, NewPlace, AddedIndex;
	entity_t	*ent;
	
//	packet_loss = false;
	cl.last_frame = cl.current_frame;
	cl.last_sequence = cl.current_sequence;
	cl.current_frame = MSG_ReadByte();
	cl.current_sequence = MSG_ReadByte();
	if (cl.need_build == 2)
	{
		cl.frames[0].count = cl.frames[1].count = cl.frames[2].count = 0;
		cl.need_build = 1;
		cl.reference_frame = cl.current_frame;
	}
	else if (cl.last_sequence != cl.current_sequence)
	{
		if (cl.reference_frame >= 1 && cl.reference_frame <= MAX_FRAMES)
		{
			RemovePlace = OrigPlace = NewPlace = AddedIndex = 0;
			for(i=0;i<cl.num_entities;i++)
			{
				if (RemovePlace >= cl.NumToRemove || cl.RemoveList[RemovePlace] != i)
				{
					if (NewPlace < cl.frames[1].count &&
						cl.frames[1].states[NewPlace].index == i)
					{
						cl.frames[2].states[AddedIndex] = cl.frames[1].states[NewPlace];
						AddedIndex++;
						cl.frames[2].count++;
					}
					else if (OrigPlace < cl.frames[0].count &&
							 cl.frames[0].states[OrigPlace].index == i)
					{
						cl.frames[2].states[AddedIndex] = cl.frames[0].states[OrigPlace];
						AddedIndex++;
						cl.frames[2].count++;
					}
				}
				else
					RemovePlace++;

				if (cl.frames[0].states[OrigPlace].index == i)
					OrigPlace++;
				if (cl.frames[1].states[NewPlace].index == i)
					NewPlace++;
			}
			cl.frames[0] = cl.frames[2];
		}
		cl.frames[1].count = cl.frames[2].count = 0;
		cl.need_build = 1;
		cl.reference_frame = cl.current_frame;
	}
	else
	{
		cl.need_build = 0;
	}

	for (i=1, ent=cl_entities+1; i<cl.num_entities; i++, ent++)
	{
		ent->baseline.flags &= ~ENT_STATE_ON;
	}

	for (i=0; i<cl.frames[0].count; i++)
	{
		ent = CL_EntityNum (cl.frames[0].states[i].index);
		if (!ent)
			return false;
		
		ent->model = cl.model_precache[cl.frames[0].states[i].modelindex];
		ent->baseline.flags |= ENT_STATE_ON;
	}

	return true;
}

/*
=======================
CL_ClearEdicts (Hexen II)
=======================
*/
qboolean CL_ClearEdicts (void)
{
	int			count, index, i;
	entity_t	*ent;
	
	count = MSG_ReadByte();
	if (cl.need_build)
	{
		cl.NumToRemove = count;
	}
	
	for (i=0; i<count; i++)
	{
		index = MSG_ReadShort();
		if (cl.need_build)
			cl.RemoveList[i] = index;

		ent = CL_EntityNum (index);
		if (!ent)
			return false;
		ent->baseline.flags &= ~ENT_STATE_ON;
	}

	return true;
}

/*
==================
CL_ParseUpdate_H2
==================
*/
qboolean CL_ParseUpdate_H2 (entity_t *ent, int num, int bits)
{
	entity_state_t	*ref_ent, *set_ent, dummy;
	qboolean		forcelink;
	int				i, modnum, flag;
	model_t			*model;

	ent->baseline.flags |= ENT_STATE_ON;

	ref_ent = NULL;

	for (i=0; i < cl.frames[0].count; i++)
	{
		if (cl.frames[0].states[i].index == num)
		{
			ref_ent = &cl.frames[0].states[i];
//			if (entnum == 2) fprintf(FH,"Found Reference\n");
			break;
		}
	}

	if (!ref_ent)
	{
		ref_ent = &ent->baseline;
		ref_ent->index = num;
	}

	if (bits & U_CLEAR_ENT)
	{
		memset(ent, 0, sizeof(entity_t));
		memset(ref_ent, 0, sizeof(entity_state_t));
		ref_ent->index = num;
	}

	if (cl.need_build)		// new sequence, first valid frame
	{
		set_ent = &cl.frames[1].states[cl.frames[1].count];
		cl.frames[1].count++;
		*set_ent = *ref_ent;
	}
	else
		set_ent = &dummy;

	forcelink = (ent->msgtime != cl.mtime_prev) ? true : false;
	ent->msgtime = cl.mtime;
	
	if (bits & U_MODEL)
	{
		modnum = MSG_ReadShort ();
		if (modnum >= MAX_MODELS)
			Host_Error ("CL_ParseModel: bad modnum");
	}
	else
		modnum = ref_ent->modelindex;
		
	model = cl.model_precache[modnum];
	set_ent->modelindex = modnum;
	
	if (model != ent->model)
	{
		ent->model = model;

	// automatic animation (torches, etc) can be either all together
	// or randomized
		if (model)
		{
			if (model->synctype == ST_RAND)
				ent->syncbase = rand()*(1.0/RAND_MAX);//(float)(rand()&0x7fff) / 0x7fff;
			else
				ent->syncbase = 0.0;
		}
		else
			forcelink = true;	// hack to make null model players work
		
		if (num > 0 && num <= cl.maxclients)
			R_TranslatePlayerSkin (num - 1);		// **** FIXME: need Hexen II-specific proc ****
	}
	
	if (bits & U_FRAME)
		set_ent->frame = ent->frame = MSG_ReadByte ();
	else
		ent->frame = ref_ent->frame;

	if (bits & U_COLORMAP_H2)
		set_ent->colormap = i = MSG_ReadByte();
	else
		i = ref_ent->colormap;

	if (num && num <= cl.maxclients)
		ent->colormap = /*ent->sourcecolormap =*/ cl.scores[num-1].translations;
	/*else
		ent->sourcecolormap = vid.colormap;*/

	ent->colorshade = i;
//	ent->colormap = i ? globalcolormap : ent->sourcecolormap;

	if (bits & U_SKIN_H2)
	{
		set_ent->skin = ent->skinnum = MSG_ReadByte();
		set_ent->drawflags = ent->drawflags = MSG_ReadByte();
	}
	else
	{
		ent->skinnum = ref_ent->skin;
		ent->drawflags = ref_ent->drawflags;
	}

	if (bits & U_EFFECTS_H2)
	{
		set_ent->effects = ent->effects = MSG_ReadByte();
//		if (num == 2) fprintf(FH,"Read effects %d\n",set_ent->effects);
	}
	else
	{
		ent->effects = ref_ent->effects;
		//if (num == 2) fprintf(FH,"restored effects %d\n",ref_ent->effects);
	}

// shift the known values for interpolation
	VectorCopy (ent->msg_origin, ent->msg_origin_prev);
	VectorCopy (ent->msg_angles, ent->msg_angles_prev);

	for (i = 0; i < 3; i++)
	{
		if (bits & (U_ORIGIN1 << i))
		{
			set_ent->origin[i] = ent->msg_origin[i] = MSG_ReadCoord();
		}
		else
			ent->msg_origin[i] = ref_ent->origin[i];
	
		flag = (i == 0) ? U_ANGLE1 : (i == 1) ? U_ANGLE2 : U_ANGLE3;
		if (bits & flag)
		{
			set_ent->angles[i] = ent->msg_angles[i] = MSG_ReadAngle();
#ifdef _DEBUG
			if (ent->model && (ent->model->type == mod_brush))
				i *= 1;
#endif
		}
		else
			ent->msg_angles[i] = ref_ent->angles[i];
	}

	// JDH: blatant hack to smooth rotating entities
	if (sv.active && (bits & (U_ANGLE1 | U_ANGLE2 | U_ANGLE3)))
	{
		edict_t *ed = EDICT_NUM(num);
		if (ed)
			VectorCopy (ed->v.angles, ent->msg_angles);
	}

	if (bits & U_SCALE)
	{
		set_ent->scale = ent->scale = MSG_ReadByte();
		set_ent->abslight = ent->abslight = MSG_ReadByte();
	}
	else
	{
		ent->scale = ref_ent->scale;
		ent->abslight = ref_ent->abslight;
	}

	return forcelink;
}

/*
=======================
CL_ParseUpdateInv (Hexen II)
=======================
*/
void CL_ParseUpdateInv (void)
{
	int		sc1, sc2;
	byte	test;

	sc1 = sc2 = 0;

	test = MSG_ReadByte();
	if (test & 1)
		sc1 |= ((int)MSG_ReadByte());
	if (test & 2)
		sc1 |= ((int)MSG_ReadByte())<<8;
	if (test & 4)
		sc1 |= ((int)MSG_ReadByte())<<16;
	if (test & 8)
		sc1 |= ((int)MSG_ReadByte())<<24;
	if (test & 16)
		sc2 |= ((int)MSG_ReadByte());
	if (test & 32)
		sc2 |= ((int)MSG_ReadByte())<<8;
	if (test & 64)
		sc2 |= ((int)MSG_ReadByte())<<16;
	if (test & 128)
		sc2 |= ((int)MSG_ReadByte())<<24;

	if (sc1 & SC1_HEALTH)
		cl.v.health = MSG_ReadShort();
	if (sc1 & SC1_LEVEL)
		cl.v.level = MSG_ReadByte();
	if (sc1 & SC1_INTELLIGENCE)
		cl.v.intelligence = MSG_ReadByte();
	if (sc1 & SC1_WISDOM)
		cl.v.wisdom = MSG_ReadByte();
	if (sc1 & SC1_STRENGTH)
		cl.v.strength = MSG_ReadByte();
	if (sc1 & SC1_DEXTERITY)
		cl.v.dexterity = MSG_ReadByte();
	if (sc1 & SC1_WEAPON)
		cl.v.weapon = MSG_ReadByte();
	if (sc1 & SC1_BLUEMANA)
		cl.v.bluemana = MSG_ReadByte();
	if (sc1 & SC1_GREENMANA)
		cl.v.greenmana = MSG_ReadByte();
	if (sc1 & SC1_EXPERIENCE)
		cl.v.experience = MSG_ReadLong();
	if (sc1 & SC1_CNT_TORCH)
		cl.v.cnt_torch = MSG_ReadByte();
	if (sc1 & SC1_CNT_H_BOOST)
		cl.v.cnt_h_boost = MSG_ReadByte();
	if (sc1 & SC1_CNT_SH_BOOST)
		cl.v.cnt_sh_boost = MSG_ReadByte();
	if (sc1 & SC1_CNT_MANA_BOOST)
		cl.v.cnt_mana_boost = MSG_ReadByte();
	if (sc1 & SC1_CNT_TELEPORT)
		cl.v.cnt_teleport = MSG_ReadByte();
	if (sc1 & SC1_CNT_TOME)
		cl.v.cnt_tome = MSG_ReadByte();
	if (sc1 & SC1_CNT_SUMMON)
		cl.v.cnt_summon = MSG_ReadByte();
	if (sc1 & SC1_CNT_INVISIBILITY)
		cl.v.cnt_invisibility = MSG_ReadByte();
	if (sc1 & SC1_CNT_GLYPH)
		cl.v.cnt_glyph = MSG_ReadByte();
	if (sc1 & SC1_CNT_HASTE)
		cl.v.cnt_haste = MSG_ReadByte();
	if (sc1 & SC1_CNT_BLAST)
		cl.v.cnt_blast = MSG_ReadByte();
	if (sc1 & SC1_CNT_POLYMORPH)
		cl.v.cnt_polymorph = MSG_ReadByte();
	if (sc1 & SC1_CNT_FLIGHT)
		cl.v.cnt_flight = MSG_ReadByte();
	if (sc1 & SC1_CNT_CUBEOFFORCE)
		cl.v.cnt_cubeofforce = MSG_ReadByte();
	if (sc1 & SC1_CNT_INVINCIBILITY)
		cl.v.cnt_invincibility = MSG_ReadByte();
	if (sc1 & SC1_ARTIFACT_ACTIVE)
		cl.v.artifact_active = MSG_ReadFloat();
	if (sc1 & SC1_ARTIFACT_LOW)
		cl.v.artifact_low = MSG_ReadFloat();
	if (sc1 & SC1_MOVETYPE)
		cl.v.movetype = MSG_ReadByte();
	if (sc1 & SC1_CAMERAMODE)
		cl.v.cameramode = MSG_ReadByte();
	if (sc1 & SC1_HASTED)
		cl.v.hasted = MSG_ReadFloat();
	if (sc1 & SC1_INVENTORY)
		cl.v.inventory = MSG_ReadByte();
	if (sc1 & SC1_RINGS_ACTIVE)
		cl.v.rings_active = MSG_ReadFloat();

	if (sc2 & SC2_RINGS_LOW)
		cl.v.rings_low = MSG_ReadFloat();
	if (sc2 & SC2_AMULET)
		cl.v.armor_amulet = MSG_ReadByte();
	if (sc2 & SC2_BRACER)
		cl.v.armor_bracer = MSG_ReadByte();
	if (sc2 & SC2_BREASTPLATE)
		cl.v.armor_breastplate = MSG_ReadByte();
	if (sc2 & SC2_HELMET)
		cl.v.armor_helmet = MSG_ReadByte();
	if (sc2 & SC2_FLIGHT_T)
		cl.v.ring_flight = MSG_ReadByte();
	if (sc2 & SC2_WATER_T)
		cl.v.ring_water = MSG_ReadByte();
	if (sc2 & SC2_TURNING_T)
		cl.v.ring_turning = MSG_ReadByte();
	if (sc2 & SC2_REGEN_T)
		cl.v.ring_regeneration = MSG_ReadByte();
	if (sc2 & SC2_HASTE_T)
		cl.v.haste_time = MSG_ReadFloat();
	if (sc2 & SC2_TOME_T)
		cl.v.tome_time = MSG_ReadFloat();
	if (sc2 & SC2_PUZZLE1)
		Q_snprintfz(cl.puzzle_pieces[0], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE2)
		Q_snprintfz(cl.puzzle_pieces[1], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE3)
		Q_snprintfz(cl.puzzle_pieces[2], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE4)
		Q_snprintfz(cl.puzzle_pieces[3], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE5)
		Q_snprintfz(cl.puzzle_pieces[4], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE6)
		Q_snprintfz(cl.puzzle_pieces[5], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE7)
		Q_snprintfz(cl.puzzle_pieces[6], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_PUZZLE8)
		Q_snprintfz(cl.puzzle_pieces[7], 10, "%.9s", MSG_ReadString());
	if (sc2 & SC2_MAXHEALTH)
		cl.v.max_health = MSG_ReadShort();
	if (sc2 & SC2_MAXMANA)
		cl.v.max_mana = MSG_ReadByte();
	if (sc2 & SC2_FLAGS)
		cl.v.flags = MSG_ReadFloat();
	
	if (cl.protocol == PROTOCOL_VERSION_H2_112)
	{
		if (sc2 & SC2_OBJ)
			cl.info_mask = MSG_ReadLong();
		if (sc2 & SC2_OBJ2)
			cl.info_mask2 = MSG_ReadLong();
	}
	else
	{
		cl.info_mask = 0;
		cl.info_mask2 = 0;
	}

	if ((sc1 & SC1_STAT_BAR) || (sc2 & SC2_STAT_BAR))
		Sbar_Changed();

	if ((sc1 & SC1_INV) || (sc2 & SC2_INV))
		Sbar_InvChanged();
}

/*
=======================
CL_ParseAngleInterpolate (Hexen II)
=======================
*/
void CL_ParseAngleInterpolate (void)
{
	float		compangles[2][3];
	int			i, j;
	vec3_t		deltaangles;
	
	compangles[0][0] = MSG_ReadAngle();
	compangles[0][1] = MSG_ReadAngle();
	compangles[0][2] = MSG_ReadAngle();

	for (i=0 ; i<3 ; i++)
	{
		compangles[1][i] = cl.viewangles[i];
		for (j=0 ; j<2 ; j++)
		{//standardize both old and new angles to +-180
			if(compangles[j][i]>=360)
				compangles[j][i] -= 360*((int)(compangles[j][i]/360));
			else if(compangles[j][i]<=360)
				compangles[j][i] += 360*(1+(int)(-compangles[j][i]/360));
			if(compangles[j][i]>180)
				compangles[j][i]=-360 + compangles[j][i];
			else if(compangles[j][i]<-180)
				compangles[j][i]=360 + compangles[j][i];
		}
		//get delta
		deltaangles[i] = compangles[0][i] - compangles[1][i];
			//cap delta to <=180,>=-180
		if(deltaangles[i]>180)
			deltaangles[i]+=-360;
		else if(deltaangles[i]<-180)
			deltaangles[i]+=360;
		//add the delta
		cl.viewangles[i]+=(deltaangles[i]/8);//8 step interpolation
		//cap newangles to +-180
		if(cl.viewangles[i]>=360)
			cl.viewangles[i] -= 360*((int)(cl.viewangles[i]/360));
		else if(cl.viewangles[i]<=360)
			cl.viewangles[i] += 360*(1+(int)(-cl.viewangles[i]/360));
		if(cl.viewangles[i]>180)
			cl.viewangles[i]+=-360;
		else if(cl.viewangles[i]<-180)
			cl.viewangles[i]+=360;
	}
}

/*
=======================
CL_ParseSoundPos (Hexen II)
=======================
*/
void CL_ParseSoundPos (void)
{
	//FIXME: put a field on the entity that lists the channels
	//it should update when it moves- if a certain flag
	//is on the ent, this update_channels field could
	//be set automatically by each sound and stopSound
	//called for this ent?
	vec3_t  pos;
	int 	channel, ent, i;
	
	channel = MSG_ReadShort ();
	
	ent = channel >> 3;
	channel &= 7;
	
	if (ent > MAX_EDICTS)
		Host_Error ("svc_sound_update_pos: ent = %i", ent);
	
	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();
	
	S_UpdateSoundPos (ent, channel, pos);
}

/*
===============
R_ParseParticleEffect2

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect2 (void)
{
	vec3_t		org, dmin, dmax;
	int			i, msgcount, color, effect;
	
	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	for (i=0 ; i<3 ; i++)
		dmin[i] = MSG_ReadFloat ();
	for (i=0 ; i<3 ; i++)
		dmax[i] = MSG_ReadFloat ();
	color = MSG_ReadShort ();
	msgcount = MSG_ReadByte ();
	effect = particle_remap[ MSG_ReadByte() ];

	R_RunParticleEffect2 (org, dmin, dmax, color, effect, msgcount);
}

/*
===============
R_ParseParticleEffect3

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect3 (void)
{
	vec3_t		org, box;
	int			i, msgcount, color, effect;
	
	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	for (i=0 ; i<3 ; i++)
		box[i] = MSG_ReadByte ();
	color = MSG_ReadShort ();
	msgcount = MSG_ReadByte ();
	effect = particle_remap[ MSG_ReadByte() ];

	R_RunParticleEffect3 (org, box, color, effect, msgcount);
}

/*
===============
R_ParseParticleEffect4

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect4 (void)
{
	vec3_t		org;
	int			i, msgcount, color, effect;
	float		radius;
	
	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	radius = MSG_ReadByte();
	color = MSG_ReadShort ();
	msgcount = MSG_ReadByte ();
	effect = particle_remap[ MSG_ReadByte() ];

	R_RunParticleEffect4 (org, radius, color, effect, msgcount);
}

#endif		// #ifdef HEXEN2_SUPPORT

#endif		//#ifndef RQM_SV_ONLY
