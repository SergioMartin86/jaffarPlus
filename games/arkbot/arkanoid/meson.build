# Auxiliary tool(s) for the arkanoid game. A tool dynamic_casts to this game's concrete Game/Emulator
# classes, so it only compiles for that game's single-game target. Guard on BOTH -Dgame=arkanoid and its
# emulator (-Demulator=QuickerArkBot); this keeps tools out of normal multi-game builds and avoids compiling a
# tool against a build where its classes are guarded out. Built like `jaffar` (same jaffarDependency).
if get_option('game') == 'arkanoid' and get_option('emulator') == 'QuickerArkBot'
  executable('mover', 'mover.cpp',
    cpp_args : [ '-march=native' ],
    dependencies : [ jaffarDependency, dependency('numa') ],
  )
endif
