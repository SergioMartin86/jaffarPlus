if 'QuickerNES' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERNES'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_NES'
subdir('quickerNES')
endif

if 'QuickerSDLPoP' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERSDLPOP'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_SDLPOP'
subdir('quickerSDLPoP')
endif

if 'QuickerSnes9x' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERSNES9X'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_SNES'
subdir('quickerSnes9x')
endif

if 'QuickerGPGX' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERGPGX'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_GENESIS'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_SEGACD'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_SG1000'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_SMS'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_GAMEGEAR'
subdir('quickerGPGX')
endif

if 'QuickerStella' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERSTELLA'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_A2600'
subdir('quickerStella')
endif

if 'Atari2600Hawk' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_ATARI2600HAWK'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_A2600'
subdir('atari2600Hawk')
endif

if 'QuickerSMBC' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERSMBC'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_SMBC'
subdir('quickerSMBC')
endif

if 'QuickerRAW' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERRAW'
jaffarCPPFlags += '-D__JAFFAR_ENABLE_RAW'
subdir('quickerRAW')
endif

if 'QuickerArkBot' in emulators
jaffarCPPFlags += '-D__JAFFAR_USE_QUICKERARKBOT'
subdir('quickerArkBot')
endif
