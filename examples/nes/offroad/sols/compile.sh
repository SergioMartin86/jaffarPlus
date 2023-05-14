#!/bin/bash

rm -f compiled.sol

for i in {0..99}
do
  cat race.sol.${i} >> compiled.sol
  if [ $? -eq 0 ]; then echo "|..|........|" >> compiled.sol; fi

  cat skip.sol.${i} >> compiled.sol
  if [ $? -eq 0 ]; then echo "|..|........|" >> compiled.sol; fi
done
