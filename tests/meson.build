testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]

# Code coverage configuration
if get_option('b_coverage')
  testCommonCppArgs += ['-fno-inline','-Wno-error=cpp' ]
endif

# Jaffar Sequence Tester tool
# This tool makes sure a given seqeuence runs correctly given a Jaffar script
sequenceTester = executable('jaffar-sequence-tester',
  '../source/testers/sequence-tester.cpp',
  cpp_args: [ jaffarCPPFlags, testCommonCppArgs ],
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)

# Jaffar Script Validator tool
# This tool makes sure a given script is correctly formatted for running with Jaffar
scriptValidator = executable('jaffar-script-validator',
  '../source/testers/script-validator.cpp',
  cpp_args: [ jaffarCPPFlags, testCommonCppArgs ],
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)