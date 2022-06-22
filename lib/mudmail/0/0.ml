MlID: 0
Send: 1
Reci: 83
Sent: 1240256159
Body:
\/h
  if(!IS_NPC(victim))
  vicbonus += ((GET_SKILL(victim, SKILL_PARRY)/20)+(GET_SKILL(victim, SKILL_ADVANCED_PARRY)/20) + (GET_SKILL(victim, SKILL_ADVANCED_DODGE)/20) + (GET_SKILL(victim, SKILL_DODGE)/20))

  if (victim_ac >=0)
    vicroll = ((100 - victim_ac))
  else
    vicroll = ((100 - victim_ac))
  chroll = GET_HITROLL(ch) - ((20 - GET_DEX(ch)) / 2)
  if(AFF_FLAGGED(ch, AFF_SNEAK))
    chroll += 20
  if(IS_THIEF(ch) && type == SKILL_RANGE_COMBAT)
    chroll += GET_HITROLL(ch)
  diceroll = rand_number(1, vicroll)
  if (chroll > (vicroll+vicbonus))
     dam = TRUE
  else if (diceroll < chroll)
     dam = TRUE
  else
    dam = FALSE

~
Gold: 0
Flag: f 0 0 0
$~
