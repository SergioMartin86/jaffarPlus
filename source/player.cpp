#include <chrono>
#include <argparse/argparse.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/string.hpp>
#include <gameList.hpp>
#include <emulatorList.hpp>
#include "emulator.hpp"
#include "game.hpp"
#include "playback.hpp"
#include "runner.hpp"

// Prevents the interactive player to stall for a keystroke
bool isUnattended;

// Determines that the reproduction must end on reaching the last step
bool isExitOnEnd;

// Switch to toggle whether to reload the movie on reaching the end of the sequence
bool isReload;

// Switch to toggle whether to reproduce the movie
bool isReproduce;

// Number of frames to skip between renderings
size_t frameskip;

bool mainCycle(jaffarPlus::Runner &r, const std::string &solutionFile, bool disableRender)
{
  // If sequence file defined, load it and play it
  std::string solutionFileString;
  if (jaffarCommon::file::loadStringFromFile(solutionFileString, solutionFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from solution sequence file: %s\n", solutionFile.c_str());

  // Getting input sequence
  const auto solutionSequence = jaffarCommon::string::split(solutionFileString, '\0');

  // Variable for current step in view
  ssize_t currentStep = 0;

  // Getting sequence length
  const ssize_t sequenceLength = solutionSequence.size();

  // Getting inverse frame rate from game
  const auto     frameRate        = r.getGame()->getFrameRate();
  const uint32_t inverseFrameRate = std::round((1.0 / frameRate) * 1.0e+6);

  // Getting game state size
  const auto stateSize = r.getStateSize();

  // Printing information
  jaffarCommon::logger::log("[J+] ********** Creating Playback Sequence **********\n");
  jaffarCommon::logger::refreshTerminal();

  // Instantiating playback instance
  jaffarPlus::Playback p(r);

  // Initializing playback instance
  p.initialize(solutionSequence);

  // Flag to display frame information
  bool showFrameInfo = true;

  // Finalization flag
  bool isFinalize = false;

  // Checking for repeated state hashes
  std::vector<ssize_t> repeatedHashStates;
  for (ssize_t i = 0; i < sequenceLength; i++)
    {
      const auto repeatedHashSteps = p.getStateRepeatedHashSteps(i);
      if (repeatedHashSteps.size() > 0) repeatedHashStates.push_back(i);
    }

  // Interactive section
  while (isFinalize == false)
    {
      // Updating display
      if (disableRender == false)
        if (currentStep % frameskip == 0) p.renderFrame(currentStep);

      // Loading step data
      p.loadStepData(currentStep);

      // Getting input string
      const auto &inputString = p.getStateInputString(currentStep);

      // Getting input index
      const auto &inputIndex = p.getStateInputIndex(currentStep);

      // Getting state hash
      const auto hash = p.getStateHash(currentStep);

      // Getting repeated step hashes (if any)
      const auto repeatedHashSteps = p.getStateRepeatedHashSteps(currentStep);

      // Printing data and commands
      if (showFrameInfo)
        {
          jaffarCommon::logger::clearTerminal();

          jaffarCommon::logger::log("[J+] ----------------------------------------------------------------\n");
          jaffarCommon::logger::log("[J+] Current Step #:              %lu / %lu\n", currentStep + 1, sequenceLength);
          jaffarCommon::logger::log("[J+] Playback:                    %s\n", isReproduce ? "Playing" : "Stopped");
          jaffarCommon::logger::log("[J+] Input:                       %s (0x%X)\n", inputString.c_str(), inputIndex);
          jaffarCommon::logger::log("[J+] On Finish:                   %s\n", isReload ? "Auto Reload" : "Stop");

          jaffarCommon::logger::log("[J+] Repeated Hash Steps:         [ ");
          if (repeatedHashStates.size() < 5)
            for (const auto step : repeatedHashStates) jaffarCommon::logger::log(" %ld ", step);
          else
            {
              for (size_t i = 0; i < 5; i++) jaffarCommon::logger::log(" %ld ", repeatedHashStates[i]);
              jaffarCommon::logger::log(" ... ");
            }
          jaffarCommon::logger::log(" ] \n");

          jaffarCommon::logger::log("[J+] Game Name:                  '%s'\n", r.getGame()->getName().c_str());
          jaffarCommon::logger::log("[J+] Emulator Name:              '%s'\n", r.getGame()->getEmulator()->getName().c_str());
          jaffarCommon::logger::log("[J+] State Hash:                  0x%lX%lX\n", hash.first, hash.second);
          jaffarCommon::logger::log("[J+] State Repeated Hash Steps:   [ ");
          for (const auto step : repeatedHashSteps) jaffarCommon::logger::log(" %lu ", step);
          jaffarCommon::logger::log(" ] \n");
          jaffarCommon::logger::log("[J+] State Size:                  %lu\n", stateSize);
          jaffarCommon::logger::log("[J+] Solution File:              '%s'\n", solutionFile.c_str());
          jaffarCommon::logger::log("[J+] Sequence Length:             %lu\n", sequenceLength);
          jaffarCommon::logger::log("[J+] Frame Rate:                  %f (%u)\n", frameRate, inverseFrameRate);
          p.printInfo();

          // Print General Commands
          jaffarCommon::logger::log("[J+] Commands: n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 | k: -1000 | i: +1000 | s: quicksave | p: play | r: autoreload | q: quit\n");

          // Print any game-specific commands (optional)
          r.getGame()->playerPrintCommands();

          jaffarCommon::logger::refreshTerminal();
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
          command = jaffarCommon::logger::getKeyPress();
      }

      // If it's not reproducing, grab command with a wait
      if (isReproduce == false && isUnattended == false) command = jaffarCommon::logger::waitForKeyPress();

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

      // If reloading on finish, do it now
      if (currentStep > sequenceLength && isReload == true) break;

      // If exiting on finish, do it now
      if (currentStep > sequenceLength && isExitOnEnd == true) break;

      // If not reloading on finish, simply stop
      if (currentStep > sequenceLength)
        {
          currentStep = sequenceLength;
          isReproduce = false;
      }

      // Quicksave creation command
      if (command == 's')
        {
          // Storing state file
          std::string saveFileName = "quicksave.state";

          std::string saveData;
          size_t      stateSize = r.getGame()->getEmulator()->getStateSize();
          saveData.resize(stateSize);
          jaffarCommon::serializer::Contiguous s(saveData.data(), stateSize);
          r.getGame()->getEmulator()->serializeState(s);
          if (jaffarCommon::file::saveStringToFile(saveData, saveFileName.c_str()) == false) JAFFAR_THROW_LOGIC("[ERROR] Could not save state file: %s\n", saveFileName.c_str());
          jaffarCommon::logger::log("[J+] Saved state to %s\n", saveFileName.c_str());

          // Do no show frame info again after this action
          showFrameInfo = false;
      }

      // Toggles playback from current point
      if (command == 'p') isReproduce = !isReproduce;

      // Toggles Auto Reload
      if (command == 'r') isReload = !isReload;

      // Triggers the exit
      if (command == 'q') isFinalize = true;

      // Handle any game-specific commands
      if (command == 't') r.getGame()->playerParseCommand(command);
    }

  // returning false on exit to trigger the finalization
  if (isFinalize) return false;

  // Otherwise, keep looping
  return true;
}

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile").help("path to the Jaffar configuration script (.jaffar) file to run.").required();

  program.add_argument("solutionFile").help("path to the solution sequence file (.sol) to reproduce.").required();

  program.add_argument("--reproduce").help("Starts playing from the start").default_value(false).implicit_value(true);

  program.add_argument("--reload").help("Reloads the solution after reaching the end").default_value(false).implicit_value(true);

  program.add_argument("--exitOnEnd").help("Exits the program upon reaching the last step").default_value(false).implicit_value(true);

  program.add_argument("--unattended").help("Indicates the player not to print the interactive prompt nor wait for inputs").default_value(false).implicit_value(true);

  program.add_argument("--disableRender").help("Do not render game window.").default_value(false).implicit_value(true);

  program.add_argument("--frameskip").help("How many frames to skip between renderings.").default_value(std::string("1")).required();

  // Try to parse arguments
  try
    {
      program.parse_args(argc, argv);
    }
  catch (const std::runtime_error &err)
    {
      JAFFAR_THROW_LOGIC("%s\n%s", err.what(), program.help().str().c_str());
    }

  // Parsing config file
  const std::string configFile = program.get<std::string>("configFile");

  // Parsin solution file
  const std::string solutionFile = program.get<std::string>("solutionFile");

  // Getting reload flag
  bool doReload = program.get<bool>("--reload");

  // Getting reproduce flag
  bool reproduceStart = program.get<bool>("--reproduce");

  // Getting disablerender flag
  bool disableRender = program.get<bool>("--disableRender");

  // Getting exitOnEnd flag
  bool exitOnEnd = program.get<bool>("--exitOnEnd");

  // Getting unattended flag
  bool unattended = program.get<bool>("--unattended");

  // Getting frameskip
  frameskip = std::stoul(program.get<std::string>("--frameskip"));

  // Initializing terminal
  jaffarCommon::logger::initializeTerminal();

  // Setting initial reproduction values
  isReload     = doReload;
  isReproduce  = reproduceStart;
  isExitOnEnd  = exitOnEnd;
  isUnattended = unattended;

  // If config file defined, read it now
  std::string configFileString;
  if (jaffarCommon::file::loadStringFromFile(configFileString, configFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from Jaffar config file: %s\n", configFile.c_str());

  // Parsing configuration file
  nlohmann::json config;
  try
    {
      config = nlohmann::json::parse(configFileString);
    }
  catch (const std::exception &err)
    {
      JAFFAR_THROW_LOGIC("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what());
    }

  // Getting component configurations
  auto emulatorConfig = jaffarCommon::json::getObject(config, "Emulator Configuration");
  auto gameConfig     = jaffarCommon::json::getObject(config, "Game Configuration");
  auto runnerConfig   = jaffarCommon::json::getObject(config, "Runner Configuration");

  // Disabling frameskip, if enabled
  runnerConfig["Frameskip"]["Rate"] = 0;

  // Creating runner from the configuration
  auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);

  // Initializing runner
  r->initialize();

  // Enabling rendering, if required
  if (disableRender == false)
    {
      r->getGame()->getEmulator()->initializeVideoOutput();
      r->getGame()->getEmulator()->enableRendering();
  }

  // Getting game state size
  const auto stateSize = r->getStateSize();

  // Storage for the initial state
  std::string initialState;
  initialState.resize(stateSize);

  // Getting initial state
  jaffarCommon::serializer::Contiguous s(initialState.data(), initialState.size());
  r->serializeState(s);

  // Running main cycle
  bool continueRunning = true;
  while (continueRunning == true)
    {
      // Running main cycle
      continueRunning = mainCycle(*r, solutionFile, disableRender);

      // If the exit-on-end flag is set, then do not repeat reproduction
      if (exitOnEnd == true) break;

      // If the playback repeats, then sleep and restore the initial state
      if (continueRunning == true)
        {
          // If repeating, then wait a bit before repeating to prevent fast repetition of short movies
          sleep(1);

          // Reloading the initial state
          jaffarCommon::deserializer::Contiguous d(initialState.data(), initialState.size());
          r->deserializeState(d);
      }
    }

  // If redering was enabled, finish it now
  if (disableRender == false) r->getGame()->getEmulator()->finalizeVideoOutput();

  // Ending ncurses window
  jaffarCommon::logger::finalizeTerminal();
}