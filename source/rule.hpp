#pragma once

#include "nlohmann/json.hpp"
#include "utils.hpp"
#include <vector>

enum operator_t
{
  op_equal = 0,
  op_not_equal = 1,
  op_greater = 2,
  op_greater_or_equal = 3,
  op_less = 4,
  op_less_or_equal = 5,
  op_bit_true = 6,
  op_bit_false = 7
};

enum datatype_t
{
  dt_uint8 = 0,
  dt_uint16 = 1,
  dt_uint32 = 2,
  dt_int8 = 3,
  dt_int16 = 4,
  dt_int32 = 5,
  dt_double = 6,
  dt_float = 7
};

class GameInstance;

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
  static inline bool _opBitTrue(const T a, const T b) { return getBitFlag(a,b); }
  static inline bool _opBitFalse(const T a, const T b) { return !getBitFlag(a,b); }

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
  if (_opType == op_bit_true) _opFcPtr = _opBitTrue;
  if (_opType == op_bit_false) _opFcPtr = _opBitFalse;

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
  Rule();
  virtual ~Rule() = default;

  // Stores an identifying label for the rule
  size_t _label;

  // Stores the reward associated with meeting this rule
  float _reward;

  // Special condition flags
  bool _isWinRule;
  bool _isFailRule;
  bool _isCheckpointRule;
  int _checkPointTolerance;

  // Stores rules that also satisfied if this one is
  std::vector<size_t> _satisfiesLabels;
  std::vector<size_t> _satisfiesIndexes;

  protected:

  // Conditions are evaluated frequently, so this optimized for performance
  // Operands are pre-parsed as pointers/immediates and the evaluation function
  // is a template that is created at compilation time.
  std::vector<Condition *> _conditions;
  size_t _conditionCount;
  virtual datatype_t getPropertyType(const nlohmann::json& property) = 0;
  virtual void *getPropertyPointer(const nlohmann::json& property, GameInstance* gameInstance) = 0;
  operator_t getOperationType(const std::string &operation);

  // Function to parse the json-encoded actions
  void parseActions(nlohmann::json actionsJs);

  // Function to parse game-specific actions
  virtual bool parseGameAction(nlohmann::json actionJs, size_t actionId) = 0;

  public:

  // Function to initialize game rule data
  void initialize(nlohmann::json ruleJs, void* gameInstance);

  // The rule is achieved only if all conditions are met
  inline bool evaluate() const
  {
    #pragma GCC unroll 8
    for (size_t i = 0; i < _conditionCount; i++) if(_conditions[i]->evaluate() == false) return false;
    return true;
  }

};


