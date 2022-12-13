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
 // Relevant simon magnets
 genericMagnet_t heroHorizontalMagnet;
 genericMagnet_t heroVerticalMagnet;
 float bossHealthMagnet = 0.0;
 float boss2HealthMagnet = 0.0;
 float heroMagicMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  RNGValue;
  uint8_t*  isLagFrame;
  uint8_t*  gameTimer;
  uint8_t*  gameMode;
  uint8_t*  heroPosX1;
  uint8_t*  heroPosX2;
  uint8_t*  heroPosY1;
  uint8_t*  heroPosY2;
  uint8_t*  heroLife;
  uint8_t*  heroMagic;
  uint8_t*  heroKeys;
  uint8_t*  heroMoney2;
  uint8_t*  heroMoney1;
  uint8_t*  heroScore1;
  uint8_t*  heroScore2;
  uint8_t*  heroScore3;
  uint8_t*  heroScore4;
  uint8_t*  heroScore5;
  uint8_t*  heroScore6;
  uint8_t*  heroState1;
  uint8_t*  heroState2;
  uint8_t*  heroState3;
  uint8_t*  heroState4;
  uint8_t*  heroState5;
  uint8_t*  heroState6;
  uint8_t*  heroJumpState;
  uint8_t*  bossHealth1;
  uint8_t*  bossHealth2;
  uint8_t*  bossHealth3;
  uint8_t*  bossHealth4;
  uint8_t*  attackX1;
  uint8_t*  attackX2;
  uint8_t*  attackY1;
  uint8_t*  attackY2;
  uint8_t*  inventoryHasEgg;

  // Derivative Values
  float heroPosX;
  float heroPosY;
  float heroMoney;
  float heroScore;
  float bossHealth;
  float attackX;
  float attackY;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
