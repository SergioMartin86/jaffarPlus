#pragma once

#include "quickNESInstance.h"
#include "frame.h"
#include "nlohmann/json.hpp"
#include "rule.h"
#include "state.h"
#include <parallel_hashmap/phmap.h>
#include <algorithm>
#include <chrono>
#include <memory>
#include <pthread.h>
#include <random>
#include <string>
#include <vector>
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

  // Jaffar script file
  std::string _scriptFile;

  // File output config
  double _outputSaveBestSeconds;
  std::string _outputSaveBestPath;
  std::string _outputSolutionBestPath;
  bool _showSDLPopPreview;

  // Store the number of openMP threads in use
  int _threadCount;

  // Craeting State class instance, one per openMP thread
  std::vector<State *> _state;
  State* _showState;
  size_t _ruleCount;

  // Storage for source frame data for differential load/save
  uint8_t _sourceFrameData[_FRAME_DATA_SIZE];

  // Frame counter
  size_t _stepFramesProcessedCounter;
  size_t _totalFramesProcessedCounter;

  // Frame databases
  size_t _databaseSize;
  size_t _maxDatabaseSize;
  std::vector<std::unique_ptr<Frame>> _frameDB;

  // Frame database for showing
  std::vector<Frame> _showFrameDB;

  // Storage for the win, best and worst frame
  Frame _bestFrame;
  float _bestFrameReward;
  Frame _worstFrame;
  float _worstFrameReward;

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
  bool _winFrameFound;
  Frame _winFrame;

  // SDLPop instance and Id for the show thread
  pthread_t _showThreadId;

  // Flag to indicate finalization
  bool _hasFinalized;

  // Printing stats
  void printTrainStatus();

  // Each worker processes their own unique base frames to produce new frames
  void computeFrames();

  // Argument parser
  void parseArgs(int argc, char *argv[]);

  // Function for the show thread (saves states from time to time to display progress)
  static void *showThreadFunction(void *trainPtr);
  void showSavingLoop();

  // Profiling and Debugging
  double _searchTotalTime;
  double _currentStepTime;
  double _stepHashCalculationTime;
  double _stepHashCheckingTime1;
  double _stepHashCheckingTime2;
  double _stepHashFilteringTime;
  double _stepFrameAdvanceTime;
  double _stepFrameDeserializationTime;
  double _stepFrameEncodingTime;
  double _stepFrameDecodingTime;
  double _stepFrameDBSortingTime;
};
