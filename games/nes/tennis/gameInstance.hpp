#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a magnet
struct magnetSet_t {
 float ballDirectionMagnet = 0.0f;
 float playerTurnMagnet = 0.0f;
 float ballPosYMagnet = 0.0f;
 float playerScoreMagnet = 0.0f;
 float playerBallDistanceXMagnet = 0.0f;
 float playerBallDistanceYMagnet = 0.0f;
 float oppPosYMagnet = 0.0f;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameTimer;
  uint8_t*  cycleTimer;
  uint8_t*  playerPosX;
  uint8_t*  playerPosY;
  uint8_t*  playerFrame1;
  uint8_t*  playerFrame2;
  uint8_t*  playerFrame3;
  uint8_t*  playerAnimation;
  uint8_t*  ballPosX;
  uint8_t*  ballPosY;
  uint8_t*  ballPosZ;
  uint8_t*  oppPosX;
  uint8_t*  oppPosY;
  int8_t*   ballDirection;
  uint8_t*  playerTurn;
  uint8_t*  serveFault;
  uint8_t*  normalFault;
  uint8_t*  pointEnd;
  uint8_t*  playerScore;
  uint8_t*  playerGames;
  uint8_t*  playerShotType;

  uint8_t timerTolerance;

  // Derivative Values

  float playerBallDistanceX;
  float playerBallDistanceY;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
