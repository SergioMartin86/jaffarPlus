#include "quickNESInstance.h"
#include "utils.h"
#include <iostream>
#include <omp.h>

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

 // Game specific values
 _screenScroll   = (uint16_t*) &_baseMem[0x071B];
 _marioRelPosX   = (uint8_t*)  &_baseMem[0x0207];
 _marioPosY      = (uint8_t*)  &_baseMem[0x00CE];
 _marioDirection = (uint8_t*)  &_baseMem[0x0033];
 _marioVelX      = (uint8_t*)  &_baseMem[0x0057];
 _marioVelY      = (uint8_t*)  &_baseMem[0x009F];
 _timeLeft100    = (uint8_t*)  &_baseMem[0x07F8];
 _timeLeft10     = (uint8_t*)  &_baseMem[0x07F9];
 _timeLeft1      = (uint8_t*)  &_baseMem[0x07FA];

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
}

void quickNESInstance::saveStateFile(const std::string& stateFilePath)
{
 // Saving state
 Nes_State state;
 _emu.save_state(&state);
 Auto_File_Writer stateWriter(stateFilePath.c_str());
 state.write(stateWriter);
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
  case 9: controllerCode = 0b10000001; break; // RB
  case 10: controllerCode = 0b11000000; break; // LR
  case 11: controllerCode = 0b11000001; break; // LRA
  case 12: controllerCode = 0b11000010; break; // LRB
  case 13: controllerCode = 0b00001000; break; // S
  default: EXIT_WITH_ERROR("Wrong move code passed %u\n", move);
 }

 // Running frame
 _emu.emulate_frame(controllerCode,0);
}

void quickNESInstance::printFrameInfo()
{
  printf("[Jaffar]  + Current World:   %1u-%1u\n", 1,1);
  printf("[Jaffar]  + Time Left:       %1u%1u%1u\n", *_timeLeft100, *_timeLeft10, *_timeLeft1);
  printf("[Jaffar]  + Mario Pos X:     %04u (%04u + %02u)\n", getScreenScroll() + *_marioRelPosX, getScreenScroll(), *_marioRelPosX);
  printf("[Jaffar]  + Mario Pos Y:     %02u\n", *_marioPosY);
  printf("[Jaffar]  + Mario Vel X:     %02d\n", *_marioVelX);
  printf("[Jaffar]  + Mario Vel Y:     %02d\n", *_marioVelY);
  printf("[Jaffar]  + Mario Direction: %s\n", *_marioDirection == 1 ? "Right" : "Left");
}
