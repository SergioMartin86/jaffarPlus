#pragma once

#include <emuInstance.hpp>
#include "utils.hpp"

static size_t _maxStateDiff;

enum stateType
{
  f_regular,
  f_win,
  f_fail
};

class State
{
  private:

  // Positions of the difference with respect to a base state
  uint16_t stateDiffPositions[_MAX_STATE_DIFF];
  uint8_t stateDiffValues[_MAX_STATE_DIFF];

  // Positions of the difference with respect to a base state
  uint16_t stateDiffCount;

  public:

  // The score calculated for this state
  float reward;

  // Rule status vector
  bool rulesStatus[_MAX_RULE_COUNT];

  // Differentiation functions
  inline void computeStateDifference(const uint8_t* __restrict__ baseStateData, const uint8_t* __restrict__ newStateData)
  {
   stateDiffCount = 0;
   #pragma GCC unroll 32
   #pragma GCC ivdep
   for (uint16_t i = 0; i < _STATE_DATA_SIZE; i++) if (baseStateData[i] != newStateData[i])
   {
    stateDiffPositions[stateDiffCount] = i;
    stateDiffValues[stateDiffCount] = newStateData[i];
    stateDiffCount++;
   }

   if (stateDiffCount > _maxStateDiff)
   {
    _maxStateDiff = stateDiffCount;
     if (stateDiffCount > _MAX_STATE_DIFF) EXIT_WITH_ERROR("[Error] Exceeded maximum state difference: %d > %d. Increase this maximum in the state.hpp source file and rebuild.\n", stateDiffCount, _MAX_STATE_DIFF);
   }
  }

  inline void getStateDataFromDifference(const uint8_t* __restrict__ baseStateData, uint8_t* __restrict__ stateData) const
  {
    memcpy(stateData, baseStateData, _STATE_DATA_SIZE);
    #pragma GCC unroll 32
    #pragma GCC ivdep
    for (uint16_t i = 0; i < stateDiffCount; i++) stateData[stateDiffPositions[i]] = stateDiffValues[i];
  }

  #ifndef JAFFAR_DISABLE_MOVE_HISTORY

  // Stores the entire move history of the state
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
