#pragma once

#include <Nes_Emu.h>
#include <Nes_State.h>
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

class gameInstance
{
  public:

  // Initializes the miniPop instance
  gameInstance(const std::string& romFilePath);
  gameInstance(Nes_Emu* emulator);

  // Loading/Saving state files
  void loadStateFile(const std::string& stateFilePath);
  void saveStateFile(const std::string& stateFilePath);

  // Function to advance frame
  void advanceFrame(const uint8_t &move);
  void advanceFrame(const std::string& move);

  // Minimal serialization functions
  void serializeState(uint8_t* state) const;
  void deserializeState(const uint8_t* state);

  // Emulator instance
  Nes_Emu* _emu;

  // Base low-memory pointer
  uint8_t* _baseMem;
};
