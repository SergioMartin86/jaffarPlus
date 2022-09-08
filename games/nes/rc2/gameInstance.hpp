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
 float player1SpeedMagnet = 0.0;
 float player1StandingMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameTimer;
  uint8_t*  player1Speed;
  uint8_t*  player1Angle;
  uint8_t*  player1Standing;
  uint8_t*  player1PosX1;
  uint8_t*  player1PosX2;
  uint8_t*  player1PosY1;
  uint8_t*  player1PosY2;

  uint8_t timerTolerance;

  // Derivative Values
  float player1PosX;
  float player1PosY;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint64_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
