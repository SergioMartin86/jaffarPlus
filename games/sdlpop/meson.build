srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCFlags = [ '-D_STATE_DATA_SIZE_TRAIN=2714', '-D_STATE_DATA_SIZE_PLAY=2714' ]
gameTools =  [ {'name': 'tester', 'mainSrc': srcDir + '/tester.cpp' }  ]
