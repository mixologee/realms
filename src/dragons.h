#define LANDING_SITES "../lib/kuvia.landingsites.dat"
#define PULSE_DRAGONFLIGHT PULSE_MOBILE

void dragon_flight_update(void);

extern struct landing_data *first_landing;
extern struct landing_data *last_landing;

struct landing_data {
  struct landing_data *next;
  struct landing_data *prev;
  char *area;
  int x;
  int y;
  int map;
};
