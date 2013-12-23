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
// gl_mesh.c: triangle model functions

#include "quakedef.h"

#ifndef RQM_SV_ONLY

/*
=================================================================

ALIAS MODEL DISPLAY LIST GENERATION

=================================================================
*/

#define MAX_CMDS 16384	// was 8192

/*****JDH*****/
//model_t		*aliasmodel;
//aliashdr_t	*paliashdr;
/*****JDH*****/

qboolean	used[MAX_CMDS];

// the command list holds counts and s/t values that are valid for
// every frame
int		commands[MAX_CMDS];
int		numcommands;

// all frames will have their vertexes rearranged and expanded
// so they are in the order expected by the command list
int		vertexorder[MAX_CMDS];
int		numorder;

int		allverts, alltris;

#define MAX_STRIPS 256	// was 128

int		stripverts[MAX_STRIPS];
int		striptris[MAX_STRIPS];
int		stripcount;

#ifdef HEXEN2_SUPPORT
int		stripstverts[MAX_STRIPS];
#endif

extern cvar_t gl_cache_ms2;

/*
================
StripLength
================
*/
int StripLength (int numtris, int starttri, int startv)
{
	int			m1, m2, j, k;
	mtriangle_t	*last, *check;
#ifdef HEXEN2_SUPPORT
	int			st1, st2;
#endif

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripverts[1] = last->vertindex[(startv+1)%3];
	stripverts[2] = last->vertindex[(startv+2)%3];

#ifdef HEXEN2_SUPPORT
	stripstverts[0] = last->stindex[(startv)%3];
	stripstverts[1] = last->stindex[(startv+1)%3];
	stripstverts[2] = last->stindex[(startv+2)%3];
	
	st1 = last->stindex[(startv+2)%3];
	st2 = last->stindex[(startv+1)%3];
#endif
	
	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+2)%3];
	m2 = last->vertindex[(startv+1)%3];

	// look for a matching triangle
nexttri:
	for (j=starttri+1, check=&triangles[starttri+1] ; j<numtris ; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k=0 ; k<3 ; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;

		#ifdef HEXEN2_SUPPORT
			if (check->stindex[k] != st1)
				continue;
			if (check->stindex[ (k+1)%3 ] != st2)
				continue;
		#endif

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			if (stripcount & 1)
			{
				m2 = check->vertindex[ (k+2)%3 ];
			#ifdef HEXEN2_SUPPORT
				st2 = check->stindex[ (k+2)%3 ];
			#endif
			}
			else
			{
				m1 = check->vertindex[ (k+2)%3 ];
			#ifdef HEXEN2_SUPPORT
				st1 = check->stindex[ (k+2)%3 ];
			#endif
			}

		#ifdef HEXEN2_SUPPORT
			stripstverts[stripcount+2] = check->stindex[ (k+2)%3 ];
		#endif
			stripverts[stripcount+2] = check->vertindex[ (k+2)%3 ];
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}

done:
	// clear the temp used flags
	for (j=starttri+1 ; j<numtris ; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}

/*
===========
FanLength
===========
*/
int FanLength (int numtris, int starttri, int startv)
{
	int			m1, m2, j, k;
	mtriangle_t	*last, *check;
#ifdef HEXEN2_SUPPORT
	int		st1, st2;
#endif

	used[starttri] = 2;

	last = &triangles[starttri];

	stripverts[0] = last->vertindex[(startv)%3];
	stripverts[1] = last->vertindex[(startv+1)%3];
	stripverts[2] = last->vertindex[(startv+2)%3];

#ifdef HEXEN2_SUPPORT
	stripstverts[0] = last->stindex[(startv)%3];
	stripstverts[1] = last->stindex[(startv+1)%3];
	stripstverts[2] = last->stindex[(startv+2)%3];

	st1 = last->stindex[(startv+2)%3];
	st2 = last->stindex[(startv+1)%3];
#endif

	striptris[0] = starttri;
	stripcount = 1;

	m1 = last->vertindex[(startv+0)%3];
	m2 = last->vertindex[(startv+2)%3];


	// look for a matching triangle
nexttri:
	for (j=starttri+1, check=&triangles[starttri+1] ; j<numtris ; j++, check++)
	{
		if (check->facesfront != last->facesfront)
			continue;
		for (k=0 ; k<3 ; k++)
		{
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[ (k+1)%3 ] != m2)
				continue;

		#ifdef HEXEN2_SUPPORT
			if (check->stindex[k] != st1)
				continue;
			if (check->stindex[ (k+1)%3 ] != st2)
				continue;
		#endif
			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			m2 = check->vertindex[ (k+2)%3 ];

		#ifdef HEXEN2_SUPPORT
			st2 = check->stindex[ (k+2)%3 ];
			stripstverts[stripcount+2] = st2;
		#endif

			stripverts[stripcount+2] = m2;
			striptris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}

done:
	// clear the temp used flags
	for (j=starttri+1 ; j<numtris ; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}


/*
================
BuildTris

Generate a list of trifans or strips
for the model, which holds for all frames
================
*/
void BuildTris (aliashdr_t *pheader)
{
	int		i, j, k, startv, len, bestlen, besttype, type;
	float	s, t;
	int		bestverts[1024], besttris[1024];
#ifdef HEXEN2_SUPPORT
	int		beststverts[1024];
#endif

	// build tristrips
	numorder = 0;
	numcommands = 0;
	memset (used, 0, sizeof(used));
	for (i=0 ; i<pheader->numtris ; i++)
	{
		// pick an unused triangle and start the trifan
		if (used[i])
			continue;

		bestlen = 0;
		for (type=0 ; type<2 ; type++)
//	type = 1;
		{
			for (startv=0 ; startv < 3 ; startv++)
			{
				len = (type == 1) ? StripLength (pheader->numtris, i, startv) : 
										FanLength (pheader->numtris, i, startv);
				if (len > bestlen)
				{
					besttype = type;
					bestlen = len;
					for (j=0 ; j<bestlen+2 ; j++)
					{
						bestverts[j] = stripverts[j];
					#ifdef HEXEN2_SUPPORT
						beststverts[j] = stripstverts[j];
					#endif
					}
					
					for (j=0 ; j<bestlen ; j++)
						besttris[j] = striptris[j];
				}
			}
		}

		// mark the tris on the best strip as used
		for (j=0 ; j<bestlen ; j++)
			used[besttris[j]] = 1;

		commands[numcommands++] = (besttype == 1) ? (bestlen+2) : -(bestlen+2);

		for (j=0 ; j<bestlen+2 ; j++)
		{
			// emit a vertex into the reorder buffer
			k = bestverts[j];
			vertexorder[numorder++] = k;

		#ifdef HEXEN2_SUPPORT
			k = beststverts[j];
		#endif

			// emit s/t coords into the commands stream
			s = stverts[k].s;
			t = stverts[k].t;
			
			if (!triangles[besttris[0]].facesfront && stverts[k].onseam)
				s += pheader->skinwidth / 2;	// on back side
			
			s = (s + 0.5) / pheader->skinwidth;
			t = (t + 0.5) / pheader->skinheight;

			*(float *)&commands[numcommands++] = s;
			*(float *)&commands[numcommands++] = t;
		}
	}

	commands[numcommands++] = 0;		// end of list marker

	if (developer.value > 1)
		Con_Printf ("%3i tri %3i vert %3i cmd\n", pheader->numtris, numorder, numcommands);

	allverts += numorder;
	alltris += pheader->numtris;
}

/*
================
GL_MakeAliasModelDisplayLists
================
*/
void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *paliashdr)
{
	int			i, j, count, texflags, new_width, new_height, cmdcount;
	int			*cmds, *loadcmds;
	trivertx_t	*verts;
	skingroup_t	*skingroup;
	skin_t		*skin;
	char		cache[MAX_QPATH], fullpath[MAX_OSPATH];
	FILE		*f = NULL;
	com_fileinfo_t fileinfo;
	const searchpath_t *path1;

// JDH: added caching back in - faster than rebuilding on slow machines
	if (gl_cache_ms2.value)
	{
		//
		// look for a cached version
		//
		count = Q_strcpy (cache, "glquake/", sizeof(cache));
		count += COM_StripExtension (COM_SkipPath(m->name), cache+count, sizeof(cache)-count);
		Q_strcpy (cache+count, ".ms2", sizeof(cache)-count);

		if (COM_FindFile (m->name, 0, &fileinfo))		// JDH: look for ms2 only in same folder as mdl file
		{
			path1 = fileinfo.searchpath;
			if (COM_FindFile (cache, 0, &fileinfo))
			{
				if ((path1 == fileinfo.searchpath) || COM_FilenamesEqual (path1->filename, fileinfo.searchpath->filename))
					f = COM_FOpenFile (cache, 0, NULL);
				else
				{
					if (!fileinfo.searchpath->pack)
						COM_DeleteFile (va("%s/%s", fileinfo.searchpath->filename, cache));		// so it's not found next time
				}
			}
		}
	}
	
	if (f)
	{
		fread (&numcommands, 4, 1, f);
		fread (&numorder, 4, 1, f);
		fread (&commands, numcommands * sizeof(commands[0]), 1, f);
		fread (&vertexorder, numorder * sizeof(vertexorder[0]), 1, f);
		fclose (f);
	}
	else
	{
		//
		// build it from scratch
		//
		if (developer.value > 1)
			Con_Printf ("meshing %s...\n",m->name);

		BuildTris (paliashdr);		// trifans or lists

		if (gl_cache_ms2.value)
		{
			//
			// save out the cached version
			//
			Q_snprintfz (fullpath, sizeof(fullpath), "%s/%s", path1 ? path1->filename : com_gamedir, cache);
			COM_CreatePath (fullpath);
			f = fopen (fullpath, "wb");
			if (f)
			{
				fwrite (&numcommands, 4, 1, f);
				fwrite (&numorder, 4, 1, f);
				fwrite (&commands, numcommands * sizeof(commands[0]), 1, f);
				fwrite (&vertexorder, numorder * sizeof(vertexorder[0]), 1, f);
				fclose (f);
			}
		}
	}

	// save the data out
	paliashdr->poseverts = numorder;

	//cmds = Hunk_Alloc (numcommands * 4);
	cmds = Hunk_Alloc (numcommands * 6);
	paliashdr->commands = (byte *)cmds - (byte *)paliashdr;
	
//	memcpy (cmds, commands, numcommands * 4);

	// because of external textures, each skin for a given model could be a different size.
	// This means coordinate scaling has to occur when model is drawn (either that or
	// maintain multiple command lists)

	texflags = (gl_picmip_all.value ? TEX_MIPMAP : 0);
	if (m->modhint == MOD_PLAYER)
		texflags |= TEX_PLAYER;

	for (i=0; i < paliashdr->numskins; i++)
	{
		skingroup = &paliashdr->skins[i];
		
		for (j = 0; j < 4; j++)
		{
			skin = &skingroup->skins[j];

			if (texflags & TEX_PLAYER)
			{
			// player skins get padded if enlarged, but not if shrunk  -- FIXME?
				GL_ScaleDimensions (skin->h_value, skin->v_value, &new_width, &new_height, texflags);
			}
			else
			{
			// other skins get padded _before_ reducing size (if reduction is necessary)
				for (new_width = 1 ; new_width < skin->h_value ; new_width <<= 1);
				for (new_height = 1 ; new_height < skin->v_value ; new_height <<= 1);
			}
			
		// Initially, h_value & v_value hold skin width & height, respectively.
		// From here on, however, they will store texture coordinates.

			if ((new_width >= skin->h_value) && (new_height >= skin->v_value))
			{
				// skin is padded only if it is scaled up
				skin->h_value /= (float) new_width;
				skin->v_value /= (float) new_height;
			}
			else
			{
				skin->h_value = 1.0;
				skin->v_value = 1.0;
			}
		}
	}
	

// JDH: for compatibility with MD2 models, another value is stored after each coordinate pair.
// (MD2 can store texture coords in a different order than model coords; this value ties the
//  two pairs together.  But here, the 2 indices are the same).
	cmdcount = 0;
	
	loadcmds = commands;
	while(1)
	{
		*cmds++ = count = *loadcmds++;

		if (!count)
			break;

		if (count < 0)
			count = -count;

		do
		{
			*(float *)cmds++ = *(float *)loadcmds++;
			*(float *)cmds++ = *(float *)loadcmds++;
			*cmds++ = cmdcount++;
		} while (--count);
	}

	verts = Hunk_Alloc (paliashdr->numposes * paliashdr->poseverts * sizeof(trivertx_t));
	paliashdr->posedata = (byte *)verts - (byte *)paliashdr;
	for (i=0 ; i<paliashdr->numposes ; i++)
		for (j=0 ; j<numorder ; j++)
			*verts++ = poseverts[i][vertexorder[j]];
}

#endif		//#ifndef RQM_SV_ONLY
