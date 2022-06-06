# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [ '-D_STATE_DATA_SIZE=2649', '-D_MAX_RULE_COUNT=36' ]
gameTools =  [ {'name': 'tester', 'mainSrc': srcDir + '/tester.cpp' }  ]
