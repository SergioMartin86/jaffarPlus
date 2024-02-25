#pragma once

#include <string>
#include <jaffarCommon/include/file.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/serializers/contiguous.hpp>
#include <jaffarCommon/include/deserializers/contiguous.hpp>

#ifdef _USE_SDL2
#include <SDL.h>
#endif

namespace jaffarPlus
{

// A property is a contiguous segment of memory with size, identifiable by name 
struct property_t { uint8_t* pointer; size_t size; };

class Emulator
{
  public:

  // Constructor must only do configuration parsing to perform dry runs
  Emulator(const nlohmann::json& config)
  {
    // Getting emulator name (for runtime use)
    _emulatorName = JSON_GET_STRING(config, "Emulator Name");

    // Getting disabled state properties
    auto disabledStatePropertiesJs = JSON_GET_ARRAY(config, "Disabled State Properties");
    for (const auto& property : disabledStatePropertiesJs)
    {
      // Sanity Check
      if (property.is_string() == false) EXIT_WITH_ERROR("Disabled emulator properties entries should be strings");

      // Getting property name
      _disabledStateProperties.push_back(property.get<std::string>());
    }
  };
  virtual ~Emulator() = default;

  // Initialization function
  virtual void initialize() = 0;

  // State advancing function
  virtual void advanceState(const std::string& move) = 0;

  // State serialization / deserialization functions
  size_t getStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeState(s);
    return s.getOutputSize();
  }

  inline std::string getName() const { return _emulatorName; }

  virtual void serializeState(jaffarCommon::serializer::Base& serializer) const = 0;
  virtual void deserializeState(jaffarCommon::deserializer::Base& deserializer) = 0;
  
  virtual void enableStateProperty(const std::string& property) = 0;
  virtual void disableStateProperty(const std::string& property) = 0;

  // Function to print debug information, whatever it might be
  virtual void printInfo() const = 0;

  // Get a property by name
  virtual property_t getProperty(const std::string& propertyName) const = 0;

  // Function to obtain emulator based on name
  static std::unique_ptr<Emulator> getEmulator(const nlohmann::json& emulatorConfig);

  /////// Render-related functions

  // This function enables rendering within the emulation core (does not output it to screen though)
  virtual void enableRendering() = 0;

  // This function disables rendering within the emulation core (typically enables faster emulation)
  virtual void disableRendering() = 0;
 
  // Updates the internal state of the renderer with the current game state
  virtual void updateRendererState () = 0;

  // This function gathers the necessary data for output rendering of a given state/frame
  virtual void serializeRendererState(jaffarCommon::serializer::Base& serializer) const = 0;

  // This function pushes the necessary data for output rendering of a given state/frame
  virtual void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) = 0;

  // This function returns the size of the renderer state
  virtual size_t getRendererStateSize() const = 0;

  // Opens the emulator's renderer window for output to screen
  #ifdef _USE_SDL2
  virtual void initializeVideoOutput(SDL_Window* window) = 0;
  #else
  virtual void initializeVideoOutput() = 0;
  #endif

  // Update the contents of the emulator's renderer window
  virtual void updateVideoOutput() = 0;

  // Closes the emulator's renderer window
  virtual void finalizeVideoOutput() = 0;

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;

  // Emulator name (for runtime use)
  std::string _emulatorName;
};

} // namespace jaffarPlus