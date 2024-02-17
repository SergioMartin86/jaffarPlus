#pragma once

#include <omp.h>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/hash.hpp>
#include <jaffarCommon/include/serializers/base.hpp>
#include <jaffarCommon/include/deserializers/base.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "runner.hpp"
#include "stateDb.hpp"

namespace jaffarPlus
{

class Engine final
{
 public:

  // Base constructor
  Engine(const nlohmann::json& config) 
  {
    // Getting initial state file from the configuration
    const auto initialStateFilePath = JSON_GET_STRING(config, "Initial State File Path");

    // Getting emulator name from the configuration
    const auto emulatorName = JSON_GET_STRING(config, "Emulator");

    // Getting game name from the configuration
    const auto gameName = JSON_GET_STRING(config, "Game");

    // Getting number of threads
    size_t threadCount = omp_get_max_threads();

    // Sanity check
    if (threadCount == 0) EXIT_WITH_ERROR("The number of worker threads must be at least one. Provided: %lu\n", threadCount);

    // Creating storage for the runnners (one per thread)
    _runners.resize(threadCount);

    // Initializing runners, one per thread
    #pragma omp parallel
    {
      // Getting my thread id
      int threadId = omp_get_thread_num();
    
      // Getting emulator from its name and configuring it
      auto e = jaffarPlus::Emulator::getEmulator(emulatorName, JSON_GET_OBJECT(config, "Emulator Configuration"));

      // Initializing emulator
      e->initialize();

      // Disable rendering
      e->disableRendering();

      // If initial state file defined, load it
      if (initialStateFilePath.empty() == false) e->loadStateFile(initialStateFilePath);

      // Getting game from its name and configuring it
      auto g = jaffarPlus::Game::getGame(gameName, e, JSON_GET_OBJECT(config, "Game Configuration"));

      // Initializing game
      g->initialize();

      // Parsing script rules
      g->parseRules(JSON_GET_ARRAY(config, "Rules"));

      // Creating runner from game instance
      auto r = std::make_unique<jaffarPlus::Runner>(g, JSON_GET_OBJECT(config, "Runner Configuration"));

      // Parsing Possible game inputs
      r->parseGameInputs(JSON_GET_ARRAY(config, "Game Inputs"));

      // Storing runner
      _runners[threadId] = std::move(r);
    }

    // Grabbing a runner to do continue initialization
    auto& r = *_runners[0];

    // Creating State database
    _stateDb = std::make_unique<jaffarPlus::StateDb>(r, JSON_GET_OBJECT(config, "State Database"));

    // Getting memory for the reference state
    const auto stateSize = r.getStateSize();

    // Allocating memory
    auto referenceData = malloc(stateSize);
    
    // Serializing the initial state without compression to use as reference
    jaffarCommon::serializer::Contiguous s(referenceData, stateSize);
    r.serializeState(s);

    // Setting reference data
    _stateDb->setReferenceData(referenceData);

    // Evaluate game rules on the initial state
    r.getGame()->evaluateRules();

    // Determining new game state type
    r.getGame()->updateGameStateType();

    // Running game-specific rule actions
    r.getGame()->runGameSpecificRuleActions();

    // Updating game reward
    r.getGame()->updateReward();

    // Getting reward for the initial state
    const auto reward = r.getGame()->getReward();

    // Pushing initial state to the next state database
    _stateDb->pushState(reward, r);

    // Now swapping databases so that the next becomes the current one
    _stateDb->swapStateDatabases();

    // Printing information
    LOG("[J+] Emulator Name:                    '%s'\n", emulatorName.c_str());
    LOG("[J+] Thread Count:                     %lu\n", threadCount);

    // Printing State Db information
    LOG("[J+] State Database Info:\n");
    _stateDb->printInfo(); 

    LOG("States: %lu\n", _stateDb->getCurrentStateDbSize());
  };

  /**
   * Starts solving the problem provided by the script
  */
  void run()
  {
    // Starting worker threads
    #pragma omp parallel
    workerFunction();
  }

  ~Engine() = default;

  private:

  /**
   * The main worker function -- executes entirely in parallel
  */
  void workerFunction()
  {
    // Getting my thread id
    const auto threadId = omp_get_thread_num();

    // Getting my runner
    auto& r = _runners[threadId];

    // Current base state to process
    void* baseState = _stateDb->popState();

    // While there are still states in the database, keep on grabbing them
    while (baseState != nullptr)
    {
      // Load state into runner via the state database
      _stateDb->loadStateIntoRunner(*r, baseState);
   
      // Getting possible inputs
      const auto& possibleInputs = r->getPossibleInputs();

      // Store whether this is the first input
      bool isFirstInput = true;

      // Trying out each possible input
      for (const auto input : possibleInputs)
      {
        // We don't need to reload the base state if it is the first input
        if (isFirstInput == true) _stateDb->loadStateIntoRunner(*r, baseState);

        // The next input will not longer be the first one
        isFirstInput = false;
        
        // Now advancing state with the provided input
        r->advanceState(input);

        // Getting state hash to check whether it has been seen before
        const auto hash = r->computeHash();
      }

      // Return base state to the free state queue
      _stateDb->returnFreeState(baseState);

      // Load the next base state
      baseState = _stateDb->popState();
    }
  }

  // Collection of runners for the workers to use
  std::vector<std::unique_ptr<Runner>> _runners;

  // The thread-safe state database that contains current and next step's states. 
  std::unique_ptr<jaffarPlus::StateDb> _stateDb;


};

} // namespace jaffarPlus