#2400
Mob 2400 Say Trigger~
0 b 10
~
%echo% wildly waves her wand in the air!  %say% ZIPPITY ZAP ZAP BLAM!  %echo%
points her wand at a sneaky little boy but nothing happens.  
~
#2401
vizzi vizi vee~
0 b 10
~
emote points her wand at herself.
wait 1 sec
say Vizzi Vizi VEE!
wait 1 sec
emote shimmers briefly but nothing happens.
~
#2402
Slide~
2 g 100
~
wait 25
%send% %actor% You grab the handrails on either side of the slide and push yourself down!
%teleport% %actor% 2439
~
#2403
Slide 2~
2 g 100
~
wait 7
%echo% &CYou let out a loud WOOP as you zip down the slide!&n
if %random.2% ==1
wait 5
%echo% &RYou lose control of your descent and start flying down the slide!&n
wait 5
%echo% &RYou sail of the end of the slide and crash to the ground!&n
eval dam %actor.hitp% / 10
%damage% %actor% %dam%
%echoaround% &R%actor.name% goes flying off the end of the slide and lands in a pitiful heap!&n
%teleport% %actor% 2435
%force% %actor% look
else
wait 5
%echo% &CYou sail smoothly down the slide!&n
%echoaround% &G%actor.name% slides gracefully off the slide and lands on %actor.hisher% feet!&n
%teleport% %actor% 2435
%force% %actor% look
end
~
#2404
Slide 3~
2 g 100
~
wait 10
%echo% &CYou grab the handrails on either side of the slide and push yourself down!&n
wait 10
%echo% &CYou let out a loud &BWOOP!&C as you zip down the slide!&n
%teleport% %actor% 2439
wait 10
if %random.2% ==1
  wait 25
  %send% %actor.name% You lose control of your descent and start flying down the slide!
  wait 20
  %send% %actor.name% You sail off the end of the slide and crash to the ground!
  eval dam %actor.hitp% / 10
  %damage% %actor% %dam%
  %teleport% %actor% 2435
  %echoaround% %actor.name% %actor.name% goes flying off the end of the slide and lands in a pitiful heap!
else
  wait 25
  %send% %actor.name% You sail smoothly down the slide!
  wait 20
  %send% %actor.name% You slide gracefully down the slide, acrobatically landing on your feet!
  %teleport% %actor% 2435
  %echoaround% %actor.name% %actor.name% slides gracefully off the slide and lands on %actor.hisher% feet!
end
~
#2405
actor send test~
2 g 100
~
wait 10
%send% %actor.name% This is using send actor.name test message without color.
~
$~
