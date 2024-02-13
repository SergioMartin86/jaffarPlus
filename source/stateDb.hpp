#pragma once

#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/concurrent.hpp>
#include "runner.hpp"

namespace jaffarPlus
{

class StateDb
{
  public:

  StateDb(const size_t stateSize, const nlohmann::json& config)
  {
    // Getting maximum size in Mb
    _maxSizeMb = JSON_GET_NUMBER(size_t, config, "Max Size (Mb)");

    // Converting it to pure bytes
    _maxSize = _maxSizeMb * 1024ul * 1024ul;

    // Getting maximum number of states
    _maxStates = _maxSize / stateSize;

    // Creating free state database
    _freeStateQueue = new jaffarCommon::atomicQueue_t<void*>(_maxStates);
  }

  ~StateDb()
  {
   // Freeing up space taken by the free state queue
   free(_freeStateQueue);
  }

  // Function to print relevant information
  void printInfo() const
  {
   LOG("[J+]  + Max Size (Bytes):            %lu\n", _maxSize);
   LOG("[J+]  + Max States:                  %lu\n", _maxStates);
  }

  private:

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

  /**
   * This queue will hold pointers to all the free state storage
  */
  jaffarCommon::atomicQueue_t<void*>* _freeStateQueue;
};

} // namespace jaffarPlus
