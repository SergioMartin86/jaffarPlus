project('jaffarNES','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
)

subdir('platforms')

jaffarNESIncludes = include_directories([
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include',
 'extern/phmap',
 'games/nes/smb',
 'source'
 ])

jaffarNESSrc = [
 'source/utils.cpp',
 'source/rule.cpp',
 'extern/metrohash64/metrohash64.cc'
]

executable('jaffar',
  'source/train.cpp',
  jaffarNESSrc + quickernesSrc + gameInstanceSrc,
  include_directories: [ jaffarNESIncludes, quickernesIncludes, gameInstanceIncludes ],
  dependencies: [ dependency('openmp', required: true) ],
  link_with: [ ],
  link_args: [ ],
  cpp_args: [ '-Wfatal-errors', '-Wall' ] + quickernesCFlags
)

executable('jaffarNES-play',
  'source/play.cpp',
  jaffarNESSrc + hqnSrc + quicknesSrc + gameInstanceSrc,
  include_directories: [ jaffarNESIncludes, hqnIncludes, quicknesIncludes, gameInstanceIncludes ],
  dependencies: [ dependency('sdl2'), dependency('SDL2_image')],
  link_with: [ ],
  link_args: [ '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-Wall', '-DNCURSES' ] + quicknesCFlags + hqnCFlags 
)
    
    
    