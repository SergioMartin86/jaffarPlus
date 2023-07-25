#pragma once

#define _INVERSE_FRAME_RATE 8333
#define _MOVING_ENEMY_SLOTS 10
#define _MOVING_ENEMY_OFFSET 0x10

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a magnet
struct magnetSet_t {
};

class GameInstance : public GameInstanceBase
{
 public:

 // Container for game-specific values
 uint8_t* gameMode  ;
 uint8_t* gameTimer;

 uint8_t* shot1PosX  ;
 uint8_t* shot1PosY  ;
 uint8_t* shot1Active  ;
 uint8_t* shot2PosX  ;
 uint8_t* shot2PosY  ;
 uint8_t* shot2Active  ;
 uint8_t* shipPosX  ;
 uint8_t* score10e1 ;
 uint8_t* score10e2 ;
 uint8_t* score10e3 ;
 uint8_t* score10e4 ;
 uint8_t* score10e5 ;
 uint8_t* score10e6 ;
 uint8_t* currentStage ;
 uint8_t* enemiesRemaining ;
 uint8_t* activeEnemies ;

 uint8_t* movingEnemiesState;
 uint8_t* movingEnemiesPosY;
 uint8_t* movingEnemiesPosX;

 uint8_t* endLevelTimer;
 uint8_t* endLevelState;

 double score;
 uint8_t shotCount  ;

  // Configuration
  uint8_t timerTolerance;

  int closestMovingEnemyIdx;
  uint8_t closestMovingEnemyPosX;
  uint8_t closestMovingEnemyPosY;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
