//
//   File: act.offensive.c                                                 //
//   Usage: player-level commands of an offensive nature                   //

#include "conf.h"
#include "sysdep.h"

#include "screen.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

// extern functions 
void raw_kill(struct char_data *ch, struct char_data * killer);
void check_killer(struct char_data *ch, struct char_data *vict);
int compute_armor_class(struct char_data *ch);
void sportschan(char *);
extern int arena_ok(struct char_data *ch, struct char_data *victim);
bool attack_building(CHAR_DATA *ch, char *arg);

// local functions 
ACMD(do_assist);
ACMD(do_hit);
ACMD(do_kill);
ACMD(do_backstab);
ACMD(do_order);
ACMD(do_flee);
ACMD(do_bash);
ACMD(do_trip);
ACMD(do_rescue);
ACMD(do_throw);
ACMD(do_kick);
ACMD(do_disarm);
ACMD(do_tripwire);
ACMD(do_slay);
ACMD(do_charge);
ACMD(do_defend);

void do_stomp(struct char_data *ch, struct char_data *vict);

ACMD(do_assist)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char(ch, "You're already fighting!  How can you assist someone else?\r\n");
    return;
  }

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Whom do you wish to assist?\r\n");
  else if (!(helpee = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (helpee == ch)
    send_to_char(ch, "You can't help yourself any more than this!\r\n");
  else {
    //
    // Hit the same enemy the person you're helping is.
    //
    if (FIGHTING(helpee))
      opponent = FIGHTING(helpee);
    else
      for (opponent = world[IN_ROOM(ch)].people;
	   opponent && (FIGHTING(opponent) != helpee);
	   opponent = opponent->next_in_room)
		;

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
         // prevent accidental pkill 
    else if (!CONFIG_PK_ALLOWED && !IS_NPC(opponent) && !arena_ok(ch, opponent)	)	
      act("Use 'murder' if you really want to attack $N.", FALSE,
	  ch, 0, opponent, TO_CHAR);
    else {
      send_to_char(ch, "You join the fight!\r\n");
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}


ACMD(do_hit)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict=NULL;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Hit who?\r\n");
  else if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
  {
                /* otherwise, check for buildings */
    if ( attack_building(ch, str_dup(arg)) )
       return;

    if ( attack_vehicle(ch, str_dup(arg)) )
       return;

    send_to_char(ch, "They don't seem to be here.\r\n");
    return;
  }
  else if (vict == ch) {
    send_to_char(ch, "You hit yourself...OUCH!.\r\n");
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
  {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
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
  else {
    if (AFF_FLAGGED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict))
	return;		// you can't order a charmed pet to attack a
			// player
    }
//    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)){
//      sprintf(buf2, "%s and %s engage in mortal combat", GET_NAME(ch), GET_NAME(vict));
//      sportschan(buf2);
//     }

  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL) && vict != NULL) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch)) && (ch != vict)) {
      hit(ch, vict, TYPE_UNDEFINED);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char(ch, "You do the best you can!\r\n");
 
}



ACMD(do_kill)
{
  struct char_data *vict;

  if (!*argument) 
  {
    send_to_char(ch, "Kill who?");
  } 
  else 
  {
    if (GET_LEVEL(ch) < LVL_GOD || IS_NPC(ch))
    { 
     do_hit(ch, argument, cmd, subcmd);
    }
    else 
    {
    if (!(vict = get_char_vis(ch, argument, NULL, FIND_CHAR_ROOM)))
       send_to_char(ch, "They aren't here.\r\n");
    else if (ch == vict)
       send_to_char(ch, "Your mother would be so sad.. :(\r\n");
    else if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
       send_to_char(ch, "&GWhat did they ever do to you?\r\n");
       return;
    }
    else 
     {
       act("&RYou chop $M to pieces!  Ah!  The blood!&n", FALSE, ch, 0, vict, TO_CHAR);
       act("&R$N chops you to pieces!&n", FALSE, vict, 0, ch, TO_CHAR);
       act("&R$n brutally slays $N!&n", FALSE, ch, 0, vict, TO_NOTVICT);
       raw_kill(vict, ch);
     }
    }
  }
}

ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob, apr;
  char buf[MAX_INPUT_LENGTH];

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_BACKSTAB)) && GET_WAS_THIEF(ch) == 0) {
    send_to_char(ch, "You have no idea how to do that.\r\n");
    return;
  }

 
  one_argument(argument, buf);
  
  if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM))) {  
    send_to_char(ch, "Backstab who?\r\n");
    return;
  }
  if (vict == ch) {
    send_to_char(ch, "How can you sneak up on yourself?\r\n");
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char(ch, "You need to wield a weapon to make it a success.\r\n");
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char(ch, "Only piercing weapons can be used for backstabbing.\r\n");
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char(ch, "You can't backstab a fighting person -- they're too alert!\r\n");
    return;
  }

  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }
  
  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
      {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
              return;
            }
         }
       }
  }

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = rand_number(1, 101);	// 101% is a complete failure 
  prob = GET_SKILL(ch, SKILL_BACKSTAB);
  apr = 0;

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB);
  else
       if (GET_SKILL(ch, SKILL_SECOND_STAB) >= rand_number(1, 101) && GET_SKILL(ch, SKILL_ATTACK2) >= rand_number(1, 101)) 
       {
	 apr++;  
        if (GET_SKILL(ch, SKILL_ATTACK3) >= rand_number(1, 101) && (IS_THIEF(ch) || GET_LEVEL(ch) >= LVL_GOD))
        {
 	  apr++;
         if (GET_SKILL(ch, SKILL_FOURTH_STAB) >= rand_number(1, 101) && GET_LEVEL(ch) >= LVL_GOD)
         {
	   apr++;
         }
        }
       }

      apr = MAX(-1, MIN(apr, 6));

      if(GET_LEVEL(ch) > LVL_IMMORT)
        apr=10;
 
     if (apr >= 0) {
       for (; apr >= 0 && (percent < prob); apr--)
           hit(ch, vict, SKILL_BACKSTAB);
           if (GET_EQ(ch, WEAR_DWIELD) && GET_HIT(vict) > 0 && (rand_number( 1, 100 ) < GET_SKILL(ch, SKILL_DUAL_WIELD)) || IS_GOD(ch))
                double_hit(ch, vict, SKILL_BACKSTAB);

	 }
   WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}




/*
ACMD(do_backstab)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_BACKSTAB)) {
    send_to_char(ch, "You have no idea how to do that.\r\n");
    return;
  }

  one_argument(argument, buf);

  if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM))) {
    send_to_char(ch, "Backstab who?\r\n");
    return;
  }
  if (vict == ch) {
    send_to_char(ch, "How can you sneak up on yourself?\r\n");
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char(ch, "You need to wield a weapon to make it a success.\r\n");
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char(ch, "Only piercing weapons can be used for backstabbing.\r\n");
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char(ch, "You can't backstab a fighting person -- they're too alert!\r\n");
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = rand_number(1, 101);
  prob = GET_SKILL(ch, SKILL_BACKSTAB);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB);
  else
    hit(ch, vict, SKILL_BACKSTAB);

  WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
}
*/

ACMD(do_order)
{
  char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
  bool found = FALSE;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char(ch, "Order who to do what?\r\n");
  else if (!(vict = get_char_vis(ch, name, NULL, FIND_CHAR_ROOM)) && !is_abbrev(name, "followers"))
    send_to_char(ch, "That person isn't here.\r\n");
  else if (ch == vict)
    send_to_char(ch, "You obviously suffer from skitzofrenia.\r\n");
  else {
    if (AFF_FLAGGED(ch, AFF_CHARM)) {
      send_to_char(ch, "Your superior would not aprove of you giving orders.\r\n");
      return;
    }
    if (vict) {
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !AFF_FLAGGED(vict, AFF_CHARM))
	act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
	send_to_char(ch, "%s", CONFIG_OK);
	command_interpreter(vict, message);
      }
    } else {			// This is order "followers" 
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, 0, TO_ROOM);

      for (k = ch->followers; k; k = k->next) {
	if (IN_ROOM(ch) == IN_ROOM(k->follower))
	  if (AFF_FLAGGED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    command_interpreter(k->follower, message);
	  }
      }
      if (found)
	send_to_char(ch, "%s", CONFIG_OK);
      else
	send_to_char(ch, "Nobody here is a loyal subject of yours!\r\n");
    }
  }
}



ACMD(do_flee)
{
  int i, attempt;
  struct char_data *was_fighting;
  struct room_affect *raff;

  if (GET_POS(ch) < POS_FIGHTING) {
    send_to_char(ch, "You are in pretty bad shape, unable to flee!\r\n");
    return;
  }

  for (i = 0; i < 6; i++) {
    attempt = rand_number(0, NUM_OF_DIRS - 1);	// Select a random direction 
    if (CAN_GO(ch, attempt) &&
	!ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH)) {
       if (IS_NPC(ch))
         for (raff = world[EXIT(ch, attempt)->to_room].room_affs; raff; raff = raff->next)
           if (IS_NPC(ch) && raff->type == RAFF_NPC_BARRIER) {
             send_to_char(ch, "You fail to flee in that direction due to a magical barrier blocking your way.\r\n");
             return;
           }
      act("&R$n panics, and attempts to flee!&n", TRUE, ch, 0, 0, TO_ROOM);
      was_fighting = FIGHTING(ch);
      if (do_simple_move(ch, attempt, TRUE)) {
	send_to_char(ch, "&RYou flee head over heels.&n\r\n");
        stop_fighting(ch);
      } else {
	act("&R$n tries to flee, but can't!&n", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  }
  send_to_char(ch, "&RPANIC!  You couldn't escape!&n\r\n");
}


ACMD(do_bash)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob, dam;

  one_argument(argument, arg);

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_BASH)) && GET_WAS_WARRIOR(ch) == 0) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char(ch, "You need to wield a weapon to make it a success.\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char(ch, "Bash who?\r\n");
      return;
    }
  }
  if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  }
  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }
  if(OBJ_FLAGGED(GET_EQ(ch, WEAR_WIELD), ITEM_BOW)) {
    send_to_char(ch, "&GYou cannot bash with a bow.\r\n");
    return;
  }
  if(OBJ_FLAGGED(GET_EQ(ch, WEAR_WIELD), ITEM_SLING)) {
    send_to_char(ch, "&GYou cannot bash with a sling.\r\n");
    return;
  }

  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
      {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
              return;
            }
         }
       }
  }

//  percent = rand_number(1, 101);	// 101% is a complete failure 
//  prob = GET_SKILL(ch, SKILL_BASH);

  percent = rand_number(1,6) + GET_DEX(vict) - 14;        // 101% is a complete failure
  prob = rand_number(1,6) + GET_DEX(ch) - 14;

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BASH);
    GET_POS(ch) = POS_SITTING;
    if (GET_SKILL(ch, SKILL_ADVANCED_BASH < 75))
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  } else {
    dam = GET_DAMROLL(ch) + GET_STR(ch);
   //
   // If we bash a player and they wimp out, they will move to the previous
   // room before we set them sitting.  If we try to set the victim sitting
   // first to make sure they don't flee, then we can't bash them!  So now
   // we only set them sitting if they didn't flee. -gg 9/21/98
   //
    if (damage(ch, vict, dam, SKILL_BASH) > 0) {	/* -1 = dead, 0 = miss */
      WAIT_STATE(vict, PULSE_VIOLENCE);
      if (IN_ROOM(ch) == IN_ROOM(vict))
        GET_POS(vict) = POS_SITTING;
        if (GET_SKILL(ch, SKILL_STOMP) && GET_SKILL(ch, SKILL_KICK))
           do_stomp(ch, vict);
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_trip)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob, dam;

  one_argument(argument, arg);

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_TRIP)) && GET_WAS_THIEF(ch) == 0) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char(ch, "Trip who?\r\n");
      return;
    }
  }
  if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  }
  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
      {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
      {

         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
              return;
            }
         }
       }
  }

  percent = rand_number(1, 101);        // 101% is a complete failure
  prob = GET_SKILL(ch, SKILL_TRIP);
  
//  if (MOB_FLAGGED(vict, MOB_NOTRIP))
//    percent = 101;

  if (GET_DEX(ch) < GET_DEX(vict))
     percent = 101;

  if (rand_number(1,100) < GET_SKILL(ch, SKILL_ADVANCED_TRIP) /2 )
     dam = GET_DAMROLL(ch) / 2;
  else if (rand_number(1,100) < 50 && IS_NPC(ch) && GET_MCLASS(ch) == MCLASS_ASSASSIN)
     dam = GET_DAMROLL(ch) / 2;
  else
     dam = 1;

  if (percent > (prob - 20)) {
    damage(ch, vict, 0, SKILL_TRIP);
  } else {
    if (damage(ch, vict, dam, SKILL_TRIP) > 0) {  /* -1 = dead, 0 = miss */
      WAIT_STATE(vict, PULSE_VIOLENCE);
      if (IN_ROOM(ch) == IN_ROOM(vict))
        GET_POS(vict) = POS_SITTING;
        if (GET_SKILL(ch, SKILL_STOMP) && GET_SKILL(ch, SKILL_KICK) && IS_KNIGHT(ch))
           do_stomp(ch, vict);
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_stomp(struct char_data *ch, struct char_data *vict)
{
    int chance;
   
    chance = rand_number(1, 100);
    
    if (GET_LEVEL(ch) >= LVL_GOD)
    {
    act("&GYou &RSTOMP &G$N while they are down!&n", FALSE, ch, 0, vict, TO_CHAR);
    act("&GYou have been &RSTOMPED &Gby $n!&n", FALSE, ch, 0, vict, TO_VICT);
    act("&G$n &RSTOMPS &G$N while they are down!&n", FALSE, ch, 0, vict, TO_NOTVICT);
    GET_HIT(vict) = GET_HIT(vict) - 100;
    act("&GYou &RSTOMP &G$N while they are down!&n", FALSE, ch, 0, vict, TO_CHAR);
    act("&GYou have been &RSTOMPED &Gby $n!&n", FALSE, ch, 0, vict, TO_VICT);
    act("&G$n &RSTOMPS &G$N while they are down!&n", FALSE, ch, 0, vict, TO_NOTVICT);

    GET_HIT(vict) = GET_HIT(vict) - 100;
    act("&GYou &RSTOMP &G$N while they are down!&n", FALSE, ch, 0, vict, TO_CHAR);
    act("&GYou have been &RSTOMPED &Gby $n!&n", FALSE, ch, 0, vict, TO_VICT);
    act("&G$n &RSTOMPS &G$N while they are down!&n", FALSE, ch, 0, vict, TO_NOTVICT);
    GET_HIT(vict) = GET_HIT(vict) - 100;
    }
    if (chance > 50)
    {
     chance = 0;     
     chance = rand_number(1, 100);
     act("&GYou &RSTOMP &G$N while they are down!&n", FALSE, ch, 0, vict, TO_CHAR);
    act("&GYou have been &RSTOMPED &Gby $n!&n", FALSE, ch, 0, vict, TO_VICT);
    act("&G$n &RSTOMPS &G$N while they are down!&n", FALSE, ch, 0, vict, TO_NOTVICT);
    GET_HIT(vict) = GET_HIT(vict) - (((GET_STR(ch) * GET_STR(ch)) + GET_DAMROLL(ch))/10);
    }

}


ACMD(do_rescue)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_RESCUE)) && GET_WAS_WARRIOR(ch) == 0) {
    send_to_char(ch, "You have no idea how to do that.\r\n");
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    send_to_char(ch, "Whom do you want to rescue?\r\n");
    return;
  }
  if (vict == ch) {
    send_to_char(ch, "What about fleeing instead?\r\n");
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char(ch, "How can you rescue someone you are trying to kill?\r\n");
    return;
  }
  for (tmp_ch = world[IN_ROOM(ch)].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  percent = rand_number(1, 101);	// 101% is a complete failure 
  prob = GET_SKILL(ch, SKILL_RESCUE);

  if (percent > prob) {
    send_to_char(ch, "You fail the rescue!\r\n");
    return;
  }
  send_to_char(ch, "Banzai!  To the rescue...\r\n");
  act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
  act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

  if (FIGHTING(vict) == tmp_ch)
    stop_fighting(vict);
  if (FIGHTING(tmp_ch))
    stop_fighting(tmp_ch);
  if (FIGHTING(ch))
    stop_fighting(ch);

  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);

  WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
}

ACMD(do_tripwire)
{
  struct char_data *vict = NULL;
  int percent, prob;

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_KICK)) && GET_WAS_WARRIOR(ch) == 0) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }

  if (!IS_NPC(ch))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to set a tripwire.\r\n");
              return;
            }
         }
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_TRIPWIRE)){
    send_to_char(ch, "There is already a trip wire here.");
    return;
  }

  percent = rand_number(1, 101);
  prob = GET_SKILL(ch, SKILL_TRIPWIRE) / 2;

  if(percent > prob)
  {
     send_to_char(ch, "You set a trip wire in the room");
     act("$n sets a trip wire.", TRUE, vict, 0, 0, TO_ROOM);
     SET_BIT_AR(ROOM_FLAGS(IN_ROOM(ch)), ROOM_TRIPWIRE);
     GET_ROOM_TRIPPER(IN_ROOM(ch)) = ch;
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 5);
}

ACMD(do_kick)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob, dam, roll;

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_KICK)) && GET_WAS_WARRIOR(ch) == 0) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }
  one_argument(argument, arg);


  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char(ch, "Kick who?\r\n");
      return;
    }
  }

  if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  }

  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
      {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
              return;
            }
         }
       }
  }

  // 101% is a complete failure 
  percent = ((10 - (compute_armor_class(vict) / 10)) * 2) + rand_number(1, 101);
  prob = GET_SKILL(ch, SKILL_KICK);
  dam = (GET_STR(ch) * GET_STR(ch)) + GET_DAMROLL(ch);

  if (percent > 65)
    percent = 101;
  else if (percent > 55 && percent < 66)
    dam = dam * .6;
  else if (percent > 40 && percent < 56)
    dam = dam * .8;
  else if (percent > 25 && percent < 41)
    dam = dam;
  else 
    dam = dam * 1.2;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_KICK);
  } else
  {
    if (IS_KNIGHT(ch))
    {
      percent = ((10 - (compute_armor_class(vict) / 10)) * 2) + rand_number(1, 101);
      prob = GET_SKILL(ch, SKILL_ADVANCED_KICK);
      damage(ch, vict, dam/5, SKILL_KICK);
    }
    else if (IS_NPC(ch) && GET_MCLASS(ch) == MCLASS_KNIGHT)
    {
      percent = ((10 - (compute_armor_class(vict) / 10)) * 2) + rand_number(1, 101);
      prob = GET_SKILL(ch, SKILL_ADVANCED_KICK);
      damage(ch, vict, dam, SKILL_KICK);
    }
    else   
      damage(ch, vict, dam / 10, SKILL_KICK);
  }  
 
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_disarm)
{
  int goal, roll;
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *weap; 
  struct char_data *vict;
  int success = FALSE;
 
  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_DISARM)) && GET_WAS_WARRIOR(ch) == 0) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
      vict = FIGHTING(ch);
    else {
      send_to_char(ch, "Disarm who?\r\n");
      return;
    }
  }
  if (vict == ch) {
    send_to_char(ch, "Try REMOVE and DROP instead...\r\n");
    return;
  }

  weap = GET_EQ(vict, WEAR_WIELD);
  if (!weap) {
    send_to_char(ch, "But your opponent is not wielding a weapon!\r\n");
    return;      
  }

  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
      {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to disarm another player.\r\n");
              return;
            }
         }
       }
  }

  goal = GET_SKILL(ch, SKILL_DISARM) / 2;

  // Lots o' modifiers: 
  roll = rand_number(0, 101);
  roll -= GET_DEX(ch);   // Improve odds 
  roll += GET_DEX(vict); // Degrade odds 

  roll -= GET_LEVEL(ch);
  roll += GET_LEVEL(vict);

  roll += GET_OBJ_WEIGHT(weap);

  if (GET_LEVEL(vict) >= LVL_GOD) // No disarming an immort. 
    roll = 1000;
  if (GET_LEVEL(ch) >= LVL_GOD)   // But immorts never fail! 
    roll = -1000;

  if (roll <= goal) {
    success = TRUE;
    if ((weap = GET_EQ(vict, WEAR_WIELD))) {
      if (IS_NPC(vict))

      act("You disarm $p from $N's off-hand!", FALSE, ch, weap, vict, TO_CHAR);
      act("$n disarms $p from your off-hand!", FALSE, ch, weap, vict, TO_VICT);
      act("$n disarms $p from $N's off-hand!", FALSE, ch, weap, vict, TO_NOTVICT);
      obj_to_char(unequip_char(vict, WEAR_WIELD), vict);
    } else if ((weap = GET_EQ(vict, WEAR_WIELD))) {
      if (IS_NPC(vict))

      act("You disarm $p from $N's hand!", FALSE, ch, weap, vict, TO_CHAR);
      act("$n disarms $p from your hand!", FALSE, ch, weap, vict, TO_VICT); 
      act("$n disarms $p from $N's hand!", FALSE, ch, weap, vict, TO_NOTVICT);
      obj_to_char(unequip_char(vict, WEAR_WIELD), vict);
    } else {
      log("SYSERR: do_disarm(), should have a weapon to be disarmed, but lost it!");
    }
  } else {
      act("You fail to disarm $N.", FALSE, ch, weap, vict, TO_CHAR);
      act("$n fails to disarm you.", FALSE, ch, weap, vict, TO_VICT);
      act("$n fails to disarm $N.", FALSE, ch, weap, vict, TO_NOTVICT);
  }

  if (!GET_LEVEL(ch) >= LVL_GOD)
    WAIT_STATE(ch, PULSE_VIOLENCE);

  if (success && IS_NPC(vict))
    set_fighting(ch, vict);
}

ACMD(do_slay)
{
  struct char_data *vict;
  char buf[MAX_INPUT_LENGTH];

  one_argument(argument, buf);


  if (IS_NPC(ch)) {
     send_to_char(ch, "You can't slay. Try returning. \r\n");
     return;
   }
 
 if ((vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)) != NULL) {
    if(!IS_NPC(vict) && (GET_LEVEL(ch)) < LVL_GOD) {
     send_to_char(ch, "Go slay someone your own size. \r\n");
     return;
    }
  act("$n slays $N in cold &rblood&n. \r\n", FALSE, 0, 0, ch, TO_NOTVICT);
  act("You slay $N in cold &rblood&n. \r\n", TRUE, ch, 0, vict, TO_CHAR);
  act("$n slays you in cold &rblood&n. \r\n", FALSE, ch, 0, vict, TO_VICT);
  raw_kill(vict, ch);
  return;
  } 
  else 
   {
    send_to_char(ch, "Who do you wish to slay? \r\n");
    return;
   }
  if (IS_NPC(vict)) {
   extract_char(vict);
  }
}

ACMD(do_charge)
{
  struct char_data *victim, *next_victim;
  int dam, percent, prob;

  if (!ch)
    return;
  dam = percent = prob = 0;

  if (!IS_NPC(ch) && GET_WAS_WARRIOR(ch) == 0) {
    send_to_char(ch, "You better leave all the martial arts to the fighters!\r\n");
    return;
  }


  for (victim = world[ch->in_room].people; victim; victim = next_victim) 
  {
    next_victim = victim->next_in_room;

    if (victim == ch)
      continue;

    if (!IS_NPC(victim) && GET_LEVEL(victim) >= 1)
      continue;
    if (!IS_NPC(ch) && !IS_NPC(victim))
      continue;
    if (!IS_NPC(ch) && IS_NPC(victim) && AFF_FLAGGED(victim, AFF_CHARM))
      continue;
    if(IS_NPC(victim) && MOB_FLAGGED(victim, MOB_PEACEFUL))
      continue;
 

    percent = ((10 - (compute_armor_class(victim) / 10)) * 2) + rand_number(1, 101);
    prob = GET_SKILL(ch, SKILL_CHARGE);

  if (percent > prob) {
    damage(ch, victim, 0, SKILL_CHARGE);
  } else
  {
    if (IS_KNIGHT(ch))
    {
      percent = ((10 - (compute_armor_class(victim) / 10)) * 2) + rand_number(1, 101);
      prob = GET_SKILL(ch, SKILL_ADVANCED_CHARGE);
      damage(ch, victim, GET_LEVEL(ch), SKILL_CHARGE);
    }
    else if (IS_NPC(ch) && GET_MCLASS(ch) == MCLASS_KNIGHT)
    {
      percent = ((10 - (compute_armor_class(victim) / 10)) * 2) + rand_number(1, 101);
      prob = GET_SKILL(ch, SKILL_ADVANCED_CHARGE);
      damage(ch, victim, GET_LEVEL(ch), SKILL_CHARGE);
    }
    else
    {
      percent = ((10 - (compute_armor_class(victim) / 10)) * 2) + rand_number(1, 101);
      prob = GET_SKILL(ch, SKILL_ADVANCED_CHARGE);
      damage(ch, victim, GET_LEVEL(ch)/2, SKILL_CHARGE);
    }
    
    if (dam)
      GET_POS(victim) = POS_SITTING;

   }
  }
  if (!IS_NPC(ch))
    WAIT_STATE(ch, PULSE_VIOLENCE);
  
}
ACMD(do_throw)
{
  struct char_data *victim;
  struct obj_data *obj;
  int dam, percent, prob;
  char arg[MAX_STRING_LENGTH];

  dam = percent = prob = 0;

  if (!IS_NPC(ch) && GET_WAS_THIEF(ch) == 0) {
    send_to_char(ch,"You better leave throwing to thieves!\r\n");
    return;
  }

  one_argument(argument, arg);

  victim = get_char_room_vis(ch, arg, NULL);

  if (FIGHTING(ch))
    victim = FIGHTING(ch);

  if (!*arg) {
    send_to_char(ch,"Throw at whom?\r\n");
    return;
  }

  if (!victim) {
    send_to_char(ch,"Throw at whom?\r\n");
    return;
  }


  if (GET_EQ(ch, WEAR_HOLD)) {
    send_to_char(ch,"No free hand to throw with.\r\n");
    return;
  }
  
  if(!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
    send_to_char(ch,"You have nothing to throw.\r\n");
    return;

  }

  if(IS_NPC(victim) && MOB_FLAGGED(victim, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

  if (!IS_NPC(victim) && !IS_NPC(ch) && !arena_ok(ch, victim))
  {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(victim, PLR_OUTLAW))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
              return;
            }
         }
       }
  }

  percent = ((10 - (compute_armor_class(victim) / 10)) * 2) + rand_number(1, 101);
  prob = GET_SKILL(ch, SKILL_THROW);

  if (percent > prob) {
    damage(ch, victim, 0, SKILL_THROW);
  } else
  {
    if (IS_NINJA(ch))
    {
      percent = ((10 - (compute_armor_class(victim) / 10)) * 2) + rand_number(1, 101);
      prob = GET_SKILL(ch, SKILL_ADVANCED_THROW);
      dam = dice(GET_OBJ_VAL(obj,1), GET_OBJ_VAL(obj,2)) * rand_number(2,4);
      damage(ch, victim, dam, SKILL_THROW);
    }
    else
    {
      dam = dice(GET_OBJ_VAL(obj,1), GET_OBJ_VAL(obj,2));
      damage(ch, victim, dam, SKILL_THROW);
    }
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);

  if (dam) {
    obj_from_char(obj);
    obj_to_char(obj, victim);
  } else {
    obj_from_char(obj);
    obj_to_room(obj, ch->in_room);
  }
}

ACMD(do_defend)
{
  if (IS_NPC(ch))
    return;

  if (!FIGHTING(ch)) {
    send_to_char(ch, "But you aren't even fighting!\r\n");
    return;
  }

  GET_DEFEND(ch) += 3;
  WAIT_STATE(ch, PULSE_VIOLENCE*3);

}

ACMD(do_cripple)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob, dam, roll;

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_CRIPPLE))) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }

  if (!IS_NINJA(ch)) {
    send_to_char(ch, "You must be a ninja to cripple someone.\r\n");
    return;
  }

  one_argument(argument, arg);


  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char(ch, "Cripple who?\r\n");
      return;
    }
  }

  if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  }

  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
      {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
              return;
            }
         }
       }
  }
 
  mag_affects(GET_LEVEL(ch), ch, vict, SKILL_CRIPPLE, SAVING_SPELL);
  damage(ch, vict, GET_LEVEL(ch), SKILL_CRIPPLE);
}

ACMD(do_entangle)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int percent, prob, dam, roll;

  if ((IS_NPC(ch) || !GET_SKILL(ch, SKILL_ENTANGLE))) {
    send_to_char(ch, "You have no idea how.\r\n");
    return;
  }

  if (!IS_ROGUE(ch)) {
    send_to_char(ch, "You must be a rougea to entangle someone.\r\n");
    return;
  }

  one_argument(argument, arg);


  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char(ch, "Entangle who?\r\n");
      return;
    }
  }

  if (vict == ch) {
    send_to_char(ch, "Aren't we funny today...\r\n");
    return;
  }

  if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_PEACEFUL)) {
    send_to_char(ch, "&GWhat did they ever do to you?\r\n");
    return;
  }

  if (!IS_NPC(vict) && !IS_NPC(ch) && !arena_ok(ch, vict))
      {
     if (!PLR_FLAGGED(ch, PLR_OUTLAW) && !PLR_FLAGGED(vict, PLR_OUTLAW))
      {
         if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
         {
            if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
            {
              send_to_char(ch, "You must be in NPK or CPK to attack another player.\r\n");
              return;
            }
         }
       }
  }

  mag_affects(GET_LEVEL(ch), ch, vict, SKILL_ENTANGLE, SAVING_SPELL);
  damage(ch, vict, GET_LEVEL(ch)/4, SKILL_ENTANGLE);
}

