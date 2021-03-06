/**************************************************************************
*  File: comm.c                                            Part of tbaMUD *
*  Usage: Communication, socket handling, main(), central game loop.      *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#define __COMM_C__

#include "conf.h"
#include "sysdep.h"

#if CIRCLE_GNU_LIBC_MEMORY_TRACK
# include <mcheck.h>
#endif

#ifdef CIRCLE_MACINTOSH		/* Includes for the Macintosh */
# define SIGPIPE 13
# define SIGALRM 14
  /* GUSI headers */
# include <sys/ioctl.h>
  /* Codewarrior dependant */
# include <SIOUX.h>
# include <console.h>
#endif

#ifdef CIRCLE_WINDOWS		/* Includes for Win32 */
# ifdef __BORLANDC__
#  include <dir.h>
# else /* MSVC */
#  include <direct.h>
# endif
# include <mmsystem.h>
#endif /* CIRCLE_WINDOWS */

#ifdef CIRCLE_AMIGA		/* Includes for the Amiga */
# include <sys/ioctl.h>
# include <clib/socket_protos.h>
#endif /* CIRCLE_AMIGA */

#ifdef CIRCLE_ACORN		/* Includes for the Acorn (RiscOS) */
# include <socklib.h>
# include <inetlib.h>
# include <sys/ioctl.h>
#endif

/* Note, most includes for all platforms are in sysdep.h.  The list of
 * files that is included is controlled by conf.h for that platform. */

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "house.h"
#include "oasis.h"
#include "genolc.h"
#include "dg_scripts.h"
#include "dg_event.h"
#include "screen.h"
#include "arena.h"
#include "crystalgate.h"
#include "quest.h"
#include "mail.h"

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#else
#include "telnet.h"
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

/* externs */
extern struct ban_list_element *ban_list;
extern int num_invalid;
extern char *GREETINGS;
extern const char *tbamud_version;
extern const char *oasisolc_version;
extern const char *ascii_pfiles_version;
extern int circle_restrict;
extern int mini_mud;
extern int no_rent_check;
extern int *cmd_sort_info;
extern int news_counter;
extern int gnews_counter;

extern struct time_info_data time_info;		/* In db.c */
extern char *help;

/* mob intelligence */
extern void noncombat_intel(void);


/* local globals */
struct descriptor_data *descriptor_list = NULL;		/* master desc list */
struct txt_block *bufpool = 0;	/* pool of large output buffers */
int buf_largecount = 0;		/* # of large buffers which exist */
int buf_overflows = 0;		/* # of overflows of output */
int buf_switches = 0;		/* # of switches from small to large buf */
int circle_shutdown = 0;	/* clean shutdown */
int circle_reboot = 0;		/* reboot the game after a shutdown */
int no_specials = 0;		/* Suppress ass. of special routines */
int max_players = 0;		/* max descriptors available */
int tics_passed = 0;			/* for extern checkpointing */
int scheck = 0;			/* for syntax checking mode */
struct timeval null_time;	/* zero-valued time structure */
byte reread_wizlist;		/* signal: SIGUSR1 */
byte emergency_unban;		/* signal: SIGUSR2 */
FILE *logfile = NULL;		/* Where to send the log messages. */
int dg_act_check;		/* toggle for act_trigger */
unsigned long pulse = 0;        /* number of pulses since game start */
static bool fCopyOver;          /* Are we booting in copyover mode? */
ush_int port;
socket_t mother_desc;
int log_this_messg;
char *last_act_message = NULL;

void proc_color(char *inbuf, int color);

char StatColors(int cur, int max);
char *DisplayPrompt(struct char_data *ch);
char *strip_colors(const char *arg);
int GET_TOTAL_LEVEL(struct char_data *ch);



/* local functions */
RETSIGTYPE reread_wizlists(int sig);
RETSIGTYPE unrestrict_game(int sig);
RETSIGTYPE reap(int sig);
RETSIGTYPE checkpointing(int sig);
RETSIGTYPE hupsig(int sig);
ssize_t perform_socket_read(socket_t desc, char *read_point,size_t space_left);
ssize_t perform_socket_write(socket_t desc, const char *txt,size_t length);
void echo_off(struct descriptor_data *d);
void echo_on(struct descriptor_data *d);
void circle_sleep(struct timeval *timeout);
int get_from_q(struct txt_q *queue, char *dest, int *aliased);
void init_game(ush_int port);
void signal_setup(void);
void game_loop(socket_t mother_desc);
socket_t init_socket(ush_int port);
int new_descriptor(socket_t s);
int get_max_players(void);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void timediff(struct timeval *diff, struct timeval *a, struct timeval *b);
void timeadd(struct timeval *sum, struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
void nonblock(socket_t s);
int perform_subst(struct descriptor_data *t, char *orig, char *subst);
void record_usage(void);
char *make_prompt(struct descriptor_data *point);
void check_idle_passwords(void);
void heartbeat(int heart_pulse);
void init_descriptor (struct descriptor_data *newd, int desc);
size_t proc_colors(char *txt, size_t maxlen, int parse);

struct in_addr *get_bind_addr(void);
int parse_ip(const char *addr, struct in_addr *inaddr);
int set_sendbuf(socket_t s);
void free_bufpool(void);
void setup_log(const char *filename, int fd);
int open_logfile(const char *filename, FILE *stderr_fp);
#if defined(POSIX)
sigfunc *my_signal(int signo, sigfunc *func);
#endif

byte webster_file_ready = FALSE;/* signal: SIGUSR2 */
RETSIGTYPE websterlink(int sig);
extern void handle_webster_file();
void copyover_recover(void);
size_t proc_colors(char *txt, size_t maxlen, int parse);
char *email_error_message(int error_code);
int validate_email(char *check_addr);
char *output_email(char *mail_addr);

//RETSIGTYPE SigTerm(int sig);

void weather_change(void);
void check_fishing();
void fishing_message();
void check_mining();
void death_tick();
void regen_update();
void check_auction();
void warzone_update();
//void turn_whointo_html();
void quest_update();
void cycle_crygate_msgs();
void check_lumber();
void lose_blood();

/* extern functions */
void reboot_wizlists(void);
void boot_world(void);
void affect_update(void);	/* In magic.c */
void mobile_activity(void);
void perform_violence(void);
void show_string(struct descriptor_data *d, char *input);
int isbanned(char *hostname);
void weather_and_time(int mode);
int perform_alias(struct descriptor_data *d, char *orig, size_t maxlen);
void free_messages(void);
void board_clear_all(void);
void free_social_messages(void);
void free_invalid_list(void);
void free_command_list(void);
void free_save_list(void);
void load_config(void);
void new_hist_messg(struct descriptor_data *d, const char *msg);
extern void room_affect_update(void);
extern void SpecialMessageUpdate(void);

/* extern fcnts */
WILD_DATA *get_wd(COORD_DATA *coord);
void    reboot_wizlists(void);
void    boot_world(void);
void    affect_update(void);    /* In spells.c */
void    mobile_activity(void);
void    perform_violence(void);
void    show_string(DESCRIPTOR_DATA *d, char *input);
int             isbanned(char *hostname);
void    weather_and_time(int mode);
void    event_init(void);
void    event_process(void);
void    wild_remove_update(void);
void    ferry_move(void);
void    build_works_process(void);
void    SaveBuildings(void);
void    SaveShips(void);
void    SaveGoodsMarketsTable(bool bTime);
void    SaveMarkets(void);
void    SaveTradingPost(void);

#ifdef __CXREF__
#undef FD_ZERO
#undef FD_SET
#undef FD_ISSET
#undef FD_CLR
#define FD_ZERO(x)
#define FD_SET(x, y) 0
#define FD_ISSET(x, y) 0
#define FD_CLR(x, y)
#endif

/*  main game loop and related stuff */

#if defined(CIRCLE_WINDOWS) || defined(CIRCLE_MACINTOSH)
/* Windows and Mac do not have gettimeofday, so we'll simulate it. Borland C++ 
 * warns: "Undefined structure 'timezone'" */
void gettimeofday(struct timeval *t, struct timezone *dummy)
{
#if defined(CIRCLE_WINDOWS)
  DWORD millisec = GetTickCount();
#elif defined(CIRCLE_MACINTOSH)
  unsigned long int millisec;
  millisec = (int)((float)TickCount() * 1000.0 / 60.0);
#endif

  t->tv_sec = (int) (millisec / 1000);
  t->tv_usec = (millisec % 1000) * 1000;
}

#endif	/* CIRCLE_WINDOWS || CIRCLE_MACINTOSH */

int main(int argc, char **argv)
{
  int pos = 1;
  const char *dir;

#ifdef MEMORY_DEBUG 
  zmalloc_init(); 
#endif

#if CIRCLE_GNU_LIBC_MEMORY_TRACK
  mtrace();	/* This must come before any use of malloc(). */
#endif

#ifdef CIRCLE_MACINTOSH
  /* ccommand() calls the command line/io redirection dialog box from
   * Codewarriors's SIOUX library. */
  argc = ccommand(&argv);
  /* Initialize the GUSI library calls.  */
  GUSIDefaultSetup();
#endif

  /* Load the game configuration. We must load BEFORE we use any of the 
   * constants stored in constants.c.  Otherwise, there will be no variables 
   * set to set the rest of the vars to, which will mean trouble --> Mythran */
  CONFIG_CONFFILE = NULL;
  while ((pos < argc) && (*(argv[pos]) == '-')) {
    if (*(argv[pos] + 1) == 'f') {
      if (*(argv[pos] + 2))
	CONFIG_CONFFILE = argv[pos] + 2;
      else if (++pos < argc)
	CONFIG_CONFFILE = argv[pos];
      else {
	puts("SYSERR: File name to read from expected after option -f.");
	exit(1);
      }
    }
    pos++;
  }
  pos = 1;

  if (!CONFIG_CONFFILE)
    CONFIG_CONFFILE = strdup(CONFIG_FILE);

  load_config();

  port = CONFIG_DFLT_PORT;
  dir = CONFIG_DFLT_DIR;

  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
    case 'f':
      if (! *(argv[pos] + 2))
	++pos;
      break;
    case 'o':
      if (*(argv[pos] + 2))
	CONFIG_LOGNAME = argv[pos] + 2;
      else if (++pos < argc)
	CONFIG_LOGNAME = argv[pos];
      else {
	puts("SYSERR: File name to log to expected after option -o.");
	exit(1);
      }
      break;
    case 'C': /* -C<socket number> - recover from copyover, this is the control socket */
       fCopyOver = TRUE;
       mother_desc = atoi(argv[pos]+2);
      break;
    case 'd':
      if (*(argv[pos] + 2))
	dir = argv[pos] + 2;
      else if (++pos < argc)
	dir = argv[pos];
      else {
	puts("SYSERR: Directory arg expected after option -d.");
	exit(1);
      }
      break;
    case 'm':
      mini_mud = 1;
      no_rent_check = 1;
      puts("Running in minimized mode & with no rent check.");
      break;
    case 'c':
      scheck = 1;
      puts("Syntax check mode enabled.");
      break;
    case 'q':
      no_rent_check = 1;
      puts("Quick boot mode -- rent check supressed.");
      break;
    case 'r':
      circle_restrict = 1;
      puts("Restricting game -- no new players allowed.");
      break;
    case 's':
      no_specials = 1;
      puts("Suppressing assignment of special routines.");
      break;
    case 'h':
      /* From: Anil Mahajan. Do NOT use -C, this is the copyover mode and 
       * without the proper copyover.dat file, the game will go nuts! */
      printf("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [port #]\n"
              "  -c             Enable syntax check mode.\n"
              "  -d <directory> Specify library directory (defaults to 'lib').\n"
              "  -h             Print this command line argument help.\n"
              "  -m             Start in mini-MUD mode.\n"
	      "  -f<file>       Use <file> for configuration.\n"
	      "  -o <file>      Write log to <file> instead of stderr.\n"
              "  -q             Quick boot (doesn't scan rent for object limits)\n"
              "  -r             Restrict MUD -- no new players allowed.\n"
              "  -s             Suppress special procedure assignments.\n"
              " Note:		These arguments are 'CaSe SeNsItIvE!!!'\n",
		 argv[0]
      );
      exit(0);
    default:
      printf("SYSERR: Unknown option -%c in argument string.\n", *(argv[pos] + 1));
      break;
    }
    pos++;
  }

  if (pos < argc) {
    if (!isdigit(*argv[pos])) {
      printf("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [port #]\n", argv[0]);
      exit(1);
    } else if ((port = atoi(argv[pos])) <= 1024) {
      printf("SYSERR: Illegal port number %ld.\n", port);
      exit(1);
    }
  }

  /* All arguments have been parsed, try to open log file. */
  setup_log(CONFIG_LOGNAME, STDERR_FILENO);

  /* Moved here to distinguish command line options and to show up
   * in the log if stderr is redirected to a file. */
  log("Loading configuration.");
  log("%s", tbamud_version);
  log("%s", oasisolc_version);
  log("%s", DG_SCRIPT_VERSION);
  log("%s", ascii_pfiles_version);

  if (chdir(dir) < 0) {
    perror("SYSERR: Fatal error changing to data directory");
    exit(1);
  }
  log("Using %s as data directory.", dir);

  if (scheck)
    boot_world();
  else {
    log("Running game on port %ld.", port);
    init_game(port);
  }

  log("Clearing game world.");
  destroy_db();

  if (!scheck) {
    log("Clearing other memory.");
    free_bufpool();             /* comm.c */
    free_player_index();	/* players.c */
    free_messages();		/* fight.c */
    free_text_files();		/* db.c */
    board_clear_all();		/* boards.c */
    free(cmd_sort_info);	/* act.informative.c */
    free_command_list();        /* act.informative.c */
    free_social_messages();	/* act.social.c */
//    free_help_table();		/* db.c */
    free_invalid_list();	/* ban.c */
    free_save_list();		/* genolc.c */
    free_strings(&config_info, OASIS_CFG); /* oasis_delete.c */
  }

  if (last_act_message) 
    free(last_act_message);

  /* probably should free the entire config here.. */
  free(CONFIG_CONFFILE);

  log("Done.");

#ifdef MEMORY_DEBUG
  zmalloc_check();
#endif

  return (0);
}

int enter_player_game(struct descriptor_data *d);
extern time_t boot_time;

/* Reload players after a copyover */
void copyover_recover()
{
  struct descriptor_data *d;
  FILE *fp;
  char host[1024];
  int desc, player_i;
  bool fOld;
  char name[MAX_INPUT_LENGTH];
  long pref;

  log ("Copyover recovery initiated");

  fp = fopen (COPYOVER_FILE, "r");
  /* there are some descriptors open which will hang forever then ? */
  if (!fp) {
    perror ("copyover_recover:fopen");
    log ("Copyover file not found. Exitting.\n\r");
    exit (1);
  }

  /* In case something crashes - doesn't prevent reading  */
  unlink (COPYOVER_FILE);

  /* read boot_time - first line in file */
  fscanf(fp, "%ld\n", &boot_time);

  for (;;) {
    fOld = TRUE;
    fscanf (fp, "%d %ld %s %s\n", &desc, &pref, name, host);
    if (desc == -1)
      break;

    /* Write something, and check if it goes error-free */
    if (write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r") < 0) {
      close (desc); /* nope */
      continue;
    }

    /* create a new descriptor */
    CREATE (d, struct descriptor_data, 1);
    memset ((char *) d, 0, sizeof (struct descriptor_data));
    init_descriptor (d,desc); /* set up various stuff */

    strcpy(d->host, host);
    d->next = descriptor_list;
    descriptor_list = d;

    d->connected = CON_CLOSE;

    /* Now, find the pfile */
    CREATE(d->character, struct char_data, 1);
    clear_char(d->character);
    CREATE(d->character->player_specials, struct player_special_data, 1);
    d->character->desc = d;

    if ((player_i = load_char(name, d->character)) >= 0) {
      GET_PFILEPOS(d->character) = player_i;
      if (!PLR_FLAGGED(d->character, PLR_DELETED)) {
        REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
        REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
	REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRYO);
      } else
        fOld = FALSE;
    } else
      fOld = FALSE;

    /* Player file not found?! */
    if (!fOld) {
      write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r");
      close_socket (d);
    } else {
      write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r");
      GET_PREF(d->character) = pref;
      enter_player_game(d);
      d->connected = CON_PLAYING;
      look_at_room(d->character, 0);
    }
  }
  fclose (fp);
}

/* Init sockets, run game, and cleanup sockets */
void init_game(ush_int port)
{
  /* We don't want to restart if we crash before we get up. */
  touch(KILLSCRIPT_FILE);

  circle_srandom(time(0));

  log("Finding player limit.");
  max_players = get_max_players();

  /* If copyover mother_desc is already set up */
  if (!fCopyOver) {
     log ("Opening mother connection.");
     mother_desc = init_socket (port);
  }

  event_init();

  /* set up hash table for find_char() */
  init_lookup_table();

  boot_db();

#if defined(CIRCLE_UNIX) || defined(CIRCLE_MACINTOSH)
  log("Signal trapping.");
  signal_setup();
#endif

  /* If we made it this far, we will be able to restart without problem. */
  remove(KILLSCRIPT_FILE);

  if (fCopyOver) /* reload players */
  copyover_recover();

  log("Entering game loop.");

  game_loop(mother_desc);

  Crash_save_all();

  log("Closing all sockets.");
  while (descriptor_list)
    close_socket(descriptor_list);

  CLOSE_SOCKET(mother_desc);

  if (circle_reboot != 2)
    save_all();

  log("Saving current MUD time.");
  save_mud_time(&time_info);

  if (circle_reboot) {
    log("Rebooting.");
    exit(52);			/* what's so great about HHGTTG, anyhow? */
  }
  log("Normal termination of game.");
}

/* init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens. */
socket_t init_socket(ush_int port)
{
  socket_t s;
  struct sockaddr_in sa;
  int opt;

#ifdef CIRCLE_WINDOWS
  {
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(1, 1);

    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
      log("SYSERR: WinSock not available!");
      exit(1);
    }

    /* 4 = stdin, stdout, stderr, mother_desc.  Windows might keep sockets and 
     * files separate, in which case this isn't necessary, but we will err on 
     * the side of caution. */
    if ((wsaData.iMaxSockets - 4) < max_players) {
      max_players = wsaData.iMaxSockets - 4;
    }
    log("Max players set to %d", max_players);

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
      log("SYSERR: Error opening network connection: Winsock error #%d",
	  WSAGetLastError());
      exit(1);
    }
  }
#else
  /* Should the first argument to socket() be AF_INET or PF_INET?  I don't
   * know, take your pick.  PF_INET seems to be more widely adopted, and
   * Comer (_Internetworking with TCP/IP_) even makes a point to say that
   * people erroneously use AF_INET with socket() when they should be using
   * PF_INET.  However, the man pages of some systems indicate that AF_INET
   * is correct; some such as ConvexOS even say that you can use either one.
   * All implementations I've seen define AF_INET and PF_INET to be the same
   * number anyway, so the point is (hopefully) moot. */

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("SYSERR: Error creating socket");
    exit(1);
  }
#endif				/* CIRCLE_WINDOWS */

#if defined(SO_REUSEADDR) && !defined(CIRCLE_MACINTOSH)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0){
    perror("SYSERR: setsockopt REUSEADDR");
    exit(1);
  }
#endif

  set_sendbuf(s);

/* The GUSI sockets library is derived from BSD, so it defines SO_LINGER, even 
 * though setsockopt() is unimplimented. (from Dean Takemori) */
#if defined(SO_LINGER) && !defined(CIRCLE_MACINTOSH)
  {
    struct linger ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0)
      perror("SYSERR: setsockopt SO_LINGER");	/* Not fatal I suppose. */
  }
#endif

  /* Clear the structure */
  memset((char *)&sa, 0, sizeof(sa));

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr = *(get_bind_addr());

  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("SYSERR: bind");
    CLOSE_SOCKET(s);
    exit(1);
  }
  nonblock(s);
  listen(s, 5);
  return (s);
}

int get_max_players(void)
{
#ifndef CIRCLE_UNIX
  return (CONFIG_MAX_PLAYING);
#else

  int max_descs = 0;
  const char *method;

/* First, we'll try using getrlimit/setrlimit.  This will probably work
 * on most systems.  HAS_RLIMIT is defined in sysdep.h. */
#ifdef HAS_RLIMIT
  {
    struct rlimit limit;

    /* find the limit of file descs */
    method = "rlimit";
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("SYSERR: calling getrlimit");
      exit(1);
    }

    /* set the current to the maximum */
    limit.rlim_cur = limit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("SYSERR: calling setrlimit");
      exit(1);
    }
#ifdef RLIM_INFINITY
    if (limit.rlim_max == RLIM_INFINITY)
      max_descs = CONFIG_MAX_PLAYING + NUM_RESERVED_DESCS;
    else
      max_descs = MIN(CONFIG_MAX_PLAYING + NUM_RESERVED_DESCS, limit.rlim_max);
#else
    max_descs = MIN(CONFIG_MAX_PLAYING + NUM_RESERVED_DESCS, limit.rlim_max);
#endif
  }

#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
  method = "OPEN_MAX";
  max_descs = OPEN_MAX;		/* Uh oh.. rlimit didn't work, but we have
				 * OPEN_MAX */
#elif defined (_SC_OPEN_MAX)
  /* Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
   * try the POSIX sysconf() function.  (See Stevens' _Advanced Programming
   * in the UNIX Environment_). */
  method = "POSIX sysconf";
  errno = 0;
  if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
    if (errno == 0)
      max_descs = CONFIG_MAX_PLAYING + NUM_RESERVED_DESCS;
    else {
      perror("SYSERR: Error calling sysconf");
      exit(1);
    }
  }
#else
  /* if everything has failed, we'll just take a guess */
  method = "random guess";
  max_descs = CONFIG_MAX_PLAYING + NUM_RESERVED_DESCS;
#endif

  /* now calculate max _players_ based on max descs */
  max_descs = MIN(CONFIG_MAX_PLAYING, max_descs - NUM_RESERVED_DESCS);

  if (max_descs <= 0) {
    log("SYSERR: Non-positive max player limit!  (Set at %d using %s).",
	    max_descs, method);
    exit(1);
  }
  log("   Setting player limit to %d using %s.", max_descs, method);
  return (max_descs);
#endif /* CIRCLE_UNIX */
}

/* game_loop contains the main loop which drives the entire MUD.  It
 * cycles once every 0.10 seconds and is responsible for accepting new
 * new connections, polling existing connections for input, dequeueing
 * output and sending it out to players, and calling "heartbeat" functions
 * such as mobile_activity(). */
void game_loop(socket_t mother_desc)
{
  fd_set input_set, output_set, exc_set, null_set;
  struct timeval last_time, opt_time, process_time, temp_time;
  struct timeval before_sleep, now, timeout;
  char comm[MAX_INPUT_LENGTH];
  struct descriptor_data *d, *next_d;
  int missed_pulses, maxdesc, aliased;

  /* initialize various time values */
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  FD_ZERO(&null_set);

  gettimeofday(&last_time, (struct timezone *) 0);

  /* The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho.  The.. */
  while (!circle_shutdown) {

    /* Sleep if we don't have any connections */
    if (descriptor_list == NULL) {
      log("No connections.  Going to sleep.");
      FD_ZERO(&input_set);
      FD_SET(mother_desc, &input_set);
      if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
	if (errno == EINTR)
	  log("Waking up to process signal.");
	else
	  perror("SYSERR: Select coma");
      } else
	log("New connection.  Waking up.");
      gettimeofday(&last_time, (struct timezone *) 0);
    }
    /* Set up the input, output, and exception sets for select(). */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(mother_desc, &input_set);

    maxdesc = mother_desc;
    for (d = descriptor_list; d; d = d->next) {
#ifndef CIRCLE_WINDOWS
      if (d->descriptor > maxdesc)
	maxdesc = d->descriptor;
#endif
      FD_SET(d->descriptor, &input_set);
      FD_SET(d->descriptor, &output_set);
      FD_SET(d->descriptor, &exc_set);
    }

    /* At this point, we have completed all input, output and heartbeat
     * activity from the previous iteration, so we have to put ourselves
     * to sleep until the next 0.1 second tick.  The first step is to
     * calculate how long we took processing the previous iteration. */

    gettimeofday(&before_sleep, (struct timezone *) 0); /* current time */
    timediff(&process_time, &before_sleep, &last_time);

    /* If we were asleep for more than one pass, count missed pulses and sleep
     * until we're resynchronized with the next upcoming pulse. */
    if (process_time.tv_sec == 0 && process_time.tv_usec < OPT_USEC) {
      missed_pulses = 0;
    } else {
      missed_pulses = process_time.tv_sec * PASSES_PER_SEC;
      missed_pulses += process_time.tv_usec / OPT_USEC;
      process_time.tv_sec = 0;
      process_time.tv_usec = process_time.tv_usec % OPT_USEC;
    }

    /* Calculate the time we should wake up */
    timediff(&temp_time, &opt_time, &process_time);
    timeadd(&last_time, &before_sleep, &temp_time);

    /* Now keep sleeping until that time has come */
    gettimeofday(&now, (struct timezone *) 0);
    timediff(&timeout, &last_time, &now);

    /* Go to sleep */
    do {
      circle_sleep(&timeout);
      gettimeofday(&now, (struct timezone *) 0);
      timediff(&timeout, &last_time, &now);
    } while (timeout.tv_usec || timeout.tv_sec);

    /* Poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("SYSERR: Select poll");
      return;
    }
    /* If there are new connections waiting, accept them. */
    if (FD_ISSET(mother_desc, &input_set))
      new_descriptor(mother_desc);

    /* Kick out the freaky folks in the exception set and marked for close */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &exc_set)) {
	FD_CLR(d->descriptor, &input_set);
	FD_CLR(d->descriptor, &output_set);
	close_socket(d);
      }
    }

    /* Process descriptors with input pending */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &input_set))
	if (process_input(d) < 0)
	  close_socket(d);
    }

    /* Process commands we just read from process_input */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;

      /* Not combined to retain --(d->wait) behavior. -gg 2/20/98 If no wait 
       * state, no subtraction.  If there is a wait state then 1 is subtracted.
       * Therefore we don't go less than 0 ever and don't require an 'if' 
       * bracket. -gg 2/27/99 */
      if (d->character) {
        GET_WAIT_STATE(d->character) -= (GET_WAIT_STATE(d->character) > 0);
        
        if (GET_DEFEND(d->character) > 0)
           GET_DEFEND(d->character) -= 1;

        if (GET_WAIT_STATE(d->character))
          continue;
      }

      if (!get_from_q(&d->input, comm, &aliased))
        continue;

      if (d->character) {
	/* Reset the idle timer & pull char back from void if necessary */
	d->character->char_specials.timer = 0;
	if (STATE(d) == CON_PLAYING && GET_WAS_IN(d->character) != NOWHERE) {
	  if (IN_ROOM(d->character) != NOWHERE)
	    char_from_room(d->character);
	  char_to_room(d->character, GET_WAS_IN(d->character));
	  GET_WAS_IN(d->character) = NOWHERE;
	  act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
	}
        GET_WAIT_STATE(d->character) = 1;
      }
      d->has_prompt = FALSE;

      if (d->showstr_count) /* Reading something w/ pager */
	show_string(d, comm);
      else if (d->str)		/* Writing boards, mail, etc. */
	string_add(d, comm);
      else if (STATE(d) != CON_PLAYING) /* In menus, etc. */
	nanny(d, comm);
      else {			/* else: we're playing normally. */
	if (aliased)		/* To prevent recursive aliases. */
	  d->has_prompt = TRUE;	/* To get newline before next cmd output. */
	else if (perform_alias(d, comm, sizeof(comm)))    /* Run it through aliasing system */
	  get_from_q(&d->input, comm, &aliased);
	command_interpreter(d->character, comm); /* Send it to interpreter */
      }
    }

    /* Send queued output out to the operating system (ultimately to user). */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (*(d->output) && FD_ISSET(d->descriptor, &output_set)) {
	/* Output for this player is ready */
	if (process_output(d) < 0)
	  close_socket(d);
	else
	  d->has_prompt = 1;
      }
    }

    /* Print prompts for other descriptors who had no other output */
    for (d = descriptor_list; d; d = d->next) {
      if (!d->has_prompt) {
	write_to_descriptor(d->descriptor, make_prompt(d));
	d->has_prompt = TRUE;
      }
    }

    /* Kick out folks in the CON_CLOSE or CON_DISCONNECT state */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (STATE(d) == CON_CLOSE || STATE(d) == CON_DISCONNECT)
	close_socket(d);
    }

    /* Now, we execute as many pulses as necessary--just one if we haven't
     * missed any pulses, or make up for lost time if we missed a few
     * pulses by sleeping for too long. */
    missed_pulses++;

    if (missed_pulses <= 0) {
      log("SYSERR: **BAD** MISSED_PULSES NONPOSITIVE (%d), TIME GOING BACKWARDS!!", missed_pulses);
      missed_pulses = 1;
    }

    /* If we missed more than 30 seconds worth of pulses, just do 30 secs */
    if (missed_pulses > 30 RL_SEC) {
      log("SYSERR: Missed %d seconds worth of pulses.", missed_pulses / PASSES_PER_SEC);
      missed_pulses = 30 RL_SEC;
    }

    /* Now execute the heartbeat functions */
    while (missed_pulses--)
      heartbeat(++pulse);

    /* Check for any signals we may have received. */
    if (reread_wizlist) {
      reread_wizlist = FALSE;
      mudlog(CMP, LVL_IMMORT, TRUE, "Signal received - rereading wizlists.");
      reboot_wizlists();
    }
    if (emergency_unban) {
      emergency_unban = FALSE;
      mudlog(BRF, LVL_IMMORT, TRUE, "Received SIGUSR2 - completely unrestricting game (emergent)");
      ban_list = NULL;
      circle_restrict = 0;
      num_invalid = 0;
    }
    if (webster_file_ready) {
      webster_file_ready = FALSE;
      handle_webster_file();
    }

#ifdef CIRCLE_UNIX
    /* Update tics_passed for deadlock protection (UNIX only) */
    tics_passed++;
#endif
  }
}

void heartbeat(int heart_pulse)
{
  static int mins_since_crashsave = 0;

  event_process();

  if (!(heart_pulse % PULSE_DG_SCRIPT))
    script_trigger_check();

  if (!(heart_pulse % PULSE_ZONE))
    zone_update();

  if (!(heart_pulse % PULSE_IDLEPWD))		/* 15 seconds */
    check_idle_passwords();

  if (!(heart_pulse % PULSE_MOBILE))
    mobile_activity();

  if (!(heart_pulse % PULSE_BLOOD))
    lose_blood();

//  if (!(heart_pulse % PULSE_VIOLENCE))
//    perform_violence();

//  if (pulse % 260 == 0) 

  if (!(heart_pulse % (150 * PASSES_PER_SEC))) {
    weather_and_time(1);
    check_time_triggers();
    affect_update();
    point_update();
    noncombat_intel();
//    room_affect_update();

  }

  if (!(heart_pulse % (25 * PASSES_PER_SEC)))
  {
    check_fishing();
    check_mining();
    check_lumber();
    build_works_process();
  }

//  if (!(heart_pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC)))
//    check_timed_quests();

  if (!(heart_pulse % (10 * PASSES_PER_SEC)))
    fishing_message();

  if (!(heart_pulse % (10 * PASSES_PER_SEC))) {
    death_tick();
    room_affect_update();
  }
        

  if (!(heart_pulse % (5 * PASSES_PER_SEC)))
     cycle_crygate_msgs();

  if (!(heart_pulse % PULSE_MOBILE/2))
   regen_update();

  if (!(heart_pulse % PULSE_MOBILE))
  {
    mobile_activity();
    regen_update();
  }

  if (!(heart_pulse % PULSE_AUCTION))
    check_auction();

  if (!(heart_pulse % PULSE_HOURLY))
    draft_timeout();  /* Check timeout on draft mails */

  if (!(heart_pulse % PULSE_VIOLENCE))
  {
    perform_violence();
//    warzone_update();
  }

/*  if (in_arena == ARENA_START)
    if(!(pulse % PULSE_ARENA))
        start_arena();
  if (in_arena == ARENA_RUNNING)
    if(!(pulse % PULSE_ARENA))
        do_game();
*/

  if (!(pulse % 40 == 0))
    SpecialMessageUpdate();

/*  if (!(pulse % (30 * PASSES_PER_SEC)))
     turn_whointo_html();
*/
  if (CONFIG_AUTO_SAVE && !(heart_pulse % PULSE_AUTOSAVE)) {	/* 1 minute */
    if (++mins_since_crashsave >= CONFIG_AUTOSAVE_TIME) {
      mins_since_crashsave = 0;
      Crash_save_all();
      House_save_all();
    }
  }

  if (!(heart_pulse % PULSE_USAGE))
    record_usage();

  if (!(heart_pulse % PULSE_TIMESAVE))
  save_mud_time(&time_info);

  /* Every pulse! Don't want them to stink the place up... */
  extract_pending_chars();
}

/* new code to calculate time differences, which works on systems for which 
 * tv_usec is unsigned (and thus comparisons for something being < 0 fail).  
 * Based on code submitted by ss@sirocco.cup.hp.com. Code to return the time 
 * difference between a and b (a-b). Always returns a nonnegative value 
 * (floors at 0). */
void timediff(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
  if (a->tv_sec < b->tv_sec)
    *rslt = null_time;
  else if (a->tv_sec == b->tv_sec) {
    if (a->tv_usec < b->tv_usec)
      *rslt = null_time;
    else {
      rslt->tv_sec = 0;
      rslt->tv_usec = a->tv_usec - b->tv_usec;
    }
  } else {			/* a->tv_sec > b->tv_sec */
    rslt->tv_sec = a->tv_sec - b->tv_sec;
    if (a->tv_usec < b->tv_usec) {
      rslt->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
      rslt->tv_sec--;
    } else
      rslt->tv_usec = a->tv_usec - b->tv_usec;
  }
}

/* Add 2 time values.  Patch sent by "d. hall" to fix 'static' usage. */
void timeadd(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
  rslt->tv_sec = a->tv_sec + b->tv_sec;
  rslt->tv_usec = a->tv_usec + b->tv_usec;

  while (rslt->tv_usec >= 1000000) {
    rslt->tv_usec -= 1000000;
    rslt->tv_sec++;
  }
}

void record_usage(void)
{
  int sockets_connected = 0, sockets_playing = 0;
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next) {
    sockets_connected++;
    if (IS_PLAYING(d))
      sockets_playing++;
  }

  log("nusage: %-3d sockets connected, %-3d sockets playing",
	  sockets_connected, sockets_playing);

#ifdef RUSAGE	/* Not RUSAGE_SELF because it doesn't guarantee prototype. */
  {
    struct rusage ru;

    getrusage(RUSAGE_SELF, &ru);
    log("rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
	    ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
  }
#endif
}

/* Turn off echoing (specific to telnet client) */
void echo_off(struct descriptor_data *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_ECHO,
    (char) 0,
  };

  write_to_output(d, "%s", off_string);
}

/* Turn on echoing (specific to telnet client) */
void echo_on(struct descriptor_data *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_ECHO,
    (char) 0
  };

  write_to_output(d, "%s", on_string);
}

#define COLOR_ON(ch) (!IS_NPC(ch) ? (PRF_FLAGGED((ch), PRF_COLOR_1) || PRF_FLAGGED((ch), PRF_COLOR_2) ? 1 : 0) : 0)

/* Color replacement arrays. Renx -- 011100 */
#define A "\x1B["
const char *ANSI[] = { "&", A"0m",A"0m",A"0;30m",A"0;34m",A"0;32m",A"0;36m",A"0;31m",
     A"0;35m",A"0;33m",A"0;37m",A"1;30m",A"1;34m",A"1;32m",A"1;36m",A"1;31m",
     A"1;35m",A"1;33m",A"1;37m",A"40m",A"44m",A"42m",A"46m",A"41m",A"45m",
     A"43m",A"47m",A"5m",A"4m",A"1m",A"7m"
     ,"!"};
#undef A
const char CCODE[] = "&nNdbgcrmywDBGCRMYW01234567luoe!";

size_t proc_colors(char *txt, size_t maxlen, int parse)
{
  char *d, *s, *c, *p;
  int i;

  if (!txt || !strchr(txt, '&')) /* skip out if no color codes     */
    return strlen(txt);

  s = txt;
  CREATE(d, char, maxlen);
  p = d;

  for( ; *s && (d-p < maxlen); ) {
    /* no color code - just copy */
    if (*s != '&') {
      *d++ = *s++;
      continue;
    }

    /* if we get here we have a color code */
    s++; /* s now points to the code */


    if (!parse) { /* not parsing, just skip the code, unless it's @@ */
      if (*s == '&') {
        *d++ = '&';
      }
      s++; /* skip to next (non-colorcode) char */
      continue;
    }

    /* parse the color code */
    for (i = 0; CCODE[i] != '!'; i++) { /* do we find it ? */
      if ((*s) == CCODE[i]) {           /* if so :*/

        /* c now points to the first char in color code*/
        for(c = (char *)ANSI[i] ; *c && (d-p < maxlen); )
          *d++ = *c++;

        break;
      }
    }
   /* If we couldn't find any correct color code let's just skip it - Welcor */
    s++;

  } /* for loop */

  /* make sure txt is NULL - terminated */
  d = '\0';
  strncpy(txt, p, maxlen-1);

  free(p);

  return strlen(txt);
}

/* NEW PROMPT START */

char *make_prompt(struct descriptor_data *d)               
{
  static char prompt[MAX_PROMPT_LENGTH];
  extern char *DisplayPrompt(struct char_data *);


  if (d->showstr_count)
    snprintf(prompt, sizeof(prompt),
            "[ Return to continue, (q)uit, (r)efresh, (b)ack, or page number (%d/%d) ]",
            d->showstr_page, d->showstr_count);

  else if (d->str)                  
    strcpy(prompt, "] ");  
  else if (STATE(d) == CON_PLAYING && !IS_NPC(d->character)) {
    *prompt = '\0';

    if (!IS_NPC(d->character)) {             
      if (GET_INVIS_LEV(d->character))
        sprintf(prompt, "&G[&gi %ld&G]&n ", GET_INVIS_LEV(d->character));
     
      if (PLR_FLAGGED(d->character, PLR_INCRYGATE))
        sprintf(prompt, "%s&WIn Crystal Gate Travel System&n ", prompt);

//      if (has_mail(GET_IDNUM(d->character)))
//        sprintf(prompt, "%s&R[MAIL] ", prompt);

      if (GET_NEWS(d->character) < top_news_version)
        sprintf(prompt, "%s&Y[&yNEWS&Y]&n ", prompt);

      if (GET_GNEWS(d->character) < top_gnews_version && GET_LEVEL(d->character) > LVL_IMM)
        sprintf(prompt, "%s&M[&mGNEWS&M]&n ", prompt);

      if (PRF_FLAGGED(d->character, PRF_AFK))
        sprintf(prompt, "%s&C[AFK]&n ", prompt);
  
      if (PLR_FLAGGED(d->character, PLR_TAGGED))
        sprintf(prompt, "%s&Y(IT)&n ", prompt);
  
      if (PLR_FLAGGED(d->character, PLR_OUTLAW))
        sprintf(prompt, "%s&W(&ROUTLAW&W)&n ", prompt);

     if (PRF_FLAGGED(d->character, PRF_ROOMFLAGS))
        sprintf(prompt, "%s&Y[&y%5d&Y]&n ", prompt, GET_ROOM_VNUM(IN_ROOM(d->character)));

     if (PLR_FLAGGED(d->character, PLR_PARTYIN))
        sprintf(prompt, "%s&c<&CIn the Mist&c>&n ", prompt); 

    }

      strcat(prompt, DisplayPrompt(d->character));
      strcat(prompt, " ");            
      proc_color(prompt, (clr(d->character, C_NRM)));
  } else
    *prompt = '\0';

  if (d->character && PLR_FLAGGED(d->character, PLR_IACGA))
    strcat(prompt, "\377\371");       

  return (prompt);
}



/* NEW PROMPT END   */

/*
char *make_prompt(struct descriptor_data *d)
{
  static char prompt[MAX_PROMPT_LENGTH];

  char *hcol, *mcol, *mvcol;


  if (d->showstr_count)
    snprintf(prompt, sizeof(prompt),
            "\r\n[ Return to continue, (q)uit, (r)efresh, (b)ack, or page number (%d/%d) ]",
            d->showstr_page, d->showstr_count);
  else if (d->str)
    strcpy(prompt, "] ");       
  else if (STATE(d) == CON_PLAYING && !IS_NPC(d->character)) {
    int count;
    size_t len = 0;

    *prompt = '\0';

    if (GET_INVIS_LEV(d->character) && len < sizeof(prompt)) {
      count = snprintf(prompt + len, sizeof(prompt) - len, "i%ld ", GET_INVIS_LEV(d->character));
      if (count >= 0)
        len += count;
    }

      struct char_data *ch = d->character;

        if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_INCRYGATE))
        {
         snprintf(prompt, sizeof(prompt), "%sIn Crystal Gate Travel System%s ", BWHT, CNRM);
         return(prompt);
        }


        if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_PARTYIN))
        {
          snprintf(prompt, sizeof(prompt), "%s<%sIn the Mist%s>%s ", cyn, BCYN, cyn, CNRM );
          return(prompt);
        }

        if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS)){
      count = snprintf(prompt + len, sizeof(prompt) - len, "%s[%5d] ",BYEL,
                       GET_ROOM_VNUM(IN_ROOM(ch)));
          len += count;
        }

        if (PLR_FLAGGED(ch, PLR_OUTLAW)){
          count = snprintf(prompt + len, sizeof(prompt) - len, "%s(%sOutlaw%s) ", BWHT, BRED, BWHT);
          len += count;
       }

        if (PLR_FLAGGED(ch, PLR_TAGGED)){
          count = snprintf(prompt + len, sizeof(prompt) - len, "%s(IT) ", BYEL);
          len += count;
        }

        if (PRF_FLAGGED(ch, PRF_AFK)){
          count = snprintf(prompt + len, sizeof(prompt) - len, "%s[AFK] ", BCYN);
          len += count;
        }

        if (GET_NEWS(ch) < top_news_version ){
          count = snprintf(prompt + len, sizeof(prompt) - len, "%s[NEWS] ", BYEL);
          len += count;
        }
    
        if (GET_GNEWS(ch) < top_gnews_version && GET_LEVEL(ch) > LVL_IMM) {
         count = snprintf(prompt + len, sizeof(prompt) - len, "%s[%sGNEWS%s]%s ", BMAG, KMAG, BMAG, CNRM);
         len += count;
        }

        if (GET_HIT(ch) == GET_MAX_HIT(ch))
            hcol = BBLU;
        else if (GET_HIT(ch) < ((GET_MAX_HIT(ch)/4)))
            hcol = BRED;
        else if (GET_HIT(ch) < ((GET_MAX_HIT(ch)/4)*2))
            hcol = BYEL;
        else
            hcol = BGRN;

        if (GET_MANA(ch) == GET_MAX_MANA(ch))
            mcol = BBLU;
        else if (GET_MANA(ch) < ((GET_MAX_MANA(ch)/4)))
            mcol = BRED;
        else if (GET_MANA(ch) < ((GET_MAX_MANA(ch)/4)*2))
            mcol = BYEL;
        else
            mcol = BGRN;

        if (GET_MOVE(ch) == GET_MAX_MOVE(ch))
            mvcol = BBLU;
        else if (GET_MOVE(ch) < ((GET_MAX_MOVE(ch)/4)))
            mvcol = BRED;
        else if (GET_MOVE(ch) < ((GET_MAX_MOVE(ch)/4)*2))
            mvcol = BYEL;
        else
            mvcol = BGRN;
  

        count = snprintf(prompt + len, sizeof(prompt) - len, "%s< ", BBLU);
          len += count;

        count = snprintf(prompt + len, sizeof(prompt) - len, "%s", hcol);
          len += count;
        count = snprintf(prompt + len, sizeof(prompt) - len, "%ld%sH ", GET_HIT(ch), BBLU);
          len += count;

        count = snprintf(prompt + len, sizeof(prompt) - len, "%s", mcol);
          len += count;
        count = snprintf(prompt + len, sizeof(prompt) - len, "%ld%sM ", GET_MANA(ch), BBLU);
          len += count;


        count = snprintf(prompt + len, sizeof(prompt) - len, "%s", mvcol);
          len += count;

       if (RIDING(ch)) {
        count = snprintf(prompt + len, sizeof(prompt) - len, "%ld%sV ", GET_MOVE(RIDING(ch)), BBLU);
          len += count;
        } else {
        count = snprintf(prompt + len, sizeof(prompt) - len, "%ld%sV ", GET_MOVE(ch), BBLU);
          len += count;
        }

    if (PRF_FLAGGED(d->character, PRF_BUILDWALK) && len < sizeof(prompt)) {
      count = snprintf(prompt + len, sizeof(prompt) - len, "%sBUILDWALKING ", BYEL);
      if (count >= 0)
        len += count;
    }
//
//      if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NPK))
//      {
//        count = snprintf(prompt + len, sizeof(prompt) - len, "%sNPK%s", BYEL, BBLU);
//        if (count >= 0)
//          len += count;
//      }
//      else if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_CPK))
//      {
//        count = snprintf(prompt + len, sizeof(prompt) - len, "%sCPK%s", BRED, BBLU);
//        if (count >= 0)
//          len += count;
//      }
//      else
//      {
//        count = snprintf(prompt + len, sizeof(prompt) - len, "%sLPK%s", BGRN, BBLU);
//        if (count >= 0)
//          len += count;
//      }
//
    if (len < sizeof(prompt))
      strncat(prompt, "> ", sizeof(prompt) - len - 1);  
  } else if (STATE(d) == CON_PLAYING && IS_NPC(d->character))
    snprintf(prompt, sizeof(prompt), "%s%s>%s ", GET_NAME(d->character), BBLU,CNRM);
  else
    *prompt = '\0';

  strcat(prompt, CNRM);

  if (d->character && PLR_FLAGGED(d->character, PLR_IACGA))
    strcat(prompt, "\377\371");


  return (prompt);
}
*/



/* NOTE: 'txt' must be at most MAX_INPUT_LENGTH big. */
void write_to_q(const char *txt, struct txt_q *queue, int aliased)
{
  struct txt_block *newt;

  CREATE(newt, struct txt_block, 1);
  newt->text = strdup(txt);
  newt->aliased = aliased;

  /* queue empty? */
  if (!queue->head) {
    newt->next = NULL;
    queue->head = queue->tail = newt;
  } else {
    queue->tail->next = newt;
    queue->tail = newt;
    newt->next = NULL;
  }
}

void ch_printf(CHAR_DATA *ch, char *fmt, ...)
{
  char buf[MAX_STRING_LENGTH*2];
  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  send_to_char(ch, buf);
}


/* NOTE: 'dest' must be at least MAX_INPUT_LENGTH big. */
int get_from_q(struct txt_q *queue, char *dest, int *aliased)
{
  struct txt_block *tmp;

  /* queue empty? */
  if (!queue->head)
    return (0);

  strcpy(dest, queue->head->text);	/* strcpy: OK (mutual MAX_INPUT_LENGTH) */
  *aliased = queue->head->aliased;

  tmp = queue->head;
  queue->head = queue->head->next;
  free(tmp->text);
  free(tmp);

  return (1);
}

/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
  if (d->large_outbuf) {
    d->large_outbuf->next = bufpool;
    bufpool = d->large_outbuf;
  }
  while (d->input.head) {
    struct txt_block *tmp = d->input.head;
    d->input.head = d->input.head->next;
    free(tmp->text);
    free(tmp);
  }
}

/* Add a new string to a player's output queue. For outside use. */
size_t write_to_output(struct descriptor_data *t, const char *txt, ...)
{
  va_list args;
  size_t left;

  va_start(args, txt);
  left = vwrite_to_output(t, txt, args);
  va_end(args);

  return left;
}

/* Add a new string to a player's output queue. */
size_t vwrite_to_output(struct descriptor_data *t, const char *format, va_list args)
{
  const char *text_overflow = "\r\nOVERFLOW\r\n";
  static char txt[MAX_STRING_LENGTH];
  size_t wantsize;
  int size;

  /* if we're in the overflow state already, ignore this new output */
  if (t->bufspace == 0)
    return (0);

  wantsize = size = vsnprintf(txt, sizeof(txt), format, args);
  if (t->character)
    wantsize = size = proc_colors(txt, sizeof(txt), COLOR_ON(t->character));
  /* If exceeding the size of the buffer, truncate it for the overflow message */
  if (size < 0 || wantsize >= sizeof(txt)) {
    size = sizeof(txt) - 1;
    strcpy(txt + size - strlen(text_overflow), text_overflow);	/* strcpy: OK */
  }

  /* If the text is too big to fit into even a large buffer, truncate
   * the new text to make it fit.  (This will switch to the overflow
   * state automatically because t->bufspace will end up 0.) */
  if (size + t->bufptr + 1 > LARGE_BUFSIZE) {
    size = LARGE_BUFSIZE - t->bufptr - 1;
    txt[size] = '\0';
    buf_overflows++;
  }

  /* If we have enough space, just write to buffer and that's it! If the
   * text just barely fits, then it's switched to a large buffer instead. */
  if (t->bufspace > size) {
    strcpy(t->output + t->bufptr, txt);	/* strcpy: OK (size checked above) */
    t->bufspace -= size;
    t->bufptr += size;
    return (t->bufspace);
  }

  buf_switches++;

  /* if the pool has a buffer in it, grab it */
  if (bufpool != NULL) {
    t->large_outbuf = bufpool;
    bufpool = bufpool->next;
  } else {			/* else create a new one */
    CREATE(t->large_outbuf, struct txt_block, 1);
    CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
    buf_largecount++;
  }

  strcpy(t->large_outbuf->text, t->output);	/* strcpy: OK (size checked previously) */
  t->output = t->large_outbuf->text;	/* make big buffer primary */
  strcat(t->output, txt);	/* strcat: OK (size checked) */

  /* set the pointer for the next write */
  t->bufptr = strlen(t->output);

  /* calculate how much space is left in the buffer */
  t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;

  return (t->bufspace);
}

void free_bufpool(void)
{
  struct txt_block *tmp;

  while (bufpool) {
    tmp = bufpool->next;
    if (bufpool->text)
      free(bufpool->text);
    free(bufpool);
    bufpool = tmp;
  }
}

void send_to_arena(char *messg)
{
struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {
      if (d->descriptor && messg && PRF_FLAGGED(d->character, PRF2_ARENA))
//        SEND_TO_Q(messg, d);
        write_to_descriptor(d->descriptor, messg);
 }
}
                  
void send_to_wildsect(WILD_DATA *wd, const char *messg)
{
        DESCRIPTOR_DATA *i;

        if (!wd || !messg || !*messg)
                return;

        for (i = descriptor_list; i; i = i->next)
        {
                if (STATE(i) != CON_PLAYING || i->character == NULL)
                        continue;
                if (!AWAKE(i->character) || !IN_WILD(i->character))
                        continue;
                if (wd != get_wd(world[IN_ROOM(i->character)].coord))
                        continue;
                write_to_descriptor(i, messg);
        }
}

/*  socket handling */
/* get_bind_addr: Return a struct in_addr that should be used in our
 * call to bind().  If the user has specified a desired binding
 * address, we try to bind to it; otherwise, we bind to INADDR_ANY.
 * Note that inet_aton() is preferred over inet_addr() so we use it if
 * we can.  If neither is available, we always bind to INADDR_ANY. */
struct in_addr *get_bind_addr()
{
  static struct in_addr bind_addr;

  /* Clear the structure */
  memset((char *) &bind_addr, 0, sizeof(bind_addr));

  /* If DLFT_IP is unspecified, use INADDR_ANY */
  if (CONFIG_DFLT_IP == NULL) {
    bind_addr.s_addr = htonl(INADDR_ANY);
  } else {
    /* If the parsing fails, use INADDR_ANY */
    if (!parse_ip(CONFIG_DFLT_IP, &bind_addr)) {
      log("SYSERR: DFLT_IP of %s appears to be an invalid IP address",
          CONFIG_DFLT_IP);
      bind_addr.s_addr = htonl(INADDR_ANY);
    }
  }

  /* Put the address that we've finally decided on into the logs */
  if (bind_addr.s_addr == htonl(INADDR_ANY))
    log("Binding to all IP interfaces on this host.");
  else
    log("Binding only to IP address %s", inet_ntoa(bind_addr));

  return (&bind_addr);
}

#ifdef HAVE_INET_ATON
/* inet_aton's interface is the same as parse_ip's: 0 on failure, non-0 if
 * successful. */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
  return (inet_aton(addr, inaddr));
}

#elif HAVE_INET_ADDR

/* inet_addr has a different interface, so we emulate inet_aton's */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
  long ip;

  if ((ip = inet_addr(addr)) == -1) {
    return (0);
  } else {
    inaddr->s_addr = (unsigned long) ip;
    return (1);
  }
}

#else
/* If you have neither function - sorry, you can't do specific binding. */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
  log("SYSERR: warning: you're trying to set DFLT_IP but your system has no "
      "functions to parse IP addresses (how bizarre!)");
  return (0);
}
#endif /* INET_ATON and INET_ADDR */

/* Sets the kernel's send buffer size for the descriptor */
int set_sendbuf(socket_t s)
{
#if defined(SO_SNDBUF) && !defined(CIRCLE_MACINTOSH)
  int opt = MAX_SOCK_BUF;

  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt SNDBUF");
    return (-1);
  }
#endif

  return (0);
}

/* Initialize a descriptor */
void init_descriptor (struct descriptor_data *newd, int desc)
{
    static int last_desc = 0;	/* last descriptor number */

  newd->descriptor = desc;
  newd->idle_tics = 0;
  newd->output = newd->small_outbuf;
  newd->bufspace = SMALL_BUFSIZE - 1;
  newd->login_time = time(0);
  *newd->output = '\0';
  newd->bufptr = 0;
  newd->has_prompt = 1;  /* prompt is part of greetings */
  STATE(newd) = CON_GET_NAME;
  CREATE(newd->history, char *, HISTORY_SIZE);
  if (++last_desc == 1000)
    last_desc = 1;
  newd->desc_num = last_desc;
}

int new_descriptor(socket_t s)
{
  socket_t desc;
  int sockets_connected = 0;
  socklen_t i;
  struct descriptor_data *newd;
  struct sockaddr_in peer;
  struct hostent *from;

  /* accept the new connection */
  i = sizeof(peer);
  if ((desc = accept(s, (struct sockaddr *) &peer, &i)) == INVALID_SOCKET) {
    perror("SYSERR: accept");
    return (-1);
  }
  /* keep it from blocking */
  nonblock(desc);

  /* set the send buffer size */
  if (set_sendbuf(desc) < 0) {
    CLOSE_SOCKET(desc);
    return (0);
  }

  /* make sure we have room for it */
  for (newd = descriptor_list; newd; newd = newd->next)
    sockets_connected++;

  if (sockets_connected >= CONFIG_MAX_PLAYING) {
    write_to_descriptor(desc, "Sorry, the game is full right now... please try again later!\r\n");
    CLOSE_SOCKET(desc);
    return (0);
  }
  /* create a new descriptor */
  CREATE(newd, struct descriptor_data, 1);

  /* find the sitename */
  if (CONFIG_NS_IS_SLOW ||
      !(from = gethostbyaddr((char *) &peer.sin_addr,
		             sizeof(peer.sin_addr), AF_INET))) {

    /* resolution failed */
    if (!CONFIG_NS_IS_SLOW)
      perror("SYSERR: gethostbyaddr");

    /* find the numeric site address */
    strncpy(newd->host, (char *)inet_ntoa(peer.sin_addr), HOST_LENGTH);	/* strncpy: OK (n->host:HOST_LENGTH+1) */
    *(newd->host + HOST_LENGTH) = '\0';
  } else {
    strncpy(newd->host, from->h_name, HOST_LENGTH);	/* strncpy: OK (n->host:HOST_LENGTH+1) */
    *(newd->host + HOST_LENGTH) = '\0';
  }

  /* determine if the site is banned */
  if (isbanned(newd->host) == BAN_ALL) {
    CLOSE_SOCKET(desc);
    mudlog(CMP, LVL_GOD, TRUE, "Connection attempt denied from [%s]", newd->host);
    free(newd);
    return (0);
  }

  /* initialize descriptor data */
   init_descriptor(newd, desc);

  /* prepend to list */
  newd->next = descriptor_list;
  descriptor_list = newd;

char greet_copy[MAX_STRING_LENGTH];

  *greet_copy = '\0';  
  sprintf(greet_copy, "%s", GREETINGS);
  proc_colors(greet_copy, MAX_STRING_LENGTH, TRUE);
  write_to_output(newd, "%s", greet_copy);    
//  write_to_output(newd, "%s", GREETINGS);


  return (0);
}

/* Send all of the output that we've accumulated for a player out to the 
 * player's descriptor. 32 byte GARBAGE_SPACE in MAX_SOCK_BUF used for:
 *	 2 bytes: prepended \r\n
 *	14 bytes: overflow message
 *	 2 bytes: extra \r\n for non-comapct
 *      14 bytes: unused */
int process_output(struct descriptor_data *t)
{
  char i[MAX_SOCK_BUF], *osb = i + 2;
  int result;

  /* we may need this \r\n for later -- see below */
  strcpy(i, "\r\n");	/* strcpy: OK (for 'MAX_SOCK_BUF >= 3') */

  /* now, append the 'real' output */
  strcpy(osb, t->output);	/* strcpy: OK (t->output:LARGE_BUFSIZE < osb:MAX_SOCK_BUF-2) */

  /* if we're in the overflow state, notify the user */
  if (t->bufspace == 0)
    strcat(osb, "**OVERFLOW**\r\n");	/* strcpy: OK (osb:MAX_SOCK_BUF-2 reserves space) */

  /* add the extra CRLF if the person isn't in compact mode */
  if (STATE(t) == CON_PLAYING && t->character && !IS_NPC(t->character) && !PRF_FLAGGED(t->character, PRF_COMPACT))
    strcat(osb, "\r\n");	/* strcpy: OK (osb:MAX_SOCK_BUF-2 reserves space) */

  /* add a prompt */
  strcat(i, make_prompt(t));	/* strcpy: OK (i:MAX_SOCK_BUF reserves space) */

  /* now, send the output.  If this is an 'interruption', use the prepended
   * CRLF, otherwise send the straight output sans CRLF. */
  if (t->has_prompt) {
    t->has_prompt = FALSE;
    result = write_to_descriptor(t->descriptor, i);
    if (result >= 2)
      result -= 2;
  } else
    result = write_to_descriptor(t->descriptor, osb);

  if (result < 0) {	/* Oops, fatal error. Bye! */
    close_socket(t);
    return (-1);
  } else if (result == 0)	/* Socket buffer full. Try later. */
    return (0);

  /* Handle snooping: prepend "% " and send to snooper. */
  if (t->snoop_by)
    write_to_output(t->snoop_by, "%% %*s%%%%", result, t->output);

  /* The common case: all saved output was handed off to the kernel buffer. */
  if (result >= t->bufptr) {
    /* If we were using a large buffer, put the large buffer on the buffer pool
     * and switch back to the small one. */
    if (t->large_outbuf) {
      t->large_outbuf->next = bufpool;
      bufpool = t->large_outbuf;
      t->large_outbuf = NULL;
      t->output = t->small_outbuf;
    }
    /* reset total bufspace back to that of a small buffer */
    t->bufspace = SMALL_BUFSIZE - 1;
    t->bufptr = 0;
    *(t->output) = '\0';

    /* If the overflow message or prompt were partially written, try to save
     * them. There will be enough space for them if this is true.  'result'
     * is effectively unsigned here anyway. */
    if ((unsigned int)result < strlen(osb)) {
      size_t savetextlen = strlen(osb + result);

      strcat(t->output, osb + result);
      t->bufptr   -= savetextlen;
      t->bufspace += savetextlen;
    }

  } else {
    /* Not all data in buffer sent.  result < output buffersize. */
    strcpy(t->output, t->output + result);	/* strcpy: OK (overlap) */
    t->bufptr   -= result;
    t->bufspace += result;
  }

  return (result);
}

/* perform_socket_write: takes a descriptor, a pointer to text, and a
 * text length, and tries once to send that text to the OS.  This is
 * where we stuff all the platform-dependent stuff that used to be
 * ugly #ifdef's in write_to_descriptor(). This function must return:
 * -1  If a fatal error was encountered in writing to the descriptor.
 *  0  If a transient failure was encountered (e.g. socket buffer full).
 * >0  To indicate the number of bytes successfully written, possibly
 *     fewer than the number the caller requested be written.
 * Right now there are two versions of this function: one for Windows,
 * and one for all other platforms. */

#if defined(CIRCLE_WINDOWS)
ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
  ssize_t result;

  result = send(desc, txt, length, 0);

  if (result > 0) {
    /* Write was successful */
    return (result);
  }

  if (result == 0) {
    /* This should never happen! */
    log("SYSERR: Huh??  write() returned 0???  Please report this!");
    return (-1);
  }

  /* result < 0: An error was encountered. */

  /* Transient error? */
  if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR)
    return (0);

  /* Must be a fatal error. */
  return (-1);
}

#else

#if defined(CIRCLE_ACORN)
#define write	socketwrite
#endif

/* perform_socket_write for all Non-Windows platforms */
ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
  ssize_t result;

  result = write(desc, txt, length);

  if (result > 0) {
    /* Write was successful. */
    return (result);
  }

  if (result == 0) {
    /* This should never happen! */
    log("SYSERR: Huh??  write() returned 0???  Please report this!");
    return (-1);
  }

  /* result < 0, so an error was encountered - is it transient? Unfortunately, 
   * different systems use different constants to indicate this. */

#ifdef EAGAIN		/* POSIX */
  if (errno == EAGAIN)
    return (0);
#endif

#ifdef EWOULDBLOCK	/* BSD */
  if (errno == EWOULDBLOCK)
    return (0);
#endif

#ifdef EDEADLK		/* Macintosh */
  if (errno == EDEADLK)
    return (0);
#endif

  /* Looks like the error was fatal.  Too bad. */
  return (-1);
}
#endif /* CIRCLE_WINDOWS */

/* write_to_descriptor takes a descriptor, and text to write to the descriptor.
 * It keeps calling the system-level write() until all the text has been 
 * delivered to the OS, or until an error is encountered. Returns:
 * >=0  If all is well and good.
 *  -1  If an error was encountered, so that the player should be cut off. */
int write_to_descriptor(socket_t desc, const char *txt)
{
  ssize_t bytes_written;
  size_t total = strlen(txt), write_total = 0;

  while (total > 0) {
    bytes_written = perform_socket_write(desc, txt, total);

    if (bytes_written < 0) {
      /* Fatal error.  Disconnect the player. */
      perror("SYSERR: Write to socket");
      return (-1);
    } else if (bytes_written == 0) {
      /* Temporary failure -- socket buffer full. */
      return (write_total);
    } else {
      txt += bytes_written;
      total -= bytes_written;
      write_total += bytes_written;
    }
  }

  return (write_total);
}

/* Same information about perform_socket_write applies here. I like
 * standards, there are so many of them. -gg 6/30/98 */
ssize_t perform_socket_read(socket_t desc, char *read_point, size_t space_left)
{
  ssize_t ret;

#if defined(CIRCLE_ACORN)
  ret = recv(desc, read_point, space_left, MSG_DONTWAIT);
#elif defined(CIRCLE_WINDOWS)
  ret = recv(desc, read_point, space_left, 0);
#else
  ret = read(desc, read_point, space_left);
#endif

  /* Read was successful. */
  if (ret > 0)
    return (ret);

  /* read() returned 0, meaning we got an EOF. */
  if (ret == 0) {
    log("WARNING: EOF on socket read (connection broken by peer)");
    return (-1);
  }

  /* Read returned a value < 0: there was an error. */
#if defined(CIRCLE_WINDOWS)	/* Windows */
  if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR)
    return (0);
#else

#ifdef EINTR		/* Interrupted system call - various platforms */
  if (errno == EINTR)
    return (0);
#endif

#ifdef EAGAIN		/* POSIX */
  if (errno == EAGAIN)
    return (0);
#endif

#ifdef EWOULDBLOCK	/* BSD */
  if (errno == EWOULDBLOCK)
    return (0);
#endif /* EWOULDBLOCK */

#ifdef EDEADLK		/* Macintosh */
  if (errno == EDEADLK)
    return (0);
#endif

#ifdef ECONNRESET
  if (errno == ECONNRESET)
    return (-1);
#endif

#endif /* CIRCLE_WINDOWS */

  /* We don't know what happened, cut them off. This qualifies for
   * a SYSERR because we have no idea what happened at this point.*/
  perror("SYSERR: perform_socket_read: about to lose connection");
  return (-1);
}

/* ASSUMPTION: There will be no newlines in the raw input buffer when this
 * function is called.  We must maintain that before returning.
 *
 * Ever wonder why 'tmp' had '+8' on it?  The crusty old code could write
 * MAX_INPUT_LENGTH+1 bytes to 'tmp' if there was a '$' as the final character 
 * in the input buffer.  This would also cause 'space_left' to drop to -1, 
 * which wasn't very happy in an unsigned variable.  Argh. So to fix the 
 * above, 'tmp' lost the '+8' since it doesn't need it and the code has been 
 * changed to reserve space by accepting one less character. (Do you really 
 * need 256 characters on a line?) -gg 1/21/2000 */
int process_input(struct descriptor_data *t)
{
  int buf_length, failed_subst;
  ssize_t bytes_read;
  size_t space_left;
  char *ptr, *read_point, *write_point, *nl_pos = NULL;
  char tmp[MAX_INPUT_LENGTH];

  /* first, find the point where we left off reading data */
  buf_length = strlen(t->inbuf);
  read_point = t->inbuf + buf_length;
  space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

  do {
    if (space_left <= 0) {
      log("WARNING: process_input: about to close connection: input overflow");
      return (-1);
    }

    bytes_read = perform_socket_read(t->descriptor, read_point, space_left);

    if (bytes_read < 0)	/* Error, disconnect them. */
      return (-1);
    else if (bytes_read == 0)	/* Just blocking, no problems. */
      return (0);

    /* at this point, we know we got some data from the read */
    *(read_point + bytes_read) = '\0';	/* terminate the string */
    
    /* search for a newline in the data we just read */
    for (ptr = read_point; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;

    read_point += bytes_read;
    space_left -= bytes_read;

/* on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
 * causing the MUD to hang when it encounters input not terminated by a
 * newline.  This was causing hangs at the Password: prompt, for example.
 * I attempt to compensate by always returning after the _first_ read, instead
 * of looping forever until a read returns -1.  This simulates non-blocking
 * I/O because the result is we never call read unless we know from select()
 * that data is ready (process_input is only called if select indicates that
 * this descriptor is in the read set).  JE 2/23/95. */
#if !defined(POSIX_NONBLOCK_BROKEN)
  } while (nl_pos == NULL);
#else
  } while (0);

  if (nl_pos == NULL)
    return (0);
#endif /* POSIX_NONBLOCK_BROKEN */

  /* okay, at this point we have at least one newline in the string; now we
   * can copy the formatted data to a new array for further processing. */

  read_point = t->inbuf;

  while (nl_pos != NULL) {
    write_point = tmp;
    space_left = MAX_INPUT_LENGTH - 1;

    /* The '> 1' reserves room for a '$ => $$' expansion. */
    for (ptr = read_point; (space_left > 1) && (ptr < nl_pos); ptr++) {
      if (*ptr == '\b' || *ptr == 127) { /* handle backspacing or delete key */
	if (write_point > tmp) {
	  if (*(--write_point) == '$') {
	    write_point--;
	    space_left += 2;
	  } else
	    space_left++;
	}
      } else if (isascii(*ptr) && isprint(*ptr)) {
	if ((*(write_point++) = *ptr) == '$') {		/* copy one character */
	  *(write_point++) = '$';	/* if it's a $, double it */
	  space_left -= 2;
	} else
	  space_left--;
      }
    }

    *write_point = '\0';

    if ((space_left <= 0) && (ptr < nl_pos)) {
      char buffer[MAX_INPUT_LENGTH + 64];

      snprintf(buffer, sizeof(buffer), "Line too long.  Truncated to:\r\n%s\r\n", tmp);
      if (write_to_descriptor(t->descriptor, buffer) < 0)
	return (-1);
    }
    if (t->snoop_by)
      write_to_output(t->snoop_by, "%% %s\r\n", tmp);
    failed_subst = 0;

    if (*tmp == '!' && !(*(tmp + 1)))	/* Redo last command. */
      strcpy(tmp, t->last_input);	/* strcpy: OK (by mutual MAX_INPUT_LENGTH) */
    else if (*tmp == '!' && *(tmp + 1)) {
      char *commandln = (tmp + 1);
      int starting_pos = t->history_pos,
	  cnt = (t->history_pos == 0 ? HISTORY_SIZE - 1 : t->history_pos - 1);

      skip_spaces(&commandln);
      for (; cnt != starting_pos; cnt--) {
	if (t->history[cnt] && is_abbrev(commandln, t->history[cnt])) {
	  strcpy(tmp, t->history[cnt]);	/* strcpy: OK (by mutual MAX_INPUT_LENGTH) */
	  strcpy(t->last_input, tmp);	/* strcpy: OK (by mutual MAX_INPUT_LENGTH) */
          write_to_output(t, "%s\r\n", tmp);
	  break;
	}
        if (cnt == 0)	/* At top, loop to bottom. */
	  cnt = HISTORY_SIZE;
      }
    } else if (*tmp == '^') {
      if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
	strcpy(t->last_input, tmp);	/* strcpy: OK (by mutual MAX_INPUT_LENGTH) */
    } else if ((*tmp == '-') && (*(tmp+1) == '-') && !(*(tmp+2))) {
        write_to_output(t, "\r\n ");
        flush_queues(t);
    } else {
      strcpy(t->last_input, tmp);	/* strcpy: OK (by mutual MAX_INPUT_LENGTH) */
      if (t->history[t->history_pos])
	free(t->history[t->history_pos]);	/* Clear the old line. */
      t->history[t->history_pos] = strdup(tmp);	/* Save the new. */
      if (++t->history_pos >= HISTORY_SIZE)	/* Wrap to top. */
	t->history_pos = 0;
    }

    if (!failed_subst)
      write_to_q(tmp, &t->input, 0);

    /* find the end of this line */
    while (ISNEWL(*nl_pos))
      nl_pos++;

    /* see if there's another newline in the input buffer */
    read_point = ptr = nl_pos;
    for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
	nl_pos = ptr;
  }

  /* now move the rest of the buffer up to the beginning for the next pass */
  write_point = t->inbuf;
  while (*read_point)
    *(write_point++) = *(read_point++);
  *write_point = '\0';

  return (1);
}

/* Perform substitution for the '^..^' csh-esque syntax orig is the orig string, 
 * i.e. the one being modified.  subst contains the substition string, i.e. 
 * "^telm^tell" */
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
  char newsub[MAX_INPUT_LENGTH + 5];

  char *first, *second, *strpos;

  /* First is the position of the beginning of the first string (the one to be 
   * replaced. */
  first = subst + 1;

  /* now find the second '^' */
  if (!(second = strchr(first, '^'))) {
    write_to_output(t, "Invalid substitution.\r\n");
    return (1);
  }
  /* terminate "first" at the position of the '^' and make 'second' point
   * to the beginning of the second string */
  *(second++) = '\0';

  /* now, see if the contents of the first string appear in the original */
  if (!(strpos = strstr(orig, first))) {
    write_to_output(t, "Invalid substitution.\r\n");
    return (1);
  }
  /* now, we construct the new string for output. */

  /* first, everything in the original, up to the string to be replaced */
  strncpy(newsub, orig, strpos - orig);	/* strncpy: OK (newsub:MAX_INPUT_LENGTH+5 > orig:MAX_INPUT_LENGTH) */
  newsub[strpos - orig] = '\0';

  /* now, the replacement string */
  strncat(newsub, second, MAX_INPUT_LENGTH - strlen(newsub) - 1);	/* strncpy: OK */

  /* now, if there's anything left in the original after the string to
   * replaced, copy that too. */
  if (((strpos - orig) + strlen(first)) < strlen(orig))
    strncat(newsub, strpos + strlen(first), MAX_INPUT_LENGTH - strlen(newsub) - 1);	/* strncpy: OK */

  /* terminate the string in case of an overflow from strncat */
  newsub[MAX_INPUT_LENGTH - 1] = '\0';
  strcpy(subst, newsub);	/* strcpy: OK (by mutual MAX_INPUT_LENGTH) */

  return (0);
}

void close_socket(struct descriptor_data *d)
{
  struct descriptor_data *temp;

  REMOVE_FROM_LIST(d, descriptor_list, next);
  CLOSE_SOCKET(d->descriptor);
  flush_queues(d);

  /* Forget snooping */
  if (d->snooping)
    d->snooping->snoop_by = NULL;

  if (d->snoop_by) {
    write_to_output(d->snoop_by, "Your victim is no longer among us.\r\n");
    d->snoop_by->snooping = NULL;
  }

  if (d->character) {
    /* If we're switched, this resets the mobile taken. */
    d->character->desc = NULL;

    /* Plug memory leak, from Eric Green. */
    if (!IS_NPC(d->character) && PLR_FLAGGED(d->character, PLR_MAILING) && d->str) {
      if (*(d->str))
        free(*(d->str));
      free(d->str);
      d->str = NULL;
    } else if (d->backstr && !IS_NPC(d->character) && !PLR_FLAGGED(d->character, PLR_WRITING)) {
      free(d->backstr);      /* editing description ... not olc */
      d->backstr = NULL;
    }

    add_llog_entry(d->character, LAST_DISCONNECT);

    if (IS_PLAYING(d) || STATE(d) == CON_DISCONNECT) {
      struct char_data *link_challenged = d->original ? d->original : d->character;

      /* We are guaranteed to have a person. */
      act("$n has lost $s link.", TRUE, link_challenged, 0, 0, TO_ROOM);
      save_char(link_challenged);
      mudlog(NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(link_challenged)), TRUE, "Closing link to: %s.", GET_NAME(link_challenged));
    } else {
      mudlog(CMP, LVL_IMMORT, TRUE, "Losing player: %s.", GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
      free_char(d->character);
    }
  } else
    mudlog(CMP, LVL_IMMORT, TRUE, "Losing descriptor without char.");

  /* JE 2/22/95 -- part of my unending quest to make switch stable */
  if (d->original && d->original->desc)
    d->original->desc = NULL;

  /* Clear the command history. */
  if (d->history) {
    int cnt;
    for (cnt = 0; cnt < HISTORY_SIZE; cnt++)
      if (d->history[cnt])
	free(d->history[cnt]);
    free(d->history);
  }

  if (d->showstr_head)
    free(d->showstr_head);
  if (d->showstr_count)
    free(d->showstr_vector);

  /*. Kill any OLC stuff .*/
  switch (d->connected) {
    case CON_OEDIT:
    case CON_REDIT:
    case CON_ZEDIT:
    case CON_MEDIT:
    case CON_SEDIT:
    case CON_TEDIT:
    case CON_TRIGEDIT:
    case CON_AEDIT:
      cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
      break;
  }

  free(d);
}

void check_idle_passwords(void)
{
  struct descriptor_data *d, *next_d;

  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    if (STATE(d) != CON_PASSWORD && STATE(d) != CON_GET_NAME)
      continue;
    if (!d->idle_tics) {
      d->idle_tics++;
      continue;
    } else {
      echo_on(d);
      write_to_output(d, "\r\nTimed out... goodbye.\r\n");
      STATE(d) = CON_CLOSE;
    }
  }
}

/* I tried to universally convert Circle over to POSIX compliance, but
 * alas, some systems are still straggling behind and don't have all the
 * appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
 * O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
 * this and various other NeXT fixes.) */

#if defined(CIRCLE_WINDOWS)

void nonblock(socket_t s)
{
  unsigned long val = 1;
  ioctlsocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_AMIGA)

void nonblock(socket_t s)
{
  long val = 1;
  IoctlSocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_ACORN)

void nonblock(socket_t s)
{
  int val = 1;
  socket_ioctl(s, FIONBIO, &val);
}

#elif defined(CIRCLE_VMS)

void nonblock(socket_t s)
{
  int val = 1;

  if (ioctl(s, FIONBIO, &val) < 0) {
    perror("SYSERR: Fatal error executing nonblock (comm.c)");
    exit(1);
  }
}

#elif defined(CIRCLE_UNIX) || defined(CIRCLE_OS2) || defined(CIRCLE_MACINTOSH)

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void nonblock(socket_t s)
{
  int flags;

  flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, flags) < 0) {
    perror("SYSERR: Fatal error executing nonblock (comm.c)");
    exit(1);
  }
}
#endif  /* CIRCLE_UNIX || CIRCLE_OS2 || CIRCLE_MACINTOSH */


/*  signal-handling functions (formerly signals.c).  UNIX only. */
#if defined(CIRCLE_UNIX) || defined(CIRCLE_MACINTOSH)
RETSIGTYPE reread_wizlists(int sig)
{
  reread_wizlist = TRUE;
}

RETSIGTYPE unrestrict_game(int sig)
{
  emergency_unban = TRUE;
}

RETSIGTYPE websterlink(int sig)
{
  webster_file_ready = TRUE;
}

/*RETSIGTYPE SigTerm(int sig)
{

   mudlog(BRF, LVL_GOD, TRUE, "ERROR: RECIEVED TERM SIGNAL");
   send_to_all("&RATTENTION: Message from game server: Emergency shutdown called.\r\n&n");
   send_to_all("&YExecuting emergency save processes.\r\n");
   log("Server Message: SIGTERM signal and has terminted, running save functions");   
     

   
//   log("Server Message: Saving players...." );
//   save_all();
//   log("Server Message: Completed player saves....");
//   log("Server Message: Saving world and house files....");
//   House_save_all();
//   log("Server Message: Completed world and house saves....");

   log("Emergency shutdown complete.");
//   core_dump_real(__FILE__, __LINE__);

   exit(0);
}
*/
#ifdef CIRCLE_UNIX

/* clean up our zombie kids to avoid defunct processes */
RETSIGTYPE reap(int sig)
{
  while (waitpid(-1, NULL, WNOHANG) > 0);

  my_signal(SIGCHLD, reap);
}

/* Dying anyway... */
RETSIGTYPE checkpointing(int sig)
{
#ifndef MEMORY_DEBUG
  if (!tics_passed) {
    log("SYSERR: CHECKPOINT shutdown: tics not updated. (Infinite loop suspected)");
    abort();
  } else
    tics_passed = 0;
#endif
}

/* Dying anyway... */
RETSIGTYPE hupsig(int sig)
{
  log("SYSERR: Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
  exit(1); /* perhaps something more elegant should substituted */
}

#endif	/* CIRCLE_UNIX */

/* This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted for uniformity, because BSD systems
 * do not restart select(), even if SA_RESTART is used.
 * Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
 * I just define it to be the old signal.  If your system doesn't have
 * sigaction either, you can use the same fix.
 * SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric. */

#ifndef POSIX
#define my_signal(signo, func) signal(signo, func)
#else
sigfunc *my_signal(int signo, sigfunc *func)
{
  struct sigaction sact, oact;

  sact.sa_handler = func;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
#ifdef SA_INTERRUPT
  sact.sa_flags |= SA_INTERRUPT;	/* SunOS */
#endif

  if (sigaction(signo, &sact, &oact) < 0)
    return (SIG_ERR);

  return (oact.sa_handler);
}
#endif				/* POSIX */

void signal_setup(void)
{
#ifndef CIRCLE_MACINTOSH
  struct itimerval itime;
  struct timeval interval;

  /* user signal 1: reread wizlists.  Used by autowiz system. */
  my_signal(SIGUSR1, reread_wizlists);

  /* user signal 2: unrestrict game.  Used for emergencies if you lock
   * yourself out of the MUD somehow. */
  my_signal(SIGUSR2, websterlink);

  /* set up the deadlock-protection so that the MUD aborts itself if it gets
   * caught in an infinite loop for more than 3 minutes. */
  interval.tv_sec = 180;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, NULL);
  my_signal(SIGVTALRM, checkpointing);

  /* just to be on the safe side: */
  my_signal(SIGHUP, hupsig);
  my_signal(SIGCHLD, reap);
#endif /* CIRCLE_MACINTOSH */
  my_signal(SIGINT, hupsig);
  my_signal(SIGTERM, hupsig);
  my_signal(SIGPIPE, SIG_IGN);
  my_signal(SIGALRM, SIG_IGN);
//  my_signal(SIGSEGV, SigTerm);
}

#endif	/* CIRCLE_UNIX || CIRCLE_MACINTOSH */
/* Public routines for system-to-player-communication. */
size_t send_to_char(struct char_data *ch, const char *messg, ...)
{
  if (ch->desc && messg && *messg) {
    size_t left;
    va_list args;

    va_start(args, messg);
    left = vwrite_to_output(ch->desc, messg, args);
    va_end(args);
    return left;
  }
  return 0;
}

void send_to_all(const char *messg, ...)
{
  struct descriptor_data *i;
  va_list args;

  if (messg == NULL)
    return;

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING)
      continue;

    va_start(args, messg);
    vwrite_to_output(i, messg, args);
    va_end(args);
  }
}

void send_to_outdoor(const char *messg, ...)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next) {
    va_list args;

    if (STATE(i) != CON_PLAYING || i->character == NULL)
      continue;
    if (!AWAKE(i->character) || !OUTSIDE(i->character))
      continue;

    va_start(args, messg);
    vwrite_to_output(i, messg, args);
    va_end(args);
  }
}

void send_to_room(room_rnum room, const char *messg, ...)
{
  struct char_data *i;
  va_list args;

  if (messg == NULL)
    return;

  for (i = world[room].people; i; i = i->next_in_room) {
    if (!i->desc)
      continue;

    va_start(args, messg);
    vwrite_to_output(i->desc, messg, args);
    va_end(args);
  }
}

/* Thx to Jamie Nelson of 4D for this contribution */
void send_to_range(room_vnum start, room_vnum finish, const char *messg, ...)
{
  struct char_data *i;
  va_list args;
  int j;

  if (start > finish) {
    log("send_to_range passed start room value greater then finish.");
    return;
  }
  if (messg == NULL)
    return;

  for (j = 0; j < top_of_world; j++) {
    if (GET_ROOM_VNUM(j) >= start && GET_ROOM_VNUM(j) <= finish) {
      for (i = world[j].people; i; i = i->next_in_room) {
        if (!i->desc)
          continue;

        va_start(args, messg);
        vwrite_to_output(i->desc, messg, args);
        va_end(args);
      }
    }
  }
}

const char *ACTNULL = "<NULL>";
#define CHECK_NULL(pointer, expression) \
  if ((pointer) == NULL) i = ACTNULL; else i = (expression);
/* higher-level communication: the act() function */
void perform_act(const char *orig, struct char_data *ch, struct obj_data *obj,
		const void *vict_obj, const struct char_data *to)
{
  const char *i = NULL;
  char lbuf[MAX_STRING_LENGTH], *buf, *j;
  bool uppercasenext = FALSE;
  struct char_data *dg_victim = NULL;
  struct obj_data *dg_target = NULL;
  char *dg_arg = NULL;

  buf = lbuf;

  for (;;) {
    if (*orig == '$') {
      switch (*(++orig)) {
      case 'n':
	i = PERS(ch, to);
	break;
      case 'N':
	CHECK_NULL(vict_obj, PERS((const struct char_data *) vict_obj, to));
	dg_victim = (struct char_data *) vict_obj;
	break;
      case 'm':
	i = HMHR(ch);
	break;
      case 'M':
	CHECK_NULL(vict_obj, HMHR((const struct char_data *) vict_obj));
	dg_victim = (struct char_data *) vict_obj;
	break;
      case 's':
	i = HSHR(ch);
	break;
      case 'S':
	CHECK_NULL(vict_obj, HSHR((const struct char_data *) vict_obj));
	dg_victim = (struct char_data *) vict_obj;
	break;
      case 'e':
	i = HSSH(ch);
	break;
      case 'E':
	CHECK_NULL(vict_obj, HSSH((const struct char_data *) vict_obj));
	dg_victim = (struct char_data *) vict_obj;
	break;
      case 'o':
	CHECK_NULL(obj, OBJN(obj, to));
	break;
      case 'O':
	CHECK_NULL(vict_obj, OBJN((const struct obj_data *) vict_obj, to));
	dg_target = (struct obj_data *) vict_obj;
	break;
      case 'p':
	CHECK_NULL(obj, OBJS(obj, to));
	break;
      case 'P':
	CHECK_NULL(vict_obj, OBJS((const struct obj_data *) vict_obj, to));
	dg_target = (struct obj_data *) vict_obj;
	break;
      case 'a':
	CHECK_NULL(obj, SANA(obj));
	break;
      case 'A':
	CHECK_NULL(vict_obj, SANA((const struct obj_data *) vict_obj));
	dg_target = (struct obj_data *) vict_obj;
	break;
       case 'T':
 	CHECK_NULL(vict_obj, (const char *) vict_obj);
 	dg_arg = (char *) vict_obj;
	break;
      case 't':
 	CHECK_NULL(obj, (char *) obj);
	break;
      case 'F':
	CHECK_NULL(vict_obj, fname((const char *) vict_obj));
	break;
      /* uppercase previous word */
      case 'u':
        for (j=buf; j > lbuf && !isspace((int) *(j-1)); j--);
        if (j != buf)
          *j = UPPER(*j);
        i = "";
        break;
      /* uppercase next word */
      case 'U':
        uppercasenext = TRUE;
        i = "";
        break;
      /* Buildings */
      case 'b':
        CHECK_NULL(vict_obj, ((BUILDING_DATA *) vict_obj)->description);
        break;
      /* Vehicles */
      case 'v':
        CHECK_NULL( vict_obj, ((VEHICLE_DATA *) vict_obj)->short_description );
        break;
      case 'V':
        CHECK_NULL( vict_obj, ((VEHICLE_DATA *) vict_obj)->name );
        break;
      case '$':
	i = "$";
	break;
      default:
	log("SYSERR: Illegal $-code to act(): %c", *orig);
	log("SYSERR: %s", orig);
	i = "";
	break;
      }
      while ((*buf = *(i++)))
        {
        if (uppercasenext && !isspace((int) *buf))
          {
          *buf = UPPER(*buf);
          uppercasenext = FALSE;
          }
	buf++;
        }
      orig++;
    } else if (!(*(buf++) = *(orig++))) {
      break;
    } else if (uppercasenext && !isspace((int) *(buf-1))) {
      *(buf-1) = UPPER(*(buf-1));
      uppercasenext = FALSE;
    }
  }

  *(--buf) = '\r';
  *(++buf) = '\n';
  *(++buf) = '\0';

  if (to->desc)
    write_to_output(to->desc, "%s", CAP(lbuf));

  if ((IS_NPC(to) && dg_act_check) && (to != ch))
    act_mtrigger(to, lbuf, ch, dg_victim, obj, dg_target, dg_arg);

  if (last_act_message) 
    free(last_act_message); 
  last_act_message = strdup(lbuf); 
}

char *act(const char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, const void *vict_obj, int type)
{
  const struct char_data *to;
  int to_sleeping;

  if (!str || !*str)
    return NULL;

  /* Warning: the following TO_SLEEP code is a hack. I wanted to be able to tell
   * act to deliver a message regardless of sleep without adding an additional 
   * argument.  TO_SLEEP is 128 (a single bit high up).  It's ONLY legal to 
   * combine TO_SLEEP with one other TO_x command.  It's not legal to combine 
   * TO_x's with each other otherwise. TO_SLEEP only works because its value 
   * "happens to be" a single bit; do not change it to something else.  In 
   * short, it is a hack. */

  /* check if TO_SLEEP is there, and remove it if it is. */
  if ((to_sleeping = (type & TO_SLEEP)))
    type &= ~TO_SLEEP;

  /* this is a hack as well - DG_NO_TRIG is 256 -- Welcor */
  /* If the bit is set, unset dg_act_check, thus the ! below */
  if (!(dg_act_check = !IS_SET(type, DG_NO_TRIG))) 
    REMOVE_BIT(type, DG_NO_TRIG);

  if (type == TO_CHAR) {
    if (ch && SENDOK(ch)) { 
      perform_act(str, ch, obj, vict_obj, ch); 
      return last_act_message; 
    }
    return NULL;
  }

  if (type == TO_VICT) {
    if ((to = (const struct char_data *) vict_obj) != NULL && SENDOK(to)) { 
      perform_act(str, ch, obj, vict_obj, to); 
      return last_act_message; 
    }
    return NULL;
  }

#include "screen.h"
  if (type == TO_GMOTE && !IS_NPC(ch)) {
    struct descriptor_data *i;
    char buf[MAX_STRING_LENGTH];
    
    for (i = descriptor_list; i; i = i->next) {
      if (!i->connected && i->character &&
          !PRF_FLAGGED(i->character, PRF_NOGOSS) &&
          !PLR_FLAGGED(i->character, PLR_WRITING) &&
          !ROOM_FLAGGED(IN_ROOM(i->character), ROOM_SOUNDPROOF)) {

        sprintf(buf, "%s%s%s", CCYEL(i->character, C_NRM), str, CCNRM(i->character, C_NRM));
        perform_act(buf, ch, obj, vict_obj, i->character); 
      }
    }
    return last_act_message;
  }
  /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

  if (ch && IN_ROOM(ch) != NOWHERE)
    to = world[IN_ROOM(ch)].people;
  else if (obj && IN_ROOM(obj) != NOWHERE)
    to = world[IN_ROOM(obj)].people;
  else {
    log("SYSERR: no valid target to act()!");
    return NULL;
  }

  for (; to; to = to->next_in_room) {
    if (!SENDOK(to) || (to == ch))
      continue;
    if (hide_invisible && ch && !CAN_SEE(to, ch))
      continue;
    if (type != TO_ROOM && to == vict_obj)
      continue;
    perform_act(str, ch, obj, vict_obj, to); 
  }
  return last_act_message;
}

/* Prefer the file over the descriptor. */
void setup_log(const char *filename, int fd)
{
  FILE *s_fp;

#if defined(__MWERKS__) || defined(__GNUC__)
  s_fp = stderr;
#else
  if ((s_fp = fdopen(STDERR_FILENO, "w")) == NULL) {
    puts("SYSERR: Error opening stderr, trying stdout.");

    if ((s_fp = fdopen(STDOUT_FILENO, "w")) == NULL) {
      puts("SYSERR: Error opening stdout, trying a file.");

      /* If we don't have a file, try a default. */
      if (filename == NULL || *filename == '\0')
        filename = "log/syslog";
    }
  }
#endif

  if (filename == NULL || *filename == '\0') {
    /* No filename, set us up with the descriptor we just opened. */
    logfile = s_fp;
    puts("Using file descriptor for logging.");
    return;
  }

  /* We honor the default filename first. */
  if (open_logfile(filename, s_fp))
    return;

  /* Well, that failed but we want it logged to a file so try a default. */
  if (open_logfile("log/syslog", s_fp))
    return;

  /* Ok, one last shot at a file. */
  if (open_logfile("syslog", s_fp))
    return;

  /* Erp, that didn't work either, just die. */
  puts("SYSERR: Couldn't open anything to log to, giving up.");
  exit(1);
}

int open_logfile(const char *filename, FILE *stderr_fp)
{
  if (stderr_fp)	/* freopen() the descriptor. */
    logfile = freopen(filename, "w", stderr_fp);
  else
    logfile = fopen(filename, "w");

  if (logfile) {
    printf("Using log file '%s'%s.\n",
		filename, stderr_fp ? " with redirection" : "");
    return (TRUE);
  }

  printf("SYSERR: Error opening file '%s': %s\n", filename, strerror(errno));
  return (FALSE);
}

/* This may not be pretty but it keeps game_loop() neater than if it was inline. */
#if defined(CIRCLE_WINDOWS)
void circle_sleep(struct timeval *timeout)
{
  Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
}

#else
void circle_sleep(struct timeval *timeout)
{
  if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, timeout) < 0) {
    if (errno != EINTR) {
      perror("SYSERR: Select sleep");
      exit(1);
    }
  }
}

#endif /* CIRCLE_WINDOWS */

int validate_email(char *check_addr)
{
  char *host, mail_addr[MAX_EMAIL_LENGTH];

  if (strlen(check_addr) > MAX_EMAIL_LENGTH-1)
    return EMAIL_TOO_LONG;

  snprintf(mail_addr, MAX_EMAIL_LENGTH-1, "%s", check_addr);

  if (mail_addr == NULL || !(*mail_addr))
    return EMAIL_NO_ADDRESS;

  if (!(host = strchr(mail_addr, '@')))
    return EMAIL_NO_AT_SYMBOL;

   *host = '\0';
   host ++;

  if ((gethostbyname(host)) == NULL)
    return EMAIL_BANNED;

  if (strstr(host, "root"))
    return EMAIL_BANNED;

  return EMAIL_SUCCESS;
}

char *email_error_message(int error_code)
{
  static char error_text[MAX_INPUT_LENGTH];

  *error_text = '\0';

  switch (error_code) {
    case EMAIL_SUCCESS:      sprintf(error_text, "Success.  Valid email address.");
                             break;

    case EMAIL_TOO_LONG:     sprintf(error_text, "Invalid address. Too many characters (max: %d)", MAX_EMAIL_LENGTH);
                             break;

    case EMAIL_NO_ADDRESS:   sprintf(error_text, "Invalid address. Blank address supplied.");
                             break;

    case EMAIL_NO_AT_SYMBOL: sprintf(error_text, "Invalid address. An email address must have an 'at' symbol.");
                             break;

    case EMAIL_BAD_HOST:     sprintf(error_text, "Invalid address. The email host doesn't exist.");
                             break;

    case EMAIL_BANNED:       sprintf(error_text, "Invalid address. The email address is banned.");
                             break;

    case EMAIL_INVALID:      sprintf(error_text, "Invalid address.");
                             break;

    default:                 sprintf(error_text, "Invalid address. Unknown Reason.");
                             break;
  }

  return (error_text);
}


char *output_email(char *mail_addr)
{
  static char output[MAX_EMAIL_LENGTH+2];
  char *in_ptr, *out_ptr;

  in_ptr  = mail_addr;
  out_ptr = output;

  while (in_ptr && (out_ptr - output < MAX_EMAIL_LENGTH+1)) {
    if (*in_ptr == '@') *out_ptr++ = '@';
    *out_ptr++ = *in_ptr++;
  }
  *out_ptr = '\0';
  return output;
}


