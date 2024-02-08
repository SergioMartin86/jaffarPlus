#pragma once

#include <nesInstance.hpp>
#include <jaffarCommon/include/hash.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/logger.hpp>
#include <emulator.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerNES final : public Emulator
{
  public:

  static std::string getName() { return "QuickerNES"; } 

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
    bool status = jaffarCommon::loadStringFromFile(romFileData, _romFilePath.c_str());
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

  inline void serializeDifferentialState(
    uint8_t* __restrict__ outputData,
    size_t* outputDataPos,
    const size_t outputDataMaxSize,
    const uint8_t* __restrict__ referenceData,
    size_t* referenceDataPos,
    const size_t referenceDataMaxSize,
    const bool useZlib) const
  {
    _quickerNES.serializeDifferentialState(outputData, outputDataPos, outputDataMaxSize, referenceData, referenceDataPos, referenceDataMaxSize, useZlib);
  }

  inline void deserializeDifferentialState(
    const uint8_t* __restrict__ inputData,
    size_t* inputDataPos,
    const size_t inputDataMaxSize,
    const uint8_t* __restrict__ referenceData,
    size_t* referenceDataPos,
    const size_t referenceDataMaxSize,
    const bool useZlib)
  {
    _quickerNES.deserializeDifferentialState(inputData, inputDataPos, inputDataMaxSize, referenceData, referenceDataPos, referenceDataMaxSize, useZlib);
  }


  inline void printInfo() const override
  {
     printMemoryBlockHash("NES LRAM");
     printMemoryBlockHash("NES SRAM");
     printMemoryBlockHash("NES NTAB");
     printMemoryBlockHash("NES CHRR");
     printMemoryBlockHash("NES SPRT");
  }

  property_t getProperty(const std::string& propertyName) const override
  {
     if (propertyName == "NES LRAM") return property_t(_quickerNES.getLowMem(),       _quickerNES.getLowMemSize());
     if (propertyName == "NES SRAM") return property_t(_quickerNES.getWorkMem(),      _quickerNES.getWorkMemSize());
     if (propertyName == "NES NTAB") return property_t(_quickerNES.getNametableMem(), _quickerNES.getNametableMemSize());
     if (propertyName == "NES CHRR") return property_t(_quickerNES.getCHRMem(),       _quickerNES.getCHRMemSize());
     if (propertyName == "NES SPRT") return property_t(_quickerNES.getSpriteMem(),    _quickerNES.getSpriteMemSize());

     EXIT_WITH_ERROR("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());  
  }

  private:

  void printMemoryBlockHash(const std::string& blockName) const
  {
   auto p = getProperty(blockName);
   auto hash = jaffarCommon::hashToString(jaffarCommon::calculateMetroHash(p.pointer, p.size));
   LOG("[J+] %s Hash:        %s\n", blockName.c_str(), hash.c_str());
  }

  NESInstance _quickerNES;

  std::string _romFilePath;
  std::string _romFileSHA1;

};

} // namespace emulator

} // namespace jaffarPlus