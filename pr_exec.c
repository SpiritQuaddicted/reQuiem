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

#include "quakedef.h"


typedef struct
{
	int		s;
	dfunction_t	*f;
} prstack_t;

/********JDH********/
#define	MAX_STACK_DEPTH_OLD		32
#define	MAX_STACK_DEPTH			64

#ifdef HEXEN2_SUPPORT
  extern int *pr_field_map;
#endif
/********JDH********/

prstack_t	pr_stack[MAX_STACK_DEPTH];
int		pr_depth;

#define	LOCALSTACK_SIZE		2048
int		localstack[LOCALSTACK_SIZE];
int		localstack_used;


qboolean	pr_trace;
dfunction_t	*pr_xfunction;
int		pr_xstatement;


int		pr_argc;

#ifdef PR_DEBUGGER
qboolean	pr_debug;
#endif

char *pr_opnames[] =
{
	"DONE",

	"MUL_F",
	"MUL_V",
	"MUL_FV",
	"MUL_VF",

	"DIV",

	"ADD_F",
	"ADD_V",

	"SUB_F",
	"SUB_V",

	"EQ_F",
	"EQ_V",
	"EQ_S",
	"EQ_E",
	"EQ_FNC",

	"NE_F",
	"NE_V",
	"NE_S",
	"NE_E",
	"NE_FNC",

	"LE",
	"GE",
	"LT",
	"GT",

	"INDIRECT",
	"INDIRECT",
	"INDIRECT",
	"INDIRECT",
	"INDIRECT",
	"INDIRECT",

	"ADDRESS",

	"STORE_F",
	"STORE_V",
	"STORE_S",
	"STORE_ENT",
	"STORE_FLD",
	"STORE_FNC",

	"STOREP_F",
	"STOREP_V",
	"STOREP_S",
	"STOREP_ENT",
	"STOREP_FLD",
	"STOREP_FNC",

	"RETURN",

	"NOT_F",
	"NOT_V",
	"NOT_S",
	"NOT_ENT",
	"NOT_FNC",

	"IF",
	"IFNOT",

	"CALL0",
	"CALL1",
	"CALL2",
	"CALL3",
	"CALL4",
	"CALL5",
	"CALL6",
	"CALL7",
	"CALL8",

	"STATE",

	"GOTO",

	"AND",
	"OR",

	"BITAND",
	"BITOR"
};

extern char *PR_GlobalString (int ofs);
extern char *PR_GlobalStringNoContents (int ofs);


//=============================================================================

/*
=================
PR_PrintStatement
=================
*/
void PR_PrintStatement (dstatement_t *s)
{
	int	i;

	if ((unsigned)s->op < sizeof(pr_opnames)/sizeof(pr_opnames[0]))
	{
		Con_Printf ("%s ",  pr_opnames[s->op]);
		i = strlen(pr_opnames[s->op]);
		for ( ; i<10 ; i++)
			Con_Print (" ");
	}

	if (s->op == OP_IF || s->op == OP_IFNOT)
	{
		Con_Printf ("%sbranch %i",PR_GlobalString(s->a),s->b);
	}
	else if (s->op == OP_GOTO)
	{
		Con_Printf ("branch %i",s->a);
	}
	else if ( (unsigned)(s->op - OP_STORE_F) < 6)
	{
		Con_Printf ("%s",PR_GlobalString(s->a));
		Con_Printf ("%s", PR_GlobalStringNoContents(s->b));
	}
	else
	{
		if (s->a)
			Con_Printf ("%s",PR_GlobalString(s->a));
		if (s->b)
			Con_Printf ("%s",PR_GlobalString(s->b));
		if (s->c)
			Con_Printf ("%s", PR_GlobalStringNoContents(s->c));
	}
	Con_Print ("\n");
}

/*
============
PR_StackTrace
============
*/
void PR_StackTrace (void)
{
	dfunction_t	*f;
	int		i;

	if (pr_depth == 0)
	{
		Con_Print ("<NO STACK>\n");
		return;
	}

	pr_stack[pr_depth].f = pr_xfunction;
	for (i=pr_depth ; i>=0 ; i--)
	{
		f = pr_stack[i].f;

		if (!f)
			Con_Print ("<NO FUNCTION>\n");
		else
			Con_Printf ("%12s : %s\n", /*pr_strings +*/ f->s_file, /*pr_strings +*/ f->s_name);
	}
}


/*
============
PR_Profile_f
============
*/
void PR_Profile_f (cmd_source_t src)
{
	dfunction_t	*f, *best;
	int		i, max, num;
	
	if (!progs)
	{
		Con_Print ("Profile: no progs.dat loaded!\n" );
		return;
	}

	num = 0;
	do
	{
		max = 0;
		best = NULL;
		for (i=0 ; i<progs->numfunctions ; i++)
		{
			f = &pr_functions[i];
			if (f->profile > max)
			{
				max = f->profile;
				best = f;
			}
		}
		if (best)
		{
			if (num < 10)
				Con_Printf ("%7i %s\n", best->profile, /*pr_strings +*/ best->s_name);
			num++;
			best->profile = 0;
		}
	} while (best);
}

#ifdef PR_DEBUGGER
/*
============
PR_Debugger_f
============
*/
void PR_Debugger_f (cmd_source_t src)
{
	pr_debug = !pr_debug;
}

void SCR_DrawDebugger (byte *text, int len)
{
	int y, x;
	
	for (y = 0; y < 1; y++)
	{
		for (x = 0 ; x < vid.width/8 ; x++)
		{
			if (text[x] == '\n')
				break;
			
			Draw_Character ((x+1)*8, y, text[x]);
		}
	}
}

void SCR_UpdateDebugger (byte *data, int len)
{
	extern mpic_t	*conback;

	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);

	GL_Set2D ();
	R_PolyBlend ();
	Draw_Pic (0, 0, conback);
	SCR_DrawDebugger (data, len);
	R_BrightenScreen ();
	
	GL_EndRendering ();
}

/*
============
PR_ShowQC
============
*/
void PR_ShowQC (byte *data, int len)
{
	double	oldtime = 0, newtime;

	S_ClearBuffer ();		// so dma doesn't loop current sound

	key_lastpress = -1;		// JDH: so previous keypresses don't count
	while (1)
	{
		newtime = Sys_DoubleTime ();
		if (newtime-oldtime >= 0.1)
		{
			SCR_UpdateDebugger (data, len);
			oldtime = newtime;
		}

		key_count = -3;		// purge keyup events, then wait for a key down
		Sys_SendKeyEvents ();

		if (key_lastpress == K_ESCAPE)
			break;
	}
}

#define PR_TABWIDTH 4

int PR_FormatQC (byte *datain, int sizein, byte *dataout)
{
	int sizeout, numlines, linelen;
	byte *curr;

	sizeout = 0;
	numlines = 1;
	linelen = 0;

	for (curr = datain; curr < datain+sizein; curr++)
	{
		if ((*curr == '\n') || (*curr == '\r'))
		{
			sizeout += linelen;
			if (dataout)
				dataout[sizeout] = '\n';
			sizeout++;
			linelen = 0;
			numlines++;
			if ((*curr == '\r') && (*(curr+1) == '\n'))
				curr++;
		}
		else if (*curr == '\t')
		{
			do 
			{
				if (dataout)
					dataout[sizeout+linelen] = ' ';
				linelen++;
			}
			while (linelen % PR_TABWIDTH);
		}
		else
		{
			if (dataout)
				dataout[sizeout+linelen] = *curr;
			linelen++;
		}
	}

	return sizeout;
}
	
/*
============
PR_EnterDebugger
============
*/
void PR_EnterDebugger (void)
{
	byte *data1, *data2;
	int origsize, newsize;
	
	data1 = COM_LoadMallocFile ("prtest.qc", FILE_NO_PAKS | FILE_NO_DZIPS);
	if (!data1)
		return;

	origsize = com_filesize;
	newsize = PR_FormatQC (data1, origsize, NULL);
	
	data2 = malloc (newsize);
	if (data2)
	{
		PR_FormatQC (data1, origsize, data2);
		PR_ShowQC (data2, newsize);
		free (data2);
	}

	free (data1);
}

#endif		// #ifdef PR_DEBUGGER

/*
============
PR_RunError

Aborts the currently executing function
============
*/
void PR_RunError (const char *error, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr, error);
	vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	PR_PrintStatement (pr_statements + pr_xstatement);
	PR_StackTrace ();
	Con_Printf ("%s\n", string);
	
	pr_depth = 0;		// dump the stack so host_error can shutdown functions

	Host_Error ("Program error");
}

/*
============================================================================
PR_ExecuteProgram

The interpretation main loop
============================================================================
*/

/*
====================
PR_EnterFunction

Returns the new program statement counter
====================
*/
int PR_EnterFunction (dfunction_t *f)
{
	int	i, j, c, o;

	pr_stack[pr_depth].s = pr_xstatement;
	pr_stack[pr_depth].f = pr_xfunction;	
	pr_depth++;
	
	if (pr_depth >= MAX_STACK_DEPTH)
		PR_RunError ("stack overflow");
	else if (pr_depth >= MAX_STACK_DEPTH_OLD)
	{
		Con_DPrintf ("WARNING: QuakeC has exceeded the original call stack depth of %i (%i)\n", 
						MAX_STACK_DEPTH_OLD, pr_depth);
	}

// save off any locals that the new function steps on
	c = f->locals;
	if (localstack_used + c > LOCALSTACK_SIZE)
		PR_RunError ("PR_ExecuteProgram: locals stack overflow\n");

	for (i=0 ; i<c ; i++)
		localstack[localstack_used+i] = ((int *)pr_globals)[f->parm_start + i];
	localstack_used += c;

// copy parameters
	o = f->parm_start;
	for (i=0 ; i<f->numparms ; i++)
	{
		for (j=0 ; j<f->parm_size[i] ; j++)
		{
			((int *)pr_globals)[o] = ((int *)pr_globals)[OFS_PARM0+i*3+j];
			o++;
		}
	}

	pr_xfunction = f;
	return f->first_statement - 1;	// offset the s++
}

/*
====================
PR_LeaveFunction
====================
*/
int PR_LeaveFunction (void)
{
	int	i, c;

	if (pr_depth <= 0)
		Sys_Error ("prog stack underflow");

// restore locals from the stack
	c = pr_xfunction->locals;
	localstack_used -= c;
	if (localstack_used < 0)
		PR_RunError ("PR_ExecuteProgram: locals stack underflow\n");

	for (i=0 ; i < c ; i++)
		((int *)pr_globals)[pr_xfunction->parm_start + i] = localstack[localstack_used+i];

// up stack
	pr_depth--;
	pr_xfunction = pr_stack[pr_depth].f;
	return pr_stack[pr_depth].s;
}

/*
====================
PR_ExecuteProgram
====================
*/
void PR_ExecuteProgram (func_t fnum)
{
	eval_t		*a, *b, *c, *ptr;
	int		i, s, runaway, exitdepth;
	dstatement_t	*st;
	dfunction_t	*f, *newf;
	edict_t		*ed;

#ifdef HEXEN2_SUPPORT
	int startFrame, endFrame;
	float val, switch_float;
	int case_type=-1;
#endif
	//	DWORD dwTicks;

	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start
	char	*funcname;
	char	*remaphint;
	// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end

	if (!fnum || fnum >= progs->numfunctions)
	{
		if (PR_GLOBAL(self))
			ED_Print (PROG_TO_EDICT(PR_GLOBAL(self)));
		
		Host_Error ("PR_ExecuteProgram: NULL function");
	}
	
#ifdef PR_DEBUGGER
	if (pr_debug)
	{
		PR_EnterDebugger ();
		pr_debug = false;
	}
#endif

	f = &pr_functions[fnum];

	runaway = 100000;
	pr_trace = false;

// make a stack frame
	exitdepth = pr_depth;

	s = PR_EnterFunction (f);
	
while (1)
{
	s++;	// next statement
	st = &pr_statements[s];

	a = (eval_t *)&pr_globals[(unsigned short)st->a];		// JDH: originally signed
	b = (eval_t *)&pr_globals[(unsigned short)st->b];
	c = (eval_t *)&pr_globals[(unsigned short)st->c];

	if (!--runaway)
		PR_RunError ("runaway loop error");
		
	pr_xfunction->profile++;
	pr_xstatement = s;
	
	if (pr_trace)
		PR_PrintStatement (st);
		
	switch (st->op)
	{
	case OP_ADD_F:
		c->_float = a->_float + b->_float;
		break;
	case OP_ADD_V:
		c->vector[0] = a->vector[0] + b->vector[0];
		c->vector[1] = a->vector[1] + b->vector[1];
		c->vector[2] = a->vector[2] + b->vector[2];
		break;
		
	case OP_SUB_F:
		c->_float = a->_float - b->_float;
		break;
	case OP_SUB_V:
		c->vector[0] = a->vector[0] - b->vector[0];
		c->vector[1] = a->vector[1] - b->vector[1];
		c->vector[2] = a->vector[2] - b->vector[2];
		break;

	case OP_MUL_F:
		c->_float = a->_float * b->_float;
		break;
	case OP_MUL_V:
		c->_float = a->vector[0]*b->vector[0]
				+ a->vector[1]*b->vector[1]
				+ a->vector[2]*b->vector[2];
		break;
	case OP_MUL_FV:
		c->vector[0] = a->_float * b->vector[0];
		c->vector[1] = a->_float * b->vector[1];
		c->vector[2] = a->_float * b->vector[2];
		break;
	case OP_MUL_VF:
		c->vector[0] = b->_float * a->vector[0];
		c->vector[1] = b->_float * a->vector[1];
		c->vector[2] = b->_float * a->vector[2];
		break;

	case OP_DIV_F:
		c->_float = a->_float / b->_float;
		break;
	
	case OP_BITAND:
		c->_float = (int)a->_float & (int)b->_float;
		break;
	
	case OP_BITOR:
		c->_float = (int)a->_float | (int)b->_float;
		break;
	
		
	case OP_GE:
		c->_float = a->_float >= b->_float;
		break;
	case OP_LE:
		c->_float = a->_float <= b->_float;
		break;
	case OP_GT:
		c->_float = a->_float > b->_float;
		break;
	case OP_LT:
		c->_float = a->_float < b->_float;
		break;
	case OP_AND:
		c->_float = a->_float && b->_float;
		break;
	case OP_OR:
		c->_float = a->_float || b->_float;
		break;
		
	case OP_NOT_F:
		c->_float = !a->_float;
		break;
	case OP_NOT_V:
		c->_float = !a->vector[0] && !a->vector[1] && !a->vector[2];
		break;
	case OP_NOT_S:
		c->_float = !a->string || !pr_strings[a->string];
		break;
	case OP_NOT_FNC:
		c->_float = !a->function;
		break;
	case OP_NOT_ENT:
		c->_float = (PROG_TO_EDICT(a->edict) == sv.edicts);
		break;

	case OP_EQ_F:
		c->_float = a->_float == b->_float;
		break;
	case OP_EQ_V:
		c->_float = (a->vector[0] == b->vector[0]) &&
					(a->vector[1] == b->vector[1]) &&
					(a->vector[2] == b->vector[2]);
		break;
	case OP_EQ_S:
		c->_float = !strcmp(pr_strings+a->string,pr_strings+b->string);
		break;
	case OP_EQ_E:
		c->_float = a->_int == b->_int;
		break;
	case OP_EQ_FNC:
		c->_float = a->function == b->function;
		break;


	case OP_NE_F:
		c->_float = a->_float != b->_float;
		break;
	case OP_NE_V:
		c->_float = (a->vector[0] != b->vector[0]) ||
					(a->vector[1] != b->vector[1]) ||
					(a->vector[2] != b->vector[2]);
		break;
	case OP_NE_S:
		c->_float = strcmp(pr_strings+a->string,pr_strings+b->string);
		break;
	case OP_NE_E:
		c->_float = a->_int != b->_int;
		break;
	case OP_NE_FNC:
		c->_float = a->function != b->function;
		break;

//==================
	case OP_STORE_F:
	case OP_STORE_ENT:
	case OP_STORE_FLD:		// integers
	case OP_STORE_S:
	case OP_STORE_FNC:		// pointers
		b->_int = a->_int;
		break;
	case OP_STORE_V:
		b->vector[0] = a->vector[0];
		b->vector[1] = a->vector[1];
		b->vector[2] = a->vector[2];
		break;
		
	case OP_STOREP_F:
	case OP_STOREP_ENT:
	case OP_STOREP_FLD:		// integers
	case OP_STOREP_S:
	case OP_STOREP_FNC:		// pointers
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->_int = a->_int;
		break;
	case OP_STOREP_V:
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->vector[0] = a->vector[0];
		ptr->vector[1] = a->vector[1];
		ptr->vector[2] = a->vector[2];
		break;
		
	case OP_ADDRESS:
		ed = PROG_TO_EDICT(a->edict);
//#ifdef PARANOID
//		NUM_FOR_EDICT(ed);		// make sure it's in range
//#endif
		if (ed == (edict_t *)sv.edicts && sv.state == ss_active)
			PR_RunError ("assignment to world entity");
	/******JDH******/
	#ifdef HEXEN2_SUPPORT
		c->_int = (byte *)((int *)&ed->v + pr_field_map[b->_int]) - (byte *)sv.edicts;
	#else
		c->_int = (byte *)((int *)&ed->v + b->_int) - (byte *)sv.edicts;
	#endif
	/******JDH******/
		break;
		
	case OP_LOAD_F:
	case OP_LOAD_FLD:
	case OP_LOAD_ENT:
	case OP_LOAD_S:
	case OP_LOAD_FNC:
		ed = PROG_TO_EDICT(a->edict);
//#ifdef PARANOID
//		NUM_FOR_EDICT(ed);		// make sure it's in range
//#endif
	/******JDH******/
	#ifdef HEXEN2_SUPPORT
		a = (eval_t *)((int *)&ed->v + pr_field_map[b->_int]);
	#else
		a = (eval_t *)((int *)&ed->v + b->_int);
	#endif
	/******JDH******/
		c->_int = a->_int;
		break;

	case OP_LOAD_V:
		ed = PROG_TO_EDICT(a->edict);
//#ifdef PARANOID
	//	NUM_FOR_EDICT(ed);		// make sure it's in range
//#endif
	/******JDH******/
	#ifdef HEXEN2_SUPPORT
		a = (eval_t *)((int *)&ed->v + pr_field_map[b->_int]);
	#else
		a = (eval_t *)((int *)&ed->v + b->_int);
	#endif
	/******JDH******/
		c->vector[0] = a->vector[0];
		c->vector[1] = a->vector[1];
		c->vector[2] = a->vector[2];
		break;
		
//==================

	case OP_IFNOT:
		if (!a->_int)
			s += st->b - 1;	// offset the s++
		break;
		
	case OP_IF:
		if (a->_int)
			s += st->b - 1;	// offset the s++
		break;
		
	case OP_GOTO:
		s += st->a - 1;	// offset the s++
		break;
		
	case OP_CALL8:
	case OP_CALL7:
	case OP_CALL6:
	case OP_CALL5:
	case OP_CALL4:
	case OP_CALL3:
	case OP_CALL2:
		#ifdef HEXEN2_SUPPORT
			// Copy second arg to shared space
			if (hexen2 && st->c) 
				VectorCopy (c->vector, G_VECTOR(OFS_PARM1));
		#endif
	case OP_CALL1: 
		#ifdef HEXEN2_SUPPORT
			// Copy first arg to shared space
			if (hexen2 && st->b) 
				VectorCopy (b->vector, G_VECTOR(OFS_PARM0));
		#endif
	case OP_CALL0:
//		dwTicks = GetTickCount();
	
		pr_argc = st->op - OP_CALL0;
		if (!a->function)
			PR_RunError ("NULL function");

		newf = &pr_functions[a->function];

		/*
		if (newf->first_statement < 0)
		{	// negative statements are built in functions
			i = -newf->first_statement;
			if (i >= pr_numbuiltins)
				PR_RunError ("Bad builtin call number");
			pr_builtins[i] ();
			break;
		}
		*/

		i = newf->first_statement;
		if (i < 0)
		{	
			// negative statements are built in functions
			i = -i;
// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  start

			if ((i >= pr_numbuiltins) || (pr_builtins[i] == pr_ebfs_builtins[0].function))
			{
				funcname = /*pr_strings +*/ newf->s_name;
				if (pr_builtin_remap.value)
				{
					remaphint = NULL;
				}
				else
				{
					remaphint = "Try \"builtin remapping\" by setting PR_BUILTIN_REMAP to 1\n";
				}

				PR_RunError ("Bad builtin call number %i for %s\nPlease contact the PROGS.DAT author\nUse BUILTINLIST to see all assigned builtin functions\n%s", i, funcname, remaphint);
			}

// 2001-09-14 Enhanced BuiltIn Function System (EBFS) by Maddes  end
			pr_builtins[i] ();

//			dwTicks = GetTickCount() - dwTicks;
//			if (( dwTicks > 0 ) && (st->op == OP_CALL1))
//				Con_DPrintf ("CALL1: i=%i (%i ms)\n", i, dwTicks);
			break;
		}

#ifdef _DEBUG
//	pr_trace = !strcmp(newf->s_name, "CycleWeaponCommand");
#endif
		s = PR_EnterFunction (newf);
		break;

	case OP_DONE:
	case OP_RETURN:
		pr_globals[OFS_RETURN] = pr_globals[(unsigned short)st->a];		// JDH: originally signed
		pr_globals[OFS_RETURN+1] = pr_globals[(unsigned short)st->a+1];
		pr_globals[OFS_RETURN+2] = pr_globals[(unsigned short)st->a+2];
		s = PR_LeaveFunction ();
		if (pr_depth == exitdepth)
			return;		// all done
		break;
		
	case OP_STATE:
		ed = PROG_TO_EDICT(PR_GLOBAL(self));
	#ifdef FPS_20
		ed->v.nextthink = PR_GLOBAL(time) + 0.05;
	#else
	  #ifdef HEXEN2_SUPPORT
		ed->v.nextthink = PR_GLOBAL(time) + (hexen2 ? HX_FRAME_TIME : 0.1);
	  #else
		ed->v.nextthink = PR_GLOBAL(time) + 0.1;
	  #endif
	#endif
		if (a->_float != ed->v.frame)
		{
			ed->v.frame = a->_float;
		}
		ed->v.think = b->function;
		break;
		
#ifdef HEXEN2_SUPPORT
	case OP_MULSTORE_F: // f *= f
		b->_float *= a->_float;
		break;
	case OP_MULSTORE_V: // v *= f
		b->vector[0] *= a->_float;
		b->vector[1] *= a->_float;
		b->vector[2] *= a->_float;
		break;
	case OP_MULSTOREP_F: // e.f *= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float *= a->_float);
		break;
	case OP_MULSTOREP_V: // e.v *= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->vector[0] = (ptr->vector[0] *= a->_float);
		c->vector[0] = (ptr->vector[1] *= a->_float);
		c->vector[0] = (ptr->vector[2] *= a->_float);
		break;

	case OP_DIVSTORE_F: // f /= f
		b->_float /= a->_float;
		break;
	case OP_DIVSTOREP_F: // e.f /= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float /= a->_float);
		break;

	case OP_ADDSTORE_F: // f += f
		b->_float += a->_float;
		break;
	case OP_ADDSTORE_V: // v += v
		b->vector[0] += a->vector[0];
		b->vector[1] += a->vector[1];
		b->vector[2] += a->vector[2];
		break;
	case OP_ADDSTOREP_F: // e.f += f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float += a->_float);
		break;
	case OP_ADDSTOREP_V: // e.v += v
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->vector[0] = (ptr->vector[0] += a->vector[0]);
		c->vector[1] = (ptr->vector[1] += a->vector[1]);
		c->vector[2] = (ptr->vector[2] += a->vector[2]);
		break;

	case OP_SUBSTORE_F: // f -= f
		b->_float -= a->_float;
		break;
	case OP_SUBSTORE_V: // v -= v
		b->vector[0] -= a->vector[0];
		b->vector[1] -= a->vector[1];
		b->vector[2] -= a->vector[2];
		break;
	case OP_SUBSTOREP_F: // e.f -= f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->_float = (ptr->_float -= a->_float);
		break;
	case OP_SUBSTOREP_V: // e.v -= v
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		c->vector[0] = (ptr->vector[0] -= a->vector[0]);
		c->vector[1] = (ptr->vector[1] -= a->vector[1]);
		c->vector[2] = (ptr->vector[2] -= a->vector[2]);
		break;

	case OP_FETCH_GBL_F:
	case OP_FETCH_GBL_S:
	case OP_FETCH_GBL_E:
	case OP_FETCH_GBL_FNC:
		i = (int) b->_float;
		if (i < 0 || i > G_INT((unsigned short) st->a - 1))
		{
			PR_RunError("array index out of bounds: %d", i);
		}
		a = (eval_t *) &pr_globals[(unsigned short) st->a + i];
		c->_int = a->_int;
		break;
	case OP_FETCH_GBL_V:
		i = (int) b->_float;
		if (i < 0 || i > G_INT((unsigned short) st->a - 1))
		{
			PR_RunError("array index out of bounds: %d", i);
		}
		a = (eval_t *) &pr_globals[(unsigned short) st->a + ((int) b->_float)*3];
		c->vector[0] = a->vector[0];
		c->vector[1] = a->vector[1];
		c->vector[2] = a->vector[2];
		break;

	case OP_CSTATE:		// Cycle state
		ed = PROG_TO_EDICT( *pr_global_ptrs.self );
		ed->v.nextthink = *pr_global_ptrs.time + HX_FRAME_TIME;
		ed->v.think = pr_xfunction - pr_functions;
		*pr_global_ptrs.cycle_wrapped = false;
		startFrame = (int) a->_float;
		endFrame = (int) b->_float;
		if( startFrame <= endFrame )
		{ // Increment
			if( ed->v.frame < startFrame || ed->v.frame > endFrame )
			{
				ed->v.frame = startFrame;
				break;
			}
			ed->v.frame++;
			if( ed->v.frame > endFrame )
			{
				*pr_global_ptrs.cycle_wrapped = true;
				ed->v.frame = startFrame;
			}
			break;
		}
		// Decrement
		if( ed->v.frame > startFrame || ed->v.frame < endFrame )
		{
			ed->v.frame = startFrame;
			break;
		}
		ed->v.frame--;
		if( ed->v.frame < endFrame )
		{
			*pr_global_ptrs.cycle_wrapped = true;
			ed->v.frame = startFrame;
		}
		break;

	case OP_CWSTATE:	// Cycle weapon state
		ed = PROG_TO_EDICT( *pr_global_ptrs.self );
		ed->v.nextthink = *pr_global_ptrs.time + HX_FRAME_TIME;
		ed->v.think = pr_xfunction - pr_functions;
		*pr_global_ptrs.cycle_wrapped = false;
		startFrame = (int) a->_float;
		endFrame = (int) b->_float;
		if( startFrame <= endFrame )
		{ // Increment
			if( ed->v.weaponframe < startFrame || ed->v.weaponframe > endFrame )
			{
				ed->v.weaponframe = startFrame;
				break;
			}
			ed->v.weaponframe++;
			if( ed->v.weaponframe > endFrame )
			{
				*pr_global_ptrs.cycle_wrapped = true;
				ed->v.weaponframe = startFrame;
			}
			break;
		}
		// Decrement
		if( ed->v.weaponframe > startFrame || ed->v.weaponframe < endFrame )
		{
			ed->v.weaponframe = startFrame;
			break;
		}
		ed->v.weaponframe--;
		if( ed->v.weaponframe < endFrame )
		{
			*pr_global_ptrs.cycle_wrapped = true;
			ed->v.weaponframe = startFrame;
		}
		break;

	case OP_THINKTIME:
		ed = PROG_TO_EDICT(a->edict);
#ifdef PARANOID
		NUM_FOR_EDICT(ed); // Make sure it's in range
#endif
		if ( (ed == (edict_t *) sv.edicts) && (sv.state == ss_active) )
		{
			PR_RunError( "assignment to world entity" );
		}
		ed->v.nextthink = *pr_global_ptrs.time + b->_float;
		break;

	case OP_BITSET:		// f (+) f
		b->_float = (int) b->_float | (int) a->_float;
		break;
	case OP_BITSETP:	// e.f (+) f
		ptr = (eval_t *)((byte *)sv.edicts + b->_int);
		ptr->_float = (int) ptr->_float | (int) a->_float;
		break;
	case OP_BITCLR:		// f (-) f
		b->_float = (int) b->_float & ~((int) a->_float);
		break;
	case OP_BITCLRP:	// e.f (-) f
		ptr = (eval_t *)((byte *) sv.edicts + b->_int);
		ptr->_float = (int) ptr->_float & ~((int) a->_float);
		break;

	case OP_RAND0:
		val = rand()*(1.0/RAND_MAX);//(rand()&0x7fff)/((float)0x7fff);
		G_FLOAT(OFS_RETURN) = val;
		break;
	case OP_RAND1:
		val = rand()*(1.0/RAND_MAX)*a->_float;
		G_FLOAT(OFS_RETURN) = val;
		break;
	case OP_RAND2:
		if ( a->_float < b->_float )
		{
			val = a->_float + (rand()*(1.0/RAND_MAX)*(b->_float - a->_float));
		}
		else
		{
			val = b->_float + (rand()*(1.0/RAND_MAX)*(a->_float - b->_float));
		}
		G_FLOAT(OFS_RETURN) = val;
		break;
	case OP_RANDV0:
		val = rand()*(1.0/RAND_MAX);
		G_FLOAT(OFS_RETURN+0) = val;
		val = rand()*(1.0/RAND_MAX);
		G_FLOAT(OFS_RETURN+1) = val;
		val = rand()*(1.0/RAND_MAX);
		G_FLOAT(OFS_RETURN+2) = val;
		break;
	case OP_RANDV1:
		val = rand()*(1.0/RAND_MAX)*a->vector[0];
		G_FLOAT(OFS_RETURN+0) = val;
		val = rand()*(1.0/RAND_MAX)*a->vector[1];
		G_FLOAT(OFS_RETURN+1) = val;
		val = rand()*(1.0/RAND_MAX)*a->vector[2];
		G_FLOAT(OFS_RETURN+2) = val;
		break;
	case OP_RANDV2:
		for(i = 0; i < 3; i++)
		{
			if ( a->vector[i] < b->vector[i] )
			{
				val = a->vector[i] + (rand()*(1.0/RAND_MAX)*(b->vector[i] - a->vector[i]));
			}
			else
			{
				val = b->vector[i] + (rand()*(1.0/RAND_MAX)*(a->vector[i] - b->vector[i]));
			}
			G_FLOAT(OFS_RETURN+i) = val;
		}
		break;
	case OP_SWITCH_F:
		case_type = OP_SWITCH_F;
		switch_float = a->_float;
		s += st->b-1; // -1 to offset the s++
		break;
	case OP_SWITCH_V:
		PR_RunError("switch v not done yet!");
		break;
	case OP_SWITCH_S:
		PR_RunError("switch s not done yet!");
		break;
	case OP_SWITCH_E:
		PR_RunError("switch e not done yet!");
		break;
	case OP_SWITCH_FNC:
		PR_RunError("switch fnc not done yet!");
		break;

	case OP_CASERANGE:
			if (case_type != OP_SWITCH_F)
				PR_RunError("caserange fucked!");
			if ((switch_float >= a->_float) && (switch_float <= b->_float))
			{
				s += st->c-1; // -1 to offset the s++
			}
		break;
	case OP_CASE:
		switch (case_type)
		{
		case OP_SWITCH_F:
				if(switch_float == a->_float)
				{
					s += st->b-1; // -1 to offset the s++
				}
				break;
		case OP_SWITCH_V:
		case OP_SWITCH_S:
		case OP_SWITCH_E:
		case OP_SWITCH_FNC:
				PR_RunError("case not done yet!");
				break;
		default:
				PR_RunError("fucked case!");

		}
		break;
#endif	// #ifdef HEXEN2_SUPPORT

	default:
		PR_RunError ("Bad opcode %i", st->op);
	}
}

}
