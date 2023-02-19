#include <unistd.h>
#include <stdlib.h>
#include <set>
#include "argparse.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include <vector>
#include <omp.h>
#include <atomic>

std::atomic<uint64_t> goodRNGFound;
std::atomic<uint64_t> badRNGFound;

// Functions for the show thread
void* progressThreadFunction(void *ptr)
{
 while(true)
 {
  sleep(1);
  uint64_t goodRNG = goodRNGFound;
  uint64_t totalRNG = goodRNG + badRNGFound;
  printf("[Progress] Ratio %lu:%lu (%f)\n", goodRNG, totalRNG, ((double)goodRNG/(double)totalRNG));
 }
 return NULL;
}

void tester(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "2.0");

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
  auto statusConfig = loadStringFromFile(configFileString, configFile.c_str());
  if (statusConfig == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  // If sequence file defined, load it
  std::string solutionFile = program.get<std::string>("solutionFile");

  nlohmann::json config;
  try { config = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // Checking whether it contains the rules field
  if (isDefined(config, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rules' key.\n");

  // Checking whether it contains the emulator configuration field
  if (isDefined(config, "Emulator Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Emulator Configuration' key.\n");

  // Checking whether it contains the Game configuration field
  if (isDefined(config, "Game Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Configuration' key.\n");

  // Getting number of openMP threads
  int _threadCount = omp_get_max_threads();

  // Creating game instances, one per openMP thread
  std::vector<GameInstance*> _gameInstances;
  std::vector<EmuInstance*> _emuInstances;

  // Resizing containers based on thread count
  _gameInstances.resize(_threadCount);
  _emuInstances.resize(_threadCount);

  // Initializing thread-specific instances
  #pragma omp parallel
  {
   // Getting thread id
   int threadId = omp_get_thread_num();

   // Doing this as a critical section so not all threads try to access files at the same time
   #pragma omp critical
   {
    // Creating game and emulator instances, and parsing rules
    _emuInstances[threadId] = new EmuInstance(config["Emulator Configuration"]);
    _gameInstances[threadId] = new GameInstance(_emuInstances[threadId], config["Game Configuration"]);
    _gameInstances[threadId]->parseRules(config["Rules"]);
   }
  }

  // Loading solution file
  std::string moveSequence;
  std::vector<std::string> moveList;

  auto statusSolution = loadStringFromFile(moveSequence, solutionFile.c_str());
  if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", solutionFile.c_str());

  // Getting move list
  moveList.clear();
  moveList = split(moveSequence, ' ');
  int sequenceLength = moveList.size();

  // Storing initial state
  uint8_t* state = (uint8_t*) malloc(_STATE_DATA_SIZE_TRAIN);
  _emuInstances[0]->serializeState(state);

  // Storing rule count
  size_t ruleCount = _gameInstances[0]->_rules.size();

  // Printing info
  printf("[Jaffar] Testing sequence file: %s (%u steps)\n", solutionFile.c_str(), sequenceLength);

  // Flag to indicate a seed was found
  bool seedFound = false;

  // Creating progress reporting thread
  pthread_t progressThread;
  if (pthread_create(&progressThread, NULL, progressThreadFunction, NULL) != 0) EXIT_WITH_ERROR("[ERROR] Could not create show thread.\n");

  // Resetting counters
  goodRNGFound = 0;
  badRNGFound = 0;

  // Running different RNGs
  #pragma omp parallel for
  for (uint64_t seed = 0; seed < 0xFFFFFFFF; seed++)
  {
   int threadId = omp_get_thread_num();

   // Reloading state
   _emuInstances[threadId]->deserializeState(state);

   // Setting RNG
   _gameInstances[threadId]->setRNGState(seed);

   // Storage for rules
   bool rulesStatus[ruleCount];

   // Storage for state type
   stateType_t stateType;

   // Running solution
   for (int i = 0; i < sequenceLength; i++)
   {
    // Running move
    _gameInstances[threadId]->advanceStateString(moveList[i]);

    // Checking rules
    memset(rulesStatus, 0, ruleCount * sizeof(bool));
    _gameInstances[threadId]->evaluateRules(rulesStatus);

    // Checking rule type
    stateType = _gameInstances[threadId]->getStateType(rulesStatus);

    // Break if not regular
    if (stateType != f_regular) break;
   }

   // If winning RNG seed, print value and exit
   if (stateType == f_win)
   {
    goodRNGFound++;
    if (seedFound == false) { printf("Found seed: %lu\n", seed); seedFound = true; }
   }
   else badRNGFound++;
  }

  // If no seeds were found, fail
  printf("No seed found\n");
  exit(-1);
}

int main(int argc, char *argv[])
{
 tester(argc, argv);
}
