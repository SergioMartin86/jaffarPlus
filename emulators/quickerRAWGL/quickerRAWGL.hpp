#pragma once

#include <RAWGLInstance.hpp>
#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <mutex>

namespace jaffarPlus
{

namespace emulator
{

class QuickerRAWGL final : public Emulator
{
public:
  static std::string getName() { return "QuickerRAWGL"; }

  // Constructor must only do configuration parsing
  QuickerRAWGL(const nlohmann::json& config) : Emulator(config)
  {
    // Parsing rom file path
    _gameDataPath = jaffarCommon::json::getString(config, "Game Data Path");

    // Parsing initial RAM Data file
    _initialRAMDataFilePath = jaffarCommon::json::getString(config, "Initial RAM Data File Path");

    // Getting initial sequence file path
    _initialSequenceFilePath = jaffarCommon::json::getString(config, "Initial Sequence File Path");

    // Instantiating emulator
    _quickerRAWGL = std::make_unique<rawspace::EmuInstance>(config);
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _mutex.lock();
    _quickerRAWGL->initialize(_gameDataPath);
    _mutex.unlock();

    // Advancing the state using the initial sequence, if provided
    if (_initialSequenceFilePath != "")
    {
      // Getting input parser from the internal emulator
      const auto inputParser = _quickerRAWGL->getInputParser();

      // Load initial sequence
      std::string initialSequenceFileString;
      if (jaffarCommon::file::loadStringFromFile(initialSequenceFileString, _initialSequenceFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial sequence file: %s\n", _initialSequenceFilePath.c_str());

      // Getting input sequence
      const auto initialSequence = jaffarCommon::string::split(initialSequenceFileString, '\0');

      // Running inputs in the initial sequence
      for (const auto& inputString : initialSequence) advanceStateImpl(inputParser->parseInputString(inputString));
    }

    // Pushing initial RAM data
    if (_initialRAMDataFilePath != "")
    {
      // Load initial RAM Data
      std::string initialRAMDataString;
      if (jaffarCommon::file::loadStringFromFile(initialRAMDataString, _initialRAMDataFilePath) == false)
        JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from RAM Data file: %s\n", _initialRAMDataFilePath.c_str());

      // Verifying RAM Data file size
      if (initialRAMDataString.size() != 256) JAFFAR_THROW_LOGIC("[ERROR] Wrong RAM file size %lu != 256\n", initialRAMDataString.size());

      // Pushing data into RAM
      uint8_t* oldRAM = (uint8_t*)_quickerRAWGL->getRamPointer();
      uint8_t* newRAM = (uint8_t*)initialRAMDataString.data();
      for (size_t i = 0; i < 128; i++)
      {
        oldRAM[i * 2 + 0] = newRAM[i * 2 + 1];
        oldRAM[i * 2 + 1] = newRAM[i * 2 + 0];
      }
    }
  }

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override { return _quickerRAWGL->getInputParser(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override { _quickerRAWGL->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override { _quickerRAWGL->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override { _quickerRAWGL->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string& propertyName) const override
  {
    if (propertyName == "RAM") return property_t(_quickerRAWGL->getRamPointer(), 256);
    if (propertyName == "Threads Data") return property_t((uint8_t*)_quickerRAWGL->getThreadsData(), _quickerRAWGL->getThreadsDataSize());
    if (propertyName == "Script Stack Data") return property_t((uint8_t*)_quickerRAWGL->getScriptStackData(), _quickerRAWGL->getScriptStackDataSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    // enableStateProperties();
    _quickerRAWGL->initializeVideoOutput();
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerRAWGL->finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerRAWGL->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerRAWGL->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base& serializer) const override
  {
    serializeState(serializer);
    serializer.push(_quickerRAWGL->getPixelsPtr(), _quickerRAWGL->getPixelsSize());
    serializer.push(_quickerRAWGL->getPalettePtr(), _quickerRAWGL->getPaletteSize());
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(_quickerRAWGL->getPixelsPtr(), _quickerRAWGL->getPixelsSize());
    deserializer.pop(_quickerRAWGL->getPalettePtr(), _quickerRAWGL->getPaletteSize());
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerRAWGL->updateRenderer(); }

private:
  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;

  std::unique_ptr<rawspace::EmuInstance> _quickerRAWGL;

  std::string _gameDataPath;
  std::string _initialSequenceFilePath;
  std::mutex  _mutex;
  std::string _initialRAMDataFilePath;
};

} // namespace emulator

} // namespace jaffarPlus