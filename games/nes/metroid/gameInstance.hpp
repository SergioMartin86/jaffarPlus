#pragma once

#define _INVERSE_FRAME_RATE 16667

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0; // How strong the magnet is
 float center = 0.0;  // What is the central point of attraction
};

// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t samusHorizontalMagnet;
 genericMagnet_t samusVerticalMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* gameMode;
  uint8_t* NMIFlag;
  uint8_t* samusPosXRaw;
  uint8_t* screenPosX1;
  uint8_t* screenPosX2;
  uint8_t* samusPosYRaw;
  uint8_t* screenPosY1;
  uint8_t* screenPosY2;
  uint8_t* samusAnimation;
  uint8_t* samusDirection;
  uint8_t* samusDoorSide;
  uint8_t* samusDoorState;
  uint8_t* equipmentFlags;

  uint8_t* door1State;
  uint8_t* door2State;
  uint8_t* door3State;
  uint8_t* door4State;

  // Derivative Values
  float samusPosX;
  float samusPosY;

  void printFullMoveList();
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  _uint128_t computeHash() const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves() const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
  void setRNGState(const uint64_t RNGState) override;
};
