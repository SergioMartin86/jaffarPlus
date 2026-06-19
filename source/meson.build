# Declaring emulator dependencies
emulatorDependencies = []

# Common Jaffar include dirs
includeDirs = [ '.', '../emulators' ]
includeFlags = [ ]

# If no custom game folder was specified, add the default one
gamesFolder = get_option('gamesFolder')
if gamesFolder== ''
  includeDirs += '../games'
else # Oterwise, add it manually
  includeFlags += '-I' + gamesFolder
endif

# Auto-generate the game include + detection lists by scanning the games folder, so adding a new
# game header requires no manual wiring in gameList.hpp -- just re-run `meson setup`. The generated
# headers (gameIncludes.gen.hpp / gameDetect.gen.hpp) live in the games folder (already on the
# include path) and are git-ignored.
gamesPath = gamesFolder == '' ? meson.current_source_dir() / '..' / 'games' : gamesFolder
run_command(find_program('python3'),
            gamesPath / 'genGameList.py',
            gamesPath,
            gamesPath / 'gameIncludes.gen.hpp',
            gamesPath / 'gameDetect.gen.hpp',
            check: true)

# Creating include directories
jaffarIncludes = include_directories(includeDirs)

# Common Jaffar CPPFlags
jaffarCPPFlags = [
  '-Wfatal-errors',
  '-Wall',
  '-D__INLINE__=inline',
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

# Fine-grained hot-loop profiling (off by default; adds clock_gettime overhead per timed region)
if get_option('detailedProfiling') == true
  jaffarCPPFlags += '-DJAFFARPLUS_DETAILED_PROFILING'
endif