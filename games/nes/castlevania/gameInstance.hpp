#pragma once

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity; // How strong the magnet is
 float center;  // What is the central point of attraction
 float min;  // What is the minimum input value to the calculation.
 float max;  // What is the maximum input value to the calculation.
};

// Datatype to describe a magnet
struct stairMagnet_t {
 float reward; // How strong the magnet is
 uint8_t mode;  // Specifies the use mode of the stairs
};

// Datatype to describe a magnet
struct weaponMagnet_t {
 float reward; // How strong the magnet is
 uint8_t weapon;  // Specifies the weapon number
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 genericMagnet_t simonHorizontalMagnet;
 genericMagnet_t simonVerticalMagnet;
 genericMagnet_t bossHorizontalMagnet;
 genericMagnet_t bossVerticalMagnet;
 genericMagnet_t batMedusaHorizontalMagnet;
 genericMagnet_t batMedusaVerticalMagnet;
 genericMagnet_t simonHeartMagnet;
 float freezeTimeMagnet;
 stairMagnet_t simonStairMagnet;
 weaponMagnet_t simonWeaponMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* gameMode;
  uint8_t* gameSubMode;
  uint16_t* stageTimer;
  uint8_t* currentStage;
  uint8_t* currentSubStage;
  uint8_t* simonLives;
  uint16_t* screenOffset;
  uint8_t* simonStairMode;
  uint8_t* simonPosY;
  uint16_t* simonPosX;
  uint8_t* simonHealth;
  uint8_t* simonInvulnerability;
  uint8_t* simonKneelingMode;
  uint8_t* subweaponShotCount;
  uint8_t* whipLength;
  uint8_t* simonHeartCount;
  uint8_t* simonImage;
  uint8_t* subweaponNumber;
  uint8_t* simonFacingDirection;
  uint8_t* simonState;
  uint8_t* simonSubState;
  uint8_t* simonVerticalSpeed;
  uint8_t* simonVerticalDirection;
  uint8_t* bossHealth;
  uint8_t* bossPosX;
  uint8_t* bossPosY;
  uint8_t* bossIsActive;
  uint8_t* bossImage;
  uint8_t* bossState;
  uint8_t* grabItemTimer;
  uint8_t* enemy1PosX;
  uint8_t* enemy2PosX;
  uint8_t* enemy3PosX;
  uint8_t* enemy1State;
  uint8_t* enemy2State;
  uint8_t* enemy3State;
  uint8_t* batMedusa1PosX;
  uint8_t* batMedusa1PosY;
  uint8_t* freezeTimeTimer;
  uint8_t* itemDropCounter;
  uint8_t* RNGState;

  // Hash-specific configuration
  std::set<std::string> hashIncludes;

  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  uint64_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
