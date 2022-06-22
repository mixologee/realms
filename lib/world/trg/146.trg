#14600
Scimitar proc~
1 b 100
~
if (GET_EQ(ch, WEAR_WIELD) &
GET_OBJ_VNUM(GET_EQ(ch, WEAR_WIELD)) == 14608) {
DamAmnt = dice(45, 2);
Dice = dice(15, 6);
if (DamAmnt >=40)
act("Black and red flames leap from your scimitar, searing you and $n!" FALSE, vict, 0, 0, TO_ROOM);
act("Demonic flames leap from $N's scimitar, burning hot into your flesh!" FALSE, vict, 0, 0, TO_VICTIM); }
GET_HIT(vict) -= DamAmnt
}
~
$~
