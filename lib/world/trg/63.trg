#6315
Eat your soul..~
0 b 10
~
wait 1 sec
say May I eat your soul?
wait 5 sec
say MAY I?
end
~
#6321
dog trigger~
0 g 100
~
if %object.type% == FOOD 
  wait 1 sec 
  emote swallows %object.shortdesc% without even chewing. 
  wait 1 sec 
  emote looks up at %actor.name%, hoping for some more. 
  %purge% %object% 
else 
  wait 1 s 
  drop %object.name.car% 
end
~
#6324
hobo trigger~
0 b 10
~
say Please, someone, I need water.  My throat is parched!
~
#6325
dog lick~
0 g 100
~
lick %actor.name%
~
#6326
ello dearie~
0 g 100
~
say 'ello dearie!
~
#6328
keepsies~
0 b 10
~
say You should know, I only play for keepsies.
~
#6329
Flix marble~
0 g 100
~
get marble
%purge% marble
wait 3
%echo% %self.name% flicks a marble with pure concentration.
%load% obj 6328 %room%
~
#6330
take marble~
0 c 100
get~
if %arg% != marble
return 0
endif
say Hey you can't take that!
mkill %actor%
~
#6449
golden gates~
0 g 100
~
wait 2
%echo% %blue%This is a test.
~
#6452
butterflies~
2 b 10
~
%echo% &MA cloud of multicolored butterflies flutter around the flowers.
~
#6493
riko-pub~
2 b 10
~
%echo%&GYou see the barman wipe down the counter top with a warm damp cloth.
~
#6499
fluffybun's development~
0 b 10
~
eval num %random.8%
switch %num%
case 1
say A trick is something a whore does for money.
wait 2 sec
say Or candy!
break
case 2
say No touching!
break
case 3
say What? Oh, no! It's the cops! Oh! And a .. construction worker.
break
case 4
say There are very few intelligent, attractive, and straight men in this town.
wait 2 sec
say Well, that certainly leaves me out.
wait 2 sec
say You said single, right?
break
case 5
say When a man needs to prove to a woman that he's actually... 
wait 2 sec
say When a man loves a woman, and he actually wants to make love, um, to her, something very, very special happens. And with deep, deep concentration and great focus he is often able to achieve an erec...
wait 2 sec
say I'm sorry, this is gonna get a little - eww - gross.
break
case 6
say Where am I? Am I in two-thirds of a hospital room?
break
case 7
say What would Saddam do?
break
case 8
say Okay, who'd like a banger in the mouth?
break
~
#6504
Cheese Shop~
0 b 100
~
*No Script Set
~
#6505
Bouzuki Loader~
0 d 100
Good Morning to you Sir!~
%load% mob 6334
~
#6506
MapTest~
2 c 3
zmap~
eval width %arg% 
eval display 11 
while %display% <= %width% * 10 + %width% 
set x%display% . 
eval display %display% + 1 
done 
set here %actor.room.vnum% 
* 
set d1 north 
set d2 east 
set d3 south 
set d4 west 
* 
eval %d1% -10 
eval %d2% 1 
eval %d3% 10 
eval %d4% -1 
* 
eval temp (%width% + 1) / 2 
eval middle %temp% * 10 + %temp% 
eval c1 %middle% + %%%d1%%% 
eval c2 %middle% + %%%d2%%% 
eval c3 %middle% + %%%d3%%% 
eval c4 %middle% + %%%d4%%% 
* 
while %dir% < 4 
eval dir %dir% + 1 
eval cur_dir %%d%dir%%% 
eval cur_rm %%here.%cur_dir%%% 
eval start %%c%dir%%% 
if %cur_rm% 
eval %start% %%here.%cur_dir%(vnum)%% 
eval temp %%%start%.sector%% 
eval x%start% %temp.charat(1)% 
eval directionlist %d1% %d2% %d3% %d4% 
while %directionlist% 
eval firstdir %directionlist.car% 
eval newroom %start% + %%%firstdir%%% 
eval %newroom% %%%start%.%firstdir%(vnum)%% 
eval vnumexist %%%newroom%%% 
if %vnumexist% 
eval temp %%%newroom%.sector%% 
eval x%newroom% %temp.charat(1)% 
end 
set directionlist %directionlist.cdr% 
done 
end 
done 
* 
set x%middle% X 
eval i 1 
while %i% <= %width% 
eval j 2 
eval num %i% * 10 + 1 
set msg %%x%num%%% 
while %j% <= %width% 
eval num %i% * 10 + %j% 
set msg %msg% %%x%num%%% 
eval j %j% + 1 
done 
eval msg%i% %msg% 
eval i %i% + 1 
done 
* 
eval i 1 
while %i% <= %width% 
eval themsg %%msg%i%%% 
%send% %actor% %themsg% 
eval i %i% + 1 
done
~
#6507
Get hair from nest~
1 c 100
get~
if %arg% == hair nest
%send% %actor% You carefully pull a thread of silken human hair out of the woven mass of twigs.
%echoaround% %actor% %actor.name% carefully pulls a thread of silken human hair out of the woven mass of twigs.
%load% o 6511 %actor%
%purge% %self%
else
return 0
end
~
$~
