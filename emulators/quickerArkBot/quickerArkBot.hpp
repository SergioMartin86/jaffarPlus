#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <emulator.hpp>
#include <ArkBotInstance.hpp>
#include <inputParser.hpp>

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
    // Getting initial state file from the configuration
    _initialLevel = jaffarCommon::json::getNumber<uint8_t>(config, "Initial Level");

    // Getting initial state file from the configuration
    _initialScore = jaffarCommon::json::getNumber<uint32_t>(config, "Initial Score");

    // Allocating emulator
    _quickerArkBot = new ark::EmuInstance(_initialLevel, _initialScore);
  };

  ~QuickerArkBot() { delete _quickerArkBot; }

  void initializeImpl() override
  {
    // Initializing emulator
    _quickerArkBot->initialize();
  }

  // Function to get a reference to the input parser from the base emulator
  virtual jaffar::InputParser *getInputParser() const { return _quickerArkBot->getInputParser(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t &input) override { _quickerArkBot->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _quickerArkBot->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _quickerArkBot->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override { _quickerArkBot->printInformation(); }

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "ArkBot State") return property_t((uint8_t *)_quickerArkBot->getGameState(), sizeof(GameState));

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override {}

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override {}

  __INLINE__ void enableRendering() override {}

  __INLINE__ void disableRendering() override {}

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override { serializeState(serializer); }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override { deserializeState(deserializer); }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  GameState *getGameState() { return _quickerArkBot->getGameState(); }

  __INLINE__ void showRender() override {}

  __INLINE__ void doSoftReset() { _quickerArkBot->doSoftReset(); };

  private:

  ark::EmuInstance *_quickerArkBot;
  uint8_t           _initialLevel;
  uint32_t          _initialScore;
};

} // namespace emulator

} // namespace jaffarPlus