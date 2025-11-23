#!/bin/bash

scriptName="_runScript.jaffar"
emuConfig=`jaffar-player-pipe pipeDreamScript.jaffar pipeDreamInput.sol --disableRender --runCommand t`
cat pipeBotScript.jaffar | sed -e "s/__REPLACE__/${emuConfig}/g" > ${scriptName}
jaffar ${scriptName}
playerStartRow=`jaffar-player ${scriptName} pipeBotInput.sol --disableRender --runCommand t | head -n 1 | cut -d ':' -f 2`
playerStartCol=`jaffar-player ${scriptName} pipeBotInput.sol --disableRender --runCommand t | tail -n 1 | cut -d ':' -f 2`
python3 ./postprocess.py /tmp/jaffar.best.sol ${playerStartRow} ${playerStartCol} > _output.sol 