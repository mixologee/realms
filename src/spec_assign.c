/* ************************************************************************
*   File: spec_assign.c                                 
*   Usage: Functions to assign function pointers to objs/mobs/rooms        *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "screen.h"
#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"


/* external globals */
extern int mini_mud;

/* NOTE:  Remember when adding in a new SPECIAL func, to add it into the array at the BOTTOM of this file - Slurk */


/* external functions */
SPECIAL(dump);
SPECIAL(trade_shops);
SPECIAL(pet_shops);
SPECIAL(cityguard);
SPECIAL(receptionist);
SPECIAL(cryogenicist);
SPECIAL(guild_guard);
SPECIAL(guild);
SPECIAL(puff);
SPECIAL(cockroach);
SPECIAL(cockroach2);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(mayor);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(bank);
SPECIAL(gen_board);
SPECIAL(hunter);
SPECIAL(jenna);
SPECIAL(blaze_room);
SPECIAL(cleric);
SPECIAL(soul_master);
SPECIAL(ferry);
SPECIAL(ramona);
SPECIAL(octopus);
SPECIAL(cricket);
SPECIAL(spider);
SPECIAL(tmech);
SPECIAL(pblade);
SPECIAL(blocker);
SPECIAL(marshall_mirror);
SPECIAL(doctor);
SPECIAL(armorer);
SPECIAL(smithy);
SPECIAL(crystalgate);
SPECIAL(bank_teller);
SPECIAL(questmaster);
SPECIAL(goddess);
SPECIAL(tradingpost);

//SPECIAL(trade_shop);
//void assign_kings_castle(void);

/* local functions */
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void ASSIGNROOM(room_vnum room, SPECIAL(fname));
void ASSIGNMOB(mob_vnum mob, SPECIAL(fname));
void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname));

/* functions to perform assignments */

void ASSIGNMOB(mob_vnum mob, SPECIAL(fname))
{
  mob_rnum rnum;

  if ((rnum = real_mobile(mob)) != NOBODY)
    mob_index[rnum].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant mob #%d", mob);
}

void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname))
{
  obj_rnum rnum;

  if ((rnum = real_object(obj)) != NOTHING)
    obj_index[rnum].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant obj #%d", obj);
}

void ASSIGNROOM(room_vnum room, SPECIAL(fname))
{
  room_rnum rnum;

  if ((rnum = real_room(room)) != NOWHERE)
    world[rnum].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant room #%d", room);
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{

  ASSIGNMOB(1, puff);
  ASSIGNMOB(100, jenna);

  // PIRATE SCHOOL OF HARDKNOCKS
/*  ASSIGNMOB(2101, thief);
  ASSIGNMOB(2102, magic_user);
  ASSIGNMOB(2103, cleric);
  ASSIGNMOB(2105, thief);
  ASSIGNMOB(2106, magic_user);
  ASSIGNMOB(2107, cleric);
*/
  // zone 31
  ASSIGNMOB(3100, magic_user);
  ASSIGNMOB(3115, magic_user);
  ASSIGNMOB(3110, cleric);
  ASSIGNMOB(3111, cleric); 
/* FOODCHAIN MOBS / HUNTERS */
//  ASSIGNMOB(5403, hunter);        // sly fox - wilderness
//  ASSIGNMOB(5404, hunter);        // ranger - wilderness

  ASSIGNMOB(5800, guild);
// Mini-Zones
// ASSIGNMOB(5978, hunter);        // carnivorous plant
  ASSIGNMOB(5983, bank);
  ASSIGNMOB(5984, bank);
  
// God Facility
  ASSIGNMOB(6000, tmech);           // T-mech

// The Tropical City
  ASSIGNMOB(6100, bank);
  ASSIGNMOB(6102, guild);
  ASSIGNMOB(6103, guild);
  ASSIGNMOB(6104, guild);
  ASSIGNMOB(6105, guild);
  ASSIGNMOB(6107, magic_user);      //cityguards are casters
  ASSIGNMOB(6110, magic_user);      //aging terran is mage
// ANCIENT SEWERS OF TEK 
  ASSIGNMOB(6902, cockroach);
  ASSIGNMOB(6969, hunter);
  ASSIGNMOB(6904, cleric);

// The Lower Sewers 
  ASSIGNMOB(7007, spider); 

// The City Sewers 
  ASSIGNMOB(7100, cockroach2);
  
// The Cityi
  ASSIGNMOB(7205, bank);
  ASSIGNMOB(7207, guild);
  ASSIGNMOB(7208, guild);
  ASSIGNMOB(7209, guild);
  ASSIGNMOB(7210, guild);

// The Projects 
  ASSIGNMOB(7303, thief);
  ASSIGNMOB(7304, thief);
  ASSIGNMOB(7306, magic_user);
  ASSIGNMOB(7310, cleric);

// Pirate Hoarde 
  ASSIGNMOB(10001, blocker);
  ASSIGNMOB(10002, magic_user);
  ASSIGNMOB(10003, cleric);
  ASSIGNMOB(10004, magic_user);
  ASSIGNMOB(10005, thief);
  ASSIGNMOB(10006, thief);
  ASSIGNMOB(10008, magic_user);
  ASSIGNMOB(10009, cleric);  

// Burial Chamber of the Great Pirate Kings 
  ASSIGNMOB(11004, thief);
  ASSIGNMOB(11005, magic_user);
  ASSIGNMOB(11006, cleric);
  ASSIGNMOB(11103, magic_user);
  ASSIGNMOB(11104, cleric);
  ASSIGNMOB(11106, ramona);
  ASSIGNMOB(11107, blocker);
// Zone 112
  ASSIGNMOB(11203, magic_user);
  }
/* assign special procedures to objects */
void assign_objects(void)
{
  ASSIGNOBJ(6000, gen_board);	/* god board */
//  ASSIGNOBJ(7275, gen_board);   /* realestate board */
//  ASSIGNOBJ(7276, gen_board);   /* mortal board */
//  ASSIGNOBJ(3600, gen_board);	/* cashcard */
  ASSIGNOBJ(6111, pblade);      /* poison blade*/
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  room_rnum i;

  ASSIGNROOM(7291, pet_shops);
  ASSIGNROOM(11232, marshall_mirror);

  ASSIGNROOM(6540, tradingpost);

  ASSIGNROOM(7212, smithy);
  ASSIGNROOM(7212, armorer);
  
  ASSIGNROOM(7297, doctor);
  ASSIGNROOM(653, goddess);

  ASSIGNROOM(6543, smithy);
  ASSIGNROOM(6397, armorer);


  if (CONFIG_DTS_ARE_DUMPS)
    for (i = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_DEATH))
	world[i].func = dump;
}

struct spec_func_data {
   char *name;
   SPECIAL(*func);
};

struct spec_func_data spec_func_list[] = {
{"Dump", 		dump },
//{"Trade Shop", 		trade_shops },
{"Pet Shop", 		pet_shops },
//{"Postmaster", 		postmaster },
{"City Guard", 		cityguard },
{"Receptionist",	receptionist },
{"Cryogenicist", 	cryogenicist },
{"Guild Guard", 	guild_guard },
{"Guild",		guild },
{"Puff",		puff },
{"Cockroach",		cockroach },
{"Cockroach 2",		cockroach2 },
{"Fido",		fido },
{"Janitor",		janitor },
{"Mayor",		mayor },
{"Snake",		snake },
{"Thief",		thief },
{"Magic User",		magic_user },
{"Bank",		bank },
{"General Board",	gen_board },
{"Hunter", 		hunter },
{"Jenna",		jenna },
//{"Blaze Room",		blaze_room },
{"Cleric",		cleric },
{"Soul Master",		soul_master },
//{"Ferry",		ferry },
{"Ramona",		ramona },
{"Octopus",		octopus },
{"Cricket",		cricket },
{"Spider",		spider },
{"Tmech",		tmech },
{"PBlade",		pblade },
{"Blocker",		blocker },
{"Marshall Mirror",	marshall_mirror },
{"Doctor",		doctor },
{"Armorer",		armorer },
{"Smithy",		smithy },
//{"Crystal Gate",	crystalgate },
//{"Bank Teller",		bank_teller },
{"Questmaster",		questmaster },
{"\n",			NULL }
};


char *get_spec_func_name(SPECIAL(*func))
{
  int i;

  for (i = 0; *(spec_func_list[i].name) != '\n'; i++) {
    if (func == spec_func_list[i].func) 
      return (spec_func_list[i].name);
  }
  return NULL;
}
