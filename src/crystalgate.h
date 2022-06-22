///////////////////////////////////////////////////////////////////////////////////
////                   The Realms of Kuvia Codebase                            ////
///////////////////////////////////////////////////////////////////////////////////
////                                                                           ////
//// File : crystalgate.h                                                      ////
////                                                                           ////
//// Usage: full cross world transportation system                             ////
////                                                                           ////
///////////////////////////////////////////////////////////////////////////////////

/* symbol definitions */
#define SYMBOL_NONE         0
#define SYMBOL_RUBY         1
#define SYMBOL_EMERALD      2
#define SYMBOL_DIAMOND      3
#define SYMBOL_JADE         4
#define SYMBOL_LAPIS        5
#define SYMBOL_AMETHYST     6
#define SYMBOL_SAPPHIRE     7
#define SYMBOL_CITRINE      8
#define SYMBOL_AQUAMARINE   9
#define SYMBOL_CARNELIAN   10
#define MAX_SYMBOL         11


/* set the room to be the holding pen for travelers */
#define CRYGATE_ROOM        5 

/* set this as the room for messages to cycle through */
#define CRYGATE_ROOM_MSGS real_room(5)

/* set below to max # of gates ingame, everytime we add one, make this number +1 */
#define MAX_GATES 	   50 /* 34 gates in wilderness randomly spread out define for min_level for gates */

#define NO_MIN_LEVEL        1


/* table structures */

struct crystalgate_type {
    char    *name;      /* Name to where it goes        */
    int     vnum;       /* What vnum their location is  */
    int     symbol[3];  /* the 3 symbols used travel    */
    int     min_level;  /* Min level to activate it     */
    bool    disabled;   /* is it disabled?              */
    bool    to_wild;    /* Does it go to the wildnerness*/
    int     x;          /* if to_wild, what X           */
    int     y;          /* if to_wild, what Y           */
};

struct symbol_type {
    char    *name;		/* symbol name */
    int     symbol;		/* symbol define # */
    char    *press_string;	/* glow txt for symbol */
};


extern const struct crystalgate_type crystalgate_table [];
extern const struct symbol_type symbol_table [];

/* function declarations */
/*
void cmd_press			(char_data *ch, char *argument);
void fix_facets			(char_data *ch, obj_data *obj, int symbol);
void reset_crystalgate		(char_data *ch, obj_data *obj);
void check_valid_sequence	(char_data *ch, obj_data *obj);
void cmd_crystalgate		(char_data *ch, char *argument);
char * return_gatesymbol	(int symbol);
*/
