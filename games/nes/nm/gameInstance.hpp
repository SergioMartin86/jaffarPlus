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
 float lapProgressMagnet = 0.0f;
 genericMagnet_t carHorizontalMagnet;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameTimer;
  uint8_t*  trafficLightTimer;
  uint8_t*  carRPM;
  uint8_t*  carSpeed;
  uint8_t*  carGear;
  uint8_t*  playerPos1;
  uint8_t*  playerPos2;
  uint8_t*  playerPos3;
  uint8_t*  lapProgress1;
  uint8_t*  lapProgress2;
  uint8_t*  carTireDamage;
  uint8_t*  carPosX;
  uint8_t*   carTireAngle;
  uint8_t*   carTurnState1;
  uint8_t*   carTurnState2;
  uint8_t* gamePhase;

  uint8_t timerTolerance;

  // Derivative Values
  float lapProgress;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
