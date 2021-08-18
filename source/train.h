#pragma once

#include "blastemInstance.h"
#include "frame.h"
#include "nlohmann/json.hpp"
#include "rule.h"
#include <absl/container/flat_hash_set.h>
#include <absl/container/flat_hash_map.h>
#include <algorithm>
#include <chrono>
#include <memory>
#include <mpi.h>
#include <pthread.h>
#include <random>
#include <string>
#include <vector>

// Number of frames to cache for showing purposes
#define SHOW_FRAME_COUNT 1000

// Hash database age cutoff frequency
#define HASHDB_AGE_CUTOFF_FREQ 10

// Struct to hold all of the frame's magnet information
struct magnetInfo_t
{
 float positionX;
 float intensityX;
 float intensityY;
};

class Train
{
  public:
  Train(int argc, char *argv[]);
  void run();

  private:

  // Jaffar script files
  std::vector<std::string> _scriptFiles;

  // File output config
  double _outputSaveBestSeconds;
  double _outputSaveCurrentSeconds;
  std::string _outputSaveBestPath;
  std::string _outputSaveCurrentPath;
  std::string _outputSolutionBestPath;
  std::string _outputSolutionCurrentPath;
  bool _showSDLPopPreview;

  // Worker id and count
  size_t _workerId;
  size_t _workerCount;
  std::string _romFilePath;
  std::string _saveFilePath;

  // Communication schedule for frame exchange
  std::vector<size_t> _communicationSchedule;

  // Craeting SDLPop and State class instances and rule vector
  blastemInstance* _blastem;
  std::vector<Rule *> _rules;

  // Frame counter
  size_t _currentStep;
  size_t _globalFrameCounter;
  size_t _stepFramesProcessedCounter;
  size_t _totalFramesProcessedCounter;

  // Frame counters per worker
  std::vector<size_t> _localBaseFrameCounts;
  size_t _maxFrameCount;
  size_t _maxFrameWorkerId;
  size_t _minFrameCount;
  size_t _minFrameWorkerId;

  // Frame databases
  size_t _maxGlobalDatabaseSize;
  size_t _maxLocalDatabaseSize;
  std::vector<std::unique_ptr<Frame>> _currentFrameDB;
  std::vector<std::unique_ptr<Frame>> _nextFrameDB;

  // Frame database for showing
  std::vector<Frame> _showFrameDB;

  // Storage for the best frame
  Frame _bestFrame;
  float _globalBestFrameScore;

  // Hash information
  absl::flat_hash_map<uint64_t, uint16_t> _hashDatabase;
  absl::flat_hash_set<uint64_t> _newHashes;
  size_t _hashAgeThreshold;
  size_t _globalHashCollisions;

  // Per-step local hash collision counter
  size_t _newCollisionCounter;

  // Per-step frame processed counter
  size_t _localStepFramesProcessedCounter;

  // Storage for the position of win rules, for win detection
  bool _winFrameFound;
  Frame _globalWinFrame;
  Frame _localWinFrame;
  bool _localWinFound;

  // Storage for rule serialization size
  size_t _frameSerializedSize;
  MPI_Datatype _mpiFrameType;

  // SDLPop instance and Id for the show thread
  pthread_t _showThreadId;

  // Database flushing rule mask
  std::vector<uint8_t> _flushingMask;

  // Flag to indicate finalization
  bool _hasFinalized;

  // Printing stats
  void printTrainStatus();

  // Each worker processes their own unique base frames to produce new frames
  void computeFrames();

  // Workers sorts their databases and communicates partial results
  void framePostprocessing();

  // Obtains the score of a given frame
  float getFrameReward(const Frame &frame);

  // Evaluates the rule set on a given frame. Returns true if it is a fail.
  void evaluateRules(Frame &frame);

  // Marks the given rule as satisfied, executes its actions, and recursively runs on its sub-satisfied rules
  void satisfyRule(Frame &frame, const size_t ruleId);

  // Print Rule information
  void printRuleStatus(const Frame &frame);

  // Redistribute frames uniformly among workers
  void distributeFrames();

  // Sharing hash entries among workers and cut hash tables databases to size
  void updateHashDatabases();

  // Adds a new hash entry while making sure the number of hash entries don't exceed the maximum
  void addHashEntry(uint64_t hash);

  // Argument parser
  void parseArgs(int argc, char *argv[]);

  // Function for the show thread (saves states from time to time to display progress)
  static void *showThreadFunction(void *trainPtr);
  void showSavingLoop();

  // Functions that check special flags for a given frame
  void checkSpecialActions(const Frame &frame);
  bool checkFail(const Frame &frame);
  bool checkWin(const Frame &frame);
  bool checkFlush(const Frame &frame);
  void addFlushMask(const Frame &frame);

  // Function to get the static rewards obtained from rules
  float getRuleRewards(const Frame &frame);

  // Function to get magnet information
  magnetInfo_t getKidMagnetValues(const Frame &frame, const int room);
  magnetInfo_t getGuardMagnetValues(const Frame &frame, const int room);

  // Profiling and Debugging
  double _searchTotalTime;
  double _currentStepTime;
  double _frameDistributionTime;
  double _frameComputationTime;
  double _framePostprocessingTime;
  double _hashPostprocessingTime;
};
