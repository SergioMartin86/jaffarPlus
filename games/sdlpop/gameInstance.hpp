#pragma once

#include "gameInstanceBase.hpp"

enum hashType
{
  NONE,
  INDEX_ONLY,
  FULL,
};

// Struct to hold all of the frame's magnet information
struct magnetInfo_t
{
 float positionX;
 float intensityX;
 float intensityY;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Configurable hash settings
  bool _hashKidCurrentHp;
  bool _hashGuardCurrentHp;
  bool _hashTrobCount;

  uint8_t timerTolerance;

  std::map<int, hashType> _hashTypeTrobs;
  std::vector<int> _hashTypeStatic;
  std::map<std::pair<int, int>, hashType> _hashTypeMobs;

  // Function to get magnet information
  magnetInfo_t getKidMagnetValues(const bool* rulesStatus, const int room) const;
  magnetInfo_t getGuardMagnetValues(const bool* rulesStatus, const int room) const;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
