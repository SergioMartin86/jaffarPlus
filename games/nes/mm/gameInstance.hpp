#pragma once

#define _INVERSE_FRAME_RATE 16667
#define ENEMY_COUNT 8

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
 genericMagnet_t marbleXMagnet;
 genericMagnet_t marbleYMagnet;
 genericMagnet_t marbleZMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameTimer;
  uint8_t*  gameCycle;
  uint8_t*  winFlag;
  uint8_t*  marbleState;
  uint8_t*  marbleFlags;
  uint8_t*  marblePosX1;
  uint8_t*  marblePosX2;
  uint8_t*  marblePosX3;
  uint8_t*  marblePosY1;
  uint8_t*  marblePosY2;
  uint8_t*  marblePosY3;
  uint8_t*  marblePosZ1;
  uint8_t*  marbleAirtime;
  int8_t*   marbleVelX;
  int8_t*   marbleVelY;
  uint8_t*  marbleDeadFlag;
  uint8_t*  marbleSurfaceAngle;

  float  marblePosX;
  float  marblePosY;
  float  marblePosZ;
  float  surfaceAngleX;
  float  surfaceAngleY;

  std::set<std::string> hashIncludes;
  uint8_t timerTolerance;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
