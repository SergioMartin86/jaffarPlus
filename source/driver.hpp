#pragma once

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
    JSON_GET_OBJECT(config, "Emulator Configuration"),
    JSON_GET_OBJECT(config, "Game Configuration"),
    JSON_GET_OBJECT(config, "Runner Configuration"));

   // Creating engine from the configuration
   _engine = jaffarPlus::Engine::getEngine(
    JSON_GET_OBJECT(config, "Emulator Configuration"),
    JSON_GET_OBJECT(config, "Game Configuration"),
    JSON_GET_OBJECT(config, "Runner Configuration"),
    JSON_GET_OBJECT(config, "Engine Configuration"));

   // Getting maximum number of steps (zero is not established)
   _maxSteps = _engine->getMaximumStep();
  }

  // Resets the execution back to the starting point
  void initialize()
  {
   // Resetting step counter
   _currentStep = 0;
  }

  // Start running engine loop
  void run()
  {
   // Running engine until a termination point
   while(true)
   {
      // If found winning state, report it now
      if (_engine->getWinStates().size() > 0)  { LOG("[J++] Success: A solution was found in %lu steps.\n", _currentStep); exit(0); }

      // If ran out of states, finish now
      if (_engine->getStateCount() == 0) { LOG("[J++] Fail: engine ran out of states at step %lu without reaching a solution.\n", _currentStep); exit(-1); }

      // If maximum step established and reached, finish now
      if (_maxSteps > 0 && _currentStep >= _maxSteps) { LOG("[J++] Fail: Maximum step count (%lu) reached without a solution.\n", _currentStep); exit(-1); }

      // Getting best state so far
      auto bestState = _engine->getStateDb()->getBestState();

      // Loading best state into runner
      _engine->getStateDb()->loadStateIntoRunner(*_runner, bestState);

      // Dumping best solution so far into a file
      _runner->dumpInputHistoryToFile("jaffar.best.sol");

      // Printing information
      LOG("[J++] Emulator Name:                               '%s'\n", _runner->getGame()->getEmulator()->getName().c_str());
      LOG("[J++] Game Name:                                   '%s'\n", _runner->getGame()->getName().c_str());
      LOG("[J++] Current Step #:                              %lu (Max: %lu)\n", _currentStep, _maxSteps);

      // Printing engine information
      LOG("[J++] Engine Information: \n");
      _engine->printInfo();
      LOG("[J++] Runner Information (Best State): \n");
      _runner->printInfo();
      LOG("[J++] Game Information (Best State): \n");
      _runner->getGame()->printInfo();
      LOG("[J++] Emulator Information (Best State): \n");
      _runner->getGame()->getEmulator()->printInfo();

      // Division rule to separate different steps
      LOG("[J++] --------------------------------------------------------------\n");

      // Increasing step counter
      _currentStep++;

      // Running engine step 
      _engine->runStep();
   }
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
};

} // namespace jaffarPlus