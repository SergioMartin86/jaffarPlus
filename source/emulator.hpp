#pragma once

#include <string>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <SDL2/SDL.h>

namespace jaffarPlus
{

// A property is a contiguous segment of memory with size, identifiable by name
struct property_t
{
  uint8_t *pointer;
  size_t   size;
};

class Emulator
{
  public:

  // Constructor must only do configuration parsing to perform dry runs
  Emulator(const nlohmann::json &config)
  {
    // Getting emulator name (for runtime use)
    _emulatorName = jaffarCommon::json::getString(config, "Emulator Name");
  };

  virtual ~Emulator() = default;

  // Initialization function
  __INLINE__ void initialize()
  {
    if (_isInitialized == true) JAFFAR_THROW_LOGIC("This emulator instance was already initialized");

    // Calling emulator-specific initializer
    initializeImpl();

    // Set this as initialized
    _isInitialized = true;
  }

  // State advancing function
  virtual void advanceState(const std::string &move) = 0;

  // State serialization / deserialization functions
  size_t getStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeState(s);
    return s.getOutputSize();
  }

  __INLINE__ bool isInitialized() const { return _isInitialized; }

  __INLINE__ std::string getName() const { return _emulatorName; }

  virtual void initializeImpl() = 0;

  virtual void serializeState(jaffarCommon::serializer::Base &serializer) const = 0;
  virtual void deserializeState(jaffarCommon::deserializer::Base &deserializer) = 0;

  virtual void   loadFullState(const std::string &state) = 0;
  virtual void   saveFullState(std::string &state)       = 0;
  virtual size_t getFullStateSize()                      = 0;

  // Function to print debug information, whatever it might be
  virtual void printInfo() const = 0;

  // Get a property by name
  virtual property_t getProperty(const std::string &propertyName) const = 0;

  // Function to obtain emulator based on name
  static std::unique_ptr<Emulator> getEmulator(const nlohmann::json &emulatorConfig);

  /////// Render-related functions

  // This function enables rendering within the emulation core (does not output it to screen though)
  virtual void enableRendering(SDL_Window *window) = 0;

  // This function disables rendering within the emulation core (typically enables faster emulation)
  virtual void disableRendering() = 0;

  // Updates the internal state of the renderer with the current game state
  virtual void updateRendererState(const size_t stepIdx, const std::string input) = 0;

  // This function gathers the necessary data for output rendering of a given state/frame
  virtual void serializeRendererState(jaffarCommon::serializer::Base &serializer) const = 0;

  // This function pushes the necessary data for output rendering of a given state/frame
  virtual void deserializeRendererState(jaffarCommon::deserializer::Base &deserializer) = 0;

  // This function returns the size of the renderer state
  virtual size_t getRendererStateSize() const = 0;

  // Shows the contents of the emulator's renderer into the window
  virtual void showRender() = 0;

  // Emulator name (for runtime use)
  std::string _emulatorName;

  // Stores whether the emulator has been initialized
  bool _isInitialized = false;
};

} // namespace jaffarPlus