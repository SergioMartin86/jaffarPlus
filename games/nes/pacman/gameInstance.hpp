#pragma once

#define _INVERSE_FRAME_RATE 8333

#include "gameInstanceBase.hpp"
#include <set>

#define DIRECTION_UP 0
#define DIRECTION_LEFT 1
#define DIRECTION_DOWN 2
#define DIRECTION_RIGHT 3

#define ROW_COUNT 25
#define COL_COUNT 19

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
 float min = 0.0;  // What is the minimum input value to the calculation.
 float max = 0.0;  // What is the maximum input value to the calculation.
};

// Datatype to describe a magnet
struct magnetSet_t {
};

class GameInstance : public GameInstanceBase
{
 public:

 // Container for game-specific values
  uint8_t* globalTimer;

  uint8_t* playerPosX1;
  uint8_t* playerPosX2;
  uint8_t* playerPosY1;
  uint8_t* playerPosY2;
  uint8_t* playerFrame;

  uint8_t* playerDirection1;
  uint8_t* playerDirection2;
  uint8_t* currentLevel;
  uint8_t* remainingPellets;

  uint8_t* ghost0PosX1;
  uint8_t* ghost0PosX2;
  uint8_t* ghost1PosX1;
  uint8_t* ghost1PosX2;
  uint8_t* ghost2PosX1;
  uint8_t* ghost2PosX2;
  uint8_t* ghost3PosX1;
  uint8_t* ghost3PosX2;

  uint8_t* ghost0PosY1;
  uint8_t* ghost0PosY2;
  uint8_t* ghost1PosY1;
  uint8_t* ghost1PosY2;
  uint8_t* ghost2PosY1;
  uint8_t* ghost2PosY2;
  uint8_t* ghost3PosY1;
  uint8_t* ghost3PosY2;

  uint8_t* ghost0Direction;
  uint8_t* ghost1Direction;
  uint8_t* ghost2Direction;
  uint8_t* ghost3Direction;

  uint8_t* captureCount;
  uint8_t* captureCountPrev;
  uint8_t* captureCountTotal;
  float* captureCountReward;

  uint8_t* ghost0State;
  uint8_t* ghost1State;
  uint8_t* ghost2State;
  uint8_t* ghost3State;

  uint8_t* pelletMap;

  uint8_t* score1;
  uint8_t* score2;
  uint8_t* score3;
  uint8_t* score4;
  uint8_t* score5;

  // Pathing
  std::vector<std::vector<int>> pelletPath;

  // Derivative values
  float playerPosY;
  float playerPosX;
  float ghost0PosY;
  float ghost0PosX;
  float ghost1PosY;
  float ghost1PosX;
  float ghost2PosY;
  float ghost2PosX;
  float ghost3PosY;
  float ghost3PosX;
  float score;

  // Container for game-specific values
  float   targetPelletDistance;
  uint8_t targetPelletRow;
  uint8_t targetPelletCol;
  uint8_t targetPelletPosX;
  uint8_t targetPelletPosY;
  uint8_t targetPelletsTaken;

  // Configuration
  uint8_t timerTolerance;
  bool disableGhosts;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
