#pragma once

#include <memory>
#include <unordered_set>
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
#include <gameList.hpp>
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
    _inputHistoryMaxSize       = jaffarCommon::json::getNumber<uint32_t>(inputHistoryJs, "Max Size");

    // Storing game inputs for delayed parsing
    _allowedInputSetsJs = jaffarCommon::json::getArray<nlohmann::json>(config, "Allowed Input Sets");

    // Print option: do not print allowed inputs
    _showAllowedInputs = jaffarCommon::json::getBoolean(config, "Show Allowed Inputs");

    // Print option: do not print placeholders for inputs not supported in this frame
    _showEmptyInputSlots = jaffarCommon::json::getBoolean(config, "Show Empty Input Slots");

    // Stores candidate inputs
    _testCandidateInputs = jaffarCommon::json::getBoolean(config, "Test Candidate Inputs");

    // Getting candidate input sets
    _candidateInputSetsJs = jaffarCommon::json::getArray<nlohmann::json>(config, "Candidate Input Sets");

    // Getting frame skip rate
    _frameskipRate = jaffarCommon::json::getNumber<size_t>(config, "Frameskip Rate");
  }

  void initialize()
  {
    if (_isInitialized == true) JAFFAR_THROW_LOGIC("This runner instance was already initialized");

    // Initializing game, if not already initialized
    if (_game->isInitialized() == false) _game->initialize();

    // Parsing possible game inputs
    for (const auto &inputSetJs : _allowedInputSetsJs) _allowedInputSets.push_back(parseInputSet(inputSetJs));

    // If testing candidate inputs, parse them now
    if (_testCandidateInputs == true)
      for (const auto &inputSetJs : _candidateInputSetsJs) _candidateInputSets.push_back(parseInputSet(inputSetJs));

    // If storing input history, allocate input history storage
    if (_inputHistoryEnabled == true)
      {
        // Calculating bit storage for the possible inputs index
        _inputIndexSizeBits = jaffarCommon::bitwise::getEncodingBitsForElementCount(_maxInputIndex);

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
    // Creating new input set to add
    auto inputSet = std::make_unique<InputSet>();

    // Getting input set condition array
    const auto &conditions = jaffarCommon::json::getArray<nlohmann::json>(inputSetJs, "Conditions");

    // Getting input string array
    const auto &inputsJs = jaffarCommon::json::getArray<std::string>(inputSetJs, "Inputs");

    // Parsing input set conditions
    for (const auto &condition : conditions) inputSet->addCondition(_game->parseCondition(condition));

    // Parsing input set inputs
    for (const auto &input : inputsJs) inputSet->addInput(registerInput(input));

    // Keep track of maximum size (for better output)
    _largestInputSetSize = std::max(_largestInputSetSize, inputSet->getInputIndexes().size());

    // Getting stop evaluating flag
    inputSet->setStopInputEvaluationFlag(jaffarCommon::json::getBoolean(inputSetJs, "Stop Input Evaluation"));

    // Returning new input set
    return inputSet;
  }

  InputSet::inputIndex_t registerInput(const std::string &input)
  {
    // Getting input hash
    const auto inputHash = jaffarCommon::hash::hashString(input);

    // Getting index for the new input
    InputSet::inputIndex_t inputIdx = _game->getEmulator()->registerInput(input);

    // Adding new input hash->index to the map
    _inputHashMap[inputHash] = inputIdx;

    // Adding new input index->string to the map
    _inputStringMap[inputIdx] = input;

    // Register maximum input index to determine how many bytes to use for input history storage
    _maxInputIndex = std::max(_maxInputIndex, inputIdx + 1);

    // Returning this input's index (either new or the one registered before)
    return inputIdx;
  }

  std::vector<InputSet::inputIndex_t> getInputsFromInputSets(const std::vector<std::unique_ptr<InputSet>> &inputSets) const
  {
    // Storage for the possible input set
    std::vector<InputSet::inputIndex_t> possibleInputs;

    // For all registered input sets, see which ones satisfy their conditions and add them
    for (const auto &inputSet : inputSets)
      if (inputSet->evaluate() == true)
        {
          possibleInputs.insert(possibleInputs.end(), inputSet->getInputIndexes().begin(), inputSet->getInputIndexes().end());

          // Getting stop evaluating flag
          bool stopEvaluating = inputSet->getStopInputEvaluationFlag();

          // If stop evaluation is set, then return now
          if (stopEvaluating) break;
      }

    // Return possible inputs
    return possibleInputs;
  }

  __INLINE__ const auto getAllowedInputs() const
  {
    // Getting possible inputs based on the configuration file
    auto allowedInputs = getInputsFromInputSets(_allowedInputSets);

    // Getting additional inputs based on the custom game function
    _game->getAdditionalAllowedInputs(allowedInputs);

    return allowedInputs;
  }

  __INLINE__ const auto getCandidateInputs() const { return getInputsFromInputSets(_candidateInputSets); }

  // Function to get the input index of a given input
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

  bool isInputAllowed(const std::string &inputString)
  {
    // Getting input hash
    const auto inputHash = jaffarCommon::hash::hashString(inputString);

    // Returning true if found; false, otherwise
    return _inputHashMap.contains(inputHash);
  }

  // Function to advance state.
  __INLINE__ void advanceState(const InputSet::inputIndex_t inputIdx)
  {
    // Safety check -- Disabled, for performance
    // if (_inputStringMap.contains(inputIdx) == false) JAFFAR_THROW_RUNTIME("Move Index %u not found in runner\n", inputIdx);

    // Performing the requested input
    _game->advanceState(inputIdx);

    // Pushing normal input into the history
    pushInput(inputIdx);

    // If frameskip was set, execute it now
    for (size_t i = 0; i < _frameskipRate; i++)
      {
        _game->advanceState(inputIdx);
        pushInput(inputIdx);
      }
  }

  __INLINE__ void pushInput(const InputSet::inputIndex_t inputIdx)
  {
    // If storing input history, do it now. Unless we've reached the maximum
    if (_inputHistoryEnabled == true && _currentInputCount < _inputHistoryMaxSize) setInput(_currentInputCount, inputIdx);

    // Advancing step counter
    _currentInputCount++;
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
    serializer.pushContiguous(&_currentInputCount, sizeof(_currentInputCount));
  }

  // Deeserialization routine
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base &deserializer)
  {
    // Performing differential serialization of the internal game instance
    _game->deserializeState(deserializer);

    // Deserializing input history
    if (_inputHistoryEnabled == true) deserializer.pop(_inputHistory.data(), _inputHistory.size());

    // Deserializing current step
    deserializer.popContiguous(&_currentInputCount, sizeof(_currentInputCount));
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

    // For each entry, add the input string up to the current step or the maximum size
    for (size_t i = 0; i < _currentInputCount && i < _inputHistoryMaxSize; i++)
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

  std::string getInputStringFromIndex(const InputSet::inputIndex_t input) { return _inputStringMap[input]; }

  // Function to print relevant information
  void printInfo() const
  {
    // Getting state hash
    const auto hash = jaffarCommon::hash::hashToString(computeHash());

    // Calculating hash tolerance stage
    auto hashStepToleranceStage = getHashStepToleranceStage();

    // Memory usage
    jaffarCommon::logger::log("[J+]  + Input History Enabled: %s\n", _inputHistoryEnabled ? "true" : "false");
    if (_inputHistoryEnabled == true)
      {
        jaffarCommon::logger::log("[J+]    + Possible Input Count: %u (Encoded in %lu bits)\n", _maxInputIndex, _inputIndexSizeBits);
        jaffarCommon::logger::log(
          "[J+]    + Input History Size: %u steps (%lu Bytes, %lu Bits)\n", _inputHistoryMaxSize, _inputHistory.size(), _inputIndexSizeBits * _inputHistoryMaxSize);
    }

    // Printing runner state
    jaffarCommon::logger::log("[J+]  + Current Input Count: %u\n", _currentInputCount);
    jaffarCommon::logger::log("[J+]  + Hash: %s\n", hash.c_str());
    jaffarCommon::logger::log("[J+]  + Hash Step Tolerance Stage: %u / %u\n", hashStepToleranceStage, _hashStepTolerance);

    // Printing frameskip information
    if (_frameskipRate > 0) jaffarCommon::logger::log("[J+]  + Frameskip Rate: %lu\n", _frameskipRate);

    // Check whether we want to print inputs
    if (_showAllowedInputs == true)
      {
        // Getting allowed inputs
        const auto &possibleInputs = getAllowedInputs();

        // Printing them
        jaffarCommon::logger::log("[J+]  + Allowed Inputs:\n");

        size_t currentInputIdx = 0;
        for (const auto inputIdx : possibleInputs)
          {
            jaffarCommon::logger::log("[J+]    + '%s'\n", _inputStringMap.at(inputIdx).c_str());
            currentInputIdx++;
          }

        if (_showEmptyInputSlots)
          for (; currentInputIdx < _largestInputSetSize; currentInputIdx++) jaffarCommon::logger::log("[J+]    + ----- \n");
    }
  }

  __INLINE__ uint32_t getHashStepToleranceStage() const { return _currentInputCount % (_hashStepTolerance + 1); }
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

  // Stores whether the game has been initialized
  bool _isInitialized = false;

  // Pointer to the game instance
  std::unique_ptr<Game> _game;

  // Maximum step (max input history)
  uint32_t _inputHistoryMaxSize;

  // Storage for the current step of the runner
  uint32_t _currentInputCount = 0;

  // Storage for the hash step tolerance
  uint32_t _hashStepTolerance;

  // Specifies whether to store the input history
  bool _inputHistoryEnabled;

  // Maximum size of input index in bits
  size_t _inputIndexSizeBits;

  // Storage for the input history
  std::vector<uint8_t> _inputHistory;

  ///////////////////////////////
  // Input processing variables
  //////////////////////////////

  // Storage for the maximum index to use to register a new input
  InputSet::inputIndex_t _maxInputIndex = 0;

  // Hash map for input indexing
  std::map<jaffarCommon::hash::hash_t, InputSet::inputIndex_t> _inputHashMap;

  // Map for getting the allowed input from index
  std::map<InputSet::inputIndex_t, std::string> _inputStringMap;

  ///////////////////////////////
  //  Frameskip. These are frames that are simply skipped over, but they inputs registered nevertheless
  //////////////////////////////

  // How many frames to skip
  size_t _frameskipRate;

  ///////////////////////////////////////////
  // Allowed and candidate input sets
  //////////////////////////////////////////

  // Allowed input configuration
  std::vector<nlohmann::json> _allowedInputSetsJs;

  // Candidate input configuration
  std::vector<nlohmann::json> _candidateInputSetsJs;

  // Set of candidate input sets
  std::vector<std::unique_ptr<InputSet>> _candidateInputSets;

  // Vector of allowed input sets
  std::vector<std::unique_ptr<InputSet>> _allowedInputSets;

  // Largest input set sizedetected
  size_t _largestInputSetSize = 0;

  // Whether to test for candidate inputs
  bool _testCandidateInputs;

  // Show a placeholder for inputs not supported in this frame
  bool _showEmptyInputSlots;

  // Show allowed inputs enable/disable
  bool _showAllowedInputs;
};

} // namespace jaffarPlus