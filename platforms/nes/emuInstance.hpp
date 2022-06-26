#pragma once

#include <emuInstanceBase.hpp>
#include <Nes_Emu.h>
#include <Nes_State.h>
#include <utils.hpp>
#include <string>
#include <vector>
#include <utils.hpp>
#include <state.hpp>

class EmuInstance : public EmuInstanceBase
{
 public:

 // Emulator instance
 Nes_Emu* _nes;

 // Base low-memory pointer
 uint8_t* _baseMem;
 uint8_t* _ppuNameTableMem;

 EmuInstance(const nlohmann::json& config) : EmuInstanceBase(config)
 {
  // Checking whether configuration contains the rom file
  if (isDefined(config, "Rom File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rom File' key.\n");
  std::string romFilePath = config["Rom File"].get<std::string>();

  // Checking whether configuration contains the state file
  if (isDefined(config, "State File") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State File' key.\n");
  std::string stateFilePath = config["State File"].get<std::string>();

  // Creating new emulator and loading rom
  auto emu = new Nes_Emu;
  std::string romData;
  loadStringFromFile(romData, romFilePath.c_str());
  Mem_File_Reader romReader(romData.data(), (int)romData.size());
  Auto_File_Reader romFile(romReader);
  auto result = emu->load_ines(romFile);
  if (result != 0) EXIT_WITH_ERROR("Could not initialize emulator with rom file: %s\n", romFilePath.c_str());

  // Setting emulator
  setEmulator(emu);

  // Loading state file, if specified
  if (stateFilePath != "") loadStateFile(stateFilePath);
 }

 void setEmulator(Nes_Emu* emulator)
 {
  _nes = emulator;

  // Setting base memory pointer
  _baseMem = _nes->low_mem();
  _ppuNameTableMem = _nes->nametable_mem();
 }

 void loadStateFile(const std::string& stateFilePath) override
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
  _nes->load_state(state);
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  std::string stateData;
  stateData.resize(_STATE_DATA_SIZE_TRAIN);
  serializeState((uint8_t*)stateData.data());
  saveStringToFile(stateData, stateFilePath.c_str());
 }

 void serializeState(uint8_t* state) const override
 {
  Mem_Writer w(state, _STATE_DATA_SIZE_TRAIN, 0);
  Auto_File_Writer a(w);
  _nes->save_state(a);
 }

 void deserializeState(const uint8_t* state) override
 {
  Mem_File_Reader r(state, _STATE_DATA_SIZE_TRAIN);
  Auto_File_Reader a(r);
  _nes->load_state(a);
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

 static uint8_t moveStringToCode(const std::string& move)
 {
  uint8_t moveCode = 0;

  for (size_t i = 0; i < move.size(); i++) switch(move[i])
  {
    case 'R': moveCode |= 0b10000000; break;
    case 'L': moveCode |= 0b01000000; break;
    case 'D': moveCode |= 0b00100000; break;
    case 'U': moveCode |= 0b00010000; break;
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

 static std::string moveCodeToString(const uint8_t move)
 {
  std::string moveString;

  if (move & 0b10000000) moveString += 'R'; else moveString += '.';
  if (move & 0b01000000) moveString += 'L'; else moveString += '.';
  if (move & 0b00100000) moveString += 'D'; else moveString += '.';
  if (move & 0b00010000) moveString += 'U'; else moveString += '.';
  if (move & 0b00001000) moveString += 'S'; else moveString += '.';
  if (move & 0b00000100) moveString += 's'; else moveString += '.';
  if (move & 0b00000010) moveString += 'B'; else moveString += '.';
  if (move & 0b00000001) moveString += 'A'; else moveString += '.';

  return moveString;
 }

 void advanceState(const std::string& move)
 {
  advanceState(moveStringToCode(move));
 }

 void advanceState(const INPUT_TYPE move) override
 {
  _nes->emulate_frame(move,0);
 }

};
