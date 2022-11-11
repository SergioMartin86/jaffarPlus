#pragma once

#define ROOM_COUNT 256
//#define _INVERSE_FRAME_RATE 16667
#define _INVERSE_FRAME_RATE 4166

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity; // How strong the magnet is
 uint8_t room; // Which room does the magnet correspond to
 float center;  // What is the central point of attraction
 float min;  // What is the minimum input value to the calculation.
 float max;  // What is the maximum input value to the calculation.
 bool active; // Indicates whether the value for this magnet has specified
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 genericMagnet_t kidHorizontalMagnet;
 genericMagnet_t kidVerticalMagnet;
};
class GameInstance : public GameInstanceBase
{
 public:

  uint16_t* gameTimer;
  uint8_t* gameFrame;
  uint8_t* isLagFrame;
  uint16_t* inputCode;

  uint8_t* kidRoom;
  uint8_t* kidPosX;
  uint8_t* kidPosY;
  uint8_t* kidDirection;
  uint8_t* kidBuffered;
  uint8_t* kidHP;
  uint8_t* kidFrame;
  uint8_t* kidAction;
  uint8_t* kidCol;
  uint8_t* kidRow;
  uint8_t* kidHangingState;
  uint8_t* kidCrouchState;
  uint8_t* kidGrabState;
  uint8_t* kidSequenceStep;
  uint8_t* kidClimbingType1;
  uint8_t* kidClimbingType2;

  uint8_t* exitDoorState;

  // Artificial memory positions
  uint8_t* kidPrevFrame;
  int8_t* kidFrameDiff;

  // Settings
  uint8_t timerTolerance;
  bool skipFrames;

  uint16_t advanceState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
