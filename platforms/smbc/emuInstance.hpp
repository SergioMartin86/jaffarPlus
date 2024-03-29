#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>
#include <SMB.hpp>
#include "Emulation/Controller.hpp"

class EmuInstance : public EmuInstanceBase
{
 public:

 std::string _romData;
 SMBEngine* _nes;

 // Emulator instance
 //TBD

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  //  loading rom
  if (loadStringFromFile(_romData, romFilePath.c_str()) != true) EXIT_WITH_ERROR("[ERROR] Could not load rom file.\n");
  romImage = (uint8_t*)_romData.data();

  _nes = new SMBEngine(romImage);
  _nes->reset();
  _nes->update();


//  size_t stateSize = _nes->getStateSizeFast();
//  printf("Size: %lu\n", stateSize);
//  std::string bootState;
//  bootState.resize(stateSize);
//  _nes->saveState((uint8_t*) bootState.data());
//  saveStringToFile(bootState, "boot.state");
//  exit(0);

  // Loading state file, if specified
  if (stateFilePath != "") loadStateFile(stateFilePath);
 }

 void loadStateFile(const std::string& stateFilePath) override
 {
  std::string stateData;
  if (loadStringFromFile(stateData, stateFilePath.c_str()) == false) EXIT_WITH_ERROR("[ERROR] Could not state file.\n"); ;
  _nes->loadState((uint8_t*) stateData.data());
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  std::string stateData;
  stateData.resize(_nes->getStateSize());
  serializeState((uint8_t*)stateData.data());
  saveStringToFile(stateData, stateFilePath.c_str());
 }

 void serializeState(uint8_t* state) const override
 {
  #ifdef _JAFFAR_PLAY
   _nes->saveState(state);
  #else
   _nes->saveStateFast(state);
  #endif
 }

 void deserializeState(const uint8_t* state) override
 {
  #ifdef _JAFFAR_PLAY
   _nes->loadState(state);
  #else
   _nes->loadStateFast(state);
 #endif
 }

 // Controller input bits
 // 0 - A / 1
 // 1 - B / 2
 // 2 - Select / 4
 // 3 - Start / 8
 // 4 - Up / 16
 // 5 - Down / 32
 // 6 - Left / 64
 // 7 - Right / 128

 // Move Format:
 // RLDUTSBA
 // ........

 static inline INPUT_TYPE moveStringToCode(const std::string& move)
 {
  INPUT_TYPE moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'U': moveCode |= 0b00010000; break;
    case 'D': moveCode |= 0b00100000; break;
    case 'L': moveCode |= 0b01000000; break;
    case 'R': moveCode |= 0b10000000; break;
    case 'S': moveCode |= 0b00001000; break;
    case 's': moveCode |= 0b00000100; break;
    case 'B': moveCode |= 0b00000010; break;
    case 'A': moveCode |= 0b00000001; break;
    case '.': break;
    case '|': break;
    default: EXIT_WITH_ERROR("Move provided cannot be parsed: '%s', unrecognized character: '%c'\n", move.c_str(), move[i]);
  }

  return moveCode;
 }

 static inline std::string moveCodeToString(const INPUT_TYPE move)
 {
  std::string moveString = "|..|";

  if (move & 0b00010000) moveString += 'U'; else moveString += '.';
  if (move & 0b00100000) moveString += 'D'; else moveString += '.';
  if (move & 0b01000000) moveString += 'L'; else moveString += '.';
  if (move & 0b10000000) moveString += 'R'; else moveString += '.';
  if (move & 0b00001000) moveString += 'S'; else moveString += '.';
  if (move & 0b00000100) moveString += 's'; else moveString += '.';
  if (move & 0b00000010) moveString += 'B'; else moveString += '.';
  if (move & 0b00000001) moveString += 'A'; else moveString += '.';

  moveString += "|";
  return moveString;
 }


 void advanceState(const std::string& move)
 {
  advanceState(moveStringToCode(move));
 }

 void advanceState(const INPUT_TYPE move) override
 {
  if (move & 0b10000000) _nes->controller1->setButtonState(BUTTON_RIGHT, true); else _nes->controller1->setButtonState(BUTTON_RIGHT, false);
  if (move & 0b01000000) _nes->controller1->setButtonState(BUTTON_LEFT, true); else _nes->controller1->setButtonState(BUTTON_LEFT, false);
  if (move & 0b00100000) _nes->controller1->setButtonState(BUTTON_DOWN, true); else _nes->controller1->setButtonState(BUTTON_DOWN, false);
  if (move & 0b00010000) _nes->controller1->setButtonState(BUTTON_UP, true); else _nes->controller1->setButtonState(BUTTON_UP, false);
  if (move & 0b00001000) _nes->controller1->setButtonState(BUTTON_START, true); else _nes->controller1->setButtonState(BUTTON_START, false);
  if (move & 0b00000100) _nes->controller1->setButtonState(BUTTON_SELECT, true); else _nes->controller1->setButtonState(BUTTON_SELECT, false);
  if (move & 0b00000010) _nes->controller1->setButtonState(BUTTON_B, true); else _nes->controller1->setButtonState(BUTTON_B, false);
  if (move & 0b00000001) _nes->controller1->setButtonState(BUTTON_A, true); else _nes->controller1->setButtonState(BUTTON_A, false);

  _nes->update();
 }

};
