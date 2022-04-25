#pragma once

#define _INVERSE_FRAME_RATE 16667

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
struct stairMagnet_t {
 float reward = 0.0; // How strong the magnet is
 uint8_t mode = 0;  // Specifies the use mode of the stairs
};

// Datatype to describe a magnet
struct weaponMagnet_t {
 float reward = 0.0; // How strong the magnet is
 uint8_t weapon = 0;  // Specifies the weapon number
};

// Nametable Tile Magnet
struct nametableTileMagnet_t {
 float reward = 0.0;
 uint16_t pos = 0;
 uint8_t value = 0;
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
 float simonHeartMagnet = 0.0;
 float freezeTimeMagnet = 0.0;
 float bossStateTimerMagnet = 0.0;
 float bossHealthMagnet = 0.0;
 stairMagnet_t simonStairMagnet;
 weaponMagnet_t simonWeaponMagnet;
 std::vector<nametableTileMagnet_t> scrollTileMagnets;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* gameMode;
  uint8_t* gameSubMode;
  uint16_t* stageTimer;
  uint8_t* isLagFrame;
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
  uint8_t* batMedusa1State;
  uint8_t* batMedusa1PosX;
  uint8_t* batMedusa1PosY;
  uint8_t* freezeTimeTimer;
  uint8_t* itemDropCounter;
  uint8_t* RNGState;
  uint8_t* stairAnimationFrame;
  uint8_t* bossStateTimer;
  uint8_t* simonScreenOffsetX;
  uint8_t* levelTransitionTimer;
  uint8_t* enemy1HolyWaterLockState;
  uint8_t* holyWaterFire1Timer;

  // PPU Stage-Specific values
  uint8_t* stage51ScrollTile1;
  uint8_t* stage51ScrollTile2;

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
