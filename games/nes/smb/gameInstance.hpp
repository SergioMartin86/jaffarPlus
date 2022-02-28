#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"

// Datatype to describe a magnet
struct magnet_t {
 float intensity; // How strong the magnet is
 float min;  // What is the minimum input value to the calculation.
 float max;  // What is the maximum input value to the calculation.
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant mario magnets
 magnet_t marioScreenOffsetMagnet;
 magnet_t marioHorizontalMagnet;
 magnet_t marioVerticalMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint16_t* screenScroll;
  uint8_t* marioAnimation;
  uint8_t* marioState;

  uint8_t* marioBasePosX;
  uint8_t* marioRelPosX;
  uint8_t* marioSubpixelPosX;

  uint8_t* marioPosY;
  uint8_t* marioSubpixelPosY;

  uint8_t* marioMovingDirection;
  uint8_t* marioFacingDirection;
  uint8_t* marioFloatingMode;
  uint8_t* marioWalkingMode;
  uint8_t* marioWalkingDelay;
  uint8_t* marioWalkingFrame;
  int8_t* marioMaxVelLeft;
  int8_t* marioMaxVelRight;
  int8_t* marioVelX;
  int8_t* marioXMoveForce;
  int8_t* marioVelY;
  int8_t* marioFracVelY;
  uint8_t* marioGravity;
  uint8_t* marioFriction;
  uint8_t* timeLeft100;
  uint8_t* timeLeft10;
  uint8_t* timeLeft1;

  uint8_t* screenBasePosX;
  uint8_t* screenRelPosX;

  uint8_t* currentWorldRaw;
  uint8_t* currentStageRaw;
  uint8_t* levelEntryFlag;
  uint8_t* gameMode;

  uint8_t* enemy1Active;
  uint8_t* enemy2Active;
  uint8_t* enemy3Active;
  uint8_t* enemy4Active;
  uint8_t* enemy5Active;

  uint8_t* enemy1State;
  uint8_t* enemy2State;
  uint8_t* enemy3State;
  uint8_t* enemy4State;
  uint8_t* enemy5State;

  uint8_t* enemy1Type;
  uint8_t* enemy2Type;
  uint8_t* enemy3Type;
  uint8_t* enemy4Type;
  uint8_t* enemy5Type;

  uint8_t* marioCollision;
  uint8_t* enemyCollision;
  uint8_t* hitDetectionFlag;

  uint8_t* powerUpActive;

  uint8_t* animationTimer;
  uint8_t* jumpSwimTimer;
  uint8_t* runningTimer;
  uint8_t* blockBounceTimer;
  uint8_t* sideCollisionTimer;
  uint8_t* jumpspringTimer;
  uint8_t* climbSideTimer;
  uint8_t* gameControlTimer;
  uint8_t* enemyFrameTimer;
  uint8_t* frenzyEnemyTimer;
  uint8_t* bowserFireTimer;
  uint8_t* stompTimer;
  uint8_t* airBubbleTimer;
  uint8_t* fallPitTimer;
  uint8_t* multiCoinBlockTimer;
  uint8_t* invincibleTimer;
  uint8_t* starTimer;

  uint8_t* player1Input;
  uint8_t* player1Buttons;
  uint8_t* player1GamePad1;
  uint8_t* player1GamePad2;

  uint16_t* warpAreaOffset;

  // Derivative values
  uint16_t marioPosX;
  uint16_t screenPosX;
  int16_t marioScreenOffset;
  uint8_t currentWorld;
  uint8_t currentStage;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint64_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
