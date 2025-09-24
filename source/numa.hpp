#pragma once

#include <jaffarCommon/parallel.hpp>
#include <numa.h>

namespace jaffarPlus
{

/**
* Number of NUMA domains detected
*/
static int _numaCount;

/**
* Number of NUMA domains per group
*/
static int _numaDomainsPerGroup;

/**
* Number of NUMA groups
*/
static int _numaGroupCount;

/**
 * Number of threads to use
 */
static int _threadCount;

/**
 * NUMA Distance Matrix
 */
static std::vector<std::vector<size_t>> _numaDistanceMatrix;

/**
 * NUMA Preference Matrix
 */
static std::vector<std::vector<size_t>> _numaPreferenceMatrix;

/**
 * Delegate thread per numa domain
 */
static std::vector<ssize_t> _numaDelegateThreadId;

/**
* Thread local storage of preferred NUMA domain
*/
static thread_local int _preferredNumaDomain;

/**
* Thread local storage of preferred NUMA Group
*/
static thread_local int _preferredNumaGroup;

/**
* Thread local storage of my cpu id
*/
static thread_local int _myThreadId;

/*
* Function to initialize NUMA / core affinity aspects
*/
__INLINE__ void initializeNUMA(const int numaDomainsPerGroup = 1)
{
    // Getting number of threads
    _threadCount = jaffarCommon::parallel::getMaxThreadCount();

    // Accept serial execution if thread count is one
    if (_threadCount == 1)
    {
      // Setting numa domains per group, if grouping is used
      _numaDomainsPerGroup = 1;

      // Getting number of noma domains
      _numaCount = 1;
    }
    else
    {
      // Setting numa domains per group, if grouping is used
      _numaDomainsPerGroup = numaDomainsPerGroup;

      // Getting number of noma domains
      _numaCount = numa_max_node() + 1;
    }
    
    // Checking that NUMA grouping is correct
    if (_numaCount % _numaDomainsPerGroup > 0) JAFFAR_THROW_RUNTIME("Hash DB grouping (%lu) does not divide the NUMA domain count exactly (%lu)\n", _numaDomainsPerGroup, _numaCount);

    // Checking whether the numa library calls are available
    const auto numaAvailable = numa_available();
    if (numaAvailable != 0) JAFFAR_THROW_RUNTIME("The system does not provide NUMA detection support.");

    // Setting number of NUMA groups
    _numaGroupCount = _numaCount / _numaDomainsPerGroup;

    // Overriding thread count, if requested
    if (auto* value = std::getenv("JAFFAR_ENGINE_OVERRIDE_MAX_THREAD_COUNT")) jaffarCommon::parallel::setThreadCount(std::stoul(value));

    // Check NUMA distances
    _numaDistanceMatrix.resize(_numaCount);
    for(ssize_t i = 0; i < _numaCount; i++) _numaDistanceMatrix[i].resize(_numaCount);
    for(ssize_t i = 0; i < _numaCount; i++) for(ssize_t j = 0; j < _numaCount; j++) _numaDistanceMatrix[i][j] = (size_t)numa_distance(i,j);

    // printf("NUMA Distances:\n");
    // for(ssize_t i = 0; i < _numaCount; i++)
    // {
    //   for(ssize_t j = 0; j < _numaCount; j++)
    //   {
    //     printf("%3lu ", _numaDistanceMatrix[i][j]);
    //   }
    //   printf("\n");
    // }

    // Establishing NUMA preference matrix
    _numaPreferenceMatrix.resize(_numaCount);
    for(ssize_t i = 0; i < _numaCount; i++)
    {
      // Putting NUMA domains in the same group based on distance
      std::map<size_t, std::vector<int>> _localPreferenceMap;
      for(ssize_t j = 0; j < _numaCount; j++) _localPreferenceMap[_numaDistanceMatrix[i][j]].push_back(j);

      // Assigning NUMA domains based on local preference
      for (const auto& entry : _localPreferenceMap)
      {
        const auto& localPreferenceGroup = entry.second;
        for (ssize_t j = 0; j < (ssize_t)localPreferenceGroup.size(); j++)
        {
          const size_t offset = (j + i) % localPreferenceGroup.size();
          _numaPreferenceMatrix[i].push_back(localPreferenceGroup[offset]);
        }  
      }
    }

    // printf("NUMA Locality Preferences:\n");
    // for(ssize_t i = 0; i < _numaCount; i++)
    // {
    //   for(ssize_t j = 0; j < _numaCount; j++)
    //   {
    //     printf("%3lu ", _numaPreferenceMatrix[i][j]);
    //   }
    //   printf("\n");
    // }

    //// Setting one delegate thread per NUMA domain for administrative (allocation) purposes
    // Initializing numa delegate thread storage
    _numaDelegateThreadId.resize(_numaCount);
    for (int i = 0; i < _numaCount; i++) _numaDelegateThreadId[i] = -1;

    // Detecting thread-specific NUMA information
    std::mutex workMutex;
    JAFFAR_PARALLEL
    {
      // Getting thread information from the system
      _myThreadId          = sched_getcpu();
      _preferredNumaDomain = numa_node_of_cpu(_myThreadId);
      _preferredNumaGroup = _preferredNumaDomain / _numaDomainsPerGroup;

      // Setting myself as numa delegate, if I'm the lowest cpu id in the numa domain
      workMutex.lock();
      if (_numaDelegateThreadId[_preferredNumaDomain] == -1) _numaDelegateThreadId[_preferredNumaDomain] = _myThreadId;
      workMutex.unlock();
    }
}

} // namespace jaffarPlus