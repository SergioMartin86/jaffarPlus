project('jaffar','c','cpp',
  version: '2.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release']
)

# Getting game name and includes
game = get_option('game')
subdir(['games/' + game]) 

# Parsing platform and including its source/includes
platform = game.split('/')[0]
subdir('platforms/' + platform)

# Common Jaffar include dirs
jaffarIncludes = include_directories([
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include',
 'extern/phmap',
 'source',
 'games',
 'platforms'
 ])

# Common Jaffar sources
jaffarSrc = [
 'source/utils.cpp',
 'source/rule.cpp',
 'extern/metrohash64/metrohash64.cc',
 'games/gameInstanceBase.cpp'
]

# Common Jaffar CFlags

jaffarCFlags = [ '-Wfatal-errors', '-Wall' ] 

# Building main Jaffar training executable
executable('jaffar',
  'source/train.cpp',
  jaffarSrc + platformTrainSrc + gameInstanceSrc,
  include_directories: [ jaffarIncludes, platformTrainIncludes, gameInstanceIncludes ],
  dependencies: [  platformTrainDependencies, dependency('openmp', required: true) ],
  link_with: [ platformTrainLibs ],
  link_args: [ platformTrainLFlags ],
  cpp_args: [ jaffarCFlags, platformTrainCFlags, gameInstanceCFlags ]
)

# Building playback/validation tool
executable('jaffar-play',
  'source/play.cpp',
  jaffarSrc + platformPlaySrc + gameInstanceSrc,
  include_directories: [ jaffarIncludes, platformPlayIncludes, gameInstanceIncludes ],
  dependencies: [ platformPlayDependencies ],
  link_with: [ platformPlayLibs ],
  link_args: [ platformPlayLFlags, '-lncurses' ],
  cpp_args: [ jaffarCFlags, platformPlayCFlags, gameInstanceCFlags, '-DNCURSES' ] 
)
    
    
    