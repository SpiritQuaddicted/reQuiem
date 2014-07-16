
#include "quakedef.h"

#ifndef RQM_SV_ONLY

#ifdef HEXEN2_SUPPORT

#define M_NO_LEVEL_LIST
#include "menu_defs.h"

#ifndef _WIN32
#include <ctype.h>		// for toupper()
#endif

extern qboolean	m_entersound, has_portals;
extern int		m_save_demonum;
extern menu_t	*menu_current, menu_keys, menu_sp;
extern int		m_yofs;
extern cvar_t	cl_playerclass;

void M_Menu_Class_f (cmd_source_t src);
void M_Class_Draw (void);
qboolean M_Class_Key (int key, qboolean down);

void M_Menu_Difficulty_f (cmd_source_t src);
void M_Difficulty_Draw (void);
qboolean M_Difficulty_Key (int key, qboolean down);

static const char *ClassNamesU[NUM_CLASSES] =
{
	"PALADIN",
	"CRUSADER",
	"NECROMANCER",
	"ASSASSIN",
	"DEMONESS"
};

static const char *DiffNames[NUM_CLASSES][NUM_DIFFLEVELS] =
{
	{	// Paladin
		"APPRENTICE",
		"SQUIRE",
		"ADEPT",
		"LORD"
	},
	{	// Crusader
		"GALLANT",
		"HOLY AVENGER",
		"DIVINE HERO",
		"LEGEND"
	},
	{	// Necromancer
		"SORCERER",
		"DARK SERVANT",
		"WARLOCK",
		"LICH KING"
	},
	{	// Assassin
		"ROGUE",
		"CUTTHROAT",
		"EXECUTIONER",
		"WIDOW MAKER"
	},
	{	// Demoness
		"LARVA",
		"SPAWN",
		"FIEND",
		"SHE BITCH"
	}
};


/*menu_t menu_main_H2 =
{
	M_TITLE(NULL, "title0"), M_Main_Draw, M_Main_Key, M_Main_Escape, 0, 0, 4,
	{
		{"SINGLE PLAYER", M_Menu_SinglePlayer_f},
		{"MULTIPLAYER", M_Menu_MultiPlayer_f},
		{"OPTIONS", M_Menu_Options_f},
//		{"HELP", M_Menu_Help_f},
		{"QUIT", M_Menu_Quit_f}
	}
};

menu_t menu_sp_H2 =
{
	M_TITLE(NULL, "title1"), M_SinglePlayer_Draw, M_SinglePlayer_Key, M_Menu_Main_f, 0, 0, 5,
	{
		{"NEW GAME", M_StartNewGame},
		{"LOAD", M_Menu_Load_f},
		{"SAVE", M_Menu_Save_f},
		{"SELECT MAP", M_Menu_Maps_f},
		{"VIEW DEMO", M_Menu_Demos_f}
// Portals-specific:
//		{"OLD MISSION"},
//		{"VIEW INTRO"}
	}
};

menu_t menu_mp_H2 =
{
	M_TITLE(NULL, "title4"), M_MultiPlayer_Draw, M_MultiPlayer_Key, M_Menu_Main_f, 0, 0, 3,
	{
		{"JOIN A GAME", M_CheckEnter_NetMenu},
		{"NEW GAME", M_CheckEnter_NetMenu},
		{"SETUP", M_Menu_Setup_f}
//		{"LOAD"},
//		{"SAVE"}
	}
};
*/

menu_t menu_class =
{
	NULL, "title2", M_Menu_Class_f, M_Class_Draw, M_Class_Key, NULL, &menu_sp, 0, 0, 0, 0
			// num_items is set in M_Menu_Class_f
};

menu_t menu_difficulty =
{
	NULL, "title5", M_Menu_Difficulty_f, M_Difficulty_Draw, M_Difficulty_Key, NULL, &menu_class, 0, 0, 0, NUM_DIFFLEVELS
};

int		setup_class;
int		m_enter_portals;
double	introTime = 0.0;


void M_IPrint (int cx, int cy, const char *str)
{
	cx += (vid.width - 320) / 2;
	cy += (vid.height - 200) / 2;

	while (*str)
	{
		Draw_Character (cx, cy, ((unsigned char)(*str))+256);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic2 (int x, int y, const mpic_t *pic)
{
	// centered H and V
	Draw_TransPic (x + ((vid.width - 320) >> 1), y + ((vid.height - 200)>>1), pic);
}

void M_PrintBig_H2 (int cx, int cy, const char *str)
{
	char upr[64];
	int i;

	for (i = 0; i < 64 && str[i]; i++)
		upr[i] = toupper (str[i]);

	upr[i] = 0;
	M_PrintBig (cx, cy, upr);
}

void M_DrawTitle_H2 (const char *name)
{
	mpic_t *p;

	p = Draw_GetCachePic (va ("gfx/menu/%s.lmp", name), false);
	if (p)
//		Draw_Pic ((vid.width - p->width)/2, m_yofs - 34 , p);
		M_DrawTransPic ((320 - p->width) / 2, -34 , p);

//	if (menu_current != &menu_keys)
	if (!(menu_current->flags & M_NO_QUAKELOGO))
	{
		p = Draw_GetCachePic ("gfx/menu/hplaque.lmp", false);
//		Draw_Pic ((vid.width - 320)/2, m_yofs - 34, p);
		M_DrawTransPic (0, -34, p);
	}
}

//=============================================================================
/* DIFFICULTY MENU */

void M_Menu_Difficulty_f (cmd_source_t src)
{
	key_dest = key_menu;
	menu_current = &menu_difficulty;
}

void M_Difficulty_Draw (void)
{
	int		i;

	M_DrawTitle_H2 (menu_current->lmp);

	setup_class = cl_playerclass.value;

	if (setup_class < 1 || setup_class > NUM_CLASSES)
		setup_class = NUM_CLASSES;
	setup_class--;

	for (i = 0; i < NUM_DIFFLEVELS; ++i)
		M_PrintBig (72, 39+(i*20), DiffNames[setup_class][i]);

	M_DrawSpinningCursor (43, 39 + menu_current->cursor * 20);
}

qboolean M_Difficulty_Key (int key, qboolean down)
{
	switch (key)
	{
/*	case K_LEFTARROW:
	case K_RIGHTARROW:
		break;*/

	case K_ENTER:
		Cvar_SetValueDirect (&skill, menu_current->cursor);
		m_entersound = true;
		if (m_enter_portals)
		{
            // Pa3PyX: needed to play intro properly now
            introTime = Sys_DoubleTime ();
			cl.intermission = 12;
			cl.completed_time = cl.time;
			key_dest = key_game;
			menu_current = NULL;
			cls.demonum = m_save_demonum;
		}
		else
			Cbuf_AddText ("map demo1\n", SRC_COMMAND);
		return true;

/*	default:
		key_dest = key_game;
		menu_current = NULL;
		break;*/
	}

	return false;
}

//=============================================================================
/* CHARACTER CLASS MENU */

void M_Menu_Class_f (cmd_source_t src)
{
	key_dest = key_menu;
	menu_current = &menu_class;
    // Pa3PyX: non-Portals
	menu_class.num_items = (m_enter_portals ? NUM_CLASSES : NUM_CLASSES - 1);
}

void M_Class_Draw (void)
{
	int		i;

	M_DrawTitle_H2 (menu_current->lmp);

    /* Pa3PyX: No demoness in non-Portals please, that's against the plot!
               Multiplayer only. */
	for (i = 0; i < menu_current->num_items; ++i)
		M_PrintBig (72, 39+(i*20), ClassNamesU[i]);
    // Pa3PyX: end code

	M_DrawSpinningCursor (43, 39 + menu_current->cursor * 20);

	i = (has_portals ? 43 : 33);
	M_DrawPic (263, i + 21, Draw_GetCachePic (va("gfx/cport%d.lmp", menu_current->cursor + 1), true));
	M_DrawTransPic (254, i, Draw_GetCachePic ("gfx/menu/frame.lmp", true));
}

qboolean M_Class_Key (int key, qboolean down)
{
	switch (key)
	{
/*	case K_LEFTARROW:
	case K_RIGHTARROW:
		break;*/

	case K_ENTER:
		Cbuf_AddText (va("playerclass %d\n", menu_current->cursor+1), SRC_COMMAND);
		m_entersound = true;
		M_Menu_Difficulty_f (SRC_COMMAND);
		/*if (!class_flag)
		{
			M_Menu_Difficulty_f();
		}
		else
		{
			key_dest = key_game;
			menu_current = NULL;
		}*/
		return true;
/*	default:
		key_dest = key_game;
		menu_current = NULL;
		break;*/
	}

	return false;
}
#endif		// #ifdef HEXEN2_SUPPORT

#endif		//#ifndef RQM_SV_ONLY
