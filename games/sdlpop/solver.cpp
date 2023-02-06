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

  // Copyprotection alternate solutions
  std::vector<level_t> copyProtSolutions(COPYPROT_SOLUTION_COUNT);

  // Loading solution files
  for (const auto& level : config["Explorer Configuration"]["Level Data"])
  {
   level_t lvlStruct;
   lvlStruct.levelId = level["Level Id"].get<uint8_t>();

   if (lvlStruct.levelId != 15)
   {
    lvlStruct.solutionFile = level["Solution File"].get<std::string>();
    auto statusSolution = loadStringFromFile(lvlStruct.moveSequence, lvlStruct.solutionFile.c_str());
    if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", lvlStruct.solutionFile.c_str());
    lvlStruct.moveListStrings = split(lvlStruct.moveSequence, ' ');
    lvlStruct.sequenceLength = lvlStruct.moveListStrings.size();
    for (size_t i = 0; i < lvlStruct.sequenceLength; i++) lvlStruct.moveList.push_back(EmuInstance::moveStringToCode(lvlStruct.moveListStrings[i]));
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

   lvlStruct.stateFile = level["State File"].get<std::string>();
   _emuInstance->loadStateFile(lvlStruct.stateFile);
   _gameInstance->popState(lvlStruct.stateData);
   lvlStruct.RNGOffset = level["RNG Offset"].get<uint8_t>();
   levels.push_back(lvlStruct);
  }


  std::vector<rng_t> sampleCutsceneRNGs({0x8d6453c7,
  0x3e3fb958,
  0xbad8ef0d,
  0xaa5360f6,
  0x3f056963,
  0x1fd941e4,
  0xbc7f9849,
  0x85b5d7a2,
  0x1a68753f,
  0x9250d1b0,
  0x89fa7dc5,
  0x95fa0267,
  0x9d79ce78,
  0x997090b2,
  0x205b08f,
  0x116d5040,
  0x629960ba,
  0x7cb91fb7,
  0xbcb003e9,
  0x819fdc2,
  0xd3204a4c,
  0xa2b39551,
  0x3fdcb653,
  0xd5dd6494,
  0x497c37ae,
  0xd25e63fb,
  0xfa2b0a4d,
  0xa3598136,
  0xff30960,
  0x1cf0de35,
  0xb610e657,
  0xedbd0428,
  0xc2dcf3e2,
  0xb2237a7f,
  0x372e74f1,
  0x4e7171ea,
  0x92c39fb4,
  0x40677359,
  0xb542c49b,
  0x8ece42fc,
  0x3aae4956,
  0x105f6743,
  0x546967d5,
  0x911903de,
  0xbe22e148,
  0x2c40f8bd,
  0xb089151f,
  0x9f5a7510,
  0x43a42c0a,
  0x2076e47,
  0xa21b46f9,
  0xf9d0ab12,
  0xf3b4e21c,
  0x679f5261,
  0x9cb4dbe3,
  0x5a062e64,
  0x79a7cffe,
  0xca73138b,
  0xfbf7b65d,
  0x5cd61b86,
  0xeac4f630,
  0x3000a445,
  0x51d55ce7,
  0xb4e542f8,
  0x418ba932,
  0x93601b0f,
  0xe14a9a01,
  0x7858493a,
  0x9619b184,
  0xabe35269,
  0xf2fc1c2b,
  0x1fcec6cc,
  0x99ff6ba6,
  0x533688d3,
  0x2d5c15e5,
  0xb1eb682e,
  0xed08e818,
  0x25aa00cd,
  0xa040ddaf,
  0x6df10de0,
  0x71c40b5a,
  0xa58ca0d7,
  0x23be58a8,
  0x505fe19d,
  0x67488bc6,
  0xf705a573,
  0xfea5ac34,
  0xfdb621d9,
  0x4d1a1572,
  0xa7f5b04f,
  0x5f125700,
  0xeafb2e55});

  uint32_t initialRNG = 0x273A9EBB;
  std::vector<uint8_t> cutsceneDelayCounts({  0,  0, 40,  0, 18,  0,  1,  0,  1, 62,  0,  0,  6,  0,  0 });
  std::vector<uint8_t> endDelayCounts(     {  0, 13,  0,  0,  0,  0,  0,  0,  1,  2,  0,  0,  0,  0,  0 });

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
//  if (copyprot_plac != 4)
//  {
//   printf("Bad Seed!\n");
//   exit(0);
//  }

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
    for (uint8_t k = 0; k < cutsceneDelays[i][q]; k++) gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);

//    if (levels[i].levelId == 9) if (gameState.random_seed != sampleCutsceneRNGs[q])
//    {
//     printf("RNGs %u Diverge 0x%08X != 0x%08X\n", q+30, gameState.random_seed, sampleCutsceneRNGs[q]);
//     exit(0);
//    }

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

   const auto& lvlSource = levels[i].levelId != 15 ? levels[i] : copyProtSolutions[copyprot_plac];
   uint16_t lastUpPosition = 0; for (uint16_t pos = 0; pos < lvlSource.sequenceLength; pos++) if (lvlSource.moveListStrings[pos] == "....U..") lastUpPosition = pos;
   uint16_t lastUpDiff = lvlSource.sequenceLength - lastUpPosition;

   for (int j = 0; j < lvlSource.sequenceLength && gameState.current_level == levels[i].levelId; j++)
   {
    printf("Level %u, Step %04u/%04u: - RNG: 0x%08x, Loose: %u, Move: '%s', Room: %u", gameState.current_level, j+1, lvlSource.sequenceLength-1, gameState.random_seed, gameState.last_loose_sound, lvlSource.moveListStrings[j].c_str(), gameState.Kid.room);
    if (endDelayCounts[i] > 0) if (j == lastUpPosition + endDelayCounts[i]) printf(" <<---- New U");
    printf("\n");
    _gameInstance->advanceGameState(lvlSource.moveList[j]);

    if (gameState.hitp_curr == 0)
    {
     printf("Death detected!\n");
     exit(0);
    }
    //printf("Step %u - Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", j, gameState.current_level, lvlSource.moveList[j].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
   }

   // Adding end delays
   for (ssize_t q = 0; q < endDelayCounts[i]; q++)
   {
    auto prev = gameState.random_seed;
    for (uint8_t k = 0; k < endWaitDelays[i][q]; k++) gameState.random_seed = _emuInstance->advanceRNGState(gameState.random_seed);
    printf("Do End Delay: 0x%08x %s -> 0x%08x\n", prev, (q == endDelayCounts[i] - lastUpDiff) ? " <<---- New U" : "", gameState.random_seed);
   }

//   if (i == 2) exit(0);
  }

  return 0;
}
