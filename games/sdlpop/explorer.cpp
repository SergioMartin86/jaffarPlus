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

  // Copyprotection alternate solutions
  std::vector<level_t> copyProtSolutions(COPYPROT_SOLUTION_COUNT);

  // Loading solution files
  for (size_t i = 0; i < config["Explorer Configuration"]["Level Data"].size(); i++)
  {
   const auto& level = config["Explorer Configuration"]["Level Data"][i];
   levels[i].levelId = level["Level Id"].get<uint8_t>();
   printf("Loading Level %d...\n", levels[i].levelId);

   // Loading level solution
   if (levels[i].levelId != 15)
   {
    levels[i].solutionFile = level["Solution File"].get<std::string>();
    auto statusSolution = loadStringFromFile(levels[i].moveSequence, levels[i].solutionFile.c_str());
    if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", levels[i].solutionFile.c_str());
    levels[i].moveListStrings = split(levels[i].moveSequence, ' ');
    levels[i].sequenceLength = levels[i].moveListStrings.size();
    for (size_t j = 0; j < levels[i].sequenceLength; j++)  levels[i].moveList.push_back(EmuInstance::moveStringToCode(levels[i].moveListStrings[j]));
   }
   else
   {
    for (uint8_t cidx = 0; cidx < COPYPROT_SOLUTION_COUNT; cidx++)
    {
     copyProtSolutions[cidx].solutionFile = level["Solution File"].get<std::string>() + std::to_string(cidx);
     auto statusSolution = loadStringFromFile(copyProtSolutions[cidx].moveSequence, copyProtSolutions[cidx].solutionFile.c_str());
     if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", copyProtSolutions[cidx].solutionFile.c_str());
     copyProtSolutions[cidx].moveListStrings = split(copyProtSolutions[cidx].moveSequence, ' ');
     copyProtSolutions[cidx].sequenceLength = copyProtSolutions[cidx].moveListStrings.size();
     for (size_t j = 0; j < copyProtSolutions[cidx].sequenceLength; j++)  copyProtSolutions[cidx].moveList.push_back(EmuInstance::moveStringToCode(copyProtSolutions[cidx].moveListStrings[j]));
    }
   }

   levels[i].stateFile = level["State File"].get<std::string>();
   _emuInstances[0]->loadStateFile(levels[i].stateFile);
   _gameInstances[0]->popState(levels[i].stateData);
   levels[i].RNGOffset = level["RNG Offset"].get<uint8_t>();
  }

  std::map<rng_t, clockTick_t> initialSet;

  seed_was_init = 1;
  hashMap_t goodRNGSet;
  uint8_t maxLevel = 0;

  clockTick_t currTick = 0;
  for (; currTick <= MAX_CLOCK_TICKS; currTick++)
  {
   const rng_t curRNG = getRNGFromClockTick(currTick);
   if (initialSet.contains(curRNG) == false) initialSet[curRNG] = currTick;
  }

  printf("Last clock tick: %u\n", currTick);
  printf("Entries: %lu\n", initialSet.size());

  // Processing copyright
//  initialSet.clear();
//  initialSet[0x01C21549] = 103;
  for (auto& solution : initialSet)
  {
   gameState.random_seed = solution.first;
   init_copyprot();
//   if (copyprot_plac == 4)
    goodRNGSet[std::make_pair(gameState.random_seed, 0)] = solution_t { .clockTick = solution.second, .copyProtPlace = (uint8_t)copyprot_plac };
  }

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
      rng_t curSeed = flatSet[idx].rng;
      auto newEntry = flatSet[idx].solution;

      // Now adding cutscene delays (first one is mandatory)
      for (size_t q = 0; q < cutsceneDelays[i].size() && q < MAX_CUTSCENE_DELAY; q++) // Cutscene frames to wait for
      {
       for (uint8_t k = 0; k < cutsceneDelays[i][q]; k++) curSeed = _emuInstances[0]->advanceRNGState(curSeed);

       #ifdef STORE_DELAY_HISTORY
       newEntry.cutsceneDelays[i] = q+1;
       #endif

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
   std::vector<std::pair<std::pair<rng_t, uint8_t>, solution_t>> currentSet(goodRNGSet.begin(), goodRNGSet.end());

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

     if (levels[i].levelId != 15)
       for (; curMov < levels[i].sequenceLength && gameState.current_level == levels[i].levelId; curMov++)  _gameInstances[threadId]->advanceGameState(levels[i].moveList[curMov]);
     else
       for (; curMov < copyProtSolutions[currentSet[rngIdx].second.copyProtPlace].sequenceLength && gameState.current_level == levels[i].levelId; curMov++)  _gameInstances[threadId]->advanceGameState(copyProtSolutions[currentSet[rngIdx].second.copyProtPlace].moveList[curMov]);

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
       rng_t curSeed = flatSet[idx].rng;
       auto newEntry = flatSet[idx].solution;

       #ifdef STORE_DELAY_HISTORY
       newEntry.endDelays[i] = 0;
       #endif

       newFlatSet.push_back(solutionFlat_t { .rng = curSeed, .looseSound = flatSet[idx].looseSound, .solution = newEntry });

       // Now adding cutscene delays (first one is mandatory)
       for (size_t q = 0; q < endWaitDelays[i].size(); q++) // Cutscene frames to wait for
       {
        for (uint8_t k = 0; k < endWaitDelays[i][q]; k++) curSeed = _emuInstances[0]->advanceRNGState(curSeed);

        #ifdef STORE_DELAY_HISTORY
        newEntry.endDelays[i] = q+1;
        #endif

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
  std::map<rng_t, solution_t> uniqueSet;
  std::map<rng_t, solution_t> sortedSet;

  for (auto& rng : goodRNGSet)
  {
   bool replaceEntry = false;
   if (uniqueSet.contains(rng.second.clockTick) == false) replaceEntry = true;
   if (uniqueSet.contains(rng.second.clockTick) == true && (uniqueSet[rng.second.clockTick].costlyDelay > rng.second.costlyDelay)) replaceEntry = true;
   if (uniqueSet.contains(rng.second.clockTick) == true && (uniqueSet[rng.second.clockTick].costlyDelay == rng.second.costlyDelay && uniqueSet[rng.second.clockTick].totalDelay > rng.second.totalDelay)) replaceEntry = true;
   if (replaceEntry == true) uniqueSet[rng.second.clockTick] = rng.second;
  }
  for (auto& rng : uniqueSet) sortedSet[rng.second.costlyDelay] = rng.second;

  for (const auto& rng : sortedSet)
  {
    printf("0x%08X - ClockTick: %u\n", getRNGFromClockTick(rng.second.clockTick), rng.second.clockTick);

    #ifdef STORE_DELAY_HISTORY
    printf(" + Cutscene Delays: { %2u", rng.second.cutsceneDelays[0]); for (size_t i = 1; i < 15; i++) printf(", %2u", rng.second.cutsceneDelays[i]);  printf(" }\n");
    printf(" + End Delays:      { %2u", rng.second.endDelays[0]);      for (size_t i = 1; i < 15; i++) printf(", %2u", rng.second.endDelays[i]);     printf(" }\n");
    #endif

    printf(" + Total Delay: %u\n", rng.second.totalDelay);
    printf(" + Costly Delay: %u\n", rng.second.costlyDelay);
    printf(" + CopyProtPlace: %u\n", rng.second.copyProtPlace);
  }

  printf("Max Level: %u\n", levels[maxLevel].levelId);

  return 0;
}
