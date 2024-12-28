#pragma once

#include <cstdlib>
#include <memory>
#include <numa.h>
#include <utmpx.h>
#include <cstdlib>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/parallel.hpp>
#include "base.hpp"

namespace jaffarPlus
{

namespace stateDb
{

/**
 * Thread local storage of preferred NUMA domain
 */
thread_local static int _preferredNumaDomain;

/**
 * Thread local storage of my cpu id
 */
thread_local static int _myThreadId;

class Numa : public stateDb::Base
{
  public:

  Numa(Runner &r, const nlohmann::json &config)
    : stateDb::Base(r, config)
  {
    // Checking whether the numa library calls are available
    const auto numaAvailable = numa_available();
    if (numaAvailable != 0) JAFFAR_THROW_RUNTIME("NUMA State Db selected, but the system does not provide NUMA support.");

    // Getting number of noma domains
    _numaCount = numa_max_node() + 1;

    // Checking maximum db sizes per each numa
    _maxSizePerNumaMb = jaffarCommon::json::getArray<size_t>(config, "Max Size per NUMA Domain (Mb)");
    if (_maxSizePerNumaMb.size() < (size_t)_numaCount)
      JAFFAR_THROW_LOGIC("System has %d NUMA domains but only sizes for %lu of them provided.", _numaCount, _maxSizePerNumaMb.size());
  }

  ~Numa() = default;

  void initializeImpl() override
  {
    // Setting counters for database state popping
    _numaNonLocalDatabaseStateCount = 0;
    _numaLocalDatabaseStateCount    = 0;
    _numaDatabaseStateNotFoundCount = 0;

    // Setting counters for free state getting
    _numaNonLocalFreeStateCount = 0;
    _numaLocalFreeStateCount    = 0;
    _numaFreeStateNotFoundCount = 0;
    _numaStealingFreeStateCount = 0;

    // Getting maximum state db size in Mb and bytes
    for (int i = 0; i < _numaCount; i++)
      {
        double sizeMb = _maxSizePerNumaMb[i];
        // For testing purposes, the maximum size can be overriden via environment variables
        if (auto *value = std::getenv("JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB")) sizeMb = std::stoul(value);
        _maxSizePerNuma.push_back(std::ceil((double)sizeMb * 1024.0 * 1024.0));
      }

    // Getting maximum allocatable memory in each NUMA domain
    std::vector<size_t> maxFreeMemoryPerNuma(_numaCount);
    for (int i = 0; i < _numaCount; i++)
      {
        size_t freeMemory = 0;
        numa_node_size64(i, (long long *)&freeMemory);
        maxFreeMemoryPerNuma[i] = freeMemory;
      }

    // Checking if this is enough memory to satisfy requirement
    for (int i = 0; i < _numaCount; i++)
      if (_maxSizePerNuma[i] > (size_t)maxFreeMemoryPerNuma[i])
        JAFFAR_THROW_RUNTIME("The requested memory (%lu) for NUMA domain %d exceeds its available free space (%lu)\n", _maxSizePerNuma[i], i, maxFreeMemoryPerNuma[i]);

    // Getting maximum number of states for each NUMA domain
    _maxStatesPerNuma.resize(_numaCount);
    _currentStatesPerNuma.resize(_numaCount);
    for (int i = 0; i < _numaCount; i++) _maxStatesPerNuma[i] = std::max(_maxSizePerNuma[i] / _stateSize, 1ul);

    // Getting totals for statistics
    _maxSize   = 0;
    _maxStates = 0;
    for (int i = 0; i < _numaCount; i++)
      {
        _maxSize += _maxSizePerNuma[i];
        _maxStates += _maxStatesPerNuma[i];
        _numaCurrentStateQueues.push_back(new jaffarCommon::concurrent::Deque<void *>());
        _numaNextStateQueues.push_back(new jaffarCommon::concurrent::concurrentMultimap_t<float, void *>());
      }

    // Getting number of bytes to reserve for each NUMA domain
    _allocableBytesPerNuma.resize(_numaCount);
    for (int i = 0; i < _numaCount; i++) _allocableBytesPerNuma[i] = _maxStatesPerNuma[i] * _stateSize;

    // Creating internal buffers, one per NUMA domain
    _internalBuffersStart.resize(_numaCount);
    _internalBuffersEnd.resize(_numaCount);
    for (int i = 0; i < _numaCount; i++)
      {
        _internalBuffersStart[i] = (uint8_t *)numa_alloc_onnode(_allocableBytesPerNuma[i], i);
        if (_internalBuffersStart[i] == NULL) JAFFAR_THROW_RUNTIME("Error trying to allocate memory for numa domain %d\n", i);
        _internalBuffersEnd[i] = &_internalBuffersStart[i][_allocableBytesPerNuma[i]];
      }

    // Initializing numa delegate thread storage
    _numaDelegateThreadId.resize(_numaCount);
    for (int i = 0; i < _numaCount; i++) _numaDelegateThreadId[i] = std::numeric_limits<int>::max();

    // Determining the preferred numa domain for each thread. This depends on OpenMP using always the same set of threads.
    JAFFAR_PARALLEL
    {
      _myThreadId          = sched_getcpu();
      int node             = numa_node_of_cpu(_myThreadId);
      _preferredNumaDomain = node;

      // Setting myself as numa delegate, if I'm the lowest cpu id in the numa domain
      _workMutex.lock();
      if (_myThreadId < _numaDelegateThreadId[node]) _numaDelegateThreadId[node] = _myThreadId;
      _workMutex.unlock();
    }

    // Getting system's page size (typically 4K but it may change in the future)
    const size_t pageSize = sysconf(_SC_PAGESIZE);

    // Initializing the internal buffers
    for (int numaNodeIdx = 0; numaNodeIdx < _numaCount; numaNodeIdx++) JAFFAR_PARALLEL_FOR
    for (size_t i = 0; i < _allocableBytesPerNuma[numaNodeIdx]; i += pageSize) _internalBuffersStart[numaNodeIdx][i] = 1;

    // Adding the state pointers to the free state queues
    _freeStateQueues.resize(_numaCount);
    for (int numaNodeIdx = 0; numaNodeIdx < _numaCount; numaNodeIdx++)
      {
        _freeStateQueues[numaNodeIdx] = std::make_unique<jaffarCommon::concurrent::atomicQueue_t<void *>>(_maxStatesPerNuma[numaNodeIdx]);
        for (size_t i = 0; i < _maxStatesPerNuma[numaNodeIdx]; i++) _freeStateQueues[numaNodeIdx]->try_push((void *)&_internalBuffersStart[numaNodeIdx][i * _stateSize]);
      }
  }

  // Function to print relevant information
  void printInfoImpl() const override
  {
    for (int i = 0; i < _numaCount; i++)
      jaffarCommon::logger::log("[J+]  + NUMA Domain %d                  States: %lu (Max: %lu), Size: %.3f Mb (%.6f Gb)\n",
                                i,
                                _currentStatesPerNuma[i],
                                _maxStatesPerNuma[i],
                                (double)_maxSizePerNuma[i] / (1024.0 * 1024.0),
                                (double)_maxSizePerNuma[i] / (1024.0 * 1024.0 * 1024.0));

    size_t totalDatabaseStatesRequested = _numaNonLocalDatabaseStateCount + _numaLocalDatabaseStateCount + _numaDatabaseStateNotFoundCount;
    jaffarCommon::logger::log("[J+] + Database Popping State Rates:\n");
    jaffarCommon::logger::log("[J+]  + Numa Locality Success Rate:                     %5.3f%%\n",
                              100.0 * (double)_numaLocalDatabaseStateCount.load() / (double)totalDatabaseStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa Locality Fail Rate:                        %5.3f%%\n",
                              100.0 * (double)_numaNonLocalDatabaseStateCount.load() / (double)totalDatabaseStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa No DB State Found Rate:                    %5.3f%%\n",
                              100.0 * (double)_numaDatabaseStateNotFoundCount.load() / (double)totalDatabaseStatesRequested);

    size_t totalFreeStatesRequested = _numaNonLocalFreeStateCount + _numaLocalFreeStateCount + _numaFreeStateNotFoundCount + _numaStealingFreeStateCount;
    jaffarCommon::logger::log("[J+] + Get Free State Rates:\n");
    jaffarCommon::logger::log("[J+]  + Numa Locality Success Rate:                     %5.3f%%\n",
                              100.0 * (double)_numaLocalFreeStateCount.load() / (double)totalFreeStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa Locality Fail Rate:                        %5.3f%%\n",
                              100.0 * (double)_numaNonLocalFreeStateCount.load() / (double)totalFreeStatesRequested);
    jaffarCommon::logger::log("[J+]  + State DB Stealing Rate:                         %5.3f%%\n",
                              100.0 * (double)_numaStealingFreeStateCount.load() / (double)totalFreeStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa No Free State Found Rate:                  %5.3f%%\n",
                              100.0 * (double)_numaFreeStateNotFoundCount.load() / (double)totalFreeStatesRequested);
  }

  __INLINE__ void *getFreeState() override
  {
    // Storage for the new free state space
    void *stateSpace;

    // Trying to get free space for a new state
    bool success = _freeStateQueues[_preferredNumaDomain]->try_pop(stateSpace);

    // If successful, return the pointer immediately
    if (success == true)
      {
        _numaLocalFreeStateCount++;
        return stateSpace;
    }

    // Trying all other free state queues now
    for (int i = 0; (size_t)i < _freeStateQueues.size(); i++)
      if (i != _preferredNumaDomain)
        {
          // Trying to get free space for a new state
          bool success = _freeStateQueues[i]->try_pop(stateSpace);

          // If successful, return the pointer immediately
          if (success == true)
            {
              _numaNonLocalFreeStateCount++;
              return stateSpace;
          }
      }

    // If failed, then try to get it from the back of my numa-specific queues. Looking for the worst state possible
    success = _numaCurrentStateQueues[_preferredNumaDomain]->pop_back_get(stateSpace);

    // If failed, then try other numa domains
    for (int i = 0; i < _numaCount && success == false; i++)
      if (i != _preferredNumaDomain) success = _numaCurrentStateQueues[i]->pop_back_get(stateSpace);

    // If successful, return the pointer immediately
    if (success == true)
      {
        _numaStealingFreeStateCount++;
        return stateSpace;
    }

    // Otherwise, return a null pointer. The state will be discarded
    _numaFreeStateNotFoundCount++;
    return nullptr;
  }

  __INLINE__ int getStateNumaDomain(void *const statePtr)
  {
    for (int i = 0; i < _numaCount; i++)
      if (isStateInNumaDomain(statePtr, i)) return i;

    // Check for error
    JAFFAR_THROW_RUNTIME("Did not find the corresponding numa domain for the provided state pointer. This must be a bug in Jaffar\n");
  }

  __INLINE__ bool isStateInNumaDomain(void *const statePtr, const int numaDomainId)
  {
    return statePtr >= _internalBuffersStart[numaDomainId] && statePtr <= _internalBuffersEnd[numaDomainId];
  }

  __INLINE__ void returnFreeState(void *const statePtr) override
  {
    // Finding out to which database this state pointer belongs to
    const auto numaIdx = getStateNumaDomain(statePtr);

    // Pushing the state in the corresponding queue
    bool success = _freeStateQueues[numaIdx]->try_push(statePtr);

    // Check for success
    if (success == false) JAFFAR_THROW_RUNTIME("Failed on pushing free state back. This must be a bug in Jaffar\n");
  }

  __INLINE__ void advanceStep() override
  {
    // Calculation of best and worst states
    float bestStateReward  = std::numeric_limits<float>::lowest();
    float worstStateReward = std::numeric_limits<float>::max();

    // Copying state pointers into the numa-specific queues
    JAFFAR_PARALLEL
    {
      // Only process if I am the delegate
      if (_myThreadId == _numaDelegateThreadId[_preferredNumaDomain])
        {
          // Updating state count per numa
          _currentStatesPerNuma[_preferredNumaDomain] = _numaNextStateQueues[_preferredNumaDomain]->size();

          // Logic for best/worst state
          float stateReward = 0.0f;
          void *statePtr    = nullptr;

          // Establishing best state (the first of this numa queue is a candidate)
          if (_numaNextStateQueues[_preferredNumaDomain]->begin() != _numaNextStateQueues[_preferredNumaDomain]->end())
            {
              const auto &stateItr = _numaNextStateQueues[_preferredNumaDomain]->begin();
              stateReward          = stateItr->first;
              statePtr             = stateItr->second;

              _workMutex.lock();
              if (stateReward > bestStateReward)
                {
                  bestStateReward = stateReward;
                  _bestState      = statePtr;
              }
              _workMutex.unlock();
          }

          // Now dumping states from next state queue to the current one
          while (_numaNextStateQueues[_preferredNumaDomain]->begin() != _numaNextStateQueues[_preferredNumaDomain]->end())
            {
              const auto &stateItr = _numaNextStateQueues[_preferredNumaDomain]->begin();
              stateReward          = stateItr->first;
              statePtr             = stateItr->second;

              // Extracting next state
              _numaNextStateQueues[_preferredNumaDomain]->unsafe_extract(stateItr);

              // Pushing state in the correct numa domain queue
              _numaCurrentStateQueues[_preferredNumaDomain]->push_back_no_lock(statePtr);
            }

          // Now establishing worst state (the last of this numa queue is a candidate)
          if (statePtr != nullptr)
            {
              _workMutex.lock();
              if (stateReward < worstStateReward)
                {
                  worstStateReward = stateReward;
                  _worstState      = statePtr;
              }
              _workMutex.unlock();
          }
      }
    }
  }

  __INLINE__ bool pushStateImpl(const float reward, void *statePtr) override
  {
    // Getting the corresponding numa domain for this state
    int targetNumaIdx = getStateNumaDomain(statePtr);

    // Inserting new state into the next state database
    _numaNextStateQueues[targetNumaIdx]->insert({reward, statePtr});

    // If succeeded, return true
    return true;
  }

  __INLINE__ void *popState() override
  {
    // Pointer to return
    void *statePtr;

    // Check the numa's state queue first
    bool success = _numaCurrentStateQueues[_preferredNumaDomain]->pop_front_get(statePtr);
    if (success == true)
      {
        _numaLocalFreeStateCount++;
        return statePtr;
    }

    // If still no success, check the other numa queues
    for (int i = 0; i < _numaCount; i++)
      if (i != _preferredNumaDomain)
        {
          bool success = _numaCurrentStateQueues[i]->pop_front_get(statePtr);
          if (success == true)
            {
              _numaNonLocalDatabaseStateCount++;
              return statePtr;
          }
      }

    // If no success at all, just return a nullptr
    _numaDatabaseStateNotFoundCount++;
    return nullptr;
  }

  /**
   * Gets the current number of states in the current state database
   */
  __INLINE__ size_t getStateCount() const override
  {
    size_t stateCount = 0;
    for (int i = 0; i < _numaCount; i++) stateCount += _numaCurrentStateQueues[i]->wasSize();
    return stateCount;
  }

  /**
   * This function returns a pointer to the best state found in the current state database
   */
  virtual void *getBestState() const override { return _bestState; }

  /**
   * This function returns a pointer to the worst state found in the current state database
   */
  virtual void *getWorstState() const override { return _worstState; }

  private:

  /**
   * Number of numa domains
   */
  int _numaCount;

  void *_bestState  = nullptr;
  void *_worstState = nullptr;

  /**
   * Count of local database states retrieved
  */
  std::atomic<size_t> _numaLocalDatabaseStateCount;

  /**
   * Count of non-local database states retrieved
  */
  std::atomic<size_t> _numaNonLocalDatabaseStateCount;

  /**
   * Count of non-local database states retrieved
  */
  std::atomic<size_t> _numaDatabaseStateNotFoundCount;

  /**
   * Count of local free states retrieved
  */
  std::atomic<size_t> _numaLocalFreeStateCount;

  /**
   * Count of non-local free states retrieved
  */
  std::atomic<size_t> _numaNonLocalFreeStateCount;

  /**
   * Count of free states stolen from the back of the state databse
  */
  std::atomic<size_t> _numaStealingFreeStateCount;

  /**
   * Count of free states failed to be retrieved
  */
  std::atomic<size_t> _numaFreeStateNotFoundCount;

  /**
   * User-provided maximum megabytes to use for the state database per numa domain
   */
  std::vector<size_t> _maxSizePerNumaMb;

  /**
   * User-provided maximum bytes to use for the state database per numa domain
   */
  std::vector<size_t> _maxSizePerNuma;

  /**
   * Calculated maximum states to use for the state database per numa domain
   */
  std::vector<size_t> _maxStatesPerNuma;

  /**
   * Current states in each numa domain
   */
  std::vector<size_t> _currentStatesPerNuma;

  /**
   * Delegate thread per numa domain
   */
  std::vector<int> _numaDelegateThreadId;

  /**
   * Mutex to set delegate thread id and finding best/worst states
   */
  std::mutex _workMutex;

  /**
   * Numa state queues allow the thread to search for a current state that belongs to it through the current state database
   */
  std::vector<jaffarCommon::concurrent::Deque<void *> *> _numaCurrentStateQueues;

  /**
   * Numa state queues for the next step's states
   */
  std::vector<jaffarCommon::concurrent::concurrentMultimap_t<float, void *> *> _numaNextStateQueues;

  /**
   * This queue will hold pointers to all the free state storage
   */
  std::vector<std::unique_ptr<jaffarCommon::concurrent::atomicQueue_t<void *>>> _freeStateQueues;

  /**
   * Start pointer for the internal buffers for the state database
   */
  std::vector<uint8_t *> _internalBuffersStart;

  /**
   * End pointer for each of the internal buffers
   */
  std::vector<uint8_t *> _internalBuffersEnd;

  /**
   * Number of bytes to allocate per NUMA domain
   */
  std::vector<size_t> _allocableBytesPerNuma;
};

} // namespace stateDb

} // namespace jaffarPlus
