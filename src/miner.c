/************************************************************************
 * OasisOLC - Objects / oedit.c					v2.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/

#include "screen.h"
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "boards.h"
#include "constants.h"
#include "shop.h"
#include "genolc.h"
#include "genobj.h"
#include "genzon.h"
#include "oasis.h"
#include "improved-edit.h"
#include "dg_olc.h"
#include "screen.h"
#include "handler.h"
#include "logger.h"

void obj_to_char(struct obj_data *object, struct char_data *ch);
void obj_to_room(struct obj_data *object, room_rnum room);
ACMD(do_smithery);
void show_smithlist(struct char_data *ch);
void make_smith_item(char *type, char *comp1, char *comp2, struct char_data *ch);
void take_ore(struct char_data *ch, int cost, int ore1, int ore2);
bool has_ore(struct char_data *ch, int cost, int ore);
void get_ore_mine(struct char_data *ch);
void get_gem_mine(struct char_data *ch);

SPECIAL(armorer);
SPECIAL(smithy);

const char *ore_prefix[] = {
  "Adamant",
  "Bro",
  "Copi",
  "Gild",
  "Ox",
  "Mith",
  "Silv",
  "Titan"
};

const char *ore_suffix[] = {
  "antite",
  "rium",
  "rum",
  "olden",
  "erric",
  "ril",
  "um",
  "ium"
};


ACMD(do_smithery)
{
   char type[MAX_INPUT_LENGTH],comp1[MAX_INPUT_LENGTH],comp2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];

   half_chop(argument, type, buf);
   two_arguments(buf, comp1, comp2);

   
   if(!*argument)
   {
     show_smithlist(ch);
     send_to_char(ch, "&RUsage: smith <type> <comp1> <comp2>&n\r\n");
     return; 
   }

   if(!*type || !*comp1 || !*comp2)
   {
     send_to_char(ch, "&RUsage: smith <type> <comp1> <comp2>&n\r\n");
     return;
   }

   if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_SMITH))
        make_smith_item(type,comp1, comp2, ch);
   else
     send_to_char(ch, "\r\n&RYou must be at a blacksmith to make equipment.&n\r\n");

 return;
}

void show_smithlist(struct char_data *ch)
{

  send_to_char(ch, "&CItems for smithing &G- &WOre Cost (per component)&n\r\n"
                   "&C-------------------&G-&W-------------------------&n\r\n"
                   "&CDagger            &G -      &W50     \r\n"
                   "&CSword             &G -      &W70     \r\n"
                   "&CMace              &G -      &W70     \r\n"
                   "&CBow               &G -      &W60     \r\n"
                   "&CArmor             &G -     &W100     \r\n"
                   "&CBoots             &G -      &W60     \r\n"
                   "&CGloves            &G -      &W40     \r\n"
                   "&CNecklace          &G -      &W35     \r\n"
                   "&CCloak             &G -      &W45     \r\n"
                   "&CRing              &G -      &W25     \r\n"
                   "&CBracelet          &G -      &W30     \r\n"
                   "&CGlowing Orb       &G -      &W50     \r\n"
                   "&CPants             &G -      &W70     \r\n"
                   "&CSleeves           &G -      &W70     \r\n"
                   "&CShield            &G -      &W90     \r\n"
                   "&CBelt              &G -      &W30     \r\n"
                   "&CSceptre           &G -      &W80     \r\n"
                   "&CHelmet            &G -      &W50     \r\n");

  return;
}

const char *smith_items[] = {
  "2700",
  "2701",
  "2702",
  "2703",
  "2704",
  "2705",
  "2706",
  "2707",
  "2708",
  "2709",
  "2710",
  "2711",
  "2712",
  "2713",
  "2714",
  "2715", 
  "2716",
  "2717"
};

const char *ores[] = {
  "adamantite",
  "bronze",
  "copper",
  "gold",
  "iron",
  "mithril",
  "silver",
  "titanium"
};


void make_smith_item(char *type, char *comp1, char *comp2, struct char_data *ch)
{
   struct obj_data *obj;
   int obj_num=0, ore1=0, ore2=0, cost=0;
   char shortdesc[MAX_INPUT_LENGTH], longdesc[MAX_INPUT_LENGTH], newname[MAX_INPUT_LENGTH];
   obj_rnum r_num;


   if (is_abbrev(type, "dagger")) {
      obj_num=0;     
      cost = 50;
   } else if (is_abbrev(type, "sword")) {
      obj_num=1;
      cost = 75;
   } else if (is_abbrev(type, "mace")) {
      obj_num=2;
      cost = 70;
   } else if (is_abbrev(type, "bow")) {
      obj_num=3;
      cost = 60;
   } else if (is_abbrev(type, "armor")) {
      obj_num=4;
      cost = 100;
   } else if (is_abbrev(type, "boots")) {
      obj_num=5;
      cost = 60;
   } else if (is_abbrev(type, "glovees")) {
      obj_num=6;
      cost = 40;
   } else if (is_abbrev(type, "necklace")) {
      obj_num=7;
      cost = 35;
   } else if (is_abbrev(type, "cloak")) {
      obj_num=8;
      cost = 45;
   } else if (is_abbrev(type, "ring")) {
      obj_num=9;
      cost = 25;
   } else if (is_abbrev(type, "bracelet")) {
      obj_num=10;
      cost = 30;
   } else if (is_abbrev(type, "orb")) {
      obj_num=11;
      cost = 50;
   } else if (is_abbrev(type, "pants")) {
      obj_num=12;
      cost = 70;
   } else if (is_abbrev(type, "sleeves")) {
      obj_num=13;
      cost = 70;
   }  else if (is_abbrev(type, "shield")) {
      obj_num=14;
      cost = 90;
   } else if (is_abbrev(type, "belt")) {
      obj_num=15;
      cost = 30;
   } else if (is_abbrev(type, "sceptre")) {
      obj_num=16;
      cost = 80;
   } else if (is_abbrev(type, "helmet")) {
      obj_num=17;
      cost = 50;
   } else {
      log("Error in make_smith_item function - line 150");
      return;
   }
   
   if (is_abbrev(comp1, "adamantite")) {
      ore1=0;
   } else if (is_abbrev(comp1, "bronze")) {
      ore1=1;
   } else if (is_abbrev(comp1, "copper")) {
      ore1=2;
   } else if (is_abbrev(comp1, "gold")) {
      ore1=3;
   } else if (is_abbrev(comp1, "iron")) {
      ore1=4;
   } else if (is_abbrev(comp1, "mithril")) {
      ore1=5;
   } else if (is_abbrev(comp1, "silver")) {
      ore1=6;
   } else if (is_abbrev(comp1, "titanium")) {
      ore1=7;
   } else {
      log("Error in make_smith_item function - line 170");
      nmudlog(MISC_LOG, LVL_GOD, TRUE, "MINER: Error in make_smith_item_function - line 170");
      return;
   }


   if (is_abbrev(comp2, "adamantite")) {
      ore2=0;
   } else if (is_abbrev(comp2, "bronze")) {
      ore2=1;
   } else if (is_abbrev(comp2, "copper")) {
      ore2=2;
   } else if (is_abbrev(comp2, "gold")) {
      ore2=3;
   } else if (is_abbrev(comp2, "iron")) {
      ore2=4;
   } else if (is_abbrev(comp2, "mithril")) {
      ore2=5;
   } else if (is_abbrev(comp2, "silver")) {
      ore2=6;
   } else if (is_abbrev(comp2, "titanium")) {
      ore2=7;
   } else {
      log("Error in make_smith_item function - line 170");
      return;
   }


    if ((r_num = real_object(((long int) atoi(smith_items[obj_num])))) == NOTHING) {
      send_to_char(ch, "&RThere is no object with that name.\r\n");
      return;
    }

    if (ores[ore1] == ores[ore2]) {
      send_to_char(ch, "&RYou must use 2 different types of ore to make an item.\r\n");
      return;  
    }
 
    if(!has_ore(ch, cost, ore1))
    {
      send_to_char(ch, "&RYou do not have enough %s ore to make this item.\r\n", ores[ore1]);
      return;
    }
    else if(!has_ore(ch, cost, ore1))
    {
      send_to_char(ch, "&RYou do not have enough %s ore to make this item.\r\n", ores[ore2]);
      return;
    }
    else
    {
      
      take_ore(ch, cost, ore1, ore2);
      obj = read_object(r_num, REAL);
   
      if (GET_PROF(ch, PROF_SMITHING) > 100)
        SET_SKILL(ch, PROF_SMITHING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(10,25), 5000));

      snprintf(newname, sizeof(newname), "%s %s %s", obj->name, ores[ore1], ores[ore2]);
      snprintf(shortdesc, sizeof(shortdesc), "%s %s%s", obj->short_description, ore_prefix[ore1], ore_suffix[ore2]);
      snprintf(longdesc, sizeof(longdesc), "%s %s%s is lying here.", obj->description, ore_prefix[ore1], ore_suffix[ore2]);

      obj->name =  strdup(newname);
      obj->short_description =  strdup(shortdesc);
      obj->description =  strdup(longdesc);
      GET_OBJ_LEVEL(obj) = GET_LEVEL(ch);

      obj_to_char(obj, ch);

      act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
      act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
      act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    }

    

return;
}

bool has_ore(struct char_data *ch, int cost, int ore)
{

  switch(ore)
  {
    case 0:
     if(GET_ADAMANTITE_ORE(ch) < cost)
       return(FALSE);
     break;
    case 1:
     if(GET_BRONZE_ORE(ch) < cost)
       return(FALSE);
     break;
    case 2:
     if(GET_COPPER_ORE(ch) < cost)
       return(FALSE);
     break;
    case 3:
     if(GET_GOLD_ORE(ch) < cost)
       return(FALSE);
     break;
    case 4:
     if(GET_IRON_ORE(ch) < cost)
       return(FALSE);
     break;
    case 5:
     if(GET_MITHRIL_ORE(ch) < cost)
       return(FALSE);
     break;
    case 6:
     if(GET_SILVER_ORE(ch) < cost)
       return(FALSE);
     break;
    case 7:
     if(GET_TITANIUM_ORE(ch) < cost)
       return(FALSE);
     break;
    default:
       return(FALSE);
     break;
  }

return (TRUE);
}

void take_ore(struct char_data *ch, int cost, int ore1, int ore2)
{

  switch(ore1)
  {
    case 0:
     if(GET_ADAMANTITE_ORE(ch) > cost)
       GET_ADAMANTITE_ORE(ch) -= cost;
     break;
    case 1:
     if(GET_BRONZE_ORE(ch) > cost)
       GET_BRONZE_ORE(ch) -= cost;
     break;
    case 2:
     if(GET_COPPER_ORE(ch) > cost)
       GET_COPPER_ORE(ch) -= cost;
     break;
    case 3:
     if(GET_GOLD_ORE(ch) > cost)
       GET_GOLD_ORE(ch) -= cost;
     break;
    case 4:
     if(GET_IRON_ORE(ch) > cost)
       GET_IRON_ORE(ch) -= cost;
     break;
    case 5:
     if(GET_MITHRIL_ORE(ch) > cost)
       GET_MITHRIL_ORE(ch) -= cost;
     break;
    case 6:
     if(GET_SILVER_ORE(ch) > cost)
       GET_SILVER_ORE(ch) -= cost;
     break;
    case 7:
     if(GET_TITANIUM_ORE(ch) > cost)
       GET_TITANIUM_ORE(ch) -= cost;
     break;
    default:
     break;
  }

  switch(ore2)
  {
    case 0:
     if(GET_ADAMANTITE_ORE(ch) > cost)
       GET_ADAMANTITE_ORE(ch) -= cost;
     break;
    case 1:
     if(GET_BRONZE_ORE(ch) > cost)
       GET_BRONZE_ORE(ch) -= cost;
     break;
    case 2:
     if(GET_COPPER_ORE(ch) > cost)
       GET_COPPER_ORE(ch) -= cost;
     break;
    case 3:
     if(GET_GOLD_ORE(ch) > cost)
       GET_GOLD_ORE(ch) -= cost;
     break;
    case 4:
     if(GET_IRON_ORE(ch) > cost)
       GET_IRON_ORE(ch) -= cost;
     break;
    case 5:
     if(GET_MITHRIL_ORE(ch) > cost)
       GET_MITHRIL_ORE(ch) -= cost;
     break;
    case 6:
     if(GET_SILVER_ORE(ch) > cost)
       GET_SILVER_ORE(ch) -= cost;
     break;
    case 7:
     if(GET_TITANIUM_ORE(ch) > cost)
       GET_TITANIUM_ORE(ch) -= cost;
     break;
    default:
     break;
  }

return;
}

ACMD(do_mine)
{
   char arg[MAX_INPUT_LENGTH]; 

   one_argument(argument, arg);

   if (!*argument)
   {
     send_to_char(ch, "What do you want to mine? (ore/gems)\r\n");
     return;
   }

   if (PLR_FLAGGED(ch, PLR_OREMINE) || PLR_FLAGGED(ch, PLR_GEMMINE))
   {
     send_to_char(ch, "You are already mining.\r\n");
     return;
   }

   if(!(ROOM_FLAGGED(IN_ROOM(ch), ROOM_MINE)))
   {
     send_to_char(ch, "You would have much more luck doing this IN a mine.\r\n");
     return;
   }

   if(!(str_cmp(arg, "ore")))
   {
     send_to_char(ch, "You begin to mine for ore.\r\n");
     SET_BIT_AR(PLR_FLAGS(ch), PLR_OREMINE); 
     GET_MINE_TIME(ch)  = rand_number(5,10); 
   }
   else if (!(str_cmp(arg, "gems")) || !(str_cmp(arg, "gem")))
   {
     send_to_char(ch, "You begin to mine for gems.\r\n");
     SET_BIT_AR(PLR_FLAGS(ch), PLR_GEMMINE);
     GET_MINE_TIME(ch)  = rand_number(5,10);
   }
   else
     send_to_char(ch, "I don't know what that is.\r\n");
 
return;   
}



void check_mining() {

  struct descriptor_data *d;
  int dig;

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) continue;
    if (IS_NPC(d->character)) continue;

    if (PLR_FLAGGED(d->character, PLR_OREMINE) && !ROOM_FLAGGED(d->character->in_room, ROOM_MINE))
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_OREMINE);

    if (PLR_FLAGGED(d->character, PLR_GEMMINE) && !ROOM_FLAGGED(d->character->in_room, ROOM_MINE))
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_GEMMINE);

    if (PLR_FLAGGED(d->character, PLR_OREMINE) || PLR_FLAGGED(d->character, PLR_GEMMINE)) {

      dig = rand_number(1, 10);

      if (dig >= 7 && dig <= 8) {
        send_to_char(d->character, "&GTime goes by and you keep diggin' away.... &n\r\n");
      } else if (dig >= 6) {
       send_to_char(d->character, "&cYou see a slight glimmer in the dirt.&n\r\n");
       GET_MINE_TIME(d->character) -= 1;
      } else if (dig >= 4) {
       send_to_char(d->character, "&CYou feel like you are getting very close to somethnig.&n\r\n");
       GET_MINE_TIME(d->character) -= 2;
      } else if (dig >= 2) {
       send_to_char(d->character, "&CYou hear something clink as you strike the ground.&n\r\n");
       GET_MINE_TIME(d->character) -= 2;
      }
    }

    if ((PLR_FLAGGED(d->character, PLR_OREMINE) || PLR_FLAGGED(d->character, PLR_GEMMINE)) && GET_MINE_TIME(d->character) < 1)
    {
         if (PLR_FLAGGED(d->character, PLR_OREMINE))
         {
          REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_OREMINE);
          get_ore_mine(d->character);
         }
         if (PLR_FLAGGED(d->character, PLR_GEMMINE))
         {
          REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_GEMMINE);
          get_gem_mine(d->character);
         }
      
    }
  }
}

void get_ore_mine(struct char_data *ch)
{
   int ranore=0,ranmat=0;

   ranore = rand_number(1,8);
   ranmat = rand_number(10,20);

   if (GET_PROF(ch, PROF_MINING) > 100)
   {
    if (rand_number(0,5001) > GET_PROF(ch, PROF_MINING))
    {
        SET_SKILL(ch, PROF_MINING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(1,25), 5000));
    }
    else
    {
        ranmat = ranmat*2;
        SET_SKILL(ch, PROF_MINING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(10,25), 5000));
    }
   }
   send_to_char(ch, "\r\n");

     switch(ranore)
     {
       case 1:
        GET_ADAMANTITE_ORE(ch) += ranmat;
        send_to_char(ch, "&mYou receive %d pieces of adamantite ore.\r\n", ranmat);
        break;
       case 2:
        GET_BRONZE_ORE(ch)+= ranmat;
        send_to_char(ch, "&rYou receive %d pieces of bronze ore.\r\n", ranmat);
        break;
       case 3:
        GET_COPPER_ORE(ch)+= ranmat;
        send_to_char(ch, "&yYou receive %d pieces of copper ore.\r\n", ranmat);
        break;
       case 4:
        GET_GOLD_ORE(ch)+= ranmat;
        send_to_char(ch, "&YYou receive %d pieces of gold ore.\r\n", ranmat);
        break;
       case 5:
        GET_IRON_ORE(ch)+= ranmat;
        send_to_char(ch, "&cYou receive %d pieces of iron ore.\r\n", ranmat);
        break;
       case 6:
        GET_MITHRIL_ORE(ch)+= ranmat;
        send_to_char(ch, "&WYou receive %d pieces of mithril ore.\r\n", ranmat);
        break;
       case 7:
        GET_SILVER_ORE(ch)+= ranmat;
        send_to_char(ch, "&wYou receive %d pieces of silver ore.\r\n", ranmat);
        break;
       case 8:
        GET_TITANIUM_ORE(ch)+= ranmat;
        send_to_char(ch, "&BYou receive %d pieces of titanium ore.\r\n", ranmat);
        break;
       default:
         break;
     }
  
  send_to_char(ch, "&n");
  return;
}

void get_gem_mine(struct char_data *ch)
{
   int rangem=0,ranmat=0;

   rangem = rand_number(1,14);
   ranmat = rand_number(10,20);

   if (GET_PROF(ch, PROF_MINING) > 100)
   {
    if (rand_number(0,5001) > GET_PROF(ch, PROF_MINING))
    {
        SET_SKILL(ch, PROF_MINING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(1,25), 5000));
    }
    else
    {
        ranmat = ranmat*2;
        SET_SKILL(ch, PROF_MINING, MIN(GET_PROF(ch, PROF_MINING) + rand_number(10,25), 5000));
    }
   }
   send_to_char(ch, "\r\n");

    send_to_char(ch, "\r\n");

    switch(rangem)
    {
       case 1:
       case 9:
       case 11:
       case 13:
        GET_CRYSTAL(ch)+= ranmat;
        send_to_char(ch, "&wYou receive %d crystal gems.\r\n", ranmat);
         break;
       case 2:
       case 10:
       case 12:
       case 14:
        GET_DIAMOND(ch)+= ranmat;
        send_to_char(ch, "&WYou receive %d diamond gems.\r\n", ranmat);
         break;
       case 3:
        GET_EMERALD(ch)+= ranmat;
        send_to_char(ch, "&GYou receive %d emerald gems.\r\n", ranmat);
         break;
       case 4:
        GET_GARNET(ch)+= ranmat,
        send_to_char(ch, "&rYou receive %d garnet gems.\r\n", ranmat);
         break;
       case 5:
        GET_JADE(ch)+= ranmat;
        send_to_char(ch, "&gYou receive %d jade gems.\r\n", ranmat);
         break;
       case 6:
        GET_RUBY(ch)+= ranmat;
        send_to_char(ch, "&RYou receive %d ruby gems.\r\n", ranmat);
         break;
       case 7:
        GET_SAPPHIRE(ch)+= ranmat;
        send_to_char(ch, "&CYou receive %d sapphire gems.\r\n", ranmat);
         break;
       case 8:
        GET_TOPAZ(ch)+= ranmat;
        send_to_char(ch, "&BYou receive %d topaz gems.\r\n", ranmat);
         break;
        default:
         break;
     }
  send_to_char(ch, "&n");
  return;
}

SPECIAL(smithy)
{
  int i, inc=0;
  struct obj_data *obj;
  char buf[MAX_INPUT_LENGTH], buf1[MAX_INPUT_LENGTH];

  two_arguments(argument, buf, buf1);
  
  send_to_char(ch,"\r\n&W");

  if (CMD_IS("improve")) {

    if(!*buf)
    {
      send_to_char(ch, "What do you want to improve?\r\n");

      return(TRUE);
    }

    if(!*buf1)
    {
      send_to_char(ch, "What stats do you want to improve? ( HR, DR, HP, MN, MV, SS ) \r\n");
      return(TRUE);
    }

    if (!(obj = get_obj_in_list_vis(ch, buf, NULL, ch->carrying)))
    {
     send_to_char(ch, "You are not carrying that.\r\n");
     return(TRUE);
    }

    if (!OBJ_FLAGGED(obj, ITEM_IMPROVE))
    {
     send_to_char(ch, "You can't improve on this item.\r\n");
     return(TRUE);
    }
    else
    {
    switch (LOWER(*buf1)) {
      case 'h':
       if(!strcmp(buf1,"hr"))
       {
         if(GET_CRYSTAL(ch) < 5)
         {
           send_to_char(ch, "You need at least %ld more crystal gems to improve %s.", 
                             (5-GET_CRYSTAL(ch)), obj->short_description);
           break;
         }
         else if(GET_GARNET(ch) < 20)
         {
           send_to_char(ch, "You need at least %ld more garnet gems to improve %s.", 
                             (20-GET_GARNET(ch)), obj->short_description);
           break;
         }
         else
         {
          for (i = 0; i < MAX_OBJ_AFFECT; i++) {
           if (i > 1)
           {
             send_to_char(ch, "You can't improve this item any more.");
             break;
           }
           if ((obj->affected[i].location == APPLY_NONE || obj->affected[i].location == APPLY_HITROLL) && i < 2)
           {
            if(obj->affected[i].modifier < 3) {
             obj->affected[i].location = APPLY_HITROLL;
             obj->affected[i].modifier += 1;   
             GET_CRYSTAL(ch) -= 5;
             GET_GARNET(ch) -= 20; 
             send_to_char(ch, "You raise the hitroll on %s by 1.", obj->short_description);
             break;
            } else {
             send_to_char(ch, "You can't add anymore hitroll to this item.");
             break;
            }
           }
          }
         }
        }
       else if(!strcmp(buf1,"hp"))
       {
         if(GET_DIAMOND(ch) < 5)
         {
           send_to_char(ch, "You need at least %ld more diamond gems to improve %s.",
                             (5-GET_DIAMOND(ch)), obj->short_description);
           break;
         }
         else if(GET_SAPPHIRE(ch) < 10)
         {
           send_to_char(ch, "You need at least %ld more sapphire gems to improve %s.",
                             (10-GET_SAPPHIRE(ch)), obj->short_description);
           break;
         }
         else
         {
         for (i = 0; i < MAX_OBJ_AFFECT; i++) {
          if (i > 1)
          {
             send_to_char(ch, "You can't add anymore to this item.");
             break;
          }
          if ((obj->affected[i].location == APPLY_NONE || obj->affected[i].location == APPLY_HIT) && i < 2)
          {
           if(obj->affected[i].modifier < 30) {
             obj->affected[i].location = APPLY_HIT;
             inc = rand_number(1,10);
             obj->affected[i].modifier += inc;
             GET_DIAMOND(ch) -= 5;
             GET_SAPPHIRE(ch) -= 10;
             send_to_char(ch, "You raise the hitpoints on %s by %d.", obj->short_description, inc);
             break;
           } else {
             send_to_char(ch, "You can't add anymore hitpoints to this item.");
             break;
            }
          }
         }
        }
       }
      break;
      case 'd':
      
       if(GET_CRYSTAL(ch) < 5)
         {
           send_to_char(ch, "You need at least %ld more crystal gems to improve %s.",
                             (5-GET_CRYSTAL(ch)), obj->short_description);
           break;
         }
         else if(GET_RUBY(ch) < 20)
         {
           send_to_char(ch, "You need at least %ld more ruby gems to improve %s.",
                             (20-GET_RUBY(ch)), obj->short_description);
           break;
         }
         else
         {
         for (i = 0; i < MAX_OBJ_AFFECT; i++) {
          if (i > 1)
          {
             send_to_char(ch, "You can't add anymore to this item.");
             break;
          }
          if ((obj->affected[i].location == APPLY_NONE || obj->affected[i].location == APPLY_DAMROLL) && i < 2)
          {
           if (obj->affected[i].modifier < 3) { 
             obj->affected[i].location = APPLY_DAMROLL;
             obj->affected[i].modifier += 1;
             GET_CRYSTAL(ch) -= 5;
             GET_RUBY(ch) -= 20;
             send_to_char(ch, "You raise the damroll on %s by 1.", obj->short_description);
             break;
           } else {
             send_to_char(ch, "You can't add anymore damroll to this item.");
             break;
            }
          }
         }
        }
      break;
      case 'm':
       if(!strcmp(buf1,"mn"))
       {
         if(GET_DIAMOND(ch) < 5)
         {
           send_to_char(ch, "You need at least %ld more diamond gems to improve %s.",
                             (5-GET_DIAMOND(ch)), obj->short_description);
           break;
         }
         else if(GET_TOPAZ(ch) < 10)
         {
           send_to_char(ch, "You need at least %ld more topaz gems to improve %s.",
                             (10-GET_TOPAZ(ch)), obj->short_description);
           break;
         }
         else
         {
         for (i = 0; i < MAX_OBJ_AFFECT; i++) {
          if (i > 1)
          {
             send_to_char(ch, "You can't add anymore to this item.");
             break;
          }
          if ((obj->affected[i].location == APPLY_NONE || obj->affected[i].location == APPLY_MANA) && i < 2)
          {
           if (obj->affected[i].modifier < 30) {
            obj->affected[i].location = APPLY_MANA;
            inc = rand_number(1,10);
            obj->affected[i].modifier += inc;
            GET_DIAMOND(ch) -= 5;
            GET_TOPAZ(ch) -= 10;
            send_to_char(ch, "You raise the mana on %s by %d.", obj->short_description, inc);
            break;
           } else {
             send_to_char(ch, "You can't add anymore mana to this item.");
             break;
            }
          }
         }
        }
       }
       else if(!strcmp(buf1,"mv"))
       {
         if(GET_DIAMOND(ch) < 5)
         {
           send_to_char(ch, "You need at least %ld more diamond gems to improve %s.",
                             (5-GET_DIAMOND(ch)), obj->short_description);
           break;
         }
         else if(GET_EMERALD(ch) < 10)
         {
           send_to_char(ch, "You need at least %ld more emerald gems to improve %s.",
                             (10-GET_EMERALD(ch)), obj->short_description);
           break;
         }
         else
         {
         for (i = 0; i < MAX_OBJ_AFFECT; i++) {
          if (i > 1)
          {
             send_to_char(ch, "You can't add any more to this item.");
             break;
          }
          if ((obj->affected[i].location == APPLY_NONE || obj->affected[i].location == APPLY_MOVE) && i < 2)
          {
           if (obj->affected[i].modifier < 75) {
            obj->affected[i].location = APPLY_MOVE;
            inc = rand_number(10,20);
            obj->affected[i].modifier += inc;
            GET_DIAMOND(ch) -= 5;
            GET_EMERALD(ch) -= 10;
            send_to_char(ch, "You raise the movements on %s by %d.", obj->short_description, inc);
            break;
           } else {
             send_to_char(ch, "You can't add anymore movement to this item.");
             break;
           }

          }
         }
        }
       }
      break;
      case 's':
       if(GET_CRYSTAL(ch) < 5)
         {
           send_to_char(ch, "You need at least %ld more crystal gems to improve %s.",
                             (5-GET_CRYSTAL(ch)), obj->short_description);
           break;
         }
         else if(GET_JADE(ch) < 20)
         {
           send_to_char(ch, "You need at least %ld more jade gems to improve %s.",
                             (20-GET_JADE(ch)), obj->short_description);
           break;
         }
         else
         {
         for (i = 0; i < MAX_OBJ_AFFECT; i++) {
          if (i > 1)
          {
 	     send_to_char(ch, "You can't add any more to this item.");
             break;
          } 
          if ((obj->affected[i].location == APPLY_NONE || obj->affected[i].location == APPLY_SAVING_SPELL) && i < 2)
          {
           if (obj->affected[i].modifier > -3) {
            obj->affected[i].location = APPLY_SAVING_SPELL;
            obj->affected[i].modifier -= 1;
            GET_DIAMOND(ch) -= 5;
            GET_JADE(ch) -= 20;
            send_to_char(ch, "You decreased the save spell on %s by 1.", obj->short_description);
            break;
           } else {
             send_to_char(ch, "You can't decrease the save spell an further on this item.");
             break;
           }
          }
         }
        }
      break;
     default:
       send_to_char(ch, "What stats do you want to improve? ( HR, DR, HP, MN, MV, SS ) \r\n");
       break;
    }
   send_to_char(ch, "&n\r\n");
   } 
  }
  else
    return(FALSE);
return(TRUE);
}

SPECIAL(armorer)
{
  struct obj_data *obj;
  char buf[MAX_INPUT_LENGTH], buf1[MAX_INPUT_LENGTH];

  two_arguments(argument, buf, buf1);

  send_to_char(ch, "\r\n&W");
  if (CMD_IS("repair")) {
    if(!*buf)
    {
      send_to_char(ch, "What do you want to repair?\r\n");
      return(TRUE);
    }
    if(!*buf1)
    {
      send_to_char(ch, "What kind of ore do you want to use?\r\n");
      return(TRUE);
    }

    if (!(obj = get_obj_in_list_vis(ch, buf, NULL, ch->carrying)))
    {
     send_to_char(ch, "You are not carrying item.\r\n");
     return(TRUE);
    }
//    else 
//    {
    if(GET_OBJ_COND(obj) < 10)
    {
      send_to_char(ch, "That isn't damaged enough for me to work on it.\r\n");
      return(TRUE);
    }
    else if (OBJ_FLAGGED(obj, ITEM_NO_REPAIR))
    {
      send_to_char(ch, "Sorry, I can't repair that item.\r\n");
      return(TRUE);
    }
    else
    {
    switch (LOWER(*buf1)) {
      case 'b':
        if(GET_BRONZE_ORE(ch) > 1)
        {
          GET_BRONZE_ORE(ch) -= 1; 
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 bronze ore to repair %s.\r\n", obj->short_description); 
        }
        else
          send_to_char(ch, "You do not have any bronze ore.\r\n");
        break;
      case 'c':
        if(GET_COPPER_ORE(ch) > 1)
        {
          GET_COPPER_ORE(ch) -= 1;
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 copper ore to repair %s.\r\n", obj->short_description);
        }
        else
          send_to_char(ch, "You do not have any copper ore.\r\n");
        break;
      case 'a':
        if(GET_ADAMANTITE_ORE(ch) > 1)
        {
          GET_ADAMANTITE_ORE(ch) -= 1;
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 adamantite ore to repair %s.\r\n", obj->short_description);
        }
        else
          send_to_char(ch, "You do not have any adamantite ore.\r\n");
        break;
      case 'g':
        if(GET_GOLD_ORE(ch) > 1)
        {
          GET_GOLD_ORE(ch) -= 1;
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 gold ore to repair %s.\r\n", obj->short_description);
        }
        else
          send_to_char(ch, "You do not have any gold ore.\r\n");
        break;
      case 's':
        if(GET_SILVER_ORE(ch) > 1)
        {
          GET_SILVER_ORE(ch) -= 1;
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 silver ore to repair %s.\r\n", obj->short_description);
        }
        else
          send_to_char(ch, "You do not have any silver ore.\r\n");
        break;
      case 't':
        if(GET_TITANIUM_ORE(ch) > 1)
        {
          GET_TITANIUM_ORE(ch) -= 1;
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 titanium ore to repair %s.\r\n", obj->short_description);
        }
        else
          send_to_char(ch, "You do not have any titanium ore.\r\n");
        break;
      case 'm':
        if(GET_MITHRIL_ORE(ch) > 1)
        {
          GET_MITHRIL_ORE(ch) -= 1;
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 mithril ore to repair %s.\r\n", obj->short_description);
        }
        else
          send_to_char(ch, "You do not have any mithril ore.\r\n");
        break;
      case 'i':
        if(GET_IRON_ORE(ch) > 1)
        {
          GET_IRON_ORE(ch) -= 1;
          GET_OBJ_COND(obj) += MIN(GET_OBJ_COND(obj) + rand_number(10,30), 5000);
          send_to_char(ch, "You use 1 iron ore to repair %s.\r\n", obj->short_description);
        }
        else
          send_to_char(ch, "You do not have any iron ore.\r\n");
        break;
    }
    send_to_char(ch, "&n");
    if (GET_OBJ_COND(obj) < 0)
        GET_OBJ_COND(obj) = 0;
   } 
   
  } 
  else
   return(FALSE);

return(TRUE);
}

