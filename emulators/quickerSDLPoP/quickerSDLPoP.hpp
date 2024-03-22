#pragma once

#include <memory>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <emulator.hpp>
#include <sdlpopInstance.hpp>

namespace jaffarPlus
{

namespace emulator
{

class QuickerSDLPoP final : public Emulator
{
  public:

  static std::string getName() { return "QuickerSDLPoP"; }

  // Constructor must only do configuration parsing
  QuickerSDLPoP(const nlohmann::json &config)
    : Emulator(config)
  {
    _QuickerSDLPoP = std::make_unique<SDLPoPInstance>(config);
  };

  void initializeImpl() override
  {
    _QuickerSDLPoP->initialize();
  }

  // State advancing function
  void advanceState(const std::string &input) override { _QuickerSDLPoP->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _QuickerSDLPoP->serializeState(serializer); };
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _QuickerSDLPoP->deserializeState(deserializer); };

  __INLINE__ void loadFullState(const std::string& state) override
  {
    jaffarCommon::deserializer::Contiguous d(state.data(), state.size());
    deserializeState(d);
  }

  __INLINE__ void saveFullState(std::string& state) override
  {
    jaffarCommon::serializer::Contiguous s(state.data(), state.size());
    serializeState(s);
  }

  size_t getFullStateSize() override
  {
    return _QuickerSDLPoP->getFullStateSize();
  }

  __INLINE__ void printInfo() const override
  {
  }

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "Game State") return property_t((uint8_t*)_QuickerSDLPoP->getGameState(), _QuickerSDLPoP->getFullStateSize());
    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  __INLINE__ void   enableRendering() override { _QuickerSDLPoP->enableRendering(); }
  __INLINE__ void   disableRendering() override { _QuickerSDLPoP->disableRendering();  }
  __INLINE__ void   updateRendererState() override {  }
  __INLINE__ void   serializeRendererState(jaffarCommon::serializer::Base &serializer) const override { serializeState(serializer); }
  __INLINE__ void   deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override { deserializeState(deserializer); }

  __INLINE__ size_t getRendererStateSize() const override
  {
    return _QuickerSDLPoP->getFullStateSize();
  }

  __INLINE__ void initializeVideoOutput(SDL_Window *window) override
  {
  }

  __INLINE__ void updateVideoOutput() override
  {
  }

  __INLINE__ void finalizeVideoOutput() override
  {
  }

  private:

  std::unique_ptr<SDLPoPInstance> _QuickerSDLPoP;
};

} // namespace emulator

} // namespace jaffarPlus