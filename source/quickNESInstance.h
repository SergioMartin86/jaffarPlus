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

  // Loading/Saving state files
  void loadStateFile(const std::string& stateFilePath);
  void saveStateFile(const std::string& stateFilePath);

  // Advance a frame
  void advanceFrame(const uint8_t &move);

  // Print information about the current frame
  void printFrameInfo();

  // Function to update values that are derived from raw game data
  void updateDerivedValues();

  // Minimal serialization functions
  void serializeState(uint8_t* state) const;
  void deserializeState(const uint8_t* state);

  // This function computes the hash for the current state
  inline uint64_t computeHash() const
  {
    // Storage for hash calculation
    MetroHash64 hash;

    // Adding fixed hash elements
    hash.Update(*_screenScroll);
    hash.Update(*_marioFrame);
    hash.Update(*_marioRelPosX);
    hash.Update(*_marioPosY);
    hash.Update(*_marioDirection);
    hash.Update(*_marioVelX);
    hash.Update(*_marioVelY);
    hash.Update(*_currentWorldRaw);
    hash.Update(*_currentStageRaw);

    uint64_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  // Emulator instance
  Nes_Emu _emu;

  // Base low-memory pointer
  uint8_t* _baseMem;

  // Game specific values
  uint16_t* _screenScroll; inline uint16_t getScreenScroll() { return swap_endian<uint16_t>(*_screenScroll); };
  uint8_t* _marioFrame;
  uint8_t* _marioRelPosX;
  uint8_t* _marioPosY;
  uint8_t* _marioDirection;
  int8_t* _marioVelX;
  int8_t* _marioVelY;
  uint8_t* _timeLeft100;
  uint8_t* _timeLeft10;
  uint8_t* _timeLeft1;
  uint8_t* _currentWorldRaw;
  uint8_t* _currentStageRaw;

  // Derivative values
  uint16_t _marioPosX;
  uint8_t _currentWorld;
  uint8_t _currentStage;
};
