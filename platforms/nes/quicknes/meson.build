# Source files and include dirs for QuickNES

srcDir = meson.current_source_dir()

quicknesSrc = [
  srcDir + '/fex/blargg_common.cpp',
  srcDir + '/fex/blargg_errors.cpp',
  srcDir + '/fex/Data_Reader.cpp',
  srcDir + '/abstract_file.cpp',
  srcDir + '/apu_state.cpp',
  srcDir + '/Blip_Buffer.cpp',
  srcDir + '/Effects_Buffer.cpp',
  srcDir + '/Mapper_Fme7.cpp',
  srcDir + '/Mapper_Mmc5.cpp',
  srcDir + '/Mapper_Namco106.cpp',
  srcDir + '/Mapper_Vrc6.cpp',
  srcDir + '/misc_mappers.cpp',
  srcDir + '/Mmc24.cpp',
  srcDir + '/Multi_Buffer.cpp',
  srcDir + '/Nes_Apu.cpp',
  srcDir + '/Nes_Buffer.cpp',
  srcDir + '/Nes_Cart.cpp',
  srcDir + '/Nes_Core.cpp',
  srcDir + '/Nes_Cpu.cpp',
  srcDir + '/nes_data.cpp',
  srcDir + '/Nes_Effects_Buffer.cpp',
  srcDir + '/Nes_Emu.cpp',
  srcDir + '/Nes_File.cpp',
  srcDir + '/Nes_Fme7_Apu.cpp',
  srcDir + '/Nes_Mapper.cpp',
  srcDir + '/nes_mappers.cpp',
  srcDir + '/Nes_Mmc1.cpp',
  srcDir + '/Nes_Mmc3.cpp',
  srcDir + '/Nes_Namco_Apu.cpp',
  srcDir + '/Nes_Oscs.cpp',
  srcDir + '/Nes_Ppu.cpp',
  srcDir + '/Nes_Ppu_Impl.cpp',
  srcDir + '/Nes_Ppu_Rendering.cpp',
  srcDir + '/Nes_State.cpp',
  srcDir + '/nes_util.cpp',
  srcDir + '/Nes_Vrc6_Apu.cpp'
]

quicknesIncludes = include_directories([
  '.',
  'fex'
])

quicknesCFlags = [ '-Wno-deprecated-enum-enum-conversion', '-Wno-multichar', '-D__LIBRETRO__' ]
