#include "quickNESInstance.h"
#include "utils.h"
#include <iostream>
#include <omp.h>
#include <frame.h>

quickNESInstance::quickNESInstance(const std::string& romFilePath)
{
 std::string romData;
 loadStringFromFile(romData, romFilePath.c_str());
 Mem_File_Reader romReader(romData.data(), (int)romData.size());
 Auto_File_Reader romFile(romReader);
 auto result = _emu.load_ines(romFile);

 if (result != 0) EXIT_WITH_ERROR("Could not initialize emulator with rom file: %s\n", romFilePath.c_str());

 // Getting base and specific values' pointers
 _baseMem = _emu.low_mem();

 // Thanks to https://datacrystal.romhacking.net/wiki/Super_Mario_Bros.:RAM_map for helping me find some of these items
 // Game specific values
 _screenScroll         = (uint16_t*) &_baseMem[0x071B];
 _marioAnimation       = (uint8_t*)  &_baseMem[0x0001];
 _marioState           = (uint8_t*)  &_baseMem[0x000E];

 _marioBasePosX        = (uint8_t*)  &_baseMem[0x006D];
 _marioRelPosX         = (uint8_t*)  &_baseMem[0x0086];

 _marioPosY            = (uint8_t*)  &_baseMem[0x00CE];
 _marioMovingDirection = (uint8_t*)  &_baseMem[0x0045];
 _marioFacingDirection = (uint8_t*)  &_baseMem[0x0033];
 _marioFloatingMode    = (uint8_t*)  &_baseMem[0x001D];
 _marioWalkingMode     = (uint8_t*)  &_baseMem[0x0702];
 _marioWalkingDelay    = (uint8_t*)  &_baseMem[0x070C];
 _marioWalkingFrame    = (uint8_t*)  &_baseMem[0x070D];
 _marioMaxVelLeft      = (int8_t*)   &_baseMem[0x0450];
 _marioMaxVelRight     = (int8_t*)   &_baseMem[0x0456];
 _marioVelX            = (int8_t*)   &_baseMem[0x0057];
 _marioXMoveForce      = (int8_t*)   &_baseMem[0x0705];
 _marioVelY            = (int8_t*)   &_baseMem[0x009F];
 _marioFracVelY        = (int8_t*)   &_baseMem[0x0433];
 _marioGravity         = (uint8_t*)  &_baseMem[0x0709];
 _marioFriction        = (uint8_t*)  &_baseMem[0x0701];
 _timeLeft100          = (uint8_t*)  &_baseMem[0x07F8];
 _timeLeft10           = (uint8_t*)  &_baseMem[0x07F9];
 _timeLeft1            = (uint8_t*)  &_baseMem[0x07FA];

 _screenBasePosX       = (uint8_t*)  &_baseMem[0x071A];
 _screenRelPosX        = (uint8_t*)  &_baseMem[0x071C];

 _currentWorldRaw      = (uint8_t*)  &_baseMem[0x075F];
 _currentStageRaw      = (uint8_t*)  &_baseMem[0x075C];
 _levelEntryFlag       = (uint8_t*)  &_baseMem[0x0752];

 _enemy1Active         = (uint8_t*)  &_baseMem[0x000F];
 _enemy2Active         = (uint8_t*)  &_baseMem[0x0010];
 _enemy3Active         = (uint8_t*)  &_baseMem[0x0011];
 _enemy4Active         = (uint8_t*)  &_baseMem[0x0012];
 _enemy5Active         = (uint8_t*)  &_baseMem[0x0013];

 _enemy1State          = (uint8_t*)  &_baseMem[0x001E];
 _enemy2State          = (uint8_t*)  &_baseMem[0x001F];
 _enemy3State          = (uint8_t*)  &_baseMem[0x0020];
 _enemy4State          = (uint8_t*)  &_baseMem[0x0021];
 _enemy5State          = (uint8_t*)  &_baseMem[0x0022];

 _enemy1Type           = (uint8_t*)  &_baseMem[0x0016];
 _enemy2Type           = (uint8_t*)  &_baseMem[0x0017];
 _enemy3Type           = (uint8_t*)  &_baseMem[0x0018];
 _enemy4Type           = (uint8_t*)  &_baseMem[0x0019];
 _enemy5Type           = (uint8_t*)  &_baseMem[0x001A];

 _marioCollision       = (uint8_t*)  &_baseMem[0x0490];
 _enemyCollision       = (uint8_t*)  &_baseMem[0x0491];
 _hitDetectionFlag     = (uint8_t*)  &_baseMem[0x0722];

 _powerUpActive        = (uint8_t*)  &_baseMem[0x0014];

 _animationTimer       = (uint8_t*)  &_baseMem[0x0781];
 _jumpSwimTimer        = (uint8_t*)  &_baseMem[0x0782];
 _runningTimer         = (uint8_t*)  &_baseMem[0x0783];
 _blockBounceTimer     = (uint8_t*)  &_baseMem[0x0784];
 _sideCollisionTimer   = (uint8_t*)  &_baseMem[0x0785];
 _jumpspringTimer      = (uint8_t*)  &_baseMem[0x0786];
 _climbSideTimer       = (uint8_t*)  &_baseMem[0x0787];
 _enemyFrameTimer      = (uint8_t*)  &_baseMem[0x0789];
 _frenzyEnemyTimer     = (uint8_t*)  &_baseMem[0x078F];
 _bowserFireTimer      = (uint8_t*)  &_baseMem[0x0790];
 _stompTimer           = (uint8_t*)  &_baseMem[0x0791];
 _airBubbleTimer       = (uint8_t*)  &_baseMem[0x0792];
 _fallPitTimer         = (uint8_t*)  &_baseMem[0x0795];
 _multiCoinBlockTimer  = (uint8_t*)  &_baseMem[0x079D];
 _invincibleTimer      = (uint8_t*)  &_baseMem[0x079E];
 _starTimer            = (uint8_t*)  &_baseMem[0x079F];

 _player1Input         = (uint8_t*)  &_baseMem[0x06FC];
 _player1Buttons       = (uint8_t*)  &_baseMem[0x074A];
 _player1GamePad1      = (uint8_t*)  &_baseMem[0x000A];
 _player1GamePad2      = (uint8_t*)  &_baseMem[0x000D];

 _warpAreaOffset       = (uint16_t*) &_baseMem[0x0750];
}

void quickNESInstance::loadStateFile(const std::string& stateFilePath)
{
 // Loading state data
 std::string stateData;
 if (loadStringFromFile(stateData, stateFilePath.c_str()) == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());
 Mem_File_Reader stateReader(stateData.data(), (int)stateData.size());
 Auto_File_Reader stateFile(stateReader);

 // Loading state data into state object
 Nes_State state;
 state.read(stateFile);

 // Loading state object into the emulator
 _emu.load_state(state);

 // Updating derivative values after load
 updateDerivedValues();
}

void quickNESInstance::saveStateFile(const std::string& stateFilePath)
{
 // Saving state
 Nes_State state;
 _emu.save_state(&state);
 Auto_File_Writer stateWriter(stateFilePath.c_str());
 state.write(stateWriter);
}

void quickNESInstance::updateDerivedValues()
{
 // Recalculating derived values
 _marioPosX = (uint16_t)*_marioBasePosX * 256 + (uint16_t)*_marioRelPosX;
 _screenPosX = (uint16_t)*_screenBasePosX * 256 + (uint16_t)*_screenRelPosX;
 _marioScreenOffset = _marioPosX - _screenPosX;
 _currentWorld = *_currentWorldRaw + 1;
 _currentStage = *_currentStageRaw + 1;
}

void quickNESInstance::serializeState(uint8_t* state) const
{
 Mem_Writer w(state, _FRAME_DATA_SIZE, 0);
 Auto_File_Writer a(w);
 _emu.save_state(a);
}

void quickNESInstance::deserializeState(const uint8_t* state)
{
 Mem_File_Reader r(state, _FRAME_DATA_SIZE);
 Auto_File_Reader a(r);
 _emu.load_state(a);
 updateDerivedValues();
}

void quickNESInstance::serializeStateSmall(uint8_t* state) const
{
 _emu.serialize(state);
}

void quickNESInstance::deserializeStateSmall(const uint8_t* state)
{
 _emu.deserialize(state);
 updateDerivedValues();
}

void quickNESInstance::advanceFrame(const uint8_t &move)
{
 // Controller input bits
 // 0 - A / 1
 // 1 - B / 2
 // 2 - Select / 4
 // 3 - Start / 8
 // 4 - Up / 16
 // 5 - Down / 32
 // 6 - Left / 64
 // 7 - Right / 128

 // Possible moves
 // Move Ids =        0    1    2    3    4    5     6     7     8    9     10    11      12    13
 //_possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "S" };

 // Encoding movement into the NES controller code
 uint32_t controllerCode = 0;
 switch (move)
 {
  case 0: controllerCode = 0b00000000; break; // .
  case 1: controllerCode = 0b01000000; break; // L
  case 2: controllerCode = 0b10000000; break; // R
  case 3: controllerCode = 0b00100000; break; // D
  case 4: controllerCode = 0b00000001; break; // A
  case 5: controllerCode = 0b00000010; break; // B
  case 6: controllerCode = 0b01000001; break; // LA
  case 7: controllerCode = 0b10000001; break; // RA
  case 8: controllerCode = 0b01000010; break; // LB
  case 9: controllerCode = 0b10000010; break; // RB
  case 10: controllerCode = 0b11000000; break; // LR
  case 11: controllerCode = 0b11000001; break; // LRA
  case 12: controllerCode = 0b11000010; break; // LRB
  case 13: controllerCode = 0b00001000; break; // S
  default: EXIT_WITH_ERROR("Wrong move code passed %u\n", move);
 }

 // Running frame
 _emu.emulate_frame(controllerCode,0);

 // Updating derived values
 updateDerivedValues();
}

void quickNESInstance::printFrameInfo()
{
  printf("[JaffarNES]  + Current World-Stage:    %1u-%1u\n", _currentWorld, _currentStage);
  printf("[JaffarNES]  + Time Left:              %1u%1u%1u\n", *_timeLeft100, *_timeLeft10, *_timeLeft1);
  printf("[JaffarNES]  + Mario Animation:        %02u\n", *_marioAnimation);
  printf("[JaffarNES]  + Mario State:            %02u\n", *_marioState);
  printf("[JaffarNES]  + Screen Pos X:           %04u (%02u * 256 = %04u + %02u)\n", _screenPosX, *_screenBasePosX, (uint16_t)*_screenBasePosX * 255, *_screenRelPosX);
  printf("[JaffarNES]  + Mario Pos X:            %04u (%02u * 256 = %04u + %02u)\n", _marioPosX, *_marioBasePosX, (uint16_t)*_marioBasePosX * 255, *_marioRelPosX);
  printf("[JaffarNES]  + Mario / Screen Offset:  %04d\n", _marioScreenOffset);
  printf("[JaffarNES]  + Mario Pos Y:            %02u\n", *_marioPosY);
  printf("[JaffarNES]  + Mario Vel X:            %02d (Force: %02d, MaxL: %02d, MaxR: %02d)\n", *_marioVelX, *_marioXMoveForce, *_marioMaxVelLeft, *_marioMaxVelRight);
  printf("[JaffarNES]  + Mario Vel Y:            %02d (%02d)\n", *_marioVelY, *_marioFracVelY);
  printf("[JaffarNES]  + Mario Gravity:          %02u\n", *_marioGravity);
  printf("[JaffarNES]  + Mario Friction:         %02u\n", *_marioFriction);
  printf("[JaffarNES]  + Mario Moving Direction: %s\n", *_marioMovingDirection == 1 ? "Right" : "Left");
  printf("[JaffarNES]  + Mario Facing Direction: %s\n", *_marioFacingDirection == 1 ? "Right" : "Left");
  printf("[JaffarNES]  + Mario Floating Mode:    %02u\n", *_marioFloatingMode);
  printf("[JaffarNES]  + Mario Walking:          %02u %02u %02u\n", *_marioWalkingMode, *_marioWalkingDelay, *_marioWalkingFrame);
  printf("[JaffarNES]  + Player 1 Inputs:        %02u %02u %02u %02u\n", *_player1Input, *_player1Buttons, *_player1GamePad1, *_player1GamePad2);
  printf("[JaffarNES]  + Powerup Active:         %1u\n", *_powerUpActive);
  printf("[JaffarNES]  + Enemy Active:           %1u%1u%1u%1u%1u\n", *_enemy1Active, *_enemy2Active, *_enemy3Active, *_enemy4Active, *_enemy5Active);
  printf("[JaffarNES]  + Enemy State:            %02u %02u %02u %02u %02u\n", *_enemy1State, *_enemy2State, *_enemy3State, *_enemy4State, *_enemy5State);
  printf("[JaffarNES]  + Enemy Type:             %02u %02u %02u %02u %02u\n", *_enemy1Type, *_enemy2Type, *_enemy3Type, *_enemy4Type, *_enemy5Type);
  printf("[JaffarNES]  + Hit Detection Flags:    %02u %02u %02u\n", *_marioCollision, *_enemyCollision, *_hitDetectionFlag);
  printf("[JaffarNES]  + Level Entry Flag:       %02u\n", *_levelEntryFlag);
  printf("[JaffarNES]  + Warp Area Offset:       %04u\n", *_warpAreaOffset);
  printf("[JaffarNES]  + Timers:                 %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u %02u\n", *_animationTimer, *_jumpSwimTimer, *_runningTimer, *_blockBounceTimer, *_sideCollisionTimer, *_jumpspringTimer, *_climbSideTimer, *_enemyFrameTimer, *_frenzyEnemyTimer, *_bowserFireTimer, *_stompTimer, *_airBubbleTimer, *_multiCoinBlockTimer, *_invincibleTimer, *_starTimer);
}
