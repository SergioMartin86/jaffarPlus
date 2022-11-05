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

  uint16_t* gameTimer;
  uint8_t* gameFrame;
  uint8_t* isLagFrame;

  uint8_t* kidRoom;
  uint8_t* kidPosX;
  uint8_t* kidPosY;
  uint8_t* kidDirection;
  uint8_t* kidHP;
  uint8_t* kidFrame;
  uint8_t* kidAction;
  uint8_t* kidCol;
  uint8_t* kidRow;

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
