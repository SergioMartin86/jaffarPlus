#pragma once

#include <memory>
#include <set>
#include <vector>
#include <cstdint>
#include <jaffarCommon/bitwise.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/deserializers/differential.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/serializers/differential.hpp>
#include "../games/gameList.hpp"
#include "game.hpp"
#include "inputSet.hpp"

namespace jaffarPlus
{

class Runner final
{
  public:

  // Base constructor
  Runner(std::unique_ptr<Game> &game, const nlohmann::json &config)
    : _game(std::move(game))
  {
    _hashStepTolerance = jaffarCommon::json::getNumber<uint32_t>(config, "Hash Step Tolerance");

    const auto &inputHistoryJs = jaffarCommon::json::getObject(config, "Store Input History");
    _inputHistoryEnabled       = jaffarCommon::json::getBoolean(inputHistoryJs, "Enabled");
    _inputHistoryMaxSize       = jaffarCommon::json::getNumber<uint32_t>(inputHistoryJs, "Max Size (Steps)");

    // Storing game inputs for delayed parsing
    _possibleInputsJs = jaffarCommon::json::getArray<nlohmann::json>(config, "Possible Inputs");
  }

  void initialize()
  {
    if (_isInitialized == true) JAFFAR_THROW_LOGIC("This runner instance was already initialized");

    // Initializing emulator, if not already initialized
    if (_game->isInitialized() == false) _game->initialize();

    // Parsing possible game inputs
    for (const auto &inputSetJs : _possibleInputsJs) _inputSets.insert(std::move(parseInputSet(inputSetJs)));

    // If storing input history, allocate input history storage
    if (_inputHistoryEnabled == true)
    {
      // Calculating bit storage for the possible inputs index
      _inputIndexSizeBits = jaffarCommon::bitwise::getEncodingBitsForElementCount(_currentInputIndex);

      // Total size in bits for the input history
      size_t inputHistorySizeBits = _inputHistoryMaxSize * _inputIndexSizeBits;

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

  std::unique_ptr<InputSet> parseInputSet(const nlohmann::json &inputSetJs)
  {
    // Checking format
    if (inputSetJs.is_object() == false) JAFFAR_THROW_LOGIC("[ERROR] Input set provided must be a JSON object type. Dump: %s.\n", inputSetJs.dump(2).c_str());

    // Creating new input set to add
    auto inputSet = std::make_unique<InputSet>();

    // Getting input set condition array
    const auto &conditions = jaffarCommon::json::getArray<nlohmann::json>(inputSetJs, "Conditions");

    // Getting input string array
    const auto &inputsJs = jaffarCommon::json::getArray<nlohmann::json>(inputSetJs, "Inputs");

    // Parsing input set conditions
    for (const auto &condition : conditions) inputSet->addCondition(_game->parseCondition(condition));

    // Parsing input set inputs
    for (const auto &inputJs : inputsJs)
    {
      // Checking format
      if (inputJs.is_string() == false) JAFFAR_THROW_LOGIC("[ERROR] Inputs provided must be of string type. Dump: %s.\n", inputSetJs.dump(2).c_str());

      // Getting string input
      const auto &input = inputJs.get<std::string>();

      // Getting input hash
      const auto inputHash = jaffarCommon::hash::hashString(input);

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
    for (const auto &inputSet : _inputSets)
      if (inputSet->evaluate() == true) possibleInputs.insert(inputSet->getInputIndexes().begin(), inputSet->getInputIndexes().end());

    // Return possible inputs
    return possibleInputs;
  }

  // Function to advance state.
  __INLINE__ jaffarPlus::InputSet::inputIndex_t getInputIndex(const std::string &input) const
  {
    // Getting input hash
    const auto inputHash = jaffarCommon::hash::hashString(input);

    // Getting input index from the hash map
    auto it = _inputHashMap.find(inputHash);
    if (it == _inputHashMap.end()) JAFFAR_THROW_LOGIC("[ERROR] Input '%s' provided but has not been registered as allowed input first.\n", input.c_str());
    const auto inputIndex = it->second;

    return inputIndex;
  }

  // Function to advance state.
  void advanceState(const InputSet::inputIndex_t inputIdx)
  {
    // Safety check
    if (_inputStringMap.contains(inputIdx) == false) JAFFAR_THROW_RUNTIME("Move Index %u not found in runner\n", inputIdx);

    // Getting input string
    const auto &inputString = _inputStringMap[inputIdx];

    // Performing the requested input
    _game->advanceState(inputString);

    // If storing input history, do it now
    if (_inputHistoryEnabled == true)
    {
      // Checking we haven't exeeded maximum step
      if (_currentStep >= _inputHistoryMaxSize)
        JAFFAR_THROW_RUNTIME("[ERROR] Trying to advance step when storing input history and the maximum step (%lu) has been reached\n", _inputHistoryMaxSize);

      // Storing the new more in the input history
      setInput(_currentStep, inputIdx);
    }

    // Advancing step counter
    _currentStep++;
  }

  __INLINE__ void setInput(const size_t stepId, const InputSet::inputIndex_t inputIdx)
  {
    // Using bit-encoded copy to store the new input
    jaffarCommon::bitwise::bitcopy(_inputHistory.data(), _inputHistory.size(), stepId, &inputIdx, sizeof(InputSet::inputIndex_t), 0, 1, _inputIndexSizeBits);
  }

  __INLINE__ InputSet::inputIndex_t getInput(const size_t stepId) const
  {
    // Temporary storage for the new input
    InputSet::inputIndex_t inputIdx = 0;

    // Using bit-encoded copy to store the new input
    jaffarCommon::bitwise::bitcopy(&inputIdx, sizeof(InputSet::inputIndex_t), 0, _inputHistory.data(), _inputHistory.size(), stepId, 1, _inputIndexSizeBits);

    // Returning value
    return inputIdx;
  }

  // Serialization routine
  __INLINE__ void serializeState(jaffarCommon::serializer::Base &serializer) const
  {
    // Performing differential serialization of the internal game instance
    _game->serializeState(serializer);

    // Serializing input history data
    if (_inputHistoryEnabled == true) serializer.push(_inputHistory.data(), _inputHistory.size());

    // Serializing current step
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
  }

  // Deeserialization routine
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer)
  {
    // Performing differential serialization of the internal game instance
    _game->deserializeState(deserializer);

    // Deserializing input history
    if (_inputHistoryEnabled == true) deserializer.pop(_inputHistory.data(), _inputHistory.size());

    // Deserializing current step
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
  }

  // Getting the maximum differntial state size
  __INLINE__ size_t getStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    this->serializeState(s);
    return s.getOutputSize();
  }

  // Getting the maximum differntial state size
  __INLINE__ size_t getDifferentialStateSize(const size_t maxDifferences) const
  {
    jaffarCommon::serializer::Differential s;
    this->serializeState(s);
    size_t contiguousSize = s.getOutputSize();
    return contiguousSize + maxDifferences;
  }

  // This function computes the hash for the current runner state
  __INLINE__ jaffarCommon::hash::hash_t computeHash() const
  {
    // Storage for hash calculation
    MetroHash128 hashEngine;

    // Calculating hash tolerance stage
    auto hashStepToleranceStage = getHashStepToleranceStage();

    // Hashing step tolerance stage
    hashEngine.Update(hashStepToleranceStage);

    // Processing hashing from the game proper
    _game->computeHash(hashEngine);

    jaffarCommon::hash::hash_t result;
    hashEngine.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  // Function to dump current inputs to a file
  std::string getInputHistoryString() const
  {
    // Fail if input history is not enabled
    if (_inputHistoryEnabled == false) return "";

    // Getting the history into a string
    std::string inputHistoryString;

    // For each entry, add the input string
    for (size_t i = 0; i < _currentStep; i++)
    {
      // Getting input index
      const auto inputIdx = getInput(i);

      // Safety check
      if (_inputStringMap.contains(inputIdx) == false) JAFFAR_THROW_RUNTIME("Move Index %u not found in runner\n", inputIdx);

      // Getting input string
      const std::string &inputString = _inputStringMap.at(inputIdx);

      // Adding it to the story
      inputHistoryString += inputString;
      inputHistoryString += "\n";
    }

    return inputHistoryString;
  }

  // Function to print relevant information
  void printInfo() const
  {
    // Getting state hash
    const auto hash = jaffarCommon::hash::hashToString(computeHash());

    // Calculating hash tolerance stage
    auto hashStepToleranceStage = getHashStepToleranceStage();

    // Memory usage
    jaffarCommon::logger::log("[J++]  + Input History Enabled: %s\n", _inputHistoryEnabled ? "true" : "false");
    if (_inputHistoryEnabled == true)
    {
      jaffarCommon::logger::log("[J++]    + Possible Input Count: %u (Encoded in %lu bits)\n", _currentInputIndex, _inputIndexSizeBits);
      jaffarCommon::logger::log(
        "[J++]    + Input History Size: %u steps (%lu Bytes, %lu Bits)\n", _inputHistoryMaxSize, _inputHistory.size(), _inputIndexSizeBits * _inputHistoryMaxSize);
    }

    // Printing runner state
    jaffarCommon::logger::log("[J++]  + Current Step: %u\n", _currentStep);
    jaffarCommon::logger::log("[J++]  + Hash: %s\n", hash.c_str());
    jaffarCommon::logger::log("[J++]  + Hash Step Tolerance Stage: %u / %u\n", hashStepToleranceStage, _hashStepTolerance);

    // Getting possible inputs
    const auto &possibleInputs = getPossibleInputs();

    // Printing them
    jaffarCommon::logger::log("[J++]  + Possible Inputs:\n");
    for (const auto inputIdx : possibleInputs) jaffarCommon::logger::log("[J++]    + '%s'\n", _inputStringMap.at(inputIdx).c_str());
  }

  __INLINE__ uint32_t getHashStepToleranceStage() const { return _currentStep % (_hashStepTolerance + 1); }
  __INLINE__ Game    *getGame() const { return _game.get(); }

  __INLINE__ bool   getInputHistoryEnabled() const { return _inputHistoryEnabled; }
  __INLINE__ size_t getInputHistoryMaximumStep() const { return _inputHistoryMaxSize; }

  // Function to obtain runner based on game and emulator choice
  static std::unique_ptr<Runner> getRunner(const nlohmann::json &emulatorConfig, const nlohmann::json &gameConfig, const nlohmann::json &runnerConfig)
  {
    // Getting Game
    auto game = Game::getGame(emulatorConfig, gameConfig);

    // Creating new runner
    auto r = std::make_unique<Runner>(game, runnerConfig);

    // Returning runner
    return r;
  }

  __INLINE__ bool isInitialized() const { return _isInitialized; }

  private:

  // Pointer to the game instance
  std::unique_ptr<Game> _game;

  // Maximum step (max input history)
  uint32_t _inputHistoryMaxSize;

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
  std::map<jaffarCommon::hash::hash_t, InputSet::inputIndex_t> _inputHashMap;

  // Map for getting the allowed input from index
  std::map<InputSet::inputIndex_t, std::string> _inputStringMap;

  // Set of allowed input sets
  std::unordered_set<std::unique_ptr<InputSet>> _inputSets;

  // JSON of possible inputs stored for delayed parsing
  std::vector<nlohmann::json> _possibleInputsJs;

  // Stores whether the game has been initialized
  bool _isInitialized = false;
};

} // namespace jaffarPlus