#pragma once

#include "gameInstanceBase.hpp"
#include <set>

#define ROOM_COUNT 24

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity; // How strong the magnet is
 uint8_t room; // Which room does the magnet correspond to
 float center;  // What is the central point of attraction
 float min;  // What is the minimum input value to the calculation.
 float max;  // What is the maximum input value to the calculation.
};

// Datatype to describe a magnet
struct magnetSet_t {
 // Relevant simon magnets
 genericMagnet_t kidHorizontalMagnet;
 genericMagnet_t kidVerticalMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* RNGState;
  uint8_t* framePhase;
  uint16_t* kidPosX;
  uint8_t* kidPosY;
  uint8_t* kidFrame;
  uint8_t* kidMovement;
  uint8_t* kidFallWait;
  uint8_t* kidHP;
  uint8_t* kidRoom;
  uint8_t* guardPosX;
  uint8_t* guardHP;
  uint8_t* guardNotice;
  uint8_t* guardFrame;
  uint8_t* guardMovement;
  uint8_t* drawnRoom;
  uint8_t* screenTransition;
  uint8_t* globalTimer;
  uint8_t* screenDrawn;
  uint8_t* isPaused;

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
