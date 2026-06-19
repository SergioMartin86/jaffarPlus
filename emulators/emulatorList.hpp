#pragma once

#include <emulator.hpp>

// AUTO-GENERATED emulator #include list (one guarded #include per emulator header found under
// emulators/). Produced at configure time by genRegistry.py -- adding an emulator header requires
// no edits here, just re-running `meson setup`. See emulatorDetect.gen.hpp for the detection list.
#include <emulatorIncludes.gen.hpp>

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

  // AUTO-GENERATED detection list (one guarded DETECT_EMULATOR per discovered emulator)
#include <emulatorDetect.gen.hpp>

  // Check if recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Emulator '%s' not recognized\n", emulatorName.c_str());

  // Returning emulator pointer
  return e;
}

} // namespace jaffarPlus
