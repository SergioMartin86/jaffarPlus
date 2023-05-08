#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "gfx.h"
#include "snapshot.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "logger.h"
#include "display.h"
#include "conffile.h"
#include "statemanager.h"

#include "unix.hpp"
#include "snes/snes.hpp"
extern thread_local bool doRendering;

class EmuInstance : public EmuInstanceBase
{
 public:

 uint8_t* _baseMem;
 uint8_t* _apuMem;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  int argc = 2;
  char romPath[4096];
  char bin[] = "./snes9x";
  strcpy(romPath, romFilePath.c_str());
  char* argv[2] = { bin, romPath };

  doRendering = false;
  initSnes9x(argc, argv);

  _baseMem = Memory.RAM;
  _apuMem = SNES::smp.apuram;
//  saveStateFile("boot.state");
//  exit(0);

   loadStateFile(stateFilePath);

//   Printing State size
//  printf("State Size: %u\n", S9xFreezeSizeFast());
//  exit(0);
 }

 void loadStateFile(const std::string& stateFilePath) override
 {
  if (stateFilePath == "") { S9xReset(); return; }

  // Loading state data
  std::string stateData;
  if (loadStringFromFile(stateData, stateFilePath.c_str()) == false) EXIT_WITH_ERROR("Could not find/read state file: '%s'\n", stateFilePath.c_str());

  // Loading state object into the emulator
  memStream stream((uint8_t*)stateData.data(), S9xFreezeSize());
  S9xUnfreezeFromStream(&stream);
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  std::string stateBuffer;
  stateBuffer.resize(S9xFreezeSize());

  memStream stream((uint8_t*)stateBuffer.data(), S9xFreezeSize());
  S9xFreezeToStream(&stream);

  saveStringToFile(stateBuffer, stateFilePath.c_str());
 }

 void serializeState(uint8_t* state) const override
 {
#ifdef PREVENT_RENDERING
  memStream stream(state, _STATE_DATA_SIZE_TRAIN);
  S9xFreezeToStreamFast(&stream);
#else
  memStream stream(state, _STATE_DATA_SIZE_PLAY);
  S9xFreezeToStream(&stream);
#endif
 }

 void deserializeState(const uint8_t* state) override
 {
#ifdef PREVENT_RENDERING
  memStream stream(state, _STATE_DATA_SIZE_TRAIN);
  S9xUnfreezeFromStreamFast(&stream);
#else
  memStream stream(state, _STATE_DATA_SIZE_PLAY);
  S9xUnfreezeFromStream(&stream);
#endif
 }

 static INPUT_TYPE moveStringToCode(const std::string& move)
 {
  INPUT_TYPE moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
   case 'U': moveCode |= SNES_UP_MASK; break;
   case 'D': moveCode |= SNES_DOWN_MASK; break;
   case 'L': moveCode |= SNES_LEFT_MASK; break;
   case 'R': moveCode |= SNES_RIGHT_MASK; break;
   case 'A': moveCode |= SNES_A_MASK; break;
   case 'B': moveCode |= SNES_B_MASK; break;
   case 'X': moveCode |= SNES_X_MASK; break;
   case 'Y': moveCode |= SNES_Y_MASK; break;
   case 'l': moveCode |= SNES_TL_MASK; break;
   case 'r': moveCode |= SNES_TR_MASK; break;
   case 'S': moveCode |= SNES_START_MASK; break;
   case 's': moveCode |= SNES_SELECT_MASK; break;
   case '.': break;
   case '|': break;
  }

  //printf("move %s - %u\n", move.c_str(), moveCode);

  return moveCode;
 }

 static std::string moveCodeToString(const INPUT_TYPE move)
 {
  std::string moveString = "|..|";

  if (move & SNES_UP_MASK)     moveString += 'U'; else moveString += '.';
  if (move & SNES_DOWN_MASK)   moveString += 'D'; else moveString += '.';
  if (move & SNES_LEFT_MASK)   moveString += 'L'; else moveString += '.';
  if (move & SNES_RIGHT_MASK)  moveString += 'R'; else moveString += '.';
  if (move & SNES_SELECT_MASK) moveString += 's'; else moveString += '.';
  if (move & SNES_START_MASK)  moveString += 'S'; else moveString += '.';
  if (move & SNES_Y_MASK)      moveString += 'Y'; else moveString += '.';
  if (move & SNES_B_MASK)      moveString += 'B'; else moveString += '.';
  if (move & SNES_X_MASK)      moveString += 'X'; else moveString += '.';
  if (move & SNES_A_MASK)      moveString += 'A'; else moveString += '.';
  if (move & SNES_TL_MASK)     moveString += 'l'; else moveString += '.';
  if (move & SNES_TR_MASK)     moveString += 'r'; else moveString += '.';

//  printf("move %u - %s\n", move, moveString.c_str());

  moveString += "|............|";
  return moveString;
 }

 void advanceState(const INPUT_TYPE move) override
 {
  MovieSetJoypad(0, move);
  S9xMainLoop();
  //printf("move %u -> %u\n", move, *((uint16*)&_baseMem[0x0272]));
 }

};
