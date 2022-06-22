#define MAX_WOLF_PACT     3
#define MAX_LION_PACT     3
#define MAX_KOBOLD_PACT   5
#define MAX_BANDIT_PACT   6
#define MAX_WASP_PACT     5

struct party_mem {

  struct char_data *mem;
  struct char_data *nextmem;
};

struct mob_party {

  int id;
  int range;
  int danger;
  int num_mems;
  struct char_data *target;
  struct char_data *leader;
  struct party_mem *mobs;
  struct mob_party *next;
};

