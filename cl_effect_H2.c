
#include "quakedef.h"

#ifdef HEXEN2_SUPPORT

#define CE_NONE				0
#define CE_RAIN				1
#define CE_FOUNTAIN			2
#define CE_QUAKE			3
#define CE_WHITE_SMOKE		4   // whtsmk1.spr
#define CE_BLUESPARK		5	// bspark.spr
#define CE_YELLOWSPARK		6	// spark.spr
#define CE_SM_CIRCLE_EXP	7	// fcircle.spr
#define CE_BG_CIRCLE_EXP	8	// fcircle.spr
#define CE_SM_WHITE_FLASH	9	// sm_white.spr
#define CE_WHITE_FLASH		10	// gryspt.spr
#define CE_YELLOWRED_FLASH  11  // yr_flash.spr
#define CE_BLUE_FLASH       12  // bluflash.spr
#define CE_SM_BLUE_FLASH    13  // bluflash.spr
#define CE_RED_FLASH		14  // redspt.spr
#define CE_SM_EXPLOSION		15  // sm_expld.spr
#define CE_LG_EXPLOSION		16  // bg_expld.spr
#define CE_FLOOR_EXPLOSION	17  // fl_expld.spr
#define CE_RIDER_DEATH		18
#define CE_BLUE_EXPLOSION   19  // xpspblue.spr
#define CE_GREEN_SMOKE      20  // grnsmk1.spr
#define CE_GREY_SMOKE       21  // grysmk1.spr
#define CE_RED_SMOKE        22  // redsmk1.spr
#define CE_SLOW_WHITE_SMOKE 23  // whtsmk1.spr
#define CE_REDSPARK         24  // rspark.spr
#define CE_GREENSPARK       25  // gspark.spr
#define CE_TELESMK1         26  // telesmk1.spr
#define CE_TELESMK2         27  // telesmk2.spr
#define CE_ICEHIT           28  // icehit.spr
#define CE_MEDUSA_HIT       29  // medhit.spr
#define CE_MEZZO_REFLECT    30  // mezzoref.spr
#define CE_FLOOR_EXPLOSION2 31  // flrexpl2.spr
#define CE_XBOW_EXPLOSION   32  // xbowexpl.spr
#define CE_NEW_EXPLOSION    33  // gen_expl.spr
#define CE_MAGIC_MISSILE_EXPLOSION   34  // mm_expld.spr
#define CE_GHOST			35  // ghost.spr
#define CE_BONE_EXPLOSION	36
#define CE_REDCLOUD			37
#define CE_TELEPORTERPUFFS  38
#define CE_TELEPORTERBODY   39
#define CE_BONESHARD		40
#define CE_BONESHRAPNEL		41
#define CE_FLAMESTREAM		42	//Flamethrower
#define CE_SNOW				43
#define CE_GRAVITYWELL		44
#define CE_BLDRN_EXPL		45
#define CE_ACID_MUZZFL		46
#define CE_ACID_HIT			47
#define CE_FIREWALL_SMALL	48
#define CE_FIREWALL_MEDIUM	49
#define CE_FIREWALL_LARGE	50
#define CE_LBALL_EXPL		51
#define	CE_ACID_SPLAT		52
#define	CE_ACID_EXPL		53
#define	CE_FBOOM			54
#define CE_CHUNK			55
#define CE_BOMB				56
#define CE_BRN_BOUNCE		57
#define CE_LSHOCK			58
#define CE_FLAMEWALL		59
#define CE_FLAMEWALL2		60
#define CE_FLOOR_EXPLOSION3 61
#define CE_ONFIRE			62

extern cvar_t	sv_ce_scale;

#ifndef RQM_SV_ONLY

#define MAX_EFFECT_ENTITIES		256

static entity_t EffectEntities[MAX_EFFECT_ENTITIES];
static qboolean EntityUsed[MAX_EFFECT_ENTITIES];
static int EffectEntityCount;

static int NewEffectEntity(void);
static void FreeEffectEntity(int index);

void CL_ClearEffects(void)
{
	memset(cl.effects, 0, sizeof(cl.effects));
	memset(EntityUsed, 0, sizeof(EntityUsed));
	EffectEntityCount = 0;
}

#endif		//#ifndef RQM_SV_ONLY

// All changes need to be in SV_SendEffect(), SV_ParseEffect(),
// SV_SaveEffects(), SV_LoadEffects(), CL_ParseEffect()
void SV_SendEffect(sizebuf_t *sb, int index)
{
	qboolean	DoTest;
	struct effect_t *effect;
	vec3_t		TestO1/*, Diff*/;
	float		/*Size,*/ TestDistance;
	int			i,count;

	DoTest = ((sb == &sv.reliable_datagram) && (sv_ce_scale.value > 0));

	VectorCopy(vec3_origin, TestO1);
	TestDistance = 0;

	effect = &sv.effects[index];
	switch (effect->type)
	{
		case CE_RAIN:
		case CE_SNOW:
		case CE_FOUNTAIN:
			DoTest = false;
			break;

		case CE_QUAKE:
			VectorCopy(effect->Quake.origin, TestO1);
			TestDistance = 700;
			break;

		case CE_WHITE_SMOKE:
		case CE_GREEN_SMOKE:
		case CE_GREY_SMOKE:
		case CE_RED_SMOKE:
		case CE_SLOW_WHITE_SMOKE:
		case CE_TELESMK1:
		case CE_TELESMK2:
		case CE_GHOST:
		case CE_REDCLOUD:
		case CE_FLAMESTREAM:
		case CE_ACID_MUZZFL:
		case CE_FLAMEWALL:
		case CE_FLAMEWALL2:
		case CE_ONFIRE:
		case CE_SM_WHITE_FLASH:
		case CE_YELLOWRED_FLASH:
		case CE_BLUESPARK:
		case CE_YELLOWSPARK:
		case CE_SM_CIRCLE_EXP:
		case CE_BG_CIRCLE_EXP:
		case CE_SM_EXPLOSION:
		case CE_LG_EXPLOSION:
		case CE_FLOOR_EXPLOSION:
		case CE_BLUE_EXPLOSION:
		case CE_REDSPARK:
		case CE_GREENSPARK:
		case CE_ICEHIT:
		case CE_MEDUSA_HIT:
		case CE_MEZZO_REFLECT:
		case CE_FLOOR_EXPLOSION2:
		case CE_XBOW_EXPLOSION:
		case CE_NEW_EXPLOSION:
		case CE_MAGIC_MISSILE_EXPLOSION:
		case CE_BONE_EXPLOSION:
		case CE_BLDRN_EXPL:
		case CE_ACID_HIT:
		case CE_LBALL_EXPL:
		case CE_FIREWALL_SMALL:
		case CE_FIREWALL_MEDIUM:
		case CE_FIREWALL_LARGE:
		case CE_ACID_SPLAT:
		case CE_ACID_EXPL:
		case CE_FBOOM:
		case CE_BRN_BOUNCE:
		case CE_LSHOCK:
		case CE_BOMB:
		case CE_FLOOR_EXPLOSION3:
		case CE_WHITE_FLASH:
		case CE_BLUE_FLASH:
		case CE_SM_BLUE_FLASH:
		case CE_RED_FLASH:
			VectorCopy(effect->Smoke.origin, TestO1);
			TestDistance = 250;
			break;

		case CE_RIDER_DEATH:
		case CE_GRAVITYWELL:
			DoTest = false;
			break;

		case CE_TELEPORTERPUFFS:
		case CE_TELEPORTERBODY:
			VectorCopy(effect->Teleporter.origin, TestO1);
			TestDistance = 350;
			break;

		case CE_BONESHARD:
		case CE_BONESHRAPNEL:
			VectorCopy(effect->Missile.origin, TestO1);
			TestDistance = 600;
			break;

		case CE_CHUNK:
			VectorCopy(effect->Chunk.origin, TestO1);
			TestDistance = 600;
			break;

		default:
//			Sys_Error ("SV_SendEffect: bad type");
			PR_RunError ("SV_SendEffect: bad type");
			break;
	}

	if (!DoTest)
		count = 1;
	else
	{
		count = svs.maxclients;
		TestDistance = (float)TestDistance * sv_ce_scale.value;
		TestDistance *= TestDistance;
	}


	for (i = 0; i < count; i++)
	{
		/*************JDH************
		if (DoTest)
		{
			if (svs.clients[i].active)
			{
				sb = &svs.clients[i].datagram;
				VectorSubtract(svs.clients[i].edict->v.origin,TestO1,Diff);
				Size = (Diff[0]*Diff[0]) + (Diff[1]*Diff[1]) + (Diff[2]*Diff[2]);

				if (Size > TestDistance)
					continue;

				if (sv_ce_max_size.value > 0 && sb->cursize > sv_ce_max_size.value)
					continue;
			}
			else continue;
		}*/

		MSG_WriteByte (sb, svc_start_effect);
		MSG_WriteByte (sb, index);
		MSG_WriteByte (sb, effect->type);

		switch (effect->type)
		{
			case CE_RAIN:
				MSG_WriteCoord(sb, effect->Rain.min_org[0]);
				MSG_WriteCoord(sb, effect->Rain.min_org[1]);
				MSG_WriteCoord(sb, effect->Rain.min_org[2]);
				MSG_WriteCoord(sb, effect->Rain.max_org[0]);
				MSG_WriteCoord(sb, effect->Rain.max_org[1]);
				MSG_WriteCoord(sb, effect->Rain.max_org[2]);
				MSG_WriteCoord(sb, effect->Rain.e_size[0]);
				MSG_WriteCoord(sb, effect->Rain.e_size[1]);
				MSG_WriteCoord(sb, effect->Rain.e_size[2]);
				MSG_WriteCoord(sb, effect->Rain.dir[0]);
				MSG_WriteCoord(sb, effect->Rain.dir[1]);
				MSG_WriteCoord(sb, effect->Rain.dir[2]);
				MSG_WriteShort(sb, effect->Rain.color);
				MSG_WriteShort(sb, effect->Rain.count);
				MSG_WriteFloat(sb, effect->Rain.wait);
				break;

			case CE_SNOW:
				MSG_WriteCoord(sb, effect->Rain.min_org[0]);
				MSG_WriteCoord(sb, effect->Rain.min_org[1]);
				MSG_WriteCoord(sb, effect->Rain.min_org[2]);
				MSG_WriteCoord(sb, effect->Rain.max_org[0]);
				MSG_WriteCoord(sb, effect->Rain.max_org[1]);
				MSG_WriteCoord(sb, effect->Rain.max_org[2]);
				MSG_WriteByte(sb, effect->Rain.flags);
				MSG_WriteCoord(sb, effect->Rain.dir[0]);
				MSG_WriteCoord(sb, effect->Rain.dir[1]);
				MSG_WriteCoord(sb, effect->Rain.dir[2]);
				MSG_WriteByte(sb, effect->Rain.count);
				//MSG_WriteShort(sb, effect->Rain.veer);
				break;

			case CE_FOUNTAIN:
				MSG_WriteCoord(sb, effect->Fountain.pos[0]);
				MSG_WriteCoord(sb, effect->Fountain.pos[1]);
				MSG_WriteCoord(sb, effect->Fountain.pos[2]);
				MSG_WriteAngle(sb, effect->Fountain.angle[0]);
				MSG_WriteAngle(sb, effect->Fountain.angle[1]);
				MSG_WriteAngle(sb, effect->Fountain.angle[2]);
				MSG_WriteCoord(sb, effect->Fountain.movedir[0]);
				MSG_WriteCoord(sb, effect->Fountain.movedir[1]);
				MSG_WriteCoord(sb, effect->Fountain.movedir[2]);
				MSG_WriteShort(sb, effect->Fountain.color);
				MSG_WriteByte(sb, effect->Fountain.cnt);
				break;

			case CE_QUAKE:
				MSG_WriteCoord(sb, effect->Quake.origin[0]);
				MSG_WriteCoord(sb, effect->Quake.origin[1]);
				MSG_WriteCoord(sb, effect->Quake.origin[2]);
				MSG_WriteFloat(sb, effect->Quake.radius);
				break;

			case CE_WHITE_SMOKE:
			case CE_GREEN_SMOKE:
			case CE_GREY_SMOKE:
			case CE_RED_SMOKE:
			case CE_SLOW_WHITE_SMOKE:
			case CE_TELESMK1:
			case CE_TELESMK2:
			case CE_GHOST:
			case CE_REDCLOUD:
			case CE_FLAMESTREAM:
			case CE_ACID_MUZZFL:
			case CE_FLAMEWALL:
			case CE_FLAMEWALL2:
			case CE_ONFIRE:
				MSG_WriteCoord(sb, effect->Smoke.origin[0]);
				MSG_WriteCoord(sb, effect->Smoke.origin[1]);
				MSG_WriteCoord(sb, effect->Smoke.origin[2]);
				MSG_WriteFloat(sb, effect->Smoke.velocity[0]);
				MSG_WriteFloat(sb, effect->Smoke.velocity[1]);
				MSG_WriteFloat(sb, effect->Smoke.velocity[2]);
				MSG_WriteFloat(sb, effect->Smoke.framelength);
				MSG_WriteFloat(sb, effect->Smoke.frame);
				break;

			case CE_SM_WHITE_FLASH:
			case CE_YELLOWRED_FLASH:
			case CE_BLUESPARK:
			case CE_YELLOWSPARK:
			case CE_SM_CIRCLE_EXP:
			case CE_BG_CIRCLE_EXP:
			case CE_SM_EXPLOSION:
			case CE_LG_EXPLOSION:
			case CE_FLOOR_EXPLOSION:
			case CE_FLOOR_EXPLOSION3:
			case CE_BLUE_EXPLOSION:
			case CE_REDSPARK:
			case CE_GREENSPARK:
			case CE_ICEHIT:
			case CE_MEDUSA_HIT:
			case CE_MEZZO_REFLECT:
			case CE_FLOOR_EXPLOSION2:
			case CE_XBOW_EXPLOSION:
			case CE_NEW_EXPLOSION:
			case CE_MAGIC_MISSILE_EXPLOSION:
			case CE_BONE_EXPLOSION:
			case CE_BLDRN_EXPL:
			case CE_ACID_HIT:
			case CE_ACID_SPLAT:
			case CE_ACID_EXPL:
			case CE_LBALL_EXPL:
			case CE_FIREWALL_SMALL:
			case CE_FIREWALL_MEDIUM:
			case CE_FIREWALL_LARGE:
			case CE_FBOOM:
			case CE_BOMB:
			case CE_BRN_BOUNCE:
			case CE_LSHOCK:
			case CE_WHITE_FLASH:
			case CE_BLUE_FLASH:
			case CE_SM_BLUE_FLASH:
			case CE_RED_FLASH:
				MSG_WriteCoord(sb, effect->Smoke.origin[0]);
				MSG_WriteCoord(sb, effect->Smoke.origin[1]);
				MSG_WriteCoord(sb, effect->Smoke.origin[2]);
				break;

			case CE_RIDER_DEATH:
				MSG_WriteCoord(sb, effect->RD.origin[0]);
				MSG_WriteCoord(sb, effect->RD.origin[1]);
				MSG_WriteCoord(sb, effect->RD.origin[2]);
				break;

			case CE_TELEPORTERPUFFS:
				MSG_WriteCoord(sb, effect->Teleporter.origin[0]);
				MSG_WriteCoord(sb, effect->Teleporter.origin[1]);
				MSG_WriteCoord(sb, effect->Teleporter.origin[2]);
				break;

			case CE_TELEPORTERBODY:
				MSG_WriteCoord(sb, effect->Teleporter.origin[0]);
				MSG_WriteCoord(sb, effect->Teleporter.origin[1]);
				MSG_WriteCoord(sb, effect->Teleporter.origin[2]);
				MSG_WriteFloat(sb, effect->Teleporter.velocity[0][0]);
				MSG_WriteFloat(sb, effect->Teleporter.velocity[0][1]);
				MSG_WriteFloat(sb, effect->Teleporter.velocity[0][2]);
				MSG_WriteFloat(sb, effect->Teleporter.skinnum);
				break;

			case CE_BONESHARD:
			case CE_BONESHRAPNEL:
				MSG_WriteCoord(sb, effect->Missile.origin[0]);
				MSG_WriteCoord(sb, effect->Missile.origin[1]);
				MSG_WriteCoord(sb, effect->Missile.origin[2]);
				MSG_WriteFloat(sb, effect->Missile.velocity[0]);
				MSG_WriteFloat(sb, effect->Missile.velocity[1]);
				MSG_WriteFloat(sb, effect->Missile.velocity[2]);
				MSG_WriteFloat(sb, effect->Missile.angle[0]);
				MSG_WriteFloat(sb, effect->Missile.angle[1]);
				MSG_WriteFloat(sb, effect->Missile.angle[2]);
				MSG_WriteFloat(sb, effect->Missile.avelocity[0]);
				MSG_WriteFloat(sb, effect->Missile.avelocity[1]);
				MSG_WriteFloat(sb, effect->Missile.avelocity[2]);

				break;

			case CE_GRAVITYWELL:
				MSG_WriteCoord(sb, effect->RD.origin[0]);
				MSG_WriteCoord(sb, effect->RD.origin[1]);
				MSG_WriteCoord(sb, effect->RD.origin[2]);
				MSG_WriteShort(sb, effect->RD.color);
				MSG_WriteFloat(sb, effect->RD.lifetime);
				break;

			case CE_CHUNK:
				MSG_WriteCoord(sb, effect->Chunk.origin[0]);
				MSG_WriteCoord(sb, effect->Chunk.origin[1]);
				MSG_WriteCoord(sb, effect->Chunk.origin[2]);
				MSG_WriteByte (sb, effect->Chunk.type);
				MSG_WriteCoord(sb, effect->Chunk.srcVel[0]);
				MSG_WriteCoord(sb, effect->Chunk.srcVel[1]);
				MSG_WriteCoord(sb, effect->Chunk.srcVel[2]);
				MSG_WriteByte (sb, effect->Chunk.numChunks);

				//Con_Printf("Adding %d chunks on server...\n",sv.effects[index].Chunk.numChunks);
				break;

			default:
	//			Sys_Error ("SV_SendEffect: bad type");
				PR_RunError ("SV_SendEffect: bad type");
				break;
		}
	}
}

void SV_UpdateEffects(sizebuf_t *sb)
{
	int index;

	for(index=0;index<MAX_EFFECTS;index++)
		if (sv.effects[index].type)
			SV_SendEffect(sb,index);
}

void SV_ParseEffect(sizebuf_t *sb)
{
	int			index;
	byte		type;
	struct effect_t	*effect;


	type = G_FLOAT(OFS_PARM0);

	for (index = 0; index < MAX_EFFECTS; index++)
	{
		effect = &sv.effects[index];
		if (!effect->type || (effect->expire_time && (effect->expire_time <= sv.time)))
			break;
	}

	if (index >= MAX_EFFECTS)
	{
		PR_RunError ("MAX_EFFECTS reached");
		return;
	}

//	Con_Printf("Effect #%d\n",index);

	memset(effect, 0, sizeof(struct effect_t));

	effect->type = type;
	G_FLOAT(OFS_RETURN) = index;

	switch(type)
	{
		case CE_RAIN:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Rain.min_org);
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Rain.max_org);
			VectorCopy(G_VECTOR(OFS_PARM3), effect->Rain.e_size);
			VectorCopy(G_VECTOR(OFS_PARM4), effect->Rain.dir);
			effect->Rain.color = G_FLOAT(OFS_PARM5);
			effect->Rain.count = G_FLOAT(OFS_PARM6);
			effect->Rain.wait = G_FLOAT(OFS_PARM7);
			effect->Rain.next_time = 0;
			break;

		case CE_SNOW:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Rain.min_org);
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Rain.max_org);
			effect->Rain.flags = G_FLOAT(OFS_PARM3);
			VectorCopy(G_VECTOR(OFS_PARM4), effect->Rain.dir);
			effect->Rain.count = G_FLOAT(OFS_PARM5);
			effect->Rain.next_time = 0;
			break;

		case CE_FOUNTAIN:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Fountain.pos);
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Fountain.angle);
			VectorCopy(G_VECTOR(OFS_PARM3), effect->Fountain.movedir);
			effect->Fountain.color = G_FLOAT(OFS_PARM4);
			effect->Fountain.cnt = G_FLOAT(OFS_PARM5);
			break;

		case CE_QUAKE:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Quake.origin);
			effect->Quake.radius = G_FLOAT(OFS_PARM2);
			break;

		case CE_WHITE_SMOKE:
		case CE_GREEN_SMOKE:
		case CE_GREY_SMOKE:
		case CE_RED_SMOKE:
		case CE_SLOW_WHITE_SMOKE:
		case CE_TELESMK1:
		case CE_TELESMK2:
		case CE_GHOST:
		case CE_REDCLOUD:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Smoke.origin);
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Smoke.velocity);
			effect->Smoke.framelength = G_FLOAT(OFS_PARM3);
			effect->Smoke.frame = 0;
			effect->expire_time = sv.time + 1;
			break;

		case CE_ACID_MUZZFL:
		case CE_FLAMESTREAM:
		case CE_FLAMEWALL:
		case CE_FLAMEWALL2:
		case CE_ONFIRE:
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Smoke.velocity);
			effect->Smoke.framelength = HX_FRAME_TIME;
			effect->Smoke.frame = G_FLOAT(OFS_PARM3);
		case CE_SM_WHITE_FLASH:
		case CE_YELLOWRED_FLASH:
		case CE_BLUESPARK:
		case CE_YELLOWSPARK:
		case CE_SM_CIRCLE_EXP:
		case CE_BG_CIRCLE_EXP:
		case CE_SM_EXPLOSION:
		case CE_LG_EXPLOSION:
		case CE_FLOOR_EXPLOSION:
		case CE_FLOOR_EXPLOSION3:
		case CE_BLUE_EXPLOSION:
		case CE_REDSPARK:
		case CE_GREENSPARK:
		case CE_ICEHIT:
		case CE_MEDUSA_HIT:
		case CE_MEZZO_REFLECT:
		case CE_FLOOR_EXPLOSION2:
		case CE_XBOW_EXPLOSION:
		case CE_NEW_EXPLOSION:
		case CE_MAGIC_MISSILE_EXPLOSION:
		case CE_BONE_EXPLOSION:
		case CE_BLDRN_EXPL:
		case CE_ACID_HIT:
		case CE_ACID_SPLAT:
		case CE_ACID_EXPL:
		case CE_LBALL_EXPL:
		case CE_FIREWALL_SMALL:
		case CE_FIREWALL_MEDIUM:
		case CE_FIREWALL_LARGE:
		case CE_FBOOM:
		case CE_BOMB:
		case CE_BRN_BOUNCE:
		case CE_LSHOCK:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Smoke.origin);
			effect->expire_time = sv.time + 1;
			break;

		case CE_WHITE_FLASH:
		case CE_BLUE_FLASH:
		case CE_SM_BLUE_FLASH:
		case CE_RED_FLASH:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Flash.origin);
			effect->expire_time = sv.time + 1;
			break;

		case CE_GRAVITYWELL:
			effect->RD.color = G_FLOAT(OFS_PARM2);
			effect->RD.lifetime = G_FLOAT(OFS_PARM3);
		case CE_RIDER_DEATH:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->RD.origin);
			break;

		case CE_TELEPORTERBODY:
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Teleporter.velocity[0]);
			effect->Teleporter.skinnum = G_FLOAT(OFS_PARM3);
		case CE_TELEPORTERPUFFS:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Teleporter.origin);
			effect->expire_time = sv.time + 1;
			break;

		case CE_BONESHARD:
		case CE_BONESHRAPNEL:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Missile.origin);
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Missile.velocity);
			VectorCopy(G_VECTOR(OFS_PARM3), effect->Missile.angle);
			VectorCopy(G_VECTOR(OFS_PARM2), effect->Missile.avelocity);

			effect->expire_time = sv.time + 10;
			break;

		case CE_CHUNK:
			VectorCopy(G_VECTOR(OFS_PARM1), effect->Chunk.origin);
			effect->Chunk.type = G_FLOAT(OFS_PARM2);
			VectorCopy(G_VECTOR(OFS_PARM3), effect->Chunk.srcVel);
			effect->Chunk.numChunks = G_FLOAT(OFS_PARM4);

			effect->expire_time = sv.time + 3;
			break;


		default:
//			Sys_Error ("SV_ParseEffect: bad type");
			PR_RunError ("SV_SendEffect: bad type");
	}

	SV_SendEffect(sb, index);
}

void SV_SaveEffects(FILE *FH)
{
	int index, count;
	struct effect_t *effect;

	for (index = count = 0; index < MAX_EFFECTS; index++)
	{
		if (sv.effects[index].type)
			count++;
	}

	fprintf(FH, "Effects: %d\n", count);

	for (index = count = 0; index < MAX_EFFECTS; index++)
	{
		effect = &sv.effects[index];
		if (effect->type)
		{
			fprintf(FH,"Effect: %d %d %f: ",index,effect->type,effect->expire_time);

			switch(effect->type)
			{
				case CE_RAIN:
					fprintf(FH, "%f ", effect->Rain.min_org[0]);
					fprintf(FH, "%f ", effect->Rain.min_org[1]);
					fprintf(FH, "%f ", effect->Rain.min_org[2]);
					fprintf(FH, "%f ", effect->Rain.max_org[0]);
					fprintf(FH, "%f ", effect->Rain.max_org[1]);
					fprintf(FH, "%f ", effect->Rain.max_org[2]);
					fprintf(FH, "%f ", effect->Rain.e_size[0]);
					fprintf(FH, "%f ", effect->Rain.e_size[1]);
					fprintf(FH, "%f ", effect->Rain.e_size[2]);
					fprintf(FH, "%f ", effect->Rain.dir[0]);
					fprintf(FH, "%f ", effect->Rain.dir[1]);
					fprintf(FH, "%f ", effect->Rain.dir[2]);
					fprintf(FH, "%d ", effect->Rain.color);
					fprintf(FH, "%d ", effect->Rain.count);
					fprintf(FH, "%f\n", effect->Rain.wait);
					break;

				case CE_SNOW:
					fprintf(FH, "%f ", effect->Rain.min_org[0]);
					fprintf(FH, "%f ", effect->Rain.min_org[1]);
					fprintf(FH, "%f ", effect->Rain.min_org[2]);
					fprintf(FH, "%f ", effect->Rain.max_org[0]);
					fprintf(FH, "%f ", effect->Rain.max_org[1]);
					fprintf(FH, "%f ", effect->Rain.max_org[2]);
					fprintf(FH, "%d ", effect->Rain.flags);
					fprintf(FH, "%f ", effect->Rain.dir[0]);
					fprintf(FH, "%f ", effect->Rain.dir[1]);
					fprintf(FH, "%f ", effect->Rain.dir[2]);
					fprintf(FH, "%d ", effect->Rain.count);
					//fprintf(FH, "%d ", effect->Rain.veer);
					break;

				case CE_FOUNTAIN:
					fprintf(FH, "%f ", effect->Fountain.pos[0]);
					fprintf(FH, "%f ", effect->Fountain.pos[1]);
					fprintf(FH, "%f ", effect->Fountain.pos[2]);
					fprintf(FH, "%f ", effect->Fountain.angle[0]);
					fprintf(FH, "%f ", effect->Fountain.angle[1]);
					fprintf(FH, "%f ", effect->Fountain.angle[2]);
					fprintf(FH, "%f ", effect->Fountain.movedir[0]);
					fprintf(FH, "%f ", effect->Fountain.movedir[1]);
					fprintf(FH, "%f ", effect->Fountain.movedir[2]);
					fprintf(FH, "%d ", effect->Fountain.color);
					fprintf(FH, "%d\n", effect->Fountain.cnt);
					break;

				case CE_QUAKE:
					fprintf(FH, "%f ", effect->Quake.origin[0]);
					fprintf(FH, "%f ", effect->Quake.origin[1]);
					fprintf(FH, "%f ", effect->Quake.origin[2]);
					fprintf(FH, "%f\n", effect->Quake.radius);
					break;

				case CE_WHITE_SMOKE:
				case CE_GREEN_SMOKE:
				case CE_GREY_SMOKE:
				case CE_RED_SMOKE:
				case CE_SLOW_WHITE_SMOKE:
				case CE_TELESMK1:
				case CE_TELESMK2:
				case CE_GHOST:
				case CE_REDCLOUD:
				case CE_ACID_MUZZFL:
				case CE_FLAMESTREAM:
				case CE_FLAMEWALL:
				case CE_FLAMEWALL2:
				case CE_ONFIRE:
					fprintf(FH, "%f ", effect->Smoke.origin[0]);
					fprintf(FH, "%f ", effect->Smoke.origin[1]);
					fprintf(FH, "%f ", effect->Smoke.origin[2]);
					fprintf(FH, "%f ", effect->Smoke.velocity[0]);
					fprintf(FH, "%f ", effect->Smoke.velocity[1]);
					fprintf(FH, "%f ", effect->Smoke.velocity[2]);
					fprintf(FH, "%f ", effect->Smoke.framelength);
					fprintf(FH, "%f\n", effect->Smoke.frame);
					break;

				case CE_SM_WHITE_FLASH:
				case CE_YELLOWRED_FLASH:
				case CE_BLUESPARK:
				case CE_YELLOWSPARK:
				case CE_SM_CIRCLE_EXP:
				case CE_BG_CIRCLE_EXP:
				case CE_SM_EXPLOSION:
				case CE_LG_EXPLOSION:
				case CE_FLOOR_EXPLOSION:
				case CE_FLOOR_EXPLOSION3:
				case CE_BLUE_EXPLOSION:
				case CE_REDSPARK:
				case CE_GREENSPARK:
				case CE_ICEHIT:
				case CE_MEDUSA_HIT:
				case CE_MEZZO_REFLECT:
				case CE_FLOOR_EXPLOSION2:
				case CE_XBOW_EXPLOSION:
				case CE_NEW_EXPLOSION:
				case CE_MAGIC_MISSILE_EXPLOSION:
				case CE_BONE_EXPLOSION:
				case CE_BLDRN_EXPL:
				case CE_BRN_BOUNCE:
				case CE_LSHOCK:
				case CE_ACID_HIT:
				case CE_ACID_SPLAT:
				case CE_ACID_EXPL:
				case CE_LBALL_EXPL:
				case CE_FIREWALL_SMALL:
				case CE_FIREWALL_MEDIUM:
				case CE_FIREWALL_LARGE:
				case CE_FBOOM:
				case CE_BOMB:
					fprintf(FH, "%f ", effect->Smoke.origin[0]);
					fprintf(FH, "%f ", effect->Smoke.origin[1]);
					fprintf(FH, "%f\n", effect->Smoke.origin[2]);
					break;

				case CE_WHITE_FLASH:
				case CE_BLUE_FLASH:
				case CE_SM_BLUE_FLASH:
				case CE_RED_FLASH:
					fprintf(FH, "%f ", effect->Flash.origin[0]);
					fprintf(FH, "%f ", effect->Flash.origin[1]);
					fprintf(FH, "%f\n", effect->Flash.origin[2]);
					break;

				case CE_RIDER_DEATH:
					fprintf(FH, "%f ", effect->RD.origin[0]);
					fprintf(FH, "%f ", effect->RD.origin[1]);
					fprintf(FH, "%f\n", effect->RD.origin[2]);
					break;

				case CE_GRAVITYWELL:
					fprintf(FH, "%f ", effect->RD.origin[0]);
					fprintf(FH, "%f ", effect->RD.origin[1]);
					fprintf(FH, "%f", effect->RD.origin[2]);
					fprintf(FH, "%d", effect->RD.color);
					fprintf(FH, "%f\n", effect->RD.lifetime);
					break;
				case CE_TELEPORTERPUFFS:
					fprintf(FH, "%f ", effect->Teleporter.origin[0]);
					fprintf(FH, "%f ", effect->Teleporter.origin[1]);
					fprintf(FH, "%f\n", effect->Teleporter.origin[2]);
					break;

				case CE_TELEPORTERBODY:
					fprintf(FH, "%f ", effect->Teleporter.origin[0]);
					fprintf(FH, "%f ", effect->Teleporter.origin[1]);
					fprintf(FH, "%f\n", effect->Teleporter.origin[2]);
					break;

				case CE_BONESHARD:
				case CE_BONESHRAPNEL:
					fprintf(FH, "%f ", effect->Missile.origin[0]);
					fprintf(FH, "%f ", effect->Missile.origin[1]);
					fprintf(FH, "%f ", effect->Missile.origin[2]);
					fprintf(FH, "%f ", effect->Missile.velocity[0]);
					fprintf(FH, "%f ", effect->Missile.velocity[1]);
					fprintf(FH, "%f ", effect->Missile.velocity[2]);
					fprintf(FH, "%f ", effect->Missile.angle[0]);
					fprintf(FH, "%f ", effect->Missile.angle[1]);
					fprintf(FH, "%f ", effect->Missile.angle[2]);
					fprintf(FH, "%f ", effect->Missile.avelocity[0]);
					fprintf(FH, "%f ", effect->Missile.avelocity[1]);
					fprintf(FH, "%f ", effect->Missile.avelocity[2]);
					break;

				case CE_CHUNK:
					fprintf(FH, "%f ", effect->Chunk.origin[0]);
					fprintf(FH, "%f ", effect->Chunk.origin[1]);
					fprintf(FH, "%f ", effect->Chunk.origin[2]);
					fprintf(FH, "%d ", effect->Chunk.type);
					fprintf(FH, "%f ", effect->Chunk.srcVel[0]);
					fprintf(FH, "%f ", effect->Chunk.srcVel[1]);
					fprintf(FH, "%f ", effect->Chunk.srcVel[2]);
					fprintf(FH, "%d ", effect->Chunk.numChunks);
					break;

				default:
					PR_RunError ("SV_SaveEffect: bad type");
					break;
			}

		}
	}
}

/*
===============
SV_ClearEffects
===============
*/
void SV_ClearEffects(void)
{
	memset(sv.effects, 0, sizeof(sv.effects));
}

/*
===============
SV_LoadEffects
===============
*/
void SV_LoadEffects(FILE *FH)
{
	int index, total, count;
	struct effect_t *effect;

	// Since the map is freshly loaded, clear out any effects as a result of
	// the loading
	SV_ClearEffects();

	fscanf(FH, "Effects: %d\n", &total);

	for (count = 0; count < total; count++)
	{
		fscanf(FH, "Effect: %d ", &index);
		effect = &sv.effects[index];

		fscanf(FH, "%d %f: ", &effect->type, &effect->expire_time);

		switch (effect->type)
		{
			case CE_RAIN:
				fscanf(FH, "%f ", &effect->Rain.min_org[0]);
				fscanf(FH, "%f ", &effect->Rain.min_org[1]);
				fscanf(FH, "%f ", &effect->Rain.min_org[2]);
				fscanf(FH, "%f ", &effect->Rain.max_org[0]);
				fscanf(FH, "%f ", &effect->Rain.max_org[1]);
				fscanf(FH, "%f ", &effect->Rain.max_org[2]);
				fscanf(FH, "%f ", &effect->Rain.e_size[0]);
				fscanf(FH, "%f ", &effect->Rain.e_size[1]);
				fscanf(FH, "%f ", &effect->Rain.e_size[2]);
				fscanf(FH, "%f ", &effect->Rain.dir[0]);
				fscanf(FH, "%f ", &effect->Rain.dir[1]);
				fscanf(FH, "%f ", &effect->Rain.dir[2]);
				fscanf(FH, "%d ", &effect->Rain.color);
				fscanf(FH, "%d ", &effect->Rain.count);
				fscanf(FH, "%f\n", &effect->Rain.wait);
				break;

			case CE_SNOW:
				fscanf(FH, "%f ", &effect->Rain.min_org[0]);
				fscanf(FH, "%f ", &effect->Rain.min_org[1]);
				fscanf(FH, "%f ", &effect->Rain.min_org[2]);
				fscanf(FH, "%f ", &effect->Rain.max_org[0]);
				fscanf(FH, "%f ", &effect->Rain.max_org[1]);
				fscanf(FH, "%f ", &effect->Rain.max_org[2]);
				fscanf(FH, "%d ", &effect->Rain.flags);
				fscanf(FH, "%f ", &effect->Rain.dir[0]);
				fscanf(FH, "%f ", &effect->Rain.dir[1]);
				fscanf(FH, "%f ", &effect->Rain.dir[2]);
				fscanf(FH, "%d ", &effect->Rain.count);
				//fscanf(FH, "%d ", &effect->Rain.veer);
				break;

			case CE_FOUNTAIN:
				fscanf(FH, "%f ", &effect->Fountain.pos[0]);
				fscanf(FH, "%f ", &effect->Fountain.pos[1]);
				fscanf(FH, "%f ", &effect->Fountain.pos[2]);
				fscanf(FH, "%f ", &effect->Fountain.angle[0]);
				fscanf(FH, "%f ", &effect->Fountain.angle[1]);
				fscanf(FH, "%f ", &effect->Fountain.angle[2]);
				fscanf(FH, "%f ", &effect->Fountain.movedir[0]);
				fscanf(FH, "%f ", &effect->Fountain.movedir[1]);
				fscanf(FH, "%f ", &effect->Fountain.movedir[2]);
				fscanf(FH, "%d ", &effect->Fountain.color);
				fscanf(FH, "%d\n", &effect->Fountain.cnt);
				break;

			case CE_QUAKE:
				fscanf(FH, "%f ", &effect->Quake.origin[0]);
				fscanf(FH, "%f ", &effect->Quake.origin[1]);
				fscanf(FH, "%f ", &effect->Quake.origin[2]);
				fscanf(FH, "%f\n", &effect->Quake.radius);
				break;

			case CE_WHITE_SMOKE:
			case CE_GREEN_SMOKE:
			case CE_GREY_SMOKE:
			case CE_RED_SMOKE:
			case CE_SLOW_WHITE_SMOKE:
			case CE_TELESMK1:
			case CE_TELESMK2:
			case CE_GHOST:
			case CE_REDCLOUD:
			case CE_ACID_MUZZFL:
			case CE_FLAMESTREAM:
			case CE_FLAMEWALL:
			case CE_FLAMEWALL2:
			case CE_ONFIRE:
				fscanf(FH, "%f ", &effect->Smoke.origin[0]);
				fscanf(FH, "%f ", &effect->Smoke.origin[1]);
				fscanf(FH, "%f ", &effect->Smoke.origin[2]);
				fscanf(FH, "%f ", &effect->Smoke.velocity[0]);
				fscanf(FH, "%f ", &effect->Smoke.velocity[1]);
				fscanf(FH, "%f ", &effect->Smoke.velocity[2]);
				fscanf(FH, "%f ", &effect->Smoke.framelength);
				fscanf(FH, "%f\n", &effect->Smoke.frame);
				break;

			case CE_SM_WHITE_FLASH:
			case CE_YELLOWRED_FLASH:
			case CE_BLUESPARK:
			case CE_YELLOWSPARK:
			case CE_SM_CIRCLE_EXP:
			case CE_BG_CIRCLE_EXP:
			case CE_SM_EXPLOSION:
			case CE_LG_EXPLOSION:
			case CE_FLOOR_EXPLOSION:
			case CE_FLOOR_EXPLOSION3:
			case CE_BLUE_EXPLOSION:
			case CE_REDSPARK:
			case CE_GREENSPARK:
			case CE_ICEHIT:
			case CE_MEDUSA_HIT:
			case CE_MEZZO_REFLECT:
			case CE_FLOOR_EXPLOSION2:
			case CE_XBOW_EXPLOSION:
			case CE_NEW_EXPLOSION:
			case CE_MAGIC_MISSILE_EXPLOSION:
			case CE_BONE_EXPLOSION:
			case CE_BLDRN_EXPL:
			case CE_BRN_BOUNCE:
			case CE_LSHOCK:
			case CE_ACID_HIT:
			case CE_ACID_SPLAT:
			case CE_ACID_EXPL:
			case CE_LBALL_EXPL:
			case CE_FBOOM:
			case CE_FIREWALL_SMALL:
			case CE_FIREWALL_MEDIUM:
			case CE_FIREWALL_LARGE:
			case CE_BOMB:
				fscanf(FH, "%f ", &effect->Smoke.origin[0]);
				fscanf(FH, "%f ", &effect->Smoke.origin[1]);
				fscanf(FH, "%f\n", &effect->Smoke.origin[2]);
				break;

			case CE_WHITE_FLASH:
			case CE_BLUE_FLASH:
			case CE_SM_BLUE_FLASH:
			case CE_RED_FLASH:
				fscanf(FH, "%f ", &effect->Flash.origin[0]);
				fscanf(FH, "%f ", &effect->Flash.origin[1]);
				fscanf(FH, "%f\n", &effect->Flash.origin[2]);
				break;

			case CE_RIDER_DEATH:
				fscanf(FH, "%f ", &effect->RD.origin[0]);
				fscanf(FH, "%f ", &effect->RD.origin[1]);
				fscanf(FH, "%f\n", &effect->RD.origin[2]);
				break;

			case CE_GRAVITYWELL:
				fscanf(FH, "%f ", &effect->RD.origin[0]);
				fscanf(FH, "%f ", &effect->RD.origin[1]);
				fscanf(FH, "%f", &effect->RD.origin[2]);
				fscanf(FH, "%d", &effect->RD.color);
				fscanf(FH, "%f\n", &effect->RD.lifetime);
				break;

			case CE_TELEPORTERPUFFS:
				fscanf(FH, "%f ", &effect->Teleporter.origin[0]);
				fscanf(FH, "%f ", &effect->Teleporter.origin[1]);
				fscanf(FH, "%f\n", &effect->Teleporter.origin[2]);
				break;

			case CE_TELEPORTERBODY:
				fscanf(FH, "%f ", &effect->Teleporter.origin[0]);
				fscanf(FH, "%f ", &effect->Teleporter.origin[1]);
				fscanf(FH, "%f\n", &effect->Teleporter.origin[2]);
				break;

			case CE_BONESHARD:
			case CE_BONESHRAPNEL:
				fscanf(FH, "%f ", &effect->Missile.origin[0]);
				fscanf(FH, "%f ", &effect->Missile.origin[1]);
				fscanf(FH, "%f ", &effect->Missile.origin[2]);
				fscanf(FH, "%f ", &effect->Missile.velocity[0]);
				fscanf(FH, "%f ", &effect->Missile.velocity[1]);
				fscanf(FH, "%f ", &effect->Missile.velocity[2]);
				fscanf(FH, "%f ", &effect->Missile.angle[0]);
				fscanf(FH, "%f ", &effect->Missile.angle[1]);
				fscanf(FH, "%f ", &effect->Missile.angle[2]);
				fscanf(FH, "%f ", &effect->Missile.avelocity[0]);
				fscanf(FH, "%f ", &effect->Missile.avelocity[1]);
				fscanf(FH, "%f ", &effect->Missile.avelocity[2]);
				break;

			case CE_CHUNK:
				fscanf(FH, "%f ", &effect->Chunk.origin[0]);
				fscanf(FH, "%f ", &effect->Chunk.origin[1]);
				fscanf(FH, "%f ", &effect->Chunk.origin[2]);
//				fscanf(FH, "%d ", &effect->Chunk.type);
				fscanf(FH, "%c ", &effect->Chunk.type);
				fscanf(FH, "%f ", &effect->Chunk.srcVel[0]);
				fscanf(FH, "%f ", &effect->Chunk.srcVel[1]);
				fscanf(FH, "%f ", &effect->Chunk.srcVel[2]);
//				fscanf(FH, "%d ", &effect->Chunk.numChunks);
				fscanf(FH, "%c ", &effect->Chunk.numChunks);
				break;

			default:
				PR_RunError ("SV_SaveEffect: bad type");
				break;
		}
	}
}

#ifndef RQM_SV_ONLY

/*
=======================
CL_FreeEffect (Hexen II)
=======================
*/
void CL_FreeEffect(int index)
{
	struct effect_t *effect;
	int i;

	effect = &cl.effects[index];
	switch (effect->type)
	{
		case CE_RAIN:
			break;

		case CE_SNOW:
			break;

		case CE_FOUNTAIN:
			break;

		case CE_QUAKE:
			break;

		case CE_WHITE_SMOKE:
		case CE_GREEN_SMOKE:
		case CE_GREY_SMOKE:
		case CE_RED_SMOKE:
		case CE_SLOW_WHITE_SMOKE:
		case CE_TELESMK1:
		case CE_TELESMK2:
		case CE_GHOST:
		case CE_REDCLOUD:
		case CE_ACID_MUZZFL:
		case CE_FLAMESTREAM:
		case CE_FLAMEWALL:
		case CE_FLAMEWALL2:
		case CE_ONFIRE:
			FreeEffectEntity(effect->Smoke.entity_index);
			break;

		// Just go through animation and then remove
		case CE_SM_WHITE_FLASH:
		case CE_YELLOWRED_FLASH:
		case CE_BLUESPARK:
		case CE_YELLOWSPARK:
		case CE_SM_CIRCLE_EXP:
		case CE_BG_CIRCLE_EXP:
		case CE_SM_EXPLOSION:
		case CE_LG_EXPLOSION:
		case CE_FLOOR_EXPLOSION:
		case CE_FLOOR_EXPLOSION3:
		case CE_BLUE_EXPLOSION:
		case CE_REDSPARK:
		case CE_GREENSPARK:
		case CE_ICEHIT:
		case CE_MEDUSA_HIT:
		case CE_MEZZO_REFLECT:
		case CE_FLOOR_EXPLOSION2:
		case CE_XBOW_EXPLOSION:
		case CE_NEW_EXPLOSION:
		case CE_MAGIC_MISSILE_EXPLOSION:
		case CE_BONE_EXPLOSION:
		case CE_BLDRN_EXPL:
		case CE_BRN_BOUNCE:
		case CE_LSHOCK:
		case CE_ACID_HIT:
		case CE_ACID_SPLAT:
		case CE_ACID_EXPL:
		case CE_LBALL_EXPL:
		case CE_FBOOM:
		case CE_BOMB:
		case CE_FIREWALL_SMALL:
		case CE_FIREWALL_MEDIUM:
		case CE_FIREWALL_LARGE:

			FreeEffectEntity(effect->Smoke.entity_index);
			break;

		// Go forward then backward through animation then remove
		case CE_WHITE_FLASH:
		case CE_BLUE_FLASH:
		case CE_SM_BLUE_FLASH:
		case CE_RED_FLASH:
			FreeEffectEntity(effect->Flash.entity_index);
			break;

		case CE_RIDER_DEATH:
			break;

		case CE_GRAVITYWELL:
			break;

		case CE_TELEPORTERPUFFS:
			for (i=0;i<8;++i)
				FreeEffectEntity(effect->Teleporter.entity_index[i]);
			break;

		case CE_TELEPORTERBODY:
			FreeEffectEntity(effect->Teleporter.entity_index[0]);
			break;

		case CE_BONESHARD:
		case CE_BONESHRAPNEL:
			FreeEffectEntity(effect->Missile.entity_index);
			break;
		case CE_CHUNK:
			//Con_Print("Freeing a chunk here\n");
			for (i=0;i < effect->Chunk.numChunks;i++)
			{
				if(effect->Chunk.entity_index[i] != -1)
				{
					FreeEffectEntity(effect->Chunk.entity_index[i]);
				}
			}
			break;

	}

	memset(effect, 0, sizeof(struct effect_t));
}

/*
=======================
CL_ParseEffect (Hexen II)
=======================
*/
// All changes need to be in SV_SendEffect(), SV_ParseEffect(),
// SV_SaveEffects(), SV_LoadEffects(), CL_ParseEffect()
void CL_ParseEffect(void)
{
	int			index, i;
	struct effect_t *effect;
	qboolean	ImmediateFree;
	entity_t	*ent;
	int			dir;
	float		angleval, sinval, cosval;
	float		skinnum, final;

	ImmediateFree = false;

	index = MSG_ReadByte();
	effect = &cl.effects[index];
	if (effect->type)
		CL_FreeEffect(index);

	memset(effect, 0, sizeof(struct effect_t));

	effect->type = MSG_ReadByte();

	switch (effect->type)
	{
		case CE_RAIN:
			effect->Rain.min_org[0] = MSG_ReadCoord();
			effect->Rain.min_org[1] = MSG_ReadCoord();
			effect->Rain.min_org[2] = MSG_ReadCoord();
			effect->Rain.max_org[0] = MSG_ReadCoord();
			effect->Rain.max_org[1] = MSG_ReadCoord();
			effect->Rain.max_org[2] = MSG_ReadCoord();
			effect->Rain.e_size[0] = MSG_ReadCoord();
			effect->Rain.e_size[1] = MSG_ReadCoord();
			effect->Rain.e_size[2] = MSG_ReadCoord();
			effect->Rain.dir[0] = MSG_ReadCoord();
			effect->Rain.dir[1] = MSG_ReadCoord();
			effect->Rain.dir[2] = MSG_ReadCoord();
			effect->Rain.color = MSG_ReadShort();
			effect->Rain.count = MSG_ReadShort();
			effect->Rain.wait = MSG_ReadFloat();
			break;

		case CE_SNOW:
			effect->Rain.min_org[0] = MSG_ReadCoord();
			effect->Rain.min_org[1] = MSG_ReadCoord();
			effect->Rain.min_org[2] = MSG_ReadCoord();
			effect->Rain.max_org[0] = MSG_ReadCoord();
			effect->Rain.max_org[1] = MSG_ReadCoord();
			effect->Rain.max_org[2] = MSG_ReadCoord();
			effect->Rain.flags = MSG_ReadByte();
			effect->Rain.dir[0] = MSG_ReadCoord();
			effect->Rain.dir[1] = MSG_ReadCoord();
			effect->Rain.dir[2] = MSG_ReadCoord();
			effect->Rain.count = MSG_ReadByte();
			//effect->Rain.veer = MSG_ReadShort();
			break;

		case CE_FOUNTAIN:
			effect->Fountain.pos[0] = MSG_ReadCoord ();
			effect->Fountain.pos[1] = MSG_ReadCoord ();
			effect->Fountain.pos[2] = MSG_ReadCoord ();
			effect->Fountain.angle[0] = MSG_ReadAngle ();
			effect->Fountain.angle[1] = MSG_ReadAngle ();
			effect->Fountain.angle[2] = MSG_ReadAngle ();
			effect->Fountain.movedir[0] = MSG_ReadCoord ();
			effect->Fountain.movedir[1] = MSG_ReadCoord ();
			effect->Fountain.movedir[2] = MSG_ReadCoord ();
			effect->Fountain.color = MSG_ReadShort ();
			effect->Fountain.cnt = MSG_ReadByte ();
			AngleVectors (effect->Fountain.angle, effect->Fountain.vforward,
						  effect->Fountain.vright, effect->Fountain.vup);
			break;

		case CE_QUAKE:
			effect->Quake.origin[0] = MSG_ReadCoord ();
			effect->Quake.origin[1] = MSG_ReadCoord ();
			effect->Quake.origin[2] = MSG_ReadCoord ();
			effect->Quake.radius = MSG_ReadFloat ();
			break;

		case CE_WHITE_SMOKE:
		case CE_GREEN_SMOKE:
		case CE_GREY_SMOKE:
		case CE_RED_SMOKE:
		case CE_SLOW_WHITE_SMOKE:
		case CE_TELESMK1:
		case CE_TELESMK2:
		case CE_GHOST:
		case CE_REDCLOUD:
		case CE_ACID_MUZZFL:
		case CE_FLAMESTREAM:
		case CE_FLAMEWALL:
		case CE_FLAMEWALL2:
		case CE_ONFIRE:
			effect->Smoke.origin[0] = MSG_ReadCoord ();
			effect->Smoke.origin[1] = MSG_ReadCoord ();
			effect->Smoke.origin[2] = MSG_ReadCoord ();

			effect->Smoke.velocity[0] = MSG_ReadFloat ();
			effect->Smoke.velocity[1] = MSG_ReadFloat ();
			effect->Smoke.velocity[2] = MSG_ReadFloat ();

			effect->Smoke.framelength = MSG_ReadFloat ();
			if (cl.protocol == PROTOCOL_VERSION_H2_112)
				effect->Smoke.frame = MSG_ReadFloat ();
			else
				effect->Smoke.frame = 0;
			if ((effect->Smoke.entity_index = NewEffectEntity()) != -1)
			{
				ent = &EffectEntities[effect->Smoke.entity_index];
				VectorCopy(effect->Smoke.origin, ent->origin);

				if ((effect->type == CE_WHITE_SMOKE) ||
					(effect->type == CE_SLOW_WHITE_SMOKE))
					ent->model = Mod_ForName("models/whtsmk1.spr", true);
				else if (effect->type == CE_GREEN_SMOKE)
					ent->model = Mod_ForName("models/grnsmk1.spr", true);
				else if (effect->type == CE_GREY_SMOKE)
					ent->model = Mod_ForName("models/grysmk1.spr", true);
				else if (effect->type == CE_RED_SMOKE)
					ent->model = Mod_ForName("models/redsmk1.spr", true);
				else if (effect->type == CE_TELESMK1)
					ent->model = Mod_ForName("models/telesmk1.spr", true);
				else if (effect->type == CE_TELESMK2)
					ent->model = Mod_ForName("models/telesmk2.spr", true);
				else if (effect->type == CE_REDCLOUD)
					ent->model = Mod_ForName("models/rcloud.spr", true);
				else if (effect->type == CE_FLAMESTREAM)
					ent->model = Mod_ForName("models/flamestr.spr", true);
				else if (effect->type == CE_ACID_MUZZFL)
				{
					ent->model = Mod_ForName("models/muzzle1.spr", true);
					ent->drawflags = DRF_TRANSLUCENT | MLS_ABSLIGHT;
					ent->abslight = 0.2;
				}
				else if (effect->type == CE_FLAMEWALL)
					ent->model = Mod_ForName("models/firewal1.spr", true);
				else if (effect->type == CE_FLAMEWALL2)
					ent->model = Mod_ForName("models/firewal2.spr", true);
				else if (effect->type == CE_ONFIRE)
				{
					float rdm = rand() & 3;

					if (rdm < 1)
						ent->model = Mod_ForName("models/firewal1.spr", true);
					else if (rdm < 2)
						ent->model = Mod_ForName("models/firewal2.spr", true);
					else
						ent->model = Mod_ForName("models/firewal3.spr", true);

					ent->drawflags = DRF_TRANSLUCENT;
					ent->abslight = 1;
					ent->frame = effect->Smoke.frame;
				}

				if (effect->type != CE_REDCLOUD && effect->type != CE_ACID_MUZZFL
					&& effect->type != CE_FLAMEWALL)
					ent->drawflags = DRF_TRANSLUCENT;

				if (effect->type == CE_FLAMESTREAM)
				{
					ent->drawflags = DRF_TRANSLUCENT | MLS_ABSLIGHT;
					ent->abslight = 1;
					ent->frame = effect->Smoke.frame;
				}

				if (effect->type == CE_GHOST)
				{
					ent->model = Mod_ForName("models/ghost.spr", true);
					ent->drawflags = DRF_TRANSLUCENT | MLS_ABSLIGHT;
					ent->abslight = .5;
				}
			}
			else
				ImmediateFree = true;
			break;

		case CE_SM_WHITE_FLASH:
		case CE_YELLOWRED_FLASH:
		case CE_BLUESPARK:
		case CE_YELLOWSPARK:
		case CE_SM_CIRCLE_EXP:
		case CE_BG_CIRCLE_EXP:
		case CE_SM_EXPLOSION:
		case CE_LG_EXPLOSION:
		case CE_FLOOR_EXPLOSION:
		case CE_FLOOR_EXPLOSION3:
		case CE_BLUE_EXPLOSION:
		case CE_REDSPARK:
		case CE_GREENSPARK:
		case CE_ICEHIT:
		case CE_MEDUSA_HIT:
		case CE_MEZZO_REFLECT:
		case CE_FLOOR_EXPLOSION2:
		case CE_XBOW_EXPLOSION:
		case CE_NEW_EXPLOSION:
		case CE_MAGIC_MISSILE_EXPLOSION:
		case CE_BONE_EXPLOSION:
		case CE_BLDRN_EXPL:
		case CE_BRN_BOUNCE:
		case CE_LSHOCK:
		case CE_ACID_HIT:
		case CE_ACID_SPLAT:
		case CE_ACID_EXPL:
		case CE_LBALL_EXPL:
		case CE_FBOOM:
		case CE_BOMB:
		case CE_FIREWALL_SMALL:
		case CE_FIREWALL_MEDIUM:
		case CE_FIREWALL_LARGE:
			effect->Smoke.origin[0] = MSG_ReadCoord ();
			effect->Smoke.origin[1] = MSG_ReadCoord ();
			effect->Smoke.origin[2] = MSG_ReadCoord ();
			if ((effect->Smoke.entity_index = NewEffectEntity()) != -1)
			{
				ent = &EffectEntities[effect->Smoke.entity_index];
				VectorCopy(effect->Smoke.origin, ent->origin);

				if (effect->type == CE_BLUESPARK)
					ent->model = Mod_ForName("models/bspark.spr", true);
				else if (effect->type == CE_YELLOWSPARK)
					ent->model = Mod_ForName("models/spark.spr", true);
				else if (effect->type == CE_SM_CIRCLE_EXP)
					ent->model = Mod_ForName("models/fcircle.spr", true);
				else if (effect->type == CE_BG_CIRCLE_EXP)
					ent->model = Mod_ForName("models/xplod29.spr", true);
				else if (effect->type == CE_SM_WHITE_FLASH)
					ent->model = Mod_ForName("models/sm_white.spr", true);
				else if (effect->type == CE_YELLOWRED_FLASH)
				{
					ent->model = Mod_ForName("models/yr_flsh.spr", true);
					ent->drawflags = DRF_TRANSLUCENT;
				}
				else if (effect->type == CE_SM_EXPLOSION)
					ent->model = Mod_ForName("models/sm_expld.spr", true);
				else if (effect->type == CE_LG_EXPLOSION)
					ent->model = Mod_ForName("models/bg_expld.spr", true);
				else if (effect->type == CE_FLOOR_EXPLOSION)
					ent->model = Mod_ForName("models/fl_expld.spr", true);
				else if (effect->type == CE_FLOOR_EXPLOSION3)
					ent->model = Mod_ForName("models/biggy.spr", true);
				else if (effect->type == CE_BLUE_EXPLOSION)
					ent->model = Mod_ForName("models/xpspblue.spr", true);
				else if (effect->type == CE_REDSPARK)
					ent->model = Mod_ForName("models/rspark.spr", true);
				else if (effect->type == CE_GREENSPARK)
					ent->model = Mod_ForName("models/gspark.spr", true);
				else if (effect->type == CE_ICEHIT)
					ent->model = Mod_ForName("models/icehit.spr", true);
				else if (effect->type == CE_MEDUSA_HIT)
					ent->model = Mod_ForName("models/medhit.spr", true);
				else if (effect->type == CE_MEZZO_REFLECT)
					ent->model = Mod_ForName("models/mezzoref.spr", true);
				else if (effect->type == CE_FLOOR_EXPLOSION2)
					ent->model = Mod_ForName("models/flrexpl2.spr", true);
				else if (effect->type == CE_XBOW_EXPLOSION)
					ent->model = Mod_ForName("models/xbowexpl.spr", true);
				else if (effect->type == CE_NEW_EXPLOSION)
					ent->model = Mod_ForName("models/gen_expl.spr", true);
				else if (effect->type == CE_MAGIC_MISSILE_EXPLOSION)
					ent->model = Mod_ForName("models/mm_expld.spr", true);
				else if (effect->type == CE_BONE_EXPLOSION)
					ent->model = Mod_ForName("models/bonexpld.spr", true);
				else if (effect->type == CE_BLDRN_EXPL)
					ent->model = Mod_ForName("models/xplsn_1.spr", true);
				else if (effect->type == CE_ACID_HIT)
					ent->model = Mod_ForName("models/axplsn_2.spr", true);
				else if (effect->type == CE_ACID_SPLAT)
					ent->model = Mod_ForName("models/axplsn_1.spr", true);
				else if (effect->type == CE_ACID_EXPL)
				{
					ent->model = Mod_ForName("models/axplsn_5.spr", true);
					ent->drawflags = MLS_ABSLIGHT;
					ent->abslight = 1;
				}
				else if (effect->type == CE_FBOOM)
					ent->model = Mod_ForName("models/fboom.spr", true);
				else if (effect->type == CE_BOMB)
					ent->model = Mod_ForName("models/pow.spr", true);
				else if (effect->type == CE_LBALL_EXPL)
					ent->model = Mod_ForName("models/Bluexp3.spr", true);
				else if (effect->type == CE_FIREWALL_SMALL)
					ent->model = Mod_ForName("models/firewal1.spr", true);
				else if (effect->type == CE_FIREWALL_MEDIUM)
					ent->model = Mod_ForName("models/firewal5.spr", true);
				else if (effect->type == CE_FIREWALL_LARGE)
					ent->model = Mod_ForName("models/firewal4.spr", true);
				else if (effect->type == CE_BRN_BOUNCE)
					ent->model = Mod_ForName("models/spark.spr", true);
				else if (effect->type == CE_LSHOCK)
				{
					ent->model = Mod_ForName("models/vorpshok.mdl", true);
					ent->drawflags = MLS_TORCH;
					ent->angles[2] = 90;
					ent->scale = 255;
				}
			}
			else
			{
				ImmediateFree = true;
			}
			break;

		case CE_WHITE_FLASH:
		case CE_BLUE_FLASH:
		case CE_SM_BLUE_FLASH:
		case CE_RED_FLASH:
			effect->Flash.origin[0] = MSG_ReadCoord ();
			effect->Flash.origin[1] = MSG_ReadCoord ();
			effect->Flash.origin[2] = MSG_ReadCoord ();
			effect->Flash.reverse = 0;
			if ((effect->Flash.entity_index = NewEffectEntity()) != -1)
			{
				ent = &EffectEntities[effect->Flash.entity_index];
				VectorCopy(effect->Flash.origin, ent->origin);

				if (effect->type == CE_WHITE_FLASH)
					ent->model = Mod_ForName("models/gryspt.spr", true);
				else if (effect->type == CE_BLUE_FLASH)
					ent->model = Mod_ForName("models/bluflash.spr", true);
				else if (effect->type == CE_SM_BLUE_FLASH)
					ent->model = Mod_ForName("models/sm_blue.spr", true);
				else if (effect->type == CE_RED_FLASH)
					ent->model = Mod_ForName("models/redspt.spr", true);

				ent->drawflags = DRF_TRANSLUCENT;

			}
			else
			{
				ImmediateFree = true;
			}
			break;

		case CE_RIDER_DEATH:
			effect->RD.origin[0] = MSG_ReadCoord ();
			effect->RD.origin[1] = MSG_ReadCoord ();
			effect->RD.origin[2] = MSG_ReadCoord ();
			break;

		case CE_GRAVITYWELL:
			effect->RD.origin[0] = MSG_ReadCoord ();
			effect->RD.origin[1] = MSG_ReadCoord ();
			effect->RD.origin[2] = MSG_ReadCoord ();
			effect->RD.color = MSG_ReadShort ();
			effect->RD.lifetime = MSG_ReadFloat ();
			break;

		case CE_TELEPORTERPUFFS:
			effect->Teleporter.origin[0] = MSG_ReadCoord ();
			effect->Teleporter.origin[1] = MSG_ReadCoord ();
			effect->Teleporter.origin[2] = MSG_ReadCoord ();

			effect->Teleporter.framelength = .05;
			dir = 0;
			for (i=0;i<8;++i)
			{
				if ((effect->Teleporter.entity_index[i] = NewEffectEntity()) != -1)
				{
					ent = &EffectEntities[effect->Teleporter.entity_index[i]];
					VectorCopy(effect->Teleporter.origin, ent->origin);

					angleval = dir * M_PI*2 / 360;

					sinval = sin(angleval);
					cosval = cos(angleval);

					effect->Teleporter.velocity[i][0] = 10*cosval;
					effect->Teleporter.velocity[i][1] = 10*sinval;
					effect->Teleporter.velocity[i][2] = 0;
					dir += 45;

					ent->model = Mod_ForName("models/telesmk2.spr", true);
					ent->drawflags = DRF_TRANSLUCENT;
				}
			}
			break;

		case CE_TELEPORTERBODY:
			effect->Teleporter.origin[0] = MSG_ReadCoord ();
			effect->Teleporter.origin[1] = MSG_ReadCoord ();
			effect->Teleporter.origin[2] = MSG_ReadCoord ();

			effect->Teleporter.velocity[0][0] = MSG_ReadFloat ();
			effect->Teleporter.velocity[0][1] = MSG_ReadFloat ();
			effect->Teleporter.velocity[0][2] = MSG_ReadFloat ();

			skinnum = MSG_ReadFloat ();

			effect->Teleporter.framelength = .05;
			dir = 0;
			if ((effect->Teleporter.entity_index[0] = NewEffectEntity()) != -1)
			{
				ent = &EffectEntities[effect->Teleporter.entity_index[0]];
				VectorCopy(effect->Teleporter.origin, ent->origin);

				ent->model = Mod_ForName("models/teleport.mdl", true);
				ent->drawflags = SCALE_TYPE_XYONLY | DRF_TRANSLUCENT;
				ent->scale = 100;
				ent->skinnum = skinnum;
			}
			break;

		case CE_BONESHARD:
		case CE_BONESHRAPNEL:
			effect->Missile.origin[0] = MSG_ReadCoord ();
			effect->Missile.origin[1] = MSG_ReadCoord ();
			effect->Missile.origin[2] = MSG_ReadCoord ();

			effect->Missile.velocity[0] = MSG_ReadFloat ();
			effect->Missile.velocity[1] = MSG_ReadFloat ();
			effect->Missile.velocity[2] = MSG_ReadFloat ();

			effect->Missile.angle[0] = MSG_ReadFloat ();
			effect->Missile.angle[1] = MSG_ReadFloat ();
			effect->Missile.angle[2] = MSG_ReadFloat ();

			effect->Missile.avelocity[0] = MSG_ReadFloat ();
			effect->Missile.avelocity[1] = MSG_ReadFloat ();
			effect->Missile.avelocity[2] = MSG_ReadFloat ();

			if ((effect->Missile.entity_index = NewEffectEntity()) != -1)
			{
				ent = &EffectEntities[effect->Missile.entity_index];
				VectorCopy(effect->Missile.origin, ent->origin);
				if (effect->type == CE_BONESHARD)
					ent->model = Mod_ForName("models/boneshot.mdl", true);
				else if (effect->type == CE_BONESHRAPNEL)
					ent->model = Mod_ForName("models/boneshrd.mdl", true);
			}
			else
				ImmediateFree = true;
			break;

		case CE_CHUNK:
			effect->Chunk.origin[0] = MSG_ReadCoord ();
			effect->Chunk.origin[1] = MSG_ReadCoord ();
			effect->Chunk.origin[2] = MSG_ReadCoord ();

			effect->Chunk.type = MSG_ReadByte ();

			effect->Chunk.srcVel[0] = MSG_ReadCoord ();
			effect->Chunk.srcVel[1] = MSG_ReadCoord ();
			effect->Chunk.srcVel[2] = MSG_ReadCoord ();

			effect->Chunk.numChunks = MSG_ReadByte ();

			effect->Chunk.time_amount = 4.0;

			effect->Chunk.aveScale = 30 + 100 * (effect->Chunk.numChunks / 40.0);

			if(effect->Chunk.numChunks > 16)effect->Chunk.numChunks = 16;

			for (i=0;i < effect->Chunk.numChunks;i++)
			{
				if ((effect->Chunk.entity_index[i] = NewEffectEntity()) != -1)
				{
					ent = &EffectEntities[effect->Chunk.entity_index[i]];
					VectorCopy(effect->Chunk.origin, ent->origin);

					VectorCopy(effect->Chunk.srcVel, effect->Chunk.velocity[i]);
					VectorScale(effect->Chunk.velocity[i], .80 + ((rand()%4)/10.0), effect->Chunk.velocity[i]);
					// temp modify them...
					effect->Chunk.velocity[i][0] += (rand()%140)-70;
					effect->Chunk.velocity[i][1] += (rand()%140)-70;
					effect->Chunk.velocity[i][2] += (rand()%140)-70;

					// are these in degrees or radians?
					ent->angles[0] = rand()%360;
					ent->angles[1] = rand()%360;
					ent->angles[2] = rand()%360;

					ent->scale = effect->Chunk.aveScale + rand()%40;

					// make this overcomplicated
					final = (rand()%100)*.01;
					if ((effect->Chunk.type==THINGTYPE_GLASS) || (effect->Chunk.type==THINGTYPE_REDGLASS) ||
							(effect->Chunk.type==THINGTYPE_CLEARGLASS) || (effect->Chunk.type==THINGTYPE_WEBS))
					{
						if (final<0.20)
							ent->model = Mod_ForName ("models/shard1.mdl", true);
						else if (final<0.40)
							ent->model = Mod_ForName ("models/shard2.mdl", true);
						else if (final<0.60)
							ent->model = Mod_ForName ("models/shard3.mdl", true);
						else if (final<0.80)
							ent->model = Mod_ForName ("models/shard4.mdl", true);
						else
							ent->model = Mod_ForName ("models/shard5.mdl", true);

						if (effect->Chunk.type==THINGTYPE_CLEARGLASS)
						{
							ent->skinnum=1;
							ent->drawflags |= DRF_TRANSLUCENT;
						}
						else if (effect->Chunk.type==THINGTYPE_REDGLASS)
						{
							ent->skinnum=2;
						}
						else if (effect->Chunk.type==THINGTYPE_WEBS)
						{
							ent->skinnum=3;
							ent->drawflags |= DRF_TRANSLUCENT;
						}
					}
					else if (effect->Chunk.type==THINGTYPE_WOOD)
					{
						if (final < 0.25)
							ent->model = Mod_ForName ("models/splnter1.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/splnter2.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/splnter3.mdl", true);
						else
							ent->model = Mod_ForName ("models/splnter4.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_METAL)
					{
						if (final < 0.25)
							ent->model = Mod_ForName ("models/metlchk1.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/metlchk2.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/metlchk3.mdl", true);
						else
							ent->model = Mod_ForName ("models/metlchk4.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_FLESH)
					{
						if (final < 0.33)
							ent->model = Mod_ForName ("models/flesh1.mdl", true);
						else if (final < 0.66)
							ent->model = Mod_ForName ("models/flesh2.mdl", true);
						else
							ent->model = Mod_ForName ("models/flesh3.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_BROWNSTONE)
					{
						if (final < 0.25)
							ent->model = Mod_ForName ("models/schunk1.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/schunk2.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/schunk3.mdl", true);
						else
							ent->model = Mod_ForName ("models/schunk4.mdl", true);
						ent->skinnum = 1;
					}
					else if ((effect->Chunk.type==THINGTYPE_CLAY) || (effect->Chunk.type==THINGTYPE_BONE))
					{
						if (final < 0.25)
							ent->model = Mod_ForName ("models/clshard1.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/clshard2.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/clshard3.mdl", true);
						else
							ent->model = Mod_ForName ("models/clshard4.mdl", true);
						if (effect->Chunk.type==THINGTYPE_BONE)
						{
							ent->skinnum=1;//bone skin is second
						}
					}
					else if (effect->Chunk.type==THINGTYPE_LEAVES)
					{
						if (final < 0.33)
							ent->model = Mod_ForName ("models/leafchk1.mdl", true);
						else if (final < 0.66)
							ent->model = Mod_ForName ("models/leafchk2.mdl", true);
						else
							ent->model = Mod_ForName ("models/leafchk3.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_HAY)
					{
						if (final < 0.33)
							ent->model = Mod_ForName ("models/hay1.mdl", true);
						else if (final < 0.66)
							ent->model = Mod_ForName ("models/hay2.mdl", true);
						else
							ent->model = Mod_ForName ("models/hay3.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_CLOTH)
					{
						if (final < 0.33)
							ent->model = Mod_ForName ("models/clthchk1.mdl", true);
						else if (final < 0.66)
							ent->model = Mod_ForName ("models/clthchk2.mdl", true);
						else
							ent->model = Mod_ForName ("models/clthchk3.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_WOOD_LEAF)
					{
						if (final < 0.14)
							ent->model = Mod_ForName ("models/splnter1.mdl", true);
						else if (final < 0.28)
							ent->model = Mod_ForName ("models/leafchk1.mdl", true);
						else if (final < 0.42)
							ent->model = Mod_ForName ("models/splnter2.mdl", true);
						else if (final < 0.56)
							ent->model = Mod_ForName ("models/leafchk2.mdl", true);
						else if (final < 0.70)
							ent->model = Mod_ForName ("models/splnter3.mdl", true);
						else if (final < 0.84)
							ent->model = Mod_ForName ("models/leafchk3.mdl", true);
						else
							ent->model = Mod_ForName ("models/splnter4.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_WOOD_METAL)
					{
						if (final < 0.125)
							ent->model = Mod_ForName ("models/splnter1.mdl", true);
						else if (final < 0.25)
							ent->model = Mod_ForName ("models/metlchk1.mdl", true);
						else if (final < 0.375)
							ent->model = Mod_ForName ("models/splnter2.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/metlchk2.mdl", true);
						else if (final < 0.625)
							ent->model = Mod_ForName ("models/splnter3.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/metlchk3.mdl", true);
						else if (final < 0.875)
							ent->model = Mod_ForName ("models/splnter4.mdl", true);
						else
							ent->model = Mod_ForName ("models/metlchk4.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_WOOD_STONE)
					{
						if (final < 0.125)
							ent->model = Mod_ForName ("models/splnter1.mdl", true);
						else if (final < 0.25)
							ent->model = Mod_ForName ("models/schunk1.mdl", true);
						else if (final < 0.375)
							ent->model = Mod_ForName ("models/splnter2.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/schunk2.mdl", true);
						else if (final < 0.625)
							ent->model = Mod_ForName ("models/splnter3.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/schunk3.mdl", true);
						else if (final < 0.875)
							ent->model = Mod_ForName ("models/splnter4.mdl", true);
						else
							ent->model = Mod_ForName ("models/schunk4.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_METAL_STONE)
					{
						if (final < 0.125)
							ent->model = Mod_ForName ("models/metlchk1.mdl", true);
						else if (final < 0.25)
							ent->model = Mod_ForName ("models/schunk1.mdl", true);
						else if (final < 0.375)
							ent->model = Mod_ForName ("models/metlchk2.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/schunk2.mdl", true);
						else if (final < 0.625)
							ent->model = Mod_ForName ("models/metlchk3.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/schunk3.mdl", true);
						else if (final < 0.875)
							ent->model = Mod_ForName ("models/metlchk4.mdl", true);
						else
							ent->model = Mod_ForName ("models/schunk4.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_METAL_CLOTH)
					{
						if (final < 0.14)
							ent->model = Mod_ForName ("models/metlchk1.mdl", true);
						else if (final < 0.28)
							ent->model = Mod_ForName ("models/clthchk1.mdl", true);
						else if (final < 0.42)
							ent->model = Mod_ForName ("models/metlchk2.mdl", true);
						else if (final < 0.56)
							ent->model = Mod_ForName ("models/clthchk2.mdl", true);
						else if (final < 0.70)
							ent->model = Mod_ForName ("models/metlchk3.mdl", true);
						else if (final < 0.84)
							ent->model = Mod_ForName ("models/clthchk3.mdl", true);
						else
							ent->model = Mod_ForName ("models/metlchk4.mdl", true);
					}
					else if (effect->Chunk.type==THINGTYPE_ICE)
					{
						ent->model = Mod_ForName("models/shard.mdl", true);
						ent->skinnum=0;
						ent->frame = rand()%2;
						ent->drawflags |= DRF_TRANSLUCENT|MLS_ABSLIGHT;
						ent->abslight = 0.5;
					}
					else if (effect->Chunk.type==THINGTYPE_METEOR)
					{
						ent->model = Mod_ForName("models/tempmetr.mdl", true);
						ent->skinnum = 0;
					}
					else if (effect->Chunk.type==THINGTYPE_ACID)
					{	// no spinning if possible...
						ent->model = Mod_ForName("models/sucwp2p.mdl", true);
						ent->skinnum = 0;
					}
					else if (effect->Chunk.type==THINGTYPE_GREENFLESH)
					{	// spider guts
						if (final < 0.33)
							ent->model = Mod_ForName ("models/sflesh1.mdl", true);
						else if (final < 0.66)
							ent->model = Mod_ForName ("models/sflesh2.mdl", true);
						else
							ent->model = Mod_ForName ("models/sflesh3.mdl", true);

						ent->skinnum = 0;
					}
					else// if (effect->Chunk.type==THINGTYPE_GREYSTONE)
					{
						if (final < 0.25)
							ent->model = Mod_ForName ("models/schunk1.mdl", true);
						else if (final < 0.50)
							ent->model = Mod_ForName ("models/schunk2.mdl", true);
						else if (final < 0.75)
							ent->model = Mod_ForName ("models/schunk3.mdl", true);
						else
							ent->model = Mod_ForName ("models/schunk4.mdl", true);
						ent->skinnum = 0;
					}
				}
			}
			for(i=0; i < 3; i++)
			{
				effect->Chunk.avel[i][0] = rand()%850 - 425;
				effect->Chunk.avel[i][1] = rand()%850 - 425;
				effect->Chunk.avel[i][2] = rand()%850 - 425;
			}

			break;

		default:
			Host_Error ("CL_ParseEffect: bad type");		// JDH: was Sys_Error
	}

	if (ImmediateFree)
	{
		effect->type = CE_NONE;
	}
}

/*
=======================
CL_EndEffect (Hexen II)
=======================
*/
void CL_EndEffect(void)
{
	int index;

	index = MSG_ReadByte();

	CL_FreeEffect(index);
}

void CL_LinkEntity(entity_t *ent)
{
	if (cl_numvisedicts < MAX_VISEDICTS)
	{
		cl_visedicts[cl_numvisedicts++] = ent;
	}
}

void CL_UpdateEffects(void)
{
	int			index, cur_frame, i;
	struct effect_t *effect;
	vec3_t		mymin, mymax;
	float		frametime;
//	edict_t test;
//	trace_t	trace;
	vec3_t		org, org2, alldir, snow_org;
	int			x_dir, y_dir;
	entity_t	*ent;
	float		distance, smoketime;

	if (cls.state == ca_disconnected)
		return;

	frametime = cl.time - cl.oldtime;
	if (!frametime) return;
//	Con_Printf("Here at %f\n",cl.time);

	for (index = 0; index < MAX_EFFECTS; index++)
	{
		effect = &cl.effects[index];
		if (!effect->type)
			continue;

		switch (effect->type)
		{
			case CE_RAIN:
				org[0] = effect->Rain.min_org[0];
				org[1] = effect->Rain.min_org[1];
				org[2] = effect->Rain.max_org[2];

				org2[0] = effect->Rain.e_size[0];
				org2[1] = effect->Rain.e_size[1];
				org2[2] = effect->Rain.e_size[2];

				x_dir = effect->Rain.dir[0];
				y_dir = effect->Rain.dir[1];

				effect->Rain.next_time += frametime;
				if (effect->Rain.next_time >= effect->Rain.wait)
				{
					R_RainEffect(org, org2, x_dir, y_dir, effect->Rain.color, effect->Rain.count);
					effect->Rain.next_time = 0;
				}
				break;

			case CE_SNOW:
				VectorCopy(effect->Rain.min_org,org);
				VectorCopy(effect->Rain.max_org,org2);
				VectorCopy(effect->Rain.dir,alldir);

				VectorAdd(org, org2, snow_org);

				snow_org[0] *= 0.5;
				snow_org[1] *= 0.5;
				snow_org[2] *= 0.5;

				snow_org[2] = r_origin[2];

				VectorSubtract(snow_org, r_origin, snow_org);

				distance = VectorNormalize(snow_org);

				effect->Rain.next_time += frametime;
				//jfm:  fixme, check distance to player first
				if (effect->Rain.next_time >= 0.10 && distance < 1024)
				{
					R_SnowEffect(org, org2, effect->Rain.flags, alldir, effect->Rain.count);
					effect->Rain.next_time = 0;
				}
				break;

			case CE_FOUNTAIN:
				mymin[0] = (-3 * effect->Fountain.vright[0] * effect->Fountain.movedir[0]) +
						   (-3 * effect->Fountain.vforward[0] * effect->Fountain.movedir[1]) +
						   (2 * effect->Fountain.vup[0] * effect->Fountain.movedir[2]);
				mymin[1] = (-3 * effect->Fountain.vright[1] * effect->Fountain.movedir[0]) +
						   (-3 * effect->Fountain.vforward[1] * effect->Fountain.movedir[1]) +
						   (2 * effect->Fountain.vup[1] * effect->Fountain.movedir[2]);
				mymin[2] = (-3 * effect->Fountain.vright[2] * effect->Fountain.movedir[0]) +
						   (-3 * effect->Fountain.vforward[2] * effect->Fountain.movedir[1]) +
						   (2 * effect->Fountain.vup[2] * effect->Fountain.movedir[2]);
				mymin[0] *= 15;
				mymin[1] *= 15;
				mymin[2] *= 15;

				mymax[0] = (3 * effect->Fountain.vright[0] * effect->Fountain.movedir[0]) +
						   (3 * effect->Fountain.vforward[0] * effect->Fountain.movedir[1]) +
						   (10 * effect->Fountain.vup[0] * effect->Fountain.movedir[2]);
				mymax[1] = (3 * effect->Fountain.vright[1] * effect->Fountain.movedir[0]) +
						   (3 * effect->Fountain.vforward[1] * effect->Fountain.movedir[1]) +
						   (10 * effect->Fountain.vup[1] * effect->Fountain.movedir[2]);
				mymax[2] = (3 * effect->Fountain.vright[2] * effect->Fountain.movedir[0]) +
						   (3 * effect->Fountain.vforward[2] * effect->Fountain.movedir[1]) +
						   (10 * effect->Fountain.vup[2] * effect->Fountain.movedir[2]);
				mymax[0] *= 15;
				mymax[1] *= 15;
				mymax[2] *= 15;

				R_RunParticleEffect2 (effect->Fountain.pos, mymin, mymax,
					                  effect->Fountain.color, pt_fastgrav, effect->Fountain.cnt);

/*				memset(&test,0,sizeof(test));
				trace = SV_Move (effect->Fountain.pos, mymin, mymax, mymin, false, &test);
				Con_Printf("Fraction is %f\n",trace.fraction);*/
				break;

			case CE_QUAKE:
				R_RunQuakeEffect (effect->Quake.origin,effect->Quake.radius);
				break;

			case CE_WHITE_SMOKE:
			case CE_GREEN_SMOKE:
			case CE_GREY_SMOKE:
			case CE_RED_SMOKE:
			case CE_SLOW_WHITE_SMOKE:
			case CE_TELESMK1:
			case CE_TELESMK2:
			case CE_GHOST:
			case CE_REDCLOUD:
			case CE_FLAMESTREAM:
			case CE_ACID_MUZZFL:
			case CE_FLAMEWALL:
			case CE_FLAMEWALL2:
			case CE_ONFIRE:
				effect->Smoke.time_amount += frametime;
				ent = &EffectEntities[effect->Smoke.entity_index];

				smoketime = effect->Smoke.framelength;
				if (!smoketime)
					smoketime = HX_FRAME_TIME;

				ent->origin[0] += (frametime/smoketime) * effect->Smoke.velocity[0];
				ent->origin[1] += (frametime/smoketime) * effect->Smoke.velocity[1];
				ent->origin[2] += (frametime/smoketime) * effect->Smoke.velocity[2];

				while(effect->Smoke.time_amount >= smoketime)
				{
					ent->frame++;
					effect->Smoke.time_amount -= smoketime;
				}

				if (ent->frame >= ent->model->numframes)
				{
					CL_FreeEffect(index);
				}
				else
					CL_LinkEntity(ent);

				break;

			// Just go through animation and then remove
			case CE_SM_WHITE_FLASH:
			case CE_YELLOWRED_FLASH:
			case CE_BLUESPARK:
			case CE_YELLOWSPARK:
			case CE_SM_CIRCLE_EXP:
			case CE_BG_CIRCLE_EXP:
			case CE_SM_EXPLOSION:
			case CE_LG_EXPLOSION:
			case CE_FLOOR_EXPLOSION:
			case CE_FLOOR_EXPLOSION3:
			case CE_BLUE_EXPLOSION:
			case CE_REDSPARK:
			case CE_GREENSPARK:
			case CE_ICEHIT:
			case CE_MEDUSA_HIT:
			case CE_MEZZO_REFLECT:
			case CE_FLOOR_EXPLOSION2:
			case CE_XBOW_EXPLOSION:
			case CE_NEW_EXPLOSION:
			case CE_MAGIC_MISSILE_EXPLOSION:
			case CE_BONE_EXPLOSION:
			case CE_BLDRN_EXPL:
			case CE_BRN_BOUNCE:
			case CE_ACID_HIT:
			case CE_ACID_SPLAT:
			case CE_ACID_EXPL:
			case CE_LBALL_EXPL:
			case CE_FBOOM:
			case CE_BOMB:
			case CE_FIREWALL_SMALL:
			case CE_FIREWALL_MEDIUM:
			case CE_FIREWALL_LARGE:

				effect->Smoke.time_amount += frametime;
				ent = &EffectEntities[effect->Smoke.entity_index];

				if (effect->type != CE_BG_CIRCLE_EXP)
				{
					while (effect->Smoke.time_amount >= HX_FRAME_TIME)
					{
						ent->frame++;
						effect->Smoke.time_amount -= HX_FRAME_TIME;
					}
				}
				else
				{
					while (effect->Smoke.time_amount >= HX_FRAME_TIME * 2)
					{
						ent->frame++;
						effect->Smoke.time_amount -= HX_FRAME_TIME * 2;
					}
				}
				if (ent->frame >= ent->model->numframes)
				{
					CL_FreeEffect(index);
				}
				else
					CL_LinkEntity(ent);
				break;


			case CE_LSHOCK:
				ent = &EffectEntities[effect->Smoke.entity_index];
				if(ent->skinnum==0)
					ent->skinnum=1;
				else if(ent->skinnum==1)
					ent->skinnum=0;
				ent->scale-=10;
				if (ent->scale<=10)
				{
					CL_FreeEffect(index);
				}
				else
					CL_LinkEntity(ent);
				break;

			// Go forward then backward through animation then remove
			case CE_WHITE_FLASH:
			case CE_BLUE_FLASH:
			case CE_SM_BLUE_FLASH:
			case CE_RED_FLASH:
				effect->Flash.time_amount += frametime;
				ent = &EffectEntities[effect->Flash.entity_index];

				while(effect->Flash.time_amount >= HX_FRAME_TIME)
				{
					if (!effect->Flash.reverse)
					{
						if (ent->frame >= ent->model->numframes-1)  // Ran through forward animation
						{
							effect->Flash.reverse = 1;
							ent->frame--;
						}
						else
							ent->frame++;

					}
					else
						ent->frame--;

					effect->Flash.time_amount -= HX_FRAME_TIME;
				}

				if ((ent->frame <= 0) && (effect->Flash.reverse))
				{
					CL_FreeEffect(index);
				}
				else
					CL_LinkEntity(ent);
				break;

			case CE_RIDER_DEATH:
				effect->RD.time_amount += frametime;
				if (effect->RD.time_amount >= 1)
				{
					effect->RD.stage++;
					effect->RD.time_amount -= 1;
				}

				VectorCopy(effect->RD.origin,org);
				org[0] += sin(effect->RD.time_amount * 2 * M_PI) * 30;
				org[1] += cos(effect->RD.time_amount * 2 * M_PI) * 30;

				if (effect->RD.stage <= 6)
//					R_RiderParticle(effect->RD.stage+1,effect->RD.origin);
					R_RiderParticle(effect->RD.stage+1,org);
				else
				{
					// To set the rider's origin point for the particles
					R_RiderParticle(0,org);
					if (effect->RD.stage == 7)
					{
						cl.cshifts[CSHIFT_BONUS].destcolor[0] = 255;
						cl.cshifts[CSHIFT_BONUS].destcolor[1] = 255;
						cl.cshifts[CSHIFT_BONUS].destcolor[2] = 255;
						cl.cshifts[CSHIFT_BONUS].percent = 256;
					}
					else if (effect->RD.stage > 13)
					{
//						effect->RD.stage = 0;
						CL_FreeEffect(index);
					}
				}
				break;

			case CE_GRAVITYWELL:

				effect->RD.time_amount += frametime*2;
				if (effect->RD.time_amount >= 1)
					effect->RD.time_amount -= 1;

				VectorCopy(effect->RD.origin,org);
				org[0] += sin(effect->RD.time_amount * 2 * M_PI) * 30;
				org[1] += cos(effect->RD.time_amount * 2 * M_PI) * 30;

				if (effect->RD.lifetime < cl.time)
				{
					CL_FreeEffect(index);
				}
				else
					R_GravityWellParticle(rand()%8,org, effect->RD.color);

				break;

			case CE_TELEPORTERPUFFS:
				effect->Teleporter.time_amount += frametime;
				smoketime = effect->Teleporter.framelength;

				ent = &EffectEntities[effect->Teleporter.entity_index[0]];
				while(effect->Teleporter.time_amount >= HX_FRAME_TIME)
				{
					ent->frame++;
					effect->Teleporter.time_amount -= HX_FRAME_TIME;
				}
				cur_frame = ent->frame;

				if (cur_frame >= ent->model->numframes)
				{
					CL_FreeEffect(index);
					break;
				}

				for (i=0;i<8;++i)
				{
					ent = &EffectEntities[effect->Teleporter.entity_index[i]];

					ent->origin[0] += (frametime/smoketime) * effect->Teleporter.velocity[i][0];
					ent->origin[1] += (frametime/smoketime) * effect->Teleporter.velocity[i][1];
					ent->origin[2] += (frametime/smoketime) * effect->Teleporter.velocity[i][2];
					ent->frame = cur_frame;

					CL_LinkEntity(ent);
				}
				break;

			case CE_TELEPORTERBODY:
				effect->Teleporter.time_amount += frametime;
				smoketime = effect->Teleporter.framelength;

				ent = &EffectEntities[effect->Teleporter.entity_index[0]];
				while(effect->Teleporter.time_amount >= HX_FRAME_TIME)
				{
					ent->scale -= 15;
					effect->Teleporter.time_amount -= HX_FRAME_TIME;
				}

				ent = &EffectEntities[effect->Teleporter.entity_index[0]];
				ent->angles[1] += 45;

				if (ent->scale <= 10)
				{
					CL_FreeEffect(index);
				}
				else
				{
					CL_LinkEntity(ent);
				}
				break;

			case CE_BONESHARD:
			case CE_BONESHRAPNEL:
				effect->Missile.time_amount += frametime;
				ent = &EffectEntities[effect->Missile.entity_index];

//		ent->angles[0] = effect->Missile.angle[0];
//		ent->angles[1] = effect->Missile.angle[1];
//		ent->angles[2] = effect->Missile.angle[2];

				ent->angles[0] += frametime * effect->Missile.avelocity[0];
				ent->angles[1] += frametime * effect->Missile.avelocity[1];
				ent->angles[2] += frametime * effect->Missile.avelocity[2];

				ent->origin[0] += frametime * effect->Missile.velocity[0];
				ent->origin[1] += frametime * effect->Missile.velocity[1];
				ent->origin[2] += frametime * effect->Missile.velocity[2];

				CL_LinkEntity(ent);
				break;

			case CE_CHUNK:
				effect->Chunk.time_amount -= frametime;
				if(effect->Chunk.time_amount < 0)
				{
					CL_FreeEffect(index);
				}
				else
				{
					for (i=0;i < effect->Chunk.numChunks;i++)
					{
						vec3_t oldorg;
						mleaf_t		*l;
						int			moving = 1;

						ent = &EffectEntities[effect->Chunk.entity_index[i]];

						VectorCopy(ent->origin, oldorg);

						ent->origin[0] += frametime * effect->Chunk.velocity[i][0];
						ent->origin[1] += frametime * effect->Chunk.velocity[i][1];
						ent->origin[2] += frametime * effect->Chunk.velocity[i][2];

						l = Mod_PointInLeaf (ent->origin, cl.worldmodel);
						if(l->contents!=CONTENTS_EMPTY) //||in_solid==true
						{	// bouncing prolly won't work...
							VectorCopy(oldorg, ent->origin);

							effect->Chunk.velocity[i][0] = 0;
							effect->Chunk.velocity[i][1] = 0;
							effect->Chunk.velocity[i][2] = 0;

							moving = 0;
						}
						else
						{
							ent->angles[0] += frametime * effect->Chunk.avel[i%3][0];
							ent->angles[1] += frametime * effect->Chunk.avel[i%3][1];
							ent->angles[2] += frametime * effect->Chunk.avel[i%3][2];
						}

						if(effect->Chunk.time_amount < frametime * 3)
						{	// chunk leaves in 3 frames
							ent->scale *= .7;
						}

						CL_LinkEntity(ent);

						effect->Chunk.velocity[i][2] -= frametime * 500; // apply gravity

						switch (effect->Chunk.type)
						{
						case THINGTYPE_FLESH:
							if (moving) R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, BLOODSHOT_TRAIL);
							break;
						case THINGTYPE_ICE:
							if (moving) R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ICE_TRAIL);
							break;
						case THINGTYPE_ACID:
							if (moving) R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ACID_TRAIL);
							break;
						case THINGTYPE_METEOR:
							R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, GRENADE_TRAIL);
							break;
						case THINGTYPE_GREENFLESH:
							if (moving) R_RocketTrail (oldorg, ent->origin, &ent->trail_origin, ACID_TRAIL);
							break;

						}
					}
				}
				break;
		}
	}
}

//==========================================================================
//
// NewEffectEntity
//
//==========================================================================

static int NewEffectEntity(void)
{
	entity_t	*ent;
	int counter;

	if(cl_numvisedicts == MAX_VISEDICTS)
	{
		return -1;
	}
	if(EffectEntityCount == MAX_EFFECT_ENTITIES)
	{
		return -1;
	}

	for(counter=0;counter<MAX_EFFECT_ENTITIES;counter++)
		if (!EntityUsed[counter])
			break;

	EntityUsed[counter] = true;
	EffectEntityCount++;
	ent = &EffectEntities[counter];
	memset(ent, 0, sizeof(*ent));
	ent->colormap = vid.colormap;

	return counter;
}

static void FreeEffectEntity(int index)
{
	EntityUsed[index] = false;
	EffectEntityCount--;
}

#endif		// #ifdef HEXEN2_SUPPORT
#endif		//#ifndef RQM_SV_ONLY
