#include <unistd.h>
#include <stdlib.h>
#include <set>
#include "argparse.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include <parallel_hashmap/phmap.h>
#include <omp.h>

// Configuration for parallel hash maps
#define MAPNAME phmap::parallel_flat_hash_map
#define MAPEXTRAARGS , phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 4, std::mutex
template <class K, class V> using HashMapT = MAPNAME<K, V MAPEXTRAARGS>;
using hashMap_t = HashMapT<uint32_t, uint32_t>;

uint64_t processedRNGs;
uint64_t targetRNGs;
uint64_t successRNGs;

struct level_t
{
 uint8_t levelId;
 std::string solutionFile;
 std::string moveSequence;
 std::vector<std::string> moveList;
 uint16_t sequenceLength;
 std::string stateFile;
 uint8_t stateData[_STATE_DATA_SIZE];
 uint8_t RNGOffset;
 uint8_t cutsceneRNGRate;
};

// Functions for the show thread
void* progressThreadFunction(void *ptr)
{
 while(true)
 {
  sleep(1);
  printf("[Progress] Success %lu - Total %lu/%lu (%.2f%%)\n", successRNGs, processedRNGs, targetRNGs, ((double)processedRNGs/(double)targetRNGs)*100.0);
 }
 return NULL;
}

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "2.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Parsing config file
  std::string configFile = program.get<std::string>("configFile");
  std::string configFileString;
  auto statusConfig = loadStringFromFile(configFileString, configFile.c_str());
  if (statusConfig == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  nlohmann::json config;
  try { config = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // Checking whether it contains the rules field
  if (isDefined(config, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rules' key.\n");

  // Checking whether it contains the emulator configuration field
  if (isDefined(config, "Emulator Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Emulator Configuration' key.\n");

  // Checking whether it contains the Game configuration field
  if (isDefined(config, "Game Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Configuration' key.\n");

  // Checking whether it contains the Tester configuration field
  if (isDefined(config, "Tester Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Tester Configuration' key.\n");

  // Progress variables
  processedRNGs = 0;
  targetRNGs = 0;
  successRNGs = 0;

  pthread_t progressThread;
  if (pthread_create(&progressThread, NULL, progressThreadFunction, NULL) != 0) EXIT_WITH_ERROR("[ERROR] Could not create show thread.\n");

  // Getting number of openMP threads
  auto _threadCount = omp_get_max_threads();

  // Creating game instances, one per openMP thread
  std::vector<GameInstance*> _gameInstances(_threadCount);
  std::vector<EmuInstance*> _emuInstances(_threadCount);

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
   }
  }

  // Level solution storage
  std::vector<level_t> levels;

  // Loading solution files
  for (const auto& level : config["Tester Configuration"]["Level Data"])
  {
   level_t lvlStruct;
   lvlStruct.levelId = level["Level Id"].get<uint8_t>();
   lvlStruct.solutionFile = level["Solution File"].get<std::string>();
   auto statusSolution = loadStringFromFile(lvlStruct.moveSequence, lvlStruct.solutionFile.c_str());
   if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", lvlStruct.solutionFile.c_str());
   lvlStruct.moveList = split(lvlStruct.moveSequence, ' ');
   lvlStruct.sequenceLength = lvlStruct.moveList.size();
   lvlStruct.stateFile = level["State File"].get<std::string>();
   _emuInstances[0]->loadStateFile(lvlStruct.stateFile);
   _gameInstances[0]->popState(lvlStruct.stateData);
   lvlStruct.RNGOffset = level["RNG Offset"].get<uint8_t>();
   lvlStruct.cutsceneRNGRate = level["Cutscene RNG Rate"].get<uint8_t>();
   levels.push_back(lvlStruct);
  }

//  uint32_t curRNG = 0x602D5760;
//  printf("0x%08X\n", curRNG);
//  for (size_t i = 0; i < 4; i++)
//  {
//   curRNG = emuInstance->reverseRNGState(curRNG);
//   printf("0x%08X\n", curRNG);
//  }
//  exit(0);

  const uint8_t posCopyProt = 4;
  seed_was_init = 1;
//  uint32_t maxRNG = 0xFFFFFFFF;
  uint32_t maxRNG = 0x000FFFFF;
  auto currentLastLooseSound = 0;
  hashMap_t goodRNGSet;
  uint8_t maxLevel = 0;

  #pragma omp parallel for
  for (uint32_t rngState = 0; rngState < maxRNG; rngState++)
  {
   gameState.random_seed = rngState;
   init_copyprot();
   if (copyprot_plac == posCopyProt) goodRNGSet[gameState.random_seed] = rngState;
  }
  printf("Copyright Success Rate: %lu/%u (%.2f%%)\n", goodRNGSet.size(), maxRNG, ((double)goodRNGSet.size() / (double)maxRNG)*100.0);

  for (size_t i = 0; i < levels.size(); i++)
  {
   // Adding cutscene rng states
   if (levels[i].cutsceneRNGRate > 0)
   {
    hashMap_t tmpRNGSet;
    for (const auto& rng : goodRNGSet)
    {
     gameState.random_seed = rng.first;
     for (size_t q = 0; q < 23; q++) // 2 seconds of cutscenes
     {
      for (uint8_t k = 0; k < levels[i].cutsceneRNGRate; k++) gameState.random_seed = _emuInstances[0]->advanceRNGState(gameState.random_seed);
      tmpRNGSet[gameState.random_seed] = rng.second;
     }
    }

    goodRNGSet = tmpRNGSet;
   }

   hashMap_t tmpRNGSet;

//   goodRNGSet.clear();
//   goodRNGSet.insert(0x92AEBFFF);

   std::vector<std::pair<uint32_t, uint32_t>> currentSet;
   for (const auto& rng : goodRNGSet) currentSet.push_back(std::make_pair(rng.first, rng.second));
   processedRNGs = 0;
   successRNGs = 0;
   targetRNGs = currentSet.size();
   uint8_t curMaxLevel = 0;

   #pragma omp parallel
   {
    // Getting thread id
    int threadId = omp_get_thread_num();

    #pragma omp for
    for (size_t rngIdx = 0; rngIdx < currentSet.size(); rngIdx++)
    {
     _gameInstances[threadId]->pushState(levels[i].stateData);
     gameState.random_seed = currentSet[rngIdx].first;
     gameState.last_loose_sound = currentLastLooseSound;

     for (uint8_t k = 0; k < levels[i].RNGOffset; k++) gameState.random_seed = _emuInstances[threadId]->advanceRNGState(gameState.random_seed);

     for (int j = 0; j < levels[i].sequenceLength && gameState.current_level == levels[i].levelId; j++)
     {
      _gameInstances[threadId]->advanceState(levels[i].moveList[j]);
      //printf("Step %u - Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", j, gameState.current_level, levels[i].moveList[j].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
     }

     if (gameState.current_level != levels[i].levelId)
     {

      if (i > curMaxLevel)
      {
       curMaxLevel = i;
       printf("Current Best RNG: 0x%08X\n", currentSet[rngIdx].second);
      }

      tmpRNGSet[gameState.random_seed] = currentSet[rngIdx].second;
      successRNGs = tmpRNGSet.size();
     }

     #pragma omp atomic
     processedRNGs++;
    }
   }

   maxLevel = i;
   printf("Level %u, Success Rate: %lu/%lu (%.2f%%)\n", levels[i].levelId, tmpRNGSet.size(), goodRNGSet.size(), ((double)tmpRNGSet.size() / (double)goodRNGSet.size())*100.0);
   if (tmpRNGSet.size() == 0) break;
   goodRNGSet = tmpRNGSet;
   currentLastLooseSound = gameState.last_loose_sound;
  }

  printf("Max Level: %u\n", levels[maxLevel].levelId);
}
