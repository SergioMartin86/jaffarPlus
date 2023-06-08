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
 genericMagnet_t batmanHorizontalMagnet;
 genericMagnet_t batmanVerticalMagnet;
 float batmanDistanceToBossMagnet = 0.0;
 float batmanHPMagnet = 0.0;
 float bossHPMagnet = 0.0;
 float ammoMagnet = 0.0;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* gameMode;
  uint8_t* globalTimer;
  uint8_t* RNGValue;

  uint8_t* batmanAnimationState;
  uint8_t* batmanAction;
  uint8_t* batmanDirection;
  uint8_t* batmanHP;
  uint8_t* batmanPosX1;
  uint8_t* batmanPosX2;
  uint8_t* batmanPosX3;

  uint8_t* batmanPosY1;
  uint8_t* batmanPosY2;
  uint8_t* batmanPosY3;

  uint8_t* batmanJumpTimer;

  uint8_t* batmanScreenPosX;
  uint8_t* batmanScreenPosY;
  uint8_t* batmanWeapon;
  uint8_t* batmanAmmo;

  uint8_t* bossX;
  uint8_t* bossY;
  uint8_t* bossHP;

  float batmanPosX;
  float batmanPosY;

  uint8_t* levelStartTimer;

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
