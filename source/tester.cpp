#include <chrono>
#include <argparse/argparse.hpp>
#include "emulators/emulator.hpp"
#include "common/logger.hpp"
#include "common/json.hpp"

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
  std::string configFile = program.get<std::string>("configFile");
  std::string configFileString;
  if (jaffarPlus::loadStringFromFile(configFileString, configFile) == false) 
    EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  // If sequence file defined, load it and play it
  std::string solutionFile = program.get<std::string>("solutionFile");
  std::string solutionFileString;
  if (jaffarPlus::loadStringFromFile(solutionFileString, solutionFile) == false) 
    EXIT_WITH_ERROR("[ERROR] Could not find or read from solution sequence file: %s\n%s \n", solutionFile.c_str(), program.help().str().c_str());

  // Parsing configuration file
  nlohmann::json config;
  try { config = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // Getting initial state file from the configuration
  auto initialStateFilePath = JSON_GET_STRING(config, "Initial State File Path");

  // Getting emulator name from the configuration
  auto emulatorName = JSON_GET_STRING(config, "Emulator");

  // Getting emulator from its name and configuring it
  auto e = jaffarPlus::Emulator::getEmulator(emulatorName, JSON_GET_OBJECT(config, "Emulator Configuration"));

  // Initializing emulator
  e->initialize();

  // If initial state file defined, load it
  if (initialStateFilePath.empty() == false) e->loadStateFile(initialStateFilePath);

  // Getting input sequence
  auto solutionSequence = jaffarPlus::split(solutionFileString, ' ');

  // Getting sequence length
  auto sequenceLength = solutionSequence.size();

  // Printing information
  LOG("[J+] Emulator Name:          '%s'\n", e->getName().c_str());
  LOG("[J+] Config File:            '%s'\n", configFile.c_str());
  LOG("[J+] Solution File:          '%s'\n", solutionFile.c_str());
  LOG("[J+] Sequence Length:         %lu\n", sequenceLength);
  LOG("[J+] Running...\n");
  
  // Playing input sequence
  auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& input : solutionSequence) e->advanceState(input);
  auto tf = std::chrono::high_resolution_clock::now();

  // Calculating running time
  auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
  double elapsedTimeSeconds = (double)dt * 1.0e-9;

  // Printing time information
  LOG("[J+] Elapsed time:            %3.3fs\n", (double)dt * 1.0e-9);
  LOG("[J+] Performance:             %.3f inputs / s\n", (double)sequenceLength / elapsedTimeSeconds);

  // Printing debug information
  e->printDebugInformation();
}
