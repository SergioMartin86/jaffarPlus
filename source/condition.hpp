#pragma once

#include "property.hpp"
#include <jaffarCommon/bitwise.hpp>

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
    op_bit_false,
    op_modulo_zero,
    op_modulo_non_zero
  };

  Condition(const operator_t opType) : _opType(opType) {}

  virtual __INLINE__ bool evaluate() const = 0;
  virtual ~Condition()                     = default;

  static __INLINE__ operator_t getOperatorType(const std::string& operation)
  {
    if (operation == "==") return op_equal;
    if (operation == "!=") return op_not_equal;
    if (operation == ">") return op_greater;
    if (operation == ">=") return op_greater_or_equal;
    if (operation == "<") return op_less;
    if (operation == "<=") return op_less_or_equal;
    if (operation == "%0") return op_modulo_zero;
    if (operation == "%N") return op_modulo_non_zero;
    if (operation == "BitTrue") return op_bit_true;
    if (operation == "BitFalse") return op_bit_false;

    JAFFAR_THROW_LOGIC("[Error] Unrecognized operator: %s\n", operation.c_str());

    return op_equal;
  }

protected:
  const operator_t _opType;
};

template <typename T>
class _vCondition : public Condition
{
public:
  _vCondition(const operator_t opType, Property* property1, Property* property2, T immediate1, T immediate2)
      : Condition(opType), _property1(property1), _property2(property2), _immediate1(immediate1), _immediate2(immediate2)
  {
    switch (_opType)
    {
      case op_equal: _opFcPtr = _opEqual; break;
      case op_not_equal: _opFcPtr = _opNotEqual; break;
      case op_greater: _opFcPtr = _opGreater; break;
      case op_greater_or_equal: _opFcPtr = _opGreaterOrEqual; break;
      case op_less: _opFcPtr = _opLess; break;
      case op_less_or_equal: _opFcPtr = _opLessOrEqual; break;
      case op_bit_true: _opFcPtr = _opBitTrue; break;
      case op_bit_false: _opFcPtr = _opBitFalse; break;
      case op_modulo_zero: _opFcPtr = _opModuloZero; break;
      case op_modulo_non_zero: _opFcPtr = _opModuloNonZero; break;
      default: break;
    }
  }

  __INLINE__ bool evaluate() const
  {
    T immediate1 = _immediate1;
    T immediate2 = _immediate2;
    if (_property1 != nullptr) immediate1 = _property1->getValue<T>();
    if (_property2 != nullptr) immediate2 = _property2->getValue<T>();
    return _opFcPtr(immediate1, immediate2);
  }

private:
  static __INLINE__ bool _opEqual(const T a, const T b) { return a == b; }
  static __INLINE__ bool _opNotEqual(const T a, const T b) { return a != b; }
  static __INLINE__ bool _opGreater(const T a, const T b) { return a > b; }
  static __INLINE__ bool _opGreaterOrEqual(const T a, const T b) { return a >= b; }
  static __INLINE__ bool _opLess(const T a, const T b) { return a < b; }
  static __INLINE__ bool _opLessOrEqual(const T a, const T b) { return a <= b; }
  static __INLINE__ bool _opBitTrue(const T a, const T b) { return jaffarCommon::bitwise::getBitFlag(a, b); }
  static __INLINE__ bool _opBitFalse(const T a, const T b) { return !jaffarCommon::bitwise::getBitFlag(a, b); }
  static __INLINE__ bool _opModuloZero(const T a, const T b) { return (uint64_t)a % (uint64_t)b == 0; }
  static __INLINE__ bool _opModuloNonZero(const T a, const T b) { return (uint64_t)a % (uint64_t)b > 0; }

  bool (*_opFcPtr)(const T, const T);

  Property* const _property1;
  Property* const _property2;
  const T         _immediate1;
  const T         _immediate2;
};

} // namespace jaffarPlus