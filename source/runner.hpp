#pragma once

#include <cstdint>
#include <memory>
#include <common/hash.hpp>
#include <common/json.hpp>
#include "game.hpp"

namespace jaffarPlus
{

class Runner final
{
 public:

   // Base constructor
  Runner(std::unique_ptr<Game>& game, const nlohmann::json& config) : _game(std::move(game))
  {
   _hashStepTolerance = JSON_GET_NUMBER(uint32_t, config, "Hash Step Tolerance");

    const auto& inputHistoryJs = JSON_GET_OBJECT(config, "Input History");
    _storeInputHistory = JSON_GET_BOOLEAN(inputHistoryJs, "Store Input History");
    _maximumStep = JSON_GET_NUMBER(uint32_t, inputHistoryJs, "Maximum Step");
    
    const auto& stateCompressionJs = JSON_GET_OBJECT(config, "State Compression");
    _useDifferentialCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Differential Compression");
    _maximumDifferences = JSON_GET_NUMBER(size_t, stateCompressionJs, "Max Difference (bytes)");
    _useDifferentialCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Zlib Compression");

    // If storing input history, allocate input history storage
    if (_storeInputHistory == true)
    {
      // Getting all possible inputs
      const auto possibleInputs = _game->getAllowedInputs();

      // Calculating bit storage for the possible inputs index
      const size_t possibleInputSize = possibleInputs.size();
      size_t indexSizeBits = 0;
      size_t indexCapacity = 1;
      while (indexCapacity < possibleInputSize) { indexCapacity <<= 1, indexSizeBits++; }; 

      // Total size in bits for the input history
      size_t inputHistorySizeBits = _maximumStep * indexSizeBits;

      // Total size in bytes
      _inputHistorySizeBytes = inputHistorySizeBits / 8;

      // Add one byte if not perfectly divisible by 8
      if (inputHistorySizeBits % 8 > 0) _inputHistorySizeBytes++;

      printf("Input size: %lu, Index bit capacity: %lu (bits: %lu)\n", possibleInputSize, indexCapacity, indexSizeBits);
      printf("Input History Size Bytes: %lu (Bits: %lu)\n", _inputHistorySizeBytes, inputHistorySizeBits);

      // Allocating storage now
      _inputHistory = std::make_unique<uint8_t>(_inputHistorySizeBytes);
    }
  }

  // Function to advance state. Returns a vector with the performed inputs (including skip frames)
  void advanceState(const std::string& input)
  {
    // Performing the requested input
    _game->advanceState(input);

    // Advancing step counter
    _currentStep++;
  }

  // Serialization routine 
  inline void serializeState(uint8_t* outputStateData) const
  { 
     size_t pos = 0;

     // Serializing runner-specific data
     memcpy(&outputStateData[pos], &_currentStep, sizeof(_currentStep));
     pos += sizeof(_currentStep);

     // Serializing internal emulator state
     _game->serializeState(&outputStateData[pos]);
     pos += _game->getStateSize();
  }

  // Deserialization routine
  inline void deserializeState(const uint8_t* inputStateData)
  {
     size_t pos = 0;

     // Deserializing game-specific data
     memcpy(&_currentStep, &inputStateData[pos], sizeof(_currentStep));
     pos += sizeof(_currentStep);

     // Deserializing state data into the emulator
     _game->deserializeState(&inputStateData[pos]); 
     pos += _game->getStateSize();
  }

  // This function returns the size of the game state
  size_t getStateSize() const
  {
    size_t stateSize = 0;

    // Adding the size of game specific storage
    stateSize += sizeof(_currentStep);

    // Adding the size of the emulator state
    stateSize += _game->getStateSize();

    return stateSize;
  }

  // This function computes the hash for the current runner state
  inline hash_t computeHash() const 
  {
    // Storage for hash calculation
    MetroHash128 hashEngine;

    // Calculating hash tolerance stage
    auto hashStepToleranceStage = getHashStepToleranceStage();

    // Hashing step tolerance stage
    hashEngine.Update(hashStepToleranceStage);

    // Processing hashing from the game proper
    _game->computeHash(hashEngine);
 
    hash_t result;
    hashEngine.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  // Function to print
  void printInfo() const
  {
   // Getting state hash 
   const auto hash = hashToString(computeHash());

   // Calculating hash tolerance stage
   auto hashStepToleranceStage = getHashStepToleranceStage();

   // Printing runner state
   LOG("[J+]  + Current Step: %u\n", _currentStep);
   LOG("[J+]  + Hash: %s\n", hash.c_str());
   LOG("[J+]  + Hash Step Tolerance Stage: %u / %u\n", hashStepToleranceStage, _hashStepTolerance);
  }

  inline uint32_t getHashStepToleranceStage() const { return  _currentStep % (_hashStepTolerance + 1); }
      
  inline Game* getGame() const { return _game.get(); }

 private:

  // Pointer to the game instance
  std::unique_ptr<Game> _game;

  // Maximum step (max input history)
  uint32_t _maximumStep;

  // Storage for the current step of the runner
  uint32_t _currentStep = 0;

  // Storage for the hash step tolerance
  uint32_t _hashStepTolerance;

  // Storage for the calculated hash step tolerance stage
  uint32_t _hashStepToleranceStage;

  // Specifies whether to store the input history
  bool _storeInputHistory;

  // Total size for the input history in bytes
  size_t _inputHistorySizeBytes;

  // Storage for the input history
  std::unique_ptr<uint8_t> _inputHistory;

  // Whether to use differential compression
  bool _useDifferentialCompression;

  // Maximum number of difference bytes to tolerate if using differential compression
  size_t _maximumDifferences;

  // Whether to use Zlib compression
  bool _useZlibCompression;

  // Type for input indexing
  typedef uint32_t inputIndex_t; 

  // Hash map for input indexing
  std::map<hash_t, inputIndex_t> _inputHashMap;
};

} // namespace jaffarPlus