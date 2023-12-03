#pragma once

//#define _INVERSE_FRAME_RATE 16667
#define _INVERSE_FRAME_RATE 8333
#define _PLAYER_POS 0

#include "gameInstanceBase.hpp"
#include "map"

// Datatype to describe a point magnet
struct pointMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float x = 0.0;  // What is the x point of attraction
 float y = 0.0;  // What is the y point of attraction
};

// Datatype to describe an angle magnet
struct angleMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float angle = 0.0;  // What is the angle we look for
};

// Datatype to describe a magnet
struct magnetSet_t {
 float playerCurrentLapMagnet = 0.0;
 float playerLapProgressMagnet = 0.0;
 float playerAccelMagnet = 0.0;
 float cameraDistanceMagnet = 0.0;
 float recoveryTimerMagnet = 0.0;
 angleMagnet_t car1AngleMagnet;
 pointMagnet_t pointMagnet;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint16_t* globalTimer;
  uint8_t*  preRaceTimer;
  uint8_t*  frameType;
  uint8_t*  lagFrame;

  uint8_t*  cameraPosX1;
  uint8_t*  cameraPosX2;
  uint8_t*  cameraPosY1;
  uint8_t*  cameraPosY2;

  uint8_t*  player1PosX1;
  uint8_t*  player1PosX2;
  uint8_t*  player1PosY1;
  uint8_t*  player1PosY2;
  uint8_t*  player1PosY3;
  int8_t*   player1Accel;
  uint8_t*  player1Angle1;
  uint8_t*  player1Angle2;
  uint8_t*  player1Angle3;
  uint8_t*  player1LapsRemaining;
  uint8_t*  player1LapsRemainingPrev;
  uint8_t*  player1Checkpoint;
  uint8_t*  player1Input1;
  uint8_t*  player1Input2;
  uint8_t*  player1Input3;

  uint8_t*  player1RecoveryMode;
  uint8_t*  player1RecoveryTimer;
  uint8_t*  player1CanControlCar;

  uint8_t* activeFrame1;
  uint8_t* activeFrame2;
  uint8_t* activeFrame3;
  uint8_t* activeFrame4;

  uint16_t player1PosX;
  uint16_t player1PosY;
  uint16_t cameraPosX;
  uint16_t cameraPosY;
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
