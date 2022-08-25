#!/bin/bash

export OMP_NUM_THREADS=128

runJaffar()
{
 seed=$(((RANDOM<<15|RANDOM)))
 echo "Running Seed: $seed"
 cat lvl10c.jaffar | sed -e 's/"RNG Value":.*/"RNG Value": '$seed',/g' > lvl10crng.jaffar
 frame=`jaffar lvl10crng.jaffar | tee /dev/stderr | grep "Winning" | cut -d' ' -f6`
 echo "$seed $frame"
 echo "$seed $frame" >> results.txt
}

while true
do
 date
  sleep 1
  runJaffar &
 wait
done
