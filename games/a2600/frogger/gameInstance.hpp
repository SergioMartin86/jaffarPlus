#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
};

// Datatype to describe a magnet
struct magnetSet_t {
};

class GameInstance : public GameInstanceBase
{
 public:

  uint8_t* gameStarted;
  uint8_t* currentStage;
  uint8_t* gameTimer;
  uint8_t* waitTimer;
  uint8_t* playerState;
  uint8_t* playerPosX;
  uint8_t* playerPosY;
  uint8_t* score1;
  uint8_t* score2;
  uint8_t* filledPlaces;
  uint8_t* startDelay;

  uint8_t* musicTimer1;
  uint8_t* musicTimer2;

  // Container for game-specific values
  uint8_t timerTolerance;

  // Derivative Values
  float score;
  bool slot1;
  bool slot2;
  bool slot3;
  bool slot4;
  bool slot5;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
