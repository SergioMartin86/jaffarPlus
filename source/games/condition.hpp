#pragma once

#include "property.hpp"

namespace jaffarPlus
{

class Condition
{
  public:

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

  Condition(const operator_t opType) : _opType(opType) { }

  virtual inline bool evaluate() = 0;
  virtual ~Condition() = default;

  protected:

  const operator_t _opType;
};

template <typename T>
class _vCondition : public Condition
{
  public:

  _vCondition(const operator_t opType, Property *property1, Property * property2, T immediate1, T immediate2) : Condition(opType)
  {
    switch (_opType)
    {
      case op_equal            : _opFcPtr = _opEqual; break;
      case op_not_equal        : _opFcPtr = _opNotEqual; break;
      case op_greater          : _opFcPtr = _opGreater;break;
      case op_greater_or_equal : _opFcPtr = _opGreaterOrEqual; break;
      case op_less             : _opFcPtr = _opLess; break;
      case op_less_or_equal    : _opFcPtr = _opLessOrEqual; break;
      case op_bit_true         : _opFcPtr = _opBitTrue; break;
      case op_bit_false        : _opFcPtr = _opBitFalse; break;
      default: break;
    }

    _immediate1 = immediate1;
    _immediate2 = immediate2;
  }

  inline bool evaluate()
  {
    if (_property1 != NULL) _immediate1 = _property1->getValue<T>();
    if (_property2 != NULL) _immediate2 = _property2->getValue<T>();
    return _opFcPtr(_immediate1, _immediate2);
  }

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

  Property* const _property1;
  Property* const _property2;
  T _immediate1;
  T _immediate2;
};

} // namespace jaffarPlus