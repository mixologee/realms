#19800
Load Mob~
2 f 100
~
%load% mob 19800
detach 19800 %self.id%
~
#19801
Death Script To Reload~
0 f 100
~
%teleport% %self% 19800
attach 19800 %self.room.id%
%teleport% %self% %actor%
~
#19802
Set Sneak and Purge Self~
0 n 100
~
if %self.room.vnum% == 19800
  eval num %random.100% - 1
  set i 0
  while %i% < 101
    while %num.vnum% == 0
      eval num %random.100% - 1
    done
    eval i %i% + 1
  done
  mgoto %num%
  attach 103 %self.room.id%
  %purge% %self%
else
  set people %self.room.people%
  while %people%
    if %people.is_pc%
      emote shouts Hi-Keeba!
      backstab %actor.name%
      halt
    else
      set people %people.next_in_room%
    end
  done
end
~
#19803
Load Mob On Entrance~
2 g 100
~
%load% mob 19800
detach 19803 %self.id%
~
$~
