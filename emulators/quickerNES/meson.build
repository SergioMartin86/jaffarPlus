# Study tracers (RE aids in the CPU core) are compiled out by default: their per-instruction checks
# cost ~30% engine throughput even when the env vars are unset.
quickerNESTracerOption = 'enableStudyTracers=' + (get_option('quickerNESStudyTracers') ? 'true' : 'false')

if emulator == 'QuickerNES'
quickerNESSubproject = subproject('quickerNES', default_options : [ 'buildQuickNES=false', quickerNESTracerOption ])
endif

if emulator == 'QuickerNESArkanoid'
quickerNESSubproject = subproject('quickerNES', default_options : [ 'buildQuickNES=false', 'enableArkanoidInputs=true', quickerNESTracerOption ])
endif

# Getting QuickerNES dependency
quickerNESDependency = quickerNESSubproject.get_variable('quickerNESDependency')
emulatorDependencies += quickerNESDependency