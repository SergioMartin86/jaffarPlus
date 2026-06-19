#pragma once

/**
 * @file numa.hpp
 * @brief NUMA topology detection: distance/preference matrices and per-domain delegate-thread
 *        selection, identifying threads by OpenMP thread id and using sched_getcpu only for the
 *        NUMA domain lookup.
 */

#include <cstdlib>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/parallel.hpp>
#include <map>
#include <mutex>
#include <numa.h>
#include <vector>

namespace jaffarPlus
{

static int _numaCount; ///< Number of NUMA domains detected.

static int _threadCount; ///< Number of threads to use.

static std::vector<std::vector<size_t>> _numaDistanceMatrix; ///< NUMA distance matrix; entry [i][j] is the reported distance from domain i to domain j.

static std::vector<std::vector<size_t>>
    _numaPreferenceMatrix; ///< NUMA preference matrix; row i lists domains ordered by ascending distance from domain i (ties rotated per source domain).

static std::vector<ssize_t> _numaDelegateThreadId; ///< Delegate thread (OpenMP thread id) per NUMA domain, or -1 if none assigned.

static thread_local int _preferredNumaDomain; ///< Thread-local preferred NUMA domain (the NUMA node of the CPU the thread currently runs on).

static thread_local int _myThreadId; ///< Thread-local dense OpenMP thread id of the current thread.

/**
 * @brief Initializes NUMA / core-affinity state.
 * @details Detects the thread count and NUMA domain count, optionally overrides the thread count from
 * the JAFFAR_ENGINE_OVERRIDE_MAX_THREAD_COUNT environment variable, builds the NUMA distance and
 * preference matrices, and selects one delegate thread per NUMA domain. Each worker identifies itself
 * by its OpenMP thread id and derives its preferred NUMA domain from sched_getcpu.
 * @throws A runtime error if the system does not provide NUMA detection support.
 */
__INLINE__ void initializeNUMA()
{
  // Getting number of threads
  _threadCount = jaffarCommon::parallel::getMaxThreadCount();

  // Getting number of numa domains (serial execution collapses to a single domain)
  if (_threadCount == 1)
    _numaCount = 1;
  else
    _numaCount = numa_max_node() + 1;

  // Checking whether the numa library calls are available
  const auto numaAvailable = numa_available();
  if (numaAvailable != 0) JAFFAR_THROW_RUNTIME("The system does not provide NUMA detection support.");

  // Overriding thread count, if requested
  if (auto* value = std::getenv("JAFFAR_ENGINE_OVERRIDE_MAX_THREAD_COUNT")) jaffarCommon::parallel::setThreadCount(std::stoul(value));

  // Check NUMA distances
  _numaDistanceMatrix.resize(_numaCount);
  for (ssize_t i = 0; i < _numaCount; i++) _numaDistanceMatrix[i].resize(_numaCount);
  for (ssize_t i = 0; i < _numaCount; i++)
    for (ssize_t j = 0; j < _numaCount; j++) _numaDistanceMatrix[i][j] = (size_t)numa_distance(i, j);

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
  for (ssize_t i = 0; i < _numaCount; i++)
  {
    // Putting NUMA domains in the same group based on distance
    std::map<size_t, std::vector<int>> _localPreferenceMap;
    for (ssize_t j = 0; j < _numaCount; j++) _localPreferenceMap[_numaDistanceMatrix[i][j]].push_back(j);

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
    // Identify this worker by its dense OpenMP thread id (unique 0..N-1), NOT sched_getcpu(): the CPU
    // id collides whenever threads share a core -- under oversubscription, or simply unpinned
    // scheduling on CI runners that don't set OMP_PROC_BIND. A colliding id corrupts the per-NUMA
    // delegate selection below (and StateDb's delegate-gated queue allocation), leaving a domain with
    // no queues so the search immediately "runs out of states". The preferred NUMA domain still comes
    // from the actual CPU the thread is currently on.
    _myThreadId          = jaffarCommon::parallel::getThreadId();
    _preferredNumaDomain = numa_node_of_cpu(sched_getcpu());

    // Setting myself as numa delegate, if I'm the first thread seen in this numa domain
    workMutex.lock();
    if (_numaDelegateThreadId[_preferredNumaDomain] == -1) _numaDelegateThreadId[_preferredNumaDomain] = _myThreadId;
    workMutex.unlock();
  }
}

} // namespace jaffarPlus