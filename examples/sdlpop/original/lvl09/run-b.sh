#!/bin/bash

export JAFFAR_MAX_FRAME_DATABASE_SIZE_MB=100
export JAFFAR_HASH_AGE_THRESHOLD=10
export OMP_NUM_THREADS=24

runJaffar() 
{
 seed=$(((RANDOM<<15|RANDOM)))
 echo "Running Seed: $seed" 
 frame=`jaffar-train --RNGSeed $seed --savFile lvl09b.sav lvl09b.jaffar | tee /dev/stderr | grep "Winning" | cut -d' ' -f6`
 echo "$seed $frame"
 echo "$seed $frame" >> results-b.txt
}

while true
do
 date
  sleep 1
  runJaffar &
 wait
done
