#pragma once

#include <emuInstance.hpp>
#include "utils.hpp"

static size_t _maxFrameDiff;

enum frameType
{
  f_regular,
  f_win,
  f_fail
};

class Frame
{
  private:

  // Positions of the difference with respect to a base frame
  uint16_t frameDiffPositions[_MAX_FRAME_DIFF];
  uint8_t frameDiffValues[_MAX_FRAME_DIFF];

  // Positions of the difference with respect to a base frame
  uint16_t frameDiffCount;

  public:

  // The score calculated for this frame
  float reward;

  // Rule status vector
  bool rulesStatus[_MAX_RULE_COUNT];

  // Differentiation functions
  inline void computeFrameDifference(const uint8_t* __restrict__ baseFrameData, const uint8_t* __restrict__ newFrameData)
  {
   frameDiffCount = 0;
   #pragma GCC unroll 32
   #pragma GCC ivdep
   for (uint16_t i = 0; i < _FRAME_DATA_SIZE; i++) if (baseFrameData[i] != newFrameData[i])
   {
    frameDiffPositions[frameDiffCount] = i;
    frameDiffValues[frameDiffCount] = newFrameData[i];
    frameDiffCount++;
   }

   if (frameDiffCount > _maxFrameDiff)
   {
    _maxFrameDiff = frameDiffCount;
     if (frameDiffCount > _MAX_FRAME_DIFF) EXIT_WITH_ERROR("[Error] Exceeded maximum frame difference: %d > %d. Increase this maximum in the frame.hpp source file and rebuild.\n", frameDiffCount, _MAX_FRAME_DIFF);
   }
  }

  inline void getFrameDataFromDifference(const uint8_t* __restrict__ baseFrameData, uint8_t* __restrict__ stateData) const
  {
    memcpy(stateData, baseFrameData, _FRAME_DATA_SIZE);
    #pragma GCC unroll 32
    #pragma GCC ivdep
    for (uint16_t i = 0; i < frameDiffCount; i++) stateData[frameDiffPositions[i]] = frameDiffValues[i];
  }

  #ifndef JAFFAR_DISABLE_MOVE_HISTORY

  // Stores the entire move history of the frame
  uint16_t moveHistory[_MAX_MOVELIST_SIZE];

  // Move r/w operations
  inline void setMove(const size_t idx, const uint16_t move)
  {
    moveHistory[idx] = move;
  }

  inline uint16_t getMove(const size_t idx) const
  {
   return moveHistory[idx];
  }

  #endif
};
