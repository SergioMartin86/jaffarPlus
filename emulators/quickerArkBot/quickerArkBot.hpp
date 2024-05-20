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
  QuickerArkBot(const nlohmann::json &config) : Emulator(config)
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

    #ifdef _JAFFAR_PLAYER
     // Parsing rom file path
     _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");
    #endif

    // Allocating emulator
    _quickerArkBot = new ark::EmuInstance(_initialLevel, _initialScore, isVerify);
  };

  ~QuickerArkBot()
  {
    delete _quickerArkBot;
  }

  void initializeImpl() override
  {
    // Initializing emulator
    _quickerArkBot->initialize(_romFilePath);
  }

  // State advancing function
  void advanceState(const std::string &input) override
  {
     _quickerArkBot->advanceState(input);
  }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override
   {
     _quickerArkBot->serializeState(serializer);
   };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override
   {
    _quickerArkBot->deserializeState(deserializer);
   };

  __INLINE__ void loadFullState(const std::string &state) override
  {
    jaffarCommon::deserializer::Contiguous d(state.data(), state.size());
    deserializeState(d);
  }
  __INLINE__ void saveFullState(std::string &state) override
  {
    jaffarCommon::serializer::Contiguous s(state.data(), state.size());
    serializeState(s);
  }

  size_t getFullStateSize() override
  {
    jaffarCommon::serializer::Contiguous s;
    serializeState(s);
    return s.getOutputSize();
  }

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string &propertyName) const override
  {
    // if (propertyName == "RAM") return property_t(_quickerArkBot.getRamPointer(), 512);
    // if (propertyName == "Threads Data") return property_t((uint8_t *)_quickerArkBot.getThreadsData(), _quickerArkBot.getThreadsDataSize());
    // if (propertyName == "Script Stack Data") return property_t((uint8_t *)_quickerArkBot.getScriptStackData(), _quickerArkBot.getScriptStackDataSize());

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void enableStateProperty(const std::string &property)
   {
   }

  __INLINE__ void disableStateProperty(const std::string &property)
   {
   }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    // enableStateProperties();
    _quickerArkBot->initializeVideoOutput();
  }

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
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override
  {
    deserializeState(deserializer);
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override
   {
     _quickerArkBot->updateRenderer();
   }

  private:

  ark::EmuInstance* _quickerArkBot;
  std::string _romFilePath = "";
  uint8_t _initialLevel;
  uint32_t _initialScore;
};

} // namespace emulator

} // namespace jaffarPlus