/* ************************************************************************
*   File: structs.h                                                       *
*  Usage: header file for central structures and contstants               *
*                                                                         *
************************************************************************ */


/*
 * Intended use of this macro is to allow external packages to work with
 * a variety of CircleMUD versions without modifications.  For instance,
 * an IS_CORPSE() macro was introduced in pl13.  Any future code add-ons
 * could take into account the CircleMUD version and supply their own
 * definition for the macro if used on an older version of CircleMUD.
 * You are supposed to compare this with the macro CIRCLEMUD_VERSION()
 * in utils.h.  See there for usage.
 */
#define _TBAMUD	0x030550 /* Major/Minor/Patchlevel - MMmmPP */

/*
 * If you want equipment to be automatically equipped to the same place
 * it was when players rented, set the define below to 1.  Please note
 * that this will require erasing or converting all of your rent files.
 * And of course, you have to recompile everything.  We need this feature
 * for CircleMUD to be complete but we refuse to break binary file
 * compatibility.
 */
#define USE_AUTOEQ	1	/* TRUE/FALSE aren't defined yet. */


#define NUM_OF_COLORS	24
/* preamble *************************************************************/

/*
 * As of bpl20, it should be safe to use unsigned data types for the
 * various virtual and real number data types.  There really isn't a
 * reason to use signed anymore so use the unsigned types and get
 * 65,535 objects instead of 32,768.
 *
 * NOTE: This will likely be unconditionally unsigned later.
 */
#define CIRCLE_UNSIGNED_INDEX	0	/* 0 = signed, 1 = unsigned */

#if CIRCLE_UNSIGNED_INDEX
# define IDXTYPE	int
# define NOWHERE	((IDXTYPE)~0)
# define NOTHING	((IDXTYPE)~0)
# define NOBODY		((IDXTYPE)~0)
# define NOFLAG         ((IDXTYPE)~0)
# define IDXTYPE_MAX LONG_MAX /* Used for compatibility checks. */ 
# define IDXTYPE_MIN 0         /* Used for compatibility checks. */ 
#else
# define IDXTYPE	int
# define IDXTYPE_MAX LONG_MAX  /* Used for compatibility checks. */ 
# define IDXTYPE_MIN LONG_MIN  /* Used for compatibility checks. */ 
# define NOWHERE	((IDXTYPE)-1)	/* nil reference for rooms	*/
# define NOTHING	((IDXTYPE)-1)	/* nil reference for objects	*/
# define NOBODY		((IDXTYPE)-1)	/* nil reference for mobiles	*/
# define NOFLAG         ((IDXTYPE)-1)   /* nil reference for flags      */
#endif

#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)

#define EVENTFUNC(name) \
        long (name)(void *event_obj)

#define C_FUNC(name) \
        void (name) (DESCRIPTOR_DATA *d, char *arg, void *info)

/* room-related defines *************************************************/
#define AFTERLIFE_MIN   300
#define AFTERLIFE_MAX   310
#define AFTERLIFE_MIN2  400
#define AFTERLIFE_MAX2  410
#define AFTERLIFE_MIN3  500
#define AFTERLIFE_MAX3  520
 
/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NORTHEAST                               6
#define SOUTHEAST                               7
#define SOUTHWEST                               8
#define NORTHWEST                               9

#define NUM_DIRECTIONS 10 

/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK		 0   /* Dark			*/
#define ROOM_DEATH		 1   /* Death trap		*/
#define ROOM_NOMOB		 2   /* MOBs not allowed		*/
#define ROOM_INDOORS		 3   /* Indoors			*/
#define ROOM_PEACEFUL		 4   /* Violence not allowed	*/
#define ROOM_SOUNDPROOF		 5   /* Shouts, gossip blocked	*/
#define ROOM_NOTRACK		 6   /* Track won't go through	*/
#define ROOM_NOMAGIC		 7   /* Magic not allowed		*/
#define ROOM_TUNNEL		 8   /* room for only 1 pers	*/
#define ROOM_PRIVATE		 9   /* Can't teleport in		*/
#define ROOM_GODROOM		 10  /* LVL_GOD+ only allowed	*/
#define ROOM_HOUSE		 11  /* (R) Room is a house	*/
#define ROOM_HOUSE_CRASH	 12  /* (R) House needs saving	*/
#define ROOM_ATRIUM		 13  /* (R) The door to a house	*/
#define ROOM_OLC		 14  /* (R) Modifyable/!compress	*/
#define ROOM_BFS_MARK		 15  /* (R) breath-first srch mrk	*/
#define ROOM_NPK                 16  // NPK ROOM
#define ROOM_CPK                 17  // CPK ROOM
#define ROOM_SALTWATER_FISH      18  /* Player can fish here     */
#define ROOM_FRESHWATER_FISH     19  /* Player can fish here too */
#define ROOM_TREASUREWATER_FISH  20
#define ROOM_ARENA               21  /*  An Arena Room Fight no die*/
#define ROOM_NOVIEW              22
#define ROOM_WILDERNESS          23
#define ROOM_WORLDMAP            24   /* World-map style maps here */
#define ROOM_TRIPWIRE            25
#define ROOM_RECALL              26
#define ROOM_MINE                27
#define ROOM_SMITH               28
#define ROOM_FOUNTAIN            29
#define ROOM_REGEN               30
#define ROOM_WEATHERPROOF        31
#define ROOM_OUTDOORS            32
#define ROOM_CRYSTALGATE         33   /* travel room, nomagic, nospeak etc */
#define ROOM_FREIGHTOK           34
#define ROOM_ALTAR               35
#define ROOM_BLACKMARKET         36
#define ROOM_DONATION            37
#define ROOM_NOMOB_BARRIER       38
#define ROOM_EXTRAMOVE           39
#define ROOM_MAIL 		 40
#define ROOM_WILD_STATIC         41   /* q (R) static wilderness room                 */
#define ROOM_WILD_REMOVE         42           /* r (R) wild room to be deleted                */
#define ROOM_SAVEROOM            43              /* s room is a loadroom                                 */
#define ROOM_BUILDING_WORKS      44               /* t (R) building works in progress...  */
#define ROOM_NO_RECALL           45
#define ROOM_NO_TELEPORT	 46
#define MAX_ROOM_FLAGS           47


/*Zone info: Used in zone_data.zone_flags */
/*NEW ZEDIT FUCNTIONALITY*/
#define ZONE_CLOSED     0
#define ZONE_NOIMMORT   1 
#define ZONE_QUEST      2
#define ZONE_GRID       3
#define ZONE_NOBUILD    4
#define ZONE_NOASTRAL   5
#define ZONE_WILDERNESS 6
#define NUM_ZONE_FLAGS  7

/* Room Affections  in room_affs */
/*#define RAFF_NULL		  0 *//* Dont use this		       */
#define RAFF_SHELTER		  1 /* Wilderness quit point           */
#define RAFF_TRACKS		  2 /* PC/NPC tracks left to follow    */
#define RAFF_NPC_BARRIER          3 /* Room barrier, stops mobs        */
#define RAFF_PC_BARRIER           4 /* Room barrier, stops players     */
#define RAFF_GAS		  5 /* Room temp gas flag - spells?    */
#define RAFF_ACID		  6 /* Room temp acid flag - spells?   */
#define RAFF_FIRE		  7 /* Room temp fire flag - spells?   */
#define RAFF_WIND		  8 /* Room temp wind flag - spells?   */
#define RAFF_FLOOD		  9 /* Room temp flood flag - spells?  */
#define RAFF_BLOOD		 10 /* Blood in room		       */


/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR		(1 << 0)   /* Exit is a door		*/
#define EX_CLOSED		(1 << 1)   /* The door is closed	*/
#define EX_LOCKED		(1 << 2)   /* The door is locked	*/
#define EX_PICKPROOF		(1 << 3)   /* Lock can't be picked	*/
#define EX_HIDDEN               (1 << 4)   /* hidden exits wooo */
#define EX_NOETH                (1 << 5)

/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0		   /* Indoors			*/
#define SECT_CITY            1		   /* In a city			*/
#define SECT_FIELD           2		   /* In a field		*/
#define SECT_FOREST          3		   /* In a forest		*/
#define SECT_HILLS           4		   /* In the hills		*/
#define SECT_MOUNTAIN        5		   /* On a mountain		*/
#define SECT_WATER_SWIM      6		   /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7		   /* Water - need a boat	*/
#define SECT_FLYING	     8		   /* Wheee!			*/
#define SECT_UNDERWATER	     9		   /* Underwater		*/
#define SECT_ROAD           10
#define SECT_ROCK_MOUNTAIN  11
#define SECT_SNOW_MOUNTAIN  12
#define SECT_RUINS          13
#define SECT_JUNGLE         14
#define SECT_SWAMP          15
#define SECT_LAVA           16
#define SECT_ENTRANCE       17
#define SECT_FARM           18
#define SECT_VOLCANIC       19
#define SECT_AIR            20
#define SECT_STREET         21
#define SECT_BEACH          22
#define SECT_HOUSE          23
#define SECT_SHOP           24
#define SECT_BLANK          25
#define SECT_DESERT         26
#define SECT_MINE           27
#define SECT_ARCTIC         28
#define SECT_ARROYO 	    29
#define SECT_AVIARY 	    30
#define SECT_BAR            31
#define SECT_BARRACKS       32
#define SECT_BASTION        33
#define SECT_BEDROCK        34
#define SECT_CALDERA        35
#define SECT_CANYON         36
#define SECT_CASTLE         37
#define SECT_CATHEDRAL      38
#define SECT_CAVE           39
#define SECT_CAVERN         40
#define SECT_CEMERTARY      41
#define SECT_CHURCH         42
#define SECT_CITADEL        43
#define SECT_CLIFF          44
#define SECT_CONIFEROUS_FOREST      45
#define SECT_DECIDUOUS_FOREST      46
#define SECT_DELTA          47
#define SECT_DUNES          48
#define SECT_ENCAMPMENT     49
#define SECT_ESTUARY        50
#define SECT_FEN            51
#define SECT_FOOTHILLS      52
#define SECT_FOOTPATH       53
#define SECT_FORTRESS       54
#define SECT_GAME_TRAIL     55
#define SECT_GARDEN         56
#define SECT_GLACIER        57
#define SECT_GORGE          58
#define SECT_GROVE          59
#define SECT_GUILD          60
#define SECT_HAMLET         61
#define SECT_HIGHLANDS      62
#define SECT_INLET          63
#define SECT_KNOLL          64
#define SECT_LAKE           65
#define SECT_LOWLANDS       66
#define SECT_MALPAIS        67
#define SECT_MARKETPLACE    68
#define SECT_MAUSOLEUM      69
#define SECT_MORTUARY       70
#define SECT_PALACE         71
#define SECT_PLAINS         72
#define SECT_PRISON         73
#define SECT_PUB            74
#define SECT_RIVER          75
#define SECT_SCHOOL         76
#define SECT_SNOWFIELD      77
#define SECT_STREAM         78
#define SECT_STREAMBED      79
#define SECT_TEMPLE         80
#define SECT_TENT           81
#define SECT_TOWN           82
#define SECT_TUNDRA         83
#define SECT_UNDERGROUND    84
#define SECT_VILLAGE        85
#define SECT_VOLCANO        86
#define SECT_WATERSHED      87
#define SECT_OCEAN          88
#define SECT_ALTAR          89
#define SECT_NOPASS         90
#define SECT_TRADEPOST      91
#define SECT_REEF           92      /* Z */         /* scogliera                                                    */
#define SECT_SEA            93      /* ~ */         /* mari e oceani                                                */
#define SECT_ARTIC          94      /* G */         /* tundra artica                                                */
#define SECT_COAST          95      /* . */         /* costa                                                                */
#define SECT_WILD_EXIT      96      /* @ */         /* exit from wilderness                                 */
#define SECT_ZONE_BORDER    97     /* I */         /* border for a zone                                    */
#define SECT_ZONE_INSIDE    98      /*   */         /* inside a zone                                                */
#define SECT_PORT           99      /* H */         /* porto                                                                */
#define SECT_RIVER_NAVIGABLE   100     /* k */         /* fiume percorribile con navi                  */
#define SECT_FORESTED_MOUNTAIN 101      /* T */         /* montagna boscosa                             */
#define SECT_SHIP_STERN     102                             /* ship stern (no map)                                  */
#define SECT_SHIP_DECK      103                             /* ship deck (no map)                                   */
#define SECT_SHIP_BOW       104                             /* ship bow (no map)                                    */
#define SECT_SHIP_UNDERDK   105                             /* ship underdeck (no map)                              */
#define SECT_SHIP_CABIN     106                            /* ship cabin (no map)                                  */
#define SECT_SHIP_HOLD      107                             /* ship hold (no map)                                   */
#define SECT_FERRY_DECK     108                             /* ferryboat room (no map)                              */
#define SECT_FORD           109     /* u */         /* guado                                                                */
#define SECT_SHALLOWS       110     /* - */         /* shallows                                                             */
#define SECT_BRIDGE         111
#define SECT_CRYGATE        112
#define MAX_SECT_TYPES      113



/* Wilderness types: used in room_data.sector_type */
#define NUM_WILD_SECTORS    52

#define WILD_ZONEENT 0
#define WILD_ALTAR 1
#define WILD_TRADEPOST 2
#define WILD_ZONEINTERIOR 3
#define WILD_IMPASSABLELAKE 4
#define WILD_IMPASSABLEWALL 5
#define WILD_IMPASSABLEFENCE 6
#define WILD_MAINROAD 7
#define WILD_CLAYROAD 8
#define WILD_FORESTROAD 9
#define WILD_FOOTPATH 10
#define WILD_STONEBRIDGE 11
#define WILD_WOODBRIDGE 12
#define WILD_DOCK 13
#define WILD_IMPASSABLECLIFF 14
#define WILD_IMPASSABLEFOREST 15
#define WILD_IMPASSABLEBRUSH 16
#define WILD_IMPASSABLESEA 17
#define WILD_CLEARRIVER 18
#define WILD_BIGRIVER 19
#define WILD_POND 20
#define WILD_STREAM 21
#define WILD_SPRING 22
#define WILD_LAKE 23
#define WILD_BAY 24
#define WILD_MURKYLAKE 25
#define WILD_MURKYRIVER 26
#define WILD_LAVAFLOW 27
#define WILD_LAVAPOOL 28
#define WILD_FIELD 29
#define WILD_LOWLANDS 30
#define WILD_FOREST 31
#define WILD_DARKFOREST 32
#define WILD_FORESTHILLS 33
#define WILD_MOUNTAINS 34
#define WILD_FORESTDUNES 35
#define WILD_BEACH 36
#define WILD_SHORELINE 37
#define WILD_DELTA 38
#define WILD_GRASSDUNES 39
#define WILD_PINEFOREST 40
#define WILD_SAVANNAH 41
#define WILD_DESERT 42
#define WILD_GARDEN 43
#define WILD_CHARREDFOREST 44
#define WILD_TUNDRA 45
#define WILD_JUNGLE 46
#define WILD_FLOODPLAIN 47
#define WILD_MARSH 48
#define WILD_DARKMARSH 49
#define WILD_DARKMOUNTAINS 50
#define WILD_CRYGATE 51

#define MAPZONE             1000000
#define BUILDZONE           5000000
#define MODVAL              2000 
/* char and mob-related defines *****************************************/


/* PC classes */
#define CLASS_UNDEFINED	  -1
#define CLASS_MAGIC_USER   0
#define CLASS_CLERIC       1
#define CLASS_THIEF        2
#define CLASS_WARRIOR      3
#define CLASS_RANGER       4
#define CLASS_KNIGHT       5
#define CLASS_PALADIN      6
#define CLASS_ROGUE        7
#define CLASS_ASSASSIN     8
#define CLASS_NINJA        9
#define CLASS_MYSTIC       10
#define CLASS_SHAMAN       11
#define CLASS_PRIEST       12
#define CLASS_NECROMANCER  13
#define CLASS_SORCERER     14
#define CLASS_ELEMENTALIST 15

#define NUM_CLASSES	  16  /* This must be the number of classes!! */

#define MCLASS_UNDEFINED   0
#define MCLASS_HUMANOID    1
#define MCLASS_UNDEAD      2
#define MCLASS_ANIMAL      3
#define MCLASS_GENERIC     4  
#define MCLASS_GIANT       5
#define MCLASS_MAGIC_USER  6
#define MCLASS_CLERIC      7
#define MCLASS_THIEF       8        
#define MCLASS_WARRIOR     9        
#define MCLASS_RANGER      10        
#define MCLASS_KNIGHT      11          
#define MCLASS_PALADIN     12          
#define MCLASS_ROGUE       13   
#define MCLASS_ASSASSIN    14             
#define MCLASS_NINJA       15  
#define MCLASS_MYSTIC      16  
#define MCLASS_SHAMAN      17  
#define MCLASS_PRIEST      18  
#define MCLASS_NECROMANCER 19   
#define MCLASS_SORCERER    20    
#define MCLASS_ELEMENTALIST    21
#define MCLASS_ELEMENTAL   22
#define MCLASS_BLACKDRAGON 23
#define MCLASS_BLUEDRAGON  24
#define MCLASS_REDDRAGON   25
#define MCLASS_GREENDRAGON 26
#define MCLASS_GOLDDRAGON  27
#define MCLASS_RAND_DRAGON 28 // this one can randomly use breaths of all 4 main types, we can add in more colors/procs later
#define MCLASS_PLANT       29

#define NUM_MCLASSES       30  /* this must be the number of mob classes */   




/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* Clan ranks */
#define CLAN_UNDEFINED     -1
#define CLAN_NONE           0

#define NUM_CLAN_RANKS      6 /* 0,1,2... leader is top number */
#define CLAN_LEADER         (NUM_CLAN_RANKS-1)
#define CLAN_ADVISOR        (CLAN_LEADER-1)
#define MAX_CLAN_APPLICANTS 20
#define NUM_CLAN_GUARDS     2

#define NUM_STARTROOMS      3

/* Positions */
#define POS_DEAD       0	/* dead			*/
#define POS_MORTALLYW  1	/* mortally wounded	*/
#define POS_INCAP      2	/* incapacitated	*/
#define POS_STUNNED    3	/* stunned		*/
#define POS_SLEEPING   4	/* sleeping		*/
#define POS_RESTING    5	/* resting		*/
#define POS_SITTING    6	/* sitting		*/
#define POS_FIGHTING   7	/* fighting		*/
#define POS_STANDING   8	/* standing		*/
#define POS_MEDITATE   9
#define POS_MINING    10

/* Player flags: used by char_data.char_specials.act */
#define PLR_KILLER	 0   /* Player is a player-killer		*/
#define PLR_THIEF	 1   /* Player is a player-thief		*/
#define PLR_FROZEN	 2   /* Player is frozen			*/
#define PLR_DONTSET      3   /* Don't EVER set (ISNPC bit)	*/
#define PLR_WRITING	 4   /* Player writing (board/mail/olc)	*/
#define PLR_MAILING	 5   /* Player is writing mail		*/
#define PLR_CRASH	 6   /* Player needs to be crash-saved	*/
#define PLR_SITEOK	 7   /* Player has been site-cleared	*/
#define PLR_NOSHOUT	 8   /* Player not allowed to shout/goss	*/
#define PLR_NOTITLE	 9   /* Player not allowed to set title	*/
#define PLR_DELETED	 10  /* Player deleted - space reusable	*/
#define PLR_LOADROOM	 11  /* Player uses nonstandard loadroom	*/
#define PLR_NOWIZLIST	 12  /* Player shouldn't be on wizlist	*/
#define PLR_NODELETE	 13  /* Player shouldn't be deleted	*/
#define PLR_INVSTART	 14  /* Player should enter game wizinvis	*/
#define PLR_CRYO	 15  /* Player is cryo-saved (purge prog)	*/
#define PLR_IN_CPK       16
#define PLR_IN_NPK       17
#define PLR_NOTDEADYET	 18  /* (R) Player being extracted.	*/
#define PLR_FISHING      19  /* Player has a line in the water   */
#define PLR_FISH_ON      20  /* Player has a fish on their line  */
#define PLR_AFTERLIFE    21  // player is in the afterlife
#define PLR_TAKE_SMOKE   22  // attacking castle smoke
#define PLR_TAKE_BLAZE   23  // attacking castle blaze
#define PLR_TAKE_CANN    24  // attacking castle cannibis
#define PLR_TAKE_SATI    25  // attacking castle sativa
#define PLR_QUESTOR      26  /* LJ Quest*/
#define PLR_BANNED       27  // arena
#define PLR_DYING        28 // dying player
#define PLR_DELCRASH     29
#define PLR_OREMINE      30
#define PLR_GEMMINE      31
#define PLR_OUTLAW       32
#define PLR_TAGGED       33
#define PLR_DEAF         34
#define PLR_PARTYIN      35
#define PLR_IACGA        36 
#define PLR_INCRYGATE    37  /* in crystalgate, traveling! */
#define PLR_LUMBER       38
#define PLR_COOKING      39
#define PLR_TANNING      40
#define PLR_LEATHERING   41
#define PLR_SKINNING     42
#define PLR_MAPEDITOR    43
#define PLR_PRECEPTOR    44
#define PLR_COMBATSET2   45
#define PLR_CAMPED       46               /* s (R) must save mounts and vehicles too*/

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         0  /* Mob has a callable spec-proc	*/
#define MOB_SENTINEL     1  /* Mob should not move		*/
#define MOB_SCAVENGER    2  /* Mob picks up stuff on the ground	*/
#define MOB_ISNPC        3  /* (R) Automatically set on all Mobs	*/
#define MOB_AWARE	 4  /* Mob can't be backstabbed		*/
#define MOB_AGGRESSIVE   5  /* Mob auto-attacks everybody nearby	*/
#define MOB_STAY_ZONE    6  /* Mob shouldn't wander out of zone	*/
#define MOB_WIMPY        7  /* Mob flees if severely injured	*/
#define MOB_AGGR_EVIL	 8  /* Auto-attack any evil PC's		*/
#define MOB_AGGR_GOOD	 9  /* Auto-attack any good PC's		*/
#define MOB_AGGR_NEUTRAL 10 /* Auto-attack any neutral PC's	*/
#define MOB_MEMORY	 11 /* remember attackers if attacked	*/
#define MOB_HELPER	 12 /* attack PCs fighting other NPCs	*/
#define MOB_NOCHARM	 13 /* Mob can't be charmed		*/
#define MOB_NOSUMMON	 14 /* Mob can't be summoned		*/
#define MOB_NOSLEEP	 15 /* Mob can't be slept		*/
#define MOB_NOBASH	 16 /* Mob can't be bashed (e.g. trees)	*/
#define MOB_NOBLIND	 17 /* Mob can't be blinded		*/
#define MOB_NOTDEADYET   18 /* (R) Mob being extracted.		*/
#define MOB_PREY         19 /* Mobs can be hunted                */
#define MOB_HUNTER       20 /* MOBS can hunt prey                */
#define MOB_QUESTMASTER  21 /* QUEST MASTER */
#define MOB_QUEST        22 /* qLJ quest*/
#define MOB_TRADEBEAST   23 /* trade shops */
#define MOB_METAL_MINER  24
#define MOB_GEM_MINER    25
#define MOB_SCALES       26
#define MOB_SKINS        27
#define MOB_DAY_ONLY     28 /* mob only repops during the day */
#define MOB_NIGHT_ONLY   29 /* mob only repops at night */
#define MOB_MOUNTABLE    30
#define MOB_BLOCKN	 31
#define MOB_BLOCKS	 32
#define MOB_BLOCKE	 33
#define MOB_BLOCKW 	 34
#define MOB_BLOCKU	 35
#define MOB_BLOCKD 	 36
#define MOB_PEACEFUL     37
#define MOB_PARTYMEM     38
#define MOB_BLOCKALL     39
#define MOB_MAILMAN	 40
#define MOB_ENCOUNTER    41
#define MOB_DRAFT_ANIMAL 42               /* v mob can be yoked to vehicles               */

/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF         0   /* Room descs won't normally be shown */
#define PRF_COMPACT       1   /* No extra CRLF pair before prompts */
#define PRF_NOSHOUT       2   /* Can't hear shouts */
#define PRF_NOTELL        3   /* Can't receive tells */
#define PRF_DISPHP        4   /* Display hit points in prompt */
#define PRF_DISPMANA      5   /* Display mana points in prompt */
#define PRF_DISPMOVE      6   /* Display move points in prompt */
#define PRF_AUTOEXIT      7   /* Display exits in a room */
#define PRF_NOHASSLE      8   /* Aggr mobs won't attack */
#define PRF_QUEST         9   /* On quest */
#define PRF_SUMMONABLE   10   /* Can be summoned */
#define PRF_NOREPEAT     11   /* No repetition of comm commands */
#define PRF_HOLYLIGHT    12   /* Can see in dark */
#define PRF_COLOR_1      13   /* Color (low bit) */
#define PRF_COLOR_2      14   /* Color (high bit) */
#define PRF_NOWIZ        15   /* Can't hear wizline */
#define PRF_LOG1         16   /* On-line System Log (low bit) */
#define PRF_LOG2         17   /* On-line System Log (high bit) */
#define PRF_NOAUCT       18   /* Can't hear auction channel */
#define PRF_NOGOSS       19   /* Can't hear gossip channel */
#define PRF_NOGRATZ      20   /* Can't hear grats channel */
#define PRF_ROOMFLAGS    21   /* Can see VNUMs */
#define PRF_DISPAUTO     22   /* Show prompt HP, MP, MV when < 25% */
#define PRF_CLS          23   /* Clear screen in OLC */
#define PRF_BUILDWALK    24   /* Build new rooms while walking */
#define PRF_AFK          25   /* AFK flag */
#define PRF_AUTOLOOT     26   /* Loot everything from a corpse */
#define PRF_AUTOGOLD     27   /* Loot gold from a corpse */
#define PRF_AUTOSPLIT    28   /* Split gold with group */
#define PRF_AUTOSAC      29   /* Sacrifice a corpse */
#define PRF_AUTOASSIST   30   /* Auto-assist toggle */
#define PRF_AUTOMAP      31
#define PRF_WILDTEXT     32               /* w see wild with word instead of graph*/
#define PRF_WILDBLACK    33               /* x see wild symbols in b/w                    */
#define PRF_WILDSMALL    34               /* y see wild reduced to the max                */
#define NUM_PRF_FLAGS    35

/* DO NOT ADD ANYMORE PRF_XXXX FLAGS */

#define PRF2_NOSPORT      0 // arena channel
#define PRF2_CLANTALK     1 /* Can't hear clan channel            */
#define PRF2_ALLCTELL     2 /* Can't hear all clan channels(imm)  */
#define PRF2_AUTOMAP      3   /* Show map at the side of room descs */
#define PRF2_ARENA        4
#define PRF2_PRECEPTOR    5 /* Char is marked Preceptor          */
#define PRF2_NOPRECEPTOR  6 /* Can't hear precept channel */
#define PRF2_NOHINT       7 /* Can't hear hint channel */
#define PRF2_NOZONERESET  8 /* Don't show zoneresets on complete syslog */
#define PRF2_NOPRAY       9 /* Can't hear pray channel */

#define NUM_PRF2_FLAGS    10

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_BLIND              1	   /* (R) Char is blind		*/
#define AFF_INVISIBLE          2	   /* Char is invisible		*/
#define AFF_DETECT_ALIGN       3	   /* Char is sensitive to align*/
#define AFF_DETECT_INVIS       4	   /* Char can see invis chars  */
#define AFF_DETECT_MAGIC       5	   /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE         6	   /* Char can sense hidden life*/
#define AFF_WATERWALK	       7	   /* Char can walk on water	*/
#define AFF_SANCTUARY          8	   /* Char protected by sanct.	*/
#define AFF_GROUP              9	   /* (R) Char is grouped	*/
#define AFF_CURSE              10   /* Char is cursed		*/
#define AFF_INFRAVISION        11	   /* Char can see in dark	*/
#define AFF_POISON             12	   /* (R) Char is poisoned	*/
#define AFF_PROTECT_EVIL       13	   /* Char protected from evil  */
#define AFF_PROTECT_GOOD       14	   /* Char protected from good  */
#define AFF_SLEEP              15   /* (R) Char magically asleep	*/
#define AFF_NOTRACK	       16	   /* Char can't be tracked	*/
#define AFF_DEAD	       17	   /* Room for future expansion	*/
#define AFF_UNUSED17	       18	   /* Room for future expansion	*/
#define AFF_SNEAK              19	   /* Char can move quietly	*/
#define AFF_HIDE               20	   /* Char is hidden		*/
#define AFF_UNUSED20	       21	   /* Room for future expansion	*/
#define AFF_CHARM              22	   /* Char is charmed		*/
#define AFF_REGEN              23
#define AFF_MELEEHEAL          24
#define AFF_EVASION            25
#define AFF_CLOAKED_MOVE       26
#define AFF_ENTANGLE           27
#define AFF_PLAGUE             28
#define AFF_MANASHIELD         29
#define AFF_FLYING             30   /* Char is flying */
#define AFF_SCUBA              31   /* Room for future expansion */
#define AFF_SPARKSHIELD        32
#define AFF_TAMED              33
#define AFF_IMAGES             34
#define AFF_DEATH_FIELD        35
#define AFF_HEROISM            36
#define AFF_COURAGE            37
#define AFF_VENGEANCE          38
#define AFF_DIVINE_PROT        39
#define AFF_QUICKNESS          40
#define AFF_REFLECT            41
#define AFF_DEFLECT            42

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING	 0	/* Playing - Nominal state		*/
#define CON_CLOSE	 1	/* User disconnect, remove character.	*/
#define CON_GET_NAME	 2	/* By what name ..?			*/
#define CON_NAME_CNFRM	 3	/* Did I get that right, x?		*/
#define CON_PASSWORD	 4	/* Password:				*/
#define CON_NEWPASSWD	 5	/* Give me a password for x		*/
#define CON_CNFPASSWD	 6	/* Please retype password:		*/
#define CON_QEMAIL       7      /* Players valid email addy             */
#define CON_QSEX	 8	/* Sex?					*/
#define CON_QCLASS	 9	/* Class?				*/
#define CON_RMOTD	 10	/* PRESS RETURN after MOTD		*/
#define CON_MENU	 11	/* Your choice: (main menu)		*/
#define CON_EXDESC	 12	/* Enter a new description:		*/
#define CON_CHPWD_GETOLD 13	/* Changing passwd: get old		*/
#define CON_CHPWD_GETNEW 14	/* Changing passwd: get new		*/
#define CON_CHPWD_VRFY   15	/* Verify new password			*/
#define CON_DELCNF1	 16	/* Delete confirmation 1		*/
#define CON_DELCNF2	 17	/* Delete confirmation 2		*/
#define CON_DISCONNECT	 18	/* In-game link loss (leave character)	*/
#define CON_OEDIT	 19	/* OLC mode - object editor		*/
#define CON_REDIT	 20	/* OLC mode - room editor		*/
#define CON_ZEDIT	 21	/* OLC mode - zone info editor		*/
#define CON_MEDIT	 22	/* OLC mode - mobile editor		*/
#define CON_SEDIT	 23	/* OLC mode - shop editor		*/
#define CON_TEDIT	 24	/* OLC mode - text editor		*/
#define CON_CEDIT	 25	/* OLC mode - conf editor		*/
#define CON_AEDIT        26     /* OLC mode - social (action) edit      */
#define CON_FASK         27     // for following
#define CON_FANS         28     // for following
#define CON_CLANEDIT     29     /* OLC mode - clan editor               */
#define CON_TRIGEDIT     30	/* OLC mode - trigger edit              */
#define CON_HEDIT        31
#define CON_PLR_DESC     32     /* Enter a new description:             */
#define CON_QEDIT        33
#define CON_MAILEDIT     34

/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WEAR_WIELD     16
#define WEAR_HOLD      17
#define WEAR_AURA      18
#define WEAR_DWIELD    19

#define NUM_WEARS      20	/* This must be the # of eq positions!! */


/* object-related defines ********************************************/


/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT       1		/* Item is a light source	*/
#define ITEM_SCROLL      2		/* Item is a scroll		*/
#define ITEM_WAND        3		/* Item is a wand		*/
#define ITEM_STAFF       4		/* Item is a staff		*/
#define ITEM_WEAPON      5		/* Item is a weapon		*/
#define ITEM_FIREWEAP N  6		/* Unimplemented		*/
#define ITEM_CRYGATE     7		/* CRYSTAL GATE TRANSPORT NODE 	*/
#define ITEM_TREASURE    8		/* Item is a treasure, not gold	*/
#define ITEM_ARMOR       9		/* Item is armor		*/
#define ITEM_POTION     10 		/* Item is a potion		*/
#define ITEM_WORN       11		/* Unimplemented		*/
#define ITEM_OTHER      12		/* Misc object			*/
#define ITEM_TRASH      13		/* Trash - shopkeeps won't buy	*/
#define ITEM_TRAP       14		/* Unimplemented		*/
#define ITEM_CONTAINER  15		/* Item is a container		*/
#define ITEM_NOTE       16		/* Item is note 		*/
#define ITEM_DRINKCON   17		/* Item is a drink container	*/
#define ITEM_KEY        18		/* Item is a key		*/
#define ITEM_FOOD       19		/* Item is food			*/
#define ITEM_MONEY      20		/* Item is money (gold)		*/
#define ITEM_PEN        21		/* Item is a pen		*/
#define ITEM_BOAT       22		/* Item is a boat		*/
#define ITEM_FOUNTAIN   23		/* Item is a fountain		*/
#define ITEM_POLE       24               /* Fishing Pole                 */
#define ITEM_BOW        25               /* shoots arrows */
#define ITEM_SLING      26               /* shoots rocks */
#define ITEM_CROSSBOW   27               /* shoots bolts */
#define ITEM_BOLT       28
#define ITEM_ARROW      29
#define ITEM_ROCK       30
#define ITEM_CRUM_KEY   31
#define ITEM_COMP       32
#define ITEM_AUTOQUEST  33
#define ITEM_THROW      34
#define ITEM_FURNITURE  35	
#define ITEM_QUEST      36
#define ITEM_MISSILE_CONT 37
#define ITEM_GOODS      38
#define ITEM_MAP        39
#define ITEM_SPELLBOOK  40                              /* item is a spellbook                                  */
#define ITEM_FIRESTONE  41
#define ITEM_EARTHSTONE 42
#define ITEM_AIRSTONE   43
#define ITEM_WATERSTONE 44
#define ITEM_DEATHSTONE 45

/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE		 0  /* Item can be takes		*/
#define ITEM_WEAR_FINGER	 1  /* Can be worn on finger	*/
#define ITEM_WEAR_NECK		 2  /* Can be worn around neck 	*/
#define ITEM_WEAR_BODY		 3  /* Can be worn on body 	*/
#define ITEM_WEAR_HEAD		 4  /* Can be worn on head 	*/
#define ITEM_WEAR_LEGS		 5  /* Can be worn on legs	*/
#define ITEM_WEAR_FEET		 6  /* Can be worn on feet	*/
#define ITEM_WEAR_HANDS		 7  /* Can be worn on hands	*/
#define ITEM_WEAR_ARMS		 8  /* Can be worn on arms	*/
#define ITEM_WEAR_SHIELD	 9  /* Can be used as a shield	*/
#define ITEM_WEAR_ABOUT		 10 /* Can be worn about body 	*/
#define ITEM_WEAR_WAIST 	 11 /* Can be worn around waist 	*/
#define ITEM_WEAR_WRIST		 12 /* Can be worn on wrist 	*/
#define ITEM_WEAR_WIELD		 13 /* Can be wielded		*/
#define ITEM_WEAR_HOLD		 14 /* Can be held		*/
#define ITEM_WEAR_LIGHT          15
#define ITEM_WEAR_AURA           16 /* Special Aura position    */
#define ITEM_WEAR_DWIELD         17

/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW           0	/* Item is glowing		*/
#define ITEM_HUM            1	/* Item is humming		*/
#define ITEM_NORENT         2	/* Item cannot be rented	*/
#define ITEM_NODONATE       3	/* Item cannot be donated	*/
#define ITEM_NOINVIS	    4	/* Item cannot be made invis	*/
#define ITEM_INVISIBLE      5	/* Item is invisible		*/
#define ITEM_MAGIC          6	/* Item is magical		*/
#define ITEM_NODROP         7	/* Item is cursed: can't drop	*/
#define ITEM_BLESS          8	/* Item is blessed		*/
#define ITEM_ANTI_GOOD      9	/* Not usable by good people	*/
#define ITEM_ANTI_EVIL      10	/* Not usable by evil people	*/
#define ITEM_ANTI_NEUTRAL   11	/* Not usable by neutral people	*/
#define ITEM_ANTI_MAGIC_USER  12	/* Not usable by mages		*/
#define ITEM_ANTI_CLERIC    13	/* Not usable by clerics	*/
#define ITEM_ANTI_THIEF	    14	/* Not usable by thieves	*/
#define ITEM_ANTI_WARRIOR   15	/* Not usable by warriors	*/
#define ITEM_NOSELL	    16	/* Shopkeepers won't touch it	*/
#define ITEM_NOTWEAK        17   /* no tweak                     */
#define ITEM_NODET          18    // no det
#define ITEM_UNIQUE_SAVE    19    // ascii objs
#define ITEM_NOBADTWEAK     20    // no bad tweaking
#define ITEM_NO_DAMAGE      21
#define ITEM_IMPROVE        22
#define ITEM_SKINNABLE      23
#define ITEM_TANNABLE       24
#define ITEM_COOKABLE  	    25
#define ITEM_SCALEABLE      26
#define ITEM_ATTACK         27  /* has weapon proc hardcoded */
#define ITEM_CHARGEABLE     28  /* item can not be recharged */
#define ITEM_MANAREGEN      29
#define ITEM_HITPREGEN      30
#define ITEM_DONATED        31  /* item has been donated and is now !sellable */
#define ITEM_IS_SPELLBOOK   32              /* k Item has spellbook data attached   */
#define ITEM_HAS_SPELLS     33              /* l Item has spell data attached               */
#define ITEM_HAS_TRAPS      34              /* m Item has trap data attached                */
#define ITEM_NO_REPAIR      35
#define ITEM_TWO_HANDED     36

/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0	/* No effect			*/
#define APPLY_STR               1	/* Apply to strength		*/
#define APPLY_DEX               2	/* Apply to dexterity		*/
#define APPLY_INT               3	/* Apply to intelligence	*/
#define APPLY_WIS               4	/* Apply to wisdom		*/
#define APPLY_CON               5	/* Apply to constitution	*/
#define APPLY_CHA		6	/* Apply to charisma		*/
#define APPLY_CLASS             7	/* Reserved			*/
#define APPLY_LEVEL             8	/* Reserved			*/
#define APPLY_AGE               9	/* Apply to age			*/
#define APPLY_CHAR_WEIGHT      10	/* Apply to weight		*/
#define APPLY_CHAR_HEIGHT      11	/* Apply to height		*/
#define APPLY_MANA             12	/* Apply to max mana		*/
#define APPLY_HIT              13	/* Apply to max hit points	*/
#define APPLY_MOVE             14	/* Apply to max move points	*/
#define APPLY_GOLD             15	/* Reserved			*/
#define APPLY_EXP              16	/* Reserved			*/
#define APPLY_AC               17	/* Apply to Armor Class		*/
#define APPLY_HITROLL          18	/* Apply to hitroll		*/
#define APPLY_DAMROLL          19	/* Apply to damage roll		*/
#define APPLY_SAVING_PARA      20	/* Apply to save throw: paralz	*/
#define APPLY_SAVING_ROD       21	/* Apply to save throw: rods	*/
#define APPLY_SAVING_PETRI     22	/* Apply to save throw: petrif	*/
#define APPLY_SAVING_BREATH    23	/* Apply to save throw: breath	*/
#define APPLY_SAVING_SPELL     24	/* Apply to save throw: spells	*/


/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)	/* Container can be closed	*/
#define CONT_PICKPROOF      (1 << 1)	/* Container is pickproof	*/
#define CONT_CLOSED         (1 << 2)	/* Container is closed		*/
#define CONT_LOCKED         (1 << 3)	/* Container is locked		*/


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15


/* Deity Defines */

#define DEITY_NONE      0
#define DEITY_FEBRUUS   1     // GOD OF PURIFICATION AND INITIATION
#define DEITY_TAGNI     2     // GODDESS OF WITCHCRAFT
#define DEITY_TERMINUS  3     // GOD OF BOUNDARIES
#define DEITY_VIRBIUS   4     // GOD OF OUTCASTS AND OUTLAWS
#define DEITY_ASTREA    5     // GODDESS OF JUSTICE AND PURITY
#define DEITY_CARMINA   6     // GODDESS OF SPELLS AND ENCHANTMENTS
#define DEITY_FORTUNA   7     // GODDESS OF FORTUNE
#define DEITY_NOX       8     // GOD OF THE NIGHT
#define DEITY_UMBRIA    9     // GODDESS OF SHADOWS
#define DEITY_SURYA     10    // GODDESS OF SUN
#define DEITY_VAYU      11    // GOD OF WEATHER
#define DEITY_COEUS     12    // GOD OF KNOWLEDGE
#define DEITY_MAAHES    13    // GOD OF WAR
#define DEITY_GHEDE     14    // GOD OF DEATH
#define DEITY_MARFA     15    // GODDESS OF NATURE
#define MAX_DEITY       16

#define ETHOS_UNDEFINED 0
#define ETHOS_GOOD      1
#define ETHOS_NEUTRAL   2
#define ETHOS_EVIL      3
#define ETHOS_ALL       4
#define MAX_ETHOS       5

/* other miscellaneous defines *******************************************/


/* Player conditions */
#define DRUNK        0
#define HUNGER       1
#define THIRST       2
#define FULL    HUNGER

/* Sun state for weather_data */
#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5

// AUCTION STUFF

#define AUC_NULL_STATE		0       /* not doing anything */
#define AUC_OFFERING		1       /* object has been offfered */
#define AUC_GOING_ONCE		2	/* object is going once! */
#define AUC_GOING_TWICE		3	/* object is going twice! */
#define AUC_LAST_CALL		4	/* last call for the object! */
#define AUC_SOLD		5

/* Auction cancle states */
#define AUC_NORMAL_CANCEL	6	/* normal cancellation of auction */
#define AUC_QUIT_CANCEL		7	/* auction canclled because player quit */
#define AUC_WIZ_CANCEL		8	/* auction cancelled by a god */

/* Other auctioneer functions */
#define AUC_STAT		9
#define AUC_BID			10

/* 128 Bits */
#define RF_ARRAY_MAX    4
#define PM_ARRAY_MAX    4
#define PR_ARRAY_MAX    4
#define AF_ARRAY_MAX    4
#define TW_ARRAY_MAX    4
#define EF_ARRAY_MAX    4
#define ZN_ARRAY_MAX    4

/* other #defined constants **********************************************/
/* *************** ROOM TRIGGERS ***********************************************/
/* type of triggered events */
#define TRIG_NONE                               0                               /* No trigger dude!                                             */
#define TRIG_TELEPORT                   1                               /* TELEPORT Event                                               */
#define TRIG_COLLAPSE                   2                               /* COLLAPSE Event                                               */
#define TRIG_EXPLOSION                  3                               /* EXPLOSION Event                                              */
#define TRIG_HEAL                               4                               /* HEAL Event                                                   */

#define MAX_TRIG_TYPE                   4

/* triggered events are activated by.. */
#define TRIG_WHO_ALL                    0                               /* Trigger by everybody                                 */
#define TRIG_WHO_PC                             1                               /* Trigger by pc                                                */
#define TRIG_WHO_NPC                    2                               /* Trigger by mob                                               */

#define MAX_TRIG_WHO                    2

/* action that activate the trigger */
#define TRIG_ACT_ENTER                  (1 << 0)                /* a when enter in room                                 */
#define TRIG_ACT_EXIT                   (1 << 1)                /* b when exit from room                                */
#define TRIG_ACT_CAST                   (1 << 2)                /* c when cast a spell                                  */
#define TRIG_ACT_FIGHT_START    (1 << 3)                /* d when start a fight                                 */
#define TRIG_ACT_FIGHT_END              (1 << 4)                /* e when start a fight                                 */
#define TRIG_ACT_GET                    (1 << 5)                /* f when get something                                 */
#define TRIG_ACT_DROP                   (1 << 6)                /* g when drop something                                */
#define TRIG_ACT_REST                   (1 << 7)                /* h when rest                                                  */
#define TRIG_ACT_SLEEP                  (1 << 8)                /* i when sleep                                                 */
#define TRIG_ACT_SPEAK                  (1 << 9)                /* j when speak                                                 */

#define MAX_TRIG_ACT                    TRIG_ACT_SPEAK
#define NUM_TRIG_ACT                    10

#define MAX_TRIG_DELAY                  10

/* ** ITEM TRAPS ************************************************** */
/* trap damage types */
#define TRAP_DAM_NONE                   0
#define TRAP_DAM_SLEEP                  1
#define TRAP_DAM_TELEPORT               2
#define TRAP_DAM_FIRE                   3
#define TRAP_DAM_COLD                   4
#define TRAP_DAM_ACID                   5
#define TRAP_DAM_ENERGY                 6
#define TRAP_DAM_BLUNT                  7
#define TRAP_DAM_PIERCE                 8
#define TRAP_DAM_SLASH                  9

/* trap triggers */
#define TRAP_ACT_ROOM                   (1 << 0)                /* a when ch enter the room                             */
#define TRAP_ACT_OPEN                   (1 << 1)                /* b on open                                                    */
#define TRAP_ACT_CLOSE                  (1 << 2)                /* c on close                                                   */
#define TRAP_ACT_GET                    (1 << 3)                /* d on get                                                             */
#define TRAP_ACT_PUT                    (1 << 4)                /* e on put                                                             */
#define TRAP_ACT_DROP                   (1 << 5)                /* f on drop                                                    */
#define TRAP_ACT_GIVE                   (1 << 6)                /* g on give                                                    */
#define TRAP_ACT_LOOKIN                 (1 << 7)                /* f on look inside                                             */
#define TRAP_ACT_USE                    (1 << 8)                /* h when used (wear, wield, etc.)              */

/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 *
 * LVL_IMPL should always be the HIGHEST possible immortal level, and
 * LVL_IMMORT should always be the LOWEST immortal level.  The number of
 * mortal levels will always be LVL_IMMORT - 1.
 */

#define LVL_IMPL	43
#define LVL_GRGOD	42
#define LVL_GOD		41
#define LVL_IMMORT	40
#define LVL_IMM         41

/* Builders that have access to the saveall command */
#define LVL_BUILDER	LVL_GRGOD

/* Level of the 'freeze' command */
#define LVL_FREEZE	LVL_GRGOD

#define NUM_OF_DIRS	6	/* number of directions in a room (nsewud) */
#define MAGIC_NUMBER	(0x06)	/* Arbitrary number that won't be in a string */

/* Map options (settable in cedit) */
#define MAP_OFF      0
#define MAP_ON       1
#define MAP_IMM_ONLY 2


/* History */
#define HIST_ALL       0	/* Every channel     */
#define HIST_SAY       1	/* Say channel       */
#define HIST_GOSSIP    2	/* Gossip Channel    */
#define HIST_WIZNET    3	/* Wiznet Channel    */
#define HIST_TELL      4	/* Telepath Channel  */
#define HIST_SHOUT     5	/* Shout Channel     */
#define HIST_GRATS     6	/* Grats Channel     */
#define HIST_HOLLER    7	/* Holler Channel    */
#define HIST_AUCTION   8	/* Auction Channel   */
#define HIST_IMPL      9	/* ImpTalk Channel   */
#define HIST_GRGOD    10	/* Grand God Channel */
#define HIST_GOD      11	/* Norm God Channel  */
#define HIST_PREC     12 	/* Preceptor Channel */

#define NUM_HIST      13	/* Maximum Channels  */

/*
 * OPT_USEC determines how many commands will be processed by the MUD per
 * second and how frequently it does socket I/O.  A low setting will cause
 * actions to be executed more frequently but will increase overhead due to
 * more cycling to check.  A high setting (e.g. 1 Hz) may upset your players
 * as actions (such as large speedwalking chains) take longer to be executed.
 * You shouldn't need to adjust this.
 */
#define OPT_USEC	100000		/* 10 passes per second */
#define PASSES_PER_SEC	(1000000 / OPT_USEC)
#define RL_SEC		* PASSES_PER_SEC
#define RL_MIN         * 60 RL_SEC

#define PULSE_ZONE      (20 RL_SEC)
#define PULSE_MOBILE    (15 RL_SEC)
#define PULSE_VIOLENCE  ( 1 RL_SEC)
#define PULSE_BLOOD     ( 3 RL_SEC)
#define PULSE_AUTOSAVE	(60 RL_SEC)
#define PULSE_IDLEPWD	(15 RL_SEC)
#define PULSE_SANITY	(30 RL_SEC)
#define PULSE_USAGE        ( 5 RL_MIN) /* 5 mins */
#define PULSE_TIMESAVE (30 RL_MIN) /* should be >= SECS_PER_MUD_HOUR */
#define PULSE_HOURLY   (60 RL_MIN)

#define PULSE_TWITCH    (60 * 60 RL_SEC) // HC hint
#define PULSE_AUCTION	(15 RL_SEC)
#define PULSE_OBJDET    (720 * 60 RL_SEC)
#define PULSE_ARENA     (75 RL_SEC)
#define PULSE_CRYGATE   (25 RL_SEC)

/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (12 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH       1024          /* Max length of prompt        */
#define GARBAGE_SPACE		32          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE		1024        /* Static output buffer size   */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE	   (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)
#define AFK_MSG_LENGTH          96


/* ** MAX_PWD_LENGTH changed from 10 to 30 for ascii test - Sam ** */
#define EXDSCR_LENGTH           240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE              3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define HISTORY_SIZE            5     /* Keep last 5 commands. */
#define MAX_STRING_LENGTH       49152
#define MAX_INPUT_LENGTH        512   /* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH    1024  /* Max size of *raw* input */
#define MAX_MESSAGES            60
#define MAX_NAME_LENGTH         20
#define MAX_PWD_LENGTH          30
#define MAX_EMAIL_LENGTH        80    /* Max size of email for PC */
#define MAX_TITLE_LENGTH        80
#define HOST_LENGTH             40
#define PLR_DESC_LENGTH         4096
#define MAX_SKILLS              200
#define MAX_AFFECT              32
#define MAX_OBJ_AFFECT          6     /* Used in obj_file_elem */
#define MAX_NOTE_LENGTH         4000  /* arbitrary */
#define MAX_LAST_ENTRIES        6000  /* arbitrary */
#define MAX_HELP_KEYWORDS       256
#define MAX_HELP_ENTRY          MAX_STRING_LENGTH
#define MAX_COMPLETED_QUESTS    1024
#define MAX_OBJ_AFF                             6                               /* Max object affection                                 */

#define MAX_CMD_LENGTH 16384         /* 16k should be plenty and then some */


/*  Professions Variables */

#define PROF_MINING             1
#define PROF_SMITHING           2
#define PROF_WOODWORKING        3
#define PROF_FISHING            4
#define PROF_SKINNING           5
#define PROF_TANNING            6
#define PROF_LEATHERWORKING     7
#define PROF_COOKING            8
#define MAX_PROFS               8

#define SCALE_FISH              1
#define SCALE_DRAGON            2
#define SCALE_MERMAID           3
#define MAX_SCALES              3

#define PELT_DEER               1
#define PELT_DRAGON             2
#define PELT_OGRE               3
#define PELT_PIG                4
#define PELT_BEAVER             5
#define PELT_WOLF               6
#define PELT_FOX                7
#define PELT_COYOTE             8
#define MAX_PELTS               8

#define WOOD_OAK                1
#define WOOD_PINE               2
#define WOOD_ELM                3
#define WOOD_BEECH              4
#define WOOD_BIRCH              5
#define WOOD_CEDAR              6
#define WOOD_FIR                7
#define WOOD_WILLOW             8
#define MAX_WOOD                8

#define LEATHER_LIGHT           1
#define LEATHER_THICK		2
#define LEATHER_HEAVY		3
#define LEATHER_SOFT		4
#define LEATHER_HARD		5
#define LEATHER_HARDENED	6
#define LEATHER_BATTLE		7
#define LEATHER_STUDDED		8
#define MAX_LEATHER		8

/* Formation Types */
#define FORM_SOLO               0
#define FORM_RANDOM             1  /* Random Formation   */
#define FORM_STANDARD           2  /* Standard Formation */
#define FORM_SPEAR              3  /* Spear Formation    */
#define FORM_PHALANX            4  /* Phalanx Formation  */
#define FORM_SHIELD             5  /* Shield Formation   */
#define FORM_DELTA              6  /* Delta Formation    */
#define FORM_SPHERE             7  /* Sphere Formation   */
#define FORM_STAR               8  /* Star Formation     */

#define MAXCOLSIZE     2000
#define MAXROWSIZE     2000
#define MAPX           2000
#define MAPY           2000
#define MIL            256
#define MAPIMAGE       "../map.png"

#define MSL MAX_STRING_LENGTH

#define MAX_STOCK 6

/*
 * A MAX_PWD_LENGTH of 10 will cause BSD-derived systems with MD5 passwords
 * and GNU libc 2 passwords to be truncated.  On BSD this will enable anyone
 * with a name longer than 5 character to log in with any password.  If you
 * have such a system, it is suggested you change the limit to 20.
 *
 * Please note that this will erase your player files.  If you are not
 * prepared to do so, simply erase these lines but heed the above warning.
 */
#if defined(HAVE_UNSAFE_CRYPT) && MAX_PWD_LENGTH == 10
#error You need to increase MAX_PWD_LENGTH to at least 20.
#error See the comment near these errors for more explanation.
#endif


/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed long int	        sh_int;
typedef unsigned long int      	ush_int;
#if !defined(__cplusplus)	/* Anyone know a portable method? */
typedef char			bool;
#endif

#if !defined(CIRCLE_WINDOWS) || defined(LCC_WIN32)	/* Hm, sysdep.h? */
typedef signed char			byte;
#endif

/* Various virtual (human-reference) number types. */
typedef IDXTYPE room_vnum;
typedef IDXTYPE obj_vnum;
typedef IDXTYPE mob_vnum;
typedef IDXTYPE zone_vnum;
typedef IDXTYPE shop_vnum;
typedef IDXTYPE trig_vnum;
typedef IDXTYPE qst_vnum;

/* Various real (array-reference) number types. */
typedef IDXTYPE room_rnum;
typedef IDXTYPE obj_rnum;
typedef IDXTYPE mob_rnum;
typedef IDXTYPE zone_rnum;
typedef IDXTYPE shop_rnum;
typedef IDXTYPE trig_rnum;
typedef IDXTYPE qst_rnum;

typedef struct char_data chData;
typedef struct descriptor_data dsData;
typedef struct room_data rmData;
typedef struct obj_data obData;

typedef struct  coord_data                              COORD_DATA;
typedef struct  extra_descr_data                EXTRA_DESCR;

typedef struct  obj_flag_data                   OBJ_FLAG_DATA;
typedef struct  obj_affected_type               OBJ_AFFECTED;
typedef struct  obj_spells_data                 OBJ_SPELLS_DATA;
typedef struct  obj_trap_data                   OBJ_TRAP_DATA;
typedef struct  obj_data                                OBJ_DATA;

typedef struct  terrain_data                    TERRAIN_DATA;

typedef struct  room_direction_data             EXIT_DATA;
typedef struct  room_affection_data             ROOM_AFFECT;
typedef struct  room_extra_data                 ROOM_EXTRA;
typedef struct  room_trigger_data               TRIGGER_DATA;
typedef struct  room_data                               ROOM_DATA;

typedef struct  memory_rec_struct               MEMORY_REC;
typedef struct  time_info_data                  TIME_INFO_DATA;
typedef struct  time_data                               TIME_DATA;

typedef struct  char_player_data                PLAYER_DATA;
typedef struct  char_ability_data               CHAR_ABIL_DATA;
typedef struct  char_point_data                 CHAR_POINT_DATA;
typedef struct  known_data                              KNOWN_DATA;
typedef struct  known_course                    KNOWN_COURSE;
typedef struct  player_special_data             PLAYER_SPECIAL;
typedef struct  mob_special_data                MOB_SPECIAL;
typedef struct  affected_type                   AFFECTED_DATA;
typedef struct  follow_type                             FOLLOW_DATA;
typedef struct  char_data                               CHAR_DATA;

typedef struct  txt_block                               TXT_BLOCK;
typedef struct  txt_q                                   TXT_Q;
typedef struct  descriptor_data                 DESCRIPTOR_DATA;

typedef struct  msg_type                                MSG_TYPE;
typedef struct  message_type                    MESSAGE_TYPE;
typedef struct  message_list                    MESSAGE_LIST;

typedef struct  weather_data                    WEATHER_DATA;
typedef struct  weapon_prof_data                WEAPON_PROF;
typedef struct  index_data                              INDEX_DATA;
typedef struct  who_list                                WHO_LIST;

typedef struct  q_element_data                  Q_ELEM_DATA;
typedef struct  queue_data                              QUEUE_DATA;
typedef struct  event_data                              EVENT_DATA;

typedef struct  book_page_data                  BOOK_PAGE;
typedef struct  spellbook_data                  SPELLBOOK;
typedef struct  book_type_data                  BOOK_TYPE;

typedef struct  wild_remove                             WILD_REMOVE;
typedef struct  life_data                               LIFE_DATA;
typedef struct  wild_data                               WILD_DATA;
typedef struct  survey_data                             SURVEY_DATA;
typedef struct  pstone_data                             PSTONE_DATA;

typedef struct  yoke_data                               YOKE_DATA;
typedef struct  vehicle_points_data             VEHICLE_PTS;
typedef struct  vehicle_index                   VEHICLE_INDEX;
typedef struct  vehicle_data                    VEHICLE_DATA;
typedef struct  stable_rent_data                STABLE_RENT;

typedef struct  auth_data                               AUTH_DATA;

typedef struct  building_points_data    BUILDING_PTS;
typedef struct  building_commands               BUILDING_CMD;
typedef struct  building_type_data              BUILDING_TYPE;
typedef struct  building_works                  BLD_WORKS;
typedef struct  building_data                   BUILDING_DATA;

typedef struct  ship_value_data                 SHIP_VAL_DATA;
typedef struct  ship_type_data                  SHIP_TYPE;
typedef struct  ship_data                               SHIP_DATA;
typedef struct  port_data                               PORT_DATA;
typedef struct  course_data                             COURSE_DATA;
typedef struct  course_step                             COURSE_STEP;
typedef struct  ferry_data                              FERRY_DATA;

typedef struct  good_type_data                  GOOD_TYPE;
typedef struct  good_data                               GOOD_DATA;
typedef struct  trading_post_data               TRADING_POST;
typedef struct  trp_good_data                   TRP_GOOD;
typedef struct  market_vars                             MARKET_VARS;
typedef struct  market_affections               MARKET_AFF;
typedef struct  market_data                             MARKET_DATA;
typedef struct  market_good_data                MARKET_GOOD;


/* db.h */
typedef struct  reset_com                               RESET_COM;
typedef struct  zone_data                               ZONE_DATA;
typedef struct  zone_wild_data                  ZONE_WILD;
typedef struct  reset_q_element                 RESET_Q_ELEM;
typedef struct  reset_q_type                    RESET_Q_TYPE;
typedef struct  player_index_element    PLR_INDEX_ELEM;
typedef struct  help_index_element              HELP_INDEX_ELEM;
typedef struct  ban_list_element                BAN_LIST_ELEM;

/* interpreter.h */
typedef struct  command_info                    COMMAND_INFO;
typedef struct  alias_data                              ALIAS_DATA;

/* shop.h */
typedef struct  stack_data                              STACK_DATA;

/* In Spells.h */
typedef struct  spell_info_type                 SPELL_INFO_DATA;
typedef struct  attack_hit_type                 ATTACK_HIT_TYPE;

/* house.h */
typedef struct house_control_rec                HOUSE_CONTROL;

typedef struct help_index_element		HELP_DATA;
/*
 * Bitvector type for 32 bit unsigned long bitvectors.
 * 'unsigned long long' will give you at least 64 bits if you have GCC.
 *
 * Since we don't want to break the pfiles, you'll have to search throughout
 * the code for "bitvector_t" and change them yourself if you'd like this
 * extra flexibility.
 */
typedef unsigned long int	bitvector_t;

/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/

#define NUM_OBJ_VAL_POSITIONS 4
/* object flags; used in obj_data */
struct obj_flag_data {
   int	value[NUM_OBJ_VAL_POSITIONS];	/* Values of the item (see list)    */
   byte type_flag;	/* Type of item			    */
   int level;		/* Minimum level of object.		*/
   int  wear_flags[TW_ARRAY_MAX];       /* Where you can wear it            */
   int  extra_flags[EF_ARRAY_MAX];      /* If it hums, glows, etc.          */
//   int /*bitvector_t*/	wear_flags;	/* Where you can wear it	    */
//   int /*bitvector_t*/	extra_flags;	/* If it hums, glows, etc.	    */
   int	weight;		/* Weigt what else                  */
   int	cost;		/* Value when sold (gp.)            */
   int	condition;	/* Cost to keep pr. real day        */
   int	timer;   	/* Used for item damage/repair      */
   int  bitvector[AF_ARRAY_MAX];        /* To set chars bits                */
   int  bound_id;	/* ID of player object is bound to */
   int                           quality;                                        /* Item quality                                         */

//   long /*bitvector_t*/	bitvector;	/* To set chars bits                */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};

/* Spells attached to items */
struct obj_spells_data
{
  OBJ_SPELLS_DATA       *next;                                          /* link to the next node                                */
  sh_int                        spellnum;                                       /* number of the spell                                  */
  sh_int                        level;                                          /* level at which it will be cast               */
  sh_int                        percent;                                        /* % of success                                                 */
};

/* for Trapped Items */
struct obj_trap_data
{
  OBJ_TRAP_DATA         *next;                                          /* link to the next node                                */
  bool                          whole_room;                                     /* Just ch or the whole room?                   */
  int                           action;                                         /* Action that trigger the trap                 */
  int                           dam_type;                                       /* Which damage cause the trap                  */
  int                           charges;                                        /* How many times the trap act                  */
};

/*
struct mail_data {

   long id;
   bool received;
   long recip;
   struct char_data *sender;

   struct obj_data *obj;
   struct mail_data *next;

};
*/
/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_vnum item_number;	/* Where in data-base			*/
   room_rnum in_room;		/* In what room -1 when conta/carr	*/

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

   char	*name;                    /* Title of object :get etc.        */
   char	*description;		  /* When in room                     */
   char	*short_description;       /* when worn/carry/in cont.         */
   char	*action_description;      /* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;	  /* Worn by?			      */
   sh_int worn_on;		  /* Worn where?		      */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   long id;                       /* used by DG triggers - unique id  */
   struct trig_proto_list *proto_script; /* list of default triggers  */
   struct script_data *script;    /* script info for the object       */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
   struct char_data *sitting_here;/* who is sitting in it             */

   struct mail_data *in_mail;     /* Which mail object is attached to */

  OBJ_DATA                      *prev_content;                          /* For 'contains' lists                                 */
  OBJ_DATA                      *first_content;                         /* Contains objects                                             */
  OBJ_DATA                      *last_content;                          /* Contains objects                                             */
  VEHICLE_DATA          *in_vehicle;                            /* In which vehicle?                                    */
  
  int count;
  void                          *special;                                       /* for special data attached on items   */

};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files		   */
struct obj_file_elem {
   obj_vnum item_number;

#if USE_AUTOEQ
   sh_int location;
#endif
   int	value[NUM_OBJ_VAL_POSITIONS];
   int  extra_flags[EF_ARRAY_MAX];
//   int /*bitvector_t*/	extra_flags;
   int	weight;
   int	timer;
   int  bitvector[AF_ARRAY_MAX];
//   long /*bitvector_t*/	bitvector;
   struct obj_affected_type affected[MAX_OBJ_AFFECT];
};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
   int	time;
   int	rentcode;
   int	net_cost_per_diem;
   int	gold;
   int	account;
   int	nitems;
   int	spare0;
   int	spare1;
   int	spare2;
   int	spare3;
   int	spare4;
   int	spare5;
   int	spare6;
   int	spare7;
};
/* ======================================================================= */

/* other miscellaneous structures ***************************************/

typedef struct name_number                      NAME_NUMBER;

struct name_number
{
  char                          *name;
  int                           number;
};

/* room-related structures ************************************************/

/* ex sector_type */
struct terrain_data
{
  char                          *name;                                          /* Sector name                                                  */
  char                          *description;                           /* Sector description (when entered)    */
  char                          *map;                                           /* normal map (with symbols)                    */
  char                          *map2;                                          /* map with letters (aAnmvb etc)                */
  char                          *mapbw;                                         /* map for black and white                              */
  sh_int                        movement_loss;                          /* cost in movement points                              */
};

/* room triggered events data */
struct room_trigger_data
{
  TRIGGER_DATA          *next;                                          /* link to the next global node                 */
  bool                          active;                                         /* is trigger active?                                   */
  char                          *text;                                          /* some useful spot                                             */
  int                           type;                                           /* type of trigger                                              */
  int                           whotrig;                                        /* who trig the event?                                  */
  int                           action;                                         /* triggered by which action?                   */
  int                           timer;                                          /* delay for event                                              */
  int                           random;                                         /* event is sure or has random % ?              */
  int                           value[4];                                       /* see room_trigger.c                                   */
};

struct room_direction_data {
  EXIT_DATA                     *next;                                          /* next exit in room linked list                */
  EXIT_DATA                     *prev;                                          /* prev exit in room linked list                */
  EXIT_DATA                     *rexit;                                         /* Reverse exit pointer                                 */
  COORD_DATA            *coord;                                         /* for wilderness use                                   */
  COORD_DATA            *rvcoord;                                       /* for wilderness use                                   */

   char	*general_description;       /* When look DIR.			*/
  char description;

   char	*keyword;		/* for open/close			*/

   sh_int /*bitvector_t*/ exit_info;	/* Exit info			*/
   obj_vnum key;		/* Key's number (-1 for no key)		*/
   room_rnum to_room;		/* Where direction leads (NOWHERE)	*/

  room_vnum                     vnum;                                           /* VNum of destination                                  */
  room_vnum                     rvnum;                                          /* Vnum of room in opposite dir                 */
  int                           vdir;                                           /* Physical "direction"                                 */

};

struct known_data
{
  KNOWN_DATA            *next;                                          /* next node in list                                    */
  KNOWN_DATA            *prev;                                          /* prev node in list                                    */
  char                          *name;                                          /* by which name he's known                             */
  long                          idnum;                                          /* idnum of the known player                    */
};

struct known_course
{
  KNOWN_COURSE          *next;
  int                           coursenum;
};

struct MapStruct {
   int x;
   int y;
};

struct room_affect {
   int type;
   int timer;
   int value;
   char *text;
   int room;
   struct char_data *ch;
   struct room_affect *next;
}; 

/* extra data (mainly for buildings) */
struct room_extra_data
{
  int                           vnum;                                           /* building/ship vnum                                   */
  sh_int                        max_hp;                                         /* max room hp                                                  */
  sh_int                        curr_hp;                                        /* curr room hp                                                 */
};

struct room_custom {
   int wild_type;               /* Wildneress Sector                    */
   int room_flags2;             /* More Roomflags                       */
   int blood;                   /* Blood counter for room               */
   long room_affections;        /* Bitvector for room affections        */
   struct MapStruct map_coords; /* Wildneress map coordinates           */
};

/* ================== Memory Structure for room ======================= */
struct room_data {
   room_vnum number;		/* Rooms number	(vnum)		      */
   zone_rnum zone;              /* Room zone (for resetting)          */
   int	sector_type;            /* sector type (move/hide)            */
   char	*name;                  /* Rooms name 'You are ...'           */
   char	*description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   int room_flags[RF_ARRAY_MAX];   /* DEATH, DARK, etc */
   int ticks_left;

//   int /*bitvector_t*/ room_flags;		/* DEATH,DARK ... etc */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);

   struct trig_proto_list *proto_script; /* list of default triggers  */
   struct script_data *script;  /* script info for the room           */

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */
   struct char_data *tripper;

   int wild_type;               /* Wildneress Sector                    */
   int room_flags2;             /* More Roomflags                       */
   int blood;                   /* Blood counter for room               */
   struct room_affect *room_affs; /* room affections */
//   long room_affections;        /* Bitvector for room affections        */
   struct MapStruct map_coords; /* Wildneress map coordinates           */

   int x;
   int y;

  BUILDING_DATA         *buildings;                                     /* list of buildings in room                    */
  VEHICLE_DATA          *vehicles;                                      /* list of vehicles in room                             */
  SHIP_DATA             *ships;                                         /* List of Ships in room                                */
  FERRY_DATA            *ferryboat;                                     /* one single ferryboat in each room    */
  EVENT_DATA            *action;                                        /* event for action which takes time    */
  PSTONE_DATA           *portal_stone;                          /* a single portal stone in each room   */
  ROOM_EXTRA            *extra_data;                            /* for ship/building rooms                              */
  TRIGGER_DATA          *trigger;                                       /* For triggered events                                 */
  COORD_DATA            *coord;                                         /* wilderness                                                   */
  OBJ_DATA                      *first_content;                         /* Head of list of items in room                */
  OBJ_DATA                      *last_content;                          /* Tail of list of items in room                */
  EXIT_DATA                     *first_exit;                            /* Directions                                                   */
  EXIT_DATA                     *last_exit;                             /* Directions                                                   */ 
  ROOM_DATA                     *next;
  ROOM_AFFECT           *affections;

};
/* ====================================================================== */

/* clan-related structures ************************************************/
struct clan_type {
  int  number;            /* clan's UNIQUE ID Number      */
  char *name;             /* No color name of clan (string)  */
  char *applicants[MAX_CLAN_APPLICANTS];/* Pointer to strings            */
  char *leadersname;      /* Leader's (Player's) Name     */
  char *rank_name[NUM_CLAN_RANKS]; /* Rank names                      */
  char *member_look_str;  /* The clan's colored name      */
  room_vnum clan_entr_room;    /* VNUM of clan Entrance Room */
  room_vnum clan_recall;           /* VNUM of clan recall room        */
  mob_rnum guard[NUM_CLAN_GUARDS]; /* RNUM of clan guard              */
  int  direction;         /* Direction of clan entrance   */
  int  pkill;             /* TRUE if pkill desired        */
  long  clan_gold;         /* clan gold                    */
  obj_vnum clan_eq[NUM_CLAN_RANKS];/* clan equipment                  */
  struct clan_type *next;
};

/* ====================================================================== */

/* char-related structures ************************************************/
struct news_type {
   int number;
   char *msg;
   struct news_type *next;
};

/* memory structure for characters */
struct memory_rec_struct {
   long	id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   int hours, day, month;
   sh_int year;
};


/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   int	played;     /* This is the total accumulated time played in secs */
};

/* The pclean_criteria_data is set up in config.c and used in db.c to
   determine the conditions which will cause a player character to be
   deleted from disk if the automagic pwipe system is enabled (see config.c).
*/
struct pclean_criteria_data {
  int level;		/* max level for this time limit	*/
  int days;		/* time limit in days			*/
}; 


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char	passwd[MAX_PWD_LENGTH+1]; /* character's password      */
   char	*name;	       /* PC / NPC s name (kill ...  )         */
   char	*short_descr;  /* for NPC 'actions'                    */
   char	*long_descr;   /* for 'look'			       */
   char	*description;  /* Extra descriptions                   */
   char	*title;        /* PC / NPC's title                     */
   byte sex;           /* PC / NPC's sex                       */
   byte chclass;       /* PC / NPC's class		       */
   byte level;         /* PC / NPC's level                     */
   sh_int hometown;    /* PC s Hometown (zone)                 */
   struct time_data time;  /* PC's AGE in days                 */
   ubyte weight;       /* PC / NPC's weight                    */
   ubyte height;       /* PC / NPC's height                    */
   int clan;           /* PC / NPC's clan                      */
   int rank;           /* PC / NPC's clan rank                 */
   struct char_data *betted_on;
  VEHICLE_DATA          *drive_vehicle;                         /* Is driving a vehicle?                                */
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte cha;
};

/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   long int mana;
   long int max_mana;     /* Max mana for PC/NPC			   */
   long int hit;
   long int max_hit;      /* Max hit for PC/NPC                      */
   long int move;
   long int max_move;     /* Max move for PC/NPC                     */

   int armor;        /* Internal -100..100, external -10..10 AC */
   long int gold;           /* Money carried                           */
   long int bank_gold;	/* Gold the char has in a bank account	   */
   long int exp;            /* The experience of the player            */

   long int hitroll;       /* Any bonus or penalty to the hit roll    */
   long int damroll;       /* Any bonus or penalty to the damage roll */
};


/* 
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
   int	alignment;		/* +-1000 for alignments                */
   long	idnum;			/* player's idnum; -1 for mobiles	*/

   int act[PM_ARRAY_MAX];       /* act flag for NPC's; player flag for PC's */
   int affected_by[AF_ARRAY_MAX]; /* Bitvector for spells/skills affected by */

//   long /*bitvector_t*/ act;	/* act flag for NPC's; player flag for PC's */

//   long /*bitvector_t*/	affected_by;
				/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};

/* Player warning record/player history */
struct warning_data {
  char *admin;                // Name of admin setting it
  char *message;              // Message sent to player 
  time_t time;                // Time the message/warning was sent
  struct warning_data *next;  // Next warning in the list
};

#define MAX_FORM_POSITIONS 15

/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
   struct char_data *fighting;	/* Opponent				*/
   struct char_data *hunting;	/* Char hunted by this char		*/

   struct obj_data *furniture;      /* Object the char is sitting in        */
   struct char_data *next_in_furniture; /* The next person sitting */

   byte position;		/* Standing, fighting, sleeping, etc.	*/

   int	carry_weight;		/* Carried weight			*/
   byte carry_items;		/* Number of items carried		*/
   int	timer;			/* Timer for update			*/

   int form_pos;   /* formation position */
   int form_total[MAX_FORM_POSITIONS];  /* formation status (leader) */
   struct char_special_data_saved saved; /* constants saved in plrfile	*/

   struct char_data *riding;	// Who are they riding? (DAK)
   struct char_data *ridden_by; // Who is riding them? (DAK)

   int form_type;

};


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
   byte skills[MAX_SKILLS+1];	/* array of skills plus skill 0		*/
   byte PADDING0;		/* used to be spells_to_learn		*/
   bool talks[MAX_TONGUE];	/* PC s Tongues 0 for NPC		*/
   int	wimp_level;		/* Below this # of hit points, flee!	*/
   byte freeze_level;		/* Level of god who froze char, if any	*/
   sh_int invis_level;		/* level of invisibility		*/
   room_vnum load_room;		/* Which room to place char in		*/
   int pref[PR_ARRAY_MAX];      /* preference flags for players */

//   long /*bitvector_t*/	pref;	/* preference flags for PC's.		*/
//   long /*bitvector_t*/	pref2;	/* preference flags for PC's.		*/
   ubyte bad_pws;		/* number of bad password attemps	*/
   sbyte conditions[3];         /* Drunk, full, thirsty			*/
   struct txt_block *comm_hist[NUM_HIST]; /* Player's comms history */

   room_vnum recall_room;
   room_vnum altar_room;
   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   COORD_DATA            *load_coord;                            /* Which coord to place char in                 */
   bool                  athelm;                                         /* Is manouvering a ship                                */

   ubyte screen_width;          /* width of page */
   ubyte page_length;
   long int ironore;
   long int bronzeore;
   long int goldore;
   long int silverore;
   long int copperore;
   long int titaniumore;
   long int mithrilore;
   long int adamantiteore;
   long int crystal;
   long int diamond;
   long int jade;
   long int ruby;
   long int emerald;
   long int sapphire;
   long int garnet;
   long int topaz;
   char *p_align;
   char *p_tolvl;
   char *p_totxp;
   char *p_pktyp;
   int minetime;
   ubyte spare10;
   int spells_to_learn;		/* How many can you learn yet this level*/
   int olc_zone;
   int clan;
   int room_in_before_party;
   int monster_kills;
   int player_kills;
   int player_deaths;
   int deaths;
   int souls;
   int crewleader;
   int was_cleric;
   int was_thief;
   int was_warrior;
   int was_mage;
   int clan_rank;
   int was_ranger;
   int was_paladin;
   int was_knight;
   int was_ninja;
   int was_rogue;
   int was_assassin;
   int was_priest;
   int was_shaman;
   int was_mystic;
   int was_sorcerer;
   int was_elementalist;
   int was_necromancer;
   int bet_amt;
   int betted_on;
   int questmob;
   struct char_data *questgiver;
   int questobj;
   int nextquest;
   int questpoints;
   int timer;
   long	spare111;
   int	dying_time;
   long int	qkills;
   long int	retribution_dam;
   int	cpks;
   int metahp;
   int metamana;
   int metamove;
   int current_news;
   int current_gnews;
   long int prof[MAX_PROFS+1];
   long int spare29;
   int crygate_destination;
   int crygate_destinationx;
   int crygate_destinationy;
   int crygate_traveltime;
   int lumber_type;
   long int leathers[MAX_LEATHER];
   long int lumber[MAX_WOOD];
   long int skins[MAX_PELTS];
   long int scales[MAX_SCALES];
   int lumber_time;
   int skin_time;
   int scale_time;
   int cook_time;
   int tan_time;
   int security;
   int maprow;
   int mapcol;
   int recall_x;
   int recall_y;
   int altar_x;
   int altar_y;
   int pkblood;
   long int shares[MAX_STOCK];
   qst_vnum *completed_quests;           /* Quests completed              */
   int    num_completed_quests;          /* Number completed              */
   int    current_quest;                 /* vnum of current quest         */
   int    quest_time;                    /* time left on current quest    */
   int    quest_counter;                 /* Count of targets left to get  */
   int querycommand;
   int queryintcommand;
   void (*queryfunc)();
   int (*queryintfunc)();
   char queryprompt[1000];
   char queryprompt2[1000];
   int breath;
   int images;
   int ethos;
   int deity;
   int defend;
   int damdealt;
   int damtaken;
   int totdamdealt;
   int totdamtaken;
   char afk_msg[AFK_MSG_LENGTH+1];
   char *email;        /* PC email addy                        */
   char *prompt;
   long logs;
   float spare30;
   float spare31;
   float spare32;
   float spare33;
   float spare34;
   float spare35;
   float spare36;
   float spare37;
   float spare38;
   float spare39;
};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
   struct player_special_data_saved saved;

   char	*poofin;		/* Description on arrival of a god.     */
   char	*poofout;		/* Description upon a god's exit.       */
   struct alias_data *aliases;	/* Character's aliases			*/
   struct warning_data *warnings;
   long last_tell;		/* idnum of last tell from		*/
   void *last_olc_targ;		/* olc control				*/
   int last_olc_mode;		/* olc control				*/
   char *host;
   char                  **wildmap;                                      /* player wild map                                              */
   KNOWN_COURSE          *courses;                                       /* known sail courses                                   */
   bool  athelm;
  int                           load_building;                          /* Which building to place char in              */
  int                           load_ship;                                      /* which ship to place char in                  */
  int                           load_coord;                         
  KNOWN_DATA            *first_known;                           /* introduction code                                    */
  KNOWN_DATA            *last_known;                            /* introduction code                                    */

};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   memory_rec *memory;	    /* List of attackers to remember	       */
   byte	attack_type;        /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   byte damnodice;          /* The number of damage dice's	       */
   byte damsizedice;        /* The size of the damage dice's           */
   int attack1;
   int attack2;
   int attack3;
   int class;		    /* mob class                               */
   int bound_id;  	    /* id of char mob is bound to              */ 
   struct mail_edit_data *ml_list;  /* Mail info for sending from mobs by script */
   int timer;
   int owner_id;
  VEHICLE_DATA          *hitched_to;                            /* Is mob dragging a vehicle?                   */
  YOKE_DATA                     *yoke;                                          /* for yokeing pointers list                    */

};

struct mail_config {
  int mail_allowed;           /* Is mudmail allowed? (to disable whole mudmail system) Yes         */
  int objects_allowed;        /* object mailing allowed                                Yes         */
  int gold_allowed;           /* gold mailing allowed?                                 Yes         */
  int stamp_cost;             /* Stamp Cost                                            150 coins   */
  int object_cost;            /* Cost per object for each attachment                   30 coins    */
  int min_level;              /* Min level that can mail                               1           */
  int min_free_level;         /* Min level for free mail                               Immortal    */
  int allow_drafts;           /* Can players save draft mudmails?                      Yes         */
  int draft_timeout;          /* No. of days a draft mail can stay in the outbox       28 days     */
  int safe_mailing;           /* Can players be attacked or stolen from while mailing? Yes         */
  int min_mail_anywhere;      /* Min level that can send mail from anywhere?           Immortal    */
  int min_send_to_all;        /* Min level that can send mail to all players?          Implementor */
};

/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long /*bitvector_t*/	bitvector; /* Tells which bits to set (AFF_XXX) */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== */
/* Custom Code */
/* Custom Code - Colors*/
#define CLR_PLRDESC     0
#define CLR_PLREQ       1
#define CLR_RMPLR       2
#define CLR_RMMOB       3
#define CLR_RMOBJ       4
#define CLR_RMNUMBER    5
#define CLR_RMNAME      6
#define CLR_RMDESC      7
#define CLR_RMFLAG      8
#define CLR_RMSCTR      9
#define CLR_EXITS       10
#define CLR_TITLE       11
#define CLR_HEADING     12
#define CLR_DASHBAR     13
#define CLR_FIELDNAME   14
#define CLR_FIELDVALUE  15
#define CLR_BRACKETS    16
#define CLR_INVENTORY   17
#define CLR_EQLOCATION  18
#define CLR_FIELDNUMBER 19
#define CLR_OLCFVAL     20
#define CLR_OLCFVAL2    21
#define CLR_OLCFNAME    22
#define CLR_OLCFNUMBER  23

#define NUM_OF_COLORS   24


//typedef class char_custom
//{
//public:
//  char colors[NUM_OF_COLORS];   /* PC's Custom colors                   */
//  long pref2Flags;              /* PC's Prefrence flags                 */

//private:
//  int approveFlags;             /* PC/NPC Approved                      */
//  int changeNumber;             /* PC's prompt change display           */
//  int clanNumber;               /* PC's Clan number                     */
//  int clanRank;                 /* PC's Rank in clan                    */
 // long deathFlags;              /* PC's Freshly killed to undead        */
//  obj_rnum deathObject;         /* PC's Death Object                    */
//  int deaths;                   /* PC's Total deaths                    */
//  int deathTime;                /* PC's Death time remaining            */
//  char* email;                  /* PC's Email address                   */
//  int godChangeNumber;          /* PC's prompt change display - gods    */
//  long godFlags;                /* PC's Immortal's god flags            */
//  int godNewsNumber;            /* PC's godnews counter                 */
//  room_rnum gohome;             /* PC's Home                            */
//  int kills;                    /* PC's Total monster kills             */
//  int map_size[2];              /* PC's Wildneress map size             */
//  int mirrorDestination;        /* PC's portal destination              */
//  int mirrorTime;               /* PC's time to arrive                  */
//  long multiFlags;              /* PC's Multiclass flags                */
//  long muteFlags;               /* PC's Mute Flags                      */
//  int newsNumber;               /* PC's news counter                    */
//  room_rnum office;             /* PC's Immortal's office               */
//  int pks;                      /* PC's Player kills                    */
//  char* prompt;                 /* PC's Custom prompt                   */
//  long questPoints;             /* PC's Earned Questing Points          */
//  long systemFlags;             /* PC's System flags for nmudlog        */
//  int totalLevel;               /* PC/NPC's total earned levels         */
//  int train;                    /* PC's Training sessions for stats     */

//  friend class char_data;
//} c_custom;

struct deity_data {
   char godname[20];
   int ethos;
};

struct special_message_data {
   char *TheMessage;
   char *TheSender;
   char *TheReciever;
   int TimeInTransit;
   int TotalTimeToMessage;
   struct special_message_data *next;
};

struct char_data {
  CHAR_DATA                     *next_in_obj;                           /* For obj->people list                                 */
  CHAR_DATA                     *next_in_building;                      /* For building->people list                    */
  CHAR_DATA                     *next_in_vehicle;                       /* For vehicle/ship->people list                */
  OBJ_DATA                      *in_obj;                                        /* Furniture objects                                    */
  ROOM_DATA                     *last_room;
  BUILDING_DATA         *in_building;                           /* In which building                                    */
  SHIP_DATA                     *in_ship;                                       /* Char is on a ship?                                   */
  VEHICLE_DATA          *in_vehicle;                            /* Is inside a vehicle?                                 */
  OBJ_DATA                      *first_carrying;                        /* Head of object list                                  */
  OBJ_DATA                      *last_carrying;                         /* Tail of object list                                  */
  EVENT_DATA            *action;                                        /* event for action which takes time    */


   int pfilepos;			 /* playerfile pos		  */
   mob_rnum nr;                          /* Mob's rnum			  */
   room_rnum in_room;                    /* Location (real room number)	  */
   room_rnum was_in_room;		 /* location for linkdead people  */
   int wait;				 /* wait for how many loops	  */

   struct char_player_data player;       /* Normal data                   */
   struct char_ability_data real_abils;	 /* Abilities without modifiers   */
   struct char_ability_data aff_abils;	 /* Abils with spells/stones/etc  */
   struct char_point_data points;        /* Points                        */
   struct char_special_data char_specials;	/* PC/NPC specials	  */
   struct player_special_data *player_specials; /* PC specials		  */
   struct mob_special_data mob_specials;	/* NPC specials		  */

   struct affected_type *affected;       /* affected by what spells       */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   long id;                            /* used by DG triggers - unique id */
   struct trig_proto_list *proto_script; /* list of default triggers      */
   struct script_data *script;         /* script info for the object      */
   struct script_memory *memory;       /* for mob memory triggers         */

   struct char_data *next_in_room;     /* For room->people - list         */
   struct char_data *next;             /* For either monster or ppl-list  */
   struct char_data *next_fighting;    /* For fighting list               */
   struct char_data *next_in_party;

   struct follow_type *followers;        /* List of chars followers       */

   struct char_data *master;             /* Who is char following?        */  
   struct char_data *formation[5][3];    /* standard 3x3 formation        */
   struct char_data *asked_to_follow;    /* char was asked to be followed */

   struct char_data *my_dragon;		/* Bond the dragon to the player */
   struct char_data *my_rider;		/* Bond the player to the dragon */
   bool has_dragon;			/* Identify that we have a dragon*/
   bool dragonflight;			/* Dragon is in flight */
   int dragonidle;			/* Dragon is idle */
   int map_dest;			/* Destination on the map */
   int dest_x;				/* Destination X coord */
   int dest_y;				/* Destination Y coord */
   bool backtracking;			/* Dragon is backtracking for landing spot */
   int launch_x;			/* Take off spot X coord */
   int launch_y;  			/* Take off spot Y coord */

   long pref;
};


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct char_file_u {
   /* char_player_data */
   char	name[MAX_NAME_LENGTH+1];
   char	description[EXDSCR_LENGTH];
   char	title[MAX_TITLE_LENGTH+1];
   byte sex;
   byte chclass;
   byte level;
   sh_int hometown;
   time_t birth;   /* Time of birth of character     */
   int	played;    /* Number of secs played in total */
   ubyte weight;
   ubyte height;

   char	pwd[MAX_PWD_LENGTH+1];    /* character's password */

   struct char_special_data_saved char_specials_saved;
   struct player_special_data_saved player_specials_saved;
   struct char_ability_data abilities;
   struct char_point_data points;
   struct affected_type affected[MAX_AFFECT];

   time_t last_logon;		/* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];	/* host of last logon */
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
   char	*text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};

struct descriptor_data {
   socket_t	descriptor;	/* file descriptor for socket		*/
   char	host[HOST_LENGTH+1];	/* hostname				*/
   byte	bad_pws;		/* number of bad pw attemps this login	*/
   byte idle_tics;		/* tics idle at password prompt		*/
   int	connected;		/* mode of 'connectedness'		*/
   int	desc_num;		/* unique num assigned to desc		*/
   time_t login_time;		/* when the person connected		*/
   char *showstr_head;		/* for keeping track of an internal str	*/
   char **showstr_vector;	/* for paging through texts		*/
   int  showstr_count;		/* number of pages to page through	*/
   int  showstr_page;		/* which page are we currently showing?	*/
   char	**str;			/* for the modify-str system		*/
   char *backstr;		/* backup string for modify-str system	*/
   size_t max_str;	        /* maximum size of string in modify-str	*/
   long	mail_to;		/* name for mail system			*/
   int	has_prompt;		/* is the user at a prompt?             */
   char	inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input		*/
   char	last_input[MAX_INPUT_LENGTH]; /* the last input			*/
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
   char *output;		/* ptr to the current output buffer	*/
   char **history;		/* History of commands, for ! mostly.	*/
   int	history_pos;		/* Circular array position.		*/
   int  bufptr;			/* ptr to end of current output		*/
   int	bufspace;		/* space left in the output buffer	*/
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;		/* q of unprocessed input		*/
   struct char_data *character;	/* linked to char			*/
   struct char_data *original;	/* original char if switched		*/
   struct char_data *killer;
   struct descriptor_data *snooping; /* Who is this char snooping	*/
   struct descriptor_data *snoop_by; /* And who is snooping this char	*/
   struct descriptor_data *next; /* link to next descriptor		*/
   struct oasis_olc_data *olc;   /* OLC info                            */
};



/* other miscellaneous structures ***************************************/


struct msg_type {
   char	*attacker_msg;  /* message to attacker */
   char	*victim_msg;    /* message to victim   */
   char	*room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg;	/* messages when death			*/
   struct msg_type miss_msg;	/* messages when miss			*/
   struct msg_type hit_msg;	/* messages when hit			*/
   struct msg_type god_msg;	/* messages when hit on god		*/
   struct message_type *next;	/* to next messages of this kind.	*/
};


struct message_list {
   int	a_type;			/* Attack type				*/
   int	number_of_attacks;	/* How many attack messages to chose from. */
   struct message_type *msg;	/* List of messages.			*/
};

/* used in the socials */
struct social_messg {
  int act_nr;
  char *command;               /* holds copy of activating command */
  char *sort_as;              /* holds a copy of a similar command or
                               * abbreviation to sort by for the parser */
  int hide;                   /* ? */
  int min_victim_position;    /* Position of victim */
  int min_char_position;      /* Position of char */
  int min_level_char;          /* Minimum level of socialing char */

  /* No argument was supplied */
  char *char_no_arg;
  char *others_no_arg;

  /* An argument was there, and a victim was found */
  char *char_found;
  char *others_found;
  char *vict_found;

  /* An argument was there, as well as a body part, and a victim was found */
  char *char_body_found;
  char *others_body_found;
  char *vict_body_found;

  /* An argument was there, but no victim was found */
  char *not_found;

  /* The victim turned out to be the character */
  char *char_auto;
  char *others_auto;

  /* If the char cant be found search the char's inven and do these: */
  char *char_obj_found;
  char *others_obj_found;
};

struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   sh_int todam;    /* Damage Bonus/Penalty                */
   sh_int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   byte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
   sh_int hitp;
   sh_int shock;
};


/*
 * Element in monster and object index-tables.
 *
 * NOTE: Assumes sizeof(mob_vnum) >= sizeof(obj_vnum)
 */
struct index_data {
   mob_vnum	vnum;	/* virtual number of this mob/obj		*/
   int		number;	/* number of existing units of this mob/obj	*/
   SPECIAL(*func);
   char *farg;         /* string argument for special function      */
   struct trig_data *proto;     /* for triggers... the trigger */
};

/* linked list for mob/object prototype trigger lists */
struct trig_proto_list {
  int vnum;                             /* vnum of the trigger   */
  struct trig_proto_list *next;         /* next trigger          */
};

struct guild_info_type {
  int pc_class;
  room_vnum guild_room;
  int direction;
};

/*
 * Config structs
 * 
 */
 
 /*
 * The game configuration structure used for configurating the game play 
 * variables.
 */
struct game_data {
  int pk_allowed;         /* Is player killing allowed? 	  */
  int pt_allowed;         /* Is player thieving allowed?	  */
  int level_can_shout;	  /* Level player must be to shout.	  */
  int holler_move_cost;	  /* Cost to holler in move points.	  */
  int tunnel_size;        /* Number of people allowed in a tunnel.*/
  int max_exp_gain;       /* Maximum experience gainable per kill.*/
  int max_exp_loss;       /* Maximum experience losable per death.*/
  int max_npc_corpse_time;/* Num tics before NPC corpses decompose*/
  int max_pc_corpse_time; /* Num tics before PC corpse decomposes.*/
  int idle_void;          /* Num tics before PC sent to void(idle)*/
  int idle_rent_time;     /* Num tics before PC is autorented.	  */
  int idle_max_level;     /* Level of players immune to idle.     */
  int dts_are_dumps;      /* Should items in dt's be junked?	  */
  int load_into_inventory;/* Objects load in immortals inventory. */
  int track_through_doors;/* Track through doors while closed?    */
  int immort_level_ok;    /* Automatically level mortals to imm?  */
  int no_mort_to_immort;  /* Prevent mortals leveling to imms?    */
  int disp_closed_doors;  /* Display closed doors in autoexit?    */
  int password_retrieval; /* is password retrieval allowed?       */

  char *OK;               /* When player receives 'Okay.' text.	  */
  char *NOPERSON;         /* 'No-one by that name here.'	  */
  char *NOEFFECT;         /* 'Nothing seems to happen.'	          */
  int map_option;         /* MAP_ON, MAP_OFF or MAP_IMM_ONLY      */
  int map_size;           /* Default size for map command         */
  int minimap_size;       /* Default size for mini-map (automap)  */
};



/*
 * The rent and crashsave options.
 */
struct crash_save_data {
  int free_rent;          /* Should the MUD allow rent for free?  */
  int max_obj_save;       /* Max items players can rent.          */
  int min_rent_cost;      /* surcharge on top of item costs.	  */
  int auto_save;          /* Does the game automatically save ppl?*/
  int autosave_time;      /* if auto_save=TRUE, how often?        */
  int crash_file_timeout; /* Life of crashfiles and idlesaves.    */
  int rent_file_timeout;  /* Lifetime of normal rent files in days*/
};


/*
 * The room numbers. 
 */
struct room_numbers {
  room_vnum mortal_start_room;	/* vnum of room that mortals enter at.  */
  room_vnum immort_start_room;  /* vnum of room that immorts enter at.  */
  room_vnum frozen_start_room;  /* vnum of room that frozen ppl enter.  */
  room_vnum donation_room_1;    /* vnum of donation room #1.            */
  room_vnum donation_room_2;    /* vnum of donation room #2.            */
  room_vnum donation_room_3;    /* vnum of donation room #3.	        */
};


/*
 * The game operational constants.
 */
struct game_operation {
  ush_int DFLT_PORT;        /* The default port to run the game.  */
  char *DFLT_IP;            /* Bind to all interfaces.		  */
  char *DFLT_DIR;           /* The default directory (lib).	  */
  char *LOGNAME;            /* The file to log messages to.	  */
  int max_playing;          /* Maximum number of players allowed. */
  int max_filesize;         /* Maximum size of misc files.	  */
  int max_bad_pws;          /* Maximum number of pword attempts.  */
  int siteok_everyone;	    /* Everyone from all sites are SITEOK.*/
  int nameserver_is_slow;   /* Is the nameserver slow or fast?	  */
  int use_new_socials;      /* Use new or old socials file ?      */
  int auto_save_olc;        /* Does OLC save to disk right away ? */
  char *MENU;               /* The MAIN MENU.			  */
  char *GET_LOGIN_COLOR;    /* Ask for color to be tuned on or not*/
  char *WELC_MESSG;	    /* The welcome message.		  */
  char *START_MESSG;        /* The start msg for new characters.  */
};

/*
 * The Autowizard options.
 */
struct autowiz_data {
  int use_autowiz;        /* Use the autowiz feature?		*/
  int min_wizlist_lev;    /* Minimun level to show on wizlist.	*/
};

/*
 * The main configuration structure;
 */
struct config_data {
  char                   *CONFFILE;	/* config file path	 */
  struct game_data       play;		/* play related config   */
  struct crash_save_data csd;		/* rent and save related */
  struct room_numbers    room_nums;	/* room numbers          */
  struct game_operation  operation;	/* basic operation       */
  struct autowiz_data    autowiz;	/* autowiz related stuff */
  struct mail_config     mail;         /* config for mudmails   */
};

#ifdef MEMORY_DEBUG
#include "zmalloc.h"
#endif


/*  Bring overs from Lyon */

struct auth_data
{
  AUTH_DATA                     *next;                                          /* next node in authorization list              */
  long                          id_num;                                         /* player id num                                                */
  char                          *name;                                          /* player name (for fast display)               */
};


/* =================================================================== */
/*                           W E A T H E R                             */
/* =================================================================== */

struct weather_data
{
  sh_int                        temp;                                           // temperature
  sh_int                        humidity;                                       // humidity
  sh_int                        precip_rate;                            // precipitation rate
  sh_int                        precip_change;                          // precipitation change
  sh_int                        wind_dir;                                       // wind direction
  sh_int                        windspeed;                                      // wind speed
  sh_int                        pressure;                                       // pressure
  sh_int                        pressure_change;                        // change of pressure
  ush_int                       free_energy;
};

/* =================================================================== */
/*                             E V E N T S                             */
/* =================================================================== */

/* number of queues to use (reduces enqueue cost) */
#define NUM_EVENT_QUEUES                10

struct q_element_data
{
  Q_ELEM_DATA           *next;
  Q_ELEM_DATA           *prev;
  void                          *data;
  long                          key;
};

struct queue_data
{
  Q_ELEM_DATA           *head[NUM_EVENT_QUEUES];
  Q_ELEM_DATA           *tail[NUM_EVENT_QUEUES];
};

struct event_data
{
  Q_ELEM_DATA           *q_el;
  void                          *event_obj;
  EVENTFUNC(*func);
};


/* Things that use events */
typedef struct  ship_sail_event                 SAIL_EVENT;
typedef struct  room_teleport_event             ROOM_TELEPORT;
typedef struct  room_heal_event                 ROOM_HEAL;
typedef struct  camp_event                              CAMP_EVENT;
typedef struct  ferry_move_event                FERRY_EVENT;

struct ship_sail_event
{
  SHIP_DATA                     *ship;
  sh_int                        type;
};

struct room_teleport_event
{
  CHAR_DATA                     *ch;
  ROOM_DATA                     *orig_room;
  ROOM_DATA                     *target;
  char                          *msg_to_char;
  char                          *msg_to_room;
  int                           delay;
};

struct room_heal_event
{
  CHAR_DATA                     *ch;
  ROOM_DATA                     *orig_room;
  char                          *msg_to_char;
  char                          *msg_to_room;
  int                           delay;
};

struct camp_event
{
  CHAR_DATA                     *ch;
  ROOM_DATA                     *pRoom;
};

struct ferry_move_event
{
  FERRY_DATA            *ferry;
  sh_int                        dir;
};

/* Sun state for Sunlight */
#define SUN_DARK                                0
#define SUN_RISE                                1
#define SUN_LIGHT                               2
#define SUN_SET                                 3
#define MOON_RISE                               4
#define MOON_LIGHT                              5
#define MOON_SET                                6

/* Moon phases for MoonPhase */
#define MOON_NEW                                        0
#define MOON_WAXING_CRESCENT            1
#define MOON_WAXING_HALF                        2
#define MOON_WAXING_THREE_QUARTERS      3
#define MOON_FULL                                       4
#define MOON_WANING_THREE_QUARTERS      5
#define MOON_WANING_HALF                        6
#define MOON_WANING_CRESCENT            7

/* Seasons */
#define WINTER                                  0
#define SPRING                                  1
#define SUMMER                                  2
#define AUTUMN                                  3

#define NUM_SEASONS                           4

/* ==================================================================== */
/*                          W I L D E R N E S S                                                 */
/* ==================================================================== */
#define WILD_ZONE                               100000

#define WILD_Y                                  2000
#define WILD_X                                  2000

#define MAP_SIZE                                20
#define MAP_X                                   (MAP_SIZE * 3)
#define MAP_Y                                   (MAP_SIZE * 3)

#define MAP_WEATH_Y                             (WILD_Y / MAP_SIZE)
#define MAP_WEATH_X                             (WILD_X / MAP_SIZE)

#define WILD_HASH                               41
#define WDATA_HASH                              11
#define SECT_HASH                               11

#define MAX_SURVEY_DIST                 50                              /* max distance for survey                              */
#define MAX_TRACK_DIST                  20                              /* max distance for track                               */
/*#define MAX_SAIL_DIST                 30                      */      /* max distance for sailing                             */
#define MAX_SCAN_DIST                   5                               /* max distance for scanning                    */

#define WS_ADD_ROOM                             0
#define WS_REM_ROOM                             1

#define MAX_LNK_STONE                   5

#define MOD_Y                                   6
#define MOD_X                                   8
#define MOD_SMALL_Y                             2
#define MOD_SMALL_X                             3

#define RADIUS_BASE                             7
#define RADIUS_SMALL                    3


struct wild_remove
{
  WILD_REMOVE           *next;
  ROOM_DATA                     *wRoom;
  int                           timer;
};

struct coord_data
{
  sh_int                x;
  sh_int                y;
  sh_int                level;
};

struct life_data
{
  LIFE_DATA                     *next;
  char                          *name;                                          /* encounter name                                               */
  sh_int                        sect;                                           /* type of terrain                                              */
  sh_int                        num_max;                                        /* max number                                                   */
  sh_int                        num_min;                                        /* min number                                                   */
  sh_int                        num_curr;                                       /* current number (used only in wd-> )  */
  int                           vnum;                                           /* mob vnum                                                             */
  float                         regen_rate;                                     /* how many units repop per mud day             */
};

/* Wilderness Sector Data */
struct wild_data
{
  WILD_DATA                     *next;                                          /* next node in global list                             */
  COORD_DATA            *coord;                                         /* coordinates                                                  */
  LIFE_DATA                     *life_table[SECT_HASH];         /* encounters table                                             */
  char                          ws_map[MAP_SIZE][MAP_SIZE+3]; /* wildsector ascii map                           */
  sh_int                        num_of_rooms;                           /* number of currently loaded rooms             */
  time_t                        last_load;                                      /* last time it has been loaded                 */
};

struct survey_data
{
  SURVEY_DATA           *next;
  COORD_DATA            *coord;
  char                          *descriz;
};

struct pstone_data
{
  PSTONE_DATA           *next;                                          /* next node in global dble linked list */
  PSTONE_DATA           *prev;                                          /* next node in global dble linked list */
  COORD_DATA            coord;                                          /* coord where to place the pstone              */
  char                          *name;                                          /* keyword list for 'travel' cmd                */
  char                          *short_descr;                           /* for lists_stone_in_room                              */
  char                          *description;                           /* for examine, look at, etc                    */
  sh_int                        vnum;                                           /* stone vnum                                                   */
  sh_int                        link_to[MAX_LNK_STONE];         /* vnum of stones linked with this one  */
};

/* ==================================================================== */
/*                          B U I L D I N G S                           */
/* ==================================================================== */

#define BUILDING_ZONE                   5001

#define BLD_HASH                                11

/* types of buildings */
#define BLD_HUT                                 0
#define BLD_HOUSE                               1
#define BLD_STABLE                              2
#define BLD_STORE                               3
#define BLD_WORKSHOP                    4
#define BLD_CASTLE                              5                               /* TODO template!!!!                                    */

#define NUM_BUILDING_TYPE               10

/* who own the building? */
#define BLD_OWN_CHAR                    1                               /* a single char own the building               */
#define BLD_OWN_CLAN                    2                               /* a clan building                                              */

/* who can enter the building */
#define BLD_ENT_FREE                    0                               /* everybody can enter                                  */
#define BLD_ENT_CHAR                    1                               /* owner and authorized only                    */
#define BLD_ENT_CLAN                    2                               /* clan members and authorized only             */

/* building flags */
#define BLD_F_CLANHALL                  (1 << 0)                /* building is the clanhall of a clan   */
#define BLD_F_RUIN                              (1 << 1)                /* building is destroyed                                */

/* max number of defending mobs */
#define BLD_MAX_MOB                             5

/* build works phases */
#define BWKS_BASE                               1
#define BWKS_WALLS                              2
#define BWKS_FINAL                              3

struct building_points_data
{
  sh_int                        health;
  sh_int                        defense;
  sh_int                        npc_attack;                                     /* for auto-defending building ie Towers        */
  sh_int                        mob_attack;                                     /* for auto-generating defenders mobs           */
};

/*
 *  * commands for loading objects, mobs and other things on building
 *   *
 *    * valid commands are:
 *     *
 *      * Load a mobile:       M <mob vnum> <quantity> <building room vnum> <unused>
 *       * Load an object:      O <obj vnum> <quantity> <building room vnum> <unused>
 *        *
 *         */
struct building_commands
{
  BUILDING_CMD          *next;
  char                          cmd;                                            /* type of command                                              */
  int                           arg[4];                                         /* 4 integer arguments                                  */
};

struct building_type_data
{
  BUILDING_CMD          *cmds_list;                                     /* list of "reset" commands                             */
  BUILDING_PTS          range_min;                                      /* for random bld points generation             */
  BUILDING_PTS          range_max;                                      /*           =                                                  */
  char                          *name;                                          /* name of the building type                    */
  int                           vnum;                                           /* type of building (see above)                 */
  int                           size;                                           /* number of rooms                                              */
  int                           entrance;                                       /* entrance position in **rooms array   */
  int                           cost;                                           /* building cost (in GOLD coins)                */
  mob_vnum                      vmob[BLD_MAX_MOB];                      /* vnum of defenders mobs                               */
};


struct building_works
{
  BLD_WORKS                     *next;                                          /* next node in global list                             */
  BLD_WORKS                     *prev;                                          /* prev node in global list                             */
  COORD_DATA            *coord;                                         /* at which coord we're building?               */
  ROOM_DATA                     *in_room;                                       /* in which room we're building?                */
  bool                          authorized;                                     /* are building works authorized ?              */
  long                          owner_id;                                       /* building owner id                                    */
  sh_int                        owner_type;                                     /* this is a clan building or a private one? */
  sh_int                        timer;                                          /* how log before this work phase end?  */
  sh_int                        phase;                                          /* in which work phase are we?                  */
  int                           type;                                           /* code of building type                                */
  int                           num;                                            /* work number (for contracts)                  */
};


struct building_data
{
  BUILDING_DATA         *next;                                          /* for next node in global list                 */
  BUILDING_DATA         *next_in_room;                          /* for room->buildings list                             */
  BUILDING_CMD          *cmds_list;                                     /* list of "reset" commands                             */
  COORD_DATA            *coord;                                         /* for building location in wilderness  */
  BUILDING_TYPE         *type;                                          /* prototype of the building                    */
  ROOM_DATA                     *in_room;                                       /* in which room is placed                              */
  ROOM_DATA                     **rooms;                                        /* rooms list                                                   */
  CHAR_DATA                     *people;                                        /* list of people inside                                */
  TRADING_POST          *trp;                                           /* for trading post usage                               */
  AUTH_DATA                     *auth;                                          /* list of people authorized to enter   */
  BUILDING_PTS          max_val;
  BUILDING_PTS          curr_val;
  char                          *keyword;                                       /* for enter, attack, look, etc.                */
  char                          *description;                           /* for look at room, etc.                               */
  char                          can_enter;                                      /* who can enter (see above)                    */
  char                          owner_type;                                     /* type of owner (see above)                    */
  sh_int                        size;                                           /* size in rooms of the building                */
  int                           owner_id;                                       /* player id num or clan vnum                   */
  int                           vnum;                                           /* building vnum                                                */
  int                           flags;                                          /* building flags                                               */
  mob_vnum                      vmob[BLD_MAX_MOB];                      /* vnum of defenders mobs                               */
  room_vnum                     vnum_room;                                      /* for building in zones                                */
};


/* ========================================================== */

/* ==================================================================== */
/*                          V E H I C L E S                                                     */
/* ==================================================================== */

#define VEH_CART                                0
#define VEH_WAGON                               1

#define MAX_VEH_TYPE                    10

#define VEH_FLY                                 (1 << 0)
#define VEH_INVISIBLE                   (1 << 1)
#define VEH_DESTROYED                   (1 << 2)

struct yoke_data
{
  YOKE_DATA                     *next;
  YOKE_DATA                     *prev;
  CHAR_DATA                     *mob;
};


struct vehicle_points_data
{
  sh_int                        speed;                                          /* !UNUSED! how fast it moves                   */
  sh_int                        capacity;                                       /* how much it can contains                             */
  sh_int                        health;                                         /* how much damage it can suffer                */
  sh_int                        passengers;                                     /* how many people can go inside                */
  sh_int                        draft_mobs;                                     /* how many draft animals it can have   */
};


struct vehicle_index
{
  VEHICLE_PTS           value;
  char                          *name;                                          /* keyword list                                                 */
  char                          *short_description;                     /* displayed in look_at_room et similar */
  char                          *description;                           /* displayed in look_at_target etc.             */
  sh_int                        vnum;                                           /* vnum of vehicle proto                                */
};


struct vehicle_data
{
  VEHICLE_DATA          *next;                                          /* next node in global list                             */
  VEHICLE_DATA          *prev;                                          /* prev node in global list                             */
  VEHICLE_DATA          *next_in_room;                          /* next node in room list                               */
  VEHICLE_INDEX         *type;
  ROOM_DATA                     *in_room;                                       /* in which room is?                                    */
  ROOM_DATA                     *last_room;                                     /* in which room was?                                   */
  ROOM_DATA                     *veh_room;                                      /* room for placing people                              */
  CHAR_DATA                     *people;                                        /* list of people inside the vehicle    */
  CHAR_DATA                     *wagoner;                                       /* who's driving the vehicle?                   */
  OBJ_DATA                      *first_content;                         /* list of loaded goods                                 */
  OBJ_DATA                      *last_content;                          /* list of loaded goods                                 */
  YOKE_DATA                     *first_yoke;
  YOKE_DATA                     *last_yoke;
  VEHICLE_PTS           max_val;
  VEHICLE_PTS           curr_val;
  char                          *name;                                          /* keyword list                                                 */
  char                          *short_description;                     /* displayed in look_at_room et similar */
  char                          *description;                           /* displayed in look_at_target etc.             */
  int                           flags;                                          /* vehicle bitvector                                    */
  long                          owner_id;                                       /* who's the vehicle owner?                             */
};


/* ==================================================================== */
/* Stables for renting mounts                                                           */
/* ==================================================================== */

#define ST_RENT_NONE                    0
#define ST_RENT_MOUNT                   1
#define ST_RENT_VEHICLE                 2

struct stable_rent_data
{
  STABLE_RENT           *next;
  STABLE_RENT           *prev;
  char                          *filename;
  char                          *playername;
  char                          *typedescr;
  sh_int                        type;
  room_vnum                     stable_vnum;
  long                          id_player;
};


/* ==================================================================== */
/*                            S H I P S                                 */
/* ==================================================================== */
#define SHIP_ZONE                               5002

#define NUM_SHIP_TYPE                   10

#define SHIP_SPEED_BASE                 20

#define SAIL_MANUAL                             1
#define SAIL_COURSE                             2

#define SHIP_IN_PORT                    (1 << 0)
#define SHIP_SAIL                               (1 << 1)
#define SHIP_COMBAT                             (1 << 2)
#define SHIP_AT_ANCHOR                  (1 << 3)
#define SHIP_IN_COURSE                  (1 << 4)


struct ship_value_data
{
  sh_int                        speed;
  sh_int                        power;
  sh_int                        defense;
  sh_int                        equip;                                          /* crew                                                                 */
  sh_int                        loads;                                          /* load capacity (weight)                               */
  sh_int                        health;                                         /* hp                                                                   */
};

struct ship_type_data
{
  SHIP_VAL_DATA         value;
  char                          *name;                                          /* name of type of ship                                 */
  char                          *short_descr;                           /* short description                                    */
  sh_int                        skill_min;                                      /* minimum skill in NAVIGATION required */
  int                           vnum;                                           /* vnum of the prototype                                */
  int                           size;                                           /* number of rooms                                              */
};


struct ship_data
{
  SHIP_DATA                     *next;                                          /* next node in the global list                 */
  SHIP_DATA                     *prev;                                          /* prev node in the global list                 */
  SHIP_DATA                     *next_in_room;                          /* next node in the room list                   */
  SHIP_TYPE                     *type;                                          /* type of ship                                                 */
  CHAR_DATA                     *people;                                        /* who's on the ship                                    */
  CHAR_DATA                     *helmperson;                            /* who's at the helm?                                   */
  ROOM_DATA                     *in_room;                                       /* in which room is the ship                    */
  ROOM_DATA                     *last_room;                                     /*                                                                              */
  ROOM_DATA                     **rooms;                                        /* array of ship rooms                                  */
  ROOM_DATA                     *storeroom;                                     /* la stiva                                                             */
  EVENT_DATA            *action;                                        /* event for action which takes time    */
  COURSE_DATA           *course;                                        /* for automatic sailing                                */
  COURSE_STEP           *cstep;                                         /* current step in automatic sailing    */
  COORD_DATA            *port;                                          /* in which port ship is?                               */
  AUTH_DATA                     *authorized;                            /* list of authorized ppl                               */
  SHIP_VAL_DATA         curr_val;                                       /* current values                                               */
  SHIP_VAL_DATA         max_val;                                        /* standard values                                              */
  char                          *name;                                          /* the name of the ship                                 */
  int                           vnum;                                           /* vnum of the ship                                             */
  int                           direction;                                      /* naviga verso...                                              */
  int                           flags;                                          /* ship bitvector                                               */
  long                          idowner;                                        /* player owner of the ship                             */
  long                          idcaptain;                                      /* player captain of the ship                   */
};

struct port_data
{
  PORT_DATA                     *next;
  COORD_DATA            *coord;
  char                          *name;
};

struct course_step
{
  COURSE_STEP           *next;
  COURSE_STEP           *prev;
  COORD_DATA            coord;
};

struct course_data
{
  COURSE_DATA           *next;                                          /* next node in the global list                 */
  COORD_DATA            *port_orig;                                     /* coords of starting port                              */
  COORD_DATA            *port_end;                                      /* coords of destination port                   */
  COURSE_STEP           *first_step;                            /* first step out of the port                   */
  COURSE_STEP           *last_step;                                     /* last step                                                    */
  char                          *name;                                          /* course name                                                  */
  int                           vnum;                                           /* course vnum                                                  */
};


#define FERRY_ZONE                             100000// 5003
#define FERRY_WAIT                              10
#define FERRY_SPEED                             15

struct ferry_data
{
  FERRY_DATA            *next;                                          /* next node in global list                             */
  FERRY_DATA            *prev;                                          /* prev node in global list                             */
  COORD_DATA            *coord;                                         /* loading coordinates                                  */
  CHAR_DATA                     *mob;                                           /* the mob that control the ferry               */
  ROOM_DATA                     *room;                                          /* ferry room                                                   */
  ROOM_DATA                     *in_room;                                       /* where it is                                                  */
  EVENT_DATA            *action;
  char                          place;                                          /* in which site is?                                    */
  sh_int                        vnum;                                           /* vnum of the ferry                                    */
  sh_int                        dir;                                            /* direction from first to second site  */
  sh_int                        timer;                                          /* minutes counter                                              */
  int                           cost;                                           /* how much it charges to carry ppl             */
};

/* ==================================================================== */
/* Goods                                                                                */
/* ==================================================================== */

/* Global limits */
#define MAX_GOOD                                200
#define MAX_MARKET                              20

/* Types of goods */
#define TYPE_CORN                               0
#define TYPE_FLOUR                              1
#define TYPE_FABRIC                             2
#define TYPE_HIDE                               3
#define TYPE_DYE                                4
#define TYPE_METAL                              5
#define TYPE_ORE                                6
#define TYPE_SPICE                              7
#define TYPE_SUGAR                              8
#define TYPE_OIL                                9
#define TYPE_WOOD                               10

#define MAX_TYPE_CODE                   11

/* Types of TP */
#define TP_GCLOSE                               0
#define TP_GSTRICT                              1
#define TP_GLIMITED                             2
#define TP_GFREE                                3

#define NUM_TP_TYPES                    4

/* Goods <--> Markets data */
struct market_good_data
{
  sh_int                        total_tp;                                       /* in how many tp the good is sold?             */
  sh_int                        good_appet;
  int                           qty;                                            /* market's good quantity                               */
  int                           price;
  float                         comm_prod;
  float                         demand;                                         /* market's good demand                                 */
  float                         comm_closure;
};

/* Types of good */
struct good_type_data
{
  char                          *name;                                          /* type of good                                                 */
  int                           vnum;                                           /* vnum of type                                                 */
  int                           prod_avg[NUM_SEASONS];          /* minimal weekly seasonal production   */
  int                           elasticity;
  float                         cons_speed;                                     /* market's good consumption speed              */
};

/* Goods data */
struct good_data
{
  GOOD_TYPE                     *gtype;                                         /* type of good                                                 */
  char                          *name;                                          /* good name                                                    */
  char                          *unit;                                          /* good unit of measurement                             */
  char                          *short_descr;                           /* good unit + good name                                */
  sh_int                        code;                                           /* which type of good is                                */
  sh_int                        weight;                                         /* how much one unit weight                             */
  sh_int                        life;                                           /* how much it last                                             */
  sh_int                        docg;                                           /* 1 stop osmotic production                    */
  int                           cost;                                           /* how much one unit cost                               */
  int                           vnum;                                           /* good vnum                                                    */
  int                           mkvnum;                                         /* source market vnum                                   */
  int                           quality;                                        /* for production code (act.works.c)    */
};

/* Goods in TP data */
struct trp_good_data
{
  TRP_GOOD                      *next;                                          /* next node in tp goods list                   */
  TRP_GOOD                      *prev;                                          /* prev node in tp goods list                   */
  int                           goods_vnum;                                     /* vnum of the goods                                    */
  int                           quantity;                                       /* how many available                                   */
  int                           prev_qty;                                       /* qty prev last reset                                  */
  int                           stock;                                          /* has this goods at stock?                             */
};

/* Trading Posts data */
struct trading_post_data
{
  TRADING_POST          *next;                                          /* next node in global list                             */
  TRADING_POST          *prev;                                          /* prev node in global list                             */
  TRADING_POST          *next_in_market;                        /* next node in market list                             */
  MARKET_DATA           *market;                                        /* pointer to market area                               */
  TRP_GOOD                      *first_tpgood;                          /* first good in tp list                                */
  TRP_GOOD                      *last_tpgood;                           /* last good in tp list                                 */
  ROOM_DATA                     *in_room;                                       /* in which room is placed?                             */
  int                           vnum;                                           /* vnum of the trading post                             */
  int                           type;                                           /* type of trading post (see TP_Gxxxx)  */
};


/* Values for markets */
struct market_vars
{
  float                         prod_var;
  float                         price_var;
  float                         closure_var;
};

#define MKT_EXTRA_PROD                  (1 << 0)
#define MKT_MALUS_PROD                  (1 << 1)
#define MKT_EXTRA_PRICE                 (1 << 2)
#define MKT_MALUS_PRICE                 (1 << 3)
#define MKT_PESTILENCE                  (1 << 4)
#define MKT_INSECT                              (1 << 5)
#define MKT_DROUGHT                             (1 << 6)
#define MKT_FLOOD                               (1 << 7)
#define MKT_WAR                                 (1 << 8)

#define MOD_PROD                                1
#define MOD_PRICE                               2
#define MOD_CLOSURE                             3

struct market_affections
{
  MARKET_AFF            *next;
  sh_int                        duration;                                       /* For how long its effects will last   */
  sh_int                        what;                                           /* which value modifies                                 */
  float                         modifier;                                       /* value modifier                                               */
  long                          bitvector;                                      /* Tells which bits to set (MKT_XXX)    */
};


/* Markets data */
struct market_data
{
  MARKET_DATA           *next;                                          /* next node in global list                             */
  MARKET_DATA           *prev;                                          /* prev node in global list                             */
  TRADING_POST          *tp_list;                                       /* list of trading posts                                */
  MARKET_AFF            *affect;                                        /* list of affections                                   */
  COORD_DATA            heart;                                          /* "virtual" tp, market center                  */
  MARKET_VARS           var_real;
  MARKET_VARS           var_mod;
  char                          *name;                                          /* name of the market                                   */
  int                           vnum;                                           /* vnum of the market                                   */
  int                           size;                                           /* how big the market is? (radius)              */
  int                           num_of_tp;                                      /* how many tp this market has?                 */
  int                           affections;                                     /* bitvector of affections                              */
};


struct room_affection_data
{
  ROOM_AFFECT           *next;
  ROOM_AFFECT           *next_in_room;
  COORD_DATA            *coord;                                         /* coord of the room (for wild usage)   */
  room_vnum                     vroom;                                          /* vnum of the room          */
  char                          *text;                                          /* some useful spot                            */
  int                           timer;                                          /* how many ticks this affection lasts  */
  int                           bitvector;                                      /* Tells which bits to set (RAFF_xxx)   */
  int                           spell;                                          /* the spell number                             */
  int                           level;                                          /* level of the spell                         */
  int                           value;                                          /* some useful spot                          */
};

/* =================================================================== */
/*                          S P E L L B O O K S                        */
/* =================================================================== */
#define BOOK_TRAVELLING                 0
#define BOOK_BOOK                               1
#define BOOK_TOME                               2

#define PAGE_BLANK                              0
#define PAGE_WRITTEN                    1
#define PAGE_RUINED                             2

#define MIN_PAGES                               3

struct book_page_data
{
  BOOK_PAGE                     *next;                                          /* pointer to the next page                             */
  BOOK_PAGE                     *prev;                                          /* pointer to the prev page                             */
  char                          *spellname;                                     /* name of the spell                                    */
  sh_int                        status;                                         /* page status                                                  */
  sh_int                        flags;                                          /* page blank/written                                   */
  sh_int                        spellnum;                                       /* num of the spell written in the page */
};

struct spellbook_data
{
  BOOK_PAGE                     *first_page;                            /* first book page                                              */
  BOOK_PAGE                     *last_page;                                     /* last book page                                               */
  sh_int                        type;                                           /* type of spellbook                                    */
  sh_int                        num_of_spells;                          /* how many spells the book contains    */
  sh_int                        num_of_pages;                           /* how many pages are in the book               */
};

struct book_type_data
{
  sh_int                        max_num_pages;
  sh_int                        mult;
};


/* Help System Variables */
int hnum;
int newhelp;
struct help_keywords *hkey;
struct help_keywords *keyword_list;

struct help_keywords
{
  int helpid;
  char *keyword;
  int category;
  struct help_keywords *next;
};


/* Restring obj/mob defines */

#define RS_OBJ_NAME   0
#define RS_OBJ_DESC   1
#define RS_OBJ_SDESC  2
#define RS_OBJ_ADESC  3
#define RS_MOB_NAME   4
#define RS_MOB_DESC   5
#define RS_MOB_SDESC  6
#define RS_MOB_LDESC  7
