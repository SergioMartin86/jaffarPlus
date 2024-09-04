#pragma once

#include <algorithm>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <jaffarCommon/timing.hpp>
#include <jaffarCommon/parallel.hpp>
#include <gameList.hpp>
#include <emulatorList.hpp>
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
    jaffarCommon::logger::log("[J+] Using %lu worker threads.\n", _threadCount);

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

    // Reserving storage for timing information
    _threadStepTime.resize(_threadCount);
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

    // Resetting state counts
    _droppedStatesNoStorage           = 0;
    _droppedStatesFailedSerialization = 0;
    _droppedStatesCheckpoint          = 0;
    _repeatedStates                   = 0;
    _failedStates                     = 0;
    _winStates                        = 0;
    _normalStates                     = 0;

    // Resetting checkpoint counters
    _checkpointLevel     = 0;
    _checkpointTolerance = 0;
    _checkpointCutoff    = 0;

    // Resetting counter for the current step
    _currentStep = 0;
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

    // Clearing win state reward
    _stepBestWinState.reward = -std::numeric_limits<float>::infinity();

    // Performing one computation step in parallel
    JAFFAR_PARALLEL
    workerFunction();

    // Advancing hash database state
    const auto t0 = jaffarCommon::timing::now();
    _hashDb->advanceStep();
    _advanceHashDbThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t0);

    // Swapping next and current state databases
    const auto t1 = jaffarCommon::timing::now();
    _stateDb->advanceStep();
    _advanceStateDbThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t1);

    // Computing step time
    _currentStepTime = jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), tStep);

    // Computing total running time
    _totalRunningTime += _currentStepTime;

    // Getting maximum thread step time
    _maxThreadStepTimeThreadId = 0;
    _maxThreadStepTime = _threadStepTime[0];
    for (size_t i = 0; i < _threadCount; i++) if (_threadStepTime[i] > _maxThreadStepTime)
    {
      _maxThreadStepTimeThreadId = i;
      _maxThreadStepTime = _threadStepTime[i];
    }

    // Processing thread-average step timing
    _runnerStateAdvanceAverageTime = _runnerStateAdvanceThreadRawTime / _threadCount;
    _runnerStateLoadAverageTime    = _runnerStateLoadThreadRawTime / _threadCount;
    _runnerStateSaveAverageTime    = _runnerStateSaveThreadRawTime / _threadCount;
    _calculateHashAverageTime      = _calculateHashThreadRawTime / _threadCount;
    _checkHashAverageTime          = _checkHashThreadRawTime / _threadCount;
    _ruleCheckingAverageTime       = _ruleCheckingThreadRawTime / _threadCount;
    _getFreeStateAverageTime       = _getFreeStateThreadRawTime / _threadCount;
    _returnFreeStateAverageTime    = _returnFreeStateThreadRawTime / _threadCount;
    _calculateRewardAverageTime    = _calculateRewardThreadRawTime / _threadCount;
    _popBaseStateDbAverageTime     = _popBaseStateDbThreadRawTime / _threadCount;
    _getAllowedInputsAverageTime   = _getAllowedInputsThreadRawTime / _threadCount;
    _getCandidateInputsAverageTime = _getCandidateInputsThreadRawTime / _threadCount;
    _advanceHashDbAverageTime      = _advanceHashDbThreadRawTime.load();
    _advanceStateDbAverageTime     = _advanceStateDbThreadRawTime.load();

    // Sub-total thread-average step timing
    _subTotalAverageTime = 0;
    _subTotalAverageTime += _runnerStateAdvanceAverageTime;
    _subTotalAverageTime += _runnerStateLoadAverageTime;
    _subTotalAverageTime += _runnerStateSaveAverageTime;
    _subTotalAverageTime += _calculateHashAverageTime;
    _subTotalAverageTime += _checkHashAverageTime;
    _subTotalAverageTime += _ruleCheckingAverageTime;
    _subTotalAverageTime += _getFreeStateAverageTime;
    _subTotalAverageTime += _returnFreeStateAverageTime;
    _subTotalAverageTime += _calculateRewardAverageTime;
    _subTotalAverageTime += _getAllowedInputsAverageTime;
    _subTotalAverageTime += _getCandidateInputsAverageTime;
    _subTotalAverageTime += _popBaseStateDbAverageTime;
    _subTotalAverageTime += _advanceHashDbAverageTime;
    _subTotalAverageTime += _advanceStateDbAverageTime;

    // Processing cumulative timing
    _runnerStateAdvanceAverageCumulativeTime += _runnerStateAdvanceAverageTime;
    _runnerStateLoadAverageCumulativeTime += _runnerStateLoadAverageTime;
    _runnerStateSaveAverageCumulativeTime += _runnerStateSaveAverageTime;
    _calculateHashAverageCumulativeTime += _calculateHashAverageTime;
    _checkHashAverageCumulativeTime += _checkHashAverageTime;
    _ruleCheckingAverageCumulativeTime += _ruleCheckingAverageTime;
    _getFreeStateAverageCumulativeTime += _getFreeStateAverageTime;
    _returnFreeStateAverageCumulativeTime += _returnFreeStateAverageTime;
    _calculateRewardAverageCumulativeTime += _calculateRewardAverageTime;
    _popBaseStateDbAverageCumulativeTime += _popBaseStateDbAverageTime;
    _getAllowedInputsAverageCumulativeTime += _getAllowedInputsAverageTime;
    _getCandidateInputsAverageCumulativeTime += _getCandidateInputsAverageTime;
    _advanceHashDbAverageCumulativeTime += _advanceHashDbAverageTime;
    _advanceStateDbAverageCumulativeTime += _advanceStateDbAverageTime;

    // Sub-total cumulative time calculation
    _subTotalAverageCumulativeTime = 0;
    _subTotalAverageCumulativeTime += _runnerStateAdvanceAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _runnerStateLoadAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _runnerStateSaveAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _calculateHashAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _checkHashAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _ruleCheckingAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _getFreeStateAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _returnFreeStateAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _calculateRewardAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _popBaseStateDbAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _getAllowedInputsAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _getCandidateInputsAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _advanceHashDbAverageCumulativeTime;
    _subTotalAverageCumulativeTime += _advanceStateDbAverageCumulativeTime;

    // Processing state counters
    _totalBaseStatesProcessed += _stepBaseStatesProcessed;
    _totalNewStatesProcessed += _stepNewStatesProcessed;

    // Advancing step
    _currentStep++;
  }

  ~Engine() = default;

  // Relevant data for the driver

  auto &getStateDb() const { return _stateDb; }
  auto  getStepBestWinState() const { return _stepBestWinState; }
  auto  getWinStatesFound() const { return _winStates.load(); }
  auto  getStateCount() const { return _stateDb->getStateCount(); }

  /**
   * Information printing function
   */
  void printInfo()
  {
    // Printing information
    jaffarCommon::logger::log("[J+] Elapsed Time (Step/Total):                  %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_currentStepTime),
                              100.0 * ((double)(_subTotalAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_totalRunningTime),
                              100.0 * ((double)_subTotalAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Runner State Avance (Step/Total):        %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_runnerStateAdvanceAverageTime),
                              100.0 * ((double)(_runnerStateAdvanceAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_runnerStateAdvanceAverageCumulativeTime),
                              100.0 * ((double)_runnerStateAdvanceAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Runner State Load (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_runnerStateLoadAverageTime),
                              100.0 * ((double)(_runnerStateLoadAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_runnerStateLoadAverageCumulativeTime),
                              100.0 * ((double)_runnerStateLoadAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Runner State Save (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_runnerStateSaveAverageTime),
                              100.0 * ((double)(_runnerStateSaveAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_runnerStateSaveAverageCumulativeTime),
                              100.0 * ((double)_runnerStateSaveAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Hash Calculation (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_calculateHashAverageTime),
                              100.0 * ((double)(_calculateHashAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_calculateHashAverageCumulativeTime),
                              100.0 * ((double)_calculateHashAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Hash Checking (Step/Total):              %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_checkHashAverageTime),
                              100.0 * ((double)(_checkHashAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_checkHashAverageCumulativeTime),
                              100.0 * ((double)_checkHashAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Rule Checking (Step/Total):              %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_ruleCheckingAverageTime),
                              100.0 * ((double)(_ruleCheckingAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_ruleCheckingAverageCumulativeTime),
                              100.0 * ((double)_ruleCheckingAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Get Free State (Step/Total):             %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_getFreeStateAverageTime),
                              100.0 * ((double)(_getFreeStateAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_getFreeStateAverageCumulativeTime),
                              100.0 * ((double)_getFreeStateAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Return Free State (Step/Total):          %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_returnFreeStateAverageTime),
                              100.0 * ((double)(_returnFreeStateAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_returnFreeStateAverageCumulativeTime),
                              100.0 * ((double)_returnFreeStateAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Calculate Reward (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_calculateRewardAverageTime),
                              100.0 * ((double)(_calculateRewardAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_calculateRewardAverageCumulativeTime),
                              100.0 * ((double)_calculateRewardAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Popping Base State (Step/Total):         %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_popBaseStateDbAverageTime),
                              100.0 * ((double)(_popBaseStateDbAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_popBaseStateDbAverageCumulativeTime),
                              100.0 * ((double)_popBaseStateDbAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Get Allowed Inputs (Step/Total):         %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_getAllowedInputsAverageTime),
                              100.0 * ((double)(_getAllowedInputsAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_getAllowedInputsAverageCumulativeTime),
                              100.0 * ((double)_getAllowedInputsAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Get Candidate Inputs (Step/Total):       %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_getCandidateInputsAverageTime),
                              100.0 * ((double)(_getCandidateInputsAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_getCandidateInputsAverageCumulativeTime),
                              100.0 * ((double)_getCandidateInputsAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Advance Hash Db (Step/Total):            %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_advanceHashDbAverageTime),
                              100.0 * ((double)(_advanceHashDbAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_advanceHashDbAverageCumulativeTime),
                              100.0 * ((double)_advanceHashDbAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+]  + Advance State Db (Step/Total):           %9.3fs (%7.3f%%) / %9.3fs (%3.3f%%)\n",
                              1.0e-6 * (double)(_advanceStateDbAverageTime),
                              100.0 * ((double)(_advanceStateDbAverageTime) / (double)(_currentStepTime)),
                              1.0e-6 * (double)(_advanceStateDbAverageCumulativeTime),
                              100.0 * ((double)_advanceStateDbAverageCumulativeTime) / (double)(_totalRunningTime));

    jaffarCommon::logger::log("[J+] Checkpoint (Level/Tolerance/Cutoff):         %lu / %lu / %lu\n", _checkpointLevel, _checkpointTolerance, _checkpointCutoff);
    jaffarCommon::logger::log("[J+] Base States Processed:                       %.3f Mstates (Total: %.3f Mstates)\n",
                              1.0e-6 * (double)_stepBaseStatesProcessed,
                              1.0e-6 * (double)_totalBaseStatesProcessed);
    jaffarCommon::logger::log("[J+] New States Processed:                        %.3f Mstates (Total: %.3f Mstates)\n",
                              1.0e-6 * (double)_stepNewStatesProcessed,
                              1.0e-6 * (double)_totalNewStatesProcessed);

    jaffarCommon::logger::log("[J+] Base States Performance:                     %.3f Mstates/s (Average: %.3f Mstates/s)\n",
                              1.0e-6 * (double)_stepBaseStatesProcessed / (1.0e-6 * (double)_currentStepTime),
                              1.0e-6 * (double)_totalBaseStatesProcessed / (1.0e-6 * (double)_totalRunningTime));
    jaffarCommon::logger::log("[J+] New States Performance:                      %.3f Mstates/s (Average: %.3f Mstates/s)\n",
                              1.0e-6 * (double)_stepNewStatesProcessed / (1.0e-6 * (double)_currentStepTime),
                              1.0e-6 * (double)_totalNewStatesProcessed / (1.0e-6 * (double)_totalRunningTime));

    jaffarCommon::logger::log("[J+] Dropped States (No Storage Available):       %lu (%5.3f%% of New States Processed) \n",
                              _droppedStatesNoStorage.load(),
                              100.0 * (double)_droppedStatesNoStorage.load() / (double)_totalNewStatesProcessed);
    jaffarCommon::logger::log("[J+] Dropped States (Failed Serialization):       %lu (%5.3f%% of New States Processed) \n",
                              _droppedStatesFailedSerialization.load(),
                              100.0 * (double)_droppedStatesFailedSerialization.load() / (double)_totalNewStatesProcessed);
    jaffarCommon::logger::log("[J+] Dropped States (Checkpoint):                 %lu (%5.3f%% of New States Processed) \n",
                              _droppedStatesCheckpoint.load(),
                              100.0 * (double)_droppedStatesCheckpoint.load() / (double)_totalNewStatesProcessed);
    jaffarCommon::logger::log("[J+] Failed States:                               %lu (%5.3f%% of New States Processed) \n",
                              _failedStates.load(),
                              100.0 * (double)_failedStates.load() / (double)_totalNewStatesProcessed);
    jaffarCommon::logger::log("[J+] Repeated States:                             %lu (%5.3f%% of New States Processed) \n",
                              _repeatedStates.load(),
                              100.0 * (double)_repeatedStates.load() / (double)_totalNewStatesProcessed);
    jaffarCommon::logger::log("[J+] Normal States:                               %lu (%5.3f%% of New States Processed) \n",
                              _normalStates.load(),
                              100.0 * (double)_normalStates.load() / (double)_totalNewStatesProcessed);
    jaffarCommon::logger::log("[J+] Win States:                                  %lu (%5.3f%% of New States Processed) \n",
                              _winStates.load(),
                              100.0 * (double)_winStates.load() / (double)_totalNewStatesProcessed);

    // Print state database information
    jaffarCommon::logger::log("[J+] State Database Information:\n");
    _stateDb->printInfo();

    jaffarCommon::logger::log("[J+] Hash Database Information:\n");
    _hashDb->printInfo();

    // Printing candidate moves
    jaffarCommon::logger::log("[J+] Candidate Moves:\n");
    for (const auto &entry : _candidateInputsDetected)
      {
        jaffarCommon::logger::log("[J+]  + Hash: %s\n", jaffarCommon::hash::hashToString(entry.first).c_str());
        for (const auto input : entry.second) jaffarCommon::logger::log("[J+]    + %3lu %s\n", input, _runners[0]->getInputStringFromIndex(input).c_str());
      }
  }

  private:

  enum inputResult_t
  {
    repeated,
    droppedNoStorage,
    droppedFailedSerialization,
    droppedCheckpoint,
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

    // Starting to measure thread-specific step time
    const auto threadTime0 = jaffarCommon::timing::now();

    // Getting my runner
    auto &r = _runners[threadId];

    // Current base state to process
    const auto t             = jaffarCommon::timing::now();
    void      *baseStateData = _stateDb->popState();
    _popBaseStateDbThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t);

    // While there are still states in the database, keep on grabbing them
    while (baseStateData != nullptr)
      {
        // Increasing base state counter
        _stepBaseStatesProcessed++;

        // Load state into runner via the state database
        const auto t0 = jaffarCommon::timing::now();
        _stateDb->loadStateIntoRunner(*r, baseStateData);
        _runnerStateLoadThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t0);

        // Getting allowed inputs
        const auto t1            = jaffarCommon::timing::now();
        const auto allowedInputs = r->getAllowedInputs();
        _getAllowedInputsThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t1);

        // Trying out each possible input in the set
        for (auto inputItr = allowedInputs.begin(); inputItr != allowedInputs.end(); inputItr++) runNewInput(*r, baseStateData, *inputItr);

        // Getting candidate inputs
        const auto t2              = jaffarCommon::timing::now();
        auto       candidateInputs = r->getCandidateInputs();
        _getCandidateInputsThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t2);

        // Finding unique candidate inputs
        std::vector<InputSet::inputIndex_t> uniqueCandidateInputs;
        for (const auto &input : candidateInputs)
          if (allowedInputs.contains(input) == false) uniqueCandidateInputs.push_back(input);

        // Run each candidate input
        for (const auto input : uniqueCandidateInputs)
          {
            // Getting discriminating hash key to identify this type of states
            auto stateInputHash = r->getGame()->getStateInputHash();

            // Making sure we don't try the input if it was already detected
            if (_candidateInputsDetected.contains(stateInputHash))
              if (_candidateInputsDetected[stateInputHash].contains(input)) continue;

            // Running input
            const auto result = runNewInput(*r, baseStateData, input);

            // If this is not a repeated state, store it as new candidate input
            if (result != inputResult_t::repeated) _candidateInputsDetected[stateInputHash].insert(input);
          }

        // Return base state to the free state queue
        const auto t8 = jaffarCommon::timing::now();
        _stateDb->returnFreeState(baseStateData);
        _returnFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t8);

        // Pulling next state from the database
        const auto t9 = jaffarCommon::timing::now();
        baseStateData = _stateDb->popState();
        _popBaseStateDbThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t9);
      }

    // Taking final thread-specific time measurement
    _threadStepTime[threadId] = jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), threadTime0);
  }

  __INLINE__ inputResult_t runNewInput(Runner &r, const void *baseStateData, const InputSet::inputIndex_t input)
  {
    // Increasing new state counter
    _stepNewStatesProcessed++;

    // Re-loading base state
    const auto t0 = jaffarCommon::timing::now();
    _stateDb->loadStateIntoRunner(r, baseStateData);
    _runnerStateLoadThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t0);

    // Running input
    const auto result = runInput(r, input);

    // Update counters depending on the outcomes
    if (result == inputResult_t::normal) _normalStates++;
    if (result == inputResult_t::repeated) _repeatedStates++;
    if (result == inputResult_t::failed) _failedStates++;
    if (result == inputResult_t::win) _winStates++;
    if (result == inputResult_t::droppedNoStorage) _droppedStatesNoStorage++;
    if (result == inputResult_t::droppedFailedSerialization) _droppedStatesFailedSerialization++;
    if (result == inputResult_t::droppedCheckpoint) _droppedStatesCheckpoint++;

    // Checking whether this state's checkpoint is new
    const auto stateCheckpointLevel     = r.getGame()->getCheckpointLevel();
    const auto stateCheckpointTolerance = r.getGame()->getCheckpointTolerance();
    if (stateCheckpointLevel > _checkpointLevel)
      {
        _checkpointLevel     = stateCheckpointLevel;
        _checkpointTolerance = stateCheckpointTolerance;
        _checkpointCutoff    = _currentStep + stateCheckpointTolerance;
    }

    // Returning result
    return result;
  }

  __INLINE__ inputResult_t runInput(Runner &r, const InputSet::inputIndex_t input)
  {
    // Now advancing state with the provided input
    const auto t1 = jaffarCommon::timing::now();
    r.advanceState(input);
    _runnerStateAdvanceThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t1);

    // Computing runner hash
    const auto t2   = jaffarCommon::timing::now();
    const auto hash = r.computeHash();
    _calculateHashThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t2);

    // Checking if hash is repeated (i.e., has been seen before)
    const auto t3         = jaffarCommon::timing::now();
    bool       hashExists = _hashDb->checkHashExists(hash);
    _checkHashThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t3);

    // If state is repeated then we are not interested in it, continue
    if (hashExists == true) return inputResult_t::repeated;

    // Evaluating game rules based on the new state
    const auto t4 = jaffarCommon::timing::now();
    r.getGame()->evaluateRules();

    // Checking whether this state meets checkpoint
    if (_currentStep > _checkpointCutoff)
      {
        const auto stateCheckpointLevel = r.getGame()->getCheckpointLevel();

        // If state does not meet checkpoint, then do not process it further
        if (stateCheckpointLevel < _checkpointLevel) return inputResult_t::droppedCheckpoint;
    }

    // Determining state type
    r.getGame()->updateGameStateType();

    // Getting state type
    const auto stateType = r.getGame()->getStateType();
    _ruleCheckingThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t4);

    // Now we have determined the state is not repeated, check if it's not a failed state
    if (stateType == Game::stateType_t::fail) return inputResult_t::failed;

    // Now that the state is not failed nor repeated, this is effectively a new state to add
    const auto t5           = jaffarCommon::timing::now();
    void      *newStateData = _stateDb->getFreeState();
    _getFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t5);

    // If couldn't get any memory, simply drop the state
    if (newStateData == nullptr) return inputResult_t::droppedNoStorage;

    // Updating state reward
    const auto t6 = jaffarCommon::timing::now();
    r.getGame()->updateReward();

    // Getting state reward
    const auto reward = r.getGame()->getReward();
    _calculateRewardThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t6);

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
        _returnFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t7);

        // Returning a win result
        return inputResult_t::win;
    }

    // If this is a normal state and has possible inputs store it in the next state database
    if (stateType == Game::stateType_t::normal)
      {
        // If this is a normal state, push into the state database
        const auto t8      = jaffarCommon::timing::now();
        auto       success = _stateDb->pushState(reward, r, newStateData);
        _runnerStateSaveThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t8);

        // Attempting to serialize state and push it into the database
        // This might fail when using differential serialization due to insufficient space for differentials
        // In that, case we just drop the state and continue, while keeping a counter
        if (success == false)
          {
            // Freeing up state memory
            const auto t9 = jaffarCommon::timing::now();
            _stateDb->returnFreeState(newStateData);
            _returnFreeStateThreadRawTime += jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t9);

            // Returning dropped result by failed serialization
            return inputResult_t::droppedFailedSerialization;
        }
    }

    // If store succeeded, return a normal execution
    return inputResult_t::normal;
  }

  // Set of candidate inputs
  jaffarCommon::concurrent::HashMap_t<jaffarCommon::hash::hash_t, jaffarCommon::concurrent::HashSet_t<InputSet::inputIndex_t>> _candidateInputsDetected;

  // Counter for the current step
  size_t _currentStep = 0;

  // Collection of runners for the workers to use
  std::vector<std::unique_ptr<Runner>> _runners;

  // The thread-safe state database that contains current and next step's states.
  std::unique_ptr<jaffarPlus::stateDb::Base> _stateDb;

  // The thread-safe hash database to check for repeated states
  std::unique_ptr<jaffarPlus::HashDb> _hashDb;

  // Set of win states found
  std::mutex _stepBestWinStateLock;
  winState   _stepBestWinState;

  // Checkpoint information
  size_t _checkpointLevel;
  size_t _checkpointTolerance;
  size_t _checkpointCutoff;

  ///////////////// Configuration

  // Thread count (set by openMP)
  size_t _threadCount;

  //////////////// Statistics

  // Counter for dropped states due to lack of free states
  std::atomic<size_t> _droppedStatesNoStorage;

  // Counter for dropped states due to failed (differential) serialization
  std::atomic<size_t> _droppedStatesFailedSerialization;

  // Counter for dropped states due to not meeting checkpoint
  std::atomic<size_t> _droppedStatesCheckpoint;

  // Counter for repeated states (detected via hash collision)
  std::atomic<size_t> _repeatedStates;

  // Counter for failed states (reached a point in the game that is considered a loss)
  std::atomic<size_t> _failedStates;

  // Counter for win states
  std::atomic<size_t> _winStates;

  // Counter for normal states
  std::atomic<size_t> _normalStates;

  // Counter for the number of base states processed
  std::atomic<size_t> _stepBaseStatesProcessed;
  std::atomic<size_t> _totalBaseStatesProcessed;

  // Counter for the number of new states processed
  std::atomic<size_t> _stepNewStatesProcessed;
  std::atomic<size_t> _totalNewStatesProcessed;

  //////////////// Timing

  // Overall running time of current step
  size_t _currentStepTime;

  // Thread-specific running time of current step
  std::vector<size_t> _threadStepTime;
  size_t _maxThreadStepTime;
  size_t _maxThreadStepTimeThreadId;

  // Total running time so far
  size_t _totalRunningTime;

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

  // Get candidate inputs time
  std::atomic<size_t> _getAllowedInputsThreadRawTime;
  std::atomic<size_t> _getAllowedInputsAverageTime;
  std::atomic<size_t> _getAllowedInputsAverageCumulativeTime;

  // Get candidate inputs time
  std::atomic<size_t> _getCandidateInputsThreadRawTime;
  std::atomic<size_t> _getCandidateInputsAverageTime;
  std::atomic<size_t> _getCandidateInputsAverageCumulativeTime;

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

  // Sub-total calculation
  size_t _subTotalAverageTime;
  size_t _subTotalAverageCumulativeTime;
};

} // namespace jaffarPlus