#pragma once

#include <emulator.hpp>

#ifdef __JAFFAR_USE_QUICKNES
#include "quickNES/quickNES.hpp"
#endif

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

#ifdef __JAFFAR_USE_QUICKERNEORAW
#include "quickerNEORAW/quickerNEORAW.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERRAWGL
#include "quickerRAWGL/quickerRAWGL.hpp"
#endif

#ifdef __JAFFAR_USE_ARKBOT
#include "quickerArkBot/quickerArkBot.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERMGBA
#include "quickerMGBA/quickerMGBA.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERGAMBATTE
#include "quickerGambatte/quickerGambatte.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERDSDA
#include "quickerDSDA/quickerDSDA.hpp"
#endif

#ifdef __JAFFAR_USE_QUICKERBAN
#include "quickerBan/quickerBan.hpp"
#endif

#ifdef __JAFFAR_USE_PIPEBOT
#include "pipeBot/pipeBot.hpp"
#endif


namespace jaffarPlus
{
#define DETECT_EMULATOR(EMULATOR)                                                                                                                                                  \
  if (emulatorName == EMULATOR::getName())                                                                                                                                         \
  {                                                                                                                                                                                \
    e            = std::make_unique<EMULATOR>(emulatorConfig);                                                                                                                     \
    isRecognized = true;                                                                                                                                                           \
  }

std::unique_ptr<Emulator> Emulator::getEmulator(const nlohmann::json& emulatorConfig)
{
  // Base pointer for the emulator
  std::unique_ptr<Emulator> e;

  // Flag to indicate whether the name was recognized
  bool isRecognized = false;

  // Getting emulator name
  const auto& emulatorName = jaffarCommon::json::getString(emulatorConfig, "Emulator Name");

// Detecting emulator
#ifdef __JAFFAR_USE_QUICKNES
  DETECT_EMULATOR(emulator::QuickNES);
#endif

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

#ifdef __JAFFAR_USE_QUICKERNEORAW
  DETECT_EMULATOR(emulator::QuickerNEORAW);
#endif

#ifdef __JAFFAR_USE_QUICKERRAWGL
  DETECT_EMULATOR(emulator::QuickerRAWGL);
#endif

#ifdef __JAFFAR_USE_ARKBOT
  DETECT_EMULATOR(emulator::QuickerArkBot);
#endif

#ifdef __JAFFAR_USE_QUICKERMGBA
  DETECT_EMULATOR(emulator::QuickerMGBA);
#endif

#ifdef __JAFFAR_USE_QUICKERGAMBATTE
  DETECT_EMULATOR(emulator::QuickerGambatte);
#endif

#ifdef __JAFFAR_USE_QUICKERDSDA
  DETECT_EMULATOR(emulator::QuickerDSDA);
#endif

#ifdef __JAFFAR_USE_QUICKERBAN
  DETECT_EMULATOR(emulator::QuickerBan);
#endif

#ifdef __JAFFAR_USE_PIPEBOT
  DETECT_EMULATOR(emulator::PipeBot);
#endif


  // Check if recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Emulator '%s' not recognized\n", emulatorName.c_str());

  // Returning emulator pointer
  return e;
}

} // namespace jaffarPlus
