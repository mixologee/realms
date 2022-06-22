// This file contains all of the mob combat and noncombat spellcasting, some general interaction with the world.
//-slurk


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

ACMD(do_kick);
void do_maul(struct char_data *ch, struct char_data *vict);
void do_mob_kick(struct char_data *ch, struct char_data *vict);
void do_mob_disarm(struct char_data *ch, struct char_data *vict);
void do_mob_bash(struct char_data *ch, struct char_data *vict);
void npc_steal(struct char_data *ch, struct char_data *victim);
void do_generic_skill(struct char_data *ch, struct char_data *vict, int type);
int compute_armor_class(struct char_data *ch);
int mag_savingthrow(struct char_data *ch, int type, int modifier);

/* noncombat functions */
void undead_noncombat(struct char_data *ch);
void humanoid_noncombat(struct char_data *ch);
void animal_noncombat(struct char_data *ch);
void generic_noncombat(struct char_data *ch);
void giant_noncombat(struct char_data *ch);
void elemental_noncombat(struct char_data *ch);
void magic_user_noncombat(struct char_data *ch);
void cleric_noncombat(struct char_data *ch);
void thief_noncombat(struct char_data *ch);
void warrior_noncombat(struct char_data *ch);
void ranger_noncombat(struct char_data *ch);
void knight_noncombat(struct char_data *ch);
void paladin_noncombat(struct char_data *ch);
void rogue_noncombat(struct char_data *ch);
void assassin_noncombat(struct char_data *ch);
void ninja_noncombat(struct char_data *ch);
void mystic_noncombat(struct char_data *ch);
void sorcerer_noncombat(struct char_data *ch);
void priest_noncombat(struct char_data *ch);
void necro_noncombat(struct char_data *ch);
void elementalist_noncombat(struct char_data *ch);
void shaman_noncombat(struct char_data *ch);
void blackdragon_noncombat(struct char_data *ch);
void bluedragon_noncombat(struct char_data *ch);
void reddragon_noncombat(struct char_data *ch);
void greendragon_noncombat(struct char_data *ch);
void golddragon_noncombat(struct char_data *ch);
void randomdragon_noncombat(struct char_data *ch);
void plant_noncombat(struct char_data *ch);

/* combat functions */
void undead_combat(struct char_data *ch);
void humanoid_combat(struct char_data *ch);
void animal_combat(struct char_data *ch);
void generic_combat(struct char_data *ch);
void giant_combat(struct char_data *ch);
void elemental_combat(struct char_data *ch);
void magic_user_combat(struct char_data *ch);
void cleric_combat(struct char_data *ch);
void thief_combat(struct char_data *ch);          
void warrior_combat(struct char_data *ch);
void ranger_combat(struct char_data *ch);
void knight_combat(struct char_data *ch);
void paladin_combat(struct char_data *ch);
void rogue_combat(struct char_data *ch);
void assassin_combat(struct char_data *ch);
void ninja_combat(struct char_data *ch);
void mystic_combat(struct char_data *ch);
void sorcerer_combat(struct char_data *ch);
void priest_combat(struct char_data *ch);
void necro_combat(struct char_data *ch);
void elementalist_combat(struct char_data *ch);
void shaman_combat(struct char_data *ch);
void blackdragon_combat(struct char_data *ch);
void bluedragon_combat(struct char_data *ch);
void reddragon_combat(struct char_data *ch);
void greendragon_combat(struct char_data *ch);
void golddragon_combat(struct char_data *ch);
void randomdragon_combat(struct char_data *ch);
void plant_combat(struct char_data *ch);

/* create mob intelligence */
void noncombat_intel(void)
{
  struct char_data *mob = NULL;
  struct char_data *next_mob = NULL;
 
  for (mob = character_list; mob; mob = mob->next) {
//   next_mob = mob->next;

   if (!IS_NPC(mob))
     continue;

   if (mob->char_specials.fighting)
     continue;

   if (IN_ROOM(mob) < 1)
     continue;

   if (GET_POS(mob) != POS_STANDING)
     continue;

   if (MOB_FLAGGED(mob, MOB_SPEC))
     continue;
  
   if (rand_number(1,100) > 10)
     continue;

   switch(GET_MCLASS(mob)) {
      case 0:  // HUMANOID
//       if (rand_number(1,100) < 80) // this is to stop spammy mobs from talkin shit all day
       humanoid_noncombat(mob);
       break;       
      case 1:  // UNDEAD
       undead_noncombat(mob);
        break;
      case 2:  // ANIMAL
       animal_noncombat(mob);
       break;
      case 3:  // GENERIC
       generic_noncombat(mob);
       break;
      case 4:  // GIANT
        giant_noncombat(mob);
        break;
      case 5:  // MAGIC USER
        magic_user_noncombat(mob);
        break;
      case 6:  // CLERIC
        cleric_noncombat(mob);
        break;
      case 7:  // THIEF
        thief_noncombat(mob);
        break;
      case 8:  // WARRIOR
        warrior_noncombat(mob);
        break;
      case 9:  // RANGER
        ranger_noncombat(mob);
        break;
      case 10: // KNIGHT
        knight_noncombat(mob);
        break;
      case 11: // PALADIN
        paladin_noncombat(mob);
        break;
      case 12: // ROGUE
        rogue_noncombat(mob);
        break;
      case 13: // ASSASSIN
        assassin_noncombat(mob);
        break;
      case 14: // NINJA
        ninja_noncombat(mob);
        break;
      case 15: // MYSTIC
        mystic_noncombat(mob);
        break;
      case 16: // SHAMAN
        shaman_noncombat(mob);
        break;
      case 17: // PRIEST
        priest_noncombat(mob);
        break;
      case 18: // NECROMANCER
        necro_noncombat(mob);
        break;
      case 19: // SORCERER
        sorcerer_noncombat(mob);
        break;   
      case 20: // ELEMENTALIST
        elementalist_noncombat(mob);
        break;
      case 21: // ELEMENTAL
        elemental_noncombat(mob);
        break;
      case 22: // BLACKDRAGON
        blackdragon_noncombat(mob);
        break;
      case 23: // BLUEDRAGON
        bluedragon_noncombat(mob);
        break;
      case 24: // REDDRAGON
        reddragon_noncombat(mob);
        break;
      case 25: // GREENDRAGON
        greendragon_noncombat(mob);
        break;
      case 26: // GOLDDRAGON
        golddragon_noncombat(mob);
        break;
      case 27: // RAND_DRAGON
        randomdragon_noncombat(mob);
        break;
      case 28:
        plant_noncombat(mob);
        break;
      default:
       break;
    }//close switch
  }//close for
}//close func

/* List all non-combat functions and give them shit to do while they aren't fighting */

void undead_noncombat(struct char_data *ch)
{
act("$n moans in agony.", FALSE, ch, 0, 0, TO_ROOM);
return;
}

void humanoid_noncombat(struct char_data *ch)
{}
void animal_noncombat(struct char_data *ch)
{}
void generic_noncombat(struct char_data *ch)
{}
void giant_noncombat(struct char_data *ch)
{}
void elemental_noncombat(struct char_data *ch)
{}
void plant_noncombat(struct char_data *ch)
{}
void magic_user_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_INVISIBLE))
        cast_spell(ch, ch, NULL, SPELL_INVISIBLE);
      break;      
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      if(!AFF_FLAGGED(ch, AFF_MANASHIELD))
        cast_spell(ch, ch, NULL, SPELL_MANASHIELD);
      break;
    case 4:
      if(GET_MANA(ch) < (GET_MAX_MANA(ch)/4))
        cast_spell(ch, ch, NULL, SPELL_LIFETAP);
      break;
    default:
      break;
  }

 return;
}
void cleric_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_BLIND))
        cast_spell(ch, ch, NULL, SPELL_CURE_BLIND);
      break;
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      cast_spell(ch, ch, NULL, SPELL_BLESS);
      break;
    case 4:
      if(GET_HIT(ch) < (GET_MAX_HIT(ch)))
        cast_spell(ch, ch, NULL, SPELL_HEAL);
      break;
    case 5:
      if(!AFF_FLAGGED(ch, AFF_CURSE))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_CURSE);
      break;
    case 6:
      if(!AFF_FLAGGED(ch, AFF_POISON))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_POISON);
      break;
    case 7:
      if(!AFF_FLAGGED(ch, AFF_SANCTUARY))
        cast_spell(ch, ch, NULL, SPELL_SANCTUARY);
      break;
    default:
      break;
  }

 return;

}

void thief_noncombat(struct char_data *ch)
{
  struct char_data *cons;

  for (cons = world[IN_ROOM(ch)].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && GET_LEVEL(cons) < LVL_IMMORT && !rand_number(0, 4)) {
      npc_steal(ch, cons);
      return;
    }

  return;
}

void warrior_noncombat(struct char_data *ch)
{
  act("$n sharpens $S weapon in preparation for the next battle.", FALSE, ch, 0, 0, TO_ROOM);
  return;
}
void ranger_noncombat(struct char_data *ch)
{
  act("$n sharpens $S weapon in preparation for the next battle.", FALSE, ch, 0, 0, TO_ROOM);
  return;
}
void knight_noncombat(struct char_data *ch)
{
  act("$n sharpens $S weapon in preparation for the next battle.", FALSE, ch, 0, 0, TO_ROOM);
  return;
}
void paladin_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_PROTECT_GOOD))
        cast_spell(ch, ch, NULL, SPELL_PROT_FROM_GOOD);
      break;
    case 3:
      if(!AFF_FLAGGED(ch, AFF_PROTECT_EVIL))
        cast_spell(ch, ch, NULL, SPELL_PROT_FROM_EVIL);
      break;
    case 4:
      if(!AFF_FLAGGED(ch, AFF_MELEEHEAL))
        cast_spell(ch, ch, NULL, SPELL_MELEEHEAL);
      break;
    default:
      break;
  }

 return;

}
void rogue_noncombat(struct char_data *ch)
{
  struct char_data *cons;

  for (cons = world[IN_ROOM(ch)].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && GET_LEVEL(cons) < LVL_IMMORT && !rand_number(0, 4)) {
      npc_steal(ch, cons);
      return;
    }

  return;

}
void assassin_noncombat(struct char_data *ch)
{
  struct char_data *cons;

  for (cons = world[IN_ROOM(ch)].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && GET_LEVEL(cons) < LVL_IMMORT && !rand_number(0, 4)) {
      npc_steal(ch, cons);
      return;
    }

  return;

}
void ninja_noncombat(struct char_data *ch)
{
  struct char_data *cons;

  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_CLOAKED_MOVE))
        cast_spell(ch, ch, NULL, SPELL_CLOAKED_MOVE);
      break;
    case 2:
      if(!AFF_FLAGGED(ch, AFF_EVASION))
        cast_spell(ch, ch, NULL, SPELL_EVASION);
      break;
    case 3:
       for (cons = world[IN_ROOM(ch)].people; cons; cons = cons->next_in_room)
         if (!IS_NPC(cons) && GET_LEVEL(cons) < LVL_IMMORT && !rand_number(0, 4)) {
           npc_steal(ch, cons);
          break;
       }
      break;
    default:
      break;
  }

 return;
}
void mystic_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_BLIND))
        cast_spell(ch, ch, NULL, SPELL_CURE_BLIND);
      break;
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      cast_spell(ch, ch, NULL, SPELL_BLESS);
      break;
    case 4:
      if(GET_HIT(ch) < (GET_MAX_HIT(ch)))
        cast_spell(ch, ch, NULL, SPELL_HEAL);
      break;
    case 5:
      if(!AFF_FLAGGED(ch, AFF_CURSE))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_CURSE);
      break;
    case 6:
      if(!AFF_FLAGGED(ch, AFF_POISON))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_POISON);
      break;
    case 7:
      if(!AFF_FLAGGED(ch, AFF_SANCTUARY))
        cast_spell(ch, ch, NULL, SPELL_SANCTUARY);
      break;
    default:
      break;
  }

 return;

}
void sorcerer_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_INVISIBLE))
        cast_spell(ch, ch, NULL, SPELL_INVISIBLE);
      break;
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      if(!AFF_FLAGGED(ch, AFF_MANASHIELD))
        cast_spell(ch, ch, NULL, SPELL_MANASHIELD);
      break;
    case 4:
      if(GET_MANA(ch) < (GET_MAX_MANA(ch)/4))
        cast_spell(ch, ch, NULL, SPELL_LIFETAP);
      break;
    default:
      break;
  }

 return;
}
void priest_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_BLIND))
        cast_spell(ch, ch, NULL, SPELL_CURE_BLIND);
      break;
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      cast_spell(ch, ch, NULL, SPELL_BLESS);
      break;
    case 4:
      if(GET_HIT(ch) < (GET_MAX_HIT(ch)))
        cast_spell(ch, ch, NULL, SPELL_HEAL);
      break;
    case 5:
      if(!AFF_FLAGGED(ch, AFF_CURSE))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_CURSE);
      break;
    case 6:
      if(!AFF_FLAGGED(ch, AFF_POISON))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_POISON);
      break;
    case 7:
      if(!AFF_FLAGGED(ch, AFF_SANCTUARY))
        cast_spell(ch, ch, NULL, SPELL_SANCTUARY);
      break;
    case 8:
      if(!AFF_FLAGGED(ch, AFF_REGEN))
        cast_spell(ch, ch, NULL, SPELL_REGEN);
      break;
    default:
      break;
  }

 return;
}
void necro_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_INVISIBLE))
        cast_spell(ch, ch, NULL, SPELL_INVISIBLE);
      break;
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      if(!AFF_FLAGGED(ch, AFF_MANASHIELD))
        cast_spell(ch, ch, NULL, SPELL_MANASHIELD);
      break;
    case 4:
      if(GET_MANA(ch) < (GET_MAX_MANA(ch)/4))
        cast_spell(ch, ch, NULL, SPELL_LIFETAP);
      break;
    case 5:
      if(!AFF_FLAGGED(ch, AFF_IMAGES))
        cast_spell(ch, ch, NULL, SPELL_CLONE_IMAGES);
      break;
    default:
      break;
  }

 return;
}
void elementalist_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_INVISIBLE))
        cast_spell(ch, ch, NULL, SPELL_INVISIBLE);
      break;
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      if(!AFF_FLAGGED(ch, AFF_MANASHIELD))
        cast_spell(ch, ch, NULL, SPELL_MANASHIELD);
      break;
    case 4:
      if(GET_MANA(ch) < (GET_MAX_MANA(ch)/4))
        cast_spell(ch, ch, NULL, SPELL_LIFETAP);
      break;
    case 5:
      if(!AFF_FLAGGED(ch, AFF_DEATH_FIELD))
        cast_spell(ch, ch, NULL, SPELL_DEATH_FIELD);
      break;
    default:
      break;
  }

 return;
}
void shaman_noncombat(struct char_data *ch)
{
  switch(rand_number(0,10))
  {
    case 1:
      if(!AFF_FLAGGED(ch, AFF_BLIND))
        cast_spell(ch, ch, NULL, SPELL_CURE_BLIND);
      break;
    case 2:
      cast_spell(ch, ch, NULL, SPELL_ARMOR);
      break;
    case 3:
      cast_spell(ch, ch, NULL, SPELL_BLESS);
      break;
    case 4:
      if(GET_HIT(ch) < (GET_MAX_HIT(ch)))
        cast_spell(ch, ch, NULL, SPELL_HEAL);
      break;
    case 5:
      if(!AFF_FLAGGED(ch, AFF_CURSE))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_CURSE);
      break;
    case 6:
      if(!AFF_FLAGGED(ch, AFF_POISON))
        cast_spell(ch, ch, NULL, SPELL_REMOVE_POISON);
      break;
    case 7:
      if(!AFF_FLAGGED(ch, AFF_SANCTUARY))
        cast_spell(ch, ch, NULL, SPELL_SANCTUARY);
      break;
    default:
      break;
  }

 return;
}
void blackdragon_noncombat(struct char_data *ch)
{}
void bluedragon_noncombat(struct char_data *ch)
{}
void reddragon_noncombat(struct char_data *ch)
{}
void greendragon_noncombat(struct char_data *ch)
{}
void golddragon_noncombat(struct char_data *ch)
{}
void randomdragon_noncombat(struct char_data *ch)
{}


void combat_intel(struct char_data *mob)
{

/*  for (mob = character_list; mob; mob = next_mob) {
   next_mob = mob->next;
*/
   if (!IS_NPC(mob))
     return;

   if (IN_ROOM(mob) < 1)
     return;

   if (GET_POS(mob) < POS_FIGHTING)
     return;

   if (MOB_FLAGGED(mob, MOB_SPEC))
     return;

   switch(GET_MCLASS(mob)) {
      case 0:  /* HUMANOID
//       if (rand_number(1,100) < 80)  this is to stop spammy mobs from talkin shit all day */
       humanoid_combat(mob);
       break;
      case 1:  /* UNDEAD */
       undead_combat(mob);
       break;
      case 2:  /* ANIMAL */
       animal_combat(mob);
       break;
      case 3:  /* GENERIC */
       generic_combat(mob);
       break;
      case 4:  /* GIANT */
        giant_combat(mob);
        break;
      case 5:  /* MAGIC USER */
        magic_user_combat(mob);
        break;
      case 6:  /* CLERIC */
        cleric_combat(mob);
        break;
      case 7:  /* THIEF */
        thief_combat(mob);
        break;
      case 8:  /* WARRIOR */
        warrior_combat(mob);
        break;
      case 9:  /* RANGER*/
        ranger_combat(mob);
        break;
      case 10: /* KNIGHT*/
        knight_combat(mob);
        break;
      case 11: /* PALADIN*/
        paladin_combat(mob);
        break;
      case 12: /* ROGUE*/
        rogue_combat(mob);
        break;
      case 13: /* ASSASSIN*/
        assassin_combat(mob);
        break;
      case 14: /* NINJA*/
        ninja_combat(mob);
        break;
      case 15: /* MYSTIC */
        mystic_combat(mob);
        break;
      case 16: /* SHAMAN */
        shaman_combat(mob);
        break;
      case 17: /* PRIEST*/
        priest_combat(mob);
        break;
      case 18: /* NECROMANCER*/
        necro_combat(mob);
        break;
      case 19: /* SORCERER*/
        sorcerer_combat(mob);
        break;
      case 20: /* ELEMENTALIST */
        elementalist_combat(mob);
        break;
      case 21: /* ELEMENTAL*/
        elemental_combat(mob);
        break;
      case 22: /* BLACKDRAGON*/
        blackdragon_combat(mob);
        break;
      case 23: /* BLUEDRAGON*/
        bluedragon_combat(mob);
        break;
      case 24: /* REDDRAGON*/
        reddragon_combat(mob);
        break;
      case 25: /* GREENDRAGON*/
        greendragon_combat(mob);
        break;
      case 26: /* GOLDDRAGON*/
        golddragon_combat(mob);
        break;
      case 27: /* RAND_DRAGON*/
        randomdragon_combat(mob);
        break;
      case 28:
        plant_combat(mob);
        break;
      default:
        log("mob class not found");
        break;
    }/*close switch*/
//  }/*close for*/
return;
}/*close func*/

/* List out all combat functions, use this to make them cast and use skills vs enemies */

void undead_combat(struct char_data *ch) 
{
struct char_data *m=NULL;

    if (!ch->char_specials.fighting)
      return;

    if (rand_number(1, 130) > 100)
      return;
    
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
     m = FIGHTING(ch);   
     return;
    }
 
//    m=pick_victim(ch->char_specials.fighting); // pick victim was another function i removed cause of it using form 
						 // that isnt complete
    do_generic_skill(ch, m, SKILL_KICK);						
//    do_mob_kick(ch, m);

act("$n says 'WooZAHHHHHHHHH'", FALSE, ch, 0, 0, TO_ROOM);
return;
}

void humanoid_combat(struct char_data *ch) 
{}
void animal_combat(struct char_data *ch)  
{}
void generic_combat(struct char_data *ch)  
{}
void giant_combat(struct char_data *ch)  
{}
void elemental_combat(struct char_data *ch)  
{}
void plant_combat(struct char_data *ch)
{}

void magic_user_combat(struct char_data *ch)  
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !rand_number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
    return;

  if (GET_LEVEL(ch) > 13 && rand_number(0, 10) == 0)
    cast_spell(ch, vict, NULL, SPELL_POISON);

  if (GET_LEVEL(ch) > 7 && rand_number(0, 8) == 0)
    cast_spell(ch, vict, NULL, SPELL_BLINDNESS);

  if (GET_LEVEL(ch) > 12 && rand_number(0, 12) == 0) {
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }

  if (rand_number(0, 4))
    return;

  switch (GET_LEVEL(ch)) {
    case 4:
    case 5:
      cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
      break;
    case 6:
    case 7:
      cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
      break;
    case 8:
    case 9:
      cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
      break;
    case 10:
    case 11:
      cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
      break;
    case 12:
    case 13:
      cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
      break;
    case 14:
    case 15:
    case 16:
    case 17:
      cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
      break;
    default:
      cast_spell(ch, vict, NULL, SPELL_FIREBALL);
      break;
  }
  return;
}

void cleric_combat(struct char_data *ch)  
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;
  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);
  /* Hm...didn't pick anyone...I'll wait a round. */
  if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 10)) {
     cast_spell(ch, ch, NULL, SPELL_HEAL);
     return;
  }

  switch (GET_LEVEL(ch)) {
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_AURA_BOLT);
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_MANA_BLAST);
    cast_spell(vict, vict, NULL, SPELL_HEAL);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_HARM);
    break;
  }
  return;
}

void thief_combat(struct char_data *ch)  
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
      case 1: case 2: case 3: case 4:
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      }

    }
  }
  return;

}

void warrior_combat(struct char_data *ch)  
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
       case 1: case 2: case 3: case 4:
         /* bash */
         do_mob_bash(ch, vict);
        break;
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      }

    }
  }
  return;
}

void ranger_combat(struct char_data *ch)  
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
       case 1: case 2: case 3: case 4:
         /* bash */
         do_mob_bash(ch, vict);
        break;
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      }

    }
  }
  return;
}
void knight_combat(struct char_data *ch)  
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
       case 1: case 2: case 3: case 4:
         /* bash */
         do_mob_bash(ch, vict);
        break;
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      }

    }
  }
  return;
}
void paladin_combat(struct char_data *ch)  
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
       case 1: case 2: case 3: case 4:
         /* bash */
         do_mob_bash(ch, vict);
        break;
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_BASH);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_KICK);
        break;
      }

    }
  }
  return;
}
void rogue_combat(struct char_data *ch)  
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
      case 1: case 2: case 3: case 4:
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      }

    }
  }
  return;
}
void assassin_combat(struct char_data *ch)
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
      case 1: case 2: case 3: case 4:
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      }

    }
  }
  return;
}
void ninja_combat(struct char_data *ch)
{
  struct char_data *vict;
  int att_type = 0;

  if (!AWAKE(ch))
    return;

  /* if two people are fighting in a room */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

      vict = FIGHTING(ch);

      if (rand_number(1, 5) == 5) {
       act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0, TO_ROOM);
      }
    if (GET_POS(ch) == POS_FIGHTING) {

      att_type = rand_number(1,40);

      switch(att_type) {
      case 1: case 2: case 3: case 4:
      case 5: case 6: case 7: case 8: case 9: case 10:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;

      case 11:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 12: case 13:
          do_mob_disarm(ch, vict);
        break;
      case 14: case 15: case 16:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      case 17:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      default:
        do_generic_skill(ch, vict, SKILL_TRIP);
        break;
      }
    }
  }
  return;
}
void mystic_combat(struct char_data *ch)
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;
  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);
    /* Hm...didn't pick anyone...I'll wait a round. */
  if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 10)) {
     cast_spell(ch, ch, NULL, SPELL_HEAL);
     return;
  }

  switch (rand_number(1, GET_LEVEL(ch))) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
    cast_spell(ch, vict, NULL, SPELL_AURA_BOLT);
    break;
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    cast_spell(ch, vict, NULL, SPELL_MANA_BLAST);
    cast_spell(ch, ch, NULL, SPELL_HEAL);
    break;
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  default:
    cast_spell(ch, vict, NULL, SPELL_HARM);
    break;
  }
  return;

}
void sorcerer_combat(struct char_data *ch)
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;
  /* if I didn't pick any of those, then just slam the guy I'm fighting 
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch)) {
    vict = FIGHTING(ch);
 //   return;
  }*/

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */

  switch (rand_number(1, GET_LEVEL(ch))) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL);
    break;
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
    cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN);
    break;
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
    cast_spell(ch, vict, NULL, SPELL_BLIZZARD);
  default:
    cast_spell(ch, vict, NULL, SPELL_SHOCKSTORM);
    break;
  }
  return;

}
void priest_combat(struct char_data *ch)
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;
  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);
  /* Hm...didn't pick anyone...I'll wait a round. */
  if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 10)) {
     cast_spell(ch, ch, NULL, SPELL_HEAL);
     return;
  }

  switch (rand_number(1, GET_LEVEL(ch))) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
    cast_spell(ch, vict, NULL, SPELL_AURA_BOLT);
    break;
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    cast_spell(ch, vict, NULL, SPELL_MANA_BLAST);
    cast_spell(vict, vict, NULL, SPELL_HEAL);
    break;
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  default:
    cast_spell(ch, vict, NULL, SPELL_HARM);
    break;
  }
  return;

}
void necro_combat(struct char_data *ch)
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;
  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);
  /* Hm...didn't pick anyone...I'll wait a round. */

  switch (rand_number(1, GET_LEVEL(ch))) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_CONFUSION);
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    cast_spell(ch, vict, NULL, SPELL_DRAIN_LIFE);
    break;
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  default:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL);
    break;
  }
  return;
}

void elementalist_combat(struct char_data *ch)
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;
  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);

  
  switch (rand_number(1, GET_LEVEL(ch))) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    cast_spell(ch, FIGHTING(ch), NULL, SPELL_CONJURE_ELEMENTAL);
    break;
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
    if(rand_number(0,10) > 5)
      cast_spell(ch, FIGHTING(ch), NULL, SPELL_SUMMON_CELESTIAL);
    else
      cast_spell(ch, FIGHTING(ch), NULL, SPELL_SUMMON_FIEND);
    break;
  default:
    cast_spell(ch, FIGHTING(ch), NULL, SPELL_CONJURE_FAMILIAR);
    break;
  }
  return;

}

void shaman_combat(struct char_data *ch)
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)
    return;
  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);
  /* Hm...didn't pick anyone...I'll wait a round. */
  if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 10)) {
     cast_spell(ch, ch, NULL, SPELL_HEAL);
     return;
  }

  switch (rand_number(1, GET_LEVEL(ch))) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
    cast_spell(ch, vict, NULL, SPELL_HAMMER_OF_DIVINITY);
    cast_spell(vict, vict, NULL, SPELL_HEAL);
    break;
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  default:
    cast_spell(ch, vict, NULL, SPELL_STARFIRE);
    break;
  }
  return;
}
void blackdragon_combat(struct char_data *ch)
{}
void bluedragon_combat(struct char_data *ch)
{}
void reddragon_combat(struct char_data *ch)
{}
void greendragon_combat(struct char_data *ch)
{}
void golddragon_combat(struct char_data *ch)
{
  struct char_data *vict;

  if (GET_POS(ch) != POS_FIGHTING)                 
    return;   

  /* if I didn't pick any of those, then just slam the guy I'm fighting */

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room))
      vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 10)) {
     cast_spell(ch, ch, NULL, SPELL_HEAL);
     return;
  }

  switch (rand_number(0,50)) {
  case  0:
     act("&C$n roars in anger.", FALSE, ch, 0, 0, TO_ROOM);
  case 20:
        do_maul(ch, vict);
//        do_generic_skill(ch, vict, SKILL_MAUL);
  case 40:
    if (rand_number(0,20) == 14)
      cast_spell(ch, vict, NULL, SPELL_FIRE_BREATH);
  default:  
//    cast_spell(ch, vict, NULL, SPELL_FIRE_BREATH);
    break;  
  }
  return;  
}
void randomdragon_combat(struct char_data *ch)
{}


void do_maul(struct char_data *ch, struct char_data *vict)
{
  int mob_perc, mob_prob;
  mob_perc = mob_prob = 0;

  mob_prob = rand_number(1,100) + GET_STR(ch);
  mob_perc = (100 - (int)(100 * GET_LEVEL(ch) / 250));
  
  if (GET_LEVEL(vict) > LVL_IMMORT)
    mob_prob = 0;

  if (mob_prob > mob_perc) 
    damage(ch, vict, 0, SKILL_MAUL);
  else {
    damage(ch, vict, GET_LEVEL(ch), SKILL_MAUL);
    WAIT_STATE(ch, PULSE_VIOLENCE * rand_number(0,3));
  }

}
