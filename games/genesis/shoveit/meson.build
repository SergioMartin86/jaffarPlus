# Auxiliary tool(s) for the shoveit game. A tool dynamic_casts to this game's concrete Game/Emulator
# classes, so it only compiles for that game's single-game target. Guard on BOTH -Dgame=shoveit and its
# emulator (-Demulator=QuickerGPGX); this keeps tools out of normal multi-game builds and avoids compiling a
# tool against a build where its classes are guarded out. Built like `jaffar` (same jaffarDependency).
if get_option('game') == 'shoveit' and get_option('emulator') == 'QuickerGPGX'
  executable('translator', 'translator.cpp',
    cpp_args : [ '-march=native' ],
    dependencies : [ jaffarDependency, dependency('numa') ],
  )
endif
