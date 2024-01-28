#pragma once

#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>

// If we use NCurses, we need to use the appropriate printing function
#ifdef NCURSES
  #include <ncurses.h>
  #define LOG printw
#else
  #define LOG printf
#endif

namespace jaffarPlus
{
  
#ifdef EXIT_WITH_ERROR
#undef EXIT_WITH_ERROR
#endif 

#define EXIT_WITH_ERROR(...) jaffarPlus::exitWithError(__FILE__, __LINE__, __VA_ARGS__)
inline void exitWithError [[noreturn]] (const char *fileName, const int lineNumber, const char *format, ...)
{
  char *outstr = 0;
  va_list ap;
  va_start(ap, format);
  int ret = vasprintf(&outstr, format, ap);
  if (ret < 0) exit(-1);

  std::string outString = outstr;
  free(outstr);

  char info[1024];

  snprintf(info, sizeof(info) - 1, " + From %s:%d\n", fileName, lineNumber);
  outString += info;

  throw std::runtime_error(outString.c_str());
}

} // namespace jaffarPlus