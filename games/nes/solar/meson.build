# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [ '-D_STATE_DATA_SIZE_TRAIN=21002', '-D_STATE_DATA_SIZE_PLAY=21002', '-D_MAX_RULE_COUNT=16', '-D_DISABLE_XDELTA3' ] # '-D_DISABLE_XDELTA3' 
gameTools =  [ ]