#pragma once

//#define _INVERSE_FRAME_RATE 16667
#define _INVERSE_FRAME_RATE 8333

#include "gameInstanceBase.hpp"
#include "map"

// Datatype to describe a magnet
struct magnetSet_t {
 float remainingBlocksMagnet = 0.0;
 float remainingHitsMagnet = 0.0;
 float ball1PosYMagnet = 0.0;
 float fallingPowerUpPosYMagnet = 0.0;
 float horizontalDistaceToLowestBallMagnet = 0.0;
 float lowestBallPosYMagnet = 0.0;
 float bossHPMagnet = 0.0;
 float lastInputStepMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameMode;
  uint8_t*  currentLevel;
  uint8_t*  lastInput;
  uint8_t*  remainingBlocks;
  uint8_t*  remainingServeTime;

  uint8_t*  ball1X;
  uint8_t*  ball1Y;
  uint8_t*  ball2X;
  uint8_t*  ball2Y;
  uint8_t*  ball3X;
  uint8_t*  ball3Y;
  uint8_t*  paddleState;
  uint8_t*  fallingPowerUpType;
  uint8_t*  fallingPowerUpPosY;
  uint8_t*  paddlePosX;
  uint8_t*  paddlePowerUp1;
  uint8_t*  paddlePowerUp2;
  uint8_t*  warpIsActive;

  uint8_t*  enemy1PosX;
  uint8_t*  enemy2PosX;
  uint8_t*  enemy3PosX;

  uint8_t*  enemy1PosY;
  uint8_t*  enemy2PosY;
  uint8_t*  enemy3PosY;

  uint8_t*  laser1PosX;
  uint8_t*  laser2PosX;
  uint8_t*  laser1PosY;
  uint8_t*  laser2PosY;

  uint8_t*  prevPaddlePowerUp2;
  uint16_t*  lastInputStep;
  uint16_t*  stepNumber;
  uint8_t*  bossHP;

  uint8_t lowestBallPosY;
  uint8_t lowestBallPosX;
  uint8_t timerTolerance;
  float horizontalDistanceToLowestBall;

  uint16_t ballHitsRemaining;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
