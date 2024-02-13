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

  // Parsing state compression configuration
  const auto& stateCompressionJs = JSON_GET_OBJECT(config, "State Compression");
  const auto useDifferentialCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Differential Compression");
  const auto maximumDifferences = JSON_GET_NUMBER(size_t, stateCompressionJs, "Max Difference (bytes)");
  const auto useZlibCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Zlib Compression");

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
  jaffarPlus::Runner r(g, JSON_GET_OBJECT(config, "Runner Configuration"));

  // Parsing Possible game inputs
  r.parseGameInputs(JSON_GET_ARRAY(config, "Game Inputs"));

  // Getting game state size
  const auto stateSize = r.getStateSize();

  // Getting differential state size
  size_t differentialStateSize = r.getDifferentialStateSize(maximumDifferences);

  // Calculating state storage size
  size_t stateStorageSize = useDifferentialCompression ? differentialStateSize : stateSize;

  // Creating State database
  jaffarPlus::StateDb stateDb(stateStorageSize, JSON_GET_OBJECT(config, "State Database"));

  // Printing information
  LOG("[J+] Emulator Name:                   '%s'\n", emulatorName.c_str());
  LOG("[J+] Config File:                     '%s'\n", configFile.c_str());
  LOG("[J+] State Size:                       %lu\n", stateSize);
  LOG("[J+] Use Differential Compression:     %s\n", useDifferentialCompression ? "true" : "false");
  if (useDifferentialCompression == true)
  {
  LOG("[J+]  + Using Zlib Compression:        %s\n", useZlibCompression ? "true" : "false");
  LOG("[J+]  + Differential State Size:       %lu\n", differentialStateSize);
  } 

  // Printing State Db information
  stateDb.printInfo(); 
}
