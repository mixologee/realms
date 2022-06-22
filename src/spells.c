/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "interpreter.h"
#include "clan.h"
#include "dg_scripts.h"
#include "logger.h"

extern room_rnum r_mortal_start_room;
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;

extern int mini_mud;
extern int pk_allowed;

void clearMemory(struct char_data * ch);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int mag_savingthrow(struct char_data * ch, int type, int modifier);
void name_to_drinkcon(struct obj_data * obj, int type);
void name_from_drinkcon(struct obj_data * obj);
int compute_armor_class(struct char_data *ch);
extern struct clan_type *clan_info;
/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{
  int water;

  if (ch == NULL || obj == NULL)
    return;
  /* level = MAX(MIN(level, LVL_IMPL), 1);	 - not used */

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      name_from_drinkcon(obj);
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
      if (water > 0) {
	if (GET_OBJ_VAL(obj, 1) >= 0)
	  name_from_drinkcon(obj);
	GET_OBJ_VAL(obj, 2) = LIQ_WATER;
	GET_OBJ_VAL(obj, 1) += water;
	name_to_drinkcon(obj, LIQ_WATER);
	weight_change_object(obj, water);
	act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
}

const char *conjure_mob[] = {
  "126",
  "127",
  "128",
  "129",
  "130"
};

ASPELL(spell_conjure)
{
  struct char_data *mob = NULL;
  struct obj_data *held = GET_EQ(ch, WEAR_HOLD);
  int num=0;
  mob_rnum mnum;

  if (!held)
  {
     send_to_char(ch, "You seem to be missing a key component to the spell.\r\n");
     return;
  }

  switch(GET_OBJ_TYPE(held))
  {
    case ITEM_FIRESTONE:
     send_to_char(ch, "&RThe stone in your hand expands into a large fire elemental.\r\n");
     num=0;
     break;
    case ITEM_EARTHSTONE:
     send_to_char(ch, "&yThe stone in your hand expands into a large earth elemental.\r\n");
     num=1;
     break;
    case ITEM_AIRSTONE:
     send_to_char(ch, "&CThe stone in your hand expands into a large air elemental.\r\n");
     num=2;
     break;
    case ITEM_WATERSTONE:
     send_to_char(ch, "&BThe stone in your hand expands into a large water elemental.\r\n");
     num=3;
     break;
    case ITEM_DEATHSTONE:
     send_to_char(ch, "&wThe stone in your hand expands into a large death elemental.\r\n");
     num=4;
     break;
    default:
     num = -1;
     break;
  }

  if (num < 0) {
     send_to_char(ch, "You must be holding an elemental conjuring item.\r\n");
     return;
  } else {

     mnum = real_mobile(atoi(conjure_mob[num]));
     mob = read_mobile(mnum, REAL);
     char_to_room(mob, IN_ROOM(ch));
     extract_obj(unequip_char(ch, WEAR_HOLD));
     GET_MAX_HIT(mob) += GET_LEVEL(ch)*10;
     GET_HIT(mob) = GET_MAX_HIT(mob);
     GET_LEVEL(mob) = GET_LEVEL(ch)-10;

     switch(num)
     {
        case 0:
           GET_HITROLL(mob) += GET_LEVEL(ch);
           break;
        case 1:
           GET_MAX_HIT(mob) += GET_LEVEL(ch)*10;
           GET_HIT(mob) = GET_MAX_HIT(mob);
           break;
        case 2:
           GET_AC(mob) = -50;
           break;
        case 3:
           GET_DAMROLL(mob) += GET_LEVEL(ch);
           break;
        case 4:
           GET_HITROLL(mob) += GET_LEVEL(ch);
           GET_DAMROLL(mob) += GET_LEVEL(ch);
           GET_MAX_HIT(mob) += GET_LEVEL(ch)*20;
           GET_HIT(mob) = GET_MAX_HIT(mob);
           GET_AC(mob) = -100;
           break;
        default:
           break;
     }

     add_follower(mob, ch);
     perform_group(ch, mob);
     perform_group(mob, ch);
  }
}

ASPELL(spell_recall)
{
  room_rnum recallroom;

  if (victim == NULL || IS_NPC(victim))
    return;

  if(ROOM_FLAGGED(IN_ROOM(victim), ROOM_NO_RECALL))
  {
	send_to_char(victim, "You can't recall from here.\r\n");
	return;
  }

  if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(victim)), ZONE_NOASTRAL) && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS)) {
    send_to_char(ch, "A bright flash prevents your spell from working!");
    return;
  }



  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  recallroom = real_room(GET_RECALL(victim));
  char_to_room(victim, recallroom);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);
}


ASPELL(spell_teleport)
{
  room_rnum to_room;

  if (victim == NULL || IS_NPC(victim))
    return;

  if(ROOM_FLAGGED(IN_ROOM(victim), ROOM_NO_TELEPORT))
  {
        send_to_char(victim, "You can't recall from here.\r\n");
        return;
  }

  if (ZONE_FLAGGED(GET_ROOM_ZONE(IN_ROOM(victim)), ZONE_NOASTRAL)) {
    send_to_char(ch, "A bright flash prevents your spell from working!");
    return;
  }


  do {
    to_room = rand_number(0, top_of_world);
  } while (SECT(to_room) == SECT_NOPASS || ROOM_FLAGGED(to_room, ROOM_GODROOM) || ZONE_FLAGGED(GET_ROOM_ZONE(to_room), ZONE_NOASTRAL));

  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);

  if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS))
    MakeWildRoom(ch,UP);

  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);
}

#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_summon)
{
  char buf[MAX_STRING_LENGTH];

  if (ch == NULL || victim == NULL)
    return;

  if (GET_LEVEL(victim) > (GET_LEVEL(ch) + 5)) {
    send_to_char(ch, SUMMON_FAIL);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char(ch, "Sorry, you can't summon mobs.");
    return;
  }

    if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE)) {
      sprintf(buf, "%s just tried to summon you to: %s.\r\n"
	      "%s failed because you have summon protection on.\r\n"
	      "Type NOSUMMON to allow other players to summon you.\r\n",
	      GET_NAME(ch), world[ch->in_room].name,
	      (ch->player.sex == SEX_MALE) ? "He" : "She");
      send_to_char(victim, buf);

      sprintf(buf, "You failed because %s has summon protection on.\r\n",
	      GET_NAME(victim));
      send_to_char(ch, buf);

      sprintf(buf, "%s failed summoning %s to %s.",
	      GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
      nmudlog(MISC_LOG, LVL_GOD, TRUE, "MAGIC: %s failed to summon %s to %s", GET_NAME(ch), GET_NAME(victim), world[IN_ROOM(ch)].name);
//      mudlog(BRF, LVL_IMMORT, TRUE, buf);
      return;
    }
  

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);
}



ASPELL(spell_locate_object)
{
  struct obj_data *i;
  char name[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int j;

  /*
   * FIXME: This is broken.  The spell parser routines took the argument
   * the player gave to the spell and located an object with that keyword.
   * Since we're passed the object and not the keyword we can only guess
  * at what the player originally meant to search for. -gg
   */
  strcpy(name, fname(obj->name));
  j = level / 2;

  for (i = object_list; i && (j > 0); i = i->next) {
    if (!isname(name, i->name))
      continue;

    if (i->carried_by)
      sprintf(buf, "%s is being carried by %s.\r\n",
	      i->short_description, PERS(i->carried_by, ch));
    else if (i->in_room != NOWHERE)
      sprintf(buf, "%s is in %s.\r\n", i->short_description,
	      world[i->in_room].name);
    else if (i->in_obj)
      sprintf(buf, "%s is in %s.\r\n", i->short_description,
	      i->in_obj->short_description);
    else if (i->worn_by)
      sprintf(buf, "%s is being worn by %s.\r\n",
	      i->short_description, PERS(i->worn_by, ch));
    else
      sprintf(buf, "%s's location is uncertain.\r\n",
	      i->short_description);

    CAP(buf);
    send_to_char(ch, buf);
    j--;
  }

  if (j == level / 2)
    send_to_char(ch, "You sense nothing.\r\n");
}



ASPELL(spell_charm)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch)
    send_to_char(ch, "You like yourself even better!\r\n");
  else if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE))
    send_to_char(ch, "You fail because SUMMON protection is on!\r\n");
  else if (AFF_FLAGGED(victim, AFF_SANCTUARY))
    send_to_char(ch, "Your victim is protected by sanctuary!\r\n");
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char(ch, "Your victim resists!\r\n");
  else if (AFF_FLAGGED(ch, AFF_CHARM))
    send_to_char(ch, "You can't have any followers of your own!\r\n");
  else if (AFF_FLAGGED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
    send_to_char(ch, "You fail.\r\n");
  /* player charming another player - no legal reason for this */
  else if (!pk_allowed && !IS_NPC(victim))
    send_to_char(ch, "You fail - shouldn't be doing it anyway.\r\n");
  else if (circle_follow(victim, ch))
    send_to_char(ch, "Sorry, following in circles can not be allowed.\r\n");
  else if (mag_savingthrow(victim, SAVING_PARA, 0))
    send_to_char(ch, "Your victim resists!\r\n");
  else {
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM;

    if (GET_INT(victim))
      af.duration = 24 * 18 / GET_INT(victim);
    else
      af.duration = 24 * 18;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim)) {
      REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_AGGRESSIVE);
      REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_SPEC);
    }
  }
}



ASPELL(spell_identify)
{
  int i;
  int found;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if (obj) {
    send_to_char(ch, "You feel informed:\r\n");
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2,sizeof(buf2));
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(ch, buf);

  if (GET_OBJ_BOUND_ID(obj) != NOBODY) {
    if (get_name_by_id(GET_OBJ_BOUND_ID(obj)) != NULL) 
      send_to_char(ch, "Bound To: %s\r\n", CAP(get_name_by_id(GET_OBJ_BOUND_ID(obj))));
    else
      send_to_char(ch, "Bound To: Nobody\r\n");
  }
   
  if (obj->obj_flags.bitvector) {
      send_to_char(ch, "Item will give you following abilities:  ");
      sprintbitarray(GET_OBJ_AFFECT(obj), affected_bits, AF_ARRAY_MAX, buf);
      strcat(buf, "\r\n");
      send_to_char(ch, buf);
    }
    send_to_char(ch, "Item is: ");
    sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, buf);
    strcat(buf, "\r\n");
    send_to_char(ch, buf);

    sprintf(buf, "Weight: %d, Value: %d, Condition: %d, Min Level: %d\r\n",
    GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_COND(obj), GET_OBJ_LEVEL(obj));
    send_to_char(ch, buf);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
   case ITEM_POTION:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);

      if (GET_OBJ_VAL(obj, 1) >= 1)
	sprintf(buf + strlen(buf), " %s", skill_name(GET_OBJ_VAL(obj, 1)));
      if (GET_OBJ_VAL(obj, 2) >= 1)
	sprintf(buf + strlen(buf), " %s", skill_name(GET_OBJ_VAL(obj, 2)));
      if (GET_OBJ_VAL(obj, 3) >= 1)
	sprintf(buf + strlen(buf), " %s", skill_name(GET_OBJ_VAL(obj, 3)));
      strcat(buf, "\r\n");
      send_to_char(ch, buf);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);
      sprintf(buf + strlen(buf), " %s\r\n", skill_name(GET_OBJ_VAL(obj, 3)));
      sprintf(buf + strlen(buf), "It has %d maximum charge%s and %d remaining.\r\n",
	      GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
	      GET_OBJ_VAL(obj, 2));
      send_to_char(ch, buf);
      break;
    case ITEM_SLING:
    case ITEM_BOW:
    case ITEM_CROSSBOW:
    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%dD%d'", GET_OBJ_VAL(obj, 1),
	      GET_OBJ_VAL(obj, 2));
      sprintf(buf + strlen(buf), " for an average per-round damage of %.1f.\r\n",
	      (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
      send_to_char(ch, buf);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %d\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(ch, buf);
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	if (!found) {
	  send_to_char(ch, "Can affect you as :\r\n");
	  found = TRUE;
	}
	sprinttype(obj->affected[i].location, apply_types, buf2, sizeof(buf2));
	sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
	send_to_char(ch, buf);
      }
    }
  } else if (victim) {		/* victim */
    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(ch, buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %ld years, %d months, %d days and %d hours old.\r\n",
	      GET_NAME(victim), age(victim)->year, age(victim)->month,
	      age(victim)->day, age(victim)->hours);
      send_to_char(ch, buf);
    }
    sprintf(buf, "Height %d cm, Weight %d pounds\r\n",
	    GET_HEIGHT(victim), GET_WEIGHT(victim));
    sprintf(buf + strlen(buf), "Level: %d, Hits: %ld, Mana: %ld\r\n",
	    GET_LEVEL(victim), GET_HIT(victim), GET_MANA(victim));
    sprintf(buf + strlen(buf), "AC: %d, Hitroll: %ld, Damroll: %ld\r\n",
	    compute_armor_class(victim), GET_HITROLL(victim), GET_DAMROLL(victim));
    sprintf(buf + strlen(buf), "Str: %d/%d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d\r\n",
	GET_STR(victim), GET_ADD(victim), GET_INT(victim),
	GET_WIS(victim), GET_DEX(victim), GET_CON(victim), GET_CHA(victim));
    send_to_char(ch, buf);

  }
}


/*
 * Cannot use this spell on an equipped object or it will mess up the
 * wielding character's hit/dam totals.
 */
ASPELL(spell_enchant_weapon)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  /* Either already enchanted or not a weapon. */
  if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || OBJ_FLAGGED(obj, ITEM_MAGIC))
    return;

  /* Make sure no other affections. */
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (obj->affected[i].location != APPLY_NONE)
      return;

  SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC);
  SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_UNIQUE_SAVE);

  obj->affected[0].location = APPLY_HITROLL;
  obj->affected[0].modifier = 1 + (level >= 18);

  obj->affected[1].location = APPLY_DAMROLL;
  obj->affected[1].modifier = 1 + (level >= 20);

  if (IS_GOOD(ch)) {
    SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
    act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
  } else if (IS_EVIL(ch)) {
    SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
    act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
  } else
    act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
}


ASPELL(spell_detect_poison)
{
  if (victim) {
    if (victim == ch) {
      if (AFF_FLAGGED(victim, AFF_POISON))
        send_to_char(ch, "You can sense poison in your blood.\r\n");
      else
        send_to_char(ch, "You feel healthy.\r\n");
    } else {
      if (AFF_FLAGGED(victim, AFF_POISON))
        act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
        act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      if (GET_OBJ_VAL(obj, 3))
	act("You sense that $p has been contaminated.",FALSE,ch,obj,0,TO_CHAR);
      else
	act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
	    TO_CHAR);
      break;
    default:
      send_to_char(ch, "You sense that it should not be consumed.\r\n");
    }
  }
}

ASPELL(spell_revival)
{

   if (GET_POS(victim) != POS_DEAD)
   {
     send_to_char(ch, "That person is not dead.");
     return;
   }

   if (GET_HIT(ch) < (GET_HIT(victim) + 20))
   {
     send_to_char(ch, "You are not strong enough to revive this person.");
     return;
   }

   if (victim)
   {
     send_to_room(IN_ROOM(ch), "%s stands over the dead body and sprinkles some healing herbs in %s's wounds.\r\n", 
                               GET_NAME(ch), GET_NAME(victim));
     send_to_room(IN_ROOM(ch), "%s sits up, caughing harshly.", GET_NAME(victim));
     GET_POS(victim) = POS_RESTING;
     GET_HIT(victim) = 5;
     GET_HIT(ch) = (long int)(GET_HIT(ch) - (GET_HIT(victim) * 0.75));
     REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_DYING);
     GET_DYING_TIME(victim) = -1;
   }

}

/*
ASPELL(spell_clan_recall)
{
  struct clan_type *cptr;
  if (victim == NULL)
    return;

  if (IS_NPC(victim))
    if (!(victim->master == ch))
      return;

  for (cptr = clan_info; cptr->number != GET_CLAN(victim); cptr = cptr->next);

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);

  if(!IS_NPC(victim)) {
    if(GET_CLAN(victim) > 0)
      char_to_room(victim, real_room(cptr->clan_recall));
    else
      char_to_room(victim, r_mortal_start_room);
  }

  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
// uncomment if you use dg scripts
  entry_memory_mtrigger(victim);
  greet_mtrigger(victim, -1);
  greet_memory_mtrigger(victim);

  return;
}
*/
ASPELL(spell_know_alignment)
{
  int al = GET_ALIGNMENT(victim);
  char buf[MAX_STRING_LENGTH];

  if (!victim || !ch)
    return;

  if (IS_NPC(victim)) {
     if (al > 700)
        sprintf(buf, "&WThis creature has an aura of pure white.\r\n");
      else if (al > 350)
         sprintf(buf, "&WThis creature is a very upstanding being.\r\n");
      else if (al > 25)
         sprintf(buf, "&wThis creature can sometimes be kind and thoughtful.\r\n");
      else if (al > -25)
         sprintf(buf, "&GThis creature has a bright green aura.\r\n");
      else if (al > -100)
         sprintf(buf, "&wThis creature isn't the worst being you've ever encountered.\r\n");
      else if (al > -350)
         sprintf(buf, "&RThis creature could always be nicer, but it probably won't happen.\r\n");
      else if (al > -700)
         sprintf(buf, "&RThis creature has an aura of the devil itself.\r\n");
      else
         sprintf(buf, "&YThere isn't much to say about this creature.\r\n");
   }
   else {
      if (al > 700)
         sprintf(buf, "&W%s has an aura of pure white.\r\n", GET_NAME(victim));
      else if (al > 350)
         sprintf(buf, "&W%s is a very upstanding person.\r\n", GET_NAME(victim));
      else if (al > 25)
         sprintf(buf, "&w%s can sometimes be kind and thoughtful.\r\n", GET_NAME(victim));
      else if (al > -25)
         sprintf(buf, "&G%s has a bright green aura.\r\n", GET_NAME(victim));
      else if (al > -100)
         sprintf(buf, "&w%s isn't the worst being you've ever encountered.\r\n", GET_NAME(victim));
      else if (al > -350)
         sprintf(buf, "&R%s could always be nicer, but it probably won't happen.\r\n", GET_NAME(victim));
      else if (al > -700)         
         sprintf(buf, "&R%s has an aura of the devil itself.\r\n", GET_NAME(victim));
      else
         sprintf(buf, "&YThere isn't much to say about %s.\r\n", GET_NAME(victim));
   }   
   send_to_char(ch, buf);
}

ASPELL(spell_recharge)
{
  int num_recharged, meltdown, value;
  value = num_recharged = meltdown = 0;   
  char buf[MAX_STRING_LENGTH];

  if (!OBJ_FLAGGED(obj, ITEM_CHARGEABLE)) {
    send_to_char(ch, "This item is unrechargeable.\r\n");
    return;
  }      

// WANDS FIRST!  obj val2 = current, val1 = max allowable

  if (GET_OBJ_TYPE(obj) == ITEM_WAND) {
    if (GET_OBJ_VAL(obj, 2) < GET_OBJ_VAL(obj, 1)) {
      value = GET_OBJ_VAL(obj, 1) + rand_number(0,3);
      num_recharged = rand_number(-2, value);
      
        if (num_recharged < 1)
          sprintf(buf, "&CThe gods do not favor you today, and grant you no recharge for your wand.&n\r\n");
        
        else if ((num_recharged > 1) && ((num_recharged + GET_OBJ_VAL(obj, 2)) <= GET_OBJ_VAL(obj,1))) {
          sprintf(buf, "The gods favor your request and grant you %d recharges for your wand.\r\n", num_recharged);
          GET_OBJ_VAL(obj, 2) += num_recharged;
        }
        
        else if ((num_recharged > 1) && ((num_recharged + GET_OBJ_VAL(obj, 2)) > GET_OBJ_VAL(obj , 1))) {
          sprintf(buf, "The gods are displeased with your actions and have destroyed your wand.\r\n");
          meltdown = (GET_LEVEL(ch) * rand_number(0,4)) + 10;
          GET_HIT(ch) -= meltdown;
          extract_obj(obj);
          send_to_char(ch, "The gods are displeased with your actions and have destroyed your wand.\r\n");
          act("&C$n is sprayed with debris as the recharge spell has destroyed their wand!&n", FALSE, ch, 0, 0, TO_ROOM);
        }    
        
        else 
           sprintf(buf, "You shouldn't see this message with recharging, please notify a god at once.\r\n");
    } 
  }
 send_to_char(ch, buf);
}        
      

