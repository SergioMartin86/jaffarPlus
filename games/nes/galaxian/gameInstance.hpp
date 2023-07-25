#pragma once

#define _INVERSE_FRAME_RATE 8333

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
 uint8_t* gameTimer ;
 uint8_t* shotCharge;
 uint8_t* shotPosX  ;
 uint8_t* shotPosY  ;
 uint8_t* shipPosX  ;
 uint8_t* score10e1 ;
 uint8_t* score10e2 ;
 uint8_t* score10e3 ;
 uint8_t* score10e4 ;
 uint8_t* score10e5 ;
 uint8_t* score10e6 ;
 uint8_t* shipState ;
 uint8_t* flagCount ;

 double score;

 // Artificial Values
 uint8_t killedEnemies ;

  // Configuration
  uint8_t timerTolerance;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
