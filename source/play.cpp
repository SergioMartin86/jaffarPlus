#include <ncurses.h>
#include <unistd.h>
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

  program.add_argument("--reproduce")
    .help("Plays the entire sequence without interruptions")
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

  nlohmann::json configJs;
  try { configJs = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // If sequence file defined, load it and play it
  std::string moveSequence;
  std::string solutionFile = program.get<std::string>("solutionFile");
  auto statusSolution = loadStringFromFile(moveSequence, solutionFile.c_str());
  if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n%s \n", solutionFile.c_str(), program.help().str().c_str());

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


/*
  hqn::HQNState hqnState;
  hqnState.loadROM(romFilePath.c_str());
  hqn::GUIController* gui = hqn::GUIController::create(hqnState);
  gui->setScale(2);

  // Initializing game state
  GameInstance gameInstance(configJs);

  // Storage for sequence frames
  std::vector<uint8_t*> frameSequenceData;
  std::vector<size_t> frameSequenceSize;

  // Saving initial frame
  uint8_t* state = (uint8_t*) malloc(_STATE_DATA_SIZE);
  size_t savedSize;
  hqnState.saveState(state, stateSize, &savedSize);
  frameSequenceData.push_back(state);
  frameSequenceSize.push_back(savedSize);

  // Iterating move list in the sequence
  for (int i = 0; i < sequenceLength; i++)
  {
    gameInstance.advanceFrame(moveList[i]);

    // Storing new frame
    state = (uint8_t*) malloc(stateSize);
    hqnState.saveState(state, stateSize, &savedSize);
    frameSequenceData.push_back(state);
    frameSequenceSize.push_back(savedSize);
  }

  printw("[Jaffar] Opening Game window...\n");

  // Variable for current step in view
  int currentStep = 0;

  // Print command list
  if (isReproduce == false)
  {
   printw("[Jaffar] Available commands:\n");
   printw("[Jaffar]  n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 | s: quicksave | q: quit\n");
  }

  // Flag to display frame information
  bool showFrameInfo = true;

  // Interactive section
  int command;
  do
  {
   // Loading requested step
   hqnState.loadState((char*)frameSequenceData[currentStep], frameSequenceSize[currentStep]);
   gameInstance.advanceFrame(moveList[currentStep]);

    // Update display
    gui->update(true);

    if (showFrameInfo)
    {
      printw("[Jaffar] ----------------------------------------------------------------\n");
      printw("[Jaffar] Current Step #: %d / %d\n", currentStep, sequenceLength);
      printw("[Jaffar]  + Move: %s\n", moveList[currentStep].c_str());
      gameInstance.printStateInfo();
    }

    // Resetting show frame info flag
    showFrameInfo = true;

    // If we're reproducing do not have an interactive interface
    if (isReproduce)
    {
     usleep(16667);
     currentStep++;
     if (currentStep > sequenceLength) { currentStep--; isReproduce = false; }
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
      genEmuInstance.saveStateFile(saveFileName);
      printw("[Jaffar] Saved state to %s\n", saveFileName.c_str());

      // Do no show frame info again after this action
      showFrameInfo = false;
    }

  } while (command != 'q');

  // Ending ncurses window
  endwin();
  */
}
