#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <set>
#include <jaffarCommon/include/serializers/base.hpp>
#include <jaffarCommon/include/deserializers/base.hpp>
#include <jaffarCommon/include/serializers/contiguous.hpp>
#include <jaffarCommon/include/deserializers/contiguous.hpp>
#include <jaffarCommon/include/bitwise.hpp>
#include <jaffarCommon/include/hash.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/logger.hpp>
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
  }

  void parseGameInputs(const nlohmann::json& gameInputsJs)
  {
    for (const auto& inputSetJs : gameInputsJs) _inputSets.insert(std::move(parseInputSet(inputSetJs)));

    // If storing input history, allocate input history storage
    if (_inputHistoryEnabled == true)
    {
      // Calculating bit storage for the possible inputs index
      _inputIndexSizeBits = jaffarCommon::getEncodingBitsForElementCount(_currentInputIndex);

      // Total size in bits for the input history
      size_t inputHistorySizeBits = _maximumStep * _inputIndexSizeBits;

      // Total size in bytes
      size_t inputHistorySizeBytes = inputHistorySizeBits / 8;

      // Add one byte if not perfectly divisible by 8
      if (inputHistorySizeBits % 8 > 0) inputHistorySizeBytes++;

      // Allocating storage now
      _inputHistory.resize(inputHistorySizeBytes);

      // Clearing storage (set to zero)
      memset(_inputHistory.data(), 0, _inputHistory.size());
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
      const auto inputHash = jaffarCommon::hashString(input);

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
  inline jaffarPlus::InputSet::inputIndex_t getInputIndex(const std::string& input) const
  {
    // Getting input hash
    const auto inputHash = jaffarCommon::hashString(input);

    // Getting input index from the hash map
    auto it = _inputHashMap.find(inputHash);
    if (it == _inputHashMap.end()) EXIT_WITH_ERROR("[ERROR] Input '%s' provided but has not been registered as allowed input first.\n", input.c_str());
    const auto inputIndex = it->second;

    return inputIndex;
  }

    // Function to advance state.
  void advanceState(const InputSet::inputIndex_t inputIdx)
  {
    // Getting input string
    const auto& inputString = _inputStringMap[inputIdx];

    // Performing the requested input
    _game->advanceState(inputString);

    // If storing input history, do it now
    if (_inputHistoryEnabled == true) 
    {
      // Checking we haven't exeeded maximum step
      if (_currentStep >= _maximumStep) EXIT_WITH_ERROR("[ERROR] Trying to advance step when storing input history and the maximum step (%lu) has been reached\n", _maximumStep);
      
      // Storing the new more in the input history
      jaffarCommon::bitcopy(_inputHistory.data(), _currentStep, (const uint8_t*)&inputIdx, 0, 1, _inputIndexSizeBits);
    }

    // Advancing step counter
    _currentStep++;
  }

  // Serialization routine
  inline void serializeState(jaffarCommon::serializer::Base& serializer) const
  { 
    // Performing differential serialization of the internal game instance
    _game->serializeState(serializer);

    // Serializing input history data
    if (_inputHistoryEnabled == true) serializer.push(_inputHistory.data(), _inputHistory.size());

    // Serializing current step
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
  }

  // Deeserialization routine
  inline void deserializeState(jaffarCommon::deserializer::Base& deserializer)
  { 
    // Performing differential serialization of the internal game instance
    _game->deserializeState(deserializer);

    // Deserializing input history
    if (_inputHistoryEnabled == true) deserializer.pop(_inputHistory.data(), _inputHistory.size());

    // Deserializing current step
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
  }

  // Getting the maximum differntial state size
  inline size_t getStateSize() const
  { 
    jaffarCommon::serializer::Contiguous s;
    this->serializeState(s);
    return s.getOutputSize();
  }

  // Getting the maximum differntial state size
  inline size_t getDifferentialStateSize(const size_t maxDifferences) const
  { 
    jaffarCommon::serializer::Differential s;
    this->serializeState(s);
    size_t contiguousSize = s.getOutputSize();
    return contiguousSize + maxDifferences;
  }

  // This function computes the hash for the current runner state
  inline jaffarCommon::hash_t computeHash() const 
  {
    // Storage for hash calculation
    MetroHash128 hashEngine;

    // Calculating hash tolerance stage
    auto hashStepToleranceStage = getHashStepToleranceStage();

    // Hashing step tolerance stage
    hashEngine.Update(hashStepToleranceStage);

    // Processing hashing from the game proper
    _game->computeHash(hashEngine);
 
    jaffarCommon::hash_t result;
    hashEngine.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  // Function to print
  void printInfo() const
  {
   // Getting state hash 
   const auto hash = jaffarCommon::hashToString(computeHash());

   // Calculating hash tolerance stage
   auto hashStepToleranceStage = getHashStepToleranceStage();

   // Memory usage
   LOG("[J+]  + Input History Enabled: %s\n", _inputHistoryEnabled ? "true" : "false");
   if (_inputHistoryEnabled == true)
   {
    LOG("[J+]    + Possible Input Count: %u (Encoded in %lu bits)\n", _currentInputIndex, _inputIndexSizeBits);
    LOG("[J+]    + Input History Size: %u steps (%lu Bytes, %lu Bits)\n", _maximumStep, _inputHistory.size(), _inputIndexSizeBits * _maximumStep);
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

  // Maximum size of input index in bits
  size_t _inputIndexSizeBits;

  // Storage for the input history
  std::vector<uint8_t> _inputHistory;

  ///////////////////////////////
  // Input processing variables
  //////////////////////////////
  
  // Storage for the index to use to register a new input. Should start at zero
  InputSet::inputIndex_t _currentInputIndex = 0;

  // Hash map for input indexing
  std::map<jaffarCommon::hash_t, InputSet::inputIndex_t> _inputHashMap;

  // Map for getting the allowed input from index
  std::map<InputSet::inputIndex_t, std::string> _inputStringMap;

  // Set of allowed input sets
  std::unordered_set<std::unique_ptr<InputSet>> _inputSets;
};

} // namespace jaffarPlus