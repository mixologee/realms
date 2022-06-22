/* Our version of mob factions */

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

extern int top_of_partylist;
extern struct mob_party *party_list;

const char *wolf_faction[] =
{
  "1000005",
  "1000006",
  "1000007",
};

const char *lion_faction[] =
{
  "1000008",
  "1000009",
  "1000010"
};

const char *kobold_faction[] =
{
  "1000011",
  "1000012",
  "1000013",
  "1000014",
  "1000015"
};

const char *bandit_faction[] =
{
  "1000017",
  "1000021",
  "1000022",
  "1000023",
  "1000024",
  "1000025"
};

const char *wasp_faction[] =
{
  "1000032",
  "1000033",
  "1000034",
  "1000035",
  "1000036"
};

void create_mob_party(struct char_data *ch, int danger)
{
   int num_in_party=0;
   int x, mob;//, y;
   mob_rnum mobnum;
//   room_rnum wroom = 6300;
   struct mob_party *new_party;
//   struct party_mem *mpm;
   struct char_data *wm, *leader;

   num_in_party = rand_number(1,9);

   CREATE(new_party, struct mob_party, top_of_partylist + 1);
//   CREATE(new_party->mobs, struct party_mem, 1);

   clear_party(new_party);
   new_party->next = party_list;
   party_list = new_party;
   top_of_partylist++;


   mob = rand_number(0, MAX_WOLF_PACT);
   mobnum = real_mobile(atoi(wolf_faction[mob]));
   leader = read_mobile(mobnum, REAL);
   SET_BIT_AR(MOB_FLAGS(leader), MOB_PARTYMEM);

   new_party->id = top_of_partylist;
   new_party->range = 100;
   new_party->danger = danger;
   new_party->leader = leader;
   new_party->num_mems = num_in_party;

//   new_party->mobs->mem = leader;
//   new_party->mobs->nextmem = NULL;

   char_to_room(leader, IN_ROOM(ch));

   for(x=1;x <= num_in_party;x++) {
      mob = rand_number(0, MAX_WOLF_PACT);
      mobnum = real_mobile(atoi(wolf_faction[mob]));
      wm = read_mobile(mobnum, REAL);
      SET_BIT_AR(MOB_FLAGS(wm), MOB_PARTYMEM);

//      CREATE(new_party->mobs, struct party_mem, 1);
//      new_party->mobs->nextmem = new_party->mobs->mem;
//      new_party->mobs->mem = wm;

      char_to_room(wm, IN_ROOM(ch));
      add_follower(wm, leader);
      perform_group(leader, wm);
      perform_group(wm, leader);
   }

//   new_party->mobs->nextmem = NULL;

}

ACMD(do_show_party)
{
  struct mob_party *tmp = NULL;

  tmp = party_list;

  send_to_char(ch, "List of Mob Parties currently in the Realms\r\n");
  send_to_char(ch, "ID   Range  Danger  NumMem     Leader     \r\n");
  send_to_char(ch, "-------------------------------------------\r\n");


  for ( ; tmp ; tmp = tmp->next) {
    send_to_char(ch, "%-5d %-5d  %-5d %-2d     %s\r\n", tmp->id, tmp->range, tmp->danger, tmp->num_mems, tmp->leader->player.short_descr);
  }
  
//  for (x=0;x < top_of_partylist;x++) {
//    send_to_char(ch, "%-5d %-5d  %-5d      %s\r\n", 
//                 party_list[x].id, party_list[x].range, party_list[x].danger, party_list[x].leader->player.short_descr);
//    send_to_char(ch, "%-5d %-5d  %-5d\r\n",
//                 party_list[x].id, party_list[x].range, party_list[x].danger);
//
//  }
}

ACMD(do_makemp)
{
  if(!*argument) {
    send_to_char(ch, "You need to set the danger level of the mob party (0-1000).");
    return;
  }

   create_mob_party(ch, atoi(argument));
}


