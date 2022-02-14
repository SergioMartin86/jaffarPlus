#include "train.hpp"
#include "argparse.hpp"
#include "utils.hpp"
#include <omp.h>
#include <unistd.h>
#include <algorithm>

void Train::run()
{
  printf("[Jaffar] ----------------------------------------------------------------\n");
  printf("[Jaffar] Launching Jaffar...\n");

  if (_outputSaveBestSeconds > 0)
  {
    printf("[Jaffar] Saving best state every: %.3f seconds.\n", _outputSaveBestSeconds);
    printf("[Jaffar]  + Savefile Path: %s\n", _outputSaveBestPath.c_str());
    printf("[Jaffar]  + Solution Path: %s\n", _outputSolutionBestPath.c_str());
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
    /// Main state processing cycle begin
    /////////////////////////////////////////////////////////////////

    computeStates();

    /////////////////////////////////////////////////////////////////
    /// Main state processing cycle end
    /////////////////////////////////////////////////////////////////

    // Advancing step
    _currentStep++;

    // Terminate if all DBs are depleted and no winning rule was found
    _databaseSize = _stateDB.size();
    if (_databaseSize == 0)
    {
      printf("[Jaffar] State database depleted with no winning states, finishing...\n");
      terminate = true;
    }

    // Terminate if a winning rule was found
    if (_winStateFound)
    {
      printf("[Jaffar] Winning state reached in %u moves, finishing...\n", _currentStep-1);
      terminate = true;
    }

    // Terminate if maximum number of states was reached
    if (_currentStep >= _maxMoveCount)
    {
      printf("[Jaffar] Maximum state number reached, finishing...\n");
      printf("[Jaffar] To run Jaffar for more steps, increase 'Max Move Count' in the .jaffar file.\n");
      terminate = true;
    }
  }

  // Print winning state if found
  if (_winStateFound == true)
  {
    printf("[Jaffar] Win State Information:\n");
    size_t timeStep = _currentStep-1;
    size_t curMins = timeStep / 720;
    size_t curSecs = (timeStep - (curMins * 720)) / 12;
    size_t curMilliSecs = ceil((double)(timeStep - (curMins * 720) - (curSecs * 12)) / 0.012);
    printf("[Jaffar]  + Solution IGT:  %2lu:%02lu.%03lu\n", curMins, curSecs, curMilliSecs);

    uint8_t winStateData[_STATE_DATA_SIZE];
    _winState.getStateDataFromDifference(_referenceStateData, winStateData);
    _gameInstances[0]->pushState((uint8_t*)winStateData);
    _gameInstances[0]->printStateInfo(_winState.rulesStatus);

    // Print Move History
    if (_storeMoveList)
    {
     printf("[Jaffar]  + Move List: ");
     for (uint16_t i = 0; i < _currentStep; i++)
       printf("%s ", _possibleMoves[_winState.getMove(i)].c_str());
     printf("\n");
    }
  }

  // Marking the end of the run
  _hasFinalized = true;

  // Stopping show thread
  pthread_join(_showThreadId, NULL);

  // If it has finalized with a win, save the winning state
  if (_outputSaveBestSeconds > 0.0)
  {
   auto lastState = _winStateFound ? _winState : _bestState;

   // Storing the solution sequence
   if (_storeMoveList)
   {
    std::string solutionString;
    solutionString += _possibleMoves[lastState.getMove(0)];
    for (size_t i = 1; i < _currentStep; i++)
     solutionString += std::string(" ") + _possibleMoves[lastState.getMove(i)];
    solutionString += std::string(" .");
    std::string outputBestSolutionFilePath = _outputSaveBestPath + std::string(".best.sol");
    saveStringToFile(solutionString, outputBestSolutionFilePath.c_str());
   }
  }
}

void Train::limitStateDatabase(std::vector<State*>& stateDB, size_t limit)
{
 // If global states exceed the maximum allowed, sort and truncate all excessive states
 std::nth_element(stateDB.begin(), stateDB.begin() + _maxDatabaseSizeLowerBound, stateDB.end(), [](const auto &a, const auto &b) { return a->reward > b->reward; });

 // Recycle excess states
 for (size_t i = limit; i < stateDB.size(); i++) _freeStatesQueue.push(stateDB[i]);

 // Resizing new states database to lower bound
 stateDB.resize(limit);
}

void Train::computeStates()
{
  // Initializing counters
  _stepBaseStatesProcessedCounter = 0;
  _stepNewStatesProcessedCounter = 0;
  _newCollisionCounter = 0;
  _stepMaxStatesInMemory = 0;
  size_t newStatesCounter = 0;
  size_t startHashEntryCount = _hashDB.size();

  // Creating shared database for new states
  std::vector<State*> newStates;

  // Storing current source state data for decoding
  uint8_t currentSourceStateData[_STATE_DATA_SIZE];
  memcpy(currentSourceStateData, _referenceStateData, _STATE_DATA_SIZE);

  // Updating reference data with the first entry of the latest states, for encoding
  _stateDB[0]->getStateDataFromDifference(currentSourceStateData, _referenceStateData);

  // Initializing step timers
  _stepHashCalculationTime = 0.0;
  _stepHashCheckingTime = 0.0;
  _stepStateAdvanceTime = 0.0;
  _stepStateDeserializationTime = 0.0;
  _stepStateEncodingTime = 0.0;
  _stepStateDecodingTime = 0.0;
  _stepStateCreationTime = 0.0;
  _stepStateDBSortingTime = 0.0;

  // Processing state database in parallel
  #pragma omp parallel
  {
    // Getting thread id
    int threadId = omp_get_thread_num();

    // Storage for base states
    uint8_t baseStateData[_STATE_DATA_SIZE];

    // Profiling timers
    double threadHashCalculationTime = 0.0;
    double threadHashCheckingTime = 0.0;
    double threadStateAdvanceTime = 0.0;
    double threadStateDeserializationTime = 0.0;
    double threadStateDecodingTime = 0.0;
    double threadStateEncodingTime = 0.0;
    double threadStateCreationTime = 0.0;

    // Computing always the last state while resizing the database to reduce memory footprint
    #pragma omp for schedule(dynamic, 1024)
    for (auto& baseState : _stateDB)
    {
      auto t0 = std::chrono::steady_clock::now(); // Profiling
      baseState->getStateDataFromDifference(currentSourceStateData, baseStateData);
      auto tf = std::chrono::steady_clock::now();
      threadStateDecodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Getting possible moves for the current state
      t0 = std::chrono::steady_clock::now(); // Profiling
      _gameInstances[threadId]->pushState(baseStateData);
      std::vector<uint8_t> possibleMoveIds = _gameInstances[threadId]->getPossibleMoveIds();
      tf = std::chrono::steady_clock::now();
      threadStateDeserializationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Running possible moves
      for (size_t idx = 0; idx < possibleMoveIds.size(); idx++)
      {
        // Increasing  states processed counter
        #pragma omp atomic
        _stepNewStatesProcessedCounter++;

        // Getting possible move id
        auto moveId = possibleMoveIds[idx];

        // If this comes after the first move, we need to reload the base state
        if (idx > 0)
        {
         t0 = std::chrono::steady_clock::now(); // Profiling
         _gameInstances[threadId]->pushState(baseStateData);
         tf = std::chrono::steady_clock::now();
         threadStateDeserializationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
        }

        // Perform the selected move
        t0 = std::chrono::steady_clock::now(); // Profiling
        _gameInstances[threadId]->advanceState(moveId);
        tf = std::chrono::steady_clock::now();
        threadStateAdvanceTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // Compute hash value
        t0 = std::chrono::steady_clock::now(); // Profiling
        auto hash = _gameInstances[threadId]->computeHash();
        tf = std::chrono::steady_clock::now();
        threadHashCalculationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // Checking for the existence of the hash in the hash database
        t0 = std::chrono::steady_clock::now(); // Profiling
        bool collisionDetected = !_hashDB.insert({hash, _currentStep}).second;
        tf = std::chrono::steady_clock::now();
        threadHashCheckingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // If collision detected, discard this state
        if (collisionDetected) { _newCollisionCounter++; continue; }

        // Getting rule status from the base state
        bool rulesStatus[_ruleCount];
        memcpy(rulesStatus, baseState->rulesStatus, sizeof(rulesStatus));

        // Evaluating rules on the new state
        _gameInstances[threadId]->evaluateRules(rulesStatus);

        // Getting state type
        stateType type = _gameInstances[threadId]->getStateType(rulesStatus);

        // If state type is failed, continue to the next possible move
        if (type == f_fail) continue;

        // Storing the state data
        t0 = std::chrono::steady_clock::now(); // Profiling

        // Allocating new state, checking free state queue if there's a storage we can reuse
        State* newState;
        bool foundFreeStateStorage = false;

        // If found, we take directly from the queue
        #pragma omp critical(stateQueue)
         if (_freeStatesQueue.empty() == false)
         {
          newState = _freeStatesQueue.front();
          _freeStatesQueue.pop();
          foundFreeStateStorage = true;
         }

        // Otherwise we allocate a new storage
        if (foundFreeStateStorage == false) newState = new State;

        // Copying rule status into new state
        memcpy(newState->rulesStatus, rulesStatus, sizeof(rulesStatus));

        // Copying move list and adding new move
        if (_storeMoveList)
        {
         newState->setMoveHistory(baseState->moveHistory);
         newState->setMove(_currentStep, moveId);
        }

        // Calculating current reward
        newState->reward = _gameInstances[threadId]->getStateReward(newState->rulesStatus);

        tf = std::chrono::steady_clock::now(); // Profiling
        threadStateCreationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count(); // Profiling

        // Encoding the state data
        t0 = std::chrono::steady_clock::now(); // Profiling

        uint8_t gameState[_STATE_DATA_SIZE];
        _gameInstances[threadId]->popState(gameState);
        newState->computeStateDifference(_referenceStateData, gameState);

        tf = std::chrono::steady_clock::now(); // Profiling
        threadStateEncodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count(); // Profiling

        // If state has succeded or is a regular state, adding it in the corresponding database
        #pragma omp critical(insertState)
        {
         // Storing new winning state
         if (type == f_win) { _winStateFound = true; _winState = *newState; };

         // Adding state to the new state database
         newStates.push_back(newState);

         // Increasing new state counter
         newStatesCounter++;

         // Calculating the number of states in use in memory
         size_t statesInUse = _stateDB.size() - _stepBaseStatesProcessedCounter + newStates.size();

         // Increasing maximum states in use if needed
         if (statesInUse > _stepMaxStatesInMemory) _stepMaxStatesInMemory = statesInUse;

         // If new state db exceeds upper bound, limit it back to lower bound
         if (statesInUse > _maxDatabaseSizeUpperBound)
         {
          auto DBSortingTimeBegin = std::chrono::steady_clock::now(); // Profiling

          // Checking if limiting will help at all
          if (newStates.size() < _maxDatabaseSizeLowerBound)
           EXIT_WITH_ERROR("[ERROR] New states database (%lu) is smaller than lower bound (%lu) and will not bring the total states (%lu - %lu + %lu = %lu) under the upper bound (%lu).\n", newStates.size(), _maxDatabaseSizeLowerBound, _stateDB.size(), _stepBaseStatesProcessedCounter, newStates.size(), statesInUse, _maxDatabaseSizeUpperBound);

          // Limiting new states DB to lower bound size and recycling its states
          #pragma omp critical(stateQueue)
          limitStateDatabase(newStates, _maxDatabaseSizeLowerBound);

          auto DBSortingTimeEnd = std::chrono::steady_clock::now();                                                                      // Profiling
          _stepStateDBSortingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(DBSortingTimeEnd - DBSortingTimeBegin).count(); // Profiling
         }
        }
      }

      // Adding used base state back into free state queue
      #pragma omp critical(stateQueue)
      _freeStatesQueue.push(baseState);

      // Increasing counter for base states processed
      #pragma omp atomic
      _stepBaseStatesProcessedCounter++;
    }

    // Updating timers
    #pragma omp critical
    {
     _stepHashCalculationTime += threadHashCalculationTime;
     _stepHashCheckingTime += threadHashCheckingTime;
     _stepStateAdvanceTime += threadStateAdvanceTime;
     _stepStateDeserializationTime += threadStateDeserializationTime;
     _stepStateEncodingTime += threadStateEncodingTime;
     _stepStateDecodingTime += threadStateDecodingTime;
     _stepStateCreationTime += threadStateCreationTime;
    }
  }

  // Updating timer averages
  _stepHashCalculationTime /= _threadCount;
  _stepHashCheckingTime /= _threadCount;
  _stepStateAdvanceTime /= _threadCount;
  _stepStateDeserializationTime /= _threadCount;
  _stepStateEncodingTime /= _threadCount;
  _stepStateDecodingTime /= _threadCount;
  _stepStateCreationTime /= _threadCount;

  // Calculating new state ratio (new / old)
  _stepNewStateRatio = (double)newStatesCounter / (double)_stateDB.size();

  // Clearing all old states
  _stateDB.clear();

  // Updating max states in memory counter
  if (_stepMaxStatesInMemory > _totalMaxStatesInMemory) { _totalMaxStatesInMemory = _stepMaxStatesInMemory; _maxNewStateRatio = _stepNewStateRatio; _maxNewStateRatioStep = _currentStep; }

  // Sorting local DB states by reward
  auto DBSortingTimeBegin = std::chrono::steady_clock::now(); // Profiling

  // If size exceeds the lower bound, limit it
  if (newStates.size() > _maxDatabaseSizeLowerBound) limitStateDatabase(newStates, _maxDatabaseSizeLowerBound);

  // Looking for and storing best/worst state
  _bestStateReward = -std::numeric_limits<float>::infinity();
  _worstStateReward = std::numeric_limits<float>::infinity();
  for (const auto& state : newStates)
  {
   if (state->reward > _bestStateReward) { _bestState = *state; _bestStateReward = _bestState.reward; }
   if (state->reward < _worstStateReward) { _worstState = *state; _worstStateReward = _worstState.reward; }
  }

  // Setting new states to be current states for the next step
  std::swap(newStates, _stateDB);

  auto DBSortingTimeEnd = std::chrono::steady_clock::now();                                                                           // Profiling
  _stepStateDBSortingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(DBSortingTimeEnd - DBSortingTimeBegin).count(); // Profiling

  // Summing state processing counters
  _totalStatesProcessedCounter += _stepNewStatesProcessedCounter;

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
  printf("[Jaffar] ----------------------------------------------------------------\n");
  printf("[Jaffar] Current Step #: %u (Max: %u)\n", _currentStep, _maxMoveCount);
  printf("[Jaffar] Worst Reward / Best Reward: %f / %f\n", _worstStateReward, _bestStateReward);
  printf("[Jaffar] Base States Performance: %.3f States/s\n", (double)_stepBaseStatesProcessedCounter / (_currentStepTime / 1.0e+9));
  printf("[Jaffar] New States Performance:  %.3f States/s\n", (double)_stepNewStatesProcessedCounter / (_currentStepTime / 1.0e+9));
  printf("[Jaffar] State size: %lu bytes\n", _stateSize);
  printf("[Jaffar] States Processed: (Step/Total): %lu / %lu\n", _stepNewStatesProcessedCounter, _totalStatesProcessedCounter);
  printf("[Jaffar] State DB Entries (Total / Max): %lu (%.3fmb) / %lu (%.3fmb)\n", _databaseSize, (double)(_databaseSize * sizeof(State)) / (1024.0 * 1024.0), _maxDatabaseSizeLowerBound, (double)(_maxDatabaseSizeLowerBound * sizeof(State)) / (1024.0 * 1024.0));
  printf("[Jaffar] Elapsed Time (Step/Total):   %3.3fs / %3.3fs\n", _currentStepTime / 1.0e+9, _searchTotalTime / 1.0e+9);
  printf("[Jaffar]   + Hash Calculation:        %3.3fs\n", _stepHashCalculationTime / 1.0e+9);
  printf("[Jaffar]   + Hash Checking:           %3.3fs\n",  _stepHashCheckingTime / 1.0e+9);
  printf("[Jaffar]   + Hash Filtering:          %3.3fs\n", _stepHashFilteringTime / 1.0e+9);
  printf("[Jaffar]   + State Advance:           %3.3fs\n", _stepStateAdvanceTime / 1.0e+9);
  printf("[Jaffar]   + State Deserialization:   %3.3fs\n", _stepStateDeserializationTime / 1.0e+9);
  printf("[Jaffar]   + State Encoding:          %3.3fs\n", _stepStateEncodingTime / 1.0e+9);
  printf("[Jaffar]   + State Decoding:          %3.3fs\n", _stepStateDecodingTime / 1.0e+9);
  printf("[Jaffar]   + State Creation:          %3.3fs\n", _stepStateCreationTime / 1.0e+9);
  printf("[Jaffar]   + State Sorting            %3.3fs\n", _stepStateDBSortingTime / 1.0e+9);
  printf("[Jaffar] New States Created Ratio (Step/Max(Step)):  %.3f, %.3f (%u)\n", _stepNewStateRatio, _maxNewStateRatio, _maxNewStateRatioStep);
  printf("[Jaffar] Max States In Memory (Step/Max): %lu (%.3fmb) / %lu (%.3fmb)\n", _stepMaxStatesInMemory, (double)(_stepMaxStatesInMemory * sizeof(State)) / (1024.0 * 1024.0), _totalMaxStatesInMemory, (double)(_totalMaxStatesInMemory * sizeof(State)) / (1024.0 * 1024.0));
  printf("[Jaffar] Max State State Difference: %u / %u\n", _maxStateDiff, _maxDifferenceCount);
  printf("[Jaffar] Hash DB Collisions (Step/Total): %lu / %lu\n", _newCollisionCounter, _hashCollisions);
  printf("[Jaffar] Hash DB Entries (Step/Total): %lu / %lu\n", _currentStep == 0 ? 0 : _hashStepNewEntries[_currentStep-1], _hashEntriesTotal);
  printf("[Jaffar] Hash DB Size (Step/Total/Max): %.3fmb, %.3fmb, <%.0f,%.0f>mb\n", _hashSizeStep, _hashSizeCurrent, _hashSizeLowerBound, _hashSizeUpperBound);
  printf("[Jaffar] Hash DB Step Threshold: %u\n", _hashStepThreshold);
  printf("[Jaffar] Best State Information:\n");

  uint8_t bestStateData[_STATE_DATA_SIZE];
  _bestState.getStateDataFromDifference(_referenceStateData, bestStateData);
  _gameInstances[0]->pushState(bestStateData);
  _gameInstances[0]->printStateInfo(_bestState.rulesStatus);

  // Print Move History
  if (_storeMoveList)
  {
   printf("[Jaffar]  + Move List: ");
   for (size_t i = 0; i < _currentStep; i++)
     printf("%s ", _possibleMoves[_bestState.getMove(i)].c_str());
   printf("\n");
  }
}

Train::Train(int argc, char *argv[])
{
  // Getting number of openMP threads
  _threadCount = omp_get_max_threads();

  // Profiling information
  _searchTotalTime = 0.0;
  _currentStepTime = 0.0;

  // Initializing counters
  _stepBaseStatesProcessedCounter = 0;
  _stepNewStatesProcessedCounter = 0;
  _totalStatesProcessedCounter = 0;
  _stepMaxStatesInMemory = 0;
  _totalMaxStatesInMemory = 0;
  _newCollisionCounter = 0;
  _hashEntriesTotal = 0;
  _hashStepThreshold = 0;
  _hashEntriesStep = 0;
  _stepNewStateRatio = 0.0;
  _maxNewStateRatio = 0.0;
  _maxNewStateRatioStep = 0;

  // Setting starting step
  _currentStep = 0;

  // Flag to determine if win state was found
  _winStateFound = false;

  // Parsing max hash DB entries
  if (const char *hashSizeLowerBoundString = std::getenv("JAFFAR_MAX_HASH_DATABASE_SIZE_LOWER_BOUND_MB")) _hashSizeLowerBound = std::stol(hashSizeLowerBoundString);
  else EXIT_WITH_ERROR("[Jaffar] JAFFAR_MAX_HASH_DATABASE_SIZE_LOWER_BOUND_MB environment variable not defined.\n");

  if (const char *hashSizeUpperBoundString = std::getenv("JAFFAR_MAX_HASH_DATABASE_SIZE_UPPER_BOUND_MB")) _hashSizeUpperBound = std::stol(hashSizeUpperBoundString);
  else EXIT_WITH_ERROR("[Jaffar] JAFFAR_MAX_HASH_DATABASE_SIZE_UPPER_BOUND_MB environment variable not defined.\n");

  // Parsing max state DB lower bound
  size_t maxDBSizeMbLowerBound = 0;
  if (const char *MaxDBMBytesLowerBoundEnvString = std::getenv("JAFFAR_MAX_STATE_DATABASE_SIZE_LOWER_BOUND_MB")) maxDBSizeMbLowerBound = std::stol(MaxDBMBytesLowerBoundEnvString);
  else EXIT_WITH_ERROR("[Jaffar] JAFFAR_MAX_STATE_DATABASE_SIZE_LOWER_BOUND_MB environment variable not defined.\n");

  size_t maxDBSizeMbUpperBound = 0;
  if (const char *MaxDBMBytesUpperBoundEnvString = std::getenv("JAFFAR_MAX_STATE_DATABASE_SIZE_UPPER_BOUND_MB")) maxDBSizeMbUpperBound = std::stol(MaxDBMBytesUpperBoundEnvString);
  else EXIT_WITH_ERROR("[Jaffar] JAFFAR_MAX_STATE_DATABASE_SIZE_UPPER_BOUND_MB environment variable not defined.\n");

  // Parsing file output frequency
  _outputSaveBestSeconds = -1.0;
  if (const char *outputSaveBestSecondsEnv = std::getenv("JAFFAR_SAVE_BEST_EVERY_SECONDS")) _outputSaveBestSeconds = std::stof(outputSaveBestSecondsEnv);

  // Parsing savegame files output path
  _outputSaveBestPath = "/tmp/jaffar";
  if (const char *outputSaveBestPathEnv = std::getenv("JAFFAR_SAVE_BEST_PATH")) _outputSaveBestPath = std::string(outputSaveBestPathEnv);

  // Parsing solution files output path
  _outputSolutionBestPath = "/tmp/jaffar.best.sol";
  if (const char *outputSolutionBestPathEnv = std::getenv("JAFFAR_SOLUTION_BEST_PATH")) _outputSolutionBestPath = std::string(outputSolutionBestPathEnv);

  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar", "2.0");

  program.add_argument("configFile")
    .help("path to the Jaffar configuration script (.jaffar) file to run.")
    .required();

  // Try to parse arguments
  try { program.parse_args(argc, argv);  }
  catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

  // Parsing config file
  std::string configFile = program.get<std::string>("configFile");
  std::string configFileString;
  auto status = loadStringFromFile(configFileString, configFile.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());

  nlohmann::json config;
  try { config = nlohmann::json::parse(configFileString); }
  catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

  // Resizing containers based on thread count
  _gameInstances.resize(_threadCount);

  // Initializing thread-specific instances
  #pragma omp parallel
  {
   // Getting thread id
   int threadId = omp_get_thread_num();

   // Doing this as a critical section so not all threads try to access files at the same time
   #pragma omp critical
   {
    _gameInstances[threadId] = new GameInstance();
    _gameInstances[threadId]->initialize(config);
   }
  }

  // Storing rule count
  _ruleCount = _gameInstances[0]->_rules.size();

  // Parsing state configuration
  State::parseConfiguration(config);

  // Calculating max DB size bounds
  _maxDatabaseSizeLowerBound = floor(((double)maxDBSizeMbLowerBound * 1024.0 * 1024.0) / ((double)_stateSize));
  _maxDatabaseSizeUpperBound = floor(((double)maxDBSizeMbUpperBound * 1024.0 * 1024.0) / ((double)_stateSize));

  printf("[Jaffar] Game initialized.\n");

  // Setting initial values
  _hasFinalized = false;
  _hashCollisions = 0;

  // Setting win status
  _winStateFound = false;

  // Computing initial hash
  const auto hash = _gameInstances[0]->computeHash();

  auto initialState = new State;
  uint8_t gameState[_STATE_DATA_SIZE];
  _gameInstances[0]->popState(gameState);

  // Storing initial state as base for differential comparison
  memcpy(_referenceStateData, gameState, _STATE_DATA_SIZE);

  // Storing initial state difference
  initialState->computeStateDifference(_referenceStateData, gameState);
  for (size_t i = 0; i < _ruleCount; i++) initialState->rulesStatus[i] = false;

  // Evaluating Rules on initial state
  _gameInstances[0]->evaluateRules(initialState->rulesStatus);

  // Evaluating Score on initial state
  initialState->reward = _gameInstances[0]->getStateReward(initialState->rulesStatus);

  // Registering hash for initial state
  _hashDB[0] = hash;

  // Copying initial state into the best/worst state
  _bestState = *initialState;
  _worstState = *initialState;
  _bestStateReward = initialState->reward;
  _worstStateReward = initialState->reward;

  // Adding state to the initial database
  _databaseSize = 1;
  _stateDB.push_back(initialState);

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
  // Timer for saving states
  auto bestStateSaveTimer = std::chrono::steady_clock::now();

  while (_hasFinalized == false)
  {
    // Sleeping for one second to prevent excessive overheads
    sleep(1);

    // Checking if we need to save best state
    if (_outputSaveBestSeconds > 0.0 && _currentStep > 1)
    {
      double bestStateTimerElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - bestStateSaveTimer).count();
      if (bestStateTimerElapsed / 1.0e+9 > _outputSaveBestSeconds)
      {
       // Storing the best and worst solution sequences
        if (_storeMoveList)
        {
         std::string bestSolutionString;
         bestSolutionString += _possibleMoves[_bestState.getMove(0)];
         for (size_t i = 1; i < _currentStep; i++)
          bestSolutionString += std::string(" ") + _possibleMoves[_bestState.getMove(i)];
         bestSolutionString += std::string(" .");
         std::string outputSolPath = _outputSaveBestPath + std::string(".best.sol");
         saveStringToFile(bestSolutionString, outputSolPath.c_str());

         std::string worstSolutionString;
         worstSolutionString += _possibleMoves[_worstState.getMove(0)];
         for (size_t i = 1; i < _currentStep; i++)
          worstSolutionString += std::string(" ") + _possibleMoves[_worstState.getMove(i)];
         worstSolutionString += std::string(" .");
         std::string outputWorstSolPath = _outputSaveBestPath + std::string(".worst.sol");
         saveStringToFile(worstSolutionString, outputWorstSolPath.c_str());
        }

        // Resetting timer
        bestStateSaveTimer = std::chrono::steady_clock::now();
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
