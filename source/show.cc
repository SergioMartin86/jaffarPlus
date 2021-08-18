#include "argparse.hpp"
#include "nlohmann/json.hpp"
#include "frame.h"
#include "blastemInstance.h"
#include "utils.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
  // Defining arguments
  argparse::ArgumentParser program("jaffar-show", "1.0");

  program.add_argument("romFile")
    .help("Specifies the path to the genesis rom file (.bin) from which to start.")
    .required();

  program.add_argument("savFile")
    .help("Specifies the path to the blastem state file (.state) from which to start.")
    .required();

  program.add_argument("stateFile")
    .help("Specifies the path to the jaffar state file (.sav) to load.")
    .required();

  // Parsing command line
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
    fprintf(stderr, "[Jaffar] Error parsing command line arguments: %s\n%s", err.what(), program.help().str().c_str());
    exit(-1);
  }

  double updateEverySeconds = 1.0;
  if (const char *updateEverySecondsEnv = std::getenv("JAFFAR2_SHOW_UPDATE_EVERY_SECONDS"))
    updateEverySeconds = std::stof(updateEverySecondsEnv);
  else
    fprintf(stderr, "[Jaffar] Warning: Environment variable JAFFAR2_SHOW_UPDATE_EVERY_SECONDS is not defined. Using default: 1.0s\n");

  // Getting romfile path
  auto romFilePath = program.get<std::string>("romFile");

  // Getting savefile path
  std::string saveFilePath = program.get<std::string>("savFile");

  // Getting savefile path
  std::string stateFilePath = program.get<std::string>("stateFile");

  // Loading save file contents
  std::string saveString;
  bool status = loadStringFromFile(saveString, saveFilePath.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not load save state from file: %s\n", saveFilePath.c_str());

  // Initializing showing SDLPop Instance
  blastemInstance showBlastem;
  showBlastem.initialize(romFilePath.c_str(), saveFilePath.c_str(), false, false);

  // Constant loop of updates
  while (true)
  {
    // Reloading save file
    uint8_t* saveData = (uint8_t*) malloc (sizeof(uint8_t) * _STATE_DATA_SIZE);
    bool status = loadBinFromFile(saveData, _STATE_DATA_SIZE, stateFilePath.c_str());

    if (status == true)
    {
      // Loading data into state
      showBlastem.loadState(saveData);

      // Drawing frame
      showBlastem.redraw();
    }

    // Freeing mem
    free(saveData);

    // Adding sanity pause
    useconds_t waitLength = std::floor(updateEverySeconds * 1000000.0);
    usleep(waitLength);
  }
}
