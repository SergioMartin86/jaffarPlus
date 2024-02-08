#include <chrono>
#include <jaffarCommon/extern/argparse/argparse.hpp>
#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/string.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "game.hpp"
#include "emulator.hpp"
#include "runner.hpp"

int main(int argc, char *argv[])
{
 // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  program.add_argument("--cycleType")
    .help("Specifies the emulation actions to be performed per each input. Possible values: 'Simple': performs only advance state, 'Rerecord': performs load/advance/save, and 'Full': performs load/advance/save/advance.")
    .default_value(std::string("Simple"));

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

  // Getting reproduce flag
  const std::string cycleType = program.get<std::string>("--cycleType");
 
  // Parsing cycleType
  bool recognizedCycleType = false;
  if (cycleType == "Simple") recognizedCycleType = true;
  if (cycleType == "Rerecord") recognizedCycleType = true;
  if (cycleType == "Full") recognizedCycleType = true;
  if (recognizedCycleType == false) EXIT_WITH_ERROR("[ERROR] Unrecognized cycle type '%s'");
 
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

  // Getting differential state size
  size_t differentialStateSize = r.getDifferentialStateSize(maximumDifferences);

  // Printing information
  LOG("[J+] Emulator Name:                   '%s'\n", emulatorName.c_str());
  LOG("[J+] Config File:                     '%s'\n", configFile.c_str());
  LOG("[J+] Solution File:                   '%s'\n", solutionFile.c_str());
  LOG("[J+] Cycle Type:                      '%s'\n", cycleType.c_str());
  LOG("[J+] State Size:                       %lu\n", stateSize);
  LOG("[J+] Use Differential Compression:     %s\n", useDifferentialCompression ? "true" : "false");
  if (useDifferentialCompression == true)
  {
  LOG("[J+]  + Using Zlib Compression:        %s\n", useZlibCompression ? "true" : "false");
  LOG("[J+]  + Differential State Size:       %lu\n", differentialStateSize);
  } 
  LOG("[J+] Sequence Length:                  %lu\n", sequenceLength);
  LOG("[J+] ********** Running Test **********\n");

  // Serializing initial state
  auto currentState = (uint8_t *)malloc(stateSize);
  r.serializeState(currentState);

  // Storage for diffential compression
  auto diffentialStateData = (uint8_t *)malloc(differentialStateSize);
  size_t differentialDataPos = 0;
  size_t referenceDataPos = 0;
  r.serializeDifferentialState(diffentialStateData, &differentialDataPos, differentialStateSize, currentState, &referenceDataPos, stateSize, useZlibCompression);
  size_t maxDifferentialStateSize = differentialDataPos;

  // Check whether to perform each action
  const bool doPreAdvance = cycleType == "Full";
  const bool doDeserialize = cycleType == "Rerecord" || cycleType == "Full";
  const bool doSerialize = cycleType == "Rerecord" || cycleType == "Full";

  // Actually running the sequence
  auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& input : solutionSequence)
  {
    if (doPreAdvance == true) r.advanceState(input);

    if (doDeserialize == true)
    {
      if (useDifferentialCompression == false) r.deserializeState(currentState);

      if (useDifferentialCompression == true) 
      {
        size_t differentialDataPos = 0;
        size_t referenceDataPos = 0;
        r.deserializeDifferentialState(diffentialStateData, &differentialDataPos, differentialStateSize, currentState, &referenceDataPos, stateSize, useZlibCompression);
      }
    } 

    r.advanceState(input);

    if (doSerialize == true)
    {
      if (useDifferentialCompression == false) r.serializeState(currentState);

      if (useDifferentialCompression == true)
      {
        size_t differentialDataPos = 0;
        size_t referenceDataPos = 0;
        r.serializeDifferentialState(diffentialStateData, &differentialDataPos, differentialStateSize, currentState, &referenceDataPos, stateSize, useZlibCompression);
        maxDifferentialStateSize = std::max(maxDifferentialStateSize, differentialDataPos);
      } 
    } 
  }
  auto tf = std::chrono::high_resolution_clock::now();

  // Calculating running time
  auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
  double elapsedTimeSeconds = (double)dt * 1.0e-9;

  // Printing time information
  LOG("[J+] Elapsed time:                  %3.3fs\n", (double)dt * 1.0e-9);
  LOG("[J+] Performance:                   %.3f inputs / s\n", (double)sequenceLength / elapsedTimeSeconds);
  if (useDifferentialCompression == true)
  {
  LOG("[J+] Max Differential State Size:   %lu\n", maxDifferentialStateSize);
  } 

  // Printing Information
  LOG("[J+] Runner Information:\n");
  r.printInfo();

  LOG("[J+] Emulator Information:\n");
  r.getGame()->getEmulator()->printInfo();

  LOG("[J+] Game Information:\n");
  r.getGame()->printInfo();
}
