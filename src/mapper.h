/* Defines for ASCII Automapper */

#define ACSIIMAPX     10
#define ACSIIMAPY      8
/* You can change MAXDEPTH to 1 if the diagonal directions are confusing */
#define MAXDEPTH  2

#define BOUNDARY(x, y) (((x) < 0) || ((y) < 0) || ((x) > ASCIIMAPX) || ((y) > ASCIIMAPY))

typedef struct  map_type                MAP_TYPE;

#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/* Structure for the map itself */
struct map_type
{
  char   tegn;  /* Character to print at this map coord */
  int    vnum;  /* Room this coord represents */
  int    depth; /* Recursive depth this coord was found at */
  EXT_BV    info;
  bool   can_see;
};

/* mapper.c */
void  draw_map (struct char_data *ch, const char *desc );
char *get_exits(struct char_data *ch);




