testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]
testTimeout = 120

# Jaffar Sequence Tester tool
# This tool makes sure a given seqeuence runs correctly given a Jaffar script
sequenceTester = executable('jaffar-sequence-tester',
  '../source/testers/sequence-tester.cpp',
  cpp_args: jaffarCPPFlags,
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)

# Adding example validator test
bash = find_program('bash')
validateExamplesScript = 'validateExamples.sh'
test('scriptValidator',
      bash,
      workdir : meson.current_source_dir(),
      timeout: testTimeout,
      args : [ validateExamplesScript ],
      suite : [ 'examples' ])