#!/bin/bash
# run.sh <scenario> [param]  -> forge, bootstrap, magnet-search, report furthest progress
set -e
cd /tmp/chamber
export ASAN_OPTIONS="detect_leaks=0"; export OMP_NUM_THREADS=64
J=/home/jaffar/jaffarPlus/build-sdlpop/jaffar
P=/home/jaffar/jaffarPlus/build-sdlpop/jaffar-player
SCEN=$1; PARAM=$2
python3 chamber.py $SCEN $PARAM >/dev/null
python3 -c "print('\n'.join(['|.|.....|']*${BOOT:-60}))" > null.sol
$P pipe.jaffar null.sol --reproduce --disableRender --exitOnEnd --unattended --runCommand s 2>/dev/null >/dev/null
mv -f quicksave.state chamber.state
timeout 120 $J chamber.jaffar 2>/dev/null | grep -iE "Exit Reason" | tail -1
# reproduce furthest-progress best.sol; report max-x and the trajectory around it
$P chamber.jaffar /tmp/jaffar.best.sol --reproduce --disableRender --exitOnEnd --unattended 2>/dev/null \
 | awk '/Global Step Counter/{match($0,/[0-9]+/);s=substr($0,RSTART,RLENGTH)+0}
   /\[Kid\]/{if(match($0,/Pos.x: *[0-9]+/)){x=substr($0,RSTART,RLENGTH);gsub(/[^0-9]/,"",x)+0}
     if(match($0,/Pos.y: [0-9.]+ \(([0-9]+)\)/,yy)){y=yy[1]}
     if(match($0,/Frame: *[0-9]+/)){f=substr($0,RSTART,RLENGTH);gsub(/[^0-9]/,"",f)}
     if(match($0,/Action: *[0-9]+/)){a=substr($0,RSTART,RLENGTH);gsub(/[^0-9]/,"",a)}
     print s,x,y,f,a}' \
 | awk '{print > "/tmp/chamber/traj.txt"} {if($2+0>mx){mx=$2;mxs=$0}} END{print "  MAX-X reached: "mx"  (at "mxs")"}'
echo "  trajectory tail:"; tail -6 /tmp/chamber/traj.txt | awk '{print "    step"$1" x="$2" y="$3" f="$4" a="$5}'
