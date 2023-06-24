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
 genericMagnet_t screenHorizontalMagnet;
 genericMagnet_t playerHorizontalMagnet;
 genericMagnet_t playerVerticalMagnet;
};

class GameInstance : public GameInstanceBase
{
 public:

  // Container for game-specific values
  uint8_t* gameMode           ;
  uint8_t* screenPosX1        ;
  uint8_t* screenPosX2        ;
  uint8_t* playerPosX         ;
  uint8_t* playerPosY         ;
  uint8_t* playerJumpStrength ;
  uint8_t* playerAttackState  ;
  uint8_t* playerAction       ;
  uint8_t* playerDirection    ;
  uint8_t* shunHP2          ;
  uint8_t* shunMP2          ;
  uint8_t* seiyaHP2          ;
  uint8_t* seiyaMP2          ;
  uint8_t* MenuShunMP1            ;
  uint8_t* MenuShunMP2            ;
  uint8_t* MenuShunHP1            ;
  uint8_t* MenuShunHP2            ;

  uint8_t timerTolerance;

  float screenPosX;
  float realPlayerPosX;

  std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
  GameInstance(EmuInstance* emu, const nlohmann::json& config);
  _uint128_t computeHash(const uint16_t currentStep = 0) const override;
  void updateDerivedValues() override;
  std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
  magnetSet_t getMagnetValues(const bool* rulesStatus) const;
  float getStateReward(const bool* rulesStatus) const override;
  void printStateInfo(const bool* rulesStatus) const override;
};
