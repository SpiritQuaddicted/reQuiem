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
// gl_drawsprite.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

/*
================
R_GetSpriteFrame
================
*/
mspriteframe_t *R_GetSpriteFrame (entity_t *currententity, msprite_t *psprite)
{
	mspritegroup_t	*pspritegroup;
	mspriteframe_t	*pspriteframe;
	int				i, numframes, frame;
	float			*pintervals, fullinterval, targettime, time;

	frame = currententity->frame;

	if ((frame >= psprite->numframes) || (frame < 0))
	{
		Con_Printf ("R_DrawSprite: no such frame %d\n", frame);
		frame = 0;
	}

	if (psprite->frames[frame].type == SPR_SINGLE)
	{
		pspriteframe = psprite->frames[frame].frameptr;
	}
	else
	{
		pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
		pintervals = pspritegroup->intervals;
		numframes = pspritegroup->numframes;
		fullinterval = pintervals[numframes-1];

		time = cl.time + currententity->syncbase;

	// when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
	// are positive, so we don't have to worry about division by 0
		targettime = time - ((int)(time / fullinterval)) * fullinterval;

		for (i=0 ; i<(numframes-1) ; i++)
		{
			if (pintervals[i] > targettime)
				break;
		}

		pspriteframe = pspritegroup->frames[i];
	}

	return pspriteframe;
}

/*
=================
R_SetSpritesState
=================
*/
// joe: from FuhQuake
/*******JDH*******/
//void R_SetSpritesState (qboolean state)
void R_SetSpritesState (entity_t *currententity, mspriteframe_t *frame, qboolean state)
/*******JDH*******/
{
/*	static qboolean	r_state = false;

	if (r_state == state)
		return;

	r_state = state;*/

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		if (state)
		{
		//	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_BLEND);

			if (currententity->drawflags & DRF_TRANSLUCENT)
			{
				glColor4f (1, 1, 1, r_wateralpha.value);
			}
			else glColor3f (1, 1, 1);
		}
		else
		{
			glDisable (GL_BLEND);
		}

		return;
	}
#endif

	if (state)
	{
		GL_DisableMultitexture ();
		
		if (currententity->effects & EF_ADDITIVE)		// JDH: Marcher's hi-res sprites
		{
			glBlendFunc (GL_SRC_ALPHA, GL_ONE);
			glEnable (GL_BLEND);
			glDepthMask (GL_FALSE);		// disable zbuffer updates
		}
		else if (frame->is_tex_external || (currententity->model->modhint == MOD_SPR32))
		{
		//	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable (GL_BLEND);
			glDepthMask (GL_FALSE);
		}
		else
		{
			glEnable (GL_ALPHA_TEST);
		}
	}
	else
	{
		if (currententity->effects & EF_ADDITIVE)
		{
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable (GL_BLEND);
			glDepthMask (GL_TRUE);	// enable zbuffer updates
		}
		else if (frame->is_tex_external || (currententity->model->modhint == MOD_SPR32))
		{
			glDisable (GL_BLEND);
			glDepthMask (GL_TRUE);
		}
		else
		{
			glDisable (GL_ALPHA_TEST);
		}
	}
}

/*
=================
R_DrawSpriteModel
=================
*/
void R_DrawSpriteModel (entity_t *e, msprite_t *psprite, mspriteframe_t *frame)
{
	vec3_t			point, right, up;

	// don't even bother culling, because it's just a single
	// polygon without a surface cache

	if (psprite->type == SPR_ORIENTED)
	{
		// bullet marks on walls
	/******JDH*******/
		//AngleVectors (currententity->angles, NULL, right, up);
		AngleVectors (e->angles, NULL, right, up);
	/******JDH*******/
	}
	else if (psprite->type == SPR_FACING_UPRIGHT)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			vec3_t tvec;
			
			VectorSubtract (e->origin, r_origin, tvec);
			VectorNormalize (tvec);		// tvec[2] is same as DotProduct (tvec, up) 
										//  because up is 0, 0, 1
			if ((tvec[2] > 0.999848) || (tvec[2] < -0.999848))	// cos(1 degree) = 0.999848
				return;
			
			right[0] = tvec[1];
			right[1] = -tvec[0];
		}
		else
	#endif
		{
			right[0] = e->origin[1] - r_origin[1];
			right[1] = -(e->origin[0] - r_origin[0]);
		}
		
		right[2] = 0;
		VectorNormalize (right);
		VectorSet (up, 0, 0, 1);
	}
	else if (psprite->type == SPR_VP_PARALLEL_UPRIGHT)
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			// vpn[2] is same as DotProduct (vpn, up) because up is 0, 0, 1

			if ((vpn[2] > 0.999848) || (vpn[2] < -0.999848))	// cos(1 degree) = 0.999848
				return;

			// CrossProduct (up, vpn, right)
			right[0] = vpn[1];								
			right[1] = -vpn[0];
			right[2] = 0;
			VectorNormalize (right);
		}
		else
	#endif
		VectorCopy (vright, right);
		
		VectorSet (up, 0, 0, 1);
	}
#ifdef HEXEN2_SUPPORT
	else if (hexen2 && (psprite->type == SPR_VP_PARALLEL_ORIENTED))
	{
	// generate the sprite's axes, parallel to the viewplane, but rotated in
	// that plane around the center according to the sprite entity's roll
	// angle. So vpn stays the same, but vright and vup rotate
		float	angle, sr, cr;
		int i;
		
		angle = e->angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		for (i=0 ; i<3 ; i++)
		{
			right[i] = vright[i] * cr + vup[i] * sr;
			up[i] = vright[i] * -sr + vup[i] * cr;
		}
	}
#endif
	else
	{	// normal sprite
		VectorCopy (vup, up);
		VectorCopy (vright, right);
	}

	GL_Bind (frame->gl_texturenum);

	glBegin (GL_QUADS);
/*
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
#endif
*/
	glTexCoord2f (0, 1);
	VectorMA (e->origin, frame->down, up, point);
	VectorMA (point, frame->left, right, point);
	glVertex3fv (point);

	glTexCoord2f (0, 0);
	VectorMA (e->origin, frame->up, up, point);
	VectorMA (point, frame->left, right, point);
	glVertex3fv (point);

	glTexCoord2f (1, 0);
	VectorMA (e->origin, frame->up, up, point);
	VectorMA (point, frame->right, right, point);
	glVertex3fv (point);

	glTexCoord2f (1, 1);
	VectorMA (e->origin, frame->down, up, point);
	VectorMA (point, frame->right, right, point);
	glVertex3fv (point);
	
	glEnd ();
/*
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
#endif
*/
}

/******JDH******/

/*
=================
R_DrawSprite
=================
*/
void R_DrawSprite (entity_t *currententity)
{
	msprite_t		*psprite;
	mspriteframe_t	*frame;
	
	psprite = currententity->model->cache.data;
	frame = R_GetSpriteFrame (currententity, psprite);

	R_SetSpritesState (currententity, frame, true);
	R_DrawSpriteModel (currententity, psprite, frame);
	R_SetSpritesState (currententity, frame, false);
}

/*
=================
R_DrawSprites
=================
*/
void R_DrawSprites (void)
{
	int			i;
	entity_t	*currententity;
	
	/*for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = cl_visedicts[i];
		
		switch (currententity->model->type)
		{
		case mod_sprite:
			R_SetSpritesState (true);
			R_DrawSpriteModel (currententity);
			break;

		default:
			break;
		}
	}
	R_SetSpritesState (false);*/

	
	for (i = 0; i < cl_numvisedicts; i++)
	{
		currententity = cl_visedicts[i];
		if (currententity->model->type == mod_sprite)
		{
			R_DrawSprite (currententity);
		}
	}
	
//	R_SetSpritesState (currententity, false);
}
/******JDH******/

#endif		//#ifndef RQM_SV_ONLY

