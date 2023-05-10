#pragma once

//#define _INVERSE_FRAME_RATE 16667
#define _INVERSE_FRAME_RATE 8333
#define _PLAYER_POS 0

#include "gameInstanceBase.hpp"
#include "map"

// Datatype to describe a magnet
struct magnetSet_t {
 float playerCurrentLapMagnet = 0.0;
 float playerLapProgressMagnet = 0.0;
 float playerTurboCounterMagnet = 0.0;
 float playerAccelMagnet = 0.0;
 float playerCashMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameTimer;
  uint8_t*  raceNumber;
  uint8_t*  raceType;
  uint8_t*  preRaceTimer;

  uint8_t*  p1LapProgress;
  uint8_t*  p1CurrentLap;
  uint8_t*  p1TurboCounter;
  uint8_t*  p1Accel;
  uint8_t*  p1Angle;
  uint8_t*  p1PosX;
  uint8_t*  p1PosY;

  uint8_t* p1Cash1;
  uint8_t* p1Cash2;
  uint8_t* p1Cash3;
  uint8_t* p1CashGrabbed;

  uint8_t* trackType;
  uint8_t* menuState1;
  uint8_t* menuState2;
  uint8_t* menuState3;
  uint8_t* menuRaceStartTimer;
  uint8_t* menuRaceState;
  uint8_t* menuSelectorX;
  uint8_t* menuSelectorY;

  uint8_t timerTolerance;

  uint8_t maxCheckpointId;
  uint8_t checkpointId;
  uint8_t checkpointPosX;
  uint8_t checkpointPosY;
  float checkpointDistance;

  bool invertedTrack;
  bool isSkipRun;

  // Checkpoints
  std::map<uint8_t, std::map<uint8_t, std::pair<uint8_t, uint8_t>>> checkpoints;

  // Derivative Values
  float lapProgress;
  uint16_t totalCash;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
