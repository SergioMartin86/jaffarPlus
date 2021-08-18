project('jaffarNES','cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++17', 'default_library=static', 'buildtype=debugoptimized'],
)

cxx = meson.get_compiler('cpp')

inc = include_directories([
 'extern/libco',
 'extern/quicknes',
 'extern/quicknes/fex',
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include'
  ])

jaffar2Files = [
  'source/quicknesInstance.cc',
  'source/utils.cc',
  'source/frame.cc',
  'source/rule.cc',
  'extern/metrohash64/metrohash64.cc'
]

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

default_library='static'
abs_base = subproject('abseil').get_variable('absl_base_dep')
abs_cont = subproject('abseil').get_variable('absl_container_dep')
default_library='shared'

mpi_dep = dependency('mpi', language: 'cpp', required: false)  
if mpi_dep.found() == false
 mpi_dep = dependency('mpich', required: false)
endif

boost_dep = dependency('boost')

deps = [
  mpi_dep,
  boost_dep,
  abs_base,
  abs_cont
]

quicknesLib = shared_library('quicknes',
  quicknesFiles,
  cpp_args: [ '-Wfatal-errors', '-DDISABLE_AUTO_FILE', '-D__LIBRETRO__', '-DNDEBUG', '-Wno-multichar', '-fno-exceptions', '-fomit-frame-pointer', '-flto' ],
  include_directories: inc,
  dependencies : [ boost_dep ],
  link_args: [ '-lm' ]
  )
    
executable('jaffar2-test',
  'source/test.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ quicknesLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )
  
executable('jaffar2-train',
  'source/train.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ quicknesLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )
  
executable('jaffar2-play',
  'source/play.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ quicknesLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )

executable('jaffar2-show',
  'source/show.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ quicknesLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )
  