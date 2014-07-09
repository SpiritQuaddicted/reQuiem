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
// r_part.c

#include "quakedef.h"

#define DEFAULT_NUM_PARTICLES	2048
#define ABSOLUTE_MIN_PARTICLES	512
#define ABSOLUTE_MAX_PARTICLES	8192

static	int	ramp1[8] = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
static	int	ramp2[8] = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
int	ramp3[8] = {0x6d, 0x6b, 6, 5, 4, 3};

particle_t	*particles, *active_particles, *free_particles;
static	int		r_numparticles;

vec3_t			r_pright, r_pup, r_ppn;

static void Classic_LoadParticleTexture (void)
{
	int		i, x, y;
	unsigned int	data[32][32];

#ifdef HEXEN2_SUPPORT
//	if (hexen2)
		Hexen2_LoadParticleTexture ();
#endif

	particletexture = texture_extension_number++;
	GL_Bind (particletexture);

	// clear to transparent white
	for (i=0 ; i<32*32 ; i++)
		((unsigned *)data)[i] = 0x00FFFFFF;

	// draw a circle in the top left corner
	for (x=0 ; x<16 ; x++)
	{
		for (y=0 ; y<16 ; y++)
		{
			if ((x - 7.5) * (x - 7.5) + (y - 7.5) * (y - 7.5) <= 8 * 8)
				data[y][x] = 0xFFFFFFFF;	// solid white
		}
	}

//	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	x = y = 32;
	GL_Upload32 (NULL, (unsigned *)data, &x, &y, TEX_MIPMAP | TEX_ALPHA);
}

/*
===============
R_InitParticles
===============
*/
void Classic_InitParticles (void)
{
	int	i;

	if ((i = COM_CheckParm ("-particles")) && i+1 < com_argc)
	{
		r_numparticles = (int)(Q_atoi(com_argv[i+1]));
		r_numparticles = bound(ABSOLUTE_MIN_PARTICLES, r_numparticles, ABSOLUTE_MAX_PARTICLES);
	}
	else
	{
		r_numparticles = DEFAULT_NUM_PARTICLES;
	}

	particles = (particle_t *)Hunk_AllocName (r_numparticles * sizeof(particle_t), "classic:particles");

//#ifdef GLQUAKE
	Classic_LoadParticleTexture ();
//#endif
}

/*
===============
AllocParticle
===============
*/
particle_t *AllocParticle (float lifetime)
{
	particle_t *p;

	if (!free_particles)
		return NULL;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

// JDH: I set starttime to cl.time-lifetime so that when rewinding a demo,
//   particle effect lasts the appropriate time (because particle isn't
//	 spawned until the _beginning_ of the time period)

	p->starttime = cl.time - lifetime;
	p->endtime = cl.time + lifetime;
	return p;
}

/*
===============
R_EntityParticles
===============
*/

#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];
vec3_t		avelocities[NUMVERTEXNORMALS];
float		beamlength = 16;

void R_EntityParticles (entity_t *ent)
{
	int		i, count;
	particle_t	*p;
	float		angle, dist, sr, sp, sy, cr, cp, cy;
	vec3_t		forward;

	dist = 64;
	count = 50;

	if (!avelocities[0][0])
		for (i=0 ; i<NUMVERTEXNORMALS ; i++) {
			avelocities[i][0] = (rand() & 255) * 0.01;
			avelocities[i][1] = (rand() & 255) * 0.01;
			avelocities[i][2] = (rand() & 255) * 0.01;
		}

	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		angle = cl.time * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = cl.time * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = cl.time * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);

		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		p = AllocParticle (0.01);
		if (!p)
			return;
		/*if (!free_particles)
			return;
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;*/

		p->color = 0x6f;
		p->type = pt_explode;

		p->org[0] = ent->origin[0] + r_avertexnormals[i][0]*dist + forward[0]*beamlength;
		p->org[1] = ent->origin[1] + r_avertexnormals[i][1]*dist + forward[1]*beamlength;
		p->org[2] = ent->origin[2] + r_avertexnormals[i][2]*dist + forward[2]*beamlength;
	}
}

/*
===============
R_ClearParticles
===============
*/
void Classic_ClearParticles (void)
{
	int	i;

	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ; i<r_numparticles ; i++)
		particles[i].next = &particles[i+1];
	particles[r_numparticles-1].next = NULL;
}


void R_ReadPointFile_f (cmd_source_t src)
{
	FILE		*f;
	vec3_t		org;
	int		r, c;
	particle_t	*p;
	char		name[MAX_OSPATH];

	Q_snprintfz (name, sizeof(name), "maps/%s.pts", sv.name);

	f = COM_FOpenFile (name, 0, NULL);
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}

	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for ( ; ; )
	{
		r = fscanf (f,"%f %f %f\n", &org[0], &org[1], &org[2]);
		if (r != 3)
			break;
		c++;

		p = AllocParticle (0);
		if (!p)
		{
			Con_Print ("Not enough free particles\n");
			break;
		}
		/*if (!free_particles)
		{
			Con_Print ("Not enough free particles\n");
			break;
		}
		p = free_particles;
		free_particles = p->next;
		p->next = active_particles;
		active_particles = p;*/

		p->starttime = 0;
		p->endtime = 99999;
		p->color = (-c)&15;
		p->type = pt_static;
		VectorCopy (vec3_origin, p->vel);
		VectorCopy (org, p->org);
	}

	fclose (f);
	Con_Printf ("%i points read\n", c);
}

/*
===============
R_ParseParticleEffect

Parse an effect out of the server message
===============
*/
void R_ParseParticleEffect (qboolean parse_only)
{
	vec3_t		org, dir;
	int		i, count, color;

	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	for (i=0 ; i<3 ; i++)
		dir[i] = MSG_ReadChar () * (1.0 / 16);
	count = MSG_ReadByte ();
	color = MSG_ReadByte ();

	if (parse_only)
		return;

// HACK: unfortunately the effect of the exploding barrel is quite poor
// in the original progs - no light, *sigh* - so I added some extras - joe
	if (count == 255)
	{
		dlight_t	*dl;

		// joe: nehahra has improved features for barrels
		if (nehahra)
			return;

		if (r_explosiontype.value == 3)
			R_RunParticleEffect (org, dir, 225, 50);
		else
			R_ParticleExplosion (org);

		// the missing light
		if (r_explosionlight.value)
		{
			dl = CL_AllocDlight (0);
			VectorCopy (org, dl->origin);
			dl->radius = 150 + 200 * bound (0, r_explosionlight.value, 1);
			dl->starttime = cl.time;
			dl->endtime = cl.time + 0.5;
			dl->decay = 300;
//#ifdef GLQUAKE
//			dl->type = SetDlightColor (r_explosionlightcolor.value, lt_explosion, true);
			SetDlightColor (dl, (int) r_explosionlightcolor.value, lt_explosion, true);
//#endif
		}
	}
	else
	{
		R_RunParticleEffect (org, dir, color, count);
	}
}

/*
===============
R_ParticleExplosion
===============
*/
void Classic_ParticleExplosion (vec3_t org)
{
	int		i, j;
	particle_t	*p;

	if (r_explosiontype.value == 1)	// just a sprite
		return;

	for (i=0 ; i<1024 ; i++)
	{
		p = AllocParticle(5);
		if (!p) return;

		p->color = ramp1[0];
		p->ramp = rand() & 3;
		p->type = (i & 1) ? pt_explode : pt_explode2;

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() % 32) - 16);
			p->vel[j] = (rand() % 512) - 256;
		}
	}
}

/*
===============
R_ColorMappedExplosion		// joe: previously R_ParticleExplosion2
===============
*/
void Classic_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength)
{
	int		i, j;
	particle_t	*p;
	int		colorMod = 0;

	if (r_explosiontype.value == 1)
		return;

	for (i=0 ; i<512 ; i++)
	{
		p = AllocParticle (0.3);
		if (!p) return;

		p->color = colorStart + (colorMod % colorLength);
		colorMod++;
		p->type = pt_blob;

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() % 32) - 16);
			p->vel[j] = (rand() % 512) - 256;
		}
	}
}

/*
===============
R_BlobExplosion
===============
*/
void Classic_BlobExplosion (vec3_t org)
{
	int		i, j;
	particle_t	*p;

	for (i=0 ; i<1024 ; i++)
	{
		p = AllocParticle (1 + (rand() & 8) * 0.05);
		if (!p) return;

		if (i & 1)
		{
			p->type = pt_blob;
			p->color = 66 + rand() % 6;
		}
		else
		{
			p->type = pt_blob2;
			p->color = 150 + rand() % 6;
		}

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() % 32) - 16);
			p->vel[j] = (rand() % 512) - 256;
		}
	}
}

/*
===============
R_RunParticleEffect
===============
*/
void Classic_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, j;
	particle_t	*p;

	for (i=0 ; i<count ; i++)
	{
		p = AllocParticle (0.1 * (rand() % 5));
		if (!p) 
			return;

		p->color = (color & ~7) + (rand() & 7);
		p->type = pt_grav;

		for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand() & 15) - 8);
			p->vel[j] = dir[j] * 15;
		}
	}
}

/*
===============
R_SparkShower
  LordHavoc: added this for spawning sparks/dust (which have strong gravity)
===============
*/

/*
void R_SparkShower (vec3_t org, vec3_t dir, int count, int type)
{
	int			i, j;
	particle_t	*p;

	p = AllocParticle (1);
	if (!p)
		return;

	if (type == 0) // sparks
	{
                p->texnum = smoke_tex;
		p->scale = 20;
		p->alpha = 255;
		p->color = (rand()&3)+12;
		p->type = pt_bulletpuff;
		p->die = cl.time + 1;
		VectorCopy(org, p->org);
		p->vel[0] = p->vel[1] = p->vel[2] = 0;
	}
	else // blood
	{
                p->texnum = blood_tex;
		p->scale = 24;
		p->alpha = 255;
		p->color = (rand()&3)+12;
		p->type = pt_bloodcloud;
		p->die = cl.time + 0.5;
		VectorCopy(org, p->org);
		p->vel[0] = p->vel[1] = p->vel[2] = 0;
		return;
	}
	for (i=0 ; i<count ; i++)
	{
		p = NextFreeParticle ();
		if (!p)
			return;

                p->texnum = flare_tex;
		p->scale = 5;
		p->alpha = 255;
		p->die = cl.time + 0.0625 * (rand()&15);
                p->type = pt_dust;
                p->ramp = (rand()&3);
                p->color = ramp1[(int)p->ramp];
                for (j=0 ; j<3 ; j++)
		{
			p->org[j] = org[j] + ((rand()&7)-4);
			p->vel[j] = dir[j] + (rand()%192)-96;
                }
        }
}
*/
/*
===============
R_LavaSplash
===============
*/
void Classic_LavaSplash (vec3_t org)
{
	int		i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i++)
	{
		for (j=-16 ; j<16 ; j++)
		{
			for (k=0 ; k<1 ; k++)
			{
				p = AllocParticle (2 + (rand() & 31) * 0.02);
				if (!p) return;

				p->color = 224 + (rand() & 7);
				p->type = pt_grav;

				dir[0] = j * 8 + (rand() & 7);
				dir[1] = i * 8 + (rand() & 7);
				dir[2] = 256;

				p->org[0] = org[0] + dir[0];
				p->org[1] = org[1] + dir[1];
				p->org[2] = org[2] + (rand() & 63);

				VectorNormalize (dir);
				vel = 50 + (rand() & 63);
				VectorScale (dir, vel, p->vel);
			}
		}
	}
}

/*
===============
R_TeleportSplash
===============
*/
void Classic_TeleportSplash (vec3_t org)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;

	for (i=-16 ; i<16 ; i+=4)
	{
		for (j=-16 ; j<16 ; j+=4)
		{
			for (k=-24 ; k<32 ; k+=4)
			{
				p = AllocParticle (0.2 + (rand() & 7) * 0.02);
				if (!p) return;

				p->color = 7 + (rand() & 7);
				p->type = pt_grav;

				dir[0] = j * 8;
				dir[1] = i * 8;
				dir[2] = k * 8;

				p->org[0] = org[0] + i + (rand() & 3);
				p->org[1] = org[1] + j + (rand() & 3);
				p->org[2] = org[2] + k + (rand() & 3);

				VectorNormalize (dir);
				vel = 50 + (rand() & 63);
				VectorScale (dir, vel, p->vel);
			}
		}
	}
}

/*
===============
R_RocketTrail
===============
*/
void Classic_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type)
{
	vec3_t		point, delta, dir;
	float		len;
	int		i, j, num_particles;
	particle_t	*p;
	static	int	tracercount;

	VectorCopy (start, point);
	VectorSubtract (end, start, delta);
	if (!(len = VectorLength(delta)))
		goto done;
	VectorScale (delta, 1 / len, dir);	//unit vector in direction of trail

	switch (type)
	{
	case BLOOD_TRAIL:
		len /= 6;
		break;

	default:
		len /= 3;
		break;
	}

	if (!(num_particles = (int)len))
		goto done;

	VectorScale (delta, 1.0 / num_particles, delta);

	for (i=0 ; i < num_particles; i++)
	{
		p = AllocParticle (2);
		if (!p) break;

		VectorClear (p->vel);

		switch (type)
		{
		case GRENADE_TRAIL:
			p->ramp = (r_grenadetrail.value == 2) ? (rand() & 3) : (rand() & 3) + 2;
			p->color = ramp3[(int)p->ramp];
			p->type = pt_fire;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;

		case BLOOD_TRAIL:
			p->type = pt_grav;
			p->color = 67 + (rand() & 3);
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;

		case SLIGHT_BLOOD_TRAIL:
			p->type = pt_grav;
			p->color = 67 + (rand() & 3);
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;

		case TRACER1_TRAIL:
		case TRACER2_TRAIL:
			p->endtime = cl.time + 0.5;
			p->starttime = cl.time - 0.5;
			p->type = pt_static;
			p->color = (type == TRACER1_TRAIL) ? 52 + ((tracercount & 4) << 1) : 230 + ((tracercount & 4) << 1);
			tracercount++;

			VectorCopy (start, p->org);
			if (tracercount & 1)
			{
				p->vel[0] = 90 * dir[1];
				p->vel[1] = 90 * -dir[0];
			}
			else
			{
				p->vel[0] = 90 * -dir[1];
				p->vel[1] = 90 * dir[0];
			}
			break;

		case VOOR_TRAIL:
			p->color = 9*16 + 8 + (rand() & 3);
			p->type = pt_static;
			p->endtime = cl.time + 0.3;
			p->starttime = cl.time - 0.3;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() & 15) - 8);
			break;

		case NEHAHRA_SMOKE:
			// nehahra smoke tracer
			p->color = 12 + (rand() & 3);
			p->type = pt_fire;
			p->endtime = cl.time + 1;
			p->starttime = cl.time - 1;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() & 3) - 2);
			break;

		case ROCKET_TRAIL:
		case LAVA_TRAIL:
		default:
			p->ramp = (r_rockettrail.value == 2) ? (rand() & 3) + 2 : (rand() & 3);
			p->color = ramp3[(int)p->ramp];
			p->type = pt_fire;
			for (j=0 ; j<3 ; j++)
				p->org[j] = start[j] + ((rand() % 6) - 3);
			break;
		}

		VectorAdd (point, delta, point);
	}

done:
	VectorCopy (point, *trail_origin);
}

/*
===============
Classic_DrawParticles (originally R_DrawParticles)
===============
*/
extern	cvar_t	sv_gravity;

void Classic_DrawParticles (void)
{
	particle_t		*p, *kill;
	int				i, j;
	float			grav, time1, time2, time3, dvel, frametime;
	unsigned char	*at, theAlpha;
	vec3_t			up, right;
	float			dist, scale, r_partscale;

	if (!active_particles)
		return;

	r_partscale = 0.004 * tan(r_refdef.fov_x * (M_PI / 180) * 0.5f);

	GL_Bind (particletexture);

	glEnable (GL_BLEND);
	if (!gl_solidparticles.value)
		glDepthMask (GL_FALSE);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBegin (GL_TRIANGLES);

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);

	frametime = fabs(cl.ctime - cl.oldtime);
	if (cl.paused)		// joe: pace from FuhQuake
		frametime = 0;
	time3 = frametime * 15;
	time2 = frametime * 10; // 15;
	time1 = frametime * 5;
	grav = frametime * sv_gravity.value * 0.05;
	dvel = 4 * frametime;

	for ( ; ; )
	{
		kill = active_particles;
		if (kill && PARTICLE_INACTIVE(kill))
		{
			active_particles = kill->next;
			kill->next = free_particles;
			free_particles = kill;
			continue;
		}
		break;
	}

	for (p = active_particles, j=0 ; p ; p = p->next)
	{
		for ( ; ; )
		{
			kill = p->next;
			if (kill && PARTICLE_INACTIVE(kill))
			{
				p->next = kill->next;
				kill->next = free_particles;
				free_particles = kill;
				continue;
			}
			break;
		}

		// JDH (from BJP): Improve sound when many particles
		if (++j % 8192 == 0)
			S_ExtraUpdateTime ();

		// hack a scale up to keep particles from disapearing
		dist = (p->org[0] - r_origin[0])*vpn[0] + (p->org[1] - r_origin[1])*vpn[1] + (p->org[2] - r_origin[2])*vpn[2];
		scale = 1 + dist * r_partscale;

		at = (byte *)&d_8to24table[(int)p->color];
		theAlpha = (p->type == pt_fire) ? 255 * (6 - p->ramp) / 6 : 255;
		glColor4ub (at[0], at[1], at[2], theAlpha);
		glTexCoord2f (0, 0);
		glVertex3fv (p->org);
		glTexCoord2f (1, 0);
		glVertex3f (p->org[0] + up[0]*scale, p->org[1] + up[1]*scale, p->org[2] + up[2]*scale);
		glTexCoord2f (0, 1);
		glVertex3f (p->org[0] + right[0]*scale, p->org[1] + right[1]*scale, p->org[2] + right[2]*scale);
		p->org[0] += p->vel[0] * frametime;
		p->org[1] += p->vel[1] * frametime;
		p->org[2] += p->vel[2] * frametime;

		switch (p->type)
		{
		case pt_static:
			break;

		case pt_fire:
			p->ramp += time1;
			if (p->ramp >= 6)
				p->endtime = -1;
			else
				p->color = ramp3[(int)p->ramp];
			p->vel[2] += grav;
			break;

		case pt_explode:
			p->ramp += time2;
			if (p->ramp >= 8)
				p->endtime = -1;
			else
				p->color = ramp1[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav * 30;
			break;

		case pt_explode2:
			p->ramp += time3;
			if (p->ramp >= 8)
				p->endtime = -1;
			else
				p->color = ramp2[(int)p->ramp];
			for (i=0 ; i<3 ; i++)
				p->vel[i] -= p->vel[i]*frametime;
			p->vel[2] -= grav * 30;
			break;

		case pt_blob:
			for (i=0 ; i<3 ; i++)
				p->vel[i] += p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_blob2:
			for (i=0 ; i<2 ; i++)
				p->vel[i] -= p->vel[i]*dvel;
			p->vel[2] -= grav;
			break;

		case pt_grav:
		case pt_slowgrav:
			p->vel[2] -= grav;
			break;

#ifndef _WIN32
        default:
            break;      // shut up compiler warnings
#endif
		}
	}

	glEnd ();
	glDisable (GL_BLEND);
	glDepthMask (GL_TRUE);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor3f (1, 1, 1);
}

void R_InitParticles (void)
{
	Classic_InitParticles ();
	QMB_InitParticles ();
}

void R_ClearParticles (void)
{
	Classic_ClearParticles ();
	QMB_ClearParticles ();
}

void R_DrawParticles (void)
{
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		Hexen2_DrawParticles();
		return;
	}
#endif

	Classic_DrawParticles ();

	if (qmb_initialized)		// JDH: added check
		QMB_DrawParticles ();
}

void R_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength)
{
	if (qmb_initialized && gl_part_explosions.value)
		QMB_ColorMappedExplosion (org, colorStart, colorLength);
	else
		Classic_ColorMappedExplosion (org, colorStart, colorLength);
}

#define RunParticleEffect(var, org, dir, color, count)			\
	if (qmb_initialized && gl_part_##var.value)			\
		QMB_RunParticleEffect (org, dir, color, count);		\
	else								\
		Classic_RunParticleEffect (org, dir, color, count);

void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	if (color == 73 || color == 225)
	{
		RunParticleEffect(blood, org, dir, color, count);
		return;
	}

	switch (count)
	{
	case 10:
	case 20:
	case 30:
		RunParticleEffect(spikes, org, dir, color, count);
		break;
	default:
		RunParticleEffect(gunshots, org, dir, color, count);
	}
}

void R_RocketTrail (vec3_t start, vec3_t end, vec3_t *trail_origin, trail_type_t type)
{
#ifdef HEXEN2_SUPPORT
	if (hexen2)
		Hexen2_RocketTrail (start, end, type);
	else
#endif
	if (qmb_initialized && gl_part_trails.value)
		QMB_RocketTrail (start, end, trail_origin, type);
	else
		Classic_RocketTrail (start, end, trail_origin, type);
}

#define ParticleFunction(var, name)			\
void R_##name (vec3_t org) {				\
	if (qmb_initialized && gl_part_##var.value)	\
		QMB_##name (org);			\
	else						\
		Classic_##name (org);			\
}


ParticleFunction(explosions, ParticleExplosion);
ParticleFunction(blobs, BlobExplosion);
ParticleFunction(lavasplash, LavaSplash);
ParticleFunction(telesplash, TeleportSplash);

