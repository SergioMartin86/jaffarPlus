# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCPPFlags = [ '-D_STATE_DATA_SIZE_TRAIN=2201', '-D_STATE_DATA_SIZE_PLAY=37321' ]
gameInstanceCFlags = [ ]
gameTools =  [  ]
