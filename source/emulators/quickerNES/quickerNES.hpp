#pragma once

#include <nesInstance.hpp>
#include <common/hash.hpp>
#include <common/json.hpp>
#include <common/utils.hpp>
#include <common/logger.hpp>
#include <emulators/emulator.hpp>

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
  void advanceState(const std::string& move) override
  {
    _quickerNES.advanceState(move);
  }

  size_t getStateSize() const override
  {
   return _quickerNES.getStateSize();
  };
  
  inline void serializeState(uint8_t *state) const override
  {
    _quickerNES.serializeState(state);
  };
  
  inline void deserializeState(const uint8_t *state) override
  {
    _quickerNES.deserializeState(state);
  };

  inline void printDebugInformation() const override
  {
   auto lowMem = getProperty("RAM");
   auto lowMemHashString = hashToString(calculateMetroHash(lowMem.first, lowMem.second));
   LOG("[J+] Final State Hash:        %s\n", lowMemHashString.c_str());
  }

  property_t getProperty(const std::string& propertyName) const override
  {
     if (propertyName == "RAM") return property_t(_quickerNES.getLowMem(), _quickerNES.getLowMemSize());

     EXIT_WITH_ERROR("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());  
  }

  inline std::string getName() const override { return "QuickerNES"; } 

  private:

  NESInstance _quickerNES;

  std::string _romFilePath;
  std::string _romFileSHA1;

};

} // namespace jaffarPlus