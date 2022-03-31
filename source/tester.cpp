#include <unistd.h>
#include <stdlib.h>
#include <set>
#include "argparse.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include <parallel_hashmap/phmap.h>

// Configuration for parallel hash sets
#define SETNAME phmap::parallel_flat_hash_set
#define SETEXTRAARGS , phmap::priv::hash_default_hash<V>, phmap::priv::hash_default_eq<V>, std::allocator<V>, 4, std::mutex
template <class V> using HashSetT = SETNAME<V SETEXTRAARGS>;
using hashSet_t = HashSetT<uint32_t>;

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

  // Initializing emulator
  auto emuInstance = new EmuInstance(config["Emulator Configuration"]);

  // Initializing game state
  GameInstance gameInstance(emuInstance, config["Game Configuration"]);

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
   emuInstance->loadStateFile(lvlStruct.stateFile);
   gameInstance.popState(lvlStruct.stateData);
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
  uint32_t maxRNG = 100000;
  auto currentLastLooseSound = 0;
  hashSet_t goodRNGSet;
  uint8_t maxLevel = 0;

  for (uint32_t rngState = 0; rngState < maxRNG; rngState++)
  {
   gameState.random_seed = rngState;
   init_copyprot();
   if (copyprot_plac == posCopyProt) goodRNGSet.insert(rngState);
  }
  printf("Success Counter: %lu/%u (%.2f%%)\n", goodRNGSet.size(), maxRNG, ((double)goodRNGSet.size() / (double)maxRNG)*100.0);


  for (size_t i = 0; i < levels.size(); i++)
  {
   // Adding cutscene rng states
   if (levels[i].cutsceneRNGRate > 0)
   {
    hashSet_t tmpRNGSet;
    for (const uint32_t rng : goodRNGSet)
    {
     gameState.random_seed = rng;
     for (size_t q = 0; q < 23; q++) // 2 seconds of cutscenes
     {
      for (uint8_t k = 0; k < levels[i].cutsceneRNGRate; k++) gameState.random_seed = emuInstance->advanceRNGState(gameState.random_seed);
      tmpRNGSet.insert(gameState.random_seed);
     }
    }

    goodRNGSet = tmpRNGSet;
   }

   hashSet_t tmpRNGSet;

   for (const uint32_t currentRNG : goodRNGSet)
   {
    gameInstance.pushState(levels[i].stateData);
    gameState.random_seed = currentRNG;
    gameState.last_loose_sound = currentLastLooseSound;

    for (uint8_t k = 0; k < levels[i].RNGOffset; k++) gameState.random_seed = emuInstance->advanceRNGState(gameState.random_seed);

    for (int j = 0; j < levels[i].sequenceLength && gameState.current_level == levels[i].levelId; j++)
    {
     gameInstance.advanceState(levels[i].moveList[j]);
     printf("Step %u - Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", j, gameState.current_level, levels[i].moveList[j].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
    }

    if (gameState.current_level == levels[i].levelId || gameState.current_level == 13) if (i > maxLevel) { maxLevel = i; break; }
    if (gameState.current_level != levels[i].levelId) tmpRNGSet.insert(gameState.random_seed);
   }

   printf("Level %u, Success Counter: %lu/%lu (%.2f%%)\n", levels[i].levelId, tmpRNGSet.size(), goodRNGSet.size(), ((double)tmpRNGSet.size() / (double)goodRNGSet.size())*100.0);
   if (tmpRNGSet.size() == 0) break;
   goodRNGSet = tmpRNGSet;
   currentLastLooseSound = gameState.last_loose_sound;
  }

  printf("Max Level: %u\n", levels[maxLevel].levelId);
}
