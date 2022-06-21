#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>

#include "shared.h"
#include "sms_ntsc.h"
#include "md_ntsc.h"
#include "main.h"

class EmuInstance : public EmuInstanceBase
{
 public:

 uint8_t* _68KRam;
 uint8_t* _Z80Ram;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  /* set default config */
  error_init();
  set_config_defaults();

  /* mark all BIOS as unloaded */
  system_bios = 0;
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();
  if(!load_rom(romFilePath.c_str())) EXIT_WITH_ERROR("Could not initialize emulator with rom file: %s\n", romFilePath.c_str());

  /* initialize system hardware */
  system_init();
  system_reset();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();
  loadStateFile(stateFilePath);

  // Getting pointer to 68K and Z80 Ram
  _68KRam = work_ram;
  _Z80Ram = zram;
 }

 void loadStateFile(const std::string& stateFilePath) override
 {
  // Loading state data
  std::string stateData;
  if (loadStringFromFile(stateData, stateFilePath.c_str()) == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());

  // Loading state object into the emulator
  state_load((uint8_t*)stateData.c_str());
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  std::string stateBuffer;
  stateBuffer.resize(STATE_SIZE);
  size_t stateSize = state_save((uint8_t*)stateBuffer.c_str());
  stateBuffer.resize(stateSize);
  saveStringToFile(stateBuffer, stateFilePath.c_str());
 }

 void serializeState(uint8_t* state) const override
 {
  #ifdef _JAFFAR_PLAY
   state_save(state);
  #else
   state_save_light(state);
  #endif
 }

 void deserializeState(const uint8_t* state) override
 {
  #ifdef _JAFFAR_PLAY
   state_load(state);
  #else
   state_load_light(state);
  #endif
//  printf("size: %lu\n", size);
//  exit(0);
 }

 static INPUT_TYPE moveStringToCode(const std::string& move)
 {
  INPUT_TYPE moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'U': moveCode |= INPUT_UP; break;
    case 'D': moveCode |= INPUT_DOWN; break;
    case 'L': moveCode |= INPUT_LEFT; break;
    case 'R': moveCode |= INPUT_RIGHT; break;
    case 'A': moveCode |= INPUT_A; break;
    case 'B': moveCode |= INPUT_B; break;
    case 'C': moveCode |= INPUT_C; break;
    case 'S': moveCode |= INPUT_START; break;
    case 'X': moveCode |= INPUT_X; break;
    case 'Y': moveCode |= INPUT_Y; break;
    case 'Z': moveCode |= INPUT_Z; break;
    case 'M': moveCode |= INPUT_MODE; break;
    case '.': break;
    case '|': break;
    default: EXIT_WITH_ERROR("Move provided cannot be parsed: '%s', unrecognized character: '%c'\n", move.c_str(), move[i]);
  }

  return moveCode;
 }

 static std::string moveCodeToString(const INPUT_TYPE move)
 {
  std::string moveString;

  if (move & INPUT_UP) moveString += 'U'; else moveString += '.';
  if (move & INPUT_DOWN) moveString += 'D'; else moveString += '.';
  if (move & INPUT_LEFT) moveString += 'L'; else moveString += '.';
  if (move & INPUT_RIGHT) moveString += 'R'; else moveString += '.';
  if (move & INPUT_A) moveString += 'A'; else moveString += '.';
  if (move & INPUT_B) moveString += 'B'; else moveString += '.';
  if (move & INPUT_C) moveString += 'C'; else moveString += '.';
  if (move & INPUT_START) moveString += 'S'; else moveString += '.';
  if (move & INPUT_X) moveString += 'X'; else moveString += '.';
  if (move & INPUT_Y) moveString += 'Y'; else moveString += '.';
  if (move & INPUT_Z) moveString += 'Z'; else moveString += '.';
  if (move & INPUT_MODE) moveString += 'M'; else moveString += '.';

  return moveString;
 }

 void advanceState(const INPUT_TYPE move) override
 {
  jaffarInput = move;
  system_frame_gen(1);
 }

};
