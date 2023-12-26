#pragma once

#include "gameInstanceBase.hpp"
#define ROOM_COUNT 25

enum hashType
{
  NONE,
  INDEX_ONLY,
  FULL,
};

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0f; // How strong the magnet is
 uint8_t room = 0; // Which room does the magnet correspond to
 float center = 0.0f;  // What is the central point of attraction
 bool active = false; // Indicates whether the value for this magnet has specified
};

// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t kidHorizontalMagnet;
 genericMagnet_t kidVerticalMagnet;
 genericMagnet_t guardHorizontalMagnet;
 genericMagnet_t guardVerticalMagnet;
 float kidDirectionMagnet = 0.0f;
 float levelDoorOpenMagnet = 0.0f;
 float unitedWithShadowMagnet = 0.0f;
 float guardHPMagnet = 0.0f;
};

struct tileWatch_t
{
 std::map<int, hashType> _hashTypeTrobs;
 std::vector<int> _hashTypeStatic;
 std::map<std::pair<int, int>, hashType> _hashTypeMobs;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Configurable hash settings
  bool _hashKidCurrentHp;
  bool _hashGuardCurrentHp;
  bool _hashTrobCount;
  bool _hashGuardPositionX;

  uint8_t timerTolerance;
  float kidPosY;
  int kidFrameDiff;
  uint8_t deadGuardCount;

  std::vector<tileWatch_t> levelTileHashes;

  bool initializeCopyprot;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState);
};
