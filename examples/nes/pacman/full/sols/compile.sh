#!/bin/bash

rm -f compiled.sol

for i in {2..23}
do
  cat full.sol.${i} >> compiled.sol
  echo "|..|........|" >> compiled.sol

  cat skip.sol.${i} >> compiled.sol
  echo "|..|........|" >> compiled.sol
done
