#pragma once

//#define _INVERSE_FRAME_RATE 16667
#define _INVERSE_FRAME_RATE 4166

#define _BULLET_COUNT 10
#define _ENEMY_COUNT 10

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
 float min = 0.0;  // What is the minimum input value to the calculation.
 float max = 0.0;  // What is the maximum input value to the calculation.
};


// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t playerHorizontalMagnet;
 genericMagnet_t playerVerticalMagnet;
 float scoreMagnet = 0.0f;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* gameMode;
  uint8_t* isLagFrame;
  uint8_t* frameCounter;
  uint8_t* currentLevel;
  uint8_t* playerInvincibilityTimer;
  uint8_t* playerVelX;
  uint8_t* playerVelY;
  uint8_t* playerDeadFlag;
  uint8_t* playerWeapon;
  uint8_t* screenScroll1;
  uint8_t* screenScroll2;
  uint8_t* playerAnimationFrame;
  uint8_t* playerAimDirection;
  uint8_t* playerPosScreenX;
  uint8_t* playerPosScreenY;
  uint8_t* playerBulletState;
  uint8_t* playerBulletPosX;
  uint8_t* playerBulletPosY;
  uint8_t* enemyType;
  uint8_t* enemyPosX;
  uint8_t* enemyPosY;
  uint8_t* enemyHP;
  uint8_t* score1;
  uint8_t* score2;

  // Derivative Values
  float playerPosX;
  float playerPosY;
  float score;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
