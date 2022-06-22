//
//   File: fight.c                                                         
//  Usage: Combat system                                                   
//                                                                         

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "constants.h"
#include "warzone.h"
#include "dg_scripts.h"
#include "arena.h"
#include "quest.h"
#include "logger.h"

// Structures 
struct char_data *combat_list = NULL;	// head of l-list of fighting chars 
struct char_data *next_combat_list = NULL;
struct follow_type *k;
// External structures 
extern struct message_list fight_messages[MAX_MESSAGES];

// External procedures 
char *fread_action(FILE *fl, int nr);
ACMD(do_flee);
ACMD(do_assist);
int backstab_mult(int level, int hitroll, int ac);
int thaco(int ch_class, int level);
int ok_damage_shopkeeper(struct char_data *ch, struct char_data *victim);
struct char_data *read_mobile(mob_vnum nr, int type);
void diag_char_to_char(struct char_data *i, struct char_data *ch);
void free_followers(struct char_data *ch);

void death_tick();
void arena_kill(struct char_data * ch, struct char_data * victim);
int DoWeaponProc(struct char_data *ch, struct char_data *vict, int dam);
void DoEquipmentProc(struct char_data *ch, struct char_data *vict);

void combat_intel(struct char_data *mob);

extern int rev_dir[];
extern struct index_data *obj_index;
extern struct room_data *world;

// local functions 
void perform_group_gain(struct char_data *ch, int base, struct char_data *victim);
void dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type);
void appear(struct char_data *ch);
void load_messages(void);
void free_messages(void);
void free_messages_type(struct msg_type *msg);
void check_killer(struct char_data *ch, struct char_data *vict);
void make_corpse(struct char_data *ch, struct char_data *killer);
void player_cpkd(struct char_data *ch, struct char_data *killer);
void player_npkd(struct char_data *ch, struct char_data *killer);
void make_char_corpse(struct char_data *ch, struct char_data *victim);
void change_alignment(struct char_data *ch, struct char_data *victim);
void death_cry(struct char_data *ch);
void raw_kill(struct char_data *ch, struct char_data *killer);
void die(struct char_data *ch, struct char_data *killer);
void group_gain(struct char_data *ch, struct char_data *victim);
void solo_gain(struct char_data *ch, struct char_data *victim);
char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural);
void perform_violence(void);
int compute_armor_class(struct char_data *ch);
int compute_thaco(struct char_data *ch, struct char_data *vict);
void find_victim(struct char_data *ch);
int get_danger(struct char_data *ch);
extern void make_blood(struct char_data *ch, struct char_data *victim);
int calc_parry(struct char_data *ch);
extern int exp_multiplier;

//EQ CONditions
void get_materials(struct char_data *ch, struct char_data *vict);
void get_materials_find(struct char_data *ch);
void damage_eq(struct char_data *vict, int dam);

//MOB INTELLIGENCE 
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

int form_mod_dam(struct char_data *ch, struct char_data *victim, int dam);

ACMD(do_plunder);

// Weapon attack texts 
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "&Rhits&n"},		// 0 
  {"sting", "&Rstings&n"},
  {"whip", "&Rwhips&n"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},	// 5 
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"},	// 10 
  {"pierce", "&Rpierces&n"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"stab", "stabs"}
};

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))

// The Fight related routines 

void appear(struct char_data *ch)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INVISIBLE | AFF_HIDE);

  if (GET_LEVEL(ch) < LVL_IMMORT)
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  else
    act("You feel a strange presence as $n appears, seemingly from nowhere.",
	FALSE, ch, 0, 0, TO_ROOM);
}


int compute_armor_class(struct char_data *ch)
{
  int armorclass = GET_AC(ch);

  if (AWAKE(ch))
    armorclass += dex_app[GET_DEX(ch)].defensive * 10;

  return (MAX(-100, armorclass));      // -100 is lowest 
}


void free_messages_type(struct msg_type *msg)
{
  if (msg->attacker_msg)	free(msg->attacker_msg);
  if (msg->victim_msg)		free(msg->victim_msg);
  if (msg->room_msg)		free(msg->room_msg);
}


void free_messages(void)
{
  int i;

  for (i = 0; i < MAX_MESSAGES; i++)
    while (fight_messages[i].msg) {
      struct message_type *former = fight_messages[i].msg;

      free_messages_type(&former->die_msg);
      free_messages_type(&former->miss_msg);
      free_messages_type(&former->hit_msg);
      free_messages_type(&former->god_msg);

      fight_messages[i].msg = fight_messages[i].msg->next;
      free(former);
    }
}


void load_messages(void)
{
  FILE *fl;
  int i, type;
  struct message_type *messages;
  char chk[128];

  if (!(fl = fopen(MESS_FILE, "r"))) {
    log("SYSERR: Error reading combat message file %s: %s", MESS_FILE, strerror(errno));
    exit(1);
  }

  for (i = 0; i < MAX_MESSAGES; i++) {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = NULL;
  }

  fgets(chk, 128, fl);
  while (!feof(fl) && (*chk == '\n' || *chk == '*'))
    fgets(chk, 128, fl);

  while (*chk == 'M') {
    fgets(chk, 128, fl);
    sscanf(chk, " %d\n", &type);
    for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	 (fight_messages[i].a_type); i++);
    if (i >= MAX_MESSAGES) {
      log("SYSERR: Too many combat messages.  Increase MAX_MESSAGES and recompile.");
      exit(1);
    }
    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_action(fl, i);
    messages->die_msg.victim_msg = fread_action(fl, i);
    messages->die_msg.room_msg = fread_action(fl, i);
    messages->miss_msg.attacker_msg = fread_action(fl, i);
    messages->miss_msg.victim_msg = fread_action(fl, i);
    messages->miss_msg.room_msg = fread_action(fl, i);
    messages->hit_msg.attacker_msg = fread_action(fl, i);
    messages->hit_msg.victim_msg = fread_action(fl, i);
    messages->hit_msg.room_msg = fread_action(fl, i);
    messages->god_msg.attacker_msg = fread_action(fl, i);
    messages->god_msg.victim_msg = fread_action(fl, i);
    messages->god_msg.room_msg = fread_action(fl, i);
    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);
  }

  fclose(fl);
}


void update_pos(struct char_data *victim)
{
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
    return;
  else if (GET_HIT(victim) > 0)
    GET_POS(victim) = POS_STANDING;
  else if (GET_HIT(victim) <= -11)
    GET_POS(victim) = POS_DEAD;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POS_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POS_INCAP;
  else
    GET_POS(victim) = POS_STUNNED;
}


void check_killer(struct char_data *ch, struct char_data *vict)
{
  if (PLR_FLAGGED(vict, PLR_KILLER) || PLR_FLAGGED(vict, PLR_THIEF))
    return;
  if (PLR_FLAGGED(ch, PLR_KILLER) || IS_NPC(ch) || IS_NPC(vict) || ch == vict)
    return;

  SET_BIT_AR(PLR_FLAGS(ch), PLR_KILLER);
  send_to_char(ch, "If you want to be a PLAYER KILLER, so be it...\r\n");
  nmudlog(INFO_LOG, LVL_GOD, TRUE, "KILLER FLAG set on %s for initiating attack on %s at %s", GET_NAME(ch), GET_NAME(vict), world[IN_ROOM(vict)].name);
//  mudlog(BRF, LVL_IMMORT, TRUE, "PC Killer bit set on %s for initiating attack on %s at %s.",
//	    GET_NAME(ch), GET_NAME(vict), world[IN_ROOM(vict)].name);
}


// start one char fighting another (yes, it is horrible, I know... )  
void set_fighting(struct char_data *ch, struct char_data *vict)
{
  if (ch == vict)
    return;

  if (FIGHTING(ch)) {
    core_dump();
    return;
  }

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (AFF_FLAGGED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP);

  if(PLR_FLAGGED(ch, PLR_GEMMINE)) 
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GEMMINE);

  if(PLR_FLAGGED(ch, PLR_OREMINE))
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_OREMINE);
 

  FIGHTING(ch) = vict;
  GET_POS(ch) = POS_FIGHTING;
  if (!CONFIG_PK_ALLOWED)
    check_killer(ch, vict);
}



// remove a char from the list of fighting chars 
void stop_fighting(struct char_data *ch)
{
  struct char_data *temp;

  if (ch == next_combat_list)
    next_combat_list = ch->next_fighting;

  REMOVE_FROM_LIST(ch, combat_list, next_fighting);
  ch->next_fighting = NULL;
  FIGHTING(ch) = NULL;
  GET_POS(ch) = POS_STANDING;
  update_pos(ch);
}



void make_corpse(struct char_data *ch, struct char_data *killer)
{
  char buf2[MAX_NAME_LENGTH + 64];
  struct obj_data *corpse, *o, *pot;
  struct obj_data *money;
  int i, death_room=6305;
  long int corpse_room, p=7200, onum;
  room_rnum location;

  corpse_room = 106;
  location = real_room(corpse_room);
  
  if (IS_NPC(ch))
  {
  stop_fighting(ch);
  stop_fighting(killer);
  corpse = create_obj();

  corpse->item_number = NOTHING;
  IN_ROOM(corpse) = NOWHERE;
  corpse->name = strdup("corpse");

  snprintf(buf2, sizeof(buf2), "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = strdup(buf2);

  snprintf(buf2, sizeof(buf2), "the corpse of %s", GET_NAME(ch));
  corpse->short_description = strdup(buf2);

  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
  SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE);
  SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);

  if (MOB_FLAGGED(ch, MOB_SCALES))
    SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_SCALEABLE);
  if (MOB_FLAGGED(ch, MOB_SKINS))
    SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_SKINNABLE);
 
  if (GET_PROF(killer, PROF_SKINNING) > rand_number(0,5000))
  {
    if (rand_number(1,10) > 5)
      SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_SKINNABLE);
    else
      SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_SCALEABLE);
  }

  GET_OBJ_VAL(corpse, 0) = 0;	// You can't store stuff in a corpse 
  GET_OBJ_VAL(corpse, 3) = 1;	// corpse identifier 
  GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  GET_OBJ_COND(corpse) = 0;
  if (IS_NPC(ch))
    GET_OBJ_TIMER(corpse) = CONFIG_MAX_NPC_CORPSE_TIME;
  else
    GET_OBJ_TIMER(corpse) = CONFIG_MAX_PC_CORPSE_TIME;
   
   SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_UNIQUE_SAVE);

  // transfer character's inventory to the corpse 
  corpse->contains = ch->carrying;
  for (o = corpse->contains; o != NULL; o = o->next_content)
    o->in_obj = corpse;
  object_list_new_owner(corpse, NULL);

  // transfer character's equipment to the corpse 
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i)){
      remove_otrigger(GET_EQ(ch, i), ch);
      obj_to_obj(unequip_char(ch, i), corpse);
    }
LoadPotions(corpse);
//  if (rand_number(1,10) > 8)
//    for (i = 0;i < 6; i++)
//      if (rand_number(1,10) > 7)
//      {    
//       onum = real_object(p);
//       pot = read_object(onum, REAL);
//       obj_to_obj(pot, corpse);
//      }
  // transfer gold 
  if (GET_GOLD(ch) > 0) {
    //
    // following 'if' clause added to fix gold duplication loophole
    // The above line apparently refers to the old "partially log in,
    // kill the game character, then finish login sequence" duping
    // bug. The duplication has been fixed (knock on wood) but the
    // test below shall live on, for a while. -gg 3/3/2002
    //
    if (IS_NPC(ch) || ch->desc) {
      money = create_money(GET_GOLD(ch));
      obj_to_obj(money, corpse);
    }
    GET_GOLD(ch) = 0;
  }
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;
  obj_to_room(corpse, IN_ROOM(ch));
  extract_char(ch);
  }
  else
  {
   death_room = GET_ALTAR(ch);
   location = real_room(death_room);
   char_from_room(ch);
   char_to_room(ch, location);
   GET_HIT(ch) = 5;
   GET_POS(ch) = POS_RESTING;
  }
}

void player_cpkd(struct char_data *ch, struct char_data *killer)
{
 char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
 struct obj_data *corpse, *obj, *next_obj;
 struct char_data *mob;
 int i, death_room=10;
 room_rnum location, dead_room;
 mob_rnum r_num;

 dead_room = IN_ROOM(ch);
 location = real_room(death_room);
 r_num = real_mobile(2);

  stop_fighting(ch);
  stop_fighting(killer);

 if (GET_LEVEL(killer) > GET_LEVEL(ch))
    GET_PKBLOOD(killer) += (GET_LEVEL(killer) - GET_LEVEL(ch)) * 2;
 else
    GET_PKBLOOD(killer) += rand_number(1,10);

  corpse = create_obj();

  corpse->item_number = NOTHING;
  IN_ROOM(corpse) = NOWHERE;
  corpse->name = strdup("corpse");

  snprintf(buf2, sizeof(buf2), "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = strdup(buf2);

  snprintf(buf2, sizeof(buf2), "the corpse of %s", GET_NAME(ch));
  corpse->short_description = strdup(buf2);

  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
  SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE);
  SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);
  GET_OBJ_VAL(corpse, 0) = 0;
  GET_OBJ_VAL(corpse, 3) = 1;
  GET_OBJ_WEIGHT(corpse) = (GET_WEIGHT(ch) + IS_CARRYING_W(ch)) * 5;
  GET_OBJ_COND(corpse) = 0;
  GET_OBJ_TIMER(corpse) = CONFIG_MAX_PC_CORPSE_TIME;

// transfer character's inventory to the corpse
  for (obj = ch->carrying; obj; obj = next_obj) 
  {
    next_obj = obj->next_content;
  
    if (rand_number(1, 50) <= 15)
         obj_to_obj(obj, corpse);
  }
        

//   transfer character's equipment to the corpse
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && rand_number(1, 50) <= 25)
      if (GET_OBJ_BOUND_ID(GET_EQ(ch,i)) != NOBODY)
        GET_OBJ_BOUND_ID(GET_EQ(ch,i)) = NOBODY;
      obj_to_obj(unequip_char(ch, i), corpse);

//  transfer gold
  if (GET_GOLD(ch) > 0) {
//    
//      following 'if' clause added to fix gold duplication loophole
//      The above line apparently refers to the old "partially log in,
//      kill the game character, then finish login sequence" duping
//      bug. The duplication has been fixed (knock on wood) but the
//      test below shall live on, for a while. -gg 3/3/2002
//     
    GET_GOLD(ch) = 0;
  }
  obj_to_room(corpse, IN_ROOM(ch));

//  mob = read_mobile(r_num, REAL);


  send_to_char(ch, "\r\n&CYou get a strange feeling as your soul is ripped from your body.&n\r\n");
  snprintf(buf2, sizeof(buf2), "The undead corpse of %s is here rotting away.\r\n", GET_NAME(ch));
  snprintf(buf, sizeof(buf), "The undead corpse of %s", GET_NAME(ch));

   death_room = GET_ALTAR(ch);
   location = real_room(death_room);
   char_from_room(ch);
   char_to_room(ch, location);
   GET_HIT(ch) = 5;
   GET_POS(ch) = POS_RESTING;
} 
/*      ch, "You feel yourself being pulled into a bright light...\r\n");
        char_from_room(ch);
        if (GET_LEVEL(ch) <= 10)
         char_to_room(ch, real_room(rand_number(AFTERLIFE_MIN, AFTERLIFE_MAX)));
        else if (GET_LEVEL(ch) > 10 && GET_LEVEL(ch) <= 20)
         char_to_room(ch, real_room(rand_number(AFTERLIFE_MIN2, AFTERLIFE_MAX2)));
        else
         char_to_room(ch, real_room(rand_number(AFTERLIFE_MIN3, AFTERLIFE_MAX3)));
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        SET_BIT_AR(PLR_FLAGS(ch), PLR_AFTERLIFE);
        update_pos(ch);
        look_at_room(ch, 0);

  } else {

    send_to_char(ch, "You feel yourself being pulled into a bright light...\r\n");
        char_from_room(ch);
        if (GET_LEVEL(ch) <= 10)
         char_to_room(ch, real_room(rand_number(AFTERLIFE_MIN, AFTERLIFE_MAX)));
        else if (GET_LEVEL(ch) > 10 && GET_LEVEL(ch) <= 20)
         char_to_room(ch, real_room(rand_number(AFTERLIFE_MIN2, AFTERLIFE_MAX2)));
        else
         char_to_room(ch, real_room(rand_number(AFTERLIFE_MIN3, AFTERLIFE_MAX3)));
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        SET_BIT_AR(PLR_FLAGS(ch), PLR_AFTERLIFE);
        update_pos(ch);
        look_at_room(ch, 0);
  
  }*/

void player_npkd(struct char_data *ch, struct char_data *killer)
{
  
 char buf2[MAX_INPUT_LENGTH];
 struct obj_data *corpse;
 int death_room=0;
 room_rnum location, dead_room;
 mob_rnum r_num;

 dead_room = IN_ROOM(ch);
 location = real_room(death_room);
 r_num = real_mobile(2);

  stop_fighting(killer);
  stop_fighting(ch);
  corpse = create_obj();

  corpse->item_number = NOTHING;
  IN_ROOM(corpse) = NOWHERE;
  corpse->name = strdup("corpse");

  snprintf(buf2, sizeof(buf2), "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = strdup(buf2);

  snprintf(buf2, sizeof(buf2), "the corpse of %s", GET_NAME(ch));
  corpse->short_description = strdup(buf2);

  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
  SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE);
  SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);
  GET_OBJ_VAL(corpse, 0) = 0;
  GET_OBJ_VAL(corpse, 3) = 1;
  GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  GET_OBJ_COND(corpse) = 0;
  GET_OBJ_TIMER(corpse) = CONFIG_MAX_PC_CORPSE_TIME;
  obj_to_room(corpse, IN_ROOM(ch));

   death_room = GET_ALTAR(ch);
   location = real_room(death_room);
   char_from_room(ch);
   char_to_room(ch, location);
   GET_HIT(ch) = 5;
   GET_POS(ch) = POS_RESTING;
   return;
}


// When ch kills victim 
void change_alignment(struct char_data *ch, struct char_data *victim)
{
  //
  // new alignment change algorithm: if you kill a monster with alignment A,
  // you move 1/16th of the way to having alignment -A.  Simple and fast.
  //
  GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) / 200;
}



void death_cry(struct char_data *ch)
{
  int door;

  act("&RYour blood freezes as you hear $n's death cry.&n", FALSE, ch, 0, 0, TO_ROOM);

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (CAN_GO(ch, door))
      send_to_room(world[IN_ROOM(ch)].dir_option[door]->to_room, "&rYour blood freezes as you hear someone's death cry.&n\r\n");
  
//    ADVANCE PLAYER DEATH COUNT
    if (!IS_NPC(ch))
    GET_DEATHS(ch)++;
}



void raw_kill(struct char_data *ch, struct char_data *killer)
{

  if (FIGHTING(ch))
    stop_fighting(ch);

//  if (FIGHTING(killer))
//    stop_fighting(killer);

  while (ch->affected)
    affect_remove(ch, ch->affected);

  /* To make ordinary commands work in scripts.  welcor*/  
  GET_POS(ch) = POS_STANDING; 
  
  if (killer) {
    if (!death_mtrigger(ch, killer))
      goto killp;

//  death_cry(ch);
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_TAKE_SMOKE);
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_TAKE_BLAZE);
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_TAKE_CANN);
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_TAKE_SATI);

  if (IS_NPC(ch))
  {
      GET_POS(ch) = POS_DEAD;
      death_cry(ch);
      if (killer)
       autoquest_trigger_check(killer, ch, NULL, AQ_MOB_KILL);
      make_corpse(ch, killer);
      if (killer) {
       autoquest_trigger_check(killer, NULL, NULL, AQ_MOB_SAVE);
       autoquest_trigger_check(killer, NULL, NULL, AQ_ROOM_CLEAR);
       }
  }
  else
  {
      killp:
      GET_POS(ch) = POS_DEAD;
      GET_HIT(ch) = 0;
   
      if(GET_DYING_TIME(ch) == 0)
      {
        GET_DYING_TIME(ch) = -1;
        death_cry(ch);
        make_corpse(ch, killer);
     //   GET_DYING_TIME(ch) = 10;
      }
  }
//  if (IS_NPC(ch))
//   extract_char(ch);
 }
}
void death_tick()
{
 struct descriptor_data *d;
 
   for (d = descriptor_list;d;d = d->next)
   {
    if (d->connected) continue;
    if (IS_NPC(d->character)) continue;

     if (GET_DYING_TIME(d->character) != 0 && PLR_FLAGGED(d->character, PLR_DYING))
     {  
       GET_DYING_TIME(d->character) = GET_DYING_TIME(d->character) - 1;

       switch(GET_DYING_TIME(d->character))
       {
	case 4:
	   send_to_char(d->character, "&RYou feel the cold grip of death begin to tighten it's grip about your entire body.\r\n"
				      "You muscles begin to stiffen and your vision begins to blur slowly until it becomes\r\n"
				      "very dark.\r\n");
	   break;

	case 2:
	   send_to_char(d->character, "&WA strange looking creature appears in a puff of smoke. It surveys the area and looks\r\n"
				      "at your corpse happily. A few sprinkles of dust and liquid are laid across your forehead.\r\n"
				      "Quickly, the creature disappears.");
           break;

	default:

	   break;
       }

     }
   
    if (GET_DYING_TIME(d->character) == 0)
    {
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_DYING);  
      die(d->character, d->killer);
    }
   }
}

void die(struct char_data *ch, struct char_data *killer)
{
  int dam=0, w_type;
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);

  if (zone_table[world[ch->in_room].zone].number == WZ_ZONE && !IS_NPC(ch)) {
    room_rnum location;
    int room;
    char buf[MAX_INPUT_LENGTH]; 

    room = 106;
    location = real_room(room);

    stop_follower(ch);
    GET_HIT(ch) = 5;
    update_pos(ch);
    char_from_room(ch);
    char_to_room(ch, location);
    look_at_room(ch, 0);
    act("$n disappears in a puff of smoke.", FALSE, ch, 0, 0, TO_ROOM);
   sprintf(buf, "%s has been defeated by in their clanwar.", GET_NAME(ch));
   log("%s", buf); 
  } else { 
//  gain_exp(ch, -(GET_EXP(ch) / 1000));
  if (!IS_NPC(ch))
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_KILLER | PLR_THIEF);
 
  if(!IS_NPC(ch) && IS_KNIGHT(ch) && GET_SKILL(ch, SKILL_DEATHWISH) > rand_number(0,100)) {
    if (wielded && GET_OBJ_TYPE(wielded) == (ITEM_WEAPON || ITEM_BOW || ITEM_CROSSBOW || ITEM_SLING)) {
      w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
      // Add weapon-based damage if a weapon is being wielded
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
    } 
    dam += GET_DAMROLL(ch);
    damage(ch, killer, dam/2, w_type);
  } 
    raw_kill(ch, killer);
  }
}

void get_materials_find(struct char_data *ch)
{
   int ranfind=0,rannum=0,ranmat=0;

   ranfind = rand_number(1,2);
   rannum = rand_number(1,8);
   ranmat = rand_number(1,5);

   send_to_char(ch, "\r\n");

   if (ranfind == 1)
   {
     switch(rannum)
     {
       case 1:
        GET_ADAMANTITE_ORE(ch) += ranmat;
        send_to_char(ch, "&mYou find %d pieces of adamantite ore.\r\n", ranmat);
        break;
       case 2:
        GET_BRONZE_ORE(ch)+= ranmat;
        send_to_char(ch, "&rYou find %d pieces of bronze ore.\r\n", ranmat);
        break;
       case 3:
        GET_COPPER_ORE(ch)+= ranmat;
        send_to_char(ch, "&yYou find %d pieces of copper ore.\r\n", ranmat);
        break;
       case 4:
        GET_GOLD_ORE(ch)+= ranmat;
        send_to_char(ch, "&YYou find %d pieces of gold ore.\r\n", ranmat);
        break;
       case 5:
        GET_IRON_ORE(ch)+= ranmat;
        send_to_char(ch, "&cYou find %d pieces of iron ore.\r\n", ranmat);
        break;
       case 6:
        GET_MITHRIL_ORE(ch)+= ranmat;
        send_to_char(ch, "&WYou find %d pieces of mithril ore.\r\n", ranmat);
        break;
       case 7:
        GET_SILVER_ORE(ch)+= ranmat;
        send_to_char(ch, "&wYou find %d pieces of silver ore.\r\n", ranmat);
        break;
       case 8:
        GET_TITANIUM_ORE(ch)+= ranmat;
        send_to_char(ch, "&BYou find %d pieces of titanium ore.\r\n", ranmat);
        break;
       default:
         break;
     }
   }

   if (ranfind ==2)
   {
    switch(rannum)
    {
       case 1:
        GET_CRYSTAL(ch)+= ranmat;
        send_to_char(ch, "&wYou find %d crystal gems.\r\n", ranmat);
         break;
       case 2:
        GET_DIAMOND(ch)+= ranmat;
        send_to_char(ch, "&WYou find %d diamond gems.\r\n", ranmat);
         break;
       case 3:
        GET_EMERALD(ch)+= ranmat;
        send_to_char(ch, "&GYou find %d emerald gems.\r\n", ranmat);
         break;
       case 4:
        GET_GARNET(ch)+= ranmat,
        send_to_char(ch, "&rYou find %d garnet gems.\r\n", ranmat);
         break;
       case 5:
        GET_JADE(ch)+= ranmat;
        send_to_char(ch, "&gYou find %d jade gems.\r\n", ranmat);
         break;
       case 6:
        GET_RUBY(ch)+= ranmat;
        send_to_char(ch, "&RYou find %d ruby gems.\r\n", ranmat);
         break;
       case 7:
        GET_SAPPHIRE(ch)+= ranmat;
        send_to_char(ch, "&CYou find %d sapphire gems.\r\n", ranmat);
         break;
       case 8:
        GET_TOPAZ(ch)+= ranmat;
        send_to_char(ch, "&BYou find %d topaz gems.\r\n", ranmat);
         break;
        default:
         break;
     }
   }
  send_to_char(ch, "&n");
  return;
}

void get_materials(struct char_data *ch, struct char_data *vict)
{
   int ranore=0,ranmat=0, rangem=0;
 
   ranore = rand_number(1,8);
   rangem = rand_number(1,14);
   ranmat = rand_number(1,5);

   send_to_char(ch, "\r\n");

   if (MOB_FLAGGED(vict, MOB_METAL_MINER))
   {
     switch(ranore)
     {
       case 1:
        GET_ADAMANTITE_ORE(ch) += ranmat;
	send_to_char(ch, "&mYou receive %d pieces of adamantite ore.\r\n", ranmat);
        break;
       case 2:
	GET_BRONZE_ORE(ch)+= ranmat;
        send_to_char(ch, "&rYou receive %d pieces of bronze ore.\r\n", ranmat);
        break;
       case 3:
   	GET_COPPER_ORE(ch)+= ranmat;
        send_to_char(ch, "&yYou receive %d pieces of copper ore.\r\n", ranmat);
        break;
       case 4:
   	GET_GOLD_ORE(ch)+= ranmat;
        send_to_char(ch, "&YYou receive %d pieces of gold ore.\r\n", ranmat);
        break;
       case 5:
   	GET_IRON_ORE(ch)+= ranmat;
        send_to_char(ch, "&cYou receive %d pieces of iron ore.\r\n", ranmat);
        break;
       case 6:
   	GET_MITHRIL_ORE(ch)+= ranmat;
        send_to_char(ch, "&WYou receive %d pieces of mithril ore.\r\n", ranmat);
        break;
       case 7:
   	GET_SILVER_ORE(ch)+= ranmat;
        send_to_char(ch, "&wYou receive %d pieces of silver ore.\r\n", ranmat);
        break;
       case 8:
   	GET_TITANIUM_ORE(ch)+= ranmat;
        send_to_char(ch, "&BYou receive %d pieces of titanium ore.\r\n", ranmat);
        break;
       default:
         break;
     }
   }
   
   if (MOB_FLAGGED(vict, MOB_GEM_MINER))
   {
    switch(rangem)
    {
       case 1:
       case 9:
       case 11:
       case 13:
        GET_CRYSTAL(ch)+= ranmat;
        send_to_char(ch, "&wYou receive %d crystal gems.\r\n", ranmat);
         break;
       case 2:
       case 10:
       case 12:
       case 14:
	GET_DIAMOND(ch)+= ranmat;
        send_to_char(ch, "&WYou receive %d diamond gems.\r\n", ranmat);
         break;
       case 3:
	GET_EMERALD(ch)+= ranmat;
        send_to_char(ch, "&GYou receive %d emerald gems.\r\n", ranmat);
         break;
       case 4:
	GET_GARNET(ch)+= ranmat,
        send_to_char(ch, "&rYou receive %d garnet gems.\r\n", ranmat);
         break;
       case 5:
        GET_JADE(ch)+= ranmat;
        send_to_char(ch, "&gYou receive %d jade gems.\r\n", ranmat);
         break;
       case 6:
	GET_RUBY(ch)+= ranmat;
        send_to_char(ch, "&RYou receive %d ruby gems.\r\n", ranmat);
         break;
       case 7:
	GET_SAPPHIRE(ch)+= ranmat;
        send_to_char(ch, "&CYou receive %d sapphire gems.\r\n", ranmat);
         break;
       case 8:
	GET_TOPAZ(ch)+= ranmat;
        send_to_char(ch, "&BYou receive %d topaz gems.\r\n", ranmat);
         break;
        default:
         break;
     }
   }    
  send_to_char(ch, "&n");
  return;
}

void perform_group_gain(struct char_data *ch, int base, struct char_data *victim)
{
  int share;

  share = MIN(CONFIG_MAX_EXP_GAIN, base);

  if (share > 1)
    send_to_char(ch, "&BYou receive your share of experience -- &R%d &Bpoints.&n\r\n", (share*exp_multiplier));
  else
    send_to_char(ch, "&BYou receive your share of experience -- &R1&B measly little point!&n\r\n");

  gain_exp(ch, share);
  change_alignment(ch, victim);
}


void group_gain(struct char_data *ch, struct char_data *victim)
{
  int tot_members, base, tot_gain;
  struct char_data *k;
  struct follow_type *f;

  if (!(k = ch->master))
    k = ch;

  if (AFF_FLAGGED(k, AFF_GROUP) && (IN_ROOM(k) == IN_ROOM(ch)))
    tot_members = 1;
  else
    tot_members = 0;

  for (f = k->followers; f; f = f->next)
    if (AFF_FLAGGED(f->follower, AFF_GROUP) && IN_ROOM(f->follower) == IN_ROOM(ch))
      tot_members++;

  // round up to the next highest tot_members 
  tot_gain = GET_EXP(victim) + tot_members - 1;
  
  if (tot_members > 3)
    tot_members = 2;

  // prevent illegal xp creation when killing players 
  if (!IS_NPC(victim))
    tot_gain = MIN(CONFIG_MAX_EXP_LOSS * 2 / 3, tot_gain);

  if (tot_members > 0)
    base = MAX(1, (tot_gain * tot_members));
  else
    base = tot_gain;

  
  if (AFF_FLAGGED(k, AFF_GROUP) && IN_ROOM(k) == IN_ROOM(ch))
     perform_group_gain(k, base, victim);

  for (f = k->followers; f; f = f->next)
    if (AFF_FLAGGED(f->follower, AFF_GROUP) && IN_ROOM(f->follower) == IN_ROOM(ch))
      perform_group_gain(f->follower, base, victim);
   
}


void solo_gain(struct char_data *ch, struct char_data *victim)
{
  int exp=0, lvl=0, plvl=0;
  
  lvl = GET_LEVEL(ch);

  switch(lvl)
  {
     case 1:
        plvl=500;
        break;
     case 2:
        plvl=1000;
        break;
     case 3:
        plvl=5000;
        break;
     case 4:
        plvl=15000;
        break;
     case 5:
        plvl=30000;
        break;
     case 6:
        plvl=50000;
        break;
     case 7:
        plvl=60000;
        break;
     case 8:
        plvl=70000;
        break;
     case 9:
        plvl=80000;
        break;
     case 10:
        plvl=90000;
        break;
     case 11:
        plvl=95000;
        break;
     case 12:
        plvl=100000;
        break;
     case 13:
        plvl=110000;
        break;
     case 14:
        plvl=125000;
        break;
     case 15:
        plvl=150000;
        break;
     case 16:
        plvl=175000;
        break;
     case 17:
        plvl=200000;
        break;
     case 18:
        plvl=225000;
        break;
     case 19:
        plvl=275000;
        break;
     case 20:
        plvl=300000;
        break;
     case 21:
        plvl=325000;
        break;
     case 22:
        plvl=350000;
        break;
     case 23:
        plvl=400000;
        break;
     case 24:
        plvl=425000;
        break;
     case 25:
        plvl=475000;
        break;
     case 26:
        plvl=550000;
        break;
     case 27:
        plvl=600000;
        break;
     case 28:
        plvl=700000;
        break;
     case 29:
        plvl=800000;
        break;
     case 30:
        plvl=875000;
        break;
     default:
        plvl=CONFIG_MAX_EXP_GAIN;
        break;
  }

  exp = MIN(plvl, GET_EXP(victim));

  if (exp < 0)
     exp = 0;

  if (exp > 1)
    send_to_char(ch, "&BYou receive&R %d &Bexperience points.&n\r\n", (exp*exp_multiplier));
  else
    send_to_char(ch, "&RYou receive one lousy experience point.&n\r\n");

  gain_exp(ch, exp);
  change_alignment(ch, victim);
}


char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural)
{
  static char buf[256];
  char *cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
	for (; *weapon_plural; *(cp++) = *(weapon_plural++));
	break;
      case 'w':
	for (; *weapon_singular; *(cp++) = *(weapon_singular++));
	break;
      default:
	*(cp++) = '#';
	break;
      }
    } else
      *(cp++) = *str;

    *cp = 0;
  }				// For 

  return (buf);
}

struct dam_weapon_type {
    const char *to_room;
    const char *to_char;
    const char *to_victim;
};

// message for doing damage with a weapon 
void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		      int w_type)
{
  char *buf, *buf2;
  int msgnum;

  struct dam_weapon_type dam_weapons1[] = {

    // use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") 
    {
      "&G$n tries to &R#w&G $N, but $E dodges $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $E dodges your attack.&n",
      "&G$n tries to &R#w&G you, but you dodge $S attack.&n"
    },

    {
      "&G$n tries to &R#w&G $N, but $e parries $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $e parries your attack.&n",
      "&G$n tries to &R#w&G you, but you parry $S attack.&n"
    },

    {
      "&G$n tries to &R#w&G $N, but $e evades $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $e evades your attack.&n",
      "&G$n tries to &R#w&G you, but you evade $S attack.&n"
    },

    {
      "&G$n tries to &R#w&G $N, but misses.&n",	// 0: 0     
      "&GYou try to &R#w&G $N, but miss.&n",
      "&G$n tries to &R#w&G you, but misses.&n"
    },

    {
      "&G$n &Cbarely touches &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Cbarely touch &G$N with your deadly &R#w!!&n",
      "&G$n &Cbarely touches &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Cscratches &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Cscratch &G$N with your deadly &R#w!!&n",
      "&G$n &Cscratches &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Cbruises &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Cbruise &G$N with your deadly &R#w!!&n",
      "&G$n &Cbruises &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Chits &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Chit &G$N with your deadly &R#w!!&n",
      "&G$n &Chits &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Cinjures &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Cinjure &G$N with your deadly &R#w!!&n",
      "&G$n &Cinjures &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Ywounds &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Ywound &G$N with your deadly &R#w!!&n",
      "&G$n &Ywounds &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Ysmites &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Ysmite &G$N with your deadly &R#w!!&n",
      "&G$n &Ysmites &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Ymassacres &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Ymassacre &G$N with your deadly &R#w!!&n",
      "&G$n &Ymassacres &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Ydecimates &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Ydecimate &G$N with your deadly &R#w!!&n",
      "&G$n &Ydecimates &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Ymaims &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Ymaim &G$N with your deadly &R#w!!&n",
      "&G$n &Ymaims &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Mmauls &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Mmaul &G$N with your deadly &R#w!!&n",
      "&G$n &Mmauls &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Mrends &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Mrend &G$N with your deadly &R#w!!&n",
      "&G$n &Mrends &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Mmutilates &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Mmutilate &G$N with your deadly &R#w!!&n",
      "&G$n &Mmutilates &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Mpulverizes &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Mpulverize &G$N with your deadly &R#w!!&n",
      "&G$n &Mpulverizes &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Mdemolishes &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Mdemolish &G$N with your deadly &R#w!!&n",
      "&G$n &Mdemolishes &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Rmangles &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Rmangle &G$N with your deadly &R#w!!&n",
      "&G$n &Rmangles &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Robliterates &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Robliterate &G$N with your deadly &R#w!!&n",
      "&G$n &Robliterates &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Rannihilates &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Rannihilate &G$N with your deadly &R#w!!&n",
      "&G$n &Ranihilates &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Rbrutally maims &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Rbrutally maims &G$N with your deadly &R#w!!&n",
      "&G$n &Rbrutally maims &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Rvisciously mauls &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Rvisciously mauls &G$N with your deadly &R#w!!&n",
      "&G$n &Rvisciously mauls &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n &Rhorribly rend &G$N with $s deadly &R#w!!&n",    // 8: > 23
      "&GYou &Rhorribly rend &G$N with your deadly &R#w!!&n",
      "&G$n &Rhooribly rendds &Gyou with $s deadly &R#w!!&n"
    },

    {
      "&G$n tries to &R#w&G $N, but $E defends $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $E defends your attack.&n",
      "&G$n tries to &R#w&G you, but you defend $S attack.&n"
    },

    {
      "&G$n tries to &R#w&G $N, but $E counters $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $E counters your attack.&n",
      "&G$n tries to &R#w&G you, but you counter $S attack.&n"
    }
  };

struct dam_weapon_type dam_weapons2[] = {
    {
      "&G$n tries to &R#w&G $N, but $E dodges $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $E dodges your attack.&n",
      "&G$n tries to &R#w&G you, but you dodge $S attack.&n"
    },

    {
      "&G$n tries to &R#w&G $N, but $e parries $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $e parries your attack.&n",
      "&G$n tries to &R#w&G you, but you parry $S attack.&n"
    },

    {
      "&G$n tries to &R#w&G $N, but $e evades $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $e evades your attack.&n",
      "&G$n tries to &R#w&G you, but you evade $S attack.&n"
    },

{
 "&G$n's #w &Wmisses &G$N.&n",
 "&GYour #w &Wmisses &G$N.&n",
 "&G$n's #w &Wmisses &Gyou.&n"
},

{
 "&G$n's #w &Cbarely touches &G$N.&n",
 "&GYour #w &Cbarely touches &G$N.&n",
 "&G$n's #w &Cbarely touches &Gyou.&n"
},

{
 "&G$n's #w &Cscratches &G$N.&n",
 "&GYour #w &Cscratches &G$N.&n",
 "&G$n's #w &Cscratches &Gyou.&n"
},

{
 "&G$n's #w &Cbruises &G$N.&n",
 "&GYour #w &Cbruises &G$N.&n",
 "&G$n's #w &Cbruises &Gyou.&n"
},

{
 "&G$n's #w &Chits &G$N.&n",
 "&GYour #w &Chits &G$N.&n",
 "&G$n's #w &Chits &Gyou.&n"
},

{
 "&G$n's #w &Cinjures &G$N.&n",
 "&GYour #w &Cinjures &G$N.&n",
 "&G$n's #w &Cinjures &Gyou.&n"
},

{
 "&G$n's #w &Ywounds &G$N.&n",
 "&GYour #w &Ywounds &G$N.&n",
 "&G$n's #w &Ywounds &Gyou.&n"
},

{
 "&G$n's #w &Ysmites &G$N.&n",
 "&GYour #w &Ysmites &G$N.&n",
 "&G$n's #w &Ysmites &Gyou.&n"
},

{
 "&G$n's #w &Ymassacres &G$N.&n",
 "&GYour #w &Ymassacres &G$N.&n",
 "&G$n's #w &Ymassacres &Gyou.&n"
},

{
 "&G$n's #w &Ydecimates &G$N.&n",
 "&GYour #w &Ydecimates &G$N.&n",
 "&G$n's #w &Ydecimates &Gyou.&n"
},

{
 "&G$n's #w &Ymaims &G$N.&n",
 "&GYour #w &Ymaims &G$N.&n",
 "&G$n's #w &Ymaims &Gyou.&n"
},

{
 "&G$n's #w &Mmauls &G$N.&n",
 "&GYour #w &Mmauls &G$N.&n",
 "&G$n's #w &Mmauls &Gyou.&n"
},

{
 "&G$n's #w &Mrends &G$N.&n",
 "&GYour #w &Mrends &G$N.&n",
 "&G$n's #w &Mrends &Gyou.&n"
},

{
 "&G$n's #w &Mmutilates &G$N.&n",
 "&GYour #w &Mmutilates &G$N.&n",
 "&G$n's #w &Mmutilates &Gyou.&n"
},

{
 "&G$n's #w &Mpulverizes &G$N.&n",
 "&GYour #w &Mpulverizes &G$N.&n",
 "&G$n's #w &Mpulverizes &Gyou.&n"
},

{
 "&G$n's #w &Mdemolishes &G$N.&n",
 "&GYour #w &Mdemolishes &G$N.&n",
 "&G$n's #w &Mdemolishes &Gyou.&n"
},

{
 "&G$n's #w &Rmangles &G$N.&n",
 "&GYour #w &Rmangles &G$N.&n",
 "&G$n's #w &Rmangles &Gyou.&n"
},

{
 "&G$n's #w &Robliterates &G$N.&n",
 "&GYour #w &Robliterates &G$N.&n",
 "&G$n's #w &Robliterates &Gyou.&n"
},

{
 "&G$n's #w &Rannihilates &G$N.&n",
 "&GYour #w &Rannihilates &G$N.&n",
 "&G$n's #w &Rannihilates &Gyou.&n"
},

{
 "&G$n's #w &Rbrutally maims &G$N.&n",
 "&GYour #w &Rbrutally maims &G$N.&n",
 "&G$n's #w &Rbrutally maims &Gyou.&n"
},

{
 "&G$n's #w &Rvisciously mauls &G$N.&n",
 "&GYour #w &Rvisciously mauls &G$N.&n",
 "&G$n's #w &Rvisciously mauls &Gyou.&n"
},

{
 "&G$n's #w &Rhorribly rends &G$N.&n",
 "&GYour #w &Rhorribly rends &G$N.&n",
 "&G$n's #w &Rhorribly rends &Gyou.&n"
},
    {
      "&G$n tries to &R#w&G $N, but $E defends $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $E defends your attack.&n",
      "&G$n tries to &R#w&G you, but you defend $S attack.&n"
    },

    {
      "&G$n tries to &R#w&G $N, but $E counters $S attack.&n", // 0: 0
      "&GYou try to &R#w&G $N, but $E counters your attack.&n",
      "&G$n tries to &R#w&G you, but you counter $S attack.&n"
    }
};

  w_type -= TYPE_HIT;		// Change to base of table with text 
  
  if (dam == -4)        msgnum = 25;
  else if (dam == -5)   msgnum = 26;
  else if (dam == -2)   msgnum = 0;
  else if (dam == -1)   msgnum = 1;
  else if (dam == -3)   msgnum = 2;
  else if (dam == 0)	msgnum = 3;
  else if (dam <= 5)    msgnum = 4;
  else if (dam <= 10)   msgnum = 5;
  else if (dam <= 15)   msgnum = 6;
  else if (dam <= 20)   msgnum = 7;
  else if (dam <= 25)   msgnum = 8;
  else if (dam <= 30)   msgnum = 9;
  else if (dam <= 35)   msgnum = 10;
  else if (dam <= 40)   msgnum = 11;
  else if (dam <= 45)   msgnum = 12;
  else if (dam <= 50)   msgnum = 13;
  else if (dam <= 55)   msgnum = 14;
  else if (dam <= 60)   msgnum = 15;
  else if (dam <= 65)   msgnum = 16;
  else if (dam <= 70)   msgnum = 17;
  else if (dam <= 75)   msgnum = 18;
  else if (dam <= 80)   msgnum = 19;
  else if (dam <= 90)   msgnum = 20;
  else if (dam <= 100)  msgnum = 21;
  else if (dam <= 130)  msgnum = 22;
  else if (dam <= 160)  msgnum = 23;
  else                  msgnum = 24;

/* Blunt, when you are changing this down here, use IS_SET_AR(PLR_FLAG(), PLR_COMBATSET2)) if its set, show msgset2 */

  if (msgnum > 3) {
  // damage message to onlookers 
  buf = replace_string(dam_weapons2[msgnum].to_room,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);
  }

  // damage message to damager 
  if(!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_COMBATSET2))
  {
  buf = replace_string(dam_weapons2[msgnum].to_char,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  send_to_char(ch, "&W[&R%d&W]&n ", dam);
  act(buf, FALSE, ch, NULL, victim, TO_CHAR);
  } else {
  buf = replace_string(dam_weapons1[msgnum].to_char,
          attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  send_to_char(ch, "&W[&R%d&W]&n ", dam);
  act(buf, FALSE, ch, NULL, victim, TO_CHAR);
  }

  // damage message to damagee 
  if(!IS_NPC(victim) && IS_SET_AR(PLR_FLAGS(victim), PLR_COMBATSET2))
  {
  buf = replace_string(dam_weapons2[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  send_to_char(victim, "&W[&R%d&W]&n ", dam);
  act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
  } else {
  buf = replace_string(dam_weapons2[msgnum].to_victim,
          attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  send_to_char(victim, "&W[&R%d&W]&n ", dam);
  act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
  }


}


//
//  message for doing damage with a spell or skill
//  C3.0: Also used for weapon damage on miss and death blows
//
int skill_message(int dam, struct char_data *ch, struct char_data *vict,
		      int attacktype)
{
  int i, j, nr;
  struct message_type *msg;

  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);

  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) > LVL_IMMORT)) {
	act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      } else if (dam != 0) {
        //
        // Don't send redundant color codes for TYPE_SUFFERING & other types
        // of damage without attacker_msg.
        //
	if (GET_POS(vict) == POS_DEAD) {
          if (msg->die_msg.attacker_msg) {
            send_to_char(ch, "&W[&R%d&W]&n ", dam);
            send_to_char(ch, CCYEL(ch, C_CMP));
            act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
            send_to_char(ch, CCNRM(ch, C_CMP));
          }
          send_to_char(vict, "&W[&R%d&W]&n ", dam);
	  send_to_char(vict, CCRED(vict, C_CMP));
	  act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(vict, CCNRM(vict, C_CMP));

	  act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	} else {
          if (msg->hit_msg.attacker_msg) {
            send_to_char(ch, "&W[&R%d&W]&n ", dam);
	    send_to_char(ch, CCYEL(ch, C_CMP));
	    act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	    send_to_char(ch, CCNRM(ch, C_CMP));
          }
          send_to_char(vict, "&W[&R%d&W]&n ", dam);
	  send_to_char(vict, CCRED(vict, C_CMP));
	  act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(vict, CCNRM(vict, C_CMP));

	  act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	}
      } else if (ch != vict) {	// Dam == 0 
        if (msg->miss_msg.attacker_msg) {
          send_to_char(ch, "&W[&R%d&W]&n ", dam);
	  send_to_char(ch, CCYEL(ch, C_CMP));
	  act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(ch, CCNRM(ch, C_CMP));
        }
        send_to_char(vict, "&W[&R%d&W]&n ", dam);
	send_to_char(vict, CCRED(vict, C_CMP));
	act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	send_to_char(vict, CCNRM(vict, C_CMP));

	act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      }
      return (1);
    }
  }
  return (0);
}

void damage_eq(struct char_data *vict, int dam)
{
  int eq_pos=0,eq_dam=0;
//  struct obj_data *obj=NULL;
  int rand_eq[26] ={0,13,3,1,5,9,2,3,13,4,5,3,7,8,9,10,4,11,9,3,12,13,5,14,3,13};

  eq_pos = rand_number(0,25);

  switch(rand_eq[eq_pos])
  {
      case 0:
      case 1:
      case 2:
      case 6:
      case 7:
      case 10:
      case 11:
         eq_dam = rand_number(0,3);
         break;

      case 3:
      case 4:
      case 5:
      case 8:
      case 9:
      case 12:
      case 13:
      case 14:
        eq_dam = rand_number(0,2);
        break;

      default:
        log("Something wrong with eq damage function - line 1203 fight.c");
        break;
  }
  
  if (GET_EQ(vict, rand_eq[eq_pos]) && !OBJ_FLAGGED(GET_EQ(vict, rand_eq[eq_pos]), ITEM_NO_DAMAGE)) 
   GET_OBJ_COND(GET_EQ(vict,rand_eq[eq_pos])) += eq_dam;
   
  return;
}
//
// Alert: As of bpl14, this function returns the following codes:
//	< 0	Victim died.
//	= 0	No damage.
//	> 0	How much damage done.
//
int damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype)
{
  
  bool missile = FALSE;
  struct char_data *temp;
  bool reflect = FALSE;
  int i;
  int adjdam;
//  struct obj_data *chweapon=NULL;

  if (GET_POS(victim) <= POS_DEAD) {
    // This is "normal"-ish now with delayed extraction. -gg 3/15/2001 
    if (PLR_FLAGGED(victim, PLR_NOTDEADYET) || MOB_FLAGGED(victim, MOB_NOTDEADYET))
      return (-1);

    log("SYSERR: Attempt to damage corpse '%s' in room #%d by '%s'.",
		GET_NAME(victim), GET_ROOM_VNUM(IN_ROOM(victim)), GET_NAME(ch));
    die(victim, ch);
    return (-1);			// -je, 7/7/92 
  }

  if (ch->in_room != victim->in_room)
    missile = TRUE;

  // peaceful rooms 
  if (ch->nr != real_mobile(DG_CASTER_PROXY) &&
      ch != victim && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char(ch, "&YARRRRR....THERE BE NO FIGHTING HERE...&n\r\n");
    return (0);
  }

  // shopkeeper protection 
  if (!ok_damage_shopkeeper(ch, victim))
    return (0);

  if (attacktype == SKILL_TRIPWIRE){
    GET_POS(victim) = POS_SITTING;
    return(0);
  }

  // You can't damage an immortal! 
  if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_GOD))
    dam = 0;

  if (!IS_NPC(victim) && GET_DEFEND(victim) > 0)
    dam = -3;

  if ((victim != ch) && (!missile)) {
    // Start the attacker fighting the victim 
    if (GET_POS(ch) > POS_STUNNED && (FIGHTING(ch) == NULL))
      set_fighting(ch, victim);

    // Start the victim fighting the attacker 
    if (GET_POS(victim) > POS_STUNNED && (FIGHTING(victim) == NULL)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch))
	remember(victim, ch);
    }
  }

  //   Don't kill your own pet, you dick! 
  if (victim->master == ch)
    {
    GET_HIT(ch) -= 10;
    stop_follower(victim);
    }

  // If the attacker is invisible, he becomes visible 
  if (AFF_FLAGGED(ch, AFF_INVISIBLE | AFF_HIDE))
    appear(ch);

  // Cut damage in half if victim has sanct, to a minimum 1 
  if (AFF_FLAGGED(victim, AFF_SANCTUARY) && dam >= 2 && attacktype != TYPE_REFLECT)
    dam /= 2;

  if (PLR_FLAGGED(victim, PLR_FISHING) && dam >= 4)
    dam = (int)(dam * 1.5);

  if (GET_POS(ch) == POS_MEDITATE)
    dam = (int)(dam * 1.5);
 
  if (AFF_FLAGGED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) && dam >= 4 && attacktype != TYPE_REFLECT)
    dam = (int)(dam * .8);

  if (AFF_FLAGGED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) && dam >= 4 && attacktype != TYPE_REFLECT)
    dam = (int)(dam * .8);
 
         
  // Check for PK if this is not a PK MUD 
//  if (!CONFIG_PK_ALLOWED) {
//    check_killer(ch, victim);
//    if (PLR_FLAGGED(ch, PLR_KILLER) && (ch != victim))
//      dam = 0;
//  }
 
  if (IS_ASSASSIN(ch) && attacktype == SKILL_BACKSTAB)
  {
   if ((rand_number(1, 100)) > ((int) GET_SKILL(ch, SKILL_ADVANCED_BACKSTAB)*.75) || GET_LEVEL(ch) >= LVL_GOD)
   {
     dam += 100;
     send_to_room(IN_ROOM(ch), "&R%s twists a blade in the back of %s.\r\n&n", GET_NAME(ch), GET_NAME(victim));
   }
  }
 
  if (IS_WARRIOR(ch) && !IS_RANGER(ch) && attacktype != TYPE_REFLECT) 
  {
   if ((rand_number(1, 100)) > 98 || GET_LEVEL(ch) >= LVL_GOD)
   {
     dam += 100;
     send_to_room(IN_ROOM(ch), "&R%s lands a critical hit.\r\n&n", GET_NAME(ch));
   }
  }

  if (GET_CLASS(ch) == CLASS_RANGER && attacktype != TYPE_REFLECT)
  {
   if ((rand_number(1, 100)) > 98 || GET_LEVEL(ch) >= LVL_GOD)
   {
     dam += 150;
     send_to_room(IN_ROOM(ch), "&R%s lands a critical hit.\r\n&n", GET_NAME(ch));
   }
  }
  
  if (IS_MAGIC_USER(ch) && attacktype != TYPE_REFLECT)
    dam = (dam / 3);
  
  if (IS_CLERIC(ch) && attacktype != TYPE_REFLECT)
    dam = (dam / 2);

  if(dam > 0)
    form_mod_dam(ch, victim, dam);

  // Set the maximum damage per round and subtract the hit points 
//  dam = MAX(MIN(dam, 100), 0);
  if (AFF_FLAGGED(victim, AFF_MANASHIELD) && !IS_NPC(victim) && GET_MANA(victim) > GET_MAX_MANA(victim)/4) {
    GET_MANA(victim) -= dam ;

   if(!IS_NPC(ch)) {
    GET_DAMD(ch) += dam;
    GET_TOT_DAMD(ch) += dam;
   }

   if(!IS_NPC(victim)) {
    GET_DAMT(victim) += dam;
    GET_TOT_DAMT(victim) += dam;
   }
  }
  else if (AFF_FLAGGED(victim, AFF_VENGEANCE) && !IS_NPC(victim) && IS_WEAPON(attacktype)) {
    GET_HIT(ch) -= dam;

   if(!IS_NPC(ch)) {
    GET_DAMD(ch) += dam;
    GET_TOT_DAMD(ch) += dam;
   }

   if(!IS_NPC(victim)) {
    GET_DAMT(victim) += dam;
    GET_TOT_DAMT(victim) += dam;
   }

    reflect = TRUE;
  }
  else if (AFF_FLAGGED(victim, AFF_DIVINE_PROT) && !IS_NPC(victim) && !IS_WEAPON(attacktype)) {
    GET_HIT(ch) -=dam; 

   if(!IS_NPC(ch)) {
    GET_DAMD(ch) += dam;
    GET_TOT_DAMD(ch) += dam;
   }

   if(!IS_NPC(victim)) {
    GET_DAMT(victim) += dam;
    GET_TOT_DAMT(victim) += dam;
   }

    reflect = TRUE;
  }
  else if (AFF_FLAGGED(victim, AFF_REFLECT) && !IS_NPC(victim) && attacktype != TYPE_REFLECT)
  {
    GET_HIT(victim) -= dam;
    GET_HIT(ch) -= dam/2;

   if(!IS_NPC(ch)) {
    GET_DAMD(ch) += dam;
    GET_TOT_DAMD(ch) += dam;
    GET_DAMT(ch) += dam/2;
    GET_TOT_DAMT(ch) += dam/2;
   }

   if(!IS_NPC(victim)) {
    GET_DAMT(victim) += dam;
    GET_TOT_DAMT(victim) += dam;
    GET_DAMD(victim) += dam/2;
    GET_TOT_DAMD(victim) += dam/2;
   }

    act("&YYour reflection shield bounces $n's attack back at $M.&n", FALSE, victim, 0, 0, TO_CHAR);
    act("&Y$N's reflection shield bounces some of your attack back to you.&n", FALSE, ch, 0, 0, TO_CHAR);
    act("&Y$N's reflection shield bounces some of $n's attack back at $M.&n", FALSE, ch, 0, 0, TO_NOTVICT);
  }
  else if (dam == -5) {
    GET_HIT(ch) -= GET_DAMROLL(victim);

   if(!IS_NPC(ch)) {
    GET_DAMD(ch) += dam;
    GET_TOT_DAMD(ch) += dam;
   }

   if(!IS_NPC(victim)) {
    GET_DAMT(victim) += dam;
    GET_TOT_DAMT(victim) += dam;
   }

  }
  else {
    GET_HIT(victim) -= dam;

   if(!IS_NPC(ch)) {
    GET_DAMD(ch) += dam;
    GET_TOT_DAMD(ch) += dam;
   }

   if(!IS_NPC(victim)) {
    GET_DAMT(victim) += dam;
    GET_TOT_DAMT(victim) += dam;
   }

  }

  if (AFF_FLAGGED(victim, AFF_MANASHIELD) && !IS_NPC(victim) && GET_MANA(victim) < GET_MAX_MANA(victim)/4)
    affect_from_char(victim, SPELL_MANASHIELD);

  if (AFF_FLAGGED(victim, AFF_SPARKSHIELD)) {
    GET_HIT(victim) -= dam*0.8; // only shave off a tiny bit of damage
    adjdam = rand_number(0,dam);
    GET_HIT(ch) -= adjdam; // do random damage to attacker

   if(!IS_NPC(ch)) {
    GET_DAMD(ch) += dam*0.8;
    GET_TOT_DAMD(ch) += dam*0.8;
    GET_DAMT(ch) += adjdam;
    GET_TOT_DAMT(ch) += adjdam;
   }

   if(!IS_NPC(victim)) {
    GET_DAMT(victim) += dam*0.8;
    GET_TOT_DAMT(victim) += dam*0.8;
    GET_DAMD(victim) += adjdam;
    GET_TOT_DAMD(victim) += adjdam;
   }

    act("&YSparks shoot from your sparkshield searing $n.&n", FALSE, ch, 0, 0, TO_CHAR);
    act("&YSparks shoot from $N's sparkshield searing you.&n", FALSE, ch, 0, 0, TO_VICT);
    act("&YSparks shoot from $N's sparkshield searing $n.&n", FALSE, ch, 0, 0, TO_NOTVICT);
  }

  if(!IS_NPC(victim))
    GET_RET_DAM(victim) += dam / 2;

  if(!IS_NPC(ch) && AFF_FLAGGED(ch, AFF_MELEEHEAL))
    GET_HIT(ch) += MIN(GET_MAX_HIT(ch), (GET_HIT(ch) + ((dam/100) * rand_number(1,12))));

  // Gain exp for the hit 
  if ((ch != victim) && (!missile) && !IS_NPC(ch))
    gain_exp(ch, ((GET_LEVEL(victim)/10) * dam));

  if(GET_EQ(ch, WEAR_WIELD) && IS_WEAPON(attacktype))
  {
    GET_OBJ_COND(GET_EQ(ch, WEAR_WIELD)) += 1;
  }

  damage_eq(victim, dam);
  update_pos(victim);

  //
  // skill_message sends a message from the messages file in lib/misc.
  // dam_message just sends a generic "You hit $n extremely hard.".
  // skill_message is preferable to dam_message because it is more
  // descriptive.
  // 
  // If we are _not_ attacking with a weapon (i.e. a spell), always use
  // skill_message. If we are attacking with a weapon: If this is a miss or a
  // death blow, send a skill_message if one exists; if not, default to a
  // dam_message. Otherwise, always send a dam_message.
  //

  if (!IS_WEAPON(attacktype) || missile) {
    skill_message(dam, ch, victim, attacktype);
  }
  else {
    if ((GET_POS(victim) == POS_DEAD) || ((dam == 0) && (attacktype != TYPE_REFLECT))) {
      if (!skill_message(dam, ch, victim, attacktype)) {
	dam_message(dam, ch, victim, attacktype);
      }
    } else {
      dam_message(dam, ch, victim, attacktype);
     }
  }

  if(AFF_FLAGGED(victim, AFF_DIVINE_PROT) && reflect == TRUE) {
      act("$n's shield of divine protection reflects the spell back to its caster.", TRUE, victim, 0, 0, TO_ROOM);
      temp = victim;
      victim = ch;
      ch = temp;
  }
  if(AFF_FLAGGED(victim, AFF_VENGEANCE) && reflect == TRUE) {
      act("$n's shield of vengeance deflects the attack to its owner.", TRUE, victim, 0, 0, TO_ROOM);
      temp = victim;
      victim = ch;
      ch = temp;
  }

  // Use send_to_char -- act() doesn't send message if you are DEAD. 
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char(victim, "You are mortally wounded, and will die soon, if not aided.\r\n");
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char(victim, "You are incapacitated an will slowly die, if not aided.\r\n");
    break;
  case POS_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char(victim, "You're stunned, but will probably regain consciousness again.\r\n");
    break;
  case POS_DEAD:
    act("&W$n is dead!  R.I.P.&n", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char(victim, "&WYou are dead!  Sorry...&n\r\n");
    break;

  default:			// >= POSITION SLEEPING 
    if (dam > (GET_MAX_HIT(victim) / 4))
      send_to_char(victim, "%sThat really did HURT!%s\r\n",
                   CCGRN(ch, C_NRM), CCNRM(ch, C_NRM));

    if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 4)) {
      send_to_char(victim, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
		CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
      if (ch != victim && MOB_FLAGGED(victim, MOB_WIMPY))
	do_flee(victim, NULL, 0, 0);
    }
    if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
	GET_HIT(victim) < GET_WIMP_LEV(victim) && GET_HIT(victim) > 0) {
      send_to_char(victim, "%sYou wimp out, and attempt to flee!%s\r\n",
                   CCRED(ch, C_NRM), CCNRM(ch, C_NRM));
      do_flee(victim, NULL, 0, 0);
    }
    break;
  }

  // Help out poor linkless people who are attacked 
  if (!IS_NPC(victim) && !(victim->desc) && GET_POS(victim) != POS_DEAD) {
    do_flee(victim, NULL, 0, 0);
    if (!FIGHTING(victim)) {
      act("&W$n is rescued by divine forces.&n", FALSE, victim, 0, 0, TO_ROOM);
      GET_WAS_IN(victim) = IN_ROOM(victim);
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }

  // Uh oh.  Victim died. 
  if (GET_POS(victim) == POS_DEAD || PLR_FLAGGED(victim, PLR_DYING)) {
    if (ch != victim && (IS_NPC(victim) || victim->desc)) {
      if (AFF_FLAGGED(ch, AFF_GROUP)) {
	group_gain(ch, victim);
        make_blood(ch, victim);
      }
      else {
        solo_gain(ch, victim);
        make_blood(ch, victim);
      }
    }

/*
    if (FIGHTING(victim) != victim && (IS_NPC(victim) || victim->desc) && attacktype == TYPE_SUFFERING ) {
      if (AFF_FLAGGED(FIGHTING(victim), AFF_GROUP))
        group_gain(FIGHTING(victim), victim);
      else
        solo_gain(FIGHTING(victim), victim);
    }
*/  
  // stop someone from fighting if they're stunned or worse
   if (GET_POS(victim) <= POS_DEAD || FIGHTING(victim) != NULL)
       stop_fighting(victim);
       
  if(!IS_NPC(ch) && IS_NPC(victim)&& PLR_FLAGGED(ch, PLR_QUESTOR))
    if(GET_MOB_VNUM(victim) == GET_QUESTMOB(ch))
      GET_QKILLS(ch) = GET_QKILLS(ch) - 1;

  if(!IS_NPC(ch) && IS_NPC(victim))
    get_materials(ch, victim);  
  
  if (!IS_NPC(ch) && IS_NPC(victim) && (GET_MOB_VNUM(victim) == 301 || GET_MOB_VNUM(victim) == 401 || GET_MOB_VNUM(victim) == 501))
     GET_SOULS(ch) += 1;

    if (!IS_NPC(victim)) {
      nmudlog(DEATH_LOG, LVL_GOD, TRUE, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch), world[IN_ROOM(victim)].name);
//      mudlog(BRF, LVL_IMMORT, TRUE, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch), world[IN_ROOM(victim)].name);
      if (MOB_FLAGGED(ch, MOB_MEMORY))
	forget(ch, victim);
    }

    if (!IS_NPC(victim) && !IS_NPC(ch))
        GET_PLAYER_KILLS(ch)++;
    if (!IS_NPC(victim) && !IS_NPC(ch) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
        GET_CPKS(ch)++;
    if (!IS_NPC(ch) && IS_NPC(victim))
        GET_MONSTER_KILLS(ch)++;
    
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK) && (!IS_NPC(ch) && !IS_NPC(victim)))
    {
       player_npkd(victim, ch);
       return(-1);
    }
    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK) && (!IS_NPC(ch) && !IS_NPC(victim)))
    {
      for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(victim, i))
          perform_remove(victim, i);
    }

    if (PLR_FLAGGED(ch, PLR_OUTLAW) && PLR_FLAGGED(victim, PLR_OUTLAW))
    {
      for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(victim, i)) 
          perform_remove(victim, i);        
    }

    if (!IS_NPC(ch) && ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA)){
      arena_kill(ch, victim);
      return(-1);
    }

    if(!IS_NPC(victim)){
      SET_BIT_AR(PLR_FLAGS(victim), PLR_DYING); 
      victim->desc->killer = ch;
      GET_DYING_TIME(victim) = 5;
      stop_follower(victim);
      stop_fighting(ch);
      stop_fighting(victim);
    }

    die(victim, ch);
    return (-1);
  }
  return (dam);
}


//
// Calculate the THAC0 of the attacker.
//
// 'victim' currently isn't used but you could use it for special cases like
// weapons that hit evil creatures easier or a weapon that always misses
// attacking an animal.
//
int compute_thaco(struct char_data *ch, struct char_data *victim)
{
  int calc_thaco;

  if (!IS_NPC(ch))
    calc_thaco = thaco(GET_CLASS(ch), GET_LEVEL(ch));
  else		// THAC0 for monsters is set in the HitRoll 
    calc_thaco = 20;
  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= GET_HITROLL(ch);
  calc_thaco -= (int) ((GET_INT(ch) - 13) / 1.5);	// Intelligence helps! 
  calc_thaco -= (int) ((GET_WIS(ch) - 13) / 1.5);	// So does wisdom 

  return calc_thaco;
}

int calc_parry(struct char_data *ch)
{
  int pnts=0;

  if (IS_KNIGHT(ch))
  { 
   pnts = rand_number(1,6) + GET_STR(ch) - 10;
   if(pnts > 7)
    return 1;
   else
    return 0;
  }
  else if (IS_WARRIOR(ch) || IS_THIEF(ch))
  {
   pnts = rand_number(1,6) + GET_STR(ch) - 14;
   if(pnts > 9)
    return 1;
   else
    return 0;
  }
  else
  {
   pnts = rand_number(1,3) + GET_STR(ch) - 14;
   if(pnts > 8)
    return 1;
   else
    return 0;
  }
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  struct obj_data *wieldedv = GET_EQ(victim, WEAR_WIELD);

  int w_type, victim_ac, calc_thaco, dam=0, diceroll, bsmult, defend=FALSE, dodge=FALSE;
  int parry=FALSE, evade=FALSE, counterattack=FALSE, vicroll=0, chroll=0, vicbonus=0;

  /* check if the character has a fight trigger */
  fight_mtrigger(ch);

  // Do some sanity checking, in case someone flees, etc. 
  if (IN_ROOM(ch) != IN_ROOM(victim)) {
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

  if(!IS_NPC(victim) && PLR_FLAGGED(victim, PLR_DYING)){
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

//MOB INTELLIGENCE BEGINS
   if (IS_MOB(ch) && GET_POS(ch) == POS_STANDING) {
                switch(GET_MCLASS(ch)){   
                    case 0:  // HUMANOID
                        humanoid_combat(ch);   
                        break;              
                    case 1:  // UNDEAD   
                        undead_combat(ch);
                        break;
                    case 2:  // ANIMAL
                        animal_combat(ch);
                        break;
                    case 3:  // GENERIC   
                        generic_combat(ch);
                        break;             
                    case 4:  // GIANT        
                        giant_combat(ch);
                        break;
                    case 5:  // MAGIC USER
                        magic_user_combat(ch);     
                        break;
                    case 6: // CLERIC     
                        cleric_combat(ch);
                        break;          
                    case 7:  // THIEF
                        thief_combat(ch);
                        break;
                    case 8:  // WARRIOR 
                        warrior_combat(ch);
                        break;
                    case 9:  // RANGER
                        ranger_combat(ch);
                        break;
                    case 10: // KNIGHT
                        knight_combat(ch);
                        break;
                    case 11: // PALADIn
                        paladin_combat(ch);
                        break;
                    case 12: // ROGUE
                        rogue_combat(ch);
                        break;
                    case 13: // ASSASSIN
                        assassin_combat(ch);
                        break;
                    case 14: // NINJA
                        ninja_combat(ch);
                        break;
                    case 15: // MYSTIC
                        mystic_combat(ch);
                        break;
                    case 16: // SHAMAN
                        shaman_combat(ch);
                        break;
                    case 17: // PRIEST
                       priest_combat(ch);
                        break;            
                    case 18: // NECROMANCER 
                        necro_combat(ch);  
                        break;             
                    case 19: // SORCERER              
                        sorcerer_combat(ch);
                        break;        
                    case 20: // ELEMENTALIST
                        elementalist_combat(ch);
                        break;
                    case 21: // ELEMENTAL
                        elemental_combat(ch);
                        break;
                    case 22: // BLACKDRAGON  
                        blackdragon_combat(ch);
                        break; 
                    case 23: // BLUEDRAGON
                        bluedragon_combat(ch);
                        break; 
                    case 24: // REDDRAGON  
                        reddragon_combat(ch);
                        break; 
                    case 25: // GREENDRAGON  
                        greendragon_combat(ch);
                        break; 
                    case 26: // GOLDDRAGON  
                        golddragon_combat(ch);
                        break; 
                    case 27: // RAND_DRAGON  
                        randomdragon_combat(ch);
                        break;       
                    default:              
                        break;          
                }
    }
//MOB INTELLIGENCE ENDS



 
  // Find the weapon type (for display purposes only) 
  if (wielded && GET_OBJ_TYPE(wielded) == (ITEM_WEAPON || ITEM_BOW || ITEM_CROSSBOW || ITEM_SLING))
    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else {
    if (IS_NPC(ch) && ch->mob_specials.attack_type != 0)
      w_type = ch->mob_specials.attack_type + TYPE_HIT;
    else
      w_type = TYPE_HIT;
  }

  
  // Calculate chance of hit. Lower THAC0 is better for attacker. 
  calc_thaco = compute_thaco(ch, victim);

  // Calculate the raw armor including magic armor.  Lower AC is better for defender. 
  victim_ac = compute_armor_class(victim);

  // roll the die and take your chances... 
  diceroll = rand_number(1, 20);

  //
  // Decide whether this is a hit or a miss.
  //
  // Victim asleep = hit, otherwise:
  //     1   = Automatic miss.
  //   2..19 = Checked vs. AC.
  //    20   = Automatic hit.
  //

/*  if (!IS_NPC(ch))
  {
   if (diceroll == 20 || !AWAKE(victim))
     dam = TRUE;
   else if (diceroll == 1)
     dam = FALSE;
   else if (AFF_FLAGGED(ch, AFF_ENTANGLE) && rand_number(1,100) < 20)
     dam = FALSE;
   else
     dam = ((calc_thaco - diceroll) <= victim_ac);
  }
  else
  {
  if (diceroll >= 5 || !AWAKE(victim))
     dam = TRUE;
   else if (diceroll == 1)
     dam = FALSE;
   else if (AFF_FLAGGED(ch, AFF_ENTANGLE) && rand_number(1,100) < 20)
     dam = FALSE;
   else
     dam = ((calc_thaco - diceroll) <= victim_ac);
  }
*/

//  if (victim_ac >=0)
//    vicroll = ((100 - victim_ac));
//  else
//    vicroll = ((100 + victim_ac));
  

  chroll = GET_HITROLL(ch) - ((20 - GET_DEX(ch)) * 5);

  if(AFF_FLAGGED(ch, AFF_SNEAK))
    chroll += 10;
 
  if(IS_THIEF(ch) && type == SKILL_RANGE_COMBAT)
    chroll += GET_HITROLL(ch);

//  diceroll = rand_number(1, chroll);

  if ((chroll + (victim_ac/10)) > rand_number(1,100))
     dam = TRUE;
  else
    dam = FALSE;


 if(IS_NPC(victim))
 {
  if ((rand_number( 1, 100 ) < 10) && M_IS_THIEF(victim))
  {
     dam = -2;
     dodge = TRUE;
  }
  else
     dodge = FALSE;

  if ((rand_number( 1, 100 ) < 20) && (M_IS_NINJA(victim) || M_IS_ASSASSIN(victim)))
  {
     dam = -2;
     dodge = TRUE;
  }
  else
     dodge = FALSE;

  if ((rand_number( 1, 100 ) < 10) && M_IS_WARRIOR(victim) && GET_EQ(victim, WEAR_WIELD))
  {
     dam = -1;
     parry = TRUE;
  }
  else
     parry = FALSE;

    if ((rand_number( 1, 100 ) < 20) && M_IS_KNIGHT(victim) && GET_EQ(victim, WEAR_WIELD))
  {
     dam = -1;
     parry = TRUE;
  }
  else
     parry = FALSE;
 }

 if(!IS_NPC(victim))
 { 
  if (GET_SKILL(victim, SKILL_DODGE) > 0 && !IS_NPC(victim))
  {
        if(calc_parry(victim))
        {
             dam = -2;
             dodge = TRUE;
        }
        else
             dodge = FALSE;
  }
  else if (GET_SKILL(victim, SKILL_ADVANCED_DODGE) > 0 && !IS_NPC(victim))
  {
        if(calc_parry(victim))
        {
             dam = -2;
             dodge = TRUE;
        }
        else
             dodge = FALSE;
  }
  else
     dodge = FALSE;

  if (GET_SKILL(victim, SKILL_PARRY) > 0 && GET_EQ(victim, WEAR_WIELD) && !IS_NPC(victim))
  {
	if(calc_parry(victim))
	{
	     dam = -1;
	     parry = TRUE;
	}
	else
	     parry = FALSE;	 
  }
  else if (GET_SKILL(victim, SKILL_ADVANCED_PARRY) > 0 && GET_EQ(victim, WEAR_WIELD) && !IS_NPC(victim))
  {
        if(calc_parry(victim))
        {
             dam = -1;
             parry = TRUE;
        }
        else
             parry = FALSE;
  }
  else
     parry = FALSE;

  if (!IS_NPC(victim) && AFF_FLAGGED(victim, AFF_EVASION) && rand_number(1,100) < 50)
  {
     dam = -3;
     evade = TRUE;
  } 

  if (!IS_NPC(victim) && GET_DEFEND(victim) > 0) 
  {
     dam = -4;
     defend = TRUE;
  }
 
/*  if (!IS_NPC(victim) && IS_KNIGHT(victim) && rand_number(1,100) < GET_LEVEL(ch)/2)
  {
     dam = -5;
     counterattack = TRUE;

     if (wieldedv && GET_OBJ_TYPE(wieldedv) == ITEM_WEAPON)
       w_type = GET_OBJ_VAL(wieldedv, 3) + TYPE_HIT;
     else {
      if (IS_NPC(ch) && ch->mob_specials.attack_type != 0)
        w_type = ch->mob_specials.attack_type + TYPE_HIT;
      else
        w_type = TYPE_HIT;
     }
  }*/
 }

  if (!dam)
      damage(ch, victim, 0, type == SKILL_BACKSTAB ? SKILL_BACKSTAB : w_type);
  else if (dodge == TRUE)
      damage(ch, victim, dam, w_type);
  else if (defend == TRUE)
      damage(ch, victim, dam, w_type);
  else if(parry == TRUE)
      damage(ch, victim, dam, w_type);
  else if (evade == TRUE)
      damage(ch, victim, dam, w_type);
//  else if (counterattack = TRUE)
//      damage(ch, victim, dam, w_type);
  else {
    // okay, we know the guy has been hit.  now calculate damage. 

    // Start with the damage bonuses: the damroll and strength apply 
    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);
 
    /* check for weapon to send weapon diceroll info to weapon procs */
    int dr=0;

    if (wielded)
       dr = dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));

    // Maybe holding arrow? 
    if (wielded && GET_OBJ_TYPE(wielded) == (ITEM_WEAPON || ITEM_BOW || ITEM_CROSSBOW || ITEM_SLING)) {
      // Add weapon-based damage if a weapon is being wielded 
      dam += dr;
    } else {
      // If no weapon, add bare hand damage instead 
      if (IS_NPC(ch))
	dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
      else
	dam += rand_number(0, 2);	// Max 2 bare hand damage for players 
    }

    //
    // Include a damage multiplier if victim isn't ready to fight:
    //
    // Position sitting  1.33 x normal
    // Position resting  1.66 x normal
    // Position sleeping 2.00 x normal
    // Position stunned  2.33 x normal
    // Position incap    2.66 x normal
    // Position mortally 3.00 x normal
    //
    // Note, this is a hack because it depends on the particular
    // values of the POSITION_XXX constants.
    //
    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;

    // at least 1 hp damage min per hit 
    dam = MAX(1, dam);

    if (AFF_FLAGGED(ch, AFF_SNEAK))
      affect_from_char(ch, SKILL_SNEAK);

    if (type == SKILL_BACKSTAB)
    {
      dam += DoWeaponProc(ch, victim, dr);
      bsmult = backstab_mult(GET_LEVEL(ch), GET_HITROLL(ch), victim_ac);
      if (!IS_NPC(ch) && IS_THIEF(ch))
         bsmult += (1 / 2);
      if (!IS_NPC(ch) && (rand_number( 1, 100 ) < GET_SKILL(ch, SKILL_ADVANCED_BACKSTAB) / 4) && IS_ASSASSIN(ch))
         bsmult += 1;
      dam *= bsmult;
      damage(ch, victim, dam, SKILL_BACKSTAB);
    }
    else 
    {
      dam += DoWeaponProc(ch, victim, dr);
      damage(ch, victim, dam, w_type);
    }
  }

  /* check if the victim has a hitprcnt trigger */
  hitprcnt_mtrigger(victim);
}


/* replica of hit() */
void double_hit(struct char_data *ch, struct char_data *victim, int type)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  struct obj_data *wieldedv = GET_EQ(victim, WEAR_WIELD);

  int w_type, victim_ac, calc_thaco, dam=0, diceroll, bsmult, defend=FALSE, dodge=FALSE;
  int parry=FALSE, evade=FALSE, counterattack=FALSE, vicroll=0, chroll=0, vicbonus=0;

  fight_mtrigger(ch);

  if (IN_ROOM(ch) != IN_ROOM(victim)) {
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

  if(!IS_NPC(victim) && PLR_FLAGGED(victim, PLR_DYING)){
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

   if (IS_MOB(ch) && GET_POS(ch) == POS_STANDING) {
                switch(GET_MCLASS(ch)){
                    case 0:  /* HUMANOID*/
                        humanoid_combat(ch);
                        break;
                    case 1:  /* UNDEAD*/
                        undead_combat(ch);
                        break;
                    case 2:  /* ANIMAL*/
                        animal_combat(ch);
                        break;
                    case 3:  /* GENERIC*/
                        generic_combat(ch);
                        break;
                    case 4:  /* GIANT*/
                        giant_combat(ch);
                        break;
                    case 5:  /* MAGIC USER*/
                        magic_user_combat(ch);
                        break;
                    case 6: /* CLERIC*/
                        cleric_combat(ch);
                        break;
                    case 7:  /* THIEF*/
                        thief_combat(ch);
                        break;
                    case 8:  /* WARRIOR*/
                        warrior_combat(ch);
                        break;
                    case 9:  /* RANGER*/
                        ranger_combat(ch);
                        break;
                    case 10: /* KNIGHT*/
                        knight_combat(ch);
                        break;
                    case 11: /* PALADIn*/
                        paladin_combat(ch);
                        break;
                    case 12: /* ROGUE*/
                        rogue_combat(ch);
                        break;
                    case 13: /* ASSASSIN*/
                        assassin_combat(ch);
                        break;
                    case 14: /* NINJA*/
                        ninja_combat(ch);
                        break;
                    case 15: /* MYSTIC*/
                        mystic_combat(ch);
                        break;
                    case 16: /* SHAMAN*/
                        shaman_combat(ch);
                        break;
                    case 17: /* PRIEST*/
                       priest_combat(ch);
                        break;
                    case 18: /* NECROMANCER*/
                        necro_combat(ch);
                        break;
                    case 19: /* SORCERER*/
                        sorcerer_combat(ch);
                        break;
                    case 20: /* ELEMENTALIST*/
                        elementalist_combat(ch);
                        break;
                    case 21: /* ELEMENTAL*/
                        elemental_combat(ch);
                        break;
                    case 22: /* BLACKDRAGON*/
                        blackdragon_combat(ch);
                        break;
                    case 23: /* BLUEDRAGON*/
                        bluedragon_combat(ch);
                        break;
                    case 24: /* REDDRAGON*/
                        reddragon_combat(ch);
                        break;
                    case 25: /* GREENDRAGON*/
                        greendragon_combat(ch);
                        break;
                    case 26: /* GOLDDRAGON*/
                        golddragon_combat(ch);
                        break;
                    case 27: /* RAND_DRAGON*/
                        randomdragon_combat(ch);
                        break;
                    default:
                        break;
                }
    }
  if (wielded && GET_OBJ_TYPE(wielded) == (ITEM_WEAPON || ITEM_BOW || ITEM_CROSSBOW || ITEM_SLING))
    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else {
    if (IS_NPC(ch) && ch->mob_specials.attack_type != 0)
      w_type = ch->mob_specials.attack_type + TYPE_HIT;
    else
      w_type = TYPE_HIT;
  }
/*
  calc_thaco = compute_thaco(ch, victim);
  victim_ac = compute_armor_class(victim);
  diceroll = rand_number(1, 20);
  vicbonus += ((GET_SKILL(victim, SKILL_PARRY)/20)+(GET_SKILL(victim, SKILL_ADVANCED_PARRY)/20) + (GET_SKILL(victim, SKILL_ADVANCED_DODGE)/20) + (GET_SKILL(victim, SKILL_DODGE)/20));

  if (victim_ac >=0)
    vicroll = ((100 - victim_ac));
  else
    vicroll = ((100 - victim_ac));

  chroll = GET_HITROLL(ch) - ((20 - GET_DEX(ch)) / 2);

  if(AFF_FLAGGED(ch, AFF_SNEAK))
    chroll += 20;

  if(IS_THIEF(ch) && type == SKILL_RANGE_COMBAT)
    chroll += GET_HITROLL(ch);

  diceroll = rand_number(1, vicroll);

  if (chroll > (vicroll+vicbonus))
     dam = TRUE;
  else if (diceroll < chroll)
     dam = TRUE;
  else
    dam = FALSE;
*/

  chroll = GET_HITROLL(ch) - ((20 - GET_DEX(ch)) * 5);

  if(AFF_FLAGGED(ch, AFF_SNEAK))
    chroll += 10;

  if(IS_THIEF(ch) && type == SKILL_RANGE_COMBAT)
    chroll += GET_HITROLL(ch);

  if ((chroll + (victim_ac/10)) > rand_number(1,100))
     dam = TRUE;
  else
    dam = FALSE;


 if(IS_NPC(victim))
 {
  if ((rand_number( 1, 100 ) < 50) && M_IS_THIEF(victim))
  {
     dam = -2;
     dodge = TRUE;
  }
  else
     dodge = FALSE;

  if ((rand_number( 1, 100 ) < 50) && (M_IS_NINJA(victim) || M_IS_ASSASSIN(victim)))
  {
     dam = -2;
     dodge = TRUE;
  }
  else
     dodge = FALSE;

  if ((rand_number( 1, 100 ) < 50) && M_IS_WARRIOR(victim) && GET_EQ(victim, WEAR_WIELD))
  {
     dam = -1;
     parry = TRUE;
  }
  else
     parry = FALSE;

    if ((rand_number( 1, 100 ) < 50) && M_IS_KNIGHT(victim) && GET_EQ(victim, WEAR_WIELD))
  {
     dam = -1;
     parry = TRUE;
  }
  else
     parry = FALSE;
 }

 if(!IS_NPC(victim))
 {
  if (GET_SKILL(victim, SKILL_DODGE) > 0 && !IS_NPC(victim))
  {
        if(calc_parry(victim))
        {
             dam = -2;
             dodge = TRUE;
        }
        else
             dodge = FALSE;
  }
  else if (GET_SKILL(victim, SKILL_ADVANCED_DODGE) > 0 && !IS_NPC(victim))
  {
        if(calc_parry(victim))
        {
             dam = -2;
             dodge = TRUE;
        }
        else
             dodge = FALSE;
  }
  else
     dodge = FALSE;

  if (GET_SKILL(victim, SKILL_PARRY) > 0 && GET_EQ(victim, WEAR_WIELD) && !IS_NPC(victim))
  {
        if(calc_parry(victim))
        {
             dam = -1;
             parry = TRUE;
        }
        else
             parry = FALSE;
  }
  else if (GET_SKILL(victim, SKILL_ADVANCED_PARRY) > 0 && GET_EQ(victim, WEAR_WIELD) && !IS_NPC(victim))
  {
        if(calc_parry(victim))
        {
             dam = -1;
             parry = TRUE;
        }
        else
             parry = FALSE;
  }
  else
     parry = FALSE;

  if (!IS_NPC(victim) && AFF_FLAGGED(victim, AFF_EVASION) && rand_number(1,100) < 50)
  {
     dam = -3;
     evade = TRUE;
  }

  if (!IS_NPC(victim) && GET_DEFEND(victim) > 0)
  {
     dam = -4;
     defend = TRUE;
  }

/*  if (!IS_NPC(victim) && IS_KNIGHT(victim) && rand_number(1,100) < GET_LEVEL(ch)/2)
  {
     dam = -5;
     counterattack = TRUE;

     if (wieldedv && GET_OBJ_TYPE(wieldedv) == ITEM_WEAPON)
       w_type = GET_OBJ_VAL(wieldedv, 3) + TYPE_HIT;
     else {
      if (IS_NPC(ch) && ch->mob_specials.attack_type != 0)
        w_type = ch->mob_specials.attack_type + TYPE_HIT;
      else
        w_type = TYPE_HIT;
     }
  }*/
 }

  if (!dam)
      damage(ch, victim, 0, type == SKILL_BACKSTAB ? SKILL_BACKSTAB : w_type);
  else if (dodge == TRUE)
      damage(ch, victim, dam, w_type);
  else if (defend == TRUE)
      damage(ch, victim, dam, w_type);
  else if(parry == TRUE)
      damage(ch, victim, dam, w_type);
  else if (evade == TRUE)
      damage(ch, victim, dam, w_type);
//  else if (counterattack = TRUE)
//      damage(ch, victim, dam, w_type);
  else {

    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);


    if (wielded && GET_OBJ_TYPE(wielded) == (ITEM_WEAPON || ITEM_BOW || ITEM_CROSSBOW || ITEM_SLING)) {
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
    } else {
      if (IS_NPC(ch))
        dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
      else
        dam += rand_number(0, 2);    
    }

    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;

    dam = MAX(1, dam);

    if (AFF_FLAGGED(ch, AFF_SNEAK))
      affect_from_char(ch, SKILL_SNEAK);

    if (type == SKILL_BACKSTAB)
    {
      bsmult = backstab_mult(GET_LEVEL(ch), GET_HITROLL(ch), victim_ac);
      if (IS_THIEF(ch))
         bsmult += (1 / 2);
      if (rand_number( 1, 100 ) < GET_SKILL(ch, SKILL_ADVANCED_BACKSTAB) / 4)
         bsmult += 1;
      dam *= bsmult;
      damage(ch, victim, dam, SKILL_BACKSTAB);
    }
    else
      damage(ch, victim, dam, w_type);
  }

  hitprcnt_mtrigger(victim);
}

// control the fights going on.  Called every 2 seconds from comm.c. 
void perform_violence(void)
{
  struct char_data *ch, *vict;

  for (ch = combat_list; ch; ch = next_combat_list) {
    next_combat_list = ch->next_fighting;

    if (FIGHTING(ch) == NULL || IN_ROOM(ch) != IN_ROOM(FIGHTING(ch))) {
      stop_fighting(ch);
      continue;
    }

    if (IS_NPC(ch)) {
      if (GET_MOB_WAIT(ch) > 0) {
	GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
	continue;
      }
      GET_MOB_WAIT(ch) = 0;
      if (GET_POS(ch) < POS_FIGHTING) {
	GET_POS(ch) = POS_FIGHTING;
	act("&G$n scrambles to $s feet!&n", TRUE, ch, 0, 0, TO_ROOM);
      }
    }

    if (GET_POS(ch) < POS_FIGHTING) {
      send_to_char(ch, "You can't fight while sitting!!\r\n");
      GET_POS(ch) = POS_FIGHTING;
      continue;
    }
    
   find_victim(ch);
    
    for (k = ch->followers; k; k=k->next) {
      if (!IS_NPC(k->follower) && PRF_FLAGGED(k->follower, PRF_AUTOASSIST) &&
        (k->follower->in_room == ch->in_room) && !FIGHTING(k->follower))
       do_assist(k->follower, GET_NAME(ch), 0, 0);
    }

    vict = FIGHTING(ch);

   int perc=0, val=0, apr=0;
    if (IS_NPC(ch)) {
       perc = rand_number(1, 101);
       val = ch->mob_specials.attack1;
       if (val > perc) {
        apr++;
        perc = rand_number(1,101);
        val = ch->mob_specials.attack2;
          if (val > perc) {
            apr++;
            perc = rand_number(1,101);
            val = ch->mob_specials.attack3;
            if (val > perc) {
             apr++;
            }
          }
       }
     
     combat_intel(ch);
     apr = MAX(-1, MIN(apr, 4));
     if (apr >= 0) {
       for (; apr >= 0 && FIGHTING(ch); apr--) {
         hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
         if (GET_EQ(ch, WEAR_DWIELD) && GET_HIT(vict) > 0)
            double_hit(ch, FIGHTING(ch), TYPE_UNDEFINED);	 
       }  
     }
    }

   if (!IS_NPC(ch) && GET_HIT(vict) > 0) {
     diag_char_to_char(vict, ch);
//     DoWeaponProc(ch, vict);
     DoEquipmentProc(ch, vict);
    
    if (!IS_NPC(ch) && GET_POS(vict) >= POS_STUNNED && !AFF_FLAGGED(ch, AFF_VENGEANCE) && !AFF_FLAGGED(ch, AFF_DIVINE_PROT))
    {
    

    hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    if (GET_EQ(ch, WEAR_DWIELD) && GET_HIT(vict) > 0)
      double_hit(ch, FIGHTING(ch), TYPE_UNDEFINED);

    if (rand_number( 1, 100 ) < GET_SKILL(ch, SKILL_ATTACK2) / 2)  /* 50% chance to activate second hit */
         hit(ch, vict, TYPE_UNDEFINED);
       if (GET_EQ(ch, WEAR_DWIELD) && GET_HIT(vict) > 0)
         double_hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    }

    if (!IS_NPC(ch) && GET_POS(vict) >= POS_STUNNED && (IS_WARRIOR(ch) || IS_ROGUE(ch) || IS_ASSASSIN(ch)) && !AFF_FLAGGED(ch, AFF_VENGEANCE) && !AFF_FLAGGED(ch, AFF_DIVINE_PROT)) {

    if (rand_number( 1, 100 ) < GET_SKILL(ch, SKILL_ATTACK3) / 3)  /* 25% chance to activate third hit */
       hit(ch, vict, TYPE_UNDEFINED);
    if (GET_EQ(ch, WEAR_DWIELD) && GET_HIT(vict) > 0)
      double_hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    }

    if (!IS_NPC(ch) && GET_POS(vict) >= POS_STUNNED && (IS_PALADIN(ch) || IS_KNIGHT(ch) || IS_RANGER(ch)) && !AFF_FLAGGED(ch, AFF_VENGEANCE) && !AFF_FLAGGED(ch, AFF_DIVINE_PROT)) {

    if (rand_number( 1, 100 ) < GET_SKILL(ch, SKILL_ATTACK4) / 4)  /* 25% chance to activate fourth hit */
       hit(ch, vict, TYPE_UNDEFINED);
    if (GET_EQ(ch, WEAR_DWIELD) && GET_HIT(vict) > 0)
      double_hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    }

    if (!IS_NPC(ch) && GET_POS(vict) >= POS_STUNNED && IS_KNIGHT(ch) && !AFF_FLAGGED(ch, AFF_VENGEANCE) && !AFF_FLAGGED(ch, AFF_DIVINE_PROT)) {

    if (rand_number( 1, 100 ) < GET_SKILL(ch, SKILL_ATTACK5) / 5)  /* 25% chance to activate fifth hit */
       hit(ch, vict, TYPE_UNDEFINED);
     if (GET_EQ(ch, WEAR_DWIELD) && GET_HIT(vict) > 0)
       double_hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    
    }
    
    if (MOB_FLAGGED(ch, MOB_SPEC) && GET_MOB_SPEC(ch) && !MOB_FLAGGED(ch, MOB_NOTDEADYET)) {
      char actbuf[MAX_INPUT_LENGTH] = "";
      (GET_MOB_SPEC(ch)) (ch, ch, 0, actbuf);
    }
   }
  }  
}

int get_danger(struct char_data *ch)
{
   int danger=0;

   if (IS_THIEF(ch) || IS_WARRIOR(ch))
   {
      danger += GET_STR(ch) - 13;
      danger += GET_DEX(ch) - 13;
      danger += (GET_HITROLL(ch)) / 5; 
      danger += GET_DAMROLL(ch)-25;
      danger += (GET_MAX_HIT(ch)-500) / 100;
   }

   if (IS_CLERIC(ch) || IS_MAGIC_USER(ch))
   {
      danger += GET_INT(ch) - 13;
      danger += GET_WIS(ch) - 13;
      danger += (GET_LEVEL(ch) - 20) * 5;
      danger += (GET_MAX_MANA(ch)-500) / 100;
   }

   if(PLR_FLAGGED(ch, AFF_HEROISM))
    danger += 500;
   
return(danger);
}

void find_victim(struct char_data *ch)
{  
  struct char_data *k, *d;
  struct follow_type *f;
//  int num_members = 1, attack = 1, counter = 2; 
  int danger=0, newdanger=0;
   
  if (GET_POS(ch) < POS_FIGHTING) {
    stop_fighting(ch);
    return;
  }  
            
  k = (FIGHTING(ch)->master ? FIGHTING(ch)->master : FIGHTING(ch));

  if (k == NULL)
     k = FIGHTING(ch);

  danger = get_danger(k);
  d = k;
  // find the number of members in the group 
//  for (f = k->followers; f; f = f->next)
//    num_members += 1;
  
//  if (num_members != 1)
//    attack = rand_number(1, num_members);

   for (f = k->followers; f; f = f->next) {
     if (IN_ROOM(FIGHTING(ch)) != IN_ROOM(f->follower))
       continue;
     else {
       newdanger = get_danger(f->follower);
       if (danger < newdanger) {
  	 danger = newdanger;
         d = f->follower;
       }
     }
   }
   
//      if (GET_POS(d) < POS_STANDING)
//        FIGHTING(ch) = k;
//      else
        FIGHTING(ch) = d;

  // victim is the leader of the group 
/*  if (attack == 1) {  
    FIGHTING(ch) = k;
    return;
  } else {  // victim is one of the leaders followers 
    for (f = k->followers; f; f = f->next) {
      if (IN_ROOM(FIGHTING(ch)) != IN_ROOM(f->follower))
        counter += 1;
      else if (counter != attack)
        counter += 1;
      else {
        if (GET_POS(f->follower) < POS_STANDING)
           FIGHTING(ch) = k;
        else
           FIGHTING(ch) = f->follower;
        return;
      }
    }
  }*/
  if (d == NULL)
    log("SYSERR: find_victim reached end of loop without finding a victim");
  return;  
}

ACMD(do_plunder)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *newobj;
  struct char_data *vict;

  two_arguments(argument, arg1, arg2);


  if (!*arg2)
    send_to_char(ch, "From who?\r\n");
  else if (!(vict = get_char_vis(ch, arg2, NULL, FIND_CHAR_ROOM)))
  {
    send_to_char(ch, "They don't seem to be here.\r\n");
    return;
  } 
  else {

   if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK) && (!IS_NPC(ch) && !IS_NPC(vict)) || PLR_FLAGGED(vict, PLR_OUTLAW)) 
   {
    if (!*arg1) {
      send_to_char(ch, "Plunder what?\r\n");
      return;
    }
    else if (!(obj = get_obj_in_list_vis(ch, arg1, NULL, vict->carrying))) {
      send_to_char(ch, "%s doesn't seem to have %s %s.\r\n", GET_NAME(vict), AN(arg1), arg1);
      return;
    }
    else {
      obj_from_char(obj);
      obj_to_char(obj, ch);
      act("&GYou plunder $p from $N.&n", FALSE, ch, obj, vict, TO_CHAR);
      send_to_char(vict, "&G%s plunders %s from you.&n", GET_NAME(ch), GET_OBJ_NAME(obj));
      act("&G$n plunders $p from $N.&n", TRUE, ch, obj, vict, TO_NOTVICT);
    }
   }
  }

  return;
}

