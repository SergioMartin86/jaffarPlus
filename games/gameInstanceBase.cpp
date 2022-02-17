#include "gameRule.hpp"
#include "gameInstanceBase.hpp"

// Rule parser
void GameInstanceBase::parseRules(const nlohmann::json rulesConfig)
{
 // Processing rules
 for (size_t ruleId = 0; ruleId < rulesConfig.size(); ruleId++)
 {
  GameRule* rule = new GameRule();
  rule->initialize(rulesConfig[ruleId], (void*)this);
  _rules.push_back(rule);
 }

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
stateType GameInstanceBase::getStateType(const bool* rulesStatus) const
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

// Evaluates the rule set on a given frame. Returns true if it is a fail.
void GameInstanceBase::evaluateRules(bool* rulesStatus) const
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
void GameInstanceBase::satisfyRule(bool* rulesStatus, const size_t ruleId) const
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
