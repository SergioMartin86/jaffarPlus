#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <utils.hpp>
#include <stdio.h>
#include <sys/stat.h>
#include "xdelta3.h"

#ifndef _DISABLE_XDELTA3
 #define _MAX_DIFFERENCE_COUNT 330
static size_t _maxStateDiff;
#endif

enum stateType
{
  f_regular,
  f_win,
  f_fail
};

static size_t _ruleCount;
static size_t _stateSize;
static size_t _moveHistorySize;
static size_t _moveHistoryOffset;
static size_t _frameDataSize;
static size_t _frameDataOffset;
static size_t _ruleStatusSize;
static size_t _ruleStatusOffset;

static bool _storeMoveHistory;
static uint16_t _maxMoveCount;

class State
{
  public:

 static size_t getSize()
 {
  size_t offset = sizeof(State);

  _moveHistoryOffset = offset;
  _moveHistorySize = sizeof(INPUT_TYPE) * _maxMoveCount;
  offset += _moveHistorySize;

  _frameDataOffset = offset;
  _frameDataSize = _STATE_DATA_SIZE_TRAIN;
  offset += _frameDataSize;

  _ruleStatusOffset = offset;
  _ruleStatusSize = sizeof(bool) * _ruleCount;
  offset += _ruleStatusSize;

  return offset;
 }

 inline void copy(const State* src) { memcpy(this, src, _stateSize); }
 inline bool* getRuleStatus() { return (bool*)((uint8_t*)this + _ruleStatusOffset); }

#ifndef _DISABLE_XDELTA3

  // Positions of the difference with respect to a base frame
  uint8_t diffOutput[_MAX_DIFFERENCE_COUNT];
  usize_t diffSize;

  inline void computeStateDifference(const uint8_t* __restrict__ baseStateData, const uint8_t* __restrict__ newStateData)
  {
   int ret = xd3_encode_memory(newStateData, _STATE_DATA_SIZE_TRAIN, baseStateData, _STATE_DATA_SIZE_TRAIN, diffOutput, &diffSize, sizeof(diffOutput), XD3_NOCOMPRESS);
   if (diffSize > _MAX_DIFFERENCE_COUNT) EXIT_WITH_ERROR("[Error] Exceeded maximum frame difference: %d > %d. Increase this maximum in the state.hpp source file and rebuild.\n", diffSize, _MAX_DIFFERENCE_COUNT);
   if (ret != 0) EXIT_WITH_ERROR("[Error] State Encode failure: %d: %s\n", ret, xd3_strerror(ret));
   if (diffSize > _maxStateDiff) _maxStateDiff = diffSize;
  }

  inline void getStateDataFromDifference(const uint8_t* __restrict__ baseStateData, uint8_t* __restrict__ stateData) const
  {
   usize_t output_size;
   int ret = xd3_decode_memory (diffOutput, diffSize, baseStateData, _STATE_DATA_SIZE_TRAIN, stateData, &output_size, _STATE_DATA_SIZE_TRAIN, XD3_NOCOMPRESS);
   if (ret != 0) EXIT_WITH_ERROR("[Error] State Decode failure: %d: %s\n", ret, xd3_strerror(ret));
  }

#else

  inline void computeStateDifference(const uint8_t* __restrict__ baseStateData, const uint8_t* __restrict__ newStateData)
  {
   memcpy((uint8_t*)this + _frameDataOffset, newStateData, _frameDataSize);
  }

  inline void getStateDataFromDifference(const uint8_t* __restrict__ baseStateData, uint8_t* __restrict__ stateData) const
  {
   memcpy(stateData, (uint8_t*)this + _frameDataOffset, _frameDataSize);
  }

#endif

  // Parsing configuration
  static void parseConfiguration(const nlohmann::json& config)
  {
   if (isDefined(config, "Max Move Count") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Max Move Count' key.\n");
   _maxMoveCount = config["Max Move Count"].get<uint16_t>();

#ifndef _DISABLE_XDELTA3

   // Checking whether it contains the emulator configuration field
   if (isDefined(config, "Max Difference Count") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Max Difference Count' key.\n");
   uint16_t maxDifferenceCount = config["Max Difference Count"].get<uint16_t>();


   if (maxDifferenceCount != (uint16_t)_MAX_DIFFERENCE_COUNT) EXIT_WITH_ERROR("[Error] Configured 'Max Difference Count' (%u) does not concide with _MAX_DIFFERENCE_COUNT (%u) in state.hpp source file. Adjust the value in either place and rebuild.\n", maxDifferenceCount, (uint16_t)_MAX_DIFFERENCE_COUNT);
#endif
  }

  // Move r/w operations
  inline void setMove(const size_t idx, const uint8_t move)
  {
    *((INPUT_TYPE*)((uint8_t*)this + _moveHistoryOffset + idx)) = move;
  }

  inline uint8_t getMove(const size_t idx) const
  {
   return *((INPUT_TYPE*)((uint8_t*)this + _moveHistoryOffset + idx));
  }

  // The score calculated for this frame
  float reward;
};
