#pragma once

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <pthread.h>
#include <stdarg.h>
#include <iterator>
#include <limits.h>

namespace jaffarPlus
{

inline std::string simplifyMove(const std::string& move)
{
 std::string simpleMove;

 bool isEmptyMove = true;
 for (size_t i = 0; i < move.size(); i++) if (move[i] != '.' && move[i] != '|') { simpleMove += move[i]; isEmptyMove = false; }
 if (isEmptyMove) return ".";
 return simpleMove;
}

// Taken from https://stackoverflow.com/a/4956493
template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}


// Class for pthread lock implementation
class Mutex
{
 pthread_mutex_t _lock;

 public:

 Mutex()
 {
  pthread_mutex_init(&_lock, 0);
 }

 ~Mutex()
 {
  pthread_mutex_destroy(&_lock);
 }

 inline void lock()
 {
  pthread_mutex_lock(&_lock);
 }

 inline void unlock()
 {
  pthread_mutex_unlock(&_lock);
 }

 inline bool trylock()
 {
  return pthread_mutex_trylock(&_lock) == 0;
 }
};

inline bool getBitFlag(const uint8_t value, const uint8_t idx)
{
  if (((idx == 7) && (value & 0b10000000)) ||
      ((idx == 6) && (value & 0b01000000)) ||
      ((idx == 5) && (value & 0b00100000)) ||
      ((idx == 4) && (value & 0b00010000)) ||
      ((idx == 3) && (value & 0b00001000)) ||
      ((idx == 2) && (value & 0b00000100)) ||
      ((idx == 1) && (value & 0b00000010)) ||
      ((idx == 0) && (value & 0b00000001))) return true;
  return false;
}


inline size_t countButtonsPressedString(const std::string& input) { size_t count = 0; for (size_t i = 0; i < input.size(); i++) if (input[i] != '.') count++; return count; };

template<typename T> inline uint16_t countButtonsPressedNumber(const T& input) {
 uint16_t count = 0;
 if (input & 0b0000000000000001) count++;
 if (input & 0b0000000000000010) count++;
 if (input & 0b0000000000000100) count++;
 if (input & 0b0000000000001000) count++;
 if (input & 0b0000000000010000) count++;
 if (input & 0b0000000000100000) count++;
 if (input & 0b0000000001000000) count++;
 if (input & 0b0000000010000000) count++;
 if (input & 0b0000000100000000) count++;
 if (input & 0b0000001000000000) count++;
 if (input & 0b0000010000000000) count++;
 if (input & 0b0000100000000000) count++;
 if (input & 0b0001000000000000) count++;
 if (input & 0b0010000000000000) count++;
 if (input & 0b0100000000000000) count++;
 if (input & 0b1000000000000000) count++;
 return count;
};

static auto moveCountComparerString = [](const std::string& a, const std::string& b) { return countButtonsPressedString(a) < countButtonsPressedString(b); };
static auto moveCountComparerNumber = [](const uint64_t a, const uint64_t b) { return countButtonsPressedNumber(a) < countButtonsPressedNumber(b); };

// Function to split a vector into n mostly fair chunks
template <typename T>
std::vector<T> splitVector(const T size, const T n)
{
  std::vector<T> subSizes(n);

  T length = size / n;
  T remain = size % n;

  for (T i = 0; i < n; i++)
    subSizes[i] = i < remain ? length + 1 : length;

  return subSizes;
}

// Function to split a string into a sub-strings delimited by a character
// Taken from stack overflow answer to https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
// By Evan Teran

template <typename Out>
inline void split(const std::string &s, char delim, Out result)
{
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim))
  {
    *result++ = item;
  }
}

static inline std::vector<std::string> split(const std::string &s, char delim)
{
 std::string newString = s;
 std::replace(newString.begin(), newString.end(), '\n', ' ');
 std::vector<std::string> elems;
 split(newString, delim, std::back_inserter(elems));
 return elems;
}

// Taken from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c/116220#116220
static inline std::string slurp(std::ifstream &in)
{
  std::ostringstream sstr;
  sstr << in.rdbuf();
  return sstr.str();
}

static inline bool loadStringFromFile(std::string &dst, const std::string& fileName)
{
  std::ifstream fi(fileName);

  // If file not found or open, return false
  if (fi.good() == false) return false;

  // Reading entire file
  dst = slurp(fi);

  // Closing file
  fi.close();

  return true;
}

// Save string to a file
static inline bool saveStringToFile(const std::string &src, const std::string& fileName)
{
  FILE *fid = fopen(fileName.c_str(), "w");
  if (fid != NULL)
  {
    fwrite(src.c_str(), 1, src.size(), fid);
    fclose(fid);
    return true;
  }
  return false;
}

} // namespace jaffarPlus
