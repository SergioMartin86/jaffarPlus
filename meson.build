project('jaffar++','c','cpp',
  version: '3.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
)

# Accessing source directory
subdir('source')

# Accessing emulators
subdir('emulators')

# Jaffar 
executable('jaffar',
  'source/jaffar.cpp',
  cpp_args: jaffarCPPFlags,
  sources : jaffarSources,
  dependencies : [ jaffarDependencies, dependency('openmp', required: true) ],
  include_directories : jaffarIncludes
)

# Jaffar Tester tool
executable('jaffar-tester',
  'source/tester.cpp',
  cpp_args: jaffarCPPFlags,
  sources : jaffarSources,
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)

# Jaffar Player tool
executable('jaffar-player',
  'source/player.cpp',
  cpp_args: [ jaffarCPPFlags, '-DNCURSES', '-D_USE_SDL2' ],
  link_args: [ '-lncurses' ],
  sources : jaffarSources,
  dependencies : [ jaffarDependencies, dependency('sdl2'), dependency('SDL2_image') ],
  include_directories : jaffarIncludes
)