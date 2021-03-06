
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
#include "logger.h"
#include "genmob.h"

ACMD(do_checkloadstatus);
ACMD(do_rain);
ACMD(do_gozone);
ACMD(do_dbs);
ACMD(do_godchan);
ACMD(do_gocoord);
ACMD(do_peace_all);
ACMD(do_finddoor);
ACMD(do_findkey);
ACMD(do_xprate);
ACMD(do_ptell);
ACMD(do_snatch);
ACMD(do_zlock);
ACMD(do_zunlock);
ACMD(do_vfile);

extern void vwear_obj(int type, char_data * ch);
extern void vwear_object(int wearpos, char_data * ch);
extern int exp_multiplier;
extern const char *NOPERSON;
struct obj_data *get_object_on_character(struct char_data *ch, struct char_data *vict, char *query);
struct obj_data *find_in_container(struct obj_data *container, char *query);
void clearMemory(struct char_data *ch);
int save_zone(zone_rnum zone_num);

void do_mbind(struct char_data *ch, struct char_data *vict);
void restring_object(struct obj_data *obj, int rs_pos, char *new_string) ;
void restring_mobile(struct char_data *mob, int rs_pos, char *new_string);

ACMD(do_rain) 
{                          
  struct descriptor_data *d;
  obj_data *object = NULL;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  obj_rnum obj_vnum, rnum;

  one_argument(argument, arg);



if (!*arg) {
    send_to_char(ch, "\r\nSyntax: rain <vnumber>\r\n"            
                   "Use vnum o <objtype> to find what you want to rain");
    return;       
  }                     

  if ((obj_vnum = atoi(arg)) < 0) {
    send_to_char(ch, "\r\nPlease specify a number that is not negative!\r\n");
    return;        
  }           

  if ((rnum = real_object(obj_vnum)) < 0) {
    send_to_char(ch, "\r\nThere is no object with that number.\r\n");
    return;      
  }              

  for (d = descriptor_list ; d ; d = d->next) {
    if (!d->connected && d->character && d->character != ch) {
      if (GET_LEVEL(d->character) >= LVL_IMM)
        continue;
      else {                      
        object = read_object(rnum, REAL);
        obj_to_room(object, d->character->in_room);
        act("&WHappy Days, the gods of Kuvia have rained the lands "
            "with $p, to aid you in your quest.",
            FALSE, d->character, object, 0, TO_CHAR);
      }
    }
  }
  if (object) {
    act("You rain $p over the world.", FALSE, ch, object, 0, TO_CHAR);
    sprintf(buf, "It starts raining %s, thanks to the goodness of %s", object->short_description, GET_NAME(ch));
    nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s rained %s", GET_NAME(ch), object->short_description);
  } else
    act("There are no mortals to receive your gifts.", FALSE, ch, 0, 0, TO_CHAR);
}

ACMD(do_gozone)            
{
room_rnum location = NOWHERE;
  zone_rnum zone = 0;       
  bool found = FALSE;         
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  one_argument(argument, arg);


  if (!*arg) {    
    send_to_char(ch,"Syntax: gozone <zone number>\n\r");
    return;
  }                      

  if (!isdigit(*arg)) {
     send_to_char(ch, "You must give a number.\n\r");
    return;    
  }             

  if ((location = atoi(arg)) < 0) {
    send_to_char(ch, "All zone numbers are positive!\n\r");
    return;
  }                

  for (zone = 0 ; zone <= top_of_zone_table ; zone++) {
    if (zone_table[zone].number == location) {
      found = TRUE;
      break;  
    }         
  }

  if (!found) {
    send_to_char(ch, "That zone could not be found. Use SHOW ZONE to "
              "find the correct number.\n\r");
    return;
  }

  for (location = zone_table[zone].bot ; location <= zone_table[zone].top ; location++) {
    if (real_room(location) != NOWHERE)
      break;
  }

  if ((location = real_room(location)) == NOWHERE) {
    send_to_char(ch, "The beginning room of the zone does not seem to exist!\n\r");
    return;
  }

  if (POOFOUT(ch))
    sprintf(buf, "%s&n", POOFOUT(ch));
  else
    sprintf(buf, "&R$n disappears in a puff of smoke.&n");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);

  if (POOFIN(ch))
    sprintf(buf, "%s&n", POOFIN(ch));
  else
    sprintf(buf, "&R$n appears with an ear-splitting bang.&n");


  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}


ACMD(do_peace_all)   
{                
  struct descriptor_data *d; 

  act("$n has declared world peace, for now!", FALSE, ch, 0, 0, TO_ROOM);
  send_to_room(IN_ROOM(ch), "The world is at peace.\r\n");
  nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s put the world at peace", GET_NAME(ch));
  for (d = descriptor_list; d; d = d->next) {
      if (FIGHTING(d->character))
        stop_fighting(d->character);
      if (IS_NPC(d->character)) 
        stop_fighting(d->character);
        clearMemory(d->character);
  }
}

 

ACMD(do_dbs)
{
 // use apply_types for hps etc, 

  char syntax[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int found = 0, linect = 0;
  int apply = 1;
  int i, nr, k;

  sprintf(syntax, "&BSyntax: &Cdbs <affect>&n\n\r&WWhere affect is one of the following:&c\n\r");
//  for (i = 1; i <= MAX_OBJ_AFFECT; i++) {
  for (i = 1; i <= 24; i++) {
    if (strstr(apply_types[i], "UNUSED"))
      continue;

    if (linect!= 0 && ((linect) % 3) == 0)
      sprintf(syntax, "%s\n\r", syntax);

    sprintf(syntax, "%s %-7s", syntax, apply_types_small[i]);
    linect++;
  }
  sprintf(syntax, "%s\n\r", syntax);

  if (!*argument) {
    send_to_char(ch,"&c%s",syntax);
    return;
  }

  skip_spaces(&argument);

//  for (; apply <= MAX_OBJ_AFFECT; apply++) {
  for (; apply <= 24; apply++) {
    if (is_abbrev(argument, apply_types_small[apply])) {
      found = 1;
      break;
    }
  }

  if (!found) {
    send_to_char(ch,"%s", syntax);
    return;
  }


  sprintf(buf, "\r\n&WLooking for objects with the &R%s &Waffect:"
               "\r\n&c-------------------------------------------------------"
               "\r\n&C  #     vnum      +/-      name"
               "\r\n&c-------------------------------------------------------&n\r\n",apply_types[apply]);

  for (nr = 0; nr <= top_of_objt; nr++) {
    for (k=0;k<=24;k++){
//    for (k = 0; k < MAX_OBJ_AFFECT; k++) {
      if (obj_proto[nr].affected[k].location == apply)
        sprintf(buf, "%s&W%3d. &c[&C%6d&c] &W- &g(&G%4d&g)   &m%s&n\n\r", buf, found++, obj_index[nr].vnum,
                obj_proto[nr].affected[k].modifier, obj_proto[nr].short_description);
    }
  }
  page_string(ch->desc, buf, 1);
}    


ACMD(do_godchan)
{             
  struct descriptor_data *d;                     
  char chanbuf[MAX_INPUT_LENGTH], chanbuf1[MAX_INPUT_LENGTH];           
  char buf1[MAX_STRING_LENGTH];
  char prefix[20];
  int level = 0, emote = FALSE, potato=0;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char(ch, "Say what?\r\n");
    return;
  }

  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument + 1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
        send_to_char(ch, "You can't use an immortal channel above your own level.\r\n");      
        return;
      }
    } else if (emote)
      argument++;
    break;
  default:
    break;
  }
  switch (subcmd) {
  case SCMD_IMPL:
    sprintf(chanbuf, "&nImplementors&R [%s]: &n%s\r\n", GET_NAME(ch), argument);
    sprintf(chanbuf1, "%s", chanbuf);
    level = LVL_IMPL;
    potato = PRF_NOWIZ;
 //   add_history(d->character, chanbuf, HIST_IMPL);
    break;
  case SCMD_GRGOD:
    sprintf(chanbuf, "&MGrGods&R [%s]:&n %s\r\n", GET_NAME(ch), argument);
    sprintf(chanbuf1, "%s", chanbuf);
    level = LVL_GRGOD;
    potato = PRF_NOWIZ;
 //   add_history(d->character, chanbuf, HIST_GRGOD);
    break;
  case SCMD_GOD:
    sprintf(chanbuf, "&gGod &R[%s]: &n%s\r\n", GET_NAME(ch), argument);
    sprintf(chanbuf1, "%s", chanbuf);
    level = LVL_GOD;
    potato = PRF_NOWIZ;
 //   add_history(d->character, chanbuf, HIST_GOD);
    break;
  case SCMD_WIZNET:
    sprintf(chanbuf, "&CWiznet &R[%s]: &n%s\r\n", GET_NAME(ch), argument);
    level = LVL_IMMORT;
    potato = PRF_NOWIZ;
 //   add_history(d->character, chanbuf, HIST_WIZNET);
    break;            
  case SCMD_PRECEPTORCHAN:
    if (PLR_FLAGGED(ch, PLR_PRECEPTOR)) {
    sprintf(chanbuf, "&YPreceptor &R[%s]: &n%s\r\n", GET_NAME(ch), argument);
    level = LVL_IMMORT;
    potato = PRF2_NOPRECEPTOR;
 //   add_history(d->character, chanbuf, HIST_PREC);
    break;
    }
    break;
  case SCMD_IMMORT:
    *prefix = '\0';

    if (GET_LEVEL(ch) >= LVL_IMMORT)   
      sprintf(prefix, "&G");

    if (GET_LEVEL(ch) > LVL_IMMORT && !level) {
      sprintf(chanbuf, "%s%s&B(&W%d&B) &W:: &C%s%s\r\n", prefix, GET_NAME(ch),GET_LEVEL(ch),  emote ? "<--- " : "", argument);
      sprintf(chanbuf1,"&CSomeone &W:: &C%s%s\r\n",  emote ? "<--- " : "", argument);
    } else if (GET_LEVEL(ch) > LVL_IMMORT && level) {
      sprintf(chanbuf, "%s%s&B(&W%d&B) &W:: &C%s%s\r\n", prefix, GET_NAME(ch), level, emote ? "<--- " : "", argument);
      sprintf(chanbuf1,"&CSomeone :: %s%s\r\n", emote ? "<--- " : "", argument);
    } else {     
      sprintf(chanbuf, "%s%s&W :: &C%s%s\r\n", prefix, GET_NAME(ch), emote ? "<--- " : "", argument);
      sprintf(chanbuf1,"&CSomeone &W:: &C%s%s\r\n", emote ? "<--- " : "", argument);
    }     
    potato = PRF_NOWIZ;
//    add_history(d->character, chanbuf, HIST_WIZNET);
    break;         
  default:
    send_to_char(ch, "&RERROR in do_channel, please report to Slurk with reference to do_channel error.&n\r\n");
    return;         
    break;             
  }

  for (d = descriptor_list; d; d = d->next) {

   if(STATE(d) != CON_PLAYING)
      continue;
   if (GET_LEVEL(d->character) <= LVL_IMMORT)
      continue;
   if (PRF_FLAGGED(d->character, potato))
      continue;
   if ((PRF_FLAGGED(d->character, PRF_NOREPEAT)))
      continue;
  
//      if (CAN_SEE(d->character, ch))
        send_to_char(d->character, chanbuf);
//      else
//        send_to_char(d->character, chanbuf1);
    }
}


ACMD(do_vwear)
{
  int i, j;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];

  struct virtual_wear_location {
    const char *loc;
    void (*pointer) (int pos, char_data *ch);
    int pos;
  }
  fields[] = {
               { "finger"    , vwear_object, ITEM_WEAR_FINGER },
               { "neck"      , vwear_object, ITEM_WEAR_NECK },
               { "body"      , vwear_object, ITEM_WEAR_BODY },
               { "head"      , vwear_object, ITEM_WEAR_HEAD },
               { "legs"      , vwear_object, ITEM_WEAR_LEGS },
               { "feet"      , vwear_object, ITEM_WEAR_FEET },
               { "hands"     , vwear_object, ITEM_WEAR_HANDS },
               { "arms"      , vwear_object, ITEM_WEAR_ARMS },
               { "shield"    , vwear_object, ITEM_WEAR_SHIELD },
               { "about"     , vwear_object, ITEM_WEAR_ABOUT },
               { "waist"     , vwear_object, ITEM_WEAR_WAIST },
               { "wrist"     , vwear_object, ITEM_WEAR_WRIST },
               { "wield"     , vwear_object, ITEM_WEAR_WIELD },
               { "held"      , vwear_object, ITEM_WEAR_HOLD },
               { "aura"      , vwear_object, ITEM_WEAR_AURA },
               { "light"     , vwear_obj   , ITEM_LIGHT },
               { "scroll"    , vwear_obj   , ITEM_SCROLL },
               { "wand"      , vwear_obj   , ITEM_WAND },
               { "staff"     , vwear_obj   , ITEM_STAFF },
               { "weapon"    , vwear_obj   , ITEM_WEAPON },
               { "crygate"   , vwear_obj   , ITEM_CRYGATE },
               { "treasure"  , vwear_obj   , ITEM_TREASURE },
               { "armor"    , vwear_obj   , ITEM_ARMOR },
               { "potion"    , vwear_obj   , ITEM_POTION },
               { "worn"      , vwear_obj   , ITEM_WORN },
               { "other"     , vwear_obj   , ITEM_OTHER },
               { "trash"     , vwear_obj   , ITEM_TRASH },
               { "trap"      , vwear_obj   , ITEM_TRAP },
               { "container" , vwear_obj   , ITEM_CONTAINER },
               { "note"      , vwear_obj   , ITEM_NOTE },
               { "drinkcontainer", vwear_obj, ITEM_DRINKCON },
               { "key"       , vwear_obj   , ITEM_KEY },
               { "food"      , vwear_obj   , ITEM_FOOD },
               { "money"     , vwear_obj   , ITEM_MONEY },
               { "pen"       , vwear_obj   , ITEM_PEN },
               { "boat"      , vwear_obj   , ITEM_BOAT },
               { "fountain"  , vwear_obj   , ITEM_FOUNTAIN },
               { "pole",    vwear_obj , ITEM_POLE },
               { "bow"       , vwear_obj   , ITEM_BOW },
               { "sling"      , vwear_obj   , ITEM_SLING },
               { "crossbow", vwear_obj , ITEM_CROSSBOW },
               { "bolt"       , vwear_obj , ITEM_BOLT },
               { "arrow"     , vwear_obj , ITEM_ARROW },
               { "rock"       , vwear_obj , ITEM_ROCK },
               { "crumblekey", vwear_obj , ITEM_CRUM_KEY },
               { "comp"      , vwear_obj , ITEM_COMP },
               { "autoquest", vwear_obj , ITEM_AUTOQUEST },
               { "throw"      , vwear_obj , ITEM_THROW },
               { "furniture"  , vwear_obj , ITEM_FURNITURE },
               { "bow"        , vwear_obj , ITEM_BOW },
	       { "crossbow"   , vwear_obj , ITEM_CROSSBOW },
	       { "sling"      , vwear_obj , ITEM_SLING },
               { "\n"        , NULL        , 0 }
             };

  one_argument(argument, arg);

  if (!*arg) {
    strcpy(buf, "Syntax: vwear <type>\r\n");
    strcat(buf, "\r\nItem types:\r\n");
    for (j = 0, i = 0 ; strcmp(fields[i].loc, "\n") ; i++) {
      sprintf(buf, "%s%-15s%s", buf, fields[i].loc,
              (!(++j % 5) ? "\r\n" : ""));
    }
    if ((j % 5))
      strcat(buf, "\r\n");
    send_to_char(ch, buf);
    return;
  }

  for (i = 0 ; *(fields[i].loc) != '\n' ; i++) {
    if (!strncmp(arg, fields[i].loc, strlen(arg)))
      break;
  }

  if (!strcmp(fields[i].loc, "\n")) {
    send_to_char(ch, "That is not an option.\r\n");
    return;
  } else
    ((*fields[i].pointer)(fields[i].pos, ch));
}


ACMD(do_gocoord)      
{
  int room, x, y; 
  int maxX, maxY;
  maxX = maxY = 1999;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  half_chop(argument, arg, buf);

  if (!*arg || !*buf) {
    send_to_char(ch,"Syntax: gocoord <X> <Y>\r\n");
    return;            
  }

  if (!isdigit(*arg) || !isdigit(*buf)) {
    send_to_char(ch, "Only digits are coordinates!\r\n\rSyntax: gocoord <X> <Y>\r\n");
    return;
  }

  if (atoi(arg) >= maxX || atoi(buf) >= maxY) {
    send_to_char(ch, "Max coordinates are 1999x1999\r\n");
    return;
  }

  x = atoi(arg);
  y = atoi(buf);


  room = MAPZONE + (y * MAPY) + x;

  if (real_room(room) < 0) {
    send_to_char(ch, "Coordinates X:%d Y:%d does not exist!\r\n", x, y);
    return;     
  }

  char_from_room(ch);
  char_to_room(ch, real_room(room));
  look_at_room(ch, 0);
}


ACMD(do_unbind)
{
  char obj_name[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  char buf[MSL];

  one_argument(argument, obj_name);

  if (!*obj_name) {
    send_to_char(ch, "Usage: unbind <obj_name>\r\nYou must be carrying this object to unbind it.\r\n");
    return;
  }
  
  if (!(obj = get_obj_in_list_vis(ch, obj_name, NULL, ch->carrying))) {
    send_to_char(ch, "You are not carrying a %s.\r\n", obj_name);
    return;
  }
  
  if (GET_OBJ_BOUND_ID(obj) == NOBODY) {
    send_to_char(ch, "&CThe &R%s &Cis not bound to anybody.&n\r\n", obj->short_description);
    return;
  }

  if (get_name_by_id(GET_OBJ_BOUND_ID(obj)) == NULL) {
    send_to_char(ch, "The person the object was bound to has deleted, this item has been unbound!\r\n");
    GET_OBJ_BOUND_ID(obj) = NOBODY;
    return;
  }

  sprintf(obj_name, "%s", obj->short_description);
  
  send_to_char(ch, "&Y%s&C was bound to &Y%s&n\r\n", CAP(obj_name), get_name_by_id(GET_OBJ_BOUND_ID(obj)));
  send_to_char(ch, "&RThis item has now been unbound!\r\n");
  sprintf(buf, "%s", get_name_by_id(GET_OBJ_BOUND_ID(obj)));
  GET_OBJ_BOUND_ID(obj) = NOBODY;
  nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s used the unbind command on %s, was bound to %s", GET_NAME(ch), obj->short_description, buf);
}


void do_mbind(struct char_data *ch, struct char_data *vict)
{
  GET_MOB_BOUND_ID(vict) = GET_IDNUM(ch);  
}


ACMD(do_bind)
{

  char char_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  struct char_data *vict;

  two_arguments(argument, obj_name, char_name);

  if (!*obj_name) {
    send_to_char(ch, "Usage: bind <obj> <name>\r\nYou must have this obj in your inventory to bind it.\r\n");
    return;
  }

  if (!(obj = get_obj_in_list_vis(ch, obj_name, NULL, ch->carrying))) {
    send_to_char(ch, "You are not carrying a %s.\r\n", obj_name);
    return;
  }

  if (!*char_name) {
    send_to_char(ch, "To whom would you like to bind %s?\r\n", obj->short_description);
    return;
  }

  if (!(vict = get_char_vis(ch, char_name, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "No one by that name currently logged in.\r\n");
    return;
  }

  if (GET_OBJ_BOUND_ID(obj) != NOBODY) {
    if (GET_OBJ_BOUND_ID(obj) == GET_IDNUM(vict)) {
      send_to_char(ch, "&CIt is already bound to &Y%s&n\r\n", GET_NAME(vict));
      return;
    }
    else {
      send_to_char(ch, "&CIt is currently bound to &Y%s&C. Unbind it first!&n\r\n", get_name_by_id(GET_OBJ_BOUND_ID(obj)));
      return;
    }
  }
  GET_OBJ_BOUND_ID(obj) = GET_IDNUM(vict);
  send_to_char(ch, "&Y%s &Cis now bound to &Y%s&C.&n", obj->short_description, GET_NAME(vict));
  nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s used the bind command on %s, binding it to %s", GET_NAME(ch), obj->short_description, get_name_by_id(GET_OBJ_BOUND_ID(obj)));
//GET_NAME(ch), obj->short_description, get_name_by_id(GET_OBJ_BOUND_ID(obj)));
}


ACMD(do_barrier)
{
  struct room_affect *raff;
  char buf[MAX_INPUT_LENGTH];
  
  one_argument(argument, buf);

  if (!*buf) {
    send_to_char(ch, "Syntax: barrier <mob> | <player>\r\n");
    return;
  }
  
//  if (strcmp(buf, "mob")) {
  if (is_abbrev(buf, "mob")) {
  
    CREATE(raff, struct room_affect, 1);
     raff->type = RAFF_NPC_BARRIER;
     raff->timer = 10;
     raff->value = 0;
     raff->ch = NULL;
     raff->text = NULL;
     raff->room = IN_ROOM(ch); 
     raff->next = world[IN_ROOM(ch)].room_affs;
     world[IN_ROOM(ch)].room_affs = raff;
   }
   
//   else if (strcmp(buf, "player"))  {
  else if (is_abbrev(buf, "player")) {  
  
    CREATE(raff, struct room_affect, 1);
     raff->type = RAFF_PC_BARRIER;
     raff->timer = 10;
     raff->value = 0;
     raff->ch = NULL;
     raff->text = NULL;
     raff->room = IN_ROOM(ch);
     raff->next = world[IN_ROOM(ch)].room_affs;
     world[IN_ROOM(ch)].room_affs = raff;
   }  
   else
     send_to_char(ch, "Syntax: barrier <mob> | <player>\r\n");

}

ACMD(do_finddoor)
{
  int d, vnum = NOTHING, num = 0;
  size_t len, nlen;
  room_rnum i;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH] = {0};
  struct char_data *tmp_char;
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Format: finddoor <obj/vnum>\r\n");
  } else if (is_number(arg)) {
    vnum = atoi(arg);
    obj = &obj_proto[real_object(vnum)];
  } else {
    generic_find(arg,
         FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_WORLD | FIND_OBJ_EQUIP,
         ch, &tmp_char, &obj);
    if (!obj)
      send_to_char(ch, "What key do you want to find a door for?\r\n");
    else
      vnum = GET_OBJ_VNUM(obj);
  }
  if (vnum != NOTHING) {
    if (GET_OBJ_TYPE(obj) != ITEM_KEY)
      send_to_char(ch, "It seems that %s isn't a key.\r\n", GET_OBJ_SHORT(obj));
    else {
      len = snprintf(buf, sizeof(buf), "Doors unlocked by key [%d] %s are:\r\n",
                      vnum, GET_OBJ_SHORT(obj));
      for (i = 0; i <= top_of_world; i++) {
        for (d = 0; d < NUM_OF_DIRS; d++) {
          if (world[i].dir_option[d] && world[i].dir_option[d]->key &&
              world[i].dir_option[d]->key == vnum) {
            nlen = snprintf(buf + len, sizeof(buf) - len,
                            "[%3d] Room %d, %s (%s)\r\n",
                            ++num, world[i].number,
                            dirs[d], world[i].dir_option[d]->keyword);
            if (len + nlen >= sizeof(buf) || nlen < 0)
              break;
            len += nlen;
          }
        } /* for all directions */
      } /* for all rooms */
      if (num > 0)
        page_string(ch->desc, buf, 1);
      else
        send_to_char(ch, "No doors were found for key [%d] %s.\r\n",
                         vnum, GET_OBJ_SHORT(obj));
    }
  }
} 

ACMD(do_findkey)
{
  int dir, key;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  any_one_arg(argument, arg); /* Because "in" is a valid direction */

  if (!*arg) {
    send_to_char(ch, "Format: findkey <dir>\r\n");
  } else if ((dir = search_block(arg, dirs, FALSE)) >= 0) { 
 //   || (dir = search_block(arg, abbr_dirs, FALSE)) >= 0 ) {
    if (!EXIT(ch, dir)) {
      send_to_char(ch, "There's no exit in that direction!\r\n");
    } else if ((key = EXIT(ch, dir)->key) == NOTHING || key == 0) {
      send_to_char(ch, "There's no key for that exit.\r\n");
    } else {
      sprintf(buf, "obj %d", key);
      do_checkloadstatus(ch, buf, 0, 0);
    }
  } else {
    send_to_char(ch, "What direction is that?!?\r\n");
  }
} 


ACMD(do_viewall)
{
  struct descriptor_data *d;
  int found = 0;
  char buf[MSL];

  one_argument(argument, buf);

  if (!argument) {
    send_to_char(ch, "Usage: viewall players\r\n");
    return;
  }

  send_to_char(ch, "&MName             Level  Room   Position   Hps%%   Mana%%   Moves%%\r\n&n");
  send_to_char(ch, "&m---------------- ------ ------ ---------- ------ ------- ---------\r\n&n");

  for (d = descriptor_list; d; d = d->next) {
    if (!IS_PLAYING(d))
      continue;

    if (GET_LEVEL(d->character) <= GET_LEVEL(ch)) {
      send_to_char(ch, "&G%-16s &G%-6d &Y%-9d &R%-11s &C%-6ld &C%-6ld &C%-6ld\r\n&n", GET_NAME(d->character), 
GET_LEVEL(d->character), 
            GET_ROOM_VNUM(IN_ROOM(d->character)), position_types[(int)GET_POS(d->character)], 
            (GET_HIT(d->character)*100)/GET_MAX_HIT(d->character),
            (GET_MANA(d->character)*100)/GET_MAX_MANA(d->character),
            (GET_MOVE(d->character)*100)/GET_MAX_MOVE(d->character));
      found++;
    }
   }

   if (!found)
     send_to_char(ch, "&WNo one is online!&n\r\n");
}


ACMD(do_xprate)
{
  int rate = 0;
  int oldrate = 0;
  char buf[MIL];
  struct descriptor_data *pt;

  oldrate = exp_multiplier;
 
  one_argument(argument, buf);
  
  rate = atoi(buf);

  if (!*buf) {
    send_to_char(ch, "You must enter a valid positive whole number between 1 and 5 to change the experience multiplier\r\n");
    return;
  }

if ((GET_IDNUM(ch) == 97 || GET_IDNUM(ch) == 1) && (rate > 5)) {
  send_to_char(ch, "Being all powerful, you can set the rate past the threshold of 5..\r\n"
                   "You have now set the multiplier to %d!\r\n", rate);
  exp_multiplier = rate;

   for (pt = descriptor_list; pt; pt = pt->next)
     if (IS_PLAYING(pt) && pt->character)                  
      send_to_char(pt->character, "&Y%s HAS CHANGED THE EXPERIENCE MULTIPLIER TO %d!!\r\n-Enjoy\r\n", GET_NAME(ch), exp_multiplier);

  nmudlog(SYS_LOG, LVL_GOD, TRUE, "&C%s&R has changed the XP RATE to &Y%d&n", GET_NAME(ch), exp_multiplier);
} else {

  if (rate <=0) {
    send_to_char(ch, "You canot set the experience multiplier at or below 0!\r\n"
                     "Defaulting to multiplier back to the original rate of %d...Complete!\r\n", oldrate);
    exp_multiplier = oldrate;
  } else if (rate > 5) {
    send_to_char(ch, "You cannot set the experience multiplier past 5!\r\n"
                     "Setting multiplier back to original rate of %d...Complete!\r\n", oldrate);
    exp_multiplier = oldrate;
  } else {
    send_to_char(ch,  "Setting multiplier to %d...Complete!\r\n", atoi(buf));
    rate = atoi(buf);
    exp_multiplier = rate;
 
   for (pt = descriptor_list; pt; pt = pt->next)
     if (IS_PLAYING(pt) && pt->character)
      send_to_char(pt->character, "&Y%s HAS CHANGED THE EXPERIENCE MULTIPLIER TO %d!!\r\n-Enjoy\r\n", GET_NAME(ch), exp_multiplier);
  
   nmudlog(SYS_LOG, LVL_GOD, TRUE, "%s changed the XP MULTIPLIER to %d", GET_NAME(ch), exp_multiplier);
  }
}
}



ACMD(do_ptell)
{
  struct char_data *target;
  struct descriptor_data *d;
  int level = LVL_IMM;
  char arg[MSL], buf[MSL], buf2[MSL];

  half_chop(argument, arg, buf2);
  
  if (!*arg || !*buf2) {
    send_to_char(ch, "Syntax: ptell <player> <message>\r\n");
    return;
  }

  if (target == ch) {
   send_to_char(ch, "Why would you respond to yourself.. tard!\r\n");
   return;
  }

  if (!(target = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "%s\r\n", CONFIG_NOPERSON);
    return;
  }

  if (level >= LVL_IMM) {
    sprintf(buf1, "&Y[PRAY]&W %s's response:: &C%s&n\r\n", GET_NAME(ch), buf2);
    send_to_char(target, buf1);
    sprintf(buf, "&Y[PRAY]&W %s pray responds:: &M%s &W: &C%s\r\n", GET_NAME(ch), GET_NAME(target), buf2);
  }

  for (d = descriptor_list; d; d = d->next) {
    if ((!d->connected) && (GET_LEVEL(d->character) >= level) && (!PRF_FLAGGED(d->character, PRF2_NOPRAY)) &&
       (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING)) && (d->character != ch || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
        if (CAN_SEE(d->character, ch))
          send_to_char(d->character, buf1);
    }
  }
}




ACMD(do_snatch)
{

  struct char_data *victim;
  struct obj_data *obj;
  char name[MAX_INPUT_LENGTH];
  char victname[MAX_INPUT_LENGTH];

  two_arguments(argument, name, victname);

  if (!*name || !*victname)
    send_to_char(ch, "Syntax: chown (object) (character).\r\n");
  else if (!(victim = get_char_vis(ch, victname, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "No one by that name here.\r\n");
  else if (victim == ch)
    send_to_char(ch, "Are you sure you're feeling ok?\r\n");
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch) && (GET_IDNUM(ch) == 1 || GET_IDNUM(ch) == 97))
    send_to_char(ch, "That's really not such a good idea.\r\n");
  else {
    obj = get_object_on_character(ch, victim, name);

    if (!obj) {
      send_to_char(ch, "%s does not appear to have the %s.\r\n", GET_NAME(victim), name);
      return;
    }

    act("&C$n makes a magical gesture and $p flies from $N to $m.&n", FALSE, ch, obj, victim, TO_NOTVICT);
    act("&C$n makes a magical gesture and $p flies away from you to $m.&n", FALSE, ch, obj, victim, TO_VICT);
    act("&CYou make a magical gesture and $p flies away from $N to you.&n", FALSE, ch, obj, victim, TO_CHAR);
    nmudlog(TAKE_LOG, LVL_IMPL, TRUE, "%s took %s from %s", ch, obj, victim);


    if (obj->carried_by)
      obj_from_char(obj);
    else if (obj->in_obj)
      obj_from_obj(obj);
    else if (obj->worn_by)
       unequip_char(victim, obj->worn_on);

    obj_to_char(obj, ch);
    save_char(ch);
    save_char(victim);
  } 


}


ACMD(do_zlock)
{
  struct descriptor_data *d;
  zone_vnum znvnum;
  zone_rnum zn;
  char      arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  int       counter = 0;
  bool      fail = FALSE;

  two_arguments(argument, arg, arg2);

  if (!*arg) {
    send_to_char(ch, "Usage: %szlock <zone number>%s\r\n", QYEL, QNRM);
    send_to_char(ch, "%s       zlock list%s\r\n\r\n", QYEL, QNRM);
    send_to_char(ch, "Locks a zone so that building or editing is not possible.\r\n");
    send_to_char(ch, "The 'list' shows all currently locked zones.\r\n");
    send_to_char(ch, "'zlock all' will lock every zone with the GRID flag set.\r\n");
    send_to_char(ch, "'zlock all all' will lock every zone in the MUD.\r\n");
    return;
  }
  if (is_abbrev(arg, "all")) {
    if (GET_LEVEL(ch) < LVL_GRGOD) {
      send_to_char(ch, "You do not have sufficient access to lock all zones.\r\n");
      return;
    }
    if (!*arg2) {
      for (zn = 0; zn <= top_of_zone_table; zn++) {
        if (!ZONE_FLAGGED(zn, ZONE_NOBUILD) && ZONE_FLAGGED(zn, ZONE_GRID)) {
          counter++;
          SET_BIT_AR(ZONE_FLAGS(zn), ZONE_NOBUILD);
          if (save_zone(zn)) {
        
            for (d = descriptor_list ; d ; d = d->next) {
             if (!d->connected && d->character && d->character != ch) {
               if (GET_LEVEL(d->character) >= LVL_GOD)
                 send_to_char(ch, "&R%s has locked zone %d", GET_NAME(ch), zone_table[zn].number);
             }
           }
            nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s has locked zone %d", GET_NAME(ch), zone_table[zn].number);
            log("(GC) %s has locked zone %d", GET_NAME(ch), zone_table[zn].number);
          } else {
            fail = TRUE;
          }
        }
      }
    } else if (is_abbrev(arg2, "all")) {
      for (zn = 0; zn <= top_of_zone_table; zn++) {
        if (!ZONE_FLAGGED(zn, ZONE_NOBUILD)) {
          counter++;
          SET_BIT_AR(ZONE_FLAGS(zn), ZONE_NOBUILD);
          if (save_zone(zn)) {


            for (d = descriptor_list ; d ; d = d->next) {
             if (!d->connected && d->character && d->character != ch) {
               if (GET_LEVEL(d->character) >= LVL_GOD)
                 send_to_char(ch, "&R%s has locked zone %d", GET_NAME(ch), zone_table[zn].number);
             }
           }
            nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s has locked zone %d", GET_NAME(ch), zone_table[zn].number);
            log("(GC) %s has locked zone %d", GET_NAME(ch), zone_table[zn].number);
          } else {
            fail = TRUE;
          }
        }
      }
    }
    if (counter == 0) {
      send_to_char(ch, "There are no unlocked zones to lock!\r\n");
      return;
    }
    if (fail) {
      send_to_char(ch, "Unable to save zone changes.  Check syslog!\r\n");
      return;
    }
    send_to_char(ch, "%d zones have now been locked.\r\n", counter);
    nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s has locked &RALL&C zones", GET_NAME(ch));
    return;
  }
  if (is_abbrev(arg, "list")) {
    /* Show all locked zones */
    for (zn = 0; zn <= top_of_zone_table; zn++) {
      if (ZONE_FLAGGED(zn, ZONE_NOBUILD)) {
        if (!counter) send_to_char(ch, "Locked Zones\r\n");

        send_to_char(ch, "[%s%3d%s] %s%-*s %s%-1s%s\r\n",
          QGRN, zone_table[zn].number, QNRM, QCYN, count_color_chars(zone_table[zn].name)+30, zone_table[zn].name,
          QYEL, zone_table[zn].builders ? zone_table[zn].builders : "None.", QNRM);
        counter++;
      }
    }
    if (counter == 0) {
      send_to_char(ch, "There are currently no locked zones!\r\n");
    }
    return;
  }
  else if ((znvnum = atoi(arg)) == 0) {
    send_to_char(ch, "Usage: %szlock <zone number>%s\r\n", QYEL, QNRM);
    return;
  }

  if ((zn = real_zone(znvnum)) == NOWHERE) {
    send_to_char(ch, "That zone does not exist!\r\n");
    return;
  }

  /* Check the builder list */
  if (GET_LEVEL(ch) < LVL_GRGOD && !is_name(GET_NAME(ch), zone_table[zn].builders) && GET_OLC_ZONE(ch) != znvnum) {
    send_to_char(ch, "You do not have sufficient access to lock that zone!\r\n");
    return;
  }

  /* If we get here, player has typed 'zlock <num>' */
  if (ZONE_FLAGGED(zn, ZONE_NOBUILD)) {
    send_to_char(ch, "Zone %d is already locked!\r\n", znvnum);
    return;
  }
  SET_BIT_AR(ZONE_FLAGS(zn), ZONE_NOBUILD);
  if (save_zone(zn)) {
    nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s has locked zone %d", GET_NAME(ch), znvnum);
  }
  else
  {
    send_to_char(ch, "Unable to save zone changes.  Check syslog!\r\n");
  }
}

ACMD(do_zunlock)
{
  zone_vnum znvnum;
  zone_rnum zn;
  char      arg[MAX_INPUT_LENGTH];
  int       counter = 0;
  bool      fail = FALSE;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Usage: %szunlock <zone number>%s\r\n", QYEL, QNRM);
    send_to_char(ch, "%s       zunlock list%s\r\n\r\n", QYEL, QNRM);
    send_to_char(ch, "Unlocks a 'locked' zone to allow building or editing.\r\n");
    send_to_char(ch, "The 'list' shows all currently unlocked zones.\r\n");
    send_to_char(ch, "'zunlock all' will unlock every zone in the MUD.\r\n");
    return;
  }
  if (is_abbrev(arg, "all")) {
    if (GET_LEVEL(ch) < LVL_GRGOD) {
      send_to_char(ch, "You do not have sufficient access to lock zones.\r\n");
      return;
    }
    for (zn = 0; zn <= top_of_zone_table; zn++) {
      if (ZONE_FLAGGED(zn, ZONE_NOBUILD)) {
        counter++;
        REMOVE_BIT_AR(ZONE_FLAGS(zn), ZONE_NOBUILD);
        if (save_zone(zn)) {
          log("(GC) %s has unlocked zone %d", GET_NAME(ch), zone_table[zn].number);
        } else {
          fail = TRUE;
        }
      }
    }
    if (counter == 0) {
      send_to_char(ch, "There are no locked zones to unlock!\r\n");
      return;
    }
    if (fail) {
      send_to_char(ch, "Unable to save zone changes.  Check syslog!\r\n");
      return;
    }
    send_to_char(ch, "%d zones have now been unlocked.\r\n", counter);
    nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s has unlocked &RALL&C zones", GET_NAME(ch));
    return;
  }
  if (is_abbrev(arg, "list")) {
    /* Show all unlocked zones */
    for (zn = 0; zn <= top_of_zone_table; zn++) {
      if (!ZONE_FLAGGED(zn, ZONE_NOBUILD)) {
        if (!counter) send_to_char(ch, "Unlocked Zones\r\n");

        send_to_char(ch, "[%s%3d%s] %s%-*s %s%-1s%s\r\n",
          QGRN, zone_table[zn].number, QNRM, QCYN, count_color_chars(zone_table[zn].name)+30, zone_table[zn].name,
          QYEL, zone_table[zn].builders ? zone_table[zn].builders : "None.", QNRM);
        counter++;
      }
    }
    if (counter == 0) {
      send_to_char(ch, "There are currently no unlocked zones!\r\n");
    }
    return;
  }
  else if ((znvnum = atoi(arg)) == 0) {
    send_to_char(ch, "Usage: %szunlock <zone number>%s\r\n", QYEL, QNRM);
    return;
  }

  if ((zn = real_zone(znvnum)) == NOWHERE) {
    send_to_char(ch, "That zone does not exist!\r\n");
    return;
  }

  /* Check the builder list */
  if (GET_LEVEL(ch) < LVL_GRGOD && !is_name(GET_NAME(ch), zone_table[zn].builders) && GET_OLC_ZONE(ch) != znvnum) {
    send_to_char(ch, "You do not have sufficient access to unlock that zone!\r\n");
    return;
  }

  /* If we get here, player has typed 'zunlock <num>' */
  if (!ZONE_FLAGGED(zn, ZONE_NOBUILD)) {
    send_to_char(ch, "Zone %d is already unlocked!\r\n", znvnum);
    return;
  }
  REMOVE_BIT_AR(ZONE_FLAGS(zn), ZONE_NOBUILD);
  if (save_zone(zn)) {
    nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s has unlocked zone %d", GET_NAME(ch), znvnum);
  }
  else
  {
    send_to_char(ch, "Unable to save zone changes.  Check syslog!\r\n");
  }
}


ACMD(do_vfile) // virtual online file reader
{
  
  char syscom[120], *filename, buf[MIL];
  FILE *fp;

  switch (subcmd) {
  	case SCMD_V_BUGS:
                filename = BUG_FILE;
		break;
 	case SCMD_V_IDEAS:
		filename = IDEA_FILE;
		break;
	case SCMD_V_TYPOS:
		filename = TYPO_FILE;
		break;
	default:
		send_to_char(ch, "Invalid command!");
		nmudlog(SYS_LOG, LVL_GOD, TRUE, "V-File reached default case");
		return;
  }

      if (!(fp = fopen(filename, "r"))) {
         send_to_char(ch, "Error opening file for read\r\n");
	 return;
      }
  
  sprintf(syscom, "tail -100 %s", filename);

  sprintf(buf, "&CContents of file: &Y%s\r\n&n", filename);
  send_to_char(ch, buf);
  send_to_char(ch, "&W-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-&n\r\n");

  while (fgets(syscom, 120, fp) != NULL) {
     sprintf(buf, "&G%s&n\r", syscom);
     send_to_char(ch, buf);
  }
  fclose(fp);
}


ACMD(do_restring) {

  struct obj_data *obj;
  struct char_data *vict;
  int found=0;
  char type[MIL], name[MIL], field[MIL], input[MIL];

  argument = two_arguments(argument, type, name);
  half_chop(argument, field, input);

  if (!*type || !*name || !*field || !*input) {
    send_to_char(ch, "Syntax: restring <obj|mob> <name> <field> <input>\r\n");
    return;
  }

  if (is_abbrev(type, "object")) {
    if ((obj = get_obj_in_equip_vis(ch, name, NULL, ch->equipment))) {
      found = 1;
    } else if
       ((obj = get_obj_in_list_vis(ch, name, NULL, ch->carrying))) {
        found = 1;
    } else if
       ((obj = get_obj_in_list_vis(ch, name, NULL, world[IN_ROOM(ch)].contents))) {
        found = 1;
    } else if
       ((obj = get_obj_vis(ch, name, NULL))) {
        found = 1;
    }

  if (!found) {
    send_to_char(ch, "Couldn't find that object,\r\n");
    return;
  }

  if (is_abbrev(field, "short")) {
    send_to_char(ch, "%s's short description restrung to %s\r\n", obj->short_description, input);
    restring_object(obj, RS_OBJ_SDESC, input);
  } else if
      (is_abbrev(field, "long")) {
      send_to_char(ch, "%s's description restrung to, %s\r\n", obj->short_description, input);
      restring_object(obj, RS_OBJ_DESC, input);
  } else if
     (is_abbrev(field, "action")) {
      send_to_char(ch, "%s's action description restrung to, %s\r\n", obj->short_description, input);
      restring_object(obj, RS_OBJ_ADESC, input);
  } else if
      (is_abbrev(field, "name")) {
      send_to_char(ch, "%s's name list restrung to, %s\r\n", obj->short_description, input);
      restring_object(obj, RS_OBJ_NAME, input);
  } else { // INVALID
      send_to_char(ch, "That is not a valid object restring field:\n");
      send_to_char(ch, "restring obj <object name> <name|short|long|action> <input>\r\n");
      return;
    }
  }
  else if(is_abbrev(type, "mobile")) {
 
    vict = get_char_vis(ch, name, NULL, FIND_CHAR_ROOM);

    if (!vict) {
      send_to_char(ch, "Couldn't find that mobile. Sorry.\r\n");
      return;
    }

    if (!IS_NPC(vict)) {
      send_to_char(ch, "Cannot restring players.\r\n");
      return;
    }

    if(is_abbrev(field, "short")) {
      send_to_char(ch, "%s's short description restrung to, %s\r\n", GET_SDESC(vict), input);
      restring_mobile(vict, RS_MOB_SDESC, input);
    }
    else if(is_abbrev(field, "long")) {
      send_to_char(ch, "%s's long description restrung to, %s\r\n", GET_SDESC(vict), input);
      restring_mobile(vict, RS_MOB_LDESC, input);
    }
    else if(is_abbrev(field, "desc")) {
      send_to_char(ch, "%s's description restrung to, %s\r\n", GET_SDESC(vict), input);
      restring_mobile(vict, RS_MOB_DESC, input);
    }
    else if(is_abbrev(field, "name")) {
      send_to_char(ch, "%s's name restrung to, %s\r\n", GET_SDESC(vict), input);
      restring_mobile(vict, RS_MOB_NAME, input);
    } else { // Not a valid restring field
      send_to_char(ch, "That is not a valid mobile restring field:\n");
      send_to_char(ch, "restring mob <object name> <name|desc|short|long> <input>\r\n");
      return;
    }
  } else { //Not a valid type
    send_to_char(ch, "That is not a valid type:\n");
    send_to_char(ch, "restring <obj|mob> <object name> <field> <input>\r\n");
    return;
  }
} 
