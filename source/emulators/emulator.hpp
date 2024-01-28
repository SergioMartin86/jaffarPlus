#pragma once

#include <utils.hpp>
#include <nlohmann/json.hpp>

namespace jaffarPlus
{

class Emulator
{
  public:

  typedef uint64_t input_t;

  // Constructor must only do configuration parsing
  Emulator(const nlohmann::json& config) {};
  virtual ~Emulator() = default;

  // State advancing function
  virtual void advanceState(const std::string &move) = 0;

  // State file load / save functions
  inline void loadStateFile(const std::string &stateFilePath)
  {
    std::string stateData;
    bool status = loadStringFromFile(stateData, stateFilePath.c_str());
    if (status == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());
    deserializeState((uint8_t *)stateData.data());
  }

  inline void saveStateFile(const std::string &stateFilePath) const
  {
    std::string stateData;
    stateData.resize(getStateSize());
    serializeState((uint8_t *)stateData.data());
    saveStringToFile(stateData, stateFilePath.c_str());
  }

  // State serialization / deserialization functions
  virtual size_t getStateSize() const = 0;
  virtual void serializeState(uint8_t *state) const = 0;
  virtual void deserializeState(const uint8_t *state) = 0;

  // Move parsing functions
  static inline input_t moveStringToCode(const std::string& move);
  static inline std::string moveCodeToString(const input_t move);
};

} // namespace jaffarPlus