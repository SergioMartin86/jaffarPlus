#pragma once

#include "quickNESInstance.h"
#include "nlohmann/json.hpp"
#include <cstddef>
#include "frame.h"
#include <string>
#include <vector>
#include <set>

// Struct to hold all of the frame's magnet information
struct magnetInfo_t
{
 float intensityX;
 float intensityY;
};

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

   // W1-1, some keys can be skipped
   if (_nes->_currentWorld == 1 && _nes->_currentStage == 1) return { 0, 1, 2, 3, 4, 6, 7, 9, 10 };

   // By default try all possible combinations
   return { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
  }

  // Function to get magnet information
  inline magnetInfo_t getMarioMagnetValues(const bool* rulesStatus) const
  {
   // Storage for magnet information
   magnetInfo_t magnetInfo;
   magnetInfo.intensityX = 0.0f;
   magnetInfo.intensityY = 0.0f;

   // Iterating rule vector
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
   {
    if (rulesStatus[ruleId] == true)
    {
      const auto& rule = _rules[ruleId];
      magnetInfo.intensityX = rule->_marioMagnetIntensityX;
      magnetInfo.intensityY = rule->_marioMagnetIntensityY;
    }
   }

   return magnetInfo;
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
    auto marioMagnet = getMarioMagnetValues(rulesStatus);

    // Evaluating mario magnet's reward on position
    float velXRatio = (float)*_nes->_marioVelX/40.0f;
    reward += marioMagnet.intensityX * (float)_nes->getScreenScroll();
    reward += marioMagnet.intensityX * (float)*_nes->_marioRelPosX *(0.75f + 0.25f * velXRatio);
    if (marioMagnet.intensityY > 0.0f) reward += marioMagnet.intensityY * (256.0f - (float)*_nes->_marioPosY);
    if (marioMagnet.intensityY < 0.0f) reward += -1.0f * marioMagnet.intensityY * (float)*_nes->_marioPosY;

    // Evaluating mario magnet's Y reward on velocity
    if (marioMagnet.intensityY > 0.0f) reward += -1.0f * marioMagnet.intensityY * (float)*_nes->_marioVelY;
    if (marioMagnet.intensityY < 0.0f) reward += marioMagnet.intensityY * (float)*_nes->_marioVelY;

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
