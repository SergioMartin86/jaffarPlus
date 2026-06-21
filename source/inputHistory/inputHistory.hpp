#pragma once

/**
 * @file inputHistory.hpp
 * @brief Abstract interface for how a search remembers the sequence of inputs ("path") that produced
 *        each state. The rest of JaffarPlus only ever talks to this base class; concrete strategies
 *        (none / raw bit-packed / shared prefix trie) live in their own headers and are selected from
 *        the .jaffar config. See inputHistoryFactory.hpp for construction.
 *
 * Two groups of operations live on this one interface:
 *  - per-runner "cursor" ops (pushInput, (de)serialize, toString): each Runner owns an instance and
 *    advances it as it explores.
 *  - "manager" ops on stored cold-slot bytes (initColdSlot, releaseColdSlot, captureColdToFull, sizes):
 *    these only use the shared backing (e.g. the trie), so the StateDb calls them on a reference instance.
 *
 * Cold vs full: the StateDb stores a compact "cold" representation in each state slot; standalone
 * snapshot buffers (best/worst/win, player initial state) hold a self-contained "full" representation.
 * For raw/none the two coincide; for the trie the cold form is a 4-byte node id while the full form is a
 * reconstructed, self-contained copy.
 */

#include "../inputSet.hpp"
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <map>
#include <memory>
#include <string>

namespace jaffarPlus
{

class InputHistory
{
public:
  virtual ~InputHistory() = default;

  // ---- per-runner cursor ---------------------------------------------------------------------------

  /// @brief Resets the cursor to the empty path (step 0).
  virtual void reset() = 0;

  /// @brief Records one applied input, advancing the path and the step counter.
  virtual void pushInput(const InputSet::inputIndex_t input) = 0;

  /// @brief Number of inputs applied so far (the current step counter).
  virtual size_t getInputCount() const = 0;

  /// @brief Writes the compact "cold" path representation (stored in each StateDb slot).
  virtual void serializeCold(jaffarCommon::serializer::Base& s) const = 0;
  /// @brief Restores the cursor from a "cold" representation.
  virtual void deserializeCold(jaffarCommon::deserializer::Base& d) = 0;

  /// @brief Writes the self-contained "full" path representation (for standalone snapshot buffers).
  virtual void serializeFull(jaffarCommon::serializer::Base& s) const = 0;
  /// @brief Restores the cursor from a "full" representation.
  virtual void deserializeFull(jaffarCommon::deserializer::Base& d) = 0;

  /// @brief Reconstructs the path as a newline-separated string of input strings (the solution).
  virtual std::string toString(const std::map<InputSet::inputIndex_t, std::string>& inputStringMap) const = 0;

  // ---- manager ops on stored bytes (use the shared backing only) -----------------------------------

  /// @brief Size, in bytes, of the cold representation in a StateDb slot.
  virtual size_t getColdSize() const = 0;
  /// @brief Size, in bytes, of the full (self-contained) representation in a snapshot buffer.
  virtual size_t getFullSize() const = 0;

  /// @brief Prepares a fresh/recycled cold slot (e.g. marks it as holding no trie node). Default: no-op.
  virtual void initColdSlot(void* cold) const { (void)cold; }
  /// @brief Releases any shared resource a freed cold slot was holding (trie GC). @p shard is the
  /// freeing thread's id, used to recycle into a contention-free per-thread pool. Default: no-op.
  virtual void releaseColdSlot(void* cold, const size_t shard) const { (void)cold; (void)shard; }
  /// @brief Converts a stored cold representation into a self-contained full one (best/worst snapshot).
  virtual void captureColdToFull(const void* cold, void* full) const = 0;

  /// @brief Approximate resident memory of any shared structure (e.g. the trie), in bytes. Default: 0.
  virtual size_t getApproxMemoryBytes() const { return 0; }
};

} // namespace jaffarPlus
