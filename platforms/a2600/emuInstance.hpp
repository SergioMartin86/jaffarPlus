#pragma once

#include <emuInstanceBase.hpp>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>
#include "OSystem.hxx"
#include "Settings.hxx"
#include "MediaFactory.hxx"
#include "Serializer.hxx"
#include "StateManager.hxx"
#include "Console.hxx"
#include "Switches.hxx"
#include "M6532.hxx"
#include "TIA.hxx"

class EmuInstance : public EmuInstanceBase
{
 public:

 std::string _romData;
 std::unique_ptr<OSystem> _a2600;
 uint8_t* _ram;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  bool enableTIA = true;
#ifndef _JAFFAR_PLAY
  // Checking whether configuration contains the state file
  if (isDefined(config, "Enable TIA") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Enable TIA' key.\n");
  enableTIA = config["Enable TIA"].get<bool>();
#endif

  Settings::Options opts;
  _a2600 = MediaFactory::createOSystem();
  if(!_a2600->initialize(opts)) EXIT_WITH_ERROR("ERROR: Couldn't create A2600 System");
  const string romfile = romFilePath;
  const FSNode romnode(romfile);

  _a2600->createConsole(romnode);
  _ram = _a2600->console().riot().getRAM();

//  Serializer state;
//  _a2600->state().saveState(state);
//  size_t stateSize = state.size();
//  printf("Size: %lu\n", stateSize);
//    saveStateFile("boot.state");
//    exit(0);

  // Loading state file, if specified
  if (stateFilePath != "") loadStateFile(stateFilePath);

  _a2600->console().tia()._isTiaEnabled = enableTIA;
 }

 void loadStateFile(const std::string& stateFilePath) override
 {
  Serializer state(stateFilePath);
  _a2600->state().loadState(state);
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  Serializer state(stateFilePath);
  _a2600->state().saveState(state);
 }

 void serializeState(uint8_t* state) const override
 {
  Serializer gameState;
  _a2600->state().saveState(gameState);

#ifdef _JAFFAR_PLAY
  gameState.getByteArray(state, _STATE_DATA_SIZE_PLAY);
#else
  gameState.getByteArray(state, _STATE_DATA_SIZE_TRAIN);
#endif
 }

 void deserializeState(const uint8_t* state) override
 {
  Serializer gameState;

#ifdef _JAFFAR_PLAY
  gameState.putByteArray(state, _STATE_DATA_SIZE_PLAY);
#else
  gameState.putByteArray(state, _STATE_DATA_SIZE_TRAIN);
#endif

  _a2600->state().loadState(gameState);
 }

 // Controller input bits
 // 0 - Up / 1
 // 1 - Down / 2
 // 2 - Left / 4
 // 3 - Right / 8
 // 4 - -- / 16
 // 5 - B / 32
 // 6 - l / 64
 // 7 - r / 128

 // Move Format:
 // rlB-RLDU
 // ........

 static inline INPUT_TYPE moveStringToCode(const std::string& move)
 {
  INPUT_TYPE moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'U': moveCode |= 0b00000001; break;
    case 'D': moveCode |= 0b00000010; break;
    case 'L': moveCode |= 0b00000100; break;
    case 'R': moveCode |= 0b00001000; break;
    case 'B': moveCode |= 0b00100000; break;
    case 'r': moveCode |= 0b10000000; break;
    case 'l': moveCode |= 0b01000000; break;
    case '.': break;
    case '|': break;
    default: EXIT_WITH_ERROR("Move provided cannot be parsed: '%s', unrecognized character: '%c'\n", move.c_str(), move[i]);
  }

  return moveCode;
 }

 static inline std::string moveCodeToString(const INPUT_TYPE move)
 {

  std::string moveString = "|";

  if (move & 0b10000000) moveString += 'r'; else moveString += '.';
  if (move & 0b01000000) moveString += 'l'; else moveString += '.';

  moveString += "....|";

  if (move & 0b00000001) moveString += 'U'; else moveString += '.';
  if (move & 0b00000010) moveString += 'D'; else moveString += '.';
  if (move & 0b00000100) moveString += 'L'; else moveString += '.';
  if (move & 0b00001000) moveString += 'R'; else moveString += '.';
  if (move & 0b00100000) moveString += 'B'; else moveString += '.';

  moveString += "|";
  return moveString;
 }


 void advanceState(const std::string& move)
 {
  advanceState(moveStringToCode(move));
 }

 void advanceState(const INPUT_TYPE move) override
 {
  if (move & 0b00000001) _a2600->console().leftController().write(Controller::DigitalPin::One,   false); else _a2600->console().leftController().write(Controller::DigitalPin::One,   true);
  if (move & 0b00000010) _a2600->console().leftController().write(Controller::DigitalPin::Two,   false); else _a2600->console().leftController().write(Controller::DigitalPin::Two,   true);
  if (move & 0b00000100) _a2600->console().leftController().write(Controller::DigitalPin::Three, false); else _a2600->console().leftController().write(Controller::DigitalPin::Three, true);
  if (move & 0b00001000) _a2600->console().leftController().write(Controller::DigitalPin::Four,  false); else _a2600->console().leftController().write(Controller::DigitalPin::Four,  true);
  if (move & 0b00100000) _a2600->console().leftController().write(Controller::DigitalPin::Six,   false); else _a2600->console().leftController().write(Controller::DigitalPin::Six,   true);
  if (move & 0b10000000) _a2600->console().switches().values() &= ~0x01; else _a2600->console().switches().values() |= 0x01;
  if (move & 0b01000000) _a2600->console().switches().values() &= ~0x40; else _a2600->console().switches().values() |= 0x40;

  _a2600->advanceFrame();
 }

};
