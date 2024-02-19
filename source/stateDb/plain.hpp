#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/serializers/contiguous.hpp>
#include <jaffarCommon/include/serializers/differential.hpp>
#include <jaffarCommon/include/deserializers/contiguous.hpp>
#include <jaffarCommon/include/deserializers/differential.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/concurrent.hpp>
#include <jaffarCommon/include/timing.hpp>
#include "../runner.hpp"
#include "base.hpp"

#define _JAFFAR_STATE_PADDING_BYTES 64

namespace jaffarPlus
{

namespace stateDb
{

class Plain : public stateDb::Base
{
  public:

  Plain(Runner& r, const nlohmann::json& config) : stateDb::Base(r, config)
  {
    // Getting system's page size (typically 4K but it may change in the future)
    const size_t pageSize = sysconf(_SC_PAGESIZE);

    // Creating free state queue
    _freeStateQueue = std::make_unique<jaffarCommon::atomicQueue_t<void*>>(_maxStates);

    // Allocating space for the states
    auto status = posix_memalign((void**)&_internalBuffer, pageSize, _maxSize);
    if (status != 0) EXIT_WITH_ERROR("Could not allocate aligned memory for the state database");
    
    // Doing first touch for every page 
    #pragma omp parallel for 
    for (size_t i = 0; i < _maxSize; i += pageSize) _internalBuffer[i] = 1;

    // Adding the state pointers to the free state queue
    for (size_t i = 0; i < _maxStates; i++) 
     _freeStateQueue->try_push((void*) &_internalBuffer[i * _stateSize]);
  }

  ~Plain() = default;

  // Function to print relevant information
  void printInfoImpl() const override
  {
   LOG("[J+]  + Total States:                  %lu\n", _currentStateDb.wasSize() + _nextStateDb.size() + _freeStateQueue->was_size());
  }

  inline void* getFreeState() override
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

  inline void returnFreeState(void* const statePtr) override
  {
    // Trying to get free space for a new state
    bool success = _freeStateQueue->try_push(statePtr);

    // If successful, return the pointer immediately
    if (success == false) EXIT_WITH_ERROR("Failed on pushing free state back. This must be a bug in Jaffar\n");
  }

  inline void pushStateImpl(const float reward, void* statePtr) override
  {
    // Inserting state into the database
    _nextStateDb.insert({reward, statePtr});
  }

  inline void* popState() override
  {
    // Pointer to return 
    void* statePtr;
    
    // Trying to pop the next state from the current state database
    const auto success = _currentStateDb.pop_front_get(statePtr);

    // If not successful, return a null pointer
    if (success == false) return nullptr;

    return statePtr;
  }

  inline void* getBestState() const override
  {
    return _currentStateDb.front();
  }

  inline void* getWorstState() const override
  {
    return _currentStateDb.back();
  }

  /**
   * Copies the pointers from the next state database into the current one, starting with the largest rewards, and clears it.
  */
  inline void advanceStep() override
  {
    // Copying state pointers
    for (auto& statePtr : _nextStateDb) _currentStateDb.push_back_no_lock(statePtr.second);

    // Clearing next state db
    _nextStateDb.clear();
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
   * Internal buffer for the state database
   */ 
  uint8_t* _internalBuffer;
};

} // namespace stateDb

} // namespace jaffarPlus
