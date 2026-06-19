#pragma once

/**
 * @file runner.hpp
 * @brief Drives a Game forward one input at a time, managing the allowed/candidate input sets,
 *        frameskip, input-history recording and state hashing/serialization.
 */

#include "game.hpp"
#include "inputSet.hpp"
#include <atomic>
#include <cstdint>
#include <gameList.hpp>
#include <jaffarCommon/bitwise.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <memory>
#include <unordered_set>
#include <vector>

namespace jaffarPlus
{

/**
 * @brief Owns a @ref Game instance and advances it according to configured inputs.
 *
 * @details The runner parses allowed and candidate input sets from the configuration, registers
 * every input string to a numeric index, and on each step advances the game by one input (plus any
 * configured frameskip frames). It can record the sequence of applied inputs as a bit-packed input
 * history, serialize/deserialize its state together with the game's, and compute a hash of the
 * current state.
 */
class Runner final
{
public:
  /**
   * @brief Constructs a runner that takes ownership of @p game and reads its settings from @p config.
   * @param game   The game instance to drive; ownership is moved into the runner.
   * @param config Runner configuration object, supplying the keys "Hash Step Tolerance",
   *               "Store Input History" (with "Enabled" and "Max Size"), "Allowed Input Sets",
   *               "Show Allowed Inputs", "Show Empty Input Slots", "Test Candidate Inputs",
   *               "Candidate Input Sets", "Frameskip" (with "Rate", "Use Custom Input",
   *               "Custom Input") and "Bypass Hash Calculation".
   */
  Runner(std::unique_ptr<Game>& game, const nlohmann::json& config) : _game(std::move(game))
  {
    _hashStepTolerance = jaffarCommon::json::getNumber<uint32_t>(config, "Hash Step Tolerance");

    const auto inputHistoryJs = jaffarCommon::json::getObject(config, "Store Input History");
    _inputHistoryEnabled      = jaffarCommon::json::getBoolean(inputHistoryJs, "Enabled");
    _inputHistoryMaxSize      = jaffarCommon::json::getNumber<uint32_t>(inputHistoryJs, "Max Size");

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

    // Getting frame skip configuration
    const auto frameskipJs   = jaffarCommon::json::getObject(config, "Frameskip");
    _frameskipRate           = jaffarCommon::json::getNumber<size_t>(frameskipJs, "Rate");
    _frameskipUseCustomInput = jaffarCommon::json::getBoolean(frameskipJs, "Use Custom Input");
    _frameskipCustomInput    = jaffarCommon::json::getString(frameskipJs, "Custom Input");

    // Option to bypass hash calculation via MetroHash and get it straight from the game
    _bypassHashCalculation = jaffarCommon::json::getBoolean(config, "Bypass Hash Calculation");
  }

  /**
   * @brief Initializes the runner: initializes the game, parses input sets, and registers inputs.
   *
   * @details Initializes the game if needed, parses the allowed input sets (and candidate input sets
   * when "Test Candidate Inputs" is enabled), registers the inputs declared by the game's code,
   * resolves the custom frameskip input index, and allocates the bit-packed input-history storage
   * when input history is enabled.
   * @throws A logic error if the runner was already initialized.
   */
  void initialize()
  {
    if (_isInitialized == true) JAFFAR_THROW_LOGIC("This runner instance was already initialized");

    // Initializing game, if not already initialized
    if (_game->isInitialized() == false) _game->initialize();

    // Parsing possible game inputs
    for (const auto& inputSetJs : _allowedInputSetsJs) _allowedInputSets.push_back(parseInputSet(inputSetJs));

    // If testing candidate inputs, parse them now
    if (_testCandidateInputs == true)
      for (const auto& inputSetJs : _candidateInputSetsJs) _candidateInputSets.push_back(parseInputSet(inputSetJs));

    // Parsing inputs declared directly by the game's code
    for (const auto& input : _game->getAllPossibleInputs()) registerInput(input);

    // Resolve the custom frameskip input index once (it is constant), so advanceState() can reuse it
    // instead of re-hashing + re-looking-it-up every skipped frame. Also validates it early -- a bad
    // "Custom Input" now fails here at init rather than crashing deep in the simulation loop.
    if (_frameskipUseCustomInput) _frameskipCustomInputIdx = getInputIndex(_frameskipCustomInput);

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

  /**
   * @brief Builds an @ref InputSet from its JSON description.
   *
   * @details Parses the set's "Conditions" (via the game's condition parser) and "Inputs" (each
   * registered via @ref registerInput), updates @ref _largestInputSetSize, and applies the
   * "Stop Input Evaluation" flag.
   * @param inputSetJs JSON object describing the input set ("Conditions", "Inputs",
   *                   "Stop Input Evaluation").
   * @return The newly created input set.
   */
  std::unique_ptr<InputSet> parseInputSet(const nlohmann::json& inputSetJs)
  {
    // Creating new input set to add
    auto inputSet = std::make_unique<InputSet>();

    // Getting input set condition array
    const auto& conditions = jaffarCommon::json::getArray<nlohmann::json>(inputSetJs, "Conditions");

    // Getting input string array
    const auto& inputsJs = jaffarCommon::json::getArray<std::string>(inputSetJs, "Inputs");

    // Parsing input set conditions
    for (const auto& condition : conditions) inputSet->addCondition(_game->parseCondition(condition));

    // Parsing input set inputs
    for (const auto& input : inputsJs) inputSet->addInput(registerInput(input));

    // Keep track of maximum size (for better output)
    _largestInputSetSize = std::max(_largestInputSetSize, inputSet->getInputIndexes().size());

    // Getting stop evaluating flag
    inputSet->setStopInputEvaluationFlag(jaffarCommon::json::getBoolean(inputSetJs, "Stop Input Evaluation"));

    // Returning new input set
    return inputSet;
  }

  /**
   * @brief Registers an input string and returns its numeric index.
   *
   * @details Obtains the index from the emulator, records the hash->index and index->string mappings,
   * and updates @ref _maxInputIndex. Registering an already-registered input returns its prior index.
   * @param input The input string to register.
   * @return The index assigned to the input.
   */
  InputSet::inputIndex_t registerInput(const std::string& input)
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

  /**
   * @brief Collects the input indexes of every input set whose conditions currently evaluate to true.
   *
   * @details Iterates the given input sets in order; for each set that evaluates to true, its input
   * indexes are appended to the result. If a satisfied set has its stop-evaluation flag set,
   * iteration stops after that set.
   * @param inputSets The input sets to evaluate.
   * @return The concatenated input indexes of the satisfied input sets.
   */
  std::vector<InputSet::inputIndex_t> getInputsFromInputSets(const std::vector<std::unique_ptr<InputSet>>& inputSets) const
  {
    // Storage for the possible input set
    std::vector<InputSet::inputIndex_t> possibleInputs;

    // For all registered input sets, see which ones satisfy their conditions and add them
    for (const auto& inputSet : inputSets)
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

  /**
   * @brief Returns the inputs currently allowed for the game's state.
   *
   * @details Collects the inputs from the allowed input sets and appends any additional inputs
   * supplied by the game via @ref Game::getAdditionalAllowedInputs.
   * @return The allowed input indexes.
   */
  __INLINE__ const auto getAllowedInputs() const
  {
    // Getting possible inputs based on the configuration file
    auto allowedInputs = getInputsFromInputSets(_allowedInputSets);

    // Getting additional inputs based on the custom game function
    _game->getAdditionalAllowedInputs(allowedInputs);

    return allowedInputs;
  }

  /**
   * @brief Returns the inputs currently allowed by the candidate input sets.
   * @return The candidate input indexes whose input sets evaluate to true.
   */
  __INLINE__ const auto getCandidateInputs() const { return getInputsFromInputSets(_candidateInputSets); }

  /**
   * @brief Looks up the index registered for an input string.
   * @param input The input string to look up.
   * @return The index registered for the input.
   * @throws A logic error if the input was not previously registered.
   */
  __INLINE__ jaffarPlus::InputSet::inputIndex_t getInputIndex(const std::string& input) const
  {
    // Getting input hash
    const auto inputHash = jaffarCommon::hash::hashString(input);

    // Getting input index from the hash map
    auto it = _inputHashMap.find(inputHash);
    if (it == _inputHashMap.end()) JAFFAR_THROW_LOGIC("[ERROR] Input '%s' provided but has not been registered as allowed input first.\n", input.c_str());
    const auto inputIndex = it->second;

    return inputIndex;
  }

  /**
   * @brief Reports whether an input string has been registered.
   * @param inputString The input string to test.
   * @return true if the input is registered, false otherwise.
   */
  bool isInputRegistered(const std::string& inputString)
  {
    // Getting input hash
    const auto inputHash = jaffarCommon::hash::hashString(inputString);

    // Returning true if found; false, otherwise
    return _inputHashMap.contains(inputHash);
  }

  /**
   * @brief Advances the game by one input, then by the configured number of frameskip frames.
   *
   * @details Applies @p inputIdx to the game and records it in the input history. Then, for each of
   * the @ref _frameskipRate skipped frames, advances the game with either the custom frameskip input
   * (when "Use Custom Input" is set) or the same @p inputIdx, recording each in the input history.
   * @param inputIdx The index of the input to apply for this step.
   */
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
      InputSet::inputIndex_t frameskipInputIdx = inputIdx;
      if (_frameskipUseCustomInput) frameskipInputIdx = _frameskipCustomInputIdx;
      _game->advanceState(frameskipInputIdx);
      pushInput(frameskipInputIdx);
    }
  }

  /**
   * @brief Records an applied input into the input history and advances the input counter.
   *
   * @details When input history is enabled, stores @p inputIdx at the current step unless capacity
   * (@ref _inputHistoryMaxSize) has been reached, in which case the input is counted but not recorded
   * and a one-time warning is emitted. The input counter is always advanced.
   * @param inputIdx The index of the input that was applied.
   */
  __INLINE__ void pushInput(const InputSet::inputIndex_t inputIdx)
  {
    if (_inputHistoryEnabled == true)
    {
      // Store the input, unless we've reached the configured history capacity
      if (_currentInputCount < _inputHistoryMaxSize) { setInput(_currentInputCount, inputIdx); }

      // Past capacity, further inputs are counted but not recorded: any solution longer than
      // "Store Input History / Max Size" is silently truncated and will not reproduce in full.
      // Warn once (across all threads) so this is not mistaken for a complete solution.
      else if (_inputHistoryTruncationWarned.exchange(true) == false)
        jaffarCommon::logger::log("[J+] Warning: input history exceeded its maximum size (%u). Solutions longer than this are truncated; increase 'Store "
                                  "Input History / Max Size' to record them in full.\n",
                                  _inputHistoryMaxSize);
    }

    // Advancing step counter
    _currentInputCount++;
  }

  /**
   * @brief Writes an input index into the bit-packed input history at a given step.
   * @param stepId   The step (history slot) to write to.
   * @param inputIdx The input index to store.
   */
  __INLINE__ void setInput(const size_t stepId, const InputSet::inputIndex_t inputIdx)
  {
    // Using bit-encoded copy to store the new input
    jaffarCommon::bitwise::bitcopy(_inputHistory.data(), _inputHistory.size(), stepId, &inputIdx, sizeof(InputSet::inputIndex_t), 0, 1, _inputIndexSizeBits);
  }

  /**
   * @brief Reads the input index stored in the bit-packed input history at a given step.
   * @param stepId The step (history slot) to read from.
   * @return The input index stored at @p stepId.
   */
  __INLINE__ InputSet::inputIndex_t getInput(const size_t stepId) const
  {
    // Temporary storage for the new input
    InputSet::inputIndex_t inputIdx = 0;

    // Using bit-encoded copy to store the new input
    jaffarCommon::bitwise::bitcopy(&inputIdx, sizeof(InputSet::inputIndex_t), 0, _inputHistory.data(), _inputHistory.size(), stepId, 1, _inputIndexSizeBits);

    // Returning value
    return inputIdx;
  }

  /**
   * @brief Serializes the runner state: the game state, the input history, and the input counter.
   * @param serializer The serializer to write the state into.
   */
  __INLINE__ void serializeState(jaffarCommon::serializer::Base& serializer) const
  {
    // Performing serialization of the internal game instance
    _game->serializeState(serializer);

    // Serializing input history data
    if (_inputHistoryEnabled == true) serializer.push(_inputHistory.data(), _inputHistory.size());

    // Serializing current step
    serializer.pushContiguous(&_currentInputCount, sizeof(_currentInputCount));
  }

  /**
   * @brief Restores the runner state: the game state, the input history, and the input counter.
   * @param deserializer The deserializer to read the state from.
   */
  __INLINE__ void deserializeState(jaffarCommon::deserializer::Base& deserializer)
  {
    // Performing serialization of the internal game instance
    _game->deserializeState(deserializer);

    // Deserializing input history
    if (_inputHistoryEnabled == true) deserializer.pop(_inputHistory.data(), _inputHistory.size());

    // Deserializing current step
    deserializer.popContiguous(&_currentInputCount, sizeof(_currentInputCount));
  }

  /**
   * @brief Computes the size in bytes of the serialized runner state.
   * @return The contiguous serialized state size in bytes.
   */
  __INLINE__ size_t getStateSize() const
  {
    jaffarCommon::serializer::Contiguous s;
    this->serializeState(s);
    return s.getOutputSize();
  }

  /**
   * @brief Computes a hash of the current runner state.
   *
   * @details When "Bypass Hash Calculation" is enabled, returns the game's direct state hash.
   * Otherwise, hashes the current hash-step-tolerance stage together with the game's hash
   * contribution using MetroHash128.
   * @return The computed state hash.
   */
  __INLINE__ jaffarCommon::hash::hash_t computeHash() const
  {
    // If normal hash calculation is to be bypassed, get it straight from the game
    if (_bypassHashCalculation == true) return _game->getDirectStateHash();

    // Storage for hash calculation
    MetroHash128 hashEngine;

    // Calculating hash tolerance stage
    auto hashStepToleranceStage = getHashStepToleranceStage();

    // Hashing step tolerance stage
    hashEngine.Update(hashStepToleranceStage);

    // Processing hashing from the game proper
    _game->computeHash(hashEngine);

    jaffarCommon::hash::hash_t result;
    hashEngine.Finalize(reinterpret_cast<uint8_t*>(&result));
    return result;
  }

  /**
   * @brief Builds a newline-separated string of the recorded input history.
   *
   * @details Returns an empty string when input history is disabled. Otherwise concatenates the
   * input string for each recorded step up to the current input count or the history capacity.
   * @return The recorded inputs, one per line.
   * @throws A runtime error if a recorded input index has no registered string.
   */
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
      const std::string& inputString = _inputStringMap.at(inputIdx);

      // Adding it to the story
      inputHistoryString += inputString;
      inputHistoryString += "\n";
    }

    return inputHistoryString;
  }

  /**
   * @brief Returns the input string registered for an input index.
   * @param input The input index to resolve.
   * @return The registered input string for @p input.
   */
  std::string getInputStringFromIndex(const InputSet::inputIndex_t input) { return _inputStringMap[input]; }

  /**
   * @brief Logs runner state information.
   *
   * @details Logs input-history configuration, the current input count, the state hash, the
   * hash-step-tolerance stage and the frameskip rate. When "Show Allowed Inputs" is set, also logs
   * the currently allowed inputs (with empty-slot placeholders when "Show Empty Input Slots" is set).
   */
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
      jaffarCommon::logger::log("[J+]    + Input History Size: %u steps (%lu Bytes, %lu Bits)\n", _inputHistoryMaxSize, _inputHistory.size(),
                                _inputIndexSizeBits * _inputHistoryMaxSize);
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
      const auto& possibleInputs = getAllowedInputs();

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

  /** @brief Returns the current hash-step-tolerance stage (current input count modulo tolerance + 1). */
  __INLINE__ uint32_t getHashStepToleranceStage() const { return _currentInputCount % (_hashStepTolerance + 1); }
  /** @brief Returns a pointer to the owned game instance. */
  __INLINE__ Game* getGame() const { return _game.get(); }

  /** @brief Returns whether input-history recording is enabled. */
  __INLINE__ bool getInputHistoryEnabled() const { return _inputHistoryEnabled; }
  /** @brief Returns the maximum number of input-history steps that can be recorded. */
  __INLINE__ size_t getInputHistoryMaximumStep() const { return _inputHistoryMaxSize; }

  /**
   * @brief Creates a runner from the emulator, game and runner configurations.
   *
   * @details Builds the game via @ref Game::getGame and wraps it in a new runner.
   * @param emulatorConfig Configuration for the emulator.
   * @param gameConfig     Configuration for the game.
   * @param runnerConfig   Configuration for the runner.
   * @return The newly created runner.
   */
  static std::unique_ptr<Runner> getRunner(const nlohmann::json& emulatorConfig, const nlohmann::json& gameConfig, const nlohmann::json& runnerConfig)
  {
    // Getting Game
    auto game = Game::getGame(emulatorConfig, gameConfig);

    // Creating new runner
    auto r = std::make_unique<Runner>(game, runnerConfig);

    // Returning runner
    return r;
  }

  /** @brief Returns whether the runner has been initialized. */
  __INLINE__ bool isInitialized() const { return _isInitialized; }

private:
  bool _isInitialized = false; ///< Whether the runner has been initialized.

  std::unique_ptr<Game> _game; ///< The owned game instance driven by the runner.

  uint32_t _inputHistoryMaxSize; ///< Maximum number of steps that can be recorded in the input history.

  uint32_t _currentInputCount = 0; ///< Number of inputs applied so far (the current step counter).

  uint32_t _hashStepTolerance; ///< Hash step tolerance, used to derive the hash-step-tolerance stage.

  bool _inputHistoryEnabled; ///< Whether the input history is recorded.

  /// @brief Warn-once guard (shared across all runner instances/threads) for input-history truncation.
  static inline std::atomic<bool> _inputHistoryTruncationWarned = false;

  size_t _inputIndexSizeBits; ///< Number of bits used to encode each input index in the history.

  std::vector<uint8_t> _inputHistory; ///< Bit-packed storage for the recorded input history.

  bool _bypassHashCalculation; ///< Whether @ref computeHash returns the game's direct hash instead of hashing via MetroHash128.

  ///////////////////////////////
  // Input processing variables
  //////////////////////////////

  InputSet::inputIndex_t _maxInputIndex = 0; ///< One past the highest registered input index; determines the input-history encoding width.

  std::map<jaffarCommon::hash::hash_t, InputSet::inputIndex_t> _inputHashMap; ///< Maps an input string's hash to its index.

  std::map<InputSet::inputIndex_t, std::string> _inputStringMap; ///< Maps an input index back to its input string.

  ///////////////////////////////
  //  Frameskip. These are frames that are simply skipped over, but they inputs registered nevertheless
  //////////////////////////////

  size_t _frameskipRate; ///< Number of frames to skip after each applied input.

  bool _frameskipUseCustomInput; ///< Whether skipped frames use the custom input; false repeats the applied input.

  std::string _frameskipCustomInput; ///< The custom input string to apply on skipped frames.

  /// @brief Resolved index of the custom frameskip input, computed once at initialize() so advanceState()
  /// does not re-hash + re-look-up the input string on every skipped frame.
  InputSet::inputIndex_t _frameskipCustomInputIdx{};

  ///////////////////////////////////////////
  // Allowed and candidate input sets
  //////////////////////////////////////////

  std::vector<nlohmann::json> _allowedInputSetsJs; ///< JSON descriptions of the allowed input sets (parsed at initialization).

  std::vector<nlohmann::json> _candidateInputSetsJs; ///< JSON descriptions of the candidate input sets (parsed at initialization).

  std::vector<std::unique_ptr<InputSet>> _candidateInputSets; ///< Parsed candidate input sets.

  std::vector<std::unique_ptr<InputSet>> _allowedInputSets; ///< Parsed allowed input sets.

  size_t _largestInputSetSize = 0; ///< Largest input set size detected (used for output formatting).

  bool _testCandidateInputs; ///< Whether candidate input sets are parsed and tested.

  bool _showEmptyInputSlots; ///< Whether placeholders are printed for unused input slots.

  bool _showAllowedInputs; ///< Whether allowed inputs are printed in @ref printInfo.
};

} // namespace jaffarPlus