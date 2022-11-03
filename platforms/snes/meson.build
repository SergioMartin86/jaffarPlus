# Declaring source and include dirs for a Genesis game instance / emulator

inputType = 'uint16_t'

srcDir = meson.current_source_dir()
emuInstanceSrc = [ 
  srcDir + '/bsnes-115/hiro/hiro.cpp',
  srcDir + '/bsnes-115/ruby/ruby.cpp',
  srcDir + '/bsnes-115/libco/libco.c',
  srcDir + '/bsnes-115/bsnes/emulator/emulator.cpp',
  srcDir + '/bsnes-115/bsnes/filter/filter.cpp',
  srcDir + '/bsnes-115/bsnes/lzma/lzma.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/interface/interface.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/system/system.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/controller/controller.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/cartridge/cartridge.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/memory/memory.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/cpu/cpu.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/smp/smp.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/dsp/dsp.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/ppu/ppu.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/ppu-fast/ppu.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/expansion/expansion.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/coprocessor/coprocessor.cpp',
  srcDir + '/bsnes-115/bsnes/sfc/slot/slot.cpp',
  srcDir + '/bsnes-115/bsnes/gb/Core/apu.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/camera.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/display.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/gb.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/joypad.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/mbc.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/memory.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/printer.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/random.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/rewind.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/save_state.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/sgb.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/sm83_cpu.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/symbol_hash.c',
  srcDir + '/bsnes-115/bsnes/gb/Core/timing.c',
  srcDir + '/bsnes-115/bsnes/processor/arm7tdmi/arm7tdmi.cpp',
  srcDir + '/bsnes-115/bsnes/processor/spc700/spc700.cpp',
  srcDir + '/bsnes-115/bsnes/processor/wdc65816/wdc65816.cpp',
  srcDir + '/bsnes-115/bsnes/target-bsnes/bsnes.cpp',
  srcDir + '/bsnes-115/bsnes/target-bsnes/program/program.cpp',
  srcDir + '/bsnes-115/bsnes/target-bsnes/input/input.cpp',
  srcDir + '/bsnes-115/bsnes/target-bsnes/presentation/presentation.cpp',
  srcDir + '/bsnes-115/bsnes/target-bsnes/settings/settings.cpp',
  srcDir + '/bsnes-115/bsnes/target-bsnes/tools/tools.cpp',
  srcDir + '/bsnes-115/bsnes/target-bsnes/resource/resource.cpp',
]

emuInstanceIncludes = include_directories([
    '.',
    'bsnes-115/hiro', 
    'bsnes-115/ruby',
    'bsnes-115/bsnes',
    'bsnes-115/bsnes/target-bsnes', 
    'bsnes-115/',
   ])

# Platform's specific includes, source files and flags for training
commonCFlags = [ 
                  '-MMD',
                  '-MP',
                  '-MF',
                  '-DBUILD_PERFORMANCE',
                  '-fno-strict-aliasing',
                  '-fwrapv',
                  '-Wno-trigraphs',
                  '-DGB_INTERNAL',
                  '-DDISABLE_DEBUGGER',
                  '-D_GNU_SOURCE',
                  '-Wno-multichar',
                  '-Wno-maybe-uninitialized',
                  '-Wno-reorder',
                  '-Wno-parentheses',
                  '-DHIRO_GTK=3',
                  '-DVIDEO_GLX',
                  '-D_NO_MAIN'
               ]
                        

platformTrainSrc = [ emuInstanceSrc ]
platformTrainIncludes = [ emuInstanceIncludes ]
platformTrainCFlags = [ commonCFlags,
                        '-DPREVENT_RENDERING'
                      ]
platformTrainLibs = [ ] 
platformTrainLFlags = [ 
            '-lz',
            '-lm',
            '-lncurses' ]
platformTrainDependencies = [ 
          dependency('sdl2'),
          dependency('SDL2_image'),
          dependency('gtk+-3.0'),
          dependency('gtksourceview-3.0'),
          dependency('x11'),
          dependency('xrandr'),
          dependency('GL'),
          ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes ]
platformPlaySrc = [ emuInstanceSrc ]
platformPlayCFlags = [ commonCFlags ]
platformPlayLibs = [ ] 
platformPlayLFlags = [ platformTrainLFlags ]
platformPlayDependencies = [ platformTrainDependencies  ]
