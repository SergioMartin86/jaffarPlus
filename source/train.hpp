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
#include <queue>
#include <mutex>

// Configuration for parallel hash maps
#define MAPNAME phmap::parallel_flat_hash_map
#define MAPEXTRAARGS , phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 4, std::mutex
template <class K, class V> using HashMapT = MAPNAME<K, V MAPEXTRAARGS>;
using hashMap_t = HashMapT<uint64_t, uint16_t>;

class Train
{
  public:
  Train(int argc, char *argv[]);
  void run();

  private:

  // Storage for current step
  uint16_t _currentStep;

  // File output config
  double _outputSaveBestSeconds;
  std::string _outputSaveBestPath;
  std::string _outputSolutionBestPath;
  bool _showSDLPopPreview;

  // Store the number of openMP threads in use
  int _threadCount;

  // Creating game instances, one per openMP thread
  std::vector<GameInstance*> _gameInstances;
  size_t _ruleCount;

  // State counter
  size_t _stepBaseStatesProcessedCounter;
  size_t _stepNewStatesProcessedCounter;
  size_t _totalStatesProcessedCounter;
  size_t _stepMaxStatesInMemory;
  size_t _totalMaxStatesInMemory;
  float _stepNewStateRatio;
  float _maxNewStateRatio;
  uint16_t _maxNewStateRatioStep;

  // Storage for source state data for differential load/save
  uint8_t _referenceStateData[_STATE_DATA_SIZE];

  // State databases
  size_t _databaseSize;
  size_t _maxDatabaseSizeLowerBound;
  size_t _maxDatabaseSizeUpperBound;
  std::vector<State*> _stateDB;
  std::queue<State*> _freeStatesQueue;

  // Storage for the win, best and worst state
  State _bestState;
  float _bestStateReward;
  State _worstState;
  float _worstStateReward;

  // Hash information
  hashMap_t _hashDB;
  size_t _hashEntriesTotal;
  ssize_t _hashEntriesStep;
  double _hashSizeStep;
  double _hashSizeCurrent;
  double _hashSizeLowerBound;
  double _hashSizeUpperBound;
  std::vector<size_t> _hashStepNewEntries;
  uint16_t _hashStepThreshold;
  size_t _hashCollisions;
  size_t _hashAgeThreshold;

  // Per-step local hash collision counter
  size_t _newCollisionCounter;

  // Storage for the position of win rules, for win detection
  bool _winStateFound;
  State _winState;

  // SDLPop instance and Id for the show thread
  pthread_t _showThreadId;

  // Flag to indicate finalization
  bool _hasFinalized;

  // Printing stats
  void printTrainStatus();

  // Each worker processes their own unique base states to produce new states
  void computeStates();

  // Argument parser
  void parseArgs(int argc, char *argv[]);

  // Function to limit the size of a state database and recycle its states
  void limitStateDatabase(std::vector<State*>& stateDB, size_t limit);

  // Function for the show thread (saves states from time to time to display progress)
  static void *showThreadFunction(void *trainPtr);
  void showSavingLoop();

  // Profiling and Debugging
  double _searchTotalTime;
  double _currentStepTime;
  double _stepHashCalculationTime;
  double _stepHashCheckingTime;
  double _stepHashFilteringTime;
  double _stepStateAdvanceTime;
  double _stepStateDeserializationTime;
  double _stepStateEncodingTime;
  double _stepStateDecodingTime;
  double _stepStateDBSortingTime;
  double _stepStateCreationTime;
};
