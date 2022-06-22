#400
just in case open mud on reset~
2 f 100
~
%door% 462 up room 429
%door% 429 down room 462
set is_open 0
global is_open
~
#401
remove mud door~
2 g 100
~
if %is_open% == 0
%echo% The mud fills in the hole above you.
%door% 462 up purge 0
%door% 429 down purge 0
set is_open 1
global is_open
end
~
#402
open mud after death~
2 c 100
push~
if %arg% !=mud
return 0
else
if %is_open% == 0
return 0
else
%door% 462 up room 429
%door% 429 down room 462
set is_open 0
global is_open
end
end
~
#403
rat poison~
0 k 100
~
* No Script
~
#404
rat poison~
0 k 25
~
switch %random.3%
case 1
%echo% One of the rats swings his tail launching a dart straight into your leg.
dg_cast 'poison' %actor.name%
%echo% You feel very sick.
break
case 2
%echo% One of the rats swings his tail launching a dart at you but nearly misses!
return 0
break
default
return 0
break
done
~
#405
Trip Room~
2 g 100
~
if %actor.is_pc%
  if %random.6% < 2
     %send% %actor% You slip climbing down the muddy rocks and dirty the seat of your pants.
     %echoaround% %actor% %actor.name% slips, slides and falls on %hisher% bottom.
nop %actor.pos(sitting)%
  end
end
~
#418
Bottom Cliff~
2 g 100
~
if %actor.is_pc%
   if %direction% == west
       if %actor.dex% < 18
%send% %actor% As you slide down the cliff you slide along a sharp edge, ending with it catching on your chin. Ouch!
            %echoaround% %actor% %actor.name% slides down the cliff face, looking slightly pained.
        return 0
    else
%send% %actor% You slip going down the cliff, but manage to avoid the sharp edges.
            %echoaround% %actor% %actor.name% slides down the cliff face, missing all the sharp rocks.
      end
    end
  end
~
#424
Bubbling Pool~
2 b 10
~
%echo% Small bubbles rise from the pool, popping loudly at the surface.
~
#425
Waterfall Knockdown~
2 g 100
~
wait 15 sec
if !%actor.affect(WATERWALK)%
  if !%actor.affect(FLYING)%
   %teleport% %actor% 427
  end
end
~
#467
Ice Slip~
2 q 100
~
if %actor.is_pc%
if %random.20% < 5
    %send% %actor% Your feet fly out from under you, and you land hard on the ice.
    %echoaround% %actor% %actor.name%'s feet fly out from under %himher%, and %heshe% lands hard on the ice.
%damage% %actor% %random.4%
return 0
  end
end
~
$~
