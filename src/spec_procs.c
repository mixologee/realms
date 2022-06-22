/**************************************************************************
*  File: spec_procs.c                                      Part of tbaMUD *
*  Usage: Implementation of special procedures for mobiles/objects/rooms. *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

/* For more examples: 
 * ftp://ftp.circlemud.org/pub/CircleMUD/contrib/snippets/specials */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "constants.h"

/*   external vars  */
extern struct time_info_data time_info;
extern struct spell_info_type spell_info[];
extern struct guild_info_type guild_info[];

/* extern functions */
ACMD(do_drop);
ACMD(do_gen_door);
ACMD(do_say);
ACMD(do_action);
room_rnum find_target_room(struct char_data *ch, char *rawroomstr);

extern struct command_info cmd_info[]; 
void do_mob_kick(struct char_data *ch, struct char_data *vict);
void do_mob_disarm(struct char_data *ch, struct char_data *vict);
void do_mob_bash(struct char_data *ch, struct char_data *vict);
void do_generic_skill(struct char_data *ch, struct char_data *vict, int type);
/* local functions */
void sort_spells(void);
int compare_spells(const void *x, const void *y);
const char *how_good(int percent);
void list_skills(struct char_data *ch);
SPECIAL(guild);
SPECIAL(dump);
SPECIAL(mayor);
void npc_steal(struct char_data *ch, struct char_data *victim);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(warrior);
SPECIAL(magic_user);
SPECIAL(guild_guard);
SPECIAL(puff);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(cityguard);
SPECIAL(pet_shops);
SPECIAL(bank);
SPECIAL(cockroach);
SPECIAL(cockroach2);
SPECIAL(hunter);
SPECIAL(jenna);
SPECIAL(blaze_room);
SPECIAL(cleric);
SPECIAL(soul_master);
SPECIAL(ferry);
SPECIAL(ramona);
SPECIAL(tmech);
SPECIAL(pblade);
SPECIAL(blocker);
SPECIAL(marshall_mirror);
SPECIAL(doctor);
SPECIAL(crystalgate);

/* Special procedures for mobiles */
int spell_sort_info[MAX_SKILLS + 1];

int compare_spells(const void *x, const void *y)
{
  int	a = *(const int *)x,
	b = *(const int *)y;

  return strcmp(spell_info[a].name, spell_info[b].name);
}

void sort_spells(void)
{
  int a;

  /* initialize array, avoiding reserved. */
  for (a = 1; a <= MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  qsort(&spell_sort_info[1], MAX_SKILLS, sizeof(int), compare_spells);
}

const char *how_good(int percent)
{
  if (percent < 0)  
    return " error)";
  if (percent == 0)
    return " &rUnknown&n";
  if (percent <= 10)
    return " &mInept&n";
  if (percent <= 20)
    return " &RPoor&n";
  if (percent <= 40)
    return " &yAverage&n";
  if (percent <= 55)  
    return " &YDecent&n";
  if (percent <= 70)
    return " &GAdept&n";
  if (percent <= 80)
    return " &GSkilled&n";
  if (percent <= 85)
    return "&cSuperb&n";
  return " &CMastered&n";
}

/*
const char *how_good(int percent)
{
  if (percent < 0)
    return " error)";
  if (percent == 0)
    return " (not learned)";
  if (percent <= 10)
    return " (awful)";
  if (percent <= 20)
    return " (bad)";
  if (percent <= 40)
    return " (poor)";
  if (percent <= 55)
    return " (average)";
  if (percent <= 70)
    return " (fair)";
  if (percent <= 80)
    return " (good)";
  if (percent <= 85)
    return " (very good)";

  return " (superb)";
}
*/

const char *prac_types[] = {
  "spell",
  "skill"
};

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
#define PRAC_TYPE	3	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
extern int prac_params[4][NUM_CLASSES];

#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS(ch)])
#define MINGAIN(ch) (prac_params[MIN_PER_PRAC][(int)GET_CLASS(ch)])
#define MAXGAIN(ch) (prac_params[MAX_PER_PRAC][(int)GET_CLASS(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])

void list_skills(struct char_data *ch)
{
  const char *overflow = "\r\n**OVERFLOW**\r\n";
  int i, sortpos;
  size_t len = 0, nlen;
  char buf2[MAX_STRING_LENGTH];

  len = snprintf(buf2, sizeof(buf2), "You have %d practice session%s remaining.\r\n"
	"You know of the following %ss:\r\n", GET_PRACTICES(ch),
	GET_PRACTICES(ch) == 1 ? "" : "s", SPLSKL(ch));

  for (sortpos = 0; sortpos <= MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)]) {
      nlen = snprintf(buf2 + len, sizeof(buf2) - len, "&B[&C%-20s &B-&n  %7s&B]&n\r\n", spell_info[i].name, how_good(GET_SKILL(ch, i)));
      if (len + nlen >= sizeof(buf2) || nlen < 0)
        break;
      len += nlen;
    }
  }
  if (len >= sizeof(buf2))
    strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

  page_string(ch->desc, buf2, TRUE);
}

SPECIAL(guild)
{
  int skill_num, percent;

  if (IS_NPC(ch) || !CMD_IS("practice"))
    return (FALSE);

  skip_spaces(&argument);

  if (!*argument) {
    list_skills(ch);
    return (TRUE);
  }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char(ch, "You do not seem to be able to practice now.\r\n");
    return (TRUE);
  }

  skill_num = find_skill_num(argument, 0, strlen(argument),spell_check, 0);
//  skill_num++;

//  send_to_char(ch, "Skill Num = %d, Skill Name = %s or %s\r\n", skill_num, spell_check[skill_num-1], spell_info[skill_num].name);

  if (skill_num < 1 ||
      GET_LEVEL(ch) < spell_info[skill_num].min_level[(int) GET_CLASS(ch)]) {
    send_to_char(ch, "You do not know of that %s.\r\n", SPLSKL(ch));
    return (TRUE);
  }
  if (GET_SKILL(ch, skill_num) >= LEARNED(ch)) {
    send_to_char(ch, "You are already learned in that area.\r\n");
    GET_SKILL(ch, skill_num) = 100;
    return (TRUE);
  }
  send_to_char(ch, "You practice for a while...\r\n");
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    send_to_char(ch, "You are now learned in that area.\r\n");

  return (TRUE);
}

SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  for (k = world[IN_ROOM(ch)].contents; k; k = world[IN_ROOM(ch)].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return (FALSE);

  do_drop(ch, argument, cmd, SCMD_DROP);

  for (k = world[IN_ROOM(ch)].contents; k; k = world[IN_ROOM(ch)].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    send_to_char(ch, "You are awarded for outstanding performance.\r\n");
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
  }
  return (TRUE);
}

SPECIAL(mayor)
{
  char actbuf[MAX_INPUT_LENGTH];

  const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
  const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path = NULL;
  static int path_index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      path_index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      path_index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return (FALSE);

  switch (path[path_index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[path_index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_UNLOCK);	/* strcpy: OK */
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_OPEN);	/* strcpy: OK */
    break;

  case 'C':
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_CLOSE);	/* strcpy: OK */
    do_gen_door(ch, strcpy(actbuf, "gate"), 0, SCMD_LOCK);	/* strcpy: OK */
    break;

  case '.':
    move = FALSE;
    break;

  }

  path_index++;
  return (FALSE);
}

/* General special procedures for mobiles. */

void npc_steal(struct char_data *ch, struct char_data *victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) > LVL_IMMORT)
    return;
  if (!CAN_SEE(ch, victim))
    return;
  if (CONFIG_SAFE_MAILING && PLR_FLAGGED(victim, PLR_MAILING))
    return;

  if (AWAKE(victim) && (rand_number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (GET_GOLD(victim) * rand_number(1, 10)) / 100;
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}

/* Quite lethal to low-level characters. */
SPECIAL(snake)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING || !FIGHTING(ch))
    return (FALSE);

  if (IN_ROOM(FIGHTING(ch)) != IN_ROOM(ch) || rand_number(0, GET_LEVEL(ch)) != 0)
    return (FALSE);

  act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
  act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
  call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
  return (TRUE);
}

SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd || GET_POS(ch) != POS_STANDING)
    return (FALSE);

  for (cons = world[IN_ROOM(ch)].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && GET_LEVEL(cons) < LVL_IMMORT && !rand_number(0, 4)) {
      npc_steal(ch, cons);
      return (TRUE);
    }

  return (FALSE);
}

SPECIAL(magic_user)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (FALSE);

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !rand_number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
    vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
    return (TRUE);

  if (GET_LEVEL(ch) > 13 && rand_number(0, 10) == 0)
    cast_spell(ch, vict, NULL, SPELL_POISON);

  if (GET_LEVEL(ch) > 7 && rand_number(0, 8) == 0)
    cast_spell(ch, vict, NULL, SPELL_BLINDNESS);

  if (GET_LEVEL(ch) > 12 && rand_number(0, 12) == 0) {
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }

  if (rand_number(0, 4))
    return (TRUE);

  switch (GET_LEVEL(ch)) {
    case 4:
    case 5:
      cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
      break;
    case 6:
    case 7:
      cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
      break;
    case 8:
    case 9:
      cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
      break;
    case 10:
    case 11:
      cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
      break;
    case 12:
    case 13:
      cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
      break;
    case 14:
    case 15:
    case 16:
    case 17:
      cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
      break;
    default:
      cast_spell(ch, vict, NULL, SPELL_FIREBALL);
      break;
  }
  return (TRUE);
}

SPECIAL(guild_guard) 
{ 
  int i, direction; 
  struct char_data *guard = (struct char_data *)me; 
  const char *buf = "The guard humiliates you, and blocks your way.\r\n"; 
  const char *buf2 = "The guard humiliates $n, and blocks $s way."; 

  if (!IS_MOVE(cmd) || AFF_FLAGGED(guard, AFF_BLIND)) 
    return (FALSE); 

  if (GET_LEVEL(ch) >= LVL_IMMORT) 
    return (FALSE); 

  /* find out what direction they are trying to go */ 
  for (direction = 0; direction < NUM_DIRECTIONS; direction++) 
    if (!strcmp(cmd_info[cmd].command, dirs[direction])) 
      break; 

  for (i = 0; guild_info[i].guild_room != NOWHERE; i++) { 
    /* Wrong guild. */ 
    if (GET_ROOM_VNUM(IN_ROOM(ch)) != guild_info[i].guild_room) 
      continue; 

    /* Wrong direction. */ 
    if (direction != guild_info[i].direction) 
      continue; 

    /* Allow the people of the guild through. */ 
    if (!IS_NPC(ch) && GET_CLASS(ch) == guild_info[i].pc_class) 
      continue; 

    send_to_char(ch, "%s", buf); 
    act(buf2, FALSE, ch, 0, 0, TO_ROOM); 
    return (TRUE); 
  } 

  return (FALSE); 
} 

SPECIAL(puff)
{
  char actbuf[MAX_INPUT_LENGTH];

  if (cmd)
    return (FALSE);

  switch (rand_number(0, 60)) {
    case 0:
      do_say(ch, strcpy(actbuf, "My god!  It's full of stars!"), 0, 0);	/* strcpy: OK */
      return (TRUE);
    case 1:
      do_say(ch, strcpy(actbuf, "How'd all those fish get up here?"), 0, 0);	/* strcpy: OK */
      return (TRUE);
    case 2:
      do_say(ch, strcpy(actbuf, "I'm a very female dragon."), 0, 0);	/* strcpy: OK */
      return (TRUE);
    case 3:
      do_say(ch, strcpy(actbuf, "I've got a peaceful, easy feeling."), 0, 0);	/* strcpy: OK */
      return (TRUE);
    default:
      return (FALSE);
  }
}

SPECIAL(fido)
{
  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content) {
    if (!IS_CORPSE(i))
      continue;

    act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
    for (temp = i->contains; temp; temp = next_obj) {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_room(temp, IN_ROOM(ch));
    }
    extract_obj(i);
    return (TRUE);
  }
  return (FALSE);
}

SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return (TRUE);
  }
  return (FALSE);
}

SPECIAL(cityguard)
{
  struct char_data *tch, *evil, *spittle;
  int max_evil, min_cha;

  if (cmd || !AWAKE(ch) || FIGHTING(ch))
    return (FALSE);

  max_evil = 1000;
  min_cha = 6;
  spittle = evil = NULL;

  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
    if (!CAN_SEE(ch, tch))
      continue;
    if (!IS_NPC(tch) && PLR_FLAGGED(tch, PLR_KILLER)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
    
    if (!IS_NPC(tch) && PLR_FLAGGED(tch, PLR_PRECEPTOR)) {
       act("$n says, 'Look, it's one of those Preceptors!'", FALSE, ch, 0, 0, TO_ROOM);
       return(TRUE);
    }

    if (!IS_NPC(tch) && PLR_FLAGGED(tch, PLR_THIEF)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }

    if (FIGHTING(tch) && GET_ALIGNMENT(tch) < max_evil && (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
      max_evil = GET_ALIGNMENT(tch);
      evil = tch;
    }

    if (GET_CHA(tch) < min_cha) {
      spittle = tch;
      min_cha = GET_CHA(tch);
    }
  }

  if (evil && GET_ALIGNMENT(FIGHTING(evil)) >= 0) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }

  /* Reward the socially inept. */
  if (spittle && !rand_number(0, 9)) {
    static int spit_social;

    if (!spit_social)
      spit_social = find_command("spit");

    if (spit_social > 0) {
      char spitbuf[MAX_NAME_LENGTH + 1];
      strncpy(spitbuf, GET_NAME(spittle), sizeof(spitbuf));	/* strncpy: OK */
      spitbuf[sizeof(spitbuf) - 1] = '\0';
      do_action(ch, spitbuf, spit_social, 0);
      return (TRUE);
    }
  }
  return (FALSE);
}

#define PET_PRICE(pet) (GET_LEVEL(pet) * 300)
SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  room_rnum pet_room;
  struct char_data *pet;

  /* Gross. */
  pet_room = IN_ROOM(ch) + 1;

  if (CMD_IS("list")) {
    send_to_char(ch, "Available pets are:\r\n");
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      /* No, you can't have the Implementor as a pet if he's in there. */
      if (!IS_NPC(pet))
        continue;
      send_to_char(ch, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    two_arguments(argument, buf, pet_name);

    if (!(pet = get_char_room(buf, NULL, pet_room)) || !IS_NPC(pet)) {
      send_to_char(ch, "There is no such pet!\r\n");
      return (TRUE);
    }
    if (GET_GOLD(ch) < PET_PRICE(pet)) {
      send_to_char(ch, "You don't have enough gold!\r\n");
      return (TRUE);
    }
    GET_GOLD(ch) -= PET_PRICE(pet);

    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT_AR(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      snprintf(buf, sizeof(buf), "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = strdup(buf);

      snprintf(buf, sizeof(buf), "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = strdup(buf);
    }
    char_to_room(pet, IN_ROOM(ch));
    add_follower(pet, ch);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char(ch, "May you enjoy your pet.\r\n");
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return (TRUE);
  }

  /* All commands except list and buy */
  return (FALSE);
}

/* Special procedures for objects. */
SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      send_to_char(ch, "Your current balance is %ld coins.\r\n", GET_BANK_GOLD(ch));
    else
      send_to_char(ch, "You currently have no money deposited.\r\n");
    return (TRUE);
  } else if (CMD_IS("deposit")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char(ch, "How much do you want to deposit?\r\n");
      return (TRUE);
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char(ch, "You don't have that many coins!\r\n");
      return (TRUE);
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    send_to_char(ch, "You deposit %d coins.\r\n", amount);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return (TRUE);
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char(ch, "How much do you want to withdraw?\r\n");
      return (TRUE);
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char(ch, "You don't have that many coins deposited!\r\n");
      return (TRUE);
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    send_to_char(ch, "You withdraw %d coins.\r\n", amount);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return (TRUE);
  } else
    return (FALSE);
}

SPECIAL(cleric)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (FALSE);

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !rand_number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
    vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
  {
    cast_spell(ch, ch, NULL, SPELL_SANCTUARY);
    GET_MANA(ch) += 75;
    return (TRUE);
  }

  if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 10))
     cast_spell(ch, ch, NULL, SPELL_HEAL);

  if (rand_number(0, 4))
    return (TRUE);

  switch (GET_LEVEL(ch)) {
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_AURA_BOLT);
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_MANA_BLAST);
    cast_spell(vict, vict, NULL, SPELL_HEAL);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_HARM);
    break;
  }
  return (TRUE);

}

SPECIAL(jenna)
{
  if (cmd)
    return (FALSE);

  switch (rand_number(0, 60)) {
  case 0:
    act("&G$n looks at you with a cute, but deviant smile.&n", FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 1:
    act("&G$n jumps on you and begins to lick your face.&n", FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 2:
    act("&G$n patiently waits for BluntMaster to return.&n", FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 3:
    act("&G$n runs around the room, playing with a tennis ball.&n", FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  default:
    return (FALSE);
  }
}

SPECIAL(cockroach)
{
  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content) {
    if (!IS_CORPSE(i))
      continue;

    act("&C$n looks at you, looks around, then devours a corpse.&n", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_room(temp, IN_ROOM(ch));
    }
    extract_obj(i);
    return (TRUE);
  }

  return (FALSE);
}

SPECIAL(cockroach2)
{
  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content) {
    if (!IS_CORPSE(i))
      continue;

    act("&G$n looks at you, looks at a corpse, grins evilly and tears into it.&n", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
      next_obj = temp->next_content;
      obj_from_obj(temp);
      obj_to_room(temp, IN_ROOM(ch));
    }
    extract_obj(i);
   switch (rand_number(0, 3)) {
  case 0:
    act("&C$n smiles and says, 'MMMMMMM MMMMMMM BITCH!'&n", FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 1:
    act("&C$n looks around the room and says, 'Now THATS some good corpse.'&n", FALSE, ch, 0, 0, TO_ROOM);
        /* strcpy: OK */
    return (TRUE);
  case 2:
    act("&C$n grabs a napkin, sits down next to the corpse and says, 'Yum yum.'&n", FALSE, ch, 0, 0,TO_ROOM);
    /* strcpy: OK */
    return (TRUE);
  case 3:
    act("&C$n stomps it's tiny foot and says, 'Dibs on the corpse yo!'&n", FALSE, ch, 0, 0, TO_ROOM);
 /* strcpy: OK */
    return (TRUE);
  default:
    return (TRUE);
      }

  }

  return (FALSE);
}

SPECIAL(octopus)// This guy blocks any nod god chars from crossing the ocean.
{
  struct char_data *octopus = (struct char_data *)me;
  const char *buf = "&RThe octopus whips it's massive tentacles at you, preventing your escape!&n\r\n";
  const char *buf2 = "&RThe octopus whips it's massive tentacles at $n, preventing $m from leaving.&n\r\n";

  if (!IS_MOVE(cmd) || AFF_FLAGGED(octopus, AFF_BLIND))
    return (FALSE);

  if (GET_LEVEL(ch) >= LVL_GRGOD)
    return (FALSE);
/*
 *     if (CMD_IS("gohome"))
 *         send_to_char(ch, "%s", buf3);
 *             return (TRUE);
 *             */
    send_to_char(ch, "%s", buf);
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);

  return (FALSE);
}

SPECIAL(blocker)// This guy blocks any nod god chars from crossing the ocean.
{
  struct char_data *blocker = (struct char_data *)me;
  const char *buf2 = "&R$n can't seem to get through.&n\r\n";

  if (!IS_MOVE(cmd) || AFF_FLAGGED(blocker, AFF_BLIND))
    return (FALSE);

  if (GET_LEVEL(ch) >= LVL_GRGOD || IS_NPC(ch))
    return (FALSE);
/*
 *     if (CMD_IS("gohome"))
 *         send_to_char(ch, "%s", buf3);
 *             return (TRUE);
 *             */
    send_to_char(ch, "%s stops you from passing.\r\n", GET_NAME(blocker));
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);

  return (FALSE);
}

SPECIAL(spider)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING || !FIGHTING(ch))
    return (FALSE);

  if (IN_ROOM(FIGHTING(ch)) != IN_ROOM(ch) || rand_number(0, GET_LEVEL(ch)) != 0
)
    return (FALSE);

  act("&R$n sinks its fangs deep into $N!&n", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
  act("&R$n sinks its fangs deeply into your flesh!", 1, ch, 0, FIGHTING(ch), TO_VICT);
  call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
  return (TRUE);
}

SPECIAL(cricket)
{
  char actbuf[MAX_INPUT_LENGTH];

  if (cmd)
    return (FALSE);

  switch (rand_number(0, 10)) {
  case 0:
    do_say(ch, strcpy(actbuf, "Chirp."), 0, 0);
    return (TRUE);
  case 1:
    do_say(ch, strcpy(actbuf, "Chirp chirp."), 0, 0);        /* strcp
y: OK */
    return (TRUE);
  case 2:
    do_say(ch, strcpy(actbuf, "Chirpa chirpa."), 0, 0);    /* strcpy: OK */
    return (TRUE);
  case 3:
    do_say(ch, strcpy(actbuf, "Chip chir, chip chir"), 0, 0);
/* strcpy: OK */
    return (TRUE);
  case 4:
    do_say(ch, strcpy(actbuf, "Chip chip chir. Chip chip chi chir chir. Chip chippity chip chippy chippy chip chir chirp."), 0, 0);
    return (TRUE);
  case 5:
    do_say(ch, strcpy(actbuf, "Chir chip, chir chip."), 0, 0);
    return (TRUE);
  default:
    return (FALSE);
  }
}

SPECIAL(tmech)
{
  char actbuf[MAX_INPUT_LENGTH];
  if (cmd)
    return (FALSE);

  switch (rand_number(0, 20)) {
  case 0:
    act("&G$n scans the area for intruders.&n", FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 1:
    act("&G$n emits a red laser and performs a wide scan of the area.&n",
    FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 5:
    do_say(ch, strcpy(actbuf, "All systems normal."), 0, 0);
    return (TRUE);
  case 10:
    act("&G$n performs a quick uplink to the main system database.&n", FALSE,
    ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 15:
    act("&G$n connects itself to a wall port, for a quick system upgrade.&n",
    FALSE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  case 20:
    do_say(ch, strcpy(actbuf, "All system scans show no intruder activity."),
    0, 0);
    return (TRUE);
  default:
    return (FALSE);
  }
}

SPECIAL(hunter)
{

//struct char_data *tch;
//
//
////  if (cmd || !AWAKE(ch) || FIGHTING(ch))
////    return (FALSE);
//
////  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
////  {
////    if (!IS_NPC(tch))
////      return(TRUE);
//
//  //  if(IS_SET(MOB_FLAGS(tch), MOB_PREY) && GET_POS(tch) < POS_STUNNED && GET_LEVEL(ch) > GET_LEVEL(tch))
//    //  {
//       //   if (rand_number(1, 60) > 40)
//       //      act("&C$n screams 'MMMMMMM.....dinner time!!!'&n", FALSE, ch, 0, 0, TO_ROOM);
//         //    hit(ch, tch, TYPE_UNDEFINED);
//          //     return(TRUE);
//          //    }
//
//          //  }
 return (FALSE);
}

SPECIAL(pblade)
{
  if (cmd || GET_POS(ch) != POS_FIGHTING || !FIGHTING(ch))
    return (FALSE);

  if (IN_ROOM(FIGHTING(ch)) != IN_ROOM(ch) || rand_number(0, GET_LEVEL(ch)) != 0
)
    return (FALSE);

  act("&R$N's blade stings $n!&n", 1, ch, 0, FIGHTING(ch), TO_NOTVICT
);
  act("&R$N's blade stings you!&n", 1, ch, 0, FIGHTING(ch), TO_VICT);
  call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
  return (TRUE);
}
SPECIAL(soul_master)
{

    if (CMD_IS("blaze"))
    {

        if (GET_SOULS(ch) >= GET_LEVEL(ch))
        {
          GET_SOULS(ch) -= GET_LEVEL(ch);
          send_to_char(ch, "You see the white light fade into the distance...");
          char_from_room(ch);
          char_to_room(ch, real_room(106));
          GET_HIT(ch) = GET_MAX_HIT(ch) / 2;
          GET_MANA(ch) = GET_MAX_MANA(ch) / 2;
          GET_MOVE(ch) = GET_MAX_MOVE(ch) / 2;
          REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_AFTERLIFE);
          REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_DYING);
          update_pos(ch);
          look_at_room(ch, 0);
          return TRUE;
        }
        else
        {
           send_to_char(ch, "You don't have enough souls for that");
           return TRUE;
        }
      }
      else if (CMD_IS("train"))
      {
        if (GET_SOULS(ch) >= 500)
        {
          GET_SOULS(ch) -= 500;
          send_to_char(ch, "&WThe necromancer looks at you, makes a gesture, and you feel magic coursing through your veins.&n");

          if (GET_STR(ch) < 20)
          {
             ch->real_abils.str = GET_STR(ch) + 1;
             affect_total(ch);
          }
          if (GET_INT(ch) < 20)
          {
             ch->real_abils.intel = GET_INT(ch) + 1;
             affect_total(ch);
          }
          if (GET_WIS(ch) < 20)
          {
             ch->real_abils.wis = GET_WIS(ch) + 1;
             affect_total(ch);
          }
          if (GET_DEX(ch) < 20)
          {
             ch->real_abils.dex = GET_DEX(ch) + 1;
             affect_total(ch);
          }
          if (GET_CON(ch) < 20)
          {
             ch->real_abils.con = GET_CON(ch) + 1;
             affect_total(ch);
          }
          if (GET_CHA(ch) < 20)
          {
             ch->real_abils.cha = GET_CHA(ch) + 1;
             affect_total(ch);
          }
          return TRUE;
        }
        else
        {
        send_to_char(ch, "You don't have enough souls for that");
           return TRUE;
        }
      }
      else
      {
      return FALSE;
    }
}

SPECIAL(ramona)
{
   int flee_room1=11118, flee_room2=11111, flee_room3=11103;
   long int k1=11101, k2=11102, k3=11103, onum;
   struct obj_data *key;
   char buf[MAX_STRING_LENGTH];
   room_rnum location;

   if (GET_HIT(ch) < (GET_MAX_HIT(ch) /2) && GET_ROOM_VNUM(IN_ROOM(ch)) != 11118 && GET_ROOM_VNUM(IN_ROOM(ch)) != 11111 && GET_ROOM_VNUM(IN_ROOM(ch)) != 11103)
   {
    snprintf(buf, sizeof(buf), "%d", flee_room1);
    act("&c$n dissappears in a cloud of smoke.&n", FALSE, ch, 0, 0, TO_ROOM);
    onum = real_object(k1);
    key = read_object(onum, REAL);
    obj_to_room(key, IN_ROOM(ch));
    location = find_target_room(ch, buf);
    char_from_room(ch);
    char_to_room(ch, location);
    GET_HIT(ch) += (GET_MAX_HIT(ch) / 3);
    return (TRUE);
   }

   if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 3) && GET_ROOM_VNUM(IN_ROOM(ch)) != 11111 && GET_ROOM_VNUM(IN_ROOM(ch)) != 11103)
   {
    snprintf(buf, sizeof(buf), "%d", flee_room2);
    act("&c$n dissappears in a cloud of smoke.&n", FALSE, ch, 0, 0, TO_ROOM);
    onum = real_object(k2);
    key = read_object(onum, REAL);
    obj_to_room(key, IN_ROOM(ch));
    location = find_target_room(ch, buf);
    char_from_room(ch);
    char_to_room(ch, location);
    GET_HIT(ch) += (GET_MAX_HIT(ch) / 4);
    return (TRUE);
   }

   if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 4) && GET_ROOM_VNUM(IN_ROOM(ch)) != 11118 && GET_ROOM_VNUM(IN_ROOM(ch)) != 11103)
   {
    snprintf(buf, sizeof(buf), "%d", flee_room3);
    act("&c$n dissappears in a cloud of smoke.&n", FALSE, ch, 0, 0, TO_ROOM);
    onum = real_object(k3);
    key = read_object(onum, REAL);
    obj_to_room(key, IN_ROOM(ch));
    location = find_target_room(ch, buf);
    char_from_room(ch);
    char_to_room(ch, location);
    GET_HIT(ch) += (GET_MAX_HIT(ch) / 2);
    return (TRUE);
   }
    struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (FALSE);

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !rand_number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
    vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
  {
    cast_spell(ch, ch, NULL, SPELL_SANCTUARY);
    GET_MANA(ch) += 75;
    return (TRUE);
  }

  if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 5))
     cast_spell(ch, ch, NULL, SPELL_HEAL);


  switch (GET_LEVEL(ch)) {
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_AURA_BOLT);
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_MANA_BLAST);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_HARM);
    break;
  }

return(FALSE);
}

SPECIAL(marshall_mirror)
{
  int k1=11201, onum;
  struct obj_data *key;

  if (CMD_IS("look"))
  {
   if (GET_ROOM_VNUM(IN_ROOM(ch)) == 11232)
   {
       onum = real_object(k1);
       key = read_object(onum, REAL);
       obj_to_char(key, ch);
       send_to_char(ch, "\r\n&YIn the mirror you notice something run behind you. As you turn around to see what it was, it's gone. You notice something in your pocket.\r\n&n");
       return(TRUE);
   }
  }

return(TRUE);
}

SPECIAL(doctor)
{
   /* Special Procedure  - meta_physician                */
   /* Player Characters can train stats or gain          */
   /* health/mana via a metaphysician                    */
   /* Muerte of ButterMud - Telnet://betterbox.net:4000  */

 char meta_type[256]= {"     "}; /* Array for meta type  name */
 long meta_gold = 0;
 long meta_qp   = 0;
 long hp=0,mana=0,move=0;
 /* Parse command parm and set meta cost accordingly */

  if (CMD_IS("meta")) {
     argument = one_argument(argument, meta_type);

      if (strcmp(meta_type,"health")==0) {
        meta_gold = 750000;
        meta_qp  = 750;}

      if (strcmp(meta_type, "mana")==0) {
        meta_gold = 750000;
        meta_qp  = 750;}

      if (strcmp(meta_type, "move")==0) {
        meta_gold = 750000;
        meta_qp  = 750;}

      if (strcmp(meta_type, "str")==0) {
        meta_gold = 750000;
        meta_qp  = 500;}

      if (strcmp(meta_type, "int")==0) {
        meta_gold = 750000;
        meta_qp  = 500;}

      if (strcmp(meta_type, "wis")==0) {
        meta_gold = 750000;
        meta_qp  = 500;}

      if (strcmp(meta_type, "dex")==0) {
        meta_gold = 750000;
        meta_qp  = 500;}

      if (strcmp(meta_type, "con")==0) {
        meta_gold = 750000;
        meta_qp  = 500;}

      if (strcmp(meta_type, "cha")==0) {
        meta_gold = 750000;
        meta_qp  = 500;}
      /* Gold and Exp validity check */

      if (meta_gold > 300000) {

      if ((GET_GOLD(ch) < meta_gold)) {
          send_to_char(ch, "You don't have enough gold!\r\n");
          return (TRUE);}

      if ((GET_QUESTPOINTS(ch) < meta_qp)) {
          send_to_char(ch, "You haven't enough quest points!\r\n");
          return (TRUE);}

      /* Extract Cash and experience */

       GET_GOLD(ch) -= meta_gold;
       GET_QUESTPOINTS(ch) -= meta_qp;
       send_to_char(ch, "The MetaPhysician accepts your payment and begins the procedure... \r\n");

      /* Boost Stats */

      if (strcmp(meta_type,"health")==0) {
       if(GET_META_HP(ch) < 250)
       {
         hp = rand_number(1, 10);
         GET_MAX_HIT(ch) += hp;
         GET_META_HP(ch) += hp;
         send_to_char(ch, "Your vitality increases!\r\n");
       }
       else
          send_to_char(ch, "Sorry, you can't gain anymore health here.\r\n");
      return 1;}

      if (strcmp(meta_type,"mana")==0) {
       if(GET_META_MANA(ch) < 250)
       {
          mana = rand_number(1, 10);
          GET_MAX_MANA(ch) += mana;
          GET_META_MANA(ch) += mana;
          send_to_char(ch, "You feel a surge in magical power!\r\n");
       }
       else
          send_to_char(ch, "Sorry, you can't gain anymore mana here.\r\n");
      return 1;}

      if (strcmp(meta_type,"move")==0) {
       if(GET_META_MV(ch) < 250)
       {
          move = rand_number(1, 10);
          GET_MAX_MOVE(ch) += move;
          GET_META_MV(ch) += move;
          send_to_char(ch, "You feel like running!\r\n");
      }
       else
          send_to_char(ch, "Sorry, you can't gain anymore moves here.\r\n");
      return 1;}

      if (strcmp(meta_type,"str")==0) {
       if (GET_STR(ch) <= 19) {
        ch->real_abils.str = GET_STR(ch) + 1;
        send_to_char(ch, "You feel a sudden surge in your strength.\r\n");
        affect_total(ch);
        return 1;} else
        send_to_char(ch, "You are as naturally strong as you can be.\r\n");
      }
      if (strcmp(meta_type,"int")==0) {
       if (GET_INT(ch) < 20) {
        ch->real_abils.intel = GET_INT(ch) + 1;
        send_to_char(ch, "You feel an increase in your understanding.\r\n");
	affect_total(ch);
        return 1;} else
        send_to_char(ch, "Sorry, you can understand no more.\r\n");
      }
      if (strcmp(meta_type,"wis")==0) {
       if (GET_WIS(ch) < 20) {
	ch->real_abils.wis = GET_WIS(ch) + 1;
        send_to_char(ch, "You suddenly feel more wise.\r\n");
        affect_total(ch);
        return 1;} else
        send_to_char(ch, "Sorry, we cannot make you any more wise.\r\n");
      }
      if (strcmp(meta_type,"dex")==0) {
       if (GET_DEX(ch) < 20) {
        ch->real_abils.dex = GET_DEX(ch) + 1;
        send_to_char(ch, "You feel suddenly more agile!\r\n");
        affect_total(ch);
        return 1;} else
        send_to_char(ch, "Sorry, but you are allready as agile as can be.\r\n");
      }
      if (strcmp(meta_type,"con")==0)  {
       if (GET_CON(ch) < 20) {
        ch->real_abils.con = GET_CON(ch) + 1;
        send_to_char(ch, "You feel suddenly more sturdy.\r\n");
        affect_total(ch);
        return 1;} else
        send_to_char(ch, "Sorry, you are as hardy an individual as can be.\r\n");
      }
      if (strcmp(meta_type,"cha")==0)  {
       if (GET_CHA(ch) < 20) {
        ch->real_abils.cha = GET_CHA(ch) + 1;
        send_to_char(ch, "You social engineering skills increase!\r\n");
        affect_total(ch);
        return 1;} else
        send_to_char(ch, "Sorry, your charisma stuns me allready.\r\n");
      }
      }

     /* If it gets this far, show them the menu */

      send_to_char(ch, "Select an operation from the following...\r\n");
      send_to_char(ch, "_____________________________ \r\n");
      send_to_char(ch, "Meta Operation    Qpnts  Gold \r\n");
      send_to_char(ch, "_____________________________ \r\n");
      send_to_char(ch, "health            750    750k \r\n");
      send_to_char(ch, "mana              750    750k \r\n");
      send_to_char(ch, "move              750    750k \r\n");
      send_to_char(ch, "------ \r\n");
      send_to_char(ch, "str               500    750k \r\n");
      send_to_char(ch, "int               500    750k \r\n");
      send_to_char(ch, "wis               500    750k \r\n");
      send_to_char(ch, "dex               500    750k \r\n");
      send_to_char(ch, "con               500    750k \r\n");
      send_to_char(ch, "cha               500    750k \r\n");
      GET_GOLD(ch) += meta_gold;
      GET_QUESTPOINTS(ch) += meta_qp;
      return 1; }

      return 0;
}

SPECIAL(warrior)
{
  struct char_data *vict;
  int att_type = 0;
 
  if (cmd || !AWAKE(ch))
    return FALSE;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);          

      switch(att_type) {  
       case 1: case 2: case 3: case 4:
	 /* bash */
	 do_mob_bash(ch, vict);
        break;
      case 5: case 6: case 7: case 8: case 9: case 10:
	do_generic_skill(ch, vict, SKILL_KICK);
	break;

      case 11:
	do_generic_skill(ch, vict, SKILL_BASH);
	break;
      case 12: case 13:     
	  do_mob_disarm(ch, vict);
	break; 
      case 14: case 15: case 16:
	do_generic_skill(ch, vict, SKILL_KICK);
	break;
      case 17: 
	do_generic_skill(ch, vict, SKILL_BASH);
	break;
      default:
        break;
      }
  
    }
  }
  return FALSE;  
}

SPECIAL(goddess)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[5];
  int recall=6300;
  room_rnum location;

  if (CMD_IS("look")) {
     argument = one_argument(argument, arg);

      if (strcmp(arg,"goddess")==0) {
        snprintf(buf, sizeof(buf), "%d", recall);
        location = find_target_room(ch, buf);
        send_to_char(ch, "&WThe Goddess waves her hand, and the room spins madly...&n\r\n");
        char_from_room(ch);
        char_to_room(ch, location);
      }
  }

  return 0;
}

