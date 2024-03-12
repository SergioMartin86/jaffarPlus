#pragma once

#include <string>
#include <vector>
#include "jaffarCommon/deserializers/contiguous.hpp"
#include "jaffarCommon/hash.hpp"
#include "jaffarCommon/serializers/contiguous.hpp"
#include "runner.hpp"

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

    // Storage for the step's game state data
    void *gameStateData;

    // Storage for the step's renderer state data
    void *rendererStateData;

    // Storage for the step's hash
    jaffarCommon::hash::hash_t stateHash;
  };

  Playback(Runner &runner, const std::vector<std::string> &inputSequence)
    : _runner(&runner)
  {
    // Getting game state size
    _gameStateSize = _runner->getStateSize();

    // Getting renderer state size
    _rendererStateSize = _runner->getGame()->getEmulator()->getRendererStateSize();

    // For each input in the sequence, store the game's state
    for (size_t i = 0; i <= inputSequence.size(); i++)
    {
      // Creating new step
      step_t step;

      // Setting step input string
      step.inputString = i < inputSequence.size() ? inputSequence[i] : "<End Of Sequence>";

      // Getting input index
      step.inputIndex = i < inputSequence.size() ? _runner->getInputIndex(step.inputString) : 0;

      // Getting state hash
      step.stateHash = _runner->computeHash();

      // Allocating space for the game state data
      step.gameStateData = malloc(_gameStateSize);

      // Serializing game state
      jaffarCommon::serializer::Contiguous sg(step.gameStateData, _gameStateSize);
      _runner->serializeState(sg);

      // Allocating space for the renderer state data
      step.rendererStateData = malloc(_rendererStateSize);

      // Updating renderer state
      _runner->getGame()->getEmulator()->updateRendererState();

      // Serializing renderer state
      jaffarCommon::serializer::Contiguous sr(step.rendererStateData, _rendererStateSize);
      _runner->getGame()->getEmulator()->serializeRendererState(sr);

      // Advancing state
      _runner->advanceState(step.inputIndex);

      // Evaluate game rules
      _runner->getGame()->evaluateRules();

      // Determining new game state type
      _runner->getGame()->updateGameStateType();

      // Updating game reward
      _runner->getGame()->updateReward();

      // Adding step to the internal storage
      _sequence.push_back(step);
    }
  };

  ~Playback() {}

  __INLINE__ std::string getStateInputString(const size_t currentStep) const { return getStep(currentStep).inputString; }
  __INLINE__ jaffarPlus::InputSet::inputIndex_t getStateInputIndex(const size_t currentStep) const { return getStep(currentStep).inputIndex; }
  __INLINE__ void                              *getStateData(const size_t currentStep) const { return getStep(currentStep).gameStateData; }
  __INLINE__ jaffarCommon::hash::hash_t getStateHash(const size_t currentStep) const { return getStep(currentStep).stateHash; }

  __INLINE__ void renderFrame(const size_t currentStep)
  {
    const auto                            &step = getStep(currentStep);
    jaffarCommon::deserializer::Contiguous d(step.rendererStateData, _rendererStateSize);
    _runner->getGame()->getEmulator()->deserializeRendererState(d);
    _runner->getGame()->getEmulator()->updateVideoOutput();
  }

  void printInfo(const size_t stepId) const
  {
    // Deserializing appropriate state
    jaffarCommon::deserializer::Contiguous d(getStateData(stepId), _gameStateSize);
    _runner->deserializeState(d);

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
  Runner *_runner;

  // Storage for game state size
  size_t _gameStateSize;

  // Storage for renderer state size
  size_t _rendererStateSize;

  // Storage for the sequence data
  std::vector<step_t> _sequence;
};

} // namespace jaffarPlus
