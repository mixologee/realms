/**************************************************************************
*  File: act.wizard.c                                      Part of tbaMUD *
*  Usage: Player-level god commands and other goodies.                    *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
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
#include "screen.h"
#include "constants.h"
#include "oasis.h"
#include "dg_scripts.h"
#include "shop.h"
#include "arena.h"
#include "png.h"
#include "quest.h"
#include "logger.h"

/* external vars */
extern FILE *player_fl;
extern struct attack_hit_type attack_hit_text[];
extern char *class_abbrevs[];
extern time_t boot_time;
extern int circle_shutdown, circle_reboot;
extern int circle_restrict;
extern int buf_switches, buf_largecount, buf_overflows;
extern int top_of_p_table;
extern socket_t mother_desc;
extern ush_int port;
extern const char *pc_class_types[];
extern int top_of_p_table;
extern struct player_index_element *player_table;
extern const char *raffected_bits[];

extern room_rnum party_room;
extern room_vnum reclass_room;
extern struct castle_info *castles[];
extern int in_arena;
extern int in_start_arena;
extern int start_time;
extern int game_length;
extern int lo_lim;
extern int hi_lim;
extern int cost_per_lev;
extern int time_to_start;
extern int time_left_in_game;
extern int arena_pot;
extern int bet_pot;
extern struct clan_type *clan_info;
extern int cnum;

/* external functions */
int level_exp(int chclass, int level);
void show_shops(struct char_data *ch, char *value);
void hcontrol_list_houses(struct char_data *ch, char *arg);
void do_start(struct char_data *ch);
void appear(struct char_data *ch);
void reset_zone(zone_rnum zone);
void roll_real_abils(struct char_data *ch);
int parse_class(char arg);
void run_autowiz(void);
int save_all(void);
struct char_data *find_char(long n);
void print_zone(struct char_data *ch, zone_vnum vnum);
char *get_spec_func_name(SPECIAL(*func));
char* add_commas(long X);
char *Commatize(long num);


char *email_error_message(int error_code);
int validate_email(char *check_addr);
char *output_email(char *mail_addr);

int do_multi(struct char_data *ch, int class);
void perform_remove(struct char_data *ch, int pos);
void build_player_index(void);
void delete_aliases(const char *charname);

zone_rnum real_zone_by_thing(room_vnum vznum);
SPECIAL(shop_keeper);
void Crash_rentsave(struct char_data * ch, int cost);
void new_hist_messg(struct descriptor_data *d, const char *msg);
void clearMemory(struct char_data *ch);
int perform_set_dg_var(struct char_data *ch, struct char_data *vict, char *val_arg);
struct time_info_data *real_time_passed(time_t t2, time_t t1);


/* local functions */
int perform_set(struct char_data *ch, struct char_data *vict, int mode, char *val_arg);
void perform_immort_invis(struct char_data *ch, int level);
ACMD(do_godlist);
ACMD(do_echo);
ACMD(do_send);
room_rnum find_target_room(struct char_data *ch, char *rawroomstr);
ACMD(do_at);
ACMD(do_goto);
ACMD(do_trans);
ACMD(do_teleport);
ACMD(do_vnum);
void list_zone_commands_room(struct char_data *ch, room_vnum rvnum);
void do_stat_room(struct char_data *ch, struct room_data *rm);
void do_stat_object(struct char_data *ch, struct obj_data *j);
void do_app_object(struct char_data *ch, struct obj_data *j);
void do_stat_character(struct char_data *ch, struct char_data *k);
ACMD(do_stat);
ACMD(do_shutdown);
void stop_snooping(struct char_data *ch);
ACMD(do_snoop);
ACMD(do_switch);
ACMD(do_return);
ACMD(do_load);
ACMD(do_vstat);
ACMD(do_purge);
ACMD(do_syslog);
ACMD(do_advance);
ACMD(do_restore);
void perform_immort_vis(struct char_data *ch);
ACMD(do_invis);
ACMD(do_gecho);
ACMD(do_poofset);
ACMD(do_dc);
ACMD(do_wizlock);
ACMD(do_date);
ACMD(do_last);
ACMD(do_force);
ACMD(do_tag);
ACMD(do_wiznet);
ACMD(do_zreset);
ACMD(do_wizutil);
size_t print_zone_to_buf(char *bufptr, size_t left, zone_rnum zone, int listall);
size_t print_zone_to_buf_mort(char *bufptr, size_t left, zone_rnum zone, int listall);
ACMD(do_show);
ACMD(do_set);
void snoop_check(struct char_data *ch);
ACMD(do_saveall);
ACMD(do_zpurge);
void clean_llog_entries(void);
ACMD(do_list_llog_entries);
struct char_data *is_in_game(long idnum);
ACMD(do_links);
void mob_checkload(struct char_data *ch, mob_vnum mvnum);
void obj_checkload(struct char_data *ch, obj_vnum ovnum);
void trg_checkload(struct char_data *ch, trig_vnum tvnum);
ACMD(do_zcheck);
ACMD(do_checkloadstatus);
ACMD(do_poofs);
ACMD(do_copyover);
ACMD(do_peace);
void mod_llog_entry(struct last_entry *llast,int type);
ACMD(do_plist);
ACMD(do_wizupdate);

ACMD(do_mist);
ACMD(do_appraise);
ACMD(do_restore_all);
ACMD(do_setcrew);
ACMD(do_enroll_crew);
ACMD(do_remove_crew);
ACMD(do_set_rank);
ACMD(do_crewtalk);
ACMD(do_cinfo);
ACMD(do_cwho);
ACMD(do_untag);
ACMD(do_wizhelp);
ACMD(do_png);

int purge_room(room_rnum room)
{
  int j;
  struct char_data *vict;

  if (room == NOWHERE || room > top_of_world) return 0;

  for (vict = world[room].people; vict; vict = vict->next_in_room) {
    if (!IS_NPC(vict))
      continue;

    /* Dump inventory. */
    while (vict->carrying)
      extract_obj(vict->carrying);

    /* Dump equipment. */
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(vict, j))
        extract_obj(GET_EQ(vict, j));

    /* Dump character. */
    extract_char(vict);
  }

  /* Clear the ground. */
  while (world[room].contents)
    extract_obj(world[room].contents);

  return 1;
}

ACMD(do_echo)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char(ch, "Yes.. but what?\r\n");
  else {
    char buf[MAX_INPUT_LENGTH + 4];

    if (subcmd == SCMD_EMOTE)
      snprintf(buf, sizeof(buf), "$n %s", argument);
    else {
      strlcpy(buf, argument, sizeof(buf));
      nmudlog(INFO_LOG, LVL_GOD, TRUE, "%s echod: %s", GET_NAME(ch), buf);      
//mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE, "(GC) %s echoed: %s", GET_NAME(ch), buf);
      }
    act(buf, FALSE, ch, 0, 0, TO_ROOM);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}

ACMD(do_untag)
{

  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Untag who?\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "%s\r\n", CONFIG_NOPERSON);
    return;
  }

    send_to_char(vict, "The gods feel you are no longer IT material!\r\n");
    REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_TAGGED);
    return;
}

ACMD(do_tag)
{

  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Tag who?\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "%s\r\n", CONFIG_NOPERSON);
    return;
  }

  if(GET_LEVEL(ch) > LVL_IMMORT){
    send_to_char(ch, "You tag %s, WOO HOO you're not it anymore!\r\n",
                 GET_NAME(vict));
    send_to_char(vict, "HA HA You're it!\r\n");
    SET_BIT_AR(PLR_FLAGS(vict), PLR_TAGGED);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_TAGGED);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_TAGGED) && PLR_FLAGGED(vict, PLR_TAGGED)){
    send_to_char(ch, "Why would you tag someon that's already it...\r\n");
    send_to_char(vict, "Silly %s, tryin' to tag someone who's it...\r\n",
                 GET_NAME(ch));
    return;
  }

  if (PLR_FLAGGED(ch, PLR_TAGGED) && !PLR_FLAGGED(vict, PLR_TAGGED)){
    send_to_char(ch, "You tag %s, WOO HOO you're not it anymore!\r\n",
                 GET_NAME(vict));
    send_to_char(vict, "HA HA You're it!\r\n");
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_TAGGED);
    SET_BIT_AR(PLR_FLAGS(vict), PLR_TAGGED);
    return;
  }

}
/*
ACMD(do_send)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char(ch, "Send what to who?\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "%s", CONFIG_NOPERSON);
    return;
  }
  send_to_char(vict, "%s\r\n", buf);
  nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s sent %s: %s", GET_NAME(ch), GET_NAME(vict), buf);
//  mudlog(CMP, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s sent %s: %s", GET_NAME(ch), GET_NAME(vict), buf);

  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(ch, "Sent.\r\n");
  else
    send_to_char(ch, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
}
*/
/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_rnum find_target_room(struct char_data *ch, char *rawroomstr)
{
  room_rnum location = NOWHERE;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char(ch, "You must supply a room number or name.\r\n");
    return (NOWHERE);
  }

  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    if ((location = real_room((room_vnum)atoi(roomstr))) == NOWHERE) {
      send_to_char(ch, "No room exists with that number.\r\n");
      return (NOWHERE);
    }
  } else {
    struct char_data *target_mob;
    struct obj_data *target_obj;
    char *mobobjstr = roomstr;
    int num;

    num = get_number(&mobobjstr);
    if ((target_mob = get_char_vis(ch, mobobjstr, &num, FIND_CHAR_WORLD)) != NULL) {
      if ((location = IN_ROOM(target_mob)) == NOWHERE) {
        send_to_char(ch, "That character is currently lost.\r\n");
        return (NOWHERE);
      }
    } else if ((target_obj = get_obj_vis(ch, mobobjstr, &num)) != NULL) {
      if (IN_ROOM(target_obj) != NOWHERE)
        location = IN_ROOM(target_obj);
      else if (target_obj->carried_by && IN_ROOM(target_obj->carried_by) != NOWHERE)
        location = IN_ROOM(target_obj->carried_by);
      else if (target_obj->worn_by && IN_ROOM(target_obj->worn_by) != NOWHERE)
        location = IN_ROOM(target_obj->worn_by);

      if (location == NOWHERE) {
        send_to_char(ch, "That object is currently not in a room.\r\n");
        return (NOWHERE);
      }
    }

    if (location == NOWHERE) {
      send_to_char(ch, "Nothing exists by that name.\r\n");
      return (NOWHERE);
    }
  }

  /* A location has been found -- if you're >= GRGOD, no restrictions. */
  if (GET_LEVEL(ch) >= LVL_GRGOD)
    return (location);

  if (ROOM_FLAGGED(location, ROOM_GODROOM))
    send_to_char(ch, "You are not godly enough to use that room!\r\n");
  else if (ROOM_FLAGGED(location, ROOM_PRIVATE) && world[location].people && world[location].people->next_in_room)
    send_to_char(ch, "There's a private conversation going on in that room.\r\n");
  else if (ROOM_FLAGGED(location, ROOM_HOUSE) && !House_can_enter(ch, GET_ROOM_VNUM(location)))
    send_to_char(ch, "That's private property -- no trespassing!\r\n");
  else
    return (location);

  return (NOWHERE);
}

ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  room_rnum location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char(ch, "You must supply a room number or a name.\r\n");
    return;
  }

  if (!*command) {
    send_to_char(ch, "What do you want to do there?\r\n");
    return;
  }

  if ((location = find_target_room(ch, buf)) == NOWHERE)
    return;

  /* a location has been found. */
  original_loc = IN_ROOM(ch);
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (IN_ROOM(ch) == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}

ACMD(do_mist)
{
  room_rnum location;
  int old_room, party_spot;

  party_spot = 1100;
  old_room = GET_ROOM_IN_BEFORE_PARTY(ch);

 if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK) || ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
 {
    send_to_char(ch, "Sorry, but you can't get to the party from here.");
    return;
 }

  if (!PLR_FLAGGED(ch, PLR_PARTYIN))
  {
    GET_ROOM_IN_BEFORE_PARTY(ch) = GET_ROOM_VNUM(IN_ROOM(ch));
    snprintf(buf, sizeof(buf), "%d", party_spot);
    act("&c$n's body shatters into shining drops of vapor!&n", FALSE, ch, 0, 0, TO_ROOM);
    act("&cYou feel disoriented as your body shatters into vapor, suddenly the vapor coalesces and you find yourself elsewhere!&n", 
	FALSE, ch, 0, 0, TO_CHAR);
    location = find_target_room(ch, buf);
    char_from_room(ch);
    char_to_room(ch, location);
    SET_BIT_AR(PLR_FLAGS(ch), PLR_PARTYIN);
    look_at_room(ch, 0);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_PARTYIN)){
    snprintf(buf, sizeof(buf), "%d", old_room);
    act("&WA haze of vapor coalesces into the form of $n.&n", FALSE, ch, 0, 0, TO_ROOM);
    act("&WYou feel disoriented as your body shatters into vapor, suddenly the vapor coalesces and you find yourself elsewhere!&n", 
	FALSE, ch, 0, 0, TO_CHAR);
    location = find_target_room(ch, buf);
    char_from_room(ch);
    char_to_room(ch, location);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_PARTYIN);
    look_at_room(ch, 0);
  }
}

ACMD(do_goto)
{
  room_rnum location;

  if ((location = find_target_room(ch, argument)) == NOWHERE)
    return;

  if (ZONE_FLAGGED(GET_ROOM_ZONE(location), ZONE_NOIMMORT) && (GET_LEVEL(ch) >= LVL_IMMORT) && (GET_LEVEL(ch) < LVL_GOD)) {
    send_to_char(ch, "Sorry, that zone is off limits to masters!");
    return;
  }

  snprintf(buf, sizeof(buf), "$n %s", POOFOUT(ch) ? POOFOUT(ch) : "disappears in a puff of smoke.");
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  char_from_room(ch);
  char_to_room(ch, location);
  if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS)) {
    MakeWildRoom(IN_ROOM(ch), UP);
    log("Called MakeWildRoom\r\n");
  }

  snprintf(buf, sizeof(buf), "$n %s", POOFIN(ch) ? POOFIN(ch) : "appears with an ear-splitting bang.");
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  look_at_room(ch, 0);
  enter_wtrigger(&world[IN_ROOM(ch)], ch, -1);

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
       send_to_char(ch, "&RNotice: You have left NEUTRAL PLAYER KILL and entered CHAOTIC PLAYER KILL!\r\n");
    }
  }


//  look_at_room(ch, 0);
//  enter_wtrigger(&world[IN_ROOM(ch)], ch, -1);
}

ACMD(do_trans)
{
  char buf[MAX_INPUT_LENGTH];
  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char(ch, "Whom do you wish to transfer?\r\n");
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
      send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (victim == ch)
      send_to_char(ch, "That doesn't make much sense, does it?\r\n");
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char(ch, "Go transfer someone your own size.\r\n");
	return;
      }
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, IN_ROOM(ch));
      if (PLR_FLAGGED(victim, PLR_PARTYIN)) {
          REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_PARTYIN);
          GET_ROOM_IN_BEFORE_PARTY(victim) = IN_ROOM(ch);
        }
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(victim, 0);

      enter_wtrigger(&world[IN_ROOM(victim)], victim, -1);
    }
  } else {			/* Trans All */
    if (GET_LEVEL(ch) < LVL_GRGOD) {
      send_to_char(ch, "I think not.\r\n");
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i->character && i->character != ch) {
	victim = i->character;
	if (GET_LEVEL(victim) >= GET_LEVEL(ch))
	  continue;
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, IN_ROOM(ch));
	if (PLR_FLAGGED(victim, PLR_PARTYIN)) {
          REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_PARTYIN);
	  GET_ROOM_IN_BEFORE_PARTY(victim) = IN_ROOM(ch);
        }
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	look_at_room(victim, 0);
        enter_wtrigger(&world[IN_ROOM(victim)], victim, -1);
      }
    send_to_char(ch, "%s", CONFIG_OK);
  }
}

ACMD(do_teleport)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct char_data *victim;
  room_rnum target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char(ch, "Whom do you wish to teleport?\r\n");
  else if (!(victim = get_char_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (victim == ch)
    send_to_char(ch, "Use 'goto' to teleport yourself.\r\n");
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char(ch, "Maybe you shouldn't do that.\r\n");
  else if (!*buf2)
    send_to_char(ch, "Where do you wish to send this person?\r\n");
  else if ((target = find_target_room(ch, buf2)) != NOWHERE) {
    send_to_char(ch, "%s", CONFIG_OK);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(victim, 0);
    enter_wtrigger(&world[IN_ROOM(victim)], victim, -1);
  }
}

ACMD(do_vnum)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  int good_arg = 0;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    send_to_char(ch, "Usage: vnum { obj | mob | room | trig } <name>\r\n");
    return;
  }
  if (is_abbrev(buf, "mob") && (good_arg = 1))
    if (!vnum_mobile(buf2, ch))
      send_to_char(ch, "No mobiles by that name.\r\n");

  if (is_abbrev(buf, "obj") && (good_arg =1 ))
    if (!vnum_object(buf2, ch))
      send_to_char(ch, "No objects by that name.\r\n");

  if (is_abbrev(buf, "room") && (good_arg = 1))
    if (!vnum_room(buf2, ch))
      send_to_char(ch, "No rooms by that name.\r\n");

  if (is_abbrev(buf, "trig") && (good_arg = 1))
    if (!vnum_trig(buf2, ch))
      send_to_char(ch, "No triggers by that name.\r\n");

  if (!good_arg)
     send_to_char(ch, "Usage: vnum { obj | mob | room | trig } <name>\r\n");
 }

#define ZOCMD zone_table[zrnum].cmd[subcmd]

void list_zone_commands_room(struct char_data *ch, room_vnum rvnum)
{
  zone_rnum zrnum = real_zone_by_thing(rvnum);
  room_rnum rrnum = real_room(rvnum), cmd_room = NOWHERE;
  int subcmd = 0, count = 0;

  if (zrnum == NOWHERE || rrnum == NOWHERE) {
    send_to_char(ch, "No zone information available.\r\n");
    return;
  }

  get_char_colors(ch);

  send_to_char(ch, "Zone commands in this room:%s\r\n", yel);
  while (ZOCMD.command != 'S') {
    switch (ZOCMD.command) {
      case 'M':
      case 'O':
      case 'T':
      case 'V':
        cmd_room = ZOCMD.arg3;
        break;
      case 'D':
      case 'R':
        cmd_room = ZOCMD.arg1;
        break;
      default:
        break;
    }
    if (cmd_room == rrnum) {
      count++;
      /* start listing */
      switch (ZOCMD.command) {
        case 'M':
          send_to_char(ch, "%sLoad %s [%s%d%s], Max : %d\r\n",
                  ZOCMD.if_flag ? " then " : "",
                  mob_proto[ZOCMD.arg1].player.short_descr, cyn,
                  mob_index[ZOCMD.arg1].vnum, yel, ZOCMD.arg2
                  );
          break;
        case 'G':
          send_to_char(ch, "%sGive it %s [%s%d%s], Max : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel,
    	      ZOCMD.arg2
    	      );
          break;
        case 'O':
          send_to_char(ch, "%sLoad %s [%s%d%s], Max : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel,
    	      ZOCMD.arg2
    	      );
          break;
        case 'E':
          send_to_char(ch, "%sEquip with %s [%s%d%s], %s, Max : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel,
    	      equipment_types[ZOCMD.arg3],
    	      ZOCMD.arg2
    	      );
          break;
        case 'P':
          send_to_char(ch, "%sPut %s [%s%d%s] in %s [%s%d%s], Max : %d\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg1].short_description,
    	      cyn, obj_index[ZOCMD.arg1].vnum, yel,
    	      obj_proto[ZOCMD.arg3].short_description,
    	      cyn, obj_index[ZOCMD.arg3].vnum, yel,
    	      ZOCMD.arg2
    	      );
          break;
        case 'R':
          send_to_char(ch, "%sRemove %s [%s%d%s] from room.\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      obj_proto[ZOCMD.arg2].short_description,
    	      cyn, obj_index[ZOCMD.arg2].vnum, yel
    	      );
          break;
        case 'D':
          send_to_char(ch, "%sSet door %s as %s.\r\n",
    	      ZOCMD.if_flag ? " then " : "",
    	      dirs[ZOCMD.arg2],
    	      ZOCMD.arg3 ? ((ZOCMD.arg3 == 1) ? "closed" : "locked") : "open"
    	      );
          break;
        case 'T':
          send_to_char(ch, "%sAttach trigger %s%s%s [%s%d%s] to %s\r\n",
            ZOCMD.if_flag ? " then " : "",
            cyn, trig_index[ZOCMD.arg2]->proto->name, yel,
            cyn, trig_index[ZOCMD.arg2]->vnum, yel,
            ((ZOCMD.arg1 == MOB_TRIGGER) ? "mobile" :
              ((ZOCMD.arg1 == OBJ_TRIGGER) ? "object" :
                ((ZOCMD.arg1 == WLD_TRIGGER)? "room" : "????"))));
          break;
        case 'V':
          send_to_char(ch, "%sAssign global %s:%d to %s = %s\r\n",
            ZOCMD.if_flag ? " then " : "",
            ZOCMD.sarg1, ZOCMD.arg2,
            ((ZOCMD.arg1 == MOB_TRIGGER) ? "mobile" :
              ((ZOCMD.arg1 == OBJ_TRIGGER) ? "object" :
                ((ZOCMD.arg1 == WLD_TRIGGER)? "room" : "????"))),
            ZOCMD.sarg2);
          break;
        default:
          send_to_char(ch, "<Unknown Command>\r\n");
          break;
      }
    }
    subcmd++;
  }
  send_to_char(ch, nrm);
  if (!count)
    send_to_char(ch, "None!\r\n");
}
#undef ZOCMD

void do_stat_room(struct char_data *ch, struct room_data *rm)
{
  struct extra_descr_data *desc;
  int i, found, column;
  struct obj_data *j;
  struct char_data *k;
  struct room_affect *raff;


  send_to_char(ch, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name, CCNRM(ch, C_NRM));

  sprinttype(rm->sector_type, sector_types, buf2, sizeof(buf2));
  send_to_char(ch, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], IDNum: [%5ld], Type: %s\r\n",
	  zone_table[rm->zone].number, CCGRN(ch, C_NRM), rm->number,
	  CCNRM(ch, C_NRM), IN_ROOM(ch), (long) rm->number + ROOM_ID_BASE, buf2);

  sprintbitarray(rm->room_flags, room_bits, RF_ARRAY_MAX, buf2);
  send_to_char(ch, "SpecProc: %s, Flags: %s\r\n", rm->func == NULL ? "None" : get_spec_func_name(rm->func), buf2);

  send_to_char(ch, "Description:\r\n%s", rm->description ? rm->description : "  None.\r\n");

  if (rm->ex_description) {
    send_to_char(ch, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next)
      send_to_char(ch, " [%s]", desc->keyword);
    send_to_char(ch, "%s\r\n", CCNRM(ch, C_NRM));
  }

  if (rm->room_affs) {
    for (raff = world[IN_ROOM(ch)].room_affs; raff; raff = raff->next) {
      send_to_char(ch, "&GRoom Affect: Affect %s (ticks: %d value: %d)!&n\r\n", raffected_bits[rm->room_affs->type], rm->room_affs->timer, rm->room_affs->value);
    }
  } else {
      send_to_char(ch, "&GRoom Affects: None&n\r\n");
  }

  send_to_char(ch, "&gWild Info: X:%d Y:%d&n\r\n", rm->x, rm->y);
  send_to_char(ch, "Chars present:%s", CCYEL(ch, C_NRM));
  column = 14;	/* ^^^ strlen ^^^ */
  for (found = FALSE, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;

    column += send_to_char(ch, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
		!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB"));
    if (column >= 62) {
      send_to_char(ch, "%s\r\n", k->next_in_room ? "," : "");
      found = FALSE;
      column = 0;
    }
  }
  send_to_char(ch, "%s", CCNRM(ch, C_NRM));

  if (rm->contents) {
    send_to_char(ch, "Contents:%s", CCGRN(ch, C_NRM));
    column = 9;	/* ^^^ strlen ^^^ */

    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;

      column += send_to_char(ch, "%s %s", found++ ? "," : "", j->short_description);
      if (column >= 62) {
	send_to_char(ch, "%s\r\n", j->next_content ? "," : "");
	found = FALSE;
        column = 0;
      }
    }
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
  }

  for (i = 0; i < NUM_OF_DIRS; i++) {
    char buf1[128];

    if (!rm->dir_option[i])
      continue;

    if (rm->dir_option[i]->to_room == NOWHERE)
      snprintf(buf1, sizeof(buf1), " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
    else
      snprintf(buf1, sizeof(buf1), "%s%5d%s", CCCYN(ch, C_NRM), GET_ROOM_VNUM(rm->dir_option[i]->to_room), CCNRM(ch, C_NRM));

    sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2, sizeof(buf2));

    send_to_char(ch, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywords: %s, Type: %s\r\n%s",
	CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1,
	rm->dir_option[i]->key == NOTHING ? -1 : rm->dir_option[i]->key,
	rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None", buf2,
	rm->dir_option[i]->general_description ? rm->dir_option[i]->general_description : "  No exit description.\r\n");
  }

  /* check the room for a script */
  do_sstat_room(ch, rm);

  list_zone_commands_room(ch, rm->number);
}

void do_app_object(struct char_data *ch, struct obj_data *j)
{
  int i, found;
  obj_vnum vnum;
  struct obj_data *j2;
  char *icol="&n";

  vnum = GET_OBJ_VNUM(j);
  send_to_char(ch, "\r\n&BName&W:&G %s     &b[&G%s&B]\r\n&n", j->short_description ? j->short_description : "<None>", j->name);



  sprintbitarray(GET_OBJ_WEAR(j), wear_bits, TW_ARRAY_MAX, buf);
  send_to_char(ch, "&BItem Type&W: &G%s &BEquippable&W: &G%s &BWeight&W:&G %d &BValue&W:&G %d\r\n",
                       item_types[GET_OBJ_TYPE(j)],buf,GET_OBJ_WEIGHT(j), GET_OBJ_COST(j));

  sprintbitarray(GET_OBJ_EXTRA(j), extra_bits, EF_ARRAY_MAX, buf);
  send_to_char(ch, "&BLevel Restrictions&W: &G%-2d &BAttributes&W:&G %s\r\n",
                        GET_OBJ_LEVEL(j),buf);

  if (GET_OBJ_BOUND_ID(j) != NOBODY) {
    if (get_name_by_id(GET_OBJ_BOUND_ID(j)) != NULL)
      send_to_char(ch, "&BBound To&W: &G%s&n\r\n", CAP(get_name_by_id(GET_OBJ_BOUND_ID(j))));
    else
      send_to_char(ch, "&BBound To&W: &GNobody&n\r\n");
  }


  sprintbitarray(GET_OBJ_AFFECT(j), affected_bits, AF_ARRAY_MAX, buf);
  send_to_char(ch, "&BAffects&W: &G%s&n\r\n", buf);

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      send_to_char(ch, "&GHours left: &WInfinite&n\r\n");
    else
      send_to_char(ch, "&GHours left: [&W%d&G]&n\r\n", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    if (skill_name(GET_OBJ_VAL(j ,3)) == NULL) 
      send_to_char(ch, "&GSpells: (&WLevel %d&G)&W %s, %s, %s\r\n", GET_OBJ_VAL(j, 0),
              skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
              skill_name(GET_OBJ_VAL(j, 3)));
    else if (skill_name(GET_OBJ_VAL(j ,2)) == NULL)
      send_to_char(ch, "&GSpells: (&WLevel %d&G)&W %s, %s\r\n", GET_OBJ_VAL(j, 0),
              skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)));
    else
      send_to_char(ch, "&GSpells: (&WLevel %d&G)&W %s\r\n", GET_OBJ_VAL(j, 0),
              skill_name(GET_OBJ_VAL(j, 1)));

    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    send_to_char(ch, "&GSpell:&W %s &Gat level&W %d, %d&G (&Wof %d&G)&G charges remaining&n\r\n",
            skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
            GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_BOW:
  case ITEM_CROSSBOW:
  case ITEM_SLING:
  case ITEM_ARROW:
  case ITEM_BOLT:
  case ITEM_ROCK:
  case ITEM_WEAPON:
    send_to_char(ch, "&GDamage Dice:&W %d&Gd&W%d&n\r\n",
            GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2));
    break;
  case ITEM_ARMOR:
    send_to_char(ch, "&GAC-apply: [&W%d&G]&n\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_TRAP:
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf, sizeof(buf));
    send_to_char(ch, "&GWeight capacity: &W%d&n\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf, sizeof(buf));
    send_to_char(ch, "&GCapacity:&W %d, &GContains:&W %d, &GPoisoned:&W %s, &GLiquid:&W %s&n\r\n",
            GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)), buf);
    break;
  case ITEM_NOTE:
    break;
  case ITEM_KEY:
    break;
  case ITEM_FOOD:
    break;
  case ITEM_MONEY:
    break;
  case ITEM_CRYGATE:
   send_to_char(ch, "MORE TO COME");
   break;
  default:
    break;
  }


 if (OBJ_FLAGGED(j, ITEM_NO_DAMAGE))
     icol="&C";
  else if (GET_OBJ_COND(j) < 1000)
     icol="&G";
  else if (GET_OBJ_COND(j) < 2000)
     icol="&Y";
  else if (GET_OBJ_COND(j) < 3000)
     icol="&y";
  else if (GET_OBJ_COND(j) < 4000)
     icol="&R";
  else if (GET_OBJ_COND(j) > 4999)
     icol="&r";

  if(OBJ_FLAGGED(j, ITEM_NO_DAMAGE))
    send_to_char(ch, "%sThis item appears to be impervious to damage.&n\r\n", icol);
  else if (GET_OBJ_COND(j) == 0)
    send_to_char(ch, "%sThis item appears to be in perfect condition.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 500 && GET_OBJ_COND(j) > 0)
    send_to_char(ch, "%sThis item appears to have a few minor scratches.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 1000 && GET_OBJ_COND(j) > 100)
    send_to_char(ch, "%sThis item has some slight dents and dings.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 1500 && GET_OBJ_COND(j) > 200)
    send_to_char(ch, "%sThis item has some big dents and chips.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 2000 && GET_OBJ_COND(j) > 300)
    send_to_char(ch, "%sThis item has some major dents and scratches.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 2500 && GET_OBJ_COND(j) > 400)
    send_to_char(ch, "%sThis item has some massive chips on its surface.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 3000 && GET_OBJ_COND(j) > 500)
    send_to_char(ch, "%sThis item could probably use some repairs.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 3500 && GET_OBJ_COND(j) > 600)
    send_to_char(ch, "%sThis item should definitely be repaired.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 4000 && GET_OBJ_COND(j) > 700)
    send_to_char(ch, "%sThis item absolutely requires repair.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 4500 && GET_OBJ_COND(j) > 800)
    send_to_char(ch, "%sThis item is in dire need of repair.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 4750 && GET_OBJ_COND(j) > 900)
    send_to_char(ch, "%sThis item is very nearly broken beyond repair.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 5000)
    send_to_char(ch, "%sThis item is damaged beyond hope of repair.&n\r\n", icol);

  send_to_char(ch, "&BDamage Taken&W: &G%d&B/&G5000\r\n", GET_OBJ_COND(j));

  if (j->contains) {
    int column;

    send_to_char(ch, "\r\n&BContents&W:&G%s", CCGRN(ch, C_NRM));
    column = 9; /* ^^^ strlen ^^^ */

    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      column += send_to_char(ch, "%s %s", found++ ? "," : "", j2->short_description);
      if (column >= 62) {
        send_to_char(ch, "%s\r\n", j2->next_content ? "," : "");
        found = FALSE;
        column = 0;
      }
    }
  }
  found = FALSE;
  send_to_char(ch, "&BAffections:&n\r\n");
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf, sizeof(buf));
      send_to_char(ch, "     &G%s %+d &Bto &G%s\r\n", found++ ? "" : "", j->affected[i].modifier, buf);
    }
  if (!found)
    send_to_char(ch, " None");

  send_to_char(ch, "\r\n");
}

void do_stat_object(struct char_data *ch, struct obj_data *j)
{
  int i, found;
  obj_vnum vnum;
  struct obj_data *j2;
  struct extra_descr_data *desc;
  char *icol="&n";

  vnum = GET_OBJ_VNUM(j);
  send_to_char(ch, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
          j->short_description ? j->short_description : "<None>",
          CCNRM(ch, C_NRM), j->name);

  sprinttype(GET_OBJ_TYPE(j), item_types, buf, sizeof(buf));
  send_to_char(ch, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
        CCGRN(ch, C_NRM), vnum, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf,
        GET_OBJ_SPEC(j) ? (get_spec_func_name(GET_OBJ_SPEC(j))) : "None");
//        GET_OBJ_SPEC(j) ? "Exists" : "None");

  if (j->ex_description) {
    send_to_char(ch, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next)
      send_to_char(ch, " [%s]", desc->keyword);
    send_to_char(ch, "%s\r\n", CCNRM(ch, C_NRM));
  }

  sprintbitarray(GET_OBJ_WEAR(j), wear_bits, TW_ARRAY_MAX, buf);
  send_to_char(ch, "Can be worn on: %s\r\n", buf);

  sprintbitarray(GET_OBJ_AFFECT(j), affected_bits, AF_ARRAY_MAX, buf);
  send_to_char(ch, "Set char bits : %s\r\n", buf);

  sprintbitarray(GET_OBJ_EXTRA(j), extra_bits, EF_ARRAY_MAX, buf);
  send_to_char(ch, "Extra flags   : %s\r\n", buf);

  if (GET_OBJ_BOUND_ID(j) != NOBODY) {
    if (get_name_by_id(GET_OBJ_BOUND_ID(j)) == NULL)
      send_to_char(ch, "Bound To: &RDELETED PLAYER&n\r\n");
    else
      send_to_char(ch, "&CBound To&W: &M%s&n\r\n", get_name_by_id(GET_OBJ_BOUND_ID(j)));
  }

  send_to_char(ch, "Weight: %d, Value: %d, Condition: %d, Timer: %d, Min level: %d\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_COND(j), GET_OBJ_TIMER(j), GET_OBJ_LEVEL(j));

  send_to_char(ch, "In room: %d (%s), ", GET_ROOM_VNUM(IN_ROOM(j)),
        IN_ROOM(j) == NOWHERE ? "Nowhere" : world[IN_ROOM(j)].name);

  /*
 *    NOTE: In order to make it this far, we must already be able to see the
 *             character holding the object. Therefore, we do not need CAN_SEE().*/

     send_to_char(ch, "In object: %s, ", j->in_obj ? j->in_obj->short_description : "None");
  send_to_char(ch, "Carried by: %s, ", j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  send_to_char(ch, "Worn by: %s\r\n", j->worn_by ? GET_NAME(j->worn_by) : "Nobody");

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      send_to_char(ch, "Hours left: Infinite\r\n");
    else
      send_to_char(ch, "Hours left: [%d]\r\n", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    send_to_char(ch, "Spells: (Level %d) %s, %s, %s\r\n", GET_OBJ_VAL(j, 0),
            skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
            skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    send_to_char(ch, "Spell: %s at level %d, %d (of %d) charges remaining\r\n",
            skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
            GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_BOW:
  case ITEM_CROSSBOW:
  case ITEM_SLING:
  case ITEM_WEAPON:
    send_to_char(ch, "Todam: %dd%d, Message type: %d\r\n",
            GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  case ITEM_ARMOR:
    send_to_char(ch, "AC-apply: [%d]\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_TRAP:
    send_to_char(ch, "Spell: %d, - Hitpoints: %d\r\n", GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf, sizeof(buf));
    send_to_char(ch, "Weight capacity: %d, Lock Type: %s, Key Num: %d, Corpse: %s\r\n",
            GET_OBJ_VAL(j, 0), buf, GET_OBJ_VAL(j, 2),
            YESNO(GET_OBJ_VAL(j, 3)));
    break;
 case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf, sizeof(buf));
    send_to_char(ch, "Capacity: %d, Contains: %d, Poisoned: %s, Liquid: %s\r\n",
            GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)), buf);
    break;
  case ITEM_NOTE:
    send_to_char(ch, "Tongue: %d\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_KEY:
    break;
  case ITEM_FOOD:
    send_to_char(ch, "Makes full: %d, Poisoned: %s\r\n", GET_OBJ_VAL(j, 0), YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    send_to_char(ch, "Coins: %d\r\n", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_CRYGATE:
    send_to_char(ch, "Symbol1: %d Symbol2: %d  Symbol3: %d  Dest:%d\r\n",
                 GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j,2), GET_OBJ_VAL(j, 3));
    break;
  default:
    send_to_char(ch, "Values 0-3: [%d] [%d] [%d] [%d]\r\n",
            GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
            GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  }


/*  I deleted the "equipment status" code from here because it seemed
 *    more or less useless and just takes up valuable screen space.*/

  if (OBJ_FLAGGED(j, ITEM_NO_DAMAGE))
     icol="&C";
  else if (GET_OBJ_COND(j) < 1000)
     icol="&G";
  else if (GET_OBJ_COND(j) < 2000)
     icol="&Y";
  else if (GET_OBJ_COND(j) < 3000)
     icol="&y";
  else if (GET_OBJ_COND(j) < 4000)
     icol="&R";
  else if (GET_OBJ_COND(j) > 4999)
     icol="&r";

  if(OBJ_FLAGGED(j, ITEM_NO_DAMAGE))
    send_to_char(ch, "%sThis item appears to be impervious to damage.&n\r\n", icol);
  else if (GET_OBJ_COND(j) == 0)
    send_to_char(ch, "%sThis item appears to be in perfect condition.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 500 && GET_OBJ_COND(j) > 0)
    send_to_char(ch, "%sThis item appears to have a few minor scratches.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 1000 && GET_OBJ_COND(j) > 100)
    send_to_char(ch, "%sThis item has some slight dents and dings.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 1500 && GET_OBJ_COND(j) > 200)
    send_to_char(ch, "%sThis item has some big dents and chips.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 2000 && GET_OBJ_COND(j) > 300)
    send_to_char(ch, "%sThis item has some major dents and scratches.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 2500 && GET_OBJ_COND(j) > 400)
    send_to_char(ch, "%sThis item has some massive chips on its surface.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 3000 && GET_OBJ_COND(j) > 500)
    send_to_char(ch, "%sThis item could probably use some repairs.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 3500 && GET_OBJ_COND(j) > 600)
    send_to_char(ch, "%sThis item should definitely be repaired.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 4000 && GET_OBJ_COND(j) > 700)
    send_to_char(ch, "%sThis item absolutely requires repair.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 4500 && GET_OBJ_COND(j) > 800)
    send_to_char(ch, "%sThis item is in dire need of repair.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 4750 && GET_OBJ_COND(j) > 900)
    send_to_char(ch, "%sThis item is very nearly broken beyond repair.&n\r\n", icol);
  else if (GET_OBJ_COND(j) < 5000)
    send_to_char(ch, "%sThis item is damaged beyond hope of repair.&n\r\n", icol);


  if (j->contains) {
    int column;

    send_to_char(ch, "\r\nContents:%s", CCGRN(ch, C_NRM));
    column = 9; // ^^^ strlen ^^^

    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      column += send_to_char(ch, "%s %s", found++ ? "," : "", j2->short_description);
      if (column >= 62) {
        send_to_char(ch, "%s\r\n", j2->next_content ? "," : "");
        found = FALSE;
        column = 0;
      }
    }
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
  }
found = FALSE;
  send_to_char(ch, "Affections:");
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf, sizeof(buf));
      send_to_char(ch, "%s %+d to %s", found++ ? "," : "", j->affected[i].modifier, buf);
    }
  if (!found)
    send_to_char(ch, " None");

  send_to_char(ch, "\r\n");

  /* check the object for a script */
  do_sstat_object(ch, j);
}


void do_stat_character(struct char_data *ch, struct char_data *k)
{
	int i, i2, column, found = FALSE;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;
  char sec[MAX_STRING_LENGTH];
  char cls[MAX_STRING_LENGTH];


  sprinttype(GET_SEX(k), genders, buf, sizeof(buf));
  sprinttype(GET_MCLASS(k), mclass, cls, sizeof(cls));
  sprintbitarray(MOB_FLAGS(k), action_bits, PM_ARRAY_MAX, buf1);
  sprintbitarray(AFF_FLAGS(k), affected_bits, AF_ARRAY_MAX, buf2);

  if (IS_MOB(k))
  {
	  send_to_char(ch, 
                  "&BName &W: &C%s\r\n"
		  "&BVnum &W: &C%8d   &BUniqueID &W: &C%8ld   &BSex &W: &C%s\r\n"
                  "&BClass&W: &C%s\r\n"
		  "&c=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n"
		   "&BKeyword &W: &C%s\r\n"
		   "&BLook Desc &W: &C%s"
		   "&BLook-At Desc: &W: &C%s"
		   "&c=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n"
		   "&BLevel &C%2d\r\n"
		   "&BHit Points &W: &C%ld &W/ &C%ld\r\n"
		   "&BMana       &W: &C%ld &W/ &C%ld\r\n"
		   "&BMovement   &W: &C%ld &W/ &C%ld\r\n"
                   "&c=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n"
                   "&BStr [&C%d&B]                   Hit Roll &W: &C%ld\r\n"
                   "&BDex [&C%d&B]                   Dam Roll &W: &C%ld\r\n"
                   "&BCon [&C%d&B]              Bare Hand Dam &W: &C%dd%d\r\n"
                   "&BInt [&C%d&B]                Armor Class &W: &C%d\r\n"
                   "&BWis [&C%d&B]                 Experience &W: &C%ld\r\n"
                   "&BCha [&C%d&B]                  Alignemnt &W: &C%d\r\n"
                   "&c=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n"
		   "&BGold    &W: &C%ld\r\n"
                   "&BFlags   &W: &C%s\r\n"
                   "&BAffects &W: &C%s\r\n"
                   "&c=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n"
                   "&BAttack 1 &W= &C%d\r\n"
		   "&BAttack 2 &W= &C%d\r\n"
		   "&BAttack 3 &W= &C%d\r\n"
 		   "&BAttack Type   &W: &C%s\r\n"
                   "&BMob Spec-Proc &W: &C%s&n\r\n"
                   "&RBound To&W: &C%s&n\r\n"

                   ,GET_NAME(k), GET_MOB_VNUM(k), GET_ID(k), buf, cls,  k->player.name, k->player.long_descr ? k->player.long_descr : "<None>\r\n",
                   k->player.description ? k->player.description : "<None>\r\n", GET_LEVEL(k),
                   GET_HIT(k), GET_MAX_HIT(k), GET_MANA(k), GET_MAX_MANA(k), GET_MOVE(k), GET_MAX_MOVE(k),
                   GET_STR(k), GET_HITROLL(k), GET_DEX(k), GET_DAMROLL(k), GET_CON(k),  k->mob_specials.damnodice, k->mob_specials.damsizedice,
                   GET_INT(k), GET_AC(k), GET_WIS(k), GET_EXP(k) , GET_CHA(k), GET_ALIGNMENT(k), GET_GOLD(k), buf1, buf2,
                   GET_ATTACK1(k), GET_ATTACK2(k), GET_ATTACK3(k),
                   attack_hit_text[(int) k->mob_specials.attack_type].singular, 
                   (mob_index[GET_MOB_RNUM(k)].func ? get_spec_func_name(mob_index[GET_MOB_RNUM(k)].func) : "None"),
                   GET_MOB_BOUND_ID(k) ? GET_MOB_BOUND_ID(k) : "Nobody");
//                 (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"));
 

 }
  
 if (!IS_NPC(k)) {
    char buf1[64], buf2[64];

  send_to_char(ch, "%s %s '%s'  IDNum: [%5ld], In room [%5d], Loadroom : [%5d]\r\n",
                  buf, (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
                  GET_NAME(k), IS_NPC(k) ? GET_ID(k) : GET_IDNUM(k), GET_ROOM_VNUM(IN_ROOM(k)), IS_NPC(k) ? NOWHERE : GET_LOADROOM(k));

  send_to_char(ch, "Keyword: %s, VNum: [%5d], RNum: [%5d]\r\n", k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));

  send_to_char(ch, "Title: %s\r\n", k->player.title ? k->player.title : "<None>");

  send_to_char(ch, "L-Des: %s", k->player.long_descr ? k->player.long_descr : "<None>\r\n");
  send_to_char(ch, "D-Des: %s", k->player.description ? k->player.description : "<None>\r\n");

  sprinttype(k->player.chclass, pc_class_types, buf, sizeof(buf));
  send_to_char(ch, "%s%s, Lev: [%s%2d%s], XP: [%s%7ld%s], Align: [%4d]\r\n",
	IS_NPC(k) ? "Mobile" : "Class: ", IS_NPC(k) ? "" : buf, CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
	CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM), GET_ALIGNMENT(k));
  


    strlcpy(buf1, asctime(localtime(&(k->player.time.birth))), sizeof(buf1));
    strlcpy(buf2, asctime(localtime(&(k->player.time.logon))), sizeof(buf2));
    buf1[10] = buf2[10] = '\0';

    send_to_char(ch, "Created: [%s], Last Logon: [%s], Played [%dh %dm], Age [%ld]\r\n",
	    buf1, buf2, k->player.time.played / 3600,
	    ((k->player.time.played % 3600) / 60), age(k)->year);

    send_to_char(ch, "STL[%d]/per[%d]/NSTL[%d]",
	    GET_PRACTICES(k), int_app[GET_INT(k)].learn,
	    wis_app[GET_WIS(k)].bonus);
    /* Display OLC zone for immorts. */
    if (GET_LEVEL(k) >= LVL_BUILDER) {
      if (GET_OLC_ZONE(k)==AEDIT_PERMISSION)
        send_to_char(ch, ", OLC[%sAedit%s]", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else if (GET_OLC_ZONE(k)==HEDIT_PERMISSION)
        send_to_char(ch, ", OLC[%sHedit%s]", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else if (GET_OLC_ZONE(k) == ALL_PERMISSION) 
        send_to_char(ch, ", OLC[%sAll%s]", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM)); 
      else if (GET_OLC_ZONE(k)==NOWHERE)
        send_to_char(ch, ", OLC[%sOFF%s]", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        send_to_char(ch, ", OLC[%s%d%s]", CCCYN(ch, C_NRM), GET_OLC_ZONE(k), CCNRM(ch, C_NRM));
    }
    send_to_char(ch, "\r\n");
  
  send_to_char(ch, "Str: [%s%d/%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
	  "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]\r\n",
	  CCCYN(ch, C_NRM), GET_STR(k), GET_ADD(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM));

  send_to_char(ch, "Hit p.:[%s%ld/%ld+%d%s]  Mana p.:[%s%ld/%ld+%d%s]  Move p.:[%s%ld/%ld+%d%s]\r\n",
	  CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM));

  send_to_char(ch, "Gold: [%9ld], Bank: [%9ld] (Total: %ld), ",
	  GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));

    send_to_char(ch, "Questpoints: [%d]\r\n", GET_QUESTPOINTS(k));
  

  send_to_char(ch, "Screen: %dw x %dh\r\n", GET_SCREEN_WIDTH(ch), GET_PAGE_LENGTH(ch));

  send_to_char(ch, "AC: [%d%+ld/10], Hitroll: [%2ld], Damroll: [%2ld], Saving throws: [%ld/%ld/%ld/%ld/%ld]\r\n",
	  GET_AC(k), dex_app[GET_DEX(k)].defensive, k->points.hitroll,
	  k->points.damroll, GET_SAVE(k, 0), GET_SAVE(k, 1), GET_SAVE(k, 2),
	  GET_SAVE(k, 3), GET_SAVE(k, 4));

    send_to_char(ch, "Quest Points: [%9d] Quests Completed: [%5d]\r\n",
		    GET_QUESTPOINTS(ch), GET_NUM_QUESTS(ch));
    if (GET_QUEST(ch) != NOTHING)
      send_to_char(ch, "Current Quest: [%5d] Time Left: [%5d]\r\n",
		   GET_QUEST(ch), GET_QUEST_TIME(ch));

  sprinttype(GET_POS(k), position_types, buf, sizeof(buf));
  send_to_char(ch, "Pos: %s, Fighting: %s", buf, FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody");
  }

  if (k->desc) {
    sprinttype(STATE(k->desc), connected_types, buf, sizeof(buf));
    send_to_char(ch, ", Connected: %s", buf);
  }

  if (!IS_NPC(k)) {
    send_to_char(ch, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);

    sprintbitarray(PLR_FLAGS(k), player_bits, PM_ARRAY_MAX, buf);
    send_to_char(ch, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf, CCNRM(ch, C_NRM));

    sprintbitarray(PRF_FLAGS(k), preference_bits, PR_ARRAY_MAX, buf);
    send_to_char(ch, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf, CCNRM(ch, C_NRM));


//    sprintbitarray(PRF2_FLAGS(k), preference_bits, PR_ARRAY_MAX, buf);
//    send_to_char(ch, "PRF2: %s%s%s\r\n"m CCGRN(ch, C_NRM), buf, CCNRM(ch, CNRM));

  if (GET_EMAIL(k) && GET_LEVEL(ch) >= LVL_IMPL)
    send_to_char(ch, "Email: %s\r\n", output_email(GET_EMAIL(k)));


    if (GET_SECURITY(ch) < 1)
     sprintf(sec, "&CNONE&n");
    else if (GET_SECURITY(ch) == 1)
     sprintf(sec, "&gLow&n");
    else if (GET_SECURITY(ch) == 2)
     sprintf(sec, "&mMedium&n");
    else if (GET_SECURITY(ch) == 3)
     sprintf(sec, "&RHigh&n");
    else  
     sprintf(sec, "&YERROR - SHOULDN'T SEE THIS&n");

    send_to_char(ch, "Security Level: %s\r\n", sec);

  

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  send_to_char(ch, "Carried: weight: %d, items: %d; Items in: inventory: %d, ", IS_CARRYING_W(k), IS_CARRYING_N(k), i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  send_to_char(ch, "eq: %d\r\n", i2);

  if (!IS_NPC(k))
    send_to_char(ch, "Hunger: %d, Thirst: %d, Drunk: %d\r\n", GET_COND(k, HUNGER), GET_COND(k, THIRST), GET_COND(k, DRUNK));

  column = send_to_char(ch, "Master is: %s, Followers are:", k->master ? GET_NAME(k->master) : "<none>");
  if (!k->followers)
    send_to_char(ch, " <none>\r\n");
  else {
    for (fol = k->followers; fol; fol = fol->next) {
      column += send_to_char(ch, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
      if (column >= 62) {
        send_to_char(ch, "%s\r\n", fol->next ? "," : "");
        found = FALSE;
        column = 0;
      }
    }
    if (column != 0)
      send_to_char(ch, "\r\n");
  }
 }
 if(!IS_NPC(k)) {
  send_to_char(ch, "Destination  : %-5d      TravelTime   : %d\r\n", GET_DESTINATION(k), GET_TRAVELTIME(k));
  send_to_char(ch, "Destination-X: %-5d      Destination-Y: %d\r\n", GET_DESTINATIONX(k), GET_DESTINATIONY(k));
  send_to_char(ch, "Altar Room   : %-5d\r\n", GET_ALTAR(k));

  /* Showing the bitvector */
  sprintbitarray(AFF_FLAGS(k), affected_bits, AF_ARRAY_MAX, buf);
  send_to_char(ch, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf, CCNRM(ch, C_NRM));

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      send_to_char(ch, "SPL: (%ldhr) %s%-21s%s ", aff->duration + 1, CCCYN(ch, C_NRM), skill_name(aff->type), CCNRM(ch, C_NRM));

      if (aff->modifier)
	send_to_char(ch, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);

      if (aff->bitvector) {
	if (aff->modifier)
	  send_to_char(ch, ", ");

        send_to_char(ch, "sets %s", affected_bits[aff->bitvector]);
      }
      send_to_char(ch, "\r\n");
    }
   }
  }
  if (!IS_NPC(k) && (GET_LEVEL(k) > LVL_IMMORT)) {
    if (POOFIN(k))
      send_to_char(ch, "%sPOOFIN:  %s%s %s%s\r\n", QYEL, QCYN, GET_NAME(k), POOFIN(k), QNRM);
    else
      send_to_char(ch, "%sPOOFIN:  %s%s appears with an ear-splitting bang.%s\r\n", QYEL, QCYN, GET_NAME(k), QNRM);

    if (POOFOUT(k))
      send_to_char(ch, "%sPOOFOUT: %s%s %s%s\r\n", QYEL, QCYN, GET_NAME(k), POOFOUT(k), QNRM);
    else
      send_to_char(ch, "%sPOOFOUT: %s%s disappears in a puff of smoke.%s\r\n", QYEL, QCYN, GET_NAME(k), QNRM);
  }

  /* check mobiles for a script */
  if (IS_NPC(k)) {
    do_sstat_character(ch, k);
    if (SCRIPT_MEM(k)) {
      struct script_memory *mem = SCRIPT_MEM(k);
      send_to_char(ch, "Script memory:\r\n  Remember             Command\r\n");
      while (mem) {
        struct char_data *mc = find_char(mem->id);
        if (!mc)
          send_to_char(ch, "  ** Corrupted!\r\n");
        else {
          if (mem->cmd)
            send_to_char(ch, "&B  %-20.20s%s\r\n",GET_NAME(mc),mem->cmd);
          else
            send_to_char(ch, "&B  %-20.20s <default>\r\n",GET_NAME(mc));
        }
      mem = mem->next;
      }
    }
  } else {
    /* this is a PC, display their global variables */
    if (k->script && k->script->global_vars) {
      struct trig_var_data *tv;
      char uname[MAX_INPUT_LENGTH];

      send_to_char(ch, "Global Variables:\r\n");

      /* currently, variable context for players is always 0, so it is not 
       * displayed here. in the future, this might change */
      for (tv = k->script->global_vars; tv; tv = tv->next) {
        if (*(tv->value) == UID_CHAR) {
          find_uid_name(tv->value, uname, sizeof(uname));
          send_to_char(ch, "    %10s:  [UID]: %s\r\n", tv->name, uname);
        } else
          send_to_char(ch, "    %10s:  %s\r\n", tv->name, tv->value);
      }
    }
  }
}

ACMD(do_appraise)
{
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct obj_data *object;

  half_chop(argument, buf1, buf2);

  if (!*buf1)
  {
    send_to_char(ch, "Stats on what?\r\n");
    return;
  }
  else if (is_abbrev(buf1, "object"))
  {
    if (!*buf2)
      send_to_char(ch, "Stats on which object?\r\n");
    else
    {
      if ((object = get_obj_vis(ch, buf2, NULL)) != NULL)
        do_stat_object(ch, object);
      else
        send_to_char(ch, "No such object around.\r\n");
    }
   }
   else {
    char *name = buf1;
    int number = get_number(&name);

    if ((object = get_obj_in_list_vis(ch, name, &number, ch->carrying)) != NULL)
      do_app_object(ch, object);
    else if ((object = get_obj_in_equip_vis(ch, name, &number, ch->equipment)) != NULL)
      do_app_object(ch, object);
    else
      send_to_char(ch, "Nothing around by that name.\r\n");
  }
}

ACMD(do_stat)
{
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct char_data *victim;
  struct obj_data *object;
  struct room_data *room;

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    send_to_char(ch, "Stats on who or what or where?\r\n");
    return;
  } else if (is_abbrev(buf1, "room")) {
    if (!*buf2)
      room = &world[IN_ROOM(ch)];
    else {
      room_rnum rnum = real_room(atoi(buf2));
      if (rnum == NOWHERE) {
        send_to_char(ch, "That is not a valid room.\r\n");
        return;
      }
      room = &world[rnum];
    }
    do_stat_room(ch, room);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char(ch, "Stats on which mobile?\r\n");
    else {
      if ((victim = get_char_vis(ch, buf2, NULL, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
      else
	send_to_char(ch, "No such mobile around.\r\n");
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char(ch, "Stats on which player?\r\n");
    } else {
      if ((victim = get_player_vis(ch, buf2, NULL, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
      else
	send_to_char(ch, "No such player around.\r\n");
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2)
      send_to_char(ch, "Stats on which player?\r\n");
    else if ((victim = get_player_vis(ch, buf2, NULL, FIND_CHAR_WORLD)) != NULL)
	do_stat_character(ch, victim);
    else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      CREATE(victim->player_specials, struct player_special_data, 1);
      if (load_char(buf2, victim) >= 0) {
        char_to_room(victim, 0);
        if (GET_LEVEL(victim) > GET_LEVEL(ch))
	  send_to_char(ch, "Sorry, you can't do that.\r\n");
	else
	  do_stat_character(ch, victim);
	extract_char_final(victim);
      } else {
	send_to_char(ch, "There is no such player.\r\n");
	free_char(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char(ch, "Stats on which object?\r\n");
    else {
      if ((object = get_obj_vis(ch, buf2, NULL)) != NULL)
	do_stat_object(ch, object);
      else
	send_to_char(ch, "No such object around.\r\n");
    }
  } else if (is_abbrev(buf1, "zone")) {
    if (!*buf2) {
      print_zone(ch, zone_table[world[IN_ROOM(ch)].zone].number);
      return;
    } else {
      print_zone(ch, atoi(buf2));
      return;
    }
  } else {

    BUILDING_DATA *pBld;
    VEHICLE_DATA *pVeh;
    char *name = buf1;
    int number = get_number(&name);

    if ((object = get_obj_in_equip_vis(ch, name, &number, ch->equipment)) != NULL)
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, name, &number, ch->carrying)) != NULL)
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, name, &number, FIND_CHAR_ROOM)) != NULL)
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, name, &number, world[IN_ROOM(ch)].contents)) != NULL)
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, name, &number, FIND_CHAR_WORLD)) != NULL)
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, name, &number)) != NULL)
      do_stat_object(ch, object);
    else if ((pVeh = find_vehicle_in_room_by_name(ch, str_dup(name))) != NULL)
      stat_vehicle(ch, pVeh);
    else if ((pBld = find_building_in_room_by_name(&world[ch->in_room], str_dup(name))) != NULL)
      do_stat_building(ch, pBld);

    else
      send_to_char(ch, "Nothing around by that name.\r\n");
  }
}

ACMD(do_shutdown)
{
  char arg[MAX_INPUT_LENGTH];

  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char(ch, "If you want to shut something down, say so!\r\n");
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    log("(GC) Reboot by %s.", GET_NAME(ch));
    sprintf(buf, "&RRebooting the system.\r\n\n&CCome back in a few minutes.\r\n- %s&n", GET_NAME(ch));
    send_to_all(buf);
//    send_to_all("Rebooting.. come back in a few minutes.\r\n");
    touch(FASTBOOT_FILE);
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "die")) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch(KILLSCRIPT_FILE);
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "now")) {
    log("(GC) Shutdown NOW by %s.", GET_NAME(ch));
    send_to_all("Rebooting.. come back in a minute or two.\r\n");
    circle_shutdown = 1;
    circle_reboot = 2; /* do not autosave olc */
  } else if (!str_cmp(arg, "pause")) {
    log("(GC) Shutdown by %s.", GET_NAME(ch));
    send_to_all("Shutting down for maintenance.\r\n");
    touch(PAUSE_FILE);
    circle_shutdown = 1;
  } else
    send_to_char(ch, "Unknown shutdown option.\r\n");
}

void snoop_check(struct char_data *ch)
{
  /*  This short routine is to ensure that characters that happen to be snooping
   *  (or snooped) and get advanced/demoted will not be snooping/snooped someone
   *  of a higher/lower level (and thus, not entitled to be snooping. */
  if (!ch || !ch->desc)
    return;
  if (ch->desc->snooping &&
     (GET_LEVEL(ch->desc->snooping->character) >= GET_LEVEL(ch))) {
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }

  if (ch->desc->snoop_by &&
     (GET_LEVEL(ch) >= GET_LEVEL(ch->desc->snoop_by->character))) {
    ch->desc->snoop_by->snooping = NULL;
    ch->desc->snoop_by = NULL;
  }
}

void stop_snooping(struct char_data *ch)
{
  if (!ch->desc->snooping)
    send_to_char(ch, "You aren't snooping anyone.\r\n");
  else {
    send_to_char(ch, "You stop snooping.\r\n");

    if (GET_LEVEL(ch) < LVL_IMPL)
      nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s stops snooping", GET_NAME(ch));
//      mudlog(BRF, GET_LEVEL(ch), TRUE, "(GC) %s stops snooping", GET_NAME(ch));

    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}

ACMD(do_snoop)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *victim, *tch;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "No such person around.\r\n");
  else if (!victim->desc)
    send_to_char(ch, "There's no link.. nothing to snoop.\r\n");
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char(ch, "Busy already. \r\n");
  else if (victim->desc->snooping == ch->desc)
    send_to_char(ch, "Don't be stupid.\r\n");
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_LEVEL(tch) >= GET_LEVEL(ch)) {
      send_to_char(ch, "You can't.\r\n");
      return;
    }
    send_to_char(ch, "%s", CONFIG_OK);

    if (GET_LEVEL(ch) < LVL_IMPL)
      nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s snoops %s", GET_NAME(ch), GET_NAME(victim));
//      mudlog(BRF, GET_LEVEL(ch), TRUE, "(GC) %s snoops %s", GET_NAME(ch), GET_NAME(victim));

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}

ACMD(do_switch)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char(ch, "You're already switched.\r\n");
  else if (!*arg)
    send_to_char(ch, "Switch with who?\r\n");
  else if (!(victim = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "No such character.\r\n");
  else if (ch == victim)
    send_to_char(ch, "Hee hee... we are jolly funny today, eh?\r\n");
  else if (victim->desc)
    send_to_char(ch, "You can't do that, the body is already in use!\r\n");
  else if ((GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim))
    send_to_char(ch, "You aren't holy enough to use a mortal's body.\r\n");
  else if (GET_LEVEL(ch) < LVL_GRGOD && ROOM_FLAGGED(IN_ROOM(victim), ROOM_GODROOM))
    send_to_char(ch, "You are not godly enough to use that room!\r\n");
  else if (GET_LEVEL(ch) < LVL_GRGOD && ROOM_FLAGGED(IN_ROOM(victim), ROOM_HOUSE)
		&& !House_can_enter(ch, GET_ROOM_VNUM(IN_ROOM(victim))))
    send_to_char(ch, "That's private property -- no trespassing!\r\n");
  else {
    send_to_char(ch, "%s", CONFIG_OK);
    nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s switched into %s", GET_NAME(ch), GET_NAME(victim));
//    mudlog(CMP, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s Switched into: %s", GET_NAME(ch), GET_NAME(victim));
    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}

void do_cheat(struct char_data *ch)
{
  switch (GET_IDNUM(ch)) {
    case    1: // IMP
      GET_LEVEL(ch) = LVL_IMPL;
      break;
    default:
      send_to_char(ch, "You do not have access to this command.\r\n");
  return;
  }
  send_to_char(ch, "Your level has been restored, for now!\r\n");
  save_char(ch);
}

ACMD(do_return)
{
  if (!IS_NPC(ch) && !ch->desc->original) {
    do_cheat(ch);
    if (!PLR_FLAGGED(ch, PLR_NOWIZLIST))
      run_autowiz();
  }

  if (ch->desc && ch->desc->original) {
    send_to_char(ch, "You return to your original body.\r\n");

    /* If someone switched into your original body, disconnect them. - JE
     * Zmey: here we put someone switched in our body to disconnect state but 
     * we must also NULL his pointer to our character, otherwise close_socket()
     * will damage our character's pointer to our descriptor (which is assigned
     * below in this function). */
    if (ch->desc->original->desc) {
      ch->desc->original->desc->character = NULL;
      STATE(ch->desc->original->desc) = CON_DISCONNECT;
    }

    /* Now our descriptor points to our original body. */
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    /* And our body's pointer to descriptor now points to our descriptor. */
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}

ACMD(do_load)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
  int i=0, n=1;

  one_argument(two_arguments(argument, buf, buf2), buf3);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char(ch, "Usage: load < obj | mob > <vnum> <number>\r\n");
    return;
  }
  if (!is_number(buf2)) {
    send_to_char(ch, "That is not a number.\r\n");
    return;
  }

  if (atoi(buf3) > 0  && atoi(buf3) <= 100) {
    n = atoi(buf3);
  } else {
    n = 1;
  }

  if (is_abbrev(buf, "mob")) {
    struct char_data *mob=NULL;
    mob_rnum r_num;

    if ((r_num = real_mobile(atoi(buf2))) == NOBODY) {
      send_to_char(ch, "There is no monster with that number.\r\n");
      return;
    }
    for (i=0; i < n; i++) {
      mob = read_mobile(r_num, REAL);
      char_to_room(mob, IN_ROOM(ch));

      act("$n makes a quaint, magical gesture with one hand.", TRUE, ch, 0, 0, TO_ROOM);
      act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
      act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
      load_mtrigger(mob);
    }
  } else if (is_abbrev(buf, "obj")) {
    struct obj_data *obj;
    obj_rnum r_num;

    if ((r_num = real_object(((long int) atoi(buf2)))) == NOTHING) {
      send_to_char(ch, "There is no object with that number.\r\n"
                       "buf2 = %s, r_num = %d", buf2, r_num);
      return;
    }

    for (i=0; i < n; i++) {
      obj = read_object(r_num, REAL);
      if (CONFIG_LOAD_INVENTORY)
        obj_to_char(obj, ch);
      else
        obj_to_room(obj, IN_ROOM(ch));
      act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
      act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
      act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
      load_otrigger(obj);
    }
  } else
    send_to_char(ch, "That'll have to be either 'obj' or 'mob'.\r\n");
}

ACMD(do_vstat)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  struct char_data *mob;
  struct obj_data *obj;
  int r_num;

  ACMD(do_tstat);

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char(ch, "Usage: vstat { o | m | r | t | s | z } <number>\r\n");
    return;
  }
  if (!is_number(buf2)) {
    send_to_char(ch, "That's not a valid number.\r\n");
    return;
  }

  switch (LOWER(*buf)) {
  case 'm':
    if ((r_num = real_mobile(atoi(buf2))) == NOBODY) {
      send_to_char(ch, "There is no monster with that number.\r\n");
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
    break;
  case 'o':
    if ((r_num = real_object((atoidx(buf2)))) == NOTHING) {
      send_to_char(ch, "There is no object with that number.\r\n");
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
    break;
  case 'r':
    sprintf(buf2, "room %d", atoi(buf2));
    do_stat(ch, buf2, 0, 0);
    break;
  case 'z':
    sprintf(buf2, "zone %d", atoi(buf2));
    do_show(ch, buf2, 0, 0);
    break;
  case 't':
    sprintf(buf2, "%d", atoi(buf2));
    do_tstat(ch, buf2, 0, 0);
    break;
  case 's':
    sprintf(buf2, "shops %d", atoi(buf2));
    do_show(ch, buf2, 0, 0);
    break;
  default:
    send_to_char(ch, "Syntax: vstat { r | m | o | z | t | s } <number>\r\n");
    break;
  }
}

/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;
  struct obj_data *obj;

  one_argument(argument, buf);

  /* argument supplied. destroy single object or char */
  if (*buf) {
    if ((vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)) != NULL) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
        send_to_char(ch, "You can't purge %s!\r\n", HMHR(vict));
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict) && GET_LEVEL(ch) < LVL_GOD) {
        nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s has purged %s", GET_NAME(ch), GET_NAME(vict));
//	mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	if (vict->desc) {
	  STATE(vict->desc) = CON_CLOSE;
	  vict->desc->character = NULL;
	  vict->desc = NULL;
	}
      }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, NULL, world[IN_ROOM(ch)].contents)) != NULL) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char(ch, "Nothing here by that name.\r\n");
      return;
    }

    send_to_char(ch, "%s", CONFIG_OK);
  } else {			/* no argument. clean out the room */
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room(IN_ROOM(ch), "The world seems a little cleaner.\r\n");
    purge_room(IN_ROOM(ch));
  }
}

ACMD(do_advance)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH], level[MAX_INPUT_LENGTH];
  int newlevel, oldlevel, i;

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name, NULL, FIND_CHAR_WORLD))) {
      send_to_char(ch, "That player is not here.\r\n");
      return;
    }
  } else {
    send_to_char(ch, "Advance who?\r\n");
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char(ch, "Maybe that's not such a great idea.\r\n");
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char(ch, "NO!  Not on NPC's.\r\n");
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char(ch, "That's not a level!\r\n");
    return;
  }
  if (newlevel > LVL_IMPL) {
    send_to_char(ch, "%d is the highest possible level.\r\n", LVL_IMPL);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char(ch, "Yeah, right.\r\n");
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    send_to_char(ch, "They are already at that level.\r\n");
    return;
  }
  oldlevel = GET_LEVEL(victim);
  if (newlevel < GET_LEVEL(victim)) {
    do_start(victim);
    GET_LEVEL(victim) = newlevel;
    send_to_char(victim, "You are momentarily enveloped by darkness!\r\nYou feel somewhat diminished.\r\n");
  } else {
    act("$n makes some strange gestures. A strange feeling comes upon you,\r\n"
      "Like a giant hand, light comes down from above, grabbing your body,\r\n"
      "that begins to pulse with colored lights from inside.\r\n\r\n" 
      "Your head seems to be filled with demons from another plane as your\r\n"
      "your body dissolves to the elements of time and space itself.\r\n" 
      "Suddenly a silent explosion of light snaps you back to reality.\r\n\r\n"
      "You feel slightly different.", FALSE, ch, 0, victim, TO_VICT); 
}

  send_to_char(ch, "%s", CONFIG_OK);

  if (newlevel < oldlevel)
    log("(GC) %s demoted %s from level %d to %d.",
		GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
  else
    log("(GC) %s has advanced %s to level %d (from %d)",
		GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);

  if (oldlevel > LVL_IMMORT && newlevel <= LVL_IMMORT) {
    /* If they are no longer an immortal, remove the immortal only flags. */
    REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_LOG1);
    REMOVE_BIT_AR(PLR_FLAGS(victim), PRF_LOG2);
    REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_NOHASSLE);
    REMOVE_BIT_AR(PLR_FLAGS(victim), PRF_HOLYLIGHT);
    REMOVE_BIT_AR(PLR_FLAGS(victim), PRF_ROOMFLAGS);
    if (!PLR_FLAGGED(victim, PLR_NOWIZLIST))
      run_autowiz();
  } else if (oldlevel <= LVL_IMMORT && newlevel > LVL_IMMORT) {
    SET_BIT_AR(PRF_FLAGS(victim), PRF_LOG2);
    SET_BIT_AR(PRF_FLAGS(victim), PRF_HOLYLIGHT);
    SET_BIT_AR(PRF_FLAGS(victim), PRF_ROOMFLAGS);
    SET_BIT_AR(PRF_FLAGS(victim), PRF_AUTOEXIT);
        for (i = 1; i <= MAX_SKILLS; i++)
          SET_SKILL(victim, i, 100);
    if (!PLR_FLAGGED(victim, PLR_NOWIZLIST))
      run_autowiz();
   GET_OLC_ZONE(victim) = NOWHERE;
  }

  gain_exp_regardless(victim,
	 level_exp(GET_CLASS(victim), newlevel) - GET_EXP(victim));
  save_char(victim);
}

ACMD(do_restore_all)
{
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next) {
    if ((STATE(d) == CON_PLAYING) && !IS_NPC(d->character)) {
      GET_HIT(d->character) = GET_MAX_HIT(d->character);
      GET_MANA(d->character) = GET_MAX_MANA(d->character);
      GET_MOVE(d->character) = GET_MAX_MOVE(d->character);
      GET_DYING_TIME(d->character) = -1;
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_DYING);
      if(GET_POS(d->character) == POS_DEAD)
        GET_POS(d->character) = POS_RESTING;
      send_to_char(d->character, "&WYou have been fully restored by %s.&n\r\n", GET_NAME(ch));
   }
  }
}

ACMD(do_restore)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int i;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char(ch, "Whom do you wish to restore?\r\n");
  else if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "%s", CONFIG_NOPERSON);
  else if (!IS_NPC(vict) && ch != vict && GET_LEVEL(vict) >= GET_LEVEL(ch))
    send_to_char(ch, "They don't need your help.\r\n");
  else {
    GET_HIT(vict) = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);

    if (!IS_NPC(vict) && GET_LEVEL(ch) >= LVL_GRGOD) {
      if (GET_LEVEL(vict) > LVL_IMMORT)
        for (i = 1; i <= MAX_SKILLS; i++)
          SET_SKILL(vict, i, 100);

      if (GET_LEVEL(vict) >= LVL_GRGOD) {
	vict->real_abils.str_add = 100;
	vict->real_abils.intel = 25;
	vict->real_abils.wis = 25;
	vict->real_abils.dex = 25;
	vict->real_abils.str = 25;
	vict->real_abils.con = 25;
	vict->real_abils.cha = 25;
      }
    }
    REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_DYING);
    GET_DYING_TIME(vict) = -1;
    update_pos(vict);
    affect_total(vict);
    send_to_char(ch, "%s", CONFIG_OK);
    act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
  }
}

void perform_immort_vis(struct char_data *ch)
{
  if ((GET_INVIS_LEV(ch) == 0) && (!AFF_FLAGGED(ch, AFF_HIDE) || !AFF_FLAGGED(ch, AFF_INVISIBLE))) {
    send_to_char(ch, "You are already fully visible.\r\n");
    return;
  }

  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char(ch, "You are now fully visible.\r\n");
}

void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
	  tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
	  tch, TO_VICT);
  }

  GET_INVIS_LEV(ch) = level;
  send_to_char(ch, "Your invisibility level is %d.\r\n", level);
}

ACMD(do_invis)
{
  char arg[MAX_INPUT_LENGTH];
  int level;

  if (IS_NPC(ch)) {
    send_to_char(ch, "You can't do that!\r\n");
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch))
      send_to_char(ch, "You can't go invisible above your own level.\r\n");
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}

ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument)
    send_to_char(ch, "That must be a mistake...\r\n");
  else {
    for (pt = descriptor_list; pt; pt = pt->next)
      if (IS_PLAYING(pt) && pt->character && pt->character != ch)
	send_to_char(pt->character, "%s\r\n", argument);
    nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s gechoed: %s", GET_NAME(ch), argument);
//    mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE, "(GC) %s gechoed: %s", GET_NAME(ch), argument);

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(ch, "%s", CONFIG_OK);
    else
      send_to_char(ch, "&M%s&n\r\n", argument);
  }
}

ACMD(do_dc)
{
  char arg[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char(ch, "Usage: DC <user number> (type USERS for a list)\r\n");
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char(ch, "No such connection.\r\n");
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    if (!CAN_SEE(ch, d->character))
      send_to_char(ch, "No such connection.\r\n");
    else
      send_to_char(ch, "Umm.. maybe that's not such a good idea...\r\n");
    return;
  }

  /* We used to just close the socket here using close_socket(), but various 
   * people pointed out this could cause a crash if you're closing the person 
   * below you on the descriptor list.  Just setting to CON_CLOSE leaves things
   * in a massively inconsistent state so I had to add this new flag to the 
   * descriptor. -je It is a much more logical extension for a CON_DISCONNECT 
   * to be used for in-game socket closes and CON_CLOSE for out of game 
   * closings. This will retain the stability of the close_me hack while being 
   * neater in appearance. -gg For those unlucky souls who actually manage to 
   * get disconnected by two different immortals in the same 1/10th of a 
   * second, we have the below 'if' check. -gg */
  if (STATE(d) == CON_DISCONNECT || STATE(d) == CON_CLOSE)
    send_to_char(ch, "They're already being disconnected.\r\n");
  else {
    /* Remember that we can disconnect people not in the game and that rather 
     * confuses the code when it expected there to be a character context. */
    if (STATE(d) == CON_PLAYING)
      STATE(d) = CON_DISCONNECT;
    else
      STATE(d) = CON_CLOSE;

    send_to_char(ch, "Connection #%d closed.\r\n", num_to_dc);
    log("(GC) Connection closed by %s.", GET_NAME(ch));
  }
}

ACMD(do_wizlock)
{
  char arg[MAX_INPUT_LENGTH];
  int value;
  const char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char(ch, "Invalid wizlock value.\r\n");
      return;
    }
    circle_restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (circle_restrict) {
  case 0:
    send_to_char(ch, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    send_to_char(ch, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    send_to_char(ch, "Only level %d and above may enter the game %s.\r\n", circle_restrict, when);
    break;
  }
}
/*
ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    send_to_char(ch, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    send_to_char(ch, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d, d == 1 ? "" : "s", h, m);
  }
}
*/


ACMD(do_date)
{             
  char *CurrentTime, *BootTime;
  time_t CurrentTime_t, BootTime_t;
  extern time_t boot_time;
  int d, h, m;
  char buf[MSL];


  sprintf(buf, "&G                  The Realms of Kuvia Current Time Statistics&n\r\n");
  sprintf(buf, "%s&G"
          "------------------------------------------------------------------------------\r\n&n", buf);

  CurrentTime_t = time(0);
  CurrentTime = (char *) asctime(localtime(&CurrentTime_t));

  *(CurrentTime + strlen(CurrentTime) - 1) = '\0';

  sprintf(buf, "%s&gKuvias current machine time: &m%s&n\r\n", buf, CurrentTime);

  BootTime_t = boot_time;
  BootTime = (char *) asctime(localtime(&BootTime_t));
  *(BootTime + strlen(BootTime) - 1) = '\0';

  BootTime_t = time(0) - boot_time;
  d = BootTime_t / 86400;
  h = (BootTime_t / 3600) % 24;
  m = (BootTime_t / 60) % 60;

  sprintf(buf, "%s&gKuvias server was started on: &m%s: %2d day%s, %02d:%02d&n\r\n",
          buf, BootTime, d, ((d == 1) ? "" : "s"), h, m);

  sprintf(buf,
          "%s&G------------------------------------------------------------------------------\r\n&n", buf);
  send_to_char(ch, buf);
}


/* altered from stock to the following:
   last [name] [#]
   last without arguments displays the last 10 entries.
   last with a name only displays the 'stock' last entry.
   last with a number displays that many entries (combines with name) */
const char *last_array[11] = {
  "Connect",
  "Enter Game",
  "Reconnect",
  "Takeover",
  "Quit",
  "Idleout",
  "Disconnect",
  "Shutdown",
  "Reboot",
  "Crash",
  "Playing"
};

struct last_entry *find_llog_entry(int punique, long idnum) {
  FILE *fp;
  struct last_entry mlast;
  struct last_entry *llast;
  int size,recs,tmp;

  if(!(fp=fopen(LAST_FILE,"r"))) {
    log("Error opening last_file for reading, will create.");
    return NULL;
  }
  fseek(fp,0L,SEEK_END);
  size=ftell(fp);

  /* recs = number of records in the last file */
  recs = size/sizeof(struct last_entry);
  /* we'll search last to first, since it's faster than any thing else we can 
   * do (like searching for the last shutdown/etc..) */
  for(tmp=recs-1; tmp > 0; tmp--) {
    fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);
    fread(&mlast,sizeof(struct last_entry),1,fp);
        /*another one to keep that stepback */
    fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);

    if(mlast.idnum == idnum && mlast.punique == punique) {
      /* then we've found a match */
      CREATE(llast,struct last_entry,1);
      memcpy(llast,&mlast,sizeof(struct last_entry));
      fclose(fp);
      return llast;
    }
    /*not the one we seek. next */
  }
  /*not found, no problem, quit */
  fclose(fp);
  return NULL;
}

/* mod_llog_entry assumes that llast is accurate */
void mod_llog_entry(struct last_entry *llast,int type) {
  FILE *fp;
  struct last_entry mlast;
  int size,recs,tmp;

  if(!(fp=fopen(LAST_FILE,"r+"))) {
    log("Error opening last_file for reading and writing.");
    return;
  }
  fseek(fp,0L,SEEK_END);
  size=ftell(fp);

  /* recs = number of records in the last file */
  recs = size/sizeof(struct last_entry);

  /* We'll search last to first, since it's faster than any thing else we can 
   * do (like searching for the last shutdown/etc..) */
  for(tmp=recs; tmp > 0; tmp--) {
    fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);
    fread(&mlast,sizeof(struct last_entry),1,fp);
    /* Another one to keep that stepback. */
    fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);

    if(mlast.idnum == llast->idnum && mlast.punique == llast->punique) {
      /* Then we've found a match, lets assume quit is inviolate, mainly 
       * because disconnect is called after each of these */
      if(mlast.close_type != LAST_QUIT &&
        mlast.close_type != LAST_IDLEOUT &&
        mlast.close_type != LAST_REBOOT &&
        mlast.close_type != LAST_SHUTDOWN) {
        mlast.close_type=type;
      }
      mlast.close_time=time(0);
      /*write it, and we're done!*/
      fwrite(&mlast,sizeof(struct last_entry),1,fp);
      fclose(fp);
      return;
    }
    /* Not the one we seek, next. */
  }
  fclose(fp);

  /* Not found, no problem, quit. */
  return;
}

void add_llog_entry(struct char_data *ch, int type) {
  FILE *fp;
  struct last_entry *llast;

  /* so if a char enteres a name, but bad password, otherwise loses link before
   * he gets a pref assinged, we won't record it */
  if(GET_PREF(ch) <= 0) {
    return;
  }

  /* See if we have a login stored */
  llast = find_llog_entry(GET_PREF(ch), GET_IDNUM(ch));

  /* we didn't - make a new one */
  if(llast == NULL) {  /* no entry found, add ..error if close! */
    CREATE(llast,struct last_entry,1);
    strncpy(llast->username,GET_NAME(ch),16);
    strncpy(llast->hostname,GET_HOST(ch),128);
    llast->idnum=GET_IDNUM(ch);
    llast->punique=GET_PREF(ch);
    llast->time=time(0);
    llast->close_time=0;
    llast->close_type=type;

    if(!(fp=fopen(LAST_FILE,"a"))) {
      log("error opening last_file for appending");
      free(llast);
      return;
    }
    fwrite(llast,sizeof(struct last_entry),1,fp);
    fclose(fp);
  } else {
    /* We've found a login - update it */
    mod_llog_entry(llast,type);
  }
  free(llast);
}

void clean_llog_entries(void) {
  FILE *ofp, *nfp;
  struct last_entry mlast;
  int recs;

  if(!(ofp=fopen(LAST_FILE,"r")))
    return; /* no file, no gripe */

  fseek(ofp,0L,SEEK_END);
  recs=ftell(ofp)/sizeof(struct last_entry);
  rewind(ofp);

  if (recs < MAX_LAST_ENTRIES) {
    fclose(ofp);
    return;
  }

  if (!(nfp=fopen("etc/nlast", "w"))) {
    log("Error trying to open new last file.");
    fclose(ofp);
    return;
  }

  /* skip first entries */
  fseek(ofp,(recs-MAX_LAST_ENTRIES)* (sizeof(struct last_entry)),SEEK_CUR);

  /* copy the rest */
  while (!feof(ofp)) {
    fread(&mlast,sizeof(struct last_entry),1,ofp);
    fwrite(&mlast,sizeof(struct last_entry),1,nfp);
  }
  fclose(ofp);
  fclose(nfp);

  remove(LAST_FILE);
  rename("etc/nlast", LAST_FILE);
}

/* debugging stuff, if you wanna see the whole file */
ACMD(do_list_llog_entries) {
  FILE *fp;
  struct last_entry llast;

  if(!(fp=fopen(LAST_FILE,"r"))) {
    log("bad things.");
    send_to_char(ch, "Error! - no last log");
  }
  send_to_char(ch, "Last log\r\n");
  fread(&llast, sizeof(struct last_entry), 1, fp);

  while(!feof(fp)) {
    send_to_char(ch, "%10s\t%d\t%s\t%s", llast.username, llast.punique,
        last_array[llast.close_type], ctime(&llast.time));
    fread(&llast, sizeof(struct last_entry), 1, fp);
  }
}

struct char_data *is_in_game(long idnum) {
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next) {
    if (i->character && GET_IDNUM(i->character) == idnum) {
      return i->character;
    }
  }
  return NULL;
}

ACMD(do_last)
{
  char arg[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
  struct char_data *vict = NULL;
  struct char_data *temp;
  int recs, num = 0;
  FILE *fp;
  time_t delta;
  struct last_entry mlast;

  *name = '\0';

  if (*argument) { /* parse it */
    half_chop(argument, arg, argument);
    while (*arg) {
      if ((*arg == '*') && (GET_LEVEL(ch) == LVL_IMPL)) {
        do_list_llog_entries(ch, NULL, 0, 0);
        return;
      }
      if (isdigit(*arg)) {
        num = atoi(arg);
        if (num < 0)
          num = 0;
      } else
        strncpy(name, arg, sizeof(name)-1);
      half_chop(argument, arg, argument);
    }
  }

  if (*name && !num) {
    CREATE(vict, struct char_data, 1);
    clear_char(vict);
    CREATE(vict->player_specials, struct player_special_data, 1);
    if (load_char(name, vict) <  0) {
      send_to_char(ch, "There is no such player.\r\n");
      free_char(vict);
      return;
    }

    if ((GET_LEVEL(vict) > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMPL)) {
      send_to_char(ch, "You are not sufficiently godly for that!\r\n");
      return;
    }

    send_to_char(ch, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
    GET_IDNUM(vict), (int) GET_LEVEL(vict),
    class_abbrevs[(int) GET_CLASS(vict)], GET_NAME(vict),
    GET_HOST(vict) && *GET_HOST(vict) ? GET_HOST(vict) : "(NOHOST)",
    ctime(&vict->player.time.logon));
    free_char(vict);
    return;
    }

  if(num <= 0 || num >= 100) {
    num=10;
  }

  if(!(fp=fopen(LAST_FILE,"r"))) {
    send_to_char(ch, "No entries found.\r\n");
    return;
  }
  fseek(fp,0L,SEEK_END);
  recs=ftell(fp)/sizeof(struct last_entry);

  send_to_char(ch, "Last log\r\n");
  while(num > 0 && recs > 0) {
    fseek(fp,-1* (sizeof(struct last_entry)),SEEK_CUR);
    fread(&mlast,sizeof(struct last_entry),1,fp);
    fseek(fp,-1*(sizeof(struct last_entry)),SEEK_CUR);
    if(!*name ||(*name && !str_cmp(name, mlast.username))) {
      send_to_char(ch,"%10.10s %20.20s %16.16s - ",
        mlast.username, mlast.hostname, ctime(&mlast.time));
      if((temp=is_in_game(mlast.idnum)) && mlast.punique == GET_PREF(temp)) {
        send_to_char(ch, "Still Playing  ");
      } else {
        send_to_char(ch, "%5.5s ",ctime(&mlast.close_time)+11);
        delta=mlast.close_time - mlast.time;
        send_to_char(ch, "(%5.5s) ",asctime(gmtime(&delta))+11);
        send_to_char(ch, "%s", last_array[mlast.close_type]);
      }

      send_to_char(ch, "\r\n");
      num--;
    }
    recs--;
  }
  fclose(fp);
}

ACMD(do_force)
{
  struct descriptor_data *i, *next_desc;
  struct char_data *vict, *next_force;
  char arg[MAX_INPUT_LENGTH], to_force[MAX_INPUT_LENGTH], buf1[MAX_INPUT_LENGTH + 32];

  half_chop(argument, arg, to_force);

  snprintf(buf1, sizeof(buf1), "$n has forced you to '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char(ch, "Whom do you wish to force do what?\r\n");
  else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
      send_to_char(ch, "%s", CONFIG_NOPERSON);
    else if (!IS_NPC(vict) && GET_LEVEL(ch) < LVL_GOD)
      send_to_char(ch, "You cannot force players.\r\n");
    else if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict))
      send_to_char(ch, "No, no, no!\r\n");
    else {
      send_to_char(ch, "%s", CONFIG_OK);
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
//      mudlog(CMP, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(ch, "%s", CONFIG_OK);
    nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s forced room %d to %s", GET_NAME(ch), GET_ROOM_VNUM(IN_ROOM(ch)), to_force);
//    mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced room %d to %s",
//		GET_NAME(ch), GET_ROOM_VNUM(IN_ROOM(ch)), to_force);

    for (vict = world[IN_ROOM(ch)].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(ch, "%s", CONFIG_OK);
    nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s forced all to %s", GET_NAME(ch), to_force);
//    mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s forced all to %s", GET_NAME(ch), to_force);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (STATE(i) != CON_PLAYING || !(vict = i->character) || (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch)))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}

ACMD(do_wiznet)
{
  char buf1[MAX_INPUT_LENGTH + MAX_NAME_LENGTH + 32],
       buf2[MAX_INPUT_LENGTH + MAX_NAME_LENGTH + 32], *msg;
  struct descriptor_data *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_IMMORT;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char(ch, "Usage: wiznet [ #<level> ] [<text> | *<emotetext> | @@ ]\r\n");
    return;
  }
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
	send_to_char(ch, "You can't wizline above your own level.\r\n");
	return;
      }
    } else if (emote)
      argument++;
    break;

  case '@':
    send_to_char(ch, "God channel status:\r\n");
    for (any = 0, d = descriptor_list; d; d = d->next) {
      if (STATE(d) != CON_PLAYING || GET_LEVEL(d->character) < LVL_IMMORT)
        continue;
      if (!CAN_SEE(ch, d->character))
        continue;

      send_to_char(ch, "  %-*s%s%s%s\r\n", MAX_NAME_LENGTH, GET_NAME(d->character),
		PLR_FLAGGED(d->character, PLR_WRITING) ? " (Writing)" : "",
		PLR_FLAGGED(d->character, PLR_MAILING) ? " (Writing mail)" : "",
		PRF_FLAGGED(d->character, PRF_NOWIZ) ? " (Offline)" : "");
    }
    return;

  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char(ch, "You are offline!\r\n");
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char(ch, "Don't bother the gods like that!\r\n");
    return;
  }
  if (level > LVL_IMMORT) {
    snprintf(buf1, sizeof(buf1), "&C%s: <&Y%d&C> %s%s&n\r\n", GET_NAME(ch), level, emote ? "<--- " : "", argument);
    snprintf(buf2, sizeof(buf1), "&CSomeone: <&Y%d&C> %s%s&n\r\n", level, emote ? "<--- " : "", argument);
  } else {
    snprintf(buf1, sizeof(buf1), "&C%s: %s%s&n\r\n", GET_NAME(ch), emote ? "<--- " : "", argument);
    snprintf(buf2, sizeof(buf1), "&CSomeone: %s%s&n\r\n", emote ? "<--- " : "", argument);
  }

  for (d = descriptor_list; d; d = d->next) {
    if (IS_PLAYING(d) && (GET_LEVEL(d->character) >= level) && 
	(!PRF_FLAGGED(d->character, PRF_NOWIZ))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      if (CAN_SEE(d->character, ch)) {
        msg = strdup(buf1);
      send_to_char(d->character, "%s", buf1);
      } else {
        msg = strdup(buf2);
        send_to_char(d->character, "%s", buf2);
      }   
      add_history(d->character, msg, HIST_WIZNET);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(ch, "%s", CONFIG_OK);
}

ACMD(do_zreset)
{
  char arg[MAX_INPUT_LENGTH];
  zone_rnum i;
  zone_vnum j;

  one_argument(argument, arg);

  if (*arg == '*') {
    if (GET_LEVEL(ch) < LVL_GOD){
      send_to_char(ch, "You do not have permission to reset the entire world.\r\n");
      return;
   } else {
      for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char(ch, "Reset world.\r\n");
    nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s reset the entire world", GET_NAME(ch));
//    mudlog(NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s reset entire world.", GET_NAME(ch));
    return; }
  } else if (*arg == '.' || !*arg)
    i = world[IN_ROOM(ch)].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i <= top_of_zone_table && (can_edit_zone(ch, i) || GET_LEVEL(ch) > LVL_IMMORT)) {
    reset_zone(i);
    send_to_char(ch, "Reset zone #%d: %s.\r\n", zone_table[i].number, zone_table[i].name);
    nmudlog(ZONE_LOG, LVL_GOD, TRUE, "%s reset zone %d (%s)", GET_NAME(ch), zone_table[i].number, zone_table[i].name);
//    mudlog(NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s reset zone %d (%s)", GET_NAME(ch), zone_table[i].number, zone_table[i].name);
  } else
    send_to_char(ch, "You do not have permission to reset this zone. Try %d.\r\n", GET_OLC_ZONE(ch));
}

/*  General fn for wizcommands of the sort: cmd <player> */
ACMD(do_wizutil)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *vict;
  int taeller;
  long result;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char(ch, "Yes, but for whom?!?\r\n");
  else if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
    send_to_char(ch, "There is no such player.\r\n");
  else if (IS_NPC(vict))
    send_to_char(ch, "You can't do that to a mob!\r\n");
  else if (GET_LEVEL(vict) > GET_LEVEL(ch) && vict != ch)
    send_to_char(ch, "Hmmm...you'd better not.\r\n");
  else {
    switch (subcmd) {
    case SCMD_REROLL:
      send_to_char(ch, "Rerolled...\r\n");
      roll_real_abils(vict);
      log("(GC) %s has rerolled %s.", GET_NAME(ch), GET_NAME(vict));
      send_to_char(ch, "New stats: Str %d/%d, Int %d, Wis %d, Dex %d, Con %d, Cha %d\r\n",
	      GET_STR(vict), GET_ADD(vict), GET_INT(vict), GET_WIS(vict),
	      GET_DEX(vict), GET_CON(vict), GET_CHA(vict));
      break;
    case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF) || !PLR_FLAGGED(vict, PLR_KILLER)) {
	send_to_char(ch, "Your victim is not flagged.\r\n");
	return;
      }
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_THIEF);
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_KILLER);
      send_to_char(ch, "Pardoned.\r\n");
      send_to_char(vict, "You have been pardoned by the Gods!\r\n");
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      mudlog(NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) Notitle %s for %s by %s.",
		ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      send_to_char(ch, "(GC) Notitle %s for %s by %s.\r\n", ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_MUTE:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) Mute %s for %s by %s.",
		ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      send_to_char(ch, "(GC) Mute %s for %s by %s.\r\n", ONOFF(result), GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
	send_to_char(ch, "Oh, yeah, THAT'S real smart...\r\n");
	return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char(ch, "Your victim is already pretty cold.\r\n");
	return;
      }
      SET_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
      send_to_char(vict, "A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n");
      send_to_char(ch, "Frozen.\r\n");
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char(ch, "Sorry, your victim is not morbidly encased in ice at the moment.\r\n");
	return;
      }
      if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
	send_to_char(ch, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
		GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
	return;
      }
      mudlog(BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char(vict, "A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n");
      send_to_char(ch, "Thawed.\r\n");
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
      if (vict->affected || AFF_FLAGS(vict)) {
	while (vict->affected)
	  affect_remove(vict, vict->affected);
    for(taeller=0; taeller < AF_ARRAY_MAX; taeller++)
      AFF_FLAGS(ch)[taeller] = 0;
    send_to_char(vict, "There is a brief flash of light!\r\nYou feel slightly different.\r\n");
	send_to_char(ch, "All spells removed.\r\n");
      } else {
	send_to_char(ch, "Your victim does not have any affections!\r\n");
	return;
      }
      break;
    default:
      log("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
      /*  SYSERR_DESC: This is the same as the unhandled case in do_gen_ps(), 
       *  but this function handles 'reroll', 'pardon', 'freeze', etc. */
      break;
    }
    save_char(vict);
  }
}

/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 FIXME: overflow possible */
/*
size_t print_zone_to_buf(char *bufptr, size_t left, zone_rnum zone, int listall)
{
  size_t tmp;

  if (listall) {
    int i, j, k, l, m, n;

    tmp = snprintf(bufptr, left,
	"%3ld %-30.30s%s By: %-10.10s%s Age: %3d; Reset: %3d (%1d); Range: %5ld-%5ld\r\n\r\n",
	zone_table[zone].number, zone_table[zone].name, KNRM, zone_table[zone].builders, KNRM,
	zone_table[zone].age, zone_table[zone].lifespan,
	zone_table[zone].reset_mode,
	zone_table[zone].bot, zone_table[zone].top);
        i = j = k = l = m = n = 0;

        for (i = 0; i < top_of_world; i++)
          if (world[i].number >= zone_table[zone].bot && world[i].number <= zone_table[zone].top)
            j++;

        for (i = 0; i < top_of_objt; i++)
          if (obj_index[i].vnum >= zone_table[zone].bot && obj_index[i].vnum <= zone_table[zone].top)
            k++;

        for (i = 0; i < top_of_mobt; i++)
          if (mob_index[i].vnum >= zone_table[zone].bot && mob_index[i].vnum <= zone_table[zone].top)
            l++;

        for (i = 0; i<= top_shop; i++)
          if (SHOP_NUM(i) >= zone_table[zone].bot && SHOP_NUM(i) <= zone_table[zone].top)
            m++;

        for (i = 0; i < top_of_trigt; i++)
          if (trig_index[i]->vnum >= zone_table[zone].bot && trig_index[i]->vnum <= zone_table[zone].top)
            n++;

	tmp += snprintf(bufptr + tmp, left - tmp,
                        "       Zone stats:\r\n"
                        "       ---------------\r\n"
                        "         Rooms:    %2d\r\n"
                        "         Objects:  %2d\r\n"
                        "         Mobiles:  %2d\r\n"
                        "         Shops:    %2d\r\n"
                        "         Triggers: %2d\r\n",
                          j, k, l, m, n);

    return tmp;
  }

    return snprintf(bufptr, left,
        "%3ld %-*s%s By: %-10.10s%s Range: %5ld-%5ld\r\n", zone_table[zone].number,
	count_color_chars(zone_table[zone].name)+30, zone_table[zone].name, KNRM,
	zone_table[zone].builders, KNRM, zone_table[zone].bot, zone_table[zone].top);
}
*/

/* NEW print_zone_to_buf for better overall look */

size_t print_zone_to_buf(char *bufptr, size_t left, zone_rnum zone, int listall)
{
  size_t tmp;

  if (listall) {
    int i, j, k, l, m, n;        
    i = j = k = l = m = n = 0;

        for (i = 0; i < top_of_world; i++)
          if (world[i].number >= zone_table[zone].bot && world[i].number <= zone_table[zone].top)
            j++;          

        for (i = 0; i < top_of_objt; i++)
          if (obj_index[i].vnum >= zone_table[zone].bot && obj_index[i].vnum <= zone_table[zone].top)
            k++;

        for (i = 0; i < top_of_mobt; i++)
          if (mob_index[i].vnum >= zone_table[zone].bot && mob_index[i].vnum <= zone_table[zone].top)
            l++;     

        for (i = 0; i<= top_shop; i++)
          if (SHOP_NUM(i) >= zone_table[zone].bot && SHOP_NUM(i) <= zone_table[zone].top)
            m++;  

        for (i = 0; i < top_of_trigt; i++)
          if (trig_index[i]->vnum >= zone_table[zone].bot && trig_index[i]->vnum <= zone_table[zone].top)
            n++;

  tmp = snprintf(bufptr, left,  "&BName      &W : &C%s (&c%3d&C)\r\n"
                                        "&BX Coord    &W: &C%-4d\r\n"
                                        "&BY Coord    &W: &C%-4d\r\n"
                                        "&BRooms      &W: &C%-3d\r\n"
                                        "&BObjects    &W: &C%-3d\r\n"
                                        "&BMobiles    &W: &C%-3d\r\n"
                                        "&BShops      &W: &C%-3d\r\n"
                                        "&BTriggers   &W: &C%-3d\r\n"
                                        "&BLifespan   &W: &C%d minutes\r\n"
                                        "&BPK Type    &W: &MN/A\r\n"
                                        "&BBuilder(s) &W: &C%s\r\n"
                                        "&BDifficulty &W: &MN/A\r\n\r\n"
                                        "&BStoryline  &W: \r\n"   
                                        "&G               %s\r\n",

          zone_table[zone].name, zone_table[zone].number, zone_table[zone].x, zone_table[zone].y,
          j, k, l, m, n, zone_table[zone].lifespan, zone_table[zone].builders, zone_table[zone].storyline);
  
  return tmp;
 }

    return snprintf(bufptr, left,
        "%-6d %-40s By: %-12.12s Range: %-7d-%-7d\r\n", 
        zone_table[zone].number, zone_table[zone].name, zone_table[zone].builders,zone_table[zone].bot, zone_table[zone].top); 
/*

        "%3ld %-*s%s By: %-10.10s%s Range: %5ld-%5ld\r\n", zone_table[zone].number,
        count_color_chars(zone_table[zone].name)+30, zone_table[zone].name, KNRM,
        zone_table[zone].builders, KNRM, zone_table[zone].bot, zone_table[zone].top);
*/
}

size_t print_zone_to_buf_mort(char *bufptr, size_t left, zone_rnum zone, int listall)
{
  size_t tmp;

  if (listall) {
    int i, j, k, l, m, n;        
    i = j = k = l = m = n = 0;

        for (i = 0; i < top_of_world; i++)
          if (world[i].number >= zone_table[zone].bot && world[i].number <= zone_table[zone].top)
            j++;          

  tmp = snprintf(bufptr, left,  "&BName      &W : &C%s&n\r\n"
                                        "&BRooms     &W : &C%d\r\n"
                                        "&BLifespan   &W: %d minutes\r\n"
                                        "&BPK Type    &W: &MN/A\r\n"
                                        "&BBuilder(s) &W: &C%s\r\n"
                                        "&BDifficulty &W: &MN/A\r\n\r\n"
                                        "&BStoryline  &W: \r\n   &G          Not available at this time.\r\n",

          zone_table[zone].name, j,  zone_table[zone].lifespan, zone_table[zone].builders);
  
  return tmp;
 }
    return snprintf(bufptr, left,
        "%3d %-*s%s By: %-10.10s%s Range: %5d-%5d\r\n", zone_table[zone].number,
        count_color_chars(zone_table[zone].name)+30, zone_table[zone].name, KNRM,
        zone_table[zone].builders, KNRM, zone_table[zone].bot, zone_table[zone].top);
}

ACMD(do_show)
{
  int i, j, k, l, con;		/* i, j, k to specifics? */
  size_t len, nlen;
  zone_rnum zrn;
  zone_vnum zvn;
  byte self = FALSE;
  struct char_data *vict = NULL, *here = NULL;
  struct obj_data *obj;
  struct descriptor_data *d;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  int ttl=0, classes=0;
  char *lvlcol;
  int tp=0;
  char GAME_GOLD=0;
  char cname[MAX_NAME_LENGTH];

  struct show_struct {
    const char *cmd;
    const char level;
  } fields[] = {
    { "nothing",	0  },				/* 0 */
    { "zones",		1  },			/* 1 */
    { "player",		1  },
    { "rent",		LVL_GOD },
    { "stats",		LVL_GOD },
    { "errors",		LVL_GOD },			/* 5 */
    { "death",		LVL_GOD },
    { "godrooms",	LVL_GOD },
    { "shops",		LVL_GOD },
    { "houses",		LVL_GOD },
    { "snoop",		LVL_GOD },			/* 10 */
    { "gamegold",       LVL_IMPL },
    { "playerbase",     LVL_IMPL },
    { "arena",          LVL_GRGOD },
    { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char(ch, "Show options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
	send_to_char(ch, "%-15s%s", fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    send_to_char(ch, "\r\n");
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));	/* strcpy: OK (argument <= MAX_INPUT_LENGTH == arg) */

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char(ch, "You are not godly enough for that!\r\n");
    return;
  }
  if (!strcmp(value, "."))
    self = TRUE;
  buf[0] = '\0';

  switch (l) {
  /* show zone */
  case 1:
    /* tightened up by JE 4/6/93 */
    if (self)
      print_zone_to_buf(buf, sizeof(buf), world[IN_ROOM(ch)].zone, 1);
    else if (*value && is_number(value)) {
      for (zvn = atoi(value), zrn = 0; zone_table[zrn].number != zvn && zrn <= top_of_zone_table; zrn++);
      if (zrn <= top_of_zone_table) {
        if (GET_LEVEL(ch) < 41)
	  print_zone_to_buf_mort(buf, sizeof(buf), zrn, 1);
        else
          print_zone_to_buf(buf, sizeof(buf), zrn, 1);
      }
      else {
	send_to_char(ch, "That is not a valid zone.\r\n");
	return;
      }
    } else {
      char *buf2;
      for (len = zrn = 0; zrn <= top_of_zone_table; zrn++) {
        if (*value) {
          buf2 = strtok(strdup(zone_table[zrn].builders), " ");
          while (buf2) {
            if (!str_cmp(buf2, value))
              break;
            buf2 = strtok(NULL, " ");
          }
          if (!buf2)
	    continue;
	}
	nlen = print_zone_to_buf(buf + len, sizeof(buf) - len, zrn, 0);
        if (len + nlen >= sizeof(buf) || nlen < 0)
          break;
        len += nlen;
      }
    }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show player */
  case 2:
    if (!*value) {
      send_to_char(ch, "A name would help.\r\n");
      return;
    }

    CREATE(vict, struct char_data, 1);
    clear_char(vict);
    CREATE(vict->player_specials, struct player_special_data, 1);
    if (load_char(value, vict) < 0) {
      send_to_char(ch, "There is no such player.\r\n");
      free_char(vict);
      return;
    }
/*
    if (GET_WAS_WARRIOR(vict) > 0)
      classes++;
    if (GET_WAS_THIEF(vict) > 0)
      classes++;
    if (GET_WAS_MAGE(vict) > 0)
      classes++;
    if (GET_WAS_CLERIC(vict) > 0)
      classes++;
*/

    classes = (GET_WAS_WARRIOR(vict) + GET_WAS_CLERIC(vict) + GET_WAS_THIEF(vict) + GET_WAS_MAGE(vict) + GET_WAS_PALADIN(vict)+
               GET_WAS_KNIGHT(vict)+ GET_WAS_RANGER(vict)+ GET_WAS_MYSTIC(vict)+ GET_WAS_PRIEST(vict)+ GET_WAS_SHAMAN(vict)+
               GET_WAS_NECROMANCER(vict)+ GET_WAS_ELEMENTALIST(vict)+ GET_WAS_SORCERER(vict)+ GET_WAS_ROGUE(vict)+
               GET_WAS_NINJA(vict)+ GET_WAS_ASSASSIN(vict));

    if(GET_LEVEL(vict) > 40)
     lvlcol = "&C";
    else if(GET_LEVEL(vict) == 40)
     lvlcol = "&R";
    else
     lvlcol = "&Y";

 switch(classes)
    {
     case 1:
       ttl = GET_LEVEL(vict);
       break;
     case 2:
       ttl = GET_LEVEL(vict) + 40;
       break;
     case 3:
       ttl = GET_LEVEL(vict) + 80;
       break;
     case 4:
       ttl = GET_LEVEL(vict) + 120;
       break;
     default:
       ttl = GET_LEVEL(vict) + 160;
    }
   

    send_to_char(ch, "\r\n"
                     "&BPlayer: &G%s                  \r\n"
//                     "&BGuild: &G%d                   \r\n"
                     "&BLevel: %s%d(%d)               \r\n"
                     "&BClass: &C%s                   \r\n"
                     "&BNeutral Player Kills: &Y%d    \r\n"
                     "&BChaotic Player Kills: &Y%d&n \r\n",
                    GET_NAME(vict), 
//                    get_blank_clan_name(GET_CLAN(vict)),
                    lvlcol, GET_LEVEL(vict),ttl,
                    pc_class_types[(int)GET_CLASS(vict)],
                    GET_PLAYER_KILLS(vict), GET_CPKS(vict));


   if (!(here = get_char_vis(ch, value, NULL, FIND_CHAR_WORLD)))
     send_to_char(ch, "&C%s isn't around right now.\r\n", GET_NAME(vict));
   else
     send_to_char(ch, "&C%s is currently online!\r\n", GET_NAME(vict));

   if (GET_LEVEL(ch) == LVL_IMPL)
     send_to_char(ch, "&WLast:&R %-20.16s&n\r\n", ctime(&vict->player.time.logon));

    free_char(vict);
    break;

  /* show rent */
  case 3:
    if (!*value) {
      send_to_char(ch, "A name would help.\r\n");
      return;
    }
    Crash_listrent(ch, value);
    break;

  /* show stats */
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    send_to_char(ch,
	"Current stats:\r\n"
	"  %5d players in game  %5d connected\r\n"
	"  %5d registered\r\n"
	"  %5d mobiles          %5d prototypes\r\n"
	"  %5d objects          %5d prototypes\r\n"
	"  %5d rooms            %5d zones\r\n"
        "  %5d triggers         %5d shops\r\n"
      	"  %5d large bufs       %5d autoquests\r\n"
	"  %5d buf switches     %5d overflows\r\n",
	i, con,
	top_of_p_table + 1,
	j, top_of_mobt + 1,
	k, top_of_objt + 1,
	top_of_world + 1, top_of_zone_table + 1,
	top_of_trigt + 1, top_shop + 1,
	buf_largecount, total_quests,
	buf_switches, buf_overflows
	);
    break;

  /* show errors */
  case 5:
    len = strlcpy(buf, "Errant Rooms\r\n------------\r\n", sizeof(buf));
    for (i = 0, k = 0; i <= top_of_world; i++)
      for (j = 0; j < NUM_OF_DIRS; j++) {
      	if (!W_EXIT(i,j))
      	  continue;
        if (W_EXIT(i,j)->to_room == 0) {
	    nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: (void   ) [%5d] %-*s%s (%s)\r\n", ++k, GET_ROOM_VNUM(i), count_color_chars(world[i].name)+40, world[i].name, QNRM, dirs[j]);
            if (len + nlen >= sizeof(buf) || nlen < 0)
              break;
            len += nlen;
        }
        if (W_EXIT(i,j)->to_room == NOWHERE && !W_EXIT(i,j)->general_description) {
	    nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: (Nowhere) [%5d] %-*s%s (%s)\r\n", ++k, GET_ROOM_VNUM(i), count_color_chars(world[i].name)+ 40, world[i].name, QNRM, dirs[j]);
            if (len + nlen >= sizeof(buf) || nlen < 0)
              break;
            len += nlen;
        }
      }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show death */
  case 6:
    len = strlcpy(buf, "Death Traps\r\n-----------\r\n", sizeof(buf));
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_DEATH)) {
        nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: [%5d] %s%s\r\n", ++j, GET_ROOM_VNUM(i), world[i].name, QNRM);
        if (len + nlen >= sizeof(buf) || nlen < 0)
          break;
        len += nlen;
      }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show godrooms */
  case 7:
    len = strlcpy(buf, "Godrooms\r\n--------------------------\r\n", sizeof(buf));
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_GODROOM)) {
        nlen = snprintf(buf + len, sizeof(buf) - len, "%2d: [%5d] %s%s\r\n", ++j, GET_ROOM_VNUM(i), world[i].name, QNRM);
        if (len + nlen >= sizeof(buf) || nlen < 0)
          break;
        len += nlen;
      }
    page_string(ch->desc, buf, TRUE);
    break;

  /* show shops */
  case 8:
    show_shops(ch, value);
    break;

  /* show houses */
  case 9:
    hcontrol_list_houses(ch, value);
    break;

  /* show snoop */
  case 10:
    i = 0;
    send_to_char(ch, "People currently snooping:\r\n--------------------------\r\n");
    for (d = descriptor_list; d; d = d->next) {
      if (d->snooping == NULL || d->character == NULL)
	continue;
      if (STATE(d) != CON_PLAYING || GET_LEVEL(ch) < GET_LEVEL(d->character))
	continue;
      if (!CAN_SEE(ch, d->character) || IN_ROOM(d->character) == NOWHERE)
	continue;
      i++;
      send_to_char(ch, "%-10s%s - snooped by %s%s.\r\n", GET_NAME(d->snooping->character), QNRM, GET_NAME(d->character), QNRM);
    }
    if (i == 0)
      send_to_char(ch, "No one is currently snooping.\r\n");
    break;
  /* game gold */
  case 11:
   for (tp = 0; tp <= top_of_p_table; tp++)
   {
    snprintf(cname, sizeof(cname), "%s", player_table[tp].name);
    CREATE(vict, struct char_data, 1);
    clear_char(vict);
    CREATE(vict->player_specials, struct player_special_data, 1);
     if (load_char(value, vict) < 0) {
       GAME_GOLD += (GET_GOLD(vict) + GET_BANK_GOLD(vict));
       free_char(vict);
     }
   }
   send_to_char(ch, "\r\n&RThere is &Y%s &Rgold coins in the game.&n\r\n", Commatize(GAME_GOLD));
   break;

  case 12:
   len = strlcpy(buf, "&R\r\nPlayer              Level&n\r\n----------------------------------\r\n", sizeof(buf));
   for (tp=0;tp <= top_of_p_table; tp++)
   {
    nlen = snprintf(buf+len, sizeof(buf)-len, "&W%-20s          &Y%-3d&n\r\n", player_table[tp].name, 
player_table[tp].level);
    if (len + nlen >= sizeof(buf) || nlen < 0)
      break ;
    len += nlen;
   }
   page_string(ch->desc, buf, TRUE);
   break;
  /* show arena */
  case 13:
    if (in_arena == ARENA_OFF) {
        sprintf(buf, "The Arena is closed right now.\r\n");
    } else if (in_arena == ARENA_START) {
        sprintf(buf, "Arena will start in %d hour(s)\r\n", time_to_start);
        sprintf(buf, "%sIt will last for %d hour(s)\r\n", buf, game_length);
    } else if (in_arena == ARENA_RUNNING) {
        sprintf(buf, "Arena will end in %d hour(s)\r\n", time_left_in_game);
    }
    send_to_char(ch, buf);
    break;


  /* show what? */
  default:
    send_to_char(ch, "Sorry, I don't understand that.\r\n");
    break;
  }
}

/* The \do_set function */

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
        if (on) SET_BIT_AR(flagset, flags); \
        else if (off) REMOVE_BIT_AR(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))




/* The set options available */
  struct set_struct {
    const char *cmd;
    const char level;
    const char pcnpc;
    const char type;
  } set_fields[] = {
   { "ac",		LVL_BUILDER, 	BOTH, 	NUMBER },  /* 0  */
   { "afk",             LVL_BUILDER,	PC,	BINARY },  /* 1  */
   { "age",		LVL_GOD,	BOTH,	NUMBER },
   { "align",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "bank",		LVL_BUILDER, 	PC, 	NUMBER },
   { "brief",		LVL_GOD, 	PC, 	BINARY },  /* 5  */
   { "cha",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "class",		LVL_BUILDER, 	BOTH, 	MISC },
   { "color",		LVL_GOD, 	PC, 	BINARY },
   { "con", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "damroll",		LVL_BUILDER, 	BOTH, 	NUMBER },  /* 10 */
   { "deleted",		LVL_IMPL, 	PC, 	BINARY },
   { "dex", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "drunk",		LVL_BUILDER, 	BOTH, 	MISC   },
   { "email",           LVL_IMPL,       PC,     MISC   },
   { "exp", 		LVL_GOD, 	BOTH, 	NUMBER },  /* 15 */
   { "frozen",		LVL_GRGOD, 	PC,	BINARY },  
   { "gold",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "height",		LVL_BUILDER,	BOTH,	NUMBER },
   { "hitpoints",       LVL_BUILDER, 	BOTH, 	NUMBER },
   { "hitroll",		LVL_BUILDER, 	BOTH, 	NUMBER },  /* 20 */
   { "hunger",		LVL_BUILDER, 	BOTH, 	MISC   },    
   { "int", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "invis",		LVL_GOD, 	PC, 	NUMBER },
   { "invstart",        LVL_BUILDER,	PC, 	BINARY },
   { "killer",		LVL_GOD, 	PC, 	BINARY },  /* 25 */
   { "level",		LVL_GRGOD, 	BOTH, 	NUMBER }, 
   { "loadroom",	LVL_BUILDER, 	PC, 	MISC   },
   { "mana",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "maxhit",	        LVL_BUILDER, 	BOTH, 	NUMBER },
   { "maxmana",       	LVL_BUILDER, 	BOTH, 	NUMBER },  /* 30 */
   { "maxmove",		LVL_BUILDER, 	BOTH, 	NUMBER },  
   { "move",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "nodelete",	LVL_GOD, 	PC, 	BINARY },
   { "nohassle",	LVL_GOD, 	PC, 	BINARY },
   { "nosummon",	LVL_BUILDER,	PC,	BINARY },  /* 35 */
   { "nowizlist", 	LVL_GRGOD, 	PC, 	BINARY }, 
   { "olc",		LVL_GRGOD,	PC,	MISC   },
   { "password",	LVL_IMPL, 	PC,	MISC   },
   { "poofin",		LVL_GOD,	PC,	MISC   },
   { "poofout",         LVL_GOD,	PC,	MISC   },  /* 40 */
   { "practices", 	LVL_GOD, 	PC, 	NUMBER },   
   { "quest",		LVL_GOD, 	PC, 	BINARY },
   { "room",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "sex", 		LVL_GOD, 	BOTH, 	MISC   },
   { "showvnums", 	LVL_BUILDER, 	PC,	BINARY },  /* 45 */
   { "siteok",		LVL_GOD, 	PC, 	BINARY }, 
   { "str",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "stradd",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "thief",		LVL_GOD, 	PC, 	BINARY },
   { "thirst",		LVL_BUILDER, 	BOTH, 	MISC   },  /* 50 */
   { "title",		LVL_GOD, 	PC, 	MISC   }, 
   { "variable",        LVL_GRGOD,	PC,	MISC   },
   { "weight",		LVL_BUILDER,	BOTH,	NUMBER },
   { "wis", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "questpoints",     LVL_GOD,        PC,     NUMBER },  /* 55 */
   { "clan",            LVL_GOD,        PC,     NUMBER }, 
   { "souls",           LVL_IMPL,       BOTH,   NUMBER },
   { "rank",            LVL_GOD,        PC,     NUMBER },
   { "clan_gold",       LVL_IMPL,       PC,     BINARY },
   { "hometown",        LVL_GRGOD,      PC,     NUMBER },  /* 60 */
   { "screenwidth",     LVL_GOD,        PC,     NUMBER }, 
   { "ore",             LVL_IMPL,       PC,     NUMBER },
   { "gems",            LVL_IMPL,       PC,     NUMBER },
   { "security",        LVL_IMPL,       PC,     NUMBER },
   { "preceptor",       LVL_IMPL,       PC,     BINARY },  /* 65 */
   { "shares",          LVL_IMPL,       PC,     NUMBER }, 
   { "pagelength",      LVL_IMPL,       PC,     NUMBER },
   { "questhistory",    LVL_GOD,        PC,     NUMBER },
   { "ethos",           LVL_GRGOD,      PC,     NUMBER },
   { "deity",           LVL_GRGOD,      PC,     NUMBER },  /* 70 */
   { "saves",           LVL_IMPL,       BOTH,   NUMBER },
   { "recall",          LVL_IMPL,       PC,     NUMBER },
   { "\n", 0, BOTH, MISC }
  };

int perform_set(struct char_data *ch, struct char_data *vict, int mode, char *val_arg)
{
  int i, on = 0, off = 0, value = 0, qvnum;
  room_rnum rnum;
  room_vnum rvnum;
  int a, b;
  a = b = 0;

  /* Check to make sure all the levels are correct */
  if (GET_LEVEL(ch) != LVL_IMPL) {
    if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
      send_to_char(ch, "Maybe that's not such a great idea...\r\n");
      return (0);
    }
  }
  if (GET_LEVEL(ch) < set_fields[mode].level) {
    send_to_char(ch, "You are not godly enough for that!\r\n");
    return (0);
  }

  /* Make sure the PC/NPC is correct */
  if (IS_NPC(vict) && !(set_fields[mode].pcnpc & NPC)) {
    send_to_char(ch, "You can't do that to a beast!\r\n");
    return (0);
  } else if (!IS_NPC(vict) && !(set_fields[mode].pcnpc & PC)) {
    send_to_char(ch, "That can only be done to a beast!\r\n");
    return (0);
  }

  /* Find the value of the argument */
  if (set_fields[mode].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char(ch, "Value must be 'on' or 'off'.\r\n");
      return (0);
    }
    send_to_char(ch, "%s %s for %s.\r\n", set_fields[mode].cmd, ONOFF(on), GET_NAME(vict));
  } else if (set_fields[mode].type == NUMBER) {
    value = atoi(val_arg);
    send_to_char(ch, "%s's %s set to %d.\r\n", GET_NAME(vict), set_fields[mode].cmd, value);
  } else
    send_to_char(ch, "%s", CONFIG_OK);

  switch (mode) {
    case 0: /* ac */
      vict->points.armor = RANGE(-100, 100);
      affect_total(vict);
      break;
    case 1: /* afk */
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_AFK);
      break;
    case 2: /* age */
      if (value < 2 || value > 200) {	/* Arbitrary limits. */
        send_to_char(ch, "Ages 2 to 200 accepted.\r\n");
        return (0);
      }
      /* NOTE: May not display the exact age specified due to the integer
       * division used elsewhere in the code.  Seems to only happen for
       * some values below the starting age (17) anyway. -gg 5/27/98 */
      vict->player.time.birth = time(0) - ((value - 17) * SECS_PER_MUD_YEAR);
      break;
    case 3: /* align */
      GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
      affect_total(vict);
      break;
    case 4: /* bank */
      GET_BANK_GOLD(vict) = RANGE(0, 100000000);
      break;
    case 5: /* brief */
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
      break;
    case 6:  /* cha */
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
        RANGE(3, 25);
      else
        RANGE(3, 20);
      vict->real_abils.cha = value;
      affect_total(vict);
      break;
    case 7: /* class */
      if ((i = parse_class(*val_arg)) == CLASS_UNDEFINED) {
        send_to_char(ch, "That is not a class.\r\n");
        return (0);
      }
      GET_CLASS(vict) = i;
      break;
    case 8:  /* color */
      SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
      break;
    case 9: /* con */
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
        RANGE(3, 25);
      else
        RANGE(3, 20);
      vict->real_abils.con = value;
      affect_total(vict);
      break;
    case 10:  /* damroll */
      vict->points.damroll = RANGE(-20, 1000);
      affect_total(vict);
      break;
    case 11: /* delete */
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
      break;
    case 12: /* dex */
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
        RANGE(3, 25);
      else
        RANGE(3, 20);
      vict->real_abils.dex = value;
      affect_total(vict);
      break;
    case 13: /* drunk */
      if (!str_cmp(val_arg, "off")) {
        GET_COND(vict, DRUNK) = -1;
        send_to_char(ch, "%s's drunkenness is now off.\r\n", GET_NAME(vict));
      } else if (is_number(val_arg)) {
        value = atoi(val_arg);
        RANGE(0, 24);
        GET_COND(vict, DRUNK) = value;
        send_to_char(ch, "%s's drunkenness set to %d.\r\n", GET_NAME(vict), value);
      } else {
        send_to_char(ch, "Must be 'off' or a value from 0 to 24.\r\n");
        return (0);
      }
      break;
    case 14: /* email */
       set_email(vict, val_arg);
       send_to_char(ch, "%s's email set to %s\r\n", GET_NAME(vict), GET_EMAIL(vict) ? GET_EMAIL(vict) : "Nothing");
       break;
    case 15: /* exp */
      vict->points.exp = RANGE(0, 50000000);
      break;
    case 16: /* frozen */
      if (ch == vict && on) {
        send_to_char(ch, "Better not -- could be a long winter!\r\n");
        return (0);
      }
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
      break;
    case 17: /* gold */
      GET_GOLD(vict) = RANGE(0, 100000000);
      break;
    case 18: /* height */
      GET_HEIGHT(vict) = value;
      affect_total(vict);
      break;
    case 19: /* hit */
      vict->points.hit = RANGE(-9, vict->points.max_hit);
      affect_total(vict);
      break;
    case 20: /* hitroll */
      vict->points.hitroll = RANGE(-20, 1000);
      affect_total(vict);
      break;
    case 21: /* hunger */
      if (!str_cmp(val_arg, "off")) {
        GET_COND(vict, HUNGER) = -1;
        send_to_char(ch, "%s's hunger is now off.\r\n", GET_NAME(vict));
      } else if (is_number(val_arg)) {
        value = atoi(val_arg);
        RANGE(0, 24);
        GET_COND(vict, HUNGER) = value;
        send_to_char(ch, "%s's hunger set to %d.\r\n", GET_NAME(vict), value);
      } else {
        send_to_char(ch, "Must be 'off' or a value from 0 to 24.\r\n");
        return (0);
       }
       break;
   case 22: /* int */
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
        RANGE(3, 25);
      else
        RANGE(3, 20);
      vict->real_abils.intel = value;
      affect_total(vict);
      break;
    case 23: /* invis */
      if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
        send_to_char(ch, "You aren't godly enough for that!\r\n");
        return (0);
      }
      GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
      break;
    case 24: /* invistart */
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
      break;
    case 25: /* killer */
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
      break;
    case 26: /* level */
      if ((!IS_NPC(vict) && value > GET_LEVEL(ch)) || value > LVL_IMPL) {
        send_to_char(ch, "You can't do that.\r\n");
        return (0);
      }
      RANGE(1, LVL_IMPL);
      vict->player.level = value;
      break;
    case 27: /* loadroom */
      if (!str_cmp(val_arg, "off")) {
        REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
      } else if (is_number(val_arg)) {
        rvnum = atoi(val_arg);
        if (real_room(rvnum) != NOWHERE) {
          SET_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
          GET_LOADROOM(vict) = rvnum;
          send_to_char(ch, "%s will enter at room #%d.\r\n", GET_NAME(vict), GET_LOADROOM(vict));
        } else {
        send_to_char(ch, "That room does not exist!\r\n");
	  return (0);
        }
      } else {
        send_to_char(ch, "Must be 'off' or a room's virtual number.\r\n");
        return (0);
      }
      break;
    case 28: /* mana */
      vict->points.mana = RANGE(0, vict->points.max_mana);
      affect_total(vict);
      break;
    case 29: /* maxhit */
      vict->points.max_hit = RANGE(1, 5000);
      affect_total(vict);
      break;
    case 30: /* maxmana */
      vict->points.max_mana = RANGE(1, 5000);
      affect_total(vict);
      break;
    case 31: /* maxmove */
      vict->points.max_move = RANGE(1, 5000);
      affect_total(vict);
      break;
    case 32: /* move */
      vict->points.move = RANGE(0, vict->points.max_move);
      affect_total(vict);
      break;
    case 33: /* nodelete */
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
      break;
    case 34: /* nohassle */
      if (GET_LEVEL(ch) < LVL_GOD && ch != vict) {
        send_to_char(ch, "You aren't godly enough for that!\r\n");
        return (0);
      }
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
      break;
    case 35: /* nosummon */
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
      send_to_char(ch, "Nosummon %s for %s.\r\n", ONOFF(!on), GET_NAME(vict));
      break;
    case 36: /* nowiz */
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
      break;
    case 37: /* olc */
      if (is_abbrev(val_arg, "socials") || is_abbrev(val_arg, "actions") || is_abbrev(val_arg, "aedit"))
        GET_OLC_ZONE(vict) = AEDIT_PERMISSION;
      else if (is_abbrev(val_arg, "hedit") || is_abbrev(val_arg, "help")) 
        GET_OLC_ZONE(vict) = HEDIT_PERMISSION;
      else if (*val_arg == '*' || is_abbrev(val_arg, "all")) 
        GET_OLC_ZONE(vict) = ALL_PERMISSION;
      else if (is_abbrev(val_arg, "off"))
        GET_OLC_ZONE(vict) = NOWHERE;
      else if (!is_number(val_arg))  {
        send_to_char(ch, "Value must be a zone number, 'aedit', 'hedit', 'off' or 'all'.\r\n");
        return (0);
      } else
        GET_OLC_ZONE(vict) = atoi(val_arg);
      break;
    case 38: /* password */
      if (GET_LEVEL(vict) >= LVL_GRGOD) {
        send_to_char(ch, "You cannot change that.\r\n");
        return (0);
      }
      strncpy(GET_PASSWD(vict), CRYPT(val_arg, GET_NAME(vict)), MAX_PWD_LENGTH);	/* strncpy: OK (G_P:MAX_PWD_LENGTH) */
      *(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';
      send_to_char(ch, "Password changed to '%s'.\r\n", val_arg);
      break;
    case 39: /* poofin */
      if ((vict == ch) || (GET_LEVEL(ch) == LVL_IMPL)) {
        skip_spaces(&val_arg);

        if (POOFIN(vict)) 
          free(POOFIN(vict)); 

      if (!*val_arg)
          POOFIN(vict) = NULL;
        else
          POOFIN(vict) = strdup(val_arg);
        }
      break;
    case 40: /* poofout */
      if ((vict == ch) || (GET_LEVEL(ch) == LVL_IMPL)) {
        skip_spaces(&val_arg);

        if (POOFOUT(vict)) 
          free(POOFOUT(vict)); 
	
	if (!*val_arg)
          POOFOUT(vict) = NULL;
        else
          POOFOUT(vict) = strdup(val_arg);
        }
      break;
    case 41: /* practices */
      GET_PRACTICES(vict) = RANGE(0, 100);
      break;
    case 42: /* quest */
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
      break;
    case 43: /* room */
      if ((rnum = real_room(value)) == NOWHERE) {
        send_to_char(ch, "No room exists with that number.\r\n");
        return (0);
      }
      if (IN_ROOM(vict) != NOWHERE)
        char_from_room(vict);
      char_to_room(vict, rnum);
      break;
    case 44: /* sex */
      if ((i = search_block(val_arg, genders, FALSE)) < 0) {
        send_to_char(ch, "Must be 'male', 'female', or 'neutral'.\r\n");
        return (0);
      }
      GET_SEX(vict) = i;
      break;
    case 45: /* showvnums */
      SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
      break;
    case 46: /* siteok */
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
      break;
    case 47: /* str */
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
        RANGE(3, 25);
      else
        RANGE(3, 20);
      vict->real_abils.str = value;
      vict->real_abils.str_add = 0;
      affect_total(vict);
      break;
    case 48: /* stradd */
      vict->real_abils.str_add = RANGE(0, 100);
      if (value > 0)
        vict->real_abils.str = 20;
      affect_total(vict);
      break;
    case 49: /* thief */
      SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
      break;
    case 50: /* thirst */
      if (!str_cmp(val_arg, "off")) {
        GET_COND(vict, THIRST) = -1;
        send_to_char(ch, "%s's thirst is now off.\r\n", GET_NAME(vict));
      } else if (is_number(val_arg)) {
        value = atoi(val_arg);
        RANGE(0, 24);
        GET_COND(vict, THIRST) = value;
        send_to_char(ch, "%s's thirst set to %d.\r\n", GET_NAME(vict), value);
      } else {
        send_to_char(ch, "Must be 'off' or a value from 0 to 24.\r\n");
        return (0);
      }
      break;
  case 51: /* title */
      set_title(vict, val_arg);
      send_to_char(ch, "%s's title is now: %s\r\n", GET_NAME(vict), GET_TITLE(vict));
      break;
    case 52: /* variable */
      return perform_set_dg_var(ch, vict, val_arg);
      break;
    case 53: /* weight */
      GET_WEIGHT(vict) = value;
      affect_total(vict);
      break;
    case 54: /* wis */
      if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
        RANGE(3, 25);
      else
        RANGE(3, 20);
      vict->real_abils.wis = value;
      affect_total(vict);
      break;
    case 55: /* questpoints */
      GET_QUESTPOINTS(vict) = RANGE(0, 100000000);
      break;
    case 56:
     RANGE(0, cnum);
     if (value == 0)
       GET_CLAN_RANK(vict) = CLAN_NONE;
     GET_CLAN(vict) = value;
     break;
    case 57:
      GET_SOULS(vict) = atoi(val_arg);
      break;
  case 58:
    RANGE(0, CLAN_LEADER);
    if (GET_CLAN(vict) == CLAN_NONE) {
      send_to_char(ch, "Target must be in a clan first.\r\n");
      return (0);
    }
    GET_CLAN_RANK(vict) = value;
    break;
  case 59:
    send_to_char(ch, "Set clan_gold disabled for now.\r\n");
    return (0);
    break;
  case 60:
    RANGE(0, NUM_STARTROOMS);
    GET_HOME(vict) = value;
    break;
  case 61:
    GET_SCREEN_WIDTH(vict) = RANGE(40, 200);
    break;
  case 62:
    GET_ADAMANTITE_ORE(vict) = value;
    GET_BRONZE_ORE(vict) = value;
    GET_COPPER_ORE(vict) = value;
    GET_GOLD_ORE(vict) = value;
    GET_IRON_ORE(vict) = value;
    GET_MITHRIL_ORE(vict) = value;
    GET_SILVER_ORE(vict) = value;
    GET_TITANIUM_ORE(vict) = value;
    break;
  case 63:
    GET_CRYSTAL(vict) = value;
    GET_DIAMOND(vict) = value;
    GET_EMERALD(vict) = value;
    GET_GARNET(vict) = value;
    GET_JADE(vict) = value;
    GET_RUBY(vict) = value;
    GET_SAPPHIRE(vict) = value;
    GET_TOPAZ(vict) = value;
    break;
  case 64:
    if (!IS_CODER(ch))
      send_to_char(ch, "You do not have permission to use this function.\r\n");
    else {
      if (value < 0 || value > 3) {
        send_to_char(ch, "Values are: 0-None, 1-Low, 2-Medium, 3-High\r\n");
        send_to_char(ch, "Setting to default security settings.\r\n");
        SET_SECURITY(vict, 0);
        break;}  
      else {    
    SET_SECURITY(vict, value);
    break;}}
  case 65: /* killer */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_PRECEPTOR);        
    break;
  case 66:
     for(i=0;i<MAX_STOCK;i++)
       SET_SHARES(vict,i,RANGE(0, 1000000));
     break;
  case 67:
     if (value < 22)
       send_to_char(ch, "You cannot set the pagelength that low.\r\n");
     else
       GET_PAGE_LENGTH(ch) = value;
     break;
  case 68: /* questhistory */
      qvnum = atoi(val_arg);
      if (real_quest(qvnum) == NOTHING) {
        send_to_char(ch, "That quest doesn't exist.\r\n");
        return FALSE;
      } else {
        if (is_complete(vict, qvnum)) {
          remove_completed_quest(vict, qvnum);
          send_to_char(ch, "Quest %d removed from history for player %s.\r\n",
			qvnum, GET_NAME(vict));
        } else {
          add_completed_quest(vict, qvnum);
          send_to_char(ch, "Quest %d added to history for player %s.\r\n",
			qvnum, GET_NAME(vict));
        }
        break;
      }
  case 69: /* ethos */
     if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0,2);
      GET_ETHOS(vict) = value;
      send_to_char(ch, "%s's %s set to %d.", GET_NAME(vict), set_fields[mode].cmd, value);
     } else
      send_to_char(ch, "Must be a number from 0 to 2 (0=Good, 1=Evil, 2=Neutral)\r\n");
     break;
  case 70: /* deity */
     if (is_number(val_arg)) {
       value = atoi(val_arg);
       RANGE(0, (MAX_DEITY-1));
       GET_DEITY(vict) = value;
       send_to_char(ch, "%s's %s set to %d.", GET_NAME(vict), set_fields[mode].cmd, value);
     } else
       send_to_char(ch, "Must be a number from 0 to %d", (MAX_DEITY-1));
     break;
  case 71: /*saves*/
      send_to_char(ch, "Due to resetting all of your saves.  We must first remove all of your equipment to avoid stat loss.\r\n");
      for (i = 0; i < NUM_WEARS; i++)
       if (GET_EQ(ch, i)) {                     
        perform_remove(ch, i);
      }  
        for(i=0;i<MAX_SAVING;i++)
          GET_SAVE(vict, i) = 0;
     break;
  case 72: /*recall*/
     send_to_char(ch, "Resetting %s's recall to defalt (6300)\r\n"
                      "Resetting %s's Altar to default (6305)\r\n", GET_NAME(vict),GET_NAME(vict));
     GET_RECALL(vict) = 6300;
     GET_ALTAR(vict) = 6305;
     break;
  default:
    send_to_char(ch, "Can't set that!\r\n");
    return (0);
   }
  return (1);
}

ACMD(do_set)
{
  struct char_data *vict = NULL, *cbuf = NULL;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  int mode, len, player_i = 0, retval;
  char is_file = 0, is_player = 0;
  int i, j;
  i = j = 0;

  half_chop(argument, name, buf);

  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob"))
    half_chop(buf, name, buf);

  half_chop(buf, field, buf);

  if (!*name || !*field) {
    send_to_char(ch, "Usage: set <victim> <field> <value>\r\n\r\nAvailable fields are:\r\n");
     for (j = 0, i = 0; set_fields[i].level;i++)
      if (set_fields[i].level <= GET_LEVEL(ch))
        send_to_char(ch, "&c%-15s%s&n", set_fields[i].cmd, (!(++j % 4) ? "\r\n" : " "));
      send_to_char(ch, "\r\n");
    return;
  }

  /* find the target */
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name, NULL, FIND_CHAR_WORLD))) {
	send_to_char(ch, "There is no such player.\r\n");
	return;
      }
    } else { /* is_mob */
      if (!(vict = get_char_vis(ch, name, NULL, FIND_CHAR_WORLD))) {
	send_to_char(ch, "There is no such creature.\r\n");
	return;
      }
    }
  } else if (is_file) {
    /* try to load the player off disk */
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    CREATE(cbuf->player_specials, struct player_special_data, 1);
    if ((player_i = load_char(name, cbuf)) > -1) {
      if (GET_LEVEL(cbuf) > GET_LEVEL(ch)) {
	free_char(cbuf);
	send_to_char(ch, "Sorry, you can't do that.\r\n");
	return;
      }
      vict = cbuf;
    } else {
      free_char(cbuf);
      send_to_char(ch, "There is no such player.\r\n");
      return;
    }
  }

  /* find the command in the list */
  len = strlen(field);
  for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
    if (!strncmp(field, set_fields[mode].cmd, len))
      break;

  if (*(set_fields[mode].cmd) == '\n') {
    retval = 0; /* skips saving below */
    send_to_char(ch, "Can't set that!\r\n");
  } else
  /* perform the set */
  retval = perform_set(ch, vict, mode, buf);

  /* save the character if a change was made */
  if (retval) {
    if (!is_file && !IS_NPC(vict))
      save_char(vict);
    if (is_file) {
      GET_PFILEPOS(cbuf) = player_i;
      save_char(cbuf);
      send_to_char(ch, "Saved in file.\r\n");
    }
  }

  /* free the memory if we allocated it earlier */
  if (is_file)
    free_char(cbuf);
}

ACMD(do_saveall)
{
 if (GET_LEVEL(ch) < LVL_BUILDER)
    send_to_char (ch, "You are not holy enough to use this privelege.\n\r");
 else {
    save_all();
    House_save_all();
    send_to_char(ch, "World and house files saved.\n\r");
 }
}

ACMD(do_links)
{
  zone_rnum zrnum;
  zone_vnum zvnum;
  room_rnum nr, to_room;
  int first, last, j;
  char arg[MAX_INPUT_LENGTH];


  skip_spaces(&argument);
  one_argument(argument, arg);

  if (!is_number(arg)) { 
    zrnum = world[IN_ROOM(ch)].zone;
    zvnum = zone_table[zrnum].number;
  } else {
    zvnum = atoi(arg);
    zrnum = real_zone(zvnum);
  }

  if (zrnum == NOWHERE || zvnum == NOWHERE) {
    send_to_char(ch, "No zone was found with that number.\n\r");
    return;
  }

  last  = zone_table[zrnum].top;
  first = zone_table[zrnum].bot;

  send_to_char(ch, "Zone %d is linked to the following zones:\r\n", zvnum);
  for (nr = 0; nr <= top_of_world && (GET_ROOM_VNUM(nr) <= last); nr++) {
    if (GET_ROOM_VNUM(nr) >= first) {
      for (j = 0; j < NUM_OF_DIRS; j++) {
        if (world[nr].dir_option[j]) {
          to_room = world[nr].dir_option[j]->to_room;
          if (to_room != NOWHERE && (zrnum != world[to_room].zone))
          send_to_char(ch, "%3d %-30s at %5d (%-5s) ---> %5d\r\n",
                       zone_table[world[to_room].zone].number,
                       zone_table[world[to_room].zone].name,
                       GET_ROOM_VNUM(nr), dirs[j], world[to_room].number);
        }
      }
    }
  }
}

/* Zone Checker Code below */
/*mob limits*/
#define MAX_DAMROLL_ALLOWED      MAX(GET_LEVEL(mob)/5, 1)
#define MAX_HITROLL_ALLOWED      MAX(GET_LEVEL(mob)/3, 1)
#define MAX_GOLD_ALLOWED         GET_LEVEL(mob)*3000
#define MAX_EXP_ALLOWED          GET_LEVEL(mob)*GET_LEVEL(mob) * 120
#define MAX_LEVEL_ALLOWED        LVL_IMPL
#define GET_OBJ_AVG_DAM(obj)     (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1))
/* arbitrary limit for per round dam */
#define MAX_MOB_DAM_ALLOWED      500

#define ZCMD2 zone_table[zone].cmd[cmd_no]  /*fom DB.C*/

/*item limits*/
#define MAX_DAM_ALLOWED            50    /* for weapons  - avg. dam*/
#define MAX_AFFECTS_ALLOWED        3

/* Armor class limits*/
#define TOTAL_WEAR_CHECKS  (NUM_ITEM_WEARS-2)  /*minus Wield and Take*/
struct zcheck_armor {
  bitvector_t bitvector;          /* from Structs.h                       */
  int ac_allowed;                 /* Max. AC allowed for this body part  */
  char *message;                  /* phrase for error message            */
} zarmor[] = {
  {ITEM_WEAR_FINGER, 10, "Ring"},
  {ITEM_WEAR_NECK,   10, "Necklace"},
  {ITEM_WEAR_BODY,   10, "Body armor"},
  {ITEM_WEAR_HEAD,   10, "Head gear"},
  {ITEM_WEAR_LEGS,   10, "Legwear"},
  {ITEM_WEAR_FEET,   10, "Footwear"},
  {ITEM_WEAR_HANDS,  10, "Glove"},
  {ITEM_WEAR_ARMS,   10, "Armwear"},
  {ITEM_WEAR_SHIELD, 10, "Shield"},
  {ITEM_WEAR_ABOUT,  10, "Cloak"},
  {ITEM_WEAR_WAIST,  10, "Belt"},
  {ITEM_WEAR_WRIST,  10, "Wristwear"},
  {ITEM_WEAR_HOLD,   10, "Held item"}
};

/*These are strictly boolean*/
#define CAN_WEAR_WEAPONS         0     /* toggle - can a weapon also be a piece of armor? */
#define MAX_APPLIES_LIMIT        1     /* toggle - is there a limit at all?               */
#define CHECK_ITEM_RENT          0     /* do we check for rent cost == 0 ?                */
#define CHECK_ITEM_COST          0     /* do we check for item cost == 0 ?                */
/* Applies limits !! Very Important:  Keep these in the same order as in Structs.h.
 * To ignore an apply, set max_aff to -99. These will be ignored if MAX_APPLIES_LIMIT = 0 */
struct zcheck_affs {
  int aff_type;    /*from Structs.h*/
  int min_aff;     /*min. allowed value*/
  int max_aff;     /*max. allowed value*/
  char *message;   /*phrase for error message*/
} zaffs[] = {
  {APPLY_NONE,         0, -99, "unused0"},
  {APPLY_STR,         -5,   3, "strength"},
  {APPLY_DEX,         -5,   3, "dexterity"},
  {APPLY_INT,         -5,   3, "intelligence"},
  {APPLY_WIS,         -5,   3, "wisdom"},
  {APPLY_CON,         -5,   3, "constitution"},
  {APPLY_CHA,         -5,   3, "charisma"},
  {APPLY_CLASS,        0,   0, "class"},
  {APPLY_LEVEL,        0,   0, "level"},
  {APPLY_AGE,        -10,  10, "age"},
  {APPLY_CHAR_WEIGHT,-50,  50, "character weight"},
  {APPLY_CHAR_HEIGHT,-50,  50, "character height"},
  {APPLY_MANA,       -50,  50, "mana"},
  {APPLY_HIT,        -50,  50, "hit points"},
  {APPLY_MOVE,       -50,  50, "movement"},
  {APPLY_GOLD,         0,   0, "gold"},
  {APPLY_EXP,          0,   0, "experience"},
  {APPLY_AC,         -10,  10, "magical AC"},
  {APPLY_HITROLL,      0, -99, "hitroll"},       /* Handled seperately below */
  {APPLY_DAMROLL,      0, -99, "damroll"},       /* Handled seperately below */
  {APPLY_SAVING_PARA, -2,   2, "saving throw (paralysis)"},
  {APPLY_SAVING_ROD,  -2,   2, "saving throw (rod)"},
  {APPLY_SAVING_PETRI,-2,   2, "saving throw (death)"},
  {APPLY_SAVING_BREATH,-2,  2, "saving throw (breath)"},
  {APPLY_SAVING_SPELL,-2,   2, "saving throw (spell)"}
};

/* These are ABS() values. */
#define MAX_APPLY_HITROLL_TOTAL   5
#define MAX_APPLY_DAMROLL_TOTAL   5

/*room limits*/
/* Off limit zones are any zones a player should NOT be able to walk to (ex. Limbo) */
const int offlimit_zones[] = {0,12,13,14,-1};  /*what zones can no room connect to (virtual num) */
#define MIN_ROOM_DESC_LENGTH   80       /* at least one line - set to 0 to not care. */
#define MAX_COLOUMN_WIDTH      80       /* at most 80 chars per line */

ACMD (do_zcheck)
{
  zone_rnum zrnum;
  struct obj_data *obj;
  struct char_data *mob = NULL;
  room_vnum exroom=0;
  int ac=0;
  int affs=0, tohit, todam, value;
  int i = 0, j = 0, k = 0, l = 0, m = 0, found = 0; /* found is used as a 'send now' flag*/
  float avg_dam;
  size_t len = 0;
  struct extra_descr_data *ext, *ext2;
  one_argument(argument, buf);

  if (!is_number(buf) || !strcmp(buf, "."))
    zrnum = world[IN_ROOM(ch)].zone;
  else
    zrnum = real_zone(atoi(buf));

  if (zrnum == NOWHERE) {
    send_to_char(ch, "Check what zone ?\r\n");
    return;
  } else
    send_to_char(ch, "Checking zone %d!\r\n", zone_table[zrnum].number);

 /* Check mobs */

  send_to_char(ch, "Checking Mobs for limits...\r\n");
  /*check mobs first*/
  for (i=0; i<top_of_mobt;i++) {
      if (real_zone_by_thing(mob_index[i].vnum) == zrnum) {  /*is mob in this zone?*/
        mob = &mob_proto[i];
        if (!strcmp(mob->player.name, "mob unfinished") && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Alias hasn't been set.\r\n");

        if (!strcmp(mob->player.short_descr, "the unfinished mob") && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Short description hasn't been set.\r\n");

        if (!strncmp(mob->player.long_descr, "An unfinished mob stands here.", 30) && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Long description hasn't been set.\r\n");

        if (mob->player.description && *mob->player.description) {
          if (!strncmp(mob->player.description, "It looks unfinished.", 20) && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Description hasn't been set.\r\n");
          else if (strncmp(mob->player.description, "   ", 3) && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Description hasn't been formatted. (/fi)\r\n");
        }

        if (GET_LEVEL(mob)>MAX_LEVEL_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Is level %d (limit: 1-%d)\r\n",
                          GET_LEVEL(mob), MAX_LEVEL_ALLOWED);

        if (GET_DAMROLL(mob)>MAX_DAMROLL_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Damroll of %ld is too high (limit: %d)\r\n",
                          GET_DAMROLL(mob), MAX_DAMROLL_ALLOWED);

        if (GET_HITROLL(mob)>MAX_HITROLL_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Hitroll of %ld is too high (limit: %d)\r\n",
                          GET_HITROLL(mob), MAX_HITROLL_ALLOWED);

        /* avg. dam including damroll per round of combat */
        avg_dam = (((mob->mob_specials.damsizedice / 2.0) * mob->mob_specials.damnodice)+GET_DAMROLL(mob));
        if (avg_dam>MAX_MOB_DAM_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- average damage of %4.1f is too high (limit: %d)\r\n",
                          avg_dam, MAX_MOB_DAM_ALLOWED);

        if (mob->mob_specials.damsizedice == 1 &&
            mob->mob_specials.damnodice == 1 &&
            GET_LEVEL(mob) == 0 &&
            (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Needs to be fixed - %sAutogenerate!%s\r\n", CCYEL(ch, C_NRM), CCNRM(ch, C_NRM));

        if (MOB_FLAGGED(mob, MOB_AGGRESSIVE) && (MOB_FLAGGED(mob, MOB_AGGR_GOOD) || MOB_FLAGGED(mob, MOB_AGGR_EVIL) || MOB_FLAGGED(mob, MOB_AGGR_NEUTRAL)) && (found=1))
	 len += snprintf(buf + len, sizeof(buf) - len,
          "- Both aggresive and agressive to align.\r\n");

        if ((GET_GOLD(mob) > MAX_GOLD_ALLOWED) && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Set to %ld Gold (limit : %d).\r\n",
                                  GET_GOLD(mob),
                                  MAX_GOLD_ALLOWED);

        if (GET_EXP(mob)>MAX_EXP_ALLOWED && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- Has %ld experience (limit: %d)\r\n",
                              GET_EXP(mob), MAX_EXP_ALLOWED);
        if ((AFF_FLAGGED(mob, AFF_GROUP) || AFF_FLAGGED(mob, AFF_CHARM) || AFF_FLAGGED(mob, AFF_POISON)) && (found = 1))
	  len += snprintf(buf + len, sizeof(buf) - len,
                          "- Has illegal affection bits set (%s %s %s)\r\n",
                              AFF_FLAGGED(mob, AFF_GROUP) ? "GROUP" : "",
                              AFF_FLAGGED(mob, AFF_CHARM) ? "CHARM" : "",
                              AFF_FLAGGED(mob, AFF_POISON) ? "POISON" : "");


        if (!MOB_FLAGGED(mob, MOB_SENTINEL) && !MOB_FLAGGED(mob, MOB_STAY_ZONE) && (found = 1))
          len += snprintf(buf + len, sizeof(buf) - len,
                            "- Neither SENTINEL nor STAY_ZONE bits set.\r\n");

        if (MOB_FLAGGED(mob, MOB_SPEC) && (found = 1))
          len += snprintf(buf + len, sizeof(buf) - len,
                            "- SPEC flag needs to be removed.\r\n");

          /* Additional mob checks.*/
          if (found) {
            send_to_char(ch,
                    "%s[%5d]%s %-30s: %s\r\n",
                    CCCYN(ch, C_NRM), GET_MOB_VNUM(mob),
                    CCYEL(ch, C_NRM), GET_NAME(mob),
                    CCNRM(ch, C_NRM));
            send_to_char(ch, buf);
          }
          /* reset buffers and found flag */
          strcpy(buf, "");
          found = 0;
          len = 0;
        }   /* mob is in zone */
    }  /* check mobs */

 /* Check objects */
  send_to_char(ch, "\r\nChecking Objects for limits...\r\n");
  for (i=0; i<top_of_objt; i++) {
    if (real_zone_by_thing(obj_index[i].vnum) == zrnum) { /*is object in this zone?*/
      obj = &obj_proto[i];
      switch (GET_OBJ_TYPE(obj)) {
        case ITEM_MONEY:
          if ((value = GET_OBJ_VAL(obj, 1))>MAX_GOLD_ALLOWED && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Is worth %d (money limit %d coins).\r\n",
                                 value, MAX_GOLD_ALLOWED);
          break;
	case ITEM_BOW:
	case ITEM_CROSSBOW:
	case ITEM_SLING:
        case ITEM_WEAPON:
          if (GET_OBJ_VAL(obj, 3) >= NUM_ATTACK_TYPES && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- has out of range attack type %d.\r\n",
                                 GET_OBJ_VAL(obj, 3));

          if (GET_OBJ_AVG_DAM(obj)>MAX_DAM_ALLOWED && (found=1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Damroll is %2.1f (limit %d)\r\n",
                                 GET_OBJ_AVG_DAM(obj), MAX_DAM_ALLOWED);
          break;
        case ITEM_ARMOR:
          ac=GET_OBJ_VAL(obj,0);
          for (j=0; j<TOTAL_WEAR_CHECKS;j++) {
            if (CAN_WEAR(obj,zarmor[j].bitvector) && (ac>zarmor[j].ac_allowed) && (found=1))
              len += snprintf(buf + len, sizeof(buf) - len,
                                   "- Has AC %d (%s limit is %d)\r\n",
                                   ac, zarmor[j].message, zarmor[j].ac_allowed);
          }
          break;

      }  /*switch on Item_Type*/

      if (!CAN_WEAR(obj, ITEM_WEAR_TAKE)) {
        if ((GET_OBJ_COST(obj) || (GET_OBJ_WEIGHT(obj) && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) ||
           GET_OBJ_COND(obj)) && (found = 1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- is NO_TAKE, but has cost (%d) weight (%d) or condition (%d) set.\r\n",
                          GET_OBJ_COST(obj), GET_OBJ_WEIGHT(obj), GET_OBJ_COND(obj));
      } else {
        if (GET_OBJ_COST(obj) == 0 && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- has 0 cost (min. 1).\r\n");

        if (GET_OBJ_WEIGHT(obj) == 0 && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- has 0 weight (min. 1).\r\n");

        if (GET_OBJ_WEIGHT(obj) > MAX_OBJ_WEIGHT && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "  Weight is too high: %d (limit  %d).\r\n",
                          GET_OBJ_WEIGHT(obj), MAX_OBJ_WEIGHT);


        if (GET_OBJ_COST(obj) > MAX_OBJ_COST && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- has %d cost (max %d).\r\n",
                          GET_OBJ_COST(obj), MAX_OBJ_COST);
      }

      if (GET_OBJ_LEVEL(obj) > LVL_IMMORT-1 && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                          "- has min level set to %d (max %d).\r\n",
                          GET_OBJ_LEVEL(obj), LVL_IMMORT-1);

      if (obj->action_description && *obj->action_description &&
          GET_OBJ_TYPE(obj) != ITEM_STAFF &&
          GET_OBJ_TYPE(obj) != ITEM_WAND &&
          GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
          GET_OBJ_TYPE(obj) != ITEM_NOTE && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                          "- has action_description set, but is inappropriate type.\r\n");

      /*first check for over-all affections*/
      for (affs=0, j = 0; j < MAX_OBJ_AFFECT; j++)
        if (obj->affected[j].modifier) affs++;

      if (affs>MAX_AFFECTS_ALLOWED && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                          "- has %d affects (limit %d).\r\n",
                             affs, MAX_AFFECTS_ALLOWED);

      /*check for out of range affections. */
      for (j=0;j<MAX_OBJ_AFFECT;j++)
        if (zaffs[(int)obj->affected[j].location].max_aff != -99 && /* only care if a range is set */
            (obj->affected[j].modifier > zaffs[(int)obj->affected[j].location].max_aff ||
             obj->affected[j].modifier < zaffs[(int)obj->affected[j].location].min_aff ||
             zaffs[(int)obj->affected[j].location].min_aff == zaffs[(int)obj->affected[j].location].max_aff) && (found=1))
          len += snprintf(buf + len, sizeof(buf) - len,
                          "- apply to %s is %d (limit %d - %d).\r\n",
                               zaffs[(int)obj->affected[j].location].message,
                               obj->affected[j].modifier,
                               zaffs[(int)obj->affected[j].location].min_aff,
                               zaffs[(int)obj->affected[j].location].max_aff);

     /* special handling of +hit and +dam because of +hit_n_dam */
     for (todam=0, tohit=0, j=0;j<MAX_OBJ_AFFECT;j++) {
       if (obj->affected[j].location == APPLY_HITROLL)
         tohit += obj->affected[j].modifier;
       if (obj->affected[j].location == APPLY_DAMROLL)
         todam += obj->affected[j].modifier;
     }
     if (abs(todam) > MAX_APPLY_DAMROLL_TOTAL && (found=1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- total damroll %d out of range (limit +/-%d.\r\n",
                       todam, MAX_APPLY_DAMROLL_TOTAL);
     if (abs(tohit) > MAX_APPLY_HITROLL_TOTAL && (found=1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- total hitroll %d out of range (limit +/-%d).\r\n",
                       tohit, MAX_APPLY_HITROLL_TOTAL);


     for (ext2 = NULL, ext = obj->ex_description; ext; ext = ext->next)
       if (strncmp(ext->description, "   ", 3))
         ext2 = ext;

     if (ext2 && (found = 1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- has unformatted extra description\r\n");
     /* Additional object checks. */
     if (found) {
        send_to_char(ch, "[%5d] %-30s: \r\n", GET_OBJ_VNUM(obj), obj->short_description);
        send_to_char(ch, buf);
      }
      strcpy(buf, "");
      len = 0;
      found = 0;
    }   /*object is in zone*/
  } /*check objects*/

  /* Check rooms */
  send_to_char(ch, "\r\nChecking Rooms for limits...\r\n");
  for (i=0; i<top_of_world;i++) {
    if (world[i].zone==zrnum) {
      for (j = 0; j < NUM_OF_DIRS; j++) {
        /*check for exit, but ignore off limits if you're in an offlimit zone*/
        if (!world[i].dir_option[j])
          continue;
        exroom=world[i].dir_option[j]->to_room;
        if (exroom==NOWHERE)
          continue;
        if (world[exroom].zone == zrnum)
          continue;
        if (world[exroom].zone == world[i].zone)
          continue;

        for (k=0;offlimit_zones[k] != -1;k++) {
          if (world[exroom].zone == real_zone(offlimit_zones[k]) && (found = 1))
            len += snprintf(buf + len, sizeof(buf) - len,
                            "- Exit %s cannot connect to %d (zone off limits).\r\n",
                            dirs[j], world[exroom].number);
        } /* for (k.. */
      } /* cycle directions */

     if (ROOM_FLAGGED(i, ROOM_ATRIUM | ROOM_HOUSE | ROOM_HOUSE_CRASH | ROOM_OLC | ROOM_BFS_MARK))
         len += snprintf(buf + len, sizeof(buf) - len,
         "- Has illegal affection bits set (%s %s %s %s %s)\r\n",
                            ROOM_FLAGGED(i, ROOM_ATRIUM) ? "ATRIUM" : "",
                            ROOM_FLAGGED(i, ROOM_HOUSE) ? "HOUSE" : "",
                            ROOM_FLAGGED(i, ROOM_HOUSE_CRASH) ? "HCRSH" : "",
                            ROOM_FLAGGED(i, ROOM_OLC) ? "OLC" : "",
                            ROOM_FLAGGED(i, ROOM_BFS_MARK) ? "*" : "");

      if ((MIN_ROOM_DESC_LENGTH) && strlen(world[i].description)<MIN_ROOM_DESC_LENGTH && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                        "- Room description is too short. (%4.4d of min. %d characters).\r\n",
                             (int)strlen(world[i].description), MIN_ROOM_DESC_LENGTH);

      if (strncmp(world[i].description, "   ", 3) && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                        "- Room description not formatted with indent (/fi in the editor).\r\n");

      /* strcspan = size of text in first arg before any character in second arg */
      if ((strcspn(world[i].description, "\r\n")>MAX_COLOUMN_WIDTH) && (found=1))
        len += snprintf(buf + len, sizeof(buf) - len,
                        "- Room description not wrapped at %d chars (/fi in the editor).\r\n",
                             MAX_COLOUMN_WIDTH);

     for (ext2 = NULL, ext = world[i].ex_description; ext; ext = ext->next)
       if (strncmp(ext->description, "   ", 3))
         ext2 = ext;

     if (ext2 && (found = 1))
       len += snprintf(buf + len, sizeof(buf) - len,
                       "- has unformatted extra description\r\n");

      if (found) {
        send_to_char(ch, "[%5d] %-30s: \r\n",
                       world[i].number, world[i].name ? world[i].name : "An unnamed room");
        send_to_char(ch, buf);
        strcpy(buf, "");
        len = 0;
        found = 0;
      }
    } /*is room in this zone?*/
  } /*checking rooms*/

  for (i=0; i<top_of_world;i++) {
    if (world[i].zone==zrnum) {
      m++;
      for (j = 0, k = 0; j < NUM_OF_DIRS; j++)
        if (!world[i].dir_option[j])
          k++;

      if (k == NUM_OF_DIRS)
        l++;
    }
  }
  if (l * 3 > m)
    send_to_char(ch, "More than 1/3 of the rooms are not linked.\r\n");

}

void mob_checkload(struct char_data *ch, mob_vnum mvnum)
{
  int cmd_no;
  zone_rnum zone;
  mob_rnum mrnum = real_mobile(mvnum);

  if (mrnum == NOBODY) {
      send_to_char(ch, "That mob does not exist.\r\n");
      return;
  }

  send_to_char(ch, "Checking load info for the mob %s...\r\n",
                    mob_proto[mrnum].player.short_descr);

  for (zone=0; zone <= top_of_zone_table; zone++) {
    for (cmd_no = 0; ZCMD2.command != 'S'; cmd_no++) {
      if (ZCMD2.command != 'M')
        continue;

      /* read a mobile */
      if (ZCMD2.arg1 == mrnum) {
        send_to_char(ch, "  [%5d] %s (%d MAX)\r\n",
                         world[ZCMD2.arg3].number,
                         world[ZCMD2.arg3].name,
                         ZCMD2.arg2);
      }
    }
  }
}

void obj_checkload(struct char_data *ch, obj_vnum ovnum)
{
  int cmd_no;
  zone_rnum zone;
  obj_rnum ornum = real_object(ovnum);
  room_vnum lastroom_v = 0;
  room_rnum lastroom_r = 0;
  mob_rnum lastmob_r = 0;

  if (ornum ==NOTHING) {
    send_to_char(ch, "That object does not exist.\r\n");
    return;
  }

  send_to_char(ch, "Checking load info for the obj %s...\r\n",
                   obj_proto[ornum].short_description);

  for (zone=0; zone <= top_of_zone_table; zone++) {
    for (cmd_no = 0; ZCMD2.command != 'S'; cmd_no++) {
      switch (ZCMD2.command) {
        case 'M':
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          lastmob_r = ZCMD2.arg1;
          break;
        case 'O':                   /* read an object */
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          if (ZCMD2.arg1 == ornum)
            send_to_char(ch, "  [%5d] %s (%d Max)\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             ZCMD2.arg2);
          break;
        case 'P':                   /* object to object */
          if (ZCMD2.arg1 == ornum)
            send_to_char(ch, "  [%5d] %s (Put in another object [%d Max])\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             ZCMD2.arg2);
          break;
        case 'G':                   /* obj_to_char */
          if (ZCMD2.arg1 == ornum)
            send_to_char(ch, "  [%5d] %s (Given to %s [%d][%d Max])\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             mob_proto[lastmob_r].player.short_descr,
                             mob_index[lastmob_r].vnum,
                             ZCMD2.arg2);
          break;
        case 'E':                   /* object to equipment list */
          if (ZCMD2.arg1 == ornum)
            send_to_char(ch, "  [%5d] %s (Equipped to %s [%d][%d Max])\r\n",
                             lastroom_v,
                             world[lastroom_r].name,
                             mob_proto[lastmob_r].player.short_descr,
                             mob_index[lastmob_r].vnum,
                             ZCMD2.arg2);
            break;
          case 'R': /* rem obj from room */
            lastroom_v = world[ZCMD2.arg1].number;
            lastroom_r = ZCMD2.arg1;
            if (ZCMD2.arg2 == ornum)
              send_to_char(ch, "  [%5d] %s (Removed from room)\r\n",
                               lastroom_v,
                               world[lastroom_r].name);
            break;
      }/* switch */
    } /*for cmd_no......*/
  }  /*for zone...*/
}

void trg_checkload(struct char_data *ch, trig_vnum tvnum)
{
  int cmd_no, found = 0;
  zone_rnum zone;
  trig_rnum trnum = real_trigger(tvnum);
  room_vnum lastroom_v = 0;
  room_rnum lastroom_r = 0, k;
  mob_rnum lastmob_r = 0, i;
  obj_rnum lastobj_r = 0, j;
  struct trig_proto_list *tpl;

  if (trnum == NOTHING) {
    send_to_char(ch, "That trigger does not exist.\r\n");
    return;
  }

  send_to_char(ch, "Checking load info for the %s trigger '%s':\r\n",
                    trig_index[trnum]->proto->attach_type == MOB_TRIGGER ? "mobile" :
                    (trig_index[trnum]->proto->attach_type == OBJ_TRIGGER ? "object" : "room"),
                    trig_index[trnum]->proto->name);

  for (zone=0; zone <= top_of_zone_table; zone++) {
    for (cmd_no = 0; ZCMD2.command != 'S'; cmd_no++) {
      switch (ZCMD2.command) {
        case 'M':
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          lastmob_r = ZCMD2.arg1;
          break;
        case 'O':                   /* read an object */
          lastroom_v = world[ZCMD2.arg3].number;
          lastroom_r = ZCMD2.arg3;
          lastobj_r = ZCMD2.arg1;
          break;
        case 'P':                   /* object to object */
          lastobj_r = ZCMD2.arg1;
          break;
        case 'G':                   /* obj_to_char */
          lastobj_r = ZCMD2.arg1;
          break;
        case 'E':                   /* object to equipment list */
          lastobj_r = ZCMD2.arg1;
          break;
        case 'R':                   /* rem obj from room */
          lastroom_v = 0;
          lastroom_r = 0;
          lastobj_r = 0;
          lastmob_r = 0;
        case 'T':                   /* trigger to something */
          if (ZCMD2.arg2 != trnum)
            break;
          if (ZCMD2.arg1 == MOB_TRIGGER) {
            send_to_char(ch, "mob [%5d] %-60s (zedit room %5d)\r\n",
                               mob_index[lastmob_r].vnum,
                               mob_proto[lastmob_r].player.short_descr,
                               lastroom_v);
            found = 1;
          } else if (ZCMD2.arg1 == OBJ_TRIGGER) {
            send_to_char(ch, "obj [%5d] %-60s  (zedit room %d)\r\n",
                               obj_index[lastobj_r].vnum,
                               obj_proto[lastobj_r].short_description,
                               lastroom_v);
            found = 1;
          } else if (ZCMD2.arg1==WLD_TRIGGER) {
            send_to_char(ch, "room [%5d] %-60s (zedit)\r\n",
                               lastroom_v,
                               world[lastroom_r].name);
            found = 1;
          }
        break;
      } /* switch */
    } /*for cmd_no......*/
  }  /*for zone...*/

  for (i = 0; i < top_of_mobt; i++) {
    if (!mob_proto[i].proto_script)
      continue;

    for (tpl = mob_proto[i].proto_script;tpl;tpl = tpl->next)
      if (tpl->vnum == tvnum) {
        send_to_char(ch, "mob [%5d] %s\r\n",
                         mob_index[i].vnum,
                         mob_proto[i].player.short_descr);
        found = 1;
      }
  }

  for (j = 0; j < top_of_objt; j++) {
    if (!obj_proto[j].proto_script)
      continue;

    for (tpl = obj_proto[j].proto_script;tpl;tpl = tpl->next)
      if (tpl->vnum == tvnum) {
        send_to_char(ch, "obj [%5d] %s\r\n",
                         obj_index[j].vnum,
                         obj_proto[j].short_description);
        found = 1;
      }
  }

  for (k = 0;k < top_of_world; k++) {
    if (!world[k].proto_script)
      continue;

    for (tpl = world[k].proto_script;tpl;tpl = tpl->next)
      if (tpl->vnum == tvnum) {
        send_to_char(ch, "room[%5d] %s\r\n",
                         world[k].number,
                         world[k].name);
        found = 1;
      }
  }

  if (!found)
    send_to_char(ch, "This trigger is not attached to anything.\r\n");
}

ACMD(do_checkloadstatus)
{
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  two_arguments(argument, buf1, buf2);

  if ((!*buf1) || (!*buf2) || (!isdigit(*buf2))) {
    send_to_char(ch, "Checkload <M | O | T> <vnum>\r\n");
    return;
  }

  if (LOWER(*buf1) == 'm') {
    mob_checkload(ch, atoidx(buf2));
    return;
  }

  if (LOWER(*buf1) == 'o') {
    obj_checkload(ch, atoidx(buf2));
    return;
  }

  if (LOWER(*buf1) == 't') {
    trg_checkload(ch, atoidx(buf2));
    return;
  }
}
/* Zone Checker code above. */

/* (c) 1996-97 Erwin S. Andreasen. */
ACMD(do_copyover)
{
  FILE *fp;
  struct descriptor_data *d, *d_next;
  char buf [100], buf2[100];

  fp = fopen (COPYOVER_FILE, "w");
    if (!fp) {
      send_to_char (ch, "Copyover file not writeable, aborted.\n\r");
      return;
    }

   sprintf (buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r", GET_NAME(ch));

   /* write boot_time as first line in file */
   fprintf(fp, "%ld\n", boot_time);

   /* For each playing descriptor, save its state */
   for (d = descriptor_list; d ; d = d_next) {
     struct char_data * och = d->character;
   /* We delete from the list , so need to save this */
     d_next = d->next;

  /* drop those logging on */
   if (!d->character || d->connected > CON_PLAYING) {
     write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r");
     close_socket (d); /* throw'em out */
   } else {
      fprintf (fp, "%d %ld %s %s\n", d->descriptor, GET_PREF(och), GET_NAME(och), d->host);
      /* save och */
      Crash_rentsave(och,0);
      save_char(och);
      write_to_descriptor (d->descriptor, buf);
    }
  }

  fprintf (fp, "-1\n");
  fclose (fp);

  /* exec - descriptors are inherited */
  sprintf (buf, "%ld", port);
  sprintf (buf2, "-C%d", mother_desc);

  /* Ugh, seems it is expected we are 1 step above lib - this may be dangerous! */
  chdir ("..");

  /* Close reserve and other always-open files and release other resources */
   execl (EXE_FILE, "circle", buf2, buf, (char *) NULL);

   /* Failed - successful exec will not return */
   perror ("do_copyover: execl");
   send_to_char (ch, "Copyover FAILED!\n\r");

 exit (1); /* too much trouble to try to recover! */
}




ACMD(do_peace)
{
  struct char_data *vict, *next_v;

  act ("As $n makes a strange arcane gesture, a golden light descends\r\n"
       "from the heavens stopping all the fighting.\r\n",FALSE, ch, 0, 0, TO_ROOM);
  send_to_room(IN_ROOM(ch), "Everything is quite peaceful now.\r\n");
  for(vict=world[IN_ROOM(ch)].people; vict; vict=next_v) {
    next_v = vict->next_in_room;
    if (FIGHTING(vict))
      stop_fighting(vict);
    if (IS_NPC(vict))
      clearMemory(vict);
  }
}

ACMD(do_zpurge)
{
  int vroom, room, vzone = 0, zone = 0;
  char arg[MAX_INPUT_LENGTH];
  int purge_all = FALSE;
  one_argument(argument, arg);
  if (*arg == '.' || !*arg) {
    zone = world[IN_ROOM(ch)].zone;
    vzone = zone_table[zone].number;
  }
  else if (is_number(arg)) {
    vzone = atoi(arg);
    zone = real_zone(vzone);
    if (zone == NOWHERE || zone > top_of_zone_table) {
      send_to_char(ch, "That zone doesn't exist!\r\n");
      return;
    }
  }
  else if (*arg == '*') {
    purge_all = TRUE;
  }
  else {
    send_to_char(ch, "That isn't a valid zone number!\r\n");
    return;
  }
  if (GET_LEVEL(ch) < LVL_GOD && !can_edit_zone(ch, zone)) {
    send_to_char(ch, "You can only purge your own zone!\r\n");
    return;
  }
  if (!purge_all) {
    for (vroom = zone_table[zone].bot; vroom <= zone_table[zone].top; vroom++) {
      purge_room(real_room(vroom));
    }
    send_to_char(ch, "Purged zone #%d: %s.\r\n", zone_table[zone].number, zone_table[zone].name);
    mudlog(NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s purged zone %d (%s)", GET_NAME(ch), zone, zone_table[zone].name);
  }
  else {
    for (room = 0; room <= top_of_world; room++) {
      purge_room(room);
    }
    send_to_char(ch, "Purged world.\r\n");
    mudlog(NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE, "(GC) %s purged entire world.", GET_NAME(ch));
  }
}

ACMD(do_file)
{
  FILE *req_file;
  int cur_line = 0, num_lines = 0, req_lines = 0, i, j, l;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], line[READ_SIZE];
  size_t len = 0, nlen;

  struct file_struct {
    char *cmd;
    char level;
    char *file;
  } fields[] = {
    { "bugs",           LVL_GOD,    "../lib/misc/bugs"},
    { "typos",          LVL_GOD,    "../lib/misc/typos"},
    { "ideas",          LVL_GOD,    "../lib/misc/ideas"},
    { "xnames",         LVL_GOD,    "../lib/misc/xnames"},
    { "levels",         LVL_GOD,    "../log/levels" },
    { "rip",            LVL_GOD,    "../log/rip" },
    { "players",        LVL_GOD,    "../log/newplayers" },
    { "rentgone",       LVL_GOD,    "../log/rentgone" },
    { "errors",         LVL_GOD,    "../log/errors" },
    { "godcmds",        LVL_GOD,    "../log/godcmds" },
    { "syslog",         LVL_GOD,    "../syslog" },
    { "crash",          LVL_GOD,    "../syslog.CRASH" },
    { "help",           LVL_GOD,    "../log/help" },
    { "changelog",      LVL_GOD,    "../changelog" },
    { "\n", 0, "\n" }
  };

   skip_spaces(&argument);

   if (!*argument) {
     send_to_char(ch, "USAGE: file <option> <num lines>\r\n\r\nFile options:\r\n");
     for (j = 0, i = 0; fields[i].level; i++)
       if (fields[i].level <= GET_LEVEL(ch))
         send_to_char(ch, "%-15s%s\r\n", fields[i].cmd, fields[i].file);
     return;
   }

   two_arguments(argument, field, value);

   for (l = 0; *(fields[l].cmd) != '\n'; l++)
     if (!strncmp(field, fields[l].cmd, strlen(field)))
     break;

   if(*(fields[l].cmd) == '\n') {
     send_to_char(ch, "That is not a valid option!\r\n");
     return;
   }

   if (GET_LEVEL(ch) < fields[l].level) {
     send_to_char(ch, "You are not godly enough to view that file!\r\n");
     return;
   }
   
   if(!*value)
     req_lines = 15; /* Default is the last 15 lines. */
   else
     req_lines = atoi(value);

   if (!(req_file=fopen(fields[l].file,"r"))) {
     mudlog(BRF, LVL_IMPL, TRUE,
            "SYSERR: Error opening file %s using 'file' command.",
            fields[l].file);
     return;
   }

   get_line(req_file, line);
   while (!feof(req_file)) {
     num_lines++;
     get_line(req_file,line);
   }
   rewind(req_file);

   /* Limit the maximum number of lines to 300. */
   req_lines = MIN(MIN(req_lines, num_lines), 300);

   len = snprintf(buf, sizeof(buf), "Last %d lines of %s:\r\n", req_lines, fields[l].file);

   if (req_lines == num_lines)
     len += snprintf(buf + len, sizeof(buf) - len, "Top of file.\r\n");
   get_line(req_file,line);
   while (!feof(req_file)) {
     cur_line++;
     if(cur_line > (num_lines - req_lines)) {
       nlen = snprintf(buf + len, sizeof(buf) - len, "%s\r\n", line);
       if (len + nlen >= sizeof(buf) || nlen < 0)
         break;
       len += nlen;
     }
     get_line(req_file,line);
   }
   fclose(req_file);

   if (len >= sizeof(buf)) {
     const char *overflow = "\r\n**OVERFLOW**\r\n";
     strcpy(buf + sizeof(buf) - strlen(overflow) - 1, overflow); /* strcpy: OK */
   }
   page_string(ch->desc, buf, 1);
}

ACMD(do_changelog)
{
  time_t rawtime;      
  char tmstr[MAX_INPUT_LENGTH], line[READ_SIZE], last_buf[READ_SIZE],
      buf[READ_SIZE];   
  FILE *fl, *new;
 
  skip_spaces(&argument);
 
  if (!*argument) {
    send_to_char(ch, "Usage: changelog <change>\r\n");
    return;
  }
 
  sprintf(buf, "%s.bak", CHANGE_LOG_FILE);
  if (rename(CHANGE_LOG_FILE, buf)) {
    mudlog(BRF, LVL_IMPL, TRUE,
           "SYSERR: Error making backup changelog file (%s)", buf);
    return;
  }
 
  if (!(fl = fopen(buf, "r"))) {
    mudlog(BRF, LVL_IMPL, TRUE,
           "SYSERR: Error opening backup changelog file (%s)", buf);
    return;
  }
 
  if (!(new = fopen(CHANGE_LOG_FILE, "w"))) {
    mudlog(BRF, LVL_IMPL, TRUE,
           "SYSERR: Error opening new changelog file (%s)", CHANGE_LOG_FILE);
    return;
  }
 
  while (get_line(fl, line)) {
    if (*line != '[')
      fprintf(new, "%s\n", line);
    else {
      strcpy(last_buf, line);
      break;
    }
  }
 
  rawtime = time(0);
  strftime(tmstr, sizeof(tmstr), "%b %d %Y", localtime(&rawtime));
 
  sprintf(buf, "[%s] - %s", tmstr, GET_NAME(ch));
 
  fprintf(new, "%s\n", buf);
  fprintf(new, "  %s\n", argument);
 
  if (strcmp(buf, last_buf))
    fprintf(new, "%s\n", line);
 
  while (get_line(fl, line))
    fprintf(new, "%s\n", line);
 
  fclose(fl);
  fclose(new);
  send_to_char(ch, "Change added.\r\n");
}

#define PLIST_FORMAT \
  "Usage: plist [minlev[-maxlev]] [-n name] [-d days] [-h hours] [-i] [-m]"

ACMD(do_plist)
{
  int i, len = 0, count = 0;
  char mode, name_search[MAX_NAME_LENGTH], time_str[MAX_STRING_LENGTH];
  struct time_info_data time_away;
  int low = 0, high = LVL_IMPL, low_day = 0, high_day = 10000, low_hr = 0, high_hr = 24;

  skip_spaces(&argument);
  strcpy(buf, argument);        /* strcpy: OK (sizeof: argument == buf) */
  name_search[0] = '\0';

  while (*buf) {
    char arg[MAX_INPUT_LENGTH], buf1[MAX_INPUT_LENGTH];

    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      if (sscanf(arg, "%d-%d", &low, &high) == 1)
        high = low;
      strcpy(buf, buf1);        /* strcpy: OK (sizeof: buf1 == buf) */
    } else if (*arg == '-') {
      mode = *(arg + 1);        /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'l':
        half_chop(buf1, arg, buf);
        sscanf(arg, "%d-%d", &low, &high);
        break;
      case 'n':
        half_chop(buf1, name_search, buf);
        break;
      case 'i':
        strcpy(buf, buf1);
        low = LVL_GOD;
        break;
      case 'm':
        strcpy(buf, buf1); 
        high = LVL_GOD - 1;
        break;
      case 'd':
        half_chop(buf1, arg, buf);
        if (sscanf(arg, "%d-%d", &low_day, &high_day) == 1)
          high_day = low_day;
        break;
      case 'h':
        half_chop(buf1, arg, buf);
        if (sscanf(arg, "%d-%d", &low_hr, &high_hr) == 1)
          high_hr = low_hr;
        break;
      default:
        send_to_char(ch, "%s\r\n", PLIST_FORMAT);
        return;   
      }
    } else {
      send_to_char(ch, "%s\r\n", PLIST_FORMAT);
      return;
    }
  }

  len = 0;   
  len += snprintf(buf + len, sizeof(buf) - len, "@W[ Id] (Lv) Name         Last@n\r\n"
                  "%s-----------------------------------------------%s\r\n", CCCYN(ch, C_NRM),
                  CCNRM(ch, C_NRM));

  for (i = 0; i <= top_of_p_table; i++) {
    if (player_table[i].level < low || player_table[i].level > high)
      continue;

    time_away = *real_time_passed(time(0), player_table[i].last);

    if (*name_search && str_cmp(name_search, player_table[i].name))
      continue;

    if (time_away.day > high_day || time_away.day < low_day)
      continue;
    if (time_away.hours > high_hr || time_away.hours < low_hr)
      continue;

    strcpy(time_str, asctime(localtime(&player_table[i].last)));
    time_str[strlen(time_str) - 1] = '\0';

    len += snprintf(buf + len, sizeof(buf) - len, "[%3ld] (%2d) %c%-15s %s\r\n",
                    player_table[i].id, player_table[i].level,
                    UPPER(*player_table[i].name), player_table[i].name + 1, time_str);
    count++;
  }
  snprintf(buf + len, sizeof(buf) - len, "%s-----------------------------------------------%s\r\n"
           "%d players listed.\r\n", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM), count);
  page_string(ch->desc, buf, TRUE);
}

ACMD(do_wizupdate)
{
  run_autowiz();
  send_to_char(ch, "Wizlists updated.\n\r");
}

ACMD(do_reclass)
{
  char buf[MAX_INPUT_LENGTH];
  int i, room;
  int oldcl=100;

  one_argument(argument, buf);

  room = 112;

  if (!*buf)
  {
     send_to_char(ch, "Reclass to what?\r\n");
     return;
  }

  if (GET_LEVEL(ch) < LVL_IMMORT)
  {
     send_to_char(ch, "Sorry, you are too small for that.\r\n");
     return;
  }

  if (IN_ROOM(ch) != real_room(room) && GET_LEVEL(ch) <= LVL_GOD)
  {
     send_to_char(ch,"You are not in the proper place for that!\r\n");
     return;
  }

  send_to_char(ch, "\r\n");

  switch(*buf)
  {
    case 'M':
    case 'm':
     if(!strcmp(buf, "mage"))
     {
       if (GET_WAS_MAGE(ch) != 0 && (GET_WAS_CLERIC(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0))
       {
       send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
       break;
       }
       send_to_char(ch, "You have just reclassed to Mage!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_MAGIC_USER;
       }
       else
       {
        for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_MAGIC_USER);
       }
       break;
     }
     else if(!strcmp(buf, "mystic"))
     {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_CLERIC)
       {
       send_to_char(ch, "You can't reclass to this unless you are a cleric.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Mystic!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_MYSTIC;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_MYSTIC);
       }
       break;
     }

     else
      send_to_char(ch, "Sorry, you can't do that.");
     break;

    case 'T':
    case 't':
     if(!strcmp(buf, "thief"))
     {
       if (GET_WAS_THIEF(ch) != 0 && (GET_WAS_CLERIC(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_MAGE(ch) == 0))
       {
       send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Thief!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_THIEF;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
          if (GET_EQ(ch, i))
           perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_THIEF);       
      }
       break;
     }
     else
      send_to_char(ch, "Sorry, you can't do that.");
      break;

 case 'W':
    case 'w':
     if(!strcmp(buf, "warrior"))
     {
       if (GET_WAS_WARRIOR(ch) != 0 && (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_THIEF(ch) == 0))
       {
       send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Warrior!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_WARRIOR;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_WARRIOR);
       }
       break;
     }
     else
      send_to_char(ch, "Sorry, you can't do that.");
      break;

    case 'R':
    case 'r':
      if(!strcmp(buf, "ranger"))
      {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_WARRIOR)
       {
       send_to_char(ch, "You can't reclass to this unless you are a warrior.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Ranger!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_RANGER;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_RANGER);
       }
       break;
       }
      else if (!strcmp(buf, "rogue"))
      {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_THIEF)
       {
       send_to_char(ch, "You can't reclass to this unless you are a thief.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Rogue!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_ROGUE;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_ROGUE);
       }
       break;

      }
      else
       send_to_char(ch, "You can't do that.");
       break;
    case 'C':
    case 'c':
     if(!strcmp(buf, "cleric"))
     {
       if (GET_WAS_CLERIC(ch) != 0 && (GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0))
       {
       send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Cleric!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_CLERIC;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_CLERIC);
       }
       break;
     }
     else
      send_to_char(ch, "Sorry, you can't do that.");
      break;

    case 'K':
    case 'k':
     if(!strcmp(buf, "knight"))
     {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_WARRIOR)
       {
       send_to_char(ch, "You can't reclass to this unless you are a warrior.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Knight!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_KNIGHT;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_KNIGHT);
       }
       break;
     }
     else
      send_to_char(ch, "Sorry, you can't do that.");
      break;

    case 'P':
    case 'p':
      if(!strcmp(buf, "paladin"))
      {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_WARRIOR)
       {
       send_to_char(ch, "You can't reclass to this unless you are a warrior.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Paladin!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_PALADIN;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_PALADIN);
       }
       break;
      }
      else if(!strcmp(buf, "priest"))
      {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_CLERIC)
       {
       send_to_char(ch, "You can't reclass to this unless you are a cleric.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Priest!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_PRIEST;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_PRIEST);
       }
       break;
      }
      else
       send_to_char(ch, "You can't do that.");
       break;

   case 'A':
   case 'a':
   if(!strcmp(buf, "assassin"))
   {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_THIEF)
       {
       send_to_char(ch, "You can't reclass to this unless you are a thief.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Assassin!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_ASSASSIN;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_ASSASSIN);
       }
       break;
     }
     else
      send_to_char(ch, "Sorry, you can't do that.");
      break;

   case 'N':
   case 'n':
    if(!strcmp(buf, "ninja"))
    {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_THIEF)
       {
       send_to_char(ch, "You can't reclass to this unless you are a thief.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Ninja!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_NINJA;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_NINJA);
       }
       break;
     }
     else if(!strcmp(buf, "necromancer"))
     {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_MAGIC_USER)
       {
       send_to_char(ch, "You can't reclass to this unless you are a mage.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Necromancer!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_NECROMANCER;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_NECROMANCER);
       }
       break;
     }
     else
       send_to_char(ch, "You can't do that.");
       break;
    case 'S':
    case 's':
    if(!strcmp(buf, "sorcerer"))
     {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_MAGIC_USER)
       {
       send_to_char(ch, "You can't reclass to this unless you are a mage.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Sorcerer!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_SORCERER;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_SORCERER);
       }
       break;
     }
    else if (!strcmp(buf, "shaman")){
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }
       if (GET_CLASS(ch) != CLASS_CLERIC)
       {
       send_to_char(ch, "You can't reclass to this unless you are a cleric.");
       break;
       }
       send_to_char(ch, "You have just reclassed to Shaman!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_SHAMAN;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_SHAMAN);
       }
       break;
     }
     else
       send_to_char(ch, "Sorry, thats not an option.");
       break;

    case 'e':
    case 'E':
     if(!strcmp(buf, "elementalist"))
     {
       if (GET_WAS_CLERIC(ch) == 0 || GET_WAS_MAGE(ch) == 0 || GET_WAS_WARRIOR(ch) == 0 || GET_WAS_THIEF(ch) == 0)
       {
         send_to_char(ch, "You can't reclass to this until you have finished the other classes.");
         break;
       }

       if (GET_CLASS(ch) != CLASS_MAGIC_USER)
       {
       send_to_char(ch, "You can't reclass to this unless you are a mage.");
       break;
       }

       send_to_char(ch, "You have just reclassed to Elementalist!");
       if (GET_LEVEL(ch) >= LVL_GOD)
       {
         oldcl = GET_CLASS(ch);
         GET_CLASS(ch) = CLASS_ELEMENTALIST;
       }
       else
       {
         for (i = 0; i < NUM_WEARS; i++)
         {
           if (GET_EQ(ch, i))
            perform_remove(ch, i);
         }
         oldcl = do_multi(ch, CLASS_ELEMENTALIST);
       }
       break;
     }
     else
       send_to_char(ch, "Sorry, thats not an option.");
       break;

    default:
       send_to_char(ch, "Sorry, thats not an option.");
       break;

 }

 switch(oldcl)
  {
    case 0:
       GET_WAS_MAGE(ch) += 1;
       break;
    case 2:
       GET_WAS_THIEF(ch) += 1;
       break;
    case 3:
       GET_WAS_WARRIOR(ch) += 1;
       break;
    case 1:
       GET_WAS_CLERIC(ch) += 1;
       break;
    case 4:
       GET_WAS_RANGER(ch) += 1;
       break;
    case 5:
       GET_WAS_KNIGHT(ch) += 1;
       break;
    case 6:
       GET_WAS_PALADIN(ch) += 1;
       break;
    case 7:
       GET_WAS_ROGUE(ch) += 1;
       break;
    case 8:
       GET_WAS_NINJA(ch) += 1;
       break;
    case 9:
       GET_WAS_ASSASSIN(ch) += 1;
       break;
    case 10:
       GET_WAS_MYSTIC(ch) += 1;
       break;
    case 11:
       GET_WAS_SHAMAN(ch) += 1;
       break;
    case 12:
       GET_WAS_PRIEST(ch) += 1;
       break;
    case 13:
       GET_WAS_NECROMANCER(ch) += 1;
       break;
    case 14:
       GET_WAS_ELEMENTALIST(ch) += 1;
       break;
    case 15:
       GET_WAS_SORCERER(ch) += 1;
       break;
    default:
       break;
  }
}

#define godlisttitle "\r\n&Y                          Current Gods Created&n"
#define linebreak "\r\n&c-----------------------------------------------------------------------------&n\r\n"

ACMD(do_godlist)
{
  int gcount, tcount=0;
  *buf=0;
  char here[MSL];
  
/*
   if (!(here = get_char_vis(ch, value, NULL, FIND_CHAR_WORLD)))
     send_to_char(ch, "&C%s isn't around right now.\r\n", GET_NAME(vict));
   else
     send_to_char(ch, "&C%s is currently online!\r\n", GET_NAME(vict));

   if (GET_LEVEL(ch) == LVL_IMPL)
     send_to_char(ch, "&WLast:&R %-20.16s&n\r\n", ctime(&vict->player.time.logon));

*/


  for (gcount=0; gcount <= top_of_p_table; gcount++) {
   if ((player_table + gcount)->level >= LVL_GOD) {
     sprintf(buf, "%s &c(%d)&C%-15s&n", buf, (player_table + gcount)->level, (player_table + gcount)->name);
     tcount++;
     }
    if (tcount == 4) {
     tcount=0;
     strcat(buf, "\r\n");
  }
}
   send_to_char(ch, "%s", godlisttitle);
   send_to_char(ch, "%s", linebreak);
   send_to_char(ch, "%s", buf);
   send_to_char(ch, "%s", linebreak);

//  page_string(ch->desc, buf, 1);
}

ACMD(do_wizhelp)
{
  extern int *cmd_sort_info;
  int no, i, cmd_num, j;
  char k[MAX_STRING_LENGTH];

  sprintf(buf, "The following commands are available to you:\r\n");

  for (no = 0, j = GET_LEVEL(ch); j >= LVL_GOD; j--) {
    if (no % 6)
      sprintf(buf + strlen(buf), "\r\n");

      if (j == 43)
        sprintf(k,"Implementor Commands");
      if (j == 42)
        sprintf(k,"Builder Commands");
      if (j == 41)
        sprintf(k,"God Commands");
      if (j < 41) 
        sprintf(k,"Mortal Commands");

      sprintf(buf + strlen(buf), "\r\n&c%2d: &y%s&n\r\n", j, k);

    for (no = 0, cmd_num = 1; complete_cmd_info[cmd_num].command[0] != '\n' ; cmd_num++) {
      i = cmd_sort_info[cmd_num];
      if (GET_LEVEL(ch) >= complete_cmd_info[i].minimum_level &&
          (complete_cmd_info[i].minimum_level == j)) {
        sprintf(buf + strlen(buf), " %-11s%s", complete_cmd_info[i].command, (!(++no % 6) ? "\r\n" : ""));
      }
    }
  }

  if (no % 6)
    strcat(buf, "\r\n");

  page_string(ch->desc, buf, 1);
}

const char *logtypes[] = {
  "off", "brief", "normal", "complete", "\n"
};

ACMD(do_syslog)
{
  char arg[MAX_INPUT_LENGTH];
  int tp;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char(ch, "Your syslog is currently %s.\r\n",
        logtypes[(PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0)]);
    send_to_char(ch, "Usage: syslog { Off | Brief | Normal | Complete }\r\n");
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char(ch, "Usage: syslog { Off | Brief | Normal | Complete }\r\n");
    return;
  }
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
  SET_BIT_AR(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

  send_to_char(ch, "Your syslog is now %s.\r\n", logtypes[tp]);
}

