#pragma once

#include "gameInstanceBase.hpp"

// Datatype to describe a magnet
struct magnet_t {
 float intensity; // How strong the magnet is
 float min;  // What is the minimum input value to the calculation.
 float max;  // What is the maximum input value to the calculation.
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 magnet_t simonHorizontalMagnet;
 magnet_t simonVerticalMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* simonAnimation;
  uint8_t* simonState;

  // Derivative values
  uint16_t simonPosX;
  uint16_t screenPosX;
  uint8_t currentStage;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint64_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
