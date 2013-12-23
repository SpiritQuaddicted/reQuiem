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
// common.c -- misc functions used in client and server

#include "quakedef.h"

//#define NUM_SAFE_ARGVS  7
#define NUM_SAFE_ARGVS  5

static	const char	*largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
static	const char	*argvdummy = " ";

static const char *safeargvs[NUM_SAFE_ARGVS] =
{
//	"-stdvid",
//	"-dibonly",
	"-nolan",
	"-nosound",
	"-nocdaudio",
	"-nojoy",
	"-nomouse"
};

cvar_t  registered = {"registered", "0"};
cvar_t  cmdline = {"cmdline", "", CVAR_FLAG_SERVER};

qboolean	msg_suppress_1 = 0;

char	com_token[1024];
int		com_argc;
const char	**com_argv;

#define CMDLINE_LENGTH	256
char	com_cmdline[CMDLINE_LENGTH];

//============================================================================

// ClearLink is used for new headnodes
void ClearLink (link_t *l)
{
	l->prev = l->next = l;
}

void RemoveLink (link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

void InsertLinkBefore (link_t *l, link_t *before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
}

void InsertLinkAfter (link_t *l, link_t *after)
{
	l->next = after->next;
	l->prev = after;
	l->prev->next = l;
	l->next->prev = l;
}

/*
============================================================================

			LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/


void Q_memcpy (void *dest, void *src, int count)
{
	int	i;

	if (count <= 0)
		return;

	if ((((long)dest | (long)src | count) & 3) == 0)
	{
		count >>= 2;
		for (i=0 ; i<count ; i++)
			((int *)dest)[i] = ((int *)src)[i];
	}
	else
		for (i=0 ; i<count ; i++)
			((byte *)dest)[i] = ((byte *)src)[i];
}

int Q_strncpy (char *dest, int bufsize, const char *src, int count)
{
	char *start = dest;

//	if (bufsize-1 < count)
//		count = bufsize-1;

	if (bufsize <= 0)
		return 0;
	
	if (count > 0)
	{
		while (*src && count-- && --bufsize)		// NOTE: order is important; count must be dec'd *before* bufsize
			*dest++ = *src++;
	}

	*dest = 0;

#if _DEBUG
	if (!bufsize)
	{
		char tmp[16];
		strncpy (tmp, start, 12);
		tmp[12] = 0;
		Con_DPrintf ("\x02""WARNING: overflow in Q_strncpy (\"%s...\")\n", tmp);
	}
#endif

	return dest-start;
}

#if 0
int Q_strcpy (char *dest, const char *src, int bufsize)
{
	char *start = dest;

//	while (*src)
//		*dest++ = *src++;
//	*dest = 0;

	if (bufsize <= 0)
		return 0;

	while (*src && --bufsize)
		*dest++ = *src++;

#if _DEBUG
	if (!bufsize)
		Con_DPrintf ("\x02""WARNING: overflow in Q_strcpy\n");
#endif

	*dest = 0;
	return dest-start;
}

int Q_strncpy (char *dest, const char *src, int count)		/*** NO null termination! ***/
{
	char *start;

#ifdef _DEBUG
	if (!strncmp(src, "exec quake.rc", count))
		start = dest;
//	if (count == 4)
//		Con_DPrintf ("\x02""Q_strncpy: count is 4; could be sizeof(char *)?\n");
#endif

	if (count <= 0)
		return 0;

	start = dest;

	/*while (count-- > 0)
	{
		*dest = *src;
		if (!*src++)
			break;
		dest++;
	}*/

	while (*src && count--)
		*dest++ = *src++;

/*	if (count)
//	if (count > 0)
		*dest = 0;
	else if (*(dest-1) != 0)
	{
		Con_DPrintf ("\x02""WARNING: overflow in Q_strncpy\n");
		*(dest-1) = 0;
	}*/

	return dest-start;
}

int Q_strncpyz (char *dest, const char *src, int size)
{
	if (size > 0)
	{
		strncpy (dest, src, size - 1);
		dest[size-1] = 0;
	}

	return size-1;		/****** FIXME *****/
}

#endif

int Q_snprintfz (char *dest, int size, const char *fmt, ...)
{
	va_list		argptr;
	int			len;

	va_start (argptr, fmt);
	len = vsnprintf (dest, size, fmt, argptr);
	va_end (argptr);

	if (len >= size)
	{
		Con_DPrintf ("\x02""WARNING: overflow in Q_snprintfz\n");
		len = size-1;
	}

	dest[size-1] = 0;
	return len;
}

int Q_atoi (const char *str)
{
	int	val, sign, c;

	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
	{
		sign = 1;
	}

	val = 0;

// check for hex
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val << 4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val << 4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val << 4) + c - 'A' + 10;
			else
				return val*sign;
		}
	}

// check for character
	if (str[0] == '\'')
		return sign * str[1];

// assume decimal
	while (1)
	{
		c = *str++;
		if (c <'0' || c > '9')
			return val*sign;
		val = val*10 + c - '0';
	}

	return 0;
}

float Q_atof (const char *str)
{
	double	val;
	int	sign, c, decimal, total;

	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
	{
		sign = 1;
	}

	val = 0;

// check for hex
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val * 16) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val * 16) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val * 16) + c - 'A' + 10;
			else
				return val*sign;
		}
	}

// check for character
	if (str[0] == '\'')
		return sign * str[1];

// assume decimal
	decimal = -1;
	total = 0;
	while (1)
	{
		c = *str++;
		if (c == '.')
		{
			decimal = total;
			continue;
		}
		if (c <'0' || c > '9')
			break;
		val = val*10 + c - '0';
		total++;
	}

	if (decimal == -1)
		return val*sign;
	while (total > decimal)
	{
		val /= 10;
		total--;
	}

	return val*sign;
}

/*
============================================================================

			BYTE ORDER FUNCTIONS

============================================================================
*/

//qboolean	bigendien;

short (*BigShort) (short l);
int   (*BigLong) (int l);
float (*BigFloat) (float l);

/****************** JDH ****************/
//short			 (*LittleShort) (short l);
//int			 (*LittleLong) (int l);
//float			 (*LittleFloat) (float l);
/****************** JDH ****************/

short ShortSwap (short l)
{
	byte	b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

short ShortNoSwap (short l)
{
	return l;
}

int LongSwap (int l)
{
	byte	b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int LongNoSwap (int l)
{
	return l;
}

float FloatSwap (float f)
{
	union
	{
		float   f;
		byte    b[4];
	} dat1, dat2;


	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap (float f)
{
	return f;
}

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

// writing functions

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte	*buf;

//#ifdef PARANOID
//	if (c < -128 || c > 127)
//		Sys_Error ("MSG_WriteChar: range error");
//#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;

//#ifdef PARANOID
//	if (c < 0 || c > 255)
//		Sys_Error ("MSG_WriteByte: range error");
//#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteCmd (sizebuf_t *sb, int c)
{
	byte	*buf;

//#ifdef PARANOID
//	if (c < 0 || c > 255)
//		Sys_Error ("MSG_WriteCmd: range error");
//#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
	sb->lastcmdpos = sb->cursize - 1;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;

//#ifdef PARANOID
//	if (c < ((short)0x8000) || c > (short)0x7fff)
//		Sys_Error ("MSG_WriteShort: range error");
//#endif

	buf = SZ_GetSpace (sb, 2);
	buf[0] = c & 0xff;
	buf[1] = c >> 8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte	*buf;

	buf = SZ_GetSpace (sb, 4);
	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = c >> 24;
}

void MSG_WriteFloat (sizebuf_t *sb, float f)
{
	union
	{
		float	f;
		int	l;
	} dat;


	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, const char *s)
{
	if (!s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, strlen(s) + 1);
}

void MSG_WriteCoord (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f * 8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, ((int)f * 256 / 360) & 255);
}

// precise aim from [sons]Quake
void MSG_WritePreciseAngle (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f * 65536.0 / 360) & 65535);
}

// reading functions
int		msg_readcount;
qboolean	msg_badread;

void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	if (msg_readcount >= net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	return (signed char)net_message.data[msg_readcount++];
}

int MSG_ReadByte (void)
{
	if (msg_readcount >= net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	return (unsigned char)net_message.data[msg_readcount++];
}


#define MSG_READSHORT(c)						\
	if (msg_readcount+2 > net_message.cursize)	\
	{											\
		msg_badread = true;						\
		return -1;								\
	}											\
	c = LittleShort (*(short *) (net_message.data + msg_readcount));	\
	msg_readcount += 2;

int MSG_ReadShort (void)
{
	int	c;

	MSG_READSHORT(c);
	return c;
}

int MSG_ReadLong (void)
{
	int	c;

	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	// JDH: this is equivalent on little-endian, don't know about big?
	c = LittleLong (*(int *) (net_message.data + msg_readcount));

/*	c = net_message.data[msg_readcount]
		+ (net_message.data[msg_readcount+1] << 8)
		+ (net_message.data[msg_readcount+2] << 16)
		+ (net_message.data[msg_readcount+3] << 24);
*/
	msg_readcount += 4;

	return c;
}

/*float MSG_ReadFloat (void)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;

	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;		// ** this should probably be a different value **
	}

	dat.b[0] = net_message.data[msg_readcount];
	dat.b[1] = net_message.data[msg_readcount+1];
	dat.b[2] = net_message.data[msg_readcount+2];
	dat.b[3] = net_message.data[msg_readcount+3];
	msg_readcount += 4;

	dat.l = LittleLong (dat.l);

	return dat.f;
}
*/

float MSG_ReadFloat (void)
{
	float f;

	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;		// ** this should probably be a different value **
	}

	// JDH: this is equivalent on little-endian, don't know about big?
	f = LittleFloat (*(float *) (net_message.data + msg_readcount));

	msg_readcount += 4;

	return f;
}

char *MSG_ReadString (void)
{
	static	char	string[MAXPRINTMSG];
	int		l, c;

	l = 0;
	do {
		if (msg_readcount >= net_message.cursize)
		{
			msg_badread = true;
			break;
		}
		c = (signed char)net_message.data[msg_readcount++];
		if (!c)
			break;
		/*c = MSG_ReadChar ();
		if (c == -1 || c == 0)
			break;*/
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);

	string[l] = 0;

	return string;
}

float MSG_ReadCoord (void)
{
	int c;

	MSG_READSHORT(c);
	return c * (1.0 / 8);
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0 / 256);
}

// precise aim from [sons]Quake
float MSG_ReadPreciseAngle (void)
{
	int c;

	MSG_READSHORT(c);
	return c * (360.0 / 65536);
}

//===========================================================================

void SZ_Alloc (sizebuf_t *buf, int startsize)
{
	if (startsize < 256)
		startsize = 256;
	buf->data = Hunk_AllocName (startsize, "sizebuf");
	buf->maxsize = startsize;
	buf->cursize = 0;
	buf->lastcmdpos = 0;
}

void SZ_Free (sizebuf_t *buf)
{
//      Z_Free (buf->data);
//      buf->data = NULL;
//      buf->maxsize = 0;
	buf->cursize = 0;
	buf->lastcmdpos = 0;
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->lastcmdpos = 0;
}

void SZ_CheckSpace (sizebuf_t *buf, int length)
{
	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error ("SZ_CheckSpace: overflow without allowoverflow set");

		if (length > buf->maxsize)
			Sys_Error ("SZ_CheckSpace: %i is > full buffer size", length);

		buf->overflowed = true;
		Con_Print ("SZ_CheckSpace: overflow");
		SZ_Clear (buf);
	}
}

void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void	*data;

	SZ_CheckSpace (buf, length);

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write (sizebuf_t *buf, const void *data, int length)
{
	memcpy (SZ_GetSpace(buf, length), data, length);
}

void SZ_Print (sizebuf_t *buf, const char *data)
{
	int	len;

	len = strlen(data) + 1;

// byte * cast to keep VC++ happy
	if (buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len), data, len);	// no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len - 1) - 1, data, len); // write over trailing 0
}

//============================================================================

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
const char *COM_Parse (const char *data)
{
	int	c, len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

// skip whitespace (JDH: typecast to byte so chars > 127 work)
skipwhite:
	while (((c = *(byte *)data) <= ' '))
	{
		if (c == 0)
			return NULL;		// end of file;
		data++;
	}

// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}


// handle quoted strings specially
	if (c == '\"')
	{
		data++;		
		while (1)
		{
			c = *data++;
			if (c == '\"' || !c)
			{
				com_token[len] = 0;
				//return data;
				return (c ? data : data-1);
			}
			com_token[len] = c;
			len++;
		}
	}

// parse single characters
	if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || c == ':')
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *(byte *)data;				// JDH: high-ASCII support
		// joe, from ProQuake: removed ':' so that ip:port works
		if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' /*|| c==':'*/)
			break;
	} while (c > 32);

	com_token[len] = 0;
	return data;
}


/*
================
COM_FindNextParm

Returns the position (start to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int COM_FindNextParm (const char *parm, int start)
{
	int	i;

	for (i=start ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP sometimes clears appkit vars.
//JDH:	if (!strcmp(parm,com_argv[i]))
		if (!Q_strcasecmp(parm, com_argv[i]))
			return i;
	}

	return 0;
}

/*
================
COM_CheckRegistered

Looks for the pop.lmp file and verifies it.
Sets the "registered" cvar.
================
*/
void COM_CheckRegistered (void)
{
	FILE	*h;

	h = COM_FOpenFile ("gfx/pop.lmp", 0, NULL);
	if (h)
	{
		Cvar_SetDirect (&cmdline, com_cmdline);
		Cvar_SetValueDirect (&registered, 1);
		fclose (h);
	}

	registered.flags |= CVAR_FLAG_READONLY;
	cmdline.flags |= CVAR_FLAG_READONLY;
}

/*
================
COM_InitArgv
================
*/
void COM_InitArgv (int argc, const char **argv)
{
	qboolean	safe;
	int		i, j, n;

// reconstitute the command line for the cmdline externally visible cvar
	n = 0;

	for (j=0 ; j<MAX_NUM_ARGVS && j<argc ; j++)
	{
		i = 0;

		while ((n < (CMDLINE_LENGTH - 1)) && argv[j][i])
			com_cmdline[n++] = argv[j][i++];

		if (n < (CMDLINE_LENGTH - 1))
			com_cmdline[n++] = ' ';
		else
			break;
	}

	com_cmdline[n] = 0;

	safe = false;

	for (com_argc = 0 ; com_argc < MAX_NUM_ARGVS && com_argc < argc ; com_argc++)
	{
		largv[com_argc] = argv[com_argc];
		if (!strcmp("-safe", argv[com_argc]))
			safe = true;
	}

	if (safe)
	{
	// force all the safe-mode switches. Note that we reserved extra space in
	// case we need to add these, so we don't need an overflow check
		for (i=0 ; i<NUM_SAFE_ARGVS ; i++)
		{
			largv[com_argc] = safeargvs[i];
			com_argc++;
		}
	}

	largv[com_argc] = argvdummy;
	com_argv = largv;

//JDH - moved check for mission pack switches to COM_InitFilesystem
}

/*
================
COM_Init
================
*/
void COM_Init (void)
{
//	byte    swaptest[2] = {1, 0};

// set the byte swapping variables in a portable manner
//	if (*(short *)swaptest == 1)
	{
//		bigendien = false;
		BigShort = ShortSwap;
		BigLong = LongSwap;
		BigFloat = FloatSwap;

/****************** JDH ****************
//		LittleShort = ShortNoSwap;
//		LittleLong = LongNoSwap;
//		LittleFloat = FloatNoSwap;
	}
	else
	{
		bigendien = true;
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
****************** JDH ****************/
	}

	Cvar_RegisterBool (&registered);
	Cvar_RegisterString (&cmdline);

	COM_InitFilesystem ();
	COM_CheckRegistered ();
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char *va (const char *format, ...)
{
	va_list         argptr;
	static	char	string[8][2048];
	static	int	idx = 0;

	idx++;
	if (idx == 8)
		idx = 0;

	va_start (argptr, format);
	vsnprintf (string[idx], sizeof(string[idx]), format, argptr);
	va_end (argptr);

	return string[idx];
}

char *CopyString (const char *in)
{
	char	*out;

	out = Z_Malloc (strlen(in) + 1);
	strcpy (out, in);

	return out;
}
