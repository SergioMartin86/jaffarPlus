#include <unistd.h>
#include <stdlib.h>
#include <set>
#include "argparse.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include <parallel_hashmap/phmap.h>
#include <omp.h>

struct solution_t
{
 uint32_t initialRNG;
 uint64_t timeStep;
 std::vector<uint32_t> cutsceneDelays;
 uint16_t totalDelay;
};

// Configuration for parallel hash maps
#define MAPNAME phmap::parallel_flat_hash_map
#define MAPEXTRAARGS , phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 4, std::mutex
template <class K, class V> using HashMapT = MAPNAME<K, V MAPEXTRAARGS>;
using hashMap_t = HashMapT<std::pair<uint32_t, uint8_t>, solution_t>;

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
 uint8_t stateData[_STATE_DATA_SIZE_TRAIN];
 uint8_t RNGOffset;
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

void solve(int argc, char *argv[])
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

  // Creating game instances, one per openMP thread
  GameInstance* _gameInstance;
  EmuInstance* _emuInstance;

  // Creating game and emulator instances, and parsing rules
  _emuInstance = new EmuInstance(config["Emulator Configuration"]);
  _gameInstance = new GameInstance(_emuInstance, config["Game Configuration"]);

//  uint32_t curRNG = 0x5973FFAB;
//  for (size_t i = 0; i < 12; i++)
//  {
//   uint32_t bigEndianRNG;
//   ((uint8_t*)&bigEndianRNG)[3] = ((uint8_t*)&curRNG)[0];
//   ((uint8_t*)&bigEndianRNG)[2] = ((uint8_t*)&curRNG)[1];
//   ((uint8_t*)&bigEndianRNG)[1] = ((uint8_t*)&curRNG)[2];
//   ((uint8_t*)&bigEndianRNG)[0] = ((uint8_t*)&curRNG)[3];
//
//   curRNG = _emuInstance->advanceRNGState(curRNG);
//
//   printf("RNG: 0x%08x\n", bigEndianRNG);
//  }
//  exit(0);

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
   _emuInstance->loadStateFile(lvlStruct.stateFile);
   _gameInstance->popState(lvlStruct.stateData);
   lvlStruct.RNGOffset = level["RNG Offset"].get<uint8_t>();
   levels.push_back(lvlStruct);
  }


  seed_was_init = 1;
  size_t curDelayIdx = 0;
  std::vector<uint8_t> delays({30, 21, 26, 16, 01 });
  gameState.random_seed = 0xFAD7CF1D;
  printf("0x%08X\n", gameState.random_seed);
  init_copyprot();
  printf("0x%08X\n", gameState.random_seed);
  init_copyprot();
  printf("0x%08X\n", gameState.random_seed);
  printf("Copy Prot Place: %u\n", copyprot_plac);
  if (copyprot_plac != 4) exit(0);

  uint32_t currentRNG = gameState.random_seed;
  uint8_t currentLastLooseSound = 0;
  gameState.last_loose_sound = 0;

  for (size_t i = 0; i < levels.size(); i++)
  {
   // Adding cutscene rng states
   if (cutsceneDelays[i].size() > 0)
   {
    for (ssize_t q = 0; q < delays[curDelayIdx]; q++)
    {
     uint32_t bigEndianRNG;
     ((uint8_t*)&bigEndianRNG)[3] = ((uint8_t*)&gameState.random_seed)[0];
     ((uint8_t*)&bigEndianRNG)[2] = ((uint8_t*)&gameState.random_seed)[1];
     ((uint8_t*)&bigEndianRNG)[1] = ((uint8_t*)&gameState.random_seed)[2];
     ((uint8_t*)&bigEndianRNG)[0] = ((uint8_t*)&gameState.random_seed)[3];
//     for (uint8_t k = 0; k < cutsceneDelays[i][q]; k++) gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);
     printf("Do Delay: 0x%08x\n", bigEndianRNG);
    }

    curDelayIdx++;
    //exit(0);
   }

   currentRNG = gameState.random_seed;
   currentLastLooseSound = gameState.last_loose_sound;

   _gameInstance->pushState(levels[i].stateData);

   gameState.random_seed = currentRNG;
   gameState.last_loose_sound = currentLastLooseSound;

   uint32_t bigEndianRNG;
   ((uint8_t*)&bigEndianRNG)[3] = ((uint8_t*)&gameState.random_seed)[0];
   ((uint8_t*)&bigEndianRNG)[2] = ((uint8_t*)&gameState.random_seed)[1];
   ((uint8_t*)&bigEndianRNG)[1] = ((uint8_t*)&gameState.random_seed)[2];
   ((uint8_t*)&bigEndianRNG)[0] = ((uint8_t*)&gameState.random_seed)[3];
   printf("PreLevel00: 0x%08x <<<--- Press enter here\n", bigEndianRNG);

   for (uint8_t k = 0; k < levels[i].RNGOffset; k++)
   {
//    gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);

    uint32_t bigEndianRNG;
    ((uint8_t*)&bigEndianRNG)[3] = ((uint8_t*)&gameState.random_seed)[0];
    ((uint8_t*)&bigEndianRNG)[2] = ((uint8_t*)&gameState.random_seed)[1];
    ((uint8_t*)&bigEndianRNG)[1] = ((uint8_t*)&gameState.random_seed)[2];
    ((uint8_t*)&bigEndianRNG)[0] = ((uint8_t*)&gameState.random_seed)[3];
    printf("PreLevel%02u: 0x%08x\n", k+1, bigEndianRNG);
   }

//   if (i == 2) exit(0);
   for (int j = 0; j < levels[i].sequenceLength && gameState.current_level == levels[i].levelId; j++)
   {
//    _gameInstance->advanceState(levels[i].moveList[j]);
    //printf("Step %u - Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", j, gameState.current_level, levels[i].moveList[j].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
    uint32_t bigEndianRNG;
    ((uint8_t*)&bigEndianRNG)[3] = ((uint8_t*)&gameState.random_seed)[0];
    ((uint8_t*)&bigEndianRNG)[2] = ((uint8_t*)&gameState.random_seed)[1];
    ((uint8_t*)&bigEndianRNG)[1] = ((uint8_t*)&gameState.random_seed)[2];
    ((uint8_t*)&bigEndianRNG)[0] = ((uint8_t*)&gameState.random_seed)[3];

    printf("Level %u, Step %04u/%04u: - RNG: 0x%08x, Loose: %u, Move: '%s', Room: %u\n", gameState.current_level, j+1, levels[i].sequenceLength-1, bigEndianRNG, gameState.last_loose_sound, levels[i].moveList[j+1].c_str(), gameState.Kid.room);
   }

   if (gameState.current_level == levels[i].levelId) exit(0);
  }
}

void explore(int argc, char *argv[])
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

  // Level solution storage
  std::vector<level_t> levels(config["Tester Configuration"]["Level Data"].size());

  // Loading solution files
  for (size_t i = 0; i < config["Tester Configuration"]["Level Data"].size(); i++)
  {
   const auto& level = config["Tester Configuration"]["Level Data"][i];
   levels[i].levelId = level["Level Id"].get<uint8_t>();
   levels[i].solutionFile = level["Solution File"].get<std::string>();
   auto statusSolution = loadStringFromFile(levels[i].moveSequence, levels[i].solutionFile.c_str());
   if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", levels[i].solutionFile.c_str());
   levels[i].moveList = split(levels[i].moveSequence, ' ');
   levels[i].sequenceLength = levels[i].moveList.size();
   levels[i].stateFile = level["State File"].get<std::string>();
   _emuInstances[0]->loadStateFile(levels[i].stateFile);
   _gameInstances[0]->popState(levels[i].stateData);
   levels[i].RNGOffset = level["RNG Offset"].get<uint8_t>();
  }

  std::map<uint64_t, solution_t> initialSet;

  uint8_t d  = 0;
  uint8_t h  = 12;
  uint8_t m  = 0;
  uint8_t s  = 0;
  uint8_t ds = 10;
  std::string ampm = "am";

  uint32_t curRNG = 0x00EA8E0F;
  for (size_t i = 0; i <= 1800000 && d == 0; i++)
//  for (size_t i = 0; i < 350000; i++)
  {
    if (initialSet.contains(curRNG) == false) initialSet[curRNG] = solution_t { .initialRNG = curRNG, .timeStep = i };
//    if (i == 1710000) { printf("+%02ud %02u:%02u:%02u.%02u %s - 0x%08X\n", d, h, m, s, ds, ampm.c_str(), curRNG); exit(0); }
    curRNG += 0x343FD;
    if (i > 0 && i % 10 == 0) curRNG -= 0x343FD;
    if (i > 0 && i % 96 == 0) curRNG += 0x343FD;
    if (i > 0 && i % 11000 == 0) curRNG -= 0x343FD;
    ds += 5;
    if (ds >= 100)
    {
     ds = ds % 100;
     s++;
     if (s == 60 )
     {
      s = 0;
      m++;
      if (m == 60 )
      {
       m = 0;
       if (h == 11)
       {
         if (ampm == "am") ampm = "pm";
         else if (ampm == "pm") { ampm = "am"; d++; }
         h = 12;
       }
       else { h++; h = h % 12;}
      }
     }
    }
  }
//  exit(0);
//  printf("Initial Set Size: %lu\n", initialSet.size());

  const uint8_t posCopyProt = 4;
  seed_was_init = 1;
//  uint32_t maxRNG = 0xFFFFFFFF;
//  uint32_t maxRNG = 0x000FFFFF;
  hashMap_t goodRNGSet;
  uint8_t maxLevel = 0;

//  initialSet.clear();
//  initialSet.push_back(solution_t { .initialRNG = 0x718D7F13, .timeStep = 212276 });
  for (const auto& solution : initialSet)
  {
   gameState.random_seed = solution.first;
   init_copyprot();
   init_copyprot();
   if (copyprot_plac == posCopyProt)  goodRNGSet[std::make_pair(gameState.random_seed, 0)] = solution.second;
  }
  printf("Copyright Success Rate: %lu/%lu (%.2f%%)\n", goodRNGSet.size(), initialSet.size(), ((double)goodRNGSet.size() / (double)initialSet.size())*100.0);

//  for (const auto& rng : goodRNGSet) printf("0x%08X\n", rng.second);
//  exit(0);

  for (size_t i = 0; i < levels.size(); i++)
  {
   // Adding cutscene rng states
   if (cutsceneDelays[i].size() > 0)
   {
    hashMap_t tmpRNGSet;
    for (const auto& rng : goodRNGSet)
    {
     uint32_t curSeed = rng.first.first;
     auto newEntry = rng.second;
     newEntry.cutsceneDelays.push_back(0);
     for (size_t q = 1; q < cutsceneDelays[i].size(); q++) // Cutscene frames to wait for
     {
      for (uint8_t k = 0; k < cutsceneDelays[i][q]; k++) curSeed = _emuInstances[0]->advanceRNGState(curSeed);
      newEntry.cutsceneDelays[newEntry.cutsceneDelays.size()-1] = q;
      tmpRNGSet[std::make_pair(curSeed, rng.first.second)] = newEntry;
     }
    }

    goodRNGSet = tmpRNGSet;
   }

   std::vector<std::pair<std::pair<uint32_t, uint8_t>, solution_t>> currentSet(goodRNGSet.begin(), goodRNGSet.end());

   processedRNGs = 0;
   successRNGs = 0;
   targetRNGs = currentSet.size();
   uint8_t curMaxLevel = 0;

   hashMap_t tmpRNGSet;

   #pragma omp parallel
   {
    // Getting thread id
    int threadId = omp_get_thread_num();

    #pragma omp for
    for (size_t rngIdx = 0; rngIdx < currentSet.size(); rngIdx++)
    {
     _gameInstances[threadId]->pushState(levels[i].stateData);
     roomleave_result = 0;
     gameState.random_seed = currentSet[rngIdx].first.first;
     gameState.last_loose_sound = currentSet[rngIdx].first.second;

     for (uint8_t k = 0; k < levels[i].RNGOffset; k++) gameState.random_seed = _emuInstances[threadId]->advanceRNGState(gameState.random_seed);

     size_t curMov = 0;
     for (; curMov < levels[i].sequenceLength && gameState.current_level == levels[i].levelId; curMov++)
     {
//      _gameInstances[threadId]->advanceState(levels[i].moveList[curMov]);
      //printf("Step %u - Base Level: %u / Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", curMov, levels[i].levelId, gameState.current_level, levels[i].moveList[curMov].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
     }

     if (gameState.current_level != levels[i].levelId)
     {
      if (i > curMaxLevel) curMaxLevel = i;
      #pragma omp critical
      tmpRNGSet[std::make_pair(gameState.random_seed, gameState.last_loose_sound)] = currentSet[rngIdx].second;
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
  }

  std::map<uint16_t, solution_t> finalSet;

  for (auto& rng : goodRNGSet)
  {
    rng.second.totalDelay = 0;
    for (size_t i = 0; i < rng.second.cutsceneDelays.size(); i++) rng.second.totalDelay += rng.second.cutsceneDelays[i];
    finalSet[rng.second.totalDelay] = rng.second;
  }

  for (const auto& rng : finalSet)
  {
    printf("0x%08X - Time: %lu Delays: [ ", rng.second.initialRNG, rng.second.timeStep);
    for (size_t i = 0; i < rng.second.cutsceneDelays.size(); i++) printf(" %02u ", rng.second.cutsceneDelays[i]);
    printf("] Total: %u\n", rng.second.totalDelay);
  }

  printf("Max Level: %u\n", levels[maxLevel].levelId);
}

int main(int argc, char *argv[])
{
// solve(argc, argv);
 explore(argc, argv);
}
