/* ************************************************************************
*   File: spells.h                                                        *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
************************************************************************ */

#define DEFAULT_STAFF_LVL	12
#define DEFAULT_WAND_LVL	12

#define CAST_UNDEFINED	(-1)
#define CAST_SPELL	0
#define CAST_POTION	1
#define CAST_WAND	2
#define CAST_STAFF	3
#define CAST_SCROLL	4

#define MAG_DAMAGE	(1 << 0)
#define MAG_AFFECTS	(1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS	(1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS	(1 << 5)
#define MAG_MASSES	(1 << 6)
#define MAG_AREAS	(1 << 7)
#define MAG_SUMMONS	(1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL	(1 << 10)
#define BREATH_DAMAGE   (1 << 11)

#define TYPE_UNDEFINED               (-1)
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */

/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */

#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                   7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOR_SPRAY            10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROT_FROM_EVIL         34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD	     45 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD	     46 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_ARMOR	     47 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL	     48 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL	     49 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION	     50 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERWALK		     51 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_AURA_BOLT              52
#define SPELL_MANA_BLAST             53
#define SPELL_SONIC_BLAST            54
#define SPELL_DISPEL_MAGIC           55
#define SPELL_REVIVAL                56
#define SPELL_CLAN_RECALL            57
#define SPELL_HURRICANE              58
#define SPELL_EAGLE_FLIGHT           59
#define SPELL_CONJURE_BEAST          60
#define SPELL_WHIRLWIND              61
#define SPELL_CONJURE_TREE           62
#define SPELL_RAT_SWARM              63
#define SPELL_HELLSTORM              64
#define SPELL_HAMMER_OF_DIVINITY     65
#define SPELL_STARFIRE               66
#define SPELL_ADVANCED_PHASE         67
#define SPELL_ADVANCED_SUMMON        68
#define SKILL_ADVANCED_HEAL          69
#define SPELL_MASS_HEAL              70
#define SPELL_MASS_SANCTUARY         71
#define SPELL_REGEN                  72
#define SPELL_MASS_REMOVE_CURSE      73
#define SPELL_DEATH_FIELD            74
#define SPELL_DRAIN_LIFE             75
#define SPELL_CONTROL_UNDEAD         76
#define SPELL_CONFUSION              77
#define SPELL_MASS_CURSE             78

//SORCERER SPELLS
#define SPELL_SHOCKSTORM             79
#define SPELL_BLIZZARD               80
#define SPELL_MASS_ENERGY_DRAIN      81
#define SPELL_MASS_PLAGUE            82
#define SPELL_MANASHIELD             83
#define SPELL_METEOR_STORM           84

//ELEMENTALIST SPELLS
#define SPELL_SUMMON_CELESTIAL       85
#define SPELL_CONJURE_ELEMENTAL      86
#define SPELL_CONJURE_FAMILIAR       87
#define SPELL_ADVANCED_CHARM         88
#define SPELL_CLONE_IMAGES           89
#define SPELL_MAJOR_CREATION         90
#define SPELL_SUMMON_FIEND           91
#define SPELL_MELEEHEAL              92
#define SPELL_EVASION                93
#define SPELL_CLOAKED_MOVE           94
#define SKILL_ENTANGLE               95
#define SPELL_RETRIBUTION            96

#define SPELL_PLAGUE                 97
#define SPELL_REFRESH                98
#define SPELL_MASS_REFRESH           99
#define SPELL_LIFETAP               100

/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_BACKSTAB              101 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                  102 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                  103 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                  104 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK             105 /* Reserved Skill[] DO NOT CHANGE */
/* Undefined                        136 */
#define SKILL_RESCUE                106 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                 107 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                 108 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK		    109 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_ATTACK2               110  
#define SKILL_ATTACK3               111
#define SKILL_SECOND_STAB           112 /* Second stab    */
#define SKILL_TRI_STAB	            113 /* Third stab     */
#define SKILL_FOURTH_STAB	    114 /* fourth stab    */
#define SKILL_SCAN                  115
#define SKILL_DISARM                116
#define SKILL_STOMP                 117
#define SKILL_RANGE_COMBAT          118
#define SKILL_TRIP                  119

//knight skills
#define SKILL_ATTACK4               120
#define SKILL_ADVANCED_KICK         121
#define SKILL_ADVANCED_PARRY        122
#define SKILL_ADVANCED_CHARGE       123
#define SKILL_ATTACK5               124
#define SKILL_PARRY                 125
#define SPELL_HEROISM               126

//RANGER SKILLS 
#define SKILL_REVEAL                127
#define SKILL_ADVANCED_MEDITATE     128
#define SKILL_SENSE                 129  
#define SKILL_SUMMON_MOUNT          130

//PALADIN SKILLS
#define SPELL_PROT_FROM_GOOD        131
#define SPELL_VENGEANCE             132
#define SPELL_DIVINE_PROT           133
#define SPELL_COURAGE               134

//ROGUE SKILLS
#define SKILL_ADVANCED_STEAL        135
#define SKILL_ADVANCED_TRIP         136
#define SKILL_TRIPWIRE              137
#define SKILL_DUAL_THROW            138

//ASSASSIN SKILLS
#define SKILL_ADVANCED_HIDE         139
#define SKILL_ADVANCED_DODGE        140
#define SKILL_ADVANCED_THROW        141
#define SKILL_ADVANCED_BACKSTAB     142
#define SKILL_AMBUSH                143
#define SKILL_ADVANCED_TRACK        144

#define SKILL_CHARGE                145
#define SKILL_DODGE                 146
#define SKILL_THROW                 147

// DRAGON SKILLS

#define SKILL_RAKE                  148 // rakes claws against player
#define SKILL_MAUL		    149 // mauls player with mouth, bite like attack
#define SKILL_TAILWHIP              150 // uses tail to whip/smash players
#define SKILL_ROAR		    151 // roars at players, doing something like room_aff faerie fire
#define SKILL_WINGFLAP		    152 // wind attack that knocks players over
#define SKILL_BITEDEAD		    153 // VERY SMALL % chance that mob can bite player in half/ raw_death

#define SKILL_MOUNT		    154 // Mounting (DAK)
#define SKILL_RIDING		    155 // Riding (DAK)
#define SKILL_TAME		    156 // Ability to tame (DAK)
#define SPELL_KNOW_ALIGNMENT        157
#define SPELL_RECHARGE              158
#define SKILL_NAVIGATION            159
#define SKILL_ADVANCED_BASH         160
#define SKILL_DEFEND                161
#define SKILL_DEATHWISH		    162
#define SKILL_GREATER_SENSES        163
#define SPELL_STONESKIN             164
#define SKILL_CRIPPLE               165
#define SKILL_COUNTERATTACK         166
#define SPELL_QUICKNESS             167
#define SPELL_MASS_QUICK            168
#define SPELL_REFLECT               169
#define SKILL_DUAL_WIELD            170
#define MAX_SPELLS                  171
/* New skills may be added here up to MAX_SKILLS (200) */


/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with objects (such as SPELL_IDENTIFY used with scrolls of
 *  identify) or non-players (such as NPC-only spells).
 */

#define SPELL_IDENTIFY               201
#define SPELL_FIRE_BREATH            202
#define SPELL_GAS_BREATH             203
#define SPELL_FROST_BREATH           204
#define SPELL_ACID_BREATH            205
#define SPELL_LIGHTNING_BREATH       206
#define SPELL_SPARKSHIELD            207

#define SPELL_DG_AFFECT              298 
#define TOP_SPELL_DEFINE	     299
/* NEW NPC/OBJECT SPELLS can be inserted here up to 299 */


/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     300
#define TYPE_STING                   301
#define TYPE_WHIP                    302
#define TYPE_SLASH                   303
#define TYPE_BITE                    304
#define TYPE_BLUDGEON                305
#define TYPE_CRUSH                   306
#define TYPE_POUND                   307
#define TYPE_CLAW                    308
#define TYPE_MAUL                    309
#define TYPE_THRASH                  310
#define TYPE_PIERCE                  311
#define TYPE_BLAST		     312
#define TYPE_PUNCH		     313
#define TYPE_STAB		     314

#define TYPE_COUNTER                 397
#define TYPE_REFLECT                 398
/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SUFFERING		     399



#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4
#define MAX_SAVING    5


#define TAR_IGNORE      (1 << 0)
#define TAR_CHAR_ROOM   (1 << 1)
#define TAR_CHAR_WORLD  (1 << 2)
#define TAR_FIGHT_SELF  (1 << 3)
#define TAR_FIGHT_VICT  (1 << 4)
#define TAR_SELF_ONLY   (1 << 5) /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF   	(1 << 6) /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     (1 << 7)
#define TAR_OBJ_ROOM    (1 << 8)
#define TAR_OBJ_WORLD   (1 << 9)
#define TAR_OBJ_EQUIP	(1 << 10)

struct spell_info_type {
   byte min_position;	/* Position for caster	 */
   int mana_min;	/* Min amount of mana used by a spell (highest lev) */
   int mana_max;	/* Max amount of mana used by a spell (lowest lev) */
   int mana_change;	/* Change in mana used by spell from lev to lev */

   int min_level[NUM_CLASSES];
   int routines;
   byte violent;
   int targets;         /* See below for use with TAR_XXX  */
   const char *name;	/* Input size not limited. Originates from string constants. */
   const char *wear_off_msg;	/* Input size not limited. Originates from string constants. */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
   const char	*singular;
   const char	*plural;
};


#define ASPELL(spellname) \
void	spellname(int level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj)

#define MANUAL_SPELL(spellname)	spellname(level, caster, cvict, ovict);

ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_teleport);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_dispel_magic);
ASPELL(spell_revival);
ASPELL(spell_clan_recall);
ASPELL(spell_know_alignment);
ASPELL(spell_recharge);

/* basic magic calling functions */
extern char *spell_check[];

int find_skill_num(char *argument,int begin,int length,char **list,int mode);

int breath_damage(int level, struct char_data *ch, struct char_data *victim, int spellnum, int savetype);

int mag_damage(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_groups(int level, struct char_data *ch, int spellnum, int savetype);

void mag_masses(int level, struct char_data *ch, int spellnum, int savetype);

void mag_areas(int level, struct char_data *ch, int spellnum, int savetype);

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
 int spellnum, int savetype);

void mag_points(int level, struct char_data *ch, struct char_data *victim,
 int spellnum, int savetype);

void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int type);

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
  int spellnum, int type);

void mag_creations(int level, struct char_data *ch, int spellnum);

int	call_magic(struct char_data *caster, struct char_data *cvict,
  struct obj_data *ovict, int spellnum, int level, int casttype);

void	mag_objectmagic(struct char_data *ch, struct obj_data *obj,
			char *argument);

int	cast_spell(struct char_data *ch, struct char_data *tch,
  struct obj_data *tobj, int spellnum);


/* other prototypes */
void spell_level(int spell, int chclass, int level);
void init_spell_levels(void);
const char *skill_name(int num);
