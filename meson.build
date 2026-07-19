project('jaffarPlus','c','cpp',
  version: '2.0.0',
  license: 'MIT',
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

# libtbbmalloc must be pulled into the executable's DT_NEEDED so the glibc loader maps it once,
# single-threaded, at process startup. Otherwise oneTBB lazily dlopen()s it on its first scalable
# allocation -- which happens concurrently across NUMA-delegate threads inside StateDb::initialize()'s
# OpenMP-parallel region -- and the concurrent first-time dlopen races the loader and segfaults at
# high thread counts (e.g. 256). We reference none of its symbols, so --no-as-needed is required to
# stop the (Ubuntu-default) linker from dropping it again.
tbbMallocLinkArgs = [ '-Wl,--no-as-needed', '-ltbbmalloc', '-Wl,--as-needed' ]

# Do not build any targets if this is a subproject
if meson.is_subproject() == false

  # Jaffar
  jaffar = executable('jaffar',
    'source/jaffar.cpp',
    cpp_args: [ '-march=native' ],
    link_args: tbbMallocLinkArgs,
    dependencies : [ jaffarDependency, dependency('numa') ],
  )

  # Jaffar Player tool. Built with NESHAWK_FULL_AV so the NesHawk core keeps its audio/video paths
  # (the pixel pipeline that fills the PPU framebuffer); the search binary above stays headless for
  # speed. The macro is inert for every other emulator.
  jaffarPlayer = executable('jaffar-player',
    'source/player.cpp',
    cpp_args: [ NCursesCPPFlags, '-D_JAFFAR_PLAYER', '-DNESHAWK_FULL_AV' ],
    link_args: tbbMallocLinkArgs,
    dependencies : [ jaffarDependency ],
  )

  # Per-game auxiliary tools (each guarded by its game's single-game target; see games/meson.build)
  subdir('games')

  # Build test suite if requested
  if get_option('buildTests') == true
    subdir('tests')
  endif # buildTests

endif # is_subproject()