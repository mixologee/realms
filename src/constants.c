/**************************************************************************
*  File: constants.c                                       Part of tbaMUD *
*  Usage: Numeric and string contants used by the MUD.                    *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "interpreter.h"	/* alias_data */

cpp_extern const char *tbamud_version = "&CRealms of Kuvia v1.5&n";
cpp_extern const char *oasisolc_version = "OasisOLC 2.0.6";
cpp_extern const char *ascii_pfiles_version = "ASCII Player Files 3.0.1";
cpp_extern const char *kuvia_version = "The Realms of Kuvia v1.5"; // slurk added for other menus etc.

/* strings corresponding to ordinals/bitvectors in structs.h */
/* (Note: strings for class definitions in class.c instead of here) */

/* cardinal directions */
const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

/* ROOM_x */
const char *room_bits[] = {
  "DARK",
  "DEATH",
  "NO_MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "NO_TRACK",
  "NO_MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "HOUSE",
  "HCRSH",
  "ATRIUM",
  "OLC",
  "*",
  "NPK",
  "CPK",
  "SALT_WATER_FISHING",
  "FRESH_WATER_FISHING",
  "TREASURE_WATER_FISHING",
  "ARENA",
  "NOVIEW",
  "WILDERNESS",
  "WORLDMADP",
  "TRIPWIRE",
  "RECALL",
  "MINE",
  "SMITH",
  "FOUNTAIN",
  "REGEN",
  "WEATHERPROOF",
  "OUTDOORS",
  "CRYSTALGATE",
  "FREIGHTOK",
  "ALTAR",
  "BLACKMARKET",
  "DONATION",
  "NO_MOB_BARRIER",
  "EXTRA_MOVE_COST",
  "MAIL",
  "WILD_STATIC",
  "WILD_REMOVE",
  "SAVEROOM",
  "BUILDING_WORKS",
  "NO_RECALL",
  "NO_TELEPORT",
  "NPC_BARRIER",
  "PC_BARRIER",
  "\n"
};

/* ZN_x */
const char *zone_bits[] = {
  "CLOSED",
  "!MASTER",
  "QUEST",
  "GRID",
  "!BUILD",
  "!ASTRAL",
  "WILDERNESS",
  "\n"
};


/* EX_x */
const char *exit_bits[] = {
  "DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "HIDDEN",
  "\n"
};

/* SECT_ */
const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "In Flight",
  "Underwater",
  "Road",
  "Rock Mountain",
  "Snow Mountain",
  "Ruins",
  "Jungle",
  "Swamp",
  "Lava",
  "Entrance",
  "Farm",
  "Volcanic",
  "Air",
  "Street",
  "Beach",
  "House",
  "Shop",
//  "Empty",
  "Blank",
  "Desert",
  "Mineshaft",
  "Arctic ",
  "Arroyo ",
  "Aviary",
  "Bar ",
  "Barracks ",
  "Bastion ",
  "Bedrock ",
  "Caldera ",
  "Canyon ",
  "Castle ",
  "Cathedral ",
  "Cave ",
  "Cavern ",
  "Cemetary ",
  "Church ",
  "Citadel ",
  "Cliff ",
  "Coniferous Forest ",
  "Deciduous Forest ",
  "Delta ",
  "Dunes ",
  "Encampment",
  "Estuary ",
  "Fen ",
  "Foothills ",
  "Footpath ",
  "Fortress ",
  "Game Trail ",
  "Garden ",
  "Glacier ",
  "Gorge ",
  "Grove ",
  "Guild ",
  "Hamlet ",
  "Highlands ",
  "Inlet ",
  "Knoll ",
  "Lake ",
  "Lowlands ",
  "Malpais ",
  "Marketplace ",
  "Mausoleum ",
  "Mortuary ",
  "Palace ",
  "Plain ",
  "Prison",
  "Pub ",
  "River ",
  "School ",
  "Snowfield",
  "Stream ",
  "Streambed",
  "Temple ",
  "Tent ",
  "Town ",
  "Tundra ",
  "Underground",
  "Village",
  "Volcano",
  "Watershed",
  "Ocean",
  "Altar",
  "NoPass",
  "TradeShop",
  "Reef",
  "Sea",
  "Artic",
  "Coast",
  "WildExit",
  "ZoneBorder",
  "ZoneInside",
  "Port",
  "RiverNavigable",
  "Forested Mountain",
  "Ship Stern",
  "Ship Deck",
  "Ship Bow",
  "Ship Underdeck",
  "Ship Cabin",
  "Ship Hold",
  "Ferry Deck",
  "Ford",
  "Shallows",
  "Bridge",
  "Crystal Gate",
  "\n"
};

/* SEX_x. Not used in sprinttype() so no \n. */
const char *genders[] =
{
  "neutral",
  "male",
  "female",
  "\n"
};

/* MOB CLASS */
const char *mclass[] =
{
  "humanoid",
  "undead", 
  "animal",
  "generic", 
  "giant", 
  "magic user",
  "cleric",
  "thief",
  "warrior",
  "ranger", 
  "knight", 
  "paladin", 
  "rogue",
  "assassin", 
  "ninja",
  "mystic",
  "shaman",
  "priest",
  "necromancer",
  "sorcerer",
  "elementalist",
  "elemental",
  "black dragon", 
  "blue dragon", 
  "red dragon", 
  "green dragon",
  "gold dragon", 
  "random dragon",
  "plant",
  "\n"
};


/* POS_x */
const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "Meditating",
  "Mining",
  "\n"
};

/* PLR_x */
const char *player_bits[] = {
  "KILLER",
  "THIEF",
  "FROZEN",
  "DONTSET",
  "WRITING",
  "MAILING",
  "CSH",
  "SITEOK",
  "NOSHOUT",
  "NOTITLE",
  "DELETED",
  "LOADRM",
  "NO_WIZL",
  "NO_DEL",
  "INVST",
  "CRYO",
  "CPK",
  "NPK",
  "RESERVED",
  "FISHING",
  "FISHING_ON",
  "AFTERLIFE",
  "TAKE_SMOKE",
  "TAKE_BLAZE",
  "TAKE_CANNI",
  "TAKE_SATIV",
  "QUESTOR",
  "BANNED",
  "DYING",
  "DELCRASH",
  "MININGORE",
  "MININGGEM",
  "OUTLAW",
  "TAGGED",
  "DEAF",
  "PARTYIN",
  "IACGA",
  "CRYSTALGATE",
  "LUMBERING",
  "COOKING",
  "TANNING",
  "LEATHERING",
  "SKINNING",
  "MAPEDITOR",
  "PRECEPTOR",
  "COMBATSET2",
  "CAMPED",
  "DEAD",    // You should never see this.
  "\n"
};


/* MOB_x */
const char *action_bits[] = {
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "AWARE",
  "AGGR",
  "STAY-ZONE",
  "WIMPY",
  "AGGR_EVIL",
  "AGGR_GOOD",
  "AGGR_NEUTRAL",
  "MEMORY",
  "HELPER",
  "NO_CHARM",
  "NO_SUMMN",
  "NO_SLEEP",
  "NO_BASH",
  "NO_BLIND",
  "RESERVED",
  "PREY",
  "HUNTER",
  "QUESTMASTER",
  "QUESTMOB",
  "TRADEBEAST",
  "METAL_MINER",
  "GEM_MINER",
  "SCALES",
  "SKINS",
  "DAY_LOAD",
  "NIGHT_LOAD",
  "MOUNTABLE",
  "BLOCK NORTH",
  "BLOCK SOUTH",
  "BLOCK EAST",
  "BLOCK WEST",
  "BLOCK UP",
  "BLOCK DOWN",
  "PEACEFUL",
  "PARTY MEMBER",
  "MAIL",
  "ENCOUNTER",
  "DRAFT_ANIMAL",
  "DEAD",    /* You should never see this. */
  "\n"
};

/* PRF_x */
const char *preference_bits[] = {
  "BRIEF",
  "COMPACT",
  "NO_SHOUT",
  "NO_TELL",
  "D_HP",
  "D_MANA",
  "D_MOVE",
  "AUTOEX",
  "NO_HASS",
  "QUEST",
  "SUMN",
  "NO_REP",
  "LIGHT",
  "C1",
  "C2",
  "NO_WIZ",
  "L1",
  "L2",
  "NO_AUC",
  "NO_GOS",
  "NO_GTZ",
  "RMFLG",
  "D_AUTO",
  "CLS",
  "BLDWLK",
  "AFK",
  "AUTOLOOT",
  "AUTOGOLD",
  "AUTOSPLIT",
  "AUTOSAC",
  "AUTOASSIST",
  "AUTOMAP",
  "WILD_TEXT",
  "WILD_BLACK",
  "WILD_SMALL",
  "\n"
};

/* AFF_x */
const char *affected_bits[] =
{
  "UNUSED",
  "BLIND",
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",
  "WATWALK",
  "SANCT",
  "GROUP",
  "CURSE",
  "INFRA",
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "NO_TRACK",
  "DEAD",
  "UNUSED",
  "SNEAK",
  "HIDE",
  "UNUSED",
  "CHARM",
  "REGEN",
  "MELEEHEAL",
  "EVASION",
  "CLOAKED MOVE",
  "ENTANGLE",
  "PLAGUE",
  "MANASHIELD",
  "FLYING",
  "SCUBA",
  "SPARKSHIELD",
  "TAMED",
  "CLONE IMAGES",
  "DEATH FIELD",
  "HEROISM",
  "COURAGE",
  "VENGEANCE",
  "DIVINE PROT",
  "QUICKNESS",
  "REFLECT",
  "DEFLECT MAGIC",
  "\n"
};

/* CON_x */
const char *connected_types[] = {
  "Playing",
  "Disconnecting",
  "Get name",
  "Confirm name",
  "Get password",
  "Get new PW",
  "Confirm new PW",
  "Select sex",
  "Select class",
  "Reading MOTD",
  "Main Menu",
  "Get descript.",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",
  "Self-Delete 1",
  "Self-Delete 2",
  "Disconnecting",
  "Object edit",
  "Room edit",
  "Zone edit",
  "Mobile edit",
  "Shop edit",
  "Text edit",
  "Config edit",
  "Social edit",
  "Trigger edit",
  "Help edit",
  "\n"
};

/* WEAR_x - for eq list. Not used in sprinttype() so no \n. */
const char *wear_where[] = {
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn around neck>   ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on legs>       ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as shield>     ",
  "<worn about body>    ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<held>               ",
  "<aura>               ",
  "<dual wielded>       "
};

/* WEAR_x - for stat */
const char *equipment_types[] = {
  "Used as light",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "Aura",
  "Dual wielded",
  "\n"
};

/* ITEM_x (ordinal object types) */
const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FURNITURE",
  "CRYGATE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "FREE2",
  "CONTAINER",
  "NOTE",
  "LIQ CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "FOUNTAIN",
  "FISHING POLE",
  "BOW",
  "SLING",
  "CROSSBOW",
  "BOLT",
  "ARROW",
  "ROCK",
  "CRUMBLEKEY",
  "COMPONENT",
  "AUTOQUEST",
  "THROW",
  "CRYSTALGATE",
  "QUEST_ITEM",
  "MISSILE_CONTAINER",
  "GOODS",
  "MAP",
  "SPELLBOOK",
  "FIRESTONE",
  "EARTHSTONE",
  "AIRSTONE",
  "WATERSTONE",
  "DEATHSTONE",
  "\n"
};

/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "LIGHT",
  "AURA",
  "DWIELD",
  "\n"
};

/* ITEM_x (extra bits) */
const char *extra_bits[] = {
  "GLOW",
  "HUM",
  "NO_RENT",
  "NO_DONATE",
  "NO_INVIS",
  "INVISIBLE",
  "MAGIC",
  "NO_DROP",
  "BLESS",
  "ANTI_GOOD",
  "ANTI_EVIL",
  "ANTI_NEUTRAL",
  "ANTI_MAGE",
  "ANTI_CLERIC",
  "ANTI_THIEF",
  "ANTI_WARRIOR",
  "NO_SELL",
  "NO_TWEAK",
  "NO_DET",
  "UNIQUE",
  "NOBADTWEAK",
  "NO_DAMAGE",
  "SMITHY",
  "SKINNABLE",
  "TANNABLE",
  "COOKABLE",
  "SCALEABLE",
  "ATTACK",
  "CHARGEABLE",
  "MANAREGEN",
  "HITPREGEN",
  "DONATED",
  "IS_SPELLBOOK",
  "HAS_SPELLS",
  "HAS_TRAPS",
  "NO_REPAIR",
  "TWO_HANDED",
  "\n"
};

/* APPLY_x */
const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "CHA",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MAXMANA",
  "MAXHIT",
  "MAXMOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "\n"
};

/* APPLY_X_small */
const char *apply_types_small[] = {
"none",
"str",
"dex",
"int",
"wis",
"con",
"cha",
"class",
"level",
"age",
"weight",
"height",
"mana", 
"hps",
"mvs", 
"gold",
"exp",
"armor",
"hr",
"dr",
"svpara",
"svrod",
"svpeti",
"svbrth",
"svspll",
"\n"
};


/* CONT_x */
const char *container_bits[] = {
  "CLOSEABLE",
  "PICKPROOF",
  "CLOSED",
  "LOCKED",
  "\n",
};

/* LIQ_x */
const char *drinks[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "clear water",
  "\n"
};

/* Other constants for liquids, one-word alias for each drink. */
const char *drinknames[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local",
  "juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt",
  "water",
  "\n"
};

/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
int drink_aff[][3] = {
  {0, 1, 10},
  {3, 2, 5},
  {5, 2, 5},
  {2, 2, 5},
  {1, 2, 5},
  {6, 1, 4},
  {0, 1, 8},
  {10, 0, 0},
  {3, 3, 3},
  {0, 4, -8},
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 0, 13}
};

/* color of the various drinks */
const char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "crystal clear",
  "\n"
};

/* level of fullness for drink containers. Not used in sprinttype() so no \n. */
const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};

/* str, int, wis, dex, con applies, [ch] strength apply (all) */
cpp_extern const struct str_app_type str_app[] = {
  {-5, -4, 0, 0},	/* str = 0 */
  {-5, -4, 3, 1},	/* str = 1 */
  {-3, -2, 3, 2},
  {-3, -1, 10, 3},
  {-2, -1, 25, 4},
  {-2, -1, 55, 5},	/* str = 5 */
  {-1, 0, 80, 6},
  {-1, 0, 90, 7},
  {0, 0, 100, 8},
  {0, 0, 100, 9},
  {0, 0, 115, 10},	/* str = 10 */
  {0, 0, 115, 11},
  {0, 0, 140, 12},
  {0, 0, 140, 13},
  {0, 0, 170, 14},
  {0, 0, 170, 15},	/* str = 15 */
  {0, 1, 195, 16},
  {1, 1, 220, 18},
  {1, 2, 255, 20},	/* str = 18 */
  {3, 7, 640, 40},
  {3, 8, 700, 40},	/* str = 20 */
  {4, 9, 810, 40},
  {4, 10, 970, 40},
  {5, 11, 1130, 40},
  {6, 12, 1440, 40},
  {7, 14, 1750, 40},	/* str = 25 */
  {1, 3, 280, 22},	/* str = 18/0 - 18-50 */
  {2, 3, 305, 24},	/* str = 18/51 - 18-75 */
  {2, 4, 330, 26},	/* str = 18/76 - 18-90 */
  {2, 5, 380, 28},	/* str = 18/91 - 18-99 */
  {3, 6, 480, 30}	/* str = 18/100 */
};

/* [dex] skill apply (thieves only) */
cpp_extern const struct dex_skill_type dex_app_skill[] = {
  {-99, -99, -90, -99, -60},	/* dex = 0 */
  {-90, -90, -60, -90, -50},	/* dex = 1 */
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},	/* dex = 5 */
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},	/* dex = 10 */
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},		/* dex = 15 */
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},		/* dex = 18 */
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},		/* dex = 20 */
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25}		/* dex = 25 */
};

/* [dex] apply (all) */
cpp_extern const struct dex_app_type dex_app[] = {
  {-7, -7, 6},		/* dex = 0 */
  {-6, -6, 5},		/* dex = 1 */
  {-4, -4, 5},
  {-3, -3, 4},
  {-2, -2, 3},
  {-1, -1, 2},		/* dex = 5 */
  {0, 0, 1},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},		/* dex = 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -1},		/* dex = 15 */
  {1, 1, -2},
  {2, 2, -3},
  {2, 2, -4},		/* dex = 18 */
  {3, 3, -4},
  {3, 3, -4},		/* dex = 20 */
  {4, 4, -5},
  {4, 4, -5},
  {4, 4, -5},
  {5, 5, -6},
  {5, 5, -6}		/* dex = 25 */
};

/* [con] apply (all) */
cpp_extern const struct con_app_type con_app[] = {
  {-4, 20},		/* con = 0 */
  {-3, 25},		/* con = 1 */
  {-2, 30},
  {-2, 35},
  {-1, 40},
  {-1, 45},		/* con = 5 */
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},		/* con = 10 */
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},		/* con = 15 */
  {2, 95},
  {2, 97},
  {3, 99},		/* con = 18 */
  {3, 99},
  {4, 99},		/* con = 20 */
  {5, 99},
  {5, 99},
  {5, 99},
  {6, 99},
  {6, 99}		/* con = 25 */
};

/* [int] apply (all) */
cpp_extern const struct int_app_type int_app[] = {
  {3},		/* int = 0 */
  {5},		/* int = 1 */
  {7},
  {8},
  {9},
  {10},		/* int = 5 */
  {11},
  {12},
  {13},
  {15},
  {17},		/* int = 10 */
  {19},
  {22},
  {25},
  {30},
  {35},		/* int = 15 */
  {40},
  {45},
  {50},		/* int = 18 */
  {53},
  {55},		/* int = 20 */
  {56},
  {57},
  {58},
  {59},
  {60}		/* int = 25 */
};

/* [wis] apply (all) */
cpp_extern const struct wis_app_type wis_app[] = {
  {0},	/* wis = 0 */
  {0},  /* wis = 1 */
  {0},
  {0},
  {0},
  {0},  /* wis = 5 */
  {0},
  {0},
  {0},
  {0},
  {0},  /* wis = 10 */
  {0},
  {2},
  {2},
  {3},
  {3},  /* wis = 15 */
  {3},
  {4},
  {5},	/* wis = 18 */
  {6},
  {6},  /* wis = 20 */
  {6},
  {6},
  {7},
  {7},
  {7}  /* wis = 25 */
};

int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};

int movement_loss[] =
{
  1,    // Inside
  1,    // City
  2,    // Field
  3,    // Forest
  4,    // Hills
  6,    // Mountains
  4,    // Swimming
  1,    // Unswimable
  1,    // Flying
  5,    // Underwater
  8,    // Rock Mountain
 10,    // Snow Mountain
  2,    // Ruins
  4,    // Jungle
  6,    // Swamp
  2,    // Lava
  2,    // Entrance
  4,    // Farm
  6,    // Volcanic
  2,    // Air
  2,    // Street
  4,    // Beach
  2,    // House
  2,    // Shop
  2,    // Blank
  50,   //Desert
  4,    //Mine
  5,    //Arctic
  4,    //Arroyo
  3,    //Aviary
  2,    //Bar
  2,    //Barracks
  2,    //Bastion
  2,    //Bedrock
  2,    //Caldera
  2,    //Canyon
  2,    //Castle
  2,    //Cathedral
  2,    //Cave
  2,    //Cavern
  2,    //Cemetary
  2,    //Church
  2,    //Citadel
  2,    //Cliff
  4,    //Coniferous Forest
  4,    //Deciduous Forest
  6,    //Delta
 10,    //Dunes
  2,    //Encampment
  2,    //Estuary
  5,    //Fen
  5,    //Foothills
  3,    //Footpath
  2,    //Fortress
  2,    //Game Trail
  2,    //Garden
  8,    //Glacier
  2,    //Gorge
  3,    //Grove
  2,    //Guild
  2,    //Hamlet
  3,    //Highlands
  4,    //Inlet
  8,    //Jungle
  2,    //Knoll
  6,    //Lake
  4,    //Lowlands
  2,    //Malpais
  2,    //Marketplace
  2,    //Mausoleum
  2,    //Mortuary
  2,    //Palace
  2,    //Plain
  2,    //Prison
  2,    //Pub
  7,    //River
  2,    //School
  7,    //Snowfield
  6,    //Stream
  4,    //Streambed
  2,    //Temple
  2,    //Tent
  2,    //Town
  8,    //Tundra
  3,    //Underground
  2,    //Village
  5,    //Volcano
  2,    //Watershed
 10,    //Ocean
  2,     //Link
  5,
  9,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5
};

/* Not used in sprinttype(). */
const char *weekdays[] = {
  "the Day of the Moon",
  "the Day of the Bull",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the Day of the Great Gods",
  "the Day of the Sun"
};

/* Not used in sprinttype(). */
const char *month_name[] = {
  "Month of Winter",		/* 0 */
  "Month of the Winter Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};

/* mob trigger types */
const char *trig_types[] = {
  "Global",
  "Random",
  "Command",
  "Speech",
  "Act",
  "Death",
  "Greet",
  "Greet-All",
  "Entry",
  "Receive",
  "Fight",
  "HitPrcnt",
  "Bribe",
  "Load",
  "Memory",
  "Cast",
  "Leave",
  "Door",
  "UNUSED",
  "Time",
  "\n"
};

/* obj trigger types */
const char *otrig_types[] = {
  "Global",
  "Random",
  "Command",
  "UNUSED",
  "UNUSED",
  "Timer",
  "Get",
  "Drop",
  "Give",
  "Wear",
  "UNUSED",
  "Remove",
  "UNUSED",
  "Load",
  "UNUSED",
  "Cast",
  "Leave",
  "UNUSED",
  "Consume",
  "Time",
  "\n"
};

/* wld trigger types */
const char *wtrig_types[] = {
  "Global",
  "Random",
  "Command",
  "Speech",
  "UNUSED",
  "Zone Reset",
  "Enter",
  "Drop",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "Cast",
  "Leave",
  "Door",
  "UNUSED",
  "Time",
  "\n"
};

const char *log_types[] = {
  "INFO",
  "MISC", 
  "SYS",
  "OLC",
  "GOD",
  "DEATH",
  "ZONE",
  "CONN",
  "SUGG",
  "BUG",
  "IDEA",
  "TYPO",
  "GIVE",
  "TAKE",
  "\n"
};

const char *history_types[] = {
  "all",   
  "say",   
  "gossip",
  "wiznet",
  "tell",
  "shout",
  "grats",
  "holler",
  "auction",
  "\n"   
};

const char *moon_look[] =
{
  "new moon",                                           // 0
  "waxing crescent moon",
  "waxing half moon",
  "waxing three-quarter moon",
  "full moon",
  "waning three-quarter moon",          // 5
  "waning half moon",
  "waning crescent moon"                        // 7
};

/* buildings ************************************************ */

const char *bld_type_descr[] =
{
  "HUT",
  "HOUSE",
  "PALACE",
  "STORE",
  "WORKSHOP",
  "TAVERN",
  "CHURCH",
  "WALL",
  "STRONGHOLD",
  "CASTLE",
  "TOWER",
  "MAGE_TOWER",
  "FARM",
  "\n"
};

const char *bld_flags_descr[] =
{
  "CLANHALL",
  "RUIN",
  "\n"
};

const char *bld_owner_descr[] =
{
  "NONE",
  "CHARACTER",
  "CLAN",
  "\n"
};

const char *bld_enter_descr[] =
{
  "EVERYONE",
  "OWNER_AND_GUESTS",
  "CLAN_MEMBERS",
  "\n"
};

const char *ship_flags[] =
{
  "IN_PORT",
  "SAILING",
  "COMBAT",
  "AT_ANCHOR",
  "IN_COURSE",
  "\n"
};

const char *ethos_names[] = {
  "good", 
  "neutral", 
  "evil",
  "\n"
};

const char *dist_descr[] =
{
  "hundreds of miles away in the distance",
  "far off in the skyline",
  "many miles away at great distance",
  "far off many miles away",
  "tens of miles away in the distance",
  "far off in the distance",
  "several miles away",
  "off in the distance",
  "not far from here",
  "in near vicinity",
  "in the immediate area"
};

const char *exits_color[] =
{
  "&b&2",               // NORTH
  "&b&3",               // EAST
  "&b&4",               // SOUTH
  "&b&1",               // WEST
  "&b&6",               // UP
  "&b&7",               // DOWN
  "&b&2",               // NORTHEAST
  "&b&3",               // SOUTHEAST
  "&b&4",               // SOUTHWEST
  "&b&1"                // NORTHWEST
};

const char *trig_type_descr[] =
{
  "none",
  "teleport",
  "collapse",
  "explosion",
  "heal"
};

const char *trig_act_descr[] =
{
  "ENTER",
  "EXIT",
  "CAST",
  "FIGHT_START",
  "FIGHT_END",
  "GET",
  "DROP",
  "REST",
  "SLEEP",
  "SPEAK",
  "\n"
};

const char *trig_who_descr[] =
{
  "EVERYBODY",
  "PC",
  "NPC"
};


const char *trap_dam_descr[] =
{
  "NONE",
  "SLEEP",
  "TELEPORT",
  "FIRE",
  "COLD",
  "ACID",
  "ENERGY",
  "BLUNT",
  "PIERCE",
  "SLASH"
};

const char *trap_dam_descl[] =
{
  "none",
  "sleep",
  "teleport",
  "fire",
  "cold",
  "acid",
  "energy",
  "blunt",
  "pierce",
  "slash"
};

const char *trap_act_list[] =
{
  "ENTER_ROOM",
  "OPEN",
  "CLOSE",
  "GET",
  "PUT",
  "DROP",
  "GIVE",
  "LOOK_IN",
  "USE",
  "\n"
};

const char *trap_act_descr[] =
{
  "is set off when someone enter in the room",
  "is set off when opened",
  "is set off when closed",
  "is set off by get",
  "is set off by put",
  "is set off by drop",
  "is set off by give",
  "is set off when looked inside",
  "is set off when used",
  "\n"
};

const struct deity_data deities[] = {
  { "None",		ETHOS_ALL},
  {"test evil",        ETHOS_EVIL},
  {"test good", 	ETHOS_GOOD},
  {"test neut",         ETHOS_NEUTRAL},
  {" ",  		ETHOS_UNDEFINED}
};

/* --- End of constants arrays. --- */

/* Various arrays we count so we can check the world files.  These
 * must be at the bottom of the file so they're pre-declared. */
size_t	room_bits_count = sizeof(room_bits) / sizeof(room_bits[0]) - 1,
	action_bits_count = sizeof(action_bits) / sizeof(action_bits[0]) - 1,
	affected_bits_count = sizeof(affected_bits) / sizeof(affected_bits[0]) - 1,
	extra_bits_count = sizeof(extra_bits) / sizeof(extra_bits[0]) - 1,
	wear_bits_count = sizeof(wear_bits) / sizeof(wear_bits[0]) - 1;

const char *st_rent_descr[] =
{
  "none",
  "Mount",
  "Vehicle"
};

const char *raffected_bits [] = {
  "No Affect",
  "SHELTER",        /* Wilderness quit point           */
  "TRACKS",         /* PC/NPC tracks left to follow    */
  "NPC_BARRIER",    /* Room barrier, stops mobs        */
  "PC_BARRIER",     /* Room barrier, stops players     */
  "GAS",            /* Room temp gas flag - spells?    */
  "ACID",           /* Room temp acid flag - spells?   */
  "FIRE",           /* Room temp fire flag - spells?   */
  "WIND",           /* Room temp wind flag - spells?   */
  "FLOOD",          /* Room temp flood flag - spells?  */
  "BLOOD"           /* Blood in room                   */
};

const char *help_category[] = {
  "NULL - SHOULD NOT SEE THIS",
  "Commands",
  "Info",
  "Oasis",
  "Skills",
  "Spells",
  "Socials",
  "Subjects",
  "Wizhelp",
  "Help"
};



const char *set_types[] = {
    "nothing",
    "info",
    "misc",
    "sys",
    "olc",
    "god",
    "death",
    "zone",
    "conn",
    "sugg",
    "bug",
    "idea",
    "typo",
    "give",
    "take",
    "all",
    "off",
    "\n"
};

const char *form_types[] = {
  "Solo",
  "Random",
  "Standard",
  "Spear",
  "Phalanx",
  "Shield"
};

