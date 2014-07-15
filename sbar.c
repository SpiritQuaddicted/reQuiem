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
// sbar.c -- status bar code

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#define	SBAR_HEIGHT		24

int		sb_updates;		// if >= vid.numpages, no update needed

mpic_t		*sb_ibar;
mpic_t		*sb_sbar;
mpic_t		*sb_scorebar;

mpic_t		*sb_weapons[7][8];	// 0 is active, 1 is owned, 2-5 are flashes
mpic_t		*sb_ammo[4];
mpic_t		*sb_sigil[4];
mpic_t		*sb_armor[3];
mpic_t		*sb_items[32];

mpic_t		*sb_faces[5][2];	// 0 is dead, 1-4 are alive
					// 0 is static, 1 is temporary animation
mpic_t		*sb_face_invis;
mpic_t		*sb_face_quad;
mpic_t		*sb_face_invuln;
mpic_t		*sb_face_invis_invuln;

qboolean	sb_showscores;

mpic_t		*rsb_invbar[2];
mpic_t		*rsb_weapons[5];
mpic_t		*rsb_items[2];
mpic_t		*rsb_ammo[3];
mpic_t		*rsb_teambord;		// PGM 01/19/97 - team color border

//MED 01/04/97 added two more weapons + 3 alternates for grenade launcher
mpic_t		*hsb_weapons[7][5];   // 0 is active, 1 is owned, 2-5 are flashes
//MED 01/04/97 added array to simplify weapon parsing
int		hipweapons[4] = {HIT_LASER_CANNON_BIT, HIT_MJOLNIR_BIT, 4, HIT_PROXIMITY_GUN_BIT};
//MED 01/04/97 added hipnotic items array
mpic_t		*hsb_items[2];

void Sbar_MiniDeathmatchOverlay (int sbarlines);
void Sbar_DeathmatchOverlay (void);

// by joe
int	sbar_xofs;
cvar_t	scr_centersbar = {"scr_centersbar", "1", CVAR_FLAG_ARCHIVE};	// JT021305 - default to centered

#ifdef HEXEN2_SUPPORT
  int sv_kingofhill = 0;
#endif

extern	cvar_t	scr_sbarsize;

/*
===============
Sbar_Height
   returns the actual height in pixels, including any scale factors
===============
*/
int Sbar_Height (void)
{
	int h;

	if ((scr_viewsize.value >= scr_viewsize.maxvalue) || cl.intermission)
		h = 0;

	else if ((scr_viewsize.value >= scr_viewsize.maxvalue-10) ||
		((scr_viewsize.value >= scr_viewsize.maxvalue-20) && !cl_sbar.value))
		h = SBAR_HEIGHT;
	else
		h = 2*SBAR_HEIGHT;

	if (scr_sbarsize.value > 0)
		h *= vid.width * (scr_sbarsize.value/100.0) / 320.0;

	return h;
}

/*
===============
Sbar_ShowScores

Tab key down
===============
*/
void Sbar_ShowScores (cmd_source_t src)
{
	if (sb_showscores)
		return;
	sb_showscores = true;
	sb_updates = 0;
}

/*
===============
Sbar_DontShowScores

Tab key up
===============
*/
void Sbar_DontShowScores (cmd_source_t src)
{
	sb_showscores = false;
	sb_updates = 0;
}

/*
===============
Sbar_Changed
===============
*/
void Sbar_Changed (void)
{
	sb_updates = 0;	// update next frame
}

void Sbar_Hipnotic_Init (void)
{
	int	i;

	hsb_weapons[0][0] = Draw_PicFromWad ("inv_laser");
	hsb_weapons[0][1] = Draw_PicFromWad ("inv_mjolnir");
	hsb_weapons[0][2] = Draw_PicFromWad ("inv_gren_prox");
	hsb_weapons[0][3] = Draw_PicFromWad ("inv_prox_gren");
	hsb_weapons[0][4] = Draw_PicFromWad ("inv_prox");

	hsb_weapons[1][0] = Draw_PicFromWad ("inv2_laser");
	hsb_weapons[1][1] = Draw_PicFromWad ("inv2_mjolnir");
	hsb_weapons[1][2] = Draw_PicFromWad ("inv2_gren_prox");
	hsb_weapons[1][3] = Draw_PicFromWad ("inv2_prox_gren");
	hsb_weapons[1][4] = Draw_PicFromWad ("inv2_prox");

	for (i=0 ; i<5 ; i++)
	{
		hsb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_laser", i+1));
		hsb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_mjolnir", i+1));
		hsb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_gren_prox", i+1));
		hsb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_prox_gren", i+1));
		hsb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_prox", i+1));
	}

	hsb_items[0] = Draw_PicFromWad ("sb_wsuit");
	hsb_items[1] = Draw_PicFromWad ("sb_eshld");

// joe: better reload these, coz they might look different
// --> JDH: um, no, coz the first 20 bytes have been overwritten
//		by the first call to Draw_PicFromWad

/*	sb_weapons[0][4] = Draw_PicFromWad ("inv_rlaunch");
	sb_weapons[1][4] = Draw_PicFromWad ("inv2_rlaunch");
	sb_items[0] = Draw_PicFromWad ("sb_key1");
	sb_items[1] = Draw_PicFromWad ("sb_key2");
*/
}

void Sbar_Rogue_Init (void)
{
	rsb_invbar[0] = Draw_PicFromWad ("r_invbar1");
	rsb_invbar[1] = Draw_PicFromWad ("r_invbar2");

	rsb_weapons[0] = Draw_PicFromWad ("r_lava");
	rsb_weapons[1] = Draw_PicFromWad ("r_superlava");
	rsb_weapons[2] = Draw_PicFromWad ("r_gren");
	rsb_weapons[3] = Draw_PicFromWad ("r_multirock");
	rsb_weapons[4] = Draw_PicFromWad ("r_plasma");

	rsb_items[0] = Draw_PicFromWad ("r_shield1");
	rsb_items[1] = Draw_PicFromWad ("r_agrav1");

// PGM 01/19/97 - team color border
	rsb_teambord = Draw_PicFromWad ("r_teambord");
// PGM 01/19/97 - team color border

	rsb_ammo[0] = Draw_PicFromWad ("r_ammolava");
	rsb_ammo[1] = Draw_PicFromWad ("r_ammomulti");
	rsb_ammo[2] = Draw_PicFromWad ("r_ammoplasma");
}

/*
===============
Sbar_LoadWadPics
===============
*/
void Sbar_LoadWadPics (void)
{
	int	i;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		Sbar_LoadWadPics_H2 ();
		return;
	}
#endif

	sb_weapons[0][0] = Draw_PicFromWad ("inv_shotgun");
	sb_weapons[0][1] = Draw_PicFromWad ("inv_sshotgun");
	sb_weapons[0][2] = Draw_PicFromWad ("inv_nailgun");
	sb_weapons[0][3] = Draw_PicFromWad ("inv_snailgun");
	sb_weapons[0][4] = Draw_PicFromWad ("inv_rlaunch");
	sb_weapons[0][5] = Draw_PicFromWad ("inv_srlaunch");
	sb_weapons[0][6] = Draw_PicFromWad ("inv_lightng");

	sb_weapons[1][0] = Draw_PicFromWad ("inv2_shotgun");
	sb_weapons[1][1] = Draw_PicFromWad ("inv2_sshotgun");
	sb_weapons[1][2] = Draw_PicFromWad ("inv2_nailgun");
	sb_weapons[1][3] = Draw_PicFromWad ("inv2_snailgun");
	sb_weapons[1][4] = Draw_PicFromWad ("inv2_rlaunch");
	sb_weapons[1][5] = Draw_PicFromWad ("inv2_srlaunch");
	sb_weapons[1][6] = Draw_PicFromWad ("inv2_lightng");

	for (i=0 ; i<5 ; i++)
	{
		sb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_shotgun", i+1));
		sb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_sshotgun", i+1));
		sb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_nailgun", i+1));
		sb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_snailgun", i+1));
		sb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_rlaunch", i+1));
		sb_weapons[2+i][5] = Draw_PicFromWad (va("inva%i_srlaunch", i+1));
		sb_weapons[2+i][6] = Draw_PicFromWad (va("inva%i_lightng", i+1));
	}

	sb_ammo[0] = Draw_PicFromWad ("sb_shells");
	sb_ammo[1] = Draw_PicFromWad ("sb_nails");
	sb_ammo[2] = Draw_PicFromWad ("sb_rocket");
	sb_ammo[3] = Draw_PicFromWad ("sb_cells");

	sb_armor[0] = Draw_PicFromWad ("sb_armor1");
	sb_armor[1] = Draw_PicFromWad ("sb_armor2");
	sb_armor[2] = Draw_PicFromWad ("sb_armor3");

	sb_items[0] = Draw_PicFromWad ("sb_key1");
	sb_items[1] = Draw_PicFromWad ("sb_key2");
	sb_items[2] = Draw_PicFromWad ("sb_invis");
	sb_items[3] = Draw_PicFromWad ("sb_invuln");
	sb_items[4] = Draw_PicFromWad ("sb_suit");
	sb_items[5] = Draw_PicFromWad ("sb_quad");

	sb_sigil[0] = Draw_PicFromWad ("sb_sigil1");
	sb_sigil[1] = Draw_PicFromWad ("sb_sigil2");
	sb_sigil[2] = Draw_PicFromWad ("sb_sigil3");
	sb_sigil[3] = Draw_PicFromWad ("sb_sigil4");

	for (i = 0; i < 5; i++)
	{
		sb_faces[i][0] = Draw_PicFromWad (va("face%d", 5-i));
		sb_faces[i][1] = Draw_PicFromWad (va("face_p%d", 5-i));
	}
/*
	sb_faces[4][0] = Draw_PicFromWad ("face1");
	sb_faces[4][1] = Draw_PicFromWad ("face_p1");
	sb_faces[3][0] = Draw_PicFromWad ("face2");
	sb_faces[3][1] = Draw_PicFromWad ("face_p2");
	sb_faces[2][0] = Draw_PicFromWad ("face3");
	sb_faces[2][1] = Draw_PicFromWad ("face_p3");
	sb_faces[1][0] = Draw_PicFromWad ("face4");
	sb_faces[1][1] = Draw_PicFromWad ("face_p4");
	sb_faces[0][0] = Draw_PicFromWad ("face5");
	sb_faces[0][1] = Draw_PicFromWad ("face_p5");
*/
	sb_face_invis = Draw_PicFromWad ("face_invis");
	sb_face_invuln = Draw_PicFromWad ("face_invul2");
	sb_face_invis_invuln = Draw_PicFromWad ("face_inv2");
	sb_face_quad = Draw_PicFromWad ("face_quad");

	sb_sbar = Draw_PicFromWad ("sbar");
	sb_ibar = Draw_PicFromWad ("ibar");
	sb_scorebar = Draw_PicFromWad ("scorebar");


//MED 01/04/97 added new hipnotic weapons
	if (hipnotic)
		Sbar_Hipnotic_Init ();

	else if (rogue)
		Sbar_Rogue_Init ();
}

/*
===============
Sbar_ClearWadPics
===============
*/
void Sbar_ClearWadPics (void)
{
	int	i, j;

	for (i = 0; i < 7; i++)
		for (j = 0; j < 8; j++)
			sb_weapons[i][j] = NULL;

	for (i = 0; i < 4; i++)
	{
		sb_ammo[i] = NULL;
		sb_sigil[i] = NULL;
	}

	for (i = 0; i < 3; i++)
		sb_armor[i] = NULL;

	for (i = 0; i < 32; i++)
		sb_items[i] = NULL;

	for (i = 0; i < 5; i++)
	{
		sb_faces[i][0] = NULL;
		sb_faces[i][1] = NULL;
	}

	sb_face_invis = sb_face_quad = sb_face_invuln = sb_face_invis_invuln = NULL;
	sb_sbar = sb_ibar = sb_scorebar = NULL;

// Rogue:
	rsb_invbar[0] = rsb_invbar[1] = NULL;
	rsb_items[0] = rsb_items[1] = NULL;
	rsb_ammo[0] = rsb_ammo[1] = rsb_ammo[2] = NULL;
	rsb_teambord = NULL;

	for (i = 0; i < 5; i++)
		rsb_weapons[i] = NULL;

// Hipnotic:
	for (i = 0; i < 7; i++)
		for (j = 0; j < 5; j++)
			hsb_weapons[i][j] = NULL;

	hsb_items[0] = hsb_items[1] = NULL;
}

/*
===============
Sbar_Init
===============
*/
void Sbar_Init (void)
{
#ifdef HEXEN2_SUPPORT
	Sbar_Hexen2_Init ();
#endif

	// by joe
	Cvar_RegisterBool (&scr_centersbar);

	Cmd_AddCommand ("+showscores", Sbar_ShowScores, 0);
	Cmd_AddCommand ("-showscores", Sbar_DontShowScores, 0);

//	Sbar_LoadWadPics ();		// JDH: now done in Draw_Init
}

//=============================================================================

// drawing routines are relative to the status bar location

/*
=============
Sbar_DrawPic
=============
*/
void Sbar_DrawPic (int x, int y, const mpic_t *pic)
{
	Draw_Pic (x + sbar_xofs, y + (vid.height-SBAR_HEIGHT), pic);
}

/*
=============
Sbar_DrawSubPic
=============
JACK: Draws a portion of the picture in the status bar.
*/
void Sbar_DrawSubPic (int x, int y, const mpic_t *pic, int srcx, int srcy, int width, int height)
{
	Draw_SubPic (x, y + (vid.height-SBAR_HEIGHT), pic, srcx, srcy, width, height);
}

/*
=============
Sbar_DrawTransPic
=============
*/
void Sbar_DrawTransPic (int x, int y, const mpic_t *pic)
{
	Draw_TransPic (x + sbar_xofs, y + (vid.height-SBAR_HEIGHT), pic);
}

/*
================
Sbar_DrawCharacter

Draws one solid graphics character
================
*/
void Sbar_DrawCharacter (int x, int y, int num)
{
	Draw_Character (x + 4 + sbar_xofs, y + vid.height-SBAR_HEIGHT, num);
}

/*
================
Sbar_DrawString
================
*/
void Sbar_DrawString (int x, int y, const char *str)
{
	Draw_String (x + sbar_xofs, y + (vid.height-SBAR_HEIGHT), str);
}

/*
=============
Sbar_itoa
=============
*/
int Sbar_itoa (int num, char *buf)
{
/*	char	*str;
	int	pow10, dig;

	str = buf;

	if (num < 0)
	{
		*str++ = '-';
		num = -num;
	}

	for (pow10 = 10 ; num >= pow10 ; pow10 *= 10)
		;

	do {
		pow10 /= 10;
		dig = num/pow10;
		*str++ = '0'+dig;
		num -= dig*pow10;
	} while (pow10 != 1);

	*str = 0;

	return str-buf;
*/
	return Q_snprintfz (buf, 12, "%d", num);
}

/*
==================
Sbar_IntermissionNumber
==================
*/
void Sbar_IntermissionNumber (int x, int y, int num, int digits, qboolean altcolor)
{
	char	str[12], *ptr;
	int		len;

	len = Sbar_itoa (num, str);
	ptr = str;
	if (len > digits)
		ptr += (len-digits);
	if (len < digits)
		x += (digits-len)*DRAW_BIGCHARWIDTH;

	Draw_BigNumString (x, y, ptr, (altcolor ? DRAWNUM_ALTCOLOR : 0));
}

/*
=============
Sbar_DrawNum
=============
*/
void Sbar_DrawNum (int x, int y, int num, int digits, qboolean altcolor)
{
	Sbar_IntermissionNumber (x + sbar_xofs, y + (vid.height-SBAR_HEIGHT), num, digits, altcolor);
}

//=============================================================================

int	fragsort[MAX_SCOREBOARD];

char	scoreboardtext[MAX_SCOREBOARD][20];
int	scoreboardtop[MAX_SCOREBOARD];
int	scoreboardbottom[MAX_SCOREBOARD];
int	scoreboardcount[MAX_SCOREBOARD];
int	scoreboardlines;

/*
===============
Sbar_SortFrags
===============
*/
void Sbar_SortFrags (void)
{
	int	i, j, k;

// sort by frags
	scoreboardlines = 0;
	for (i=0 ; i<cl.maxclients ; i++)
	{
		if (cl.scores[i].name[0])
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cl.scores[fragsort[j]].frags < cl.scores[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
}

int Sbar_ColorForMap (int m)
{
	return m < 128 ? m + 8 : m + 8;
}

/*
===============
Sbar_UpdateScoreboard
===============
*/
void Sbar_UpdateScoreboard (void)
{
	int		i, k, top, bottom;
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	memset (scoreboardtext, 0, sizeof(scoreboardtext));

	for (i=0 ; i<scoreboardlines; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		Q_snprintfz (&scoreboardtext[i][1], sizeof(scoreboardtext[i]), "%3i %s", s->frags, s->name);

		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		scoreboardtop[i] = Sbar_ColorForMap (top);
		scoreboardbottom[i] = Sbar_ColorForMap (bottom);
	}
}

#define SBAR_ENDDELAY  2
/*
===============
Sbar_SoloScoreboard
===============
*/
void Sbar_SoloScoreboard (int width)
{
	char	str[80], *name;
	int		minutes, seconds, tens, units, len, maxlen;

	sprintf (str,"Monsters:%3i /%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
	Sbar_DrawString (8, 4, str);

	sprintf (str,"Secrets :%3i /%3i", cl.stats[STAT_SECRETS], cl.stats[STAT_TOTALSECRETS]);
	Sbar_DrawString (8, 12, str);

// time
	minutes = cl.time / 60;
	seconds = cl.time - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	sprintf (str, "Time :%3i:%i%i", minutes, tens, units);
//	Sbar_DrawString (184, 4, str);
	Sbar_DrawString (152 + (width-152-96)/2, 4, str);	// 152=19*8, 96=12*8

// draw level name
	len = strlen (cl.levelname);

// JDH: scrolling level name
	//Sbar_DrawString (232 - len*4, 12, cl.levelname);

	maxlen = width/8 - 19;
	if (len > maxlen)
	{
		// scroll the name back & forth (hence the *2)
		int maxval, ofs;

		maxval = (len - maxlen + SBAR_ENDDELAY)*2;
		ofs = (int)(host_time*10) % maxval;
		if (ofs < SBAR_ENDDELAY)
		{
			ofs = 0;		// spend a little longer at the beginning
		}
		else
		{
			ofs -= SBAR_ENDDELAY;
			if (ofs > len-maxlen)
			{
				if (ofs < maxval/2)
				{
					ofs = len-maxlen;		// spend a little longer at the end
				}
				else
				{
					// scroll back to the beginning
					ofs = maxval - ofs - SBAR_ENDDELAY;
				}
			}
		}

		len = Q_strncpy (str, sizeof(str), cl.levelname + ofs, maxlen);
		//str[maxlen] = 0;
		//len = maxlen;
		name = str;
	}
	else name = cl.levelname;

	Sbar_DrawString (152 + (width-8-152-len*8)/2, 12, name);
// JDH
}

/*
===============
Sbar_DrawScoreboard
===============
*/
void Sbar_DrawScoreboard (qboolean headsup)
{
	int old_ofs, len, width;

	// JDH: width is now based on length of level name
	old_ofs = sbar_xofs;

	len = strlen (cl.levelname);
	width = (len+19)*DRAW_CHARWIDTH;
	width = bound (320, width, vid.width);
	width = min (width, 480);

//	if (!headsup)		// I think I like it with background box better
	{
		if (width > 320)
		{
			if (sbar_xofs > 0)
				sbar_xofs = (vid.width - width) / 2;

			len = (width-40)/2;
			Draw_SubPic (sbar_xofs,          vid.height-SBAR_HEIGHT, sb_scorebar,   0, 0,  20, SBAR_HEIGHT);
			Draw_SubPic (sbar_xofs+20,       vid.height-SBAR_HEIGHT, sb_scorebar,  20, 0, len, SBAR_HEIGHT);
			Draw_SubPic (sbar_xofs+20+len,   vid.height-SBAR_HEIGHT, sb_scorebar,  20, 0, len, SBAR_HEIGHT);
			Draw_SubPic (sbar_xofs+20+len*2, vid.height-SBAR_HEIGHT, sb_scorebar, 300, 0,  20, SBAR_HEIGHT);
		}
		else
		{
			Sbar_DrawPic (0, 0, sb_scorebar);
		}
	}

	Sbar_SoloScoreboard (width);
	if (cl.gametype == GAME_DEATHMATCH)
		Sbar_DeathmatchOverlay ();

	sbar_xofs = old_ofs;
}

//=============================================================================

/*
===============
Sbar_DrawInventory
===============
*/
void Sbar_DrawInventory (void)
{
	int			i, j, flashon, ystart;
	char		num[6];
	float		time;
	qboolean	headsup;	// joe
	mpic_t		*invbar, *icon;

	// by joe
	headsup = !(cl_sbar.value || scr_viewsize.value < 100);

	if (rogue)
	{
		if (cl.stats[STAT_ACTIVEWEAPON] >= RIT_LAVA_NAILGUN)
			invbar = rsb_invbar[0];
		else
			invbar = rsb_invbar[1];		// secondary ammo hilited
	}
	else
	{
		invbar = sb_ibar;
	}

	if (!headsup)
	{
		Sbar_DrawPic (0, -24, invbar);
	}

// weapons
	ystart = (hipnotic) ? -100 : -68;
	for (i=0 ; i<7 ; i++)
	{
		if (cl.items & (IT_SHOTGUN << i))
		{
			time = cl.item_gettime[i];
			flashon = (int)((cl.time - time)*10);
			if (flashon < 0)
				flashon = 0;
			if (flashon >= 10)
				flashon = (cl.stats[STAT_ACTIVEWEAPON] == (IT_SHOTGUN << i)) ? 1 : 0;
			else
				flashon = (flashon % 5) + 2;

			icon = sb_weapons[flashon][i];
			if (headsup)
			{
				if (i || vid.height>200)
					Sbar_DrawSubPic ((vid.width-30), ystart - (7-i)*16, icon, 0, 0, 24, 16);
			}
			else
			{
				Sbar_DrawPic (i*24, -16, icon);
			}

			if (flashon > 1)
				sb_updates = 0;		// force update to remove flash
		}
	}

// MED 01/04/97
// hipnotic weapons
	if (hipnotic)
	{
		int	grenadeflashing = 0, left, top;

		for (i=0 ; i<4 ; i++)
		{
			if (cl.items & (1 << hipweapons[i]))
			{
				time = cl.item_gettime[hipweapons[i]];
				flashon = (int)((cl.time - time) * 10);
				if (flashon < 0)
					flashon = 0;
				if (flashon >= 10)
					flashon = (cl.stats[STAT_ACTIVEWEAPON] == (1 << hipweapons[i])) ? 1 : 0;
				else
					flashon = (flashon % 5) + 2;

			// check grenade launcher
				if (i == 2)
				{
					if (!(cl.items & HIT_PROXIMITY_GUN) || !flashon)
						continue;

					grenadeflashing = 1;
					icon = hsb_weapons[flashon][2];
					left = 96;
					top = ystart - 48;
				}
				else if (i == 3)
				{
					if (cl.items & (IT_SHOTGUN << 4))		// standard grenade launcher
					{
						if (grenadeflashing)
							continue;		// icon already drawn

						icon = hsb_weapons[flashon][3];
					}
					else
					{
						icon = hsb_weapons[flashon][4];
					}

					left = 96;
					top = ystart - 48;
				}
				else		// laser cannon or hammer
				{
					icon = hsb_weapons[flashon][i];
					left = 176 + (i*24);
					top = ystart + (i*16);
				}

				if (headsup)
				{
					Sbar_DrawSubPic ((vid.width-30), top, icon, 0, 0, 24, 16);
				}
				else
				{
					Sbar_DrawPic (left, -16, icon);
				}

				if (flashon > 1)
					sb_updates = 0;      // force update to remove flash
			}
		}
	}

// rogue weapons
	if (rogue)
	{	// check for alternate weapons
		// JDH: draw icon for non-selected weapons if they are newly acquired (flashing),
		//		or if player doesn't have the equivalent id weapon

		for (i=0 ; i<5 ; i++)
		{
			if (cl.items & (RIT_LAVA_NAILGUN << i))
			{
				time = cl.item_gettime[12+i];		// RIT_LAVA_NAILGUN = 4096 = 1<<12
				flashon = (int)((cl.time - time)*10);
				if (flashon < 0)
					flashon = 0;
				if (flashon >= 10)
					flashon = (cl.stats[STAT_ACTIVEWEAPON] == (RIT_LAVA_NAILGUN << i)) ? 1 : 0;
				else
					flashon = (flashon % 5) + 2;

				if (flashon == 1)	// active weapon
				{
					icon = rsb_weapons[i];
				}
				else
				{
				// if player has equivalent id weapon, icon has already been drawn
					if ((flashon == 0) && (cl.items & (IT_SHOTGUN << (i+2))))
						continue;

					icon = sb_weapons[flashon][i+2];		// rogue has no custom icons
				}

				if (headsup)		// JDH: this was missing from Joe's code
				{
					if (vid.height>200)
						Sbar_DrawSubPic ((vid.width-30), ystart - (5-i)*16, icon, 0, 0, 24, 16);
				}
				else
				{
					Sbar_DrawPic ((i+2)*24, -16, icon);
				}

				if (flashon > 1)
					sb_updates = 0;		// force update to remove flash
			}
		}
	}

// ammo counts
	for (i=0 ; i<4 ; i++)
	{
		sprintf (num, "%3i", cl.stats[STAT_SHELLS+i]);
		if (headsup)
		{
			Sbar_DrawSubPic ((vid.width-48), -24 - (4-i)*11, invbar, 3+(i*48), 0, 42, 11);
			for (j = 0; j < 3; j++)
			{
				if (num[j] != ' ')
					Draw_Character ((vid.width - 41 + j*8), vid.height-SBAR_HEIGHT-24 - (4-i)*11, 18 + num[j] - '0');
			}
		}
		else
		{
			for (j = 0; j < 3; j++)
			{
				if (num[j] != ' ')
					Sbar_DrawCharacter ((6*i+1+j)*8 - 2, -24, 18 + num[j] - '0');
			}
		}
	}

	flashon = 0;

// items
	for (i=0 ; i<6 ; i++)
	{
		if (cl.items & (1<<(17+i)))
		{
			time = cl.item_gettime[17+i];
			if (time && time > cl.time - 2 && flashon)
			{	// flash frame
				sb_updates = 0;
			}
			else
			{	//MED 01/04/97 changed keys
				if (!hipnotic || (i>1))
					Sbar_DrawPic (192 + i*16, -16, sb_items[i]);
			}
			if (time && time > cl.time - 2)
			sb_updates = 0;
		}
	}

//MED 01/04/97 added hipnotic items
// hipnotic items
	if (hipnotic)
	{
		for (i=0 ; i<2 ; i++)
		{
			if (cl.items & (1<<(24+i)))
			{
				time = cl.item_gettime[24+i];
				if (time && time > cl.time - 2 && flashon)	// flash frame
					sb_updates = 0;
				else
					Sbar_DrawPic (288 + i*16, -16, hsb_items[i]);
				if (time && time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}

// rogue items
	if (rogue)
	{
	// new rogue items
		for (i=0 ; i<2 ; i++)
		{
			if (cl.items & (1<<(29+i)))
			{
				time = cl.item_gettime[29+i];
				if (time && time > cl.time - 2 && flashon)	// flash frame
					sb_updates = 0;
				else
					Sbar_DrawPic (288 + i*16, -16, rsb_items[i]);
				if (time && time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}
	else
	{
	// sigils
		for (i=0 ; i<4 ; i++)
		{
			if (cl.items & (1<<(28+i)))
			{
				time = cl.item_gettime[28+i];
				if (time && time > cl.time - 2 && flashon)	// flash frame
					sb_updates = 0;
				else
					Sbar_DrawPic (320-32 + i*8, -16, sb_sigil[i]);
				if (time && time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}
}

//=============================================================================

/*
===============
Sbar_DrawFrags
===============
*/
void Sbar_DrawFrags (void)
{
	int		i, k, l, top, bottom, x, y, f;
	char		num[12];
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines <= 4 ? scoreboardlines : 4;

	x = 23;
	y = vid.height - SBAR_HEIGHT - 23;

	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		// modified by joe, was only "xofs"
		Draw_Fill (sbar_xofs + x*DRAW_CHARWIDTH + 10, y, 28, 4, top);
		Draw_Fill (sbar_xofs + x*DRAW_CHARWIDTH + 10, y+4, 28, 3, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Sbar_DrawCharacter ((x+1)*DRAW_CHARWIDTH, -24, num[0]);
		Sbar_DrawCharacter ((x+2)*DRAW_CHARWIDTH, -24, num[1]);
		Sbar_DrawCharacter ((x+3)*DRAW_CHARWIDTH, -24, num[2]);

		if (k == cl.viewentity - 1)
		{
			Sbar_DrawCharacter (x*DRAW_CHARWIDTH+2, -24, 16);
			Sbar_DrawCharacter ((x+4)*DRAW_CHARWIDTH-4, -24, 17);
		}
		x += 4;
	}
}

//=============================================================================

/*
===============
Sbar_DrawFace
===============
*/
void Sbar_DrawFace (void)
{
	int	f, anim;
	mpic_t *pic;

// PGM 01/19/97 - team color drawing
// PGM 03/02/97 - fixed so color swatch only appears in CTF modes
	if (rogue && (cl.maxclients != 1)
		&& (teamplay.value > 3) && (teamplay.value < 7))
	{
		int		top, bottom, xofs;
		char		num[12];
		scoreboard_t	*s;

		s = &cl.scores[cl.viewentity - 1];
		// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		xofs = (cl.gametype == GAME_DEATHMATCH) ? 113 : ((vid.width - 320) >> 1) + 113;

		Sbar_DrawPic (112, 0, rsb_teambord);
		Draw_Fill (xofs, vid.height-SBAR_HEIGHT+3, 22, 9, top);
		Draw_Fill (xofs, vid.height-SBAR_HEIGHT+12, 22, 9, bottom);

		// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		if (top == 8)
		{
			if (num[0] != ' ')
				Sbar_DrawCharacter(109, 3, 18 + num[0] - '0');
			if (num[1] != ' ')
				Sbar_DrawCharacter(116, 3, 18 + num[1] - '0');
			if (num[2] != ' ')
				Sbar_DrawCharacter(123, 3, 18 + num[2] - '0');
		}
		else
		{
			Sbar_DrawCharacter ( 109, 3, num[0]);
			Sbar_DrawCharacter ( 116, 3, num[1]);
			Sbar_DrawCharacter ( 123, 3, num[2]);
		}

		return;
	}
// PGM 01/19/97 - team color drawing

	if ((cl.items & (IT_INVISIBILITY | IT_INVULNERABILITY)) == (IT_INVISIBILITY | IT_INVULNERABILITY))
	{
		pic = sb_face_invis_invuln;
	}
	else if (cl.items & IT_QUAD)
	{
		pic = sb_face_quad;
	}
	else if (cl.items & IT_INVISIBILITY)
	{
		pic = sb_face_invis;
	}
	else if (cl.items & IT_INVULNERABILITY)
	{
		pic = sb_face_invuln;
	}
	else
		pic = NULL;

	if (!pic)
	{
		f = cl.stats[STAT_HEALTH] / 20;
		f = bound (0, f, 4);

		if ((cl.time <= cl.faceanim_endtime) && (cl.time >= cl.faceanim_starttime))		// JDH: starttime check for demo rewind
		{
			anim = 1;
			sb_updates = 0;		// make sure the anim gets drawn over
		}
		else
		{
			anim = 0;
		}

		pic = sb_faces[f][anim];
	}

	Sbar_DrawPic (112, 0, pic);
}

/*
===============
Sbar_SetScale - JDH
===============
*/
#if 0
void Sbar_SetScale (void)
{
	float scale = scr_sbarsize.value;

	if (scale < 0)
	{
		Cvar_SetDirect (&scr_sbarsize, "0");
		scale = 0;
	}

	if (scale)
	{
		sbar_screenwidth = 320.0 / (scale/100.0);
		sbar_screenheight = sbar_screenwidth * vid.height / vid.width;
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, sbar_screenwidth, sbar_screenheight, 0, -99999, 99999);
	}
	else
	{
		sbar_screenwidth = vid.width;
		sbar_screenheight = vid.height;
	}
}
#else
void Sbar_SetScale (qboolean enable)
{
	static unsigned orig_vidwidth, orig_vidheight;
	float scale = scr_sbarsize.value;

	if (scale <= 0)
		return;

	assert (!!orig_vidwidth != enable);

	if (enable)
	{
		orig_vidwidth = vid.width;
		orig_vidheight = vid.height;
		vid.width = ceil(320.0 / (scale/100.0));
		vid.height = vid.width * orig_vidheight / orig_vidwidth;
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, vid.width, vid.height, 0, -99999, 99999);
	}
	else
	{
		// restore standard projection
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, orig_vidwidth, orig_vidheight, 0, -99999, 99999);
		vid.width = orig_vidwidth;
		vid.height = orig_vidheight;
		orig_vidwidth = orig_vidheight = 0;
	}
}
#endif

/*
===============
Sbar_Draw
===============
*/
void Sbar_Draw (void)
{
	qboolean	headsup;	// joe
	int			sbarlines;

	if (scr_con_current == vid.height)
		return;		// console is full screen

	if (cls.state != ca_connected)
		return;

//	headsup = !(cl_sbar.value || scr_viewsize.value < 100);
//	if ((sb_updates >= vid.numpages) && !headsup)
//		return;

	Sbar_SetScale (true);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		sbar_xofs = (vid.width - 320) >> 1;
		Sbar_Draw_H2 ();
		goto SBAR_DRAW_EXIT;
	}
#endif

//	scr_copyeverything = 1;
	sb_updates++;

	// by joe
	headsup = !(cl_sbar.value || scr_viewsize.value < 100);
	sbar_xofs = (!scr_centersbar.value || (cl.gametype == GAME_DEATHMATCH)) ? 0 : (vid.width - 320) >> 1;

	if (cl.intermission || (scr_viewsize.value >= scr_viewsize.maxvalue))
		sbarlines = 0;		// no status bar at all
	else if (scr_viewsize.value >= scr_viewsize.maxvalue-10)
		sbarlines = 1;		// no inventory
	else
		sbarlines = 2;

	if (sbarlines > 1)
	{
		Sbar_DrawInventory ();
		if ((!headsup || vid.width < 512) && cl.maxclients != 1)
			Sbar_DrawFrags ();
	}

	if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
	{
		Sbar_DrawScoreboard (headsup);
		sb_updates = 0;
	}
	else if (sbarlines)
	{
		//if (cl_sbar.value || scr_viewsize.value < 100)
		if (!headsup)
			Sbar_DrawPic (0, 0, sb_sbar);

	// keys (hipnotic only)
		//MED 01/04/97 moved keys here so they would not be overwritten
		if (hipnotic)
		{
			if (cl.items & IT_KEY1)
				Sbar_DrawPic (209, 3, sb_items[0]);
			if (cl.items & IT_KEY2)
				Sbar_DrawPic (209, 12, sb_items[1]);
		}

	// armor
		if (cl.items & IT_INVULNERABILITY)
		{
			Sbar_DrawNum (24, 0, 666, 3, 1);
			Sbar_DrawPic (0, 0, draw_disc);
		}
		else
		{
			Sbar_DrawNum (24, 0, cl.stats[STAT_ARMOR], 3, cl.stats[STAT_ARMOR] <= 25);

			if (rogue)
			{
				if (cl.items & RIT_ARMOR3)
					Sbar_DrawPic (0, 0, sb_armor[2]);
				else if (cl.items & RIT_ARMOR2)
					Sbar_DrawPic (0, 0, sb_armor[1]);
				else if (cl.items & RIT_ARMOR1)
					Sbar_DrawPic (0, 0, sb_armor[0]);
			}
			else
			{
				if (cl.items & IT_ARMOR3)
					Sbar_DrawPic (0, 0, sb_armor[2]);
				else if (cl.items & IT_ARMOR2)
					Sbar_DrawPic (0, 0, sb_armor[1]);
				else if (cl.items & IT_ARMOR1)
					Sbar_DrawPic (0, 0, sb_armor[0]);
			}
		}

	// face
		Sbar_DrawFace ();

	// health
		Sbar_DrawNum (136, 0, cl.stats[STAT_HEALTH], 3, cl.stats[STAT_HEALTH] <= 25);

	// ammo icon
		if (rogue)
		{
			if (cl.items & RIT_SHELLS)
				Sbar_DrawPic (224, 0, sb_ammo[0]);
			else if (cl.items & RIT_NAILS)
				Sbar_DrawPic (224, 0, sb_ammo[1]);
			else if (cl.items & RIT_ROCKETS)
				Sbar_DrawPic (224, 0, sb_ammo[2]);
			else if (cl.items & RIT_CELLS)
				Sbar_DrawPic (224, 0, sb_ammo[3]);
			else if (cl.items & RIT_LAVA_NAILS)
				Sbar_DrawPic (224, 0, rsb_ammo[0]);
			else if (cl.items & RIT_PLASMA_AMMO)
				Sbar_DrawPic (224, 0, rsb_ammo[1]);
			else if (cl.items & RIT_MULTI_ROCKETS)
				Sbar_DrawPic (224, 0, rsb_ammo[2]);
		}
		else
		{
			if (cl.items & IT_SHELLS)
				Sbar_DrawPic (224, 0, sb_ammo[0]);
			else if (cl.items & IT_NAILS)
				Sbar_DrawPic (224, 0, sb_ammo[1]);
			else if (cl.items & IT_ROCKETS)
				Sbar_DrawPic (224, 0, sb_ammo[2]);
			else if (cl.items & IT_CELLS)
				Sbar_DrawPic (224, 0, sb_ammo[3]);
		}

		Sbar_DrawNum (248, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);
	}

// added by joe
	// clear unused areas in GL (necessary if cl_sbar = 1)
	if (vid.width > 320 && !headsup)
	{
		int sbarheight = sbarlines * SBAR_HEIGHT;

		// left
		if (scr_centersbar.value)
			Draw_TileClear (0, vid.height - sbarheight, sbar_xofs, sbarheight);
		// right
		Draw_TileClear (320 + sbar_xofs, vid.height - sbarheight, vid.width - (320 + sbar_xofs), sbarheight);
	}

	if (vid.width > 320)
	{
		if (cl.gametype == GAME_DEATHMATCH)
			Sbar_MiniDeathmatchOverlay (sbarlines);
	}

#ifdef HEXEN2_SUPPORT
SBAR_DRAW_EXIT:
#endif
	Sbar_SetScale (false);
/*	if (scr_sbarsize.value)
	{
		// restore standard projection
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, vid.width, vid.height, 0, -99999, 99999);
	}*/
}

//=============================================================================

/*
==================
Sbar_DeathmatchOverlay
==================
*/
void Sbar_DeathmatchOverlay (void)
{
	mpic_t			*pic;
	int				i, k, l, top, bottom, x, y, f, xofs;
	char			*name, num[12];
	scoreboard_t	*s;

//	scr_copyeverything = 1;
//	scr_fullupdate = 0;

	xofs = (vid.width - 320) >> 1;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		name = "menu/title8";
	else
#endif
		name = "ranking";

	pic = Draw_GetCachePic (va("gfx/%s.lmp", name), false);
	if (pic)
		Draw_Pic (xofs + 160 - pic->width/2, 0, pic);	// by joe
//	M_Print (76, 11, "Rankings");

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;

	x = 80 + xofs;
	y = 40;
	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill (x, y, 40, 4, top);
		Draw_Fill (x, y+4, 40, 4, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

	#ifdef HEXEN2_SUPPORT
		if ((hexen2) && (k == sv_kingofhill))
			Draw_Character ( x-12 , y-1, 130);
	#endif
		Draw_Character (x+8 , y, num[0]);
		Draw_Character (x+16 , y, num[1]);
		Draw_Character (x+24 , y, num[2]);

		if (k == cl.viewentity - 1)
			Draw_Character (x - 8, y, 12);

	// draw name
		Draw_String (x+64, y, s->name);

		y += 10;
	}
}

/*
==================
Sbar_MiniDeathmatchOverlay
==================
*/
void Sbar_MiniDeathmatchOverlay (int sbarlines)
{
	int		i, k, l, top, bottom, x, y, f, numlines;
	char		num[12];
	scoreboard_t	*s;

	if (vid.width < 512 || !sbarlines)
		return;

//	scr_copyeverything = 1;
//	scr_fullupdate = 0;

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;
	y = vid.height - sbarlines*SBAR_HEIGHT;
	numlines = sbarlines*SBAR_HEIGHT/8;
	if (numlines < 3)
		return;

	// find us
	for (i=0 ; i<scoreboardlines ; i++)
		if (fragsort[i] == cl.viewentity - 1)
			break;

	i = (i == scoreboardlines) ? 0 : i - numlines/2;
	i = bound(0, i, scoreboardlines - numlines);

	x = 324;
	for ( ; i < scoreboardlines && y < vid.height - 8 ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill (x, y+1, 40, 3, top);
		Draw_Fill (x, y+4, 40, 4, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Draw_Character (x+8 , y, num[0]);
		Draw_Character (x+16 , y, num[1]);
		Draw_Character (x+24 , y, num[2]);

		if (k == cl.viewentity - 1)
		{
			Draw_Character (x, y, 16);
			Draw_Character (x + 32, y, 17);
		}

	// draw name
		Draw_String (x+48, y, s->name);

		y += 8;
	}
}

/*
==================
Sbar_FormatTime
==================
*/
const char * Sbar_FormatTime (double time, qboolean use_tenths)
{
	static char buf[16];
	int	mins, secs, tens, len;

	mins = time / 60;
	secs = time - 60*mins;

	len = Q_snprintfz (buf, sizeof(buf), "%i:%02i", mins, secs);
	if (use_tenths)
	{
		tens = (int)(time * 10) % 10;
		Q_snprintfz (buf+len, sizeof(buf)-len, ":%i", tens);
	}

	return buf;
}

/*
==================
Sbar_DrawTime
==================
*/
/*
#define SBTIME_TENTHS 0x0001
#define SBTIME_TIGHT  0x0002
void Sbar_DrawTime (double time, int right, int top, int flags)
{
	int charwidth, len, i;

	charwidth = (flags & SBTIME_TIGHT) ? DRAW_BIGNUMWIDTH-2 : DRAW_BIGNUMWIDTH;
	len = strlen (str);

	for (i = len-1; i >= 0; i--)
	{
		if (str[i] == ':')
		{
			right -= DRAW_BIGNUMWIDTH/2;
			Draw_TransPic (right, top, sb_colon);
		}
		else
		{
			right -= charwidth;
			Draw_TransPic (right, top, sb_nums[0][str[i]-'0']);
		}
	}
}
*/
/*
void Sbar_DrawTime (double time, int x, int y, int flags)
{
	int	width, mins, secs, tens;

	mins = time/60;
	Sbar_IntermissionNumber (x, y, mins, 3, 0);

	x += 3*SB_NUMWIDTH;
	if (flags & SBTIME_TIGHT)
	{
		width = SB_NUMWIDTH-2;
	}
	else
	{
		width = SB_NUMWIDTH;
		x += 2;
	}

	secs = time - mins*60;
	Draw_TransPic (x, y, sb_colon);
	Draw_TransPic (x += SB_NUMWIDTH/2, y, sb_nums[0][secs/10]);
	Draw_TransPic (x += width, y, sb_nums[0][secs%10]);

	if (flags & SBTIME_TENTHS)
	{
		tens = (int)(time * 10) % 10;
		Draw_TransPic (x += width, y, sb_colon);
		Draw_TransPic (x += SB_NUMWIDTH/2, y, sb_nums[0][tens]);
	}
}
*/
void Sbar_DrawStat (int num, int total, int x, int y)
{
/*	char buf[16];

	Q_snprintfz (buf, sizeof(buf), "%d/%d", num, total);
	Draw_BigNumString (x+152, y, buf, DRAWNUM_ALIGNRIGHT);
	return;
*/
	Sbar_IntermissionNumber (x, y, num, 3, false);
	x += 3*DRAW_BIGCHARWIDTH;

	Draw_BigNumString (x, y, "/", 0);		//Draw_TransPic (x, y, sb_slash);
	Sbar_IntermissionNumber (x + 8, y, total, 3, false);
}

/*
==================
Sbar_IntermissionOverlay
==================
*/
void Sbar_IntermissionOverlay (void)
{
	mpic_t	*pic;
	int	xofs, yofs;
	const char *str;

//	scr_copyeverything = 1;
//	scr_fullupdate = 0;

	if (cl.gametype == GAME_DEATHMATCH)
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	xofs = (vid.width - 320) >> 1;
	yofs = (vid.height - 240) >> 1;

	pic = Draw_GetCachePic ("gfx/inter.lmp", false);
	if (pic && pic->width == 320 && pic->height == 200)
	{
	// JDH: inter.lmp in Quake Beta is 320x200 and includes "Completed" plaque
		Draw_TransPic (xofs, yofs, pic);
		yofs += 56;
	}
	else
	{
		mpic_t *pic2 = Draw_GetCachePic ("gfx/complete.lmp", false);
		Draw_Pic (xofs + 64, yofs + 16, pic2);		// +16 was originally +24

		yofs += 64;		// was 56
		Draw_TransPic (xofs, yofs, pic);
	}


	// time
//	Sbar_DrawTime (cl.completed_time, xofs + 160, yofs + 8, 0);
//	Sbar_DrawTime (cl.completed_time, xofs + 240 + 3*DRAW_BIGNUMWIDTH, yofs + 8, 0);
	str = Sbar_FormatTime (cl.completed_time, false);
	Draw_BigNumString (xofs + 240 + 3*DRAW_BIGCHARWIDTH, yofs + 8, str, DRAWNUM_ALIGNRIGHT);
		// originally was xofs + 160; I changed to right-justified

	// secrets
	Sbar_DrawStat (cl.stats[STAT_SECRETS], cl.stats[STAT_TOTALSECRETS], xofs + 160, yofs + 48);

	// monsters
	Sbar_DrawStat (cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS], xofs + 160, yofs + 88);
/*
	// secrets
	Sbar_IntermissionNumber (xofs + 160, yofs + 48, cl.stats[STAT_SECRETS], 3, 0);
	Draw_TransPic (xofs + 232, yofs + 48, sb_slash);
	Sbar_IntermissionNumber (xofs + 240, yofs + 48, cl.stats[STAT_TOTALSECRETS], 3, 0);

	// monsters
	Sbar_IntermissionNumber (xofs + 160, yofs + 88, cl.stats[STAT_MONSTERS], 3, 0);
	Draw_TransPic (xofs + 232, yofs + 88, sb_slash);
	Sbar_IntermissionNumber (xofs + 240, yofs + 88, cl.stats[STAT_TOTALMONSTERS], 3, 0);
*/
}


/*
==================
Sbar_FinaleOverlay
==================
*/
void Sbar_FinaleOverlay (void)
{
	mpic_t	*pic;

//	scr_copyeverything = 1;

	pic = Draw_GetCachePic ("gfx/finale.lmp", false);
	if (pic)
		Draw_TransPic ((vid.width-pic->width)/2, 16, pic);
}

#endif		//#ifndef RQM_SV_ONLY
