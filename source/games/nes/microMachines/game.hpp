#pragma once

#include <common/json.hpp>
#include "../../game.hpp"

namespace jaffarPlus
{

namespace games
{

namespace NES
{

namespace microMachines
{

class Game : public jaffarPlus::Game
{
 public:

  static inline std::string getName() { return "NES / Micro Machines"; } 

  size_t getGameSpecificStorageSize() const override { return sizeof(uint32_t); }

  Game(std::unique_ptr<Emulator>& emulator, const nlohmann::json& config) : jaffarPlus::Game(emulator, config)
  {
    // Timer tolerance
    timerTolerance = JSON_GET_NUMBER(uint8_t, config, "Timer Tolerance");
  }

  void initializeImpl() override
  {
    // Getting NES low RAM pointer
    auto ram = _emulator->getProperty("LRAM").pointer;

    // Game-specific values
    frameType                         = (uint8_t*)   &ram[0x007C];
    lagFrame                          = (uint8_t*)   &ram[0x01F8];
    cameraPosX1                       = (uint8_t*)   &ram[0x00D5];
    cameraPosX2                       = (uint8_t*)   &ram[0x00D4];
    cameraPosY1                       = (uint8_t*)   &ram[0x00D7];
    cameraPosY2                       = (uint8_t*)   &ram[0x00D6];
    currentRace                       = (uint8_t*)   &ram[0x0308];

    player1PosX1                      = (uint8_t*)   &ram[0x03DE];
    player1PosX2                      = (uint8_t*)   &ram[0x03DA];
    player1PosY1                      = (uint8_t*)   &ram[0x03EA];
    player1PosY2                      = (uint8_t*)   &ram[0x03E6];
    player1PosY3                      = (uint8_t*)   &ram[0x03EE];
    player1Accel                      = (int8_t*)    &ram[0x0386];
    player1AccelTimer1                = (uint8_t*)   &ram[0x0102];
    player1AccelTimer2                = (uint8_t*)   &ram[0x0103];
    player1AccelTimer3                = (uint8_t*)   &ram[0x010E];
    player1Inertia1                   = (uint8_t*)   &ram[0x00B0];
    player1Inertia2                   = (uint8_t*)   &ram[0x00B2];
    player1Inertia3                   = (uint8_t*)   &ram[0x00B4];
    player1Inertia4                   = (uint8_t*)   &ram[0x00B6];
    player1Angle1                     = (uint8_t*)   &ram[0x04B2];
    player1Angle2                     = (uint8_t*)   &ram[0x040A];
    player1Angle3                     = (uint8_t*)   &ram[0x04CA];
    player1LapsRemaining              = (uint8_t*)   &ram[0x04B6];
    player1LapsRemainingPrev          = (uint8_t*)   &ram[0x07FF];
    player1Checkpoint                 = (uint8_t*)   &ram[0x04CE];
    player1Input1                     = (uint8_t*)   &ram[0x009B];
    player1Input2                     = (uint8_t*)   &ram[0x00CF];
    player1Input3                     = (uint8_t*)   &ram[0x0352];

    player1RecoveryMode               = (uint8_t*)   &ram[0x0416];
    player1RecoveryTimer              = (uint8_t*)   &ram[0x041A];
    player1ResumeTimer                = (uint8_t*)   &ram[0x00DA];
    player1CanControlCar              = (uint8_t*)   &ram[0x01BF];
    player1TankFireTimer              = (uint8_t*)   &ram[0x041E];

    preRaceTimer                      = (uint8_t*)   &ram[0x00DD];
    activeFrame1                      = (uint8_t*)   &ram[0x00B0];
    activeFrame2                      = (uint8_t*)   &ram[0x00B2];
    activeFrame3                      = (uint8_t*)   &ram[0x00B4];
    activeFrame4                      = (uint8_t*)   &ram[0x00B6];

    playerLastInputKey                = (uint8_t*)   &ram[0x009B];
    playerLastInputFrame              = (uint16_t*)  &ram[0x01A0];

    // Game-specific values
    currentStep                       = (uint32_t*)  &_gameSpecificStorage[0x0000];
    *currentStep = 0;
  }

  std::vector<std::string> advanceStateImpl(const std::string& input) override
  {
    *player1LapsRemainingPrev = *player1LapsRemaining;
    *currentStep = *currentStep + 1;

    _emulator->advanceState(input);

    if (input != "|..|........|") *playerLastInputFrame = *currentStep;

    return {input};
  }

  hash_t computeHash() const override
  {
    // Storage for hash calculation
    MetroHash128 hash;

    if (timerTolerance > 0) hash.Update(*currentStep % (timerTolerance+1));

    hash.Update(*cameraPosX1);
    hash.Update(*cameraPosX2);
    hash.Update(*cameraPosY1);
    hash.Update(*cameraPosY2);
    hash.Update(*currentRace);

    hash.Update(*frameType);
    hash.Update(*lagFrame);
    hash.Update(*preRaceTimer);
    hash.Update(*player1PosX1);
    hash.Update(*player1PosX2);
    hash.Update(*player1PosY1);
    hash.Update(*player1PosY2);
    // hash.Update(*player1PosY3);
    hash.Update(*player1Accel);
    hash.Update(*player1AccelTimer1);
    hash.Update(*player1AccelTimer2);
    hash.Update(*player1AccelTimer3);
    hash.Update(*player1Inertia1);
    hash.Update(*player1Inertia2);
    hash.Update(*player1Inertia3);
    hash.Update(*player1Inertia4);
    // hash.Update(*player1Angle1);
    hash.Update(*player1Angle2);
    // hash.Update(*player1Angle3);
    hash.Update(*player1LapsRemaining);
    hash.Update(*player1LapsRemainingPrev);
    hash.Update(*player1Checkpoint);

    hash.Update(*player1RecoveryMode);
    hash.Update(*player1RecoveryTimer);
    hash.Update(*player1CanControlCar);
    hash.Update(*player1ResumeTimer);
    hash.Update(*player1TankFireTimer > 0);

    // hash.Update(*activeFrame1);
    // hash.Update(*activeFrame2);
    // hash.Update(*activeFrame3);
    // hash.Update(*activeFrame4);
    // hash.Update(_emu->_baseMem[0x039E]);
    // hash.Update(*playerLastInputFrame);
    
    _uint128_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  void updateGameSpecificValues() override
  {
    player1PosX = (uint16_t)*player1PosX1 * 256 + (uint16_t)*player1PosX2;
    player1PosY = (uint16_t)*player1PosY1 * 256 + (uint16_t)*player1PosY2;

    cameraPosX = (uint16_t)*cameraPosX1 * 256 + (uint16_t)*cameraPosX2;
    cameraPosY = (uint16_t)*cameraPosY1 * 256 + (uint16_t)*cameraPosY2;
  }

  void printStateInfoImpl() const override
  {
    LOG("[J+]  + Current Race:                       %03u\n", *currentRace);
    LOG("[J+]  + Current Step:                       %03u\n", *currentStep);
    LOG("[J+]  + Pre Race Timer:                     %03u\n", *preRaceTimer);
    LOG("[J+]  + Frame Type / Lag:                   %03u %03u\n", *frameType, *lagFrame);

    LOG("[J+]  + Active Frames:                      %03u %03u %03u %03u\n", *activeFrame1, *activeFrame2, *activeFrame3, *activeFrame4 );

    LOG("[J+]  + Last Input / Frame                  %03u\n",  *player1Input1);
    LOG("[J+]  + Laps Remaining:                     %1u (Prev: %1u)\n", *player1LapsRemaining, *player1LapsRemainingPrev);
    LOG("[J+]  + Checkpoint Id:                      %03u\n", *player1Checkpoint);

    LOG("[J+]  + Player Accel (Timers):              %03d (%03u, %03u, %03u)\n", *player1Accel, *player1AccelTimer1, *player1AccelTimer2, *player1AccelTimer3);
    LOG("[J+]  + Player Inertia:                     %03u, %03u, %03u, %03u\n", *player1Inertia1, *player1Inertia2, *player1Inertia3, *player1Inertia4);
    LOG("[J+]  + Player Angle:                       %03u %03u\n", *player1Angle1, *player1Angle2);

    LOG("[J+]  + Camera Pos X:                       %05u\n", cameraPosX);
    LOG("[J+]  + Camera Pos Y:                       %05u\n", cameraPosY);

    LOG("[J+]  + Player Pos X:                       %05u\n", player1PosX);
    LOG("[J+]  + Player Pos Y:                       %05u\n", player1PosY);
    LOG("[J+]  + Player Recovery:                    %03u (%03u)\n", *player1RecoveryMode, *player1RecoveryTimer);
    LOG("[J+]  + Player Resume Timer:                %03u\n", *player1ResumeTimer);
    LOG("[J+]  + Player Can Control Car:             %03u\n", *player1CanControlCar);
    LOG("[J+]  + Player Tank Fire Timer:             %03u\n", *player1TankFireTimer);

    LOG("[J+]  + Last Input / Frame                  %03u / %03u\n",  *playerLastInputKey, *playerLastInputFrame);
  }

  private:

  // Container for game-specific values
  uint8_t*  currentRace;
  uint8_t*  preRaceTimer;
  uint8_t*  frameType;
  uint8_t*  lagFrame;

  uint8_t*  cameraPosX1;
  uint8_t*  cameraPosX2;
  uint8_t*  cameraPosY1;
  uint8_t*  cameraPosY2;

  uint8_t*  player1PosX1;
  uint8_t*  player1PosX2;
  uint8_t*  player1PosY1;
  uint8_t*  player1PosY2;
  uint8_t*  player1PosY3;
  uint8_t*  player1Inertia1;
  uint8_t*  player1Inertia2;
  uint8_t*  player1Inertia3;
  uint8_t*  player1Inertia4;
  uint8_t*  player1AccelTimer1;
  uint8_t*  player1AccelTimer2;
  uint8_t*  player1AccelTimer3;
  int8_t*   player1Accel;
  uint8_t*  player1Angle1;
  uint8_t*  player1Angle2;
  uint8_t*  player1Angle3;
  uint8_t*  player1LapsRemaining;
  uint8_t*  player1LapsRemainingPrev;
  uint8_t*  player1Checkpoint;
  uint8_t*  player1Input1;
  uint8_t*  player1Input2;
  uint8_t*  player1Input3;

  uint8_t*  player1RecoveryMode;
  uint8_t*  player1RecoveryTimer;
  uint8_t*  player1CanControlCar;
  uint8_t*  player1ResumeTimer;
  uint8_t*  player1TankFireTimer;
  uint8_t*  playerLastInputKey;
  uint16_t*  playerLastInputFrame;

  uint8_t* activeFrame1;
  uint8_t* activeFrame2;
  uint8_t* activeFrame3;
  uint8_t* activeFrame4;

  uint16_t player1PosX;
  uint16_t player1PosY;
  uint16_t cameraPosX;
  uint16_t cameraPosY;
  uint8_t timerTolerance;
  uint32_t* currentStep;
};

} // namespace microMachines

} // namespace NES

} // namespace games

} // namespace jaffarPlus