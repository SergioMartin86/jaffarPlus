project('jaffar++','c','cpp',
  version: '3.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
  subproject_dir: 'extern'
)

# Accessing source directory
subdir('source')

# Accessing emulators
subdir('emulators')

# Grabbing jaffarCommon dependency
jaffarCommonSubproject = subproject('jaffarCommon')
jaffarCommonDependency = jaffarCommonSubproject.get_variable('jaffarCommonDependency')

# Jaffar dependencies all together
jaffarDependencies = [ jaffarCommonDependency, emulatorDependencies]

# Jaffar 

executable('jaffar',
  'source/jaffar.cpp',
  cpp_args: [ jaffarCPPFlags, '-DNCURSES' ],
  dependencies : jaffarDependencies,
  link_args: [ '-lncurses' ],
  include_directories : jaffarIncludes
)

# Jaffar Tester tool
executable('jaffar-tester',
  'source/tester.cpp',
  cpp_args: jaffarCPPFlags,
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)

# Jaffar Player tool

jaffarPlayerDependencies = [
  dependency('sdl2'),
  dependency('SDL2_image')
]

executable('jaffar-player',
  'source/player.cpp',
  cpp_args: [ jaffarCPPFlags, '-DNCURSES', '-D_USE_SDL2' ],
  link_args: [ '-lncurses' ],
  dependencies : [ jaffarDependencies, jaffarPlayerDependencies ],
  include_directories : jaffarIncludes
)