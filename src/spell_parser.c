/* ************************************************************************
*   File: spell_parser.c                               
*  Usage: top-level magic routines; outside points of entry to magic sys. *
*                                                                         *
*                                                                         *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "screen.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "dg_scripts.h"


#define SINFO spell_info[spellnum]

/* local globals */
struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];
void make_lower(char *begin, char *end);

/* local functions */
void say_spell(struct char_data *ch, int spellnum, struct char_data *tch, struct obj_data *tobj);
void spello(int spl, const char *name, int max_mana, int min_mana, int mana_change, int minpos, int targets, int violent, int routines, const char *wearoff);
int mag_manacost(struct char_data *ch, int spellnum);
ACMD(do_cast);
void unused_spell(int spl);
void mag_assign_spells(void);
extern int arena_ok(struct char_data * ch, struct char_data * victim);

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, 200 should provide
 * ample slots for skills.
 */

char *spell_check[] =
{
"godly armor",
"teleport",    
"bless",       
"blindness",   
"burning hands",          
"call lightning",
"charm",       
"chill touch", 
"clone",      
"color spray",
"control weather",
"create food",
"create water",
"cure blind",
"cure critic",
"cure light",  
"curse",      
"detect align",
"detect invis",
"detect magic",
"detect poison",
"dispel evil", 
"quake",  
"enchant weapon",
"energy drain",
"fireball",    
"harm",        
"heal",        
"invisible",   
"lightning bolt",
"locate object",
"magic missile",
"poison",      
"protection from evil",
"remove curse",
"sanctify",   
"shocking grasp",
"sleep",       
"strength",    
"summon",      
"ventriloquate",
"word of recall",
"remove poison",
"sense life",  
"animate dead",
"dispel good",
"group armor", 
"group heal",  
"transport",
"infravision", 
"waterwalk",   
"aura bolt",  
"mana blast", 
"sonic blast",
"dispel magic",
"revive",
"clan recall",
"hurricane",  
"eagle flight",
"conjure beast",
"whirlwind",  
"conjure tree",
"rat swarm",  
"hellstorm",  
"hammer of divinity",
"starfire",   
"advanced phase",
"advanced summon",
"advanced heal",
"mass heal",  
"mass sanctify",
"regen",      
"mass remove curse",
"death field",
"drain life", 
"control undead",
"confusion",  	
"mass curse", 
"shockstorm", 
"blizzard",   
"mass energy drain",
"mass plague",
"mana shield", 
"meteor storm",
"summon celestial",
"conjure elemental",
"conjure familiar",
"advanced charm",
"clone images",
"major creation",
"summon fiend",
"meleeheal",  
"evasion",    
"cloaked move",
"entangle",
"retribution",
"plague",     
"refresh",   
"mass refresh",
"lifetap",   
"backstab",  
"bash",       
"hide",       
"kick",       
"pick lock",  
"rescue",     
"sneak",      
"steal",      
"track",      
"second attack",   
"third attack",   
"dual stab",
"tri stab",   
"fourth stab",
"scan",      
"disarm",    
"stomp",     
"range combat",
"trip",      
"fourth attack",   
"advanced kick",
"advanced parry",
"advanced charge",
"fifth attack",   
"parry",     
"heroism",   
"reveal",    
"advanced meditate",
"sense",     
"summon mount",
"protection from good",
"vengeance", 
"divine prot",
"courage",   
"advanced steal",
"advanced trip",
"tripwire",  
"dual throw",
"advanced hide",
"advanced dodge",
"advanced throw",
"advanced backstab",
"ambush",    
"advanced track",
"charge",    
"dodge",     
"throw",  
"rake",
"maul",
"tailwhip",
"roar",
"wingflap",
"bitedead",
"mount",
"riding",
"tame", 
"know alignment",
"recharge",
"navigation",
"advnaced bash",
"defend",
"death wish",
"greater senses",
"stone skin",
"cripple",
"counter attack",
"quickness",
"mass quickness",
"reflect",
"dual wield",
"\n"
};

struct syllable {
  const char *org;
  const char *news;
};


struct syllable syls[] = {
  {" ", " "},
  {"bal", "herbra"},
  {"nat", "rii"},
  {"nug", "tala"},
  {"blind", "bing"},
  {"er", "toka"},
  {"cai", "tama"},
  {"qe", "bira"},
  {"slb", "th"},
  {"js", "srik"},
  {"io", "mas"},
  {"irila", "leeto"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "raml"},
  {"mir", "bar"},
  {"mok", "xaner"},
  {"perco", "setus"},
  {"ness", "famrii"},
  {"420", "tilillia"},
  {"reatin", "dooda"},
  {"ba", "shirm"},
  {"re", "bandara"},
  {"son", "bru"},
  {"keta", "min"},
  {"tri", "progna"},
  {"ganjen", "mofo"},
  {"word of", "mariw"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};

const char *unused_spellname = "!UNUSED!"; /* So we can get &unused_spellname */

int mag_manacost(struct char_data *ch, int spellnum)
{
   int m, c;

   m = MAX(SINFO.mana_max - (SINFO.mana_change * (GET_LEVEL(ch) - SINFO.min_level[0])), SINFO.mana_min);
   c = MAX(SINFO.mana_max - (SINFO.mana_change * (GET_LEVEL(ch) - SINFO.min_level[1])), SINFO.mana_min);

   if (GET_CLASS(ch) == CLASS_WARRIOR || GET_CLASS(ch) == CLASS_THIEF)
     return MIN(m, c);
   else
     return MAX(SINFO.mana_max - (SINFO.mana_change * (GET_LEVEL(ch) - SINFO.min_level[(int) GET_CLASS(ch)])), SINFO.mana_min);

}


void say_spell(struct char_data *ch, int spellnum, struct char_data *tch,
	            struct obj_data *tobj)
{
  char lbuf[256], buf[256], buf1[256], buf2[256];	/* FIXME */
  const char *format;

  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strlcpy(lbuf, skill_name(spellnum), sizeof(lbuf));

  while (lbuf[ofs]) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].news);	/* strcat: BAD */
	ofs += strlen(syls[j].org);
        break;
      }
    }
    /* i.e., we didn't find a match in syls[] */
    if (!*syls[j].org) {
      log("No entry in syllable table for substring of '%s'", lbuf);
      ofs++;
    }
  }

  if (tch != NULL && IN_ROOM(tch) == IN_ROOM(ch)) {
    if (tch == ch)
      format = "$n closes $s eyes and utters the words, '%s'.";
    else
      format = "$n stares at $N and utters the words, '%s'.";
  } else if (tobj != NULL &&
	     ((IN_ROOM(tobj) == IN_ROOM(ch)) || (tobj->carried_by == ch)))
    format = "$n stares at $p and utters the words, '%s'.";
  else
    format = "$n utters the words, '%s'.";

  snprintf(buf1, sizeof(buf1), format, skill_name(spellnum));
  snprintf(buf2, sizeof(buf2), format, buf);

  for (i = world[IN_ROOM(ch)].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_CLASS(ch) == GET_CLASS(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && IN_ROOM(tch) == IN_ROOM(ch)) {
    snprintf(buf1, sizeof(buf1), "$n stares at you and utters the words, '%s'.",
	    GET_CLASS(ch) == GET_CLASS(tch) ? skill_name(spellnum) : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}

/*
 * This function should be used anytime you are not 100% sure that you have
 * a valid spell/skill number.  A typical for() loop would not need to use
 * this because you can guarantee > 0 and <= TOP_SPELL_DEFINE.
 */
const char *skill_name(int num)
{
  if (num > 0 && num <= TOP_SPELL_DEFINE)
    return (spell_info[num].name);
  else if (num == -1)
    return ("UNUSED");
  else
    return ("UNDEFINED");
}

	 
int find_skill_num(char *argument,int begin,int length,char **list,int mode)
{
    int guess, found, search;

    /* If the word contain 0 letters, then a match is already found */
    found = (length < 1);

    guess = 0;

    /* Search for a match */

    if(mode)
    while ( !found && *(list[guess]) != '\n' )
    {
        found = (length==strlen(list[guess]));
        for ( search = 0; search < length && found; search++ )
            found=(*(argument+begin+search)== *(list[guess]+search));
        guess++;
    } else {
        while ( !found && *(list[guess]) != '\n' ) {
            found=1;
            for(search=0;( search < length && found );search++)
                found=(*(argument+begin+search)== *(list[guess]+search));
            guess++;
        }
    }

    return ( found ? guess : -1 );
}


/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data *caster, struct char_data *cvict,
	     struct obj_data *ovict, int spellnum, int level, int casttype)
{
  int savetype;

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return (0);

  if (!cast_wtrigger(caster, cvict, ovict, spellnum))
    return 0;
  if (!cast_otrigger(caster, ovict, spellnum))
    return 0;
  if (!cast_mtrigger(caster, cvict, spellnum))
    return 0;

  if (ROOM_FLAGGED(IN_ROOM(caster), ROOM_NOMAGIC)) {
    send_to_char(caster, "Your magic fizzles out and dies.\r\n");
    act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
    return (0);
  }
  if (ROOM_FLAGGED(IN_ROOM(caster), ROOM_PEACEFUL) &&
      (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
    send_to_char(caster, "A flash of white light fills the room, dispelling your violent magic!\r\n");
    act("White light from no particular source suddenly fills the room, then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
    return (0);
  }
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }

  if(casttype == CAST_STAFF) 
  {
   if(IS_NPC(cvict))
   {
    if (mag_damage(level, caster, cvict, spellnum, savetype) == -1)
      return(-1);

    if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
      mag_unaffects(level, caster, cvict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_AREAS))
      mag_areas(level, caster, spellnum, savetype);
   }
   else
   {
    if (IS_SET(SINFO.routines, MAG_AFFECTS))
      mag_affects(level, caster, cvict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_POINTS))
      mag_points(level, caster, cvict, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_GROUPS))
      mag_groups(level, caster, spellnum, savetype);

    if (IS_SET(SINFO.routines, MAG_MASSES))
      mag_masses(level, caster, spellnum, savetype);
   }
  }
  else 
  {
  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    if (mag_damage(level, caster, cvict, spellnum, savetype) == -1)
      return (-1);	/* Successful and target died, don't cast again. */

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_CHARM:		MANUAL_SPELL(spell_charm); break;
    case SPELL_CREATE_WATER:	MANUAL_SPELL(spell_create_water); break;
    case SPELL_DETECT_POISON:	MANUAL_SPELL(spell_detect_poison); break;
    case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_IDENTIFY:	MANUAL_SPELL(spell_identify); break;
    case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object); break;
    case SPELL_SUMMON:		MANUAL_SPELL(spell_summon); break;
    case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); break;
    case SPELL_TELEPORT:	MANUAL_SPELL(spell_teleport); break;
    case SPELL_REVIVAL:         MANUAL_SPELL(spell_revival); break;
    case SPELL_CONJURE_ELEMENTAL:         MANUAL_SPELL(spell_conjure); break;
//    case SPELL_CLAN_RECALL:	MANUAL_SPELL(spell_clan_recall); break;
    case SPELL_KNOW_ALIGNMENT:  MANUAL_SPELL(spell_know_alignment); break;
    case SPELL_RECHARGE:        MANUAL_SPELL(spell_recharge); break;
//    case SPELL_DISPEL_MAGIC:    MANUAL_SPELL(spell_dispel_magic);break;
    }
  }
  return (1);
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */
void mag_objectmagic(struct char_data *ch, struct obj_data *obj,
		          char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      send_to_char(ch, "It seems powerless.\r\n");
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      /* Level to cast spell at. */
      k = GET_OBJ_VAL(obj, 0) ? GET_OBJ_VAL(obj, 0) : DEFAULT_STAFF_LVL;

      /*
       * Problem : Area/mass spells on staves can cause crashes.
       * Solution: Remove the special nature of area/mass spells on staves.
       * Problem : People like that behavior.
       * Solution: We special case the area/mass spells here.
       */
      if (HAS_SPELL_ROUTINE(GET_OBJ_VAL(obj, 3), MAG_MASSES | MAG_AREAS)) {
        for (i = 0, tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
	  i++;
	while (i-- > 0)
	  call_magic(ch, NULL, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
      } else {
	for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
	  next_tch = tch->next_in_room;
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), k, CAST_STAFF);
	}
      }
    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	if (obj->action_description)
	  act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	else
	  act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description)
	act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
	act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else if (IS_SET(spell_info[GET_OBJ_VAL(obj, 3)].routines, MAG_AREAS | MAG_MASSES)) {
      /* Wands with area spells don't need to be pointed. */
      act("You point $p outward.", FALSE, ch, obj, NULL, TO_CHAR);
      act("$n points $p outward.", TRUE, ch, obj, NULL, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      send_to_char(ch, "It seems powerless.\r\n");
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 GET_OBJ_VAL(obj, 0), CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3),
		 DEFAULT_WAND_LVL, CAST_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;

    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i <= 3; i++)
      if (call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_SCROLL) <= 0)
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;
    act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i <= 3; i++)
      if (call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i),
		       GET_OBJ_VAL(obj, 0), CAST_POTION) <= 0)
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type %d in mag_objectmagic.",
	GET_OBJ_TYPE(obj));
    break;
  }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */
int cast_spell(struct char_data *ch, struct char_data *tch,
	           struct obj_data *tobj, int spellnum)
{
  if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE) {
    log("SYSERR: cast_spell trying to call spellnum %d/%d.", spellnum,
	TOP_SPELL_DEFINE);
    return (0);
  }
    
  if (GET_POS(ch) < SINFO.min_position) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char(ch, "You dream about great magical powers.\r\n");
      break;
    case POS_RESTING:
      send_to_char(ch, "You cannot concentrate while resting.\r\n");
      break;
    case POS_SITTING:
      send_to_char(ch, "You can't do this sitting!\r\n");
      break;
    case POS_FIGHTING:
      send_to_char(ch, "Impossible!  You can't concentrate enough!\r\n");
      break;
    default:
      send_to_char(ch, "You can't do much of anything like this!\r\n");
      break;
    }
    return (0);
  }
  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char(ch, "You are afraid you might hurt your master!\r\n");
    return (0);
  }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
    send_to_char(ch, "You can only cast this spell upon yourself!\r\n");
    return (0);
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char(ch, "You cannot cast this spell upon yourself!\r\n");
    return (0);
  }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !AFF_FLAGGED(ch, AFF_GROUP)) {
    send_to_char(ch, "You can't cast this spell if you're not in a group!\r\n");
    return (0);
  }
  send_to_char(ch, "%s", CONFIG_OK);
  say_spell(ch, spellnum, tch, tobj);

  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */
ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0, words=0;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  char buf3[MAX_INPUT_LENGTH], buf4[MAX_NAME_LENGTH];
  char buf5[MAX_NAME_LENGTH], buf6[MAX_NAME_LENGTH];
  char spell[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  skip_spaces(&argument);

  half_chop(argument, buf, buf2);
  if (buf2 != NULL)
  half_chop(buf2, buf3, buf4);
  if (buf4 != NULL)
  two_arguments(buf4, buf5, buf6);

  if(!*buf3)
   words = 1;
  else if(!*buf5)
   words = 2;
  else if(!*buf6)
   words = 3;
  else if(*buf6)
   words = 4;

  if (words == 1)
  {
     snprintf(spell, sizeof(spell), "%s", buf);
     s = spell;
     t = NULL;
  }
  else if (words == 2)
  {
    make_lower(argument, &spell[0]);
    spellnum = find_skill_num(argument, 0, strlen(argument),spell_check, 0);
    if ((spellnum > 0) && (spellnum <= MAX_SPELLS))
    {
     s = spell;
     t = NULL;
    }
    else
    {
     s = buf;
     t = buf3;
    }
  }
  else if (words == 3)
  {
    make_lower(argument, &spell[0]);
    spellnum = find_skill_num(argument, 0, strlen(argument),spell_check, 0);
    if ((spellnum > 0) && (spellnum <= MAX_SPELLS))
    {
     s = spell;
     t = NULL;
    }
    else
    {
     snprintf(s, sizeof(s), "%s %s", buf, buf3);
     t = buf4;
    }
  }
  else if (words == 4)
  {
    snprintf(spell, sizeof(spell), "%s %s %s", buf, buf3, buf5);
    s = spell;
    t = buf6;
  }
  else
  { 
    send_to_char(ch, "Cast what where?\r\n");
    return;
  }
  
  spellnum = find_skill_num(s, 0, strlen(s),spell_check, 0);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char(ch, "Cast what?!?\r\n");
    return;
  }
  if (GET_LEVEL(ch) < SINFO.min_level[(int)GET_LEVEL(ch)] && (GET_WAS_MAGE(ch) == 0 && GET_WAS_CLERIC(ch) == 0)) {
    send_to_char(ch, "You do not know that spell!\r\n");
    return;
  }
  if (GET_SKILL(ch, spellnum) == 0) {
    send_to_char(ch, "You are unfamiliar with that spell.\r\n");
    return;
  }

  if (!IS_NPC(ch) && AFF_FLAGGED(ch, AFF_DIVINE_PROT) && SINFO.violent) {
    send_to_char(ch, "You can't cast an offensive spell while protected by divine forces.\r\n");
    return;
  }

  /* Find the target */
  if (t != NULL) {
    char arg[MAX_INPUT_LENGTH];

    strlcpy(arg, t, sizeof(arg));
    one_argument(arg, t);
    skip_spaces(&t);
  }

  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_vis(ch, t, NULL, FIND_CHAR_ROOM)) != NULL)
	target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t, NULL, FIND_CHAR_WORLD)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, NULL, ch->carrying)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, NULL, world[IN_ROOM(ch)].contents)) != NULL)
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t, NULL)) != NULL)
	target = TRUE;

  } else {			/* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	!SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      send_to_char(ch, "Upon %s should the spell be cast?\r\n",
		IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD | TAR_OBJ_EQUIP) ? "what" : "who");
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char(ch, "You shouldn't cast that on yourself -- could be bad for your health!\r\n");
    return;
  }
  if (!target) {
    send_to_char(ch, "Cannot find the target of your spell!\r\n");
    return;
  }
  mana = mag_manacost(ch, spellnum);
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_GOD)) {
    send_to_char(ch, "You haven't the energy to cast that spell!\r\n");
    return;
  }

  if(tch && IS_NPC(tch) && MOB_FLAGGED(tch, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

   if (tch && !IS_NPC(tch) && SINFO.violent && !IS_SET(SINFO.routines, MAG_AREAS) && !arena_ok(ch, tch))
     {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(tch, PLR_OUTLAW))
      {
       if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
       {
        if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
        {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))
         {
           send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
           return;
         }
        }
       }
      }
     }

  if (tch && AFF_FLAGGED(tch, AFF_DEFLECT))
  {
     send_to_char(ch, "&M%s is protected from magic as your spell defuses upon contact.&n\r\n", GET_NAME(tch));
     return;
  }

  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (rand_number(0, 101) > GET_SKILL(ch, spellnum)) {
    if (!tch || !skill_message(0, ch, tch, spellnum))
      {
      send_to_char(ch, "You lost your concentration!\r\n");
      send_to_char(ch, "&WYou are drained of %d mana.&n\r\n", mana);
      }
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana / 2)));
    if (SINFO.violent && tch && IS_NPC(tch))
     {
      hit(tch, ch, TYPE_UNDEFINED);
     }
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    send_to_char(ch, "&GYou utter the magic phrase, \"%s\" expending %d mana.&n\r\n", spell_check[spellnum-1],mana);
    if (cast_spell(ch, tch, tobj, spellnum)) {
      if (GET_LEVEL(ch) > LVL_IMMORT)
         WAIT_STATE(ch, 0);
      else if (GET_CLASS(ch) == CLASS_MAGIC_USER)
         WAIT_STATE(ch, (int)(PULSE_VIOLENCE / 3));
      else if (GET_CLASS(ch) == CLASS_CLERIC)
         WAIT_STATE(ch, (int)(PULSE_VIOLENCE / 2));
      else
         WAIT_STATE(ch, PULSE_VIOLENCE);
      if (mana > 0)
	GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
    }
  }
}



void spell_level(int spell, int chclass, int level)
{
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
    return;
  }

  if (chclass < 0 || chclass >= NUM_CLASSES) {
    log("SYSERR: assigning '%s' to illegal class %d/%d.", skill_name(spell),
		chclass, NUM_CLASSES - 1);
    bad = 1;
  }

  if (level < 1 || level > LVL_IMPL) {
    log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),
		level, LVL_IMPL);
    bad = 1;
  }

  if (!bad)    
    spell_info[spell].min_level[chclass] = level;
}


/* Assign the spells on boot up */
void spello(int spl, const char *name, int max_mana, int min_mana,
	int mana_change, int minpos, int targets, int violent, int routines, const char *wearoff)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
  spell_info[spl].min_level[i] = LVL_GOD;
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
  spell_info[spl].name = name;
  spell_info[spl].wear_off_msg = wearoff;
}


void unused_spell(int spl)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_IMPL + 1;
  spell_info[spl].mana_max = 0;
  spell_info[spl].mana_min = 0;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = 0;
  spell_info[spl].targets = 0;
  spell_info[spl].violent = 0;
  spell_info[spl].routines = 0;
  spell_info[spl].name = unused_spellname;
}

#define skillo(skill, name) spello(skill, name, 0, 0, 0, 0, 0, 0, 0, NULL);


/*
 * Arguments for spello calls:
 *
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).
 *
 * spellname: The name of the spell.
 *
 * maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell).
 *
 * minmana :  The minimum mana this spell will take, no matter how high
 * level the caster is.
 *
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|').
 *
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 * spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 * set on any spell that inflicts damage, is considered aggressive (i.e.
 * charm, curse), or is otherwise nasty.
 *
 * routines:  A list of magic routines which are associated with this spell
 * if the spell uses spell templates.  Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

/*
 * NOTE: SPELL LEVELS ARE NO LONGER ASSIGNED HERE AS OF Circle 3.0 bpl9.
 * In order to make this cleaner, as well as to make adding new classes
 * much easier, spell levels are now assigned in class.c.  You only need
 * a spello() call to define a new spell; to decide who gets to use a spell
 * or skill, look in class.c.  -JE 5 Feb 1996
 */

void mag_assign_spells(void)
{
  int i;

  /* Do not change the loop below. */
  for (i = 0; i <= TOP_SPELL_DEFINE; i++)
    unused_spell(i);
  /* Do not change the loop above. */

  spello(SPELL_ANIMATE_DEAD, "animate dead", 35, 10, 5, POS_STANDING,
	TAR_OBJ_ROOM, FALSE, MAG_SUMMONS,
	NULL);

  spello(SPELL_ARMOR, "godly armor", 30, 5, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You feel less protected.");

  spello(SPELL_AURA_BOLT, "aura bolt", 25, 5, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_MANA_BLAST, "mana blast", 50, 16, 10, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_BLESS, "bless", 35, 5, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS,
	"You feel less righteous.");

  spello(SPELL_BLINDNESS, "blindness", 32, 8, 4, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS,
	"You feel a cloak of blindness dissolve.");

  spello(SPELL_BLIZZARD, "blizzard", 50, 25, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_BURNING_HANDS, "burning hands", 30, 10, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_CALL_LIGHTNING, "call lightning", 40, 20, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_CHARM, "charm person", 75, 40, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL,
	"You feel more self-confident.");

  spello(SPELL_CHILL_TOUCH, "chill touch", 30, 10, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS,
	"You feel your strength return.");

  spello(SPELL_CLAN_RECALL, "clan recall", 20, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_CLOAKED_MOVE, "cloaked move", 35, 20, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel less stealthy.");

  spello(SPELL_CLONE, "clone", 80, 65, 5, POS_STANDING,
	TAR_SELF_ONLY, FALSE, MAG_SUMMONS,
	NULL);

  spello(SPELL_CLONE_IMAGES, "clone images", 100, 70, 5, POS_STANDING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_COLOR_SPRAY, "color spray", 30, 15, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_CONJURE_BEAST, "conjure beast", 50, 30, 50, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

//  spello(SPELL_CONJURE_ELEMENTAL, "conjure elemental", 75, 50, 5, POS_FIGHTING,
//        TAR_IGNORE, TRUE, 0,
//        NULL);

  spello(SPELL_CONJURE_ELEMENTAL, "conjure elemental", 20, 10, 5, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_MANUAL,
        NULL);

  spello(SPELL_CONJURE_FAMILIAR, "conjure familiar", 75, 50, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_CONJURE_TREE, "conjure tree", 50, 20, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_CONFUSION, "confusion", 50, 20, 10, POS_FIGHTING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_CONTROL_UNDEAD, "control undead", 50, 40, 2, POS_STANDING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_CONTROL_WEATHER, "control weather", 75, 25, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_COURAGE, "courage", 30, 5, 5, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel less courageous.");

  spello(SPELL_CREATE_FOOD, "create food", 30, 5, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS,
	NULL);

  spello(SPELL_CREATE_WATER, "create water", 30, 5, 5, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_CURE_BLIND, "cure blind", 30, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS,
	NULL);

  spello(SPELL_CURE_CRITIC, "cure critic", 30, 10, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS,
	NULL);

  spello(SPELL_CURE_LIGHT, "cure light", 30, 12, 4, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS,
	NULL);

  spello(SPELL_CURSE, "curse", 80, 30, 10, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS,
	"You feel more optimistic.");

  spello(SPELL_DEATH_FIELD, "death field", 50, 30, 5, POS_STANDING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_KNOW_ALIGNMENT, "know alignment", 50, 25, 5, POS_STANDING,
        TAR_CHAR_ROOM, TRUE, MAG_MANUAL, NULL);

  spello(SPELL_DETECT_ALIGN, "detect alignment", 20, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
	"You feel less aware.");

  spello(SPELL_DETECT_INVIS, "detect invisibility", 20, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
	"Your eyes stop tingling.");

  spello(SPELL_DETECT_MAGIC, "detect magic", 20, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
	"The detect magic wears off.");

  spello(SPELL_DETECT_POISON, "detect poison", 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL,
	"The detect poison wears off.");

  spello(SPELL_DISPEL_EVIL, "dispel evil", 40, 20, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_DISPEL_GOOD, "dispel good", 40, 20, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_DISPEL_MAGIC, "dispel magic", 30, 5, 5, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
        NULL);

  spello(SPELL_DIVINE_PROT, "divine protection", 30, 5, 5, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel less protected.");

  spello(SPELL_DRAIN_LIFE, "drain life", 50, 25, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_EAGLE_FLIGHT, "eagle flight", 50, 10, 5, POS_STANDING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_EARTHQUAKE, "quake", 40, 18, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS,
	NULL);

  spello(SPELL_ENCHANT_WEAPON, "enchant weapon", 150, 100, 10, POS_STANDING,
	TAR_OBJ_INV, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_ENERGY_DRAIN, "energy drain", 40, 40, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_MANUAL,
	NULL);

  spello(SPELL_EVASION, "evasion", 35, 20, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel slower.");

  spello(SPELL_FIREBALL, "fireball", 40, 24, 4, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_GROUP_ARMOR, "group armor", 50, 30, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS,
	NULL);

  spello(SPELL_GROUP_HEAL, "group heal", 80, 60, 5, POS_FIGHTING,
	TAR_IGNORE, FALSE, MAG_GROUPS,
	NULL);

  spello(SPELL_GROUP_RECALL, "transport", 150, 100, 10, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_HAMMER_OF_DIVINITY, "hammer of divinity", 50, 20, 10, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_HARM, "harm", 50, 25, 5, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_HEAL, "heal", 60, 35, 4, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_UNAFFECTS,
	NULL);

  spello(SPELL_HELLSTORM, "hellstorm", 50, 30, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS,
        NULL);

  spello(SPELL_HEROISM, "heroism", 30, 5, 5, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel less heroic.");

  spello(SPELL_HURRICANE, "hurricane", 50, 20, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_INFRAVISION, "infravision", 25, 10, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
	"Your night vision seems to fade.");

  spello(SPELL_INVISIBLE, "invisibility", 35, 13, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS,
	"You feel yourself exposed.");

  spello(SPELL_LIFETAP, "lifetap", 60, 35, 4, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_UNAFFECTS,
        NULL);

  spello(SPELL_LIGHTNING_BOLT, "lightning bolt", 30, 15, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_LOCATE_OBJECT, "locate object", 25, 20, 1, POS_STANDING,
	TAR_OBJ_WORLD, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_MAGIC_MISSILE, "magic missile", 25, 5, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_MAJOR_CREATION, "major creation", 75, 50, 5, POS_STANDING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_MANASHIELD, "mana shield", 50, 50, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You are no long protected by your magic.");

  spello(SPELL_MASS_CURSE, "mass curse", 50, 30, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_MASS_ENERGY_DRAIN, "mass energy drain", 50, 30, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_MASS_PLAGUE, "mass plague", 50, 30, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_MASS_REFRESH, "mass refresh", 50, 30, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_MASS_REMOVE_CURSE, "mass remove curse", 50, 25, 5, POS_STANDING,
        TAR_IGNORE, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_MASS_SANCTUARY, "mass sanctify", 200, 100, 10, POS_STANDING,
        TAR_IGNORE, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_MASS_QUICK, "mass quickness", 50, 10, 10, POS_STANDING,
        TAR_IGNORE, TRUE, MAG_GROUPS,
        NULL);

  spello(SPELL_MELEEHEAL, "meleeheal", 35, 20, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        NULL);

  spello(SPELL_METEOR_STORM, "meteor storm", 50, 30, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS,
        NULL);

  spello(SPELL_POISON, "poison", 50, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE,
	MAG_AFFECTS | MAG_ALTER_OBJS,
	"You feel less sick.");

  spello(SPELL_PLAGUE, "plague", 50, 20, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE,
        MAG_AFFECTS | MAG_ALTER_OBJS,
        "You feel less sick.");

  spello(SPELL_PROT_FROM_EVIL, "protection from evil", 40, 10, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
	"You feel less protected.");

  spello(SPELL_PROT_FROM_GOOD, "protection from good", 40, 10, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
        "You feel less protected.");

  spello(SPELL_QUICKNESS, "quickness", 40, 10, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel less nimble.");

  spello(SPELL_RAT_SWARM, "rat swarm", 50, 10, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS,
        NULL);

  spello(SPELL_REFRESH, "refresh", 50, 15, 10, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_UNAFFECTS,
        NULL);

  spello(SPELL_REGEN, "regen", 35, 20, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel less healthy.");

  spello(SPELL_REFLECT, "reflect", 40, 10, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
        "You sense a barrier disappearing from around you.");

  spello(SPELL_REMOVE_CURSE, "remove curse", 45, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE,
	MAG_UNAFFECTS | MAG_ALTER_OBJS,
	NULL);

  spello(SPELL_REMOVE_POISON, "remove poison", 40, 8, 10, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS,
	NULL);

  spello(SPELL_RETRIBUTION, "retribution", 100, 75, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_REVIVAL, "revive", 200, 5, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
        NULL);

  spello(SPELL_SANCTUARY, "sanctify", 75, 75, 10, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"The white aura around your body fades.");

  spello(SPELL_SENSE_LIFE, "sense life", 20, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS,
	"You feel less aware of your surroundings.");

  spello(SPELL_SHOCKING_GRASP, "shocking grasp", 30, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
	NULL);

  spello(SPELL_SHOCKSTORM, "shockstorm", 50, 20, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS,
        NULL);

  spello(SPELL_SLEEP, "sleep", 40, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS,
	"You feel less tired.");

  spello(SPELL_SONIC_BLAST, "sonic blast", 50, 20, 4, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS,
        NULL);

  spello(SPELL_STARFIRE, "starfire", 50, 15, 10, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE,
        NULL);

  spello(SPELL_STRENGTH, "strength", 35, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"You feel weaker.");

  spello(SPELL_STONESKIN, "stone skin", 35, 20, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "Your skins becomes softer.");

  spello(SPELL_SUMMON, "summon", 50, 25, 5, POS_STANDING,
	TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_SUMMON_CELESTIAL, "summon celestial", 50, 25, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_SUMMON_FIEND, "summon fiend", 75, 50, 5, POS_FIGHTING,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_TELEPORT, "teleport", 75, 50, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_VENGEANCE, "vengeance", 30, 5, 5, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
        "You feel less vengeful.");

  spello(SPELL_WATERWALK, "waterwalk", 40, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS,
	"Your feet seem less buoyant.");

  spello(SPELL_WHIRLWIND, "whirlwind", 50, 15, 10, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS,
        NULL);

  spello(SPELL_WORD_OF_RECALL, "word of recall", 20, 10, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL,
	NULL);


  /* NON-castable spells should appear below here. */
  spello(SPELL_ADVANCED_CHARM, "advanced charm", 0, 0, 0, 0,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_ADVANCED_PHASE, "advanced phase", 0, 0, 0, 0,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_ADVANCED_SUMMON, "advanced summon", 0, 0, 0, 0,
        TAR_IGNORE, TRUE, 0,
        NULL);

  spello(SPELL_IDENTIFY, "identify", 0, 0, 0, 0,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL,
	NULL);

  spello(SPELL_RECHARGE, "recharge", 1500, 1500, 1500, POS_STANDING,
         TAR_OBJ_INV, FALSE, MAG_MANUAL, NULL);

  spello(SPELL_FIRE_BREATH, "fire breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, MAG_AREAS,
	NULL);

  spello(SPELL_GAS_BREATH, "gas breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0,
	NULL);

  spello(SPELL_FROST_BREATH, "frost breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0,
	NULL);

  spello(SPELL_ACID_BREATH, "acid breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0,
	NULL);

  spello(SPELL_LIGHTNING_BREATH, "lightning breath", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0,
	NULL);

  spello(SPELL_SPARKSHIELD, "sparkshield", 5000, 2500, 1000, POS_STANDING, 
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, "Your sparkshield terminates.");

  /* you might want to name this one something more fitting to your theme -Welcor*/
  spello(SPELL_DG_AFFECT, "Script-inflicted", 0, 0, 0, POS_SITTING,
	TAR_IGNORE, TRUE, 0,
	NULL);

  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

  skillo(SKILL_ADVANCED_BACKSTAB, "advanced backstab");
  skillo(SKILL_ADVANCED_CHARGE, "advanced charge");
  skillo(SKILL_ADVANCED_DODGE, "advanced dodge");
  skillo(SKILL_ADVANCED_HEAL, "advanced heal");
  skillo(SKILL_ADVANCED_HIDE, "advanced hide");
  skillo(SKILL_ADVANCED_KICK, "advanced kick");
  skillo(SKILL_ADVANCED_MEDITATE, "advanced meditate");
  skillo(SKILL_ADVANCED_STEAL, "advanced steal");
  skillo(SKILL_ADVANCED_THROW, "advanced throw");
  skillo(SKILL_ADVANCED_TRACK, "advanced track");
  skillo(SKILL_ADVANCED_TRIP, "advanced trip");
  skillo(SKILL_AMBUSH, "ambush");
  skillo(SKILL_BACKSTAB, "backstab");
  skillo(SKILL_BASH, "bash");
  skillo(SKILL_CHARGE, "charge");
  skillo(SKILL_FOURTH_STAB, "deathstab");
  skillo(SKILL_DISARM, "disarm");
  skillo(SKILL_DODGE, "dodge");
  skillo(SKILL_SECOND_STAB, "dual stab");
  skillo(SKILL_DUAL_THROW, "dual throw");
  skillo(SKILL_ATTACK5, "fifth attack");
  skillo(SKILL_ATTACK4, "fourth attack");
  skillo(SKILL_HIDE, "hide");
  skillo(SKILL_KICK, "kick");
  skillo(SKILL_PARRY, "parry");
  skillo(SKILL_PICK_LOCK, "pick lock");
  skillo(SKILL_RANGE_COMBAT, "range combat");
  skillo(SKILL_RESCUE, "rescue");
  skillo(SKILL_REVEAL, "reveal");
  skillo(SKILL_SCAN, "scan");
  skillo(SKILL_ATTACK2, "second attack");
  skillo(SKILL_SENSE, "sense");
  skillo(SKILL_SNEAK, "sneak");
  skillo(SKILL_STEAL, "steal");
  skillo(SKILL_STOMP, "stomp");
  skillo(SKILL_SUMMON_MOUNT, "summon mount");
  skillo(SKILL_ATTACK3, "third attack");
  skillo(SKILL_THROW, "throw");
  skillo(SKILL_TRACK, "track");
  skillo(SKILL_TRIP, "trip");
  skillo(SKILL_TRIPWIRE, "tripwire");
  skillo(SKILL_TRI_STAB, "tristab");
  skillo(SKILL_RAKE, "rake");
  skillo(SKILL_MAUL, "maul");
  skillo(SKILL_TAILWHIP, "tailwhip");
  skillo(SKILL_ROAR, "roar");
  skillo(SKILL_WINGFLAP, "wingflap");
  skillo(SKILL_BITEDEAD, "bitedead");
  skillo(SKILL_MOUNT, "mount");
  skillo(SKILL_RIDING, "riding");
  skillo(SKILL_TAME, "tame");
  skillo(SKILL_DEFEND, "defend");
  skillo(SKILL_ADVANCED_BASH, "advanced bash");
  skillo(SKILL_NAVIGATION, "navigation");
  skillo(SKILL_DEATHWISH, "death wish");
  skillo(SKILL_GREATER_SENSES, "greater senses");
  skillo(SKILL_CRIPPLE, "cripple");
  skillo(SKILL_ENTANGLE, "entangle");
  skillo(SKILL_COUNTERATTACK, "counter attack");
  skillo(SKILL_ADVANCED_PARRY, "advanced parry");
  skillo(SKILL_DUAL_WIELD, "dual wield");
}

