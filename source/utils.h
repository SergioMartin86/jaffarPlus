#pragma once

#include "nlohmann/json.hpp"
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

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

char * path_dirname(const char *path);
char * basename_no_extension(const char *path);
char *path_extension(char const *path);
char is_path_sep(char c);
extern bool saveBinToFile(const uint8_t* src, size_t size, const char *fileName);
extern bool loadBinFromFile(uint8_t* dst, size_t size, const char *fileName);
