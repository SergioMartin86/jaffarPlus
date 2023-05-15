#!/bin/bash

cp race.state.base race.state
if [ $? -ne 0 ]; then echo "Failed to copy initial state"; exit -1; fi

for i in {13..99}
do
  echo "Doing race ${i}"
  cp race.state race.state.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy base ${i}"; exit -1; fi
  jaffar race.jaffar
  if [ $? -ne 0 ]; then echo "Failed to solve race ${i}"; exit -1; fi
  cp jaffar.best.sol sols/race.sol.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy solution for race ${i}"; exit -1; fi
  cp jaffar.best.state skip.state
  if [ $? -ne 0 ]; then echo "Failed to copy state for race ${i}"; exit -1; fi
  cp jaffar.best.state skip.state.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy state for race ${i}"; exit -1; fi
  jaffar skip.jaffar
  if [ $? -ne 0 ]; then echo "Failed to skip to next race ${i}"; exit -1; fi
  cp jaffar.best.sol sols/skip.sol.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy solution for skip ${i}"; exit -1; fi
  cp jaffar.best.state race.state
  if [ $? -ne 0 ]; then echo "Failed to copy state for race ${i}"; exit -1; fi
done
