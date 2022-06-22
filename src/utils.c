/**************************************************************************
*  File: utils.c                                           Part of tbaMUD *
*  Usage: Various internal functions of a utility nature.                 *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"

/* external globals */
extern struct time_data time_info;
//extern void vwear_obj(int type, char_data * ch);
//extern void vwear_object(int wearpos, char_data * ch);
extern struct deity_data deities[];
extern int rev_dir[];

ROOM_DATA *find_wild_room( ROOM_DATA *wRoom, int dir, bool makenew );
int damage_obj(CHAR_DATA *ch, OBJ_DATA *obj, int damage);
void separate_obj( OBJ_DATA *obj );

/* local functions */
struct time_info_data *real_time_passed(time_t t2, time_t t1);
struct time_info_data *mud_time_passed(time_t t2, time_t t1);
void prune_crlf(char *txt);

char *add_commas(long num);
char *Commatize(long num);

/* creates a random number in interval [from;to] */
int rand_number(int from, int to)
{
  /* error checking in case people call this incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
    log("SYSERR: rand_number() should be called with lowest, then highest. (%d, %d), not (%d, %d).", from, to, to, from);
  }

  /* This should always be of the form: ((float)(to - from + 1) * rand() / 
   * (float)(RAND_MAX + from) + from); If you are using rand() due to historical
   * non-randomness of the lower bits in older implementations.  We always use 
   * circle_random() though, which shouldn't have that problem. Mean and 
   * standard deviation of both are identical (within the realm of statistical
   * identity) if the rand() implementation is non-broken. */
  return ((circle_random() % (to - from + 1)) + from);
}

/* Find a percentage, with error checking. */
int percentage(int amount, int max)
{
        if (max > 0)
                return ((amount * 100) / max);

        return (amount * 100);
}

/* simulates dice roll */
int dice(int num, int size)
{
  int sum = 0;

  if (size <= 0 || num <= 0)
    return (0);

  while (num-- > 0)
    sum += rand_number(1, size);

  return (sum);
}

bool roll(int val)
{
        int num = rand_number(1, 20);

        if (num == 20 || num > val)
                return (0);

        return (1);
}

/* Be wary of sign issues with this. */
int MIN(int a, int b)
{
  return (a < b ? a : b);
}

/* Be wary of sign issues with this. */
int MAX(int a, int b)
{
  return (a > b ? a : b);
}

/* color issue fix -- skip color codes, _then_ capitalize */
char *CAP(char *txt)
{
  char *p = txt;

  while (*p == '@' && *(p+1))
    p += 2;
  if (*p)
    *p = UPPER(*p);
  return (txt);
}

#if !defined(HAVE_STRLCPY)
/* A 'strlcpy' function in the same fashion as 'strdup' below. This copies up 
 * to totalsize - 1 bytes from the source string, placing them and a trailing 
 * NUL into the destination string. Returns the total length of the string it 
 * tried to copy, not including the trailing NUL.  So a '>= totalsize' test 
 * says it was truncated. (Note that you may have _expected_ truncation 
 * because you only wanted a few characters from the source string.) */
size_t strlcpy(char *dest, const char *source, size_t totalsize)
{
  strncpy(dest, source, totalsize - 1);	/* strncpy: OK (we must assume 'totalsize' is correct) */
  dest[totalsize - 1] = '\0';
  return strlen(source);
}
#endif

#if !defined(HAVE_STRDUP)
/* Create a duplicate of a string */
char *strdup(const char *source)
{
  char *new_z;

  CREATE(new_z, char, strlen(source) + 1);
  return (strcpy(new_z, source)); /* strcpy: OK */
}
#endif

/* Create a duplicate of a string */
char *str_dup(const char *source)
{
        char *new_z;

        CREATE(new_z, char, strlen(source) + 1);
        return (strcpy(new_z, source));
}

/* Strips \r\n from end of string. */
void prune_crlf(char *txt)
{
  int i = strlen(txt) - 1;

  while (txt[i] == '\n' || txt[i] == '\r')
    txt[i--] = '\0';
}

#ifndef str_cmp
/* str_cmp: a case-insensitive version of strcmp(). Returns: 0 if equal, > 0 
 * if arg1 > arg2, or < 0 if arg1 < arg2. Scan until strings are found 
 * different or we reach the end of both. */
int str_cmp(const char *arg1, const char *arg2)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    log("SYSERR: str_cmp() passed a NULL pointer, %p or %p.", arg1, arg2);
    return (0);
  }

  for (i = 0; arg1[i] || arg2[i]; i++)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}
#endif

#ifndef strn_cmp
/* strn_cmp: a case-insensitive version of strncmp(). Returns: 0 if equal, > 0 
 * if arg1 > arg2, or < 0 if arg1 < arg2. Scan until strings are found 
 * different, the end of both, or n is reached. */
int strn_cmp(const char *arg1, const char *arg2, int n)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    log("SYSERR: strn_cmp() passed a NULL pointer, %p or %p.", arg1, arg2);
    return (0);
  }

  for (i = 0; (arg1[i] || arg2[i]) && (n > 0); i++, n--)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}
#endif

/* New variable argument log() function.  Works the same as the old for
 * previously written code but is very nice for new code. */
void basic_mud_vlog(const char *format, va_list args)
{
  time_t ct = time(0);
  char *time_s = asctime(localtime(&ct));

  if (logfile == NULL) {
    puts("SYSERR: Using log() before stream was initialized!");
    return;
  }

  if (format == NULL)
    format = "SYSERR: log() received a NULL format.";

  time_s[strlen(time_s) - 1] = '\0';

  fprintf(logfile, "%-15.15s :: ", time_s + 4);
  vfprintf(logfile, format, args);
  fputc('\n', logfile);
  fflush(logfile);
}

void make_lower(char *begin, char *end)
{
int x;
    x=0;
    while(begin[x]){
        end[x]=LOWER(begin[x]);
        x++;
    }
    end[x] = NULL;
}

void make_upper(char *begin, char *end)
{
int x;
    x=0;
    while(begin[x]){
        end[x]=UPPER(begin[x]);
        x++;
    }
    end[x]=NULL;
}
/* So mudlog() can use the same function. */
void basic_mud_log(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  basic_mud_vlog(format, args);
  va_end(args);
}

/* the "touch" command, essentially. */
int touch(const char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    log("SYSERR: %s: %s", path, strerror(errno));
    return (-1);
  } else {
    fclose(fl);
    return (0);
  }
}

/* Log mud messages to a file & to online imm's syslogs. - Fen */
void mudlog(int type, int level, int file, const char *str, ...)
{
  char buf[MAX_STRING_LENGTH];
  struct descriptor_data *i;
  va_list args;

  if (str == NULL)
    return;	/* eh, oh well. */

  if (file) {
    va_start(args, str);
    basic_mud_vlog(str, args);
    va_end(args);
  }

  if (level < 0)
    return;

  strcpy(buf, "[ ");	/* strcpy: OK */
  va_start(args, str);
  vsnprintf(buf + 2, sizeof(buf) - 6, str, args);
  va_end(args);
  strcat(buf, " ]\r\n");	/* strcat: OK */

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING || IS_NPC(i->character)) /* switch */
      continue;
    if (GET_LEVEL(i->character) < level)
      continue;
    if (PLR_FLAGGED(i->character, PLR_WRITING))
      continue;
//    if (type > (PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0))
    if ((type != ZRT) && (type > (PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0)))
      continue;
    if ((type == ZRT) && ((!PRF_FLAGGED(i->character, PRF_LOG1)) || (!PRF_FLAGGED(i->character, PRF_LOG2))))
      continue;
    if ((type == ZRT) && (PRF_FLAGGED(i->character, PRF_LOG1)) || (PRF_FLAGGED(i->character, PRF_LOG2)) && 
(PRF_FLAGGED(i->character, PRF2_NOZONERESET)))
      continue;

    send_to_char(i->character, "%s%s%s", CCGRN(i->character, C_NRM), buf, CCNRM(i->character, C_NRM));
  }
}



/* If you don't have a 'const' array, just cast it as such.  It's safer to cast
 * a non-const array as const than to cast a const one as non-const. Doesn't 
 * really matter since this function doesn't change the array though. */
size_t sprintbit(bitvector_t bitvector, const char *names[], char *result, size_t reslen)
{
  size_t len = 0;
  int nlen;
  long nr;

  *result = '\0';

  for (nr = 0; bitvector && len < reslen; bitvector >>= 1) {
    if (IS_SET(bitvector, 1)) {
      nlen = snprintf(result + len, reslen - len, "%s ", *names[nr] != '\n' ? names[nr] : "UNDEFINED");
      if (len + nlen >= reslen || nlen < 0)
        break;
      len += nlen;
    }

    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    len = strlcpy(result, "NOBITS ", reslen);

  return (len);
}

size_t sprinttype(int type, const char *names[], char *result, size_t reslen)
{
  int nr = 0;

  while (type && *names[nr] != '\n') {
    type--;
    nr++;
  }

  return strlcpy(result, *names[nr] != '\n' ? names[nr] : "UNDEFINED", reslen);
}

void sprintbitarray(int bitvector[], const char *names[], int maxar, char *result)
{
  int nr, teller, found = FALSE;

  *result = '\0';

  for(teller = 0; teller < maxar && !found; teller++)
    for (nr = 0; nr < 32 && !found; nr++) {
      if (IS_SET_AR(bitvector, (teller*32)+nr)) {
        if (*names[(teller*32)+nr] != '\n') {
          if (*names[(teller*32)+nr] != '\0') {
            strcat(result, names[(teller*32)+nr]);
            strcat(result, " ");
          }
	} else {
          strcat(result, "UNDEFINED ");
        }
      }
      if (*names[(teller*32)+nr] == '\n')
        found = TRUE;
    }

  if (!*result)
    strcpy(result, "NOBITS ");
}

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data *real_time_passed(time_t t2, time_t t1)
{
  long secs;
  static struct time_info_data now;

  secs = t2 - t1;

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
  /* secs -= SECS_PER_REAL_DAY * now.day; - Not used. */

  now.month = -1;
  now.year = -1;

  return (&now);
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data *mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  static struct time_info_data now;

  secs = t2 - t1;

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;	/* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17;	/* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);	/* 0..XX? years */

  return (&now);
}

time_t mud_time_to_secs(struct time_info_data *now)
{
  time_t when = 0;

  when += now->year  * SECS_PER_MUD_YEAR;
  when += now->month * SECS_PER_MUD_MONTH;
  when += now->day   * SECS_PER_MUD_DAY;
  when += now->hours * SECS_PER_MUD_HOUR;
  return (time(NULL) - when);
}

struct time_info_data *age(struct char_data *ch)
{
  static struct time_info_data player_age;

  player_age = *mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;	/* All players start at 17 */

  return (&player_age);
}

/* Check if making CH follow VICTIM will create an illegal Follow Loop. */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return (TRUE);
  }

  return (FALSE);
}

/* Called when stop following persons, or stopping charm. This will NOT do if 
 * a character quits or dies. */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (ch->master == NULL) {
    core_dump();
    return;
  }

  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM))
      affect_from_char(ch, SPELL_CHARM);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) {	/* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else {			/* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    free(j);
  }

  ch->master = NULL;
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_CHARM);
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
}

int num_followers_charmed(struct char_data *ch)
{
  struct follow_type *lackey;
  int total = 0;

  for (lackey = ch->followers; lackey; lackey = lackey->next)
    if (AFF_FLAGGED(lackey->follower, AFF_CHARM) && lackey->follower->master == ch)
      total++;

  return (total);
}

/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}

/* Do NOT call this before having checked if a circle of followers will arise. 
 * CH will follow leader. */
void add_follower(struct char_data *ch, struct char_data *leader)
{
  struct follow_type *k;

  if (ch->master) {
    core_dump();
    return;
  }

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/* get_line reads the next non-blank line off of the input stream. The newline 
 * character is removed from the input.  Lines which begin with '*' are 
 * considered to be comments. Returns the number of lines advanced in the file.
 * Buffer given must be at least READ_SIZE (256) characters large. */
int get_line(FILE *fl, char *buf)
{
  char temp[READ_SIZE];
  int lines = 0;
  int sl;

  do {
    if (!fgets(temp, READ_SIZE, fl))
      return (0);
    lines++;
  } while (*temp == '*' || *temp == '\n' || *temp == '\r');

  /* Last line of file doesn't always have a \n, but it should. */
  sl = strlen(temp);
  while (sl > 0 && (temp[sl - 1] == '\n' || temp[sl - 1] == '\r'))
    temp[--sl] = '\0';

  strcpy(buf, temp); /* strcpy: OK, if buf >= READ_SIZE (256) */
  return (lines);
}

int get_filename(char *filename, size_t fbufsize, int mode, const char *orig_name)
{
  const char *prefix, *middle, *suffix;
  char name[PATH_MAX], *ptr;

  if (orig_name == NULL || *orig_name == '\0' || filename == NULL) {
    log("SYSERR: NULL pointer or empty string passed to get_filename(), %p or %p.",
		orig_name, filename);
    return (0);
  }

  switch (mode) {
  case CRASH_FILE:
    prefix = LIB_PLROBJS;
    suffix = SUF_OBJS;
    break;
  case ETEXT_FILE:
    prefix = LIB_PLRTEXT;
    suffix = SUF_TEXT;
    break;
  case SCRIPT_VARS_FILE:
    prefix = LIB_PLRVARS;
    suffix = SUF_MEM;
    break;
  case PLR_FILE:
    prefix = LIB_PLRFILES;
    suffix = SUF_PLR;
    break;
  case WARN_FILE:
    prefix = LIB_PLRWARN;
    suffix = SUF_WARN;
    break;
  default:
    return (0);
  }

  strlcpy(name, orig_name, sizeof(name));
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }

  snprintf(filename, fbufsize, "%s%s"SLASH"%s.%s", prefix, middle, name, suffix);
  return (1);
}

int num_pc_in_room(struct room_data *room)
{
  int i = 0;
  struct char_data *ch;

  for (ch = room->people; ch != NULL; ch = ch->next_in_room)
    if (!IS_NPC(ch))
      i++;

  return (i);
}

/* This function (derived from basic fork() abort() idea by Erwin S. Andreasen)
 * causes your MUD to dump core (assuming you can) but continue running. The 
 * core dump will allow post-mortem debugging that is less severe than assert();
 * Don't call this directly as core_dump_unix() but as simply 'core_dump()' so 
 * that it will be excluded from systems not supporting them. You still want to
 * call abort() or exit(1) for non-recoverable errors, of course. Wonder if 
 * flushing streams includes sockets? */
extern FILE *player_fl;
void core_dump_real(const char *who, int line)
{
  log("SYSERR: Assertion failed at %s:%d!", who, line);

#if 1	/* By default, let's not litter. */
#if defined(CIRCLE_UNIX)
  /* These would be duplicated otherwise...make very sure. */
  fflush(stdout);
  fflush(stderr);
  fflush(logfile);
  /* Everything, just in case, for the systems that support it. */
  fflush(NULL);

  /* Kill the child so the debugger or script doesn't think the MUD crashed. 
   * The 'autorun' script would otherwise run it again. */
  if (fork() == 0)
    abort();
#endif
#endif
}

int count_color_chars(char *string)
{
  int i, len;
  int num = 0;

	if (!string || !*string)
		return 0;

	len = strlen(string);
  for (i = 0; i < len; i++) {
    while (string[i] == '@') {
      if (string[i + 1] == '@')
        num++;
      else
        num += 2;
      i += 2;
    }
  }
  return num;
}

/* Rules (unless overridden by ROOM_DARK): Inside and City rooms are always 
 * lit. Outside rooms are dark at sunset and night. */
int room_is_dark(room_rnum room)
{
  if (!VALID_ROOM_RNUM(room)) {
    log("room_is_dark: Invalid room rnum %d. (0-%d)", room, top_of_world);
    return (FALSE);
  }

  if (world[room].light)
    return (FALSE);

  if (ROOM_FLAGGED(room, ROOM_DARK))
    return (TRUE);

  if (SECT(room) == SECT_INSIDE || SECT(room) == SECT_CITY)
    return (FALSE);

  if (Sunlight == SUN_SET || Sunlight == SUN_DARK)
    return (TRUE);

  return (FALSE);
}

int levenshtein_distance(char *s1, char *s2)
{   
  int s1_len = strlen(s1), s2_len = strlen(s2);
  int **d, i, j; 

  CREATE(d, int *, s1_len + 1);

  for (i = 0; i <= s1_len; i++) {   
    CREATE(d[i], int, s2_len + 1);
    d[i][0] = i;
  }

  for (j = 0; j <= s2_len; j++)
    d[0][j] = j; 
  for (i = 1; i <= s1_len; i++)
    for (j = 1; j <= s2_len; j++)
      d[i][j] = MIN(d[i - 1][j] + 1, MIN(d[i][j - 1] + 1,
      d[i - 1][j - 1] + ((s1[i - 1] == s2[j - 1]) ? 0 : 1)));

  i = d[s1_len][s2_len];

  for (j = 0; j <= s1_len; j++)
    free(d[j]);
  free(d);

  return i;
} 
													   
void char_from_furniture(struct char_data *ch)
{
  struct obj_data *furniture;
  struct char_data *tempch;
  int i, found = 0;

  if (!SITTING(ch))
    return;

  if (!(furniture = SITTING(ch))){
    log("SYSERR: No furniture for char in char_from_furniture.");
    SITTING(ch) = NULL;
    NEXT_SITTING(ch) = NULL;
    return;
  }
  
  if (!(tempch = OBJ_SAT_IN_BY(furniture))){
    log("SYSERR: Char from furniture, but no furniture!");
    SITTING(ch) = NULL;
    NEXT_SITTING(ch) = NULL;
    return;
  }
    
  if (tempch == ch){
    if (!NEXT_SITTING(ch))  
      OBJ_SAT_IN_BY(furniture) = NULL;
    else
      OBJ_SAT_IN_BY(furniture) = NEXT_SITTING(ch);
    GET_OBJ_VAL(furniture, 1) -= 1;
    SITTING(ch) = NULL;
    NEXT_SITTING(ch) = NULL;
    return;
  }

  for (i = 0; i < GET_OBJ_VAL(furniture, 1) && found == 0; i++){
    if (NEXT_SITTING(tempch) != ch){
      NEXT_SITTING(tempch) = NEXT_SITTING(ch);
      found++;
    }   
  }
  if (found)
    log("SYSERR: Char flagged as sitting, but not in furniture.");
  else
    GET_OBJ_VAL(furniture, 1) -= 1;
   
  SITTING(ch) = NULL;
  NEXT_SITTING(ch) = NULL;

 return;
}


void vwear_obj(int type, struct char_data * ch)
{
  int nr, found =0;  
  char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';         

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (GET_OBJ_TYPE(&obj_proto[nr]) == type) {
      sprintf(buf, "%s%3d. [%5d] %s&n\r\n", buf, ++found,
              obj_index[nr].vnum, obj_proto[nr].short_description);
    }
  }
  page_string(ch->desc, buf, 1);            
}

void vwear_object(int wearpos, struct char_data * ch)
{
  int nr, found = 0;                  
  char buf[MAX_STRING_LENGTH];        

  buf[0] = '\0';        
  for (nr = 0; nr <= top_of_objt; nr++) {
    if ((GET_OBJ_TYPE(&obj_proto[nr]) == ITEM_SCROLL) ||
        (GET_OBJ_TYPE(&obj_proto[nr]) == ITEM_WAND) ||
        (GET_OBJ_TYPE(&obj_proto[nr]) == ITEM_STAFF) ||
        (GET_OBJ_TYPE(&obj_proto[nr]) == ITEM_POTION))
      continue;

    if (CAN_WEAR(&obj_proto[nr], wearpos)) {
      sprintf(buf, "%s%3d. [%5d] %s&n\r\n", buf, ++found,
              obj_index[nr].vnum, obj_proto[nr].short_description);
    }
  }
  page_string(ch->desc, buf, 1);
}


void *my_free(void *ptr)
{

    if (ptr != NULL)       
    {
        if (!strcmp(ptr,"@"))
            return(NULL);
        strcpy(ptr,"@");
        free(ptr);
    }
    return(NULL);
}  


/*               
   strfrmt (String Format) function
   Used by automap/map system
   Re-formats a string to fit within a particular size box.       
   Recognises @ color codes, and if a line ends in one color, the
   next line will start with the same color.
   Ends every line with @n to prevent color bleeds.
*/
char *strfrmt(char *str, int w, int h, int justify, int hpad, int vpad)
{    
  static char ret[MAX_STRING_LENGTH];
  char line[MAX_INPUT_LENGTH];
  char *sp = str;
  char *lp = line; 
  char *rp = ret;
  char *wp;
  int wlen = 0, llen = 0, lcount = 0;
  char last_color='n';
  bool new_line_started = FALSE;          

  memset(line, '\0', MAX_INPUT_LENGTH); 
  /* Nomalize spaces and newlines */          
  /* Split into lines, including convert \\ into \r\n */
  while(*sp) {                  
    /* eat leading space */
    while(*sp && isspace(*sp)) sp++;      
    /* word begins */
    wp = sp;       
    wlen = 0;
    while(*sp) { /* Find the end of the word */
      if(isspace(*sp)) break;
      if(*sp=='\\' && sp[1] && sp[1]=='\\') {
        if(sp!=wp)
          break; /* Finish dealing with the current word */
        sp += 2; /* Eat the marker and any trailing space */
        while(*sp && isspace(*sp)) sp++;
        wp = sp;
        /* Start a new line */
        if(hpad)
          for(; llen < w; llen++)
            *lp++ = ' ';
        *lp++ = '\r';
        *lp++ = '\n';
        *lp++ = '\0';
        rp += sprintf(rp, "%s", line);
        llen = 0;
        lcount++;
        lp = line;
      } else if (*sp=='`'||*sp=='$'||*sp=='#'||*sp=='&') {
       if (sp[1] && (sp[1]==*sp))
          wlen++; /* One printable char here */
        if (*sp=='&' && (sp[1]!=*sp)) /* Color code, not @@ */
          last_color = sp[1];
        sp += 2; /* Eat the whole code regardless */
      } else {
        wlen++;
        sp++;
      }
    }
    if(llen + wlen + (lp==line ? 0 : 1) > w) {
      /* Start a new line */       
      if(hpad)               
        for(; llen < w; llen++)
          *lp++ = ' ';
      *lp++ = '&';  /* 'normal' color */    
      *lp++ = 'n';
      *lp++ = '\r'; /* New line */
      *lp++ = '\n';
      *lp++ = '\0';
      sprintf(rp, "%s", line);       
      rp += strlen(line);     
      llen = 0;  
      lcount++;    
      lp = line; 
      if (last_color != 'n') {
        *lp++ = '&';  /* restore previous color */
        *lp++ = last_color;
        new_line_started = TRUE;          
      }
    }
    /* add word to line */                    
    if (lp!=line && new_line_started!=TRUE) {           
      *lp++ = ' ';              
      llen++;              
    }
    new_line_started = FALSE;
    llen += wlen ; 
    for( ; wp!=sp ; *lp++ = *wp++);
  }
  /* Copy over the last line */
  if(lp!=line) {
    if(hpad)      
      for(; llen < w; llen++)
        *lp++ = ' ';
    *lp++ = '\r';
    *lp++ = '\n';
    *lp++ = '\0';             
    sprintf(rp, "%s", line);
    rp += strlen(line);          
    lcount++;           
  }                  
  if(vpad) {         
    while(lcount < h) {
      if(hpad) {
        memset(rp, ' ', w);
        rp += w; 
      }           
      *rp++ = '\r';
      *rp++ = '\n';              
      lcount++;
    }
   *rp = '\0';              
  }
  return ret; 
}              

/*     
   strpaste (String Paste) function
   Takes two long strings (multiple lines) and joins them side-by-side.
   Used by the automap/map system  
*/                           
char *strpaste(char *str1, char *str2, char *joiner)
{
  static char ret[MAX_STRING_LENGTH+1];     
  char *sp1 = str1;
  char *sp2 = str2;               
  char *rp = ret;  
  int jlen = strlen(joiner);

  while((rp - ret) < MAX_STRING_LENGTH && (*sp1 || *sp2)) {
     /* Copy line from str1 */
    while((rp - ret) < MAX_STRING_LENGTH && *sp1 && !ISNEWL(*sp1))
      *rp++ = *sp1++;
    /* Eat the newline */     
    if(*sp1) {
      if(sp1[1] && sp1[1]!=sp1[0] && ISNEWL(sp1[1]))
        sp1++;                            
      sp1++;
    }

    /* Add the joiner */
    if((rp - ret) + jlen >= MAX_STRING_LENGTH)
      break;               
    strcpy(rp, joiner);
    rp += jlen;              

     /* Copy line from str2 */     
    while((rp - ret) < MAX_STRING_LENGTH && *sp2 && !ISNEWL(*sp2))
      *rp++ = *sp2++;          
    /* Eat the newline */
    if(*sp2) {    
      if(sp2[1] && sp2[1]!=sp2[0] && ISNEWL(sp2[1]))
        sp2++;      
      sp2++;     
    }            

    /* Add the newline */   
    if((rp - ret) + 2 >= MAX_STRING_LENGTH)
      break;            
    *rp++ = '\r';    
    *rp++ = '\n';    
  }
  /* Close off the string */
  *rp = '\0';              
  return ret;    
}                 

/** A string converter designed to deal with the compile sensitive IDXTYPE. 
 *  * Relies on the friendlier strtol function. 
 *   * @pre Assumes that NOWHERE, NOTHING, NOBODY, NOFLAG, etc are all equal. 
 *    * @param str_to_conv A string of characters to attempt to convert to an 
 *     * IDXTYPE number. 
 *      * @retval IDXTYPE A valid index number, or NOWHERE if not valid. 
 *       */ 
IDXTYPE atoidx( const char *str_to_conv ) 
{ 
  long int result; 
  
  /* Check for errors */ 
  errno = 0; 
  
  result = strtol(str_to_conv, NULL, 10); 
  
  if ( errno || (result > IDXTYPE_MAX) || (result < 0) ) 
    return NOWHERE; /* All of the NO* settings should be the same */ 
  else 
    return (IDXTYPE) result; 
} 

char* add_commas(long X)
{
  int oldlen = 0;
  int newlen = 0;
  int numcommas = 0;
  int i;
  int j;
  char temp[MAX_INPUT_LENGTH];
  static char commastring[MAX_INPUT_LENGTH];

  *temp='\0';
  *commastring='\0';

  sprintf(temp,"%ld",X);
  oldlen = strlen(temp);
  if (X < 0) //don't count the negative sign
  {
    numcommas = (oldlen-2)/3;
    newlen = oldlen + numcommas;
  }
  else
  {
    numcommas = (oldlen-1)/3;
    newlen = oldlen + numcommas;
  }

  //build the new string from the right,
  //inserting commas as needed
  for(i=oldlen, j=newlen; i>0; i--,j--)
  {
      if ((i-1)%3 == 0) //need a comma after this digit
      {
        commastring[newlen-j] = temp[oldlen-i];
        if (temp[oldlen-i] != '-')
        {
          j--;
          commastring[newlen - j] = ',';
        }
      }
      else
      {
        commastring[newlen-j] = temp[oldlen-i];
      }
  }
  commastring[newlen] = '\0';
  return commastring;
}

char *Commatize(long num)
{
  #define COUNT_PER_GRP  3
  #define BUF_COUNT     10
  #define COUNT_PER_BUF 16

  int i, j, len, neg = (num<0);
  char string[COUNT_PER_BUF];
  static char comma_string[BUF_COUNT][COUNT_PER_BUF];
  static int which = 0;

  sprintf(string, "%ld", num);
  len = strlen(string);

  for (i = j = 0; string[i]; ++i) {
   if ((len - i) % COUNT_PER_GRP == 0 && i && i - neg)
     comma_string[which][j++] = ',';
     comma_string[which][j++] = string[i];
  }

  comma_string[which][j++] = '\0';

  i = which;

  which = (which + 1) % BUF_COUNT;

  return comma_string[i];

  #undef COUNT_PER_GRP
  #undef BUF_COUNT
  #undef COUNT_PER_BUF
}

EXIT_DATA *make_exit( ROOM_DATA *pRoom, ROOM_DATA *to_room, sh_int door )
{
        EXIT_DATA *pexit, *texit;
        bool broke;

        CREATE( pexit, EXIT_DATA, 1 );
        pexit->coord            = NULL;
        pexit->rvcoord          = NULL;
        pexit->vdir                     = door;
        pexit->rvnum            = pRoom->number;
        pexit->to_room          = to_room;

        if ( to_room )
        {
                if ( pRoom->zone == WILD_ZONE && to_room->zone == WILD_ZONE)
                {
                        CREATE( pexit->rvcoord, COORD_DATA, 1 );
                        *pexit->rvcoord                 = *pRoom->coord;
                        CREATE( pexit->coord, COORD_DATA, 1 );
                        *pexit->coord                   = *to_room->coord;
                        texit = get_exit_to_coord( to_room, rev_dir[door], pRoom->coord );
                }
                else
                {
                        pexit->vnum = to_room->number;
                        texit = get_exit_to( to_room, rev_dir[door], pRoom->number );
                }
                if ( texit )    /* assign reverse exit pointers */
                {
                        texit->rexit                    = pexit;
                        pexit->rexit                    = texit;
                }
        }

        broke = FALSE;

        for ( texit = pRoom->first_exit; texit; texit = texit->next )
        {
                if ( door < texit->vdir )
                {
                        broke = TRUE;
                        break;
                }
        }
        if ( !pRoom->first_exit )
                pRoom->first_exit                       = pexit;
        else
        {
                /* keep exits in incremental order - insert exit into list */
                if ( broke && texit )
                {
                        if ( !texit->prev )
                                pRoom->first_exit       = pexit;
                        else
                                texit->prev->next       = pexit;
                        pexit->prev                             = texit->prev;
                        pexit->next                             = texit;
                        texit->prev                             = pexit;

                        return ( pexit );
                }
                pRoom->last_exit->next          = pexit;
        }
        pexit->next                                             = NULL;
        pexit->prev                                             = pRoom->last_exit;
        pRoom->last_exit                                = pexit;

        return ( pexit );
}

EXIT_DATA *get_exit(ROOM_DATA *room, sh_int dir)
{
        EXIT_DATA *xit;

        if (!room)
        {
                log("SYSERR: Get_exit: NULL room");
                return (NULL);
        }

        for (xit = room->first_exit; xit; xit = xit->next )
        {
                if (xit->vdir == dir)
                {
                        if (EXIT_FLAGGED(xit, EX_HIDDEN))
                                return (NULL);
                        return (xit);
                }
        }

        if (IS_WILD(room) && (dir != UP && dir != DOWN))
        {
                /*
 *                  * this is the hack.. find_wild_room creates the
 *                                   * destination room and the exits between
 *                                                    * these two rooms
 *                                                                     */
                find_wild_room(room, dir, TRUE);

                for (xit = room->first_exit; xit; xit = xit->next)
                {
                        if (xit->vdir == dir)
                                return (xit);
                }
        }

        return (NULL);
}

/*
 *  * Function to get an exit, leading the the specified room
 *   */
EXIT_DATA *get_exit_to( ROOM_DATA *room, sh_int dir, int vnum )
{
        EXIT_DATA *xit;

        if ( !room )
        {
                log( "Get_exit: NULL room" );
                return ( NULL );
        }

        for (xit = room->first_exit; xit; xit = xit->next )
                if ( xit->vdir == dir && xit->vnum == vnum )
                        return ( xit );
        return ( NULL );
}

/*
 *  * Function to get the nth exit of a room                       -Thoric
 *   */
EXIT_DATA *get_exit_num( ROOM_DATA *room, sh_int count )
{
        EXIT_DATA *xit;
        int cnt;

        if ( !room )
        {
                log( "Get_exit: NULL room" );
                return ( NULL );
        }

        for (cnt = 0, xit = room->first_exit; xit; xit = xit->next )
                if ( ++cnt == count )
                        return ( xit );

        return ( NULL );
}

/*
 *  * Function to get an exit, leading to the specified coordinates
 *   */
EXIT_DATA *get_exit_to_coord( ROOM_DATA *room, sh_int dir, COORD_DATA *coord )
{
        EXIT_DATA *xit;

        if ( !room )
        {
                log( "Get_exit: NULL room" );
                return ( NULL );
        }

        for (xit = room->first_exit; xit; xit = xit->next )
                if ( xit->vdir == dir &&
                     ( xit->coord->x == coord->x && xit->coord->y == coord->y ) )
                        return ( xit );
        return ( NULL );
}

/*
*  * Remove an exit from a room                                   -Thoric
*   */
void extract_exit( ROOM_DATA *room, EXIT_DATA *pexit )
{
        UNLINK( pexit, room->first_exit, room->last_exit, next, prev );
        if ( pexit->rexit )
                pexit->rexit->rexit = NULL;
        if ( pexit->keyword )
                STRFREE( pexit->keyword );
        if ( pexit->description )
                STRFREE( pexit->description );

        if ( pexit->coord )
                DISPOSE(pexit->coord);

        DISPOSE( pexit );
}

/*
int is_valid_deity(int char_ethos, int deity)
{
  int allowed = deities[deity].ethos;

  switch (char_ethos) {
    case ETHOS_EVIL:
      if (allowed == ETHOS_EVIL || allowed == ETHOS_ALL)
      return (TRUE);
      break;
    case ETHOS_NEUTRAL:
      if (allowed == ETHOS_NEUTRAL || allowed == ETHOS_ALL)
      return (TRUE);
      break;
    case ETHOS_GOOD:
      if (allowed == ETHOS_GOOD || allowed == ETHOS_ALL)
      return (TRUE);
      break;
    default:
      log("SYSERR: Undefined character ethos sent to is_valid_deith in utils.c");
      break;
   }
   return (FALSE);
}


int is_valid_ethos(int class, int ethos)
{ // only use this to restrict certain classes from being anything but one ethos
   switch(GET_CLASS(ch)) {
     case CLASS_ASSASSIN:
       if (ethos == ETHOS_EVIL)
         return(TRUE);
       break;
// do this for EVERY class
     default:
       return (TRUE); // all classes not specifically restricted can be any ethos
    }
    return (FALSE);
}
*/


bool valid_exit(EXIT_DATA *pexit)
{
        if (!pexit)
                return (FALSE);

        if ( EXIT_FLAGGED(pexit, EX_CLOSED)     ||
//             EXIT_FLAGGED(pexit, EX_FALSE)      ||
             EXIT_FLAGGED(pexit, EX_HIDDEN) )          
                return (FALSE);

        return (TRUE);
}


bool water_sector(int sect)
{
        if (sect == SECT_WATER_NOSWIM || sect == SECT_RIVER_NAVIGABLE || sect == SECT_SEA)
                return (TRUE);

        /* TODO -- Simple river can be crossed with a check.. */
        if (sect == SECT_RIVER)
                return (TRUE);

        return (FALSE);
}

EXIT_DATA *find_exit(ROOM_DATA *room, sh_int dir)
{
        EXIT_DATA *xit;

        if (!room)
        {
                log("SYSERR: Get_exit: NULL room");
                return (NULL);
        }

        for (xit = room->first_exit; xit; xit = xit->next )
        {
                if (xit->vdir == dir)
                        return (xit);
        }

        return (NULL);
}

time_t days_passed(time_t last_date)
{
        TIME_INFO_DATA *tdiff = NULL;
        time_t days = 0;

        tdiff = mud_time_passed(time(0), last_date);

        if (tdiff->year > 0)
                days += tdiff->year  * 35 * 24;
        if (tdiff->month > 0)
                days += tdiff->month * 24;
        days += tdiff->day;

        return (days);
}

/* Assumption - NULL obj means check all equipment */
void check_damage_obj(CHAR_DATA *ch, OBJ_DATA *obj, int chance)
{
        OBJ_DATA *dobj = NULL;
        bool done = FALSE;
        int damage_pos, stop = 0;

        /* Increase wear and tear to stimulate economy... */
        chance *= 2;

        if ( rand_number(1, 100) > chance )
                return;

        /* Null object means damage a random equipped one... */
        if (!obj)
        {
                while (!done && stop <= 30)
                {
                        damage_pos = rand_number(1, NUM_WEARS);
                        dobj = GET_EQ(ch, damage_pos);
                        if (dobj)
                        {
                                done = TRUE;
                                damage_obj(ch, dobj, dice(1, 3));
                        }
                        stop++;
                }
        }
        else
                damage_obj(ch, obj, dice(1, 3));
}

int damage_obj(CHAR_DATA *ch, OBJ_DATA *obj, int damage)
{
        int damres = 0;

        if (!ch || !obj || damage < 0)
                return (0);

        /* undestructable items exit here.. */
        if (GET_OBJ_COND(obj) == -1 || OBJ_FLAGGED(obj, ITEM_NO_DAMAGE) )
                return (0);

        /* magic items are strong */
        if (OBJ_FLAGGED(obj, ITEM_MAGIC) && !rand_number(0, 3))
                return (0);

        /* blessed items too, and better */
        if (OBJ_FLAGGED(obj, ITEM_BLESS) && !rand_number(0, 2))
                return (0);

        /* hmmm.. here or in check_damage_obj?? */
        separate_obj(obj);

        GET_OBJ_COND(obj) -= damage;
        GET_OBJ_COND(obj) = URANGE(0, GET_OBJ_COND(obj), 5000);

        return (damres);
}


int GET_TOTAL_LEVEL(struct char_data *ch)
{
  int classes, level = 0;

    classes = (GET_WAS_WARRIOR(ch) + GET_WAS_CLERIC(ch) + GET_WAS_THIEF(ch) + GET_WAS_MAGE(ch) + GET_WAS_PALADIN(ch)+
               GET_WAS_KNIGHT(ch)+ GET_WAS_RANGER(ch)+ GET_WAS_MYSTIC(ch)+ GET_WAS_PRIEST(ch)+ GET_WAS_SHAMAN(ch)+
               GET_WAS_NECROMANCER(ch)+ GET_WAS_ELEMENTALIST(ch)+ GET_WAS_SORCERER(ch)+ GET_WAS_ROGUE(ch)+
               GET_WAS_NINJA(ch)+ GET_WAS_ASSASSIN(ch));

    switch(classes)
    {
       case 1: 
          level = GET_LEVEL(ch);
           break;
       case 2:
          level = GET_LEVEL(ch) + 40;
          break;
       case 3:
          level = GET_LEVEL(ch) + 80;
          break;
       case 4: 
          level = GET_LEVEL(ch) + 120;
          break;
       default: 
          level = GET_LEVEL(ch) + 160;
     }
    
     return(level);
}


char *strip_colors(const char *arg)
{
  const char *input;
  const char *i;
  char *point;
  char lbuf[MAX_STRING_LENGTH];
  char lbuf2[MAX_STRING_LENGTH];

  point = lbuf;   
  input = arg;      

  for ( ; *input != '\0' ; ) {
    if (*input != '&') {      
      *point++ = *input++;
      continue;
    }               

    switch (*(++input)) {        
    case '&':       
      sprintf(lbuf2, "&&"); 
      i = lbuf2;   
      break;            
    default:          
      sprintf(lbuf2, "&&%c", *(input));
      i = lbuf2;       
      break;         
    }
    ++input;

    for ( ; (*point = *i) != '\0' ; )                
      ++point, ++i;

  }                
  *point = '\0';
  return strdup(lbuf);  
}

const int potions[] = 
{
    7200,
    7201
};

void LoadPotions(struct obj_data *corpse)
{
  struct obj_data *object;
  int i, rand1, rand2;
  obj_vnum onum;

  // pick one of the potions
  rand1 = rand_number(0,1);

  // load it on the corpse after determining how many to load
  if (potions[rand1] == 7201) //armor potion should only load 1x
    rand2 = rand_number(0,1);
  else
    rand2 = rand_number(0, 5);
  
  for (i = 0; i < 5; i++) {
   if (rand_number(0,100) <= 25) {
     onum = real_object(potions[rand1]);
     object = read_object(onum, REAL);
     obj_to_obj(object, corpse);
   }
  }
}

//Unimplemented, but has possibilities
/*
void LoadJunk(struct obj_data *corpse)
{
  struct obj_data *object;
  int junklist[] = { };
  int i, j,  rand1, rand2;
 
  for (j=0;j != '\n';j++) 
    holder++;

  rand1 = rand_number(0, holder);

  rand2 = rand_number(0, 2);

  for (i=0;i<2;i++) {
    if (rand_number(0,700) <= 10) {
      object= read_object(junklist[rand1], VIRTUAL);
      obj_to_obj(object, corpse);
    }
  }
}
*/

int get_flag_by_name(const char *flag_list[], char *flag_name)
{
  int i=0;
  for ( ; flag_list[i] && *flag_list[i] && strcmp(flag_list[i], "\n") !=0; i++)
    if (!strcmp(flag_list[i], flag_name))
      return(i);
    return(NOFLAG);
}
