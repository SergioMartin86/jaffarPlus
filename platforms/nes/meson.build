subdir('hqn')
subdir('quicknes')
subdir('quickernes')

# Declaring source and include dirs for a NES game instance / emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ srcDir + '/emuInstance.cpp' ]
emuInstanceIncludes = include_directories(['.'])

# Train/Playback specific includes

platformTrainSrc = [ emuInstanceSrc, quickernesSrc ]
platformPlaySrc = [ emuInstanceSrc, hqnSrc, quicknesSrc ]
platformTrainIncludes = [ emuInstanceIncludes, quickernesIncludes ]
platformPlayIncludes = [ emuInstanceIncludes, hqnIncludes, quicknesIncludes ]