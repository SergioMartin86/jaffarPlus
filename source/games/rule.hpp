#pragma once

#include <common/json.hpp>
#include <vector>

enum operator_t
{
  op_equal,
  op_not_equal,
  op_greater,
  op_greater_or_equal,
  op_less,
  op_less_or_equal,
  op_bit_true,
  op_bit_false
};

enum datatype_t
{
  dt_uint8,
  dt_uint16,
  dt_uint32,
  dt_int8,
  dt_int16,
  dt_int32,
  dt_double,
  dt_float
};

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

class Rule final
{
  public:

  typedef size_t ruleLabel_t;

  Rule(const size_t label) : _label(label) {};
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

  size_t getLabel() const { return _label; }
  float getReward() const { return _reward; }
  bool getWinRule() const { return _isWinRule; }
  bool getFailRule() const { return _isFailRule; }
  bool getCheckpointRule() const { return _isCheckpointRule; }
  size_t getCheckpointTolerance() const { return _checkPointTolerance; }

  private:

  // Conditions are evaluated frequently, so this optimized for performance
  // Operands are pre-parsed as pointers/immediates and the evaluation function
  // is a template that is created at compilation time.
  std::vector<Condition *> _conditions;

  inline operator_t getOperationType(const std::string &operation)
  {
    if (operation == "==") return op_equal;
    if (operation == "!=") return op_not_equal;
    if (operation == ">") return op_greater;
    if (operation == ">=") return op_greater_or_equal;
    if (operation == "<") return op_less;
    if (operation == "<=") return op_less_or_equal;
    if (operation == "BitTrue") return op_bit_true;
    if (operation == "BitFalse") return op_bit_false;

    EXIT_WITH_ERROR("[Error] Rule %lu, unrecognized operator: %s\n", _label, operation.c_str());

    return op_equal;
  }

  // Stores an identifying label for the rule
  const size_t _label;

  // Stores the reward associated with meeting this rule
  float _reward = 0.0;

  // Special condition flags
  bool _isWinRule = false;
  bool _isFailRule = false;
  bool _isCheckpointRule = false;
  size_t _checkPointTolerance = 0;

  // Stores rules that also satisfied if this one is
  std::vector<size_t> _satisfiesLabels;
  std::vector<size_t> _satisfiesIndexes;

  // Storage for game-specific actions
  std::vector<std::function<void()>> _actions;
};


