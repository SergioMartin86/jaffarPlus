testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]
testTimeout = 120

###### Unit tests (pure logic, emulator-independent)

# Uses the system gtest (libgtest-dev) when present, falling back to the
# bundled wrap (extern/gtest.wrap) otherwise. 'main: true' links gtest_main.
gtestDependency = dependency('gtest', main: true, fallback: [ 'gtest', 'gtest_main_dep' ])

unitPropertyCondition = executable('unit-propertyAndCondition',
      'unit/propertyAndCondition.cpp',
      cpp_args : testCommonCppArgs + jaffarCPPFlags,
      include_directories : jaffarIncludes,
      dependencies : [ jaffarCommonDependency, gtestDependency ])

test('propertyAndCondition',
      unitPropertyCondition,
      suite : [ 'unit' ])

unitRuleInputSet = executable('unit-ruleAndInputSet',
      'unit/ruleAndInputSet.cpp',
      cpp_args : testCommonCppArgs + jaffarCPPFlags,
      include_directories : jaffarIncludes,
      dependencies : [ jaffarCommonDependency, gtestDependency ])

test('ruleAndInputSet',
      unitRuleInputSet,
      suite : [ 'unit' ])

###### Adding example validator test

bash = find_program('bash')
validateExamplesScript = 'validateExamples.sh'

test('ValidateExamples',
      bash,
      workdir : meson.current_source_dir(),
      timeout: testTimeout,
      args : [ validateExamplesScript, emulator ],
      suite : [ 'ScriptValidation' ])

######## Full run tests

# Setting env variables

## Test Emulator / GridWalker -- a designed puzzle with a known optimal solution,
## used to test the engine's main execution path end-to-end (search + reproduce).

if emulator == 'TestEmulator'

test('gridWalker_optimal',
      bash,
      workdir : meson.current_source_dir() + '/testEmulator',
      timeout: testTimeout,
      args : [ 'checkOptimal.sh', jaffar.full_path(), jaffarPlayer.full_path() ],
      suite : [ 'runs', 'TestEmulator', 'gridWalker' ])

endif

## NES / Sprilo

if emulator == 'QuickerNES'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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

# Negative path: a missing solution file must make the player fail cleanly
test('race04_short_missing_solution',
      jaffarPlayer,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : [ 'race04_short.jaffar', 'does_not_exist.sol', '--reproduce', '--disableRender', '--exitOnEnd', '--unattended' ],
      should_fail : true,
      suite : [ 'reproductions', 'quickerNES', 'sprilo' ])

endif

## SDLPoP / Prince Of Persia

if emulator == 'QuickerSDLPoP'

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

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
testEnvVars += 'JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB=100'

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/doom',
      timeout: testTimeout,
      args : 'freedoom011.reduced.e1m1.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'QuickerDSDA', 'Doom' ])
endif
