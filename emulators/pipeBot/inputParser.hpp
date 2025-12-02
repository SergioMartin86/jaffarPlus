#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>

namespace jaffar
{

  struct input_t
  {
    uint8_t type = 0;
  };


class InputParser
{

public:

  InputParser(const nlohmann::json &config)
  {
  }

  inline input_t parseInputString(const std::string& inputString) const
  {
    // Converting input into a stream for parsing
    std::istringstream ss(inputString);

    // Start separator
    if (ss.get() != '|') reportBadInputString(inputString);

    // Storage for the input
    input_t input;

    // Parsing console inputs
    parseInput(input, ss, inputString);

    // End separator
    if (ss.get() != '|') reportBadInputString(inputString);

    // If its not the end of the stream, then extra values remain and its invalid
    ss.get();
    if (ss.eof() == false) reportBadInputString(inputString);

    // Returning valid / input pair
    return input;
  }

  private:

  static inline void reportBadInputString(const std::string& inputString)
  {
    JAFFAR_THROW_LOGIC("Could not decode input string: '%s'\n", inputString.c_str());
  }

  static inline bool parseInput(input_t& input, std::istringstream& ss, const std::string& inputString)
  {
    char c = 0;

    // Getting piece type
    c = ss.get(); // Hundreds
    if (c != ' ') input.type += 100 * ( (uint8_t)c - 48 );

    c = ss.get(); // Tenths
    if (c != ' ') input.type += 10 * ( (uint8_t)c - 48 );

    c = ss.get(); // Units
    if (c != ' ') input.type += (uint8_t)c - 48;

    return true;
  }

}; // class InputParser

} // namespace jaffar