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
      suite : [ 'examples' ])

######## Full run tests

# Setting env variables
testEnvVars = [ 'JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB=2' ]

test('race01',
      jaffar,
      workdir : meson.current_source_dir() + '/../examples/nes/sprilo/',
      timeout: testTimeout,
      args : 'race04_firstlap.jaffar',
      env : testEnvVars,
      suite : [ 'examples', 'nes', 'sprilo' ])
