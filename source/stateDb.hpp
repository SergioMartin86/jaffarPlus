#pragma once

#include <stdlib.h>
#include <numa.h>
#include <unistd.h>
#include <memory>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/concurrent.hpp>
#include <jaffarCommon/include/timing.hpp>
#include "runner.hpp"

#define _JAFFAR_STATE_PADDING_BYTES 64

namespace jaffarPlus
{

class StateDb
{
  public:

  StateDb(const size_t stateSizeRaw, const nlohmann::json& config) :
  _stateSizeRaw (stateSizeRaw)
  {
    // Getting system's page size
    size_t pageSize = sysconf(_SC_PAGESIZE);

    // We want to align each state to 512 bits (64 bytes) to favor vectorized access
    // Now calculating the necessary padding to reach the next multiple of 64 bytes
    _stateSize = ((_stateSizeRaw / _JAFFAR_STATE_PADDING_BYTES) + 1) * _JAFFAR_STATE_PADDING_BYTES;

    // Padding is the difference between the aligned state size and the raw one
    _stateSizePadding = _stateSize - _stateSizeRaw;

    // Getting maximum size in Mb
    _maxSizeMb = JSON_GET_NUMBER(size_t, config, "Max Size (Mb)");

    // Converting it to pure bytes
    _maxSize = _maxSizeMb * 1024ul * 1024ul;

    // Getting maximum number of states
    _maxStates = _maxSize / _stateSize;

    // Creating free state queue
    _freeStateQueue = std::make_unique<jaffarCommon::atomicQueue_t<void*>>(_maxStates);

    // Allocating space for the states
    auto status = posix_memalign((void**)&_internalBuffer, pageSize, _maxSize);
    if (status != 0) EXIT_WITH_ERROR("Could not allocate aligned memory for the state database");
    
    // Timing
    auto t0 = jaffarCommon::now(); 

    // Doing first touch every page (pages are 4K)
    #pragma omp parallel for 
    for (size_t i = 0; i < _maxSize; i += pageSize) _internalBuffer[i] = 1;

    // Adding the state pointers to the free state queue
    for (size_t i = 0; i < _maxStates; i++) 
     _freeStateQueue->try_push((void*) &_internalBuffer[i * _stateSize]);

    // Timing
    _initializationTime = jaffarCommon::timeDeltaSeconds(jaffarCommon::now(), t0); 

    LOG("[J+] Free State Queue Size:            %d\n", _freeStateQueue->was_size());
    LOG("[J+] State Db Initialization Time:     %f\n", _initializationTime);
  }

  // Function to print relevant information
  void printInfo() const
  {
   LOG("[J+]  + State Size:                    %lu (%lu + %lu Padding to %u)\n", _stateSize, _stateSizeRaw, _stateSizePadding, _JAFFAR_STATE_PADDING_BYTES);
   LOG("[J+]  + Max States:                    %lu\n", _maxStates);
   LOG("[J+]  + Max Size (Bytes):              %lu bytes (%.3f Mb, %.6f Gb)\n", _maxSize, (double)_maxSize / (1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0 * 1024.0));
  }

  inline void* getFreeState()
  {
    // Storage for the new free state space
    void* stateSpace;

    // Trying to get free space for a new state
    bool success = _freeStateQueue->try_pop(stateSpace);

    // If successful, return the pointer immediately
    if (success == true) return stateSpace;
    
    // If failed, then try to get it from the back of the current state database
    success = _currentStateDb.pop_back_get(stateSpace);

    // If successful, return the pointer immediately 
    if (success == true) return stateSpace;

    // Otherwise, return a null pointer. The state will be discarded
    return nullptr;
  }

  inline void returnFreeState(void* const statePtr)
  {
    // Trying to get free space for a new state
    bool success = _freeStateQueue->try_push(statePtr);

    // If successful, return the pointer immediately
    if (success == false) EXIT_WITH_ERROR("Failed on pushing free state back. This must be a bug in Jaffar\n");
  }

  private:

  /**
   * The current state database used as read-only source of base states
  */
  jaffarCommon::concurrentDeque<void*> _currentStateDb;

  /**
   * The next state database, where new states are stored as they are created
  */
  jaffarCommon::concurrentMultimap_t<float, void*> _nextStateDb;

  /**
   * This queue will hold pointers to all the free state storage
  */
  std::unique_ptr<jaffarCommon::atomicQueue_t<void*>> _freeStateQueue;

  /**
   *  The dropoff rate represents the percentage of states to be discarded at once, when the database runs out of states
  */
  double _dropoffRate;

  /**
   * Stores the size occupied by each state (with padding)
  */
  size_t _stateSize;

  /**
   * Stores the size occupied by each state (raw, without padding)
  */
  const size_t _stateSizeRaw;

  /**
   * Stores the size occupied by each state (raw, without padding)
  */
  size_t _stateSizePadding;

  /**
   * Internal buffer for the state database
   */ 
  uint8_t* _internalBuffer;

  /**
   * Maximum size (Mb) for the state database to grow to
  */
  double _maxSizeMb;

  /**
   * Maximum size (bytes) for the state database to grow
  */
  size_t _maxSize;

  /**
   * Number of maximum states in execution
  */
  size_t _maxStates;

  //////////// Timing variables

  double _initializationTime;
};

} // namespace jaffarPlus
