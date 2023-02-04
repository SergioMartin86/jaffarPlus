#include <unistd.h>
#include <stdlib.h>
#include "argparse.hpp"
#include <explorer.hpp>
#include <omp.h>

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

  // Creating game instances, one per openMP thread
  GameInstance* _gameInstance;
  EmuInstance* _emuInstance;

  // Creating game and emulator instances, and parsing rules
  _emuInstance = new EmuInstance(config["Emulator Configuration"]);
  _gameInstance = new GameInstance(_emuInstance, config["Game Configuration"]);

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


  uint32_t initialRNG = 0x4ED3A367;
  std::vector<uint8_t> cutsceneDelayCounts({  0,  0, 25,  0, 39,  0, 12,  0, 14,  8,  0,  0,  0,  0,  0 });
  std::vector<uint8_t> endDelayCounts(     {  0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0 });

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
  bool foundRNG = false;
  for (; curTime <= 1573040; curTime++)
  {
    if (curRNG == initialRNG)  { printf("i: %d -> +%02ud %02u:%02u:%02u.%02lu %s - 0x%08X\n", curTime, d, h, m, s, ns / 1000000, ampm.c_str(), curRNG); foundRNG = true; break; }
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

  if (foundRNG == false)
  {
   printf("Start RNG not found!\n");
   exit(0);
  }

  seed_was_init = 1;
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


  // Solver Start

  for (size_t i = 0; i < levels.size(); i++)
  {
   // Adding cutscene rng states
   for (ssize_t q = 0; q < cutsceneDelayCounts[i]; q++)
   {
    auto prev = gameState.random_seed;
//    if (prev != rngSet[q])
//    {
//     printf("RNGs %lu do not coincide: 0x%08x should be 0x%08x\n", q, prev, rngSet[q]);
//     exit(0);
//    }
    for (uint8_t k = 0; k < cutsceneDelays[i][q]; k++) gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);
    printf("Do Cutscene Delay: 0x%08x -> 0x%08x\n", prev, gameState.random_seed);
   }

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
    printf("Level %u, Step %04u/%04u: - RNG: 0x%08x, Loose: %u, Move: '%s', Room: %u", gameState.current_level, j+1, levels[i].sequenceLength-1, gameState.random_seed, gameState.last_loose_sound, levels[i].moveListStrings[j].c_str(), gameState.Kid.room);
    if (endDelayCounts[i] > 0) if (j - endDelayCounts[i] > 0) if (levels[i].moveListStrings[j-endDelayCounts[i]] == "....U..") printf(" <<---- New U");
    printf("\n");
    _gameInstance->advanceGameState(levels[i].moveList[j]);
    if (gameState.hitp_curr == 0)
    {
     printf("Death detected!\n");
     exit(0);
    }
    //printf("Step %u - Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", j, gameState.current_level, levels[i].moveList[j].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
   }

   // Adding end delays
   for (ssize_t q = 0; q < endDelayCounts[i]; q++)
   {
    auto prev = gameState.random_seed;
    for (uint8_t k = 0; k < endWaitDelays[i][q]; k++) gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);
    printf("Do End Delay: 0x%08x -> 0x%08x", prev, gameState.random_seed);
    if (levels[i].moveListStrings[levels[i].sequenceLength-q-1] == "....U..") printf(" <<---- New U");
    printf("\n");
   }

//   if (i == 2) exit(0);
  }

  return 0;
}
