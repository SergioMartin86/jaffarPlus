#pragma once

#include "emuInstance.hpp"
#include "nlohmann/json.hpp"
#include "state.hpp"
#include "metrohash128.h"
#include <set>

class GameRule;

struct checkPoint_t
{
 // Level: How deep is the checkpoint (how many others have been reached before)
 int level;

 // Tolerance: how many steps to run for before the frames that did not reach it are filtered out
 int tolerance;
};

class GameInstanceBase
{
 public:

  // Storage for rules
  std::vector<GameRule *> _rules;

  // Underlying emulator instance
  EmuInstance *_emu;

  // virtual destructor
  virtual ~GameInstanceBase() = default;

  // Rule parser
  void parseRules(const nlohmann::json rulesConfig);

  // Function to return frame type
  stateType_t getStateType(const bool* rulesStatus) const;

  // Function to return incresing checkpoint level
  checkPoint_t getCheckpointLevel(const bool* rulesStatus) const;

  // Function to advance state. Returns the number of skipped frames
  virtual std::vector<INPUT_TYPE> advanceGameState(const INPUT_TYPE &move) = 0;
  std::vector<INPUT_TYPE> advanceStateString(const std::string& move) {  return advanceGameState(_emu->moveStringToCode(move)); }

  // Serialization/Deserialization Routines
  void popState(uint8_t* __restrict__ outputStateData) const { _emu->serializeState(outputStateData); }
  void pushState(const uint8_t* __restrict__ inputStateData) { _emu->deserializeState(inputStateData);  updateDerivedValues(); }

  void saveStateFile(const std::string& outputFilePath) const { _emu->saveStateFile(outputFilePath); }
  void loadStateFile(const std::string& inputFilePath)
  {
   _emu->loadStateFile(inputFilePath);
   updateDerivedValues();
  }

  // Evaluates the rule set on a given frame.
  void evaluateRules(bool* rulesStatus) const;

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  void satisfyRule(bool* rulesStatus, const size_t ruleId) const;

  // This function computes the hash for the current state
  virtual _uint128_t computeHash(const uint16_t currentStep = 0) const = 0;

  // This function updates derivate values (those who require calculation from raw values) after a state is re-loaded
  virtual void updateDerivedValues() = 0;

  // Function to determine the current possible moves
  virtual std::vector<std::string> getPossibleMoves(const bool* rulesStatus) const = 0;

  // Obtains the score of a given frame
  virtual float getStateReward(const bool* rulesStatus) const = 0;

  // Function to print
  virtual void printStateInfo(const bool* rulesStatus) const = 0;

  // Function to save a trace
  virtual std::string getFrameTrace() const { return ""; }

  ////// Functions to detect possible moves

  // This function encodes a minimal set of the state variables, to differentiate possible move sets
  virtual uint64_t getStateMoveHash() const { return 0; }

  // This function returns a set of candidate moves
  virtual std::set<INPUT_TYPE> getCandidateMoves() const { return std::set<INPUT_TYPE>(); }
};
