# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCPPFlags = [ '-D_STATE_DATA_SIZE_TRAIN=21002', '-D_STATE_DATA_SIZE_PLAY=21002', '-D_NES_PLAYER_2']
gameInstanceCFlags = [ ]
gameTools =  [ ]