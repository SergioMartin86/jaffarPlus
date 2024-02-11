#pragma once

#include <vector>
#include <string>
#include "jaffarCommon/include/serializers/contiguous.hpp"
#include "jaffarCommon/include/deserializers/contiguous.hpp"
#include "runner.hpp"

namespace jaffarPlus
{

class Playback
{
  public:

  struct step_t 
  {
    // Storage for the step's input string
    std::string inputString;

    // Storage for the step's input index
    jaffarPlus::InputSet::inputIndex_t inputIndex;

    // Storage for the step's state data
    void* stateData;
  };

  Playback(Runner& runner, const std::vector<std::string>& inputSequence) : _runner(&runner)
  {
    // Getting state size
    _stateSize = _runner->getStateSize();

    // For each input in the sequence, store the game's state
    for (const auto& input : inputSequence)
    {
      // Creating new step
      step_t step;

      // Setting step input string
      step.inputString = input;

      // Getting input index
      step.inputIndex = _runner->getInputIndex(input);

      // Allocating space for the step data
      step.stateData = malloc(_stateSize);

      // Serializing state
      jaffarCommon::serializer::Contiguous s(step.stateData, _stateSize);
      _runner->serializeState(s);

      // Advancing state
      _runner->advanceState(step.inputIndex);
    }
  };

  private:

  // Pointer to runner
  Runner* _runner;
 
  // Storage for state size
  size_t _stateSize;

  // Storage for the sequence data
  std::vector<step_t> _sequence;
};

} // namespace jaffarPlus
