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

// draw.h -- these are the only functions outside the refresh allowed
// to touch the vid buffer

#ifdef GLQUAKE
typedef struct
{
	int			width, height;
	int			texnum;
	float		sl, tl, sh, th;
} mpic_t;
#else
typedef struct
{
	int			width;
	short		height;
	byte		alpha;
	byte		pad;
	byte		data[4];	// variable sized
} mpic_t;
#endif

// indices of special chars from the conchars2 lmp
//typedef enum {CHAR2_SCRUP, CHAR2_SCRDN, CHAR2_ARROWLT, CHAR2_ARROWRT, CHAR2_ARROWUP, CHAR2_ARROWDN};

#define DRAW_CHARWIDTH  8
#define DRAW_CHARHEIGHT 8

#define DRAW_BIGCHARWIDTH  24
#define DRAW_BIGCHARHEIGHT 24

#ifdef HEXEN2_SUPPORT
#  define DRAW_BIGCHARWIDTH_H2  12
#  define DRAW_BIGCHARHEIGHT_H2 16
#endif

// flags for Draw_BigNumString:
#define DRAWNUM_ALIGNRIGHT  0x0001
#define DRAWNUM_TIGHT       0x0002
#define DRAWNUM_ALTCOLOR    0x0080

extern	mpic_t		*draw_disc;	// also used on sbar

#define Draw_Character(x, y, num) Draw_Character_Scaled((x), (y), (num), 1.0)
#define Draw_Pic(x, y, pic) Draw_Pic_Scaled((x), (y), (pic), 1.0)
#define Draw_TransPic(x, y, pic) Draw_Pic((x), (y), (pic))
#define Draw_TransPic_Scaled(x, y, pic, scale) Draw_Pic_Scaled((x), (y), (pic), (scale))

void Draw_Init (void);
void Draw_ReloadWadFile (void);
void Draw_Character_Scaled (int x, int y, int num, float scale);
//void Draw_Character2 (int x, int y, int num);
void Draw_DebugChar (char num);
void Draw_SubPic (int x, int y, const mpic_t *pic, int s, int t, int width, int height);
void Draw_Pic_Scaled (int x, int y, const mpic_t *pic, float scale);
//void Draw_TransPic (int x, int y, const mpic_t *pic);
  void Draw_TransPicTranslate (int x, int y, mpic_t *pic, const byte *translation, int playertype);
void Draw_ConsoleBackground (int lines);
void Draw_ConbackSolid (void);
void Draw_BeginDisc (void);
void Draw_EndDisc (void);
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FillRGB (int x, int y, int w, int h, const byte rgb[3]);
void Draw_DimScreen (void);
void Draw_String (int x, int y, const char *str);
void Draw_Alt_String (int x, int y, const char *str);
void Draw_BigString (int x, int y, const char *str);
void Draw_BigNumString (int x, int y, const char *str, int flags);
mpic_t *Draw_PicFromWad (const char *name);
mpic_t *Draw_GetCachePic (const char *path, qboolean crash);
mpic_t * Draw_FindCachePic (const char *filepath);
qpic_t * Draw_LoadCachePic (const char *filepath);
mpic_t * Draw_AddCachePic (const char *filepath, byte *data, int width, int height);
void Draw_CopyPicToPic (const qpic_t *pic_src, qpic_t *pic_dest, int x, int y);
void Draw_Crosshair (void);
void Draw_TextBox (int x, int y, int width, int height);
void GL_SetFontFilter (float filter);

#ifdef HEXEN2_SUPPORT
  void Draw_SmallString(int x, int y, const char *str);
//  void Draw_PicCropped(int x, int y, mpic_t *pic);
#endif
