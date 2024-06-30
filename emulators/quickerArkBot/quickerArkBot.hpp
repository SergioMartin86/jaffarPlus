#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <mutex>
#include <emulator.hpp>
#include <ArkBotInstance.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerArkBot final : public Emulator
{
  public:

  static std::string getName() { return "QuickerArkBot"; }

  // Constructor must only do configuration parsing
  QuickerArkBot(const nlohmann::json &config)
    : Emulator(config)
  {
    // Verifying only if reproducing video
    bool isVerify = false;
#ifdef _JAFFAR_PLAYER
    isVerify = true;
#endif

    // Getting initial state file from the configuration
    _initialLevel = jaffarCommon::json::getNumber<uint8_t>(config, "Initial Level");

    // Getting initial state file from the configuration
    _initialScore = jaffarCommon::json::getNumber<uint32_t>(config, "Initial Score");

    // Parsing controller configuration
    _controller1Type = jaffarCommon::json::getString(config, "Controller 1 Type");
    _controller2Type = jaffarCommon::json::getString(config, "Controller 2 Type");

#ifdef _JAFFAR_PLAYER
    // Parsing rom file path
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");

    // Parsing initial RAM data path
    _ramDataFilePath = jaffarCommon::json::getString(config, "RAM Data File Path");
#endif

    // Allocating emulator
    _quickerArkBot = new ark::EmuInstance(_initialLevel, _initialScore, isVerify);
  };

  ~QuickerArkBot() { delete _quickerArkBot; }

  void initializeImpl() override
  {
    // Setting controller types
    _quickerArkBot->setController1Type(_controller1Type);
    _quickerArkBot->setController2Type(_controller2Type);

    // Initializing emulator
    _quickerArkBot->initialize(_romFilePath);

#ifdef _JAFFAR_PLAYER

    // Loading initial RAM state file
    std::string ramData;
    {
      auto success = jaffarCommon::file::loadStringFromFile(ramData, _ramDataFilePath);
      if (success == false) JAFFAR_THROW_LOGIC("Could not find RAM data file: %s\n", _ramDataFilePath.c_str());
    }

    // Setting initial ram data for quickerNES
    memcpy(_quickerArkBot->getRamPointer(), ramData.data(), 0x800);

#endif
  }

  // State advancing function
  void advanceState(const std::string &input) override { _quickerArkBot->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _quickerArkBot->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _quickerArkBot->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override { _quickerArkBot->printInformation(); }

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "ArkBot State") return property_t((uint8_t *)_quickerArkBot->getGameState(), sizeof(GameState));

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void enableStateProperty(const std::string &property) override {}

  __INLINE__ void disableStateProperty(const std::string &property) override {}

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override { _quickerArkBot->initializeVideoOutput(); }

  __INLINE__ void postInitialSequenceHook() override
  {
    // Resetting arkbot after playing the initial sequence
    _quickerArkBot->doSoftReset();
  };

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerArkBot->finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerArkBot->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerArkBot->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override
  {
    serializeState(serializer);
    serializer.push(_quickerArkBot->getBlitPointer(), _quickerArkBot->getBlitSize());
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(_quickerArkBot->getBlitPointer(), _quickerArkBot->getBlitSize());
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  GameState *getGameState() { return _quickerArkBot->getGameState(); }

  __INLINE__ void showRender() override { _quickerArkBot->updateRenderer(); }

  private:

  ark::EmuInstance *_quickerArkBot;
  std::string       _romFilePath     = "";
  std::string       _ramDataFilePath = "";
  uint8_t           _initialLevel;
  uint32_t          _initialScore;
  std::string       _controller1Type;
  std::string       _controller2Type;
};

} // namespace emulator

} // namespace jaffarPlus