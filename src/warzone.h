void warzone_update(void);
int warzone_reset(struct descriptor_data *d, int castle, zone_rnum i);
int num_in_castleone();
int num_in_castletwo();
int mobs_in_castleone();
int mobs_in_castletwo();
int num_in_castlethree();
int num_in_castlefour();
int mobs_in_castlethree();
int mobs_in_castlefour();
void save_castle_info();

#define NUM_CASTLES             4
#define WZ_ZONE2                199
#define WZ_ZONE			200
#define CASTLEONE_TOP		20049
#define CASTLEONE_BOTTOM	20000
#define CASTLETWO_TOP		20099
#define CASTLETWO_BOTTOM	20050
#define CASTLETHREE_TOP         19900
#define CASTLETHREE_BOTTOM      19949
#define CASTLEFOUR_TOP          19950
#define CASTLEFOUR_BOTTOM       19999

#define GIVE_CASTLE_PLAYER(ch, cas_num)  (castles[cas_num - 1].owner_p = ch)
#define TAKE_CASTLE_PLAYER(cas_num)      (castles[cas_num - 1].old_owner_p = castles[cas_num - 1].owner_p)

#define GIVE_CASTLE_CREW(ch, cas_num)    (castles[cas_num - 1].owner_c = GET_CLAN_NAME(ch))
#define TAKE_CASTLE_CREW(cas_num)        (castles[cas_num - 1].old_owner_c = castles[cas_num - 1].owner_c)

#define GET_CASTLE_NUMBER(cas_num)       (castles[cas_num - 1].castle_number)
#define GET_CASTLE_NAME(cas_num)         (castles[cas_num - 1].castle_name)

struct castle_info {

  int castle_number;
  char castle_name;
  struct char_data *owner_p;
  struct char_data *old_owner_p;
  int owner_c;
  int old_owner_c;

};

