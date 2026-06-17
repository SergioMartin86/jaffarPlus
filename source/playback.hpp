#pragma once

#include "jaffarCommon/deserializers/contiguous.hpp"
#include "jaffarCommon/hash.hpp"
#include "jaffarCommon/serializers/contiguous.hpp"
#include "runner.hpp"
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace jaffarPlus
{

class Playback final
{
public:
  struct step_t
  {
    // Storage for the step's input string
    std::string inputString;

    // Storage for the step's input index
    jaffarPlus::InputSet::inputIndex_t inputIndex;

    // Stores whether the move is allowed by the current move set
    bool isInputAllowed;

    // Storage for the step's game state data
    void* gameStateData;

    // Storage for the step's renderer state data
    void* rendererStateData;

    // Storage for the step's hash
    jaffarCommon::hash::hash_t stateHash;

    // Stores which other steps had the same repeated hashes
    std::vector<size_t> _repeatedHashSteps;
  };

  Playback(Runner& runner) : _runner(&runner)
  {
    // Getting game state size
    _gameStateSize = _runner->getStateSize();

    // Getting renderer state size
    _rendererStateSize = _runner->getGame()->getEmulator()->getRendererStateSize();
  };

  void initialize(const std::vector<std::string>& inputSequence)
  {
    // For each input in the sequence, store the game's state
    for (size_t i = 0; i <= inputSequence.size(); i++)
    {
      // Creating new step
      step_t step;

      // Checking if this is the end of the sequence
      bool isEndOfSequence = i == inputSequence.size();

      // Setting step input string
      step.inputString = isEndOfSequence == false ? inputSequence[i] : "<End Of Sequence>";

      // Checking if the input is allowed
      bool isRegisteredInput = _runner->isInputRegistered(step.inputString);

      // Getting input index
      if (isEndOfSequence == true) step.inputIndex = 0;
      if (isEndOfSequence == false && isRegisteredInput == true) step.inputIndex = _runner->getInputIndex(step.inputString);
      if (isEndOfSequence == false && isRegisteredInput == false) step.inputIndex = _runner->registerInput(step.inputString);

      // Checking if the input is allowed
      step.isInputAllowed = false;
      if (isRegisteredInput == true)
      {
        auto allowedInputs  = _runner->getAllowedInputs();
        step.isInputAllowed = std::find(allowedInputs.begin(), allowedInputs.end(), step.inputIndex) != allowedInputs.end();
      }

      // Getting state hash
      step.stateHash = _runner->computeHash();

      // Recording duplicate states: any earlier steps already filed under this step's hash are
      // exactly the repeated states the engine would have pruned on encountering them. Looking them
      // up in a hash map is O(1) amortized, versus the previous O(n^2) scan over every prior step,
      // so this scales to long movies. The earlier steps are stored in ascending order, matching the
      // previous behaviour.
      auto& sameHashSteps     = _hashOccurrences[step.stateHash];
      step._repeatedHashSteps = sameHashSteps;
      sameHashSteps.push_back(i);

      // Allocating space for the game state data
      step.gameStateData = malloc(_gameStateSize);

      // Serializing game state
      jaffarCommon::serializer::Contiguous sg(step.gameStateData, _gameStateSize);
      _runner->serializeState(sg);

      // Allocating space for the renderer state data
      step.rendererStateData = malloc(_rendererStateSize);

      // Updating renderer state
      _runner->getGame()->getEmulator()->updateRendererState(i, step.inputString);

      // Serializing renderer state
      jaffarCommon::serializer::Contiguous sr(step.rendererStateData, _rendererStateSize);
      _runner->getGame()->getEmulator()->serializeRendererState(sr);

      // Advancing state
      if (i < inputSequence.size()) _runner->advanceState(step.inputIndex);
      if (i == inputSequence.size()) _runner->advanceState(_sequence.rbegin()->inputIndex);

      // Evaluate game rules
      _runner->getGame()->evaluateRules();

      // Determining new game state type
      _runner->getGame()->updateGameStateType();

      // Updating game reward
      _runner->getGame()->updateReward();

      // Adding step to the internal storage
      _sequence.push_back(step);
    }
  }

  ~Playback()
  {
    // Freeing up memory reserved during initialization
    for (const auto& step : _sequence)
    {
      free(step.gameStateData);
      free(step.rendererStateData);
    }
  }

  __INLINE__ std::string getStateInputString(const size_t currentStep) const { return getStep(currentStep).inputString; }
  __INLINE__ jaffarPlus::InputSet::inputIndex_t getStateInputIndex(const size_t currentStep) const { return getStep(currentStep).inputIndex; }
  __INLINE__ void*                              getStateData(const size_t currentStep) const { return getStep(currentStep).gameStateData; }
  __INLINE__ const std::vector<size_t> getStateRepeatedHashSteps(const size_t currentStep) const { return getStep(currentStep)._repeatedHashSteps; }
  __INLINE__ jaffarCommon::hash::hash_t getStateHash(const size_t currentStep) const { return getStep(currentStep).stateHash; }
  __INLINE__ bool                       isInputAllowed(const size_t currentStep) const { return getStep(currentStep).isInputAllowed; }

  __INLINE__ void renderFrame(const size_t currentStep)
  {
    const auto&                            step = getStep(currentStep);
    jaffarCommon::deserializer::Contiguous d(step.rendererStateData, _rendererStateSize);
    _runner->getGame()->getEmulator()->deserializeRendererState(d);
    _runner->getGame()->getEmulator()->showRender();
  }

  void loadStepData(const size_t stepId)
  {
    // Deserializing appropriate state
    jaffarCommon::deserializer::Contiguous d(getStateData(stepId), _gameStateSize);
    _runner->deserializeState(d);
  }

  void printInfo() const
  {
    // Now printing information
    jaffarCommon::logger::log("[J+] Runner Information: \n");
    _runner->printInfo();
    jaffarCommon::logger::log("[J+] Game Information: \n");
    _runner->getGame()->printInfo();
    jaffarCommon::logger::log("[J+] Emulator Information: \n");
    _runner->getGame()->getEmulator()->printInfo();
  }

private:
  // Step getter
  step_t getStep(const size_t stepId) const
  {
    if (stepId >= _sequence.size()) JAFFAR_THROW_RUNTIME("Requested step %lu which exceeds sequence size %lu", stepId, _sequence.size());
    return _sequence.at(stepId);
  }

  // Hash functor for 128-bit state hashes (std::pair<uint64_t, uint64_t>), so they can key an
  // unordered_map for O(1) duplicate-state lookup.
  struct hashHasher_t
  {
    size_t operator()(const jaffarCommon::hash::hash_t& h) const noexcept { return h.first ^ (h.second + 0x9E3779B97F4A7C15ULL + (h.first << 6) + (h.first >> 2)); }
  };

  // Pointer to runner
  Runner* _runner;

  // Storage for game state size
  size_t _gameStateSize;

  // Storage for renderer state size
  size_t _rendererStateSize;

  // Storage for the sequence data
  std::vector<step_t> _sequence;

  // Maps each state hash to the steps (ascending) at which it occurred, used to detect the repeated
  // states the engine would have deduplicated.
  std::unordered_map<jaffarCommon::hash::hash_t, std::vector<size_t>, hashHasher_t> _hashOccurrences;
};

} // namespace jaffarPlus
