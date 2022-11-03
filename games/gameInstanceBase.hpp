#pragma once

#include "emuInstance.hpp"
#include "nlohmann/json.hpp"
#include "state.hpp"
#include "metrohash128.h"
#include <set>

class GameRule;

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
  stateType getStateType(const bool* rulesStatus) const;

  // Function to advance state. Returns the number of skipped frames
  virtual uint16_t advanceState(const INPUT_TYPE &move) { _emu->advanceState(move); updateDerivedValues(); return 0; };
  uint16_t advanceStateString(const std::string& move) { return advanceState(_emu->moveStringToCode(move)); }

  // Serialization/Deserialization Routines
  void popState(uint8_t* __restrict__ outputStateData) const { _emu->serializeState(outputStateData); }
  void pushState(const uint8_t* __restrict__ inputStateData) { _emu->deserializeState(inputStateData);  updateDerivedValues(); }

  void saveStateFile(const std::string& outputFilePath) const { _emu->saveStateFile(outputFilePath); }
  void loadStateFile(const std::string& inputFilePath) { _emu->loadStateFile(inputFilePath);  updateDerivedValues(); }

  // Evaluates the rule set on a given frame. Returns true if it is a fail.
  void evaluateRules(bool* rulesStatus) const;

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  void satisfyRule(bool* rulesStatus, const size_t ruleId) const;

  // This function computes the hash for the current state
  virtual _uint128_t computeHash() const = 0;

  // This function updates derivate values (those who require calculation from raw values) after a state is re-loaded
  virtual void updateDerivedValues() = 0;

  // Function to determine the current possible moves
  virtual std::vector<std::string> getPossibleMoves() const = 0;

  // Obtains the score of a given frame
  virtual float getStateReward(const bool* rulesStatus) const = 0;

  // Function to print
  virtual void printStateInfo(const bool* rulesStatus) const = 0;

  // Function to set RNG state
  virtual void setRNGState(const uint64_t RNGState) = 0;
};
