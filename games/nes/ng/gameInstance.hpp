#pragma once

#define _INVERSE_FRAME_RATE 16667
#define ENEMY_COUNT 8

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
struct weaponMagnet_t {
 float reward = 0.0; // How strong the magnet is
 uint8_t weapon = 0;  // Specifies the weapon number
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 genericMagnet_t ninjaHorizontalMagnet;
 genericMagnet_t ninjaVerticalMagnet;
 genericMagnet_t bossHorizontalMagnet;
 genericMagnet_t bossVerticalMagnet;
 genericMagnet_t ninjaPowerMagnet;
 weaponMagnet_t ninjaWeaponMagnet;
 float bossHealthMagnet = 0.0;
 float ninjaBossDistanceMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  frameCounter             ;
  uint8_t*  gameTime                 ;
  uint8_t*  gameMode                 ;
  uint8_t*  currentStage             ;
  uint8_t*  currentSubStage          ;
  uint8_t*  ninjalives               ;
  uint8_t*  ninjaPower               ;
  uint8_t*  ninjaHP                  ;
  uint8_t*  bossHP                   ;
  uint8_t*  ninjaStateFlags          ;
  uint8_t*  ninjaIsDead              ;
  uint8_t*  ninjaPosX                ;
  uint8_t*  ninjaPosXFrac            ;
  uint8_t*  ninjaSpeedX              ;
  uint8_t*  ninjaSpeedXFrac          ;
  uint8_t*  ninjaPosY                ;
  uint8_t*  ninjaSpeedY              ;
  uint8_t*  ninjaSpeedYFrac          ;
  uint8_t*  ninjaCollisionFlags      ;
  uint8_t*  ninjaFlinchDirection     ;
  uint8_t*  ninjaInvincibilityTimer  ;
  uint8_t*  ninjaWeapon              ;
  uint8_t*  ninjaAnimationType       ;
  uint8_t*  ninjaAnimationOffset     ;
  uint8_t*  ninjaAnimationTimer      ;
  uint8_t*  ninjaAnimationFrame      ;
  uint8_t*  enemyCount               ;
  uint8_t*  timeoutSeconds1          ;
  uint8_t*  timeoutSeconds60         ;
  uint8_t*  screenScroll1            ;
  uint8_t*  screenScroll2            ;
  uint8_t*  screenScroll3            ;

  uint8_t*  ninjaProjectile1PosX;
  uint8_t*  ninjaProjectile2PosX;
  uint8_t*  ninjaProjectile3PosX;

  uint8_t*  ninjaProjectile1PosY;
  uint8_t*  ninjaProjectile2PosY;
  uint8_t*  ninjaProjectile3PosY;

  uint8_t*  enemyPosX;
  uint8_t*  enemyPosXFrac;
  uint8_t*  enemyPosY;
  uint8_t*  enemyPosYFrac;
  uint8_t*  enemyVelX;
  uint8_t*  enemyVelXFrac;
  uint8_t*  enemyVelY;
  uint8_t*  enemyVelYFrac;

  uint8_t*  enemySlots;
  uint8_t*  enemyFlags;
  uint8_t*  enemyType;
  uint8_t*  enemyAITimer;
  uint8_t*  enemyAIStage;
  uint8_t*  enemyHP;
  uint8_t*  enemyActions;
  uint8_t*  enemyLastIdx;
  uint8_t*  enemyCollision;

  uint8_t* ppuIndicator;
  uint8_t timerTolerance;

  // Derivative Values
  double absolutePosX;
  float ninjaBossDistance;

  uint8_t ppuIndicatorBit6;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint64_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
