#!/bin/bash

scriptFiles=`find ../examples -name "*.jaffar"`

set -e

# Getting absolute path to the Jaffar engine
jaffarPath=`realpath ../build/jaffar`

# Override values
export JAFFAR_IS_DRY_RUN=1
export JAFFAR_DRIVER_OVERRIDE_DRIVER_MAX_STEP=1
export JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10
export JAFFAR_ENGINE_OVERRIDE_MAX_THREAD_COUNT=1

# Getting emulator to test
emulatorName=${1}

echo ${scriptFiles}

for f in ${scriptFiles}
do
 dirName=`dirname ${f}`
 fileName=`basename ${f}`
 echo ${dirName}
 echo ${fileName}

 pushd ${dirName}
 echo "Folder: ${dirName}"

 # Checking if the selected emulator is being tested
 set +e
 cat ${fileName} | grep \"${emulatorName}\"
 emulatorFound=$?
 set -e

 if [ ${emulatorFound} -ne 0 ]; then
   echo "Emulator not found, ignoring example"
 else
   echo "Running Command: ${jaffarPath} ${fileName}"
   ${jaffarPath} --dryRun ${fileName}
 fi
 
 popd
done
