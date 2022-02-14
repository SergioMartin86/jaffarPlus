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

  // Function to advance frame
  virtual void advanceFrame(const uint8_t &move) = 0;
  virtual void advanceFrame(const std::string& move) = 0;

  // Minimal serialization functions
  virtual void serializeState(uint8_t* state) const  = 0;
  virtual void deserializeState(const uint8_t* state)  = 0;
};
