///////////////////////////////////////////////////////////////////////////////////
////                   The Realms of Kuvia Codebase                            ////
///////////////////////////////////////////////////////////////////////////////////
////									       ////
//// File : crystalgate.c                                                      ////
////                                                                           ////
//// Usage: full cross world transportation system                             ////
////                      						       ////
///////////////////////////////////////////////////////////////////////////////////


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
#include "house.h"
#include "warzone.h"
#include "clan.h"
#include "arena.h"
#include "logger.h"
#include "crystalgate.h"

#define CRYSTALGATEMSG "There is no CrystalGate present in the area."
#define MAX_MSGS 5

void cmd_press                  (char_data *ch, char *argument);
void fix_facets                 (char_data *ch, obj_data *obj, int symbol);
void reset_crystalgate          (char_data *ch, obj_data *obj);
void check_valid_sequence       (char_data *ch, obj_data *obj);
void cmd_crystalgate            (char_data *ch, char *argument);


/* simple god function to show current symbols created, set to LVL_BUILDER in interp.c */

ACMD(do_symbols)
{
    int i, count;
    i = count = 0;
    char buf[MAX_STRING_LENGTH];

    for (i=1;i < (MAX_SYMBOL -1);i++)
    {
      if (symbol_table[i].name == NULL)  
        break;

      sprintf(buf, "%s ", symbol_table[i].name);
      send_to_char(ch, "&C%s&n  ", buf);
    }


      sprintf(buf, "%s ", symbol_table[i].name);

}


/* the following table is laid out as such
   Name, room_num, symbol1, symbol2, symbol3, minlevel, enabled, towilderness, Xcoor, Ycoord

   Name - City name to go to
   Room_num = room virtual number, or gate room
   symbol1-3 = the 3 symbol combination to open the gate to this city
   minlevel = minimum level char must be to goto this city, or use these coordinates (useful for 2nd continents etc)
   enabled = TRUE or FALSE, is the gate active or not
   towilderness = TRUE or FALSE, does it go to the wilderness - USElESS NOW
   X-Y coords = coords of the zone the gate is, this has to be a zone linked to the map!

*/

/* NOTE::  changing the way this works a little, the wilderness x/y coords will now be the CURRENT zone the gate resides in
           so for Taris'Vaal the x/y is 1000/1000 because that is where it "technically" resides at on the world map, that 
           way we can use the zones coords to throw off a random bad travel and port to wilderness within say 20 rooms of
           the x/y in any direction.  This makes the TRUE/FALSE part of the crystalgate_table useless, but thats fine
           If the zone is not linked to the map, use -1, -1 so that we can account for this and dont allow random ports
           for players going to these areas, maybe we just extend the time in travel since its no-fail
*/
 
const struct crystalgate_type crystalgate_table[] =
/*  Destination      rnum    symbol1             symbol2           symbol3          minlev disabled  wilderness  x  y */
{

   {"Narh Joon"    , 9700 , {SYMBOL_AMETHYST  , SYMBOL_AMETHYST  , SYMBOL_AMETHYST   }, 20, FALSE, FALSE, 1206, 1704 },
   {"Tropical City", 6122 , {SYMBOL_AMETHYST  , SYMBOL_SAPPHIRE  , SYMBOL_CITRINE    }, 1 , FALSE, FALSE,  660,  878 },
   {"Reosian Isles", 4100 , {SYMBOL_AQUAMARINE, SYMBOL_AQUAMARINE, SYMBOL_AQUAMARINE }, 35, FALSE, FALSE, 1981, 1981 },
   {"D'Arythnia   ", 1000 , {SYMBOL_AQUAMARINE, SYMBOL_CITRINE   , SYMBOL_EMERALD    }, 15, FALSE, FALSE,  975,  951 },
   {"Pirate Tombs" , 11000, {SYMBOL_AQUAMARINE, SYMBOL_JADE      , SYMBOL_SAPPHIRE   }, 20, FALSE, FALSE,   20, 1984 },
   {"Taris Mines  ", 2600 , {SYMBOL_CARNELIAN , SYMBOL_JADE      , SYMBOL_RUBY       }, 10, FALSE, FALSE,  967, 1025 },
   {"Pirate Hoarde", 10000, {SYMBOL_CITRINE   , SYMBOL_CITRINE   , SYMBOL_AMETHYST   }, 20, FALSE, FALSE, 1972,   24 },
   {"Pirate School", 2100 , {SYMBOL_CITRINE   , SYMBOL_CITRINE   , SYMBOL_CITRINE    }, 10, FALSE, FALSE,  956, 1000 },
   {"Silverwood"   , 3100 , {SYMBOL_EMERALD   , SYMBOL_EMERALD   , SYMBOL_EMERALD    }, 15, FALSE, FALSE,  984, 1038 },
   {"Isle of Siren", 14600, {SYMBOL_EMERALD   , SYMBOL_JADE      , SYMBOL_RUBY       }, 30, FALSE, FALSE,  700,  700 },
   {"Cocytus      ", 13485, {SYMBOL_EMERALD   , SYMBOL_RUBY      , SYMBOL_RUBY       }, 35, FALSE, FALSE, 1294, 1767 },
   {"Darkmoor     ", 4600 , {SYMBOL_EMERALD   , SYMBOL_SAPPHIRE  , SYMBOL_RUBY       }, 15, FALSE, FALSE, 1016, 1007 },
   {"Taris'Vaal"   , 6302 , {SYMBOL_LAPIS     , SYMBOL_LAPIS     , SYMBOL_LAPIS      }, NO_MIN_LEVEL, FALSE, FALSE, 1000, 1000 },
   {"Victoria"     , 2200 , {SYMBOL_RUBY      , SYMBOL_DIAMOND   , SYMBOL_RUBY       }, 15, FALSE, FALSE,  932, 1109 },
   {"The City"     , 7243 , {SYMBOL_SAPPHIRE  , SYMBOL_AMETHYST  , SYMBOL_SAPPHIRE   }, 1 , FALSE, FALSE, 1639, 1163 },
   {"Taris Caverns", 400  , {SYMBOL_SAPPHIRE  , SYMBOL_CARNELIAN , SYMBOL_DIAMOND    }, 1 , FALSE, FALSE, 1022,  999 },
   {NULL, -1, {-1, -1, -1}, -1, FALSE, FALSE, -1, -1}
};

/* the following table is used to store the symbols and a brief glow description you'll see on the crystalgate face
   example ("Circle", SYMBOL_CIRCLE, "with a soft white glow")

   This table must be in the EXACT same order as it is in crystalgate.h, so when you add new symbols add them there too
*/


const struct symbol_type symbol_table[] = 
{
  {"none",        SYMBOL_NONE,        "with boggled look on your face"},
  {"Ruby",        SYMBOL_RUBY,        "with a deep reddish glow"},
  {"Emerald",     SYMBOL_EMERALD,     "with a bright greenish glow"},
  {"Diamond",     SYMBOL_DIAMOND,     "with a white glow"},
  {"Jade",        SYMBOL_JADE,        "with a dark greenish glow"},
  {"Lapis",       SYMBOL_LAPIS,       "with a purple glow"},
  {"Amethyst",    SYMBOL_AMETHYST,    "with a transparent violet glow"},
  {"Sapphire",    SYMBOL_SAPPHIRE,    "with a deep blueish glow"},
  {"Citrine",     SYMBOL_CITRINE,     "with a bright orange glow"},
  {"Aquamarine",  SYMBOL_AQUAMARINE,  "with a cyan glow"},
  {"Carnelian",   SYMBOL_CARNELIAN,   "with a reddish-brown glow"},
  {NULL, -1}
};



/* the following table is used for the in-transit txt messages you get shown while waiting to exit on the other end */
/* Add-subtract messages at will, but change the # of  MAX_MSGS above to indicate the new amount*/

const char *crygate_messages[] =
  {
    "&CStrands of time and space intertwine in a magnificient display of light.\r\n&n",
    "&GYou catch glimpses of other journeymen while traveling past multiple travel nodes.&n\r\n",
    "&YFlashes of intense light temporarily blind you as you travel to your destination.&n\r\n",
    "&GYou feel the consiousness of various minds around you as you float in a stream of light.&n\r\n",
    "\n"
  };


/* the following command is for when the character presses the symbols, on success the gate takes them, if not it resets - simple! */

ACMD(do_press)
{
  char arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  obj_data *obj;
  bool IS_CRYSTALGATE = FALSE;
  int symbol, i;

  if (!*argument) {
    send_to_char(ch, "Press what?\r\n");
    return;
  }

  argument = one_argument(argument, arg);
  
  for (obj = world[IN_ROOM(ch)].contents; obj; obj = obj->next_content)
  {
    if (GET_OBJ_TYPE(obj) == ITEM_CRYGATE || ROOM_FLAGGED(IN_ROOM(ch), ROOM_CRYSTALGATE))
    {
      IS_CRYSTALGATE = TRUE;
      break;
    }
    else
    {
      IS_CRYSTALGATE = FALSE;
      send_to_char(ch, "There is no gate here.\r\n");
      return;
    }
  }

  if (arg[0] == '\0')
  {
    if (!IS_CRYSTALGATE || obj == NULL || IS_CRYSTALGATE == FALSE) 
    {
      send_to_char(ch, "%s\r\n", CRYSTALGATEMSG);
      return;
    }
    send_to_char(ch, "Which symbol do you wish to press?\r\n");
    send_to_char(ch, "Looking around the various nodes on the gate you can make out the following symbols:\r\n");

    for (i=1;i<MAX_SYMBOL;i++)
    {
      if (symbol_table[i].name == NULL)  
        break;
      sprintf(buf, "%s ", symbol_table[i].name);
      send_to_char(ch, "%s", buf);
    }
  return;
  }

  /* look for the symbol in the list */

  symbol = -1;


  for (i=1;i<MAX_SYMBOL;i++) {
    if (symbol_table[i].name == NULL) 
      break;
    if (is_abbrev(arg, LOWER(symbol_table[i].name)))
      symbol = i;
  }


/*  
  for (i=1;i<MAX_SYMBOL;i++)
  {
    if (symbol_table[i].name == NULL)
      break;
    if (LOWER(arg[0]) == LOWER(symbol_table[i].name[0]))
      symbol = i;
  }
*/

  if (symbol == -1)
  {
    send_to_char(ch, "You cannot locate the %s symbol!\r\n\r\n", arg);
    act("$n seems to be having trouble locating some of the symbols.&n", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char(ch, "The only symbols present are:\r\n");
    
    for (i=1;i<MAX_SYMBOL;i++)
    {
      if (symbol_table[i].name == NULL)
        break;
      sprintf(buf, "%s ", symbol_table[i].name);
      send_to_char(ch, "%s", buf);
    }
  
  send_to_char(ch, "\r\n");
  return;
  }

  sprintf(buf, "You press the %s symbol and it lights up with %s.\r\n",symbol_table[symbol].name, symbol_table[symbol].press_string);
  send_to_char(ch, "%s", buf);
  act("You see $n pushing some of the symbols on the CrystalGate, but you can't make out which ones.\r\n", FALSE, ch, 0, 0, TO_ROOM);

  fix_facets(ch, obj, symbol); //function call to set the OBJ_VALS on the gate to store the symbols pressed

  WAIT_STATE(ch, (PULSE_CRYGATE/5) + rand_number(-2, 2));
}



/* fix_facets function, basic light-up of the facets symbols */

void fix_facets(char_data *ch, obj_data *obj, int symbol)
{
  int val1, val2, val3, dest;
  val1=val2=val3=dest=0;

  if (GET_OBJ_VAL(obj, 0) == 0)
  {
    GET_OBJ_VAL(obj, 0) = symbol;
    return;
  }
  if (GET_OBJ_VAL(obj, 1) == 0)
  {
    GET_OBJ_VAL(obj, 1) = symbol;
    return;
  }
  if (GET_OBJ_VAL(obj, 2) == 0)
  {
    GET_OBJ_VAL(obj, 2) = symbol;


    /* on pressing the last symbol, check to see if it activates the gate or not */
  check_valid_sequence(ch, obj);

    return;

  }
    /* missing a symbol, gate fails, reset it */

    reset_crystalgate(ch, obj);

  send_to_char(ch, "&WYou failed to create a stable connection between two nodes.\r\n\r\n");
  send_to_char(ch, "\r\n&RThe gate begins to hum loudly as the facets begin changing colors rapidly.  The air around the gate suddenly gets very cold.");
  send_to_char(ch, "&BA ball of pure energy exits the gate and slams into you, knocking you to your knees.&n\r\n");

  if (GET_LEVEL(ch) <= 15)
    GET_HIT(ch) -= rand_number(5,10);
  else if (GET_LEVEL(ch) > 15 || GET_LEVEL(ch) < 25)
    GET_HIT(ch) -= rand_number(15, 40);
  else if (GET_LEVEL(ch) > 25 || GET_LEVEL(ch) < 35)
    GET_HIT(ch) -= rand_number(40, 65);
  else if (GET_LEVEL(ch) > 35 || GET_LEVEL(ch) > 40)
    GET_HIT(ch) -= rand_number(65, 140);  

  GET_POS(ch) = POS_SITTING;

    return;
}


/* reset the crystalgate, used on invalid sequence and on boot of the game */

void reset_crystalgate(char_data *ch, obj_data *obj)
{
      GET_OBJ_VAL(obj, 0) = 0;
      GET_OBJ_VAL(obj, 1) = 0;
      GET_OBJ_VAL(obj, 2) = 0;
      GET_OBJ_VAL(obj, 3) = 0;

     send_to_char(ch, "\n\n&CAll the symbols on the gate fade.&n\n\n");
     act("&CAll the symbols on the gate fade.&n", FALSE, ch, 0, 0, TO_ROOM);
    return;
}


/* check_valid_sequence function, checks to see if the inputs are valid symbols and if the location exists, if so OPENUP! */

void check_valid_sequence(char_data *ch, obj_data *obj)
{
  int i, time, passfail;
  char buf[MAX_STRING_LENGTH];
  long int room;
  room_rnum targ;
  long int startX, startY;

  for (i=0;i<=MAX_GATES;i++)
  {
    if (crystalgate_table[i].name == NULL)
      return;

    if (crystalgate_table[i].symbol[0] == GET_OBJ_VAL(obj, 0) &&
        crystalgate_table[i].symbol[1] == GET_OBJ_VAL(obj, 1) &&
        crystalgate_table[i].symbol[2] == GET_OBJ_VAL(obj, 2) &&
        GET_LEVEL(ch) >= crystalgate_table[i].min_level &&
        !crystalgate_table[i].disabled)

    {
    
      send_to_char(ch, "The CrystalGate begins to hum as the symbols you pushed connect to their destination.\r\n\r\n");  
      act("The CrystalGate begins to hum as the symbols $n pushed connect to their destination.", FALSE, ch, 0, 0, TO_ROOM);
      sprintf(buf, "\r\n&WThe crystal facets pulse with blinding white light.  \r\nYou close your eyes to the light as a vortex opens up before you leading to your destination.\r\n\r\n");
      send_to_char(ch, "%s", buf);
      act("&WThe crystal facets pulse with blinding white light.\r\nYou turn your head as a vortex opens up within the archway&n", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(ch, "You take a step forward into the vortex and time around you stands still.\r\n");
      act("$n steps into the vortex and vanishes.\r\nThe vortex quickly closes as the CrystalGate consumes its energy.", FALSE, ch, 0, 0, TO_ROOM);      
      GET_OBJ_VAL(obj, 3) = crystalgate_table[i].vnum;
      if (GET_OBJ_VAL(obj, 3) <= 0) {
        room = 6300;
        send_to_char(ch, "&YA malfunction in the portal system has dumped you into Taris'Vaal City Square.\r\n");
        nmudlog(MISC_LOG, LVL_GOD, TRUE, "CryGate: ERROR: Object Value 3 = 0, defaulted to Taris'Vaal City");
      }
      else {
        room = GET_OBJ_VAL(obj, 3);
      }
      targ = real_room(room);
      startX = crystalgate_table[i].x;
      startY = crystalgate_table[i].y;

      passfail = rand_number(1, 100) - rand_number(1, 100);

/*
   if we roll less than or equal to 12 send them to start + random between 1-25 on both X and Y coords 
   if we roll a 100, send them to start - random between 1-25 on both X and Y coords
   otherwise, send them to the correct destination
*/   
/*
      if (passfail <= 12) {
        GET_DESTINATIONX(ch) = startX + rand_number(1,25);
        GET_DESTINATIONY(ch) = startY + rand_number(1,25);
        room = MAPZONE + (startY * MAPY) + startX;
        time = 1;
      } 
      else if (passfail >= 100) {
        GET_DESTINATIONX(ch) = startX - rand_number(1,25);
        GET_DESTINATIONY(ch) = startY - rand_number(1,25);
        room = MAPZONE + (startY * MAPY) + startX;
        time = 1;
      } 
      else {
        GET_DESTINATIONX(ch) = startX;
        GET_DESTINATIONY(ch) = startY;
        time = rand_number(10, 25);
      }    
*/

GET_DESTINATION(ch) = startX;
GET_DESTINATIONY(ch) = startY;
time = rand_number(10,25);


      reset_crystalgate(ch, obj);

/* set the character up */  

      GET_TRAVELTIME(ch) = time;  
      if (!PLR_FLAGGED(ch, PLR_INCRYGATE))
        SET_BIT_AR(PLR_FLAGS(ch), PLR_INCRYGATE);
      GET_DESTINATION(ch) = room;    
      GET_DESTINATIONX(ch) = crystalgate_table[i].x;
      GET_DESTINATIONY(ch) = crystalgate_table[i].y;
      char_from_room(ch);
      char_to_room(ch, real_room(CRYGATE_ROOM));

      return;
      }
   }
  send_to_char(ch, "&WYou failed to create a stable connection between two nodes.\r\n\r\n");
  send_to_char(ch, "\r\n&RThe gate begins to hum loudly as the facets begin changing colors rapidly.  The air around the gate suddenly becomes very cold.\r\n");
  send_to_char(ch, "&BA ball of pure energy exits the gate and slams into you, knocking you to your knees.\r\n&n");
  act("The CrystalGate's facets begin changing color rapidly and it begins to hum as the air around it gets very cold.", FALSE, ch, 0, 0, TO_ROOM);
  act("$N gets slammed with a ball of pure energy emitted by the gate.", FALSE, ch, 0, 0, TO_ROOM);
  GET_HIT(ch) -= rand_number(50, 400);
  GET_POS(ch) = POS_SITTING;
  reset_crystalgate(ch, obj);
  return;
}



/* this function has a few things to do, one is to cycle through crygate_msgs and display to char, then it also uses
   GET_TRAVELTIME(ch) and with every heart_pulse % (5 * PASSES_PER_SECOND) it deducts 1 point from traveltime.  Currently
   traveltime is a random number from 10-25.  Once the countdown is completed, move character from holding pen and drop
   them to their respective destination, which is saved under GET_DESTINATION(ch)
*/

void cycle_crygate_msgs(void)
{
  char_data *ch;
  room_vnum dest;

  for (ch = world[CRYGATE_ROOM_MSGS].people; ch; ch = ch->next_in_room) {
    if (GET_TRAVELTIME(ch) <= 0) {
      if (GET_DESTINATION(ch) < 1){
        GET_DESTINATION(ch) = real_room(6302);
        if (PLR_FLAGGED(ch, PLR_INCRYGATE))
          REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_INCRYGATE);
        GET_TRAVELTIME(ch) = 0;
        GET_DESTINATION(ch) = 0;
        send_to_char(ch, "ERROR: please tell an imm you saw this when using CrystalGate system.\r\n&n");
        nmudlog(MISC_LOG, LVL_GOD, TRUE, "CryGate: ERROR: Crygate generated a Traveltime or Destination less than 1 for %s", GET_NAME(ch));
      }
      char_from_room(ch);
      dest = real_room(GET_DESTINATION(ch));
      char_to_room(ch, dest);
      if (PLR_FLAGGED(ch, PLR_INCRYGATE))
        REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_INCRYGATE);
      GET_TRAVELTIME(ch) = 0;
      GET_DESTINATION(ch) = 0;
      GET_DESTINATIONX(ch) = 0;
      GET_DESTINATIONY(ch) = 0;
      act("\r\n&RTime suddenly catches back up to you and slams you with a sudden shockwave of energy that knocks you to your knees", FALSE, ch, 0, 0, TO_CHAR);
      GET_HIT(ch) -= rand_number(1,50);
      GET_POS(ch) = POS_SITTING;
      act("\r\n&CYou take some time to catch your breath before trying to stand up and leave.\r\n", FALSE, ch, 0, 0, TO_CHAR);
      look_at_room(ch, 0);    
      act("&CAs $n steps out of the vortex, a sudden shockwave slams into $M, knocking $M to $M knees.", FALSE, ch, 0, 0, TO_ROOM);
    } else
      GET_TRAVELTIME(ch) -= 1;
  }
  send_to_room(CRYGATE_ROOM_MSGS, crygate_messages[rand_number(0, MAX_MSGS)]);
}


/* cmd_crystalgate - immortal function to see all available gates, and all info used in creating gate locations
                  such as symbols, minlev, disabled, wilderness, x-y coords
*/

ACMD(do_crystalgate)
{
  int i;
  char buf[MAX_STRING_LENGTH];

if(GET_LEVEL(ch) >= LVL_GOD)
{

  send_to_char(ch, "&Y Current CrystalGate locations in our realm:\r\n");
  send_to_char(ch, "&CGate Name       &c| &CSymbol 1  Symbol 2  Symbol 3     &c|&C Min &c| &CDisab &c| &CWild &c |&C    X/Y    &c| &Cvnum&n\n");
  send_to_char(ch, "&c----------------|----------------------------------|-----|-------|-------|-----------|------&n\n");

  for (i=0;i<=MAX_GATES;i++)
  {
    if (crystalgate_table[i].name == NULL)
      return;

      sprintf(buf, "&m%-16s&c| &m%-10s %-10s %-10s&c | &m%3d&c | &m%5d&c | &m%5d&c | &m%-4d/%-4d &c| &m%d&n\n",
        crystalgate_table[i].name, 
        symbol_table[crystalgate_table[i].symbol[0]].name,
        symbol_table[crystalgate_table[i].symbol[1]].name,
        symbol_table[crystalgate_table[i].symbol[2]].name,
        crystalgate_table[i].min_level, crystalgate_table[i].disabled,
        crystalgate_table[i].to_wild, crystalgate_table[i].x, crystalgate_table[i].y, crystalgate_table[i].vnum);
      send_to_char(ch, buf);
    }
    return;
  }
  else if (GET_LEVEL(ch) < 41)
  send_to_char(ch, "You do not have the security clearance to view the CrystalGate Listing, check with Blunt or Slurk for info if needed\r\n");
 }



/* This function is for when the shit hits the fan and a player gets stuck in the holding pen, or one of the coords gets fubar. */

ACMD(do_fixtrans)
{               

  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg) {            
    send_to_char(ch, "Who is having CrystalGate Transport Problems?\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "%s\r\n", CONFIG_NOPERSON);
    return;
  }

  if (PLR_FLAGGED(vict, PLR_INCRYGATE)) {
    send_to_char(ch, "Fixing %s's Transport issues.\r\n", GET_NAME(vict));
    REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_INCRYGATE);
    GET_DESTINATION(vict) = 0;
    GET_TRAVELTIME(vict) = 0;
   return;
  }
  nmudlog(GOD_LOG, LVL_IMPL, TRUE, "CryGate: %s used the fixtrans command on %s", GET_NAME(ch), GET_NAME(vict));
}
