#include <unistd.h>
#include <stdlib.h>
#include "argparse.hpp"
#include <explorer.hpp>
#include <omp.h>

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

int main(int argc, char *argv[])
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
  //1573040
  for (; curTime <= 1573040 && d == 0; curTime++)
  {
    if (initialSet.contains(curRNG) == false) initialSet[curRNG] = solution_t { .initialRNG = curRNG, .timeStep = curTime };
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

  for (const auto& solution : initialSet)
  {
   gameState.random_seed = solution.first;
   init_copyprot();
   if (copyprot_plac == posCopyProt)
    goodRNGSet[std::make_pair(gameState.random_seed, 0)] = solution.second;
  }
  printf("Copyright Success Rate: %lu/%lu (%.2f%%)\n", goodRNGSet.size(), initialSet.size(), ((double)goodRNGSet.size() / (double)initialSet.size())*100.0);


  ////// Explorer Start
  for (size_t i = 0; i < levels.size(); i++)
  {
   // Flattening current set
   printf("Starting Level: %u\n", levels[i].levelId);

   // Adding cutscene rng delays
   #define MAX_CUTSCENE_DELAY 44
   if (cutsceneDelays[i].size() > 0)
   {
    printf("Adding cutscene delays...\n");
    printf("Flattening Set...\n");
    std::vector<solutionFlat_t> flatSet;
    flatSet.reserve(goodRNGSet.size());
    for (const auto& x : goodRNGSet) flatSet.push_back(solutionFlat_t { .rng = x.first.first, .looseSound = x.first.second, .solution = x.second });
    goodRNGSet.clear();

    int currentUnflatteningStep = 0;

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
       newEntry.totalDelay++;
       newFlatSet.push_back(solutionFlat_t { .rng = curSeed, .looseSound = flatSet[idx].looseSound, .solution = newEntry });
      }
     }

     // Printing progress and clearing input set
     #pragma omp master
     {
      printf("Unflattening set...\n");
      flatSet.clear();
     }

     // Pushing new states to RNG set
     #pragma omp critical
     {
      if (currentUnflatteningStep % 10 == 0) printf("Unflatenning Step: %u/%u\n", currentUnflatteningStep, _threadCount);
      for (size_t idx = 0; idx < newFlatSet.size(); idx++)
      {
       auto key = std::make_pair(newFlatSet[idx].rng, newFlatSet[idx].looseSound);
       bool isKeyPresent = goodRNGSet.contains(key);
       bool addEntry = false;
       if (isKeyPresent == false) addEntry = true;
       if ((isKeyPresent == true) &&  (goodRNGSet[key].totalDelay > newFlatSet[idx].solution.totalDelay)) addEntry = true;
       if (addEntry == true) goodRNGSet[key] = newFlatSet[idx].solution;
      }
      currentUnflatteningStep++;
     }
    }
   }

   // Storage for new states
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

    if (endWaitDelays[i].size() > 0)
    {
     printf("Adding start wait delays...\n");
     printf("Flattening Set...\n");
     std::vector<solutionFlat_t> flatSet;
     flatSet.reserve(goodRNGSet.size());
     for (const auto& x : goodRNGSet) flatSet.push_back(solutionFlat_t { .rng = x.first.first, .looseSound = x.first.second, .solution = x.second });
     goodRNGSet.clear();
     int currentUnflatteningStep = 0;

     #pragma omp parallel
     {
      std::vector<solutionFlat_t> newFlatSet;

      #pragma omp for
      for (size_t idx = 0; idx < flatSet.size(); idx++)
      {
       uint32_t curSeed = flatSet[idx].rng;
       auto newEntry = flatSet[idx].solution;

       newEntry.endDelays[i] = 0;
       newFlatSet.push_back(solutionFlat_t { .rng = curSeed, .looseSound = flatSet[idx].looseSound, .solution = newEntry });

       // Now adding cutscene delays (first one is mandatory)
       for (size_t q = 0; q < endWaitDelays[i].size(); q++) // Cutscene frames to wait for
       {
        for (uint8_t k = 0; k < endWaitDelays[i][q]; k++) curSeed = _emuInstances[0]->advanceRNGState(curSeed);
        newEntry.endDelays[i] = q+1;
        newEntry.totalDelay++;
        if (levels[i].levelId != 15) newEntry.costlyDelay++; // Increase costly delay, except for copyright level, where it doesn't matter
        newFlatSet.push_back(solutionFlat_t { .rng = curSeed, .looseSound = flatSet[idx].looseSound, .solution = newEntry });
       }
      }

      // Clearing old set
      #pragma omp master
      {
       printf("Unflattening set...\n");
       flatSet.clear();
      }

      // Pushing new states to RNG set
      #pragma omp critical
      {
       if (currentUnflatteningStep % 10 == 0) printf("Unflatenning Step: %u/%u\n", currentUnflatteningStep, _threadCount);
       for (size_t idx = 0; idx < newFlatSet.size(); idx++)
       {

        auto key = std::make_pair(newFlatSet[idx].rng, newFlatSet[idx].looseSound);
        bool isKeyPresent = goodRNGSet.contains(key);
        bool addEntry = false;
        if (isKeyPresent == false) addEntry = true;
        if ((isKeyPresent == true) &&  (goodRNGSet[key].costlyDelay > newFlatSet[idx].solution.costlyDelay)) addEntry = true;
        if (addEntry == true) goodRNGSet[key] = newFlatSet[idx].solution;

       }
       currentUnflatteningStep++;
      }
     }
    }
  }

  // Printing Final Set
  printf("Printing final set...\n");
  std::map<uint32_t, solution_t> uniqueSet;
  std::map<uint16_t, solution_t> sortedSet;

  for (auto& rng : goodRNGSet)
  {
   bool replaceEntry = false;
   if (uniqueSet.contains(rng.second.timeStep) == false) replaceEntry = true;
   if (uniqueSet.contains(rng.second.timeStep) == true && (uniqueSet[rng.second.timeStep].costlyDelay > rng.second.costlyDelay)) replaceEntry = true;
   if (uniqueSet.contains(rng.second.timeStep) == true && (uniqueSet[rng.second.timeStep].costlyDelay == rng.second.costlyDelay && uniqueSet[rng.second.timeStep].totalDelay > rng.second.totalDelay)) replaceEntry = true;
   if (replaceEntry == true) uniqueSet[rng.second.timeStep] = rng.second;
  }
  for (auto& rng : uniqueSet) sortedSet[rng.second.totalDelay] = rng.second;

  for (const auto& rng : sortedSet)
  {
    printf("0x%08X - Time: %u\n", rng.second.initialRNG, rng.second.timeStep);
    printf(" + Cutscene Delays: { %2u", rng.second.cutsceneDelays[0]); for (size_t i = 1; i < 15; i++) printf(", %2u", rng.second.cutsceneDelays[i]);  printf(" }\n");
    printf(" + End Delays:      { %2u", rng.second.endDelays[0]);      for (size_t i = 1; i < 15; i++) printf(", %2u", rng.second.endDelays[i]);     printf(" }\n");
    printf(" + Total Delay: %u\n", rng.second.totalDelay);
    printf(" + Costly Delay: %u\n", rng.second.costlyDelay);
  }

  printf("Max Level: %u\n", levels[maxLevel].levelId);

  return 0;
}
