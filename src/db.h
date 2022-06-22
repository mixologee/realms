/**************************************************************************
*  File: db.h                                              Part of tbaMUD *
*  Usage: Header file for database handling.                              *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

/* arbitrary constants used by index_boot() (must be unique) */
#define DB_BOOT_WLD	0
#define DB_BOOT_MOB	1
#define DB_BOOT_OBJ	2
#define DB_BOOT_ZON	3
#define DB_BOOT_SHP	4
#define DB_BOOT_HLP	5
#define DB_BOOT_TRG	6
#define DB_BOOT_QST     7

#if defined(CIRCLE_MACINTOSH)
#define LIB_WORLD	":world:"
#define LIB_TEXT	":text:"
#define LIB_TEXT_HELP   ":text:help:"
#define LIB_MISC	":misc:"
#define LIB_HELPSYS     ":helpsys:"
#define LIB_ETC		":etc:"
#define LIB_PLRTEXT	":plrtext:"
#define LIB_PLROBJS	":plrobjs:"
#define LIB_PLRVARS	":plrvars:"
#define LIB_PLRWARN     ":plrwarn:"
#define LIB_PLRFILES	":plrfiles:"
#define LIB_MLFILES     ":mudmail:"
#define LIB_HOUSE	":house:"
#define LIB_WIZLIST     ":wizlist:"
#define SLASH		":"
#elif defined(CIRCLE_AMIGA) || defined(CIRCLE_UNIX) || defined(CIRCLE_WINDOWS) || defined(CIRCLE_ACORN) || defined(CIRCLE_VMS)
#define LIB_WORLD	"world/"
#define LIB_TEXT	"text/"
#define LIB_TEXT_HELP   "text/help/"
#define LIB_MISC	"misc/"
#define LIB_HELPSYS     "helpsys/"
#define LIB_ETC		"etc/"
#define LIB_PLRTEXT	"plrtext/"
#define LIB_PLROBJS	"plrobjs/"
#define LIB_PLRVARS	"plrvars/"
#define LIB_PLRWARN     "plrwarn/"
#define LIB_HOUSE	"house/"
#define LIB_PLRFILES    "plrfiles/"
#define LIB_MLFILES     "mudmail/"
#define LIB_WIZLIST     "../wizlist/"
#define LIB_STABLES             "stables/"
#define LIB_BUILDINGS   "buildings/"
#define LIB_GOODS               "goods/"
#define LIB_CLANS               "clans/"
#define LIB_SHIPS               "ships/"
#define LIB_AREA                "area/"
#define LIB_DATA                "data/"

#define SLASH		"/"
#else
#error "Unknown path components."
#endif

#define SUF_OBJS	"objs"
#define SUF_TEXT	"text"
#define SUF_MEM	        "mem"
#define SUF_PLR		"plr"
#define SUF_WARN        "warn"
#define SUF_MAIL        "ml"

#if defined(CIRCLE_AMIGA)
#define EXE_FILE "/bin/circle" /* maybe use argv[0] but it's not reliable */
#define KILLSCRIPT_FILE "/.killscript"  /* autorun: shut mud down       */
#define PAUSE_FILE      "/pause"        /* autorun: don't restart mud   */
#elif defined(CIRCLE_MACINTOSH)
#define EXE_FILE "::bin:circle" /* maybe use argv[0] but it's not reliable */
#define FASTBOOT_FILE	"::.fastboot"	/* autorun: boot without sleep	*/
#define KILLSCRIPT_FILE	"::.killscript"	/* autorun: shut mud down	*/
#define PAUSE_FILE	"::pause"	/* autorun: don't restart mud	*/
#else
#define EXE_FILE "bin/circle" /* maybe use argv[0] but it's not reliable */
#define FASTBOOT_FILE   "../.fastboot"  /* autorun: boot without sleep  */
#define KILLSCRIPT_FILE "../.killscript"/* autorun: shut mud down       */
#define PAUSE_FILE      "../pause"      /* autorun: don't restart mud   */
#endif

/* names of various files and directories */
#define INDEX_FILE	"index"		/* index of world files		*/
#define MINDEX_FILE	"index.mini"	/* ... and for mini-mud-mode	*/
#define WLD_PREFIX	LIB_WORLD"wld"SLASH	/* room definitions	*/
#define MOB_PREFIX	LIB_WORLD"mob"SLASH	/* monster prototypes	*/
#define OBJ_PREFIX	LIB_WORLD"obj"SLASH	/* object prototypes	*/
#define ZON_PREFIX	LIB_WORLD"zon"SLASH	/* zon defs & command tables */
#define SHP_PREFIX	LIB_WORLD"shp"SLASH	/* shop definitions	*/
#define TRG_PREFIX	LIB_WORLD"trg"SLASH	/* trigger files	*/
#define HLP_PREFIX      LIB_TEXT"help"SLASH     /* Help files           */
#define QST_PREFIX      LIB_WORLD"qst"SLASH     /* quest files          */
#define BLD_PREFIX              LIB_WORLD"bld"SLASH             /* For building templates                       */
#define SHIP_PREFIX             LIB_WORLD"ship"SLASH    /* For ship templates                           */
#define WLS_PREFIX      LIB_WORLD"wls"SLASH     /* wilderness definitions                       */

#define CREDITS_FILE	LIB_TEXT"credits" /* for the 'credits' command	*/
#define NEWS_FILE	LIB_TEXT"news"	/* for the 'news' command	*/
#define MOTD_FILE	LIB_TEXT"motd"	/* messages of the day / mortal	*/
#define IMOTD_FILE	LIB_TEXT"imotd"	/* messages of the day / immort	*/
#define GREETINGS_FILE	LIB_TEXT"greetings"	/* The opening screen.	*/
#define HELP_PAGE_FILE	LIB_TEXT_HELP"help"	/* for HELP <CR>	*/
#define IHELP_PAGE_FILE LIB_TEXT_HELP"ihelp"    /* for HELP <CR> imms   */
#define INFO_FILE	LIB_TEXT"info"		/* for INFO		*/
#define WIZLIST_FILE    LIB_WIZLIST"wizlist"  
//#define WIZLIST_FILE	LIB_TEXT"wizlist"	/* for WIZLIST		*/
#define IMMLIST_FILE	LIB_TEXT"immlist"	/* for IMMLIST		*/
#define BACKGROUND_FILE	LIB_TEXT"background"/* for the background story	*/
#define POLICIES_FILE	LIB_TEXT"policies"  /* player policies/rules	*/
#define HANDBOOK_FILE	LIB_TEXT"handbook"  /* handbook for new immorts	*/
//#define HELP_FILE       "help.hlp"

#define IDEA_FILE	LIB_MISC"ideas"	   /* for the 'idea'-command	*/
#define TYPO_FILE	LIB_MISC"typos"	   /*         'typo'		*/
#define BUG_FILE	LIB_MISC"bugs"	   /*         'bug'		*/
#define MESS_FILE	LIB_MISC"messages" /* damage messages		*/
#define SOCMESS_FILE	LIB_MISC"socials"  /* messages for social acts	*/
#define SOCMESS_FILE_NEW LIB_MISC"socials.new"  /* messages for social acts with aedit patch*/
#define XNAME_FILE	LIB_MISC"xnames"   /* invalid name substrings	*/

#define CONFIG_FILE	LIB_ETC"config"    /* OasisOLC * GAME CONFIG FL */
#define PLAYER_FILE	LIB_ETC"players"   /* the player database	*/
#define MAIL_FILE	LIB_ETC"plrmail"   /* for the mudmail system	*/
#define MAIL_FILE_TMP	LIB_ETC"plrmail_tmp"   /* for the mudmail system	*/
#define BAN_FILE	LIB_ETC"badsites"  /* for the siteban system	*/
#define HCONTROL_FILE	LIB_ETC"hcontrol"  /* for the house system	*/
#define TIME_FILE	LIB_ETC"time"	   /* for calendar system	*/
#define BANK_FILE       LIB_ETC"bank"

#define CHANGE_LOG_FILE "../changelog"     /* for the changelog         */

#define GOS_LOG         "gossip"	//created - gossip chan
#define SAY_LOG         "say"		//created - say (not mob talk)
#define TELL_LOG        "telepath"      //created - telepaths
#define WIZ_LOG         "wiznet"	//created - wiznet talk
#define EMOTE_LOG       "emote"         //created - gemote/zemote/emote etc
#define QUEST_LOG       "quest"         //created - quest channel
#define GENCOM_LOG      "gencom"        //created - used in gen_com for ask etc.


#define HELP_FILE       LIB_ETC"help"
#define CLAN_FILE       LIB_ETC"clans"     /* the clan file */
#define CLAN_DIP_FILE   LIB_CLANS"clandata"             /* stores clan diplomacy data           */
#define NEWSPAPER_FILE  LIB_TEXT"newspaper"             /* Trade Newspaper                                      */

#define SECT_FILE               LIB_DATA"sectors.data"
#define LIFE_FILE               LIB_DATA"life.data"
#define NEWSPAPER_FILE  LIB_TEXT"newspaper"             /* Trade Newspaper                                      */

/* new bitvector data for use in player_index_element */
#define PINDEX_DELETED		(1 << 0)	/* deleted player	*/
#define PINDEX_NODELETE		(1 << 1)	/* protected player	*/
#define PINDEX_SELFDELETE	(1 << 2)	/* player is selfdeleting*/
#define PINDEX_NOWIZLIST	(1 << 3)	/* Player shouldn't be on wizlist*/

/* public procedures in db.c */
void	boot_db(void);
void	destroy_db(void);
int	create_entry(char *name);
void	zone_update(void);
char	*fread_string(FILE *fl, const char *error);
long	get_id_by_name(const char *name);
char	*get_name_by_id(long id);
void	save_mud_time(struct time_info_data *when);
void	free_text_files(void);
void	free_help_table(void);
void	free_player_index(void);
void    clean_pfiles(void);
void    load_disabled(void); //Slurk
void    save_disabled(void); //Slurk
void    free_disabled(void); //Slurk

zone_rnum real_zone(zone_vnum vnum);
room_rnum real_room(room_vnum vnum);
mob_rnum real_mobile(mob_vnum vnum);
obj_rnum real_object(obj_vnum vnum);

int	load_char(const char *name, struct char_data *ch);
void	save_char(struct char_data *ch);
void	init_char(struct char_data *ch);
struct char_data* create_char(void);
struct char_data *read_mobile(mob_vnum nr, int type);
int	vnum_mobile(char *searchname, struct char_data *ch);
void	clear_char(struct char_data *ch);
void	reset_char(struct char_data *ch);
void	free_char(struct char_data *ch);
void	save_player_index(void);
long    get_ptable_by_name(const char *name);
void    remove_player(int pfilepos);

struct obj_data *create_obj(void);
void	clear_object(struct obj_data *obj);
void	free_obj(struct obj_data *obj);
struct obj_data *read_object(obj_vnum nr, int type);
int	vnum_object(char *searchname, struct char_data *ch);
int     vnum_room(char *, struct char_data *);
int     vnum_trig(char *, struct char_data *);

char    *fread_string_nospace( FILE *fp );
char    *fread_line( FILE *fp );
char    *fread_word( FILE *fp );
char    fread_letter( FILE *fp );
int             fread_number( FILE *fp );
void    fread_to_eol( FILE *fp );

#define REAL 0
#define VIRTUAL 1

/* structure for the reset commands */
struct reset_com {
   char	command;   /* current command                      */

   bool if_flag;	/* if TRUE: exe only if preceding exe'd */
   int	arg1;		/*                                      */
   int	arg2;		/* Arguments to the command             */
   int	arg3;		/*                                      */
   int line;		/* line number this command appears on  */
   char *sarg1;		/* string argument                      */
   char *sarg2;		/* string argument                      */

   /* Commands: 
    *  'M': Read a mobile 
    *  'O': Read an object 
    *  'G': Give obj to mob
    *  'P': Put obj in obj
    *  'G': Obj to char
    *  'E': Obj to char equip
    *  'D': Set state of door
    *  'T': Trigger command
    *  'V': Assign a variable */
};


struct zone_wild_data
{
  COORD_DATA    z_start;
  COORD_DATA    z_end;
  char                  *survey;                                                /* description when see from wilderness */
  char                  *flyto;                                                 /* keyword for targeting with dragons   */
  int                   dist;                                                   /* how far is visible?                                  */
};

/* zone definition structure. for the 'zone-table'   */
struct zone_data {
   char	*name;		    /* name of this zone                  */
   char *builders;          /* namelist of builders allowed to    */
                            /* modify this zone.		  */
   int	lifespan;           /* how long between resets (minutes)  */
   int	age;                /* current age of this zone (minutes) */
   room_vnum bot;           /* starting room number for this zone */
   room_vnum top;           /* upper limit for rooms in this zone */

   int zone_flags[ZN_ARRAY_MAX];
   int min_level;
   int max_level;


   char *storyline;         /* story for the zone                 */

   int	reset_mode;         /* conditions for reset (see below)   */
   zone_vnum number;	    /* virtual number of this zone	  */
   struct reset_com *cmd;   /* command table for reset	          */
   int x;
   int y;
 
   char *type;		    /* cpk/npk/lpk                        */
   int lowlev;              /* min level */
   int highlev;             /* max level */

   ZONE_WILD wild;          /* wilderness info for zone           */

   /* Reset mode:
    *   0: Don't reset, and don't update age.
    *   1: Reset if no PC's are located in zone.
    *   2: Just reset. */
};

/* for queueing zones for update   */
struct reset_q_element {
   zone_rnum zone_to_reset;            /* ref to zone_data */
   struct reset_q_element *next;
};

/* structure for the update queue     */
struct reset_q_type {
   struct reset_q_element *head;
   struct reset_q_element *tail;
};

/* Added level, flags, and last, primarily for pfile autocleaning.  You can also
 * use them to keep online statistics, and add race, class, etc if you like. */
struct player_index_element {
   char	*name;
   long id;
   int level;
   int flags;
   time_t last;
};


struct help_index_element {
   int index;      /*Future Use */
   char *keywords;   /*Keyword Place holder and sorter */
   char *entry;      /*Entries for help files with Keywords at very top*/
   char *akeys;      /*Associated keywords*/
   int min_level;    /*Min Level to read help entry*/
   int category;
   struct help_index_element *next;
};

/* don't change these */
#define BAN_NOT 	0
#define BAN_NEW 	1
#define BAN_SELECT	2
#define BAN_ALL		3

#define BANNED_SITE_LENGTH    50
struct ban_list_element {
   char	site[BANNED_SITE_LENGTH+1];
   int	type;
   time_t date;
   char	name[MAX_NAME_LENGTH+1];
   struct ban_list_element *next;
};

/* for the "buffered" rent and house object loading */
struct obj_save_data_t {
  struct obj_data *obj;
  int locate;
  struct obj_save_data_t *next;
};
typedef struct obj_save_data_t obj_save_data;


/* global buffering system */

#ifndef __DB_C__
extern struct config_data config_info;

extern struct room_data *world;
extern room_rnum top_of_world;
extern ROOM_AFFECT *raff_list;

extern struct zone_data *zone_table;
extern zone_rnum top_of_zone_table;

extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct player_special_data dummy_mob;

extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern mob_rnum top_of_mobt;

extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern struct obj_data *obj_proto;
extern obj_rnum top_of_objt;

extern struct social_messg *soc_mess_list;
extern int top_of_socialt;

extern struct shop_data *shop_index;
extern int top_shop;

extern struct index_data **trig_index;
extern struct trig_data *trigger_list;
extern int top_of_trigt;
extern long max_mob_id;
extern long max_obj_id;
extern int dg_owner_purged;
extern int Sunlight;
extern int MoonPhase;       

char    buf[MAX_STRING_LENGTH];
char    buf1[MAX_STRING_LENGTH];
char    buf2[MAX_STRING_LENGTH];
char    buf3[MAX_STRING_LENGTH];
char    arg[MAX_STRING_LENGTH];

#endif /* __DB_C__ */

#define DISABLED_FILE     "disabled.cmds"        //disabled cmds file
#define END_MARKER        "END"                 // for Load and Save functions

typedef struct disabled_data DISABLED_DATA;
extern DISABLED_DATA *disabled_first;

struct disabled_data {
  DISABLED_DATA *next;                          //pointer to next
  struct command_info const *command;           //pointer to cmd struct
  char *disabled_by;                            //name of god that disabled the cmd
  sh_int level;                                 //god level
  int subcmd;                                   //the sub command, if any used
};
   
/*   Lyo Stuff  */
extern TERRAIN_DATA             *terrain_type[MAX_SECT_TYPES];

extern int                              Sunlight;
extern int                              MoonPhase;
 
