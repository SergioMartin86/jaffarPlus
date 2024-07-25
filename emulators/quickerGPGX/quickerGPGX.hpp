#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <emulator.hpp>
#include <gpgxInstance.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerGPGX final : public Emulator
{
  public:

  static std::string getName() { return "QuickerGPGX"; }

  // Constructor must only do configuration parsing
  QuickerGPGX(const nlohmann::json &config)
    : Emulator(config)
  {
    // Getting initial state file from the configuration
    _initialStateFilePath = jaffarCommon::json::getString(config, "Initial State File Path");

    // For testing purposes, the initial state file can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERGPGX_OVERRIDE_INITIAL_STATE_FILE_PATH")) _initialStateFilePath = std::string(value);

    // Parsing controller configuration
    _systemType      = jaffarCommon::json::getString(config, "System Type");
    _controller1Type = jaffarCommon::json::getString(config, "Controller 1 Type");
    _controller2Type = jaffarCommon::json::getString(config, "Controller 2 Type");

    // Parsing rom file path
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");

    // For testing purposes, the rom file path can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERGPGX_OVERRIDE_ROM_FILE_PATH")) _romFilePath = std::string(value);

    // Parsing rom file SHA1
    _romFileSHA1 = jaffarCommon::json::getString(config, "Rom File SHA1");

    // For testing purposes, the rom file SHA1 can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERGPGX_OVERRIDE_ROM_FILE_SHA1")) _romFileSHA1 = std::string(value);
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _quickerGPGX.initialize();

    // Setting controller types
    _quickerGPGX.setController1Type(_controller1Type);
    _quickerGPGX.setController2Type(_controller2Type);

    // Reading from ROM file
    std::string romFileData;
    bool        status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = jaffarCommon::hash::getSHA1String(romFileData);
    if (_romFileSHA1 != actualRomSHA1)
      JAFFAR_THROW_LOGIC("ROM file: '%s' expected SHA1 ('%s') does not concide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());

    // Loading rom into emulator
    _quickerGPGX.loadROM(_romFilePath);

    // If initial state file defined, load it
    if (_initialStateFilePath.empty() == false)
    {
      // Reading from initial state file
      std::string initialState;
      bool        success = jaffarCommon::file::loadStringFromFile(initialState, _initialStateFilePath);
      if (success == false) JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial state file: %s\n", _initialStateFilePath.c_str());

      // Deserializing initial state into the emulator
      enableStateProperties();
      jaffarCommon::deserializer::Contiguous d(initialState.data(), initialState.size());
      deserializeState(d);
    }

    // Now disabling state properties, as requested
    disableStateProperties();
  }

  // State advancing function
  void advanceState(const std::string &input) override { _quickerGPGX.advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _quickerGPGX.serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _quickerGPGX.deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "RAM") return property_t(_quickerGPGX.getWorkRamPointer(), _quickerGPGX.getWorkRamSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void enableStateProperty(const std::string &property) override { _quickerGPGX.enableStateBlock(property); }

  __INLINE__ void disableStateProperty(const std::string &property) override { _quickerGPGX.disableStateBlock(property); }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    enableStateProperties();
    _quickerGPGX.initializeVideoOutput();
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerGPGX.finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerGPGX.enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerGPGX.disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override { serializeState(serializer); }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override { deserializeState(deserializer); }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerGPGX.updateRenderer(); }

  private:

  gpgx::EmuInstance _quickerGPGX;

  std::string _systemType;
  std::string _controller1Type;
  std::string _controller2Type;
  std::string _romFilePath;
  std::string _romFileSHA1;
  std::string _initialStateFilePath;
};

} // namespace emulator

} // namespace jaffarPlus