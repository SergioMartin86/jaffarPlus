#pragma once

#include "numa.hpp"
#include <cstdlib>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/parallel.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <memory>
#include <utmpx.h>

#define _JAFFAR_STATE_PADDING_BYTES 16

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

  ~StateDb()
  {
    // Free the per-NUMA queues allocated with `new` in initialize() (one per delegate domain; the
    // other entries are nullptr, which delete safely ignores).
    for (auto* queue : _numaCurrentStateQueues) delete queue;
    for (auto* queue : _numaNextStateQueues) delete queue;
  }

  __INLINE__ void initialize()
  {
    // Getting game state size
    _stateSizeRaw = _runner->getStateSize();

    // States are stored raw (uncompressed). We align each state to a padding boundary to favor
    // vectorized access and prevent false sharing.
    _stateSize = _stateSizeRaw;

    // Padding state size to enforce alignment and prevent false sharing as much as possible
    const size_t baseStateSize = _stateSize;
    if (_stateSize % _JAFFAR_STATE_PADDING_BYTES != 0) _stateSize = ((baseStateSize / _JAFFAR_STATE_PADDING_BYTES) + 1) * _JAFFAR_STATE_PADDING_BYTES;

    // Padding is the difference between the aligned state size and the raw one
    _stateSizePadding = _stateSize - baseStateSize;

    // Allocating/zeroing the per-thread statistics counters. These were previously global atomics
    // bumped on every pop/get-free; at high thread count that single-cache-line ping-pong cost more
    // than the work it measured. They are now plain per-thread counters (indexed by the dense OpenMP
    // thread id) reduced only when printInfo() is called.
    _statCounters.assign(_threadCount, statCounters_t{});

    // Allocating/zeroing the per-thread free-slot caches (empty at start)
    _freeStateCache.assign(_threadCount, freeStateCache_t{});

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
    // The current-state queue is a DrainBuffer: it is filled single-threaded in advanceStep() and
    // then drained concurrently from both ends by the workers, never both at once (the step barrier
    // separates the phases). Its capacity is the NUMA domain's full state count -- it can never hold
    // more current states than that. The claim counters are 32-bit, so guard the capacity.
    for (int i = 0; i < _numaCount; i++)
      if (_maxStatesPerNuma[i] > (size_t)UINT32_MAX)
        JAFFAR_THROW_RUNTIME("State count per NUMA domain (%lu) exceeds the DrainBuffer capacity limit (%u)\n", _maxStatesPerNuma[i], UINT32_MAX);

    JAFFAR_PARALLEL
    {
      if (_myThreadId == _numaDelegateThreadId[_preferredNumaDomain])
      {
        auto* currentQueue = new jaffarCommon::concurrent::DrainBuffer<void*>();
        currentQueue->reserve(_maxStatesPerNuma[_preferredNumaDomain]);
        _numaCurrentStateQueues[_preferredNumaDomain] = currentQueue;
        _numaNextStateQueues[_preferredNumaDomain]    = new jaffarCommon::concurrent::concurrentMultimap_t<float, void*>();
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
        for (size_t i = 0; i < _maxStatesPerNuma[_preferredNumaDomain]; i++)
          _freeStateQueues[_preferredNumaDomain]->try_push((void*)&_internalBuffersStart[_preferredNumaDomain][i * _stateSize]);
      }
    }
  }

  /**
   * Saves the runner state into the provided state data pointer
   */
  __INLINE__ size_t saveStateFromRunner(Runner& r, void* statePtr)
  {
    // Serializing the runner state directly (raw, uncompressed) into the destination slot
    jaffarCommon::serializer::Contiguous s(statePtr, _stateSizeRaw);
    r.serializeState(s);
    return s.getOutputSize();
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
      // Getting state from the runner
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
    // Deserializing the raw (uncompressed) state directly from the slot into the runner
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
    for (int i = 0; i < _numaCount; i++)
      if (i == 0 || i == _numaCount - 1)
        jaffarCommon::logger::log("[J+]  + NUMA Domain %3d                States: %lu (Max: %lu), Size: %.3f Mb (%.6f Gb)\n", i, _currentStatesPerNuma[i], _maxStatesPerNuma[i],
                                  (double)_maxSizePerNuma[i] / (1024.0 * 1024.0), (double)_maxSizePerNuma[i] / (1024.0 * 1024.0 * 1024.0));

    // Reducing the per-thread statistics counters into totals (off the hot path, only when printing)
    size_t localDatabaseState = 0, nonLocalDatabaseState = 0, databaseStateNotFound = 0;
    size_t localFreeState = 0, nonLocalFreeState = 0, stealingFreeState = 0, freeStateNotFound = 0;
    size_t distanceAccumulator = 0;
    size_t freeStateCacheHit = 0, freeStateCacheReturn = 0;
    for (const auto& sc : _statCounters)
    {
      localDatabaseState += sc.localDatabaseState;
      nonLocalDatabaseState += sc.nonLocalDatabaseState;
      databaseStateNotFound += sc.databaseStateNotFound;
      localFreeState += sc.localFreeState;
      nonLocalFreeState += sc.nonLocalFreeState;
      stealingFreeState += sc.stealingFreeState;
      freeStateNotFound += sc.freeStateNotFound;
      distanceAccumulator += sc.distanceAccumulator;
      freeStateCacheHit += sc.freeStateCacheHit;
      freeStateCacheReturn += sc.freeStateCacheReturn;
    }

    size_t totalDatabaseStatesRequested = nonLocalDatabaseState + localDatabaseState + databaseStateNotFound;
    jaffarCommon::logger::log("[J+] + Database Popping State Rates:\n");
    jaffarCommon::logger::log("[J+]  + Numa Locality Success Rate:                     %5.3f%%\n", 100.0 * (double)localDatabaseState / (double)totalDatabaseStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa Locality Fail Rate:                        %5.3f%%\n", 100.0 * (double)nonLocalDatabaseState / (double)totalDatabaseStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa No DB State Found Rate:                    %5.3f%%\n", 100.0 * (double)databaseStateNotFound / (double)totalDatabaseStatesRequested);

    // Thread-local free-slot cache effectiveness: fraction of all getFreeState / returnFreeState
    // calls served locally (and thus kept off the shared free-state queue).
    const size_t totalGets    = freeStateCacheHit + nonLocalFreeState + localFreeState + stealingFreeState + freeStateNotFound;
    const size_t totalReturns = freeStateCacheReturn + /* shared-queue spills are not separately counted */ 0;
    jaffarCommon::logger::log("[J+] + Free-Slot Cache:\n");
    jaffarCommon::logger::log("[J+]  + Get Cache Hit Rate:                             %5.3f%% (%lu hits)\n",
                              totalGets == 0 ? 0.0 : 100.0 * (double)freeStateCacheHit / (double)totalGets, freeStateCacheHit);
    jaffarCommon::logger::log("[J+]  + Return Cache Absorb Count:                      %lu\n", freeStateCacheReturn);
    (void)totalReturns;

    size_t totalFreeStatesRequested = nonLocalFreeState + localFreeState + freeStateNotFound + stealingFreeState;
    jaffarCommon::logger::log("[J+] + Get Free State Rates (shared-queue only):\n");
    jaffarCommon::logger::log("[J+]  + Numa Locality Success Rate:                     %5.3f%%\n", 100.0 * (double)localFreeState / (double)totalFreeStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa Locality Fail Rate:                        %5.3f%%\n", 100.0 * (double)nonLocalFreeState / (double)totalFreeStatesRequested);
    jaffarCommon::logger::log("[J+]  + State DB Stealing Rate:                         %5.3f%%\n", 100.0 * (double)stealingFreeState / (double)totalFreeStatesRequested);
    jaffarCommon::logger::log("[J+]  + Numa No Free State Found Rate:                  %5.3f%%\n", 100.0 * (double)freeStateNotFound / (double)totalFreeStatesRequested);

    size_t NUMAAccessCount = nonLocalDatabaseState + localDatabaseState + nonLocalFreeState + localFreeState + stealingFreeState;
    jaffarCommon::logger::log("[J+]  + Average NUMA Distance:                          %lu / %lu = %5.3f\n", distanceAccumulator, NUMAAccessCount,
                              (double)distanceAccumulator / (double)NUMAAccessCount);
  }

  __INLINE__ void* getFreeState(const size_t threadId)
  {
    auto& sc = _statCounters[threadId];

    // First, try this thread's own free-slot cache (a "magazine"). A slot this worker freed earlier
    // is served straight back to it with no shared-queue atomic and no NUMA lookup -- and it is the
    // hottest, most NUMA-local memory available (LIFO). This recycling is where the win comes from:
    // a worker frees ~1 slot and requests several per base state, so each free typically satisfies a
    // subsequent request locally, removing it from both the get and the return shared-queue traffic.
    auto& cache = _freeStateCache[threadId];
    if (cache.count > 0)
    {
      sc.freeStateCacheHit++;
      return cache.slots[--cache.count];
    }

    // Storage for the new free state space
    void* stateSpace;

    // Trying to get free space for a new state
    for (size_t i = 0; i < (size_t)_numaCount; i++)
    {
      const auto numaIdx = _numaPreferenceMatrix[_preferredNumaDomain][i];
      // Skip NUMA domains with no worker thread: their queues are never allocated (see initialize()).
      if (_freeStateQueues[numaIdx] == nullptr) continue;
      bool success = _freeStateQueues[numaIdx]->try_pop(stateSpace);

      // If successful, return the pointer immediately
      if (success == true)
      {
        sc.distanceAccumulator += _numaDistanceMatrix[_preferredNumaDomain][numaIdx];
        if (numaIdx == (size_t)_preferredNumaDomain)
          sc.localFreeState++;
        else
          sc.nonLocalFreeState++;
        return stateSpace;
      }
    }

    // If failed, then try to get it from the back of my numa-specific queues. Looking for the worst state possible

    // Trying to get free space for a new state
    for (size_t i = 0; i < (size_t)_numaCount; i++)
    {
      const auto numaIdx = _numaPreferenceMatrix[_preferredNumaDomain][i];
      if (_numaCurrentStateQueues[numaIdx] == nullptr) continue;
      bool success = _numaCurrentStateQueues[numaIdx]->pop_back_get(stateSpace);

      // If successful, return the pointer immediately
      if (success == true)
      {
        sc.distanceAccumulator += _numaDistanceMatrix[_preferredNumaDomain][numaIdx];
        sc.stealingFreeState++;
        return stateSpace;
      }
    }

    // Otherwise, return a null pointer. The state will be discarded
    sc.freeStateNotFound++;
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

  __INLINE__ void returnFreeState(void* const statePtr, const size_t threadId)
  {
    // Return the slot to this thread's own free-slot cache when there is room, so the next
    // getFreeState() on this thread can reuse it without touching the shared queue (no atomic, no
    // NUMA scan). The cache is bounded so a worker can never hoard more than a negligible number of
    // slots from the shared pool (FREE_STATE_CACHE_CAPACITY * threadCount total).
    auto& cache = _freeStateCache[threadId];
    if (cache.count < FREE_STATE_CACHE_CAPACITY)
    {
      _statCounters[threadId].freeStateCacheReturn++;
      cache.slots[cache.count++] = statePtr;
      return;
    }

    // Cache full: spill to the shared free-state queue of the slot's owning NUMA domain
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
        auto& nextMap     = *_numaNextStateQueues[_preferredNumaDomain];
        auto* currentBuf  = _numaCurrentStateQueues[_preferredNumaDomain];

        // Updating state count per numa
        _currentStatesPerNuma[_preferredNumaDomain] = nextMap.size();

        // Resetting the current-state buffer for this step's fill (it was drained by the workers in
        // the previous step; the step barrier guarantees no drain is in flight here)
        currentBuf->clear();

        // Drain the (reward-ordered) next-state map into the current buffer in a single forward
        // traversal, then free the whole map at once. This replaces the previous N-iteration loop of
        // begin()+unsafe_extract (each of which unlinks and frees one node): a plain traversal is
        // cheaper per element, and a single bulk clear() deallocates all nodes without the per-node
        // extract bookkeeping. The map iterates in descending-reward order (std::greater), so the
        // first element seen is the best and the last is the worst -- same ordering as before.
        float firstReward = 0.0f, lastReward = 0.0f;
        void* firstPtr = nullptr, * lastPtr = nullptr;
        for (const auto& entry : nextMap)
        {
          if (firstPtr == nullptr)
          {
            firstReward = entry.first;
            firstPtr    = entry.second;
          }
          currentBuf->push_back_no_lock(entry.second);
          lastReward = entry.first;
          lastPtr    = entry.second;
        }

        // Releasing all next-map nodes in one go
        nextMap.clear();

        // Updating the cross-NUMA best/worst states (shared, hence locked) once per domain
        if (firstPtr != nullptr)
        {
          _workMutex.lock();
          if (firstReward > bestStateReward)
          {
            bestStateReward = firstReward;
            _bestState      = firstPtr;
          }
          if (lastReward < worstStateReward)
          {
            worstStateReward = lastReward;
            _worstState      = lastPtr;
          }
          _workMutex.unlock();
        }
      }
    }
  }

  __INLINE__ void* popState()
  {
    auto& sc = _statCounters[jaffarCommon::parallel::getThreadId()];

    // Pointer to return
    void* statePtr;

    // Trying to next state
    for (size_t i = 0; i < (size_t)_numaCount; i++)
    {
      const auto numaIdx = _numaPreferenceMatrix[_preferredNumaDomain][i];
      // Skip NUMA domains with no worker thread: their queues are never allocated (see initialize()).
      if (_numaCurrentStateQueues[numaIdx] == nullptr) continue;
      bool success = _numaCurrentStateQueues[numaIdx]->pop_front_get(statePtr);

      // If successful, return the pointer immediately
      if (success == true)
      {
        sc.distanceAccumulator += _numaDistanceMatrix[_preferredNumaDomain][numaIdx];
        if (numaIdx == (size_t)_preferredNumaDomain)
          sc.localDatabaseState++;
        else
          sc.nonLocalDatabaseState++;
        return statePtr;
      }
    }

    // If no success at all, just return a nullptr
    sc.databaseStateNotFound++;
    return nullptr;
  }

  /**
   * Pops a batch of base states from the current state database into the provided buffer,
   * preferring NUMA-local domains. Returns the number of states actually retrieved.
   *
   * This is the batched counterpart of popState(): it grabs a whole run of states under a single
   * Deque-lock acquisition (per NUMA domain) instead of locking once per state. On a light
   * emulator with dozens of worker threads per NUMA domain, the single-state lock/unlock is the
   * dominant cost ("Popping Base State" ~37% of wall time); batching amortizes it ~maxCount-fold.
   * Statistics counters are accumulated once per batch rather than once per state.
   */
  __INLINE__ size_t popStates(void** elements, const size_t maxCount, const size_t threadId)
  {
    auto& sc = _statCounters[threadId];

    // Trying the NUMA domains in preference order; take the whole batch from the first non-empty one
    for (size_t i = 0; i < (size_t)_numaCount; i++)
    {
      const auto numaIdx = _numaPreferenceMatrix[_preferredNumaDomain][i];
      // Skip NUMA domains with no worker thread: their queues are never allocated (see initialize()).
      if (_numaCurrentStateQueues[numaIdx] == nullptr) continue;

      const size_t count = _numaCurrentStateQueues[numaIdx]->pop_front_get_batch(elements, maxCount);

      // If we got anything, account for it once for the whole batch and return
      if (count > 0)
      {
        sc.distanceAccumulator += _numaDistanceMatrix[_preferredNumaDomain][numaIdx] * count;
        if (numaIdx == (size_t)_preferredNumaDomain)
          sc.localDatabaseState += count;
        else
          sc.nonLocalDatabaseState += count;
        return count;
      }
    }

    // If no success at all, the databases are drained for this step
    sc.databaseStateNotFound++;
    return 0;
  }

  /**
   * Gets the current number of states in the current state database
   */
  __INLINE__ size_t getStateCount() const
  {
    size_t stateCount = 0;
    // Per-NUMA current-state queues are only allocated for domains that have a delegate thread.
    // Skip any unallocated (null) queue so this (informational) count can't dereference a null
    // pointer when threads don't cover every NUMA domain.
    for (int i = 0; i < _numaCount; i++)
      if (_numaCurrentStateQueues[i] != nullptr) stateCount += _numaCurrentStateQueues[i]->wasSize();
    return stateCount;
  }

  /**
   * This function returns a pointer to the best state found in the current state database
   */
  __INLINE__ void* getBestState() const { return _bestState; }

  /**
   * This function returns a pointer to the worst state found in the current state database
   */
  __INLINE__ void* getWorstState() const { return _worstState; }

  /**
   * Gets the state sizes, as stored in the database
   */
  __INLINE__ size_t getStateSizeInDatabase() const { return _stateSize; }

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
   * Per-thread statistics counters for state-DB popping / free-state acquisition.
   *
   * These were global atomics incremented on every single pop/get-free; at high thread count the
   * shared cache line ping-ponged across all cores and cost more than the operation it measured.
   * They are now plain per-thread counters, cache-line aligned to avoid false sharing between
   * adjacent threads, and reduced into totals only when printInfo() runs (off the hot path).
   */
  struct alignas(64) statCounters_t
  {
    size_t localDatabaseState    = 0;
    size_t nonLocalDatabaseState = 0;
    size_t databaseStateNotFound = 0;
    size_t localFreeState        = 0;
    size_t nonLocalFreeState     = 0;
    size_t stealingFreeState     = 0;
    size_t freeStateNotFound     = 0;
    size_t distanceAccumulator   = 0;
    size_t freeStateCacheHit     = 0; // getFreeState served from the thread-local cache
    size_t freeStateCacheReturn  = 0; // returnFreeState absorbed by the thread-local cache
  };
  std::vector<statCounters_t> _statCounters;

  /**
   * Capacity of each worker's thread-local free-slot cache ("magazine"). Small: it only needs to
   * bridge a worker's own returns to its own subsequent getFreeState() requests within a step, so a
   * handful of slots captures essentially all of the recycling. Kept tiny so the total number of
   * slots parked in caches (this * threadCount) stays a negligible fraction of the state DB, and so
   * memory pressure / drop behaviour is effectively unchanged.
   */
  static constexpr size_t FREE_STATE_CACHE_CAPACITY = 32;

  /**
   * Per-thread (OpenMP-thread-indexed) cache of free state slots, fronting the shared free-state
   * queues. Cache-line aligned so adjacent threads' caches never share a line (no false sharing).
   */
  struct alignas(64) freeStateCache_t
  {
    size_t count = 0;
    void*  slots[FREE_STATE_CACHE_CAPACITY];
  };
  std::vector<freeStateCache_t> _freeStateCache;

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
  std::vector<jaffarCommon::concurrent::DrainBuffer<void*>*> _numaCurrentStateQueues;

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
