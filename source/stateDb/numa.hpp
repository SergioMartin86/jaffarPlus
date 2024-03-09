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
thread_local static int preferredNumaDomain;

class Numa : public stateDb::Base
{
  public:
  Numa(Runner &r, const nlohmann::json &config) : stateDb::Base(r, config)
  {
    // Checking whether the numa library calls are available
    const auto numaAvailable = numa_available();
    if (numaAvailable != 0) JAFFAR_THROW_RUNTIME("NUMA State Db selected, but the system does not provide NUMA support.");

    // Getting number of noma domains
    _numaCount = numa_max_node() + 1;

    // Checking maximum db sizes per each numa
    const auto &maxSizePerNumaMbJs = jaffarCommon::json::getArray<size_t>(config, "Max Size per NUMA Domain (Mb)");
    if (maxSizePerNumaMbJs.size() != (size_t)_numaCount) JAFFAR_THROW_LOGIC("System has %d NUMA domains but only sizes for %lu of them provided.", _numaCount, maxSizePerNumaMbJs.size());

    // Getting scavenge depth
    _scavengerQueuesSize = jaffarCommon::json::getNumber<size_t>(config, "Scavenger Queues Size");
    _scavengingDepth = jaffarCommon::json::getNumber<size_t>(config, "Scavenging Depth");

    // Creating scavenger queues
    for (int i = 0; i < _numaCount; i++) _scavengerQueues.push_back(std::make_unique<jaffarCommon::concurrent::Deque<void *>>());

    // Getting maximum state db size in Mb and bytes
    size_t numaSizeSum = 0;
    for (const auto &entry : maxSizePerNumaMbJs)
    {
      auto sizeMb = entry;

      // For testing purposes, the maximum size 
      if (auto* value = std::getenv("JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB")) sizeMb = std::stoul(value) / _numaCount;

      _maxSizePerNumaMb.push_back(sizeMb);
      _maxSizePerNuma.push_back(sizeMb * 1024ul * 1024ul);
      numaSizeSum += sizeMb;
    }

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
    for (int i = 0; i < _numaCount; i++) _maxStatesPerNuma[i] = _maxSizePerNuma[i] / _stateSize;

    // Getting totals for statistics
    _maxSize = 0;
    _maxStates = 0;
    for (int i = 0; i < _numaCount; i++)
    {
      _maxSize += _maxSizePerNuma[i];
      _maxStates += _maxStatesPerNuma[i];
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

    // Determining the preferred numa domain for each thread. This depends on OpenMP using always the same set of threads.
    JAFFAR_PARALLEL
    {
      int cpu = sched_getcpu();
      int node = numa_node_of_cpu(cpu);
      preferredNumaDomain = node;
    }
  }

  ~Numa() = default;

  void initialize() override
  {
    // Getting system's page size (typically 4K but it may change in the future)
    const size_t pageSize = sysconf(_SC_PAGESIZE);

    // Initializing the internal buffers
    for (int numaNodeIdx = 0; numaNodeIdx < _numaCount; numaNodeIdx++)
     JAFFAR_PARALLEL_FOR
     for (size_t i = 0; i < _allocableBytesPerNuma[numaNodeIdx]; i += pageSize) _internalBuffersStart[numaNodeIdx][i] = 1;

    // Adding the state pointers to the free state queues
    _freeStateQueues.resize(_numaCount);
    for (int numaNodeIdx = 0; numaNodeIdx < _numaCount; numaNodeIdx++)
    {
      _freeStateQueues[numaNodeIdx] = std::make_unique<jaffarCommon::concurrent::atomicQueue_t<void *>>(_maxStatesPerNuma[numaNodeIdx]);
      for (size_t i = 0; i < _maxStatesPerNuma[numaNodeIdx]; i++)
        _freeStateQueues[numaNodeIdx]->try_push((void *)&_internalBuffersStart[numaNodeIdx][i * _stateSize]);
    }
  }

  // Function to print relevant information
  void printInfoImpl() const override
  {
    for (int i = 0; i < _numaCount; i++)
      jaffarCommon::logger::log("[J++]  + NUMA Domain %d                  Max States: %lu, Size: %.3f Mb (%.6f Gb)\n", i, _maxStatesPerNuma[i], (double)_maxSizePerNuma[i] / (1024.0 * 1024.0), (double)_maxSizePerNuma[i] / (1024.0 * 1024.0 * 1024.0));
  }

  inline void *getFreeState() override
  {
    // Storage for the new free state space
    void *stateSpace;

    // Trying to get free space for a new state
    bool success = _freeStateQueues[preferredNumaDomain]->try_pop(stateSpace);

    // If successful, return the pointer immediately
    if (success == true) return stateSpace;

    // Trying all other free state queues now
    for (int i = 0; (size_t)i < _freeStateQueues.size(); i++)
      if (i != preferredNumaDomain)
      {
        // Trying to get free space for a new state
        bool success = _freeStateQueues[i]->try_pop(stateSpace);

        // If successful, return the pointer immediately
        if (success == true) return stateSpace;
      }

    // If failed, then try to get it from the back of the current state database
    success = _currentStateDb.pop_back_get(stateSpace);

    // If successful, return the pointer immediately
    if (success == true) return stateSpace;

    // Otherwise, return a null pointer. The state will be discarded
    return nullptr;
  }

  inline int getStateNumaDomain(void *const statePtr)
  {
    for (int i = 0; i < _numaCount; i++)
      if ((statePtr >= _internalBuffersStart[i]) && (statePtr <= _internalBuffersEnd[i]))
        return i;

    // Check for error
    JAFFAR_THROW_RUNTIME("Did not find the corresponding numa domain for the provided state pointer. This must be a bug in Jaffar\n");
  }

  inline void returnFreeState(void *const statePtr) override
  {
    // Finding out to which database this state pointer belongs to
    const auto numaIdx = getStateNumaDomain(statePtr);

    // Pushing the state in the corresponding queue
    bool success = _freeStateQueues[numaIdx]->try_push(statePtr);

    // Check for success
    if (success == false) JAFFAR_THROW_RUNTIME("Failed on pushing free state back. This must be a bug in Jaffar\n");
  }

  inline void *popState() override
  {
    // Pointer to return
    void *statePtr;

    // Check the numa's scavenger queue first
    bool success = _scavengerQueues[preferredNumaDomain]->pop_front_get(statePtr);
    if (success == true) return statePtr;

    // Starting scavenging process
    for (size_t i = 0; i < _scavengingDepth; i++)
    {
      // Trying to pop the next state from the current state database
      success = _currentStateDb.pop_front_get(statePtr);

      // If no success, break cycle
      if (success == false) break;

      // Otherwise, get numa domain of state
      const auto numaIdx = getStateNumaDomain(statePtr);

      // If its my preferred numa, return it immediately
      if (numaIdx == preferredNumaDomain) return statePtr;

      // Otherwise, place it in the corresponding scavenge queue
      _scavengerQueues[numaIdx]->push_front(statePtr);
    }

    // If still no success, check the other scavenger queues
    for (int i = 0; i < _numaCount; i++)
      if (i != preferredNumaDomain)
      {
        bool success = _scavengerQueues[i]->pop_front_get(statePtr);
        if (success == true) return statePtr;
      }

    // If no success at all, just return a nullptr
    return nullptr;
  }

  /**
   * Gets the current number of states in the current state database
   */
  inline size_t getStateCount() const override
  {
    return _currentStateDb.wasSize();
  }

  private:
  /**
   * Number of numa domains
   */
  int _numaCount;

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
   * Scavenger queues allow the thread to search for a state that belongs to it through the current state database
   */
  std::vector<std::unique_ptr<jaffarCommon::concurrent::Deque<void *>>> _scavengerQueues;

  /**
   * Size of the scavenger queues
   */
  size_t _scavengerQueuesSize;

  /**
   * How far should each thread scavenge for a state belonging to its numa domain
   */
  size_t _scavengingDepth;

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
