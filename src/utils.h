/**************************************************************************
*  File: utils.h                                           Part of tbaMUD *
*  Usage: Header file, utility macros and prototypes of utility funcs.    *
*                                                                         *
*  All rights reserved.  See license for complete information.            *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

/* external declarations and prototypes */
extern struct weather_data weather_info;
extern FILE *logfile;

#define log			basic_mud_log

#define READ_SIZE	256

/* public functions in utils.c */
void	basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void	basic_mud_vlog(const char *format, va_list args);
int	touch(const char *path);
void	mudlog(int type, int level, int file, const char *str, ...) __attribute__ ((format (printf, 4, 5)));
void	log_death_trap(struct char_data *ch);
int	rand_number(int from, int to);
int	dice(int number, int size);
size_t	sprintbit(bitvector_t vektor, const char *names[], char *result, size_t reslen);
size_t	sprinttype(int type, const char *names[], char *result, size_t reslen);
void sprintbitarray(int bitvector[], const char *names[], int maxar, char *result);
int	get_line(FILE *fl, char *buf);
int	get_filename(char *filename, size_t fbufsize, int mode, const char *orig_name);
time_t	mud_time_to_secs(struct time_info_data *now);
struct  time_info_data *age(struct char_data *ch);
int	num_pc_in_room(struct room_data *room);
void	core_dump_real(const char *who, int line);
int     count_color_chars(char *string);
int	room_is_dark(room_rnum room);
int     levenshtein_distance(char *s1, char *s2);
void    *my_free(void *ptr);
bool    sub_gold(CHAR_DATA *ch, int amount);
void    sub_all_gold(OBJ_DATA *list, int *amount);
bool    roll(int val);
time_t  days_passed(time_t last_date);
void check_damage_obj(CHAR_DATA *ch, OBJ_DATA *obj, int chance);


/* SLURKS ADDINS */
char* add_commas(long X);
int is_valid_deity(int char_ethos, int deity);
int is_valid_ethos(int class, int ethos);
char *str_dup(const char *source);
int get_flag_by_name(const char *flag_list[], char *flag_name);

/*---------------*/

IDXTYPE atoidx( const char *str_to_conv );

#define core_dump()		core_dump_real(__FILE__, __LINE__)


/* Only provide our versions if one isn't in the C library. These macro names
 * will be defined by sysdep.h if a strcasecmp or stricmp exists. */
#ifndef str_cmp
int	str_cmp(const char *arg1, const char *arg2);
#endif
#ifndef strn_cmp
int	strn_cmp(const char *arg1, const char *arg2, int n);
#endif

/* random functions in random.c */
void circle_srandom(unsigned long initial_seed);
unsigned long circle_random(void);

/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

int MAX(int a, int b);
int MIN(int a, int b);
char *CAP(char *txt);

/* Followers */
int	num_followers_charmed(struct char_data *ch);
void	die_follower(struct char_data *ch);
void	add_follower(struct char_data *ch, struct char_data *leader);
void	stop_follower(struct char_data *ch);
bool	circle_follow(struct char_data *ch, struct char_data *victim);

/* in act.informative.c */
void	look_at_room(struct char_data *ch, int mode);
void    add_history(struct char_data *ch, char *msg, int type);

/* in act.movmement.c */
int	do_simple_move(struct char_data *ch, int dir, int following);
int	perform_move(struct char_data *ch, int dir, int following);

/* in limits.c */
int	mana_gain(struct char_data *ch);
int	hit_gain(struct char_data *ch);
int	move_gain(struct char_data *ch);
void	advance_level(struct char_data *ch);
void	set_title(struct char_data *ch, char *title);
void    set_email(struct char_data *ch, char *mail_addr);
void	gain_exp(struct char_data *ch, int gain);
void	gain_exp_regardless(struct char_data *ch, int gain);
void	gain_condition(struct char_data *ch, int condition, int value);
void	point_update(void);
void	update_pos(struct char_data *victim);

void char_from_furniture(struct char_data *ch);
#define SITTING(ch)             ((ch)->char_specials.furniture)
#define NEXT_SITTING(ch)        ((ch)->char_specials.next_in_furniture)
#define OBJ_SAT_IN_BY(obj)      ((obj)->sitting_here)

/* various constants */

/* defines for mudlog() */
#define OFF	0
#define BRF	1
#define NRM	2
#define CMP	3
#define ZRT	4

/* get_filename() */
#define CRASH_FILE	 0
#define ETEXT_FILE       1
#define SCRIPT_VARS_FILE 2
#define PLR_FILE         3
#define WARN_FILE        4
#define MAX_FILES        5

/* breadth-first searching */
#define BFS_ERROR		(-1)
#define BFS_ALREADY_THERE	(-2)
#define BFS_NO_PATH		(-3)

/* These constants should be configurable. See act.informative.c and utils.c 
 * for other places to change. */
/* mud-life time */
#define SECS_PER_MUD_HOUR	75
#define SECS_PER_MUD_DAY	(24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH	(35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR	(17*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN	60
#define SECS_PER_REAL_HOUR	(60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY	(24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR	(365*SECS_PER_REAL_DAY)

#define MUD_DAYS_PER_YEAR               595

/* string utils */
#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')

/* See also: ANA, SANA */
#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")
#define UAN(string)                             (strchr("aeiouAEIOU", *string) ? "An" : "A")

/* memory utils */
#define CREATE(result, type, number)  do {\
	if ((number) * sizeof(type) <= 0)	\
		log("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);	\
	if (!((result) = (type *) calloc ((number), sizeof(type))))	\
		{ perror("SYSERR: malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("SYSERR: realloc failure"); abort(); } } while(0)

/* the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++. */
#define REMOVE_FROM_LIST(item, head, next)	\
   if ((item) == (head))		\
      head = (item)->next;		\
   else {				\
      temp = head;			\
      while (temp && (temp->next != (item))) \
	 temp = temp->next;		\
      if (temp)				\
         temp->next = (item)->next;	\
   }					\

/* basic bitvector utils */
#define Q_FIELD(x)  ((int) (x) / 32)
#define Q_BIT(x)    (1 << ((x) % 32))
#define IS_SET_AR(var, bit)       ((var)[Q_FIELD(bit)] & Q_BIT(bit))
#define SET_BIT_AR(var, bit)      ((var)[Q_FIELD(bit)] |= Q_BIT(bit))
#define REMOVE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] &= ~Q_BIT(bit))
#define TOGGLE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] = (var)[Q_FIELD(bit)] ^ Q_BIT(bit))
#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) ^= (bit))

/* Accessing player specific data structures on a mobile is a very bad thing
 * to do.  Consider that changing these variables for a single mob will change
 * it for every other single mob in the game.  If we didn't specifically check
 * for it, 'wimpy' would be an extremely bad thing for a mob to do, as an
 * example.  If you really couldn't care less, change this to a '#if 0'. */
#if 1
/* Subtle bug in the '#var', but works well for now. */
#define CHECK_PLAYER_SPECIAL(ch, var) \
	(*(((ch)->player_specials == &dummy_mob) ? (log("SYSERR: Mob using '"#var"' at %s:%d.", __FILE__, __LINE__), &(var)) : &(var)))
#else
#define CHECK_PLAYER_SPECIAL(ch, var)	(var)
#endif

#define MOB_FLAGS(ch)	((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch)	((ch)->char_specials.saved.act)
#define PRF_FLAGS(ch) CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.pref))
#define AFF_FLAGS(ch)	((ch)->char_specials.saved.affected_by)
#define ROOM_FLAGS(loc)	(world[(loc)].room_flags)
#define ZONE_FLAGS(rnum)       (zone_table[(rnum)].zone_flags)
#define ZONE_MINLVL(rnum)      (zone_table[(rnum)].min_level)
#define ZONE_MAXLVL(rnum)      (zone_table[(rnum)].max_level)


#define SPELL_ROUTINES(spl)	(spell_info[spl].routines)

/* IS_MOB() acts as a VALID_MOB_RNUM()-like function.*/
#define IS_NPC(ch)	(IS_SET_AR(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)	(IS_NPC(ch) && GET_MOB_RNUM(ch) <= top_of_mobt && \
				GET_MOB_RNUM(ch) != NOBODY)

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET_AR(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET_AR(AFF_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET_AR(PRF_FLAGS(ch), (flag)))
#define PRF2_FLAGGED(ch, flag) (IS_SET_AR(PRF2_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET_AR(ROOM_FLAGS(loc), (flag)))
#define ZONE_FLAGGED(rnum, flag) (IS_SET_AR(zone_table[(rnum)].zone_flags, (flag)))
#define EXIT_FLAGGED(exit, flag) (IS_SET((exit)->exit_info, (flag)))
#define OBJAFF_FLAGGED(obj, flag) (IS_SET_AR(GET_OBJ_AFFECT(obj), (flag)))
#define OBJVAL_FLAGGED(obj, flag) (IS_SET(GET_OBJ_VAL((obj), 1), (flag)))
#define OBJWEAR_FLAGGED(obj, flag) (IS_SET_AR(GET_OBJ_WEAR(obj), (flag)))
#define OBJ_FLAGGED(obj, flag) (IS_SET_AR(GET_OBJ_EXTRA(obj), (flag)))
#define HAS_SPELL_ROUTINE(spl, flag) (IS_SET(SPELL_ROUTINES(spl), (flag)))

#define LOG_FLAGGED(ch, flag) (IS_SET(GET_LOGS(ch), (flag)))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PLR_FLAGS(ch), (flag))) & Q_BIT(flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PRF_FLAGS(ch), (flag))) & Q_BIT(flag))
#define PRF2_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PRF2_FLAGS(ch), (flag))) & Q_BIT(flag))

/* new define for quick check */
#define DEAD(ch) (PLR_FLAGGED((ch), PLR_NOTDEADYET) || MOB_FLAGGED((ch), MOB_NOTDEADYET))

/* room utils */
#define SECT(room)	(VALID_ROOM_RNUM(room) ? \
				world[(room)].sector_type : SECT_INSIDE)

#define GET_ROOM_ZONE(room)   (VALID_ROOM_RNUM(room) ? world[(room)].zone : NOWHERE)

#define IS_DARK(room)	room_is_dark((room))
#define IS_LIGHT(room)  (!IS_DARK(room))

#define VALID_ROOM_RNUM(rnum)	((rnum) != NOWHERE && (rnum) <= top_of_world)
#define GET_ROOM_VNUM(rnum) \
	((room_vnum)(VALID_ROOM_RNUM(rnum) ? world[(rnum)].number : NOWHERE))
#define GET_ROOM_SPEC(room) \
	(VALID_ROOM_RNUM(room) ? world[(room)].func : NULL)

/* char utils */
#define IN_ROOM(ch)	((ch)->in_room)
#define GET_WAS_IN(ch)	((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch)->year)

#define GET_PC_NAME(ch)	((ch)->player.name)
#define GET_NAME(ch)    (IS_NPC(ch) ? \
			 (ch)->player.short_descr : GET_PC_NAME(ch))
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_PASSWD(ch)	((ch)->player.passwd)
#define GET_EMAIL(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.email))
#define GET_PFILEPOS(ch)((ch)->pfilepos)

#define GET_PROMPT(ch)  CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.prompt))

/* I wonder if this definition of GET_REAL_LEVEL should be the definition of 
 * GET_LEVEL?  JE */
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_CLASS(ch)   ((ch)->player.chclass)
#define GET_HEIGHT(ch)	((ch)->player.height)
#define GET_WEIGHT(ch)	((ch)->player.weight)
#define GET_SEX(ch)	((ch)->player.sex)

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_ADD(ch)     ((ch)->aff_abils.str_add)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)

#define GET_EXP(ch)	  ((ch)->points.exp)
#define GET_AC(ch)        ((ch)->points.armor)
#define GET_HIT(ch)	  ((ch)->points.hit)
#define GET_MAX_HIT(ch)	  ((ch)->points.max_hit)
#define GET_MOVE(ch)	  ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)	  ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_GOLD(ch)	  ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)	  ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)

#define GET_POS(ch)	  ((ch)->char_specials.position)
#define GET_IDNUM(ch)	  ((ch)->char_specials.saved.idnum)
#define GET_ID(x)         ((x)->id)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)	  ((ch)->char_specials.fighting)
#define HUNTING(ch)	  ((ch)->char_specials.hunting)
#define GET_SAVE(ch, i)	  ((ch)->char_specials.saved.apply_saving_throw[i])
#define SET_SAVE(ch, i, pct) do { CHECK_PLAYER_SPECIAL((ch), ((ch)->char_specials.saved.apply_saving_throw[(i)]) = pct) } while(0)
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)

#define GET_COND(ch, i)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.conditions[(i)]))
#define GET_LOADROOM(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.load_room))
#define GET_PRACTICES(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.spells_to_learn))
#define GET_AFK_MSG(ch)         CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.afk_msg))
#define GET_INVIS_LEV(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.invis_level))
#define GET_WIMP_LEV(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.wimp_level))
#define GET_FREEZE_LEV(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.freeze_level))
#define GET_BAD_PWS(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.bad_pws))
#define GET_TALK(ch, i)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.talks[i]))
#define POOFIN(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->poofin))
#define POOFOUT(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->poofout))
#define GET_OLC_ZONE(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.olc_zone))
#define GET_LAST_OLC_TARG(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_targ))
#define GET_LAST_OLC_MODE(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_mode))
#define GET_ALIASES(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->aliases))
#define GET_WARNINGS(ch)        CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->warnings))
#define GET_LAST_TELL(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_tell))
#define GET_PREF(ch)      ((ch)->pref)
#define GET_HOST(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->host))
#define GET_HISTORY(ch, i)      CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.comm_hist[i]))
#define GET_PAGE_LENGTH(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.page_length))

#define GET_SKILL(ch, i)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.skills[i]))
#define SET_SKILL(ch, i, pct)	do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.skills[i]) = pct; } while(0)

#define GET_EQ(ch, i)		((ch)->equipment[i])

#define GET_MOB_SPEC(ch)	(IS_MOB(ch) ? mob_index[(ch)->nr].func : NULL)
#define GET_MOB_RNUM(mob)	((mob)->nr)
#define GET_MOB_VNUM(mob)	(IS_MOB(mob) ? \
				 mob_index[GET_MOB_RNUM(mob)].vnum : NOBODY)

#define GET_DEFAULT_POS(ch)	((ch)->mob_specials.default_pos)
#define MEMORY(ch)		((ch)->mob_specials.memory)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch) ==0) || (GET_STR(ch) != 20)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (100)
#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)
#define CAN_SEE_IN_DARK(ch) \
   (AFF_FLAGGED(ch, AFF_INFRAVISION) || (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_HOLYLIGHT)))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

/* These three deprecated. */
#define WAIT_STATE(ch, cycle) do { GET_WAIT_STATE(ch) = (cycle); } while(0)
#define CHECK_WAIT(ch)                ((ch)->wait > 0)
#define GET_MOB_WAIT(ch)      GET_WAIT_STATE(ch)
/* New, preferred macro. */
#define GET_WAIT_STATE(ch)    ((ch)->wait)

/* descriptor-based utils. We should make more. -gg */
#define STATE(d)	((d)->connected)

#define IS_PLAYING(d)   (STATE(d) == CON_TEDIT || STATE(d) == CON_REDIT ||      \
                        STATE(d) == CON_MEDIT || STATE(d) == CON_OEDIT ||       \
                        STATE(d) == CON_ZEDIT || STATE(d) == CON_SEDIT ||       \
                        STATE(d) == CON_CEDIT || STATE(d) == CON_PLAYING ||     \
                        STATE(d) == CON_TRIGEDIT || STATE(d) == CON_AEDIT ||    \
		        STATE(d) == CON_HEDIT)

#define SENDOK(ch)	(((ch)->desc || SCRIPT_CHECK((ch), MTRIG_ACT)) && \
			(to_sleeping || AWAKE(ch)) && \
			!PLR_FLAGGED((ch), PLR_WRITING))

/* object utils */
/* Check for NOWHERE or the top array index? If using unsigned types, the top 
 * array index will catch everything. If using signed types, NOTHING will 
 * catch the majority of bad accesses. */
#define VALID_OBJ_RNUM(obj)	(GET_OBJ_RNUM(obj) <= top_of_objt && \
				 GET_OBJ_RNUM(obj) != NOTHING)

#define GET_OBJ_LEVEL(obj)      ((obj)->obj_flags.level)
#define GET_OBJ_PERM(obj)       ((obj)->obj_flags.bitvector)
#define GET_OBJ_TYPE(obj)	((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)	((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)	((obj)->obj_flags.cost_per_day)
#define GET_OBJ_AFFECT(obj)	((obj)->obj_flags.bitvector)
#define GET_OBJ_EXTRA(obj)	((obj)->obj_flags.extra_flags)
#define GET_OBJ_EXTRA_AR(obj, i)   ((obj)->obj_flags.extra_flags[(i)])
#define GET_OBJ_WEAR(obj)	((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)	((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj)	((obj)->obj_flags.weight)
#define GET_OBJ_TIMER(obj)	((obj)->obj_flags.timer)
#define GET_OBJ_RNUM(obj)	((obj)->item_number)
#define GET_OBJ_VNUM(obj)	(VALID_OBJ_RNUM(obj) ? \
				obj_index[GET_OBJ_RNUM(obj)].vnum : NOTHING)
#define GET_OBJ_SPEC(obj)	(VALID_OBJ_RNUM(obj) ? \
				obj_index[GET_OBJ_RNUM(obj)].func : NULL)

#define IS_CORPSE(obj)		(GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
					GET_OBJ_VAL((obj), 3) == 1)

#define CAN_WEAR(obj, part)	OBJWEAR_FLAGGED((obj), (part))
#define GET_OBJ_SHORT(obj)      ((obj)->short_description)

/* Compound utilities and other macros. */
/* Used to compute version. To see if the code running is newer than 3.0pl13, 
 * you would use: #if _CIRCLEMUD > CIRCLEMUD_VERSION(3,0,13) */
#define CIRCLEMUD_VERSION(major, minor, patchlevel) \
       (((major) << 16) + ((minor) << 8) + (patchlevel))

#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouAEIOU", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouAEIOU", *(obj)->name) ? "an" : "a")

/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)	(!AFF_FLAGGED(sub, AFF_BLIND) && \
   (IS_LIGHT(IN_ROOM(sub)) || AFF_FLAGGED((sub), AFF_INFRAVISION)))

#define INVIS_OK(sub, obj) \
 ((!AFF_FLAGGED((obj),AFF_INVISIBLE) || AFF_FLAGGED(sub,AFF_DETECT_INVIS)) && \
 (!AFF_FLAGGED((obj), AFF_HIDE) || AFF_FLAGGED(sub, AFF_SENSE_LIFE)))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || (!IS_NPC(sub) && PRF_FLAGGED(sub, PRF_HOLYLIGHT)))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= (IS_NPC(obj) ? 0 : GET_INVIS_LEV(obj))) && \
   IMM_CAN_SEE(sub, obj)))
/* End of CAN_SEE */

#define INVIS_OK_OBJ(sub, obj) \
  (!OBJ_FLAGGED((obj), ITEM_INVISIBLE) || AFF_FLAGGED((sub), AFF_DETECT_INVIS))

/* Is anyone carrying this object and if so, are they visible? */
#define CAN_SEE_OBJ_CARRIER(sub, obj) \
  ((!obj->carried_by || CAN_SEE(sub, obj->carried_by)) &&	\
   (!obj->worn_by || CAN_SEE(sub, obj->worn_by)))

#define MORT_CAN_SEE_OBJ(sub, obj) \
  (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj) && CAN_SEE_OBJ_CARRIER(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   (MORT_CAN_SEE_OBJ(sub, obj) || (!IS_NPC(sub) && PRF_FLAGGED((sub), PRF_HOLYLIGHT)))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))

#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define EXIT(ch, door)  (world[IN_ROOM(ch)].dir_option[door])
#define W_EXIT(room, num)     (world[(room)].dir_option[(num)])
#define R_EXIT(room, num)     ((room)->dir_option[(num)])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
			 (EXIT(ch,door)->to_room != NOWHERE) && \
			 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : class_abbrevs[(int)GET_CLASS(ch)])


#define IS_MAGIC_USER(ch)       (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_MAGIC_USER || \
                                 GET_CLASS(ch) == CLASS_NECROMANCER || \
                                 GET_CLASS(ch) == CLASS_ELEMENTALIST || \
                                 GET_CLASS(ch) == CLASS_SORCERER))

#define IS_CLERIC(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_CLERIC || \
                                 GET_CLASS(ch) == CLASS_PRIEST || \
                                 GET_CLASS(ch) == CLASS_MYSTIC || \
                                 GET_CLASS(ch) == CLASS_SHAMAN || \
                                 GET_CLASS(ch) == CLASS_PALADIN))

#define IS_THIEF(ch)            (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_THIEF || \
                                 GET_CLASS(ch) == CLASS_ASSASSIN || \
                                 GET_CLASS(ch) == CLASS_ROGUE || \
                                 GET_CLASS(ch) == CLASS_NINJA))

#define IS_WARRIOR(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_WARRIOR || \
                                 GET_CLASS(ch) == CLASS_RANGER || \
                                 GET_CLASS(ch) == CLASS_KNIGHT || \
                                 GET_CLASS(ch) == CLASS_PALADIN))

#define IS_RANGER(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_RANGER))
#define IS_PALADIN(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_PALADIN))
#define IS_KNIGHT(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_KNIGHT))
#define IS_SHAMAN(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_SHAMAN))
#define IS_MYSTIC(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_MYSTIC))
#define IS_PRIEST(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_PRIEST))
#define IS_NINJA(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_NINJA))
#define IS_ASSASSIN(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_ASSASSIN))
#define IS_ROGUE(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_ROGUE))
#define IS_NECROMANCER(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_NECROMANCER))
#define IS_ELEMENTALIST(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_ELEMENTALIST))
#define IS_SORCERER(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_SORCERER))

#define M_IS_MAGIC_USER(ch)       (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_MAGIC_USER || \
                                 GET_MCLASS(ch) == CLASS_NECROMANCER || \
                                 GET_MCLASS(ch) == CLASS_ELEMENTALIST || \
                                 GET_MCLASS(ch) == CLASS_SORCERER))

#define M_IS_CLERIC(ch)           (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_CLERIC || \
                                 GET_MCLASS(ch) == CLASS_PRIEST || \
                                 GET_MCLASS(ch) == CLASS_MYSTIC || \
                                 GET_MCLASS(ch) == CLASS_SHAMAN))

#define M_IS_THIEF(ch)            (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_THIEF || \
                                 GET_MCLASS(ch) == CLASS_ASSASSIN || \
                                 GET_MCLASS(ch) == CLASS_ROGUE || \
                                 GET_MCLASS(ch) == CLASS_NINJA))

#define M_IS_WARRIOR(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_WARRIOR || \
                                 GET_MCLASS(ch) == CLASS_RANGER || \
                                 GET_MCLASS(ch) == CLASS_KNIGHT || \
                                 GET_MCLASS(ch) == CLASS_PALADIN))

#define M_IS_RANGER(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_RANGER))
#define M_IS_PALADIN(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_PALADIN))
#define M_IS_KNIGHT(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_KNIGHT))
#define M_IS_SHAMAN(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_SHAMAN))
#define M_IS_MYSTIC(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_MYSTIC))
#define M_IS_PRIEST(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_PRIEST))
#define M_IS_NINJA(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_NINJA))
#define M_IS_ASSASSIN(ch)       (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_ASSASSIN))
#define M_IS_ROGUE(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_ROGUE))
#define M_IS_NECROMANCER(ch)    (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_NECROMANCER))
#define M_IS_ELEMENTALIST(ch)   (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_ELEMENTALIST))
#define M_IS_SORCERER(ch)          (IS_NPC(ch) && \
                                (GET_MCLASS(ch) == CLASS_SORCERER))


#define OUTSIDE(ch) (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_INDOORS))

/* OS compatibility */
/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

#if !defined(YES)
#define YES 1
#endif

#if !defined(NO)
#define NO 0
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

/* NOCRYPT can be defined by an implementor manually in sysdep.h. CIRCLE_CRYPT 
 * is a variable that the 'configure' script automatically sets when it 
 * determines whether or not the system is capable of encrypting. */
#if defined(NOCRYPT) || !defined(CIRCLE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

/* Config macros */
#define CONFIG_CONFFILE             config_info.CONFFILE

#define CONFIG_PK_ALLOWED       config_info.play.pk_allowed
#define CONFIG_PT_ALLOWED       config_info.play.pt_allowed
#define CONFIG_LEVEL_CAN_SHOUT  config_info.play.level_can_shout
#define CONFIG_HOLLER_MOVE_COST config_info.play.holler_move_cost
#define CONFIG_TUNNEL_SIZE      config_info.play.tunnel_size
#define CONFIG_MAX_EXP_GAIN     config_info.play.max_exp_gain
#define CONFIG_MAX_EXP_LOSS     config_info.play.max_exp_loss
#define CONFIG_EXP_MULTIPLIER   config_info.play.exp_multiplier
#define CONFIG_MAX_NPC_CORPSE_TIME config_info.play.max_npc_corpse_time
#define CONFIG_MAX_PC_CORPSE_TIME config_info.play.max_pc_corpse_time
#define CONFIG_IDLE_VOID        config_info.play.idle_void
#define CONFIG_IDLE_RENT_TIME   config_info.play.idle_rent_time
#define CONFIG_IDLE_MAX_LEVEL   config_info.play.idle_max_level
#define CONFIG_DTS_ARE_DUMPS    config_info.play.dts_are_dumps
#define CONFIG_LOAD_INVENTORY   config_info.play.load_into_inventory
#define CONFIG_TRACK_T_DOORS    config_info.play.track_through_doors
#define CONFIG_NO_MORT_TO_IMMORT config_info.play.no_mort_to_immort
#define CONFIG_OK               config_info.play.OK
#define CONFIG_NOPERSON         config_info.play.NOPERSON
#define CONFIG_NOEFFECT         config_info.play.NOEFFECT
#define CONFIG_DISP_CLOSED_DOORS config_info.play.disp_closed_doors
#define CONFIG_PWD_RETRIEVAL    config_info.play.password_retrieval

/* Crash Saves */
#define CONFIG_FREE_RENT        config_info.csd.free_rent
#define CONFIG_MAX_OBJ_SAVE     config_info.csd.max_obj_save
#define CONFIG_MIN_RENT_COST    config_info.csd.min_rent_cost
#define CONFIG_AUTO_SAVE        config_info.csd.auto_save
#define CONFIG_AUTOSAVE_TIME    config_info.csd.autosave_time
#define CONFIG_CRASH_TIMEOUT    config_info.csd.crash_file_timeout
#define CONFIG_RENT_TIMEOUT     config_info.csd.rent_file_timeout

/* Room Numbers */
#define CONFIG_MORTAL_START     config_info.room_nums.mortal_start_room
#define CONFIG_IMMORTAL_START   config_info.room_nums.immort_start_room
#define CONFIG_FROZEN_START     config_info.room_nums.frozen_start_room
#define CONFIG_DON_ROOM_1       config_info.room_nums.donation_room_1
#define CONFIG_DON_ROOM_2       config_info.room_nums.donation_room_2
#define CONFIG_DON_ROOM_3       config_info.room_nums.donation_room_3

/* Game Operation */
#define CONFIG_DFLT_PORT        config_info.operation.DFLT_PORT
#define CONFIG_DFLT_IP          config_info.operation.DFLT_IP
#define CONFIG_MAX_PLAYING      config_info.operation.max_playing
#define CONFIG_MAX_FILESIZE     config_info.operation.max_filesize
#define CONFIG_MAX_BAD_PWS      config_info.operation.max_bad_pws
#define CONFIG_SITEOK_ALL       config_info.operation.siteok_everyone
#define CONFIG_OLC_SAVE         config_info.operation.auto_save_olc
#define CONFIG_NEW_SOCIALS      config_info.operation.use_new_socials
#define CONFIG_NS_IS_SLOW       config_info.operation.nameserver_is_slow
#define CONFIG_DFLT_DIR         config_info.operation.DFLT_DIR
#define CONFIG_LOGNAME          config_info.operation.LOGNAME
#define CONFIG_MENU             config_info.operation.MENU
#define CONFIG_WELC_MESSG       config_info.operation.WELC_MESSG
#define CONFIG_START_MESSG      config_info.operation.START_MESSG

/* Autowiz */
#define CONFIG_USE_AUTOWIZ      config_info.autowiz.use_autowiz
#define CONFIG_MIN_WIZLIST_LEV  config_info.autowiz.min_wizlist_lev

#define GET_COUNTDOWN(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.timer))
#define GET_QUESTMOB(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.questmob))
#define GET_QUESTOBJ(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.questobj))
#define GET_QUESTGIVER(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.questgiver))
#define GET_NEXTQUEST(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.nextquest))
#define GET_QKILLS(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.qkills))
#define GET_QUESTPOINTS(ch)  CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.questpoints))
#define GET_PLAYER_CREW(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.clan))
#define GET_DYING_TIME(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.dying_time))
#define GET_PAGE_LENGTH(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.page_length))
#define GET_SCREEN_WIDTH(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.screen_width))

#define GET_IRON_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.ironore))
#define GET_SILVER_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.silverore))
#define GET_GOLD_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.goldore))
#define GET_BRONZE_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.bronzeore))
#define GET_COPPER_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.copperore))
#define GET_TITANIUM_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.titaniumore))
#define GET_MITHRIL_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.mithrilore))
#define GET_ADAMANTITE_ORE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.adamantiteore))

#define GET_DIAMOND(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.diamond))
#define GET_CRYSTAL(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.crystal))
#define GET_JADE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.jade))
#define GET_RUBY(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.ruby))
#define GET_EMERALD(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.emerald))
#define GET_SAPPHIRE(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.sapphire))
#define GET_GARNET(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.garnet))
#define GET_TOPAZ(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.topaz))

#define GET_P_ALIGN_COL(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.p_align))
#define GET_P_TOLVL_COL(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.p_tolvl))
#define GET_P_XPTOT_COL(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.p_totxp))
#define GET_P_PKTYP_COL(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.p_pktyp))

#define GET_MINE_TIME(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.minetime))
#define GET_COOK_TIME(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.cook_time))
#define GET_CHOP_TIME(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.lumber_time))
#define GET_SCALE_TIME(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.scale_time))
#define GET_SKIN_TIME(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.skin_time))
#define GET_TAN_TIME(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.tan_time))

#define GET_PLAYER_KILLS(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.player_kills))
#define GET_CPKS(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.cpks))
#define GET_MONSTER_KILLS(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.monster_kills))
#define GET_PLAYER_DEATHS(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.player_deaths))
#define GET_DEATHS(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.deaths))
#define GET_SOULS(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.souls))
#define GET_DEFEND(ch)      CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.defend))

#define GET_CREW_NUM(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.clan))
#define GET_CREW_LEADER(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.crewleader))

#define GET_WAS_MAGE(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_mage))
#define GET_WAS_CLERIC(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_cleric))
#define GET_WAS_THIEF(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_thief))
#define GET_WAS_WARRIOR(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_warrior))
#define GET_WAS_RANGER(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_ranger))
#define GET_WAS_PALADIN(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_paladin))
#define GET_WAS_KNIGHT(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_knight))
#define GET_WAS_MYSTIC(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_mystic))
#define GET_WAS_PRIEST(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_priest))
#define GET_WAS_SHAMAN(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_shaman))
#define GET_WAS_NECROMANCER(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_necromancer))
#define GET_WAS_ELEMENTALIST(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_elementalist))
#define GET_WAS_SORCERER(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_sorcerer))
#define GET_WAS_NINJA(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_ninja))
#define GET_WAS_ASSASSIN(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_assassin))
#define GET_WAS_ROGUE(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.was_rogue))

#define GET_RET_DAM(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.retribution_dam))
#define GET_RECALL(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.recall_room))
#define GET_META_HP(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.metahp))
#define GET_META_MANA(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.metamana))
#define GET_META_MV(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.metamove))
#define GET_ALTAR(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.altar_room))
#define GET_PKBLOOD(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.pkblood))

#define GET_DAMD(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.damdealt))
#define GET_DAMT(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.damtaken))
#define GET_TOT_DAMD(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.totdamdealt))
#define GET_TOT_DAMT(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.totdamtaken))

#define GET_RECALLX(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.recall_x))
#define GET_RECALLY(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.recall_y))
#define GET_ALTARX(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.altar_x))
#define GET_ALTARY(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.altar_y))



#define GET_SCREEN_WIDTH(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.screen_width))

#define GET_ROOM_IN_BEFORE_PARTY(ch)        CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.room_in_before_party))

#define GET_CLAN(ch)            ((ch)->player.clan)
#define GET_CLAN_RANK(ch)       ((ch)->player.rank)
#define GET_OBJ_COND(obj)       ((obj)->obj_flags.condition)
#define GET_OBJ_NAME(obj)       ((obj)->name)

#define GET_ROOM_TRIPPER(loc)   (world[(loc)].tripper)

#define GET_FORM_POS(ch)        ((ch)->char_specials.form_pos)
#define GET_FORM_TOTAL(ch, i)   ((ch)->char_specials.form_total[i])
#define GET_FORM_TYPE(ch)       ((ch)->char_specials.form_type)


#define GET_HOME(ch)    ((ch)->player.hometown)

#define EXIT2(roomnum, door) (world[(roomnum)].dir_option[door])
#define CAN_GO2(roomnum, door) (EXIT2(roomnum, door) && \
                       (EXIT2(roomnum, door)->to_room != NOWHERE) && \
                       !IS_SET(EXIT2(roomnum,door)->exit_info, EX_CLOSED))

#define GET_BETTED_ON(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.betted_on))
#define GET_BET_AMT(ch)      CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.bet_amt))

#define GET_NEWS(ch)      CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.current_news))
#define GET_GNEWS(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.current_gnews))

#define GET_PROF(ch, i)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.prof[i]))
#define SET_PROF(ch, i, pct)   do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.prof[i]) = pct; } while(0)

#define GET_LUMBER_TYPE(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.lumber_type))
#define GET_LUMBER(ch, i)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.lumber[i]))
#define SET_LUMBER(ch, i, pct)   do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.lumber[i]) = pct; } while(0)

#define GET_SKINS(ch, i)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.skins[i]))
#define SET_SKINS(ch, i, pct)   do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.skins[i]) = pct; } while(0)

#define GET_SCALES(ch, i)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.scales[i]))
#define SET_SCALES(ch, i, pct)   do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.scales[i]) = pct; } while(0)

#define GET_LEATHER(ch, i)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.leathers[i]))
#define SET_LEATHER(ch, i, pct)   do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.leathers[i]) = pct; } while(0)

#define HAS_HIGHSECURITY(ch)  ((!strcmp(GET_NAME(ch), "Slurk")) || (!strcmp(GET_NAME(ch), "Bluntmaster")) || (!strcmp(GET_NAME(ch), "Mixologee")))
#define HAS_MEDSECURITY(ch)   (!strcmp(GET_NAME(ch), "Anubis"))
#define HAS_LOWSECURITY(ch)   (!strcmp(GET_NAME(ch), "Fizban"))

#define GET_DESTINATION(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.crygate_destination))
#define GET_DESTINATIONX(ch)  CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.crygate_destinationx))
#define GET_DESTINATIONY(ch)  CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.crygate_destinationy))
#define GET_TRAVELTIME(ch)    CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.crygate_traveltime))

#define GET_LOGS(ch)          CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.logs))

#define GET_SECURITY(ch)   ((ch)->player_specials->saved.security)
#define SET_SECURITY(ch, lvl) do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.security) = lvl; } while(0)

#define IS_CODER(ch)  (!strcmp((ch)->player.name, "Slurk") || !strcmp((ch)->player.name, "Bluntmaster"))

#define GET_MAPX(ch) CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.maprow))
#define GET_MAPY(ch) CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.mapcol))

#define GET_BALANCE(ch)   ((ch)->points.bank_gold)
#define GET_SHARES(ch, i)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.shares[i]))
#define SET_SHARES(ch, i, pct)   do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.shares[i]) = pct; } while(0)

/* Autoquests data */
#define GET_QUEST(ch)           CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.current_quest))
#define GET_QUEST_COUNTER(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.quest_counter))
#define GET_QUEST_TIME(ch)      CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.quest_time))
#define GET_NUM_QUESTS(ch)      CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.num_completed_quests))

#define GET_QUEST_TYPE(ch)      (real_quest(GET_QUEST((ch))) != NOTHING ? aquest_table[real_quest(GET_QUEST((ch)))].type : AQ_UNDEFINED )

#define GET_ATTACK1(mob)  ((mob)->mob_specials.attack1)
#define GET_ATTACK2(mob)  ((mob)->mob_specials.attack2)
#define GET_ATTACK3(mob)  ((mob)->mob_specials.attack3)
#define GET_MCLASS(mob)   ((mob)->mob_specials.class)

#define URANGE(a, b, c)          ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))

#define IS_DRAGON(mob)           (IS_NPC(mob) && (GET_MCLASS(mob) == MCLASS_BLACKDRAGON || \
                                 GET_MCLASS(mob) == MCLASS_BLUEDRAGON || GET_MCLASS(mob) == MCLASS_REDDRAGON || \
                                 GET_MCLASS(mob) == MCLASS_GREENDRAGON || GET_MCLASS(mob) == MCLASS_GOLDDRAGON || \
                                 GET_MCLASS(mob) == MCLASS_RAND_DRAGON))  

#define RIDING(ch)	  ((ch)->char_specials.riding)		// (DAK)
#define RIDDEN_BY(ch)	  ((ch)->char_specials.ridden_by)	// (DAK)

#define GET_IMAGES(ch)           CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.images))

#define GET_ETHOS(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.ethos))
#define GET_DEITY(ch)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.deity))

#define GET_OBJ_BOUND_ID(obj)  ((obj)->obj_flags.bound_id)
#define GET_MOB_BOUND_ID(mob)  ((mob)->mob_specials.bound_id)


//TEST FOR SURVEY

struct Survey {
        int X;
        int Y;
        int dist;
        char *description;
        sh_int Flags; /*For Future Expansion */
        struct Survey *next;
};

/* Mail Stuff */
char    *add_commas(long X);
#define MOB_MAIL(ch)           ((ch)->mob_specials.ml_list)
#define IN_MAIL(obj)   ((obj)->in_mail)

/* Mudmail Configuraton */
#define CONFIG_CAN_MAIL          config_info.mail.mail_allowed
#define CONFIG_CAN_MAIL_OBJ      config_info.mail.objects_allowed
#define CONFIG_CAN_MAIL_GOLD     config_info.mail.gold_allowed
#define CONFIG_STAMP_COST        config_info.mail.stamp_cost
#define CONFIG_OBJECT_COST       config_info.mail.object_cost
#define CONFIG_MIN_MAIL_LEVEL    config_info.mail.min_level
#define CONFIG_FREE_MAIL_LEVEL   config_info.mail.min_free_level
#define CONFIG_DRAFTS_ALLOWED    config_info.mail.allow_drafts
#define CONFIG_DRAFT_TIMEOUT     config_info.mail.draft_timeout
#define CONFIG_SAFE_MAILING      config_info.mail.safe_mailing
#define CONFIG_MIN_MAIL_ANYWHERE config_info.mail.min_mail_anywhere
#define CONFIG_MIN_SEND_TO_ALL   config_info.mail.min_send_to_all

/*   LYO STUFF   */

EXIT_DATA *find_exit( ROOM_DATA *room, sh_int dir );
EXIT_DATA *get_exit( ROOM_DATA *room, sh_int dir );
EXIT_DATA *get_exit_to( ROOM_DATA *room, sh_int dir, int vnum );
EXIT_DATA *get_exit_num( ROOM_DATA *room, sh_int count );
EXIT_DATA *get_exit_to_coord( ROOM_DATA *room, sh_int dir, COORD_DATA *coord );
EXIT_DATA *make_exit( ROOM_DATA *pRoom, ROOM_DATA *to_room, sh_int door );
bool    valid_exit( EXIT_DATA *pexit );
void    extract_exit( ROOM_DATA *room, EXIT_DATA *pexit );
bool    water_sector(int sect);


/* vehicles.c */
VEHICLE_DATA *find_vehicle(char *name);
VEHICLE_DATA *find_vehicle_in_room_by_name(CHAR_DATA *ch, char *name);
OBJ_DATA *obj_to_vehicle(OBJ_DATA *obj, VEHICLE_DATA *vehicle);
bool    vehicle_can_move(VEHICLE_DATA *vehicle);
bool    vehicle_can_go(VEHICLE_DATA *pVeh, ROOM_DATA *to_room);
bool    hitched_mob(CHAR_DATA *ch);
void    list_vehicle_to_char(VEHICLE_DATA *vlist, CHAR_DATA *ch);
void    vehicle_to_room(VEHICLE_DATA *pVeh, ROOM_DATA *pRoom);
void    vehicle_from_room(VEHICLE_DATA *pVeh);
void    obj_from_vehicle(OBJ_DATA *obj);
void    char_to_vehicle(CHAR_DATA *ch, VEHICLE_DATA *vehicle);
void    char_from_vehicle(CHAR_DATA *ch);
void    stop_be_wagoner(CHAR_DATA *ch);

#define IS_BUILDING(sub)                        ((sub)->zone == BUILDING_ZONE)
//#define IN_BUILDING(sub)                        (IS_BUILDING(IN_ROOM(sub)))
#define IN_BUILDING(ch)                         (ROOM_FLAGGED(IN_ROOM(ch), ROOM_BUILDING_WORKS))

#define IS_WILD(sub)                            ((sub)->zone == WILD_ZONE)
//#define IN_WILD(sub)                            (IS_WILD((sub)->in_room ))
#define IN_WILD(ch)                             (ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS)) 

#define IS_SHIP(sub)                            ((sub)->zone == SHIP_ZONE)
#define IN_SHIP(sub)                            (IS_SHIP(IN_ROOM(sub)))

#define IS_DECK(sub)                            (SECT((sub)) >= SECT_SHIP_STERN && SECT((sub)) <= SECT_SHIP_BOW)
#define ON_DECK(sub)                            (IS_DECK((sub)->in_room))

#define IS_FERRY(sub)                           ((sub)->zone == FERRY_ZONE)
//#define IN_FERRY(sub)                           (IS_FERRY(IN_ROOM(sub)))
#define IN_FERRY(ch)                             (world[IN_ROOM(ch)].sector_type == SECT_FERRY_DECK)

#define S_NAME(sn)                              (terrain_type[(sn)] != NULL ? terrain_type[(sn)]->name : "WildRoomName")
#define S_DESC(sn)                              (terrain_type[(sn)] != NULL ? terrain_type[(sn)]->description : "WildRoomDesc")


#define ROOM_RNAME(sub)                         (IS_WILD(sub) ? S_NAME((sub)->sector_type ) : \
                                                                      ((sub)->name ? (sub)->name : "NoName"))
#define ROOM_RDESC(sub)                         (IS_WILD(sub) ? S_DESC((sub)->sector_type ) : \
                                                                      ((sub)->description ? (sub)->description : \
                                                                                        "NoDescription"))
#define ROOM_NAME(sub)                          (ROOM_RNAME((sub)->in_room))
#define ROOM_DESC(sub)                          (ROOM_RDESC((sub)->in_room))


#define MOB_OWNER(mob)                          ((mob)->mob_specials.owner_id)
#define GET_RY(sub)                                     (world[sub].y)
#define GET_RX(sub)                                     (world[sub].x)
#define GET_RZ(sub)                                     ((sub)->coord->level)
#define GET_Y(sub)                                      (world[IN_ROOM(sub)].y)
#define GET_X(sub)                                      (world[IN_ROOM(sub)].x)
#define GET_Z(sub)                                      (GET_RZ((sub)->in_room))

#define BUILDING_FLAGS(bld)                     ((bld)->flags)
#define BUILDING_FLAGGED(bld, flag)     (IS_SET(BUILDING_FLAGS(bld), (flag)))

#define SHIP_FLAGS(ship)                        ((ship)->flags)
#define SHIP_FLAGGED(ship, flag)        (IS_SET(SHIP_FLAGS(ship), (flag)))

#define EXIT_FLAGS(loc)                         ((loc)->exit_info)

#define IS_GOD(ch)                                      (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_GOD))
#define IS_IMMORTAL(ch)                         (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_IMMORT))
#define IS_MORTAL(ch)                           (GET_LEVEL(ch) < LVL_IMMORT)

#define WAGONER(ch)                                     ((ch)->player.drive_vehicle)
#define GET_REAL_HITROLL(ch)            (str_app[GET_STR(ch)].tohit + GET_HITROLL(ch))
#define GET_REAL_DAMROLL(ch)            (str_app[GET_STR(ch)].todam + GET_DAMROLL(ch))

#define GET_LOADBUILDING(ch)            CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_building))
#define GET_LOADCOORD(ch)                       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_coord))
#define GET_LOAD_Y(ch)                          CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_coord->y))
#define GET_LOAD_X(ch)                          CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_coord->x))
#define GET_LOADSHIP(ch)                        CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_ship))

#define GET_OBJ_QUALITY(obj)         ((obj)->obj_flags.quality)

#define GET_LOADBUILDING(ch)            CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_building))
#define GET_LOADCOORD(ch)                       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_coord))
#define GET_LOAD_Y(ch)                          CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_coord->y))
#define GET_LOAD_X(ch)                          CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_coord->x))
#define GET_LOADSHIP(ch)                        CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->load_ship))

#define ROOM_NAME(sub)                          (ROOM_RNAME((sub)->in_room))
#define ROOM_DESC(sub)                          (ROOM_RDESC((sub)->in_room))

#define IS_SAME_GROUP(i,j) 		((i) && (j) && AFF_FLAGGED((i), AFF_GROUP) && \
				        AFF_FLAGGED((j), AFF_GROUP) && ((i)->master || (j)->master) && \
       					(((i) == (j)->master) || ((i)->master == (j)) || ((i)->master == (j)->master))) 

/* ========================== S M A U G ============================ */
#define QUICKLINK(point)        str_dup((point))
#define QUICKMATCH(p1, p2)      ((p1) && (p2) && (strcmp((p1), (p2)) == 0))

#define DISPOSE(point)                                                                                  \
do                                                                                                                              \
{                                                                                                                               \
  if (!(point))                                                                                                 \
  {                                                                                                                             \
        log( "Freeing null pointer %s:%d", __FILE__, __LINE__ );        \
        fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }                                                                                                                             \
  else free(point);                                                                                             \
  point = NULL;                                                                                                 \
} while(0)

#define STRFREE(point)                                                                                  \
do                                                                                                                              \
{                                                                                                                               \
  if (!(point))                                                                                                 \
  {                                                                                                                             \
        log( "Freeing null pointer %s:%d", __FILE__, __LINE__ );        \
        fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }                                                                                                                             \
  else free((point));                                                                                   \
  point = NULL;                                                                                                 \
} while(0)

/* double-linked list handling macros -Thoric */

#define LINK(link, first, last, next, prev)                                             \
do                                                                                                                              \
{                                                                                                                               \
    if ( !(first) )                                                                                             \
      (first)                                   = (link);                                               \
    else                                                                                                                \
      (last)->next                              = (link);                                               \
    (link)->next                                = NULL;                                                 \
    (link)->prev                                = (last);                                               \
    (last)                                              = (link);                                               \
} while(0)

#define INSERT(link, insert, first, next, prev)                                 \
do                                                                                                                              \
{                                                                                                                               \
    (link)->prev                                = (insert)->prev;                               \
    if ( !(insert)->prev )                                                                              \
      (first)                                   = (link);                                               \
    else                                                                                                                \
      (insert)->prev->next              = (link);                                               \
    (insert)->prev                              = (link);                                               \
    (link)->next                                = (insert);                                             \
} while(0)

#define UNLINK(link, first, last, next, prev)                                   \
do                                                                                                                              \
{                                                                                                                               \
    if ( !(link)->prev )                                                                                \
      (first)                                   = (link)->next;                                 \
    else                                                                                                                \
      (link)->prev->next                = (link)->next;                                 \
    if ( !(link)->next )                                                                                \
      (last)                                    = (link)->prev;                                 \
    else                                                                                                                \
      (link)->next->prev                = (link)->prev;                                 \
} while(0)






