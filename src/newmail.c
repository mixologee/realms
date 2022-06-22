/**************************************************************************
*  File: objsave.c                                         Part of tbaMUD *
*  Usage: loading/saving player objects for rent and crash-save           *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"

/* external variables */

extern struct mail_data *mail_list;
long int top_of_maillist;

int save_mail_record(struct obj_data *obj, long recip, struct char_data *sender);

ACMD(do_sendmail)
{
  char name[MAX_INPUT_LENGTH], item[MAX_INPUT_LENGTH];
  struct mail_data *mail;
  struct obj_data *obj;
  long id;
  FILE *fl;
  
  two_arguments(argument, item, name);

  if(!*item) {
    send_to_char(ch, "What are we sending?\r\n");
    return;
  }

  if(!*name) {
    send_to_char(ch, "Who are we sending stuff to?\r\n");
    return;
  }

  if ((id = get_id_by_name(name)) < 0) {
    send_to_char(ch, "No one by that name is registered here!");
    return;
  }

  if (!(obj = get_obj_vis(ch, item, NULL)) != NULL) {
     send_to_char(ch, "You do not have that item.\r\n");
     return;
   }

   send_to_char(ch, "You want to mail %s to %s.\r\n", obj->short_description, name);
  
   CREATE(mail, struct mail_data, 1);
   mail->id = top_of_maillist;
   mail->next = mail_list;
   mail_list = mail;
   mail->recip = id;
   mail->sender = ch;
   mail->obj = obj;
   mail->received = FALSE;
   top_of_maillist++;   
   obj_from_char(obj);
}


ACMD(do_receive_mail)
{
  struct mail_data *tmp=NULL;
  struct obj_data *obj;
  bool has_mail = FALSE;

  tmp = mail_list;

  for ( ; tmp ; tmp = tmp->next) {

    if (tmp->recip != GET_IDNUM(ch) || tmp->received == TRUE) {
      continue;
    }

    obj = create_obj();
    obj->item_number = NOWHERE;

    obj = tmp->obj;
    tmp->received = TRUE;

    send_to_char(ch, "You receive %s from %s.\r\n", obj->short_description, GET_NAME(tmp->sender));
    obj_to_char(obj, ch);
    has_mail = TRUE;
  }

  if (has_mail == FALSE)
    send_to_char(ch, "You have no mail waiting for you right now.\r\n");

}

ACMD(do_show_mail)
{
  struct mail_data *tmp = NULL;

  tmp = mail_list;

  send_to_char(ch, "List of Sent Mail in the Realms                                \r\n");
  send_to_char(ch, "MailID   RecipID    Sender         Item Name               Received? \r\n");
  send_to_char(ch, "--------------------------------------------------------------------\r\n");


  for ( ; tmp ; tmp = tmp->next) {
    send_to_char(ch, "   %-5ld   %-5ld  %-20s %-20s %-3s\r\n", tmp->id, tmp->recip, tmp->sender->player.name, tmp->obj->short_description, tmp->received ? "Yes" : "No");
  }
}

