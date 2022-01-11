project('jaffarNES','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++17', 'default_library=static', 'buildtype=debugoptimized'],
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

inc = include_directories([
 'extern/quicknes',
 'extern/quicknes/fex',
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include',
 'source'
 ])

jaffarNESSrc = [
 'source/utils.cc',
 'source/frame.cc',
 'source/quickNESInstance.cc'
]

quicknesLib = shared_library('quicknes',
  quicknesFiles,
  cpp_args: [ '-Wfatal-errors', '-D__LIBRETRO__', '-DNDEBUG', '-Wno-multichar', '-fno-exceptions', '-fomit-frame-pointer', '-flto' ],
  include_directories: inc,
  dependencies : [ ],
  link_args: [ '-lm' ]
  )
    
executable('testRun',
  'source/test.cc',
  jaffarNESSrc,
  include_directories: inc,
  link_with: [ quicknesLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-Wall', '-Wno-multichar' ]
  )
