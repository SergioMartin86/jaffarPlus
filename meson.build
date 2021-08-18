project('jaffar2', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++17', 'default_library=static', 'buildtype=debugoptimized'],
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

inc = include_directories([
 'extern/libco',
 'extern/blastem',
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include'
  ])

jaffar2Files = [
  'source/blastemInstance.cc',
  'source/utils.cc',
  'source/frame.cc',
  'source/rule.cc',
  'extern/metrohash64/metrohash64.cc'
]

blastemFiles = [
  'extern/libco/libco.c',
  'extern/blastem/blastem.c',
  'extern/blastem/system.c',
  'extern/blastem/genesis.c',
  'extern/blastem/vdp.c',
  'extern/blastem/controller_info.c',
  'extern/blastem/render_sdl.c',
  'extern/blastem/io.c',
  'extern/blastem/romdb.c',
  'extern/blastem/i2c.c',
  'extern/blastem/nor.c',
  'extern/blastem/sega_mapper.c',
  'extern/blastem/serialize.c',
  'extern/blastem/config.c',
  'extern/blastem/tern.c',
  'extern/blastem/util.c',
  'extern/blastem/jaffarInject.c',
  'extern/blastem/paths.c',
  'extern/blastem/gst.c',
  'extern/blastem/68kinst.c',
  'extern/blastem/m68k_core.c',
  'extern/blastem/m68k_core_x86.c',
  'extern/blastem/gen.c',
  'extern/blastem/backend.c',
  'extern/blastem/mem.c',
  'extern/blastem/arena.c',
  'extern/blastem/gen_x86.c',
  'extern/blastem/backend_x86.c',
  'extern/blastem/ym2612.c',
  'extern/blastem/psg.c',
  'extern/blastem/vgm.c',
  'extern/blastem/render_audio.c',
  'extern/blastem/bindings.c',
  'extern/blastem/z80inst.c',
  'extern/blastem/z80_to_x86.c',
]

default_library='static'
abs_base = subproject('abseil').get_variable('absl_base_dep')
abs_cont = subproject('abseil').get_variable('absl_container_dep')
default_library='shared'

sdl2_dep = dependency('sdl2', required: true)
sdl2_image_dep = dependency('sdl2_image', required: true)
glew_dep = dependency('glew', required: true) 
 
mpi_dep = dependency('mpi', language: 'cpp', required: false)  
if mpi_dep.found() == false
 mpi_dep = dependency('mpich', required: false)
endif

boost_dep = dependency('boost')

deps = [
  mpi_dep,
  boost_dep,
  sdl2_dep,
  sdl2_image_dep,
  abs_base,
  abs_cont
]

blastemLib = shared_library('blastem',
  blastemFiles,
  c_args: [   
  '-flto',
  '-I/usr/include/libdrm',
  '-std=gnu99',
  '-Wreturn-type',
  '-Werror=return-type',
  '-Werror=implicit-function-declaration',
  '-Wno-unused-value',
  '-Wpointer-arith',
  '-Werror=pointer-arith',
  '-Wno-unused-function',
  '-DHAS_PROC',
  '-DHAVE_UNISTD_H',
  '-DX86_64',
  '-m64'
  ],
  include_directories: inc,
  dependencies : [ sdl2_dep, sdl2_image_dep, glew_dep ],
  link_args: [ '-lm' ]
  )
    
executable('jaffar2-test',
  'source/test.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ blastemLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )
  
executable('jaffar2-train',
  'source/train.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ blastemLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )
  
executable('jaffar2-play',
  'source/play.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ blastemLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )

executable('jaffar2-show',
  'source/show.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ blastemLib ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )
  