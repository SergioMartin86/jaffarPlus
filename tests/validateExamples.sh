#!/bin/bash

scriptFiles=`find ../examples -name "*.jaffar"`

set -e

# Getting absolute path to the Jaffar engine
jaffarPath=`realpath ../build/jaffar`

# Override values
export JAFFAR_OVERRIDE_RETURN_CODE=0
export JAFFAR_DRIVER_OVERRIDE_DRIVER_MAX_STEP=1
export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10
export JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_SHA1="DB25BABE3ECB0203BF33A1FB2D0C06E22D87543C"
export JAFFAR_QUICKERNES_OVERRIDE_ROM_FILE_PATH=`realpath nes/testRom.nes`
export JAFFAR_QUICKERNES_OVERRIDE_INITIAL_STATE_FILE_PATH=""
export JAFFAR_QUICKERSNES9X_OVERRIDE_ROM_FILE_SHA1="7752CAA676D15D4D0C42F25548C3E86128A416DB"
export JAFFAR_QUICKERSNES9X_OVERRIDE_ROM_FILE_PATH=`realpath snes/testRom.nes`
export JAFFAR_QUICKERSNES9X_OVERRIDE_INITIAL_STATE_FILE_PATH=""

for f in ${scriptFiles}
do
 dirName=`dirname ${f}`
 fileName=`basename ${f}`
 echo ${dirName}
 echo ${fileName}

 pushd ${dirName}
 echo "Folder: ${dirName}"
 echo "Running Command: ${jaffarPath} ${fileName}"
  ${jaffarPath} ${fileName}
 popd
done
