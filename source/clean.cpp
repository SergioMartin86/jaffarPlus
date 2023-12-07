#include <atomic>
#include "argparse.hpp"
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include "omp.h"

int main(int argc, char *argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-clean", "1.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  program.add_argument("solutionFile")
    .help("path to the solution sequence file (.sol) to reproduce.")
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

  // If sequence file defined, load it and play it
  std::string solutionFile = program.get<std::string>("solutionFile");

  // Checking whether it contains the rules field
  if (isDefined(config, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rules' key.\n");

  // Checking whether it contains the emulator configuration field
  if (isDefined(config, "Emulator Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Emulator Configuration' key.\n");

  // Checking whether it contains the Game configuration field
  if (isDefined(config, "Game Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Configuration' key.\n");

  // Getting number of openMP threads
  int _threadCount = omp_get_max_threads();

  // Creating game instances, one per openMP thread
  std::vector<GameInstance*> gameInstances;
  std::vector<EmuInstance*> emuInstances;

  // Resizing containers based on thread count
  gameInstances.resize(_threadCount);
  emuInstances.resize(_threadCount);

  // Statistics
  std::atomic<size_t> totalInputCount = 0;
  std::atomic<size_t> cleanedInputCount = 0;

  // Storage for hash sequence
  std::vector<_uint128_t> hashSequence;

  // Storage for new movie inputs
  std::vector<INPUT_TYPE> newInputs;

  // Initializing thread-specific instances
  #pragma omp parallel
  {
   // Getting thread id
   int threadId = omp_get_thread_num();

   // Doing this as a critical section so not all threads try to access files at the same time
   #pragma omp critical
   {
    // Creating game and emulator instances, and parsing rules
    emuInstances[threadId] = new EmuInstance(config["Emulator Configuration"]);
    gameInstances[threadId] = new GameInstance(emuInstances[threadId], config["Game Configuration"]);
    gameInstances[threadId]->parseRules(config["Rules"]);
   }

   // Move sequence and information
   std::string moveSequence;
   std::vector<std::string> moveList;
   size_t sequenceLength;

   // Loading solution file
   auto statusSolution = loadStringFromFile(moveSequence, solutionFile.c_str());
   if (statusSolution == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n", solutionFile.c_str());

   // Getting move list
   moveList.clear();
   moveList = split(moveSequence, ' ');

   // Getting sequence size
   sequenceLength = moveList.size();

   // Computing input size (in bits)
   size_t inputSize = sizeof(INPUT_TYPE) * 8;

   // Storage initial state
   uint8_t initialState[_STATE_DATA_SIZE_TRAIN];
   gameInstances[threadId]->popState(initialState);

   #pragma omp master
   {
    // Resizing new inputs vector
    newInputs.resize(sequenceLength);

    printf("Pre-computing hash sequence\n");

    // Running the normal sequence to get hash data
    for (size_t curFrame = 0; curFrame <= sequenceLength; curFrame++)
    {
     // Getting hash
     _uint128_t curHash = gameInstances[threadId]->computeHash(curFrame);
     hashSequence.push_back(curHash);

     // Advancing frame
     if (curFrame < sequenceLength) gameInstances[threadId]->advanceStateString(moveList[curFrame]);
    }
   }

   #pragma omp barrier

   // Storage for current state
   uint8_t currentState[_STATE_DATA_SIZE_TRAIN];

   // Rewinding to the initial frame
   gameInstances[threadId]->pushState(initialState);

   // This variable is used to advance the thread to the correct frame
   size_t threadFrame = 0;

   // Starting frame iteration
   #pragma omp for schedule(dynamic,16)
   for (size_t curFrame = 0; curFrame < sequenceLength; curFrame++)
   {
    printf("Processing Frame %lu/%lu\n", curFrame, sequenceLength);

    // Advancing frame if we are behind
    while (threadFrame < curFrame) gameInstances[threadId]->advanceStateString(moveList[threadFrame++]);

    // Storing current state
    gameInstances[threadId]->popState(currentState);

    // Converting string input to numerical code
    auto move = emuInstances[threadId]->moveStringToCode(moveList[curFrame]);

    // Storage for possibly reduced move
    auto newMove = move;

    // Iterating over non-zero inputs
    INPUT_TYPE inputPos = 0b00000001;

    for (size_t i = 0; i < inputSize; i++)
    {
     // Running check if input is non-null
     if (newMove & inputPos)
     {
      // Keeping statistics
      totalInputCount++;

      // Reloading current frame
      gameInstances[threadId]->pushState(currentState);

      // Removing input
      auto proposalMove = newMove ^ inputPos;

      // Running proposal move
      gameInstances[threadId]->advanceGameState(proposalMove);

      // Flag to indicate whether the proposal move was successful
      bool isSuccessful = true;

      // Checking hashes until end of movie
      for (size_t tmpFrame = curFrame+1; tmpFrame < sequenceLength; tmpFrame++)
      {
       // Getting new hash
       _uint128_t curHash = gameInstances[threadId]->computeHash(tmpFrame);

       // Checking the new hash and next official frame's hash coincide
       bool hashesCoincide = curHash == hashSequence[tmpFrame];

       // If hashes do not coincide, end process
       if (hashesCoincide == false)
       {
        printf("Fail %lu/%lu/%lu\n", curFrame, tmpFrame, sequenceLength);
        isSuccessful = false; break;
       }

       // Otherwise, keep running the rest of the frames
       gameInstances[threadId]->advanceStateString(moveList[tmpFrame]);
      }

      // If successful, store it as new new move
      if (isSuccessful)
      {
       newMove = proposalMove;
       cleanedInputCount++;
       printf("Success\n");
      }
     }

     // Moving input position
     inputPos <<= 1;
    }

    // Storing new move
    newInputs[curFrame] = newMove;

    // Reloading current frame
    gameInstances[threadId]->pushState(currentState);
   }

   #pragma omp barrier

   #pragma omp master
   {
    // Printing statistics
    printf("Cleaned Inputs: %lu/%lu\n", cleanedInputCount.load(), totalInputCount.load());

    printf("Generating new sequence...\n");

    // Generating new sequence
    std::string newSequence;
    for (size_t curFrame = 0; curFrame < sequenceLength; curFrame++)
    {
     // Getting new move string
     std::string newMove = emuInstances[threadId]->moveCodeToString(newInputs[curFrame]);

     // Appending new move in the sequence
     newSequence += newMove + std::string("\n");
    }

    // Dumping new sequence
    std::string outputFilePath = solutionFile + std::string(".cleaned");
    saveStringToFile(newSequence, outputFilePath.c_str());
    printf("Output file saved: '%s'\n", outputFilePath.c_str());
  }
 }
}
