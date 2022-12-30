#pragma once

#include "state.hpp"
#include "rule.hpp"
#include "gameInstance.hpp"
#include "nlohmann/json.hpp"
#include <parallel_hashmap/phmap.h>
#include <algorithm>
#include <chrono>
#include <pthread.h>
#include <random>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>

// Configuration for parallel hash sets
#define SETNAME phmap::parallel_flat_hash_set
#define SETEXTRAARGS , phmap::priv::hash_default_hash<V>, phmap::priv::hash_default_eq<V>, std::allocator<V>, 4, std::mutex
template <class V> using HashSetT = SETNAME<V SETEXTRAARGS>;
using hashSet_t = HashSetT<_uint128_t>;

class Train
{
  public:
  Train(const nlohmann::json& config);
  ~Train() = default;
  void run();
  void reset();

  // Configuration
  nlohmann::json _config;
  std::string _configFile;

  // Storage for current step
  uint16_t _currentStep;

  // Saving intermediate result settings
  bool _outputEnabled;
  double _outputSaveFrequency;
  std::string _outputSolutionBestPath;
  std::string _outputSolutionWorstPath;
  std::string _outputStateBestPath;
  std::string _outputStateWorstPath;

  // Store the number of openMP threads in use
  int _threadCount;

  // Creating game instances, one per openMP thread
  std::vector<GameInstance*> _gameInstances;
  std::vector<std::vector<GameInstance*>> _verificationInstances;
  GameInstance* _showGameInstance;

  // State counter
  size_t _stepBaseStatesProcessedCounter;
  size_t _stepNewStatesProcessedCounter;
  size_t _totalStatesProcessedCounter;
  size_t _stepMaxStatesInMemory;
  size_t _totalMaxStatesInMemory;
  float _stepNewStateRatio;
  float _maxNewStateRatio;
  uint16_t _maxNewStateRatioStep;
  size_t _invalidStateCount;

  // Storage for source state data for differential load/save
  uint8_t _referenceStateData[_STATE_DATA_SIZE_TRAIN];
  uint8_t _initialStateData[_STATE_DATA_SIZE_TRAIN];

  // State databases
  size_t _databaseSize;
  size_t _maxDBSizeMbLowerBound;
  size_t _maxDBSizeMbUpperBound;
  size_t _maxDatabaseSizeLowerBound;
  size_t _maxDatabaseSizeUpperBound;
  std::vector<State*> _stateDB;

  // Queue for free states
  uint8_t* _mainStateStorage;
  std::queue<State*> _freeStateQueue;
  Mutex _freeStateQueueMutex;
  State *_firstState;

  // Storage for the win, best and worst state
  Mutex _bestStateMutex;
  State* _bestState;
  float _bestStateReward;
  State* _worstState;
  float _worstStateReward;

  // Hash information
  ssize_t _hashDBCount;
  uint16_t _hashCurAge;
  std::unique_ptr<hashSet_t> _hashCurDB;
  std::deque<std::unique_ptr<hashSet_t>> _hashPastDBs;
  std::deque<uint16_t> _hashDBAges;

  size_t _hashEntriesTotal;
  ssize_t _hashEntriesStep;
  double _hashSizeStep;
  double _hashSizeCurrent;
  double _hashSizeUpperBound;
  std::vector<size_t> _hashStepNewEntries;
  size_t _hashCollisions;
  size_t _hashAgeThreshold;

  // Per-step local hash collision counter
  size_t _newCollisionCounter;

  // Win state management
  uint16_t _winStateStepTolerance;
  std::map<uint16_t, std::vector<State*>> _winStateHistory;
  std::map<uint16_t, float> _winStateHistoryStepBestRewards;
  float _winStateHistoryBestReward;
  size_t _totalWinStatesFound;
  State* _bestWinState;
  uint16_t _bestWinStateStep;
  uint8_t _bestWinStateData[_STATE_DATA_SIZE_TRAIN];

  // SDLPop instance and Id for the show thread
  pthread_t _showThreadId;

  // Stats flags
  bool _showHashInfo;
  bool _showTimingInfo;

  // Flag to indicate finalization
  bool _hasFinalized;

  // Printing stats
  void printTrainStatus();

  // Flag for printing winning information
  bool _showWinStateInfo;

  // Computes total hash count
  size_t hashGetTotalCount() const;

  // Each worker processes their own unique base states to produce new states
  void computeStates();

  // Argument parser
  void parseArgs(int argc, char *argv[]);

  // Function to limit the size of a state database and recycle its states
  void limitStateDatabase(std::vector<State*>& stateDB, size_t limit);

  // Calculates hash db size from its entries
  double hashSizeFromEntries (const ssize_t entries) const;
  size_t hashEntriesFromSize(const double size) const;

  // Function for the show thread (saves states from time to time to display progress)
  static void *showThreadFunction(void *trainPtr);
  void showSavingLoop();

  // Profiling and Debugging
  double _searchTotalTime;
  double _currentStepTime;
  ssize_t _stepHashCalculationTime;
  ssize_t _stepHashCheckingTime;
  ssize_t _stepHashFilteringTime;
  ssize_t _stepStateAdvanceTime;
  ssize_t _stepStateDeserializationTime;
  ssize_t _stepStateEncodingTime;
  ssize_t _stepStateDecodingTime;
  ssize_t _stepStateDBSortingTime;
  ssize_t _stepStateCreationTime;
  ssize_t _stepStateEvaluationTime;
};
