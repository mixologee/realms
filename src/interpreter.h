/* ************************************************************************
*   File: interpreter.h                                                   *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
************************************************************************ */

#define ACMD(name)  \
   void name(struct char_data *ch, char *argument, int cmd, int subcmd)

ACMD(do_move);

#define CMD_NAME (complete_cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, complete_cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (complete_cmd_info[cmdnum].command_pointer == do_move)

void	command_interpreter(struct char_data *ch, char *argument);
int	search_block(char *arg, const char **list, int exact);
char	lower( char c );
char	*one_argument(char *argument, char *first_arg);
char	*one_word(char *argument, char *first_arg);
char	*any_one_arg(char *argument, char *first_arg);
char	*two_arguments(char *argument, char *first_arg, char *second_arg);
int	fill_word(char *argument);
void	half_chop(char *string, char *arg1, char *arg2);
void	nanny(struct descriptor_data *d, char *arg);
int	is_abbrev(const char *arg1, const char *arg2);
int	is_number(const char *str);
int	find_command(const char *command);
void	skip_spaces(char **string);
char	*delete_doubledollar(char *string);

/* Password Retrieval Email functions */
char    *delete_doubleat (char *string);
bool send_email(char *recipient, char *subj, char *bodytext);

int     top_news_version;
int     top_gnews_version;

/* WARNING: if you have added diagonal directions and have them at the
 * beginning of the command list.. change this value to 11 or 15 (depending) */
/* reserve these commands to come straight from the cmd list then start
 * sorting */
#define RESERVE_CMDS                7

/* for compatibility with 2.20: */
#define argument_interpreter(a, b, c) two_arguments(a, b, c)


struct command_info {
   const char *command;
   const char *sort_as;
   byte minimum_position;
   void	(*command_pointer)
	   (struct char_data *ch, char *argument, int cmd, int subcmd);
   sh_int minimum_level;
   int	subcmd;
};

/*
 * Necessary for CMD_IS macro.  Borland needs the structure defined first
 * so it has been moved down here.
 */
#ifndef __INTERPRETER_C__
extern struct command_info *complete_cmd_info;
#endif

/*
 * Alert! Changed from 'struct alias' to 'struct alias_data' in bpl15
 * because a Windows 95 compiler gives a warning about it having similiar
 * named member.
 */
struct alias_data {
  char *alias;
  char *replacement;
  int type;
  struct alias_data *next;
};

#define ALIAS_SIMPLE	0
#define ALIAS_COMPLEX	1

#define ALIAS_SEP_CHAR	';'
#define ALIAS_VAR_CHAR	'$'
#define ALIAS_GLOB_CHAR	'*'

/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH	NORTH
#define SCMD_EAST	EAST
#define SCMD_SOUTH	SOUTH
#define SCMD_WEST	WEST
#define SCMD_UP		UP
#define SCMD_DOWN	DOWN

/* do_gen_ps */
#define SCMD_INFO       0
#define SCMD_HANDBOOK   1 
#define SCMD_CREDITS    2
#define SCMD_NEWS       3
#define SCMD_WIZLIST    4
#define SCMD_POLICIES   5
#define SCMD_VERSION    6
#define SCMD_IMMLIST    7
#define SCMD_MOTD	8
#define SCMD_IMOTD	9
#define SCMD_CLEAR	10
#define SCMD_WHOAMI	11
#define SCMD_CHANGES    12
#define SCMD_GCHANGES   13

/* do_gen_tog */
#define SCMD_NOSUMMON   0
#define SCMD_NOHASSLE   1
#define SCMD_BRIEF      2
#define SCMD_COMPACT    3
#define SCMD_NOTELL	4
#define SCMD_NOAUCTION	5
#define SCMD_DEAF	6
#define SCMD_NOGOSSIP	7
#define SCMD_NOGRATZ	8
#define SCMD_NOWIZ	9
#define SCMD_QUEST	10
#define SCMD_ROOMFLAGS	11
#define SCMD_NOREPEAT	12
#define SCMD_HOLYLIGHT	13
#define SCMD_SLOWNS	14
#define SCMD_AUTOEXIT	15
#define SCMD_TRACK	16
#define SCMD_CLS        17
#define SCMD_BUILDWALK  18
#define SCMD_AFK        19
#define SCMD_OUTLAW     20
#define SCMD_XAP_OBJS   21
#define SCMD_CLANTALK   22
#define SCMD_ALLCTELL   23
#define SCMD_SCREENWIDTH 24
#define SCMD_AUTOMAP    25
#define SCMD_IACGA	26
#define SCMD_ARENA      27
#define SCMD_PRECEPTOR  28
#define SCMD_NOSHOUT    29
#define SCMD_NOHINT     30

/* do_wizutil */
#define SCMD_REROLL	0
#define SCMD_PARDON     1
#define SCMD_NOTITLE    2
#define SCMD_SQUELCH    3
#define SCMD_FREEZE	4
#define SCMD_THAW	5
#define SCMD_UNAFFECT	6
#define SCMD_MUTE       7

/* do_spec_com */
#define SCMD_WHISPER	0
#define SCMD_ASK	1

/* do_gen_com */
#define SCMD_HOLLER	0
#define SCMD_SHOUT	1
#define SCMD_GOSSIP	2
#define SCMD_AUCTION	3
#define SCMD_GRATZ	4
#define SCMD_GEMOTE     5

/* do_shutdown */
#define SCMD_SHUTDOW	0
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUI	0
#define SCMD_QUIT	1

/* do_date */
#define SCMD_DATE	0
#define SCMD_UPTIME	1

/* do_commands */
#define SCMD_COMMANDS	0
#define SCMD_SOCIALS	1
#define SCMD_WIZHELP	2

/* do_drop */
#define SCMD_DROP	0
#define SCMD_JUNK	1
#define SCMD_DONATE	2

/* do_gen_write */
#define SCMD_BUG	0
#define SCMD_TYPO	1
#define SCMD_IDEA	2

/* do_look */
#define SCMD_LOOK	0
#define SCMD_READ	1

/* do_qcomm */
#define SCMD_QSAY	0
#define SCMD_QECHO	1

/* do_pour */
#define SCMD_POUR	0
#define SCMD_FILL	1

/* do_poof */
#define SCMD_POOFIN	0
#define SCMD_POOFOUT	1

/* do_hit */
#define SCMD_HIT	0
#define SCMD_MURDER	1

/* do_eat */
#define SCMD_EAT	0
#define SCMD_TASTE	1
#define SCMD_DRINK	2
#define SCMD_SIP	3

/* do_use */
#define SCMD_USE	0
#define SCMD_QUAFF	1
#define SCMD_RECITE	2

/* do_echo */
#define SCMD_ECHO	0
#define SCMD_EMOTE	1

/* do_gen_door */
#define SCMD_OPEN       0
#define SCMD_CLOSE      1
#define SCMD_UNLOCK     2
#define SCMD_LOCK       3
#define SCMD_PICK       4

/* do_olc */
#define SCMD_OASIS_REDIT	0
#define SCMD_OASIS_OEDIT	1
#define SCMD_OASIS_ZEDIT	2
#define SCMD_OASIS_MEDIT	3
#define SCMD_OASIS_SEDIT	4
#define SCMD_OASIS_CEDIT	5
#define SCMD_OLC_SAVEINFO	7
#define SCMD_OASIS_RLIST 	8
#define SCMD_OASIS_MLIST	9
#define SCMD_OASIS_OLIST	10
#define SCMD_OASIS_SLIST	11
#define SCMD_OASIS_ZLIST        12
#define SCMD_OASIS_LINKS        13
#define SCMD_OASIS_AEDIT        14
#define SCMD_OASIS_CLANEDIT     15
#define SCMD_OASIS_TRIGEDIT     16
#define SCMD_OASIS_TLIST        17
#define SCMD_OASIS_QLIST        18

/* do_clan */
#define SCMD_CLAN_WHO      0
#define SCMD_CLAN_TELL     1
#define SCMD_CLAN_APPLY    2
#define SCMD_CLAN_REJECT   3
#define SCMD_CLAN_ACCEPT   4
#define SCMD_CLAN_DISMISS  5
#define SCMD_CLAN_REVOKE   6
#define SCMD_CLAN_RESIGN   7
#define SCMD_CLAN_PROMOTE  8
#define SCMD_CLAN_DEMOTE   9
#define SCMD_CLAN_DEPOSIT  10
#define SCMD_CLAN_BALANCE  11
#define SCMD_CLAN_WITHDRAW_GOLD 12
#define SCMD_SHOW_CLAN     13

/* do_vfile */

#define SCMD_V_BUGS   0
#define SCMD_V_IDEAS  1
#define SCMD_V_TYPOS  2

/*GodLookups*/

#define IS_SLURK(ch)                 (!strcmp(GET_NAME(ch), "Slurk"))
#define IS_UNTOUCHABLE(ch)	     (!strcmp(GET_NAME(ch), "Slurk") || !strcmp(GET_NAME(ch), "Bluntmaster") || !strcmp(GET_NAME(ch), "Aenarion") ||  !strcmp(GET_NAME(ch), "Anubis"))


/* god channel SCMD */
#define SCMD_IMPL           0
#define SCMD_GRGOD          1   
#define SCMD_GOD	    2 
#define SCMD_IMMORT         3 
#define SCMD_WIZNET         4
#define SCMD_PRECEPTORCHAN  5
