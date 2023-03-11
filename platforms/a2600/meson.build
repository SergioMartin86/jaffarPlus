subdir('stella')

inputType = 'uint8_t'

# Declaring source and include dirs for the emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, stellaSrc ]
platformTrainIncludes = [ emuInstanceIncludes, stellaIncludes ]
platformTrainCPPFlags = [ stellaCFlags, '-DSDL_SUPPORT' ]
platformTrainCFlags = [ stellaCFlags ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes, stellaIncludes ]
platformPlaySrc = [ emuInstanceSrc, stellaSrc ]
platformPlayCPPFlags = [ stellaCFlags, '-DSDL_SUPPORT' ]
platformPlayCFlags = [ stellaCFlags, '-DSDL_SUPPORT' ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ ]
platformPlayDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]
