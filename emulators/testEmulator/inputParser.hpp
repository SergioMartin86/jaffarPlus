#pragma once

#include <cstdint>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>
#include <sstream>
#include <string>

namespace jaffar
{

// A test-emulator input is a single cursor move. The input string format is a
// single character framed by pipes, e.g. "|U|", "|D|", "|L|", "|R|", "|.|".
struct input_t
{
  // 0 = none, 1 = up, 2 = down, 3 = left, 4 = right
  uint8_t move = 0;
};

class InputParser
{
public:
  InputParser(const nlohmann::json& config) {}

  inline input_t parseInputString(const std::string& inputString) const
  {
    std::istringstream ss(inputString);

    // Start separator
    if (ss.get() != '|') reportBadInputString(inputString);

    // Decoding the move character
    input_t    input;
    const char c = (char)ss.get();
    switch (c)
    {
      case '.': input.move = 0; break;
      case 'U': input.move = 1; break;
      case 'D': input.move = 2; break;
      case 'L': input.move = 3; break;
      case 'R': input.move = 4; break;
      default: reportBadInputString(inputString);
    }

    // End separator
    if (ss.get() != '|') reportBadInputString(inputString);

    // No trailing characters allowed
    ss.get();
    if (ss.eof() == false) reportBadInputString(inputString);

    return input;
  }

private:
  static inline void reportBadInputString(const std::string& inputString) { JAFFAR_THROW_LOGIC("Could not decode input string: '%s'\n", inputString.c_str()); }

}; // class InputParser

} // namespace jaffar
