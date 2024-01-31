#pragma once

#include <set>
#include <memory>
#include <common/hash.hpp>
#include <common/json.hpp>
#include <emulators/emulator.hpp>

namespace jaffarPlus
{

class Game
{
 public:

  // Base constructor
  Game(std::unique_ptr<Emulator>& emulator, const nlohmann::json& config) : _emulator(std::move(emulator))
  {
    // Parse anything that might be generally relevant here
  };

  Game() = delete;
  virtual ~Game() = default;

  // Function to initialize game instance
  void initialize()
  {
    // Parsing and allocating game-specific storage
    _gameSpecificStorageSize = getGameSpecificStorageSize();
    _gameSpecificStorage = (uint8_t*)calloc(_gameSpecificStorageSize, sizeof(uint8_t));

    // Initializing the particular game selected
    initializeImpl();

    // Updating game specific values
    updateGameSpecificValues();

    // Then re-evaluate rules
    // evaluateRules();
  }


  // Function to advance state. Returns a vector with the performed inputs (including skip frames)
  virtual std::vector<std::string> advanceState(const std::string& input)
  {
    // Performing the requested input
    const auto performedInputs = advanceStateImpl(input);

    // Updating derived values
    updateGameSpecificValues();

    // Then re-evaluate rules
    // evaluateRules();

    return performedInputs;
  }

  // Serialization routine -- simply a call to the underlying emulator
  void serializeState(uint8_t* outputStateData) const
  { 
     size_t pos = 0;

     // Serializing game-specific data
     memcpy(&outputStateData[pos], _gameSpecificStorage, _gameSpecificStorageSize);
     pos += _gameSpecificStorageSize;

     // Serializing internal emulator state
     _emulator->serializeState(&outputStateData[pos]);
     pos += _emulator->getStateSize();
  }

  // Deserialization routine
  void deserializeState(const uint8_t* inputStateData)
  {
     size_t pos = 0;

     // Deserializing game-specific data
     memcpy(_gameSpecificStorage, &inputStateData[pos], _gameSpecificStorageSize);
     pos += _gameSpecificStorageSize;

     // Deserializing state data into the emulator
     _emulator->deserializeState(&inputStateData[pos]); 
     pos += _emulator->getStateSize();

     // Updating the game's derived values
     updateGameSpecificValues();

     // Then re-evaluate rules
     // evaluateRules();
  }

  // This function returns the size of the game state
  size_t getStateSize() const
  {
    size_t stateSize = 0;

    // Adding the size of game specific storage
    stateSize += _gameSpecificStorageSize;

    // Adding the size of the emulator state
    stateSize += _emulator->getStateSize();

    return stateSize;
  }

  // This function computes the hash for the current state
  virtual hash_t computeHash() const = 0;

  // This function updates derivate values (those who require calculation from raw values) after a state is re-loaded
  virtual void updateGameSpecificValues() = 0;

  // Function to print
  void printStateInfo() const
  {
   const auto hash = hashToString(computeHash());
   LOG("[J+]  + Hash:                               %s\n", hash.c_str());

   printStateInfoImpl();
  }

  // Function to get emulator name
  static std::string getName();

  // Returns pointer to the internal emulator
  Emulator* getEmulator() { return _emulator.get(); }

  // Function to obtain emulator based on name
  static std::unique_ptr<Game> getGame(const std::string& gameName, std::unique_ptr<Emulator>& emulator, const nlohmann::json& config);

  protected:

  virtual void initializeImpl() = 0;
  virtual size_t getGameSpecificStorageSize() const = 0;
  virtual void printStateInfoImpl() const = 0;
  virtual std::vector<std::string> advanceStateImpl(const std::string& input) = 0;
  
  // Underlying emulator instance
  const std::unique_ptr<Emulator> _emulator;

  // Game-Specific storage size and pointer
  size_t _gameSpecificStorageSize;
  uint8_t* _gameSpecificStorage;
};

} // namespace jaffarPlus