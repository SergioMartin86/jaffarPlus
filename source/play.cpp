#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <map>
#include "argparse.hpp"
#include "playbackInstance.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"

// Function to check for keypress taken from https://github.com/ajpaulson/learning-ncurses/blob/master/kbhit.c
int kbhit()
{
  int ch, r;

  // turn off getch() blocking and echo
  nodelay(stdscr, TRUE);
  noecho();

  // check for input
  ch = getch();
  if (ch == ERR) // no input
    r = FALSE;
  else // input
  {
    r = TRUE;
    ungetch(ch);
  }

  // restore block and echo
  echo();
  nodelay(stdscr, FALSE);

  return (r);
}

int getKeyPress()
{
  while (!kbhit())
  {
    usleep(100000ul);
    refresh();
  }
  return getch();
}


void loadSolutionFile(
                      std::map<_uint128_t, uint16_t>& hashMap,
                      bool& hashCollisionFound,
                      uint16_t& hashCollisionStep,
                      uint16_t& hashCollisionPrev,
                      std::vector<uint8_t*>& stateSequence,
                      std::vector<bool*>& ruleStatusSequence,
                      bool& failConditionFound,
                      uint16_t& failConditionStep,
                      std::string& moveSequence,
                      std::vector<std::string>& moveList,
                      int& sequenceLength,
                      const std::string& solutionFile,
                      const std::string& stateFile,
                      GameInstance& gameInstance,
                      const size_t ruleCount,
                      std::vector<std::string>& unpackedMoveSequence,
                      bool& unsupportedMoveFound,
                      uint16_t& unsupportedMoveStep
                      )
{
 // Clearing hash map
 hashMap.clear();
 hashCollisionFound = false;
 hashCollisionStep = 0;
 hashCollisionPrev = 0;

 // Loading solution file
 auto statusSolution = loadStringFromFile(moveSequence, solutionFile.c_str());
 if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", solutionFile.c_str());

 // Getting move list
 moveList.clear();
 moveList = split(moveSequence, ' ');

 // Clearing unpacked move sequence
 unpackedMoveSequence.clear();

 // Getting sequence size
 sequenceLength = moveList.size();
 moveList.push_back(".");
 moveList.push_back(".");

 // Flag to indicate whether a fail condition was met
 failConditionFound = false;
 failConditionStep = 0;

 // Flag to indicate whether a move is given that is not otherwise supported
 unsupportedMoveFound = false;
 unsupportedMoveStep = 0;

 // Saving initial frame
 uint8_t* state = (uint8_t*) malloc(_STATE_DATA_SIZE_PLAY);
 gameInstance.loadStateFile(stateFile);
 gameInstance.updateDerivedValues();
 gameInstance.popState(state);

 // Clearing state sequence
 stateSequence.clear();
 stateSequence.push_back(state);

 // Saving initial rule status
 bool* rulesStatus = (bool*) calloc(ruleCount, sizeof(bool));
 gameInstance.evaluateRules(rulesStatus);
 ruleStatusSequence.push_back(rulesStatus);

 // Checking if fail condition was met
 auto stateType = gameInstance.getStateType(rulesStatus);
 if (failConditionFound == false && stateType == f_fail)
 {
  failConditionFound = true;
  failConditionStep = 0;
 }

 // Adding current hash to the set
 _uint128_t curHash = gameInstance.computeHash();
 hashMap[curHash] = 0;

 // Iterating move list in the sequence
 for (ssize_t i = 0; i < (ssize_t)moveList.size(); i++)
 {
  // Getting possible moves
  auto possibleMoves = gameInstance.getPossibleMoves(rulesStatus);
  bool moveFound = false;
  for (const auto& move : possibleMoves) if (EmuInstance::moveStringToCode(move) == EmuInstance::moveStringToCode(moveList[i])) moveFound = true;
  if (moveFound == false)
  if (unsupportedMoveFound == false)
  if (i < sequenceLength)
  {
   unsupportedMoveFound = true;
   unsupportedMoveStep = i-1;
  }

  // Advancing state
  auto newMoveList = gameInstance.advanceStateString(moveList[i]);
  std::vector<std::string> newMoveListString;
  for (const auto& move : newMoveList) newMoveListString.push_back(EmuInstance::moveCodeToString(move));

  // Storing full unpacked sequence
  unpackedMoveSequence.insert(unpackedMoveSequence.end(), newMoveListString.begin(), newMoveListString.end());

  // Adding current hash to the set
  _uint128_t curHash = gameInstance.computeHash();
  if (hashCollisionFound == false && hashMap.contains(curHash))
  {
   hashCollisionStep = i;
   hashCollisionPrev = hashMap[curHash];
   hashCollisionFound = true;
  }
  hashMap[curHash] = i;

  // Storing new state
  state = (uint8_t*) malloc(_STATE_DATA_SIZE_PLAY);
  gameInstance.popState(state);
  stateSequence.push_back(state);

  // Storing new rules
  rulesStatus = (bool*) malloc(ruleCount * sizeof(bool));
  memcpy(rulesStatus, ruleStatusSequence[i], ruleCount * sizeof(bool));
  gameInstance.evaluateRules(rulesStatus);
  ruleStatusSequence.push_back(rulesStatus);

  // Checking if fail condition was met
  auto stateType = gameInstance.getStateType(rulesStatus);
  if (failConditionFound == false && stateType == f_fail)
  {
   failConditionFound = true;
   failConditionStep = i;
  }
 }
}


int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-play", "2.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  program.add_argument("solutionFile")
    .help("path to the solution sequence file (.sol) to reproduce.")
    .required();

  program.add_argument("--reproduce")
    .help("Plays the entire sequence without interruptions and exit at the end.")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("--disableRender")
    .help("Do not render game window.")
    .default_value(false)
    .implicit_value(true);

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

  // If sequence file defined, load it and play it
  std::string solutionFile = program.get<std::string>("solutionFile");

  // Checking whether it contains the rules field
  if (isDefined(config, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rules' key.\n");

  // Checking whether it contains the emulator configuration field
  if (isDefined(config, "Emulator Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Emulator Configuration' key.\n");

  // Checking whether it contains the Game configuration field
  if (isDefined(config, "Game Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Configuration' key.\n");

  // Checking whether it contains the Game configuration field
  if (isDefined(config, "Playback Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Playback Configuration' key.\n");

  // Getting reproduce flag
  bool isReproduce = program.get<bool>("--reproduce");
  bool refreshOnFinish = isReproduce;

  // Getting reproduce flag
  bool disableRender = program.get<bool>("--disableRender");

  // Initializing ncurses screen
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  // This storage will indicate whether a repeated hash was found
  std::map<_uint128_t, uint16_t> hashMap;
  bool hashCollisionFound;
  uint16_t hashCollisionStep;
  uint16_t hashCollisionPrev;

  // Printing info
  printw("[Jaffar] Playing sequence file: %s\n", solutionFile.c_str());
  printw("[Jaffar] Generating frame sequence...\n");

  refresh();

  // Initializing emulator
  auto emuInstance = new EmuInstance(config["Emulator Configuration"]);
  std::string stateFile = config["Emulator Configuration"]["State File"].get<std::string>();

  // Initializing game state
  GameInstance gameInstance(emuInstance, config["Game Configuration"]);
  gameInstance.parseRules(config["Rules"]);

  // Storing rule count
  size_t ruleCount = gameInstance._rules.size();

  // Initializing playback instance
  printw("[Jaffar] Opening Game window...\n");
  PlaybackInstance playbackInstance(&gameInstance, config["Playback Configuration"]);

  // Storage for sequence frames and rule evaluation
  std::vector<uint8_t*> stateSequence;
  std::vector<bool*> ruleStatusSequence;

  // Flag to indicate whether a fail condition was met
  bool failConditionFound;
  uint16_t failConditionStep;

  // Flag to indicate whether a move is unsupported by the bo
  bool unsupportedMoveFound;
  uint16_t unsupportedMoveStep;

  // Move sequence and information
  std::string moveSequence;
  std::vector<std::string> moveList;
  std::vector<std::string> unpackedMoveSequence;
  int sequenceLength;

  // Variable for current step in view
  int currentStep = 0;

  // Loading solution
  loadSolutionFile(hashMap, hashCollisionFound, hashCollisionStep, hashCollisionPrev, stateSequence, ruleStatusSequence, failConditionFound, failConditionStep, moveSequence, moveList, sequenceLength, solutionFile, stateFile, gameInstance, ruleCount, unpackedMoveSequence, unsupportedMoveFound, unsupportedMoveStep);

  // Flag to continue running playback
  bool continueRunning = true;

  // Flag to display frame information
  bool showFrameInfo = true;

  // Rule status
  bool* rulesStatus = (bool*) calloc(ruleCount, sizeof(bool));

  // Interactive section
  int command;
  do
  {
   // Loading requested step
   gameInstance.pushState(stateSequence[currentStep]);
   gameInstance.evaluateRules(rulesStatus);

   // Updating display
   if (disableRender == false) playbackInstance.renderFrame(currentStep, moveList[currentStep]);

   // Getting possible moves
   auto possibleMoves = gameInstance.getPossibleMoves(rulesStatus);

   // Showing frame information
   if (showFrameInfo)
   {
     clear();
     printw("[Jaffar] ----------------------------------------------------------------\n");
     printw("[Jaffar] Current Step #: %d / %d\n", currentStep, sequenceLength);
     printw("[Jaffar]  + Move: %s\n", moveList[currentStep].c_str());
     printw("[Jaffar]  + Possible Moves: '%s'", possibleMoves[0].c_str()); for (size_t i = 1; i < possibleMoves.size(); i++) printw(", '%s'", possibleMoves[i].c_str()); printw("\n");
     printw("[Jaffar]  + Hash Collision Found:   %s (%u -> %u)\n", hashCollisionFound ? "True" : "False", hashCollisionStep+1, hashCollisionPrev+1);
     printw("[Jaffar]  + Fail State Found:       %s (%u)\n", failConditionFound ? "True" : "False", failConditionStep+1);
     printw("[Jaffar]  + Unsupported Move Found: %s (%u)\n", unsupportedMoveFound ? "True" : "False", unsupportedMoveStep+1);
     gameInstance.printStateInfo(ruleStatusSequence[currentStep]);
     printw("[Jaffar] Commands: n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 | k: -1000 | i: +1000 | g: set RNG | s: quicksave | p: play | d: unpack | q: quit\n");
     playbackInstance.printPlaybackCommands();
     refresh();
   }

   // Resetting show frame info flag
   showFrameInfo = true;

   // If we're reproducing do not have an interactive interface
   if (isReproduce)
   {
    currentStep++;
    if (currentStep > sequenceLength)
    {
     if (refreshOnFinish)
     {
      sleep(2);
      loadSolutionFile(hashMap, hashCollisionFound, hashCollisionStep, hashCollisionPrev, stateSequence, ruleStatusSequence, failConditionFound, failConditionStep, moveSequence, moveList, sequenceLength, solutionFile, stateFile, gameInstance, ruleCount, unpackedMoveSequence, unsupportedMoveFound, unsupportedMoveStep);
      currentStep = 0;
     }
     else
     {
      isReproduce = false;
     }
    }

    continue;
   }

   // Get command
   command = getKeyPress();

   // Parsing command
   showFrameInfo = playbackInstance.parseCommand(command, stateSequence[currentStep]);

   // Advance/Rewind commands
   if (command == 'n') currentStep = currentStep - 1;
   if (command == 'm') currentStep = currentStep + 1;
   if (command == 'h') currentStep = currentStep - 10;
   if (command == 'j') currentStep = currentStep + 10;
   if (command == 'y') currentStep = currentStep - 100;
   if (command == 'u') currentStep = currentStep + 100;
   if (command == 'k') currentStep = currentStep - 1000;
   if (command == 'i') currentStep = currentStep + 1000;

   // Correct current step if requested more than possible
   if (currentStep < 0) currentStep = 0;
   if (currentStep > sequenceLength) currentStep = sequenceLength;

   // Quicksave creation command
   if (command == 's')
   {
     // Storing state file
     std::string saveFileName = "jaffar.state";
     gameInstance.saveStateFile(saveFileName);
     printw("[Jaffar] Saved state to %s\n", saveFileName.c_str());

     // Do no show frame info again after this action
     showFrameInfo = false;
   }

   // Storing unpacked solution
   if (command == 'd')
   {
     // Storing replay file
     std::string sequenceFileName = solutionFile + std::string(".translated");

     // Unpacking full move sequence
     std::string unpackedSequence;
     for (const auto& move : unpackedMoveSequence) { unpackedSequence.append(move); unpackedSequence.append("\n"); }

     // Storing file
     saveStringToFile(unpackedSequence, sequenceFileName.c_str());

     printw("[Jaffar] Saved unpacked sequence to %s\n", sequenceFileName.c_str());

     // Do no show frame info again after this action
     showFrameInfo = false;
   }

   // RNG setting command
   if (command == 'g')
   {
     // Obtaining RNG state
     printw("Enter new RNG state: ");

     // Setting input as new rng
     char str[80];
     getstr(str);
     auto newRNG = std::stoull(str);
     gameInstance.setRNGState(newRNG);

     // Replacing current sequence
     gameInstance.popState(stateSequence[currentStep]);
   }

   // Start playback from current point
   if (command == 'p') isReproduce = true;

   // Start playback from current point
   if (command == 'q') continueRunning = false;

  } while (continueRunning);

  // Ending ncurses window
  endwin();
}

