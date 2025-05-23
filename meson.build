project('jaffarPlus','c','cpp',
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

# Creating Jaffar's own dependency
jaffarDependency = declare_dependency(
  dependencies: [ jaffarCommonDependency, emulatorDependencies, ],
  compile_args: jaffarCPPFlags,
  include_directories: jaffarIncludes
)

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

  # Jaffar 
  jaffar = executable('jaffar',
    'source/jaffar.cpp',
    cpp_args: [ '-march=native' ],
    dependencies : [ jaffarDependency, dependency('numa') ],
  )

  # Jaffar Player tool
  jaffarPlayer = executable('jaffar-player',
    'source/player.cpp',
    cpp_args: [ NCursesCPPFlags, '-D_JAFFAR_PLAYER' ],
    dependencies : [ jaffarDependency ],
  )

  # Build test suite if requested
  if get_option('buildTests') == true
    subdir('tests')
  endif # buildTests

endif # is_subproject()