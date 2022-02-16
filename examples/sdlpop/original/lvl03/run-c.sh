#!/bin/bash

export JAFFAR_MAX_FRAME_DATABASE_SIZE_MB=2000
export JAFFAR_HASH_AGE_THRESHOLD=10
export OMP_NUM_THREADS=24
export JAFFAR_SAVE_BEST_PATH=$HOME/jaffar/examples/tas/lvl03/jaffar-c.best.sav
export JAFFAR_SAVE_CURRENT_PATH=$HOME/jaffar/examples/tas/lvl03/jaffar-c.current.sav

runJaffar() 
{
 seed=$(((RANDOM<<15|RANDOM)))
 echo "Running Seed: $seed" 
 frame=`jaffar-train --RNGSeed $seed --savFile lvl03c.sav lvl03c.jaffar | tee /dev/stderr | grep "Winning" | cut -d' ' -f6`
 echo "$seed $frame"
 echo "$seed $frame" >> results-c.txt
}

while true
do
 date
  sleep 1
  runJaffar &
 wait
done
