#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <set>
#include <xdelta3/xdelta3.h>
#include <common/bitwise.hpp>
#include <common/hash.hpp>
#include <common/json.hpp>
#include "game.hpp"
#include "inputSet.hpp"


namespace jaffarPlus
{

class Runner final
{
 public:

   // Base constructor
  Runner(std::unique_ptr<Game>& game, const nlohmann::json& config) : _game(std::move(game))
  {
   _hashStepTolerance = JSON_GET_NUMBER(uint32_t, config, "Hash Step Tolerance");

    const auto& inputHistoryJs = JSON_GET_OBJECT(config, "Store Input History");
    _inputHistoryEnabled = JSON_GET_BOOLEAN(inputHistoryJs, "Enabled");
    _maximumStep = JSON_GET_NUMBER(uint32_t, inputHistoryJs, "Maximum Step");
    
    const auto& stateCompressionJs = JSON_GET_OBJECT(config, "State Compression");
    _useDifferentialCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Differential Compression");
    _maximumDifferences = JSON_GET_NUMBER(size_t, stateCompressionJs, "Max Difference (bytes)");
    _useDifferentialCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Zlib Compression");
  }

  void parseGameInputs(const nlohmann::json& gameInputsJs)
  {
    for (const auto& inputSetJs : gameInputsJs) _inputSets.insert(std::move(parseInputSet(inputSetJs)));

    // If storing input history, allocate input history storage
    if (_inputHistoryEnabled == true)
    {
      // Calculating bit storage for the possible inputs index
      _inputIndexSizeBits = getEncodingBitsForElementCount(_currentInputIndex);

      // Total size in bits for the input history
      size_t inputHistorySizeBits = _maximumStep * _inputIndexSizeBits;

      // Total size in bytes
      _inputHistorySizeBytes = inputHistorySizeBits / 8;

      // Add one byte if not perfectly divisible by 8
      if (inputHistorySizeBits % 8 > 0) _inputHistorySizeBytes++;

      // Allocating storage now
      _inputHistory.resize(_inputHistorySizeBytes);

      // Clearing storage (set to zero)
      memset(_inputHistory.data(), 0, _inputHistorySizeBytes);
    }
  }

  std::unique_ptr<InputSet> parseInputSet(const nlohmann::json& inputSetJs)
  {
    // Checking format
    if (inputSetJs.is_object() == false) EXIT_WITH_ERROR("[ERROR] Input set provided must be a JSON object type. Dump: %s.\n", inputSetJs.dump(2).c_str());

    // Creating new input set to add
    auto inputSet = std::make_unique<InputSet>();

    // Getting input set condition array
    const auto& conditions = JSON_GET_ARRAY(inputSetJs, "Conditions");

    // Getting input string array
    const auto& inputsJs = JSON_GET_ARRAY(inputSetJs, "Inputs");

    // Parsing input set conditions
    for (const auto& condition : conditions) inputSet->addCondition(_game->parseCondition(condition));  

    // Parsing input set inputs
    for (const auto& inputJs : inputsJs)
    {
      // Checking format
      if (inputJs.is_string() == false) EXIT_WITH_ERROR("[ERROR] Inputs provided must be of string type. Dump: %s.\n", inputSetJs.dump(2).c_str());

      // Getting string input
      const auto& input = inputJs.get<std::string>();

      // Getting input hash
      const auto inputHash = hashString(input);

      // Getting index for the new input
      InputSet::inputIndex_t inputIdx = 0;
      
      // Checking if input has already been added globally. If not, add it
      if (_inputHashMap.contains(inputHash) == false)
      {
        // Getting input index and advancing it
        inputIdx = _currentInputIndex++;

        // Adding new input hash->index to the map
        _inputHashMap[inputHash] = inputIdx;

        // Adding new input index->string to the map
        _inputStringMap[inputIdx] = input;
      }

      // If it is, just get it from there
      if (_inputHashMap.contains(inputHash) == true) inputIdx = _inputHashMap[inputHash];

      // Register the new input
      inputSet->addInput(inputIdx);
    }

    return inputSet;
  }

  std::set<InputSet::inputIndex_t> getPossibleInputs() const
  {
    // Storage for the possible input set
    std::set<InputSet::inputIndex_t> possibleInputs;

    // For all registered input sets, see which ones satisfy their conditions and add them
    for (const auto& inputSet : _inputSets)
     if (inputSet->evaluate() == true)
       possibleInputs.insert(inputSet->getInputIndexes().begin(), inputSet->getInputIndexes().end());

    // Return possible inputs
    return possibleInputs;
  }
  
  // Function to advance state.
  void advanceState(const std::string& input)
  {
    // Getting input hash
    const auto inputHash = hashString(input);

    // Getting input index from the hash map
    auto it = _inputHashMap.find(inputHash);
    if (it == _inputHashMap.end()) EXIT_WITH_ERROR("[ERROR] Input '%s' provided but has not been registered as allowed input first.\n", input.c_str());
    const auto inputIndex = it->second;

    advanceState(inputIndex);
  }

    // Function to advance state.
  void advanceState(const InputSet::inputIndex_t inputIdx)
  {
    // Getting input string
    const auto& inputString = _inputStringMap[inputIdx];

    // Performing the requested input
    _game->advanceState(inputString);

    // If storing input history, do it now
    if (_inputHistoryEnabled == true) bitcopy(_inputHistory.data(), _currentStep, (const uint8_t*)&inputIdx, 0, 1, _inputIndexSizeBits);

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

    // If enabled, store input history    
    if (_inputHistoryEnabled == true)
    {
      memcpy(&outputStateData[pos], _inputHistory.data(), _inputHistorySizeBytes);
      pos += _inputHistorySizeBytes;
    }

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

    // If enabled, recover input history    
    if (_inputHistoryEnabled == true)
    {
      memcpy(_inputHistory.data(), &inputStateData[pos], _inputHistorySizeBytes);
      pos += _inputHistorySizeBytes;
    }

    // Deserializing state data into the emulator
    _game->deserializeState(&inputStateData[pos]); 
    pos += _game->getStateSize();
  }

  // Differential serialization routine
  inline void serializeDifferentialState(uint8_t* outputStateData, uint8_t* referenceState) const
  { 

  }

  // This function returns the size of the game state
  size_t getStateSize() const
  {
    size_t stateSize = 0;

    // Adding the size of game specific storage
    stateSize += sizeof(_currentStep);

    // If enabled, factor in the size of input history    
    if (_inputHistoryEnabled == true) stateSize += _inputHistorySizeBytes;

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

   // Memory usage
   LOG("[J+]  + Input History Enabled: %s\n", _inputHistoryEnabled ? "true" : "false");
   if (_inputHistoryEnabled == true)
   {
    LOG("[J+]    + Possible Input Count: %u (Encoded in %lu bits)\n", _currentInputIndex, _inputIndexSizeBits);
    LOG("[J+]    + Input History Size: %u steps (%lu Bytes, %lu Bits)\n", _maximumStep, _inputHistorySizeBytes, _inputIndexSizeBits * _maximumStep);
   }

   // Printing runner state
   LOG("[J+]  + Current Step: %u\n", _currentStep);
   LOG("[J+]  + Hash: %s\n", hash.c_str());
   LOG("[J+]  + Hash Step Tolerance Stage: %u / %u\n", hashStepToleranceStage, _hashStepTolerance);

   // Getting possible inputs
   const auto& possibleInputs = getPossibleInputs();

   // Printing them
   LOG("[J+]  + Possible Inputs:\n");
   for (const auto inputIdx : possibleInputs) 
    printf("[J+]    + '%s'\n", _inputStringMap.at(inputIdx).c_str());
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
  bool _inputHistoryEnabled;

  // Total size for the input history in bytes
  size_t _inputHistorySizeBytes;

  // Maximum size of input index in bits
  size_t _inputIndexSizeBits;

  // Storage for the input history
  std::vector<uint8_t> _inputHistory;

  // Whether to use differential compression
  bool _useDifferentialCompression;

  // Maximum number of difference bytes to tolerate if using differential compression
  size_t _maximumDifferences;

  // Whether to use Zlib compression
  bool _useZlibCompression;

  ///////////////////////////////
  // Input processing variables

  // Storage for the index to use to register a new input. Should start at zero
  InputSet::inputIndex_t _currentInputIndex = 0;

  // Hash map for input indexing
  std::map<hash_t, InputSet::inputIndex_t> _inputHashMap;

  // Map for getting the allowed input from index
  std::map<InputSet::inputIndex_t, std::string> _inputStringMap;

  // Set of allowed input sets
  std::unordered_set<std::unique_ptr<InputSet>> _inputSets;
};

} // namespace jaffarPlus