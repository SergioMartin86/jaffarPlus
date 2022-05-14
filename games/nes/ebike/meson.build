# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [ '-D_STATE_FIXED_SIZE=0', '-D_STATE_DIFFERENTIAL_SIZE=12801', '-D_MAX_RULE_COUNT=8' ]
gameTools =  [ ]