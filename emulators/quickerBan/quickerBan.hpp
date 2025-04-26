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

class QuickerBan final : public Emulator
{
public:
  static std::string getName() { return "QuickerBan"; }

  // Constructor must only do configuration parsing
  QuickerBan(const nlohmann::json& config) : Emulator(config)
  {
    // Creating internal emulator instance
    _quickerBan = std::make_unique<jaffar::EmuInstance>(config);
  };

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override { return _quickerBan->getInputParser(); }

  void initializeImpl() override { _quickerBan->initialize(); }

  jaffar::EmuInstance* getEmulator() const { return _quickerBan.get(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override { _quickerBan->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override { _quickerBan->serializeState(serializer); };
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override { _quickerBan->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string& propertyName) const override
  {
    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  void              initializeVideoOutput() override {}
  void              finalizeVideoOutput() override {}
  __INLINE__ void   enableRendering() override {}
  __INLINE__ void   disableRendering() override {}
  __INLINE__ void   updateRendererState(const size_t stepIdx, const std::string input) override {}
  __INLINE__ void   serializeRendererState(jaffarCommon::serializer::Base& serializer) const override {}
  __INLINE__ void   deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override {}
  __INLINE__ size_t getRendererStateSize() const override { return 0; }

  __INLINE__ void showRender() override {}

private:
  std::unique_ptr<jaffar::EmuInstance> _quickerBan;
};

} // namespace emulator

} // namespace jaffarPlus