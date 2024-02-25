#pragma once

#include <jaffarCommon/include/logger.hpp>
#include <jaffarCommon/include/serializers/contiguous.hpp>
#include <jaffarCommon/include/serializers/differential.hpp>
#include <jaffarCommon/include/deserializers/contiguous.hpp>
#include <jaffarCommon/include/deserializers/differential.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/concurrent.hpp>
#include "../runner.hpp"

#define _JAFFAR_STATE_PADDING_BYTES 64

namespace jaffarPlus
{

namespace stateDb 
{

class Base
{
  public:

  Base(Runner& r, const nlohmann::json& config)
  {
    ///////// Parsing configuration

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

    // We want to align each state to 512 bits (64 bytes) to favor vectorized access
    // Now calculating the necessary padding to reach the next multiple of 64 bytes
    _stateSize = _stateSizeEffective;
    if (_stateSize % _JAFFAR_STATE_PADDING_BYTES != 0)
     _stateSize = ((_stateSizeEffective / _JAFFAR_STATE_PADDING_BYTES) + 1) * _JAFFAR_STATE_PADDING_BYTES;

    // Padding is the difference between the aligned state size and the raw one
    _stateSizePadding = _stateSize - _stateSizeEffective;
  }

  virtual ~Base()
  {
    // Freeing up reference data buffer
    free(_referenceData);
  }

  // Function to print relevant information
  void printInfo() const
  {
   const size_t currentStateCount = getStateCount();
   const size_t currentStateBytes = currentStateCount * _stateSize;

   LOG("[J++]  + Current State Count:           %lu (%f Mstates) /  %lu (%f Mstates) Max / %5.2f%% Full\n", currentStateCount, (double)currentStateCount * 1.0e-6, _maxStates, (double)_maxStates * 1.0e-6, 100.0 * (double)currentStateCount / (double)_maxStates);
   LOG("[J++]  + Current State Size:            %.3f Mb (%.6f Gb) / %.3f Mb (%.6f Gb) Max\n", (double)currentStateBytes / (1024.0 * 1024.0), (double)currentStateBytes / (1024.0 * 1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0 * 1024.0));
   LOG("[J++]  + State Size Raw:                %lu bytes\n", _stateSizeRaw);
   LOG("[J++]  + State Size Effective:          %lu bytes\n", _stateSizeEffective);
   LOG("[J++]  + State Size in DB:              %lu bytes (%lu padding bytes to %u)\n", _stateSize, _stateSizePadding, _JAFFAR_STATE_PADDING_BYTES);
   LOG("[J++]  + Use Differential Compression:  %s\n", _useDifferentialCompression ? "true" : "false");
   LOG("[J++]  + Use Zlib Compression:          %s\n", _useZlibCompression ? "true" : "false");

   printInfoImpl();
  }

  virtual void* getFreeState() = 0;
  virtual void returnFreeState(void* const statePtr) = 0;
  virtual void* popState() = 0;
  virtual void advanceStep() = 0;
  virtual size_t getStateCount() const = 0;

  inline void pushState(const float reward, Runner& r, void* statePtr)
  {
    // Check that we got a free state (we did not overflow state memory)
    if (statePtr == nullptr) EXIT_WITH_ERROR("Ran out of free states\n");

    // Serializing the runner state into the memory received (if using differential compression)
    if (_useDifferentialCompression == true)
    {
      jaffarCommon::serializer::Differential s(statePtr, _differentialStateSize, _referenceData, _stateSizeRaw, _useZlibCompression);
      r.serializeState(s);
    }

    // Serializing the runner state into the memory received (if no compression is used)
    if (_useDifferentialCompression == false)
    {
      jaffarCommon::serializer::Contiguous s(statePtr, _stateSizeRaw);
      r.serializeState(s);
    }

    // Calling particular implementation of the push state function
    pushStateImpl(reward, statePtr);
  }

  /**
   * Loads the state into the runner, performing the appropriate decompression (or not) procedure
  */
  inline void loadStateIntoRunner(Runner& r, const void* statePtr)
  {
    // Serializing the runner state into the memory received (if using differential compression)
    if (_useDifferentialCompression == true)
    {
      jaffarCommon::deserializer::Differential d(statePtr, _differentialStateSize, _referenceData, _stateSizeRaw, _useZlibCompression);
      r.deserializeState(d);
    }

    // Serializing the runner state into the memory received (if no compression is used)
    if (_useDifferentialCompression == false)
    {
      jaffarCommon::deserializer::Contiguous d(statePtr, _stateSizeRaw);
      r.deserializeState(d);
    }
  }

  /**
   * This function returns a pointer to the best state found in the current state database
  */
  inline void* getBestState() const
  {
    return _currentStateDb.front();
  }

  /**
   * This function returns a pointer to the worst state found in the current state database
  */
  inline void* getWorstState() const
  {
    return _currentStateDb.back();
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

  protected:

  virtual void pushStateImpl(const float reward, void* statePtr) = 0;
  virtual void printInfoImpl() const = 0;
  
  /**
   * The current state database used as read-only source of base states
  */
  jaffarCommon::concurrentDeque<void*> _currentStateDb;

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
};

} // namespace stateDb

} // namespace jaffarPlus
