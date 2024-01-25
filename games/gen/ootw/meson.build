# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [  ]
gameInstanceCPPFlags = [ '-D_STATE_DATA_SIZE_TRAIN=67013', '-D_STATE_DATA_SIZE_PLAY=144650' ]
gameTools =  [ ]
