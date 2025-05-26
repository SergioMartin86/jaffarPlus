if emulator == 'QuickNES'
quickNESSubproject = subproject('quickerNES', default_options : 'buildQuickNES=true')
endif

# Getting QuickNES dependency
quickNESDependency = quickNESSubproject.get_variable('quickNESDependency')
emulatorDependencies += quickNESDependency