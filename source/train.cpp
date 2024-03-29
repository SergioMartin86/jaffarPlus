#include "train.hpp"
#include "argparse.hpp"
#include "utils.hpp"
#include <omp.h>
#include <unistd.h>
#include <algorithm>
#include <string>
#include <set>

bool Train::run()
{
  auto searchTimeBegin = std::chrono::high_resolution_clock::now();      // Profiling
  auto currentStepTimeBegin = std::chrono::high_resolution_clock::now(); // Profiling
  uint16_t firstWinStep = 0;
  bool foundSolution = false;

  while (_hasFinalized == false)
  {
    // Profiling information
    auto searchTimeEnd = std::chrono::high_resolution_clock::now();                                                            // Profiling
    _searchTotalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(searchTimeEnd - searchTimeBegin).count(); // Profiling

    auto currentStepTimeEnd = std::chrono::high_resolution_clock::now();                                                                 // Profiling
    _currentStepTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentStepTimeEnd - currentStepTimeBegin).count(); // Profiling
    currentStepTimeBegin = std::chrono::high_resolution_clock::now();                                                                    // Profiling

    // Printing search status
    printTrainStatus();

    /// Main state processing cycle begin
    computeStates();

    // Terminate if all DBs are depleted and no winning rule was found
    _databaseSize = _stateDB.size();
    if (_databaseSize == 0)
    {
      printf("[Jaffar] State database depleted with no winning states, finishing...\n");
      _hasFinalized = true;
    }

    // Terminate if a winning rule was found
    if (_winStateHistory.empty() == false)
    {
     _totalWinStatesFound = 0;
     for (const auto& step : _winStateHistory)
     {
      _totalWinStatesFound += step.second.size();
      _winStateHistoryStepBestRewards[step.first] = -std::numeric_limits<float>::infinity();
      for (const auto& state : step.second)
      {
       if (state->reward > _winStateHistoryStepBestRewards[step.first]) _winStateHistoryStepBestRewards[step.first] = state->reward;
       if (state->reward > _winStateHistoryBestReward)
       {
        _winStateHistoryBestReward = state->reward;
        _bestWinState = state;
        _bestWinStateStep = step.first;
        _bestWinState->getStateDataFromDifference(_referenceStateData, _bestWinStateData);
       }
      }
     }

     // Now considering whether the tolerance was met
     firstWinStep = _winStateHistory.begin()->first;
     if (_currentStep >= firstWinStep + _winStateStepTolerance) _hasFinalized = true;
    }

    // Terminate if maximum number of states was reached
    if (_currentStep >= _maxMoveCount)
    {
      printf("[Jaffar] Maximum step reached, finishing...\n");
      printf("[Jaffar] To run Jaffar for more steps, increase 'Max Move Count' in the .jaffar file.\n");
      _hasFinalized = true;
    }

    // Advancing step if not finished
    if (_hasFinalized == false) _currentStep++;
  }

  // Print winning state if found
  if (_showWinStateInfo == true && _winStateHistory.empty() == false)
  {
   printf("[Jaffar] Winning state reached in %u moves (%.3fs), finishing...\n", _bestWinStateStep, _searchTotalTime / 1.0e+9);
   printf("[Jaffar]  + Winning Frame Info:\n");

   foundSolution = true;
   _gameInstances[0]->pushState(_bestWinStateData);
   _gameInstances[0]->printStateInfo(_bestWinState->getRuleStatus());

    if (_storeMoveHistory && _showMoveList)
    {
     printf("[Jaffar]  + Win Move List: ");
     for (uint16_t i = 0; i < _bestWinStateStep; i++)
      printf("%s ", simplifyMove(EmuInstance::moveCodeToString(_bestWinState->getMove(i))).c_str());
     printf("\n");
    }
  }

  // Stopping show thread
  if (_outputEnabled)
  {
   if (_outputSaveFrequency > 0.0) pthread_join(_showThreadId, NULL);

   // If it has finalized with a win, save the winning state
   auto lastState = _winStateHistory.empty() ? _bestState : _bestWinState;
   auto lastStep = _winStateHistory.empty() ? _currentStep : _bestWinStateStep+1;

   // Storing the solution sequence
   if (_storeMoveHistory)
   {
    std::string solutionString;
    solutionString += EmuInstance::moveCodeToString(lastState->getMove(0));
    for (ssize_t i = 1; i < lastStep; i++) solutionString += std::string("\n") + EmuInstance::moveCodeToString(lastState->getMove(i));
    saveStringToFile(solutionString, _outputSolutionBestPath.c_str());
   }

   // Storing the solution state
   if (_storeMoveHistory)
   {
    std::string winStateString((char*)_bestWinStateData, _STATE_DATA_SIZE_TRAIN);
    saveStringToFile(winStateString, _outputStateBestPath.c_str());
   }
  }

  return foundSolution;
}

void Train::limitStateDatabase(std::vector<State*>& stateDB, size_t limit)
{
 // If global states exceed the maximum allowed, sort and truncate all excessive states
 std::nth_element(stateDB.begin(), stateDB.begin() + _maxDatabaseSizeLowerBound, stateDB.end(), [](const auto &a, const auto &b) { return a->reward > b->reward; });

 // Recycle excess states
 _freeStateQueue.insert(_freeStateQueue.end(), stateDB.begin() + limit, stateDB.end());

 // Resizing new states database to lower bound
 stateDB.resize(limit);
}

size_t Train::hashGetTotalCount() const
{
 size_t totalCount = _hashCurDB->size();
 for (const auto& hashDB : _hashPastDBs) totalCount += hashDB->size();
 return totalCount;
}

double Train::hashSizeFromEntries(const ssize_t entries) const
{
 // Just an approximation of how much the hash table requires
 return (((double)sizeof(_uint128_t) + (double)sizeof(void*) ) *(double)entries) / (512.0 * 1024.0);
};

size_t Train::hashEntriesFromSize(const double size) const
{
  // Just an approximation of how much the hash table requires
  return (size_t)((size * (512.0 * 1024.0)) / ((double)sizeof(_uint128_t) + (double)sizeof(void*)));
};

bool Train::checkHash(const _uint128_t hash) const
{
 bool collisionDetected = false;

 // Checking for past hash DBs
 for (const auto& hashDB : _hashPastDBs)
 {
  collisionDetected |= hashDB->contains(hash);
  if (collisionDetected == true) break;
 }

 // If not found in the past, check the present db
 if (collisionDetected == false) collisionDetected = !_hashCurDB->insert(hash).second;

 // Returning the state
 return collisionDetected;
}

void Train::computeStates()
{
  // Initializing counters
  _stepBaseStatesProcessedCounter = 0;
  _stepNewStatesProcessedCounter = 0;
  _newCollisionCounter = 0;
  _stepMaxStatesInMemory = 0;
  size_t newStatesCounter = 0;
  size_t startHashEntryCount = hashGetTotalCount();

  // Creating shared database for new states
  std::vector<State*> newStates;

  // Storing current source state data for decoding
  uint8_t currentSourceStateData[_STATE_DATA_SIZE_TRAIN];
  memcpy(currentSourceStateData, _referenceStateData, _STATE_DATA_SIZE_TRAIN);

  // Updating reference data with the first entry of the latest states, for encoding
  _stateDB[0]->getStateDataFromDifference(currentSourceStateData, _referenceStateData);

  // Counting levels
  std::map<uint8_t, size_t> levelCounts;

  // Initializing step timers
  _stepHashCalculationTime = 0;
  _stepHashCheckingTime = 0;
  _stepStateAdvanceTime = 0;
  _stepStateDeserializationTime = 0;
  _stepStateEncodingTime = 0;
  _stepStateDecodingTime = 0;
  _stepStateCreationTime = 0;
  _stepStateDBSortingTime = 0;
  _stepStateEvaluationTime = 0;
  _stepHashFilteringTime = 0;

  // Processing state database in parallel
  #pragma omp parallel
  {
    // Getting thread id
    int threadId = omp_get_thread_num();

    // Storage for base states
    uint8_t baseStateData[_STATE_DATA_SIZE_TRAIN];

    // Storage for copies and pointer to the base state
    State* baseStateContent = (State*) malloc(_stateSize);
    State* baseFramePointer;

    // Profiling timers
    ssize_t threadHashCalculationTime = 0;
    ssize_t threadHashCheckingTime = 0;
    ssize_t threadStateAdvanceTime = 0;
    ssize_t threadStateDeserializationTime = 0;
    ssize_t threadStateDecodingTime = 0;
    ssize_t threadStateEncodingTime = 0;
    ssize_t threadStateCreationTime = 0;
    ssize_t threadStateEvaluationTime = 0;

    // Computing always the last state while resizing the database to reduce memory footprint
    #pragma omp for
    for (auto& baseState : _stateDB)
    {
      auto t0 = std::chrono::high_resolution_clock::now(); // Profiling
      baseState->getStateDataFromDifference(currentSourceStateData, baseStateData);
      auto tf = std::chrono::high_resolution_clock::now();
      threadStateDecodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Getting possible moves for the current state
      t0 = std::chrono::high_resolution_clock::now(); // Profiling
      _gameInstances[threadId]->pushState(baseStateData);
      auto possibleMovesString = _gameInstances[threadId]->getPossibleMoves(baseState->getRuleStatus());

      // Converting possible moves to their corresponding codes
      std::vector<INPUT_TYPE> possibleMoves;
      for (const auto& move : possibleMovesString) possibleMoves.push_back(EmuInstance::moveStringToCode(move));

      // Storage to find new possible key inputs
      std::set<INPUT_TYPE> candidateMoveSet;
      std::set<INPUT_TYPE> possibleMoveSet;

      // If detecting new moves, try candidate moves too
      if (_detectPossibleMoves == true)
      {
       // Creating a set with the moves already added
       possibleMoveSet.insert(possibleMoves.begin(), possibleMoves.end());

       // Obtaining candidate moves that are not already in the possible move set
       auto candidateMoves = _gameInstances[threadId]->getCandidateMoves();
       for (const auto move : candidateMoves) if (possibleMoveSet.contains(move) == false) candidateMoveSet.insert(move);

       // Creating a sorted vector out of candidate moves, regarding the number of buttons pressed
       std::vector<INPUT_TYPE> sortedCandidateMoves(candidateMoveSet.begin(), candidateMoveSet.end());
       std::sort(sortedCandidateMoves.begin(), sortedCandidateMoves.end(), moveCountComparerNumber);

       // Now adding the new candidate move set into the possible move list
       possibleMoves.insert(possibleMoves.end(), sortedCandidateMoves.begin(), sortedCandidateMoves.end());
      }

      // Making copy of base state data and pointer
      baseStateContent->copy(baseState);
      baseFramePointer = baseState;

      tf = std::chrono::high_resolution_clock::now();
      threadStateDeserializationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

      // Getting move hash for detecting new moves
      uint64_t moveHash = 0;
      if (_detectPossibleMoves == true) moveHash = _gameInstances[threadId]->getStateMoveHash();

      // Running possible moves
      for (size_t idx = 0; idx < possibleMoves.size(); idx++)
      {
        // Increasing  states processed counter
        #pragma omp atomic
        _stepNewStatesProcessedCounter++;

        // Getting possible move and its code
        auto move = possibleMoves[idx];

        // If this comes after the first move, we need to reload the base state
        if (idx > 0)
        {
         t0 = std::chrono::high_resolution_clock::now(); // Profiling
         _gameInstances[threadId]->pushState(baseStateData);
         tf = std::chrono::high_resolution_clock::now();
         threadStateDeserializationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();
        }

        // Perform the selected move
        t0 = std::chrono::high_resolution_clock::now(); // Profiling
        _gameInstances[threadId]->advanceGameState(move);
        tf = std::chrono::high_resolution_clock::now();
        threadStateAdvanceTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // Compute hash value
        t0 = std::chrono::high_resolution_clock::now(); // Profiling
        auto hash = _gameInstances[threadId]->computeHash(_currentStep);
        tf = std::chrono::high_resolution_clock::now();
        threadHashCalculationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // Checking for the existence of the hash in the hash database
        t0 = std::chrono::high_resolution_clock::now(); // Profiling
        bool collisionDetected = checkHash(hash);
        tf = std::chrono::high_resolution_clock::now();
        threadHashCheckingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count();

        // If collision detected, discard this state
        if (collisionDetected) { _newCollisionCounter++; continue; }

        // This is a new hash. If detecting new moves, check if this is a candidate move and, if so, add it
        if (_detectPossibleMoves == true && candidateMoveSet.contains(move))
          #pragma omp critical
            _newMoveKeySet[moveHash].insert(move);

        // Storing the state data
        t0 = std::chrono::high_resolution_clock::now(); // Profiling

        // Obtaining free state from the base pointer and, if not from the queue
        State* newState;
        if (baseFramePointer != NULL)
        {
         newState = baseFramePointer;
         baseFramePointer = NULL;
        }
        else
        {
         // Grabbing lock from free state queue
         _freeStateQueueMutex.lock();

         // If we ran out of free states, gotta check if we can free up the worst states from the next state db
         if (_freeStateQueue.empty())
         {
          // Trying to limit new states DB to lower bound size and recycling its states
          #pragma omp critical(newFrameDB)
          {
           auto DBSortingTimeBegin = std::chrono::high_resolution_clock::now(); // Profiling
           if (newStates.size() > _maxDatabaseSizeLowerBound) limitStateDatabase(newStates, _maxDatabaseSizeLowerBound);
           else EXIT_WITH_ERROR("[ERROR] Ran out of free states. Increase 'State Database', 'Max Size Upper Bound (Mb)'\n");
           auto DBSortingTimeEnd = std::chrono::high_resolution_clock::now();                                                                      // Profiling
           _stepStateDBSortingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(DBSortingTimeEnd - DBSortingTimeBegin).count(); // Profiling
          }
         }

         // Taking free state pointer
          newState = _freeStateQueue.back();
         _freeStateQueue.pop_back();

         // Releasing lock
         _freeStateQueueMutex.unlock();
        }

        // Getting rule status from the base state
        newState->copy(baseStateContent);

        // Evaluating rules on the new state
        _gameInstances[threadId]->evaluateRules(newState->getRuleStatus());

        // Getting state type
        auto type = _gameInstances[threadId]->getStateType(newState->getRuleStatus());

        // Getting checkpoint level
        auto frameCheckpoint = _gameInstances[threadId]->getCheckpointLevel(newState->getRuleStatus());

        tf = std::chrono::high_resolution_clock::now(); // Profiling
        threadStateEvaluationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count(); // Profiling

        // If state type is failed, continue to the next possible move
        bool discardFrame = type == f_fail;

        // If frame does not meet checkpoint level and its step tolerance, discard it
        if (frameCheckpoint.level < _checkpointLevel && _currentStep >= _checkpointCutoffStep) discardFrame = true;

        // Discarding unnecessary frames
        if (discardFrame == true)
        {
         _freeStateQueueMutex.lock();
         _freeStateQueue.push_back(newState);
         _freeStateQueueMutex.unlock();
         continue;
        }

        // Setting checkpoint found flag
        if (frameCheckpoint.level > _checkpointLevel)
        {
         _checkpointTolerance = frameCheckpoint.tolerance;
         _checkpointCutoffStep = _currentStep + frameCheckpoint.tolerance;
         _checkpointLevel = frameCheckpoint.level;
        }

        // Storing the state data
        t0 = std::chrono::high_resolution_clock::now(); // Profiling

        // Copying move list and adding new move
        if (_storeMoveHistory)  newState->setMove(_currentStep, move);

        // Calculating current reward
        newState->reward = _gameInstances[threadId]->getStateReward(newState->getRuleStatus());

        tf = std::chrono::high_resolution_clock::now(); // Profiling
        threadStateCreationTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count(); // Profiling

        // Encoding the state data
        t0 = std::chrono::high_resolution_clock::now(); // Profiling

        uint8_t gameState[_STATE_DATA_SIZE_TRAIN];
        _gameInstances[threadId]->popState(gameState);
        newState->computeStateDifference(_referenceStateData, gameState);

        tf = std::chrono::high_resolution_clock::now(); // Profiling
        threadStateEncodingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(tf - t0).count(); // Profiling

        // If state has succeded or is a regular state, adding it in the corresponding database
        #pragma omp critical(newFrameDB)
        {
         // Storing new winning state
         if (type == f_win) _winStateHistory[_currentStep].push_back(newState);

         // Adding state to the new state database
         if (type == f_regular) newStates.push_back(newState);

         // Increasing new state counter
         newStatesCounter++;

         // Calculating the number of states in use in memory
         size_t statesInUse = _stateDB.size() - _stepBaseStatesProcessedCounter + newStates.size();

         // Increasing maximum states in use if needed
         if (statesInUse > _stepMaxStatesInMemory) _stepMaxStatesInMemory = statesInUse;
        }
      }

      // Adding used base state back into free state queue, if it wasn't recycled locally
      if (baseFramePointer != NULL)
      {
       _freeStateQueueMutex.lock();
       _freeStateQueue.push_back(baseState);
       _freeStateQueueMutex.unlock();
      }

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
     _stepStateEvaluationTime += threadStateEvaluationTime;
    }

    // Deallocating base state storage
    free(baseStateContent);
  }

  // Updating timer averages
  _stepHashCalculationTime /= _threadCount;
  _stepHashCheckingTime /= _threadCount;
  _stepStateAdvanceTime /= _threadCount;
  _stepStateDeserializationTime /= _threadCount;
  _stepStateEncodingTime /= _threadCount;
  _stepStateDecodingTime /= _threadCount;
  _stepStateCreationTime /= _threadCount;
  _stepStateEvaluationTime /= _threadCount;

  // Calculating new state ratio (new / old)
  _stepNewStateRatio = (double)newStatesCounter / (double)_stateDB.size();

  // Sorting local DB states by reward
  auto DBSortingTimeBegin = std::chrono::high_resolution_clock::now(); // Profiling

  // Clearing all old states
  _stateDB.clear();

  // Updating max states in memory counter
  if (_stepMaxStatesInMemory > _totalMaxStatesInMemory) { _totalMaxStatesInMemory = _stepMaxStatesInMemory; _maxNewStateRatio = _stepNewStateRatio; _maxNewStateRatioStep = _currentStep; }

  // If size exceeds the lower bound, limit it
  if (newStates.size() > _maxDatabaseSizeLowerBound) limitStateDatabase(newStates, _maxDatabaseSizeLowerBound);

  // Looking for and storing best/worst state
  _bestStateReward = -std::numeric_limits<float>::infinity();
  _worstStateReward = std::numeric_limits<float>::infinity();
  _bestStateMutex.lock();
  for (const auto& state : newStates)
  {
   if (state->reward > _bestStateReward) {  _bestState->copy(state); _bestStateReward = _bestState->reward; }
   if (state->reward < _worstStateReward) { _worstState->copy(state); _worstStateReward = _worstState->reward; }
  }
  _bestStateMutex.unlock();

  // Setting new states to be current states for the next step
  std::swap(newStates, _stateDB);

  // Summing state processing counters
  _totalStatesProcessedCounter += _stepNewStatesProcessedCounter;

  auto DBSortingTimeEnd = std::chrono::high_resolution_clock::now();                                                                           // Profiling
  _stepStateDBSortingTime += std::chrono::duration_cast<std::chrono::nanoseconds>(DBSortingTimeEnd - DBSortingTimeBegin).count(); // Profiling

  // Filtering old hashes if we reach the upper bound
  auto hashFilteringTimeBegin = std::chrono::high_resolution_clock::now(); // Profiling

  // Calculating and storing new entries created in this step and calculating size in mb to evaluate filtering
  _hashStepNewEntries.push_back(hashGetTotalCount() - startHashEntryCount);
  double hashSizeCurrentDB = hashSizeFromEntries(_hashCurDB->size());

  if (hashSizeCurrentDB > _hashSizeUpperBound)
  {
   // Discard one past DB
   _hashPastDBs.pop_front();

   // Push current one
   _hashPastDBs.push_back(std::move(_hashCurDB));

   // Create new current one
   _hashCurDB = std::make_unique<hashSet_t>();
   _hashCurDB->reserve(hashEntriesFromSize(_hashSizeUpperBound));

   // Updating age
   _hashDBAges.pop_front();
   _hashDBAges.push_back(_hashCurAge);
   _hashCurAge = _currentStep;
  }

  auto hashFilteringTimeEnd = std::chrono::high_resolution_clock::now();                                                                           // Profiling
  _stepHashFilteringTime = std::chrono::duration_cast<std::chrono::nanoseconds>(hashFilteringTimeEnd - hashFilteringTimeBegin).count(); // Profiling

  // Hash Statistics
  _hashCollisions += _newCollisionCounter;
  _hashEntriesStep = hashGetTotalCount() - startHashEntryCount;
  _hashSizeStep = hashSizeFromEntries(_hashEntriesStep);
  _hashSizeCurrent = hashSizeFromEntries(hashGetTotalCount());
  _hashEntriesTotal = hashGetTotalCount();
}

void Train::printTrainStatus()
{
  ssize_t totalStepTime = _stepHashCalculationTime + _stepHashCheckingTime + _stepHashFilteringTime + _stepStateAdvanceTime + _stepStateDeserializationTime + _stepStateEncodingTime + _stepStateDecodingTime + _stepStateEvaluationTime + _stepStateCreationTime + _stepStateDBSortingTime;

  printf("[Jaffar] ----------------------------------------------------------------\n");
  printf("[Jaffar] Config File: %s\n", _configFile.c_str());
  printf("[Jaffar] Current Step #: %u (Max: %u)\n", _currentStep, _maxMoveCount);
  printf("[Jaffar] Checkpoint Level / Tolerance / Cutoff: %u / %u / %u\n", _checkpointLevel, _checkpointTolerance, _checkpointCutoffStep);
  printf("[Jaffar] Worst Reward / Best Reward: %f / %f\n", _worstStateReward, _bestStateReward);

  if (_winStateStepTolerance > 0)
  {
   printf("[Jaffar] Win States Step Tolerance: %u\n", _winStateStepTolerance);
   printf("[Jaffar] Win State Best Reward:     %f\n", _winStateHistoryBestReward);
   printf("[Jaffar] Win States Found:          %lu\n", _totalWinStatesFound);
   for (const auto& step : _winStateHistoryStepBestRewards)
   printf("[Jaffar] + Step %5u: %lu  (Best: %f)\n", step.first, _winStateHistory[step.first].size(), _winStateHistoryStepBestRewards[step.first]);
  }

  printf("[Jaffar] Base States Performance: %.3f States/s\n", (double)_stepBaseStatesProcessedCounter / (_currentStepTime / 1.0e+9));
  printf("[Jaffar] New States Performance:  %.3f States/s\n", (double)_stepNewStatesProcessedCounter / (_currentStepTime / 1.0e+9));
  printf("[Jaffar] State size: %lu bytes\n", _stateSize);
  printf("[Jaffar] States Processed: (Step/Total/Invalid): %lu / %lu / %lu\n", _stepNewStatesProcessedCounter, _totalStatesProcessedCounter, _invalidStateCount);
  printf("[Jaffar] State DB Entries (Total / Max): %lu (%.3fmb)\n", _databaseSize, (double)(_databaseSize * _stateSize) / (1024.0 * 1024.0));
  printf("[Jaffar] State DB Lower / Upper Bounds:  %lu (%.3fmb) / %lu (%.3fmb)\n", _maxDatabaseSizeLowerBound, (double)(_maxDatabaseSizeLowerBound * _stateSize) / (1024.0 * 1024.0), _maxDatabaseSizeUpperBound, (double)(_maxDatabaseSizeUpperBound * _stateSize) / (1024.0 * 1024.0));
  printf("[Jaffar] Elapsed Time (Step/Total):   %3.3fs / %3.3fs\n", _currentStepTime / 1.0e+9, _searchTotalTime / 1.0e+9);

  if (_showTimingInfo)
  {
   printf("[Jaffar]   + Hash Calculation:        %5.2f%% (%lu)\n", ((double)_stepHashCalculationTime / (double)totalStepTime) * 100.0f, _stepHashCalculationTime);
   printf("[Jaffar]   + Hash Checking:           %5.2f%% (%lu)\n", ((double)_stepHashCheckingTime / (double)totalStepTime) * 100.0f, _stepHashCheckingTime);
   printf("[Jaffar]   + Hash Filtering:          %5.2f%% (%lu)\n", ((double)_stepHashFilteringTime / (double)totalStepTime) * 100.0f, _stepHashFilteringTime);
   printf("[Jaffar]   + State Advance:           %5.2f%% (%lu)\n", ((double)_stepStateAdvanceTime / (double)totalStepTime) * 100.0f, _stepStateAdvanceTime);
   printf("[Jaffar]   + State Deserialization:   %5.2f%% (%lu)\n", ((double)_stepStateDeserializationTime / (double)totalStepTime) * 100.0f, _stepStateDeserializationTime);
   printf("[Jaffar]   + State Encoding:          %5.2f%% (%lu)\n", ((double)_stepStateEncodingTime / (double)totalStepTime) * 100.0f, _stepStateEncodingTime);
   printf("[Jaffar]   + State Decoding:          %5.2f%% (%lu)\n", ((double)_stepStateDecodingTime / (double)totalStepTime) * 100.0f, _stepStateDecodingTime);
   printf("[Jaffar]   + State Evaluation:        %5.2f%% (%lu)\n", ((double)_stepStateEvaluationTime / (double)totalStepTime) * 100.0f, _stepStateEvaluationTime);
   printf("[Jaffar]   + State Creation:          %5.2f%% (%lu)\n", ((double)_stepStateCreationTime / (double)totalStepTime) * 100.0f, _stepStateCreationTime);
   printf("[Jaffar]   + State Sorting            %5.2f%% (%lu)\n", ((double)_stepStateDBSortingTime / (double)totalStepTime) * 100.0f, _stepStateDBSortingTime);
  }

  printf("[Jaffar] New States Created Ratio (Step/Max(Step)):  %.3f, %.3f (%u)\n", _stepNewStateRatio, _maxNewStateRatio, _maxNewStateRatioStep);
  printf("[Jaffar] Max States In Memory (Step/Max): %lu (%.3fmb) / %lu (%.3fmb)\n", _stepMaxStatesInMemory, (double)(_stepMaxStatesInMemory * _stateSize) / (1024.0 * 1024.0), _totalMaxStatesInMemory, (double)(_totalMaxStatesInMemory * _stateSize) / (1024.0 * 1024.0));

  if (_useXDelta3) printf("[Jaffar] XDelta3 Max State Difference: %lu / %u\n", _currentXDelta3DMaxDiff, (uint32_t)_maxXDelta3Differences);

  if (_showHashInfo)
  {
   printf("[Jaffar] Hash DB Collisions (Step/Total): %lu / %lu\n", _newCollisionCounter, _hashCollisions);
   printf("[Jaffar] Hash DB Entries (Step/Total): %lu / %lu\n", _currentStep == 0 ? 0 : _hashStepNewEntries[_currentStep-1], _hashEntriesTotal);
   printf("[Jaffar] Hash DB Size (Step/Total/Max): %.3fmb, %.3fmb, %.0fmb (%lu x %.0fmb)\n", _hashSizeStep, _hashSizeCurrent, _hashSizeUpperBound * _hashDBCount, _hashDBCount, _hashSizeUpperBound);
   for (ssize_t i = 0; i < _hashDBCount-1; i++) printf("[Jaffar]   + Hash DB %lu Size / Step: %.3fmb / %u\n", i, hashSizeFromEntries(_hashPastDBs[i]->size()), _hashDBAges[i]);
   printf("[Jaffar]   + Hash DB %lu Size / Step: %.3fmb / %u\n", _hashDBCount-1, hashSizeFromEntries(_hashCurDB->size()), _hashCurAge);
  }

  printf("[Jaffar] Best State Information:\n");

  uint8_t bestStateData[_STATE_DATA_SIZE_TRAIN];
  _bestState->getStateDataFromDifference(_referenceStateData, bestStateData);
  _gameInstances[0]->pushState(bestStateData);
  _gameInstances[0]->printStateInfo(_bestState->getRuleStatus());

  // If detecting new moves, show list here
  if (_detectPossibleMoves == true)
  {
    printf("[Jaffar] Possible Move List: \n");

    for (const auto& key : _newMoveKeySet)
    {
     std::vector<INPUT_TYPE> vec(key.second.begin(), key.second.end());
     std::sort(vec.begin(), vec.end(), moveCountComparerNumber);
     auto itr = vec.begin();
     std::string simpleMove = simplifyMove(EmuInstance::moveCodeToString(*itr));
     printf("if (stateMiniHash == 0x%04X) moveList.insert(moveList.end(), { \"%s\"", key.first, simpleMove.c_str());
     itr++;
     for (; itr != vec.end(); itr++)
     {
      std::string simpleMove = simplifyMove(EmuInstance::moveCodeToString(*itr));
       printf(", \"%s\"", simpleMove.c_str());
     }
     printf(" });\n");
     //printf("Size: %lu\n", vec.size());
    }
  }

  // Print Move History
  if (_storeMoveHistory && _showMoveList)
  {
   printf("[Jaffar]  + Move List: ");
   for (size_t i = 0; i < _currentStep; i++)
     printf("%s ", simplifyMove(EmuInstance::moveCodeToString(_bestState->getMove(i))).c_str());
   printf("\n");
  }
}

Train::Train(const nlohmann::json& config)
{
  // Creating copy of the configuration
  _config = config;

  // Getting number of openMP threads
  _threadCount = omp_get_max_threads();

  // Parsing Jaffar configuration
  if (isDefined(_config, "Jaffar Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Jaffar Configuration' key.\n");

  // Reading win state tolerance
  if (isDefined(_config["Jaffar Configuration"], "Win State Step Tolerance") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Win State Step Tolerance' key.\n");
  _winStateStepTolerance = _config["Jaffar Configuration"]["Win State Step Tolerance"].get<uint16_t>();

  // Reading whether to search for new key inputs
  if (isDefined(_config["Jaffar Configuration"], "Detect Possible Moves") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Detect Possible Moves' key.\n");
   _detectPossibleMoves = _config["Jaffar Configuration"]["Detect Possible Moves"].get<bool>();

  // Parsing database sizes
  if (isDefined(_config["Jaffar Configuration"], "State Database") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'State Database' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["State Database"], "Max Size Lower Bound (Mb)") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'State Database', 'Max Size Lower Bound (Mb)' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["State Database"], "Max Size Upper Bound (Mb)") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'State Database', 'Max Size Upper Bound (Mb)' key.\n");
  _maxDBSizeMbLowerBound = _config["Jaffar Configuration"]["State Database"]["Max Size Lower Bound (Mb)"].get<size_t>();
  _maxDBSizeMbUpperBound = _config["Jaffar Configuration"]["State Database"]["Max Size Upper Bound (Mb)"].get<size_t>();

  if (isDefined(_config["Jaffar Configuration"], "Hash Database") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Hash Database' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Hash Database"], "Max Size (Mb)") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Hash Database', 'Max Size (Mb)' key.\n");
  _hashSizeUpperBound = _config["Jaffar Configuration"]["Hash Database"]["Max Size (Mb)"].get<size_t>();

  if (isDefined(_config["Jaffar Configuration"]["Hash Database"], "Database Count") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Hash Database', 'Database Count' key.\n");
  _hashDBCount = _config["Jaffar Configuration"]["Hash Database"]["Database Count"].get<size_t>();

  // Parsing console output config
  if (isDefined(_config["Jaffar Configuration"], "Show Winning State Information") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Show Winning State Information' key.\n");
  if (isDefined(_config["Jaffar Configuration"], "Show Hash Information") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Show Winning State Information' key.\n");
  if (isDefined(_config["Jaffar Configuration"], "Show Timing Information") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Show Winning State Information' key.\n");
  if (isDefined(_config["Jaffar Configuration"], "Show Move List") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Show Move List' key.\n");
  _showMoveList = _config["Jaffar Configuration"]["Show Move List"].get<bool>();
  _showWinStateInfo = _config["Jaffar Configuration"]["Show Winning State Information"].get<bool>();
  _showHashInfo = _config["Jaffar Configuration"]["Show Hash Information"].get<bool>();
  _showTimingInfo = _config["Jaffar Configuration"]["Show Timing Information"].get<bool>();

  // Parsing file output frequency
  if (isDefined(_config["Jaffar Configuration"], "Save Intermediate Results") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Save Intermediate Results"], "Enabled") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results'.'Enabled' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Save Intermediate Results"], "Save Numbered States") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results'.'Save Numbered States' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Save Intermediate Results"], "Frequency (s)") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results'.'Frequency (s)' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Save Intermediate Results"], "Best Solution Path") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results'.'Best Solution Path' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Save Intermediate Results"], "Worst Solution Path") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results'.'Worst Solution Path' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Save Intermediate Results"], "Best State Path") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results'.'Best Solution Path' key.\n");
  if (isDefined(_config["Jaffar Configuration"]["Save Intermediate Results"], "Worst State Path") == false) EXIT_WITH_ERROR("[ERROR] Jaffar Configuration missing 'Save Intermediate Results'.'Worst Solution Path' key.\n");
  _outputEnabled = _config["Jaffar Configuration"]["Save Intermediate Results"]["Enabled"].get<bool>();
  _outputSaveNumberedStates = _config["Jaffar Configuration"]["Save Intermediate Results"]["Save Numbered States"].get<bool>();
  _outputSaveFrequency = _config["Jaffar Configuration"]["Save Intermediate Results"]["Frequency (s)"].get<float>();
  _outputSolutionBestPath = _config["Jaffar Configuration"]["Save Intermediate Results"]["Best Solution Path"].get<std::string>();
  _outputSolutionWorstPath = _config["Jaffar Configuration"]["Save Intermediate Results"]["Worst Solution Path"].get<std::string>();
  _outputStateBestPath = _config["Jaffar Configuration"]["Save Intermediate Results"]["Best State Path"].get<std::string>();
  _outputStateWorstPath = _config["Jaffar Configuration"]["Save Intermediate Results"]["Worst State Path"].get<std::string>();

  // Checking whether it contains the rules field
  if (isDefined(_config, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Rules' key.\n");

  // Checking whether it contains the emulator configuration field
  if (isDefined(_config, "Emulator Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Emulator Configuration' key.\n");

  // Checking whether it contains the Game configuration field
  if (isDefined(_config, "Game Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Game Configuration' key.\n");

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
    // Creating game and emulator instances, and parsing rules
    auto emuInstance = new EmuInstance(_config["Emulator Configuration"]);
    _gameInstances[threadId] = new GameInstance(emuInstance, _config["Game Configuration"]);
    _gameInstances[threadId]->parseRules(_config["Rules"]);
   }
  }

  // Instanciating show game instance
  auto showEmuInstance = new EmuInstance(_config["Emulator Configuration"]);
  _showGameInstance = new GameInstance(showEmuInstance, _config["Game Configuration"]);
  _showGameInstance->parseRules(_config["Rules"]);

  // Storing rule count
  _ruleCount = _gameInstances[0]->_rules.size();

  // Parsing state configuration
  if (isDefined(_config, "Jaffar Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Jaffar Configuration' key.\n");
  if (isDefined(_config["Jaffar Configuration"], "State Configuration") == false) EXIT_WITH_ERROR("[ERROR] Configuration file missing 'Jaffar Configuration' key.\n");
  State::parseConfiguration(_config["Jaffar Configuration"]["State Configuration"]);
  _stateSize = State::getSize();

  // Calculating max DB size bounds
  _maxDatabaseSizeLowerBound = floor(((double)_maxDBSizeMbLowerBound * 1024.0 * 1024.0) / ((double)_stateSize));
  _maxDatabaseSizeUpperBound = floor(((double)_maxDBSizeMbUpperBound * 1024.0 * 1024.0) / ((double)_stateSize));

  // Pre-allocating and touching State containers
  _mainStateStorage = (uint8_t*)malloc(_maxDatabaseSizeUpperBound * _stateSize);
  #pragma omp parallel for
  for (size_t i = 0; i < _maxDatabaseSizeUpperBound * _stateSize; i += 1024) *(uint8_t*)&_mainStateStorage[i] = (uint8_t)0;
  for (size_t i = 0; i < _maxDatabaseSizeUpperBound; i++) _freeStateQueue.push_back((State*)(_mainStateStorage + _stateSize * i));

  // Storing initial state
  _gameInstances[0]->popState(_initialStateData);

  // Creating storage for win, best and worst states
  _bestState = (State*) malloc(_stateSize);
  _worstState = (State*) malloc(_stateSize);

  // Initializing show thread
  if (_outputEnabled && _outputSaveFrequency > 0.0)
   if (pthread_create(&_showThreadId, NULL, showThreadFunction, this) != 0)
    EXIT_WITH_ERROR("[ERROR] Could not create show thread.\n");
}

void Train::reset()
{
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
 _invalidStateCount = 0;
 _hashEntriesTotal = 0;
 _hashEntriesStep = 0;
 _hashCurAge = 0;
 _stepNewStateRatio = 0.0;
 _maxNewStateRatio = 0.0;
 _maxNewStateRatioStep = 0;
 _checkpointLevel = 0;
 _checkpointTolerance = 0;
 _checkpointCutoffStep = 0;

 // Initializing step timers
 _stepHashCalculationTime = 0;
 _stepHashCheckingTime = 0;
 _stepStateAdvanceTime = 0;
 _stepStateDeserializationTime = 0;
 _stepStateEncodingTime = 0;
 _stepStateDecodingTime = 0;
 _stepStateCreationTime = 0;
 _stepStateDBSortingTime = 0;
 _stepStateEvaluationTime = 0;
 _stepHashFilteringTime = 0;

 _hashCollisions = 0;
 _hashEntriesStep = 0;
 _hashSizeStep = 0;
 _hashSizeCurrent = 0;
 _hashEntriesTotal = 0;

 _worstStateReward = 0.0;
 _bestStateReward = 0.0;

 _winStateHistoryBestReward = -std::numeric_limits<float>::infinity();
 _totalWinStatesFound = 0.0f;

 // Setting starting step
 _currentStep = 0;

 // Clearing state database
 limitStateDatabase(_stateDB, 0);

 // Setting initial values
 _hasFinalized = false;
 _hashCollisions = 0;

 // Computing initial hash
 const auto hash = _gameInstances[0]->computeHash(9999);

 auto _firstState = (State*) malloc(_stateSize);
 _gameInstances[0]->pushState(_initialStateData);

 // Storing initial state as base for differential comparison
 memcpy(_referenceStateData, _initialStateData, _STATE_DATA_SIZE_TRAIN);
 _firstState->computeStateDifference(_referenceStateData, _initialStateData);

 // Storing initial state difference
 for (size_t i = 0; i < _ruleCount; i++) _firstState->getRuleStatus()[i] = false;

 // Evaluating Rules on initial state
 _gameInstances[0]->evaluateRules(_firstState->getRuleStatus());

 // Evaluating Score on initial state
 _firstState->reward = _gameInstances[0]->getStateReward(_firstState->getRuleStatus());

 // Creating hash databases and registering hash for initial state
 _hashCurDB = std::make_unique<hashSet_t>();
 _hashCurDB->reserve(hashEntriesFromSize(_hashSizeUpperBound));
 for (ssize_t i = 0; i < _hashDBCount-1; i++)
 {
  _hashPastDBs.push_back(std::make_unique<hashSet_t>());
  _hashDBAges.push_back(0);
 }
 _hashCurDB->insert(hash);

 // Copying initial state into the best/worst state
 _bestState->copy(_firstState);
 _worstState->copy(_firstState);

 // Adding state to the initial database
 _databaseSize = 1;
 _stateDB.clear();
 _stateDB.push_back(_firstState);
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
  auto bestStateSaveTimer = std::chrono::high_resolution_clock::now();

  while (_hasFinalized == false)
  {
    // Sleeping for one second to prevent excessive overheads
    sleep(1);

    // Checking if we need to save best state
    if (_outputSaveFrequency > 0.0 && _currentStep > 1)
    {
      double bestStateTimerElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - bestStateSaveTimer).count();
      if (bestStateTimerElapsed / 1.0e+9 > _outputSaveFrequency)
      {

       // Storing the best and worst solution sequences
       if (_storeMoveHistory)
       {
         std::string bestSolutionString;
         bestSolutionString += EmuInstance::moveCodeToString(_bestState->getMove(0));
         for (ssize_t i = 1; i < _currentStep-1; i++) bestSolutionString += std::string("\n") + EmuInstance::moveCodeToString(_bestState->getMove(i));
         saveStringToFile(bestSolutionString, _outputSolutionBestPath.c_str());

         if (_outputSaveNumberedStates == true)
         {
          std::string solWithStep = _outputSolutionBestPath + std::string(".") + std::to_string(_currentStep);
          saveStringToFile(bestSolutionString, solWithStep.c_str());
         }

         std::string worstSolutionString;
         worstSolutionString += EmuInstance::moveCodeToString(_worstState->getMove(0));
         for (ssize_t i = 1; i < _currentStep-1; i++) worstSolutionString += std::string("\n") + EmuInstance::moveCodeToString(_worstState->getMove(i));
         saveStringToFile(worstSolutionString, _outputSolutionWorstPath.c_str());
       }

        // Resetting timer
        bestStateSaveTimer = std::chrono::high_resolution_clock::now();
      }
    }
  }
}

#ifndef DISABLE_MAIN

int main(int argc, char *argv[])
{
 // Parsing command line arguments
 argparse::ArgumentParser program("jaffar", "2.0");

 program.add_argument("configFile")
   .help("path to the Jaffar configuration script (.jaffar) file to run.")
   .required();

 // Try to parse arguments
 try { program.parse_args(argc, argv);  }
 catch (const std::runtime_error &err) { EXIT_WITH_ERROR("%s\n%s", err.what(), program.help().str().c_str()); }

 // Parsing _config file
 std::string configFile = program.get<std::string>("configFile");
 std::string configFileString;
 auto status = loadStringFromFile(configFileString, configFile.c_str());
 if (status == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from Jaffar _config file: %s\n%s \n", configFile.c_str(), program.help().str().c_str());
 nlohmann::json config;
 try { config = nlohmann::json::parse(configFileString); }
 catch (const std::exception &err) { EXIT_WITH_ERROR("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what()); }

 printf("[Jaffar] Initializing Jaffar...\n");
 Train train(config);
 train._configFile = configFile;
 train.reset();

  // Running Search
 printf("[Jaffar] Running...\n");
 auto success = train.run();

 if (success) return 0;
 return 1;
}

#endif
