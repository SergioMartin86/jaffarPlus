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
    // Getting disabled state properties
    const auto disabledStateProperties = jaffarCommon::json::getArray<std::string>(config, "Disabled State Properties");
    for (const auto &property : disabledStateProperties) _disabledStateProperties.push_back(property);

    // Getting emulator name (for runtime use)
    _emulatorName = jaffarCommon::json::getString(config, "Emulator Name");

    // Getting initial sequence file path
    _initialSequenceFilePath = jaffarCommon::json::getString(config, "Initial Sequence File Path");
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
      for (const auto &input : initialSequence) advanceState(input);
    }
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

  __INLINE__ void disableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) disableStateProperty(property);
  }
  __INLINE__ void enableStateProperties()
  {
    for (const auto &property : _disabledStateProperties) enableStateProperty(property);
  }

  // Function to print debug information, whatever it might be
  virtual void printInfo() const = 0;

  // Get a property by name
  virtual property_t getProperty(const std::string &propertyName) const = 0;

  // Function to obtain emulator based on name
  static std::unique_ptr<Emulator> getEmulator(const nlohmann::json &emulatorConfig);

  /////// Render-related functions

  // This function opens the video output (e.g., window)
  virtual void initializeVideoOutput() = 0;

  // This function closes the video output (e.g., window)
  virtual void finalizeVideoOutput() = 0;

  // This function enables rendering within the emulation core (does not output it to screen though)
  virtual void enableRendering() = 0;

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

  protected:

  virtual void enableStateProperty(const std::string &property) = 0;

  virtual void disableStateProperty(const std::string &property) = 0;

  // Emulator name (for runtime use)
  std::string _emulatorName;

  // Stores whether the emulator has been initialized
  bool _isInitialized = false;

  // Collection of state blocks to disable during engine run
  std::vector<std::string> _disabledStateProperties;

  // File containing an initial sequence to run before starting
  std::string _initialSequenceFilePath;
};

} // namespace jaffarPlus