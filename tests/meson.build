testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]
testTimeout = 120

###### Adding example validator test

bash = find_program('bash')
validateExamplesScript = 'validateExamples.sh'

foreach emulator : emulators

test(emulator,
      bash,
      workdir : meson.current_source_dir(),
      timeout: testTimeout,
      args : [ validateExamplesScript, emulator ],
      suite : [ 'ScriptValidation' ])

endforeach

######## Full run tests

# Setting env variables

if 'QuickerNES' in emulators

## NES / Sprilo

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=1' ]

test('race04_short_contiguous',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : 'race04_short_contiguous.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'quickerNES', 'sprilo' ])

test('race04_short_differential',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : 'race04_short_differential.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'quickerNES', 'sprilo' ])

test('race04_short_plain',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : 'race04_short_plain.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'quickerNES', 'sprilo' ])

test('race04_short_reproduce',
      jaffarPlayer,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : [ 'race04_short_contiguous.jaffar', 'race04_short.sol', '--reproduce', '--disableRender', '--exitOnEnd', '--unattended' ],
      suite : [ 'reproductions', 'quickerNES', 'sprilo' ])

## NES / Super Mario Bros

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/superMarioBros',
      timeout: testTimeout,
      args : 'world101a.quickerNES.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'quickerNES', 'superMarioBros' ])
      
endif

if 'QuickerSDLPoP' in emulators

## SDLPoP / Prince Of Persia

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=1' ]

test('lvl01',
      jaffar,
      workdir : meson.current_source_dir() + '/sdlpop',
      timeout: testTimeout,
      args : 'lvl01a.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'sdlpop' ])

test('lvl01_reproduce',
      jaffarPlayer,
      workdir : meson.current_source_dir() + '/sdlpop',
      timeout: testTimeout,
      args : [ 'lvl01a.jaffar', 'lvl01a.sol', '--reproduce', '--disableRender', '--exitOnEnd', '--unattended' ],
      suite : [ 'reproductions', 'sdlpop' ])

endif

if 'QuickerSnes9x' in emulators

## SNES / Christmas Craze

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/snes/christmasCraze',
      timeout: testTimeout,
      args : 'stage01.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerSnes9x', 'christmasCraze' ])

endif

if 'QuickerGPGX' in emulators

## Genesis / Dino Runner

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/genesis/dinoRunner',
      timeout: testTimeout,
      args : 'stage01.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerGPGX', 'dinoRunner' ])

endif

if 'QuickerStella' in emulators

## A2600 / Hellway

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/a2600/hellway',
      timeout: testTimeout,
      args : 'stage01.stella.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerStella', 'hellway' ])
      
endif

if 'Atari2600Hawk' in emulators

## A2600 / Hellway

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/a2600/hellway',
      timeout: testTimeout,
      args : 'stage01.a2600hawk.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'Atari2600Hawk', 'hellway' ])
      
endif

if 'QuickerSMBC' in emulators

## NES / Super Mario Bros

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/superMarioBros',
      timeout: testTimeout,
      args : 'world101a.quickerSMBC.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerSMBC', 'superMarioBros' ])
endif

