#pragma once

#define _INVERSE_FRAME_RATE 16667
#define ENEMY_COUNT 8
#define ORB_COUNT 8

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
 genericMagnet_t ninjaPowerMagnet;
 genericMagnet_t ninjaHPMagnet;
 genericMagnet_t bossHPMagnet;
 weaponMagnet_t ninjaWeaponMagnet;
 float ninjaBossDistanceMagnet = 0.0;
 float traceMagnet = 0.0;
};


class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t*  gameMode                 ;
  uint8_t*  currentStage             ;
  uint8_t*  globalTimer             ;
  uint8_t*  ninjaAnimation1           ;
  uint8_t*  ninjaAnimation2           ;
  uint8_t*  ninjaAction       ;
  uint8_t*  ninjaFrame               ;
  uint8_t*  ninjaWeapon              ;
  uint8_t*  ninjaPower               ;
  uint8_t*  ninjaPowerMax            ;
  uint8_t*  ninjaHP                  ;
  uint8_t*  ninjaPosX1               ;
  uint8_t*  ninjaPosX2               ;
  int8_t*   ninjaSpeedX1              ;
  uint8_t*  ninjaSpeedX2              ;
  uint8_t*  ninjaPosY1                ;
  uint8_t*  ninjaPosY2              ;
  int8_t*   ninjaSpeedY1           ;
  uint8_t*  ninjaSpeedY2           ;
  uint8_t*  screenPosX1            ;
  uint8_t*  screenPosX2            ;
  uint8_t*  screenPosX3            ;
  uint8_t*  screenPosY1            ;
  uint8_t*  screenPosY2            ;
  uint8_t*  screenPosY3            ;
  uint8_t*  bossHP                   ;
  uint8_t*  bossPosY                   ;
  uint8_t*  bossPosX                   ;
  uint8_t*  ninjaInvincibilityTimer;
  uint8_t*  ninjaInvincibilityState;
  uint8_t*  ninjaSwordType              ;
  uint8_t*  ninjaVerticalCollision              ;
  uint8_t*  ninjaHorizontalCollision              ;
  uint8_t*  ninjaDirection              ;
  uint8_t*  bufferedMovement;
  uint8_t* levelExitFlag;
  uint8_t* levelExitFlag2;

  uint8_t*  orbStateVector                   ;
  uint8_t*  enemyStateVector                   ;

  uint8_t* collisionFlags;

  uint8_t timerTolerance;
  bool enableB;

  // Trace
  bool useTrace = false;
  std::string traceFile;
  float traceTolerance;
  std::vector<std::pair<float, float>> trace;
  size_t tracePos;

  // Derivative Values
  float absolutePosX;
  float absolutePosY;
  float ninjaBossDistance;

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
