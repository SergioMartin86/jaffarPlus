#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <mutex>
#include <emulator.hpp>
#include <stellaInstance.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerStella final : public Emulator
{
  public:

  static std::string getName() { return "QuickerStella"; }

  // Constructor must only do configuration parsing
  QuickerStella(const nlohmann::json &config)
    : Emulator(config)
  {
    // Parsing rom file path
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");

    // For testing purposes, the rom file path can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERSTELLA_OVERRIDE_ROM_FILE_PATH")) _romFilePath = std::string(value);

    // Parsing rom file SHA1
    _romFileSHA1 = jaffarCommon::json::getString(config, "Rom File SHA1");

    // For testing purposes, the rom file SHA1 can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_QUICKERSTELLA_OVERRIDE_ROM_FILE_SHA1")) _romFileSHA1 = std::string(value);

    // Getting disabled state properties
    const auto disabledStateProperties = jaffarCommon::json::getArray<std::string>(config, "Disabled State Properties");
    for (const auto &property : disabledStateProperties) _disabledStateProperties.push_back(property);

    // Creating internal emulator instance
    _quickerStella = std::make_unique<stella::EmuInstance>(config);
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _mutex.lock();
    _quickerStella->initialize();
    _mutex.unlock();

    // Reading from ROM file
    std::string romFileData;
    bool        status = jaffarCommon::file::loadStringFromFile(romFileData, _romFilePath.c_str());
    if (status == false) JAFFAR_THROW_LOGIC("Could not find/read from ROM file: %s\n", _romFilePath.c_str());

    // Getting SHA1 of ROM for checksum
    auto actualRomSHA1 = jaffarCommon::hash::getSHA1String(romFileData);
    if (_romFileSHA1 != actualRomSHA1)
      JAFFAR_THROW_LOGIC("ROM file: '%s' expected SHA1 ('%s') does not concide with the one read ('%s')\n", _romFilePath.c_str(), _romFileSHA1.c_str(), actualRomSHA1.c_str());

    // Loading rom into emulator
    _quickerStella->loadROM(_romFilePath);
  }

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser *getInputParser() const override { return _quickerStella->getInputParser(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t &input) override { _quickerStella->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _quickerStella->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _quickerStella->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "RAM") return property_t(_quickerStella->getWorkRamPointer(), _quickerStella->getWorkRamSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void disableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) disableStateProperty(property);
  }

  __INLINE__ void enableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) enableStateProperty(property);
  }

  __INLINE__ void enableStateProperty(const std::string &property) { _quickerStella->enableStateBlock(property); }

  __INLINE__ void disableStateProperty(const std::string &property) { _quickerStella->disableStateBlock(property); }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    enableStateProperties();
    _quickerStella->initializeVideoOutput();
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerStella->finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerStella->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerStella->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override { serializeState(serializer); }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override { deserializeState(deserializer); }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerStella->updateRenderer(); }

  private:

  std::unique_ptr<stella::EmuInstance> _quickerStella;

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;

  std::string _romFilePath;
  std::string _romFileSHA1;

  // Mutex required to avoid crashes during initialization
  std::mutex _mutex;
};

} // namespace emulator

} // namespace jaffarPlus