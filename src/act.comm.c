/**************************************************************************
*  File: act.comm.c                                        Part of tbaMUD *
*  Usage: Player-level communication commands.                            *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "improved-edit.h"
#include "dg_scripts.h"
#include "logger.h"

/* local functions */
void perform_tell(struct char_data *ch, struct char_data *vict, char *arg);
int is_tell_ok(struct char_data *ch, struct char_data *vict);
ACMD(do_say);
ACMD(do_gsay);
ACMD(do_tell);
ACMD(do_talk);
ACMD(do_reply);
ACMD(do_spec_comm);
ACMD(do_write);
ACMD(do_page);
ACMD(do_gen_comm);
ACMD(do_qcomm);
void handle_webster_file(void);

static long last_webster_teller = -1L;

void LogChannel(const char *chan, const char *txt, ...);
char  *strip_colors(const char *arg);

ACMD(do_gossip);

ACMD(do_gossip)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

    for (pt = descriptor_list; pt; pt = pt->next)
      if (IS_PLAYING(pt) && pt->character && pt->character != ch)
        send_to_char(pt->character, "&y%s gossips, '%s'&n\r\n", GET_NAME(ch), argument);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "&yYou gossip, '%s'&n", CONFIG_OK);
    else
      send_to_char(ch, "&yYou gossip, '%s'\r\n&n", argument);
      LogChannel(GOS_LOG, "%s: %s", GET_NAME(ch), argument);

}

ACMD(do_say)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char(ch, "Yes, but WHAT do you want to say?\r\n");
  else {
    char buf[MAX_INPUT_LENGTH + 14], *msg;
    struct char_data *vict;

    snprintf(buf, sizeof(buf), "&C$n says, '%s'&n", argument);
    msg = act(buf, FALSE, ch, 0, 0, TO_ROOM | DG_NO_TRIG);

    for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room)
      if (vict != ch && GET_POS(vict) > POS_SLEEPING)
        add_history(vict, msg, HIST_SAY);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else {
      sprintf(buf, "&CYou say, '%s'&n", argument);
      msg = act(buf, FALSE, ch, 0, 0, TO_CHAR | DG_NO_TRIG);
      add_history(ch, msg, HIST_SAY);
      if (!IS_NPC(ch))
      LogChannel(SAY_LOG, "%s: %s", GET_NAME(ch), argument);
    }
  }
  /* trigger check */
  speech_mtrigger(ch, argument);
  speech_wtrigger(ch, argument);
}

ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;

  skip_spaces(&argument);

  if (!AFF_FLAGGED(ch, AFF_GROUP)) {
    send_to_char(ch, "But you are not the member of a group!\r\n");
    return;
  }
  if (!*argument)
    send_to_char(ch, "Yes, but WHAT do you want to group-say?\r\n");
  else {

    if (ch->master)
      k = ch->master;
    else
      k = ch;

    snprintf(buf, sizeof(buf), "&G$n tells the group, '%s'&n", argument);

    if (AFF_FLAGGED(k, AFF_GROUP) && (k != ch))
      act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
    for (f = k->followers; f; f = f->next)
      if (AFF_FLAGGED(f->follower, AFF_GROUP) && (f->follower != ch))
        act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);
    
    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else
      send_to_char(ch, "&GYou tell the group, '%s'&n\r\n", argument);
  }
}

void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  char *msg; 

  snprintf(buf, sizeof(buf), "%s$n tells you, '%s'%s", CBRED(vict, C_NRM), arg, CCNRM(vict, C_NRM));
  msg = act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
  add_history(vict, msg, HIST_TELL);

  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(ch, "%s", CONFIG_OK);
  else {
    snprintf(buf, sizeof(buf), "%sYou tell $N, '%s'%s", CBRED(ch, C_NRM), arg, CCNRM(ch, C_NRM));
    msg = act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);     
    add_history(ch, msg, HIST_TELL);
    LogChannel(TELL_LOG, "%s to %s: %s", GET_NAME(ch), GET_NAME(vict), arg);
  }

/*  if (PRF_FLAGGED(vict, PRF_AFK))
  {
    if ( (GET_AFKMSG(vict)) && (*(GET_AFKMSG(vict))) )
      sprintf(buf, "%s%s %s%s", CBGRN(ch, C_NRM), GET_NAME(vict), GET_AFKMSG(vict), CCNRM(ch, C_NRM) );
    else
      sprintf(buf, "%s%s is afk right now, so %s may not respond right away.%s", CBGRN(ch, C_NRM), GET_NAME(vict), 
              GET_SEX(vict) == SEX_NEUTRAL ? "it" : (GET_SEX(vict) == SEX_MALE ? "he" : "she"), CCNRM(ch, C_NRM));
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
*/


  if (!IS_NPC(vict) && !IS_NPC(ch))
    GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

int is_tell_ok(struct char_data *ch, struct char_data *vict)
{
  if (ch == vict)
    send_to_char(ch, "You try to tell yourself something.\r\n");
  else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char(ch, "You can't tell other people while you have notell on.\r\n");
  else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SOUNDPROOF) && (GET_LEVEL(ch) < LVL_GOD))
    send_to_char(ch, "The walls seem to absorb your words.\r\n");
  else if (!IS_NPC(vict) && !vict->desc)        /* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if ((!IS_NPC(vict) && PRF_FLAGGED(vict, PRF_NOTELL)) || (ROOM_FLAGGED(IN_ROOM(vict), ROOM_SOUNDPROOF) && (GET_LEVEL(ch) < LVL_GOD)))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else
    return (TRUE);

  return (FALSE);
}

/* Yes, do_tell probably could be combined with whisper and ask, but it is
 * called frequently, and should IMHO be kept as tight as possible. */
ACMD(do_tell)
{
  struct char_data *vict = NULL;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char(ch, "Who do you wish to tell what??\r\n");
  else if (!strcmp(buf, "m-w")) {
    char word[MAX_INPUT_LENGTH], *p, *q;

    if (last_webster_teller != -1L) {
      if (GET_IDNUM(ch) == last_webster_teller) {
        send_to_char(ch, "You are still waiting for a response.\r\n");
        return;
      } else {
        send_to_char(ch, "Hold on, m-w is busy. Try again in a couple of seconds.\r\n");
        return;
      }
    }
    /* only a-z and +/- allowed */
    for (p = buf2, q = word; *p ; p++) {
      if ((LOWER(*p) <= 'z' && LOWER(*p) >= 'a') || (*p == '+') || (*p == '-'))
        *q++ = *p;
    }
    *q = '\0';

    if (!*word) {
      send_to_char(ch, "Sorry, only letters and +/- are allowed characters.\r\n");
      return;
    }
    snprintf(buf, sizeof(buf), "../bin/webster %s %d &", word, (int) getpid());
    system(buf);
    last_webster_teller = GET_IDNUM(ch);
    send_to_char(ch, "You look up '%s' in Merriam-Webster.\r\n", word);

  } else if (GET_LEVEL(ch) < LVL_IMMORT && !(vict = get_player_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (GET_LEVEL(ch) >= LVL_IMMORT && !(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (is_tell_ok(ch, vict))
    perform_tell(ch, vict, buf2);
   
}

ACMD(do_talk)
{
  struct char_data *vict = NULL;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char(ch, "Who do you wish to talk to??\r\n");
  else if (!strcmp(buf, "m-w")) {
    char word[MAX_INPUT_LENGTH], *p, *q;

    /* only a-z and +/- allowed */
    for (p = buf2, q = word; *p ; p++) {
      if ((LOWER(*p) <= 'z' && LOWER(*p) >= 'a') || (*p == '+') || (*p == '-'))
        *q++ = *p;
    }
    *q = '\0';

    if (!*word) {
      send_to_char(ch, "Sorry, only letters and +/- are allowed characters.\r\n");
      return;
    }
    snprintf(buf, sizeof(buf), "../bin/webster %s %d &", word, (int) getpid());
    system(buf);
    last_webster_teller = GET_IDNUM(ch);
    send_to_char(ch, "You look up '%s' in Merriam-Webster.\r\n", word);

  } else if (GET_LEVEL(ch) < LVL_IMMORT && !(vict = get_player_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (GET_LEVEL(ch) >= LVL_IMMORT && !(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (is_tell_ok(ch, vict))
  {
    snprintf(buf3, sizeof(buf), "&CYou say to %s, '%s", GET_NAME(vict), buf2);
    act(buf3, FALSE, ch, 0, vict, TO_CHAR);
    snprintf(buf3, sizeof(buf), "&C%s says to you, '%s", GET_NAME(ch), buf2);
    act(buf3, FALSE, ch, 0, vict, TO_VICT);
    snprintf(buf3, sizeof(buf), "&C%s says to %s, '%s", GET_NAME(ch), GET_NAME(vict), buf2);
    act(buf3, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}

ACMD(do_reply)
{
  struct char_data *tch = character_list;

  if (IS_NPC(ch))
    return;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char(ch, "You have nobody to reply to!\r\n");
  else if (!*argument)
    send_to_char(ch, "What is your reply?\r\n");
  else {
    /* Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again. A descriptor list based 
     * search would be faster although we could not find link dead people.  
     * Not that they can hear tells anyway. :) -gg 2/24/98 */
    while (tch != NULL && (IS_NPC(tch) || GET_IDNUM(tch) != GET_LAST_TELL(ch)))
      tch = tch->next;

    if (tch == NULL)
      send_to_char(ch, "They are no longer playing.\r\n");
    else if (is_tell_ok(ch, tch))
      perform_tell(ch, tch, argument);
  }
}

ACMD(do_spec_comm)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct char_data *vict;
  const char *action_sing, *action_plur, *action_others;
  char *col;


  switch (subcmd) {
  case SCMD_WHISPER:
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
    col = BBLU;
    break;

  case SCMD_ASK:
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
    col = BBLU;
    break;

  default:
    action_sing = "oops";
    action_plur = "oopses";
    action_others = "$n is tongue-tied trying to speak with $N.";
    break;
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char(ch, "Whom do you want to %s.. and what??\r\n", action_sing);
  else if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (vict == ch)
    send_to_char(ch, "You can't get your mouth close enough to your ear...\r\n");
  else {

    snprintf(buf1, sizeof(buf1), "%s$n %s you, '%s'", col, action_plur, buf2);
    act(buf1, FALSE, ch, 0, vict, TO_VICT);

    if ((!IS_NPC(ch)) && (PRF_FLAGGED(ch, PRF_NOREPEAT))) 
      send_to_char(ch, "%s", CONFIG_OK);
    else
      send_to_char(ch, "%sYou %s %s, '%s'\r\n", col, action_sing, GET_NAME(vict), buf2);
    act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}

/* buf1, buf2 = MAX_OBJECT_NAME_LENGTH (if it existed) */
ACMD(do_write)
{
  struct obj_data *paper, *pen = NULL;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {		/* nothing was delivered */
    send_to_char(ch, "Write?  With what?  ON what?  What are you trying to do?!?\r\n");
    return;
  }
  if (*penname) {		/* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, NULL, ch->carrying))) {
      send_to_char(ch, "You have no %s.\r\n", papername);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, NULL, ch->carrying))) {
      send_to_char(ch, "You have no %s.\r\n", penname);
      return;
    }
  } else {		/* there was one arg.. let's see what we can find */
    if (!(paper = get_obj_in_list_vis(ch, papername, NULL, ch->carrying))) {
      send_to_char(ch, "There is no %s in your inventory.\r\n", papername);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {	/* oops, a pen.. */
      pen = paper;
      paper = NULL;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char(ch, "That thing has nothing to do with writing.\r\n");
      return;
    }
    /* One object was found.. now for the other one. */
    if (!GET_EQ(ch, WEAR_HOLD)) {
      send_to_char(ch, "You can't write with %s %s alone.\r\n", AN(papername), papername);
      return;
    }
    if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
      send_to_char(ch, "The stuff in your hand is invisible!  Yeech!!\r\n");
      return;
    }
    if (pen)
      paper = GET_EQ(ch, WEAR_HOLD);
    else
      pen = GET_EQ(ch, WEAR_HOLD);
  }

  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN)
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  else {
    char *backstr = NULL;

    /* Something on it, display it as that's in input buffer. */
    if (paper->action_description) {
      backstr = strdup(paper->action_description);
      send_to_char(ch, "There's something written on it already:\r\n");
      send_to_char(ch, "%s", paper->action_description);
    }

    /* we can write - hooray! */
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
    send_editor_help(ch->desc);
    string_write(ch->desc, &paper->action_description, MAX_NOTE_LENGTH, 0, backstr);
  }
}

ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *vict;
  char arg[MAX_INPUT_LENGTH];

  half_chop(argument, arg, buf2);

  if (IS_NPC(ch))
    send_to_char(ch, "Monsters can't page.. go away.\r\n");
  else if (!*arg)
    send_to_char(ch, "Whom do you wish to page?\r\n");
  else {

    snprintf(buf, sizeof(buf), "\007\007*$n* %s", buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_GOD) {
	for (d = descriptor_list; d; d = d->next)
	  if (STATE(d) == CON_PLAYING && d->character)
	    act(buf, FALSE, ch, 0, d->character, TO_VICT);
      } else
	send_to_char(ch, "You will never be godly enough to do that!\r\n");
      return;
    }
    if ((vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)) != NULL) {
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
	send_to_char(ch, "%s", CONFIG_OK);
      else
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
    } else
      send_to_char(ch, "There is no such person in the game!\r\n");
  }
}

int first_word_is_name(struct char_data *ch, char * argument) {
    char buf[MAX_INPUT_LENGTH];
    char name_p[MAX_INPUT_LENGTH];
    if (argument == NULL || IS_NPC(ch))
        return 0;
    strlcpy(buf, argument, sizeof(buf));
    one_argument(buf, name_p);
    return !str_cmp(GET_NAME(ch), name_p);
}

/* generalized communication func, originally by Fred C. Merkel (Torg) */
ACMD(do_gen_comm)
{
  struct descriptor_data *i;
  char color_on[24];
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], *msg;
  bool emoting = FALSE;

  /* Array of flags which must _not_ be set in order for comm to be heard */
  int channels[] = {
    0,
    PRF_NOSHOUT,
    PRF_NOGOSS,
    PRF_NOAUCT,
    PRF_NOGRATZ,
    PRF_NOGOSS,
    0
  };



  int hist_type[] = {                    
    HIST_ALL,
    HIST_SAY,
    HIST_GOSSIP,
    HIST_WIZNET,
    HIST_TELL,
    HIST_SHOUT,
    HIST_GRATS,
    HIST_HOLLER,
    HIST_AUCTION,
    HIST_IMPL,
    HIST_GRGOD,
    HIST_GOD,
    HIST_PREC,
  };

/*
  int hist_type[] = {
    HIST_HOLLER,
    HIST_SHOUT,
    HIST_GOSSIP,
    HIST_AUCTION,
    HIST_GRATS,
    HIST_HOLLER,
    HIST_AUCTION,
    HIST_IMPL,
    HIST_GRGOD,
    HIST_GOD,
    HIST_PREC,
    HIST_WIZNET,
  };
*/
  /* com_msgs: [0] Message if you can't perform the action because of noshout
   *           [1] name of the action
   *           [2] message if you're not on the channel
   *           [3] a color string. */
  const char *com_msgs[][4] = {
    {"You cannot holler!!\r\n",
      "holler",
      "",
    KYEL},

    {"You cannot shout!!\r\n",
      "shout",
      "Turn off your noshout flag first!\r\n",
    KYEL},

    {"You cannot gossip!!\r\n",
      "gossip",
      "You aren't even on the channel!\r\n",
    KYEL},

    {"You cannot auction!!\r\n",
      "auction",
      "You aren't even on the channel!\r\n",
    KMAG},

    {"You cannot congratulate!\r\n",
      "congrat",
      "You aren't even on the channel!\r\n",
    KGRN},

    {"You cannot gossip your emotions!\r\n",
      "gossip",
      "You aren't even on the channel!\r\n",
    KYEL}
  };

  /* to keep pets, etc from being ordered to shout */
  if (!ch->desc)
    return;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char(ch, "%s", com_msgs[subcmd][0]);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SOUNDPROOF) && (GET_LEVEL(ch) < LVL_GOD)) {
    send_to_char(ch, "The walls seem to absorb your words.\r\n");
    return;
  }

  if (subcmd == SCMD_GOSSIP && *argument == '*') {
    subcmd = SCMD_GEMOTE;
  }

  if (subcmd == SCMD_GEMOTE) {
    ACMD(do_gmote);
    if (*argument == '*')
      do_gmote(ch, argument + 1, 0, 1);
    else
      do_gmote(ch, argument, 0, 1);

    return;
  }

  /* level_can_shout defined in config.c */
  if (GET_LEVEL(ch) < CONFIG_LEVEL_CAN_SHOUT) {
    send_to_char(ch, "You must be at least level %d before you can %s.\r\n", CONFIG_LEVEL_CAN_SHOUT, com_msgs[subcmd][1]);
    return;
  }
  /* make sure the char is on the channel */
  if (PRF_FLAGGED(ch, channels[subcmd])) {
    send_to_char(ch, "%s", com_msgs[subcmd][2]);
    return;
  }
  /* skip leading spaces */
  skip_spaces(&argument);

  /* make sure that there is something there to say! */
  if (!*argument) {
    send_to_char(ch, "Yes, %s, fine, %s we must, but WHAT???\r\n", com_msgs[subcmd][1], com_msgs[subcmd][1]);
    return;
  }
  if (subcmd == SCMD_HOLLER) {
    if (GET_MOVE(ch) < CONFIG_HOLLER_MOVE_COST) {
      send_to_char(ch, "You're too exhausted to holler.\r\n");
      return;
    } else
      GET_MOVE(ch) -= CONFIG_HOLLER_MOVE_COST;
  }
  /* set up the color on code */
  strlcpy(color_on, com_msgs[subcmd][3], sizeof(color_on));

  /* first, set up strings to be given to the communicator */
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(ch, "%s", CONFIG_OK);
  else {
    if (!first_word_is_name(ch, argument)) {
      snprintf(buf1, sizeof(buf1), "%sYou %s, '%s%s'%s", COLOR_LEV(ch) >= C_CMP ? color_on : "", com_msgs[subcmd][1], argument, COLOR_LEV(ch) >= C_CMP
      ? color_on : "", CCNRM(ch, C_CMP));
    } else {
      emoting = TRUE;
      snprintf(buf1, sizeof(buf1), "%s%s: %s%s", COLOR_LEV(ch) >= C_CMP ? color_on : "", com_msgs[subcmd][1], CAP(argument), CCNRM(ch, C_CMP));
    }
    msg = act(buf1, FALSE, ch, 0, 0, TO_CHAR);
    add_history(ch, msg, hist_type[subcmd]);
    LogChannel(GENCOM_LOG, "%s: %s", GET_NAME(ch), argument);
  }
  if (!emoting)
    snprintf(buf1, sizeof(buf1), "$n %ss, '%s'", com_msgs[subcmd][1], argument);

  /* now send all the strings out */
  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING || i == ch->desc || !i->character )
      continue;
    if (!IS_NPC(ch) && (PRF_FLAGGED(i->character, channels[subcmd]) || PLR_FLAGGED(i->character, PLR_WRITING)))
      continue;

    if (ROOM_FLAGGED(IN_ROOM(i->character), ROOM_SOUNDPROOF) && (GET_LEVEL(ch) < LVL_GOD))
      continue;

    if (subcmd == SCMD_SHOUT &&
       ((world[IN_ROOM(ch)].zone != world[IN_ROOM(i->character)].zone) ||
       !AWAKE(i->character)))
      continue;

    snprintf(buf2, sizeof(buf2), "&c%s&n", buf1); 
    msg = act(buf2, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
    add_history(i->character, msg, hist_type[subcmd]);
    LogChannel(GENCOM_LOG, "%s: %s", GET_NAME(ch), argument);
  }
}

ACMD(do_qcomm)
{
  if (!PRF_FLAGGED(ch, PRF_QUEST)) {
    send_to_char(ch, "You aren't even part of the quest!\r\n");
    return;
  }
  skip_spaces(&argument);

  if (!*argument)
    send_to_char(ch, "%c%s?  Yes, fine, %s we must, but WHAT??\r\n", UPPER(*CMD_NAME), CMD_NAME + 1, CMD_NAME);
  else {
    struct descriptor_data *i;

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else if (subcmd == SCMD_QSAY) {
      snprintf(buf, sizeof(buf), "You quest-say, '%s'", argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    } else
      act(argument, FALSE, ch, 0, argument, TO_CHAR);

    if (subcmd == SCMD_QSAY)
      snprintf(buf, sizeof(buf), "$n quest-says, '%s'", argument);
    else {
      strlcpy(buf, argument, sizeof(buf));
      nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s qechoed: %s", GET_NAME(ch), argument);      
      LogChannel(QUEST_LOG, "%s: %s", GET_NAME(ch), argument);
      //mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE, "(GC) %s qechoed: %s", GET_NAME(ch), argument);
      }
    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i != ch->desc && PRF_FLAGGED(i->character, PRF_QUEST))
       act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
  }
}

void handle_webster_file(void) {
  FILE *fl;
  struct char_data *ch = find_char(last_webster_teller);
  char info[MAX_STRING_LENGTH], line[READ_SIZE];
  size_t len = 0, nlen = 0;

  last_webster_teller = -1L;

  if (!ch) /* they quit ? */
    return;

  fl = fopen("websterinfo", "r");
  if (!fl) {
    send_to_char(ch, "It seems Merriam-Webster is offline..\r\n");
    return;
  }

  unlink("websterinfo");

  get_line(fl, line);
  while (!feof(fl)) {
    nlen = snprintf(info + len, sizeof(info) - len, "%s\r\n", line);
    if (len + nlen >= sizeof(info) || nlen < 0)
      break;
    len += nlen;
    get_line(fl, line);
  }

  if (len >= sizeof(info)) {
    const char *overflow = "\r\n**OVERFLOW**\r\n";
    strcpy(info + sizeof(info) - strlen(overflow) - 1, overflow); /* strcpy: OK */
  }
  fclose(fl);

  send_to_char(ch, "You get this feedback from Merriam-Webster:\r\n");
  page_string(ch->desc, info, 1);
}
