#pragma once

#include "quickNESInstance.h"
#include "nlohmann/json.hpp"
#include <cstddef>
#include "frame.h"
#include "rule.h"
#include <string>
#include <vector>
#include <set>

class State
{
  public:

  State(const std::string romFile, const std::string stateFile, const nlohmann::json rulesConfig);

  // Print Rule information
  void printRuleStatus(const bool* rulesStatus);

  std::vector<Rule *> _rules;
  size_t _ruleCount;
  quickNESInstance *_nes;

  // Function to determine the current possible moves
  inline std::vector<uint8_t> getPossibleMoveIds() const
  {
   // Possible moves
   // Move Ids =        0    1    2    3    4    5     6     7     8    9     10    11      12    13
   //_possibleMoves = {".", "L", "R", "D", "A", "B", "LA", "RA", "LB", "RB", "LR", "LRA", "LRB", "S" };

   // If Mario's state is not normal (!= 8), there's nothing to do except wait
   if (*_nes->_marioState != 8) return { 0 };

   // By default try all possible combinations
   return { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
  }

  // Function to get magnet information
  inline magnetSet_t getMagnetValues(const bool* rulesStatus) const
  {
   // Storage for magnet information
   magnetSet_t magnetSet;

   magnetSet.marioScreenOffsetMagnet.intensity = 0.0f;
   magnetSet.marioScreenOffsetMagnet.max = 0.0f;

   magnetSet.screenHorizontalMagnet.intensity = 0.0f;
   magnetSet.screenHorizontalMagnet.max = 0.0f;

   magnetSet.marioHorizontalMagnet.intensity = 0.0f;
   magnetSet.marioHorizontalMagnet.max = 0.0f;

   magnetSet.marioVerticalMagnet.intensity = 0.0f;
   magnetSet.marioVerticalMagnet.max = 0.0f;

   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++) if (rulesStatus[ruleId] == true) magnetSet = _rules[ruleId]->_magnetSet;

   return magnetSet;
  }

  // Obtains the score of a given frame
  inline float getFrameReward(const bool* rulesStatus) const
  {
    // Getting rewards from rules
    float reward = 0.0;
    for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
     if (rulesStatus[ruleId] == true)
      reward += _rules[ruleId]->_reward;

    // Getting magnet values for the kid
    auto magnetSet = getMagnetValues(rulesStatus);

    // Evaluating screen screen magnet value
    reward += magnetSet.screenHorizontalMagnet.intensity * std::min((float)_nes->_screenPosX, magnetSet.screenHorizontalMagnet.max);

    // Evaluating mario / screen offset magnet value
    reward += magnetSet.marioScreenOffsetMagnet.intensity * std::min((float)_nes->_marioScreenOffset, magnetSet.marioScreenOffsetMagnet.max);

    // Evaluating mario magnet's reward on position X
    reward += magnetSet.marioHorizontalMagnet.intensity * std::min((float)_nes->_marioPosX, magnetSet.marioHorizontalMagnet.max);

    // Evaluating mario magnet's reward on position Y
    if (magnetSet.marioVerticalMagnet.intensity > 0.0f) reward += magnetSet.marioVerticalMagnet.intensity * (256.0f - std::min((float)*_nes->_marioPosY, magnetSet.marioVerticalMagnet.max));
    if (magnetSet.marioVerticalMagnet.intensity < 0.0f) reward += magnetSet.marioVerticalMagnet.intensity * (000.0f - std::max((float)*_nes->_marioPosY, magnetSet.marioVerticalMagnet.max));

    // If mario is getting into the tube, reward timer going down
    if (*_nes->_marioState == 3) reward += 1000.0f * (24.0f - (float)*_nes->_climbSideTimer);

    // Returning reward
    return reward;
  }

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

  // Serialization/Deserialization Routines
  inline void pushState(const uint8_t* __restrict__ inputStateData) const
  {
    _nes->deserializeState(inputStateData);
  }

  inline void popState(uint8_t* __restrict__ outputStateData) const
  {
   _nes->serializeState(outputStateData);
  }

  // Get frame type
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

};
