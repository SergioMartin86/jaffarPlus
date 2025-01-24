testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]
testTimeout = 120

###### Adding example validator test

bash = find_program('bash')
validateExamplesScript = 'validateExamples.sh'

test(emulator,
      bash,
      workdir : meson.current_source_dir(),
      timeout: testTimeout,
      args : [ validateExamplesScript, emulator ],
      suite : [ 'ScriptValidation' ])

######## Full run tests

# Setting env variables

## NES / Sprilo

if emulator == 'QuickerNES'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=1' ]

test('race04_short',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : 'race04_short.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'quickerNES', 'sprilo' ])

test('race04_short_reproduce',
      jaffarPlayer,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : [ 'race04_short.jaffar', 'race04_short.sol', '--reproduce', '--disableRender', '--exitOnEnd', '--unattended' ],
      suite : [ 'reproductions', 'quickerNES', 'sprilo' ])

endif

## SDLPoP / Prince Of Persia

if emulator == 'QuickerSDLPoP'

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

if emulator == 'QuickerSnes9x'

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

## Genesis / Dino Runner

if emulator == 'QuickerGPGX'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/genesis/dinoRunner',
      timeout: testTimeout,
      args : 'stage01.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerGPGX', 'dinoRunner' ])

endif

## A2600 / Hellway

if emulator == 'QuickerStella'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/a2600/hellway',
      timeout: testTimeout,
      args : 'stage01.stella.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerStella', 'hellway' ])
      
endif

## A2600 / Hellway

if emulator == 'Atari2600Hawk'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/a2600/hellway',
      timeout: testTimeout,
      args : 'stage01.a2600hawk.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'Atari2600Hawk', 'hellway' ])
      
endif

## NES / Super Mario Bros

if emulator == 'QuickerSMBC'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/superMarioBros',
      timeout: testTimeout,
      args : 'world101a.quickerSMBC.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerSMBC', 'superMarioBros' ])
endif

## ArkBot / Arkanoid

if emulator == 'QuickerArkBot'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/arkbot',
      timeout: testTimeout,
      args : 'lvl01.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerArkBot', 'arkanoid' ])
endif


## QuickerMGBA / Toll Runner

if emulator == 'QuickerMGBA'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/gba/tollRunner',
      timeout: testTimeout,
      args : 'stage01.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerMGBA', 'tollRunner' ])
endif


## QuickerGambatte / A Slime Travel

if emulator == 'QuickerGambatte'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/gbc/aSlimeTravel',
      timeout: testTimeout,
      args : 'stage01.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerGambatte', 'aSlimeTravel' ])
endif

## QuickerDSDA / Doom

if emulator == 'QuickerDSDA'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=100' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/doom',
      timeout: testTimeout,
      args : 'freedoom011.reduced.e1m1.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerDSDA', 'Doom' ])
endif
