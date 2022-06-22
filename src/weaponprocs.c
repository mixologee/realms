
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

void DoEquipmentProc(struct char_data *ch, struct char_data *vict)
{
  int DamAmnt, Duration, Dam, Dice;
  Dam = Duration = 0;
  extern struct index_data *obj_index;
  struct affected_type af;
 
  if (GET_EQ(ch, WEAR_HEAD) && GET_OBJ_VNUM(GET_EQ(ch, WEAR_HEAD)) == 6897) {
    DamAmnt = dice(12,8);
    Dice = dice(10,3);
    Duration = rand_number(15,20);

    if (DamAmnt >= 60 ) {
      if (Dice <= 20) {
        act("&mA wave of energy emerges from the eye pounding $n!", FALSE, vict, 0, 0, TO_ROOM);
        act("&mA wave of energy emerges from the eye pounding you hard!", FALSE, vict, 0, 0, TO_VICT);
        DamAmnt = rand_number(40, 60);
      }                 
      if (Dice >= 21) {
        act("&WA bright light flashes from within the eye blinding $n!", FALSE, vict, 0, 0, TO_ROOM);
        act("&WA bright light flashes from within the eye blinding you!", FALSE, vict, 0, 0, TO_VICT);
        DamAmnt = 0;

       af.type = SPELL_BLINDNESS;
       af.modifier = 0;
       af.location = 0;
       af.duration = Duration;
       af.bitvector = AFF_BLIND;
       affect_from_char(vict, SPELL_BLINDNESS);
       affect_to_char(vict, &af);
      }                 

      GET_HIT(vict) -= DamAmnt;
    }
  }

// add new If statements here for equipment

} // This is the closing bracket!!
  


int DoWeaponProc(struct char_data *ch, struct char_data *vict, int dr)
{
  int DamAmnt, Dam, Dice;
  Dam = 0;
  extern struct index_data *obj_index;
  struct obj_data *weap=NULL;
  struct affected_type af;

  if  (GET_EQ(ch, WEAR_WIELD)) 
  {
    weap = GET_EQ(ch, WEAR_WIELD);

    switch (GET_OBJ_VNUM(weap))
    { 
      case 6898:
        DamAmnt = dice(10,9);
        Dice = dice(10, 3);
  
        if (DamAmnt >= 60) 
	{
          if (Dice <= 20) 
	  { 
            act("&BBolts of electricity appear from $n's dagger striking $N!", FALSE, ch, 0, vict, TO_NOTVICT);
            act("&BBolts of electricity appear from $n's dagger striking you!", FALSE, ch, 0, vict, TO_VICT);
            act("&BBolts of electricity appear from your dagger striking $N!", FALSE, ch, 0, vict, TO_CHAR);
	    GET_HIT(vict) -= DamAmnt;
          }
          if (Dice >= 21) 
	  {
            act("&RWaves of fire spew forth from $n's dagger engulfing $N in flames!", FALSE, ch, 0, vict, TO_NOTVICT);
            act("&RWaves of fire spew forth from $n's dagger engulfing you in flames!", FALSE, ch, 0, vict, TO_VICT);
            act("&RWaves of fire spew forth from your dagger engulfing $N in flames!", FALSE, ch, 0, 0, TO_CHAR);
            GET_HIT(vict) -= DamAmnt;
          }
        }
        break;
 
      case 4638:
	if ((dr - GET_DAMROLL(ch)) <= 10) 
      	{
          act("$n's whip spins around your legs, and $n pulls your legs out from under you!", FALSE, ch, 0, vict, TO_VICT);
	  act("Your whip spins around $N's legs, and you pull $s legs out from under $m!", FALSE, ch, 0, vict, TO_CHAR);
          act("$n's whip spins around $N's legs, and  pulls $s legs out from under $m!", FALSE, ch, 0, vict, TO_NOTVICT);
          dr = MAX(dr, dice(6,6));
        }          
        break;

      case 11251:
        Dice = dice(10, 3);
        DamAmnt = dice(GET_OBJ_LEVEL(weap), 4);

        if (Dice >= 21)
        {
          act("&M$N shivers as $S body is taken hold by a curse from your weapon!", FALSE, ch, 0, vict, TO_CHAR);
          act("&MYou shiver as $n's dagger unleashes a curse upon you!", FALSE, ch, 0, vict, TO_VICT);
          act("&M$N shivers as $S body is taken hold by a curse from $s!", FALSE, ch, 0, vict, TO_NOTVICT);
          GET_HIT(vict) -= DamAmnt;
 
          af.type = SPELL_CURSE;
          af.modifier = 0;
          af.location = 0;
          af.duration = 10;
          af.bitvector = AFF_CURSE;
          affect_to_char(vict, &af);
        }
        break;

      default:
        break;
    }
  }
// add new If statements here for weapons

  return(dr);
} // This is closing bracket, add new after last bracket above this.


