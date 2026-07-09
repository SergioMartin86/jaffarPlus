# Auxiliary tool(s) for the princeOfPersia game. Guarded on BOTH -Dgame=princeOfPersia and its emulator
# (-Demulator=QuickerNES) so tools stay out of normal multi-game builds (see games/meson.build).
if get_option('game') == 'princeOfPersia' and get_option('emulator') == 'QuickerNES'
  executable('pop-cpu-trace', 'cpuTrace.cpp',
    cpp_args : [ '-march=native' ],
    dependencies : [ jaffarDependency, dependency('numa') ],
  )
endif
