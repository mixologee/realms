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
if %direction% == down
%send% %actor% &rYou jump through the hole in the nest, and are beaten all across your body by sharp limbs as you fall.
%send% %actor% You land face-first on a limb, and the rest of your body drags you down.
%send% %actor% Another limb smashes your ribs - you choke on your own blood.
%send% %actor% Just when you thought it was over, a branch whacks you in the crotch - you can see nothing but stars!
%damage% %actor% %random.100%
%damage% %actor% %random.100%
%damage% %actor% %random.100%
end
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
2 g 15
~
eval check %%findmob.%self.vnum%(4614)%%
if %check% > 0
  %send% %actor% You feel hairy limbs reach down and pull you into the treetops!
  %echoaround% %actor% %actor.name% utters a muffled cry as %actor.heshe% is suddenly lifted up and away into the branches!
  switch %self.vnum%
    case 4617
      set endroom 4711
    break
    case 4634
      set endroom 4712
    break
    case 4635
      set endroom 4713
    break
    case 4692
      set endroom 4614
    break
    case 4693
      set endroom 4615
    break
    case 4694
      set endroom 4616
    break
    case 4695
      set endroom 4617
    break
  done
  %teleport% %actor% %endroom%
  %force% %actor% look
end
~
#4634
<none>~
2 bq 15
~
* No Script
~
#4635
<none>~
2 bq 15
~
* No Script
~
#4688
Serpent stalker~
0 h 100
~
set i 1
while %i% < 100
  if %i% != 1
    wait 3 s
  end
  if %actor.room% == %self.room%
  else
    switch %actor.room.vnum%
case 4688                                                                                                                                 
                 case 4731
      case 4732
      case 4733
      case 4734
      case 4735
      case 4736
      case 4737
      case 4749
      case 4750
      case 4752
      case 4753
      case 4754
      case 4755
        %teleport% %self% %actor.room.vnum%
        %echo% A massive serpent bursts from the murky depths and looms over you, opening its yellow-toothed maw as it readies an attack.
        mkill %actor%
      break
      default
        halt
      break
    done
    eval i %i% + 1
  end
done
~
#4692
<none>~
2 bq 15
~
* No Script
~
#4693
<none>~
2 bq 15
~
* No Script
~
#4694
<none>~
2 bq 15
~
* No Script
~
#4695
<none>~
2 bq 15
~
* No Script
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
#4765
Rischka defiance~
0 g 100
~
wait 5
say You shall not pass!
wait 5
say This forest belongs to the Treefolk. We shall not allow for another human treachery!
~
$~
