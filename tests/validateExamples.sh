#!/bin/bash

scriptFiles=`find ../examples -name "*.jaffar"`

set -e

for f in ${scriptFiles}
do
 dirName=`dirname ${f}`
 fileName=`basename ${f}`
 echo ${dirName}
 echo ${fileName}

 pushd ${dirName}
  JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10 JAFFAR_DRIVER_OVERRIDE_DRIVER_MAX_STEP=1 JAFFAR_OVERRIDE_RETURN_CODE=1 jaffar ${fileName}
 popd
done
