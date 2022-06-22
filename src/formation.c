/* Another fine Bluntmaster Production *
 *      Formation Stuff                */

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

const char *form_types[];
extern const char *class_abbrevs[];
ACMD(do_group);
ACMD(do_ungroup);
void check_form_type(struct char_data *ch);
int perform_group(struct char_data *ch, struct char_data *vict);
void print_group(struct char_data *ch);
int form_mod_dam(struct char_data *ch, struct char_data *victim, int dam);
ACMD(do_follow);


ACMD(do_group)
{
  char buf[MAX_STRING_LENGTH];
  struct char_data *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf) {
    print_group(ch);
    return;
  }

  if (ch->master) {
    act("You cannot enroll group members without being head of a group.",
        FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (!str_cmp(buf, "all")) {
    perform_group(ch, ch);
    for (found = 0, f = ch->followers; f; f = f->next)
      found += perform_group(ch, f->follower);
    if (!found)
      send_to_char(ch, "Everyone following you is already in your group.\r\n");
    return;
  }

  if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if ((vict->master != ch) && (vict != ch))
    act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!AFF_FLAGGED(vict, AFF_GROUP))
      perform_group(ch, vict);
    else {
      if (ch != vict)
        act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
      act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
      act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
      REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
    }
  }

}

ACMD(do_ungroup)
{
  char buf[MAX_INPUT_LENGTH];
  struct follow_type *f, *next_fol;
  struct char_data *tch;

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(AFF_FLAGGED(ch, AFF_GROUP))) {
      send_to_char(ch, "But you lead no group!\r\n");
      return;
    }

    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (AFF_FLAGGED(f->follower, AFF_GROUP)) {
        REMOVE_BIT_AR(AFF_FLAGS(f->follower), AFF_GROUP);
        act("$N has disbanded the group.", TRUE, f->follower, NULL, ch, TO_CHAR);
        if (!AFF_FLAGGED(f->follower, AFF_CHARM))
          stop_follower(f->follower);
      }
    }

    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    send_to_char(ch, "You disband the group.\r\n");
    return;
  }
  if (!(tch = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM))) {
    send_to_char(ch, "There is no such person!\r\n");
    return;
  }
  if (tch->master != ch) {
    send_to_char(ch, "That person is not following you!\r\n");
    return;
  }

  if (!AFF_FLAGGED(tch, AFF_GROUP)) {
    send_to_char(ch, "That person isn't in your group.\r\n");
    return;
  }

  REMOVE_BIT_AR(AFF_FLAGS(tch), AFF_GROUP);

  act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
  act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
  act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);

  if (!AFF_FLAGGED(tch, AFF_CHARM))
    stop_follower(tch);

}

ACMD(do_posse)
{
  char buf[MAX_STRING_LENGTH];
  struct char_data *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf) {
    print_group(ch);
    return;
  }

  if (ch->master) {
    act("You can not enroll group members without being head of a posse.",
        FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (!str_cmp(buf, "all")) {
    perform_group(ch, ch);
    for (found = 0, f = ch->followers; f; f = f->next)
      found += perform_group(ch, f->follower);
    if (!found)
      send_to_char(ch, "Everyone following you is already in your posse.\r\n");
    return;
  }

  if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if ((vict->master != ch) && (vict != ch))
    act("$N must follow you to enter your posse.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!AFF_FLAGGED(vict, AFF_GROUP))
      perform_group(ch, vict);
    else {
      if (ch != vict)
        act("$N is no longer a member of your posse.", FALSE, ch, 0, vict, TO_CHAR);
      act("You have been kicked out of $n's posse!", FALSE, ch, 0, vict, TO_VICT);
      act("$N has been kicked out of $n's posse!", FALSE, ch, 0, vict, TO_NOTVICT);
      REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
      GET_FORM_TYPE(vict) = FORM_SOLO;
    }
  }
}

ACMD(do_boot)
{
  char buf[MAX_INPUT_LENGTH];
  struct follow_type *f, *next_fol;
  struct char_data *tch;

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(AFF_FLAGGED(ch, AFF_GROUP)) || GET_FORM_TYPE(ch) == FORM_SOLO) {
      send_to_char(ch, "But you lead no posse!\r\n");
      return;
    }

    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (AFF_FLAGGED(f->follower, AFF_GROUP)) {
        REMOVE_BIT_AR(AFF_FLAGS(f->follower), AFF_GROUP);
        act("$N has disbanded the posse.", TRUE, f->follower, NULL, ch, TO_CHAR);
        GET_FORM_TYPE(f->follower) = FORM_SOLO;
        if (!AFF_FLAGGED(f->follower, AFF_CHARM))
          stop_follower(f->follower);
      }
    }

    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    GET_FORM_TYPE(ch) = FORM_SOLO;
    send_to_char(ch, "You disband the posse.\r\n");
    return;
  }
  if (!(tch = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM))) {
    send_to_char(ch, "There is no such person!\r\n");
    return;
  }
  if (tch->master != ch) {
    send_to_char(ch, "That person is not following you!\r\n");
    return;
  }

  if (!AFF_FLAGGED(tch, AFF_GROUP)) {
    send_to_char(ch, "That person isn't in your posse.\r\n");
    return;
  }

  REMOVE_BIT_AR(AFF_FLAGS(tch), AFF_GROUP);
  GET_FORM_TYPE(tch) = FORM_SOLO;
  act("$N is no longer a member of your posse.", FALSE, ch, 0, tch, TO_CHAR);
  act("You have been kicked out of $n's posse!", FALSE, ch, 0, tch, TO_VICT);
  act("$N has been kicked out of $n's posse!", FALSE, ch, 0, tch, TO_NOTVICT);

  if (!AFF_FLAGGED(tch, AFF_CHARM))
    stop_follower(tch);

}

ACMD(do_leader)
{
   struct char_data *newlead;
   struct follow_type *f;
   char buf[MAX_NAME_LENGTH];

   if (!*argument)
   {
     send_to_char(ch, "Who do you want to make the leader?");
     return;
   }

   one_argument(argument, buf);

   if (ch->master)
   {
     send_to_char(ch, "You have to lead the group to change leaders.\r\n");
     return;
   }
   if (!(newlead = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
   {
     send_to_char(ch, "%s", CONFIG_NOPERSON);
     return;
   }
   if ((newlead->master != ch) && (newlead != ch))
   {
     send_to_char(ch, "That person is not in your posse.\r\n");
     return;
   }

   if (ch == newlead)
   {
    send_to_char(ch, "You are already the leader of this posse.");
    return;
   }

   for (f = ch->followers; f; f = f->next)
    if (AFF_FLAGGED(f->follower, AFF_GROUP) && IN_ROOM(f->follower) == IN_ROOM(ch) && f->follower->master == ch)
      do_follow(f->follower, GET_NAME(newlead), 0, 0);

   do_follow(ch, GET_NAME(newlead), 0, 0);

}

ACMD(do_autoassist)
{
    if (!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_AUTOASSIST))
    {
       SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOASSIST);
       send_to_char(ch, "Autoassist ON");
    }
    else
    {
       REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AUTOASSIST);
       send_to_char(ch, "Autoassist OFF");
    }
}

ACMD(do_autosplit)
{
    if (!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_AUTOSPLIT))
    {
       SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOSPLIT);
       send_to_char(ch, "Autosplit ON");
    }
    else
    {
       REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_AUTOSPLIT);
       send_to_char(ch, "Autosplit OFF");
    }
}


ACMD(do_formstat) /* Lars - 02/00 */
{
  struct char_data *k;
  struct follow_type *f;
  int i, counter = 0, found = FALSE;
  char buf[MAX_INPUT_LENGTH];

  /* Are we grouped? */
  if (!AFF_FLAGGED(ch, AFF_GROUP) || GET_FORM_TYPE(ch) == FORM_SOLO) {
       GET_FORM_POS(ch) = 1;
       send_to_char(ch, "You are not in a posse.\r\n");
       return;
  }
  
    k = (ch->master ? ch->master : ch);

    send_to_char(ch, "&WFormation Type = &G%s\r\n", form_types[GET_FORM_TYPE(ch)]);

    if (ch->master)
       send_to_char(ch, "\r\nYour posse's formation looks like:\r\n\r\n");
    else
       send_to_char(ch, "\r\n%s's posse is formed as follows:\r\n\r\n", GET_NAME(k));
    /* format the output */
     for (i = 0; i < MAX_FORM_POSITIONS; i++) {  /* loop through total form positions */
      if (GET_FORM_POS(k) == i) {   /* lets check for the leader first*/
        snprintf(buf, sizeof(buf), "[%-20s] ", GET_NAME(k));  /* found, spit out the output*/
        send_to_char(ch, "%s", buf);
        if (counter >= 2) {   /* checks if this is the 3rd entry on a line*/
          send_to_char(ch, "\r\n");
          counter = 0;
        } else
          counter++;  /* it's not, increment */
      } else {
        for (f = k->followers; f; f = f->next) {  //onto the followers */
          if (GET_FORM_POS(f->follower) == i) {   //are you the one? */
           snprintf(buf, sizeof(buf),  "[%-20s] ", GET_NAME(f->follower));
            send_to_char(ch, "%s", buf);
            found = TRUE;
          }
        }
        if (!found)
          send_to_char(ch, "[                    ] "); /* empty brackets to format menu logically if no one is found */
        if (counter >= 2) {  /* more checking for rows */
          send_to_char(ch, "\r\n");
         counter = 0;
        } else
          counter++; /* increment */
      }
      found = FALSE;
   }
}


ACMD(do_reform)
{
  struct char_data *vict;
  struct follow_type *f;
  int j, i, spot_hold, lead_spot;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  two_arguments(argument, buf, buf2);

  if (!*buf)
  {
    send_to_char(ch, "Whom do you wish to reform?\r\n");
    return;
  }
  if (ch->master)
  {
    send_to_char(ch, "You have to lead the group to reform members.\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
  {
    send_to_char(ch, "%s", CONFIG_NOPERSON);
    return;
  }
  if ((vict->master != ch) && (vict != ch))
  {
    send_to_char(ch, "That person is not in your group.\r\n");
    return;
  }
  if (!AFF_FLAGGED(ch, AFF_GROUP))
  {
    send_to_char(ch, "That person is not in your group.\r\n");
    return;
  }
  if (!*buf2)
  {
    send_to_char(ch, "Reform to which position?\r\n"
                     "[ fl] [ fc] [ fr]\r\n"
                     "[fcl] [fcc] [fcr]\r\n"
                     "[ cl] [ cc] [ cr]\r\n"
                     "[bcl] [bcc] [bcr]\r\n"
                     "[ bl] [ bc] [ br]\r\n");
    return;
  }

  if (!strcmp(buf2, "fl")) {
    j = 0;
    send_to_char(ch, "%s is moved to Front Left.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "fc")) {
    j = 1;
    send_to_char(ch, "%s is moved to Front Center.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "fr")) {
    j = 2;
    send_to_char(ch, "%s is moved to Front Right.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "fcl")) {
    j = 3;
    send_to_char(ch, "%s is moved to Front Center Left.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "fcc")) {
    j = 4;
    send_to_char(ch, "%s is moved to Front Center Center.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "fcr")) {
    j = 5;
    send_to_char(ch, "%s is moved to Front Center Right.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "cl")) {
    j = 6;
    send_to_char(ch, "%s is moved to Center Left.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "cc")) {
    j = 7;
    send_to_char(ch, "%s is moved to Center Center.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "cr")) {
    j = 8;
    send_to_char(ch, "%s is moved to Center Right.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "bcl")) {
    j = 9;
    send_to_char(ch, "%s is moved to Back Center Left.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "bcc")) {
    j = 10;
    send_to_char(ch, "%s is moved to Back Center Center.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "bcr")) {
    j = 11;
    send_to_char(ch, "%s is moved to Back Center Right.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "bl")) {
    j = 12;
    send_to_char(ch, "%s is moved to Back Left.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "bc")) {
    j = 13;
    send_to_char(ch, "%s is moved to Back Center.\r\n", GET_NAME(vict));
  }
  else if (!strcmp(buf2, "br")) {
    j = 14;
    send_to_char(ch, "%s is moved to Back Right.\r\n", GET_NAME(vict));
  }
  else {
    send_to_char(ch, "Reform to which position?\r\n"
		     "[ fl] [ fc] [ fr]\r\n"
                     "[fcl] [fcc] [fcr]\r\n"
                     "[ cl] [ cc] [ cr]\r\n"
                     "[bcl] [bcc] [bcr]\r\n"
                     "[ bl] [ bc] [ br]\r\n", buf);
    return;
  }

  if (j > MAX_FORM_POSITIONS-1 || j < 0)
  {
    send_to_char(ch, "That is not a valid position.\r\n");
    return;
  }


  for (i = 0; i < MAX_FORM_POSITIONS; i++)
  {
    if (GET_FORM_POS(ch) == j && ch != vict)
    {
      spot_hold = GET_FORM_POS(vict);
      GET_FORM_POS(ch) = spot_hold;
      GET_FORM_POS(vict) = j;
      check_form_type(ch);
      return;
    }

    for (f = ch->followers; f; f = f->next)
    {
      if (GET_FORM_POS(f->follower) == j && f->follower != vict)
      {
        spot_hold = GET_FORM_POS(vict);
        GET_FORM_POS(f->follower) = spot_hold;
        GET_FORM_POS(vict) = j;
        check_form_type(ch);
        return;
      }
      else if (vict == ch && GET_FORM_POS(f->follower) == j)
      {
        spot_hold = GET_FORM_POS(vict);
        GET_FORM_POS(f->follower) = spot_hold;
        GET_FORM_POS(vict) = j;
        check_form_type(ch);
        return;
      }
      else
      {
        GET_FORM_POS(vict) = j;
        GET_FORM_TOTAL(ch, i) = 0;
        check_form_type(ch);
        return;
      }
    }
   }
}

int find_form_pos(struct char_data *ch)
{
  int i;

  for (i = 0; i < MAX_FORM_POSITIONS; i++) {
    if (GET_FORM_TOTAL(ch, i) > 0)  /* occupied? */
      continue;
    else {
      GET_FORM_TOTAL(ch, i) = 1;  /* position is free, take it! */
      return (i);
    }
  }
  return (0);   /* if all else fails, should never get here */
}

void check_form_type(struct char_data *ch)
{
  bool pos[5][3]; 
  struct follow_type *f;
  int i, y=0, counter=0, formval=0;

  for (i = 0; i < MAX_FORM_POSITIONS; i++)
  {
     if (GET_FORM_POS(ch) == i)
     {
       formval += i+1;
     }
     else {
      for (f = ch->followers; f; f = f->next)
      {
        if (GET_FORM_POS(f->follower) == i) {
          formval += i+1;
        }
      }
     }

    if(y>=2) {
      y=0;
      if(counter>=4)
       counter=0;
      else
       counter++;
    }
    else
      y++;

  }

 /* ROW VALUES		Column Values		Form Position Values (for calculations, form positions are 0-14)
 * Row 1 = 6		Col 1 = 35		[1]  [2]  [3]
 * Row 2 = 15		Col 2 = 40		[4]  [5]  [6]
 * Row 3 = 24		Col 3 = 45		[7]  [8]  [9]
 * Row 4 = 33					[10] [11] [12]	
 * Row 5 = 42					[13] [14] [15]
 */

  if (formval == 6 || formval == 21) {
         GET_FORM_TYPE(ch) = FORM_STANDARD;
/*
      Form Position Values	Form Position Values	Form Position Values
       [X]  [X]  [X]		 [X]  [X]  [X]		 [ ]  [ ]  [ ]
       [ ]  [ ]  [ ]		 [X]  [X]  [X]		 [ ]  [ ]  [ ]	
       [ ]  [ ]  [ ]		 [ ]  [ ]  [ ]		 [ ]  [ ]  [ ]
       [ ]  [ ]  [ ]		 [ ]  [ ]  [ ]		 [ ]  [ ]  [ ]
       [ ]  [ ]  [ ]		 [ ]  [ ]  [ ]		 [ ]  [ ]  [ ]  
*/
  }
  else if (formval == 45 || formval == 78 || formval == 120) {
         GET_FORM_TYPE(ch) = FORM_PHALANX;
/*
      Form Position Values    Form Position Values    Form Position Values
       [X]  [X]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [X]  [X]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [X]  [X]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [ ]  [ ]  [ ]           [X]  [X]  [X]           [X]  [X]  [X]
       [ ]  [ ]  [ ]           [ ]  [ ]  [ ]           [X]  [X]  [X]
*/
  }
  else if (formval == 24 || formval == 32 || formval == 40) {
         GET_FORM_TYPE(ch) = FORM_SPEAR;
/*
      Form Position Values    Form Position Values    Form Position Values
       [ ]  [X]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ]
       [ ]  [ ]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ]
       [ ]  [X]  [ ]           [ ]  [ ]  [ ]           [ ]  [X]  [ ]
       [ ]  [ ]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ]
       [ ]  [X]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ]
*/
  }
  else if (formval == 43 || formval == 48 || formval == 56) {
         GET_FORM_TYPE(ch) = FORM_SHIELD;
/*
      Form Position Values    Form Position Values    Form Position Values
       [X]  [X]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [X]  [ ]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [X]  [ ]  [X]           [X]  [ ]  [X]           [X]  [X]  [X]
       [ ]  [X]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ]
       [ ]  [ ]  [ ]           [ ]  [ ]  [ ]           [ ]  [ ]  [ ]
*/
  }
  else if (formval == 30 || formval == 62 || formval == 99) {
         GET_FORM_TYPE(ch) = FORM_DELTA;  
/*
      Form Position Values    Form Position Values    Form Position Values
       [ ]  [X]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ] 
       [X]  [X]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [X]  [ ]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [ ]  [ ]  [ ]           [X]  [ ]  [X]           [X]  [X]  [X]
       [ ]  [ ]  [ ]           [ ]  [ ]  [ ]           [X]  [ ]  [X]
*/
 }
  else if (formval == 72 || formval == 80 || formval == 88) {
         GET_FORM_TYPE(ch) = FORM_SPHERE;
/*
      Form Position Values    Form Position Values    Form Position Values
       [ ]  [X]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ]
       [X]  [ ]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [X]  [X]  [X]           [X]  [ ]  [X]           [X]  [X]  [X]
       [X]  [ ]  [X]           [X]  [X]  [X]           [X]  [X]  [X]
       [ ]  [X]  [ ]           [ ]  [X]  [ ]           [ ]  [X]  [ ]
*/
 }
  else if (formval == 36 || formval == 42) {
         GET_FORM_TYPE(ch) = FORM_STAR;
/*
      Form Position Values    Form Position Values    Form Position Values
       [ ]  [X]  [ ]           [X]  [ ]  [X]           [ ]  [ ]  [ ]
       [X]  [ ]  [X]           [ ]  [X]  [ ]           [ ]  [ ]  [ ]
       [ ]  [X]  [ ]           [X]  [ ]  [X]           [ ]  [ ]  [ ]
       [X]  [ ]  [X]           [ ]  [X]  [ ]           [ ]  [ ]  [ ]
       [ ]  [ ]  [ ]           [ ]  [ ]  [ ]           [ ]  [ ]  [ ]
*/
  }
  else
    GET_FORM_TYPE(ch) = FORM_RANDOM;
  
  for (f = ch->followers; f; f = f->next)
    GET_FORM_TYPE(f->follower) = GET_FORM_TYPE(ch);

  return;
}

int perform_group(struct char_data *ch, struct char_data *vict)
{
  if (AFF_FLAGGED(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
    return (0);

  SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
/*  if (ch != vict)
 *      act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
 *        act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
 *          act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
 *          */  return (1);
}

void print_group(struct char_data *ch)
{
  struct char_data *k;
  struct follow_type *f;

  if (!AFF_FLAGGED(ch, AFF_GROUP))
    send_to_char(ch, "But you are not the member of a group!\r\n");
  else {
    char buf[MAX_STRING_LENGTH];

    send_to_char(ch, "Your group consists of:\r\n");

    k = (ch->master ? ch->master : ch);

    if (AFF_FLAGGED(k, AFF_GROUP)) {
      snprintf(buf, sizeof(buf), "     [%4ldH %4ldM %4ldV] [%2d %s] $N (Head of group)",
              GET_HIT(k), GET_MANA(k), GET_MOVE(k), GET_LEVEL(k), CLASS_ABBR(k));
      act(buf, FALSE, ch, 0, k, TO_CHAR);
    }

    for (f = k->followers; f; f = f->next) {
      if (!AFF_FLAGGED(f->follower, AFF_GROUP))
        continue;

      snprintf(buf, sizeof(buf), "     [%4ldH %4ldM %4ldV] [%2d %s] $N", GET_HIT(f->follower),
              GET_MANA(f->follower), GET_MOVE(f->follower),
              GET_LEVEL(f->follower), CLASS_ABBR(f->follower));
      act(buf, FALSE, ch, 0, f->follower, TO_CHAR);
    }
  }
}

int form_mod_dam(struct char_data *ch, struct char_data *victim, int dam)
{
 
    switch(GET_FORM_TYPE(ch))
    {
      case FORM_SOLO:
      case FORM_RANDOM:
      case FORM_STANDARD:
         break;

      case FORM_SPEAR:

         if (GET_FORM_POS(victim) == 7)
           dam += dam*.3;
         else if (GET_FORM_POS(victim) > 7)
           dam -= dam*.5;
         else
           dam += dam*.5;

         break;

      case FORM_PHALANX:
         if (GET_FORM_POS(ch) < 3)
           dam += dam*.2;
         break;

      case FORM_SHIELD:

         if (GET_FORM_POS(victim) < 6)
           dam += dam*.5;
         else
           dam -= dam*.3;

         break;

      case FORM_DELTA:
      case FORM_SPHERE:   
      case FORM_STAR:  
      default:
	break;
    }

    switch(GET_FORM_TYPE(victim))
    {
      case FORM_SOLO:
      case FORM_RANDOM:
      case FORM_STANDARD:
         break;

      case FORM_SPEAR:

         if (GET_FORM_POS(victim) == 7)
           dam -= dam*.3;
         else if (GET_FORM_POS(victim) > 7)
           dam -= dam*.5;
         else 
           dam += dam*.2;

         break;

      case FORM_PHALANX:
         if (GET_FORM_POS(ch) > 5)
           dam -= dam*.2;
         break;

      case FORM_SHIELD:

         if (GET_FORM_POS(victim) > 5)
           dam -= dam*.5;
         else
           dam += dam*.2;

         break;

      case FORM_DELTA:
         break;
      case FORM_SPHERE:

         if (GET_FORM_POS(victim) == 4 || GET_FORM_POS(victim) == 7 || GET_FORM_POS(victim) == 10)
           dam -= dam*.7;
       
         break;

      case FORM_STAR:
      default:
        break;
    }

 return dam;
}

/* Useful later for casting/bows/misc where you need to be in specific position, can re-write if works better another way 
   I didn't make one for every spot as I could probably do a switch and return vals based on that.. but for now this works */

bool IS_BACK_ROW(struct char_data *ch)
{
  if (GET_FORM_POS(ch) == 12 || GET_FORM_POS(ch) == 13 || GET_FORM_POS(ch) == 14)
     return (TRUE);
  else 
     return (FALSE);
}

bool IS_CENTERBACK_ROW(struct char_data *ch)
{
  if (GET_FORM_POS(ch) == 9 || GET_FORM_POS(ch) == 10 || GET_FORM_POS(ch) == 11) 
    return (TRUE);
  else
    return (FALSE);
}
