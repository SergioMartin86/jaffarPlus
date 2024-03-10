#pragma once

#include <cstdlib>
#include <memory>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/parallel.hpp>
#include "base.hpp"

namespace jaffarPlus
{

namespace stateDb
{

class Plain : public stateDb::Base
{
  public:

  Plain(Runner &r, const nlohmann::json &config) : stateDb::Base(r, config)
  {
    // Getting maximum state db size in Mb
    _maxSizeMb = jaffarCommon::json::getNumber<size_t>(config, "Max Size (Mb)");

    // For testing purposes, the maximum size can be overriden by environment variables
    if (auto *value = std::getenv("JAFFAR_ENGINE_OVERRIDE_MAX_STATEDB_SIZE_MB")) _maxSizeMb = std::stoul(value);
  }

  ~Plain() = default;

  void initializeImpl() override
  {
    // Converting it to pure bytes
    _maxSize = _maxSizeMb * 1024ul * 1024ul;

    // Getting maximum number of states
    _maxStates = _maxSize / _stateSize;

    // Creating free state queue
    _freeStateQueue = std::make_unique<jaffarCommon::concurrent::atomicQueue_t<void *>>(_maxStates);

    // Getting system's page size (typically 4K but it may change in the future)
    const size_t pageSize = sysconf(_SC_PAGESIZE);

    // Allocating space for the states
    auto status = posix_memalign((void **)&_internalBuffer, pageSize, _maxSize);
    if (status != 0) JAFFAR_THROW_RUNTIME("Could not allocate aligned memory for the state database");

    // Doing first touch for every page
    JAFFAR_PARALLEL_FOR
    for (size_t i = 0; i < _maxSize; i += pageSize) _internalBuffer[i] = 1;

    // Adding the state pointers to the free state queue
    for (size_t i = 0; i < _maxStates; i++)
      _freeStateQueue->try_push((void *)&_internalBuffer[i * _stateSize]);
  }

  // Function to print relevant information
  void printInfoImpl() const override
  {
  }

  inline void *getFreeState() override
  {
    // Storage for the new free state space
    void *stateSpace;

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

  inline void returnFreeState(void *const statePtr) override
  {
    // Trying to get free space for a new state
    bool success = _freeStateQueue->try_push(statePtr);

    // If successful, return the pointer immediately
    if (success == false) JAFFAR_THROW_RUNTIME("Failed on pushing free state back. This must be a bug in Jaffar\n");
  }

  inline void *popState() override
  {
    // Pointer to return
    void *statePtr;

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
  std::unique_ptr<jaffarCommon::concurrent::atomicQueue_t<void *>> _freeStateQueue;

  /**
   * Internal buffer for the state database
   */
  uint8_t *_internalBuffer;

  /**
   * Configured maximum size (Mb) for the state database to grow to
   */
  size_t _maxSizeMb;
};

} // namespace stateDb

} // namespace jaffarPlus
