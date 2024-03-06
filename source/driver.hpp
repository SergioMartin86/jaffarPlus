#pragma once

#include <limits>
#include "game.hpp"
#include "runner.hpp"
#include "engine.hpp"

namespace jaffarPlus
{

class Driver final
{
 public:

  // Base constructor
  Driver(const nlohmann::json& config) 
  {
   // Creating runner from the configuration
   _runner = jaffarPlus::Runner::getRunner(
    jaffarCommon::json::getObject(config, "Emulator Configuration"),
    jaffarCommon::json::getObject(config, "Game Configuration"),
    jaffarCommon::json::getObject(config, "Runner Configuration"));

   // Creating engine from the configuration
   _engine = jaffarPlus::Engine::getEngine(
    jaffarCommon::json::getObject(config, "Emulator Configuration"),
    jaffarCommon::json::getObject(config, "Game Configuration"),
    jaffarCommon::json::getObject(config, "Runner Configuration"),
    jaffarCommon::json::getObject(config, "Engine Configuration"));

   // Getting maximum number of steps (zero is not established)
   _maxSteps = _engine->getMaximumStep();

   // Getting driver configuration
   const auto& driverConfig = jaffarCommon::json::getObject(config, "Driver Configuration");

   // Getting end win delay config
   _endOnFirstWinState = jaffarCommon::json::getBoolean(driverConfig, "End On First Win State");

   // Allocating space for the current best and worst states
   _stateSize = _runner->getStateSize();
   _bestStateStorage  = (uint8_t*) malloc (_stateSize);
   _worstStateStorage = (uint8_t*) malloc (_stateSize);
  }

  ~Driver()
  {
    free(_bestStateStorage);
    free(_worstStateStorage);
  }

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
  }

  // Start running engine loop
  int run()
  {
   // If using ncurses, initialize terminal now
  jaffarCommon::logger::initializeTerminal();

   // Showing initial state's information
   dumpInformation();

   // Storage to print the exit reason
   std::string exitReason;
   
   // Running engine until a termination point
   while(true)
   {
      // If found winning state, report it now
      if (_endOnFirstWinState && _engine->getWinStates().size() > 0)  { exitReason = "Exiting on first solution found"; break; }

      // If ran out of states, finish now
      if (_engine->getStateCount() == 0) { exitReason = "Engine ran out of states"; break; }

      // If maximum step established and reached, finish now
      if (_maxSteps > 0 && _currentStep >= _maxSteps) { exitReason = "Maximum step count reached."; break; }

      // Running engine step 
      _engine->runStep();

      // Summing amount of win states found
      _winStatesFound += _engine->getWinStates().size();

      // Increasing step counter
      _currentStep++;

      // Printing information and dumping information and files
      dumpInformation();
   }

   // If using ncurses, terminate terminal now
   jaffarCommon::logger::finalizeTerminal();

   // Final report
   dumpInformation();

   // Printing exit reason
   jaffarCommon::logger::log("[J++] Step %lu - Exit Reason: %s\n", _currentStep, exitReason.c_str());

   // Exit code depends on if win state was found
   if (_winStatesFound == 0) return -1;
   return 0;
  }

  void dumpInformation()
  {
    // If using ncurses, clear terminal before printing the information for this step
    jaffarCommon::logger::clearTerminal();

    // Updating best and worst states
    updateBestState();
    updateWorstState();

    // Showing current state of execution and dumping best states
    printInfo();
    
    // Dump Best State
    _engine->getStateDb()->loadStateIntoRunner(*_runner, _bestStateStorage);
    _runner->dumpInputHistoryToFile("jaffar.best.sol");

    // Dump Worst State
    _engine->getStateDb()->loadStateIntoRunner(*_runner, _worstStateStorage);
    _runner->dumpInputHistoryToFile("jaffar.worst.sol");

    // If using ncurses, refresh terminal now
    jaffarCommon::logger::refreshTerminal();
  }

  void updateWorstState()
  {
    // Getting worst state so far
    auto worstState = _engine->getStateDb()->getWorstState();

    // Saving worst state into the storage
    memcpy(_worstStateStorage, worstState, _stateSize);

    // Loading worst state state into runner
    _engine->getStateDb()->loadStateIntoRunner(*_runner, _worstStateStorage);

    // Updating worst state reward
    _worstStateReward = _runner->getGame()->getReward();
  }

  void updateBestState()
  {
    // If we haven't found any winning state, simply use the currently best state
    if (_winStatesFound == 0)
    {
      // Getting best state so far
      auto bestState = _engine->getStateDb()->getBestState();

      // Saving best state into the storage
      memcpy(_bestStateStorage, bestState, _stateSize);

      // Loading best state state into runner
      _engine->getStateDb()->loadStateIntoRunner(*_runner, _bestStateStorage);

      // Updating best state reward
      _bestStateReward = _runner->getGame()->getReward();
    }

    // If we have found a winning state in this step that improves on the current best, save it now
    if (_engine->getWinStates().size() > 0)
    {
      // Getting best win state (best reward) for the current step
      auto winStateEntry = _engine->getWinStates().begin();
    
      // Getting the reward of this step's winning state reward
      auto winStateReward = winStateEntry->first;

      // If the reward if better than the current best, then make it the new best state
      if (winStateReward > _bestWinStateReward)
      {
        // Saving new best
        _bestWinStateReward = winStateReward;

        // Getting new best win state's pointer
        auto bestWinStatePtr = winStateEntry->second;
        
        // Saving win state into the storage
        memcpy(_bestStateStorage, bestWinStatePtr, _stateSize);
      }
    }
  }

  // Function to show current state of execution
  void printInfo()
  {
    // Printing information
    jaffarCommon::logger::log("[J++] Emulator Name:                               '%s'\n", _runner->getGame()->getEmulator()->getName().c_str());
    jaffarCommon::logger::log("[J++] Game Name:                                   '%s'\n", _runner->getGame()->getName().c_str());
    jaffarCommon::logger::log("[J++] Current Step #:                              %lu (Max: %lu)\n", _currentStep, _maxSteps);

    if (_winStatesFound == 0)
    jaffarCommon::logger::log("[J++] Current Reward (Best / Worst):               %.3f / %.3f (Diff: %.3f)\n", _bestStateReward, _worstStateReward, _bestStateReward - _worstStateReward);
    
    if (_winStatesFound > 0)
    {
    jaffarCommon::logger::log("[J++] Best Win State Reward:                       %.3f\n", _bestStateReward);
    jaffarCommon::logger::log("[J++] Win States Found:                            %lu\n", _winStatesFound);
    }

    // Printing engine information
    jaffarCommon::logger::log("[J++] Engine Information: \n");
    _engine->printInfo();

    // Loading best state into runner
    _engine->getStateDb()->loadStateIntoRunner(*_runner, _bestStateStorage);

   // Printing best state information to screen
   jaffarCommon::logger::log("[J++] Runner Information (Best State): \n");
   _runner->printInfo();
   jaffarCommon::logger::log("[J++] Game Information (Best State): \n");
   _runner->getGame()->printInfo();
   jaffarCommon::logger::log("[J++] Emulator Information (Best State): \n");
   _runner->getGame()->getEmulator()->printInfo(); 

   // Division rule to separate different steps
   jaffarCommon::logger::log("[J++] --------------------------------------------------------------\n");
  }

  // Function to obtain driver based on configuration
  static std::unique_ptr<Driver> getDriver(const nlohmann::json& config)
  {
      // Creating new engine
      auto d = std::make_unique<Driver>(config);

      // Initializing engine
      d->initialize();

      // Returning engine
      return d;
  }

  private:

  // Pointer to the internal Jaffar engine
  std::unique_ptr<Engine> _engine;

  // Pointer to runner to use for printing information and saving partial results
  std::unique_ptr<Runner> _runner;

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
  uint8_t* _bestStateStorage;

  // Storage for the current worst (win or otherwise) state
  uint8_t* _worstStateStorage;

  // Storage size of a runner state
  size_t _stateSize;
};

} // namespace jaffarPlus