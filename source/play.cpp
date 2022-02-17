#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
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
  std::string moveSequence;
  std::string solutionFile = program.get<std::string>("solutionFile");
  auto statusSolution = loadStringFromFile(moveSequence, solutionFile.c_str());
  if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n%s \n", solutionFile.c_str(), program.help().str().c_str());

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
  bool exitOnFinish = isReproduce;

  // Initializing ncurses screen
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  // Getting move list
  const auto moveList = split(moveSequence, ' ');

  // Getting sequence size
  const int sequenceLength = moveList.size()-1;

  // Printing info
  printw("[Jaffar] Playing sequence file: %s\n", solutionFile.c_str());
  printw("[Jaffar] Sequence Size: %d moves.\n", sequenceLength);
  printw("[Jaffar] Generating frame sequence...\n");

  refresh();


  // Initializing emulator
  auto emuInstance = new EmuInstance(config["Emulator Configuration"]);

  // Initializing game state
  GameInstance gameInstance(emuInstance, config["Game Configuration"]);
  gameInstance.parseRules(config["Rules"]);

  // Initializing playback instance
  printw("[Jaffar] Opening Game window...\n");
  PlaybackInstance playbackInstance(&gameInstance, config["Playback Configuration"]);

  // Storage for sequence frames and rule evaluation
  std::vector<uint8_t*> stateSequence;
  std::vector<bool*> ruleStatusSequence;

  // Saving initial frame
  uint8_t* state = (uint8_t*) malloc(_STATE_DATA_SIZE);
  gameInstance.popState(state);
  stateSequence.push_back(state);

  // Saving initial rule status
  bool* rulesStatus = (bool*) calloc(_ruleCount, sizeof(bool));
  gameInstance.evaluateRules(rulesStatus);
  ruleStatusSequence.push_back(rulesStatus);

  // Iterating move list in the sequence
  for (int i = 0; i < sequenceLength; i++)
  {
   // Advancing state
   gameInstance.advanceState(moveList[i]);

   // Storing new state
   state = (uint8_t*) malloc(_STATE_DATA_SIZE);
   gameInstance.popState(state);
   stateSequence.push_back(state);

   // Storing new rules
   rulesStatus = (bool*) malloc(_ruleCount * sizeof(bool));
   memcpy(rulesStatus, ruleStatusSequence[i], _ruleCount * sizeof(bool));
   gameInstance.evaluateRules(rulesStatus);
   ruleStatusSequence.push_back(rulesStatus);
  }

  // Variable for current step in view
  int currentStep = 0;

  // Flag to display frame information
  bool showFrameInfo = true;

  // Interactive section
  int command;
  do
  {
   // Loading requested step
   gameInstance.pushState(stateSequence[currentStep]);

   // Updating display
   playbackInstance.renderFrame();

   // Showing frame information
   if (showFrameInfo)
   {
     clear();
     printw("[Jaffar] ----------------------------------------------------------------\n");
     printw("[Jaffar] Current Step #: %d / %d\n", currentStep, sequenceLength);
     printw("[Jaffar]  + Move: %s\n", moveList[currentStep].c_str());
     gameInstance.printStateInfo(ruleStatusSequence[currentStep]);
     printw("[Jaffar] Commands: n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 | s: quicksave | p: play | q: quit\n");
   }

   // Resetting show frame info flag
   showFrameInfo = true;

   // If we're reproducing do not have an interactive interface
   if (isReproduce)
   {
    currentStep++;
    if (currentStep > sequenceLength)
    {
     if (exitOnFinish) { endwin(); return 0; }
     currentStep--; isReproduce = false;
    }
    continue;
   }

   // Get command
   command = getKeyPress();

   // Advance/Rewind commands
   if (command == 'n') currentStep = currentStep - 1;
   if (command == 'm') currentStep = currentStep + 1;
   if (command == 'h') currentStep = currentStep - 10;
   if (command == 'j') currentStep = currentStep + 10;
   if (command == 'y') currentStep = currentStep - 100;
   if (command == 'u') currentStep = currentStep + 100;

   // Correct current step if requested more than possible
   if (currentStep < 0) currentStep = 0;
   if (currentStep > sequenceLength) currentStep = sequenceLength;

   // Quicksave creation command
   if (command == 's')
   {
     // Storing replay file
     std::string saveFileName = "jaffar.state";
     gameInstance.saveStateFile(saveFileName);
     printw("[Jaffar] Saved state to %s\n", saveFileName.c_str());

     // Do no show frame info again after this action
     showFrameInfo = false;
   }

   // Start playback from current point
   if (command == 'p') isReproduce = true;

  } while (command != 'q');

  // Ending ncurses window
  endwin();
}
