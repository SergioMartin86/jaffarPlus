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

  StateDb(Runner& r, const nlohmann::json& config)
  {
    ///////// Parsing configuration

    // Getting maximum size in Mb
    _maxSizeMb = JSON_GET_NUMBER(size_t, config, "Max Size (Mb)");

    // Parsing state compression configuration
    const auto& stateCompressionJs = JSON_GET_OBJECT(config, "Compression");
    _useDifferentialCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Differential Compression");
    _maximumDifferences = JSON_GET_NUMBER(size_t, stateCompressionJs, "Max Difference (bytes)");
    _useZlibCompression = JSON_GET_BOOLEAN(stateCompressionJs, "Use Zlib Compression");

    ///////// Getting original state sizes from the runner
    
    // Getting game state size
    _stateSizeRaw = r.getStateSize();

    // Creating storage for reference data
    _referenceData = malloc (_stateSizeRaw);

    // Getting differential state size
    if (_useDifferentialCompression) _differentialStateSize = r.getDifferentialStateSize(_maximumDifferences);

    // The effective state size is how many bytes does the runner really need to store a state
    _stateSizeEffective = _useDifferentialCompression ? _differentialStateSize : _stateSizeRaw;

    // Getting system's page size
    const size_t pageSize = sysconf(_SC_PAGESIZE);

    // We want to align each state to 512 bits (64 bytes) to favor vectorized access
    // Now calculating the necessary padding to reach the next multiple of 64 bytes
    _stateSize = ((_stateSizeEffective / _JAFFAR_STATE_PADDING_BYTES) + 1) * _JAFFAR_STATE_PADDING_BYTES;

    // Padding is the difference between the aligned state size and the raw one
    _stateSizePadding = _stateSize - _stateSizeEffective;

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

  ~StateDb()
  {
    // Freeing up reference data buffer
    free(_referenceData);
  }

  // Function to print relevant information
  void printInfo() const
  {
   LOG("[J+]  + State Size Raw:                %lu bytes\n", _stateSizeRaw);
   LOG("[J+]  + State Size Effective:          %lu bytes\n", _stateSizeEffective);
   LOG("[J+]  + State Size in DB:              %lu bytes (%lu padding bytes to %u)\n", _stateSize, _stateSizePadding, _JAFFAR_STATE_PADDING_BYTES);
   LOG("[J+]  + Max State DB Entries:          %lu (%f Mstates)\n", _maxStates, (double)_maxStates * 1.0e-6);
   LOG("[J+]  + Max State DB Size:             %lu bytes (%.3f Mb, %.6f Gb)\n", _maxSize, (double)_maxSize / (1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0 * 1024.0));
   LOG("[J+]  + Use Differential Compression:  %s\n", _useDifferentialCompression ? "true" : "false");
   LOG("[J+]  + Use Zlib Compression:          %s\n", _useZlibCompression ? "true" : "false");
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

  inline void pushState(const float reward, Runner& r)
  {
    // Getting memory to serialize the state in
    auto statePtr = getFreeState();

    // Check that we got a free state (we did not overflow state memory)
    if (statePtr == nullptr) EXIT_WITH_ERROR("Ran out of free states\n");

    // Serializing the runner state into the memory received (if using differential compression)
    if (_useDifferentialCompression == true)
    {
      jaffarCommon::serializer::Differential s(statePtr, _differentialStateSize, _referenceData, _stateSizeRaw, _useZlibCompression);
      r.serializeState(s);
    }

    // Serializing the runner state into the memory received (if no compression is used)
    if (_useDifferentialCompression == true)
    {
      jaffarCommon::serializer::Contiguous s(statePtr, _stateSizeRaw);
      r.serializeState(s);
    }

    // Inserting state into the database
    _nextStateDb.insert({reward, statePtr});
  }

  /**
   * Copies the pointers from the next state database into the current one, starting with the largest rewards, and clears it.
  */
  inline void swapStateDatabases()
  {
    // Copying state pointers
    for (auto& statePtr : _nextStateDb) _currentStateDb.push_back(statePtr.second);

    // Clearing next state db
    _nextStateDb.clear();
  }

  /**
   * This function sets the reference data required for differential compression
   * 
   * It must be of the same size as _stateSizeRaw
  */
  inline void setReferenceData(const void* referenceData)
  {
   memcpy(_referenceData, referenceData, _stateSizeRaw);
  }

  /**
   * Gets the current number of states in the next state database
  */
  inline size_t getNextStateDbSize() const
  {
    return _nextStateDb.size();
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
   * Stores the size occupied by each state (with padding)
  */
  size_t _stateSize;

  /**
   * Stores the size occupied by each state without compression
  */
  size_t _stateSizeRaw;

  /**
   * Depending on whether we use compression, how many bytes does every runner state actually need
  */
  size_t _stateSizeEffective;

  /**
   * Stores the size actually occupied by each state, after adding padding
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

  //////////// Configuration

  // Stores whether to use differential compression
  bool _useDifferentialCompression;
  
  // If using differential compression, the maximum number of differences allowed
  size_t _maximumDifferences;

  // if using differential compression, whether to use Zlib post-compression
  bool _useZlibCompression;

  // If using differential compression, store differential state size
  size_t _differentialStateSize = 0;

  // Storage for reference data required for differential compression
  void* _referenceData;

  //////////// Timing variables

  double _initializationTime;
};

} // namespace jaffarPlus
