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

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

  # Jaffar 
  executable('jaffar',
    'source/jaffar.cpp',
    cpp_args: [ jaffarCPPFlags, '-DNCURSES' ],
    dependencies : jaffarDependencies,
    link_args: [ '-lncurses' ],
    include_directories : jaffarIncludes
  )

  # Jaffar Player tool
  executable('jaffar-player',
    'source/player.cpp',
    cpp_args: [ jaffarCPPFlags, '-DNCURSES', '-D_USE_SDL2' ],
    link_args: [ '-lncurses' ],
    dependencies : [ jaffarDependencies, dependency('sdl2'), dependency('SDL2_image')],
    include_directories : jaffarIncludes
  )

  # Build test suite if requested
  if get_option('buildTests') == true
    subdir('tests')
  endif # buildTests

endif # is_subproject()