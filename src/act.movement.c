/**************************************************************************
*  File: act.movement.c                                    Part of tbaMUD *
*  Usage: Movement commands, door handling, & sleep/rest/etc state.       *
*                                                                         *
*  All rights reserved.  See license complete information.                *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

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
#include "constants.h"
#include "dg_scripts.h"

/* external functions */
int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);
int buildwalk(struct char_data *ch, int dir);
void look_at_room2(struct char_data *ch, int ignore_brief);
int perform_group(struct char_data *ch, struct char_data *vict);
extern long pulse;
extern void make_tracks(struct char_data *ch, int room);


/* local functions */
int has_boat(struct char_data *ch);
int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname);
int has_key(struct char_data *ch, obj_vnum key);
void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd);
int ok_pick(struct char_data *ch, obj_vnum keynum, int pickproof, int scmd);
ACMD(do_gen_door);
ACMD(do_enter);
ACMD(do_leave);
ACMD(do_stand);
ACMD(do_sit);
ACMD(do_rest);
ACMD(do_sleep);
ACMD(do_wake);
//ACMD(do_mbind);

ACMD(do_follow);

void get_materials_find(struct char_data *ch);
void dismount_char(struct char_data * ch);
void mount_char(struct char_data *ch, struct char_data *mount);
void dtmessage(struct char_data *ch);

void do_mbind(struct char_data *ch, struct char_data *vict);

/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
  struct obj_data *obj;
  int i;

  if (GET_LEVEL(ch) > LVL_IMMORT)
    return (1);

  if (AFF_FLAGGED(ch, AFF_WATERWALK) || AFF_FLAGGED(ch, AFF_FLYING))
    return (1);

  /* non-wearable boats in inventory will do it */
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
      return (1);

  /* and any boat you're wearing will do it too */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
      return (1);

  return (0);
}

/* Simple function to determine if char can fly. */
int has_flight(struct char_data *ch)
{
//  struct obj_data *obj;
//  int i;

  if (GET_LEVEL(ch) > LVL_IMMORT)
    return (1);

  if (AFF_FLAGGED(ch, AFF_FLYING))
    return (1);

  /* Non-wearable flying items in inventory will do it. */
//  for (obj = ch->carrying; obj; obj = obj->next_content)
//    if (OBJAFF_FLAGGED(obj, AFF_FLYING) && OBJAFF_FLAGGED(obj, AFF_FLYING))
//      return (1);

  /* Any equipped objects with AFF_FLYING will do it too. */
//  for (i = 0; i < NUM_WEARS; i++)
//    if (GET_EQ(ch, i) && OBJAFF_FLAGGED(obj, AFF_FLYING))
//      return (1);

  return (0);
}

/* Simple function to determine if char can scuba. */ 
int has_scuba(struct char_data *ch)
{
//  struct obj_data *obj;
//  int i;

  if (GET_LEVEL(ch) > LVL_IMMORT)
    return (1);

  if (AFF_FLAGGED(ch, AFF_SCUBA))
    return (1);

  /* Non-wearable scuba items in inventory will do it. */
//  for (obj = ch->carrying; obj; obj = obj->next_content)
//    if (OBJAFF_FLAGGED(obj, AFF_SCUBA) && (find_eq_pos(ch, obj, NULL) < 0))
//      return (1);

  /* Any equipped objects with AFF_SCUBA will do it too. */
//  for (i = 0; i < NUM_WEARS; i++)
//    if (GET_EQ(ch, i) && OBJAFF_FLAGGED(obj, AFF_SCUBA))
//      return (1);

  return (0);
}

bool check_block(struct char_data *ch, int dir)
{
  struct char_data *i;

  for (i = world[IN_ROOM(ch)].people; i; i = i->next_in_room)
    if (ch != i) {
	if (!IS_NPC(i) && GET_LEVEL(ch) < LVL_GOD)
	   continue;
	if (MOB_FLAGGED(i, MOB_BLOCKN) && dir == NORTH) {
	   send_to_char(ch, "&R%s blocks your way!&n\r\n", CAP(i->player.short_descr));
	   return (TRUE);
 	} else  if (MOB_FLAGGED(i, MOB_BLOCKS) && dir == SOUTH) {
           send_to_char(ch, "&R%s blocks your way!&n\r\n", CAP(i->player.short_descr));
           return (TRUE);
        } else if (MOB_FLAGGED(i, MOB_BLOCKE) && dir == EAST) {
           send_to_char(ch, "&R%s blocks your way!&n\r\n", CAP(i->player.short_descr));
           return (TRUE);
        } else if (MOB_FLAGGED(i, MOB_BLOCKW) && dir == WEST) {
           send_to_char(ch, "&R%s blocks your way!&n\r\n", CAP(i->player.short_descr));
           return (TRUE);
        } else if (MOB_FLAGGED(i, MOB_BLOCKU) && dir == UP) {
           send_to_char(ch, "&R%s blocks your way!&n\r\n", CAP(i->player.short_descr));
           return (TRUE);
        } else if (MOB_FLAGGED(i, MOB_BLOCKD) && dir == DOWN) {
           send_to_char(ch, "&R%s blocks your way!&n\r\n", CAP(i->player.short_descr));
           return (TRUE);
        } else
	  continue;
    }

  return (FALSE);
}


/* do_simple_move assumes that there is no master, no followers and that the
 * direction exists. It returns 1 for success, 0 if failure. */
int do_simple_move(struct char_data *ch, int dir, int need_specials_check)
{
  char throwaway[MAX_INPUT_LENGTH] = ""; /* Functions assume writable. */
  room_rnum was_in = IN_ROOM(ch);
  int need_movement;
  struct char_data *tripper;
  struct room_affect *raff;
  int same_room = 0, riding = 0, ridden_by = 0;
  int vnum;
  char buf2[MAX_INPUT_LENGTH];
  bool blocked = FALSE;
  room_rnum location;

  /* Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug */
  if (need_specials_check && special(ch, dir + 1, throwaway))
    return (0);

  /* blocked by a leave trigger ? */
  if (!leave_mtrigger(ch, dir) || IN_ROOM(ch) != was_in) /* prevent teleport crashes */
    return 0;
  if (!leave_wtrigger(&world[IN_ROOM(ch)], ch, dir) || IN_ROOM(ch) != was_in) /* prevent teleport crashes */
    return 0;
  if (!leave_otrigger(&world[IN_ROOM(ch)], ch, dir) || IN_ROOM(ch) != was_in) /* prevent teleport crashes */
    return 0;

   // check if they're mounted
   if (RIDING(ch))    riding = 1;
   if (RIDDEN_BY(ch)) ridden_by = 1;
   
   // if they're mounted, are they in the same room w/ their mount(ee)?
   if (riding && RIDING(ch)->in_room == ch->in_room)
     same_room = 1;
   else if (ridden_by && RIDDEN_BY(ch)->in_room == ch->in_room)
     same_room = 1;
 
   // tamed mobiles cannot move about (DAK)
   if (ridden_by && same_room && AFF_FLAGGED(ch, AFF_TAMED)) {
     send_to_char(ch, "You've been tamed.  Now act it!\r\n");
     return 0;
   }
  
   if (!IS_NPC(ch)) 
     blocked = check_block(ch, dir);

   if (blocked == TRUE)
     return 0; 

  /* charmed? */
  if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master && IN_ROOM(ch) == IN_ROOM(ch->master)) {
    send_to_char(ch, "The thought of leaving your master makes you weep.\r\n");
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
    return (0);
  }


  /* if this room or the one we're going to needs a boat, check for one */
  if ((SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM)) {
   if ((riding && !has_boat(RIDING(ch))) || !has_boat(ch)) {
      send_to_char(ch, "You need a boat to go there.\r\n");
      return (0);
    }
  }

  /* If this room or the one we're going to needs flight, check for it. */
  if ((SECT(IN_ROOM(ch)) == SECT_FLYING) || (SECT(EXIT(ch, dir)->to_room) == SECT_FLYING)) {
    if (!has_flight(ch)) {
      send_to_char(ch, "You need to be flying to go there!\r\n");
      return (0);
    }
  }

  /* If this room or the one we're going to needs scuba, check for it. */
  if ((SECT(IN_ROOM(ch)) == SECT_UNDERWATER) || (SECT(EXIT(ch, dir)->to_room) == SECT_UNDERWATER)) {
    if (!has_scuba(ch)) {
      send_to_char(ch, "You need to be able to breathe water to go there!\r\n");
      return (0);
    }
  }

  if (SECT(EXIT(ch, dir)->to_room) == SECT_NOPASS) {
    if (!has_scuba(ch)) {
      send_to_char(ch, "You can't seem to go there!\r\n");
      return (0);
    }
  }

  if (!IS_NPC(ch) && (ROOM_FLAGGED(IN_ROOM(ch), ROOM_RECALL)))
  {
    GET_RECALL(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
    GET_RECALLX(ch) = zone_table[GET_OLC_ZONE(ch)].x;
    GET_RECALLY(ch) = zone_table[GET_OLC_ZONE(ch)].y;
  }

  if (!IS_NPC(ch) && (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ALTAR)))
  {
    GET_ALTAR(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
    GET_ALTARX(ch) = zone_table[GET_OLC_ZONE(ch)].x;
    GET_ALTARY(ch) = zone_table[GET_OLC_ZONE(ch)].y;
  }

  /* move points needed is avg. move loss for src and destination sect type */
  need_movement = (movement_loss[SECT(IN_ROOM(ch))] +
		   movement_loss[SECT(EXIT(ch, dir)->to_room)]) / 2;

   if (ROOM_FLAGGED(EXIT(ch,dir)->to_room, ROOM_EXTRAMOVE) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_EXTRAMOVE))
      need_movement += need_movement/2;

   if (riding) {
     if (GET_MOVE(RIDING(ch)) < need_movement) {
       send_to_char(ch, "Your mount is too exhausted.\r\n");
       return 0;
     }
   } else {
     if (GET_MOVE(ch) < need_movement && !IS_NPC(ch)) {
       if (need_specials_check && ch->master)
         send_to_char(ch, "You are too exhausted to follow.\r\n");
       else
         send_to_char(ch, "You are too exhausted.\r\n");
       return 0;
     }
   }
   
   if (riding && GET_SKILL(ch, SKILL_RIDING) < rand_number(1, 101)-rand_number(-4,need_movement)) {
    act("$N rears backwards, throwing you to the ground.", FALSE, ch, 0, RIDING(ch), TO_CHAR);
    act("You rear backwards, throwing $n to the ground.", FALSE, ch, 0, RIDING(ch), TO_VICT);
    act("$N rears backwards, throwing $n to the ground.", FALSE, ch, 0, RIDING(ch), TO_NOTVICT);
    dismount_char(ch);
    damage(ch, ch, dice(1,6), -1);
    return (0);
  }

   vnum = world[EXIT(ch, dir)->to_room].number;

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ATRIUM)) {
     if (!House_can_enter(ch, vnum)) {
//    if (!House_can_enter(ch, GET_ROOM_VNUM(EXIT(ch, dir)->to_room))) {
      send_to_char(ch, "That's private property -- no trespassing!\r\n");
      return (0);
    }
  }

/* check zone level restrictions */

/* 
 * if zone min > PC lvl = NOT OK
 * if zone max < PC lvl = NOT OK
 * if zone max > PC lvl = OK
*/

  if (ZONE_FLAGGED(GET_ROOM_ZONE(EXIT(ch, dir)->to_room), ZONE_WILDERNESS)) {

      send_to_char(ch, "You are too powerful for that area.\r\n");
      send_to_char(ch, "Zone Min Lvl: %d\r\n", ZONE_MINLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)));
      send_to_char(ch, "Zone Max Lvl: %d\r\n", ZONE_MAXLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)));
      send_to_char(ch, "Your level  : %d\r\n", GET_LEVEL(ch));
      send_to_char(ch, "Get zone    : %c\r\n", GET_ROOM_ZONE(EXIT(ch, dir)->to_room));
      send_to_char(ch, "Exit        : %d\r\n", EXIT(ch, dir)->to_room);
      if (GET_LEVEL(ch) > ZONE_MAXLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)))
        send_to_char(ch, "Your level is less than the max level -- OK to pass\r\n");
      else
        send_to_char(ch, "Your level is greater than the max level--NOTOK to pass\r\n");
  } 

    if ((ZONE_MINLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)) != -1) && ZONE_MINLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)) > GET_LEVEL(ch) && !ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_WILDERNESS)) {
      send_to_char(ch, "You are not ready to enter that area.\r\n");
      return (0);
    }


    if ((ZONE_MAXLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)) != -1) && ZONE_MAXLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)) < GET_LEVEL(ch)) {
      send_to_char(ch, "You are too powerful for that area.\r\n");
      send_to_char(ch, "Zone Min Lvl: %d\r\n", ZONE_MINLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)));
      send_to_char(ch, "Zone Max Lvl: %d\r\n", ZONE_MAXLVL(GET_ROOM_ZONE(EXIT(ch, dir)->to_room)));
      send_to_char(ch, "Your level  : %d\r\n", GET_LEVEL(ch));
      send_to_char(ch, "Get zone    : %c\r\n", GET_ROOM_ZONE(EXIT(ch, dir)->to_room));
      send_to_char(ch, "Exit        : %d\r\n", EXIT(ch, dir)->to_room);
      return (0);
    }

  /* Check zone flag restrictions */
  if (ZONE_FLAGGED(GET_ROOM_ZONE(EXIT(ch, dir)->to_room), ZONE_CLOSED) && !ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_WILDERNESS)) {
    send_to_char(ch, "A mysterious barrier forces you back! That area is off-limits.\r\n");
    return (0);
  }
  if (ZONE_FLAGGED(GET_ROOM_ZONE(EXIT(ch, dir)->to_room), ZONE_NOIMMORT) && (GET_LEVEL(ch) >= LVL_IMMORT) && (GET_LEVEL(ch) < LVL_GRGOD) && !ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_WILDERNESS)) {
    send_to_char(ch, "A mysterious barrier forces you back! That area is off-limits.\r\n");
    return (0);
  }


  if ((riding || ridden_by) && ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_TUNNEL) &&
	      num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) >= CONFIG_TUNNEL_SIZE) {
    if (CONFIG_TUNNEL_SIZE > 1)
      send_to_char(ch, "There isn't enough room for you to go there!\r\n");
    else
      send_to_char(ch, "There isn't enough room there for more than one person!\r\n");
    return (0);
  } else {
     if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_TUNNEL) &&
         num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) > 1) {
       send_to_char(ch, "There isn't enough room there for more than one person!\r\n");
       return 0;
     }
  }

/*DRAGON*/
   if (ch->dragonflight) {
     send_to_char(ch, "Sit still!  You cannot go anywhere until you land!\r\n");
     return (0);
   }

  /* Mortals and low level gods cannot enter greater god rooms. */
  if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_GODROOM) &&
	GET_LEVEL(ch) < LVL_GOD) {
    send_to_char(ch, "You aren't godly enough to use that room!\r\n");
    return (0);
  }

  if (SECT(EXIT(ch, dir)->to_room) == SECT_NOPASS &&  GET_LEVEL(ch) < LVL_GOD) {
      send_to_char(ch, "No matter how hard you try, you can't see to get through.\r\n");
    return (0);
  }
 

  if (GET_POS(ch) == POS_MEDITATE)
  {
    send_to_char(ch, "You are too busy meditating to move.");
    return (0);
  }

  if (AFF_FLAGGED(ch, AFF_ENTANGLE))
    need_movement *= 4;

  if (GET_MOVE(ch) < need_movement && !IS_NPC(ch)) {
    if (need_specials_check && ch->master)
      send_to_char(ch, "You are too exhausted to follow.\r\n");
    else
      send_to_char(ch, "You are too exhausted.\r\n");

    return (0);
  }

  for (raff = world[EXIT(ch, dir)->to_room].room_affs; raff; raff = raff->next)
     if (IS_NPC(ch) && raff->type == RAFF_NPC_BARRIER) {
       send_to_char(ch, "You cannot go in that direction, there is a magical barrier blocking you.\r\n");
       return (0);
     }

  /* Now we know we're allowed to go into the room. */
  if (GET_LEVEL(ch) <= LVL_IMMORT && !IS_NPC(ch))
    GET_MOVE(ch) -= need_movement;

   if (GET_LEVEL(ch) <= LVL_IMMORT && !IS_NPC(ch) && !(riding || ridden_by))
     GET_MOVE(ch) -= need_movement;
   else if (riding)
     GET_MOVE(RIDING(ch)) -= need_movement;
   else if (ridden_by)
     GET_MOVE(RIDDEN_BY(ch)) -= need_movement;
 
   if (riding) {
     if (!IS_AFFECTED(RIDING(ch), AFF_SNEAK)) {
       if (IS_AFFECTED(ch, AFF_SNEAK)) {
         sprintf(buf2, "$n leaves %s.", dirs[dir]);
         act(buf2, TRUE, RIDING(ch), 0, 0, TO_ROOM);
       } else {
         sprintf(buf2, "$n rides $N %s.", dirs[dir]);
         act(buf2, TRUE, ch, 0, RIDING(ch), TO_NOTVICT);
       }
     }
   } else if (ridden_by) {
     if (!IS_AFFECTED(ch, AFF_SNEAK)) {
       if (IS_AFFECTED(RIDDEN_BY(ch), AFF_SNEAK)) {
         sprintf(buf2, "$n leaves %s.", dirs[dir]);
         act(buf2, TRUE, ch, 0, 0, TO_ROOM);
       } else {
         sprintf(buf2, "$n rides $N %s.", dirs[dir]);
         act(buf2, TRUE, RIDDEN_BY(ch), 0, ch, TO_NOTVICT);
       }
     }
   } else if (!AFF_FLAGGED(ch, AFF_SNEAK)) {
    char buf2[MAX_STRING_LENGTH];

    snprintf(buf2, sizeof(buf2), "$n leaves %s.", dirs[dir]);
    act(buf2, TRUE, ch, 0, 0, TO_ROOM);
  }
  was_in = IN_ROOM(ch);

  if ((ROOM_FLAGGED(ch->in_room, ROOM_SALTWATER_FISH) ||
       ROOM_FLAGGED(ch->in_room, ROOM_FRESHWATER_FISH)) &&
      (PLR_FLAGGED(ch, PLR_FISHING) || PLR_FLAGGED(ch, PLR_FISH_ON))) {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
    send_to_char(ch, "\r\n&YYou pack up your fishing gear and move on.&n\r\n\r\n");
  }

  if(PLR_FLAGGED(ch, PLR_LUMBER)) {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_LUMBER);
    send_to_char(ch, "\r\n&YYou pack up your axe and move on.&n\r\n\r\n");
  }

  if(PLR_FLAGGED(ch, PLR_GEMMINE)) {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GEMMINE);
    send_to_char(ch, "\r\n&YYou pack up your mining gear and move on.&n\r\n\r\n");
  }

  if(PLR_FLAGGED(ch, PLR_OREMINE)) {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_OREMINE);
    send_to_char(ch, "\r\n&YYou pack up your mining gear and move on.&n\r\n\r\n");
  }
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);

  if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS) && (world[IN_ROOM(ch)].wild_type != WILD_ZONEENT)) {
    MakeWildRoom(IN_ROOM(ch), dir);
//    log("Called MakeWildRoom\r\n");
  }
     

   if (riding && same_room && RIDING(ch)->in_room != ch->in_room) {
     char_from_room(RIDING(ch));
     char_to_room(RIDING(ch), ch->in_room);
   } else if (ridden_by && same_room && RIDDEN_BY(ch)->in_room != ch->in_room) {
     char_from_room(RIDDEN_BY(ch));
     char_to_room(RIDDEN_BY(ch), ch->in_room);
   }
 
   if (!IS_AFFECTED(ch, AFF_SNEAK)) {
     if (riding && same_room && !IS_AFFECTED(RIDING(ch), AFF_SNEAK)) {
       sprintf(buf2, "$n arrives from %s%s, riding $N.",
               (dir < UP  ? "the " : ""),
               (dir == UP ? "below": dir == DOWN ? "above" : dirs[rev_dir[dir]]));
       act(buf2, TRUE, ch, 0, RIDING(ch), TO_ROOM);
     } else if (ridden_by && same_room && !IS_AFFECTED(RIDDEN_BY(ch), AFF_SNEAK)) {
       sprintf(buf2, "$n arrives from %s%s, ridden by $N.",
       	      (dir < UP  ? "the " : ""),
       	      (dir == UP ? "below": dir == DOWN ? "above" : dirs[rev_dir[dir]]));
       act(buf2, TRUE, ch, 0, RIDDEN_BY(ch), TO_ROOM);
     } else if (!riding || (riding && !same_room))
       act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
   }
  /* move them first, then move them back if they aren't allowed to go. Also,
   * see if an entry trigger disallows the move */
  if (!entry_mtrigger(ch) || !enter_wtrigger(&world[IN_ROOM(ch)], ch, dir)) {
    char_from_room(ch);
    char_to_room(ch, was_in);
    return 0;
  }

 if (ch->desc != NULL)
    look_at_room2(ch, 0);

  if (!IS_NPC(ch) && (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ALTAR)))
  {
    GET_ALTAR(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
    GET_ALTARX(ch) = zone_table[GET_OLC_ZONE(ch)].x;
    GET_ALTARY(ch) = zone_table[GET_OLC_ZONE(ch)].y;
  }

  if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_TRIPWIRE) && !IS_NPC(ch))
  {
    tripper = GET_ROOM_TRIPPER(IN_ROOM(ch));
    if(rand_number(1,100) < (GET_WIS(ch) + GET_DEX(ch)+rand_number(1,64)) && tripper != ch && tripper != ch->master)
    {
      if(rand_number(1,100) < 80)
      {
        REMOVE_BIT_AR(ROOM_FLAGS(IN_ROOM(ch)), ROOM_TRIPWIRE);
        GET_ROOM_TRIPPER(IN_ROOM(ch)) = NULL;
      }
      send_to_char(ch, "\r\nYou trip over a trip wire.\r\n");
      damage(tripper, ch, 0, SKILL_TRIPWIRE);
      return (0);
    }
    else
      send_to_char(ch, "\r\nYou notice a tripwire on the ground.\r\n");

  }

 if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
  {
    if (!PLR_FLAGGED(ch, PLR_IN_NPK) && !PLR_FLAGGED(ch, PLR_IN_CPK))
    {
     SET_BIT_AR(PLR_FLAGS(ch), PLR_IN_NPK);
     send_to_char(ch, "&RNotice: You have entered a NEUTRAL PLAYER KILLING area!&n\r\n");
    }
    if (PLR_FLAGGED(ch, PLR_IN_CPK))
    {
       REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_IN_CPK);
       SET_BIT_AR(PLR_FLAGS(ch), PLR_IN_NPK);
       send_to_char(ch, "&RNotice: You have left CHAOTIC PLAYER KILL and entered NEUTRAL PLAYER KILL!&n\r\n");
    }
  }

  if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK) && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
  {
    if(PLR_FLAGGED(ch, PLR_IN_CPK) || PLR_FLAGGED(ch, PLR_IN_NPK))
    {
       REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_IN_NPK);
       REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_IN_CPK);
       send_to_char(ch, "&WNotice: You have left a player killing area!&n\r\n");
    }
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
  {
    if (!PLR_FLAGGED(ch, PLR_IN_CPK) && !PLR_FLAGGED(ch, PLR_IN_NPK))
    {
     SET_BIT_AR(PLR_FLAGS(ch), PLR_IN_CPK);
     send_to_char(ch, "&RNotice: You have entered a CHAOTIC PLAYER KILLING area! (Read HELP CPK, you have been warned.)&n\r\n");
    }
    if (PLR_FLAGGED(ch, PLR_IN_NPK))
    {
       REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_IN_NPK);
       SET_BIT_AR(PLR_FLAGS(ch), PLR_IN_CPK);
       send_to_char(ch, "&RNotice: You have left NEUTRAL PLAYER KILL and entered CHAOTIC PLAYER KILL! (Read HELP CPK, you have been warned.)&n\r\n");
    }
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_DEATH) && GET_LEVEL(ch) <= LVL_IMMORT) {
    dtmessage(ch);
    death_cry(ch);
    location = real_room(GET_ALTAR(ch));
    char_from_room(ch);
    char_to_room(ch, location);
    GET_HIT(ch) = 5;
    GET_POS(ch) = POS_RESTING;
    return (0);
  }

  entry_memory_mtrigger(ch);
  if (!greet_mtrigger(ch, dir)) {
    char_from_room(ch);
    char_to_room(ch, was_in);
    look_at_room(ch, 0);
    make_tracks(ch, IN_ROOM(ch));

     
   } else greet_memory_mtrigger(ch);
  

  return (1);
}

int perform_move(struct char_data *ch, int dir, int need_specials_check)
{
  room_rnum was_in;
  struct follow_type *k, *next;

  if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS || FIGHTING(ch))
    return (0);
  else if ((!EXIT(ch, dir) && !buildwalk(ch, dir)) || EXIT(ch, dir)->to_room == NOWHERE)
    send_to_char(ch, "Alas, you cannot go that way...\r\n");
  else if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) && EXIT_FLAGGED(EXIT(ch, dir), EX_HIDDEN))
    send_to_char(ch, "Alas, you cannot go that way...\r\n");
  else if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED)) {
    if (EXIT(ch, dir)->keyword)
      send_to_char(ch, "The %s seems to be closed.\r\n", fname(EXIT(ch, dir)->keyword));
    else
      send_to_char(ch, "It seems to be closed.\r\n");
  } else {
    if (!ch->followers)
      return (do_simple_move(ch, dir, need_specials_check));

    was_in = IN_ROOM(ch);
    if (!do_simple_move(ch, dir, need_specials_check))
      return (0);

    for (k = ch->followers; k; k = next) {
      next = k->next;
      if ((IN_ROOM(k->follower) == was_in) &&
	  (GET_POS(k->follower) >= POS_STANDING)) {
	act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
	perform_move(k->follower, dir, 1);
      }
    }
    return (1);
  }
  return (0);
}

ACMD(do_move)
{
  /* This is basically a mapping of cmd numbers to perform_move indices. It 
   * cannot be done in perform_move because perform_move is called by other 
   * functions which do not require the remapping. */
  perform_move(ch, subcmd, 0);
}

int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname)
{
  int door;
  if (*dir) {                   /*/ a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) {        /*/ Partial Match */
      send_to_char(ch, "That's not a direction.\r\n");
      return (-1);
    }
    if (EXIT(ch, door) && !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)){
      if (EXIT(ch, door)->keyword) {
        if (isname(type, EXIT(ch, door)->keyword))
          return (door);
        else {
          send_to_char(ch, "I see no %s there.\r\n", type);
          return (-1);
        }
      } else
        return (door);
    } else {
      send_to_char(ch, "I really don't see how you can %s anything there.\r\n", cmdname);
      return (-1);
    }
  } else {                      /*/ try to locate the keyword */
    if (!*type) {
      send_to_char(ch, "What is it you want to %s?\r\n", cmdname);
      return (-1);
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door) && !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
        if (EXIT(ch, door)->keyword)
          if (isname(type, EXIT(ch, door)->keyword))
            return (door);

    send_to_char(ch, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    return (-1);
  }
}

int has_key(struct char_data *ch, obj_vnum key)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return (1);

  if (GET_EQ(ch, WEAR_HOLD))
    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
      return (1);

  return (0);
}

#define NEED_OPEN	(1 << 0)
#define NEED_CLOSED	(1 << 1)
#define NEED_UNLOCKED	(1 << 2)
#define NEED_LOCKED	(1 << 3)

const char *cmd_door[] =
{
  "open",
  "close",
  "unlock",
  "lock",
  "pick"
};

const int flags_door[] =
{
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_OPEN,
  NEED_CLOSED | NEED_LOCKED,
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_CLOSED | NEED_LOCKED
};

#define EXITN(room, door)		(world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)	((obj) ?\
		(REMOVE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(REMOVE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define CLOSE_DOOR(room, obj, door)	((obj) ?\
		(SET_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(SET_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)	((obj) ?\
		(SET_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(SET_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))
#define UNLOCK_DOOR(room, obj, door)	((obj) ?\
		(REMOVE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(REMOVE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))
#define TOGGLE_LOCK(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd)
{
  char buf[MAX_STRING_LENGTH];
  size_t len;
  room_rnum other_room = NOWHERE;
  struct room_direction_data *back = NULL;

  if (!door_mtrigger(ch, scmd, door))
    return;

  if (!door_wtrigger(ch, scmd, door))
    return;

  len = snprintf(buf, sizeof(buf), "$n %ss ", cmd_door[scmd]);
  if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
    if ((back = world[other_room].dir_option[rev_dir[door]]) != NULL)
      if (back->to_room != IN_ROOM(ch))
	back = NULL;

  switch (scmd) {
  case SCMD_OPEN:
    OPEN_DOOR(IN_ROOM(ch), obj, door);
    if (back)
      OPEN_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(ch, "%s", CONFIG_OK);
    break;

  case SCMD_CLOSE:
    CLOSE_DOOR(IN_ROOM(ch), obj, door);
    if (back)
      CLOSE_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(ch, "%s", CONFIG_OK);
    break;

  case SCMD_LOCK:
    LOCK_DOOR(IN_ROOM(ch), obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(ch, "*Click*\r\n");
    break;

  case SCMD_UNLOCK:
    UNLOCK_DOOR(IN_ROOM(ch), obj, door);
    if (back)
      UNLOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(ch, "*Click*\r\n");
    break;

  case SCMD_PICK:
    TOGGLE_LOCK(IN_ROOM(ch), obj, door);
    if (back)
      TOGGLE_LOCK(other_room, obj, rev_dir[door]);
    send_to_char(ch, "The lock quickly yields to your skills.\r\n");
    len = strlcpy(buf, "$n skillfully picks the lock on ", sizeof(buf));
    break;
  }

  /* Notify the room. */
  if (len < sizeof(buf))
    snprintf(buf + len, sizeof(buf) - len, "%s%s.",
	obj ? "" : "the ", obj ? "$p" : EXIT(ch, door)->keyword ? "$F" : "door");
  if (!obj || IN_ROOM(obj) != NOWHERE) {
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_CHAR);
  }

  /* Notify the other room */
  if (back && (scmd == SCMD_OPEN || scmd == SCMD_CLOSE))
      send_to_room(EXIT(ch, door)->to_room, "The %s is %s%s from the other side.\r\n",
		back->keyword ? fname(back->keyword) : "door", cmd_door[scmd],
		scmd == SCMD_CLOSE ? "d" : "ed");
}

int ok_pick(struct char_data *ch, obj_vnum keynum, int pickproof, int scmd)
{
  int percent, skill_lvl;

  if (scmd != SCMD_PICK)
    return (1);

  percent = rand_number(1, 101);
  skill_lvl = GET_SKILL(ch, SKILL_PICK_LOCK) + dex_app_skill[GET_DEX(ch)].p_locks;

  if (keynum == NOTHING)
    send_to_char(ch, "Odd - you can't seem to find a keyhole.\r\n");
  else if (pickproof)
    send_to_char(ch, "It resists your attempts to pick it.\r\n");
  else if (percent > skill_lvl)
    send_to_char(ch, "You failed to pick the lock.\r\n");
  else
    return (1);

  return (0);
}

#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? ((GET_OBJ_TYPE(obj) == \
    ITEM_CONTAINER) && OBJVAL_FLAGGED(obj, CONT_CLOSEABLE)) :\
    (EXIT_FLAGGED(EXIT(ch, door), EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door) ((obj) ? (!OBJVAL_FLAGGED(obj, \
    CONT_CLOSED)) : (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door)	((obj) ? (!OBJVAL_FLAGGED(obj, \
    CONT_LOCKED)) : (!EXIT_FLAGGED(EXIT(ch, door), EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? (OBJVAL_FLAGGED(obj, \
    CONT_PICKPROOF)) : (EXIT_FLAGGED(EXIT(ch, door), EX_PICKPROOF)))
#define DOOR_IS_CLOSED(ch, obj, door) (!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door) (!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 2)) : \
    (EXIT(ch, door)->key))

ACMD(do_gen_door)
{
  int door = -1;
  obj_vnum keynum;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj = NULL;
  struct char_data *victim = NULL;

  skip_spaces(&argument);
  if (!*argument) {
    send_to_char(ch, "%c%s what?\r\n", UPPER(*cmd_door[subcmd]), cmd_door[subcmd] + 1);
    return;
  }
  two_arguments(argument, type, dir);
  if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    door = find_door(ch, type, dir, cmd_door[subcmd]);

  if ((obj) && (GET_OBJ_TYPE(obj) != ITEM_CONTAINER)) {
    obj = NULL;
    door = find_door(ch, type, dir, cmd_door[subcmd]);
  }

  if ((obj) || (door >= 0)) {
    keynum = DOOR_KEY(ch, obj, door);
    if (!(DOOR_IS_OPENABLE(ch, obj, door)))
      act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    else if (!DOOR_IS_OPEN(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_OPEN))
      send_to_char(ch, "But it's already closed!\r\n");
    else if (!DOOR_IS_CLOSED(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_CLOSED))
      send_to_char(ch, "But it's currently open!\r\n");
    else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_LOCKED))
      send_to_char(ch, "Oh.. it wasn't locked, after all..\r\n");
    else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_UNLOCKED))
      send_to_char(ch, "It seems to be locked.\r\n");
    else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_GOD) &&
	     ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
      send_to_char(ch, "You don't seem to have the proper key.\r\n");
    else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd))
      do_doorcmd(ch, obj, door, subcmd);
  }
  return;
}
ACMD(do_enter)
{
        EXIT_DATA *pexit;
        ROOM_DATA *pRoom;

        pRoom = &world[IN_ROOM(ch)];

        one_argument(argument, buf);

        if (*buf)                       /* an argument was supplied, search for door keyword */
        {
                for (pexit = pRoom->first_exit; pexit; pexit = pexit->next)
                {
                        if (pexit->keyword)
                        {
                                if (!str_cmp(pexit->keyword, buf))
                                {
                                        perform_move(ch, pexit->vdir, 1);
                                        return;
                                }
                        }
                }

                if (pRoom->buildings)
                {
                        BUILDING_DATA *bld = find_building_in_room_by_name(pRoom, str_dup(buf));

                        if (bld)
                        {
                                enter_building(ch, bld);
                                return;
                        }
                }

                if ( pRoom->vehicles )
                {
                        VEHICLE_DATA *vehicle = find_vehicle_in_room_by_name(ch, str_dup(buf));

                        if (vehicle)
                        {
                                char_enter_vehicle(ch, vehicle);
                                return;
                        }
                }

                sprintf(buf2, "There is no %s here.\r\n", buf);
                send_to_char(ch, buf2);
        }
        else if (ROOM_FLAGGED(ch->in_room, ROOM_INDOORS))
                send_to_char(ch, "You are already indoors.\r\n");
        else
        {
                /* try to locate an entrance */
                for ( pexit = pRoom->first_exit; pexit; pexit = pexit->next )
                {
                        if ( pexit->to_room != NULL )
                        {
                                if (!EXIT_FLAGGED(pexit, EX_CLOSED) &&
                                        ROOM_FLAGGED(pexit->to_room, ROOM_INDOORS))
                                {
                                        perform_move(ch, pexit->vdir, 1);
                                        return;
                                }
                        }
                }

                if (pRoom->buildings)
                {
                        BUILDING_DATA *bld = find_char_owned_building(ch);

                        if (bld)
                        {
                                enter_building(ch, bld);
                                return;
                        }
                }

                send_to_char(ch, "You can't seem to find anything to enter.\r\n");
        }
}
/*
ACMD(do_enter)
{
  char buf[MAX_INPUT_LENGTH];
  int door;

  one_argument(argument, buf);


  if (*buf) {		*/	/* an argument was supplied, search for door
				 * keyword */
/*    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char(ch, "There is no %s here.\r\n", buf);
  } else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_INDOORS))
    send_to_char(ch, "You are already indoors.\r\n");
  else {*/
    /* try to locate an entrance */
/*    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
	      ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char(ch, "You can't seem to find anything to enter.\r\n");
  }
}
*/
ACMD(do_leave)
{
  int door;

  if (OUTSIDE(ch))
    send_to_char(ch, "You are outside.. where do you want to go?\r\n");
  else if (ch->in_building != NULL)
    char_from_building(ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
	    !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char(ch, "I see no obvious exits to the outside.\r\n");
  }
}

ACMD(do_stand)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char(ch, "You are already standing.\r\n");
    break;
  case POS_SITTING:
    send_to_char(ch, "You stand up.\r\n");
    act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    /* Were they sitting in something? */
    char_from_furniture(ch);
    /* Will be sitting after a successful bash and may still be fighting. */
    GET_POS(ch) = FIGHTING(ch) ? POS_FIGHTING : POS_STANDING;
    break;
  case POS_RESTING:
    send_to_char(ch, "You stop resting, and stand up.\r\n");
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    /* Were they sitting in something. */
    char_from_furniture(ch);
    break;
  case POS_SLEEPING:
    send_to_char(ch, "You have to wake up first!\r\n");
    break;
  case POS_FIGHTING:
    send_to_char(ch, "Do you not consider fighting as standing?\r\n");
    break;
  default:
    send_to_char(ch, "You stop floating around, and put your feet on the ground.\r\n");
    act("$n stops floating around, and puts $s feet on the ground.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  }
}

ACMD(do_sit)
{
  char arg[MAX_STRING_LENGTH];
  struct obj_data *furniture;
  struct char_data *tempch;
  int found;

  one_argument(argument, arg);
 
  if (!*arg)
    found = 0;
  if (!(furniture = get_obj_in_list_vis(ch, arg, NULL, world[ch->in_room].contents)))
    found = 0;
  else
    found = 1;
  
  switch (GET_POS(ch)) {
  case POS_STANDING:
    if (found == 0) {
      send_to_char(ch, "You sit down.\r\n");
      act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POS_SITTING;
    } else {   
      if (GET_OBJ_TYPE(furniture) != ITEM_FURNITURE) {
        send_to_char(ch, "You can't sit on that!\r\n");
        return;
      } else if (GET_OBJ_VAL(furniture, 1) > GET_OBJ_VAL(furniture, 0)) {
        /* Val 1 is current number sitting, 0 is max in sitting. */
        act("$p looks like it's all full.", TRUE, ch, furniture, 0, TO_CHAR);
        log("SYSERR: Furniture %d holding too many people.", GET_OBJ_VNUM(furniture));
        return;
      } else if (GET_OBJ_VAL(furniture, 1) == GET_OBJ_VAL(furniture, 0)) {
        act("There is no where left to sit upon $p.", TRUE, ch, furniture, 0, TO_CHAR);
        return;
      } else { 
        if (OBJ_SAT_IN_BY(furniture) == NULL)
          OBJ_SAT_IN_BY(furniture) = ch;
        for (tempch = OBJ_SAT_IN_BY(furniture); tempch != ch ; tempch = NEXT_SITTING(tempch)) {
          if (NEXT_SITTING(tempch))
            continue;
          NEXT_SITTING(tempch) = ch;
        }
        act("You sit down upon $p.", TRUE, ch, furniture, 0, TO_CHAR);
        act("$n sits down upon $p.", TRUE, ch, furniture, 0, TO_ROOM);
        SITTING(ch) = furniture;
        NEXT_SITTING(ch) = NULL;
        GET_OBJ_VAL(furniture, 1) += 1;
        GET_POS(ch) = POS_SITTING;
      }
    }
    break;
  case POS_SITTING:
    send_to_char(ch, "You're sitting already.\r\n");
    break;
  case POS_RESTING:
    send_to_char(ch, "You stop resting, and sit up.\r\n");
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    send_to_char(ch, "You have to wake up first.\r\n");
    break;
  case POS_FIGHTING:
    send_to_char(ch, "Sit down while fighting? Are you MAD?\r\n");
    break;
  default:
    send_to_char(ch, "You stop floating around, and sit down.\r\n");
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}

ACMD(do_rest)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char(ch, "You sit down and rest your tired bones.\r\n");
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_SITTING:
    send_to_char(ch, "You rest your tired bones.\r\n");
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    send_to_char(ch, "You are already resting.\r\n");
    break;
  case POS_SLEEPING:
    send_to_char(ch, "You have to wake up first.\r\n");
    break;
  case POS_FIGHTING:
    send_to_char(ch, "Rest while fighting?  Are you MAD?\r\n");
    break;
  default:
    send_to_char(ch, "You stop floating around, and stop to rest your tired bones.\r\n");
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  }
}

ACMD(do_sleep)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
  case POS_SITTING:
  case POS_RESTING:
    send_to_char(ch, "You go to sleep.\r\n");
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char(ch, "You are already sound asleep.\r\n");
    break;
  case POS_FIGHTING:
    send_to_char(ch, "Sleep while fighting?  Are you MAD?\r\n");
    break;
  default:
    send_to_char(ch, "You stop floating around, and lie down to sleep.\r\n");
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}

ACMD(do_wake)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int self = 0;

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char(ch, "Maybe you should wake yourself up first.\r\n");
    else if ((vict = get_char_vis(ch, arg, NULL, FIND_CHAR_ROOM)) == NULL)
      send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (vict == ch)
      self = 1;
    else if (AWAKE(vict))
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (AFF_FLAGGED(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else if (GET_POS(vict) < POS_SLEEPING)
      act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_SITTING;
    }
    if (!self)
      return;
  }
  if (AFF_FLAGGED(ch, AFF_SLEEP))
    send_to_char(ch, "You can't wake up!\r\n");
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char(ch, "You are already awake...\r\n");
  else {
    send_to_char(ch, "You awaken, and sit up.\r\n");
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
  }
}

ACMD(do_follow)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *leader;
  struct follow_type *f;
  int i, num_followin = 1;
  bool spotopen = TRUE;
  one_argument(argument, buf);

  if (*buf)
  {
    if (!(leader = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
    {
      send_to_char(ch, "%s", CONFIG_NOPERSON);
      return;
    }
  }
  else
  {
    send_to_char(ch, "Whom do you wish to follow?\r\n");
    return;
  }

  if (ch->master == leader)
  {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }

  if (leader->master != NULL)
  {
    send_to_char(ch, "They are already following someone else.");
    return;
  }

  if ((GET_LEVEL(ch) + 15) < GET_LEVEL(leader) || (GET_LEVEL(ch) - 15) > GET_LEVEL(leader))
  {
    act("You're levels are too far apart.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }

  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master))
  {
    act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
  }
  else
  {                     // Not Charmed follow person
    if (leader == ch)
    {
      if (!ch->master)
      {
        send_to_char(ch, "You are already following yourself.\r\n");
        return;
      }
      stop_follower(ch);
    }
    else
    {
      if (circle_follow(ch, leader))
      {
        send_to_char(ch, "Sorry, but following in loops is not allowed.\r\n");
        return;
      }
      if (ch->master)
        stop_follower(ch);

          for (f = leader->followers; f; f = f->next)
            num_followin++;


          if (num_followin < MAX_FORM_POSITIONS)
          {
           REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
           add_follower(ch, leader);
           perform_group(leader, ch);
           perform_group(ch, leader);
           check_form_type(leader);

           for (f = leader->followers; f; f = f->next)
           {
             log("Follower = %s", GET_NAME(f->follower));
           }

           for (i = 0; i < MAX_FORM_POSITIONS; i++)
           {
             
               if (GET_FORM_POS(leader) == i)
               {
                 continue;
               }

               for (f = leader->followers; f; f = f->next)
               {
		if (GET_FORM_POS(f->follower) == i)
                  spotopen = FALSE;  
               }

              if(spotopen)
                GET_FORM_POS(ch) = i;
           }
          }
          else
           send_to_char(ch, "The formation is already full, sorry.");
   return;
  }
 }
}


ACMD(do_mount) 
{
   char arg[MAX_INPUT_LENGTH];
   struct char_data *vict;
   
   one_argument(argument, arg);
   
   if (!arg || !*arg) {
     send_to_char(ch, "Mount who?\r\n");
     return;
   } else if (!(vict = get_char_room_vis(ch, arg, NULL))) {
     send_to_char(ch, "There is no-one by that name here.\r\n");
     return;
   } else if (!IS_NPC(vict) && GET_LEVEL(ch) <= LVL_IMMORT) {
     send_to_char(ch, "Ehh... no.\r\n");
     return;
   } else if (RIDING(ch) || RIDDEN_BY(ch)) {
     send_to_char(ch, "You are already mounted.\r\n");
     return;
   } else if (RIDING(vict) || RIDDEN_BY(vict)) {
     send_to_char(ch, "It is already mounted.\r\n");
     return;
   } else if (GET_LEVEL(ch) <= LVL_IMMORT && IS_NPC(vict) && !MOB_FLAGGED(vict, MOB_MOUNTABLE)) {
     send_to_char(ch, "You can't mount that!\r\n");
     return;
   } else if (!GET_SKILL(ch, SKILL_MOUNT)) {
     send_to_char(ch, "First you need to learn *how* to mount.\r\n");
     return;
   } else if (GET_SKILL(ch, SKILL_MOUNT) <= rand_number(1, 101)) {
     act("You try to mount $N, but slip and fall off.", FALSE, ch, 0, vict, TO_CHAR);
     act("$n tries to mount you, but slips and falls off.", FALSE, ch, 0, vict, TO_VICT);
     act("$n tries to mount $N, but slips and falls off.", TRUE, ch, 0, vict, TO_NOTVICT);
     damage(ch, ch, dice(1, 2), -1);
     return;
   }
   
   act("You mount $N.", FALSE, ch, 0, vict, TO_CHAR);
   act("$n mounts you.", FALSE, ch, 0, vict, TO_VICT);
   act("$n mounts $N.", TRUE, ch, 0, vict, TO_NOTVICT);
   mount_char(ch, vict);
   
   if (IS_NPC(vict) && !AFF_FLAGGED(vict, AFF_TAMED) && !MOB_FLAGGED(vict, MOB_MOUNTABLE)) {
     act("$N suddenly bucks upwards, throwing you violently to the ground!", FALSE, ch, 0, vict, TO_CHAR);
     act("$n is thrown to the ground as $N violently bucks!", TRUE, ch, 0, vict, TO_NOTVICT);
     act("You buck violently and throw $n to the ground.", FALSE, ch, 0, vict, TO_VICT);
     dismount_char(ch);
     damage(ch, ch, dice(1,3), -1);
   }
 }
 
ACMD(do_dismount) {
   if (!RIDING(ch)) {
     send_to_char(ch, "You aren't even riding anything.\r\n");
     return;
   } else if (SECT(ch->in_room) == SECT_WATER_NOSWIM && !has_boat(ch)) {
     send_to_char(ch, "Yah, right, and then drown...\r\n");
     return;
   } else if (ch->dragonflight) {
     send_to_char(ch, "Dismounting in mid-air may not be the brightest thing you've ever done!\r\n");
     return;
   }
   
   act("You dismount $N.", FALSE, ch, 0, RIDING(ch), TO_CHAR);
   act("$n dismounts from you.", FALSE, ch, 0, RIDING(ch), TO_VICT);
   act("$n dismounts $N.", TRUE, ch, 0, RIDING(ch), TO_NOTVICT);
   dismount_char(ch);
 }
 
 
 ACMD(do_buck) {
   if (!RIDDEN_BY(ch)) {
     send_to_char(ch, "You're not even being ridden!\r\n");
     return;
   } else if (AFF_FLAGGED(ch, AFF_TAMED)) {
     send_to_char(ch, "But you're tamed!\r\n");
     return;
   }
   
   act("You quickly buck, throwing $N to the ground.", FALSE, ch, 0, RIDDEN_BY(ch), TO_CHAR);
   act("$n quickly bucks, throwing you to the ground.", FALSE, ch, 0, RIDDEN_BY(ch), TO_VICT);
   act("$n quickly bucks, throwing $N to the ground.", FALSE, ch, 0, RIDDEN_BY(ch), TO_NOTVICT);
   GET_POS(RIDDEN_BY(ch)) = POS_SITTING;
   dismount_char(ch);
   
   if (rand_number(0, 4)) {
     send_to_char(ch, "You hit the ground hard!\r\n");
     damage(ch, ch, dice(2,4), -1);
   }
   
   /* you might want to call set_fighting() or some non-sense here if you
 *     want the mount to attack the unseated rider or vice-versa.*/
 }
 
 
 ACMD(do_tame) {
   char arg[MAX_INPUT_LENGTH];
   struct affected_type af;
   struct char_data *vict;
   
   one_argument(argument, arg);
   
   if (!arg || !*arg) {
     send_to_char(ch, "Tame who?\r\n");
     return;
   } else if (!(vict = get_char_room_vis(ch, arg, NULL))) {
     send_to_char(ch, "They're not here.\r\n");
     return;
   } else if (GET_LEVEL(ch) <= LVL_IMMORT && IS_NPC(vict) && !MOB_FLAGGED(vict, MOB_MOUNTABLE)) {
     send_to_char(ch, "You can't do that to them.\r\n");
     return;
   } else if (!GET_SKILL(ch, SKILL_TAME)) {
     send_to_char(ch, "You don't even know how to tame something.\r\n");
     return;
   } else if (!IS_NPC(vict) && GET_LEVEL(ch) <= LVL_IMMORT) {
     send_to_char(ch, "You can't do that.\r\n");
     return;
   } else if (GET_SKILL(ch, SKILL_TAME) <= rand_number(1, 101)) {
     send_to_char(ch, "You fail to tame it.\r\n");
     return;
   }
   
   af.type = SKILL_TAME;
   af.duration = 24;
   af.modifier = 0;
   af.location = APPLY_NONE;
   af.bitvector = AFF_TAMED;
   affect_join(vict, &af, FALSE, FALSE, FALSE, FALSE);
   
   act("You tame $N.", FALSE, ch, 0, vict, TO_CHAR);
   act("$n tames you.", FALSE, ch, 0, vict, TO_VICT);
   act("$n tames $N.", FALSE, ch, 0, vict, TO_NOTVICT);
   do_mbind(ch, vict);

 }

void dtmessage(struct char_data *ch)
{
   switch(ch->in_room)
   {
     default:
       send_to_char(ch, "&RYou have fallen into a death trap!\r\n");
   }
}
