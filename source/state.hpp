#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <utils.hpp>
#include <stdio.h>
#include <sys/stat.h>
#include "xdelta3.h"

enum stateType_t
{
  f_regular,
  f_win,
  f_fail,
  f_checkpoint
};

static size_t _ruleCount;
static size_t _stateSize;
static size_t _moveHistorySize;
static size_t _moveHistoryOffset;
static size_t _frameDataSize;
static size_t _frameDataOffset;
static size_t _ruleStatusSize;
static size_t _ruleStatusOffset;
static size_t _xdelta3DiffCountSize;
static size_t _xdelta3DiffCountOffset;

static bool _storeMoveHistory;
static uint16_t _maxMoveCount;
static bool _useXDelta3;
static uint16_t _maxXDelta3Differences;
static bool _useXdelta3ZLibCompression;
static size_t _currentXDelta3DMaxDiff;

class State
{
  public:

  // The score calculated for this frame
  float reward;

  static size_t getSize()
  {
   size_t offset = sizeof(State);

   _moveHistoryOffset = offset;
   _moveHistorySize = _storeMoveHistory ? sizeof(INPUT_TYPE) * _maxMoveCount : 0;
   offset += _moveHistorySize;

   _frameDataOffset = offset;
   _frameDataSize = _useXDelta3 ? _maxXDelta3Differences : _STATE_DATA_SIZE_TRAIN;
   offset += _frameDataSize;

   _ruleStatusOffset = offset;
   _ruleStatusSize = sizeof(bool) * _ruleCount;
   offset += _ruleStatusSize;

   if (_useXDelta3)
   {
    _xdelta3DiffCountOffset = offset;
    _xdelta3DiffCountSize = sizeof(usize_t);
    offset += _xdelta3DiffCountSize;
   }

   return offset;
  }

  inline void copy(const State* src) { memcpy(this, src, _stateSize); }
  inline bool* getRuleStatus() { return (bool*)((uint8_t*)this + _ruleStatusOffset); }

  inline void computeStateDifference(const uint8_t* __restrict__ baseStateData, const uint8_t* __restrict__ newStateData)
  {
   if (_useXDelta3)
   {
    usize_t* diffCount = ((usize_t*)((uint8_t*)this +_xdelta3DiffCountOffset));
    int ret = xd3_encode_memory(newStateData, _STATE_DATA_SIZE_TRAIN, baseStateData, _STATE_DATA_SIZE_TRAIN, (uint8_t*)this + _frameDataOffset, diffCount, _frameDataSize, _useXdelta3ZLibCompression ? 0 : XD3_NOCOMPRESS);
    if (*diffCount > _maxXDelta3Differences) EXIT_WITH_ERROR("[Error] Exceeded maximum frame difference: %d > %d.\n", *diffCount, _maxXDelta3Differences);
    if (ret != 0)
    {
     fprintf(stderr, "[Error] State Encode failure: %d: %s\n", ret, xd3_strerror(ret));
     const size_t newSize = 1048576;
     uint8_t* newBuf = (uint8_t*)malloc(1048576 * sizeof(uint8_t));
     xd3_encode_memory(newStateData, _STATE_DATA_SIZE_TRAIN, baseStateData, _STATE_DATA_SIZE_TRAIN, newBuf, diffCount, newSize, _useXdelta3ZLibCompression ? 0 : XD3_NOCOMPRESS);
     if (*diffCount > _maxXDelta3Differences) EXIT_WITH_ERROR("[Error] Attempted encoding size: %d > %d.\n", *diffCount, _maxXDelta3Differences);
    }
    if (*diffCount > _currentXDelta3DMaxDiff) _currentXDelta3DMaxDiff = *diffCount;
   }
   else
   {
    memcpy((uint8_t*)this + _frameDataOffset, newStateData, _frameDataSize);
   }
  }

  inline void getStateDataFromDifference(const uint8_t* __restrict__ baseStateData, uint8_t* __restrict__ stateData) const
  {
   if (_useXDelta3)
   {
    usize_t output_size;
    usize_t* diffCount = ((usize_t*)((uint8_t*)this +_xdelta3DiffCountOffset));
    int ret = xd3_decode_memory ((uint8_t*)this + _frameDataOffset, *diffCount, baseStateData, _STATE_DATA_SIZE_TRAIN, stateData, &output_size, _STATE_DATA_SIZE_TRAIN, _useXdelta3ZLibCompression ? 0 : XD3_NOCOMPRESS);
    if (ret != 0) EXIT_WITH_ERROR("[Error] State Decode failure: %d: %s\n", ret, xd3_strerror(ret));
   }
   else
   {
    memcpy(stateData, (uint8_t*)this + _frameDataOffset, _frameDataSize);
   }
  }

  // Parsing configuration
  static void parseConfiguration(const nlohmann::json& config)
  {
   if (isDefined(config, "Max Move Count") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Max Move Count' key.\n");
   _maxMoveCount = config["Max Move Count"].get<uint16_t>();

   if (isDefined(config, "Store Move History") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'Store Move History' key.\n");
   _storeMoveHistory = config["Store Move History"].get<bool>();

   if (isDefined(config, "XDelta3 Enabled") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'XDelta3 Enabled' key.\n");
   _useXDelta3 = config["XDelta3 Enabled"].get<bool>();

   if (_useXDelta3)
   {
    if (isDefined(config, "XDelta3 Max Differences") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'XDelta3 Max Differences' key.\n");
    _maxXDelta3Differences = config["XDelta3 Max Differences"].get<uint16_t>();

    if (isDefined(config, "XDelta3 Use Zlib Compression") == false) EXIT_WITH_ERROR("[ERROR] State Configuration missing 'XDelta3 Use Zlib Compression' key.\n");
    _useXdelta3ZLibCompression = config["XDelta3 Use Zlib Compression"].get<bool>();
   }
  }

  // Move r/w operations
  inline void setMove(const size_t idx, const INPUT_TYPE move)
  {
   INPUT_TYPE* basePos = (INPUT_TYPE*)((uint8_t*)this + _moveHistoryOffset);
   basePos[idx] = move;
  }

  inline INPUT_TYPE getMove(const size_t idx) const
  {
   INPUT_TYPE* basePos = (INPUT_TYPE*)((uint8_t*)this + _moveHistoryOffset);
   return basePos[idx];
  }
};
