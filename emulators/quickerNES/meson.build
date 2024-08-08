if emulator == 'QuickerNES'
quickerNESSubproject = subproject('quickerNES', default_options : 'buildQuickNES=false')
endif

if emulator == 'QuickerNESArkanoid'
quickerNESSubproject = subproject('quickerNES', default_options : [ 'buildQuickNES=false', 'enableArkanoidInputs=true'])
endif

# Getting QuickerNES dependency
quickerNESDependency = quickerNESSubproject.get_variable('quickerNESDependency')
emulatorDependencies += quickerNESDependency