#include "condition.hpp"
#include "property.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <jaffarCommon/exceptions.hpp>
#include <stdexcept>

using namespace jaffarPlus;

// ------------------------------------------------------------------
// Property
// ------------------------------------------------------------------

TEST(Property, DatatypeSizes)
{
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_uint8), 1u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_uint16), 2u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_uint32), 4u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_uint64), 8u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_int8), 1u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_int16), 2u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_int32), 4u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_int64), 8u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_bool), 1u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_float32), 4u);
  EXPECT_EQ(Property::getDatatypeSize(Property::dt_float64), 8u);
}

TEST(Property, GetSizeMatchesDatatype)
{
  uint64_t storage = 0;
  Property p("p", &storage, Property::dt_uint32, Property::little);
  EXPECT_EQ(p.getSize(), 4u);
}

TEST(Property, ParseDatatypeName)
{
  EXPECT_EQ(Property::parseDatatypeName("UINT8"), Property::dt_uint8);
  EXPECT_EQ(Property::parseDatatypeName("UINT16"), Property::dt_uint16);
  EXPECT_EQ(Property::parseDatatypeName("UINT32"), Property::dt_uint32);
  EXPECT_EQ(Property::parseDatatypeName("UINT64"), Property::dt_uint64);
  EXPECT_EQ(Property::parseDatatypeName("INT8"), Property::dt_int8);
  EXPECT_EQ(Property::parseDatatypeName("INT16"), Property::dt_int16);
  EXPECT_EQ(Property::parseDatatypeName("INT32"), Property::dt_int32);
  EXPECT_EQ(Property::parseDatatypeName("INT64"), Property::dt_int64);
  EXPECT_EQ(Property::parseDatatypeName("BOOL"), Property::dt_bool);
  EXPECT_EQ(Property::parseDatatypeName("FLOAT32"), Property::dt_float32);
  EXPECT_EQ(Property::parseDatatypeName("FLOAT64"), Property::dt_float64);
  EXPECT_THROW(Property::parseDatatypeName("NOT_A_TYPE"), std::logic_error);
}

TEST(Property, ParseEndiannessName)
{
  EXPECT_EQ(Property::parseEndiannessName("Little"), Property::little);
  EXPECT_EQ(Property::parseEndiannessName("Big"), Property::big);
  EXPECT_THROW(Property::parseEndiannessName("Middle"), std::logic_error);
}

TEST(Property, Getters)
{
  uint32_t storage = 0;
  Property p("myProp", &storage, Property::dt_uint32, Property::little);
  EXPECT_EQ(p.getName(), "myProp");
  EXPECT_EQ(p.getDatatype(), Property::dt_uint32);
  EXPECT_EQ(p.getPointer(), &storage);
  EXPECT_EQ(p.getNameHash(), jaffarCommon::hash::hashString("myProp"));
}

TEST(Property, GetValueLittleEndianAllTypes)
{
  {
    uint8_t  v = 0xAB;
    Property p("u8", &v, Property::dt_uint8, Property::little);
    EXPECT_EQ(p.getValue<uint8_t>(), 0xAB);
  }
  {
    uint16_t v = 0x1234;
    Property p("u16", &v, Property::dt_uint16, Property::little);
    EXPECT_EQ(p.getValue<uint16_t>(), 0x1234);
  }
  {
    uint32_t v = 0x12345678u;
    Property p("u32", &v, Property::dt_uint32, Property::little);
    EXPECT_EQ(p.getValue<uint32_t>(), 0x12345678u);
  }
  {
    uint64_t v = 0x123456789ABCDEF0ull;
    Property p("u64", &v, Property::dt_uint64, Property::little);
    EXPECT_EQ(p.getValue<uint64_t>(), 0x123456789ABCDEF0ull);
  }
  {
    int8_t   v = -5;
    Property p("i8", &v, Property::dt_int8, Property::little);
    EXPECT_EQ(p.getValue<int8_t>(), -5);
  }
  {
    int16_t  v = -1234;
    Property p("i16", &v, Property::dt_int16, Property::little);
    EXPECT_EQ(p.getValue<int16_t>(), -1234);
  }
  {
    int32_t  v = -123456;
    Property p("i32", &v, Property::dt_int32, Property::little);
    EXPECT_EQ(p.getValue<int32_t>(), -123456);
  }
  {
    int64_t  v = -123456789012ll;
    Property p("i64", &v, Property::dt_int64, Property::little);
    EXPECT_EQ(p.getValue<int64_t>(), -123456789012ll);
  }
  {
    bool     v = true;
    Property p("b", &v, Property::dt_bool, Property::little);
    EXPECT_EQ(p.getValue<bool>(), true);
  }
  {
    float    v = 3.5f;
    Property p("f32", &v, Property::dt_float32, Property::little);
    EXPECT_FLOAT_EQ(p.getValue<float>(), 3.5f);
  }
  {
    double   v = 3.5;
    Property p("f64", &v, Property::dt_float64, Property::little);
    EXPECT_DOUBLE_EQ(p.getValue<double>(), 3.5);
  }
}

TEST(Property, GetValueBigEndianSwapsBytes)
{
  // 1-byte: no swap
  {
    uint8_t  v = 0xAB;
    Property p("u8", &v, Property::dt_uint8, Property::big);
    EXPECT_EQ(p.getValue<uint8_t>(), 0xAB);
  }
  // 2-byte: bytes in memory order {0x12,0x34} read big-endian (MSB first) -> 0x1234
  {
    uint8_t  bytes[2] = {0x12, 0x34};
    Property p("u16", bytes, Property::dt_uint16, Property::big);
    EXPECT_EQ(p.getValue<uint16_t>(), 0x1234);
  }
  // 4-byte
  {
    uint8_t  bytes[4] = {0x12, 0x34, 0x56, 0x78};
    Property p("u32", bytes, Property::dt_uint32, Property::big);
    EXPECT_EQ(p.getValue<uint32_t>(), 0x12345678u);
  }
  // 8-byte
  {
    uint8_t  bytes[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Property p("u64", bytes, Property::dt_uint64, Property::big);
    EXPECT_EQ(p.getValue<uint64_t>(), 0x0102030405060708ull);
  }
}

// Stores v with its bytes reversed and reads it back through a big-endian
// Property, which must undo the reversal and recover v. Exercises the
// big-endian swap path of getValue<T>() for every multi-byte datatype.
template <typename T>
static void checkBigEndianRoundTrip(Property::datatype_t dt, T v)
{
  uint8_t        buf[sizeof(T)];
  const uint8_t* src = reinterpret_cast<const uint8_t*>(&v);
  for (size_t i = 0; i < sizeof(T); i++) buf[i] = src[sizeof(T) - 1 - i];
  Property p("x", buf, dt, Property::big);
  EXPECT_EQ(p.getValue<T>(), v);
}

TEST(Property, GetValueBigEndianRoundTripAllMultiByteTypes)
{
  checkBigEndianRoundTrip<uint16_t>(Property::dt_uint16, 0xABCD);
  checkBigEndianRoundTrip<uint32_t>(Property::dt_uint32, 0xDEADBEEFu);
  checkBigEndianRoundTrip<uint64_t>(Property::dt_uint64, 0x0123456789ABCDEFull);
  checkBigEndianRoundTrip<int16_t>(Property::dt_int16, -4321);
  checkBigEndianRoundTrip<int32_t>(Property::dt_int32, -987654);
  checkBigEndianRoundTrip<int64_t>(Property::dt_int64, -1234567890123ll);
  checkBigEndianRoundTrip<float>(Property::dt_float32, -12.5f);
  checkBigEndianRoundTrip<double>(Property::dt_float64, 6.0221409e23);
}

TEST(Property, GetValueIncompatibleSizeThrows)
{
  uint8_t  v = 0;
  Property p("u8", &v, Property::dt_uint8, Property::little);
  EXPECT_THROW(p.getValue<uint16_t>(), std::logic_error);
}

// ------------------------------------------------------------------
// Condition
// ------------------------------------------------------------------

TEST(Condition, GetOperatorType)
{
  EXPECT_EQ(Condition::getOperatorType("=="), Condition::op_equal);
  EXPECT_EQ(Condition::getOperatorType("!="), Condition::op_not_equal);
  EXPECT_EQ(Condition::getOperatorType(">"), Condition::op_greater);
  EXPECT_EQ(Condition::getOperatorType(">="), Condition::op_greater_or_equal);
  EXPECT_EQ(Condition::getOperatorType("<"), Condition::op_less);
  EXPECT_EQ(Condition::getOperatorType("<="), Condition::op_less_or_equal);
  EXPECT_EQ(Condition::getOperatorType("%0"), Condition::op_modulo_zero);
  EXPECT_EQ(Condition::getOperatorType("%N"), Condition::op_modulo_non_zero);
  EXPECT_EQ(Condition::getOperatorType("BitTrue"), Condition::op_bit_true);
  EXPECT_EQ(Condition::getOperatorType("BitFalse"), Condition::op_bit_false);
  EXPECT_THROW(Condition::getOperatorType("<<"), std::logic_error);
}

// Exercises the six comparison operators for an arbitrary type T, evaluating
// both the satisfied and unsatisfied case to cover both branches.
template <typename T>
static void checkComparisonOperators()
{
  const T lo = static_cast<T>(2);
  const T hi = static_cast<T>(5);

  EXPECT_TRUE(_vCondition<T>(Condition::op_equal, nullptr, nullptr, lo, lo).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_equal, nullptr, nullptr, lo, hi).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_not_equal, nullptr, nullptr, lo, hi).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_not_equal, nullptr, nullptr, lo, lo).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_greater, nullptr, nullptr, hi, lo).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_greater, nullptr, nullptr, lo, hi).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_greater_or_equal, nullptr, nullptr, hi, hi).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_greater_or_equal, nullptr, nullptr, lo, hi).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_less, nullptr, nullptr, lo, hi).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_less, nullptr, nullptr, hi, lo).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_less_or_equal, nullptr, nullptr, hi, hi).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_less_or_equal, nullptr, nullptr, hi, lo).evaluate());
}

// Exercises the bit and modulo operators (integer-only).
template <typename T>
static void checkBitAndModuloOperators()
{
  // bit index 2 of 0b00000100 (=4) is set
  EXPECT_TRUE(_vCondition<T>(Condition::op_bit_true, nullptr, nullptr, static_cast<T>(4), static_cast<T>(2)).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_bit_true, nullptr, nullptr, static_cast<T>(4), static_cast<T>(1)).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_bit_false, nullptr, nullptr, static_cast<T>(4), static_cast<T>(1)).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_bit_false, nullptr, nullptr, static_cast<T>(4), static_cast<T>(2)).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_modulo_zero, nullptr, nullptr, static_cast<T>(10), static_cast<T>(5)).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_modulo_zero, nullptr, nullptr, static_cast<T>(10), static_cast<T>(3)).evaluate());

  EXPECT_TRUE(_vCondition<T>(Condition::op_modulo_non_zero, nullptr, nullptr, static_cast<T>(10), static_cast<T>(3)).evaluate());
  EXPECT_FALSE(_vCondition<T>(Condition::op_modulo_non_zero, nullptr, nullptr, static_cast<T>(10), static_cast<T>(5)).evaluate());
}

TEST(Condition, ComparisonOperatorsAcrossTypes)
{
  checkComparisonOperators<uint8_t>();
  checkComparisonOperators<uint16_t>();
  checkComparisonOperators<uint32_t>();
  checkComparisonOperators<uint64_t>();
  checkComparisonOperators<int8_t>();
  checkComparisonOperators<int16_t>();
  checkComparisonOperators<int32_t>();
  checkComparisonOperators<int64_t>();
  checkComparisonOperators<float>();
  checkComparisonOperators<double>();
}

TEST(Condition, BitAndModuloOperatorsAcrossIntegerTypes)
{
  checkBitAndModuloOperators<uint8_t>();
  checkBitAndModuloOperators<uint16_t>();
  checkBitAndModuloOperators<uint32_t>();
  checkBitAndModuloOperators<uint64_t>();
  checkBitAndModuloOperators<int8_t>();
  checkBitAndModuloOperators<int16_t>();
  checkBitAndModuloOperators<int32_t>();
  checkBitAndModuloOperators<int64_t>();
}

TEST(Condition, EvaluatesAgainstPropertyOperands)
{
  uint32_t a = 7;
  uint32_t b = 7;
  Property pa("a", &a, Property::dt_uint32, Property::little);
  Property pb("b", &b, Property::dt_uint32, Property::little);

  // property-vs-property: 7 == 7
  EXPECT_TRUE(_vCondition<uint32_t>(Condition::op_equal, &pa, &pb, 0, 0).evaluate());

  b = 9;
  EXPECT_FALSE(_vCondition<uint32_t>(Condition::op_equal, &pa, &pb, 0, 0).evaluate());
  EXPECT_TRUE(_vCondition<uint32_t>(Condition::op_less, &pa, &pb, 0, 0).evaluate());

  // property-vs-immediate: only operand 1 bound to a property (7 > 5)
  EXPECT_TRUE(_vCondition<uint32_t>(Condition::op_greater, &pa, nullptr, 0, 5).evaluate());
}
