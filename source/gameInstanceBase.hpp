#pragma once

#include "gameData.hpp"
#include "emuInstance.hpp"
#include "nlohmann/json.hpp"
#include "frame.hpp"
#include "rule.hpp"
#include "metrohash64.h"
#include <set>

// If we use NCurses, we need to use the appropriate printing function
#ifdef NCURSES
 #include <ncurses.h>
 #define LOG printw
#else
 #define LOG printf
#endif

class GameInstanceBase
{
 public:

  // Container for game-specific values
  gameData_t _gameData;

  // Storage for rules
  std::vector<Rule *> _rules;

  // Counter for number of rules
  size_t _ruleCount;

  // Underlying emulator instance
  EmuInstance *_emu;

  // Constructor for the underlying emulator using a rom file and a state
  GameInstanceBase(const std::string romFile, const std::string stateFile)
  {
    // Creating quickNES instance
    _emu = new EmuInstance(romFile);
    _emu->loadStateFile(stateFile);
  }

  // Constructor using an already existing emulator
  GameInstanceBase(EmuInstance* emu)
  {
    // Copying emu instance pointer
    _emu = emu;
  }

  // virtual destructor
  virtual ~GameInstanceBase() = default;

  // Rule parser
  void parseRules(const nlohmann::json rulesConfig)
  {
   // Processing rules
   for (size_t ruleId = 0; ruleId < rulesConfig.size(); ruleId++)
    _rules.push_back(new Rule(rulesConfig[ruleId], this));

   // Setting global rule count
   _ruleCount = _rules.size();

   if (_ruleCount > _MAX_RULE_COUNT) EXIT_WITH_ERROR("[ERROR] Configured Jaffar to run %lu rules, but the specified script contains %lu. Modify frame.h and rebuild to run this script.\n", _MAX_RULE_COUNT, _ruleCount);

   // Checking for repeated rule labels
   std::set<size_t> ruleLabelSet;
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
   {
    size_t label = _rules[ruleId]->_label;
    ruleLabelSet.insert(label);
    if (ruleLabelSet.size() < ruleId + 1)
     EXIT_WITH_ERROR("[ERROR] Rule label %lu is repeated in the configuration file.\n", label);
   }

   // Looking for rule satisfied sub-rules indexes that match their labels
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
    for (size_t satisfiedId = 0; satisfiedId < _rules[ruleId]->_satisfiesLabels.size(); satisfiedId++)
    {
     bool foundLabel = false;
     size_t label = _rules[ruleId]->_satisfiesLabels[satisfiedId];
     if (label == _rules[ruleId]->_label) EXIT_WITH_ERROR("[ERROR] Rule %lu references itself in satisfied vector.\n", label);

     for (size_t subRuleId = 0; subRuleId < _ruleCount; subRuleId++)
      if (_rules[subRuleId]->_label == label)
      {
       _rules[ruleId]->_satisfiesIndexes[satisfiedId] = subRuleId;
       foundLabel = true;
       break;
      }
     if (foundLabel == false) EXIT_WITH_ERROR("[ERROR] Could not find rule label %lu, specified as satisfied by rule %lu.\n", label, satisfiedId);
    }
  }

  // Function to return frame type
  inline frameType getFrameType(const bool* rulesStatus) const
  {
   frameType type = f_regular;

   for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
    if (rulesStatus[ruleId] == true)
    {
     if (_rules[ruleId]->_isFailRule == true) type = f_fail;
     if (_rules[ruleId]->_isWinRule == true) type = f_win;
    }

   return type;
  }

  // Function to print rule status
  void printRuleStatus(const bool* rulesStatus) const
  {
   LOG("[Jaffar]  + Rule Status: ");
   for (size_t i = 0; i < _ruleCount; i++)
   {
     if (i > 0 && i % 60 == 0) printf("\n                         ");
     printf("%d", rulesStatus[i] ? 1 : 0);
   }
   LOG("\n");
  }

  // Function to advance frame
  void advanceFrame(const uint8_t &move) { _emu->advanceFrame(move); updateDerivedValues(); };
  void advanceFrame(const std::string& move) { _emu->advanceFrame(move); updateDerivedValues(); };

  // Serialization/Deserialization Routines
  inline void popState(uint8_t* __restrict__ outputStateData) const { _emu->serializeState(outputStateData); }
  inline void pushState(const uint8_t* __restrict__ inputStateData) { _emu->deserializeState(inputStateData);  updateDerivedValues(); }

  // Evaluates the rule set on a given frame. Returns true if it is a fail.
  inline void evaluateRules(bool* rulesStatus) const
  {
    for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
    {
      // Evaluate rule only if it's active
      if (rulesStatus[ruleId] == false)
      {
        // Checking if conditions are met
        bool isSatisfied = _rules[ruleId]->evaluate();

        // If it's achieved, update its status and run its actions
        if (isSatisfied) satisfyRule(rulesStatus, ruleId);
      }
    }
  }

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  inline void satisfyRule(bool* rulesStatus, const size_t ruleId) const
  {
   // Recursively run actions for the yet unsatisfied rules that are satisfied by this one and mark them as satisfied
   for (size_t satisfiedIdx = 0; satisfiedIdx < _rules[ruleId]->_satisfiesIndexes.size(); satisfiedIdx++)
   {
    // Obtaining index
    size_t subRuleId = _rules[ruleId]->_satisfiesIndexes[satisfiedIdx];

    // Only activate it if it hasn't been activated before
    if (rulesStatus[subRuleId] == false) satisfyRule(rulesStatus, subRuleId);
   }

   // Setting status to satisfied
   rulesStatus[ruleId] = true;
  }

  // This function computes the hash for the current state
  virtual inline uint64_t computeHash() const = 0;

  // This function sets the relevant pointers for game data
  virtual void setGameValuePointers() = 0;

  // This function updates derivate values (those who require calculation from raw values) after a state is re-loaded
  virtual inline void updateDerivedValues() = 0;

  // Function to determine the current possible moves
  virtual inline std::vector<uint8_t> getPossibleMoveIds() const = 0;

  // Function to get magnet information
  virtual inline magnetSet_t getMagnetValues(const bool* rulesStatus) const = 0;

  // Obtains the score of a given frame
  virtual inline float getFrameReward(const bool* rulesStatus) const = 0;

  // Function to print
  virtual void printStateInfo() const = 0;
};
