#pragma once

#ifndef _MAX_RULE_COUNT
 #define _MAX_RULE_COUNT 30
#endif

#ifndef _MAX_MOVELIST_SIZE
 #define _MAX_MOVELIST_SIZE 510
#endif

#define _MAX_MOVELIST_STORAGE ((_MAX_MOVELIST_SIZE/2) + 1)
#define _FRAME_DATA_SIZE 3148

#include "nlohmann/json.hpp"
#include "rule.h"
#include <string>
#include <vector>

const std::vector<std::string> _possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "S" };

enum frameType
{
  f_regular,
  f_win,
  f_fail
};

class Frame
{
  public:
  Frame();

  // Fixed state data
  uint8_t stateData[_FRAME_DATA_SIZE];

  // Rule status vector
  bool rulesStatus[_MAX_RULE_COUNT];

  // Differentiation functions
  inline void setStateData(const uint8_t* __restrict__ newStateData)
  {
    memcpy(stateData, newStateData, _FRAME_DATA_SIZE);
  }

  inline void getStateData(uint8_t* __restrict__ curStateData) const
  {
    memcpy(curStateData, stateData, _FRAME_DATA_SIZE);
  }

#ifndef JAFFAR_DISABLE_MOVE_HISTORY

  // Stores the entire move history of the frame
  char moveHistory[_MAX_MOVELIST_STORAGE];

  // Move r/w operations
  inline void setMove(const size_t idx, const uint8_t move)
  {
    size_t basePos = idx / 2;
    uint8_t baseVal = moveHistory[basePos];
    uint8_t newVal;

    if (idx % 2 == 0) newVal = (baseVal & 0xF0) | (move & 0x0F);
    if (idx % 2 == 1) newVal = (baseVal & 0x0F) | (move << 4);

    moveHistory[basePos] = newVal;
  }

  inline uint8_t getMove(const size_t idx)
  {
   size_t basePos = idx / 2;
   uint8_t val = moveHistory[basePos];
   if (idx % 2 == 0) val = val & 0x0F;
   if (idx % 2 == 1) val = val >> 4;
   return val;
  }

#endif

  // The score calculated for this frame
  float reward;

  // Positions of the difference with respect to a base frame
  uint16_t frameDiffCount;
};
