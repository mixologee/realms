/* ************************************************************************
*   File: range.c                                      
*  Usage: player commands for ranged combat                               *
*                                                                         *
************************************************************************ */

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

int skill_roll(struct char_data *ch, int skill_num);
void strike_missile(struct char_data *ch, struct char_data *tch,
                    struct obj_data *missile, int dir, int attacktype);
void miss_missile(struct char_data *ch, struct char_data *tch,
                  struct obj_data *missile, int dir, int attacktype);
void mob_reaction(struct char_data *ch, struct char_data *vict, int dir);
void fire_missile(struct char_data *ch, char arg1[MAX_INPUT_LENGTH],
                  struct obj_data *missile, int pos, int range, int dir);

void perform_wear(struct char_data *ch, struct obj_data *obj, int where);

/* extern variables */
extern int pk_allowed;
extern const char *dirs[];
extern int rev_dir[];
ACMD(do_flee);

 ACMD(do_shoot)
{
   struct obj_data *missile;
   char arg2[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   int dir, range;

   if (!GET_EQ(ch, WEAR_WIELD)) {
     send_to_char(ch, "You aren't wielding a shooting weapon!\r\n");
     return;
   }

   if (!GET_EQ(ch, WEAR_HOLD)) {
     send_to_char(ch, "You need to be holding a missile!\r\n");
     return;
   }

   if (IS_NPC(ch) || (!GET_SKILL(ch, SKILL_RANGE_COMBAT) && (GET_WAS_WARRIOR(ch) == 0))) {
	send_to_char(ch, "You have no idea how to do that.\r\n");
	return;
  }

   missile = GET_EQ(ch, WEAR_HOLD);

   if ((GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_SLING) &&
       (GET_OBJ_TYPE(missile) == ITEM_ROCK))
        range = GET_EQ(ch, WEAR_WIELD)->obj_flags.value[0];
   else
     if ((GET_OBJ_TYPE(missile) == ITEM_ARROW) &&
         (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_BOW))
          range = GET_EQ(ch, WEAR_WIELD)->obj_flags.value[0];
   else
     if ((GET_OBJ_TYPE(missile) == ITEM_BOLT) &&
         (GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_CROSSBOW))
          range = GET_EQ(ch, WEAR_WIELD)->obj_flags.value[0];

   else {
     send_to_char(ch, "You should wield a missile weapon and hold a missile!\r\n");
     return;
   }

   two_arguments(argument, arg1, arg2);

   if (!*arg1 || !*arg2) {
     send_to_char(ch, "You should try: shoot <someone> <direction>\r\n");
     return;
   }

  if (IS_DARK(ch->in_room)) {
     send_to_char(ch, "You can't see that far.\r\n");
     return;
   } 

   if ((dir = search_block(arg2, dirs, FALSE)) < 0) {
     send_to_char(ch, "What direction?\r\n");
     return;
   }

//   if (!CAN_GO(ch, dir)) {
//     send_to_char(ch, "Something blocks the way!\r\n");
//     return;
//   }

   if (range > 3)
     range = 3;
   if (range < 1)
     range = 1;

   if (CAN_GO(ch, dir))
     fire_missile(ch, arg1, missile, WEAR_HOLD, range, dir);
   else
   {
     send_to_char(ch, "Something blocks the way!\r\n");
     return;
   }
}


int skill_roll(struct char_data *ch, int skill_num)
{
   if (rand_number(0, 101) > GET_SKILL(ch, skill_num))
     return FALSE;
   else
     return TRUE;
}


void strike_missile(struct char_data *ch, struct char_data *tch,
                    struct obj_data *missile, int dir, int attacktype)
{
   long int dam;
   extern struct str_app_type str_app[];
   char buf[MAX_STRING_LENGTH];

   dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
   dam += dice(missile->obj_flags.value[1],
               missile->obj_flags.value[2]);
   dam += GET_DAMROLL(ch);

   damage(ch, tch, dam, attacktype);

   send_to_char(ch, "You hit!\r\n");
   sprintf(buf, "$P flies in from the %s and strikes %s.",
           dirs[rev_dir[dir]], GET_NAME(tch));
   act(buf, FALSE, tch, 0, missile, TO_ROOM);
   sprintf(buf, "$P flies in from the %s and hits YOU! %s was the culprit!",
                 dirs[rev_dir[dir]], GET_NAME(ch));
   act(buf, FALSE, tch, 0, missile, TO_CHAR);
   return;
}

void miss_missile(struct char_data *ch, struct char_data *tch,
                 struct obj_data *missile, int dir, int attacktype)
{
   char buf[MAX_STRING_LENGTH];

   sprintf(buf, "$P flies in from the %s and hits the ground! %s needs to work on their aim!",
                 dirs[rev_dir[dir]], GET_NAME(ch));
   act(buf, FALSE, tch, 0, missile, TO_ROOM);
   act(buf, FALSE, tch, 0, missile, TO_CHAR);
   send_to_char(ch, "You missed!\r\n");
}


void mob_reaction(struct char_data *ch, struct char_data *vict, int dir)
{
   char buf[MAX_STRING_LENGTH];

   if (IS_NPC(vict) && (!FIGHTING(vict)) && (GET_POS(vict) > POS_STUNNED))
   {   
      /* can remember so charge! */
     if (IS_SET_AR(MOB_FLAGS(vict), MOB_MEMORY)) {
       remember(vict, ch);
       sprintf(buf, "$n bellows in pain!");
       act(buf, FALSE, vict, 0, 0, TO_ROOM);
       if (GET_POS(vict) == POS_STANDING) {
         if (!do_simple_move(vict, rev_dir[dir], 1))
           act("$n stumbles while trying to run!", FALSE, vict, 0, 0, TO_ROOM);
       } else
       GET_POS(vict) = POS_STANDING;

     /* can't remember so try to run away */
      }
     } else {
       do_flee(vict, "", 0, 0);
     }
   
}

void fire_missile(struct char_data *ch, char arg1[MAX_INPUT_LENGTH],
                   struct obj_data *missile, int pos, int range, int dir)
{
   bool shot = FALSE, found = FALSE;
   int attacktype;
   long int room, nextroom, distance;
   struct char_data *vict;
   struct obj_data *nextobj;

   if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
     send_to_char(ch, "This room just has such a peaceful, easy feeling...\r\n");
     return;
   }

   room = ch->in_room;

   if (CAN_GO2(room, dir))
     nextroom = EXIT2(room, dir)->to_room;
   else
     nextroom = NOWHERE;

 for (distance = 1; ((nextroom != NOWHERE) && (distance <= range)); distance++) {

     for (vict = world[nextroom].people; vict ; vict= vict->next_in_room) {
       if ((isname(arg1, GET_NAME(vict))) && (CAN_SEE(ch, vict))) {
         found = TRUE;
         break;
       }
     }

     if (found == 1) {

       /* Daniel Houghton's missile modification */
       if (missile && ROOM_FLAGGED(vict->in_room, ROOM_PEACEFUL)) {
         send_to_char(ch, "Nah.  Leave them in peace.\r\n");
         return;
       }
       
       if (missile && !IS_NPC(vict) && !ROOM_FLAGGED(IN_ROOM(vict), ROOM_NPK) && !ROOM_FLAGGED(IN_ROOM(vict), ROOM_CPK)) {
         send_to_char(ch, "&RSorry, but you can only attack players in NPK or CPK areas.&n\r\n");
         return;
       } 

      switch(GET_OBJ_TYPE(missile)) {
         case ITEM_ARROW:
           act("$n draws $M bow and fires!", TRUE, ch, 0, 0, TO_ROOM);
           send_to_char(ch, "You draw your bow and fire!\r\n");
           attacktype = SKILL_RANGE_COMBAT;
           break;
         case ITEM_ROCK:
           act("$n slings a rock at something!", TRUE, ch, 0, 0, TO_ROOM);
           send_to_char(ch, "You sling a rock at something!\r\n");
           attacktype = SKILL_RANGE_COMBAT;
           break;
         case ITEM_BOLT:
           act("$n gets something in the crosshairs of $M crossbow and lets a bolt fly!", TRUE, ch, 0, 0, TO_ROOM);
           send_to_char(ch, "You let a bolt fly form your crossbow!\r\n");
           attacktype = SKILL_RANGE_COMBAT;
           break;
         default:
           attacktype = TYPE_UNDEFINED;
         break;
       }

       if (attacktype != TYPE_UNDEFINED)
         shot = skill_roll(ch, attacktype);
       else
         shot = FALSE;

        if (shot == TRUE) {
         strike_missile(ch, vict, missile, dir, attacktype);
         if (rand_number(0, 1))
           obj_to_char(unequip_char(ch, pos), vict);   
         else 
           extract_obj(unequip_char(ch, pos));        
       } else {
       /* ok missed so move missile into new room */
         miss_missile(ch, vict, missile, dir, attacktype);
         if (!rand_number(0, 2))
           obj_to_room(unequip_char(ch, pos), vict->in_room);
         else
           extract_obj(unequip_char(ch, pos));
       }

       if (!(nextobj = get_obj_in_list_vis(ch, missile->name, NULL, ch->carrying)))
          send_to_char(ch, "You are out of a %s's.\r\n", missile->description);
       else
          perform_wear(ch, nextobj, WEAR_HOLD);

       /* either way mob remembers */
       mob_reaction(ch, vict, dir);
       WAIT_STATE(ch, PULSE_VIOLENCE);
       return;

     }

     room = nextroom;
     if (CAN_GO2(room, dir))
       nextroom = EXIT2(room, dir)->to_room;
     else
       nextroom = NOWHERE;
   }

   send_to_char(ch, "Can't find your target!\r\n");
   return;

}

