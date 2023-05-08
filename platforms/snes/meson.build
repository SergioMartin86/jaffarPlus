# Declaring source and include dirs for a Genesis game instance / emulator

inputType = 'uint16_t'

srcDir = meson.current_source_dir()
emuInstanceSrc = [ 
  srcDir + '/snes9x/apu/apu.cpp',
  srcDir + '/snes9x/apu/bapu/dsp/sdsp.cpp',
  srcDir + '/snes9x/apu/bapu/dsp/SPC_DSP.cpp',
  srcDir + '/snes9x/apu/bapu/smp/smp.cpp',
  srcDir + '/snes9x/apu/bapu/smp/smp_state.cpp',
  srcDir + '/snes9x/bsx.cpp',
  srcDir + '/snes9x/c4.cpp',
  srcDir + '/snes9x/c4emu.cpp',
  srcDir + '/snes9x/cheats.cpp',
  srcDir + '/snes9x/cheats2.cpp',
  srcDir + '/snes9x/clip.cpp',
  srcDir + '/snes9x/conffile.cpp',
  srcDir + '/snes9x/controls.cpp',
  srcDir + '/snes9x/cpu.cpp',
  srcDir + '/snes9x/cpuexec.cpp',
  srcDir + '/snes9x/cpuops.cpp',
  srcDir + '/snes9x/crosshairs.cpp',
  srcDir + '/snes9x/dma.cpp',
  srcDir + '/snes9x/dsp.cpp',
  srcDir + '/snes9x/dsp1.cpp',
  srcDir + '/snes9x/dsp2.cpp',
  srcDir + '/snes9x/dsp3.cpp',
  srcDir + '/snes9x/dsp4.cpp',
  srcDir + '/snes9x/fxinst.cpp',
  srcDir + '/snes9x/fxemu.cpp',
  srcDir + '/snes9x/gfx.cpp',
  srcDir + '/snes9x/globals.cpp',
  srcDir + '/snes9x/logger.cpp',
  srcDir + '/snes9x/memmap.cpp',
  srcDir + '/snes9x/msu1.cpp',
  srcDir + '/snes9x/movie.cpp',
  srcDir + '/snes9x/obc1.cpp',
  srcDir + '/snes9x/ppu.cpp',
  srcDir + '/snes9x/stream.cpp',
  srcDir + '/snes9x/sa1.cpp',
  srcDir + '/snes9x/sa1cpu.cpp',
  srcDir + '/snes9x/screenshot.cpp',
  srcDir + '/snes9x/sdd1.cpp',
  srcDir + '/snes9x/sdd1emu.cpp',
  srcDir + '/snes9x/seta.cpp',
  srcDir + '/snes9x/seta010.cpp',
  srcDir + '/snes9x/seta011.cpp',
  srcDir + '/snes9x/seta018.cpp',
  srcDir + '/snes9x/snapshot.cpp',
  srcDir + '/snes9x/snes9x.cpp',
  srcDir + '/snes9x/spc7110.cpp',
  srcDir + '/snes9x/srtc.cpp',
  srcDir + '/snes9x/tile.cpp',
  srcDir + '/snes9x/filter/2xsai.cpp',
  srcDir + '/snes9x/filter/blit.cpp',
  srcDir + '/snes9x/filter/epx.cpp',
  srcDir + '/snes9x/filter/hq2x.cpp',
  srcDir + '/snes9x/filter/snes_ntsc.c',
  srcDir + '/snes9x/statemanager.cpp',
  srcDir + '/snes9x/unix/unix.cpp',
  srcDir + '/snes9x/unix/x11.cpp',
]

emuInstanceIncludes = include_directories([
    '.',
    'snes9x/',
    'snes9x/unix',
    'snes9x/apu',
    'snes9x/apu/bapu',
    'snes9x/filter', 
   ])

# Platform's specific includes, source files and flags for training
commonCFlags = [ 
                  '-DNO_DUP_MAIN',
                  '-DHAVE_MKSTEMP',
                  '-DHAVE_STRINGS_H',
                  '-DHAVE_SYS_IOCTL_H',
                  '-DHAVE_STDINT_H',
                  '-DRIGHTSHIFT_IS_SAR',
                  '-DUSE_XVIDEO',
                  '-DNOSOUND'
               ]
                        

platformTrainSrc = [ emuInstanceSrc ]
platformTrainIncludes = [ emuInstanceIncludes ]
platformTrainCPPFlags = [ commonCFlags,'-DPREVENT_RENDERING']
platformTrainCFlags = platformTrainCPPFlags
platformTrainLibs = [ ] 
platformTrainLFlags = [ 
            '-lz',
            '-lm',
            '-lncurses' ]
platformTrainDependencies = [ 
          dependency('x11', required : false),
          dependency('xext', required : false),
          dependency('xv', required : false),
          ]

# Platform's specific includes, source files and flags for playback

platformPlayIncludes = [ emuInstanceIncludes ]
platformPlaySrc = [ emuInstanceSrc ]
platformPlayCPPFlags = [ commonCFlags ]
platformPlayCFlags = platformPlayCPPFlags
platformPlayLibs = [ ] 
platformPlayLFlags = [ platformTrainLFlags ]
platformPlayDependencies = [ platformTrainDependencies  ]
