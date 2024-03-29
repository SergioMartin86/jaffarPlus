#pragma once

#define _INVERSE_FRAME_RATE 33334
#define _ROOM_COUNT_ 256

#include "gameInstanceBase.hpp"
#include <set>

#define LESTER_DIR_LEFT 1
#define LESTER_DIR_RIGHT 2
#define LESTER_DIR_NONE 100

// Datatype to describe a generic magnet
struct genericMagnet_t {
 float intensity = 0.0f; // How strong the magnet is
 uint8_t room = 0; // Which room does the magnet correspond to
 float center = 0.0f;  // What is the central point of attraction
 bool active = false; // Indicates whether the value for this magnet has specified
};

// Datatype to describe a magnet
struct magnetSet_t {
 genericMagnet_t alienHorizontalMagnet;
 genericMagnet_t lesterHorizontalMagnet;
 genericMagnet_t lesterVerticalMagnet;
 genericMagnet_t elevatorVerticalMagnet;

 float   lesterDirectionMagnet = 0;
 float gunChargeMagnet = 0;
 float lesterGunLoadMagnet = 0;
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
 int16_t* lesterMomentum1;
 int16_t* lesterMomentum2;
 int16_t* lesterMomentum3;
 int16_t* lesterRoom;
 int16_t* lesterAction;
 int16_t* lesterDirection;
 int16_t* lesterState;
 int16_t* lesterDeadState;
 int16_t* gameScriptState;
 int16_t* gameAnimState;
 int16_t* lesterHasGun;
 int16_t* lesterGunAmmo;
 int16_t* lesterGunLoad;
 int16_t* gameTimer;
 int16_t* fumesState;

 int16_t* alienState;
 int16_t* alienRoom;
 int16_t* alienPosX;

 int16_t* elevatorPosY;

 // Container for game-specific values
 uint8_t timerTolerance;
 std::string levelCode;

 // Derivative Values
 float lesterFullMomentum;

 // Cheat engine like data
 std::vector<int16_t> vmVariablesPrevValues;
 std::vector<bool> vmVariablesDisplay;

 std::set<int> watchVMVariables;

 void vmVariablesClear();
 void vmVariablesKeepEqual();
 void vmVariablesKeepDifferent();
 void vmVariablesKeepGreater();
 void vmVariablesKeepSmaller();

 std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) override;
 GameInstance(EmuInstance* emu, const nlohmann::json& config);
 _uint128_t computeHash(const uint16_t currentStep = 0) const override;
 void updateDerivedValues() override;
 std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const override;
 magnetSet_t getMagnetValues(const bool* rulesStatus) const;
 float getStateReward(const bool* rulesStatus) const override;
 void printStateInfo(const bool* rulesStatus) const override;
};
