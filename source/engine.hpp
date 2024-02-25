#pragma once

#include <omp.h>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/hash.hpp>
#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/timing.hpp>
#include <jaffarCommon/include/serializers/base.hpp>
#include <jaffarCommon/include/deserializers/base.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "game.hpp"
#include "runner.hpp"
#include "stateDb/plain.hpp"
#include "stateDb/numa.hpp"
#include "hashDb.hpp"

namespace jaffarPlus
{

class Engine final
{
 public:

  // Base constructor
  Engine(const nlohmann::json& emulatorConfig, const nlohmann::json& gameConfig, const nlohmann::json& runnerConfig, const nlohmann::json& engineConfig) 
  {
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
    
      // Creating runner from the configuration
      auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);
     
      // Disable emulator rendering
      r->getGame()->getEmulator()->disableRendering();

      // Storing runner
      _runners[threadId] = std::move(r);
    }

    // Grabbing a runner to do continue initialization
    auto& r = *_runners[0];

    // Getting maximum bumber of steps
    _maxStepCount = r.getMaximumStep();

    // Creating State database
    const auto& stateDatabaseJs = JSON_GET_OBJECT(engineConfig, "State Database");
    const auto& stateDatabaseType = JSON_GET_STRING(stateDatabaseJs, "Type");
    bool stateDatabaseTypeRecognized = false;
    if (stateDatabaseType == "Plain") { _stateDb = std::make_unique<jaffarPlus::stateDb::Plain>(r, JSON_GET_OBJECT(engineConfig, "State Database")); stateDatabaseTypeRecognized = true; }
    if (stateDatabaseType == "Numa Aware") { _stateDb = std::make_unique<jaffarPlus::stateDb::Numa>(r, JSON_GET_OBJECT(engineConfig, "State Database")); stateDatabaseTypeRecognized = true; }
    if (stateDatabaseTypeRecognized == false) EXIT_WITH_ERROR("State database type '%s' not recognized", stateDatabaseType.c_str());

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

    // Advancing the step in the state database
    _stateDb->advanceStep();

    // Creating hash database 
    _hashDb = std::make_unique<jaffarPlus::HashDb>(JSON_GET_OBJECT(engineConfig, "Hash Database"));
   
    // Getting hash from first state
    const auto hash = r.computeHash();

    // Adding it to the hash DB
    _hashDb->checkHashExists(hash);
  };

  /**
   * Resets execution back to step zero and clears all databases and counters
   */
  void initialize()
  {
    // Initializing cumulative timing
    _runnerStateAdvanceAverageCumulativeTime = 0;
    _runnerStateLoadAverageCumulativeTime = 0;
    _runnerStateSaveAverageCumulativeTime = 0;
    _checkHashAverageCumulativeTime = 0;
    _ruleCheckingAverageCumulativeTime = 0;
    _getFreeStateAverageCumulativeTime = 0;
    _returnFreeStateAverageCumulativeTime = 0;
    _calculateRewardAverageCumulativeTime = 0;
    _advanceHashDbAverageCumulativeTime = 0;
    _advanceStateDbAverageCumulativeTime = 0;

    // Updating total running time
    _totalRunningTime = 0;

    // Resetting databases
  } 

  /**
   * Runs a single step of the Jaffar Engine
  */
  void runStep()
  {
     // Computing step time
     const auto tStep = jaffarCommon::now();

     // Clearing step timing
     _runnerStateAdvanceThreadRawTime = 0;
     _runnerStateLoadThreadRawTime = 0;
     _runnerStateSaveThreadRawTime = 0;
     _checkHashThreadRawTime = 0;
     _ruleCheckingThreadRawTime = 0;
     _getFreeStateThreadRawTime = 0;
     _returnFreeStateThreadRawTime = 0;
     _calculateRewardThreadRawTime = 0;
     _advanceHashDbThreadRawTime = 0;
     _advanceStateDbThreadRawTime = 0;

     // Clearing win states vector
     _winStatesFound.clear();

     // Clearing step counters
     _stepBaseStatesProcessed = 0;
     _stepNewStatesProcessed = 0;

     // Performing one computation step in parallel
     #pragma omp parallel
     workerFunction();

     // Advancing hash database state
     const auto t0 = jaffarCommon::now();
     _hashDb->advanceStep();
     _advanceHashDbThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t0); 

     // Swapping next and current state databases
     const auto t1 = jaffarCommon::now();
     _stateDb->advanceStep();
     _advanceStateDbThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t1); 

     // Update reference data (should be part of the differential state db, automatically with the best )

     // Processing step and cumulative timing
     _runnerStateAdvanceAverageTime            = _runnerStateAdvanceThreadRawTime / _threadCount;
     _runnerStateAdvanceAverageCumulativeTime += _runnerStateAdvanceAverageTime;
     _runnerStateLoadAverageTime               = _runnerStateLoadThreadRawTime / _threadCount;
     _runnerStateLoadAverageCumulativeTime    += _runnerStateLoadAverageTime;
     _runnerStateSaveAverageTime               = _runnerStateSaveThreadRawTime / _threadCount;
     _runnerStateSaveAverageCumulativeTime    += _runnerStateSaveAverageTime;
     _checkHashAverageTime                     = _checkHashThreadRawTime / _threadCount;
     _checkHashAverageCumulativeTime          += _checkHashAverageTime;
     _ruleCheckingAverageTime                  = _ruleCheckingThreadRawTime / _threadCount;
     _ruleCheckingAverageCumulativeTime       += _ruleCheckingAverageTime;
     _getFreeStateAverageTime                  = _getFreeStateThreadRawTime / _threadCount;
     _getFreeStateAverageCumulativeTime       += _getFreeStateAverageTime;
     _returnFreeStateAverageTime               = _returnFreeStateThreadRawTime / _threadCount;
     _returnFreeStateAverageCumulativeTime    += _returnFreeStateAverageTime;
     _calculateRewardAverageTime               = _calculateRewardThreadRawTime / _threadCount;
     _calculateRewardAverageCumulativeTime    += _calculateRewardAverageTime;
     _advanceHashDbAverageTime                 = _advanceHashDbThreadRawTime.load();
     _advanceHashDbAverageCumulativeTime      += _advanceHashDbAverageTime;
     _advanceStateDbAverageTime                = _advanceStateDbThreadRawTime.load();
     _advanceStateDbAverageCumulativeTime     += _advanceStateDbAverageTime;

     // Computing step time
     _currentStepTime = jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), tStep);

     // Computing total running time
     _totalRunningTime += _currentStepTime;

         auto& r = *_runners[0];
         r.getGame()->printInfo();
  }

  ~Engine() = default;

  // Relevant data for the driver

  auto getWorstState() const { return _stateDb->getWorstState(); }
  auto getBestState() const { return _stateDb->getBestState(); }
  auto getWinStates() const { return _winStatesFound; }
  auto getStateCount () const { return _stateDb->getStateCount(); }
  auto getMaximumStep () const { return _maxStepCount; }

  /**
   * Information printing function
  */
  void printInfo()
  {
    // Printing information
    LOG("[J++] Thread Count:                                %lu\n", _threadCount);

    LOG("[J++] Elapsed Time (Step/Total):                  %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double)(_currentStepTime),
         100.0,
         1.0e-9 * (double)(_totalRunningTime),
         100.0);

    LOG("[J++]  + Runner State Avance (Step/Total):        %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_runnerStateAdvanceAverageTime),
         100.0  * ((double)(_runnerStateAdvanceAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_runnerStateAdvanceAverageCumulativeTime),
         100.0  * ((double) _runnerStateAdvanceAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Runner State Load (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_runnerStateLoadAverageTime),
         100.0  * ((double)(_runnerStateLoadAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_runnerStateLoadAverageCumulativeTime),
         100.0  * ((double) _runnerStateLoadAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Runner State Save (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_runnerStateSaveAverageTime),
         100.0  * ((double)(_runnerStateSaveAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_runnerStateSaveAverageCumulativeTime),
         100.0  * ((double) _runnerStateSaveAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Hash Checking (Step/Total):              %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_checkHashAverageTime),
         100.0  * ((double)(_checkHashAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_checkHashAverageCumulativeTime),
         100.0  * ((double) _checkHashAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Rule Checking (Step/Total):              %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_ruleCheckingAverageTime),
         100.0  * ((double)(_ruleCheckingAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_ruleCheckingAverageCumulativeTime),
         100.0  * ((double) _ruleCheckingAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Get Free State (Step/Total):             %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_getFreeStateAverageTime),
         100.0  * ((double)(_getFreeStateAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_getFreeStateAverageCumulativeTime),
         100.0  * ((double) _getFreeStateAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Return Free State (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_returnFreeStateAverageTime),
         100.0  * ((double)(_returnFreeStateAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_returnFreeStateAverageCumulativeTime),
         100.0  * ((double) _returnFreeStateAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Calculate Reward (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_calculateRewardAverageTime),
         100.0  * ((double)(_calculateRewardAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_calculateRewardAverageCumulativeTime),
         100.0  * ((double) _calculateRewardAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Advance Hash Db (Step/Total):            %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_advanceHashDbAverageTime),
         100.0  * ((double)(_advanceHashDbAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_advanceHashDbAverageCumulativeTime),
         100.0  * ((double) _advanceHashDbAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++]  + Advance State Db (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
         1.0e-9 * (double) (_advanceStateDbAverageTime),
         100.0  * ((double)(_advanceStateDbAverageTime) / (double)(_currentStepTime)),
         1.0e-9 * (double) (_advanceStateDbAverageCumulativeTime),
         100.0  * ((double) _advanceStateDbAverageCumulativeTime) / (double)(_totalRunningTime));

    LOG("[J++] Base States Performance:                     %.3f States/s\n", (double)_stepBaseStatesProcessed / (1.0e-9 * (double) _currentStepTime));
    LOG("[J++] New States Performance:                      %.3f States/s\n", (double)_stepNewStatesProcessed / (1.0e-9 * (double) _currentStepTime));

    // Print state database information
    LOG("[J++] State Database Information:\n");
    _stateDb->printInfo();

    LOG("[J++] Hash Database Information:\n");
    _hashDb->printInfo();
  }

  // Function to obtain engine based on game, emulator, and runner config
  static std::unique_ptr<Engine> getEngine(const nlohmann::json& emulatorConfig, const nlohmann::json& gameConfig, const nlohmann::json& runnerConfig, const nlohmann::json& engineConfig)
  {
      // Creating new engine
      auto e = std::make_unique<Engine>(emulatorConfig, gameConfig, runnerConfig, engineConfig);

      // Initializing engine
      e->initialize();

      // Returning engine
      return e;
  }
  
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
      const auto t0 = jaffarCommon::now();
      _stateDb->loadStateIntoRunner(*r, baseStateData);
     _runnerStateLoadThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t0);

      // Getting possible inputs
      const auto& possibleInputs = r->getPossibleInputs();

      // Trying out each possible input
      for (auto inputItr = possibleInputs.begin(); inputItr != possibleInputs.end(); inputItr++)
      {
        // Increasing new state counter
        _stepNewStatesProcessed++;

        // We don't need to reload the base state if it is the first input
        const auto t0 = jaffarCommon::now();
        if (inputItr != possibleInputs.begin()) _stateDb->loadStateIntoRunner(*r, baseStateData);
        _runnerStateLoadThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t0);

        // Now advancing state with the provided input
        const auto t1 = jaffarCommon::now();
        r->advanceState(*inputItr);
        _runnerStateAdvanceThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t1);

        // Getting state hash to check whether it has been seen before
        const auto t2 = jaffarCommon::now();
        const auto hash = r->computeHash();
        bool hashExists = _hashDb->checkHashExists(hash);
        _checkHashThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t2);

        // If state is repeated then we are not interested in it, continue
        if (hashExists == true) continue;

        // Evaluating game rules based on the new state
        const auto t3 = jaffarCommon::now();
        r->getGame()->evaluateRules();

        // Determining state type
        r->getGame()->updateGameStateType();

        // Getting state type
        const auto stateType = r->getGame()->getStateType();
        _ruleCheckingThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t3);

        // Now we have determined the state is not repeated, check if it's not a failed state
        if (stateType == Game::stateType_t::fail) continue;
        
        // Now that the state is not failed nor repeated, this is effectively a new state to add
        const auto t4 = jaffarCommon::now();
        void* newStateData = nullptr;

        // Grab a free state from the state db
        newStateData = _stateDb->getFreeState();
        _getFreeStateThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t4);

        // If couldn't get any memory, simply drop the state
        if (newStateData == nullptr) continue;

        // Updating state reward
        const auto t5 = jaffarCommon::now();
        r->getGame()->updateReward();

        // Getting state reward
        const auto reward = r->getGame()->getReward();  
        _calculateRewardThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t5);  

        // If we are here, this is a new state to push into the next step's database
        const auto t6 = jaffarCommon::now();
        _stateDb->pushState(reward, *r, newStateData);
        _runnerStateSaveThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t6);

        // If this is a win state, register it
        if (stateType == Game::stateType_t::win) _winStatesFound.push_back(newStateData);
      }

      // Return base state to the free state queue
      const auto t7 = jaffarCommon::now();
      _stateDb->returnFreeState(baseStateData);
      _returnFreeStateThreadRawTime += jaffarCommon::timeDeltaNanoseconds(jaffarCommon::now(), t7);
    }
  }

  // Collection of runners for the workers to use
  std::vector<std::unique_ptr<Runner>> _runners;

  // The thread-safe state database that contains current and next step's states. 
  std::unique_ptr<jaffarPlus::stateDb::Base> _stateDb;

  // The thread-safe hash database to check for repeated states
  std::unique_ptr<jaffarPlus::HashDb> _hashDb;

  // Set of win states found
  std::vector<void*> _winStatesFound;

  ///////////////// Configuration

  // Maximum number of steps to run
  size_t _maxStepCount;

  // Thread count (set by openMP)
  size_t _threadCount;

  //////////////// Statistics

  // Running time of current step
  size_t _currentStepTime;

  // Total running time so far
  size_t _totalRunningTime = 0;

  // Counter for the number of base states processed
  std::atomic<size_t> _stepBaseStatesProcessed;

  // Counter for the number of new states processed
  std::atomic<size_t> _stepNewStatesProcessed;

  // Time spent advancing runner state per step
  std::atomic<size_t> _runnerStateAdvanceThreadRawTime;
  std::atomic<size_t> _runnerStateAdvanceAverageTime;
  std::atomic<size_t> _runnerStateAdvanceAverageCumulativeTime = 0;

  // Time spent loading states into the runner
  std::atomic<size_t> _runnerStateLoadThreadRawTime;
  std::atomic<size_t> _runnerStateLoadAverageTime;
  std::atomic<size_t> _runnerStateLoadAverageCumulativeTime = 0;

  // Time spent saving runner states into the state db
  std::atomic<size_t> _runnerStateSaveThreadRawTime;
  std::atomic<size_t> _runnerStateSaveAverageTime;
  std::atomic<size_t> _runnerStateSaveAverageCumulativeTime = 0;

  // Time spent checking hash
  std::atomic<size_t> _checkHashThreadRawTime;
  std::atomic<size_t> _checkHashAverageTime;
  std::atomic<size_t> _checkHashAverageCumulativeTime = 0;

  // Rule checking time
  std::atomic<size_t> _ruleCheckingThreadRawTime;
  std::atomic<size_t> _ruleCheckingAverageTime;
  std::atomic<size_t> _ruleCheckingAverageCumulativeTime = 0;

  // Get free state time
  std::atomic<size_t> _getFreeStateThreadRawTime;
  std::atomic<size_t> _getFreeStateAverageTime;
  std::atomic<size_t> _getFreeStateAverageCumulativeTime = 0;

  // Return free state time
  std::atomic<size_t> _returnFreeStateThreadRawTime;
  std::atomic<size_t> _returnFreeStateAverageTime;
  std::atomic<size_t> _returnFreeStateAverageCumulativeTime = 0;

  // Reward calculation time
  std::atomic<size_t> _calculateRewardThreadRawTime;
  std::atomic<size_t> _calculateRewardAverageTime;
  std::atomic<size_t> _calculateRewardAverageCumulativeTime = 0;

  // Advance Hash DB time
  std::atomic<size_t> _advanceHashDbThreadRawTime;
  std::atomic<size_t> _advanceHashDbAverageTime;
  std::atomic<size_t> _advanceHashDbAverageCumulativeTime = 0;

  // Advance State DB time
  std::atomic<size_t> _advanceStateDbThreadRawTime;
  std::atomic<size_t> _advanceStateDbAverageTime;
  std::atomic<size_t> _advanceStateDbAverageCumulativeTime = 0;
};

} // namespace jaffarPlus