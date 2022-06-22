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

extern struct room_data *world;
extern struct char_data *mobs;
extern struct char_data *character_list;
extern struct obj_data *objs;
extern struct obj_data *object_list;
extern room_rnum top_of_world;
extern struct zone_data *zone_table_array;
extern struct zone_data *zone_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct Survey *survey_list;


void make_tracks(struct char_data *ch, int room)
{
struct room_affect *raff;
int exists = 0;

        if (!world[room].number || !ch )
          return;

        raff = world[room].room_affs;

        while (raff) {
          if (raff->type == RAFF_TRACKS) {
             if (raff->ch == ch) {
               raff->timer = 10;
               raff->value = (int)time(NULL) ;
               exists = 1;
             }
          }
          raff = raff->next;
        }

        if (!exists) {
           CREATE(raff, struct room_affect, 1);
           raff->type = RAFF_TRACKS;
           raff->ch = ch;
           raff->timer = 10;
           raff->value = (int)time(NULL);
           raff->room = room;
           raff->next = world[room].room_affs;
           world[room].room_affs = raff;
        }
}

void room_affect_text(struct char_data *ch)
{
  char buf[MAX_STRING_LENGTH], blood;
  struct room_affect *raff;            
//  int room;

//    room = IN_ROOM(ch);

    
    buf[0] = '\0';
   
    for (raff = world[IN_ROOM(ch)].room_affs; raff; raff = raff->next) {
       if (raff->type == RAFF_BLOOD) 
         blood = raff->value;
       if(raff->type == RAFF_NPC_BARRIER)             
            send_to_char(ch, "&YSurrounding the area is a pulsating energy barrier.&n\n\r");
       if (raff->type == RAFF_PC_BARRIER)
            send_to_char(ch, "&MSurrounding the area is a magical energy barrier.&n\r\n");
       if (raff->type == RAFF_FIRE)
            send_to_char(ch, "&RA raging inferno burns your skin as the smoke causes you to choke.&n\r\n");
       if (raff->type == RAFF_SHELTER) {
         if (raff->timer > 5)
            send_to_char(ch, "&CThe shelter looks new and very  well constructed, and the fire is shooting high into the air, leaving a trail of smoke as the breeze blows it around.\r\n&n");
         else if (raff->timer > 4)
            send_to_char(ch, "&CThe shelter looks well constructed, but small frays can be seen in the ends of the fabric.  The fire is burning strong, giving off a strong smoke line that can be seen for miles.&n\r\n");
         else if (raff->timer > 3)
            send_to_char(ch, "&CThe shelter seems like it has been empty for a while now, the fire seems to be dying out with the passing moments.&n\r\n");
         else if (raff->timer > 2)
            send_to_char(ch, "&CThe shelter seems very weathered, the fabrics are torn and faded and the fire is all but gone out.&n\r\n");
         else if (raff->timer > 1)
            send_to_char(ch, "&CThe shelter is well past its prime as it has almost fully collapsed.  What could have only been a fire is barely even smoldering.&n\r\n");
         else  
            send_to_char(ch, "&CThe remains of a shelter lay here scattered across the ground.&n\r\n");
       } //endif                 
     }//endfor

    if (blood) {
      if (blood > 10)             
        strcpy(buf,"An awe-inspiring amount of bloody vital organs, limbs, and bones form a thick patch of grisly filth here - the smell is sickening!\n\r");
      else if (blood > 8)
         strcpy(buf,"A venerable lake of crimson-dyed organs and bodily fluids cover the area.\n\r");
      else if (blood > 7)
         strcpy(buf,"The stench of blood and defecation rises from the soaked floor swimming with severed organs.\n\r");
      else if (blood > 6)        
         strcpy(buf,"The odor of the blood in the room makes you want to vomit.\n\r");
      else if (blood > 4)         
         strcpy(buf,"A large quantity of blood has been spilt in the dust, already attracting flies and ants.\n\r");
      else if (blood > 2)              
         strcpy(buf,"A thick pool of blood flows in the rise and fall of the ground.\n\r");
      else if (blood > 1)              
         strcpy(buf,"A slick of crimson blood shines upon the ground.\n\r");
      else if (blood > 0)              
         strcpy(buf,"Some drops of blood from a battle litter the ground.\n\r");
      send_to_char(ch, "&R%s&n", buf);
    }// end if blood
}                                 


void remove_room_affect(struct room_affect *raff, char type)
{
  int i;
  struct room_affect *next_raff;

  i = raff->room;

  if (world[i].room_affs == raff) {
    world[i].room_affs = raff->next;
  } else {
      next_raff = world[i].room_affs;
      while (next_raff->next && next_raff->next != raff)
        next_raff = next_raff->next;
        if (!next_raff->next) {
          log("ERROR: Room affects, no room aff found in remove_room_affect");
          return;
        }
        next_raff->next = raff->next;
  }

  if (raff->type == RAFF_SHELTER) {
    send_to_room(i, "&CWith the passing breeze, the remains of the shelter fall apart and quickly blow away.&n\r\n");
  }
  else if (raff->type == RAFF_BLOOD) {
    switch (rand_number(0, 5)) {     
         case 0:
            send_to_room(i, "Some blood seems down into the cracks in the ground, leaving only a few spots of red residue.\n\r");
            break;
         case 1:     
            send_to_room(i, "A thick stream of blood bubbles and flows away from the pool.\n\r");
            break;  
         case 2:
            send_to_room(i, "The ground rumbles and cracks, allowing some blood to seep into the depths.\n\r");
            break;
         case 3:                   
            send_to_room(i, "Wisps of blood get strewn through the air as a strong wind blows through.\n\r");
            break;        
         case 4:
            send_to_room(i, "A patch of coagulated blood turns grey and shatters into a mote of dust.\n\r");
            break;
         case 5:                   
            send_to_room(i, "You gag in disgust as some of the native critters arrive and begin to lap up some blood.\n\r");
            break;
    }
  }
  else if (raff->type == RAFF_NPC_BARRIER) 
    send_to_room(i, "&YYour gasp for air as the magical barrier collapses.&n\r\n");  
  else if (raff->type == RAFF_PC_BARRIER)
    send_to_room(i, "&MYour eyes burn as the magical barrier collapses around you.&n\r\n"); 
  else if (raff->type == RAFF_FIRE)
    send_to_room(i, "&RYou breath a sigh of relief as the fire finally burns out.&n\r\n");

 if (raff->text)
   raff->text = my_free(raff->text);

   raff = my_free(raff);
}


void remove_all_affects(void)
{
  int i;
  struct room_affect *raff, *next;

  for (i=0;i<top_of_world;i++) {
   if (world[i].room_affs != '\0') {
     for (raff=world[i].room_affs;raff;raff=next) {
        next=raff->next;
        raff->timer = 0;
        raff->text = '\0';
        raff->type = 0;
        raff->value = 0;
        raff->room = 0;
        raff->ch = '\0';
  remove_room_affect(raff,0);
  remove_room_affect(raff,1);
  remove_room_affect(raff,2);
  remove_room_affect(raff,3);
  remove_room_affect(raff,4);
  remove_room_affect(raff,5);
  remove_room_affect(raff,6);
  remove_room_affect(raff,7);
  remove_room_affect(raff,8);
  remove_room_affect(raff,9);
  remove_room_affect(raff,10);


      }
    }
  }

}



void room_affect_update(void)
{
  int i;
  struct room_affect *raff, *next;

  for (i = 0; i < top_of_world; i++) {
    if (world[i].room_affs) {
      for (raff = world[i].room_affs; raff; raff = next) {
       next = raff->next;
       if (raff->timer != -1) {
         raff->timer = raff->timer - 1;
//         if (raff->type == RAFF_BLOOD)
//           raff->value = raff->value + 1;

         if (raff->type == RAFF_SHELTER) {
           if (rand_number(0,3) == 2) {
             send_to_room(i, "The shelters fire burns and flames up with the incoming breeze.\r\n");
           }
         }
         if (raff->timer == 1 && raff->type == RAFF_NPC_BARRIER) 
           send_to_room(i, "&YThe barrier starts to fluctuate as its energy is depleated.&n\r\n");
         if (raff->timer == 1 && raff->type == RAFF_PC_BARRIER)
           send_to_room(i, "&MThe magical barrier begins to pulsate and loose its footing.&n\r\n");
         if (raff->type == RAFF_FIRE) {
           if (rand_number(0, 3) == 2) 
             send_to_room(i, "&RThe raging inferno burns your skin.&n\r\n");
         }  
       }

       if (raff->timer <= 0)
         remove_room_affect(raff, 0);
     
     }
    }
  }
}



void make_blood(struct char_data *ch, struct char_data *vict)
{
  struct room_affect *raff;          
  int room;
  char buf[MAX_STRING_LENGTH];
  bool blood = FALSE;

  if (!ch || !vict)
    return;         
  if (vict) 
    room = IN_ROOM(vict);     
  if (room < 0 || room >= top_of_world)
    return;
  if (!world[room].number)
    return;

 /* check for blood first, if blood, increase body count and timer */
 for (raff = world[room].room_affs; raff; raff = raff->next) {
   if (raff->type == RAFF_BLOOD) {
     blood = TRUE;
     raff->value = raff->value + 1;
     raff->timer = rand_number(15,27);
   }
 }

  /* if no blood, make some */
  if (!blood)
  {
    CREATE(raff, struct room_affect, 1);
    raff->type = RAFF_BLOOD;      
    raff->timer = rand_number(15,27);
    raff->value = 1;
    raff->ch = 0;
    sprintf(buf,"%s killed by %s",(IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)),(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    raff->text = strdup(buf);
    raff->room = room;    
    raff->next = world[room].room_affs;           
    world[room].room_affs = raff;        
  }
}

void remove_blood(int room, int blood)
{
  struct room_affect *raff, *next;

  raff = world[room].room_affs;

  while (raff && blood) {
    next = raff->next;
    if (raff->type == RAFF_BLOOD)
    {
      remove_room_affect(raff, 123);
      blood--;
    }
    raff = next;
  }
}

int count_blood(int room)
{
  struct room_affect *raff;
  int blood;
  blood = 0;

  if (!world[room].number)
    return(0);

  for (raff = world[room].room_affs; raff; raff = raff->next) {
    if (raff->type == RAFF_BLOOD)
      blood++;
  }
  return (blood);
}

