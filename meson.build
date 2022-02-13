project('jaffarNES','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++20', 'default_library=shared', 'buildtype=release'],
)

quicknesFiles = [
  'extern/quicknes/fex/blargg_common.cpp',
  'extern/quicknes/fex/blargg_errors.cpp',
  'extern/quicknes/fex/Data_Reader.cpp',
  'extern/quicknes/abstract_file.cpp',
  'extern/quicknes/apu_state.cpp',
  'extern/quicknes/Blip_Buffer.cpp',
  'extern/quicknes/Effects_Buffer.cpp',
  'extern/quicknes/Mapper_Fme7.cpp',
  'extern/quicknes/Mapper_Mmc5.cpp',
  'extern/quicknes/Mapper_Namco106.cpp',
  'extern/quicknes/Mapper_Vrc6.cpp',
  'extern/quicknes/misc_mappers.cpp',
  'extern/quicknes/Mmc24.cpp',
  'extern/quicknes/Multi_Buffer.cpp',
  'extern/quicknes/Nes_Apu.cpp',
  'extern/quicknes/Nes_Buffer.cpp',
  'extern/quicknes/Nes_Cart.cpp',
  'extern/quicknes/Nes_Core.cpp',
  'extern/quicknes/Nes_Cpu.cpp',
  'extern/quicknes/nes_data.cpp',
  'extern/quicknes/Nes_Effects_Buffer.cpp',
  'extern/quicknes/Nes_Emu.cpp',
  'extern/quicknes/Nes_File.cpp',
  'extern/quicknes/Nes_Fme7_Apu.cpp',
  'extern/quicknes/Nes_Mapper.cpp',
  'extern/quicknes/nes_mappers.cpp',
  'extern/quicknes/Nes_Mmc1.cpp',
  'extern/quicknes/Nes_Mmc3.cpp',
  'extern/quicknes/Nes_Namco_Apu.cpp',
  'extern/quicknes/Nes_Oscs.cpp',
  'extern/quicknes/Nes_Ppu.cpp',
  'extern/quicknes/Nes_Ppu_Impl.cpp',
  'extern/quicknes/Nes_Ppu_Rendering.cpp',
  'extern/quicknes/Nes_State.cpp',
  'extern/quicknes/nes_util.cpp',
  'extern/quicknes/Nes_Vrc6_Apu.cpp'
]

jaffarNES_inc = include_directories([
 'extern/quicknes',
 'extern/quicknes/fex',
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include',
 'extern/phmap',
 'source'
 ])

jaffarNESSrc = [
 'source/utils.cc',
 'source/frame.cc',
 'source/state.cc',
 'source/rule.cc',
 'source/quickNESInstance.cc',
 'extern/metrohash64/metrohash64.cc'
]

openMP_dep = dependency('openmp', required: true)

executable('jaffarNES-train',
  'source/train.cc',
  jaffarNESSrc + quicknesFiles,
  include_directories: jaffarNES_inc,
  dependencies: [ openMP_dep ],
  link_with: [ ],
  link_args: [ ],
  cpp_args: [ '-Wfatal-errors', '-D__LIBRETRO__', '-Wall', '-Wno-multichar' ]
)

hqnFiles = [
  'extern/hqn/src/hqn.cpp',
  'extern/hqn/src/hqn_gui_controller.cpp',
  'extern/hqn/src/hqn_surface.cpp',
  'extern/hqn/src/hqn_util.cpp',
  'extern/hqn/src/options.cpp',
  'extern/hqn/quicknes/fex/blargg_common.cpp',
  'extern/hqn/quicknes/fex/blargg_errors.cpp',
  'extern/hqn/quicknes/fex/Data_Reader.cpp',
  'extern/hqn/quicknes/abstract_file.cpp',
  'extern/hqn/quicknes/apu_state.cpp',
  'extern/hqn/quicknes/Blip_Buffer.cpp',
  'extern/hqn/quicknes/Effects_Buffer.cpp',
  'extern/hqn/quicknes/Mapper_Fme7.cpp',
  'extern/hqn/quicknes/Mapper_Mmc5.cpp',
  'extern/hqn/quicknes/Mapper_Namco106.cpp',
  'extern/hqn/quicknes/Mapper_Vrc6.cpp',
  'extern/hqn/quicknes/misc_mappers.cpp',
  'extern/hqn/quicknes/Mmc24.cpp',
  'extern/hqn/quicknes/Multi_Buffer.cpp',
  'extern/hqn/quicknes/Nes_Apu.cpp',
  'extern/hqn/quicknes/Nes_Buffer.cpp',
  'extern/hqn/quicknes/Nes_Cart.cpp',
  'extern/hqn/quicknes/Nes_Core.cpp',
  'extern/hqn/quicknes/Nes_Cpu.cpp',
  'extern/hqn/quicknes/nes_data.cpp',
  'extern/hqn/quicknes/Nes_Effects_Buffer.cpp',
  'extern/hqn/quicknes/Nes_Emu.cpp',
  'extern/hqn/quicknes/Nes_File.cpp',
  'extern/hqn/quicknes/Nes_Fme7_Apu.cpp',
  'extern/hqn/quicknes/Nes_Mapper.cpp',
  'extern/hqn/quicknes/nes_mappers.cpp',
  'extern/hqn/quicknes/Nes_Mmc1.cpp',
  'extern/hqn/quicknes/Nes_Mmc3.cpp',
  'extern/hqn/quicknes/Nes_Namco_Apu.cpp',
  'extern/hqn/quicknes/Nes_Oscs.cpp',
  'extern/hqn/quicknes/Nes_Ppu.cpp',
  'extern/hqn/quicknes/Nes_Ppu_Impl.cpp',
  'extern/hqn/quicknes/Nes_Ppu_Rendering.cpp',
  'extern/hqn/quicknes/Nes_State.cpp',
  'extern/hqn/quicknes/nes_util.cpp',
  'extern/hqn/quicknes/Nes_Vrc6_Apu.cpp'
]

jaffarNESplay_inc = include_directories([
 'extern/hqn/src',
 'extern/hqn/quicknes',
 'extern/hqn/quicknes/fex',
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include',
 'extern/phmap',
 'source'
 ])
    
sdl2_dep = dependency('sdl2')
sdl2_image_dep = dependency('SDL2_image')

executable('jaffarNES-play',
  'source/play.cc',
  jaffarNESSrc +  hqnFiles,
  include_directories: jaffarNESplay_inc,
  dependencies: [ sdl2_dep, sdl2_image_dep],
  link_with: [ ],
  link_args: [ '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-D__LIBRETRO__', '-Wall', '-Wno-multichar', '-DNCURSES' ] 
)
    
    
    