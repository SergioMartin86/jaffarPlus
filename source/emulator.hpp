#pragma once

#include <SDL2/SDL.h>
#include <inputParser.hpp>
#include <inputSet.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <string>

namespace jaffarPlus
{

// A property is a contiguous segment of memory with size, identifiable by name
struct property_t
{
  uint8_t* pointer;
  size_t   size;
};

class Emulator
{
public:
  // Struct that holds the string of an input together with its emulator-specific input data
  struct inputEntry_t
  {
    // Input string
    std::string inputString;

    // Emulator-specific input data
    jaffar::input_t inputData;
  };

  // Constructor must only do configuration parsing to perform dry runs
  Emulator(const nlohmann::json& config)
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

  // Function to register inputs, to prevent the emulator from having to decode string inputs every time
  // If the entry is already registered, the registration function will simply return the previously registered id
  __INLINE__ InputSet::inputIndex_t registerInput(const std::string inputString)
  {
    // Registration is done only at the beginning with linear complexity O(n) to optimize for read access O(1)
    for (size_t i = 0; i < _inputMap.size(); i++)
      if (inputString == _inputMap[i].inputString) return i;

    // Otherwise, getting decoded input data from the emulator
    auto inputData = getInputParser()->parseInputString(inputString);

    // Otherwise, register it as a new entry
    _inputMap.push_back({inputString, inputData});

    // Returning current index
    return _inputMap.size() - 1;
  }

  // Function to return the information about a previously registered input
  __INLINE__ const inputEntry_t& getRegisteredInput(const InputSet::inputIndex_t inputIdx) const { return _inputMap[(size_t)inputIdx]; }

  // State advancing function
  void advanceState(const InputSet::inputIndex_t input) { advanceStateImpl(_inputMap[input].inputData); };

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

  virtual void serializeState(jaffarCommon::serializer::Base& serializer) const = 0;
  virtual void deserializeState(jaffarCommon::deserializer::Base& deserializer) = 0;

  // Function to get a reference to the input parser from the base emulator
  virtual jaffar::InputParser* getInputParser() const = 0;

  // Function to print debug information, whatever it might be
  virtual void printInfo() const = 0;

  // Get a property by name
  virtual property_t getProperty(const std::string& propertyName) const = 0;

  // Function to obtain emulator based on name
  static std::unique_ptr<Emulator> getEmulator(const nlohmann::json& emulatorConfig);

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
  virtual void serializeRendererState(jaffarCommon::serializer::Base& serializer) const = 0;

  // This function pushes the necessary data for output rendering of a given state/frame
  virtual void deserializeRendererState(jaffarCommon::deserializer::Base& deserializer) = 0;

  // This function returns the size of the renderer state
  virtual size_t getRendererStateSize() const = 0;

  // Shows the contents of the emulator's renderer into the window
  virtual void showRender() = 0;

protected:
  // Function to advance state
  virtual void advanceStateImpl(const jaffar::input_t& input) = 0;

  // Emulator name (for runtime use)
  std::string _emulatorName;

  // Stores whether the emulator has been initialized
  bool _isInitialized = false;

private:
  // Storage that maps an input id to its input data
  std::vector<inputEntry_t> _inputMap;
};

} // namespace jaffarPlus