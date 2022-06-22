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
wait 10
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
%send% %actor.name% &CYou grab the handrails on either side of the slide and push yourself down!&n
%teleport% %actor% 2439
wait 20
%send% %actor.name% &CYou let out a loud WOOP as you zip down the slide!&n
 if %random.2% ==1
  wait 15
  %send% %actor.name% &RYou lose control of your descent and start flying down the slide!&n
  wait 15
  %send% %actor.name% &RYou sail off the end of the slide and crash to the ground!&n
  eval dam %actor.hitp% / 10
  %damage% %actor% %dam%
  %teleport% %actor% 2435
  %echoaround% %actor.name% &R%actor.name% goes flying off the end of the slide and lands in a pitiful heap!&n
 else
  wait 15
  %send% %actor.name% &CYou sail smoothly down the slide!&n
  wait 15
  %teleport% %actor% 2435
  %send% %actor.name% &GYou slide gracefully off the slide and land on your feet!&n
  %echoaround% %actor.name% &G%actor.name% slides gracefully off the slide and lands on %actor.hisher% feet!&n
end
~
$~
