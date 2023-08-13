#pragma once

#define _INVERSE_FRAME_RATE 16667
#define ENEMY_COUNT 8
#define ORB_COUNT 8

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 genericMagnet_t duckHorizontalMagnet;
 genericMagnet_t duckVerticalMagnet;
 float traceMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameMode                 ;
  uint8_t*  globalTimer             ;
  uint8_t*  duckPosX1               ;
  uint8_t*  duckPosX2               ;
  uint8_t*  duckFrameCycle;
  uint8_t*  duckPosY               ;
  uint8_t*  grabbedWeapon               ;

  uint8_t timerTolerance;

  // Trace
  bool useTrace = false;
  std::string traceFile;
  float traceTolerance;
  std::vector<std::pair<float, float>> trace;
  size_t tracePos;

  // Derivative Values
  float duckPosX;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  std::string getFrameTrace() const override;
};
