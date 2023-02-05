#include <set>
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include <parallel_hashmap/phmap.h>

#define COPYPROT_SOLUTION_COUNT 14

static std::vector<std::vector<uint8_t>> cutsceneDelays =
 {
/* 01 */  {},
/* 15 */  {},
/* 02 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 03 */  {},
/* 04 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 05 */  {},
/* 06 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 07 */  {},
/* 08 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 09 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 10 */  {},
/* 11 */  {},
/* 12 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 13 */  {},
/* 14 */  {}
 };

static std::vector<std::vector<uint8_t>> endWaitDelays =
 {
/* 01 */  {},
/* 15 */  {2,2,2,2,2,2,2,2},
/* 02 */  {},
/* 03 */  {},
/* 04 */  {},
/* 05 */  {},
/* 06 */  {},
/* 07 */  {2, 2, 2, 2, 2, 2, 2, 2, 2},
/* 08 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 09 */  {4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
/* 10 */  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/* 11 */  {2, 2, 2, 2, 2, 2, 2},
/* 12 */  {},
/* 13 */  {},
/* 14 */  {}
 };

struct solution_t
{
 uint32_t initialRNG;
 uint32_t timeStep;
 uint8_t cutsceneDelays[16];
 uint8_t endDelays[16];
 uint16_t totalDelay = 0;
 uint16_t costlyDelay = 0;
 uint8_t copyProtPlace = 0;
};

struct solutionFlat_t
{
 uint32_t rng;
 uint8_t looseSound;
 solution_t solution;
};

// Configuration for parallel hash maps
#define MAPNAME phmap::parallel_flat_hash_map
#define MAPEXTRAARGS , phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, std::allocator<std::pair<const K, V>>, 4, std::mutex
template <class K, class V> using HashMapT = MAPNAME<K, V MAPEXTRAARGS>;
using hashMap_t = HashMapT<std::pair<uint32_t, uint8_t>, solution_t>;

uint64_t processedRNGs;
uint64_t targetRNGs;
uint64_t successRNGs;
bool processing = false;

struct level_t
{
 uint8_t levelId;
 std::string solutionFile;
 std::string moveSequence;
 std::vector<std::string> moveListStrings;
 std::vector<uint8_t> moveList;
 uint16_t sequenceLength;
 std::string stateFile;
 uint8_t stateData[_STATE_DATA_SIZE_TRAIN];
 uint8_t RNGOffset;
};
