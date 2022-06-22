//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://
//:   File        : maputils.cc                     Part of The Outlands   ://
//:   Date        : October 11, 2004                                       ://
//:   Usage       : Ascii Character Room Mapping                           ://
//:   Programmer  : Aenarion (Michael T. Kohler)                           ://
//:                                                                        ://
//:   COPYRIGHT (C) The Outlands and Michael T. Kohler                     ://
//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://
//:                             MODIFICATIONS                              ://
//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://
//:   Programmer  :                                                        ://
//:   Date        :                                                        ://
//:   Modification:                                                        ://
//:---::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::---://

#include <gd.h>
#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "comm.h"
#include "screen.h"
#include "interpreter.h"
#include "oasis.h"

extern int get_zon_num(int vnum);
extern int create_dir(int room, int dir);
extern int get_top(int);

void LinkZone(int x, int y, int linkto, int dir);

extern int numsurvs;
extern struct Survey *survey_list;
extern struct Survey *surv;

/* Internal variables */
int Top_of_Wild = 0;
char Wildneress[MAPY][MAPX];	/* Map Array	*/
long int charcount=0;
long int charsinterp=0;
/* Internal functions */

struct pix {
 int r;
 int g;
 int b;
};

struct zone_coord {

  int zn;
  int x;
  int y;
};

struct zone_coord ZoneXY[] =
{
   {  63, 1000, 1000 },
   {  41, 1981, 1981 },
   { 100,   24, 1980 },
   { 110, 1980,   16 },
   {  97, 1206, 1704 },
   {  72, 1639, 1163 },
   {  21,  956, 1000 },
   {  61,  660,  879 },
   {  31,  984, 1038 },
   {   4, 1022,  999 },
   {  10,  975,  951 },
   {  26,  967, 1025 },
   {  46, 1016, 1007 },
   { 146,  700,  700 },
   { 132, 1294, 1767 },
   {-1,0,0}
};

struct pix Pixels[MAPY][MAPX];


const char *flags[] =
{
 "&W@",
 "&WT",
 "&W$",
 " ",
 "&bS",
 "&MH",
 "&wH",
 "&W:",
 "&w:", 
 "&y:",
 "&Y:",
 "&w=",
 "&m=",
 "&y=",
 "&W^",
 "&g\"",
 "&g%",
 "&bS",
 "&CS",
 "&BS",
 "&C*",
 "&Cs",
 "&C*",
 "&BS",
 "&BS",
 "&mS",
 "&MS",
 "&RS",
 "&r*",
 "&Gn",
 "&G\"",
 "&G+",
 "&g+",
 "&gn",
 "&w^",
 "&G~",
 "&Y~",
 "&cS",
 "&gx",
 "&g~",
 "&g^",
 "&yn",
 "&y~",
 "&Y*",
 "&r+",
 "&c*",
 "&Gx",
 "&G*",
 "&g*",
 "&gx",
 "&r^",
 "&CC"
};

struct MapSymbolTableStruct {

  char Symbol;
  int Sector;
  int WildType;
  char *Name;
  char *Desc;
  int red;
  int green;
  int blue;
  int flag; 
  int rflags[NUM_ROOM_FLAGS];
  char *TypeName;
};

/* The following is the symbol table that links the room name,
 * description, sector, wild_type, and flags to a specific
 * RGB color in the ../lib/map/map.png file.
 */
struct MapSymbolTableStruct MapSymbolTable[] =
{

{ '@', SECT_INSIDE, WILD_ZONEENT,
   "Zone Entrances",
   "Zone Entrances\r\n",
   255, 255, 255,
   WILD_ZONEENT, {0}, "zone entrances"
},



{ 'T', SECT_ALTAR, WILD_ALTAR,
   "An Ancient Altar From a Forgotten Age",
   "An Ancient Altar From a Forgotten Age\r\n",
   255, 102, 102,
   WILD_ALTAR, {ROOM_INDOORS, ROOM_ALTAR, ROOM_NOMOB}, "an ancient altar from a forgotten age"
},

{ '$', SECT_TRADEPOST, WILD_TRADEPOST,
   "Trading Posts",
   "This area is currently under construction.\r\n",
   254, 254, 254,
   WILD_TRADEPOST, {ROOM_FREIGHTOK}, "trading posts"
},



{ ' ', SECT_NOPASS, WILD_ZONEINTERIOR,
   "Zone Interiors",
   "Zone Interiors\r\n",
   0, 0, 0,
   WILD_ZONEINTERIOR, {ROOM_GODROOM}, "zone interiors"
},



{ 'S', SECT_NOPASS, WILD_IMPASSABLELAKE,
  "A Deep Blue Lake",
  "A Deep Blue Lake\r\n",
  0, 0, 255,
  WILD_IMPASSABLELAKE, {ROOM_GODROOM}, "a deep blue lake"
},



{ 'H', SECT_NOPASS, WILD_IMPASSABLEWALL,
   "City Walls",
   "City Walls\r\n",
   255, 102, 255,
   WILD_IMPASSABLEWALL, {ROOM_GODROOM}, "city walls"
},



{ 'H', SECT_NOPASS, WILD_IMPASSABLEFENCE,
   "An Iron Fence",
   "An Iron Fence\r\n",
   168, 96, 170,
   WILD_IMPASSABLEFENCE, {ROOM_GODROOM}, "an iron fence"
},



{ ':', SECT_ROAD, WILD_MAINROAD,
   "A Well-Paved Road",
   "A Well-Paved Road\r\n",
   204, 102, 0,
   WILD_MAINROAD, {ROOM_FREIGHTOK}, "a well-paved road"
},



{ ':', SECT_ROAD, WILD_CLAYROAD,
   "A Road of Packed Clay",
   "A Road of Packed Clay\r\n",
   102, 102, 0,
   WILD_CLAYROAD, {ROOM_FREIGHTOK}, "a road of packed clay"
},



{ ':', SECT_ROAD, WILD_FORESTROAD,
   "A Narrow Forest Trail",
   "A Narrow Forest Trail\r\n",
   255, 204, 153,
   WILD_FORESTROAD, {ROOM_FREIGHTOK}, "a narrow forest trail"
},



{ ':', SECT_FOOTPATH, WILD_FOOTPATH,
   "An Overgrown Trail",
   "An Overgrown Trail\r\n",
   255, 255, 153,
   WILD_FOOTPATH, {0}, "an overgrown trail"
},



{ '=', SECT_ROAD, WILD_STONEBRIDGE,
   "A Sturdy Stone Bridge",
   "A Sturdy Stone Bridge\r\n",
   153, 102, 153,
   WILD_STONEBRIDGE, {ROOM_FREIGHTOK}, "a sturdy stone bridge"
},



{ '=', SECT_ROAD, WILD_WOODBRIDGE,
   "A Rickety Wooden Bridge",
   "A Rickety Wooden Bridge\r\n",
   204, 153, 153,
   WILD_WOODBRIDGE, {0}, "a rickety wooden bridge"
},



{ '=', SECT_PORT, WILD_DOCK,
   "A Sturdy Wooden Dock",
   "A Sturdy Wooden Dock\r\n",
   138, 83, 34,
   WILD_DOCK, {ROOM_FREIGHTOK}, "a sturdy wooden dock"
},



{ '^', SECT_NOPASS, WILD_IMPASSABLECLIFF,
   "A Sheer Granite Cliff",
   "A Sheer Granite Cliff\r\n",
   229, 237, 229,
   WILD_IMPASSABLECLIFF, {ROOM_GODROOM}, "a sheer granite cliff"
},



{ '"', SECT_NOPASS, WILD_IMPASSABLEFOREST,
   "Thick Treestumps",
   "Thick Treestumps\r\n",
   21, 63, 21,
   WILD_IMPASSABLEFOREST, {ROOM_GODROOM}, "thick treestumps"
},



{ '%', SECT_NOPASS, WILD_IMPASSABLEBRUSH,
   "Thorny Underbrush",
   "Thorny Underbrush\r\n",
   90, 126, 10,
   WILD_IMPASSABLEBRUSH, {ROOM_GODROOM}, "thorny underbrush"
},



{ 'S', SECT_NOPASS, WILD_IMPASSABLESEA,
   "The Deep Blue Sea",
   "The Deep Blue Sea\r\n",
   59, 125, 187,
   WILD_IMPASSABLESEA, {ROOM_GODROOM}, "the deep blue sea"
},



{ 'S', SECT_WATER_NOSWIM, WILD_CLEARRIVER,
   "A Crystal Clear River",
   "A Crystal Clear River\r\n",
   102, 153, 255,
   WILD_CLEARRIVER, {ROOM_FRESHWATER_FISH}, "a crystal clear river"
},



{ 'S', SECT_WATER_NOSWIM, WILD_BIGRIVER,
   "A Wide, Powerful River",
   "A Wide, Powerful River\r\n",
   51, 102, 204,
   WILD_BIGRIVER, {ROOM_FRESHWATER_FISH}, "a wide, powerful river"
},



{ '*', SECT_WATER_NOSWIM, WILD_POND,
   "A Sparkling Clear Pond",
   "A Sparkling Clear Pond\r\n",
   51, 255, 255,
   WILD_POND, {ROOM_FRESHWATER_FISH}, "a sparkling clear pond"
},



{ 's', SECT_WATER_NOSWIM, WILD_STREAM,
   "A Small, Cool Stream",
   "A Small, Cool Stream\r\n",
   102, 255, 255,
   WILD_STREAM, {ROOM_FRESHWATER_FISH}, "a small, cool stream"
},



{ '*', SECT_WATER_NOSWIM, WILD_SPRING,
   "A Gurgling Spring",
   "A Gurgling Spring\r\n",
   51, 255, 204,
   WILD_SPRING, {ROOM_FRESHWATER_FISH}, "a gurgling spring"
},



{ 'S', SECT_WATER_NOSWIM, WILD_LAKE,
   "A Deep Blue Lake",
   "A Deep Blue Lake\r\n",
   105, 182, 249,
   WILD_LAKE, {ROOM_FRESHWATER_FISH}, "a deep blue lake"
},


{ 'S', SECT_WATER_NOSWIM, WILD_BAY,
   "A Shallow Bay",
   "A Shallow Bay\r\n",
   43, 149, 233,
   WILD_BAY, {ROOM_SALTWATER_FISH}, "a shallow bay"
},



{ 'S', SECT_WATER_NOSWIM, WILD_MURKYLAKE,
   "A Warm, Murky Lake",
   "A Warm, Murky Lake\r\n",
   102, 102, 153,
   WILD_MURKYLAKE, {ROOM_FRESHWATER_FISH, ROOM_NPK}, "a warm, murky lake"
},



{ 'S', SECT_WATER_NOSWIM, WILD_MURKYRIVER,
   "A Warm, Murky River",
   "A Warm, Murky River\r\n",
   153, 51, 255,
   WILD_MURKYRIVER, {ROOM_FRESHWATER_FISH, ROOM_NPK}, "a warm, murky river"
},



{ 'S', SECT_WATER_NOSWIM, WILD_LAVAFLOW,
   "A Flowing River of Molten Rock",
   "A Flowing River of Molten Rock\r\n",
   102, 51, 0,
   WILD_LAVAFLOW, {0}, "a flowing river of molten rock"
},



{ '*', SECT_WATER_NOSWIM, WILD_LAVAPOOL,
   "The Mouth of a Volcano",
   "The Mouth of a Volcano\r\n",
   153, 0, 0,
   WILD_LAVAPOOL, {0}, "the mouth of a volcano"
},



{ 'n', SECT_FIELD, WILD_FIELD,
   "Rolling Grassy Plains",
   "Rolling Grassy Plains\r\n",
   153, 153, 0,
   WILD_FIELD, {0}, "rolling grassy plains"
},



{ '"', SECT_LOWLANDS, WILD_LOWLANDS,
   "A Deep Grassy Valley",
   "A Deep Grassy Valley\r\n",
   153, 153, 102,
   WILD_LOWLANDS, {0}, "a deep grassy valley"
},



{ '+', SECT_FOREST, WILD_FOREST,
   "Densely Forested Wilderness",
   "Densely Forested Wilderness\r\n",
   102, 153, 0,
   WILD_FOREST, {0}, "densely forested wilderness"
},



{ '+', SECT_FOREST, WILD_DARKFOREST,
   "A Dark and Foreboding Forest",
   "A Dark and Foreboding Forest\r\n",
   51, 102, 51,
   WILD_DARKFOREST, {ROOM_NPK}, "a dark and foreboding forest"
},



{ 'n', SECT_FOREST, WILD_FORESTHILLS,
   "Densely Forested Foothills",
   "Densely Forested Foothills\r\n",
   153, 102, 0,
   WILD_FORESTHILLS, {0}, "densely forested foothills"
},



{ '^', SECT_MOUNTAIN, WILD_MOUNTAINS,
   "Climbing Rocky Mountains",
   "Climbing Rocky Mountains\r\n",
   153, 153, 153,
   WILD_MOUNTAINS, {0}, "climbing rocky mountains"
},



{ '~', SECT_BEACH, WILD_FORESTDUNES,
   "Forested Sand Dunes",
   "Forested Sand Dunes\r\n",
    102, 255, 102,
    WILD_FORESTDUNES, {0}, "forested sand dunes"
},



{ '~', SECT_BEACH, WILD_BEACH,
   "The Sandy Dunes of a Beach",
   "The Sandy Dunes of a Beach\r\n",
   255, 255, 51,
   WILD_BEACH, {0}, "the sandy dunes of a beach"
},



{ 'S', SECT_WATER_SWIM, WILD_SHORELINE,
   "The Pounding Ocean Surf",
   "The Pounding Ocean Surf\r\n",
   255, 204, 255,
   WILD_SHORELINE, {ROOM_SALTWATER_FISH}, "the pounding ocean surf"
},



{ 'x', SECT_DELTA, WILD_DELTA,
   "A Marshy River Delta",
   "A Marshy River Delta\r\n",
   6, 102, 102,
   WILD_DELTA, {0}, "a marshy river delta"
},



{ '~', SECT_BEACH, WILD_GRASSDUNES,
   "Grass Covered Sand Dunes",
   "Grass Covered Sand Dunes\r\n",
   0, 153, 153,
   WILD_GRASSDUNES, {0}, "grass covered sand dunes"
},



{ '^', SECT_FOREST, WILD_PINEFOREST,
   "A Dense Pine Forest",
   "A Dense Pine Forest\r\n",
   102, 102, 51,
   WILD_PINEFOREST, {0}, "a dense pine forest"
},



{ 'n', SECT_DESERT, WILD_SAVANNAH,
   "A Warm Savannah",
   "A Warm Savannah\r\n",
   204, 153, 0,
   WILD_SAVANNAH, {0}, "a hot, dry savannah"
},



{ '~', SECT_DESERT, WILD_DESERT,
   "A Vast, Arid Desert",
   "A Vast, Arid Desert\r\n",
   255, 204, 102,
   WILD_DESERT, {0}, "a vast, arid desert"
},



{ '*', SECT_GARDEN, WILD_GARDEN,
   "A Well-Tended Garden",
   "A Well-Tended Garden\r\n",
   51, 204, 51,
   WILD_GARDEN, {0}, "a well-tended garden"
},



{ '+', SECT_FOREST, WILD_CHARREDFOREST,
   "A Charred and Blackened Forest",
   "A Charred and Blackened Forest\r\n",
   153, 0, 51,
   WILD_CHARREDFOREST, {ROOM_NPK}, "a charred and blackened forest"
},



{ '*', SECT_TUNDRA, WILD_TUNDRA,
   "A Cold, Barren Tundra",
   "A Cold, Barren Tundra\r\n",
   51, 102, 102,
   WILD_TUNDRA, {0}, "a cold, barren tundra"
},



{ 'x', SECT_JUNGLE, WILD_JUNGLE,
   "A Hot Misty Jungle",
   "A Hot Misty Jungle\r\n",
   102, 204, 0,
   WILD_JUNGLE, {0}, "a hot misty jungle"
},



{ '*', SECT_JUNGLE, WILD_FLOODPLAIN,
   "A Marshy Floodplain",
   "A Marshy Floodplain\r\n",
   0, 204, 102,
   WILD_FLOODPLAIN, {ROOM_NPK}, "a marshy floodplain"
},



{ '*', SECT_JUNGLE, WILD_MARSH,
   "A Stinking Marsh",
   "A Stinking Marsh\r\n",
   204, 255, 0,
   WILD_MARSH, {ROOM_NPK}, "a stinking marsh"
},



{ 'x', SECT_JUNGLE, WILD_DARKMARSH,
   "An Ominous Marsh",
   "An Ominous Marsh\r\n",
   204, 204, 0,
   WILD_DARKMARSH, {ROOM_NPK}, "an ominous marsh"
},



{ '^', SECT_MOUNTAIN, WILD_DARKMOUNTAINS,
   "A Dark, Twisted Mountain Range",
   "A Dark, Twisted Mountain Range\r\n",
   115, 113, 113,
   WILD_DARKMOUNTAINS, {ROOM_NPK}, "a dark, twisted mountain range"
},

{ 'C', SECT_CRYGATE, WILD_CRYGATE,
   "A Massive Ebony Obelisk, Surrounded by Crystalline Formations",
   "A Massive Ebony Obelisk, Surrounded by Crystalline Formations\r\n",
   255, 80, 50,
   WILD_CRYGATE, {ROOM_INDOORS, ROOM_CRYSTALGATE, ROOM_NOMOB}, "a large, crystal archway"
},


  {'Q',
   0,0,
   NULL,
   NULL,
   0,0,0,
   0,{0}, "\n"
  }
};

struct Survey *survey_list;
struct Survey *surv;

// FUNCTION: ReadMapFile
// USAGE   : Read in an Ascii Map
void ReadMapFile(void)
{
  int y, x;		// Coordinates for Wildneress
  char ch;		// Character Place Holder
  FILE *fp;


  // Iterate through each character in the file
  if((fp=fopen(MAPIMAGE,"rb"))==NULL){
       perror("Could not open zones file");
       exit(1);
  }
  // load it to the Wildneress Array
  for (y = 0 ; !EOF ; y++) {	// Y Coords
    for (x = 0 ; !EOF ; x++) {// X Coords
      ch = getc(fp);	// Read in Each Character
      if (ch == -1)	//EOF
        break;
      if (ch == '\n') //End of Line
        break;
      Wildneress[y][x] = ch; // Load the Character into the grid
    }
  }
  fclose(fp);
}

/* FUNCTION: InterperateMapArray
 USAGE   : From the Generated grid Interperate each character and create a
           room with using the specified information.
*/
void InterperateMapArray(void)
{
  int y, x;	/* Coordinates for Wildneress */
  int z, i;
  int t, rf;	/* Search Block Location*/
  char buf[MAX_STRING_LENGTH];

  for (y = MAPY - 1, z = MAPZONE ; y >= 0 ; y--) {
    for (x = 0; x < MAPX ; x++, z++) {	/* X Coord*/

      for (t = 0; MapSymbolTable[t].Symbol != 'Q'; t++) {	/* Compare with Symbol Table*/
        if (MapSymbolTable[t].Symbol == Wildneress[y][x] && MapSymbolTable[t].red == Pixels[y][x].r &&
                MapSymbolTable[t].green == Pixels[y][x].g && MapSymbolTable[t].blue == Pixels[y][x].b){	/* Match Found break*/

          break;
        }
      }

      if (MapSymbolTable[t].Symbol == 'Q') {
        log(buf);
        fprintf(stderr, "\r\nbad color at %d %d\r\n", x, MAPY - y);
      }
/* ALTARS */

      if (MapSymbolTable[t].Symbol == 'T')
      { 
       CREATE(surv, struct Survey, 1);
	surv->next = survey_list;
        survey_list = surv;
	surv->description = "An old, forgotten altar to the gods of Kuvia can be sensed";
        surv->X = x;
        surv->Y = MAPY - y;
        surv->dist = 50;  /* Increasing this makes them way to visible, though up to like 70-80 might be ok, but 120
                           was overkill*/

/*        log("top = %d, x = %d, y = %d, MAPY-Y = %d", top_of_world, x, y, (MAPY - y)); */

        RECREATE(world, struct room_data, top_of_world + 2);
        top_of_world++;
        world[top_of_world].zone = 100000;
        world[top_of_world].number = MAPZONE + (y * MAPY) + x;
        world[top_of_world].func = NULL;
        world[top_of_world].contents = NULL;
        world[top_of_world].people = NULL;
        world[top_of_world].light = 0;
        world[top_of_world].ex_description = NULL;
        world[top_of_world].script = NULL;
        world[top_of_world].proto_script = NULL;
        world[top_of_world].coord = NULL;


      /* Load of the Type, Sector, Name, and Description */
      world[top_of_world].x = x;
      world[top_of_world].y = MAPY - y;
      world[top_of_world].sector_type = MapSymbolTable[t].Sector;
      world[top_of_world].wild_type = MapSymbolTable[t].WildType;
      world[top_of_world].name = (char *) MapSymbolTable[t].Name;
      world[top_of_world].description = (char *) MapSymbolTable[t].Desc;
      SET_BIT_AR(ROOM_FLAGS(top_of_world), ROOM_WILDERNESS);


      for(i = 0; i < NUM_ROOM_FLAGS; i++) {
        if(MapSymbolTable[t].rflags[i] > 0) {
          rf = MapSymbolTable[t].rflags[i];
          SET_BIT_AR(ROOM_FLAGS(top_of_world), rf);
        }
      }

  //    MakeWildRoom(top_of_world, UP);
//      log("Altar Loaded room %d", world[top_of_world].number);
      }

/* Crystal Gates */

      if (MapSymbolTable[t].Symbol == 'C')
      {
       CREATE(surv, struct Survey, 1);
        surv->next = survey_list;
        survey_list = surv;
        surv->description = "A large, crystal archway can be seen";
        surv->X = x;
        surv->Y = MAPY - y;
        surv->dist = 50;  /* Increasing this makes them way to visible, though up to like 70-80 might be ok, but 120
                           was overkill*/
      }

   }
  }
 log("Map Interpreted");
}

ACMD(do_listwild)
{
  int i;

  for(i=0;i<=top_of_world;i++)
  {
   if(ROOM_FLAGGED(real_room(world[i].number), ROOM_WILDERNESS))
     send_to_char(ch, "Room Number = %d, Name = %s\r\n", world[i].number, world[i].name);
  }
}
/* FUNCTION: InterperateMapArray
 *  USAGE   : From the Generated grid Interperate each character and create a
 *             room with using the specified information.
 *             */
void MakeWildRoom(int room, int dir)
{
  int y, x, i, rf;      /* Coordinates for Wildneress */
  long int z;   /* Room Number*/
  int t;        /* Search Block Location*/
  int zn = 100000;   /* Zone Number*/
  int from;
  bool n=FALSE, s=FALSE, w=FALSE, e=FALSE;
  int rn, rs, re, rw;

  x = world[room].x;
//  y = world[room].y;
  y = MAPY - world[room].y - 1;
  z = world[room].number;

  switch(dir)
  {
   case NORTH:
     from = SOUTH;
     break;
   case SOUTH:
     from = NORTH;
     break;
   case EAST:
     from =  WEST;
     break;
   case WEST:
     from = EAST;
     break;
   case UP:
     from = DOWN;
     break;
   case DOWN:
     from = UP;
     break;
   default:
     log("Illegal direction - MakeWildRoom() - maputils.c line 770");
     break;
  }


  for (i=0; i<=top_of_world; i++)
  {
     if (world[i].number == z-2000)
       n=TRUE;
     if (world[i].number == z+2000)
       s=TRUE;
     if (world[i].number == z+1)
       e=TRUE;
     if (world[i].number == z-1)
       w=TRUE;
  }

/* MAKE ROOM NORTH */
   if (from != NORTH && n==FALSE) {
 
      for (t = 0; MapSymbolTable[t].Symbol != 'Q'; t++) {       /* Compare with Symbol Table*/
        if (MapSymbolTable[t].Symbol == Wildneress[y][x] && MapSymbolTable[t].red == Pixels[y][x].r &&
              MapSymbolTable[t].green == Pixels[y][x].g && MapSymbolTable[t].blue == Pixels[y][x].b){ /* Match Found break*/

          break;
        }
      }

        RECREATE(world, struct room_data, top_of_world + 2);
        top_of_world++;
        world[top_of_world].zone = zn;
        world[top_of_world].number = z-2000;
        world[top_of_world].func = NULL;
        world[top_of_world].contents = NULL;
        world[top_of_world].people = NULL;
        world[top_of_world].light = 0;
        world[top_of_world].ex_description = NULL;
        world[top_of_world].script = NULL;
        world[top_of_world].proto_script = NULL;
        world[top_of_world].coord = NULL;

      /* Load of the Type, Sector, Name, and Description*/
      
      world[top_of_world].x = x;
      world[top_of_world].y = MAPY - y;
      world[top_of_world].sector_type = MapSymbolTable[t].Sector;
      world[top_of_world].wild_type = MapSymbolTable[t].WildType;
      world[top_of_world].name = (char *) MapSymbolTable[t].Name;
      world[top_of_world].description = (char *) MapSymbolTable[t].Desc;
      SET_BIT_AR(ROOM_FLAGS(top_of_world), ROOM_WILDERNESS);

      CREATE(world[room].dir_option[NORTH], struct room_direction_data, 1);
      CREATE(world[top_of_world].dir_option[SOUTH], struct room_direction_data, 1);

      world[room].dir_option[NORTH]->general_description = NULL;
      world[room].dir_option[NORTH]->keyword = NULL;
      world[room].dir_option[NORTH]->to_room = top_of_world;
      world[room].dir_option[NORTH]->exit_info = 0;

      world[top_of_world].dir_option[SOUTH]->general_description = NULL;
      world[top_of_world].dir_option[SOUTH]->keyword = NULL;
      world[top_of_world].dir_option[SOUTH]->to_room = room;
      world[top_of_world].dir_option[SOUTH]->exit_info = 0;

      for(i = 0; i < NUM_ROOM_FLAGS; i++) {
        if(MapSymbolTable[t].rflags[i] > 0) {
          rf = MapSymbolTable[t].rflags[i];
          SET_BIT_AR(ROOM_FLAGS(top_of_world), rf);
        }
      }

      if (MapSymbolTable[t].Symbol == 'C')
      {
        obj_vnum onum;
        struct obj_data *obj;
        onum = real_object(1000000);
        obj = read_object(onum, REAL);
        obj_to_room(obj, real_room(top_of_world));

      }
    } else {

     for(rn=0; rn < top_of_world;rn++) {
       if ((world[rn].number == z-2000))
         break;
      }

      CREATE(world[room].dir_option[NORTH], struct room_direction_data, 1);
      CREATE(world[rn].dir_option[SOUTH], struct room_direction_data, 1);

      world[room].dir_option[NORTH]->general_description = NULL;
      world[room].dir_option[NORTH]->keyword = NULL;
      world[room].dir_option[NORTH]->to_room = rn;
      world[room].dir_option[NORTH]->exit_info = 0;

      world[rn].dir_option[SOUTH]->general_description = NULL;
      world[rn].dir_option[SOUTH]->keyword = NULL;
      world[rn].dir_option[SOUTH]->to_room = room;
      world[rn].dir_option[SOUTH]->exit_info = 0;

    }


/* MAKE ROOM SOUTH */
    if (from != SOUTH && s==FALSE) {


      for (t = 0; MapSymbolTable[t].Symbol != 'Q'; t++) {       /* Compare with Symbol Table*/
        if (MapSymbolTable[t].Symbol == Wildneress[y-2][x] && MapSymbolTable[t].red == Pixels[y-2][x].r &&
                MapSymbolTable[t].green == Pixels[y-2][x].g && MapSymbolTable[t].blue == Pixels[y-2][x].b){ /* Match Found break*/

          break;
        }
      }

        RECREATE(world, struct room_data, top_of_world + 2);
        top_of_world++;
        world[top_of_world].zone = zn;
        world[top_of_world].number = z+2000;
        world[top_of_world].func = NULL;
        world[top_of_world].contents = NULL;
        world[top_of_world].people = NULL;
        world[top_of_world].light = 0;
        world[top_of_world].ex_description = NULL;
        world[top_of_world].script = NULL;
        world[top_of_world].proto_script = NULL;
        world[top_of_world].coord = NULL;

      /* Load of the Type, Sector, Name, and Description*/
      world[top_of_world].x = x;
      world[top_of_world].y = MAPY - y -2;
      world[top_of_world].sector_type = MapSymbolTable[t].Sector;
      world[top_of_world].wild_type = MapSymbolTable[t].WildType;
      world[top_of_world].name = (char *) MapSymbolTable[t].Name;
      world[top_of_world].description = (char *) MapSymbolTable[t].Desc;
      SET_BIT_AR(ROOM_FLAGS(top_of_world), ROOM_WILDERNESS);

      CREATE(world[room].dir_option[SOUTH], struct room_direction_data, 1);
      CREATE(world[top_of_world].dir_option[NORTH], struct room_direction_data, 1);

      world[room].dir_option[SOUTH]->general_description = NULL;
      world[room].dir_option[SOUTH]->keyword = NULL;
      world[room].dir_option[SOUTH]->to_room = top_of_world;
      world[room].dir_option[SOUTH]->exit_info = 0;

      world[top_of_world].dir_option[NORTH]->general_description = NULL;
      world[top_of_world].dir_option[NORTH]->keyword = NULL;
      world[top_of_world].dir_option[NORTH]->to_room = room;
      world[top_of_world].dir_option[NORTH]->exit_info = 0;

      for(i = 0; i < NUM_ROOM_FLAGS; i++) {
        if(MapSymbolTable[t].rflags[i] > 0) {
          rf = MapSymbolTable[t].rflags[i];
          SET_BIT_AR(ROOM_FLAGS(top_of_world), rf);
        }
      }

      if (MapSymbolTable[t].Symbol == 'C')
      {
        obj_vnum onum;
        struct obj_data *obj;
        onum = real_object(1000000);
        obj = read_object(onum, REAL);
        obj_to_room(obj, real_room(top_of_world));

      }
   } else {

     for(rs=0; rs < top_of_world;rs++) {
       if ((world[rs].number == z+2000))
         break;
      }

      CREATE(world[room].dir_option[SOUTH], struct room_direction_data, 1);
      CREATE(world[rs].dir_option[NORTH], struct room_direction_data, 1);

      world[room].dir_option[SOUTH]->general_description = NULL;
      world[room].dir_option[SOUTH]->keyword = NULL;
      world[room].dir_option[SOUTH]->to_room = rs;
      world[room].dir_option[SOUTH]->exit_info = 0;

      world[rs].dir_option[NORTH]->general_description = NULL;
      world[rs].dir_option[NORTH]->keyword = NULL;
      world[rs].dir_option[NORTH]->to_room = room;
      world[rs].dir_option[NORTH]->exit_info = 0;

   }

/* MAKE ROOM EAST  */
    if (from != EAST && e==FALSE) {

      for (t = 0; MapSymbolTable[t].Symbol != 'Q'; t++) {       /* Compare with Symbol Table*/
        if (MapSymbolTable[t].Symbol == Wildneress[y][x+1] && MapSymbolTable[t].red == Pixels[y][x+1].r &&
                MapSymbolTable[t].green == Pixels[y][x+1].g && MapSymbolTable[t].blue == Pixels[y][x+1].b){ /* Match Found break*/

          break;
        }
      }

        RECREATE(world, struct room_data, top_of_world + 2);
        top_of_world++;
        world[top_of_world].zone = zn;
        world[top_of_world].number = z+1;
        world[top_of_world].func = NULL;
        world[top_of_world].contents = NULL;
        world[top_of_world].people = NULL;
        world[top_of_world].light = 0;
        world[top_of_world].ex_description = NULL;
        world[top_of_world].script = NULL;
        world[top_of_world].proto_script = NULL;
        world[top_of_world].coord = NULL;

      /* Load of the Type, Sector, Name, and Description*/
      world[top_of_world].x = x+1;
      world[top_of_world].y = MAPY - y - 1;
      world[top_of_world].sector_type = MapSymbolTable[t].Sector;
      world[top_of_world].wild_type = MapSymbolTable[t].WildType;
      world[top_of_world].name = (char *) MapSymbolTable[t].Name;
      world[top_of_world].description = (char *) MapSymbolTable[t].Desc;
      SET_BIT_AR(ROOM_FLAGS(top_of_world), ROOM_WILDERNESS);

      CREATE(world[room].dir_option[EAST], struct room_direction_data, 1);
      CREATE(world[top_of_world].dir_option[WEST], struct room_direction_data, 1);

      world[room].dir_option[EAST]->general_description = NULL;
      world[room].dir_option[EAST]->keyword = NULL;
      world[room].dir_option[EAST]->to_room = top_of_world;
      world[room].dir_option[EAST]->exit_info = 0;

      world[top_of_world].dir_option[WEST]->general_description = NULL;
      world[top_of_world].dir_option[WEST]->keyword = NULL;
      world[top_of_world].dir_option[WEST]->to_room = room;
      world[top_of_world].dir_option[WEST]->exit_info = 0;

      for(i = 0; i < NUM_ROOM_FLAGS; i++) {
        if(MapSymbolTable[t].rflags[i] > 0) {
          rf = MapSymbolTable[t].rflags[i];
          SET_BIT_AR(ROOM_FLAGS(top_of_world), rf);
        }
      }

      if (MapSymbolTable[t].Symbol == 'C')
      {
        obj_vnum onum;
        struct obj_data *obj;
        onum = real_object(1000000);
        obj = read_object(onum, REAL);
        obj_to_room(obj, real_room(top_of_world));

      }
  } else {

     for(re=0; re < top_of_world;re++) {
       if ((world[re].number == z+1))
         break;
      }

      CREATE(world[room].dir_option[EAST], struct room_direction_data, 1);
      CREATE(world[re].dir_option[WEST], struct room_direction_data, 1);

      world[room].dir_option[EAST]->general_description = NULL;
      world[room].dir_option[EAST]->keyword = NULL;
      world[room].dir_option[EAST]->to_room = re;
      world[room].dir_option[EAST]->exit_info = 0;

      world[re].dir_option[WEST]->general_description = NULL;
      world[re].dir_option[WEST]->keyword = NULL;
      world[re].dir_option[WEST]->to_room = room;
      world[re].dir_option[WEST]->exit_info = 0;

  }

/* MAKE ROOM WEST  */
    if (from != WEST && w==FALSE) {

      for (t = 0; MapSymbolTable[t].Symbol != 'Q'; t++) {       /* Compare with Symbol Table*/
        if (MapSymbolTable[t].Symbol == Wildneress[y][x-1] && MapSymbolTable[t].red == Pixels[y][x-1].r &&
                MapSymbolTable[t].green == Pixels[y][x-1].g && MapSymbolTable[t].blue == Pixels[y][x-1].b){ /* Match Found break*/

          break;
        }
      }

        RECREATE(world, struct room_data, top_of_world + 2);
        top_of_world++;
        world[top_of_world].zone = zn;
        world[top_of_world].number = z-1;
        world[top_of_world].func = NULL;
        world[top_of_world].contents = NULL;
        world[top_of_world].people = NULL;
        world[top_of_world].light = 0;
        world[top_of_world].ex_description = NULL;
        world[top_of_world].script = NULL;
        world[top_of_world].proto_script = NULL;
        world[top_of_world].coord = NULL;

      /* Load of the Type, Sector, Name, and Description*/
      world[top_of_world].x = x-1;
      world[top_of_world].y = MAPY - y - 1;
      world[top_of_world].sector_type = MapSymbolTable[t].Sector;
      world[top_of_world].wild_type = MapSymbolTable[t].WildType;
      world[top_of_world].name = (char *) MapSymbolTable[t].Name;
      world[top_of_world].description = (char *) MapSymbolTable[t].Desc;
      SET_BIT_AR(ROOM_FLAGS(top_of_world), ROOM_WILDERNESS);

      CREATE(world[room].dir_option[WEST], struct room_direction_data, 1);
      CREATE(world[top_of_world].dir_option[EAST], struct room_direction_data, 1);

      world[room].dir_option[WEST]->general_description = NULL;
      world[room].dir_option[WEST]->keyword = NULL;
      world[room].dir_option[WEST]->to_room = top_of_world;
      world[room].dir_option[WEST]->exit_info = 0;

      world[top_of_world].dir_option[EAST]->general_description = NULL;
      world[top_of_world].dir_option[EAST]->keyword = NULL;
      world[top_of_world].dir_option[EAST]->to_room = room;
      world[top_of_world].dir_option[EAST]->exit_info = 0;

      for(i = 0; i < NUM_ROOM_FLAGS; i++) {
        if(MapSymbolTable[t].rflags[i] > 0) {
          rf = MapSymbolTable[t].rflags[i];
          SET_BIT_AR(ROOM_FLAGS(top_of_world), rf);
        }
      }

      if (MapSymbolTable[t].Symbol == 'C')
      {
        obj_vnum onum;
        struct obj_data *obj;
        onum = real_object(1000000);
        obj = read_object(onum, REAL);
        obj_to_room(obj, real_room(top_of_world));

      }
   } else {

     for(rw=0; rw < top_of_world;rw++) {
       if ((world[rw].number == z-1))
         break;
      }

      CREATE(world[room].dir_option[WEST], struct room_direction_data, 1);
      CREATE(world[rw].dir_option[EAST], struct room_direction_data, 1);

      world[room].dir_option[WEST]->general_description = NULL;
      world[room].dir_option[WEST]->keyword = NULL;
      world[room].dir_option[WEST]->to_room = rw;
      world[room].dir_option[WEST]->exit_info = 0;

      world[rw].dir_option[EAST]->general_description = NULL;
      world[rw].dir_option[EAST]->keyword = NULL;
      world[rw].dir_option[EAST]->to_room = room;
      world[rw].dir_option[EAST]->exit_info = 0;

   }


return;
}


void ZoneCoords(void)
{
  int i, j;

  for (i = 0; i <= top_of_zone_table; i++)
  {
     for (j = 0; ZoneXY[j].zn != -1; j++)
     {
       if(zone_table[i].number == ZoneXY[j].zn)
       {
	 zone_table[i].x = ZoneXY[j].x;
         zone_table[i].y = ZoneXY[j].y;
         continue;
       }
     }      
  }
}


// FUNCTION: ReadMapZones
// USAGE   : Reads the Zones.map file containing the coordinates, room number of zone,
//         : and link direction information.  It passes this info to the function LinkZone.
void ReadMapZones(void)
{
  char input[20];		// Line by line input from stream
  char fname[256];
  int x, y;		// X and Y coordinates
  int linkto;		// Room in zone to link to wildneress
  int direction;		// direction of wildneress from linkto
  FILE *fp;

  // Open Wildneress.map file
 sprintf(fname, "../Zones.map");
 if((fp=fopen(fname,"rb"))==NULL){
       perror("Could not open zones file");
       exit(1);
  }

  while(get_line(fp, input)) {
    // Read over the comments
    if (input[0] == '#')
      continue;

    if (!strcmp(input, "$"))
      break;

    sscanf(input, "%d %d %d %d", &x, &y, &linkto, &direction);
    LinkZone(x, y, linkto, direction);
  }
  log("zones linked");
  fclose(fp);
}


// FUNCTION: LinkZone
// USAGE   : Receives parameters from the ReadMapZones function.  We free the memory
//         : where the current exits are stored for the specified direction and re links
//         : to the new location on the map.
void LinkZone(int x, int y, int linkto, int dir)
{
  int ZoneRoom = real_room(linkto);
  int revdir = -1;
  int i, t, rf, entr;

//  log("Zone room = %d", ZoneRoom);

  switch (dir) {
  case NORTH:
//    y++;
    revdir = SOUTH;
    break;
  case EAST:
//    x++;
    revdir = WEST;
    break;
  case SOUTH:
//    y--;
    revdir = NORTH;
    break;
  case WEST:
//    x--;
    revdir = EAST;
    break;
  case UP:
    revdir = DOWN;
    break;
  case DOWN:
    revdir = UP;
    break;
  default:
    log("Invalid ::dir:: option specified in LinkZone");
    break;
  }

  for (t = 0; MapSymbolTable[t].Symbol != 'Q'; t++) {       /* Compare with Symbol Table*/
    if (MapSymbolTable[t].Symbol == Wildneress[MAPY - y][x] && MapSymbolTable[t].red == Pixels[MAPY - y][x].r &&
            MapSymbolTable[t].green == Pixels[MAPY - y][x].g && MapSymbolTable[t].blue == Pixels[MAPY - y][x].b){ /* Match Found break*/

       break;
    }
  }

        RECREATE(world, struct room_data, top_of_world + 2);
        top_of_world++;
        world[top_of_world].zone = 100000;
        world[top_of_world].number = MAPZONE + (y * MAPY) + x;
        world[top_of_world].func = NULL;
        world[top_of_world].contents = NULL;
        world[top_of_world].people = NULL;
        world[top_of_world].light = 0;
        world[top_of_world].ex_description = NULL;
        world[top_of_world].script = NULL;
        world[top_of_world].proto_script = NULL;
        world[top_of_world].coord = NULL;

      /* Load of the Type, Sector, Name, and Description*/
      world[top_of_world].x = x;
      world[top_of_world].y = y;
      world[top_of_world].sector_type = MapSymbolTable[t].Sector;
      world[top_of_world].wild_type = MapSymbolTable[t].WildType;
      world[top_of_world].name = (char *) MapSymbolTable[t].Name;
      world[top_of_world].description = (char *) MapSymbolTable[t].Desc;
      SET_BIT_AR(ROOM_FLAGS(top_of_world), ROOM_WILDERNESS);

      entr = top_of_world;
      MakeWildRoom(entr,dir);

      CREATE(world[ZoneRoom].dir_option[dir], struct room_direction_data, 1);
      CREATE(world[entr].dir_option[revdir], struct room_direction_data, 1);

      world[ZoneRoom].dir_option[dir]->general_description = NULL;
      world[ZoneRoom].dir_option[dir]->keyword = NULL;
      world[ZoneRoom].dir_option[dir]->to_room = entr;
      world[ZoneRoom].dir_option[dir]->exit_info = 0;

      world[entr].dir_option[revdir]->general_description = NULL;
      world[entr].dir_option[revdir]->keyword = NULL;
      world[entr].dir_option[revdir]->to_room = ZoneRoom;
      world[entr].dir_option[revdir]->exit_info = 0;

      for(i = 0; i < NUM_ROOM_FLAGS; i++) {
        if(MapSymbolTable[t].rflags[i] > 0) {
          rf = MapSymbolTable[t].rflags[i];
          SET_BIT_AR(ROOM_FLAGS(entr), rf);
        }
      }

  /* Set flag in zone room */
  world[ZoneRoom].sector_type = SECT_INSIDE;
  world[ZoneRoom].wild_type = WILD_ZONEENT;
  world[entr].sector_type = SECT_INSIDE;
  world[entr].wild_type = WILD_ZONEENT;
  log("top = %d, entr = %d, x = %d, y = %d, MAPY-Y = %d", top_of_world, entr, x, y, (MAPY - y));
}

// FUNCTION: PrintWildMap
// USAGE   : From the coordinates of the player's room this function draw the map
//         : to the screen.

void PrintWildMap(chData *ch)
{
  /* Center of the map */
  int WMapX = world[ch->in_room].x;
  int WMapY = world[ch->in_room].y;
  /* Read Buffer Length */
  int XBufLen=10;
  int YBufLen=10;
 /* Starting Points top left coordinates of displayed map */
  int StartX;
  int StartY;
  /* Coordinates */
  int x, y;
  int x1, y1;
  int wt, t;

  XBufLen = GET_MAPX(ch);
  YBufLen = GET_MAPY(ch);

  if (!XBufLen || !YBufLen) {
    XBufLen = MAXROWSIZE;
    YBufLen = MAXCOLSIZE;
  }

  StartX = WMapX - (XBufLen / 2);
  StartY = WMapY - (YBufLen / 2);

  if (StartX < 0)
    StartX = 0;
  if (StartY < 0)
    StartY = 0;

  send_to_char(ch, "&M%s&n\r\n", world[IN_ROOM(ch)].name);
  for (y = StartY + YBufLen, y1 = 0 ; y1 <= YBufLen ; y--, y1++) {
    for (x = StartX, x1 = 0; x1 <= XBufLen ; x++, x1++) {

      if (y == WMapY && x == WMapX)
        send_to_char(ch,"&W#");
    /* Take care of the corners of the map */
      else if (y <= 0 || x <= 0 || y > MAPY - 1 || x > MAPY - 1)
        send_to_char(ch," ");
      else {
      for (t = 0; MapSymbolTable[t].Symbol != 'Q'; t++) {       /* Compare with Symbol Table*/
        if (MapSymbolTable[t].Symbol == Wildneress[MAPY - y][x] && MapSymbolTable[t].red == Pixels[MAPY -y][x].r &&
                MapSymbolTable[t].green == Pixels[MAPY - y][x].g && MapSymbolTable[t].blue == Pixels[MAPY - y][x].b){ /* Match Found break*/

          break;
        }
      }
        wt = (int) MAPZONE + (y * MAPY) + x;
        send_to_char(ch,"%s", flags[MapSymbolTable[t].WildType]);
      }
      if (x1 == XBufLen)
        send_to_char(ch,"\r\n");
    }
  }
}

char GetMapSymbol(gdImagePtr im, int pixel)
{
  int i;

  for (i = 0; MapSymbolTable[i].Symbol != 'Q' ; ++i) {
    if (gdImageGreen(im, pixel) == MapSymbolTable[i].green &&
        gdImageRed(im, pixel) == MapSymbolTable[i].red &&
        gdImageBlue(im, pixel) == MapSymbolTable[i].blue)
      return MapSymbolTable[i].Symbol;
  }
  return 'o';
}

// FUNCTION: LoadMapImage
// USAGE   : Loads the map.png image, reads every pixel and translates
//         : the RGB value to a symbol and populates the Wildneress[][] array.
void LoadMapImage(void)
{
  FILE *MapImage;
  gdImagePtr im;
  char Symbol;
  int pixel, x, y;

  if (!(MapImage = fopen(MAPIMAGE, "rb"))) {
    log("Error opening map.png, aborting.");
    return;
  }

  im = gdImageCreateFromPng(MapImage);

  for (y = 0; y < gdImageSY(im); ++y) {
    for (x = 0; x < gdImageSX(im); ++x) {
      pixel = gdImageGetPixel(im, x, y );
      Symbol = GetMapSymbol(im, pixel);
      Pixels[y][x].r = gdImageRed(im, pixel);
      Pixels[y][x].g = gdImageGreen(im, pixel);
      Pixels[y][x].b = gdImageBlue(im, pixel);
      Wildneress[y][x] = Symbol;
      charcount++;
    }
  }

  fclose(MapImage);
  gdImageDestroy(im);
  return;
}

int GetSymbolNum(char Symbol)
{
  int i;

  for (i = 0 ; MapSymbolTable[i].Symbol != 'Q' ; i++) {
    if (Symbol == MapSymbolTable[i].Symbol)
      return i;
  }

  return -1;
}

void SaveMapImage(void)
{
  FILE *MapImage;
  int x, y, sect;
  int MapFlags[NUM_WILD_SECTORS];
  gdImagePtr im;
  int r,g,b, i;

  im = gdImageCreate(MAPX, MAPY);

  for (x = 0; x < NUM_WILD_SECTORS; x++)
    r =  MapSymbolTable[x].red;
    g = MapSymbolTable[x].green;
    b =  MapSymbolTable[x].blue;

    
    i = gdImageColorAllocate(im, r, g, b);
    MapFlags[x] = i;

  for (y = 0; y < MAPY; y++) {
    for (x = 0; x < MAPX; x++) {
      sect = GetSymbolNum(Wildneress[y][x]);

      if (sect == -1)
        sect =  WILD_IMPASSABLESEA;

      gdImageLine(im, x, y, x, y, MapFlags[sect]);
    }
  }

  if (!(MapImage = fopen(MAPIMAGE, "wb" ))) {
    log("Error opening map.png, aborting.");
    return;
  }

  gdImagePng(im, MapImage);
  fclose(MapImage);
  gdImageDestroy(im);
}

ACMD(do_png)
{
  int x, y;
  long int pixelloc;

  if(*argument) {
    send_to_char(ch, "Huh?");
    return;
  }

  x = world[IN_ROOM(ch)].x;
  y = world[IN_ROOM(ch)].y;
  pixelloc = x*y;

  send_to_char(ch, "Current Room Pixel Info:\r\n"
                   "X- %d   Y- %d\r\n Room Name %s \r\n"
                   "Wild Type= %d\r\n"
                   "Sect Type= %d\r\n"
                   "R= %d G= %d B= %d\r\n",
                   x, y, world[IN_ROOM(ch)].name,
                   world[IN_ROOM(ch)].wild_type,
                   world[IN_ROOM(ch)].sector_type,
		   Pixels[y][x].r,
                   Pixels[y][x].g,
                   Pixels[y][x].b);       
}

ACMD(do_mapsize)
{
  char cRow[MIL], cCol[MIL];
  int iRow, iCol;

  two_arguments(argument, cRow, cCol);

  if (!*cRow || !*cCol) {
    send_to_char(ch, "Syntax: mapsize <row length> <column length>\r\n"
              "\r\n"
              "Row length is the horizontal length, and the column length is the\r\n"
              "vertical length.\r\n");
    return;
  }

  if (!isdigit(*cRow) || !isdigit(*cCol)) {
    send_to_char(ch, "Row and Column must be numeric.\r\n");
    return;
  }

  iRow = atoi(cRow);
  iCol = atoi(cCol);

  if (iRow % 2 || iCol % 2) {
    send_to_char(ch, "Row and column must be even numbers!\r\n");
    return;
  }

  if (iCol > MAXCOLSIZE) {
    send_to_char(ch, "Column size cannot be greater than %d.\r\n", MAXCOLSIZE);
    return;
  }

  if (iRow > MAXROWSIZE) {
    send_to_char(ch, "Row size cannot be greater than %d.\r\n", MAXROWSIZE);
    return;
  }

  GET_MAPX(ch) = MIN(20, iRow);
  GET_MAPY(ch) = MIN(20, iCol);

  send_to_char(ch, "Your map size has been set to ROW: %d, COL: %d.\r\n", iRow, iCol);
}

ACMD(do_mapsave)
{

  SaveMapImage();

  send_to_char(ch, "Wildneress map has been saved.\r\n");

}

ACMD(do_mapset)
{
  int i;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];


  if (!PLR_FLAGGED(ch, PLR_MAPEDITOR)) {
    send_to_char(ch, "You are unable to edit the wildneress map.\r\n");
    return;
  }

  if (!ROOM_FLAGGED(ch->in_room, ROOM_WILDERNESS)) {
    send_to_char(ch, "This room is not part of the wildneress.\r\n");
    return;
  }

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "Syntax: mapset <type>\r\n\r\n");
    sprintf(buf + strlen(buf), "Avilable types:\r\n");

    for (i = 0 ; MapSymbolTable[i].Symbol != '~' ; i++) {
      sprintf(buf + strlen(buf), "&n%-15s %-5d &n%s\r\n", MapSymbolTable[i].TypeName,
              MapSymbolTable[i].Symbol, MapSymbolTable[i].Name);
    }

    page_string(ch->desc, buf, 1);
    return;
  }

  for (i = 0 ; MapSymbolTable[i].Symbol != '~' ; i++) {
    if (!strncmp(arg, MapSymbolTable[i].TypeName, strlen(arg)))
      break;
  }

  if (MapSymbolTable[i].Symbol == '~') {
    send_to_char(ch, "Specified type does not exist, type mapset for a list of valid types.\r\n");
    return;
  }

  world[ch->in_room].sector_type = MapSymbolTable[i].Sector;
  world[ch->in_room].wild_type = MapSymbolTable[i].WildType;
  world[ch->in_room].name = (char *) MapSymbolTable[i].Name;
  world[ch->in_room].description = (char *) MapSymbolTable[i].Desc;

  Wildneress[MAPY - 1 - world[ch->in_room].y][world[ch->in_room].x] = MapSymbolTable[i].Symbol;
}

const char *wildmobs[] =
{
  "1000000",
  "1000001",
  "1000002",
  "1000003",
  "1000004",
  "1000005",
  "1000006",
  "1000007",
  "1000008",
  "1000009",
  "1000010",
  "1000011",
  "1000012",
  "1000013",
  "1000014",
  "1000015",
  "1000016",
  "1000017",
  "1000018",
  "1000019",
  "1000020",
  "1000021",
  "1000022",
  "1000023",
  "1000024",
  "1000025",
  "1000026",
  "1000027",
  "1000028",
  "1000029",
  "1000030",
  "1000031",
  "1000032",
  "1000033",
  "1000034",
  "1000035",
  "1000036",
  "1000037",
  "1000038",
  "1000039"
};


void PopulateWild(int i)
{
  int x, y, danger, mob;
  mob_rnum mobnum;
  room_rnum wroom;
  struct char_data *wm;

   x = world[i].x;
   y = world[i].y;

   if(x > 1000 && y > 1000)
     danger = ((x - 1000) + (y - 1000)) / 2;
   else if(x < 1000 && y > 1000)
     danger = ((1000-x) + (y - 1000)) / 2;
   else if(x > 1000 && y < 1000)
     danger = ((x - 1000) + (y - 1000)) / 2;
   else if(x < 1000 && y < 1000)
     danger = ((1000 - x) + (1000 - y)) / 2;
   else
     danger =0;

   if(danger < 200)
     mob = rand_number(0,10);
   else if(danger < 400)
     mob = rand_number(5,15);
   else if(danger < 600)
     mob = rand_number(12,22);
   else if(danger < 800)
     mob = rand_number(20,27);
   else if(danger < 1000)
     mob = rand_number(25,35);
   else
     mob = rand_number(30,39);
  
   if (rand_number(0,1000) > 900 && world[i].sector_type != SECT_NOPASS)
   {  
    mobnum = real_mobile(atoi(wildmobs[mob]));
    wroom = world[i].number;
    wm = read_mobile(mobnum, REAL);
    char_to_room(wm, real_room(wroom));
   }
 
}



