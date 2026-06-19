#pragma once

#include <emulator.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <memory>
#include <sdlpopInstance.hpp>
#include <sstream>

// SDLPoP's final rendered software surface (defined in the linked SDLPoP core), used for screenshots.
extern "C" SDL_Surface* get_final_surface();

namespace jaffarPlus
{

namespace emulator
{

class QuickerSDLPoP final : public Emulator
{
public:
  static std::string getName() { return "QuickerSDLPoP"; }

  // Constructor must only do configuration parsing
  QuickerSDLPoP(const nlohmann::json& config) : Emulator(config)
  {
    _QuickerSDLPoP = std::make_unique<SDLPoPInstance>(config);

    // Getting initial state file path
    _stateFilePath = jaffarCommon::json::getString(config, "Initial State File");

    // Optional fixed input sequence to replay after loading the initial state (default: none)
    _initialSequenceFilePath = config.contains("Initial Sequence File Path") ? config["Initial Sequence File Path"].get<std::string>() : std::string("");

    // RNG overriding configuration
    _overrideRNGEnabled            = jaffarCommon::json::getBoolean(config, "Override RNG Enabled");
    _overrideRNGValue              = jaffarCommon::json::getNumber<uint32_t>(config, "Override RNG Value");
    _overrideLooseTileSoundEnabled = jaffarCommon::json::getBoolean(config, "Override Loose Tile Sound Enabled");
    _overrideLooseTileSoundValue   = jaffarCommon::json::getNumber<uint32_t>(config, "Override Loose Tile Sound Value");
    _initializeCopyProtection      = jaffarCommon::json::getBoolean(config, "Initialize Copy Protection");

    // Exploration-only: when true, cosmetic torch/potion animations stop consuming the shared RNG, so
    // random_seed advances only on gameplay (mainly guard combat). Lets the search hash the seed soundly
    // without torch noise. Solutions found this way do NOT transcribe to the real game. Default false.
    _disableNonGameplayRNG = config.contains("Disable Non-Gameplay RNG") ? config["Disable Non-Gameplay RNG"].get<bool>() : false;
  };

  void initializeImpl() override
  {
    _QuickerSDLPoP->initialize();

    // Apply the cosmetic-RNG toggle before any frames are stepped (incl. the initial sequence replay)
    _QuickerSDLPoP->setDisableNonGameplayRNG(_disableNonGameplayRNG);

    // If an initial state is provided, load it now
    if (_stateFilePath != "")
    {
      std::string stateFileData;
      if (jaffarCommon::file::loadStringFromFile(stateFileData, _stateFilePath) == false) JAFFAR_THROW_LOGIC("Could not initial state file: %s\n", _stateFilePath.c_str());

      jaffarCommon::deserializer::Contiguous deserializer(stateFileData.data());
      _QuickerSDLPoP->deserializeState(deserializer);
    }

    // Check if RNG elements need overriding
    if (_overrideRNGEnabled) _QuickerSDLPoP->setRNGValue(_overrideRNGValue);
    if (_overrideLooseTileSoundEnabled) _QuickerSDLPoP->setLooseTileSound(_overrideLooseTileSoundValue);

    // Check if copy protection needs initializing
    if (_initializeCopyProtection) _QuickerSDLPoP->initializeCopyProtection();

    // If a fixed initial sequence is provided, replay it now (advances the state past the prefix)
    if (_initialSequenceFilePath != "")
    {
      std::string seqData;
      if (jaffarCommon::file::loadStringFromFile(seqData, _initialSequenceFilePath) == false)
        JAFFAR_THROW_LOGIC("Could not load initial sequence file: %s\n", _initialSequenceFilePath.c_str());

      std::istringstream iss(seqData);
      std::string        line;
      while (std::getline(iss, line))
      {
        // Trim trailing carriage-return / whitespace
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\n')) line.pop_back();
        if (line.empty()) continue;
        auto inputData = getInputParser()->parseInputString(line);
        _QuickerSDLPoP->advanceState(inputData);
      }
    }

    // Resetting global step counter
    _QuickerSDLPoP->getGameState()->globalStepCount = 0;
  }

  // Function to get a reference to the input parser from the base emulator
  jaffar::InputParser* getInputParser() const override { return _QuickerSDLPoP->getInputParser(); }

  // State advancing function
  void advanceStateImpl(const jaffar::input_t& input) override { _QuickerSDLPoP->advanceState(input); }

  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const override { _QuickerSDLPoP->serializeState(serializer); };

  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer) override { _QuickerSDLPoP->deserializeState(deserializer); };

  __INLINE__ void printInfo() const override {}

  __INLINE__ property_t getProperty(const std::string& propertyName) const override
  {
    if (propertyName == "Game State") return property_t((uint8_t*)_QuickerSDLPoP->getGameState(), _QuickerSDLPoP->getFullStateSize());
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

    _window = SDL_CreateWindow("JaffarPlus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 100, SDL_WINDOW_RESIZABLE);
    if (_window == nullptr) JAFFAR_THROW_LOGIC("Coult not open SDL window");
  }

  // This function closes the video output (e.g., window)
  void finalizeVideoOutput() override { SDL_DestroyWindow(_window); }

  __INLINE__ void enableRendering() override { _QuickerSDLPoP->enableRendering(_window); }

  __INLINE__ void disableRendering() override { _QuickerSDLPoP->disableRendering(); }

  __INLINE__ void updateRendererState(const size_t stepIdx, const std::string input) override
  {
    _renderStepIdx = stepIdx;
    _renderInput   = input != "<End Of Sequence>" ? getInputParser()->parseInputString(input) : jaffar::input_t();
  }

  __INLINE__ void serializeRendererState(jaffarCommon::serializer::Base& serializer) const override
  {
    serializeState(serializer);
    serializer.push(&_renderStepIdx, sizeof(_renderStepIdx));
    serializer.push(&_renderInput, sizeof(_renderInput));
  }

  __INLINE__ void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializeState(deserializer);
    deserializer.pop(&_renderStepIdx, sizeof(_renderStepIdx));
    deserializer.pop(&_renderInput, sizeof(_renderInput));
  }

  __INLINE__ size_t getRendererStateSize() const override { return getStateSize() + sizeof(_renderStepIdx) + sizeof(_renderInput); }

  __INLINE__ void showRender() override { _QuickerSDLPoP->updateRenderer(_renderStepIdx, _renderInput); }

  // Saves the last rendered frame (SDLPoP's final software surface) as a BMP. Works headlessly under
  // SDL_VIDEODRIVER=offscreen/dummy since the surface is drawn in memory regardless of a real display.
  __INLINE__ void saveScreenshot(const std::string& path) override
  {
    SDL_Surface* surface = get_final_surface();
    if (surface == nullptr) return;
    if (SDL_SaveBMP(surface, path.c_str()) == 0)
      jaffarCommon::logger::log("[J+] Saved screenshot: %s\n", path.c_str());
    else
      jaffarCommon::logger::log("[J+] Screenshot failed (%s): %s\n", path.c_str(), SDL_GetError());
  }

private:
  std::unique_ptr<SDLPoPInstance> _QuickerSDLPoP;

  // initial state file path
  std::string _stateFilePath;
  std::string _initialSequenceFilePath;

  // RNG overriding configuration
  bool     _overrideRNGEnabled;
  uint32_t _overrideRNGValue;
  bool     _overrideLooseTileSoundEnabled;
  uint32_t _overrideLooseTileSoundValue;
  bool     _initializeCopyProtection;
  bool     _disableNonGameplayRNG;

  // Internal render state variables
  size_t          _renderStepIdx;
  jaffar::input_t _renderInput;
  SDL_Window*     _window;
};

} // namespace emulator

} // namespace jaffarPlus