subdir('source')

inputType = 'uint8_t'

# Declaring source and include dirs for the emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, awSrc ]
platformTrainIncludes = [ emuInstanceIncludes, awIncludes ]
platformTrainCPPFlags = [ awCFlags ]
platformTrainCFlags = [ awCFlags ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [ awDeps ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes, awIncludes ]
platformPlaySrc = [ emuInstanceSrc, awSrc ]
platformPlayCPPFlags = [ awCFlags, '-D_USE_SDL_' ]
platformPlayCFlags = [ awCFlags, '-D_USE_SDL_' ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ ]
platformPlayDependencies = [ awDeps, dependency('sdl2'), dependency('SDL2_image') ]
