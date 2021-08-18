#include "train.h"
#include "argparse.hpp"
#include "utils.h"
#include <omp.h>
#include <unistd.h>
#include <boost/sort/sort.hpp>
#include <set>

void Train::run()
{
  if (_workerId == 0)
  {
    printf("[Jaffar2] ----------------------------------------------------------------\n");
    printf("[Jaffar2] Launching Jaffar II...\n");
    printf("[Jaffar2] Using configuration file(s): "); for (size_t i = 0; i < _scriptFiles.size(); i++) printf("%s ", _scriptFiles[i].c_str()); printf("\n");
    printf("[Jaffar2] Starting search with %lu workers.\n", _workerCount);
    printf("[Jaffar2] Frame DB entries per worker: %lu\n", _maxLocalDatabaseSize);

    if (_outputSaveBestSeconds > 0)
    {
      printf("[Jaffar2] Saving best frame every: %.3f seconds.\n", _outputSaveBestSeconds);
      printf("[Jaffar2]  + Savefile Path: %s\n", _outputSaveBestPath.c_str());
      printf("[Jaffar2]  + Solution Path: %s\n", _outputSolutionBestPath.c_str());
    }

    if (_outputSaveCurrentSeconds > 0)
    {
      printf("[Jaffar2] Saving current frame every: %.3f seconds.\n", _outputSaveCurrentSeconds);
      printf("[Jaffar2]  + Savefile Path: %s\n", _outputSaveCurrentPath.c_str());
      printf("[Jaffar2]  + Solution Path: %s\n", _outputSolutionCurrentPath.c_str());
    }

    // Sleep for a second to show this message
    sleep(2);
  }

  // Wait for all workers to be ready
  MPI_Barrier(MPI_COMM_WORLD);

  auto searchTimeBegin = std::chrono::steady_clock::now();      // Profiling
  auto currentStepTimeBegin = std::chrono::steady_clock::now(); // Profiling

  // Storage for termination trigger
  bool terminate = false;

  while (terminate == false)
  {
    // If this is the root rank, plot the best frame and print information
    if (_workerId == 0)
    {
      // Profiling information
      auto searchTimeEnd = std::chrono::steady_clock::now();                                                            // Profiling
      _searchTotalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(searchTimeEnd - searchTimeBegin).count(); // Profiling

      auto currentStepTimeEnd = std::chrono::steady_clock::now();                                                                 // Profiling
      _currentStepTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentStepTimeEnd - currentStepTimeBegin).count(); // Profiling
      currentStepTimeBegin = std::chrono::steady_clock::now();                                                                    // Profiling

      // Printing search status
      printTrainStatus();

      // Updating show frames
      if (_currentFrameDB.size() > 0)
        for (size_t i = 0; i < SHOW_FRAME_COUNT; i++)
          _showFrameDB[i] = *_currentFrameDB[i % _currentFrameDB.size()];
    }

    /////////////////////////////////////////////////////////////////
    /// Main frame processing cycle begin
    /////////////////////////////////////////////////////////////////

    // 1) Workers exchange new base frames uniformly
    MPI_Barrier(MPI_COMM_WORLD);                                        // Profiling
    auto frameDistributionTimeBegin = std::chrono::steady_clock::now(); // Profiling
    distributeFrames();
    MPI_Barrier(MPI_COMM_WORLD);                                                                                                                  // Profiling
    auto frameDistributionTimeEnd = std::chrono::steady_clock::now();                                                                             // Profiling
    _frameDistributionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(frameDistributionTimeEnd - frameDistributionTimeBegin).count(); // Profiling

    // 2) Workers process their own base frames
    MPI_Barrier(MPI_COMM_WORLD);                                       // Profiling
    auto frameComputationTimeBegin = std::chrono::steady_clock::now(); // Profiling
    computeFrames();
    MPI_Barrier(MPI_COMM_WORLD);                                                                                                               // Profiling
    auto frameComputationTimeEnd = std::chrono::steady_clock::now();                                                                           // Profiling
    _frameComputationTime = std::chrono::duration_cast<std::chrono::nanoseconds>(frameComputationTimeEnd - frameComputationTimeBegin).count(); // Profiling

    // 3) Workers sort their databases and communicate partial results
    MPI_Barrier(MPI_COMM_WORLD);                                          // Profiling
    auto framePostprocessingTimeBegin = std::chrono::steady_clock::now(); // Profiling
    framePostprocessing();
    MPI_Barrier(MPI_COMM_WORLD);                                                                                                                        // Profiling
    auto framePostprocessingTimeEnd = std::chrono::steady_clock::now();                                                                                 // Profiling
    _framePostprocessingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(framePostprocessingTimeEnd - framePostprocessingTimeBegin).count(); // Profiling

    // 4) Workers exchange hash information and update hash databases
    MPI_Barrier(MPI_COMM_WORLD);                                   // Profiling
    auto hashExchangeTimeBegin = std::chrono::steady_clock::now(); // Profiling
    updateHashDatabases();
    MPI_Barrier(MPI_COMM_WORLD);                                                                                                   // Profiling
    auto hashExchangeTimeEnd = std::chrono::steady_clock::now();                                                                   // Profiling
    _hashPostprocessingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(hashExchangeTimeEnd - hashExchangeTimeBegin).count(); // Profiling

    /////////////////////////////////////////////////////////////////
    /// Main frame processing cycle end
    /////////////////////////////////////////////////////////////////

    // Terminate if DB is depleted and no winning rule was found
    if (_globalFrameCounter == 0)
    {
      if (_workerId == 0) printf("[Jaffar2] Frame database depleted with no winning frames, finishing...\n");
      terminate = true;
    }

    // Terminate if a winning rule was found
    if (_winFrameFound == true)
    {
      if (_workerId == 0) printf("[Jaffar2] Winning frame reached in %lu moves, finishing...\n", _currentStep+1);
      terminate = true;
    }

    // Terminate if maximum number of frames was reached
    if (_currentStep > _maxSteps)
    {
      if (_workerId == 0) printf("[Jaffar2] Maximum frame number reached, finishing...\n");
      terminate = true;
    }

    // Broadcasting whether a winning frame was found
    MPI_Bcast(&terminate, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    // Advancing step
    _currentStep++;

//    printf("Press Enter...\n");
//    getchar();
  }

  // Print winning frame if found
  if (_workerId == 0 && _winFrameFound == true)
  {
    printf("[Jaffar2] Win Frame Information:\n");
    _blastem->loadState(_globalWinFrame.frameStateData);
    _blastem->printState();

    printRuleStatus(_globalWinFrame);

    // Print Move History
    if (_storeMoveList)
    {
     printf("[Jaffar2]  + Move List: ");
     for (size_t i = 0; i <= _currentStep; i++)
       printf("%s ", _possibleMoves[_globalWinFrame.getMove(i)].c_str());
     printf("\n");
    }
  }

  // Marking the end of the run
  _hasFinalized = true;

  // Stopping show thread
  if (_workerId == 0) pthread_join(_showThreadId, NULL);

  // Barrier to wait for all workers
  MPI_Barrier(MPI_COMM_WORLD);
}

void Train::printRuleStatus(const Frame &frame)
{
 printf("[Jaffar2]  + Rule Status: ");
 for (size_t i = 0; i < frame.rulesStatus.size(); i++)
 {
   if (i > 0 && i % 60 == 0) printf("\n                         ");
   printf("%d", frame.rulesStatus[i] ? 1 : 0);
 }
 printf("\n");
}

void Train::distributeFrames()
{
  // Getting worker's own base frame count
  size_t localBaseFrameCount = _currentFrameDB.size();

  // Gathering all of te worker's base frame counts
  MPI_Allgather(&localBaseFrameCount, 1, MPI_UNSIGNED_LONG, _localBaseFrameCounts.data(), 1, MPI_UNSIGNED_LONG, MPI_COMM_WORLD);

  // Getting maximum frame count of all
  _maxFrameCount = 0;
  _maxFrameWorkerId = 0;
  for (size_t i = 0; i < _workerCount; i++)
   if (_localBaseFrameCounts[i] > _maxFrameCount)
    { _maxFrameCount = _localBaseFrameCounts[i]; _maxFrameWorkerId = i; }

  // Getting minimum frame count of all
  _minFrameCount = _maxFrameCount;
  _minFrameWorkerId = 0;
  for (size_t i = 0; i < _workerCount; i++)
   if (_localBaseFrameCounts[i] < _minFrameCount)
    { _minFrameCount = _localBaseFrameCounts[i]; _minFrameWorkerId = i; }

  // Figuring out work distribution
  std::vector<size_t> targetLocalNextFrameCounts = splitVector(_globalFrameCounter, _workerCount);
  std::vector<size_t> remainLocalNextFrameCounts = targetLocalNextFrameCounts;

  // Determining all-to-all send/recv counts
  std::vector<std::vector<int>> allToAllSendCounts(_workerCount);
  std::vector<std::vector<int>> allToAllRecvCounts(_workerCount);
  for (size_t i = 0; i < _workerCount; i++) allToAllSendCounts[i].resize(_workerCount, 0);
  for (size_t i = 0; i < _workerCount; i++) allToAllRecvCounts[i].resize(_workerCount, 0);

  // Iterating over sending ranks
  for (size_t sendWorkerId = 0; sendWorkerId < _workerCount; sendWorkerId++)
  {
    auto sendFramesCount = _localBaseFrameCounts[sendWorkerId];
    size_t recvWorkerId = 0;

    while (sendFramesCount > 0)
    {
     size_t maxRecv = std::min(remainLocalNextFrameCounts[recvWorkerId], 4096ul);
     size_t maxSend = std::min(sendFramesCount, 4096ul);
     size_t exchangeCount = std::min(maxRecv, maxSend);

     sendFramesCount -= exchangeCount;
     remainLocalNextFrameCounts[recvWorkerId] -= exchangeCount;
     allToAllSendCounts[sendWorkerId][recvWorkerId] += exchangeCount;
     allToAllRecvCounts[recvWorkerId][sendWorkerId] += exchangeCount;

     recvWorkerId++;
     if (recvWorkerId == _workerCount) recvWorkerId = 0;
    }
  }

  // Getting current frame count
  const size_t currentFrameDBSize = _currentFrameDB.size();

  // Passing own part of experiences to itself
  size_t ownExperienceCount = allToAllSendCounts[_workerId][_workerId];
  for (size_t i = 0; i < ownExperienceCount; i++)
  {
    // Getting last frames
    auto frame = std::move(_currentFrameDB[currentFrameDBSize - ownExperienceCount + i]);

    // Adding new frame into the data base
    _nextFrameDB.push_back(std::move(frame));
  }

  // Removing frame database
  _currentFrameDB.resize(currentFrameDBSize - ownExperienceCount);

  // Storage for MPI requests
  MPI_Request reqs[2];

  // Exchanging frames with all other workers at a one by one basis
  // This could be done more efficiently in terms of message traffic and performance by using an MPI_Alltoallv operation
  // However, this operation requires that all buffers are allocated simultaneously which puts additional pressure on memory
  // Given this bot needs as much memory as possible, the more memory efficient pair-wise communication is used
  size_t nSteps = _workerCount / 2;

  // Perform right-wise communication first
  size_t leftNeighborId = _workerId == 0 ? _workerCount - 1 : _workerId - 1;
  size_t rightNeighborId = _workerId == _workerCount - 1 ? 0 : _workerId + 1;
  for (size_t step = 0; step < nSteps; step++)
  {
    // Variables to keep track of current send and recv positions
    size_t currentSendPosition;
    size_t currentRecvPosition;

    // Getting counts for the current worker
    int leftRecvCount = allToAllRecvCounts[_workerId][leftNeighborId];
    int rightSendCount = allToAllSendCounts[_workerId][rightNeighborId];

    // Reserving exchange buffers
    char *leftRecvFrameExchangeBuffer = (char *)malloc(_frameSerializedSize * leftRecvCount);
    char *rightSendFrameExchangeBuffer = (char *)malloc(_frameSerializedSize * rightSendCount);

    // Serializing always the last frame and reducing vector size to minimize memory usage
    currentSendPosition = 0;
    if ((leftNeighborId == rightNeighborId && rightNeighborId < _workerId) == false)
      for (int i = 0; i < rightSendCount; i++)
      {
        // Getting current frame count
        const size_t count = _currentFrameDB.size();

        // Getting last frame
        const auto &frame = _currentFrameDB[count - 1];

        // Serializing frame
        frame->serialize(&rightSendFrameExchangeBuffer[currentSendPosition]);

        // Advancing send buffer pointer
        currentSendPosition += _frameSerializedSize;

        // Removing last frame by resizing
        _currentFrameDB.resize(count - 1);
      }

    // Posting receive requests
    MPI_Irecv(leftRecvFrameExchangeBuffer, leftRecvCount, _mpiFrameType, leftNeighborId, 0, MPI_COMM_WORLD, &reqs[0]);

    // Making sure receives are posted before sends to prevent intermediate buffering
    MPI_Barrier(MPI_COMM_WORLD);

    // Posting send requests
    MPI_Isend(rightSendFrameExchangeBuffer, rightSendCount, _mpiFrameType, rightNeighborId, 0, MPI_COMM_WORLD, &reqs[1]);

    // Waiting for requests to finish
    MPI_Waitall(2, reqs, MPI_STATUS_IGNORE);

    // Freeing send buffers
    free(rightSendFrameExchangeBuffer);

    // Waiting for everyone to have cleaned their buffers before creating new frames
    MPI_Barrier(MPI_COMM_WORLD);

    currentRecvPosition = 0;
    if ((leftNeighborId == rightNeighborId && rightNeighborId > _workerId) == false)
      for (int frameId = 0; frameId < leftRecvCount; frameId++)
      {
        // Creating new frame
        auto newFrame = std::make_unique<Frame>();

        // Deserializing frame
        newFrame->deserialize(&leftRecvFrameExchangeBuffer[currentRecvPosition]);

        // Adding new frame into the data base
        _nextFrameDB.push_back(std::move(newFrame));

        // Advancing send buffer pointer
        currentRecvPosition += _frameSerializedSize;
      }

    // Freeing left right receive buffer
    free(leftRecvFrameExchangeBuffer);

    // Waiting for everyone to have cleaned their buffers before continuing
    MPI_Barrier(MPI_COMM_WORLD);

    // Shifting neighbors by one
    if (leftNeighborId == 0)
      leftNeighborId = _workerCount - 1;
    else
      leftNeighborId = leftNeighborId - 1;

    if (rightNeighborId == _workerCount - 1)
      rightNeighborId = 0;
    else
      rightNeighborId = rightNeighborId + 1;
  }

  // Perform left-wise communication second
  leftNeighborId = _workerId == 0 ? _workerCount - 1 : _workerId - 1;
  rightNeighborId = _workerId == _workerCount - 1 ? 0 : _workerId + 1;
  for (size_t step = 0; step < nSteps; step++)
  {
    // Variables to keep track of current send and recv positions
    size_t currentSendPosition;
    size_t currentRecvPosition;

    // Getting counts for the current worker
    int leftSendCount = allToAllSendCounts[_workerId][leftNeighborId];
    int rightRecvCount = allToAllRecvCounts[_workerId][rightNeighborId];

    // Reserving exchange buffers
    char *leftSendFrameExchangeBuffer = (char *)malloc(_frameSerializedSize * leftSendCount);
    char *rightRecvFrameExchangeBuffer = (char *)malloc(_frameSerializedSize * rightRecvCount);

    // Serializing always the last frame and reducing vector size to minimize memory usage
    currentSendPosition = 0;
    if ((leftNeighborId == rightNeighborId && rightNeighborId > _workerId) == false)
      for (int i = 0; i < leftSendCount; i++)
      {
        // Getting current frame count
        const size_t count = _currentFrameDB.size();

        // Getting last frame
        const auto &frame = _currentFrameDB[count - 1];

        // Serializing frame
        frame->serialize(&leftSendFrameExchangeBuffer[currentSendPosition]);

        // Advancing send buffer pointer
        currentSendPosition += _frameSerializedSize;

        // Removing last frame by resizing
        _currentFrameDB.resize(count - 1);
      }

    // Posting receive requests
    MPI_Irecv(rightRecvFrameExchangeBuffer, rightRecvCount, _mpiFrameType, rightNeighborId, 0, MPI_COMM_WORLD, &reqs[0]);

    // Making sure receives are posted before sends to prevent intermediate buffering
    MPI_Barrier(MPI_COMM_WORLD);

    // Posting send requests
    MPI_Isend(leftSendFrameExchangeBuffer, leftSendCount, _mpiFrameType, leftNeighborId, 0, MPI_COMM_WORLD, &reqs[1]);

    // Waiting for requests to finish
    MPI_Waitall(2, reqs, MPI_STATUS_IGNORE);

    // Freeing send buffers
    free(leftSendFrameExchangeBuffer);

    // Waiting for everyone to have cleaned their buffers before creating new frames
    MPI_Barrier(MPI_COMM_WORLD);

    // Deserializing contiguous buffer into the new frame database
    currentRecvPosition = 0;
    if ((leftNeighborId == rightNeighborId && rightNeighborId < _workerId) == false)
      for (int frameId = 0; frameId < rightRecvCount; frameId++)
      {
        // Creating new frame
        auto newFrame = std::make_unique<Frame>();

        // Deserializing frame
        newFrame->deserialize(&rightRecvFrameExchangeBuffer[currentRecvPosition]);

        // Adding new frame into the data base
        _nextFrameDB.push_back(std::move(newFrame));

        // Advancing send buffer pointer
        currentRecvPosition += _frameSerializedSize;
      }

    // Freeing right receive buffer
    free(rightRecvFrameExchangeBuffer);

    // Waiting for everyone to have cleaned their buffers before continuing
    MPI_Barrier(MPI_COMM_WORLD);

    // Shifting neighbors by one
    if (leftNeighborId == 0)
      leftNeighborId = _workerCount - 1;
    else
      leftNeighborId = leftNeighborId - 1;

    if (rightNeighborId == _workerCount - 1)
      rightNeighborId = 0;
    else
      rightNeighborId = rightNeighborId + 1;
  }

  // Swapping database pointers
  _currentFrameDB = std::move(_nextFrameDB);

  // If global frames exceed 1.3x the maximum allowed, sor and truncate all excessive frames
  if (_globalFrameCounter > 1.3 * _maxGlobalDatabaseSize)
   if (_currentFrameDB.size() > _maxLocalDatabaseSize)
   {
    boost::sort::block_indirect_sort(_currentFrameDB.begin(), _currentFrameDB.end(), [](const auto &a, const auto &b) { return a->reward > b->reward; });
    _currentFrameDB.resize(_maxLocalDatabaseSize);
   }
}

void Train::computeFrames()
{
  // Initializing counters
  _localStepFramesProcessedCounter = 0;
  _newCollisionCounter = 0;

  // Creating thread-local storage for new frames
  std::vector<std::unique_ptr<Frame>> newFrames;

//  _blastem->initialize(_romFilePath.c_str(), _saveFilePath.c_str(), true, true);

  // Computing always the last frame while resizing the database to reduce memory footprint
  for (size_t baseFrameIdx = 0; baseFrameIdx < _currentFrameDB.size(); baseFrameIdx++)
  {
    // Storage for the base frame
    const auto& baseFrame = _currentFrameDB[baseFrameIdx];

    // Getting possible moves for the current frame
    auto gameState = _blastem->getGameState(baseFrame->frameStateData);
    std::vector<uint8_t> possibleMoveIds = _blastem->getPossibleMoveIds(gameState);

    // If the restart flag is activated, then also try hitting Ctrl+A
    if (baseFrame->isRestart == true)
    {
     possibleMoveIds.push_back(14);
     baseFrame->isRestart = false;
    }

    // Running possible moves
    for (size_t idx = 0; idx < possibleMoveIds.size(); idx++)
    {
      // Increasing  frames processed counter
      _localStepFramesProcessedCounter++;

      // Getting possible move id
      auto moveId = possibleMoveIds[idx];

      // Getting possible move string
      std::string move = _possibleMoves[moveId].c_str();

      // Loading frame state
      _blastem->loadState(baseFrame->frameStateData);

      // Perform the selected move
      int error = _blastem->playFrame(move);
      if (error == 1)
      {
       _blastem->initialize(_romFilePath.c_str(), _saveFilePath.c_str(), true, true);
      }

      // Compute hash value
      auto hash = _blastem->computeHash();

      // Checking for the existence of the hash in the hash databases
      bool collisionDetected = _hashDatabase.contains(hash);

      // If no collision detected with the normal databases, check the new hash DB
      if (collisionDetected == false)
       {
         collisionDetected |= _newHashes.contains(hash);

         // If now there, add it now
         if (collisionDetected == false) _newHashes.insert(hash);
       }

      // If collision detected, increase collision counter
      if (collisionDetected) _newCollisionCounter++;

      // If collision detected, discard this frame
      if (collisionDetected) continue;

      // Creating new frame, mixing base frame information and the current blastem state
      auto newFrame = std::make_unique<Frame>();
      newFrame->rulesStatus = baseFrame->rulesStatus;

      // If required, store move history
      if (_storeMoveList == true)
      {
       newFrame->moveHistory = baseFrame->moveHistory;
       newFrame->setMove(_currentStep, moveId);
      }

      // Evaluating rules on the new frame
      evaluateRules(*newFrame);

      // Check whether the frame needs to be flushed because it didn't reach a certain checkpoint
      bool isFlushedFrame = checkFlush(*newFrame);

      // If frame is to be flushed, discard it and proceed to the next one
      if (isFlushedFrame) continue;

      // Checks whether any fail rules were activated
      bool isFailFrame = checkFail(*newFrame);

      // If frame has failed, discard it and proceed to the next one
      if (isFailFrame) continue;

      // Check special actions for this state
      checkSpecialActions(*newFrame);

      // Storing the frame data
      _blastem->saveState(newFrame->frameStateData);

      // Calculating current reward
      newFrame->reward = getFrameReward(*newFrame);

      // Check if the frame triggers a win condition
      bool isWinFrame = checkWin(*newFrame);

      // If frame has succeded, then flag it
      if (isWinFrame)
      {
        _localWinFound = true;
         _localWinFrame = *newFrame;
      }

      // Contributing to flush mask
      addFlushMask(*newFrame);

      // Adding novel frame in the next frame database
      _nextFrameDB.push_back(std::move(newFrame));
    }

    // Freeing memory for the used base frame
    _currentFrameDB[baseFrameIdx].reset();
  }
}

void Train::framePostprocessing()
{
  // Updating flushing mask globally
  auto localFlushMask = _flushingMask;
  MPI_Allreduce(localFlushMask.data(), _flushingMask.data(), _ruleCount, MPI_UINT8_T, MPI_BOR, MPI_COMM_WORLD);

  // Clearing current frame DB
  _currentFrameDB.clear();

  // Sorting local DB frames by reward
  boost::sort::block_indirect_sort(_nextFrameDB.begin(), _nextFrameDB.end(), [](const auto &a, const auto &b) { return a->reward > b->reward; });

  // Calculating global frame count
  size_t localFrameDatabaseSize = _nextFrameDB.size();
  MPI_Allreduce(&localFrameDatabaseSize, &_globalFrameCounter, 1, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD);

  // Calculating how many frames need to be cut
  size_t framesToCut = _globalFrameCounter >_maxGlobalDatabaseSize ? _globalFrameCounter - _maxGlobalDatabaseSize : 0;

  // Approximating to the cutoff score logarithmically
  size_t globalCurrentFramesCut = _globalFrameCounter;

  // Declaring an initially permissive threshold
  float currentCutoffScore = -1.0f;

  int passingFramesIdx = 0;
  if (framesToCut > 0)
  {
   float upperBound = _globalBestFrameScore;
   float lowerBound = 0;

   // With 24 steps of binary search, we're pretty sure we found a balance
   for (size_t i = 0; i < 24; i++)
   {
    // Setting cutoff at the middle
    currentCutoffScore = std::floor((upperBound + lowerBound) * 0.5f);

    // Adjusting index to the last frame to satisfy the cutoff in the sorted database
    while (passingFramesIdx < localFrameDatabaseSize && _nextFrameDB[passingFramesIdx]->reward >= currentCutoffScore) passingFramesIdx++;
    if (passingFramesIdx == localFrameDatabaseSize) passingFramesIdx--;
    while (passingFramesIdx > 0 && _nextFrameDB[passingFramesIdx]->reward <= currentCutoffScore) passingFramesIdx--;
    size_t localCurrentFramesCut = localFrameDatabaseSize - passingFramesIdx;

    // Getting global frame cutoff
    MPI_Allreduce(&localCurrentFramesCut, &globalCurrentFramesCut, 1, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD);

    // Moving the lower/upper bounds, depending on the case
    if (globalCurrentFramesCut > framesToCut) upperBound = currentCutoffScore;
    if (globalCurrentFramesCut < framesToCut) lowerBound = currentCutoffScore;
   }
  }

  // Finding local best frame reward
  float localBestFrameScore = -std::numeric_limits<float>::infinity();
  if (_nextFrameDB.empty() == false) localBestFrameScore = _nextFrameDB[0]->reward;

  // If there are remaining frames, find best global frame reward/win
  if (_globalFrameCounter > 0)
  {
   struct mpiLoc { float val; int loc; };
   mpiLoc mpiLocInput;
   mpiLoc mpiLocResult;
   mpiLocInput.val = localBestFrameScore;
   mpiLocInput.loc = _workerId;
   MPI_Allreduce(&mpiLocInput, &mpiLocResult, 1, MPI_FLOAT_INT, MPI_MAXLOC, MPI_COMM_WORLD);
   int globalBestFrameRank = mpiLocResult.loc;

   // Serializing, broadcasting, and deserializing best frame
   char frameBcastBuffer[_frameSerializedSize];
   if (_workerId == (size_t)globalBestFrameRank) _nextFrameDB[0]->serialize(frameBcastBuffer);
   MPI_Bcast(frameBcastBuffer, 1, _mpiFrameType, globalBestFrameRank, MPI_COMM_WORLD);
   _bestFrame.deserialize(frameBcastBuffer);
   _bestFrame.reward = getFrameReward(_bestFrame);

   // Exchanging the fact that a win frame has been found
   int winRank = -1;
   int localHasWinFrame = _localWinFound == false ? 0 : 1;
   std::vector<int> globalHasWinFrameVector(_workerCount);
   MPI_Allgather(&localHasWinFrame, 1, MPI_INT, globalHasWinFrameVector.data(), 1, MPI_INT, MPI_COMM_WORLD);
   for (size_t i = 0; i < _workerCount; i++)
     if (globalHasWinFrameVector[i] == 1) winRank = i;

   // If win frame found, broadcast it
   if (winRank >= 0)
   {
     char winRankBuffer[_frameSerializedSize];
     if ((size_t)winRank == _workerId) _localWinFrame.serialize(winRankBuffer);
     MPI_Bcast(winRankBuffer, 1, _mpiFrameType, winRank, MPI_COMM_WORLD);
     _globalWinFrame.deserialize(winRankBuffer);
     _winFrameFound = true;
   }
  }

  // Copying frames which pass the cutoff into the database
  _currentFrameDB.clear();
  for (size_t i = 0; i < localFrameDatabaseSize; i++)
   if (_nextFrameDB[i]->reward >= currentCutoffScore)
   _currentFrameDB.push_back(std::move(_nextFrameDB[i]));

  // Clearing next frame DB
  _nextFrameDB.clear();

  // Updating global frame counter
  size_t localBaseFrameCount = _currentFrameDB.size();
  MPI_Allreduce(&localBaseFrameCount, &_globalFrameCounter, 1, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD);

  // Finding global best frame reward
  MPI_Allreduce(&localBestFrameScore, &_globalBestFrameScore, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

  // Summing frame processing counters
  MPI_Allreduce(&_localStepFramesProcessedCounter, &_stepFramesProcessedCounter, 1, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD);
  _totalFramesProcessedCounter += _stepFramesProcessedCounter;
}

void Train::updateHashDatabases()
{
 // Discarding older hashes
 if (_currentStep % HASHDB_AGE_CUTOFF_FREQ == 0)
  for (auto hashItr = _hashDatabase.cbegin(); hashItr != _hashDatabase.cend();)
   if (_currentStep - hashItr->second > _hashAgeThreshold) _hashDatabase.erase(hashItr++);
   else hashItr++;

 // Gathering number of new hash entries
 int localNewHashEntryCount = (int)_newHashes.size();
 std::vector<int> globalNewHashEntryCounts(_workerCount);
 MPI_Allgather(&localNewHashEntryCount, 1, MPI_INT, globalNewHashEntryCounts.data(), 1, MPI_INT, MPI_COMM_WORLD);

 // Serializing new hash entries
 std::vector<uint64_t> localNewHashVector;
 localNewHashVector.resize(localNewHashEntryCount);
 size_t curPos = 0;
 for (const auto &hash : _newHashes) localNewHashVector[curPos++] = hash;

 // Freeing new hashes vector
 _newHashes.clear();

 // We use one broadcast operation per worker (as opposed to allgatherv) to reduce the amount of memory used at any given time
 for (size_t curWorkerId = 0; curWorkerId < _workerCount; curWorkerId++)
 {
  size_t workerNewHashCount = globalNewHashEntryCounts[curWorkerId];

  // Allocating memory
  std::vector<uint64_t> workerNewHashVector(workerNewHashCount);

  // Depending on the caller rank, we use a fresh buffer or the new hash vector
  void* hashBuffer = _workerId == curWorkerId ? localNewHashVector.data() : workerNewHashVector.data();

  // Broadcasting worker's new hashes
  MPI_Bcast(hashBuffer, workerNewHashCount, MPI_UNSIGNED_LONG, curWorkerId, MPI_COMM_WORLD);

  // Adding new hashes into the database
  for (size_t i = 0; i < workerNewHashCount; i++)
   _hashDatabase.insert({ ((uint64_t*)hashBuffer)[i], _currentStep});
 }

 // Finding global collision counter
 size_t globalNewCollisionCounter;
 MPI_Allreduce(&_newCollisionCounter, &globalNewCollisionCounter, 1, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD);
 _globalHashCollisions += globalNewCollisionCounter;
}

void Train::evaluateRules(Frame &frame)
{
  for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  {
    // Evaluate rule only if it's active
    if (frame.rulesStatus[ruleId] == false)
    {
      // Checking dependencies first. If not met, continue to the next rule
      bool dependenciesMet = true;
      for (size_t i = 0; i < _rules[ruleId]->_dependenciesIndexes.size(); i++)
        if (frame.rulesStatus[_rules[ruleId]->_dependenciesIndexes[i]] == false)
          dependenciesMet = false;

      // If dependencies aren't met, then continue to next rule
      if (dependenciesMet == false) continue;

      // Checking if conditions are met
      bool isSatisfied = _rules[ruleId]->evaluate();

      // If it's achieved, update its status and run its actions
      if (isSatisfied) satisfyRule(frame, ruleId);
    }
  }
}

void Train::checkSpecialActions(const Frame &frame)
{
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (frame.rulesStatus[ruleId] == true)
  {
   // Checking if this rule makes guard disappear
   if (_rules[ruleId]->_isRemoveGuard == true)
   {
    _blastem->_state.guardPositionY = 250;
    _blastem->_state.guardPositionX = 250;
   }
  }
}

bool Train::checkFail(const Frame &frame)
{
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (frame.rulesStatus[ruleId] == true)
   if (_rules[ruleId]->_isFailRule == true)
    return true;

 return false;
}

bool Train::checkWin(const Frame &frame)
{
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (frame.rulesStatus[ruleId] == true)
   if (_rules[ruleId]->_isWinRule == true)
    return true;

 return false;
}

bool Train::checkFlush(const Frame &frame)
{
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (_flushingMask[ruleId] == 1 && frame.rulesStatus[ruleId] == false)
   return true;

 return false;
}

void Train::addFlushMask(const Frame &frame)
{
 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (frame.rulesStatus[ruleId] == true)
   if (_rules[ruleId]->_isFlushRule == true)
    _flushingMask[ruleId] = 1;
}

float Train::getRuleRewards(const Frame &frame)
{
 float reward = 0.0;

 for (size_t ruleId = 0; ruleId < _rules.size(); ruleId++)
  if (frame.rulesStatus[ruleId] == true)
    reward += _rules[ruleId]->_reward;
 return reward;
}

void Train::satisfyRule(Frame &frame, const size_t ruleId)
{
 // Recursively run actions for the yet unsatisfied rules that are satisfied by this one and mark them as satisfied
 for (size_t satisfiedIdx = 0; satisfiedIdx < _rules[ruleId]->_satisfiesIndexes.size(); satisfiedIdx++)
 {
  // Obtaining index
  size_t subRuleId = _rules[ruleId]->_satisfiesIndexes[satisfiedIdx];

  // Only activate it if it hasn't been activated before
  if(frame.rulesStatus[subRuleId] == false)
   satisfyRule(frame, subRuleId);
 }

 // Setting status to satisfied
 frame.rulesStatus[ruleId] = true;

 // It it contained a restart action, set it now (so it doesn't repeat later)
 if (_rules[ruleId]->_isRestartRule) frame.isRestart = true;
}

void Train::printTrainStatus()
{
  printf("[Jaffar2] ----------------------------------------------------------------\n");
  printf("[Jaffar2] Current Step #: %lu / %lu\n", _currentStep, _maxSteps);
  printf("[Jaffar2] Best Reward: %f\n", _globalBestFrameScore);
  printf("[Jaffar2] Database Size: %lu / ~%lu\n", _globalFrameCounter, _maxLocalDatabaseSize * _workerCount);
  printf("[Jaffar2] Frames Processed: (Step/Total): %lu / %lu\n", _stepFramesProcessedCounter, _totalFramesProcessedCounter);
  printf("[Jaffar2] Elapsed Time (Step/Total): %3.3fs / %3.3fs\n", _currentStepTime / 1.0e+9, _searchTotalTime / 1.0e+9);
  printf("[Jaffar2] Performance: %.3f Frames/s\n", (double)_stepFramesProcessedCounter / (_currentStepTime / 1.0e+9));

  printf("[Jaffar2] Frame Distribution Time:   %3.3fs\n", _frameDistributionTime / 1.0e+9);
  printf("[Jaffar2] Frame Computation Time:    %3.3fs\n", _frameComputationTime / 1.0e+9);
  printf("[Jaffar2] Hash Postprocessing Time:  %3.3fs\n", _hashPostprocessingTime / 1.0e+9);
  printf("[Jaffar2] Frame Postprocessing Time: %3.3fs\n", _framePostprocessingTime / 1.0e+9);

  printf("[Jaffar2] Frame DB Entries: Min %lu (Worker: %lu) / Max %lu (Worker: %lu)\n", _minFrameCount, _minFrameWorkerId, _maxFrameCount, _maxFrameWorkerId);
  printf("[Jaffar2] Hash DB Collisions: %lu\n", _globalHashCollisions);
  printf("[Jaffar2] Hash DB Entries: %lu\n", _hashDatabase.size());

  printf("[Jaffar2] Frame DB Size: Min %.3fmb  / Max: %.3fmb\n", (double)(_minFrameCount * Frame::getSerializationSize()) / (1024.0 * 1024.0), (double)(_maxFrameCount * Frame::getSerializationSize()) / (1024.0 * 1024.0));
  printf("[Jaffar2] Hash DB Size: %.3fmb\n", (double)(_hashDatabase.size() * sizeof(std::pair<uint64_t, uint32_t>)) / (1024.0 * 1024.0));

  printf("[Jaffar2] Best Frame Information:\n");

  _blastem->loadState(_bestFrame.frameStateData);
  _blastem->printState();

  printRuleStatus(_bestFrame);

  // Getting kid room
  int kidCurrentRoom = _blastem->_state.kidRoom;

  // Getting magnet values for the kid
  auto kidMagnet = getKidMagnetValues(_bestFrame, kidCurrentRoom);

  printf("[Jaffar2]  + Kid Horizontal Magnet Intensity / Position: %.1f / %.0f\n", kidMagnet.intensityX, kidMagnet.positionX);
  printf("[Jaffar2]  + Kid Vertical Magnet Intensity: %.1f\n", kidMagnet.intensityY);

  // Getting guard room
  int guardCurrentRoom = _blastem->_state.guardRoom;

  // Getting magnet values for the guard
  auto guardMagnet = getGuardMagnetValues(_bestFrame, guardCurrentRoom);

  printf("[Jaffar2]  + Guard Horizontal Magnet Intensity / Position: %.1f / %.0f\n", guardMagnet.intensityX, guardMagnet.positionX);
  printf("[Jaffar2]  + Guard Vertical Magnet Intensity: %.1f\n", guardMagnet.intensityY);

  // Print Move History
  if (_storeMoveList)
  {
   printf("[Jaffar2]  + Last 30 Moves: ");
   size_t startMove = (size_t)std::max((int)0, (int)_currentStep-30);
   for (size_t i = startMove; i <= _currentStep; i++)
     printf("%s ", _possibleMoves[_bestFrame.getMove(i)].c_str());
   printf("\n");
  }
  fflush(stdout);
}

magnetInfo_t Train::getKidMagnetValues(const Frame &frame, const int room)
{
 // Storage for magnet information
 magnetInfo_t magnetInfo;
 magnetInfo.positionX = 0.0f;
 magnetInfo.intensityX = 0.0f;
 magnetInfo.intensityY = 0.0f;

 // Iterating rule vector
 for (size_t ruleId = 0; ruleId < frame.rulesStatus.size(); ruleId++)
 {
  if (frame.rulesStatus[ruleId] == true)
  {
    const auto& rule = _rules[ruleId];

    for (size_t i = 0; i < _rules[ruleId]->_kidMagnetPositionX.size(); i++)
     if (rule->_kidMagnetPositionX[i].room == room)
      magnetInfo.positionX = rule->_kidMagnetPositionX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_kidMagnetIntensityX.size(); i++)
     if (rule->_kidMagnetIntensityX[i].room == room)
      magnetInfo.intensityX = rule->_kidMagnetIntensityX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_kidMagnetIntensityY.size(); i++)
     if (rule->_kidMagnetIntensityY[i].room == room)
      magnetInfo.intensityY = rule->_kidMagnetIntensityY[i].value;
  }
 }

 return magnetInfo;
}

magnetInfo_t Train::getGuardMagnetValues(const Frame &frame, const int room)
{
 // Storage for magnet information
 magnetInfo_t magnetInfo;
 magnetInfo.positionX = 0.0f;
 magnetInfo.intensityX = 0.0f;
 magnetInfo.intensityY = 0.0f;

 // Iterating rule vector
 for (size_t ruleId = 0; ruleId < frame.rulesStatus.size(); ruleId++)
  if (frame.rulesStatus[ruleId] == true)
  {
    const auto& rule = _rules[ruleId];

    for (size_t i = 0; i < _rules[ruleId]->_guardMagnetPositionX.size(); i++)
     if (rule->_guardMagnetPositionX[i].room == room)
      magnetInfo.positionX = rule->_guardMagnetPositionX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_guardMagnetIntensityX.size(); i++)
     if (rule->_guardMagnetIntensityX[i].room == room)
      magnetInfo.intensityX = rule->_guardMagnetIntensityX[i].value;

    for (size_t i = 0; i < _rules[ruleId]->_guardMagnetIntensityY.size(); i++)
     if (rule->_guardMagnetIntensityY[i].room == room)
      magnetInfo.intensityY = rule->_guardMagnetIntensityY[i].value;
  }

 return magnetInfo;
}

float Train::getFrameReward(const Frame &frame)
{
  // Accumulator for total reward
  float reward = getRuleRewards(frame);

  // Getting kid room
  int kidCurrentRoom = _blastem->_state.kidRoom;

  // Getting magnet values for the kid
  auto kidMagnet = getKidMagnetValues(frame, kidCurrentRoom);

  // Getting kid's current frame
  const auto curKidFrame = _blastem->_state.kidFrame;

  // Evaluating kidMagnet's reward on the X axis
  const float kidDiffX = std::abs(_blastem->_state.kidPositionX - kidMagnet.positionX);
  reward += (float) kidMagnet.intensityX * (512.0f - kidDiffX);

  // For positive Y axis kidMagnet, rewarding climbing frames
  if ((float) kidMagnet.intensityY > 0.0f)
    reward += (float) kidMagnet.intensityY * (512.0f - _blastem->_state.kidPositionY);

  // For negative Y axis kidMagnet, rewarding falling/climbing down frames
  if ((float) kidMagnet.intensityY < 0.0f)
    reward += (float) -1.0f * kidMagnet.intensityY * (_blastem->_state.kidPositionY);

  // Getting guard room
  int guardCurrentRoom = _blastem->_state.guardRoom;

  // Getting magnet values for the guard
  auto guardMagnet = getGuardMagnetValues(frame, guardCurrentRoom);

  // Getting guard's current frame
  const auto curGuardFrame = _blastem->_state.guardFrame;

  // Evaluating guardMagnet's reward on the X axis
  const float guardDiffX = std::abs(_blastem->_state.guardPositionX - guardMagnet.positionX);
  reward += (float) guardMagnet.intensityX * (512.0f - guardDiffX);

  // For positive Y axis guardMagnet
  if ((float) guardMagnet.intensityY > 0.0f)
  {
   // Adding absolute reward for Y position
   reward += (float) guardMagnet.intensityY * (512.0f - _blastem->_state.guardPositionY);
  }

  // For negative Y axis guardMagnet, rewarding falling/climbing down frames
  if ((float) guardMagnet.intensityY < 0.0f)
  {
    // Falling start
    if (curGuardFrame >= 102 && curGuardFrame <= 105) reward += -1.0f * (float) guardMagnet.intensityY;

    // Falling itself
    if (curGuardFrame == 106) reward += -2.0f + (float) guardMagnet.intensityY;

    // Adding absolute reward for Y position
    reward += (float) -1.0f * guardMagnet.intensityY * (_blastem->_state.guardPositionY);
  }

  // Returning reward
  return reward;
}

Train::Train(int argc, char *argv[])
{
  // Initialize the MPI environment
  MPI_Init(&argc, &argv);

  // Get the number of processes
  int mpiSize;
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  _workerCount = (size_t)mpiSize;

  // Get the rank of the process
  int mpiRank;
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
  _workerId = (size_t)mpiRank;

  // Profiling information
  _searchTotalTime = 0.0;
  _currentStepTime = 0.0;
  _frameDistributionTime = 0.0;
  _frameComputationTime = 0.0;
  _framePostprocessingTime = 0.0;

  // Initializing counters
  _stepFramesProcessedCounter = 0;
  _totalFramesProcessedCounter = 0;
  _newCollisionCounter = 0;
  _localStepFramesProcessedCounter = 0;

  // Setting starting step
  _currentStep = 0;

  // Parsing max hash DB entries
  if (const char *hashAgeThresholdString = std::getenv("JAFFAR2_HASH_AGE_THRESHOLD"))
   _hashAgeThreshold = std::stol(hashAgeThresholdString);
  else if (_workerId == 0)
   EXIT_WITH_ERROR("[Jaffar2] JAFFAR2_HASH_AGE_THRESHOLD environment variable not defined.\n");

  // Parsing max frame DB entries
  size_t frameDBMaxMBytes = 0;
  if (const char *frameDBMaxMBytesEnv = std::getenv("JAFFAR2_MAX_WORKER_FRAME_DATABASE_SIZE_MB"))
    frameDBMaxMBytes = std::stol(frameDBMaxMBytesEnv);
  else if (_workerId == 0)
   EXIT_WITH_ERROR("[Jaffar2] JAFFAR2_MAX_WORKER_FRAME_DATABASE_SIZE_MB environment variable not defined. Using conservative default...\n");

  // Parsing file output frequency
  _outputSaveBestSeconds = -1.0;
  if (const char *outputSaveBestSecondsEnv = std::getenv("JAFFAR2_SAVE_BEST_EVERY_SECONDS")) _outputSaveBestSeconds = std::stof(outputSaveBestSecondsEnv);
  _outputSaveCurrentSeconds = -1.0;
  if (const char *outputSaveCurrentSecondsEnv = std::getenv("JAFFAR2_SAVE_CURRENT_EVERY_SECONDS")) _outputSaveCurrentSeconds = std::stof(outputSaveCurrentSecondsEnv);

  // Parsing savegame files output path
  _outputSaveBestPath = "/tmp/jaffar.best.sav";
  if (const char *outputSaveBestPathEnv = std::getenv("JAFFAR2_SAVE_BEST_PATH")) _outputSaveBestPath = std::string(outputSaveBestPathEnv);
  _outputSaveCurrentPath = "/tmp/jaffar.current.sav";
  if (const char *outputSaveCurrentPathEnv = std::getenv("JAFFAR2_SAVE_CURRENT_PATH")) _outputSaveCurrentPath = std::string(outputSaveCurrentPathEnv);

  // Parsing solution files output path
  _outputSolutionBestPath = "/tmp/jaffar.best.sol";
  if (const char *outputSolutionBestPathEnv = std::getenv("JAFFAR2_SOLUTION_BEST_PATH")) _outputSolutionBestPath = std::string(outputSolutionBestPathEnv);
  _outputSolutionCurrentPath = "/tmp/jaffar.current.sol";
  if (const char *outputSolutionCurrentPathEnv = std::getenv("JAFFAR2_SOLUTION_CURRENT_PATH")) _outputSolutionCurrentPath = std::string(outputSolutionCurrentPathEnv);

  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar2a-train", "1.0");

  program.add_argument("--romFile")
    .help("Specifies the path to the genesis rom file (.bin) from which to start.")
    .required();

  program.add_argument("--savFile")
    .help("Specifies the path to the blastem savefile (.state) from which to start.")
    .default_value(std::string("quicksave.sav"))
    .required();

  program.add_argument("--maxSteps")
    .help("Specifies the maximum number of steps to run jaffar for.")
    .action([](const std::string& value) { return std::stoul(value); })
    .required();

  program.add_argument("--disableHistory")
    .help("Do not store the move history during training to save memory.")
    .default_value(false)
    .implicit_value(true);

  program.add_argument("jaffarFiles")
    .help("path to the Jaffar configuration script (.jaffar) file(s) to run.")
    .remaining()
    .required();

  // Try to parse arguments
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
    if (_workerId == 0) fprintf(stderr, "%s\n%s", err.what(), program.help().str().c_str());
    exit(-1);
  }

  // Establishing whether to store move history
  _storeMoveList = program.get<bool>("--disableHistory") == false;

  // Getting romfile path
  _romFilePath = program.get<std::string>("--romFile");

  // Getting savefile path
  _saveFilePath = program.get<std::string>("--savFile");

  // Loading save file contents
  std::string saveString;
  bool status = loadStringFromFile(saveString, _saveFilePath.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not load save state from file: %s\n", _saveFilePath.c_str());

  // Parsing max steps
  _maxSteps = program.get<size_t>("--maxSteps");

  // The move list contains two moves per byte
  _moveListStorageSize = _maxSteps;

  // Calculating DB sizes
  _maxLocalDatabaseSize = floor(((double)frameDBMaxMBytes * 1024.0 * 1024.0) / ((double)Frame::getSerializationSize()));
  _maxGlobalDatabaseSize = _maxLocalDatabaseSize * _workerCount;

  // Parsing config files
  _scriptFiles = program.get<std::vector<std::string>>("jaffarFiles");
  std::vector<nlohmann::json> scriptFilesJs;
  for (size_t i = 0; i < _scriptFiles.size(); i++)
  {
   std::string scriptString;
   status = loadStringFromFile(scriptString, _scriptFiles[i].c_str());
   if (status == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from config file: %s\n%s \n", _scriptFiles[i].c_str(), program.help().str().c_str());

   nlohmann::json scriptJs;
   try
   {
    scriptJs = nlohmann::json::parse(scriptString);
   }
   catch (const std::exception &err)
   {
     if (_workerId == 0) fprintf(stderr, "[Error] Parsing configuration file %s. Details:\n%s\n", _scriptFiles[i].c_str(), err.what());
     exit(-1);
   }

   // Checking whether it contains the rules field
   if (isDefined(scriptJs, "Rules") == false) EXIT_WITH_ERROR("[ERROR] Train configuration file '%s' missing 'Rules' key.\n", _scriptFiles[i].c_str());

   // Adding it to the collection
   scriptFilesJs.push_back(scriptJs);
  }

  // Creating Blastem Instance
  _blastem = new blastemInstance;
  _blastem->initialize(_romFilePath.c_str(), _saveFilePath.c_str(), true, true);

   // Adding rules
   for (size_t scriptId = 0; scriptId < scriptFilesJs.size(); scriptId++)
    for (size_t ruleId = 0; ruleId < scriptFilesJs[scriptId]["Rules"].size(); ruleId++)
     _rules.push_back(new Rule(scriptFilesJs[scriptId]["Rules"][ruleId], _blastem));

   // Setting global rule count
   _ruleCount = _rules.size();

   // Checking for repeated rule labels
   std::set<size_t> ruleLabelSet;
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
   {
    size_t label = _rules[ruleId]->_label;
    ruleLabelSet.insert(label);
    if (ruleLabelSet.size() < ruleId + 1)
     EXIT_WITH_ERROR("[ERROR] Rule label %lu is repeated in the configuration file.\n", label);
   }

   // Looking for rule dependency indexes that match their labels
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
    for (size_t depId = 0; depId < _rules[ruleId]->_dependenciesLabels.size(); depId++)
    {
     bool foundLabel = false;
     size_t label = _rules[ruleId]->_dependenciesLabels[depId];
     if (label == _rules[ruleId]->_label) EXIT_WITH_ERROR("[ERROR] Rule %lu references itself in dependencies vector.\n", label);
     for (size_t subRuleId = 0; subRuleId < _ruleCount; subRuleId++)
      if (_rules[subRuleId]->_label == label)
      {
       _rules[ruleId]->_dependenciesIndexes[depId] = subRuleId;
       foundLabel = true;
       break;
      }
     if (foundLabel == false) EXIT_WITH_ERROR("[ERROR] Could not find rule label %lu, specified as dependency by rule %lu.\n", label, _rules[ruleId]->_label);
    }

   // Looking for rule satisfied sub-rules indexes that match their labels
   for (size_t ruleId = 0; ruleId < _ruleCount; ruleId++)
    for (size_t satisfiedId = 0; satisfiedId < _rules[ruleId]->_satisfiesLabels.size(); satisfiedId++)
    {
     bool foundLabel = false;
     size_t label = _rules[ruleId]->_satisfiesLabels[satisfiedId];
     if (label == _rules[ruleId]->_label) EXIT_WITH_ERROR("[ERROR] Rule %lu references itself in satisfied vector.\n", label);

     for (size_t subRuleId = 0; subRuleId < _ruleCount; subRuleId++)
      if (_rules[subRuleId]->_label == label)
      {
       _rules[ruleId]->_satisfiesIndexes[satisfiedId] = subRuleId;
       foundLabel = true;
       break;
      }
     if (foundLabel == false) EXIT_WITH_ERROR("[ERROR] Could not find rule label %lu, specified as satisfied by rule %lu.\n", label, satisfiedId);
    }

  // Allocating database flushing mask
  _flushingMask.resize(_ruleCount, 0);

  printf("[Jaffar2] MPI Rank %lu/%lu: Blastem initialized.\n", _workerId, _workerCount);
  fflush(stdout);
  MPI_Barrier(MPI_COMM_WORLD);

  // Setting initial status for each rule
  std::vector<char> rulesStatus(_ruleCount);
  for (size_t i = 0; i < _ruleCount; i++) rulesStatus[i] = false;

  _frameSerializedSize = Frame::getSerializationSize();
  MPI_Type_contiguous(_frameSerializedSize, MPI_BYTE, &_mpiFrameType);
  MPI_Type_commit(&_mpiFrameType);

  // Setting initial values
  _hasFinalized = false;
  _globalHashCollisions = 0;
  _globalBestFrameScore = 0;

  // Setting win status
  _winFrameFound = false;
  _localWinFound = false;

  // Initializing frame counts per worker
  _localBaseFrameCounts.resize(_workerCount);

  // Creating initial frame on the root rank
  if (_workerId == 0)
  {
    // Computing initial hash
    const auto hash = _blastem->computeHash();

    auto initialFrame = std::make_unique<Frame>();
    _blastem->saveState(initialFrame->frameStateData);
    initialFrame->rulesStatus = rulesStatus;

    // Evaluating Rules on initial frame
    evaluateRules(*initialFrame);

    // Evaluating Score on initial frame
    initialFrame->reward = getFrameReward(*initialFrame);

    // Registering hash for initial frame
    _hashDatabase.insert({ hash, 0 });

    // Copying initial frame into the best frame
    _bestFrame = *initialFrame;

    // Filling show database
    _showFrameDB.resize(SHOW_FRAME_COUNT);
    for (size_t i = 0; i < SHOW_FRAME_COUNT; i++) _showFrameDB[i] = *initialFrame;

    // Adding frame to the current database
    _currentFrameDB.push_back(std::move(initialFrame));

    // Initializing show thread
    if (pthread_create(&_showThreadId, NULL, showThreadFunction, this) != 0)
      EXIT_WITH_ERROR("[ERROR] Could not create show thread.\n");
  }

  // Starting global frame counter
  _globalFrameCounter = 1;
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
  auto currentFrameSaveTimer = std::chrono::steady_clock::now();

  // Counter for the current best frame id
  size_t currentFrameId = 0;

  while (_hasFinalized == false)
  {
    // Sleeping for one second to prevent excessive overheads
    sleep(1);

    // Checking if we need to save best frame
    if (_outputSaveBestSeconds > 0.0)
    {
      double bestFrameTimerElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - bestFrameSaveTimer).count();
      if (bestFrameTimerElapsed / 1.0e+9 > _outputSaveBestSeconds)
      {
        // Saving best frame data
        saveBinToFile(_bestFrame.frameStateData, _STATE_DATA_SIZE,_outputSaveBestPath.c_str());

        // Storing the solution sequence
        if (_storeMoveList)
        {
         std::string solutionString;
         solutionString += _possibleMoves[_bestFrame.getMove(0)];
         for (size_t i = 1; i <= _currentStep; i++)
          solutionString += std::string(" ") + _possibleMoves[_bestFrame.getMove(i)];
         saveStringToFile(solutionString, _outputSolutionBestPath.c_str());
        }

        // Resetting timer
        bestFrameSaveTimer = std::chrono::steady_clock::now();
      }
    }

    // Checking if we need to save current frame
    if (_outputSaveCurrentSeconds > 0.0)
    {
      double currentFrameTimerElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - currentFrameSaveTimer).count();
      if (currentFrameTimerElapsed / 1.0e+9 > _outputSaveCurrentSeconds)
      {
        // Saving best frame data
        saveBinToFile(_showFrameDB[currentFrameId].frameStateData, _STATE_DATA_SIZE, _outputSaveCurrentPath.c_str());

        // Storing the solution sequence
        if (_storeMoveList)
        {
         std::string solutionString;
         solutionString += _possibleMoves[_showFrameDB[currentFrameId].getMove(0)];
         for (size_t i = 1; i <= _currentStep; i++)
          solutionString += std::string(" ") + _possibleMoves[_showFrameDB[currentFrameId].getMove(i)];
         saveStringToFile(solutionString, _outputSolutionCurrentPath.c_str());
        }

        // Resetting timer
        currentFrameSaveTimer = std::chrono::steady_clock::now();

        // Increasing counter
        currentFrameId = (currentFrameId + 1) % SHOW_FRAME_COUNT;
      }
    }
  }

  // If it has finalized with a win, save the winning frame
  if (_outputSaveBestSeconds > 0.0)
  {
   auto lastFrame = _winFrameFound ? _globalWinFrame : _bestFrame;

   saveBinToFile(lastFrame.frameStateData, _STATE_DATA_SIZE, _outputSaveBestPath.c_str());

   // Storing the solution sequence
   if (_storeMoveList)
   {
    std::string solutionString;
    solutionString += _possibleMoves[lastFrame.getMove(0)];
    for (size_t i = 1; i <= _currentStep; i++)
     solutionString += std::string(" ") + _possibleMoves[lastFrame.getMove(i)];
    saveStringToFile(solutionString, _outputSolutionBestPath.c_str());
   }
  }
}

int main(int argc, char *argv[])
{
  Train train(argc, argv);

  // Running Search
  train.run();

  return MPI_Finalize();
}
