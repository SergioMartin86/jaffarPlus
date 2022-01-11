#pragma once

#include <utils.h>
#include <Nes_Emu.h>
#include <Nes_State.h>
#include <blargg_errors.h>
#include <string>

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

  private:

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
  uint8_t* _marioVelX;
  uint8_t* _marioVelY;
  uint8_t* _timeLeft100;
  uint8_t* _timeLeft10;
  uint8_t* _timeLeft1;
  uint8_t* _currentWorld;
  uint8_t* _currentStage;

  // Derivative values
  uint16_t _marioPosX;
};
