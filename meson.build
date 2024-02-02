project('jaffar++','c','cpp',
  version: '3.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
)

# Accessing source directory
subdir('source')

# Accessing emulators
subdir('emulators')

# Jaffar Tester tool
executable('jaffar-tester',
  'source/tester.cpp',
  cpp_args: jaffarCPPFlags,
  sources : jaffarSources,
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)