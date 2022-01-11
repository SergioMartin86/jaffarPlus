#pragma once

#ifndef _MAX_FRAME_DIFF
 #define _MAX_FRAME_DIFF 0
#endif

#ifndef _MAX_RULE_COUNT
 #define _MAX_RULE_COUNT 30
#endif

#ifndef _MAX_MOVELIST_SIZE
 #define _MAX_MOVELIST_SIZE 510
#endif

#define _MAX_MOVELIST_STORAGE ((_MAX_MOVELIST_SIZE/2) + 1)
#define _FRAME_DIFFERENTIAL_SIZE 0
#define _FRAME_FIXED_SIZE 3148
#define _FRAME_DATA_SIZE (_FRAME_DIFFERENTIAL_SIZE + _FRAME_FIXED_SIZE)

#include "nlohmann/json.hpp"
#include "rule.h"
#include <string>
#include <vector>

const std::vector<std::string> _possibleMoves = {".", "S", "U", "L", "R", "D", "LU", "LD", "RU", "RD", "SR", "SL", "SU", "SD", "CA"};

enum frameType
{
  f_regular,
  f_win,
  f_fail
};

extern size_t _maxFrameDiff;

class Frame
{
  public:
  Frame();

  // Positions of the difference with respect to a base frame
  uint16_t frameDiffPositions[_MAX_FRAME_DIFF];
  uint8_t frameDiffValues[_MAX_FRAME_DIFF];

  // Fixed state data
  uint8_t fixedStateData[_FRAME_FIXED_SIZE];

  // Rule status vector
  bool rulesStatus[_MAX_RULE_COUNT];

  // Differentiation functions
  inline void computeFrameDifference(const uint8_t* __restrict__ baseFrameData, const uint8_t* __restrict__ newFrameData)
  {
   frameDiffCount = 0;
   #pragma GCC unroll 32
   #pragma GCC ivdep
   for (uint16_t i = 0; i < _FRAME_DIFFERENTIAL_SIZE; i++) if (baseFrameData[i] != newFrameData[i])
   {
    frameDiffPositions[frameDiffCount] = i;
    frameDiffValues[frameDiffCount] = (uint8_t)newFrameData[i];
    frameDiffCount++;
   }

   if (frameDiffCount > _maxFrameDiff)
   {
    _maxFrameDiff = frameDiffCount;
     if (frameDiffCount > _MAX_FRAME_DIFF) EXIT_WITH_ERROR("[Error] Exceeded maximum frame difference: %d > %d. Increase this maximum in the frame.h source file and rebuild.\n", frameDiffCount, _MAX_FRAME_DIFF);
   }
   memcpy(fixedStateData, &newFrameData[_FRAME_DIFFERENTIAL_SIZE], _FRAME_FIXED_SIZE);
  }

  inline void getFrameDataFromDifference(const uint8_t* __restrict__ baseFrameData, uint8_t* __restrict__ stateData) const
  {
    memcpy(stateData, baseFrameData, _FRAME_DIFFERENTIAL_SIZE);
    #pragma GCC unroll 32
    #pragma GCC ivdep
    for (uint16_t i = 0; i < frameDiffCount; i++) stateData[frameDiffPositions[i]] = frameDiffValues[i];
    memcpy(&stateData[_FRAME_DIFFERENTIAL_SIZE], fixedStateData, _FRAME_FIXED_SIZE);
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
