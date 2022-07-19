# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [ '-D_STATE_DATA_SIZE_TRAIN=12816', '-D_STATE_DATA_SIZE_PLAY=12816', '-D_MAX_RULE_COUNT=32', '-D_DISABLE_XDELTA3' ] # '-D_DISABLE_XDELTA3' 
gameTools =  [ ]