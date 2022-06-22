// New News system, adaptation from Outlands - Slurk

#include "conf.h"
#include "sysdep.h"

#include "nedit.h"

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

#define MAX_NEWS_LENGTH 81982
#define MAX_SUBJECT_LENGTH 100

// Globals
bool editing_news = false;
char news_subject[100];
int news_counter = 0;
int gnews_counter = 0;

char news_body[MAX_NEWS_LENGTH];


void nedit_string_cleanup(struct descriptor_data *d, int term)
{

  FILE *fl;
  FILE *newscenter; //place holder for count of news/gnews
  char *file_path = OLC_PATH(d);
  char *news_type = OLC_STORAGE(d);

  if (!file_path)
    term = STRINGADD_ABORT;

  switch (term) {
     case STRINGADD_SAVE:
       if (!(fl = fopen(file_path, "w")))
         mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE, "&RError: nedit_string_cleanup: unable to write to '%s'", file_path);        
       else {
         if (*d->str) {
           fprintf(fl, "%ld\n", d->mail_to);
           fprintf(fl, "%s\n", GET_NAME(d->character));
           fprintf(fl, "%s\n", news_subject);
           strip_cr(*d->str);
           fputs(*d->str, fl);
         }
  
         fclose(fl);
         write_to_output(d, "Saved!\r\n");
       }

       if (!(newscenter = fopen("../lib/news/newscenter", "w")))
         mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE, "&Error: nedit_string_cleanup: unable to write to newscenter");
       else {
         if (!strcmp(news_type, "news"))
           news_counter++;
       else if (!strcmp(news_type, "godnews"))
           gnews_counter++;

        fprintf(newscenter, "%d %d", news_counter, gnews_counter);
        fclose(newscenter);
       }
       break;
   
     case STRINGADD_ABORT:
       write_to_output(d, "News Editor Aborted\r\n");
       act("$n stops writing.", TRUE, d->character, 0, 0, TO_ROOM);
       break;
       
     default:
       mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE, "&RError: nedit_string_cleanup: Unknown terminator status.");
       break;
   }

   cleanup_olc(d, CLEANUP_ALL);
   STATE(d) = CON_PLAYING;
   editing_news = false;
}

ACMD(do_nedit)
{
  char subject[100], news_file[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  int news_number;

  if (editing_news) {
    send_to_char(ch, "A news file is already being edited, please wait.\r\n");
    return;
  }

  half_chop(argument, arg, buf);

  if (!*argument) {
    send_to_char(ch, "Syntax: nedit <news | gnews> <subject>\r\n");
    return;
  }

  if (ch->desc->olc) {
    mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE, "&RError: %s already has an olc structure", GET_NAME(ch));
    free(ch->desc->olc);
  }

  if (is_abbrev(arg, "news")) {
    sprintf(news_file, "%s/%d.news", NEWS_DIR, news_counter);
    news_number=news_counter;
  } else if (is_abbrev(arg, "gnews")) {
    sprintf(news_file, "%s/%d.gnews", NEWS_DIR, gnews_counter);
    news_number=gnews_counter;
  } else {
    send_to_char(ch, "Syntax: nedit <news | gnews> <subject>\r\n");
    return;
  }

  CREATE(ch->desc->olc, struct oasis_olc_data, 1);
  OLC_STORAGE(ch->desc) = strdup(arg);

  memset(news_body, '\0', MAX_NEWS_LENGTH);

  editing_news = true;
  sprintf(subject, "%s", buf);

  clear_screen(ch->desc);
  send_editor_help(ch->desc);
  OLC_PATH(ch->desc) = strdup(news_file);

  strcpy(news_subject, subject);
  string_write(ch->desc, (char **) &news_body, MAX_NEWS_LENGTH, news_number, NULL);

  SET_BIT_AR(PLR_FLAGS, PRL_WRITING);
  STATE(ch->desc) = CON_NEDIT;

}

ACMD(do_readnews)      
{
  int news_number;
  bool found; 
  char news_dir[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];     

  one_argument(argument, arg);
  memset(buf,'\0', MAX_STRING_LENGTH);

  switch (subcmd) {  
  case SCMD_READ_NEWS:
    if (!*arg)
      news_number = (GET_NEWS(ch) >= news_counter - 1) ? news_counter - 1 : GET_NEWS(ch);
    else if (!isdigit(*arg)) {
      ch->write("Syntax: news <number>\r\n");
      return;
    } else         
      news_number = atoi(arg);

    sprintf(news_dir, "%s/%d.news", NEWS_DIR, news_number);
    break;
  case SCMD_READ_GOD_NEWS:
    if (!*arg)
      news_number = (GET_GNEWS(ch) >= god_news_counter - 1) ? god_news_counter - 1: GET_GNEWS(ch);
    else if (!isdigit(*arg)) {
      ch->write("Syntax: godnews <number>\r\n");
      return;
    } else
      news_number = atoi(arg);
    sprintf(news_dir, "%s/%d.godnews", NEWS_DIR, news_number);
    break;
  default:
    nmudlog(SYS_LOG, MAX(LVL_IMM, GET_INVIS_LEV(ch)), TRUE, "&RError: invalid sub-commands passed to do_readnews");
    return;
  }
  found = read_news(buf, news_dir);
  page_string(ch->desc, buf, 1);

  if (found && subcmd == SCMD_READ_NEWS && GET_NEWS(ch) <= news_number)
    GET_NEWS(ch) = GET_NEWS(ch) + 1;
  else if (found && subcmd == SCMD_READ_GOD_NEWS && GET_GNEWS(ch) <= news_number)
    GET_GNEWS(ch) = GET_GNEWS(ch) + 1;
}

bool read_news(char *buf, char *path)
{
  FILE *fl;
  char input[MAX_STRING_LENGTH], author[MAX_NAME_LENGTH], subject[MAX_SUBJECT_LENGTH];
  int number;

  memset(input, '\0', MAX_STRING_LENGTH);

  if (!(fl = fopen(path, "r"))) {
    sprintf(buf, "There is no news file with that number.\r\n");
    return (false);
  }                    

  fscanf(fl, "%d", &number);
  fscanf(fl, "%s", author);
  get_line(fl, subject);            

  sprintf(buf,                     
          "Number : %d\r\n"
          "Author : %s\r\n"   
          "Subject: %s\r\n"           
          "-----------------------------------------------------------------------",
          number, author, subject);

  for ( ; !feof(fl) ; ) {
    sprintf(buf, "%s%s\r\n", buf, input);
    get_line(fl, input);      
  }

  return (true);   
}
