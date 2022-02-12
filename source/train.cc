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
  printf("[JaffarNES] Max Frame DB entries: %lu\n", _maxDatabaseSizeLowerBound);

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
    _winFrame.getFrameDataFromDifference(_referenceFrameData, winFrameData);
    _state[0]->pushState((uint8_t*)winFrameData);
    _state[0]->_nes->printFrameInfo();
    _state[0]->printRuleStatus(_winFrame.rulesStatus);

    #ifndef JAFFARNES_DISABLE_MOVE_HISTORY

    // Print Move History
    printf("[JaffarNES]  + Move List: ");
    for (uint16_t i = 0; i < _currentStep; i++)
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

void Train::limitFrameDatabase(std::vector<Frame*>& frameDB, size_t limit)
{
 // If global frames exceed the maximum allowed, sort and truncate all excessive frames
 std::nth_element(frameDB.begin(), frameDB.begin() + _maxDatabaseSizeLowerBound, frameDB.end(), [](const auto &a, const auto &b) { return a->reward > b->reward; });

 // Recycle excess frames
 for (size_t i = limit; i < frameDB.size(); i++) _freeFramesQueue.push(frameDB[i]);

 // Resizing new frames database to lower bound
 frameDB.resize(limit);
}

void Train::computeFrames()
{
  // Initializing counters
  _stepBaseFramesProcessedCounter = 0;
  _stepNewFramesProcessedCounter = 0;
  _newCollisionCounter = 0;
  _stepMaxFramesInMemory = 0;
  size_t newFramesCounter = 0;
  size_t startHashEntryCount = _hashDB.size();

  // Creating shared database for new frames
  std::vector<Frame*> newFrames;

  // Storing current source frame data for decoding
  uint8_t currentSourceFrameData[_FRAME_DATA_SIZE];
  memcpy(currentSourceFrameData, _referenceFrameData, _FRAME_DATA_SIZE);

  // Updating reference data with the first entry of the latest frames, for encoding
  _frameDB[0]->getFrameDataFromDifference(currentSourceFrameData, _referenceFrameData);

  // Initializing step timers
  _stepHashCalculationTime = 0.0;
  _stepHashCheckingTime = 0.0;
  _stepFrameAdvanceTime = 0.0;
  _stepFrameDeserializationTime = 0.0;
  _stepFrameEncodingTime = 0.0;
  _stepFrameDecodingTime = 0.0;
  _stepFrameCreationTime = 0.0;
  _stepFrameDBSortingTime = 0.0;

  // Processing frame database in parallel
  #pragma omp parallel
  {
    // Getting thread id
    int threadId = omp_get_thread_num();

    // Storage for base frames
    uint8_t baseFrameData[_FRAME_DATA_SIZE];

    // Profiling timers
    double threadHashCalculationTime = 0.0;
    double threadHashCheckingTime = 0.0;
    double threadFrameAdvanceTime = 0.0;
    double threadFrameDeserializationTime = 0.0;
    double threadFrameDecodingTime = 0.0;
    double threadFrameEncodingTime = 0.0;
    double threadFrameCreationTime = 0.0;

    // Computing always the last frame while resizing the database to reduce memory footprint
    #pragma omp for schedule(dynamic, 1024)
    for (auto& baseFrame : _frameDB)
    {
      auto t0 = std::chrono::steady_clock::now(); // Profiling
      baseFrame->getFrameDataFromDifference(currentSourceFrameData, baseFrameData);
      auto tf = std::chrono::steady_clock::now();
      threadFrameDecodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Getting possible moves for the current frame
      t0 = std::chrono::steady_clock::now(); // Profiling
      _state[threadId]->pushState(baseFrameData);
      std::vector<uint8_t> possibleMoveIds = _state[threadId]->getPossibleMoveIds();
      tf = std::chrono::steady_clock::now();
      threadFrameDeserializationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Running possible moves
      for (size_t idx = 0; idx < possibleMoveIds.size(); idx++)
      {
        // Increasing  frames processed counter
        #pragma omp atomic
        _stepNewFramesProcessedCounter++;

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

        // Checking for the existence of the hash in the hash database
        t0 = std::chrono::steady_clock::now(); // Profiling
        bool collisionDetected = !_hashDB.insert({hash, _currentStep}).second;
        tf = std::chrono::steady_clock::now();
        threadHashCheckingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // If collision detected, discard this frame
        if (collisionDetected) { _newCollisionCounter++; continue; }

        // Getting rule status from the base frame
        bool rulesStatus[_MAX_RULE_COUNT];
        memcpy(rulesStatus, baseFrame->rulesStatus, sizeof(Frame::rulesStatus));

        // Evaluating rules on the new frame
        _state[threadId]->evaluateRules(rulesStatus);

        // Getting frame type
        frameType type = _state[threadId]->getFrameType(rulesStatus);

        // If frame type is failed, continue to the next possible move
        if (type == f_fail) continue;

        // Storing the frame data
        t0 = std::chrono::steady_clock::now(); // Profiling

        // Allocating new frame, checking free frame queue if there's a storage we can reuse
        Frame* newFrame;
        bool foundFreeFrameStorage = false;

        // If found, we take directly from the queue
        #pragma omp critical(frameQueue)
         if (_freeFramesQueue.empty() == false)
         {
          newFrame = _freeFramesQueue.front();
          _freeFramesQueue.pop();
          foundFreeFrameStorage = true;
         }

        // Otherwise we allocate a new storage
        if (foundFreeFrameStorage == false) newFrame = new Frame;

        // Copying rule status into new frame
        memcpy(newFrame->rulesStatus, rulesStatus, sizeof(Frame::rulesStatus));

        // Copying move list and adding new move
        #ifndef JAFFARNES_DISABLE_MOVE_HISTORY
        memcpy(newFrame->moveHistory, baseFrame->moveHistory, sizeof(Frame::moveHistory));
        newFrame->setMove(_currentStep, moveId);
        #endif

        // Calculating current reward
        newFrame->reward = _state[threadId]->getFrameReward(newFrame->rulesStatus);

        tf = std::chrono::steady_clock::now(); // Profiling
        threadFrameCreationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count(); // Profiling

        // Encoding the frame data
        t0 = std::chrono::steady_clock::now(); // Profiling

        uint8_t gameState[_FRAME_DATA_SIZE];
        _state[threadId]->_nes->serializeState(gameState);
        newFrame->computeFrameDifference(_referenceFrameData, gameState);

        tf = std::chrono::steady_clock::now(); // Profiling
        threadFrameEncodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count(); // Profiling

        // If frame has succeded or is a regular frame, adding it in the corresponding database
        #pragma omp critical(insertFrame)
        {
         // Storing new winning frame
         if (type == f_win) { _winFrameFound = true; _winFrame = *newFrame; };

         // Adding frame to the new frame database
         newFrames.push_back(newFrame);

         // Increasing new frame counter
         newFramesCounter++;

         // Calculating the number of frames in use in memory
         size_t framesInUse = _frameDB.size() - _stepBaseFramesProcessedCounter + newFrames.size();

         // Increasing maximum frames in use if needed
         if (framesInUse > _stepMaxFramesInMemory) _stepMaxFramesInMemory = framesInUse;

         // If new frame db exceeds upper bound, limit it back to lower bound
         if (framesInUse > _maxDatabaseSizeUpperBound)
         {
          auto DBSortingTimeBegin = std::chrono::steady_clock::now(); // Profiling

          // Checking if limiting will help at all
          if (newFrames.size() < _maxDatabaseSizeLowerBound)
           EXIT_WITH_ERROR("[ERROR] New frames database (%lu) is smaller than lower bound (%lu) and will not bring the total frames (%lu - %lu + %lu = %lu) under the upper bound (%lu).\n", newFrames.size(), _maxDatabaseSizeLowerBound, _frameDB.size(), _stepBaseFramesProcessedCounter, newFrames.size(), framesInUse, _maxDatabaseSizeUpperBound);

          // Limiting new frames DB to lower bound size and recycling its frames
          #pragma omp critical(frameQueue)
          limitFrameDatabase(newFrames, _maxDatabaseSizeLowerBound);

          auto DBSortingTimeEnd = std::chrono::steady_clock::now();                                                                      // Profiling
          _stepFrameDBSortingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(DBSortingTimeEnd - DBSortingTimeBegin).count(); // Profiling
         }
        }
      }

      // Adding used base frame back into free frame queue
      #pragma omp critical(frameQueue)
      _freeFramesQueue.push(baseFrame);

      // Increasing counter for base frames processed
      #pragma omp atomic
      _stepBaseFramesProcessedCounter++;
    }

    // Updating timers
    #pragma omp critical
    {
     _stepHashCalculationTime += threadHashCalculationTime;
     _stepHashCheckingTime += threadHashCheckingTime;
     _stepFrameAdvanceTime += threadFrameAdvanceTime;
     _stepFrameDeserializationTime += threadFrameDeserializationTime;
     _stepFrameEncodingTime += threadFrameEncodingTime;
     _stepFrameDecodingTime += threadFrameDecodingTime;
     _stepFrameCreationTime += threadFrameCreationTime;
    }
  }

  // Updating timer averages
  _stepHashCalculationTime /= _threadCount;
  _stepHashCheckingTime /= _threadCount;
  _stepFrameAdvanceTime /= _threadCount;
  _stepFrameDeserializationTime /= _threadCount;
  _stepFrameEncodingTime /= _threadCount;
  _stepFrameDecodingTime /= _threadCount;
  _stepFrameCreationTime /= _threadCount;

  // Calculating new frame ratio (new / old)
  _stepNewFrameRatio = (double)newFramesCounter / (double)_frameDB.size();

  // Clearing all old frames
  _frameDB.clear();

  // Updating max frames in memory counter
  if (_stepMaxFramesInMemory > _totalMaxFramesInMemory) { _totalMaxFramesInMemory = _stepMaxFramesInMemory; _maxNewFrameRatio = _stepNewFrameRatio; _maxNewFrameRatioStep = _currentStep; }

  // Sorting local DB frames by reward
  auto DBSortingTimeBegin = std::chrono::steady_clock::now(); // Profiling

  // If size exceeds the lower bound, limit it
  if (newFrames.size() > _maxDatabaseSizeLowerBound) limitFrameDatabase(newFrames, _maxDatabaseSizeLowerBound);

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
  _stepFrameDBSortingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(DBSortingTimeEnd - DBSortingTimeBegin).count(); // Profiling

  // Summing frame processing counters
  _totalFramesProcessedCounter += _stepNewFramesProcessedCounter;

  // Calculating and storing new entries created in this step and calculating size in mb to evaluate filtering
  auto getHashSizeFromEntries = [](const ssize_t entries) { return ( ( (double)sizeof(std::pair<const uint16_t, uint64_t>) + (double)sizeof(void*) ) *(double)entries) / (1024.0 * 1024.0); };
  auto getHashEntriesFromSize = [](const double size) { return (ssize_t)((size * 1024.0 * 1024.0) / ( (double)sizeof(std::pair<const uint16_t, uint64_t>) + (double)sizeof(void*) )); };

  _hashStepNewEntries.push_back(_hashDB.size() - startHashEntryCount);
  _hashSizeCurrent = getHashSizeFromEntries(_hashDB.size());

  // Filtering old hashes if we reach the upper bound
  auto hashFilteringTimeBegin = std::chrono::steady_clock::now(); // Profiling
  if (_hashSizeCurrent > _hashSizeUpperBound)
  {
   // Calculating how many old hash entries we need to delete to reach the lower bound
   size_t targetDeletedHashEntries = getHashEntriesFromSize(_hashSizeCurrent - _hashSizeLowerBound);

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
  _hashSizeStep = getHashSizeFromEntries(_hashEntriesStep);
  _hashSizeCurrent = getHashSizeFromEntries(_hashDB.size());
  _hashEntriesTotal = _hashDB.size();
}



void Train::printTrainStatus()
{
  printf("[JaffarNES] ----------------------------------------------------------------\n");
  printf("[JaffarNES] Current Step #: %u (Max: %u)\n", _currentStep, _MAX_MOVELIST_SIZE);
  printf("[JaffarNES] Worst Reward / Best Reward: %f / %f\n", _worstFrameReward, _bestFrameReward);
  printf("[JaffarNES] Base Frames Performance: %.3f Frames/s\n", (double)_stepBaseFramesProcessedCounter / (_currentStepTime / 1.0e+9));
  printf("[JaffarNES] New Frames Performance:  %.3f Frames/s\n", (double)_stepNewFramesProcessedCounter / (_currentStepTime / 1.0e+9));
  printf("[JaffarNES] Frames Processed: (Step/Total): %lu / %lu\n", _stepNewFramesProcessedCounter, _totalFramesProcessedCounter);
  printf("[JaffarNES] Frame DB Entries (Total / Max): %lu (%.3fmb) / %lu (%.3fmb)\n", _databaseSize, (double)(_databaseSize * sizeof(Frame)) / (1024.0 * 1024.0), _maxDatabaseSizeLowerBound, (double)(_maxDatabaseSizeLowerBound * sizeof(Frame)) / (1024.0 * 1024.0));
  printf("[JaffarNES] Elapsed Time (Step/Total):   %3.3fs / %3.3fs\n", _currentStepTime / 1.0e+9, _searchTotalTime / 1.0e+9);
  printf("[JaffarNES]   + Hash Calculation:        %3.3fs\n", _stepHashCalculationTime / 1.0e+9);
  printf("[JaffarNES]   + Hash Checking:           %3.3fs\n",  _stepHashCheckingTime / 1.0e+9);
  printf("[JaffarNES]   + Hash Filtering:          %3.3fs\n", _stepHashFilteringTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Advance:           %3.3fs\n", _stepFrameAdvanceTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Deserialization:   %3.3fs\n", _stepFrameDeserializationTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Encoding:          %3.3fs\n", _stepFrameEncodingTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Decoding:          %3.3fs\n", _stepFrameDecodingTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Creation:          %3.3fs\n", _stepFrameCreationTime / 1.0e+9);
  printf("[JaffarNES]   + Frame Sorting            %3.3fs\n", _stepFrameDBSortingTime / 1.0e+9);
  printf("[JaffarNES] New Frames Created Ratio (Step/Max(Step)):  %.3f, %.3f (%u)\n", _stepNewFrameRatio, _maxNewFrameRatio, _maxNewFrameRatioStep);
  printf("[JaffarNES] Max Frames In Memory (Step/Max): %lu (%.3fmb) / %lu (%.3fmb)\n", _stepMaxFramesInMemory, (double)(_stepMaxFramesInMemory * sizeof(Frame)) / (1024.0 * 1024.0), _totalMaxFramesInMemory, (double)(_totalMaxFramesInMemory * sizeof(Frame)) / (1024.0 * 1024.0));
  printf("[JaffarNES] Max Frame State Difference: %lu / %d\n", _maxFrameDiff, _MAX_FRAME_DIFF);
  printf("[JaffarNES] Hash DB Collisions (Step/Total): %lu / %lu\n", _newCollisionCounter, _hashCollisions);
  printf("[JaffarNES] Hash DB Entries (Step/Total): %lu / %lu\n", _currentStep == 0 ? 0 : _hashStepNewEntries[_currentStep-1], _hashEntriesTotal);
  printf("[JaffarNES] Hash DB Size (Step/Total/Max): %.3fmb, %.3fmb, <%.0f,%.0f>mb\n", _hashSizeStep, _hashSizeCurrent, _hashSizeLowerBound, _hashSizeUpperBound);
  printf("[JaffarNES] Hash DB Step Threshold: %u\n", _hashStepThreshold);
  printf("[JaffarNES] Best Frame Information:\n");

  uint8_t bestFrameData[_FRAME_DATA_SIZE];
  _bestFrame.getFrameDataFromDifference(_referenceFrameData, bestFrameData);
  _state[0]->pushState(bestFrameData);
  _state[0]->_nes->printFrameInfo();
  _state[0]->printRuleStatus(_bestFrame.rulesStatus);

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
  _stepBaseFramesProcessedCounter = 0;
  _stepNewFramesProcessedCounter = 0;
  _totalFramesProcessedCounter = 0;
  _stepMaxFramesInMemory = 0;
  _totalMaxFramesInMemory = 0;
  _newCollisionCounter = 0;
  _hashEntriesTotal = 0;
  _hashStepThreshold = 0;
  _hashEntriesStep = 0;
  _stepNewFrameRatio = 0.0;
  _maxNewFrameRatio = 0.0;
  _maxNewFrameRatioStep = 0;

  // Setting starting step
  _currentStep = 0;

  // Flag to determine if win frame was found
  _winFrameFound = false;

  // Parsing max hash DB entries
  if (const char *hashSizeLowerBoundString = std::getenv("JAFFARNES_MAX_HASH_DATABASE_SIZE_LOWER_BOUND_MB")) _hashSizeLowerBound = std::stol(hashSizeLowerBoundString);
  else EXIT_WITH_ERROR("[JaffarNES] JAFFARNES_MAX_HASH_DATABASE_SIZE_LOWER_BOUND_MB environment variable not defined.\n");

  if (const char *hashSizeUpperBoundString = std::getenv("JAFFARNES_MAX_HASH_DATABASE_SIZE_UPPER_BOUND_MB")) _hashSizeUpperBound = std::stol(hashSizeUpperBoundString);
  else EXIT_WITH_ERROR("[JaffarNES] JAFFARNES_MAX_HASH_DATABASE_SIZE_UPPER_BOUND_MB environment variable not defined.\n");

  // Parsing max frame DB lower bound
  size_t maxDBSizeMbLowerBound = 0;
  if (const char *MaxDBMBytesLowerBoundEnvString = std::getenv("JAFFARNES_MAX_FRAME_DATABASE_SIZE_LOWER_BOUND_MB")) maxDBSizeMbLowerBound = std::stol(MaxDBMBytesLowerBoundEnvString);
  else EXIT_WITH_ERROR("[JaffarNES] JAFFARNES_MAX_FRAME_DATABASE_SIZE_LOWER_BOUND_MB environment variable not defined.\n");

  size_t maxDBSizeMbUpperBound = 0;
  if (const char *MaxDBMBytesUpperBoundEnvString = std::getenv("JAFFARNES_MAX_FRAME_DATABASE_SIZE_UPPER_BOUND_MB")) maxDBSizeMbUpperBound = std::stol(MaxDBMBytesUpperBoundEnvString);
  else EXIT_WITH_ERROR("[JaffarNES] JAFFARNES_MAX_FRAME_DATABASE_SIZE_UPPER_BOUND_MB environment variable not defined.\n");

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

  // Calculating max DB size bounds
  _maxDatabaseSizeLowerBound = floor(((double)maxDBSizeMbLowerBound * 1024.0 * 1024.0) / ((double)sizeof(Frame)));
  _maxDatabaseSizeUpperBound = floor(((double)maxDBSizeMbUpperBound * 1024.0 * 1024.0) / ((double)sizeof(Frame)));

  // Resizing containers based on thread count
  _state.resize(_threadCount);

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

  // Setting win status
  _winFrameFound = false;

  // Computing initial hash
  const auto hash = _state[0]->_nes->computeHash();

  auto initialFrame = new Frame;
  uint8_t gameState[_FRAME_DATA_SIZE];
  _state[0]->_nes->serializeState(gameState);

  // Storing initial frame as base for differential comparison
  memcpy(_referenceFrameData, gameState, _FRAME_DATA_SIZE);

  // Storing initial frame difference
  initialFrame->computeFrameDifference(_referenceFrameData, gameState);
  for (size_t i = 0; i < _ruleCount; i++) initialFrame->rulesStatus[i] = false;

  // Evaluating Rules on initial frame
  _state[0]->evaluateRules(initialFrame->rulesStatus);

  // Evaluating Score on initial frame
  initialFrame->reward = _state[0]->getFrameReward(initialFrame->rulesStatus);

  // Registering hash for initial frame
  _hashDB[0] = hash;

  // Copying initial frame into the best/worst frame
  _bestFrame = *initialFrame;
  _worstFrame = *initialFrame;
  _bestFrameReward = initialFrame->reward;
  _worstFrameReward = initialFrame->reward;

  // Adding frame to the initial database
  _databaseSize = 1;
  _frameDB.push_back(initialFrame);

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

    // Checking if we need to save best frame
    if (_outputSaveBestSeconds > 0.0 && _currentStep > 1)
    {
      double bestFrameTimerElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - bestFrameSaveTimer).count();
      if (bestFrameTimerElapsed / 1.0e+9 > _outputSaveBestSeconds)
      {
        #ifndef JAFFARNES_DISABLE_MOVE_HISTORY

        // Storing the solution sequence
        std::string bestSolutionString;
        bestSolutionString += _possibleMoves[_bestFrame.getMove(0)];
        for (size_t i = 1; i < _currentStep; i++)
         bestSolutionString += std::string(" ") + _possibleMoves[_bestFrame.getMove(i)];
        bestSolutionString += std::string(" .");
        std::string outputSolPath = _outputSaveBestPath + std::string(".best.sol");
        saveStringToFile(bestSolutionString, outputSolPath.c_str());

        // Storing the solution sequence
        std::string worstSolutionString;
        worstSolutionString += _possibleMoves[_worstFrame.getMove(0)];
        for (size_t i = 1; i < _currentStep; i++)
         worstSolutionString += std::string(" ") + _possibleMoves[_worstFrame.getMove(i)];
        worstSolutionString += std::string(" .");
        std::string outputWorstSolPath = _outputSaveBestPath + std::string(".worst.sol");
        saveStringToFile(worstSolutionString, outputWorstSolPath.c_str());

        #endif

        // Resetting timer
        bestFrameSaveTimer = std::chrono::steady_clock::now();
      }
    }
  }
}

int main(int argc, char *argv[])
{
  Train train(argc, argv);

  // Running Search
  train.run();
}
