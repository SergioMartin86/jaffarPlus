#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <mutex>
#include <emulator.hpp>
#include <NEORAWInstance.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerNEORAW final : public Emulator
{
  public:

  static std::string getName() { return "QuickerNEORAW"; }

  // Constructor must only do configuration parsing
  QuickerNEORAW(const nlohmann::json &config)
    : Emulator(config)
  {
    // Getting initial state file from the configuration
    _initialStateFilePath = jaffarCommon::json::getString(config, "Initial State File Path");

    // Parsing rom file path
    _gameDataPath = jaffarCommon::json::getString(config, "Game Data Path");

    // Parsing initial RAM Data file
    _initialRAMDataFilePath = jaffarCommon::json::getString(config, "Initial RAM Data File Path");
    _initialRAMDataEndiannes = jaffarCommon::json::getString(config, "Initial RAM Data Endiannes");

    // Instantiating emulator
    _quickerNEORAW = std::make_unique<rawspace::EmuInstance>(config);
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _mutex.lock();
    _quickerNEORAW->initialize(_gameDataPath);

    // If initial state file defined, load it
    if (_initialStateFilePath.empty() == false)
      {
        // Reading from initial state file
        std::string initialState;
        bool        success = jaffarCommon::file::loadStringFromFile(initialState, _initialStateFilePath);
        if (success == false) JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from initial state file: %s\n", _initialStateFilePath.c_str());

        // Deserializing initial state into the emulator
        jaffarCommon::deserializer::Contiguous d(initialState.data(), initialState.size());
        deserializeState(d);
    }

    _mutex.unlock();

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
        uint8_t *oldRAM = (uint8_t *)_quickerNEORAW->getRamPointer();
        uint8_t *newRAM = (uint8_t *)initialRAMDataString.data();
        if (_initialRAMDataEndiannes == "Big Endian")
        {
          for (size_t i = 0; i < 128; i++)
            {
              oldRAM[i * 2 + 0] = newRAM[i * 2 + 1];
              oldRAM[i * 2 + 1] = newRAM[i * 2 + 0];
            }
        }
        else memcpy(oldRAM, newRAM, 256);
    }
    
    #ifndef _JAFFAR_PLAYER
    _quickerNEORAW->disableStateBlock("NVS");
    #endif
  }

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser *getInputParser() const override { return _quickerNEORAW->getInputParser(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t &input) override 
  {
    _quickerNEORAW->advanceState(input);
  }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _quickerNEORAW->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _quickerNEORAW->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "RAM") return property_t(_quickerNEORAW->getRamPointer(), 256);
    if (propertyName == "Threads Data") return property_t((uint8_t *)_quickerNEORAW->getThreadsData(), _quickerNEORAW->getThreadsDataSize());
    if (propertyName == "Script Stack Data") return property_t((uint8_t *)_quickerNEORAW->getScriptStackData(), _quickerNEORAW->getScriptStackDataSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    _quickerNEORAW->initializeVideoOutput();
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerNEORAW->finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerNEORAW->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerNEORAW->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override
  {
    serializeState(serializer);
    serializer.push(_quickerNEORAW->getPixelsPtr(), _quickerNEORAW->getPixelsSize());
    serializer.push(_quickerNEORAW->getPalettePtr(), _quickerNEORAW->getPaletteSize());
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(_quickerNEORAW->getPixelsPtr(), _quickerNEORAW->getPixelsSize());
    deserializer.pop(_quickerNEORAW->getPalettePtr(), _quickerNEORAW->getPaletteSize());
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerNEORAW->updateRenderer(); }

  private:

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;

  std::unique_ptr<rawspace::EmuInstance> _quickerNEORAW;

  std::string _initialStateFilePath;
  std::string _gameDataPath;
  std::mutex  _mutex;
  std::string _initialRAMDataFilePath;
  std::string _initialRAMDataEndiannes;
};

} // namespace emulator

} // namespace jaffarPlus