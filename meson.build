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

# xDelta3 Configuration
xdelta3Src = [ 'extern/xdelta3/xdelta3.c' ]
xdelta3CFlags = [ '-DXD3_DEBUG=0', '-fno-builtin', '-DNDEBUG=1']
xdelta3LFlags = [ '-Llzma' ]

# Common Jaffar include dirs
jaffarIncludes = include_directories([
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include',
 'extern/phmap',
 'extern/xdelta3',
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

jaffarCFlags = [ '-Wfatal-errors', '-Wall', '-DINPUT_TYPE=' + inputType ]

# Building main Jaffar training executable
executable('jaffar',
  'source/train.cpp',
  jaffarSrc + platformTrainSrc + gameInstanceSrc + xdelta3Src,
  include_directories: [ jaffarIncludes, platformTrainIncludes, gameInstanceIncludes ],
  dependencies: [  platformTrainDependencies, dependency('openmp', required: true) ],
  link_with: [ platformTrainLibs ],
  link_args: [ platformTrainLFlags, xdelta3LFlags ],
  cpp_args: [ jaffarCFlags, platformTrainCFlags, gameInstanceCFlags, xdelta3CFlags ],
  c_args: [ platformTrainCFlags, gameInstanceCFlags ]
)

# Building playback/validation tool
executable('jaffar-play',
  'source/play.cpp',
  jaffarSrc + platformPlaySrc + gameInstanceSrc + xdelta3Src,
  include_directories: [ jaffarIncludes, platformPlayIncludes, gameInstanceIncludes ],
  dependencies: [ platformPlayDependencies ],
  link_with: [ platformPlayLibs ],
  link_args: [ platformPlayLFlags, '-lncurses', xdelta3LFlags ],
  cpp_args: [ jaffarCFlags, platformPlayCFlags, gameInstanceCFlags, '-DNCURSES', xdelta3CFlags, '-D_JAFFAR_PLAY' ],
  c_args: [ platformPlayCFlags, gameInstanceCFlags ] 
)

# Building any necessary game-specific tools
foreach g : gameTools
  executable('jaffar-' + g['name'],
    g['mainSrc'],
    jaffarSrc + platformTrainSrc + gameInstanceSrc,
    include_directories: [ jaffarIncludes, platformTrainIncludes, gameInstanceIncludes ],
    dependencies: [  platformTrainDependencies, dependency('openmp', required: true) ],
    link_with: [ platformTrainLibs ],
    link_args: [ platformTrainLFlags ],
    cpp_args: [ jaffarCFlags, platformTrainCFlags, gameInstanceCFlags ]
  )
endforeach

# Building playback/validation tool

# Listing required schema files

korali_cppFlags_raw =  run_command('python3', [ '-m', 'korali.cxx', '--cflags' ]).stdout().strip().split()
korali_cppFlags = [ ]
foreach item : korali_cppFlags_raw
 if item not in ['-std=c++17']
  korali_cppFlags += item
 endif
endforeach
korali_cppLibs  =  run_command('python3', [ '-m', 'korali.cxx', '--libs' ]).stdout().strip().split()
rlIncludes = include_directories(['rl/_environment'])
rlSrc = [ 'rl/_environment/environment.cpp' ]

executable('jaffar-rl',
  'rl/run-vracer.cpp',
  jaffarSrc + platformTrainSrc + gameInstanceSrc + xdelta3Src + rlSrc + 'source/train.cpp',
  include_directories: [ jaffarIncludes, platformTrainIncludes, gameInstanceIncludes, rlIncludes ],
  dependencies: [  platformTrainDependencies, dependency('openmp', required: true) ],
  link_with: [ platformTrainLibs ],
  link_args: [ platformTrainLFlags, xdelta3LFlags, korali_cppLibs ],
  cpp_args: [ jaffarCFlags, platformTrainCFlags, gameInstanceCFlags, xdelta3CFlags, korali_cppFlags, '-DDISABLE_MAIN' ],
  c_args: [ platformTrainCFlags, gameInstanceCFlags ]
)
