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
prompt~
0 c 100
~
if %self% != %actor%
  halt
  return 0
end
if %cmd% == help
  set prompt 'prompt
  if %prompt.contains('%arg%)% && %arg.contains(pr)%
    %send% %self% Syntax:
    %send% %self% fprompt <prompt>
    %send% %self% fprompt default
    %send% %self% prompt default
    %send% %self% prompt <prompt> 
    %send% %self% tcolor
    %send% %self% @n
    %send% %self% @RImmortal ONLY!:@n 
    %send% %self% @n
    %send% %self% Prompt default will set your prompt to a default setting, with only the
    %send% %self% bare minimum of information posted in your prompt.
    %send% %self% @n
    %send% %self% Tcolor stands for "toggle color". This will automate percentile coloring
    %send% %self% for life/mana/move/ and opponent's hp in your fprompt.
    %send% %self% @n
    %send% %self% Typing prompt with anything following it other than default will alter your
    %send% %self% prompt to be whatever follows it. ie. To have <current hp>/<max hp> as your
    %send% %self% prompt you would type: 'prompt #H/#MH'.
    %send% %self% @n
    %send% %self% Valid Fields for the prompt are as follows:
    %send% %self% @n
    %send% %self% Prompt Commands are:
    %send% %self% #H    - current hp
    %send% %self% #MH   - max hp
    %send% %self% #MN   - current mana
    %send% %self% #MM   - max mana
    %send% %self% #V    - current move
    %send% %self% #MV   - max move
    %send% %self% #G    - current gold
    %send% %self% #EXP  - current experience
    %send% %self% #S    - room sector
    %send% %self% #EXIT - displays exits in current room
    %send% %self% @n
    %send% %self% #N    - GOD ONLY displays room vnum
    %send% %self% @n
    %send% %self% When using the scripted prompt we suggest that you turn off the hard-coded prompt,
    %send% %self% this can be done by typing 'display' off.
  else
    return 0
  end
elseif %cmd% == prompt || %cmd% == fprompt
  if %cmd% == prompt%
    set revcmd fprompt
  else
    set revcmd prompt
  end
  if %arg% == default
    rdelete s%cmd% %self.id%
    %send% %self% %cmd% set to default. Remember that this does not delete the other
    %send% %self% your %revcmd%.
  else
    if %arg.contains(actor)% || %arg.contains(self)%
      %teleport% %self% 10 *arbitrary room number, a room with a command script with no script but a * as the arg,
      *sole intent is to freeze cheaters
      nop %self.title(I was caught attempting to cheat with the prompt, now I'm being punished.)%
      halt
    end
    set s%cmd% %arg%
    remote s%cmd% %self.id%
    %send% %self% %cmd% has been set.
  end
elseif %cmd% == tcolor
  if %self.varexists(tcolor)%
    rdelete tcolor %self.id%
    %send% %self% Tcolor is now off.
  else
    set tcolor 1
    remote tcolor %self.id%
    %send% %self% Tcolor is now on!
  end
else
  return 0
  wait 1
  if %self.fighting%
    set var f
  end
  if %self.varexists(s%var%prompt)%
    eval p %%self.s%var%prompt%%
  else
    set p #H/#MH >
  end
  set n %p.strlen%
  set i 1
  while %i% <= %n%
    set c %p.charat(%i%)%
    if !%c%
      set out @n @n
    elseif %c% == #
      set in_var 1
      unset var
      while %in_var%
        eval i %i% + 1
        set c %p.charat(%i%)%
        eval 2 %2% + 1
        switch %c%
          case #
            set out %out%#
            set in_var 0
          break
          case H
            set var %var%H
          break
          case P
            set var %var%P
          break
          case M
            set var %var%M
          break
          case V
            set var %var%V
          break
          case G
            set var %var%G
          break
          case E
            set var %var%E
          break
          case X
            set var %var%X
          break
          case I
            set var %var%I
          break
          case T
            set var %var%T
          break
          case S
            set var %var%S
          break
          case N
            set var %var%N
          break
          case O
            set var %var%O
          break
        done
        switch %var%
          case H
            set out %self.hitp%
            set in_var 0
            set max %self.maxhitp%
          break
          case MH
            set out %self.maxhitp%
            set in_var 0
          break
          case MN
            set out %self.mana%
            set max %self.maxmana%
            set in_var 0
          break
          case MM
            set out %self.maxmana%
            set in_var 0
          break
          case V
            set out %self.move%
            set max %self.maxmove%
            set in_var 0
          break
          case MV
            set out %self.maxmove%
            set in_var 0
          break
          case G
            set out %self.gold%
            set in_var 0
          break
          case EXP
            set out %self.exp%
            set in_var 0
          break
          case S
            set out %self.room.sector%
            set in_var 0
          break
          case EXIT
            set exits Exits:
            set 1 north
            set 2 east
            set 3 south
            set 4 west
            set 5 up
            set 6 down
            set d 1
            while %d% < 7
              eval dir %%%d%%%
              eval check %%self.room.%dir%%%
              if %check%
                eval door %%self.room.%dir%(bits)%% /= closed
                if %door%
                  set exits %exits% (%dir%)
                else
                  set exits %exits% %dir%
                end
              end
              eval d %d% + 1
            done
            set out %exits%
            set in_var 0
          break
          case N
            if %self.level% > 30
              set out %self.room.vnum%
            else
              set out %c%
            end
            set in_var 0
          break
          case OH
            if %self.fighting%
              eval oh ((%self.fighting.hitp% / %self.fighting.maxhitp%) * 100)
              set out %oh%
            else
              set out %c%
            end
            set in_var 0
          break
        done
        if %self.varexists(tcolor)%
          if %max% || %out% == %oh%
            if %out% == %oh%
              set per %oh%
            else
              eval per ((%out% / %max%) * 100)
            end
            unset max
            if %per% > 90
              set col @W%out%@n
            elseif %per% > 70
              set col @G%out%@n
            elseif %per% > 40
              set col @Y%out%@n
            elseif %per% > 10
              set col @R%out%@n
            else
              set col @D%out%@n
            end
            set out %col%
          end
        end
      done
    else
      set out %c%
    end
    set prmpt %prmpt%%out%
    eval i %i% + 1
  done
  %send% %self% %prmpt%
end 
~
$~
