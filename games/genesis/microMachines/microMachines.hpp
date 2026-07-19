#pragma once

#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <vector>

namespace jaffarPlus
{

namespace games
{

namespace genesis
{

class MicroMachines final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "Genesis / Micro Machines"; }

  MicroMachines(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Parsing the reference-line waypoints from the game config (NOT hardcoded, so a different race can be run
    // without recompiling). "Reference Projection" holds the reference solution's sampled path (per-point X,Y) --
    // used as an ORDERED chain of waypoints (see ruleUpdatePostHook). "Waypoint Reach Radius" is how close (in
    // position units) the car must come to a waypoint for it to count as reached. Parsed here in the constructor
    // (not registerGameProperties) because finalizeGameConfig() validates leftovers earlier.
    // "Reference Projection" (waypoint line) is OPTIONAL: omit it for a from-scratch search that rewards the
    // game's own checkpoint progress instead of following a reference line (see the Checkpoint Progress magnet).
    if (_gameConfigRemaining.contains("Reference Projection"))
    {
      const auto  refProj = jaffarCommon::json::popObject(_gameConfigRemaining, "Reference Projection");
      const float reachR  = jaffarCommon::json::getNumber<float>(refProj, "Waypoint Reach Radius");
      _wpReachRadius2     = reachR * reachR;
      // How many waypoints ahead of the furthest-reached one to scan for reachability. >1 allows NON-SEQUENTIAL
      // traversal: reaching a later waypoint credits it even if intermediate ones were bypassed (shortcut discovery).
      // Bounded so a jump can't latch onto a spatially-near but index-distant waypoint where the track loops back.
      _wpLookahead = jaffarCommon::json::getNumber<int>(refProj, "Waypoint Lookahead");
      if (_wpLookahead < 1) JAFFAR_THROW_LOGIC("'Waypoint Lookahead' must be >= 1, got %d", _wpLookahead);
      const auto& pathJs = jaffarCommon::json::getArray<nlohmann::json>(refProj, "Path");
      for (size_t i = 0; i < pathJs.size(); i++)
      {
        _refPathX.push_back(jaffarCommon::json::getNumber<float>(pathJs[i], "X"));
        _refPathY.push_back(jaffarCommon::json::getNumber<float>(pathJs[i], "Y"));
      }
      // Cumulative arc length to each waypoint (used by the single-magnet, fully-continuous arc-progress reward:
      // with per-frame-dense waypoints, "distance travelled along the reference line" IS the reward -- see
      // ruleUpdatePostHook / _trackArcMagnet).
      float arc = 0.0f;
      for (size_t i = 0; i < _refPathX.size(); i++)
      {
        if (i > 0)
        {
          const float dx = _refPathX[i] - _refPathX[i - 1], dy = _refPathY[i] - _refPathY[i - 1];
          arc += std::sqrt(dx * dx + dy * dy);
        }
        _refPathArc.push_back(arc);
      }
      _refPathTotalArc = arc;
    }

    // ---- Checkpoint Guide: progress judged by the GAME's own checkpoint counter, not by our geometry ----
    // The reference-line waypoint reward is gameable: progress is credited when the car comes within a reach
    // radius of a point on the reference's line, so the car can claim waypoints while drifting off the road
    // (radius too big) or freeze when a legitimate line sits just outside it (radius too small). No radius value
    // works for both -- measured: r32/d50 died at step 358, r20/d250 at 2087, r32/d250 at 627, r20/d1000 at 500.
    // Instead, credit progress ONLY when the game's own track-position counter (0xA69C) reaches the next
    // checkpoint the REFERENCE actually visited. That is un-gameable (the game decides), route-correct (a
    // wrong-way or off-route car never advances the list), and geometry-free (the car may take ANY line between
    // checkpoints). The position of each checkpoint is used solely as a soft magnet to STEER toward the next one.
    // "Checkpoints" = ordered [{Id, X, Y}] captured from the reference's first arrival at each checkpoint id.
    if (_gameConfigRemaining.contains("Checkpoint Guide"))
    {
      const auto  guide = jaffarCommon::json::popObject(_gameConfigRemaining, "Checkpoint Guide");
      const auto& pts   = jaffarCommon::json::getArray<nlohmann::json>(guide, "Checkpoints");
      for (size_t i = 0; i < pts.size(); i++)
      {
        _cpGuideId.push_back(jaffarCommon::json::getNumber<int>(pts[i], "Id"));
        _cpGuideX.push_back(jaffarCommon::json::getNumber<float>(pts[i], "X"));
        _cpGuideY.push_back(jaffarCommon::json::getNumber<float>(pts[i], "Y"));
      }
    }

    // Sub-pixel precision of the player-position dedup hash: number of fraction bits of the 16.16 position kept
    // when hashing (0 = whole pixel / coarsest merging, 8 = 1/256px, 16 = full sub-pixel / finest, largest state
    // space). Finer keeps genuinely-distinct states (e.g. the fast racing line) apart at the cost of a bigger
    // frontier. See computeAdditionalHashing.
    if (_gameConfigRemaining.contains("Coverage Magnet")) _coverageMagnet = jaffarCommon::json::popNumber<float>(_gameConfigRemaining, "Coverage Magnet");
    _positionHashGranularity = jaffarCommon::json::popNumber<uint32_t>(_gameConfigRemaining, "Position Hash Granularity");
    if (_positionHashGranularity > 16) JAFFAR_THROW_LOGIC("'Position Hash Granularity' must be 0..16, got %u", _positionHashGranularity);

    // Sub-unit precision of the car's 16.16 velocity vector in the dedup hash (fraction bits kept). Velocity is
    // the turning-momentum DOF; too coarse merges primed-to-turn with going-straight, too fine bloats the frontier.
    _velocityHashGranularity = jaffarCommon::json::popNumber<uint32_t>(_gameConfigRemaining, "Velocity Hash Granularity");
    if (_velocityHashGranularity > 16) JAFFAR_THROW_LOGIC("'Velocity Hash Granularity' must be 0..16, got %u", _velocityHashGranularity);
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _workRAM = _emulator->getProperty("RAM").pointer;

    // Registering native game properties
    registerGameProperty("Current Race", &_workRAM[0x8080], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos X", &_workRAM[0x80AE], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Camera Pos Y", &_workRAM[0x80B0], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos X", &_workRAM[0x80A6], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos Y", &_workRAM[0x80A8], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Vel X", &_workRAM[0xA65A], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Vel Y", &_workRAM[0xA680], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Angle", &_workRAM[0xA656], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Pos Z", &_workRAM[0xA64C], Property::datatype_t::dt_int16, Property::endianness_t::little);
    registerGameProperty("Player 1 Vel Z", &_workRAM[0xA64E], Property::datatype_t::dt_int16, Property::endianness_t::little);
    // LAP ADDRESS FIX: the TRUE laps-remaining counter is 0xA698 (4 -> 0 across the race; laps-done is 0xA69A).
    // 0xA69E -- which this property was previously (wrongly) bound to -- is a FINISH FLAG (0 -> 1 on the win
    // frame only). The old binding made win rules fire "by accident" (finish flag == 1) and would break any
    // lap-progress reward (e.g. NES-style lap bonuses or a lap-skip search, which need the real counter).
    registerGameProperty("Player 1 Current Laps Remaining", &_workRAM[0xA698], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Laps Done", &_workRAM[0xA69A], Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Race Finished", &_workRAM[0xA69E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Checkpoint 1", &_workRAM[0xA6DE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Checkpoint 2", &_workRAM[0xA6E1], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Offroad Checkpoint", &_workRAM[0xA6E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    // Dense, position-driven track-progress index (advances monotonically along the racing line; stays put while the car is stalled).
    registerGameProperty("Player 1 Track Position", &_workRAM[0xA69C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    // Fall/recovery state machine (0 = driving; 1 = falling off the track/table; 4 = vacuum/replace animation).
    // Zero for the ENTIRE clean 2219 record; nonzero during every observed fall and punishment teleport. Rules
    // fail on it immediately so doomed lineages die at the fall frame instead of zombie-evicting honest states
    // for the ~50-frame recovery duration (the cause of three consecutive out-of-states frontier collapses).
    registerGameProperty("Player 1 Fall State", &_workRAM[0xA66C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Recovery Mode", &_workRAM[0xA672], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Recovery Timer", &_workRAM[0xA670], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pre-Race Timer", &_workRAM[0x8000], Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Registering derivative game properties
    registerGameProperty("Player 1 Previous Laps Remaining", &_player1LapsRemainingPrev, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Player 1 Previous Checkpoint", &_player1CheckpointPrev, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    // Single-step DROP of the cumulative track-position counter (0xA69C): 0 while racing forward; >0 when
    // the car re-crosses a zone boundary backward (1) or gets PUNISHED (reset of up to a lap, e.g. 30).
    // Lets rules prune punished/backward states: e.g. { "Player 1 Track Position Drop", ">", 1 } -> Fail.
    registerGameProperty("Player 1 Track Position Drop", &_trackPosDrop, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player 1 Honey Bog", &_honeyBog, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Current Step", &_currentStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Last Input Step", &_lastInputStep, Property::datatype_t::dt_uint16, Property::endianness_t::little);

    // Getting some properties' pointers now for quick access later
    _currentRace = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Race")]->getPointer();
    _cameraPosX  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Camera Pos X")]->getPointer();
    _cameraPosY  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Camera Pos Y")]->getPointer();

    _player1PosX = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos X")]->getPointer();
    _player1PosY = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Pos Y")]->getPointer();

    _player1VelX  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Vel X")]->getPointer();
    _player1VelY  = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Vel Y")]->getPointer();
    _player1Angle = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Angle")]->getPointer();

    _player1LapsRemaining     = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Current Laps Remaining")]->getPointer();
    _player1LapsDone          = (uint16_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Laps Done")]->getPointer();
    _player1Checkpoint1       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Checkpoint 1")]->getPointer();
    _player1Checkpoint2       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Checkpoint 2")]->getPointer();
    _player1OffroadCheckpoint = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Offroad Checkpoint")]->getPointer();
    _player1TrackPosition     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Track Position")]->getPointer();

    _player1RecoveryMode  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Recovery Mode")]->getPointer();
    _player1RecoveryTimer = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player 1 Recovery Timer")]->getPointer();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  // One search "step" advances the chosen input for a fixed number of emulator frames. Set to 1
  // for full per-frame granularity (one emulator step per Jaffar step). Larger values hold the
  // chosen input across a block, trading control granularity for a smaller search space (the
  // chosen input -- not a forced accelerate -- so coast/brake remains available either way).
  static constexpr size_t _framesPerStep = 1;

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _player1LapsRemainingPrev = *_player1LapsRemaining;
    _trackPosPrev             = _workRAM[0xA69C];
    _player1CheckpointPrev    = *_player1Checkpoint1;

    for (size_t i = 0; i < _framesPerStep; i++) _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;

    _currentStep++;
  }

  // True when the player is strictly ahead of every CPU opponent, judged by the game's own per-racer track
  // progress: lap (0xA69E) * 1000 + checkpoint (0xA69C), racer structs 0xEA apart. Used to drop opponent
  // positions from the dedup hash once we've passed them all (see computeAdditionalHashing) and printed each step.
  __INLINE__ bool isPlayerInFirst() const
  {
    // 0xA69C is the CUMULATIVE track-progress counter (lap * lapLen + zone; e.g. 36 -> 145 over race 1), so it
    // already encodes laps -- compare it directly. (Previously multiplied 0xA69E here as "lap", but 0xA69E is
    // the finish flag, not a lap counter -- see the lap-address fix in registerGameProperties.)
    const int             playerProg   = (int)_workRAM[0xA69C];
    static const uint32_t oppCpBase[3] = {0xA786u, 0xA870u, 0xA95Au}; // opponent cumulative-progress bytes (stride 0xEA)
    for (const uint32_t cpb : oppCpBase)
    {
      const int oppProg = (int)_workRAM[cpb];  // cumulative, same scale as the player's 0xA69C
      if (oppProg >= playerProg) return false; // an opponent is level or ahead -> not first
    }
    return true;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(_workRAM[0xA65A]);
    hashEngine.Update(_workRAM[0xA6DA]);

    // Player position at quantized sub-pixel resolution (hash-fidelity audit): the coarse integer-pixel hash
    // merged states differing only in the 16.16 sub-pixel, which then diverge -- so faster racing lines were
    // silently pruned. Hashing full sub-pixel restores the distinction but explodes the state count; we instead
    // keep only _positionHashGranularity fraction bits (0 = whole pixel, 8 = 1/256px, 16 = full sub-pixel) via a
    // right-shift, trading dedup vs fidelity ("Position Hash Granularity" config key).
    const uint32_t posX1616 = _workRAM[0xA60A] | (_workRAM[0xA60B] << 8) | (_workRAM[0xA60C] << 16) | (_workRAM[0xA60D] << 24);
    const uint32_t posY1616 = _workRAM[0xA60E] | (_workRAM[0xA60F] << 8) | (_workRAM[0xA610] << 16) | (_workRAM[0xA611] << 24);
    const uint32_t shift    = 16u - _positionHashGranularity; // fraction bits kept -> bits to discard
    hashEngine.Update(posX1616 >> shift);
    hashEngine.Update(posY1616 >> shift);

    // Real car VELOCITY vector (signed 16.16) at 0xA612 (X) / 0xA616 (Y). A RAM-diff of the reference confirmed
    // these track the actual per-frame motion (corr >0.95) -- they are the car's turning momentum, NOT the
    // "position-history ring buffer" previously assumed (that wrong assumption excluded 0xA612-0xA633 from the
    // hash). Leaving velocity out merged states with identical position but different heading/momentum -- e.g. a
    // state swinging into a corner vs coasting straight -- so the search kept only one and could not represent the
    // turn. We hash it quantized to _velocityHashGranularity fraction bits (arithmetic shift keeps the sign).
    const int32_t  velX1616 = (int32_t)(_workRAM[0xA612] | (_workRAM[0xA613] << 8) | (_workRAM[0xA614] << 16) | (_workRAM[0xA615] << 24));
    const int32_t  velY1616 = (int32_t)(_workRAM[0xA616] | (_workRAM[0xA617] << 8) | (_workRAM[0xA618] << 16) | (_workRAM[0xA619] << 24));
    const uint32_t vshift   = 16u - _velocityHashGranularity;
    hashEngine.Update(velX1616 >> vshift);
    hashEngine.Update(velY1616 >> vshift);

    // Vertical (Z) state: the car MINIJUMPS over the track's "fruity loop" borders -- e.g. crossing the start
    // line (frame 57 of the first search solution: 0xA64C arcs 0 -> -32 -> -50 -> -54 -> -44 -> -20 -> 0, and
    // immediately bounces a second time). 0xA64C = Z height (int16; 0 = grounded, negative = airborne),
    // 0xA64E = Z velocity (int16; ramps a constant +14/frame = gravity). Leaving these out hashed an AIRBORNE
    // car identically to a GROUNDED one at the same X/Y and horizontal velocity, so the two merged and the
    // search could not represent a jump at all -- and velZ, which determines the whole remaining arc, was
    // invisible. Same class of bug as the velX/velY omission above. Both are 0 while grounded, so this adds no
    // frontier cost outside actual jumps; the values are small ints (~-54..38), so we hash them at full
    // resolution. (Opponents' Z live at the same offsets + the 0xEA racer stride: 0xA736/0xA820/0xA90A.)
    const int16_t posZ = (int16_t)(_workRAM[0xA64C] | (_workRAM[0xA64D] << 8));
    const int16_t velZ = (int16_t)(_workRAM[0xA64E] | (_workRAM[0xA64F] << 8));
    hashEngine.Update(posZ);
    hashEngine.Update(velZ);

    // FINE HEADING (0xA650-51, 16-bit angle mod 2048): the car's facing, which L/R rotate ~114 units / 10
    // frames -- INCLUDING IN THE AIR, where the velocity vector stays ballistic and the hashed coarse angle
    // (0xA656, 8 directions) never ticks. Without this, airborne states with different headings (straight vs
    // steered) hash identically and MERGE, so whichever input expands first dominates every jump -- observed
    // as the search always air-steering L+B through the frame-265..327 platform jump, killing momentum on
    // landing. Quantized >> 6 (32 buckets / circle, ~11 deg) to keep the frontier cost modest: on the ground
    // heading is mostly implied by the velocity vector; the buckets matter exactly where they should (air,
    // drift). Also 0xA648 (small steering-state byte 0..3) raw.
    const uint16_t fineAngle = (uint16_t)(_workRAM[0xA650] | (_workRAM[0xA651] << 8));
    hashEngine.Update((uint16_t)(fineAngle >> 6));
    hashEngine.Update(_workRAM[0xA648]);

    // Opponent racer positions (the 3 CPU boats), quantized to WHOLE pixels -- but ONLY while they can still
    // affect us. Once the player is strictly in FIRST place it has passed every opponent; they are behind and can
    // no longer block/collide, so we drop them from the hash to avoid frontier bloat (typically ~80% of frames).
    // "First" is judged by the game's own per-racer track progress: lap (0xA69E) * 1000 + checkpoint (0xA69C),
    // racer structs 0xEA apart. Each racer is a 0xEA-byte struct (player 16.16 pos 0xA60A; opponents at +0xEA,
    // +0x1D4, +0x2BE); whole-pixel quantization keeps their positions from bloating the frontier with sub-pixel jitter.
    // NOTE: opponent positions DISABLED in the hash (race01 experiment) -- hashing 3 opponents' pixel
    // positions while the player isn't in first (most of the race) multiplies the frontier by every
    // distinct opponent configuration, causing frontier explosion over the long 2611-frame race. Drop
    // them: states that differ only in opponent positions now merge. Re-enable if opponents prove to
    // matter for the racing line.
    // if (isPlayerInFirst() == false)
    // {
    //   static const uint32_t oppPosBase[3] = {0xA6F4u, 0xA7DEu, 0xA8C8u};
    //   for (const uint32_t base : oppPosBase)
    //   {
    //     const int32_t ox = (int32_t)(_workRAM[base + 0] | (_workRAM[base + 1] << 8) | (_workRAM[base + 2] << 16) | (_workRAM[base + 3] << 24));
    //     const int32_t oy = (int32_t)(_workRAM[base + 4] | (_workRAM[base + 5] << 8) | (_workRAM[base + 6] << 16) | (_workRAM[base + 7] << 24));
    //     hashEngine.Update(ox >> 16); // integer pixel X
    //     hashEngine.Update(oy >> 16); // integer pixel Y
    //   }
    // }
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    // Cumulative track-position drop this step (see property registration)
    const int curTp = (int)_workRAM[0xA69C];
    _trackPosDrop   = (uint8_t)std::max(0, (int)_trackPosPrev - curTp);

    // Honey-bog flag: the car is on the honey slowdown surface AND feeling its drag effect. Two RAM addresses,
    // one for the SURFACE and one for its EFFECT -- neither position nor speed, which are not reliable (the honey
    // is not one fixed patch, and speed drops on corners/collisions too):
    //   - 0xA649 == 1 : the racer is on the honey surface tile (the "distinct surface").
    //   - 0xA655 == 0 : the surface-drag EFFECT is active. This flag is 1 during all normal driving, corners AND
    //                   the start-line opponent collision (~frame 43), and goes 0 ONLY while a surface is actually
    //                   dragging the car (the launch grip and the honey). Validated by full-RAM diff over two
    //                   movies: 0xA649==1 && 0xA655==0 fires only on the honey crossing (grid (9,11)) and is False
    //                   at the collision, the start (surf 0 there), and every clean pass.
    _honeyBog = (_workRAM[0xA649] == 1 && _workRAM[0xA655] == 0) ? 1 : 0;

    // Coverage exploration: mark the current grid cell (0xA658=gx, 0xA656=gy) visited. Reward = distinct cells
    // visited, so a search maximizing it drives the car naturally all over the track (for terrain effect logging).
    if (_coverageMagnet > 0.0f)
    {
      const int cell = ((int)_workRAM[0xA658] & 31) * 32 + ((int)_workRAM[0xA656] & 31);
      if ((_coverage[cell >> 3] & (1 << (cell & 7))) == 0)
      {
        _coverage[cell >> 3] |= (uint8_t)(1 << (cell & 7));
        _coverageCount++;
      }
    }

    // Re-calculating distance to camera
    _player1DistanceToCameraX = std::abs((float)*_cameraPosX - (float)*_player1PosX);
    _player1DistanceToCameraY = std::abs((float)*_cameraPosY - (float)*_player1PosY);
    _player1DistanceToCamera  = sqrtf(_player1DistanceToCameraX * _player1DistanceToCameraX + _player1DistanceToCameraY * _player1DistanceToCameraY);
  }

  __INLINE__ void ruleUpdatePreHook() override
  {
    // Resetting magnets ahead of rule re-evaluation
    _playerCurrentLapMagnet       = 0.0;
    _playerLapProgressMagnet      = 0.0;
    _trackPositionMagnet          = 0.0;
    _trackDistanceMagnet          = 0.0;
    _trackArcMagnet               = 0.0;
    _checkpointProgressMagnet     = 0.0;
    _speedMagnet                  = 0.0;
    _lowYMagnet                   = 0.0;
    _cpGuideMagnet                = 0.0;
    _cpGuidePointMagnet           = 0.0;
    _cameraDistanceMagnet         = 0.0;
    _recoveryTimerMagnet          = 0.0;
    _lastInputMagnet              = 0.0;
    _playerVelMagnet              = 0.0;
    _player1AngleMagnet.intensity = 0.0;
    _player1AngleMagnet.angle     = 0.0;

    _pointMagnet.intensity = 0.0;
    _pointMagnet.x         = 0.0;
    _pointMagnet.y         = 0.0;
  }

  __INLINE__ void ruleUpdatePostHook() override
  {
    // Updating distance to user-defined point
    _player1DistanceToPointX = std::abs((float)_pointMagnet.x - (float)*_player1PosX);
    _player1DistanceToPointY = std::abs((float)_pointMagnet.y - (float)*_player1PosY);
    _player1DistanceToPoint  = sqrtf(_player1DistanceToPointX * _player1DistanceToPointX + _player1DistanceToPointY * _player1DistanceToPointY);

    _player1DistanceToMagnetAngle = std::abs(_player1AngleMagnet.angle - (float)*_player1Angle);

    // ---- Progress: sequential waypoints along the reference line (breadcrumb) --------------------------------
    // The reference path is an ORDERED chain of waypoints. Progress = the index of the furthest waypoint reached IN
    // ORDER (each counts only once the car comes within the reach radius of it), plus a fractional gradient toward
    // the NEXT, not-yet-reached waypoint. This cannot be gamed by driving straight: to advance, the car must
    // physically reach the next waypoint, and where the line turns that waypoint is off the straight-ahead path --
    // so the car is forced to turn to follow it. Driving past instead pulls the car AWAY from the next waypoint,
    // shrinking the fractional term and LOWERING the reward. The current lap is folded in (total = lap * N + within-
    // lap index), and the within-lap index resets when the game lap counter increments, so laps stack cleanly.
    // ---- From-scratch progress (NO reference line needed) --------------------------------------------------
    // The track checkpoint (0xA69C) runs 57 -> 1 within a lap; a wrap (low -> high) marks a completed lap. Total
    // progress = lapsDone*57 + (57 - cp), latched monotone -- drives the search along the track without any
    // waypoints. Paired with a direction-agnostic speed term, this lets jaffar discover the racing line itself.
    {
      const int cp = (int)_workRAM[0xA69C];
      if (cp > _cpPrev + 20) _cpLapsDone++; // checkpoint wrapped 1 -> 57 = a lap was completed
      _cpPrev          = cp;
      const float prog = (float)_cpLapsDone * 57.0f + (57.0f - (float)cp);
      if (prog > _cpProgress) _cpProgress = prog; // monotone latch
      const int32_t vx  = (int32_t)(_workRAM[0xA612] | (_workRAM[0xA613] << 8) | (_workRAM[0xA614] << 16) | (_workRAM[0xA615] << 24));
      const int32_t vy  = (int32_t)(_workRAM[0xA616] | (_workRAM[0xA617] << 8) | (_workRAM[0xA618] << 16) | (_workRAM[0xA619] << 24));
      const float   fvx = (float)vx / 65536.0f, fvy = (float)vy / 65536.0f;
      _playerSpeed = std::sqrt(fvx * fvx + fvy * fvy); // px/frame, any direction
    }

    // ---- Checkpoint Guide progress: driven by the game's own checkpoint counter (0xA69C) ----
    if (_cpGuideId.empty() == false)
    {
      const int curCp = (int)*_player1TrackPosition; // 0xA69C: the game's cumulative track-progress counter
      // Advance to the furthest REFERENCE-VISITED checkpoint the game says we have reached. Monotonic (latched):
      // it only moves forward, and only through ids the reference itself visited, so an off-route or reversing
      // car earns nothing. Because the game increments 0xA69C, this cannot be gamed by our own geometry.
      while (_cpGuideIdx + 1 < (int)_cpGuideId.size() && _cpGuideId[_cpGuideIdx + 1] <= curCp) _cpGuideIdx++;

      // Soft steering magnet: straight-line distance to the NEXT reference checkpoint's position. This only
      // points the car at the next gate -- it does NOT dictate the line taken to get there.
      if (_cpGuideIdx + 1 < (int)_cpGuideX.size())
      {
        const float dx = (float)*_player1PosX - _cpGuideX[_cpGuideIdx + 1];
        const float dy = (float)*_player1PosY - _cpGuideY[_cpGuideIdx + 1];
        _cpGuideDist   = std::sqrt(dx * dx + dy * dy);
      }
      else
        _cpGuideDist = 0.0f;
    }

    // ---- Reference-line waypoint progress (breadcrumb + arc): only when a Reference Projection was provided ----
    if (_refPathX.empty()) return;
    // CONTINUOUS breadcrumb: the dense Path spans the ENTIRE race (all laps), so there is no per-lap reset
    // and no lap multiplier -- _wpReached walks 0 -> N monotonically across the whole race. A real lap term
    // (tried with laps-done after the lap-address fix) creates a +N*magnet reward CLIFF at each lap line: the
    // first state to cross out-ranks every laggard by millions -> mass eviction -> the frontier collapses to
    // whichever lineage crossed first, not the fastest (measured: best fell 4 checkpoints behind vs the
    // continuous version's +8 ahead). lap stays 0.
    const int lap = 0;
    const int N   = (int)_refPathX.size();
    if (lap > _projLapPrev)
    {
      _projLapPrev = lap;
      _wpReached   = 0; // new lap: restart the waypoint chain
    }

    {
      const float cx = (float)*_player1PosX, cy = (float)*_player1PosY;

      // Non-sequential (shortcut-allowing) advance: scan a bounded forward window of waypoints and jump the reached
      // index to the FURTHEST one currently within reach, even if intermediate waypoints were never approached (a
      // car that shortcuts wp3 -> wp5 is credited wp5). The window (_wpLookahead) bounds the jump so the car can't
      // latch onto a spatially-near but index-distant waypoint where the track loops back. _wpReached stays
      // monotonic (we only ever move it forward), so this never rewards going backward.
      {
        const int scanEnd = std::min(N, _wpReached + 1 + _wpLookahead);
        for (int j = _wpReached + 1; j < scanEnd; j++)
        {
          const float ddx = cx - _refPathX[j], ddy = cy - _refPathY[j];
          if (ddx * ddx + ddy * ddy <= _wpReachRadius2) _wpReached = j; // furthest reachable in the window
        }
      }

      // Discrete progress: total waypoints reached (coarse). With the window >1 this is a shortcut-aware count.
      _wpDiscrete = lap * N + _wpReached;

      // Continuous complement: straight-line distance to the NEAREST not-yet-reached waypoint within the forward
      // window. The reward subtracts a weighted form of this (closer = better), giving a smooth per-frame gradient
      // that pulls the car toward the next target -- sequential OR a shortcut target ahead. Zero once none remain.
      {
        const int scanEnd = std::min(N, _wpReached + 1 + _wpLookahead);
        float     best    = -1.0f;
        for (int j = _wpReached + 1; j < scanEnd; j++)
        {
          const float ndx = cx - _refPathX[j], ndy = cy - _refPathY[j];
          const float d2 = ndx * ndx + ndy * ndy;
          if (best < 0.0f || d2 < best) best = d2;
        }
        _distToNext = best < 0.0f ? 0.0f : std::sqrt(best);
      }

      // Single-magnet, fully-continuous ARC progress: cumulative reference arc length up to the next frame MINUS
      // the straight-line distance to that frame = "distance travelled along the reference line". With per-frame
      // waypoints this is smooth across the whole search (no discrete/continuous staircase); one 1.0 magnet does
      // the whole job. The in-order reached backbone (above) keeps it from corner-cutting (unlike raw projection).
      if (_wpReached + 1 < N)
      {
        const float adx = cx - _refPathX[_wpReached + 1], ady = cy - _refPathY[_wpReached + 1];
        _wpArcProgress = (float)lap * _refPathTotalArc + _refPathArc[_wpReached + 1] - std::sqrt(adx * adx + ady * ady);
      }
      else
        _wpArcProgress = (float)lap * _refPathTotalArc + _refPathTotalArc;
    }
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    // Storing previous lap count
    serializer.pushContiguous(&_player1LapsRemainingPrev, sizeof(_player1LapsRemainingPrev));
    serializer.pushContiguous(&_player1CheckpointPrev, sizeof(_player1CheckpointPrev));
    serializer.pushContiguous(&_trackPosPrev, sizeof(_trackPosPrev));
    serializer.pushContiguous(&_trackPosDrop, sizeof(_trackPosDrop));
    serializer.pushContiguous(&_honeyBog, sizeof(_honeyBog));
    if (_coverageMagnet > 0.0f)
    {
      serializer.pushContiguous(_coverage, sizeof(_coverage));
      serializer.pushContiguous(&_coverageCount, sizeof(_coverageCount));
    }
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_wpReached, sizeof(_wpReached));
    serializer.pushContiguous(&_cpGuideIdx, sizeof(_cpGuideIdx));
    serializer.pushContiguous(&_projLapPrev, sizeof(_projLapPrev));
    serializer.pushContiguous(&_cpProgress, sizeof(_cpProgress));
    serializer.pushContiguous(&_cpLapsDone, sizeof(_cpLapsDone));
    serializer.pushContiguous(&_cpPrev, sizeof(_cpPrev));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    // Restoring previous lap count
    deserializer.popContiguous(&_player1LapsRemainingPrev, sizeof(_player1LapsRemainingPrev));
    deserializer.popContiguous(&_player1CheckpointPrev, sizeof(_player1CheckpointPrev));
    deserializer.popContiguous(&_trackPosPrev, sizeof(_trackPosPrev));
    deserializer.popContiguous(&_trackPosDrop, sizeof(_trackPosDrop));
    deserializer.popContiguous(&_honeyBog, sizeof(_honeyBog));
    if (_coverageMagnet > 0.0f)
    {
      deserializer.popContiguous(_coverage, sizeof(_coverage));
      deserializer.popContiguous(&_coverageCount, sizeof(_coverageCount));
    }
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_wpReached, sizeof(_wpReached));
    deserializer.popContiguous(&_cpGuideIdx, sizeof(_cpGuideIdx));
    deserializer.popContiguous(&_projLapPrev, sizeof(_projLapPrev));
    deserializer.popContiguous(&_cpProgress, sizeof(_cpProgress));
    deserializer.popContiguous(&_cpLapsDone, sizeof(_cpLapsDone));
    deserializer.popContiguous(&_cpPrev, sizeof(_cpPrev));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Subtracting reward for having made an input recently (for early termination)
    reward += _lastInputMagnet * _lastInputStep;

    // Evaluating player current lap  magnet
    reward += _playerCurrentLapMagnet * (float)(*_player1LapsRemaining);

    // Evaluating player lap progress  magnet
    reward += _playerLapProgressMagnet * (float)(*_player1Checkpoint1);

    // Track-progress: DISCRETE waypoints reached in order (coarse, keeps the car in sequence on the line) plus a
    // CONTINUOUS distance-to-next-waypoint gradient (smooth per-frame pull toward the next waypoint). See
    // ruleUpdatePostHook. The distance magnet is subtracted (closer to the next waypoint = higher reward).
    reward += _trackPositionMagnet * (float)_wpDiscrete;
    reward += -1.0f * _trackDistanceMagnet * _distToNext;

    // Single-magnet, fully-continuous alternative to the two above: arc length travelled along the reference line.
    reward += _trackArcMagnet * _wpArcProgress;

    // From-scratch (no reference): reward the game's own checkpoint progress along the track + raw speed (any
    // direction). Lets jaffar discover the racing line itself. See ruleUpdatePostHook.
    reward += _checkpointProgressMagnet * _cpProgress;
    reward += _speedMagnet * _playerSpeed;

    // Track-coverage exploration reward: distinct grid cells visited so far along this lineage.
    reward += _coverageMagnet * (float)_coverageCount;

    // Low-Y clearance: reward for being above the baseline (honey-avoidance corridor). Zero unless a rule enabled it.
    // CLAMPED so climbing above the corridor earns nothing (uncapped, the search flies up off the table to farm the
    // reward instead of progressing) and a deep dip can't dominate: clearance is bounded to [-30, +25] px.
    if (_lowYMagnet != 0.0f)
    {
      float clr = _lowYBaseline - (float)*_player1PosY;
      if (clr > 25.0f) clr = 25.0f;
      if (clr < -30.0f) clr = -30.0f;
      reward += _lowYMagnet * clr;
    }

    // Checkpoint Guide: progress = how many REFERENCE-VISITED checkpoints the GAME says we have passed
    // (un-gameable, route-correct), minus a soft magnet toward the next checkpoint's position (steering only).
    reward += _cpGuideMagnet * (float)_cpGuideIdx;
    reward += -1.0f * _cpGuidePointMagnet * _cpGuideDist;

    // Distance to point magnet
    reward += -1.0 * _pointMagnet.intensity * _player1DistanceToPoint;

    // Distance to camera
    reward += -1.0 * _cameraDistanceMagnet * _player1DistanceToCamera;

    // Evaluating player health  magnet
    reward += _recoveryTimerMagnet * (float)(*_player1RecoveryTimer);

    // Calculating angle magnet
    reward += (255.0 - _player1DistanceToMagnetAngle) * _player1AngleMagnet.intensity;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    if (std::abs(_pointMagnet.intensity) > 0.0f)
    {
      jaffarCommon::logger::log("[J+]  + Point Magnet                             Intensity: %.5f, X: %3.3f, Y: %3.3f\n", _pointMagnet.intensity, _pointMagnet.x, _pointMagnet.y);
      jaffarCommon::logger::log("[J+]    + Distance X                             %3.3f\n", _player1DistanceToPointX);
      jaffarCommon::logger::log("[J+]    + Distance Y                             %3.3f\n", _player1DistanceToPointY);
      jaffarCommon::logger::log("[J+]    + Total Distance                         %3.3f\n", _player1DistanceToPoint);
    }

    if (std::abs(_cameraDistanceMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Camera Distance Magnet                   Intensity: %.5f, Dist: %3.3f\n", _cameraDistanceMagnet, _player1DistanceToCamera);
    if (std::abs(_recoveryTimerMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Recovery Timer Magnet                    Intensity: %.5f\n", _recoveryTimerMagnet);
    if (std::abs(_playerCurrentLapMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Player Current Lap Magnet                Intensity: %.5f\n", _playerCurrentLapMagnet);
    if (std::abs(_playerLapProgressMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Player Lap Progress Magnet               Intensity: %.5f\n", _playerLapProgressMagnet);
    if (std::abs(_trackPositionMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Reference Progress Magnet                Intensity: %.5f, Waypoint: %d / %d (lap %d)\n", _trackPositionMagnet, _wpReached,
                                (int)_refPathX.size(), _projLapPrev);
    if (std::abs(_trackDistanceMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Track Distance Magnet                    Intensity: %.5f, Dist to next wp: %.2f\n", _trackDistanceMagnet, _distToNext);
    if (std::abs(_trackArcMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Track Arc Magnet                         Intensity: %.5f, Arc progress: %.1f / %.0f (wp %d / %d)\n", _trackArcMagnet, _wpArcProgress,
                                _refPathTotalArc, _wpReached, (int)_refPathX.size());
    jaffarCommon::logger::log("[J+]  + Player In First Place:                   %s (opponent positions %s hash)\n", isPlayerInFirst() ? "YES" : "NO",
                              isPlayerInFirst() ? "EXCLUDED from" : "included in");
    if (std::abs(_checkpointProgressMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Checkpoint Progress Magnet               Intensity: %.5f, Progress: %.1f (laps done %d, cp %d)\n", _checkpointProgressMagnet, _cpProgress,
                                _cpLapsDone, (int)_workRAM[0xA69C]);
    if (std::abs(_cpGuideMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Checkpoint Guide Magnet                 Intensity: %.1f, Checkpoint: %d / %lu (game cp %d), dist to next: %.1f\n", _cpGuideMagnet,
                                _cpGuideIdx, _cpGuideId.size(), (int)*_player1TrackPosition, _cpGuideDist);

    if (std::abs(_speedMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Speed Magnet                             Intensity: %.5f, Speed: %.2f px/frame\n", _speedMagnet, _playerSpeed);
    if (std::abs(_lowYMagnet) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Low Y Magnet                             Intensity: %.5f, Clearance: %.1f (baseline %.0f, posY %d)\n", _lowYMagnet,
                                _lowYBaseline - (float)*_player1PosY, _lowYBaseline, (int)*_player1PosY);
    if (std::abs(_lastInputMagnet) > 0.0f) jaffarCommon::logger::log("[J+]  + Last Input Magnet                      Intensity: %.5f\n", _lastInputMagnet);
    if (std::abs(_player1AngleMagnet.intensity) > 0.0f)
      jaffarCommon::logger::log("[J+]  + Player Angle Magnet                      Intensity: %.5f, Angle: %3.0f, Dist: %3.0f\n", _player1AngleMagnet.intensity,
                                _player1AngleMagnet.angle, _player1DistanceToMagnetAngle);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    if (actionType == "Set Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto x         = jaffarCommon::json::getNumber<float>(actionJs, "X");
      auto y         = jaffarCommon::json::getNumber<float>(actionJs, "Y");
      auto action    = [=, this]() { this->_pointMagnet = pointMagnet_t{.intensity = intensity, .x = x, .y = y}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Current Lap Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_playerCurrentLapMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Lap Progress Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_playerLapProgressMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Track Position Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_trackPositionMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Track Distance Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_trackDistanceMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Track Arc Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_trackArcMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Checkpoint Progress Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_checkpointProgressMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Checkpoint Guide Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_cpGuideMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Checkpoint Point Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_cpGuidePointMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Speed Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_speedMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    // Low-Y magnet: rewards vertical CLEARANCE above a baseline y (intensity * (Baseline - posY)). Gated to a hazard
    // zone (e.g. the honey band, via a Track Position condition), it makes staying HIGH (out of the sticky deep zone)
    // out-reward the faster low line every frame of the approach, so the clean corridor dominates the frontier at any
    // size -- the honey line earns a negative clearance and is culled before it ever reaches the sticky surface.
    if (actionType == "Set Low Y Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto baseline  = jaffarCommon::json::getNumber<float>(actionJs, "Baseline");
      auto action    = [=, this]()
      {
        this->_lowYMagnet   = intensity;
        this->_lowYBaseline = baseline;
      };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player Vel Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_playerVelMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Camera Distance Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_cameraDistanceMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Recovery Timer Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_recoveryTimerMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Last Input Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto action    = [=, this]() { this->_lastInputMagnet = intensity; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    if (actionType == "Set Player 1 Angle Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto angle     = jaffarCommon::json::getNumber<float>(actionJs, "Angle");
      auto action    = [=, this]() { this->_player1AngleMagnet = angleMagnet_t{.intensity = intensity, .angle = angle}; };
      rule.addAction(action);
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  // Datatype to describe a point magnet
  struct pointMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float x         = 0.0; // What is the x point of attraction
    float y         = 0.0; // What is the y point of attraction
  };

  // Datatype to describe an angle magnet
  struct angleMagnet_t
  {
    float intensity = 0.0; // How strong the magnet is
    float angle     = 0.0; // What is the angle we look for
  };

  // Magnets (used to determine state reward and have Jaffar favor a direction or action)
  float         _playerCurrentLapMagnet   = 0.0;
  float         _playerLapProgressMagnet  = 0.0;
  float         _trackPositionMagnet      = 0.0;
  float         _trackDistanceMagnet      = 0.0;
  float         _trackArcMagnet           = 0.0;
  float         _checkpointProgressMagnet = 0.0;
  float         _speedMagnet              = 0.0;
  float         _lowYMagnet               = 0.0; // reward for clearance above _lowYBaseline (honey-avoidance corridor)
  float         _lowYBaseline             = 0.0; // baseline y; reward = _lowYMagnet * (_lowYBaseline - posY)
  float         _cpGuideMagnet            = 0.0; // reward per reference-visited checkpoint passed (game-judged)
  float         _cpGuidePointMagnet       = 0.0; // soft steering pull toward the next checkpoint's position
  float         _cameraDistanceMagnet     = 0.0;
  float         _recoveryTimerMagnet      = 0.0;
  float         _lastInputMagnet          = 0.0;
  float         _playerVelMagnet          = 0.0;
  angleMagnet_t _player1AngleMagnet;
  pointMagnet_t _pointMagnet;

  // Property pointers for quick access
  uint8_t* _workRAM;
  uint8_t* _currentRace;

  uint16_t* _player1LapsRemaining;
  uint16_t* _player1LapsDone;
  uint16_t  _player1LapsRemainingPrev;

  uint8_t* _player1Checkpoint1;
  uint8_t* _player1Checkpoint2;
  uint8_t* _player1OffroadCheckpoint;
  uint8_t* _player1TrackPosition;
  uint8_t  _player1CheckpointPrev = 0;
  uint8_t  _trackPosPrev          = 0;    // 0xA69C value before the last advance
  uint8_t  _trackPosDrop          = 0;    // max(0, prev - current) of 0xA69C this step
  uint8_t  _honeyBog              = 0;    // 1 = on a sticky surface (0xA649!=0) AND bogged below the speed threshold
  float    _honeySpeedThreshold   = 2.5f; // px/frame; clean sticky passes hold >=3.0, the honey sinks to ~1.2
  float    _coverageMagnet        = 0.0f; // >0 enables track-coverage exploration (reward per distinct grid cell)
  int      _coverageCount         = 0;    // distinct grid cells visited along this lineage
  uint8_t  _coverage[128]         = {0};  // 32x32 visited-cell bitmap

  // Sequential-waypoint progress along the reference line (breadcrumb; see ruleUpdatePostHook).
  // From-scratch checkpoint progress (no reference line): see ruleUpdatePostHook.
  float _cpProgress  = 0.0f; // latched total track progress = lapsDone*57 + (57 - checkpoint)
  int   _cpLapsDone  = 0;    // laps completed, detected by checkpoint wrap (low -> high)
  int   _cpPrev      = 57;   // previous checkpoint value (race starts at 57), for wrap detection
  float _playerSpeed = 0.0f; // current speed magnitude (px/frame) from the 16.16 velocity vector

  std::vector<int>   _cpGuideId;           // ordered checkpoint ids the REFERENCE visited
  std::vector<float> _cpGuideX, _cpGuideY; // their positions (steering targets only)
  int                _cpGuideIdx  = 0;     // furthest reference checkpoint reached, per the game's counter (latched)
  float              _cpGuideDist = 0.0f;  // distance to the next reference checkpoint (steering gradient)

  int   _wpReached     = 0;    // furthest reference waypoint reached IN ORDER this lap (latched, monotonic)
  int   _wpDiscrete    = 0;    // total discrete progress = lap * N + _wpReached (derived each frame)
  float _distToNext    = 0.0f; // straight-line distance to the next not-yet-reached waypoint (continuous gradient)
  float _wpArcProgress = 0.0f; // arc length travelled along the reference line (single-magnet continuous reward)
  int   _projLapPrev   = 0;    // last observed lap, to detect lap crossings (resets _wpReached)

  // Reference-line waypoints parsed from the game config ("Reference Projection"): the reference solution's sampled
  // path (X, Y), walked as an ordered waypoint chain. Config-driven so a different race needs no rebuild.
  std::vector<float> _refPathX;
  std::vector<float> _refPathY;
  std::vector<float> _refPathArc;                     // cumulative arc length to each waypoint (arc-progress reward)
  float              _refPathTotalArc         = 0.0f; // total reference-line arc length (one lap)
  float              _wpReachRadius2          = 0.0f; // squared reach radius: how close (position units) counts as reaching a waypoint
  int                _wpLookahead             = 1;    // forward scan window; >1 allows non-sequential (shortcut) waypoint jumps
  uint32_t           _positionHashGranularity = 8;    // sub-pixel fraction bits kept in the dedup hash (config-driven)
  uint32_t           _velocityHashGranularity = 4;    // velocity fraction bits kept in the dedup hash (config-driven)

  uint8_t* _player1RecoveryMode;
  uint8_t* _player1RecoveryTimer;

  uint16_t* _player1PosX;
  uint16_t* _player1PosY;

  uint16_t* _player1VelX;
  uint16_t* _player1VelY;

  uint16_t* _player1Angle;
  uint16_t* _cameraPosX;
  uint16_t* _cameraPosY;

  // Game-Specific values
  float _player1DistanceToPointX;
  float _player1DistanceToPointY;
  float _player1DistanceToPoint;
  float _player1DistanceToCameraX;
  float _player1DistanceToCameraY;
  float _player1DistanceToCamera;
  float _player1DistanceToMagnetAngle;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace genesis

} // namespace games

} // namespace jaffarPlus