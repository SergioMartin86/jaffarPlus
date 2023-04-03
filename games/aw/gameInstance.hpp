#pragma once

#define _INVERSE_FRAME_RATE 16667
#define _ROOM_COUNT_ 256

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
struct magnetSet_t {
 genericMagnet_t lesterHorizontalMagnet[_ROOM_COUNT_];
 genericMagnet_t lesterVerticalMagnet[_ROOM_COUNT_];
 genericMagnet_t alienHorizontalMagnet[_ROOM_COUNT_];

 float gunChargeMagnet = 0;
 float gunPowerLoadMagnet = 0;
 float stage01VineStateMagnet = 0;
 float lesterAngularMomentumMagnet = 0;
 float shield1HorizontalMagnet = 0;
};

class GameInstance : public GameInstanceBase
{
 public:

 int16_t* randomSeed;
 int16_t* pauseSlices;
 int16_t* scrollY;
 int16_t* heroAction;
 int16_t* heroPosX;
 int16_t* heroPosY;
 int16_t* heroPosMask;
 int16_t* heroActionPosMask;
 int16_t* heroPosJumpCrouch;

 // Container for game-specific values
 uint8_t timerTolerance;


 std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
 GameInstance(EmuInstance* emu, const nlohmann::json& config);
 _uint128_t computeHash() const override;
 void updateDerivedValues() override;
 std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
 magnetSet_t getMagnetValues(const bool* rulesStatus) const;
 float getStateReward(const bool* rulesStatus) const override;
 void printStateInfo(const bool* rulesStatus) const override;
};
