project('jaffar++','c','cpp',
  version: '3.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
  subproject_dir: 'extern'
)

# Getting emulator list from the user configuration
emulators = get_option('emulators')

# Accessing source directory
subdir('source')

# Accessing emulators
subdir('emulators')

# Grabbing jaffarCommon dependency
jaffarCommonSubproject = subproject('jaffarCommon')
jaffarCommonDependency = jaffarCommonSubproject.get_variable('jaffarCommonDependency')

# Jaffar dependencies all together
jaffarDependencies = [
   jaffarCommonDependency,
   emulatorDependencies
  ]

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

  # Jaffar 
  jaffar = executable('jaffar',
    'source/jaffar.cpp',
    cpp_args: [ jaffarCPPFlags ],
    dependencies : [ jaffarDependencies, dependency('numa') ],
    include_directories : jaffarIncludes
  )

  # Jaffar Player tool
  jaffarPlayer = executable('jaffar-player',
    'source/player.cpp',
    cpp_args: [ jaffarCPPFlags, NCursesCPPFlags, '-D_JAFFAR_PLAYER' ],
    dependencies : [ jaffarDependencies ],
    include_directories : jaffarIncludes
  )

  # Build test suite if requested
  if get_option('buildTests') == true
    subdir('tests')
  endif # buildTests

endif # is_subproject()