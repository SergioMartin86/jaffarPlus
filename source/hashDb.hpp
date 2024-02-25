#pragma once

#include <atomic>
#include <jaffarCommon/include/hash.hpp>
#include <jaffarCommon/include/json.hpp>
#include <jaffarCommon/include/concurrent.hpp>

namespace jaffarPlus
{

class HashDb final
{
  public:
  
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

    // The internal set for the hash store
    jaffarCommon::HashSet_t<jaffarCommon::hash_t> hashSet = { };
  };

  HashDb(const nlohmann::json& config)
  {
   _maxStoreCount = JSON_GET_NUMBER(size_t, config, "Max Store Count");
   _maxStoreSizeMb = JSON_GET_NUMBER(double, config, "Max Store Size (Mb)");

   // Calculating the maximum store size in entries
   _maxStoreEntries = std::floor((_maxStoreSizeMb / _bytesPerEntry) * 1024.0 * 1024.0);

   // Creating first hash db store
   _hashStores.push_back(hashStore_t({ .id = _currentHashStoreId++, .age = _currentAge }));

   // Resizing counter vectors
   for (size_t i = 0; i < _maxStoreCount; i++)
   {
    _queryCounters.emplace_back(std::make_unique<std::atomic<size_t>>(0));
    _collisionCounters.emplace_back(std::make_unique<std::atomic<size_t>>(0));
   }
  }

  ~HashDb() = default;

  // Function to print relevant information
  void printInfo() const
  {
   LOG("[J++]  + Max Store Count:               %lu\n", _maxStoreCount);
   LOG("[J++]  + Max Store Size:                %f Mb (%.2f Gb)\n", _maxStoreSizeMb, _maxStoreSizeMb / 1024.0);
   LOG("[J++]  + Max Store Entries:             %lu (%.2f Mentries)\n", _maxStoreEntries, (double)_maxStoreEntries / (1024.0 * 1024.0));
   LOG("[J++]  + Total Max Entries:             %lu (%.2f Mentries)\n", _maxStoreEntries * _maxStoreCount, ((double)_maxStoreEntries * _maxStoreCount) / (1024.0 * 1024.0));

   // Printing hash store information
   LOG("[J++]  + Hash Stores (%lu / %lu):\n", _hashStores.size(), _maxStoreCount);
   
   auto itr = _hashStores.rbegin();
   size_t curHashStoreIdx = 0;
   while (itr != _hashStores.rend())
   {
   LOG("[J++]    + [%02lu] - Age: %lu, Entries: %.3f M, Size: %.3f Mb, Check Count: %lu, Collision Count: %lu (Rate %.3f%%)\n",
       itr->id,
       itr->age,
       (double)itr->hashSet.size() / (1024.0 * 1024.0) ,
       (_bytesPerEntry * (double)itr->hashSet.size())  / (1024.0 * 1024.0),
       _queryCounters[curHashStoreIdx]->load(),
       _collisionCounters[curHashStoreIdx]->load(),
       100.0 * (double)_collisionCounters[curHashStoreIdx]->load() / (double)_queryCounters[curHashStoreIdx]->load());
   itr++;
   curHashStoreIdx++; 
   }
  }

  /**
   * Function to check whether the provided hash is already present in any of the hash stores
  */
  inline bool checkHashExists(const jaffarCommon::hash_t hash)
  {
    // The current hash store is the latest to be entered
    auto itr = _hashStores.rbegin();
    size_t curHashStoreIdx = 0;

    // Checking for the rest of the hash stores in reverse order, to increase chances of early collision detection
    while (itr != _hashStores.rend())
    {
      // Increasing query count for this hash store position
      _queryCounters[curHashStoreIdx]->operator++();

      // Flag to indicate whether a collision has been found
      bool collisionFound = false;

      // If it is the first hash db, check at the same time as we insert
      if (curHashStoreIdx == 0) collisionFound = itr->hashSet.insert(hash).second == false;

      // Otherwise, we simply check (no inserts)   
      if (curHashStoreIdx > 0)  collisionFound = itr->hashSet.contains(hash);

      // If collision is found, register it and return
      if (collisionFound == true)
      {
        // Increasing counter for collisions 
        _collisionCounters[curHashStoreIdx]->operator++();

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
  inline void insertHash(const jaffarCommon::hash_t hash)
  {
    // The current hash store is the latest to be entered
    auto itr = _hashStores.rbegin();
    auto& currentHashStore = *itr;

    // Inserting hash
    currentHashStore.hashSet.insert(hash);
  }

  /**
   * This function serves to indicate a new step has started.
   * The current age is increased, and if the current database exceeds its maximum
   * entries, it is send to the past db collection and a new one is created
  */
  inline void advanceStep()
  {
    // The current hash store is the latest to be entered
    auto itr = _hashStores.rbegin();
    auto& currentHashStore = *itr;

    // If the current hash store exceeds the entry limit, push put a new one in
    if (currentHashStore.hashSet.size() > _maxStoreEntries)
    {
      // First, if we already reached the maximum hash stores, then discard the oldest one first
      if (_hashStores.size() == _maxStoreCount) _hashStores.pop_front();

      // Now create the new one, by pushing it from the back
      _hashStores.push_back(hashStore_t({ .id = _currentHashStoreId++, .age = _currentAge }));
    }

    // Increasing age
    _currentAge++;
  }

  private:

  /**
   * Calculated empirically, by filling a hash set with a huge number of distinct hashes
   * and measuring the maximum resident (memory) set size after execution.
  */
  const double _bytesPerEntry = 32.0;

  /**
   * Identifier count for hash db stores
  */
  size_t _currentHashStoreId = 0;

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
   * Counter to store how many checks and collisions happened so far
   * This is done at an index level (and not at an individual store level) because
   * we are interested in knowing how frequently queries reach (and hit) the latest
   * hash stores (and not any one in particular)
  */
  std::vector<std::unique_ptr<std::atomic<size_t>>> _queryCounters;
  std::vector<std::unique_ptr<std::atomic<size_t>>> _collisionCounters;
};

} // namespace jaffarPlus