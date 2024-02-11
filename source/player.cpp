#include <chrono>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/extern/argparse/argparse.hpp>
#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/string.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "game.hpp"
#include "emulator.hpp"
#include "runner.hpp"
#include "playback.hpp"

int main(int argc, char *argv[])
{
 // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  program.add_argument("solutionFile")
    .help("path to the solution sequence file (.sol) to reproduce.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Parsing config file
  const std::string configFile = program.get<std::string>("configFile");
  std::string configFileString;
  if (jaffarCommon::loadStringFromFile(configFileString, configFile) == false) 
    EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  // If sequence file defined, load it and play it
  const std::string solutionFile = program.get<std::string>("solutionFile");
  std::string solutionFileString;
  if (jaffarCommon::loadStringFromFile(solutionFileString, solutionFile) == false) 
    EXIT_WITH_ERROR("[ERROR] Could not find or read from solution sequence file: %s\n%s \n", solutionFile.c_str(), program.help().str().c_str());

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

  // Getting emulator from its name and configuring it
  auto e = jaffarPlus::Emulator::getEmulator(emulatorName, JSON_GET_OBJECT(config, "Emulator Configuration"));

  // Initializing emulator
  e->initialize();

  // If initial state file defined, load it
  if (initialStateFilePath.empty() == false) e->loadStateFile(initialStateFilePath);

  // Getting game from its name and configuring it
  auto g = jaffarPlus::Game::getGame(gameName, e, JSON_GET_OBJECT(config, "Game Configuration"));

  // Initializing game
  g->initialize();

  // Parsing script rules
  g->parseRules(JSON_GET_ARRAY(config, "Rules"));

  // Creating runner from game instance
  jaffarPlus::Runner r(g, JSON_GET_OBJECT(config, "Runner Configuration"));

  // Parsing Possible game inputs
  r.parseGameInputs(JSON_GET_ARRAY(config, "Game Inputs"));

  // Getting game state size
  const auto stateSize = r.getStateSize();

  // Getting input sequence
  const auto solutionSequence = jaffarCommon::split(solutionFileString, ' ');

  // Getting sequence length
  const auto sequenceLength = solutionSequence.size();

  // Initializing terminal
  jaffarCommon::initializeTerminal();

  // Printing information
  LOG("[J+] Emulator Name:                   '%s'\n", emulatorName.c_str());
  LOG("[J+] Config File:                     '%s'\n", configFile.c_str());
  LOG("[J+] Solution File:                   '%s'\n", solutionFile.c_str());
  LOG("[J+] State Size:                       %lu\n", stateSize);
  LOG("[J+] Sequence Length:                  %lu\n", sequenceLength);
  LOG("[J+] ********** Creating Playback Sequence **********\n");

  // Instantiating playback object
  jaffarPlus::Playback p(r, solutionSequence);

  // Ending ncurses window
  jaffarCommon::finalizeTerminal();
}
