# Declaring emulator dependencies
emulatorDependencies = []

# Common Jaffar include dirs
jaffarIncludes = include_directories([
 '.'
 ])

# Common Jaffar CPPFlags
jaffarCPPFlags = [
  '-Wfatal-errors',
  '-Wall',
  '-Werror'
  ]

# Code coverage configuration
if get_option('b_coverage')
  jaffarCPPFlags += [ '-fno-inline', '-Wno-error=cpp' ]
endif

# Checking whether to use NCurses
if get_option('useNCurses') == true
  jaffarCPPFlags += [ '-fno-inline', '-Wno-error=cpp', '-DNCURSES' ]
endif