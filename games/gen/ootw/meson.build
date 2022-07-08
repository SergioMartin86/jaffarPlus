# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [ '-D_STATE_DATA_SIZE_TRAIN=67013', '-D_STATE_DATA_SIZE_PLAY=144650', '-D_MAX_RULE_COUNT=40' ] # '-D_DISABLE_XDELTA3'
gameTools =  [ ]