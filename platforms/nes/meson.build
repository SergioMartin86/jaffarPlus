subdir('hqn')
subdir('quickernes')

inputType = 'uint8_t'

# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])
quickNesCPPFlags = [ '-Wno-multichar', '-Wno-deprecated-enum-enum-conversion', '-Wno-deprecated-enum-float-conversion' ]


# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, quickernesSrc ]
platformTrainIncludes = [ emuInstanceIncludes, quickernesIncludes ]
platformTrainCFlags = [ quickernesCFlags ]
platformTrainCPPFlags = [ quickNesCPPFlags ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [  ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes, hqnIncludes, quickernesIncludes ]
platformPlaySrc = [ emuInstanceSrc, hqnSrc, quickernesSrc ]
platformPlayCFlags = [ hqnCFlags, quickernesCFlags ]
platformPlayCPPFlags = [ quickNesCPPFlags ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ ]
platformPlayDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]