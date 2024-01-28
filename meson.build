project('jaffar','c','cpp',
  version: '2.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
)

# Declaring dependency vector
jaffarDependencies = []

# xDelta3 Configuration
xdelta3Src = [ 'extern/xdelta3/xdelta3.c' ]
xdelta3CFlags = [ '-DXD3_DEBUG=0', '-fno-builtin', '-DNDEBUG=1']
xdelta3LFlags = [ '-Llzma' ]

# Common Jaffar include dirs
jaffarIncludes = include_directories([
 'extern/metrohash128',
 'extern/argparse',
 'extern/json/single_include',
 'extern/phmap',
 'extern/xdelta3',
 'source'
 ])

# Common Jaffar sources
jaffarSrc = [
 'source/utils.cpp',
 'source/rule.cpp',
 'extern/metrohash128/metrohash128.cpp',
 'games/gameInstanceBase.cpp'
]

# Common Jaffar CFlags

jaffarCPPFlags = [ '-Wfatal-errors', '-Wall', '-Werror' ]
jaffarCommonSources = [  ]

# Accessing source directory
subdir('source')

# Building main Jaffar training executable
#executable('jaffar',
#  'source/train.cpp',
#  jaffarSrc + platformTrainSrc + gameInstanceSrc + xdelta3Src,
#  include_directories: [ jaffarIncludes, platformTrainIncludes, gameInstanceIncludes ],
#  dependencies: [  jaffarDependencies, platformTrainDependencies, dependency('openmp', required: true) ],
#  link_with: [ platformTrainLibs ],
#  link_args: [ platformTrainLFlags, xdelta3LFlags ],
#  cpp_args: [ jaffarCPPFlags, platformTrainCPPFlags, gameInstanceCPPFlags, xdelta3CFlags ],
#  c_args: [ platformTrainCFlags, gameInstanceCFlags ]
#)
#
## Building playback/validation tool
#executable('jaffar-play',
#  'source/play.cpp',
#  jaffarSrc + platformPlaySrc + gameInstanceSrc + xdelta3Src,
#  include_directories: [ jaffarIncludes, platformPlayIncludes, gameInstanceIncludes ],
#  dependencies: [ jaffarDependencies, platformPlayDependencies ],
#  link_with: [ platformPlayLibs ],
#  link_args: [ platformPlayLFlags, '-lncurses', xdelta3LFlags ],
#  cpp_args: [ jaffarCPPFlags, platformPlayCPPFlags, gameInstanceCPPFlags, '-DNCURSES', xdelta3CFlags, '-D_JAFFAR_PLAY' ],
#  c_args: [ platformPlayCFlags, gameInstanceCFlags ] 
#)
#
## Building input cleaner tool
#executable('jaffar-clean',
#  'source/clean.cpp',
#  jaffarSrc + platformTrainSrc + gameInstanceSrc + xdelta3Src,
#  include_directories: [ jaffarIncludes, platformTrainIncludes, gameInstanceIncludes ],
#  dependencies: [  platformTrainDependencies, dependency('openmp', required: true) ],
#  link_with: [ platformTrainLibs ],
#  link_args: [ platformTrainLFlags, xdelta3LFlags ],
#  cpp_args: [ jaffarCPPFlags, platformTrainCPPFlags, gameInstanceCPPFlags, xdelta3CFlags ],
#  c_args: [ platformTrainCFlags, gameInstanceCFlags ]
#)
#
## Building any necessary game-specific tools
#foreach g : gameTools
#  executable('jaffar-' + g['name'],
#    g['mainSrc'],
#    jaffarSrc + platformTrainSrc + gameInstanceSrc,
#    include_directories: [ jaffarIncludes, platformTrainIncludes, gameInstanceIncludes ],
#    dependencies: [  platformTrainDependencies, dependency('openmp', required: true), g['deps'] ],
#    link_with: [ platformTrainLibs ],
#    link_args: [ platformTrainLFlags ],
#    cpp_args: [ jaffarCPPFlags, platformTrainCPPFlags, gameInstanceCPPFlags ],
#    c_args: [ jaffarCPPFlags, platformTrainCFlags, gameInstanceCFlags ]
#  )
#endforeach
#

# Building tester tool
executable('jaffar-tester',
  'source/tester.cpp',
  cpp_args: jaffarCPPFlags,
  sources : jaffarCommonSources,
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)