testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]
testTimeout = 120

###### Adding example validator test

bash = find_program('bash')
validateExamplesScript = 'validateExamples.sh'
test('scriptValidator',
      bash,
      workdir : meson.current_source_dir(),
      timeout: testTimeout,
      args : [ validateExamplesScript ],
      suite : [ 'runs' ])

######## Full run tests

# Setting env variables

## NES / Sprilo

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=1' ]

test('race04_short_contiguous',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : 'race04_short_contiguous.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'nes', 'sprilo' ])

test('race04_short_differential',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : 'race04_short_differential.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'nes', 'sprilo' ])

test('race04_short_plain',
      jaffar,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : 'race04_short_plain.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'nes', 'sprilo' ])

test('race04_short_reproduce',
      jaffarPlayer,
      workdir : meson.current_source_dir() + '/nes/sprilo',
      timeout: testTimeout,
      args : [ 'race04_short_contiguous.jaffar', 'race04_short.sol', '--reproduce', '--disableRender', '--exitOnEnd', '--unattended' ],
      suite : [ 'reproductions', 'nes', 'sprilo' ])

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

## SNES / Christmas Craze

testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=10' ]

test('stage01',
      jaffar,
      workdir : meson.current_source_dir() + '/snes/christmasCraze',
      timeout: testTimeout,
      args : 'stage01.jaffar',
      env : testEnvVars,
      suite : [ 'runs', 'snes', 'christmasCraze' ])