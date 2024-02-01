#pragma once

#include <metrohash128/metrohash128.h>
#include <sha1/sha1.hpp>
#include <string>
#include <cstdio>

namespace jaffarPlus
{

typedef _uint128_t hash_t;

inline hash_t calculateMetroHash(const void *data, size_t size)
{
  MetroHash128 hash;
  hash.Update(data, size);
  hash_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

inline std::string hashToString(const hash_t hash)
{
  // Creating hash string
  char hashStringBuffer[256];
  sprintf(hashStringBuffer, "0x%lX%lX", hash.first, hash.second);
  return std::string(hashStringBuffer);
}

inline hash_t hashString(const std::string& string) { return calculateMetroHash(string.data(), string.size()); }

}