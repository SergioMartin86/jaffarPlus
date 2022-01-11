#pragma once

#include "quickNESInstance.h"
#include "nlohmann/json.hpp"
#include "utils.h"
#include <vector>

enum operator_t
{
  op_equal = 0,
  op_not_equal = 1,
  op_greater = 2,
  op_greater_or_equal = 3,
  op_less = 4,
  op_less_or_equal = 5
};

enum datatype_t
{
  dt_byte = 0,
  dt_sbyte = 1,
  dt_short = 2,
  dt_int = 3,
  dt_word = 4,
  dt_dword = 5,
  dt_ulong = 6
};

// Modifier that specifies whether to store move list
extern bool _storeMoveList;

class Condition
{
  public:
  Condition(const operator_t opType)
  {
    _opType = opType;
  }

  virtual inline bool evaluate() = 0;
  virtual ~Condition() = default;

  protected:
  operator_t _opType;
};

template <typename T>
class _vCondition : public Condition
{
  public:
  _vCondition(const operator_t opType, void *property1, void *property2, T immediate);
  inline bool evaluate() override;

  private:
  static inline bool _opEqual(const T a, const T b) { return a == b; }
  static inline bool _opNotEqual(const T a, const T b) { return a != b; }
  static inline bool _opGreater(const T a, const T b) { return a > b; }
  static inline bool _opGreaterOrEqual(const T a, const T b) { return a >= b; }
  static inline bool _opLess(const T a, const T b) { return a < b; }
  static inline bool _opLessOrEqual(const T a, const T b) { return a <= b; }

  bool (*_opFcPtr)(const T, const T);

  T *_property1;
  T *_property2;
  T _immediate;
};

template <typename T>
_vCondition<T>::_vCondition(const operator_t opType, void *property1, void* property2, T immediate) : Condition(opType)
{
  if (_opType == op_equal) _opFcPtr = _opEqual;
  if (_opType == op_not_equal) _opFcPtr = _opNotEqual;
  if (_opType == op_greater) _opFcPtr = _opGreater;
  if (_opType == op_greater_or_equal) _opFcPtr = _opGreaterOrEqual;
  if (_opType == op_less) _opFcPtr = _opLess;
  if (_opType == op_less_or_equal) _opFcPtr = _opLessOrEqual;

  _property1 = (T *)property1;
  _property2 = (T *)property2;
  _immediate = immediate;
}

template <typename T>
inline bool _vCondition<T>::evaluate()
{
  if (_property2 != NULL) return _opFcPtr(*_property1, *_property2);
  return _opFcPtr(*_property1, _immediate);
}

class Rule
{
  public:
  Rule(nlohmann::json ruleJs, quickNESInstance *nes);

  // Stores an identifying label for the rule
  size_t _label;

  // Stores the reward associated with meeting this rule
  float _reward;

  // Special condition flags
  bool _isWinRule;
  bool _isFailRule;

  // Stores magnet information
  float _marioMagnetIntensityX;
  float _marioMagnetIntensityY;

  // Stores rules that also satisfied if this one is
  std::vector<size_t> _satisfiesLabels;
  std::vector<size_t> _satisfiesIndexes;

  private:

  // Conditions are evaluated frequently, so this optimized for performance
  // Operands are pre-parsed as pointers/immediates and the evaluation function
  // is a template that is created at compilation time.
  std::vector<Condition *> _conditions;
  size_t _conditionCount;
  datatype_t getPropertyType(const std::string &property);
  void *getPropertyPointer(const std::string &property, quickNESInstance *nes);
  operator_t getOperationType(const std::string &operation);

  // Function to parse the json-encoded actions
  void parseActions(nlohmann::json actionsJs);

  public:

  // The rule is achieved only if all conditions are met
  inline bool evaluate() const
  {
    #pragma GCC unroll 8
    for (size_t i = 0; i < _conditionCount; i++) if(_conditions[i]->evaluate() == false) return false;
    return true;
  }

};


