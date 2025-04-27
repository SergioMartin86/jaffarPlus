#include "emulator.hpp"
#include "game.hpp"
#include "runner.hpp"
#include <argparse/argparse.hpp>
#include <chrono>
#include <cstdlib>
#include <emulatorList.hpp>
#include <gameList.hpp>
#include <jaffarCommon/concurrent.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/parallel.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/string.hpp>
#include <jaffarCommon/timing.hpp>
#include <math.h>
#include <random>
#include <thread>

// Set to 60 for last input optimization
#define _CURRENT_STAGE 60

// Jingle time to wait for next level. Use unless it's already level 60 (last one)
#if _CURRENT_STAGE == 60
#define _JINGLE_TIME 0ul
#else
#define _JINGLE_TIME 150ul
#endif

#define _MAX_STROKES 20
#define _ANGLE_COUNT 256
#define _INITIAL_MAX_FRAMES 30000
#define _MAX_NEW_CANDIDATES_PER_STROKE 8000
#define _STATE_SIZE 2223ul
// #define _CONSIDER_SPEED_INCREMENT
#define _CHECK_COLLISIONS

struct candidate_t
{
  uint16_t _frameCount = 0;
  uint8_t  _pendingBalls;
  uint8_t  _rate;
  uint8_t  _strokeCount = 0;
  int8_t   _direction[_MAX_STROKES];

#ifdef _CONSIDER_SPEED_INCREMENT
  uint8_t _speedIncrement[_MAX_STROKES];
#endif

  uint8_t _angle[_MAX_STROKES];
  uint8_t _speed[_MAX_STROKES];
  uint8_t _scoredBalls[_MAX_STROKES];

  uint16_t _angleFrames[_MAX_STROKES];
  uint16_t _extraAngleFrames[_MAX_STROKES];
  uint16_t _speedFrames[_MAX_STROKES];
  uint16_t _playFrames[_MAX_STROKES];
  uint16_t transitionFrames[_MAX_STROKES];

  uint8_t _state[_STATE_SIZE];
};

__INLINE__ void serializeState(const jaffarPlus::games::nes::LunarBall* game, candidate_t* candidate)
{
  jaffarCommon::serializer::Contiguous s(candidate->_state);
  game->serializeState(s);
}

__INLINE__ void deserializeState(jaffarPlus::games::nes::LunarBall* game, const candidate_t* candidate)
{
  jaffarCommon::deserializer::Contiguous d(candidate->_state);
  game->deserializeState(d);
}

__INLINE__ void printCandidateInfo(const candidate_t* candidate)
{
  jaffarCommon::logger::log("[J+] Frames: %10lu", candidate->_frameCount);
  jaffarCommon::logger::log(" - Rate:          %3u", candidate->_rate);
  jaffarCommon::logger::log(" - Pending Balls: %3u", candidate->_pendingBalls);

  jaffarCommon::logger::log(" - Scored Balls [");
  for (uint8_t i = 0; i < candidate->_strokeCount; i++) jaffarCommon::logger::log(" %3u", candidate->_scoredBalls[i]);
  jaffarCommon::logger::log(" ]");

  jaffarCommon::logger::log(" - Angles [");
  for (uint8_t i = 0; i < candidate->_strokeCount; i++) jaffarCommon::logger::log(" %3u", candidate->_angle[i]);
  jaffarCommon::logger::log(" ]");

  jaffarCommon::logger::log(" - Speeds [");
  for (uint8_t i = 0; i < candidate->_strokeCount; i++) jaffarCommon::logger::log(" %3u", candidate->_speed[i]);
  jaffarCommon::logger::log(" ]");

#ifdef _CONSIDER_SPEED_INCREMENT
  jaffarCommon::logger::log(" - Increments [");
  for (uint8_t i = 0; i < candidate->_strokeCount; i++) jaffarCommon::logger::log(" %3u", candidate->_speedIncrement[i]);
  jaffarCommon::logger::log(" ]");
#endif

  jaffarCommon::logger::log(" - Direction [");
  for (uint8_t i = 0; i < candidate->_strokeCount; i++) jaffarCommon::logger::log(" %s", candidate->_direction[i] == -1 ? "L" : "R");
  jaffarCommon::logger::log(" ]");

  jaffarCommon::logger::log("\n");
}

__INLINE__ candidate_t* getFreeCandidateMove(jaffarCommon::concurrent::atomicQueue_t<void*>* freeCandidateQueue)
{
  // Storage for the new free state space
  void* candidatePtr;

  // Trying to get free space for a new state
  bool success = freeCandidateQueue->try_pop(candidatePtr);

  // If successful, return the pointer immediately
  if (success == true) return (candidate_t*)candidatePtr;

  // Otherwise, fail
  JAFFAR_THROW_LOGIC("Could not find free candidate space");
}

__INLINE__ void returnFreeCandidateMove(jaffarCommon::concurrent::atomicQueue_t<void*>* freeCandidateQueue, candidate_t* candidatePtr)
{
  // Trying to get free space for a new state
  bool success = freeCandidateQueue->try_push(candidatePtr);

  // If successful, return the pointer immediately
  if (success == false) JAFFAR_THROW_RUNTIME("Failed on pushing free state back. This must be a bug in Jaffar\n");
}

__INLINE__ void saveBestCandidate(candidate_t* candidatePtr)
{
  FILE* fptr;
  fptr = fopen("jaffar.best.sol", "w");

  for (uint8_t stroke = 0; stroke < candidatePtr->_strokeCount; stroke++)
  {
    if (candidatePtr->_angleFrames[stroke] > 0)
    {
      for (size_t i = 0; i < candidatePtr->_angleFrames[stroke]; i++)
      {
        if (candidatePtr->_direction[stroke] == -1) fprintf(fptr, "|..|..L.....|\n");
        if (candidatePtr->_direction[stroke] == 1) fprintf(fptr, "|..|...R....|\n");
      }

      fprintf(fptr, "|..|........|\n");
      fprintf(fptr, "|..|........|\n");
    }

    for (size_t i = 0; i < candidatePtr->_extraAngleFrames[stroke]; i++)
    {
      if (candidatePtr->_direction[stroke] == -1) fprintf(fptr, "|..|..L.....|\n");
      if (candidatePtr->_direction[stroke] == 1) fprintf(fptr, "|..|...R....|\n");
      fprintf(fptr, "|..|........|\n");
      fprintf(fptr, "|..|........|\n");
    }

    for (size_t i = 0; i < candidatePtr->_speedFrames[stroke]; i++) fprintf(fptr, "|..|........|\n");

    fprintf(fptr, "|..|......B.|\n");
    fprintf(fptr, "|..|......B.|\n");

    for (size_t i = 0; i < candidatePtr->_playFrames[stroke]; i++) fprintf(fptr, "|..|........|\n");
    for (size_t i = 0; i < candidatePtr->transitionFrames[stroke]; i++) fprintf(fptr, "|..|........|\n");
  }

  fclose(fptr);
}

__INLINE__ uint8_t getAngleDistance(const uint8_t angle1, const uint8_t angle2)
{
  int16_t a1 = angle1;
  int16_t a2 = angle2;

  int16_t d1 = std::abs(((a1 + 256) - a2) % 256);
  int16_t d2 = std::abs(((a2 + 256) - a1) % 256);

  return (uint8_t)std::min(d1, d2);
}

__INLINE__ void adjustAngle(uint8_t targetAngle, uint8_t currentStroke, candidate_t* candidate, jaffarPlus::games::nes::LunarBall* g, const uint16_t maxFrames)
{
  // Getting initial angle distance
  auto angleDistance = getAngleDistance(*g->_ball0Angle, targetAngle);
  // printf("%u - %u = %u\n", *g->_ball0Angle, targetAngle, getAngleDistance(*g->_ball0Angle, targetAngle));

  // Initializing frame count
  candidate->_angleFrames[currentStroke]      = 0;
  candidate->_extraAngleFrames[currentStroke] = 0;

  // If initial distance is zero, we are set
  if (angleDistance == 0) return;

  // Getting direction we need to go to get to the desired angle
  auto relativeAngle                   = ((targetAngle + _ANGLE_COUNT) - *g->_ball0Angle) % _ANGLE_COUNT;
  auto directionKey                    = g->_leftInputIdx;
  candidate->_direction[currentStroke] = -1;
  if (relativeAngle < _ANGLE_COUNT / 2)
  {
    directionKey                         = g->_rightInputIdx;
    candidate->_direction[currentStroke] = 1;
  }

  // While we are far from our target, advance by keeping the key pressed
  while (getAngleDistance(*g->_ball0Angle, targetAngle) >= 2 && candidate->_frameCount < (maxFrames - _JINGLE_TIME))
  {
    // printf("%u - %u = %u\n", *g->_ball0Angle, targetAngle, getAngleDistance(*g->_ball0Angle, targetAngle));
    // Advancing cue
    candidate->_frameCount++;
    candidate->_angleFrames[currentStroke]++;
    g->advanceState(directionKey);
  }

  // If we advanced by pressing the direction button, wait two frames before checking again
  if (candidate->_angleFrames[currentStroke] > 0)
  {
    g->advanceState(g->_nullInputIdx);
    g->advanceState(g->_nullInputIdx);
    candidate->_frameCount++;
    candidate->_frameCount++;
  }

  // printf("%u - %u = %u\n", *g->_ball0Angle, targetAngle, getAngleDistance(*g->_ball0Angle, targetAngle));

  // Special case 2: Distance is only 1
  while (getAngleDistance(*g->_ball0Angle, targetAngle) == 1 && candidate->_frameCount < (maxFrames - _JINGLE_TIME))
  {
    // Press direction key only once
    candidate->_frameCount++;
    candidate->_extraAngleFrames[currentStroke]++;
    g->advanceState(directionKey);
    g->advanceState(g->_nullInputIdx);
    g->advanceState(g->_nullInputIdx);
    candidate->_frameCount++;
    candidate->_frameCount++;
  }

  // printf("-------------------------\n");
}

int main(int argc, char* argv[])
{
  // Parsing command line arguments
  argparse::ArgumentParser program("jaffar-tester", "1.0");

  program.add_argument("configFile").help("path to the Jaffar configuration script (.jaffar) file to run.").required();

  // Try to parse arguments
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err)
  {
    JAFFAR_THROW_LOGIC("%s\n%s", err.what(), program.help().str().c_str());
  }

  // Parsing config file
  const std::string configFile = program.get<std::string>("configFile");

  // If config file defined, read it now
  std::string configFileString;
  if (jaffarCommon::file::loadStringFromFile(configFileString, configFile) == false)
    JAFFAR_THROW_LOGIC("[ERROR] Could not find or read from Jaffar config file: %s\n", configFile.c_str());

  // Parsing configuration file
  nlohmann::json config;
  try
  {
    config = nlohmann::json::parse(configFileString);
  }
  catch (const std::exception& err)
  {
    JAFFAR_THROW_LOGIC("[ERROR] Parsing configuration file %s. Details:\n%s\n", configFile.c_str(), err.what());
  }

  // Getting component configurations
  auto emulatorConfig = jaffarCommon::json::getObject(config, "Emulator Configuration");
  auto gameConfig     = jaffarCommon::json::getObject(config, "Game Configuration");
  auto runnerConfig   = jaffarCommon::json::getObject(config, "Runner Configuration");

  // Getting number of threads
  auto threadCount = jaffarCommon::parallel::getMaxThreadCount();

  // Sanity check
  if (threadCount == 0) JAFFAR_THROW_LOGIC("The number of worker threads must be at least one. Provided: %lu\n", threadCount);

  // Collection of runners for the workers to use
  std::vector<std::unique_ptr<jaffarPlus::Runner>> _runners;

  // Printing initial information
  jaffarCommon::logger::log("[J+] Using %lu worker threads.\n", threadCount);

  // Creating storage for the runnners (one per thread)
  _runners.resize(threadCount);

  // Creating runners, one per thread
  JAFFAR_PARALLEL
  {
    // Getting my thread id
    int threadId = jaffarCommon::parallel::getThreadId();

    // Creating runner from the configuration
    auto r = jaffarPlus::Runner::getRunner(emulatorConfig, gameConfig, runnerConfig);

    // Initializing runner
    r->initialize();

    // Getting internal emulator
    auto e = dynamic_cast<jaffarPlus::emulator::QuickerNES*>(r->getGame()->getEmulator());

    // Disabling rendering
    e->disableRendering();

    // Storing runner
    _runners[threadId] = std::move(r);
  }

  // Getting base runner
  auto r = _runners[0].get();

  // Getting game pointer
  auto g = dynamic_cast<jaffarPlus::games::nes::LunarBall*>(r->getGame());

  // Getting state size
  jaffarCommon::serializer::Contiguous sizeEvaluator;
  g->serializeState(sizeEvaluator);
  size_t stateSize = sizeEvaluator.getOutputSize();
  if (stateSize != _STATE_SIZE)
  {
    fprintf(stderr, "State Size mismatch: %lu vs %lu\n", stateSize, _STATE_SIZE);
    exit(-1);
  }
  jaffarCommon::logger::log("[J+] State Size: %lu\n", stateSize);

  ///////// Creating free candidate database

  // Calculating max candidate
  size_t _maxCandidates            = 1; // Initial candidate
  size_t _speedIncrementMultiplier = 1;
#ifdef _CONSIDER_SPEED_INCREMENT
  _speedIncrementMultiplier = 2;
#endif
  _maxCandidates += _MAX_NEW_CANDIDATES_PER_STROKE;                                        // The states that survive through next iteration
  _maxCandidates += _MAX_NEW_CANDIDATES_PER_STROKE * 256 * 78 * _speedIncrementMultiplier; // The states that survive through next iteration

  // Calculating max state size
  size_t candidateSize = sizeof(candidate_t);
  size_t _maxSize      = _maxCandidates * candidateSize;
  jaffarCommon::logger::log("[J+]  + State Database                  Max States: %lu, Size: %.3f Mb (%.6f Gb)\n", _maxCandidates, (double)_maxSize / (1024.0 * 1024.0),
                            (double)_maxSize / (1024.0 * 1024.0 * 1024.0));

  // Creating candidate database
  auto _freeCandidateQueue = std::make_unique<jaffarCommon::concurrent::atomicQueue_t<void*>>(_maxCandidates);

  // Initializing free candidate db
  jaffarCommon::logger::log("[J+] Initializing Candidate State Db...\n");

  // Getting system's page size (typically 4K but it may change in the future)
  const size_t pageSize = sysconf(_SC_PAGESIZE);

  // Allocating space for the candidate
  uint8_t* _internalBuffer;
  auto     status = posix_memalign((void**)&_internalBuffer, pageSize, _maxSize);
  if (status != 0) JAFFAR_THROW_RUNTIME("Could not allocate aligned memory for the state database");

  // Doing first touch for every page
  JAFFAR_PARALLEL_FOR
  for (size_t i = 0; i < _maxSize; i += pageSize) _internalBuffer[i] = 1;

  // Adding the candidate pointers to the free candidate queue
  for (size_t i = 0; i < _maxCandidates; i++) returnFreeCandidateMove(_freeCandidateQueue.get(), (candidate_t*)&_internalBuffer[i * candidateSize]);

  ///// Setting initial metadata

  // Getting current stage
  auto currentStage = *g->_currentStage;
  if (currentStage == 60)
    if (currentStage != _CURRENT_STAGE)
    {
      fprintf(stderr, "Wrong stage selected %u vs %u\n", currentStage, _CURRENT_STAGE);
      exit(1);
    }

  // Candidate Move queues
  std::multimap<float, candidate_t*, std::greater<float>> _nextCandidateMoves;
  std::mutex                                              _nextCandidateMovesMutex;
  jaffarCommon::concurrent::atomicQueue_t<candidate_t*>   _currentCandidateMoves(_maxCandidates);

  // Win candidate information
  std::mutex   _bestWinCandidateMutex;
  candidate_t* _bestWinCandidate   = nullptr;
  size_t       _winCandidatesFound = 0;

  // Stroke counter / index
  uint8_t currentStroke = 0;

  // Statistics
  bool                isFinished                = false;
  std::atomic<size_t> totalProcessedCandidates  = 0;
  std::atomic<size_t> strokeProcessedCandidates = 0;
  size_t              totalCandidates           = 0;
  size_t              repeatedStates            = 0;

  // Storing max frames
  uint32_t _maxFrames = _INITIAL_MAX_FRAMES;

  // Taking initial time
  auto t0 = jaffarCommon::timing::now();

  // Taking stroke time
  auto ts = jaffarCommon::timing::now();

  ///////// Creating reporter thread
  auto infoThread = std::thread(
      [&]()
      {
        while (isFinished == 0)
        {
          // Elapsed step time
          auto totalElapsedTime  = jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), t0);
          auto strokeElapsedTime = jaffarCommon::timing::timeDeltaMicroseconds(jaffarCommon::timing::now(), ts);

          // Printing information
          jaffarCommon::logger::log("[J+] Current Stroke:              %u / %u\n", currentStroke + 1, _MAX_STROKES);
          jaffarCommon::logger::log("[J+] Performance (Total):         %8.3f candidates/s\n", (double)totalProcessedCandidates.load() / (1.0e-6 * (double)(totalElapsedTime)));
          jaffarCommon::logger::log("[J+] Stroke Elapsed Time:         %.3fs / Total: %.3fs\n", 1.0e-6 * (double)(strokeElapsedTime), 1.0e-6 * (double)(totalElapsedTime));
          jaffarCommon::logger::log("[J+] Processed Candidates:        %lu / Total: %lu\n", strokeProcessedCandidates.load(), totalProcessedCandidates.load());

          jaffarCommon::logger::log("[J+] Remaining Candidates:        %lu / %lu\n", _currentCandidateMoves.was_size(), totalCandidates);
          jaffarCommon::logger::log("[J+] Next Candidates:             %lu\n", _nextCandidateMoves.size());

#ifdef _CHECK_COLLISIONS
          jaffarCommon::logger::log("[J+] Repeated States:             %lu\n", repeatedStates);
#endif

          jaffarCommon::logger::log("[J+] Win Candidates Count:        %lu\n", _winCandidatesFound);

          _bestWinCandidateMutex.lock();
          if (_bestWinCandidate != nullptr)
          {
            jaffarCommon::logger::log("[J+] Best Win State:\n");
            printCandidateInfo(_bestWinCandidate);
          }
          _bestWinCandidateMutex.unlock();

          jaffarCommon::logger::log("[J+] ---------------------------------\n");

          // Sleeping between reports
          sleep(10);
        }
      });

  ///////////// Creating initial candidate
  auto initialCandidate           = getFreeCandidateMove(_freeCandidateQueue.get());
  initialCandidate->_pendingBalls = g->_pendingBalls;
  initialCandidate->_frameCount   = 0;

  // Getting the state on the first candidate
  serializeState(g, initialCandidate);
  _nextCandidateMoves.insert({0.0f, initialCandidate});

  //////////// Main Cycle: Running strokes, one after the other
  for (; currentStroke < _MAX_STROKES; currentStroke++)
  // if (_winCandidatesFound == 0) // Used for min strokes
  {
    // Creating Hash Store for this step
    jaffarCommon::concurrent::HashSet_t<jaffarCommon::hash::hash_t> hashSet = {};

    // Resetting timer and counters
    ts                        = jaffarCommon::timing::now();
    strokeProcessedCandidates = 0;

    ///// Generating candidate actions
    jaffarCommon::logger::log("[J+] Creating candidate moves for stroke %lu...\n", currentStroke);

    // Generating new states
    size_t currentCandidateCounter = 0;
    while (_nextCandidateMoves.begin() != _nextCandidateMoves.end())
    {
      // Extracting next state
      auto srcCandidateItr    = _nextCandidateMoves.begin();
      auto srcCandidateReward = srcCandidateItr->first;
      auto srcCandidatePtr    = srcCandidateItr->second;
      auto srcCandidateEntry  = _nextCandidateMoves.extract(srcCandidateItr);

      jaffarCommon::logger::log("[J+] Processing entry %lu / %u with reward: %f - Remaining Balls: %u - Frame Count: %u\n", currentCandidateCounter, _MAX_NEW_CANDIDATES_PER_STROKE,
                                srcCandidateReward, srcCandidatePtr->_pendingBalls, srcCandidatePtr->_frameCount);
      for (size_t i = 0; i < 256; i++)
        for (size_t j = 255; j >= 24; j -= 3) // For full power exploration
          if (j > 247 || j == 141 || j == 201)
#ifdef _CONSIDER_SPEED_INCREMENT
            for (size_t inc : std::vector<size_t>({253, 3}))
#endif
            {
              auto dstCandidate = getFreeCandidateMove(_freeCandidateQueue.get());
              *dstCandidate     = *srcCandidatePtr;

#ifdef _CONSIDER_SPEED_INCREMENT
              dstCandidate->_speedIncrement[currentStroke] = inc;
#endif

              dstCandidate->_angle[currentStroke] = i;
              dstCandidate->_speed[currentStroke] = j;
              dstCandidate->_strokeCount          = currentStroke + 1;

              dstCandidate->_angleFrames[currentStroke]     = 0;
              dstCandidate->_speedFrames[currentStroke]     = 0;
              dstCandidate->_playFrames[currentStroke]      = 0;
              dstCandidate->transitionFrames[currentStroke] = 0;

              _currentCandidateMoves.push(dstCandidate);
            }

      // Freeing source state
      returnFreeCandidateMove(_freeCandidateQueue.get(), srcCandidatePtr);

      // Increasing candidate counter
      currentCandidateCounter++;
    }

    // Clearing next candidate move storage
    _nextCandidateMoves.clear();

    // Statistics
    totalCandidates = _currentCandidateMoves.was_size();
    jaffarCommon::logger::log("[J+] Created %lu candidate moves...\n", totalCandidates);

    // Running main loop
    JAFFAR_PARALLEL
    {
      // Getting my thread id
      int threadId = jaffarCommon::parallel::getThreadId();

      // Getting thread's own runner
      auto& r = _runners[threadId];
      auto  g = dynamic_cast<jaffarPlus::games::nes::LunarBall*>(r->getGame());

      // Current candidate to handle
      candidate_t* candidate;

      // Iterate while there are candidates left
      while (_currentCandidateMoves.try_pop(candidate) == true)
      {
        // printf("Candidate - Angle: %u, Power: %u\n", candidate->_angle, candidate->_speed);

        // Loading candidate state into the game
        deserializeState(g, candidate);

        // Remembering the initial pending balls
        auto initialPendingBalls = g->_pendingBalls;

// Setting up required parameters
#ifdef _CONSIDER_SPEED_INCREMENT
        const auto targetSpeedIncrement = candidate->_speedIncrement[currentStroke];
#endif

        const auto targetAngle = candidate->_angle[currentStroke];
        const auto targetSpeed = candidate->_speed[currentStroke];

        // Getting appropriate initial angle
        adjustAngle(targetAngle, currentStroke, candidate, g, _maxFrames);

        // Waiting for correct power to arrive
        candidate->_speedFrames[currentStroke] = 0;
        while ((*g->_ball0Speed != targetSpeed
#ifdef _CONSIDER_SPEED_INCREMENT
                || *g->_speedIncrement != targetSpeedIncrement
#endif
                ) &&
               candidate->_frameCount < (_maxFrames - _JINGLE_TIME))
        {
          candidate->_frameCount++;
          candidate->_speedFrames[currentStroke]++;
          g->advanceState(g->_nullInputIdx);
        }

        // Making sure we got the right parameters
        if (candidate->_frameCount < (_maxFrames - _JINGLE_TIME))
        {
#ifdef _CONSIDER_SPEED_INCREMENT
          if (*g->_speedIncrement != targetSpeedIncrement) JAFFAR_THROW_RUNTIME("Wrong speed increment");
#endif
          if (*g->_ball0Angle != targetAngle)
          {
            fprintf(stderr, "Wrong Angle %u vs %u\n", *g->_ball0Angle, targetAngle);
            JAFFAR_THROW_RUNTIME("Wrong angle");
          }
          if (*g->_ball0Speed != targetSpeed) JAFFAR_THROW_RUNTIME("Wrong speed");

          // Launching the ball (pressing B button until state changes)
          g->advanceState(g->_buttonInputIdx);
          candidate->_frameCount++;
          g->advanceState(g->_buttonInputIdx);
          candidate->_frameCount++;
        }

        // Remember frame count up to this point (useful for last input on stage 60)
        auto afterButtonStepCount = candidate->_frameCount;

        // Keeping track of candidate status
        size_t                     currentFrame   = 0;
        bool                       isFinite       = false;
        bool                       collisionFound = false;
        jaffarCommon::hash::hash_t hash{};
        candidate->_playFrames[currentStroke] = 0;

        // Advancing until finalization criteria is met
        while (collisionFound == false && candidate->_frameCount < (_maxFrames - _JINGLE_TIME))
        {
          // Advancing game
          g->advanceState(g->_nullInputIdx);
          currentFrame++;
          candidate->_playFrames[currentStroke]++;
          candidate->_frameCount++;

#ifdef _CHECK_COLLISIONS
          // Check only the first few movements
          if (currentFrame < 5)
          {
            // Calculating hash
            hash = r->computeHash();

            // Checking for collisions
            collisionFound = hashSet.insert(hash).second == false;
          }
#endif

          // If the number of frames is multiple of a high number, print it for debugging reasons
          if (currentFrame % 50000 == 0)
          {
            auto hashString = jaffarCommon::hash::hashToString(hash);
            jaffarCommon::logger::log("[J+] Current Frame: %lu - Angle: %u, Power: %u, Hash: %s\n", currentFrame, candidate->_angle[currentStroke],
                                      candidate->_speed[currentStroke], hashString.c_str());
          }

          // Termination Criteria: Cue ball enters a pocket
          if (*g->_ball0State == jaffarPlus::games::nes::LunarBall::ballState_t::entering || *g->_ball0State == jaffarPlus::games::nes::LunarBall::ballState_t::entered) break;

          // // Bug prevention: if stage is different than the current before balls stopped moving, it's a glitched state
          // if (*g->_currentStage != currentStage) break;

          // // Bug prevention: if first palette byte changes, it's a glitched state.
          // if (*g->_firstPaletteByte != 0x3F) break;

          // Moving balls is zero, play has finished
          if (g->_movingBalls == 0)
          {
            isFinite = true;
            break;
          }
        }

        // If cue ball was inserted, reset pending balls now
        if (*g->_ball0State == jaffarPlus::games::nes::LunarBall::ballState_t::entering || *g->_ball0State == jaffarPlus::games::nes::LunarBall::ballState_t::entered)
          g->_pendingBalls = initialPendingBalls;

        // Flag to remember the candidate was stored (otherwise free its memory)
        bool candidateWasStored = false;

// Check collisions once again
#ifdef _CHECK_COLLISIONS
        // Calculating hash
        hash = r->computeHash();

        // Checking for collisions
        collisionFound = hashSet.insert(hash).second == false;
#endif

        // If we found a collision, add counter
        if (collisionFound == true) repeatedStates++;

        // Storing scored balls
        candidate->_scoredBalls[currentStroke] = initialPendingBalls - g->_pendingBalls;

        // Ball pocketing heuristic flag
        bool correctPocketing = true;

        // // Heuristic: in stroke 0 we do not pocket any balls to reset rate
        // if (currentStroke == 0 && candidate->_scoredBalls[0] > 0) correctPocketing = false;

        // // Heuristic: in stroke 1 we pocket at least one ball
        // if (currentStroke == 1 && candidate->_scoredBalls[1] == 0) correctPocketing = false;

        // Heuristic: for next steps, do not fail to pocket twice in a row
        // if (currentStroke > 1 && candidate->_scoredBalls[currentStroke - 1] == 0 && candidate->_scoredBalls[currentStroke] == 0) correctPocketing = false;

        // Deadlock Prevention: If scoring 7 balls in level 60, its a break
        if (candidate->_scoredBalls[currentStroke] == 7) correctPocketing = false;

        // If valid, add it to the next stroke candidate list
        if (isFinite == true && collisionFound == false && correctPocketing == true)
        {
          // auto hashString = jaffarCommon::hash::hashToString(hash);
          // jaffarCommon::logger::log("[J+] Successful Candidate Angle: %u, Power: %u, Pending Balls: %u -> %u, Frames: %lu, Ball 0 State: %u - Hash: %s\n",
          // candidate->_angle[stroke], candidate->_speed[stroke], candidate->_pendingBalls, g->_pendingBalls, candidate->_frameCount, *g->_ball0State, hashString.c_str());

          // Set new candidate pending balls
          candidate->_pendingBalls = g->_pendingBalls;
          // printf("Scored balls: %u / Pending Balls: %u / Initial Pending Balls: %u\n",  candidate->_scoredBalls[0], candidate->_pendingBalls, initialPendingBalls);

          // Set new bonus rate
          candidate->_rate = *g->_rate;

          // Push back into the next candidate move list, if there are balls remaining
          if (candidate->_pendingBalls > 0)
          {
            // Advancing until next inactive state
            while (*g->_ball0State != jaffarPlus::games::nes::LunarBall::ballState_t::inactive && candidate->_frameCount < (_maxFrames - _JINGLE_TIME))
            {
              g->advanceState(g->_nullInputIdx);
              candidate->_frameCount++;
              candidate->transitionFrames[currentStroke]++;
            }

            // Advance once more
            g->advanceState(g->_nullInputIdx);
            candidate->_frameCount++;
            candidate->transitionFrames[currentStroke]++;

            // Storing next candidate, only if does not exceed max frames
            if (candidate->_frameCount < (_maxFrames - _JINGLE_TIME))
            {
              // Calculating reward function
              float reward = 0.0f;

              // Punishing pending balls
              reward -= candidate->_pendingBalls * 500.0f;

              // Punishing rate balls
              reward -= candidate->_rate * 10.0f;

              // Punishing incurred frames
              reward -= candidate->_frameCount;

              // Flag to indicate whether we will add an entry
              bool addEntry = false;

              // Pushing Candidate, if it is worth adding it
              _nextCandidateMovesMutex.lock();

              // Check if we reached the entry limit
              const size_t entryCount = _nextCandidateMoves.size();

              // If not, simply add the entry
              if (entryCount < _MAX_NEW_CANDIDATES_PER_STROKE) addEntry = true;

              // If we did reach the limit, check whether we replace the worst one
              else
              {
                // Checking worst entry
                const auto& worstEntryItr    = _nextCandidateMoves.rbegin();
                const auto  worstEntryReward = worstEntryItr->first;
                const auto  worstEntryPtr    = worstEntryItr->second;

                // If yes, remove the worst entry
                if (reward > worstEntryReward)
                {
                  returnFreeCandidateMove(_freeCandidateQueue.get(), worstEntryPtr);
                  _nextCandidateMoves.erase(std::prev(_nextCandidateMoves.end()));
                  addEntry = true;
                }
              }

              // If adding new entry, do it now
              if (addEntry == true)
              {
                _nextCandidateMoves.insert({reward, candidate});
                candidateWasStored = true;
              }

              _nextCandidateMovesMutex.unlock();
            }
          }

          // If no pending balls, advance until level changes
          if (candidate->_pendingBalls == 0)
          {
            if (currentStage != 60) // If this is not the last stage, do not advance like this
              while (*g->_currentStage == currentStage && candidate->_frameCount < _maxFrames)
              {
                g->advanceState(g->_nullInputIdx);
                candidate->transitionFrames[currentStroke]++;
                candidate->_frameCount++;
              }

            // If we didn't exceed the maximum frames
            if (candidate->_frameCount < _maxFrames)
            {
              // Get best win frame lock
              _bestWinCandidateMutex.lock();

              // We found a new win candidate
              _winCandidatesFound++;

              // Storing total frame count of the candidate
              auto endFrameCount = candidate->_frameCount;

              // Special case for lvl60, we store current frame count as max, but evaluate the candidate based on the frames at last input
              if (currentStage == 60) candidate->_frameCount = afterButtonStepCount;

              // If this is a better win candidate or if one hasn't been found yet, replace the current best one
              bool saveNewBest = false;
              if (_bestWinCandidate == nullptr) saveNewBest = true;
              if (_bestWinCandidate != nullptr && candidate->_frameCount < _bestWinCandidate->_frameCount) saveNewBest = true;
              if (saveNewBest == true)
              {
                // Setting new max frames (no candidate must surpass the current best)
                _maxFrames = endFrameCount;

                // Temporal storage for old best candidate
                auto oldBestCandidate = _bestWinCandidate;

                // Storing new one
                _bestWinCandidate = candidate;

                // Freeing up previous, if defined
                if (oldBestCandidate != nullptr) returnFreeCandidateMove(_freeCandidateQueue.get(), oldBestCandidate);

                // Saving it to a file
                saveBestCandidate(_bestWinCandidate);

                // Setting it as stored
                candidateWasStored = true;
              }

              // Release best win frame lock
              _bestWinCandidateMutex.unlock();
            }
          }
        }

        // If the candidate was stored, then save its game state, otherwise free it up
        if (candidateWasStored == true) serializeState(g, candidate);
        if (candidateWasStored == false) returnFreeCandidateMove(_freeCandidateQueue.get(), candidate);

        // Keeping statistics
        totalProcessedCandidates++;
        strokeProcessedCandidates++;
      }
    }
  }

  // Notify if no best candidate was found
  if (_bestWinCandidate != nullptr) printCandidateInfo(_bestWinCandidate);
  if (_bestWinCandidate == nullptr) jaffarCommon::logger::log("[J+] Did not find any win candidates\n");

  // Finalizing information thread
  isFinished = true;

  // Waiting on information thread
  infoThread.join();
}
