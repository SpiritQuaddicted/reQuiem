
#include "quakedef.h"

#ifndef RQM_SV_ONLY

#ifdef HEXEN2_SUPPORT

// Function name changes from original code:
//  Sbar_DrawPic      --> Sbar_DrawPic_H2
//  Sbar_DrawTransPic --> Sbar_DrawPic_H2 (both funcs end up calling Draw_Pic)

#define BAR_TOP_HEIGHT				46.0	// standard status bar
#define BAR_BOTTOM_HEIGHT			98.0	// extended status bar w/ inventory (+showinfo)
#define BAR_TOTAL_HEIGHT			(BAR_TOP_HEIGHT+BAR_BOTTOM_HEIGHT)
#define BAR_BUMP_HEIGHT				23.0

#define	INV_MAX_CNT					15		// Max inventory array size
#define INV_MAX_ICON				 6		// Max number of inventory icons to display at once
#define INV_DISPLAY_TIME			 4

#define RING_FLIGHT					1
#define RING_WATER					2
#define RING_REGENERATION			4
#define RING_TURNING				8

extern int			sb_updates;		// if >= vid.numpages, no update needed

extern int			*pr_string_index;
extern char			*pr_global_strings;
extern int			pr_string_count;

extern qboolean		intro_playing;
extern int			in_impulse;
extern cvar_t		scr_hudspeed;
extern cvar_t		cl_playerclass;
extern const char	*ClassNames[NUM_CLASSES];

static float BarHeight;
static float BarTargetHeight;
static float ChainPosition = 0;
static float InventoryHideTime;

int			inv_order[INV_MAX_CNT];
int			inv_count, inv_startpos, inv_selected;

static qboolean inv_flg;					// true - show inventory interface


static qboolean sb_ShowInfo;
static qboolean sb_ShowDM;	/******* FINISH ME!! *******/


static int AmuletAC[NUM_CLASSES] =
{
	8,		// Paladin
	4,		// Crusader
	2,		// Necromancer
	6,		// Assassin
	6		// Demoness
};

static int BracerAC[NUM_CLASSES] =
{
	6,		// Paladin
	8,		// Crusader
	4,		// Necromancer
	2,		// Assassin
	2		// Demoness
};

static int BreastplateAC[NUM_CLASSES] =
{
	2,		// Paladin
	6,		// Crusader
	8,		// Necromancer
	4,		// Assassin
	4		// Demoness
};

static int HelmetAC[NUM_CLASSES] =
{
	4,		// Paladin
	2,		// Crusader
	6,		// Necromancer
	8,		// Assassin
	8		// Demoness
};

#define SB_MAXFRAME    16
#define SB_MAXARTIFACT 14

typedef struct 
{
	const char *basename;
	mpic_t **pics;
} sb_piclist_t;

typedef enum 
{
	SBLMP_TOPBAR1, SBLMP_TOPBAR2,  SBLMP_TOPBUMPL, SBLMP_TOPBUMPM, SBLMP_TOPBUMPR, SBLMP_BTMBAR1,  SBLMP_BTMBAR2, 
	SBLMP_HPGEM,   SBLMP_HPCHAIN,  SBLMP_CHNLCOV,  SBLMP_CHNRCOV, 
	SBLMP_GMANA,   SBLMP_GMANACOV, SBLMP_GMMANA,   SBLMP_BMANA,    SBLMP_BMANACOV, SBLMP_BMMANA, 
	SBLMP_ARMOR1,  SBLMP_ARMOR2,   SBLMP_ARMOR3,   SBLMP_ARMOR4,   SBLMP_ARTISEL, 
	SBLMP_RING_F,  SBLMP_RING_R,   SBLMP_RING_T,   SBLMP_RING_W,   SBLMP_RINGHLTH, SBLMP_RHLTHCVR, SBLMP_RHLTHCV2,
    /*-----*/ SB_NUMMISCLMPS, /*-----*/
	SBLMP_RNGFLYx, SBLMP_RNGTRNx,  SBLMP_RNGWTRx,  SBLMP_PWRBOOKx, SBLMP_DURHSTx,  SBLMP_DURSHDx,
	SBLMP_CPORTx,  SBLMP_ARTINUMx, SBLMP_ARTIx,    /*-----*/ SB_NUMLMPS /*-----*/
} sb_piclmp_t;


static mpic_t *sb_pics_misc[SB_NUMMISCLMPS];
static mpic_t *sb_pics_rngfly[SB_MAXFRAME+1];		/*  framenum 1..16 (0 unused) */
static mpic_t *sb_pics_rngtrn[SB_MAXFRAME+1];
static mpic_t *sb_pics_rngwtr[SB_MAXFRAME+1];
static mpic_t *sb_pics_pwrbook[SB_MAXFRAME+1];
static mpic_t *sb_pics_durhst[SB_MAXFRAME+1];
static mpic_t *sb_pics_durshd[SB_MAXFRAME+1];
static mpic_t *sb_pics_cport[NUM_CLASSES+1];		/* playerclass 1..5 (0 unused) */
static mpic_t *sb_pics_artinum[10];					/* digits 0..9 */
static mpic_t *sb_pics_arti[SB_MAXARTIFACT+1];		/* 00..14 */

static sb_piclist_t sb_pics[] = 
{
	{"topbar1"}, {"topbar2"},  {"topbumpl"}, {"topbumpm"}, {"topbumpr"}, {"btmbar1"},  {"btmbar2"}, 
	{"hpgem"},   {"hpchain"},  {"chnlcov"},  {"chnrcov"}, 
	{"gmana"},   {"gmanacov"}, {"gmmana"},   {"bmana"},    {"bmanacov"}, {"bmmana"},
	{"armor1"},  {"armor2"},   {"armor3"},   {"armor4"},   {"artisel"}, 
	{"ring_f"},  {"ring_r"},   {"ring_t"},   {"ring_w"},   {"ringhlth"}, {"rhlthcvr"}, {"rhlthcv2"},
	{NULL},
	{"rngfly%d",  sb_pics_rngfly},  {"rngtrn%d",  sb_pics_rngtrn},  {"rngwtr%d", sb_pics_rngwtr}, 
	{"pwrbook%d", sb_pics_pwrbook}, {"durhst%d",  sb_pics_durhst},  {"durshd%d", sb_pics_durshd}, 
	{"cport%d",   sb_pics_cport},   {"artinum%d", sb_pics_artinum}, {"arti%02d", sb_pics_arti}
};

/*
	"gfx/puzzle/%s.lmp", cl.puzzle_pieces[i]);	dozens!

	"gfx/rng%s%d.lmp", name, frame);		trn, wtr, fly; 1..16
	"gfx/%s%d.lmp", name, frame);			pwrbook, durhst, durshd; 1..16
	"gfx/cport%d.lmp", playerClass);		1..5
	"gfx/artinum%c.lmp";					0..9
	"gfx/arti%02d.lmp", artifact)));		00..14
*/
/*
===============
Sbar_Hexen2_Init
===============
*/
void Sbar_Hexen2_Init (void)
{
	BarHeight = BarTargetHeight = BAR_TOP_HEIGHT;
//	BarHeight = BAR_TOP_HEIGHT;

	assert (sizeof(sb_pics)/sizeof(sb_pics[0]) == SB_NUMLMPS);
}

mpic_t * Sbar_GetPwrbook15 (const char *name)
{
// JDH: this pic has transparent pixels in the middle, which I believe
//      to be a mistake
	qpic_t *pic;
	
	pic = (qpic_t *) COM_LoadTempFile (name, 0);
	if (!pic)
		return NULL;
	
	if ((pic->width == 32) && (pic->height == 32) && 
		(CRC_Block(pic->data, pic->width*pic->height) == 0x83D2))
	{
		int y, x;

		for (y = 6; y < 30; y++)
			for (x = 5; x < 30; x++)
				if (pic->data[y*32+x] == 255)
					pic->data[y*32+x] = 31;			// same shade, but non-transparent
	}

	return Draw_AddCachePic (name, pic->data, pic->width, pic->height);	
}

mpic_t * Sbar_GetPic2 (sb_piclmp_t lmpindex, int num)
{
	mpic_t **piclist;
	const char *base;
	char lmpname[MAX_QPATH];

	piclist = sb_pics[lmpindex].pics;
	if (!piclist)
	{
		piclist = sb_pics_misc;
		num = lmpindex;
	}
	
	if (!piclist[num])
	{	
		assert (sb_pics[lmpindex].basename != NULL);

		if (piclist == sb_pics_misc)
			base = sb_pics[lmpindex].basename;
		else
			base = va(sb_pics[lmpindex].basename, num);
		
		Q_snprintfz (lmpname, sizeof(lmpname), "gfx/%s.lmp", base);
		if ((lmpindex == SBLMP_PWRBOOKx) && (num == 15))
			piclist[num] = Sbar_GetPwrbook15 (lmpname);
		else
			piclist[num] = Draw_GetCachePic (lmpname, false);
	}

	return piclist[num];
}

mpic_t * Sbar_GetPic (const char *name)
{
	return Draw_GetCachePic (name, false);
}

#ifdef GLQUAKE

#define SB_BIGBARNAME "sbar_H2"
static mpic_t *sb_topbar, *sb_topbar_R;

qpic_t * NewPicFromHunk (int width, int height)
{
	int	total = width*height;
	qpic_t *pic;

	pic = Hunk_Alloc (sizeof(qpic_t) + total);
	pic->width = width;
	pic->height = height;
	memset (pic->data, 255, total);
	return pic;
}

/*
===============
Sbar_BuildH2Bar
  JDH: combines the 5 images that comprise the Hexen II status bar, thereby 
       avoiding visual glitches at seams (caused by filtering or scr_sbarsize)
===============
*/
void Sbar_BuildH2Bar (void)
{
/*  ________                    ________   */
/* |topbumpl|     ________     |topbumpr|  */
/* |________|____|topbumpm|____|________|  */
/* |                 |                  |  */
/* |    topbar1      |      topbar2     |  */
/* |_________________|__________________|  */

	qpic_t *pic[5], *newpic1, *newpic2, *currpic;
	int width, barheight, bumpheight, i;
	
	for (i = 0; i < 5; i++)
	{
		pic[i] = (qpic_t *) COM_LoadHunkFile (va("gfx/%s.lmp", sb_pics[i].basename), 0);
		if (!pic[i])
			return;
		SwapPic (pic[i]);
	}

	width = pic[0]->width + pic[1]->width;		// should be 320 (160*2)

	if (pic[2]->width + pic[3]->width + pic[4]->width > width)
		return;

	barheight = max(pic[0]->height, pic[1]->height);		// should be equal (46)

	bumpheight = max(pic[2]->height, pic[4]->height);		// should be equal (23)
	bumpheight = max(bumpheight, pic[3]->height);

	if (width > (int)gl_max_size.value)
	{
		newpic2 = NewPicFromHunk (pic[1]->width, barheight + bumpheight);
//		width = pic[0]->width + 2;		// extra 2 to duplicate pixels from right half
		width = pic[0]->width;
	}
	else
	{
		newpic2 = NULL;
	}

	newpic1 = NewPicFromHunk (width, barheight + bumpheight);

// pic[0]: left bar
	Draw_CopyPicToPic (pic[0], newpic1, 0, newpic1->height - pic[0]->height);

// pic[2]: left bump
	Draw_CopyPicToPic (pic[2], newpic1, 0, newpic1->height - pic[0]->height - pic[2]->height);


	currpic = (newpic2 ? newpic2 : newpic1);

// pic[1]: right bar
	Draw_CopyPicToPic (pic[1], currpic, currpic->width - pic[1]->width, currpic->height - pic[1]->height);

// pic[4]: right bump
	Draw_CopyPicToPic (pic[4], currpic, currpic->width - pic[4]->width, currpic->height - pic[1]->height - pic[4]->height);

// pic[3]: middle bump  (extra -3 is due to design quirk)
	Draw_CopyPicToPic (pic[3], newpic1, pic[0]->width - pic[3]->width/2 - 3, newpic1->height - barheight - pic[3]->height);

	if (newpic2)
	{
	// right half of middle bump
		Draw_CopyPicToPic (pic[3], newpic2, -pic[3]->width/2 - 3, newpic2->height - barheight - pic[3]->height);

	// duplicate first 2 columns of pixels from right half to end of left half (avoids seams)
//		Draw_CopyPicToPic (pic[1], newpic1, newpic1->width - 2, newpic1->height - pic[0]->height);

		sb_topbar_R = Draw_AddCachePic (SB_BIGBARNAME"2", newpic2->data, newpic2->width, newpic2->height);	
	}
	else sb_topbar_R = NULL;

	sb_topbar = Draw_AddCachePic (SB_BIGBARNAME, newpic1->data, newpic1->width, newpic1->height);	
}
#endif		//#ifdef GLQUAKE

/*
===============
Sbar_LoadWadPics_H2
===============
*/
void Sbar_LoadWadPics_H2 (void)
{
#ifdef GLQUAKE
	int mark;

	if (Draw_FindCachePic (SB_BIGBARNAME))
		return;

	mark = Hunk_LowMark ();
	Sbar_BuildH2Bar ();
	Hunk_FreeToLowMark (mark);
#endif
}

/*
===============
Sbar_InvUpdate
===============
*/
void Sbar_InvUpdate (qboolean force)
{
	if (inv_flg || force)
	{
		// Just to be safe
		if (inv_selected >= 0 && inv_count > 0)
			cl.v.inventory = inv_order[inv_selected] + 1;
		else
			cl.v.inventory = 0;

		if (!force) 
		{
//			scr_fullupdate = 0;
			inv_flg = false;  // Toggle menu off
		}

		// This will cause the server to set the client's edict's inventory value
		MSG_WriteByte (&cls.message, clc_inv_select);
		MSG_WriteByte (&cls.message, cl.v.inventory);
	}
}


/*
===============
Sbar_InvLeft_f
===============
*/
void Sbar_InvLeft_f (cmd_source_t src)
{
	if (!inv_count || cl.intermission)
	{
		return;
	}

	if (inv_flg)
	{
		if (inv_selected > 0)
		{
			inv_selected--;
			if (inv_selected < inv_startpos)
			{
				inv_startpos = inv_selected;
			}
//			scr_fullupdate = 0;
		}
	}
	else
	{
		inv_flg = true;
	}

	S_LocalSound ("misc/invmove.wav");
	InventoryHideTime = realtime + INV_DISPLAY_TIME;		// JDH: was cl.time
}

/*
===============
Sbar_InvRight_f
===============
*/
void Sbar_InvRight_f (cmd_source_t src)
{
	if (!inv_count || cl.intermission)
	{
		return;
	}

	if (inv_flg)
	{
		if (inv_selected < inv_count-1)
		{
			inv_selected++;
			if (inv_selected - inv_startpos >= INV_MAX_ICON)
			{
				// could probably be just a inv_startpos++, but just in case
				inv_startpos = inv_selected - INV_MAX_ICON + 1;
			}
//			scr_fullupdate = 0;
		}
	}
	else
	{
		inv_flg = true;
	}

	S_LocalSound ("misc/invmove.wav");
	InventoryHideTime = realtime + INV_DISPLAY_TIME;	// JDH: was cl.time
}

/*
===============
Sbar_InvUse_f
===============
*/
void Sbar_InvUse_f (cmd_source_t src)
{
	if (!inv_count || cl.intermission)
	{
		return;
	}

	S_LocalSound ("misc/invuse.wav");
	Sbar_InvUpdate(true);
	inv_flg = false;
//	scr_fullupdate = 0;
	in_impulse = 23;
}

/*
===============
Sbar_InvOff_f
===============
*/
void Sbar_InvOff_f (cmd_source_t src)
{
	inv_flg = false;
//	scr_fullupdate = 0;
}

/*
===============
Sbar_ShowDMDown_f
===============
*/
void Sbar_ShowDMDown_f (cmd_source_t src)
{
	sb_ShowDM = true;
}

/*
===============
Sbar_ShowDMUp_f
===============
*/
void Sbar_ShowDMUp_f (cmd_source_t src)
{
	sb_ShowDM = false;
}

/*
===============
Sbar_ToggleDM_f    -- FIXME --
===============
*/
void Sbar_ToggleDM_f (cmd_source_t src)
{
/*	DMMode.value += 1;
	if (DMMode.value > 2)
		DMMode.value = 0;
*/
}

/*
===============
Sbar_ShowInfoDown_f
===============
*/
void Sbar_ShowInfoDown_f (cmd_source_t src)
{
	if (sb_ShowInfo || cl.intermission)
	{
		return;
	}
	S_LocalSound ("misc/barmovup.wav");
	BarTargetHeight = BAR_TOTAL_HEIGHT;
	sb_ShowInfo = true;
	sb_updates = 0;
}

/*
===============
Sbar_ShowInfoUp_f
===============
*/
void Sbar_ShowInfoUp_f (cmd_source_t src)
{
	if (cl.intermission || (scr_viewsize.value >= scr_viewsize.maxvalue-10))
	{
		BarTargetHeight = -BAR_BUMP_HEIGHT;
	}
	else
	{
		BarTargetHeight = BAR_TOP_HEIGHT;
	}

	S_LocalSound ("misc/barmovdn.wav");
	sb_ShowInfo = false;
	sb_updates = 0;
}

/*
===============
Sbar_DrawPic_H2
===============
*/
void Sbar_DrawPic_H2 (int x, int y, const mpic_t *pic)
{
	Draw_Pic (x + ((vid.width-320)/2), y + (vid.height-(int)BarHeight), pic);
}

/*
===============
Sbar_DrawNum_H2
===============
*/
void Sbar_DrawNum_H2 (int x, int y, int num, int digits)
{
	char	str[12], *ptr;
	int	l, frame;

extern mpic_t		*draw_bignums[2][11];

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += ((digits-l)*13)/2;

	while (*ptr)
	{
		frame = (*ptr == '-') ? 11 : *ptr -'0';

		Sbar_DrawPic_H2 (x, y, draw_bignums[0][frame]);
		x += 13;
		ptr++;
	}
}

/*
====================
Sbar_DrawSmallString
====================
*/
void Sbar_DrawSmallString (int x, int y, const char *str)
{
	Draw_SmallString(x + (vid.width-320)/2, y+vid.height-(int)BarHeight, str);
}

/*
===============
Sbar_SetChainPosition
===============
*/
qboolean Sbar_SetChainPosition (float health, float maxHealth)
{
	float delta;
	float chainTargetPosition;

	if (health < 0)
	{
		health = 0;
	}
	else if (health > maxHealth)
	{
		health = maxHealth;
	}
	chainTargetPosition = (health*195)/maxHealth;
	if (fastfabs(ChainPosition-chainTargetPosition) < 0.1)
	{
		return false;
	}
	if (ChainPosition < chainTargetPosition)
	{
		delta = ((chainTargetPosition-ChainPosition)*5)*host_frametime;
		if(delta < 0.5)
		{
			delta = 0.5;
		}
		ChainPosition += delta;
		if (ChainPosition > chainTargetPosition)
		{
			ChainPosition = chainTargetPosition;
		}
	}
	else if (ChainPosition > chainTargetPosition)
	{
		delta = ((ChainPosition-chainTargetPosition)*5)*host_frametime;
		if (delta < 0.5)
		{
			delta = 0.5;
		}
		ChainPosition -= delta;
		if (ChainPosition < chainTargetPosition)
		{
			ChainPosition = chainTargetPosition;
		}
	}
	return true;
}

/*
===============
Sbar_GetManaStr
===============
*/
const char * Sbar_GetManaStr (int mana, int maxMana, int *outMana)
{
	static char tempStr[16];

	mana = bound(0, mana, maxMana);
	sprintf (tempStr, "%03d", mana);
	
	if (outMana) *outMana = mana;
	return tempStr;
}

/*
===============
Sbar_DrawRing
===============
*/
//static void Sbar_DrawRing (int x, int val, char suffix, char suffix2)
static void Sbar_DrawRing (int x, int val, sb_piclmp_t ringpic1, sb_piclmp_t ringpic2)
{
//	Sbar_DrawPic_H2(x, 119, Sbar_GetPic(va("gfx/ring_%c.lmp", suffix)));
	Sbar_DrawPic_H2(x, 119, Sbar_GetPic2(ringpic1, -1));

	if (val > 100)
		val = 100;
	
//	Sbar_DrawPic_H2( x+29 - (int)(26 * (val/100.0)), 142, Sbar_GetPic("gfx/ringhlth.lmp"));
	Sbar_DrawPic_H2( x+29 - (int)(26 * (val/100.0)), 142, Sbar_GetPic2(SBLMP_RINGHLTH, -1));
//	Sbar_DrawPic_H2( x+29, 142, Sbar_GetPic(va("gfx/rhlthcv%c.lmp", suffix2)));
	Sbar_DrawPic_H2( x+29, 142, Sbar_GetPic2(ringpic2, -1));
}
	
/*
===============
Sbar_DrawArmor
===============
*/
static void Sbar_DrawArmor (int x, int val, int suffix)
{
	char	tempStr[80];

//	Sbar_DrawPic_H2(x, 115, Sbar_GetPic(va("gfx/armor%d.lmp", suffix)));
	Sbar_DrawPic_H2(x, 115, Sbar_GetPic2(SBLMP_ARMOR1 + suffix-1, -1));
	sprintf(tempStr, "+%d", val);
	Sbar_DrawSmallString(x+3, 136, tempStr);
}

/*
===============
Sbar_DrawArtifactNumber
===============
*/
static void Sbar_DrawArtifactNumber(int x, int y, int number)
{
//	static char artiNumName[18] = "gfx/artinum0.lmp";
	int digit;

	if (number >= 10)
	{
//		artiNumName[11] = '0'+(number%100)/10;
//		Sbar_DrawPic_H2(x, y, Sbar_GetPic(artiNumName));
		digit = (number%100)/10;
		Sbar_DrawPic_H2(x, y, Sbar_GetPic2(SBLMP_ARTINUMx, digit));
	}
	
//	artiNumName[11] = '0'+number%10;
//	Sbar_DrawPic_H2(x+4, y, Sbar_GetPic(artiNumName));
	digit = number%10;
	Sbar_DrawPic_H2(x+4, y, Sbar_GetPic2(SBLMP_ARTINUMx, digit));
}

/*
===============
Sbar_DrawArtifact
===============
*/
static void Sbar_DrawArtifact(int x, int y, int artifact)
{
	int count;

	if ((artifact<0) || (artifact>SB_MAXARTIFACT))
		return;
	
//	Sbar_DrawPic_H2(x, y, Sbar_GetPic(va("gfx/arti%02d.lmp", artifact)));
	Sbar_DrawPic_H2(x, y, Sbar_GetPic2(SBLMP_ARTIx, artifact));
	
	count = (int) (&cl.v.cnt_torch)[artifact];
	if (count > 0)
	{
		Sbar_DrawArtifactNumber(x+20, y+21, count);
	}
}

/*
===============
Sbar_DrawArtifactInventory
===============
*/
static void Sbar_DrawArtifactInventory(void)
{
	int i;
	int x, y;

	if (InventoryHideTime < realtime)	// JDH: was cl.time
	{
		Sbar_InvUpdate(false);
		return;
	}
	if (!inv_flg)
	{
		return;
	}
	if (!inv_count)
	{
		Sbar_InvUpdate(false);
		return;
	}

	if (BarHeight < 0)
	{
		y = BarHeight-34;
	}
	else
	{
		y = -37;
	}

	for (i = 0, x = 64; i < INV_MAX_ICON; i++, x += 33)
	{
		if (inv_startpos+i >= inv_count)
		{
			break;
		}
		if (inv_startpos+i == inv_selected)
		{ // Highlight icon
//			Sbar_DrawPic_H2(x+9, y-12, Sbar_GetPic("gfx/artisel.lmp"));
			Sbar_DrawPic_H2(x+9, y-12, Sbar_GetPic2(SBLMP_ARTISEL, -1));
		}
		Sbar_DrawArtifact(x, y, inv_order[inv_startpos+i]);
	}
}

/*
===============
Sbar_DrawActiveArtifact
===============
*/
//static void Sbar_DrawActiveArtifact (const char *name, int *art_col)
static void Sbar_DrawActiveArtifact (sb_piclmp_t basepic, int *art_col)
{
	int frame;
//	char tempStr[24];
	
	frame = 1 + ((int)(cl.time*SB_MAXFRAME) % SB_MAXFRAME);
//	Q_snprintfz (tempStr, sizeof(tempStr), "gfx/%s%d.lmp", name, frame);
//	Draw_TransPic(vid.width - *art_col, 1, Sbar_GetPic(tempStr));
	Draw_TransPic (vid.width - *art_col, 1, Sbar_GetPic2(basepic, frame));
	*art_col += 50;
}

/*
===============
Sbar_DrawActiveArtifacts
===============
*/
static void Sbar_DrawActiveArtifacts (int ring_row)
{
	int art_col, flag;

	if (scr_con_current == vid.height)
		return;

	art_col = 50;

	if (ring_row != 1)
		art_col += 50;

	flag = (int) cl.v.artifact_active;
	if (flag & ART_TOMEOFPOWER)
	{
//		Sbar_DrawActiveArtifact( "pwrbook", &art_col );
		Sbar_DrawActiveArtifact (SBLMP_PWRBOOKx, &art_col);
	}

	if (flag & ART_HASTE)
	{
//		Sbar_DrawActiveArtifact( "durhst", &art_col );
		Sbar_DrawActiveArtifact (SBLMP_DURHSTx, &art_col);
	}

	if (flag & ART_INVINCIBILITY)
	{
//		Sbar_DrawActiveArtifact( "durshd", &art_col );
		Sbar_DrawActiveArtifact (SBLMP_DURSHDx, &art_col);
	}
}

/*
===============
Sbar_DrawActiveRing
===============
*/
//static void Sbar_DrawActiveRing (const char *name, int *ring_row)
static void Sbar_DrawActiveRing (sb_piclmp_t basepic, int *ring_row)
{
	int frame;
//	char tempStr[24];
	
	frame = 1 + ((int)(cl.time*SB_MAXFRAME) % SB_MAXFRAME);
//	Q_snprintfz (tempStr, sizeof(tempStr), "gfx/%s%d.lmp", name, frame);
//	Draw_TransPic (vid.width - 50, *ring_row, Sbar_GetPic(tempStr));
	Draw_TransPic (vid.width - 50, *ring_row, Sbar_GetPic2(basepic, frame));
	*ring_row += 33;
}

/*
===============
Sbar_DrawActiveRings
===============
*/
static int Sbar_DrawActiveRings (void)
{
	int ring_row, flag;

	if (scr_con_current == vid.height)
		return 1;		// console is full screen

	ring_row = 1;
	flag = (int) cl.v.rings_active;

	if (flag & RING_TURNING)
	{
//		Sbar_DrawActiveRing( "rngtrn", &ring_row );
		Sbar_DrawActiveRing (SBLMP_RNGTRNx, &ring_row);
	}

	if (flag & RING_WATER)
	{
//		Sbar_DrawActiveRing( "rngwtr", &ring_row );
		Sbar_DrawActiveRing (SBLMP_RNGWTRx, &ring_row);
	}

	if (flag & RING_FLIGHT)
	{
//		Sbar_DrawActiveRing( "rngfly", &ring_row );
		Sbar_DrawActiveRing (SBLMP_RNGFLYx, &ring_row);
	}

	return ring_row;
}

/*
===============
Sbar_DrawLowerBar
===============
*/
static void Sbar_DrawLowerBar(void)
{
	int		i;
	char	tempStr[MAX_QPATH];
	int		playerClass;
	int		piece;

	playerClass = cl_playerclass.value;
	if (playerClass < 1 || playerClass > NUM_CLASSES)
	{
		playerClass = 1;	// default to Paladin profile
	}

	// Backdrop
//	Sbar_DrawPic_H2(0, 46, Sbar_GetPic("gfx/btmbar1.lmp"));
//	Sbar_DrawPic_H2(160, 46, Sbar_GetPic("gfx/btmbar2.lmp"));
	Sbar_DrawPic_H2(0, 46, Sbar_GetPic2(SBLMP_BTMBAR1, -1));
	Sbar_DrawPic_H2(160, 46, Sbar_GetPic2(SBLMP_BTMBAR2, -1));

	// Stats
	Sbar_DrawSmallString(11, 48, ClassNames[playerClass-1]);

	Sbar_DrawSmallString(11, 58, "int");
	sprintf(tempStr, "%02d", (int)cl.v.intelligence);
	Sbar_DrawSmallString(33, 58, tempStr);

	Sbar_DrawSmallString(11, 64, "wis");
	sprintf(tempStr, "%02d", (int)cl.v.wisdom);
	Sbar_DrawSmallString(33, 64, tempStr);

	Sbar_DrawSmallString(11, 70, "dex");
	sprintf(tempStr, "%02d", (int)cl.v.dexterity);
	Sbar_DrawSmallString(33, 70, tempStr);

	Sbar_DrawSmallString(58, 58, "str");
	sprintf(tempStr, "%02d", (int)cl.v.strength);
	Sbar_DrawSmallString(80, 58, tempStr);

	Sbar_DrawSmallString(58, 64, "lvl");
	sprintf(tempStr, "%02d", (int)cl.v.level);
	Sbar_DrawSmallString(80, 64, tempStr);

	Sbar_DrawSmallString(58, 70, "exp");
	sprintf(tempStr, "%06d", (int)cl.v.experience);
	Sbar_DrawSmallString(80, 70, tempStr);

	// Abilities
	Sbar_DrawSmallString (11, 79, "abilities");
	i = ABILITIES_STR_INDEX + (playerClass-1)*2;
	if (i+1 < pr_string_count)
	{
		if (((int)cl.v.flags) & FL_SPECIAL_ABILITY1)
		{
			Sbar_DrawSmallString(8, 89, &pr_global_strings[pr_string_index[i]]);
		}
		if (((int)cl.v.flags) & FL_SPECIAL_ABILITY2)
		{
			Sbar_DrawSmallString(8, 96, &pr_global_strings[pr_string_index[i+1]]);
		}
	}

	// Portrait
//	Q_snprintfz (tempStr, sizeof(tempStr), "gfx/cport%d.lmp", playerClass);
//	Sbar_DrawPic_H2(134, 50, Sbar_GetPic(tempStr));
	Sbar_DrawPic_H2(134, 50, Sbar_GetPic2(SBLMP_CPORTx, playerClass));

	// Armor
	if (cl.v.armor_helmet > 0)
	{
		Sbar_DrawArmor(164, cl.v.armor_helmet, 1);
	}
	if (cl.v.armor_amulet > 0)
	{
		Sbar_DrawArmor(205, cl.v.armor_amulet, 2);
	}
	if (cl.v.armor_breastplate > 0)
	{
		Sbar_DrawArmor(246, cl.v.armor_breastplate, 3);
	}
	if (cl.v.armor_bracer > 0)
	{
		Sbar_DrawArmor(285, cl.v.armor_bracer, 4);
	}

	// Rings 
	if (cl.v.ring_flight > 0)
	{
//		Sbar_DrawRing(6, cl.v.ring_flight, 'f', 'r');
		Sbar_DrawRing(6, cl.v.ring_flight, SBLMP_RING_F, SBLMP_RHLTHCVR);
	}

	if (cl.v.ring_water > 0)
	{
//		Sbar_DrawRing(44, cl.v.ring_water, 'w', 'r');
		Sbar_DrawRing(44, cl.v.ring_water, SBLMP_RING_W, SBLMP_RHLTHCVR);
	}

	if (cl.v.ring_turning > 0)
	{
//		Sbar_DrawRing(81, cl.v.ring_turning, 't', 'r');
		Sbar_DrawRing(81, cl.v.ring_turning, SBLMP_RING_T, SBLMP_RHLTHCVR);
	}

	if (cl.v.ring_regeneration > 0)
	{
//		Sbar_DrawRing(119, cl.v.ring_regeneration, 'r', '2');
		Sbar_DrawRing(119, cl.v.ring_regeneration, SBLMP_RING_R, SBLMP_RHLTHCV2);
	}

	// Puzzle pieces
	piece = 0;
	for (i = 0; i < 8; i++)
	{
		if (cl.puzzle_pieces[i][0] == 0)
			continue;

		Q_snprintfz (tempStr, sizeof(tempStr), "gfx/puzzle/%s.lmp", cl.puzzle_pieces[i]);
		Sbar_DrawPic_H2 (194 + (piece%4)*31, 51 + (piece/4)*31, //(piece < 4 ? 51 : 82),
							Sbar_GetPic(tempStr));
		piece++;
	}
}

/*
===============
Sbar_CalcAC
===============
*/
int Sbar_CalcAC(void)
{
	int		playerClass, a;
	
	playerClass = cl_playerclass.value -1 ;
	if (playerClass < 0 || playerClass >= NUM_CLASSES)
	{
		playerClass = NUM_CLASSES-1;
	}

	a = 0;
	if (cl.v.armor_amulet > 0)
	{
		a += AmuletAC[playerClass];
		a += cl.v.armor_amulet/5;
	}
	if (cl.v.armor_bracer > 0)
	{
		a += BracerAC[playerClass];
		a += cl.v.armor_bracer/5;
	}
	if (cl.v.armor_breastplate > 0)
	{
		a += BreastplateAC[playerClass];
		a += cl.v.armor_breastplate/5;
	}
	if (cl.v.armor_helmet > 0)
	{
		a += HelmetAC[playerClass];
		a += cl.v.armor_helmet/5;
	}
	return a;
}

/*
===============
Sbar_DrawFullScreenInfo
===============
*/
void Sbar_DrawFullScreenInfo(void)
{
	int y, maxMana;
	const char *str;

	y = BarHeight-37;
//	Sbar_DrawPic_H2(3, y, Sbar_GetPic("gfx/bmmana.lmp"));
//	Sbar_DrawPic_H2(3, y+18, Sbar_GetPic("gfx/gmmana.lmp"));
	Sbar_DrawPic_H2(3, y, Sbar_GetPic2(SBLMP_BMMANA, -1));
	Sbar_DrawPic_H2(3, y+18, Sbar_GetPic2(SBLMP_GMMANA, -1));

	maxMana = cl.v.max_mana;
	
	// Blue mana
	str = Sbar_GetManaStr (cl.v.bluemana, maxMana, NULL);
	Sbar_DrawSmallString(10, y+6, str);

	// Green mana
	str = Sbar_GetManaStr (cl.v.greenmana, maxMana, NULL);
	Sbar_DrawSmallString (10, y+18+6, str);

	// HP
	Sbar_DrawNum_H2 (38, y+18, cl.v.health, 3);

	// Current inventory item
	if (inv_selected >= 0)
	{
		Sbar_DrawArtifact (288, y+7, inv_order[inv_selected]);
	}
}

/*
===============
Sbar_Draw_H2
===============
*/
void Sbar_Draw_H2 (void)
{
	const char	*str;
	int		mana, maxMana, ring_row;
	float	delta;

	if (intro_playing)
	{
//		scr_fullupdate = 0;
//		scr_copyeverything = 1;
		return;
	}

	if (scr_hudspeed.value > 0)
	{
		if (BarHeight != BarTargetHeight)
		{
			if (BarHeight < BarTargetHeight)
			{
				delta = ((BarTargetHeight-BarHeight)*scr_hudspeed.value) * host_frametime;
				delta = max( 0.5, delta );
				delta = min( BarTargetHeight - BarHeight, delta );
				BarHeight += delta;
			}
			else 
			{
				delta = ((BarHeight-BarTargetHeight)*scr_hudspeed.value) * host_frametime;
				delta = max( 0.5, delta );
				delta = min( BarHeight - BarTargetHeight, delta );
				BarHeight -= delta;
			}
//			scr_fullupdate = 0;
		}
	}
	else BarHeight = BarTargetHeight;

//	scr_copyeverything = 1;
	sb_updates++;

	if (BarHeight < 0)
	{
		Sbar_DrawFullScreenInfo();
		return;
	}

#ifdef _DEBUG
	cl.v.armor_helmet = 20;
	cl.v.armor_amulet = 30;
	cl.v.armor_breastplate = 40;
	cl.v.armor_bracer = 50;
	cl.v.ring_flight = 80;
	cl.v.ring_water = 60;
	cl.v.ring_turning = 40;
	cl.v.ring_regeneration = 20;
	strcpy (cl.puzzle_pieces[0], "cskey");
	strcpy (cl.puzzle_pieces[1], "keep1");
	strcpy (cl.puzzle_pieces[2], "keep2");
	strcpy (cl.puzzle_pieces[3], "keep3");
	strcpy (cl.puzzle_pieces[4], "mithl");
	if (inv_selected < 0)
	{
		inv_selected = 0;
		inv_order[inv_selected] = 0;
	}
	(&cl.v.cnt_torch)[0] = 3;
#endif
	
#ifdef GLQUAKE
	if (sb_topbar)
	{
		Sbar_DrawPic_H2 (0, (int)BAR_TOP_HEIGHT - sb_topbar->height, sb_topbar);
		if (sb_topbar_R)
			Sbar_DrawPic_H2 (160, (int)BAR_TOP_HEIGHT - sb_topbar_R->height, sb_topbar_R);
	}
	else
#endif
	{
//		Sbar_DrawPic_H2(0, 0, Sbar_GetPic("gfx/topbar1.lmp"));
//		Sbar_DrawPic_H2(160, 0, Sbar_GetPic("gfx/topbar2.lmp"));

//		Sbar_DrawPic_H2(0, -BAR_BUMP_HEIGHT, Sbar_GetPic ("gfx/topbumpl.lmp"));
//		Sbar_DrawPic_H2(138, -8, Sbar_GetPic("gfx/topbumpm.lmp"));
//		Sbar_DrawPic_H2(269, -BAR_BUMP_HEIGHT, Sbar_GetPic("gfx/topbumpr.lmp"));

		Sbar_DrawPic_H2(0, 0, Sbar_GetPic2(SBLMP_TOPBAR1, -1));
		Sbar_DrawPic_H2(160, 0, Sbar_GetPic2(SBLMP_TOPBAR2, -1));

		Sbar_DrawPic_H2(0, -BAR_BUMP_HEIGHT, Sbar_GetPic2(SBLMP_TOPBUMPL, -1));
		Sbar_DrawPic_H2(138, -8, Sbar_GetPic2(SBLMP_TOPBUMPM, -1));
		Sbar_DrawPic_H2(269, -BAR_BUMP_HEIGHT, Sbar_GetPic2(SBLMP_TOPBUMPR, -1));
	}

	maxMana = cl.v.max_mana;
	
	// Blue mana
	str = Sbar_GetManaStr (cl.v.bluemana, maxMana, &mana);
	Sbar_DrawSmallString(201, 22, str);
	if (mana)
	{
		Sbar_DrawPic_H2 (190, 26-(int)((mana*18.0)/(float)maxMana+0.5),
//			Sbar_GetPic("gfx/bmana.lmp"));
			Sbar_GetPic2(SBLMP_BMANA, -1));
//		Sbar_DrawPic_H2 (190, 27, Sbar_GetPic("gfx/bmanacov.lmp"));
		Sbar_DrawPic_H2 (190, 27, Sbar_GetPic2(SBLMP_BMANACOV, -1));
	}

	// Green mana
	str = Sbar_GetManaStr (cl.v.greenmana, maxMana, &mana);
	Sbar_DrawSmallString (243, 22, str);
	if (mana)
	{
		Sbar_DrawPic_H2 (232, 26-(int)((mana*18.0)/(float)maxMana+0.5),
//			Sbar_GetPic("gfx/gmana.lmp"));
			Sbar_GetPic2(SBLMP_GMANA, -1));
//		Sbar_DrawPic_H2 (232, 27, Sbar_GetPic("gfx/gmanacov.lmp"));
		Sbar_DrawPic_H2 (232, 27, Sbar_GetPic2(SBLMP_GMANACOV, -1));
	}

	// HP
	if (cl.v.health < -99)
		Sbar_DrawNum_H2(58, 14, -99, 3);
	else
		Sbar_DrawNum_H2(58, 14, cl.v.health, 3);
	
	Sbar_SetChainPosition(cl.v.health, cl.v.max_health);
//	Sbar_DrawPic_H2(45+((int)ChainPosition&7), 38, Sbar_GetPic("gfx/hpchain.lmp"));
//	Sbar_DrawPic_H2(45+(int)ChainPosition, 36, Sbar_GetPic("gfx/hpgem.lmp"));
//	Sbar_DrawPic_H2(43, 36, Sbar_GetPic("gfx/chnlcov.lmp"));
//	Sbar_DrawPic_H2(267, 36, Sbar_GetPic("gfx/chnrcov.lmp"));
	Sbar_DrawPic_H2(45+((int)ChainPosition&7), 38, Sbar_GetPic2(SBLMP_HPCHAIN, -1));
	Sbar_DrawPic_H2(45+(int)ChainPosition, 36, Sbar_GetPic2(SBLMP_HPGEM, -1));
	Sbar_DrawPic_H2(43, 36, Sbar_GetPic2(SBLMP_CHNLCOV, -1));
	Sbar_DrawPic_H2(267, 36, Sbar_GetPic2(SBLMP_CHNRCOV, -1));

	// AC
	Sbar_DrawNum_H2(105, 14, Sbar_CalcAC(), 2);

	if (BarHeight > BAR_TOP_HEIGHT)
	{
		Sbar_DrawLowerBar();
	}

	// Current inventory item
	if (inv_selected >= 0)
	{
		Sbar_DrawArtifact(144, 3, inv_order[inv_selected]);
	}

	Sbar_DrawArtifactInventory();

	ring_row = Sbar_DrawActiveRings();
	Sbar_DrawActiveArtifacts( ring_row );

/*	if (sb_ShowDM)
	{
		if (cl.gametype == GAME_DEATHMATCH)
			Sbar_DeathmatchOverlay();
		else
			Sbar_NormalOverlay();
	}
	else if (cl.gametype == GAME_DEATHMATCH && DMMode.value)
		Sbar_SmallDeathmatchOverlay();
*/
}

/*
==================
Sbar_InvReset
==================
*/
void Sbar_InvReset(void)
{
	inv_count = inv_startpos = 0;
	inv_selected = -1;
	inv_flg = false;
//	scr_fullupdate = 0;
}

/*
==================
Sbar_InvChanged
==================
*/
void Sbar_InvChanged(void)
{
	int counter, position;
	qboolean examined[INV_MAX_CNT];
	qboolean forceUpdate = false;

	memset(examined, 0, sizeof(examined)); // examined[x] = false

	if (inv_selected >= 0 && (&cl.v.cnt_torch)[inv_order[inv_selected]] == 0)
		forceUpdate = true;

	// removed items we no longer have from the order
	for (counter=position=0; counter<inv_count; counter++)
	{
		if ((&cl.v.cnt_torch)[inv_order[counter]] > 0)
		{
			inv_order[position] = inv_order[counter];
			examined[inv_order[position]] = true;

			position++;
		}
	}

	// add in the new items
	for (counter=0; counter<INV_MAX_CNT; counter++)
	{
		if (!examined[counter])
		{
			if ((&cl.v.cnt_torch)[counter] > 0)
			{
				inv_order[position] = counter;
				position++;
			}
		}
	}

	inv_count = position;
	if (inv_selected >= inv_count) 
	{
		inv_selected = inv_count-1;
		forceUpdate = true;
	}
	if (inv_count && inv_selected < 0) 
	{
		inv_selected = 0;
		forceUpdate = true;
	}
	if (forceUpdate)
	{
		Sbar_InvUpdate(true);
	}

	if (inv_startpos+INV_MAX_CNT > inv_count)
	{
		inv_startpos = inv_count - INV_MAX_CNT;
		if (inv_startpos < 0) inv_startpos = 0;
	}
}

/*
==================
Sbar_ViewSizeChanged
==================
*/
void Sbar_ViewSizeChanged(void)
{
	if (cl.intermission || scr_viewsize.value >= 110.0)
	{
		BarTargetHeight = 0.0-BAR_BUMP_HEIGHT;
		//BarHeight = 0.0-BAR_BUMP_HEIGHT;
	}
	else
	{
		BarTargetHeight = BAR_TOP_HEIGHT;
		//BarHeight = BAR_TOP_HEIGHT;
	}
}

#endif	// #ifdef HEXEN2_SUPPORT

#endif		//#ifndef RQM_SV_ONLY
