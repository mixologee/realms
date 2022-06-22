//Realms of Kuvia Codebase
//New System Logging Utilities


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

ACMD(do_setlog);


void nmudlog(int type, int level, byte file, const char *targ, ...);
void SetLogs(struct char_data *ch, int flag, int type);

void nmudlog(int type, int level, byte file, const char *targ, ...)
{
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *d;
  char *timestamp, buf1[MSL], buf2[MSL], log_msg[MSL];
  time_t ct;
  va_list args;
  int flag;

  ct = time(0); //timestamp
  timestamp = asctime(localtime(&ct));

  if (level < 0)
    return;


  switch(type) {
    case INFO_LOG:  // 0
      strcpy(buf2, "&G(&YInfo&G)");
      flag = LOG_INFO;
      break;
    case MISC_LOG:  // 1
      strcpy(buf2, "&G(&YMisc&G)");
      flag = LOG_MISC;
      break;
    case SYS_LOG:   // 2
      strcpy(buf2, "&B(&YSys Info&B)");
      flag = LOG_SYS;
      break;
    case OLC_LOG:   // 3
      strcpy(buf2, "&B(&MOLC Info&B)");
      flag = LOG_OLC;
      break;
    case GOD_LOG:   // 4
      strcpy(buf2, "&R(&YGod Info&R)");
      flag = LOG_GOD;
      break;
    case DEATH_LOG: // 5
      strcpy(buf2, "&R(&rDeath&R)");
      flag = LOG_DEATH;
      break;
    case ZONE_LOG:  // 6
      strcpy(buf2, "&B(&MZone Info&B)");
      flag = LOG_ZONE;
      break;
    case CONN_LOG:  // 7
      strcpy(buf2, "&C(&cConnection&C)");
      flag = LOG_CONN;
      break;
    case SUGG_LOG:  // 8
      strcpy(buf2, "&Y(&ySuggestion&Y)");
      flag = LOG_SUGG;
      break;
    case BUG_LOG:   // 9
      strcpy(buf2, "&R(&MBUG&R)");
      flag = LOG_BUG;
      break;
    case IDEA_LOG:  // 10
      strcpy(buf2, "&Y(&yIdea&Y)");
      flag = LOG_IDEA;
      break;
    case TYPO_LOG:  // 11
      strcpy(buf2, "&G(&gTypo&G)");
      flag = LOG_TYPO;
      break;
    case GIVE_LOG:  // 12
      strcpy(buf2, "&R(&YGIVE OBJECT&R)");
      flag = LOG_GIVE;
      break;
    case TAKE_LOG:  // 13
      strcpy(buf2, "&R(&YSNATCH OBJECT&R)");
      flag = LOG_TAKE;
      break;
    default: 
      strcpy(buf2, "&R(&rU&RN&rK&RN&rO&RW&rN&R_&rR&RE&rP&RO&rR&RT&r_&RT&rH&RI&rS&R)");
      flag = LOG_MISC;
      break;
    }

  va_start(args, targ);
  vsprintf(log_msg, targ, args);
  va_end(args);

  if (file)
    fprintf(stderr, "%-19.19s :: %s #[%s]\n", timestamp, buf2, log_msg);

  sprintf(buf1, "%s &W:: &C%s&n\r\n", buf2, log_msg);


  for (d = descriptor_list; d; d = d->next) {
    if ((!d->connected) && 
        (GET_LEVEL(d->character) >= level) &&
        (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING)) && 
        (LOG_FLAGGED(d->character, flag))) {

       send_to_char(d->character, buf1);
    }
  }

}


ACMD(do_setlog)
{
  int l, i, j, t;
  char arg[MSL], buf[MSL];

  one_argument(argument, arg);
  *buf = '\0';

  struct log_struct {
   const char *field;
   int flag;
   int level;
  }

 fields[] = {
     {"nothing", -1},
     {"info"   , LOG_INFO ,   LVL_GOD},
     {"misc"   , LOG_MISC ,   LVL_GOD},
     {"sys"    , LOG_SYS  ,   LVL_GOD},
     {"olc"    , LOG_OLC  ,   LVL_GOD},
     {"god"    , LOG_GOD  ,   LVL_GOD},
     {"death"  , LOG_DEATH,   LVL_GRGOD},
     {"zone"   , LOG_ZONE ,   LVL_GOD},
     {"conn"   , LOG_CONN ,   LVL_GOD},
     {"sugg"   , LOG_SUGG ,   LVL_GOD},
     {"bug"    , LOG_BUG  ,   LVL_IMPL},
     {"idea"   , LOG_IDEA ,   LVL_GOD},
     {"typo"   , LOG_TYPO ,   LVL_GOD},
     {"give"   , LOG_GIVE ,   LVL_IMPL},
     {"take"   , LOG_TAKE ,   LVL_IMPL},
     {"\n"     , -1},
  };


  if (!*arg) {
    send_to_char(ch, "Syntax: setlog <field>\r\n");
    send_to_char(ch, "\r\nAvailable types are:\r\n");
    for (t = GET_LEVEL(ch); t >= LVL_GOD; t--)
     for (j = 0, i = 1; i <= 14; i++)
       if ((GET_LEVEL(ch) >= fields[i].level) && (fields[i].level == t))
         sprintf(buf, "%s&%c%-15s%s&n", buf, 
              (LOG_FLAGGED(ch, fields[i].flag)) ? 'G' : 'R', 
              fields[i].field, (!(++j % 5) ? "\r\n" : ""));
   sprintf(buf, "%s&B%-15s%-15s&n\r\n", buf, "ALL", "\r\nOFF");
   send_to_char(ch, buf);
   return;
  }

  for (l = 0;*(fields[l].field) != '\n'; l++)
    if (!strncmp(arg, fields[l].field, strlen(arg)))
      break;

  if (is_abbrev(arg, "all")) {
    for (j = 0; j < MAX_LOG_FLAG; j++)
      SET_BIT(GET_LOGS(ch), j);
    send_to_char(ch, "All of your log flags have been turned ON\r\n");
    return;
  } else if (is_abbrev(arg, "off")) {
    for (j = 0; j < MAX_LOG_FLAG; j++)
      REMOVE_BIT(GET_LOGS(ch), j);
    send_to_char(ch, "All of your log flags have been turned OFF\r\n");
    return;
  } else
    SetLogs(ch, fields[l].flag, l);
}



void SetLogs(struct char_data *ch, int flag, int type)
{
  char buf[MSL];
  extern char *set_types[];

  if (!LOG_FLAGGED(ch, flag)) {
    SET_BIT(GET_LOGS(ch), flag);
    sprintf(buf, "Log flag for %s turned ON\r\n", set_types[type]);
  } else if (LOG_FLAGGED(ch, flag)) {
    REMOVE_BIT(GET_LOGS(ch), flag);
    sprintf(buf, "Log flag for %s turned OFF\r\n", set_types[type]);
  }
  send_to_char(ch, buf);
  return;
}



void LogChannel(const char *chan, const char *txt, ...)
{
  FILE *fp;
  char *timestamp;
  va_list args;
  time_t Time;
  char error[MSL], buf[MIL];
  char path[MSL];

  va_start(args, txt);
  vsprintf(buf, txt, args);
  va_end(args);

  Time = time(0);
  timestamp = asctime(localtime(&Time));
  *(timestamp + strlen(timestamp) - 6) = '\0';
 
  sprintf(path, "../log/ChannelLog/%s", chan);

  if (!(fp = fopen(path, "a"))) {
    snprintf(error, sizeof(error), "Error with opening ChannelLogs file");
    perror(error);
    exit(1);
  } else {
  fprintf(fp, "[%s] %s\n", timestamp, buf);
  }
  fclose(fp);
} 
