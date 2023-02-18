#include <set>
#include "gameInstance.hpp"
#include "emuInstance.hpp"
#include "utils.hpp"
#include <parallel_hashmap/phmap.h>

#define COPYPROT_SOLUTION_COUNT 14
//#define MAX_CLOCK_TICKS 1573040
#define MAX_CLOCK_TICKS 1573040

#define STORE_DELAY_HISTORY

#define TEST_SINGLE
#ifdef TEST_SINGLE
#define SINGLE_RNG 0x5895B9AF
#define SINGLE_TICK 990277
#endif

typedef uint32_t rng_t;
typedef uint32_t clockTick_t;

#define BASE_RNG 0x0071BA7E
#define RNG_CLOCK_TICK_INCREMENT 0x343FD

static std::vector<std::vector<uint8_t>> cutsceneDelays =
 {
/* 01 */  {},
/* 15 */  {},
/* 02 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 03 */  {},
/* 04 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 05 */  {},
/* 06 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 07 */  {},
/* 08 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 09 */  {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 10 */  {},
/* 11 */  {},
/* 12 */  {1, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 13 */  {},
/* 14 */  {}
 };

static std::vector<std::vector<uint8_t>> endWaitDelays =
 {
/* 01 */  {},
/* 15 */  {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
/* 02 */  {},
/* 03 */  {},
/* 04 */  {},
/* 05 */  {},
/* 06 */  {},
/* 07 */  {2},
/* 08 */  {3},
/* 09 */  {4},
/* 10 */  {1},
/* 11 */  {2},
/* 12 */  {},
/* 13 */  {},
/* 14 */  {}
 };

struct solution_t
{
 clockTick_t clockTick;

 #ifdef STORE_DELAY_HISTORY
 uint8_t cutsceneDelays[16];
 uint8_t endDelays[16];
 #endif

 uint16_t totalDelay = 0;
 uint16_t costlyDelay = 0;
 uint8_t copyProtPlace = 0;
};

struct solutionFlat_t
{
 rng_t rng;
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

inline rng_t getRNGFromClockTick(const clockTick_t tick) { return BASE_RNG + tick * RNG_CLOCK_TICK_INCREMENT; }
inline clockTick_t getClockTickFromRNG(const rng_t rng) { return (rng - BASE_RNG) / RNG_CLOCK_TICK_INCREMENT; }

inline uint8_t getMinutesFromIGT(const uint16_t timer) { return 60 - (timer / 719); }
inline uint16_t getTicksFromIGT(const uint16_t timer)
{
 return 719 - (timer % 719);
}
