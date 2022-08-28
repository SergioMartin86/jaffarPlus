# Source files and include dirs for QuickerNES

srcDir = meson.current_source_dir()

smbcSrc = [
srcDir + '/Emulation/Controller.cpp',
srcDir + '/Emulation/MemoryAccess.cpp',
srcDir + '/Emulation/PPU.cpp',
srcDir + '/Configuration.cpp',
srcDir + '/Util/Video.cpp',
srcDir + '/SMB/SMBEngine.cpp',
srcDir + '/SMB/SMB.cpp',
srcDir + '/SMB/SMBData.cpp', 
]

smbcIncludes = include_directories([
  '.',
  './Emulation',
  './Util',
  './SMB',
])

smbcCFlags = [ '-Wno-deprecated-enum-enum-conversion', '-Wno-multichar' ]
