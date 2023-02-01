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
 uint32_t timeStep;
 uint8_t cutsceneDelays[16];
 uint16_t totalCutsceneDelay;
 uint8_t startDelays[16];
 uint16_t totalStartDelay;
};

struct solutionFlat_t
{
 uint32_t rng;
 uint8_t looseSound;
 solution_t solution;
};

// Configuration for parallel hash maps
#define MAPNAME phmap::parallel_flat_hash_map
#define MAPEXTRAARGS , phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 4, std::mutex
template <class K, class V> using HashMapT = MAPNAME<K, V MAPEXTRAARGS>;
using hashMap_t = HashMapT<std::pair<uint32_t, uint8_t>, solution_t>;

uint64_t processedRNGs;
uint64_t targetRNGs;
uint64_t successRNGs;
bool processing = false;

struct level_t
{
 uint8_t levelId;
 std::string solutionFile;
 std::string moveSequence;
 std::vector<std::string> moveListStrings;
 std::vector<uint8_t> moveList;
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
  if (processing == true) printf("[Progress] Success %lu - Total %lu/%lu (%.2f%%)\n", successRNGs, processedRNGs, targetRNGs, ((double)processedRNGs/(double)targetRNGs)*100.0);
 }
 return NULL;
}

void explore(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-Explorer", "2.0");

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

  // Checking whether it contains the Explorer configuration field
  if (isDefined(config, "Explorer Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Explorer Configuration' key.\n");

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
  std::vector<level_t> levels(config["Explorer Configuration"]["Level Data"].size());

  // Loading solution files
  for (size_t i = 0; i < config["Explorer Configuration"]["Level Data"].size(); i++)
  {
   const auto& level = config["Explorer Configuration"]["Level Data"][i];
   levels[i].levelId = level["Level Id"].get<uint8_t>();
   printf("Loading Level %d...\n", levels[i].levelId);
   levels[i].solutionFile = level["Solution File"].get<std::string>();
   auto statusSolution = loadStringFromFile(levels[i].moveSequence, levels[i].solutionFile.c_str());
   if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", levels[i].solutionFile.c_str());
   levels[i].moveListStrings = split(levels[i].moveSequence, ' ');
   levels[i].sequenceLength = levels[i].moveListStrings.size();
   //printf("sequenceLength: %lu\n", levels[i].sequenceLength);
   for (size_t j = 0; j < levels[i].sequenceLength; j++)
   {
//    printf("Move %d, %s\n", j, levels[i].moveListStrings[j].c_str());
    levels[i].moveList.push_back(EmuInstance::moveStringToCode(levels[i].moveListStrings[j]));
   }
   levels[i].stateFile = level["State File"].get<std::string>();
   _emuInstances[0]->loadStateFile(levels[i].stateFile);
   _gameInstances[0]->popState(levels[i].stateData);
   levels[i].RNGOffset = level["RNG Offset"].get<uint8_t>();
  }

  std::map<uint32_t, solution_t> initialSet;

  uint8_t d  = 0;
  uint8_t h  = 12;
  uint8_t m  = 0;
  uint8_t s  = 0;
  int64_t ns = 4200000;
  std::string ampm = "am";

  const uint8_t posCopyProt = 4;
  seed_was_init = 1;
  hashMap_t goodRNGSet;
  uint8_t maxLevel = 0;

  uint32_t curRNG = 0x0071BA7E;
  uint32_t curTime = 0;
  //15800000
  for (; curTime <= 158000; curTime++)
  {
    if (initialSet.contains(curRNG) == false) initialSet[curRNG] = solution_t { .initialRNG = curRNG, .timeStep = curTime };
//    if (curRNG == 0x69b6329b)  { printf("i: %ld -> +%02ud %02u:%02u:%02u.%02lu %s - 0x%08X\n", i, d, h, m, s, ns / 1000000, ampm.c_str(), curRNG); }
//    if (curRNG == 0xbbe7e92)   { printf("i: %ld -> +%02ud %02u:%02u:%02u.%02lu %s - 0x%08X\n", i, d, h, m, s, ns / 1000000, ampm.c_str(), curRNG); }
//    if (curRNG == 0xd7fd5650)  { printf("i: %ld -> +%02ud %02u:%02u:%02u.%02lu %s - 0x%08X\n", i, d, h, m, s, ns / 1000000, ampm.c_str(), curRNG); }
//    if (curRNG == 0x8f8b548)  { printf("i: %ld -> +%02ud %02u:%02u:%02u.%02lu %s - 0x%08X\n", i, d, h, m, s, ns / 1000000, ampm.c_str(), curRNG); exit(0); }
//    if (i == 4) { printf("+%02ud %02u:%02u:%02u.%02lu %s - 0x%08X\n", d, h, m, s, ns / 1000000, ampm.c_str(), curRNG); exit(0); }
    curRNG += 0x343FD;
    ns += 5492550;
    if (ns >= 100000000)
    {
     ns = ns % 100000000;
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

  printf("Last Timestep: %u\n", curTime);
  printf("Entries: %lu\n", initialSet.size());
//  exit(0);
//  printf("Initial Set Size: %lu\n", initialSet.size());

//  curRNG = 0x00000DF3;
//  for (size_t i = 0; i < 70; i++)
//  {
//   printf("%lu: 0x%08X", i , curRNG);
//   if (curRNG == 0xDACDB1D4) printf(" <");
//   if (curRNG == 0x1D5935EB) printf(" !1");
//   if (curRNG == 0x152D1475) printf(" !2");
//   printf("\n");
//   curRNG = _emuInstances[0]->advanceRNGState(curRNG);
//  }

//  initialSet.clear();
//  initialSet[0x00000DF3] = solution_t { .initialRNG = 0x00000DF3, .timeStep = 662233 };
  for (const auto& solution : initialSet)
  {
   gameState.random_seed = solution.first;
//   printf("R 0x%08X\n", gameState.random_seed);
   init_copyprot();
//   printf("R 0x%08X\n", gameState.random_seed);
   if (copyprot_plac == posCopyProt)
   {
    goodRNGSet[std::make_pair(gameState.random_seed, 0)] = solution.second;
//    if (solution.second.timeStep < 300)
//    {
//     printf("Found good copyprot place: %lu (RNG 0x%08lX -> 0x%08X)\n", solution.second.timeStep, solution.first, gameState.random_seed);
//     exit(0);
//    }
   }
  }
  printf("Copyright Success Rate: %lu/%lu (%.2f%%)\n", goodRNGSet.size(), initialSet.size(), ((double)goodRNGSet.size() / (double)initialSet.size())*100.0);

//  for (const auto& rng : goodRNGSet) printf("0x%08X\n", rng.second);
//  exit(0);


  // Adding cutscene rng delays
#define MAX_CUTSCENE_DELAY 48
  for (size_t i = 0; i < levels.size(); i++)
  {
   // Flattening current set
   printf("Starting Level: %u\n", levels[i].levelId);

   if (cutsceneDelays[i].size() > 0)
   {
    printf("Adding cutscene delays...\n");
    printf("Flattening Set...\n");
    std::vector<solutionFlat_t> flatSet;
    flatSet.reserve(goodRNGSet.size());
    for (const auto& x : goodRNGSet) flatSet.push_back(solutionFlat_t { .rng = x.first.first, .looseSound = x.first.second, .solution = x.second });
    goodRNGSet.clear();

    #pragma omp parallel
    {
     std::vector<solutionFlat_t> newFlatSet;

     #pragma omp for
     for (size_t idx = 0; idx < flatSet.size(); idx++)
     {
      uint32_t curSeed = flatSet[idx].rng;
      auto newEntry = flatSet[idx].solution;

      // Now adding cutscene delays (first one is mandatory)
      for (size_t q = 0; q < cutsceneDelays[i].size() && q < MAX_CUTSCENE_DELAY; q++) // Cutscene frames to wait for
      {
       for (uint8_t k = 0; k < cutsceneDelays[i][q]; k++) curSeed = _emuInstances[0]->advanceRNGState(curSeed);
       newEntry.cutsceneDelays[i] = q+1;
       newFlatSet.push_back(solutionFlat_t { .rng = curSeed, .looseSound = flatSet[idx].looseSound, .solution = newEntry });
      }
     }

     #pragma omp master
     {
      printf("Unflattening set...\n");
      flatSet.clear();
     }

     #pragma omp critical
     for (size_t idx = 0; idx < newFlatSet.size(); idx++) goodRNGSet[std::make_pair(newFlatSet[idx].rng, newFlatSet[idx].looseSound)] = newFlatSet[idx].solution;
    }
   }

   if (startWaitDelays[i].size() > 0)
   {
    printf("Adding start wait delays...\n");
    printf("Flattening Set...\n");
    std::vector<solutionFlat_t> flatSet;
    flatSet.reserve(goodRNGSet.size());
    for (const auto& x : goodRNGSet) flatSet.push_back(solutionFlat_t { .rng = x.first.first, .looseSound = x.first.second, .solution = x.second });
    goodRNGSet.clear();

    #pragma omp parallel
    {
     std::vector<solutionFlat_t> newFlatSet;

     #pragma omp for
     for (size_t idx = 0; idx < flatSet.size(); idx++)
     {
      uint32_t curSeed = flatSet[idx].rng;
      auto newEntry = flatSet[idx].solution;

      newEntry.startDelays[i] = 0;
      newFlatSet.push_back(solutionFlat_t { .rng = curSeed, .looseSound = flatSet[idx].looseSound, .solution = newEntry });

      // Now adding cutscene delays (first one is mandatory)
      for (size_t q = 0; q < startWaitDelays[i].size(); q++) // Cutscene frames to wait for
      {
       for (uint8_t k = 0; k < startWaitDelays[i][q]; k++) curSeed = _emuInstances[0]->advanceRNGState(curSeed);
       newEntry.startDelays[i] = q+1;
       newFlatSet.push_back(solutionFlat_t { .rng = curSeed, .looseSound = flatSet[idx].looseSound, .solution = newEntry });
      }
     }

     #pragma omp master
     {
      printf("Unflattening set...\n");
      flatSet.clear();
     }

     #pragma omp critical
     for (size_t idx = 0; idx < newFlatSet.size(); idx++) goodRNGSet[std::make_pair(newFlatSet[idx].rng, newFlatSet[idx].looseSound)] = newFlatSet[idx].solution;
    }
   }

//   // Adding start wait rng delays
//   printf("Adding start wait delays...")
//   if (startWaitDelays[i].size() > 0)
//   {
//    hashMap_t tmpRNGSet;
//    for (const auto& rng : goodRNGSet)
//    {
//     uint32_t curSeed = rng.first.first;
//     auto newEntry = rng.second;
//
//     // Now adding start level delays
//     for (size_t q = 0; q < startWaitDelays[i].size(); q++) // start frames to wait for
//     {
//      for (uint8_t k = 0; k < startWaitDelays[i][q]; k++) curSeed = _emuInstances[0]->advanceRNGState(curSeed);
//      newEntry.startDelays[i] = q;
//      tmpRNGSet[std::make_pair(curSeed, rng.first.second)] = newEntry;
//     }
//    }
//   }

   std::vector<std::pair<std::pair<uint32_t, uint8_t>, solution_t>> currentSet(goodRNGSet.begin(), goodRNGSet.end());

   processedRNGs = 0;
   successRNGs = 0;
   targetRNGs = currentSet.size();
   uint8_t curMaxLevel = 0;

   hashMap_t tmpRNGSet;

   printf("Running solution...\n");
   processing = true;
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
      _gameInstances[threadId]->advanceGameState(levels[i].moveList[curMov]);
      //printf("Step %lu - Base Level: %u / Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", curMov, levels[i].levelId, gameState.current_level, levels[i].moveListStrings[curMov].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
     }

     if (gameState.next_level != levels[i].levelId)
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
    processing = false;
  }

  std::map<uint16_t, solution_t> finalSet;

  for (auto& rng : goodRNGSet)
  {
    rng.second.totalCutsceneDelay = 0;
    for (size_t i = 0; i < 15; i++) rng.second.totalCutsceneDelay += rng.second.cutsceneDelays[i];
    for (size_t i = 0; i < 15; i++) rng.second.totalStartDelay += rng.second.startDelays[i];
    finalSet[rng.second.totalCutsceneDelay + rng.second.totalStartDelay] = rng.second;
  }

  for (const auto& rng : finalSet)
  {
    printf("0x%08X - Time: %u Cutscene Delays: [ ", rng.second.initialRNG, rng.second.timeStep);
    for (size_t i = 0; i < 15; i++) printf(" %02u ", rng.second.cutsceneDelays[i]);
    printf("] Total: %u - ", rng.second.totalCutsceneDelay);

    printf("Start Delays: [ ");
    for (size_t i = 0; i < 15; i++) printf(" %02u ", rng.second.startDelays[i]);
    printf("] Total: %u\n", rng.second.totalStartDelay);
  }

  printf("Max Level: %u\n", levels[maxLevel].levelId);
}

void solve(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-Explorer", "2.0");

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

  // Checking whether it contains the Explorer configuration field
  if (isDefined(config, "Explorer Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Explorer Configuration' key.\n");

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
  for (const auto& level : config["Explorer Configuration"]["Level Data"])
  {
   level_t lvlStruct;
   lvlStruct.levelId = level["Level Id"].get<uint8_t>();
   lvlStruct.solutionFile = level["Solution File"].get<std::string>();
   auto statusSolution = loadStringFromFile(lvlStruct.moveSequence, lvlStruct.solutionFile.c_str());
   if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", lvlStruct.solutionFile.c_str());
   lvlStruct.moveListStrings = split(lvlStruct.moveSequence, ' ');
   lvlStruct.sequenceLength = lvlStruct.moveListStrings.size();

   for (size_t i = 0; i < lvlStruct.sequenceLength; i++) lvlStruct.moveList.push_back(EmuInstance::moveStringToCode(lvlStruct.moveListStrings[i]));

   lvlStruct.stateFile = level["State File"].get<std::string>();
   _emuInstance->loadStateFile(lvlStruct.stateFile);
   _gameInstance->popState(lvlStruct.stateData);
   lvlStruct.RNGOffset = level["RNG Offset"].get<uint8_t>();
   levels.push_back(lvlStruct);
  }


  uint32_t initialRNG = 0xD90FCF97;

  uint8_t d  = 0;
  uint8_t h  = 12;
  uint8_t m  = 0;
  uint8_t s  = 0;
  int64_t ns = 4200000;
  std::string ampm = "am";

  seed_was_init = 1;
  hashMap_t goodRNGSet;

  uint32_t curRNG = 0x0071BA7E;
  uint32_t curTime = 0;
  //15800000
  for (; curTime <= 158000; curTime++)
  {
    if (curRNG == initialRNG)  { printf("i: %d -> +%02ud %02u:%02u:%02u.%02lu %s - 0x%08X\n", curTime, d, h, m, s, ns / 1000000, ampm.c_str(), curRNG); break; }
    curRNG += 0x343FD;
    ns += 5492550;
    if (ns >= 100000000)
    {
     ns = ns % 100000000;
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

  seed_was_init = 1;
  size_t curDelayIdx = 0;
  std::vector<uint8_t> cutsceneDelayCounts({ 00, 00, 06, 00, 01, 00, 05, 00, 01, 01, 00, 00, 00, 00, 00 });
  gameState.random_seed = initialRNG;
  printf("0x%08X\n", gameState.random_seed);
  init_copyprot();
  printf("0x%08X\n", gameState.random_seed);
  printf("Copy Prot Place: %u\n", copyprot_plac);
  if (copyprot_plac != 4)
  {
   printf("Bad Seed!\n");
   exit(0);
  }

  uint32_t currentRNG;
  uint8_t currentLastLooseSound = 0;
  gameState.last_loose_sound = 0;

  for (size_t i = 0; i < levels.size(); i++)
  {
//   // Adding cutscene rng states
//   for (ssize_t q = 0; q < cutsceneDelayCounts[curDelayIdx]; q++)
//   {
//    uint32_t bigEndianRNG;
//    ((uint8_t*)&bigEndianRNG)[3] = ((uint8_t*)&gameState.random_seed)[0];
//    ((uint8_t*)&bigEndianRNG)[2] = ((uint8_t*)&gameState.random_seed)[1];
//    ((uint8_t*)&bigEndianRNG)[1] = ((uint8_t*)&gameState.random_seed)[2];
//    ((uint8_t*)&bigEndianRNG)[0] = ((uint8_t*)&gameState.random_seed)[3];
//    for (uint8_t k = 0; k < cutsceneDelays[i][q]; k++) gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);
//    printf("Do Delay: 0x%08x\n", bigEndianRNG);
//   }

   curDelayIdx++;

   currentRNG = gameState.random_seed;
   currentLastLooseSound = gameState.last_loose_sound;
   _gameInstance->pushState(levels[i].stateData);
   gameState.random_seed = currentRNG;
   gameState.last_loose_sound = currentLastLooseSound;

   printf("PreLevel00: 0x%08x <<<--- Press enter here\n", gameState.random_seed);

   for (uint8_t k = 0; k < levels[i].RNGOffset; k++)
   {
    gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);
    printf("PreLevel%02u: 0x%08x\n", k+1, gameState.random_seed);
   }

   for (int j = 0; j < levels[i].sequenceLength && gameState.current_level == levels[i].levelId; j++)
   {
    printf("Level %u, Step %04u/%04u: - RNG: 0x%08x, Loose: %u, Move: '%s', Room: %u\n", gameState.current_level, j+1, levels[i].sequenceLength-1, gameState.random_seed, gameState.last_loose_sound, levels[i].moveListStrings[j].c_str(), gameState.Kid.room);
    _gameInstance->advanceGameState(levels[i].moveList[j]);
    //printf("Step %u - Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", j, gameState.current_level, levels[i].moveList[j].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
   }

   if (gameState.current_level == levels[i].levelId) exit(0);
  }
}

int main(int argc, char *argv[])
{
 solve(argc, argv);
 explore(argc, argv);
}
