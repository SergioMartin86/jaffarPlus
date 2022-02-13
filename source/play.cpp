#include <ncurses.h>
#include <unistd.h>
#include <SDL.h>
#include "argparse.hpp"
#include "hqn.h"
#include "hqn_gui_controller.h"
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
  // Defining arguments
  argparse::ArgumentParser program("jaffarNES-play", "1.0");

  program.add_argument("romFile")
    .help("Specifies the path to the NES rom file (.nes) to emulate.")
    .required();

  program.add_argument("stateFile")
    .help("Specifies the path to the NES state file (.state) from which to start.")
    .required();

  program.add_argument("solutionFile")
    .help("path to the JaffarNES solution (.sol) file to run.")
    .required();

  program.add_argument("--reproduce")
    .help("Plays the entire sequence without interruptions")
    .default_value(false)
    .implicit_value(true);

  // Parsing command line
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
    fprintf(stderr, "[JaffarNES] Error parsing command line arguments: %s\n%s", err.what(), program.help().str().c_str());
    exit(-1);
  }

  // Getting reproduce path
  bool isReproduce = program.get<bool>("--reproduce");

  // Getting rom file path
  std::string romFilePath = program.get<std::string>("romFile");

  // Getting state file path
  std::string stateFilePath = program.get<std::string>("stateFile");

  // If sequence file defined, load it and play it
  std::string moveSequence;
  std::string solutionFile = program.get<std::string>("solutionFile");
  auto status = loadStringFromFile(moveSequence, solutionFile.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n%s \n", solutionFile.c_str(), program.help().str().c_str());

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
  printw("[JaffarNES] Playing sequence file: %s\n", solutionFile.c_str());
  printw("[JaffarNES] Sequence Size: %d moves.\n", sequenceLength);
  printw("[JaffarNES] Generating frame sequence...\n");

  refresh();

  // Loading state file
  std::string stateFileData;
  status = loadStringFromFile(stateFileData, stateFilePath.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from state file: %s\n%s \n", stateFilePath.c_str(), program.help().str().c_str());

  // Opening rendering window
  SDL_SetMainReady();
  // We can only call SDL_InitSubSystem once
  if (!SDL_WasInit(SDL_INIT_VIDEO))
  {
      if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
      {
          EXIT_WITH_ERROR("Failed to initialize video: %s", SDL_GetError());
      }
  }

  hqn::HQNState hqnState;
  hqnState.loadROM(romFilePath.c_str());
  hqn::GUIController* gui = hqn::GUIController::create(hqnState);
  gui->setScale(2);

  // Initializing replay generating quickNES Instance
  EmuInstance genEmuInstance(hqnState.emu());
  genEmuInstance.loadStateFile(stateFilePath);

  // Initializing game state
  GameInstance gameInstance(&genEmuInstance);

  // Storage for sequence frames
  size_t stateSize;
  hqnState.saveStateSize(&stateSize);
  std::vector<uint8_t*> frameSequenceData;
  std::vector<size_t> frameSequenceSize;

  // Saving initial frame
  uint8_t* state = (uint8_t*) malloc(stateSize);
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

  printw("[JaffarNES] Opening SDLPop window...\n");

  // Variable for current step in view
  int currentStep = 0;

  // Print command list
  if (isReproduce == false)
  {
   printw("[JaffarNES] Available commands:\n");
   printw("[JaffarNES]  n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 \n");
   printw("[JaffarNES]  s: quicksave | r: create replay | q: quit  \n");
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
      printw("[JaffarNES] ----------------------------------------------------------------\n");
      printw("[JaffarNES] Current Step #: %d / %d\n", currentStep, sequenceLength);
      printw("[JaffarNES]  + Move: %s\n", moveList[currentStep].c_str());
      printw("[JaffarNES]  + Hash:                   0x%lX\n", gameInstance.computeHash());
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
      printw("[JaffarNES] Saved state to %s\n", saveFileName.c_str());

      // Do no show frame info again after this action
      showFrameInfo = false;
    }

  } while (command != 'q');

  // Ending ncurses window
  endwin();
}
