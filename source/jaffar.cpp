#include <chrono>
#include <omp.h>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/extern/argparse/argparse.hpp>
#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/string.hpp>
#include <jaffarCommon/include/timing.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "game.hpp"
#include "emulator.hpp"
#include "runner.hpp"
#include "stateDb.hpp"

int main(int argc, char *argv[])
{
 // Parsing command line arguments
  argparse::ArgumentParser program("jaffar", "1.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Parsing config file
  const std::string configFile = program.get<std::string>("configFile");
  std::string configFileString;
  if (jaffarCommon::loadStringFromFile(configFileString, configFile) == false) 
    EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  // Parsing configuration file
  nlohmann::json config;
  try { config = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // Getting initial state file from the configuration
  const auto initialStateFilePath = JSON_GET_STRING(config, "Initial State File Path");

  // Getting emulator name from the configuration
  const auto emulatorName = JSON_GET_STRING(config, "Emulator");

  // Getting game name from the configuration
  const auto gameName = JSON_GET_STRING(config, "Game");

  // Getting number of threads
  size_t threadCount = omp_get_max_threads();

  // Creating storage for the runnners (one per thread)
  std::vector<std::unique_ptr<jaffarPlus::Runner>> runners(threadCount);

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
    runners[threadId] = std::move(r);
  }

  // Grabbing a runner to do continue initialization
  auto& r = *runners[0];

  // Creating State database
  jaffarPlus::StateDb stateDb(r, JSON_GET_OBJECT(config, "State Database"));

  // Getting memory for the reference state
  const auto stateSize = r.getStateSize();

  // Allocating memory
  auto referenceData = malloc(stateSize);
  
  // Serializing the initial state without compression to use as reference
  jaffarCommon::serializer::Contiguous s(referenceData, stateSize);
  r.serializeState(s);

  // Setting reference data
  stateDb.setReferenceData(referenceData);

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

  // Pushing initial state
  stateDb.pushState(reward, r);

  LOG("States: %lu\n", stateDb.getNextStateDbSize());

  // Printing information
  LOG("[J+] Emulator Name:                    '%s'\n", emulatorName.c_str());
  LOG("[J+] Config File:                      '%s'\n", configFile.c_str());
  LOG("[J+] Thread Count:                     %lu\n", threadCount);

  // Printing State Db information
  LOG("[J+] State Database Info:\n");
  stateDb.printInfo(); 

  

}

