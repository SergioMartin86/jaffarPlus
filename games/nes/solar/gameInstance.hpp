#pragma once

#define _INVERSE_FRAME_RATE 4166
#define SHOT_COUNT 8
#define OBJECT_COUNT 30
#define EYE_COUNT 5

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
 genericMagnet_t screenScrollMagnet;
 genericMagnet_t shipHorizontalMagnet;
 genericMagnet_t shipVerticalMagnet;
 genericMagnet_t shipVelXMagnet;
 genericMagnet_t shipVelYMagnet;
 float shipHealthMagnet = 0.0;
 float scoreMagnet = 0.0;
 float warpCounterMagnet = 0.0;
 float carryMagnet = 0.0;
 float maxWarpMagnet = 0.0;
 float eye1HealthMagnet = 0.0;
 float eye2HealthMagnet = 0.0;
 float eye3HealthMagnet = 0.0;
 float eye4HealthMagnet = 0.0;
 float eye1ReadinessMagnet = 0.0;
 float eye2ReadinessMagnet = 0.0;
 float eye3ReadinessMagnet = 0.0;
 float eye4ReadinessMagnet = 0.0;
 float shotsActiveMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameTimer;
  uint8_t*  currentStage;
  uint8_t*  prevStage;
  uint8_t*  screenScrollX1;
  uint8_t*  screenScrollX2;
  uint8_t*  screenScrollY1;
  uint8_t*  screenScrollY2;
  uint8_t*  shipCarriedObject;
  uint8_t*  shipHealth1;
  uint8_t*  shipHealth2;
  uint8_t*  shipMaxHealth1;
  uint8_t*  shipMaxHealth2;
  uint8_t*  shipUpgrades;
  uint8_t*  shipPosX1;
  uint8_t*  shipPosX2;
  uint8_t*  shipPosX3;
  uint8_t*  shipPosY1;
  uint8_t*  shipPosY2;
  uint8_t*  shipPosY3;
  uint8_t*  shipVelX1;
  uint8_t*  shipVelX2;
  uint8_t*  shipVelXS;
  uint8_t*  shipVelY1;
  uint8_t*  shipVelY2;
  uint8_t*  shipVelYS;
  uint8_t*  shipAngle;
  uint8_t*  shipFuel1;
  uint8_t*  shipFuel2;
  uint8_t*  score1;
  uint8_t*  score2;
  uint8_t*  score3;
  uint8_t*  score4;
  uint8_t*  score5;
  uint8_t*  score6;
  uint8_t*  stage0SecondWarpActive;
  uint8_t*  shotActive;
  uint8_t*  shipShields;
  uint8_t*  objectType;
  uint8_t*  objectData;
  uint8_t*  fuelDelivered;

  uint8_t*  eye0State;
  uint8_t*  eye1State;
  uint8_t*  eye2State;
  uint8_t*  eye3State;
  uint8_t*  eye4State;

  uint8_t*  eye1Health;
  uint8_t*  eye2Health;
  uint8_t*  eye3Health;
  uint8_t*  eye4Health;

  uint8_t*  eye1Aperture;
  uint8_t*  eye2Aperture;
  uint8_t*  eye3Aperture;
  uint8_t*  eye4Aperture;

  uint8_t*  eye1Timer;
  uint8_t*  eye2Timer;
  uint8_t*  eye3Timer;
  uint8_t*  eye4Timer;

  uint8_t timerTolerance;
  uint8_t lastInputKeyAccepted;
  uint8_t eyeCount;

  uint8_t*  lastInputKey;
  uint8_t*  lastInputFrame;

  // Derivative Values
  float shipHealth;
  float shipMaxHealth;
  float shipPosX;
  float shipPosY;
  float shipVelX;
  float shipVelY;
  float shipFuel;
  float score;
  float objectPosX[OBJECT_COUNT];
  float objectPosY[OBJECT_COUNT];
  float warpCounter;
  uint8_t shotsActive;
  uint8_t maxWarp;

  float eye1Readiness;
  float eye2Readiness;
  float eye3Readiness;
  float eye4Readiness;

  float screenScroll;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
