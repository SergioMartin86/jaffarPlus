#pragma once

#include "numa.hpp"
#include <atomic>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/parallel.hpp>

namespace jaffarPlus
{

class HashDb final
{
public:
  HashDb(const nlohmann::json& config)
  {
    _maxStoreCount  = jaffarCommon::json::getNumber<size_t>(config, "Max Store Count");
    _maxStoreSizeMb = jaffarCommon::json::getNumber<double>(config, "Max Store Size (Mb)");

    // For testing purposes, the maximum size can be overriden by environment variables
    if (auto* value = std::getenv("JAFFAR_ENGINE_OVERRIDE_MAX_HASHDB_SIZE_MB")) _maxStoreSizeMb = std::stoul(value);
  }

  ~HashDb()
  {
    // Deleting remaining numa stores
    for (size_t i = 0; i < _numaGroups.size(); i++) delete _numaGroups[i];
  }

  __INLINE__ void initialize()
  {
    // Converting the configured store budget to bytes. Rolling is driven by the set's actual
    // measured memory (see getStoreSizeBytes), not by an entries-to-bytes estimate.
    _maxStoreSizeBytes = (size_t)(_maxStoreSizeMb * 1024.0 * 1024.0);

    // Initializing hash store vector
    for (ssize_t i = 0; i < _numaGroupCount; i++) _numaGroups.push_back(new NUMAStore_t);

    // Creating first hash db store, and per-thread statistics counters
    for (size_t i = 0; i < _numaGroups.size(); i++)
    {
      _numaGroups[i]->_hashStores.push_back(hashStore_t({.id = _numaGroups[i]->_currentHashStoreId++, .age = _numaGroups[i]->_currentAge, .queryCount = 0, .collisionCount = 0}));
      _numaGroups[i]->_threadQueryCount.resize(_threadCount);
      _numaGroups[i]->_threadCollisionCount.resize(_threadCount);
    }
  }

  // Function to print relevant information
  void printInfo() const
  {
    // jaffarCommon::logger::log("[J+]  + Max Store Count:               %lu\n", _maxStoreCount);
    // jaffarCommon::logger::log("[J+]  + Max Store Size:                %f Mb (%.2f Gb)\n", _maxStoreSizeMb, _maxStoreSizeMb / 1024.0);
    // jaffarCommon::logger::log("[J+]  + Max Store Entries:             %lu (%.2f Mentries)\n", _maxStoreEntries, (double)_maxStoreEntries / (1024.0 * 1024.0));
    // jaffarCommon::logger::log("[J+]  + Total Max Entries:             %lu (%.2f Mentries)\n", _maxStoreEntries * _maxStoreCount,
    // ((double)_maxStoreEntries * _maxStoreCount) / (1024.0 * 1024.0));

    // Printing hash store information
    for (size_t i = 0; i < _numaGroups.size(); i++)
    {
      auto   itr             = _numaGroups[i]->_hashStores.rbegin();
      size_t curHashStoreIdx = 0;
      while (itr != _numaGroups[i]->_hashStores.rend())
      {
        const size_t queryCount     = itr == _numaGroups[i]->_hashStores.rbegin() ? _numaGroups[i]->getQueryCount() : itr->queryCount;
        const size_t collisionCount = itr == _numaGroups[i]->_hashStores.rbegin() ? _numaGroups[i]->getCollisionCount() : itr->collisionCount;

        jaffarCommon::logger::log("[J+]      + NUMA %lu - Store: %lu / %lu - Id: %lu - Age: %lu, Entries: %.3f M, Size: %.3f Mb / %.3f Mb, Check Count: %lu, Collision "
                                  "Count: %lu (Rate %.3f%%)\n",
                                  i, curHashStoreIdx + 1, _maxStoreCount, itr->id, itr->age, (double)itr->hashSet.size() / (1024.0 * 1024.0),
                                  (double)getStoreSizeBytes(*itr) / (1024.0 * 1024.0), _maxStoreSizeMb, queryCount, collisionCount,
                                  queryCount == 0 ? 0.0 : 100.0 * (double)collisionCount / (double)queryCount);
        itr++;
        curHashStoreIdx++;
      }
    }
  }

  /**
   * Function to check whether the provided hash is already present in any of the hash stores
   */
  __INLINE__ bool checkHashExists(const jaffarCommon::hash::hash_t hash)
  {
    auto* const numaGroup = _numaGroups[_preferredNumaGroup];
    const auto  threadId  = jaffarCommon::parallel::getThreadId();

    // Counting one query per state checked (into this thread's own slot, contention-free)
    numaGroup->_threadQueryCount[threadId].value++;

    // The current hash store is the latest to be entered
    auto   itr             = numaGroup->_hashStores.rbegin();
    size_t curHashStoreIdx = 0;

    // Checking for the rest of the hash stores in reverse order, to increase chances of early collision detection
    while (itr != numaGroup->_hashStores.rend())
    {
      // Flag to indicate whether a collision has been found
      bool collisionFound = false;

      // If it is the first hash db, check at the same time as we insert
      if (curHashStoreIdx == 0) collisionFound = itr->hashSet.insert(hash).second == false;

      // Otherwise, we simply check (no inserts)
      if (curHashStoreIdx > 0) collisionFound = itr->hashSet.contains(hash);

      // If collision is found, register it and return
      if (collisionFound == true)
      {
        // Increasing counter for collisions
        numaGroup->_threadCollisionCount[threadId].value++;

        // True means a collision was found
        return true;
      }

      // Increasing indexing
      itr++;
      curHashStoreIdx++;
    }

    // If no hits, then it's not collided
    return false;
  }

  /**
   * This function simply inserts a hash without checking for collisions
   */
  __INLINE__ void insertHash(const jaffarCommon::hash::hash_t hash)
  {
    // The current hash store is the latest to be entered
    auto  itr              = _numaGroups[_preferredNumaGroup]->_hashStores.rbegin();
    auto& currentHashStore = *itr;

    // Inserting hash
    currentHashStore.hashSet.insert(hash);
  }

  /**
   * This function serves to indicate a new step has started.
   * The current age is increased, and if the current database exceeds its maximum
   * size, it is sent to the past db collection and a new one is created
   */
  __INLINE__ void advanceStep()
  {
    // For each NUMA store, advance the step of its current hash store
    for (size_t i = 0; i < _numaGroups.size(); i++)
    {
      // The current hash store is the latest to be entered
      auto& currentHashStore = *_numaGroups[i]->_hashStores.rbegin();

      // If the current hash store's measured memory exceeds the budget, push a new one in
      if (getStoreSizeBytes(currentHashStore) >= _maxStoreSizeBytes)
      {
        // First, if we already reached the maximum hash stores, then discard the oldest one first
        if (_numaGroups[i]->_hashStores.size() == _maxStoreCount) _numaGroups[i]->_hashStores.pop_front();

        // Snapshotting the (summed per-thread) counters into the store being retired, then resetting
        currentHashStore.queryCount     = _numaGroups[i]->getQueryCount();
        currentHashStore.collisionCount = _numaGroups[i]->getCollisionCount();
        _numaGroups[i]->resetCounts();

        // Now create the new one, by pushing it from the back
        _numaGroups[i]->_hashStores.push_back(
            hashStore_t({.id = _numaGroups[i]->_currentHashStoreId++, .age = _numaGroups[_preferredNumaGroup]->_currentAge, .queryCount = 0, .collisionCount = 0}));
      }

      // Increasing age
      _numaGroups[i]->_currentAge++;
    }
  }

private:
  /**
   * A hash store represents a hash set, containing hashes of previously found states
   * It also contains an age, indicating how long ago it was created. The older
   * hash stores are discarded first.
   */
  struct hashStore_t
  {
    // The store id
    const size_t id;

    // The store age
    const size_t age;

    // How many queries were made
    size_t queryCount;

    // How many collisions were detected
    size_t collisionCount;

    // The internal set for the hash store
    jaffarCommon::concurrent::HashSet_t<jaffarCommon::hash::hash_t> hashSet = {};
  };

  /**
   * A query/collision counter padded to a full cache line. The hot-path hash check is the most
   * frequent operation in the engine, so these statistics are accumulated per-thread (each thread
   * touches only its own slot) instead of through a single shared atomic, whose cache line would
   * otherwise bounce between all worker cores on every check. The padding prevents two threads'
   * slots from landing on the same cache line (false sharing).
   */
  struct alignas(64) paddedCounter_t
  {
    size_t value = 0;
  };

  struct NUMAStore_t
  {
    /**
     * Identifier count for hash db stores
     */
    size_t _currentHashStoreId = 0;

    /**
     * Age is a way to define how many steps have elapsed since the hash set was created.
     *
     * In other words, what is the last step to be considered for hash collisions
     */
    size_t _currentAge = 0;

    /**
     * The current hash store (latest entry) is R/W. That is, it can be used to check whether the hash collides
     * but in doing that it is also added into the store
     *
     * The past hash stores are read only. They are only used to check whether the hash collides
     * but are not updated in the process.
     */
    std::deque<hashStore_t> _hashStores;

    /**
     * Per-thread query counts for the current hash store, indexed by OpenMP thread id.
     */
    std::vector<paddedCounter_t> _threadQueryCount;

    /**
     * Per-thread collision counts for the current hash store, indexed by OpenMP thread id.
     */
    std::vector<paddedCounter_t> _threadCollisionCount;

    // Aggregating helpers (called only at reporting / store-rollover time, never in the hot path)
    __INLINE__ size_t getQueryCount() const
    {
      size_t total = 0;
      for (const auto& c : _threadQueryCount) total += c.value;
      return total;
    }
    __INLINE__ size_t getCollisionCount() const
    {
      size_t total = 0;
      for (const auto& c : _threadCollisionCount) total += c.value;
      return total;
    }
    __INLINE__ void resetCounts()
    {
      for (auto& c : _threadQueryCount) c.value = 0;
      for (auto& c : _threadCollisionCount) c.value = 0;
    }
  };

  /**
   * Computes the actual resident memory of a hash store from the underlying set's measured
   * allocated capacity (number of slots), rather than from an entries-to-bytes estimate. This
   * tracks phmap's real footprint -- which varies with load factor and per-submap power-of-two
   * rounding -- so the configured store budget is respected instead of being over- or under-shot.
   */
  __INLINE__ size_t getStoreSizeBytes(const hashStore_t& store) const { return store.hashSet.capacity() * _bytesPerSlot + _hashStoreFixedOverhead; }

  /**
   * Number of NUMA domains per group
   */
  size_t _numaDomainsPerGroup;

  /**
   * Per-slot memory cost of the underlying phmap flat hash set: each element is stored inline
   * (one slot = sizeof(value)) plus one control byte per slot. Capacity is measured directly from
   * the set, so this no longer needs to absorb load-factor or power-of-two-rounding variance.
   */
  static constexpr size_t _bytesPerSlot = sizeof(jaffarCommon::hash::hash_t) + sizeof(uint8_t);

  /**
   * Fixed overhead of a hash store: the parallel set holds 256 (2^N) submaps, each with its own
   * table bookkeeping and mutex, allocated even when empty. Independent of the number of entries.
   */
  static constexpr size_t _hashStoreFixedOverhead = sizeof(jaffarCommon::concurrent::HashSet_t<jaffarCommon::hash::hash_t>);

  /**
   * Maximum number of stores to keep at any time
   */
  size_t _maxStoreCount;

  /**
   * Maximum store size (in megabytes)
   */
  double _maxStoreSizeMb;

  /**
   * Maximum store size, in bytes (derived from _maxStoreSizeMb)
   */
  size_t _maxStoreSizeBytes;

  /**
   * Set of hash databases stores, one per numa domain
   */
  std::vector<NUMAStore_t*> _numaGroups;
};

} // namespace jaffarPlus
