subdir('source')

inputType = 'uint8_t'

# Declaring source and include dirs for the emulator

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, smbcSrc ]
platformTrainIncludes = [ emuInstanceIncludes, smbcIncludes ]
platformTrainCFlags = [ smbcCFlags ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [  ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes, smbcIncludes ]
platformPlaySrc = [ emuInstanceSrc, smbcSrc ]
platformPlayCFlags = [ smbcCFlags, '-D_USE_SDL_' ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ ]
platformPlayDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]