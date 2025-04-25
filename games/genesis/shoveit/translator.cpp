#include <chrono>
#include <cstdlib>
#include <argparse/argparse.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/string.hpp>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/timing.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/parallel.hpp>
#include <gameList.hpp>
#include <emulatorList.hpp>
#include "emulator.hpp"
#include "game.hpp"
#include "runner.hpp"
#include <random>
#include <math.h> 
#include <thread>

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile").help("path to the Jaffar configuration script (.jaffar) file to run.").required();
  program.add_argument("InputFile").help("SOK-formatted solution to translate").required();

  // Try to parse arguments
  try
    {
      program.parse_args(argc, argv);
    }
  catch (const std::runtime_error &err)
    {
      JAFFAR_THROW_LOGIC("%s\n%s", err.what(), program.help().str().c_str());
    }

  // Parsing config file
  const std::string configFile = program.get<std::string>("configFile");

  // If config file defined, read it now
  std::string configFileString;
  if (jaffarCommon::file::loadStringFromFile(configFileString, configFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from Jaffar config file: %s\n", configFile.c_str());

  // Parsing configuration file
  nlohmann::json config;
  try
    {
      config = nlohmann::json::parse(configFileString);
    }
  catch (const std::exception &err)
    {
      JAFFAR_THROW_LOGIC("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what());
    }

  // Getting input file
  const std::string inputFile = program.get<std::string>("InputFile");

  // If input file defined, read it now
  std::string inputFileString;
  if (jaffarCommon::file::loadStringFromFile(inputFileString, inputFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from input file: %s\n", inputFile.c_str());

  // // Getting component configurations
  // auto emulatorConfig = jaffarCommon::json::getObject(config, "Emulator Configuration");
  // auto gameConfig     = jaffarCommon::json::getObject(config, "Game Configuration");
  // auto runnerConfig   = jaffarCommon::json::getObject(config, "Runner Configuration");

  // // Getting number of threads
  // auto threadCount = jaffarCommon::parallel::getMaxThreadCount();

  // // Sanity check
  // if (threadCount == 0) JAFFAR_THROW_LOGIC("The number of worker threads must be at least one. Provided: %lu\n", threadCount);

  // // Collection of runners for the workers to use
  // std::vector<std::unique_ptr<jaffarPlus::Runner>> _runners;

  // // Creating runner from the configuration
  // auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);

  // // Initializing runner
  // r->initialize();

  // // Getting internal emulator
  // auto e = dynamic_cast<jaffarPlus::emulator::QuickerGPGX *>(r->getGame()->getEmulator());

  // // Disabling rendering
  // e->disableRendering();

  // // Getting game pointer
  // auto g = dynamic_cast<jaffarPlus::games::genesis::ShoveIt *>(r->getGame());

  // Executing commands
  for (size_t i = 0; i < inputFileString.size(); i++)
  {
    // // Advance while the player is still moving
    // while (g->getMoveTimer() != 0)
    // {
    //   printf("|..|........|\n");
    //   g->advanceState(g->_nullInputIdx);
    // } 

    // // Run the required input
    // if (inputFileString[i] == 'l' || inputFileString[i] == 'L')
    // {
    //   printf("|..|..L.....|\n");
    //   g->advanceState(g->_leftInputIdx);
    // }
    // if (inputFileString[i] == 'r' || inputFileString[i] == 'R')
    // {
    //   printf("|..|...R....|\n");
    //   g->advanceState(g->_rightInputIdx);
    // }
    // if (inputFileString[i] == 'u' || inputFileString[i] == 'U')
    // {
    //   printf("|..|U.......|\n");
    //   g->advanceState(g->_upInputIdx);
    // }
    // if (inputFileString[i] == 'd' || inputFileString[i] == 'D')
    // {
    //   printf("|..|.D......|\n");
    //   g->advanceState(g->_downInputIdx);
    // }

    // // Wait until the player starts moving
    // while (g->getMoveTimer() == 0)
    // {
    //   printf("|..|........|\n");
    //   g->advanceState(g->_nullInputIdx);
    // } 

    // Run the required input
    if (inputFileString[i] == 'l' || inputFileString[i] == 'L') printf("|..|..L.....|\n");
    if (inputFileString[i] == 'r' || inputFileString[i] == 'R') printf("|..|...R....|\n");
    if (inputFileString[i] == 'u' || inputFileString[i] == 'U') printf("|..|U.......|\n");
    if (inputFileString[i] == 'd' || inputFileString[i] == 'D') printf("|..|.D......|\n");
    if (i == 0) printf("|..|........|\n");
    if (i == 0) printf("|..|........|\n");
    if (i == 0) printf("|..|........|\n");
    if (i == 0) printf("|..|........|\n");
    for (size_t j = 0; j < 15; j++) printf("|..|........|\n");


    // printf("%c\n", inputFileString[i]);
  }

  // _nullInputIdx   = _emulator->registerInput("|..|........|");
  // _leftInputIdx   = _emulator->registerInput("|..|..L.....|");
  // _rightInputIdx  = _emulator->registerInput("|..|...R....|");
  // _upInputIdx     = _emulator->registerInput("|..|U.......|");
  // _downInputIdx   = _emulator->registerInput("|..|.D......|");
  // printf("getMoveTimer: %u\n", g->getMoveTimer());

}
