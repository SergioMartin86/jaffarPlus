# C++ translation of BizHawk's NesHawk cycle-accurate NES core (header-only, no submodule).
# The core lives in extern/quickerNesHawk/source/ (translated 1:1 from the C# sources and validated
# byte-exact against them; see extern/quickerNesHawk/harness). This directory holds the jaffarPlus
# wrapper (nesHawk.hpp) and its NES joypad input parser.
nesHawkDependency = declare_dependency(
  include_directories : include_directories('.')
 )

emulatorDependencies += nesHawkDependency
