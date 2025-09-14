#pragma once

#include <atomic>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/parallel.hpp>
#include "numa.hpp"

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
    // Calculating the maximum store size in entries
    _maxStoreEntries = std::floor((_maxStoreSizeMb / _bytesPerEntry) * 1024.0 * 1024.0);

    // Initializing hash store vector
    for (ssize_t i = 0; i < _numaGroupCount; i++) 
      _numaGroups.push_back(new NUMAStore_t);

    // Creating first hash db store
    for (size_t i = 0; i < _numaGroups.size(); i++) 
      _numaGroups[i]->_hashStores.push_back(hashStore_t({.id = _numaGroups[i]->_currentHashStoreId++, .age = _numaGroups[i]->_currentAge, .queryCount = 0, .collisionCount = 0}));
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
        const size_t queryCount = itr == _numaGroups[i]->_hashStores.rbegin() ? _numaGroups[i]->_currentQueryCount.load() : itr->queryCount;
        const size_t collisionCount = itr == _numaGroups[i]->_hashStores.rbegin() ? _numaGroups[i]->_currentCollisionCount.load() : itr->collisionCount;

        jaffarCommon::logger::log("[J+]      + NUMA %lu - Store: %lu / %lu - Id: %lu - Age: %lu, Entries: %.3f M / %.3f M, Size: %.3f Mb / %.3f Mb, Check Count: %lu, Collision Count: %lu (Rate %.3f%%)\n",
                                  i,
                                  curHashStoreIdx + 1,
                                  _maxStoreCount,
                                  itr->id,
                                  itr->age,
                                  (double)itr->hashSet.size() / (1024.0 * 1024.0),
                                  (double)_maxStoreEntries / (1024.0 * 1024.0),
                                  (_bytesPerEntry * (double)itr->hashSet.size()) / (1024.0 * 1024.0),
                                  _maxStoreSizeMb,
                                  queryCount,
                                  collisionCount,
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
    // The current hash store is the latest to be entered
    auto   itr             = _numaGroups[_preferredNumaGroup]->_hashStores.rbegin();
    size_t curHashStoreIdx = 0;

    // Checking for the rest of the hash stores in reverse order, to increase chances of early collision detection
    while (itr != _numaGroups[_preferredNumaGroup]->_hashStores.rend())
    {
      // Increasing query count for this hash store position
      _numaGroups[_preferredNumaGroup]->_currentQueryCount++;

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
        _numaGroups[_preferredNumaGroup]->_currentCollisionCount++;

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
   * entries, it is send to the past db collection and a new one is created
   */
  __INLINE__ void advanceStep()
  {
    // For each NUMA store, advance the step of its current hash store
    for (size_t i = 0; i < _numaGroups.size(); i++) 
    {
      // The current hash store is the latest to be entered
      auto& currentHashStore = *_numaGroups[i]->_hashStores.rbegin();

      // If the current hash store exceeds the entry limit, push put a new one in
      if (currentHashStore.hashSet.size() > _maxStoreEntries)
      {
        // First, if we already reached the maximum hash stores, then discard the oldest one first
        if (_numaGroups[i]->_hashStores.size() == _maxStoreCount) _numaGroups[i]->_hashStores.pop_front();

        // Updating counters for the current hash db
        currentHashStore.queryCount = _numaGroups[i]->_currentQueryCount;            
        currentHashStore.collisionCount = _numaGroups[i]->_currentCollisionCount;

        // Resetting counters
        _numaGroups[i]->_currentQueryCount = 0;
        _numaGroups[i]->_currentCollisionCount = 0;

        // Now create the new one, by pushing it from the back
        _numaGroups[i]->_hashStores.push_back(hashStore_t({.id = _numaGroups[i]->_currentHashStoreId++, .age = _numaGroups[_preferredNumaGroup]->_currentAge, .queryCount = 0, .collisionCount = 0 }));
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
    * Query count for the current hash db
    */
    std::atomic<size_t> _currentQueryCount = 0;

    /**
    * Collision count for the current hash db
    */
    std::atomic<size_t> _currentCollisionCount = 0;
  };

  /**
   * Number of NUMA domains per group
   */
   size_t _numaDomainsPerGroup;

  /**
   * Calculated empirically, by filling a hash set with a huge number of distinct hashes
   * and measuring the maximum resident (memory) set size after execution.
   */
  const double _bytesPerEntry = 32.0;

  /**
   * Maximum number of stores to keep at any time
   */
  size_t _maxStoreCount;

  /**
   * Maximum store size (in megabytes)
   */
  double _maxStoreSizeMb;

  /**
   * Maximum store entries
   */
  size_t _maxStoreEntries;

  /**
   * Set of hash databases stores, one per numa domain
   */
   std::vector<NUMAStore_t*> _numaGroups;
};

} // namespace jaffarPlus
