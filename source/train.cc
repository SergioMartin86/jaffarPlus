#include "train.h"
#include "argparse.hpp"
#include "utils.h"
#include <omp.h>
#include <unistd.h>
#include <algorithm>

void Train::run()
{
  printf("[JaffarNES] ----------------------------------------------------------------\n");
  printf("[JaffarNES] Launching JaffarNES Version %s...\n", "1.0");
  printf("[JaffarNES] Using configuration file: "); printf("%s ", _scriptFile.c_str()); printf("\n");
  printf("[JaffarNES] Frame size: %lu\n", sizeof(Frame));
  printf("[JaffarNES] Max Frame DB entries: %lu\n", _maxDatabaseSize);

  if (_outputSaveBestSeconds > 0)
  {
    printf("[JaffarNES] Saving best frame every: %.3f seconds.\n", _outputSaveBestSeconds);
    printf("[JaffarNES]  + Savefile Path: %s\n", _outputSaveBestPath.c_str());
    printf("[JaffarNES]  + Solution Path: %s\n", _outputSolutionBestPath.c_str());
  }

  // Sleep for a second to show this message
  sleep(2);

  auto searchTimeBegin = std::chrono::steady_clock::now();      // Profiling
  auto currentStepTimeBegin = std::chrono::steady_clock::now(); // Profiling

  // Storage for termination trigger
  bool terminate = false;

  while (terminate == false)
  {
    // Profiling information
    auto searchTimeEnd = std::chrono::steady_clock::now();                                                            // Profiling
    _searchTotalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(searchTimeEnd - searchTimeBegin).count(); // Profiling

    auto currentStepTimeEnd = std::chrono::steady_clock::now();                                                                 // Profiling
    _currentStepTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentStepTimeEnd - currentStepTimeBegin).count(); // Profiling
    currentStepTimeBegin = std::chrono::steady_clock::now();                                                                    // Profiling

    // Printing search status
    printTrainStatus();

    /////////////////////////////////////////////////////////////////
    /// Main frame processing cycle begin
    /////////////////////////////////////////////////////////////////

    computeFrames();

    /////////////////////////////////////////////////////////////////
    /// Main frame processing cycle end
    /////////////////////////////////////////////////////////////////

    // Advancing step
    _currentStep++;

    // Terminate if all DBs are depleted and no winning rule was found
    _databaseSize = _frameDB.size();
    if (_databaseSize == 0)
    {
      printf("[JaffarNES] Frame database depleted with no winning frames, finishing...\n");
      terminate = true;
    }

    // Terminate if a winning rule was found
    if (_winFrameFound)
    {
      printf("[JaffarNES] Winning frame reached in %u moves, finishing...\n", _currentStep-1);
      terminate = true;
    }

    // Terminate if maximum number of frames was reached
    if (_currentStep > _MAX_MOVELIST_SIZE-1)
    {
      printf("[JaffarNES] Maximum frame number reached, finishing...\n");
      printf("[JaffarNES] To run JaffarNES for more steps, modify this limit in frame.h and rebuild.\n");
      terminate = true;
    }
  }

  // Print winning frame if found
  if (_winFrameFound == true)
  {
    printf("[JaffarNES] Win Frame Information:\n");
    size_t timeStep = _currentStep-1;
    size_t curMins = timeStep / 720;
    size_t curSecs = (timeStep - (curMins * 720)) / 12;
    size_t curMilliSecs = ceil((double)(timeStep - (curMins * 720) - (curSecs * 12)) / 0.012);
    printf("[JaffarNES]  + Solution IGT:  %2lu:%02lu.%03lu\n", curMins, curSecs, curMilliSecs);

    uint8_t winFrameData[_FRAME_DATA_SIZE];
    _winFrame.getFrameDataFromDifference(_sourceFrameData, winFrameData);
    _state[0]->pushState((uint8_t*)winFrameData);
    _state[0]->_nes->printFrameInfo();
    _state[0]->printRuleStatus(_winFrame.rulesStatus);

    #ifndef JAFFARNES_DISABLE_MOVE_HISTORY

    // Print Move History
    printf("[JaffarNES]  + Move List: ");
    for (uint16_t i = 0; i < _currentStep-1; i++)
      printf("%s ", _possibleMoves[_winFrame.getMove(i)].c_str());
    printf("\n");

    #endif
  }

  // Marking the end of the run
  _hasFinalized = true;

  // Stopping show thread
  pthread_join(_showThreadId, NULL);

  // If it has finalized with a win, save the winning frame
  if (_outputSaveBestSeconds > 0.0)
  {
   auto lastFrame = _winFrameFound ? _winFrame : _bestFrame;

   // Saving best frame data
   uint8_t winFrameData[_FRAME_DATA_SIZE];
   lastFrame.getFrameDataFromDifference(_sourceFrameData, winFrameData);
   std::string outputBestStateFilePath = _outputSaveBestPath + std::string(".best.sav");
   _showState->_nes->deserializeState(winFrameData);
   _showState->_nes->saveStateFile(outputBestStateFilePath);

   #ifndef JAFFARNES_DISABLE_MOVE_HISTORY

   // Storing the solution sequence
   std::string solutionString;
   solutionString += _possibleMoves[lastFrame.getMove(0)];
   for (size_t i = 1; i < _currentStep; i++)
    solutionString += std::string(" ") + _possibleMoves[lastFrame.getMove(i)];
   solutionString += std::string(" .");
   std::string outputBestSolutionFilePath = _outputSaveBestPath + std::string(".best.sol");
   saveStringToFile(solutionString, outputBestSolutionFilePath.c_str());

   #endif
  }
}

void Train::computeFrames()
{
  // Initializing counters
  _stepFramesProcessedCounter = 0;
  _newCollisionCounter = 0;
  size_t startHashEntryCount = _hashDB.size();

  // Creating shared database for new frames
  std::vector<std::unique_ptr<Frame>> newFrames;

  // Initializing step timers
  _stepHashCalculationTime = 0.0;
  _stepHashCheckingTime1 = 01.0;
  _stepHashCheckingTime2 = 0.0;
  _stepFrameAdvanceTime = 0.0;
  _stepFrameDeserializationTime = 0.0;

  // Processing frame database in parallel
  #pragma omp parallel
  {
    // Getting thread id
    int threadId = omp_get_thread_num();

    // Thread-local storage for hash
    phmap::flat_hash_set<uint64_t> threadLocalHashDB;

    // Storage for base frames
    uint8_t baseFrameData[_FRAME_DATA_SIZE];

    // Profiling timers
    double threadHashCalculationTime = 0.0;
    double threadHashCheckingTime1 = 0.0;
    double threadHashCheckingTime2 = 0.0;
    double threadFrameAdvanceTime = 0.0;
    double threadFrameDeserializationTime = 0.0;
    double threadFrameDecodingTime = 0.0;
    double threadFrameEncodingTime = 0.0;

    // Computing always the last frame while resizing the database to reduce memory footprint
    #pragma omp for schedule(static)
    for (auto& baseFrame : _frameDB)
    {
      auto t0 = std::chrono::steady_clock::now(); // Profiling
      baseFrame->getFrameDataFromDifference(_sourceFrameData, baseFrameData);
      auto tf = std::chrono::steady_clock::now();
      threadFrameDecodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Getting possible moves for the current frame
      t0 = std::chrono::steady_clock::now(); // Profiling
      _state[threadId]->pushState(baseFrameData);
      std::vector<uint8_t> possibleMoveIds = _state[threadId]->getPossibleMoveIds();
      tf = std::chrono::steady_clock::now();
      threadFrameDeserializationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Getting base frame current level
      auto curWorld = _state[threadId]->_nes->_currentWorld;
      auto curStage = _state[threadId]->_nes->_currentStage;

      // Running possible moves
      for (size_t idx = 0; idx < possibleMoveIds.size(); idx++)
      {
        // Increasing  frames processed counter
        #pragma omp atomic
        _stepFramesProcessedCounter++;

        // Getting possible move id
        auto moveId = possibleMoveIds[idx];

        // If this comes after the first move, we need to reload the base state
        if (idx > 0)
        {
         t0 = std::chrono::steady_clock::now(); // Profiling
         _state[threadId]->pushState(baseFrameData);
         tf = std::chrono::steady_clock::now();
         threadFrameDeserializationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
        }

        // Perform the selected move
        t0 = std::chrono::steady_clock::now(); // Profiling
        _state[threadId]->_nes->advanceFrame(moveId);
        tf = std::chrono::steady_clock::now();
        threadFrameAdvanceTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // Compute hash value
        t0 = std::chrono::steady_clock::now(); // Profiling
        auto hash = _state[threadId]->_nes->computeHash();
        tf = std::chrono::steady_clock::now();
        threadHashCalculationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // Checking for the existence of the hash in the hash databases (except the current one)
        t0 = std::chrono::steady_clock::now(); // Profiling
        bool collisionDetected = !threadLocalHashDB.insert(hash).second; // First check locally for collisions
        tf = std::chrono::steady_clock::now();
        threadHashCheckingTime1 += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // Then check the shared, read-only database
        t0 = std::chrono::steady_clock::now(); // Profiling
        if (collisionDetected == false) collisionDetected |= !_hashDB.insert({hash, _currentStep}).second;
        tf = std::chrono::steady_clock::now();
        threadHashCheckingTime2 += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // If collision detected, discard this frame
        if (collisionDetected) { _newCollisionCounter++; continue; }

        // Creating new frame storage
        auto newFrame = std::make_unique<Frame>();

        // Copying rule status from the base frame
        memcpy(newFrame->rulesStatus, baseFrame->rulesStatus, sizeof(Frame::rulesStatus));

        // Copying move list
        #ifndef JAFFARNES_DISABLE_MOVE_HISTORY
        memcpy(newFrame->moveHistory, baseFrame->moveHistory, sizeof(Frame::moveHistory));
        #endif

        // Storage for frame type
        frameType type = f_regular;

        // Storage for new move ids
        std::vector<uint8_t> possibleNewMoveIds;

        // Evaluating rules on the new frame
        _state[threadId]->evaluateRules(newFrame->rulesStatus);

        // Getting frame type
        type = _state[threadId]->getFrameType(newFrame->rulesStatus);

        // If required, store move history
        #ifndef JAFFARNES_DISABLE_MOVE_HISTORY
        newFrame->setMove(_currentStep, moveId);
        #endif

        // If frame type is failed, continue to the next one
        if (type == f_fail) continue;

        // Calculating current reward
        newFrame->reward = _state[threadId]->getFrameReward(newFrame->rulesStatus);

        // Storing the frame data, only if if belongs to the same level
        if (curWorld == _state[threadId]->_nes->_currentWorld && curStage == _state[threadId]->_nes->_currentStage)
        {

         t0 = std::chrono::steady_clock::now(); // Profiling
         uint8_t gameState[_FRAME_DATA_SIZE];
         _state[threadId]->_nes->serializeState(gameState);
         newFrame->computeFrameDifference(_sourceFrameData, gameState);
         tf = std::chrono::steady_clock::now(); // Profiling
         threadFrameEncodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
        }

        // If frame has succeded or is a regular frame, adding it in the corresponding database
        #pragma omp critical(insertFrame)
        {
         if (type == f_win) { _winFrameFound = true; _winFrame = *newFrame; };
         if (type == f_regular) newFrames.push_back(std::move(newFrame));
        }
      }

      // Freeing up base frame memory
      baseFrame.reset();
    }

    // Updating timers
    #pragma omp critical
    {
     _stepHashCalculationTime += threadHashCalculationTime;
     _stepHashCheckingTime1 += threadHashCheckingTime1;
     _stepHashCheckingTime2 += threadHashCheckingTime2;
     _stepFrameAdvanceTime += threadFrameAdvanceTime;
     _stepFrameDeserializationTime += threadFrameDeserializationTime;
     _stepFrameEncodingTime += threadFrameEncodingTime;
     _stepFrameDecodingTime += threadFrameDecodingTime;
    }
  }

  // Updating timer averages
  _stepHashCalculationTime /= _threadCount;
  _stepHashCheckingTime1 /= _threadCount;
  _stepHashCheckingTime2 /= _threadCount;
  _stepFrameAdvanceTime /= _threadCount;
  _stepFrameDeserializationTime /= _threadCount;
  _stepFrameEncodingTime /= _threadCount;
  _stepFrameDecodingTime /= _threadCount;

  // Clearing all old frames
  _frameDB.clear();

  // Sorting local DB frames by reward
  auto DBSortingTimeBegin = std::chrono::steady_clock::now(); // Profiling
  if (newFrames.size() > _maxDatabaseSize)
  {
   // If global frames exceed the maximum allowed, sort and truncate all excessive frames
   auto m = newFrames.begin() + _maxDatabaseSize;
   std::nth_element(newFrames.begin(), m, newFrames.end(), [](const auto &a, const auto &b) { return a->reward > b->reward; });
   newFrames.resize(_maxDatabaseSize);
  }

  // Looking for and storing best/worst frame
  _bestFrameReward = -std::numeric_limits<float>::infinity();
  _worstFrameReward = std::numeric_limits<float>::infinity();
  for (const auto& frame : newFrames)
  {
   if (frame->reward > _bestFrameReward) { _bestFrame = *frame; _bestFrameReward = _bestFrame.reward; }
   if (frame->reward < _worstFrameReward) { _worstFrame = *frame; _worstFrameReward = _worstFrame.reward; }
  }

  // Setting new frames to be current frames for the next step
  std::swap(newFrames, _frameDB);

  auto DBSortingTimeEnd = std::chrono::steady_clock::now();                                                                           // Profiling
  _stepFrameDBSortingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(DBSortingTimeEnd - DBSortingTimeBegin).count(); // Profiling

  // Summing frame processing counters
  _totalFramesProcessedCounter += _stepFramesProcessedCounter;

  // Calculating and storing new entries created in this step and calculating size in mb to evaluate filtering
  _hashStepNewEntries.push_back(_hashDB.size() - startHashEntryCount);
  _hashSizeCurrent = ((double)sizeof(std::pair<const uint16_t, uint64_t>)*(double)_hashDB.size()) / (1024.0 * 1024.0);

  // Filtering old hashes if we reach the upper bound
  auto hashFilteringTimeBegin = std::chrono::steady_clock::now(); // Profiling
  if (_hashSizeCurrent > _hashSizeUpperBound)
  {
   // Calculating how many old hash entries we need to delete to reach the lower bound
   size_t targetDeletedHashEntries = (_hashSizeLowerBound * (1024.0 * 1024.0)) /  (double)sizeof(std::pair<const uint16_t, uint64_t>);

   // Calculate how many old steps we need to forget to reach the number of entries calculated before
   size_t curDeletedHashEntries = 0;
   while(curDeletedHashEntries < targetDeletedHashEntries && _hashStepThreshold < _currentStep-1) curDeletedHashEntries += _hashStepNewEntries[_hashStepThreshold++];

   // Erasing older hashes according to the new threshold
   for (auto& hashEntry : _hashDB) _hashDB.erase_if(hashEntry.first, [this](const auto& age){return age < _hashStepThreshold;});
  }

  auto hashFilteringTimeEnd = std::chrono::steady_clock::now();                                                                           // Profiling
  _stepHashFilteringTime = std::chrono::duration_cast<std::chrono::nanoseconds>(hashFilteringTimeEnd - hashFilteringTimeBegin).count(); // Profiling

  // Hash Statistics
  _hashCollisions += _newCollisionCounter;
  _hashEntriesStep = _hashDB.size() - startHashEntryCount;
  _hashSizeStep = ((double)sizeof(std::pair<const uint16_t, uint64_t>)*(double)_hashEntriesStep) / (1024.0 * 1024.0);
  _hashSizeCurrent = ((double)sizeof(std::pair<const uint16_t, uint64_t>)*(double)_hashDB.size()) / (1024.0 * 1024.0);
  _hashEntriesTotal = _hashDB.size();
}



void Train::printTrainStatus()
{
  printf("[JaffarNES] ----------------------------------------------------------------\n");
  printf("[JaffarNES] Current Step #: %u (Max: %u)\n", _currentStep, _MAX_MOVELIST_SIZE);
  printf("[JaffarNES] Worst Reward / Best Reward: %f / %f\n", _worstFrameReward, _bestFrameReward);
  printf("[JaffarNES] Performance: %.3f Frames/s\n", (double)_stepFramesProcessedCounter / (_currentStepTime / 1.0e+9));
  printf("[JaffarNES] Frames Processed: (Step/Total): %lu / %lu\n", _stepFramesProcessedCounter, _totalFramesProcessedCounter);
  printf("[JaffarNES] Elapsed Time (Step/Total):   %3.3fs / %3.3fs\n", _currentStepTime / 1.0e+9, _searchTotalTime / 1.0e+9);
  printf("[JaffarNES]   + Hash Calculation:        %3.3fs\n", _stepHashCalculationTime / 1.0e+9);
  printf("[JaffarNES]   + Hash Checking:           %3.3fs (Local %3.3fs, Shared %3.3fs)\n",  (_stepHashCheckingTime1 + _stepHashCheckingTime2) / 1.0e+9, _stepHashCheckingTime1 / 1.0e+9, _stepHashCheckingTime2 / 1.0e+9);
  printf("[JaffarNES]   + Hash Filtering:          %3.3fs\n", _stepHashFilteringTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Advance:           %3.3fs\n", _stepFrameAdvanceTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Deserialization:   %3.3fs\n", _stepFrameDeserializationTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Encoding:          %3.3fs\n", _stepFrameEncodingTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Decoding:          %3.3fs\n", _stepFrameDecodingTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Sorting            %3.3fs\n", _stepFrameDBSortingTime / 1.0e+9);
  printf("[JaffarNES] Max Frame State Difference: %lu / %d\n", _maxFrameDiff, _MAX_FRAME_DIFF);
  printf("[JaffarNES] Frame DB Entries (Total / Max): %lu / %lu\n", _databaseSize, _maxDatabaseSize);
  printf("[JaffarNES] Frame DB Size (Total / Max): %.3fmb / %.3fmb\n", (double)(_databaseSize * sizeof(Frame)) / (1024.0 * 1024.0), (double)(_maxDatabaseSize * sizeof(Frame)) / (1024.0 * 1024.0));
  printf("[JaffarNES] Hash DB Collisions (Step/Total): %lu / %lu\n", _newCollisionCounter, _hashCollisions);
  printf("[JaffarNES] Hash DB Entries (Step/Total): %lu / %lu\n", _currentStep == 0 ? 0 : _hashStepNewEntries[_currentStep-1], _hashEntriesTotal);
  printf("[JaffarNES] Hash DB Size (Step/Total/Max): %.3fmb, %.3fmb, <%.0f,%.0f>mb\n", _hashSizeStep, _hashSizeCurrent, _hashSizeLowerBound, _hashSizeUpperBound);
  printf("[JaffarNES] Hash DB Step Threshold: %u\n", _hashStepThreshold);
  printf("[JaffarNES] Best Frame Information:\n");

  uint8_t bestFrameData[_FRAME_DATA_SIZE];
  _bestFrame.getFrameDataFromDifference(_sourceFrameData, bestFrameData);
  _state[0]->pushState(bestFrameData);
  _state[0]->_nes->printFrameInfo();
  _state[0]->printRuleStatus(_bestFrame.rulesStatus);

  // Getting magnet values for the kid
  auto marioMagnet = _state[0]->getMarioMagnetValues(_bestFrame.rulesStatus);

  printf("[JaffarNES]  + Kid Horizontal Magnet Intensity: %.1f\n", marioMagnet.intensityX);
  printf("[JaffarNES]  + Kid Vertical Magnet Intensity: %.1f\n", marioMagnet.intensityY);

  #ifndef JAFFARNES_DISABLE_MOVE_HISTORY

  // Print Move History
  printf("[JaffarNES]  + Move List: ");
  for (size_t i = 0; i < _currentStep; i++)
    printf("%s ", _possibleMoves[_bestFrame.getMove(i)].c_str());
  printf("\n");

  #endif
}

Train::Train(int argc, char *argv[])
{
  // Getting number of openMP threads
  _threadCount = omp_get_max_threads();

  // Profiling information
  _searchTotalTime = 0.0;
  _currentStepTime = 0.0;

  // Initializing counters
  _stepFramesProcessedCounter = 0;
  _totalFramesProcessedCounter = 0;
  _newCollisionCounter = 0;
  _hashEntriesTotal = 0;
  _hashStepThreshold = 0;
  _hashEntriesStep = 0;

  // Setting starting step
  _currentStep = 0;

  // Flag to determine if win frame was found
  _winFrameFound = false;

  // Parsing max hash DB entries
  if (const char *hashSizeLowerBoundString = std::getenv("JAFFARNES_MAX_HASH_DATABASE_SIZE_LOWER_BOUND_MB")) _hashSizeLowerBound = std::stol(hashSizeLowerBoundString);
  else EXIT_WITH_ERROR("[JaffarNES] JAFFARNES_MAX_HASH_DATABASE_SIZE_LOWER_BOUND_MB environment variable not defined.\n");

  if (const char *hashSizeUpperBoundString = std::getenv("JAFFARNES_MAX_HASH_DATABASE_SIZE_UPPER_BOUND_MB")) _hashSizeUpperBound = std::stol(hashSizeUpperBoundString);
  else EXIT_WITH_ERROR("[JaffarNES] JAFFARNES_MAX_HASH_DATABASE_SIZE_UPPER_BOUND_MB environment variable not defined.\n");

  // Parsing max frame DB entries
  size_t maxDBSizeMb = 0;
  if (const char *MaxDBMBytesEnvString = std::getenv("JAFFARNES_MAX_FRAME_DATABASE_SIZE_MB")) maxDBSizeMb = std::stol(MaxDBMBytesEnvString);
  else EXIT_WITH_ERROR("[JaffarNES] JAFFARNES_MAX_FRAME_DATABASE_SIZE_MB environment variable not defined.\n");

  // Parsing file output frequency
  _outputSaveBestSeconds = -1.0;
  if (const char *outputSaveBestSecondsEnv = std::getenv("JAFFARNES_SAVE_BEST_EVERY_SECONDS")) _outputSaveBestSeconds = std::stof(outputSaveBestSecondsEnv);

  // Parsing savegame files output path
  _outputSaveBestPath = "/tmp/jaffar";
  if (const char *outputSaveBestPathEnv = std::getenv("JAFFARNES_SAVE_BEST_PATH")) _outputSaveBestPath = std::string(outputSaveBestPathEnv);

  // Parsing solution files output path
  _outputSolutionBestPath = "/tmp/jaffar.best.sol";
  if (const char *outputSolutionBestPathEnv = std::getenv("JAFFARNES_SOLUTION_BEST_PATH")) _outputSolutionBestPath = std::string(outputSolutionBestPathEnv);

  // Parsing command line arguments
  argparse::ArgumentParser program("jaffarNES-train", "1.0");

  program.add_argument("romFile")
    .help("Specifies the path to the NES rom file (.nes) from which to start.")
    .required();

  program.add_argument("stateFile")
    .help("Specifies the path to the NES state file (.state) from which to start.")
    .required();

  program.add_argument("jaffarFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Getting rom file path
  auto romFilePath = program.get<std::string>("romFile");

  // Getting state file path
  auto stateFilePath = program.get<std::string>("stateFile");

  // Parsing config files
  _scriptFile = program.get<std::string>("jaffarFile");
  nlohmann::json scriptFileJs;
  std::string scriptString;
  auto status = loadStringFromFile(scriptString, _scriptFile.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar script file: %s\n%s \n", _scriptFile.c_str(), program.help().str().c_str());

  nlohmann::json scriptJs;
  try { scriptJs = nlohmann::json::parse(scriptString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", _scriptFile.c_str(), err.what()); }

  // Checking whether it contains the rules field
  if (isDefined(scriptJs, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file '%s' missing 'Rules' key.\n", _scriptFile.c_str());

  // Calculating DB sizes
  _maxDatabaseSize = floor(((double)maxDBSizeMb * 1024.0 * 1024.0) / ((double)sizeof(Frame)));

  // Resizing containers based on thread count
  _state.resize(_threadCount);

  // Instantiating state for result showing purposes
  _showState = new State(romFilePath, stateFilePath, scriptJs["Rules"]);

  // Initializing thread-specific instances
  #pragma omp parallel
  {
   // Getting thread id
   int threadId = omp_get_thread_num();

   #pragma omp critical
    _state[threadId] = new State(romFilePath, stateFilePath, scriptJs["Rules"]);
  }

  printf("[JaffarNES] QuickNES initialized.\n");

  // Setting initial values
  _hasFinalized = false;
  _hashCollisions = 0;

  // Check rule count does not exceed maximum
  _ruleCount = _state[0]->_rules.size();

  // Maximum difference between explored frames and the pivot frame
  _maxFrameDiff = 0;

  // Setting win status
  _winFrameFound = false;

  // Computing initial hash
  const auto hash = _state[0]->_nes->computeHash();

  auto initialFrame = std::make_unique<Frame>();
  uint8_t gameState[_FRAME_DATA_SIZE];
  _state[0]->_nes->serializeState(gameState);
  initialFrame->computeFrameDifference(_sourceFrameData, gameState);
  for (size_t i = 0; i < _ruleCount; i++) initialFrame->rulesStatus[i] = false;

  // Evaluating Rules on initial frame
  _state[0]->evaluateRules(initialFrame->rulesStatus);

  // Evaluating Score on initial frame
  initialFrame->reward = _state[0]->getFrameReward(initialFrame->rulesStatus);

  // Registering hash for initial frame
  _hashDB[0] = hash;

  // Copying initial frame into the best frame
  _bestFrame = *initialFrame;
  _bestFrameReward = initialFrame->reward;
 _worstFrameReward = initialFrame->reward;

  // Adding frame to the initial database
  _databaseSize = 1;
  _frameDB.push_back(std::move(initialFrame));

  // Initializing show thread
  if (pthread_create(&_showThreadId, NULL, showThreadFunction, this) != 0)
    EXIT_WITH_ERROR("[ERROR] Could not create show thread.\n");
}

// Functions for the show thread
void *Train::showThreadFunction(void *trainPtr)
{
  ((Train *)trainPtr)->showSavingLoop();
  return NULL;
}

void Train::showSavingLoop()
{
  // Timer for saving frames
  auto bestFrameSaveTimer = std::chrono::steady_clock::now();

  while (_hasFinalized == false)
  {
    // Sleeping for one second to prevent excessive overheads
    sleep(1);

//    // Checking if we need to save best frame
//    if (_outputSaveBestSeconds > 0.0)
//    {
//      double bestFrameTimerElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - bestFrameSaveTimer).count();
//      if (bestFrameTimerElapsed / 1.0e+9 > _outputSaveBestSeconds)
//      {
//        // Saving best frame data
//        std::string outputBestFilePath = _outputSaveBestPath + std::string(".best.sav");
//        uint8_t bestFrameData[_FRAME_DATA_SIZE];
//        _bestFrame.getFrameDataFromDifference(_sourceFrameData, bestFrameData);
//        _showState->_nes->deserializeState(bestFrameData);
//        _showState->_nes->saveStateFile(outputBestFilePath);
//
//        // Saving worst frame data
//        std::string outputWorstFilePath = _outputSaveBestPath + std::string(".worst.sav");
//        uint8_t worstFrameData[_FRAME_DATA_SIZE];
//        _worstFrame.getFrameDataFromDifference(_sourceFrameData, worstFrameData);
//        _showState->_nes->deserializeState(worstFrameData);
//        _showState->_nes->saveStateFile(outputWorstFilePath);
//
//        #ifndef JAFFARNES_DISABLE_MOVE_HISTORY
//
//        // Storing the solution sequence
//        std::string bestSolutionString;
//        bestSolutionString += _possibleMoves[_bestFrame.getMove(0)];
//        for (size_t i = 1; i < _currentStep; i++)
//         bestSolutionString += std::string(" ") + _possibleMoves[_bestFrame.getMove(i)];
//        bestSolutionString += std::string(" .");
//        std::string outputSolPath = _outputSaveBestPath + std::string(".best.sol");
//        saveStringToFile(bestSolutionString, outputSolPath.c_str());
//
//        // Storing the solution sequence
//        std::string worstSolutionString;
//        worstSolutionString += _possibleMoves[_worstFrame.getMove(0)];
//        for (size_t i = 1; i < _currentStep; i++)
//         worstSolutionString += std::string(" ") + _possibleMoves[_worstFrame.getMove(i)];
//        worstSolutionString += std::string(" .");
//        std::string outputWorstSolPath = _outputSaveBestPath + std::string(".worst.sol");
//        saveStringToFile(worstSolutionString, outputWorstSolPath.c_str());
//
//        #endif
//
//        // Resetting timer
//        bestFrameSaveTimer = std::chrono::steady_clock::now();
//      }
//    }
  }
}

int main(int argc, char *argv[])
{
  Train train(argc, argv);

  // Running Search
  train.run();
}
