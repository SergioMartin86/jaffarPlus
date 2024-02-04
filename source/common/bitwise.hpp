#pragma once

#include <stddef.h>
#include <cstdint>

namespace jaffarPlus
{
  
  uint8_t bitMaskTable[8] =
  {
    0b00000001,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000
  };

  uint8_t bitNotMaskTable[8] =
  {
    0b11111110,
    0b11111101,
    0b11111011,
    0b11110111,
    0b11101111,
    0b11011111,
    0b10111111,
    0b01111111
  };

  void bitcopy(uint8_t* dstBuffer, const size_t dstOffset, const uint8_t* srcBuffer, const size_t srcOffset, const size_t count, const size_t elementBitSize )
  {
    const size_t totalBitCount = count * elementBitSize;
    const size_t dstOffsetBits = dstOffset * elementBitSize;
    const size_t srcOffsetBits = srcOffset * elementBitSize;
    size_t dstPosByte = dstOffsetBits / 8;
    uint8_t dstPosBit = dstOffsetBits % 8;
    size_t srcPosByte = srcOffsetBits / 8;
    uint8_t srcPosBit = srcOffsetBits % 8;

    for (size_t i = 0; i < totalBitCount; i++)
    {
      // Clear bit in question
      dstBuffer[dstPosByte] = dstBuffer[dstPosByte] & bitNotMaskTable[dstPosBit];

      // If the corresponding bit is set in source, set it up in dst
      if ((srcBuffer[srcPosByte] & bitMaskTable[srcPosBit]) > 0) dstBuffer[dstPosByte] = dstBuffer[dstPosByte] | bitMaskTable[dstPosBit];

      // Advance bit positions
      dstPosBit++;
      srcPosBit++;

      // If crossed a byte barrier, go over the next byte
      if (dstPosBit == 8) { dstPosBit = 0; dstPosByte++; }
      if (srcPosBit == 8) { srcPosBit = 0; srcPosByte++; }
    }
  }

} // namespace jaffarPlus