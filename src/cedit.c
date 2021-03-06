/**************************************************************************
*  File: cedit.c                                           Part of tbaMUD *
*  Usage: A graphical in-game game configuration utility for OasisOLC.    *
*                                                                         *
*  Copyright 2002-2003 Kip Potter                                         *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "utils.h"
#include "db.h"
#include "constants.h"
#include "genolc.h"
#include "oasis.h"
#include "improved-edit.h"

/* Local Macros */
#define NO 0
#define YES 1

#define CHECK_VAR(var)  ((var == YES) ? "Yes" : "No")
#define TOGGLE_VAR(var)	if (var == YES) { var = NO; } else { var = YES; }

/* local functions */
void cedit_disp_menu(struct descriptor_data *d);
void cedit_save_internally(struct descriptor_data *d);
void cedit_disp_game_play_options(struct descriptor_data *d);
void cedit_disp_crash_save_options(struct descriptor_data *d);
void cedit_disp_room_numbers(struct descriptor_data *d);
void cedit_disp_operation_options(struct descriptor_data *d);
void cedit_disp_autowiz_options(struct descriptor_data *d);
int  save_config( IDXTYPE nowhere );
void reassign_rooms(void);
void cedit_setup(struct descriptor_data *d);
void cedit_save_to_disk( void );


ACMD(do_oasis_cedit)
{
  struct descriptor_data *d;
  char buf1[MAX_STRING_LENGTH];

  /* No building as a mob or while being forced. */
  if (IS_NPC(ch) || !ch->desc || STATE(ch->desc) != CON_PLAYING)
    return;

  /* Parse any arguments. */
  one_argument(argument, buf1);

  if (GET_LEVEL(ch) < LVL_IMPL) {
    send_to_char(ch, "You can't modify the game configuration.\r\n");
    return;
  }

  d = ch->desc;

  if (!*buf1) {
    CREATE(d->olc, struct oasis_olc_data, 1);
    OLC_ZONE(d) = 0;
    cedit_setup(d);
    STATE(d) = CON_CEDIT;
    act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
    SET_BIT_AR(PLR_FLAGS(ch), PLR_WRITING);

    mudlog(BRF, LVL_IMMORT, TRUE,
      "OLC: %s starts editing the game configuration.", GET_NAME(ch));
    return;
  } else if (str_cmp("save", buf1) != 0) {
    send_to_char(ch, "Yikes!  Stop that, someone will get hurt!\r\n");
    return;
  }

  send_to_char(ch, "Saving the game configuration.\r\n");
  mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE,
    "OLC: %s saves the game configuration.", GET_NAME(ch));

  cedit_save_to_disk();
}

void cedit_setup(struct descriptor_data *d)
{
  /* Create the config_data struct. */
  CREATE(OLC_CONFIG(d), struct config_data, 1);

  /* Copy the current configuration from the config_info to this one and copy
   * the game play options from the configuration info struct. */
  OLC_CONFIG(d)->play.pk_allowed          = CONFIG_PK_ALLOWED;
  OLC_CONFIG(d)->play.pt_allowed          = CONFIG_PT_ALLOWED;
  OLC_CONFIG(d)->play.level_can_shout     = CONFIG_LEVEL_CAN_SHOUT;
  OLC_CONFIG(d)->play.holler_move_cost    = CONFIG_HOLLER_MOVE_COST;
  OLC_CONFIG(d)->play.tunnel_size         = CONFIG_TUNNEL_SIZE;
  OLC_CONFIG(d)->play.max_exp_gain        = CONFIG_MAX_EXP_GAIN;
  OLC_CONFIG(d)->play.max_exp_loss        = CONFIG_MAX_EXP_LOSS;
  OLC_CONFIG(d)->play.max_npc_corpse_time = CONFIG_MAX_NPC_CORPSE_TIME;
  OLC_CONFIG(d)->play.max_pc_corpse_time  = CONFIG_MAX_PC_CORPSE_TIME;
  OLC_CONFIG(d)->play.idle_void           = CONFIG_IDLE_VOID;
  OLC_CONFIG(d)->play.idle_rent_time      = CONFIG_IDLE_RENT_TIME;
  OLC_CONFIG(d)->play.idle_max_level      = CONFIG_IDLE_MAX_LEVEL;
  OLC_CONFIG(d)->play.dts_are_dumps       = CONFIG_DTS_ARE_DUMPS;
  OLC_CONFIG(d)->play.load_into_inventory = CONFIG_LOAD_INVENTORY;
  OLC_CONFIG(d)->play.track_through_doors = CONFIG_TRACK_T_DOORS;
  OLC_CONFIG(d)->play.no_mort_to_immort   = CONFIG_NO_MORT_TO_IMMORT;
  OLC_CONFIG(d)->play.disp_closed_doors   = CONFIG_DISP_CLOSED_DOORS;

  /* Crash Saves */
  OLC_CONFIG(d)->csd.free_rent            = CONFIG_FREE_RENT;
  OLC_CONFIG(d)->csd.max_obj_save         = CONFIG_MAX_OBJ_SAVE;
  OLC_CONFIG(d)->csd.min_rent_cost        = CONFIG_MIN_RENT_COST;
  OLC_CONFIG(d)->csd.auto_save            = CONFIG_AUTO_SAVE;
  OLC_CONFIG(d)->csd.autosave_time        = CONFIG_AUTOSAVE_TIME;
  OLC_CONFIG(d)->csd.crash_file_timeout   = CONFIG_CRASH_TIMEOUT;
  OLC_CONFIG(d)->csd.rent_file_timeout    = CONFIG_RENT_TIMEOUT;

  /* Room Numbers */
  OLC_CONFIG(d)->room_nums.mortal_start_room = CONFIG_MORTAL_START;
  OLC_CONFIG(d)->room_nums.immort_start_room = CONFIG_IMMORTAL_START;
  OLC_CONFIG(d)->room_nums.frozen_start_room = CONFIG_FROZEN_START;
  OLC_CONFIG(d)->room_nums.donation_room_1   = CONFIG_DON_ROOM_1;
  OLC_CONFIG(d)->room_nums.donation_room_2   = CONFIG_DON_ROOM_2;
  OLC_CONFIG(d)->room_nums.donation_room_3   = CONFIG_DON_ROOM_3;

  /* Game Operation */
  OLC_CONFIG(d)->operation.DFLT_PORT          = CONFIG_DFLT_PORT;
  OLC_CONFIG(d)->operation.max_playing        = CONFIG_MAX_PLAYING;
  OLC_CONFIG(d)->operation.max_filesize       = CONFIG_MAX_FILESIZE;
  OLC_CONFIG(d)->operation.max_bad_pws        = CONFIG_MAX_BAD_PWS;
  OLC_CONFIG(d)->operation.siteok_everyone    = CONFIG_SITEOK_ALL;
  OLC_CONFIG(d)->operation.use_new_socials    = CONFIG_NEW_SOCIALS;
  OLC_CONFIG(d)->operation.auto_save_olc      = CONFIG_OLC_SAVE;
  OLC_CONFIG(d)->operation.nameserver_is_slow = CONFIG_NS_IS_SLOW;

  /* Autowiz */
  OLC_CONFIG(d)->autowiz.use_autowiz          = CONFIG_USE_AUTOWIZ;
  OLC_CONFIG(d)->autowiz.min_wizlist_lev      = CONFIG_MIN_WIZLIST_LEV;


  /* Mudmail */
  OLC_CONFIG(d)->mail.mail_allowed      = CONFIG_CAN_MAIL;
  OLC_CONFIG(d)->mail.objects_allowed   = CONFIG_CAN_MAIL_OBJ;
  OLC_CONFIG(d)->mail.gold_allowed      = CONFIG_CAN_MAIL_GOLD;
  OLC_CONFIG(d)->mail.stamp_cost        = CONFIG_STAMP_COST;
  OLC_CONFIG(d)->mail.min_level         = CONFIG_MIN_MAIL_LEVEL;
  OLC_CONFIG(d)->mail.min_free_level    = CONFIG_FREE_MAIL_LEVEL;
  OLC_CONFIG(d)->mail.allow_drafts      = CONFIG_DRAFTS_ALLOWED;
  OLC_CONFIG(d)->mail.draft_timeout     = CONFIG_DRAFT_TIMEOUT;
  OLC_CONFIG(d)->mail.safe_mailing      = CONFIG_SAFE_MAILING;
  OLC_CONFIG(d)->mail.min_mail_anywhere = CONFIG_MIN_MAIL_ANYWHERE;
  OLC_CONFIG(d)->mail.min_send_to_all   = CONFIG_MIN_SEND_TO_ALL;

  /* Allocate space for the strings. */
  OLC_CONFIG(d)->play.OK       = str_udup(CONFIG_OK);
  OLC_CONFIG(d)->play.NOPERSON = str_udup(CONFIG_NOPERSON);
  OLC_CONFIG(d)->play.NOEFFECT = str_udup(CONFIG_NOEFFECT);

  if (CONFIG_DFLT_IP)
    OLC_CONFIG(d)->operation.DFLT_IP     = strdup(CONFIG_DFLT_IP);
  else
    OLC_CONFIG(d)->operation.DFLT_IP     = NULL;

  if (CONFIG_DFLT_DIR)
    OLC_CONFIG(d)->operation.DFLT_DIR    = strdup(CONFIG_DFLT_DIR);
  else
    OLC_CONFIG(d)->operation.DFLT_DIR    = NULL;

  if (CONFIG_LOGNAME)
    OLC_CONFIG(d)->operation.LOGNAME     = strdup(CONFIG_LOGNAME);
  else
    OLC_CONFIG(d)->operation.LOGNAME     = NULL;

  if (CONFIG_MENU)
    OLC_CONFIG(d)->operation.MENU        = strdup(CONFIG_MENU);
  else
    OLC_CONFIG(d)->operation.MENU        = NULL;

  if (CONFIG_WELC_MESSG)
    OLC_CONFIG(d)->operation.WELC_MESSG  = strdup(CONFIG_WELC_MESSG);
  else
    OLC_CONFIG(d)->operation.WELC_MESSG  = NULL;

  if (CONFIG_START_MESSG)
    OLC_CONFIG(d)->operation.START_MESSG = strdup(CONFIG_START_MESSG);
  else
    OLC_CONFIG(d)->operation.START_MESSG = NULL;

  cedit_disp_menu(d);
}

void cedit_save_internally(struct descriptor_data *d)
{
  /* see if we need to reassign spec procs on rooms */
  int reassign = (CONFIG_DTS_ARE_DUMPS != OLC_CONFIG(d)->play.dts_are_dumps);
  /* Copy the data back from the descriptor to the config_info structure. */
  CONFIG_PK_ALLOWED          = OLC_CONFIG(d)->play.pk_allowed;
  CONFIG_PT_ALLOWED          = OLC_CONFIG(d)->play.pt_allowed;
  CONFIG_LEVEL_CAN_SHOUT     = OLC_CONFIG(d)->play.level_can_shout;
  CONFIG_HOLLER_MOVE_COST    = OLC_CONFIG(d)->play.holler_move_cost;
  CONFIG_TUNNEL_SIZE         = OLC_CONFIG(d)->play.tunnel_size;
  CONFIG_MAX_EXP_GAIN        = OLC_CONFIG(d)->play.max_exp_gain;
  CONFIG_MAX_EXP_LOSS        = OLC_CONFIG(d)->play.max_exp_loss;
  CONFIG_MAX_NPC_CORPSE_TIME = OLC_CONFIG(d)->play.max_npc_corpse_time;
  CONFIG_MAX_PC_CORPSE_TIME  = OLC_CONFIG(d)->play.max_pc_corpse_time;
  CONFIG_IDLE_VOID           = OLC_CONFIG(d)->play.idle_void;
  CONFIG_IDLE_RENT_TIME      = OLC_CONFIG(d)->play.idle_rent_time;
  CONFIG_IDLE_MAX_LEVEL      = OLC_CONFIG(d)->play.idle_max_level;
  CONFIG_DTS_ARE_DUMPS       = OLC_CONFIG(d)->play.dts_are_dumps;
  CONFIG_LOAD_INVENTORY = OLC_CONFIG(d)->play.load_into_inventory;
  CONFIG_TRACK_T_DOORS = OLC_CONFIG(d)->play.track_through_doors;
  CONFIG_NO_MORT_TO_IMMORT   = OLC_CONFIG(d)->play.no_mort_to_immort;
  CONFIG_DISP_CLOSED_DOORS   = OLC_CONFIG(d)->play.disp_closed_doors;


  /* Crash Saves */
  CONFIG_FREE_RENT            = OLC_CONFIG(d)->csd.free_rent;
  CONFIG_MAX_OBJ_SAVE         = OLC_CONFIG(d)->csd.max_obj_save;
  CONFIG_MIN_RENT_COST        = OLC_CONFIG(d)->csd.min_rent_cost;
  CONFIG_AUTO_SAVE            = OLC_CONFIG(d)->csd.auto_save;
  CONFIG_AUTOSAVE_TIME        = OLC_CONFIG(d)->csd.autosave_time;
  CONFIG_CRASH_TIMEOUT   = OLC_CONFIG(d)->csd.crash_file_timeout;
  CONFIG_RENT_TIMEOUT    = OLC_CONFIG(d)->csd.rent_file_timeout;

  /* Room Numbers */
  CONFIG_MORTAL_START = OLC_CONFIG(d)->room_nums.mortal_start_room;
  CONFIG_IMMORTAL_START = OLC_CONFIG(d)->room_nums.immort_start_room;
  CONFIG_FROZEN_START = OLC_CONFIG(d)->room_nums.frozen_start_room;
  CONFIG_DON_ROOM_1   = OLC_CONFIG(d)->room_nums.donation_room_1;
  CONFIG_DON_ROOM_2   = OLC_CONFIG(d)->room_nums.donation_room_2;
  CONFIG_DON_ROOM_3   = OLC_CONFIG(d)->room_nums.donation_room_3;

  /* Game Operation */
  CONFIG_DFLT_PORT          = OLC_CONFIG(d)->operation.DFLT_PORT;
  CONFIG_MAX_PLAYING        = OLC_CONFIG(d)->operation.max_playing;
  CONFIG_MAX_FILESIZE       = OLC_CONFIG(d)->operation.max_filesize;
  CONFIG_MAX_BAD_PWS        = OLC_CONFIG(d)->operation.max_bad_pws;
  CONFIG_SITEOK_ALL    = OLC_CONFIG(d)->operation.siteok_everyone;
  CONFIG_NEW_SOCIALS        = OLC_CONFIG(d)->operation.use_new_socials;
  CONFIG_NS_IS_SLOW = OLC_CONFIG(d)->operation.nameserver_is_slow;
  CONFIG_OLC_SAVE           = OLC_CONFIG(d)->operation.auto_save_olc;

  /* Autowiz */
  CONFIG_USE_AUTOWIZ          = OLC_CONFIG(d)->autowiz.use_autowiz;
  CONFIG_MIN_WIZLIST_LEV      = OLC_CONFIG(d)->autowiz.min_wizlist_lev;

  /* Mudmail */
  CONFIG_CAN_MAIL          = OLC_CONFIG(d)->mail.mail_allowed;
  CONFIG_CAN_MAIL_OBJ      = OLC_CONFIG(d)->mail.objects_allowed;
  CONFIG_CAN_MAIL_GOLD     = OLC_CONFIG(d)->mail.gold_allowed;
  CONFIG_STAMP_COST        = OLC_CONFIG(d)->mail.stamp_cost;
  CONFIG_OBJECT_COST       = OLC_CONFIG(d)->mail.object_cost;
  CONFIG_MIN_MAIL_LEVEL    = OLC_CONFIG(d)->mail.min_level;
  CONFIG_FREE_MAIL_LEVEL   = OLC_CONFIG(d)->mail.min_free_level;
  CONFIG_DRAFTS_ALLOWED    = OLC_CONFIG(d)->mail.allow_drafts;
  CONFIG_DRAFT_TIMEOUT     = OLC_CONFIG(d)->mail.draft_timeout;
  CONFIG_SAFE_MAILING      = OLC_CONFIG(d)->mail.safe_mailing;
  CONFIG_MIN_MAIL_ANYWHERE = OLC_CONFIG(d)->mail.min_mail_anywhere;
  CONFIG_MIN_SEND_TO_ALL   = OLC_CONFIG(d)->mail.min_send_to_all;


  /* Allocate space for the strings. */
  if (CONFIG_OK)
    free(CONFIG_OK);
  CONFIG_OK       = str_udup(OLC_CONFIG(d)->play.OK);

  if (CONFIG_NOPERSON)
    free(CONFIG_NOPERSON);
  CONFIG_NOPERSON = str_udup(OLC_CONFIG(d)->play.NOPERSON);

  if (CONFIG_NOEFFECT)
    free(CONFIG_NOEFFECT);
  CONFIG_NOEFFECT = str_udup(OLC_CONFIG(d)->play.NOEFFECT);

  if (CONFIG_DFLT_IP)
    free(CONFIG_DFLT_IP);
  if (OLC_CONFIG(d)->operation.DFLT_IP)
    CONFIG_DFLT_IP     = strdup(OLC_CONFIG(d)->operation.DFLT_IP);
  else
    CONFIG_DFLT_IP     = NULL;

  if (CONFIG_DFLT_DIR)
    free(CONFIG_DFLT_DIR);
  if (OLC_CONFIG(d)->operation.DFLT_DIR)
    CONFIG_DFLT_DIR    = strdup(OLC_CONFIG(d)->operation.DFLT_DIR);
  else
    CONFIG_DFLT_DIR    = NULL;

  if (CONFIG_LOGNAME)
    free(CONFIG_LOGNAME);
  if (OLC_CONFIG(d)->operation.LOGNAME)
    CONFIG_LOGNAME     = strdup(OLC_CONFIG(d)->operation.LOGNAME);
  else
    CONFIG_LOGNAME     = NULL;

  if (CONFIG_MENU)
    free(CONFIG_MENU);
  if (OLC_CONFIG(d)->operation.MENU)
    CONFIG_MENU        = strdup(OLC_CONFIG(d)->operation.MENU);
  else
    CONFIG_MENU        = NULL;

  if (CONFIG_WELC_MESSG)
    free(CONFIG_WELC_MESSG);
  if (OLC_CONFIG(d)->operation.WELC_MESSG)
    CONFIG_WELC_MESSG  = strdup(OLC_CONFIG(d)->operation.WELC_MESSG);
  else
    CONFIG_WELC_MESSG  = NULL;

  if (CONFIG_START_MESSG)
    free(CONFIG_START_MESSG);
  if (OLC_CONFIG(d)->operation.START_MESSG)
    CONFIG_START_MESSG = strdup(OLC_CONFIG(d)->operation.START_MESSG);
  else
    CONFIG_START_MESSG = NULL;

  /* if we changed the dts to/from dumps, reassign - Welcor */
  if (reassign)
    reassign_rooms();

  add_to_save_list(NOWHERE, SL_CFG);
}

void cedit_save_to_disk( void )
{
  /* Just call save_config and get it over with. */
  save_config( NOWHERE );
}

int save_config( IDXTYPE nowhere )
{
  FILE *fl;
  char buf[MAX_STRING_LENGTH];

  if (!(fl = fopen(CONFIG_CONFFILE, "w"))) {
    perror("SYSERR: save_config");
    return (FALSE);
  }

  fprintf(fl,
    "* This file is autogenerated by OasisOLC (CEdit).\n"
    "* Please note the following information about this file's format.\n"
    "*\n"
    "* - If variable is a yes/no or true/false based variable, use 1's and 0's\n"
    "*   where YES or TRUE = 1 and NO or FALSE = 0.\n"
    "* - Variable names in this file are case-insensitive.  Variable values\n"
    "*   are not case-insensitive.\n"
    "* -----------------------------------------------------------------------\n"
    "* Lines starting with * are comments, and are not parsed.\n"
    "* -----------------------------------------------------------------------\n\n"
    "* [ Game Play Options ]\n"
  );

  fprintf(fl, "* Is player killing allowed on the mud?\n"
              "pk_allowed = %d\n\n", CONFIG_PK_ALLOWED);
  fprintf(fl, "* Is player thieving allowed on the mud?\n"
  	      "pt_allowed = %d\n\n", CONFIG_PT_ALLOWED);
  fprintf(fl, "* What is the minimum level a player can shout/gossip/etc?\n"
              "level_can_shout = %d\n\n", CONFIG_LEVEL_CAN_SHOUT);
  fprintf(fl, "* How many movement points does shouting cost the player?\n"
  	      "holler_move_cost = %d\n\n", CONFIG_HOLLER_MOVE_COST);
  fprintf(fl, "* How many players can fit in a tunnel?\n"
              "tunnel_size = %d\n\n", CONFIG_TUNNEL_SIZE);
  fprintf(fl, "* Maximum experience gainable per kill?\n"
              "max_exp_gain = %d\n\n", CONFIG_MAX_EXP_GAIN);
  fprintf(fl, "* Maximum experience loseable per death?\n"
              "max_exp_loss = %d\n\n", CONFIG_MAX_EXP_LOSS);
  fprintf(fl, "* Number of tics before NPC corpses decompose.\n"
              "max_npc_corpse_time = %d\n\n", CONFIG_MAX_NPC_CORPSE_TIME);
  fprintf(fl, "* Number of tics before PC corpses decompose.\n"
              "max_pc_corpse_time = %d\n\n", CONFIG_MAX_PC_CORPSE_TIME);
  fprintf(fl, "* Number of tics before a PC is sent to the void.\n"
              "idle_void = %d\n\n", CONFIG_IDLE_VOID);
  fprintf(fl, "* Number of tics before a PC is autorented.\n"
              "idle_rent_time = %d\n\n", CONFIG_IDLE_RENT_TIME);
  fprintf(fl, "* Level and above of players whom are immune to idle penalties.\n"
              "idle_max_level = %d\n\n", CONFIG_IDLE_MAX_LEVEL);
  fprintf(fl, "* Should the items in death traps be junked automatically?\n"
              "dts_are_dumps = %d\n\n", CONFIG_DTS_ARE_DUMPS);
  fprintf(fl, "* When an immortal loads an object, should it load into their inventory?\n"
              "load_into_inventory = %d\n\n", CONFIG_LOAD_INVENTORY);
  fprintf(fl, "* Should PC's be able to track through hidden or closed doors?\n"
              "track_through_doors = %d\n\n", CONFIG_TRACK_T_DOORS);
  fprintf(fl, "* Should players who reach enough exp automatically level to immortal?\n"
              "no_mort_to_immort = %d\n\n", CONFIG_NO_MORT_TO_IMMORT);
  fprintf(fl, "* Should closed doors be shown on autoexit / exit?\n"
              "disp_closed_doors = %d\n\n", CONFIG_DISP_CLOSED_DOORS);

  strcpy(buf, CONFIG_OK);
  strip_cr(buf);

  fprintf(fl, "* Text sent to players when OK is all that is needed.\n"
              "ok = %s\n\n", buf);

  strcpy(buf, CONFIG_NOPERSON);
  strip_cr(buf);

  fprintf(fl, "* Text sent to players when noone is available.\n"
              "noperson = %s\n\n", buf);

  strcpy(buf, CONFIG_NOEFFECT);
  strip_cr(buf);

  fprintf(fl, "* Text sent to players when an effect fails.\n"
              "noeffect = %s\n", buf);

   /* RENT / CRASHSAVE OPTIONS */
  fprintf(fl, "\n\n\n* [ Rent/Crashsave Options ]\n");

  fprintf(fl, "* Should the MUD allow you to 'rent' for free?  (i.e. if you just quit,\n"
              "* your objects are saved at no cost, as in Merc-type MUDs.)\n"
              "free_rent = %d\n\n", CONFIG_FREE_RENT);

  fprintf(fl, "* Maximum number of items players are allowed to rent.\n"
   	      "max_obj_save = %d\n\n", CONFIG_MAX_OBJ_SAVE);

  fprintf(fl, "* Should the game automatically save people?\n"
              "auto_save = %d\n\n", CONFIG_AUTO_SAVE);

  fprintf(fl, "* If auto_save = 1, how often (in minutes) should the game save people's objects?\n"
              "autosave_time = %d\n\n", CONFIG_AUTOSAVE_TIME);

  fprintf(fl, "* Lifetime of crashfiles and force-rent (idlesave) files in days.\n"
              "crash_file_timeout = %d\n\n", CONFIG_CRASH_TIMEOUT);

  fprintf(fl, "* Lifetime of normal rent files in days.\n"
              "rent_file_timeout = %d\n\n", CONFIG_RENT_TIMEOUT);


   /* ROOM NUMBERS */
  fprintf(fl, "\n\n\n* [ Room Numbers ]\n");

  fprintf(fl, "* The virtual number of the room that mortals should enter at.\n"
  	      "mortal_start_room = %d\n\n", CONFIG_MORTAL_START);

  fprintf(fl, "* The virtual number of the room that immorts should enter at.\n"
              "immort_start_room = %d\n\n", CONFIG_IMMORTAL_START);

  fprintf(fl, "* The virtual number of the room that frozen people should enter at.\n"
	      "frozen_start_room = %d\n\n", CONFIG_FROZEN_START);

  fprintf(fl, "* The virtual numbers of the donation rooms.  Note: Add donation rooms\n"
              "* sequentially (1 & 2 before 3). If you don't, you might not be able to\n"
              "* donate. Use -1 for 'no such room'.\n"
              "donation_room_1 = %d\n"
              "donation_room_2 = %d\n"
              "donation_room_3 = %d\n\n",
              CONFIG_DON_ROOM_1 != NOWHERE ? CONFIG_DON_ROOM_1 : -1,
              CONFIG_DON_ROOM_2 != NOWHERE ? CONFIG_DON_ROOM_2 : -1,
              CONFIG_DON_ROOM_3 != NOWHERE ? CONFIG_DON_ROOM_3 : -1);


  fprintf(fl, "\n\n\n* [ Game Operation Options ]\n");

  fprintf(fl, "* This is the default port on which the game should run if no port is\n"
              "* given on the command-line.  NOTE WELL: If you're using the\n"
              "* 'autorun' script, the port number there will override this setting.\n"
              "* Change the PORT= line in autorun instead of (or in addition to)\n"
              "* changing this.\n"
              "DFLT_PORT = %ld\n\n",
              CONFIG_DFLT_PORT);

  if (CONFIG_DFLT_IP) {
    strcpy(buf, CONFIG_DFLT_IP);
    strip_cr(buf);

    fprintf(fl, "* IP address to which the MUD should bind.\nDFLT_IP = %s\n\n", buf);
  }

  if (CONFIG_DFLT_DIR) {
    strcpy(buf, CONFIG_DFLT_DIR);
    strip_cr(buf);

    fprintf(fl, "* default directory to use as data directory.\n"
                "DFLT_DIR = %s\n\n", buf);
  }

  if (CONFIG_LOGNAME) {
    strcpy(buf, CONFIG_LOGNAME);
    strip_cr(buf);

    fprintf(fl, "* What file to log messages to (ex: 'log/syslog').\n"
                "LOGNAME = %s\n\n", buf);
  }

  fprintf(fl, "* Maximum number of players allowed before game starts to turn people away.\n"
              "max_playing = %d\n\n",
              CONFIG_MAX_PLAYING);

  fprintf(fl, "* Maximum size of bug, typo, and idea files in bytes (to prevent bombing).\n"
              "max_filesize = %d\n\n",
              CONFIG_MAX_FILESIZE);

  fprintf(fl, "* Maximum number of password attempts before disconnection.\n"
              "max_bad_pws = %d\n\n",
              CONFIG_MAX_BAD_PWS);

  fprintf(fl, "* Is the site ok for everyone except those that are banned?\n"
              "siteok_everyone = %d\n\n",
              CONFIG_SITEOK_ALL);

  fprintf(fl, "* If you want to use the original social file format\n"
              "* and disable Aedit, set to 0, otherwise, 1.\n"
              "use_new_socials = %d\n\n",
              CONFIG_NEW_SOCIALS);

  fprintf(fl, "* If the nameserver is fast, set to 0, otherwise, 1.\n"
              "nameserver_is_slow = %d\n\n",
              CONFIG_NS_IS_SLOW);

  fprintf(fl, "* Should OLC autosave to disk (1) or save internally (0).\n"
              "auto_save_olc = %d\n\n",
              CONFIG_OLC_SAVE);

  if (CONFIG_MENU) {
    strcpy(buf, CONFIG_MENU);
    strip_cr(buf);

    fprintf(fl, "* The entrance/exit menu.\n"
                "MENU = \n%s~\n\n", buf);
  }

  if (CONFIG_WELC_MESSG) {
    strcpy(buf, CONFIG_WELC_MESSG);
    strip_cr(buf);

    fprintf(fl, "* The welcome message.\nWELC_MESSG = \n%s~\n\n", buf);
  }

  if (CONFIG_START_MESSG) {
    strcpy(buf, CONFIG_START_MESSG);
    strip_cr(buf);

    fprintf(fl, "* NEWBIE start message.\n"
                "START_MESSG = \n%s~\n\n", buf);
  }

  fprintf(fl, "\n\n\n* [ Autowiz Options ]\n");

  fprintf(fl, "* Should the game automatically create a new wizlist/immlist every time\n"
              "* someone immorts, or is promoted to a higher (or lower) god level?\n"
              "use_autowiz = %d\n\n",
              CONFIG_USE_AUTOWIZ);

  fprintf(fl, "* If yes, what is the lowest level which should be on the wizlist?\n"
              "min_wizlist_lev = %d\n\n",
              CONFIG_MIN_WIZLIST_LEV);


  fprintf(fl, "\n\n\n* [ Mudmail Options ]\n");

  fprintf(fl, "* Enable/Disable the whole mudmail system\n"
              "can_mail = %d\n\n",
              CONFIG_CAN_MAIL);

  fprintf(fl, "* Can objects be sent via mudmail?\n"
              "mail_objects_allowed = %d\n\n",
              CONFIG_CAN_MAIL_OBJ);

  fprintf(fl, "* Can gold coins be sent via mudmail?\n"
              "mail_gold_allowed = %d\n\n",
              CONFIG_CAN_MAIL_GOLD);

  fprintf(fl, "* How much does it cost to send mudmails\n"
              "mail_stamp_cost = %d\n\n",
              CONFIG_STAMP_COST);

  fprintf(fl, "* How much extra does it cost to add each object mudmails\n"
              "mail_object_cost = %d\n\n",
              CONFIG_OBJECT_COST);

  fprintf(fl, "* The minimum player level that's allowed to send mudmails\n"
              "mail_min_level = %d\n\n",
              CONFIG_MIN_MAIL_LEVEL);

  fprintf(fl, "* The minimum level that can send mudmails for free\n"
              "mail_min_free_level = %d\n\n",
              CONFIG_FREE_MAIL_LEVEL);

  fprintf(fl, "* Are players allowed to create draft mails for sending later?\n"
              "mail_drafts_allowed = %d\n\n",
              CONFIG_DRAFTS_ALLOWED);

  fprintf(fl, "* Number of days before a draft mail is autoremoved (0 for never) \n"
              "mail_draft_timeout = %d\n\n",
              CONFIG_DRAFT_TIMEOUT);

  fprintf(fl, "* When on, the player cannot be attacked or stolen from while mailing. \n"
              "mail_safe_mailing = %d\n\n",
              CONFIG_SAFE_MAILING);

  fprintf(fl, "* This is the minimum level that can send mail from any location (not just at postmaster). \n"
              "mail_min_mail_anywhere = %d\n\n",
              CONFIG_MIN_MAIL_ANYWHERE);

  fprintf(fl, "* This is the minimum level that can send mail to all players. \n"
              "mail_min_send_to_all = %d\n\n",
              CONFIG_MIN_SEND_TO_ALL);

  fclose(fl);

  if (in_save_list(NOWHERE, SL_CFG))
    remove_from_save_list(NOWHERE, SL_CFG);

  return (TRUE);
}

/* Menu functions - The main menu. */
void cedit_disp_menu(struct descriptor_data *d)
{
  get_char_colors(d->character);
  clear_screen(d);

  /* Menu header. */
  write_to_output(d,
  	  "OasisOLC MUD Configuration Editor\r\n"
  	  "%sG%s) Game Play Options\r\n"
  	  "%sC%s) Crashsave/Rent Options\r\n"
  	  "%sR%s) Room Numbers\r\n"
          "%sO%s) Operation Options\r\n"
          "%sA%s) Autowiz Options\r\n"
          "%sM%s) Mudmail Options\r\n"
          "%sQ%s) Quit\r\n"
          "Enter your choice : ",

          grn, nrm,
          grn, nrm,
          grn, nrm,
          grn, nrm,
          grn, nrm,
          grn, nrm,
          grn, nrm
          );

  OLC_MODE(d) = CEDIT_MAIN_MENU;
}

void cedit_disp_game_play_options(struct descriptor_data *d)
{
  get_char_colors(d->character);
  clear_screen(d);

  write_to_output(d, "\r\n\r\n"
        "%sA%s) Player Killing Allowed  : %s%s\r\n"
        "%sB%s) Player Thieving Allowed : %s%s\r\n"
        "%sC%s) Minimum Level To Shout  : %s%d\r\n"
        "%sD%s) Holler Move Cost        : %s%d\r\n"
        "%sE%s) Tunnel Size             : %s%d\r\n"
        "%sF%s) Maximum Experience Gain : %s%d\r\n"
        "%sG%s) Maximum Experience Loss : %s%d\r\n"
        "%sH%s) Max Time for NPC Corpse : %s%d\r\n"
        "%sI%s) Max Time for PC Corpse  : %s%d\r\n"
        "%sJ%s) Tics before PC sent to void : %s%d\r\n"
        "%sK%s) Tics before PC is autosaved : %s%d\r\n"
        "%sL%s) Level Immune To IDLE        : %s%d\r\n"
        "%sM%s) Death Traps Junk Items      : %s%s\r\n"
        "%sN%s) Objects Load Into Inventory : %s%s\r\n"
        "%sO%s) Track Through Doors         : %s%s\r\n"
        "%sP%s) Display Closed Doors        : %s%s\r\n"
        "%sR%s) Mortals Level To Immortal   : %s%s\r\n"
        "%sX%s) Experience Multiplier   : %s%s\r\n"
	"%s1%s) OK Message Text         : %s%s\r\n"
        "%s2%s) NOPERSON Message Text   : %s%s\r\n"
        "%s3%s) NOEFFECT Message Text   : %s%s\r\n"
        "%sQ%s) Exit To The Main Menu\r\n"
        "Enter your choice : ",
        grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->play.pk_allowed),
        grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->play.pt_allowed),
        grn, nrm, cyn, OLC_CONFIG(d)->play.level_can_shout,
        grn, nrm, cyn, OLC_CONFIG(d)->play.holler_move_cost,
        grn, nrm, cyn, OLC_CONFIG(d)->play.tunnel_size,
        grn, nrm, cyn, OLC_CONFIG(d)->play.max_exp_gain,
        grn, nrm, cyn, OLC_CONFIG(d)->play.max_exp_loss,
        grn, nrm, cyn, OLC_CONFIG(d)->play.max_npc_corpse_time,
        grn, nrm, cyn, OLC_CONFIG(d)->play.max_pc_corpse_time,

        grn, nrm, cyn, OLC_CONFIG(d)->play.idle_void,
        grn, nrm, cyn, OLC_CONFIG(d)->play.idle_rent_time,
        grn, nrm, cyn, OLC_CONFIG(d)->play.idle_max_level,
        grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->play.dts_are_dumps),
        grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->play.load_into_inventory),
        grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->play.track_through_doors),
        grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->play.disp_closed_doors),
	grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->play.no_mort_to_immort),

        grn, nrm, cyn, OLC_CONFIG(d)->play.OK,
        grn, nrm, cyn, OLC_CONFIG(d)->play.NOPERSON,
        grn, nrm, cyn, OLC_CONFIG(d)->play.NOEFFECT,

        grn, nrm
        );

  OLC_MODE(d) = CEDIT_GAME_OPTIONS_MENU;
}

void cedit_disp_crash_save_options(struct descriptor_data *d)
{
  get_char_colors(d->character);
  clear_screen(d);

  write_to_output(d, "\r\n\r\n"
  	"%sA%s) Free Rent          : %s%s\r\n"
  	"%sB%s) Max Objects Saved  : %s%d\r\n"
  	"%sC%s) Minimum Rent Cost  : %s%d\r\n"
  	"%sD%s) Auto Save          : %s%s\r\n"
  	"%sE%s) Auto Save Time     : %s%d minute(s)\r\n"
  	"%sF%s) Crash File Timeout : %s%d day(s)\r\n"
  	"%sG%s) Rent File Timeout  : %s%d day(s)\r\n"
  	"%sQ%s) Exit To The Main Menu\r\n"
  	"Enter your choice : ",
  	grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->csd.free_rent),
  	grn, nrm, cyn, OLC_CONFIG(d)->csd.max_obj_save,
  	grn, nrm, cyn, OLC_CONFIG(d)->csd.min_rent_cost,
  	grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->csd.auto_save),
  	grn, nrm, cyn, OLC_CONFIG(d)->csd.autosave_time,
  	grn, nrm, cyn, OLC_CONFIG(d)->csd.crash_file_timeout,
  	grn, nrm, cyn, OLC_CONFIG(d)->csd.rent_file_timeout,
  	grn, nrm
  	);

  OLC_MODE(d) = CEDIT_CRASHSAVE_OPTIONS_MENU;
}

void cedit_disp_room_numbers(struct descriptor_data *d)
{
  get_char_colors(d->character);
  clear_screen(d);

  write_to_output(d, "\r\n\r\n"
  	"%sA%s) Mortal Start Room   : %s%d\r\n"
  	"%sB%s) Immortal Start Room : %s%d\r\n"
  	"%sC%s) Frozen Start Room   : %s%d\r\n"
  	"%s1%s) Donation Room #1    : %s%d\r\n"
  	"%s2%s) Donation Room #2    : %s%d\r\n"
  	"%s3%s) Donation Room #3    : %s%d\r\n"
  	"%sQ%s) Exit To The Main Menu\r\n"
  	"Enter your choice : ",
  	grn, nrm, cyn, OLC_CONFIG(d)->room_nums.mortal_start_room,
  	grn, nrm, cyn, OLC_CONFIG(d)->room_nums.immort_start_room,
  	grn, nrm, cyn, OLC_CONFIG(d)->room_nums.frozen_start_room,
  	grn, nrm, cyn, OLC_CONFIG(d)->room_nums.donation_room_1,
  	grn, nrm, cyn, OLC_CONFIG(d)->room_nums.donation_room_2,
  	grn, nrm, cyn, OLC_CONFIG(d)->room_nums.donation_room_3,
  	grn, nrm
  	);

  OLC_MODE(d) = CEDIT_ROOM_NUMBERS_MENU;
}

void cedit_disp_operation_options(struct descriptor_data *d)
{
  get_char_colors(d->character);
  clear_screen(d);

  write_to_output(d, "\r\n\r\n"
  	"%sA%s) Default Port : %s%ld\r\n"
  	"%sB%s) Default IP   : %s%s\r\n"
  	"%sC%s) Default Directory   : %s%s\r\n"
  	"%sD%s) Logfile Name : %s%s\r\n"
  	"%sE%s) Max Players  : %s%d\r\n"
  	"%sF%s) Max Filesize : %s%d\r\n"
  	"%sG%s) Max Bad Pws  : %s%d\r\n"
  	"%sH%s) Site Ok Everyone : %s%s\r\n"
  	"%sI%s) Name Server Is Slow : %s%s\r\n"
        "%sJ%s) Use new socials file: %s%s\r\n"
        "%sK%s) OLC autosave to disk: %s%s\r\n"
  	"%sL%s) Main Menu           : \r\n%s%s\r\n"
  	"%sM%s) Welcome Message     : \r\n%s%s\r\n"
  	"%sN%s) Start Message       : \r\n%s%s\r\n"
    "%sQ%s) Exit To The Main Menu\r\n"
    "Enter your choice : ",
    grn, nrm, cyn, OLC_CONFIG(d)->operation.DFLT_PORT,
    grn, nrm, cyn, OLC_CONFIG(d)->operation.DFLT_IP ? OLC_CONFIG(d)->operation.DFLT_IP : "<None>",
    grn, nrm, cyn, OLC_CONFIG(d)->operation.DFLT_DIR ? OLC_CONFIG(d)->operation.DFLT_DIR : "<None>",
    grn, nrm, cyn, OLC_CONFIG(d)->operation.LOGNAME ? OLC_CONFIG(d)->operation.LOGNAME : "<None>",
    grn, nrm, cyn, OLC_CONFIG(d)->operation.max_playing,
    grn, nrm, cyn, OLC_CONFIG(d)->operation.max_filesize,
    grn, nrm, cyn, OLC_CONFIG(d)->operation.max_bad_pws,
    grn, nrm, cyn, YESNO(OLC_CONFIG(d)->operation.siteok_everyone),
    grn, nrm, cyn, YESNO(OLC_CONFIG(d)->operation.nameserver_is_slow),
    grn, nrm, cyn, YESNO(OLC_CONFIG(d)->operation.use_new_socials),
    grn, nrm, cyn, YESNO(OLC_CONFIG(d)->operation.auto_save_olc),
    grn, nrm, cyn, OLC_CONFIG(d)->operation.MENU ? OLC_CONFIG(d)->operation.MENU : "<None>",
    grn, nrm, cyn, OLC_CONFIG(d)->operation.WELC_MESSG ? OLC_CONFIG(d)->operation.WELC_MESSG : "<None>",
    grn, nrm, cyn, OLC_CONFIG(d)->operation.START_MESSG ? OLC_CONFIG(d)->operation.START_MESSG : "<None>",
    grn, nrm
    );

  OLC_MODE(d) = CEDIT_OPERATION_OPTIONS_MENU;
}

void cedit_disp_autowiz_options(struct descriptor_data *d)
{
  get_char_colors(d->character);
  clear_screen(d);

  write_to_output(d, "\r\n\r\n"
    "%sA%s) Use the autowiz        : %s%s\r\n"
    "%sB%s) Minimum wizlist level  : %s%d\r\n"
    "%sQ%s) Exit To The Main Menu\r\n"
    "Enter your choice : ",
    grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->autowiz.use_autowiz),
    grn, nrm, cyn, OLC_CONFIG(d)->autowiz.min_wizlist_lev,
    grn, nrm
    );

  OLC_MODE(d) = CEDIT_AUTOWIZ_OPTIONS_MENU;
}

void cedit_disp_mudmail_options(struct descriptor_data *d)
{
  char timeout[MAX_INPUT_LENGTH];
  get_char_colors(d->character);
  clear_screen(d);

  if (OLC_CONFIG(d)->mail.draft_timeout > 0)
    sprintf(timeout, "%d days", OLC_CONFIG(d)->mail.draft_timeout);
  else
    sprintf(timeout, "%s<Never!>", cyn);

  write_to_output(d, "\r\n\r\n"
    "%sA%s) Use mudmail system         : %s%s\r\n"
    "%sB%s) Allow object attachments   : %s%s\r\n"
    "%sC%s) Allow gold attachments     : %s%s\r\n"
    "%sD%s) Stamp Cost (Normal Mails)  : %s%d coins\r\n"
    "%sE%s) Cost per object attached   : %s%d coins\r\n"
    "%sF%s) Allow draft mudmails       : %s%s\r\n"
    "%sG%s) Draft mail timeout         : %s%s\r\n"
    "%sH%s) Min. level that can mail   : %s%d\r\n"
    "%sI%s) Min. level for free mail   : %s%d\r\n"
    "%sJ%s) Safe while mailing?        : %s%s\r\n"
    "%sK%s) Min level to mail anywhere : %s%d\r\n"
    "%sL%s) Min level to send to all   : %s%d\r\n"
    "%sQ%s) Exit To The Main Menu\r\n"
    "Enter your choice : ",
    grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->mail.mail_allowed),
    grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->mail.objects_allowed),
    grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->mail.gold_allowed),
    grn, nrm, yel, OLC_CONFIG(d)->mail.stamp_cost,
    grn, nrm, yel, OLC_CONFIG(d)->mail.object_cost,
    grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->mail.allow_drafts),
    grn, nrm, yel, timeout,
    grn, nrm, yel, OLC_CONFIG(d)->mail.min_level,
    grn, nrm, yel, OLC_CONFIG(d)->mail.min_free_level,
    grn, nrm, cyn, CHECK_VAR(OLC_CONFIG(d)->mail.safe_mailing),
    grn, nrm, yel, OLC_CONFIG(d)->mail.min_mail_anywhere,
    grn, nrm, yel, OLC_CONFIG(d)->mail.min_send_to_all,
    grn, nrm
    );

  OLC_MODE(d) = CEDIT_MUDMAIL_OPTIONS_MENU;
}



/* The event handler. */
void cedit_parse(struct descriptor_data *d, char *arg)
{
  char *oldtext = NULL;

  switch (OLC_MODE(d)) {
    case CEDIT_CONFIRM_SAVESTRING:
      switch (*arg) {
        case 'y':
        case 'Y':
          cedit_save_internally(d);
          mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE,
                 "OLC: %s modifies the game configuration.", GET_NAME(d->character));
          cleanup_olc(d, CLEANUP_CONFIG);
	  if (CONFIG_AUTO_SAVE) {
	    cedit_save_to_disk();
	    write_to_output(d, "Game configuration saved to disk.\r\n");
	  } else
            write_to_output(d, "Game configuration saved to memory.\r\n");
          return;
        case 'n':
        case 'N':
          write_to_output(d, "Game configuration not saved to memory.\r\n");
          cleanup_olc(d, CLEANUP_CONFIG);
          return;
        default :
          write_to_output(d, "\r\nThat is an invalid choice!\r\n");
          write_to_output(d, "Do you wish to save your changes? : ");
          return;
      }

    case CEDIT_MAIN_MENU:
      switch (*arg) {
        case 'g':
        case 'G':
          cedit_disp_game_play_options(d);
          OLC_MODE(d) = CEDIT_GAME_OPTIONS_MENU;
          break;

        case 'c':
        case 'C':
          cedit_disp_crash_save_options(d);
          OLC_MODE(d) = CEDIT_CRASHSAVE_OPTIONS_MENU;
          break;

        case 'r':
        case 'R':
          cedit_disp_room_numbers(d);
          OLC_MODE(d) = CEDIT_ROOM_NUMBERS_MENU;
          break;

        case 'o':
        case 'O':
          cedit_disp_operation_options(d);
          OLC_MODE(d) = CEDIT_OPERATION_OPTIONS_MENU;
          break;

        case 'a':
        case 'A':
          cedit_disp_autowiz_options(d);
          OLC_MODE(d) = CEDIT_AUTOWIZ_OPTIONS_MENU;
          break;

        case 'm':
        case 'M':
          cedit_disp_mudmail_options(d);
          OLC_MODE(d) = CEDIT_MUDMAIL_OPTIONS_MENU;
          break;

        case 'q':
        case 'Q':
          write_to_output(d, "Do you wish to save your changes? : ");
          OLC_MODE(d) = CEDIT_CONFIRM_SAVESTRING;
          break;

        default:
          write_to_output(d, "That is an invalid choice!\r\n");
          cedit_disp_menu(d);
          break;
      }
      break;

    case CEDIT_GAME_OPTIONS_MENU:
      switch (*arg) {
        case 'a':
        case 'A':
          TOGGLE_VAR(OLC_CONFIG(d)->play.pk_allowed);
          break;

        case 'b':
        case 'B':
          TOGGLE_VAR(OLC_CONFIG(d)->play.pt_allowed);
          break;

        case 'c':
        case 'C':
          write_to_output(d, "Enter the minimum level a player must be to shout, gossip, etc : ");
          OLC_MODE(d) = CEDIT_LEVEL_CAN_SHOUT;
          return;

        case 'd':
        case 'D':
          write_to_output(d, "Enter the amount it costs (in move points) to holler : ");
          OLC_MODE(d) = CEDIT_HOLLER_MOVE_COST;
          return;

        case 'e':
        case 'E':
          write_to_output(d, "Enter the maximum number of people allowed in a tunnel : ");
          OLC_MODE(d) = CEDIT_TUNNEL_SIZE;
          return;

        case 'f':
        case 'F':
          write_to_output(d, "Enter the maximum gain of experience per kill for players : ");
          OLC_MODE(d) = CEDIT_MAX_EXP_GAIN;
          return;

        case 'g':
        case 'G':
          write_to_output(d, "Enter the maximum loss of experience per death for players : ");
          OLC_MODE(d) = CEDIT_MAX_EXP_LOSS;
          return;

        case 'h':
        case 'H':
          write_to_output(d, "Enter the number of tics before NPC corpses decompose : ");
          OLC_MODE(d) = CEDIT_MAX_NPC_CORPSE_TIME;
          return;

        case 'i':
        case 'I':
          write_to_output(d, "Enter the number of tics before PC corpses decompose : ");
          OLC_MODE(d) = CEDIT_MAX_PC_CORPSE_TIME;
          return;

        case 'j':
        case 'J':
          write_to_output(d, "Enter the number of tics before PC's are sent to the void (idle) : ");
          OLC_MODE(d) = CEDIT_IDLE_VOID;
          return;

        case 'k':
        case 'K':
          write_to_output(d, "Enter the number of tics before PC's are automatically rented and forced to quit : ");
          OLC_MODE(d) = CEDIT_IDLE_RENT_TIME;
          return;

        case 'l':
        case 'L':
          write_to_output(d, "Enter the level a player must be to become immune to IDLE : ");
          OLC_MODE(d) = CEDIT_IDLE_MAX_LEVEL;
          return;

        case 'm':
        case 'M':
          TOGGLE_VAR(OLC_CONFIG(d)->play.dts_are_dumps);
          break;

        case 'n':
        case 'N':
          TOGGLE_VAR(OLC_CONFIG(d)->play.load_into_inventory);
          break;

        case 'o':
        case 'O':
          TOGGLE_VAR(OLC_CONFIG(d)->play.track_through_doors);
          break;

        case 'p':
        case 'P':
          TOGGLE_VAR(OLC_CONFIG(d)->play.disp_closed_doors);
          break;

        case 'r':
        case 'R':
	  TOGGLE_VAR(OLC_CONFIG(d)->play.no_mort_to_immort);
          break;
   

        case '1':
          write_to_output(d, "Enter the OK message : ");
          OLC_MODE(d) = CEDIT_OK;
          return;

        case '2':
          write_to_output(d, "Enter the NOPERSON message : ");
          OLC_MODE(d) = CEDIT_NOPERSON;
          return;

        case '3':
          write_to_output(d, "Enter the NOEFFECT message : ");
          OLC_MODE(d) = CEDIT_NOEFFECT;
          return;
    
        case 'q':
        case 'Q':
          cedit_disp_menu(d);
          return;

        default:
          write_to_output(d, "\r\nThat is an invalid choice!\r\n");
          cedit_disp_game_play_options(d);
      }

      cedit_disp_game_play_options(d);
      return;

    case CEDIT_CRASHSAVE_OPTIONS_MENU:
      switch (*arg) {
        case 'a':
        case 'A':
          TOGGLE_VAR(OLC_CONFIG(d)->csd.free_rent);
          break;

        case 'b':
        case 'B':
          write_to_output(d, "Enter the maximum number of items players can rent : ");
          OLC_MODE(d) = CEDIT_MAX_OBJ_SAVE;
          return;

        case 'c':
        case 'C':
          write_to_output(d, "Enter the surcharge on top of item costs : ");
          OLC_MODE(d) = CEDIT_MIN_RENT_COST;
          return;

        case 'd':
        case 'D':
          TOGGLE_VAR(OLC_CONFIG(d)->csd.auto_save);
          break;

        case 'e':
        case 'E':
          write_to_output(d, "Enter how often (in minutes) should the MUD save players : ");
          OLC_MODE(d) = CEDIT_AUTOSAVE_TIME;
          return;

        case 'f':
        case 'F':
          write_to_output(d, "Enter the lifetime of crash and idlesave files (days) : ");
          OLC_MODE(d) = CEDIT_CRASH_FILE_TIMEOUT;
          return;

        case 'g':
        case 'G':
          write_to_output(d, "Enter the lifetime of normal rent files (days) : ");
          OLC_MODE(d) = CEDIT_RENT_FILE_TIMEOUT;
          return;

        case 'q':
        case 'Q':
          cedit_disp_menu(d);
          return;

        default:
          write_to_output(d, "\r\nThat is an invalid choice!\r\n");
        }

        cedit_disp_crash_save_options(d);
        return;

    case CEDIT_ROOM_NUMBERS_MENU:
      switch (*arg) {
        case 'a':
        case 'A':
          write_to_output(d, "Enter the room's vnum where mortals should load into : ");
          OLC_MODE(d) = CEDIT_MORTAL_START_ROOM;
          return;

        case 'b':
        case 'B':
          write_to_output(d, "Enter the room's vnum where immortals should load into : ");
          OLC_MODE(d) = CEDIT_IMMORT_START_ROOM;
          return;

        case 'c':
        case 'C':
        write_to_output(d, "Enter the room's vnum where frozen people should load into : ");
        OLC_MODE(d) = CEDIT_FROZEN_START_ROOM;
        return;

      case '1':
        write_to_output(d, "Enter the vnum for donation room #1 : ");
        OLC_MODE(d) = CEDIT_DONATION_ROOM_1;
        return;

      case '2':
        write_to_output(d, "Enter the vnum for donation room #2 : ");
        OLC_MODE(d) = CEDIT_DONATION_ROOM_2;
        return;

      case '3':
        write_to_output(d, "Enter the vnum for donation room #3 : ");
        OLC_MODE(d) = CEDIT_DONATION_ROOM_3;
        return;

      case 'q':
      case 'Q':
        cedit_disp_menu(d);
        return;

      default:
        write_to_output(d, "\r\nThat is an invalid choice!\r\n");
    }

    cedit_disp_room_numbers(d);
    return;

     case CEDIT_OPERATION_OPTIONS_MENU:
       switch (*arg) {
         case 'a':
         case 'A':
           write_to_output(d, "Enter the default port number : ");
           OLC_MODE(d) = CEDIT_DFLT_PORT;
           return;

         case 'b':
         case 'B':
           write_to_output(d, "Enter the default IP Address : ");
           OLC_MODE(d) = CEDIT_DFLT_IP;
           return;

         case 'c':
         case 'C':
           write_to_output(d, "Enter the default directory : ");
           OLC_MODE(d) = CEDIT_DFLT_DIR;
           return;

         case 'd':
         case 'D':
           write_to_output(d, "Enter the name of the logfile : ");
           OLC_MODE(d) = CEDIT_LOGNAME;
           return;

         case 'e':
         case 'E':
           write_to_output(d, "Enter the maximum number of players : ");
           OLC_MODE(d) = CEDIT_MAX_PLAYING;
           return;

         case 'f':
         case 'F':
           write_to_output(d, "Enter the maximum size of the logs : ");
           OLC_MODE(d) = CEDIT_MAX_FILESIZE;
           return;

         case 'g':
         case 'G':
           write_to_output(d, "Enter the maximum number of password attempts : ");
           OLC_MODE(d) = CEDIT_MAX_BAD_PWS;
           return;

         case 'h':
         case 'H':
           TOGGLE_VAR(OLC_CONFIG(d)->operation.siteok_everyone);
           break;

         case 'i':
         case 'I':
           TOGGLE_VAR(OLC_CONFIG(d)->operation.nameserver_is_slow);
           break;

         case 'j':
         case 'J':
           TOGGLE_VAR(OLC_CONFIG(d)->operation.use_new_socials);
           send_to_char(d->character,
              "Please note that using the stock social file will disable AEDIT.\r\n");
           break;

         case 'k':
         case 'K':
           TOGGLE_VAR(OLC_CONFIG(d)->operation.auto_save_olc);
           break;

         case 'l':
         case 'L':
           OLC_MODE(d) = CEDIT_MENU;
           clear_screen(d);
           send_editor_help(d);
           write_to_output(d, "Enter the new MENU :\r\n\r\n");

           if (OLC_CONFIG(d)->operation.MENU) {
             write_to_output(d, "%s", OLC_CONFIG(d)->operation.MENU);
             oldtext = strdup(OLC_CONFIG(d)->operation.MENU);
           }

           string_write(d, &OLC_CONFIG(d)->operation.MENU, MAX_INPUT_LENGTH, 0, oldtext);
           return;

         case 'm':
         case 'M':
           OLC_MODE(d) = CEDIT_WELC_MESSG;
           clear_screen(d);
           send_editor_help(d);
           write_to_output(d, "Enter the new welcome message :\r\n\r\n");

           if (OLC_CONFIG(d)->operation.WELC_MESSG) {
             write_to_output(d, "%s", OLC_CONFIG(d)->operation.WELC_MESSG);
             oldtext = str_udup(OLC_CONFIG(d)->operation.WELC_MESSG);
           }

           string_write(d, &OLC_CONFIG(d)->operation.WELC_MESSG, MAX_INPUT_LENGTH, 0, oldtext);
           return;

         case 'n':
         case 'N':
           OLC_MODE(d) = CEDIT_START_MESSG;
           clear_screen(d);
           send_editor_help(d);
           write_to_output(d, "Enter the new newbie start message :\r\n\r\n");

           if (OLC_CONFIG(d)->operation.START_MESSG) {
             write_to_output(d, "%s", OLC_CONFIG(d)->operation.START_MESSG);
             oldtext = strdup(OLC_CONFIG(d)->operation.START_MESSG);
           }

           string_write(d, &OLC_CONFIG(d)->operation.START_MESSG, MAX_INPUT_LENGTH, 0, oldtext);
           return;

         case 'q':
         case 'Q':
           cedit_disp_menu(d);
           return;

         default:
           write_to_output(d, "\r\nThat is an invalid choice!\r\n");
      }

   cedit_disp_operation_options(d);
   return;

    case CEDIT_AUTOWIZ_OPTIONS_MENU:
      switch (*arg) {
        case 'a':
        case 'A':
          TOGGLE_VAR(OLC_CONFIG(d)->autowiz.use_autowiz);
          break;

        case 'b':
        case 'B':
          write_to_output(d, "Enter the minimum level for players to appear on the wizlist : ");
          OLC_MODE(d) = CEDIT_MIN_WIZLIST_LEV;
          return;

        case 'q':
        case 'Q':
          cedit_disp_menu(d);
          return;

        default:
          write_to_output(d, "\r\nThat is an invalid choice!\r\n");
      }

      cedit_disp_autowiz_options(d);
      return;

    case CEDIT_MUDMAIL_OPTIONS_MENU:
      switch (*arg) {
        case 'a':
        case 'A':
          TOGGLE_VAR(OLC_CONFIG(d)->mail.mail_allowed);
          break;

        case 'b':
        case 'B':
          TOGGLE_VAR(OLC_CONFIG(d)->mail.objects_allowed);
          break;

        case 'c':
        case 'C':
          TOGGLE_VAR(OLC_CONFIG(d)->mail.gold_allowed);
          break;

        case 'd':
        case 'D':
          write_to_output(d, "Enter the cost per mail (without attachments) : ");
          OLC_MODE(d) = CEDIT_STAMP_COST;
          return;

        case 'e':
        case 'E':
          write_to_output(d, "Enter the cost per object attachment : ");
          OLC_MODE(d) = CEDIT_OBJECT_COST;
          return;

        case 'f':
        case 'F':
          TOGGLE_VAR(OLC_CONFIG(d)->mail.allow_drafts);
          break;

        case 'g':
        case 'G':
          write_to_output(d, "Enter the timeout (in days) before draft mails are auto-removed (0=off) : ");
          OLC_MODE(d) = CEDIT_DRAFT_TIMEOUT;
          return;

        case 'h':
        case 'H':
          write_to_output(d, "Enter the minimum level that can send mudmails : ");
          OLC_MODE(d) = CEDIT_MIN_MAIL_LEVEL;
          return;

        case 'i':
        case 'I':
          write_to_output(d, "Enter the minimum level for free mudmails : ");
          OLC_MODE(d) = CEDIT_MIN_FREE_LEVEL;
          return;

        case 'j':
        case 'J':
          TOGGLE_VAR(OLC_CONFIG(d)->mail.safe_mailing);
          break;

        case 'k':
        case 'K':
          write_to_output(d, "Enter the minimum level that can send mail from anywhere : ");
          OLC_MODE(d) = CEDIT_MAIL_ANYWHERE;
          return;

        case 'l':
        case 'L':
          write_to_output(d, "Enter the minimum level that can send to all players : ");
          OLC_MODE(d) = CEDIT_MAIL_ALL_PLAYERS;
          return;

        case 'q':
        case 'Q':
          cedit_disp_menu(d);
          return;

        default:
          write_to_output(d, "\r\nThat is an invalid choice!\r\n");
      }

      cedit_disp_mudmail_options(d);
      return;

    case CEDIT_LEVEL_CAN_SHOUT:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the minimum level a player must be to shout, gossip, etc : ");
      } else {
        OLC_CONFIG(d)->play.level_can_shout = atoi(arg);
        cedit_disp_game_play_options(d);
      }
      break;

    case CEDIT_HOLLER_MOVE_COST:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the amount it costs (in move points) to holler : ");
      } else {
        OLC_CONFIG(d)->play.holler_move_cost = atoi(arg);
        cedit_disp_game_play_options(d);
      }
      break;

    case CEDIT_TUNNEL_SIZE:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the maximum number of people allowed in a tunnel : ");
      } else {
        OLC_CONFIG(d)->play.tunnel_size = atoi(arg);
        cedit_disp_game_play_options(d);
      }
      break;

    case CEDIT_MAX_EXP_GAIN:
      if (*arg)
        OLC_CONFIG(d)->play.max_exp_gain = atoi(arg);

      cedit_disp_game_play_options(d);
      break;

    case CEDIT_MAX_EXP_LOSS:
      if (*arg)
        OLC_CONFIG(d)->play.max_exp_loss = atoi(arg);

      cedit_disp_game_play_options(d);
      break;

    case CEDIT_MAX_NPC_CORPSE_TIME:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the number of tics before NPC corpses decompose : ");
      } else {
        OLC_CONFIG(d)->play.max_npc_corpse_time = atoi(arg);
        cedit_disp_game_play_options(d);
      }
      break;

    case CEDIT_MAX_PC_CORPSE_TIME:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the number of tics before PC corpses decompose : ");
        } else {
          OLC_CONFIG(d)->play.max_pc_corpse_time = atoi(arg);
          cedit_disp_game_play_options(d);
        }
        break;

    case CEDIT_IDLE_VOID:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the number of tics before PC's are sent to the void (idle) : ");
      } else {
        OLC_CONFIG(d)->play.idle_void = atoi(arg);
        cedit_disp_game_play_options(d);
      }
      break;

    case CEDIT_IDLE_RENT_TIME:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the number of tics before PC's are automatically rented and forced to quit : ");
      } else {
        OLC_CONFIG(d)->play.idle_rent_time = atoi(arg);
        cedit_disp_game_play_options(d);
      }
      break;

    case CEDIT_IDLE_MAX_LEVEL:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the level a player must be to become immune to IDLE : ");
      } else {
        OLC_CONFIG(d)->play.idle_max_level = atoi(arg);
        cedit_disp_game_play_options(d);
      }
      break;

    case CEDIT_OK:
      if (!genolc_checkstring(d, arg))
        break;

      if (OLC_CONFIG(d)->play.OK)
        free(OLC_CONFIG(d)->play.OK);

      OLC_CONFIG(d)->play.OK = str_udup(arg);
      strcat(OLC_CONFIG(d)->play.OK, "\r\n");

      cedit_disp_game_play_options(d);
      break;

    case CEDIT_NOPERSON:
      if (!genolc_checkstring(d, arg))
        break;

      if (OLC_CONFIG(d)->play.NOPERSON)
        free(OLC_CONFIG(d)->play.NOPERSON);

      OLC_CONFIG(d)->play.NOPERSON = str_udup(arg);
      strcat(OLC_CONFIG(d)->play.NOPERSON, "\r\n");

      cedit_disp_game_play_options(d);
      break;

    case CEDIT_NOEFFECT:
      if (!genolc_checkstring(d, arg))
        break;

      if (OLC_CONFIG(d)->play.NOEFFECT)
        free(OLC_CONFIG(d)->play.NOEFFECT);

      OLC_CONFIG(d)->play.NOEFFECT = str_udup(arg);
      strcat(OLC_CONFIG(d)->play.NOEFFECT, "\r\n");

      cedit_disp_game_play_options(d);
      break;

    case CEDIT_MAX_OBJ_SAVE:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the maximum objects a player can save : ");
        } else {
          OLC_CONFIG(d)->csd.max_obj_save = atoi(arg);
          cedit_disp_crash_save_options(d);
        }
        break;

    case CEDIT_MIN_RENT_COST:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the minimum amount it costs to rent : ");
      } else {
        OLC_CONFIG(d)->csd.min_rent_cost = atoi(arg);
        cedit_disp_crash_save_options(d);
      }
      break;

    case CEDIT_AUTOSAVE_TIME:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the interval for player's being autosaved : ");
      } else {
        OLC_CONFIG(d)->csd.autosave_time = atoi(arg);
        cedit_disp_crash_save_options(d);
      }
      break;

    case CEDIT_CRASH_FILE_TIMEOUT:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the lifetime of crash and idlesave files (days) : ");
      } else {
        OLC_CONFIG(d)->csd.crash_file_timeout = atoi(arg);
        cedit_disp_crash_save_options(d);
      }
      break;

    case CEDIT_RENT_FILE_TIMEOUT:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the lifetime of rent files (days) : ");
      } else {
        OLC_CONFIG(d)->csd.rent_file_timeout = atoi(arg);
        cedit_disp_crash_save_options(d);
      }
      break;

    case CEDIT_MORTAL_START_ROOM:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the room's vnum where mortals should load into : ");
      } else if (real_room(atoi(arg)) == NOWHERE) {
        write_to_output(d,
          "That room doesn't exist!\r\n"
          "Enter the room's vnum where mortals should load into : ");
      } else {
        OLC_CONFIG(d)->room_nums.mortal_start_room = atoi(arg);
        cedit_disp_room_numbers(d);
      }
      break;

    case CEDIT_IMMORT_START_ROOM:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the room's vnum where immortals should load into : ");
      } else if (real_room(atoi(arg)) == NOWHERE) {
        write_to_output(d,
          "That room doesn't exist!\r\n"
          "Enter the room's vnum where immortals should load into : ");
      } else {
        OLC_CONFIG(d)->room_nums.immort_start_room = atoi(arg);
        cedit_disp_room_numbers(d);
      }
      break;

    case CEDIT_FROZEN_START_ROOM:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the room's vnum where frozen people should load into : ");
      } else if (real_room(atoi(arg)) == NOWHERE) {
        write_to_output(d,
          "That room doesn't exist!\r\n"
          "Enter the room's vnum where frozen people should load into : ");
      } else {
        OLC_CONFIG(d)->room_nums.frozen_start_room = atoi(arg);
        cedit_disp_room_numbers(d);
      }
      break;

    case CEDIT_DONATION_ROOM_1:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the vnum for donation room #1 : ");
      } else if (real_room(atoi(arg)) == NOWHERE) {
        write_to_output(d,
          "That room doesn't exist!\r\n"
          "Enter the vnum for donation room #1 : ");
      } else {
        OLC_CONFIG(d)->room_nums.donation_room_1 = atoi(arg);
        cedit_disp_room_numbers(d);
      }
      break;

    case CEDIT_DONATION_ROOM_2:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the vnum for donation room #2 : ");
      } else if (real_room(atoi(arg)) == NOWHERE) {
        write_to_output(d,
          "That room doesn't exist!\r\n"
          "Enter the vnum for donation room #2 : ");
      } else {
        OLC_CONFIG(d)->room_nums.donation_room_2 = atoi(arg);
        cedit_disp_room_numbers(d);
      }
      break;

    case CEDIT_DONATION_ROOM_3:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the vnum for donation room #3 : ");
      } else if (real_room(atoi(arg)) == NOWHERE) {
        write_to_output(d,
          "That room doesn't exist!\r\n"
          "Enter the vnum for donation room #3 : ");
      } else {
        OLC_CONFIG(d)->room_nums.donation_room_3 = atoi(arg);
        cedit_disp_room_numbers(d);
      }
      break;

    case CEDIT_DFLT_PORT:
      OLC_CONFIG(d)->operation.DFLT_PORT = atoi(arg);
      cedit_disp_operation_options(d);
      break;

    case CEDIT_DFLT_IP:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the default ip address : ");
      } else {
        OLC_CONFIG(d)->operation.DFLT_IP = str_udup(arg);
        cedit_disp_operation_options(d);
      }
      break;

    case CEDIT_DFLT_DIR:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the default directory : ");
      } else {
        OLC_CONFIG(d)->operation.DFLT_DIR = str_udup(arg);
        cedit_disp_operation_options(d);
      }
      break;

    case CEDIT_LOGNAME:
      if (!*arg) {
        write_to_output(d,
          "That is an invalid choice!\r\n"
          "Enter the name of the logfile : ");
      } else {
        OLC_CONFIG(d)->operation.LOGNAME = str_udup(arg);
        cedit_disp_operation_options(d);
      }
      break;

    case CEDIT_MAX_PLAYING:
      OLC_CONFIG(d)->operation.max_playing = atoi(arg);
      cedit_disp_operation_options(d);
      break;

    case CEDIT_MAX_FILESIZE:
      OLC_CONFIG(d)->operation.max_filesize = atoi(arg);
      cedit_disp_operation_options(d);
      break;

    case CEDIT_MAX_BAD_PWS:
      OLC_CONFIG(d)->operation.max_bad_pws = atoi(arg);
      cedit_disp_operation_options(d);
      break;

    case CEDIT_MIN_WIZLIST_LEV:
      if (atoi(arg) > LVL_IMPL) {
        write_to_output(d,
          "The minimum wizlist level can't be greater than %d.\r\n"
          "Enter the minimum level for players to appear on the wizlist : ", LVL_IMPL);
      } else {
        OLC_CONFIG(d)->autowiz.min_wizlist_lev = atoi(arg);
        cedit_disp_autowiz_options(d);
      }
      break;

    case CEDIT_STAMP_COST:
      OLC_CONFIG(d)->mail.stamp_cost = MIN(MAX(atoi(arg), 0), 1000);
      cedit_disp_mudmail_options(d);
      break;

    case CEDIT_OBJECT_COST:
      OLC_CONFIG(d)->mail.object_cost = MIN(MAX(atoi(arg), 0), 1000);
      cedit_disp_mudmail_options(d);
      break;

    case CEDIT_DRAFT_TIMEOUT:
      OLC_CONFIG(d)->mail.draft_timeout = MIN(MAX(atoi(arg), 0), 3653); /* up to 10 years */
      cedit_disp_mudmail_options(d);
      break;

    case CEDIT_MIN_MAIL_LEVEL:
      OLC_CONFIG(d)->mail.min_level = MIN(MAX(atoi(arg), 0), LVL_IMPL);
      cedit_disp_mudmail_options(d);
      break;

    case CEDIT_MIN_FREE_LEVEL:
      OLC_CONFIG(d)->mail.min_free_level = MIN(MAX(atoi(arg), 0), LVL_IMPL);
      cedit_disp_mudmail_options(d);
      break;

    case CEDIT_MAIL_ANYWHERE:
      OLC_CONFIG(d)->mail.min_mail_anywhere = MIN(MAX(atoi(arg), 0), LVL_IMPL);
      cedit_disp_mudmail_options(d);
      break;

    case CEDIT_MAIL_ALL_PLAYERS:
      OLC_CONFIG(d)->mail.min_send_to_all = MIN(MAX(atoi(arg), 0), LVL_IMPL);
      cedit_disp_mudmail_options(d);
      break;

    default:  /* We should never get here, but just in case... */
      cleanup_olc(d, CLEANUP_CONFIG);
      mudlog(BRF, LVL_BUILDER, TRUE, "SYSERR: OLC: cedit_parse(): Reached default case!");
      write_to_output(d, "Oops...\r\n");
      break;
  }
}  /* End of parse_cedit() */

void reassign_rooms(void)
{
  void assign_rooms(void);
  int i;

  /* remove old funcs */
  for (i = 0; i < top_of_world; i++)
    world[i].func = NULL;

  /* reassign spec_procs */
  assign_rooms();
}

void cedit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {
  case CEDIT_MENU:
  case CEDIT_WELC_MESSG:
  case CEDIT_START_MESSG:
    cedit_disp_operation_options(d);
    break;
  }
}
