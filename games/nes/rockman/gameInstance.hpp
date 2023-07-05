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
 genericMagnet_t playerHorizontalMagnet;
 genericMagnet_t playerVerticalMagnet;
 genericMagnet_t gameTimerMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
 uint8_t* gameTimer           ;
 uint8_t* player1Input        ;
 uint8_t* player1Hold         ;
 uint8_t* cameraPosX1         ;
 uint8_t* cameraPosX2         ;
 uint8_t* cameraState         ;
 uint8_t* cameraPosY          ;
 uint8_t* playerPosX1         ;
 uint8_t* playerPosX2         ;
 uint8_t* playerPosX3         ;
 uint8_t* playerPosY1         ;
 uint8_t* playerPosY2         ;
 uint8_t* playerVelX         ;
 uint8_t* playerHP         ;
 uint8_t* groundState         ;
 uint8_t* playerInvincibility ;
 uint8_t* bulletOnScreen      ;
 uint8_t* sideObject          ;
 uint8_t* bullet1PosX         ;
 uint8_t* bullet2PosX         ;
 uint8_t* bullet3PosX         ;
 uint8_t* bullet1PosY         ;
 uint8_t* bullet2PosY         ;
 uint8_t* bullet3PosY         ;
 uint8_t* lagFrameState       ;
 uint8_t* lagFrameType        ;
 uint8_t* object0             ;
 uint8_t* gameMode            ;
 uint8_t* pauseMode;
 uint8_t* playerDirection;
 uint8_t* specialFrame;
 uint8_t* prevTimer;

 float playerPosX;
 float playerPosY;
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
