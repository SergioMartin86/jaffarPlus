#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/concurrent.hpp>
#include "base.hpp"

namespace jaffarPlus
{

namespace stateDb
{

class Plain : public stateDb::Base
{
  public:

  Plain(Runner& r, const nlohmann::json& config) : stateDb::Base(r, config)
  {
    // Converting it to pure bytes
    _maxSize = _maxSizeMb * 1024ul * 1024ul;

    // Getting maximum number of states
    _maxStates = _maxSize / _stateSize;

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

  /**
   * Gets the current number of states in the current state database
  */
  inline size_t getStateCount() const override
  {
    return _currentStateDb.wasSize();
  }

  private:

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
