subdir('hqn')
subdir('quicknes')
subdir('quickernes')

# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, quickernesSrc ]
platformTrainIncludes = [ emuInstanceIncludes, quickernesIncludes ]
platformTrainCFlags = [ quickernesCFlags ] 

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes, hqnIncludes, quicknesIncludes ]
platformPlaySrc = [ emuInstanceSrc, hqnSrc, quicknesSrc ]
platformPlayCFlags = [ hqnCFlags, quicknesCFlags ]