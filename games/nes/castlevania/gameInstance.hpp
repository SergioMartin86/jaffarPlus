#pragma once

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a magnet
struct magnet_t {
 float intensity; // How strong the magnet is
 float min;  // What is the minimum input value to the calculation.
 float max;  // What is the maximum input value to the calculation.
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 magnet_t simonHorizontalMagnet;
 magnet_t simonVerticalMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* RNGState;
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
  uint8_t* grabItemTimer;
  uint8_t* enemy1PosX;
  uint8_t* enemy2PosX;
  uint8_t* enemy3PosX;

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
