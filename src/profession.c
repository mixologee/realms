/************************************************************************
 *  * OasisOLC - Objects / oedit.c                                 v2.0    *
 *   * Original author: Levork                                              *
 *    * Copyright 1996 by Harvey Gilpin                                      *
 *     * Copyright 1997-2001 by George Greer (greerga@circlemud.org)          *
 *      ************************************************************************/

#include "screen.h"
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "boards.h"
#include "constants.h"
#include "shop.h"
#include "genolc.h"
#include "genobj.h"
#include "genzon.h"
#include "oasis.h"
#include "improved-edit.h"
#include "dg_olc.h"
#include "screen.h"
#include "handler.h"
#include "png.h"

ACMD(do_profession);
ACMD(do_learn);
ACMD(do_chop);
ACMD(do_lumber);
ACMD(do_scales);
ACMD(do_skin);
ACMD(do_leather);
ACMD(do_pelts);
ACMD(do_cook);

void list_profs(struct char_data *ch);
void learn_prof(struct char_data *ch, char *arg);
int check_profs(struct char_data *ch);
void get_lumber(struct char_data *ch);
void list_lumber(struct char_data *ch);
void list_scales(struct char_data *ch);
void list_skins(struct char_data *ch);
void list_leather(struct char_data *ch);

/* external functions */
void get_skin(struct char_data *ch, struct obj_data *sobj);
int get_spell(struct obj_data *obj);

char *prof_info[] = 
{
  "Mining",
  "Smithing",
  "Woodworking",
  "Fishing",
  "Skinning",
  "Tanning",
  "Leatherworking",
  "Cooking"
};

char *lumber_info[] =
{
  "Oak",
  "Pine",
  "Elm",
  "Beech",
  "Birch",
  "Cedar",
  "Fir",
  "Willow"
};

char *scale_info [] =
{
  "Fish",
  "Dragon",
  "Mermaid"
};

char *skin_info [] =
{
  "Deer",
  "Dragon",
  "Ogre",
  "Pig",
  "Beaver",
  "Wolf",
  "Fox",
  "Coyote"
};

char *leather_info[] = 
{
  "Light",
  "Thick",
  "Heavy",
  "Soft",
  "Hard",
  "Hardened",
  "Battle",
  "Studded"
};

char *cook_items[] =
{
  "5500",
  "5501",
  "5502",
  "5503",
  "5504",
  "5505",
  "5506",
  "5507",
  "5508",
  "5509",
  "5510",
  "5511",
  "5512",
  "5513",
  "5514",
  "5515",
  "5516",
  "5517",
  "5518",
  "5519"
};

const char *good_prof(int percent)
{
  if (percent < 0)
    return " error)";
  if (percent == 0)
    return " &rUnknown&n";
  if (percent <= 500)
    return " &mInept&n";
  if (percent <= 1000)
    return " &RPoor&n";
  if (percent <= 2000)
    return " &yAverage&n";
  if (percent <= 2500)
    return " &YDecent&n";
  if (percent <= 3250)
    return " &GAdept&n";
  if (percent <= 4000)
    return " &GSkilled&n";
  if (percent <= 4999)
    return "&cSuperb&n";
  return " &CMastered&n";
}

ACMD(do_profession)
{
  if (IS_NPC(ch))
    return;

  list_profs(ch);
}

void list_profs(struct char_data *ch)
{
  const char *overflow = "\r\n**OVERFLOW**\r\n";
  int sortpos;
  size_t len = 0, nlen;
  char buf2[MAX_STRING_LENGTH];

  for (sortpos = 0; sortpos < MAX_PROFS; sortpos++) {
     if(GET_PROF(ch, sortpos) > 100)
     {
      nlen = snprintf(buf2 + len, sizeof(buf2) - len, "&B[&C%-20s &B-&n  %7s&B]&n\r\n", prof_info[sortpos], good_prof(GET_PROF(ch, sortpos)));
     }
     else
     {
      nlen = snprintf(buf2 + len, sizeof(buf2) - len, "&B[&R%-20s &B-&n  %7s&B]&n\r\n", prof_info[sortpos], good_prof(GET_PROF(ch, sortpos)));
     }
      if (len + nlen >= sizeof(buf2) || nlen < 0)
        break;
      len += nlen;
  }
  if (len >= sizeof(buf2))
    strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

  page_string(ch->desc, buf2, TRUE);
}

ACMD(do_learn)
{
  char *buf='\0';

  if (IS_NPC(ch))
    return;

  if (!*argument) {
    send_to_char(ch, "What do you want to learn?\r\n");
    return;
  }

  one_argument(argument, buf);
 
  learn_prof(ch, buf);
}

int check_profs(struct char_data *ch)
{
  int numprofs=0, i;

  for (i = 0; i < MAX_PROFS; i++) {
     if (GET_PROF(ch, i) > 100)
       numprofs++;
  }

  return(numprofs);
}

void learn_prof(struct char_data *ch, char *buf)
{
   int profnum, base;


   if (is_abbrev(buf, "mining")) {
      profnum = PROF_MINING;
      base = 501;
   } else if (is_abbrev(buf, "smithing")) {
      profnum = PROF_SMITHING;
      base = 501;
   } else if (is_abbrev(buf, "woodworking")) {
      profnum = PROF_WOODWORKING;
      base = 501;
   } else if (is_abbrev(buf, "fishing")) {
      profnum = PROF_FISHING;
      base = 501;
   } else if (is_abbrev(buf, "skinning")) {
      profnum = PROF_SKINNING;
      base = 501;
   } else if (is_abbrev(buf, "tanning")) {
      profnum = PROF_TANNING;
      base = 501;
   } else if (is_abbrev(buf, "leatherworking")) {
      profnum = PROF_LEATHERWORKING;
      base = 501;
   } else if (is_abbrev(buf, "cooking")) {
      profnum = PROF_COOKING;
      base = 501;
   } else {
      send_to_char(ch, "We don't teach that here.");
      return;
   }

   if (GET_PROF(ch, profnum) >= 100) {
      send_to_char(ch, "You have already masterd this profession.");
      return;
   }

   if (check_profs(ch) >= 2) {
     send_to_char(ch, "You have already learned your two professions.\r\n");
     return;
   }
  
  send_to_char(ch, "You have begun you trining in %s.\r\n", buf); 
  SET_PROF(ch, profnum-1, base);
  return;
}

ACMD(do_lumber)
{
  if (IS_NPC(ch))
    return;

  list_lumber(ch);
}

void list_lumber(struct char_data *ch)
{

  const char *overflow = "\r\n**OVERFLOW**\r\n";
  int sortpos;
  size_t len = 0, nlen;
  char buf2[MAX_STRING_LENGTH];

  for (sortpos = 0; sortpos < MAX_WOOD; sortpos++) {
     nlen = snprintf(buf2 + len, sizeof(buf2) - len, "&B[&C%-20s &B-&n  %7ld&B]&n\r\n", lumber_info[sortpos], GET_LUMBER(ch, sortpos));
      if (len + nlen >= sizeof(buf2) || nlen < 0)
        break;
      len += nlen;
  }
  if (len >= sizeof(buf2))
    strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

  page_string(ch->desc, buf2, TRUE);
}


ACMD(do_chop)
{ 
   int ltype;

   if(!*argument) {
     send_to_char(ch, "What type of tree are you chopping down?\r\n");
     return;
   }

  skip_spaces(&argument);

   if (is_abbrev(argument, "oak"))       
      ltype = 0;
   else if (is_abbrev(argument, "pine")) 
      ltype = 1;
   else if (is_abbrev(argument, "elm"))
      ltype = 2;
   else if (is_abbrev(argument, "beech"))
      ltype = 3;
   else if (is_abbrev(argument, "birch"))
      ltype = 4;
   else if (is_abbrev(argument, "cedar"))
      ltype = 5;
   else if (is_abbrev(argument, "fir"))
      ltype = 6;
   else if (is_abbrev(argument, "willow"))
      ltype = 7;
   else {
      send_to_char(ch, "There doesn't see to be a %s tree around.", argument);
      return;
   }

   if (SECT(IN_ROOM(ch)) == SECT_FOREST || SECT(IN_ROOM(ch)) == SECT_CONIFEROUS_FOREST || SECT(IN_ROOM(ch)) == SECT_JUNGLE || SECT(IN_ROOM(ch)) == SECT_DECIDUOUS_FOREST) {
     send_to_char(ch, "You begin to chop at a %s tree.\r\n", lumber_info[ltype]);
     SET_BIT_AR(PLR_FLAGS(ch), PLR_LUMBER);
     GET_LUMBER_TYPE(ch) = ltype;
     GET_CHOP_TIME(ch)  = rand_number(5,10);
    }
    else
      send_to_char(ch, "You must be in a forest to chop down trees.\r\n");

    if(GET_LEVEL(ch) > 40)
      GET_CHOP_TIME(ch) = 1;

return;
}

void get_lumber(struct char_data *ch)
{
  int mat, type;

  if (IS_NPC(ch))
     return;

  type = GET_LUMBER_TYPE(ch);

  if(GET_PROF(ch, PROF_WOODWORKING) > 100)
  {
    if (GET_PROF(ch, PROF_WOODWORKING) > rand_number(0,5000))
    {
       mat = rand_number(10,25);
       SET_LUMBER(ch, type, (GET_LUMBER(ch, type) + mat));
       send_to_char(ch, "You received %d pieces of lumber from this %s tree.\r\n", mat, lumber_info[type-1]);
       SET_PROF(ch, PROF_WOODWORKING, MIN((GET_PROF(ch, PROF_WOODWORKING) + rand_number(10,25)), 5000));
       GET_LUMBER_TYPE(ch) = 0;
    }
    else 
    {
       mat = rand_number(1,3);
       SET_LUMBER(ch, type, (GET_LUMBER(ch, type) + mat));
       send_to_char(ch, "You receive %d pieces of lumber from this %s tree.\r\n", mat, lumber_info[type-1]);
       SET_PROF(ch, PROF_WOODWORKING, MIN((GET_PROF(ch, PROF_WOODWORKING) + rand_number(10,25)), 5000));
       GET_LUMBER_TYPE(ch) = 0;
    }
  }

 return;
}

void get_skins(struct char_data *ch, struct obj_data *sobj)
{
  int mat, skin, scale;

  if (IS_NPC(ch))
     return;

  skin = rand_number(1, MAX_PELTS);
  scale = rand_number(1, MAX_SCALES);

  if(GET_PROF(ch, PROF_SKINNING) > 100)
  {
    if (GET_PROF(ch, PROF_SKINNING) > rand_number(0,5000))
    {
       mat = rand_number(10,25);
       if (OBJ_FLAGGED(sobj, ITEM_SKINNABLE))
       {
         SET_SKINS(ch, skin-1, (GET_SKINS(ch, skin-1) + mat));
         send_to_char(ch, "You received %d %s pelts from %s.\r\n", mat, skin_info[skin-1], sobj->short_description);
         SET_PROF(ch, PROF_SKINNING, MIN((GET_PROF(ch, PROF_SKINNING) + rand_number(10,25)), 5000));
         obj_from_char(sobj);
       }

       if (OBJ_FLAGGED(sobj, ITEM_SCALEABLE))
       {
         SET_SCALES(ch, scale-1, (GET_SCALES(ch, scale-1) + mat));
         send_to_char(ch, "You received %d %s scales from %s.\r\n", mat, scale_info[scale-1], sobj->short_description);
         SET_PROF(ch, PROF_SKINNING, MIN((GET_PROF(ch, PROF_SKINNING) + rand_number(10,25)), 5000));
         obj_from_char(sobj);
       }
    }
    else
    {
       mat = rand_number(1,10);
       if (OBJ_FLAGGED(sobj, ITEM_SKINNABLE))
       {
         SET_SKINS(ch, skin-1, (GET_SKINS(ch, skin-1) + mat));
         send_to_char(ch, "You received %d pelts from %s.\r\n", mat, skin_info[skin-1]);
         SET_PROF(ch, PROF_SKINNING, MIN((GET_PROF(ch, PROF_SKINNING) + rand_number(10,25)), 5000));
         obj_from_char(sobj);
       }

       if (OBJ_FLAGGED(sobj, ITEM_SCALEABLE))
       {
         SET_SCALES(ch, scale-1, (GET_SCALES(ch, scale-1) + mat));
         send_to_char(ch, "You received %d %s scales from %s.\r\n", mat, scale_info[scale-1], sobj->short_description);
         SET_PROF(ch, PROF_SKINNING, MIN((GET_PROF(ch, PROF_SKINNING) + rand_number(10,25)), 5000));
         obj_from_char(sobj);
       }
    }
  }
  else
  {
       mat = rand_number(0,2);
       if (OBJ_FLAGGED(sobj, ITEM_SKINNABLE))
       {
         SET_SKINS(ch, skin-1, (GET_SKINS(ch, skin-1) + mat));
         send_to_char(ch, "You received %d %s pelts from %s.\r\n", mat, skin_info[skin-1], sobj->short_description);
         SET_PROF(ch, PROF_SKINNING, MIN((GET_PROF(ch, PROF_SKINNING) + rand_number(5,10)), 5000));
         obj_from_char(sobj);
       }

       if (OBJ_FLAGGED(sobj, ITEM_SCALEABLE))
       {
         SET_SCALES(ch, scale-1, (GET_SCALES(ch, scale-1) + mat));
         send_to_char(ch, "You received %d %s scales from %s.\r\n", mat, scale_info[scale-1], sobj->short_description);
         SET_PROF(ch, PROF_SKINNING, MIN((GET_PROF(ch, PROF_SKINNING) + rand_number(5,10)), 5000));
         obj_from_char(sobj);
       }
   }
   
 return;
}


ACMD(do_skin)
{
  char arg[MAX_INPUT_LENGTH];
  struct obj_data *obj;



  one_argument(argument, arg);

  if (!*arg)
  {
    send_to_char(ch, "Skin what?");
    return;
  }


//   if (!(obj = get_obj_in_list_vis(ch, arg, NULL, world[IN_ROOM(ch)].contents)))
//      send_to_char(ch, "You don't see %s %s here.\r\n", AN(arg), arg);
//   else 
   if (!(obj = get_obj_in_list_vis(ch, arg, NULL, ch->carrying)))
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(arg), arg);
   else
   {
    if (OBJ_FLAGGED(obj, ITEM_SKINNABLE) || OBJ_FLAGGED(obj, ITEM_SCALEABLE))
      get_skins(ch,obj);
    else
      send_to_char(ch, "That item is not skinnable.\r\n");
   }
 
 return;
}

ACMD(do_scales)
{
  if (IS_NPC(ch))
    return;

  list_scales(ch);
}

void list_scales(struct char_data *ch)
{

  const char *overflow = "\r\n**OVERFLOW**\r\n";
  int sortpos;
  size_t len = 0, nlen;
  char buf2[MAX_STRING_LENGTH];

  for (sortpos = 0; sortpos < MAX_SCALES; sortpos++) {
     nlen = snprintf(buf2 + len, sizeof(buf2) - len, "&B[&C%-20s &B-&n  %7ld&B]&n\r\n", scale_info[sortpos], GET_SCALES(ch, sortpos));
      if (len + nlen >= sizeof(buf2) || nlen < 0)
        break;
      len += nlen;
  }
  if (len >= sizeof(buf2))
    strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

  page_string(ch->desc, buf2, TRUE);
}

ACMD(do_pelts)
{
  if (IS_NPC(ch))
    return;

  list_skins(ch);
}

void list_skins(struct char_data *ch)
{

  const char *overflow = "\r\n**OVERFLOW**\r\n";
  int sortpos;
  size_t len = 0, nlen;
  char buf2[MAX_STRING_LENGTH];

  for (sortpos = 0; sortpos < MAX_PELTS; sortpos++) {
     nlen = snprintf(buf2 + len, sizeof(buf2) - len, "&B[&C%-20s &B-&n  %7ld&B]&n\r\n", skin_info[sortpos], GET_SKINS(ch, sortpos));
      if (len + nlen >= sizeof(buf2) || nlen < 0)
        break;
      len += nlen;
  }
  if (len >= sizeof(buf2))
    strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

  page_string(ch->desc, buf2, TRUE);
}

ACMD(do_leather)
{
  if (IS_NPC(ch))
    return;

  list_leather(ch);
}

void list_leather(struct char_data *ch)
{

  const char *overflow = "\r\n**OVERFLOW**\r\n";
  int sortpos;
  size_t len = 0, nlen;
  char buf2[MAX_STRING_LENGTH];

  for (sortpos = 0; sortpos < MAX_LEATHER; sortpos++) {
     nlen = snprintf(buf2 + len, sizeof(buf2) - len, "&B[&C%-20s &B-&n  %7ld&B]&n\r\n", leather_info[sortpos], GET_LEATHER(ch, sortpos));
      if (len + nlen >= sizeof(buf2) || nlen < 0)
        break;
      len += nlen;
  }
  if (len >= sizeof(buf2))
    strcpy(buf2 + sizeof(buf2) - strlen(overflow) - 1, overflow); /* strcpy: OK */

  page_string(ch->desc, buf2, TRUE);
}

ACMD(do_cook)
{
   char comp1[MAX_INPUT_LENGTH],comp2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH], comp3[MAX_INPUT_LENGTH];
   struct obj_data *obj1, *obj2, *obj3, *potion;
   int sp1=-1, sp2=-1, sp3=-1;
   
   half_chop(argument, buf, comp1);
   two_arguments(buf, comp2, comp3);


   if(!*argument)
   {
     send_to_char(ch, "&RUsage: cook <ingredient> <ingredient> <ingredient>&n\r\n");
     return;
   }

   if(!*comp1 || !*comp2 || !*comp3)
   {
     send_to_char(ch, "&RUsage: cook <ingredient> <ingredient> <ingredient>&n\r\n");
     return;
   }

   if (!(obj1 = get_obj_in_list_vis(ch, comp1, NULL, ch->carrying)))
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(comp1), comp1);
   else
   {
    if (OBJ_FLAGGED(obj1, ITEM_COOKABLE))
      sp1 = get_spell(obj1);
    else
      send_to_char(ch, "That item is not cookable.\r\n");
   }

   if (!(obj2 = get_obj_in_list_vis(ch, comp2, NULL, ch->carrying)))
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(comp2), comp2);
   else
   {
    if (OBJ_FLAGGED(obj2, ITEM_COOKABLE))
      sp2 = get_spell(obj2);
    else
      send_to_char(ch, "That item is not cookable.\r\n");
   }

   if (!(obj3 = get_obj_in_list_vis(ch, comp3, NULL, ch->carrying)))
      send_to_char(ch, "You don't see %s %s here.\r\n", AN(comp3), comp3);
   else
   {
    if (OBJ_FLAGGED(obj3, ITEM_COOKABLE))
      sp3 = get_spell(obj3);   
    else
      send_to_char(ch, "That item is not cookable.\r\n");
   }

   potion = read_object(5550, REAL);

   GET_OBJ_VAL(potion,0) = GET_LEVEL(ch);
   GET_OBJ_VAL(potion,1) = sp1;
   GET_OBJ_VAL(potion,2) = sp2;
   GET_OBJ_VAL(potion,3) = sp3;

   obj_to_char(potion, ch);

   return;
}

int get_spell(struct obj_data *obj)
{
 int spell=-1;

 return(spell);
}
