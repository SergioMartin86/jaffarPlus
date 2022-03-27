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
   levels.push_back(lvlStruct);
  }

  // Iterating move list in the sequence
  uint16_t currentStep = 0;
  for (size_t i = 0; i < levels.size(); i++)
  {
   gameInstance.pushState(levels[i].stateData);
   for (int j = 0; j < levels[i].sequenceLength; j++)
   {
    gameInstance.advanceState(levels[i].moveList[j]);
    printf("Step %u:%u - Level %u - Move: '%s' - RNG: 0x%08X\n", j, currentStep, gameState.current_level, levels[i].moveList[j].c_str(), gameState.random_seed);
    if (levels[i].levelId == 15)
    {
     gameInstance.printStateInfo(0);
     getchar();
    }
    currentStep++;
   }
  }
}
