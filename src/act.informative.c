
//
//   File: act.informative.c                      
//   Usage: Player-level commands of an informative nature                 //


#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "constants.h"
#include "clan.h"
#include "dg_scripts.h"
#include "warzone.h"
#include "logger.h"

// extern variables
extern int top_of_helpt;
extern struct help_index_element *help_table;
extern char *help;
extern struct time_info_data time_info;
extern HELP_DATA *help_info;
extern struct help_keywords *keyword_list;

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
//extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *class_abbrevs[];
extern const char *pc_class_types[];
extern void room_affect_text(struct char_data *ch);

char *Commatize(long num);

// nextern functions
ACMD(do_action);
ACMD(do_insult);
//void do_map( struct char_data *ch, char *argument );
bitvector_t find_class_bitvector(const char *arg);
int level_exp(int chclass, int level);
char *title_male(int chclass, int level);
char *title_female(int chclass, int level);
struct time_info_data *real_time_passed(time_t t2, time_t t1);
int compute_armor_class(struct char_data *ch);
int mobs_in_castleone();
int mobs_in_castletwo();
int check_disabled(const struct command_info *command);
int search_help(char *argument);
int file_to_string(const char *name, char *buf);
void proc_color(char *inbuf, int colour);
void show_building_cond(CHAR_DATA *ch, BUILDING_DATA *bld);

// local functions 
int sort_commands_helper(const void *a, const void *b);
void print_object_location(int num, struct obj_data *obj, struct char_data *ch, int recur);
void show_obj_to_char(struct obj_data *obj, struct char_data *ch, int mode);
void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, bool show);
void show_obj_modifiers(struct obj_data *obj, struct char_data *ch);
ACMD(do_look);
ACMD(do_examine);
ACMD(do_gold);
ACMD(do_worth);
ACMD(do_score);
ACMD(do_stats);
ACMD(do_inventory);
ACMD(do_equipment);
ACMD(do_time);
ACMD(do_weather);
ACMD(do_help);
ACMD(do_who);
ACMD(do_users);
ACMD(do_gen_ps);
void perform_mortal_where(struct char_data *ch, char *arg);
void perform_immort_where(struct char_data *ch, char *arg);
ACMD(do_where);
ACMD(do_levels);
ACMD(do_areas);
ACMD(do_consider);
ACMD(do_diagnose);
ACMD(do_color);
ACMD(do_toggle);
void sort_commands(void);
ACMD(do_commands);
void diag_char_to_char(struct char_data *i, struct char_data *ch);
void look_at_char(struct char_data *i, struct char_data *ch);
void list_one_char(struct char_data *i, struct char_data *ch);
void list_char_to_char(struct char_data *list, struct char_data *ch);
void do_auto_exits(struct char_data *ch);
ACMD(do_exits);
void char_to_dir(struct char_data *ch, room_rnum room);
void char_from_dir(struct char_data *ch);
void look_in_direction(struct char_data *ch, int dir);
void look_in_obj(struct char_data *ch, char *arg);
char *find_exdesc(char *word, struct extra_descr_data *list);
void look_at_target(struct char_data *ch, char *arg);
void printmap(struct char_data *ch);
void printwildmap(struct char_data *ch);
void get_map_char(struct char_data *ch, int room);
void space_to_minus(char *str);
void PrintWildMap(struct char_data *ch);

void room_affect_update(void);
void remove_room_affect(struct room_affect *raff, char type);

ACMD(do_news);
ACMD(do_gnews);
ACMD(do_scan);
ACMD(do_search);
ACMD(do_clear_input);
ACMD(do_playerinfo);
ACMD(do_stats);
ACMD(do_show_ore);
ACMD(do_show_gems);
ACMD(do_history);

ACMD(do_clear_input)
{

}

ACMD(do_playerinfo)
{

    if (!*argument) {
      send_to_char(ch, "A name would help.\r\n");
      return;
    }
    send_to_char(ch, "For now, just type &Gshow player <name>&n\r\n");
}

/* in-zone map functions */
bool can_see_map(struct char_data *ch);
void str_and_map(char *str, struct char_data *ch );


// local globals 
int *cmd_sort_info;

// For show_obj_to_char 'mode'.	/-- arbitrary 
#define SHOW_OBJ_LONG		0
#define SHOW_OBJ_SHORT		1
#define SHOW_OBJ_ACTION		2


void show_obj_to_char(struct obj_data *obj, struct char_data *ch, int mode)
{
  char *icol = "&n", *cond = "&n";

  if (!obj || !ch) {
    log("SYSERR: NULL pointer in show_obj_to_char(): obj=%p ch=%p", obj, ch);
    return;
  }

  if (OBJ_FLAGGED(obj, ITEM_NO_DAMAGE))   
     icol="&C";
  else if (GET_OBJ_COND(obj) < 1000)
     icol="&G";
  else if (GET_OBJ_COND(obj) < 2000)
     icol="&Y";
  else if (GET_OBJ_COND(obj) < 3000)
     icol="&y";
  else if (GET_OBJ_COND(obj) < 4000)
     icol="&R";
  else if (GET_OBJ_COND(obj) > 4999)
     icol="&r";

  if (OBJ_FLAGGED(obj, ITEM_NO_DAMAGE))
     cond="&C   impervious";
  else if (GET_OBJ_COND(obj) == 0)
     cond="&G      perfect";
  else if (GET_OBJ_COND(obj)  < 500)
     cond="&G    scratches";
  else if (GET_OBJ_COND(obj)  < 1000)
     cond="&G  small dings";
  else if (GET_OBJ_COND(obj)  < 1500)
     cond="&Y    big dents";
  else if (GET_OBJ_COND(obj)  < 2000)
     cond="&Y  major dents";
  else if (GET_OBJ_COND(obj)  < 2500)
     cond="&Ymassive chips";
  else if (GET_OBJ_COND(obj)  < 3000)
     cond="&y    worn down";
  else if (GET_OBJ_COND(obj)  < 3500)
     cond="&y    neglected";
  else if (GET_OBJ_COND(obj)  < 4000)
     cond="&R needs repair";
  else if (GET_OBJ_COND(obj)  < 4500)
     cond="&R    dire need";
  else if (GET_OBJ_COND(obj)  < 4750)
     cond="&Ralmost ruined";
  else if (GET_OBJ_COND(obj) > 4999)
     cond="&r       ruined";


  switch (mode) {
  case SHOW_OBJ_LONG:
    if (GET_OBJ_LEVEL(obj) > GET_LEVEL(ch))
      send_to_char(ch, "&RX %s%s&n", icol,  obj->description);
    else if (IS_WARRIOR(ch) && OBJ_FLAGGED(obj, ITEM_ANTI_WARRIOR))
      send_to_char(ch, "&RX %s%s&n", icol,  obj->description);
    else if (IS_THIEF(ch) && OBJ_FLAGGED(obj, ITEM_ANTI_THIEF))
      send_to_char(ch, "&RX %s%s&n", icol,  obj->description);
    else if (IS_MAGIC_USER(ch) && OBJ_FLAGGED(obj, ITEM_ANTI_MAGIC_USER))
      send_to_char(ch, "&RX %s%s&n", icol,  obj->description);
    else if (IS_CLERIC(ch) && OBJ_FLAGGED(obj, ITEM_ANTI_CLERIC))
      send_to_char(ch, "&RX %s%s&n", icol,  obj->description);  
    else
      send_to_char(ch, "%s%s&n",icol,  obj->description);
    break;

  case SHOW_OBJ_SHORT:
    if (GET_OBJ_LEVEL(obj) > GET_LEVEL(ch))
      send_to_char(ch, "&RX &W[%s&W] %s%s&n", cond, icol,  obj->short_description);
    else
      send_to_char(ch, "&W[%s&W] %s%s&n", cond, icol,  obj->short_description);
    break;

  case SHOW_OBJ_ACTION:
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_NOTE:
      if (obj->action_description) {
        char notebuf[MAX_NOTE_LENGTH + 64];

        snprintf(notebuf, sizeof(notebuf), "There is something written on it:\r\n\r\n%s", obj->action_description);
        page_string(ch->desc, notebuf, TRUE);
      } else
	send_to_char(ch, "&WIt's blank.&n\r\n");
      return;

    case ITEM_DRINKCON:
      send_to_char(ch, "It looks like a drink container.");
      break;

    default:
      send_to_char(ch, "You see nothing special..");
      break;
    }
    break;

  default:
    log("SYSERR: Bad display mode (%d) in show_obj_to_char().", mode);
    return;
  }
  show_obj_modifiers(obj, ch);
  send_to_char(ch, "\r\n");
}


void show_obj_modifiers(struct obj_data *obj, struct char_data *ch)
{
  if (OBJ_FLAGGED(obj, ITEM_INVISIBLE))
    send_to_char(ch, "&W (invisible)&n");

  if (OBJ_FLAGGED(obj, ITEM_BLESS) && AFF_FLAGGED(ch, AFF_DETECT_ALIGN))
    send_to_char(ch, "&B ...It's BLUE!&n");

  if (OBJ_FLAGGED(obj, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
    send_to_char(ch, "&Y ..It's Yellow!&n");


}

void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, bool show)
{
  struct obj_data *i, *j;
  bool found;
  int num;
// this function adds in obj stacking so instead of:
//   * an arrow
//   * an arrow
//   * you get this:
//   * (2) an arrow

  found = FALSE;
  for (i = list; i; i = i->next_content) {
	  num = 0;
	  for (j = list; j != i; j = j->next_content)
  if (j->item_number==NOTHING) {    /*    log("short desc"); */
	 if(strcmp(j->short_description,i->short_description)==0) break;
  }
  else if (j->item_number==i->item_number) break;
		if (j!=i) continue;
		for (j = i; j; j = j->next_content)
  if (j->item_number==NOTHING) {   /*     log("short desc2"); */
	 if(strcmp(j->short_description,i->short_description)==0) num++;
  }
  else if (j->item_number==i->item_number) num++;
		if (CAN_SEE_OBJ(ch, i)) {
	if (num!=1) {
  send_to_char(ch,"&w(%2i) ",num);
	}
  send_to_char(ch, "&w");
  show_obj_to_char(i, ch, mode);
  send_to_char(ch, "&n");
  found = TRUE;
	}
  }
  if (!found && show)
	send_to_char(ch, " Nothing.\r\n");
}

void diag_char_to_char(struct char_data *i, struct char_data *ch)
{
  struct {
    byte percent;
    const char *text;
  } diagnosis[] = {
    { 100, "&Wis in excellent condition.&n"			},
    {  90, "&Yhas a few scratches.&n"			},
    {  75, "&Ghas some small wounds and bruises.&n"		},
    {  50, "&Chas quite a few wounds.&n"			},
    {  30, "&Mhas some big nasty wounds and scratches.&n"	},
    {  15, "&Mlooks pretty hurt.&n"				},
    {   0, "&Ris in awful condition.&n"			},
    {  -1, "&Ris bleeding awfully from big wounds.&n"	},
  };
  int percent, ar_index;
  const char *pers = PERS(i, ch);

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;		// How could MAX_HIT be < 1?? 

  for (ar_index = 0; diagnosis[ar_index].percent >= 0; ar_index++)
    if (percent >= diagnosis[ar_index].percent)
      break;

  send_to_char(ch, "&Y%c%s %s&n\r\n", UPPER(*pers), pers + 1, diagnosis[ar_index].text);
}


void look_at_char(struct char_data *i, struct char_data *ch)
{
  int j, found;
  char buf2[MAX_INPUT_LENGTH];

  if (!ch->desc)
    return;

   if (i->player.description)
    send_to_char(ch, "&M%s&n",  i->player.description);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

   if (RIDING(i) && RIDING(i)->in_room == i->in_room) {
     if (RIDING(i) == ch)
       act("$e is mounted on you.", FALSE, i, 0, ch, TO_VICT);
     else {
       sprintf(buf2, "$e is mounted upon %s.", PERS(RIDING(i), ch));
       act(buf2, FALSE, i, 0, ch, TO_VICT);
     }
   } else if (RIDDEN_BY(i) && RIDDEN_BY(i)->in_room == i->in_room) {
     if (RIDDEN_BY(i) == ch)
       act("You are mounted upon $m.", FALSE, i, 0, ch, TO_VICT);
     else {
       sprintf(buf2, "$e is mounted by %s.", PERS(RIDDEN_BY(i), ch));
       act(buf2, FALSE, i, 0, ch, TO_VICT);
     }
   }

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found) {
    send_to_char(ch, "\r\n");	// act() does capitalization. 
    send_to_char(ch, "&G");
    act("$n is using:", FALSE, i, 0, ch, TO_VICT);
    
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
	send_to_char(ch, "&M%s&n", wear_where[j]);
	show_obj_to_char(GET_EQ(i, j), ch, SHOW_OBJ_SHORT);
      }
  }
  if (ch != i && (IS_THIEF(ch) || GET_LEVEL(ch) > LVL_IMMORT)) {
    found = FALSE;
    act("\r\n&GYou attempt to peek at $s inventory:&n", FALSE, i, 0, ch, TO_VICT);
//    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if ((rand_number(0, 40) < GET_LEVEL(ch))) {
	list_obj_to_char(i->carrying, ch, SHOW_OBJ_SHORT, TRUE);
	found = TRUE;
//      }
    }

    if (!found)
      send_to_char(ch, "&GYou can't see anything.&n\r\n");
  }
}

const char *blood_msg[] = {
  "Your hands are clean.",
  "Your hands have a little blood left under the fingernails.",
  "Your hands have a few drops of blood on them",
  "Your hands have quite a bit of blood on them.",
  "Your hands are dripping with blood.",
  "Your hands are stained crimson red from all the deaths you have caused."
};

void list_one_char(struct char_data *i, struct char_data *ch)
{
  const char *positions[] = {
    " is lying here, dead.",
    " is lying here, mortally wounded.",
    " is lying here, incapacitated.",
    " is lying here, stunned.",
    " is sleeping here.",
    " is resting here.",
    " is sitting here.",
    "!FIGHTING!",
    " is standing here."
  };
  
  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
    if (AFF_FLAGGED(i, AFF_INVISIBLE))
      send_to_char(ch, "&G*&n");
  
    if (!IS_NPC(i) && GET_WAS_CLERIC(i) > 0 && GET_WAS_MAGE(i) > 0 && GET_WAS_WARRIOR(i) > 0 && GET_WAS_THIEF(i) > 0 && GET_LEVEL(i) == LVL_IMMORT)
       send_to_char(ch, "&W (MASTER)");
    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
      if (IS_EVIL(i))
	send_to_char(ch, "&R(Red Aura) &n");
      else if (IS_GOOD(i))
	send_to_char(ch, "&B(Blue Aura) &n");
    }
    send_to_char(ch, "%s%s%s", BGRN, i->player.long_descr, CNRM);

    if (AFF_FLAGGED(i, AFF_SANCTUARY))
      act("...$e is surrounded by a brilliant white aura!", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_BLIND))
      act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_SPARKSHIELD))
      act("&Y...&e is surrounded by a sparking field of energy!", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_MANASHIELD))
      act("&B...&e is surrounded by a magical blue shield!", FALSE, i, 0, ch, TO_VICT);

    return;
  }

  if (IS_NPC(i))
  {
    send_to_char(ch, "%c%s", UPPER(*i->player.short_descr), i->player.short_descr + 1);
  }
  else
  {
    if (PLR_FLAGGED(i, PLR_OUTLAW))
      send_to_char(ch, "&W(&ROUTLAW&W)&n");
//    if (PLR_FLAGGED(i, PLR_PRECEPTOR) && !PLR_FLAGGED(i, PLR_OUTLAW))
//      send_to_char(ch, "&B(&YPRECEPTOR&B)&n ");
    if (GET_LEVEL(i) >= LVL_GOD)
      send_to_char(ch, "&R(&WGOD&R)&n");
    if (GET_LEVEL(i) == LVL_IMMORT && (GET_WAS_CLERIC(i) + GET_WAS_MAGE(i) + GET_WAS_WARRIOR(i) + GET_WAS_THIEF(i)) > 3)
      send_to_char(ch, "&W(MASTER)&n "); 
    send_to_char(ch, "&W%s&G %s", i->player.name, GET_TITLE(i));
  }
 

  if (AFF_FLAGGED(i, AFF_INVISIBLE))
    send_to_char(ch, " (invisible)");
  if (AFF_FLAGGED(i, AFF_HIDE))
    send_to_char(ch, " (hidden)");
  if (!IS_NPC(i) && !i->desc)
    send_to_char(ch, " (linkless)");
  if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_WRITING))
    send_to_char(ch, " (writing)");

   if (RIDING(i) && RIDING(i)->in_room == i->in_room) {
     send_to_char(ch, " is here, mounted upon ");
     if (RIDING(i) == ch)
       send_to_char(ch, "you");
     else
       send_to_char(ch, "%s", PERS(RIDING(i), ch));
     send_to_char(ch, ".");
   } else if (GET_POS(i) != POS_FIGHTING)
    send_to_char(ch, "%s%s%s", BGRN, positions[(int) GET_POS(i)], CNRM);
  else {
    if (FIGHTING(i)) {
      send_to_char(ch, "&G is here, fighting &n");
      if (FIGHTING(i) == ch)
	send_to_char(ch, "&GYOU!&n");
      else {
	if (IN_ROOM(i) == IN_ROOM(FIGHTING(i)))
	  send_to_char(ch, "%s!", PERS(FIGHTING(i), ch));
	else
	  send_to_char(ch,  "&Gsomeone who has already left!&n");
      }
    } else			// NIL fighting pointer 
      send_to_char(ch, " &Gis here struggling with thin air.&n");
  }
  
   if (!IS_NPC(i) && PRF_FLAGGED(i, PRF_AFK))
       send_to_char(ch, "&C(AFK)&n"); 

   if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_TAGGED))
       send_to_char(ch, "&M(IT)&n");

   if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) 
   {
     if (IS_EVIL(i))
       send_to_char(ch, "&R (Red Aura)&n");
     else if (IS_GOOD(i))
       send_to_char(ch, "&B (Blue Aura)&n");
   }
   
   if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_PRECEPTOR))
       send_to_char(ch, " &B(&YPRECEPTOR&B)&n");
  
  send_to_char(ch, "&n\r\n");

  if (AFF_FLAGGED(i, AFF_SANCTUARY))
    act(" &W...$e surrounded by a briliant white aura!&n", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_SPARKSHIELD))
    act(" &Y...&Y$e&Y is surrounded by a sparking shield of energy.&n", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_MANASHIELD))
    act(" &B...$e is surrounded by a magical blue shield!&n", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_VENGEANCE))
    act(" &w...$e is surrounded by a shimering silvery aura!&n", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_DIVINE_PROT))
    act(" &w...$e is surrounded by a glittering white aura!&n", FALSE, i, 0, ch, TO_VICT);

}



void list_char_to_char(struct char_data *list, struct char_data *ch)
{
  struct char_data *i;

  for (i = list; i; i = i->next_in_room)
    if (ch != i) {
       if (RIDDEN_BY(i) && RIDDEN_BY(i)->in_room == i->in_room)
         continue;
      if (CAN_SEE(ch, i))
	list_one_char(i, ch);
      else if (IS_DARK(IN_ROOM(ch)) && !CAN_SEE_IN_DARK(ch) &&
	       AFF_FLAGGED(i, AFF_INFRAVISION))
	send_to_char(ch, "%sYou see a pair of glowing red eyes looking your way.%s\r\n", BGRN, CNRM);
    }
}


void do_auto_exits(struct char_data *ch)
{
  long int door, slen = 0, hids = 0;

  send_to_char(ch, "&BExits:\r\n");

for (door = 0; door < NUM_OF_DIRS; door++) 
{
    if (!EXIT(ch, door) || EXIT(ch, door)->to_room == NOWHERE || EXIT_FLAGGED(EXIT(ch, door), EX_HIDDEN))
      {
      hids++;
      continue;
      }
    if (EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED))
    {
      if (GET_LEVEL(ch) >= LVL_GOD)
      send_to_char(ch, "%c%-4s - [%5d] Closed.\r\n", UPPER(*dirs[door]), dirs[door] + 1, GET_ROOM_VNUM(EXIT(ch, door)->to_room));
       else
      send_to_char(ch, "%c%-4s - %s.\r\n", UPPER(*dirs[door]), dirs[door] + 1, IS_DARK(EXIT(ch,
door)->to_room) &&
                !CAN_SEE_IN_DARK(ch) ? "Too dark to tell." : "Closed");
      slen++;
      continue;
    }
    slen++;

    if (GET_LEVEL(ch) >= LVL_GOD)
      send_to_char(ch, "%c%-4s - [%5d] %s\r\n", UPPER(*dirs[door]), dirs[door] + 1, GET_ROOM_VNUM(EXIT(ch, door)->to_room),
                world[EXIT(ch, door)->to_room].name);
    else
      send_to_char(ch, "%c%-4s - %s\r\n", UPPER(*dirs[door]), dirs[door] +1, IS_DARK(EXIT(ch, door)->to_room) &&
                !CAN_SEE_IN_DARK(ch) ? "Too dark to tell." : world[EXIT(ch, door)->to_room].name);
}

  if (!slen && hids > 0)
   send_to_char(ch, "%s &n\r\n", slen ? "" : "None!");



}


ACMD(do_exits)
{
  long int door, len = 0, hids  = 0;

  if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char(ch, "&RYou can't see a damned thing, you're blind!&n\r\n");
    return;
  }

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in teh Crystal Gate.\r\n");
    return;
  }

  send_to_char(ch, "&BObvious exits:\r\n");

  for (door = 0; door < NUM_OF_DIRS; door++) 
  {

    if (!EXIT(ch, door) || EXIT(ch, door)->to_room == NOWHERE || EXIT_FLAGGED(EXIT(ch, door), EX_HIDDEN))
      {
      hids++;
      continue;
      }
    if (EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED))
    {
       if (GET_LEVEL(ch) >= LVL_GOD)
      send_to_char(ch, "%c%-4s - [%5d] Closed.\r\n", UPPER(*dirs[door]), dirs[door] + 1, GET_ROOM_VNUM(EXIT(ch, door)->to_room));
       else
      send_to_char(ch, "%c%-4s - %s.\r\n", UPPER(*dirs[door]), dirs[door] + 1, IS_DARK(EXIT(ch, door)->to_room) &&
                !CAN_SEE_IN_DARK(ch) ? "Too dark to tell." : "Closed");
       len++;
       continue;
    }
    
    len++;

    if (GET_LEVEL(ch) > LVL_IMMORT)
      send_to_char(ch, "%c%-4s - [%5d] %s\r\n", UPPER(*dirs[door]), dirs[door] + 1, GET_ROOM_VNUM(EXIT(ch, door)->to_room),
		world[EXIT(ch, door)->to_room].name);
    else
      send_to_char(ch, "%c%-4s - %s\r\n", UPPER(*dirs[door]), dirs[door] + 1, IS_DARK(EXIT(ch, door)->to_room) &&
		!CAN_SEE_IN_DARK(ch) ? "Too dark to tell." : world[EXIT(ch, door)->to_room].name);
  }

  if (!len && hids > 0)
    send_to_char(ch, " None.\r\n");
 
}



void look_at_room2(struct char_data *ch, int ignore_brief)
{
//  int zone, mobs_left=0;
//  struct room_data *rm = &world[IN_ROOM(ch)];

  if (!ch->desc)
    return;
/*
  if (PLR_FLAGGED(ch, PLR_TAKE_SMOKE))
  {
     mobs_left = mobs_in_castleone();
     send_to_char(ch, "&RMobs left to kill: %d&n\r\n", mobs_left);
  }
  
  if (PLR_FLAGGED(ch, PLR_TAKE_BLAZE))
  {
     mobs_left = mobs_in_castletwo();
     send_to_char(ch, "&RMobs left to kill: %d&n\r\n", mobs_left);
  }

  if (PLR_FLAGGED(ch, PLR_TAKE_CANN))
  {
     mobs_left = mobs_in_castlethree();
     send_to_char(ch, "&RMobs left to kill: %d&n\r\n", mobs_left);
  }
  
  if (PLR_FLAGGED(ch, PLR_TAKE_SATI))
  {
     mobs_left = mobs_in_castlefour();
     send_to_char(ch, "&RMobs left to kill: %d&n\r\n", mobs_left);
  }
*/
  if (IS_DARK(IN_ROOM(ch)) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char(ch, "&BIt is pitch black...&n\r\n");
    return;
  } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char(ch, "&BYou see nothing but infinite darkness...&n\r\n");
    return;
  }
  
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS))
  {
    if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief)
        PrintWildMap(ch);

     room_affect_text(ch);
     list_building_to_char(world[ch->in_room].buildings, ch);
     list_ship_to_char(world[ch->in_room].ships, ch);
     list_vehicle_to_char(world[ch->in_room].vehicles, ch);
     list_obj_to_char(world[IN_ROOM(ch)].contents, ch, SHOW_OBJ_LONG, FALSE);
     list_char_to_char(world[IN_ROOM(ch)].people, ch);
     return;
  }
 
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {

    sprintbitarray(ROOM_FLAGS(IN_ROOM(ch)), room_bits, RF_ARRAY_MAX, buf);
    send_to_char(ch, "&M%s\r\n&m[ %s]\r\n", world[IN_ROOM(ch)].name, buf);
  } else
    send_to_char(ch, "&M%s&n\r\n", world[IN_ROOM(ch)].name);

 if ((!IS_NPC(ch) && (!PRF_FLAGGED(ch, PRF_BRIEF))) || ignore_brief || ROOM_FLAGGED(IN_ROOM(ch), ROOM_DEATH))
  {
//      if(!IS_NPC(ch) && PRF2_FLAGGED(ch, PRF2_AUTOMAP) && can_see_map(ch))
//      {
//        str_and_map(world[IN_ROOM(ch)].description, ch);
//      }
//      else
//      {
        send_to_char(ch, "&c%s&n", world[IN_ROOM(ch)].description);
//      }
  }

  if ( world[ch->in_room].portal_stone )
      ch_printf(ch, "%s\r\n", world[ch->in_room].portal_stone->description);

  if (world[ch->in_room].ferryboat)
     send_to_char(ch, "&b&4You see a ferryboat. Go UP to embark.&0\r\n");

  room_affect_text(ch);
  /* list rooms contents */
  list_building_to_char(world[ch->in_room].buildings, ch);
  list_ship_to_char(world[ch->in_room].ships, ch);
  list_vehicle_to_char(world[ch->in_room].vehicles, ch);

  // now list characters & objects
  list_obj_to_char(world[IN_ROOM(ch)].contents, ch, SHOW_OBJ_LONG, FALSE);
  list_char_to_char(world[IN_ROOM(ch)].people, ch);

  //  autoexits 
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOEXIT))
    do_auto_exits(ch);

}


void look_at_room(struct char_data *ch, int ignore_brief)
{
  if (!ch->desc)
    return;

  if (IS_DARK(IN_ROOM(ch)) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char(ch, "&BIt is pitch black...&n\r\n");
    return;
  } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char(ch, "&BYou see nothing but infinite darkness...&n\r\n");
    return;
  }

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
   send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
   return;
  }


  send_to_char(ch, "%s", CCYEL(ch, C_NRM));
  send_to_char(ch, "&M%s&n\r\n", world[IN_ROOM(ch)].name);


  if (!IS_NPC(ch) && (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief || ROOM_FLAGGED(IN_ROOM(ch), ROOM_DEATH)))
  {
        send_to_char(ch, "&c%s&n", world[IN_ROOM(ch)].description);
  }  
  
  room_affect_text(ch);
  
  // now list characters & objects 
  /* list rooms contents */
  list_building_to_char(world[ch->in_room].buildings, ch);
  list_ship_to_char(world[ch->in_room].ships, ch);
  list_vehicle_to_char(world[ch->in_room].vehicles, ch);

  send_to_char(ch, "%s", CCCYN(ch, C_NRM));
  list_obj_to_char(world[IN_ROOM(ch)].contents, ch, SHOW_OBJ_LONG, FALSE);
  send_to_char(ch, "%s", CCGRN(ch, C_NRM));
  list_char_to_char(world[IN_ROOM(ch)].people, ch);
  send_to_char(ch, "&n");
//  autoexits 
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOEXIT))
    do_auto_exits(ch);

}

void char_to_dir(struct char_data *ch, room_rnum room)
{
	IN_ROOM(ch) = (long int)world[room].people;
	world[room].people = ch;
	IN_ROOM(ch) = room;
}

void char_from_dir(struct char_data *ch)
{
	struct char_data *temp;

	if (FIGHTING(ch) !=NULL)
		stop_fighting(ch);

	REMOVE_FROM_LIST(ch, world[ch->in_room].people, next_in_room);
	IN_ROOM(ch) = NOWHERE;
	ch->next_in_room = NULL;
}
//  this might need to be uncommented, im not sure, but commenting it out
//  got rid of the compiling errors
//
//
void look_in_direction(struct char_data *ch, int dir)
{
  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }   

  if (EXIT(ch, dir)) {
    if (EXIT(ch, dir)->general_description)
      send_to_char(ch, "%s", EXIT(ch, dir)->general_description);
    else
      send_to_char(ch, "You see nothing special.\r\n");

    if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) && EXIT(ch, dir)->keyword)
      send_to_char(ch, "&rThe %s is closed.&n\r\n", fname(EXIT(ch, dir)->keyword));
    else if (EXIT_FLAGGED(EXIT(ch, dir), EX_ISDOOR) && EXIT(ch, dir)->keyword)
      send_to_char(ch, "&GThe %s is open.&n\r\n", fname(EXIT(ch, dir)->keyword));
  } else
    send_to_char(ch, "Nothing special there...\r\n");
}

/*
void look_in_direction(struct char_data *ch, int dir)
{
  int was_in;

  if (EXIT(ch, dir)) {
	if (!EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED)) {
	   was_in = ch->in_room;
	   char_from_dir(ch);
	   char_to_dir(ch, world[was_in].dir_option[dir]->to_room);
		
	  
	   look_at_room(ch, 0);

	   char_from_dir(ch);    
	   char_to_dir(ch, was_in);

	   send_to_char(ch, " ");

	}    
	if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED)) 
	  send_to_char(ch, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
	}
	else
	send_to_char(ch, "Nothing special there...\r\n");
}
*/


void look_in_obj(struct char_data *ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }


  if (!*arg)
    send_to_char(ch, "&RLook in what?&n\r\n");
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    send_to_char(ch, "&RThere doesn't seem to be %s %s here.&n\r\n", AN(arg), arg);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    send_to_char(ch, "&RThere's nothing inside that!&n\r\n");
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
      if (OBJVAL_FLAGGED(obj, CONT_CLOSED))
	send_to_char(ch, "&YIt is closed.&n\r\n");
      else {
	send_to_char(ch, "%s%s%s", 
                     CCGRN(ch, C_NRM),  fname(obj->name), CCNRM(ch, C_NRM));
	switch (bits) {
	case FIND_OBJ_INV:
	  send_to_char(ch, " (carried): \r\n");
	  break;
	case FIND_OBJ_ROOM:
	  send_to_char(ch, " (here): \r\n");
	  break;
	case FIND_OBJ_EQUIP:
	  send_to_char(ch, " (used): \r\n");
	  break;
	}

	list_obj_to_char(obj->contains, ch, SHOW_OBJ_SHORT, TRUE);
      }
    } else {		// item must be a fountain or drink container 
      if (GET_OBJ_VAL(obj, 1) <= 0)
	send_to_char(ch, "It is empty.\r\n");
      else {
	if (GET_OBJ_VAL(obj,0) <= 0 || GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0)) {
	  send_to_char(ch, "Its contents seem somewhat murky.\r\n"); // BUG 
	} else {
	  amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
	  sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2, sizeof(buf2));
	  send_to_char(ch, "It's %sfull of a %s liquid.\r\n", fullness[amt], buf2);
	}
      }
    }
  }
}



char *find_exdesc(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      return (i->description);

  return (NULL);
}


//
// Given the argument "look at <target>", figure out what object or char
// matches the target.  First, see if there is another char in the room
// with the name.  Then check local objs for exdescs.
//
// Thanks to Angus Mezick <angus@EDGIL.CCMAIL.COMPUSERVE.COM> for the
//  suggested fix to this problem.
//
void look_at_target(struct char_data *ch, char *arg)
{
  int bits, found = FALSE, j, fnum, i = 0;
  struct char_data *found_char = NULL;
  struct obj_data *obj, *found_obj = NULL;
  char *desc;

  if (!ch->desc)
    return;

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }


  if (!*arg) {
    send_to_char(ch, "&RLook at what?&n\r\n");
    return;
  }

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

        if (!bits)
        {
                BUILDING_DATA *bld = find_building_in_room_by_name(&world[ch->in_room], str_dup(arg));
                SHIP_DATA *ship = find_ship_in_room_by_name(&world[ch->in_room], str_dup(arg));
                VEHICLE_DATA *pVeh = find_vehicle_in_room_by_name(ch, str_dup(arg));

                if (bld)
                {
                        if (BUILDING_FLAGGED(bld, BLD_F_RUIN))
                                ch_printf(ch, "It's what remains of %s %s.\r\n",
                                        AN(bld->type->name), bld->type->name);
                        else
                                show_building_cond(ch, bld);
                        return;
                }

                if ( ship )
                {
                        ch_printf( ch, "'%s' is a %s ship.\r\n", ship->name, ship->type->name );
                        return;
                }

                if (pVeh)
                {
                        if (look_at_vehicle(ch, pVeh))
                                return;
                }

                if (world[ch->in_room].ferryboat && isname(arg, "ferryboats"))
                {
                        ch_printf(ch,
                                "It's a simple ferryboat that take people and goods across the river.\r\n"
                                "It will depart in %d minute%s.\r\n",
                                world[ch->in_room].ferryboat->timer,
                                (world[ch->in_room].ferryboat->timer > 1 ? "s" : ""));
                        return;
                }
        }

  // Is the target a character? 
  if (found_char != NULL) {
    look_at_char(found_char, ch);
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch) || IS_GOD(ch))
	act("&Y$n looks at you.&n", TRUE, ch, 0, found_char, TO_VICT);
      act("&Y$n looks at $N.&n", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }

  // Strip off "number." from 2.foo and friends. 
  if (!(fnum = get_number(&arg))) {
    send_to_char(ch, "&RLook at what?&n\r\n");
    return;
  }

  // Does the argument match an extra desc in the room?
  if ((desc = find_exdesc(arg, world[IN_ROOM(ch)].ex_description)) != NULL && ++i == fnum) {
    page_string(ch->desc, desc, FALSE);
    return;
  }

  // Does the argument match an extra desc in the char's equipment? 
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL && ++i == fnum) {
	send_to_char(ch, "&M%s&n", desc);
	found = TRUE;
      }

  // Does the argument match an extra desc in the char's inventory? 
  for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum) {
	send_to_char(ch, "&G%s&n", desc);
	found = TRUE;
      }
  }

  // Does the argument match an extra desc of an object in the room? 
  for (obj = world[IN_ROOM(ch)].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum) {
	send_to_char(ch, "&C%s&n", desc);
	found = TRUE;
      }

  // If an object was found back in generic_find 
  if (bits) {
    if (!found)
      show_obj_to_char(found_obj, ch, SHOW_OBJ_ACTION);
    else {
      show_obj_modifiers(found_obj, ch);
      send_to_char(ch, "\r\n");
    }
  } else if (!found)
    send_to_char(ch, "You do not see that here.\r\n");
}


ACMD(do_look)
{
  int look_type;

  if (!ch->desc)
    return;


  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char(ch, "&WYou can't see anything but stars!&n\r\n");
  else if  (PLR_FLAGGED(ch, PLR_INCRYGATE))
     send_to_char(ch, "You do not have control over your bodily functions while in the Crystal Gate.\r\n");
  else if (AFF_FLAGGED(ch, AFF_BLIND))
    send_to_char(ch, "You can't see shit yo, you blind!\r\n");
  else if (IS_DARK(IN_ROOM(ch)) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char(ch, "It is pitch black...\r\n");
    list_char_to_char(world[IN_ROOM(ch)].people, ch);	// glowing red eyes 
  } else {
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
	send_to_char(ch, "Read what?\r\n");
      else
	look_at_target(ch, arg);
      return;
    }
    if (!*arg)
    {
      // "look" alone, without an argument at all 
      look_at_room(ch, 1);
    }
    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    // did the char type 'look <direction>?' //
    else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (!*arg && (look_type = search_block(arg2, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
    else if (!*arg && *arg2)
      look_at_target(ch, arg2);
    else if (is_abbrev(arg, "around") && ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS))
      PrintWildMap(ch);
    else
      look_at_target(ch, arg);
  }
}



ACMD(do_examine)
{
  struct char_data *tmp_char;
  struct obj_data *tmp_object;
  char tempsave[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }
 
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "&RExamine what?&n\r\n");
    return;
  }

  /* look_at_target() eats the number. */
  look_at_target(ch, strcpy(tempsave, arg));	/* strcpy: OK */

  generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char(ch, "&YWhen you look inside, you see:&n\r\n");
      look_in_obj(ch, arg);
    }
  }
}


ACMD(do_worth)
{
  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;          
  }  

  if ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) == 0)
    send_to_char(ch, "&CYou're broke!&n\r\n");
  else if ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) == 1)
    send_to_char(ch, "&CYou have one miserable little gold coin.\r\n");
  else {
    send_to_char(ch, "&MCurrent Worth&n\r\n"
                     "&m------------------------\r\n"
                     "&CCarried &W: &Y%s&n\r\n"
                     "&CIn Bank &W: &Y%s&n\r\n"
                     "&m------------------------\r\n"
                     "&CTotal   &W: &Y%s&n\r\n", 
                     Commatize(GET_GOLD(ch)), Commatize(GET_BANK_GOLD(ch)), Commatize((GET_GOLD(ch) + GET_BANK_GOLD(ch))));
  }
}

ACMD(do_gold)
{
  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }

  if ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) == 0)
    send_to_char(ch, "&CYou're broke!&n\r\n");
  else if ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) == 1)
    send_to_char(ch, "&CYou have one miserable little gold coin.\r\n");
  else
    send_to_char(ch, "&CYou have &Y%ld &Cgold coins on you and &Y%ld &Ccoins in the bank.&n\r\n", GET_GOLD(ch), GET_BANK_GOLD(ch));
}

ACMD(do_stats)      
{  

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }
                     
  if (IS_NPC(ch))                    
    return;                 

   send_to_char(ch, "\r\n&RSTATS&W-> &BHP&W: &G%ld&g/&G%ld  &BMN&W: &G%ld&g/&G%ld  &BMV&W: &G%ld&g/&G%ld  &BAL&W: &G%-4d  &BHR&W: &G%ld &BDR&W: &G%ld &BAC&W: &G%-4d  &BSS&W: &G%-2ld&n\r\n",
                GET_HIT(ch), GET_MAX_HIT(ch), GET_MANA(ch), GET_MAX_MANA(ch), GET_MOVE(ch), GET_MAX_MOVE(ch),
                GET_ALIGNMENT(ch), (GET_HITROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].tohit),
                (GET_DAMROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].todam), compute_armor_class(ch),
                GET_SAVE(ch, SAVING_SPELL));
}


ACMD(do_score)
{
  struct time_info_data playing_time;
  struct affected_type *aff;
  
    if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }

  if (IS_NPC(ch))
    return;
  
  send_to_char(ch, "&B%s %s&n\r\n", GET_NAME(ch), GET_TITLE(ch));
  send_to_char(ch, "&BLevel:&n %d   &G%s&n\r\n", GET_LEVEL(ch), pc_class_types[(int)GET_CLASS(ch)]);
  
  if (age(ch)->month == 0 && age(ch)->day == 0)
    send_to_char(ch, "It's your birthday today.\r\n");

send_to_char(ch,
"&BHit Points:&n  %ld&B(&n%ld&B)     Mana:&n  %ld&B(&n%ld&B)     Movement:&n  %ld&B(&n%ld&B)&n\r\n"
"&W----------------------------------------------------------------------&n\r\n"

"&BStr:&n %-2d          &Bvs. Spell :&n %-2ld          &BHitroll:&n %ld\r\n"
"&BInt:&n %-2d          &Bvs. Rod   :&n %-2ld          &BDamroll:&n %ld\r\n"
"&BWis:&n %-2d          &Bvs. Para  :&n %-2ld          &BAC     :&n %d\r\n"
"&BDex:&n %-2d          &Bvs. Breath:&n %-2ld          &BAlign  :&n %d\r\n"
"&BCon:&n %-2d          &Bvs. Petri :&n %-2ld          &BAge    :&n %ld\r\n"
"&BCha:&n %-2d          &BMob Kills :&n %-6d      &BDeaths :&n %d\r\n"
"&RPKs:&n %-5d       &WSouls     : &n%-6d      &YGold   :&n %ld\r\n"
"&W----------------------------------------------------------------------&n\r\n",
              GET_HIT(ch), GET_MAX_HIT(ch), GET_MANA(ch), 
              GET_MAX_MANA(ch), GET_MOVE(ch), GET_MAX_MOVE(ch),
              GET_STR(ch), GET_SAVE(ch, SAVING_SPELL), 
              (GET_HITROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].tohit), 
              GET_INT(ch), GET_SAVE(ch, SAVING_ROD), 
              (GET_DAMROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].todam),
              GET_WIS(ch), GET_SAVE(ch, SAVING_PARA), compute_armor_class(ch),
              GET_DEX(ch), GET_SAVE(ch, SAVING_BREATH), GET_ALIGNMENT(ch),
              GET_CON(ch), GET_SAVE(ch, SAVING_PETRI), GET_AGE(ch),
              GET_CHA(ch), GET_MONSTER_KILLS(ch), GET_DEATHS(ch),
              GET_PLAYER_KILLS(ch), GET_SOULS(ch), GET_GOLD(ch));

send_to_char(ch, "&CCarrying Weight: %d of %ld\r\n", IS_CARRYING_W(ch), CAN_CARRY_W(ch));
  if (GET_LEVEL(ch) < LVL_IMMORT)
    send_to_char(ch, "&BExp Gained:&n %ld           &BExp to Level:&n  %ld\r\n",
	         GET_EXP(ch), level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1) - GET_EXP(ch));

  playing_time = *real_time_passed((time(0) - ch->player.time.logon) +
				  ch->player.time.played, 0);

  send_to_char(ch, "You have earned %d quest points.\r\n", GET_QUESTPOINTS(ch));
  send_to_char(ch, "You have completed %d quest%s, ",
		    GET_NUM_QUESTS(ch),
		    GET_NUM_QUESTS(ch) == 1 ? "" : "s");
  if (GET_QUEST(ch) == NOTHING)
    send_to_char(ch, "and you are not on a quest at the moment.\r\n");
  else
    send_to_char(ch, "and your current quest is %d.\r\n",
                     GET_QUEST(ch) == NOTHING ? -1 : GET_QUEST(ch));

  send_to_char(ch, "&GYou have been playing for %d day%s and %d hour%s.&n\r\n",
     playing_time.day, playing_time.day == 1 ? "" : "s",
     playing_time.hours, playing_time.hours == 1 ? "" : "s");

/*  if(GET_CLAN(ch) > CLAN_NONE)
    send_to_char(ch,   "&BClan : &W%s &R(&G%s&R)\r\n&BRank : &W%s\r\n",
     get_blank_clan_name(GET_CLAN(ch)), get_clan_name(GET_CLAN(ch)),
     get_rank_name(GET_CLAN(ch), GET_CLAN_RANK(ch)));
  */

  if (GET_PKBLOOD(ch) <= 0)
     send_to_char(ch, "&W%s\r\n", blood_msg[0]);
  else if (GET_PKBLOOD(ch) <= 10)
     send_to_char(ch, "&R%s\r\n", blood_msg[1]);
  else if (GET_PKBLOOD(ch) <= 40)
     send_to_char(ch, "&R%s\r\n", blood_msg[2]);
  else if (GET_PKBLOOD(ch) <= 100)
     send_to_char(ch, "&r%s\r\n", blood_msg[3]);
  else if (GET_PKBLOOD(ch) <= 200)
     send_to_char(ch, "&r%s\r\n", blood_msg[4]);
  else 
     send_to_char(ch, "&r%s\r\n", blood_msg[5]);

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
     send_to_char(ch, "&YYou are in NPK&n\r\n");
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
     send_to_char(ch, "&RYou are in CPK&n\r\n");

  switch (GET_POS(ch)) {
  case POS_DEAD:
    send_to_char(ch, "&RYou are DEAD!&n\r\n");
    break;
  case POS_MORTALLYW:
    send_to_char(ch, "&RYou are mortally wounded!  You should seek help!&n\r\n");
    break;
  case POS_INCAP:
    send_to_char(ch, "&MYou are incapacitated, slowly fading away...&n\r\n");
    break;
  case POS_STUNNED:
    send_to_char(ch, "&YYou are stunned!  You can't move!&n\r\n");
    break;
  case POS_SLEEPING:
    send_to_char(ch, "&WYou are sleeping.&n\r\n");
    break;
  case POS_RESTING:
    send_to_char(ch, "&GYou are resting.&n\r\n");
    break;
  case POS_SITTING:
    send_to_char(ch, "&CYou are sitting.&n\r\n");
    break;
  case POS_FIGHTING:
    send_to_char(ch, "You are fighting %s.\r\n", FIGHTING(ch) ? PERS(FIGHTING(ch), ch) : "thin air");
    break;
  case POS_STANDING:
    send_to_char(ch, "You are standing.\r\n");
    break;
  default:
    send_to_char(ch, "You are floating.\r\n");
    break;
  }

  if (GET_COND(ch, DRUNK) > 10)
    send_to_char(ch, "You are intoxicated.\r\n");

  if (GET_COND(ch, HUNGER) == 0)
    send_to_char(ch, "You are hungry.\r\n");

  if (GET_COND(ch, THIRST) == 0)
    send_to_char(ch, "You are thirsty.\r\n");
/*
  if (AFF_FLAGGED(ch, AFF_BLIND))
    send_to_char(ch, "You have been blinded!\r\n");

  if (AFF_FLAGGED(ch, AFF_INVISIBLE))
    send_to_char(ch, "You are invisible.\r\n");

  if (AFF_FLAGGED(ch, AFF_DETECT_INVIS))
    send_to_char(ch, "You are sensitive to the presence of invisible things.\r\n");

  if (AFF_FLAGGED(ch, AFF_SANCTUARY))
    send_to_char(ch, "You are protected by Sanctuary.\r\n");

  if (AFF_FLAGGED(ch, AFF_SPARKSHIELD))
    send_to_char(ch, "You are surrounded by a shimmering field of energy.\r\n");

  if (AFF_FLAGGED(ch, AFF_POISON))
    send_to_char(ch, "You are poisoned!\r\n");

  if (AFF_FLAGGED(ch, AFF_CHARM))
    send_to_char(ch, "You have been charmed!\r\n");

  if (affected_by_spell(ch, SPELL_ARMOR))
    send_to_char(ch, "You feel protected.\r\n");

  if (AFF_FLAGGED(ch, AFF_INFRAVISION))
    send_to_char(ch, "Your eyes are glowing red.\r\n");

  if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
    send_to_char(ch, "You are summonable by other players.\r\n");
*/  
  send_to_char(ch, "&MSpell Effects:\r\n");
  if (ch->affected) {
    for (aff = ch->affected; aff; aff = aff->next) 
    {
      if (aff->type != SKILL_SNEAK)         
      {
      send_to_char(ch, "&G       %-21s  - %3ld hrs", skill_name(aff->type), aff->duration +1);

      if (aff->modifier)
        send_to_char(ch, " %+d to %s", aff->modifier, apply_types[(int) aff->location]);

//      if (aff->bitvector) {
//        if (aff->modifier)
//          send_to_char(ch, " ");

//        sprintbit(aff->bitvector, affected_bits, buf, sizeof(buf));
//        send_to_char(ch, "&Wsets %s", buf);
      }
      
      send_to_char(ch, "\r\n");
      }
    }
  

}


ACMD(do_inventory)
{
  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }

  send_to_char(ch, "&GYou are carrying:&n\r\n");
  list_obj_to_char(ch->carrying, ch, SHOW_OBJ_SHORT, TRUE);
}


ACMD(do_equipment)
{
  int i, found = 0;

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }


  send_to_char(ch, "&GYou are using:&n\r\n");
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
	send_to_char(ch, "&M%s&n", wear_where[i]);
	show_obj_to_char(GET_EQ(ch, i), ch, SHOW_OBJ_SHORT);
	found = TRUE;
      } else {
	send_to_char(ch, "&M%s&n", wear_where[i]);
	send_to_char(ch, "&GSomething.&n\r\n");
	found = TRUE;
      }
    }
    else
    {
       send_to_char(ch, "&M%s&n", wear_where[i]);
       send_to_char(ch, "&wNothing.&n\r\n");
    }
  }
//  if (!found)
//    send_to_char(ch, "&G Nothing.&n\r\n");

}

ACMD(do_time)
{
  const char *suf;
  int weekday, day;

  if (PLR_FLAGGED(ch, PLR_INCRYGATE)) {
    send_to_char(ch, "You do not have control of your bodily functions while in the Crystal Gate.\r\n");
    return;
  }

  send_to_char(ch, "&C");

  // day in [1..35] 
  day = time_info.day + 1;

  // 35 days in a month, 7 days a week 
  weekday = ((35 * time_info.month) + day) % 7;

  send_to_char(ch, "It is %d o'clock %s, on %s.\r\n",
	  (time_info.hours % 12 == 0) ? 12 : (time_info.hours % 12),
	  time_info.hours >= 12 ? "pm" : "am", weekdays[weekday]);

  suf = "th";

  if (((day % 100) / 10) != 1) {
    switch (day % 10) {
    case 1:
      suf = "st";
      break;
    case 2:
      suf = "nd";
      break;
    case 3:
      suf = "rd";
      break;
    }
  }

  send_to_char(ch, "The %d%s Day of the %s, Year %ld.\r\n",
	  day, suf, month_name[time_info.month], time_info.year);
  send_to_char(ch, "&n");
}


/*
ACMD(do_weather)
{
  const char *sky_look[] = {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"
  };

  if (OUTSIDE(ch))
    {
    send_to_char(ch, "&CThe sky is %s and %s.&n\r\n", sky_look[weather_info.sky],
	    weather_info.change >= 0 ? "&Cyou feel a warm wind from south&n" :
	     "&Cyour foot tells you bad weather is due&n");
    if (GET_LEVEL(ch) >= LVL_GOD)
      send_to_char(ch, "&WPressure:&C %d &W(change: &C%d&W), Sky: &C%d&W (&C%s&W)&n\r\n",
                 weather_info.pressure,
                 weather_info.change,
                 weather_info.sky,
                 sky_look[weather_info.sky]);
    }
  else
    send_to_char(ch, "&CYou have no feeling about the weather at all.&n\r\n");
}
*/

ACMD(do_weather)
{
    char buf[MAX_INPUT_LENGTH];

        WEATHER_DATA *cond;

        if (!OUTSIDE(ch))
        {
                send_to_char(ch, "You have no feeling about the weather at all.\r\n");
                return;
        }

        if (!(cond = get_room_weather(IN_ROOM(ch))))
                return;

        if (time_info.hours < 5 || time_info.hours > 23)
        {
                if (cond->precip_rate < 30 && cond->humidity < 55)
                {
                        if (MoonPhase != MOON_NEW)
                                ch_printf(ch, "The nocturnal sky, full of stars, is graced by a %s.\r\n", moon_look[MoonPhase]);
                        else
                                send_to_char(ch, "Only stars in the sky, the moon is a black shape that cover some of them.\r\n");
                }
        }

        *buf = '\0';
        if (cond->precip_rate)
        {
                if (cond->temp <= 0)
                        strcat(buf, "It's snowing");
                else
                        strcat(buf, "It's raining");

                if      (cond->precip_rate > 65) strcat(buf, " extremely hard");
                else if (cond->precip_rate > 50) strcat(buf, " very hard");
                else if (cond->precip_rate > 30) strcat(buf, " hard");
                else if (cond->precip_rate < 15) strcat(buf, " lightly");
                strcat(buf, ", ");
        }
        else
        {
                if      (cond->humidity > 80)   strcat(buf, "It's very cloudy, ");
                else if (cond->humidity > 55)   strcat(buf, "It's cloudy, ");
                else if (cond->humidity > 25)   strcat(buf, "It's partly cloudy, ");
                else if (cond->humidity)        strcat(buf, "It's mostly clear, ");
                else                            strcat(buf, "It's clear, ");
        }

        strcat(buf, "the air is ");
        if      (cond->temp > 60)               strcat(buf, "boiling, ");
        else if (cond->temp > 52)               strcat(buf, "incredibly hot, ");
        else if (cond->temp > 37)               strcat(buf, "very, very hot, ");
        else if (cond->temp > 29)               strcat(buf, "very hot, ");
        else if (cond->temp > 25)               strcat(buf, "hot, ");
        else if (cond->temp > 18)               strcat(buf, "warm, ");
        else if (cond->temp > 9)                strcat(buf, "mild, ");
        else if (cond->temp > 1)                strcat(buf, "cool, ");
        else if (cond->temp > -5)               strcat(buf, "cold, ");
        else if (cond->temp > -10)              strcat(buf, "freezing, ");
        else if (cond->temp > -20)              strcat(buf, "well past freezing, ");
        else                                    strcat(buf, "numbingly frozen, ");

        strcat(buf, "and ");
        if (cond->windspeed <= 0)               strcat(buf, "there is absolutely no wind");
        else if (cond->windspeed < 10)          strcat(buf, "calm");
        else if (cond->windspeed < 20)          strcat(buf, "breezy");
        else if (cond->windspeed < 35)          strcat(buf, "windy");
        else if (cond->windspeed < 50)          strcat(buf, "very windy");
        else if (cond->windspeed < 70)          strcat(buf, "very, very windy");
        else if (cond->windspeed < 100)         strcat(buf, "there is a gale blowing");
        else                                    strcat(buf, "the wind is unbelievable");
        strcat(buf, ".\r\n");
        send_to_char(ch, buf);

        if (IS_IMMORTAL(ch))
                ch_printf(ch, "PR: %d  Hm: %d  Tm: %d  Wn: %d  En: %d\r\n",
                        cond->precip_rate, cond->humidity, cond->temp, cond->windspeed,
                        cond->free_energy);

        if (IS_CLERIC(ch) || IS_MAGIC_USER(ch))
        {
                if (cond->free_energy > 40000)
                        send_to_char(ch, "Wow! This place is bursting with energy!\r\n");
                else if (cond->free_energy > 30000)
                        send_to_char(ch, "The environs tingle your magical senses.\r\n");
                else if (cond->free_energy > 20000)
                        send_to_char(ch, "The area is rich with energy.\r\n");
                else if (cond->free_energy < 5000)
                        send_to_char(ch, "There is almost no magical energy here.\r\n");
                else if (cond->free_energy < 1000)
                        send_to_char(ch, "Your magical senses are dulled by the scarceness of energy here.\r\n");
        }
}

struct help_index_element *find_help(char *keyword)
{
  extern int hnum;
  int helpnumber;
  struct help_keywords *kptr;
  HELP_DATA *hptr;

  for (kptr = keyword_list; kptr; kptr = kptr->next)
    if (is_abbrev(keyword, kptr->keyword))
       helpnumber = kptr->helpid;

  for (hptr = help_info; hptr; hptr = hptr->next)
    if (hptr->index == helpnumber)
      return (hptr);

  return NULL;
}

ACMD(do_help)
{
  HELP_DATA *this_help;
  char entry[MAX_STRING_LENGTH];


  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    page_string(ch->desc, help, 0);
    return;
  }
  if (!help_table) {
    send_to_char(ch, "&RNo help file available for that. Ask Tek to make one.&n\r\n");
    return;
  }

  if (!(this_help = find_help(argument))) {
      send_to_char(ch, "&RThere is no help on that word. Type IDEA <txt> to notify us to add a file for it.&n\r\n");
      sprintf(buf, "HELP: %s tried to get help on %s", GET_NAME(ch), argument);
      log(buf);
      return;
   }

  if (this_help->min_level > GET_LEVEL(ch)) {
    send_to_char(ch, "There is no help on that word.\r\n");
    return;
   }

  if (GET_LEVEL(ch) > LVL_IMM) {
    send_to_char(ch, "\r\n&WHelp IDNUM %d\r\n", this_help->index);
   }

  sprintf(entry, "\r\n&g%s\r\n"
                 "\r\n%s\r\n\r\n"
                 "\r\nSee Also: %s\r\n&n", 
                 this_help->keywords, 
		 this_help->entry, 
		 this_help->akeys );
  
  page_string(ch->desc, entry, 0);

}



#define WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-c classlist] [-s] [-o] [-p] [-q] [-r] [-z]\r\n"

// FIXME: This whole thing just needs rewritten. 
ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  char name_search[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  char mode;
  int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0, ttl=0;
  int showclass = 0, short_list = 0, outlaws = 0, num_can_see = 0;
  int who_room = 0;
  int preceptors = 0;
  int c_level = 44, classes=0;

  skip_spaces(&argument);
  strcpy(buf, argument);	// strcpy: OK (sizeof: argument == buf)
  name_search[0] = '\0';

  while (*buf) {
    char arg[MAX_INPUT_LENGTH], buf1[MAX_INPUT_LENGTH];

    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
    } else if (*arg == '-') {
      mode = *(arg + 1);      // just in case; we destroy arg in the switch 
      switch (mode) {
      case 'o':
      case 'p':
        preceptors = 1;
        strcpy(buf, buf1);      // strcpy: OK (sizeof: buf1 == buf)
        break;
      case 'k':
	outlaws = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 'z':
	localwho = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 's':
	short_list = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 'q':
	questwho = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 'c':
	half_chop(buf1, arg, buf);
	showclass = find_class_bitvector(arg);
	break;
      default:
	send_to_char(ch, "%s", WHO_FORMAT);
	return;
      }				// end of switch 

    } else {			// endif 
      send_to_char(ch, "%s", WHO_FORMAT);
      return;
    }
  }				// end while (parser) 

  send_to_char(ch, 
     "\r\n&R[Sex] [Lvl/TL] [#Cl] [CurrentClass] &Y[   PKs] &G[Guild] &CPlayer \r\n"
     "--------------------------------------------------------------------------------------------------&n\r\n");
  
  for (; c_level > 0;c_level--) {

  for (d = descriptor_list; d; d = d->next) {
    if (!IS_PLAYING(d))
      continue;

    if (GET_LEVEL(d->character) !=c_level)  // end of sorting by levels 
     continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    classes = (GET_WAS_WARRIOR(tch) + GET_WAS_CLERIC(tch) + GET_WAS_THIEF(tch) + GET_WAS_MAGE(tch) + GET_WAS_PALADIN(tch)+ 
               GET_WAS_KNIGHT(tch)+ GET_WAS_RANGER(tch)+ GET_WAS_MYSTIC(tch)+ GET_WAS_PRIEST(tch)+ GET_WAS_SHAMAN(tch)+ 
               GET_WAS_NECROMANCER(tch)+ GET_WAS_ELEMENTALIST(tch)+ GET_WAS_SORCERER(tch)+ GET_WAS_ROGUE(tch)+ 
               GET_WAS_NINJA(tch)+ GET_WAS_ASSASSIN(tch));
/*
    if (GET_WAS_WARRIOR(tch) > 0)
      classes++;

    if (GET_WAS_THIEF(tch) > 0)
      classes++;

    if (GET_WAS_MAGE(tch) > 0)
      classes++;

    if (GET_WAS_CLERIC(tch) > 0)
      classes++;
*/
    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	!strstr(GET_TITLE(tch), name_search))
      continue;
    if ((!CAN_SEE(ch, tch) || (GET_LEVEL(tch) < low) || (GET_LEVEL(tch) > high)) && GET_LEVEL(ch) < LVL_GOD)
      continue;
    if (preceptors)
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	!PLR_FLAGGED(tch, PLR_THIEF))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[IN_ROOM(ch)].zone != world[IN_ROOM(tch)].zone)
      continue;
    if (who_room && (IN_ROOM(tch) != IN_ROOM(ch)))
      continue;
    if (showclass && !(showclass & (1 << GET_CLASS(tch))))
      continue;
    if (classes < 1)
       classes = 1;

    switch(classes)
    {
     case 1:
       ttl = GET_LEVEL(tch);
       break;
     case 2:
       ttl = GET_LEVEL(tch) + 40;
       break;
     case 3:
       ttl = GET_LEVEL(tch) + 80;   
       break;
     case 4:
       ttl = GET_LEVEL(tch) + 120;
       break;
     default:
       ttl = GET_LEVEL(tch) + 160;
    }

    if (short_list) {
     if(ttl > 200)
     {
      send_to_char(ch, "&R[ %s ] &C[%2d/%3d] [%3d] &R[%12s] &Y[%6d] &G[ %3d ] &C%s&n",
              (tch->player.sex == SEX_MALE ? "M" : "F"),
 	      GET_LEVEL(tch), ttl, classes, pc_class_types[(int)GET_CLASS(tch)], GET_PLAYER_KILLS(tch),
              GET_CLAN(tch), GET_NAME(tch));
            if (PRF_FLAGGED(tch, PRF_AFK))
             send_to_char(ch, " &C(afk)&n");
     }
     else if (ttl == 200)
     {
      send_to_char(ch, "&R[ %s ] [%2d/%3d] [%3d] [%12s] &Y[%6d] &G[ %3d ] &C%s&n",
              (tch->player.sex == SEX_MALE ? "M" : "F"),
              GET_LEVEL(tch), ttl, classes, pc_class_types[(int)GET_CLASS(tch)], GET_PLAYER_KILLS(tch),
              GET_CLAN(tch), GET_NAME(tch));
            if (PRF_FLAGGED(tch, PRF_AFK))
             send_to_char(ch, " &C(afk)&n");
     }
     else
     {
      send_to_char(ch, "&R[ %s ] &Y[%2d/%3d] [%3d] &R[%12s] &Y[%6d] &G[ %3d ] &C%s&n",
              (tch->player.sex == SEX_MALE ? "M" : "F"),
              GET_LEVEL(tch), ttl, classes, pc_class_types[(int)GET_CLASS(tch)], GET_PLAYER_KILLS(tch),
              GET_CLAN(tch), GET_NAME(tch));
            if (PRF_FLAGGED(tch, PRF_AFK))
             send_to_char(ch, " &C(afk)&n");
     }
     send_to_char(ch, "\r\n");
    } else {
      num_can_see++;
     if(ttl > 200)
     {
      send_to_char(ch, "&R[ %s ] &C[%2d/%3d] [%3d] &R[%12s] &Y[%6d] &G[ %3d ] &C%s&n",
              (tch->player.sex == SEX_MALE ? "M" : "F"),
              GET_LEVEL(tch), ttl, classes, pc_class_types[(int)GET_CLASS(tch)], GET_PLAYER_KILLS(tch),
              GET_CLAN(tch), GET_NAME(tch));
     }
     else if (ttl == 200)
     {
      send_to_char(ch, "&R[ %s ] [%2d/%3d] [%3d] [%12s] &Y[%6d] &G[ %3d ] &C%s&n",
              (tch->player.sex == SEX_MALE ? "M" : "F"),
              GET_LEVEL(tch), ttl, classes, pc_class_types[(int)GET_CLASS(tch)], GET_PLAYER_KILLS(tch),
              GET_CLAN(tch), GET_NAME(tch));
     }
     else
     {
      send_to_char(ch, "&R[ %s ] &Y[%2d/%3d] [%3d] &R[%12s] &Y[%6d] &G[ %3d ] &C%s&n",
              (tch->player.sex == SEX_MALE ? "M" : "F"),
              GET_LEVEL(tch), ttl, classes, pc_class_types[(int)GET_CLASS(tch)], GET_PLAYER_KILLS(tch),
              GET_CLAN(tch), GET_NAME(tch));
     }

      if (classes > 4 && GET_LEVEL(tch) == LVL_IMMORT)
         send_to_char(ch, "&W (MASTER)");
      if (PRF_FLAGGED(tch, PRF_AFK))
        send_to_char(ch, " &C(afk)&n");   
      if (PLR_FLAGGED(tch, PLR_PARTYIN))
        send_to_char(ch, " &cInTheMist&n");
 //     if (PLR_FLAGGED(tch, PLR_PRECEPTOR))
 //       send_to_char(ch, " &B(&YPRECEPTOR&B)&n");
      if (PLR_FLAGGED(tch, PLR_OUTLAW))
        send_to_char(ch, " &W(&ROUTLAW&W)&n");
      if (GET_INVIS_LEV(tch))
	send_to_char(ch, " (i%ld)", GET_INVIS_LEV(tch));
      else if (AFF_FLAGGED(tch, AFF_INVISIBLE))
	send_to_char(ch, "&W (invisible)&n");

      if (PLR_FLAGGED(tch, PLR_MAILING))
	send_to_char(ch, "&W (mailing)&n");
      else if (d->olc)
	send_to_char(ch, "&W (OLC)&n");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
	send_to_char(ch, "&W (writing)&n");

      if (d->original)
        send_to_char(ch, "&W (DEAD)&n");
       
      if (d->connected == CON_OEDIT)
        send_to_char(ch, "&Y (Object Edit)&n");
      if (d->connected == CON_MEDIT)
        send_to_char(ch, "&Y (Mobile Edit)&n");
      if (d->connected == CON_ZEDIT)
        send_to_char(ch, "&y (Zone Edit)&n");
      if (d->connected == CON_SEDIT)
        send_to_char(ch, "&Y (Shop Edit)&n");
      if (d->connected == CON_REDIT)
        send_to_char(ch, "&Y (Room Edit)&n");
      if (d->connected == CON_TEDIT)
        send_to_char(ch, "&Y (Text Edit)&n");
      if (d->connected == CON_AEDIT)
        send_to_char(ch, "&Y (Social Edit)&n");
      if (d->connected == CON_CEDIT)
        send_to_char(ch, "&Y (Configuration Edit)&n");

      if (PRF_FLAGGED(tch, PRF_BUILDWALK))
	send_to_char(ch, "&C (Buildwalking)&n");

      if (PLR_FLAGGED(tch, PLR_DEAF))
	send_to_char(ch, "&M (deaf)&n");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
	send_to_char(ch, "&M (notell)&n");
      if (PRF_FLAGGED(tch, PRF_QUEST))
	send_to_char(ch, "&M (quest)&n");
      if (PLR_FLAGGED(tch, PLR_THIEF))
	send_to_char(ch, "&M (THIEF)&n");
      if (PLR_FLAGGED(tch, PLR_KILLER))
	send_to_char(ch, "&M (KILLER)&n");
      if (GET_LEVEL(tch) >= LVL_IMMORT)
	send_to_char(ch, CCNRM(ch, C_SPR));
      send_to_char(ch, "\r\n");
      classes = 0;
    }				// endif shortlist 
  }	
  }			// end of for 
  if (short_list && (num_can_see % 4))
    send_to_char(ch, "\r\n");
  if (num_can_see == 0)
    send_to_char(ch, "\r\n&MNobody at all!&n\r\n");
  else if (num_can_see == 1)
    send_to_char(ch, "\r\n&YOne lonely character displayed.&n\r\n");
  else
    send_to_char(ch, "\r\n&G%d characters displayed.&n\r\n", num_can_see);
}


#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

// BIG OL' FIXME: Rewrite it all. Similar to do_who(). 
ACMD(do_users)
{
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  int low = 0, high = LVL_IMPL, num_can_see = 0;
  int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;
  int preceptors = 0;
  char buf[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);	// strcpy: OK (sizeof: argument == buf) 
  while (*buf) {
    char buf1[MAX_INPUT_LENGTH];

    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  // just in case; we destroy arg in the switch 
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	playing = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 'p':
	playing = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 'd':
	deadweight = 1;
	strcpy(buf, buf1);	// strcpy: OK (sizeof: buf1 == buf) 
	break;
      case 'l':
	playing = 1;
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	playing = 1;
	half_chop(buf1, name_search, buf);
	break;
      case 'h':
	playing = 1;
	half_chop(buf1, host_search, buf);
	break;
      case 'c':
	playing = 1;
	half_chop(buf1, arg, buf);
	showclass = find_class_bitvector(arg);
	break;
      default:
	send_to_char(ch, "%s", USERS_FORMAT);
	return;
      }				// end of switch 
    } else {			// endif 
      send_to_char(ch, "%s", USERS_FORMAT);
      return;
    }
  }				// end while (parser) 
  send_to_char(ch,
	 "&WNum Class   Name         State          Idl Login@   Site\r\n"
	 "--- ------- ------------ -------------- --- -------- ------------------------&n\r\n");

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING && playing)
      continue;
    if (STATE(d) == CON_PLAYING && deadweight)
      continue;
    if (IS_PLAYING(d)) {
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;

      if (*host_search && !strstr(d->host, host_search))
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (preceptors && !PLR_FLAGGED(tch, PLR_KILLER))
        continue;
      if (showclass && !(showclass & (1 << GET_CLASS(tch))))
	continue;
      if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
	continue;

      if (d->original)
	sprintf(classname, "&R[%2d %s]&n", GET_LEVEL(d->original),
		CLASS_ABBR(d->original));
      else
	sprintf(classname, "&R[%2d %s]&C", GET_LEVEL(d->character),
		CLASS_ABBR(d->character));
    } else
      strcpy(classname, "&C   -  &n ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (STATE(d) == CON_PLAYING && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[STATE(d)]);

    if (d->character && STATE(d) == CON_PLAYING && GET_LEVEL(d->character) < LVL_GOD)
      sprintf(idletime, "&C%3d&n", d->character->char_specials.timer *
	      SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else
      strcpy(idletime, "");

    sprintf(line, "&C%3d %-7s %-12s &C%-14s &C%-3s %-8s&n ", d->desc_num, classname,
	d->original && d->original->player.name ? d->original->player.name :
	d->character && d->character->player.name ? d->character->player.name :
	"UNDEFINED",
	state, idletime, timeptr);

    if (d->host && *d->host)
      sprintf(line + strlen(line), "&C[%s]\r\n&n", d->host);
    else
      strcat(line, "[Hostname unknown]\r\n");

    if (STATE(d) != CON_PLAYING) {
      sprintf(line2, "&C%s%s%s&n", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      strcpy(line, line2);
    }
    if (STATE(d) != CON_PLAYING ||
		(STATE(d) == CON_PLAYING && CAN_SEE(ch, d->character))) {
      send_to_char(ch, "&C%s&n", line);
      num_can_see++;
    }
  }

  send_to_char(ch, "\r\n&C%d visible sockets connected.&n\r\n", num_can_see);
}


// Generic page_string function for displaying text 
ACMD(do_gen_ps)
{
  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    page_string(ch->desc, info, 0);
    break;
//  case SCMD_WIZLIST:
//    page_string(ch->desc, wizlist, 0);
//    break;
  case SCMD_IMMLIST:
    page_string(ch->desc, immlist, 0);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    page_string(ch->desc, motd, 0);
    break;
  case SCMD_IMOTD:
    page_string(ch->desc, imotd, 0);
    break;
  case SCMD_CLEAR:
    send_to_char(ch, "\033[H\033[J");
    break;
  case SCMD_VERSION:
//    send_to_char(ch, "%s\r\n", circlemud_version);
    send_to_char(ch, "%s\r\n", ascii_pfiles_version);
    send_to_char(ch, "%s\r\n", oasisolc_version);
    send_to_char(ch, "%s\r\n", DG_SCRIPT_VERSION);
    break;
  case SCMD_WHOAMI:
    send_to_char(ch, "&GMan, you MUST be high, your %s!&n\r\n", GET_NAME(ch));
    break;
  default:
    log("SYSERR: Unhandled case in do_gen_ps. (%d)", subcmd);
    return;
  }
}


void perform_mortal_where(struct char_data *ch, char *arg)
{
  struct char_data *i;
  struct descriptor_data *d;
  struct room_data *rm = &world[IN_ROOM(ch)];

  if (!*arg) {
    send_to_char(ch, "&WPlayers around %s\r\n--------------------&n\r\n", zone_table[rm->zone].name);
    for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) != CON_PLAYING || d->character == ch)
	continue;
      if ((i = (d->original ? d->original : d->character)) == NULL)
	continue;
      if (IN_ROOM(i) == NOWHERE || !CAN_SEE(ch, i))
	continue;
      if (world[IN_ROOM(ch)].zone != world[IN_ROOM(i)].zone)
	continue;
      if (AFF_FLAGGED(i, AFF_SNEAK))
        send_to_char(ch, "&C%-20s &R- &YSomewhere&n\r\n", GET_NAME(i));
      else
        send_to_char(ch, "&C%-20s &R- &Y%s&n\r\n", GET_NAME(i), world[IN_ROOM(i)].name);
    }
  } else {			// print only FIRST char, not all. 
    for (i = character_list; i; i = i->next) {
      if (IN_ROOM(i) == NOWHERE || i == ch)
	continue;
      if (!CAN_SEE(ch, i) || world[IN_ROOM(i)].zone != world[IN_ROOM(ch)].zone)
	continue;
      if (!isname(arg, i->player.name))
	continue;
      if (AFF_FLAGGED(i, AFF_SNEAK))
        send_to_char(ch, "&CSomewhere\r\n&n");
      else
        send_to_char(ch, "&C%-25s &R- &Y%s\r\n", GET_NAME(i), world[IN_ROOM(i)].name);
      return;
    }
    send_to_char(ch, "Nobody around by that name.\r\n");
  }
}


void print_object_location(int num, struct obj_data *obj, struct char_data *ch,
			        int recur)
{
  if (num > 0)
    send_to_char(ch, "O%3d. %-25s - ", num, obj->short_description);
  else
    send_to_char(ch, "%33s", " - ");

  if (IN_ROOM(obj) != NOWHERE)
    send_to_char(ch, "[%5d] %s\r\n", GET_ROOM_VNUM(IN_ROOM(obj)), world[IN_ROOM(obj)].name);
  else if (obj->carried_by)
    send_to_char(ch, "carried by %s\r\n", PERS(obj->carried_by, ch));
  else if (obj->worn_by)
    send_to_char(ch, "worn by %s\r\n", PERS(obj->worn_by, ch));
  else if (obj->in_obj) {
    send_to_char(ch, "inside %s%s\r\n", obj->in_obj->short_description, (recur ? ", which is" : " "));
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur);
  } else
    send_to_char(ch, "in an unknown location\r\n");
}



void perform_immort_where(struct char_data *ch, char *arg)
{
  struct char_data *i;
  struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;

  if (!*arg) {
    send_to_char(ch, "&WPlayers\r\n-------&n\r\n");
    for (d = descriptor_list; d; d = d->next)
      if (STATE(d) == CON_PLAYING) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (IN_ROOM(i) != NOWHERE)) {
	  if (d->original)
	    send_to_char(ch, "&C%-20s &R- &Y[%5d] %s &M(in %s)&n\r\n",
		GET_NAME(i), GET_ROOM_VNUM(IN_ROOM(d->character)),
		world[IN_ROOM(d->character)].name, GET_NAME(d->character));
	  else
	    send_to_char(ch, "&C%-20s &R- &Y[%5d] &M%s&n\r\n", GET_NAME(i), GET_ROOM_VNUM(IN_ROOM(i)), world[IN_ROOM(i)].name);
	}
      }
  } else {
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && IN_ROOM(i) != NOWHERE && isname(arg, i->player.name)) {
	found = 1;
	send_to_char(ch, "&C%3d. %-25s &R- &Y[%5d] &M%s&n\r\n", ++num, GET_NAME(i),
		GET_ROOM_VNUM(IN_ROOM(i)), world[IN_ROOM(i)].name);
      }
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
	found = 1;
	print_object_location(++num, k, ch, TRUE);
      }
    if (!found)
      send_to_char(ch, "Couldn't find any such thing.\r\n");
  }
}



ACMD(do_where)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (GET_LEVEL(ch) >= LVL_GOD)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
  size_t nlen;
  int i, len=0;

  if (IS_NPC(ch)) {
    send_to_char(ch, "You ain't nothin' but a hound-dog.\r\n");
    return;
  }

  for (i = 1; i < LVL_IMMORT; i++) {
    nlen = snprintf(buf + len, sizeof(buf) - len, "[%2d] %8d-%-8d : ", i,
		level_exp(GET_CLASS(ch), i), level_exp(GET_CLASS(ch), i + 1) - 1);
    if (len + nlen >= sizeof(buf) || nlen < 0)
      break;
    len += nlen;

    switch (GET_SEX(ch)) {
    case SEX_MALE:
    case SEX_NEUTRAL:
      nlen = snprintf(buf + len, sizeof(buf) - len, "%s\r\n", title_male(GET_CLASS(ch), i));
      break;
    case SEX_FEMALE:
      nlen = snprintf(buf + len, sizeof(buf) - len, "%s\r\n", title_female(GET_CLASS(ch), i));
      break;
    default:
      nlen = snprintf(buf + len, sizeof(buf) - len, "Oh dear.  You seem to be sexless.\r\n");
      break;
    }
    if (len + nlen >= sizeof(buf) || nlen < 0)
      break;
    len += nlen;
  }

  if (len < sizeof(buf))
    snprintf(buf + len, sizeof(buf) - len, "[%2d] %8d          : Immortality\r\n",
		LVL_IMMORT, level_exp(GET_CLASS(ch), LVL_IMMORT));
  page_string(ch->desc, buf, TRUE);
}



ACMD(do_consider)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *victim;
  int diff;

  one_argument(argument, buf);

  send_to_char(ch, "%s", CCCYN(ch, C_NRM));

  if (!(victim = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM))) {
    send_to_char(ch, "Consider killing who?\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
    return;
  }
  if (victim == ch) {
    send_to_char(ch, "Got nuthin better to do huh?\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
    return;
  }
  if (!IS_NPC(victim)) {
    send_to_char(ch, "Death is something we all will face one day, are you ready?\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
    return;
  }
  diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

  if (diff <= -10)
{
    send_to_char(ch, "You must be some kinda chicken shit perhaps?\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= -5)
{
    send_to_char(ch, "You could do it with a needle!\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= -2)
{
    send_to_char(ch, "Ferget about it!\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= -1)
{
    send_to_char(ch, "No prob.\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff == 0)
{
    send_to_char(ch, "The perfect match!\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= 1)
{
    send_to_char(ch, "You need a little luck!\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= 2)
{
    send_to_char(ch, "You need a lot of luck!\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= 3)
{
    send_to_char(ch, "You need a lot of luck and some bangin' gear!\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= 5)
{
    send_to_char(ch, "Do you feel lucky, bitch?\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= 10)
{
    send_to_char(ch, "What're you crazy or somethin'?\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}
  else if (diff <= 100)
{
    send_to_char(ch, "AHAHAHAHAHAHAHAHAHAHAHA...You must be stoopid or somethin'!\r\n");
    send_to_char(ch, "%s", CCNRM(ch, C_NRM));
}

}



ACMD(do_diagnose)
{
  char buf[MAX_INPUT_LENGTH];
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_vis(ch, buf, NULL, FIND_CHAR_ROOM)))
      send_to_char(ch, "%s", CONFIG_NOPERSON);
    else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char(ch, "Diagnose who?\r\n");
  }
}


const char *ctypes[] = {
  "off", " "," ", "complete", "\n"
};

ACMD(do_color)
{
  char arg[MAX_INPUT_LENGTH];
  int tp;
  
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
    return;
  }
  if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char(ch, "Usage: color { Off | | | Complete }\r\n");
    return;
  }
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
  SET_BIT_AR(PRF_FLAGS(ch), (PRF_COLOR_1 * (tp & 1)) | (PRF_COLOR_2 * (tp & 2) >> 1));

  send_to_char(ch, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR), CCNRM(ch, C_OFF), ctypes[tp]);
}


ACMD(do_toggle)
{
  char buf2[4];

  if (IS_NPC(ch))
    return;

  if (GET_WIMP_LEV(ch) == 0)
    strcpy(buf2, "OFF");	// strcpy: OK 
  else
    sprintf(buf2, "%-3.3d", GET_WIMP_LEV(ch));	// sprintf: OK 

  if (GET_LEVEL(ch) > LVL_IMMORT) {
    send_to_char(ch,
          "      Buildwalk: %-3s    "
          "Clear Screen in OLC: %-3s\r\n",
        ONOFF(PRF_FLAGGED(ch, PRF_BUILDWALK)),
        ONOFF(PRF_FLAGGED(ch, PRF_CLS))
    );

    
    send_to_char(ch,
	  "      No Hassle: %-3s    "
	  "      Holylight: %-3s    "
	  "     Room Flags: %-3s\r\n"
          "  Clan Channels: %-3s\r\n",
        
	ONOFF(PRF_FLAGGED(ch, PRF_NOHASSLE)),
	ONOFF(PRF_FLAGGED(ch, PRF_HOLYLIGHT)),
	ONOFF(PRF_FLAGGED(ch, PRF_ROOMFLAGS)),
        ONOFF(PRF_FLAGGED(ch, PRF2_ALLCTELL))
    );
  }

  send_to_char(ch,
	  "Hit Pnt Display: %-3s    "
	  "     Brief Mode: %-3s    "
	  " Summon Protect: %-3s\r\n"

	  "   Move Display: %-3s    "
	  "   Compact Mode: %-3s    "
	  "       On Quest: %-3s\r\n"

	  "   Mana Display: %-3s    "
	  "         NoTell: %-3s    "
	  "   Repeat Comm.: %-3s\r\n"

	  " Auto Show Exit: %-3s    "
	  "           Deaf: %-3s    "
	  "     Wimp Level: %-3s\r\n"

	  " Gossip Channel: %-3s    "
	  "Auction Channel: %-3s    "
	  "  Grats Channel: %-3s\r\n"
          "   Clan Channel: %-3s    "

	  "    Color Level: %s\r\n  "
          "        Automap: %-3s    "
          "    Screenwidth: %-3d    ",
	  ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
	  ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
	  ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
	  YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
	  ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
	  YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
	  YESNO(PLR_FLAGGED(ch, PLR_DEAF)),
	  buf2,

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),
          ONOFF(PRF_FLAGGED(ch, PRF2_CLANTALK)),         

	  ctypes[COLOR_LEV(ch)], 
          ONOFF(PRF_FLAGGED(ch, PRF2_AUTOMAP)),
          GET_SCREEN_WIDTH(ch));
}


int sort_commands_helper(const void *a, const void *b)
{
  return strcmp(complete_cmd_info[*(const int *)a].sort_as, 
                complete_cmd_info[*(const int *)b].sort_as);
}

/* puts -'s instead of spaces */
void space_to_minus(char *str)
{
  while ((str = strchr(str, ' ')) != NULL)
    *str = '-';
}


void sort_commands(void)
{
  int a, num_of_cmds = 0;

  while (complete_cmd_info[num_of_cmds].command[0] != '\n')
    num_of_cmds++;
  num_of_cmds++;	// \n 

  CREATE(cmd_sort_info, int, num_of_cmds);

  for (a = 0; a < num_of_cmds; a++)
    cmd_sort_info[a] = a;

  // Don't sort the RESERVED or \n entries. 
  qsort(cmd_sort_info + 1, num_of_cmds - 2, sizeof(int), sort_commands_helper);
}


ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  send_to_char(ch, "%s", CCGRN(ch, C_NRM));

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD)) || IS_NPC(vict)) {
      send_to_char(ch, "Who is that?\r\n");
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char(ch, "You can't see the commands of people above your level.\r\n");
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  send_to_char(ch, "The following %s%s are available to %s:\r\n",
	  wizhelp ? "privileged " : "",
	  socials ? "socials" : "commands",
	  vict == ch ? "you" : GET_NAME(vict));
  send_to_char(ch, "Commands marked with a red star '&R*&g' means they are disabled.\r\n");
  // cmd_num starts at 1, not 0, to remove 'RESERVED' 
  for (no = 1, cmd_num = 1; complete_cmd_info[cmd_sort_info[cmd_num]].command[0] != '\n'; cmd_num++) {
    i = cmd_sort_info[cmd_num];

    if (complete_cmd_info[i].minimum_level < 0 || GET_LEVEL(vict) < complete_cmd_info[i].minimum_level)
      continue;

    if ((complete_cmd_info[i].minimum_level > LVL_IMMORT) != wizhelp)
      continue;

    if (!wizhelp && socials != (complete_cmd_info[i].command_pointer == do_action || complete_cmd_info[i].command_pointer == do_insult))
      continue;

//    send_to_char(ch, "%-11s%s", complete_cmd_info[i].command, no++ % 7 == 0 ? "\r\n" : "");
    if (check_disabled(&complete_cmd_info[i]))
       sprintf(arg, "&R*&g%s", complete_cmd_info[i].command);
    else
       sprintf(arg, "%s", complete_cmd_info[i].command);
    send_to_char(ch, "%-11s%s", arg, no++ % 7 == 0 ? "\r\n" : "");
  }

  if (no % 7 != 1)
    send_to_char(ch, "%s\r\n", CCNRM(ch, C_NRM));
}

void printmap(struct char_data *ch)
{
   int r, c, zone, room;
   int map[10][10];
   struct room_data *rm = &world[IN_ROOM(ch)];

   zone = zone_table[rm->zone].number;
   send_to_char(ch, "&RYou are in %s.&n\r\n", zone_table[rm->zone].name);   

   for (r = 0; r < 10; r++)
   {
    for (c = 0; c < 10; c++)
    {
    map[r][c] = (zone * 100) + (r * 10) + c;
    room = map[r][c];
    get_map_char(ch, room);
    }
   send_to_char(ch, "\r\n");
   }
return; 
}

void printwildmap(struct char_data *ch)
{
   int r, c, zone, room,x=0;
   int map[30][15];
   struct room_data *rm = &world[IN_ROOM(ch)];

   zone = zone_table[rm->zone].number;
   send_to_char(ch, "&RYou are in %s.&n\r\n", zone_table[rm->zone].name);

   for (r = 0; r < 15; r++)
   {
    for (c = 0; c < 30; c++)
    {
    map[r][c] = (zone * 100) + x;
    room = map[r][c];
    get_map_char(ch, room);
    x++;
    }
   send_to_char(ch, "\r\n");
   }
return;
}


void get_map_char(struct char_data *ch, int room)
{
  char buf[MAX_INPUT_LENGTH];
  room_rnum location;

  if ((location = real_room(room)) == NOWHERE)
  {
    send_to_char(ch, "&B ");
    return;
  }
  if (world[location].number == world[IN_ROOM(ch)].number)
  {
    send_to_char(ch, "&W@&n");
    return;
  }
 
  snprintf(buf, sizeof(buf), "%s", world[location].name);

 switch(*buf)
 {
  case 'O':
  {
    send_to_char(ch, "&B~&n");
    break;
  }
  case'V':
  {
     send_to_char(ch, "&K^&n");
     break;
  }
  case 'P':
  {
     send_to_char(ch, "&G#&n");
     break;
  }
  case 'S':
  {
     send_to_char(ch, "&g*&n");
     break;
  }
  case 'H':
  {
     send_to_char(ch, "&y^&n");
     break;
  }
  case 'F':
  {
     send_to_char(ch, "&g#&n");
     break;
  }
  case 'B':
  {
     send_to_char(ch, "&Y*&n");
     break;
  }
  case 'L':
  {
     send_to_char(ch, "&C~&n");
     break;
  }
  case 'R':
  {
     send_to_char(ch, "&c~&n");
     break;
  }
  case 'C':
  {
     send_to_char(ch, " ");
     break;
  }
  case 'M':
  {
     send_to_char(ch, "&R~&n");
     break;
  }
  case 'J':
  {
     send_to_char(ch, "&g#&n");
     break;
  }
  case 'D':
  {
     send_to_char(ch, "&y#&n");
     break;
  }
  
  case 'T':
  {
     send_to_char(ch, "&W$&n");
     break;
  }
  default:
  {
     send_to_char(ch, "&R?&n");
     break;
  }
 }
return;
}

ACMD(do_scan)
{
  struct char_data *i;
  int is_in, dir, dis, maxdis, found = 0;
  char buf[MAX_INPUT_LENGTH];

  const char *distance[] = {
    "right here",
    "immediately ",
    "nearby ",
    "a ways ",
    "far ",
    "very far ",
    "extremely far ",
    "impossibly far ",
  };
  
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SNEAK)) {
    send_to_char(ch, "You have no idea how to do that.\r\n");
    return;
  } 

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    act("&rYou can't see anything, you're blind!&n", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }
  if ((GET_MOVE(ch) < 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    act("&rYou are too exhausted.&n", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }

  maxdis = 2;
  if(IS_RANGER(ch)) {
   if(GET_SKILL(ch, SKILL_GREATER_SENSES) < 50)
     maxdis += 1;
   else 
     maxdis += 2;
  }

  if (GET_LEVEL(ch) > LVL_IMMORT)
    maxdis = 7;

  act("&GYou quickly scan the area and see&W:&n", TRUE, ch, 0, 0, TO_CHAR);
  act("&G$n quickly scans the area.&n", FALSE, ch, 0, 0, TO_ROOM);
  if (GET_LEVEL(ch) < LVL_IMMORT)
    GET_MOVE(ch) -= 3;

  is_in = ch->in_room;
  for (dir = 0; dir < NUM_OF_DIRS; dir++) {
    ch->in_room = is_in;
    for (dis = 0; dis <= maxdis; dis++) {
      if (((dis == 0) && (dir == 0)) || (dis > 0)) {
        for (i = world[ch->in_room].people; i; i = i->next_in_room) {
          if ((!((ch == i) && (dis == 0))) && CAN_SEE(ch, i)) {
            sprintf(buf, "&Y%33s&B: &C%s%s%s%s&n", GET_NAME(i), distance[dis],
                    ((dis > 0) && (dir < (NUM_OF_DIRS - 2))) ? "to the " : "",
                    (dis > 0) ? dirs[dir] : "",
                    ((dis > 0) && (dir > (NUM_OF_DIRS - 3))) ? "wards" : "");
            act(buf, TRUE, ch, 0, 0, TO_CHAR);
            found++;
          }
        }
      }
      if (!CAN_GO(ch, dir) || EXIT_FLAGGED(EXIT(ch, dir), EX_HIDDEN) || (world[ch->in_room].dir_option[dir]->to_room
== is_in))
        break;
      else
        ch->in_room = world[ch->in_room].dir_option[dir]->to_room;
    }
  }
  if (found == 0)
    act("&GNobody anywhere near you.&n", TRUE, ch, 0, 0, TO_CHAR);
  ch->in_room = is_in;
}

ACMD(do_search)
{
  int door, chance = 1;

  char buf[MAX_INPUT_LENGTH];

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char(ch, "&rYou're blind, you can't see a damned thing!&n\r\n");
    return;
  }

  send_to_char(ch, "\r\n&GYou begin to search the room...&n\r\n");

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE) {
      if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
        if (GET_INT(ch) >= 17)
          chance += 1;
        if (rand_number(1,6) <= chance) 
        {
        snprintf(buf, sizeof(buf),  "\r\n&WYou have found a secret door %s.&n\r\n", dirs[door]);
        send_to_char(ch, "%s", buf);
        REMOVE_BIT(EXIT(ch, door)->exit_info, EX_HIDDEN);
        }
      }
    }
  }
  return;
}

ACMD(do_show_ore)
{
   send_to_char(ch, "\r\n&MOre\r\n"
                    "---------------\r\n"
                    "&mAdamantite: %ld \r\n"
                    "&rBronze    : %ld \r\n"
                    "&yCopper    : %ld \r\n"
                    "&YGold      : %ld \r\n"
                    "&cIron      : %ld \r\n"
                    "&WMithril   : %ld \r\n"
                    "&wSilver    : %ld \r\n"
                    "&BTitanium  : %ld \r\n",
                    GET_ADAMANTITE_ORE(ch),GET_BRONZE_ORE(ch),GET_COPPER_ORE(ch),GET_GOLD_ORE(ch),
                    GET_IRON_ORE(ch),GET_MITHRIL_ORE(ch),GET_SILVER_ORE(ch),GET_TITANIUM_ORE(ch));

}

ACMD(do_show_gems)
{
   send_to_char(ch, "\r\n&MGems\r\n"
                    "---------------\r\n"
                    "&wCrystal   : %ld \r\n"
                    "&WDiamond   : %ld \r\n"
                    "&GEmerald   : %ld \r\n"
                    "&rGarnet    : %ld \r\n"
                    "&gJade      : %ld \r\n"
                    "&RRuby      : %ld \r\n"
                    "&CSapphire  : %ld \r\n"
                    "&BTopaz     : %ld \r\n",
                    GET_CRYSTAL(ch),GET_DIAMOND(ch),GET_EMERALD(ch),GET_GARNET(ch),
                    GET_JADE(ch),GET_RUBY(ch),GET_SAPPHIRE(ch),GET_TOPAZ(ch));

}

void proc_color(char *inbuf, int color);

/*
void turn_whointo_html(void)
{

  extern struct descriptor_data *descriptor_list;
  FILE *opf;
  struct descriptor_data *d;
  struct char_data *ch;
//  char mvcmd[256];
//  char modcmd[265];
  int ct = 0;

  if ((opf = fopen("/var/www/lighttpd/who.html", "w")) == 0) 
  { 
   perror("Failed to open who.html for writing.");
   return;
  }

  fprintf(opf,"<html>\n<head>\n<title>Realms of Kuvia - Who's online?</title>\n"
              "<meta http-equiv=refresh content=30;"
              "url=who.html>\n");

  fprintf(opf, "</head>\n<body bgcolor=black>"
               "<h1><font color=white>The following players are currently online:</font></h1>\n"
               "<hr><font color=white>\n");
  fprintf(opf, " <font Color=red>[Sex] [Lvl] [CurrentClass] <font color=yellow>[   Pks] <font color=lime>[Guild]<font color=cyan> Player</font><br>\n");

  for (d = descriptor_list; d; d = d->next)
   if (!d->connected)
   {
    if (d->original)
     ch = d->original;
    else if (!(ch = d->character))
     continue;

    if (GET_INVIS_LEV(ch))
     continue;

    ct++;


    if (GET_LEVEL(ch) < LVL_GOD){
      sprintf(buf, "<font color=red>[ %s ]</font>"  //M or F
                   "<font color=yellow> [ %2d]</font>"//level
                   "<font color=red> [%12s]</font>"  //class
                   "<font color=yellow> [%6d]</font>"//pks
                   "<font color=lime> [ %3d ]</font>"//clan
                   "<font color=cyan> %s</font><br>\n",//name
                    (ch->player.sex == SEX_MALE ? "M" : "F"), GET_LEVEL(ch), pc_class_types[(int)GET_CLASS(ch)], 
                    GET_PLAYER_KILLS(ch), GET_CLAN(ch), GET_NAME(ch));                
  }
    else if (GET_LEVEL(ch) == LVL_IMMORT) {
      sprintf(buf, "<font color=red>[ %s ]</font>"
                   "<font color=red> [ %2d]</font>"    
                   "<font color=red> [%12s]</font>"             
                   "<font color=yellow> [%6d]</font>"       
                   "<font color=lime> [ %3d ]</font>"   
                   "<font color=cyan> %s</font>"
                   "<font color=white> (MASTER)</font><br>\n", 
                    (ch->player.sex == SEX_MALE ? "M" : "F"), GET_LEVEL(ch), pc_class_types[(int)GET_CLASS(ch)], 
                    GET_PLAYER_KILLS(ch), GET_CLAN(ch), GET_NAME(ch));
}

    else {
      sprintf(buf, "<font color=red>[ %s ]</font>"
                   "<font color=cyan> [ %2d]</font>"    
                   "<font color=red> [%12s]</font>"             
                   "<font color=yellow> [%6d]</font>"       
                   "<font color=lime> [ %3d ]</font>"   
                   "<font color=cyan> %s <font color=white(GOD)</font><br>\n", 
                    (ch->player.sex == SEX_MALE ? "M" : "F"), GET_LEVEL(ch), pc_class_types[(int)GET_CLASS(ch)], 
                    GET_PLAYER_KILLS(ch), GET_CLAN(ch), GET_NAME(ch));

    }

    proc_color(buf, 0);
    fprintf(opf, buf);
  }
  if (!ct)
   fprintf(opf, "Nobody<br>\n");

  fprintf(opf,"</font>\n<hr>\n</body>\n</html>\n");
  fclose(opf);
//  sprintf(modcmd, "chmod 777 /var/www/lighttpd/who.html");
//  system(modcmd);
}
*/
ACMD(do_history)
{
  char arg[MAX_INPUT_LENGTH];
  int type;

  one_argument(argument, arg);

  type = search_block(arg, history_types, FALSE);
  if (!*arg || type < 0) {
    int i;

    send_to_char(ch, "Usage: mylog <");
    for (i = 0; *history_types[i] != '\n'; i++) {
      send_to_char(ch, " %s ", history_types[i]);
      if (*history_types[i + 1] == '\n')
        send_to_char(ch, ">\r\n");
      else
        send_to_char(ch, "|");
    }
    return;
  }

  if (GET_HISTORY(ch, type) && GET_HISTORY(ch, type)->text && *GET_HISTORY(ch, type)->text) {
    struct txt_block *tmp;
    for (tmp = GET_HISTORY(ch, type); tmp; tmp = tmp->next)
      send_to_char(ch, "%s", tmp->text);
/* Make this a 1 if you want history to clear after viewing */
#if 0
      free_history(ch, type);
#endif
  } else
    send_to_char(ch, "You have no history in that channel.\r\n");
}


#define HIST_LENGTH 100
void add_history(struct char_data *ch, char *str, int type)
{
  int i = 0;
  char time_str[MAX_STRING_LENGTH];
  struct txt_block *tmp;
  time_t ct;

  if (IS_NPC(ch))
    return;

  tmp = GET_HISTORY(ch, type);
  ct = time(0);
  strftime(time_str, sizeof(time_str), "%H:%M ", localtime(&ct));

  sprintf(buf, "%s%s", time_str, str);

  if (!tmp) {
    CREATE(GET_HISTORY(ch, type), struct txt_block, 1);
    GET_HISTORY(ch, type)->text = strdup(buf);
  }
  else {
    while (tmp->next)
      tmp = tmp->next;
    CREATE(tmp->next, struct txt_block, 1);
    tmp->next->text = strdup(buf);

    for (tmp = GET_HISTORY(ch, type); tmp; tmp = tmp->next, i++);

    for (; i > HIST_LENGTH && GET_HISTORY(ch, type); i--) {
      tmp = GET_HISTORY(ch, type);
      GET_HISTORY(ch, type) = tmp->next;
      if (tmp->text)
        free(tmp->text);
      free(tmp);
    }
  }
  /* add this history message to ALL */
  if (type != HIST_ALL)
    add_history(ch, str, HIST_ALL);
}

int search_help(char *argument)
{
  int chk, bot, top, mid, minlen;

   bot = 0;
   top = top_of_helpt;
   minlen = strlen(argument);

  while (bot <= top) {
    mid = (bot + top) / 2;

    if (!(chk = strn_cmp(argument, help_table[mid].keywords, minlen)))  {
      while ((mid > 0) && !strn_cmp(argument, help_table[mid - 1].keywords, minlen))
         mid--;

      while (mid < (bot + top) / 2)
        mid++;

//      if (strn_cmp(argument, help_table[mid].keywords, minlen) || level < help_table[mid].min_level)
      if (strn_cmp(argument, help_table[mid].keywords, minlen))
              break;

      return mid;
    }
    else if (chk > 0)
      bot = mid + 1;
    else
      top = mid - 1;
  }
  return NOWHERE;
}


ACMD(do_news)
{
  char news[MAX_STRING_LENGTH+1], buf[MAX_INPUT_LENGTH*2];
  char fname[MAX_INPUT_LENGTH*2];
  int newsnumber;
  FILE *fp;

  one_argument(argument, buf); 

  if(!buf || !buf[0]) {
    GET_NEWS(ch)++;
    if(GET_NEWS(ch) > top_news_version)
       GET_NEWS(ch) = top_news_version;
       newsnumber=GET_NEWS(ch);
  }
  else 
  {
    
      if(buf[0]=='n'&&GET_LEVEL(ch)>=33){
      if (!(fp=fopen("../news/news.dat", "r"))){
        send_to_char(ch, "ERROR READING NEWS.DAT!\n\r");
        return;
      }
      fscanf(fp," %d ",&top_news_version);
      fclose(fp);
      send_to_char(ch, "NEWS NEW NEWS NEW NEWS NEW NEWS!!! :)  \n\r");
       return;
      } 
  
    newsnumber = atoi(buf);
  }

  sprintf(fname,"../news/news.%d.txt",newsnumber);

  if (!(fp=fopen(fname, "r"))){
     sprintf(buf,"The news doesn't go that high.\n\r");
     send_to_char(ch, buf);
     return;
  }
  
  fclose(fp);
  file_to_string(fname,news);
  send_to_char(ch, buf);
  page_string(ch->desc, news, TRUE);
    
}


ACMD(do_gnews)      
{                    
  char gnews[MAX_STRING_LENGTH+1], buf[MAX_INPUT_LENGTH*2];
  char fname[MAX_INPUT_LENGTH*2];
  int gnewsnumber;     
  FILE *fp;                  

  one_argument(argument,buf);

  if(!buf || !buf[0]) {
    GET_GNEWS(ch)++;
    if(GET_GNEWS(ch) > top_gnews_version)
       GET_GNEWS(ch) = top_gnews_version;
       gnewsnumber=GET_GNEWS(ch);       
  }                 
  else
  {                  
    
      if(buf[0]=='n'&&GET_LEVEL(ch)>=33){
      if (!(fp=fopen("../gnews/gnews.dat", "r"))){
        send_to_char(ch, "ERROR READING GNEWS.DAT!\n\r");
        return;
      }
      fscanf(fp," %d ",&top_gnews_version);
      fclose(fp);
      send_to_char(ch, "NEW GODNEWS AWAITS You!!\r\n");
       return;
      }

    gnewsnumber = atoi(buf);
  }              

  sprintf(fname,"../gnews/gnews.%d.txt",gnewsnumber);

  if (!(fp=fopen(fname, "r"))){
     sprintf(buf,"The god news doesn't go that high.\n\r");
     send_to_char(ch, buf);
     return;
  }

  fclose(fp);
  file_to_string(fname,gnews);
  send_to_char(ch, buf);
  send_to_char(ch, gnews);
//  page_string(ch->desc, gnews, 1);


}

char *make_bar(long val, long max, long len, int color)
{
        char bar[MAX_INPUT_LENGTH] = "\0";
        int i, n;

        if (color < 1 || color > 7)
                color = 6;

        if (val >= max)
        {
                sprintf(bar, "&b&%d", color);
                for (i = 0; i < len; i++)
                        strcat(bar, "X");
        }
        else
        {
                sprintf(bar, "&b&%d", color);
                for (i = (val * len) / max, n = 0; n < i; n++)
                        strcat(bar, "X");

                sprintf(bar + strlen(bar), "&0&%d", color);
                while ((n++) < len)
                        strcat(bar, "X");
        }
        strcat(bar, "&0");

        return (str_dup(bar));
}

const char *how_far[] =
{
  "immediate area",
  "close by",
  "a ways off",
  "far to the",
  "way far to the",
  "way way far to the",
  "a damn long ways off to the",
  "\n"
};

int list_scanned_chars_to_char(CHAR_DATA *list, CHAR_DATA *ch, int dir, int dist)
{
        CHAR_DATA *i;
        int count = 0;

        for (i = list; i; i = i->next_in_room)
        {
                if (CAN_SEE(ch, i) && (i != ch))
                        count++;
        }

        if (!count)
                return (0);

        if (dist > 6) dist = 6;

        sprintf(buf, "  %s %s you see:", how_far[dist], (dir >= 0 && dir < NUM_OF_DIRS ? dirs[dir] : ""));
        for (i = list; i; i = i->next_in_room)
        {
                if (!CAN_SEE(ch, i) || (i==ch))
                        continue;

                sprintf(buf + strlen(buf), " %s", PERS(i, ch));
        }
        strcat(buf, ".");

        act(buf,FALSE, ch, 0, 0, TO_CHAR);
        return (count);
}


bool get_zone_levels(zone_rnum znum, char *buf)
{
  /* Create a string for the level restrictions for this zone. */
  if ((zone_table[znum].min_level == -1) && (zone_table[znum].max_level == -1)) {
    sprintf(buf, "<Not Set!>");
    return FALSE;
  }

  if (zone_table[znum].min_level == -1) {
    sprintf(buf, "Up to level %d", zone_table[znum].max_level);
    return TRUE;
  }

  if (zone_table[znum].max_level == -1) {
    sprintf(buf, "Above level %d", zone_table[znum].min_level);
    return TRUE;
  }

  sprintf(buf, "Levels %d to %d", zone_table[znum].min_level, zone_table[znum].max_level);
  return TRUE;
}

ACMD(do_areas)
{
  int i, hilev=-1, lolev=-1, zcount=0, lev_set;
  char arg[MAX_INPUT_LENGTH], *second, lev_str[MAX_INPUT_LENGTH];
  bool show_zone = FALSE, overlap = FALSE, overlap_shown = FALSE;

  one_argument(argument, arg);

  if (*arg) {
    /* There was an arg typed - check for level range */
    second = strchr(arg, '-');
    if (second) {
      /* Check for 1st value */
      if (second == arg)
        lolev = 0;
      else
        lolev = atoi(arg);

      /* Check for 2nd value */
      if (*(second+1) == '\0' || !isdigit(*(second+1)) )
        hilev = 100;
      else
        hilev = atoi(second+1);

    } else {
      /* No range - single number */
      lolev = atoi(arg);
      hilev = -1;  /* No high level - indicates single level */
    }
  }
  if (hilev != -1 && lolev > hilev) {
    /* Swap hi and lo lev if needed */
    i     = lolev;
    lolev = hilev;
    hilev = i;
  }
  if (hilev != -1) {
    send_to_char(ch, "&CChecking range&W: &G%d &wto &G%d&n\r\n", lolev, hilev);
    send_to_char(ch, "&y----------------------------------------------------------------------&n\r\n");
  } else if (lolev != -1) {
    send_to_char(ch, "&CChecking level&W: &G%d&n\r\n", lolev);
    send_to_char(ch, "&y----------------------------------------------------------------------&n\r\n");
  } else {
    send_to_char(ch, "&CChecking all areas.&n\r\n");
    send_to_char(ch, "&y----------------------------------------------------------------------&n\r\n");
  }
  for (i = 0; i <= top_of_zone_table; i++) {    /* Go through the whole zone table */
    show_zone = FALSE;
    overlap = FALSE;

    if (ZONE_FLAGGED(i, ZONE_GRID)) {           /* Is this zone 'on the grid' ?    */
      if (lolev == -1) {
        /* No range supplied, show all zones */
        show_zone = TRUE;
      } else if ((hilev == -1) && (lolev >= ZONE_MINLVL(i)) && (lolev <= ZONE_MAXLVL(i))) {
        /* Single number supplied, it's in this zone's range */
        show_zone = TRUE;
      } else if ((hilev != -1) && (lolev >= ZONE_MINLVL(i)) && (hilev <= ZONE_MAXLVL(i))) {
        /* Range supplied, it's completely within this zone's range (no overlap) */
        show_zone = TRUE;
      } else if ((hilev != -1) && ((lolev >= ZONE_MINLVL(i) && lolev <= ZONE_MAXLVL(i)) || (hilev <= ZONE_MAXLVL(i) && hilev >= ZONE_MINLVL(i)))) {
        /* Range supplied, it overlaps this zone's range */
        show_zone = TRUE;
        overlap = TRUE;
      } else if (ZONE_MAXLVL(i) < 0 && (lolev >= ZONE_MINLVL(i))) {
        /* Max level not set for this zone, but specified min in range */
        show_zone = TRUE;
      } else if (ZONE_MAXLVL(i) < 0 && (hilev >= ZONE_MINLVL(i))) {
        /* Max level not set for this zone, so just display it as red */
        show_zone = TRUE;
        overlap = TRUE;
      }
    }

    if (show_zone) {
      if (overlap) overlap_shown = TRUE;
      lev_set = get_zone_levels(i, lev_str);
      send_to_char(ch, "&B[&Y%3d&B] %s%-*s&n    %s%s&n\r\n", ++zcount, overlap ? QBRED : QBCYN,
                 count_color_chars(zone_table[i].name)+45, zone_table[i].name,
                 lev_set ? "&W" : "&n", lev_set ? lev_str : "&GAll Levels&n");
    }
  }
  send_to_char(ch, "&y----------------------------------------------------------------------&n\r\n"
                   "&m%d &warea%s found.&n\r\n",  zcount, zcount == 1 ? "" : "s");

  if (overlap_shown)
    send_to_char(ch, "&CAreas shown in &rred&C may have some creatures outside the specified range.&r\r\n");
}

ACMD(do_myinfo)
{

  send_to_char(ch, "                    Player Stats and Info Used by AI                      \r\n"
		   "&y----------------------------------------------------------------------&n\r\n"
                   " My current danger value is:                      %-20d                   \r\n"
		   " Total Damage Dealt since logging in:             %-20d                   \r\n"
                   " Total Damage Taken since logging in:             %-20d                   \r\n"
                   " Total Damage Dealt since player creation:        %-20d                   \r\n"
                   " Total Damage Taken since player creation:        %-20d                   \r\n"
		   , get_danger(ch), GET_DAMD(ch), GET_DAMT(ch), GET_TOT_DAMD(ch), GET_TOT_DAMT(ch));

}
