subdir('hqn')
subdir('quickernes')

# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, quickernesSrc ]
platformTrainIncludes = [ emuInstanceIncludes, quickernesIncludes ]
platformTrainCFlags = [ quickernesCFlags ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [  ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes, hqnIncludes, quickernesIncludes ]
platformPlaySrc = [ emuInstanceSrc, hqnSrc, quickernesSrc ]
platformPlayCFlags = [ hqnCFlags, quickernesCFlags ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ ]
platformPlayDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]