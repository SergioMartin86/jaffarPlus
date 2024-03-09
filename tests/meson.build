testCommonCppArgs = [ '-Wfatal-errors', '-Wall', '-Werror' ]
testCommonLinkArgs = [ '-lgtest_main', '-lgtest' ]


# Jaffar Sequence Tester tool
# This tool makes sure a given seqeuence runs correctly given a Jaffar script
sequenceTester = executable('jaffar-sequence-tester',
  '../source/testers/sequence-tester.cpp',
  cpp_args: jaffarCPPFlags,
  dependencies : jaffarDependencies,
  include_directories : jaffarIncludes
)