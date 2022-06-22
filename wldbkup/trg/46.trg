#4600
Thing stab~
0 kt 20
~
%send% %actor% A bloodsucking thing with wings stabs you with its needle, drawing blood!
%damage% %actor% %random.20%
%damage% %actor% %random.20%
~
#4612
Rischka prayer~
0 g 0
~
THIS IS BROKEN
4
real
~
#4613
Falling~
2 q 100
~
%send% %actor% &rYou jump through the hole in the nest, and are beaten all across your body by sharp limbs as you fall.
%send% %actor% You land face-first on a limb, and the rest of your body drags you down.
%send% %actor% Another limb smashes your ribs - you choke on your own blood.
%send% %actor% Just when you thought it was over, a branch whacks you in the crotch - you can see nothing but stars!
%damage% %actor% %random.100%
%damage% %actor% %random.100%
%damage% %actor% %random.100%
~
#4614
Spider bite~
0 k 0
~
switch %random.3%
case 1
%echo% The spider leaps on you and sticks its fangs into your neck!
dg_cast 'poison' %actor.name%
%echo% You feel very sick.
break
case 2
%echo% The spider leaps on you, but you manage to throw it off before it bites you!
return 0
break
default
return 0
break
done
~
#4617
Send to Queens Nest~
2 bq 0
~
%echo% You feel hairy limbs reach down and pull you up into the treetops!
%echoaround%$n utters a muffled cry as $e is suddenly lifted up and away into the branches!
%teleport% %actor% 4711
~
#4634
Send to Queens Nest~
2 bq 15
~
%echo% You feel hairy limbs reach down and pull you up into the treetops!
%echoaround%$n utters a muffled cry as $e is suddenly lifted up and away into the branches!
%teleport% %actor% 4712
~
#4635
Send to Queens Nest~
2 bq 15
~
%echo% You feel hairy limbs reach down and pull you up into the treetops!
%echoaround%$n utters a muffled cry as $e is suddenly lifted up and away into the branches!
%teleport% %actor% 4713
~
#4688
Serpent stalker~
0 b 100
~
%echo% This trigger commandlist is not complete!
~
#4692
Send to Queens Nest~
2 bq 15
~
%echo% You feel hairy limbs reach down and pull you up into the treetops!
%echoaround%$n utters a muffled cry as $e is suddenly lifted up and away into the branches!
%teleport% %actor% 4702
~
#4693
Send to Queens Nest~
2 bq 15
~
%echo% You feel hairy limbs reach down and pull you up into the treetops!
%echoaround%$n utters a muffled cry as $e is suddenly lifted up and away into the branches!
%teleport% %actor% 4705
~
#4694
Send to Queens Nest~
2 bq 15
~
%echo% You feel hairy limbs reach down and pull you up into the treetops!
%echoaround%$n utters a muffled cry as $e is suddenly lifted up and away into the branches!
%teleport% %actor% 4713
~
#4695
Send to Queens Nest~
2 bq 15
~
%echo% You feel hairy limbs reach down and pull you up into the treetops!
%echoaround%$n utters a muffled cry as $e is suddenly lifted up and away into the branches!
%teleport% %actor% 4709
~
#4761
Shade moan~
0 b 100
~
emote &Gmoans in pure agony.
~
#4762
Araknarok greeting~
0 g 100
~
say Hello there.
~
#4763
Spider Queen speech~
0 g 100
~
say Who dares enter the chamber of my sacred office unannounced? I seek no harm of dryad or elf.
~
#4764
Mother Caiman proc~
0 kt 10
~
%echo% The massive mother caiman swings its tail around, knocking you aside!
%damage% %actor% %random.75%
~
$~
