#pragma once

#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <jaffarCommon/timing.hpp>
#include <jaffarCommon/parallel.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "game.hpp"
#include "hashDb.hpp"
#include "runner.hpp"
#include "stateDb/numa.hpp"
#include "stateDb/plain.hpp"

namespace jaffarPlus
{

class Engine final
{
  public:

  // Base constructor
  Engine(const nlohmann::json &emulatorConfig, const nlohmann::json &gameConfig, const nlohmann::json &runnerConfig, const nlohmann::json &engineConfig)
  {
    // Getting number of threads
    _threadCount = jaffarCommon::parallel::getMaxThreadCount();

    // Sanity check
    if (_threadCount == 0) JAFFAR_THROW_LOGIC("The number of worker threads must be at least one. Provided: %lu\n", _threadCount);

    // Printing initial information
    jaffarCommon::logger::log("[J++] Using %lu worker threads.\n", _threadCount);

    // Creating storage for the runnners (one per thread)
    _runners.resize(_threadCount);

    // Creating runners, one per thread
    JAFFAR_PARALLEL
    {
      // Getting my thread id
      int threadId = jaffarCommon::parallel::getThreadId();

      // Creating runner from the configuration
      auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);

      // Storing runner
      _runners[threadId] = std::move(r);
    }

    // Grabbing a runner to do continue build the state databases
    auto &r = *_runners[0];

    // Creating State database
    const auto &stateDatabaseJs             = jaffarCommon::json::getObject(engineConfig, "State Database");
    const auto &stateDatabaseType           = jaffarCommon::json::getString(stateDatabaseJs, "Type");
    bool        stateDatabaseTypeRecognized = false;
    if (stateDatabaseType == "Plain")
    {
      _stateDb                    = std::make_unique<jaffarPlus::stateDb::Plain>(r, jaffarCommon::json::getObject(engineConfig, "State Database"));
      stateDatabaseTypeRecognized = true;
    }
    if (stateDatabaseType == "Numa Aware")
    {
      _stateDb                    = std::make_unique<jaffarPlus::stateDb::Numa>(r, jaffarCommon::json::getObject(engineConfig, "State Database"));
      stateDatabaseTypeRecognized = true;
    }
    if (stateDatabaseTypeRecognized == false) JAFFAR_THROW_LOGIC("State database type '%s' not recognized", stateDatabaseType.c_str());

    // Creating hash database
    _hashDb = std::make_unique<jaffarPlus::HashDb>(jaffarCommon::json::getObject(engineConfig, "Hash Database"));
  };

  /**
   * Resets execution back to step zero and clears all databases and counters
   */
  void initialize()
  {
    // Initializing runners, one per thread
    JAFFAR_PARALLEL
    {
      // Getting my thread id
      int threadId = jaffarCommon::parallel::getThreadId();

      // Creating thread's own runner
      auto &r = _runners[threadId];

      // Initializing runner
      r->initialize();

      // Disable emulator rendering
      r->getGame()->getEmulator()->disableRendering();
    }

    // Initializing State Db
    _stateDb->initialize();

    // Initializing hash database
    _hashDb->initialize();

    // Grabbing a runner to do continue initialization
    auto &r = *_runners[0];

    // Getting memory for the reference state
    const auto stateSize = r.getStateSize();

    // Allocating memory for the best win state
    _stepBestWinState.stateData = malloc(stateSize);

    // Allocating memory for reference data
    uint8_t referenceData[stateSize];

    // Serializing the initial state without compression to use as reference
    jaffarCommon::serializer::Contiguous s(referenceData, stateSize);
    r.serializeState(s);

    // Setting initial reference data, necessary for differential compression (if enabled)
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

    // Getting hash from first state
    const auto hash = r.computeHash();

    // Adding it to the hash DB
    _hashDb->insertHash(hash);
  }

  void reset()
  {
    // Initializing state counters
    _totalBaseStatesProcessed = 0;
    _totalNewStatesProcessed  = 0;

    // Initializing cumulative timing
    _runnerStateAdvanceAverageCumulativeTime = 0;
    _runnerStateLoadAverageCumulativeTime    = 0;
    _runnerStateSaveAverageCumulativeTime    = 0;
    _calculateHashAverageCumulativeTime      = 0;
    _checkHashAverageCumulativeTime          = 0;
    _ruleCheckingAverageCumulativeTime       = 0;
    _getFreeStateAverageCumulativeTime       = 0;
    _returnFreeStateAverageCumulativeTime    = 0;
    _calculateRewardAverageCumulativeTime    = 0;
    _advanceHashDbAverageCumulativeTime      = 0;
    _advanceStateDbAverageCumulativeTime     = 0;
    _popBaseStateDbAverageCumulativeTime     = 0;

    // Resetting total running time
    _totalRunningTime = 0;

    // Resetting dropped state count
    _droppedStatesNoStorage           = 0;
    _droppedStatesFailedSerialization = 0;
  }

  /**
   * Runs a single step of the Jaffar Engine
   */
  void runStep()
  {
    // Computing step time
    const auto tStep = jaffarCommon::timing::now();

    // Clearing step timing
    _runnerStateAdvanceThreadRawTime = 0;
    _runnerStateLoadThreadRawTime    = 0;
    _runnerStateSaveThreadRawTime    = 0;
    _calculateHashThreadRawTime      = 0;
    _checkHashThreadRawTime          = 0;
    _ruleCheckingThreadRawTime       = 0;
    _getFreeStateThreadRawTime       = 0;
    _returnFreeStateThreadRawTime    = 0;
    _calculateRewardThreadRawTime    = 0;
    _advanceHashDbThreadRawTime      = 0;
    _advanceStateDbThreadRawTime     = 0;
    _popBaseStateDbThreadRawTime     = 0;

    // Clearing step counters
    _stepBaseStatesProcessed = 0;
    _stepNewStatesProcessed  = 0;
    _stepWinStatesFound      = 0;

    // Clearing win state reward
    _stepBestWinState.reward = -std::numeric_limits<float>::infinity();

    // Performing one computation step in parallel
    JAFFAR_PARALLEL
    workerFunction();

    // Advancing hash database state
    const auto t0 = jaffarCommon::timing::now();
    _hashDb->advanceStep();
    _advanceHashDbThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t0);

    // Swapping next and current state databases
    const auto t1 = jaffarCommon::timing::now();
    _stateDb->advanceStep();
    _advanceStateDbThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t1);

    // Processing step and cumulative timing
    _runnerStateAdvanceAverageTime = _runnerStateAdvanceThreadRawTime / _threadCount;
    _runnerStateAdvanceAverageCumulativeTime += _runnerStateAdvanceAverageTime;
    _runnerStateLoadAverageTime = _runnerStateLoadThreadRawTime / _threadCount;
    _runnerStateLoadAverageCumulativeTime += _runnerStateLoadAverageTime;
    _runnerStateSaveAverageTime = _runnerStateSaveThreadRawTime / _threadCount;
    _runnerStateSaveAverageCumulativeTime += _runnerStateSaveAverageTime;
    _calculateHashAverageTime = _calculateHashThreadRawTime / _threadCount;
    _calculateHashAverageCumulativeTime += _calculateHashAverageTime;
    _checkHashAverageTime = _checkHashThreadRawTime / _threadCount;
    _checkHashAverageCumulativeTime += _checkHashAverageTime;
    _ruleCheckingAverageTime = _ruleCheckingThreadRawTime / _threadCount;
    _ruleCheckingAverageCumulativeTime += _ruleCheckingAverageTime;
    _getFreeStateAverageTime = _getFreeStateThreadRawTime / _threadCount;
    _getFreeStateAverageCumulativeTime += _getFreeStateAverageTime;
    _returnFreeStateAverageTime = _returnFreeStateThreadRawTime / _threadCount;
    _returnFreeStateAverageCumulativeTime += _returnFreeStateAverageTime;
    _calculateRewardAverageTime = _calculateRewardThreadRawTime / _threadCount;
    _calculateRewardAverageCumulativeTime += _calculateRewardAverageTime;
    _popBaseStateDbAverageTime = _popBaseStateDbThreadRawTime / _threadCount;
    _popBaseStateDbAverageCumulativeTime += _popBaseStateDbAverageTime;
    _advanceHashDbAverageTime = _advanceHashDbThreadRawTime.load();
    _advanceHashDbAverageCumulativeTime += _advanceHashDbAverageTime;
    _advanceStateDbAverageTime = _advanceStateDbThreadRawTime.load();
    _advanceStateDbAverageCumulativeTime += _advanceStateDbAverageTime;

    // Processing state counters
    _totalBaseStatesProcessed += _stepBaseStatesProcessed;
    _totalNewStatesProcessed += _stepNewStatesProcessed;

    // Computing step time
    _currentStepTime = jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), tStep);

    // Computing total running time
    _totalRunningTime += _currentStepTime;
  }

  ~Engine() = default;

  // Relevant data for the driver

  auto &getStateDb() const { return _stateDb; }
  auto  getStepBestWinState() const { return _stepBestWinState; }
  auto  getStepWinStatesFound() const { return _stepWinStatesFound.load(); }
  auto  getStateCount() const { return _stateDb->getStateCount(); }

  /**
   * Information printing function
   */
  void printInfo()
  {
    // Printing information
    jaffarCommon::logger::log("[J++] Elapsed Time (Step/Total):                  %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_currentStepTime),
                              100.0,
                              1.0e-9 * (double)(_totalRunningTime),
                              100.0);

    jaffarCommon::logger::log("[J++]  + Runner State Avance (Step/Total):        %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_runnerStateAdvanceAverageTime),
                              100.0 * ((double)(_runnerStateAdvanceAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_runnerStateAdvanceAverageCumulativeTime),
                              100.0 * ((double)_runnerStateAdvanceAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Runner State Load (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_runnerStateLoadAverageTime),
                              100.0 * ((double)(_runnerStateLoadAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_runnerStateLoadAverageCumulativeTime),
                              100.0 * ((double)_runnerStateLoadAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Runner State Save (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_runnerStateSaveAverageTime),
                              100.0 * ((double)(_runnerStateSaveAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_runnerStateSaveAverageCumulativeTime),
                              100.0 * ((double)_runnerStateSaveAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Hash Calculation (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_calculateHashAverageTime),
                              100.0 * ((double)(_calculateHashAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_calculateHashAverageCumulativeTime),
                              100.0 * ((double)_calculateHashAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Hash Checking (Step/Total):              %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_checkHashAverageTime),
                              100.0 * ((double)(_checkHashAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_checkHashAverageCumulativeTime),
                              100.0 * ((double)_checkHashAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Rule Checking (Step/Total):              %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_ruleCheckingAverageTime),
                              100.0 * ((double)(_ruleCheckingAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_ruleCheckingAverageCumulativeTime),
                              100.0 * ((double)_ruleCheckingAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Get Free State (Step/Total):             %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_getFreeStateAverageTime),
                              100.0 * ((double)(_getFreeStateAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_getFreeStateAverageCumulativeTime),
                              100.0 * ((double)_getFreeStateAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Return Free State (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_returnFreeStateAverageTime),
                              100.0 * ((double)(_returnFreeStateAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_returnFreeStateAverageCumulativeTime),
                              100.0 * ((double)_returnFreeStateAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Calculate Reward (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_calculateRewardAverageTime),
                              100.0 * ((double)(_calculateRewardAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_calculateRewardAverageCumulativeTime),
                              100.0 * ((double)_calculateRewardAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Popping Base State (Step/Total):         %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_popBaseStateDbAverageTime),
                              100.0 * ((double)(_popBaseStateDbAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_popBaseStateDbAverageCumulativeTime),
                              100.0 * ((double)_popBaseStateDbAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Advance Hash Db (Step/Total):            %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_advanceHashDbAverageTime),
                              100.0 * ((double)(_advanceHashDbAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_advanceHashDbAverageCumulativeTime),
                              100.0 * ((double)_advanceHashDbAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++]  + Advance State Db (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-9 * (double)(_advanceStateDbAverageTime),
                              100.0 * ((double)(_advanceStateDbAverageTime) / (double)(_currentStepTime)),
                              1.0e-9 * (double)(_advanceStateDbAverageCumulativeTime),
                              100.0 * ((double)_advanceStateDbAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J++] Base States Processed:                       %.3f Mstates (Total: %.3f Mstates)\n",
                              1.0e-6 * (double)_stepBaseStatesProcessed,
                              1.0e-6 * (double)_totalBaseStatesProcessed);
    jaffarCommon::logger::log("[J++] New States Processed:                        %.3f Mstates (Total: %.3f Mstates)\n",
                              1.0e-6 * (double)_stepNewStatesProcessed,
                              1.0e-6 * (double)_totalNewStatesProcessed);

    jaffarCommon::logger::log("[J++] Base States Performance:                     %.3f Mstates/s (Average: %.3f Mstates/s)\n",
                              1.0e-6 * (double)_stepBaseStatesProcessed / (1.0e-9 * (double)_currentStepTime),
                              1.0e-6 * (double)_totalBaseStatesProcessed / (1.0e-9 * (double)_totalRunningTime));
    jaffarCommon::logger::log("[J++] New States Performance:                      %.3f Mstates/s (Average: %.3f Mstates/s)\n",
                              1.0e-6 * (double)_stepNewStatesProcessed / (1.0e-9 * (double)_currentStepTime),
                              1.0e-6 * (double)_totalNewStatesProcessed / (1.0e-9 * (double)_totalRunningTime));

    jaffarCommon::logger::log("[J++] Dropped States (No Storage Available):       %lu (%5.3f%% of New States Processed) \n",
                              _droppedStatesNoStorage.load(),
                              100.0 * (double)_droppedStatesNoStorage.load() / (double)_totalNewStatesProcessed);
    jaffarCommon::logger::log("[J++] Dropped States (Failed Serialization):       %lu\n", _droppedStatesFailedSerialization.load());

    // Print state database information
    jaffarCommon::logger::log("[J++] State Database Information:\n");
    _stateDb->printInfo();

    jaffarCommon::logger::log("[J++] Hash Database Information:\n");
    _hashDb->printInfo();
  }

  private:

  enum inputResult_t
  {
    repeated,
    droppedNoStorage,
    droppedFailedSerialization,
    failed,
    normal,
    win
  };

  struct winState
  {
    float reward;
    void *stateData;
  };

  /**
   * The main worker function -- executes entirely in parallel
   */
  void workerFunction()
  {
    // Getting my thread id
    const auto threadId = jaffarCommon::parallel::getThreadId();

    // Getting my runner
    auto &r = _runners[threadId];

    // Current base state to process
    const auto t             = jaffarCommon::timing::now();
    void      *baseStateData = _stateDb->popState();
    _popBaseStateDbThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t);

    // While there are still states in the database, keep on grabbing them
    while (baseStateData != nullptr)
    {
      // Increasing base state counter
      _stepBaseStatesProcessed++;

      // Load state into runner via the state database
      const auto t0 = jaffarCommon::timing::now();
      _stateDb->loadStateIntoRunner(*r, baseStateData);
      _runnerStateLoadThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t0);

      // Getting possible inputs
      const auto &possibleInputs = r->getPossibleInputs();

      // Flag to check whether to free up base state data
      bool freeBaseStateData = true;

      // Trying out each possible input
      for (auto inputItr = possibleInputs.begin(); inputItr != possibleInputs.end(); inputItr++)
      {
        // Increasing new state counter
        _stepNewStatesProcessed++;

        // We only need to reload the base state data if this is not the first input
        const auto t0 = jaffarCommon::timing::now();
        if (inputItr != possibleInputs.begin()) _stateDb->loadStateIntoRunner(*r, baseStateData);
        _runnerStateLoadThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t0);

        // If this is the last input, then reuse the base state as destination
        void *freeState = nullptr;
        if (std::next(inputItr) == possibleInputs.end()) freeState = baseStateData;

        // Running input
        auto result = runInput(*r, *inputItr, freeState);

        // Update counters depending on the outcomes
        if (result == inputResult_t::win) _stepWinStatesFound++;
        if (result == inputResult_t::droppedNoStorage) _droppedStatesNoStorage++;
        if (result == inputResult_t::droppedFailedSerialization) _droppedStatesFailedSerialization++;
        if (result == inputResult_t::droppedFailedSerialization) _droppedStatesFailedSerialization++;

        // If used the base state data, there is no need to free it up later
        if (freeState != nullptr && result == inputResult_t::normal) freeBaseStateData = false;
      }

      // Return base state to the free state queue
      const auto t8 = jaffarCommon::timing::now();
      if (freeBaseStateData == true) _stateDb->returnFreeState(baseStateData);
      _returnFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t8);

      // Pulling next state from the database
      const auto t9 = jaffarCommon::timing::now();
      baseStateData = _stateDb->popState();
      _popBaseStateDbThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t9);
    }
  }

  __INLINE__ inputResult_t runInput(Runner &r, const InputSet::inputIndex_t input, void *freeState = nullptr)
  {
    // Now advancing state with the provided input
    const auto t1 = jaffarCommon::timing::now();
    r.advanceState(input);
    _runnerStateAdvanceThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t1);

    // Computing runner hash
    const auto t2   = jaffarCommon::timing::now();
    const auto hash = r.computeHash();
    _calculateHashThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t2);

    // Checking if hash is repeated (i.e., has been seen before)
    const auto t3         = jaffarCommon::timing::now();
    bool       hashExists = _hashDb->checkHashExists(hash);
    _checkHashThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t3);

    // If state is repeated then we are not interested in it, continue
    if (hashExists == true) return inputResult_t::repeated;

    // Evaluating game rules based on the new state
    const auto t4 = jaffarCommon::timing::now();
    r.getGame()->evaluateRules();

    // Determining state type
    r.getGame()->updateGameStateType();

    // Getting state type
    const auto stateType = r.getGame()->getStateType();
    _ruleCheckingThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t4);

    // Now we have determined the state is not repeated, check if it's not a failed state
    if (stateType == Game::stateType_t::fail) return inputResult_t::failed;

    // Now that the state is not failed nor repeated, this is effectively a new state to add
    void *newStateData = freeState;

    // It the free state was not provided, try and grab one from the state db
    const auto t5 = jaffarCommon::timing::now();
    if (newStateData == nullptr) newStateData = _stateDb->getFreeState();
    _getFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t5);

    // If couldn't get any memory, simply drop the state
    if (newStateData == nullptr) return inputResult_t::droppedNoStorage;

    // Updating state reward
    const auto t6 = jaffarCommon::timing::now();
    r.getGame()->updateReward();

    // Getting state reward
    const auto reward = r.getGame()->getReward();
    _calculateRewardThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t6);

    // If this is a win state, register it and return
    if (stateType == Game::stateType_t::win)
    {
      ///////////// Best win state needs to be stored if its better than any previous one found

      // Check if the new win state is the best and store it in that case
      _stepBestWinStateLock.lock();
      if (reward > _stepBestWinState.reward)
      {
        _stateDb->saveStateFromRunner(r, _stepBestWinState.stateData);
        _stepBestWinState.reward = reward;
      }
      _stepBestWinStateLock.unlock();

      // Freeing up the state data
      const auto t7 = jaffarCommon::timing::now();
      _stateDb->returnFreeState(newStateData);
      _returnFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t7);

      return inputResult_t::win;
    }

    // If this is a normal state and has possible inputs store it in the next state database
    if (stateType == Game::stateType_t::normal)
    {
      // If this is a normal state, push into the state database
      const auto t8      = jaffarCommon::timing::now();
      auto       success = _stateDb->pushState(reward, r, newStateData);
      _runnerStateSaveThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t8);

      // Attempting to serialize state and push it into the database
      // This might fail when using differential serialization due to insufficient space for differentials
      // In that, case we just drop the state and continue, while keeping a counter
      if (success == false)
      {
        const auto t9 = jaffarCommon::timing::now();
        _stateDb->returnFreeState(newStateData);
        _returnFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaNanoseconds(jaffarCommon::timing::now(), t9);
        return inputResult_t::droppedFailedSerialization;
      }
    }

    // If store succeeded, return a normal execution
    return inputResult_t::normal;
  }

  // Collection of runners for the workers to use
  std::vector<std::unique_ptr<Runner>> _runners;

  // The thread-safe state database that contains current and next step's states.
  std::unique_ptr<jaffarPlus::stateDb::Base> _stateDb;

  // The thread-safe hash database to check for repeated states
  std::unique_ptr<jaffarPlus::HashDb> _hashDb;

  // Set of win states found
  std::mutex          _stepBestWinStateLock;
  winState            _stepBestWinState;
  std::atomic<size_t> _stepWinStatesFound;

  ///////////////// Configuration

  // Thread count (set by openMP)
  size_t _threadCount;

  //////////////// Statistics

  // Running time of current step
  size_t _currentStepTime;

  // Total running time so far
  size_t _totalRunningTime;

  // Counter for dropped states due to lack of free states
  std::atomic<size_t> _droppedStatesNoStorage;

  // Counter for dropped states due to failed (differential) serialization
  std::atomic<size_t> _droppedStatesFailedSerialization;

  // Counter for the number of base states processed
  std::atomic<size_t> _stepBaseStatesProcessed;
  std::atomic<size_t> _totalBaseStatesProcessed;

  // Counter for the number of new states processed
  std::atomic<size_t> _stepNewStatesProcessed;
  std::atomic<size_t> _totalNewStatesProcessed;

  // Time spent advancing runner state per step
  std::atomic<size_t> _runnerStateAdvanceThreadRawTime;
  std::atomic<size_t> _runnerStateAdvanceAverageTime;
  std::atomic<size_t> _runnerStateAdvanceAverageCumulativeTime;

  // Time spent loading states into the runner
  std::atomic<size_t> _runnerStateLoadThreadRawTime;
  std::atomic<size_t> _runnerStateLoadAverageTime;
  std::atomic<size_t> _runnerStateLoadAverageCumulativeTime;

  // Time spent saving runner states into the state db
  std::atomic<size_t> _runnerStateSaveThreadRawTime;
  std::atomic<size_t> _runnerStateSaveAverageTime;
  std::atomic<size_t> _runnerStateSaveAverageCumulativeTime;

  // Time spent calculating hash
  std::atomic<size_t> _calculateHashThreadRawTime;
  std::atomic<size_t> _calculateHashAverageTime;
  std::atomic<size_t> _calculateHashAverageCumulativeTime;

  // Time spent checking hash
  std::atomic<size_t> _checkHashThreadRawTime;
  std::atomic<size_t> _checkHashAverageTime;
  std::atomic<size_t> _checkHashAverageCumulativeTime;

  // Rule checking time
  std::atomic<size_t> _ruleCheckingThreadRawTime;
  std::atomic<size_t> _ruleCheckingAverageTime;
  std::atomic<size_t> _ruleCheckingAverageCumulativeTime;

  // Get free state time
  std::atomic<size_t> _getFreeStateThreadRawTime;
  std::atomic<size_t> _getFreeStateAverageTime;
  std::atomic<size_t> _getFreeStateAverageCumulativeTime;

  // Return free state time
  std::atomic<size_t> _returnFreeStateThreadRawTime;
  std::atomic<size_t> _returnFreeStateAverageTime;
  std::atomic<size_t> _returnFreeStateAverageCumulativeTime;

  // Reward calculation time
  std::atomic<size_t> _calculateRewardThreadRawTime;
  std::atomic<size_t> _calculateRewardAverageTime;
  std::atomic<size_t> _calculateRewardAverageCumulativeTime;

  // Advance Hash DB time
  std::atomic<size_t> _advanceHashDbThreadRawTime;
  std::atomic<size_t> _advanceHashDbAverageTime;
  std::atomic<size_t> _advanceHashDbAverageCumulativeTime;

  // Advance State DB time
  std::atomic<size_t> _advanceStateDbThreadRawTime;
  std::atomic<size_t> _advanceStateDbAverageTime;
  std::atomic<size_t> _advanceStateDbAverageCumulativeTime;

  // Popping states from the State DB time
  std::atomic<size_t> _popBaseStateDbThreadRawTime;
  std::atomic<size_t> _popBaseStateDbAverageTime;
  std::atomic<size_t> _popBaseStateDbAverageCumulativeTime;
};

} // namespace jaffarPlus