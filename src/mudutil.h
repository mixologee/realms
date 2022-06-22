//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://
//:   File        : mudutil.h                       Part of The Outlands   ://
//:   Date        : April 28, 2007                                         ://
//:   Usage       : Custom utilities for The Outlands code base.           ://
//:   Programmer  : Aenarion (Michael T. Kohler)                           ://
//:                                                                        ://
//:   COPYRIGHT (C) The Outlands and Michael T. Kohler                     ://
//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://
//:                             MODIFICATIONS                              ://
//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://
//:   Programmer  :                                                        ://
//:   Date        :                                                        ://
//:   Modification:                                                        ://
//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://

/* Custom Colors */
extern char plrdesc[3];
extern char plreq[3];
extern char rmplr[3];
extern char rmmob[3];
extern char rmobj[3];
extern char rmname[3];

/* Custom Prototyped Functions */
void	nmudlog(int type, int level, byte file, const char *targ, ...);
void	sNpcStats(chData *npc);
void	DoPrfSet(chData *ch, int flag, const char *name);
void	DoLogSet(chData *ch, int flag, int type);
void	DoMultiSet(chData *ch, chData *victim, int flag, const char *name);
void	LogChannel(const char *Channel, const char *Text, ...);
void	LoadPotions(obData *corpse);
void	LoadAges(obData *corpse);
void	LoadOrbs(obData *corpse);
void	LoadScrolls(obData *corpse);
void	LoadRegen(obData *corpse);
void	LoadTrain(obData *corpse);
void	CorpseObjects(obData *corpse);
void    RawDeath(chData *kille);
void	StatObject(chData *ch, obData *object);
void	StatRoom(chData *ch);
void	StatCharacter(chData *ch, chData *tch);
void	update_changes(void);
char	*DisplayMultiFlags(void);
char 	*to_lower(const char *arg);
char 	*to_upper(const char *arg);
char	*strip_colors(const char *arg);
char	*stripcr(char *dest, const char *src);
char	*title_female(int chclass, int level);
char	*title_male(int chclass, int level);
int	create_dir(int room, int dir); 
chData *get_char_online_offline(char *name, int &loaded);
room_rnum next_free_room(zone_rnum zone);

/* Object Tweak Class */
class ObjTweak
{
public:
  void Tweak(obData *obj);
};

/* Searching Alghorithm Templates */
template <typename T>
extern int BinarySearch(T Key, T *Array, int Size);
template <typename T>
extern int LinearSearch(T Key, T *Array, int Size);

/* Player Titles */
struct title_t {
  const char *male;
  const char *female;
};

/* Custom Colors */
struct color_struct {
  const char *cmd;
  unsigned char level;
  char color;
  char def;
};

/* Custom Sorting used in Wizlist and Building */
struct sort_struct {
  int sort_pos;
  byte is_build;
  byte no_view;
};

struct raff_node {
  room_rnum room;	/* location in the world[] array of the room	*/
  int timer;		/* how many ticks this affection lasts 		*/
  long affection;	/* which affection does this room have 		*/
  int spell;		/* the spell number 				*/
  struct raff_node *next; /* link to the next node 			*/
};

typedef struct buffer_type
{
  char   * data;        /* The data                      */
  int      len;         /* The current len of the buffer */
  int      size;        /* The allocated size of data    */
} BUFFER;

int 	bprintf(BUFFER *buffer, char *fmt, ...);
void    __buffer_strcat(BUFFER *buffer, const char *text);

/* String Macros */
#define buffer_strcat(buffer, text)	__buffer_strcat(buffer, text)

#define WILD(room)			(world[room].custom.wild_type)

/* On/Off */
#define OFF		0
#define ON		1

/* Types for nmudlog */
#define INFO_LOG	0
#define MISC_LOG        1
#define GOD_LOG         2
#define SYS_LOG         3
#define OLC_LOG         4
#define DEATH_LOG       5
#define ZONE_LOG        6
#define CONN_LOG        7
#define SUGG_LOG        8
#define GIVE_LOG	9

/* PC Log Flags */
#define LOG_INFO       	(1 << 0)
#define LOG_MISC       	(1 << 1)
#define LOG_GOD        	(1 << 2)
#define LOG_SYS        	(1 << 3)
#define LOG_OLC        	(1 << 4)
#define LOG_DEATH      	(1 << 5)
#define LOG_ZONE       	(1 << 6)
#define LOG_CONN       	(1 << 7)
#define LOG_SUGG       	(1 << 8)
#define MAX_LOG_FLAG   	(1 << 9)

#define LOAD_MOB	0
#define LOAD_OBJ	1

/* Death Flags */
#define DTH_FRESH       (1 << 0)        /* Player has just died                 */
#define DTH_DONE        (1 << 1)        /* Player has finish death process      */
#define DTH_UNDEAD      (1 << 2)        /* Player is now undead                 */

/* Mute flags */
#define MUTE_SHOUT      (1 << 0)        /* Cannot use shout             */
#define MUTE_SAY        (1 << 1)        /* Cannot use say               */
#define MUTE_TELEPATH   (1 << 2)        /* Cannot use telepath          */
#define MUTE_GOSSIP     (1 << 3)        /* Cannot use gossip            */
#define MUTE_IMMORTAL   (1 << 4)        /* Cannot use immortal          */
#define MUTE_SONG       (1 << 5)        /* Cannot use song              */
#define MAX_MUTE        (1 << 6)        /* Shuts off all channels       */

/* More room flags */
#define ROOM2_RAVENS	(1 << 0)        /* Player is attacked by ravens */
#define ROOM2_ARROWS	(1 << 1)        /* Player is attacked by arrows */
#define ROOM2_CREROOM	(1 << 2)        /* LVL_CRE and above may enter  */

/* Room affects */
#define RAFF_SHIELDROOM	(1 << 0)	/* Shieldroom room affect for spell	*/

/* Object and NPC approval flags */
#define APPROVED	(1 << 0)	/* Object/Npc has been approved by the World Design Team	*/
#define NOAPPROVE	(1 << 1)	/* Object/Npc has not been approved by the World Design Team	*/

/* do_godchan */
#define SCMD_COSMIC             0
#define SCMD_CREATOR            1
#define SCMD_GODTALK            2
#define SCMD_SUPREMITY          3
#define SCMD_WIZNET             4
#define SCMD_IMMORTAL           5

/* do_readnews */
#define SCMD_READ_NEWS		0
#define SCMD_READ_GOD_NEWS	1

/* do_playercomm */
#define PC_CHAN_SAY             0
#define PC_CHAN_GOSSIP          1
#define PC_CHAN_SHOUT           2
#define PC_CHAN_SONG            3
#define PC_CHAN_QUEST           4
#define PC_CHAN_TALK            5
#define PC_CHAN_WHISPER         6
#define PC_CHAN_ASK             7

/* */
#define SCMD_TODO		0

/* God flags */
#define GFL_GODMODE		1
