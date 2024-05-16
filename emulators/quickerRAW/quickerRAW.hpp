#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <mutex>
#include <emulator.hpp>
#include <RAWInstance.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerRAW final : public Emulator
{
  public:

  static std::string getName() { return "QuickerRAW"; }

  // Constructor must only do configuration parsing
  QuickerRAW(const nlohmann::json &config)
    : Emulator(config)
  {
    // Getting disabled state properties
    const auto disabledStateProperties = jaffarCommon::json::getArray<std::string>(config, "Disabled State Properties");
    for (const auto &property : disabledStateProperties) _disabledStateProperties.push_back(property);

    // Getting initial state file from the configuration
    _initialStateFilePath = jaffarCommon::json::getString(config, "Initial State File Path");

    // Parsing rom file path
    _gameDataPath = jaffarCommon::json::getString(config, "Game Data Path");
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _mutex.lock();
    _quickerRAW.initialize(_gameDataPath);

    // If initial state file defined, load it
    if (_initialStateFilePath.empty() == false)
    {
      // Reading from initial state file
      std::string initialState;
      bool        success = jaffarCommon::file::loadStringFromFile(initialState, _initialStateFilePath);
      if (success == false) JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial state file: %s\n", _initialStateFilePath.c_str());

      // Deserializing initial state into the emulator
      loadFullState(initialState);
    }

    _mutex.unlock();

    // Now disabling state properties, as requested
    disableStateProperties();
  }

  // State advancing function
  void advanceState(const std::string &input) override { _quickerRAW.advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _quickerRAW.serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _quickerRAW.deserializeState(deserializer); };

  __INLINE__ void disableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) disableStateProperty(property);
  }
  __INLINE__ void enableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) enableStateProperty(property);
  }

  __INLINE__ void loadFullState(const std::string &state) override
  {
    enableStateProperties();
    jaffarCommon::deserializer::Contiguous d(state.data(), state.size());
    deserializeState(d);
    disableStateProperties();
  }
  __INLINE__ void saveFullState(std::string &state) override
  {
    enableStateProperties();
    jaffarCommon::serializer::Contiguous s(state.data(), state.size());
    serializeState(s);
    disableStateProperties();
  }

  size_t getFullStateSize() override
  {
    enableStateProperties();
    jaffarCommon::serializer::Contiguous s;
    serializeState(s);
    disableStateProperties();
    return s.getOutputSize();
  }

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "RAM") return property_t(_quickerRAW.getRamPointer(), 512);
    if (propertyName == "Threads Data") return property_t((uint8_t*)_quickerRAW.getThreadsData(), _quickerRAW.getThreadsDataSize());
    if (propertyName == "Script Stack Data") return property_t((uint8_t*)_quickerRAW.getScriptStackData(), _quickerRAW.getScriptStackDataSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void enableStateProperty(const std::string &property) { _quickerRAW.enableStateBlock(property); }

  __INLINE__ void disableStateProperty(const std::string &property) { _quickerRAW.disableStateBlock(property); }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    // enableStateProperties();
    _quickerRAW.initializeVideoOutput();
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerRAW.finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerRAW.enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerRAW.disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override
  {

  }

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override
  {
    serializeState(serializer);
    serializer.push(_quickerRAW.getPixelsPtr(), _quickerRAW.getPixelsSize());
    serializer.push(_quickerRAW.getPalettePtr(), _quickerRAW.getPaletteSize());
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(_quickerRAW.getPixelsPtr(), _quickerRAW.getPixelsSize());
    deserializer.pop(_quickerRAW.getPalettePtr(), _quickerRAW.getPaletteSize());
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerRAW.updateRenderer(); }

  private:

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;

  rawspace::EmuInstance _quickerRAW;

  std::string _initialStateFilePath;
  std::string _gameDataPath;
  std::mutex _mutex;

};

} // namespace emulator

} // namespace jaffarPlus