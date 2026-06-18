#pragma once

#include "numa.hpp"
#include <atomic>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/parallel.hpp>

namespace jaffarPlus
{

/**
 * The hash database deduplicates states across the whole search. On a multi-NUMA host it uses a
 * two-tier layout: a NUMA-local L1 store per domain in front of a single global authoritative L2
 * store. A worker first probes its own domain's L1 (local, fast); only on an L1 miss does it consult
 * L2 (which may be remote / contended). Every globally-new hash is registered in L2 exactly once, so
 * dedup stays COMPLETE -- identical to a single global table -- while the frequent within-domain
 * repeats are served locally. On a single-NUMA host (one domain) the L1 tier is skipped and a single
 * local store is used, since everything is local anyway.
 *
 * This is automatic; there is no script-level configuration for it.
 */
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
    for (auto* s : _l1) delete s;
    if (_l2 != nullptr) delete _l2;
  }

  __INLINE__ void initialize()
  {
    // The configured "Max Store Size" is the ABSOLUTE per-generation budget for the whole hash DB:
    // the global L2 plus all per-domain L1 caches combined never exceed it (so the total footprint is
    // Max Store Size x Max Store Count, independent of NUMA layout -- the tiering does not inflate it).
    // Rolling is driven by each store's actual measured memory (see getStoreSizeBytes).
    const size_t totalBudgetBytes = (size_t)(_maxStoreSizeMb * 1024.0 * 1024.0);

    if (_numaCount > 1)
    {
      // Multi-domain: split the budget so L2 + all L1 == the configured budget. The global L2 gets 3/4
      // and the per-domain L1 caches share the remaining 1/4. L2 holds the complete dedup set, which
      // benefits directly from more capacity (fewer rolled-out / re-explored states). The L1s only
      // absorb within-domain repeats, whose working set is strongly biased toward spatiotemporally
      // close states -- so a smaller cache still captures most of the local hit rate. Hence the 3:1
      // weighting toward L2.
      _maxStoreSizeBytes   = (totalBudgetBytes * 3) / 4;
      _l1MaxStoreSizeBytes = (totalBudgetBytes / 4) / (size_t)_numaCount;
      _l1.resize(_numaCount);
      for (int i = 0; i < _numaCount; i++) _l1[i] = makeStore();
    }
    else
    {
      // Single domain: no L1 tier, so the one global store gets the whole budget.
      _maxStoreSizeBytes = totalBudgetBytes;
    }

    // The single global authoritative store (holds the complete dedup set)
    _l2 = makeStore();
  }

  // Function to print relevant information
  void printInfo() const
  {
    // Single-domain (no L1 tier): report L2 as a single store
    if (_l1.empty())
    {
      const size_t q = _l2->getQueryCount(), c = _l2->getCollisionCount();
      size_t       entries = 0;
      for (const auto& s : _l2->_hashStores) entries += s.hashSet.size();
      jaffarCommon::logger::log("[J+]      + Single store: Checks: %lu, Collisions: %lu (%.3f%%), Entries: %.3f M\n", q, c, q == 0 ? 0.0 : 100.0 * (double)c / (double)q,
                                (double)entries / (1024.0 * 1024.0));
      return;
    }

    // Two-tier: aggregate L1 across domains + the global L2
    size_t l1q = 0, l1c = 0, l1entries = 0;
    for (auto* const g : _l1)
    {
      l1q += g->getQueryCount();
      l1c += g->getCollisionCount();
      for (const auto& s : g->_hashStores) l1entries += s.hashSet.size();
    }
    const size_t l2q = _l2->getQueryCount();
    const size_t l2c = _l2->getCollisionCount();
    size_t       l2entries = 0;
    for (const auto& s : _l2->_hashStores) l2entries += s.hashSet.size();

    jaffarCommon::logger::log("[J+]   Two-Tier Dedup (NUMA-local L1 + global L2):\n");
    jaffarCommon::logger::log("[J+]      + L1 (local): Checks: %lu, Local Hits: %lu (%.3f%%), Entries: %.3f M\n", l1q, l1c,
                              l1q == 0 ? 0.0 : 100.0 * (double)l1c / (double)l1q, (double)l1entries / (1024.0 * 1024.0));
    jaffarCommon::logger::log("[J+]      + L2 (global): Checks (L1 misses): %lu (%.3f%% of all), Cross-Domain Hits: %lu, Entries: %.3f M\n", l2q,
                              l1q == 0 ? 0.0 : 100.0 * (double)l2q / (double)l1q, l2c, (double)l2entries / (1024.0 * 1024.0));
    jaffarCommon::logger::log("[J+]      + Served locally (no L2 access): %.3f%% | Total dup rate: %.3f%%\n", l1q == 0 ? 0.0 : 100.0 * (double)l1c / (double)l1q,
                              l1q == 0 ? 0.0 : 100.0 * (double)(l1c + l2c) / (double)l1q);
  }

  /**
   * Function to check whether the provided hash is already present in the database
   */
  __INLINE__ bool checkHashExists(const jaffarCommon::hash::hash_t hash)
  {
    const auto threadId = jaffarCommon::parallel::getThreadId();

    // Single-domain: just the one local store
    if (_l1.empty())
    {
      _l2->_threadQueryCount[threadId].value++;
      const bool collision = probeStore(_l2, hash);
      if (collision) _l2->_threadCollisionCount[threadId].value++;
      return collision;
    }

    // L1: this domain's local store. By construction every hash in any L1 was also inserted into L2,
    // so an L1 hit is a genuine duplicate -- answered locally, with no L2 (possibly remote) access.
    // probeStore() also inserts the hash into L1's current store (caching it locally).
    auto* const l1 = _l1[_preferredNumaDomain];
    l1->_threadQueryCount[threadId].value++;
    if (probeStore(l1, hash))
    {
      l1->_threadCollisionCount[threadId].value++;
      return true;
    }

    // L1 miss -> consult the authoritative global L2. probeStore() inserts the hash into L2's current
    // store, making it globally visible (so other domains detect it as a duplicate). This is what
    // preserves complete, global dedup: every globally-new hash is registered in L2 exactly once, by
    // whichever domain first encounters it.
    _l2->_threadQueryCount[threadId].value++;
    const bool collision = probeStore(_l2, hash);
    if (collision) _l2->_threadCollisionCount[threadId].value++;
    return collision;
  }

  /**
   * This function simply inserts a hash without checking for collisions
   */
  __INLINE__ void insertHash(const jaffarCommon::hash::hash_t hash)
  {
    if (_l1.empty() == false) _l1[_preferredNumaDomain]->_hashStores.rbegin()->hashSet.insert(hash);
    _l2->_hashStores.rbegin()->hashSet.insert(hash);
  }

  /**
   * This function serves to indicate a new step has started.
   * The current age is increased, and if a store exceeds its maximum size, it is rolled.
   */
  __INLINE__ void advanceStep()
  {
    for (auto* const g : _l1) rollStore(g, _l1MaxStoreSizeBytes);
    rollStore(_l2, _maxStoreSizeBytes);
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
   * Creates a fresh store with a single (empty) hash set and per-thread counters.
   */
  __INLINE__ NUMAStore_t* makeStore()
  {
    auto* s = new NUMAStore_t;
    s->_hashStores.push_back(hashStore_t({.id = s->_currentHashStoreId++, .age = s->_currentAge, .queryCount = 0, .collisionCount = 0}));
    s->_threadQueryCount.resize(_threadCount);
    s->_threadCollisionCount.resize(_threadCount);
    return s;
  }

  /**
   * Reverse-scans a store's hash sets (newest first), inserting the hash into the current (newest)
   * set and checking-only the archived sets. Returns true if the hash was already present anywhere
   * (a collision). Inserting-while-checking on the newest set means a single probe both registers
   * the hash and detects a same-step duplicate.
   */
  __INLINE__ bool probeStore(NUMAStore_t* const store, const jaffarCommon::hash::hash_t hash)
  {
    auto   itr             = store->_hashStores.rbegin();
    size_t curHashStoreIdx = 0;
    while (itr != store->_hashStores.rend())
    {
      bool collisionFound = false;
      if (curHashStoreIdx == 0) collisionFound = itr->hashSet.insert(hash).second == false;
      else collisionFound = itr->hashSet.contains(hash);
      if (collisionFound == true) return true;
      itr++;
      curHashStoreIdx++;
    }
    return false;
  }

  /**
   * Rolls a single store: if its current (newest) set's measured memory exceeds the budget, snapshot
   * its counters, discard the oldest set if at capacity, and push a fresh current set. Then age it.
   */
  __INLINE__ void rollStore(NUMAStore_t* const g, const size_t maxBytes)
  {
    auto& currentHashStore = *g->_hashStores.rbegin();

    if (getStoreSizeBytes(currentHashStore) >= maxBytes)
    {
      // Snapshot the (summed per-thread) counters into the current store before any structural
      // change, then reset. This must precede pop_front(): when maxStoreCount == 1 the front store
      // IS the current (back) store, so popping it first would leave currentHashStore dangling.
      currentHashStore.queryCount     = g->getQueryCount();
      currentHashStore.collisionCount = g->getCollisionCount();
      g->resetCounts();

      // If we already reached the maximum hash stores, discard the oldest one
      if (g->_hashStores.size() == _maxStoreCount) g->_hashStores.pop_front();

      // Now create the new one, by pushing it from the back
      g->_hashStores.push_back(hashStore_t({.id = g->_currentHashStoreId++, .age = g->_currentAge, .queryCount = 0, .collisionCount = 0}));
    }

    // Increasing age
    g->_currentAge++;
  }

  /**
   * Computes the actual resident memory of a hash store from the underlying set's measured
   * allocated capacity (number of slots), rather than from an entries-to-bytes estimate. This
   * tracks phmap's real footprint -- which varies with load factor and per-submap power-of-two
   * rounding -- so the configured store budget is respected instead of being over- or under-shot.
   */
  __INLINE__ size_t getStoreSizeBytes(const hashStore_t& store) const { return store.hashSet.capacity() * _bytesPerSlot + _hashStoreFixedOverhead; }

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
   * Maximum (global L2) store size (in megabytes)
   */
  double _maxStoreSizeMb;

  /**
   * Maximum global L2 store size, in bytes (derived from _maxStoreSizeMb)
   */
  size_t _maxStoreSizeBytes;

  /**
   * Per-domain L1 store budget, in bytes (derived: global budget / number of NUMA domains)
   */
  size_t _l1MaxStoreSizeBytes = 0;

  /**
   * Per-domain NUMA-local L1 caches. Empty on a single-domain run.
   */
  std::vector<NUMAStore_t*> _l1;

  /**
   * The single global authoritative store (holds the complete dedup set).
   */
  NUMAStore_t* _l2 = nullptr;
};

} // namespace jaffarPlus
