#pragma once

#include <string>
#include <jaffarCommon/include/file.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/serializers/contiguous.hpp>
#include <jaffarCommon/include/deserializers/contiguous.hpp>

namespace jaffarPlus
{

// A property is a contiguous segment of memory with size, identifiable by name 
struct property_t { uint8_t* pointer; size_t size; };

class Emulator
{
  public:

  // Constructor must only do configuration parsing to perform dry runs
  Emulator(const nlohmann::json& config) {};
  virtual ~Emulator() = default;

  // Initialization function
  virtual void initialize() = 0;

  // State advancing function
  virtual void advanceState(const std::string& move) = 0;

  // State file load / save functions
  inline void loadStateFile(const std::string &stateFilePath)
  {
    std::string stateData;
    bool status = jaffarCommon::loadStringFromFile(stateData, stateFilePath.c_str());
    if (status == false) EXIT_WITH_ERROR("Could not find/read state file: %s\n", stateFilePath.c_str());
    jaffarCommon::deserializer::Contiguous d(stateData.data());
    deserializeState(d);
  }

  inline void saveStateFile(const std::string &stateFilePath) const
  {
    std::string stateData;
    stateData.resize(getStateSize());
    jaffarCommon::serializer::Contiguous s(stateData.data());
    serializeState(s);
    jaffarCommon::saveStringToFile(stateData, stateFilePath.c_str());
  }

  // State serialization / deserialization functions
  size_t getStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    serializeState(s);
    return s.getOutputSize();
  }

  virtual void serializeState(jaffarCommon::serializer::Base& serializer) const = 0;
  virtual void deserializeState(jaffarCommon::deserializer::Base& deserializer) = 0;

  // Function to print debug information, whatever it might be
  virtual void printInfo() const = 0;

  // Get a property by name
  virtual property_t getProperty(const std::string& propertyName) const = 0;

  // Function to obtain emulator based on name
  static std::unique_ptr<Emulator> getEmulator(const std::string& emulatorName, const nlohmann::json& config);

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
  virtual void launchRendererWindow() = 0;

  // Update the contents of the emulator's renderer window
  virtual void updateRendererWindow() = 0;

  // Closes the emulator's renderer window
  virtual void closeRendererWindow() = 0;
};

} // namespace jaffarPlus