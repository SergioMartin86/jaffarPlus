#include <unistd.h>
#include <stdlib.h>
#include <map>
#include "argparse.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"

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
   levels.push_back(lvlStruct);
  }

  const uint8_t posCopyProt = 4;
  seed_was_init = 1;
  uint32_t successCounter = 0;
  uint32_t maxRNG = 2000;

//  for (uint32_t rngState = 0; rngState < maxRNG; rngState++)
//  {
//   gameState.random_seed = rngState;
//   init_copyprot();
//   if (copyprot_plac == posCopyProt) successCounter++;
//   //printf("RNG: 0x%08X - > 0x%08X - CopyProtPlace: %u\n", rngState, gameState.random_seed, copyprot_plac);
//  }
//  printf("Success Counter: %u/%u (%.2f%%)\n", successCounter, maxRNG, ((double)successCounter / (double)maxRNG)*100.0);
//  exit(0);

//  for (uint32_t rngState = 0; rngState < maxRNG; rngState++)
//  {
   // Iterating move list in the sequence
   uint16_t currentStep = 0;
//   gameState.random_seed = rngState;
//   init_copyprot();

   for (uint32_t initialRngState = 0; initialRngState < maxRNG; initialRngState++)
   {
    auto currentRNG = initialRngState;
    auto currentLastLooseSound = 0;

    for (size_t i = 0; i < levels.size(); i++)
    {
     gameInstance.pushState(levels[i].stateData);
     gameState.random_seed = currentRNG;
     gameState.last_loose_sound = currentLastLooseSound;

     if (gameState.current_level != levels[i].levelId) break;
     for (uint8_t k = 0; k < levels[i].RNGOffset; k++) gameState.random_seed = emuInstance->advanceRNGState(gameState.random_seed);

     for (int j = 0; j < levels[i].sequenceLength && gameState.current_level == levels[i].levelId; j++)
     {
      gameInstance.advanceState(levels[i].moveList[j]);
      //printf("Step %u:%u - Level %u - Move: '%s' - KidRoom: %2u, KidFrame: %2u, RNG: 0x%08X, Loose: %u\n", j, currentStep, gameState.current_level, levels[i].moveList[j].c_str(), gameState.Kid.room, gameState.Kid.frame, gameState.random_seed, gameState.last_loose_sound);
      currentStep++;
     }

     currentRNG = gameState.random_seed;
     currentLastLooseSound = gameState.last_loose_sound;
    }

    printf("Current Level: %u\n", gameState.current_level);
   }
}
