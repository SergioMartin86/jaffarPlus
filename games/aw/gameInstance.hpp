#pragma once

#define _INVERSE_FRAME_RATE 33334
#define _ROOM_COUNT_ 256

#include "gameInstanceBase.hpp"
#include <set>

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0f; // How strong the magnet is
 uint8_t room = 0; // Which room does the magnet correspond to
 float center = 0.0f;  // What is the central point of attraction
 bool active = false; // Indicates whether the value for this magnet has specified
};

// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t lesterHorizontalMagnet;
 genericMagnet_t lesterVerticalMagnet;
 genericMagnet_t alienHorizontalMagnet;

 float gunChargeMagnet = 0;
 float gunPowerLoadMagnet = 0;
 float stage01VineStateMagnet = 0;
 float lesterAngularMomentumMagnet = 0;
 float shield1HorizontalMagnet = 0;
};

class GameInstance : public GameInstanceBase
{
 public:

 // LDKD
 int16_t* lesterSwimState;
 int16_t* lesterPosX;
 int16_t* lesterPosY;
 int16_t* lesterRoom;
 int16_t* lesterAction;
 int16_t* lesterAirMode;
 int16_t* lesterDeadState;
 int16_t* gameScriptState;
 int16_t* gameAnimState;

 // Container for game-specific values
 uint8_t timerTolerance;
 std::string levelCode;

 // Cheat engine like data
 std::vector<int16_t> vmVariablesPrevValues;
 std::vector<bool> vmVariablesDisplay;

 void vmVariablesClear();
 void vmVariablesKeepEqual();
 void vmVariablesKeepDifferent();

 std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
 GameInstance(EmuInstance* emu, const nlohmann::json& config);
 _uint128_t computeHash() const override;
 void updateDerivedValues() override;
 std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
 magnetSet_t getMagnetValues(const bool* rulesStatus) const;
 float getStateReward(const bool* rulesStatus) const override;
 void printStateInfo(const bool* rulesStatus) const override;
};
