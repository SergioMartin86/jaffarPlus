#pragma once

#include "numa.hpp"
#include <cstdlib>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/parallel.hpp>
#include <memory>
#include <utmpx.h>

#define _JAFFAR_STATE_PADDING_BYTES 64

namespace jaffarPlus
{

class StateDb
{
public:
  StateDb(Runner& r, const nlohmann::json& config) : _runner(&r)
  {
    // Initialization message
    jaffarCommon::logger::log("[J+] Initializing State Database...\n");

    ///////// Getting original state sizes from the runner

    // Checking maximum db sizes per each numa group
    _maxSizeMb = jaffarCommon::json::getNumber<size_t>(config, "Max Size (Mb)");

    // Overriding if provided
    if (auto* value = std::getenv("JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB")) _maxSizeMb = std::stoul(value);
  }

  ~StateDb() = default;

  __INLINE__ void initialize()
  {
    // Getting game state size
    _stateSizeRaw = _runner->getStateSize();

    // We want to align each state to 512 bits (64 bytes) to favor vectorized access
    // Now calculating the necessary padding to reach the next multiple of 64 bytes
    _stateSize = _stateSizeRaw;
    if (_stateSize % _JAFFAR_STATE_PADDING_BYTES != 0) _stateSize = ((_stateSizeRaw / _JAFFAR_STATE_PADDING_BYTES) + 1) * _JAFFAR_STATE_PADDING_BYTES;

    // Padding is the difference between the aligned state size and the raw one
    _stateSizePadding = _stateSize - _stateSizeRaw;

    // Setting counters for database state popping
    _numaNonLocalDatabaseStateCount = 0;
    _numaLocalDatabaseStateCount    = 0;
    _numaDatabaseStateNotFoundCount = 0;

    // Setting counters for free state getting
    _numaNonLocalFreeStateCount = 0;
    _numaLocalFreeStateCount    = 0;
    _numaFreeStateNotFoundCount = 0;
    _numaStealingFreeStateCount = 0;
    _numaDistanceAccumulator = 0;

    // Splitting state database equally among numa domains
    size_t stateDbSizePerNUMA = std::ceil((_maxSizeMb * 1024 * 1024) / _numaCount);
    for (int i = 0; i < _numaCount; i++) _maxSizePerNuma.push_back(stateDbSizePerNUMA);

    // Getting maximum allocatable memory in each NUMA domain
    std::vector<size_t> maxFreeMemoryPerNuma(_numaCount);
    for (int i = 0; i < _numaCount; i++)
    {
      size_t freeMemory = 0;
      numa_node_size64(i, (long long*)&freeMemory);
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
    }

    // Allocating state databases in the correponding NUMA domains
    _numaCurrentStateQueues.resize(_numaCount);
    _numaNextStateQueues.resize(_numaCount);
    JAFFAR_PARALLEL
    {
      if (_myThreadId == _numaDelegateThreadId[_preferredNumaDomain])
      {
        _numaCurrentStateQueues[_preferredNumaDomain] = new jaffarCommon::concurrent::Deque<void*>();
        _numaNextStateQueues[_preferredNumaDomain] = new jaffarCommon::concurrent::concurrentMultimap_t<float, void*>();
      }
    }

    // Getting number of bytes to reserve for each NUMA domain
    _allocableBytesPerNuma.resize(_numaCount);
    for (int i = 0; i < _numaCount; i++) _allocableBytesPerNuma[i] = _maxStatesPerNuma[i] * _stateSize;

    // Creating internal buffers, one per NUMA domain
    _internalBuffersStart.resize(_numaCount);
    _internalBuffersEnd.resize(_numaCount);
    for (int i = 0; i < _numaCount; i++)
    {
      _internalBuffersStart[i] = (uint8_t*)numa_alloc_onnode(_allocableBytesPerNuma[i], i);
      if (_internalBuffersStart[i] == NULL) JAFFAR_THROW_RUNTIME("Error trying to allocate memory for numa domain %d\n", i);
      _internalBuffersEnd[i] = &_internalBuffersStart[i][_allocableBytesPerNuma[i]];
    }

    // Getting system's page size (typically 4K but it may change in the future)
    const size_t pageSize = sysconf(_SC_PAGESIZE);

    // Initializing the internal buffers
    for (int numaNodeIdx = 0; numaNodeIdx < _numaCount; numaNodeIdx++) JAFFAR_PARALLEL_FOR
    for (size_t i = 0; i < _allocableBytesPerNuma[numaNodeIdx]; i += pageSize) _internalBuffersStart[numaNodeIdx][i] = 1;

    // Adding the state pointers to the free state queues
    _freeStateQueues.resize(_numaCount);
    JAFFAR_PARALLEL
    {
      if (_myThreadId == _numaDelegateThreadId[_preferredNumaDomain])
      {
       _freeStateQueues[_preferredNumaDomain] = std::make_unique<jaffarCommon::concurrent::atomicQueue_t<void*>>(_maxStatesPerNuma[_preferredNumaDomain]);
       for (size_t i = 0; i < _maxStatesPerNuma[_preferredNumaDomain]; i++) _freeStateQueues[_preferredNumaDomain]->try_push((void*)&_internalBuffersStart[_preferredNumaDomain][i * _stateSize]);
      }
    }
  }

  /**
   * Saves the runner state into the provided state data pointer
   */
  __INLINE__ size_t saveStateFromRunner(Runner& r, void* statePtr) const
  {
    // Storage for the state size after deserialization
    size_t serializedSize = 0;

    // Serializing the runner state into the memory received
    jaffarCommon::serializer::Contiguous s(statePtr, _stateSizeRaw);
    r.serializeState(s);
    serializedSize = s.getOutputSize();

    return serializedSize;
  }

  /**
   * Serializes the runner state and pushes it into the state database
   */
  __INLINE__ bool pushState(const float reward, Runner& r, void* statePtr)
  {
    // Check that we got a free state (we did not overflow state memory)
    if (statePtr == nullptr) JAFFAR_THROW_RUNTIME("Provided a null state -- probably ran out of free states\n");

    // Encoding internal runner state into the state pointer
    try
    {
      saveStateFromRunner(r, statePtr);
    }
    catch (const std::runtime_error& x)
    {
      // If failed return false
      return false;
    }

    // Inserting new state into the next state database
    // Getting the corresponding numa domain for this state
    int targetNumaIdx = getStateNumaDomain(statePtr);

    // Inserting new state into the next state database
    _numaNextStateQueues[targetNumaIdx]->insert({reward, statePtr});

    // If succeeded, return true
    return true;
  }

  /**
   * Loads the state into the runner
   */
  __INLINE__ void loadStateIntoRunner(Runner& r, const void* statePtr)
  {
    // Deserializing the runner state from the memory received
    jaffarCommon::deserializer::Contiguous d(statePtr, _stateSizeRaw);
    r.deserializeState(d);
  }

  // Function to print relevant information
  __INLINE__ void printInfo() const
  {
    const size_t currentStateCount = getStateCount();
    const size_t currentStateBytes = currentStateCount * _stateSize;

    jaffarCommon::logger::log("[J+]  + Current State Count:           %lu (%f Mstates) /  %lu (%f Mstates) Max / %5.2f%% Full\n", currentStateCount,
                              (double)currentStateCount * 1.0e-6, _maxStates, (double)_maxStates * 1.0e-6, 100.0 * (double)currentStateCount / (double)_maxStates);
    jaffarCommon::logger::log("[J+]  + Current State Size:            %.3f Mb (%.6f Gb) / %.3f Mb (%.6f Gb) Max\n", (double)currentStateBytes / (1024.0 * 1024.0),
                              (double)currentStateBytes / (1024.0 * 1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0 * 1024.0));
    jaffarCommon::logger::log("[J+]  + State Size Raw:                %lu bytes\n", _stateSizeRaw);
    jaffarCommon::logger::log("[J+]  + State Size in DB:              %lu bytes (%lu padding bytes to %u)\n", _stateSize, _stateSizePadding, _JAFFAR_STATE_PADDING_BYTES);

    // Only print the first and the last
    for (int i = 0; i < _numaCount; i++) if (i == 0 || i == _numaCount - 1)
      jaffarCommon::logger::log("[J+]  + NUMA Domain %3d                States: %lu (Max: %lu), Size: %.3f Mb (%.6f Gb)\n", i, _currentStatesPerNuma[i], _maxStatesPerNuma[i],
                                (double)_maxSizePerNuma[i] / (1024.0 * 1024.0), (double)_maxSizePerNuma[i] / (1024.0 * 1024.0 * 1024.0));

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

    size_t NUMAAccessCount = _numaNonLocalDatabaseStateCount + _numaLocalDatabaseStateCount + _numaNonLocalFreeStateCount + _numaLocalFreeStateCount + _numaStealingFreeStateCount;
    jaffarCommon::logger::log("[J+]  + Average NUMA Distance:                          %lu / %lu = %5.3f\n", _numaDistanceAccumulator.load(), NUMAAccessCount,
                              (double)_numaDistanceAccumulator.load() / (double)NUMAAccessCount);
                              
  }

  __INLINE__ void* getFreeState()
  {
    // Storage for the new free state space
    void* stateSpace;

    // Trying to get free space for a new state
    for (size_t i = 0; i < (size_t)_numaCount; i++)
    {
      const auto numaIdx = _numaPreferenceMatrix[_preferredNumaDomain][i];
      bool success = _freeStateQueues[numaIdx]->try_pop(stateSpace);

      // If successful, return the pointer immediately
      if (success == true)
      {
        _numaDistanceAccumulator += _numaDistanceMatrix[_preferredNumaDomain][numaIdx];
        if (numaIdx == (size_t)_preferredNumaDomain) _numaLocalFreeStateCount++;
        else _numaNonLocalFreeStateCount++;
        return stateSpace;
      }
    }

    // If failed, then try to get it from the back of my numa-specific queues. Looking for the worst state possible

    // Trying to get free space for a new state
    for (size_t i = 0; i < (size_t)_numaCount; i++)
    {
      const auto numaIdx = _numaPreferenceMatrix[_preferredNumaDomain][i];
      bool success = _numaCurrentStateQueues[numaIdx]->pop_back_get(stateSpace);

      // If successful, return the pointer immediately
      if (success == true)
      {
        _numaDistanceAccumulator += _numaDistanceMatrix[_preferredNumaDomain][numaIdx];
        _numaStealingFreeStateCount++;
        return stateSpace;
      }
    }

    // Otherwise, return a null pointer. The state will be discarded
    _numaFreeStateNotFoundCount++;
    return nullptr;
  }

  __INLINE__ int getStateNumaDomain(void* const statePtr)
  {
    for (int i = 0; i < _numaCount; i++)
      if (isStateInNumaDomain(statePtr, i)) return i;

    // Check for error
    JAFFAR_THROW_RUNTIME("Did not find the corresponding numa domain for the provided state pointer. This must be a bug in Jaffar\n");
  }

  __INLINE__ bool isStateInNumaDomain(void* const statePtr, const int numaDomainId)
  {
    return statePtr >= _internalBuffersStart[numaDomainId] && statePtr <= _internalBuffersEnd[numaDomainId];
  }

  __INLINE__ void returnFreeState(void* const statePtr)
  {
    // Finding out to which database this state pointer belongs to
    const auto numaIdx = getStateNumaDomain(statePtr);

    // Pushing the state in the corresponding queue
    bool success = _freeStateQueues[numaIdx]->try_push(statePtr);

    // Check for success
    if (success == false) JAFFAR_THROW_RUNTIME("Failed on pushing free state back. This must be a bug in Jaffar\n");
  }

  /**
   * Copies the pointers from the next state database into the current one, starting with the largest rewards, and clears it.
   */
  __INLINE__ void advanceStep()
  {
    // // Resetting NUMA distance accumulator
    // _numaDistanceAccumulator = 0;

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
        void* statePtr    = nullptr;

        // Establishing best state (the first of this numa queue is a candidate)
        if (_numaNextStateQueues[_preferredNumaDomain]->begin() != _numaNextStateQueues[_preferredNumaDomain]->end())
        {
          const auto& stateItr = _numaNextStateQueues[_preferredNumaDomain]->begin();
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
          const auto& stateItr = _numaNextStateQueues[_preferredNumaDomain]->begin();
          stateReward          = stateItr->first;
          statePtr             = stateItr->second;

          // Extracting next state
          _numaNextStateQueues[_preferredNumaDomain]->unsafe_extract(stateItr);

          // Get target NUMA domain for the state
          int targetNumaIdx = getStateNumaDomain(statePtr);

          // Pushing state in the NUMA domain that it belongs to.
          // If not possible, at least to the closest one possible
          bool success = false;
          for (size_t i = 0; i < (size_t)_numaCount; i++)
          {
            const auto numaIdx = _numaPreferenceMatrix[targetNumaIdx][i];
            if (_numaCurrentStateQueues[numaIdx]->wasSize() < _maxStatesPerNuma[numaIdx])
            {
             _numaCurrentStateQueues[numaIdx]->push_back(statePtr);
             success = true;
             break;
            }

            // Check for success
            if (success == false) JAFFAR_THROW_RUNTIME("Failed on pushing new state. This must be a bug in Jaffar\n");
          }
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

  __INLINE__ void* popState()
  {
    // Pointer to return
    void* statePtr;

    // Trying to next state
    for (size_t i = 0; i < (size_t)_numaCount; i++)
    {
      const auto numaIdx = _numaPreferenceMatrix[_preferredNumaDomain][i];
      bool success = _numaCurrentStateQueues[numaIdx]->pop_front_get(statePtr);

      // If successful, return the pointer immediately
      if (success == true)
      {
        _numaDistanceAccumulator += _numaDistanceMatrix[_preferredNumaDomain][numaIdx];
        if (numaIdx == (size_t)_preferredNumaDomain) _numaLocalDatabaseStateCount++;
        else _numaNonLocalDatabaseStateCount++;
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
  __INLINE__ size_t getStateCount() const
  {
    size_t stateCount = 0;
    for (int i = 0; i < _numaCount; i++) stateCount += _numaCurrentStateQueues[i]->wasSize();
    return stateCount;
  }

  /**
   * This function returns a pointer to the best state found in the current state database
   */
  void* getBestState() const { return _bestState; }

  /**
   * This function returns a pointer to the worst state found in the current state database
   */
  void* getWorstState() const { return _worstState; }

private:

  void* _bestState  = nullptr;
  void* _worstState = nullptr;

  Runner* const _runner;

  /**
   * Stores the size occupied by each state (with padding)
   */
  size_t _stateSize;

  /**
   * Stores the size occupied by each state
   */
  size_t _stateSizeRaw;

  /**
   * Stores the size actually occupied by each state, after adding padding
   */
  size_t _stateSizePadding;

  /**
   * Maximum size (bytes) for the state database to grow
   */
  size_t _maxSize;

  /**
   * Number of maximum states in execution
   */
  size_t _maxStates;

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
   * Accumulator for NUMA distance
   */
  std::atomic<size_t> _numaDistanceAccumulator;

  /**
   * User-provided maximum megabytes to use for the state database
   */
  size_t _maxSizeMb;

  /**
   * State database per numa domain
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
   * Mutex to set delegate thread id and finding best/worst states
   */
  std::mutex _workMutex;

  /**
   * Numa state queues allow the thread to search for a current state that belongs to it through the current state database
   */
  std::vector<jaffarCommon::concurrent::Deque<void*>*> _numaCurrentStateQueues;

  /**
   * Numa state queues for the next step's states
   */
  std::vector<jaffarCommon::concurrent::concurrentMultimap_t<float, void*>*> _numaNextStateQueues;

  /**
   * This queue will hold pointers to all the free state storage
   */
  std::vector<std::unique_ptr<jaffarCommon::concurrent::atomicQueue_t<void*>>> _freeStateQueues;

  /**
   * Start pointer for the internal buffers for the state database
   */
  std::vector<uint8_t*> _internalBuffersStart;

  /**
   * End pointer for each of the internal buffers
   */
  std::vector<uint8_t*> _internalBuffersEnd;

  /**
   * Number of bytes to allocate per NUMA domain
   */
  std::vector<size_t> _allocableBytesPerNuma;
};

} // namespace jaffarPlus
