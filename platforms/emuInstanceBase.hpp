#pragma once

#include <utils.hpp>
#include <string>

class EmuInstanceBase
{
  public:

  // Initializes the emulator instance
  EmuInstanceBase(const nlohmann::json& config) {};
  virtual ~EmuInstanceBase() = default;

  // Loading/Saving state files
  virtual void loadStateFile(const std::string& stateFilePath) = 0;
  virtual void saveStateFile(const std::string& stateFilePath) const = 0;

  // Function to advance state
  virtual void advanceState(const INPUT_TYPE move) = 0;

  // Minimal serialization functions
  virtual void serializeState(uint8_t* state) const  = 0;
  virtual void deserializeState(const uint8_t* state)  = 0;

  // Move parsing functions
  static inline INPUT_TYPE moveStringToCode(const std::string& move);
  static  inline std::string moveCodeToString(const INPUT_TYPE move);
};
