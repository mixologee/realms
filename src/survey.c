#include <math.h>
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

int numsurvs=0;
extern int Top_of_Wild;

struct Survey *survey_list;
int SurvDistance(int X1,int Y1,int X2,int Y2);

char *SurveyDists[] = {
"hundreds of miles away in the distance",     
"far off in the skyline",
"many miles away at great distance",
"far off many miles away",
"tens of miles away in the distance",
"far off in the distance",
"several miles away",
"off in the distance",
"not far from here",
"in near vicinity",
"in the immediate area"
};

void LoadSurveyList(void)     
{

  FILE *sfile;
  int num = 0;  
  int temp;
  struct Survey *surv;
  char Tag[MAX_STRING_LENGTH];
  
  if (!(sfile = fopen("../kuvia.bear","r"))) {
    perror("Opening kuvia.survey");
    return;
  }

  fscanf(sfile, "%s", Tag);
  while(1) {
    if (Tag[0]=='$')
      break;
    CREATE(surv, struct Survey, 1);
    num++;
    while(1) { 
      fscanf(sfile, "%s",Tag);
      if (Tag[0] == '#' || Tag[0] == '$') {
        surv->next = survey_list;
        survey_list = surv;
        break;
      }
      if(!strn_cmp(Tag,"DSC",3)){
         surv->description = fread_string(sfile, Tag);
      }
      else if(!strn_cmp(Tag,"XVAL",4)) {
         fscanf(sfile," %d ", &temp);
         surv->X = temp;
      }else if(!strn_cmp(Tag,"YVAL",4)) {
         fscanf(sfile," %d ", &temp);
         surv->Y = temp;
      }else if(!strn_cmp(Tag,"DIST",4)) {
         fscanf(sfile," %d ", &temp);    
         surv->dist = temp;
      }                
     }
    }

/*    for(i=Top_of_Wild;i > (top_of_world - Top_of_Wild);i--)
    {
	if(SECT(world[i].number) == SECT_ALTAR)
	{
	  surv->next = survey_list;
          survey_list = surv;
	  surv->description = "A glorious shrine to the gods can be seen";
	  surv->X = world[i].x;
          surv->Y = world[i].y;
	  surv->dist = 120;
	  num++;
	}
    }
*/        numsurvs = num;
        fclose(sfile);
        fprintf(stderr,"SURVEY DATA:  Survey objects... %d loaded into memory.\n",num);
}


ACMD(do_survey)
{
  struct Survey *surv;
  int Distance;
  int Angle;
  char *buf;
  bool found = 0;
  char Dir = -1;
  int Message = 0;

  if (!ch)
    return;

  if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS)) {
     send_to_char(ch, "\r\n&WYou must be in the wilderness to get your bearings.\r\n");
     return;
  }

  buf = '\0';

  for (surv = survey_list; surv; surv = surv->next) {
    Distance = (int)SurvDistance(world[IN_ROOM(ch)].x,world[IN_ROOM(ch)].y, (int)surv->X, (int)surv->Y);

  if (Distance <= surv->dist) {
    found = 1;
    Angle = SurvWhere(world[IN_ROOM(ch)].x, world[IN_ROOM(ch)].y, (int)surv->X, (int)surv->Y, (int)&Distance, ch);
    if (Angle == -1)
      Dir = 13;
    else if (Angle >= 360)
      Dir = 12;
    else if(Angle >= 330)
      Dir = 11;
    else if(Angle >= 300)
      Dir = 10;
    else if(Angle >= 270)
      Dir = 9;
    else if(Angle >= 240)
      Dir = 8;
    else if(Angle >= 210)               
      Dir = 7;
    else if(Angle >= 180)
      Dir = 6;
    else if(Angle >= 150)
      Dir = 5;
    else if(Angle >= 120)
      Dir = 4;
    else if(Angle >= 90)
      Dir = 3;
    else if(Angle >= 60)
      Dir = 2;
    else if(Angle >= 30)
      Dir = 1;
    else if(Angle >= 0)
      Dir = 12;

  if(Distance > 200)
      Message = 0;
  else if(Distance > 150)
    Message = 1;
  else if(Distance > 100)
    Message = 2;
  else if(Distance > 75)
    Message = 3;
  else if(Distance > 50)
    Message = 4;
  else if(Distance > 25)
    Message = 5;
  else if(Distance > 15)
    Message = 6;
  else if(Distance > 10)
    Message = 7;
  else if(Distance > 5)  
    Message = 8;
  else if(Distance > 1)
    Message = 9; 
  else                        
    Message = 10;

  if(Dir == 13)
    send_to_char(ch,"%s can be seen in the immediate area.\r\n", surv->description ? surv->description : "<NULL>");
  else
    send_to_char(ch,"%s at %d o'clock, %s.\r\n",  surv->description ? surv->description : "<NULL PLEASE REPORT>", Dir, SurveyDists[(int) Message]);
  
/* EXTRA GOD DISPLAY */      
   if (GET_LEVEL(ch)>=41)
     send_to_char(ch, "[DFO: %d] [(%d,%d)] Angle = %d\r\n", Distance, surv->X, surv->Y, Angle);
                }
        }
        if(!found)
                send_to_char(ch, "Your survey of the area yields nothing special.\r\n");
}


int SurvWhere(int X1, int Y1, int X2, int Y2, int Dist, struct char_data *ch)
{
  int Nx1, Nx2, Nx3, Ny1, Ny2, Ny3;
  Nx1 = Ny1 = 0;
  double Dist1, Dist2;
  double Tandeg, Deg;
  int Final;
  
     Nx2 = X2 - X1;
     Ny2 = Y2 - Y1;
     Nx3 = 0;
     Ny3 = Ny2;

     Dist = (int)SurvDistance(Nx1, Ny1, Nx2, Ny2);

/*     if(GET_LEVEL(ch) > LVL_GRGOD)
       send_to_char(ch, "X1 = %d, X2 = %d\r\n"
                        "Y1 = %d, Y2 = %d\r\n"
                        "Nx1 = %d, Nx2 = %d, Nx3 = %d\r\n"
                        "Ny1 = %d, Ny2 = %d, Ny3 = %d\r\n"
                        "Dist = %d\r\n",
                        X1, X2, Y1, Y2, Nx1, Nx2, Nx3, Ny1, Ny2, Ny3, Dist);
*/
/*    if (Nx2 == 0 && Ny2 == 0)
       return(-1);
    if (Nx2 == 0 && Ny1 > 0)
       return(180);
    if (Nx2 == 0 && Ny2 < 0)
       return(0);
    if (Nx2 > 0 && Ny2 == 0)
       return(90);
    if (Nx2 < 0 && Ny2 == 0)
       return(270);
*/
     Dist1 = SurvDistance(Nx1, Ny1, Nx3, Ny3);
     Dist2 = SurvDistance(Nx3, Ny3, Nx2, Ny2);

     Tandeg = Dist2 / Dist1;
     Deg = atan(Tandeg);

     Final = (Deg * 180) / 3.14159265358979323846;
/*
     if(GET_LEVEL(ch) > LVL_GRGOD)
       send_to_char(ch, "Dist1 = %e\r\n, Dist2 = %e\r\n, Tandeg = %e\r\n, Deg = %e\r\n, Final = %e\r\n",
                         Dist1, Dist2, Tandeg, Deg, Final);
*/
    if (X1 < X2 && Y1 < Y2) { 
        return(90 - Final);
    } 
    else if (X1 > X2 && Y1 < Y2) {
        return((270 + (90 - Final)));
     } 
     else if (X1 < X2 && Y1 > Y2) {
        return((90 + (90 - Final)));
     } 
     else if (X1 > X2 && Y1 > Y2) {
        return((180 + Final));
     } 
     else if (X1 == X2 && Y1 > Y2) {
        return(180);
     }
     else if (X1 == X2 && Y1 < Y2) {
        return(0);
     }
     else if (X1 > X2 && Y1 == Y2) {
        return(270);
     }
     else if (X1 < X2 && Y1 == Y2) {
        return(90);
     }
     else
        return(-1);
}

int SurvDistance(int X1,int Y1,int X2,int Y2)         
{
double ChangeX, ChangeY;
double Dist;
        ChangeX = (X1 - X2);
        ChangeX *= ChangeX;
        ChangeY = (Y1 - Y2);
        ChangeY *= ChangeY;
        Dist = sqrt((double)(ChangeX + ChangeY));       
        return(Dist);
}

int MessageDistance(struct char_data *ch, struct char_data *vict)
{
  int dist = 0;
  int chX, chY, victX, victY = 0;

  if (!ch || !vict)
    return (0);


  if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS) || !ROOM_FLAGGED(IN_ROOM(vict), ROOM_WILDERNESS)) {
    victX = zone_table[world[IN_ROOM(vict)].zone].x;
    victY = zone_table[world[IN_ROOM(vict)].zone].y;
    chX = zone_table[world[IN_ROOM(ch)].zone].x;
    chY = zone_table[world[IN_ROOM(ch)].zone].y;
  } else {
    victX = world[IN_ROOM(vict)].x;
    victY = world[IN_ROOM(vict)].y;
    chX = world[IN_ROOM(ch)].x;
    chY = world[IN_ROOM(ch)].y;
  }
  
  dist = (int)SurvDistance(chX, chY, victX, victY);
//  Distance = (int)SurvDistance(GET_MAPX(ch), GET_MAPY(ch), GET_MAPX(vict), GET_MAPY(vict));
  return (dist);
}

ACMD(do_checkdist)
{
  char arg[MAX_INPUT_LENGTH];
  int dist;
  int chX, chY, victX, victY;
  struct char_data *vict;
  int ChangeInX, ChangeInY = 0;
  double dist2;
  struct room_data *rm1, *rm2;
  rm1 = IN_ROOM(ch);
  rm2 = IN_ROOM(vict);

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char(ch, "Syntax: checkdist <name>\r\n");
    return;
  }

  if (!(vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD))) {
    send_to_char(ch, "%s", CONFIG_NOPERSON);
    return;
  }

  if (IS_NPC(vict)) {
    send_to_char(ch, "%s isn't a valid target to distance!\r\n", arg);
    return;
  }

  vict = get_char_vis(ch, arg, NULL, FIND_CHAR_WORLD);

/* if player is in wild use world[IN_ROOM(ch)].x/y otherwise IN_ROOM(ch)->zone.x/y */

  if (!ROOM_FLAGGED(IN_ROOM(ch), ROOM_WILDERNESS) || !ROOM_FLAGGED(IN_ROOM(vict), ROOM_WILDERNESS)) {
    victX = zone_table[world[IN_ROOM(vict)].zone].x;
    victY = zone_table[world[IN_ROOM(vict)].zone].y;
    chX = zone_table[world[IN_ROOM(ch)].zone].x;
    chY = zone_table[world[IN_ROOM(ch)].zone].y;
  } else {
    victX = rm2->x;
    victY = rm2->y;
    chX = rm1->x;
    chY = rm1->y;
  }
      
  ChangeInX = chX - victX;
  ChangeInX *= ChangeInX;
  ChangeInY = chY - victY;
  ChangeInY *= ChangeInY;
  dist2 = sqrt((double)(ChangeInX + ChangeInY));

  dist = (int)SurvDistance(chX, chY, victX, victY);
//  dist = (int)SurvDistance(GET_MAPX(ch), GET_MAPY(ch), GET_MAPX(vict), GET_MAPY(vict));

  send_to_char(ch, "CheckDist Info for %s\r\n"
                   "----------------------------------\r\n"
                   "Vict X : %d\r\n"
                   "vict Y : %d\r\n"
                   "ch X   : %d\r\n"
                   "ch Y   : %d\r\n"
                   "----------------------------------\r\n"
                   "ChangeX: %d\r\n"
                   "ChangeY: %d\r\n"
                   "---------------------------------\r\n"
                   "Dist1  : %d\r\n"
                   "Dist2  : %ld\r\n"
                   "----------------------------------\r\n",
                    arg, victX, victY, chX, chY, ChangeInX, ChangeInY,dist, dist2);

//  send_to_char(ch, "%s is %d from your current position.\r\n", arg, dist);
}


