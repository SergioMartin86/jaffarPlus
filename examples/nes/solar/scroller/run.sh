#/bin/bash

END=3000

for i in $(seq 1 $END);
do 
  echo "Running $i"
  jaffar scroller.jaffar > tmp
  tail -n 1000 tmp > runs/run${i}.log
  cat runs/run${i}.log | grep "Winning"
done
