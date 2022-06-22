// New Configureable Prompt Code
// -Slurk


#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "genolc.h"
#include "oasis.h"
#include "improved-edit.h"
#include "pfdefaults.h"
#include "clan.h"
#include "dg_scripts.h"
#include "constants.h"
#include "quest.h"
#include "logger.h"


extern ush_int DFLT_PORT;
extern room_data *world;
extern char *strip_colors(const char *arg);
extern int level_exp(int level);

char StatColors(int cur, int max);
char *DisplayPrompt(struct char_data *ch);
char *strip_colors(const char *arg);
int GET_TOTAL_LEVEL(struct char_data *ch);


void SetPrompt(struct char_data *ch, char *prompt)
{
  if (!GET_PROMPT(ch))
    CREATE(GET_PROMPT(ch), char, MSL);
  else {
    CREATE(GET_PROMPT(ch), char, MSL);
    GET_PROMPT(ch) = "";
  }

  if (!prompt)
    *prompt = '\0';

  if (strlen(prompt) > MSL)
    prompt[MSL] = '\0';

  GET_PROMPT(ch) = prompt;
}


ACMD(do_prompt)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

if (!*argument) {
    send_to_char(ch, "\nSyntax: prompt <type> &Rsee HELP PROMPT for details&n\r\n");
    send_to_char(ch, "        prompt default\r\n\r\n");
    send_to_char(ch, "Current prompt : %s\r\n\r\n", (GET_PROMPT(ch) ? strip_colors(GET_PROMPT(ch)) : "No Prompt Set"));
    return;         
  }              

  if (IS_NPC(ch))
    send_to_char(ch, "NPC's do not need a prompt.\r\n");
//  else if (strlen(argument)  > MSL)
//    send_to_char(ch, "Error: Prompts cannot be more than %d chars long.\r\n", MSL);
//  else if {         
    if (is_abbrev(argument, "default")) {
       if (GET_LEVEL(ch) >= LVL_IMMORT)
         argument = "&B<%h&B/&W%H&Bhp %m&B/&W%M&Bmn %v&B/&W%V&Bmv>&n";
       else               
         argument = "&B<%h&B/&W%H&Bhp %m&B/&W%M&Bmn %v&B/&W%V&Bmv>&n";
    }
   else 
     argument = "&B<%h&B/&W%H&Bhp %m&B/&W%M&Bmn %v&B/&W%V&Bmv %X&Bxp>&n";            
   SetPrompt(ch, argument);
//    GET_PROMPT(ch) = argument;
//    send_to_char(ch, "Prompt set to: %s&n\r\n", GET_PROMPT(ch));//DisplayPrompt(ch));
//  }               
}

char StatColors(int cur, int max)
{            
  int num;              

  num = (100 * cur) / max;      

  if (num >= 80)                  
    return 'W';
  else if (num >= 65)               
    return 'G';
  else if (num >= 50)
    return 'Y';
  else if (num >= 30)
    return 'M';       
  else
    return 'R';
}


char *DisplayPrompt(struct char_data *ch)
{
/*  char *prompt, *i = NULL, *point;
  static char buf[MSL], buf2[MIL];
*/

  char *point;
  const char *i=NULL, *prompt;
  char buf[MSL], buf2[MSL];


  int port;


  port = DFLT_PORT;


  if (!GET_PROMPT(ch) || !*GET_PROMPT(ch)) {
    if (GET_LEVEL(ch) < LVL_GOD) {
    sprintf(buf, "&B<%h&B/%Hhp %m&B/%Mmn %v&B/%Vmv>&n");
/*    sprintf(buf, "&B[&%c%ld&Bhp &%c%ld&Bmn &%c%ld&Bmv ",
             StatColors(GET_HIT(ch), GET_MAX_HIT(ch)), GET_HIT(ch),
             StatColors(GET_MANA(ch), GET_MAX_MANA(ch)), GET_MANA(ch),
             StatColors(GET_MOVE(ch), GET_MAX_MOVE(ch)), GET_MOVE(ch));
*/
    return buf;
    }  else {
    sprintf(buf, "&B<%h&B/&W%H&Bhp %m&B/&W%M&Bmn %v&B/&W%V&Bmv>&n");
//&B[&cRoom %d: &g%s&B]&n ", world[IN_ROOM(ch)].number, world[IN_ROOM(ch)].name);

   return buf;
    }
  }

  point = buf;
  prompt = GET_PROMPT(ch);


  for ( ; *prompt != '\0' ; ) {
    if (*prompt != '%') {
      *point++ = *prompt++;
      continue;
    }

    switch(*(++prompt)) {                  
       case 'a': // ARMOR              
          sprintf(buf2, "%d", GET_AC(ch));       
          i = buf2;       
          break;
       case 'A' : // ALIGNMENT
          sprintf(buf2, "%d", GET_ALIGNMENT(ch));
          i = buf2;
          break;  
       case 'b':   // Current Hitpoints (no color)
         sprintf(buf2, "%ld", GET_HIT(ch));
         i = buf2;               
         break;
       case 'c':   // Current Mana (no color)
         sprintf(buf2, "%ld", GET_MANA(ch));
         i = buf2;              
         break;
       case 'd':   // Damage Roll 
         sprintf(buf2, "%ld", GET_DAMROLL(ch));
         i = buf2;                  
         break;
       case 'D':       // Hit Roll
         sprintf(buf2, "%ld", GET_HITROLL(ch));
         i = buf2;   
         break;       
       case 'e':   // Current Move (no color)
         sprintf(buf2, "%ld", GET_MOVE(ch));
         i = buf2;
         break;
       case 'g':    // Gold on-hand
          sprintf(buf2, "%ld", GET_GOLD(ch));
          i = buf2;
          break;     
       case 'G':   // Gold in-bank
         sprintf(buf2, "%ld", GET_BANK_GOLD(ch));
         i = buf2;         
         break;
       case 'h':   // Current Hitpoints                
         sprintf(buf2, "&%c%ld&n", StatColors(GET_HIT(ch), GET_MAX_HIT(ch)),
              ch->points.hit);
         i = buf2;
         break;
       case 'H':   // Max Hitpoints
         sprintf(buf2, "%ld", GET_MAX_HIT(ch));
         i = buf2;
         break;
       case 'l':   // Current Level
        sprintf(buf2, "%d", GET_LEVEL(ch));
         i = buf2;
         break;
       case 'L':   // Total Level
         sprintf(buf2, "%d", GET_TOTAL_LEVEL(ch));
         i = buf2;
         break;                
       case 'm':   // Current Mana
         sprintf(buf2, "&%c%ld&n", StatColors(GET_MANA(ch), GET_MAX_MANA(ch)),
              ch->points.mana);
         i = buf2;
         break;
       case 'M':   // Max Mana           
         sprintf(buf2, "%ld", GET_MAX_MANA(ch));
         i = buf2;
         break;           
       case 'n':   // Newline
         sprintf(buf2, "\n"); 
         i = buf2;
         break;    
       case 'N':  
         sprintf(buf2, "%s", GET_NAME(ch));       
         i = buf2;
         break;                  
       case 'p':   // Game Port
         if (GET_LEVEL(ch) >= LVL_GOD) {
           sprintf(buf2, "%d", port);
           i = buf2;
         } else {
           sprintf(buf2, "%%d");
           i = buf2;
         }
         break;
       case 'P':    // PK Blood
         sprintf(buf2, "%d", GET_PKBLOOD(ch));
         i = buf2;
         break;   
       case 'q':   // end of line
         sprintf(buf2, "\0");
         i = buf2;
         break;              
       case 'Q':   // Quest points
         sprintf(buf2, "%d", GET_QUESTPOINTS(ch));
          i = buf2;
         break;                     
       case 'r':   // Room Number
         if (GET_LEVEL(ch) >= LVL_IMMORT) {
           sprintf(buf2, "%d", world[IN_ROOM(ch)].number);
           i = buf2; 
         } else {     
           sprintf(buf2, "%%r");             
           i = buf2;
         }        
         break;
       case 'R':   // Room Name    
         if (GET_LEVEL(ch) >= LVL_IMMORT) { 
           sprintf(buf2, "%s", world[IN_ROOM(ch)].name);
           i = buf2; 
          } else {                
           sprintf(buf2, "%%R");                
           i = buf2;       
         }     
         break;
       case 'T':   // Current Target
         if (FIGHTING(ch)) { 
           sprintf(buf2, "%s", GET_NAME(FIGHTING(ch)));
           i = buf2;
         } else {
           sprintf(buf2, "&RNobody&n");
           i = buf2;
         }
         break;
       case 'v':   // Current Move
         sprintf(buf2, "&%c%ld&n", StatColors(GET_MOVE(ch), GET_MAX_MOVE(ch)),
              ch->points.move);
         i = buf2;
         break;
       case 'V':   // Max Move                
         sprintf(buf2, "%ld", GET_MAX_MOVE(ch));
         i = buf2;
         break;
       case 'x':   // Xp
         sprintf(buf2, "%ld", GET_EXP(ch));
         i = buf2;
         break;                  
       case 'X':   // Xp to Level                 
         if (GET_LEVEL(ch) >= LVL_IMMORT)
           sprintf(buf2, "None Needed");
         else
           sprintf(buf2, "%ld" , level_exp(GET_LEVEL(ch) + 1) - GET_EXP(ch)); 
         i = buf2;             
         break;   
       case '%':
         sprintf(buf2, "%%");            
         i = buf2;
         break;   
       default:           
         log("Issue with DisplayPrompt()");
         break;               
       }          
       ++prompt;   

       for ( ; (*point = *i) != '\0' ; )          
         ++point, ++i;

     }
     *point = '\0';
     return buf;
}                               


