# Source files and include dirs for QuickerNES

srcDir = meson.current_source_dir()

awSrc = [
srcDir + '/bank.cpp',
srcDir + '/engine.cpp',
srcDir + '/file.cpp',
srcDir + '/main.cpp',
srcDir + '/memBuffer.cpp',
srcDir + '/mixer.cpp',
srcDir + '/parts.cpp',
srcDir + '/resource.cpp',
srcDir + '/serializer.cpp',
srcDir + '/sfxplayer.cpp',
srcDir + '/staticres.cpp',
srcDir + '/sysImplementation.cpp',
srcDir + '/util.cpp',
srcDir + '/video.cpp',
srcDir + '/vm.cpp',
]

awIncludes = include_directories([
 '.',
])

awCFlags = [ '-DAUTO_DETECT_PLATFORM', '-Wno-deprecated-enum-enum-conversion','-Wno-multichar']
awDeps = [ dependency('boost', modules : ['context']) ]

