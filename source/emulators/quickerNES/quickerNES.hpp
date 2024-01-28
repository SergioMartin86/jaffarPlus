#pragma once

#include <sha1/sha1.hpp>
#include <common/json.hpp>
#include <emulators/emulator.hpp>
#include <quickerNES/nesInstance.hpp>
#include <quickerNES/nesInstance.hpp>

namespace jaffarPlus
{

class QuickerNES final : public Emulator
{
  public:

  // Constructor must only do configuration parsing
  QuickerNES(const nlohmann::json& config) : Emulator(config)
  {
    // Parsing controller configuration
    auto controller1Type = JSON_GET_STRING(config, "Controller 1 Type");
    auto controller2Type = JSON_GET_STRING(config, "Controller 2 Type");

    _quickerNES.setController1Type(controller1Type);
    _quickerNES.setController2Type(controller2Type);

    // Parsing rom file path
    _romFilePath = JSON_GET_STRING(config, "Rom File Path");

    // Parsing rom file SHA1
    _romFileSHA1 = JSON_GET_STRING(config, "Rom File SHA1");
  };

  void initialize() override
  {
    // Reading from ROM file
    std::string romFileData;
    bool status = loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) EXIT_WITH_ERROR("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = SHA1::GetHash((uint8_t *)romFileData.data(), romFileData.size());
    if (_romFileSHA1 != actualRomSHA1) EXIT_WITH_ERROR("ROM file: '%s' expected SHA1 ('%s') does not concide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());  

    // Loading rom into emulator
    _quickerNES.loadROM((uint8_t*)romFileData.data(), romFileData.size());
  }

  // State advancing function
  void advanceState(const std::string &move) override
  {

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
   return 0;
  }

  static inline std::string moveCodeToString(const input_t move)
  {
    return 0;
  }

  inline std::string getName() const override { return "QuickerNES"; } 

  private:

  NESInstance _quickerNES;

  std::string _romFilePath;
  std::string _romFileSHA1;

};

} // namespace jaffarPlus