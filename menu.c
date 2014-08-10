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
// menu.c

// JDH: introduced data structures, consolidated code

#include "quakedef.h"

#ifndef RQM_SV_ONLY

#include "winquake.h"
#include "menu_defs.h"

#define M_CVARINFO_Y 196
#define M_VARLEFT    220

typedef void (*m_var_draw_f) (cvar_t *var, int y, qboolean selected);

static const char *MenuSoundsDefault[] = {"misc/menu1.wav", "misc/menu2.wav", "misc/menu3.wav"};

#ifdef HEXEN2_SUPPORT
  static const char *MenuSoundsHexen2[] = {"raven/menu1.wav", "raven/menu2.wav", "raven/menu3.wav"};
  extern qboolean	has_portals;
  extern int		m_enter_portals;
#endif


//extern	int		char_menufonttexture;
extern	mpic_t	draw_menufont;
extern	cvar_t	scr_menusize, scr_centermenu;
extern	cvar_t	vid_vsync, host_maxfps, scr_sshot_format, jpeg_compression_level, png_compression_level;
extern	cvar_t	gl_texturemode, gl_texbrighten, gl_zfightfix, r_oldsky, r_flatlightstyles, r_modelbrightness, gl_glows, r_powerupglow;
extern	cvar_t	nosound, sv_entpatch, sv_altnoclip;
extern	cvar_t	scr_consize, scr_conspeed, gl_conalpha, con_linespacing, con_logcenterprint, gl_consolefont, gl_smoothfont;
extern	cvar_t	scr_centersbar, scr_sbarsize, scr_hudscale, scr_notifyscale, scr_showspeed, scr_showorigin, scr_showfps, con_notifytime, _con_notifylines, con_autocomplete;
extern	cvar_t	crosshair, crosshairsize, gl_crosshairalpha, crosshaircolor, gl_crosshairimage, cl_crossx, cl_crossy;
extern	cvar_t	com_matchfilecase, sv_protocol, host_cutscenehack, sv_fishfix, sv_imp12hack, nospr32;
extern	cvar_t	v_gunkick, cl_deadbodyfilter, cl_gibfilter, cl_demo_compress, cl_demo_compress_fmt;

static const char **gMenuSounds = MenuSoundsDefault;

menu_t		*menu_current = NULL;
qboolean vid_windowedmouse = true;

/*enum {m_none, m_main, m_singleplayer, m_load, m_save, m_multiplayer,
	m_setup, m_net, m_options, m_videomodes,
//#ifdef GLQUAKE
	m_videooptions, m_particles,
//#endif
	m_keys, m_nehdemos, m_maps, m_demos, m_help, m_quit, m_serialconfig, m_modemconfig,
	m_lanconfig, m_gameoptions, m_search, m_slist,
#ifdef HEXEN2_SUPPORT
	m_class, m_difficulty
#endif
} m_state;
*/
void M_Menu_Main_f (cmd_source_t src);
	void M_Menu_SinglePlayer_f (cmd_source_t src);
		void M_Menu_Load_f (cmd_source_t src);
		void M_Menu_Save_f (cmd_source_t src);
	void M_Menu_MultiPlayer_f (cmd_source_t src);
		void M_Menu_Setup_f (cmd_source_t src);
		void M_Menu_Net_f (cmd_source_t src);
	void M_Menu_Options_f (cmd_source_t src);
		void M_Menu_Video_f (cmd_source_t src);
			void M_Menu_VidGeneral_f (cmd_source_t src);
			void M_Menu_Textures_f (cmd_source_t src);
			void M_Menu_HUD_f (cmd_source_t src);
				void M_Menu_Crosshair_f (cmd_source_t src);
			void M_Menu_Lighting_f (cmd_source_t src);
			void M_Menu_SkyWater_f (cmd_source_t src);
			void M_Menu_Particles_f (cmd_source_t src);
//			void M_Menu_VideoOptions_f (cmd_source_t src);
			void M_Menu_VideoModes_f (cmd_source_t src);
		void M_Menu_Audio_f (cmd_source_t src);
		void M_Menu_Controls_f (cmd_source_t src);
			void M_Menu_Keys_f (cmd_source_t src);
			void M_Menu_Keys2_f (cmd_source_t src);
		void M_Menu_MenuConsole_f (cmd_source_t src);
		void M_Menu_Compat_f (cmd_source_t src);
		void M_Menu_Game_f (cmd_source_t src);
	void M_Menu_NehDemos_f (cmd_source_t src);
	void M_Menu_Maps_f (cmd_source_t src);
	void M_Menu_Demos_f (cmd_source_t src);
	void M_Menu_Help_f (cmd_source_t src);
	void M_Menu_Quit_f (cmd_source_t src);
//void M_Menu_SerialConfig_f (cmd_source_t src);
//	void M_Menu_ModemConfig_f (cmd_source_t src);
void M_Menu_LanConfig_f (cmd_source_t src);
void M_Menu_GameOptions_f (cmd_source_t src);
void M_Menu_Search_f (cmd_source_t src);
void M_Menu_ServerList_f (cmd_source_t src);

void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Load_Draw (void);
		void M_Save_Draw (void);
	void M_MultiPlayer_Draw (void);
		void M_Setup_Draw (void);
		void M_Net_Draw (void);
	void M_Options_Draw (void);
		void M_Video_Draw (void);
			void M_VidGeneral_Draw (void);
			void M_Textures_Draw (void);
			void M_HUD_Draw (void);
				void M_Crosshair_Draw (void);
			void M_Lighting_Draw (void);
			void M_SkyWater_Draw (void);
			void M_Particles_Draw (void);
//			void M_VideoOptions_Draw (void);
			void M_VideoModes_Draw (void);
		void M_Audio_Draw (void);
		void M_Controls_Draw (void);
			void M_Keys_Draw (void);
		void M_MenuConsole_Draw (void);
		void M_Compat_Draw (void);
		void M_Game_Draw (void);
	void M_NehDemos_Draw (void);
	void M_Maps_Draw (void);
	void M_Demos_Draw (void);
	void M_Quit_Draw (void);
//void M_SerialConfig_Draw (void);
//	void M_ModemConfig_Draw (void);
void M_LanConfig_Draw (void);
void M_GameOptions_Draw (void);
void M_Search_Draw (void);
void M_ServerList_Draw (void);
void M_Help_Draw (void);

qboolean M_Load_Key (int key, qboolean down);
qboolean M_Save_Key (int key, qboolean down);
qboolean M_Setup_Key (int key, qboolean down);
//qboolean M_Video_Key (int key, qboolean down);
//	qboolean M_VidGeneral_Key (int key, qboolean down);
//	qboolean M_Textures_Key (int key, qboolean down);
//	qboolean M_HUD_Key (int key, qboolean down);
		qboolean M_Crosshair_Key (int key, qboolean down);
//	qboolean M_Lighting_Key (int key, qboolean down);
	qboolean M_SkyWater_Key (int key, qboolean down);
//	qboolean M_Particles_Key (int key, qboolean down);
//	qboolean M_VideoOptions_Key (int key, qboolean down);
//qboolean M_Audio_Key (int key, qboolean down);
//qboolean M_Controls_Key (int key, qboolean down);
	qboolean M_Keys_Key (int key, qboolean down);
qboolean M_MenuConsole_Key (int key, qboolean down);
//qboolean M_Compat_Key (int key, qboolean down);
//qboolean M_Game_Key (int key, qboolean down);
qboolean M_NehDemos_Key (int key, qboolean down);
qboolean M_Maps_Key (int key, qboolean down);
qboolean M_Demos_Key (int key, qboolean down);
qboolean M_Quit_Key (int key, qboolean down);
//qboolean M_SerialConfig_Key (int key, qboolean down);
//	qboolean M_ModemConfig_Key (int key, qboolean down);
qboolean M_LanConfig_Key (int key, qboolean down);
qboolean M_GameOptions_Key (int key, qboolean down);
qboolean M_Search_Key (int key, qboolean down);
qboolean M_ServerList_Key (int key, qboolean down);
qboolean M_Help_Key (int key, qboolean down);

qboolean M_Main_Close (void);
qboolean M_Keys_Close (void);
qboolean M_Crosshair_Close (void);
qboolean M_SkyWater_Close (void);
qboolean M_VideoModes_Close (void);
qboolean M_Maps_Close (void);
qboolean M_Demos_Close (void);
qboolean M_Quit_Close (void);
qboolean M_Help_Close (void);

qboolean	m_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
qboolean	m_recursiveDraw;
qboolean	m_bind_grab;			// Keys and Menu/Console menus

menu_t		*m_return_state = NULL;
double		m_close_time = -9999;
int			translate_texture;		// for player setup menu


qboolean M_OnChange_keyvar (cvar_t *, const char *);
cvar_t key_menuclose = {"key_menuclose", "ESCAPE",    CVAR_FLAG_ARCHIVE | CVAR_FLAG_NOCASE, M_OnChange_keyvar};
cvar_t key_menuprev  = {"key_menuprev",  "BACKSPACE", CVAR_FLAG_ARCHIVE | CVAR_FLAG_NOCASE, M_OnChange_keyvar};

extern qboolean	net_return_onerror;
extern char		net_return_reason[32];
//extern qboolean	menubound[256];

/*#ifdef HEXEN2_SUPPORT
#define StartingGame	((hexen2 && menu_mp_H2.cursor == 1) || (!hexen2 && menu_mp.cursor == 1))
#define JoiningGame		((hexen2 && menu_mp_H2.cursor == 0) || (!hexen2 && menu_mp.cursor == 0))
#else*/
#define StartingGame	(menu_mp.cursor == 1)
#define JoiningGame		(menu_mp.cursor == 0)
//#endif

//#define SerialConfig	(menu_net.cursor == 0)
//#define DirectConfig	(menu_net.cursor == 1)
#define	IPXConfig	(menu_net.cursor == 0)
#define	TCPIPConfig	(menu_net.cursor == 1)

void M_ConfigureNetSubsystem (void);

//int	menuwidth = 320;
//int	menuheight = 240;

int	m_yofs = 0;

#define	MAX_SAVEGAMES	12
#define MAX_VIEWITEMS   14		/* max menu items that can be seen without scrolling */

void M_ShowNehCredits (cmd_source_t src);
void M_CheckEnter_NetMenu (cmd_source_t src);
void M_StartSPGame (cmd_source_t src);
void M_StartMPGame (cmd_source_t src);
void M_Options_GoToConsole (cmd_source_t src);
void M_Options_Reset (cmd_source_t src);

// these aren't official (registered) cvars, but data structure is useful:
#define DECLARE_CVAR_PRESET(name) \
	qboolean M_OnChange_##name (cvar_t *var, const char *value);	\
	cvar_t name = {NULL, NULL, 0, M_OnChange_##name, 0, NULL, CVAR_INT, 0}

/*
qboolean M_OnChange_render_preset (cvar_t *var, char *value);
qboolean M_OnChange_particle_preset (cvar_t *var, char *value);
qboolean M_OnChange_compat_preset (cvar_t *var, char *value);
qboolean M_OnChange_exttex_preset (cvar_t *var, char *value);
qboolean M_OnChange_fbcolors_preset (cvar_t *var, char *value);
qboolean M_OnChange_aim_preset (cvar_t *var, char *value);

cvar_t render_preset   = {NULL, NULL, 0, M_OnChange_render_preset,   0, NULL, CVAR_INT, 0};
cvar_t particle_preset = {NULL, NULL, 0, M_OnChange_particle_preset, 0, NULL, CVAR_INT, 0};
cvar_t compat_preset   = {NULL, NULL, 0, M_OnChange_compat_preset,   0, NULL, CVAR_INT, 0};
cvar_t exttex_preset   = {NULL, NULL, 0, M_OnChange_exttex_preset,   0, NULL, CVAR_INT, 0};
cvar_t fbcolors_preset = {NULL, NULL, 0, M_OnChange_fbcolors_preset, 0, NULL, CVAR_INT, 0};
cvar_t aim_preset      = {NULL, NULL, 0, M_OnChange_aim_preset,      0, NULL, CVAR_INT, 0};
*/
DECLARE_CVAR_PRESET(render_preset);
DECLARE_CVAR_PRESET(particle_preset);
DECLARE_CVAR_PRESET(compat_preset);
DECLARE_CVAR_PRESET(exttex_preset);
DECLARE_CVAR_PRESET(fbcolors_preset);
DECLARE_CVAR_PRESET(aim_preset);

#ifdef HEXEN2_SUPPORT
  #define M_TITLE(str, lmp) str, lmp
#else
  #define M_TITLE(str, lmp) str
#endif

menu_t menu_main =
{
	M_TITLE("reQuiem", "title0"), M_Menu_Main_f, M_Main_Draw, NULL, M_Main_Close, NULL, 0, 0, 0, 4,
	{
		{"Single Player", M_Menu_SinglePlayer_f, NULL, M_ITEM_BIG},
		{"Multiplayer", M_Menu_MultiPlayer_f, NULL, M_ITEM_BIG},
		{"Options", M_Menu_Options_f, NULL, M_ITEM_BIG},
		{"Quit", M_Menu_Quit_f, NULL, M_ITEM_BIG}
	}
};

menu_t menu_main_nehmovie =
{
	M_TITLE("reQuiem", "title0"), M_Menu_Main_f, M_Main_Draw, NULL, M_Main_Close, NULL, 0, 0, 0, 4,
	{
		{"Play Movie", M_Menu_NehDemos_f, NULL, M_ITEM_BIG},
		{"Credits", M_ShowNehCredits, NULL, M_ITEM_BIG},
		{"Options", M_Menu_Options_f, NULL, M_ITEM_BIG},
		{"Quit", M_Menu_Quit_f, NULL, M_ITEM_BIG}
	}
};

menu_t menu_main_nehgame =
{
	M_TITLE("reQuiem", "title0"), M_Menu_Main_f, M_Main_Draw, NULL, M_Main_Close, NULL, 0, 0, 0, 5,
	{
		{"Single Player", M_Menu_SinglePlayer_f, NULL, M_ITEM_BIG},
		{"Multiplayer", M_Menu_MultiPlayer_f, NULL, M_ITEM_BIG},
		{"Credits", M_ShowNehCredits, NULL, M_ITEM_BIG},
		{"Options", M_Menu_Options_f, NULL, M_ITEM_BIG},
		{"Quit", M_Menu_Quit_f, NULL, M_ITEM_BIG}
	}
};

menu_t menu_main_nehboth =
{
	M_TITLE("reQuiem", "title0"), M_Menu_Main_f, M_Main_Draw, NULL, M_Main_Close, NULL, 0, 0, 0, 6,
	{
		{"Single Player", M_Menu_SinglePlayer_f, NULL, M_ITEM_BIG},
		{"Play Movie", M_Menu_NehDemos_f, NULL, M_ITEM_BIG},
		{"Multiplayer", M_Menu_MultiPlayer_f, NULL, M_ITEM_BIG},
		{"Credits", M_ShowNehCredits, NULL, M_ITEM_BIG},
		{"Options", M_Menu_Options_f, NULL, M_ITEM_BIG},
		{"Quit", M_Menu_Quit_f, NULL, M_ITEM_BIG}
	}
};

menu_t menu_sp =
{
	M_TITLE("Single Player", "title1"), M_Menu_SinglePlayer_f, M_SinglePlayer_Draw, NULL, NULL, &menu_main, 0, 0, 0, 5,
	{
		{"New Game", M_StartSPGame, NULL, M_ITEM_BIG},
		{"Load", M_Menu_Load_f, NULL, M_ITEM_BIG},
		{"Save", M_Menu_Save_f, NULL, M_ITEM_BIG},
		{"Maps", M_Menu_Maps_f, NULL, M_ITEM_BIG},
		{"Demos", M_Menu_Demos_f, NULL, M_ITEM_BIG}
	}
};

menu_t menu_mp =
{
	M_TITLE("Multiplayer", "title4"), M_Menu_MultiPlayer_f, M_MultiPlayer_Draw, NULL, NULL, &menu_main, 0, 0, 0, 3,
	{
		{"Join Game", M_CheckEnter_NetMenu, NULL, M_ITEM_BIG},
		{"Create Game", M_CheckEnter_NetMenu, NULL, M_ITEM_BIG},
		{"Player Setup", M_Menu_Setup_f, NULL, M_ITEM_BIG}
	}
};

menu_t menu_load =
{
	M_TITLE("Load", "load"), M_Menu_Load_f, M_Load_Draw, M_Load_Key, NULL, &menu_sp, 0, 0, 0, MAX_SAVEGAMES
};

menu_t menu_save =
{
	M_TITLE("Save", "save"), M_Menu_Save_f, M_Save_Draw, M_Save_Key, NULL, &menu_sp, 0, 0, 0, MAX_SAVEGAMES
};

menu_t menu_setup =
{
	M_TITLE("Player Setup", "title4"), M_Menu_Setup_f, M_Setup_Draw, M_Setup_Key, NULL, &menu_mp, 0, 4, 0, 5,
	{
		{"Hostname"},
		{"Your Name"},
		{"Shirt Color"},
		{"Pants Color"},
		{"Accept Changes"}
	}
};

menu_t menu_net =
{
	M_TITLE("Network Protocol", "title4"), M_Menu_Net_f, M_Net_Draw, NULL, NULL, &menu_mp, 0, 0, 0, 2,
	{
	//	{"Modem", M_Menu_SerialConfig_f},
	//	{"Direct Connect", M_Menu_SerialConfig_f},
		{"IPX", M_Menu_LanConfig_f},
		{"TCP/IP", M_Menu_LanConfig_f}
	//	{"Multiprotocol"}
	}
};

menu_t menu_options =
{
	M_TITLE("Options", "title3"), M_Menu_Options_f, M_Options_Draw, NULL, NULL, &menu_main, 0, 0, 0, 9,
	{
		{"Video",             M_Menu_Video_f, NULL, M_ITEM_BIG},
		{"Audio",             M_Menu_Audio_f, NULL, M_ITEM_BIG},
		{"Controls",          M_Menu_Controls_f, NULL, M_ITEM_BIG},
		{"Menu/Console",      M_Menu_MenuConsole_f, NULL, M_ITEM_BIG},
		{"Compatibility",     M_Menu_Compat_f, NULL, M_ITEM_BIG},
		{"Game",              M_Menu_Game_f, NULL, M_ITEM_BIG},
		{"",                  NULL, NULL, M_ITEM_DISABLED},
		{"Go to console",     M_Options_GoToConsole, NULL, M_ITEM_WHITE},
		{"Reset to defaults", M_Options_Reset, NULL, M_ITEM_WHITE}
	}
};

menu_t menu_video =
{
	M_TITLE("Video Options", "title3"), M_Menu_Video_f, M_Video_Draw, NULL/*M_Video_Key*/, NULL, &menu_options, 0, 0, 0, 10,
	{
		{"General",                M_Menu_VidGeneral_f},
		{"Textures",               M_Menu_Textures_f},
		{"HUD",                    M_Menu_HUD_f},
		{"Crosshair",              M_Menu_Crosshair_f},
		{"Lighting",               M_Menu_Lighting_f},
		{"Sky & water",            M_Menu_SkyWater_f},
		{"Particles",              M_Menu_Particles_f},
//		{"Advanced Options",       M_Menu_VideoOptions_f},
		{"Video Modes",            M_Menu_VideoModes_f},
		{"",                       NULL, NULL, M_ITEM_DISABLED},
		{"Preset config",          NULL, &render_preset, M_ITEM_WHITE}
	}
};

menu_t menu_vidgeneral =
{
	M_TITLE("General Video Options", "title3"), M_Menu_VidGeneral_f, M_VidGeneral_Draw,
		NULL/*M_VidGeneral_Key*/, NULL, &menu_video, M_ALIGN_RIGHT, 0, 0, 8,
	{
		{           "Screen size", NULL, &scr_viewsize, M_ITEM_SLIDER},
		{            "Brightness", NULL, &v_gamma, M_ITEM_SLIDER},
		{              "Contrast", NULL, &v_contrast, M_ITEM_SLIDER},
		{         "Vertical Sync", NULL, &vid_vsync},
		{        "Max frames/sec", NULL, &host_maxfps},
		{     "Smooth animations", NULL, &gl_interpolate_animation},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{     "Screenshot format", NULL, &scr_sshot_format}
		// next item depends on value of scr_sshot_format
	}
};

menu_t menu_textures =
{
	M_TITLE("Texture Options", "title3"), M_Menu_Textures_f, M_Textures_Draw,
		NULL /*M_Textures_Key*/, NULL, &menu_video, M_ALIGN_RIGHT, 0, 0, 14,
	{
		{        "Texture filter", NULL, &gl_texturemode},
		{       "Texture quality", NULL, &gl_picmip, M_ITEM_SLIDER},
		{    "Brighten world tex", NULL, &gl_texbrighten},
		{       "Detail textures", NULL, &gl_detail},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{     "External textures", NULL, &exttex_preset, M_ITEM_WHITE},
		{                 "world", NULL, &gl_externaltextures_world},
		{          "brush models", NULL, &gl_externaltextures_bmodels},
		{          "alias models", NULL, &gl_externaltextures_models},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{     "Fullbright colors", NULL, &fbcolors_preset, M_ITEM_WHITE},
		{                 "world", NULL, &gl_fb_world},
		{          "brush models", NULL, &gl_fb_bmodels},
		{          "alias models", NULL, &gl_fb_models}
	}
};

menu_t menu_hud =
{
	M_TITLE("HUD Options", "title3"), M_Menu_HUD_f, M_HUD_Draw, NULL/*M_HUD_Key*/,
		NULL, &menu_video, M_ALIGN_RIGHT, 0, 0, 13,
	{
		{      "Solid status bar", NULL, &cl_sbar},
		{   "Centered status bar", NULL, &scr_centersbar},
		{       "Status bar size", NULL, &scr_sbarsize, M_ITEM_SLIDER},
		{   "Weapon transparency", NULL, &r_drawviewmodel, M_ITEM_SLIDER},
		{           "Weapon size", NULL, &r_viewmodelsize, M_ITEM_SLIDER},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{        "HUD text scale", NULL, &scr_hudscale, M_ITEM_SLIDER},
		{     "Notify text scale", NULL, &scr_notifyscale, M_ITEM_SLIDER},
		{     "Show player speed", NULL, &scr_showspeed},
		{       "Show player pos", NULL, &scr_showorigin},
		{              "Show FPS", NULL, &scr_showfps},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{      "Message duration", NULL, &con_notifytime},
		{     "Max message lines", NULL, &_con_notifylines}
	}
};

menu_t menu_crosshair =
{
	M_TITLE("Crosshair Options", "title3"), M_Menu_Crosshair_f, M_Crosshair_Draw, M_Crosshair_Key,
		M_Crosshair_Close, &menu_video, M_ALIGN_RIGHT, 0, 0, 7,
	{
		{                 "Style", NULL, &crosshair, M_ITEM_SLIDER},
		{	               "Size", NULL, &crosshairsize, M_ITEM_SLIDER},
		{                 "Color", NULL, &crosshaircolor},
		{	       "Transparency", NULL, &gl_crosshairalpha, M_ITEM_SLIDER},
		{        "External image", NULL, &gl_crosshairimage},
		{           "Horz offset", NULL, &cl_crossx, M_ITEM_SLIDER},
		{           "Vert offset", NULL, &cl_crossy, M_ITEM_SLIDER}
	}
};

menu_t menu_lighting =
{
	M_TITLE("Lighting Options", "title3"), M_Menu_Lighting_f, M_Lighting_Draw, NULL/*M_Lighting_Key*/, NULL,
		&menu_video, M_ALIGN_RIGHT, 0, 0, 13,
	{
		{     "World light style", NULL, &gl_lightmode},
		{        "Colored lights", NULL, &gl_loadlitfiles},
		{        "Dynamic lights", NULL, &r_dynamic},
		{       "Animated lights", NULL, &r_flatlightstyles},
		{     "Model light level", NULL, &r_modelbrightness, M_ITEM_SLIDER},
		{               "Shadows", NULL, &r_shadows, M_ITEM_SLIDER},
		{       "Vertex lighting", NULL, &gl_vertexlights},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{        "Lightning glow", NULL, &gl_glows},
		{           "Other glows", NULL, &gl_flashblend},
		{   "Player powerup glow", NULL, &r_powerupglow},
		{            "Glow color", NULL, &r_explosionlightcolor},
		{           "Glow radius", NULL, &r_explosionlight, M_ITEM_SLIDER}
	}
};

menu_t menu_skywater =
{
	M_TITLE("Sky & Water Options", "title3"), M_Menu_SkyWater_f, M_SkyWater_Draw,
		M_SkyWater_Key, M_SkyWater_Close, &menu_video, M_ALIGN_RIGHT, 0, 0, 10,
	{
		{              "Sky type", NULL, &r_skytype},
		{       "Solid sky color", NULL, &r_skycolor},
		{          "Use skyboxes", NULL, &r_oldsky},
		{        "Current skybox", NULL, &r_skybox},
		{    "Fast but buggy sky", NULL, &gl_skyhack},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{        "Water caustics", NULL, &gl_caustics},
		{        "Underwater fog", NULL, &gl_waterfog},
		{      "Waterfog density", NULL, &gl_waterfog_density, M_ITEM_SLIDER},
		{           "Water alpha", NULL, &r_wateralpha, M_ITEM_SLIDER}
	}
};

menu_t menu_particles =
{
	M_TITLE("Particle Options", "title3"), M_Menu_Particles_f, M_Particles_Draw, NULL /*M_Particles_Key*/,
		NULL, &menu_video, M_ALIGN_RIGHT, 0, 0, 16,
	{
		{      "Bounce particles", NULL, &gl_bounceparticles},
		{        "Clip particles", NULL, &gl_clipparticles},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{         "Preset config", NULL, &particle_preset, M_ITEM_WHITE},
		{            "Explosions", NULL, &gl_part_explosions},
		{                "Trails", NULL, &gl_part_trails},
		{                "Spikes", NULL, &gl_part_spikes},
		{              "Gunshots", NULL, &gl_part_gunshots},
		{                 "Blood", NULL, &gl_part_blood},
		{     "Teleport splashes", NULL, &gl_part_telesplash},
		{      "Spawn explosions", NULL, &gl_part_blobs},
		{         "Lava splashes", NULL, &gl_part_lavasplash},
		{               "Inferno", NULL, &gl_part_inferno},
		{                "Flames", NULL, &gl_part_flames},
		{             "Lightning", NULL, &gl_part_lightning},
		{          "Spike trails", NULL, &gl_part_spiketrails}
	}
};

/*menu_t menu_options_video =
{
	M_TITLE("Video Options", "title3"), M_Menu_VideoOptions_f, M_VideoOptions_Draw, M_VideoOptions_Key,
		NULL, &menu_video, M_ALIGN_RIGHT, 0, 0, 4,
	{
		{     "Smooth animations", NULL, &gl_interpolate_animation},
		{        "Texture filter", NULL, &gl_texturemode},
		{       "Texture quality", NULL, &gl_picmip},
		{       "Detail textures", NULL, &gl_detail}
	}
};*/

menu_t menu_videomodes =
{
	M_TITLE("Video Modes", NULL), M_Menu_VideoModes_f, M_VideoModes_Draw, NULL, M_VideoModes_Close, &menu_video, 0, 0, 0, 0
};

menu_t menu_audio =
{
	M_TITLE("Audio Options", "title3"), M_Menu_Audio_f, M_Audio_Draw, NULL/*M_Audio_Key*/, NULL, &menu_options, M_ALIGN_RIGHT, 0, 0, 3,
	{
		{       "CD/Music Volume", NULL, &bgmvolume, M_ITEM_SLIDER},
		{          "Sound Volume", NULL, &volume, M_ITEM_SLIDER},
		{      "Disable sound FX", NULL, &nosound}
	}
};

#define M_CONTROLS_NUMITEMS 13

menu_t menu_controls =
{
	M_TITLE("Control Options", "title3"), M_Menu_Controls_f, M_Controls_Draw, NULL/*M_Controls_Key*/,
		NULL, &menu_options, M_ALIGN_RIGHT, 0, 0, M_CONTROLS_NUMITEMS,
	{
		{     "Gameplay controls", M_Menu_Keys_f, NULL, M_ITEM_WHITE},
		{        "Other controls", M_Menu_Keys2_f, NULL, M_ITEM_WHITE},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{           "Mouse Speed", NULL, &sensitivity, M_ITEM_SLIDER},
		{            "Always Run", NULL, &cl_forwardspeed},
		{      "Always Mouselook", NULL, &m_look},
		{          "Invert Mouse", NULL, &m_pitch},
		{            "Lookspring", NULL, &lookspring},
		{            "Lookstrafe", NULL, &lookstrafe},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{     "Noclip: viewangle", NULL, NULL, M_ITEM_DISABLED},
		{      "determines pitch", NULL, &sv_altnoclip},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{         "Capture Mouse", NULL, &_windowed_mouse}
	}
};

menu_t menu_menuconsole =
{
	M_TITLE("Menu/Console Options", "title3"), M_Menu_MenuConsole_f, M_MenuConsole_Draw, M_MenuConsole_Key,
		NULL, &menu_options, M_ALIGN_RIGHT, 0, 0, 14,
	{
		{           "Center menu", NULL, &scr_centermenu},
		{             "Menu size", NULL, &scr_menusize, M_ITEM_SLIDER},
		{       "Key: close menu", NULL, &key_menuclose},
		{       "Key: prev. menu", NULL, &key_menuprev},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{        "Console height", NULL, &scr_consize, M_ITEM_SLIDER},
		{         "Console speed", NULL, &scr_conspeed, M_ITEM_SLIDER},
		{         "Console alpha", NULL, &gl_conalpha, M_ITEM_SLIDER},
//		{          "Line spacing", NULL, &con_linespacing},
		{      "Log centerprints", NULL, &con_logcenterprint},
		{        "Tab-completion", NULL, &cl_advancedcompletion},
		{       "Auto-completion", NULL, &con_autocomplete},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{      "External charset", NULL, &gl_consolefont},		// for console AND menu
		{     "Smooth font edges", NULL, &gl_smoothfont}
	}
};

menu_t menu_compat =
{
	M_TITLE("Compatibility Options", "title3"), M_Menu_Compat_f, M_Compat_Draw, NULL/*M_Compat_Key*/,
		NULL, &menu_options, M_ALIGN_RIGHT, 0, 0, 14,
	{
		{         "Preset config", NULL, &compat_preset, M_ITEM_WHITE},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{   "Match filename case", NULL, &com_matchfilecase},
		{       "Load .ent files", NULL, &sv_entpatch},
		{       "Server protocol", NULL, &sv_protocol},
		{        "Fix fish count", NULL, &sv_fishfix},
		{     "Prev. weapon hack", NULL, &sv_imp12hack},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{   "Smooth cutscene cam", NULL, &host_cutscenehack},
		{     "Reduce Z-fighting", NULL, &gl_zfightfix},
		{       "Hi-res textures", NULL, &exttex_preset},
		{     "Fullbright colors", NULL, &fbcolors_preset},
		{    "Transparent models", NULL, &gl_notrans},
		{        "32-bit sprites", NULL, &nospr32}
	}
};

menu_t menu_game =
{
	M_TITLE("Game Options", "title3"), M_Menu_Game_f, M_Game_Draw, NULL,
		NULL, &menu_options, M_ALIGN_RIGHT, 0, 0, 9,
	{
		{           "Weapon kick", NULL, &v_gunkick},
		{         "Aim precision", NULL, &aim_preset},
		{      "LG tracking rate", NULL, &cl_truelightning, M_ITEM_SLIDER},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{        "Corpse removal", NULL, &cl_deadbodyfilter},
		{           "Gib removal", NULL, &cl_gibfilter},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{      "Demo compression", NULL, &cl_demo_compress},
		{       "Compress format", NULL, &cl_demo_compress_fmt}
	}
};

typedef struct menu50_s MENU_T(50) menu50_t;

menu50_t menu_keys =
{
	M_TITLE("Key Configuration", "title6"), M_Menu_Keys_f, M_Keys_Draw, M_Keys_Key,
		M_Keys_Close, &menu_controls, M_NO_QUAKELOGO | M_NO_CVARINFO | M_NO_CURSOR, 0, 0, 0
			// num_items is set in M_Menu_Keys_f
};

menu50_t menu_keys2 =
{
	M_TITLE("Key Configuration", "title6"), M_Menu_Keys2_f, M_Keys_Draw, M_Keys_Key,
		M_Keys_Close, &menu_controls, M_NO_QUAKELOGO | M_NO_CVARINFO | M_NO_CURSOR, 0, 0, 0
			// num_items is set in M_Menu_Keys_f
};

menu_t menu_maps =
{
	M_TITLE("MAPS", NULL), M_Menu_Maps_f, M_Maps_Draw, M_Maps_Key, M_Maps_Close, &menu_sp, 0, 0, 0, 0
};

menu_t menu_demos =
{
	M_TITLE("DEMOS", NULL), M_Menu_Demos_f, M_Demos_Draw, M_Demos_Key, M_Demos_Close, &menu_sp, 0, 0, 0, 0
};

menu_t menu_nehdemos =
{
	M_TITLE("DEMOS", NULL), M_Menu_NehDemos_f, M_NehDemos_Draw, M_NehDemos_Key, NULL, &menu_main, 0, 0, 0, NUM_NEHDEMOS
};

menu_t menu_lanconfig =
{
	M_TITLE("LAN Configuration", "title4"), M_Menu_LanConfig_f, M_LanConfig_Draw, M_LanConfig_Key, NULL, &menu_net, 0, -1, 0, 0
};

/*
menu_t menu_serialconfig =
{
	M_TITLE(NULL, NULL), M_Menu_SerialConfig_f, M_SerialConfig_Draw, M_SerialConfig_Key, NULL, &menu_net, 0, 0, 0, 0
};

menu_t menu_modemconfig =
{
	M_TITLE(NULL, NULL), M_Menu_ModemConfig_f, M_ModemConfig_Draw, M_ModemConfig_Key, NULL, &menu_net, 0, 0, 0, 0
};
*/

menu_t menu_quit =
{
	M_TITLE(NULL, NULL), M_Menu_Quit_f, M_Quit_Draw, M_Quit_Key, M_Quit_Close, NULL, 0, 0, 0, 0
};

menu_t menu_gameoptions =
{
	M_TITLE("LAN Game Options", "title4"), M_Menu_GameOptions_f, M_GameOptions_Draw, M_GameOptions_Key,
		NULL, &menu_lanconfig, M_ALIGN_RIGHT, 0, 0, 11,
	{
		{                     " ", M_StartMPGame, NULL},		// "Begin Game" button
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{    "Max players       ", NULL, NULL},
		{      "Game Type       ", NULL, &coop},
		{       "Teamplay       ", NULL, &teamplay},
		{          "Skill       ", NULL, &skill},
		{     "Frag Limit       ", NULL, &fraglimit},
		{     "Time Limit       ", NULL, &timelimit},
		{                      "", NULL, NULL, M_ITEM_DISABLED},
		{        "Episode       ", NULL, NULL},
		{          "Level       ", NULL, NULL}
	}
};

menu_t menu_search =
{
	M_TITLE(NULL, "title4"), M_Menu_Search_f, M_Search_Draw, M_Search_Key, NULL, NULL, 0, 0, 0, 0
};

menu_t menu_serverlist =
{
	M_TITLE(NULL, "load"), M_Menu_ServerList_f, M_ServerList_Draw, M_ServerList_Key, NULL, &menu_lanconfig, 0, 0, 0, 0
};

menu_t menu_help =
{
	M_TITLE(NULL, NULL), M_Menu_Help_f, M_Help_Draw, M_Help_Key, M_Help_Close, &menu_main, 0, 0, 0, 5
};

// JDH: data for up & down arrow characters
byte m_scroll_up_data[64] =
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0x68, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0x67, 0x67, 0x67, 0xFF, 0xFF, 0xFF,
	0xFF, 0x66, 0x66, 0x60, 0x66, 0x66, 0xFF, 0xFF,
	0x65, 0x65, 0x60, 0x60, 0xFF, 0x65, 0x65, 0xFF,
	0xFF, 0x60, 0x60, 0xFF, 0xFF, 0xFF, 0x60, 0x60,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

byte m_scroll_dn_data[64] =
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x68, 0x68, 0xFF, 0xFF, 0xFF, 0x68, 0x68, 0xFF,
	0xFF, 0x67, 0x67, 0xFF, 0x67, 0x67, 0x60, 0x60,
	0xFF, 0xFF, 0x66, 0x66, 0x66, 0x60, 0x60, 0xFF,
	0xFF, 0xFF, 0xFF, 0x65, 0x60, 0x60, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0x60, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#define M_MAX_PRESETS 5

typedef struct
{
	cvar_t		*cvar;
	float		value[M_MAX_PRESETS];
} m_preset_t;

typedef struct
{
	cvar_t		*cvar;			// local cvar used for preset config
	int			num_configs;		// including custom
	char		*config_names[M_MAX_PRESETS];
	qboolean	custom_valid;
	int			num_vars;			// #items in following array
	m_preset_t	*presets;			// pointer to array
} m_preset_config_t;

mpic_t m_scroll_up = {8, 8};
mpic_t m_scroll_dn = {8, 8};

/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character (cx + ((vid.width - 320) >> 1), line + m_yofs, num);
}

/*void M_DrawCharacter2 (int cx, int line, int num)
{
	Draw_Character2 (cx + ((vid.width - 320) >> 1), line + m_yofs, num);
}*/

void M_Print (int cx, int cy, const char *str)
{
	Draw_Alt_String (cx + ((vid.width - 320) >> 1), cy + m_yofs, str);
}

void M_PrintWhite (int cx, int cy, const char *str)
{
	Draw_String (cx + ((vid.width - 320) >> 1), cy + m_yofs, str);
}

void M_DrawTransPic (int x, int y, const mpic_t *pic)
{
	Draw_TransPic (x + ((vid.width - 320) >> 1), y + m_yofs, pic);
}

void M_PrintBig (int cx, int cy, const char *str)
{
	Draw_BigString (cx + (vid.width - 320)/2, cy + m_yofs, str);
}

void M_DrawPic (int x, int y, const mpic_t *pic)
{
	Draw_Pic (x + ((vid.width - 320) >> 1), y + m_yofs, pic);
}

void M_DrawBar (int x, int y, int width)
{
	// assumes 1 < width < 128
	char buf[128];

	buf[width--] = 0;
	buf[width--] = '\x1f';
	while (width > 0)
		buf[width--] = '\x1e';
	buf[0] = '\x1d';
	M_Print (x, y, buf);
}

/*
void M_DrawTransPicTranslate (int x, int y, mpic_t *pic)
{
	Draw_TransPicTranslate (x + ((vid.width - 320) >> 1), y + m_yofs, pic, translationTable, 0);
}
*/
void M_DrawTextBox (int x, int y, int width, int height)
{
	Draw_TextBox (x + ((vid.width - 320) >> 1), y + m_yofs, width, height);
}

#if 0
void M_SetScale (qboolean enable)
{
	float percent, maxscale;

	percent = scr_menusize.value;
	if (percent < 0)
	{
		Cvar_SetDirect (&scr_menusize, "0");
		percent = 0;
	}

	if (enable)
	{
		if (percent)
		{
			//menuwidth = 320;
			//menuheight = min(vid.height, 240);
			maxscale = min ((float)vid.width / 320.0, (float)vid.height / 240.0);
			menuwidth = vid.width / ((percent/100.0) * maxscale);
			menuheight = vid.height / ((percent/100.0) * maxscale);
			glMatrixMode (GL_PROJECTION);
			glLoadIdentity ();
			glOrtho (0, menuwidth, menuheight, 0, -99999, 99999);
		}
		else
		{
			menuwidth = vid.width;
			menuheight = vid.height;
		}

		if (scr_centermenu.value)
		{
			m_yofs = (menuheight - 200) / 2;
		}
		else
		{
		#ifdef HEXEN2_SUPPORT
			if (hexen2)
				m_yofs = 34;		// for title plaques
			else
		#endif
				m_yofs = 0;
		}
	}
	else if (percent)
	{
		// restore standard projection
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0, vid.width, vid.height, 0, -99999, 99999);
	}
}

#else
void M_SetScale (qboolean enable)
{
	static unsigned orig_vidwidth, orig_vidheight;
	float percent, maxscale;

	percent = scr_menusize.value;
	if (percent > 0)
	{
		assert (!!orig_vidwidth != enable);

		if (enable)
		{
			orig_vidwidth = vid.width;
			orig_vidheight = vid.height;
			maxscale = min (vid.width / 320.0, vid.height / 240.0);
			vid.width = ceil(vid.width / ((percent/100.0) * maxscale));
//			menuwidth = vid.width;
			vid.height = ceil(vid.height / ((percent/100.0) * maxscale));
//			menuheight = vid.height;
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
#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		m_yofs = scr_centermenu.value ? (vid.height - 200) / 2 : 34;
	}
	else
#endif
	{
		m_yofs = scr_centermenu.value ? (vid.height - 240) / 2 : 0;
	}
}
#endif

//=============================================================================

int	m_save_demonum;

/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu_f (cmd_source_t src)
{
	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (menu_current != &menu_main)
		{
			M_Menu_Main_f (src);
			return;
		}
		key_dest = key_game;
		menu_current = NULL;
		return;
	}
	if (key_dest == key_console)
	{
		Con_ToggleConsole_f (src);
	}
	else
	{
		if (menu_current)
		{
			menu_current->openproc (src);

		// JDH: if menu was closed & reopened within 2 seconds, display backspace tip
			if (realtime - m_close_time < 2)
				m_close_time = realtime;
		}
		else
			M_Menu_Main_f (src);
	}
}

/*
================
M_DrawMenuCursor
================
*/
void M_DrawMenuCursor (int x, int y)
{
	// line cursor
	M_DrawCharacter (x, y, 12+((int)(realtime*4)&1));
}

/*
====================
M_DrawSpinningCursor
====================
*/
void M_DrawSpinningCursor (int x, int y)
{
	int f;
	mpic_t *pic;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		//int f = (int)(host_time * 10)%8;

		f = (int)(realtime * 10) % 8;
		pic = Draw_GetCachePic (va ("gfx/menu/menudot%i.lmp", f+1), false);
		if (pic)
		{
			M_DrawTransPic (x-8, y-6, pic);
			return;
		}
	}
	else
#endif
	{
		//f = (int)(realtime*10) % numpics;
		if (Draw_GetCachePic("gfx/menudot3.lmp", false))
			f = (int)(realtime*10) % 6;
		else
			f = (int)(realtime*5) % 2;		// Quake beta has only 2, and speed is half

		pic = Draw_GetCachePic(va("gfx/menudot%i.lmp", f+1), false);
		if (pic)
		{
			M_DrawTransPic (x, y, pic);
			return;
		}
	}

	M_DrawMenuCursor (x, y+5);		// couldn't find fancy cursor
}

/*
================
M_Menu_DrawTitle
  default routine for drawing menu titles
================
*/
void M_Menu_DrawTitle (menu_t *menu)
{
	int x;
	mpic_t *pic;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		M_DrawTitle_H2 (menu->lmp);
		return;
	}
#endif

	if (!(menu->flags & M_NO_QUAKELOGO))
	{
		pic = Draw_GetCachePic ("gfx/qplaque.lmp", false);
		if (pic)
			M_DrawTransPic (0, 4, pic);
		else
		{
		// beta Quake has no qplaque, but each menu has one
			pic = Draw_GetCachePic ("gfx/mainmenu.lmp", false);
			if (pic && pic->width > 300)
				Draw_SubPic ((vid.width - 320) >> 1, 4 + m_yofs, pic, 0, 0, 32, pic->height);
		}
	}

	x = (320 - strlen (menu->title)*DRAW_CHARWIDTH) / 2;
	M_PrintWhite (x, 11, menu->title);
}

/*
===========
M_DrawItem
  default routine for drawing a menu item. Returns the item's height.
===========
*/
int M_DrawItem (menu_t *menu, menu_item_t *item, int y, qboolean is_selected, m_var_draw_f drawvar_proc)
{
	void		(*draw_proc)(int, int, const char *);
	int			left, itemleft;
	qboolean	hascursor;

	left = (menu->flags & M_NO_QUAKELOGO) ? 0 : 32;
	if (menu->flags & M_NO_CURSOR)
	{
		hascursor = false;
	}
	else
	{
	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			left += 26;
		else
	#endif
			left += 12;

		if (!draw_menufont.texnum)
			left += 16;		// smaller font, so center it a bit more

		hascursor = is_selected;
	}

	if (item->flags & M_ITEM_BIG)
	{
		if (!draw_menufont.texnum)
		{
			M_Print (left+16, y, item->text);
			if (hascursor)
				M_DrawMenuCursor (left, y);
			return 20;
		}

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
		{
			M_PrintBig_H2 (left+14/*72*/, y, item->text);
			if (hascursor)
				M_DrawSpinningCursor (left-15/*43*/, y);
			return 20;
		}
	#endif

		M_PrintBig (left+14, y, item->text);
		if (hascursor)
			M_DrawSpinningCursor (left-8, y);
		return 21;
	}

	draw_proc = (item->flags & M_ITEM_WHITE) ? M_PrintWhite : M_Print;
	/*if (item->flags & M_ITEM_CENTER)
	{
		itemleft = (320 - strlen(item->text)*DRAW_CHARWIDTH) / 2;
		draw_proc (itemleft, y, item->text);
	}
	else*/ if (menu->flags & M_ALIGN_RIGHT)
	{
		itemleft = M_VARLEFT - 20 - (strlen(item->text)+1)*DRAW_CHARWIDTH;
		draw_proc (itemleft, y, item->text);
		if (hascursor)
		{
			if (item->cvar)
				M_DrawMenuCursor (M_VARLEFT-20, y);
			else
				M_DrawMenuCursor (itemleft - 2*DRAW_CHARWIDTH, y);
		}
	}
	else
	{
	#ifdef HEXEN2_SUPPORT
		/*if (hexen2)
			itemleft = (draw_menufont.texnum ? left+30 : left+48);
			//itemleft = left+16;		//itemleft = 72;
		else*/
	#endif
//		itemleft = (char_menufonttexture ? left+16 : left+32);
//		itemleft = (draw_menufont.texnum ? left+16 : left+32);
		itemleft = left + 16;

		draw_proc (itemleft, y, item->text);
		if (hascursor)
			M_DrawMenuCursor (itemleft - 2*DRAW_CHARWIDTH, y);
	}

	if (item->cvar && drawvar_proc)
		drawvar_proc (item->cvar, y, is_selected);

	return MITEMHEIGHT;
}

/*
===========
M_Draw_CvarCaption
===========
*/
void M_Draw_CvarCaption (menu_t *menu)
{
	cvar_t	*var;
	char	*val, info_str[256];
	int		len;

	var = menu->items[menu->cursor].cvar;
	if (var && var->name)		// check name to exclude any 'local' cvars (eg. particle_preset)
	{
		val = (var->type == CVAR_STRING) ? va("\"%s\"", var->string) : var->string;

		len = Q_snprintfz (info_str, sizeof(info_str), "cvar: %s = %s", var->name, val);
		M_PrintWhite ((320 - len*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y, info_str);

		if (Cvar_IsDefaultValue (var))
			M_PrintWhite ((320 - 9*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y+10, "(Default)");
		/*else if (var == &sv_aim)
		{
			float dv = Q_atof(var->defaultvalue);
			M_PrintWhite (20, M_CVARINFO_Y+10, va("%f != %f", var->value, dv));
			M_PrintWhite (20, M_CVARINFO_Y+20, va("%08X != %08X", *(int *)&var->value, *(int *)&dv));
		}*/
	}
}

/*
===========
M_Draw_ScrollArrow
===========
*/
void M_Draw_ScrollArrow (menu_t *menu, int top, int maxlines, qboolean isup)
{
	int left = (menu->flags & M_NO_QUAKELOGO) ? 6 : 40;

	if (!isup)
		top += (maxlines-1)*MITEMHEIGHT;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
	{
		M_DrawCharacter (left, top, (isup ? 128 : 129));
		M_DrawCharacter (320-DRAW_CHARWIDTH, top, (isup ? 128 : 129));
	}
	else
#endif
	{
		M_DrawTransPic (left, top, (isup ? &m_scroll_up : &m_scroll_dn));
		M_DrawTransPic (320-DRAW_CHARWIDTH, top, (isup ? &m_scroll_up : &m_scroll_dn));
	//	M_DrawCharacter2 (left, top, 8 | (isup ? CHAR2_SCRUP : CHAR2_SCRDN));
	//	M_DrawCharacter2 (320-DRAW_CHARWIDTH, top, 8 | (isup ? CHAR2_SCRUP : CHAR2_SCRDN));
	}
}

/*
===========
M_Menu_Draw
  default routine for drawing menus
===========
*/
void M_Menu_Draw (menu_t *menu, int top, int maxlines, m_var_draw_f drawvar_proc)
{
	int		numitems, i, y = top;

	M_Menu_DrawTitle (menu);

	if (menu->first_item)
		M_Draw_ScrollArrow (menu, top, maxlines, true);

// FIXME: this assumes all items in a scrollable menu are the small size
	if (menu->first_item + maxlines < menu->num_items)
		M_Draw_ScrollArrow (menu, top, maxlines, false);

	if (menu->items[menu->first_item].flags & M_ITEM_BIG)
		y += 7;		// a little extra space if first item is big

	numitems = min(menu->num_items, maxlines);
	for (i = 0; i < numitems; i++)
	{
		y += M_DrawItem (menu, &menu->items[menu->first_item+i], y, (menu->first_item+i == menu->cursor), drawvar_proc);
	}

	if (!(menu->flags & M_NO_CVARINFO))
		M_Draw_CvarCaption (menu);
}

/*
===========
M_DrawKeyLegend
===========
*/
#if 0
void M_DrawKeyLegend (void)
{
	int left = vid.width - 112;
	int top = vid.height - 50;

//	if (menu_current == &menu_main)
	{
//		glColor3f (0.44, 0.44, 0.44);
		glColor3f (0.7, 0.7, 0.7);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	if (menu_current != &menu_main)
	{
	Draw_String     (left-88, top, "backspace");
	Draw_Alt_String (left, top, "previous menu");
	}
/*	if (menu_current == &menu_main)
	{
		glColor3f (1, 1, 1);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
*/
	Draw_String     (left-40,  top+10, "esc");
	Draw_Alt_String (left,     top+10, "close menu");

	Draw_Character2 (left-32,  top+20, CHAR2_ARROWUP);
	Draw_Character2 (left-24,  top+20, CHAR2_ARROWDN);
//	Draw_String     (left-8,   top,    "PGUP PGDN");
	Draw_Alt_String (left,     top+20, "move cursor");

	if (menu_current->items[menu_current->cursor].cvar)
	{
		Draw_Character2 (left-32, top+30, CHAR2_ARROWLT);
		Draw_Character2 (left-24, top+30, CHAR2_ARROWRT);
		Draw_Alt_String (left, top+30, "adjust value");
	}
	else
	{
		Draw_String     (left-56,  top+30, "enter");
		Draw_Alt_String (left, top+30, "select item");
	}

		glColor3f (1, 1, 1);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void M_DrawKeyLegend (void)
{
	int left = vid.width - 116;
	int top = 10/*vid.height - 50*/;

	Draw_Fill (left-64, top-6, 176, 51, 254);
	Draw_Fill (left-63, top-5, 174, 49, 0);

//	if (menu_current == &menu_main)
	{
//		glColor3f (0.44, 0.44, 0.44);
		glColor3f (0.7, 0.7, 0.7);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

/*	if (menu_current == &menu_main)
	{
		glColor3f (1, 1, 1);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
*/
	if (menu_current->items[menu_current->cursor].cvar)
	{
		Draw_Character2 (left-32, top, CHAR2_ARROWLT);
		Draw_Character2 (left-24, top, CHAR2_ARROWRT);
		Draw_Alt_String (left, top, "adjust value");
	}
	else
	{
		Draw_String     (left-56,  top, "enter");
		Draw_Alt_String (left, top, "select item");
	}

	Draw_Character2 (left-32,     top+10, CHAR2_ARROWUP);
	Draw_Character2 (left-24,   top+10, CHAR2_ARROWDN);
//	Draw_String     (left-8,  top, "PGUP PGDN");
	Draw_Alt_String (left, top+10, "move cursor");

	Draw_String     (left-40,  top+20, "esc");
	Draw_Alt_String (left, top+20, "close menu");

	if (menu_current != &menu_main)
	{
	Draw_String     (left-48, top+30, "bksp");
	Draw_Alt_String (left, top+30, "previous menu");
	}
		glColor3f (1, 1, 1);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
#endif


void M_DrawKeyLegend (void)
{
	double elapsed;
	char str[256], *keystr;
	const char *title;
	int  len, keylen;

	if ((menu_current == &menu_quit) || (menu_current == &menu_help))
		return;

	if (!menu_current->parentmenu)
		return;

	elapsed = realtime - m_close_time;
	if (elapsed > 4)
		return;			// show tip for 4 seconds only

	// solid for first 3.5s, then fade out
	if (elapsed > 3.5)
	{
		glDisable (GL_ALPHA_TEST);
		glEnable (GL_BLEND);
		glColor4f (1, 1, 1, 2*(4-elapsed));
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	title = (menu_current->parentmenu == &menu_main) ? "main" : menu_current->parentmenu->title;
	len = Q_snprintfz (str, sizeof(str), " returns to %s menu", title);
//	keystr = "<backspace>";
	keystr = va("<%s>", key_menuprev.string);
	keylen = strlen (keystr);
	len += keylen;
	Draw_String ((vid.width - len*DRAW_CHARWIDTH)/2, vid.height-20, keystr);
	Draw_Alt_String (keylen*DRAW_CHARWIDTH + (vid.width - len*DRAW_CHARWIDTH)/2, vid.height-20, str);

	if (elapsed > 3.5)
	{
		glEnable (GL_ALPHA_TEST);
		glDisable (GL_BLEND);
		glColor4f (1, 1, 1, 1);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
}

/*
=================
M_SetCurrentMenu
=================
*/
void M_SetCurrentMenu (menu_t *menu)
{
	menu_current = menu;
	key_dest = key_menu;
	menu->flags &= ~M_VAR_CHANGED;
	m_entersound = true;
}

qboolean M_IsReservedKey (int key)
{
	return (((key >= 32) && (key < 127)) ||	(key == K_TAB) ||		// used in Maps/Demos menus
			(key == K_UPARROW) || (key == K_DOWNARROW) ||
			(key == K_LEFTARROW) || (key == K_RIGHTARROW) ||
			(key == K_HOME) || (key == K_END) ||
			(key == K_PGUP) || (key == K_PGDN) ||
			(key == K_ENTER) || (key == K_DEL));
}

/*
===========
M_Menu_NavKey
  default handler for navigational keypresses
  (others get handled in M_Keydown)
===========
*/
qboolean M_Menu_NavKey (menu_t *menu, int key, int maxlines)
{
	menu_item_t *curr_item;

	switch (key)
	{
	case K_UPARROW:
		if (menu->cursor > 0)
		{
			S_LocalSound (gMenuSounds[0]);
			menu->cursor--;
			if (menu->cursor < menu->first_item)
				menu->first_item--;
			break;
		}
		else if (menu->num_items > maxlines)
			break;		// wraparound on a scrollable menu is confusing
		// else follow through to K_END
	case K_END:
		S_LocalSound (gMenuSounds[0]);
		menu->cursor = menu->num_items-1;
		if (menu->cursor > menu->first_item + maxlines-1)
		{
			menu->first_item = menu->cursor - (maxlines-1);
		}
		break;

	case K_DOWNARROW:
		if (menu->cursor < menu->num_items-1)
		{
			S_LocalSound (gMenuSounds[0]);
			menu->cursor++;
			if (menu->cursor > menu->first_item + maxlines-1)
			{
				menu->first_item = menu->cursor - (maxlines-1);
			}
			break;
		}
		else if (menu->num_items > maxlines)
			break;		// wraparound on a scrollable menu is confusing
		// else follow through to K_HOME
	case K_HOME:
		S_LocalSound (gMenuSounds[0]);
		menu->cursor = 0;
		menu->first_item = 0;
		break;

	case K_PGUP:
		S_LocalSound (gMenuSounds[0]);
		menu->cursor -= maxlines-1;
		if (menu->cursor < menu->first_item)
		{
			if (menu->cursor < 0)
				menu->cursor = 0;
			menu->first_item = menu->cursor;
		}
		break;

	case K_PGDN:
		S_LocalSound (gMenuSounds[0]);
		menu->cursor += maxlines-1;
		if (menu->cursor >= menu->num_items)
			menu->cursor = menu->num_items - 1;
		if (menu->cursor >= menu->first_item + maxlines-1)
		{
			menu->first_item = menu->cursor - (maxlines-1);
		}
		break;

	default:
		return false;
	}

	if (menu->cursor < M_MAX_ITEMS)
	{
		curr_item = &menu->items[menu->cursor];
		if (curr_item->flags & M_ITEM_DISABLED)
		{
			if (key == K_HOME || key == K_PGUP)
				key = K_DOWNARROW;
			if (key == K_END || key == K_PGDN)
				key = K_UPARROW;

			M_Menu_NavKey (menu, key, maxlines);
		}
	}

	return true;
}

//=============================================================================
/* MAIN MENU */

void M_Menu_Main_f (cmd_source_t src)
{
	menu_t *menu;

#ifdef HEXEN2_SUPPORT
	// ignore menu_main call in hexen.rc if changing gamedir to hexen2
	if (hexen2 && (src == SRC_CFG) && (host_time > 1.5))
		return;
#endif

//	if (key_dest != key_menu)
	if (!menu_current)			// changed 2010.02.18
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}

	if (nehahra)
	{
		if (NehGameType == NEH_TYPE_DEMO)
			menu = &menu_main_nehmovie;
		else if (NehGameType == NEH_TYPE_GAME)
			menu = &menu_main_nehgame;
		else
			menu = &menu_main_nehboth;
	}
	else menu = &menu_main;

	M_SetCurrentMenu (menu);

// This is in case gamedir has been switched to/from Nehahra
	if (menu_current->cursor >= menu_current->num_items)
		menu_current->cursor = menu_current->num_items-1;
}


void M_Main_Draw (void)
{
//	M_DrawTransPic (72, 32, Draw_GetCachePic("gfx/mainmenu.lmp", true));

	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, NULL);
}

qboolean M_Main_Close (void)
{
	key_dest = key_game;
	menu_current = NULL;
	cls.demonum = m_save_demonum;
	if (cls.demonum != -1 && !cls.demoplayback && cls.state != ca_connected)
		CL_NextDemo ();
	return true;
}

void M_ShowNehCredits (cmd_source_t src)
{
	key_dest = key_game;
	if (sv.active)
		Cbuf_AddText ("disconnect\n", src);
	Cbuf_AddText ("playdemo ENDCRED\n", src);
}

//=============================================================================
/* SINGLE PLAYER MENU */


void M_Menu_SinglePlayer_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_sp);
}


void M_SinglePlayer_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/ttl_sgl.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, NULL);

// This is in case gamedir has been switched
	while (menu_current->cursor >= menu_current->num_items)
		menu_current->cursor--;
}

void M_StartSPGame (cmd_source_t src)
{
	if (sv.active)
	{
		if (SCR_ModalMessage("Are you sure you want to\nstart a new game?\n\n(\x02Y/\x02N)", "YN") != 0)
			return;
	}

	key_dest = key_game;
	if (sv.active)
		Cbuf_AddText ("disconnect\n", src);
	Cbuf_AddText ("maxplayers 1\n", src);

	if (nehahra)
		Cbuf_AddText ("map nehstart\n", src);
#ifdef HEXEN2_SUPPORT
	else if (hexen2)
	{
		if (has_portals) m_enter_portals = 1;
		/*CL_RemoveGIPFiles(NULL);	*********** TEMP!!!! - this should go back in ***********/
		M_Menu_Class_f (src);
	}
#endif
	else
		Cbuf_AddText ("map start\n", src);
}

//=============================================================================
/* LOAD/SAVE MENU */

char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
int		loadable[MAX_SAVEGAMES];

void M_ScanSaves (menu_t *menu)
{
	int	i, j, version;
	char	name[MAX_OSPATH];
	FILE	*f;

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		menu->items[i].text = m_filenames[i];

	#ifdef HEXEN2_SUPPORT
		if (hexen2)
			Q_snprintfz (name, sizeof(name), "%s/s%i/info.dat", com_gamedir, i);
		else
	#endif
		Q_snprintfz (name, sizeof(name), "%s/s%i.sav", com_gamedir, i);

		if (!(f = fopen(name, "r")))
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		Q_strcpy (m_filenames[i], name, sizeof(m_filenames[i]));

	// change _ back to space
		for (j=0 ; j<SAVEGAME_COMMENT_LENGTH ; j++)
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		loadable[i] = true;
		fclose (f);
	}
}

void M_Menu_Load_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_load);
	M_ScanSaves (&menu_load);
}

void M_Menu_Save_f (cmd_source_t src)
{
	if (!sv.active || cl.intermission || svs.maxclients != 1)
		return;

	M_SetCurrentMenu (&menu_save);
	M_ScanSaves (&menu_save);
}

void M_Load_Draw (void)
{
	int	i;
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_load.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_DrawTitle (menu_current);

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
		M_Print (60, 32 + MITEMHEIGHT*i, m_filenames[i]);

	M_DrawMenuCursor(44, 32 + menu_current->cursor*MITEMHEIGHT);
}


void M_Save_Draw (void)
{
	int	i;
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_save.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_DrawTitle (menu_current);

	for (i=0 ; i<MAX_SAVEGAMES ; i++)
		M_Print (60, 32 + MITEMHEIGHT*i, m_filenames[i]);

	M_DrawMenuCursor(44, 32 + menu_current->cursor*MITEMHEIGHT);
}


qboolean M_Load_Key (int k, qboolean down)
{
	if (k == K_ENTER)
	{
		S_LocalSound (gMenuSounds[1]);
		if (loadable[menu_current->cursor])
		{
			key_dest = key_game;

		// Host_Loadgame_f can't bring up the loading plaque because too much
		// stack space has been used, so do it now
			SCR_BeginLoadingPlaque (NULL);

		// issue the load command
			Cbuf_AddText (va("load s%i\n", menu_current->cursor), SRC_COMMAND);
			//menu_current = NULL;
		}

		return true;
	}

	return false;
}


qboolean M_Save_Key (int k, qboolean down)
{
	if (k == K_ENTER)
	{
		key_dest = key_game;
		Cbuf_AddText (va("save s%i\n", menu_current->cursor), SRC_COMMAND);
		//menu_current = NULL;
		return true;
	}

	return false;
}

//=============================================================================
/* MULTIPLAYER MENU */


void M_Menu_MultiPlayer_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_mp);
}


void M_MultiPlayer_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_multi.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);
//	M_DrawTransPic (72, 32, Draw_GetCachePic("gfx/mp_menu.lmp", true));

	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, NULL);

//	if (serialAvailable || ipxAvailable || tcpipAvailable)
//	if (ipxAvailable || tcpipAvailable)
	if (tcpipAvailable)
		return;
	M_PrintWhite ((320 - (27*DRAW_CHARWIDTH))/2, 148, "No Communications Available");
}

void M_CheckEnter_NetMenu (cmd_source_t src)
{
	//if (serialAvailable || ipxAvailable || tcpipAvailable)	// JT021305 - remove serial
//			if (ipxAvailable || tcpipAvailable)
	if (tcpipAvailable)
		M_Menu_Net_f (src);
}

//=============================================================================
/* SETUP MENU */

int	setup_cursor_table[] = {40, 56, 80, 104, 140};

char	setup_hostname[16], setup_myname[16];
int	setup_oldtop, setup_oldbottom, setup_top, setup_bottom;

void M_Menu_Setup_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_setup);

	Q_strcpy(setup_myname, cl_name.string, sizeof(setup_myname));
	Q_strcpy(setup_hostname, hostname.string, sizeof(setup_hostname));
	setup_top = setup_oldtop = ((int)cl_color.value) >> 4;
	setup_bottom = setup_oldbottom = ((int)cl_color.value) & 15;
}

/*
=======================
M_BuildTranslationTable
=======================
*/
byte * M_BuildTranslationTable (int top, int bottom)
{
	static byte translationTable[256];
	byte	identityTable[256];
	int	j;
	byte	*dest, *source;

	for (j=0 ; j<256 ; j++)
		identityTable[j] = j;
	dest = translationTable;
	source = identityTable;
	memcpy (dest, source, 256);

// first 8 rows of palette [& row 15] have darkest color on the left;
// next 6 rows have darkest color on the right.
	if (top < 128)
		memcpy (dest + TOP_RANGE, source + top, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[TOP_RANGE+j] = source[top+15-j];

	if (bottom < 128)
		memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[BOTTOM_RANGE+j] = source[bottom+15-j];

	return translationTable;
}

/*
=======================
M_GetPlayerPic (JDH) [v0.95b2]
  - replaces mess of hacks in Draw_CachePic (Draw_AddCachePic) and Draw_TransPicTranslate
      --> loads player lmp, stores texels, and updates texture when needed
=======================
*/
mpic_t * M_GetPlayerPic (void)
{
	static mpic_t *pic;
	static int currtop = -1, currbottom = -1;
	static qpic_t *piclmp;

	unsigned *trans;
	byte *table, *src;
	int width, height, i;

	if (pic)
	{
		if ((setup_top == currtop) && (setup_bottom == currbottom))
			return pic;

		table = M_BuildTranslationTable (setup_top*16, setup_bottom*16);

		width = piclmp->width;
		height = piclmp->height;
		trans = malloc (width * height * 4);
		if (trans)
		{
			src = piclmp->data;
			for (i = width*height - 1; i >= 0; i--)
				trans[i] = d_8to24table[table[src[i]]];
/*
		#ifdef HEXEN2_SUPPORT
			pic->texnum = translate_texture + playertype;		// *** FIXME ***
		#else
*/			pic->texnum = translate_texture;
//		#endif

			GL_UpdatePicTexture (pic, trans);
			free (trans);
		}

		currtop = setup_top;
		currbottom = setup_bottom;
		return pic;
	}

	if (piclmp)
		free (piclmp);

	piclmp = (qpic_t *) COM_LoadMallocFile ("gfx/menuplyr.lmp", 0);
	if (!piclmp)
		return NULL;

	SwapPic (piclmp);
	pic = Draw_AddCachePic ("gfx/menuplyr.lmp", piclmp->data, piclmp->width, piclmp->height);
	return pic;
}

void M_Setup_Draw (void)
{
	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_multi.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_DrawTitle (menu_current);

	M_Print (64, 40, menu_current->items[0].text);
	M_DrawTextBox (160, 32, 16*DRAW_CHARWIDTH, DRAW_CHARHEIGHT);
	M_Print (168, 40, setup_hostname);

	M_Print (64, 56, menu_current->items[1].text);
	M_DrawTextBox (160, 48, 16*DRAW_CHARWIDTH, DRAW_CHARHEIGHT);
	M_Print (168, 56, setup_myname);

	M_Print (64, 80, menu_current->items[2].text);
	M_Print (64, 104, menu_current->items[3].text);

	M_DrawTextBox (60, 140-8, 14*DRAW_CHARWIDTH, DRAW_CHARHEIGHT);
	M_Print (68, 140, menu_current->items[4].text);

	p = Draw_GetCachePic ("gfx/bigbox.lmp", false);
	M_DrawTransPic (160, 64, p);

	p = M_GetPlayerPic ();
//	M_DrawTransPicTranslate (172, 72, p);
	M_DrawTransPic (172, 72, p);

	M_DrawMenuCursor(50, setup_cursor_table [menu_current->cursor]);

	if (menu_current->cursor == 0)
		M_DrawCharacter (168 + DRAW_CHARWIDTH*strlen(setup_hostname), setup_cursor_table [menu_current->cursor], 10+((int)(realtime*4)&1));

	if (menu_current->cursor == 1)
		M_DrawCharacter (168 + DRAW_CHARWIDTH*strlen(setup_myname), setup_cursor_table [menu_current->cursor], 10+((int)(realtime*4)&1));
}


qboolean M_Setup_Key (int k, qboolean down)
{
	int	l;

//	if (M_Menu_NavKey(menu_current, k, M_MAX_ITEMS))
//		goto SETUP_BOUND;

	switch (k)
	{
	case K_LEFTARROW:
		if (menu_current->cursor < 2)
			return true;
		S_LocalSound (gMenuSounds[2]);
		if (menu_current->cursor == 2)
			setup_top--;
		if (menu_current->cursor == 3)
			setup_bottom--;
		break;

	case K_RIGHTARROW:
		if (menu_current->cursor < 2)
			return true;
//forward:
		S_LocalSound (gMenuSounds[2]);
		if (menu_current->cursor == 2)
			setup_top++;
		if (menu_current->cursor == 3)
			setup_bottom++;
		break;

	case K_ENTER:
		if (menu_current->cursor == 0 || menu_current->cursor == 1)
		{
			menu_current->cursor++;
			return true;
		}

		if (menu_current->cursor == 2 || menu_current->cursor == 3)
			return true;	//goto forward;

		// menu_current->cursor == 4 (OK)
		if (strcmp(cl_name.string, setup_myname))
			Cbuf_AddText (va("name \"%s\"\n", setup_myname), SRC_COMMAND);
		if (strcmp(hostname.string, setup_hostname))
			Cvar_SetDirect (&hostname, setup_hostname);
		if (setup_top != setup_oldtop || setup_bottom != setup_oldbottom)
			Cbuf_AddText(va("color %i %i\n", setup_top, setup_bottom), SRC_COMMAND);
//		m_entersound = true;
		menu_current->parentmenu->openproc (SRC_COMMAND);	//M_Menu_MultiPlayer_f ();
		return true;

	case K_BACKSPACE:
		if (menu_current->cursor == 0)
		{
			if (strlen(setup_hostname))
				setup_hostname[strlen(setup_hostname)-1] = 0;
			return true;
		}
		if (menu_current->cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
			return true;
		}
		return false;

	default:
		if (k < 32 || k > 127)
			return false;
		if (menu_current->cursor == 0)
		{
			l = strlen(setup_hostname);
			if (l < 15)
			{
				setup_hostname[l+1] = 0;
				setup_hostname[l] = k;
			}
		}
		if (menu_current->cursor == 1)
		{
			l = strlen(setup_myname);
			if (l < 15)
			{
				setup_myname[l+1] = 0;
				setup_myname[l] = k;
			}
		}
		return true;
	}

//SETUP_BOUND:
	if (setup_top > 13)
		setup_top = 0;
	if (setup_top < 0)
		setup_top = 13;
	if (setup_bottom > 13)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 13;
	return true;
}

//=============================================================================
/* NET MENU */

int m_net_saveHeight;

char *net_helpMessage[] =
{
/* .........1.........2.... */
/*  "                        ",
  " Two computers connected",
  "   through two modems.  ",
  "                        ",

  "                        ",
  " Two computers connected",
  " by a null-modem cable. ",
  "                        ",*/

  " Novell network LANs    ",
  " or Windows 95 DOS-box. ",
  "                        ",
  "(LAN=Local Area Network)",

  " Commonly used to play  ",
  " over the Internet, but ",
  " also used on a Local   ",
  " Area Network.          "

/*  "                        ",
  "   Novell network LANs  ",
  "                        ",
  "                        ",

  "                        ",
  "   TCPIP Internet/LAN   ",
  "                        ",
  "                        "*/

};

void M_InitNetItem (menu_item_t *item, qboolean available)
{
	if (available)
	{
		item->flags |= M_ITEM_WHITE;
		item->flags &= ~M_ITEM_DISABLED;
	}
	else
	{
		item->flags &= ~M_ITEM_WHITE;
		item->flags |= M_ITEM_DISABLED;
	}
}

void M_Menu_Net_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_net);

//	M_InitNetItem (&menu_net.items[0], serialAvailable);
//	M_InitNetItem (&menu_net.items[1], serialAvailable);

	M_InitNetItem (&menu_net.items[0], ipxAvailable);
	M_InitNetItem (&menu_net.items[1], tcpipAvailable);

	if (menu_net.cursor >= menu_net.num_items)
		menu_net.cursor = 0;
	menu_net.cursor--;
	M_Menu_NavKey (menu_current, K_DOWNARROW, MAX_VIEWITEMS);
}

#if 1
void M_Net_Draw (void)
{
	int f;

	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, NULL);

#else
}

void M_Net_Draw (void)
{
	int	f;
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_multi.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_DrawTitle (menu_current);

	f = 40;		// was 32
/*	if (serialAvailable)
	{
		p = Draw_GetCachePic ("gfx/netmen1.lmp", true);
	}
	else
	{
#ifdef _WIN32
		p = NULL;
#else
		p = Draw_GetCachePic ("gfx/dim_modm.lmp", true);
#endif
	}

	if (p)
		M_DrawTransPic (72, f, p);

	f += 19;
	if (serialAvailable)
	{
		p = Draw_GetCachePic ("gfx/netmen2.lmp", true);
	}
	else
	{
#ifdef _WIN32
		p = NULL;
#else
		p = Draw_GetCachePic ("gfx/dim_drct.lmp", true);
#endif
	}

	if (p)
		M_DrawTransPic (72, f, p);*/

	f += 19;
	if (ipxAvailable)
//		p = Draw_GetCachePic ("gfx/netmen3.lmp", true);
// JT021205 - m_print menu system
//  glColor3f(0.9,0,0);
  M_PrintWhite (76, f, "IPX");
//  glColor3f(0.8,0.8,0.8);
  //  glColor3f(1,1,1);
// end new code
	else
//		p = Draw_GetCachePic ("gfx/dim_ipx.lmp", true);
//	M_DrawTransPic (72, f, p);
  M_Print (76, f, "IPX");

	f += 19;
	if (tcpipAvailable)
//		p = Draw_GetCachePic ("gfx/netmen4.lmp", true);
  M_PrintWhite (76, f, "TCP/IP");
	else
//		p = Draw_GetCachePic ("gfx/dim_tcp.lmp", true);
  M_Print (76, f, "TCP/IP");
//	M_DrawTransPic (72, f, p);

/*	if (m_net_items == 5)	// JDC, could just be removed
	{
		f += 19;
		p = Draw_GetCachePic ("gfx/netmen5.lmp", true);
		M_DrawTransPic (72, f, p);
	}*/

	M_DrawMenuCursor (60, 59 + menu_current->cursor*20);

#endif
	f = (320 - 26 * 8) / 2;
	M_DrawTextBox (f, 134, 24*8, 4*8);

	f += 8;		// 8 to 16?
	M_Print (f, 142, net_helpMessage[menu_current->cursor*4+0]);
	M_Print (f, 150, net_helpMessage[menu_current->cursor*4+1]);
	M_Print (f, 158, net_helpMessage[menu_current->cursor*4+2]);
	M_Print (f, 166, net_helpMessage[menu_current->cursor*4+3]);
}

//=============================================================================
/* OPTIONS MENU */

#define	SLIDER_RANGE	10

typedef struct
{
	char *simple_name;
	char *full_name;
} m_texturemode_t;

static const m_texturemode_t popular_filters[] =
{
	{"none",        "GL_NEAREST"},
	{"linear mips", "GL_NEAREST_MIPMAP_LINEAR"},
	{"bilinear",    "GL_LINEAR_MIPMAP_NEAREST"},
	{"trilinear",   "GL_LINEAR_MIPMAP_LINEAR"}
};

#define NUM_FILTERS (sizeof(popular_filters)/sizeof(m_texturemode_t))

void M_Menu_Options_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_options);
}

void M_CycleTextureMode (int dir)
{
	int i/*, newval = 0*/;

	for (i=0 ; i<NUM_FILTERS ; i++)
	{
		if (!Q_strcasecmp(popular_filters[i].full_name, gl_texturemode.string))
		{
			i = (i+dir+NUM_FILTERS) % NUM_FILTERS;
			break;
		}
	}

	/*if (i >= NUM_FILTERS)
		i = 0;
	else if (dir > 0)
		i = (i+1) % NUM_FILTERS;
	else if (i == 0)
		i = NUM_FILTERS-1;
	else
		i--;*/

	Cvar_SetDirect (&gl_texturemode, popular_filters[i].full_name);
}

void M_CycleImage (cvar_t *var, int dir)
{
	int i = (int)var->value;

	i = (i+dir+num_files+1) % (num_files+1);

	/*if (dir > 0)
		i = (i+1) % (num_files+1);
	else if (i == 0)
		i = num_files;
	else
		i--;*/

	Cvar_SetDirect (var, (i ? filelist[i-1].name : var->defaultvalue));
	var->value = i;
}

void M_Cycle_sv_protocol (int dir)
{
	int val, newval, i;

	val = (int) sv_protocol.value;
	newval = 0;

	for (i = 0; i < SV_NUM_PROTOCOLS; i++)
	{
		if (val == sv_valid_protocols[i].value)
		{
			newval = (i+dir+SV_NUM_PROTOCOLS) % SV_NUM_PROTOCOLS;
				// add SV_NUM_PROTOCOLS since modulus doesn't work on negative values
			break;
		}
	}

	Cvar_SetValueDirect (&sv_protocol, sv_valid_protocols[newval].value);
}

void M_Cycle_SshotFmt (int dir)
{
	int i, newval = 0;

	for (i = 0; i < SCR_NUM_SSHOT_EXTS; i++)
	{
		if (!Q_strcasecmp(scr_sshot_exts[i], scr_sshot_format.string))
		{
			newval = (i+dir+SCR_NUM_SSHOT_EXTS) % SCR_NUM_SSHOT_EXTS;
			break;
		}
	}

	Cvar_SetDirect (&scr_sshot_format, scr_sshot_exts[newval]);
}

void M_AdjustSliders (menu_t *menu, int dir, int key)
{
	cvar_t *var;
	float newval;

	var = menu->items[menu->cursor].cvar;
	if (!var) return;

	if ((key == K_ENTER) && (menu->items[menu->cursor].flags & M_ITEM_SLIDER))
	{
#if 0
	// exclude certain cvars from check - the settings act as 2- or 3-state variables,
	// even though their cvars are of type float or string.   FIXME: this is ugly!

		if ((var != &cl_forwardspeed) && (var != &m_pitch) && (var != &scr_sshot_format) && (var != &gl_texturemode))
			if ((var->type != CVAR_INT) || (var->maxvalue == CVAR_UNKNOWN_MAX) || (var->maxvalue - var->minvalue > 4))
#endif
				return;
	}

	S_LocalSound (gMenuSounds[2]);

	if (var == &scr_viewsize)							// screen size
	{
		Cvar_CycleValue (var, (dir < 0), false);
		/*scr_viewsize.value += dir * 10;
		scr_viewsize.value = bound (30, scr_viewsize.value, 120);
		Cvar_SetValueDirect (&scr_viewsize, scr_viewsize.value);*/
	}
	else if (var == &v_gamma)							// gamma
	{
		newval = v_gamma.value - dir * 0.05;
		newval = bound (0.5, newval, 1);
		Cvar_SetValueDirect (&v_gamma, newval);
	}
	else if (var == &v_contrast)						// contrast
	{
		newval = v_contrast.value + dir * 0.1;
		newval = bound (1, newval, 2);
		Cvar_SetValueDirect (&v_contrast, newval);
	}
	else if (var == &jpeg_compression_level)
	{
		newval = jpeg_compression_level.value + 5*dir;
		newval = bound(jpeg_compression_level.minvalue, newval, jpeg_compression_level.maxvalue);
		Cvar_SetValueDirect (&jpeg_compression_level, newval);
	}
	else if (var == &sensitivity)						// mouse speed
	{
		newval = sensitivity.value + dir * 0.5;
		newval = bound (1, newval, 11);
		Cvar_SetValueDirect (&sensitivity, newval);
	}
	else if (var == &cl_forwardspeed)					// always run
	{
		if (cl_forwardspeed.value > 200)
		{
			Cvar_SetDirect (&cl_forwardspeed, "200");
			Cvar_SetDirect (&cl_backspeed, "200");
		}
		else
		{
			Cvar_SetDirect (&cl_forwardspeed, "400");
			Cvar_SetDirect (&cl_backspeed, "400");
		}
	}
	else if (var == &m_pitch)							// invert mouse
	{
		Cvar_SetValueDirect (&m_pitch, -m_pitch.value);
	}
	else if (var == &scr_menusize)
	{
		newval = var->value + dir * 10;
		newval = bound (0, newval, 100);
		Cvar_SetValueDirect (var, newval);
	}
	else if (var == &scr_conspeed)
	{
		newval = var->value + dir * 100;
		newval = bound (100, newval, 1000);
		Cvar_SetValueDirect (var, newval);
	}
	else if ((var == &vid_vsync) || (var == &lookspring) || (var == &lookstrafe) || (var == &m_look))
	{
		Cvar_SetValueDirect (var, !var->value);
	}
	else if (var == &gl_texturemode)
	{
		M_CycleTextureMode (dir);
	}
	else if (var == &gl_picmip)
	{
		newval = gl_picmip.value - dir;
		newval = bound(0, newval, 4);
		Cvar_SetValueDirect (&gl_picmip, newval);
	}
	else if ((var == &cl_crossx) || (var == &cl_crossy))
	{
		newval = var->value + 4*dir;
		newval = bound (var->minvalue, newval, var->maxvalue);
		Cvar_SetValueDirect (var, newval);
	}
	else if (var == &crosshairsize)
	{
		newval = var->value + 0.5*dir;
		newval = bound (var->minvalue, newval, var->maxvalue);
		Cvar_SetValueDirect (var, newval);
	}
	else if ((var == &scr_hudscale) || (var == &scr_notifyscale))
	{
		newval = var->value + 0.25*dir;
		newval = bound (var->minvalue, newval, var->maxvalue);
		Cvar_SetValueDirect (var, newval);
	}
	else if (var == &con_notifytime)
	{
		newval = var->value + 0.5*dir;
		newval = bound (var->minvalue, newval, var->maxvalue);
		Cvar_SetValueDirect (var, newval);
	}
	else if ((var == &gl_consolefont) || (var == &gl_crosshairimage) || (var == &r_skybox))
	{
		M_CycleImage (var, dir);
	}
	else if (var == &sv_protocol)
	{
		M_Cycle_sv_protocol (dir);
	}
	else if (var == &scr_sshot_format)
	{
		M_Cycle_SshotFmt (dir);
	}
//	else if ((var == &bgmvolume) || (var == &volume) || (var == &r_shadows) ||
//		     (var == &gl_waterfog_density) || (var == &r_wateralpha))
	/*else if ((var->type == CVAR_FLOAT) && (var->minvalue == 0) && (var->maxvalue == 1))
	{
		newval = var->value + dir / (float) SLIDER_RANGE;
		newval = bound (0, newval, 1);
		Cvar_SetValueDirect (var, newval);
	}*/
	else if (var->type == CVAR_FLOAT)
	{
		newval = var->value + dir * (var->maxvalue - var->minvalue) / (float) SLIDER_RANGE;
		newval = bound (var->minvalue, newval, var->maxvalue);
		Cvar_SetValueDirect (var, newval);
	}
	else if (var->type == CVAR_INT)
	{
		Cvar_CycleValue (var, (dir < 0), true);
	}

	menu->flags |= M_VAR_CHANGED;
}

void M_DrawSlider (int x, int y, float range)
{
	int	basechar, i;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		basechar = 256;
	else
#endif
		basechar = 128;

	range = bound(0, range, 1);
	M_DrawCharacter (x-DRAW_CHARWIDTH, y, basechar);
	for (i=0 ; i<SLIDER_RANGE ; i++)
		M_DrawCharacter (x + i*DRAW_CHARWIDTH, y, basechar+1);
	M_DrawCharacter (x+i*DRAW_CHARWIDTH, y, basechar+2);

	M_DrawCharacter (x + (SLIDER_RANGE-1)*DRAW_CHARWIDTH * range, y, basechar+3);
}

void M_DrawCheckbox (int x, int y, int on)
{
	M_Print (x, y, (on ? "on" : "off"));
}

void M_Options_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_option.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, NULL);
	return;
}

void M_Options_GoToConsole (cmd_source_t src)
{
//	menu_current = NULL;
//	key_dest = key_console;
	Con_ToggleConsole_f (src);
}

void M_Options_Reset (cmd_source_t src)
{
	if (SCR_ModalMessage ("Are you sure you want to reset\nvariables and key bindings?\n\n(\x02Y/\x02N)", "YN") == 0)
	{
		Cbuf_AddText ("exec default.cfg\n", src);
	}
}

//=============================================================================
/* common code for preset configs in Options menus */

int M_GetCurrentPreset (m_preset_config_t *config)
{
	int i, j;

// Based on the current values of the cvars, determine the corresponding preset mode
	for (i = 0; i < config->num_configs-1; i++)
	{
		for (j = 0; j < config->num_vars; j++)
		{
			if (config->presets[j].cvar->value != config->presets[j].value[i])
				break;
		}

		if (j == config->num_vars)
			return i;
	}

	return config->num_configs-1;			// custom
}

void M_RecheckPresets (m_preset_config_t *config)
{
	int mode, j;

	mode = M_GetCurrentPreset (config);
	if (mode == config->num_configs-1)		// custom
	{
		config->cvar->maxvalue = mode;

		//if (!config->custom_valid)
		{
		// save the current values to the "custom" config preset
			for (j = 0; j < config->num_vars; j++)
				config->presets[j].value[mode] = config->presets[j].cvar->value;

			config->custom_valid = true;
		}
	}
	/*else if (config->cvar->value == config->num_configs-1)
	{
		// "custom" preset was selected, and user changed a value
		//  such that settings now match one of the presets
		config->cvar->maxvalue = config->num_configs-2;
	//	config->custom_valid = false;
	}*/
	else config->cvar->maxvalue = (config->custom_valid ? config->num_configs-1 : config->num_configs-2);

	config->cvar->value = mode;		// set directly so OnChange func is not invoked
}

qboolean M_OnChange_preset (m_preset_config_t *config, const char *value)
{
	int		preset, j;
	float	presetval;
	cvar_t	*cvar;

	preset = Q_atof (value);
	for (j = 0; j < config->num_vars; j++)
	{
	//	Cvar_SetValueDirect (config->presets[j].cvar, config->presets[j].value[preset]);
		cvar = config->presets[j].cvar;
		presetval = config->presets[j].value[preset];

		if (cvar == &gl_texturemode)
		{
			Cvar_SetDirect (cvar, popular_filters[(int)presetval].full_name);
			cvar->value = presetval;
		}
		else
			Cvar_SetValueDirect (cvar, presetval);
	}

	return false;		// allow change
}

//=============================================================================
/* VIDEO OPTIONS MENU */

#define M_RENDER_PRESET_TIP "(see reQuiem.txt for information)"

m_preset_t m_render_preset_vars[] =
{
	{&gl_texturemode,              {0,   1,   2,   3}},		// index into popular_filters array
	{&gl_picmip,                   {3,   0,   0,   0}},
	{&gl_detail,                   {0,   0,   1,   1}},
	{&gl_externaltextures_world,   {0,   0,   1,   1}},
	{&gl_externaltextures_bmodels, {0,   0,   1,   1}},
	{&gl_externaltextures_models,  {0,   0,   1,   1}},
	{&gl_fb_world,                 {0,   1,   1,   1}},
	{&gl_fb_bmodels,               {0,   1,   1,   1}},
	{&gl_fb_models,                {0,   1,   1,   1}},
	{&crosshair,                   {0,   0,   1,   1}},
	{&gl_loadlitfiles,             {0,   0,   1,   1}},
	{&r_dynamic,                   {0,   1,   1,   1}},
	{&gl_vertexlights,             {0,   0,   0,   1}},
	{&r_shadows,                   {0,   0,   1,   1}},
	{&gl_glows,                    {0,   0,   1,   1}},
	{&gl_flashblend,               {0,   0,   0,   1}},
	{&r_powerupglow,               {0,   0,   0,   1}},
	{&r_skytype,                   {2,   1,   0,   0}},
	{&r_oldsky,                    {1,   1,   0,   0}},
	{&gl_caustics,                 {0,   0,   1,   1}},
	{&gl_waterfog,                 {0,   0,   0,   1}},
	{&r_wateralpha,              {1.0, 1.0, 0.4, 0.4}},
	{&particle_preset,             {0,   0,   0,   1}},
	{&gl_interpolate_animation,    {0,   0,   1,   1}}
};


m_preset_config_t m_render_configs =
{
	&render_preset, 5, {"Fast", "Classic", "Standard", "Max FX", "custom"}, false,
	sizeof(m_render_preset_vars)/sizeof(m_preset_t), m_render_preset_vars
};

qboolean M_OnChange_render_preset (cvar_t *var, const char *value)
{
	return M_OnChange_preset (&m_render_configs, value);
}

void M_Menu_Video_f (cmd_source_t src)
{
	int i;

	M_SetCurrentMenu (&menu_video);

// find the corresponding value for the gl_texturemode string
	gl_texturemode.value = NUM_FILTERS;
	for (i = 0; i < NUM_FILTERS; i++)
	{
		if (!Q_strcasecmp(gl_texturemode.string, popular_filters[i].full_name))
		{
			gl_texturemode.value = i;
			break;
		}
	}

	M_RecheckPresets (&m_render_configs);
}

void M_Video_DrawVar (cvar_t *var, int y, qboolean selected)
{
	if (var == &render_preset)
		M_PrintWhite (M_VARLEFT, y, m_render_configs.config_names[(int) var->value]);
}

void M_Video_Draw (void)
{
	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, M_Video_DrawVar);

	if (menu_current->items[menu_current->cursor].cvar == &render_preset)
	{
		M_PrintWhite ((320 - strlen(M_RENDER_PRESET_TIP)*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y, M_RENDER_PRESET_TIP);
	}
}

/*
qboolean M_Video_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		break;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		break;

	default:
		return false;
	}

	return true;
}
*/
//=============================================================================
/* AUDIO OPTIONS MENU */

void M_Menu_Audio_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_audio);
}

void M_Audio_DrawVar (cvar_t *var, int y, qboolean selected)
{
	if (var == &nosound)
		M_DrawCheckbox (M_VARLEFT, y, nosound.value);
	else
		M_DrawSlider (M_VARLEFT, y, var->value);
}

void M_Audio_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_option.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_Audio_DrawVar);
}

/*qboolean M_Audio_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}

	return false;
}
*/
//=============================================================================
/* CONTROLS MENU */

void M_Menu_Controls_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_controls);

#ifdef _WIN32
	if (modestate == MS_WINDOWED)
#else
	if (vid_windowedmouse)
#endif
		menu_current->num_items = M_CONTROLS_NUMITEMS;
	else
		menu_current->num_items = M_CONTROLS_NUMITEMS-1;
}

void M_Controls_DrawVar (cvar_t *var, int y, qboolean selected)
{
	float	r;

	if (var == &sensitivity)
	{
		r = (var->value - 1)/10;
		M_DrawSlider (M_VARLEFT, y, r);
	}
	else if (var == &cl_forwardspeed)
	{
		M_DrawCheckbox (M_VARLEFT, y, (var->value > 200));
	}
	else if (var == &m_pitch)
	{
		M_DrawCheckbox (M_VARLEFT, y, var->value < 0);
	}
	else if (var == &_windowed_mouse)
	{
//		M_DrawCheckbox (M_VARLEFT, y, _windowed_mouse.value);
		M_Print (M_VARLEFT, y, (!var->value ? "off" : (var->value == 1) ? "auto" : "on"));
	}
	else		// lookspring, lookstrafe, always mouselook
	{
		M_DrawCheckbox (M_VARLEFT, y, var->value);
	}
}

void M_Controls_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_option.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_Controls_DrawVar);
}

/*
qboolean M_Controls_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}

	return false;
}
*/
//=============================================================================
/* helper functions for image-selection cvars (gl_consolefont, r_skybox, gl_crosshairimage) */

// callback for COM_FindAllFiles in M_FindCvarImages
int M_AddImageFile (com_fileinfo_t *fileinfo, int count, unsigned skybox)
{
	char	filename[MAX_QPATH];

	COM_StripExtension (fileinfo->name, filename, sizeof(filename));

	if (skybox)
	{
		if (!Sky_GetBaseName (filename))
			return 0;
	}

	if (!Cmd_CheckEntryName (filename))
	{
		Cmd_AddFilelistEntry (filename, CMD_FTYPE_FILE, 0, 0);
	}

	return 0;		// continue searching
}

void M_FindCvarImages (cvar_t *var, const char *pathlist[], qboolean skybox)
{
	int		i;

	Cmd_ClearFilelist ();
	if (!no24bit)
		COM_FindAllFiles (pathlist, "*", FILE_ANY_IMG, M_AddImageFile, skybox);

	if (var->string[0] && Q_strcasecmp(var->string, var->defaultvalue))
	{
		for (i = 0; i < num_files; i++)
		{
			if (COM_FilenamesEqual (var->string, filelist[i].name))
			{
				var->value = i+1;
				return;
			}
		}

		i = Cmd_AddFilelistEntry (var->string, CMD_FTYPE_FILE, 0, 0);
		var->value = i+1;
	}
	else var->value = 0;
}

void M_DrawImageCvar (cvar_t *var, int y)
{
	char *str;

	if (!var->value)
		str = (num_files ? "<none>" : "<none found>");
	else
		str = filelist[(int)var->value-1].name;

	M_Print (M_VARLEFT, y, str);
}

//=============================================================================
/* JDH: stuff for handling key combos & key cvars */

double m_bind_endtime;		// when keybind action times out

#define KEYMOD_LALT    0x00100
#define KEYMOD_RALT    0x00200
#define KEYMOD_ALT     0x00400
#define KEYMOD_LCTRL   0x01000
#define KEYMOD_RCTRL   0x02000
#define KEYMOD_CTRL    0x04000
#define KEYMOD_LSHIFT  0x10000
#define KEYMOD_RSHIFT  0x20000
#define KEYMOD_SHIFT   0x40000

#define NUM_KEYMODS 3

//static const char *M_KEYBIND_HINT = "Enter to change, backspace to clear";
static const char *M_KEYBIND_HINT = "Enter to change, delete to clear";

typedef struct
{
	int key;
	int bits;
} keymod_t;

keymod_t m_keymods[NUM_KEYMODS*3] =
{
	{K_ALT, KEYMOD_ALT},
	{K_LALT, KEYMOD_LALT},
	{K_RALT, KEYMOD_RALT},
	{K_CTRL, KEYMOD_CTRL},
	{K_LCTRL, KEYMOD_LCTRL},
	{K_RCTRL, KEYMOD_RCTRL},
	{K_SHIFT, KEYMOD_SHIFT},
	{K_LSHIFT, KEYMOD_LSHIFT},
	{K_RSHIFT, KEYMOD_RSHIFT}
};

qboolean M_IsModKey (int key)
{
	int i;

	for (i = 0; i < NUM_KEYMODS*3; i++)
	{
		if (key == m_keymods[i].key)
			return true;
	}

	return false;
}

qboolean M_KeyMatches (int currkey, cvar_t *keyvar)
{
	int targetkey = keyvar->value;

// can't do straight equality test because we want to accept keypress
// as long as currkey contains *at minimum* the modifiers specified in targetkey

	return (((currkey & 0xFF) == (targetkey & 0xFF)) &&
		(((currkey & 0xFFF00) & (targetkey & 0xFFF00)) == (targetkey & 0xFFF00)));
}

int M_StringToKeynum (const char *str)
{
	const char *start = str;
	char *end;
	int key, i, val = 0;

	while (1)
	{
		end = strchr (start, '+');
		if (!end)
			break;

		*end = 0;
		key = Key_StringToKeynum (start);
		*end = '+';

		for (i = 0; i < NUM_KEYMODS*3; i++)
		{
			if (key == m_keymods[i].key)
			{
				val |= m_keymods[i].bits;
				break;
			}
		}

		if (i == NUM_KEYMODS*3)
			return -1;
		start = end+1;
	}

	return (val | Key_StringToKeynum (start));
}

char * M_KeynumToString (int key)
{
	static char keystr[256];
	int len, i, j;

	keystr[0] = 0;
	len = 0;

	for (i = 0; i < NUM_KEYMODS; i++)
	{
		for (j = 1; j < 3; j++)		// just check L and R, not generic keymod
		{
			if ((key & m_keymods[i*3+j].bits) == m_keymods[i*3+j].bits)
			{
				len += Q_snprintfz (keystr+len, sizeof(keystr)-len, "%s+", Key_KeynumToString (m_keymods[i*3+j].key));
				break;
			}
		}
	}
/*
	if (key & KEYMOD_ALT)
		len += Q_snprintfz (keystr+len, sizeof(keystr)-len, "%s+", Key_KeynumToString (K_ALT));
	if (key & KEYMOD_CTRL)
		len += Q_snprintfz (keystr+len, sizeof(keystr)-len, "%s+", Key_KeynumToString (K_CTRL));
	if (key & KEYMOD_SHIFT)
		len += Q_snprintfz (keystr+len, sizeof(keystr)-len, "%s+", Key_KeynumToString (K_SHIFT));
*/
	Q_strcpy (keystr+len, Key_KeynumToString (key & 0xFF), sizeof(keystr)-len);
	return keystr;
}


qboolean M_OnChange_keyvar (cvar_t *var, const char *value)
{
	int key;
	char *s;

	if (!*value)
	{
		cvar_t *othervar = ((var == &key_menuclose) ? &key_menuprev : &key_menuclose);

		// allow clear only if the other key is set
		if (*othervar->string)
			return false;		// empty string is OK
		s = va ("%s: cannot unbind without first binding %s\n", var->name, othervar->name);
		goto KEY_NOCHANGE;
	}

	key = M_StringToKeynum (value);
	if (key < 0)
	{
		s = va ("%s: %s is not a recognized key\n", var->name, value);
		goto KEY_NOCHANGE;
	}

	if (M_IsReservedKey(key))
	{
		s = va ("%s: %s is reserved for use in menus\n", var->name, value);
		goto KEY_NOCHANGE;
	}

	return false;		// allow change

KEY_NOCHANGE:
	if (key_dest != key_menu)
		Con_Print (s);
	return true;		// returns true to prevent change
}

//=============================================================================
/* MENU/CONSOLE OPTIONS MENU */

void M_Menu_MenuConsole_f (cmd_source_t src)
{
	static const char *pathlist[2] = {"textures/charsets/", NULL};

	M_SetCurrentMenu (&menu_menuconsole);
	m_bind_grab = false;
	M_FindCvarImages (&gl_consolefont, pathlist, false);
}

void M_MenuConsole_DrawVar (cvar_t *var, int y, qboolean selected)
{
	const char *str;
	float maxval, r;

//	if ((var == &scr_centermenu) || (var == &gl_smoothfont))
	if ((var->type == CVAR_INT) && (var->maxvalue == 1) && (var->minvalue == 0))
	{
		M_DrawCheckbox (M_VARLEFT, y, var->value);
	}
	else if (var == &con_logcenterprint)
	{
		str = (!var->value ? "no" : (var->value >= 2) ? "SP+DM" : "SP only");
		M_Print (M_VARLEFT, y, str);
	}
	else if (var == &con_autocomplete)
	{
		str = (!var->value ? "off" : (var->value >= 2) ? "on" : "all but \"say\"");
		M_Print (M_VARLEFT, y, str);
	}
	else if (var == &cl_advancedcompletion)
	{
		str = (!var->value ? "basic" : (var->value >= 2) ? "partial" : "full+cycle");
		M_Print (M_VARLEFT, y, str);
	}
	else if (var == &gl_consolefont)
	{
		M_DrawImageCvar (&gl_consolefont, y);
	}
	else if ((var == &key_menuclose) || (var == &key_menuprev))
	{
		M_Print (M_VARLEFT, y, (*var->string ? var->string : "<none>"));

		if (selected)
		{
			int i, left;

			// find an empty line:
			for (i = menu_current->cursor + 1; i < menu_current->num_items; i++)
			{
				if (menu_current->items[i].flags & M_ITEM_DISABLED)
					break;
			}

			y = 32 + i*MITEMHEIGHT;
			left = (menu_current->flags & M_NO_QUAKELOGO) ? 0 : 32;
			left += (320-left-DRAW_CHARWIDTH*strlen(M_KEYBIND_HINT)) / 2;

			M_PrintWhite (left, y, M_KEYBIND_HINT);
		}
	}
	else		// scr_menusize, scr_consize, scr_conspeed, gl_conalpha, con_linespacing
	{
		if (var == &scr_conspeed)
			maxval = 1000.0;
		else
			maxval = var->maxvalue;

		r = (var->value - var->minvalue) / (maxval - var->minvalue);
		M_DrawSlider (M_VARLEFT, y, r);
	}
}

void M_MenuConsole_Draw (void)
{
	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, M_MenuConsole_DrawVar);

	if (m_bind_grab)
	{
		if (realtime >= m_bind_endtime)
		{
			m_bind_grab = false;
			return;
		}

		M_DrawTextBox (30, 42, 240, 42);
		if (m_bind_grab == 1)
		{
			M_PrintWhite (46, 56, "Press the key (or key combo)");
			M_PrintWhite (46, 66, " to be used for this action ");
		}
		else		// display warning for ~2s after invalid key is pressed
		{
			M_Print      (42, 61, "    That key is reserved!");
			if (realtime >= m_bind_endtime-3)
				m_bind_grab = 1;
		}

		if ((int)((m_bind_endtime-realtime)*4)&1)
		{
			M_DrawCharacter (146, 77, '>');
			M_DrawCharacter (162, 77, '<');
		}
		M_DrawCharacter (154, 77, '1'+(int)(m_bind_endtime-realtime));
	}
}

qboolean M_MenuConsole_Key (int k, qboolean down)
{
	cvar_t *var;
	int rawkey;

	var = menu_current->items[menu_current->cursor].cvar;
	if ((var != &key_menuclose) && (var != &key_menuprev))
		return false;

	if (m_bind_grab)
	{
		/*if (down)
		{
			if (!M_IsReservedKey(k))
			{
				m_bind_grab = false;
				Cvar_SetDirect (var, Key_KeynumToString (k));
			}
			return true;
		}*/

#ifdef _DEBUG
//		Con_DPrintf ("key %s: $%05X\n", down ? "down" : "up", k);
#endif
		// wait on keyup for alt/ctrl/shift, keydown for everything else
		rawkey = k & 0x00FF;
	//	if (((rawkey == K_ALT) || (rawkey == K_LALT) || (rawkey == K_RALT) ||
	//		(rawkey == K_CTRL) || (rawkey == K_LCTRL) || (rawkey == K_RCTRL) ||
	//		(rawkey == K_SHIFT) || (rawkey == K_LSHIFT) || (rawkey == K_RSHIFT)) == !down)
		if (M_IsModKey(rawkey) == !down)
		{
			if (M_IsReservedKey(k))			// reset timer & display warning
			{
				m_bind_endtime = realtime + 4.99;
				m_bind_grab = 2;
			}
			else
			{
				m_bind_grab = false;
				Cvar_SetDirect (var, M_KeynumToString (k));
			}
		}
		return true;
	}
	else if (down)
	{
		if (k == K_ENTER)
		{
			m_bind_grab = true;
			m_bind_endtime = realtime + 4.99;
			return true;
		}
		if (k == K_DEL)
		{
			// clear only if the other key is set
			if (((var == &key_menuclose) && *key_menuprev.string) ||
				((var == &key_menuprev) && *key_menuclose.string))
				Cvar_SetDirect (var, "");
			return true;
		}
	}

	return false;
}

//=============================================================================
// helper functions for exttex_preset and fbcolors_preset cvars,
// each of which control 3 other (actual) cvars

m_preset_t m_exttex_preset_vars[] =
{
	{&gl_externaltextures_world,   {0, 1}},
	{&gl_externaltextures_bmodels, {0, 1}},
	{&gl_externaltextures_models,  {0, 1}}
};

m_preset_t m_fbcolors_preset_vars[] =
{
	{&gl_fb_world,   {0, 1}},
	{&gl_fb_bmodels, {0, 1}},
	{&gl_fb_models,  {0, 1}}
};

m_preset_config_t m_exttex_configs =
{
	&exttex_preset, 3, {"off", "on", "custom"}, false,
	sizeof(m_exttex_preset_vars)/sizeof(m_preset_t), m_exttex_preset_vars
};

m_preset_config_t m_fbcolors_configs =
{
	&fbcolors_preset, 3, {"off", "on", "custom"}, false,
	sizeof(m_fbcolors_preset_vars)/sizeof(m_preset_t), m_fbcolors_preset_vars
};

void M_SetupTextureCvar  (m_preset_config_t *config)
{
	M_RecheckPresets (config);
}

void M_Setup_exttex_Cvar (void)
{
	M_SetupTextureCvar (&m_exttex_configs);
}

void M_Setup_fbcolors_Cvar (void)
{
	M_SetupTextureCvar (&m_fbcolors_configs);
}

qboolean M_OnChange_exttex_preset (cvar_t *var, const char *value)
{
	return M_OnChange_preset (&m_exttex_configs, value);
}

qboolean M_OnChange_fbcolors_preset (cvar_t *var, const char *value)
{
	return M_OnChange_preset (&m_fbcolors_configs, value);
}

//=============================================================================
/* COMPATIBILITY OPTIONS MENU */

// structure to store preset values for cvars:

#define M_COMPAT_TIP1 "Mappers/modders: test with SW/GL Quake"
#define M_COMPAT_TIP2    "setting to ensure compatibility!"

m_preset_t m_compat_preset_vars[] =
{
	{&com_matchfilecase, {1, 1, 0}},
	{&sv_entpatch,       {0, 0, 1}},
	{&sv_protocol,       {PROTOCOL_VERSION_STD, PROTOCOL_VERSION_STD, 0}},
	{&host_cutscenehack, {0, 0, 1}},
	{&gl_zfightfix,      {0, 0, 0}},
	{&sv_fishfix,        {0, 0, 1}},
	{&exttex_preset,     {0, 0, 1}},
	{&fbcolors_preset,   {1, 0, 1}},
	{&gl_notrans,        {1, 1, 0}},
	{&nospr32,           {1, 1, 0}}
};

m_preset_config_t m_compat_configs =
{
	&compat_preset, 4, {"SW Quake", "GL Quake", "reQuiem", "custom"}, false,
	sizeof(m_compat_preset_vars)/sizeof(m_preset_t), m_compat_preset_vars
};

qboolean M_OnChange_compat_preset (cvar_t *var, const char *value)
{
	return M_OnChange_preset (&m_compat_configs, value);
}

void M_Menu_Compat_f (cmd_source_t src)
{
	M_Setup_exttex_Cvar ();
	M_Setup_fbcolors_Cvar ();

	M_SetCurrentMenu (&menu_compat);

	M_RecheckPresets (&m_compat_configs);
}

void M_Compat_DrawVar (cvar_t *var, int y, qboolean selected)
{
	int i, val = var->value;

	if (var == &compat_preset)
	{
//		M_PrintWhite (M_VARLEFT, y, m_compat_preset_names[val]);
		M_PrintWhite (M_VARLEFT, y, m_compat_configs.config_names[val]);
	}
	else if ((var == &exttex_preset) || (var == &fbcolors_preset))
	{
		M_Print (M_VARLEFT, y, (!val ? "off" : (val == 1) ? "on" : "custom"));
	}
	else if (var == &sv_protocol)
	{
		for (i = SV_NUM_PROTOCOLS-1; i > 0; i--)
			if (val == sv_valid_protocols[i].value)
				break;

		M_Print (M_VARLEFT, y, sv_valid_protocols[i].name);
	}
	else if (var == &sv_imp12hack)
	{
		M_Print (M_VARLEFT, y, (!val ? "off" : (val >= 2) ? "force on" : "auto"));
	}
	else
	{
		if ((var == &gl_notrans) || (var == &nospr32))
			val = !val;

		M_DrawCheckbox (M_VARLEFT, y, val);
	}
}

void M_Compat_Draw (void)
{
	cvar_t		*cvar;
	m_preset_config_t	*config;
	int			i, len;
	char		info_str[40];

	if (menu_current->flags & M_VAR_CHANGED)
	{
		M_RecheckPresets (&m_compat_configs);
		menu_current->flags &= ~M_VAR_CHANGED;
	}

	M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_Compat_DrawVar);

	cvar = menu_current->items[menu_current->cursor].cvar;
	if ((cvar == &exttex_preset) || (cvar == &fbcolors_preset))
	{
		config = (cvar == &exttex_preset) ? &m_exttex_configs : &m_fbcolors_configs;
		for (i = 0; i < config->num_vars; i++)
		{
			cvar = config->presets[i].cvar;
			len = Q_snprintfz (info_str, sizeof(info_str), "%s = %s", cvar->name, cvar->string);
			M_PrintWhite ((320 - len*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y + (i-1)*10, info_str);
		}
	}
	else if ((cvar == &compat_preset) && (cvar->value > 1))
	{
		M_PrintWhite ((320 - strlen(M_COMPAT_TIP1)*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y, M_COMPAT_TIP1);
		M_PrintWhite ((320 - strlen(M_COMPAT_TIP2)*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y+10, M_COMPAT_TIP2);
	}
}

//=============================================================================
/* GAME OPTIONS MENU */

m_preset_t m_aim_preset_vars[1] =
{
	{&sv_aim,  {0.93f, 0.97f, 1}}
};

m_preset_config_t m_aim_configs =
{
	&aim_preset, 4, {"standard", "high", "max", "custom"}, false,
	sizeof(m_aim_preset_vars)/sizeof(m_preset_t), m_aim_preset_vars
};

menu_item_t *aim_preset_item = NULL;

qboolean M_OnChange_aim_preset (cvar_t *var, const char *value)
{
	return M_OnChange_preset (&m_aim_configs, value);
}

void M_Menu_Game_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_game);
	M_RecheckPresets (&m_aim_configs);
}

void M_Game_DrawVar (cvar_t *var, int y, qboolean selected)
{
	float val = var->value;

	if (var == &aim_preset)
	{
		M_Print (M_VARLEFT, y, m_aim_configs.config_names[(int)val]);
		// HACK!!  temporarily set item's cvar to sv_aim so cvar infoline gets drawn
		aim_preset_item = &menu_current->items[(y-32)/MITEMHEIGHT];
		if (selected)
			aim_preset_item->cvar = &sv_aim;
	}
	else if (var == &cl_deadbodyfilter)
	{
		M_Print (M_VARLEFT, y, (!val ? "off" : (val == 2) ? "immediate" : "delayed"));
	}
	else if (var == &cl_demo_compress)
	{
		M_Print (M_VARLEFT, y, (!val ? "off" : (val >= 2) ? "on" : "ask each time"));
	}
	else if (var == &cl_demo_compress_fmt)
	{
		M_Print (M_VARLEFT, y, (val == 1) ? "zip" : "dzip");
	}
	else if (var == &cl_truelightning)
	{
		M_DrawSlider (M_VARLEFT, y, val);
	}
	else	// v_gunkick, cl_gibfilter
	{
		M_DrawCheckbox (M_VARLEFT, y, val);
	}
}

void M_Game_Draw (void)
{
	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, M_Game_DrawVar);
	aim_preset_item->cvar = &aim_preset;
}

/*
qboolean M_Game_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}

	return false;
}
*/

//=============================================================================
/* KEY CONFIG MENUS */

typedef struct
{
	const char *cmd;
	const char *desc;
} keycmd_t;

keycmd_t keycmds_game[] =
{
	{"+attack", 		"Attack"},
	{"impulse 10", 		"Next weapon"},
	{"impulse 12",		"Prev weapon"},
	{"+forward", 		"Walk forward"},
	{"+back", 			"Backpedal"},
	{"+moveleft", 		"Step left"},
	{"+moveright", 		"Step right"},
	{"+speed", 			"Run"},
	{"+jump", 			"Jump / Swim up"},
	{"+strafe", 		"Strafe"},
	{"centerview", 		"Center view"},
	{"+mlook", 			"Mouselook on/off"},
	{"+moveup",			"Swim up"},
	{"+movedown",		"Swim down"},
	{"+left", 			"Turn left"},
	{"+right", 			"Turn right"},
	{"+lookup", 		"Look up"},
	{"+lookdown", 		"Look down"},
	{"+klook", 			"Keyboard look"},
	//{"impulse XX", 		"Add bot"},
	//{"impulse XX", 		"Remove bot"},

#ifdef HEXEN2_SUPPORT
	{"+crouch",			"crouch"},
	{"impulse 13", 		"lift object"},
	{"+infoplaque",		"objectives"},
	{"+showdm",			"info / frags"},
	{"toggle_dm",		"toggle frags"},
	{"+showinfo",		"full inventory"},
	{"invuse",			"use inv item"},
	{"impulse 44",		"drop inv item"},
	{"invleft",			"inv move left"},
	{"invright",		"inv move right"},
	{"impulse 100",		"inv:torch"},
	{"impulse 101",		"inv:qrtz flask"},
	{"impulse 102",		"inv:mystic urn"},
	{"impulse 103",		"inv:krater"},
	{"impulse 104",		"inv:chaos devc"},
	{"impulse 105",		"inv:tome power"},
	{"impulse 106",		"inv:summon stn"},
	{"impulse 107",		"inv:invisiblty"},
	{"impulse 108",		"inv:glyph"},
	{"impulse 109",		"inv:boots"},
	{"impulse 110",		"inv:repulsion"},
	{"impulse 111",		"inv:bo peep"},
	{"impulse 112",		"inv:flight"},
	{"impulse 113",		"inv:force cube"},
	{"impulse 114",		"inv:icon defn"}
#endif
};

#define	NUM_KEYCMDS_GAME		19

#ifdef HEXEN2_SUPPORT
#define	NUM_KEYCMDS_GAME_H2	(sizeof(keycmds_game)/sizeof(keycmds_game[0]))
#endif

static keycmd_t keycmds_other[] =
{
	{"toggleconsole",							"Toggle console"},
	{"screenshot",								"Screenshot"},
	{"echo Quicksaving...; wait; save quick",	"Quicksave"},
	{"echo Quickloading...; wait; load quick",	"Quickload"},
	{"menu_save",								"Save menu"},
	{"menu_load",								"Load menu"},
	{"menu_options",							"Options menu"},
	{"quit",									"Quit"}
};

#define	NUM_KEYCMDS_OTHER	(sizeof(keycmds_other)/sizeof(keycmds_other[0]))


void M_InitKeysMenu (menu50_t *menu, keycmd_t keycmds[], int numitems)
{
	int i;

	M_SetCurrentMenu ((menu_t *) menu);
	m_bind_grab = false;

	menu->num_items = numitems;

	for (i = 0; i < numitems; i++)
	{
		menu->items[i].text = keycmds[i].desc;
	// HACK!! cvar field needs to be non-null so that M_Keys_DrawBinding gets called.
	// So I just use the index
		menu->items[i].cvar = (cvar_t *) (i+1);
	}
}

void M_Menu_Keys_f (cmd_source_t src)
{
	int count;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		count = NUM_KEYCMDS_GAME_H2;
	else
#endif
		count = NUM_KEYCMDS_GAME;

	M_InitKeysMenu (&menu_keys, keycmds_game, count);
}

void M_Menu_Keys2_f (cmd_source_t src)
{
	M_InitKeysMenu (&menu_keys2, keycmds_other, NUM_KEYCMDS_OTHER);
}

void M_FindKeysForCommand (const char *command, int *twokeys)
{
	int	count, j, l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!Q_strncasecmp(b, command, l))		// JDH: was strncmp
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

void M_UnbindCommand (const char *command)
{
	int	j, l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!Q_strncasecmp(b, command, l))		// JDH: was strncmp
			Key_SetBinding (j, "");
	}
}

void M_Keys_DrawItem (cvar_t *var, int y, qboolean selected, keycmd_t keycmds[])
{
	int		keys[2], x;
	char	*name;

	M_FindKeysForCommand (keycmds[(int)var - 1].cmd, keys);

	if (keys[0] == -1)
	{
		M_Print (148, y, "???");
	}
	else
	{
		name = Key_KeynumToString (keys[0]);
		M_PrintWhite (148, y, name);
		x = strlen(name) * DRAW_CHARWIDTH;
		if (keys[1] != -1)
		{
			M_Print (148 + x + DRAW_CHARWIDTH, y, "or");
			M_PrintWhite (148 + x + 4*DRAW_CHARWIDTH, y, Key_KeynumToString (keys[1]));
		}
	}

	if (selected)
	{
		if (m_bind_grab)
			M_DrawCharacter (132, y, '=');
		else
			M_DrawMenuCursor (132, y);
	}
}

void M_Keys_DrawBinding (cvar_t *var, int y, qboolean selected)
{
	keycmd_t *keycmds;

	if (menu_current == (menu_t *)&menu_keys2)
		keycmds = keycmds_other;
	else
		keycmds = keycmds_game;

	M_Keys_DrawItem (var, y, selected, keycmds);
}

void M_Keys_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/ttl_cstm.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_Draw (menu_current, 56, MAX_VIEWITEMS+2, M_Keys_DrawBinding);

	if (m_bind_grab)
	{
		M_Print (12, 32, "Press a key or button for this action");

		if (m_bind_grab > 1)
		{
			if (m_bind_grab == 2)
				M_Print (48, M_CVARINFO_Y, "** That key is reserved! **");
			else
				M_Print (12, M_CVARINFO_Y, "** Key combos cannot be used here **");

			if (realtime >= m_bind_endtime)
				m_bind_grab = 1;
		}
	}
	else
		M_Print ((320 - DRAW_CHARWIDTH*strlen(M_KEYBIND_HINT)) / 2, 32, M_KEYBIND_HINT);

	M_DrawBar (8, 44, 38);
}

qboolean M_Keys_Close (void)
{
	if (m_bind_grab)
	{
		S_LocalSound (gMenuSounds[0]);
		m_bind_grab = false;
		return true;
	}
	//else menu_current->parentmenu->openproc ();	//M_Menu_Controls_f ();
	return false;
}

extern qboolean	consolekeys[256];

qboolean M_Keys_Key (int k, qboolean down)
{
	keycmd_t	*keycmds;
	char		cmd[80];
	const char	*cmdname;
	int			keys[2];

	if (menu_current == (menu_t *)&menu_keys2)
		keycmds = keycmds_other;
	else
		keycmds = keycmds_game;

	if (m_bind_grab)	// defining a key
	{
		//if (!menubound[k])
		if (k != K_ESCAPE)
		{
			S_LocalSound (gMenuSounds[0]);

			// Lock out some keys from being bound from the "Other Controls"
			// menu (except for the toggleconsole binding).
			cmdname = keycmds[menu_current->cursor].cmd;
			if ((keycmds == keycmds_other) && Q_strcasecmp(cmdname, "toggleconsole"))
			{
				if (k & 0xFFFFFF00)
				{
					m_bind_grab = 3;		// display "no key combos" warning
					m_bind_endtime = realtime + 2;
					return true;
				}

				if (consolekeys[k] || M_IsReservedKey(k))
				{
					m_bind_grab = 2;
					m_bind_endtime = realtime + 2;	// display warning for ~2s after invalid key is pressed
					return true;
				}
			}

			M_FindKeysForCommand (cmdname, keys);
			if (keys[1] != -1)
				M_UnbindCommand (cmdname);

			Q_snprintfz (cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), cmdname);
			Cbuf_InsertText (cmd, SRC_COMMAND);
		}

		m_bind_grab = false;
		return true;
	}

//	if (M_Menu_NavKey(menu_current, k, KEYS_SIZE))
//		goto KEYS_END;
//		return true;

	switch (k)
	{
	case K_ENTER:		// go into bind mode
		S_LocalSound (gMenuSounds[1]);
//		M_FindKeysForCommand (keycmds[menu_current->cursor].cmd, keys);
//		if (keys[1] != -1)
//			M_UnbindCommand (keycmds[menu_current->cursor].cmd);
		m_bind_grab = true;
		return true;

//	case K_BACKSPACE:
	case K_DEL:				// delete bindings
		S_LocalSound (gMenuSounds[1]);
		M_UnbindCommand (keycmds[menu_current->cursor].cmd);
		return true;

	case K_RIGHTARROW:
	case K_LEFTARROW:
		return true;		// otherwise M_AdjustSliders will get called
	}

	return false;
}

//=============================================================================
/* GENERAL VIDEO OPTIONS MENU */

void M_Menu_VidGeneral_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_vidgeneral);
	menu_current->flags |= M_VAR_CHANGED;		// so _Draw proc sets up sshot vars
}

void M_VidGeneral_DrawVar (cvar_t *var, int y, qboolean selected)
{
	float r;

//	if ((var == &vid_vsync) || (var == &gl_triplebuffer))
	if ((var->type == CVAR_INT) && (var->maxvalue == 1) && (var->minvalue == 0))
	{
		M_DrawCheckbox (M_VARLEFT, y, var->value);
	}
	else if ((var == &host_maxfps) || (var == &scr_sshot_format))
	{
		M_Print (M_VARLEFT, y, var->string);
	}
	else
	{
	// gamma and contrast by joe
		if (var == &v_gamma)
		{
			r = (1.0 - v_gamma.value) / 0.5;
		}
		else if (var == &v_contrast)
		{
			r = v_contrast.value - 1.0;
		}
		else
		{
			r = (var->value - var->minvalue) / (var->maxvalue - var->minvalue);
		}

		M_DrawSlider (M_VARLEFT, y, r);
	}
}

void M_VidGeneral_Draw (void)
{
	int num_items;
	menu_item_t *item;

	if (menu_current->flags & M_VAR_CHANGED)
	{
		num_items = menu_current->num_items;
		if (menu_current->items[num_items-1].cvar != &scr_sshot_format)
			num_items--;

		if (!Q_strcasecmp(scr_sshot_format.string, "jpg") || !Q_strcasecmp(scr_sshot_format.string, "jpeg"))
		{
			menu_current->num_items = num_items+1;
			item = &menu_current->items[num_items];
			item->cvar = &jpeg_compression_level;
			item->text = "JPG quality";
			item->flags = M_ITEM_SLIDER;
		}
		else if (!Q_strcasecmp(scr_sshot_format.string, "png"))
		{
			menu_current->num_items = num_items+1;
			item = &menu_current->items[num_items];
			item->cvar = &png_compression_level;
			item->text = "PNG compression";
			item->flags = M_ITEM_SLIDER;
		}
		else
			menu_current->num_items = num_items;

		menu_current->flags &= ~M_VAR_CHANGED;
	}

	M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_VidGeneral_DrawVar);
}

/*qboolean M_VidGeneral_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}

	return false;
}
*/
//=============================================================================
/* TEXTURE OPTIONS MENU */

#define M_TEX_PRESET_INFO  "(affects next 3 options)"

void M_Menu_Textures_f (cmd_source_t src)
{
	M_Setup_exttex_Cvar ();
	M_Setup_fbcolors_Cvar ();

	M_SetCurrentMenu (&menu_textures);
}

void M_Textures_DrawVar (cvar_t *cvar, int y, qboolean selected)
{
	int		i;
	float	r;
	char	*str;

	if (cvar == &gl_texturemode)
	{
		str = gl_texturemode.string;
		for (i = 0; i < NUM_FILTERS; i++)
		{
			if (!Q_strcasecmp (gl_texturemode.string, popular_filters[i].full_name))
			{
				str = popular_filters[i].simple_name;
				break;
			}
		}

		M_Print (M_VARLEFT, y, str);
	}
	else if (cvar == &gl_picmip)
	{
		r = (cvar->value - cvar->minvalue) / (cvar->maxvalue - cvar->minvalue);
		r = 1-r;		// quality goes up as value goes down

		M_DrawSlider (M_VARLEFT, y, r);
	}
	else if ((cvar == &exttex_preset) || (cvar == &fbcolors_preset))
	{
		M_PrintWhite (M_VARLEFT, y, (!cvar->value ? "off" : (cvar->value == 1) ? "on" : "custom"));

		if (selected)
			M_PrintWhite ((320 - strlen(M_TEX_PRESET_INFO)*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y, M_TEX_PRESET_INFO);
	}
	else
	{
		M_DrawCheckbox (M_VARLEFT, y, cvar->value);
	}
}

void M_Textures_Draw (void)
{
	if (menu_current->flags & M_VAR_CHANGED)
	{
	// make sure presets still reflect current settings
		M_Setup_exttex_Cvar ();
		M_Setup_fbcolors_Cvar ();
		menu_current->flags &= ~M_VAR_CHANGED;
	}

	M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, M_Textures_DrawVar);
}

/*qboolean M_Textures_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (menu_current, 1);
		break;

	case K_LEFTARROW:
		M_AdjustSliders (menu_current, -1);
		break;

	default:
		return false;
	}

	M_Setup_exttex_Cvar ();
	M_Setup_fbcolors_Cvar ();
	return true;
}
*/
//=============================================================================
/* HUD OPTIONS MENU */

void M_Menu_HUD_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_hud);
}

void M_HUD_DrawVar (cvar_t *var, int y, qboolean selected)
{
	float	r;

	if ((var->type == CVAR_INT) && (var->minvalue == 0) && (var->maxvalue == 1))
	{
		M_DrawCheckbox (M_VARLEFT, y, var->value);
	}
	else if (var == &con_notifytime)
	{
		M_Print (M_VARLEFT, y, va("%.1f sec", var->value));
	}
	else if (var == &_con_notifylines)
	{
		M_Print (M_VARLEFT, y, var->string);
	}
	else
	{
		r = (var->value - var->minvalue) / (var->maxvalue - var->minvalue);
		M_DrawSlider (M_VARLEFT, y, r);
	}
}

void M_HUD_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_option.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_HUD_DrawVar);
}
/*
qboolean M_HUD_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}

	return false;
}
*/
//=============================================================================
/* helper functions for color-selection cvars */

qboolean	m_palette_open = false;
byte		m_palette_custom_color[4];

void M_InitColorCvar (cvar_t *var)
{
	byte rgb[4];

	if ((var == &r_skycolor) && Cvar_IsDefaultValue(var))		// "auto"
	{
		m_palette_custom_color[3] = 0;
		var->value = 256;
	}
	else
	{
		if (StringToRGB (var->string, rgb) >= 3)
		{
			var->value = 271;		// last spot in bottom row
			m_palette_custom_color[0] = rgb[0];
			m_palette_custom_color[1] = rgb[1];
			m_palette_custom_color[2] = rgb[2];
			m_palette_custom_color[3] = 1;
		}
		else
		{
			var->value = atoi (var->string);
			m_palette_custom_color[3] = 0;
		}
	}

	m_palette_open = false;
}

void M_Draw_ColorChooser (cvar_t *var, int y)
{
	int		left, top, height, selnum, i;
	byte	rgb[4];

	left = (vid.width-320)/2;
	top =  m_yofs + 30;

	if (m_palette_open)		// draw the palette
	{
		left += 70;
		height = 16*10 + 3;
		if ((var == &r_skycolor) || m_palette_custom_color[3])
		{
			top -= 8;
			height += 10;		// extra row
		}

		Draw_Fill (left, top, 16*11 + 3, height, 0);		// black bg for palette

		/*if ((var == &r_skycolor) && Cvar_IsDefaultValue(var))		// "auto"
			selnum = 256;
		else
		{
			selnum = (int) var->value;
			if (selnum > 255)
				selnum = 271;
		}*/

		selnum = (int) var->value;

	// create a white frame by overlaying a smaller black box on a white one
		Draw_Fill (left + (selnum%16)*11 + 1, top + (selnum/16)*10 + 1, 12,  11, 254);
		Draw_Fill (left + (selnum%16)*11 + 2, top + (selnum/16)*10 + 2, 10, 9, 0);

		if (var == &r_skycolor)
		{
			Draw_String ((left + 4), top + 16*10 + 2, "*");
			Draw_String ((left + 11 + 6), top + 16*10 + 2, "automatic");
		}

		if (m_palette_custom_color[3])
		{
			Draw_String ((left + 15*11 + 3) - 8*8, top + 16*10 + 2, "Custom:");
			Draw_FillRGB (left + 15*11 + 3, top + 16*10 + 3, 8, 7, m_palette_custom_color);
		}

		for (i = 0; i < 256; i++)
		{
			Draw_Fill (left + (i%16)*11 + 3, top + (i/16)*10 + 3, 8, 7, i);
		}
	}
	else
	{
		if ((var == &r_skycolor) && Cvar_IsDefaultValue(var))		// "auto"
			M_Print (M_VARLEFT, y, var->defaultvalue);
		else
		{
			StringToRGB (var->string, rgb);
	//		Draw_Fill (left + M_VARLEFT, top + itemnum*MITEMHEIGHT + 2, 12, MITEMHEIGHT-2, 254);
	//		Draw_FillRGB (left + 221, top + itemnum*MITEMHEIGHT + 3, 10, MITEMHEIGHT-4, rgb);
			Draw_Fill (left + M_VARLEFT, m_yofs + y, 12, MITEMHEIGHT-2, 254);
			Draw_FillRGB (left + M_VARLEFT + 1, m_yofs + y + 1, 10, MITEMHEIGHT-4, rgb);
		}
	}
}

qboolean M_ColorChooser_Key (int k, cvar_t *var)
{
	int i;

	if (m_palette_open)
	{
//		if ((k == K_ENTER) || (k == K_BACKSPACE) || (k == K_ESCAPE))
		if ((k == K_ENTER) || M_KeyMatches(k, &key_menuprev) || M_KeyMatches(k, &key_menuclose))
		{
			m_palette_open = false;
		}
		else
		{
			i = (int) var->value;
			if (i > 255)		// custom/auto color is currently selected
			{
				if (k == K_UPARROW)
				{
					i -= 16;
					goto SETCHCOLOR;
				}
				if (k == K_DOWNARROW)
				{
					i -= 256;
					goto SETCHCOLOR;
				}
				if ((var != &r_skycolor) || !m_palette_custom_color[3] ||
							((k != K_RIGHTARROW) && (k != K_LEFTARROW)))
					return true;
			}

			if (m_palette_custom_color[3])
			{
				if (((i <= 15) && (i >= 10) && (k == K_UPARROW)) ||
					((i <= 255) && (i >= 250) && (k == K_DOWNARROW)) ||
					((i == 256) && ((k == K_LEFTARROW) || (k == K_RIGHTARROW))))
				{
				// move selection to custom color
					Cvar_SetDirect (var, va("%d %d %d", m_palette_custom_color[0],
									m_palette_custom_color[1], m_palette_custom_color[2]));
					var->value = 271;
					return true;
				}
			}

			if (var == &r_skycolor)
			{
				if (((i <= 7) && (k == K_UPARROW)) ||
					((i >= 240) && (i <= 247) && (k == K_DOWNARROW)) ||
					((i == 271) && ((k == K_LEFTARROW) || (k == K_RIGHTARROW))))
				{
				// move selection to auto color
					Cvar_SetDirect (var, var->defaultvalue);
					var->value = 256;
					return true;
				}
			}

			switch (k)
			{
			case K_RIGHTARROW:
				i = (i/16)*16 + ((i%16)+1)%16;
				break;
			case K_LEFTARROW:
				i = (i/16)*16 + (i%16 == 0 ? 15 : ((i%16)-1)%16);
				break;
			case K_UPARROW:
				i = (i < 16 ? 240+i: i-16);
				break;
			case K_DOWNARROW:
				i = (i < 240 ? i+16 : i-240);
				break;
			default:
				return true;
			}

		SETCHCOLOR:
			Cvar_SetDirect (var, va("%d", i));
			var->value = i;
		}

		return true;
	}
	else
	{
		if ((k == K_ENTER) || (k == K_RIGHTARROW) || (k == K_LEFTARROW))
		{
			if (menu_current->items[menu_current->cursor].cvar == var)
			{
				m_palette_open = true;
				return true;
			}
		}
	}

	return false;
}

//=============================================================================
/* CROSSHAIR OPTIONS MENU */

void M_Menu_Crosshair_f (cmd_source_t src)
{
	static const char	*pathlist[3] = {"crosshairs/", "textures/crosshairs/", NULL};

	M_SetCurrentMenu (&menu_crosshair);

	M_InitColorCvar (&crosshaircolor);
	M_FindCvarImages (&gl_crosshairimage, pathlist, false);
}

qboolean M_Crosshair_Close (void)
{
	if (m_palette_open)
	{
		m_palette_open = false;
		return true;
	}
	//else menu_current->parentmenu->openproc ();	//M_Menu_HUD_f ();
	return false;
}

void M_Crosshair_DrawVar (cvar_t *var, int y, qboolean selected)
{
	float	val;

	if (var == &gl_crosshairimage)
	{
		M_DrawImageCvar (&gl_crosshairimage, y);
	}
	else if (var == &crosshaircolor)
	{
		M_Draw_ColorChooser (var, y);
	}
	else
	{
		val = (var->value - var->minvalue) / (var->maxvalue - var->minvalue);
		M_DrawSlider (M_VARLEFT, y, val);
	}
}

void M_Crosshair_Draw (void)
{
//	mpic_t	*p;
	const char *oldtitle;

//	p = Draw_GetCachePic ("gfx/p_option.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	if (m_palette_open)
	{
		oldtitle = menu_current->title;
		menu_current->title = "Crosshair Color";
		M_Menu_DrawTitle (menu_current);
		menu_current->title = oldtitle;

		M_Draw_CvarCaption (menu_current);
		//M_Print (80, 32, menu_current->items[menu_current->cursor].text);
		M_Draw_ColorChooser (&crosshaircolor, 0);
	}
	else
	{
		M_Menu_Draw (menu_current, 32, MAX_VIEWITEMS, M_Crosshair_DrawVar);
	}

	M_SetScale (false);		// disable any projection
	Draw_Crosshair ();
	M_SetScale (true);
}

qboolean M_Crosshair_Key (int k, qboolean down)
{
	if (M_ColorChooser_Key (k, &crosshaircolor))
		return true;
/*
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}*/

	return false;
}

//=============================================================================
/* LIGHTING OPTIONS MENU */

char *expl_colors[5] =
{
	"default",
	"red",
	"blue",
	"red/blue",
	"random"
};

char *m_glow_warning[] =
{
	"0" "* ",
	"1" "Requires \"Other glows\" to be on",
/*	"1" "Requires ",
	"0" "\"Other glows\"",
	"1" " to be ",
	"0" "on",*/
	NULL
};

void M_Menu_Lighting_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_lighting);
}

void M_Lighting_DrawVar (cvar_t *cvar, int y, qboolean selected)
{
	int		x, i;
	float	r;
	char	*str;

	if (cvar == &gl_lightmode)
	{
		i = (int)cvar->value;
		M_Print (M_VARLEFT, y, (i >= 3) ? "Overbright" : (i == 2) ? "High contrast" : (i == 1) ? "JoeQuake" : "GLQuake");
	}
	else if (cvar == &r_flatlightstyles)
	{
		M_Print (M_VARLEFT, y, (cvar->value == 2) ? "off; use peak" : (cvar->value == 1) ? "off; use avg" : "on");
	}
	else if (cvar == &r_explosionlightcolor)
	{
		if (((int)cvar->value > 4) || ((int)cvar->value < 0))
			str = expl_colors[0];
		else
			str = expl_colors[(int)cvar->value];
		M_Print (M_VARLEFT, y, str);
	}
	else if ((cvar->type == CVAR_INT) && (cvar->minvalue == 0) && (cvar->maxvalue == 1))
	{
		M_DrawCheckbox (M_VARLEFT, y, cvar->value);

		if ((cvar == &r_powerupglow) && selected && cvar->value && !gl_flashblend.value)
		{
			M_PrintWhite (M_VARLEFT+20, y, "*");

			x = 3;		// (320-DRAW_CHARWIDTH*strlen(m_glow_warning))/2
			for (i = 0; ; i++)
			{
				str = m_glow_warning[i];
				if (!str) break;

				if (*str == '0')
					M_PrintWhite (x*DRAW_CHARWIDTH, 172, str + 1);
				else
					M_Print (x*DRAW_CHARWIDTH, 172, str + 1);
				x += strlen (str)-1;
			}
		}
	}
	else if (cvar->type != CVAR_STRING)
	{
		r = (cvar->value - cvar->minvalue) / (cvar->maxvalue - cvar->minvalue);
		M_DrawSlider (M_VARLEFT, y, r);
	}
}

void M_Lighting_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/p_option.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_Lighting_DrawVar);
}

/*qboolean M_Lighting_Key (int k)
{
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}

	return false;
}
*/
//=============================================================================
/* SKY/WATER OPTIONS MENU */

void M_Menu_SkyWater_f (cmd_source_t src)
{
	static const char *pathlist[3] = {"env/", "gfx/env/", NULL};

	M_SetCurrentMenu (&menu_skywater);

	M_InitColorCvar (&r_skycolor);
	M_FindCvarImages (&r_skybox, pathlist, true);
}

qboolean M_SkyWater_Close (void)
{
	if (m_palette_open)
	{
		m_palette_open = false;
		return true;
	}
	//else menu_current->parentmenu->openproc ();	//M_Menu_Video_f ();
	return false;
}

void M_SkyWater_DrawVar (cvar_t *cvar, int y, qboolean selected)
{
	float	r;

	if (cvar == &r_skytype)
	{
		M_Print (M_VARLEFT, y, (!cvar->value ? "MHQuake" : (cvar->value == 2) ? "solid" : "classic"));
	}
	else if (cvar == &r_skycolor)
	{
		M_Draw_ColorChooser (cvar, y);
	}
	else if (cvar == &r_oldsky)
	{
		M_DrawCheckbox (M_VARLEFT, y, !cvar->value);
	}
	else if (cvar == &r_skybox)
	{
		M_DrawImageCvar (cvar, y);
	}
	else if (cvar == &gl_skyhack)
	{
		M_Print (M_VARLEFT, y, (!cvar->value ? "off" : (cvar->value > 1) ? "skybox + MH" : "skybox only"));
	}
	else if (cvar == &gl_waterfog)
	{
		M_Print (M_VARLEFT, y, (!cvar->value ? "off" : (cvar->value == 2) ? "FuhQ'ed" : "normal"));
	}
	else if ((cvar->type == CVAR_INT) && (cvar->minvalue == 0) && (cvar->maxvalue == 1))
	{
		M_DrawCheckbox (M_VARLEFT, y, cvar->value);
	}
	else if (cvar->type != CVAR_STRING)
	{
		r = (cvar->value - cvar->minvalue) / (cvar->maxvalue - cvar->minvalue);
		M_DrawSlider (M_VARLEFT, y, r);
	}
}

void M_SkyWater_Draw (void)
{
//	mpic_t	*p;
	const char *oldtitle;

//	p = Draw_GetCachePic ("gfx/p_option.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	if (m_palette_open)
	{
		oldtitle = menu_current->title;
		menu_current->title = "Solid Sky Color";
		M_Menu_DrawTitle (menu_current);
		menu_current->title = oldtitle;

		M_Draw_CvarCaption (menu_current);
		//M_Print (80, 32, menu_current->items[menu_current->cursor].text);
		M_Draw_ColorChooser (&r_skycolor, 0);
	}
	else
	{
		M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_SkyWater_DrawVar);
	}
}

qboolean M_SkyWater_Key (int k, qboolean down)
{
	if (M_ColorChooser_Key (k, &r_skycolor))
		return true;
/*
	switch (k)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (1);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		return true;
	}
*/
	return false;
}

//=============================================================================
/* PARTICLES MENU */

static const char *particle_types[3] = {"Classic", "QMB", "QMB (alt)"};

extern qboolean qmb_initialized;

#define M_PARTICLE_WARNING "* QMB Texture(s) Not Found! *"
#define M_PARTICLE_PRESET_INFO  "(affects next 12 options)"

m_preset_t m_particle_preset_vars[] =
{
	{&gl_part_explosions,  {0, 1}},
	{&gl_part_trails,      {0, 1}},
	{&gl_part_spikes,      {0, 1}},
	{&gl_part_gunshots,    {0, 1}},
	{&gl_part_blood,       {0, 1}},
	{&gl_part_telesplash,  {0, 1}},
	{&gl_part_blobs,       {0, 1}},
	{&gl_part_lavasplash,  {0, 1}},
	{&gl_part_inferno,     {0, 1}},
	{&gl_part_flames,      {0, 1}},
	{&gl_part_lightning,   {0, 1}},
	{&gl_part_spiketrails, {0, 1}}
};

m_preset_config_t m_particle_configs =
{
	&particle_preset, 3, {"Classic", "QMB", "custom"}, false,
	sizeof(m_particle_preset_vars)/sizeof(m_preset_t), m_particle_preset_vars
};

int M_GetParticlePreset (void)				// used by R_ToggleParticles_f (gl_rpart.c)
{
	return M_GetCurrentPreset (&m_particle_configs);
}

void M_SetParticlePreset (int preset)		// used by R_ToggleParticles_f (gl_rpart.c)
{
	int i;

	for (i = 0; i < m_particle_configs.num_vars; i++)
		Cvar_SetValueDirect (m_particle_configs.presets[i].cvar, m_particle_configs.presets[i].value[preset]);
}


qboolean M_OnChange_particle_preset (cvar_t *var, const char *value)
{
	return M_OnChange_preset (&m_particle_configs, value);
}

void M_Menu_Particles_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_particles);
	M_RecheckPresets (&m_particle_configs);
}

void M_Particles_DrawVar (cvar_t *cvar, int y, qboolean selected)
{
	int		val;
	const char	*str;

	val = (int) cvar->value;

	if (cvar == &particle_preset)
	{
		M_PrintWhite (M_VARLEFT, y, m_particle_configs.config_names[val]);

		if (selected)
			M_PrintWhite ((320 - strlen(M_PARTICLE_PRESET_INFO)*DRAW_CHARWIDTH) / 2, M_CVARINFO_Y, M_PARTICLE_PRESET_INFO);
	}
	else
	{
		if (cvar == &gl_part_explosions)
		{
			str = ((val < 0) || (val > 2)) ? "Invalid Value" : particle_types[val];
		}
		else if ((cvar == &gl_bounceparticles) || (cvar == &gl_clipparticles))
		{
			str = (!val ? "off" : "on");
		}
		else
			str = (!val ? particle_types[0] : particle_types[1]);

		M_Print (M_VARLEFT, y, str);
	}
}

void M_Particles_Draw (void)
{
//	mpic_t	*p;

//	p = Draw_GetCachePic ("gfx/ttl_cstm.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	if (menu_current->flags & M_VAR_CHANGED)
	{
		M_RecheckPresets (&m_particle_configs);
		menu_current->flags &= ~M_VAR_CHANGED;
	}

	M_Menu_Draw/*Small*/ (menu_current, 32, MAX_VIEWITEMS, M_Particles_DrawVar);

	if (!qmb_initialized && (particle_preset.value > 0))
		M_PrintWhite ((320-strlen(M_PARTICLE_WARNING)*DRAW_CHARWIDTH)/2, 182, M_PARTICLE_WARNING);
}

//=============================================================================
/* VIDEO MENU */

void M_Menu_VideoModes_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_videomodes);
}

void M_VideoModes_Draw (void)
{
//	mpic_t	*p;
//	char	*ptr;
//	int	lnummodes, i, k, column, row;
//	vmode_t	*pv;

//	p = Draw_GetCachePic ("gfx/vidmodes.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);
	M_PrintWhite (110, 11, menu_current->title);

/*	vid_wmodes = 0;
	lnummodes = VID_NumModes ();

	for (i = 1 ; i < lnummodes && vid_wmodes < MAX_MODEDESCS ; i++)
	{
		ptr = VID_GetModeDescription (i);
		pv = VID_GetModePtr (i);

		k = vid_wmodes;

		modedescs[k].modenum = i;
		modedescs[k].desc = ptr;
		modedescs[k].iscur = 0;

		if (i == vid_modenum)
			modedescs[k].iscur = 1;

		vid_wmodes++;
	}

	if (vid_wmodes > 0)
	{
		M_Print (2*DRAW_CHARWIDTH, 36+0*MITEMHEIGHT, "Fullscreen Modes (WIDTHxHEIGHTxBPP)");

		column = DRAW_CHARWIDTH;
		row = 36+2*MITEMHEIGHT;

		for (i=0 ; i<vid_wmodes ; i++)
		{
			if (modedescs[i].iscur)
				M_PrintWhite (column, row, modedescs[i].desc);
			else
				M_Print (column, row, modedescs[i].desc);

			column += 13*DRAW_CHARWIDTH;

			if ((i % VID_ROW_SIZE) == (VID_ROW_SIZE - 1))
			{
				column = DRAW_CHARWIDTH;
				row += MITEMHEIGHT;
			}
		}
	}

	M_Print (3*8, 36 + MODE_AREA_HEIGHT * 8 + MITEMHEIGHT*2, "Video modes must be set from the");
	M_Print (3*8, 36 + MODE_AREA_HEIGHT * 8 + MITEMHEIGHT*3, "command line with -width <width>");
	M_Print (3*8, 36 + MODE_AREA_HEIGHT * 8 + MITEMHEIGHT*4, "and -bpp <bits-per-pixel>");
	M_Print (3*8, 36 + MODE_AREA_HEIGHT * 8 + MITEMHEIGHT*6, "Select windowed mode with -window");
*/
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*1, "Video modes must be set from the");
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*2, "command line with -width <x> and");
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*3, "-bpp <bits-per-pixel>.  For non-");
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*4, "standard modes, -height <y> can");
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*5, "also be specified.");

#ifdef _WIN32
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*7, "On some machines, the default");
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*8, "refresh rate can be overridden");
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*9, "by specifying -refresh <Hz>");

	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*10,"Select windowed mode with -window");
#else
	M_Print (3*DRAW_CHARWIDTH, 32 + MITEMHEIGHT*7, "Select windowed mode with -window");
#endif
}

qboolean M_VideoModes_Close (void)
{
	S_LocalSound ("misc/menu1.wav");
	//menu_current->parentmenu->openproc ();	//M_Menu_Video_f ();
	return false;
}

//=============================================================================
/* COMMON STUFF FOR MAPS AND DEMOS MENUS */

// NOTE: 320x200 res can only handle no more than 17 lines +2 for file
// searching. In GL I use 1 more line, though 320x200 is also available
// under GL too, but I force _nobody_ using that, but 320x240 instead!

#define	MAXLINES	18

char	m_prevmap[MAX_QPATH] = "";
char	searchfile[MAX_FILELENGTH] = "";

static	int	globctr = 0;
static qboolean	searchbox = false;

void M_SetFileCursor (char *prevfile)
{
	int	i;

	menu_current->first_item = menu_current->cursor = 0;

	// TODO: position demo cursor
	if (prevfile && *prevfile)
	{
		for (i=0 ; i<num_files ; i++)
		{
			if (!strcmp(filelist[i].name, prevfile))
			{
				menu_current->cursor = i;
				if (menu_current->cursor >= MAXLINES)
				{
					menu_current->first_item = menu_current->cursor - (MAXLINES-1);
					//menu_current->cursor = MAXLINES-1;
				}
				*prevfile = 0;
				break;
			}
		}
	}
}
/*
static char *toYellow (char *s)
{
	static	char	buf[20];

	Q_strcpy (buf, s, sizeof(buf));
	for (s = buf ; *s ; s++)
		if (*s >= '0' && *s <= '9')
			*s = *s - '0' + 18;

	return buf;
}
*/
void M_Files_Draw (const char *title)
{
	int				i, y;
	file_entry_t	*d;
	char			str[29];

	M_Print (140, 8, title);
	M_DrawBar (8, 24, 30);
	M_DrawBar (256, 24, 7);

	d = filelist + menu_current->first_item;
	for (i = 0, y = 32 ; i < num_files - menu_current->first_item && i < MAXLINES ; i++, y += 8, d++)
	{
		Q_strcpy (str, d->name, sizeof(str));
		if (d->type)
			M_PrintWhite (24, y, str);
		else
			M_Print (24, y, str);

		if (d->type == CMD_FTYPE_DIR)
			M_PrintWhite (256, y, "folder");
		else if (d->type == CMD_FTYPE_CDUP)
			M_PrintWhite (256, y, "  up  ");
		else if (d->type == CMD_FTYPE_FILE)
		{
			//M_PrintWhite (256, y, toYellow(va("%5ik", d->size >> 10)));
				// use M_PrintWhite here Draw_Alt_String adds 128 to chars,
				// and beta Quake conchars is missing those numbers
			M_Print (256, y, va("%5i", d->size >> 10));
			M_PrintWhite (256+5*DRAW_CHARWIDTH, y, "k");
		}
	}

	if (menu_current->num_items)
		M_DrawMenuCursor (8, 32 + (menu_current->cursor - menu_current->first_item)*8);

	if (searchbox)
	{
		M_PrintWhite (24, 48 + 8*MAXLINES, "search: ");
		M_DrawTextBox (80, 40 + 8*MAXLINES, 16*DRAW_CHARWIDTH, 1*8);
		M_PrintWhite (88, 48 + 8*MAXLINES, searchfile);

		M_DrawCharacter (88 + DRAW_CHARWIDTH*strlen(searchfile), 48 + 8*MAXLINES, ((int)(realtime*4)&1) ? 11+84 : 10);
	}
}

static void KillSearchBox (void)
{
	searchbox = false;
	memset (searchfile, 0, sizeof(searchfile));
	globctr = 0;
}

qboolean M_Files_Key (int key)
{
	int		i;
	qboolean	worx;

//	if (M_Menu_NavKey(menu_current, key, MAXLINES))
//		return true;

//	if (!searchbox && (M_KeyMatches(key, &key_menuclose) || (M_KeyMatches(key, &key_menuprev)))
//		return false;

	switch (key)
	{
	case K_BACKSPACE:
		if (!searchbox)
			return false;
		if (strcmp(searchfile, ""))
			searchfile[--globctr] = 0;
		break;

	case K_ESCAPE:
		if (!searchbox)
			return false;
		KillSearchBox ();
		break;

	default:
		if (key < 32 || key > 127)
			return false;
		searchbox = true;
		searchfile[globctr++] = key;
		worx = false;
		for (i=0 ; i<num_files ; i++)
		{
			if (COM_FilenamesEqualn(filelist[i].name, searchfile, strlen(searchfile)))
			{
				worx = true;
				S_LocalSound (gMenuSounds[0]);
				menu_current->first_item = i - MAXLINES/2 - 1;
				if (menu_current->first_item < 0)
				{
					menu_current->first_item = 0;
					//menu_current->cursor = i;
				}
				else if (menu_current->first_item > (menu_current->num_items - MAXLINES))
				{
					menu_current->first_item = menu_current->num_items - MAXLINES;
					//menu_current->cursor = MAXLINES - (menu_current->num_items - i);
				}
				//else
				//	menu_current->cursor = 10;
				menu_current->cursor = i;
				break;
			}
		}
		if (!worx)
			searchfile[--globctr] = 0;
		break;
	}

	return true;
}

//=============================================================================
/* MAPS MENU */

/*
void PrintSortedMaps (void)
{
	searchpath_t	*search;

	Cmd_ClearFilelist ();
	pak_files = 0;

	for (search = com_searchpaths ; search ; search = search->next)
	{
		if (!search->pack)
		{
			RDFlags |= (RD_STRIPEXT | RD_NOERASE);
			ReadDir (va("%s/maps", search->filename), "*.bsp");
		}
	}
	FindFilesInPak ("maps/""*.bsp");

	PrintSortedFiles ();
}
*/

// callback for COM_FindAllFiles (in M_Menu_Maps_f)
int M_AddMapToList (com_fileinfo_t *fileinfo, int count, unsigned int param)
{
	if (COM_IsRealBSP(fileinfo->name))
	{
		fileinfo->name[ strlen(fileinfo->name) - 4 ] = 0;		// remove .bsp suffix
		if (!Cmd_CheckEntryName(fileinfo->name))
			Cmd_AddFilelistEntry (fileinfo->name, CMD_FTYPE_FILE, fileinfo->filelen, 0);
	}

	return 0;		// continue searching
}

void M_Menu_Maps_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_maps);

	Cmd_ClearFilelist ();
	COM_FindAllFiles (NULL, "maps/*.bsp", 0, M_AddMapToList, 0);
	menu_maps.num_items = num_files;

	M_SetFileCursor (m_prevmap);
}

void M_Maps_Draw (void)
{
	M_Files_Draw (menu_current->title);
}

qboolean M_Maps_Close (void)
{
/*	if (searchbox)
	{
		KillSearchBox ();
		return true;
	}
	else
*/	{
		if (menu_current->num_items)
			Q_strcpy (m_prevmap, filelist[menu_current->cursor].name, sizeof(m_prevmap));
		//menu_current->parentmenu->openproc ();	//M_Menu_SinglePlayer_f ();
		return false;
	}
}

qboolean M_Maps_Key (int k, qboolean down)
{
	char *name;

	if (k == K_ENTER)
	{
		if (menu_current->num_items /*&& filelist[menu_current->cursor].type != 3*/)
		{
			name = filelist[menu_current->cursor].name;
			Cbuf_AddText (va("map %s\n", name), SRC_COMMAND);
			Q_strcpy (m_prevmap, name, sizeof(m_prevmap));
			key_dest = key_game;
//			menu_current = NULL;

			if (searchbox)
				KillSearchBox ();
			return true;
		}
	}

	return M_Files_Key (k);
}

//=============================================================================
/* DEMOS MENU */

// Nehahra's Demos Menu

#define	MAXNEHLINES	20

void M_Menu_NehDemos_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_nehdemos);
}

void M_NehDemos_Draw (void)
{
	int		i, y;

	M_Print (140, 8, menu_current->title);
	M_Print (8, 24, "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f");

	for (i = 0, y = 32 ; i < menu_current->num_items - menu_current->first_item && i < MAXNEHLINES ; i++, y += 8)
		M_Print (24, y, neh_demos[menu_current->first_item + i].description);

// line cursor
	M_DrawCharacter (8, 32 + menu_current->cursor*8, 12+((int)(realtime*4)&1));
}

qboolean M_NehDemos_Key (int k, qboolean down)
{
	if (M_Menu_NavKey(menu_current, k, MAXNEHLINES))
		return true;

	if (k == K_ENTER)
	{
		S_LocalSound (gMenuSounds[1]);
		SCR_BeginLoadingPlaque (NULL);
		Cbuf_AddText (va("playdemo %s\n", neh_demos[menu_current->cursor].name), SRC_COMMAND);
		menu_current = NULL;
		key_dest = key_game;
 	}

	return true;
}

// JoeQuake's Demos Menu
//  JDH: added option to view all demos in Quake path

const char *M_DEMOVIEW_TEXT[2] =
{
	"<tab> = switch to directory browser",
	"<tab> = switch to combined path view"
};

int		m_demoview_mode = 0;
char	m_demodir[MAX_QPATH] = "";
char	m_prevdemo[MAX_QPATH] = "";


// callback for COM_FindAllFiles (in M_Menu_Demos_f)
int M_AddDemoToList (com_fileinfo_t *fileinfo, int count, unsigned int param)
{
	if (!Cmd_CheckEntryName(fileinfo->name))
		Cmd_AddFilelistEntry (fileinfo->name, (fileinfo->isdir ? CMD_FTYPE_DIR : CMD_FTYPE_FILE), fileinfo->filelen, 0);

	return 0;		// continue searching
}


void M_BuildDemoList (void)
{
	char	filepath[MAX_OSPATH];
	int		flags;

	Cmd_ClearFilelist ();
	if (m_demoview_mode == 1)
	{
		if (!m_demodir[0])
		{
			Q_snprintfz (filepath, sizeof(filepath), "%s/*", com_basedir);
		}
		else
		{
			Q_snprintfz (filepath, sizeof(filepath), "%s/%s*", com_basedir, m_demodir);
			COM_FindDirFiles (filepath, NULL, FILE_ANY_DEMO, M_AddDemoToList, 0);

			Cmd_AddFilelistEntry ("..", CMD_FTYPE_CDUP, 0, 0);
		}

		flags = FILE_DIRS_ONLY;
		COM_FindDirFiles (filepath, NULL, flags, M_AddDemoToList, 0);
	}
	else
	{
		COM_FindAllFiles (NULL, "*", FILE_ANY_DEMO, M_AddDemoToList, 0);
	}


	menu_current->num_items = num_files;
	M_SetFileCursor (m_prevdemo);
}

void M_Menu_Demos_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_demos);

	M_BuildDemoList ();
}

void M_Demos_Draw (void)
{
	const char *str = M_DEMOVIEW_TEXT[m_demoview_mode];

	if (m_demoview_mode == 1)
		M_Print (16, 16, m_demodir);

	M_Files_Draw (menu_current->title);
	M_PrintWhite ((320 - strlen(str)*DRAW_CHARWIDTH) / 2, 214, str);
}

qboolean M_Demos_Close (void)
{
/*	if (searchbox)
	{
		KillSearchBox ();
		return true;
	}
	else
*/	{
		if (menu_current->num_items)
			Q_strcpy (m_prevdemo, filelist[menu_current->cursor].name, sizeof(m_prevdemo));
		//menu_current->parentmenu->openproc ();	//M_Menu_SinglePlayer_f ();
		return false;
	}
}

qboolean M_Demos_Key (int k, qboolean down)
{
	file_entry_t	*entry;
	char			*p;

	switch (k)
	{
	case K_ENTER:
		if (!menu_current->num_items)
			return true;

		entry = &filelist[menu_current->cursor];
		/*if (entry->type == 3)
			return;*/

		if (entry->type == CMD_FTYPE_FILE)
		{
			if (m_demoview_mode == 1)
				Cbuf_AddText (va("playdemo \"..%s/%s\"\n", m_demodir, entry->name), SRC_COMMAND);
			else
				Cbuf_AddText (va("playdemo \"%s\"\n", entry->name), SRC_COMMAND);
			Q_strcpy (m_prevdemo, entry->name, sizeof(m_prevdemo));
			key_dest = key_game;
//			menu_current = NULL;
		}
		else
		{
			if (entry->type == CMD_FTYPE_CDUP)			// parent directory
			{
				if ((p = strrchr(m_demodir, '/')))
				{
					Q_strcpy (m_prevdemo, p + 1, sizeof(m_prevdemo));
					*p = 0;
				}
			}
			else
			{
				//strncat (m_demodir, va("/%s", entry->name), sizeof(m_demodir)-1);
				int len = strlen (m_demodir);
				Q_strcpy (m_demodir + len, va("/%s", entry->name), sizeof(m_demodir)-len);
			}
			M_BuildDemoList ();
		}

		if (searchbox)
			KillSearchBox ();
		return true;

	case K_TAB:
		if (menu_current->num_items)
			Q_strcpy (m_prevdemo, filelist[menu_current->cursor].name, sizeof(m_prevdemo));
		if (searchbox)
			KillSearchBox ();
		m_demoview_mode = !m_demoview_mode;
		M_BuildDemoList ();
		return true;

	case K_BACKSPACE:
		if (!searchbox && (m_demoview_mode == 1) && m_demodir[0])
		{
			if ((p = strrchr(m_demodir, '/')))
			{
				Q_strcpy (m_prevdemo, p + 1, sizeof(m_prevdemo));
				*p = 0;
			}

			M_BuildDemoList ();
			return true;
		}
	}

	return M_Files_Key (k);
}

//=============================================================================
/* QUIT MENU */

menu_t *m_wascurrent;		// so we don't lose our place in the "real" menus
keydest_t m_was_keydest;

void M_Menu_Quit_f (cmd_source_t src)
{
	if (menu_current != &menu_quit)
	{
		menu_quit.parentmenu = ((key_dest == key_menu) ? menu_current : NULL);
		m_wascurrent = menu_current;
		m_was_keydest = key_dest;
	}

	M_SetCurrentMenu (&menu_quit);
}

qboolean M_Quit_Close (void)
{
	if (menu_current->parentmenu)
	{
		menu_current = menu_current->parentmenu;
		m_entersound = true;
	}
	else
	{
		key_dest = m_was_keydest;//key_game;
		menu_current = m_wascurrent;
	}
	return true;
}

qboolean M_Quit_Key (int key, qboolean down)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		M_Quit_Close ();
		break;

	case K_ENTER:
	case 'Y':
	case 'y':
//		key_dest = key_console;
		key_dest = m_was_keydest;
		Host_Quit ();
		break;
	}

	return true;
}

void M_Quit_Draw (void)
{
	static const char *quitmsg[] =
	{
		"0reQuiem " REQUIEM_VERSION,
		"1",
		"1Programming by",
		"0jdhack@hotmail.com",
		"1",
		"1Based on JoeQuake by",
		"0Jozsef Szalontai",
		"0",
	//	"0",
		"2Updated particles by0 Entar",
		"0",
		"1NOTICE: The copyright and trademark",
		"1 notices appearing  in your copy of",
		"1Quake(r) are not modified by the use",
		"1of reQuiem and remain in full force.",
		"0Quake(tm) is a trademark of",
		"0Id Software, Inc.",
		"0",
		"0",
		"0Press 'Y' to quit.",
		NULL
	};
	const char	**p, *c;
	int		x, y, mask;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		mask = 256;
	else
#endif
	mask = 128;

	M_DrawTextBox (0, 4, 38*DRAW_CHARWIDTH, 22*DRAW_CHARHEIGHT);
	y = 16;
	for (p = quitmsg ; *p ; p++, y += DRAW_CHARHEIGHT)
	{
		c = *p;

		x = 16 + (36 - (strlen(c + 1))) * 4;
		if (*c == '2')
		{
			c++;
			while (*c != '0' && *c != '1')
			{
				M_DrawCharacter (x, y, *c++ | mask);
				x += DRAW_CHARWIDTH;
			}
		}

		if (*c == '0')
			M_PrintWhite (x, y, c + 1);
		else
			M_Print (x, y, c + 1);
	}
}

qboolean M_Quit_OverConsole (void)
{
	return ((key_dest == key_menu) && (menu_current == &menu_quit) && (m_was_keydest == key_console));
}

//=============================================================================
/* LAN CONFIG MENU */

int	lanConfig_cursor_table [] = {72, 92, 124};

int 	lanConfig_port;
char	lanConfig_portname[12];
char	lanConfig_joinname[22];

void M_Menu_LanConfig_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_lanconfig);

	menu_lanconfig.num_items = (JoiningGame ? 3 : 2);
	if (menu_lanconfig.cursor == -1)
	{
		if (JoiningGame && TCPIPConfig)
			menu_lanconfig.cursor = 2;
		else
			menu_lanconfig.cursor = 1;
	}
	if (StartingGame && menu_lanconfig.cursor == 2)
		menu_lanconfig.cursor = 1;
	lanConfig_port = DEFAULTnet_hostport;
	sprintf (lanConfig_portname, "%u", lanConfig_port);

	net_return_onerror = false;
	net_return_reason[0] = 0;
}

void M_LanConfig_Draw (void)
{
//	mpic_t	*p;
	int	basex;
	const char	*startJoin, *protocol;

//	p = Draw_GetCachePic ("gfx/p_multi.lmp", true);
//	basex = (320-p->width)/2;
	basex = 76;
//	M_DrawPic (basex, 4, p);

	M_Menu_DrawTitle (menu_current);

	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (IPXConfig)
		protocol = "IPX";
	else
		protocol = "TCP/IP";
	M_Print (basex, 32, va ("%s - %s", startJoin, protocol));
	basex += DRAW_CHARWIDTH;

	M_Print (basex, 52, "Address:");
	if (IPXConfig)
		M_Print (basex+9*DRAW_CHARWIDTH, 52, my_ipx_address);
	else
		M_Print (basex+9*DRAW_CHARWIDTH, 52, my_tcpip_address);

	M_Print (basex, lanConfig_cursor_table[0], "Port");
	M_DrawTextBox (basex+8*DRAW_CHARWIDTH, lanConfig_cursor_table[0]-DRAW_CHARHEIGHT, 6*DRAW_CHARWIDTH, 1*DRAW_CHARHEIGHT);
	M_Print (basex+9*DRAW_CHARWIDTH, lanConfig_cursor_table[0], lanConfig_portname);

	if (JoiningGame)
	{
		M_Print (basex, lanConfig_cursor_table[1], "Search for local games...");
		M_Print (basex, 108, "Join game at:");
		M_DrawTextBox (basex+DRAW_CHARWIDTH, lanConfig_cursor_table[2]-DRAW_CHARHEIGHT, 22*DRAW_CHARWIDTH, 1*DRAW_CHARHEIGHT);
		M_Print (basex+2*DRAW_CHARWIDTH, lanConfig_cursor_table[2], lanConfig_joinname);
	}
	else
	{
		M_DrawTextBox (basex, lanConfig_cursor_table[1]-DRAW_CHARHEIGHT, 2*DRAW_CHARWIDTH, 1*DRAW_CHARHEIGHT);
		M_Print (basex+DRAW_CHARWIDTH, lanConfig_cursor_table[1], "OK");
	}

	M_DrawMenuCursor (basex-2*DRAW_CHARWIDTH, lanConfig_cursor_table [menu_current->cursor]);

	if (menu_current->cursor == 0)
		M_DrawCharacter (basex+9*DRAW_CHARWIDTH + DRAW_CHARWIDTH*strlen(lanConfig_portname), lanConfig_cursor_table [0], 10+((int)(realtime*4)&1));

	if (menu_current->cursor == 2)
		M_DrawCharacter (basex+2*DRAW_CHARWIDTH + DRAW_CHARWIDTH*strlen(lanConfig_joinname), lanConfig_cursor_table [2], 10+((int)(realtime*4)&1));

	if (*net_return_reason)
		M_PrintWhite (basex, 148, net_return_reason);
}

qboolean M_LanConfig_Key (int key, qboolean down)
{
	int		l;

//	if (M_Menu_NavKey(menu_current, key, M_MAX_ITEMS))
//		goto LAN_END;

	switch (key)
	{
	case K_ENTER:
		if (menu_current->cursor == 0)
		{
			menu_current->cursor++;
			return true;
		}

		m_entersound = true;
		M_ConfigureNetSubsystem ();

		if (menu_current->cursor == 1)
		{
			if (StartingGame)
			{
				M_Menu_GameOptions_f (SRC_COMMAND);
				break;
			}
			M_Menu_Search_f (SRC_COMMAND);
			break;
		}

		if (menu_current->cursor == 2)
		{
			m_return_state = menu_current;
			net_return_onerror = true;
			key_dest = key_game;
			menu_current = NULL;
			Cbuf_AddText (va ("connect \"%s\"\n", lanConfig_joinname), SRC_COMMAND);
			break;
		}

		break;

	case K_BACKSPACE:
		if (menu_current->cursor == 0)
		{
			if (strlen(lanConfig_portname))
				lanConfig_portname[strlen(lanConfig_portname)-1] = 0;
		}
		else if (menu_current->cursor == 2)
		{
			if (strlen(lanConfig_joinname))
				lanConfig_joinname[strlen(lanConfig_joinname)-1] = 0;
		}
		else return false;
		break;

	default:
		if (key < 32 || key > 127)
			return false;

		if (menu_current->cursor == 2)
		{
			l = strlen(lanConfig_joinname);
			if (l < 21)
			{
				lanConfig_joinname[l+1] = 0;
				lanConfig_joinname[l] = key;
			}
		}

		if (key < '0' || key > '9')
			break;
		if (menu_current->cursor == 0)
		{
			l = strlen(lanConfig_portname);
			if (l < 5)
			{
				lanConfig_portname[l+1] = 0;
				lanConfig_portname[l] = key;
			}
		}
	}

//LAN_END:
	if (StartingGame && menu_current->cursor == 2)
		menu_current->cursor = (key == K_UPARROW) ? 1 : 0;

	l = Q_atoi(lanConfig_portname);
	if (l <= 65535)
		lanConfig_port = l;
	sprintf (lanConfig_portname, "%u", lanConfig_port);
	return true;
}

//=============================================================================
/* GAME OPTIONS MENU */

int	startepisode;
int	startlevel;
int	maxplayers;
qboolean m_serverInfoMessage = false;
double	m_serverInfoMessageTime;

void M_Menu_GameOptions_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_gameoptions);

	if (maxplayers == 0)
		maxplayers = 8;		// JT021105 - changed svs.maxclients to 16
	if (maxplayers < 2)
		maxplayers = svs.maxclientslimit;
}

//int	gameoptions_cursor_table[] = {40, 0, 56, 64, 72, 80, 88, 96, 0, 112, 120};

void M_GameOptions_Draw (void)
{
//	mpic_t		*p;
	int			x;
	char		*msg;
	episode_t	*eplist;
	level_t		*levlist;

//	p = Draw_GetCachePic ("gfx/p_multi.lmp", true);
//	M_DrawPic ((320-p->width)/2, 4, p);

	M_Menu_DrawTitle (menu_current);

	for (x = 0; x < menu_current->num_items; x++)
		M_DrawItem (menu_current, &menu_current->items[x], 40 + x*MITEMHEIGHT, false, NULL);

	M_DrawTextBox (152, 32, 10*DRAW_CHARWIDTH, 1*DRAW_CHARHEIGHT);
	M_Print (160, 40, "begin game");

	M_Print (160, 60, va("%i", maxplayers));

	M_Print (160, 70, coop.value ? "Cooperative" : "Deathmatch");

	if (rogue)
	{
		switch ((int)teamplay.value)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			case 3: msg = "Tag"; break;
			case 4: msg = "Capture the Flag"; break;
			case 5: msg = "One Flag CTF"; break;
			case 6: msg = "Three Team CTF"; break;
			default: msg = "Off"; break;
		}
	}
	else
	{
		switch ((int)teamplay.value)
		{
			case 1: msg = "No Friendly Fire"; break;
			case 2: msg = "Friendly Fire"; break;
			default: msg = "Off"; break;
		}
	}
	M_Print (160, 80, msg);

	msg = ((skill.value == 0) ? "Easy" :
	       (skill.value == 1) ? "Normal" :
	       (skill.value == 2) ? "Hard" : "Nightmare");
	M_Print (160, 90, va("%s difficulty", msg));

	msg = ((fraglimit.value == 0) ? "none" : va("%i frags", (int)fraglimit.value));
	M_Print (160, 100, msg);

	msg = ((timelimit.value == 0) ? "none" : va("%i minutes", (int)timelimit.value));
	M_Print (160, 110, msg);


//MED 01/06/97 added hipnotic episodes
//PGM 01/07/97 added rogue episodes
	eplist = (hipnotic ? hipnoticepisodes : rogue ? rogueepisodes : episodes);
	M_Print (160, 130, eplist[startepisode].description);

	if (hipnotic)
	{
		eplist = hipnoticepisodes;
		levlist = hipnoticlevels;
	}
	else if (rogue)
	{
		eplist = rogueepisodes;
		levlist = roguelevels;
	}
	else
	{
		eplist = episodes;
		levlist = levels;
	}

	M_Print (160, 140, levlist[eplist[startepisode].firstLevel + startlevel].description);
	M_Print (160, 150, levlist[eplist[startepisode].firstLevel + startlevel].name);

// line cursor
//	M_DrawMenuCursor (144, gameoptions_cursor_table[menu_current->cursor]);
	M_DrawMenuCursor (160-2*DRAW_CHARWIDTH, 40 + menu_current->cursor*MITEMHEIGHT);

	if (m_serverInfoMessage)
	{
		if ((realtime - m_serverInfoMessageTime) < 5.0)
		{
			x = (320-27*DRAW_CHARWIDTH)/2;
			M_DrawTextBox (x, 160, 25*DRAW_CHARWIDTH, 4*DRAW_CHARHEIGHT);
			x += DRAW_CHARWIDTH;
			M_Print (x, 160+1*DRAW_CHARHEIGHT, "   More than 8 players   ");
			M_Print (x, 160+2*DRAW_CHARHEIGHT, "  requires using command ");
			M_Print (x, 160+3*DRAW_CHARHEIGHT, " line parameters; please ");
			M_Print (x, 160+4*DRAW_CHARHEIGHT, "    see techinfo.txt.    ");
		}
		else
		{
			m_serverInfoMessage = false;
		}
	}
}

void M_NetStart_Change (int dir)
{
	int	count;

	switch (menu_current->cursor)
	{
	case 2:
		maxplayers += dir;
		if (maxplayers > svs.maxclientslimit)
		{
			maxplayers = svs.maxclientslimit;
			m_serverInfoMessage = true;
			m_serverInfoMessageTime = realtime;
		}
		if (maxplayers < 2)
			maxplayers = 2;
		break;

	case 3:
		Cvar_SetValueDirect (&coop, coop.value ? 0 : 1);
		break;

	case 4:
		if (rogue)
			count = 6;
		else
			count = 2;

		Cvar_SetValueDirect (&teamplay, teamplay.value + dir);
		if (teamplay.value > count)
			Cvar_SetDirect (&teamplay, "0");
		else if (teamplay.value < 0)
			Cvar_SetValueDirect (&teamplay, count);
		break;

	case 5:
		Cvar_SetValueDirect (&skill, skill.value + dir);
		if (skill.value > 3)
			Cvar_SetDirect (&skill, "0");
		if (skill.value < 0)
			Cvar_SetDirect (&skill, "3");
		break;

	case 6:
		Cvar_SetValueDirect (&fraglimit, fraglimit.value + dir*10);
		if (fraglimit.value > 100)
			Cvar_SetDirect (&fraglimit, "0");
		if (fraglimit.value < 0)
			Cvar_SetDirect (&fraglimit, "100");
		break;

	case 7:
		Cvar_SetValueDirect (&timelimit, timelimit.value + dir*5);
		if (timelimit.value > 60)
			Cvar_SetDirect (&timelimit, "0");
		if (timelimit.value < 0)
			Cvar_SetDirect (&timelimit, "60");
		break;

	case 9:
		startepisode += dir;
	//MED 01/06/97 added hipnotic count
		if (hipnotic)
			count = 6;
	//PGM 01/07/97 added rogue count
	//PGM 03/02/97 added 1 for dmatch episode
		else if (rogue)
			count = 4;
		else if (registered.value)
			count = 7;
		else
			count = 2;

		if (startepisode < 0)
			startepisode = count - 1;

		if (startepisode >= count)
			startepisode = 0;

		startlevel = 0;
		break;

	case 10:
		startlevel += dir;
	//MED 01/06/97 added hipnotic episodes
		if (hipnotic)
			count = hipnoticepisodes[startepisode].levels;
	//PGM added rogue episodes
		else if (rogue)
			count = rogueepisodes[startepisode].levels;
		else
			count = episodes[startepisode].levels;

		if (startlevel < 0)
			startlevel = count - 1;

		if (startlevel >= count)
			startlevel = 0;
		break;
	}
}

void M_StartMPGame (cmd_source_t src)
{
	const char *name;

	if (sv.active)
		Cbuf_AddText ("disconnect\n", src);
	Cbuf_AddText ("listen 0\n", src);	// so host_netport will be re-examined
	Cbuf_AddText (va("maxplayers %u\n", maxplayers), src);
	SCR_BeginLoadingPlaque (NULL);

	if (hipnotic)
		name = hipnoticlevels[hipnoticepisodes[startepisode].firstLevel + startlevel].name;
	else if (rogue)
		name = roguelevels[rogueepisodes[startepisode].firstLevel + startlevel].name;
	else
		name = levels[episodes[startepisode].firstLevel + startlevel].name;

	Cbuf_AddText (va("map %s\n", name), src);
}

qboolean M_GameOptions_Key (int key, qboolean down)
{
	if (menu_current->cursor == 0)
		return false;

	switch (key)
	{
	case K_LEFTARROW:
		S_LocalSound (gMenuSounds[2]);
		M_NetStart_Change (-1);
		return true;

	case K_RIGHTARROW:
		S_LocalSound (gMenuSounds[2]);
		M_NetStart_Change (1);
		return true;

/*	case K_ENTER:
		S_LocalSound (gMenuSounds[1]);
		M_NetStart_Change (1);
		return true;*/
	}

	return false;
}

//=============================================================================
/* SEARCH MENU */

qboolean	searchComplete = false;
double		searchCompleteTime;

void M_Menu_Search_f (cmd_source_t src)
{
	key_dest = key_menu;
	menu_current = &menu_search;
	m_entersound = false;
	slistSilent = true;
	slistLocal = false;
	searchComplete = false;
	NET_Slist_f (src);
}

void M_Search_Draw (void)
{
	mpic_t	*p;
	int	x;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		M_DrawTitle_H2 (menu_current->lmp);
	else
#endif
	{
		p = Draw_GetCachePic ("gfx/p_multi.lmp", false);
		M_DrawPic ((320-p->width)/2, 4, p);
	}

	x = (320 - (12*DRAW_CHARWIDTH))/2 + 4;
	M_DrawTextBox (x-DRAW_CHARWIDTH, 32, 12*DRAW_CHARWIDTH, 1*DRAW_CHARHEIGHT);
	M_Print (x, 40, "Searching...");

	if (slistInProgress)
	{
		NET_Poll ();
		return;
	}

	if (!searchComplete)
	{
		searchComplete = true;
		searchCompleteTime = realtime;
	}

	if (hostCacheCount)
	{
		M_Menu_ServerList_f (SRC_COMMAND);
		return;
	}

	M_PrintWhite ((320 - (22*DRAW_CHARWIDTH))/2, 64, "No Quake servers found");
	if ((realtime - searchCompleteTime) < 3.0)
		return;

	M_Menu_LanConfig_f (SRC_COMMAND);
}

qboolean M_Search_Key (int key, qboolean down)
{
	return true;
}

//=============================================================================
/* SLIST MENU */

qboolean slist_sorted;

void M_Menu_ServerList_f (cmd_source_t src)
{
	M_SetCurrentMenu (&menu_serverlist);

	menu_serverlist.cursor = 0;
	net_return_onerror = false;
	net_return_reason[0] = 0;
	slist_sorted = false;
}

void M_ServerList_Draw (void)
{
	int		n;
	char	string [64];
	mpic_t	*p;

	if (!slist_sorted)
	{
		if (hostCacheCount > 1)
		{
			int		i, j;
			hostcache_t	temp;

			for (i=0 ; i<hostCacheCount ; i++)
				for (j=i+1 ; j<hostCacheCount ; j++)
					if (strcmp(hostcache[j].name, hostcache[i].name) < 0)
					{
						memcpy (&temp, &hostcache[j], sizeof(hostcache_t));
						memcpy (&hostcache[j], &hostcache[i], sizeof(hostcache_t));
						memcpy (&hostcache[i], &temp, sizeof(hostcache_t));
					}
		}
		slist_sorted = true;
	}

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		M_DrawTitle_H2 (menu_current->lmp);
	else
#endif
	{
		p = Draw_GetCachePic ("gfx/p_multi.lmp", false);
		M_DrawPic ((320-p->width)/2, 4, p);
	}

	for (n=0 ; n<hostCacheCount ; n++)
	{
		if (hostcache[n].maxusers)
			Q_snprintfz (string, sizeof(string), "%-15.15s %-15.15s %2u/%2u\n", hostcache[n].name, hostcache[n].map, hostcache[n].users, hostcache[n].maxusers);
		else
			Q_snprintfz (string, sizeof(string), "%-15.15s %-15.15s\n", hostcache[n].name, hostcache[n].map);
		M_Print (16, 32 + 8*n, string);
	}
	M_DrawMenuCursor (0, 32 + menu_current->cursor*8);

	if (*net_return_reason)
		M_PrintWhite (16, 148, net_return_reason);
}

qboolean M_ServerList_Key (int k, qboolean down)
{
	menu_current->num_items = hostCacheCount;

	switch (k)
	{
	case K_SPACE:
		M_Menu_Search_f (SRC_COMMAND);
		return true;

	case K_ENTER:
		S_LocalSound (gMenuSounds[1]);
		m_return_state = menu_current;
		net_return_onerror = true;
		slist_sorted = false;
		key_dest = key_game;
		Cbuf_AddText (va ("connect \"%s\"\n", hostcache[menu_current->cursor].cname), SRC_COMMAND);
		menu_current = NULL;
		return true;
	}

	return false;
}

//=============================================================================
/* HELP MENU */	// joe: I decided to left it in, coz svc_sellscreen use it

void M_Menu_Help_f (cmd_source_t src)
{
	if (menu_current == &menu_help)
	{
		if (key_dest == key_menu)
		{
			M_Help_Close ();
			return;
		}
	}
	else
	{
		menu_help.parentmenu = ((key_dest == key_menu) ? menu_current : NULL);
		m_wascurrent = menu_current;
	}

	M_SetCurrentMenu (&menu_help);
	menu_current->cursor = 0;
}

qboolean M_Help_Close (void)
{
	if (menu_current->parentmenu && menu_current->parentmenu->openproc)
	{
		menu_current->parentmenu->openproc (SRC_COMMAND);
	}
	else
	{
		key_dest = key_game;
		menu_current = m_wascurrent;
	}
	return true;
}

void M_Help_Draw (void)
{
	char *lmpfile;
	mpic_t *pic;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		lmpfile = va("gfx/menu/help%02i.lmp", menu_current->cursor+1);
	else
#endif
	lmpfile = va("gfx/help%i.lmp", menu_current->cursor);

	pic = Draw_GetCachePic (lmpfile, false);
	if (!pic)
		pic = Draw_GetCachePic ("gfx/help.lmp", false);		// beta Quake

	M_DrawPic (0, 0, pic);
}

qboolean M_Help_Key (int key, qboolean down)
{
	/*if (M_KeyMatches(key, &key_menuclose) || M_KeyMatches(key, &key_menuprev))
	{
		if (menu_current->parentmenu && menu_current->parentmenu->openproc)
		{
			menu_current->parentmenu->openproc (SRC_COMMAND);
		}
		else
		{
			key_dest = key_game;
			menu_current = m_wascurrent;
		}
		return true;
	}*/

	switch (key)
	{
	case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (++menu_current->cursor >= menu_current->num_items)
			menu_current->cursor = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--menu_current->cursor < 0)
			menu_current->cursor = menu_current->num_items-1;
		break;
	}

	return M_IsReservedKey (key);		// don't want default key handler called
}

//=============================================================================
/* Menu Subsystem */

void M_OnChange_gl_smoothfont (float newval)
{
	GL_Bind (m_scroll_up.texnum);
	GL_SetFontFilter (newval);

	GL_Bind (m_scroll_dn.texnum);
	GL_SetFontFilter (newval);
}

void M_LoadCharTexture (byte *data, const char *name, mpic_t *out)
{
	GL_LoadPicTexture (name, out, data, TEX_ALPHA, 1);
	GL_SetFontFilter (gl_smoothfont.value);
}

void M_Init (void)
{
	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f, 0);

	Cmd_AddCommand ("menu_main", M_Menu_Main_f, 0);
	Cmd_AddCommand ("menu_singleplayer", M_Menu_SinglePlayer_f, 0);
	Cmd_AddCommand ("menu_load", M_Menu_Load_f, 0);
	Cmd_AddCommand ("menu_save", M_Menu_Save_f, 0);
	Cmd_AddCommand ("menu_multiplayer", M_Menu_MultiPlayer_f, 0);
	Cmd_AddCommand ("menu_setup", M_Menu_Setup_f, 0);
	Cmd_AddCommand ("menu_options", M_Menu_Options_f, 0);
	Cmd_AddCommand ("menu_keys", M_Menu_Keys_f, 0);
//	Cmd_AddCommand ("menu_videomodes", M_Menu_VideoModes_f, 0);
//	Cmd_AddCommand ("menu_videooptions", M_Menu_VideoOptions_f, 0);
//	Cmd_AddCommand ("menu_particles", M_Menu_Particles_f, 0);
	Cmd_AddCommand ("help", M_Menu_Help_f, 0);
	Cmd_AddCommand ("menu_maps", M_Menu_Maps_f, 0);
	Cmd_AddCommand ("menu_demos", M_Menu_Demos_f, 0);
	Cmd_AddCommand ("menu_quit", M_Menu_Quit_f, 0);

	Cvar_RegisterString (&key_menuclose);
	Cvar_RegisterString (&key_menuprev);

	// save a texture slot for translated player picture
	translate_texture = texture_extension_number++;
#ifdef HEXEN2_SUPPORT
	texture_extension_number += NUM_CLASSES-1;
#endif

	M_LoadCharTexture (m_scroll_up_data, "m_scroll_up", &m_scroll_up);
	M_LoadCharTexture (m_scroll_dn_data, "m_scroll_dn", &m_scroll_dn);
}

void M_Draw (void)
{
	if (key_dest != key_menu)
		return;

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		gMenuSounds = MenuSoundsHexen2;
	else
#endif
		gMenuSounds = MenuSoundsDefault;

	if (menu_current == NULL)
	{
		if (m_return_state)
		{
			menu_current = m_return_state;
			m_return_state = NULL;
		}
		//else return;
		else M_Menu_Main_f (SRC_COMMAND);
	}

	if (!m_recursiveDraw)
	{
//		scr_copyeverything = 1;

		Draw_ConbackSolid ();	// redraw conback over console text
		Draw_DimScreen ();
//		scr_fullupdate = 0;
	}
	else
	{
		m_recursiveDraw = false;
	}

	M_SetScale (true);
	menu_current->drawproc();
	M_SetScale (false);

	M_DrawKeyLegend ();

	if (m_entersound)
	{
		S_LocalSound (gMenuSounds[1]);
		m_entersound = false;
	}

	S_ExtraUpdate ();
}

qboolean M_WantsMouse (void)
{
	return (m_bind_grab && (menu_current == (menu_t *)&menu_keys));
}

qboolean M_HandleKey (int key, qboolean down)
{
	void (*enterproc)(cmd_source_t);
	int mods, i, keybits, maxlines;

	if (!menu_current)
		return false;

	if (!down && (!m_bind_grab || (menu_current != &menu_menuconsole)))
		return false;

#ifdef _DEBUG
	if (key != K_LSHIFT && key != K_SHIFT)
		i = 9;
#endif

	key_menuclose.value = M_StringToKeynum (key_menuclose.string);
	key_menuprev.value = M_StringToKeynum (key_menuprev.string);

#ifdef HEXEN2_SUPPORT
	if (hexen2)
		gMenuSounds = MenuSoundsHexen2;
	else
#endif
		gMenuSounds = MenuSoundsDefault;

	mods = 0;
/*	for (i = 0; i < NUM_KEYMODS*3; i++)
	{
		if (keydown[m_keymods[i].key] && (key != m_keymods[i].key))
			mods |= m_keymods[i].bits;
	}*/
//	mods = ((keydown[K_ALT] && (key != K_ALT) && (key != K_LALT) && (key != K_RALT)) ? KEYMOD_ALT : 0) |
//			((keydown[K_CTRL] && (key != K_CTRL) && (key != K_LCTRL) && (key != K_RCTRL)) ? KEYMOD_CTRL : 0) |
//			((keydown[K_SHIFT] && (key != K_SHIFT) && (key != K_LSHIFT) && (key != K_RSHIFT)) ? KEYMOD_SHIFT : 0);
	for (i = 0; i < NUM_KEYMODS; i++)
	{
		keybits = 0;
		if (keydown[m_keymods[i*3+1].key] && (key != m_keymods[i*3+1].key))		// Left modifier
			keybits |= m_keymods[i*3+1].bits;

		if (keydown[m_keymods[i*3+2].key] && (key != m_keymods[i*3+2].key))		// Right modifier
			keybits |= m_keymods[i*3+2].bits;

		if (keybits)
			mods |= (keybits | m_keymods[i*3].bits);		// if either L or R is down, also set generic (eg. K_SHIFT)
	}
	key |= mods;

#ifdef _DEBUG
//	Con_DPrintf ("key %s: $%05X\n", down ? "down" : "up", key);
#endif

	if (down && (key == K_ENTER))
	{
		if (menu_current->cursor < M_MAX_ITEMS)
		{
			enterproc = menu_current->items[menu_current->cursor].enterproc;
			if (enterproc)
			{
				m_entersound = true;
			#ifdef HEXEN2_SUPPORT
//				if (hexen2 && (menu_current == &menu_sp_H2))
				if (hexen2 && (menu_current == &menu_sp))
					m_enter_portals = 0;
			#endif
				enterproc (SRC_COMMAND);
				return true;
			}
		}
	}

	if (menu_current->keyproc && menu_current->keyproc (key, down))
		return true;

	if (!down)
		return false;

//	mods = ((keydown[K_ALT] && (key != K_ALT)) ? KEYMOD_ALT : 0) |
//			((keydown[K_CTRL] && (key != K_CTRL)) ? KEYMOD_CTRL : 0) |
//			((keydown[K_SHIFT] && (key != K_SHIFT)) ? KEYMOD_SHIFT : 0);

	if (M_KeyMatches(key, &key_menuprev))
	{
		if (menu_current->closeproc && menu_current->closeproc ())
			return true;

		if (menu_current->parentmenu && menu_current->parentmenu->openproc)
		{
			menu_current->parentmenu->openproc (SRC_COMMAND);
			return true;
		}
	}
	else if (M_KeyMatches(key, &key_menuclose))
	{
		if (!menu_current->closeproc || !menu_current->closeproc ())
		{
			key_dest = key_game;
			m_close_time = realtime;
		}
		return true;
	}

	switch (key)
	{
	case K_ENTER:
//		m_entersound = true;
	case K_RIGHTARROW:
		M_AdjustSliders (menu_current, 1, key);
		return true;

	case K_LEFTARROW:
		M_AdjustSliders (menu_current, -1, key);
		return true;

/*	case K_ESCAPE:
		if (menu_current->closeproc && menu_current->closeproc ())
			return;
		key_dest = key_game;
		m_close_time = realtime;
		return;

	case K_BACKSPACE:
		if (menu_current->closeproc && menu_current->closeproc ())
			return;

		if (menu_current->parentmenu && menu_current->parentmenu->openproc)
		{
			menu_current->parentmenu->openproc (SRC_COMMAND);
			return;
		}
*/	}

	if ((menu_current == &menu_maps) || (menu_current == &menu_demos))
	{
		maxlines = MAXLINES;
	}
	else
	{
		maxlines = ((menu_current->flags & M_NO_CVARINFO) ? MAX_VIEWITEMS+2 : MAX_VIEWITEMS);
	}

	return M_Menu_NavKey (menu_current, key, maxlines);
}

void M_ConfigureNetSubsystem (void)
{
// enable/disable net systems to match desired config
	Cbuf_AddText ("stopdemo\n", SRC_COMMAND);
//	if (SerialConfig || DirectConfig)
//		Cbuf_AddText ("com1 enable\n");

	if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
}

#endif		//#ifndef RQM_SV_ONLY
