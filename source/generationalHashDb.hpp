#pragma once

// Generational, fixed-capacity hash database (prototype, opt-in via "Use Generational Eviction").
//
// Motivation: the default windowed HashDb discards a whole hash store at once, so right after a
// discard only ~(N-1)/N of the dedup memory holds live hashes (50% with the usual N=2), and a
// lookup must probe every one of the N stores. This design instead keeps a SINGLE set per NUMA
// group and ages entries out smoothly by generation, giving:
//   * one lookup per state (a single shard probe), and
//   * ~constant memory utilisation (no sawtooth).
//
// Memory is bounded by reserving the shards to the configured budget up front (so they never grow
// and erased slots are reused -- phmap does not release capacity on erase, so a growing-then-erasing
// table could never be bounded). Old generations are evicted in parallel at step boundaries to keep
// the live entry count under the reserved load limit.

#include "numa.hpp"
#include <array>
#include <cstdint>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/parallel.hpp>
#include <memory>
#include <mutex>
#include <phmap/parallel_hashmap/phmap.h>
#include <vector>

namespace jaffarPlus
{

class GenerationalHashDb final
{
public:
  // 256 shards (matching the parallel set's default fan-out) keeps lock contention low and lets the
  // eviction scan be parallelised one-shard-per-thread.
  static constexpr size_t   SHARD_COUNT  = 256;
  static constexpr size_t   SHARD_MASK   = SHARD_COUNT - 1;
  static constexpr uint64_t HASH_MASK_HI = 0x00000000FFFFFFFFULL; // low 32 bits of 'hi' carry the hash

  // A 128-bit value packing a 32-bit generation in the high bits of 'hi' and a 96-bit hash in the
  // remaining bits. 32 generation bits never wrap in practice (4 billion steps); a 96-bit hash keeps
  // the birthday-collision bound far beyond any reachable state count.
  struct genHash_t
  {
    uint64_t hi;
    uint64_t lo;
  };

  // Hash/Eq operate only on the 96 hash bits, ignoring the generation, so the same state inserted in
  // different generations is treated as one entry (deduplication is preserved).
  struct genHashHash
  {
    size_t operator()(const genHash_t& g) const
    {
      uint64_t x = g.lo ^ ((g.hi & HASH_MASK_HI) * 0x9E3779B97F4A7C15ULL);
      x ^= x >> 33;
      x *= 0xff51afd7ed558ccdULL;
      x ^= x >> 33;
      return (size_t)x;
    }
  };
  struct genHashEq
  {
    bool operator()(const genHash_t& a, const genHash_t& b) const { return ((a.hi ^ b.hi) & HASH_MASK_HI) == 0 && a.lo == b.lo; }
  };

  using shardSet_t = phmap::flat_hash_set<genHash_t, genHashHash, genHashEq>;

  static __INLINE__ genHash_t pack(const jaffarCommon::hash::hash_t& h, uint32_t gen) { return genHash_t{((uint64_t)gen << 32) | (h.first & HASH_MASK_HI), h.second}; }
  static __INLINE__ uint32_t  genOf(const genHash_t& g) { return (uint32_t)(g.hi >> 32); }
  static __INLINE__ size_t    shardOf(const jaffarCommon::hash::hash_t& h) { return (h.second >> 56) & SHARD_MASK; }

  // Cache-line-padded per-thread statistics counter (avoids the false sharing of a shared atomic).
  struct paddedCounter_t
  {
    alignas(64) size_t value = 0;
  };

  struct genGroup_t
  {
    std::array<shardSet_t, SHARD_COUNT> shards;
    std::array<std::mutex, SHARD_COUNT> locks;
    uint32_t                            currentGen = 0;
    uint32_t                            oldestGen  = 0;
    std::vector<paddedCounter_t>        threadQueryCount;
    std::vector<paddedCounter_t>        threadCollisionCount;
  };

  GenerationalHashDb(size_t maxStoreCount, double maxStoreSizeMb) : _maxStoreCount(maxStoreCount), _maxStoreSizeMb(maxStoreSizeMb) {}

  ~GenerationalHashDb()
  {
    for (auto* g : _groups) delete g;
  }

  __INLINE__ void initialize()
  {
    // Budget per NUMA group, matched to what the windowed scheme would use (store size x store count).
    const double budgetBytesPerGroup = _maxStoreSizeMb * 1024.0 * 1024.0 * (double)_maxStoreCount;
    const size_t totalSlots          = (size_t)(budgetBytesPerGroup / (double)_bytesPerSlot); // capacity (slots) for the budget

    // Reserve EVERY shard to the same capacity: the largest valid phmap capacity (2^k - 1) not
    // exceeding the per-shard share of the budget. Uniform sizing is essential -- hashes distribute
    // evenly across shards, so unequal shard capacities would make the smaller shards overflow (and
    // grow, which permanently breaks the memory bound since phmap never shrinks) long before the
    // larger ones fill. This rounds the total capacity DOWN to a power-of-two multiple (<= budget),
    // trading a little utilisation for a guaranteed memory ceiling.
    const size_t capPerShard = validCapacityFloor(totalSlots / SHARD_COUNT);

    for (ssize_t i = 0; i < _numaGroupCount; i++)
    {
      auto* g = new genGroup_t();
      g->threadQueryCount.resize(_threadCount);
      g->threadCollisionCount.resize(_threadCount);
      if (capPerShard > 0)
        for (size_t s = 0; s < SHARD_COUNT; s++) g->shards[s].reserve(capPerShard - capPerShard / 8); // CapacityToGrowth(capPerShard)
      _groups.push_back(g);
    }

    // Live-entry target: keep ~80% of the actual reserved capacity, below phmap's 7/8 growth trigger
    // so the set never resizes (which would break the bounded-memory guarantee).
    size_t totalCap = 0;
    if (!_groups.empty())
      for (size_t s = 0; s < SHARD_COUNT; s++) totalCap += _groups[0]->shards[s].capacity();
    _targetEntriesPerGroup = std::max<size_t>(1, (size_t)(0.80 * (double)totalCap));
  }

  // Largest valid phmap capacity (2^k - 1) not exceeding n (0 for n == 0).
  static __INLINE__ size_t validCapacityFloor(size_t n)
  {
    if (n < 1) return 0;
    size_t k = 0;
    while (((((size_t)1) << (k + 1)) - 1) <= n) k++;
    return (((size_t)1) << k) - 1;
  }

  // Single shard probe: insert-and-test the generation-tagged hash. Returns true on collision.
  __INLINE__ bool checkHashExists(const jaffarCommon::hash::hash_t hash)
  {
    auto* const g   = _groups[_preferredNumaGroup];
    const auto  tid = jaffarCommon::parallel::getThreadId();
    g->threadQueryCount[tid].value++;

    const size_t    s = shardOf(hash);
    const genHash_t v = pack(hash, g->currentGen);

    bool collision;
    {
      std::lock_guard<std::mutex> lk(g->locks[s]);
      collision = g->shards[s].insert(v).second == false;
    }
    if (collision) g->threadCollisionCount[tid].value++;
    return collision;
  }

  __INLINE__ void insertHash(const jaffarCommon::hash::hash_t hash)
  {
    auto* const                 g = _groups[_preferredNumaGroup];
    const size_t                s = shardOf(hash);
    std::lock_guard<std::mutex> lk(g->locks[s]);
    g->shards[s].insert(pack(hash, g->currentGen));
  }

  __INLINE__ void advanceStep()
  {
    for (auto* g : _groups)
    {
      // New states from the next step belong to a new generation
      g->currentGen++;

      // Evict the oldest generations until the set is back under its live-entry target. At steady
      // state one generation is added and one evicted, so this is a single parallel pass per step.
      size_t total = groupSize(g);
      while (total > _targetEntriesPerGroup && g->oldestGen < g->currentGen)
      {
        // Estimate how many of the oldest generations to drop in this pass (assuming roughly uniform
        // generation sizes), so a single scan usually suffices.
        const size_t window     = g->currentGen - g->oldestGen;
        const size_t excess     = total - _targetEntriesPerGroup;
        size_t       gensToDrop = 1;
        if (total > 0) gensToDrop = std::max<size_t>(1, (size_t)((double)excess * (double)window / (double)total));

        g->oldestGen += gensToDrop;
        if (g->oldestGen > g->currentGen) g->oldestGen = g->currentGen;

        evictOlderThan(g, g->oldestGen);
        total = groupSize(g);
      }
    }
  }

  void printInfo() const
  {
    for (size_t i = 0; i < _groups.size(); i++)
    {
      auto* const g     = _groups[i];
      size_t      total = 0, cap = 0, q = 0, c = 0;
      for (size_t s = 0; s < SHARD_COUNT; s++)
      {
        total += g->shards[s].size();
        cap += g->shards[s].capacity();
      }
      for (const auto& x : g->threadQueryCount) q += x.value;
      for (const auto& x : g->threadCollisionCount) c += x.value;

      jaffarCommon::logger::log("[J+]      + NUMA %lu - Generational - Live Generations: %u [%u..%u), Entries: %.3f M, Size: %.3f Mb, Check Count: %lu, Collision Count: %lu (Rate "
                                "%.3f%%)\n",
                                i, g->currentGen - g->oldestGen, g->oldestGen, g->currentGen, (double)total / (1024.0 * 1024.0), (double)(cap * _bytesPerSlot) / (1024.0 * 1024.0),
                                q, c, q == 0 ? 0.0 : 100.0 * (double)c / (double)q);
    }
  }

private:
  // phmap stores each value inline (one slot) plus one control byte per slot.
  static constexpr size_t _bytesPerSlot = sizeof(genHash_t) + sizeof(uint8_t);

  __INLINE__ size_t groupSize(genGroup_t* const g) const
  {
    size_t total = 0;
    for (size_t s = 0; s < SHARD_COUNT; s++) total += g->shards[s].size();
    return total;
  }

  // Erases every entry older than `threshold`, one shard per thread (each shard under its own lock).
  // Runs between steps (no concurrent inserts), so it is safe to spawn a fresh parallel region here.
  __INLINE__ void evictOlderThan(genGroup_t* const g, uint32_t threshold)
  {
    JAFFAR_PARALLEL
    {
      const size_t tid = jaffarCommon::parallel::getThreadId();
      const size_t nt  = jaffarCommon::parallel::getThreadCount();
      for (size_t s = tid; s < SHARD_COUNT; s += nt)
      {
        std::lock_guard<std::mutex> lk(g->locks[s]);
        auto&                       set = g->shards[s];
        for (auto it = set.begin(); it != set.end();)
        {
          if (genOf(*it) < threshold)
            it = set.erase(it);
          else
            ++it;
        }
      }
    }
  }

  size_t                   _maxStoreCount;
  double                   _maxStoreSizeMb;
  size_t                   _targetEntriesPerGroup = 0;
  std::vector<genGroup_t*> _groups;
};

} // namespace jaffarPlus
