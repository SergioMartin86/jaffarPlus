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
 genericMagnet_t lesterHorizontalMagnet;
 genericMagnet_t lesterVerticalMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint16_t* gameTimer;

  uint16_t* lesterPosX;
  uint16_t* lesterPosY;
  uint16_t* lesterFrame1;
  uint16_t* lesterFrame2;
  uint16_t* lesterFrame3;
  uint8_t* lesterFrame4;
  uint8_t* lesterRoom;

  int16_t* lesterForceX;
  int16_t* lesterForceY;
  int8_t* lesterForceY2;

  // Stage 01 Specific values
  uint8_t* stage01AppearTimer;
  uint8_t* stage01MonsterAnimation;
  uint8_t* stage01MetMonster;
  int16_t* stage01VineState;
  uint8_t* stage01Finish;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint64_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
