#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"

#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "logger.h"

ACMD(do_warn);
void kill_warnings(const char *charname);
void load_warnings(struct char_data *ch);
void save_warnings(struct char_data *ch);



#define WARN_USAGE \
  "Usage: warn <player> [message]\r\n"

ACMD(do_warn)
{
  bool is_file = FALSE;
  char arg[MIL];
  int num = 0, player_i = 0;
  struct char_data *vict = NULL;
  struct char_file_u ch_store;
  struct warning_data *warn = NULL;
  char buf[MSL];

  argument = one_argument(argument, arg);

  if (str_cmp(arg, "file") == 0) {
    argument = one_argument(argument, arg);
    is_file = TRUE;
  }

  skip_spaces(&argument);

  if (arg[0] == '\0')
    send_to_char(ch, "%s", WARN_USAGE);
  else {
    if (!is_file)
      vict = get_player_vis(ch, arg, NULL, FIND_CHAR_WORLD);
    else {
      if ((player_i = load_char(arg, &ch_store)) >= 0) {
	/* Allocate a new character structure and initialize it. */
	CREATE(vict, struct char_data, 1);
	clear_char(vict);

	/* Unpack the character data. */
        init_char(vict);
//	store_to_char(&ch_store, vict);

	/* Load the player's warnings. */
	load_warnings(vict);
      }
    }

    if (vict == NULL)
      send_to_char(ch, "There's nobody here by that name.\r\n");
    else if (argument[0] == '\0') {
      if (GET_WARNINGS(vict) == NULL)
	send_to_char(ch, "&Y%s &Chas no warnings.&n\r\n", GET_NAME(vict));
      else {
	send_to_char(ch, "&Y%s&C has these warnings&W:&n\r\n", GET_NAME(vict));
	for (warn = GET_WARNINGS(vict); warn != NULL; warn = warn->next) {
	  send_to_char(ch, "&g%2d) &M%-12.12s &W: &G%-10.10s &W: &Y%s&n\r\n", ++num,
		asctime(localtime(&(warn->time))) + 4, warn->admin,
		warn->message);
	}
      }
    } else {
      /* Create a new warning record. */
      CREATE(warn, struct warning_data, 1);

      /* Initialize the warning fields. */
      warn->admin = strdup(GET_NAME(ch));
      warn->message = strdup(argument);
      warn->time = time(0);
      warn->next = NULL;

      /* Add the warning to the player's warning list. */
      warn->next = GET_WARNINGS(vict);
      GET_WARNINGS(vict) = warn;

      /* Log the warning to online immortals. */
      nmudlog(GOD_LOG, LVL_GOD, TRUE, "%s issued a warning to %s : &Y%s&n", GET_NAME(ch), GET_NAME(vict), argument);

      /* Tell the player. */
      sprintf(buf, "&R%s is warning you about the following issue: %s\r\n"
                   "This issue will be stored in your history.\r\n&N", GET_NAME(ch), argument);
      send_to_char(vict, buf);
//      send_to_char(vict,
//	"You have received a warning from the immortal staff.  We kindly suggest\r\n"
//	"this warning be heeded and your ways amended.  The reason for this warning\r\n"
//	"is as follows: %s\r\n", argument);

      /* Tell the imm it was done. */
      sprintf(buf, "You have warned %s about the following: %s\r\n"
                   "This is now stored in their history file\r\n", GET_NAME(vict), argument);
      send_to_char(ch, buf);

      /* Save the victim's warning messages. */
      save_warnings(vict);
    }

    /* Clean up after ourselves. */
    if (is_file && vict != NULL)
      free_char(vict);
  }
}



void kill_warnings(const char *charname)
{
  char filename[MAX_STRING_LENGTH];

  if (get_filename(filename, sizeof(filename), WARN_FILE, charname)) {
    if (remove(filename) < 0 && errno != ENOENT)
      log("SYSERR: deleting warning file %s: %s", filename, strerror(errno));
  }
}



void load_warnings(struct char_data *ch)
{   
  char filename[MAX_STRING_LENGTH];
  char line[MAX_STRING_LENGTH], tag[MAX_STRING_LENGTH], *p = NULL;
  FILE *file = NULL;
  struct warning_data *twarn = NULL, *warn = NULL;

  get_filename(filename, sizeof(filename), WARN_FILE, GET_NAME(ch));

  if ((file = fopen(filename, "rt")) == NULL) {
    if (errno != ENOENT) {
      log("SYSERR: Couldn't open warn file '%s' for %s.", filename,
	GET_NAME(ch));
    }
  } else {
    while (get_line(file, line)) {
      /* Skip blank lines too. */
      if (line[0] == '\0')
        continue;
       
      /* Get the first word on the line. */
      p = any_one_arg(line, tag);
      
      /* Skip any whitespace. */
      skip_spaces(&p);
        
      /*
       * Copy what's left of the line back.
       */
      strncpy(line, p, sizeof(line));

      switch (tag[0]) {
      case '#':
	if (str_cmp(tag, "#WARNING") == 0) {
	  if (warn != NULL) {
	    log("SYSERR: #WARNING tag with no #END in file '%s'.", filename);
	    if (warn->admin != NULL)
	      free(warn->admin);
	    if (warn->message != NULL)
	      free(warn->message);
	    free(warn);
	  }
	  CREATE(warn, struct warning_data, 1);
	  warn->admin = strdup("Nobody");
	  warn->message = NULL;
	  warn->time = 0;
	  warn->next = NULL;
	} else if (str_cmp(tag, "#END") == 0) {
	  if (warn == NULL)
	    log("SYSERR: #END tag without #WARNING in file '%s'.", filename);
	  else {
	    if (GET_WARNINGS(ch) == NULL ||
		warn->time > GET_WARNINGS(ch)->time) {
	      warn->next = GET_WARNINGS(ch);
	      GET_WARNINGS(ch) = warn;
	    } else {
	      for (twarn = GET_WARNINGS(ch); twarn; twarn = twarn->next) {
		if (twarn->next == NULL)
		  break;
		else if (warn->time > twarn->next->time)
		  break;
	      }
	      warn->next = twarn->next;
	      twarn->next = warn;
	    }
	  }
	  warn = NULL;
	} else
	  log("SYSERR: Unknown tag [%s] in file [%s].", tag, filename);
	break;
      case 'a':
      case 'A':
        if (str_cmp(tag, "Admin") == 0) {
	  if (warn == NULL) {
	    log("SYSERR: 'Admin' tag without #WARNING tag in file '%s'.",
		filename);
	  } else {
	    if (warn->admin != NULL)
	      free(warn->admin);
	    warn->admin = strdup(line);
	  }
	} else
	  log("SYSERR: Unknown tag [%s] in file [%s].", tag, filename);
	break;
      case 'm':
      case 'M':
        if (str_cmp(tag, "Message") == 0) {
	  if (warn == NULL) {
	    log("SYSERR: 'Message' tag without #WARNING tag in file '%s'.",
		filename);
	  } else {
            if (warn->message != NULL)
	      free(warn->message);
	    warn->message = strdup(line);
	  }
	} else
	  log("SYSERR: Unknown tag [%s] in file [%s].", tag, filename);
	break;
      case 't':
      case 'T':
        if (str_cmp(tag, "Time") == 0) {
	  if (warn == NULL) {
	    log("SYSERR: 'Time' tag without #WARNING tag in file '%s'.",
		filename);
	  } else
	    warn->time = (time_t) atoi(line);
	} else
	  log("SYSERR: Unknown tag [%s] in file [%s].", tag, filename);
	break;
      default:
	log("SYSERR: Unknown tag [%s] in file [%s].", tag, filename);
	break;
      }
    }

    /* Guard against an incomplete warning. */
    if (warn != NULL) {
      log("SYSERR: #WARNING tag with no #END in file '%s'.", filename);
      if (warn->admin != NULL)
	free(warn->admin);
      if (warn->message != NULL)
	free(warn->message);
      free(warn);
    }

    /* Close the file. */
    fclose(file);
  }
} 



void save_warnings(struct char_data *ch)
{
  char filename[MAX_STRING_LENGTH];
  FILE *file = NULL;
  struct warning_data *warn = NULL;

  if (!get_filename(filename, sizeof(filename), WARN_FILE, GET_NAME(ch)))
    log("save_warnings(): Couldn't get filename for %s.", GET_NAME(ch));
  else {
    if (GET_WARNINGS(ch) == NULL)
      remove(filename);
    else if ((file = fopen(filename, "wt")) == NULL) {
      log("SYSERR: Couldn't save warnings for %s in '%s'.", GET_NAME(ch),
	filename);
    } else {
      for (warn = GET_WARNINGS(ch); warn != NULL; warn = warn->next) {
	fprintf(file, "#WARNING\n");
	fprintf(file, "Admin           %s\n", warn->admin);
	fprintf(file, "Message         %s\n", warn->message);
	fprintf(file, "Time            %d\n", (int) warn->time);
	fprintf(file, "#END\n");
      }
      fclose(file);
    }
  }
}

