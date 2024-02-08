#pragma once

#include <jaffarCommon/include/json.hpp>
#include <vector>
#include "property.hpp"
#include "condition.hpp"

namespace jaffarPlus
{

class Rule final
{
  public:

  typedef size_t label_t;

  Rule(const size_t index, const label_t label) :
   _index(index), 
   _label(label)
   {};
  ~Rule() = default;

  // The rule is achieved only if all conditions are met
  inline bool evaluate() const
  {
    for (const auto& c : _conditions) if(c->evaluate() == false) return false;
    return true;
  }

  void setReward(const float reward) { _reward = reward; }
  void setWinRule(const bool isWinRule) { _isWinRule = isWinRule; }
  void setFailRule(const bool isFailRule) { _isFailRule = isFailRule; }
  void setCheckpointRule(const bool isCheckpointRule) { _isCheckpointRule = isCheckpointRule; }
  void setCheckpointTolerance(const size_t checkPointTolerance) { _checkPointTolerance = checkPointTolerance; }
  void addAction(const std::function<void()>& function) { _actions.push_back(function); }
  void addCondition(std::unique_ptr<Condition> condition) { _conditions.insert(std::move(condition)); }
  void addSatisfyRuleLabel(const label_t satisfyRuleLabel) { _satisfyRuleLabels.insert(satisfyRuleLabel); }

  label_t getLabel() const { return _label; }
  float getReward() const { return _reward; }
  bool isWinRule() const { return _isWinRule; }
  bool isFailRule() const { return _isFailRule; }
  bool isCheckpointRule() const { return _isCheckpointRule; }
  size_t getCheckpointTolerance() const { return _checkPointTolerance; }
  std::unordered_set<label_t> getSatisfyRuleLabels() const { return _satisfyRuleLabels; }
  size_t getIndex() const { return _index; }
  const std::vector<std::function<void()>>& getActions() const { return _actions; }
  
  private:

  // Conditions are evaluated frequently, so this optimized for performance
  // Operands are pre-parsed as pointers/immediates and the evaluation function
  // is a template that is created at compilation time.
  std::unordered_set<std::unique_ptr<Condition>> _conditions;
  
  // Internal index for sequential access
  const size_t _index;

  // Stores an identifying label for the rule
  const label_t _label;

  // Stores the reward associated with meeting this rule
  float _reward = 0.0;

  // Special condition flags
  bool _isWinRule = false;
  bool _isFailRule = false;
  bool _isCheckpointRule = false;
  size_t _checkPointTolerance = 0;

  // Stores rules that also satisfied if this one is
  std::unordered_set<label_t> _satisfyRuleLabels;

  // Storage for game-specific actions
  std::vector<std::function<void()>> _actions;
};

} // namespace jaffarPlus
