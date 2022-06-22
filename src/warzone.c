/*
 * Warzone code
 *
 * Lars
 * Mar 2001
 */

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

/* External stuff */
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct char_data *character_list;
extern struct clan_info *clan_index;
void reset_zone(zone_rnum zone);

/* Internal Stuff */
int value = FALSE;
int num_of_castles = 4;
struct castle_info castles[NUM_CASTLES];

void warzone_update(void)
{
  int num1 = 0, num2 = 0, num3=0, num4=0;

  if ((num1 = num_in_castleone()) > 0) {
  }

  if ((num2 = num_in_castletwo()) > 0) {
  }

  if ((num3 = num_in_castlethree()) > 0) {
  }

  if ((num4 = num_in_castlefour()) > 0) {
  }

}

int warzone_reset(struct descriptor_data *d, int castle, zone_rnum i)
{

  char buf[MAX_INPUT_LENGTH];
  reset_zone(i);
  sprintf(buf, "Zone %li has been reset!", i);
  log(buf);

  if (castle == 1) {
        
    sprintf(buf, "Castle one has been captured!");
    log(buf);
  }
  if (castle == 2) {
    sprintf(buf, "Castle two has been captured!");
    log(buf);
  }
  if (castle == 3) {

    sprintf(buf, "Castle three has been captured!");
    log(buf);
  }
  if (castle == 4) {
    sprintf(buf, "Castle four has been captured!");
    log(buf);
  }


  return (0);
}

int num_in_castleone()
{
//  struct follow_type *f;
//  struct char_data *leader;
  struct descriptor_data *pt;
  register struct descriptor_data *d;
  register struct descriptor_data *d_last;
  int num = 0;
  zone_rnum i;

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) == CON_PLAYING) {
      if (zone_table[world[d->character->in_room].zone].number == WZ_ZONE && GET_ROOM_VNUM(IN_ROOM(d->character)) >= CASTLEONE_BOTTOM && GET_ROOM_VNUM(IN_ROOM(d->character)) <= CASTLEONE_TOP) {
	i = world[d->character->in_room].zone;
	d_last = d;
//        if (GET_CLAN(d->character) != GET_CLAN(d_last->character))
//	  return (0);
	if (mobs_in_castleone() == 0) {
          num++;

         for (pt = descriptor_list; pt; pt = pt->next)
         {
           if (!(PLR_FLAGGED(pt->character, PLR_TAKE_SMOKE)))
              continue;
           else
           {
              send_to_char(pt->character, "\r\n&WYou recieve 2,000,000 experience and 250,000 gold for a succesful onslaught on Castle Smoke.\r\n");
              gain_exp(pt->character, 2000000);
              GET_GOLD(pt->character) += 250000;
              REMOVE_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_SMOKE);
           }
	 
          if (IS_PLAYING(pt))
          {
          send_to_char(pt->character, "&YCastle Smoke has been captured.&n\r\n");
          }
         } 
          warzone_reset(d, 1, i);
	  return (0);
	}
      }
    }
  }

  return (num);
}

int mobs_in_castleone()
{
  register struct char_data *i;
  int mob = 0;

  for (i = character_list; i; i = i->next)
    if (IS_NPC(i) && GET_ROOM_VNUM(IN_ROOM(i)) >= 20000 && GET_ROOM_VNUM(IN_ROOM(i)) <= 20049)
      mob++;

  return (mob);
}

int num_in_castletwo()
{ 
//  struct follow_type *f;
//  struct char_data *leader;
  struct descriptor_data *pt;
  struct descriptor_data *d;
  struct descriptor_data *d_last;
  int num = 0;
  zone_rnum i = 0;
 
  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) == CON_PLAYING) {
      if (zone_table[world[d->character->in_room].zone].number == WZ_ZONE && GET_ROOM_VNUM(IN_ROOM(d->character)) >= CASTLETWO_BOTTOM && GET_ROOM_VNUM(IN_ROOM(d->character)) <= CASTLETWO_TOP) { 
        i = world[d->character->in_room].zone;
	d_last = d;
//        if (GET_CLAN(d->character) != GET_CLAN(d_last->character))
//	  return (0);
        if (mobs_in_castletwo() == 0) {
          num++;
          
          for (pt = descriptor_list; pt; pt = pt->next)
          {
           if (!(PLR_FLAGGED(pt->character, PLR_TAKE_BLAZE)))
              continue;
           else
           {
              send_to_char(pt->character, "&WYou recieve 2,000,000 experience and 250,000 gold for a succesful onslaught on Castle Blaze.\r\n");
              gain_exp(pt->character, 2000000);
              GET_GOLD(pt->character) += 250000;
              REMOVE_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_BLAZE);
           }

          if (IS_PLAYING(pt) && pt->character)
          {
          send_to_char(pt->character, "&YCastle Blaze has been captured.&n\r\n");
          }
          }
          warzone_reset(d, 2, i);
          return (0);
        }
      }
    }
  }

  return (num);
}

int mobs_in_castletwo()
{
  register struct char_data *i;
  int mob = 0;
 
  for (i = character_list; i; i = i->next)
    if (IS_NPC(i) && GET_ROOM_VNUM(IN_ROOM(i)) >= 20050 && GET_ROOM_VNUM(IN_ROOM(i)) <= 20099)
      mob++;
 
  return (mob);
}


int num_in_castlethree()
{
//  struct follow_type *f;
//  struct char_data *leader;
  struct descriptor_data *pt;
  struct descriptor_data *d;
  struct descriptor_data *d_last;
  int num = 0;
  zone_rnum i = 0;

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) == CON_PLAYING) {
      if (zone_table[world[d->character->in_room].zone].number == WZ_ZONE2 && GET_ROOM_VNUM(IN_ROOM(d->character)) >= CASTLETHREE_BOTTOM && GET_ROOM_VNUM(IN_ROOM(d->character)) <= CASTLETHREE_TOP)
{
        i = world[d->character->in_room].zone;
        d_last = d;
//        if (GET_CLAN(d->character) != GET_CLAN(d_last->character))
//        return (0);
        if (mobs_in_castlethree() == 0) {
          num++;


         for (pt = descriptor_list; pt; pt = pt->next)
         {
           if (!(PLR_FLAGGED(pt->character, PLR_TAKE_CANN)))
              continue;
           else
            {
              send_to_char(pt->character, "&WYou recieve 4,000,000 experience and 500,000 gold for a succesful onslaught on Castle Cannibis.\r\n");
              gain_exp(pt->character, 4000000);
              GET_GOLD(pt->character) += 500000;
              REMOVE_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_CANN);
            }

           if (IS_PLAYING(pt) && pt->character)
           {
            send_to_char(pt->character, "&YCastle Cannibis has been captured.&n\r\n");
           }
         }
         warzone_reset(d, 3, i);
         return (0);
        }
      }
    }
  }

  return (num);
}

int mobs_in_castlethree()
{
  register struct char_data *i;
  int mob = 0;

  for (i = character_list; i; i = i->next)
    if (IS_NPC(i) && GET_ROOM_VNUM(IN_ROOM(i)) >= 19900 && GET_ROOM_VNUM(IN_ROOM(i)) <= 19949)
      mob++;

  return (mob);
}

int mobs_in_castlefour()
{
  register struct char_data *i;
  int mob = 0;

  for (i = character_list; i; i = i->next)
    if (IS_NPC(i) && GET_ROOM_VNUM(IN_ROOM(i)) >= 19950 && GET_ROOM_VNUM(IN_ROOM(i)) <= 19999)
      mob++;

  return (mob);
}

int num_in_castlefour()
{
//  struct follow_type *f;
//  struct char_data *leader;
  struct descriptor_data *pt;
  struct descriptor_data *d;
  struct descriptor_data *d_last;
  int num = 0;
  zone_rnum i = 0;

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) == CON_PLAYING) {
      if (zone_table[world[d->character->in_room].zone].number == WZ_ZONE2 && GET_ROOM_VNUM(IN_ROOM(d->character)) >= CASTLEFOUR_BOTTOM && GET_ROOM_VNUM(IN_ROOM(d->character)) <= CASTLEFOUR_TOP)
{
        i = world[d->character->in_room].zone;
        d_last = d;
//        if (GET_CLAN(d->character) != GET_CLAN(d_last->character))
//        return (0);
        if (mobs_in_castlefour() == 0) {
          num++;

          for (pt = descriptor_list; pt; pt = pt->next)
          {
           if (!(PLR_FLAGGED(pt->character, PLR_TAKE_SATI)))
              continue;
           else  
            {
              send_to_char(pt->character, "&WYou recieve 4,000,000 experience and 500,000 coins for a succesful onslaught on Castle Sativa.\r\n");
              gain_exp(pt->character, 4000000);
              GET_GOLD(pt->character) += 500000;
              REMOVE_BIT_AR(PLR_FLAGS(pt->character), PLR_TAKE_SATI);
            }

           if (IS_PLAYING(pt) && pt->character)
           {
            send_to_char(pt->character, "&YCastle Sativa has been captured.&n\r\n");
           }
          }
          warzone_reset(d, 4, i);
          return (0);
        }
      }
    }
  }

  return (num);
}

