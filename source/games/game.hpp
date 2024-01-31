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
  Game(std::unique_ptr<Emulator> emulator) : _emulator(std::move(emulator)) {};
  Game() = delete;
  virtual ~Game() = default;

  // Function to advance state. Returns a vector with the performed inputs (including skip frames)
  virtual std::vector<std::string> advanceState(const std::string& input)
  {
    // Performing the requested input
    const auto performedInputs = advanceStateImpl(input);

    // Updating derived values
    updateDerivedValues();

    // Then re-evaluate rules
    // evaluateRules();

    return performedInputs;
  }

  // Serialization routine -- simply a call to the underlying emulator
  void popState(uint8_t* outputStateData) const { _emulator->serializeState(outputStateData); }

  // Deserialization routine
  void pushState(const uint8_t* inputStateData)
  {
     // 
     _emulator->deserializeState(inputStateData); 
     
     // Updating the game's derived values
     updateDerivedValues();

     // Then re-evaluate rules
     // evaluateRules();
  }

  // This function computes the hash for the current state
  virtual hash_t computeHash() const = 0;

  // This function updates derivate values (those who require calculation from raw values) after a state is re-loaded
  virtual void updateDerivedValues() = 0;

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

  virtual void printStateInfoImpl() const = 0;
  virtual std::vector<std::string> advanceStateImpl(const std::string& input) = 0;

  // Underlying emulator instance
  const std::unique_ptr<Emulator> _emulator;
};

} // namespace jaffarPlus