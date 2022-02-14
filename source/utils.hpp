#pragma once

#include <climits>
#include "nlohmann/json.hpp"
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <pthread.h>

// If we use NCurses, we need to use the appropriate printing function
#ifdef NCURSES
 #include <ncurses.h>
 #define LOG printw
#else
 #define LOG printf
#endif

// Function to split a string into a sub-strings delimited by a character
// Taken from stack overflow answer to https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
// By Evan Teran

template <typename Out>
void split(const std::string &s, char delim, Out result)
{
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim))
  {
    *result++ = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim);

// Logging functions
void exitWithError [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...);

#define EXIT_WITH_ERROR(...) \
  exitWithError(__FILE__, __LINE__, __VA_ARGS__)

// Checks if directory exists
bool dirExists(const std::string dirPath);

// Loads a string from a given file
bool loadStringFromFile(std::string &dst, const char *fileName);

// Save string to a file
bool saveStringToFile(const std::string &dst, const char *fileName);

#pragma GCC diagnostic ignored "-Wparentheses"

// Checks whether a given key is present in the JSON object.
template <typename T, typename... Key>
bool isDefined(T &js, const Key &... key)
{
  auto *tmp = &js;
  bool result = true;
  decltype(tmp->begin()) it;
  ((result && ((it = tmp->find(key)) == tmp->end() ? (result = false) : (tmp = &*it, true))), ...);
  return result;
}

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

// Taken from https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c/116220#116220
std::string slurp(std::ifstream &in);

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
