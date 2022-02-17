#!/bin/bash

export JAFFAR_MAX_FRAME_DATABASE_SIZE_MB=3000
export JAFFAR_HASH_AGE_THRESHOLD=10
export OMP_NUM_THREADS=16

runJaffar() 
{
 seed=$(((RANDOM<<15|RANDOM)))
 echo "Running Seed: $seed" 
 frame=`jaffar-train --RNGSeed $seed lvl01a.sav lvl01a.jaffar | tee /dev/stderr | grep "Winning" | cut -d' ' -f6`
 echo "$seed $frame"
 echo "$seed $frame" >> results-a.txt
}

while true
do
 date
  sleep 1
  OMP_PLACES="{0},{1},{2},{3},{4},{5},{6},{7},{16},{17},{18},{19},{20},{21},{22},{23}" runJaffar &
  sleep 1
  OMP_PLACES="{8},{9},{10},{11},{12},{13},{14},{15},{24},{25},{26},{27},{28},{29},{30},{31}" runJaffar &
 wait
done
