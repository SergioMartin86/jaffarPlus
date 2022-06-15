# Declaring source and include dirs for a Genesis game instance / emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc ]
platformTrainIncludes = [ emuInstanceIncludes ]
platformTrainCFlags = [  ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [ ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes ]
platformPlaySrc = [ emuInstanceSrc ]
platformPlayCFlags = [  ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ ]
platformPlayDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]
