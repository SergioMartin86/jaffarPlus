#pragma once

#include "gameInstanceData.hpp"
#include "emuInstance.hpp"
#include "nlohmann/json.hpp"
#include "state.hpp"
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
  gameInstanceData_t _data;

  // Storage for rules
  std::vector<Rule *> _rules;

  // Underlying emulator instance
  EmuInstance *_emu;

  // virtual destructor
  virtual ~GameInstanceBase() = default;

  // Initialize game instance using the Jaffar configuration file
  virtual void initialize(const nlohmann::json& config)
  {
   // Checking whether it contains the emulator configuration field
   if (isDefined(config, "Emulator Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Emulator Configuration' key.\n");

   // Creating emulator instance
   _emu = new EmuInstance(config["Emulator Configuration"]);

   // Setting game-specific value pointers
   setGameValuePointers();

   // Updating initial derived values
   updateDerivedValues();

   // Checking whether it contains the rules field
   if (isDefined(config, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rules' key.\n");

   // Parsing rules
   parseRules(config["Rules"]);
  }

  // Function to set base emulator pointer
  void setEmulator(EmuInstance* emu) { _emu = emu; }

  // Rule parser
  void parseRules(const nlohmann::json rulesConfig)
  {
   // Processing rules
   for (size_t ruleId = 0; ruleId < rulesConfig.size(); ruleId++)
    _rules.push_back(new Rule(rulesConfig[ruleId], this));

   // Setting global rule count
   _ruleCount = _rules.size();

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
  inline stateType getStateType(const bool* rulesStatus) const
  {
   stateType type = f_regular;

   for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
    if (rulesStatus[ruleId] == true)
    {
     if (_rules[ruleId]->_isFailRule == true) type = f_fail;
     if (_rules[ruleId]->_isWinRule == true) type = f_win;
    }

   return type;
  }

  // Function to advance frame
  void advanceState(const uint8_t &move) { _emu->advanceState(move); updateDerivedValues(); };
  void advanceState(const std::string& move) { _emu->advanceState(move); updateDerivedValues(); };

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

  // Obtains the score of a given frame
  virtual inline float getStateReward(const bool* rulesStatus) const = 0;

  // Function to print
  virtual void printStateInfo(const bool* rulesStatus) const = 0;
};
