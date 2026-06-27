# Native Excitebike physics engine (header-only). engine.hpp comes from the exciteBot submodule
# (included via an explicit relative path in exciteBikeNative.hpp to avoid putting the submodule's
# source/ on the global include path -- it shadows jaffarPlus headers like input.hpp). The generated
# track headers (track_layout.hpp / track_sections.hpp) live in this directory (gitignored); see README.
exciteBikeNativeDependency = declare_dependency(
  include_directories : include_directories('.')
 )

emulatorDependencies += exciteBikeNativeDependency
