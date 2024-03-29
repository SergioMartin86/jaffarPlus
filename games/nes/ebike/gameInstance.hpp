#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a magnet
struct magnetSet_t {
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* bikePosX;
  uint8_t* bikePosXSubpixel;
  uint8_t* intraLoopAdvance;
  uint8_t* loopsRemaining;
  uint8_t* currentLoop;
  uint8_t* raceOverFlag;

  uint8_t* bikeMoving;
  uint8_t* bikeAngle;
  uint8_t* bikeAirMode;
  uint8_t* bikeSpeedX1;
  uint8_t* bikeSpeedX256;
  uint8_t* bikeEngineTemp;

  uint8_t* bikeSpeedZ;
  uint8_t* bikePosZ;
  uint8_t* bikePosZSubpixel;

  uint8_t* bikePosY;
  uint8_t* bikeSpeedY1;
  uint8_t* bikeSpeedY2;

  uint8_t* bikeflightMode1;
  uint8_t* bikeflightMode2;
  uint8_t* bikeflightMode3;

  uint8_t* gameCycle;
  uint8_t* currentBlockX;
  uint8_t* prevBlockX;
  uint8_t* blockXTransitions;

  uint16_t* curSpeed;
  uint16_t* maxSpeed;

  uint8_t timerTolerance;


  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  uint64_t getStateMoveHash() const override;
  std::set<INPUT_TYPE> getCandidateMoves() const override;
};
