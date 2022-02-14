#pragma once

#include <emuInstanceBase.hpp>
#include <Nes_Emu.h>
#include <Nes_State.h>
#include <utils.hpp>
#include <string>
#include <vector>

// W1-1a
//#define _MAX_FRAME_DIFF 700
//#define _MAX_RULE_COUNT 5
//#define _MAX_MOVELIST_SIZE 440

// W1-1b
//#define _MAX_FRAME_DIFF 300
//#define _MAX_RULE_COUNT 8
//#define _MAX_MOVELIST_SIZE 150

// W1-1c
//#define _MAX_FRAME_DIFF 650
//#define _MAX_RULE_COUNT 7
//#define _MAX_MOVELIST_SIZE 300

// W1-2a
//#define _MAX_FRAME_DIFF 900
//#define _MAX_RULE_COUNT 7
//#define _MAX_MOVELIST_SIZE 1350

// W1-2b
//#define _MAX_FRAME_DIFF 700
//#define _MAX_RULE_COUNT 10
//#define _MAX_MOVELIST_SIZE 450

// W4-1a
//#define _MAX_FRAME_DIFF 900
//#define _MAX_RULE_COUNT 5
//#define _MAX_MOVELIST_SIZE 1500

// W4-2a
//#define _MAX_FRAME_DIFF 800
//#define _MAX_RULE_COUNT 15
//#define _MAX_MOVELIST_SIZE 700

// W4-2b
//#define _MAX_FRAME_DIFF 550
//#define _MAX_RULE_COUNT 10
//#define _MAX_MOVELIST_SIZE 500

// W8-1
//#define _MAX_FRAME_DIFF 900
//#define _MAX_RULE_COUNT 10
//#define _MAX_MOVELIST_SIZE 2500

// W8-2
//#define _MAX_FRAME_DIFF 1050
//#define _MAX_RULE_COUNT 10
//#define _MAX_MOVELIST_SIZE 1700

// W8-3
//#define _MAX_FRAME_DIFF 850
//#define _MAX_RULE_COUNT 10
//#define _MAX_MOVELIST_SIZE 1500

// W8-4a
//#define _MAX_FRAME_DIFF 650
//#define _MAX_RULE_COUNT 8
//#define _MAX_MOVELIST_SIZE 550

// W8-4b
//#define _MAX_FRAME_DIFF 650
//#define _MAX_RULE_COUNT 8
//#define _MAX_MOVELIST_SIZE 300

// W8-4c
#define _MAX_FRAME_DIFF 750
#define _MAX_RULE_COUNT 8
#define _MAX_MOVELIST_SIZE 200

// W8-4d
//#define _MAX_FRAME_DIFF 520
//#define _MAX_RULE_COUNT 8
//#define _MAX_MOVELIST_SIZE 700

// W8-4e
//#define _MAX_FRAME_DIFF 600
//#define _MAX_RULE_COUNT 8
//#define _MAX_MOVELIST_SIZE 300

#define _FRAME_DATA_SIZE 12792
const std::vector<std::string> _possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "LAB", "RAB", "LRAB" };

class EmuInstance : public EmuInstanceBase
{
 public:

 // Emulator instance
 Nes_Emu* _emu;

 // Base low-memory pointer
 uint8_t* _baseMem;

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

  // Loading state file
  loadStateFile(stateFilePath);
 }

 void setEmulator(Nes_Emu* emulator)
 {
  _emu = emulator;

  // Setting base memory pointer
  _baseMem = _emu->low_mem();
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
  _emu->load_state(state);
 }

 void saveStateFile(const std::string& stateFilePath) const override
 {
  // Saving state
  Nes_State state;
  _emu->save_state(&state);
  Auto_File_Writer stateWriter(stateFilePath.c_str());
  state.write(stateWriter);
 }

 void serializeState(uint8_t* state) const override
 {
  Mem_Writer w(state, _FRAME_DATA_SIZE, 0);
  Auto_File_Writer a(w);
  _emu->save_state(a);
 }

 void deserializeState(const uint8_t* state) override
 {
  Mem_File_Reader r(state, _FRAME_DATA_SIZE);
  Auto_File_Reader a(r);
  _emu->load_state(a);
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
 // Possible moves
 // Move Ids =        0    1    2    3    4    5     6     7     8    9     10    11      12     13    14      15
 //_possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "LAB", "RAB", "LRAB" };

 void advanceFrame(const std::string& move) override
 {
  if (move == ".") { advanceFrame(0); return; }
  if (move == "L") { advanceFrame(1); return; }
  if (move == "R") { advanceFrame(2); return; }
  if (move == "D") { advanceFrame(3); return; }
  if (move == "A") { advanceFrame(4); return; }
  if (move == "B") { advanceFrame(5); return; }
  if (move == "LA") { advanceFrame(6); return; }
  if (move == "RA") { advanceFrame(7); return; }
  if (move == "LB") { advanceFrame(8); return; }
  if (move == "RB") { advanceFrame(9); return; }
  if (move == "LR") { advanceFrame(10); return; }
  if (move == "LRA") { advanceFrame(11); return; }
  if (move == "LRB") { advanceFrame(12); return; }
  if (move == "LAB") { advanceFrame(13); return; }
  if (move == "RAB") { advanceFrame(14); return; }
  if (move == "LRAB") { advanceFrame(15); return; }

  EXIT_WITH_ERROR("Unrecognized move: %s\n", move.c_str());
 }

 void advanceFrame(const uint8_t &move) override
 {
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
   case 13: controllerCode = 0b01000011; break; // LAB
   case 14: controllerCode = 0b10000011; break; // RAB
   case 15: controllerCode = 0b11000011; break; // LRAB
   default: EXIT_WITH_ERROR("Wrong move code passed %u\n", move);
  }

  // Running frame
  _emu->emulate_frame(controllerCode,0);
 }

};
