#pragma once

#include <Nes_Emu.h>
#include <Nes_State.h>
#include <string>
#include <vector>

#define _FRAME_DATA_SIZE 12792
const std::vector<std::string> _possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "LAB", "RAB", "LRAB" };

class gameInstance
{
  public:

  // Initializes the miniPop instance
  gameInstance(const std::string& romFilePath);
  gameInstance(Nes_Emu* emulator);

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
