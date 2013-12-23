
#define MITEMHEIGHT 10

// arbitrary value for size of items array:
#define M_MAX_ITEMS 20

// menu item flags:
#define M_ITEM_WHITE    0x0001
#define M_ITEM_BIG      0x0002
//#define M_ITEM_CENTER   0x0010
#define M_ITEM_SLIDER   0x0400
#define M_ITEM_DISABLED 0x8000

typedef struct
{
	const char *text;
	void (*enterproc)(cmd_source_t src);
	cvar_t *cvar;
	int flags;
}
menu_item_t;


// menu flags:
#define M_ALIGN_RIGHT    0x0001
#define M_NO_QUAKELOGO   0x0010
#define M_NO_CVARINFO    0x0020
#define M_NO_CURSOR      0x0040
#define M_VAR_CHANGED    0x8000

/*typedef struct menu_s
{
	char *title;
#ifdef HEXEN2_SUPPORT
	char *lmp;
#endif
	void (*openproc)(void);
	void (*drawproc)(void);
	qboolean (*keyproc)(int);
	qboolean (*closeproc)(void);
	struct menu_s *parentmenu;
	int flags;
	int cursor;
	int first_item;		// when there is more than 1 screenful of items
	int num_items;
	menu_item_t items[M_MAX_ITEMS];
} menu_t;
*/

#ifdef HEXEN2_SUPPORT
  #define M_TITLE_STRINGS \
	const char *title;	\
	const char *lmp;
#else
  #define M_TITLE_STRINGS \
	const char *title;
#endif


#define MENU_T(maxitems) \
{	\
	M_TITLE_STRINGS	\
	void (*openproc)(cmd_source_t src);	\
	void (*drawproc)(void);	\
	qboolean (*keyproc)(int, qboolean);	\
	qboolean (*closeproc)(void);	\
	struct menu_s *parentmenu;	\
	int flags;	\
	int cursor;	\
	int first_item;		/* when there is more than 1 screenful of items*/	\
	int num_items;	\
	menu_item_t items[maxitems];	\
}

typedef struct menu_s MENU_T(M_MAX_ITEMS) menu_t;

typedef struct
{
	const char	*name;
	const char	*description;
} level_t;


typedef struct
{
	const char	*description;
	int		firstLevel;
	int		levels;
} episode_t;


#ifndef M_NO_LEVEL_LIST

level_t	levels[] =
{
	{"start", "Entrance"},	// 0

	{"e1m1", "Slipgate Complex"},				// 1
	{"e1m2", "Castle of the Damned"},
	{"e1m3", "The Necropolis"},
	{"e1m4", "The Grisly Grotto"},
	{"e1m5", "Gloom Keep"},
	{"e1m6", "The Door To Chthon"},
	{"e1m7", "The House of Chthon"},
	{"e1m8", "Ziggurat Vertigo"},

	{"e2m1", "The Installation"},				// 9
	{"e2m2", "Ogre Citadel"},
	{"e2m3", "Crypt of Decay"},
	{"e2m4", "The Ebon Fortress"},
	{"e2m5", "The Wizard's Manse"},
	{"e2m6", "The Dismal Oubliette"},
	{"e2m7", "Underearth"},

	{"e3m1", "Termination Central"},			// 16
	{"e3m2", "The Vaults of Zin"},
	{"e3m3", "The Tomb of Terror"},
	{"e3m4", "Satan's Dark Delight"},
	{"e3m5", "Wind Tunnels"},
	{"e3m6", "Chambers of Torment"},
	{"e3m7", "The Haunted Halls"},

	{"e4m1", "The Sewage System"},				// 23
	{"e4m2", "The Tower of Despair"},
	{"e4m3", "The Elder God Shrine"},
	{"e4m4", "The Palace of Hate"},
	{"e4m5", "Hell's Atrium"},
	{"e4m6", "The Pain Maze"},
	{"e4m7", "Azure Agony"},
	{"e4m8", "The Nameless City"},

	{"end", "Shub-Niggurath's Pit"},			// 31

	{"dm1", "Place of Two Deaths"},				// 32
	{"dm2", "Claustrophobopolis"},
	{"dm3", "The Abandoned Base"},
	{"dm4", "The Bad Place"},
	{"dm5", "The Cistern"},
	{"dm6", "The Dark Zone"},

	{"agenda", "Hidden Agenda"},				// 38
	{"efdm5", "Vortex"},
	{"efdm6", "Gunmetal"},
	{"efdm7", "Biohazard"},
	{"efdm8", "Cryptosporidium"},
	{"efdm12", "Death by the Dozen"},
	{"spinev2", "Spine ver 2"}					// 44
};

//MED 01/06/97 added hipnotic levels
level_t	hipnoticlevels[] =
{
	{"start", "Command HQ"},	// 0

	{"hip1m1", "The Pumping Station"},		// 1
	{"hip1m2", "Storage Facility"},
	{"hip1m3", "The Lost Mine"},
	{"hip1m4", "Research Facility"},
	{"hip1m5", "Military Complex"},

	{"hip2m1", "Ancient Realms"},			// 6
	{"hip2m2", "The Black Cathedral"},
	{"hip2m3", "The Catacombs"},
	{"hip2m4", "The Crypt"},
	{"hip2m5", "Mortum's Keep"},
	{"hip2m6", "The Gremlin's Domain"},

	{"hip3m1", "Tur Torment"},			// 12
	{"hip3m2", "Pandemonium"},
	{"hip3m3", "Limbo"},
	{"hip3m4", "The Gauntlet"},

	{"hipend", "Armagon's Lair"},			// 16

	{"hipdm1", "The Edge of Oblivion"}		// 17
};

//PGM 01/07/97 added rogue levels
//PGM 03/02/97 added dmatch level
level_t	roguelevels[] =
{
	{"start", "Split Decision"},
	{"r1m1", "Deviant's Domain"},
	{"r1m2", "Dread Portal"},
	{"r1m3", "Judgement Call"},
	{"r1m4", "Cave of Death"},
	{"r1m5", "Towers of Wrath"},
	{"r1m6", "Temple of Pain"},
	{"r1m7", "Tomb of the Overlord"},
	{"r2m1", "Tempus Fugit"},
	{"r2m2", "Elemental Fury I"},
	{"r2m3", "Elemental Fury II"},
	{"r2m4", "Curse of Osiris"},
	{"r2m5", "Wizard's Keep"},
	{"r2m6", "Blood Sacrifice"},
	{"r2m7", "Last Bastion"},
	{"r2m8", "Source of Evil"},
	{"ctf1", "Division of Change"}
};

#define NUM_NEHDEMOS    34

level_t neh_demos[NUM_NEHDEMOS] =
{
	{"intro",    "Prologue"},
	{"genf",     "The Beginning"},
	{"genlab",   "A Doomed Project"},
	{"nehcre",   "The New Recruits"},
	{"maxneh",   "Breakthrough"},
	{"maxchar",  "Renewal and Duty"},
	{"crisis",   "Worlds Collide"},
	{"postcris", "Darkening Skies"},
	{"hearing",  "The Hearing"},
	{"getjack",  "On a Mexican Radio"},
	{"prelude",  "Honor and Justice"},
	{"abase",    "A Message Sent"},
	{"effect",   "The Other Side"},
	{"uhoh",     "Missing in Action"},
	{"prepare",  "The Response"},
	{"vision",   "Farsighted Eyes"},
	{"maxturns", "Enter the Immortal"},
	{"backlot",  "Separate Ways"},
	{"maxside",  "The Ancient Runes"},
	{"counter",  "The New Initiative"},
	{"warprep",  "Ghosts to the World"},
	{"counter1", "A Fate Worse Than Death"},
	{"counter2", "Friendly Fire"},
	{"counter3", "Minor Setback"},
	{"madmax",   "Scores to Settle"},
	{"quake",    "One Man"},
	{"cthmm",    "Shattered Masks"},
	{"shades",   "Deal with the Dead"},
	{"gophil",   "An Unlikely Hero"},
	{"cstrike",  "War in Hell"},
	{"shubset",  "The Conspiracy"},
	{"shubdie",  "Even Death May Die"},
	{"newranks", "An Empty Throne"},
	{"seal",     "The Seal is Broken"}
};

episode_t episodes[] =
{
	{"Welcome to Quake", 0, 1},
	{"Doomed Dimension", 1, 8},
	{"Realm of Black Magic", 9, 7},
	{"Netherworld", 16, 7},
	{"The Elder World", 23, 8},
	{"Final Level", 31, 1},
	{"Deathmatch Arena", 32, 13}
};

//MED 01/06/97  added hipnotic episodes
episode_t hipnoticepisodes[] =
{
	{"Scourge of Armagon", 0, 1},
	{"Fortress of the Dead", 1, 5},
	{"Dominion of Darkness", 6, 6},
	{"The Rift", 12, 4},
	{"Final Level", 16, 1},
	{"Deathmatch Arena", 17, 1}
};

//PGM 01/07/97 added rogue episodes
//PGM 03/02/97 added dmatch episode
episode_t rogueepisodes[] =
{
	{"Introduction", 0, 1},
	{"Hell's Fortress", 1, 7},
	{"Corridors of Time", 8, 8},
	{"Deathmatch Arena", 16, 1}
};

#endif

#ifdef HEXEN2_SUPPORT
void M_PrintBig_H2 (int cx, int cy, const char *str);
void M_DrawTitle_H2 (const char *name);
void M_Menu_Class_f (cmd_source_t src);
#endif
