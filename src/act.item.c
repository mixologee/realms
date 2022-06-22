 //
//   act.item.c     
//
//   object handling routines -- get/drop and container handling 
//

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
#include "constants.h"
#include "dg_scripts.h"
#include "quest.h"
#include "logger.h"

// BEGIN AUCTION DEFINES
void auc_id(struct obj_data *obj);

int curbid = 0;			    // current bid on item being auctioned 
int aucstat = AUC_NULL_STATE;	     // state of auction.. first_bid etc.. 
struct obj_data *obj_selling = NULL;	// current object for sale
struct char_data *ch_selling = NULL;	// current character selling obj
struct char_data *ch_buying  = NULL;	// current character buying the object 

char *auctioneer[AUC_BID + 1] = {
	
	"&GThe auctioneer auctions,&W '$n puts $p up for sale at %d coins.'&n\r\n",
	"&GThe auctioneer auctions,&W '$p at %d coins going once!.'&n\r\n",
	"&GThe auctioneer auctions,&W '$p at %d coins going twice!.'&n\r\n",
	"&GThe auctioneer auctions,&W 'Last call: $p going for %d coins to $n.'&n\r\n",
	"&GThe auctioneer auctions,&W 'Nobody wanted $p.'&n\r\n",
	"&GThe auctioneer auctions,&W 'SOLD! $p to $n for %d coins!.'&n\r\n",
	"&GThe auctioneer auctions,&W 'Sorry, $n has cancelled the auction.'&n\r\n",
	"&GThe auctioneer auctions,&W 'Sorry, $n has left us, the auction can't go on.'&n\r\n",
	"&GThe auctioneer auctions,&W 'Sorry, $p has been confiscated, shame on you $n.'&n\r\n",
	"&GThe auctioneer tells you,&W '$n is selling $p for %d gold.'&n\r\n",
	"&GThe auctioneer auctions,&W '$n bids %d coins on $p.'&n\r\n"
};

// Extern Vars 
extern struct descriptor_data *descriptor_list;

// Local functions 
void start_auction(struct char_data * ch, struct obj_data * obj, int bid);
void auc_stat(struct char_data * ch, struct obj_data *obj);
void stop_auction(int type, struct char_data * ch);
void check_auction(void);
void auc_send_to_all(char *messg, bool buyer);
int GET_TOTAL_LEVEL(struct char_data *ch);

ACMD(do_auction);
ACMD(do_bid);
// END AUCTION DEFINES

// local functions
int can_take_obj(struct char_data *ch, struct obj_data *obj);
void get_check_money(struct char_data *ch, struct obj_data *obj);
int perform_get_from_room(struct char_data *ch, struct obj_data *obj);
void get_from_room(struct char_data *ch, char *arg, int amount);
void perform_give_gold(struct char_data *ch, struct char_data *vict, int amount);
void perform_give(struct char_data *ch, struct char_data *vict, struct obj_data *obj);
int perform_drop(struct char_data *ch, struct obj_data *obj, byte mode, const char *sname, room_rnum RDR);
void perform_drop_gold(struct char_data *ch, int amount, byte mode, room_rnum RDR);
struct char_data *give_find_vict(struct char_data *ch, char *arg);
void weight_change_object(struct obj_data *obj, int weight);
void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont);
void name_from_drinkcon(struct obj_data *obj);
void get_from_container(struct char_data *ch, struct obj_data *cont, char *arg, int mode, int amount);
void name_to_drinkcon(struct obj_data *obj, int type);
void wear_message(struct char_data *ch, struct obj_data *obj, int where);
void perform_wear(struct char_data *ch, struct obj_data *obj, int where);
int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);
void perform_get_from_container(struct char_data *ch, struct obj_data *obj, struct obj_data *cont, int mode);
void perform_remove(struct char_data *ch, int pos);
void perform_remove_wear(struct char_data *ch, int pos, struct obj_data *obj);
ACMD(do_remove);
ACMD(do_put);
ACMD(do_sacrifice);
ACMD(do_get);
ACMD(do_drop);
ACMD(do_give);
ACMD(do_drink);
ACMD(do_eat);
ACMD(do_pour);
ACMD(do_wear);
ACMD(do_wield);
ACMD(do_grab);
ACMD(do_split);

void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont)
{
  char buf[MAX_INPUT_LENGTH];

  if (!drop_otrigger(obj, ch))
    return;

  if (!obj) /* object might be extracted by drop_otrigger */
    return;

  if ((GET_OBJ_BOUND_ID(cont) != NOBODY) && (GET_OBJ_BOUND_ID(cont) != GET_IDNUM(ch))) {
    if (get_name_by_id(GET_OBJ_BOUND_ID(cont)) != NULL) {
      sprintf(buf, "$p belongs to %s.  You cannot put anything inside it.\r\n", CAP(get_name_by_id(GET_OBJ_BOUND_ID(cont))));
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
      return;
    }
  }

  if (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj) > GET_OBJ_VAL(cont, 0))
    act("$p won't fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
  else if (OBJ_FLAGGED(obj, ITEM_NODROP) && IN_ROOM(cont) != NOWHERE)
    act("You can't get $p out of your hand.", FALSE, ch, obj, NULL, TO_CHAR);
  else {
    obj_from_char(obj);
    obj_to_obj(obj, cont);

    act("$n puts $p in $P.", TRUE, ch, obj, cont, TO_ROOM);

// Yes, I realize this is strange until we have auto-equip on rent. -gg 
    if (OBJ_FLAGGED(obj, ITEM_NODROP) && !OBJ_FLAGGED(cont, ITEM_NODROP)) {
      SET_BIT_AR(GET_OBJ_EXTRA(cont), ITEM_NODROP);
      act("You get a strange feeling as you put $p in $P.", FALSE,
                ch, obj, cont, TO_CHAR);
    } else
      act("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
      save_char(ch);
  }
}

//
// The following put modes are supported by the code below:
//
//	1) put <object> <container>
//	2) put all.<object> <container>
//	3) put all <container>
//
//	<container> must be in inventory or on ground.
//	all objects to be put into container must be in inventory.
//

ACMD(do_put)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj, *cont;
  struct char_data *tmp_char;
  int obj_dotmode, cont_dotmode, found = 0, howmany = 1;
  char *theobj, *thecont;

  one_argument(two_arguments(argument, arg1, arg2), arg3);	//three_arguments 

  if (*arg3 && is_number(arg1)) {
    howmany = atoi(arg1);
    theobj = arg2;
    thecont = arg3;
  } else {
    theobj = arg1;
    thecont = arg2;
  }
  obj_dotmode = find_all_dots(theobj);
  cont_dotmode = find_all_dots(thecont);

  if (!*theobj)
    send_to_char(ch, "Put what in what?\r\n");
  else if (cont_dotmode != FIND_INDIV)
    send_to_char(ch, "You can only put things into one container at a time.\r\n");
  else if (!*thecont) {
    send_to_char(ch, "What do you want to put %s in?\r\n", obj_dotmode == FIND_INDIV ? "it" : "them");
  } else {
    generic_find(thecont, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
    if (!cont)
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(thecont), thecont);
    else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
      act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
    else if (OBJVAL_FLAGGED(cont, CONT_CLOSED))
      send_to_char(ch, "You'd better open it first!\r\n");
    else {
      if (obj_dotmode == FIND_INDIV) {	// put <obj> <container> 
	if (!(obj = get_obj_in_list_vis(ch, theobj, NULL, ch->carrying)))
	  send_to_char(ch, "You aren't carrying %s %s.\r\n", AN(theobj), theobj);
	else if (obj == cont && howmany == 1)
	  send_to_char(ch, "You attempt to fold it into itself, but fail.\r\n");
	else {
	  while (obj && howmany) {
	    next_obj = obj->next_content;
            if (obj != cont) {
              howmany--;
	      perform_put(ch, obj, cont);
            }
	    obj = get_obj_in_list_vis(ch, theobj, NULL, next_obj);
	  }
	}
      } else {
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
	      (obj_dotmode == FIND_ALL || isname(theobj, obj->name))) {
	    found = 1;
	    perform_put(ch, obj, cont);
	  }
	}
	if (!found) {
	  if (obj_dotmode == FIND_ALL)
	    send_to_char(ch, "You don't seem to have anything to put in it.\r\n");
	  else
	    send_to_char(ch, "You don't seem to have any %ss.\r\n", theobj);
	}
      }
    }
  }
}



int can_take_obj(struct char_data *ch, struct obj_data *obj)
{
  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
    act("$p: you can't carry that many items.", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  } else if ((GET_OBJ_TYPE(obj) == ITEM_CRYGATE) || (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN)) {
    send_to_char(ch, "You can't take that!\r\n");
    return(0);
  } else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    act("$p: you can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  } else if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) {
    act("$p: you can't take that!", FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }
  return (1);
}


void get_check_money(struct char_data *ch, struct obj_data *obj)
{
  int value = GET_OBJ_VAL(obj, 0);
  char cash[50];
 
  if (GET_OBJ_TYPE(obj) != ITEM_MONEY || value <= 0)
    return;

  extract_obj(obj);

  GET_GOLD(ch) += value;

  snprintf(cash, sizeof(cash), "%d", value);
 
  if (value == 1)
    send_to_char(ch, "There was 1 coin.\r\n");
  else
    send_to_char(ch, "There were %d coins.\r\n", value);

   if (PRF_FLAGGED(ch, PRF_AUTOSPLIT))
    do_split(ch, cash, 0, 0);

}


void perform_get_from_container(struct char_data *ch, struct obj_data *obj,
				     struct obj_data *cont, int mode)
{
  char buf[MAX_INPUT_LENGTH];

 
  if ((GET_OBJ_BOUND_ID(cont) != NOBODY) && (GET_OBJ_BOUND_ID(cont) != GET_IDNUM(ch))) {
    if (get_name_by_id(GET_OBJ_BOUND_ID(cont)) != NULL) {
      sprintf(buf, "$p belongs to %s.  You cannot get anything out of it.\r\n", CAP(get_name_by_id(GET_OBJ_BOUND_ID(cont))));
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
      return;
    }
  }  

  if (mode == FIND_OBJ_INV || can_take_obj(ch, obj)) {
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      act("$p: you can't hold any more items.", FALSE, ch, obj, 0, TO_CHAR);
    else if (get_otrigger(obj, ch) && (obj)) { /* obj may be purged */
      obj_from_obj(obj);
      obj_to_char(obj, ch);
      act("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
      act("$n gets $p from $P.", TRUE, ch, obj, cont, TO_ROOM);
      get_check_money(ch, obj);
    }
  }
}


void get_from_container(struct char_data *ch, struct obj_data *cont,
			     char *arg, int mode, int howmany)
{
  struct obj_data *obj, *next_obj;
  int obj_dotmode, found = 0;

  obj_dotmode = find_all_dots(arg);

  if (OBJVAL_FLAGGED(cont, CONT_CLOSED))
    act("$p is closed.", FALSE, ch, cont, 0, TO_CHAR);
  else if (obj_dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, NULL, cont->contains))) {
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "There doesn't seem to be %s %s in $p.", AN(arg), arg);
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
    } else {
      struct obj_data *obj_next;
      while (obj && howmany--) {
        obj_next = obj->next_content;
        perform_get_from_container(ch, obj, cont, mode);
        obj = get_obj_in_list_vis(ch, arg, NULL, obj_next);
      }
    }
  } else {
    if (obj_dotmode == FIND_ALLDOT && !*arg) {
      send_to_char(ch, "Get all of what?\r\n");
      return;
    }
    for (obj = cont->contains; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (obj_dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;
	perform_get_from_container(ch, obj, cont, mode);
      }
    }
    if (!found) {
      if (obj_dotmode == FIND_ALL)
	act("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
      else {
        char buf[MAX_STRING_LENGTH];

	snprintf(buf, sizeof(buf), "You can't seem to find any %ss in $p.", arg);
	act(buf, FALSE, ch, cont, 0, TO_CHAR);
      }
    }
  }
}


int perform_get_from_room(struct char_data *ch, struct obj_data *obj)
{
  if (can_take_obj(ch, obj) && get_otrigger(obj, ch) && (obj)) { /* obj may be purged by get_otrigger */
    obj_from_room(obj);
    obj_to_char(obj, ch);
    act("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
    get_check_money(ch, obj);
    return (1);
  }
  return (0);
}


void get_from_room(struct char_data *ch, char *arg, int howmany)
{
  struct obj_data *obj, *next_obj;
  int dotmode, found = 0;

  dotmode = find_all_dots(arg);

  if (dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents)))
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(arg), arg);
    else {
      struct obj_data *obj_next;
      while(obj && howmany--) {
	obj_next = obj->next_content;
        perform_get_from_room(ch, obj);
        obj = get_obj_in_list_vis(ch, arg, NULL, obj_next);
      }
    }
  } else {
    if (dotmode == FIND_ALLDOT && !*arg) {
      send_to_char(ch, "Get all of what?\r\n");
      return;
    }
    for (obj = world[IN_ROOM(ch)].contents; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;
	perform_get_from_room(ch, obj);
        save_char(ch);
      }
    }
    if (!found) {
      if (dotmode == FIND_ALL)
	send_to_char(ch, "There doesn't seem to be anything here.\r\n");
      else
	send_to_char(ch, "You don't see any %ss here.\r\n", arg);
    }
  }
}

ACMD(do_sacrifice)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj;



  one_argument(argument, arg);

  if (!*arg)
  {
    send_to_char(ch, "Sacrifice what?");
    return;
  }

  
   if (!(obj = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents)))
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(arg), arg);
  
   else if ((GET_OBJ_TYPE(obj) == ITEM_CRYGATE) || (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN)) {
      send_to_char(ch, "You cannot sacrifice that!\r\n");
      return;
   }

   else 

   {
      extract_obj(obj);
      send_to_char(ch, "\r\n&GYou recieve &W%d &Gexperience points from the creators for your sacrifice.&n\r\n",
                   (2 * GET_LEVEL(ch)));
      GET_EXP(ch) += (2 * GET_LEVEL(ch));
   }
  
} 


ACMD(do_get)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];

  int cont_dotmode, found = 0, mode;
  struct obj_data *cont;
  struct char_data *tmp_char;

  one_argument(two_arguments(argument, arg1, arg2), arg3);	// three_arguments 

  if (!*arg1)
    send_to_char(ch, "Get what?\r\n");
  else if (!*arg2)
    get_from_room(ch, arg1, 1);
  else if (is_number(arg1) && !*arg3)
    get_from_room(ch, arg2, atoi(arg1));
  else {
    int amount = 1;
    if (is_number(arg1)) {
      amount = atoi(arg1);
      strcpy(arg1, arg2);	// strcpy: OK (sizeof: arg1 == arg2) 
      strcpy(arg2, arg3);	// strcpy: OK (sizeof: arg2 == arg3) 
    }
    cont_dotmode = find_all_dots(arg2);
    if (cont_dotmode == FIND_INDIV) {
      mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
      if (!cont)
	send_to_char(ch, "You don't have %s %s.\r\n", AN(arg2), arg2);
      else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
	act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
      else
	get_from_container(ch, cont, arg1, mode, amount);
    } else {
      if (cont_dotmode == FIND_ALLDOT && !*arg2) {
	send_to_char(ch, "Get from all of what?\r\n");
	return;
      }
      for (cont = ch->carrying; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    found = 1;
	    get_from_container(ch, cont, arg1, FIND_OBJ_INV, amount);
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    found = 1;
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	  }
	}
      for (cont = world[IN_ROOM(ch)].contents; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, amount);
	    found = 1;
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	    found = 1;
	  }
	}
      if (!found) {
	if (cont_dotmode == FIND_ALL)
	  send_to_char(ch, "You can't seem to find any containers.\r\n");
	else
	  send_to_char(ch, "You can't seem to find any %ss here.\r\n", arg2);
      }
    }
  }
}


void perform_drop_gold(struct char_data *ch, int amount,
		            byte mode, room_rnum RDR)
{
  struct obj_data *obj;

  if (amount <= 0)
    send_to_char(ch, "Heh heh heh.. we are jolly funny today, eh?\r\n");
  else if (GET_GOLD(ch) < amount)
    send_to_char(ch, "You don't have that many coins!\r\n");
  else {
    if (mode != SCMD_JUNK) {
      WAIT_STATE(ch, PULSE_VIOLENCE);	// to prevent coin-bombing //
      obj = create_money(amount);
      if (mode == SCMD_DONATE) {
	send_to_char(ch, "You throw some gold into the air where it disappears in a puff of smoke!\r\n");
	act("$n throws some gold into the air where it disappears in a puff of smoke!",
	    FALSE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, RDR);
	act("$p suddenly appears in a puff of orange smoke!", 0, 0, obj, 0, TO_ROOM);
      } else {
        char buf[MAX_STRING_LENGTH];

        if (!drop_wtrigger(obj, ch) && (obj)) { /* obj may be purged */
          extract_obj(obj);
          return;
        }

	snprintf(buf, sizeof(buf), "$n drops %s.", money_desc(amount));
	act(buf, TRUE, ch, 0, 0, TO_ROOM);

	send_to_char(ch, "You drop some gold.\r\n");
	obj_to_room(obj, IN_ROOM(ch));
      }
    } else {
      char buf[MAX_STRING_LENGTH];

      snprintf(buf, sizeof(buf), "$n drops %s which disappears in a puff of smoke!", money_desc(amount));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);

      send_to_char(ch, "You drop some gold which disappears in a puff of smoke!\r\n");
    }
    GET_GOLD(ch) -= amount;
  }
}


#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")

int perform_drop(struct char_data *ch, struct obj_data *obj,
		     byte mode, const char *sname, room_rnum RDR)
{
  char buf[MAX_STRING_LENGTH];
  int value;

  if (!drop_otrigger(obj, ch))
    return 0;
  
  if ((mode == SCMD_DROP) && !drop_wtrigger(obj, ch))
    return 0;

  if (!obj) /* obj may be purged */
    return 0;


  if (OBJ_FLAGGED(obj, ITEM_NODROP)) {
    snprintf(buf, sizeof(buf), "You can't %s $p, it must be CURSED!", sname);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return (0);
  }

  snprintf(buf, sizeof(buf), "You %s $p.%s", sname, VANISH(mode));
  act(buf, FALSE, ch, obj, 0, TO_CHAR);

  snprintf(buf, sizeof(buf), "$n %ss $p.%s", sname, VANISH(mode));
  act(buf, TRUE, ch, obj, 0, TO_ROOM);

  obj_from_char(obj);

  if ((mode == SCMD_DONATE) && OBJ_FLAGGED(obj, ITEM_NODONATE))
    mode = SCMD_JUNK;

  switch (mode) {
  case SCMD_DROP:
    obj_to_room(obj, IN_ROOM(ch));
    return (0);
  case SCMD_DONATE:
    obj_to_room(obj, RDR);
    act("$p suddenly appears in a puff a smoke!", FALSE, 0, obj, 0, TO_ROOM);
    return (0);
  case SCMD_JUNK:
    value = MAX(1, MIN(200, GET_OBJ_COST(obj) / 16));
    extract_obj(obj);
    return (value);
  default:
    log("SYSERR: Incorrect argument %d passed to perform_drop.", mode);
    break;
  }

  return (0);
}



ACMD(do_drop)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj;
  room_rnum RDR = 0;
  byte mode = SCMD_DROP;
  int dotmode, amount = 0, multi, num_don_rooms;
  const char *sname;

  switch (subcmd) {
  case SCMD_JUNK:
    sname = "junk";
    mode = SCMD_JUNK;
    break;
  case SCMD_DONATE:
    sname = "donate";
    mode = SCMD_DONATE;
    // fail + double chance for room 1   
    num_don_rooms = (CONFIG_DON_ROOM_1 != 7232) * 2 +       
                    (CONFIG_DON_ROOM_2 != 6116) * 2 +
                    (CONFIG_DON_ROOM_3 != NOWHERE)     + 1 ; 
    switch (rand_number(0, num_don_rooms)) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      RDR = real_room(CONFIG_DON_ROOM_1);
      break;
//    case 3: RDR = real_room(CONFIG_DON_ROOM_2); break;
//    case 4: RDR = real_room(CONFIG_DON_ROOM_3); break;

    }
    if (RDR == NOWHERE) {
      send_to_char(ch, "Sorry, you can't donate anything right now.\r\n");
      return;
    }
    break;
  default:
    sname = "drop";
    break;
  }

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "What do you want to %s?\r\n", sname);
    return;
  } else if (is_number(arg)) {
    multi = atoi(arg);
    one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg))
      perform_drop_gold(ch, multi, mode, RDR);
    else if (multi <= 0)
      send_to_char(ch, "Yeah, that makes sense.\r\n");
    else if (!*arg)
      send_to_char(ch, "What do you want to %s %d of?\r\n", sname, multi);
    else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
      send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);
    else {
      do {
        next_obj = get_obj_in_list_vis(ch, arg, NULL, obj->next_content);
        amount += perform_drop(ch, obj, mode, sname, RDR);
        obj = next_obj;
      } while (obj && --multi);
    }
  } else {
    dotmode = find_all_dots(arg);

    // Can't junk or donate all 
    if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
      if (subcmd == SCMD_JUNK)
	send_to_char(ch, "Go to the dump if you want to junk EVERYTHING!\r\n");
      else
	send_to_char(ch, "Go do the donation room if you want to donate EVERYTHING!\r\n");
      return;
    }
    if (dotmode == FIND_ALL) {
      if (!ch->carrying)
	send_to_char(ch, "You don't seem to be carrying anything.\r\n");
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  amount += perform_drop(ch, obj, mode, sname, RDR);
	}
    } else if (dotmode == FIND_ALLDOT) {
      if (!*arg) {
	send_to_char(ch, "What do you want to %s all of?\r\n", sname);
	return;
      }
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);

      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg, NULL, obj->next_content);
	amount += perform_drop(ch, obj, mode, sname, RDR);
	obj = next_obj;
      }
    } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      else
	amount += perform_drop(ch, obj, mode, sname, RDR);
    }
  }

  if (amount && (subcmd == SCMD_JUNK)) {
    send_to_char(ch, "You have been rewarded by the gods!\r\n");
    act("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
    GET_GOLD(ch) += amount;
  }
}


void perform_give(struct char_data *ch, struct char_data *vict,
		       struct obj_data *obj)
{
  if (!give_otrigger(obj, ch, vict) || !obj)  /* obj might be purged */
    return;
  if (!receive_mtrigger(vict, ch, obj) || !obj)  /* obj might be purged */
    return;

  if (OBJ_FLAGGED(obj, ITEM_NODROP)) {
    act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
    act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict) > CAN_CARRY_W(vict)) {
    act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  obj_from_char(obj);
  obj_to_char(obj, vict);
  act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
  act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
  act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);

  if (GET_LEVEL(ch) || GET_LEVEL(vict) >= LVL_GOD)
    nmudlog(GIVE_LOG, LVL_IMPL, TRUE, "%s gave %s to %s!", GET_NAME(ch), obj->name, GET_NAME(vict));

  autoquest_trigger_check( ch, vict, obj, AQ_OBJ_RETURN);
}

// utility function for give 
struct char_data *give_find_vict(struct char_data *ch, char *arg)
{
  struct char_data *vict;

  skip_spaces(&arg);
  if (!*arg)
    send_to_char(ch, "To who?\r\n");
  else if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (vict == ch)
    send_to_char(ch, "What's the point of that?\r\n");
  else
    return (vict);

  return (NULL);
}


void perform_give_gold(struct char_data *ch, struct char_data *vict,
		            int amount)
{
  char buf[MAX_STRING_LENGTH];

  if (amount <= 0) {
    send_to_char(ch, "Heh heh heh ... we are jolly funny today, eh?\r\n");
    return;
  }
  if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))) {
    send_to_char(ch, "You don't have that many coins!\r\n");
    return;
  }
  send_to_char(ch, "%s", CONFIG_OK);

  snprintf(buf, sizeof(buf), "$n gives you %d gold coin%s.", amount, amount == 1 ? "" : "s");
  act(buf, FALSE, ch, 0, vict, TO_VICT);

  snprintf(buf, sizeof(buf), "$n gives %s to $N.", money_desc(amount));
  act(buf, TRUE, ch, 0, vict, TO_NOTVICT);

  if (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_GOD))
    GET_GOLD(ch) -= amount;
    GET_GOLD(vict) += amount;

  bribe_mtrigger(vict, ch, amount);
}


ACMD(do_give)
{
  char arg[MAX_STRING_LENGTH];
  int amount, dotmode;
  struct char_data *vict;
  struct obj_data *obj, *next_obj;

  argument = one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Give what to who?\r\n");
  else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg)) {
      one_argument(argument, arg);
      if ((vict = give_find_vict(ch, arg)) != NULL)
	perform_give_gold(ch, vict, amount);
      return;
    } else if (!*arg)	// Give multiple code. //
      send_to_char(ch, "What do you want to give %d of?\r\n", amount);
    else if (!(vict = give_find_vict(ch, argument)))
      return;
    else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) 
      send_to_char(ch, "You don't seem to have any %ss.\r\n", arg);
    else {
      while (obj && amount--) {
	next_obj = get_obj_in_list_vis(ch, arg, NULL, obj->next_content);
	perform_give(ch, vict, obj);
	obj = next_obj;
      }
    }
  } else {
    char buf1[MAX_INPUT_LENGTH];

    one_argument(argument, buf1);
    if (!(vict = give_find_vict(ch, buf1)))
      return;
    dotmode = find_all_dots(arg);
    if (dotmode == FIND_INDIV) {
      if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
	send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
      else
	perform_give(ch, vict, obj);
    } else {
      if (dotmode == FIND_ALLDOT && !*arg) {
	send_to_char(ch, "All of what?\r\n");
	return;
      }
      if (!ch->carrying)
	send_to_char(ch, "You don't seem to be holding anything.\r\n");
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (CAN_SEE_OBJ(ch, obj) &&
	      ((dotmode == FIND_ALL || isname(arg, obj->name))))
	    perform_give(ch, vict, obj);
            save_char(ch);
            save_char(vict);
	}
    }
  }
}



void weight_change_object(struct obj_data *obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if (IN_ROOM(obj) != NOWHERE) {
    GET_OBJ_WEIGHT(obj) += weight;
  } else if ((tmp_ch = obj->carried_by)) {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if ((tmp_obj = obj->in_obj)) {
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    log("SYSERR: Unknown attempt to subtract weight from an object.");
  }
}



void name_from_drinkcon(struct obj_data *obj)
{
  char *new_name, *cur_name, *next;
  const char *liqname;
  int liqlen, cpylen;

  if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  liqname = drinknames[GET_OBJ_VAL(obj, 2)];
  if (!isname(liqname, obj->name)) {
    log("SYSERR: Can't remove liquid '%s' from '%s' (%d) item.", liqname, obj->name, obj->item_number);
    return;
  }

  liqlen = strlen(liqname);
  CREATE(new_name, char, strlen(obj->name) - strlen(liqname)); // +1 for NUL, -1 for space 

  for (cur_name = obj->name; cur_name; cur_name = next) {
    if (*cur_name == ' ')
      cur_name++;

    if ((next = strchr(cur_name, ' ')))
      cpylen = next - cur_name;
    else
      cpylen = strlen(cur_name);

    if (!strn_cmp(cur_name, liqname, liqlen))
      continue;

    if (*new_name)
      strcat(new_name, " ");	// strcat: OK (size precalculated) 
    strncat(new_name, cur_name, cpylen);	// strncat: OK (size precalculated) 
  }

  if (GET_OBJ_RNUM(obj) == NOTHING || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);
  obj->name = new_name;
}



void name_to_drinkcon(struct obj_data *obj, int type)
{
  char *new_name;

  if (!obj || (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN))
    return;

  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
  sprintf(new_name, "%s %s", obj->name, drinknames[type]);	// sprintf: OK 

  if (GET_OBJ_RNUM(obj) == NOTHING || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);

  obj->name = new_name;
}



ACMD(do_drink)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct obj_data *temp;
  struct affected_type af;
  int amount, weight;
  int on_ground = 0;

  one_argument(argument, arg);

  if (IS_NPC(ch))	// Cannot use GET_COND() on mobs. 
    return;

  if (!*arg) {
    send_to_char(ch, "Drink from what?\r\n");
    return;
  }
  if (!(temp = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
    if (!(temp = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    } else
      on_ground = 1;
  }
  if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
      (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
    send_to_char(ch, "You can't drink from that!\r\n");
    return;
  }

  if (GET_OBJ_BOUND_ID(temp) != NOBODY) {
    if (GET_OBJ_BOUND_ID(temp) != GET_IDNUM(ch)) {
      if (get_name_by_id(GET_OBJ_BOUND_ID(temp)) == NULL)
        sprintf(buf, "$p%s belongs to someone else.  You can't drink from it.", CCNRM(ch, C_NRM));
      else
        sprintf(buf, "$p%s belongs to %s.  You can't drink from it.", CCNRM(ch, C_NRM), CAP(get_name_by_id(GET_OBJ_BOUND_ID(temp))));
      act(buf, FALSE, ch, temp, 0, TO_CHAR);
      return;
    }
  }

  if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON)) {
    send_to_char(ch, "You have to be holding that to drink from it.\r\n");
    return;
  }
  if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
    // The pig is drunk //
    send_to_char(ch, "You can't seem to get close enough to your mouth.\r\n");
    act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  if ((GET_COND(ch, HUNGER) > 20) && (GET_COND(ch, THIRST) > 0)) {
    send_to_char(ch, "Your stomach can't contain anymore!\r\n");
    return;
  }
  if (!GET_OBJ_VAL(temp, 1)) {
    send_to_char(ch, "It's empty.\r\n");
    return;
  }
  if (subcmd == SCMD_DRINK) {
    char buf[MAX_STRING_LENGTH];

    snprintf(buf, sizeof(buf), "$n drinks %s from $p.", drinks[GET_OBJ_VAL(temp, 2)]);
    act(buf, TRUE, ch, temp, 0, TO_ROOM);

    send_to_char(ch, "You drink the %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);

    if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
      amount = (25 - GET_COND(ch, THIRST)) / drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK];
    else
      amount = rand_number(3, 10);

  } else {
    act("$n sips from $p.", TRUE, ch, temp, 0, TO_ROOM);
    send_to_char(ch, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    amount = 1;
  }

  amount = MIN(amount, GET_OBJ_VAL(temp, 1));

  // You can't subtract more than the object weighs 
  weight = MIN(amount, GET_OBJ_WEIGHT(temp));

  weight_change_object(temp, -weight);	// Subtract amount 

  gain_condition(ch, DRUNK,  drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK]  * amount / 4);
  gain_condition(ch, FULL,   drink_aff[GET_OBJ_VAL(temp, 2)][FULL]   * amount / 4);
  gain_condition(ch, THIRST, drink_aff[GET_OBJ_VAL(temp, 2)][THIRST] * amount / 4);

  if (GET_COND(ch, DRUNK) > 10)
    send_to_char(ch, "You feel drunk.\r\n");

  if (GET_COND(ch, THIRST) > 20)
    send_to_char(ch, "You don't feel thirsty any more.\r\n");

  if (GET_COND(ch, FULL) > 20)
    send_to_char(ch, "You are full.\r\n");

  if (GET_OBJ_VAL(temp, 3)) {	// The crap was poisoned ! 
    send_to_char(ch, "Oops, it tasted rather strange!\r\n");
    act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = amount * 3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  // empty the container, and no longer poison. 
  GET_OBJ_VAL(temp, 1) -= amount;
  if (!GET_OBJ_VAL(temp, 1)) {	// The last bit 
    name_from_drinkcon(temp);
    GET_OBJ_VAL(temp, 2) = 0;
    GET_OBJ_VAL(temp, 3) = 0;
  }
  return;
}



ACMD(do_eat)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct obj_data *food;
  struct affected_type af;
  int amount;

  one_argument(argument, arg);

  if (IS_NPC(ch))	// Cannot use GET_COND() on mobs. 
    return;

  if (!*arg) {
    send_to_char(ch, "Eat what?\r\n");
    return;
  }
  if (!(food = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    return;
  }
  if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE(food) == ITEM_DRINKCON) ||
			       (GET_OBJ_TYPE(food) == ITEM_FOUNTAIN))) {
    do_drink(ch, argument, 0, SCMD_SIP);
    return;
  }
  if ((GET_OBJ_TYPE(food) != ITEM_FOOD) && (GET_LEVEL(ch) < LVL_GOD)) {
    send_to_char(ch, "You can't eat THAT!\r\n");
    return;
  }

  if (GET_OBJ_BOUND_ID(food) != NOBODY) {
    if (GET_OBJ_BOUND_ID(food) != GET_IDNUM(ch)) {
      if (get_name_by_id(GET_OBJ_BOUND_ID(food)) == NULL)
        sprintf(buf, "$p%s belongs to someone else.  You can't eat it.", CCNRM(ch, C_NRM));
      else
        sprintf(buf, "$p%s belongs to %s.  You can't eat it.", CCNRM(ch, C_NRM), CAP(get_name_by_id(GET_OBJ_BOUND_ID(food))));
        act(buf, FALSE, ch, food, 0, TO_CHAR);
    }
  } 

  if (GET_COND(ch, FULL) > 20) {// Stomach full 
    send_to_char(ch, "You are too full to eat more!\r\n");
    return;
  }
  if (subcmd == SCMD_EAT) {
    act("You eat $p.", FALSE, ch, food, 0, TO_CHAR);
    act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
  } else {
    act("You nibble a little bit of $p.", FALSE, ch, food, 0, TO_CHAR);
    act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
  }

  amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL(food, 0) : 1);

  gain_condition(ch, FULL, amount);

  if (GET_COND(ch, FULL) > 20)
    send_to_char(ch, "You are full.\r\n");

  if (GET_OBJ_VAL(food, 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    // The crap was poisoned ! 
    send_to_char(ch, "Oops, that tasted rather strange!\r\n");
    act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = amount * 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  if (subcmd == SCMD_EAT)
    extract_obj(food);
  else {
    if (!(--GET_OBJ_VAL(food, 0))) {
      send_to_char(ch, "There's nothing left now.\r\n");
      extract_obj(food);
    }
  }
}


ACMD(do_pour)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  struct obj_data *from_obj = NULL, *to_obj = NULL;
  int amount;

  two_arguments(argument, arg1, arg2);

  if (subcmd == SCMD_POUR) {
    if (!*arg1) {		// No arguments 
      send_to_char(ch, "From what do you want to pour?\r\n");
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
      send_to_char(ch, "You can't pour from that!\r\n");
      return;
    }
  }
  if (subcmd == SCMD_FILL) {
    if (!*arg1) {		// no arguments 
      send_to_char(ch, "What do you want to fill?  And what are you filling it from?\r\n");
      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    }
    if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
      act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!*arg2) {		// no 2nd argument 
      act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg2, NULL, world[IN_ROOM(ch)].contents))) {
      send_to_char(ch, "There doesn't seem to be %s %s here.\r\n", AN(arg2), arg2);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN) {
      act("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
    }
  }
  if (GET_OBJ_VAL(from_obj, 1) == 0) {
    act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_POUR) {	// pour 
    if (!*arg2) {
      send_to_char(ch, "Where do you want it?  Out or in what?\r\n");
      return;
    }
    if (!str_cmp(arg2, "out")) {
      act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
      act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

      weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1)); // Empty 

      name_from_drinkcon(from_obj);
      GET_OBJ_VAL(from_obj, 1) = 0;
      GET_OBJ_VAL(from_obj, 2) = 0;
      GET_OBJ_VAL(from_obj, 3) = 0;

      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg2, NULL, ch->carrying))) {
      send_to_char(ch, "You can't find it!\r\n");
      return;
    }
    if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) &&
	(GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)) {
      send_to_char(ch, "You can't pour anything into that.\r\n");
      return;
    }
  }
  if (to_obj == from_obj) {
    send_to_char(ch, "A most unproductive effort.\r\n");
    return;
  }
  if ((GET_OBJ_VAL(to_obj, 1) != 0) &&
      (GET_OBJ_VAL(to_obj, 2) != GET_OBJ_VAL(from_obj, 2))) {
    send_to_char(ch, "There is already another liquid in it!\r\n");
    return;
  }
  if (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0))) {
    send_to_char(ch, "There is no room for more.\r\n");
    return;
  }
  if (subcmd == SCMD_POUR)
    send_to_char(ch, "You pour the %s into the %s.", drinks[GET_OBJ_VAL(from_obj, 2)], arg2);

  if (subcmd == SCMD_FILL) {
    act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
    act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
  }
  // New alias 
  if (GET_OBJ_VAL(to_obj, 1) == 0)
    name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));

  // First same type liq. 
  GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

  // Then how much to pour 
  GET_OBJ_VAL(from_obj, 1) -= (amount =
			 (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));

  GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

  if (GET_OBJ_VAL(from_obj, 1) < 0) {	// There was too little 
    GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
    amount += GET_OBJ_VAL(from_obj, 1);
    name_from_drinkcon(from_obj);
    GET_OBJ_VAL(from_obj, 1) = 0;
    GET_OBJ_VAL(from_obj, 2) = 0;
    GET_OBJ_VAL(from_obj, 3) = 0;
  }
  // Then the poison boogie 
  GET_OBJ_VAL(to_obj, 3) =
    (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3));

  // And the weight boogie 
  weight_change_object(from_obj, -amount);
  weight_change_object(to_obj, amount);	// Add weight 
}



void wear_message(struct char_data *ch, struct obj_data *obj, int where)
{
  const char *wear_messages[][2] = {
    {"$n lights $p and holds it.",
    "You light $p and hold it."},

    {"$n slides $p on to $s right ring finger.",
    "You slide $p on to your right ring finger."},

    {"$n slides $p on to $s left ring finger.",
    "You slide $p on to your left ring finger."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p on $s body.",
    "You wear $p on your body."},

    {"$n wears $p on $s head.",
    "You wear $p on your head."},

    {"$n puts $p on $s legs.",
    "You put $p on your legs."},

    {"$n wears $p on $s feet.",
    "You wear $p on your feet."},

    {"$n puts $p on $s hands.",
    "You put $p on your hands."},

    {"$n wears $p on $s arms.",
    "You wear $p on your arms."},

    {"$n straps $p around $s arm as a shield.",
    "You start to use $p as a shield."},

    {"$n wears $p about $s body.",
    "You wear $p around your body."},

    {"$n wears $p around $s waist.",
    "You wear $p around your waist."},

    {"$n puts $p on around $s right wrist.",
    "You put $p on around your right wrist."},

    {"$n puts $p on around $s left wrist.",
    "You put $p on around your left wrist."},

    {"$n wields $p.",
    "You wield $p."},

    {"$n grabs $p.",
    "You grab $p."},

    {"$n wears $p as $s aura.",
    "You wear $p as your aura."},

    {"$n wields $p.",
     "You wield $p."}
  };

  act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM);
  act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}


const char *already_wearing[] = {
    "You're already using a light.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your ring fingers.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You can't wear anything else around your neck.\r\n",
    "You're already wearing something on your body.\r\n",
    "You're already wearing something on your head.\r\n",
    "You're already wearing something on your legs.\r\n",
    "You're already wearing something on your feet.\r\n",
    "You're already wearing something on your hands.\r\n",
    "You're already wearing something on your arms.\r\n",
    "You're already using a shield.\r\n",
    "You're already wearing something about your body.\r\n",
    "You already have something around your waist.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something around both of your wrists.\r\n",
    "You're already wielding a weapon.\r\n",
    "You're already holding something.\r\n",
    "You're already using a light.\r\n",
    "You're already wearing an aura.\r\n",
    "You're already dual wielding a weapon.\r\n"
};

void perform_wear(struct char_data *ch, struct obj_data *obj, int where)
{
  char buf[MAX_INPUT_LENGTH];


// ITEM_WEAR_TAKE is used for objects that do not require special bits
// to be put into that position (e.g. you can hold any object, not just
// an object with a HOLD bit.)
   

  int wear_bitvectors[] = {
    ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
    ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
    ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
    ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
    ITEM_WEAR_WIELD, ITEM_WEAR_TAKE, ITEM_WEAR_AURA, ITEM_WEAR_DWIELD 
  };

  if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj)){
    act("You're level is too low to wear $p.", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }

  // first, make sure that the wear position is valid. 
  if (!CAN_WEAR(obj, wear_bitvectors[where])) {
    act("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
    send_to_char(ch, "where val = %d, wear_bit val = %d", where, wear_bitvectors[where]);
    return;
  }
  
  // now check to see if its bound to someone else
  if (GET_OBJ_BOUND_ID(obj) != NOBODY) {
    if (GET_OBJ_BOUND_ID(obj) != GET_IDNUM(ch)) {
      if (get_name_by_id(GET_OBJ_BOUND_ID(obj)) == NULL)
        sprintf(buf, "$p%s belongs to someone else.  You can't use it.", CCNRM(ch, C_NRM));
      else
        sprintf(buf, "$p%s belongs to %s.  You can't use it.", CCNRM(ch, C_NRM), CAP(get_name_by_id(GET_OBJ_BOUND_ID(obj))));
        act(buf, FALSE, ch, obj, 0, TO_CHAR);
      return;
    }
  }

  // for neck, finger, and wrist, try pos 2 if pos 1 is already full 
  if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R))
    if (GET_EQ(ch, where))
      where++;

/*  if (GET_EQ(ch, where)) 
   {
  perform_remove_wear(ch, where, obj);
    send_to_char(ch, "&C%s&n", already_wearing[where]);
   }
*/
  if (GET_EQ(ch, where)) {
    if (where == WEAR_WIELD) {
      if (!GET_EQ(ch, WEAR_DWIELD) && GET_SKILL(ch, SKILL_DUAL_WIELD))
      	where = WEAR_DWIELD;
      else {
	send_to_char(ch, already_wearing[where]);
	return;
      }
    }
  }
  else
  {
  /* See if a trigger disallows it */
  if (!wear_otrigger(obj, ch, where) || (obj->carried_by != ch))
    return;

     wear_message(ch, obj, where);
     obj_from_char(obj);
     equip_char(ch, obj, where);
  }
}

void perform_remove_wear(struct char_data *ch, int pos, struct obj_data *obj)
{
  struct obj_data *obj2;
                                                                                                 
  if (!(obj2 = GET_EQ(ch, pos)))
    log("SYSERR: perform_remove: bad pos %d passed.", pos);
  else if (OBJ_FLAGGED(obj2, ITEM_NODROP))
    act("You can't remove $p, it must be CURSED!", FALSE, ch, obj2, 0, TO_CHAR);
  else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    act("$p: you can't carry that many items!", FALSE, ch, obj2, 0, TO_CHAR);
  else {
    if (!remove_otrigger(obj, ch))
      return;

    obj_to_char(unequip_char(ch, pos), ch);
    act("You stop using $p.", FALSE, ch, obj2, 0, TO_CHAR);
    act("$n stops using $p.", TRUE, ch, obj2, 0, TO_ROOM);
    wear_message(ch, obj, pos);
    obj_from_char(obj);
    equip_char(ch, obj, pos);
  }
}


int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg)
{
  int where = -1;

  const char *keywords[] = {
    "!RESERVED!",
    "finger",
    "!RESERVED!",
    "neck",
    "!RESERVED!",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "shield",
    "about",
    "waist",
    "wrist",
    "!RESERVED!",
    "!RESERVED!",
    "!RESERVED!",
    "aura",
    "\n"
  };
   if (!arg || !*arg) {
    if (CAN_WEAR(obj, ITEM_WEAR_LIGHT))        where = WEAR_LIGHT;
    if (CAN_WEAR(obj, ITEM_WEAR_FINGER)){      where = WEAR_FINGER_R; if (GET_EQ(ch, WEAR_FINGER_R))      where = WEAR_FINGER_L;}
    if (CAN_WEAR(obj, ITEM_WEAR_NECK)){        where = WEAR_NECK_1; if (GET_EQ(ch, WEAR_NECK_1))        where = WEAR_NECK_2;}
    if (CAN_WEAR(obj, ITEM_WEAR_BODY))        where = WEAR_BODY;
    if (CAN_WEAR(obj, ITEM_WEAR_HEAD))        where = WEAR_HEAD;
    if (CAN_WEAR(obj, ITEM_WEAR_LEGS))        where = WEAR_LEGS;
    if (CAN_WEAR(obj, ITEM_WEAR_FEET))        where = WEAR_FEET;
    if (CAN_WEAR(obj, ITEM_WEAR_HANDS))       where = WEAR_HANDS;
    if (CAN_WEAR(obj, ITEM_WEAR_ARMS))        where = WEAR_ARMS;
    if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))      where = WEAR_SHIELD;
    if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))       where = WEAR_ABOUT;
    if (CAN_WEAR(obj, ITEM_WEAR_WAIST))       where = WEAR_WAIST;
    if (CAN_WEAR(obj, ITEM_WEAR_WRIST)){       where = WEAR_WRIST_R; if (GET_EQ(ch, WEAR_WRIST_R))       where = WEAR_WRIST_L;}
    if (CAN_WEAR(obj, ITEM_WEAR_WIELD))       where = WEAR_WIELD;
    if (CAN_WEAR(obj, ITEM_WEAR_DWIELD))       where = WEAR_WIELD;
    if (CAN_WEAR(obj, ITEM_WEAR_HOLD))        where = WEAR_HOLD;
    if (CAN_WEAR(obj, ITEM_WEAR_AURA))        where = WEAR_AURA;

    } else if ((where = search_block(arg, keywords, FALSE)) < 0)
    send_to_char(ch, "'%s'?  What part of your body is THAT?\r\n", arg);

  return (where);
}

ACMD(do_unkey)
{
 struct obj_data *obj, *next_obj;

 for (obj = ch->carrying; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (OBJ_FLAGGED(obj, ITEM_KEY)) {
        obj_from_char(obj);
        obj_to_room(obj, IN_ROOM(ch) );
      }
    }

}

ACMD(do_wear)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj;
  int where, dotmode, items_worn = 0;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char(ch, "Wear what?\r\n");
    return;
  }
  dotmode = find_all_dots(arg1);

  if (*arg2 && (dotmode != FIND_INDIV)) {
    send_to_char(ch, "You can't specify the same body location for more than one item!\r\n");
    return;
  }
  if (dotmode == FIND_ALL) {
    for (obj = ch->carrying; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, 0)) >= 0) {
	items_worn++;
	perform_wear(ch, obj, where);
      }
    }
    if (!items_worn)
      send_to_char(ch, "You don't seem to have anything wearable.\r\n");
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg1) {
      send_to_char(ch, "Wear all of what?\r\n");
      return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying)))
      send_to_char(ch, "You don't seem to have any %ss.\r\n", arg1);
    else if (GET_LEVEL(ch) < GET_OBJ_LEVEL(obj))
      send_to_char(ch, "You are not experienced enough to use that.\r\n");
    else
      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg1, NULL, obj->next_content);
	if ((where = find_eq_pos(ch, obj, 0)) >= 0)
	  perform_wear(ch, obj, where);
	else
	  act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
	obj = next_obj;
      }
  } else {
    if (!(obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying)))
      send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
    else {
      if ((where = find_eq_pos(ch, obj, arg2)) >= 0)
        if (!GET_EQ(ch, where))
	perform_wear(ch, obj, where);
       else 
        perform_remove_wear(ch, where, obj);
      else if (!*arg2)
	act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}



ACMD(do_wield)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Wield what?\r\n");
  else if (GET_EQ(ch, WEAR_DWIELD))
    send_to_char (ch, "You are getting confused. Remove the secondary weapon first.\n\r");
  else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
  else {
    if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
      send_to_char(ch, "You can't wield that.\r\n");
    else if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
      send_to_char(ch, "It's too heavy for you to use.\r\n");
    else
      if(!GET_EQ(ch, WEAR_WIELD))
        perform_wear(ch, obj, WEAR_WIELD);
      else
        perform_remove_wear(ch, WEAR_WIELD, obj);
  }
}



ACMD(do_grab)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Hold what?\r\n");
  else if (GET_EQ(ch, WEAR_DWIELD))
    send_to_char (ch, "You cannot dual wield AND hold something.\n\r");
  else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
    send_to_char(ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT) {
      perform_remove_wear(ch, WEAR_LIGHT, obj);
      perform_wear(ch, obj, WEAR_LIGHT);
    }
    else {
      if (!CAN_WEAR(obj, ITEM_WEAR_HOLD) && GET_OBJ_TYPE(obj) != ITEM_WAND &&
      GET_OBJ_TYPE(obj) != ITEM_STAFF && GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
	  GET_OBJ_TYPE(obj) != ITEM_POTION)
	send_to_char(ch, "You can't hold that.\r\n");
      else if (GET_EQ(ch, WEAR_HOLD))
        perform_remove_wear(ch, WEAR_HOLD, obj);
      else
	perform_wear(ch, obj, WEAR_HOLD);
    }
  }
}



void perform_remove(struct char_data *ch, int pos)
{
  struct obj_data *obj;

  if (!(obj = GET_EQ(ch, pos)))
    log("SYSERR: perform_remove: bad pos %d passed.", pos);
  else if (OBJ_FLAGGED(obj, ITEM_NODROP))
    act("You can't remove $p, it must be CURSED!", FALSE, ch, obj, 0, TO_CHAR);
  else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    act("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
  else {
    obj_to_char(unequip_char(ch, pos), ch);
    act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);
  }
}



ACMD(do_remove)
{
  char arg[MAX_INPUT_LENGTH];
  int i, dotmode, found;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Remove what?\r\n");
    return;
  }
  dotmode = find_all_dots(arg);

  if (dotmode == FIND_ALL) {
    found = 0;
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
	perform_remove(ch, i);
	found = 1;
      }
    if (!found)
      send_to_char(ch, "You're not using anything.\r\n");
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg)
      send_to_char(ch, "Remove all of what?\r\n");
    else {
      found = 0;
      for (i = 0; i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
	    isname(arg, GET_EQ(ch, i)->name)) {
	  perform_remove(ch, i);
	  found = 1;
	}
      if (!found)
	send_to_char(ch, "You don't seem to be using any %ss.\r\n", arg);
    }
  } else {
    if ((i = get_obj_pos_in_equip_vis(ch, arg, NULL, ch->equipment)) < 0)
      send_to_char(ch, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
    else
      perform_remove(ch, i);
      if (!GET_EQ(ch, WEAR_WIELD) && (GET_EQ(ch, WEAR_DWIELD)))
	{
	  send_to_char(ch, "Since you are not wielding a weapon, you can't dual wield too!\r\n");
	  perform_remove(ch, WEAR_DWIELD);
	}
  }
}

ACMD(do_dwield)
{
	struct obj_data *obj;
	one_argument(argument, arg);
	if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_DUAL_WIELD))
		send_to_char(ch, "You have no idea how to do that.\r\n");
	else if (!*arg)
		send_to_char(ch,"Dual wield what?\r\n");
	else if (!GET_EQ(ch, WEAR_WIELD))
		send_to_char (ch,"You must choose a primary weapon.\n\r");
	else if (FIGHTING(ch))
		send_to_char(ch,"You are too busy fighting to attempt that!\r\n");
	else if (GET_EQ(ch, WEAR_SHIELD))
		send_to_char (ch,"You cannot dual wield while using a shield.\n\r");
	else if (GET_EQ(ch, WEAR_HOLD))
		send_to_char (ch,"You can't dual wield while holding an item.\n\r");
	else if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying))) {
		sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
		send_to_char(ch, buf);
	}
// this next else statement is just an example of how to
// prevent 2 two hamded weapons from being dual wielded
// edit this if you need to, or simple delete it.
	else if (OBJ_FLAGGED(obj, ITEM_TWO_HANDED) && GET_EQ(ch, WEAR_WIELD))
		send_to_char(ch, "You cant hold 2 two-handed weapons.\r\n");
	else {
		if (!CAN_WEAR(obj, ITEM_WEAR_WIELD))
 	  	  send_to_char(ch, "You can't wield that.\r\n");
		else if (GET_LEVEL(ch) == LVL_IMPL)
		  perform_wear(ch, obj, WEAR_DWIELD);
		else if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w && GET_LEVEL(ch) < LVL_IMPL)
	 	  send_to_char(ch, "It's too heavy for you to use.\r\n");
		else
                  perform_wear(ch, obj, WEAR_DWIELD);
	}
}																											
//AUCTION FUNCTIONS AND COMMANDS
void start_auction(struct char_data * ch, struct obj_data * obj, int bid)
{
    char buf[MAX_INPUT_LENGTH];

//	Take object from character and set variables 
	
//	obj_from_char(obj);
	obj_selling = obj;
	ch_selling = ch;
	ch_buying = NULL;
	curbid = bid;

//	Tell th character where his item went 
	snprintf(buf, sizeof(buf), "%s magic flies away from your hands to be auctioned!\r\n", 
                 obj_selling->short_description);
	CAP(buf);
	send_to_char(ch_selling, "%s", buf);
	
//	Anounce the item is being sold 
	sprintf(buf, auctioneer[AUC_NULL_STATE], curbid);
	auc_send_to_all(buf, FALSE);
        auc_id(obj_selling);
 	aucstat = AUC_OFFERING;
}

void take_obj(struct char_data *ch)
{

    obj_from_char(obj_selling);
}
void check_auction(void)
{
        char buf[MAX_INPUT_LENGTH];

	switch (aucstat)
	{
	case AUC_NULL_STATE:
		return;
	case AUC_OFFERING:
	{
		sprintf(buf, auctioneer[AUC_OFFERING], curbid);
		CAP(buf);
		auc_send_to_all(buf, FALSE);
		aucstat = AUC_GOING_ONCE;
		return;
	}
	case AUC_GOING_ONCE:
	{
		
		sprintf(buf, auctioneer[AUC_GOING_ONCE], curbid);
		CAP(buf);
                auc_send_to_all(buf, FALSE);
		aucstat = AUC_LAST_CALL;
		return;
	}
	case AUC_GOING_TWICE:
	{
		
		sprintf(buf, auctioneer[AUC_GOING_TWICE], curbid);
		CAP(buf);
                auc_send_to_all(buf, FALSE);
		aucstat = AUC_LAST_CALL;
		return;
	}
	case AUC_LAST_CALL:
	{
		
		if (ch_buying == NULL) {
			
			sprintf(buf, auctioneer[AUC_LAST_CALL]);
		        CAP(buf);
			auc_send_to_all(buf, FALSE);
			
			sprintf(buf, "%s flies out the sky and into your hands.\r\n", obj_selling->short_description);
			CAP(buf);
			send_to_char(ch_selling, "%s", buf);
			obj_to_char(obj_selling, ch_selling);
			
//			Reset auctioning values 
			obj_selling = NULL;
			ch_selling = NULL;
			ch_buying = NULL;
			curbid = 0;
			aucstat = AUC_NULL_STATE;
			return;
		}
		else
		{
			
			sprintf(buf, auctioneer[AUC_SOLD], curbid);
			auc_send_to_all(buf, TRUE);
			
			// Give the object to the buyer 
                        take_obj(ch_selling);
			obj_to_char(obj_selling, ch_buying);
			sprintf(buf, "%s flies out the sky and into your hands, what a steel!\r\n", obj_selling->short_description);
			CAP(buf);
			send_to_char(ch_buying, "%s", buf);

			sprintf(buf, "Congrats! You have sold %s for %d coins!\r\n", obj_selling->short_description, curbid);
			send_to_char(ch_selling, "%s", buf);
	
			// Give selling char the money for his stuff 
			GET_GOLD(ch_selling) += curbid;
			
			// Reset auctioning values 
			obj_selling = NULL;
			ch_selling = NULL;
			ch_buying = NULL;
			curbid = 0;
			aucstat = AUC_NULL_STATE;
                        // lets save here :)
//                        save_char(ch_selling, IN_ROOM(ch_selling));
//                        save_char(ch_buying, IN_ROOM(ch_buying));
			return;						
		}

	}
	}
}


ACMD(do_auction)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   struct obj_data *obj;
   int bid = 0;
	
        if ((GET_TOTAL_LEVEL(ch)<5) || (GET_TOTAL_LEVEL(ch)<200)) {
          send_to_char(ch, "\rDue to abusive players, the auction command is limited to players from total level 6 through 200.\r\n");
          return;
        }

	two_arguments(argument, arg1, arg2);
	
	if (!*arg1) {
		send_to_char(ch, "Auction what?\r\n");
		return;
	}
	else if (is_abbrev(arg1, "cancel") || is_abbrev(arg1, "stop"))
	{
		if ((ch != ch_selling && GET_LEVEL(ch) < LVL_GRGOD) || aucstat == AUC_NULL_STATE)
		{
			send_to_char(ch, "You're not even selling anything!\r\n");
			return;
		}
		else if (ch == ch_selling)
		{
			stop_auction(AUC_NORMAL_CANCEL, NULL);
			return;
		}
		else
		{
			stop_auction(AUC_WIZ_CANCEL, ch);
		}
	}
	else if (is_abbrev(arg1, "stats") || is_abbrev(arg1, "identify"))
	{
		auc_stat(ch, obj_selling);
		return;
	}
	else if (!(obj = get_obj_in_list_vis(ch, arg1, NULL, ch->carrying))) {
		sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
		send_to_char(ch, "%s", buf);
		return;
	}
	else if (!*arg2 && (bid = obj->obj_flags.cost) <= 0) {
		sprintf(buf, "What should be the minimum bid?\r\n");
		send_to_char(ch, "%s", buf);
		return;
	}
	else if (*arg2 && (bid = atoi(arg2)) <= 0)
	{
		send_to_char(ch, "Come on? One coin at least?\r\n");
		return;
	}
	else if (aucstat != AUC_NULL_STATE) {
		sprintf(buf, "Sorry, but %s is already auctioning %s at %d coins!\r\n", GET_NAME(ch_selling), obj_selling->short_description, bid);
		send_to_char(ch, "%s", buf);
		return;
	}
	else if (OBJ_FLAGGED(obj, ITEM_NOSELL))
	{
		send_to_char(ch, "Sorry but you can't sell that!\r\n");
		return;
	}
	else 
	{
           if (bid == 0)
               bid = GET_OBJ_VAL(obj, 0);

		send_to_char(ch, "%s", CONFIG_OK);
		start_auction(ch, obj, bid);
		return;
	}
}

ACMD(do_bid)
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
	int bid;

	if(IS_NPC(ch))
		return;

	one_argument(argument, arg);

	if (!*arg) {
		send_to_char(ch, "Bid yes, good idea, but HOW MUCH??\r\n");
		return;
	}
	else if(aucstat == AUC_NULL_STATE)
	{
		send_to_char(ch, "Thats very enthusiastic of you, but nothing is being SOLD!\r\n");
		return;
	}
	else if(ch == ch_selling)
	{
		send_to_char(ch, "Why bid on something your selling?  You can 'cancel' the auction!\r\n");
		return;
	}
	else if((bid = atoi(arg)) < ((int) curbid * 1.1) && ch_buying != NULL)
	{
		sprintf(buf, "You must bid at least 10 percent more than the current bid. (%d)\r\n", (int) (curbid * 1.1));
		send_to_char(ch, "%s", buf);
		return;
	}
	else if(ch_buying == NULL && bid < curbid)
	{
		sprintf(buf, "You must at least bid the minimum!\r\n");
		send_to_char(ch, "%s", buf);
		return;
	}
	else if(bid > (GET_GOLD(ch) + GET_BANK_GOLD(ch)))
	{
		send_to_char(ch, "You don't have that much gold!\r\n");
		return;
	}
	else
	{
		if (ch == ch_buying)  
                   if (GET_BANK_GOLD(ch) > bid)               
			GET_BANK_GOLD(ch) -= (bid - curbid);
                   else if (GET_GOLD(ch) > bid)
                        GET_GOLD(ch) -= bid;
                   else
                   {
                        GET_GOLD(ch) -= (bid - GET_BANK_GOLD(ch));  
                        GET_BANK_GOLD(ch) -= GET_BANK_GOLD(ch);
                   }
		else
		{
			GET_BANK_GOLD(ch) -= bid;
			
				if(!(ch_buying == NULL))
					GET_GOLD(ch_buying) += curbid;
		}

		curbid = bid;
		ch_buying = ch;
		
		
		sprintf(buf, auctioneer[AUC_BID], bid);
		auc_send_to_all(buf, TRUE);
		
		aucstat = AUC_OFFERING;
		return;
	}
}

void stop_auction(int type, struct char_data * ch)
{
   char buf[MAX_INPUT_LENGTH];
	
	switch (type)
	{

	case AUC_NORMAL_CANCEL:
		{
		
		sprintf(buf, auctioneer[AUC_NORMAL_CANCEL]);
		auc_send_to_all(buf, FALSE);
		break;
		}
	case AUC_QUIT_CANCEL:
		{
		
		sprintf(buf, auctioneer[AUC_QUIT_CANCEL]);
		auc_send_to_all(buf, FALSE);
		break;
		}
	case AUC_WIZ_CANCEL:
		{
		
		sprintf(buf, auctioneer[AUC_WIZ_CANCEL]);
		auc_send_to_all(buf, FALSE);
		break;
		}
	default:
		{
			send_to_char(ch, "Sorry, that is an unrecognised cancel command, please report.");
			return;
		}
	}

	
	if (type != AUC_WIZ_CANCEL)
	{
		sprintf(buf, "\r\n%s flies out the sky and into your hands.\r\n", obj_selling->short_description);
		CAP(buf);
		send_to_char(ch_selling, "%s", buf);
		obj_to_char(obj_selling, ch_selling);
	}
	else
	{
		sprintf(buf, "\r\n%s flies out the sky and into your hands.\r\n", obj_selling->short_description);
		CAP(buf);
		send_to_char(ch, "%s", buf);
		obj_to_char(obj_selling, ch);
	}
	
	
	if (!(ch_buying == NULL))
		GET_BANK_GOLD(ch_buying) += curbid;
		
	obj_selling = NULL;
	ch_selling	= NULL;
	ch_buying = NULL;
	curbid = 0;

	aucstat = AUC_NULL_STATE;

}

void auc_stat(struct char_data *ch, struct obj_data *obj)
{
   char buf[MAX_INPUT_LENGTH];
			
	if (aucstat == AUC_NULL_STATE)
	{
		send_to_char(ch, "Nothing is being auctioned!\r\n");
		return;
	}
	else if (ch == ch_selling)
	{
		send_to_char(ch, "You should have found that out BEFORE auctioning it!\r\n");
		return;
	}
	else if (GET_GOLD(ch) < 500)
	{
		send_to_char(ch, "You can't afford to find the stats on that, it costs 500 coins!\r\n");
		return;
	}
	else
	{
		// auctioneer tells the character the auction details 
		sprintf(buf, auctioneer[AUC_STAT], curbid);
		act(buf, TRUE, ch_selling, obj, ch, TO_VICT | TO_SLEEP);
		GET_GOLD(ch) -= 500;	

		call_magic(ch, NULL, obj_selling, SPELL_IDENTIFY, 30, CAST_SPELL);
	}
}

void auc_send_to_all(char *messg, bool buyer)
{
  struct descriptor_data *i;

  if (messg == NULL)
    return;

  for (i = descriptor_list; i; i = i->next)
  {
	if (PRF_FLAGGED(i->character, PRF_NOAUCT))
		continue;
	
	if (buyer)
		act(messg, TRUE, ch_buying, obj_selling, i->character, TO_VICT | TO_SLEEP);
	else
		act(messg, TRUE, ch_selling, obj_selling, i->character, TO_VICT | TO_SLEEP);
  }
}

void auc_id(struct obj_data *obj)
{
  int i, found;
  size_t len;

  if (obj) {
    char bitbuf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    
    sprinttype(GET_OBJ_TYPE(obj), item_types, bitbuf, sizeof(bitbuf));
    snprintf(buf2, sizeof(buf2),"\r\n&GAuctioning: &W%s&G  \r\nLvl:&W %d  &GType:&W %s &GValue:&W %d\r\n", obj->short_description, GET_OBJ_LEVEL(obj), bitbuf, GET_OBJ_COST(obj));
    auc_send_to_all(buf2, FALSE);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      len = i = 0;

      if (GET_OBJ_VAL(obj, 1) >= 1) {
        i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s", skill_name(GET_OBJ_VAL(obj, 1)));
        if (i >= 0)
          len += i;
      }

      if (GET_OBJ_VAL(obj, 2) >= 1 && len < sizeof(bitbuf)) {
        i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s", skill_name(GET_OBJ_VAL(obj, 2)));
        if (i >= 0)
          len += i;
      }

      if (GET_OBJ_VAL(obj, 3) >= 1 && len < sizeof(bitbuf)) {
        i = snprintf(bitbuf + len, sizeof(bitbuf) - len, " %s", skill_name(GET_OBJ_VAL(obj, 3)));
        if (i >= 0)
          len += i;
      }
  auc_send_to_all(bitbuf, FALSE);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      snprintf(bitbuf, sizeof(bitbuf), "Gcasts: &W%s &Y%d &Gmax  &R%d &Gremaining.\r\n", skill_name(GET_OBJ_VAL(obj, 3)), GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
      auc_send_to_all(bitbuf, FALSE);
      break;
    case ITEM_BOW:
    case ITEM_CROSSBOW:
    case ITEM_SLING:
    case ITEM_ARROW:
    case ITEM_BOLT:
    case ITEM_ROCK:
    case ITEM_WEAPON:
      snprintf(bitbuf, sizeof(bitbuf), "&W%d&GD&W%d&G' Avg. &W%.1f.",
               GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2), 
              ((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1));
      auc_send_to_all(bitbuf, FALSE);
      break;
    case ITEM_ARMOR:
      snprintf(bitbuf, sizeof(bitbuf), "&GAC &W%d", GET_OBJ_VAL(obj, 0));
      auc_send_to_all(bitbuf, FALSE);
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
          (obj->affected[i].modifier != 0)) {
        if (!found) {
//          send_to_char(ch, "&GCan affect you as :\r\n");
          found = TRUE;
        }
      sprinttype(obj->affected[i].location, apply_types, bitbuf, sizeof(bitbuf));
      snprintf(buf2, sizeof(buf2), " &W%d &Gto &W%s ", obj->affected[i].modifier, bitbuf);
      auc_send_to_all(buf2, FALSE);
      }
    }
  } 
}

