/* ************************************************************
 * File: clanedit.c     From the guild patch by Sean Mountcastle *
 * Usage: For editing clan information               May 2001 *
 * Made into seperate file by Catherine Gore                  *
 *                                                            *
 * For use with CircleMUD 3.0/OasisOLC 2.0                    *
 ************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "genolc.h"
#include "oasis.h"
#include "improved-edit.h"
#include "constants.h"
#include "clan.h"

extern int hnum;
extern int newhelp;
HELP_DATA *help_info;
extern struct help_keywords *hkey;
extern struct help_keywords *keyword_list;

extern void get_one_line(FILE *fl, char *buf);
/* external functions */
//extern struct char_data *find_clan_char(struct char_data *ch, char *arg);

/* local functions */
void hedit_disp_menu(struct descriptor_data * d);
void hedit_free_help(HELP_DATA *hptr);
void hedit_parse(struct descriptor_data *d, char *arg);
void hedit_setup_new(struct descriptor_data *d);
void hedit_setup_existing(struct descriptor_data *d, int number);
void hedit_save_to_disk();
HELP_DATA *enqueue_help(void);
void dequeue_help(int helpnum);
void free_help(HELP_DATA *h);
void order_helps(void);
void create_helpfile(void);
void load_help(void);
void save_help(void);
char *get_help_name(int help);
char *get_blank_help_name(int help);
void get_keywords(HELP_DATA *hptr);
ACMD(do_helpinfo);

/******************************************************************************/
/** Routines                                                                 **/
/******************************************************************************/
ACMD(do_oasis_hedit)
{
  struct descriptor_data *d;
  char buf1[MAX_STRING_LENGTH];
  HELP_DATA *hptr = NULL;
  int number;

  /****************************************************************************/
  /** Parse any arguments.                                                   **/
  /****************************************************************************/
  one_argument(argument, buf1);

  if (GET_LEVEL(ch) < LVL_IMPL) {
    send_to_char(ch, "You can't modify help.\r\n");
    return;
  }

  d = ch->desc;

  if (!*buf1) {
    send_to_char(ch, "Specify a help number to edit.\r\n");
    return;
  } 
  number = atoi(buf1);
  if(number == 0) {
    send_to_char(ch, "Specify a valid help to edit (-1 for new help).\r\n");
    return;
  }
  if(number == -1) {
    CREATE(d->olc, struct oasis_olc_data, 1);
    OLC_ZONE(d) = 0;
    hedit_setup_new(d);
    hedit_disp_menu(d);
  } else {
    for (hptr = help_info; hptr && hptr->index != number; hptr=hptr->next);
      if (hptr && (hptr->index == number)) {
        CREATE(d->olc, struct oasis_olc_data, 1);
        OLC_ZONE(d) = 0;
        OLC_HELP(d) = hptr;
        hedit_disp_menu(d);
      } else {
        send_to_char(d->character, "Invalid help number! arg = %s, buf1 = %s number=%d\r\n", argument, buf1, number);
        return;
      }
  }
  STATE(d) = CON_HEDIT;
  act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
  SET_BIT_AR(PLR_FLAGS(ch), PLR_WRITING);

  mudlog(BRF, LVL_IMMORT, TRUE,
    "OLC: %s starts editing help.", GET_NAME(ch));
 
//  send_to_char(ch, "Saving help.\r\n");
  mudlog(CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE,
    "OLC: %s saves the help.", GET_NAME(ch));

//  hedit_save_to_disk();
}

void hedit_disp_menu(struct descriptor_data * d)
{

  if (!OLC_HELP(d))
    hedit_setup_new(d);
  
  get_char_colors(d->character);

  write_to_output(d,
    "&c Help number &W: &G[&g%d&G]\r\n"
    "&W1) &cKeywords        &W:&g %s\r\n"
    "&W2) &cBody            &W:\r\n&g %s\r\n"
    "&W3) &cAssoc Keywords  &W:&g %s\r\n"
    "&W4) &cMin Level       &W:&g %d\r\n"
    "&WP) &cPurge Help File \r\n"
    "&WQ) &cQuit \r\n"
    "&cEnter choice &W: ",
    
    OLC_HELP(d)->index,
    OLC_HELP(d)->keywords,
    OLC_HELP(d)->entry,
    OLC_HELP(d)->akeys,
    OLC_HELP(d)->min_level);

    fprintf(stderr, "menu help item %d, hnum = %d\r\n", OLC_HELP(d)->index, hnum);
  
  OLC_MODE(d) = HEDIT_MAIN_MENU;
}

void hedit_free_help(HELP_DATA *hptr)
{
  dequeue_help(hptr->index);
}

void hedit_parse(struct descriptor_data *d, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  char *oldtext = '\0';
  int number;

  switch (OLC_MODE(d)) {
  case HEDIT_CONFIRM_SAVE:
    switch (*arg) {
    case 'y':
    case 'Y':
      save_help();
      mudlog(CMP, LVL_GOD, TRUE, "OLC: %s edits help %d", GET_NAME(d->character), OLC_HELP(d)->index);
      fprintf(stderr, "saving help item %d, hnum = %d\r\n", OLC_HELP(d)->index, hnum);
      send_to_char(d->character, "Help saved to disk and memory.\r\n");
      cleanup_olc(d, CLEANUP_STRUCTS);
      break;
    case 'n':
    case 'N':
      /* free everything up, including strings etc */
      hnum--;
      cleanup_olc(d, CLEANUP_STRUCTS);
      break;
    default:
      send_to_char(d->character, "Invalid choice!\r\n");
      send_to_char(d->character, "Do you wish to save this help file?");
      break;
    }
    return;
    
    case HEDIT_MAIN_MENU:
      switch (*arg) {
      case 'q':
      case 'Q':
        if (OLC_VAL(d)) {
          /*. Something has been modified .*/
          send_to_char(d->character, "Do you wish to save this help file?");
          OLC_MODE(d) = HEDIT_CONFIRM_SAVE;
        } else {
	  hnum--;
          cleanup_olc(d, CLEANUP_STRUCTS);
        }
        return;
      case '1':
        write_to_output(d, "Enter keywords:\r\n");
        OLC_MODE(d) = HEDIT_KEYWORDS;
        break;
      case '2':
        clear_screen(d);
        send_editor_help(d);
        write_to_output(d, "Enter help entry: (/s saves /h for help)\r\n");
        if (OLC_HELP(d)->entry) {
         write_to_output(d, "%s\r\n", OLC_HELP(d)->entry);
         oldtext = strdup(OLC_HELP(d)->entry);
        }
        string_write(d, &OLC_HELP(d)->entry, MAX_STRING_LENGTH, 0, oldtext);
        OLC_VAL(d) = 1;
        break;
      case '3':
        write_to_output(d, "Enter associated keywords:\r\n");
        OLC_MODE(d) = HEDIT_AKEYS;       
        break;
      case '4':
        write_to_output(d, "Enter minimum level to see this help:\r\n ");
        OLC_MODE(d) = HEDIT_MIN_LEVEL;
        break;
      case 'p':
      case 'P':
        if (GET_LEVEL(d->character) >= LVL_IMPL) {
          newhelp = OLC_HELP(d)->index;  /* next new clan will get this one's number  */
          /* free everything up, including strings etc */
          cleanup_olc(d, CLEANUP_ALL);
          hnum--;
          send_to_char(d->character, "Help purged.\r\n");
        } else {
          write_to_output(d, "Sorry you are not allowed to do that at this time.\r\n");
          hedit_disp_menu(d);
        }
        return;
        break;
      default:
        write_to_output(d, "Invalid choice!\r\n");
        hedit_disp_menu(d);
        break;
      }
      return;
      
      case HEDIT_KEYWORDS:
        if (OLC_HELP(d)->keywords)
          free(OLC_HELP(d)->keywords);
        OLC_HELP(d)->keywords = strdup(arg);
        break;
        
      case HEDIT_ENTRY:
        if (OLC_HELP(d)->entry)
          free(OLC_HELP(d)->entry);
        OLC_HELP(d)->entry = strdup(arg);
      break;

      case HEDIT_AKEYS:
        if (OLC_HELP(d)->akeys)
          free(OLC_HELP(d)->akeys);
        OLC_HELP(d)->akeys = strdup(arg);
        break;
              
      case HEDIT_MIN_LEVEL:
   //     if (OLC_HELP(d)->min_level)
   //       free(OLC_HELP(d)->min_level);
        OLC_HELP(d)->min_level = atoi(arg);
        break;

      default:
        /* we should never get here */
        mudlog(BRF, LVL_GOD, TRUE, "SYSERR: Reached default case in clanedit_parse");
        break;
   }
   /* If we get this far, something has been changed */
   OLC_VAL(d) = 1;
   hedit_disp_menu(d);
}

void hedit_save_to_disk()
{
  save_help();
}

/*
* Create a new clan with some default strings.
*/
void hedit_setup_new(struct descriptor_data *d)
{
  int i;

  if ((OLC_HELP(d) = enqueue_help()) != NULL) {
    OLC_HELP(d)->index = newhelp;
    OLC_HELP(d)->keywords = strdup("Help Keywords");
    OLC_HELP(d)->entry = strdup("Help Entry");
    OLC_HELP(d)->akeys = strdup("Associated Keys");
    OLC_HELP(d)->min_level = GET_LEVEL(d->character); 
  } else
    fprintf(stderr, "SYSERR: Unable to create new help!\r\n");

  hnum++;

  if (newhelp == hnum)
    newhelp++;
  else 
    newhelp = hnum;

  OLC_VAL(d) = 0;
}

/*
* Setup a new help with existing data.
*/
void hedit_setup_existing(struct descriptor_data *d, int number)
{
  HELP_DATA *hptr = NULL;

  for (hptr = help_info; hptr && hptr->index != number; hptr=hptr->next);
    if (hptr && (hptr->index == number)) {
      OLC_HELP(d) = hptr;
      hedit_disp_menu(d);
    } else {
      send_to_char(d->character, "Invalid help number!\r\n");
      return;
    }
   STATE(d) = CON_HEDIT;
}

void free_help(HELP_DATA *h)
{
//  int    i;

  if (h != NULL) {
    free(h->keywords);
    free(h->entry);
    free(h->akeys);
    free(h);
  }
}

HELP_DATA *enqueue_help(void)
{
  HELP_DATA *hptr;

  /* This is the first help loaded if true */
  if (help_info == NULL) {
    if ((help_info = (HELP_DATA *) malloc(sizeof(HELP_DATA))) == NULL) {
      fprintf(stderr, "SYSERR: Out of memory for helps!");
      exit(1);
    } else {
      help_info->next = NULL;
      return (help_info);
    }
  } else { /* help_info is not NULL */
    for (hptr = help_info; hptr->next != NULL; hptr = hptr->next)
      /* Loop does the work */;
      if ((hptr->next = (HELP_DATA *) malloc(sizeof(HELP_DATA))) == NULL) {
        fprintf(stderr, "SYSERR: Out of memory for helps!");
        exit(1);
      } else {
        hptr->next->next = NULL;
        return (hptr->next);
      }
  }
  return NULL;
}


void dequeue_help(int helpnum)
{
  HELP_DATA *hptr = NULL, *temp;

  if (helpnum < 0 || helpnum > hnum) {
    log("SYSERR: Attempting to dequeue invalid help!\r\n");
    exit(1);
  } else {
    if (help_info->index != helpnum) {
      for (hptr = help_info; hptr->next && hptr->next->index != helpnum; hptr = hptr->next)
        ;
      if (hptr->next != NULL && hptr->next->index==helpnum) {
        temp       = hptr->next;
        hptr->next = temp->next;
        free_help(temp);
      }
    } else {
      /* The first one is the one being removed */
      hptr = help_info;
      help_info = help_info->next;
      free_help(hptr);
    }
  }
}


/* Puts helps in numerical order */
void order_helps(void)
{
  HELP_DATA *hptr = NULL, *temp = NULL;

  if (hnum > 2) {
    for (hptr = help_info; (hptr->next != NULL) && (hptr->next->next != NULL); hptr = hptr->next) {
      if (hptr->next->index > hptr->next->next->index) {
        temp = hptr->next;
        hptr->next = temp->next;
        temp->next = hptr->next->next;
        hptr->next->next = temp;
      }
    }
  }

  return;
}

/* Puts helps in numerical order */
void order_help_keywords(void)
{
  struct help_keywords *hptr = NULL, *temp = NULL;

  if (hnum > 2) {
    for (hptr = keyword_list; (hptr->next != NULL) && (hptr->next->next != NULL); hptr = hptr->next) {
      if (hptr->next->helpid > hptr->next->next->helpid) {
        temp = hptr->next;
        hptr->next = temp->next;
        temp->next = hptr->next->next;
        hptr->next->next = temp;
      }
    }
  }

  return;
}


void create_helpfile(void)
{
  FILE   *hfile;

  /* Should be no reason it can't open... */
  if ((hfile = fopen(HELP_FILE, "wt")) == NULL) {
    fprintf(stderr, "SYSERR: Cannot save help!\n");
    exit(0);
 }

  /* The total number of helps */
  fprintf(hfile, "1\r\n");
  /* The Void help */
  fprintf(hfile,  "#0\r\n"
    "The Void Help~\r\n"
    "Void Body~\r\n"
    "Void Akeys~\r\n"
    "Void Level~\r\n"
    "$\r\n");

  fclose(hfile);
}

void load_help(void)
{
  FILE   *fl = NULL;
  int    helpnum = 0, line_num = 0, i = 0, tmp, tmp1, tmp2;
  long helpgold = 0;
  char   name[80], buf[MAX_INPUT_LENGTH];
  char hentry1[MAX_STRING_LENGTH], line[READ_SIZE + 1], hentry2[MAX_STRING_LENGTH];
  char   *ptr;
  size_t entrylen;
  HELP_DATA *hptr = NULL;
  bool   newh = FALSE;


  if ((fl = fopen(HELP_FILE, "rt")) == NULL) {
    fprintf(stderr, "INFO: No help file! New file will be created.");
    create_helpfile();
    fl = fopen(HELP_FILE, "rt");
  }

  line_num += get_line(fl, buf);
  if (sscanf(buf, "%d", &helpnum) != 1) {
    fprintf(stderr, "Format error in help, line %d (number of helps)\n", line_num);
    create_helpfile();
//    exit(0);
  }
  /* Setup the global total number of helps */
  hnum = helpnum;

  /* process each help in order */
  for (helpnum=0;helpnum < hnum;helpnum++) {
    /* Get the info for the individual helps */
    line_num += get_line(fl, buf);
    if (sscanf(buf, "#%d", &tmp) != 1) {
      fprintf(stderr, "Format error in help (No Unique HID), line %d\n, line = %s", line_num, line);
      exit(0);
    }
    /* create some help shaped memory space */
    if ((hptr = enqueue_help()) != NULL) {
      hptr->index = tmp;

      /* setup the global number of next new help number */
      if (!newh) {
        if (hptr->index != helpnum) {
          newhelp = helpnum;
          newh = TRUE;
        }
      }
      if (newh) {
        if (newhelp == hptr->index) {
          newhelp = hnum;
          newh = FALSE;
        }
      } else
        newhelp = hptr->index + 1;

      /* Now get the keywords of the help */
//      line_num += get_line(fl, buf);
//      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
//        *ptr = '\0';
//      hptr->keywords = strdup(buf);
        hptr->keywords = fread_string(fl, buf);
      /* Now get the body of help */
//      line_num += get_line(fl, buf);
//      while (*buf != '~' && entrylen < sizeof(hentry2) - 1) {
//     while ((ptr = strchr(buf, '~')) == NULL) {
//       entrylen += strlcpy(hentry2 + entrylen, buf, sizeof(hentry2) - entrylen);

//       snprintf(hentry1, sizeof(hentry1), "%s", buf);
//       strncat(hentry2, hentry1, sizeof(hentry2));
//
//       line_num += get_line(fl, buf);
//      }
      hptr->entry = fread_string(fl, buf);
//      hptr->entry = strdup(hentry2);

      /* Now get the associated keys */
//      line_num += get_line(fl, buf);
//      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
//        *ptr = '\0';
//      hptr->akeys = strdup(buf);
      hptr->akeys = fread_string(fl, buf);

      /* Now get the min level */
//      line_num += get_line(fl, buf);
//      if ((ptr = strchr(buf, '~')) != NULL) /* take off the '~' if it's there */
//        *ptr = '\0';
//      hptr->min_level = atoi(buf);
      hptr->min_level = atoi(fread_string(fl, buf));

      get_keywords(hptr);


    } else break;
    /* process the next help */
  }
  /* done processing helps -- close the file */
  fclose(fl);
}


void save_help(void)
{
  FILE   *hfile;
  int    helpnum = 0, i;
  HELP_DATA *hptr = help_info;

  if (hptr == NULL) {
    fprintf(stderr, "SYSERR: No helps to save!!\n");
    return;
  }
  if ((hfile = fopen(HELP_FILE, "wt")) == NULL) {
    fprintf(stderr, "SYSERR: Cannot save helps!\n");
    exit(0);
  }

  /* Put the helps in order */
//  order_helps();
//  order_help_keywords();

  /* The total number of helps */
  fprintf(hfile, "%d\r\n", hnum);

  /* Save each help */
  while (helpnum < hnum && hptr != NULL) {
    fprintf(hfile,  
		"#%d\r\n"   /* Help ID */
                "%s~\r\n"   /* Help Keywords */
	        "%s~\r\n"   /* Help Entry */
        	"%s~\r\n"   /* Help Assoc Keywords */
	        "%d~\r\n",   /* Help Min Level */
	        hptr->index,   
		hptr->keywords,
	        hptr->entry,   
		hptr->akeys, 
		hptr->min_level);

    /* process the next help */
    hptr = hptr->next;
    helpnum++;
  }
  fprintf(hfile, "$\r\n");

  /* done processing helps */
  fclose(hfile);

//  free(help_info);
  free(keyword_list);
  load_help();
}

/* Helper functions for help information */
char *get_help_name(int help)
{
  HELP_DATA *hptr;
  char *help_name;

  for (hptr = help_info; hptr && hptr->index != help; hptr = hptr->next);
  if(hptr != NULL)
    help_name = strdup(hptr->keywords);
  else
    help_name = strdup("Invalid Help");

  return (help_name);
}

char *get_blank_help_name(int help)
{
  HELP_DATA *hptr;
  char *help_name;

  for (hptr = help_info; hptr && hptr->index != help; hptr = hptr->next);
  help_name = strdup(hptr->keywords);

  return (help_name);
}


ACMD(do_helpinfo)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int i = 0;
  HELP_DATA *hptr;

  *buf = '\0';
  *buf2 = '\0';

  if (!*argument)
  {
    send_to_char(ch, "Yes, but what help?");
    return;
  }

  for (hptr = help_info; hptr; hptr = hptr->next) {

    if (hptr->index != atoi(argument))
      continue;

    snprintf(buf2, sizeof(buf2), "\r\n&CHelp #    :&G    %2d\r\n"
                                 "&CKeywords  :&G %s&n \r\n"
                                 "&CInfo      :   &G  \r\n%s&n\r\n\r\n",
      hptr->index, hptr->keywords, hptr->entry);
    strncat(buf, buf2, sizeof(buf));

    snprintf(buf2, sizeof(buf2), "&CSee Also:  %s\r\n", hptr->akeys);
    strncat(buf, buf2, sizeof(buf));

  }
  page_string(ch->desc, buf, 1);
}

void get_keywords(HELP_DATA *hptr)
{
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH], buf4[MAX_STRING_LENGTH];
  char buf5[MAX_STRING_LENGTH], buf6[MAX_STRING_LENGTH];

  snprintf(arg, sizeof(arg), "%s", hptr->keywords);

  half_chop(arg, buf, buf2);
  if(*buf2)
  half_chop(buf2, buf3, buf4);
  if(*buf4)
  two_arguments(buf4, buf5, buf6);

  if (*buf)
  {
   CREATE(hkey, struct help_keywords, 1);
   hkey->next = keyword_list;
   keyword_list = hkey;
   hkey->keyword = strdup(buf);
   hkey->helpid = hptr->index;
  }

  if (*buf2 && !*buf3)
  {
   CREATE(hkey, struct help_keywords, 1);
   hkey->next = keyword_list;
   keyword_list = hkey;
   hkey->keyword = strdup(buf2);
   hkey->helpid = hptr->index;
  }

  if (*buf3)
  {
   CREATE(hkey, struct help_keywords, 1);
   hkey->next = keyword_list;
   keyword_list = hkey;
   hkey->keyword = strdup(buf3);
   hkey->helpid = hptr->index;
  }

  if (*buf4 && !*buf5)
  {
   CREATE(hkey, struct help_keywords, 1);
   hkey->next = keyword_list;
   keyword_list = hkey;
   hkey->keyword = strdup(buf4);
   hkey->helpid = hptr->index;
  }

  if (*buf5)
  {
   CREATE(hkey, struct help_keywords, 1);
   hkey->next = keyword_list;
   keyword_list = hkey;
   hkey->keyword = strdup(buf5);
   hkey->helpid = hptr->index;
  }

  if (*buf6)
  {
   CREATE(hkey, struct help_keywords, 1);
   hkey->next = keyword_list;
   keyword_list = hkey;
   hkey->keyword = strdup(buf6);
   hkey->helpid = hptr->index;
  }
}

ACMD(do_listkeywords)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int i = 0;
  struct help_keywords *hptr;

  *buf = '\0';
  *buf2 = '\0';

  for (hptr = keyword_list; hptr; hptr = hptr->next) {
//  for (i=0; i < hnum; i++) {
    snprintf(buf2, sizeof(buf2), "Help #%d Keyword: %s\r\n", hptr->helpid, hptr->keyword);
    strncat(buf, buf2, sizeof(buf));
  }

  page_string(ch->desc, buf, 1);
}
