#pragma once

#define _INVERSE_FRAME_RATE 8333

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
};

// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t playerHorizontalMagnet;
 genericMagnet_t playerVerticalMagnet;
 float traceMagnet = 0.0;
};

class GameInstance : public GameInstanceBase
{
 public:

 uint8_t* globalTimer       ;
 uint8_t* currentZone       ;
 uint8_t* playerInput1      ;
 uint8_t* playerInput2      ;
 uint8_t* playerPosX1       ;
 uint8_t* playerPosX2       ;
 uint8_t* playerPosX2Offset ;
 uint8_t* hookLength        ;
 uint8_t* playerPosY2       ;
 uint8_t* playerDirection1  ;
 uint8_t* playerDirection2  ;
 uint8_t* gameMode          ;
 uint8_t* levelTransition   ;
 uint8_t* exitState         ;
 uint8_t* bufferedAction1   ;
 uint8_t* bufferedAction2   ;
 uint8_t* bufferedAction3   ;
 uint8_t* bufferedAction4   ;
 uint8_t* screenPosX1       ;
 uint8_t* screenPosX2       ;
 int8_t* screenPosY1       ;
 uint8_t* screenPosY2       ;
 uint8_t* swingTimer;
 uint8_t* actionTimer;
 uint8_t* currentAction;
 uint8_t* prevAction;
 uint8_t* hookTimer;
 uint8_t* hookPosX;
 uint8_t* hookPosY;
 uint8_t* hookState;
 uint8_t* hookActive;
 uint8_t* crouchTimer;

 uint8_t* bullet1PosX;
 uint8_t* bullet1Active;
 uint8_t* bullet2Active;
 uint8_t* grabbedItem1;

 float playerPosX;
 float playerPosY;
 uint8_t bulletCount;

 // Configuration
 uint8_t timerTolerance;
 uint8_t maxBullets;

 // Trace
 bool useTrace = false;
 std::string traceFile;
 float traceTolerance;
 std::vector<std::pair<float, float>> trace;
 uint16_t* tracePos;

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
