/* ************************************************************************
*   File: magic.c                                                         *
*  Usage: low-level functions for magic; spell template code              *
*                                                                         *
************************************************************************ */


#include "conf.h"
#include "sysdep.h"

#include "screen.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "constants.h"
#include "dg_scripts.h"
#include "logger.h"

/* external variables */
extern int mini_mud;
extern struct spell_info_type spell_info[];
extern int arena_ok(struct char_data * ch, struct char_data * victim);

/* external functions */
byte saving_throws(int class_num, int type, int level); /* class.c */
void clearMemory(struct char_data *ch);
void weight_change_object(struct obj_data *obj, int weight);

/* local functions */
int mag_materials(struct char_data *ch, int item0, int item1, int item2, int extract, int verbose);
void perform_mag_groups(int level, struct char_data *ch, struct char_data *tch, int spellnum, int savetype);
int mag_savingthrow(struct char_data *ch, int type, int modifier);
void affect_update(void);
/*
 * Saving throws are now in class.c as of bpl13.
 */


/*
 * Negative apply_saving_throw[] values make saving throws better!
 * Then, so do negative modifiers.  Though people may be used to
 * the reverse of that. It's due to the code modifying the target
 * saving throw instead of the random number of the character as
 * in some other systems.
 */
int mag_savingthrow(struct char_data *ch, int type, int modifier)
{
  /* NPCs use warrior tables according to some book */
  int class_sav = CLASS_WARRIOR;
  int save;

  if (!IS_NPC(ch))
    class_sav = GET_CLASS(ch);

  save = saving_throws(class_sav, type, GET_LEVEL(ch));
  save += GET_SAVE(ch, type);
  save += modifier;

  /* Throwing a 0 is always a failure. */
  if (MAX(1, save) < rand_number(0, 99))
    return (TRUE);

  /* Oops, failed. Sorry. */
  return (FALSE);
}


/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  struct affected_type *af, *next;
  struct char_data *i;

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next) {
      next = af->next;
      if (af->duration >= 1)
	af->duration--;
      else if (af->duration == -1)	/* No action */
	af->duration = -1;	/* GODs only! unlimited */
      else {
	if ((af->type > 0) && (af->type <= MAX_SPELLS))
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0))
	    if (spell_info[af->type].wear_off_msg)
	      send_to_char(i, "%s\r\n", spell_info[af->type].wear_off_msg);
	affect_remove(i, af);
      }
    }
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int mag_materials(struct char_data *ch, int item0, int item1, int item2,
		      int extract, int verbose)
{
  struct obj_data *tobj;
  struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;

  for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
    if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
      obj0 = tobj;
      item0 = -1;
    } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
      obj1 = tobj;
      item1 = -1;
    } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
      obj2 = tobj;
      item2 = -1;
    }
  }
  if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
    if (verbose) {
      switch (rand_number(0, 2)) {
      case 0:
	send_to_char(ch, "A wart sprouts on your nose.\r\n");
	break;
      case 1:
	send_to_char(ch, "Your hair falls out in clumps.\r\n");
	break;
      case 2:
	send_to_char(ch, "A huge corn develops on your big toe.\r\n");
	break;
      }
    }
    return (FALSE);
  }
  if (extract) {
    if (item0 < 0)
      extract_obj(obj0);
    if (item1 < 0)
      extract_obj(obj1);
    if (item2 < 0)
      extract_obj(obj2);
  }
  if (verbose) {
    send_to_char(ch, "A puff of smoke rises from your pack.\r\n");
    act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
  }
  return (TRUE);
}


/*
 * breath_damage will do all damage associated with dragons breath.  It calculates
 * the amnt of damage, adds modifiers, tells you what saves area and tests them and 
 * calls the damage function do do the damage
 */

int breath_damage(int level, struct char_data *ch, struct char_data *victim, int spellnum, int savetype)
{
  int dam = 0;
  
  if (victim == NULL || ch == NULL) 
   return(0);

  switch (spellnum) {
  /* ONLY BREATHS */
  case SPELL_FIRE_BREATH:
    if (IS_NPC(ch) && IS_DRAGON(ch))
      dam = dice((GET_LEVEL(ch) / 2), 10) + 10;  // level 42 dragon = 21d10 + 10 = max 220, avg 125 min 31
    break;
  default:
    break; 
  }

// if they make save, half damage done, lucky bastards

  if (mag_savingthrow(victim, savetype, 0))
    dam /= 2;

// take mana from the npc so eventually they run out

  dam += ((GET_MANA(ch)/5) + GET_LEVEL(ch));       

// now do the damage, cause they deserved it  
  return (damage(ch, victim, dam, spellnum));

}



/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 *
 * -1 = dead, otherwise the amount of damage done.
 */
int mag_damage(int level, struct char_data *ch, struct char_data *victim,
		     int spellnum, int savetype)
{
  int dam = 0;

  if (victim == NULL || ch == NULL)
    return (0);

  switch (spellnum) {
    /* Mostly mages */
  case SPELL_MAGIC_MISSILE:
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = dice((GET_LEVEL(ch) / 3), 10) + 10;
    else if (GET_WAS_MAGE(ch) > 0)
      dam = dice((GET_LEVEL(ch) / 3), 5) + 10;
    break;
  case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = dice(3, 10) + 15;
    else if (GET_WAS_MAGE(ch) > 0)
      dam = dice(3, 8) + 10;
    break;
  case SPELL_BURNING_HANDS:
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = dice(3, 12) + 3;
    else if (GET_WAS_MAGE(ch) > 0)
      dam = dice(3, 9) + 3;
    break;
  case SPELL_SONIC_BLAST:
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = dice(2, GET_LEVEL(ch));
    else if (GET_WAS_MAGE(ch) > 0)
      dam = dice(2, GET_LEVEL(ch)) - dice(10,2);
    break;
  case SPELL_SHOCKING_GRASP:
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = dice(5, 8) + 10;
    else if (GET_WAS_MAGE(ch) > 0)
      dam = dice(5, 6) + 10;
    break;
  case SPELL_LIGHTNING_BOLT:
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = dice(7, 8) + 7;
    else if (GET_WAS_MAGE(ch) > 0)
      dam = dice(7, 6) + 7;
    break;
  case SPELL_COLOR_SPRAY:
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = dice(9, 8) + 9;
    else if (GET_WAS_MAGE(ch) > 0)
      dam = dice(9, 6) + 9;
    break;
  case SPELL_FIREBALL:
    if (!IS_NPC(ch) && IS_MAGIC_USER(ch))
      dam = (int) ((GET_LEVEL(ch)*1.5) + 20);
    else if (!IS_NPC(ch) && GET_WAS_MAGE(ch) > 0)
      dam = GET_LEVEL(ch) + 20;
    else
      dam = GET_LEVEL(ch);
    break;

    /* Mostly clerics */

  case SPELL_AURA_BOLT:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    dam = dice(3, 5) + (GET_LEVEL(ch)/2) + GET_WIS(ch);
    else if (!IS_NPC(ch) && GET_WAS_CLERIC(ch) > 0)
    dam = dice(3, 5) + (GET_LEVEL(ch)/2);
    else
    dam = dice(2, 5) + (GET_LEVEL(ch)/2);
    break;

  case SPELL_MANA_BLAST:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    dam = dice(5, 8) + (level / 2);
    else if (!IS_NPC(ch) && GET_WAS_CLERIC(ch) > 0)
    dam = dice(5, 8) + (level / 2);
    else
    dam = dice(3, 8) + (level / 2);
    break;
  case SPELL_DISPEL_EVIL:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    dam = dice(4, 16) + 3;
    else if (!IS_NPC(ch) && GET_WAS_CLERIC(ch) > 0)
    dam = dice(4, 16) + 3;
    else
    dam = dice(6, 8) + 6;
    if (IS_EVIL(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (IS_GOOD(victim)) {
      act("&GThe gods protect $N.&n", FALSE, ch, 0, victim, TO_CHAR);
      return (0);
    }
    break;
  case SPELL_DISPEL_GOOD:
    if (!IS_NPC(ch) && IS_CLERIC(ch) == 'Y')
    dam = dice(4, 16) + 3;
    else if (!IS_NPC(ch) && GET_WAS_CLERIC(ch) > 0)
    dam = dice(4, 16) + 3;
    else
    dam = dice(6, 8) + 6;
    if (IS_GOOD(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (IS_EVIL(victim)) {
      act("&GThe gods protect $N.&n", FALSE, ch, 0, victim, TO_CHAR);
      return (0);
    }
    break;


  case SPELL_CALL_LIGHTNING:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    dam = dice(8, 10);
    else if (!IS_NPC(ch) && GET_WAS_CLERIC(ch) > 0)
    dam = dice(8, 10);
    else
    dam = dice(8, 10);
    break;

  case SPELL_HARM:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    dam = dice(10, 20) + 5 + level;
    else if (!IS_NPC(ch) && GET_WAS_CLERIC(ch) > 0)
    dam = dice(10, 20) + 5 + level;
    else
    dam = dice(10, 15) + 10 + level;
    break;


  case SPELL_HURRICANE:
    if (!IS_NPC(ch) && IS_MYSTIC(ch))
       dam = dice(10, 20) + 5 + level;
    break;
  case SPELL_CONJURE_TREE:
    if (!IS_NPC(ch) && IS_MYSTIC(ch))
       dam = dice(5, 10) + 5 + level;
    break;
  case SPELL_CONJURE_BEAST:
    if (!IS_NPC(ch) && IS_MYSTIC(ch))
       dam = dice(5, 10) + 5 + (level/2);
    break;

  case SPELL_HAMMER_OF_DIVINITY:
    if (!IS_NPC(ch) && IS_SHAMAN(ch))
       dam = dice(5, 20) + 5 + level;
    break;
  case SPELL_STARFIRE:
    if (!IS_NPC(ch) && IS_SHAMAN(ch))
       dam = dice(5, 10) + 5 + level;
    break;
  case SPELL_BLIZZARD:
    if (!IS_NPC(ch) && IS_SORCERER(ch))
       dam = dice(10, 10) + 5 + level;
    break;

  case SPELL_DRAIN_LIFE:
    if (!IS_NPC(ch) && IS_NECROMANCER(ch))
    {
       dam = dice(10, 20) + 5 + level;
       GET_HIT(ch) += dam;
    }
    break;

  case SPELL_RETRIBUTION:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    {
       dam = GET_RET_DAM(ch);
       GET_HIT(ch) += dam / 4;
    }
    break;

  case SPELL_ENERGY_DRAIN:
    if (!IS_NPC(ch) && IS_SORCERER(ch))
    {
       dam = GET_LEVEL(ch);
       if (GET_HIT(ch) < GET_MAX_HIT(ch))
       {
          GET_HIT(ch) += rand_number((dam*2), (dam*3));
          GET_MANA(ch) += rand_number(dam*2, (dam*6));
       }
    }
    else
      dam = rand_number((GET_LEVEL(ch)/2), GET_LEVEL(ch));
      if (GET_HIT(ch) < GET_MAX_HIT(ch))
      {
         GET_HIT(ch) += rand_number(dam, (dam*2));
         GET_MANA(ch) += rand_number(dam, (dam*3));
      }
    break;

    /* Area spells */

  case SPELL_EARTHQUAKE:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    dam = dice(4, 16) + level;
    else
    dam = dice(4, 9) + level;
    break;

  case SPELL_WHIRLWIND:
    if (!IS_NPC(ch) && IS_MYSTIC(ch))
       dam = dice(8, 10) + level;
    break;

  case SPELL_RAT_SWARM:
    if (!IS_NPC(ch) && IS_MYSTIC(ch))
       dam = dice(4, 16) + level;
    break;

  case SPELL_HELLSTORM:
    if (!IS_NPC(ch) && IS_SHAMAN(ch))
       dam = dice(3, 15 ) + level;
    break;

  case SPELL_METEOR_STORM:
    if (!IS_NPC(ch) && IS_SORCERER(ch))
       dam = dice(4, 16) + level;
    break;

  case SPELL_SHOCKSTORM:
    if (!IS_NPC(ch) && IS_SORCERER(ch))
       dam = dice(5, 9) + 5 + level;
    break;

  case SPELL_FIRE_BREATH:
     if (IS_DRAGON(ch) || GET_LEVEL(ch) >= 41)
       dam = dice((GET_LEVEL(ch) / 2), 10) + 10;
     break;
  } /* switch(spellnum) */


  /* divide damage by two if victim makes his saving throw */
  if (mag_savingthrow(victim, savetype, 0))
    dam /= 2;

  dam += ((GET_MANA(ch)/5) + GET_LEVEL(ch));

  /* and finally, inflict the damage */
  return (damage(ch, victim, dam, spellnum));
}


/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
 */

#define MAX_SPELL_AFFECTS 15	/* change if more needed */

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
		      int spellnum, int savetype)
{
  struct affected_type af[MAX_SPELL_AFFECTS];
  bool accum_affect = FALSE, accum_duration = FALSE;
  const char *to_vict = NULL, *to_room = NULL;
  int i;


  if (victim == NULL || ch == NULL)
    return;

  for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
    af[i].type = spellnum;
    af[i].bitvector = 0;
    af[i].modifier = 0;
    af[i].location = APPLY_NONE;
  }

  switch (spellnum) {

  case SPELL_CHILL_TOUCH:
    af[0].location = APPLY_STR;
    if (mag_savingthrow(victim, savetype, 0))
      af[0].duration = 1;
    else
      af[0].duration = 4;
    af[0].modifier = -1;
    accum_duration = TRUE;
    to_vict = "&WYou feel your strength wither!&n";
    break;

  case SPELL_ARMOR:
    af[0].location = APPLY_AC;
    af[0].modifier = -20;
    af[0].duration = 24;
    accum_duration = TRUE;
    to_vict = "&WYou feel someone protecting you.&n";
    break;

  case SPELL_STONESKIN:
    af[0].location = APPLY_AC;
    af[0].modifier = -GET_LEVEL(ch) - 30;
    af[0].duration = 24;
    accum_duration = TRUE;
    to_vict = "&WYour skin becomes hard as stone.&n";
    break;

  case SKILL_CRIPPLE:
    af[0].location = APPLY_STR;
    af[0].modifier = -2;
    af[0].duration = (GET_LEVEL(ch) / 4);

    af[1].location = APPLY_DEX;
    af[1].modifier = -2;
    af[1].duration = (GET_LEVEL(ch) / 4);

    af[2].location = APPLY_CON;
    af[2].modifier = -2;
    af[2].duration = (GET_LEVEL(ch) / 4);

    accum_duration = FALSE;
    to_vict = "&WYou are crippled.&n";
    break;

  case SPELL_BLESS:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 10;
    af[0].duration = (GET_LEVEL(ch) * 2);

    af[1].location = APPLY_SAVING_SPELL;
    af[1].modifier = -1;
    af[1].duration = (GET_LEVEL(ch) * 2);

    accum_duration = TRUE;
    to_vict = "&WYou feel righteous.&n";
    break;

  case SPELL_BLINDNESS:
    if (MOB_FLAGGED(victim,MOB_NOBLIND) || mag_savingthrow(victim, savetype, 0)) {
      send_to_char(ch, "&WYou fail miserably.&n\r\n");
      return;
    }
    if (!mag_savingthrow(victim, savetype, 0))
    {
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -4;
    af[0].duration = 2;
    af[0].bitvector = AFF_BLIND;

    af[1].location = APPLY_AC;
    af[1].modifier = 40;
    af[1].duration = 2;
    af[1].bitvector = AFF_BLIND;

    to_room = "&W$n seems to be blinded!&n";
    to_vict = "&WYou have been blinded!&n";
    }
    break;

  case SPELL_CURSE:
    if (mag_savingthrow(victim, savetype, 0)) {
      send_to_char(ch, "%s", CONFIG_NOEFFECT);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].duration = 1 + (GET_LEVEL(ch) / 2);
    af[0].modifier = -1;
    af[0].bitvector = AFF_CURSE;

    af[1].location = APPLY_DAMROLL;
    af[1].duration = 1 + (GET_LEVEL(ch) / 2);
    af[1].modifier = -1;
    af[1].bitvector = AFF_CURSE;

    accum_duration = TRUE;
    accum_affect = TRUE;
    to_room = "&W$n briefly glows &Rred&G!&n";
    to_vict = "&WYou feel very uncomfortable.&n";
    break;

  case SPELL_DETECT_ALIGN:
    af[0].duration = 12 + level;
    af[0].bitvector = AFF_DETECT_ALIGN;
    accum_duration = TRUE;
    to_vict = "&WYour eyes tingle.&n";
    break;

  case SPELL_DETECT_INVIS:
    af[0].duration = 12 + level;
    af[0].bitvector = AFF_DETECT_INVIS;
    accum_duration = TRUE;
    to_vict = "&WYour eyes attune to the invisible.&n";
    break;

  case SPELL_DETECT_MAGIC:
    af[0].duration = 12 + level;
    af[0].bitvector = AFF_DETECT_MAGIC;
    accum_duration = TRUE;
    to_vict = "&WYour eyes tingle.&n";
    break;

  case SPELL_INFRAVISION:
    af[0].duration = 12 + level;
    af[0].bitvector = AFF_INFRAVISION;
    accum_duration = TRUE;
    to_vict = "&WYour eyes glow red.&n";
    to_room = "&W$n's eyes glow red.&n";
    break;

  case SPELL_INVISIBLE:
    if (!victim)
      victim = ch;

    af[0].duration = 12 + (GET_LEVEL(ch) / 4);
    af[0].modifier = -40;
    af[0].location = APPLY_AC;
    af[0].bitvector = AFF_INVISIBLE;
    accum_duration = TRUE;
    to_vict = "&WYou vanish.&n";
    to_room = "&W$n slowly fades out of existence.&n";
    break;

  case SPELL_PLAGUE:
    if (mag_savingthrow(victim, savetype, 0)) {
      send_to_char(ch, "%s", CONFIG_NOEFFECT);
      return;
    }

    af[0].location = APPLY_STR;
    af[0].duration = GET_LEVEL(ch) / 4;
    af[0].modifier = -4;
    af[0].bitvector = AFF_PLAGUE;

    af[1].duration = GET_LEVEL(ch) / 4;
    af[1].modifier = 40;
    af[1].location = APPLY_AC;
    af[1].bitvector = AFF_PLAGUE;

    to_vict = "&WYou feel very sick.&n";
    to_room = "&W$n gets violently ill!&n";
    break;

  case SPELL_POISON:
    if (mag_savingthrow(victim, savetype, 0)) {
      send_to_char(ch, "%s", CONFIG_NOEFFECT);
      return;
    }

    af[0].location = APPLY_STR;
    af[0].duration = GET_LEVEL(ch);
    af[0].modifier = -2;
    af[0].bitvector = AFF_POISON;
    to_vict = "&WYou feel very sick.&n";
    to_room = "&W$n gets violently ill!&n";
    break;

  case SPELL_QUICKNESS:
    af[0].location = APPLY_DEX;
    af[0].duration = 24;
    af[0].modifier = 10;
    af[0].bitvector = AFF_QUICKNESS;
    accum_duration = FALSE;
    to_vict = "&WYou feel much more nimble!&n";
    break;

  case SPELL_PROT_FROM_EVIL:
    af[0].duration = 24;
    af[0].bitvector = AFF_PROTECT_EVIL;
    accum_duration = TRUE;
    to_vict = "&WYou feel invulnerable!&n";
    break;

  case SPELL_PROT_FROM_GOOD:
    af[0].duration = 24;
    af[0].bitvector = AFF_PROTECT_GOOD;
    accum_duration = TRUE;
    to_vict = "&WYou feel invulnerable!&n";
    break;

  case SPELL_REGEN:
    af[0].duration = 24;
    af[0].bitvector = AFF_REGEN;
    accum_duration = FALSE;
    to_vict = "&WYou feel a a surge of life run through you!&n";
    break;

  case SPELL_MELEEHEAL:
    af[0].duration = 24;
    af[0].bitvector = AFF_MELEEHEAL;
    accum_duration = FALSE;
    to_vict = "&WYou feel a a surge of life run through you!&n";
    break;

  case SPELL_EVASION:
    af[0].duration = 24;
    af[0].bitvector = AFF_EVASION;
    accum_duration = FALSE;
    to_vict = "&WYou feel quicker than normal!&n";
    break;

  case SPELL_MANASHIELD:
    af[0].duration = 10;
    af[0].bitvector = AFF_MANASHIELD;
    accum_duration = FALSE;
    to_vict = "&WYou feel magically protected!&n";
    break;

  case SPELL_CLOAKED_MOVE:
    af[0].duration = 24;
    af[0].bitvector = AFF_CLOAKED_MOVE;
    accum_duration = FALSE;
    to_vict = "&WYou feel very stealthy!&n";
    break;

  case SKILL_ENTANGLE:
    af[0].duration = 24;
    af[0].bitvector = AFF_ENTANGLE;
    accum_duration = FALSE;
    to_vict = "&WYou feel a little clumsy!&n";
    break;

  case SPELL_SANCTUARY:
    af[0].duration = 4;
    af[0].bitvector = AFF_SANCTUARY;

    accum_duration = TRUE;
    to_vict = "&WA white aura momentarily surrounds you.&n";
    to_room = "&W$n is surrounded by a white aura.&n";
    break;

  case SPELL_SLEEP:
    if (!CONFIG_PK_ALLOWED && !IS_NPC(ch) && !IS_NPC(victim))
      return;
    if (MOB_FLAGGED(victim, MOB_NOSLEEP))
      return;
    if (mag_savingthrow(victim, savetype, 0))
      return;

    af[0].duration = 4 + (GET_LEVEL(ch) / 4);
    af[0].bitvector = AFF_SLEEP;

    if (GET_POS(victim) > POS_SLEEPING) {
      send_to_char(victim, "&WYou feel very sleepy...  Zzzz......&n\r\n");
      act("&W$n goes to sleep.&n", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_STRENGTH:
    if (GET_ADD(victim) == 100)
      return;

    af[0].location = APPLY_STR;
    af[0].duration = (GET_LEVEL(ch) / 2) + 4;
    af[0].modifier = 1 + (level > 18);
    accum_duration = TRUE;
    accum_affect = TRUE;
    to_vict = "&WYou feel stronger!&n";
    break;

  case SPELL_SENSE_LIFE:
    to_vict = "&WYour feel your awareness improve.&n";
    af[0].duration = GET_LEVEL(ch);
    af[0].bitvector = AFF_SENSE_LIFE;
    accum_duration = TRUE;
    break;

  case SPELL_WATERWALK:
    af[0].duration = 24;
    af[0].bitvector = AFF_WATERWALK;
    accum_duration = TRUE;
    to_vict = "&WYou suddenly feel lighter than water.&n";
    break;

  case SPELL_REFLECT:
    af[0].duration = 24;
    af[0].bitvector = AFF_REFLECT;
    accum_duration = TRUE;
    to_vict = "&WYou feel a barrier protecting you from harm.&n";
    break;

  case SPELL_SPARKSHIELD:
    af[0].duration = -1;
    af[0].bitvector = AFF_SPARKSHIELD;
    accum_duration = FALSE;
    to_room = "&Y$n surrounds $sself in a shield of universal energy.";
    break;

  case SPELL_CLONE_IMAGES:
    af[0].duration = 10;
    af[0].bitvector = AFF_IMAGES;
    accum_duration = FALSE;
    GET_IMAGES(ch) = rand_number(1,8);
    to_room = "&Y$n creates multiple copies of $sself.";
    break;

  case SPELL_DEATH_FIELD:
    af[0].duration = 15;
    af[0].bitvector = AFF_DEATH_FIELD;
    accum_duration = FALSE;
    to_room = "&Y$n creates a field of death and decay around $sself.";
    break;

  case SPELL_HEROISM:
    af[0].duration = 45;
    af[0].bitvector = AFF_HEROISM;
    accum_duration = FALSE;
    to_room = "&Y$n starts acting very heroic.";
    break;

  case SPELL_DIVINE_PROT:
    if (AFF_FLAGGED(ch, AFF_VENGEANCE)) {
        mag_unaffects(level, ch, ch, SPELL_VENGEANCE, savetype);
    }
    af[0].duration = 30;
    af[0].bitvector = AFF_DIVINE_PROT;
    accum_duration = FALSE;
    to_room = "&Y$n creates a field of divine protection around $sself.";
    break;

  case SPELL_VENGEANCE:
    if (AFF_FLAGGED(ch, AFF_DIVINE_PROT)) {
        mag_unaffects(level, ch, ch, SPELL_DIVINE_PROT, savetype);
    }
    af[0].duration = 30;
    af[0].bitvector = AFF_VENGEANCE;
    accum_duration = FALSE;
    to_room = "&Y$n creates a field of hate and revenge around $sself.";
    break;

  case SPELL_COURAGE:
    af[0].duration = 15;
    af[0].bitvector = AFF_COURAGE;
    accum_duration = FALSE;
    to_room = "&Y$n starts acting very courageous.";
    break;

  }

  /*
   * If this is a mob that has this affect set in its mob file, do not
   * perform the affect.  This prevents people from un-sancting mobs
   * by sancting them and waiting for it to fade, for example.
   */
  if (IS_NPC(victim) && !affected_by_spell(victim, spellnum))
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      if (AFF_FLAGGED(victim, af[i].bitvector)) {
	send_to_char(ch, "%s", CONFIG_NOEFFECT);
	return;
      }

  /*
   * If the victim is already affected by this spell, and the spell does
   * not have an accumulative effect, then fail the spell.
   */
  if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect)) {
    send_to_char(ch, "%s", CONFIG_NOEFFECT);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector || (af[i].location != APPLY_NONE))
      affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


ACMD(do_sense)
{

 mag_affects(GET_LEVEL(ch), ch, ch, SPELL_DETECT_INVIS, SAVING_SPELL);
 mag_affects(GET_LEVEL(ch), ch, ch, SPELL_INFRAVISION, SAVING_SPELL);
 mag_affects(GET_LEVEL(ch), ch, ch, SPELL_SENSE_LIFE, SAVING_SPELL);

}

/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */
void perform_mag_groups(int level, struct char_data *ch,
			struct char_data *tch, int spellnum, int savetype)
{
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
    mag_points(level, ch, tch, SPELL_HEAL, savetype);
    break;
  case SPELL_GROUP_ARMOR:
    mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
    break;
  case SPELL_GROUP_RECALL:
    spell_recall(level, ch, tch, NULL);
    break;
  case SPELL_MASS_SANCTUARY:
    mag_affects(level, ch, tch, SPELL_SANCTUARY, savetype);
    break;
  case SPELL_MASS_REFRESH:
    mag_points(level, ch, tch, SPELL_MASS_REFRESH, savetype);
    break;
  case SPELL_MASS_CURSE:
    mag_affects(level, ch, tch, SPELL_CURSE, savetype);
    break;
  case SPELL_MASS_QUICK:
    mag_affects(level, ch, tch, SPELL_QUICKNESS, savetype);
    break;
  case SPELL_MASS_REMOVE_CURSE:
    mag_unaffects(level, ch, tch, SPELL_REMOVE_CURSE, savetype);
    break;
  }
}


/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */
void mag_groups(int level, struct char_data *ch, int spellnum, int savetype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!AFF_FLAGGED(ch, AFF_GROUP))
    return;
  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (IN_ROOM(tch) != IN_ROOM(ch))
      continue;
    if (!AFF_FLAGGED(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    perform_mag_groups(level, ch, tch, spellnum, savetype);
  }

  if ((k != ch) && AFF_FLAGGED(k, AFF_GROUP))
    perform_mag_groups(level, ch, k, spellnum, savetype);
  perform_mag_groups(level, ch, ch, spellnum, savetype);
}


/*
 * mass spells affect every creature in the room except the caster.
 *
 * No spells of this class currently implemented.
 */
void mag_masses(int level, struct char_data *ch, int spellnum, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;

    switch (spellnum) {
    }
  }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
 */
void mag_areas(int level, struct char_data *ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  const char *to_char = NULL, *to_room = NULL;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (spellnum) {
  case SPELL_EARTHQUAKE:
    to_char = "&WYou gesture and the earth begins to shake all around you!&n";
    to_room ="&W$n gracefully gestures and the earth begins to shake violently!&n";
    break;
  case SPELL_METEOR_STORM:
    to_char = "&WYou gesture and a thousand fiery rocks fall from the sky!&n";
    to_room ="&W$n gracefully gestures and a thousand fiery rocks fall from the sky!&n";
    break;
  case SPELL_WHIRLWIND:
    to_char = "&WYou gesture and wind begins to gust all around you!&n";
    to_room ="&W$n gracefully gestures and the wind begins to gust all around!&n";
    break;
  case SPELL_HELLSTORM:
    to_char = "&WYou gesture and a spew of lava spits up from cracks in the ground!&n";
    to_room ="&W$n gracefully gestures and lava spits up from cracks int he ground!&n";
    break;
  case SPELL_RAT_SWARM:
    to_char = "&WYou gesture and a swarm of rats attack your foes!&n";
    to_room ="&W$n gracefully gestures and a swarm of rats attacks!&n";
    break;
  case SPELL_SHOCKSTORM:
    to_char = "&WYou gesture and electricity begins to run through your hands!&n";
    to_room ="&W$n gracefully gestures and electricity begins to flow through $S hands!&n";
    break;  
   case SPELL_FIRE_BREATH:
    to_char = "&RYour skin chars as the mighty dragons breath licks your body!&n";
    to_room = "&R$n's skin chars as the mighty dragons breath licks $S body!&n";
    break;
  }

  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);
  

  for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;

    /*
     * The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     */

    if (tch == ch)
      continue;
    if (!IS_NPC(tch))
      continue;
    if (!IS_NPC(ch) && IS_NPC(tch) && AFF_FLAGGED(tch, AFF_CHARM))
      continue;
    if(IS_NPC(tch) && MOB_FLAGGED(tch, MOB_PEACEFUL))
      continue;
    if(IS_SAME_GROUP(ch, tch))
      continue;

    /* Doesn't matter if they die here so we don't check. -gg 6/24/98 */
    mag_damage(level, ch, tch, spellnum, 1);
  }
}


/*
 *  Every spell which summons/gates/conjours a mob comes through here.
 *
 *  None of these spells are currently implemented in CircleMUD; these
 *  were taken as examples from the JediMUD code.  Summons can be used
 *  for spells like clone, ariel servant, etc.
 *
 * 10/15/97 (gg) - Implemented Animate Dead and Clone.
 */

/*
 * These use act(), don't put the \r\n.
 */
const char *mag_summon_msgs[] = {
  "\r\n",
  "&W$n makes a strange magical gesture; you feel a strong breeze!&n",
  "&W$n animates a corpse!&n",
  "&W$N appears from a cloud of thick blue smoke!&n",
  "&W$N appears from a cloud of thick green smoke!&n",
  "&W$N appears from a cloud of thick red smoke!&n",
  "&W$N disappears in a thick black cloud!&n"
  "&WAs $n makes a strange magical gesture, you feel a strong breeze.&n",
  "&WAs $n makes a strange magical gesture, you feel a searing heat.&n",
  "&WAs $n makes a strange magical gesture, you feel a sudden chill.&n",
  "&WAs $n makes a strange magical gesture, you feel the dust swirl.&n",
  "&W$n magically divides!&n",
  "&W$n animates a corpse!&n"
};

/*
 * Keep the \r\n because these use send_to_char.
 */
const char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Gosh durnit!\r\n",
  "The elements resist!\r\n",
  "You failed.\r\n",
  "There is no corpse!\r\n"
};

/* These mobiles do not exist. */
#define MOB_MONSUM_I		130
#define MOB_MONSUM_II		140
#define MOB_MONSUM_III		150
#define MOB_GATE_I		160
#define MOB_GATE_II		170
#define MOB_GATE_III		180

/* Defined mobiles. */
#define MOB_ELEMENTAL_BASE	20	/* Only one for now. */
#define MOB_CLONE		10
#define MOB_ZOMBIE		11
#define MOB_AERIALSERVANT	19


void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
		      int spellnum, int savetype)
{
  struct char_data *mob = NULL;
  struct obj_data *tobj, *next_obj;
  int pfail = 0, msg = 0, fmsg = 0, num = 1, handle_corpse = FALSE, i;
  mob_vnum mob_num;

  if (ch == NULL)
    return;

  switch (spellnum) {
  case SPELL_CLONE:
    msg = 10;
    fmsg = rand_number(2, 6);	/* Random fail message. */
    mob_num = MOB_CLONE;
    pfail = 50;	/* 50% failure, should be based on something later. */
    break;

  case SPELL_ANIMATE_DEAD:
    if (obj == NULL || !IS_CORPSE(obj)) {
      act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    handle_corpse = TRUE;
    msg = 11;
    fmsg = rand_number(2, 6);	/* Random fail message. */
    mob_num = MOB_ZOMBIE;
    pfail = 10;	/* 10% failure, should vary in the future. */
    break;

  default:
    return;
  }

  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    send_to_char(ch, "&WYou are too giddy to have any followers!&n\r\n");
    return;
  }
  if (rand_number(0, 101) < pfail) {
    send_to_char(ch, "%s", mag_summon_fail_msgs[fmsg]);
    return;
  }
  for (i = 0; i < num; i++) {
    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
      send_to_char(ch, "&WYou don't quite remember how to make that creature.&n\r\n");
      return;
    }
    char_to_room(mob, IN_ROOM(ch));
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
    SET_BIT_AR(AFF_FLAGS(mob), AFF_CHARM);
    if (spellnum == SPELL_CLONE) {
      /* Don't mess up the prototype; use new string copies. */
      mob->player.name = strdup(GET_NAME(ch));
      mob->player.short_descr = strdup(GET_NAME(ch));
    }
    act(mag_summon_msgs[msg], FALSE, ch, 0, mob, TO_ROOM);
    load_mtrigger(mob);
    add_follower(mob, ch);
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}


void mag_points(int level, struct char_data *ch, struct char_data *victim,
		     int spellnum, int savetype)
{
  int healing = 0, move = 0;

  if (victim == NULL)
    return;

  if (GET_POS(victim) == POS_DEAD)
  {
    send_to_char(ch, "This person is beyond a simple healing spell.");
    return;
  }
// I added if statements to make non clerics less efficient than clerics
// hopefully it was done properly, so far no known issues -TEK- 11-05-04
  switch (spellnum) {
  case SPELL_CURE_LIGHT:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    healing = dice(2, 8) + 10 + (level / 3);
    else
    healing = dice(2, 8) + 5 + (level / 4);
    send_to_char(victim, "&WYou feel better.&n\r\n");
    break;
  case SPELL_CURE_CRITIC:
    if (!IS_NPC(ch) && IS_CLERIC(ch))
    healing = dice(4, 8) + 25 + (level / 3);
    else
    healing = dice(3, 8) + 15 + (level / 4);
    send_to_char(victim, "&WYou feel a lot better.&n\r\n");
    break;
  case SPELL_HEAL:
    if (!IS_NPC(ch) &&  IS_CLERIC(ch))
    healing = dice(5, 8) + 215 + (level / 3);
    else
    healing = dice(4, 8) + 100 + (level / 4);
    send_to_char(victim, "&WA warm feeling floods your body.&n\r\n");
    if(rand_number(1,150) < GET_SKILL(ch, SKILL_ADVANCED_HEAL) /2)
      healing *= 2;
    break;
  case SPELL_LIFETAP:
    if (!IS_NPC(ch) &&  IS_CLERIC(ch))
    healing = dice(5, 8) + 215 + (level / 3);
    else
    healing = dice(4, 8) + 100 + (level / 4);
    send_to_char(victim, "&WA warm feeling floods your body.&n\r\n");
    if(rand_number(1,150) < GET_SKILL(ch, SKILL_ADVANCED_HEAL) /2)
      healing *= 2;
    break;
  case SPELL_REFRESH:
    if (!IS_NPC(ch) &&  IS_CLERIC(ch))
    move = dice(5, 8) + 215 + (level / 3);
    else
    move = dice(4, 8) + 100 + (level / 4);
    send_to_char(victim, "&WA warm feeling floods your body.&n\r\n");
    break;
  }
  switch(spellnum)
  {
   case SPELL_LIFETAP:
    if(GET_HIT(victim) > GET_MAX_HIT(victim)/4)
    {
     GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) - (healing/4));
     GET_MANA(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + healing*4);
    }
    else
     send_to_char(ch, "You don't have enough life to tap.");
   break;
   case SPELL_REFRESH:
    GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + healing);
   break;

   default:
    GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + healing);
  }
  update_pos(victim);
}


void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
		        int spellnum, int type)
{
  int spell = 0, msg_not_affected = TRUE;
  const char *to_vict = NULL, *to_room = NULL;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_HEAL:
    /*
     * Heal also restores health, so don't give the "no effect" message
     * if the target isn't afflicted by the 'blindness' spell.
     */
    msg_not_affected = FALSE;
    /* fall-through */
  case SPELL_CURE_BLIND:
    spell = SPELL_BLINDNESS;
    to_vict = "&WYour vision returns!&n";
    to_room = "&WThere's a momentary gleam in $n's eyes.&n";
    break;
  case SPELL_REMOVE_POISON:
    spell = SPELL_POISON;
    to_vict = "&WA warm feeling runs through your body!&n";
    to_room = "&W$n looks better.&n";
    break;
  case SPELL_REMOVE_CURSE:
    spell = SPELL_CURSE;
    to_vict = "&WYou don't feel so unlucky.&n";
    break;
  case SPELL_DIVINE_PROT:
    spell = SPELL_DIVINE_PROT;
    to_vict = "&WYou are no longer protected by the gods.&n";
    break;
  case SPELL_VENGEANCE:
    spell = SPELL_VENGEANCE;
    to_vict = "&WYou no longer feel vengeful.&n";
    break;
  case SPELL_ARMOR:
    spell = SPELL_ARMOR;
    to_vict = "&WYou no longer feel protected.&n";
    break;
  case SPELL_STONESKIN:
    spell = SPELL_STONESKIN;
    to_vict = "&WYour skin becomes softer.&n";
    break;
  case SPELL_BLESS:
    spell = SPELL_BLESS;
    to_vict = "&WYou are no longer righteous.&n";
    break;
  case SPELL_CURSE:
    spell = SPELL_CURSE;
    to_vict = "&WYou are no longer cursed.&n";
    break;
  case SPELL_DETECT_ALIGN:
    spell = SPELL_DETECT_ALIGN;
    to_vict = "&WYour eyes tingle painfully.&n";
    break;	
  case SPELL_DETECT_INVIS:
    spell = SPELL_DETECT_INVIS;
    to_vict = "&WYour eyes tingle painfully.&n";
    break;
  case SPELL_DETECT_MAGIC:
    spell = SPELL_DETECT_MAGIC;
    to_vict = "&WYour eyes tingle painfully.&n";
    break;
  case SPELL_INFRAVISION:
    spell = SPELL_INFRAVISION;
    to_vict = "&WYour eyes tingle painfully.&n";
    break;
  case SPELL_INVISIBLE:
    spell = SPELL_INVISIBLE;
    to_vict = "&WYou are no longer invisible.&n";
    break;
  case SPELL_PLAGUE:
    spell = SPELL_PLAGUE;
    to_vict = "&WYou are no longer suffering from the plague.&n";
    break;
  case SPELL_POISON:
    spell = SPELL_POISON;
    to_vict = "&WYou are no longer poisoned.&n";
    break;
  case SPELL_QUICKNESS:
    spell = SPELL_QUICKNESS;
    to_vict = "&WYou are no longer as fast.&n";
    break;
  case SPELL_PROT_FROM_EVIL:
    spell = SPELL_PROT_FROM_EVIL;
    to_vict = "&WYou are no longer protected from evil.&n";
    break;
  case SPELL_PROT_FROM_GOOD:
    spell = SPELL_PROT_FROM_GOOD;
    to_vict = "&WYou are no longer protected from good.&n";
    break;
  case SPELL_REGEN:
    spell = SPELL_REGEN;
    to_vict = "&WYou are no longer regenerating.&n";
    break;
  case SPELL_MELEEHEAL:
    spell = SPELL_MELEEHEAL;
    to_vict = "&WYou are no longer healing from melee.&n";
    break;
  case SPELL_EVASION:
    spell = SPELL_EVASION;
    to_vict = "&WYou are no longer as agile.&n";
    break;
  case SPELL_MANASHIELD:
    spell = SPELL_MANASHIELD;
    to_vict = "&WYou are no longer protected by magic.&n";
    break;
  case SPELL_CLOAKED_MOVE:
    spell = SPELL_CLOAKED_MOVE;
    to_vict = "&WYou are no longer cloaked by shadows.&n";
    break;
  case SPELL_SANCTUARY:
    spell = SPELL_SANCTUARY;
    to_vict = "&WYour white aura disappears.&n";
    break;
  case SPELL_STRENGTH:
    spell = SPELL_STRENGTH;
    to_vict = "&WYour not as strong as you were.&n";
    break; 
  case SPELL_SENSE_LIFE:
    spell = SPELL_SENSE_LIFE;
    to_vict = "&WYour eye tingle painfully.&n";
    break;
  case SPELL_WATERWALK:
    spell = SPELL_WATERWALK;
    to_vict = "&WYou can no longer walk on water.&n";
    break;
  case SPELL_REFLECT:
    spell = SPELL_REFLECT;
    to_vict = "&WYour reflective barrier is gone.&n";
    break;
  case SPELL_SPARKSHIELD:
    spell = SPELL_SPARKSHIELD;
    to_vict = "&WYour shield of universal energy is gone.&n";
    break;
  case SPELL_CLONE_IMAGES:
    spell = SPELL_CLONE_IMAGES;
    to_vict = "&WYour cloned images disappear.&n";
    break;
  case SPELL_DEATH_FIELD:
    spell = SPELL_DEATH_FIELD;
    to_vict = "&WYour field of death and decay dissipates.&n";
    break;
  case SPELL_HEROISM:
    spell = SPELL_HEROISM;
    to_vict = "&WYou feel much less heroic.&n";
    break;
  case SPELL_COURAGE:
    spell = SPELL_COURAGE;
    to_vict = "&WYou feel less courageous.&n";
    break;
  default:
    log("SYSERR: unknown spellnum %d passed to mag_unaffects.", spellnum);
    return;
  }

  if (!affected_by_spell(victim, spell)) {
    if (msg_not_affected)
      send_to_char(ch, "%s", CONFIG_NOEFFECT);
    return;
  }

  affect_from_char(victim, spell);
  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);

}


void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
		         int spellnum, int savetype)
{
  const char *to_char = NULL, *to_room = NULL;

  if (obj == NULL)
    return;

  switch (spellnum) {
    case SPELL_BLESS:
      if (!OBJ_FLAGGED(obj, ITEM_BLESS) &&
	  (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch))) {
	SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BLESS);
	to_char = "&W$p glows briefly.&n";
      }
      break;
    case SPELL_CURSE:
      if (!OBJ_FLAGGED(obj, ITEM_NODROP)) {
	SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NODROP);
	if (GET_OBJ_TYPE(obj) == (ITEM_WEAPON || ITEM_BOW || ITEM_CROSSBOW || ITEM_SLING ))
	  GET_OBJ_VAL(obj, 2)--;
	to_char = "&W$p briefly glows red.&n";
      }
      break;
    case SPELL_INVISIBLE:
      if (!OBJ_FLAGGED(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
        SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_INVISIBLE);
        to_char = "&W$p vanishes.&n";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
      GET_OBJ_VAL(obj, 3) = 1;
      to_char = "&W$p steams briefly.&n";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (OBJ_FLAGGED(obj, ITEM_NODROP)) {
        REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == (ITEM_WEAPON || ITEM_BOW || ITEM_CROSSBOW || ITEM_SLING))
          GET_OBJ_VAL(obj, 2)++;
        to_char = "&W$p briefly glows blue.&n";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 0;
        to_char = "&W$p steams briefly.&n";
      }
      break;
  }

  if (to_char == NULL)
    send_to_char(ch, "%s", CONFIG_NOEFFECT);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}



void mag_creations(int level, struct char_data *ch, int spellnum)
{
  struct obj_data *tobj;
  obj_vnum z;

  if (ch == NULL)
    return;
  /* level = MAX(MIN(level, LVL_IMPL), 1); - Hm, not used. */

  switch (spellnum) {
  case SPELL_CREATE_FOOD:
    z = 7271;
    break;
  default:
    send_to_char(ch, "Spell unimplemented, it would seem.\r\n");
    return;
  }

  if (!(tobj = read_object(z, VIRTUAL))) {
    send_to_char(ch, "I seem to have goofed.\r\n");
    log("SYSERR: spell_creations, spell %d, obj %d: obj not found",
	    spellnum, z);
    return;
  }
  obj_to_char(tobj, ch);
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
  load_otrigger(tobj);
}

