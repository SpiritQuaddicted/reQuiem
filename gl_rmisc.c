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
// gl_rmisc.c

#include "quakedef.h"

#ifndef RQM_SV_ONLY

int	player_skins[MAX_SCOREBOARD];		// JDH
int	player_skins_fb[MAX_SCOREBOARD];	// by joe


/*
void R_InitOtherTextures (void)
{
	int	flags = TEX_MIPMAP | TEX_ALPHA;

	underwatertexture = GL_LoadTextureImage ("textures/water_caustic", NULL, 0, 0,  flags | (gl_waterfog.value ? TEX_COMPLAIN : 0));
	detailtexture = GL_LoadTextureImage ("textures/detail", NULL, 256, 256, flags | (gl_detail.value ? TEX_COMPLAIN : 0));
	chrometexture2 = GL_LoadTextureImage ("textures/shiny1", 0, 0, false, true);		// JT030105 - reflections
}
*/

void fractalnoise(byte *noise, int size, int startgrid)
{
   int x, y, g, g2, amplitude, min, max, size1 = size - 1, sizepower, gridpower;
   int *noisebuf;
#define n(x,y) noisebuf[((y)&size1)*size+((x)&size1)]

   for (sizepower = 0;(1 << sizepower) < size;sizepower++);
   if (size != (1 << sizepower))
      Sys_Error("fractalnoise: size must be power of 2\n");

   for (gridpower = 0;(1 << gridpower) < startgrid;gridpower++);
   if (startgrid != (1 << gridpower))
      Sys_Error("fractalnoise: grid must be power of 2\n");

   startgrid = bound(0, startgrid, size);

   amplitude = 0xFFFF; // this gets halved before use
   noisebuf = malloc(size*size*sizeof(int));
   memset(noisebuf, 0, size*size*sizeof(int));

   for (g2 = startgrid;g2;g2 >>= 1)
   {
      // brownian motion (at every smaller level there is random behavior)
      amplitude >>= 1;
      for (y = 0;y < size;y += g2)
         for (x = 0;x < size;x += g2)
            n(x,y) += (rand()&amplitude);

      g = g2 >> 1;
      if (g)
      {
         // subdivide, diamond-square algorithm (really this has little to do with squares)
         // diamond
         for (y = 0;y < size;y += g2)
            for (x = 0;x < size;x += g2)
               n(x+g,y+g) = (n(x,y) + n(x+g2,y) + n(x,y+g2) + n(x+g2,y+g2)) >> 2;
         // square
         for (y = 0;y < size;y += g2)
            for (x = 0;x < size;x += g2)
            {
               n(x+g,y) = (n(x,y) + n(x+g2,y) + n(x+g,y-g) + n(x+g,y+g)) >> 2;
               n(x,y+g) = (n(x,y) + n(x,y+g2) + n(x-g,y+g) + n(x+g,y+g)) >> 2;
            }
      }
   }
   // find range of noise values
   min = max = 0;
   for (y = 0;y < size;y++)
      for (x = 0;x < size;x++)
      {
         if (n(x,y) < min) min = n(x,y);
         if (n(x,y) > max) max = n(x,y);
      }
   max -= min;
   max++;
   // normalize noise and copy to output
   for (y = 0;y < size;y++)
      for (x = 0;x < size;x++)
         *noise++ = (byte) (((n(x,y) - min) * 256) / max);
   free(noisebuf);
#undef n
}


/*
===============
R_BuildDetailTexture
===============
*/
void R_BuildDetailTexture (void)
{
	int x, y, light;
	float vc[3], vx[3], vy[3], vn[3], lightdir[3];

	#define DETAILRESOLUTION 256

	byte data[DETAILRESOLUTION][DETAILRESOLUTION][4], noise[DETAILRESOLUTION][DETAILRESOLUTION];
//	byte data2[DETAILRESOLUTION][DETAILRESOLUTION][4];
//	int i;
	lightdir[0] = 0.5;
	lightdir[1] = 1;
	lightdir[2] = -0.25;
	VectorNormalize(lightdir);

	fractalnoise (&noise[0][0], DETAILRESOLUTION, DETAILRESOLUTION >> 4);
	for (y = 0; y < DETAILRESOLUTION; y++)
	{
		for (x = 0; x < DETAILRESOLUTION; x++)
		{
			vc[0] = x;
			vc[1] = y;
			vc[2] = noise[y][x] * (1.0f / 32.0f);
			vx[0] = x + 1;
			vx[1] = y;
			vx[2] = noise[y][(x + 1) % DETAILRESOLUTION] * (1.0f / 32.0f);
			vy[0] = x;
			vy[1] = y + 1;
			vy[2] = noise[(y + 1) % DETAILRESOLUTION][x] * (1.0f / 32.0f);
			VectorSubtract (vx, vc, vx);
			VectorSubtract (vy, vc, vy);
			CrossProduct (vx, vy, vn);
			VectorNormalize(vn);
			light = 128 - DotProduct(vn, lightdir) * 128;
			/****** JDH ******/
			//light = bound(0, light, 255);
			light = bound(0, light-23, 255);
			/****** JDH ******/
			data[y][x][0] = data[y][x][1] = data[y][x][2] = light;
			data[y][x][3] = 255;
		}
	}

#if 0
	for (i = 0; i < NUM_DETAIL_LEVELS; i++)
	{
		//if (i > 0)
		{
			for (y = 0; y < DETAILRESOLUTION; y++)
			{
				for (x = 0; x < DETAILRESOLUTION; x++)
				{
					light = data[y][x][0];
					light -= ((1.0*i)/(float)NUM_DETAIL_LEVELS)*light;
					data2[y][x][0] = data2[y][x][1] = data2[y][x][2] = light;
					
					data2[y][x][3] = 255 - (256/NUM_DETAIL_LEVELS)*i;
				}
			}
		}
	
		detailtexture[i] = texture_extension_number++;

		x = y = DETAILRESOLUTION;
		GL_Bind (detailtexture[i]);
		GL_Upload32 (NULL, (unsigned int *)data2, &x, &y, TEX_MIPMAP /*| TEX_ALPHA*/);
	}
#else
	detailtexture[0] = texture_extension_number++;

	x = y = DETAILRESOLUTION;
	GL_Bind (detailtexture[0]);
	GL_Upload32 (NULL, (unsigned int *)data, &x, &y, TEX_MIPMAP);
#endif
}

void R_InitOtherTextures (void)
{
   int   flags;

/****JDH****/
	if (isDedicated)
		return;

	if (!no24bit)
/****JDH****/
	{
		flags = TEX_MIPMAP | TEX_ALPHA;

#ifdef SHINYWATER
		chrometexture2 = GL_LoadTextureImage ("textures/", "shiny1", NULL, flags, 0);		// JT030105 - reflections
#endif		
		if (gl_waterfog.value)
			flags |= TEX_COMPLAIN;
		underwatertexture = GL_LoadTextureImage ("textures/", "water_caustic", NULL, flags, 0);
	}

	R_BuildDetailTexture ();
} 


/*
==================
R_InitTextures
==================
*/
void R_InitTextures (void)
{
	int	x, y, m;
	byte	*dest;

// create a simple checkerboard texture for the default
	r_notexture_mip = Hunk_AllocName (sizeof(texture_t) + 16*16 + 8*8 + 4*4 + 2*2, "notexture");

	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;

	for (m=0 ; m<4 ; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];
		for (y=0 ; y<(16>>m) ; y++)
			for (x=0 ; x<(16>>m) ; x++)
				*dest++ = ((y < (8 >> m)) ^ (x < (8 >> m))) ? 0 : 0x0e;
	}

#ifdef HEXEN2_SUPPORT
	for (x = 0; x < MAX_EXTRA_TEXTURES; x++)
		gl_extra_textures[x].texnum = -1;
#endif
}

/*
===============
R_CreatePlayerTexture

Translates a skin texture by the per-player color lookup
===============
*/
void R_CreatePlayerTexture (const byte *original, int inwidth, int inheight, const unsigned *translate32, 
									int scaled_width, int scaled_height, unsigned *pixels_out)
{
//JDH	static unsigned	pixels[512*256];
	unsigned		*out, frac, fracstep, pad;
	const byte		*src, *inrow;
	int				i, j;

	out = pixels_out;

	if ((scaled_width >= inwidth) && (scaled_height >= inheight))
	{
	// JDH: now creates padded texture instead of stretched
		src = original;
		for (i = 0; i < inheight; i++)
		{
			for (j = 0; j < inwidth; j++)
				*out++ = translate32[*src++];
			
			// fill in remainder of row with last pixel's color (for mipping/filtering)
			pad = *(out-1);		
			for (; j < scaled_width; j++)
				*out++ = pad;
		}

		// fill in remaining rows with a copy of the last row:
		src = (byte *) (pixels_out + (inheight-1)*scaled_width);		// last row of scaled image
		for (; i < scaled_height; i++)
		{
			for (j = 0; j < scaled_width; j++)
				*out++ = ((unsigned *)src)[j];			
		}
	}
	else
	{
		memset (pixels_out, 0, scaled_width * scaled_height * 4);
		fracstep = (inwidth << 16) / scaled_width;

		for (i = 0 ; i < scaled_height ; i++, out += scaled_width)
		{
			inrow = original + inwidth * (i * inheight / scaled_height);
			frac = fracstep >> 1;
			for (j = 0 ; j < scaled_width ; j += 4)
			{
				out[j] = translate32[inrow[frac>>16]];
				frac += fracstep;
				out[j+1] = translate32[inrow[frac>>16]];
				frac += fracstep;
				out[j+2] = translate32[inrow[frac>>16]];
				frac += fracstep;
				out[j+3] = translate32[inrow[frac>>16]];
				frac += fracstep;
			}
		}
	}

//	return pixels;
}

/*
===============
R_TranslatePlayerSkin

Translates a skin texture by the per-player color lookup
===============
*/
void R_TranslatePlayerSkin (int playernum)
{
	int			top, bottom, i, size, scaled_width, scaled_height, inwidth, inheight;
	byte		translate[256], *original;
	unsigned	translate32[256], *pixels;
	model_t		*model;
	aliashdr_t	*paliashdr;
	extern qboolean	Img_HasFullbrights (byte *pixels, int size);
/*******JDH*******/
	entity_t *currententity;
	skingroup_t *skingroup;
/*******JDH*******/

	GL_DisableMultitexture ();

	// locate the original skin pixels
	currententity = &cl_entities[1+playernum];
	if (!(model = currententity->model))
		model = cl.model_precache[currententity->baseline.modelindex];		// JDH: added for QW demos

	if (!model)
		return;		// player doesn't have a model yet
	if (model->type != mod_alias)
		return;		// only translate skins on alias models

	paliashdr = (aliashdr_t *)Mod_Extradata (model);
	if (currententity->skinnum < 0 || currententity->skinnum >= paliashdr->numskins)
	{
		Con_DPrintf ("R_TranslatePlayerSkin(%d): Invalid player skin #%d\n", playernum, currententity->skinnum);
		currententity->skinnum = 0;
	}

	inwidth = paliashdr->skinwidth;
	inheight = paliashdr->skinheight;
	size = inwidth * inheight;
//	if (size & 3)			// JDH: removed 2010/02/07
//		Sys_Error ("R_TranslatePlayerSkin: size & 3");

	skingroup = &paliashdr->skins[currententity->skinnum];
	if (skingroup->texels == 0)
	{
		// texels is set to 0 when a 24-bit texture is being used
		player_skins[playernum] = skingroup->skins[0].gl_texturenum;
		player_skins_fb[playernum] = skingroup->skins[0].fb_texturenum;
		return;
	}

	original = (byte *)paliashdr + skingroup->texels;
	top = cl.scores[playernum].colors & 0xf0;
	bottom = (cl.scores[playernum].colors & 15) << 4;

	for (i=0 ; i<256 ; i++)
		translate[i] = i;

	for (i=0 ; i<16 ; i++)
	{
		// the artists made some backward ranges. sigh.
		translate[TOP_RANGE+i] = (top < 128) ? top + i : top + 15 - i;
		translate[BOTTOM_RANGE+i] = (bottom < 128) ? bottom + i : bottom + 15 - i;
	}

	// because this happens during gameplay, do it fast
	// instead of sending it through gl_upload 8
	player_skins[playernum] = playertextures + playernum;
	GL_Bind (playertextures + playernum);

	i = (gl_picmip_all.value ? TEX_MIPMAP : 0);
	GL_ScaleDimensions (inwidth, inheight, &scaled_width, &scaled_height, i | TEX_PLAYER);

	for (i=0 ; i<256 ; i++)
		translate32[i] = d_8to24table[translate[i]];

	pixels = Q_malloc (scaled_width * scaled_height * 4);
	R_CreatePlayerTexture (original, inwidth, inheight, translate32, scaled_width, scaled_height, pixels);
	
	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	player_skins_fb[playernum] = 0;
	if (Img_HasFullbrights(original, inwidth*inheight))
	{
		player_skins_fb[playernum] = playertextures + playernum + MAX_SCOREBOARD;

		GL_Bind (player_skins_fb[playernum]);

		// set all non-fullbrights to transparent
		for (i=0 ; i<224 ; i++)
			translate32[i] &= 0x00FFFFFF;
		
		R_CreatePlayerTexture (original, inwidth, inheight, translate32, scaled_width, scaled_height, pixels);

		glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	free (pixels);
}

/*
===============
R_ParseWorldspawn (JDH)
===============
*/
void R_ParseWorldspawn (void)
{
	char key[128], value[4096];
	const char *data;
	qboolean found_sky = false;

	// reset any variables:
	Fog_NewMap ();

	data = COM_Parse (cl.worldmodel->entities);
	if (!data)
		goto EXIT_POINT; // error
	
	if (com_token[0] != '{')
		goto EXIT_POINT; // error
	
	while (1)
	{
		data = COM_Parse (data);
		if (!data)
			break; // error
		
		if (com_token[0] == '}')
			break; // end of worldspawn
		
		if (com_token[0] == '_')
			Q_strcpy (key, com_token + 1, sizeof(key));
		else
			Q_strcpy (key, com_token, sizeof(key));
		
		while (key[strlen(key)-1] == ' ') // remove trailing spaces
			key[strlen(key)-1] = 0;
		
		data = COM_Parse (data);
		if (!data)
			break; // error
		Q_strcpy (value, com_token, sizeof(value));

		if (!strcmp(key, "fog"))
		{
			Fog_InitWorldspawn (value);
		}
		else if (!strcmp (key, "sky"))
		{
			found_sky = Sky_InitWorldspawn (value);
		}
	#ifdef HEXEN2_SUPPORT
		else if (hexen2 && !strcmp (key, "skyshader"))
		{
			found_sky = Sky_InitWorldspawn (value);
		}
	#endif
	}

EXIT_POINT:
	if (!found_sky)
		Cvar_SetDirect (&r_skybox, "");
}

/*
===============
R_NewMap
===============
*/
void R_NewMap (void)
{
	int	i/*, waterline*/;
	texture_t *currtex;
	
	for (i=0 ; i<256 ; i++)
		d_lightstylevalue[i] = 264;		// normal light value

	memset (&r_worldentity, 0, sizeof(r_worldentity));
	r_worldentity.model = cl.worldmodel;

	// clear out efrags in case the level hasn't been reloaded
	// FIXME: is this one short?
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
		cl.worldmodel->leafs[i].efrags = NULL;
	
	r_oldviewleaf = NULL;
	r_oldviewleaf2 = NULL;

	r_viewleaf = NULL;
	R_ClearParticles ();

	R_ParseWorldspawn ();	// JDH
		 	
	GL_BuildLightmaps ();

	for (i=0; i < cl.worldmodel->numtextures; i++)
	{
		currtex = cl.worldmodel->textures[i];
		if (!currtex)
			continue;
		
		currtex->texturechain = NULL;
		// JDH: waterline is only needed if we do waterwarp
		/*for ( waterline = 0; waterline < 2; waterline++ )
		{
 			currtex->texturechain[ waterline ] = NULL;
			currtex->texturechain_tail[ waterline ] = &currtex->texturechain[ waterline ];
		}*/
	}	
}

/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
void R_TimeRefresh_f (cmd_source_t src)
{
	int	i;
	float	start, stop, time;

	glDrawBuffer (GL_FRONT);
	glFinish ();

	start = Sys_DoubleTime ();
	for (i=0 ; i<128 ; i++)
	{
		r_refdef.viewangles[1] = i * (360.0 / 128.0);
		R_RenderView ();
	}

	glFinish ();
	stop = Sys_DoubleTime ();
	time = stop - start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128/time);

	glDrawBuffer (GL_BACK);
	GL_EndRendering ();
}

void D_FlushCaches (void)
{
}

#endif		//#ifndef RQM_SV_ONLY
