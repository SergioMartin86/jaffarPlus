# Declaring source and include dirs for SDLPoP

inputType = 'uint8_t'

srcDir = meson.current_source_dir()
emuInstanceSrc = [ ]
emuInstanceIncludes = include_directories(['.'])

miniPoPSrc = [ srcDir + '/miniPoP/miniPoP.c'  ]
miniPoPIncludes = include_directories(['miniPoP'])

SDLPoPsrc = [   
  srcDir + '/SDLPoP/src/data.c',
  srcDir + '/SDLPoP/src/lighting.c',
  srcDir + '/SDLPoP/src/menu.c',
  srcDir + '/SDLPoP/src/midi.c',
  srcDir + '/SDLPoP/src/opl3.c',
  srcDir + '/SDLPoP/src/options.c',
  srcDir + '/SDLPoP/src/replay.c',
  srcDir + '/SDLPoP/src/screenshot.c',
  srcDir + '/SDLPoP/src/seg000.c',
  srcDir + '/SDLPoP/src/seg001.c',
  srcDir + '/SDLPoP/src/seg002.c',
  srcDir + '/SDLPoP/src/seg003.c',
  srcDir + '/SDLPoP/src/seg004.c',
  srcDir + '/SDLPoP/src/seg005.c',
  srcDir + '/SDLPoP/src/seg006.c',
  srcDir + '/SDLPoP/src/seg007.c',
  srcDir + '/SDLPoP/src/seg008.c',
  srcDir + '/SDLPoP/src/seg009.c',
  srcDir + '/SDLPoP/src/seqtbl.c',
  srcDir + '/SDLPoP/src/stb_vorbis.c'
]
    
    
sdlPopLib = shared_library('sdlPopLib',
  SDLPoPsrc,
  c_args: [ '-DDISABLE_ALL_FIXES' ],
  dependencies : [ dependency('sdl2'), dependency('SDL2_image') ],
  link_args: [ '-lm' ]
  )
  
SDLPoPIncludes = include_directories(['SDLPoP/src'])

# Platform's specific includes, source files and flags for training

platformTrainSrc = [ emuInstanceSrc, miniPoPSrc ]
platformTrainIncludes = [ emuInstanceIncludes, miniPoPIncludes ]
platformTrainCPPFlags = [ ]
platformTrainCFlags = [ '-w' ] 
platformTrainLibs = [ ] 
platformTrainLFlags = [ ]
platformTrainDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]

# Platform's specific includes, source files and flags for playback

platformPlaySrc = [ emuInstanceSrc, miniPoPSrc ]
platformPlayIncludes = [ emuInstanceIncludes, SDLPoPIncludes, miniPoPIncludes ]
platformPlayCPPFlags = [ '-DDISABLE_ALL_FIXES' ]
platformPlayCFlags = [ '-w' ]
platformPlayLibs = [ sdlPopLib ] 
platformPlayLFlags = [ '-ldl' ]
platformPlayDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]