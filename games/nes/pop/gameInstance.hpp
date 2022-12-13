#pragma once

#include "gameInstanceBase.hpp"
#include <set>

#define ROOM_COUNT 256
//#define _INVERSE_FRAME_RATE 16667
#define _INVERSE_FRAME_RATE 4166

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

  // Container for game-specific values
  uint8_t* globalTimer;
  uint8_t* RNGState;
  uint8_t* currentLevel;
  uint8_t* framePhase;
  uint8_t* bottomTextTimer;
  uint8_t* gameState;
  uint8_t* passwordTimer;

  int16_t* kidPosX;
  uint8_t* kidPosY;
  uint8_t* kidFrame;
  uint8_t* kidDirection;
  uint8_t* kidMovement;
  uint8_t* kidFallWait;
  uint8_t* kidHP;
  uint8_t* kidRoom;
  uint8_t* kidFightMode;
  uint8_t* kidJumpingState;
  uint8_t* guardAlive;
  uint8_t* guardPosX;
  uint8_t* guardHP;
  uint8_t* guardNotice;
  uint8_t* guardFrame;
  uint8_t* guardMovement;
  uint8_t* drawnRoom;
  uint8_t* screenTransition;
  uint8_t* screenDrawn;
  uint8_t* isPaused;

  uint8_t* guardPresent;
  uint8_t* guardDisappearMode;

  uint8_t* doorOpeningTimer;
  uint8_t* currentDoorState;

  uint16_t* bufferedCommand;

  // Level-Specific Tiles
  uint8_t* lvl1FirstTileBG;
  uint8_t* lvl1FirstTileFG;
  uint8_t* lvl1Room19DoorTimer;
  uint8_t* exitDoorState;
  uint8_t* lvl1Room19DoorState;
  uint8_t* lvl2LastTileFG;
  uint8_t* lvl2ExitDoorState;
  uint8_t* lvl3PreCheckpointGateTimer;
  uint8_t* lvl3ExitDoorState;
  uint8_t* lvl3SkeletonLooseTile;
  uint8_t* lvl4ExitDoorState;
  uint8_t* lvl5GateTimer;
  uint8_t* lvl7SlowFallPotionState;
  uint8_t* lvl9Room15DoorState;
  uint8_t* lvl10Room0DoorState;
  uint8_t* lvl10Room4DoorState;

  uint8_t* screenTransition2;


  // Hash-specific configuration
  std::set<std::string> hashIncludes;
  uint8_t timerTolerance;

  // Derivative values
  uint8_t isCorrectRender;
  uint8_t framesPerState;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
