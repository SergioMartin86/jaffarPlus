#pragma once

#define _INVERSE_FRAME_RATE 16667
#define OBJECT_COUNT 24

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
struct enemyHPMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 uint8_t index = 0;  // Specifies the enemy index
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 genericMagnet_t ninjaHorizontalMagnet;
 genericMagnet_t ninjaVerticalMagnet;
 genericMagnet_t ninjaPowerMagnet;
 genericMagnet_t ninjaHPMagnet;
 genericMagnet_t bossHPMagnet;
 genericMagnet_t headHPMagnet;
 float ninjaBossDistanceMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameMode                 ;
  uint8_t*  currentLevel             ;
  uint8_t*  frameCounter             ;
  uint8_t*  ninjaAnimation           ;
  uint8_t*  ninjaCurrentAction       ;
  uint8_t*  ninjaState               ;
  uint8_t*  ninjaFrame               ;
  uint8_t*  ninjaAirMode             ;
  uint8_t*  ninjaLives               ;
  uint8_t*  ninjaWeapon              ;
  uint8_t*  ninjaPower               ;
  uint8_t*  ninjaPowerMax            ;
  uint8_t*  ninjaHP                  ;
  uint8_t*  ninjaPosX                ;
  uint8_t*  ninjaPosXFrac            ;
  int8_t*  ninjaSpeedX               ;
  uint8_t*  ninjaSpeedXFrac          ;
  uint8_t*  ninjaPosY                ;
  uint8_t*  ninjaPosYFrac            ;
  int8_t*   ninjaSpeedY              ;
  uint8_t*  ninjaSpeedYFrac          ;
  uint8_t*  screenScroll1            ;
  uint8_t*  screenScroll2            ;
  uint8_t*  screenScroll3            ;
  uint8_t*  bossHP                   ;
  uint8_t*  ninjaInvincibilityTimer;

  uint8_t*  windTimer            ;
  uint8_t*  windCycle            ;

  uint8_t*  heartTimer           ;
  uint8_t*  heartState           ;
  uint8_t*  headHP               ;

  uint8_t* collisionFlags;
  uint8_t* lastInputKey;
  uint8_t* lastInputTime;

  // Object Activation Bits
  uint8_t* ObjectActivationBits1;
  uint8_t* ObjectActivationBits2;
  uint8_t* ObjectActivationBits3;

  // Shadow clone state history
  uint8_t* cloneOffset;
  uint8_t* clonePosXArray;
  uint8_t* clonePosYArray;
  uint8_t* cloneStateArray;

  // Flattened Object Activation Bits
  uint8_t ObjectActivationFlags[OBJECT_COUNT];

  uint8_t timerTolerance;
  uint8_t lastInputKeyAccepted;

  // Derivative Values
  double absolutePosX;
  double ninjaBossDistance;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
