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

    // Getting initial state file path
    _stateFilePath = jaffarCommon::json::getString(config, "Initial State File");

    // RNG overriding configuration
    _overrideRNGEnabled            = jaffarCommon::json::getBoolean(config, "Override RNG Enabled");
    _overrideRNGValue              = jaffarCommon::json::getNumber<uint32_t>(config, "Override RNG Value");
    _overrideLooseTileSoundEnabled = jaffarCommon::json::getBoolean(config, "Override Loose Tile Sound Enabled");
    _overrideLooseTileSoundValue   = jaffarCommon::json::getNumber<uint32_t>(config, "Override Loose Tile Sound Value");
    _initializeCopyProtection      = jaffarCommon::json::getBoolean(config, "Initialize Copy Protection");
  };

  void initializeImpl() override
  {
    _QuickerSDLPoP->initialize();

    // If an initial state is provided, load it now
    if (_stateFilePath != "")
    {
      std::string stateFileData;
      if (jaffarCommon::file::loadStringFromFile(stateFileData, _stateFilePath) == false) JAFFAR_THROW_LOGIC("Could not initial state file: %s\n", _stateFilePath.c_str());
      loadFullState(stateFileData);
    }

    // Check if RNG elements need overriding
    if (_overrideRNGEnabled) _QuickerSDLPoP->setRNGValue(_overrideRNGValue);
    if (_overrideLooseTileSoundEnabled) _QuickerSDLPoP->setLooseTileSound(_overrideLooseTileSoundValue);

    // Check if copy protection needs initializing
    if (_initializeCopyProtection) _QuickerSDLPoP->initializeCopyProtection();
  }

  // State advancing function
  void advanceState(const std::string &input) override { _QuickerSDLPoP->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const override { _QuickerSDLPoP->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer) override { _QuickerSDLPoP->deserializeState(deserializer); };

  __INLINE__ void enableStateProperty(const std::string &property) override { }

  __INLINE__ void disableStateProperty(const std::string &property) override { }

  __INLINE__ void loadFullState(const std::string &state) override
  {
    jaffarCommon::deserializer::Contiguous deserializer(state.data());
    _QuickerSDLPoP->deserializeState(deserializer);

    // Resetting global step counter
    _QuickerSDLPoP->getGameState()->globalStepCount = 0;
  }

  __INLINE__ void saveFullState(std::string &state) override
  {
    jaffarCommon::serializer::Contiguous s(state.data(), state.size());
    serializeState(s);
  }

  size_t getFullStateSize() override { return _QuickerSDLPoP->getFullStateSize(); }

  __INLINE__ void printInfo() const override {}

  property_t getProperty(const std::string &propertyName) const override
  {
    if (propertyName == "Game State") return property_t((uint8_t *)_QuickerSDLPoP->getGameState(), _QuickerSDLPoP->getFullStateSize());
    JAFFAR_THROW_LOGIC("Property name: '%s' not found in emulator '%s'", propertyName.c_str(), getName().c_str());
  }

  // This function opens the video output (e.g., window)
  void initializeVideoOutput() override
  {
    // Opening rendering window
    SDL_SetMainReady();

    // We can only call SDL_InitSubSystem once
    if (!SDL_WasInit(SDL_INIT_VIDEO))
      if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) JAFFAR_THROW_LOGIC("Failed to initialize video: %s", SDL_GetError());

    _window = SDL_CreateWindow("JaffarPlus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_RESIZABLE);
    if (_window == nullptr) JAFFAR_THROW_LOGIC("Coult not open SDL window");
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { SDL_DestroyWindow(_window); }

  __INLINE__ void enableRendering() override { _QuickerSDLPoP->enableRendering(_window); }

  __INLINE__ void disableRendering() override { _QuickerSDLPoP->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override
  {
    _renderStepIdx = stepIdx;

    SDLPoP::Controller          controller;
    SDLPoP::Controller::input_t newInput;

    bool isInputValid = controller.parseInputString(input);
    if (isInputValid == true) newInput = controller.getParsedInput();
    _renderInput = newInput;
  }

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base &serializer) const override
  {
    serializeState(serializer);
    serializer.push(&_renderStepIdx, sizeof(_renderStepIdx));
    serializer.push(&_renderInput, sizeof(_renderInput));
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(&_renderStepIdx, sizeof(_renderStepIdx));
    deserializer.pop(&_renderInput, sizeof(_renderInput));
  }

  __INLINE__ size_t getRendererStateSize() const override { return _QuickerSDLPoP->getFullStateSize() + sizeof(_renderStepIdx) + sizeof(_renderInput); }

  __INLINE__ void showRender() override { _QuickerSDLPoP->updateRenderer(_renderStepIdx, _renderInput); }

  private:

  std::unique_ptr<SDLPoPInstance> _QuickerSDLPoP;

  // initial state file path
  std::string _stateFilePath;

  // RNG overriding configuration
  bool     _overrideRNGEnabled;
  uint32_t _overrideRNGValue;
  bool     _overrideLooseTileSoundEnabled;
  uint32_t _overrideLooseTileSoundValue;
  bool     _initializeCopyProtection;

  // Internal render state variables
  size_t                      _renderStepIdx;
  SDLPoP::Controller::input_t _renderInput;
  SDL_Window                 *_window;
};

} // namespace emulator

} // namespace jaffarPlus