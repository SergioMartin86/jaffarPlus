#pragma once

#include "jaffarCommon/deserializers/contiguous.hpp"
#include "jaffarCommon/hash.hpp"
#include "jaffarCommon/serializers/contiguous.hpp"
#include "runner.hpp"
#include <string>
#include <vector>
#include <algorithm>

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
        auto allowedInputs = _runner->getAllowedInputs();
        step.isInputAllowed = std::find(allowedInputs.begin(), allowedInputs.end(), step.inputIndex) != allowedInputs.end();
      }
      
      // Getting state hash
      step.stateHash = _runner->computeHash();

      // Checking for repeats
      for (size_t stepIdx = 0; stepIdx < i; stepIdx++)
        if (_hashMap[stepIdx] == step.stateHash) step._repeatedHashSteps.push_back(stepIdx);

      // Entering state hash into the map to check for repeated inputs
      _hashMap[i] = step.stateHash;

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
  __INLINE__ bool isInputAllowed(const size_t currentStep) const { return getStep(currentStep).isInputAllowed; }

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

  // Pointer to runner
  Runner* _runner;

  // Storage for game state size
  size_t _gameStateSize;

  // Storage for renderer state size
  size_t _rendererStateSize;

  // Storage for the sequence data
  std::vector<step_t> _sequence;

  // Hash database to check whether there are repeated hashes in the solution
  std::map<size_t, jaffarCommon::hash::hash_t> _hashMap;
};

} // namespace jaffarPlus
