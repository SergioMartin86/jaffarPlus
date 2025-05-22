#pragma once

#include "engine.hpp"
#include "game.hpp"
#include "runner.hpp"
#include <chrono>
#include <cstdlib>
#include <limits>

namespace jaffarPlus
{

class Driver final

{
public:
  enum exitReason_t
  {
    /// Found a win state
    winStateFound = 0,

    /// Engine ran out of states
    outOfStates = 1,

    /// Maximum step reached
    maximumStepReached = 2
  };

  // Base constructor
  Driver(const std::string& configFilePath, const nlohmann::json& config) : _configFilePath(configFilePath)
  {
    // Getting job identifier from the system timer
    auto currentTime = std::chrono::system_clock::now();
    _jobId           = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count();

    // Getting driver configuration
    const auto driverConfig = jaffarCommon::json::getObject(config, "Driver Configuration");

    // Getting end win delay config
    _endOnFirstWinState = jaffarCommon::json::getBoolean(driverConfig, "End On First Win State");

    // Getting maximum number of steps (zero is not established)
    _maxSteps = jaffarCommon::json::getNumber<uint32_t>(driverConfig, "Max Steps");

    // For testing purposes, the maximum number of steps can be overriden via environment variables
    if (auto* value = std::getenv("JAFFAR_DRIVER_OVERRIDE_DRIVER_MAX_STEP")) _maxSteps = std::stoul(value);

    // Getting intermediate result configuration
    const auto saveIntermediateResultsJs = jaffarCommon::json::getObject(driverConfig, "Save Intermediate Results");
    _saveIntermediateResultsEnabled      = jaffarCommon::json::getBoolean(saveIntermediateResultsJs, "Enabled");
    _saveIntermediateFrequency           = jaffarCommon::json::getNumber<float>(saveIntermediateResultsJs, "Frequency (s)");
    _saveIntermediateBestSolutionPath    = jaffarCommon::json::getString(saveIntermediateResultsJs, "Best Solution Path");
    _saveIntermediateWorstSolutionPath   = jaffarCommon::json::getString(saveIntermediateResultsJs, "Worst Solution Path");
    _saveIntermediateBestStatePath       = jaffarCommon::json::getString(saveIntermediateResultsJs, "Best State Path");
    _saveIntermediateWorstStatePath      = jaffarCommon::json::getString(saveIntermediateResultsJs, "Worst State Path");

    // Getting component configurations
    auto emulatorConfig = jaffarCommon::json::getObject(config, "Emulator Configuration");
    auto gameConfig     = jaffarCommon::json::getObject(config, "Game Configuration");
    auto runnerConfig   = jaffarCommon::json::getObject(config, "Runner Configuration");
    auto engineConfig   = jaffarCommon::json::getObject(config, "Engine Configuration");

    // Creating runner from the configuration
    _runner = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);

    // Creating engine from the configuration
    _engine = std::make_unique<Engine>(emulatorConfig, gameConfig, runnerConfig, engineConfig);
  }

  ~Driver() {}

  // Resets the execution back to the starting point
  void initialize()
  {
    // Resetting step counter
    _currentStep = 0;

    // Resetting win state counter
    _winStatesFound = 0;

    // Resetting best states reward
    _bestWinStateReward = -std::numeric_limits<float>::infinity();
    _bestStateReward    = -std::numeric_limits<float>::infinity();

    // Resetting worst state reward
    _worstStateReward = std::numeric_limits<float>::infinity();

    // Initializing runner
    _runner->initialize();

    // Initializing engine
    _engine->initialize();

    // Allocating space for the current best and worst states
    _stateSize = _runner->getStateSize();
    _bestStateStorage.resize(_stateSize);
    _worstStateStorage.resize(_stateSize);
  }

  // Start running engine loop
  int run()
  {
    // Resetting engine
    _engine->reset();

    // Internal flag to indicate we are still running
    _hasFinished = false;

    // If using ncurses, initialize terminal now
    jaffarCommon::logger::initializeTerminal();

    // Storage for the exit
    exitReason_t exitReason;

    // Starting intermediate result saving thread
    std::thread intermediateResultSaverThread;
    if (_saveIntermediateResultsEnabled == true) intermediateResultSaverThread = std::thread([this]() { intermediateResultSaveLoop(); });

    // Running engine until a termination point
    while (true)
    {
      // If found winning state, report it now
      if (_endOnFirstWinState && _engine->getWinStatesFound() > 0)
      {
        exitReason = exitReason_t::winStateFound;
        break;
      }

      // If ran out of states, finish now
      if (_engine->getStateCount() == 0)
      {
        exitReason = _engine->getWinStatesFound() > 0 ? exitReason_t::winStateFound : exitReason_t::outOfStates;
        break;
      }

      // If maximum step established and reached, finish now
      if (_maxSteps > 0 && _currentStep >= _maxSteps)
      {
        if (_winStatesFound > 0) exitReason = exitReason_t::winStateFound;
        if (_winStatesFound == 0) exitReason = exitReason_t::maximumStepReached;
        break;
      }

      // Updating best and worst states
      updateBestState();
      updateWorstState();

      // Printing information
      printInfo();

      // Running engine step
      _engine->runStep();

      // Summing amount of win states found
      _winStatesFound = _engine->getWinStatesFound();

      // Increasing step counter
      _currentStep++;
    }

    // Setting finalized flag
    _hasFinished = true;

    // Waiting for saver thread
    if (_saveIntermediateResultsEnabled == true) intermediateResultSaverThread.join();

    // If using ncurses, terminate terminal now
    jaffarCommon::logger::finalizeTerminal();

    // Updating and storing best states
    updateBestState();
    saveBestStateInformation();

    // Final report
    printInfo();

    // Otherwise return the reason why we stopped
    return exitReason;
  }

  void saveBestStateInformation()
  {
    // Making sure the main thread is not currently writing
    _updateIntermediateResultMutex.lock();

    // Saving best solution and state
    std::string timeSuffix = std::string(".") + std::to_string(_jobId);

    // Saving files with standard name
    jaffarCommon::file::saveStringToFile(_bestSolutionStorage, _saveIntermediateBestSolutionPath);
    jaffarCommon::file::saveStringToFile(_bestStateStorage, _saveIntermediateBestStatePath);

    // Saving files with a time suffix
    jaffarCommon::file::saveStringToFile(_bestSolutionStorage, _saveIntermediateBestSolutionPath + timeSuffix);
    jaffarCommon::file::saveStringToFile(_bestStateStorage, _saveIntermediateBestStatePath + timeSuffix);

    // Making sure the main thread is not currently writing
    _updateIntermediateResultMutex.unlock();
  }

  void saveWorstStateInformation()
  {
    // Making sure the main thread is not currently writing
    _updateIntermediateResultMutex.lock();

    // Saving best solution and state
    std::string timeSuffix = std::string(".") + std::to_string(_jobId);

    // Saving best solution and state
    jaffarCommon::file::saveStringToFile(_worstSolutionStorage, _saveIntermediateWorstSolutionPath);
    jaffarCommon::file::saveStringToFile(_worstStateStorage, _saveIntermediateWorstStatePath);

    // Saving best solution and state
    jaffarCommon::file::saveStringToFile(_worstSolutionStorage, _saveIntermediateWorstSolutionPath + timeSuffix);
    jaffarCommon::file::saveStringToFile(_worstStateStorage, _saveIntermediateWorstStatePath + timeSuffix);

    // Making sure the main thread is not currently writing
    _updateIntermediateResultMutex.unlock();
  }

  void intermediateResultSaveLoop()
  {
    // Timer for saving to file
    auto lastSaveTime = jaffarCommon::timing::now();

    // Run loop while the driver is still running
    while (_hasFinished == false)
    {
      // Sleeping for 100ms intervals to prevent excessive overheads
      usleep(100000);

      // Getting time elapsed since last save
      auto currentTime              = jaffarCommon::timing::now();
      auto timeElapsedSinceLastSave = jaffarCommon::timing::timeDeltaSeconds(currentTime, lastSaveTime);

      // Checking if we need to save best state
      if (timeElapsedSinceLastSave > _saveIntermediateFrequency && _currentStep > 1)
      {
        // Saving worst and best state information
        saveBestStateInformation();
        saveWorstStateInformation();

        // Resetting timer
        lastSaveTime = jaffarCommon::timing::now();
      }
    }
  }

  void updateWorstState()
  {
    // If no states in database, there is nothing to update
    if (_engine->getStateDb()->getStateCount() == 0) return;

    // Making sure the intermediate result thread is not currently reading
    _updateIntermediateResultMutex.lock();

    // Getting worst state so far
    auto worstState = _engine->getStateDb()->getWorstState();

    // Saving worst state into the storage
    memcpy(_worstStateStorage.data(), worstState, _stateSize);

    // Loading worst state state into runner
    _engine->getStateDb()->loadStateIntoRunner(*_runner, _worstStateStorage.data());

    // Saving worst solution into storage
    _worstSolutionStorage = _runner->getInputHistoryString();

    // Updating worst state reward
    _worstStateReward = _runner->getGame()->getReward();

    // Making sure the intermediate result thread is not currently reading
    _updateIntermediateResultMutex.unlock();
  }

  void updateBestState()
  {
    // If no states in database and no win states, there is nothing to update
    if (_engine->getStateDb()->getStateCount() == 0 && _winStatesFound == 0) return;

    // Making sure the intermediate result thread is not currently reading
    _updateIntermediateResultMutex.lock();

    // If we haven't found any winning state, simply use the currently best state
    if (_winStatesFound == 0)
    {
      // Getting best state so far
      auto bestState = _engine->getStateDb()->getBestState();

      // Saving best state into the storage
      if (bestState != nullptr) memcpy(_bestStateStorage.data(), bestState, _stateSize);
    }

    // If we have found a winning state in this step that improves on the current best, save it now
    if (_engine->getWinStatesFound() > 0)
    {
      // Getting best win state (best reward) for the current step
      auto winStateEntry = _engine->getStepBestWinState();

      // If the reward if better than the current best, then make it the new best state
      if (winStateEntry.reward > _bestWinStateReward)
      {
        // Saving new best
        _bestWinStateReward = winStateEntry.reward;

        // Saving win state into the storage
        memcpy(_bestStateStorage.data(), winStateEntry.stateData, _stateSize);
      }
    }

    // Loading best state state into runner
    _engine->getStateDb()->loadStateIntoRunner(*_runner, _bestStateStorage.data());

    // Updating best state reward
    _bestStateReward = _runner->getGame()->getReward();

    // Storing best solution
    _bestSolutionStorage = _runner->getInputHistoryString();

    // Making sure the intermediate result thread is not currently reading
    _updateIntermediateResultMutex.unlock();
  }

  // Function to show current state of execution
  void printInfo()
  {
    // If using ncurses, clear terminal before printing the information for this step
    jaffarCommon::logger::clearTerminal();

    // Printing information
    jaffarCommon::logger::log("[J+] Job Id:                                      %lu\n", _jobId);
    jaffarCommon::logger::log("[J+] Script File:                                 '%s'\n", _configFilePath.c_str());
    jaffarCommon::logger::log("[J+] Emulator Name:                               '%s'\n", _runner->getGame()->getEmulator()->getName().c_str());
    jaffarCommon::logger::log("[J+] Game Name:                                   '%s'\n", _runner->getGame()->getName().c_str());
    jaffarCommon::logger::log("[J+] Current Step #:                              %lu", _currentStep);
    if (_maxSteps > 0) jaffarCommon::logger::log(" (Max: %lu)", _maxSteps);
    jaffarCommon::logger::log("\n");

    if (_winStatesFound == 0)
      jaffarCommon::logger::log("[J+] Current Reward (Best / Worst):               %.6f / %.6f (Diff: %.6f)\n", _bestStateReward, _worstStateReward,
                                _bestStateReward - _worstStateReward);

    if (_winStatesFound > 0) jaffarCommon::logger::log("[J+] Best Win State Reward:                       %.3f\n", _bestStateReward);

    // Printing engine information
    jaffarCommon::logger::log("[J+] Engine Information: \n");
    _engine->printInfo();

    // Loading best state into runner
    _engine->getStateDb()->loadStateIntoRunner(*_runner, _bestStateStorage.data());

    // Printing best state information to screen
    jaffarCommon::logger::log("[J+] Runner Information (Best State): \n");
    _runner->printInfo();
    jaffarCommon::logger::log("[J+] Game Information (Best State): \n");
    _runner->getGame()->printInfo();
    jaffarCommon::logger::log("[J+] Emulator Information (Best State): \n");
    _runner->getGame()->getEmulator()->printInfo();

    // Division rule to separate different steps
    jaffarCommon::logger::log("[J+] --------------------------------------------------------------\n");

    // If using ncurses, refresh terminal now
    jaffarCommon::logger::refreshTerminal();
  }

  // Function to obtain driver based on configuration
  static std::unique_ptr<Driver> getDriver(const std::string& configFilePath, const nlohmann::json& config)
  {
    // Creating new engine
    auto d = std::make_unique<Driver>(configFilePath, config);

    // Returning engine
    return d;
  }

  // Function to get the last step
  size_t getCurrentStep() { return _currentStep; }

private:
  // Remember path to config file for reference
  const std::string _configFilePath;

  // Pointer to the internal Jaffar engine
  std::unique_ptr<Engine> _engine;

  // Pointer to runner to use for printing information and saving partial results
  std::unique_ptr<Runner> _runner;

  // Job identifier -- to have a way for distinguishing intermediate values between different jobs
  size_t _jobId;

  // Getting maximum number of steps (zero = not established)
  size_t _maxSteps;

  // Counter for the number of steps performed. The initialization with the first state counts as step zero
  size_t _currentStep;

  // Flag to decide whether to end Jaffar on the first win state found
  bool _endOnFirstWinState;

  // The total number of win states found so far
  size_t _winStatesFound;

  // Reward for the best (win) state found to far
  float _bestWinStateReward;

  // Reward for the best (win or otherwise) state found to far
  float _bestStateReward;

  // Reward for the best (win or otherwise) state found to far
  float _worstStateReward;

  // Storage for the current best (win or otherwise) state
  std::string _bestStateStorage;

  // Storage for the current worst (win or otherwise) state
  std::string _worstStateStorage;

  // Storage for the current best (win or otherwise) state
  std::string _bestSolutionStorage;

  // Storage for the current worst (win or otherwise) state
  std::string _worstSolutionStorage;

  // Storage size of a runner state
  size_t _stateSize;

  // Internal flag to indicate the driver has finished
  __volatile__ bool _hasFinished;

  /////////////// Intermediate Result Storage

  // Whether to store intermediate results at all
  bool _saveIntermediateResultsEnabled;

  // Maximum frequency by which to save intermediate results
  float _saveIntermediateFrequency;

  // Path to store the best solution found so far
  std::string _saveIntermediateBestSolutionPath;

  // Path to store the worst solution found so far
  std::string _saveIntermediateWorstSolutionPath;

  // Path to store the best state found so far
  std::string _saveIntermediateBestStatePath;

  // Path to store the worst solution found so far
  std::string _saveIntermediateWorstStatePath;

  // Update intermediate result mutex
  std::mutex _updateIntermediateResultMutex;
};

} // namespace jaffarPlus