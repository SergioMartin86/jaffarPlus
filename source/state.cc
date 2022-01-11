#include "state.h"
#include "utils.h"

extern nlohmann::json _scriptJs;

State::State(const std::string romFile, const std::string stateFile, const nlohmann::json rulesConfig)
{
  // Creating mini pop instance
  _nes = new quickNESInstance(romFile);
  _nes->loadStateFile(stateFile);

  // Processing rules
  for (size_t ruleId = 0; ruleId < rulesConfig.size(); ruleId++)
   _rules.push_back(new Rule(rulesConfig[ruleId], _nes));

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

void State::printRuleStatus(const bool* rulesStatus)
{
 printf("[Jaffar]  + Rule Status: ");
 for (size_t i = 0; i < _ruleCount; i++)
 {
   if (i > 0 && i % 60 == 0) printf("\n                         ");
   printf("%d", rulesStatus[i] ? 1 : 0);
 }
 printf("\n");
}
