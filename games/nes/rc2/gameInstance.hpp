#pragma once

#define _INVERSE_FRAME_RATE 16667
#define _PLAYER_POS 3

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
 float playerSpeedMagnet = 0.0;
 float playerStandingMagnet = 0.0;
 float playerLapProgressMagnet = 0.0;
 float playerMoneyMagnet = 0.0;
 float playerNitroActiveMagnet = 0.0;
 float playerNitroGrabMagnet = 0.0;
 float playerLastInputMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameTimer;
  uint8_t*  isLagFrame;
  uint8_t*  trafficLightTimer;
  uint8_t*  isRaceActive;

  uint8_t*  playerSpeed1;
  uint8_t*  playerSpeed2;
  uint8_t*  playerMomentum1;
  uint8_t*  playerMomentum2;
  uint8_t*  playerAngle;
  uint8_t*  playerStanding;
  uint8_t*  playerCurrentLap;
  uint8_t*  playerLapProgress1;
  uint8_t*  playerLapProgress2;
  uint8_t*  playerPosX1;
  uint8_t*  playerPosX2;
  uint8_t*  playerPosY1;
  uint8_t*  playerPosY2;
  uint8_t*  playerPosZ;
  uint8_t*  playerFloorZ;
  uint8_t*  playerMoney1;
  uint8_t*  playerMoney2;
  uint8_t*  playerFinishPosition;
  uint8_t*  playerZipperBoost1;
  uint8_t*  playerZipperBoost2;

  uint8_t*  playerNitroCount;
  uint8_t*  playerNitroBoost;
  uint8_t*  playerNitroPhase;
  uint8_t*  playerNitroCounter;

  uint8_t*  playerNitroPrevCount;
  uint8_t*  playerNitroPickedUp;
  uint8_t*  playerLastInputKey;
  uint8_t*  playerLastInputFrame;

  uint8_t timerTolerance;

  // Derivative Values
  float playerPosX;
  float playerPosY;
  float playerMoney;
  float playerLapProgress;
  float playerSpeed;
  bool isAirborne;
  bool isNitroActive;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
