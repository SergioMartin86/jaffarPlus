# Declaring emulator dependencies
emulatorDependencies = []

# Common Jaffar include dirs
includeDirs = [ '.' ]
includeFlags = [ ]

# If no custom emulator folder was specified, add the default one
emulatorsFolder = get_option('emulatorsFolder')
if  emulatorsFolder == ''
  includeDirs += '../emulators'
else # Oterwise, add it manually
  includeFlags += '-I' + emulatorsFolder
endif

# If no custom game folder was specified, add the default one
gamesFolder = get_option('gamesFolder')
if gamesFolder== ''
  includeDirs += '../games'
else # Oterwise, add it manually
  includeFlags += '-I' + gamesFolder
endif

# Creating include directories
jaffarIncludes = include_directories(includeDirs)

# Common Jaffar CPPFlags
jaffarCPPFlags = [
  '-Wfatal-errors',
  '-Wall',
  includeFlags
  ]


# Code coverage configuration
if get_option('b_coverage')
  jaffarCPPFlags += [ '-fno-inline', '-Wno-error=cpp' ]
endif

# Checking whether to use NCurses
NCursesCPPFlags = [ ]
if get_option('useNCurses') == true
  NCursesCPPFlags = [ '-DNCURSES' ]
endif