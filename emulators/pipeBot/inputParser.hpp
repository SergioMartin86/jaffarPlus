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
    uint8_t row = 0;
    uint8_t col = 0;
  };


class InputParser
{

public:

  InputParser(const nlohmann::json &config)
  {
    _rowCount = jaffarCommon::json::getNumber<uint8_t>(config, "Row Count");
    _colCount = jaffarCommon::json::getNumber<uint32_t>(config, "Col Count");
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

    // Check for boundaries
    if (input.row >= _rowCount)  JAFFAR_THROW_LOGIC("Input row (%u) exceeds that of the number of rows (%u) when decoding string: '%s'\n", input.row, _rowCount, inputString.c_str());
    if (input.col >= _colCount)  JAFFAR_THROW_LOGIC("Input col (%u) exceeds that of the number of rows (%u) when decoding string: '%s'\n", input.col, _colCount, inputString.c_str());

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

    // Getting row
    c = ss.get(); // Hundreds
    if (c != ' ') input.row += 100 * ( (uint8_t)c - 48 );

    c = ss.get(); // Tenths
    if (c != ' ') input.row += 10 * ( (uint8_t)c - 48 );

    c = ss.get(); // Units
    if (c != ' ') input.row += (uint8_t)c - 48;

    ss.get(); // Comma

    // Getting Column
    c = ss.get(); // Hundreds
    if (c != ' ') input.col += 100 * ( (uint8_t)c - 48 );

    c = ss.get(); // Tenths
    if (c != ' ') input.col += 10 * ( (uint8_t)c - 48 );

    c = ss.get(); // Units
    if (c != ' ') input.col += (uint8_t)c - 48;

    return true;
  }

  uint8_t _rowCount;
  uint8_t _colCount;

}; // class InputParser

} // namespace jaffar