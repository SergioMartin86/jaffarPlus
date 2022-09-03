srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [ '-D_STATE_DATA_SIZE_TRAIN=2737', '-D_STATE_DATA_SIZE_PLAY=2737' ]
gameTools =  [ {'name': 'tester', 'mainSrc': srcDir + '/tester.cpp' }  ]
