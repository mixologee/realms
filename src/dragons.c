
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
#include "screen.h"
#include "constants.h"
#include "oasis.h"
#include "warzone.h"
#include "clan.h"
#include "dg_scripts.h"
#include "arena.h"
#include "crystalgate.h"
#include "mob_party.h"

struct landing_data *first_landing;
struct landing_data *last_landing;

sh_int get_terrain(int map, int x, int y); // not implemented yet, should basically look at the landing spot room
                                           // and return the sect_type

double SurvDistance(int x, int y, int x2, int y2);
int SurvWhere (int x, int y, int x2, int y2, int *dist);

void land_dragon(struct char_data *ch, struct char_data *dragon, bool arrived)
{
  char buf[MAX_STRING_LENGTH];
  int terrain;

  terrain = get_terrain(ch->map, dragon->dest_x, dragon->dest_y);

  if (dragon->backtracking) {
   // do_look();
    terrain = get_terrain(ch->map, dragon->dest_x, dragon->dest_y);
  }

  if (terrain != SECT_INSIDE && terrain != SECT_CITY && terrain != SECT_WATER_SWIM && terrain != SECT_WATER_NOSWIM &&
      terrain != SECT_UNDERWATER && terrain != SECT_LAVA && terrain != SECT_ENTRANCE && terrain != SECT_SHOP && 
      terrain != SECT_HOUSE && terrain != SECT_BLANK && terrain != SECT_MINE && terrain != SECT_ENCAMPMENT && 
      terrain != SECT_GUILD && terrain != SECT_MARKETPLACE && terrain != SECT_MAUSOLEUM && terrain != SECT_MORTUARY && 
      terrain != SECT_PALACE && terrain != SECT_PRISION && terrain != SECT_PUB && terrain != SECT_RIVER && 
      terrain != SECT_SCHOOL && terrain != SECT_STREAM && terrain != SECT_STREAMBED && terrain != SECT_TEMPLE && 
      terrain != SECT_TENT && terrain != SECT_TOWN && terrain != SECT_VOLCANO && terrain != SECT_UNDERGROUND && 
      terrain != SECT_OCEAN && terrain != SECT_ALTAR && terrain != SECT_NOPASS && terrain != WILD_ZONEENT && 
      terrain != WILD_TRADEPOST && terrain != WILD_ZONEINTERIOR && terrain != WILD_IMPASSABLELAKE && 
      terrain != WILD_IMPASSABLEWALL && terrain != WILD_IMPASSABLEFENCE && terrain != WILD_DOCK && 
      terrain != WILD_IMPASSABLECLIFF && terrain != WILD_IMPASSABLEFOREST && terrain != WILD_IMPASSABLEBRUSH && 
      terrain != WILD_IMPASSABLESEA && terrain != WILD_CLEARRIVER && terrain != WILD_BIGRIVER && terrain != WILD_POND && 
      terrain != WILD_STREAM && terrain != WILD_SPRING && terrain != WILD_LAKE && terrain != WILD_BAY && 
      terrain != WILD_MURKYLAKE && terrain != WILD_MURKEYRIVER && terrain != WILD_LAVAFLOW && terrain != WILD_LAVAPOOL) {
     if (dragon->backtracking) {
       arrived = TRUE;
       dragon->dest_x = dragon->x;
       dragon->dest_y = dragon->y;
     }
     if (!IS_NPC(ch) && arrived) {
       	ch->x = dragon->dest_x;
	ch->y = dragon->dest_y;
	ch->dragonflight = FALSE;
     }

     TOGGLE_BIT_AR(dragon->act, ACT_NOWANDER)	
