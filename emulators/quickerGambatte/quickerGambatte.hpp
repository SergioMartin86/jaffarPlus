#pragma once

#include <emuInstance.hpp>
#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <memory>

namespace jaffarPlus
{

namespace emulator
{

class QuickerGambatte final : public Emulator
{
public:
  static std::string getName() { return "QuickerGambatte"; }

  // Constructor must only do configuration parsing
  QuickerGambatte(const nlohmann::json& config) : Emulator(config)
  {
    // Getting initial state file from the configuration
    _initialStateFilePath = jaffarCommon::json::getString(config, "Initial State File Path");

    // For testing purposes, the initial state file can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERGAMBATTE_OVERRIDE_INITIAL_STATE_FILE_PATH")) _initialStateFilePath = std::string(value);

    // Getting initial sequence file path
    _initialSequenceFilePath = jaffarCommon::json::getString(config, "Initial Sequence File Path");

    // Parsing rom file path
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");

    // For testing purposes, the rom file path can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERGAMBATTE_OVERRIDE_ROM_FILE_PATH")) _romFilePath = std::string(value);

    // Parsing rom file SHA1
    _romFileSHA1 = jaffarCommon::json::getString(config, "Rom File SHA1");

    // For testing purposes, the rom file SHA1 can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_QUICKERGAMBATTE_OVERRIDE_ROM_FILE_SHA1")) _romFileSHA1 = std::string(value);

    // Getting disabled state properties
    const auto disabledStateProperties = jaffarCommon::json::getArray<std::string>(config, "Disabled State Properties");
    for (const auto& property : disabledStateProperties) _disabledStateProperties.push_back(property);

    // Creating internal emulator instance
    _quickerGambatte = std::make_unique<jaffar::EmuInstance>(config);
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _quickerGambatte->initialize();

    // Reading from ROM file
    std::string romFileData;
    bool        status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = jaffarCommon::hash::getSHA1String(romFileData);
    if (_romFileSHA1 != actualRomSHA1)
      JAFFAR_THROW_LOGIC("ROM file: '%s' expected SHA1 ('%s') does not concide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());

    if (_initialStateFilePath != "")
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

    // Getting input parser from the internal emulator
    const auto inputParser = _quickerGambatte->getInputParser();

    // Advancing the state using the initial sequence, if provided
    if (_initialSequenceFilePath != "")
    {
      // Load initial sequence
      std::string initialSequenceFileString;
      if (jaffarCommon::file::loadStringFromFile(initialSequenceFileString, _initialSequenceFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial sequence file: %s\n", _initialSequenceFilePath.c_str());

      // Getting input sequence
      const auto initialSequence = jaffarCommon::string::split(initialSequenceFileString, '\0');

      // Running inputs in the initial sequence
      for (const auto& inputString : initialSequence) advanceStateImpl(inputParser->parseInputString(inputString));
    }
  }

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override { return _quickerGambatte->getInputParser(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override { _quickerGambatte->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override { _quickerGambatte->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override { _quickerGambatte->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string& propertyName) const override
  {
    auto memoryAreas = _quickerGambatte->getMemoryAreas();
    auto memorySizes = _quickerGambatte->getMemorySizes();

    if (propertyName == "VRAM") return property_t((uint8_t*)memoryAreas.vram, memorySizes.vram);
    if (propertyName == "ROM") return property_t((uint8_t*)memoryAreas.rom, memorySizes.rom);
    if (propertyName == "WRAM") return property_t((uint8_t*)memoryAreas.wram, memorySizes.wram);
    if (propertyName == "CARTRAM") return property_t((uint8_t*)memoryAreas.cartram, memorySizes.cartram);
    if (propertyName == "OAM") return property_t((uint8_t*)memoryAreas.oam, memorySizes.oam);
    if (propertyName == "HRAM") return property_t((uint8_t*)memoryAreas.hram, memorySizes.hram);

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void enableStateProperty(const std::string& property) { _quickerGambatte->enableStateBlock(property); }

  __INLINE__ void disableStateProperty(const std::string& property) { _quickerGambatte->disableStateBlock(property); }

  __INLINE__ void disableStateProperties()
  {
    for (const auto& property : _disabledStateProperties) disableStateProperty(property);
  }
  __INLINE__ void enableStateProperties()
  {
    for (const auto& property : _disabledStateProperties) enableStateProperty(property);
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    enableStateProperties();
    _quickerGambatte->initializeVideoOutput();
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerGambatte->finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerGambatte->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerGambatte->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base& serializer) const override
  {
    // Getting video buffer size
    serializeState(serializer);
    serializer.push(_quickerGambatte->getVideoBufferPtr(), _quickerGambatte->getVideoBufferSize());
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(_quickerGambatte->getVideoBufferPtr(), _quickerGambatte->getVideoBufferSize());
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerGambatte->updateRenderer(); }

private:
  std::unique_ptr<jaffar::EmuInstance> _quickerGambatte;

  std::string _romFilePath;
  std::string _romFileSHA1;
  std::string _initialStateFilePath;
  std::string _initialSequenceFilePath;

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;
};

} // namespace emulator

} // namespace jaffarPlus