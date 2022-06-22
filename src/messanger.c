#include <math.h>
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "screen.h"
#include "constants.h"
#include "oasis.h"
#include "warzone.h"
#include "clan.h"
#include "dg_scripts.h"
#include "arena.h"
#include "crystalgate.h"
#include "mob_party.h"

extern struct room_data *world;
extern struct char_data *mobs;
extern struct char_data *character_list;
extern struct obj_data *objs;
extern struct obj_data *object_list;
extern room_rnum top_of_world;
extern struct zone_data *zone_table_array;
extern struct zone_data *zone_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;


#define MESSAGE_NOTE 		100
#define THRU_ZONE_SPEED		  2
#define MESSANGER_SPEED		 10

struct special_message_data *MessageHeadList = NULL;
int SurvDistance(int X1,int Y1,int X2,int Y2);

void FreeMessage(struct special_message_data *Mes)
{
  if (!Mes) {
    log("ERROR:: Special Message, Null sent to FreeMessage");
    return;
  }

  if (Mes->TheSender)
    Mes->TheSender = my_free(Mes->TheSender);
  if (Mes->TheReciever)
    Mes->TheReciever = my_free(Mes->TheReciever);
  if (Mes->TheMessage)
    Mes->TheMessage = my_free(Mes->TheMessage);

  Mes->TimeInTransit = 0;
  Mes->TotalTimeToMessage = 0;
  Mes->next = NULL;

  my_free(Mes);
}

void do_spec_msg(struct char_data *ch, char *arg)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct obj_data *Paper = NULL;
  struct char_data *vict;
  struct special_message_data *Mes;
  extern int MessageDistance(struct char_data *ch, struct char_data *vict);

  arg = one_argument(arg, buf);
  one_argument(arg, buf2);
  
  if (!*buf) {
    send_to_char(ch, "Syntax: message <paper> <name>\r\n");
    return;
  } 

  if (!*buf2) {
    send_to_char(ch, "Who are you sending this message to?\r\n");
    return;
  }
  
  Paper = get_obj_in_list_vis(ch, buf, NULL, ch->carrying);

  if (!Paper) {
    send_to_char(ch, "Without the paper, how can you send a message?\r\n");
    return;
  }
  
  if (GET_OBJ_TYPE(Paper) != ITEM_NOTE) {
    send_to_char(ch, "That isn't an acceptable note.\r\n");
    return;
  }
  
  if (!Paper->action_description || strlen(Paper->action_description) < 4) {
   send_to_char(ch, "There is noting written on the note, I will not send that along!\r\n");
   return;
  }

  vict = get_char_vis(ch, buf2, NULL, FIND_CHAR_WORLD);

  if (!vict) {
    send_to_char(ch, "%s is not currently in the game, I cannot deliver to someone not around!\r\n", GET_NAME(vict));
    return;
  }

  if (IS_NPC(vict)) {
    send_to_char(ch, "&CA young boy appears and says, 'Sorry, but I don't deliever messages to monsters!'\r\n"
                     "He then snatches some coins from you and disappears instantly!\r\n");
    if (GET_GOLD(ch) >= 10)
      GET_GOLD(ch) -= 10;
    else {
      GET_BANK_GOLD(ch) -= 20;
      send_to_char(ch, "&RA young boy telepaths you, 'Cheapskate.. for that I'll take twice the amount from your bank! Nah Nah!'\r\n");
    }
    return;
  }

  CREATE(Mes, struct special_message_data, 1);
  Mes->TheMessage = strdup(Paper->action_description);
  Mes->TheSender = strdup(GET_NAME(ch));
  Mes->TheReciever = strdup(GET_NAME(vict));
  Mes->TimeInTransit = 0;
  if (world[vict->in_room].zone == 11)
    Mes->TotalTimeToMessage = THRU_ZONE_SPEED;
  else
    Mes->TotalTimeToMessage = MessageDistance(ch, vict) / MESSANGER_SPEED;
  Mes->next = MessageHeadList;
  MessageHeadList = Mes;

  obj_from_char(Paper);
  extract_obj(Paper);

  if (GET_GOLD(ch) >= 5)
    GET_GOLD(ch) -= 5;
  else
    GET_BANK_GOLD(ch) -= 15;

//ERROR CHECKING

  if (GET_LEVEL(ch) == LVL_IMPL) {
    send_to_char(ch, "Message Target:  %s\r\n", Mes->TheReciever);
    send_to_char(ch, "Message Sender:  %s\r\n", Mes->TheSender);
    send_to_char(ch, "Message T Time:  %d\r\n", Mes->TimeInTransit);
  }

  send_to_char(ch, "&CA young boy appears with a flash of light and takes a small amount of gold and your message to be delievered!&n\r\n");
  act("A bright flash of light blinds you for a moment.  When you regain your sight you notice a small boy taking a note from $n.  With another flash of light, the boy is gone.", FALSE, ch, 0, 0, TO_ROOM);
  return;
}


void deliver_message(struct special_message_data *Mes, struct char_data *ch)
{
  struct obj_data *Paper;
  obj_vnum note;
  char buf[MAX_INPUT_LENGTH];

  if (!Mes || !ch) {
    log("ERROR: Special Message, null sent to deliver_message");
    return;
  }

  if (!Mes->TheMessage || !Mes->TheSender) {
    log("ERROR: Special Message, bad message or sender in deliver_message");
    return;
  }
  
  note = real_object(MESSAGE_NOTE);

  Paper = read_object(note, REAL);
  sprintf(buf, "A message from %s", Mes->TheSender);
  Paper->short_description = strdup(buf);
  Paper->action_description = strdup(Mes->TheMessage);

  act("With a flash of blinding light, a small boy appears and hands a message to $n.  Then vanishes with another flash of light.", FALSE, ch, 0, 0, TO_ROOM);
  send_to_char(ch, "&CA young boy appears with a flash of light and hands you a note!\r\n");
  obj_to_char(Paper, ch);
  return;
}


void SpecialMessageUpdate(void)
{

  struct special_message_data *Mes;
  struct special_message_data *Next, *temp;
  struct char_data *vict;
  struct char_data *get_msg_player(char *name);

  for(Mes = MessageHeadList; Mes; Mes = Next) {
                Next = Mes->next;
                if(++Mes->TimeInTransit >= Mes->TotalTimeToMessage) {
                    if (!Mes->TheReciever) {                  
                        log("ERROR: Special Message, Reciever name lost in SpecialMessageUpdate()");
                        continue;
                    }
                        vict = get_msg_player(Mes->TheReciever); 
                        if (vict == NULL) {
                                        REMOVE_FROM_LIST(Mes, MessageHeadList, next);
                                        FreeMessage(Mes);
                                        continue;
                        }
                        deliver_message(Mes, vict);
                        REMOVE_FROM_LIST(Mes, MessageHeadList, next);
                        FreeMessage(Mes);
                }
        }                       
}              
/*
int MessageDistance(struct char_data *ch, struct char_data *vict)
{
  int Distance = 0;

  if (!ch || !vict)
    return (0);

  Distance = (int)SurvDistance(GET_MAPX(ch), GET_MAPY(ch), GET_MAPX(vict), GET_MAPY(vict));
  return (Distance);;
}


int SurvDistance(int X1,int Y1,int X2,int Y2)
{
double ChangeX, ChangeY;
double Dist;
        ChangeX = (X1 - X2);
        ChangeX *= ChangeX;
        ChangeY = (Y1 - Y2);
        ChangeY *= ChangeY;
        Dist = sqrt((double)(ChangeX + ChangeY));
        return(Dist);
}
*/
