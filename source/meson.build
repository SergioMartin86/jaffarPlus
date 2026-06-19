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

# Auto-generate the include + detection lists for games and emulators by scanning their folders, so
# adding a new game/emulator header needs no manual wiring in gameList.hpp / emulatorList.hpp -- just
# re-run `meson setup`. The generated *.gen.hpp headers are written into THIS build directory (not
# the source tree), so each build dir gets its own -- which lets -Dgame=<header stem> compile only
# that single game (e.g. -Demulator=QuickerNES -Dgame=sprilo) without affecting other build dirs.
genRegistry   = meson.project_source_root() / 'genRegistry.py'
python3bin    = find_program('python3')
gamesPath     = gamesFolder == '' ? meson.current_source_dir() / '..' / 'games' : gamesFolder
emulatorsPath = meson.current_source_dir() / '..' / 'emulators'
genDir        = meson.current_build_dir()
run_command(python3bin, genRegistry, 'games', gamesPath,
            genDir / 'gameIncludes.gen.hpp', genDir / 'gameDetect.gen.hpp',
            get_option('game'), check: true)
run_command(python3bin, genRegistry, 'emulators', emulatorsPath,
            genDir / 'emulatorIncludes.gen.hpp', genDir / 'emulatorDetect.gen.hpp',
            check: true)
includeFlags += '-I' + genDir

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