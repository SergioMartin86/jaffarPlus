srcDir = meson.current_source_dir()
gameInstanceSrc = [ srcDir + '/gameInstance.cpp', srcDir + '/gameRule.cpp' ]
gameInstanceIncludes = include_directories(['.'])
gameInstanceCPPFlags = [ '-D_STATE_DATA_SIZE_TRAIN=2750', '-D_STATE_DATA_SIZE_PLAY=2750' ]
gameInstanceCFlags = [ ]
gameTools =  [
  {'name': 'tester', 'mainSrc': srcDir + '/tester.cpp', 'deps': []},
  {'name': 'explorer', 'mainSrc': srcDir + '/explorer.cpp', 'deps': [] },
  {'name': 'solver', 'mainSrc': srcDir + '/solver.cpp', 'deps': [] },
  ]
