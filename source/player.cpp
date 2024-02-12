#include <chrono>
#include <SDL.h>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/extern/argparse/argparse.hpp>
#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/string.hpp>
#include "../emulators/emulatorList.hpp"
#include "../games/gameList.hpp"
#include "game.hpp"
#include "emulator.hpp"
#include "runner.hpp"
#include "playback.hpp"

// Switch to toggle whether to reload the movie on reaching the end of the sequence
bool isReload = false; 

// Switch to toggle whether to reproduce the movie
bool isReproduce = false; 

SDL_Window* launchOutputWindow()
{
  // Opening rendering window
  SDL_SetMainReady();

  // We can only call SDL_InitSubSystem once
  if (!SDL_WasInit(SDL_INIT_VIDEO))
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
      EXIT_WITH_ERROR("Failed to initialize video: %s", SDL_GetError());

  auto window = SDL_CreateWindow("JaffarPlus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WIDTH, DEFAULT_HEIGHT, 0);
  if (window == nullptr) EXIT_WITH_ERROR("Coult not open SDL window");

  return window;
}

void closeOutputWindow(SDL_Window* window)
{
  SDL_DestroyWindow(window);
}

bool mainCycle(const std::string& configFile, const std::string& solutionFile, bool disableRender, SDL_Window* window)
{
  // If sequence file defined, load it and play it
  std::string solutionFileString;
  if (jaffarCommon::loadStringFromFile(solutionFileString, solutionFile) == false) 
    EXIT_WITH_ERROR("[ERROR] Could not find or read from solution sequence file: %s\n%s \n", solutionFile.c_str());

  // If config file defined, read it now
  std::string configFileString;
  if (jaffarCommon::loadStringFromFile(configFileString, configFile) == false) 
   EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str());

  // Parsing configuration file
  nlohmann::json config;
  try { config = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // Getting initial state file from the configuration
  const auto initialStateFilePath = JSON_GET_STRING(config, "Initial State File Path");
  
  // Getting emulator name from the configuration
  const auto emulatorName = JSON_GET_STRING(config, "Emulator");

  // Getting game name from the configuration
  const auto gameName = JSON_GET_STRING(config, "Game");

  // Getting emulator from its name and configuring it
  auto e = jaffarPlus::Emulator::getEmulator(emulatorName, JSON_GET_OBJECT(config, "Emulator Configuration"));

  // Initializing emulator
  e->initialize();

  // Enabling rendering
  if (disableRender == false) e->enableRendering();

  // Initializing emulator's video output
  if (disableRender == false) e->initializeVideoOutput(window);
  
  // If initial state file defined, load it
  if (initialStateFilePath.empty() == false) e->loadStateFile(initialStateFilePath);

  // Getting game from its name and configuring it
  auto g = jaffarPlus::Game::getGame(gameName, e, JSON_GET_OBJECT(config, "Game Configuration"));

  // Initializing game
  g->initialize();

  // Parsing script rules
  g->parseRules(JSON_GET_ARRAY(config, "Rules"));

  // Getting inverse frame rate from game
  const auto frameRate = g->getFrameRate();
  const uint32_t inverseFrameRate = std::round((1.0 / frameRate) * 1.0e+6);

  // Creating runner from game instance
  jaffarPlus::Runner r(g, JSON_GET_OBJECT(config, "Runner Configuration"));

  // Parsing Possible game inputs
  r.parseGameInputs(JSON_GET_ARRAY(config, "Game Inputs"));

  // Getting game state size
  const auto stateSize = r.getStateSize();

  // Getting input sequence
  const auto solutionSequence = jaffarCommon::split(solutionFileString, ' ');

  // Variable for current step in view
  ssize_t currentStep = 0;

  // Getting sequence length
  const ssize_t sequenceLength = solutionSequence.size();

  // Printing information
  LOG("[J+] ********** Creating Playback Sequence **********\n");
  jaffarCommon::refreshTerminal();

  // Instantiating playback object
  jaffarPlus::Playback p(r, solutionSequence);

  // Flag to display frame information
  bool showFrameInfo = true;
 
  // Finalization flag
  bool isFinalize = false;

  // Interactive section
  while (isFinalize == false)
  {
    // Updating display
    if (disableRender == false) p.renderFrame(currentStep);

    // Getting input string
    const auto &inputString = p.getStateInputString(currentStep);

    // Getting input index
    const auto &inputIndex = p.getStateInputIndex(currentStep);

    // Getting state hash
    const auto hash = p.getStateHash(currentStep);

    // Getting state data
    const auto stateData = p.getStateData(currentStep);

    // Printing data and commands
    if (showFrameInfo)
    {
      jaffarCommon::clearTerminal();

      LOG("[J+] ----------------------------------------------------------------\n");
      LOG("[J+] Current Step #:              %lu / %lu\n", currentStep + 1, sequenceLength);
      LOG("[J+] Playback:                    %s\n", isReproduce ? "Playing" : "Stopped");
      LOG("[J+] On Finish:                   %s\n", isReload ? "Auto Reload" : "Stop");
      LOG("[J+] Input:                       %s (0x%X)\n", inputString.c_str(), inputIndex);
      LOG("[J+] State Hash:                  0x%lX%lX\n", hash.first, hash.second);
      LOG("[J+] Emulator Name:              '%s'\n", emulatorName.c_str());
      LOG("[J+] Config File:                '%s'\n", configFile.c_str());
      LOG("[J+] Solution File:              '%s'\n", solutionFile.c_str());
      LOG("[J+] State Size:                  %lu\n", stateSize);
      LOG("[J+] Sequence Length:             %lu\n", sequenceLength);
      LOG("[J+] Frame Rate:                  %f (%u)\n", frameRate, inverseFrameRate);

      // Only print commands if not in reproduce mode
      LOG("[J+] Commands: n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 | k: -1000 | i: +1000 | s: quicksave | p: play | r: autoreload | q: quit\n");

      jaffarCommon::refreshTerminal();
    }

    // Resetting show frame info flag
    showFrameInfo = true;

    // Specifies the command to execute next
    int command = 0;

    // If it's reproducing, 
    if (isReproduce == true)
    {
      // Introducing sleep related to the frame rate
      usleep(inverseFrameRate);

      // Advance to the next frame  
      currentStep++;

      // Get command without interrupting
      command = jaffarCommon::getKeyPress();
    } 

    // If it's not reproducing, grab command with a wait
    if (isReproduce == false) command = jaffarCommon::waitForKeyPress();

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
    
    // If not reloading on finish, simply stop
    if (currentStep > sequenceLength && isReload == false) { currentStep = sequenceLength; isReproduce = false; }

    // If reloading on finish, do it now
    if (currentStep > sequenceLength && isReload == true) break;

    // Quicksave creation command
    if (command == 's')
    {
      // Storing state file
      std::string saveFileName = "quicksave.state";

      std::string saveData;
      saveData.resize(stateSize);
      memcpy(saveData.data(), stateData, stateSize);
      if (jaffarCommon::saveStringToFile(saveData, saveFileName.c_str()) == false) EXIT_WITH_ERROR("[ERROR] Could not save state file: %s\n", saveFileName.c_str());
      LOG("[J+] Saved state to %s\n", saveFileName.c_str());

      // Do no show frame info again after this action
      showFrameInfo = false;
    }

    // Toggles playback from current point
    if (command == 'p') isReproduce = !isReproduce;

    // Toggles Auto Reload
    if (command == 'r') isReload = !isReload;

    // Triggers the exit
    if (command == 'q') isFinalize = true;
  }

  // Close game output 
  if (disableRender == false) r.getGame()->getEmulator()->finalizeVideoOutput();

  // returning false on exit to trigger the finalization
  if (isFinalize) return false;  

  // Otherwise, keep looping
  return true;
}

int main(int argc, char *argv[])
{
 // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  program.add_argument("solutionFile")
    .help("path to the solution sequence file (.sol) to reproduce.")
    .required();

  program.add_argument("--disableRender")
    .help("Do not render game window.")
    .default_value(false)
    .implicit_value(true);
    
  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Parsing config file
  const std::string configFile = program.get<std::string>("configFile");

  // Parsin solution file
  const std::string solutionFile = program.get<std::string>("solutionFile");

  // Getting reproduce flag
  bool disableRender = program.get<bool>("--disableRender");

  // Initializing terminal
  jaffarCommon::initializeTerminal();

  // Creating output window
  auto window = disableRender ? nullptr : launchOutputWindow();

  // Running main cycle
  while (mainCycle(configFile, solutionFile, disableRender, window));

  // Closing output window
  if (disableRender == false) closeOutputWindow(window);

  // Ending ncurses window
  jaffarCommon::finalizeTerminal();
}