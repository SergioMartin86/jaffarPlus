#pragma once

#include <omp.h>
#include <cmath>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/hash.hpp>
#include <jaffarCommon/include/serializers/base.hpp>
#include <jaffarCommon/include/deserializers/base.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "game.hpp"
#include "runner.hpp"
#include "stateDb.hpp"
#include "hashDb.hpp"

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
    _emulatorName = JSON_GET_STRING(config, "Emulator");

    // Getting game name from the configuration
    _gameName = JSON_GET_STRING(config, "Game");

    // Getting number of threads
    _threadCount = omp_get_max_threads();

    // Sanity check
    if (_threadCount == 0) EXIT_WITH_ERROR("The number of worker threads must be at least one. Provided: %lu\n", _threadCount);

    // Creating storage for the runnners (one per thread)
    _runners.resize(_threadCount);

    // Initializing runners, one per thread
    #pragma omp parallel
    {
      // Getting my thread id
      int threadId = omp_get_thread_num();
    
      // Getting emulator from its name and configuring it
      auto e = jaffarPlus::Emulator::getEmulator(_emulatorName, JSON_GET_OBJECT(config, "Emulator Configuration"));

      // Initializing emulator
      e->initialize();

      // Disable rendering
      e->disableRendering();

      // If initial state file defined, load it
      if (initialStateFilePath.empty() == false) e->loadStateFile(initialStateFilePath);

      // Disabling ignored state properties
      e->disableStateProperties();

      // Getting game from its name and configuring it
      auto g = jaffarPlus::Game::getGame(_gameName, e, JSON_GET_OBJECT(config, "Game Configuration"));

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

    // Getting maximum bumber of steps
    _maxStepCount = r.getMaximumStep();

    // Creating State database
    _stateDb = std::make_unique<jaffarPlus::StateDb>(r, JSON_GET_OBJECT(config, "State Database"));

    // Getting memory for the reference state
    const auto stateSize = r.getStateSize();

    // Allocating memory
    uint8_t referenceData[stateSize];
    
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

    // Getting a free state data pointer to store the state into
    auto stateData = _stateDb->getFreeState();

    // Pushing initial state to the next state database
    _stateDb->pushState(reward, r, stateData);

    // Now swapping databases so that the next becomes the current one
    _stateDb->swapStateDatabases();

    // Creating hash database 
    _hashDb = std::make_unique<jaffarPlus::HashDb>(JSON_GET_OBJECT(config, "Hash Database"));
   
    // Getting hash from first state
    const auto hash = r.computeHash();

    // Adding it to the hash DB
    _hashDb->checkHashCollision(hash);
  };

  /**
   * Starts solving the problem provided by the script
  */
  void run()
  {
    // Current step being ran
    size_t currentStep = 0;

    // Flag to determine whether to continue running
    bool continueRunning = true;

    // Using first runner for state evaluation
    auto& r = *_runners.begin();

    // Computing total elapsed time
    const auto tStart = jaffarCommon::now();

    // Iterate until no longer need to run
    while (continueRunning == true)
    {
      // Clearing win states vector
      _winStatesFound.clear();

      // Clearing step counters
      _stepBaseStatesProcessed = 0;
      _stepNewStatesProcessed = 0;

      // Computing step time
      const auto tStep = jaffarCommon::now();

      // Performing one computation step in parallel
      #pragma omp parallel
        workerFunction();

      // Advancing hash database state
      _hashDb->advanceState();

      // Swapping next and current state databases
      _stateDb->swapStateDatabases();

      // Computing step time
      const auto currentStepTime = jaffarCommon::timeDeltaSeconds(jaffarCommon::now(), tStep);

      // Computing total running time
      const auto totalRunningTime = jaffarCommon::timeDeltaSeconds(jaffarCommon::now(), tStart);

      // Getting current number of states left in the database
      const size_t statesLeft = _stateDb->getCurrentStateDbSize();

      // Getting best state so far
      auto bestState = _stateDb->getBestState();

      // Loading best state into runner
      _stateDb->loadStateIntoRunner(*r, bestState);

      // Dumping best solution so far into a file
      r->dumpInputHistoryToFile("jaffar.best.sol");

      // Printing information
      LOG("[J+] Current Step #:                   %lu (Max: %lu)\n", currentStep, _maxStepCount);
      LOG("[J+] Emulator Name:                    '%s'\n", _emulatorName.c_str());
      LOG("[J+] Game Name:                        '%s'\n", _gameName.c_str());
      LOG("[J+] Thread Count:                     %lu\n", _threadCount);
      LOG("[J+] Elapsed Time (Step/Total):        %3.3fs / %3.3fs\n", currentStepTime, totalRunningTime);
      LOG("[J+] Base States Performance:          %.3f States/s\n", (double)_stepBaseStatesProcessed / currentStepTime);
      LOG("[J+] New States Performance:           %.3f States/s\n", (double)_stepNewStatesProcessed / currentStepTime);

      // Print state database information
      LOG("[J+] State Database Information:\n");
      _stateDb->printInfo();

      LOG("[J+] Hash Database Information:\n");
      _hashDb->printInfo();

      if (_winStatesFound.size() > 0) continueRunning = false;

      // If no states remain, then stop
      if (statesLeft == 0) continueRunning = false;

      // Increasing current step
      currentStep++;

      // Stop if reached maximum step count
      if (currentStep == _maxStepCount) continueRunning = false;
    }

    if (_winStatesFound.size() > 0) LOG("Win state reached");
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
    void* baseStateData;

    // While there are still states in the database, keep on grabbing them
    while ((baseStateData = _stateDb->popState()) != nullptr)
    {
      // Increasing base state counter
      _stepBaseStatesProcessed++;
      
      // Load state into runner via the state database
      _stateDb->loadStateIntoRunner(*r, baseStateData);

      // Getting possible inputs
      const auto& possibleInputs = r->getPossibleInputs();

      // Trying out each possible input
      for (auto inputItr = possibleInputs.begin(); inputItr != possibleInputs.end(); inputItr++)
      {
        // Increasing new state counter
        _stepNewStatesProcessed++;

        // We don't need to reload the base state if it is the first input
        if (inputItr != possibleInputs.begin()) _stateDb->loadStateIntoRunner(*r, baseStateData);

        // Now advancing state with the provided input
        r->advanceState(*inputItr);

        // Getting state hash to check whether it has been seen before
        const auto hash = r->computeHash();

        // Checking if hash exists (state is repeated)
        bool isCollision = _hashDb->checkHashCollision(hash);
        
        // If state is repeated then we are not interested in it, continue
        if (isCollision == true) continue;

        // Evaluating game rules based on the new state
        r->getGame()->evaluateRules();

        // Determining state type
        r->getGame()->updateGameStateType();

        // Getting state type
        const auto stateType = r->getGame()->getStateType();

        // Now we have determined the state is not repeated, check if it's not a failed state
        if (stateType == Game::stateType_t::fail) continue;
        
        // Now that the state is not failed nor repeated, this is effectively a new state to add
        void* newStateData = nullptr;

        // Grab a free state from the state db
        newStateData = _stateDb->getFreeState();

        // If couldn't get any memory, simply drop the state
        if (newStateData == nullptr) continue;

        // Updating state reward
        r->getGame()->updateReward();

        // Getting state reward
        const auto reward = r->getGame()->getReward();  
          
        // If we are here, this is a new state to push into the next step's database
        _stateDb->pushState(reward, *r, newStateData);

        // If this is a win state, register it
        if (stateType == Game::stateType_t::win) _winStatesFound.push_back(newStateData);
      }

      // Return base state to the free state queue
      _stateDb->returnFreeState(baseStateData);
    }
  }

  // Collection of runners for the workers to use
  std::vector<std::unique_ptr<Runner>> _runners;

  // The thread-safe state database that contains current and next step's states. 
  std::unique_ptr<jaffarPlus::StateDb> _stateDb;

  // The thread-safe hash database to check for repeated states
  std::unique_ptr<jaffarPlus::HashDb> _hashDb;

  // Set of win states found
  std::vector<void*> _winStatesFound;

  ///////////////// Configuration

  // Emulator Name
  std::string _emulatorName;

  // Game Name
  std::string _gameName;

  // Maximum number of steps to run
  size_t _maxStepCount;

  // Thread count (set by openMP)
  size_t _threadCount;

  //////////////// Statistics
   
  // Counter for the number of base states processed
  std::atomic<size_t> _stepBaseStatesProcessed;

  // Counter for the number of new states processed
  std::atomic<size_t> _stepNewStatesProcessed;

  
};

} // namespace jaffarPlus