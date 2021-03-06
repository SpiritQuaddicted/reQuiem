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

// this file is shared by quake and qcc

typedef int	func_t;
typedef int	string_t;

typedef enum {ev_void, ev_string, ev_float, ev_vector, ev_entity, ev_field, ev_function, ev_pointer} etype_t;


#define	OFS_NULL	0
#define	OFS_RETURN	1
#define	OFS_PARM0	4		// leave 3 ofs for each parm to hold vectors
#define	OFS_PARM1	7
#define	OFS_PARM2	10
#define	OFS_PARM3	13
#define	OFS_PARM4	16
#define	OFS_PARM5	19
#define	OFS_PARM6	22
#define	OFS_PARM7	25
#define	RESERVED_OFS	28


enum {
	OP_DONE,
	OP_MUL_F,
	OP_MUL_V,
	OP_MUL_FV,
	OP_MUL_VF,
	OP_DIV_F,
	OP_ADD_F,
	OP_ADD_V,
	OP_SUB_F,
	OP_SUB_V,
	
	OP_EQ_F,		// 10
	OP_EQ_V,
	OP_EQ_S,
	OP_EQ_E,
	OP_EQ_FNC,
	
	OP_NE_F,		// 15
	OP_NE_V,
	OP_NE_S,
	OP_NE_E,
	OP_NE_FNC,
	
	OP_LE,			// 20
	OP_GE,
	OP_LT,
	OP_GT,

	OP_LOAD_F,		// 24
	OP_LOAD_V,
	OP_LOAD_S,
	OP_LOAD_ENT,
	OP_LOAD_FLD,
	OP_LOAD_FNC,

	OP_ADDRESS,		// 30

	OP_STORE_F,		// 31
	OP_STORE_V,
	OP_STORE_S,
	OP_STORE_ENT,
	OP_STORE_FLD,
	OP_STORE_FNC,

	OP_STOREP_F,	// 37
	OP_STOREP_V,
	OP_STOREP_S,
	OP_STOREP_ENT,
	OP_STOREP_FLD,
	OP_STOREP_FNC,

	OP_RETURN,		// 43
	OP_NOT_F,
	OP_NOT_V,
	OP_NOT_S,
	OP_NOT_ENT,
	OP_NOT_FNC,
	OP_IF,			// 49
	OP_IFNOT,
	OP_CALL0,		// 51
	OP_CALL1,
	OP_CALL2,
	OP_CALL3,
	OP_CALL4,
	OP_CALL5,
	OP_CALL6,
	OP_CALL7,
	OP_CALL8,
	OP_STATE,		// 60
	OP_GOTO,
	OP_AND,
	OP_OR,
	
	OP_BITAND,		// 64
	OP_BITOR

#ifdef HEXEN2_SUPPORT
	,OP_MULSTORE_F,		// 66
	OP_MULSTORE_V,
	OP_MULSTOREP_F,
	OP_MULSTOREP_V,

	OP_DIVSTORE_F,		// 70
	OP_DIVSTOREP_F,

	OP_ADDSTORE_F,		// 72
	OP_ADDSTORE_V,
	OP_ADDSTOREP_F,
	OP_ADDSTOREP_V,

	OP_SUBSTORE_F,		// 76
	OP_SUBSTORE_V,
	OP_SUBSTOREP_F,
	OP_SUBSTOREP_V,

	OP_FETCH_GBL_F,		// 80
	OP_FETCH_GBL_V,
	OP_FETCH_GBL_S,
	OP_FETCH_GBL_E,
	OP_FETCH_GBL_FNC,

	OP_CSTATE,			// 85
	OP_CWSTATE,

	OP_THINKTIME,		// 87

	OP_BITSET,			// 88
	OP_BITSETP,
	OP_BITCLR,
	OP_BITCLRP,

	OP_RAND0,			// 92
	OP_RAND1,
	OP_RAND2,
	OP_RANDV0,
	OP_RANDV1,
	OP_RANDV2,

	OP_SWITCH_F,		// 98
	OP_SWITCH_V,
	OP_SWITCH_S,
	OP_SWITCH_E,
	OP_SWITCH_FNC,

	OP_CASE,			// 103
	OP_CASERANGE
#endif
};


typedef struct statement_s
{
	unsigned short	op;
	short		a, b, c;
} dstatement_t;

typedef struct
{
	unsigned short	type;		// if DEF_SAVEGLOBGAL bit is set
					// the variable needs to be saved in savegames
	unsigned short	ofs;
// JDH: string offset is converted to string ptr at loadtime
	//int		s_name;
	char		*s_name;
} ddef_t;
#define	DEF_SAVEGLOBAL	(1<<15)

#define	MAX_PARMS	8
typedef struct
{
	int		first_statement;	// negative numbers are builtins
	int		parm_start;
	int		locals;			// total ints of parms + locals
	
	int		profile;		// runtime
	
// JDH: string offsets converted to string ptrs at loadtime
//	int		s_name;
//	int		s_file;			// source file defined in
	char	*s_name;
	char	*s_file;			// source file defined in
	
	int		numparms;
	byte		parm_size[MAX_PARMS];
} dfunction_t;

#define	PROG_VERSION	6
typedef struct
{
	int		version;
	int		crc;			// check of header file
	
	int		ofs_statements;
	int		numstatements;		// statement 0 is an error

	int		ofs_globaldefs;
	int		numglobaldefs;
	
	int		ofs_fielddefs;
	int		numfielddefs;
	
	int		ofs_functions;
	int		numfunctions;		// function 0 is an empty
	
	int		ofs_strings;
	int		numstrings;		// first string is a null string

	int		ofs_globals;
	int		numglobals;
	
	int		entityfields;
} dprograms_t;
