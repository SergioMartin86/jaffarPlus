# Declaring source and include dirs for a Genesis game instance / emulator

inputType = 'uint16_t'

srcDir = meson.current_source_dir()
emuInstanceSrc = [ 
 srcDir + '/Genplus-gx/core/sound/ym3438.c',
 srcDir + '/Genplus-gx/core/sound/ym2612.c',
 srcDir + '/Genplus-gx/core/sound/eq.c',
 srcDir + '/Genplus-gx/core/sound/blip_buf.c',
 srcDir + '/Genplus-gx/core/sound/sound.c',
 srcDir + '/Genplus-gx/core/sound/psg.c',
 srcDir + '/Genplus-gx/core/sound/ym2413.c',
 srcDir + '/Genplus-gx/core/sound/opll.c',
 srcDir + '/Genplus-gx/core/z80/z80.c',
 srcDir + '/Genplus-gx/core/cart_hw/eeprom_i2c.c',
 srcDir + '/Genplus-gx/core/cart_hw/areplay.c',
 srcDir + '/Genplus-gx/core/cart_hw/ggenie.c',
 srcDir + '/Genplus-gx/core/cart_hw/md_cart.c',
 srcDir + '/Genplus-gx/core/cart_hw/megasd.c',
 srcDir + '/Genplus-gx/core/cart_hw/sram.c',
 srcDir + '/Genplus-gx/core/cart_hw/sms_cart.c',
 srcDir + '/Genplus-gx/core/cart_hw/eeprom_93c.c',
 srcDir + '/Genplus-gx/core/cart_hw/eeprom_spi.c',
 srcDir + '/Genplus-gx/core/cart_hw/svp/ssp16.c',
 srcDir + '/Genplus-gx/core/cart_hw/svp/svp.c',
 srcDir + '/Genplus-gx/core/vdp_ctrl.c',
 srcDir + '/Genplus-gx/core/cd_hw/cdc.c',
 srcDir + '/Genplus-gx/core/cd_hw/pcm.c',
 srcDir + '/Genplus-gx/core/cd_hw/cdd.c',
 srcDir + '/Genplus-gx/core/cd_hw/cd_cart.c',
 srcDir + '/Genplus-gx/core/cd_hw/gfx.c',
 srcDir + '/Genplus-gx/core/cd_hw/scd.c',
 srcDir + '/Genplus-gx/core/memz80.c',
 srcDir + '/Genplus-gx/core/state.c',
 srcDir + '/Genplus-gx/core/system.c',
 srcDir + '/Genplus-gx/core/membnk.c',
 srcDir + '/Genplus-gx/core/m68k/m68kcpu.c',
 srcDir + '/Genplus-gx/core/m68k/s68kcpu.c',
 srcDir + '/Genplus-gx/core/mem68k.c',
 srcDir + '/Genplus-gx/core/vdp_render.c',
 srcDir + '/Genplus-gx/core/loadrom.c',
 srcDir + '/Genplus-gx/core/ntsc/sms_ntsc.c',
 srcDir + '/Genplus-gx/core/ntsc/md_ntsc.c',
 srcDir + '/Genplus-gx/core/genesis.c',
 srcDir + '/Genplus-gx/core/input_hw/paddle.c',
 srcDir + '/Genplus-gx/core/input_hw/lightgun.c',
 srcDir + '/Genplus-gx/core/input_hw/teamplayer.c',
 srcDir + '/Genplus-gx/core/input_hw/graphic_board.c',
 srcDir + '/Genplus-gx/core/input_hw/sportspad.c',
 srcDir + '/Genplus-gx/core/input_hw/gamepad.c',
 srcDir + '/Genplus-gx/core/input_hw/input.c',
 srcDir + '/Genplus-gx/core/input_hw/terebi_oekaki.c',
 srcDir + '/Genplus-gx/core/input_hw/xe_1ap.c',
 srcDir + '/Genplus-gx/core/input_hw/mouse.c',
 srcDir + '/Genplus-gx/core/input_hw/activator.c',
 srcDir + '/Genplus-gx/core/io_ctrl.c',
 srcDir + '/Genplus-gx/core/tremor/floor0.c',
 srcDir + '/Genplus-gx/core/tremor/info.c',
 srcDir + '/Genplus-gx/core/tremor/framing.c',
 srcDir + '/Genplus-gx/core/tremor/vorbisfile.c',
 srcDir + '/Genplus-gx/core/tremor/block.c',
 srcDir + '/Genplus-gx/core/tremor/bitwise.c',
 srcDir + '/Genplus-gx/core/tremor/synthesis.c',
 srcDir + '/Genplus-gx/core/tremor/mdct.c',
 srcDir + '/Genplus-gx/core/tremor/registry.c',
 srcDir + '/Genplus-gx/core/tremor/sharedbook.c',
 srcDir + '/Genplus-gx/core/tremor/codebook.c',
 srcDir + '/Genplus-gx/core/tremor/window.c',
 srcDir + '/Genplus-gx/core/tremor/mapping0.c',
 srcDir + '/Genplus-gx/core/tremor/res012.c',
 srcDir + '/Genplus-gx/core/tremor/floor1.c',
 srcDir + '/Genplus-gx/core/debug/cpuhook.c',
 srcDir + '/Genplus-gx/sdl/config.c',
 srcDir + '/Genplus-gx/sdl/error.c',
 srcDir + '/Genplus-gx/sdl/fileio.c',
 srcDir + '/Genplus-gx/sdl/unzip.c',
 srcDir + '/Genplus-gx/sdl/main.c'
]

emuInstanceIncludes = include_directories([
    '.',
    'Genplus-gx/core', 
    'Genplus-gx/core/cart_hw',
    'Genplus-gx/core/cart_hw/svp', 
    'Genplus-gx/core/cd_hw', 
    'Genplus-gx/core/debug',
    'Genplus-gx/core/input_hw',
    'Genplus-gx/core/m68k',
    'Genplus-gx/core/ntsc',
    'Genplus-gx/core/sound',
    'Genplus-gx/core/tremor',
    'Genplus-gx/core/z80',
    'Genplus-gx/sdl'
   ])

# Platform's specific includes, source files and flags for training
commonCFlags = [ 
                 '-march=native',
                 '-O6',
                 '-fomit-frame-pointer',
                 '-Wall',
                 '-Wno-strict-aliasing', 
                 '-DLSB_FIRST', 
                 '-DUSE_8BPP_RENDERING', 
                 '-DUSE_LIBTREMOR', 
                 '-DMAXROMSIZE=33554432', 
                 '-DHAVE_YM3438_CORE', 
                 '-DHAVE_OPLL_CORE', 
                 '-DENABLE_SUB_68K_ADDRESS_ERROR_EXCEPTIONS',
                 '-DPACKAGE_VERSION="1.3.2"', 
                 '-DFLAC_API_EXPORTS' ,
                 '-DFLAC__HAS_OGG=0',
                 '-DHAVE_LROUND',
                 '-DHAVE_STDINT_H',
                 '-DHAVE_SYS_PARAM_H',
                 '-DUSE_DYNAMIC_ALLOC'
               ]
                        

platformTrainSrc = [ emuInstanceSrc ]
platformTrainIncludes = [ emuInstanceIncludes ]
platformTrainCFlags = [ commonCFlags,
                        '-DPREVENT_RENDERING'
                      ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ '-lz', '-lm', '-lncurses' ]
platformTrainDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes ]
platformPlaySrc = [ emuInstanceSrc ]
platformPlayCFlags = [ commonCFlags ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ platformTrainLFlags ]
platformPlayDependencies = [ dependency('sdl2'), dependency('SDL2_image') ]
