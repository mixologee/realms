/**************************************************************************
*  File: act.other.c                                       Part of tbaMUD *
*  Usage: Miscellaneous player-level commands.                             *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#define __ACT_OTHER_C__

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "house.h"
#include "constants.h"
#include "dg_scripts.h"
#include "quest.h"
#include "logger.h"

/* AUCTION STUFF*/
extern struct char_data *ch_selling;
extern struct char_data *ch_buying;
void ascii_strip_string(char *buffer);
void stop_auction(int type, struct char_data * ch);
/* end auction stuff*/

/* extern variables */
extern struct spell_info_type spell_info[];
extern const char *class_abbrevs[];
extern room_rnum r_mortal_start_room;
extern struct char_data *character_list;
extern struct room_data *world;

/* extern functions */
void list_skills(struct char_data *ch);
void appear(struct char_data *ch);
void perform_immort_vis(struct char_data *ch);
SPECIAL(shop_keeper);
ACMD(do_gen_comm);
void die(struct char_data *ch, struct char_data * killer);
void Crash_rentsave(struct char_data *ch, int cost);
int has_mail(long id);
void save_warnings(struct char_data *ch);
int compute_armor_class(struct char_data *ch);


/* local functions */
ACMD(do_quit);
ACMD(do_save);
ACMD(do_not_here);
ACMD(do_sneak);
ACMD(do_hide);
ACMD(do_steal);
ACMD(do_practice);
ACMD(do_visible);
ACMD(do_title);
ACMD(do_report);
ACMD(do_split);
ACMD(do_use);
ACMD(do_display);
ACMD(do_gen_tog);
ACMD(do_gen_write);

ACMD(do_choose_death);
ACMD(do_attack_castle);
ACMD(do_leader);
ACMD(do_meditate);
ACMD(do_posse);
ACMD(do_boot);
ACMD(do_castout);
ACMD(do_reelin);
ACMD(do_autoassist);
ACMD(do_autosplit);
ACMD(do_wimpy);

ACMD(do_quit)
{
  struct room_affect *raff;  

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS)) {
     for (raff = world[IN_ROOM(ch)].room_affs; raff; raff = raff->next) {
        if (raff->type == RAFF_SHELTER)
          break;
	else
	{
	  send_to_char(ch, "You need to start a campfire before quitting in the wilderness.\r\n");
	  return;
	}
     }
  }

  if (subcmd != SCMD_QUIT && GET_LEVEL(ch) < LVL_IMMORT)
    send_to_char(ch, "You have to type quit--no less, to quit!\r\n");
  else if (GET_POS(ch) == POS_FIGHTING)
    send_to_char(ch, "No way!  You're fighting for your life!\r\n");
  else if (GET_POS(ch) < POS_STUNNED) {
    send_to_char(ch, "You die before your time...\r\n");
    die(ch, NULL);
  } else {
    act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s has quit the game.", GET_NAME(ch));
//    mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s has quit the game.", GET_NAME(ch));

    if (GET_QUEST_TIME(ch) != -1)
      quest_timeout(ch);
 
    send_to_char(ch, "Goodbye, friend.. Come back soon!\r\n");


    if (ch == ch_selling)
                stop_auction(AUC_QUIT_CANCEL, NULL);

    /* We used to check here for duping attempts, but we may as well do it right
     * in extract_char(), since there is no check if a player rents out and it 
     * can leave them in an equally screwy situation. */

    if (CONFIG_FREE_RENT)
      Crash_rentsave(ch, 0);

    GET_LOADROOM(ch) = GET_ROOM_VNUM(IN_ROOM(ch));

    /* Stop snooping so you can't see passwords during deletion or change. */
    if (ch->desc->snoop_by) {
      write_to_output(ch->desc->snoop_by, "Your victim is no longer among us.\r\n");
      ch->desc->snoop_by->snooping = NULL;
      ch->desc->snoop_by = NULL;
    }

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOUSE) || !ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK) || !ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
      GET_LOADROOM(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
    else
      GET_LOADROOM(ch) = real_room(r_mortal_start_room);

    extract_char(ch);		/* Char is saved before extracting. */
  }
}

ACMD(do_save)
{
  if (IS_NPC(ch) || !ch->desc)
    return;

  send_to_char(ch, "Saving %s.\r\n", GET_NAME(ch));
  save_char(ch);
  save_warnings(ch);
  Crash_crashsave(ch);
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_HOUSE_CRASH))
    House_crashsave(GET_ROOM_VNUM(IN_ROOM(ch)));
  GET_LOADROOM(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
}

/* Generic function for commands which are normally overridden by special 
 * procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char(ch, "Sorry, but you cannot do that here!\r\n");
}

ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SNEAK)) {
    send_to_char(ch, "You have no idea how to do that.\r\n");
    return;
  }
  send_to_char(ch, "Okay, you'll try to move silently for a while.\r\n");
  if (AFF_FLAGGED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK);

  percent = rand_number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}

ACMD(do_hide)
{
  byte percent;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_HIDE)) {
    send_to_char(ch, "You have no idea how to do that.\r\n");
    return;
  }

  send_to_char(ch, "You attempt to hide yourself.\r\n");

  if (AFF_FLAGGED(ch, AFF_HIDE))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

  percent = rand_number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
    return;

  SET_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
}

ACMD(do_steal)
{
  struct char_data *vict;
  struct obj_data *obj;
  char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  int percent, gold, eq_pos, pcsteal = 0, ohoh = 0;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_STEAL)) {
    send_to_char(ch, "You have no idea how to do that.\r\n");
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
    return;
  }

  two_arguments(argument, obj_name, vict_name);

  if (!(vict = get_char_vis(ch, vict_name, NULL, FIND_CHAR_ROOM))) {
    send_to_char(ch, "Steal what from who?\r\n");
    return;
  } else if (vict == ch) {
    send_to_char(ch, "Come on now, that's rather stupid!\r\n");
    return;
  }

  /* 101% is a complete failure */
  percent = rand_number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if (GET_POS(vict) < POS_SLEEPING)
    percent = -1;		/* ALWAYS SUCCESS, unless heavy object. */

  if (!CONFIG_PT_ALLOWED && !IS_NPC(vict))
    pcsteal = 1;

  if (!AWAKE(vict))	/* Easier to steal from sleeping people. */
    percent -= 50;

  /* No stealing if not allowed. If it is no stealing from Imm's or Shopkeepers. */
  if (GET_LEVEL(vict) > LVL_IMMORT || pcsteal || GET_MOB_SPEC(vict) == shop_keeper)
    percent = 101;		/* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

    if (!(obj = get_obj_in_list_vis(ch, obj_name, NULL, vict->carrying))) {

      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	if (GET_EQ(vict, eq_pos) &&
	    (isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
	    CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
	  obj = GET_EQ(vict, eq_pos);
	  break;
	}
      if (!obj) {
	act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
	return;
      } else {			/* It is equipment */
	if ((GET_POS(vict) > POS_STUNNED)) {
	  send_to_char(ch, "Steal the equipment now?  Impossible!\r\n");
	  return;
	} else {
          if (!give_otrigger(obj, vict, ch) ||
              !receive_mtrigger(ch, vict, obj) ) {
            send_to_char(ch, "Impossible!\r\n");
            return;
          }
	  act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
	  obj_to_char(unequip_char(vict, eq_pos), ch);
	}
      }
    } else {			/* obj found in inventory */

      percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */

      if (percent > GET_SKILL(ch, SKILL_STEAL)) {
	ohoh = TRUE;
	send_to_char(ch, "Oops..\r\n");
	act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
      } else {			/* Steal the item */
	if (IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)) {
          if (!give_otrigger(obj, vict, ch) ||
              !receive_mtrigger(ch, vict, obj) ) {
            send_to_char(ch, "Impossible!\r\n");
            return;
          }
	  if (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char(ch, "Got it!\r\n");
	  }
	} else
	  send_to_char(ch, "You cannot carry that much.\r\n");
      }
    }
  } else {			/* Steal some coins */
    if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
      ohoh = TRUE;
      send_to_char(ch, "Oops..\r\n");
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, vict, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (GET_GOLD(vict) * rand_number(1, 10)) / 100;
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(vict) -= gold;
        if (gold > 1)
	  send_to_char(ch, "Bingo!  You got %d gold coins.\r\n", gold);
	else
	  send_to_char(ch, "You manage to swipe a solitary gold coin.\r\n");
      } else {
	send_to_char(ch, "You couldn't get any gold...\r\n");
      }
    }
  }

  if (ohoh && IS_NPC(vict) && AWAKE(vict))
    hit(vict, ch, TYPE_UNDEFINED);
}

ACMD(do_practice)
{
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (*arg)
    send_to_char(ch, "You can only practice skills in your guild.\r\n");
  else
    list_skills(ch);
}

ACMD(do_visible)
{
  if (GET_LEVEL(ch) > LVL_GOD) {
    perform_immort_vis(ch);
    return;
  }

  if AFF_FLAGGED(ch, AFF_INVISIBLE) {
    appear(ch);
    send_to_char(ch, "You break the spell of invisibility.\r\n");
  } else
    send_to_char(ch, "You are already visible.\r\n");
}

ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char(ch, "Your title is fine... go away.\r\n");
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char(ch, "You can't title yourself -- you shouldn't have abused it!\r\n");
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char(ch, "Titles can't contain the ( or ) characters.\r\n");
  else if (strlen(argument) > MAX_TITLE_LENGTH)
    send_to_char(ch, "Sorry, titles can't be longer than %d characters.\r\n", MAX_TITLE_LENGTH);
  else {
    set_title(ch, argument);
    send_to_char(ch, "Okay, you're now %s%s%s.\r\n", GET_NAME(ch), *GET_TITLE(ch) ? " " : "", GET_TITLE(ch));
  }
}

ACMD(do_report)
{
  char buf[MAX_STRING_LENGTH];


  snprintf(buf, sizeof(buf), "&G$n reports: &W%ld&G/&W%ld&GH&W  %ld&G/&W%ld&GM&W  %ld&G/&W%ld&GV &W%d&GAC  &W%d&GAl&n\r\n",
          GET_HIT(ch), GET_MAX_HIT(ch),
          GET_MANA(ch), GET_MAX_MANA(ch),
          GET_MOVE(ch), GET_MAX_MOVE(ch),
          compute_armor_class(ch), GET_ALIGNMENT(ch));

  act(buf, FALSE, ch, 0, 0, TO_ROOM);

  send_to_char(ch, "&GYou report: &W%ld&G/&W%ld&GH&W  %ld&G/&W%ld&GM&W  %ld&G/&W%ld&GV &W%d&GAC  &W%d&GAl&n\r\n",
          GET_HIT(ch), GET_MAX_HIT(ch),
          GET_MANA(ch), GET_MAX_MANA(ch),
          GET_MOVE(ch), GET_MAX_MOVE(ch),
          compute_armor_class(ch), GET_ALIGNMENT(ch));
}

ACMD(do_split)
{
  char buf[MAX_INPUT_LENGTH];
  int amount, num, share, rest;
  size_t len;
  struct char_data *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char(ch, "Sorry, you can't do that.\r\n");
      return;
    }
    if (amount > GET_GOLD(ch)) {
      send_to_char(ch, "You don't seem to have that much gold to split.\r\n");
      return;
    }
    k = (ch->master ? ch->master : ch);

    if (AFF_FLAGGED(k, AFF_GROUP) && (IN_ROOM(k) == IN_ROOM(ch)))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (IN_ROOM(f->follower) == IN_ROOM(ch)))
	num++;

    if (num && AFF_FLAGGED(ch, AFF_GROUP)) {
      share = amount / num;
      rest = amount % num;
    } else {
      send_to_char(ch, "With whom do you wish to share your gold?\r\n");
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    /* Abusing signed/unsigned to make sizeof work. */
    len = snprintf(buf, sizeof(buf), "%s splits %d coins; you receive %d.\r\n",
		GET_NAME(ch), amount, share);
    if (rest && len < sizeof(buf)) {
      snprintf(buf + len, sizeof(buf) - len,
		"%d coin%s %s not splitable, so %s keeps the money.\r\n", rest,
		(rest == 1) ? "" : "s", (rest == 1) ? "was" : "were", GET_NAME(ch));
    }
    if (AFF_FLAGGED(k, AFF_GROUP) && IN_ROOM(k) == IN_ROOM(ch) &&
		!IS_NPC(k) && k != ch) {
      GET_GOLD(k) += share;
      send_to_char(k, "%s", buf);
    }

    for (f = k->followers; f; f = f->next) {
      if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (IN_ROOM(f->follower) == IN_ROOM(ch)) &&
	  f->follower != ch) {

	GET_GOLD(f->follower) += share;
	send_to_char(f->follower, "%s", buf);
      }
    }
    send_to_char(ch, "You split %d coins among %d members -- %d coins each.\r\n",
	    amount, num, share);

    if (rest) {
      send_to_char(ch, "%d coin%s %s not splitable, so you keep the money.\r\n",
		rest, (rest == 1) ? "" : "s", (rest == 1) ? "was" : "were");
      GET_GOLD(ch) += rest;
    }
  } else {
    send_to_char(ch, "How many coins do you wish to split with your group?\r\n");
    return;
  }
}

ACMD(do_use)
{
  char buf[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  struct obj_data *mag_item;

  half_chop(argument, arg, buf);
  if (!*arg) {
    send_to_char(ch, "What do you want to %s?\r\n", CMD_NAME);
    return;
  }
  mag_item = GET_EQ(ch, WEAR_HOLD);

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      if (!(mag_item = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
	send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	return;
      }
      break;
    case SCMD_USE:
      send_to_char(ch, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      return;
    default:
      log("SYSERR: Unknown subcmd %d passed to do_use.", subcmd);
      /* SYSERR_DESC: This is the same as the unhandled case in do_gen_ps(), 
       * but in the function which handles 'quaff', 'recite', and 'use'. */
      return;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
      send_to_char(ch, "You can only quaff potions.\r\n");
      return;
    }
    break;
  case SCMD_RECITE:
    if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
      send_to_char(ch, "You can only recite scrolls.\r\n");
      return;
    }
    break;
  case SCMD_USE:
    if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_STAFF)) {
      send_to_char(ch, "You can't seem to figure out how to use it.\r\n");
      return;
    }
    break;
  }

  mag_objectmagic(ch, mag_item, buf);
}

ACMD(do_display)
{
  size_t i;

  if (IS_NPC(ch)) {
    send_to_char(ch, "Mosters don't need displays.  Go away.\r\n");
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char(ch, "Usage: prompt { { H | M | V } | all | auto | none }\r\n");
    return;
  }

  if (!str_cmp(argument, "auto")) {
    TOGGLE_BIT_AR(PRF_FLAGS(ch), PRF_DISPAUTO);
    send_to_char(ch, "Auto prompt %sabled.\r\n", PRF_FLAGGED(ch, PRF_DISPAUTO) ? "en" : "dis");
    return;
  }

  if (!str_cmp(argument, "on") || !str_cmp(argument, "all")) {
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
  } else if (!str_cmp(argument, "off") || !str_cmp(argument, "none")) {
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
  } else {
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);

    for (i = 0; i < strlen(argument); i++) {
      switch (LOWER(argument[i])) {
      case 'h':
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
	break;
      case 'm':
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
	break;
      case 'v':
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	break;
      default:
	send_to_char(ch, "Usage: prompt { { H | M | V } | all | auto | none }\r\n");
	return;
      }
    }
  }

  send_to_char(ch, "%s", CONFIG_OK);
}

ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp;
  const char *filename;
  struct stat fbuf;
  time_t ct;

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  default:
    return;
  }

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (IS_NPC(ch)) {
    send_to_char(ch, "Monsters can't have ideas - Go away.\r\n");
    return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char(ch, "That must be a mistake...\r\n");
    return;
  }
  mudlog(NRM, LVL_GOD, FALSE, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);

  if (stat(filename, &fbuf) < 0) {
    perror("SYSERR: Can't stat() file");
    /* SYSERR_DESC: This is from do_gen_write() and indicates that it cannot 
     * call the stat() system call on the file required.  The error string at
     * the end of the line should explain what the problem is. */
    return;
  }
  if (fbuf.st_size >= CONFIG_MAX_FILESIZE) {
    send_to_char(ch, "Sorry, the file is full right now.. try again later.\r\n");
    return;
  }
  if (!(fl = fopen(filename, "a"))) {
    perror("SYSERR: do_gen_write");
    /* SYSERR_DESC: This is from do_gen_write(), and will be output if the file
     * in question cannot be opened for appending to.  The error string at the 
     * end of the line should explain what the problem is. */

    send_to_char(ch, "Could not open the file.  Sorry.\r\n");
    return;
  }
  fprintf(fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
	  GET_ROOM_VNUM(IN_ROOM(ch)), argument);
  fclose(fl);
  send_to_char(ch, "Okay.  Thanks!\r\n");
}

#define TOG_OFF 0
#define TOG_ON  1
ACMD(do_gen_tog)
{
  long result;

  const char *tog_messages[][2] = {
    {"You are now safe from summoning by other players.\r\n",
    "You may now be summoned by other players.\r\n"},
    {"Nohassle disabled.\r\n",
    "Nohassle enabled.\r\n"},
    {"Brief mode off.\r\n",
    "Brief mode on.\r\n"},
    {"Compact mode off.\r\n",
    "Compact mode on.\r\n"},
    {"You can now hear tells.\r\n",
    "You are now deaf to tells.\r\n"},
    {"You can now hear auctions.\r\n",
    "You are now deaf to auctions.\r\n"},
    {"You can now hear shouts.\r\n",
    "You are now deaf to shouts.\r\n"},
    {"You can now hear gossip.\r\n",
    "You are now deaf to gossip.\r\n"},
    {"You can now hear the congratulation messages.\r\n",
    "You are now deaf to the congratulation messages.\r\n"},
    {"You can now hear the Wiz-channel.\r\n",
    "You are now deaf to the Wiz-channel.\r\n"},
    {"You are no longer part of the Quest.\r\n",
    "Okay, you are part of the Quest!\r\n"},
    {"You will no longer see the room flags.\r\n",
    "You will now see the room flags.\r\n"},
    {"You will now have your communication repeated.\r\n",
    "You will no longer have your communication repeated.\r\n"},
    {"HolyLight mode off.\r\n",
    "HolyLight mode on.\r\n"},
    {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
    "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
    {"Autoexits disabled.\r\n",
    "Autoexits enabled.\r\n"},
    {"Will no longer track through doors.\r\n",
    "Will now track through doors.\r\n"},
    {"Will no longer clear screen in OLC.\r\n",
    "Will now clear screen in OLC.\r\n"},
    {"Buildwalk Off.\r\n",
    "Buildwalk On.\r\n"},
    {"AFK flag is now off.\r\n",
    "AFK flag is now on.\r\n"},
    {"You are no longer a bad ass mutha fucker.\r\n",
     "You are now a BAD ASS MUTHA FUCKA mutha fucka!.\r\n"}
  };

  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_NOSUMMON:
    result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
    break;
  case SCMD_NOHASSLE:
    result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
    break;
  case SCMD_BRIEF:
    result = PRF_TOG_CHK(ch, PRF_BRIEF);
    break;
  case SCMD_COMPACT:
    result = PRF_TOG_CHK(ch, PRF_COMPACT);
    break;
  case SCMD_NOTELL:
    result = PRF_TOG_CHK(ch, PRF_NOTELL);
    break;
  case SCMD_NOAUCTION:
    result = PRF_TOG_CHK(ch, PRF_NOAUCT);
    break;
  case SCMD_NOSHOUT:
    result = PRF_TOG_CHK(ch, PRF_NOSHOUT);
    break;
  case SCMD_NOGOSSIP:
    result = PRF_TOG_CHK(ch, PRF_NOGOSS);
    break;
  case SCMD_NOGRATZ:
    result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
    break;
  case SCMD_NOWIZ:
    result = PRF_TOG_CHK(ch, PRF_NOWIZ);
    break;
  case SCMD_QUEST:
    result = PRF_TOG_CHK(ch, PRF_QUEST);
    break;
  case SCMD_ROOMFLAGS:
    result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
    break;
  case SCMD_NOREPEAT:
    result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
    break;
  case SCMD_HOLYLIGHT:
    result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
    break;
  case SCMD_AUTOEXIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
    break;
  case SCMD_CLS:
    result = PRF_TOG_CHK(ch, PRF_CLS);
    break;
  case SCMD_OUTLAW:
    if(GET_PKBLOOD(ch) > 0 && PLR_FLAGGED(ch, PLR_OUTLAW)) 
       send_to_char(ch, "You can't stop being an outlaw until there is no blood on your hands.\r\n");
    else
       result = PLR_TOG_CHK(ch, PLR_OUTLAW);
    break;
  case SCMD_BUILDWALK:
    if (GET_LEVEL(ch) < LVL_BUILDER) {
      send_to_char(ch, "Builders only, sorry.\r\n");
      return;
    }
    result = PRF_TOG_CHK(ch, PRF_BUILDWALK);
    if (PRF_FLAGGED(ch, PRF_BUILDWALK))
      nmudlog(OLC_LOG, LVL_GOD, TRUE, "%s turned buildwalk on.  Zone allowed: %d", GET_NAME(ch), GET_OLC_ZONE(ch));      
      //mudlog(CMP, GET_LEVEL(ch), TRUE,
      //      "OLC: %s turned buildwalk on. Allowed zone %d", GET_NAME(ch), GET_OLC_ZONE(ch));
    else
      nmudlog(OLC_LOG, LVL_GOD, TRUE, "%s turned buildwalk off.  Allowed zone %d", GET_NAME(ch), GET_OLC_ZONE(ch));
//      mudlog(CMP, GET_LEVEL(ch), TRUE,
//             "OLC: %s turned buildwalk off. Allowed zone %d", GET_NAME(ch), GET_OLC_ZONE(ch));
    break;
  case SCMD_AFK:
    result = PRF_TOG_CHK(ch, PRF_AFK);
    if (PRF_FLAGGED(ch, PRF_AFK))
      act("$n has gone AFK.", TRUE, ch, 0, 0, TO_ROOM);
    else {
      act("$n has come back from AFK.", TRUE, ch, 0, 0, TO_ROOM);
//      if (has_mail(GET_IDNUM(ch)))
//        send_to_char(ch, "You have mail waiting.\r\n");
    }
    break;
  default:
    log("SYSERR: Unknown subcmd %d in do_gen_toggle.", subcmd);
    return;
  }

  if (result)
    send_to_char(ch, "%s", tog_messages[subcmd][TOG_ON]);
  else
    send_to_char(ch, "%s", tog_messages[subcmd][TOG_OFF]);

  return;
}

ACMD(do_choose_death)
{
   if(!PLR_FLAGGED(ch, PLR_DYING)){
     send_to_char(ch, "You must be dying to do that!");
     return;
   }

   GET_DYING_TIME(ch) = 1;
}

ACMD (do_meditate)
{

    if (GET_POS(ch) == POS_RESTING && rand_number(1, 10) > 3 && GET_INT(ch) > 13)
    {
      send_to_char(ch, "&WYou meditate peacefully&n\r\n");
      if (IS_CLERIC(ch) || IS_MAGIC_USER(ch) || IS_MYSTIC(ch) || IS_PRIEST(ch) || IS_SHAMAN(ch) || IS_NECROMANCER(ch) || IS_ELEMENTALIST(ch) || IS_SORCERER(ch))
      {
       GET_MOVE(ch) += rand_number(20, 40);
       GET_MANA(ch) += rand_number(20, 40);
       if (GET_MANA(ch) > GET_MAX_MANA(ch))
          GET_MANA(ch) = GET_MAX_MANA(ch);
      }
      else
      {
       if(IS_RANGER(ch))
       {
         GET_MOVE(ch) += rand_number(20, 40);
         GET_HIT(ch) += rand_number(20, 40);
         GET_MANA(ch) += rand_number(20,40);
       }
       else
       {
         GET_MOVE(ch) += rand_number(20, 40);
         GET_HIT(ch) += rand_number(20, 40);
       }
       if (GET_HIT(ch) > GET_MAX_HIT(ch))
          GET_HIT(ch) = GET_MAX_HIT(ch);
      }
      act("$n meditates peacefully.", FALSE, ch, 0, 0, TO_ROOM);
      WAIT_STATE(ch, (PULSE_VIOLENCE*2));
    }
    else
    {
      send_to_char(ch, "&RYou fail at your attempt to meditate.&n");
      WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}

ACMD(do_attack_castle)
{
  struct descriptor_data *pt;


  if (GET_ROOM_VNUM(IN_ROOM(ch)) == 20000)
   {
    for (pt = descriptor_list; pt; pt = pt->next)
    {
      if (IS_PLAYING(pt) && (PLR_FLAGGED(pt->character, PLR_TAKE_SMOKE) || PLR_FLAGGED(pt->character, PLR_TAKE_BLAZE)))
      {
        send_to_char(ch, "&RThis castle or its sister is currently being attacked.&n");
        return;
      }
    }

    for (pt = descriptor_list; pt; pt = pt->next)
     if (IS_PLAYING(pt) && (GET_ROOM_VNUM(IN_ROOM(pt->character)) == 20000))
     {
      SET_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_SMOKE);
      send_to_room(IN_ROOM(pt->character), "&WYou are storming Castle Smoke.&n\r\n");
     }
   }

   if (GET_ROOM_VNUM(IN_ROOM(ch)) == 20050)
   {
    for (pt = descriptor_list; pt; pt = pt->next)
    {
      if (IS_PLAYING(pt) && (PLR_FLAGGED(pt->character, PLR_TAKE_SMOKE) || PLR_FLAGGED(pt->character, PLR_TAKE_BLAZE)))
      {
        send_to_char(ch, "&RThis castle or its sister is currently being attacked.&n");
        return;
      }
    }
    for (pt = descriptor_list; pt; pt = pt->next)
     if (IS_PLAYING(pt) && (GET_ROOM_VNUM(IN_ROOM(pt->character)) == 20050))
     {
      SET_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_BLAZE);
      send_to_room(IN_ROOM(pt->character), "&WYou are storming Castle Blaze.&n\r\n");
     }
   }

   if (GET_ROOM_VNUM(IN_ROOM(ch)) == 19900)
   {
    for (pt = descriptor_list; pt; pt = pt->next)
    {
      if (IS_PLAYING(pt) && (PLR_FLAGGED(pt->character, PLR_TAKE_CANN) || PLR_FLAGGED(pt->character, PLR_TAKE_SATI)))
      {
        send_to_char(ch, "&RThis castle or its sister is currently being attacked.&n");
        return;
      }
    }

    for (pt = descriptor_list; pt; pt = pt->next)
     if (IS_PLAYING(pt) && (GET_ROOM_VNUM(IN_ROOM(pt->character)) == 19900))
     {
      SET_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_CANN);
      send_to_room(IN_ROOM(pt->character), "&WYou are storming Castle Cannibis.&n\r\n");
     }
   }

   if (GET_ROOM_VNUM(IN_ROOM(ch)) == 19950)
   {
    for (pt = descriptor_list; pt; pt = pt->next)
    {
      if (IS_PLAYING(pt) && (PLR_FLAGGED(pt->character, PLR_TAKE_CANN) || PLR_FLAGGED(pt->character, PLR_TAKE_SATI)))
      {
        send_to_char(ch, "&RThis castle or its sister is currently being attacked.&n");
        return;
      }
    }

    for (pt = descriptor_list; pt; pt = pt->next)
     if (IS_PLAYING(pt) && (GET_ROOM_VNUM(IN_ROOM(pt->character)) == 19950))
     {
      SET_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_SATI);
      send_to_room(IN_ROOM(pt->character), "&WYou are storming Castle Sativa.&n\r\n");
     }
   }
}

ACMD(do_castout)
{
  struct obj_data *pole;
  int fail;

  if (PLR_FLAGGED(ch, PLR_FISHING)) {
    send_to_char(ch, "&RYou are already fishing!&n\r\n");
    return;
  }
  if (!(pole = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(pole) != ITEM_POLE)) {
    send_to_char(ch, "&RYou need to be holding a fishing pole first.&n\r\n");
    return;
  }
  if (!ROOM_FLAGGED(ch->in_room, ROOM_SALTWATER_FISH) &&  !ROOM_FLAGGED(ch->in_room, ROOM_FRESHWATER_FISH) && !ROOM_FLAGGED(ch->in_room, ROOM_TREASUREWATER_FISH)) {
    send_to_char(ch, "&RThis is not a good place to fish, you'll want to find a better spot.&n\r\n");
    return;
  }
  fail = rand_number(1, 10);
  if (fail <= 3) {
    send_to_char(ch, "&rYou pull your arm back and try to cast out your line, but it gets all tangled up.&n\r\n&RTry again.&n\r\n");
    act("&G$n pulls $s arm back, trying to cast $s fishing line out into the water, but ends up just a bit tangled.&n\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  /* Ok, now they've gone through the checks, now set them fishing */
   SET_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
   send_to_char(ch, "&GYou cast your line out into the water, hoping for a bite.&n\r\n");
   act("&G$n casts $s line out into the water, hoping to catch some food.&n\r\n", FALSE, ch, 0, 0, TO_ROOM);
 return;
}

ACMD(do_reelin)
{
  int success, t; 
  obj_vnum f_num, fish_num;
  struct obj_data *fish;
  char buf[MAX_INPUT_LENGTH];

  if (!PLR_FLAGGED(ch, PLR_FISHING)) {
    send_to_char(ch, "&RYou aren't even fishing!&n\r\n");
    return;
  }
  if (!PLR_FLAGGED(ch, PLR_FISH_ON)) {
    send_to_char(ch, "&rYou reel in your line, but alas... nothing on the end.&n\r\n&RBetter luck next time.&n\r\n");
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
    act("&G$n reels $s line in, but with nothing on the end.&n\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  /* Ok, they are fishing and have a fish on  */
  success = rand_number(1, 10);
  
  if(GET_LEVEL(ch) > 40)
    success=10;
  
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);

  if (success <= 7)
  {
  send_to_char(ch, "&rYou reel in your line, putting up a good fight, but you lose him!&n\r\n&RTry again?&n\r\n");
    act("&G$n reels $s line in, fighting with whatever is on the end, but loses the catch.&n\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_SALTWATER_FISH)) {
    fish_num = rand_number(5500, 5509);
    f_num = real_object(fish_num);
    fish = read_object(f_num, REAL);
    sprintf(buf, "&CYou reel in %s! Nice catch!&n\r\n", fish->short_description);
    act("&GWow! $n reels in a helluva catch! Looks like $p!&n\r\n", FALSE, ch, fish, 0, TO_ROOM);
    send_to_char(ch, "%s", buf);
    if (GET_PROF(ch, PROF_FISHING) > 100) {
      SET_SKILL(ch, PROF_MINING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(10,25), 5000));
      SET_BIT_AR(GET_OBJ_EXTRA(fish), ITEM_SCALEABLE);
      SET_BIT_AR(GET_OBJ_EXTRA(fish), ITEM_COOKABLE);
    }
    obj_to_char(fish, ch);
    return;
  } else if (ROOM_FLAGGED(ch->in_room, ROOM_FRESHWATER_FISH)) {
    fish_num = rand_number(5510, 5519);
    f_num = real_object(fish_num);
    fish = read_object(f_num, REAL);
    sprintf(buf, "&CYou reel in %s! Nice catch!&n\r\n", fish->short_description);
    send_to_char(ch, "%s", buf);
    if (GET_PROF(ch, PROF_FISHING) > 100) {
      SET_SKILL(ch, PROF_MINING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(10,25), 5000));
      SET_BIT_AR(GET_OBJ_EXTRA(fish), ITEM_SCALEABLE);
      SET_BIT_AR(GET_OBJ_EXTRA(fish), ITEM_COOKABLE);
    }
    obj_to_char(fish, ch);
    return;
  } else if (ROOM_FLAGGED(ch->in_room, ROOM_TREASUREWATER_FISH)) {
    t = rand_number(1, 100);
    if (t <= 10)
       fish_num = 5521;
    else if (t > 10 && t <= 20)
       fish_num = 5522;
    else if (t > 20 && t <= 30)
       fish_num = 5523;
    else if (t > 30 && t <= 40)
       fish_num = 5524;
    else if (t > 40 && t <= 60)
       fish_num = 5525;
    else if (t > 60 && t <= 70)
       fish_num = 5526;
    else if (t > 70 && t <= 80)
       fish_num = 5527;
    else if (t > 80 && t <= 90)
       fish_num = 5528;
    else if (t > 90 && t <= 95)
       fish_num = 5529;
    else if (t > 95)
       fish_num = 5530;
    else
       fish_num = 5520;

    f_num = real_object(fish_num);
    fish = read_object(f_num, REAL);
    snprintf(buf, sizeof(buf), "&CYou reel in %s! Might wanna go pawn this off somewhere!&n\r\n", fish->short_description);
    send_to_char(ch, "%s", buf);
    if (GET_PROF(ch, PROF_FISHING) > 100) {
      SET_SKILL(ch, PROF_MINING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(10,25), 5000));
      GET_OBJ_COST(fish) = 1000000;
    }
    obj_to_char(fish, ch);
    return;
  } else
  send_to_char(ch, "You should never see this message, please report it.\r\n");
  return;
}


ACMD(do_wimpy)
{
  char arg[MAX_INPUT_LENGTH];
  int wimp_lev;

  /* 'wimp_level' is a player_special. -gg 2/25/98 */
  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    if (GET_WIMP_LEV(ch)) {
      send_to_char(ch, "Your current wimp level is %d hit points.\r\n", GET_WIMP_LEV(ch));
      return;
    } else {
      send_to_char(ch, "At the moment, you're not a wimp.  (sure, sure...)\r\n");
      return;
    }
  }
  if (isdigit(*arg)) {
    if ((wimp_lev = atoi(arg)) != 0) {
      if (wimp_lev < 0)
        send_to_char(ch, "Heh, heh, heh.. we are jolly funny today, eh?\r\n");
      else if (wimp_lev > GET_MAX_HIT(ch))
        send_to_char(ch, "That doesn't make much sense, now does it?\r\n");
      else if (wimp_lev > (GET_MAX_HIT(ch) / 2))
        send_to_char(ch, "You can't set your wimp level above half your hit points.\r\n");
      else {
        send_to_char(ch, "Okay, you'll wimp out if you drop below %d hit points.\r\n", wimp_lev);
        GET_WIMP_LEV(ch) = wimp_lev;
      }
    } else {
      send_to_char(ch, "Okay, you'll now tough out fights to the bitter end.\r\n");
      GET_WIMP_LEV(ch) = 0;
    }
  } else
    send_to_char(ch, "Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n");
}

/* return TRUE if victim is known by ch, FALSE otherwise */
bool is_char_known( CHAR_DATA *ch, CHAR_DATA *victim )
{
        KNOWN_DATA *pKnow;

        if ( !ch || !victim )
        {
                log("SYSERR: NULL pointer passed to is_char_known()");
                return (FALSE);
        }

        if ( IS_NPC(ch) || IS_NPC(victim) ||
             IS_IMMORTAL(ch) || IS_IMMORTAL(victim) ||
             ch == victim )
                return (TRUE);

        /* he knows no one */
        if (!ch->player_specials->first_known)
                return (FALSE);

        for ( pKnow = ch->player_specials->first_known; pKnow; pKnow = pKnow->next )
        {
                if ( pKnow->idnum == GET_IDNUM(victim) )
                        break;
        }

        if ( pKnow )
                return (TRUE);

        return (FALSE);
}

char *take_name(CHAR_DATA *ch, CHAR_DATA *viewer)
{
        char baf[MAX_STRING_LENGTH];

        if (IS_NPC(ch) || IS_NPC(viewer) ||
            IS_IMMORTAL(ch) || IS_IMMORTAL(viewer) ||
            is_char_known(viewer, ch))
                return (GET_NAME(ch));

//        sprintf(baf, "%s", pc_class_types[(int)GET_CLASS(ch)]);
        return (str_dup(baf));
}

char *take_name_keyword(CHAR_DATA *ch, CHAR_DATA *viewer)
{
        if (IS_NPC(ch))
                return (GET_PC_NAME(ch));
        else
                return (take_name(ch, viewer));
}

ACMD(do_newb)
{
  int classes=0;
  struct descriptor_data *d;
  struct clan_type *cptr = NULL;
  struct clan_type *clan_info = NULL; 

  for (cptr = clan_info; cptr && cptr->number != 100; cptr = cptr->next);

  classes = GET_WAS_WARRIOR(ch) + GET_WAS_CLERIC(ch) + GET_WAS_THIEF(ch) + GET_WAS_MAGE(ch) + GET_WAS_PALADIN(ch)+
               GET_WAS_KNIGHT(ch)+ GET_WAS_RANGER(ch)+ GET_WAS_MYSTIC(ch)+ GET_WAS_PRIEST(ch)+ GET_WAS_SHAMAN(ch)+
               GET_WAS_NECROMANCER(ch)+ GET_WAS_ELEMENTALIST(ch)+ GET_WAS_SORCERER(ch)+ GET_WAS_ROGUE(ch)+
               GET_WAS_NINJA(ch)+ GET_WAS_ASSASSIN(ch);

  if (classes == 1)
  {
      if (GET_CLAN(ch) == 100)
      {
	perform_cinfo(GET_CLAN(ch), "%s has resigned from the guild.", GET_NAME(ch));
	do_dismiss( ch, cptr, ch );
      }
      else
      {
	GET_CLAN(ch) = 100;
	perform_cinfo(GET_CLAN(ch), "%s is now a member of the guild", GET_NAME(ch));
      }
  }
  else
    send_to_char(ch, "&RReally, after more than one class you still think you're a newb?&n\r\n");

}
