#pragma once

#include <common/json.hpp>
#include <vector>
#include <unordered_set>
#include "property.hpp"
#include "condition.hpp"

namespace jaffarPlus
{

class InputSet final
{
  public:

  // Type for input indexing
  typedef uint32_t inputIndex_t; 

  InputSet() = default;
  ~InputSet() = default;

  // The input set is activated only if all conditions are met
  inline bool evaluate() const
  {
    for (const auto& c : _conditions) if(c->evaluate() == false) return false;
    return true;
  }

  void addInput(const inputIndex_t inputIdx) { _inputIndexes.insert(inputIdx); }
  void addCondition(std::unique_ptr<Condition> condition) { _conditions.insert(std::move(condition)); }
  const std::unordered_set<inputIndex_t>& getInputIndexes() const { return _inputIndexes; }
  
  private:

  // Conditions are evaluated frequently, so this optimized for performance
  // Operands are pre-parsed as pointers/immediates and the evaluation function
  // is a template that is created at compilation time.
  std::unordered_set<std::unique_ptr<Condition>> _conditions;
  
  // Storage for game-specific actions
  std::unordered_set<inputIndex_t> _inputIndexes;
};

} // namespace jaffarPlus
