#pragma once

#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/deserializers/differential.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/serializers/differential.hpp>
#include "../runner.hpp"

#define _JAFFAR_STATE_PADDING_BYTES 64

namespace jaffarPlus
{

namespace stateDb
{

class Base
{
  public:

  Base(Runner &r, const nlohmann::json &config)
    : _runner(&r)
  {
    ///////// Parsing configuration

    // Parsing state compression configuration
    const auto &stateCompressionJs  = jaffarCommon::json::getObject(config, "Compression");
    _useDifferentialCompression     = jaffarCommon::json::getBoolean(stateCompressionJs, "Use Differential Compression");
    _maximumDifferentialSizeAllowed = jaffarCommon::json::getNumber<size_t>(stateCompressionJs, "Max Difference (bytes)");
    _useZlibCompression             = jaffarCommon::json::getBoolean(stateCompressionJs, "Use Zlib Compression");
  }

  void initialize()
  {
    // Initialization message
    jaffarCommon::logger::log("[J+] Initializing State Database...\n");

    ///////// Getting original state sizes from the runner

    // Getting game state size
    _stateSizeRaw = _runner->getStateSize();

    // Creating storage for current reference data
    _currentReferenceData = malloc(_stateSizeRaw);

    // Creating storage for  previous reference data
    _previousReferenceData = malloc(_stateSizeRaw);

    // Getting differential state size
    if (_useDifferentialCompression) _differentialStateSize = _runner->getDifferentialStateSize(_maximumDifferentialSizeAllowed);

    // The effective state size is how many bytes does the runner really need to store a state
    _stateSizeEffective = _useDifferentialCompression ? _differentialStateSize : _stateSizeRaw;

    // We want to align each state to 512 bits (64 bytes) to favor vectorized access
    // Now calculating the necessary padding to reach the next multiple of 64 bytes
    _stateSize = _stateSizeEffective;
    if (_stateSize % _JAFFAR_STATE_PADDING_BYTES != 0) _stateSize = ((_stateSizeEffective / _JAFFAR_STATE_PADDING_BYTES) + 1) * _JAFFAR_STATE_PADDING_BYTES;

    // Padding is the difference between the aligned state size and the raw one
    _stateSizePadding = _stateSize - _stateSizeEffective;

    // Setting initial value for the maximum differences found so far
    _maximumStateSizeFound = 0;

    // Calling specific initialization routine for the state db type
    initializeImpl();
  }

  // Function to print relevant information
  void printInfo() const
  {
    const size_t currentStateCount = getStateCount();
    const size_t currentStateBytes = currentStateCount * _stateSize;

    jaffarCommon::logger::log("[J+]  + Current State Count:           %lu (%f Mstates) /  %lu (%f Mstates) Max / %5.2f%% Full\n",
                              currentStateCount,
                              (double)currentStateCount * 1.0e-6,
                              _maxStates,
                              (double)_maxStates * 1.0e-6,
                              100.0 * (double)currentStateCount / (double)_maxStates);
    jaffarCommon::logger::log("[J+]  + Current State Size:            %.3f Mb (%.6f Gb) / %.3f Mb (%.6f Gb) Max\n",
                              (double)currentStateBytes / (1024.0 * 1024.0),
                              (double)currentStateBytes / (1024.0 * 1024.0 * 1024.0),
                              (double)_maxSize / (1024.0 * 1024.0),
                              (double)_maxSize / (1024.0 * 1024.0 * 1024.0));
    jaffarCommon::logger::log("[J+]  + State Size Raw:                %lu bytes\n", _stateSizeRaw);
    if (_useDifferentialCompression)
    {
      jaffarCommon::logger::log("[J+]  + State Size Effective:          %lu bytes (Diffential: %lu + Contiguous: %lu)\n",
                                _stateSizeEffective,
                                _maximumDifferentialSizeAllowed,
                                _stateSizeEffective - _maximumDifferentialSizeAllowed);
    } else
    {
      jaffarCommon::logger::log("[J+]  + State Size Effective:          %lu bytes\n", _stateSizeEffective);
    }

    jaffarCommon::logger::log("[J+]  + State Size in DB:              %lu bytes (%lu padding bytes to %u)\n", _stateSize, _stateSizePadding, _JAFFAR_STATE_PADDING_BYTES);
    jaffarCommon::logger::log("[J+]  + Use Differential Compression:  %s\n", _useDifferentialCompression ? "true" : "false");
    if (_useDifferentialCompression)
    {
      jaffarCommon::logger::log("[J+]  + Use Zlib Compression:          %s\n", _useZlibCompression ? "true" : "false");
      jaffarCommon::logger::log("[J+]  + Maximum State Size Found       %lu bytes / Max Allowed: %lu bytes\n", _maximumStateSizeFound, _differentialStateSize);
    }
    printInfoImpl();
  }

  virtual void   initializeImpl()                      = 0;
  virtual void  *getFreeState()                        = 0;
  virtual void   returnFreeState(void *const statePtr) = 0;
  virtual void  *popState()                            = 0;
  virtual size_t getStateCount() const                 = 0;

  /**
   * This function sets the initial reference data required for differential compression
   *
   * It must be of the same size as _stateSizeRaw
   */
  __INLINE__ void setReferenceData(const void *referenceData)
  {
    memcpy(_currentReferenceData, referenceData, _stateSizeRaw);
    memcpy(_previousReferenceData, referenceData, _stateSizeRaw);
  }

  /**
   * Copies the pointers from the next state database into the current one, starting with the largest rewards, and clears it.
   */
  __INLINE__ void advanceStep()
  {
    // Copying state pointers
    while (_nextStateDb.begin() != _nextStateDb.end()) _currentStateDb.push_back_no_lock(_nextStateDb.unsafe_extract(_nextStateDb.begin()).mapped());

    // Swapping the reference data pointers
    std::swap(_currentReferenceData, _previousReferenceData);

    // The new refernce data will be the best current state
    if (_currentStateDb.wasSize() > 0) memcpy(_currentReferenceData, _currentStateDb.front(), _stateSizeRaw);
  }

  __INLINE__ bool pushState(const float reward, Runner &r, void *statePtr)
  {
    // Check that we got a free state (we did not overflow state memory)
    if (statePtr == nullptr) JAFFAR_THROW_RUNTIME("Ran out of free states\n");

    // Encoding internal runner state into the state pointer
    size_t stateSize = 0;
    try
    {
      stateSize = saveStateFromRunner(r, statePtr);
    }
    catch (const std::runtime_error &x)
    {
      // If failed return false
      return false;
    }

    // If using differential compression, it is important to keep track of the current compression size
    _maximumStateSizeFound = std::max(_maximumStateSizeFound, stateSize);

    // Inserting new state into the next state database
    _nextStateDb.insert({reward, statePtr});

    // If succeeded, return true
    return true;
  }

  /**
   * Saves the runner state into the provided state data pointer
   */
  __INLINE__ size_t saveStateFromRunner(Runner &r, void *statePtr) const
  {
    // Storage for the state size after deserialization
    size_t serializedSize = 0;

    // Serializing the runner state into the memory received (if using differential compression)
    if (_useDifferentialCompression == true)
    {
      jaffarCommon::serializer::Differential s(statePtr, _differentialStateSize, _currentReferenceData, _stateSizeRaw, _useZlibCompression);
      r.serializeState(s);
      serializedSize = s.getOutputSize();
    }

    // Serializing the runner state into the memory received (if no compression is used)
    if (_useDifferentialCompression == false)
    {
      jaffarCommon::serializer::Contiguous s(statePtr, _stateSizeRaw);
      r.serializeState(s);
      serializedSize = s.getOutputSize();
    }

    return serializedSize;
  }

  /**
   * Loads the state into the runner, performing the appropriate decompression (or not) procedure
   */
  __INLINE__ void loadStateIntoRunner(Runner &r, const void *statePtr)
  {
    // Deserializing the runner state from the memory received (if using differential compression)
    if (_useDifferentialCompression == true)
    {
      jaffarCommon::deserializer::Differential d(statePtr, _differentialStateSize, _previousReferenceData, _stateSizeRaw, _useZlibCompression);
      r.deserializeState(d);
    }

    // Deserializing the runner state from the memory received (if no compression is used)
    if (_useDifferentialCompression == false)
    {
      jaffarCommon::deserializer::Contiguous d(statePtr, _stateSizeRaw);
      r.deserializeState(d);
    }
  }

  /**
   * This function returns a pointer to the best state found in the current state database
   */
  __INLINE__ void *getBestState() const { return _currentStateDb.front(); }

  /**
   * This function returns a pointer to the worst state found in the current state database
   */
  __INLINE__ void *getWorstState() const { return _currentStateDb.back(); }

  protected:

  virtual void printInfoImpl() const = 0;

  Runner *const _runner;

  /**
   * The next state database, where new states are stored as they are created
   */
  jaffarCommon::concurrent::concurrentMultimap_t<float, void *> _nextStateDb;

  /**
   * The current state database used as read-only source of base states
   */
  jaffarCommon::concurrent::Deque<void *> _currentStateDb;

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
  size_t _maximumDifferentialSizeAllowed;

  // if using differential compression, whether to use Zlib post-compression
  bool _useZlibCompression;

  // If using differential compression, store differential state size
  size_t _differentialStateSize = 0;

  // If using differential compression, the maximum differences found
  size_t _maximumStateSizeFound;

  // Storage for the current reference data required for differential compression serialization
  void *_currentReferenceData;

  // Storage for the previously used reference data required for differential compression deserialization
  void *_previousReferenceData;
};

} // namespace stateDb

} // namespace jaffarPlus
