subdir('stella')

inputType = 'uint8_t'

# Declaring source and include dirs for the emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, stellaSrc ]
platformTrainIncludes = [ emuInstanceIncludes, stellaIncludes ]
platformTrainCPPFlags = [ stellaCFlags ]
platformTrainCFlags = [ stellaCFlags ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [ ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes, stellaIncludes ]
platformPlaySrc = [ emuInstanceSrc, stellaSrc, sdl2Source ]
platformPlayCPPFlags = [ stellaCFlags, '-DSDL_SUPPORT' ]
platformPlayCFlags = [ stellaCFlags, '-DSDL_SUPPORT' ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ ]
platformPlayDependencies = [ dependency('sdl2') ]
