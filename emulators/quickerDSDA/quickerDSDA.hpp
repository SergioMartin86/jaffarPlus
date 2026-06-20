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

class QuickerDSDA final : public Emulator
{
public:
  static std::string getName() { return "QuickerDSDA"; }

  // Constructor must only do configuration parsing
  QuickerDSDA(const nlohmann::json& config) : Emulator(config)
  {
    // Getting initial sequence file path
    _initialSequenceFilePath = jaffarCommon::json::popString(_emulatorConfigRemaining, "Initial Sequence File Path");

    // Creating internal emulator instance (constructed from the original config)
    _quickerDSDA = std::make_unique<jaffar::EmuInstance>(config);

    // Keys consumed by the underlying EmuInstance (EmuInstanceBase): popped here too so the strict-key
    // check below accounts for them. The instance was constructed from the original config above.
    jaffarCommon::json::popString(_emulatorConfigRemaining, "IWAD File Path");
    jaffarCommon::json::popString(_emulatorConfigRemaining, "Expected IWAD SHA1");
    jaffarCommon::json::popNumber<size_t>(_emulatorConfigRemaining, "State Size");
    jaffarCommon::json::popNumber<unsigned int>(_emulatorConfigRemaining, "Skill Level");
    jaffarCommon::json::popNumber<unsigned int>(_emulatorConfigRemaining, "Episode");
    jaffarCommon::json::popNumber<unsigned int>(_emulatorConfigRemaining, "Map");
    jaffarCommon::json::popNumber<unsigned int>(_emulatorConfigRemaining, "Compatibility Level");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Fast Monsters");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Monsters Respawn");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "No Monsters");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Prevent Level Exit");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Prevent Game End");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Player 1 Present");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Player 2 Present");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Player 3 Present");
    jaffarCommon::json::popBoolean(_emulatorConfigRemaining, "Player 4 Present");
    jaffarCommon::json::popNumber<uint8_t>(_emulatorConfigRemaining, "Player Point of View");
    jaffarCommon::json::popArray<nlohmann::json>(_emulatorConfigRemaining, "PWADS");

    // All recognized emulator-configuration keys have now been consumed; reject any leftover (unrecognized) key.
  };

  void initializeImpl() override
  {
    // Initializing emulator
    _quickerDSDA->initialize();

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
      for (const auto& inputString : initialSequence) advanceStateImpl(getInputParser()->parseInputString(inputString));
    }

    // Creating initial save buffer
    jaffarCommon::serializer::Contiguous s(_saveBuffer, _saveBufferSize);
    _quickerDSDA->serializeState(s);
  }

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override { return _quickerDSDA->getInputParser(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override { _quickerDSDA->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override { _quickerDSDA->serializeState(serializer); };

  __INLINE__ size_t getEffectiveStateSize() const { return headlessGetEffectiveSaveSize(); }

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override { _quickerDSDA->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string& propertyName) const override
  {
    // if (propertyName == "VRAM") return property_t((uint8_t *)memoryAreas.vram, memorySizes.vram);

    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override { _quickerDSDA->initializeVideoOutput(); }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { _quickerDSDA->finalizeVideoOutput(); }

  __INLINE__ void enableRendering() override { _quickerDSDA->enableRendering(); }

  __INLINE__ void disableRendering() override { _quickerDSDA->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override {}

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base& serializer) const override
  {
    // Getting video buffer size
    serializeState(serializer);
    serializer.push(_quickerDSDA->getVideoBufferPtr(), _quickerDSDA->getVideoBufferSize());
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(_quickerDSDA->getVideoBufferPtr(), _quickerDSDA->getVideoBufferSize());
  }

  __INLINE__ size_t getRendererStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeRendererState(s);
    return s.getOutputSize();
  }

  __INLINE__ void showRender() override { _quickerDSDA->updateRenderer(); }

private:
  std::unique_ptr<jaffar::EmuInstance> _quickerDSDA;
  std::string                          _initialSequenceFilePath;

  static const size_t _saveBufferSize = 32 * 1024 * 1024;
  uint8_t             _saveBuffer[_saveBufferSize];
};

} // namespace emulator

} // namespace jaffarPlus