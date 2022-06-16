#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <utils.hpp>
#include <stdio.h>
#include <sys/stat.h>
#include "xdelta3.h"

#define _MAX_DIFFERENCE_COUNT 1500
#define _MAX_MOVELIST_SIZE 500
//#define JAFFAR_DISABLE_MOVE_HISTORY

enum stateType
{
  f_regular,
  f_win,
  f_fail
};

static size_t _maxStateDiff;
static size_t _ruleCount;

class State
{
  public:

  // Positions of the difference with respect to a base frame
  uint8_t diffOutput[_MAX_DIFFERENCE_COUNT];
  usize_t diffSize;

  // Rule status vector
  bool rulesStatus[_MAX_RULE_COUNT];

  // Differentiation functions
  inline void computeStateDifference(const uint8_t* __restrict__ baseStateData, const uint8_t* __restrict__ newStateData)
  {
   int ret = xd3_encode_memory(newStateData, _STATE_DATA_SIZE, baseStateData, _STATE_DATA_SIZE, diffOutput, &diffSize, sizeof(diffOutput), 0);
   if (diffSize > _MAX_DIFFERENCE_COUNT) EXIT_WITH_ERROR("[Error] Exceeded maximum frame difference: %d > %d. Increase this maximum in the state.hpp source file and rebuild.\n", diffSize, _MAX_DIFFERENCE_COUNT);
   if (ret != 0) EXIT_WITH_ERROR("[Error] State Encode failure: %d: %s\n", ret, xd3_strerror(ret));
   if (diffSize > _maxStateDiff) _maxStateDiff = diffSize;
  }

  inline void getStateDataFromDifference(const uint8_t* __restrict__ baseStateData, uint8_t* __restrict__ stateData) const
  {
   usize_t output_size;
   int ret = xd3_decode_memory (diffOutput, diffSize, baseStateData, _STATE_DATA_SIZE, stateData, &output_size, _STATE_DATA_SIZE, 0);
   if (ret != 0) EXIT_WITH_ERROR("[Error] State Decode failure: %d: %s\n", ret, xd3_strerror(ret));
  }

  // Parsing configuration
  static void parseConfiguration(const nlohmann::json& config)
  {
   // Checking whether it contains the state configuration field
   if (isDefined(config, "State Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State Configuration' key.\n");

   // Checking whether it contains the emulator configuration field
   if (isDefined(config["State Configuration"], "Max Difference Count") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Max Difference Count' key.\n");
   uint16_t maxDifferenceCount = config["State Configuration"]["Max Difference Count"].get<uint16_t>();

   if (isDefined(config["State Configuration"], "Max Move Count") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Max Move Count' key.\n");
   uint16_t maxMoveCount = config["State Configuration"]["Max Move Count"].get<uint16_t>();

   if (maxDifferenceCount != (uint16_t)_MAX_DIFFERENCE_COUNT) EXIT_WITH_ERROR("[Error] Configured 'Max Difference Count' (%u) does not concide with _MAX_DIFFERENCE_COUNT (%u) in state.hpp source file. Adjust the value in either place and rebuild.\n", maxDifferenceCount, (uint16_t)_MAX_DIFFERENCE_COUNT);
   if (_ruleCount > (uint16_t)_MAX_RULE_COUNT) EXIT_WITH_ERROR("[Error] Jaffar script contains %u rules, which is more than what the emulator with _MAX_RULE_COUNT (%u) was configured. Adjust the value in either place and rebuild.\n", _ruleCount, (uint16_t)_MAX_RULE_COUNT);
   if (maxMoveCount != (uint16_t)_MAX_MOVELIST_SIZE) EXIT_WITH_ERROR("[Error] Configured 'Max Move Count' (%u) does not concide with _MAX_MOVELIST_SIZE (%u) in state.hpp source file. Adjust the value in either place and rebuild.\n", maxMoveCount, (uint16_t)_MAX_MOVELIST_SIZE);

  }

#ifndef JAFFAR_DISABLE_MOVE_HISTORY

  // Stores the entire move history of the frame
  INPUT_TYPE moveHistory[_MAX_MOVELIST_SIZE];

  // Move r/w operations
  inline void setMove(const size_t idx, const INPUT_TYPE move)
  {
    moveHistory[idx] = move;
  }

  inline INPUT_TYPE getMove(const size_t idx) const
  {
   return moveHistory[idx];
  }

#endif

  // The score calculated for this frame
  float reward;

  // Positions of the difference with respect to a base frame
  uint16_t frameDiffCount;
};
