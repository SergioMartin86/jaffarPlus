#!/bin/bash

scriptFiles=`find ../examples -name "*.jaffar"`

set -e

# Override values
export JAFFAR_OVERRIDE_RETURN_CODE=0
export JAFFAR_DRIVER_OVERRIDE_DRIVER_MAX_STEP=1
export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10
export JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_SHA1="DB25BABE3ECB0203BF33A1FB2D0C06E22D87543C"
export JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_PATH=`realpath ../emulators/quickerNES/testRom.nes`
export JAFFAR_QUICKERNES_OVERRIDE_INITIAL_STATE_FILE_PATH=""

for f in ${scriptFiles}
do
 dirName=`dirname ${f}`
 fileName=`basename ${f}`
 echo ${dirName}
 echo ${fileName}

 pushd ${dirName}
  echo "Testing script file: ${f}"
  jaffar ${fileName}
 popd
done
