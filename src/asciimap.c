/**************************************************************************
 * *  File: asciimap.c                                        Part of tbaMUD *
 * *  Usage: Generates an ASCII map of the player's surroundings.            *
 * *                                                                         *
 * *  All rights reserved.  See license for complete information.            *
 * *                                                                         *
 * *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 * *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 * **************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "constants.h"
#include "dg_scripts.h"
#include "asciimap.h"

/******************************************************************************
 *  * Begin Local (File Scope) Defines and Global Variables
 *   *****************************************************************************/
/* Do not blindly change these values, as many values cause the map to stop working - backup first */
#define CANVAS_HEIGHT 19
#define CANVAS_WIDTH  51
#define LEGEND_WIDTH  15

#define DEFAULT_MAP_SIZE CONFIG_MAP_SIZE

#define MAX_MAP_SIZE (CANVAS_WIDTH - 1)/4
#define MAX_MAP      CANVAS_WIDTH

#define MAX_MAP_DIR 6
#define MAX_MAP_FOLLOW 4

#define SECT_EMPTY 30 /* anything greater than num sect types */
#define SECT_STRANGE (SECT_EMPTY + 1)
#define SECT_HERE  (SECT_STRANGE + 1)

#define DOOR_NS   -1
#define DOOR_EW   -2
#define DOOR_UP   -3
#define DOOR_DOWN -4
#define VDOOR_NS  -5
#define VDOOR_EW  -6
#define DOOR_NONE -7
#define NUM_DOOR_TYPES 7

#define MAP_CIRCLE    0
#define MAP_RECTANGLE 1

#define MAP_NORMAL  0
#define MAP_COMPACT 1

struct map_info_type
{
  int  sector_type;
  char disp[20];
};

static struct map_info_type door_info[] =
{
  { DOOR_NONE, " " },
  { VDOOR_EW,  "&m+&n" },
  { VDOOR_NS,  "&m+&n"},
  { DOOR_DOWN, "&r-&n" },
  { DOOR_UP,   "&r+&n" },
  { DOOR_EW,   "-" },
  { DOOR_NS,   "|" }
};

static struct map_info_type compact_door_info[] =
{
  { DOOR_NONE, " " },
  { VDOOR_EW,  "&m+&n" },
  { VDOOR_NS,  "&m+&n"},
  { DOOR_DOWN, "&r-&n" },
  { DOOR_UP,   "&r+&n" },
  { DOOR_EW,   "-" },
  { DOOR_NS,   "|" }
};

/* Add new sector types below for both map_info and world_map_info     */
/* The last 3 MUST remain the same, although the symbol can be changed */
/* New sectors also need to be added to the perform_map function below */
struct map_info_type map_info[] =
{
 {SECT_INSIDE,            "&c[&n.&c]&n"},       /*  0 */
 {SECT_CITY,              "&c[&wC&c]&n"},
 {SECT_FIELD,             "&c[&g,&c]&n"},
 {SECT_FOREST,            "&c[&GY&c]&n"},
 {SECT_HILLS,             "&c[&Mm&c]&n"},
 {SECT_MOUNTAIN,          "&c[&w^&c]&n"},       /*  5 */
 {SECT_WATER_SWIM,        "&c[&B~&c]&n"},
 {SECT_WATER_NOSWIM,      "&c[&b~&c]&n"},
 {SECT_FLYING,            "&c[&YF&c]&n"},
 {SECT_UNDERWATER,        "&c[&bU&c]&n"},
 {SECT_ROAD,              "&c[&y+&c]&n"},       /* 10 */
 {SECT_ROCK_MOUNTAIN,     "&c[&w^&c]&n"},
 {SECT_SNOW_MOUNTAIN,     "&c[&W^&c]&n"},
 {SECT_RUINS,             "&c[&w|&c]&n"},
 {SECT_JUNGLE,            "&c[&G|&c]&n"},
 {SECT_SWAMP,             "&c[&g\"&c]&n"},       /* 15 */
 {SECT_LAVA,              "&c[&R~&c]&n"},
 {SECT_ENTRANCE,          "&c[&W@&c]&n"},
 {SECT_FARM,              "&c[&y.&c]&n"},
 {SECT_VOLCANIC,          "&c[&r~&c]&n"},
 {SECT_AIR,               "&c[&n.&c]&n"},       /* 20 */
 {SECT_STREET,            "&c[&wC&c]&n"},
 {SECT_BEACH,             "&c[&n.&c]&n"},
 {SECT_HOUSE,             "&c[&wC&c]&n"},
 {SECT_SHOP,              "&c[&n.&c]&n"},
 {SECT_DESERT,            "&c[&n.&c]&n"},
 {SECT_MINE,              "&c[&wC&c]&n"},
 {SECT_ARCTIC,            "&c[&n.&c]&n"},
 {SECT_ARROYO,            "&c[&wC&c]&n"},
 {SECT_AVIARY,            "&c[&n.&c]&n"},       /* 30 */
 {SECT_BAR,               "&c[&wC&c]&n"},
 {SECT_BARRACKS,          "&c[&n.&c]&n"},
 {SECT_BASTION,           "&c[&wC&c]&n"},
 {SECT_BEDROCK,           "&c[&n.&c]&n"},
 {SECT_CALDERA,           "&c[&wC&c]&n"},       /* 35 */
 {SECT_CANYON,            "&c[&n.&c]&n"},
 {SECT_CASTLE,            "&c[&wC&c]&n"},
 {SECT_CATHEDRAL,         "&c[&n.&c]&n"},
 {SECT_CAVE,              "&c[&wC&c]&n"},
 {SECT_CAVERN,            "&c[&n.&c]&n"},       /* 40 */
 {SECT_CEMERTARY,          "&c[&wC&c]&n"},
 {SECT_CHURCH,            "&c[&n.&c]&n"},
 {SECT_CITADEL,           "&c[&wC&c]&n"},
 {SECT_CLIFF,             "&c[&n.&c]&n"},
 {SECT_CONIFEROUS_FOREST, "&c[&wC&c]&n"},       /* 45 */
 {SECT_DECIDUOUS_FOREST,  "&c[&n.&c]&n"},
 {SECT_DELTA,             "&c[&wC&c]&n"},
 {SECT_DUNES,             "&c[&n.&c]&n"},
 {SECT_ENCAMPMENT,        "&c[&wC&c]&n"},
 {SECT_ESTUARY,           "&c[&n.&c]&n"},       /* 50 */
 {SECT_FEN,               "&c[&wC&c]&n"},
 {SECT_FOOTHILLS,         "&c[&n.&c]&n"},
 {SECT_FOOTPATH,          "&c[&wC&c]&n"},
 {SECT_FORTRESS,          "&c[&n.&c]&n"},
 {SECT_GAME_TRAIL,        "&c[&wC&c]&n"},       /* 55 */
 {SECT_GARDEN,            "&c[&n.&c]&n"},
 {SECT_GLACIER,           "&c[&wC&c]&n"},
 {SECT_GORGE,             "&c[&n.&c]&n"},
 {SECT_GROVE,             "&c[&wC&c]&n"},
 {SECT_GUILD,             "&c[&n.&c]&n"},       /* 60 */
 {SECT_HAMLET,            "&c[&wC&c]&n"},
 {SECT_HIGHLANDS,         "&c[&n.&c]&n"},
 {SECT_INLET,             "&c[&wC&c]&n"},
 {SECT_KNOLL,             "&c[&n.&c]&n"},
 {SECT_LAKE,              "&c[&wC&c]&n"},       /* 65 */
 {SECT_LOWLANDS,          "&c[&n.&c]&n"},
 {SECT_MALPAIS,           "&c[&wC&c]&n"},
 {SECT_MARKETPLACE,       "&c[&n.&c]&n"},
 {SECT_MAUSOLEUM,         "&c[&wC&c]&n"},
 {SECT_MORTUARY,          "&c[&n.&c]&n"},       /* 70 */
 {SECT_PALACE,            "&c[&wC&c]&n"},
 {SECT_PLAINS,            "&c[&n.&c]&n"},
 {SECT_PRISON,            "&c[&wC&c]&n"},
 {SECT_PUB,               "&c[&n.&c]&n"},
 {SECT_RIVER,             "&c[&wC&c]&n"},       /* 75 */
 {SECT_SCHOOL,            "&c[&n.&c]&n"},
 {SECT_SNOWFIELD,         "&c[&wC&c]&n"},
 {SECT_STREAM,            "&c[&n.&c]&n"},
 {SECT_STREAMBED,         "&c[&wC&c]&n"},
 {SECT_TEMPLE,            "&c[&n.&c]&n"},       /* 80 */
 {SECT_TENT,              "&c[&wC&c]&n"},
 {SECT_TOWN,              "&c[&n.&c]&n"},
 {SECT_TUNDRA,            "&c[&wC&c]&n"},
 {SECT_UNDERGROUND,       "&c[&n.&c]&n"},
 {SECT_VILLAGE,           "&c[&wC&c]&n"},       /* 85 */
 {SECT_VOLCANO,           "&c[&n.&c]&n"},
 {SECT_WATERSHED,         "&c[&wC&c]&n"},
 {SECT_EMPTY,             "   "        },
 {SECT_STRANGE,           "&R[&r?&R]&n"},
 {SECT_HERE,              "&c[&Y!&c]&n"},       /* 90 */
};


static struct map_info_type world_map_info[] =
{
 {SECT_INSIDE,            "&n."},       /*  0 */
 {SECT_CITY,              "&wC"},
 {SECT_FIELD,             "&g,"},
 {SECT_FOREST,            "&GY"},
 {SECT_HILLS,             "&Mm"},
 {SECT_MOUNTAIN,          "&w^"},       /*  5 */
 {SECT_WATER_SWIM,        "&B~"},
 {SECT_WATER_NOSWIM,      "&b~"},
 {SECT_FLYING,            "&YF"},
 {SECT_UNDERWATER,        "&bU"},
 {SECT_ROAD,              "&y+"},       /* 10 */
 {SECT_ROCK_MOUNTAIN,     "&w^"},
 {SECT_SNOW_MOUNTAIN,     "&W^"},
 {SECT_RUINS,             "&w|"},
 {SECT_JUNGLE,            "&G|"},
 {SECT_SWAMP,             "&g\""},       /* 15 */
 {SECT_LAVA,              "&R~"},
 {SECT_ENTRANCE,          "&W@"},
 {SECT_FARM,              "&y."},
 {SECT_VOLCANIC,          "&r~"},
 {SECT_AIR,               "&n."},       /* 20 */
 {SECT_STREET,            "&wC"},
 {SECT_BEACH,             "&n."},
 {SECT_HOUSE,             "&wC"},
 {SECT_SHOP,              "&n."},
 {SECT_DESERT,            "&n."},
 {SECT_MINE,              "&wC"},
 {SECT_ARCTIC,            "&n."},
 {SECT_ARROYO,            "&wC"},
 {SECT_AVIARY,            "&n."},       /* 30 */
 {SECT_BAR,               "&wC"},
 {SECT_BARRACKS,          "&n."},
 {SECT_BASTION,           "&wC"},
 {SECT_BEDROCK,           "&n."},
 {SECT_CALDERA,           "&wC"},       /* 35 */
 {SECT_CANYON,            "&n."},
 {SECT_CASTLE,            "&wC"},
 {SECT_CATHEDRAL,         "&n."},
 {SECT_CAVE,              "&wC"},
 {SECT_CAVERN,            "&n."},       /* 40 */
 {SECT_CEMERTARY,          "&wC"},
 {SECT_CHURCH,            "&n."},
 {SECT_CITADEL,           "&wC"},
 {SECT_CLIFF,             "&n."},
 {SECT_CONIFEROUS_FOREST, "&wC"},       /* 45 */
 {SECT_DECIDUOUS_FOREST,  "&n."},
 {SECT_DELTA,             "&wC"},
 {SECT_DUNES,             "&n."},
 {SECT_ENCAMPMENT,        "&wC"},
 {SECT_ESTUARY,           "&n."},       /* 50 */
 {SECT_FEN,               "&wC"},
 {SECT_FOOTHILLS,         "&n."},
 {SECT_FOOTPATH,          "&wC"},
 {SECT_FORTRESS,          "&n."},
 {SECT_GAME_TRAIL,        "&wC"},       /* 55 */
 {SECT_GARDEN,            "&n."},
 {SECT_GLACIER,           "&wC"},
 {SECT_GORGE,             "&n."},
 {SECT_GROVE,             "&wC"},
 {SECT_GUILD,             "&n."},       /* 60 */
 {SECT_HAMLET,            "&wC"},
 {SECT_HIGHLANDS,         "&n."},
 {SECT_INLET,             "&wC"},
 {SECT_KNOLL,             "&n."},
 {SECT_LAKE,              "&wC"},       /* 65 */
 {SECT_LOWLANDS,          "&n."},
 {SECT_MALPAIS,           "&wC"},
 {SECT_MARKETPLACE,       "&n."},
 {SECT_MAUSOLEUM,         "&wC"},
 {SECT_MORTUARY,          "&n."},       /* 70 */
 {SECT_PALACE,            "&wC"},
 {SECT_PLAINS,            "&n."},
 {SECT_PRISON,            "&wC"},
 {SECT_PUB,               "&n."},
 {SECT_RIVER,             "&wC"},       /* 75 */
 {SECT_SCHOOL,            "&n."},
 {SECT_SNOWFIELD,         "&wC"},
 {SECT_STREAM,            "&n."},
 {SECT_STREAMBED,         "&wC"},
 {SECT_TEMPLE,            "&n."},       /* 80 */
 {SECT_TENT,              "&wC"},
 {SECT_TOWN,              "&n."},
 {SECT_TUNDRA,            "&wC"},
 {SECT_UNDERGROUND,       "&n."},
 {SECT_VILLAGE,           "&wC"},       /* 85 */
 {SECT_VOLCANO,           "&n."},
 {SECT_WATERSHED,         "&wC"},
 {SECT_EMPTY,             "   "        },
 {SECT_STRANGE,           "&R[&r?&R]&n"},
 {SECT_HERE,              "&Y!"},       /* 90 */
};


static int map[MAX_MAP][MAX_MAP];
static int offsets[4][2] ={ {-2, 0},{ 0, 2},{ 2, 0},{ 0, -2} };
static int offsets_worldmap[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0, -1} };
static int door_offsets[6][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0, -1},{ -1, 1},{ 1, 1} };
static int door_marks[6] = { DOOR_NS, DOOR_EW, DOOR_NS, DOOR_EW, DOOR_UP, DOOR_DOWN };
static int vdoor_marks[4] = { VDOOR_NS, VDOOR_EW, VDOOR_NS, VDOOR_EW };
/******************************************************************************
 *  * End Local (File Scope) Defines and Global Variables
 *   *****************************************************************************/

/******************************************************************************
 *  * Begin Local (File Scope) Function Prototypes
 *   *****************************************************************************/
static void MapArea(room_rnum room, struct char_data *ch, int x, int y, int min, int max, sh_int xpos, sh_int ypos, bool worldmap);
static char *StringMap(int centre, int size);
static char *WorldMap(int centre, int size, int mapshape, int maptype );
static char *CompactStringMap(int centre, int size);
static void perform_map( struct char_data *ch, char *argument, bool worldmap );
/******************************************************************************
 *  * End Local (File Scope) Function Prototypes
 *   *****************************************************************************/


bool can_see_map(struct char_data *ch) {
  /* Is the map funcionality disabled? */
//  if (CONFIG_MAP == MAP_OFF)
//    return FALSE;
//  else if ((CONFIG_MAP == MAP_IMM_ONLY) && (GET_LEVEL(ch) < LVL_IMMORT))
//    return FALSE;

  return TRUE;
}

/* MapArea function - create the actual map */
static void MapArea(room_rnum room, struct char_data *ch, int x, int y, int min, int max, sh_int xpos, sh_int ypos, bool worldmap)
{
  room_rnum prospect_room;
  struct room_direction_data *pexit;
  int door, ew_size=0, ns_size=0, x_exit_pos=0, y_exit_pos=0;
  sh_int prospect_xpos, prospect_ypos;

  if (map[x][y] < 0)
    return; /* this is a door */

  /* marks the room as visited */
  if(room == IN_ROOM(ch))
    map[x][y] = SECT_HERE;
  else
    map[x][y] = SECT(room);

  if ( (x < min) || ( y < min) || ( x > max ) || ( y > max) ) return;

  /* Check for exits */
  for ( door = 0; door < MAX_MAP_DIR; door++ ) {

    if( door < MAX_MAP_FOLLOW &&
        xpos+door_offsets[door][0] >= 0 &&
        xpos+door_offsets[door][0] <= ns_size &&
        ypos+door_offsets[door][1] >= 0 &&
        ypos+door_offsets[door][1] <= ew_size)
    { /* Virtual exit */

      map[x+door_offsets[door][0]][y+door_offsets[door][1]] = vdoor_marks[door] ;
      if (map[x+offsets[door][0]][y+offsets[door][1]] == SECT_EMPTY )
        MapArea(room,ch,x + offsets[door][0], y + offsets[door][1], min, max, xpos+door_offsets[door][0], ypos+door_offsets[door][1], worldmap);
      continue;
    }

    if ( (pexit = world[room].dir_option[door]) > 0  &&
         (pexit->to_room > 0 ) && (pexit->to_room != NOWHERE) &&
         (!IS_SET(pexit->exit_info, EX_CLOSED))) { /* A real exit */

      /* But is the door here... */
      switch (door) {
      case NORTH:
        if(xpos > 0 || ypos!=y_exit_pos) continue;
        break;
      case SOUTH:
        if(xpos < ns_size || ypos!=y_exit_pos) continue;
        break;
      case EAST:
        if(ypos < ew_size || xpos!=x_exit_pos) continue;
        break;
      case WEST:
        if(ypos > 0 || xpos!=x_exit_pos) continue;
        break;
      }


 /*     if ( (x < min) || ( y < min) || ( x > max ) || ( y > max) ) return;*/
      prospect_room = pexit->to_room;

        /* one way into area OR maze */
        if ( world[prospect_room].dir_option[rev_dir[door]] &&
             world[prospect_room].dir_option[rev_dir[door]]->to_room != room) {
          map[x][y] = SECT_STRANGE;
        return;
        }

      if(!worldmap)
        map[x+door_offsets[door][0]][y+door_offsets[door][1]] = door_marks[door] ;

      prospect_xpos = prospect_ypos = 0;
      switch (door) {
      case NORTH:
        prospect_xpos = ns_size;
      case SOUTH:
        prospect_ypos = world[prospect_room].dir_option[rev_dir[door]] ? y_exit_pos : ew_size/2;
      break;
      case WEST:
        prospect_ypos = ew_size;
      case EAST:
        prospect_xpos = world[prospect_room].dir_option[rev_dir[door]] ? x_exit_pos : ns_size/2;
      }

      if(worldmap) {
 if ( door < MAX_MAP_FOLLOW && map[x+offsets_worldmap[door][0]][y+offsets_worldmap[door][1]] == SECT_EMPTY )
   MapArea(pexit->to_room,ch,x + offsets_worldmap[door][0], y + offsets_worldmap[door][1], min, max, prospect_xpos, prospect_ypos, worldmap);
      } else {
 if ( door < MAX_MAP_FOLLOW && map[x+offsets[door][0]][y+offsets[door][1]] == SECT_EMPTY )
   MapArea(pexit->to_room,ch,x + offsets[door][0], y + offsets[door][1], min, max, prospect_xpos, prospect_ypos, worldmap);
      }
    } /* end if exit there */
  }
  return;
}

/* Returns a string representation of the map */
static char *StringMap(int centre, int size)
{
  static char strmap[MAX_MAP*MAX_MAP*11 + MAX_MAP*2 + 1];
  char *mp = strmap;
  char *tmp;
  int x, y;

  /* every row */
  for (x = centre - CANVAS_HEIGHT/2; x <= centre + CANVAS_HEIGHT/2; x++) {
    /* every column */
    for (y = centre - CANVAS_WIDTH/6; y <= centre + CANVAS_WIDTH/6; y++) {
      if (abs(centre - x)<=size && abs(centre-y)<=size)
        tmp = (map[x][y]<0) ? \
       door_info[NUM_DOOR_TYPES + map[x][y]].disp : \
       map_info[map[x][y]].disp ;
      else
 tmp = map_info[SECT_EMPTY].disp;
      strcpy(mp, tmp);
      mp += strlen(tmp);
    }
    strcpy(mp, "\r\n");
    mp+=2;
  }
  *mp='\0';
  return strmap;
}

static char *WorldMap(int centre, int size, int mapshape, int maptype )
{
  static char strmap[MAX_MAP*MAX_MAP*4 + MAX_MAP*2 + 1];
  char *mp = strmap;
  int x, y;
  int xmin, xmax, ymin, ymax;

  switch(maptype) {
    case MAP_COMPACT:
      xmin = centre - size;
      xmax = centre + size;
      ymin = centre - 2*size;
      ymax = centre + 2*size;
      break;
    default:
      xmin = centre - CANVAS_HEIGHT/2;
      xmax = centre + CANVAS_HEIGHT/2;
      ymin = centre - CANVAS_WIDTH/2;
      ymax = centre + CANVAS_WIDTH/2;
  }


  /* every row */
  /* for (x = centre - size; x <= centre + size; x++) { */
  for (x = xmin; x <= xmax; x++) {
    /* every column */
    /* for (y = centre - (2*size) ; y <= centre + (2*size) ; y++) {  */
    for (y = ymin ; y <= ymax ; y++) {

      if((mapshape == MAP_RECTANGLE && abs(centre - y) <= size*2  && abs(centre - x) <= size ) ||
   ((mapshape == MAP_CIRCLE) && (centre-x)*(centre-x) + (centre-y)*(centre-y)/4 <= (size * size + 1))) {
        strcpy(mp, world_map_info[map[x][y]].disp);
        mp += strlen(world_map_info[map[x][y]].disp);
      } else {
 strcpy(mp++, " ");
      }
    }
    strcpy(mp, "@n\r\n");
    mp+=4;
  }
  *mp='\0';
  return strmap;
}

static char *CompactStringMap(int centre, int size)
{
  static char strmap[MAX_MAP*MAX_MAP*12 + MAX_MAP*2 + 1];
  char *mp = strmap;
  int x, y;

  /* every row */
  for (x = centre - size; x <= centre + size; x++) {
    /* every column */
    for (y = centre - size; y <= centre + size; y++) {
      strcpy(mp, (map[x][y]<0) ? \
       compact_door_info[NUM_DOOR_TYPES + map[x][y]].disp : \
       map_info[map[x][y]].disp);
      mp += strlen((map[x][y]<0) ? \
       compact_door_info[NUM_DOOR_TYPES + map[x][y]].disp : \
       map_info[map[x][y]].disp);
    }
    strcpy(mp, "\r\n");
    mp+=2;
  }
  *mp='\0';
  return strmap;
}

/* Display a nicely formatted map with a legend */
static void perform_map( struct char_data *ch, char *argument, bool worldmap )
{
  int size = 20;
  int centre, x, y, min, max;
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int count = 0;
  int ew_size=0, ns_size=0;
  int mapshape = MAP_CIRCLE;

  two_arguments( argument, arg1 , arg2 );
  if(*arg1)
  {
    size = atoi(arg1);
  }
  if (*arg2)
  {
    if (is_abbrev(arg2, "normal")) worldmap=FALSE;
    else if (is_abbrev(arg2, "world")) worldmap=TRUE;
    else {
      send_to_char(ch, "Usage: &yzonemap <distance> [ normal | world ]&n");
      return;
    }
  }

  if(size<0) {
    size = -size;
    mapshape = MAP_RECTANGLE;
  }
  
  size = URANGE(1,size,MAX_MAP_SIZE);

  centre = MAX_MAP/2;

  if(worldmap) {
    min = centre - 2*size;
    max = centre + 2*size;
  } else {
    min = centre - size;
    max = centre + size;
  }

  /* Blank the map */
  for (x = 0; x < MAX_MAP; ++x)
      for (y = 0; y < MAX_MAP; ++y)
           map[x][y]= (!(y%2) && !worldmap) ? DOOR_NONE : SECT_EMPTY;

  /* starts the mapping with the centre room */
  MapArea(IN_ROOM(ch), ch, centre, centre, min, max, ns_size/2, ew_size/2, worldmap);

  /* marks the center, where ch is */
  map[centre][centre] = SECT_HERE;

  strcpy(buf, strfrmt(buf, LEGEND_WIDTH, CANVAS_HEIGHT + 2, FALSE, TRUE, TRUE));

  /* Start with an empty column */
  strcpy(buf1, strfrmt("",0, CANVAS_HEIGHT + 2, FALSE, FALSE, TRUE));


  /* Set up the map */
  memset(buf, ' ', CANVAS_WIDTH);
  count = (CANVAS_WIDTH);
  if(worldmap)
    count += sprintf(buf + count , "\r\n%s", WorldMap(centre, size, mapshape, MAP_NORMAL));
  else
    count += sprintf(buf + count , "\r\n%s", StringMap(centre, size));
  memset(buf + count, ' ', CANVAS_WIDTH);
  strcpy(buf + count + CANVAS_WIDTH, "\r\n");
  /* Print it all out */
  send_to_char(ch, buf);

  return;
}

/* Display a string with the map beside it */
void str_and_map(char *str, struct char_data *ch ) {
  int size, centre, x, y, min, max, char_size;
  int ew_size=0, ns_size=0;
  bool worldmap;

  /* Check MUDs map config options - if disabled, just show room decsription */
  if (!can_see_map(ch)) {
    send_to_char(ch, strfrmt(str, GET_SCREEN_WIDTH(ch), 1, FALSE, FALSE, FALSE));
    return;
  }

  worldmap = ROOM_FLAGGED(IN_ROOM(ch), ROOM_WORLDMAP) ? TRUE : FALSE ;

  if(!PRF_FLAGGED(ch, PRF_AUTOMAP)) {
    send_to_char(ch, strfrmt(str, GET_SCREEN_WIDTH(ch), 1, FALSE, FALSE, FALSE));
    return;
  }

  size = 6;
  centre = MAX_MAP/2;
  min = centre - 2*size;
  max = centre + 2*size;

  for (x = 0; x < MAX_MAP; ++x)
    for (y = 0; y < MAX_MAP; ++y)
      map[x][y]= (!(y%2) && !worldmap) ? DOOR_NONE : SECT_EMPTY;

  /* starts the mapping with the center room */
  MapArea(IN_ROOM(ch), ch, centre, centre, min, max, ns_size/2, ew_size/2, worldmap );
  map[centre][centre] = SECT_HERE;

  /* char_size = rooms + doors + padding */
  if(worldmap)
    char_size = size * 4 + 5;
  else
    char_size = 3*(size+1) + (size) + 4;

  if(worldmap)
    send_to_char(ch, strpaste(strfrmt(str, GET_SCREEN_WIDTH(ch) - char_size, size*2 + 1, FALSE, TRUE, TRUE), WorldMap(centre, size, MAP_CIRCLE, MAP_COMPACT), "   "));
  else
    send_to_char(ch, strpaste(strfrmt(str, GET_SCREEN_WIDTH(ch) - char_size, size*2 + 1, FALSE, TRUE, TRUE), CompactStringMap(centre, size), "   "));

}

ACMD(do_map) {
  if (!can_see_map(ch)) {
    send_to_char(ch, "Sorry, the map is disabled!\r\n");
    return;
  }
  perform_map(ch, argument, ROOM_FLAGGED(IN_ROOM(ch), ROOM_WORLDMAP) ? 1 : 0 );
}



