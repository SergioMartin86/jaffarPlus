#pragma once

#include <string>
#include <jaffarCommon/include/file.hpp>
#include <jaffarCommon/include/json.hpp>

namespace jaffarPlus
{

// A property is a contiguous segment of memory with size, identifiable by name 
struct property_t { uint8_t* pointer; size_t size; };

class Emulator
{
  public:

  // Constructor must only do configuration parsing to perform dry runs
  Emulator(const nlohmann::json& config) {};
  virtual ~Emulator() = default;

  // Initialization function
  virtual void initialize() = 0;

  // State advancing function
  virtual void advanceState(const std::string& move) = 0;

  // State file load / save functions
  inline void loadStateFile(const std::string &stateFilePath)
  {
    std::string stateData;
    bool status = jaffarCommon::loadStringFromFile(stateData, stateFilePath.c_str());
    if (status == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());
    deserializeState((uint8_t *)stateData.data());
  }

  inline void saveStateFile(const std::string &stateFilePath) const
  {
    std::string stateData;
    stateData.resize(getStateSize());
    serializeState((uint8_t *)stateData.data());
    jaffarCommon::saveStringToFile(stateData, stateFilePath.c_str());
  }

  // State serialization / deserialization functions
  virtual size_t getStateSize() const = 0;

  virtual void serializeState(uint8_t *state) const = 0;
  virtual void deserializeState(const uint8_t *state) = 0;

  virtual void serializeDifferentialState(
    uint8_t* __restrict__ outputData,
    size_t* outputDataPos,
    const size_t outputDataMaxSize,
    const uint8_t* __restrict__ referenceData,
    size_t* referenceDataPos,
    const size_t referenceDataMaxSize,
    const bool useZlib) const = 0;

  virtual void deserializeDifferentialState(
    const uint8_t* __restrict__ inputData,
    size_t* inputDataPos,
    const size_t inputDataMaxSize,
    const uint8_t* __restrict__ referenceData,
    size_t* referenceDataPos,
    const size_t referenceDataMaxSize,
    const bool useZlib) = 0;

  // Function to print debug information, whatever it might be
  virtual void printInfo() const = 0;

  // Get a property by name
  virtual property_t getProperty(const std::string& propertyName) const = 0;

  // Function to obtain emulator based on name
  static std::unique_ptr<Emulator> getEmulator(const std::string& emulatorName, const nlohmann::json& config);
};

} // namespace jaffarPlus