# Select the QuickerDSDA core: 'new' (headless TLS, parallel search) by default,
# or 'base' (pristine non-TLS core with full SDL rendering) for a visual jaffar-player.
doomCore = get_option('doomCore')

if doomCore == 'base'
  quickerDSDASubproject = subproject('quickerDSDA', default_options: [ 'buildBaseCore=true' ])
  quickerDSDADependency = quickerDSDASubproject.get_variable('baseDependency')
else
  quickerDSDASubproject = subproject('quickerDSDA')
  quickerDSDADependency = quickerDSDASubproject.get_variable('newDependency')
endif

emulatorDependencies += quickerDSDADependency
