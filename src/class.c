//
// File: class.c     
// Usage: Source file for class-specific code          
//

//
// This file attempts to concentrate most of the code which must be changed
// in order for new classes to be added.  If you're adding a new class,
// you should go through this entire file from beginning to end and add
// the appropriate new special cases for your new class.
//


#include "conf.h"
#include "sysdep.h"

#include "screen.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"
#include "constants.h"
#include "comm.h"
#include "logger.h"

// external functions 
void obj_to_char(struct obj_data *object, struct char_data *ch);

// local functions 
void snoop_check(struct char_data *ch);
int parse_class(char arg);
bitvector_t find_class_bitvector(const char *arg);
byte saving_throws(int class_num, int type, int level);
int thaco(int class_num, int level);
void roll_real_abils(struct char_data *ch);
int do_multi(struct char_data *ch, int class);
void do_start(struct char_data *ch);
int backstab_mult(int level, int hitroll, int ac);
int invalid_class(struct char_data *ch, struct obj_data *obj);
int level_exp(int chclass, int level);
const char *title_male(int chclass, int level);
const char *title_female(int chclass, int level);
struct char_data *get_newbie_gear(struct char_data *ch);

// Names first 
const char *class_abbrevs[] = {
  "Wi",
  "Cl",
  "Th",
  "Wa",
  "Ra",
  "Kn",
  "Pa",
  "Ro",
  "As",
  "Ni",
  "My",
  "Sh",
  "Pr",
  "Ne",
  "So",
  "El",
  "\n"
};


const char *pc_class_types[] = {
  "Wizard",
  "Cleric",
  "Thief",
  "Warrior",
  "Ranger",
  "Knight",
  "Paladin",
  "Rogue",
  "Assassin",
  "Ninja",
  "Mystic",
  "Shaman",
  "Priest",
  "Necromancer",
  "Sorcerer",
  "Elementalist",
  "\n"
};


// The menu for choosing a class in interpreter.c: 
const char *class_menu =
"\r\n"
"Select a class:\r\n"
"  [C]leric\r\n"
"  [T]hief\r\n"
"  [W]arrior\r\n"
"  [M]agic-user\r\n";



//
// The code to interpret a class letter -- used in interpreter.c when a
// new character is selecting a class and by 'set class' in act.wizard.c.
//

int parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'm': return CLASS_MAGIC_USER;
  case 'c': return CLASS_CLERIC;
  case 'w': return CLASS_WARRIOR;
  case 't': return CLASS_THIEF;
  default:  return CLASS_UNDEFINED;
  }
}

//
// bitvectors (i.e., powers of two) for each class, mainly for use in
// do_who and do_users.  Add new classes at the end so that all classes
// use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
// 1 << 5, etc.) up to the limit of your bitvector_t, typically 0-31.
//

bitvector_t find_class_bitvector(const char *arg)
{
  size_t rpos, ret = 0;

  for (rpos = 0; rpos < strlen(arg); rpos++)
    ret |= (1 << parse_class(arg[rpos]));

  return (ret);
}


//
// These are definitions which control the guildmasters for each class.
//
// The first field (top line) controls the highest percentage skill level
// a character of the class is allowed to attain in any skill.  (After
// this level, attempts to practice will say "You are already learned in
// this area."
// 
// The second line controls the maximum percent gain in learnedness a
// character is allowed per practice -- in other words, if the random
// die throw comes out higher than this number, the gain will only be
// this number instead.
//
// The third line controls the minimu percent gain in learnedness a
// character is allowed per practice -- in other words, if the random
// die throw comes out below this number, the gain will be set up to
// this number.
// 
// The fourth line simply sets whether the character knows 'spells'
// or 'skills'.  This does not affect anything except the message given
// to the character when trying to practice (i.e. "You know of the
// following spells" vs. "You know of the following skills"
//

#define SPELL	0
#define SKILL	1

// #define LEARNED_LEVEL	0  % known which is considered "learned"
// #define MAX_PER_PRAC		1  max percent gain in skill per practice
// #define MIN_PER_PRAC		2  min percent gain in skill per practice
// #define PRAC_TYPE		3  should it say 'spell' or 'skill'?	

int prac_params[4][NUM_CLASSES] = {
 // MAG		CLE	THE	WAR
  { 95,		95,	95,	95	},	// learned level 
  { 100,	100,	12,	12	},	// max per practice 
  { 25,		25,	0,	0	},	// min per practice 
  { SPELL,	SPELL,	SKILL,	SKILL	},	// prac name 
};


//
// ...And the appropriate rooms for each guildmaster/guildguard; controls
// which types of people the various guildguards let through.  i.e., the
// first line shows that from room 3017, only MAGIC_USERS are allowed
// to go south.
//
// Don't forget to visit spec_assign.c if you create any new mobiles that
// should be a guild master or guard so they can act appropriately. If you
// "recycle" the existing mobs that are used in other guilds for your new
// guild, then you don't have to change that file, only here.
//

struct guild_info_type guild_info[] = {

// Midgaard 
  { CLASS_MAGIC_USER,	3017,	SOUTH	},
  { CLASS_CLERIC,	3004,	NORTH	},
  { CLASS_THIEF,	3027,	EAST	},
  { CLASS_WARRIOR,	3021,	EAST	},
  { CLASS_RANGER,       3021,   EAST       },
  { CLASS_KNIGHT,       3021,   EAST       },
  { CLASS_PALADIN,      3021,   EAST       },
  { CLASS_ROGUE,        3027,   EAST       },
  { CLASS_ASSASSIN,     3027,   EAST       },
  { CLASS_NINJA,        3027,   EAST       },
  { CLASS_MYSTIC,       3024,   NORTH      },
  { CLASS_SHAMAN,       3024,   NORTH      },
  { CLASS_PRIEST,       3024,   NORTH      },
  { CLASS_NECROMANCER,  3017,   SOUTH      },
  { CLASS_SORCERER,     3017,   SOUTH      },
  { CLASS_ELEMENTALIST, 3017,   SOUTH      },
// Brass Dragon 
// { -999 /* all */ ,	5065,	SCMD_WEST	},

// this must go last -- add new guards above! 
  { -1, NOWHERE, -1}
};



//
// Saving throws for:
// MCTW
// PARA, ROD, PETRI, BREATH, SPELL
//     Levels 0-40
//
// Do not forget to change extern declaration in magic.c if you add to this//
 

byte saving_throws(int class_num, int type, int level)
{
  switch (class_num) {
  case CLASS_MAGIC_USER:
  case CLASS_NECROMANCER:
  case CLASS_SORCERER:
  case CLASS_ELEMENTALIST:
    switch (type) {
    case SAVING_PARA:	// Paralyzation 
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 79;
      case  3: return 78;
      case  4: return 77;
      case  5: return 76;
      case  6: return 75;
      case  7: return 73;
      case  8: return 71;
      case  9: return 70;
      case 10: return 69;
      case 11: return 67;
      case 12: return 65;
      case 13: return 64;
      case 14: return 63;
      case 15: return 63;
      case 16: return 62;
      case 17: return 61;
      case 18: return 60;
      case 19: return 58;
      case 20: return 56;
      case 21: return 55;
      case 22: return 54;
      case 23: return 52;
      case 24: return 50;
      case 25: return 48;
      case 26: return 46;
      case 27: return 44;
      case 28: return 42;
      case 29: return 40;
      case 30: return 38;
      case 31: return 30;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for mage paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	// Rods 
      switch (level) {
      case  0: return 90;
      case  1: return 55;
      case  2: return 53;
      case  3: return 51;
      case  4: return 49;
      case  5: return 47;
      case  6: return 45;
      case  7: return 43;
      case  8: return 41;
      case  9: return 40;
      case 10: return 39;
      case 11: return 37;
      case 12: return 35;
      case 13: return 33;
      case 14: return 31;
      case 15: return 30;
      case 16: return 29;
      case 17: return 27;
      case 18: return 25;
      case 19: return 23;
      case 20: return 21;
      case 21: return 20;
      case 22: return 19;
      case 23: return 17;
      case 24: return 15;
      case 25: return 14;
      case 26: return 13;
      case 27: return 12;
      case 28: return 11;
      case 29: return 10;
      case 30: return  9;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for mage rod saving throw.");
	break;
      }
    case SAVING_PETRI:	// Petrification 
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 63;
      case  3: return 61;
      case  4: return 59;
      case  5: return 57;
      case  6: return 55;
      case  7: return 53;
      case  8: return 51;
      case  9: return 50;
      case 10: return 49;
      case 11: return 47;
      case 12: return 45;
      case 13: return 43;
      case 14: return 41;
      case 15: return 40;
      case 16: return 39;
      case 17: return 37;
      case 18: return 35;
      case 19: return 33;
      case 20: return 31;
      case 21: return 30;
      case 22: return 29;
      case 23: return 27;
      case 24: return 25;
      case 25: return 23;
      case 26: return 21;
      case 27: return 19;
      case 28: return 17;
      case 29: return 15;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	// Breath weapons 
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 53;
      case 14: return 51;
      case 15: return 50;
      case 16: return 49;
      case 17: return 47;
      case 18: return 45;
      case 19: return 43;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 37;
      case 24: return 35;
      case 25: return 33;
      case 26: return 31;
      case 27: return 29;
      case 28: return 27;
      case 29: return 25;
      case 30: return 23;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for mage breath saving throw.");
	break;
      }
    case SAVING_SPELL:	// Generic spells 
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 68;
      case  3: return 66;
      case  4: return 64;
      case  5: return 62;
      case  6: return 60;
      case  7: return 58;
      case  8: return 56;
      case  9: return 55;
      case 10: return 54;
      case 11: return 52;
      case 12: return 50;
      case 13: return 48;
      case 14: return 46;
      case 15: return 45;
      case 16: return 44;
      case 17: return 42;
      case 18: return 40;
      case 19: return 38;
      case 20: return 36;
      case 21: return 35;
      case 22: return 34;
      case 23: return 32;
      case 24: return 30;
      case 25: return 28;
      case 26: return 26;
      case 27: return 24;
      case 28: return 22;
      case 29: return 20;
      case 30: return 18;
      case 31: return 16;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for mage spell saving throw.");
	break;
      }
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
    break;
  case CLASS_CLERIC:
  case CLASS_MYSTIC:
  case CLASS_PRIEST:
  case CLASS_SHAMAN:
    switch (type) {
    case SAVING_PARA:	// Paralyzation 
      switch (level) {
      case  0: return 90;
      case  1: return 60;
      case  2: return 59;
      case  3: return 48;
      case  4: return 46;
      case  5: return 45;
      case  6: return 43;
      case  7: return 40;
      case  8: return 37;
      case  9: return 35;
      case 10: return 34;
      case 11: return 33;
      case 12: return 31;
      case 13: return 30;
      case 14: return 29;
      case 15: return 27;
      case 16: return 26;
      case 17: return 25;
      case 18: return 24;
      case 19: return 23;
      case 20: return 22;
      case 21: return 21;
      case 22: return 20;
      case 23: return 18;
      case 24: return 15;
      case 25: return 14;
      case 26: return 12;
      case 27: return 10;
      case 28: return  9;
      case 29: return  8;
      case 30: return  7;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for cleric paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	// Rods 
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 66;
      case  5: return 65;
      case  6: return 63;
      case  7: return 60;
      case  8: return 57;
      case  9: return 55;
      case 10: return 54;
      case 11: return 53;
      case 12: return 51;
      case 13: return 50;
      case 14: return 49;
      case 15: return 47;
      case 16: return 46;
      case 17: return 45;
      case 18: return 44;
      case 19: return 43;
      case 20: return 42;
      case 21: return 41;
      case 22: return 40;
      case 23: return 38;
      case 24: return 35;
      case 25: return 34;
      case 26: return 32;
      case 27: return 30;
      case 28: return 29;
      case 29: return 28;
      case 30: return 27;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for cleric rod saving throw.");
	break;
      }
    case SAVING_PETRI:	// Petrification
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 64;
      case  3: return 63;
      case  4: return 61;
      case  5: return 60;
      case  6: return 58;
      case  7: return 55;
      case  8: return 53;
      case  9: return 50;
      case 10: return 49;
      case 11: return 48;
      case 12: return 46;
      case 13: return 45;
      case 14: return 44;
      case 15: return 43;
      case 16: return 41;
      case 17: return 40;
      case 18: return 39;
      case 19: return 38;
      case 20: return 37;
      case 21: return 36;
      case 22: return 35;
      case 23: return 33;
      case 24: return 31;
      case 25: return 29;
      case 26: return 27;
      case 27: return 25;
      case 28: return 24;
      case 29: return 23;
      case 30: return 22;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for cleric petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	// Breath weapons 
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 79;
      case  3: return 78;
      case  4: return 76;
      case  5: return 75;
      case  6: return 73;
      case  7: return 70;
      case  8: return 67;
      case  9: return 65;
      case 10: return 64;
      case 11: return 63;
      case 12: return 61;
      case 13: return 60;
      case 14: return 59;
      case 15: return 57;
      case 16: return 56;
      case 17: return 55;
      case 18: return 54;
      case 19: return 53;
      case 20: return 52;
      case 21: return 51;
      case 22: return 50;
      case 23: return 48;
      case 24: return 45;
      case 25: return 44;
      case 26: return 42;
      case 27: return 40;
      case 28: return 39;
      case 29: return 38;
      case 30: return 37;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for cleric breath saving throw.");
	break;
      }
    case SAVING_SPELL:	// Generic spells 
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 74;
      case  3: return 73;
      case  4: return 71;
      case  5: return 70;
      case  6: return 68;
      case  7: return 65;
      case  8: return 63;
      case  9: return 60;
      case 10: return 59;
      case 11: return 58;
      case 12: return 56;
      case 13: return 55;
      case 14: return 54;
      case 15: return 53;
      case 16: return 51;
      case 17: return 50;
      case 18: return 49;
      case 19: return 48;
      case 20: return 47;
      case 21: return 46;
      case 22: return 45;
      case 23: return 43;
      case 24: return 41;
      case 25: return 39;
      case 26: return 37;
      case 27: return 35;
      case 28: return 34;
      case 29: return 33;
      case 30: return 32;
      case 31: return 30;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for cleric spell saving throw.");
	break;
      }
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
    break;
  case CLASS_THIEF:
  case CLASS_NINJA:
  case CLASS_ROGUE:
  case CLASS_ASSASSIN:
    switch (type) {
    case SAVING_PARA:	// Paralyzation 
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 64;
      case  3: return 63;
      case  4: return 62;
      case  5: return 61;
      case  6: return 60;
      case  7: return 59;
      case  8: return 58;
      case  9: return 57;
      case 10: return 56;
      case 11: return 55;
      case 12: return 54;
      case 13: return 53;
      case 14: return 52;
      case 15: return 51;
      case 16: return 50;
      case 17: return 49;
      case 18: return 48;
      case 19: return 47;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 43;
      case 24: return 42;
      case 25: return 41;
      case 26: return 40;
      case 27: return 39;
      case 28: return 38;
      case 29: return 37;
      case 30: return 36;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for thief paralyzation saving throw.");
	break;
      }
    case SAVING_ROD:	// Rods 
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 68;
      case  3: return 66;
      case  4: return 64;
      case  5: return 62;
      case  6: return 60;
      case  7: return 58;
      case  8: return 56;
      case  9: return 54;
      case 10: return 52;
      case 11: return 50;
      case 12: return 48;
      case 13: return 46;
      case 14: return 44;
      case 15: return 42;
      case 16: return 40;
      case 17: return 38;
      case 18: return 36;
      case 19: return 34;
      case 20: return 32;
      case 21: return 30;
      case 22: return 28;
      case 23: return 26;
      case 24: return 24;
      case 25: return 22;
      case 26: return 20;
      case 27: return 18;
      case 28: return 16;
      case 29: return 14;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for thief rod saving throw.");
	break;
      }
    case SAVING_PETRI:	// Petrification 
      switch (level) {
      case  0: return 90;
      case  1: return 60;
      case  2: return 59;
      case  3: return 58;
      case  4: return 58;
      case  5: return 56;
      case  6: return 55;
      case  7: return 54;
      case  8: return 53;
      case  9: return 52;
      case 10: return 51;
      case 11: return 50;
      case 12: return 49;
      case 13: return 48;
      case 14: return 47;
      case 15: return 46;
      case 16: return 45;
      case 17: return 44;
      case 18: return 43;
      case 19: return 42;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 38;
      case 24: return 37;
      case 25: return 36;
      case 26: return 35;
      case 27: return 34;
      case 28: return 33;
      case 29: return 32;
      case 30: return 31;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for thief petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	// Breath weapons 
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 79;
      case  3: return 78;
      case  4: return 77;
      case  5: return 76;
      case  6: return 75;
      case  7: return 74;
      case  8: return 73;
      case  9: return 72;
      case 10: return 71;
      case 11: return 70;
      case 12: return 69;
      case 13: return 68;
      case 14: return 67;
      case 15: return 66;
      case 16: return 65;
      case 17: return 64;
      case 18: return 63;
      case 19: return 62;
      case 20: return 61;
      case 21: return 60;
      case 22: return 59;
      case 23: return 58;
      case 24: return 57;
      case 25: return 56;
      case 26: return 55;
      case 27: return 54;
      case 28: return 53;
      case 29: return 52;
      case 30: return 51;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief breath saving throw.");
	break;
      }
    case SAVING_SPELL:	// Generic spells 
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 59;
      case 10: return 57;
      case 11: return 55;
      case 12: return 53;
      case 13: return 51;
      case 14: return 49;
      case 15: return 47;
      case 16: return 45;
      case 17: return 43;
      case 18: return 41;
      case 19: return 39;
      case 20: return 37;
      case 21: return 35;
      case 22: return 33;
      case 23: return 31;
      case 24: return 29;
      case 25: return 27;
      case 26: return 25;
      case 27: return 23;
      case 28: return 21;
      case 29: return 19;
      case 30: return 17;
      case 31: return 15;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      case 41: return  0;
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for thief spell saving throw.");
	break;
      }
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
    break;
  case CLASS_WARRIOR:
  case CLASS_RANGER:
  case CLASS_KNIGHT:
  case CLASS_PALADIN:
    switch (type) {
    case SAVING_PARA:	// Paralyzation 
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 68;
      case  3: return 67;
      case  4: return 65;
      case  5: return 62;
      case  6: return 58;
      case  7: return 55;
      case  8: return 53;
      case  9: return 52;
      case 10: return 50;
      case 11: return 47;
      case 12: return 43;
      case 13: return 40;
      case 14: return 38;
      case 15: return 37;
      case 16: return 35;
      case 17: return 32;
      case 18: return 28;
      case 19: return 25;
      case 20: return 24;
      case 21: return 23;
      case 22: return 22;
      case 23: return 20;
      case 24: return 19;
      case 25: return 17;
      case 26: return 16;
      case 27: return 15;
      case 28: return 14;
      case 29: return 13;
      case 30: return 12;
      case 31: return 11;
      case 32: return 10;
      case 33: return  9;
      case 34: return  8;
      case 35: return  7;
      case 36: return  6;
      case 37: return  5;
      case 38: return  4;
      case 39: return  3;
      case 40: return  2;
      case 41: return  1;	// Some mobiles. 
      case 42: return  0;
      case 43: return  0;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior paralyzation saving throw.");
	break;	
      }
    case SAVING_ROD:	// Rods 
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 78;
      case  3: return 77;
      case  4: return 75;
      case  5: return 72;
      case  6: return 68;
      case  7: return 65;
      case  8: return 63;
      case  9: return 62;
      case 10: return 60;
      case 11: return 57;
      case 12: return 53;
      case 13: return 50;
      case 14: return 48;
      case 15: return 47;
      case 16: return 45;
      case 17: return 42;
      case 18: return 38;
      case 19: return 35;
      case 20: return 34;
      case 21: return 33;
      case 22: return 32;
      case 23: return 30;
      case 24: return 29;
      case 25: return 27;
      case 26: return 26;
      case 27: return 25;
      case 28: return 24;
      case 29: return 23;
      case 30: return 22;
      case 31: return 20;
      case 32: return 18;
      case 33: return 16;
      case 34: return 14;
      case 35: return 12;
      case 36: return 10;
      case 37: return  8;
      case 38: return  6;
      case 39: return  5;
      case 40: return  4;
      case 41: return  3;
      case 42: return  2;
      case 43: return  1;
      case 44: return  0;
      case 45: return  0;
      case 46: return  0;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior rod saving throw.");
	break;
      }
    case SAVING_PETRI:	// Petrification 
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 72;
      case  4: return 70;
      case  5: return 67;
      case  6: return 63;
      case  7: return 60;
      case  8: return 58;
      case  9: return 57;
      case 10: return 55;
      case 11: return 52;
      case 12: return 48;
      case 13: return 45;
      case 14: return 43;
      case 15: return 42;
      case 16: return 40;
      case 17: return 37;
      case 18: return 33;
      case 19: return 30;
      case 20: return 29;
      case 21: return 28;
      case 22: return 26;
      case 23: return 25;
      case 24: return 24;
      case 25: return 23;
      case 26: return 21;
      case 27: return 20;
      case 28: return 19;
      case 29: return 18;
      case 30: return 17;
      case 31: return 16;
      case 32: return 15;
      case 33: return 14;
      case 34: return 13;
      case 35: return 12;
      case 36: return 11;
      case 37: return 10;
      case 38: return  9;
      case 39: return  8;
      case 40: return  7;
      case 41: return  6;
      case 42: return  5;
      case 43: return  4;
      case 44: return  3;
      case 45: return  2;
      case 46: return  1;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior petrification saving throw.");
	break;
      }
    case SAVING_BREATH:	// Breath weapons 
      switch (level) {
      case  0: return 90;
      case  1: return 85;
      case  2: return 83;
      case  3: return 82;
      case  4: return 80;
      case  5: return 75;
      case  6: return 70;
      case  7: return 65;
      case  8: return 63;
      case  9: return 62;
      case 10: return 60;
      case 11: return 55;
      case 12: return 50;
      case 13: return 45;
      case 14: return 43;
      case 15: return 42;
      case 16: return 40;
      case 17: return 37;
      case 18: return 33;
      case 19: return 30;
      case 20: return 29;
      case 21: return 28;
      case 22: return 26;
      case 23: return 25;
      case 24: return 24;
      case 25: return 23;
      case 26: return 21;
      case 27: return 20;
      case 28: return 19;
      case 29: return 18;
      case 30: return 17;
      case 31: return 16;
      case 32: return 15;
      case 33: return 14;
      case 34: return 13;
      case 35: return 12;
      case 36: return 11;
      case 37: return 10;
      case 38: return  9;
      case 39: return  8;
      case 40: return  7;
      case 41: return  6;
      case 42: return  5;
      case 43: return  4;
      case 44: return  3;
      case 45: return  2;
      case 46: return  1;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior breath saving throw.");
	break;
      }
    case SAVING_SPELL:	// Generic spells 
      switch (level) {
      case  0: return 90;
      case  1: return 85;
      case  2: return 83;
      case  3: return 82;
      case  4: return 80;
      case  5: return 77;
      case  6: return 73;
      case  7: return 70;
      case  8: return 68;
      case  9: return 67;
      case 10: return 65;
      case 11: return 62;
      case 12: return 58;
      case 13: return 55;
      case 14: return 53;
      case 15: return 52;
      case 16: return 50;
      case 17: return 47;
      case 18: return 43;
      case 19: return 40;
      case 20: return 39;
      case 21: return 38;
      case 22: return 36;
      case 23: return 35;
      case 24: return 34;
      case 25: return 33;
      case 26: return 31;
      case 27: return 30;
      case 28: return 29;
      case 29: return 28;
      case 30: return 27;
      case 31: return 25;
      case 32: return 23;
      case 33: return 21;
      case 34: return 19;
      case 35: return 17;
      case 36: return 15;
      case 37: return 13;
      case 38: return 11;
      case 39: return  9;
      case 40: return  7;
      case 41: return  6;
      case 42: return  5;
      case 43: return  4;
      case 44: return  3;
      case 45: return  2;
      case 46: return  1;
      case 47: return  0;
      case 48: return  0;
      case 49: return  0;
      case 50: return  0;
      default:
	log("SYSERR: Missing level for warrior spell saving throw.");
	break;
      }
    default:
      log("SYSERR: Invalid saving throw type.");
      break;
    }
  default:
    log("SYSERR: Invalid class saving throw.");
    break;
  }

  // Should not get here unless something is wrong. 
  return 100;
}

// THAC0 for classes and levels.  (To Hit Armor Class 0) 
int thaco(int class_num, int level)
{
  switch (class_num) {
  case CLASS_MAGIC_USER:
  case CLASS_SORCERER:
  case CLASS_NECROMANCER:
  case CLASS_ELEMENTALIST:
    switch (level) {
    case  0: return 100;
    case  1: return  20;
    case  2: return  20;
    case  3: return  20;
    case  4: return  19;
    case  5: return  19;
    case  6: return  19;
    case  7: return  18;
    case  8: return  18;
    case  9: return  18;
    case 10: return  17;
    case 11: return  17;
    case 12: return  17;
    case 13: return  16;
    case 14: return  16;
    case 15: return  16;
    case 16: return  15;
    case 17: return  15;
    case 18: return  15;
    case 19: return  14;
    case 20: return  14;
    case 21: return  14;
    case 22: return  13;
    case 23: return  13;
    case 24: return  13;
    case 25: return  12;
    case 26: return  12;
    case 27: return  12;
    case 28: return  11;
    case 29: return  11;
    case 30: return  11;
    case 31: return  10;
    case 32: return  10;
    case 33: return  10;
    case 34: return   9;
    case 35: return   8;
    case 36: return   7;
    case 37: return   6;
    case 38: return   5;
    case 39: return   4;
    case 40: return   3;
    case 41: return   2;
    case 42: return   1;
    case 43: return   1;
    case 44: return   1;
    case 45: return   1;
    case 46: return   1;
    case 47: return   1;
    case 48: return   1;
    case 49: return   1;
    case 50: return   1;
    default:
      log("SYSERR: Missing level for mage thac0.");
    }
  case CLASS_CLERIC:
  case CLASS_SHAMAN:
  case CLASS_PRIEST:
  case CLASS_MYSTIC:
    switch (level) {
    case  0: return 100;
    case  1: return  20;
    case  2: return  20;
    case  3: return  20;
    case  4: return  18;
    case  5: return  18;
    case  6: return  18;
    case  7: return  16;
    case  8: return  16;
    case  9: return  16;
    case 10: return  14;
    case 11: return  14;
    case 12: return  14;
    case 13: return  12;
    case 14: return  12;
    case 15: return  12;
    case 16: return  10;
    case 17: return  10;
    case 18: return  10;
    case 19: return   8;
    case 20: return   8;
    case 21: return   8;
    case 22: return   6;
    case 23: return   6;
    case 24: return   6;
    case 25: return   4;
    case 26: return   4;
    case 27: return   4;
    case 28: return   2;
    case 29: return   2;
    case 30: return   2;
    case 31: return   1;
    case 32: return   1;
    case 33: return   1;
    case 34: return   1;
    case 35: return   1;
    case 36: return   1;
    case 37: return   1;
    case 38: return   1;
    case 39: return   1;
    case 40: return   1;
    case 41: return   1;
    case 42: return   1;
    case 43: return   1;
    case 44: return   1;
    case 45: return   1;
    case 46: return   1;
    case 47: return   1;
    case 48: return   1;
    case 49: return   1;
    case 50: return   1;
    default:
      log("SYSERR: Missing level for cleric thac0.");
    }
  case CLASS_THIEF:
  case CLASS_ROGUE:
  case CLASS_NINJA:
  case CLASS_ASSASSIN:
    switch (level) {
    case  0: return 100;
    case  1: return  20;
    case  2: return  20;
    case  3: return  19;
    case  4: return  19;
    case  5: return  18;
    case  6: return  18;
    case  7: return  17;
    case  8: return  17;
    case  9: return  16;
    case 10: return  16;
    case 11: return  15;
    case 12: return  15;
    case 13: return  14;
    case 14: return  14;
    case 15: return  13;
    case 16: return  13;
    case 17: return  12;
    case 18: return  12;
    case 19: return  11;
    case 20: return  11;
    case 21: return  10;
    case 22: return  10;
    case 23: return   9;
    case 24: return   9;
    case 25: return   8;
    case 26: return   8;
    case 27: return   7;
    case 28: return   7;
    case 29: return   6;
    case 30: return   6;
    case 31: return   5;
    case 32: return   5;
    case 33: return   4;
    case 34: return   4;
    case 35: return   3;
    case 36: return   3;
    case 37: return   3;
    case 38: return   2;
    case 39: return   2;
    case 40: return   1;
    case 41: return   1;
    case 42: return   1;
    case 43: return   1;
    case 44: return   1;
    case 45: return   1;
    case 46: return   1;
    case 47: return   1;
    case 48: return   1;
    case 49: return   1;
    case 50: return   1;

      log("SYSERR: Missing level for thief thac0.");
    }
  case CLASS_WARRIOR:
  case CLASS_RANGER:
  case CLASS_KNIGHT:
  case CLASS_PALADIN:
    switch (level) {
    case  0: return 100;
    case  1: return  20;
    case  2: return  19;
    case  3: return  18;
    case  4: return  17;
    case  5: return  16;
    case  6: return  15;
    case  7: return  14;
    case  8: return  14;
    case  9: return  13;
    case 10: return  12;
    case 11: return  11;
    case 12: return  10;
    case 13: return   9;
    case 14: return   8;
    case 15: return   7;
    case 16: return   6;
    case 17: return   5;
    case 18: return   4;
    case 19: return   3;
    case 20: return   2;
    case 21: return   1;
    case 22: return   1;
    case 23: return   1;
    case 24: return   1;
    case 25: return   1;
    case 26: return   1;
    case 27: return   1;
    case 28: return   1;
    case 29: return   1;
    case 30: return   1;
    case 31: return   1;
    case 32: return   1;
    case 33: return   1;
    case 34: return   1;
    case 35: return   1;
    case 36: return   1;
    case 37: return   1;
    case 38: return   1;
    case 39: return   1;
    case 40: return   1;
    case 41: return   1;
    case 42: return   1;
    case 43: return   1;
    case 44: return   1;
    case 45: return   1;
    case 46: return   1;
    case 47: return   1;
    case 48: return   1;
    case 49: return   1;
    case 50: return   1;
    default:
      log("SYSERR: Missing level for warrior thac0.");
    }
  default:
    log("SYSERR: Unknown class in thac0 chart.");
  }

  // Will not get there unless something is wrong. 
  return 100;
}


//
// Roll the 6 stats for a character... each stat is made of the sum of
// the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
// which priority will be given for the best to worst stats.
//

void roll_real_abils(struct char_data *ch)
{
  int i, j, k, temp;
  ubyte table[6];
  ubyte rolls[4];

  for (i = 0; i < 6; i++)
    table[i] = 0;

  for (i = 0; i < 6; i++) {

    for (j = 0; j < 4; j++)
      rolls[j] = rand_number(3, 6);

    temp = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
      MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));

    for (k = 0; k < 6; k++)
      if (table[k] < temp) {
	temp ^= table[k];
	table[k] ^= temp;
	temp ^= table[k];
      }
  }

  ch->real_abils.str_add = 0;

  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
    ch->real_abils.intel = 16;
    ch->real_abils.wis = 14;
    ch->real_abils.dex = 14;
    ch->real_abils.str = 14;
    ch->real_abils.con = 14;
    ch->real_abils.cha = 16;
    break;
  case CLASS_CLERIC:
    ch->real_abils.wis = 16;
    ch->real_abils.intel = 14;
    ch->real_abils.str = 14;
    ch->real_abils.dex = 14;
    ch->real_abils.con = 14;
    ch->real_abils.cha = 16;
    break;
  case CLASS_THIEF:
    ch->real_abils.dex = 16;
    ch->real_abils.str = 14;
    ch->real_abils.con = 14;
    ch->real_abils.intel = 14;
    ch->real_abils.wis = 14;
    ch->real_abils.cha = 16;
    break;
  case CLASS_WARRIOR:
    ch->real_abils.str = 16;
    ch->real_abils.dex = 14;
    ch->real_abils.con = 14;
    ch->real_abils.wis = 14;
    ch->real_abils.intel = 14;
    ch->real_abils.cha = 16;
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = rand_number(0, 100);
    break;
  }
  ch->aff_abils = ch->real_abils;

  GET_HITROLL(ch) = 5;
  GET_DAMROLL(ch) = 4;
}

int do_multi(struct char_data *ch, int class)
{
  if(GET_CLAN(ch) == 100)
    send_to_char(ch, "&RYou are now being removed from the newbie clan.");

  GET_CLASS(ch) = class;
  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;

  set_title(ch, NULL);

  GET_MAX_HIT(ch)  = 50 + (GET_META_HP(ch)/5);
  GET_META_HP(ch) = 0;
  GET_MAX_MANA(ch) = 50 + (GET_META_MANA(ch)/5);
  GET_META_MANA(ch) = 0;
  GET_MAX_MOVE(ch) = 50 + (GET_META_MV(ch)/5);
  GET_META_MV(ch) = 0;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    break;

  case CLASS_CLERIC:
    break;

  case CLASS_THIEF:
    if ((GET_WAS_THIEF(ch) == 0))
    {
    SET_SKILL(ch, SKILL_SNEAK, 10);
    SET_SKILL(ch, SKILL_HIDE, 5);
    SET_SKILL(ch, SKILL_STEAL, 15);
    SET_SKILL(ch, SKILL_BACKSTAB, 10);
    SET_SKILL(ch, SKILL_PICK_LOCK, 10);
    SET_SKILL(ch, SKILL_TRACK, 10);
    SET_SKILL(ch, SKILL_SECOND_STAB, 0);
    SET_SKILL(ch, SKILL_ATTACK2, 0);
    SET_SKILL(ch, SKILL_SCAN, 5);
    SET_SKILL(ch, SKILL_DODGE, 10);
    }
    else
    {
    SET_SKILL(ch, SKILL_SNEAK, GET_SKILL(ch, SKILL_SNEAK));
    SET_SKILL(ch, SKILL_HIDE, GET_SKILL(ch, SKILL_HIDE));
    SET_SKILL(ch, SKILL_STEAL, GET_SKILL(ch, SKILL_STEAL));
    SET_SKILL(ch, SKILL_BACKSTAB, GET_SKILL(ch, SKILL_BACKSTAB));
    SET_SKILL(ch, SKILL_PICK_LOCK, GET_SKILL(ch, SKILL_PICK_LOCK));
    SET_SKILL(ch, SKILL_TRACK, GET_SKILL(ch, SKILL_TRACK));
    SET_SKILL(ch, SKILL_SECOND_STAB, GET_SKILL(ch, SKILL_SECOND_STAB));
    SET_SKILL(ch, SKILL_ATTACK2, GET_SKILL(ch, SKILL_ATTACK2));
    SET_SKILL(ch, SKILL_SCAN, GET_SKILL(ch, SKILL_SCAN));
    SET_SKILL(ch, SKILL_DODGE, GET_SKILL(ch, SKILL_DODGE));
    }
    break;

  case CLASS_WARRIOR:
    if ((GET_WAS_WARRIOR(ch) == 0))
    {
    SET_SKILL(ch, SKILL_KICK, 10);
    SET_SKILL(ch, SKILL_RESCUE, 5);
    SET_SKILL(ch, SKILL_BASH, 15);
    SET_SKILL(ch, SKILL_TRACK, 10);
    SET_SKILL(ch, SKILL_ATTACK2, 5);
    SET_SKILL(ch, SKILL_ATTACK3, 0);
    SET_SKILL(ch, SKILL_DISARM, 5);
    SET_SKILL(ch, SKILL_PARRY, 5);
    SET_SKILL(ch, SKILL_DUAL_WIELD, 10);
    }
    else
    {
    SET_SKILL(ch, SKILL_CHARGE,  GET_SKILL(ch, SKILL_CHARGE));
    SET_SKILL(ch, SKILL_PARRY, GET_SKILL(ch, SKILL_PARRY));
    SET_SKILL(ch, SKILL_KICK, GET_SKILL(ch, SKILL_KICK));
    SET_SKILL(ch, SKILL_BASH, GET_SKILL(ch, SKILL_BASH));
    SET_SKILL(ch, SKILL_TRACK, GET_SKILL(ch, SKILL_TRACK));
    SET_SKILL(ch, SKILL_ATTACK2, GET_SKILL(ch, SKILL_ATTACK2));
    SET_SKILL(ch, SKILL_RESCUE, GET_SKILL(ch, SKILL_RESCUE));
    SET_SKILL(ch, SKILL_DISARM, GET_SKILL(ch, SKILL_DISARM));
    SET_SKILL(ch, SKILL_DUAL_WIELD, GET_SKILL(ch, SKILL_DUAL_WIELD));
    }
    break;

  case CLASS_RANGER:    
    if ((GET_WAS_RANGER(ch) == 0))
    {
    SET_SKILL(ch, SKILL_RANGE_COMBAT, 20);
    SET_SKILL(ch, SKILL_REVEAL, 15);
    SET_SKILL(ch, SKILL_ATTACK4, 20);
    SET_SKILL(ch, SKILL_SENSE, 10);
    SET_SKILL(ch, SKILL_ADVANCED_MEDITATE, 15);
    SET_SKILL(ch, SKILL_SUMMON_MOUNT, 10);
    SET_SKILL(ch, SKILL_GREATER_SENSES, 10);
    }
    else
    {
    SET_SKILL(ch, SKILL_RANGE_COMBAT, GET_SKILL(ch, SKILL_RANGE_COMBAT));
    SET_SKILL(ch, SKILL_REVEAL,  GET_SKILL(ch, SKILL_REVEAL));
    SET_SKILL(ch, SKILL_ATTACK4,  GET_SKILL(ch, SKILL_ATTACK4));
    SET_SKILL(ch, SKILL_SENSE,  GET_SKILL(ch, SKILL_SENSE));
    SET_SKILL(ch, SKILL_ADVANCED_MEDITATE,  GET_SKILL(ch, SKILL_ADVANCED_MEDITATE));
    SET_SKILL(ch, SKILL_SUMMON_MOUNT,  GET_SKILL(ch, SKILL_SUMMON_MOUNT));
    SET_SKILL(ch, SKILL_GREATER_SENSES,  GET_SKILL(ch, SKILL_GREATER_SENSES));
    }
    break;
  
  case CLASS_KNIGHT:
    if ((GET_WAS_KNIGHT(ch) == 0))
    {
    SET_SKILL(ch, SKILL_ADVANCED_KICK, 15);
    SET_SKILL(ch, SKILL_ATTACK4, 20);
    SET_SKILL(ch, SKILL_ATTACK5, 10);
    SET_SKILL(ch, SKILL_ADVANCED_PARRY, 15);
    SET_SKILL(ch, SKILL_ADVANCED_CHARGE, 10);
    SET_SKILL(ch, SKILL_RESCUE, 20);
    SET_SKILL(ch, SPELL_HEROISM, 15);
    SET_SKILL(ch, SKILL_STOMP, 5);
    SET_SKILL(ch, SKILL_DEFEND, 5);
    SET_SKILL(ch, SKILL_DEATHWISH, 5);
    }
    else
    {
    SET_SKILL(ch, SKILL_ADVANCED_KICK, GET_SKILL(ch, SKILL_ADVANCED_KICK));
    SET_SKILL(ch, SKILL_ATTACK4, GET_SKILL(ch, SKILL_ATTACK4));
    SET_SKILL(ch, SKILL_ATTACK5, GET_SKILL(ch, SKILL_ATTACK5));
    SET_SKILL(ch, SKILL_ADVANCED_PARRY, GET_SKILL(ch, SKILL_ADVANCED_PARRY));
    SET_SKILL(ch, SKILL_ADVANCED_CHARGE, GET_SKILL(ch, SKILL_ADVANCED_CHARGE));
    SET_SKILL(ch, SKILL_RESCUE, GET_SKILL(ch, SKILL_RESCUE));
    SET_SKILL(ch, SPELL_HEROISM, GET_SKILL(ch, SPELL_HEROISM));
    SET_SKILL(ch, SKILL_STOMP, GET_SKILL(ch, SKILL_STOMP));
    SET_SKILL(ch, SKILL_DEFEND, GET_SKILL(ch, SKILL_DEFEND));
    SET_SKILL(ch, SKILL_DEATHWISH, GET_SKILL(ch, SKILL_DEATHWISH));
    }
    break;


  case CLASS_PALADIN:
    if ((GET_WAS_PALADIN(ch) == 0))
    {
    SET_SKILL(ch, SPELL_VENGEANCE, 10);
    SET_SKILL(ch, SKILL_ATTACK4, 15);
    SET_SKILL(ch, SPELL_DIVINE_PROT, 10);
    SET_SKILL(ch, SPELL_COURAGE, 20);
    SET_SKILL(ch, SKILL_ADVANCED_BASH, 20);
    }
    else
    {
    SET_SKILL(ch, SPELL_VENGEANCE, GET_SKILL(ch, SPELL_VENGEANCE));
    SET_SKILL(ch, SKILL_ATTACK4, GET_SKILL(ch, SKILL_ATTACK4));
    SET_SKILL(ch, SPELL_DIVINE_PROT, GET_SKILL(ch, SPELL_DIVINE_PROT));
    SET_SKILL(ch, SPELL_COURAGE, GET_SKILL(ch, SPELL_COURAGE));
    SET_SKILL(ch, SKILL_ADVANCED_BASH, GET_SKILL(ch, SKILL_ADVANCED_BASH));
    }
    break;

  case CLASS_MYSTIC:
  case CLASS_SHAMAN:
  case CLASS_PRIEST:
    break;

  case CLASS_NINJA:
    if ((GET_WAS_NINJA(ch) == 0))
    {
    SET_SKILL(ch, SKILL_ADVANCED_HIDE, 20);
    SET_SKILL(ch, SKILL_ADVANCED_DODGE, 15);
    SET_SKILL(ch, SKILL_ADVANCED_THROW, 20);
    SET_SKILL(ch, SKILL_ADVANCED_TRIP, 10);
    }
    else
    {
    SET_SKILL(ch, SKILL_ADVANCED_HIDE, GET_SKILL(ch, SKILL_ADVANCED_HIDE));
    SET_SKILL(ch, SKILL_ADVANCED_DODGE, GET_SKILL(ch, SKILL_ADVANCED_DODGE));
    SET_SKILL(ch, SKILL_ADVANCED_THROW, GET_SKILL(ch, SKILL_ADVANCED_THROW));
    SET_SKILL(ch, SKILL_ADVANCED_TRIP, GET_SKILL(ch, SKILL_ADVANCED_TRIP));
    }
    break;

  case CLASS_ROGUE:
    if ((GET_WAS_ROGUE(ch) == 0))
    {
    SET_SKILL(ch, SKILL_ADVANCED_STEAL, 15);
    SET_SKILL(ch, SKILL_ATTACK3, 20);
    SET_SKILL(ch, SKILL_ADVANCED_TRIP, 10);
    SET_SKILL(ch, SKILL_TRIPWIRE, 15);
    SET_SKILL(ch, SKILL_DUAL_THROW, 10);
    }
    else
    {
    SET_SKILL(ch, SKILL_ADVANCED_STEAL, GET_SKILL(ch, SKILL_ADVANCED_STEAL));
    SET_SKILL(ch, SKILL_ATTACK3, GET_SKILL(ch, SKILL_ATTACK3));
    SET_SKILL(ch, SKILL_ADVANCED_TRIP, GET_SKILL(ch, SKILL_ADVANCED_TRIP));
    SET_SKILL(ch, SKILL_TRIPWIRE, GET_SKILL(ch, SKILL_TRIPWIRE));
    SET_SKILL(ch, SKILL_DUAL_THROW, GET_SKILL(ch, SKILL_DUAL_THROW));
    }
    break;

  case CLASS_ASSASSIN:
    if ((GET_WAS_ASSASSIN(ch) == 0))
    {
    SET_SKILL(ch, SKILL_ADVANCED_BACKSTAB, 20);
    SET_SKILL(ch, SKILL_ADVANCED_TRACK, 15);
    SET_SKILL(ch, SKILL_ATTACK3, 20);
    SET_SKILL(ch, SKILL_AMBUSH, 10);
    SET_SKILL(ch, SKILL_ADVANCED_DODGE, 15);
    SET_SKILL(ch, SKILL_ADVANCED_HIDE, 10);
    }
    else
    {
    SET_SKILL(ch, SKILL_ADVANCED_BACKSTAB, GET_SKILL(ch, SKILL_ADVANCED_BACKSTAB));
    SET_SKILL(ch, SKILL_ADVANCED_TRACK, GET_SKILL(ch, SKILL_ADVANCED_TRACK));
    SET_SKILL(ch, SKILL_ATTACK3, GET_SKILL(ch, SKILL_ATTACK3));
    SET_SKILL(ch, SKILL_AMBUSH, GET_SKILL(ch, SKILL_AMBUSH));
    SET_SKILL(ch, SKILL_ADVANCED_DODGE, GET_SKILL(ch, SKILL_ADVANCED_DODGE));
    SET_SKILL(ch, SKILL_ADVANCED_HIDE, GET_SKILL(ch, SKILL_ADVANCED_HIDE));
    }
    break;


  case CLASS_NECROMANCER:
  case CLASS_ELEMENTALIST:
  case CLASS_SORCERER:
    break;


  }

  send_to_char(ch, "\r\n"); 
//  advance_level(ch);
  nmudlog(INFO_LOG, LVL_GOD, TRUE, "%s reclassed to %s", GET_NAME(ch), pc_class_types[(int)GET_CLASS(ch)]);
//  mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s reclassed to %s", GET_NAME(ch),pc_class_types[(int)GET_CLASS(ch)]);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;

  if (CONFIG_SITEOK_ALL)
    SET_BIT_AR(PLR_FLAGS(ch), PLR_SITEOK);


  ch->player_specials->saved.olc_zone = NOWHERE;
 return(class);
}


// Some initializations for characters, including initial skills 
void do_start(struct char_data *ch)
{
  
  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;
  GET_SECURITY(ch) = 1;
  GET_ETHOS(ch) = 2;
  GET_DEITY(ch) = 0;
 
  set_title(ch, NULL);
  roll_real_abils(ch);

  GET_MAX_HIT(ch)  = 50;
  GET_MAX_MANA(ch) = 50;
  GET_MAX_MOVE(ch) = 50;

  SET_SKILL(ch, SKILL_MOUNT, 10);
  SET_SKILL(ch, SKILL_RIDING, 15);
  SET_SKILL(ch, SKILL_TAME, 10);

  GET_CLAN(ch) = 100;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    GET_WAS_MAGE(ch) = 1;
    break;

  case CLASS_CLERIC:
    GET_WAS_CLERIC(ch) = 1;
    break;

  case CLASS_THIEF:
    SET_SKILL(ch, SKILL_SNEAK, 10);
    SET_SKILL(ch, SKILL_HIDE, 5);
    SET_SKILL(ch, SKILL_STEAL, 15);
    SET_SKILL(ch, SKILL_BACKSTAB, 10);
    SET_SKILL(ch, SKILL_PICK_LOCK, 10);
    SET_SKILL(ch, SKILL_TRACK, 10);
    SET_SKILL(ch, SKILL_SECOND_STAB, 0);
    SET_SKILL(ch, SKILL_ATTACK2, 0);
    SET_SKILL(ch, SKILL_SCAN, 5);
    SET_SKILL(ch, SKILL_TRIP, 5);
    GET_WAS_THIEF(ch) = 1;
    break;

  case CLASS_WARRIOR:
    SET_SKILL(ch, SKILL_KICK, 10);
    SET_SKILL(ch, SKILL_RESCUE, 5);
    SET_SKILL(ch, SKILL_BASH, 15);
    SET_SKILL(ch, SKILL_TRACK, 10);
    SET_SKILL(ch, SKILL_ATTACK2, 5);
    SET_SKILL(ch, SKILL_ATTACK3, 0);
    SET_SKILL(ch, SKILL_DISARM, 5);
    SET_SKILL(ch, SKILL_PARRY, 5);
    SET_SKILL(ch, SKILL_DUAL_WIELD, 10);
    GET_WAS_WARRIOR(ch) = 1;
    break;
  }

//  advance_level(ch);
  nmudlog(INFO_LOG, LVL_GOD, TRUE, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
//  mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_PRACTICES(ch) += 10;

  GET_NEWS(ch) = top_news_version -5;

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;
  
  GET_GOLD(ch) = 25000;

//  log("about to run get newbie gear");
  get_newbie_gear(ch);
//  log("got newbie gear");

  if (CONFIG_SITEOK_ALL)
    SET_BIT_AR(PLR_FLAGS(ch), PLR_SITEOK);

//  log("past site ok");
  ch->player_specials->saved.olc_zone = NOWHERE;
//  log("past specials");
  send_to_char(ch, "&WWelcome to the newbie guild! Please read &Rhelp guild &Wfor more information.\r\n"
		   " You will be allowed to stay in this guild until you have finished your first \r\n"
		   " class. After you reclass the first time, you will be automatically removed from\r\n"
		   " the newbie guild. If you want to leave or join the newbie guild, please just \r\n"
	           "type &RNEWB.&n\r\n");
}

char *newb_warrior[] =
{
"3600",
"3601",
"3602",
"3603",
"3604",
"3606",
"3607",
"3608",
"3609",
};

char *newb_thief[] =
{
"3601",
"3610",
"3611",
"3607",
"3612",
"3612",
"3621",
"3609"
};
char *newb_cleric[] =
{
"3600",
"3622",
"3623",
"3624",
"3642",
"3643",
"3644",
"3625",
"3609"
};
char *newb_mage[] =
{
"3600",
"3613",
"3613",
"3614",
"3615",
"3616",
"3617",
"3609"
};


struct char_data *get_newbie_gear(struct char_data *ch)
{
  int i, items=0;
  obj_vnum onum, k;
  struct obj_data *obj;
 
  if(GET_CLASS(ch) == CLASS_WARRIOR) {
    items=9;
    for (i = 0; i < items; i++)
    {
     onum = real_object((atoidx(newb_warrior[i])));
     obj = read_object(onum, REAL);
     obj_to_char(obj, ch);
    }
  }


  if(GET_CLASS(ch) == CLASS_CLERIC) {
    items=9;
    for (i = 0; i < items; i++)
    {
     onum = real_object((atoidx(newb_cleric[i])));
     obj = read_object(onum, REAL);
     obj_to_char(obj, ch);
    }
  }

  if(GET_CLASS(ch) == CLASS_THIEF) {
    items=8;
    for (i = 0; i < items; i++)
    {
     onum = real_object((atoidx(newb_thief[i])));
     obj = read_object(onum, REAL);
     obj_to_char(obj, ch);
    }
  }

  if(GET_CLASS(ch) == CLASS_MAGIC_USER) {
    items=8;
    for (i = 0; i < items; i++)
    {
     onum = real_object((atoidx(newb_mage[i])));
     obj = read_object(onum, REAL);
     obj_to_char(obj, ch);
    }
  }

  k = 7200;
  for (i = 0; i < 10; i++)
  {
     onum = real_object(k);
     obj = read_object(onum, REAL);
     obj_to_char(obj, ch);
  }

return(ch);
}

//
// This function controls the change to maxmove, maxmana, and maxhp for
// each class every time they gain a level.
//
void advance_level(struct char_data *ch)
{
  int add_hp, add_mana = 0, add_move = 0, pracs;

  add_hp = con_app[GET_CON(ch)].hitp;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    add_hp += (rand_number(-1, 2) + (GET_CON(ch) - 14));
    add_mana = (rand_number(4, 7) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_NECROMANCER:
    add_hp += (rand_number(0, 3) + (GET_CON(ch) - 14));
    add_mana = (rand_number(1, 4) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_SORCERER:
    add_hp += (rand_number(-1, 2) + (GET_CON(ch) - 14));
    add_mana = (rand_number(3, 7) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_ELEMENTALIST:
    add_hp += (rand_number(-2, 1) + (GET_CON(ch) - 14));
    add_mana = (rand_number(6, 10) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_CLERIC:
    add_hp += (rand_number(4, 7) + (GET_CON(ch) - 14));
    add_mana = (rand_number(4, 7) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_MYSTIC:
    add_hp += (rand_number(3, 6) + (GET_CON(ch) - 14));
    add_mana = (rand_number(6, 8) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_PRIEST:
    add_hp += (rand_number(6, 9) + (GET_CON(ch) - 14));
    add_mana = (rand_number(1, 4) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_SHAMAN:
    add_hp += (rand_number(5, 8) + (GET_CON(ch) - 14));
    add_mana = (rand_number(2, 5) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_THIEF:
    add_hp += (rand_number(4, 7) + (GET_CON(ch) - 14));
    add_mana = (rand_number(-2, 1) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_ROGUE:
    add_hp += (rand_number(5, 8) + (GET_CON(ch) - 14));
    add_mana = (rand_number(-3, 0) + (GET_INT(ch) - 14));
    add_move = 12;
    break;

  case CLASS_ASSASSIN:
    add_hp += (rand_number(4, 7) + (GET_CON(ch) - 14));
    add_mana = (GET_CON(ch) - 14) - 3;
    add_move = 12;
    break;

  case CLASS_NINJA:
    add_hp += (rand_number(4, 7) + (GET_CON(ch) - 14));
    add_mana = (GET_INT(ch) - 14) - rand_number(2,3);
    add_move = 12;
    break;

  case CLASS_WARRIOR:
    add_hp += (rand_number(8, 12) + (GET_CON(ch) - 14));
    add_mana = (GET_INT(ch) - 14) - 5;
    add_move = 12;
    break;

  case CLASS_RANGER:
    add_hp += (rand_number(8, 12) + (GET_CON(ch) - 14));
    add_mana = (GET_INT(ch) - 14) - 5;
    add_move = 12;
    break;

  case CLASS_KNIGHT:
    add_hp += (rand_number(12, 15) + (GET_CON(ch) - 14));
    add_mana = (GET_INT(ch) - 14) - rand_number(2,3);
    add_move = 12;
    break;

  case CLASS_PALADIN:
    add_hp += (rand_number(9, 13) + (GET_CON(ch) - 14));
    add_mana = (GET_INT(ch) - 14) - 3;
    add_move = 12;
    break;

  }
  
  ch->points.max_hit += MAX(4, add_hp);
  ch->points.max_move += MAX(12, add_move);

  GET_GOLD(ch) += (GET_LEVEL(ch) * 500);

  if (GET_LEVEL(ch) > 1)
    ch->points.max_mana += add_mana;

    pracs = MIN(5, wis_app[GET_WIS(ch)].bonus);
    GET_PRACTICES(ch) += pracs;

  send_to_char(ch, "&GYou gain: &W%d &Ghp  &W%d &Gmana  &W%d &Gmove  &W%d &Gpractices&n\r\n", 
               add_hp, add_mana, add_move, pracs);
  snoop_check(ch);
  save_char(ch);
}


//
// This simply calculates the backstab multiplier based on a character's
// level.  This used to be an array, but was changed to be a function so
// that it would be easier to add more levels to your MUD.  This doesn't
// really create a big performance hit because it's not used very often.
//

int backstab_mult(int level, int hitroll, int ac )
{
  if (level <= 0)
    return 1;	  // level 0 
  else if (level <= 7)
    return 2;	  // level 1 - 7 
  else if (level <= 13)
    return 3;	  // level 8 - 13 
  else if (level <= 20)
    return 4;	  // level 14 - 20 
  else if (level <= 28)
    return 5;	  // level 21 - 28 
  else if (level < LVL_GOD)
    return 6;	  // all remaining mortal levels 
  else
    return 20;	  // immortals 
}


//
// invalid_class is used by handler.c to determine if a piece of equipment
// is
// usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
//

int invalid_class(struct char_data *ch, struct obj_data *obj)
{

  if (IS_NPC(ch))
    return FALSE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_MAGIC_USER) && IS_MAGIC_USER(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_CLERIC) && IS_CLERIC(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_WARRIOR) && IS_WARRIOR(ch))
    return TRUE;

  if (OBJ_FLAGGED(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch))
    return TRUE;

  return FALSE;
}


//
// SPELLS AND SKILLS.  This area defines which spells are assigned to
// which classes, and the minimum level the character must be to use
// the spell or skill.
//

void init_spell_levels(void)
{
  // MAGES 
  spell_level(SPELL_MAGIC_MISSILE, CLASS_MAGIC_USER, 1);
  spell_level(SPELL_DETECT_INVIS, CLASS_MAGIC_USER, 2);
  spell_level(SPELL_DETECT_MAGIC, CLASS_MAGIC_USER, 2);
  spell_level(SPELL_CHILL_TOUCH, CLASS_MAGIC_USER, 3);
  spell_level(SPELL_INFRAVISION, CLASS_MAGIC_USER, 3);
  spell_level(SPELL_INVISIBLE, CLASS_MAGIC_USER, 10);
  spell_level(SPELL_ARMOR, CLASS_MAGIC_USER, 4);
  spell_level(SPELL_BURNING_HANDS, CLASS_MAGIC_USER, 5);
  spell_level(SPELL_LOCATE_OBJECT, CLASS_MAGIC_USER, 6);
  spell_level(SPELL_STRENGTH, CLASS_MAGIC_USER, 12);
  spell_level(SPELL_SHOCKING_GRASP, CLASS_MAGIC_USER, 7);
  spell_level(SPELL_SLEEP, CLASS_MAGIC_USER, 13);
  spell_level(SPELL_LIGHTNING_BOLT, CLASS_MAGIC_USER, 9);
  spell_level(SPELL_BLINDNESS, CLASS_MAGIC_USER, 9);
  spell_level(SPELL_DETECT_POISON, CLASS_MAGIC_USER, 10);
  spell_level(SPELL_COLOR_SPRAY, CLASS_MAGIC_USER, 11);
  spell_level(SPELL_DISPEL_MAGIC, CLASS_MAGIC_USER, 12);
  spell_level(SPELL_ENERGY_DRAIN, CLASS_MAGIC_USER, 1);
  spell_level(SPELL_CURSE, CLASS_MAGIC_USER, 20);
  spell_level(SPELL_POISON, CLASS_MAGIC_USER, 14);
  spell_level(SPELL_FIREBALL, CLASS_MAGIC_USER, 13);
  spell_level(SPELL_CALL_LIGHTNING, CLASS_MAGIC_USER, 15);  
  spell_level(SPELL_CHARM, CLASS_MAGIC_USER, 17);
  spell_level(SPELL_KNOW_ALIGNMENT, CLASS_MAGIC_USER, 18);
  spell_level(SPELL_ENCHANT_WEAPON, CLASS_MAGIC_USER, 27);
  spell_level(SPELL_CLONE, CLASS_MAGIC_USER, 30);
  spell_level(SPELL_SONIC_BLAST, CLASS_MAGIC_USER, 21);
  spell_level(SPELL_REFRESH, CLASS_MAGIC_USER, 12);
  spell_level(SPELL_MASS_REFRESH, CLASS_MAGIC_USER, 25);
  spell_level(SPELL_MANASHIELD, CLASS_MAGIC_USER, 15);
  spell_level(SPELL_LIFETAP, CLASS_MAGIC_USER, 20);
  spell_level(SPELL_TELEPORT, CLASS_MAGIC_USER, 9);
  spell_level(SPELL_STONESKIN, CLASS_MAGIC_USER, 2);

  // CLERICS 
  spell_level(SPELL_CURE_LIGHT, CLASS_CLERIC, 1);
  spell_level(SPELL_AURA_BOLT, CLASS_CLERIC, 5);
  spell_level(SPELL_ARMOR, CLASS_CLERIC, 1);
  spell_level(SPELL_MANA_BLAST, CLASS_CLERIC, 11);
  spell_level(SPELL_CREATE_FOOD, CLASS_CLERIC, 8);
  spell_level(SPELL_CREATE_WATER, CLASS_CLERIC, 8);
  spell_level(SPELL_DETECT_POISON, CLASS_CLERIC, 3);
  spell_level(SPELL_DETECT_ALIGN, CLASS_CLERIC, 4);
  spell_level(SPELL_CURE_BLIND, CLASS_CLERIC, 5);
  spell_level(SPELL_BLESS, CLASS_CLERIC, 5);
  spell_level(SPELL_DETECT_INVIS, CLASS_CLERIC, 3);
  spell_level(SPELL_DETECT_MAGIC, CLASS_CLERIC, 4);
  spell_level(SPELL_BLINDNESS, CLASS_CLERIC, 6);
  spell_level(SPELL_INFRAVISION, CLASS_CLERIC, 7);
  spell_level(SPELL_POISON, CLASS_CLERIC, 8);
  spell_level(SPELL_GROUP_ARMOR, CLASS_CLERIC, 9);
  spell_level(SPELL_CURE_CRITIC, CLASS_CLERIC, 9);
  spell_level(SPELL_SUMMON, CLASS_CLERIC, 10);
  spell_level(SPELL_REMOVE_POISON, CLASS_CLERIC, 10);
  spell_level(SPELL_WORD_OF_RECALL, CLASS_CLERIC, 9);
  spell_level(SPELL_EARTHQUAKE, CLASS_CLERIC, 21);
  spell_level(SPELL_DISPEL_MAGIC, CLASS_CLERIC, 13);
  spell_level(SPELL_DISPEL_EVIL, CLASS_CLERIC, 14);
  spell_level(SPELL_DISPEL_GOOD, CLASS_CLERIC, 14);
  spell_level(SPELL_SANCTUARY, CLASS_CLERIC, 13);
  spell_level(SPELL_HEAL, CLASS_CLERIC, 14);
  spell_level(SPELL_SENSE_LIFE, CLASS_CLERIC, 18);
  spell_level(SPELL_HARM, CLASS_CLERIC, 19);
  spell_level(SPELL_GROUP_HEAL, CLASS_CLERIC, 22);
  spell_level(SPELL_REMOVE_CURSE, CLASS_CLERIC, 15);
  spell_level(SPELL_REVIVAL, CLASS_CLERIC, 33);
  spell_level(SPELL_RETRIBUTION, CLASS_CLERIC, 30);
  spell_level(SPELL_REFRESH, CLASS_CLERIC, 12);
  spell_level(SPELL_MASS_REFRESH, CLASS_CLERIC, 20);
  spell_level(SPELL_GROUP_RECALL, CLASS_CLERIC, 18);
  spell_level(SPELL_CALL_LIGHTNING, CLASS_CLERIC, 20);
  spell_level(SPELL_REGEN, CLASS_CLERIC, 32);

  // THIEVES 
  spell_level(SKILL_SNEAK, CLASS_THIEF, 1);
  spell_level(SKILL_PICK_LOCK, CLASS_THIEF, 1);
  spell_level(SKILL_BACKSTAB, CLASS_THIEF, 1);
  spell_level(SKILL_STEAL, CLASS_THIEF, 1);
  spell_level(SKILL_HIDE, CLASS_THIEF, 1);
  spell_level(SKILL_TRACK, CLASS_THIEF, 1);
  spell_level(SKILL_ATTACK2, CLASS_THIEF, 1);
  spell_level(SKILL_SECOND_STAB, CLASS_THIEF, 1);
  spell_level(SKILL_TRI_STAB, CLASS_THIEF, 41);
  spell_level(SKILL_FOURTH_STAB, CLASS_THIEF, 41);
  spell_level(SKILL_SCAN, CLASS_THIEF, 1);
  spell_level(SKILL_TRIP, CLASS_THIEF, 1);
  spell_level(SKILL_PARRY, CLASS_THIEF, 1);
  // WARRIORS 
  spell_level(SKILL_KICK, CLASS_WARRIOR, 1);
  spell_level(SKILL_TRACK, CLASS_WARRIOR, 1);
  spell_level(SKILL_BASH, CLASS_WARRIOR, 1);
  spell_level(SKILL_ATTACK2, CLASS_WARRIOR, 1);
  spell_level(SKILL_ATTACK3, CLASS_WARRIOR, 1);
  spell_level(SKILL_DISARM, CLASS_WARRIOR, 1);
  spell_level(SKILL_PARRY, CLASS_WARRIOR, 1);
  spell_level(SKILL_SENSE, CLASS_WARRIOR, 1);
  spell_level(SKILL_DUAL_WIELD, CLASS_WARRIOR, 1);

  // RANGER
  spell_level(SKILL_RANGE_COMBAT, CLASS_RANGER, 1);
  spell_level(SKILL_REVEAL, CLASS_RANGER, 1);
  spell_level(SKILL_ADVANCED_MEDITATE, CLASS_RANGER, 1);
  spell_level(SKILL_SENSE, CLASS_RANGER, 1);
  spell_level(SKILL_ATTACK4, CLASS_RANGER, 1);
  spell_level(SKILL_SUMMON_MOUNT, CLASS_RANGER, 1);
  spell_level(SKILL_RANGE_COMBAT, CLASS_RANGER, 1);

 //KNIGHT
  spell_level(SKILL_RESCUE, CLASS_KNIGHT, 1);
  spell_level(SKILL_ATTACK4, CLASS_KNIGHT, 1);
  spell_level(SKILL_ATTACK5, CLASS_KNIGHT, 1);
  spell_level(SKILL_ADVANCED_KICK, CLASS_KNIGHT, 1);
  spell_level(SKILL_ADVANCED_PARRY, CLASS_KNIGHT, 1);
  spell_level(SKILL_ADVANCED_CHARGE, CLASS_KNIGHT, 1);
  spell_level(SPELL_HEROISM, CLASS_KNIGHT, 1);
  spell_level(SKILL_STOMP, CLASS_KNIGHT, 1);
  spell_level(SKILL_DEFEND, CLASS_KNIGHT, 1);
  spell_level(SKILL_COUNTERATTACK, CLASS_KNIGHT, 1);

 //PALADIN
  spell_level(SPELL_MELEEHEAL, CLASS_PALADIN, 10);
  spell_level(SPELL_PROT_FROM_GOOD, CLASS_PALADIN, 5);
  spell_level(SPELL_VENGEANCE, CLASS_PALADIN, 30);
  spell_level(SKILL_ATTACK4, CLASS_PALADIN, 1);
  spell_level(SPELL_DIVINE_PROT, CLASS_PALADIN, 1);
  spell_level(SPELL_COURAGE, CLASS_PALADIN, 20);
  spell_level(SPELL_PROT_FROM_EVIL, CLASS_PALADIN, 8);
  spell_level(SPELL_PROT_FROM_GOOD, CLASS_PALADIN, 8);
  spell_level(SKILL_ADVANCED_BASH, CLASS_PALADIN, 15);

 // ROGUE
  spell_level(SKILL_ENTANGLE, CLASS_ROGUE, 10);
  spell_level(SKILL_ATTACK3, CLASS_ROGUE, 1);
  spell_level(SKILL_ADVANCED_STEAL, CLASS_ROGUE, 1);
  spell_level(SKILL_ADVANCED_TRIP, CLASS_ROGUE, 1);
  spell_level(SKILL_TRIPWIRE, CLASS_ROGUE, 1);
  spell_level(SKILL_DUAL_THROW, CLASS_ROGUE, 1);

 // NINJA
  spell_level(SKILL_ADVANCED_HIDE, CLASS_NINJA, 1);
  spell_level(SKILL_ADVANCED_DODGE, CLASS_NINJA, 1);
  spell_level(SKILL_ADVANCED_THROW, CLASS_NINJA, 1);
  spell_level(SKILL_ADVANCED_TRIP, CLASS_NINJA, 1);
  spell_level(SPELL_CLOAKED_MOVE, CLASS_NINJA, 20);
  spell_level(SPELL_EVASION, CLASS_NINJA, 10);
  spell_level(SKILL_CRIPPLE, CLASS_NINJA, 15);

 // ASSASSIN
  spell_level(SKILL_ADVANCED_BACKSTAB, CLASS_ASSASSIN, 1);
  spell_level(SKILL_ADVANCED_HIDE, CLASS_ASSASSIN, 1);
  spell_level(SKILL_ATTACK3, CLASS_ASSASSIN, 1);
  spell_level(SKILL_ADVANCED_TRACK, CLASS_ASSASSIN, 1);
  spell_level(SKILL_ADVANCED_DODGE, CLASS_ASSASSIN, 1);
  spell_level(SKILL_AMBUSH, CLASS_ASSASSIN, 1);

 // SHAMAN
  spell_level(SPELL_HELLSTORM, CLASS_SHAMAN, 1);
  spell_level(SPELL_HAMMER_OF_DIVINITY, CLASS_SHAMAN, 10);
  spell_level(SPELL_STARFIRE, CLASS_SHAMAN, 15);
  spell_level(SPELL_ADVANCED_PHASE, CLASS_SHAMAN, 20);
  spell_level(SPELL_ADVANCED_SUMMON, CLASS_SHAMAN, 25);
  spell_level(SPELL_REFLECT, CLASS_SHAMAN, 30);

 // MYSTIC
  spell_level(SPELL_HURRICANE, CLASS_MYSTIC, 25);
  spell_level(SPELL_EAGLE_FLIGHT, CLASS_MYSTIC, 5);
  spell_level(SPELL_CONJURE_BEAST, CLASS_MYSTIC, 30);
  spell_level(SPELL_WHIRLWIND, CLASS_MYSTIC, 20);
  spell_level(SPELL_CONJURE_TREE, CLASS_MYSTIC, 15);
  spell_level(SPELL_RAT_SWARM, CLASS_MYSTIC, 1);
  spell_level(SPELL_CONTROL_WEATHER, CLASS_MYSTIC, 10);

 // PRIEST
  spell_level(SPELL_MASS_HEAL, CLASS_PRIEST, 10);
  spell_level(SPELL_MASS_SANCTUARY, CLASS_PRIEST, 25);
  spell_level(SPELL_MASS_REMOVE_CURSE, CLASS_PRIEST, 15);
  spell_level(SKILL_ADVANCED_HEAL, CLASS_PRIEST, 20);

 // NECROMANCER
  spell_level(SPELL_ANIMATE_DEAD, CLASS_NECROMANCER, 25);
  spell_level(SPELL_DEATH_FIELD, CLASS_NECROMANCER, 20);
  spell_level(SPELL_DRAIN_LIFE, CLASS_NECROMANCER, 15);
  spell_level(SPELL_CONTROL_UNDEAD, CLASS_NECROMANCER, 1);
  spell_level(SPELL_CONFUSION, CLASS_NECROMANCER, 10);
  spell_level(SPELL_MASS_CURSE, CLASS_NECROMANCER, 10);

 // ELEMENTALIST
  spell_level(SPELL_SUMMON_CELESTIAL, CLASS_ELEMENTALIST, 20);
  spell_level(SPELL_CONJURE_ELEMENTAL, CLASS_ELEMENTALIST, 10);
  spell_level(SPELL_CONJURE_FAMILIAR, CLASS_ELEMENTALIST, 30);
  spell_level(SPELL_ADVANCED_CHARM, CLASS_ELEMENTALIST, 15);
  spell_level(SPELL_CLONE_IMAGES, CLASS_ELEMENTALIST, 1);
  spell_level(SPELL_MAJOR_CREATION, CLASS_ELEMENTALIST, 35);
  spell_level(SPELL_SUMMON_FIEND, CLASS_ELEMENTALIST, 20);

 // SORCERER
  spell_level(SPELL_SHOCKSTORM, CLASS_SORCERER, 30);
  spell_level(SPELL_BLIZZARD, CLASS_SORCERER, 25);
  spell_level(SPELL_MASS_ENERGY_DRAIN, CLASS_SORCERER, 20);
  spell_level(SPELL_MASS_PLAGUE, CLASS_SORCERER, 10);
  spell_level(SPELL_MANASHIELD, CLASS_SORCERER, 5);
  spell_level(SPELL_METEOR_STORM, CLASS_SORCERER, 15);
}


//
// This is the exp given to implementors -- it must always be greater
// than the exp required for immortality, plus at least 20,000 or so.
//

#define EXP_MAX  2147483647

// Function to return the exp required for each class/level 
int level_exp(int chclass, int level)
{
  if (level > LVL_IMPL || level < 0) {
    log("SYSERR: Requesting exp for invalid level %d!", level);
    return 0;
  }

//
// Gods have exp close to EXP_MAX.  This statement should never have to
// changed, regardless of how many mortal or immortal levels exist.
//
   if (level > LVL_GOD) {
     return EXP_MAX - ((LVL_IMPL - level) * 1000);
   }

  // Exp required for normal mortals is below

  switch (level) {

      case  0: return 0;
      case  1: return 1;
      case  2: return 300;
      case  3: return 500;
      case  4: return 1500;
      case  5: return 4000;
      case  6: return 8000;
      case  7: return 16000;
      case  8: return 32000;
      case  9: return 75000;
      case 10: return 150000;
      case 11: return 250000;
      case 12: return 500000;
      case 13: return 750000;
      case 14: return 1000000;
      case 15: return 2000000;
      case 16: return 5000000;
      case 17: return 7000000;
      case 18: return 10000000;
      case 19: return 20000000;
      case 20: return 35000000;
      case 21: return 60000000;
      case 22: return 90000000;
      case 23: return 125000000;
      case 24: return 175000000;
      case 25: return 250000000;
      case 26: return 325000000;
      case 27: return 400000000;
      case 28: return 475000000;
      case 29: return 550000000;
      case 30: return 625000000;
      case 31: return 700000000;
      case 32: return 775000000;
      case 33: return 850000000;
      case 34: return 925000000;
      case 35: return 1000000000;
      case 36: return 1100000000;
      case 37: return 1200000000;
      case 38: return 1300000000;
      case 39: return 1400000000;
      case 40: return 1500000000;
      // add new levels here 
      case LVL_GOD: return 2000000001;
      default: break;
    }

//
// This statement should never be reached if the exp tables in 
// this function are set up properly.  If you see exp of 123456 then 
// the tables above are incomplete -- so, complete them!
//
   
  log("SYSERR: XP tables not set up correctly in class.c!");
  return 123456;
}


// 
// Default titles of male characters.
//

const char *title_male(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Man";
  if (level == LVL_IMPL)
    return "the Implementor";

  switch (chclass) {

    case CLASS_MAGIC_USER:
    case CLASS_SORCERER:
    case CLASS_ELEMENTALIST:
    case CLASS_NECROMANCER:
    switch (level) {
      case  1: return "the Apprentice of Magic";
      case  2: return "the Spell Student";
      case  3: return "the Scholar of Magic";
      case  4: return "the Delver in Spells";
      case  5: return "the Medium of Magic";
      case  6: return "the Scribe of Magic";
      case  7: return "the Seer";
      case  8: return "the Sage";
      case  9: return "the Illusionist";
      case 10: return "the Abjurer";
      case 11: return "the Invoker";
      case 12: return "the Enchanter";
      case 13: return "the Conjurer";
      case 14: return "the Magician";
      case 15: return "the Creator";
      case 16: return "the Savant";
      case 17: return "the Magus";
      case 18: return "the Wizard";
      case 19: return "the Warlock";
      case 20: return "the Sorcerer";
      case 21: return "the Necromancer";
      case 22: return "the Thaumaturge";
      case 23: return "the Student of the Occult";
      case 24: return "the Disciple of the Uncanny";
      case 25: return "the Minor Elemental";
      case 26: return "the Greater Elemental";
      case 27: return "the Crafter of Magics";
      case 28: return "the Shaman";
      case 29: return "the Keeper of Talismans";
      case 30: return "the Archmage";
      case LVL_IMMORT: return "the Immortal Warlock";
      case LVL_GOD: return "the Avatar of Magic";
      case LVL_GRGOD: return "the God of Magic";
      default: return "the Mage";
    }
    break;

    case CLASS_CLERIC:
    case CLASS_MYSTIC:
    case CLASS_PRIEST:
    case CLASS_SHAMAN:
    switch (level) {
      case  1: return "the Believer";
      case  2: return "the Attendant";
      case  3: return "the Acolyte";
      case  4: return "the Novice";
      case  5: return "the Missionary";
      case  6: return "the Adept";
      case  7: return "the Deacon";
      case  8: return "the Vicar";
      case  9: return "the Priest";
      case 10: return "the Minister";
      case 11: return "the Canon";
      case 12: return "the Levite";
      case 13: return "the Curate";
      case 14: return "the Monk";
      case 15: return "the Healer";
      case 16: return "the Chaplain";
      case 17: return "the Expositor";
      case 18: return "the Bishop";
      case 19: return "the Arch Bishop";
      case 20: return "the Patriarch";
//    no one ever thought up these titles 21-30 
      case LVL_IMMORT: return "the Immortal Cardinal";
      case LVL_GOD: return "the Inquisitor";
      case LVL_GRGOD: return "the God of good and evil";
      default: return "the Cleric";
    }
    break;

    case CLASS_THIEF:
    case CLASS_ASSASSIN:
    case CLASS_NINJA:
    case CLASS_ROGUE:
    switch (level) {
      case  1: return "the Pilferer";
      case  2: return "the Footpad";
      case  3: return "the Filcher";
      case  4: return "the Pick-Pocket";
      case  5: return "the Sneak";
      case  6: return "the Pincher";
      case  7: return "the Cut-Purse";
      case  8: return "the Snatcher";
      case  9: return "the Sharper";
      case 10: return "the Rogue";
      case 11: return "the Robber";
      case 12: return "the Magsman";
      case 13: return "the Highwayman";
      case 14: return "the Burglar";
      case 15: return "the Thief";
      case 16: return "the Knifer";
      case 17: return "the Quick-Blade";
      case 18: return "the Killer";
      case 19: return "the Brigand";
      case 20: return "the Cut-Throat";
//    no one ever thought up these titles 21-30 
      case LVL_IMMORT: return "the Immortal Assasin";
      case LVL_GOD: return "the Demi God of thieves";
      case LVL_GRGOD: return "the God of thieves and tradesmen";
      default: return "the Thief";
    }
    break;

    case CLASS_WARRIOR:
    case CLASS_KNIGHT:
    case CLASS_PALADIN:
    case CLASS_RANGER:
    switch(level) {
      case  1: return "the Swordpupil";
      case  2: return "the Recruit";
      case  3: return "the Sentry";
      case  4: return "the Fighter";
      case  5: return "the Soldier";
      case  6: return "the Warrior";
      case  7: return "the Veteran";
      case  8: return "the Swordsman";
      case  9: return "the Fencer";
      case 10: return "the Combatant";
      case 11: return "the Hero";
      case 12: return "the Myrmidon";
      case 13: return "the Swashbuckler";
      case 14: return "the Mercenary";
      case 15: return "the Swordmaster";
      case 16: return "the Lieutenant";
      case 17: return "the Champion";
      case 18: return "the Dragoon";
      case 19: return "the Cavalier";
      case 20: return "the Knight";
//    no one ever thought up these titles 21-30 
      case LVL_IMMORT: return "the Immortal Warlord";
      case LVL_GOD: return "the Extirpator";
      case LVL_GRGOD: return "the God of war";
      default: return "the Warrior";
    }
    break;
  }

  // Default title for classes which do not have titles defined 
  return "the Classless";
}


// 
// Default titles of female characters.
//
const char *title_female(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Woman";
  if (level == LVL_IMPL)
    return "the Implementress";

  switch (chclass) {

    case CLASS_MAGIC_USER:
    case CLASS_SORCERER:
    case CLASS_NECROMANCER:
    case CLASS_ELEMENTALIST:
    switch (level) {
      case  1: return "the Apprentice of Magic";
      case  2: return "the Spell Student";
      case  3: return "the Scholar of Magic";
      case  4: return "the Delveress in Spells";
      case  5: return "the Medium of Magic";
      case  6: return "the Scribess of Magic";
      case  7: return "the Seeress";
      case  8: return "the Sage";
      case  9: return "the Illusionist";
      case 10: return "the Abjuress";
      case 11: return "the Invoker";
      case 12: return "the Enchantress";
      case 13: return "the Conjuress";
      case 14: return "the Witch";
      case 15: return "the Creator";
      case 16: return "the Savant";
      case 17: return "the Craftess";
      case 18: return "the Wizard";
      case 19: return "the War Witch";
      case 20: return "the Sorceress";
      case 21: return "the Necromancress";
      case 22: return "the Thaumaturgess";
      case 23: return "the Student of the Occult";
      case 24: return "the Disciple of the Uncanny";
      case 25: return "the Minor Elementress";
      case 26: return "the Greater Elementress";
      case 27: return "the Crafter of Magics";
      case 28: return "Shaman";
      case 29: return "the Keeper of Talismans";
      case 30: return "Archwitch";
      case LVL_IMMORT: return "the Immortal Enchantress";
      case LVL_GOD: return "the Empress of Magic";
      case LVL_GRGOD: return "the Goddess of Magic";
      default: return "the Witch";
    }
    break;

    case CLASS_CLERIC:
    case CLASS_MYSTIC:
    case CLASS_PRIEST:
    case CLASS_SHAMAN:
    switch (level) {
      case  1: return "the Believer";
      case  2: return "the Attendant";
      case  3: return "the Acolyte";
      case  4: return "the Novice";
      case  5: return "the Missionary";
      case  6: return "the Adept";
      case  7: return "the Deaconess";
      case  8: return "the Vicaress";
      case  9: return "the Priestess";
      case 10: return "the Lady Minister";
      case 11: return "the Canon";
      case 12: return "the Levitess";
      case 13: return "the Curess";
      case 14: return "the Nunne";
      case 15: return "the Healess";
      case 16: return "the Chaplain";
      case 17: return "the Expositress";
      case 18: return "the Bishop";
      case 19: return "the Arch Lady of the Church";
      case 20: return "the Matriarch";
//    no one ever thought up these titles 21-30 
      case LVL_IMMORT: return "the Immortal Priestess";
      case LVL_GOD: return "the Inquisitress";
      case LVL_GRGOD: return "the Goddess of good and evil";
      default: return "the Cleric";
    }
    break;

    case CLASS_THIEF:
    case CLASS_ROGUE:
    case CLASS_ASSASSIN:
    case CLASS_NINJA:
    switch (level) {
      case  1: return "the Pilferess";
      case  2: return "the Footpad";
      case  3: return "the Filcheress";
      case  4: return "the Pick-Pocket";
      case  5: return "the Sneak";
      case  6: return "the Pincheress";
      case  7: return "the Cut-Purse";
      case  8: return "the Snatcheress";
      case  9: return "the Sharpress";
      case 10: return "the Rogue";
      case 11: return "the Robber";
      case 12: return "the Magswoman";
      case 13: return "the Highwaywoman";
      case 14: return "the Burglaress";
      case 15: return "the Thief";
      case 16: return "the Knifer";
      case 17: return "the Quick-Blade";
      case 18: return "the Murderess";
      case 19: return "the Brigand";
      case 20: return "the Cut-Throat";
//    no one ever thought up these titles 21-30 
      case LVL_IMMORT: return "the Immortal Assasin";
      case LVL_GOD: return "the Demi Goddess of thieves";
      case LVL_GRGOD: return "the Goddess of thieves and tradesmen";
      default: return "the Thief";
    }
    break;

    case CLASS_WARRIOR:
    case CLASS_RANGER:
    case CLASS_PALADIN:
    case CLASS_KNIGHT:
    switch(level) {
      case  1: return "the Swordpupil";
      case  2: return "the Recruit";
      case  3: return "the Sentress";
      case  4: return "the Fighter";
      case  5: return "the Soldier";
      case  6: return "the Warrior";
      case  7: return "the Veteran";
      case  8: return "the Swordswoman";
      case  9: return "the Fenceress";
      case 10: return "the Combatess";
      case 11: return "the Heroine";
      case 12: return "the Myrmidon";
      case 13: return "the Swashbuckleress";
      case 14: return "the Mercenaress";
      case 15: return "the Swordmistress";
      case 16: return "the Lieutenant";
      case 17: return "the Lady Champion";
      case 18: return "the Lady Dragoon";
      case 19: return "the Cavalier";
      case 20: return "the Lady Knight";
//    no one ever thought up these titles 21-30 
      case LVL_IMMORT: return "the Immortal Lady of War";
      case LVL_GOD: return "the Queen of Destruction";
      case LVL_GRGOD: return "the Goddess of war";
      default: return "the Warrior";
    }
    break;
  }

  // Default title for classes which do not have titles defined 
  return "the Classless";
}

