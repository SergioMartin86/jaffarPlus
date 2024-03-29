#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a magnet
struct magnetSet_t {
 float scoreMagnet = 0.0f;
};

class GameInstance : public GameInstanceBase
{
 public:

  uint8_t* gameMode;
  uint8_t* gameTimer;
  uint8_t* introTimer;

  uint8_t* score1;
  uint8_t* score2;
  uint8_t* score3;

  // Container for game-specific values
  uint8_t timerTolerance;
  float score;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
