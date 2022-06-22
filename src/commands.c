
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
#include "quest.h"
#include "crystalgate.h"
#include "logger.h"

extern time_t boot_time;
extern int top_of_p_table;
extern socket_t mother_desc;
extern ush_int port;
extern const char *pc_class_types[];
extern int top_of_p_table;
extern struct player_index_element *player_table;
struct char_data * pick_victim(struct char_data *ch);
extern const char *kuvia_version;
extern struct Survey *survey_list;
extern char *wizlist;
extern int exp_multiplier;
int file_to_string(const char *name, char *buf);
char *Commatize(long num);

ACMD(do_combatset)
{
  char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];

  one_argument(argument, arg);

  if (!*arg){
    if (IS_SET_AR(PLR_FLAGS(ch), PLR_COMBATSET2))
      sprintf(buf, "You currently have combatset 2 showing.\r\n");
    else
      sprintf(buf, "You currently have combatset 1 showing.\r\n");

    send_to_char(ch, "Usage: combatset <number>\r\n\r\n"
                     "combatset 1 looks like \"You smite the guard with your pierce!\"\r\n"
                     "combatset 2 looks like \"Your pierce smites the guard!\"\r\n");
    
    send_to_char(ch, "\r\n&C%s&n",buf);
    
    return;
  }

  if (!is_number(arg)) 
  {
   send_to_char(ch, "You can only use the # 1 or 2 to signify combatset commands!\r\n");
   return;
  }

  switch (*arg) {   
  case '1':
    if (IS_SET_AR(PLR_FLAGS(ch), PLR_COMBATSET2)) 
    {
      REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_COMBATSET2);
      send_to_char(ch, "Ok! Your combat is set to option 1!\r\n");
    }
    else
      send_to_char(ch, "Ok! Your combat is set to option 1!\r\n");
    break;
  case '2':
    if (IS_SET_AR(PLR_FLAGS(ch), PLR_COMBATSET2))
    {
      send_to_char(ch, "Ok! Your combat is set to option 2!\r\n");
      break;
    }
    else
    {
     SET_BIT_AR(PLR_FLAGS(ch), PLR_COMBATSET2);
     send_to_char(ch, "Ok! Your combat is set to option 2!\r\n");
    }
    break;
  default:
    break;
  }

}


ACMD(do_zonecount)
{
 send_to_char(ch, "\r\n&WZone Count: &R%d&n\r\n", top_of_zone_table +1);
}


ACMD(do_affected)
{
  struct affected_type *aff;
  extern const char *affected_bits[];
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

  send_to_char(ch, "&MYou are affected by:\r\n"
                   "&m--------------------------------------------------\r\n");

  for (aff = ch->affected; aff; aff = aff->next) {
   if (aff->type == -1 && aff->duration == -1) {
     sprintbitarray(AFF_FLAGS(ch), affected_bits, AF_ARRAY_MAX, buf2);
     sprintf(buf, " &C%-15s &RPERM&n", buf2);
     send_to_char(ch, strcat(buf, "\r\n"));
     continue;
   }
   sprintf(buf, " &C%-15s&n", (aff->type >=0 && aff->type <= MAX_SKILLS) ? skill_name(aff->type) : "UNDEFINED");
   if (aff->modifier)
     sprintf(buf+strlen(buf), "&W - &G%3ld hr%s %4d to %s", aff->duration + 1, (aff->duration == 0 ? "" : "s"), aff->modifier, apply_types[aff->location]);
   else
     sprintf(buf+strlen(buf), "&W - &G%3ld hr%s", aff->duration + 1, (aff->duration == 0 ? "" : "s"));

   strcat(buf, "\r\n");
   send_to_char(ch, buf);
  }
}



ACMD(do_gamestats)
{
  int players, npcs, npcproto, objects, objproto, registered, connected;
  int zonecount, autoquests, shops, freight, roads, altars, rooms;  
  int tradeposts, commands, numofcmds;
  char *timestr;
  time_t uptime;
  int i, d, h, m;
  struct char_data *vict;
  struct obj_data *obj;

  players=npcs=npcproto=objects=objproto=registered=connected=0;
  zonecount=autoquests=shops=freight=roads=altars=rooms=tradeposts=0;
  commands=numofcmds=0;

  registered = (top_of_p_table + 1);
  npcproto = (top_of_mobt + 1);
  objproto = (top_of_objt + 1);
  zonecount = (top_of_zone_table + 1);
//  rooms = (top_of_world + 1);
  rooms = (top_of_world + 1) + 4000000;
  autoquests = total_quests;
  shops = (top_shop + 1);

  while (complete_cmd_info[commands].command[0] != '\n') {
    commands++;
  numofcmds++;
  }

  for (vict = character_list; vict; vict = vict->next) {
    if (IS_NPC(vict))
      npcs++;
    else if (CAN_SEE(ch, vict)) {
      players++;
    if (vict->desc)
      connected++;
    }
  }

  for (obj = object_list; obj; obj = obj->next) 
    objects++;

  for (i = 0; i < top_of_world; i++) {
    if (ROOM_FLAGGED(i, ROOM_FREIGHTOK))
      freight++;
    if (ROOM_FLAGGED(i, SECT_ROAD))
      roads++;
    if (ROOM_FLAGGED(i, ROOM_ALTAR))
      altars++;
    if (world[i].sector_type == SECT_TRADEPOST)
      tradeposts++;
  }
 

  uptime = boot_time;

  timestr = (char *) asctime(localtime(&uptime));
  *(timestr + strlen(timestr) - 1) = '\0';

  uptime = time(0) - boot_time;
  d = uptime / 86400;
  h = (uptime / 3600) % 24;
  m = (uptime / 60) % 60;
  

  send_to_char(ch, "\r\n"
                   "&C%s consists of the following:&n\r\n"
                   "&W===============================================================================================\r\n"
                   "&C%-3s &Btotal registered players, with &C%s&B being online right now!\r\n"
                   "&C%-3s &Btotal objects created and having &C%s &Bof them loaded in the game!\r\n"
                   "&C%-3s &Btotal mobiles created and having &C%s&B of them wandering around right now!\r\n"
//                   "&C%d &Btotal zones with &C%d&B total rooms, which includes a vast wilderness!\r\n"
                   "&C%-3d &Btotal zones with &C%s&B total rooms, which includes a vast wilderness!\r\n"
                   "&C%-3d &Btotal commands useable in-game!\r\n"
                   "&C%-3d &Btotal learnable professions currently implemented\r\n"
                   "&C%-3d &B trade centers located throughout the land!\r\n"
                   "&C4    &Bdifferent road types spaning &C%s &Brooms linking our zones and trade centers!\r\n"
                   "&C%-3d &Btotal altars in the wilderness used to bring yourself back to life!\r\n"
                   "&C%-3d &Bautoquests which you can complete at this time.\r\n"
                   "&C%-3d &Bshops, which you can buy and sell equipment to.&n\r\n"
                   "&C%-3d &BCrystal Gate travel points across the land!&n\r\n"
                   "&W===============================================================================================\r\n"
                   "&BIt has been &C%d &Bday%s, &C%d &Bhour%s, &C%d &Bminute%s since Kuvia has been rebooted."
                   "\r\n&BCurrent XP Multiplier is [&Y%d&B]\r\n"
                   "&W===============================================================================================\r\n"
                   "&BThere are currently &C%d &Bplayer%s connected.&n\r\n",
                    kuvia_version,
                    Commatize(registered), Commatize(players), Commatize(objproto), Commatize(objects),
                    Commatize(npcproto), Commatize(npcs), zonecount,Commatize(rooms),
                    commands, MAX_PROFS, 
                    tradeposts, Commatize(freight), //Commatize(roads),
                    altars, autoquests, shops, MAX_GATES,
                    d, d==1 ? "" : "s", h, h==1 ? "" : "s", m, m==1 ? "" : "s", exp_multiplier,
                    connected, connected==1 ? "" : "s");

}


ACMD(do_shelter)             
{
  struct room_affect *raff;
//  struct Survey *surv;   

  for (raff = world[IN_ROOM(ch)].room_affs; raff; raff = raff->next) {
     if (raff->type == RAFF_SHELTER) {     
       raff->timer = 60;
       send_to_char(ch, "You peek inside the shelter and find it empty. You quickly repair any flaws, throw some wood on the fire and settle in.\r\n");
       act("$n throws some timber on the fire and settles into the shelter.", TRUE, ch, 0, 0, TO_ROOM);
       GET_MOVE(ch) /= 2;
       GET_HIT(ch) /= 2;
       GET_MANA(ch) /= 2;           
       return;
     }
  }

  if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS)) {
     send_to_char(ch, "\r\nYou must be in the wilderness to set up a shelter.\r\n");
     return;
  }

  if (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM || world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM ||
      world[IN_ROOM(ch)].sector_type == SECT_FLYING || world[IN_ROOM(ch)].sector_type == SECT_LAVA ||
      world[IN_ROOM(ch)].sector_type == SECT_AIR || world[IN_ROOM(ch)].sector_type == SECT_RIVER ||
      world[IN_ROOM(ch)].sector_type == SECT_STREAM || world[IN_ROOM(ch)].sector_type == SECT_STREAMBED ||
      world[IN_ROOM(ch)].sector_type == SECT_OCEAN || world[IN_ROOM(ch)].sector_type == SECT_NOPASS ||
      world[IN_ROOM(ch)].sector_type == SECT_ROAD || world[IN_ROOM(ch)].sector_type == WILD_ALTAR ||
      world[IN_ROOM(ch)].sector_type == WILD_ZONEENT) {
    send_to_char(ch, "You cannot set up a shelter in this location.\r\n");
    return;
  }

  GET_MOVE(ch) /= 10;
  GET_HIT(ch) /= 2;
  GET_MANA(ch) /= 2;

  CREATE(raff, struct room_affect, 1);
  raff->type = RAFF_SHELTER;
  raff->timer = 60;
  raff->value = 0;
  raff->ch = NULL;
  raff->text = NULL;
  raff->room = IN_ROOM(ch);
  raff->next = world[IN_ROOM(ch)].room_affs;
  world[IN_ROOM(ch)].room_affs = raff;




  send_to_char(ch, "You gather some timber and pull some cloth from your bag and begin to construct a shelter.  You take some timber, pile it up and ignite it for some warmth and light.\r\n");
  act("$n gathers some timber and constructs a shelter and settles in.", TRUE, ch, 0, 0, TO_ROOM);
  nmudlog(INFO_LOG, LVL_GOD, TRUE, "%s sets up shelter at %d x %d", GET_NAME(ch), world[IN_ROOM(ch)].x, world[IN_ROOM(ch)].y);

/*  CREATE(surv, struct Survey, 1);
  surv->next = survey_list;
  survey_list = surv;
  surv->description = "You see some smoke and a faint outline of a shelter";
  surv->X = world[IN_ROOM(ch)].x;
  surv->Y = world[IN_ROOM(ch)].y;
  surv->dist = 77; */
}  


ACMD(do_raffs)
{
    int i, count;
    i = count = 0;

 send_to_char(ch, "Types are: 0=null 1=shelter 2=tracks 3=barrier 4=gas 5=acid 6=fire 7=wind 8=flood\r\n");

    for (i = 0;i < top_of_world; i++) {
      if (ROOM_FLAGGED(world[i].number, ROOM_WILDERNESS))
        send_to_char(ch, "Raff: %d, Rm: %d\r\n", world[i].room_affs->type, world[i].number);
}

}

ACMD(do_listroad)
{
  int i;
  
  send_to_char(ch, "Current Roads\r\n");
  send_to_char(ch, "------------------\r\n");
  for (i = 0; i < top_of_world; i++) {
    if (world[i].sector_type == SECT_ROAD || world[i].sector_type == WILD_MAINROAD)
      send_to_char(ch, "%s - %d\r\n",world[i].name, world[i].number);
  }
}

ACMD(do_listtp)
{
  int i;

  send_to_char(ch, "Current TPs\r\n");
  send_to_char(ch, "-----------\r\n");
  for (i = 0; i < top_of_world; i++) {
    if (world[i].sector_type == SECT_TRADEPOST)
       send_to_char(ch, "%s - %d \r\n", world[i].name, world[i].number);
  }

}

ACMD(do_newb_recall)
{
  int recall = 6300;

  if (GET_LEVEL(ch) <= 10) {
     char_from_room(ch);
     send_to_char(ch, "&CSince you are new to these lands, the gods allow you to travel for free back to the main city.\r\n&n");
     char_to_room(ch, real_room(recall));
     look_at_room(ch, 0);
  }
  else
     send_to_char(ch, "&CYou are too high of a level to use this command.&n\r\n");
 
}

ACMD(do_wizlist)
{
  char name[MSL], area[MSL], file[MIL*2], path[MSL], temp_name[MIL], file2[MIL*2];
  int i = 0;
  FILE *fp;

  two_arguments(argument, name, area);

  if (!name || !name[0]) {
      send_to_char(ch, wizlist);
      return;
  }

  if ((name || name[0])  && (!area || !area[0])) {
    strcpy(temp_name, name);
    for (i = 0; temp_name[i] != '\0'; i++)
       temp_name[i] = LOWER(temp_name[i]);
    sprintf(path, "../wizlist/%s.wiz", temp_name);

    if (!(fp = fopen(path, "r"))) {
      send_to_char(ch, "There is no wizlist entry for that god at this time.\r\n");
      nmudlog(INFO_LOG, LVL_IMPL, TRUE, "Missing wizlist entry for %s", temp_name);
      return;
    } // close the fopen for reading
   fclose(fp);
   file_to_string(path, file);
   send_to_char(ch, file);
  } // close if we have a name and no area
 

 else  if (name && area) {
    strcpy(temp_name, name);
    for (i=0; temp_name[i] != '\0'; i++)
      temp_name[i] = LOWER(temp_name[i]);
   
   if (is_abbrev(area, "accomplishments"))
    sprintf(path, "../wizlist/accomplishments/%s.acc", temp_name);
   else if (is_abbrev(area, "projects"))
     sprintf(path, "../wizlist/projects/%s.prj", temp_name);
   else if (is_abbrev(area, "zones"))
     sprintf(path, "../wizlist/zones/%s.zon", temp_name);
   else if (is_abbrev(area, "extras"))
     sprintf(path, "../wizlist/extras/%s.xtr", temp_name);
   
  if (!(fp = fopen(path, "r"))) {
   send_to_char(ch, "There is no %s file for %s at this time.\r\n", area, name);
   if (GET_LEVEL(ch) == LVL_IMPL) {
    send_to_char(ch, "Path: %s\r\n", path);
    send_to_char(ch, "LName: %s\r\n", temp_name);
    send_to_char(ch, "Input1: %s\r\n", name);
    send_to_char(ch, "Input2: %s\r\n", area);
   return;
   }// close GET_LEVEL
  } // close fopen

  fclose(fp);                
  file_to_string(path, file2);
  send_to_char(ch, "\r\n%s", file2);
//  page_string(ch->desc, file2, 1);            
  } // close name and area
}// close function



ACMD(do_email)
{
  int err;

  if (!argument || !*argument) {
    if (!GET_EMAIL(ch)) {
      send_to_char(ch, "&RYou do not currently have a registered email address.&n\r\n");
    } else {
      send_to_char(ch, "&CYour registered email address is: &Y%s&N\r\n",output_email(GET_EMAIL(ch)));
    }
    return;
  }

  skip_spaces(&argument);
  delete_doubleat(argument);

  if (IS_NPC(ch))
    send_to_char(ch, "Mob's can't receive emails!\r\n");
  else if (strlen(argument) > MAX_EMAIL_LENGTH)
    send_to_char(ch, "Sorry, email addresses can't be longer than %d characters.\r\n", MAX_EMAIL_LENGTH);
  else if ((err = validate_email(argument)) != EMAIL_SUCCESS)
    send_to_char(ch, "Sorry, that address is invalid.\r\nReason: %s\r\n", email_error_message(err));
  else {
    set_email(ch, argument);
    send_to_char(ch, "Okay, your email address is now %s.\r\n", output_email(GET_EMAIL(ch)));
  }
}


ACMD(do_pray)
{
  struct descriptor_data *d;
  int level = LVL_IMM;
  char buf1[MSL], buf2[MSL];

/*   THIS IS FOR PRAYING AT AN ALTAR

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ALTAR) && PRF2_FLAGGED(ch, PRF2_AFTERLIFE) && !*argument) {
    REMOVE_BIT_AR(PRF2_AFTERLIFE)

*/

  skip_spaces(&argument);
  delete_doubledollar(argument);
 
  if (!*argument) {
    send_to_char(ch, "What do you want to pray?\r\n");
    return; 
  }

  if (IS_NPC(ch)) {
    send_to_char(ch, "Mobs don't need to pray!\r\n");
  }

  if (GET_LEVEL(ch) >= LVL_IMM) {
    send_to_char(ch, "Imms don't need to use the pray channel.. use god chans please.\r\n");
    return;
  }

  skip_spaces(&argument);

  if (GET_LEVEL(ch) <= LVL_IMM) {
   sprintf(buf1, "&Y[PRAY]&C %s prays, '%s'\r\n&n", GET_NAME(ch), argument);
  }

  sprintf(buf2, "&YYou pray, '%s'&n\r\n", argument);
  send_to_char(ch, buf2);

  for (d = descriptor_list; d; d = d->next) {
    if ((!d->connected) && (GET_LEVEL(d->character) >= level) &&
      (!PRF_FLAGGED(d->character, PRF2_NOPRAY)) && (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING))
      && (d->character != ch || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
        if (CAN_SEE(d->character, ch))
          send_to_char(d->character, buf1);
   }
  }
}
