#pragma once

#include "../runner.hpp"
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/deserializers/differential.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/serializers/differential.hpp>

#define _JAFFAR_STATE_PADDING_BYTES 64

namespace jaffarPlus
{

namespace stateDb
{

class Base
{
public:
  Base(Runner& r, const nlohmann::json& config) : _runner(&r) {}

  virtual ~Base() = default;

  void initialize()
  {
    // Initialization message
    jaffarCommon::logger::log("[J+] Initializing State Database...\n");

    ///////// Getting original state sizes from the runner

    // Getting game state size
    _stateSizeRaw = _runner->getStateSize();

    // We want to align each state to 512 bits (64 bytes) to favor vectorized access
    // Now calculating the necessary padding to reach the next multiple of 64 bytes
    _stateSize = _stateSizeRaw;
    if (_stateSize % _JAFFAR_STATE_PADDING_BYTES != 0) _stateSize = ((_stateSizeRaw / _JAFFAR_STATE_PADDING_BYTES) + 1) * _JAFFAR_STATE_PADDING_BYTES;

    // Padding is the difference between the aligned state size and the raw one
    _stateSizePadding = _stateSize - _stateSizeRaw;

    // Calling specific initialization routine for the state db type
    initializeImpl();
  }

  // Function to print relevant information
  void printInfo() const
  {
    const size_t currentStateCount = getStateCount();
    const size_t currentStateBytes = currentStateCount * _stateSize;

    jaffarCommon::logger::log("[J+]  + Current State Count:           %lu (%f Mstates) /  %lu (%f Mstates) Max / %5.2f%% Full\n", currentStateCount,
                              (double)currentStateCount * 1.0e-6, _maxStates, (double)_maxStates * 1.0e-6, 100.0 * (double)currentStateCount / (double)_maxStates);
    jaffarCommon::logger::log("[J+]  + Current State Size:            %.3f Mb (%.6f Gb) / %.3f Mb (%.6f Gb) Max\n", (double)currentStateBytes / (1024.0 * 1024.0),
                              (double)currentStateBytes / (1024.0 * 1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0), (double)_maxSize / (1024.0 * 1024.0 * 1024.0));
    jaffarCommon::logger::log("[J+]  + State Size Raw:                %lu bytes\n", _stateSizeRaw);
    jaffarCommon::logger::log("[J+]  + State Size in DB:              %lu bytes (%lu padding bytes to %u)\n", _stateSize, _stateSizePadding, _JAFFAR_STATE_PADDING_BYTES);
    printInfoImpl();
  }

  virtual void   initializeImpl()                      = 0;
  virtual void*  getFreeState()                        = 0;
  virtual void   returnFreeState(void* const statePtr) = 0;
  virtual void*  popState()                            = 0;
  virtual size_t getStateCount() const                 = 0;

  /**
   * Copies the pointers from the next state database into the current one, starting with the largest rewards, and clears it.
   */
  virtual void advanceStep() = 0;

  /**
   * Saves the runner state into the provided state data pointer
   */
  __INLINE__ size_t saveStateFromRunner(Runner& r, void* statePtr) const
  {
    // Storage for the state size after deserialization
    size_t serializedSize = 0;

    // Serializing the runner state into the memory received
    jaffarCommon::serializer::Contiguous s(statePtr, _stateSizeRaw);
    r.serializeState(s);
    serializedSize = s.getOutputSize();

    return serializedSize;
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
      saveStateFromRunner(r, statePtr);
    }
    catch (const std::runtime_error& x)
    {
      // If failed return false
      return false;
    }

    // Inserting new state into the next state database
    return pushStateImpl(reward, statePtr);
  }

  /**
   * Loads the state into the runner
   */
  __INLINE__ void loadStateIntoRunner(Runner& r, const void* statePtr)
  {
    // Deserializing the runner state from the memory received
    jaffarCommon::deserializer::Contiguous d(statePtr, _stateSizeRaw);
    r.deserializeState(d);
  }

  /**
   * This function returns a pointer to the best state found in the current state database
   */
  virtual void* getBestState() const = 0;

  /**
   * This function returns a pointer to the worst state found in the current state database
   */
  virtual void* getWorstState() const = 0;

protected:
  /**
   * Inserts a new state into the state database
   */
  virtual bool pushStateImpl(const float reward, void* statePtr) = 0;

  virtual void printInfoImpl() const = 0;

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
};

} // namespace stateDb

} // namespace jaffarPlus
