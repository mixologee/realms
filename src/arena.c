/* ************************************************************************
 *   File: arena.c                                   Addition to CircleMUD *
 *  Usage: Implementation of a event driven arena where players pay to kill*
 *                                                                         *
 *  Writen by:  Kevin Hoogheem aka Goon                                    *
 *              Modified by Billy H. Chan (STROM)                          *
 *                                                                         *
 * Using this code without consent by Goon will make your guts spill       *
 * out or worse.. Maybe I will hire Lauraina Bobbet to come visit you      *
 ************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "arena.h"

/*  external vars  */
extern FILE *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
//extern int top_of_zone_table;
extern int restrict;
extern room_rnum top_of_world;
//extern int top_of_mobt;
//extern int top_of_objt;
extern int top_of_p_table;
extern sh_int r_mortal_start_room;

int in_arena = ARENA_OFF;
int start_time;
int game_length;
int lo_lim;
int hi_lim;
int cost_per_lev;
int time_to_start;
int time_left_in_game;
long arena_pot;
long bet_pot;

struct hall_of_fame_element *fame_list = NULL;

void send_to_arena(char *messg);


ACMD(do_bet)
{
  int newbet;
  struct char_data *bet_on;

  two_arguments(argument, arg, buf1);
 
  if (IS_NPC(ch)) {
    send_to_char(ch, "Mobs cant bet on the arena.\r\n");
    return;
  }
 
  if (!*arg) {
    if (in_arena == ARENA_OFF) {
      send_to_char(ch,"Sorry no arena is in going on.\r\n");
      return;
    } else if (in_arena == ARENA_START) {
        send_to_char(ch,"Usage: bet <player> <amt>\r\n");	 
        return;
    } else if (in_arena == ARENA_RUNNING) {
        send_to_char(ch,"Fighting has already begun - no more bets.\r\n");
        return;
    }
  }

  if (in_arena == ARENA_OFF) {
    send_to_char(ch,"The arena is closed - wait until it opens to bet.\r\n");
  } else if (in_arena == ARENA_RUNNING) {
      send_to_char(ch,"Arena is in session, no more bets.\r\n");
  } else if (!(bet_on = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)))
      send_to_char(ch, CONFIG_NOPERSON);
    else if (!(world[bet_on->in_room].zone == ARENA_ZONE && 
               ROOM_FLAGGED(IN_ROOM(bet_on), ROOM_ARENA)))
      send_to_char(ch,"Sorry that person is not in the arena.\r\n");
  else {
    if(GET_BET_AMT(ch) > 0) {
      send_to_char(ch,"Sorry you have already bet.\r\n");
      return;
    }
    GET_BETTED_ON(ch) = GET_IDNUM(bet_on); 
    newbet =  atoi(buf1);
    if (newbet == 0) {
      send_to_char(ch,"Bet some gold why dont you!\r\n");
      return;
    }
    if (newbet > GET_GOLD(ch)) {
      send_to_char (ch,"You don't have that much money!\n\r");
      return;
    }
    if (newbet > MAX_BET) {
      send_to_char(ch,"Sorry the house will not accept that much.\r\n");
      return;
    }

  *buf2 = '\0';
  GET_GOLD(ch) -= newbet;  /* substract the gold */
  arena_pot += (newbet / 2);
  bet_pot += (newbet / 2);
  GET_BET_AMT(ch) = newbet;
  sprintf(buf2, "You place %d coins on %s.\r\n", newbet, GET_NAME(bet_on));
  send_to_char(ch, buf2);
  *buf = '\0';
  sprintf(buf,"%s has placed %d coins on %s.", GET_NAME(ch),
          newbet, GET_NAME(bet_on));
  send_to_arena(buf);
  }
}


ACMD(do_arena)
{

  if (IS_NPC(ch)) {
    send_to_char(ch,"Mobs cant play in the arena.\r\n");
    return;
  }
  if (in_arena == ARENA_OFF) {
    send_to_char(ch,"The killing fields are closed right now.\r\n");
  } else if (GET_LEVEL(ch) < lo_lim) {
      sprintf(buf, "You must be at least level %d to enter this arena.\r\n", 
              lo_lim);
      send_to_char(ch,buf);
  } else if (PLR_FLAGGED(ch, PLR_KILLER) || PLR_FLAGGED(ch, PLR_THIEF)) {
      send_to_char(ch,"Wanted criminals can not play in the arena");
  } else if (GET_LEVEL(ch) > hi_lim) {
      send_to_char(ch,"Sorry the killing fields are not open to you\r\n");
  } else if (GET_GOLD(ch) < (cost_per_lev * GET_LEVEL(ch))) {
      sprintf(buf, "Sorry but you need %d coins to enter the arena\r\n",
              (cost_per_lev * GET_LEVEL(ch)) );
      send_to_char(ch,buf);      
  } else if (in_arena == ARENA_RUNNING) {
      send_to_char(ch,"The fighting has already begun.\r\n");
  } else if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA)) {
      send_to_char(ch,"You are already in the arena.\r\n");
  } else {
      act("$n has been taken to the killing fields.",FALSE, ch, 0, 0, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, real_room(rand_number(ARENA_PREP_START,ARENA_PREP_END)));
      act("$n is droped from the sky.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(ch,"You have been taken to the killing fields\r\n");
      look_at_room(ch, 0);
      sprintf(buf, "%s has joined the blood bath.", GET_NAME(ch));
      send_to_arena(buf);
      GET_GOLD(ch) -= (cost_per_lev * GET_LEVEL(ch));
      arena_pot += (cost_per_lev * GET_LEVEL(ch));
      sprintf(buf, "You pay %d coins to enter the arena\r\n", 
              (cost_per_lev * GET_LEVEL(ch)));
      send_to_char(ch,buf);
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      if (ch->affected) 
        while (ch->affected)
          affect_remove(ch, ch->affected);
  }
}


ACMD(do_chaos)
{
  char cost[MAX_INPUT_LENGTH], lolimit[MAX_INPUT_LENGTH];
  char hilimit[MAX_INPUT_LENGTH], start_delay[MAX_INPUT_LENGTH];
  char length[MAX_INPUT_LENGTH];

  /* Usage: chaos lo hi start_delay cost/lev length */

  if (in_arena != ARENA_OFF) {
    send_to_char(ch,"There is an arena running already.\r\n");
    return;
  }
  half_chop(argument, lolimit, buf);
  lo_lim = atoi(lolimit);
  half_chop(buf, hilimit, buf);
  hi_lim = atoi(hilimit);
  half_chop(buf, start_delay, buf);
  start_time = atoi(start_delay);
  half_chop(buf, cost, buf);
  cost_per_lev = atoi(cost);
  strcpy(length, buf);
  game_length = atoi(length);

  if (hi_lim > LVL_IMPL ) {
    send_to_char(ch,"Please choose a hi_lim under the Imps level\r\n");
    return;
  }
  if (lolimit < 0)
    silent_end();
  if (!*lolimit || !*hilimit || !*start_delay || !*cost || !*length) {
    send_to_char(ch,"Usage: chaos lo hi start_delay cost/lev length\r\n");
    return;
  }
  if (lo_lim > hi_lim) {
    send_to_char(ch,"Sorry, low limit must be lower than hi limit.\r\n");
    return;
  }
  if ((hi_lim < 0) || (cost_per_lev < 0) || (game_length < 0 )) {
    send_to_char(ch,"I like positive numbers thank you.\r\n");
    return;
  }
  if ( start_time <= 0) {
    send_to_char(ch,"Lets at least give them a chance to enter!\r\n");
    return;
  }
  if ((GET_LEVEL(ch) < LVL_IMPL) && (cost_per_lev < MIN_ARENA_COST)) {
    send_to_char(ch,"The minimum cost per level is 100.\r\n");
    return;
  }
  in_arena = ARENA_START;
  time_to_start = start_time;
  time_left_in_game =0;
  arena_pot =0; 
  bet_pot = 0; 
  start_arena();
}


void start_arena()
{

  if (time_to_start == 0) {
    show_jack_pot();
    in_arena = ARENA_RUNNING;    /* start the blood shed */
    time_left_in_game = game_length;
    start_game();
  } else {
      if(time_to_start >1) {
        sprintf(buf1, "The Killing Fields are open to levels %d thru %d",
                lo_lim, hi_lim);
        send_to_arena(buf1);
        sprintf(buf1, "%d coins/level to enter. %d hours to start\r\n",
                cost_per_lev, time_to_start);
        send_to_arena(buf1);
        sprintf(buf1, "Type arena to enter.");
        send_to_arena(buf1);
  } else {
      sprintf(buf1, "The Killing Fields are open to levels %d thru %d",
              lo_lim, hi_lim);
      send_to_arena(buf1);
      sprintf(buf1, "%d coins/level to enter. 1 hour to start\r\n",
	      cost_per_lev);
      send_to_arena(buf1);
      sprintf(buf1, "Type arena to enter.");
      send_to_arena(buf1);
  }
  time_to_start--;
  }
}


void show_jack_pot()
{
  sprintf(buf1, "\007\007Lets get ready to RUMBLE!!!!!!!!\r\n");
  send_to_arena(buf1);  
  sprintf(buf1, "The jackpot for this arena is %ld coins.", arena_pot);
  send_to_arena(buf1);
  sprintf(buf1, "There are %ld coins in the betting pool.", bet_pot);
  send_to_arena(buf1);
}


void start_game()
{
  register struct char_data *i;
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected) {
      i = d->character;
      if (world[i->in_room].zone == ARENA_ZONE &&
            ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA) &&
            (i->in_room != NOWHERE)) {
        send_to_char(i,"\r\nThe floor opens, droping you in the arena.\r\n");
        char_from_room(i);
        char_to_room(i, real_room(rand_number(ARENA_START_ROOM,ARENA_END_ROOM)));
        look_at_room(i, 0);
      }
    }
  }
  do_game();
}


void do_game()
{

  if (num_in_arena() == 1) {
    in_arena = ARENA_OFF; 
    find_game_winner();
  } else if(time_left_in_game == 0) {
      in_arena = ARENA_OFF; 
      do_end_game();
  } else if (num_in_arena() == 0) {
      in_arena = ARENA_OFF;
      silent_end();
  } else if ((time_left_in_game % 5) || time_left_in_game <= 4) {
      sprintf(buf, "With %d hours left in the game there are %d players left.",
              time_left_in_game, num_in_arena());
      send_to_arena(buf);
  } else if (time_left_in_game == 1) {
      sprintf(buf, "With 1 hour left in the game there are %d players left.",
              num_in_arena());
  send_to_arena(buf);
  }
  time_left_in_game--;
}


void find_game_winner()
{
  register struct char_data *i;
  struct descriptor_data *d;
  struct hall_of_fame_element *fame_node;

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      i = d->character;
      if (world[i->in_room].zone == ARENA_ZONE &&
          ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA) && 
          (!IS_NPC(i)) && (i->in_room != NOWHERE) && 
          GET_LEVEL(i) < LVL_IMMORT) {
        GET_HIT(i) = GET_MAX_HIT(i);
        GET_MANA(i) = GET_MAX_MANA(i);
        if (i->affected)
          while (i->affected)
            affect_remove(i, i->affected);
        char_from_room(i);
        char_to_room(i, r_mortal_start_room);
        look_at_room(i, 0);
        act("$n falls from the sky.", FALSE, i, 0, 0, TO_ROOM);  
        if (game_length - time_left_in_game == 1) {
          sprintf(buf, "After 1 brief hour, %s is declared the winner.\r\n",
                  GET_NAME(i));
          send_to_arena(buf);
        } else {
            sprintf(buf,"After %d hours, %s is declared the winner.\r\n",
                    game_length - time_left_in_game, GET_NAME(i));
            send_to_arena(buf);
	}
        GET_GOLD(i) += arena_pot/2;
        sprintf(buf, "You have earned %ld coins for winning the arena.\r\n",
                (arena_pot/2));
        send_to_char(i, buf);
        sprintf(buf2, "%s has been awarded %ld coins for winning arena.\r\n", 
	        GET_NAME(i), (arena_pot/2));
        send_to_arena(buf2);
        /* debuglog(buf2, FALSE); */
        log(buf2);
        CREATE(fame_node, struct hall_of_fame_element, 1);
        strncpy(fame_node->name, GET_NAME(i), MAX_NAME_LENGTH);
        fame_node->name[MAX_NAME_LENGTH] = '\0';
        fame_node->date = time(0);
        fame_node->award = (arena_pot/2);
        fame_node->next = fame_list;
        fame_list = fame_node;
        write_fame_list();
	find_bet_winners(i);
      }
    }
}


void silent_end()
{
  in_arena = ARENA_OFF;
  start_time = 0;
  game_length = 0;
  time_to_start = 0;
  time_left_in_game = 0;
  arena_pot = 0;
  bet_pot = 0;
  sprintf(buf, "It looks like no one was brave enough to enter the Arena.");
  send_to_arena(buf);
}


void do_end_game()
{
  register struct char_data *i;
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      i = d->character;
      if (ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA) && 
         (i->in_room != NOWHERE) && (!IS_NPC(i))) {
        GET_HIT(i) = GET_MAX_HIT(i);
        GET_MANA(i) = GET_MAX_MANA(i);
	GET_MOVE(i) = GET_MAX_MOVE(i);
        if (i->affected)
          while (i->affected)
            affect_remove(i, i->affected);
        char_from_room(i);
        char_to_room(i, r_mortal_start_room);
        look_at_room(i, 0);
        act("$n falls from the sky.", FALSE, i, 0, 0, TO_ROOM);
      }
    }
    sprintf(buf, "After %d hours of battle the Match is a draw.",game_length);
    send_to_arena(buf);
    time_left_in_game = 0;
}


int num_in_arena()
{
  register struct char_data *i;
  struct descriptor_data *d;
  int num = 0;

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      i = d->character;
      if (world[i->in_room].zone == ARENA_ZONE && 
          ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA) && (i->in_room != NOWHERE)) {
        if (GET_LEVEL(i) < LVL_IMMORT)
          num++;
      }
    }
  return num;
}


ACMD(do_awho)
{
  struct descriptor_data *d;
  struct char_data *tch;
  int num =0;
  *buf2 = '\0';
  
  if (in_arena == ARENA_OFF) {
    send_to_char(ch,"There is no Arena going on right now.\r\n");
    return;
  }
  sprintf(buf,"  Gladiators in the Arena\r\n");
  sprintf(buf,"%s---------------------------------------\r\n", buf);
  sprintf(buf,"%sGame Length = %-3d   Time To Start %-3d\r\n", buf,
              game_length, time_to_start);
  sprintf(buf,"%sLevel Limits %d to %d\r\n", buf, lo_lim, hi_lim);
  sprintf(buf,"%s         Jackpot = %ld\r\n", buf, arena_pot);
  sprintf(buf,"%s---------------------------------------\r\n", buf);

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      tch = d->character;
        if (world[tch->in_room].zone == ARENA_ZONE && 
	    ROOM_FLAGGED(IN_ROOM(tch), ROOM_ARENA) &&
	    (tch->in_room != NOWHERE) && GET_LEVEL(tch)<LVL_IMMORT) {
          sprintf(buf, "%s%-20.20s%s", buf,
          GET_NAME(tch),(!(++num % 3) ? "\r\n" : ""));
	}
    }
  strcat(buf, "\r\n\r\n");
  page_string(ch->desc, buf, 1);	
}


ACMD(do_ahall)
{
  char site[MAX_INPUT_LENGTH], format[MAX_INPUT_LENGTH], *timestr;
  char format2[MAX_INPUT_LENGTH];
  struct hall_of_fame_element *fame_node;

  *buf = '\0';
  *buf2 = '\0';

  if (!fame_list) {
    send_to_char(ch,"No-one is in the Hall of Fame.\r\n");
    return;
  }

  sprintf(buf2, "%s|---------------------------------------|%s\r\n",
          CCBLU(ch, C_NRM), CCNRM(ch, C_NRM));
  sprintf(buf2, "%s%s|%sPast Winners of Arena%s                  |%s\r\n",
	  buf2, CCBLU(ch, C_NRM), CCNRM(ch, C_NRM),
    	  CCBLU(ch, C_NRM),CCNRM(ch, C_NRM));
  sprintf(buf2, "%s%s|---------------------------------------|%s\r\n\r\n",
          buf2,CCBLU(ch, C_NRM), CCNRM(ch, C_NRM));

  strcpy(format, "%-10.10s  %-15.15s  %-40s\r\n");
  sprintf(buf, format, 
	    "Date",
	    "Award Amt",
	    "Name"
	  );
  strcat(buf2, buf);
  sprintf(buf, format,
	    "---------------------------------",
	    "--------------------------------",
	    "-------------------------------------------------");
  strcat(buf2, buf);    

  strcpy(format2, "%-10.10s  %-16d %s\r\n");

  for (fame_node = fame_list; fame_node; fame_node = fame_node->next) {
    if (fame_node->date) {
      timestr = asctime(localtime(&(fame_node->date)));
      *(timestr + 10) = 0;
      strcpy(site, timestr);
    } else
        strcpy(site, "Unknown");
        sprintf(buf, format2, site, fame_node->award, CAP(fame_node->name));
        strcat(buf2, buf);
  }

  page_string(ch->desc, buf2, 1);	
  return; 
}


void load_hall_of_fame(void)
{
  FILE *fl;
  int date, award;
  char name[MAX_NAME_LENGTH + 1];
  struct hall_of_fame_element *next_node;

  fame_list = 0;

  if (!(fl = fopen(HALL_FAME_FILE, "r"))) {
    perror("Unable to open hall of fame file");
    return;
  }
  while (fscanf(fl, "%s %d %d", name, &date, &award) == 3) {
    CREATE(next_node, struct hall_of_fame_element, 1);
    strncpy(next_node->name, name, MAX_NAME_LENGTH);
    next_node->name[MAX_NAME_LENGTH] = '\0';
    next_node->date = date;
    next_node->award = award;
    next_node->next = fame_list;
    fame_list = next_node;
  }

  fclose(fl);
}


void write_fame_list(void)
{
  FILE *fl;

  if (!(fl = fopen(HALL_FAME_FILE, "w"))) {
    /* syserrlog("Error writing _hall_of_fame_list", FALSE); */
    log("Error writing _hall_of_fame_list");
    return;
  }
  write_one_fame_node(fl, fame_list);/* recursively write from end to start */
  fclose(fl);

  return;
}


void write_one_fame_node(FILE * fp, struct hall_of_fame_element * node)
{
  if (node) {
    write_one_fame_node(fp, node->next);
    fprintf(fp, "%s %ld %ld\n", node->name, (long) node->date, node->award);
  }
}


void find_bet_winners(struct char_data *winner)
{
  register struct char_data *i;
  struct descriptor_data *d;

  *buf1 = '\0';
  
  for (d = descriptor_list; d; d = d->next)
  if (!d->connected) {
    i = d->character;
    if ((!IS_NPC(i)) && (i->in_room != NOWHERE) &&
          (GET_BETTED_ON(i) == GET_IDNUM(winner)) && GET_BET_AMT(i) > 0) {
      sprintf(buf1, "You have won %d coins on your bet.\r\n",GET_BET_AMT(i)*2);
      send_to_char(i,buf1);
      GET_GOLD(i) += GET_BET_AMT(i)*2;
      GET_BETTED_ON(i) = 0;
      GET_BET_AMT(i) = 0;
    }
    else /* they bet and did't win, or didn't bet at all */
      GET_BET_AMT(i) = 0;
  }
}


int arena_ok(struct char_data * ch, struct char_data * victim)
{
  if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) && 
      ROOM_FLAGGED(victim->in_room, ROOM_ARENA))
    return TRUE;
  else
    return FALSE;
}


void arena_kill(struct char_data * ch, struct char_data * victim)
{
  if (FIGHTING(ch))
    stop_fighting(ch);

  while (ch->affected)
    affect_remove(ch, ch->affected);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);

  update_pos(ch);
 
  char_from_room(ch);
  char_to_room(ch, r_mortal_start_room);
  look_at_room(ch, 0);
  act("$n is droped from the sky.", FALSE, ch, 0, 0, TO_ROOM);
}
