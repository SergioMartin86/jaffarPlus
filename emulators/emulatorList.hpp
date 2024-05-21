#pragma once

#include <emulator.hpp>

#ifdef __JAFFAR_USE_QUICKERNES
  #include "quickerNES/quickerNES.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERSDLPOP
  #include "quickerSDLPoP/quickerSDLPoP.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERSNES9X
  #include "quickerSnes9x/quickerSnes9x.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERGPGX
  #include "quickerGPGX/quickerGPGX.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERSTELLA
  #include "quickerStella/quickerStella.hpp"
#endif

#ifdef __JAFFAR_USE_ATARI2600HAWK
  #include "atari2600Hawk/atari2600Hawk.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERSMBC
  #include "quickerSMBC/quickerSMBC.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERRAW
  #include "quickerRAW/quickerRAW.hpp"
#endif

#ifdef __JAFFAR_USE_ARKBOT
  #include "quickerArkBot/quickerArkBot.hpp"
#endif

namespace jaffarPlus
{
#define DETECT_EMULATOR(EMULATOR)                                                                                                                                                  \
  if (emulatorName == EMULATOR::getName())                                                                                                                                         \
  {                                                                                                                                                                                \
    e            = std::make_unique<EMULATOR>(emulatorConfig);                                                                                                                     \
    isRecognized = true;                                                                                                                                                           \
  }

std::unique_ptr<Emulator> Emulator::getEmulator(const nlohmann::json &emulatorConfig)
{
  // Base pointer for the emulator
  std::unique_ptr<Emulator> e;

  // Flag to indicate whether the name was recognized
  bool isRecognized = false;

  // Getting emulator name
  const auto &emulatorName = jaffarCommon::json::getString(emulatorConfig, "Emulator Name");

// Detecting emulator
#ifdef __JAFFAR_USE_QUICKERNES
  DETECT_EMULATOR(emulator::QuickerNES);
#endif

#ifdef __JAFFAR_USE_QUICKERSDLPOP
  DETECT_EMULATOR(emulator::QuickerSDLPoP);
#endif

#ifdef __JAFFAR_USE_QUICKERSNES9X
  DETECT_EMULATOR(emulator::QuickerSnes9x);
#endif

#ifdef __JAFFAR_USE_QUICKERGPGX
  DETECT_EMULATOR(emulator::QuickerGPGX);
#endif

#ifdef __JAFFAR_USE_QUICKERSTELLA
  DETECT_EMULATOR(emulator::QuickerStella);
#endif

#ifdef __JAFFAR_USE_ATARI2600HAWK
  DETECT_EMULATOR(emulator::Atari2600Hawk);
#endif

#ifdef __JAFFAR_USE_QUICKERSMBC
  DETECT_EMULATOR(emulator::QuickerSMBC);
#endif

#ifdef __JAFFAR_USE_QUICKERRAW
  DETECT_EMULATOR(emulator::QuickerRAW);
#endif

#ifdef __JAFFAR_USE_ARKBOT
  DETECT_EMULATOR(emulator::QuickerArkBot);
#endif

  // Check if recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Emulator '%s' not recognized\n", emulatorName.c_str());

  // Returning emulator pointer
  return e;
}

} // namespace jaffarPlus
