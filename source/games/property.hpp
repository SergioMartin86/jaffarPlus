#pragma once

#include <string>
#include <cstring>
#include <common/hash.hpp>

namespace jaffarPlus
{


class Property
{
  public:

  enum datatype_t
  {
    dt_uint8, // Integer Little Endian
    dt_uint16,
    dt_uint32,
    dt_uint64,
    dt_int8,
    dt_int16,
    dt_int32,
    dt_int64,
    dt_bool, // Boolean
    dt_float32, // Single Precision Float (32-bit)
    dt_float64 // Double Precision Float (64-bit)
  };

  enum endianness_t
  {
    little,
    big
  };

  Property() = delete;
  Property(const std::string& name, void* const pointer, const datatype_t datatype, endianness_t endianness) :
  _name(name),
  _pointer(pointer),
  _datatype(datatype),
  _endianness(endianness),
  _nameHash(hashString(name))
  {
  }

  inline size_t getSize() const
  {
    switch (_datatype)
    {
      case dt_uint8    : return 1;        
      case dt_uint16   : return 2;      
      case dt_uint32   : return 4;      
      case dt_uint64   : return 8;
      case dt_int8     : return 1;    
      case dt_int16    : return 2;     
      case dt_int32    : return 4;     
      case dt_int64    : return 8;     
      case dt_bool     : return 1; 
      case dt_float32  : return 4;  
      case dt_float64  : return 8; 
    }

    EXIT_WITH_ERROR("Unidentified datatype %d\n", _datatype);
  }

  template <typename T> inline T getValue() const
  {
    // Otherwise convert to big endian
    const auto size = getSize();
    const auto bufferSize = sizeof(T);
    if (size != bufferSize) EXIT_WITH_ERROR("Incompatible datatypes while getting value of property '%s'", getName().c_str());

    // If its little endian, return value as is
    if (_endianness == endianness_t::little) return *(T*)_pointer;

    // Converting to big endian value byte by byte
    T value;
    const auto sBuf = (uint8_t*) _pointer;
    auto dBuf = (uint8_t*) &value;
    if (size == 1) { dBuf[0] = sBuf[0]; }
    if (size == 2) { dBuf[0] = sBuf[1]; dBuf[1] = sBuf[0]; }
    if (size == 4) { dBuf[0] = sBuf[3]; dBuf[1] = sBuf[2]; dBuf[2] = sBuf[1]; dBuf[3] = sBuf[0]; }
    if (size == 8) { dBuf[0] = sBuf[7]; dBuf[1] = sBuf[6]; dBuf[2] = sBuf[5]; dBuf[3] = sBuf[4]; dBuf[4] = sBuf[3]; dBuf[5] = sBuf[2]; dBuf[6] = sBuf[1]; dBuf[7] = sBuf[0]; }
    return value;
  }

  datatype_t getDatatype() const { return _datatype; }
  std::string getName() const { return _name; } 
  hash_t getNameHash() const { return _nameHash; }
  void* getPointer() const { return _pointer; }

  private:

  const std::string _name;
  void* const _pointer;
  const datatype_t _datatype;
  const endianness_t _endianness;
  const hash_t _nameHash;
};

} // namespace jaffarPlus
