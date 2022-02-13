#pragma once

#include <utils.h>
#include <Nes_Emu.h>
#include <Nes_State.h>
#include <blargg_errors.h>
#include <string>
#include "metrohash64.h"

class quickNESInstance
{
  public:

  // Initializes the miniPop instance
  quickNESInstance(const std::string& romFilePath);
  quickNESInstance(Nes_Emu* emulator);

  // Loading/Saving state files
  void loadStateFile(const std::string& stateFilePath);
  void saveStateFile(const std::string& stateFilePath);

  // Function to advance frame
  void advanceFrame(const uint8_t &move);
  void advanceFrame(const std::string& move);

  // Minimal serialization functions
  void serializeState(uint8_t* state) const;
  void deserializeState(const uint8_t* state);

  // Emulator instance
  Nes_Emu* _emu;

  // Base low-memory pointer
  uint8_t* _baseMem;
};
