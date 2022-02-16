#pragma once

#include <emuInstance.hpp>
#include "utils.hpp"

static uint16_t _maxDifferenceCount;
static uint16_t _maxMoveCount;
static uint16_t _ruleCount;
static bool _storeMoveList;

static size_t _stateFixedSize;
static size_t _stateVariableSize;
static size_t _stateSize;
static uint16_t _maxStateDiff;

enum stateType
{
  f_regular,
  f_win,
  f_fail
};

class State
{
  private:

  uint8_t* _basePointer;

  // Prevent direct copies
  State& operator=(const State&) = delete;

  public:

  // Positions of the difference with respect to a base state
  uint16_t* stateDiffPositions;
  uint8_t* stateDiffValues;
  uint16_t stateDiffCount;

  // Stores the entire move history of the state
  uint8_t* moveHistory;

  // The score calculated for this state
  float reward;

  // Rule status vector
  bool* rulesStatus;

  State()
  {
   // Creating new storage for variable statesize
   _basePointer = new uint8_t[_stateVariableSize];

   size_t curPos = 0;
   stateDiffPositions = (uint16_t*)&_basePointer[curPos];
   curPos += _maxDifferenceCount * sizeof(uint16_t);
   stateDiffValues = &_basePointer[curPos];
   curPos += _maxDifferenceCount * sizeof(uint8_t);

   if (_storeMoveList)
   {
    moveHistory = (uint8_t*)&_basePointer[curPos];
    curPos += _maxMoveCount * sizeof(uint8_t);
   }

   rulesStatus = (bool*)&_basePointer[curPos];
   curPos += _ruleCount * sizeof(bool);
  }

  void copy(const State* state)
  {
   memcpy(_basePointer, state->_basePointer, _stateVariableSize);
   reward = state->reward;
   stateDiffCount = state->stateDiffCount;
  }

  // Parsing configuration
  static void parseConfiguration(const nlohmann::json& config)
  {
   // Checking whether it contains the state configuration field
   if (isDefined(config, "State Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'State Configuration' key.\n");

   // Checking whether it contains the emulator configuration field
   if (isDefined(config["State Configuration"], "Max Difference Count") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Max Difference Count' key.\n");
   _maxDifferenceCount = config["State Configuration"]["Max Difference Count"].get<uint16_t>();

   if (isDefined(config["State Configuration"], "Max Move Count") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Max Move Count' key.\n");
   _maxMoveCount = config["State Configuration"]["Max Move Count"].get<uint16_t>();

   if (isDefined(config["State Configuration"], "Store Move List") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Store Move List' key.\n");
   _storeMoveList = config["State Configuration"]["Store Move List"].get<bool>();

   // Calculating fixed size of a state
   _stateFixedSize = sizeof(State);

   // Calculating variable size of a state
   _stateVariableSize = 0;
   _stateVariableSize += _maxDifferenceCount * sizeof(uint16_t);
   _stateVariableSize += _maxDifferenceCount * sizeof(uint8_t);
   if (_storeMoveList) _stateVariableSize += _maxMoveCount * sizeof(uint8_t);
   _stateVariableSize += _ruleCount * sizeof(bool);

   // Calculating state size(s)
   _stateSize = _stateFixedSize + _stateVariableSize;
  }

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
     if (stateDiffCount > _maxDifferenceCount) EXIT_WITH_ERROR("[Error] Exceeded maximum state difference: %u > %u. Increase 'Max Difference Count' in the Jaffar script file..\n", stateDiffCount, _maxDifferenceCount);
   }
  }

  inline void getStateDataFromDifference(const uint8_t* __restrict__ baseStateData, uint8_t* __restrict__ stateData) const
  {
    memcpy(stateData, baseStateData, _STATE_DATA_SIZE);
    #pragma GCC unroll 32
    #pragma GCC ivdep
    for (uint16_t i = 0; i < stateDiffCount; i++) stateData[stateDiffPositions[i]] = stateDiffValues[i];
  }

  inline void setMoveHistory(const uint8_t* sourceMoveHistory)
  {
   memcpy(moveHistory, sourceMoveHistory, sizeof(uint8_t) * _maxMoveCount);
  }

  // Move r/w operations
  inline void setMove(const size_t idx, const uint8_t move)
  {
    moveHistory[idx] = move;
  }

  inline uint8_t getMove(const size_t idx) const
  {
   return moveHistory[idx];
  }
};
