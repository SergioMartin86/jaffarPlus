#pragma once

#include <utils.hpp>
#include <emulators/emulator.hpp>
#include <quickerNES/emuInstance.hpp>

namespace jaffarPlus
{

class QuickerNES : public Emulator
{
  public:

  // Constructor must only do configuration parsing
  QuickerNES(const nlohmann::json& config) : Emulator(config)
  {

  };

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

  virtual size_t getStateSize() const override
  {
    return 0;
  };
  
  void serializeState(uint8_t *state) const override
  {

  };
  
  void deserializeState(const uint8_t *state) override
  {

  };

  static inline input_t moveStringToCode(const std::string& move)
  {

  }

  static inline std::string moveCodeToString(const input_t move)
  {

  }

  private:

  std::string romFilePath;

};

} // namespace jaffarPlus