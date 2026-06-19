#pragma once

#include <emulator.hpp>

// AUTO-GENERATED emulator #include list (one guarded #include per emulator header found under
// emulators/). Produced at configure time by genRegistry.py -- adding an emulator header requires
// no edits here, just re-running `meson setup`. See emulatorDetect.gen.hpp for the detection list.
#include <emulatorIncludes.gen.hpp>

namespace jaffarPlus
{
#define DETECT_EMULATOR(EMULATOR)                                                                                                                                                  \
  validEmulatorNames += (validEmulatorNames.empty() ? "" : ", ") + EMULATOR::getName();                                                                                            \
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

  // Collected (while detecting) so an unrecognized name can report the emulator this build supports
  std::string validEmulatorNames;

  // AUTO-GENERATED detection list (one guarded DETECT_EMULATOR per discovered emulator)
#include <emulatorDetect.gen.hpp>

  // Check if recognized
  if (isRecognized == false) JAFFAR_THROW_LOGIC("Emulator '%s' not recognized. Emulator(s) available in this build: %s\n", emulatorName.c_str(), validEmulatorNames.c_str());

  // Returning emulator pointer
  return e;
}

} // namespace jaffarPlus
