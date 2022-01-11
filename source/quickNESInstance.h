#pragma once

#include <Nes_Emu.h>
#include <Nes_State.h>
#include <blargg_errors.h>
#include <string>

class quickNESInstance
{
  public:

  // Initializes the miniPop instance
  void initialize(const std::string& romFilePath);

  // Loading/Saving state files
  void loadStateFile(const std::string& stateFilePath);
  void saveStateFile(const std::string& stateFilePath);

  // Advance a frame
  void advanceFrame(const uint8_t &move);

  // Print information about the current frame
  void printFrameInfo();

  private:

  // Emulator instance
  Nes_Emu _emu;
};
