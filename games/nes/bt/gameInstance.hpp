#pragma once

#define _INVERSE_FRAME_RATE 8333

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a magnet
struct magnetSet_t {
};

class GameInstance : public GameInstanceBase
{
 public:

 uint8_t* player1Continues;
 uint8_t* player2Continues;
 uint8_t* currentLevel;
 uint8_t* player1Lives;
 uint8_t* player2Lives;
 uint32_t* rng;
 int16_t* cameraPosX;
 int16_t* cameraPosY;
 uint8_t* levelConfigPointer;
 uint8_t* objectSlotStart;
 uint8_t* player1PosX1;
 uint8_t* player1PosX2;
 uint8_t* player1PosX3;
 uint8_t* player1PosY1;
 uint8_t* player1PosY2;
 uint8_t* player1PosY3;
 uint8_t* player1PosZ1;
 uint8_t* player1PosZ2;
 uint8_t* player1PosZ3;
 uint8_t* player2PosX1;
 uint8_t* player2PosX2;
 uint8_t* player2PosX3;
 uint8_t* player2PosY1;
 uint8_t* player2PosY2;
 uint8_t* player2PosY3;
 uint8_t* player2PosZ1;
 uint8_t* player2PosZ2;
 uint8_t* player2PosZ3;

 uint8_t* keyAddress1;
 uint8_t* keyAddress2;
 uint8_t* keyAddress3;

 float player1PosX;
 float player1PosY;
 float player1PosZ;

 float player2PosX;
 float player2PosY;
 float player2PosZ;

 // Artificial Values
 uint8_t* keyEvent1Triggered;

 // Configuration
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
