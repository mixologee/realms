/* ************************************************************************
*   File: interpreter.c                                                   *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
************************************************************************ */
#define __INTERPRETER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "genolc.h"
#include "oasis.h"
#include "improved-edit.h"
#include "pfdefaults.h"
#include "clan.h"
#include "dg_scripts.h"
#include "constants.h"
#include "quest.h"
#include "logger.h"

/* local global variables */
DISABLED_DATA *disabled_first=NULL;

int check_disabled(const struct command_info *command);

/* external variables */
extern room_rnum r_mortal_start_room;
extern room_rnum r_immort_start_room;
extern room_rnum r_frozen_start_room;
extern const char *class_menu;
extern char *motd;
extern char *imotd;
extern char *background;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int circle_restrict;
extern int no_specials;
extern int selfdelete_fastwipe;
extern int xap_objs;
extern struct clan_type *clan_info;
extern int exp_multiplier;


/* external functions */
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(struct char_data *ch);
int parse_class(char arg);
int special(struct char_data *ch, int cmd, char *arg);
int isbanned(char *hostname);
int valid_name(char *newname);
void read_aliases(struct char_data *ch);
void delete_aliases(const char *charname);
void remove_player(int pfilepos);
void load_plr_objects(struct char_data *ch);
void kill_warnings(const char *charname);
void load_warnings(struct char_data *ch);

ACMD(do_spec_msg);
//void do_spec_msg(struct char_data *ch, char *arg);

/* local functions */
int perform_dupe_check(struct descriptor_data *d);
struct alias_data *find_alias(struct alias_data *alias_list, char *str);
void free_alias(struct alias_data *a);
void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias_data *a);
int perform_alias(struct descriptor_data *d, char *orig, size_t maxlen);
int reserved_word(char *argument);
int _parse_name(char *arg, char *name);
void read_saved_vars(struct char_data *ch);
void recover_password(struct descriptor_data *d);

char *email_error_message(int error_code);
int validate_email(char *check_addr);
char *output_email(char *mail_addr);


// DISABLED CMD FUNCS

int check_disabled(const struct command_info *command);

/* CRYSTALGATE DEFINES */
void cmd_press                  (char_data *ch, char *argument);
void fix_facets                 (char_data *ch, obj_data *obj, int symbol);
void reset_crystalgate          (char_data *ch, obj_data *obj);
void check_valid_sequence       (char_data *ch, obj_data *obj);
void cmd_crystalgate            (char_data *ch, char *argument);


/* prototypes for all do_x functions. */
ACMD(do_bind);
ACMD(do_bet);
ACMD(do_arena);
ACMD(do_awho);
ACMD(do_ahall);
ACMD(do_chaos);
ACMD(do_backstab);
ACMD(do_cast);
ACMD(do_claninfo);
ACMD(do_kill);
ACMD(do_get);
ACMD(do_put);
ACMD(do_look);
ACMD(do_action);
ACMD(do_affected);
ACMD(do_auction);
ACMD(do_autoassist);
ACMD(do_autosplit);
ACMD(do_advance);
ACMD(do_aedit);
ACMD(do_afk);
ACMD(do_alias);
ACMD(do_appraise);
ACMD(do_attack_castle);
ACMD(do_areas);
ACMD(do_assist);
ACMD(do_astat);
ACMD(do_at);
ACMD(do_badname);
ACMD(do_ban);
//ACMD(do_blackmarket);
ACMD(do_bash);
ACMD(do_bid);
ACMD(do_blaze);
ACMD(do_castout);
ACMD(do_clan);
ACMD(do_checkdist);
ACMD(do_chop);
ACMD(do_color);
ACMD(do_commands);
ACMD(do_combatset);
ACMD(do_consider);
ACMD(do_credits);
ACMD(do_crystalgate);
ACMD(do_clantalk);
ACMD(do_charge);
ACMD(do_cripple);
ACMD(do_date);
ACMD(do_dbs);
ACMD(do_dc);
ACMD(do_defend);
ACMD(do_diagnose);
ACMD(do_disable);  	// disable commands function
ACMD(do_dig);
ACMD(do_disarm);
ACMD(do_display);
ACMD(do_drink);
ACMD(do_drop);
ACMD(do_choose_death);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_edit);		/* Mainly intended as a test function. */
ACMD(do_email);
ACMD(do_enlist);
ACMD(do_entangle);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_flee);
ACMD(do_finddoor);
ACMD(do_findkey);
ACMD(do_fixtrans);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_formstat);
ACMD(do_gamestats);
ACMD(do_gecho);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_give);
ACMD(do_gnews);
ACMD(do_godchan); 
//ACMD(do_godsend);
//ACMD(do_godrecieve);
ACMD(do_godlist);
ACMD(do_gold);
ACMD(do_gossip);
ACMD(do_goto);
ACMD(do_gozone);
ACMD(do_gocoord);
ACMD(do_grab);
ACMD(do_posse);
ACMD(do_gohome);
ACMD(do_gsay);
ACMD(do_hcontrol);
ACMD(do_help);
ACMD(do_helpinfo);
//ACMD(do_helpsys);
ACMD(do_hide);
ACMD(do_history);
ACMD(do_hit);
ACMD(do_house);
ACMD(do_learn);
ACMD(do_leader);
ACMD(do_lumber);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_kick);
ACMD(do_last);
ACMD(do_learn);
ACMD(do_leave);
ACMD(do_leather);
ACMD(do_levels);
ACMD(do_listkeywords);
ACMD(do_load);
/* ACMD(do_move); -- interpreter.h */
ACMD(do_mail);
ACMD(do_mailedit);
ACMD(do_map);
ACMD(do_mbind);
ACMD(do_mapset);
ACMD(do_mapsave);
ACMD(do_mapsize);
ACMD(do_meditate);
ACMD(do_mine);
ACMD(do_myinfo);
ACMD(do_newb);
ACMD(do_news);
ACMD(do_not_here);
ACMD(do_oasis_copy);
ACMD(do_olc);
ACMD(do_order);
ACMD(do_objconv);
ACMD(do_outlaw);
ACMD(do_oasis_clanedit);
ACMD(do_oasis_hedit);
ACMD(do_peace);
ACMD(do_peace_all);
ACMD(do_page);
ACMD(do_mist);
ACMD(do_pelts);
ACMD(do_playerinfo);
ACMD(do_plunder);
ACMD(do_png);
ACMD(do_poofset);
ACMD(do_pour);
ACMD(do_practice);
ACMD(do_pray);
ACMD(do_profession);
ACMD(do_prompt);
ACMD(do_preceptor);
ACMD(do_press);
ACMD(do_ptell);
ACMD(do_purge);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_quest);
ACMD(do_raffs);
ACMD(do_rain);
ACMD(do_reelin);
ACMD(do_reboot);
ACMD(do_newb_recall);
ACMD(do_reclass);
//ACMD(do_receive_mail);
ACMD(do_reform);
ACMD(do_remove);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_restore_all);
ACMD(do_restring);
ACMD(do_return);
ACMD(do_room_copy);
ACMD(do_sacrifice);
ACMD(do_save);
ACMD(do_saveall);
ACMD(do_say);
ACMD(do_score);
ACMD(do_stats);
ACMD(do_scan);
ACMD(do_scales);
ACMD(do_search);
ACMD(do_sense);
//ACMD(do_send);
//ACMD(do_sendmail);
ACMD(do_set);
ACMD(do_setlog);
ACMD(do_shelter);
ACMD(do_show);
//ACMD(do_show_mail);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skillset);
ACMD(do_skin);
ACMD(do_shoot);
ACMD(do_show_clan);
ACMD(do_show_ore);
ACMD(do_show_gems);
ACMD(do_show_save_list);
ACMD(do_slay);
ACMD(do_sleep);
ACMD(do_smithery);
ACMD(do_snatch);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_split);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_stats);
ACMD(do_steal);
ACMD(do_survey);
ACMD(do_switch);
ACMD(do_symbols);
ACMD(do_syslog);
ACMD(do_tag);
ACMD(do_talk);
ACMD(do_teleport);
ACMD(do_tell);
//ACMD(do_tell_log);
ACMD(do_time);
ACMD(do_title);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_train);
ACMD(do_trans);
ACMD(do_trip);
ACMD(do_tripwire);
ACMD(do_throw);
ACMD(do_unban);
ACMD(do_boot);
ACMD(do_update);
ACMD(do_unbind);
ACMD(do_untag);
ACMD(do_unkey);
ACMD(do_use);
ACMD(do_users);
ACMD(do_vfile);
ACMD(do_viewall);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_vwear);
ACMD(do_wake);
ACMD(do_warn);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_who);
ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_wizhelp);
ACMD(do_wizlock);
ACMD(do_wizlist);
ACMD(do_wiznet);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_worth);
ACMD(do_xprate);
ACMD(do_zlock);
ACMD(do_zunlock);
ACMD(do_zonecount);
ACMD(do_zreset);

// DRAGON STUFFS
ACMD(do_maul);


ACMD(do_clear_input);

/* DG Script ACMD's */
ACMD(do_attach);
ACMD(do_detach);
ACMD(do_tstat);
ACMD(do_masound);
ACMD(do_mkill);
ACMD(do_mjunk);
ACMD(do_mdoor);
ACMD(do_mechoaround);
ACMD(do_msend);
ACMD(do_mecho);
ACMD(do_mload);
ACMD(do_mpurge);
ACMD(do_mgoto);
ACMD(do_mat);
ACMD(do_mdamage);
ACMD(do_mteleport);
ACMD(do_mforce);
ACMD(do_mhunt);
ACMD(do_mremember);
ACMD(do_mforget);
ACMD(do_mtransform);
ACMD(do_mzoneecho);
ACMD(do_vdelete);
ACMD(do_mfollow);
ACMD(do_mmail);

ACMD(do_buck);
ACMD(do_tame);
ACMD(do_mount);
ACMD(do_dismount);

ACMD(do_makemp);
ACMD(do_show_party);
ACMD(do_listtp);
ACMD(do_listroad);
ACMD(do_barrier);

/*  LYO CODE  */
ACMD(do_survey);                                // wilderness                           // HELP done
ACMD(do_travel);                                // wilderness                           // HELP done
ACMD(do_camp);                                  // wilderness
ACMD(do_build);                                 // buildings
ACMD(do_embark);                                // ships                                        // HELP done
ACMD(do_disembark);                             // ships                                        // HELP done
ACMD(do_ship);                                  // ships
ACMD(do_drive);                                 // vehicles
ACMD(do_charge);                                // vehicles
//ACMD(do_discharge);                             // vehicles
ACMD(do_out);                                   // vehicles
ACMD(do_yoke);                                  // vehicles
ACMD(do_unyoke);                                // vehicles
//ACMD(do_clan);                                  // clans
//ACMD(do_ctell);                                 // clans
//ACMD(do_negotiate);                             // clans
//ACMD(do_politics);                              // clans
//ACMD(do_trapremove);                    // obj traps
//ACMD(do_trapstat);                              // obj traps
ACMD(do_authorize);                             // buildings & ships code
/* new immortal commands */
ACMD(do_newmoney);
//ACMD(do_newbook);
ACMD(do_newvehicle);
ACMD(do_newgoods);
ACMD(do_market);
ACMD(do_mkaff);
ACMD(do_goodinfo);
ACMD(do_tp);
ACMD(do_economy);
ACMD(do_life);
ACMD(do_remap);
ACMD(do_wildlist);
ACMD(do_bldsave);
ACMD(do_bldlist);
ACMD(do_courselist);
ACMD(do_shipsetup);
ACMD(do_shipread);
ACMD(do_traplist);
ACMD(do_listworks);

ACMD(do_dwield);

/* temp */
//ACMD(do_objsaveall);
////ACMD(do_mobsaveall);
ACMD(do_newspaper);
//

//ACMD(do_listwild);

struct command_info *complete_cmd_info;

/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

cpp_extern const struct command_info cmd_info[] = {
  { "RESERVED", "", 0, 0, 0, 0 },	/* this must be first -- for specprocs */

  /* directions must come before other commands but after RESERVED */
  { "north"    , "n"       , POS_STANDING, do_move     , 0, SCMD_NORTH },
  { "east"     , "e"       , POS_STANDING, do_move     , 0, SCMD_EAST },
  { "south"    , "s"       , POS_STANDING, do_move     , 0, SCMD_SOUTH },
  { "west"     , "w"       , POS_STANDING, do_move     , 0, SCMD_WEST },
  { "up"       , "u"       , POS_STANDING, do_move     , 0, SCMD_UP },
  { "down"     , "d"       , POS_STANDING, do_move     , 0, SCMD_DOWN },
  
  { "--"       , "-"       , POS_DEAD    , do_clear_input , 0, 0 },

  /* most important commands i think */
  { "barrier"  , "barrier" , POS_DEAD    , do_barrier  , LVL_IMPL, 0 },
  { "backstab" , "ba"      , POS_STANDING, do_backstab , 1, 0 },
  { "bstab"    , "bs"      , POS_STANDING, do_backstab , 1, 0 },
  { "auction"  , "auc"     , POS_RESTING , do_auction  , 0, 0}, 
  { "cast"     , "c"       , POS_SITTING , do_cast     , 1, 0 },
  { "formsay"  , "f"       , POS_SLEEPING, do_gsay     , 0, 0 },
  { "flee"     , "fl"      , POS_FIGHTING, do_flee     , 1, 0 },
  { "get"      , "g"       , POS_RESTING , do_get      , 0, 0 },
  { "inventory", "i"       , POS_DEAD    , do_inventory, 0, 0 }, 
  { "kill"     , "k"       , POS_FIGHTING, do_kill     , 0, 0 },
  { "look"     , "l"       , POS_RESTING , do_look     , 0, SCMD_LOOK },
  { "list"     , "lis"     , POS_STANDING, do_not_here , 0, 0 },
  { "put"      , "p"       , POS_RESTING , do_put      , 0, 0 },
  { "psay"     , "ps"      , POS_SLEEPING, do_gsay     , 0, 0 },
  { "rest"     , "r"     , POS_RESTING , do_rest     , 0, 0 },
  { "say"      , "sa"     , POS_RESTING , do_say      , 0, 0 },
  { "score"    , "sc"      , POS_DEAD    , do_score    , 0, 0 },
  { "sneak"    , "sn"   , POS_STANDING, do_sneak    , 1, 0 },
  { "scan"     , "sca"     , POS_STANDING, do_scan     , 0, 0 },

  /* now, the main list */
  { "arena"    , "aren"    , POS_STANDING, do_arena    , 0, 0 },
  { "affects"  , "affects" , POS_STANDING, do_affected , 0, 0 }, 
  { "arinfo"   , "arin"    , POS_STANDING, do_gen_tog  , 0, 0 },
  { "at"       , "at"      , POS_DEAD    , do_at       , LVL_GOD, 0 },
  { "attack"   , "att"     , POS_STANDING, do_attack_castle, 0, 0 },
  { "advance"  , "adv"     , POS_DEAD    , do_advance  , LVL_IMPL, 0 },
  { "aedit"    , "aed"     , POS_DEAD    , do_oasis_aedit    , LVL_GOD, 0 },
  { "afk"      , "af"      , POS_DEAD    , do_gen_tog  , 1, SCMD_AFK },
  { "areas"    , "are"     , POS_DEAD    , do_areas    , 0, 0 },
  { "alias"    , "ali"     , POS_DEAD    , do_alias    , 0, 0 },
  { "appraise" , "app"     , POS_STANDING, do_appraise , 1, 0},
  { "assist"   , "as"      , POS_FIGHTING, do_assist   , 1, 0 },
  { "ask"      , "ask"     , POS_RESTING , do_spec_comm, 0, SCMD_ASK },
  { "astat"    , "ast"     , POS_DEAD    , do_astat    , LVL_BUILDER, 0 },
  { "autoassist", "autoas" , POS_STANDING, do_autoassist  , 1, 0 },
  { "autoexits", "autoex"  , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOEXIT },
  { "autosplit", "autosp"  , POS_DEAD    , do_autosplit, 0, 0 },
  { "awho"     , "awho"    , POS_DEAD    , do_awho     , 0, 0 },
  { "ahall"    , "ahall"   , POS_DEAD    , do_ahall     , 0, 0 },

  
  { "badname"  , "bad"     , POS_DEAD    , do_badname  , LVL_GOD, 0 },
  { "ban"      , "ban"     , POS_DEAD    , do_ban      , LVL_GRGOD, 0 },
//  { "blackmarket"  , "bla"     , POS_STANDING, do_blackmarket , 1, 0 },
  { "balance"  , "bal"     , POS_STANDING, do_not_here , 1, 0 },
  { "bash"     , "bas"     , POS_FIGHTING, do_bash     , 1, 0 },
  { "bearings" , "bea"     , POS_STANDING, do_survey    , 1, 0 },
  { "bet"      , "bet"     , POS_RESTING , do_bet      , 1, 0 },
  { "boot"     , "boot"    , POS_DEAD    , do_boot     , 0, 0 },
  { "bid"      , "bid"     , POS_RESTING , do_bid      , 0, 0 },
  { "bind"     , "bind"    , POS_DEAD    , do_bind     , LVL_GRGOD, 0 },
  { "brief"    , "br"      , POS_DEAD    , do_gen_tog  , 0, SCMD_BRIEF },
  { "buildwalk", "buildwalk", POS_STANDING, do_gen_tog,   LVL_BUILDER, SCMD_BUILDWALK },
  { "buy"      , "bu"      , POS_STANDING, do_not_here , 0, 0 },
  { "bug"      , "bug"     , POS_DEAD    , do_gen_write, 0, SCMD_BUG },
  { "buck"     , "buc"     , POS_STANDING, do_buck	   , 0, 0 },

  { "castout"  , "cast"    , POS_SITTING , do_castout  , 1, 0 },
  { "cedit"    , "cedit"   , POS_DEAD    , do_oasis_cedit    , LVL_IMPL, 0 },
  { "check"    , "ch"      , POS_STANDING, do_not_here , 1, 0 },
  { "chaos"    , "chao"    , POS_STANDING, do_chaos   , LVL_GOD, 0 },
  { "checkdist", "checkdist", POS_DEAD   , do_checkdist, LVL_IMPL, 0 },
  { "chop"     , "cho"      , POS_STANDING, do_chop , 1, 0 },
  { "clear"    , "cle"     , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "gaccept"  , "gacc"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_ACCEPT },
  { "gapply"   , "gapp"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_APPLY },
  { "guildedit" , "guilde"    , POS_DEAD    , do_oasis_clanedit    , LVL_IMPL, 0 }, 
  { "clsolc"   , "clsolc"  , POS_DEAD    , do_gen_tog  , 0, SCMD_CLS },

  { "gbalance" , "gbal"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_BALANCE },
  { "gdemote"  , "gdem"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_DEMOTE},
  { "gdeposit" , "gdep"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_DEPOSIT },
  { "gdismiss" , "gdis"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_DISMISS },
  { "gpromote" , "gprom"   , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_PROMOTE },

  { "charge"   , "cha"     , POS_SLEEPING, do_charge   , 0, 0 },
  { "ginfo"    , "gin"     , POS_SLEEPING, do_claninfo , 0, 0 },
  { "guilds"   , "gui"     , POS_SLEEPING, do_show_clan, 0, 0 },
  { "close"    , "clo"     , POS_SITTING , do_gen_door , 0, SCMD_CLOSE },
  { "cls"      , "cls"     , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "clsolc"   , "clsolc"  , POS_DEAD    , do_gen_tog  , 0, SCMD_CLS },
  { "consider" , "con"     , POS_RESTING , do_consider , 0, 0 },
  { "color"    , "col"     , POS_DEAD    , do_color    , 0, 0 },
  { "commands" , "com"     , POS_DEAD    , do_commands , 0, SCMD_COMMANDS },
  { "combatset", "combatset", POS_STANDING, do_combatset, 0, 0 },
  { "compact"  , "comp"    , POS_DEAD    , do_gen_tog  , 0, SCMD_COMPACT },
  { "credits"  , "cred"    , POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS },
  { "cripple"  , "crip"    , POS_FIGHTING, do_cripple, 0, 0 },
  { "crygates" , "gates"   , POS_DEAD    , do_crystalgate, LVL_GOD, 0 },
  { "gtalk"    , "gt"      , POS_SLEEPING, do_clantalk , 0, 0 },
  { "gtell"    , "gte"     , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_TELL },
  { "gwho"     , "gwh"     , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_WHO },
  { "gresign"  , "gres"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_RESIGN },
  { "grevoke"  , "grev"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_REVOKE },
  { "gwithdraw", "gwit"    , POS_SLEEPING, do_clan     , 0, SCMD_CLAN_WITHDRAW_GOLD },

  { "date"     , "da"      , POS_DEAD    , do_date     , LVL_GOD, SCMD_DATE },
  { "dbs"      , "dbs"     , POS_DEAD    , do_dbs      , LVL_BUILDER, 0 },
  { "dc"       , "dc"      , POS_DEAD    , do_dc       , LVL_IMPL, 0 },
  { "def"      , "defend"  , POS_FIGHTING, do_defend   , 0, 0 },
  { "deposit"  , "depo"    , POS_STANDING, do_not_here , 1, 0 },
  { "diagnose" , "diag"    , POS_RESTING , do_diagnose , 0, 0 },
  { "disable"  , "disable" , POS_DEAD    , do_disable  , LVL_GOD, 0 },
  { "dig"      , "dig"     , POS_DEAD    , do_dig      , LVL_BUILDER, 0 },
  { "disarm"   , "dis"     , POS_FIGHTING, do_disarm   , 0, 0},
  { "dismount" , "dism"    , POS_STANDING, do_dismount , 0, 0 },
  { "display"  , "disp"    , POS_DEAD    , do_display  , 0, 0 },
  { "donate"   , "don"     , POS_RESTING , do_drop     , 0, SCMD_DONATE },
  { "drink"    , "dri"     , POS_RESTING , do_drink    , 0, SCMD_DRINK },
  { "dwield"   , "dwi"     , POS_DEAD    , do_dwield   , 0, 0 },
  { "drop"     , "dro"     , POS_RESTING , do_drop     , 0, SCMD_DROP },

  { "eat"      , "ea"      , POS_RESTING , do_eat      , 0, SCMD_EAT },
  { "echo"     , "ec"      , POS_SLEEPING, do_echo     , LVL_GOD, SCMD_ECHO },
  { "email"    , "email"   , POS_RESTING,  do_email    , 1, 0 },
  { "emote"    , "em"      , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { ":"        , ":"       , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  
  { ","        , ","       , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { "enlist"   , "enl"     , POS_STANDING, do_enlist   , 0, 0 },
  { "entangle" , "enta"    , POS_FIGHTING, do_entangle , 0, 0 },
  { "enter"    , "ente"    , POS_STANDING, do_enter    , 0, 0 },
  { "equipment", "eq"      , POS_SLEEPING, do_equipment, 0, 0 },
  { "exits"    , "ex"      , POS_RESTING , do_exits    , 0, 0 },
  { "examine"  , "exa"     , POS_SITTING , do_examine  , 0, 0 },

  { "finddoor" , "finddoor", POS_DEAD    , do_finddoor , LVL_BUILDER, 0 },
  { "findkey"  , "findkey" , POS_DEAD    , do_findkey  , LVL_BUILDER, 0 },
  { "force"    , "force"   , POS_SLEEPING, do_force    , LVL_GOD, 0 },
  { "fill"     , "fil"     , POS_STANDING, do_pour     , 0, SCMD_FILL },
  { "fixtrans", "fixtrans", POS_DEAD , do_fixtrans, LVL_IMPL, 0},
  { "follow"   , "fol"     , POS_RESTING , do_follow   , 0, 0 },
  { "formstat"     , "formst"     , POS_RESTING , do_formstat    , 0, 0 }, 
  { "freeze"   , "freeze"  , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_FREEZE },

  { "gamestats", "gamest"   , POS_SLEEPING, do_gamestats, 0, 0 },
  { "gecho"    , "gecho"   , POS_DEAD    , do_gecho    , LVL_GOD, 0 },
  { "gems"     , "gem"     , POS_RESTING , do_show_gems, 0, 0 },
  { "gnews"    , "gnews"   , POS_SLEEPING, do_gnews     , LVL_GOD, 0 }, 
  { "give"     , "giv"     , POS_RESTING , do_give     , 0, 0 },
  { "god"      , "god"     , POS_DEAD    , do_godchan  , LVL_GOD, SCMD_GOD},
  { "gods"     , "gods"    , POS_DEAD    , do_godlist  , LVL_IMPL, 0 },
//  { "godsend"  , "godsend" , POS_DEAD    , do_godsend  , LVL_GOD, 0 },
//  { "godrecieve", "godrecieve", POS_DEAD , do_godrecieve, LVL_GOD, 0 },
  { "goto"     , "go"      , POS_SLEEPING, do_goto     , LVL_GOD, 0 },
  { "gohome"   , "goh"     , POS_STANDING, do_gohome   , 0, 0 },
  { "gold"     , "gol"     , POS_RESTING , do_gold     , 0, 0 },
  { "gossip"   , "gos"     , POS_SLEEPING, do_gossip   , 0, 0 },
  { "gozone"   , "goz"     , POS_DEAD    , do_gozone   , LVL_BUILDER, 0 },
  { "gocoord"  , "gocoord" , POS_DEAD    , do_gocoord  , LVL_GOD, 0 },
  { "goodinfo" , "goodi"   , POS_DEAD    , do_goodinfo , LVL_GOD, 0 },
  { "grab"     , "grab"    , POS_RESTING , do_grab     , 0, 0 },
  { "grats"    , "grat"    , POS_SLEEPING, do_gen_comm , 0, SCMD_GRATZ },
  { "grgod"    , "grgod"   , POS_DEAD    , do_godchan  , LVL_GRGOD, SCMD_GRGOD },
  { "gtell"    , "gt"      , POS_SLEEPING, do_gsay     , 0, 0 },

  { "help"     , "he"      , POS_DEAD    , do_help     , 0, 0 },
  { "hedit"    , "hedit"   , POS_DEAD  , do_oasis_hedit , LVL_GOD, 0 },
  { "helpinfo" , "helpi"   , POS_DEAD    , do_helpinfo     , 0, 0 },

  { "handbook" , "handb"   , POS_DEAD    , do_gen_ps   , LVL_GOD, SCMD_HANDBOOK },
  { "hcontrol" , "hcontrol", POS_DEAD    , do_hcontrol , LVL_GRGOD, 0 },
  { "hit"      , "hit"     , POS_STANDING, do_hit      , 1, 0 },
  { "hide"     , "hide"    , POS_RESTING , do_hide     , 1, 0 },
  { "hold"     , "hold"    , POS_RESTING , do_grab     , 1, 0 },
//  { "holler"   , "holler"  , POS_RESTING , do_gen_comm , 1, SCMD_HOLLER },
  { "holylight", "holy"    , POS_DEAD    , do_gen_tog  , LVL_GOD, SCMD_HOLYLIGHT },
  { "house"    , "house"   , POS_RESTING , do_house    , 0, 0 },

  { "iacga"    , "ia"      , POS_DEAD    , do_gen_tog  , 0, SCMD_IACGA },
  { "idea"     , "id"      , POS_DEAD    , do_gen_write, 0, SCMD_IDEA },
  { "imp"      , "imp"     , POS_DEAD    , do_godchan  , LVL_IMPL, SCMD_IMPL },
  { "imotd"    , "imo"     , POS_DEAD    , do_gen_ps   , LVL_GOD, SCMD_IMOTD },
  { "imm"      , "imm"     , POS_DEAD    , do_godchan  , LVL_IMMORT, SCMD_IMMORT },
  { "improve"  , "impr"    , POS_RESTING , do_not_here , 0, 0 },
  { "info"     , "info"    , POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO },
  { "insult"   , "insult"  , POS_RESTING , do_insult   , 0, 0 },
  { "invis"    , "invi"    , POS_DEAD    , do_invis    , LVL_GOD, 0 },

  { "junk"     , "junk"    , POS_RESTING , do_drop     , 0, SCMD_JUNK },

  { "kick"     , "kic"      , POS_FIGHTING, do_kick     , 1, 0 },
  { "killme"   , "killm"    , POS_DEAD   , do_choose_death , 1, 0 },

  { "last"     , "last"    , POS_DEAD    , do_last     , LVL_GOD, 0 },
  { "learn"    , "lea"     , POS_STANDING, do_learn    , 0, 0 },
  { "leader"   , "lead"    , POS_DEAD    , do_leader   , 0, 0 },
  { "leather"  , "leat"     , POS_STANDING, do_leather , 0, 0 },
  { "leave"    , "leav"     , POS_STANDING, do_leave    , 0, 0 },
  { "levels"   , "lev"     , POS_DEAD    , do_levels   , 0, 0 },
  { "links"    , "lin"     , POS_STANDING, do_oasis_list    , LVL_BUILDER, SCMD_OASIS_LINKS },
  { "listkeywords"   , "listk"     , POS_DEAD    , do_listkeywords   , LVL_IMPL, 0 },
  { "listworks"   , "listw"     , POS_DEAD    , do_listworks   , 0, 0 },
  { "lock"     , "loc"     , POS_SITTING , do_gen_door , 0, SCMD_LOCK },
  { "load"     , "load"     , POS_DEAD    , do_load     , LVL_GRGOD, 0 },
  { "lumber"   , "lum"     , POS_STANDING , do_lumber   , 0, 0 },
//  { "listwild"   , "listwi"     , POS_STANDING , do_listwild   , 0, 0 },

  { "makemp"   , "makemp"     , POS_STANDING, do_makemp      , LVL_IMPL, 0 },
  { "mapsize"  , "maps"     , POS_STANDING, do_mapsize      , 1, 0 },
  { "mapset"   , "mapse"     , POS_STANDING, do_mapset      , LVL_IMPL, 0 },
  { "mapsave"  , "mapsa"     , POS_STANDING, do_mapsave      , LVL_IMPL, 0 },
  { "market"  , "market"     , POS_STANDING, do_market      , LVL_IMPL, 0 },
  { "mbind"    , "mbind"   , POS_DEAD    , do_mbind    , LVL_IMPL, 0 },
  { "motd"     , "motd"    , POS_DEAD    , do_gen_ps   , 0, SCMD_MOTD },
  { "mail"     , "mail"    , POS_STANDING, do_mail     , 1, 0 },
  { "mailedit" , "maile"    , POS_STANDING, do_mailedit     , 1, 0 },
//  { "mailreceive" , "mailr"    , POS_STANDING, do_receive_mail , LVL_IMPL, 0 },
//  { "maillist" , "maill"   , POS_STANDING, do_show_mail, LVL_IMPL, 0 },
  { "maul"     , "maul"    , POS_FIGHTING, do_maul     , LVL_IMPL, 0 },
  { "medit"    , "medit"   , POS_DEAD    , do_oasis_medit    , LVL_BUILDER, 0 },
  { "meditate" , "med"     , POS_RESTING , do_meditate , 1, 0 },
  { "message"  , "message" , POS_STANDING, do_spec_msg, 1, 0 },
  { "meta"     , "meta"    , POS_RESTING , do_not_here , 1, 0 },
  { "mine"     , "mine"    , POS_RESTING , do_mine     , 1, 0 },
  { "mist"     , "mist"    , POS_RESTING , do_mist     , 1, 0 },
  { "mlist"    , "mlist"   , POS_DEAD    , do_oasis_list    , LVL_BUILDER, SCMD_OASIS_MLIST },
  { "mmail"    , "mmail"   , POS_DEAD    , do_mmail    , -1, 0 },
  { "mount"    , "mount"   , POS_STANDING, do_mount , 0, 0 },
  { "mylog"    , "myl"    , POS_RESTING , do_history     , 1, 0 },
  { "myinfo"    , "myi"    , POS_RESTING , do_myinfo     , 1, 0 },
  { "mute"     , "mute"    , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_SQUELCH },

  { "newb"     , "newb"    , POS_SLEEPING, do_newb     , 0, 0 },
  { "news"     , "news"    , POS_SLEEPING, do_news     , 0, 0 },
  { "newspaper"     , "newsp"    , POS_SLEEPING, do_newspaper     , 0, 0 },
  { "noauction", "noauc"   , POS_SLEEPING, do_gen_tog  , 0, SCMD_NOAUCTION },
  { "nogossip" , "nogossip", POS_DEAD    , do_gen_tog  , 0, SCMD_NOGOSSIP },
  { "nograts"  , "nograts" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGRATZ },
  { "nohassle" , "nohassle", POS_DEAD    , do_gen_tog  , LVL_GOD, SCMD_NOHASSLE },
  { "norepeat" , "norepeat", POS_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT },
  { "noshout"  , "noshout" , POS_SLEEPING, do_gen_tog  , 1, SCMD_DEAF },
  { "nosummon" , "nosummon", POS_DEAD    , do_gen_tog  , 1, SCMD_NOSUMMON },
  { "notell"   , "notell"  , POS_DEAD    , do_gen_tog  , 1, SCMD_NOTELL },
  { "notitle"  , "notitle" , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_NOTITLE },
  { "nowiz"    , "nowiz"   , POS_DEAD    , do_gen_tog  , LVL_GOD, SCMD_NOWIZ },

  { "order"    , "ord"     , POS_RESTING , do_order    , 1, 0 },
  { "offer"    , "off"     , POS_STANDING, do_not_here , 1, 0 },
  { "open"     , "ope"     , POS_SITTING , do_gen_door , 0, SCMD_OPEN },
  { "ore"     , "ore"     , POS_RESTING , do_show_ore , 0, 0 },
  { "olc"      , "olc"     , POS_DEAD    , do_show_save_list , LVL_BUILDER, 0 },
  { "olist"    , "olist"   , POS_DEAD    , do_oasis_list    , LVL_BUILDER, SCMD_OASIS_OLIST },
  { "oedit"    , "oedit"   , POS_DEAD    , do_oasis_oedit    , LVL_BUILDER, 0 },
  { "outlaw"   , "ou"      , POS_DEAD    , do_gen_tog  , 1, SCMD_OUTLAW},

  { "posse"    , "pos"     , POS_RESTING , do_posse    , 1, 0 },
  { "pick"     , "pi"      , POS_STANDING, do_gen_door , 1, SCMD_PICK },
  { "page"     , "pag"     , POS_DEAD    , do_page     , LVL_GOD, 0 },
  { "pardon"   , "pardon"  , POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_PARDON },
  { "png"      , "png"     , POS_STANDING, do_png    , 1, 0 },
  { "peace"    , "peace"   , POS_DEAD    , do_peace     , LVL_GOD, 0 },
  { "peaceall" , "peaceall", POS_DEAD    , do_peace_all, LVL_IMPL, 0 },
  { "pelts"    , "pel"     , POS_STANDING, do_pelts    , 1, 0 },
  { "playerinfo", "player"     , POS_STANDING, do_playerinfo    , 1, 0 },
  { "plunder"   , "plunder", POS_STANDING, do_plunder    , 1, 0 },
  { "pinfo"    , "pinfo"     , POS_STANDING, do_playerinfo    , 1, 0 },
  { "party"    , "party"     , POS_STANDING, do_show_party    , LVL_IMPL, 0 },

  { "pirate"   , "pira"     , POS_STANDING, do_not_here , 1, 0 },
  { "policy"   , "pol"     , POS_DEAD    , do_gen_ps   , 0, SCMD_POLICIES },
  { "pour"     , "pour"    , POS_STANDING, do_pour     , 0, SCMD_POUR },
//  { "prompt"   , "pro"     , POS_DEAD    , do_display  , 0, 0 },
  { "prompt"   , "prompt"  , POS_DEAD    , do_prompt   , 0, 0 },
  { "pray"     , "pray"    , POS_RESTING , do_pray     , 1, 0 },
  { "practice" , "pra"     , POS_RESTING , do_practice , 1, 0 },
  { "press"    , "press"   , POS_STANDING, do_press   , 1, 0 },

  { "profession","pro"     , POS_RESTING , do_profession, 0, 0 },
  { "preceptor", "prec"    , POS_DEAD    , do_godchan  , LVL_IMMORT, SCMD_PRECEPTORCHAN },
  { "ptell"    , "ptell"   , POS_DEAD    , do_ptell   , LVL_GOD, 0},
  { "purge"    , "purge"   , POS_DEAD    , do_purge    , LVL_GOD, 0 },

  { "qedit"    , "qedit"   , POS_DEAD    , do_oasis_qedit, LVL_BUILDER, 0 },
  { "qlist"    , "qlist"   , POS_DEAD    , do_oasis_list, LVL_BUILDER, SCMD_OASIS_QLIST },
  { "quaff"    , "qua"     , POS_RESTING , do_use      , 0, SCMD_QUAFF },
  { "qecho"    , "qec"     , POS_DEAD    , do_qcomm    , LVL_GOD, SCMD_QECHO },
  { "quest"    , "que"     , POS_DEAD    , do_quest  , 0, 0 },
  { "qui"      , "qui"     , POS_DEAD    , do_quit     , 0, 0 },
  { "quit"     , "quit"    , POS_DEAD    , do_quit     , 0, SCMD_QUIT },
  { "qsay"     , "qsay"    , POS_RESTING , do_qcomm    , 0, SCMD_QSAY },
 
  { "raffs"    , "raffs"   , POS_DEAD    , do_raffs    , LVL_IMPL, 0 },
  { "rain"     , "rain"    , POS_DEAD    , do_rain     , LVL_BUILDER, 0 },
  { "reclass"  , "recl"    , POS_STANDING, do_reclass  , LVL_IMMORT, 0},
  { "reply"    , "rep"     , POS_SLEEPING, do_reply    , 0, 0 },
  { "read"     , "rea"     , POS_RESTING , do_look     , 0, SCMD_READ },
  { "reload"   , "reload"  , POS_DEAD    , do_reboot   , LVL_IMPL, 0 },
  { "recall"   , "recall"  , POS_STANDING, do_newb_recall, 0, 0 },
  { "recite"   , "reci"    , POS_RESTING , do_use      , 0, SCMD_RECITE },
  { "receive"  , "rece"    , POS_STANDING, do_not_here , 1, 0 },
  { "reelin"   , "reel"    , POS_STANDING, do_reelin   , 1, 0 },
  { "reform"   , "ref"     , POS_RESTING , do_reform   , 0, 0 },
  { "remove"   , "rem"     , POS_RESTING , do_remove   , 0, 0 },
  { "rent"     , "rent"    , POS_STANDING, do_not_here , 1, 0 },
  { "repair"   , "repair"  , POS_STANDING, do_not_here , 1, 0 },
  { "report"   , "repo"    , POS_RESTING , do_report   , 0, 0 },
  { "reroll"   , "rero"    , POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_REROLL },
  { "rescue"   , "resc"    , POS_FIGHTING, do_rescue   , 1, 0 },
  { "restore"  , "resto"   , POS_DEAD    , do_restore  , LVL_GOD, 0 },
  { "restring" , "restring", POS_DEAD    , do_restring , LVL_GOD, 0 },
  { "rall"     , "rall"    , POS_DEAD    , do_restore_all, LVL_IMPL, 0 },
  { "return"   , "retu"    , POS_DEAD    , do_return   , 0, 0 },
  { "redit"    , "redit"   , POS_DEAD    , do_oasis_redit    , LVL_BUILDER, 0 },
  { "rlist"    , "rlist"   , POS_DEAD    , do_oasis_list    , LVL_BUILDER, SCMD_OASIS_RLIST },
  { "rclone"   , "rclone"  , POS_DEAD    , do_oasis_copy, LVL_BUILDER, 0 },
  { "roomflags", "roomflags", POS_DEAD   , do_gen_tog  , LVL_GOD, SCMD_ROOMFLAGS },
  { "roadlist" , "roadlist", POS_DEAD    , do_listroad , LVL_IMPL, 0 },

  { "sacrifice", "sac"     , POS_RESTING , do_sacrifice, 0, 0 },
  { "'"        , "'"       , POS_RESTING , do_say      , 0, 0 },
  { "\""        , "\""       , POS_RESTING , do_say      , 0, 0 },
  { "save"     , "sav"     , POS_SLEEPING, do_save     , 0, 0 },
  { "saveall"  , "saveall" , POS_DEAD    , do_saveall  , LVL_BUILDER, 0},
  { "scales"   , "sca"     , POS_SLEEPING, do_scales   , 0, 0 },
  { "search"   , "sear"    , POS_STANDING, do_search   , 1, 0 },
  { "sell"     , "sell"    , POS_STANDING, do_not_here , 0, 0 },
  { "sedit"    , "sedit"   , POS_DEAD    , do_oasis_sedit , LVL_BUILDER, 0 },
  { "sense"    , "sense"   , POS_STANDING, do_sense    , 0, 0 },
//  { "send"     , "send"    , POS_SLEEPING, do_send     , LVL_GOD, 0 },
//  { "sendmail" , "sendm"    , POS_SLEEPING, do_sendmail     , LVL_GOD, 0 },
  { "set"      , "set"     , POS_DEAD    , do_set      , LVL_GOD, 0 },
  { "setlog"   , "setlog"  , POS_DEAD    , do_setlog   , LVL_GOD, 0 },
  { "shelter"  , "shelter" , POS_STANDING, do_shelter  , 0, 0 },
  { "shout"    , "sho"     , POS_RESTING , do_gen_comm , 0, SCMD_SHOUT },
  { "shoot"    , "shoot"   , POS_STANDING, do_shoot    , 0, 0 },
  { "show"     , "show"    , POS_DEAD    , do_show     , 0, 0 },
  { "shutdow"  , "shutdow" , POS_DEAD    , do_shutdown , LVL_IMPL, 0 },
  { "shutdown" , "shutdown", POS_DEAD    , do_shutdown , LVL_IMPL, SCMD_SHUTDOWN },
  { "sip"      , "sip"     , POS_RESTING , do_drink    , 0, SCMD_SIP },
  { "sit"      , "sit"     , POS_RESTING , do_sit      , 0, 0 },
  { "skillset" , "skillset", POS_SLEEPING, do_skillset , LVL_GRGOD, 0 },
  { "skin"     , "ski"     , POS_SLEEPING, do_skin     , 0, 0 },
  { "slay"     , "slay"    , POS_RESTING , do_slay     , LVL_GOD, 0 },
  { "sleep"    , "sl"      , POS_SLEEPING, do_sleep    , 0, 0 },
  { "slist"    , "slist"   , POS_SLEEPING, do_oasis_list    , LVL_BUILDER, SCMD_OASIS_SLIST },
  { "slowns"   , "slowns"  , POS_DEAD    , do_gen_tog  , LVL_IMPL, SCMD_SLOWNS },
  { "smithing" , "smith"   , POS_STANDING, do_smithery , 0, 0 },
  { "snatch"   , "snatch"  , POS_RESTING , do_snatch   , LVL_GRGOD, 0 },
  { "snoop"    , "snoop"   , POS_DEAD    , do_snoop    , LVL_GOD, 0 },
  { "socials"  , "socials" , POS_DEAD    , do_commands , 0, SCMD_SOCIALS },
  { "split"    , "split"   , POS_SITTING , do_split    , 1, 0 },
  { "stand"    , "st"      , POS_RESTING , do_stand    , 0, 0 },
  { "stat"     , "stat"    , POS_DEAD    , do_stat     , LVL_GOD, 0 },
  { "stats"    , "Stats"   , POS_DEAD    , do_stats    , 0, 0},
  { "steal"    , "ste"     , POS_STANDING, do_steal    , 1, 0 },
  { "switch"   , "switch"  , POS_DEAD    , do_switch   , LVL_GRGOD, 0 },
  { "symbols"  , "symbols" , POS_DEAD    , do_symbols  , LVL_BUILDER, 0 },
  { "syslog"   , "syslog"  , POS_DEAD    , do_syslog   , LVL_GOD, 0 }, 
  
  { "tell"     , "t"       , POS_DEAD    , do_tell     , 0, 0 },
  { "talk"     , "talk"    , POS_DEAD    , do_talk     , 0, 0 },
//  { "tlog"     , "tlog"    , POS_DEAD    , do_tell_log , 0, 0 },
  { "take"     , "ta"      , POS_RESTING , do_get      , 0, 0 },
  { "tame"     , "tam"     , POS_STANDING, do_tame     , 0, 0 },
  { "taste"    , "tas"     , POS_RESTING , do_eat      , 0, SCMD_TASTE },
  { "teleport" , "tele"    , POS_DEAD    , do_teleport , LVL_GOD, 0 },
  { "tedit"    , "tedit"   , POS_DEAD    , do_tedit    , LVL_GRGOD, 0 },  /* XXX: Oasisify */
  { "thaw"     , "thaw"    , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_THAW },
  { "title"    , "title"   , POS_DEAD    , do_title    , 0, 0 },
  { "time"     , "time"    , POS_DEAD    , do_time     , 0, 0 },
  { "toggle"   , "toggle"  , POS_DEAD    , do_toggle   , 0, 0 },
  { "tp"   , "tp"     , POS_DEAD    , do_tp   , LVL_IMPL, 0 },
  { "tplist"   , "tpl"     , POS_DEAD    , do_listtp   , LVL_IMPL, 0  },
  { "track"    , "track"   , POS_STANDING, do_track    , 0, 0 },
  { "trackthru", "trackthru",POS_DEAD    , do_gen_tog  , LVL_IMPL, SCMD_TRACK },
  { "train"    , "train"   , POS_DEAD    , do_not_here , 0, 0 },
  { "transfer" , "transfer", POS_SLEEPING, do_trans    , LVL_GOD, 0 },
  { "trip"     , "tri"     , POS_FIGHTING, do_trip     , 0, 0 },
  { "trigedit" , "trige"   , POS_DEAD    , do_oasis_trigedit, LVL_BUILDER, 0},
  { "tripwire" , "tripw"    , POS_STANDING, do_tripwire , 1, 0 },  
  { "tropic"   , "trop"    , POS_STANDING, do_not_here , 1, 0 },
  { "typo"     , "typo"    , POS_DEAD    , do_gen_write, 0, SCMD_TYPO },
  { "tag"      , "tag"     , POS_DEAD    , do_tag      , 0, 0 },
  { "throw"    , "thr"     , POS_FIGHTING, do_throw    , 0, 0 },

  { "unlock"   , "unlock"  , POS_SITTING , do_gen_door , 0, SCMD_UNLOCK },
  { "unban"    , "unban"   , POS_DEAD    , do_unban    , LVL_GRGOD, 0 },
  { "unbind"   , "unbind"  , POS_DEAD    , do_unbind   , LVL_GRGOD, 0 },
  { "untag"    , "unt"     , POS_DEAD    , do_untag    , LVL_GOD, 0},
  { "unaffect" , "unaffect", POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_UNAFFECT },
  { "unkey"    , "unkey"   , POS_DEAD    , do_unkey    , 0, 0 },
//  { "update"   , "upda"    , POS_DEAD    , do_update   , LVL_IMPL, 0 },
  { "uptime"   , "uptime"  , POS_DEAD    , do_date     , 1, SCMD_UPTIME },
  { "use"      , "use"     , POS_SITTING , do_use      , 1, SCMD_USE },
  { "users"    , "users"   , POS_DEAD    , do_users    , LVL_IMPL, 0 },

  { "value"    , "val"     , POS_STANDING, do_not_here , 0, 0 },
  { "version"  , "ver"     , POS_DEAD    , do_gen_ps   , 0, SCMD_VERSION },
  { "viewall"  , "viewall" , POS_DEAD    , do_viewall  , LVL_GOD, 0 },
  { "visible"  , "vis"     , POS_RESTING , do_visible  , 1, 0 },
  { "vnum"     , "vnum"    , POS_DEAD    , do_vnum     , LVL_GOD, 0 },
  { "vstat"    , "vstat"   , POS_DEAD    , do_vstat    , LVL_GOD, 0 },
  { "vwear"    , "vwear"   , POS_DEAD    , do_vwear    , LVL_GOD, 0 },
  { "vbugs"    , "vbugs"   , POS_DEAD    , do_vfile    , LVL_IMPL, SCMD_V_BUGS },
  { "videas"   , "videas"  , POS_DEAD    , do_vfile    , LVL_IMPL, SCMD_V_IDEAS },
  { "vtypos"   , "vtypos"  , POS_DEAD    , do_vfile    , LVL_IMPL, SCMD_V_TYPOS },

  { "wake"     , "wake"    , POS_SLEEPING, do_wake     , 0, 0 },
  { "warn"     , "warn"    , POS_DEAD    , do_warn     , LVL_GRGOD, 0 },
  { "wear"     , "wea"     , POS_RESTING , do_wear     , 0, 0 },
  { "weather"  , "weather" , POS_RESTING , do_weather  , 0, 0 },
  { "who"      , "who"     , POS_DEAD    , do_who      , 0, 0 },
  { "whoami"   , "whoami"  , POS_DEAD    , do_gen_ps   , 0, SCMD_WHOAMI },
  { "where"    , "where"   , POS_RESTING , do_where    , 1, 0 },
  { "whisper"  , "whisper" , POS_RESTING , do_spec_comm, 0, SCMD_WHISPER },
  { "wield"    , "wie"     , POS_RESTING , do_wield    , 0, 0 },
  { "wimpy"    , "wimpy"   , POS_DEAD    , do_wimpy    , 0, 0 },
  { "withdraw" , "withdraw", POS_STANDING, do_not_here , 1, 0 },
  { "wiznet"   , "wiz"     , POS_DEAD    , do_godchan  , LVL_IMMORT, SCMD_IMMORT },
  { ";"        , ";"       , POS_DEAD    , do_godchan  , LVL_IMMORT, SCMD_IMMORT },
  { "wizhelp"  , "wizhelp" , POS_DEAD    , do_wizhelp  , LVL_GOD, 0 },
  { "wizlist"  , "wizlist" , POS_DEAD    , do_wizlist  , 1, 0 },
  { "wizlock"  , "wizlock" , POS_DEAD    , do_wizlock  , LVL_IMPL, 0 },
  { "write"    , "write"   , POS_STANDING, do_write    , 1, 0 },
  { "worth"    , "worth"   , POS_STANDING, do_worth    , 1, 0 },

  { "xapobjs"  , "xapobjs" , POS_DEAD,     do_gen_tog  , LVL_IMPL, SCMD_XAP_OBJS },
  { "xprate"   , "xprate"  , POS_DEAD,     do_xprate   , LVL_IMPL, 0},

  { "zonemap"  , "zmap"    , POS_DEAD    , do_map      , 1, 0 },

  { "zreset"   , "zreset"  , POS_DEAD    , do_zreset   , LVL_GRGOD, 0 },
  { "zedit"    , "zedit"   , POS_DEAD    , do_oasis_zedit    , LVL_BUILDER, 0 },
  { "zonecount", "zonecount", POS_DEAD   , do_zonecount , LVL_GOD, 0 },
  { "zlist"    , "zlist"   , POS_DEAD    , do_oasis_list    , LVL_BUILDER, SCMD_OASIS_ZLIST },
  { "zlock"    , "zlock"   , POS_DEAD    , do_zlock    , LVL_BUILDER, 0 },
  { "zunlock"  , "zunlock" , POS_DEAD    , do_zunlock  , LVL_BUILDER, 0 },

  /* DG trigger commands */
  { "attach"   , "attach"  , POS_DEAD    , do_attach   , LVL_BUILDER, 0 },
  { "detach"   , "detach"  , POS_DEAD    , do_detach   , LVL_BUILDER, 0 },
  { "tlist"    , "tlist",POS_DEAD    , do_oasis_list    , LVL_BUILDER, SCMD_OASIS_TLIST },
  { "tstat"    , "tstat",POS_DEAD    , do_tstat    , LVL_BUILDER, 0 },
  { "masound"  , "masound",POS_DEAD    , do_masound  , -1, 0 },
  { "mkill"    , "mkill",POS_STANDING, do_mkill    , -1, 0 },
  { "mjunk"    , "mjunk",POS_SITTING , do_mjunk    , -1, 0 },
  { "mdamage"  , "mdamage", POS_DEAD    , do_mdamage  , -1, 0 },
  { "mdoor"    , "mdoor",POS_DEAD    , do_mdoor    , -1, 0 },
  { "mecho"    , "mecho",POS_DEAD    , do_mecho    , -1, 0 },
  { "mechoaround" , "mechoaround",POS_DEAD , do_mechoaround, -1, 0 },
  { "msend"    , "msend",POS_DEAD    , do_msend    , -1, 0 },
  { "mload"    , "mload",POS_DEAD    , do_mload    , -1, 0 },
  { "mpurge"   , "mpurge",POS_DEAD    , do_mpurge   , -1, 0 },
  { "mgoto"    , "mgoto",POS_DEAD    , do_mgoto    , -1, 0 },
  { "mat"      , "mat",POS_DEAD    , do_mat      , -1, 0 },
  { "mteleport", "mteleport",POS_DEAD    , do_mteleport, -1, 0 },
  { "mforce"   , "mforce",POS_DEAD    , do_mforce   , -1, 0 },
  { "mhunt"    , "mhunt",POS_DEAD    , do_mhunt    , -1, 0 },
  { "mremember", "mremember",POS_DEAD    , do_mremember, -1, 0 },
  { "mforget"  , "mforget",POS_DEAD    , do_mforget  , -1, 0 },
  { "mtransform","mtransform",POS_DEAD    , do_mtransform,-1, 0 },
  { "mzoneecho", "mzoneecho",POS_DEAD    , do_mzoneecho, -1, 0 },
  { "vdelete"  , "vdelete",POS_DEAD    , do_vdelete  , LVL_BUILDER, 0 },
  { "mfollow"  , "mfollow",POS_DEAD    , do_mfollow  , -1, 0 },


/*   LYO COMMANDS  */

 	{ "camp"        , "camp"        ,POS_STANDING  , do_camp       , 1, 0 },
        { "build2"       , "build2"       ,POS_STANDING  , do_build      , 1, 0 },
        { "bldlist"     , "bldlist"     ,POS_DEAD      , do_bldlist    , LVL_IMMORT, 0 },
        { "bldsave"     , "bldsave"     ,POS_DEAD      , do_bldsave    , LVL_IMMORT, 0 },
        { "charge"      , "charge"      ,POS_STANDING  , do_charge     , 0, 0 },
        { "courselist"  , "courselist"  ,POS_DEAD      , do_courselist , LVL_IMMORT, 0 },
//        { "discharge"   , "discharge"   , POS_STANDING  , do_discharge  , 0, 0 },
        { "disembark"   , "disembark"   ,POS_STANDING  , do_disembark  , 0, 0 },
        { "economy"     , "economy"     ,POS_STANDING  , do_economy    , LVL_GOD, 0 },
        { "embark"      , "embark"      ,POS_STANDING  , do_embark     , 0, 0 },
        { "newspaper"   , "newspaper"   ,POS_DEAD      , do_newspaper  , LVL_GOD, 0 },
        { "newmoney"    , "newmoney"    ,POS_DEAD      , do_newmoney   , LVL_GOD, 0 },
//        { "newbook"     , "newbook"     ,POS_DEAD      , do_newbook    , LVL_GOD, 0 },
        { "newvehicle"  , "newvehicle"  ,POS_STANDING  , do_newvehicle , LVL_GOD, 0 },
        { "newgoods"    , "newgoods"    ,POS_STANDING  , do_newgoods   , LVL_GOD, 0 },
        { "ship"        , "ship"        ,POS_STANDING  , do_ship       , 0, 0 },
//        { "politics"    , "politics"    ,POS_RESTING   , do_politics   , 0, 0 },
//        { "negotiate"   , "negotiate"   ,POS_RESTING   , do_negotiate  , 0, 0 },
        { "shipsetup"   , "shipsetup"   ,POS_DEAD      , do_shipsetup  , LVL_IMMORT, 0 },
        { "shipread"    , "shipread"    ,POS_DEAD      , do_shipread   , LVL_GRGOD, 0 },
//        { "trapremove"  , "trapremove"  ,POS_STANDING  , do_trapremove , 0, 0},
//        { "trapstat"    , "trapstat"    ,POS_STANDING  , do_trapstat   , 0, 0},
//        { "traplist"    , "traplist"    ,POS_DEAD      , do_traplist   , LVL_IMMORT, 0 },
        { "travel"      , "travel"      ,POS_STANDING  , do_travel     , 0, 0 },
        { "unyoke"      , "unyoke"      ,POS_STANDING  , do_unyoke     , 0, 0 },
        { "wildlist"    , "wildlist"    ,POS_DEAD      , do_wildlist   , LVL_IMMORT, 0 },
        { "yoke"        , "yoke"        ,POS_STANDING  , do_yoke       , 0, 0 },

  { "\n", "zzzzzzz", 0, 0, 0, 0 } };	/* this must be last */


const char *fill[] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

const char *reserved[] =
{
  "a",
  "an",
  "self",
  "me",
  "all",
  "room",
  "someone",
  "something",
  "\n"
};

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *argument)
{
  int cmd, length;
  char *line;
  char arg[MAX_INPUT_LENGTH];

  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
  
  

  /* just drop to next line for hitting CR */
  skip_spaces(&argument);
  if (!*argument){
     send_to_char(ch, "\r");
     return;
}
  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalpha(*argument)) {
    arg[0] = argument[0];
    arg[1] = '\0';
    line = argument + 1;
  } else
    line = any_one_arg(argument, arg);

  /* otherwise, find the command */
  {
    int cont;                                            /* continue the command checks */
    cont = command_wtrigger(ch, arg, line);              /* any world triggers ? */
    if (!cont) cont = command_mtrigger(ch, arg, line);   /* any mobile triggers ? */
    if (!cont) cont = command_otrigger(ch, arg, line);   /* any object triggers ? */
    if (cont) return;                                    /* yes, command trigger took over */
  }

  for (length = strlen(arg), cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
    if (!strncmp(complete_cmd_info[cmd].command, arg, length))
      if (GET_LEVEL(ch) >= complete_cmd_info[cmd].minimum_level)
	break;

  if (*complete_cmd_info[cmd].command == '\n')
    send_to_char(ch, "Huh?!?\r\n");
  else if (check_disabled(&complete_cmd_info[cmd]))					// is the command disabled?
    send_to_char(ch, "&RThis command is currently disabled.&n\r\n");
  else if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char(ch, "You try, but the mind-numbing cold prevents you...\r\n");
  else if (complete_cmd_info[cmd].command_pointer == NULL)
    send_to_char(ch, "Sorry, that command hasn't been implemented yet.\r\n");
  else if (IS_NPC(ch) && complete_cmd_info[cmd].minimum_level > LVL_IMMORT)
    send_to_char(ch, "You can't use immortal commands while switched.\r\n");
  else if (GET_POS(ch) < complete_cmd_info[cmd].minimum_position)
    switch (GET_POS(ch)) {
    case POS_DEAD:
      send_to_char(ch, "Lie still; you are DEAD!!! :-(\r\n");
      break;
    case POS_INCAP:
    case POS_MORTALLYW:
      send_to_char(ch, "You are in a pretty bad shape, unable to do anything!\r\n");
      break;
    case POS_STUNNED:
      send_to_char(ch, "All you can do right now is think about the stars!\r\n");
      break;
    case POS_SLEEPING:
      send_to_char(ch, "In your dreams, or what?\r\n");
      break;
    case POS_RESTING:
      send_to_char(ch, "Nah... You feel too relaxed to do that..\r\n");
      break;
    case POS_SITTING:
      send_to_char(ch, "Maybe you should get on your feet first?\r\n");
      break;
    case POS_FIGHTING:
      send_to_char(ch, "No way!  You're fighting for your life!\r\n");
      break;
  } else if (no_specials || !special(ch, cmd, line))
    ((*complete_cmd_info[cmd].command_pointer) (ch, line, cmd, complete_cmd_info[cmd].subcmd));
}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias_data *find_alias(struct alias_data *alias_list, char *str)
{
  while (alias_list != NULL) {
    if (*str == *alias_list->alias)	/* hey, every little bit counts :-) */
      if (!strcmp(str, alias_list->alias))
	return (alias_list);

    alias_list = alias_list->next;
  }

  return (NULL);
}


void free_alias(struct alias_data *a)
{
  if (a->alias)
    free(a->alias);
  if (a->replacement)
    free(a->replacement);
  free(a);
}


/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
  char arg[MAX_INPUT_LENGTH];
  char *repl;
  struct alias_data *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {			/* no argument specified -- list currently defined aliases */
    send_to_char(ch, "Currently defined aliases:\r\n");
    if ((a = GET_ALIASES(ch)) == NULL)
      send_to_char(ch, " None.\r\n");
    else {
      while (a != NULL) {
	send_to_char(ch, "%-15s %s\r\n", a->alias, a->replacement);
	a = a->next;
      }
    }
  } else {			/* otherwise, add or remove aliases */
    /* is this an alias we've already defined? */
    if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
      free_alias(a);
    }
    /* if no replacement string is specified, assume we want to delete */
    if (!*repl) {
      if (a == NULL)
	send_to_char(ch, "No such alias.\r\n");
      else
	send_to_char(ch, "Alias deleted.\r\n");
    } else {			/* otherwise, either add or redefine an alias */
      if (!str_cmp(arg, "alias")) {
	send_to_char(ch, "You can't alias 'alias'.\r\n");
	return;
      }
      CREATE(a, struct alias_data, 1);
      a->alias = strdup(arg);
      delete_doubledollar(repl);
      a->replacement = strdup(repl);
      if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
	a->type = ALIAS_COMPLEX;
      else
	a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      send_to_char(ch, "Alias added.\r\n");
    }
  }
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias_data *a)
{
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  char buf2[MAX_RAW_INPUT_LENGTH], buf[MAX_RAW_INPUT_LENGTH];	/* raw? */
  int num_of_tokens = 0, num;

  /* First, parse the original string */
  strcpy(buf2, orig);	/* strcpy: OK (orig:MAX_INPUT_LENGTH < buf2:MAX_RAW_INPUT_LENGTH) */
  temp = strtok(buf2, " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS) {
    tokens[num_of_tokens++] = temp;
    temp = strtok(NULL, " ");
  }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++) {
    if (*temp == ALIAS_SEP_CHAR) {
      *write_point = '\0';
      buf[MAX_INPUT_LENGTH - 1] = '\0';
      write_to_q(buf, &temp_queue, 1);
      write_point = buf;
    } else if (*temp == ALIAS_VAR_CHAR) {
      temp++;
      if ((num = *temp - '1') < num_of_tokens && num >= 0) {
	strcpy(write_point, tokens[num]);	/* strcpy: OK */
	write_point += strlen(tokens[num]);
      } else if (*temp == ALIAS_GLOB_CHAR) {
	strcpy(write_point, orig);		/* strcpy: OK */
	write_point += strlen(orig);
      } else if ((*(write_point++) = *temp) == '$')	/* redouble $ for act safety */
	*(write_point++) = '$';
    } else
      *(write_point++) = *temp;
  }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q(buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else {
    temp_queue.tail->next = input_q->head;
    input_q->head = temp_queue.head;
  }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig, size_t maxlen)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias_data *a, *tmp;

  /* Mobs don't have alaises. */
  if (IS_NPC(d->character))
    return (0);

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES(d->character)) == NULL)
    return (0);

  /* find the alias we're supposed to match */
  ptr = any_one_arg(orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return (0);

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias(tmp, first_arg)) == NULL)
    return (0);

  if (a->type == ALIAS_SIMPLE) {
    strlcpy(orig, a->replacement, maxlen);
    return (0);
  } else {
    perform_complex_alias(&d->input, ptr, a);
    return (1);
  }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, const char **list, int exact)
{
  int i, l;

  /*  We used to have \r as the first character on certain array items to
   *  prevent the explicit choice of that point.  It seems a bit silly to
   *  dump control characters into arrays to prevent that, so we'll just
   *  check in here to see if the first character of the argument is '!',
   *  and if so, just blindly return a '-1' for not found. - ae.
   */
  if (*arg == '!')
    return (-1);

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;			/* Avoid "" to match the first available
				 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return (i);
  }

  return (-1);
}


int is_number(const char *str)
{
  while (*str)
    if (!isdigit(*(str++)))
      return (0);

  return (1);
}

/*
 * Function to skip over the leading spaces of a string.
 */
void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}


/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string)
{
  char *ddread, *ddwrite;

  /* If the string has no dollar signs, return immediately */
  if ((ddwrite = strchr(string, '$')) == NULL)
    return (string);

  /* Start from the location of the first dollar sign */
  ddread = ddwrite;


  while (*ddread)   /* Until we reach the end of the string... */
    if ((*(ddwrite++) = *(ddread++)) == '$') /* copy one char */
      if (*ddread == '$')
	ddread++; /* skip if we saw 2 $'s in a row */

  *ddwrite = '\0';

  return (string);
}

char *delete_doubleat(char *string)
{
  char *ddread, *ddwrite;

  /* If the string has no dollar signs, return immediately */
  if ((ddwrite = strchr(string, '@')) == NULL)
    return (string);

  /* Start from the location of the first dollar sign */
  ddread = ddwrite;


  while (*ddread)   /* Until we reach the end of the string... */
    if ((*(ddwrite++) = *(ddread++)) == '@') /* copy one char */
      if (*ddread == '@')
	ddread++; /* skip if we saw 2 $'s in a row */

  *ddwrite = '\0';

  return (string);
}


int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}


int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
  char *begin = first_arg;

  if (!argument) {
    log("SYSERR: one_argument received a NULL pointer!");
    *first_arg = '\0';
    return (NULL);
  }

  do {
    skip_spaces(&argument);

    first_arg = begin;
    while (*argument && !isspace(*argument)) {
      *(first_arg++) = LOWER(*argument);
      argument++;
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return (argument);
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;

    if (*argument == '\"') {
      argument++;
      while (*argument && *argument != '\"') {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
      argument++;
    } else {
      while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return (argument);
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace(*argument)) {
    *(first_arg++) = LOWER(*argument);
    argument++;
  }

  *first_arg = '\0';

  return (argument);
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return (one_argument(one_argument(argument, first_arg), second_arg)); /* :-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 <S-F2>*
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 * 
 * returns 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(const char *arg1, const char *arg2)
{
  if (!*arg1)
    return (0);

  if (sizeof(arg1) > sizeof(arg2))
    return(0);
  
  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return (0);

  if (!*arg1)
    return (1);
  else
    return (0);
}



/*
 * Return first space-delimited token in arg1; remainder of string in arg2.
 *
 * NOTE: Requires sizeof(arg2) >= sizeof(string)
 */
void half_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg(string, arg1);
  skip_spaces(&temp);
  strcpy(arg2, temp);	/* strcpy: OK (documentation) */
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(const char *command)
{
  int cmd;

  for (cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(complete_cmd_info[cmd].command, command))
      return (cmd);

  return (-1);
}


int special(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data *i;
  struct char_data *k;
//  int j;

//  if(*arg)
//     skip_spaces(arg);

  /* special in room? */
  if (GET_ROOM_SPEC(IN_ROOM(ch)) != NULL)
    if (GET_ROOM_SPEC(IN_ROOM(ch)) (ch, world + IN_ROOM(ch), cmd, arg))
      return (1);

  /* special in equipment list? */
/*  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
      if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
	return (1);
*/
  /* special in inventory? */
/*  for (i = ch->carrying; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return (1);
*/
  /* special in mobile present? */
  for (k = world[IN_ROOM(ch)].people; k; k = k->next_in_room)
    if (!MOB_FLAGGED(k, MOB_NOTDEADYET))
      if (GET_MOB_SPEC(k) && GET_MOB_SPEC(k) (ch, k, cmd, arg))
	return (1);

  /* special in object present? */
  for (i = world[IN_ROOM(ch)].contents; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return (1);

  return (0);
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* This function needs to die. */
int _parse_name(char *arg, char *name)
{
  int i;

  skip_spaces(&arg);
  for (i = 0; (*name = *arg); arg++, i++, name++)
    if (!isalpha(*arg))
      return (1);

  if (!i)
    return (1);

  return (0);
}


#define RECON		1
#define USURP		2
#define UNSWITCH	3

/* This function seems a bit over-extended. */
int perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;

    if (k == d)
      continue;

    if (k->original && (GET_IDNUM(k->original) == id)) {
      /* Original descriptor was switched, booting it and restoring normal body control. */

      write_to_output(d, "\r\nMultiple login detected -- disconnecting.\r\n");
      STATE(k) = CON_CLOSE;
      if	 (!target) {
	target = k->original;
	mode = UNSWITCH;
      }
      if (k->character)
	k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
    } else if (k->character && GET_IDNUM(k->character) == id && k->original) {
      /* Character taking over their own body, while an immortal was switched to it. */

      do_return(k->character, NULL, 0, 0);
    } else if (k->character && GET_IDNUM(k->character) == id) {
      /* Character taking over their own body. */

      if (!target && STATE(k) == CON_PLAYING) {
	write_to_output(k, "\r\nThis body has been usurped!\r\n");
	target = k->character;
	mode = USURP;
      }
      k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
      write_to_output(k, "\r\nMultiple login detected -- disconnecting.\r\n");
      STATE(k) = CON_CLOSE;
    }
  }

 /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (IS_NPC(ch))
      continue;
    if (GET_IDNUM(ch) != id)
      continue;

    /* ignore chars with descriptors (already handled by above step) */
    if (ch->desc)
      continue;

    /* don't extract the target char we've found one already */
    if (ch == target)
      continue;

    /* we don't already have a target and found a candidate for switching */
    if (!target) {
      target = ch;
      mode = RECON;
      continue;
    }

    /* we've found a duplicate - blow him away, dumping his eq in limbo. */
    if (IN_ROOM(ch) != NOWHERE)
      char_from_room(ch);
    char_to_room(ch, 1);
    extract_char(ch);
  }

  /* no target for switching into was found - allow login to continue */
  if (!target)
    return (0);

  /* Okay, we've found a target.  Connect d to target. */
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
  REMOVE_BIT_AR(AFF_FLAGS(d->character), AFF_GROUP);
  STATE(d) = CON_PLAYING;

  switch (mode) {
  case RECON:
    write_to_output(d, "Reconnecting.\r\n");
    act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
    nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s [%s] has reconnected", GET_NAME(d->character), d->host);
//    mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    /* Show if any new mudmail */
    notify_on_login(d->character);
    break;
  case USURP:
    write_to_output(d, "You take over your own body, already in use!\r\n");
    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
	"$n's body has been taken over by a new spirit!",
	TRUE, d->character, 0, 0, TO_ROOM);
    nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s has re-logged in and the old socket is disconnected", GET_NAME(d->character));
//    mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE,
//	"%s has re-logged in ... disconnecting old socket.", GET_NAME(d->character));
    /* Show if any new mudmail */
    notify_on_login(d->character);
    break;
  case UNSWITCH:
    write_to_output(d, "Reconnecting to unswitched char.");
    nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s [%s] has reconnected", GET_NAME(d->character), d->host);
//    mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    break;
  }

  return (1);
}

int enter_player_game (struct descriptor_data *d)
{
    int load_result;
    room_vnum load_room;

      reset_char(d->character);
      load_warnings(d->character);
   
      if (PLR_FLAGGED(d->character, PLR_INVSTART))
        GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);

      /* We have to place the character in a room before equipping them
 *        * or equip_char() will gripe about the person in NOWHERE. */
      if ((load_room = GET_LOADROOM(d->character)) != NOWHERE)
        load_room = real_room(load_room);

      /* If char was saved with NOWHERE, or real_room above failed... */
      if (load_room == NOWHERE) {
        if (GET_LEVEL(d->character) > LVL_IMMORT)
          load_room = r_immort_start_room;
        else
          load_room = r_mortal_start_room;
      }

      if (PLR_FLAGGED(d->character, PLR_FROZEN))
        load_room = r_frozen_start_room;

      /* copyover */
      GET_ID(d->character) = GET_IDNUM(d->character);
      /* find_char helper */
      add_to_lookup_table(GET_ID(d->character), (void *)d->character);

      /* After moving saving of variables to the player file, this should only
 *        * be called in case nothing was found in the pfile. If something was
 *               * found, SCRIPT(ch) will be set. */
      if (!SCRIPT(d->character))
        read_saved_vars(d->character);

      d->character->next = character_list;
      character_list = d->character;
      char_to_room(d->character, load_room);
      load_result = Crash_load(d->character);
      save_char(d->character); 
//      kill_warnings(GET_NAME(d->character));

    return load_result;
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
  int load_result;	/* Overloaded variable */
  int player_i, i;
  struct descriptor_data *pt;
//  struct clan_type *cptr = NULL;
//  FILE *ofile;
//  char buf[256];

  /* OasisOLC states */
  struct {
    int state;
    void (*func)(struct descriptor_data *, char *);
  } olc_functions[] = {
    { CON_OEDIT, oedit_parse },
    { CON_ZEDIT, zedit_parse },
    { CON_SEDIT, sedit_parse },
    { CON_MEDIT, medit_parse },
    { CON_REDIT, redit_parse },
    { CON_CEDIT, cedit_parse },
    { CON_AEDIT, aedit_parse },
    { CON_QEDIT, qedit_parse },
    { CON_CLANEDIT, clanedit_parse },
    { CON_TRIGEDIT, trigedit_parse },
    { CON_MAILEDIT,  mailedit_parse },
    { CON_HEDIT,  hedit_parse },
    { -1, NULL }
  };
  
  skip_spaces(&arg);

  /*
   * Quick check for the OLC states.
   */
  for (player_i = 0; olc_functions[player_i].state >= 0; player_i++)
    if (STATE(d) == olc_functions[player_i].state) {
      /* send context-sensitive help if need be */
      (*olc_functions[player_i].func)(d, arg);
      return;
    }

  /* Not in OLC. */
  switch (STATE(d)) {
  case CON_GET_NAME:		/* wait for input of name */
    if (d->character == NULL) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      CREATE(d->character->player_specials, struct player_special_data, 1);
      d->character->desc = d;
    }
    if (!*arg)
      STATE(d) = CON_CLOSE;
    else {
      char buf[MAX_INPUT_LENGTH], tmp_name[MAX_INPUT_LENGTH];

      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
	  strlen(tmp_name) > MAX_NAME_LENGTH || !valid_name(tmp_name) ||
	  fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {	/* strcpy: OK (mutual MAX_INPUT_LENGTH) */
	write_to_output(d, "Invalid name, please try another.\r\nName: ");
	return;
      }
      if ((player_i = load_char(tmp_name, d->character)) > -1) {	
        GET_PFILEPOS(d->character) = player_i;

	if (PLR_FLAGGED(d->character, PLR_DELETED)) {
	  /* make sure old files are removed so the new player doesn't get
	     the deleted player's equipment (this should probably be a
	     stock behavior)
	  */
	  if((player_i = get_ptable_by_name(tmp_name)) >= 0)
	    remove_player(player_i);

	  /* We get a false positive from the original deleted character. */
	  free_char(d->character);
	  /* Check for multiple creations... */
	  if (!valid_name(tmp_name)) {
	    write_to_output(d, "Invalid name, please try another.\r\nName: ");
	    return;
	  }
	  CREATE(d->character, struct char_data, 1);
	  clear_char(d->character);
	  CREATE(d->character->player_specials, struct player_special_data, 1);
	  d->character->desc = d;
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));	/* strcpy: OK (size checked above) */
	  GET_PFILEPOS(d->character) = player_i;
	  write_to_output(d, "Did I get that right, %s (Y/N)? ", tmp_name);
	  STATE(d) = CON_NAME_CNFRM;
	} else {
	  /* undo it just in case they are set */
	  REMOVE_BIT_AR(PLR_FLAGS(d->character),
		     PLR_WRITING | PLR_MAILING | PLR_CRYO);
	  REMOVE_BIT_AR(AFF_FLAGS(d->character), AFF_GROUP);
	  write_to_output(d, "Password: ");
	  echo_off(d);
	  d->idle_tics = 0;
	  STATE(d) = CON_PASSWORD;
	}
      } else {
	/* player unknown -- make new character */

	/* Check for multiple creations of a character. */
	if (!valid_name(tmp_name)) {
	  write_to_output(d, "Invalid name, please try another.\r\nName: ");
	  return;
	}
	CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	strcpy(d->character->player.name, CAP(tmp_name));	/* strcpy: OK (size checked above) */

	write_to_output(d, "Did I get that right, %s (Y/N)? ", tmp_name);
	STATE(d) = CON_NAME_CNFRM;
      }
    }
    break;

  case CON_NAME_CNFRM:		/* wait for conf. of new name    */
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
        nmudlog(CONN_LOG, LVL_GOD, TRUE, "New character request for %s was denied from %s: Reason [SITEBAN]", GET_PC_NAME(d->character), d->host); 
//	mudlog(NRM, LVL_GOD, TRUE, "Request for new char %s denied from [%s] (siteban)", GET_PC_NAME(d->character), d->host);
	write_to_output(d, "Sorry, new characters are not allowed from your site!\r\n");
	STATE(d) = CON_CLOSE;
	return;
      }
      if (circle_restrict) {
	write_to_output(d, "Sorry, new players can't be created at the moment.\r\n");
        nmudlog(CONN_LOG, LVL_GOD, TRUE, "New character request for %s was denied from %s: Reason [WIZLOCK]", GET_PC_NAME(d->character), d->host);
//	mudlog(NRM, LVL_GOD, TRUE, "Request for new char %s denied from [%s] (wizlock)", GET_PC_NAME(d->character), d->host);
	STATE(d) = CON_CLOSE;
	return;
      }
      write_to_output(d, "New character.\r\nGive me a password for %s: ", GET_PC_NAME(d->character));
      echo_off(d);
      STATE(d) = CON_NEWPASSWD;
    } else if (*arg == 'n' || *arg == 'N') {
      write_to_output(d, "Okay, what IS it, then? ");
      free(d->character->player.name);
      d->character->player.name = NULL;
      STATE(d) = CON_GET_NAME;
    } else
      write_to_output(d, "Please type Yes or No: ");
    break;

  case CON_PASSWORD:		/* get pwd for known player      */
    /*
     * To really prevent duping correctly, the player's record should
     * be reloaded from disk at this point (after the password has been
     * typed).  However I'm afraid that trying to load a character over
     * an already loaded character is going to cause some problem down the
     * road that I can't see at the moment.  So to compensate, I'm going to
     * (1) add a 15 or 20-second time limit for entering a password, and (2)
     * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
     */

    echo_on(d);    /* turn echo back on */

    /* New echo_on() eats the return on telnet. Extra space better than none. */
    write_to_output(d, "\r\n");

    if (!*arg)
      STATE(d) = CON_CLOSE;
    else {
      if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
        if (CONFIG_PWD_RETRIEVAL && !strcmp(arg, "forgot") && GET_BAD_PWS(d->character) > 1) {
         recover_password(d);
         STATE(d) = CON_CLOSE;
        return;
         }
        nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s has entered a bad password [%s]", GET_NAME(d->character), d->host);
//	mudlog(BRF, LVL_GOD, TRUE, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	GET_BAD_PWS(d->character)++;
	save_char(d->character);
	if (++(d->bad_pws) >= CONFIG_MAX_BAD_PWS) {	/* 3 strikes and you're out. */
	  write_to_output(d, "Wrong password... disconnecting.\r\n");
	  STATE(d) = CON_CLOSE;
	} else {
          if (CONFIG_PWD_RETRIEVAL && GET_EMAIL(d->character) != NULL) {
           write_to_output(d, "Wrong password.\r\n");
           write_to_output(d, "If you have forgotten your password, enter 'forgot' to receive a new one.\r\n");
           write_to_output(d, "Password: ");
         } else {

	  write_to_output(d, "Wrong password.\r\nPassword: ");
         }
	  echo_off(d);
	}
	return;
      }

      /* Password was correct. */
      
      load_result = GET_BAD_PWS(d->character);
      GET_BAD_PWS(d->character) = 0;
      d->bad_pws = 0;
        
      if (isbanned(d->host) == BAN_SELECT &&
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	write_to_output(d, "Sorry, this char has not been cleared for login from your site!\r\n");
	STATE(d) = CON_CLOSE;
        nmudlog(CONN_LOG, LVL_GOD, TRUE, "Connection attempt for %s denied from %s", GET_NAME(d->character), d->host);
//	mudlog(NRM, LVL_GOD, TRUE, "Connection attempt for %s denied from %s", GET_NAME(d->character), d->host);
	return;
      }
      if (GET_LEVEL(d->character) < circle_restrict) {
	write_to_output(d, "The game is temporarily restricted.. try again later.\r\n");
	STATE(d) = CON_CLOSE;
        nmudlog(CONN_LOG, LVL_GOD, TRUE, "Request for login denied for %s [%s]: Reason [WIZLOCK]", GET_NAME(d->character), d->host);
//	mudlog(NRM, LVL_GOD, TRUE, "Request for login denied for %s [%s] (wizlock)", GET_NAME(d->character), d->host);
	return;
      }
      /* check and make sure no other copies of this player are logged in */
      if (perform_dupe_check(d))
	return;

      if (GET_LEVEL(d->character) > LVL_IMMORT)
	write_to_output(d, "%s", imotd);
      else 
	write_to_output(d, "%s", motd);

      if (GET_INVIS_LEV(d->character)) 
        nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s [%s] has connected (invis %ld)", GET_NAME(d->character), d->host, GET_INVIS_LEV(d->character));
//        mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE, 
//        "%s [%s] has connected. (invis %ld)", GET_NAME(d->character), d->host, 
//        GET_INVIS_LEV(d->character));
      else
        nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s [%s] has connected", GET_NAME(d->character), d->host);
//        mudlog(BRF, LVL_IMMORT, TRUE, 
//               "%s [%s] has connected.", GET_NAME(d->character), d->host);

      if (load_result) {
        write_to_output(d, "\r\n\r\n\007\007\007"
		"%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
		CCRED(d->character, C_SPR), load_result,
		(load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
	GET_BAD_PWS(d->character) = 0;
      }
      write_to_output(d, "\r\n*** PRESS RETURN: ");
      STATE(d) = CON_RMOTD;
    }
    break;

  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_PC_NAME(d->character))) {
      write_to_output(d, "\r\nIllegal password.\r\nPassword: ");
      return;
    }
    strncpy(GET_PASSWD(d->character), CRYPT(arg, GET_PC_NAME(d->character)), MAX_PWD_LENGTH);	/* strncpy: OK (G_P:MAX_PWD_LENGTH+1) */
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

    write_to_output(d, "\r\nPlease retype password: ");
    if (STATE(d) == CON_NEWPASSWD)
      STATE(d) = CON_CNFPASSWD;
    else
      STATE(d) = CON_CHPWD_VRFY;
    break;

  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
		MAX_PWD_LENGTH)) {
      write_to_output(d, "\r\nPasswords don't match... start over.\r\nPassword: ");
      if (STATE(d) == CON_CNFPASSWD)
	STATE(d) = CON_NEWPASSWD;
      else
	STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    echo_on(d);

    if (STATE(d) == CON_CNFPASSWD) {
//     write_to_output(d, "\r\nWhat is your sex (M/F)? ");
//     STATE(d) = CON_QSEX;
      write_to_output(d, "\r\nNOTE: Your email address will only be used to retrieve a lost password.  You do");
      write_to_output(d, "\r\n      have to enter an email address, but if your don't then password retrieval");
      write_to_output(d, "\r\n      is more difficult.  ");
      write_to_output(d, "\r\n      If you do not wish to enter an email, then just press enter.");
      write_to_output(d, "\r\nEnter your email address: ");
      STATE(d) = CON_QEMAIL;
    } else {
      save_char(d->character);
      write_to_output(d, "\r\nDone.\r\n%s", CONFIG_MENU);
      STATE(d) = CON_MENU;
    }
    break;

  case CON_QEMAIL:
    if (arg == NULL || !(*arg)) {
      GET_EMAIL(d->character) = NULL;
      write_to_output(d, "\r\nEmail address left blank.  You can change it later with the email command.");
    } else {
	  if ((i = validate_email(arg)) != EMAIL_SUCCESS) {
        write_to_output(d, "\r\n%s\r\nEnter your email address: ", email_error_message(i));
        return;
	  }
	  set_email(d->character, arg);
    }
    write_to_output(d, "\r\nWhat is your sex (M/F)? ");
    STATE(d) = CON_QSEX;
    break;

  case CON_QSEX:		/* query sex of new user         */
    switch (*arg) {
    case 'm':
    case 'M':
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      write_to_output(d, "That is not a sex..\r\n"
		"What IS your sex? ");
      return;
    }

    write_to_output(d, "%s\r\nClass: ", class_menu);
    STATE(d) = CON_QCLASS;
    break;

  case CON_QCLASS:
    load_result = parse_class(*arg);
    if (load_result == CLASS_UNDEFINED) {
      write_to_output(d, "\r\nThat's not a class.\r\nClass: ");
      return;
    } else
      GET_CLASS(d->character) = load_result;

    if (GET_PFILEPOS(d->character) < 0)
      GET_PFILEPOS(d->character) = create_entry(GET_PC_NAME(d->character));
    /* Now GET_NAME() will work properly. */
    init_char(d->character);
    save_char(d->character);
    save_player_index();
    write_to_output(d, "%s\r\n*** PRESS RETURN: ", motd);
    STATE(d) = CON_RMOTD;

    nmudlog(CONN_LOG, LVL_GOD, TRUE, "%s [%s] is a NEW player!", GET_NAME(d->character), d->host);
//    mudlog(NRM, LVL_IMMORT, TRUE, "%s [%s] new player.", GET_NAME(d->character), d->host);
    break;

  case CON_RMOTD:		/* read CR after printing motd   */
    write_to_output(d, "\r\n%s", CONFIG_MENU);
    STATE(d) = CON_MENU;
    break;

  case CON_MENU: {		/* get selection from main menu  */
    switch (*arg) {

    case 'n':
    case 'N':
      load_result = enter_player_game(d);
      send_to_char(d->character, "%s", CONFIG_WELC_MESSG);

      REMOVE_BIT_AR(PRF_FLAGS(d->character), PRF_COLOR_2);

      SET_BIT_AR(PRF_FLAGS(d->character), PRF_COLOR_2);

      /* If you have the copyover patch installed, the following goes in enter_player_game as well */
      /* Check for new clans for leader */
/*      if (GET_CLAN(d->character) == PFDEF_CLAN) {
        for (cptr = clan_info; cptr; cptr = cptr->next) {
          if (!strcmp(GET_NAME(d->character), cptr->leadersname))
            GET_CLAN(d->character) = cptr->number;
        }
      }
*/
      /* can't do an 'else' here, cuz they might have a clan now. */
//      if (GET_CLAN(d->character) != PFDEF_CLAN) {
        /* Now check to see if person's clan still exists */
//        for (cptr = clan_info; cptr && cptr->number != GET_CLAN(d->character); cptr = cptr->next);

//        if (cptr == NULL) {  /* Clan no longer exists */
//          GET_CLAN(d->character) = PFDEF_CLAN;
//          GET_CLAN_RANK(d->character) = PFDEF_CLANRANK;
//          GET_HOME(d->character) = 1;
//        } else {  /* Was there a change of leadership? */
//          if (!strcmp(GET_NAME(d->character), cptr->leadersname))
//            GET_CLAN_RANK(d->character) = CLAN_LEADER;
//        }
//      }

      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_SMOKE);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_BLAZE);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_CANN);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_SATI);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_PARTYIN);
//      REMOVE_BIT(PLR_FLAGS(d->character), PLR_AFTERLIFE);

            /* with the copyover patch, this next line goes in enter_player_game() */
       greet_mtrigger(d->character, -1);
       greet_memory_mtrigger(d->character);

       for (pt = descriptor_list; pt; pt = pt->next)
       {
          if (IS_PLAYING(pt))
             send_to_char(pt->character, "&W%s has just logged into the system.&n\r\n",GET_NAME(d->character));
       }

       STATE(d) = CON_PLAYING;
       if (GET_LEVEL(d->character) == 0) {
          do_start(d->character);
          send_to_char(d->character, "%s", CONFIG_START_MESSG);
       }
       look_at_room(d->character, 0);
       if (has_mail(d->character))
          send_to_char(d->character, "&R[You have mail waiting.]&n\r\n");
       d->has_prompt = 0;
       break;

    case 'y':
    case 'Y':
      load_result = enter_player_game(d);
      send_to_char(d->character, "%s", CONFIG_WELC_MESSG);

      SET_BIT_AR(PRF_FLAGS(d->character), PRF_COLOR_2);

      /* If you have the copyover patch installed, the following goes in enter_player_game as well */
      /* Check for new clans for leader */
//      if (GET_CLAN(d->character) == PFDEF_CLAN) {
//        for (cptr = clan_info; cptr; cptr = cptr->next) {
//          if (!strcmp(GET_NAME(d->character), cptr->leadersname))
//            GET_CLAN(d->character) = cptr->number;
//        }
//      }
                                                                                                         
      /* can't do an 'else' here, cuz they might have a clan now. */
//      if (GET_CLAN(d->character) != PFDEF_CLAN) {
        /* Now check to see if person's clan still exists */
//        for (cptr = clan_info; cptr && cptr->number != GET_CLAN(d->character); cptr = cptr->next);
                                                                                                         
//        if (cptr == NULL) {  /* Clan no longer exists */
//          GET_CLAN(d->character) = PFDEF_CLAN;
//          GET_CLAN_RANK(d->character) = PFDEF_CLANRANK;
//          GET_HOME(d->character) = 1;
//        } else {  /* Was there a change of leadership? */
//          if (!strcmp(GET_NAME(d->character), cptr->leadersname))
//            GET_CLAN_RANK(d->character) = CLAN_LEADER;
//        }
//      }


      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_SMOKE);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_BLAZE);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_CANN);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_TAKE_SATI);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_PARTYIN);
//      REMOVE_BIT(PLR_FLAGS(d->character), PLR_AFTERLIFE);

      /* with the copyover patch, this next line goes in enter_player_game() */
      greet_mtrigger(d->character, -1);
      greet_memory_mtrigger(d->character);

      for (pt = descriptor_list; pt; pt = pt->next)
      {
         if (IS_PLAYING(pt))
            send_to_char(pt->character, "&W%s has just logged into the system.&n\r\n",GET_NAME(d->character));
      }

      STATE(d) = CON_PLAYING;
      if (GET_LEVEL(d->character) == 0) {
	do_start(d->character);
	send_to_char(d->character, "%s", CONFIG_START_MESSG);
      }
      look_at_room(d->character, 0);
      /* Show if any new mudmail */
      notify_on_login(d->character);

      d->has_prompt = 0;
      break;

    default:
      write_to_output(d, "\r\n%s", CONFIG_MENU);
      break;
    }
    break;
  }

  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      echo_on(d);
      write_to_output(d, "\r\nIncorrect password.\r\n%s", CONFIG_MENU);
      STATE(d) = CON_MENU;
    } else {
      write_to_output(d, "\r\nEnter a new password: ");
      STATE(d) = CON_CHPWD_GETNEW;
    }
    return;
  /*
   * It's possible, if enough pulses are missed, to kick someone off
   * while they are at the password prompt. We'll just defer to let
   * the game_loop() axe them.
   */
  case CON_CLOSE:
    save_char(d->character);   
    break;

  default:
    log("SYSERR: Nanny: illegal state of con'ness (%d) for '%s'; closing connection.",
	STATE(d), d->character ? GET_NAME(d->character) : "<unknown>");
    STATE(d) = CON_DISCONNECT;	/* Safest to do. */
    break;
  }
  SET_BIT_AR(PRF_FLAGS(d->character), PRF_COLOR_2);
}


// DISABLE COMMANDS FUNCTIONS

// disable - shows list of commands that are disabled
// disable <command> - toggles commands on or off

ACMD(do_disable)
{
  int i, length;
  DISABLED_DATA *p, *temp;

  if (IS_NPC(ch)) {
    send_to_char(ch, "Monsters cannot disable commands.\r\n");
  }
  
  skip_spaces(&argument);

  if (!*argument) { //nothing specified, show list of disabled commands
    if (!disabled_first) //any disabled?
      send_to_char(ch, "&GThere are no disabled commands, yay!\r\n");
    else
    {
      send_to_char(ch, 
         "&CCommands that are currently disabled:\r\n\r\n"
         "&W Command       Disabled by     Level\r\n"
         "&W-------------------------------------\r\n");
        for (p = disabled_first; p; p = p->next)
          send_to_char(ch, "&R %-12s   &M%-12s     %3ld\r\n", p->command->command, p->disabled_by, p->level);
    }
    return;
  }

// command is given, check to see if its already disabled first

  for(length = strlen(argument), p = disabled_first; p ; p = p->next)
     if (!strncmp(argument, p->command->command, length))
      break;

  if (p) {		//This command was disabled
    if (GET_LEVEL(ch) < p->level) {
      send_to_char(ch, "This command was disabled by a higher power.\r\n");
      return;
    }

  REMOVE_FROM_LIST(p, disabled_first, next);
  send_to_char(ch, "&BCommand '&W%s&B' enabled.&n\r\n", p->command->command);
  nmudlog(SYS_LOG, LVL_GOD, TRUE, "%s has enabled the command '%s'", GET_NAME(ch), p->command->command);
//  mudlog(BRF, LVL_GOD, TRUE, "(GC) %s has enabled the command '%s'.", GET_NAME(ch), p->command->command);
  free(p->disabled_by);
  free(p);
  save_disabled();	// save it to disk for bootup
  }
  else 			// not disabled, see if the command exists
  {
       for (length = strlen(argument), i = 0; *cmd_info[i].command != '\n'; i++)
      if (!strncmp(cmd_info[i].command, argument, length))
        if (GET_LEVEL(ch) >= cmd_info[i].minimum_level)
          break;

    // Found it?
    if (*cmd_info[i].command == '\n') {
      send_to_char(ch, "You don't know of any such command.\r\n");
      return;
    }

    if (!strcmp(cmd_info[i].command, "disable")) {
      send_to_char (ch, "You cannot disable the disable command.\r\n");
      return;
    }

    // Found the effin thing, disable it
    CREATE(p, struct disabled_data, 1);
    p->command = &cmd_info[i];
    p->disabled_by = strdup(GET_NAME(ch)); // Name of disabler
    p->level = GET_LEVEL(ch); 		   // level of disabler
    p->subcmd = cmd_info[i].subcmd; 	   // sub command if any	
    p->next = disabled_first;
    disabled_first = p; 
    send_to_char(ch, "&RCommand '%s' disabled.&n\r\n", p->command->command);
    nmudlog(SYS_LOG, LVL_GOD, TRUE, "%s has disabled the command '%s'", GET_NAME(ch), p->command->command);
//    mudlog(BRF, LVL_IMMORT, TRUE, "(GC) %s has disabled the command '%s'.",
//      GET_NAME(ch), p->command->command);
    save_disabled();
  }
}


// Check if a command is disabled

int check_disabled(const struct command_info *command)
{
  DISABLED_DATA *p;

  for (p = disabled_first; p ; p = p->next)
    if (p->command->command_pointer == command->command_pointer)
      if (p->command->subcmd == command->subcmd)
        return TRUE;

  return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
  FILE *fp;
  DISABLED_DATA *p;
  int i;
  char line[READ_SIZE], name[MAX_INPUT_LENGTH], temp[MAX_INPUT_LENGTH];

  if (disabled_first)
    free_disabled();

  if ((fp = fopen(DISABLED_FILE, "r")) == NULL)
    return; /* No disabled file.. no disabled commands. */

  while (get_line(fp, line)) { 
    if (!str_cmp(line, END_MARKER))
      break; /* break loop if we encounter the END_MARKER */
    CREATE(p, struct disabled_data, 1);
    sscanf(line, "%s %d %ld %s", name, &(p->subcmd), &(p->level), temp);
    /* Find the command in the table */
    for (i = 0; *cmd_info[i].command != '\n'; i++)
      if (!str_cmp(cmd_info[i].command, name))
        break;
    if (*cmd_info[i].command == '\n') { /* command does not exist? */
      log("WARNING: load_disabled(): Skipping unknown disabled command - '%s'!", name);
      free(p);
    } else { /* add new disabled command */
      p->disabled_by = strdup(temp);
      p->command = &cmd_info[i];
      p->next = disabled_first;
      disabled_first = p;
    }
  }
  fclose(fp);
}

/* Save disabled commands */
void save_disabled()
{
  FILE *fp;
  DISABLED_DATA *p;

  if (!disabled_first) {
    /* delete file if no commands are disabled */
    unlink(DISABLED_FILE);
    return;
   }

  if ((fp = fopen(DISABLED_FILE, "w")) == NULL) {
    log("SYSERR: Could not open " DISABLED_FILE " for writing");
    return;
  }

  for (p = disabled_first; p ; p = p->next)
    fprintf (fp, "%s %d %ld %s\n", p->command->command, p->subcmd, p->level, p->disabled_by);
  fprintf(fp, "%s\n", END_MARKER);
  fclose(fp);
}
  
/* free all disabled commands from memory */
void free_disabled()
{
  DISABLED_DATA *p;

  while (disabled_first) {
    p = disabled_first;
    disabled_first = disabled_first->next;
    free(p->disabled_by);
    free(p);
  }
}


void generate_password(char *pwd)
{
  int i, numwords;
  char numgen[4];
  char *wordlist[]={"kuvia",
                    "kuvpwd",
                    "kuvnewpw",
                    "\n" };

  /* Count words in the word list */
  for(numwords=0; wordlist[numwords][0] != '\n'; numwords++);

  /* Generate the random number */
  for (i=0; i<3; i++) {
    numgen[i] = '0' + rand_number(0,9);
  }
  numgen[3] = '\0';

  /* Choose a random word*/
  i = rand_number(1, numwords);

  /* Put the word and number together */
  snprintf(pwd, MAX_PWD_LENGTH, "%s%s", wordlist[(i-1)], numgen);
}


void recover_password(struct descriptor_data *d)
{
  char mail_text[MAX_STRING_LENGTH], mail_subject[MAX_INPUT_LENGTH];
  char mail_passwd[MAX_PWD_LENGTH];
  int len=0, nlen;

  if (!d || !d->character) return;

  if (!GET_EMAIL(d->character)) {
    write_to_output(d, "Sorry, your username does not have a registered email address.\r\n");
    write_to_output(d, "Please create a new account in order to speak to an Immortal about this.\r\n");
    return;
  }

  generate_password(mail_passwd);

  nlen = snprintf(mail_text+len, (MAX_STRING_LENGTH-len)-1,
                  "The Realms of Kuvia Automatic Password Recovery\n"
                  "-----------------------------------------------\n\n"
                  "Your username on The Realms of Kuvia has requested a password change.\n"
		  "You are receiving this e-mail because this is the validated e-mail\n"
		  "addressed associated with the account.  Your new password has been\n"
                  "randomly generated, but you should login to your account as soon as\n"
                  "possible to change your password to your own choice.\n\n\n"

		  "If you did not request this password change, please notity an Immortal\n"
                  "IMMEDIATELY.  Also, please use the password below to login and change\n"
                  "your password as soon as possible.\n\n\n"

                  "- The Realms of Kuvia Admin Staff\n\n");
  len += nlen;
  nlen = snprintf(mail_text+len, (MAX_STRING_LENGTH-len)-1,
                  "Username: %s\n"
                  "Password: %s\n\n"
                  "------------------------------------------------------------------------------------\n"
                  "This email was automatically generated by The Realms of Kuvia.  Please do not reply.\n"
                  "------------------------------------------------------------------------------------\n",
                  GET_NAME(d->character), mail_passwd);
  len += nlen;

  if (!send_email(GET_EMAIL(d->character), mail_subject, mail_text))
  {
    write_to_output(d, "Sorry, Password recovery failure. Please contact an Immortal.\r\n");
  }
  else
  {
/*   FIXME   
   strncpy(GET_PASSWD(d->character), CRYPT(mail_passwd, GET_NAME(d->character)), MAX_PWD_LENGTH);
    (GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0'; */
    save_char(d->character);

    write_to_output(d, "Password recovery complete.\r\n");
    write_to_output(d, "A new password has been sent to your registered email address.\r\n");
    write_to_output(d, "- The Realms of Kuvia Admin\r\n");
  }
}


bool send_email(char *recipient, char *subj, char *bodytext)
{
  FILE *outfile;
  char cmd[MAX_INPUT_LENGTH];

  if (!recipient || !*recipient) return FALSE;
  if (!bodytext || !*bodytext) return FALSE;

  /* Write the mail to a file (pwd_mail.txt) */
  if (!(outfile = fopen("pwd_mail.txt", "w"))) {
    log("Error opening email file.");
    return FALSE;
  }
  if (fputs(bodytext, outfile) < 0) {
    log("Error writing email file.");
    return FALSE;
  }
  if (fclose(outfile)) {
    log("Error closing email file.");
    return FALSE;
  }

  /* NOTE: elm option is provided if 'email' doesn't work. uncomment as necessary */

  sprintf(cmd, "email -s \"%s\" %s < pwd_mail.txt", (subj && *subj) ? subj : "No Subject", recipient);
//  sprintf(cmd, "elm -s %s %s < pwd_mail.txt", (subj && *subj) ? subj : "No Subject", recipient);
  system(cmd);

  /* Remove the tempfile */
  sprintf(cmd, "rm pwd_mail.txt");
  system(cmd);

  return TRUE;
}

