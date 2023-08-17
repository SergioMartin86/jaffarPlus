#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
 float min = 0.0;  // What is the minimum input value to the calculation.
 float max = 0.0;  // What is the maximum input value to the calculation.
};


// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t heroHorizontalMagnet;
 genericMagnet_t heroVerticalMagnet;
 float bossHealthMagnet = 0.0;
 float heroHealthMagnet = 0.0;
 float scoreMagnet = 0.0;
 float positionImbalanceMagnet = 0.0;
 float bossHorizontalMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameMode;
  uint8_t*  gameSubMode;
  uint8_t*  gameTimer;
  uint8_t*  heroAction;
  uint8_t*  screenScroll1;
  uint8_t*  screenScroll2;
  int8_t*  bossHP;
  int8_t*  heroHP;
  uint8_t*  currentStage;
  uint8_t*  score1;
  uint8_t*  score2;
  uint8_t*  score3;
  uint8_t*  score4;
  uint8_t*  score5;
  uint8_t*  heroActionTimer;
  uint8_t*  heroScreenPosX;
  uint8_t*  heroScreenPosY;
  uint8_t*  heroAirMode;
  uint8_t*  enemyGrabCounter;
  uint8_t*  enemyShrugCounter;
  uint8_t*  enemyPosX;
  uint8_t*  enemyAction;
  uint8_t*  enemyActionTimer;
  uint8_t*  bossPosX;

  uint8_t timerTolerance;

  // Derivative Values
  int8_t enemyPosImbalance;
  float heroPosX;
  float heroPosY;
  float score;

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
