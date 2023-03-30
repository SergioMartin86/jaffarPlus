#!/bin/bash

cp full.state.base full.state
if [ $? -ne 0 ]; then echo "Failed to copy initial state"; exit -1; fi

for i in {2..23}
do
  echo "Doing level ${i}"
  cp full.state full.state.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy base ${i}"; exit -1; fi
  jaffar full.jaffar
  if [ $? -ne 0 ]; then echo "Failed to solve level ${i}"; exit -1; fi
  cp /tmp/jaffar.best.sol sols/full.sol.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy solution for level ${i}"; exit -1; fi
  cp /tmp/jaffar.best.state skip.state
  if [ $? -ne 0 ]; then echo "Failed to copy state for level ${i}"; exit -1; fi
  cp /tmp/jaffar.best.state skip.state.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy state for level ${i}"; exit -1; fi
  jaffar skip.jaffar
  if [ $? -ne 0 ]; then echo "Failed to skip to next level ${i}"; exit -1; fi
  cp /tmp/jaffar.best.sol sols/skip.sol.${i}
  if [ $? -ne 0 ]; then echo "Failed to copy solution for skip ${i}"; exit -1; fi
  cp /tmp/jaffar.best.state full.state
  if [ $? -ne 0 ]; then echo "Failed to copy state for level ${i}"; exit -1; fi
done
