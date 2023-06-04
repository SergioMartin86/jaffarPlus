#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
};

// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t playerHorizontalMagnet;
 genericMagnet_t playerVerticalMagnet;
 float bossHPMagnet = 0.0;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* globalTimer;
  uint8_t* RNGValue;
  uint8_t* lagFrame;
  uint8_t* lagType;

  uint8_t* playerAction;
  uint8_t* playerDirection;
  uint8_t* playerHP;
  uint8_t* playerPosX1;
  uint8_t* playerPosX2;
  uint8_t* playerPosX3;

  uint8_t* playerPosY1;
  uint8_t* playerPosY2;
  uint8_t* playerPosY3;

  uint8_t* bossX;
  uint8_t* bossY;
  uint8_t* bossHP;
  uint8_t* bossTimer;
  uint8_t* openLevel;

  float playerPosX;
  float playerPosY;

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
