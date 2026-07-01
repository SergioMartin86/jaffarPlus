#pragma once

#include <atomic>
#include <emulator.hpp>
#include <fstream>
#include <game.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/string.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

// --- DIAGNOSTIC: per-byte distinct-value histogram over AIRBORNE states (measures where the hash density
// lives). Cheap (load-then-fetch_or), cumulative across the run. Dumped in printInfoImpl. ---
namespace ebdiag
{
struct ByteHist
{
  std::atomic<uint64_t> bits[4]{};
  inline void rec(uint8_t v)
  {
    uint64_t m = 1ULL << (v & 63);
    int      i = v >> 6;
    if (!(bits[i].load(std::memory_order_relaxed) & m)) bits[i].fetch_or(m, std::memory_order_relaxed);
  }
  int distinct() const
  {
    int c = 0;
    for (int i = 0; i < 4; i++) c += __builtin_popcountll(bits[i].load(std::memory_order_relaxed));
    return c;
  }
};
static constexpr int NHB                = 14;
inline ByteHist      g_hb[NHB];
inline const char*   g_hbName[NHB] = {"posZ2", "posY",  "velY1",   "velY2",   "velZ",      "posZ1",   "angle",
                                      "velX1", "velX2", "flight1", "flight2", "flight3", "gameCycle", "posX2"};
} // namespace ebdiag

class ExciteBike final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Excite Bike"; }

  /// @brief Real ground control input: accel (B, or A=turbo) + lane steer + angle steer, applied together as a
  /// NES diagonal. laneDir>0 => U (decreases posY toward a lower committed lane), <0 => D. angDir>0 => L (raises
  /// the bike angle toward the committed ground angle), <0 => R (lowers it). 0 => neither on that axis.
  static __INLINE__ std::string groundInput(bool turbo, int laneDir, int angDir)
  {
    std::string s = "|..|........|";
    if (laneDir > 0)
      s[4] = 'U';
    else if (laneDir < 0)
      s[5] = 'D';
    if (angDir > 0)
      s[6] = 'L';
    else if (angDir < 0)
      s[7] = 'R';
    s[turbo ? 11 : 10] = turbo ? 'A' : 'B';
    return s;
  }

  /// @brief Real airborne control input: steer posY (laneDir>0 => U decreases posY toward a lower committed
  /// air lane, <0 => D) AND angle (angDir>0 => L raises toward the committed jump angle, <0 => R). No accel
  /// button airborne (B/A are inert off the ground). 0 => neither on that axis.
  static __INLINE__ std::string airInput(int laneDir, int angDir)
  {
    std::string s = "|..|........|";
    if (laneDir > 0)
      s[4] = 'U';
    else if (laneDir < 0)
      s[5] = 'D';
    if (angDir > 0)
      s[6] = 'L';
    else if (angDir < 0)
      s[7] = 'R';
    return s;
  }

  ExciteBike(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Search action space (built in this .hpp, not the config's "Allowed Input Sets"):
    //   AIRBORNE: fully free per-frame angle -- {raise (L), lower (R), hold} every airborne frame. No commit.
    //   GROUND:   lane commit by U/D-drop -- the bike steers U/D to a chosen lane, and the moment it issues a
    //             no-lane-change input it LOCKS that lane (U/D removed) until the next landing; throttle is
    //             automatic (B to hold the 832 cap, coast when carrying launch speed velX>832).
    // No multi-frame macro (one emulator frame per step) and no airborne-angle commitment (both removed:
    // jumps vary their angle in-flight, so the angle must stay free per-frame).

    // Opponent removal: the CPU racers (posY at 0x8D/0x8E/0x8F; player posY=0x8C) can knock a lane-locked
    // player off the bike even going straight. When enabled, every frame we park them out of the player's
    // lane band (default posY 255 = off the track) so they can never collide. See stateUpdatePostHook.
    _removeOpponents = _gameConfigRemaining.contains("Remove Opponents") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Remove Opponents") : false;
    _opponentParkY   = _gameConfigRemaining.contains("Opponent Park Y") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Opponent Park Y") : 255;

    // Master gate for the search action space. OFF (default) => advanceStateImpl applies inputs RAW, so
    // reference movies (race01a.tas.sol) replay bit-exact. Search configs set "Search Mode": true.
    _searchMode = _gameConfigRemaining.contains("Search Mode") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Search Mode") : false;
    // Descent angle: lock the HIGH descent (commit one angle) but FREE the final approach near the ground so
    // the landing stays exact. "Near the ground" uses the engine's own landing trigger: it lands when posY
    // ($8C) reaches groundY = ($A0 - $BC - height $B8); so distToGround = groundY - posY shrinks to 0 at
    // touchdown. Free the angle once distToGround <= this threshold; lock it above. -1 (default) => the whole
    // descent stays free (the plain hybrid). Larger => free more of the approach (more landing precision, less
    // breadth cut); smaller => lock more of the descent.
    _descentFreeBelowDist =
        _gameConfigRemaining.contains("Descent Free Below Ground Dist") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Descent Free Below Ground Dist") : -1;
    // Master switch for ALL airborne angle commits (climb-lock + descent). Default true. Set FALSE for full
    // per-frame airborne angle freedom (the only tol=0-exact model -- the reference's in-air angle is
    // frame-by-frame consequential to the landing). Costs airborne breadth, paid with a bigger State DB.
    _airborneAngleCommit = _gameConfigRemaining.contains("Airborne Angle Commit") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Airborne Angle Commit") : true;
    // Ground angle steering: also offer L/R to steer the bike ground angle ($AC), which the reference uses to
    // set up takeoff arcs on ramps. Default false (preserve prior behavior). When true the ground angle is also
    // hashed so the steered (takeoff-relevant) angle is preserved instead of deduped away.
    _groundAngleSteering = _gameConfigRemaining.contains("Ground Angle Steering") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Ground Angle Steering") : false;
    // Free throttle: offer BOTH B (accelerate) and coast (no accel) every ground frame instead of the auto-rule
    // (coast iff velX>832). The reference pulses B/coast at velX==832 on downhills, which the auto-rule cannot
    // express. Default false. Adds ground breadth (2x per control) -- pair with aggressive dedup.
    _freeThrottle = _gameConfigRemaining.contains("Free Throttle") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Free Throttle") : false;
    // Ground diagonals: offer lane+angle simultaneous combos (U+L, U+R, D+L, D+R). The reference presses them
    // to set up takeoffs (e.g. U+R+B at frame 1130). Needs Ground Angle Steering. Default false. Adds breadth.
    _groundDiagonals = _gameConfigRemaining.contains("Ground Diagonals") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Ground Diagonals") : false;
    // Initial block-transition count: _blockXTransitions is a running counter (256-px blocks passed) that
    // makes _bikePosX absolute; it is NOT in the emulator state, so when seeding mid-run from an Initial
    // State File this restores it (set to floor(seed posX / 256)) so _bikePosX is correct from frame 0.
    _initialBlockTransitions =
        _gameConfigRemaining.contains("Initial Block Transitions") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Initial Block Transitions") : 0;
    // Airborne hash quantization: right-shift the airborne breadth bytes by this many bits when hashing while
    // airborne (merges near-identical airborne states -> bounds breadth; dedup-only, never prunes by reward).
    _airQ = _gameConfigRemaining.contains("Airborne Hash Quantization") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Airborne Hash Quantization") : 0;
    // Per-element hash drops (experiment): list of breadth-driving bytes to EXCLUDE from the dedup hash, to
    // test which are NOT reward-relevant (dropping them cuts breadth without trailing). Names: VelZ PosZ1
    // PosZ2 PosY VelY1 VelY2 Angle Flight GameCycle.
    std::vector<std::string> drops =
        _gameConfigRemaining.contains("Hash Drops") ? jaffarCommon::json::popArray<std::string>(_gameConfigRemaining, "Hash Drops") : std::vector<std::string>{};
    auto has       = [&](const char* n) { return std::find(drops.begin(), drops.end(), std::string(n)) != drops.end(); };
    _dropVelZ      = has("VelZ");
    _dropPosZ1     = has("PosZ1");
    _dropPosZ2     = has("PosZ2");
    _dropPosY      = has("PosY");
    _dropVelY1     = has("VelY1");
    _dropVelY2     = has("VelY2");
    _dropAngle     = has("Angle");
    _dropFlight    = has("Flight");
    _dropFlight2   = has("Flight2"); // drop ONLY the fractional flight2 accumulator (keep flight3 arc info)
    _dropGameCycle = has("GameCycle");
    // Dedicated flight2 (0x380) airborne quantization (right-shift) -- coarsen the fractional accumulator
    // without dropping it entirely. 0 => full resolution (unless Flight2 is in Hash Drops).
    _flight2Q = _gameConfigRemaining.contains("Flight2 Hash Quantization") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Flight2 Hash Quantization") : 0;
    // Trace magnet: a per-step reference trajectory ("<posX> <posY> <posZ2>", one line per search step, generated by
    // `player --dumpTrace`). A SMALL reward penalty proportional to the bike's distance from the trace at the current
    // step keeps reference-adjacent states competitive in the (reward-ranked) State DB, so a non-greedy optimum (the
    // reference line, temporarily lower posX than an apparent-better route) is not evicted once the DB fills. Reward
    // stays the sole eviction driver -- the magnet just grants legitimate reward for staying near the known-good line.
    // The reference itself has distance 0 (magnet 0), so the Reference Reward Floor is unaffected. Intensities are set
    // by the "Set Trace Magnet" rule action (default all 0 => magnet off, even with a trace loaded).
    _traceFilePath = _gameConfigRemaining.contains("Trace File Path") ? jaffarCommon::json::popString(_gameConfigRemaining, "Trace File Path") : std::string("");
    if (_traceFilePath != "")
    {
      _useTrace = true;
      std::string traceData;
      const bool  status = jaffarCommon::file::loadStringFromFile(traceData, _traceFilePath.c_str());
      if (status == false && std::getenv("JAFFAR_IS_DRY_RUN") == nullptr) JAFFAR_THROW_LOGIC("Could not find/read trace file: %s\n", _traceFilePath.c_str());
      std::istringstream f(traceData);
      std::string        line;
      while (std::getline(f, line))
      {
        if (line.empty()) continue;
        const auto c = jaffarCommon::string::split(line, ' ');
        _trace.push_back(traceEntry_t{.posX    = (float)std::atof(c[0].c_str()),
                                      .posY    = (float)std::atof(c[1].c_str()),
                                      .posZ2   = (float)std::atof(c[2].c_str()),
                                      .velX    = c.size() > 3 ? (float)std::atof(c[3].c_str()) : 0.0f,
                                      .flight2 = c.size() > 4 ? (float)std::atof(c[4].c_str()) : 0.0f});
      }
    }
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    _bikePosX1        = (uint8_t*)registerGameProperty("Bike Pos X1", &_lowMem[0x0050], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikePosX2        = (uint8_t*)registerGameProperty("Bike Pos X2", &_lowMem[0x0394], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _intraLoopAdvance = (uint8_t*)registerGameProperty("Intra Loop Advance", &_lowMem[0x00ED], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _loopsRemaining   = (uint8_t*)registerGameProperty("Loops Remaining", &_lowMem[0x0057], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _currentLoop      = (uint8_t*)registerGameProperty("Current Loop", &_lowMem[0x03A4], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _raceOverFlag     = (uint8_t*)registerGameProperty("Race Over Flag", &_lowMem[0x0052], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeMoving       = (uint8_t*)registerGameProperty("Bike Moving", &_lowMem[0x000E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeAngle        = (uint8_t*)registerGameProperty("Bike Angle", &_lowMem[0x00AC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeAirMode      = (uint8_t*)registerGameProperty("Bike Air Mode", &_lowMem[0x00B0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeVelX2        = (uint8_t*)registerGameProperty("Bike Vel X2", &_lowMem[0x0094], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeVelX1        = (uint8_t*)registerGameProperty("Bike Vel X1", &_lowMem[0x0090], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeEngineTemp   = (uint8_t*)registerGameProperty("Bike Engine Temp", &_lowMem[0x03B6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeVelZ         = (uint8_t*)registerGameProperty("Bike Vel Z", &_lowMem[0x00DC], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikePosZ1        = (uint8_t*)registerGameProperty("Bike Pos Z1", &_lowMem[0x0070], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikePosZ2        = (uint8_t*)registerGameProperty("Bike Pos Z2", &_lowMem[0x00B8], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikePosY         = (uint8_t*)registerGameProperty("Bike Pos Y", &_lowMem[0x008C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeVelY1        = (uint8_t*)registerGameProperty("Bike Vel Y1", &_lowMem[0x0270], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeVelY2        = (uint8_t*)registerGameProperty("Bike Vel Y2", &_lowMem[0x0274], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeFlightMode1  = (uint8_t*)registerGameProperty("Bike Flight Mode 1", &_lowMem[0x00F6], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeFlightMode2  = (uint8_t*)registerGameProperty("Bike Flight Mode 2", &_lowMem[0x0380], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _bikeFlightMode3  = (uint8_t*)registerGameProperty("Bike Flight Mode 3", &_lowMem[0x0384], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _gameCycle        = (uint8_t*)registerGameProperty("Game Cycle", &_lowMem[0x004C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _currBlockX       = (uint8_t*)registerGameProperty("Curr Block X", &_lowMem[0x004E], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    // Player crash/wipeout flag (set to 0xFF by the game's landing-angle safety check at $DC69 when the bike
    // lands outside the safe angle range for the slope). The reference TAS keeps it 0 for all frames -> a fail
    // rule on it (Crash Flag > 0) forces crash-free play, pruning the nose-down-then-wipeout lines.
    registerGameProperty("Crash Flag", &_lowMem[0x0098], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    // Upward jump momentum ($BC): >0 while climbing, hits 0 at the apex and stays 0 through the descent. Used
    // to split the airborne angle into a climb segment (nose-up) and a descent/landing segment (nose-down),
    // matching the TAS's frame-precise jump arc (verified: TAS noses 9->11 climbing, 11->3 descending).
    _climbRemaining = (uint8_t*)registerGameProperty("Climb Remaining", &_lowMem[0x00BC], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Block X Transitions", &_blockXTransitions, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bike Pos X", &_bikePosX, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Cur Vel", &_curVel, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Max Vel", &_maxVel, Property::datatype_t::dt_uint16, Property::endianness_t::little);
    registerGameProperty("Prev Block X", &_prevBlockX, Property::datatype_t::dt_uint8, Property::endianness_t::little);

    _currentStep       = 0;
    _nullInputIdx      = _emulator->registerInput("|..|........|");
    _lastInput         = _nullInputIdx;
    _blockXTransitions = (uint8_t)_initialBlockTransitions;
    _firstPostHook     = true;
    _curVel            = 0;
    _maxVel            = 0;

    // Real control inputs actually applied to the emulator (never branched over by the search directly).
    // Airborne controls: [laneDir -1..1][angDir -1..1] -> steer posY (U/D) + angle (L/R), no accel.
    for (int ld = -1; ld <= 1; ld++)
      for (int ad = -1; ad <= 1; ad++) _airCtrl[ld + 1][ad + 1] = _emulator->registerInput(airInput(ld, ad));
    // Ground controls: [turbo?][laneDir -1..1][angDir -1..1] -> accel + lane steer (U/D) + angle steer (L/R).
    for (int t = 0; t < 2; t++)
      for (int ld = -1; ld <= 1; ld++)
        for (int ad = -1; ad <= 1; ad++) _groundCtrl[t][ld + 1][ad + 1] = _emulator->registerInput(groundInput(t == 1, ld, ad));

    // Convenience handles into the control tables (laneDir/angDir index = value+1):
    _airAngleRaise = _airCtrl[1][2]; // L: raise angle (no lane, no accel)
    _airAngleLower = _airCtrl[1][0]; // R: lower angle
    _airAngleHold  = _airCtrl[1][1]; // hold (= null)
    _groundLockB   = _groundCtrl[0][1][1]; // B, no lane steer -- the "lock here" input while accelerating
    // (coast + no-lane-steer == null == _nullInputIdx; both are treated as the lock input on the ground)

    _laneLocked = false; // fresh ground stretch: U/D allowed until the search locks a lane
    _angleLocked = false; // fresh jump: L/R allowed until the search locks the angle

    stateUpdatePostHook();
  }

  // Airborne lock phase: lock the angle during the CLIMB always, and during the HIGH descent when the
  // distance-to-ground mechanism is on; FREE the final approach near the ground (distToGround <= threshold)
  // so the landing stays exact. distToGround = groundY - posY, the engine's own landing trigger (lands at
  // posY >= groundY = $A0 - $BC - height $B8). _descentFreeBelowDist < 0 => whole descent free (plain hybrid).
  __INLINE__ bool airAngleLockPhase() const
  {
    if (!_airborneAngleCommit) return false; // full per-frame airborne angle freedom (no commit)
    if (*_climbRemaining > 0) return true;       // climb: always lock
    if (_descentFreeBelowDist < 0) return false; // descent fully free
    const int groundY      = (0xA0 - (int)*_climbRemaining - (int)*_bikePosZ2) & 0xFF;
    const int distToGround = groundY - (int)*_bikePosY;
    return distToGround > _descentFreeBelowDist; // lock the high descent; free near the ground
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Raw (pristine) mode: apply inputs directly so reference movies (race01a.tas.sol) replay bit-exact.
    if (!_searchMode)
    {
      _emulator->advanceState(input);
      _currentStep++;
      _lastInput = input;
      return;
    }

    // Search mode: every offered input is ALREADY a real control (see getAdditionalAllowedInputs), applied
    // directly -- exactly ONE emulator frame per step (no macro / fast-forward). On the GROUND, a no-lane-
    // steer input (B-only while accelerating, or null while coasting) LOCKS the current lane: U/D are dropped
    // for the rest of the ground stretch (this is the lane commit).
    const uint8_t prevAir   = *_bikeAirMode;
    const uint8_t prevClimb = *_climbRemaining; // $BC: >0 climbing, 0 at apex/descent (for the climb/descent split)
    if (prevAir == 0 && !_laneLocked && (input == _groundLockB || input == _nullInputIdx)) _laneLocked = true;
    // Lock the angle when the search picks hold during a lock phase (climb, or high descent). Computed on the
    // pre-advance state, consistent with the inputs offered by getAdditionalAllowedInputs.
    const bool lockPhase = airAngleLockPhase();
    if (prevAir > 0 && lockPhase && !_angleLocked && input == _airAngleHold) _angleLocked = true;

    _emulator->advanceState(input);
    _currentStep++;

    const uint8_t newAir = *_bikeAirMode;
    if (prevAir == 0 && newAir > 0) _angleLocked = false; // takeoff: fresh climb-angle commit
    // Apex (climb -> descent): re-commit a fresh descent angle when the descent has a lock phase (dist mech on).
    if (_descentFreeBelowDist >= 0 && newAir > 0 && prevClimb > 0 && *_climbRemaining == 0) _angleLocked = false;
    if (prevAir > 0 && newAir == 0) _laneLocked = false; // landing: fresh ground stretch

    _lastInput = input;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // Storage for hash calculation
    hashEngine.Update(*_bikePosX1);
    hashEngine.Update(*_bikePosX2);
    hashEngine.Update(*_intraLoopAdvance);
    hashEngine.Update(*_loopsRemaining);
    hashEngine.Update(*_currentLoop);
    hashEngine.Update(*_raceOverFlag);
    hashEngine.Update(*_bikeMoving);
    if (!_dropAngle && (*_bikeAirMode > 0 || _groundAngleSteering)) hashEngine.Update(*_bikeAngle); // ground angle matters for takeoffs
    hashEngine.Update(*_bikeAirMode);
    hashEngine.Update(*_bikeVelX2);
    hashEngine.Update(*_bikeVelX1);
    // hashEngine.Update(*_bikeEngineTemp    );
    // Airborne breadth drivers: when airborne, quantize (>> _airQ) to merge near-identical airborne states
    // (bounds the per-frame-airborne breadth explosion). On the ground these are ~constant / posY=lane, so
    // keep full resolution there (posY lanes matter). _airQ=0 => no quantization.
    const bool aq = (*_bikeAirMode > 0 && _airQ > 0);
    if (!_dropVelZ) hashEngine.Update((uint8_t)(aq ? (*_bikeVelZ >> _airQ) : *_bikeVelZ));
    if (!_dropPosZ1) hashEngine.Update((uint8_t)(aq ? (*_bikePosZ1 >> _airQ) : *_bikePosZ1));
    if (!_dropPosZ2) hashEngine.Update((uint8_t)(aq ? (*_bikePosZ2 >> _airQ) : *_bikePosZ2));
    // DIAGNOSTIC: record per-byte distinct values over airborne states (unquantized raw values).
    if (*_bikeAirMode > 0)
    {
      using namespace ebdiag;
      g_hb[0].rec(*_bikePosZ2);    g_hb[1].rec(*_bikePosY);     g_hb[2].rec(*_bikeVelY1);
      g_hb[3].rec(*_bikeVelY2);    g_hb[4].rec(*_bikeVelZ);     g_hb[5].rec(*_bikePosZ1);
      g_hb[6].rec(*_bikeAngle);    g_hb[7].rec(*_bikeVelX1);    g_hb[8].rec(*_bikeVelX2);
      g_hb[9].rec(*_bikeFlightMode1); g_hb[10].rec(*_bikeFlightMode2); g_hb[11].rec(*_bikeFlightMode3);
      g_hb[12].rec(*_gameCycle);   g_hb[13].rec(*_bikePosX2);
    }
    if (!_dropPosY) hashEngine.Update((uint8_t)(aq ? (*_bikePosY >> _airQ) : *_bikePosY));
    if (!_dropVelY1) hashEngine.Update((uint8_t)(aq ? (*_bikeVelY1 >> _airQ) : *_bikeVelY1));
    if (!_dropVelY2) hashEngine.Update((uint8_t)(aq ? (*_bikeVelY2 >> _airQ) : *_bikeVelY2));
    if (!_dropFlight)
    {
      hashEngine.Update(*_bikeFlightMode1);
      // flight2 (0x380) is the FRACTIONAL vertical-arc accumulator (256 distinct = the dominant airborne
      // breadth driver) but reward-irrelevant at sub-resolution; flight3 (0x384) is the integer arc part
      // (few distinct, set at takeoff -> reward-relevant, keep full). Drop or quantize ONLY flight2.
      if (!_dropFlight2)
      {
        const int f2shift = (*_bikeAirMode > 0 && _flight2Q > 0) ? _flight2Q : 0;
        hashEngine.Update((uint8_t)(*_bikeFlightMode2 >> f2shift));
      }
      hashEngine.Update(*_bikeFlightMode3);
    }
    if (!_dropGameCycle) hashEngine.Update(*_gameCycle);
    hashEngine.Update(*_currBlockX);
    // Lane-lock flag: two ground states with identical RAM but different lock status have DIFFERENT futures
    // (locked => U/D removed, can no longer change lane), so they must NOT dedup.
    hashEngine.Update((uint8_t)_laneLocked);
    hashEngine.Update((uint8_t)_angleLocked); // locked vs adjusting at the same angle are different futures
    // hashEngine.Update(*_prevBlockX        );
    // hashEngine.Update(_blockXTransitions );
    // hashEngine.Update(_curVel          );
    // hashEngine.Update(_maxVel          );

    //  for (size_t i = 0; i < 0x800; i++)
    //   if (i != 0x0014)
    //   if (i != 0x0015)
    //   if (i != 0x005C)
    //   if (i != 0x00FC)
    //   if (i != 0x00FE)
    //   if (i != 0x0020)
    //   if (i != 0x003F)
    //   if (i != 0x004C)
    //   if (i != 0x006A)
    //   if (i != 0x006B)
    //   if (i != 0x031B)
    //   if (i != 0x03A9)
    //   if (i != 0x03B5)
    //   if (i != 0x03D7)
    //   if (i != 0x03DF)
    //    hashEngine.Update(_emu->_baseMem[i]);
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    // On the first hook (e.g. right after seeding from an Initial State File) just latch _prevBlockX so we
    // don't count a spurious transition from the constructor's 0; thereafter count real block changes.
    if (!_firstPostHook && _prevBlockX != *_currBlockX) _blockXTransitions++;
    _prevBlockX    = *_currBlockX;
    _firstPostHook = false;

    // Park the CPU opponents out of the player's lane band every frame (runs after each advanceState, so the
    // off-track positions are baked into the serialized state and persist frame-to-frame -> no collisions).
    if (_removeOpponents)
    {
      _lowMem[0x008D] = (uint8_t)_opponentParkY;
      _lowMem[0x008E] = (uint8_t)_opponentParkY;
      _lowMem[0x008F] = (uint8_t)_opponentParkY;
    }

    _curVel = 256.0 * (uint16_t)*_bikeVelX1 + (uint16_t)*_bikeVelX2;
    if (_curVel > _maxVel) _maxVel = _curVel;

    _bikePosX = (float)_blockXTransitions * 256.0f + (float)*_bikePosX1 + (float)*_bikePosX2 / 256.0;
  }

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override
  {
    if (_useTrace == false)
    {
      _magnetPenalty = 0.0f;
      return;
    }
    // Index the reference trace by this state's own step counter (clamped to the trace bounds), then measure the
    // bike's distance from the reference on each anchored axis. Consumed by calculateGameSpecificReward.
    _traceStep      = (uint32_t)std::max(std::min((int)_currentStep + _traceMagnet.offset, (int)_trace.size() - 1), 0);
    _traceDistanceX = std::fabs(_trace[_traceStep].posX - _bikePosX);
    _traceDistanceY = std::fabs(_trace[_traceStep].posY - (float)*_bikePosY);
    _traceDistanceZ = std::fabs(_trace[_traceStep].posZ2 - (float)*_bikePosZ2);
    _traceDistanceV = std::fabs(_trace[_traceStep].velX - (float)((int)*_bikeVelX2 * 256 + (int)*_bikeVelX1));
    // flight2 is a CYCLIC accumulator (0-255, wraps ~every 5 frames): use the shorter of the two arc distances
    // so a just-wrapped state isn't spuriously ranked "far" from the reference right where the carry happens.
    const float dF  = std::fabs(_trace[_traceStep].flight2 - (float)*_bikeFlightMode2);
    _traceDistanceF = std::min(dF, 256.0f - dF);
    // Total magnet penalty (subtracted from the ranking reward; added back by getFloorReward so the Reference
    // Reward Floor sees the true position and never false-cancels a position-correct-but-off-anchor state).
    _magnetPenalty = _traceMagnet.intensityX * _traceDistanceX + _traceMagnet.intensityY * _traceDistanceY + _traceMagnet.intensityZ * _traceDistanceZ +
                     _traceMagnet.intensityV * _traceDistanceV + _traceMagnet.intensityF * _traceDistanceF;
  }

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    if (_bypassEmulatorState == true)
    {
      serializer.push(&_lowMem[0x0000], 0x005D0);
      serializer.push(&_lowMem[0x07E0], 0x00020);
    }

    serializer.push(&_currentStep, sizeof(_currentStep));
    serializer.push(&_lastInput, sizeof(_lastInput));
    serializer.push(&_curVel, sizeof(_curVel));
    serializer.push(&_maxVel, sizeof(_maxVel));
    serializer.push(&_blockXTransitions, sizeof(_blockXTransitions));
    serializer.push(&_prevBlockX, sizeof(_prevBlockX));
    serializer.push(&_laneLocked, sizeof(_laneLocked));
    serializer.push(&_angleLocked, sizeof(_angleLocked));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    if (_bypassEmulatorState == true)
    {
      deserializer.pop(&_lowMem[0x0000], 0x005D0);
      deserializer.pop(&_lowMem[0x07E0], 0x00020);
    }

    deserializer.pop(&_currentStep, sizeof(_currentStep));
    deserializer.pop(&_lastInput, sizeof(_lastInput));
    deserializer.pop(&_curVel, sizeof(_curVel));
    deserializer.pop(&_maxVel, sizeof(_maxVel));
    deserializer.pop(&_blockXTransitions, sizeof(_blockXTransitions));
    deserializer.pop(&_prevBlockX, sizeof(_prevBlockX));
    deserializer.pop(&_laneLocked, sizeof(_laneLocked));
    deserializer.pop(&_angleLocked, sizeof(_angleLocked));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // RANKING reward = bike track position MINUS the trace magnet penalty (a bias that keeps reference-adjacent
    // states competitive for a State DB slot). This drives eviction only. The FLOOR comparison uses getFloorReward
    // (position without the penalty), so anchoring an axis that varies independently of posX (e.g. flight2) biases
    // retention without ever false-cancelling a position-correct state. _magnetPenalty is computed in ruleUpdatePostHook.
    return _bikePosX - _magnetPenalty;
  }

  // Un-biased progress reward for the Reference Reward Floor: the true position (ranking reward + the magnet
  // penalty added back). A state at the reference position scores exactly _bikePosX regardless of its magnet
  // penalty, so the floor measures position only. See ruleUpdatePostHook / calculateGameSpecificReward.
  __INLINE__ float getFloorReward() const override { return getReward() + _magnetPenalty; }

  // One line of the reference trace for `player --dumpTrace`: the coordinates the trace magnet anchors on
  // (posX posY posZ2 velX16 flight2). %.17g on posX preserves the exact 1/256 sub-pixel value (see --dumpReward).
  __INLINE__ std::string getTraceLine() const override
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "%.17g %u %u %d %u", (double)_bikePosX, (unsigned)*_bikePosY, (unsigned)*_bikePosZ2, (int)*_bikeVelX2 * 256 + (int)*_bikeVelX1,
             (unsigned)*_bikeFlightMode2);
    return std::string(buf);
  }

  // The ENTIRE allowed-input set is built here (config "Allowed Input Sets" is empty). Commit-and-hold:
  // branch over target ANGLES only on the first airborne frame of a jump, target LANES only on the first
  // ground frame of a stretch; in between, return the single "hold" token (no branching => no angle/lane
  // wobble inflating the frontier). The committed targets persist in the state until the next transition.
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    // Raw (pristine) mode: no additional inputs -- the search uses only the config's base input set.
    if (!_searchMode) return;

    if (*_bikeAirMode > 0)
    {
      // AIRBORNE: angle commit by free-adjust-then-lock, mirroring the ground lane-lock. While the angle is
      // unlocked the search may raise (L), lower (R), or HOLD (which LOCKS the angle); once locked, only hold
      // is offered (no L/R) so the angle stops branching. The lock is reset at takeoff (fresh climb angle) and
      // at the apex ($BC: climb-momentum>0 -> ==0, fresh descent angle), so a jump commits one climb and one
      // descent angle. The lock is OPTIONAL (the search can keep adjusting), so the reference arc stays
      // reachable; the win is when a piecewise-constant arc lands identically -> 1 successor instead of 3.
      // HYBRID climb-lock / descent-free: the angle wiggle during the CLIMB ($BC>0) is far from the landing
      // and inconsequential, so we commit it (free-adjust-then-lock) to collapse breadth; the DESCENT ($BC==0)
      // determines the landing (where committed angles accumulate big losses), so it stays FULLY FREE per
      // frame. So: way up -> lock; way down -> unlock; next jump -> lock again.
      // Phases in which the lock applies: the climb always; the high descent when the distance-to-ground
      // mechanism is on; never the final approach near the ground (so the landing stays free/exact).
      const bool lockPhase = airAngleLockPhase();
      if (lockPhase && _angleLocked)
      {
        allowedInputSet.push_back(_airAngleHold); // locked phase: hold the committed angle
      }
      else
      {
        // Free-adjust. In a lock phase, choosing hold LOCKS; otherwise (free descent) it is just a per-frame hold.
        allowedInputSet.push_back(_airAngleRaise); // L
        allowedInputSet.push_back(_airAngleLower); // R
        allowedInputSet.push_back(_airAngleHold);  // hold
      }
      return;
    }

    // GROUND: automatic throttle (coast to keep launch speed when velX>832, else B to hold the 832 cap), plus
    // the lane commit by U/D-drop. While the lane is unlocked the search may steer U or D, or LOCK in place
    // (the no-lane-steer input); once locked, U/D are removed for the rest of the ground stretch.
    const int  velX16    = (int)*_bikeVelX2 * 256 + (int)*_bikeVelX1;
    const bool autoCoast = (velX16 > 832);
    // Throttle per control: B (accelerate, caps velX at 832) vs coast (no accel; a downhill then pushes velX
    // past 832 -- launch speed). Auto picks coast iff velX>832. But the reference COASTS at velX==832 in a
    // B/coast pulse on downhills (heat/sub-pixel management) -- which the auto-rule cannot express. _freeThrottle
    // => offer BOTH variants so the search can reproduce that pulse. The coast variant of a control is its
    // airborne (no-accel) input; the B variant is the ground input.
    auto pushThr = [&](InputSet::inputIndex_t bVar, InputSet::inputIndex_t coastVar) {
      if (_freeThrottle)
      {
        allowedInputSet.push_back(bVar);
        allowedInputSet.push_back(coastVar);
      }
      else
        allowedInputSet.push_back(autoCoast ? coastVar : bVar);
    };
    if (!_laneLocked)
    {
      pushThr(_groundCtrl[0][2][1], _airCtrl[2][1]); // up   (U)
      pushThr(_groundCtrl[0][0][1], _airCtrl[0][1]); // down (D)
    }
    // Ground ANGLE steering (L/R on $AC): the reference steers the ground angle to set up takeoff arcs on ramps
    // (e.g. L drives $AC 8->11, releases to 9 for the frame-569 ramp). FREE per-frame (NOT a commit) -- the
    // reference re-steers it (L for a stretch, releases, then R later), so a lock would forbid the re-steer.
    // Bounded by hashing the ground angle (see computeAdditionalHashing) so states converge once it re-settles.
    if (_groundAngleSteering)
    {
      pushThr(_groundCtrl[0][1][2], _airCtrl[1][2]); // L: raise ground angle
      pushThr(_groundCtrl[0][1][0], _airCtrl[1][0]); // R: lower ground angle
    }
    // Ground DIAGONALS: the reference presses lane AND angle simultaneously (e.g. U+R+B at frame 1130 to set up
    // a takeoff), which single-axis inputs cannot express. Offer the 4 lane*angle combos when the lane is still
    // unlocked. Needs angle steering on (the diagonals are lane+angle).
    if (_groundDiagonals && _groundAngleSteering && !_laneLocked)
    {
      pushThr(_groundCtrl[0][2][2], _airCtrl[2][2]); // U+L
      pushThr(_groundCtrl[0][2][0], _airCtrl[2][0]); // U+R
      pushThr(_groundCtrl[0][0][2], _airCtrl[0][2]); // D+L
      pushThr(_groundCtrl[0][0][0], _airCtrl[0][0]); // D+R
    }
    pushThr(_groundLockB, _nullInputIdx); // no lane steer: LOCKS the lane if not yet locked (see advanceStateImpl), else hold
  }

  // Full alphabet of inputs the game may request (the real controls -- no tokens any more).
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    std::set<std::string> s = {"|..|........|"}; // null
    for (int ld = -1; ld <= 1; ld++)
      for (int ad = -1; ad <= 1; ad++) s.insert(airInput(ld, ad)); // airborne combos (angle + lane steer)
    for (int t = 0; t < 2; t++)
      for (int ld = -1; ld <= 1; ld++)
        for (int ad = -1; ad <= 1; ad++) s.insert(groundInput(t == 1, ld, ad)); // ground combos (accel + steer)
    return s;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Current Step:                     %04u\n", _currentStep);
    jaffarCommon::logger::log("[J+]  + Game Cycle:                       %02u\n", *_gameCycle);
    jaffarCommon::logger::log("[J+]  + Current / Remaining Loop:         %02u/%02u\n", *_currentLoop, *_loopsRemaining);
    jaffarCommon::logger::log("[J+]  + Loop Advance:                     %02u\n", *_intraLoopAdvance);
    jaffarCommon::logger::log("[J+]  + Block X:                          Curr: %02u, Prev: %02u, Count: %02u\n", *_currBlockX, _prevBlockX, _blockXTransitions);
    jaffarCommon::logger::log("[J+]  + Bike Pos X:                       %.3f (%02u, %02u)\n", _bikePosX, *_bikePosX1, *_bikePosX2);
    jaffarCommon::logger::log("[J+]  + Bike Pos Y:                       %02u\n", *_bikePosY);
    jaffarCommon::logger::log("[J+]  + Bike Pos Z:                       %02u (%02u)\n", *_bikePosZ1, *_bikePosZ2);
    jaffarCommon::logger::log("[J+]  + Bike Vel:                         %05u (Max: %05u)\n", _curVel, _maxVel);
    jaffarCommon::logger::log("[J+]  + Bike Vel X:                       %02u (%02u)\n", *_bikeVelX1, *_bikeVelX2);
    jaffarCommon::logger::log("[J+]  + Bike Vel Y:                       %02u (%02u)\n", *_bikeVelY1, *_bikeVelY2);
    jaffarCommon::logger::log("[J+]  + Bike Vel Z:                       %02u\n", *_bikeVelZ);
    jaffarCommon::logger::log("[J+]  + Bike Air Mode:                    %02u\n", *_bikeAirMode);
    jaffarCommon::logger::log("[J+]  + Bike Flight:                      %02u %02u %02u\n", *_bikeFlightMode1, *_bikeFlightMode2, *_bikeFlightMode3);
    jaffarCommon::logger::log("[J+]  + Bike Engine Temp:                 %02u\n", *_bikeEngineTemp);
    jaffarCommon::logger::log("[J+]  + Bike Angle:                       %02u\n", *_bikeAngle);
    jaffarCommon::logger::log("[J+]  + Bike Moving:                      %02u\n", *_bikeMoving);
    jaffarCommon::logger::log("[J+]  + Race Over Flag:                   %02u\n", *_raceOverFlag);
    if (_useTrace == true &&
        (_traceMagnet.intensityX != 0.0f || _traceMagnet.intensityY != 0.0f || _traceMagnet.intensityZ != 0.0f || _traceMagnet.intensityV != 0.0f || _traceMagnet.intensityF != 0.0f))
      jaffarCommon::logger::log("[J+]  + Trace Magnet:                     I(X %.4f Y %.4f Z %.4f V %.4f F %.4f) off %+d | step %u | dist(X %.3f Y %.1f Z %.1f V %.1f F %.1f)\n",
                                _traceMagnet.intensityX, _traceMagnet.intensityY, _traceMagnet.intensityZ, _traceMagnet.intensityV, _traceMagnet.intensityF, _traceMagnet.offset,
                                _traceStep, _traceDistanceX, _traceDistanceY, _traceDistanceZ, _traceDistanceV, _traceDistanceF);
    // DIAGNOSTIC: where the airborne hash density lives (distinct values per byte, cumulative).
    {
      using namespace ebdiag;
      char line[512]; int off = 0;
      off += snprintf(line + off, sizeof(line) - off, "[J+]  + [HASH-DENSITY airborne distinct/256]:");
      for (int i = 0; i < NHB; i++) off += snprintf(line + off, sizeof(line) - off, " %s=%d", g_hbName[i], g_hb[i].distinct());
      jaffarCommon::logger::log("%s\n", line);
    }
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    // Sets the trace-magnet strength on each anchored axis (posX, posY, posZ2) plus an optional step offset. A
    // higher intensity holds the reference line harder (more penalty for straying); keep it SMALL so raw posX
    // still drives the search. Requires a "Trace File Path". See calculateGameSpecificReward.
    if (actionType == "Set Trace Magnet")
    {
      if (_useTrace == false) JAFFAR_THROW_LOGIC("Specified 'Set Trace Magnet' action but no 'Trace File Path' was provided.\n");
      const auto iX     = jaffarCommon::json::getNumber<float>(actionJs, "Intensity X");
      const auto iY     = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Y");
      const auto iZ     = jaffarCommon::json::getNumber<float>(actionJs, "Intensity Z");
      const auto iV     = actionJs.contains("Intensity V") ? jaffarCommon::json::getNumber<float>(actionJs, "Intensity V") : 0.0f;
      const auto iF     = actionJs.contains("Intensity F") ? jaffarCommon::json::getNumber<float>(actionJs, "Intensity F") : 0.0f;
      const auto offset = actionJs.contains("Offset") ? jaffarCommon::json::getNumber<int>(actionJs, "Offset") : 0;
      rule.addAction([=, this]() { this->_traceMagnet = traceMagnet_t{.intensityX = iX, .intensityY = iY, .intensityZ = iZ, .intensityV = iV, .intensityF = iF, .offset = offset}; });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override { return {0, 0}; }

  __INLINE__ void playerPrintCommands() const override {
    // jaffarCommon::logger::log("[J+] t: Print Initial Info\n");
  };

  __INLINE__ bool playerParseCommand(const int command) { return false; };

  uint8_t*               _lowMem;
  uint16_t               _currentStep;
  InputSet::inputIndex_t _nullInputIdx;
  InputSet::inputIndex_t _lastInput;

  uint8_t* _bikePosX1;
  uint8_t* _bikePosX2;
  uint8_t* _intraLoopAdvance;
  uint8_t* _loopsRemaining;
  uint8_t* _currentLoop;
  uint8_t* _raceOverFlag;
  uint8_t* _bikeMoving;
  uint8_t* _bikeAngle;
  uint8_t* _bikeAirMode;
  uint8_t* _bikeVelX2;
  uint8_t* _bikeVelX1;
  uint8_t* _bikeEngineTemp;
  uint8_t* _bikeVelZ;
  uint8_t* _bikePosZ1;
  uint8_t* _bikePosZ2;
  uint8_t* _bikePosY;
  uint8_t* _bikeVelY1;
  uint8_t* _bikeVelY2;
  uint8_t* _bikeFlightMode1;
  uint8_t* _bikeFlightMode2;
  uint8_t* _bikeFlightMode3;
  uint8_t* _gameCycle;
  uint8_t* _currBlockX;

  uint8_t  _blockXTransitions;
  uint16_t _curVel;
  uint16_t _maxVel;
  float    _bikePosX;
  uint8_t  _prevBlockX;

  // --- Commit-and-hold (single-frame angle/lane commitment; see getAdditionalAllowedInputs) ----------
  // --- Search action space (single-frame; see getAdditionalAllowedInputs / advanceStateImpl) -----------
  InputSet::inputIndex_t _airCtrl[3][3];       // [laneDir+1][angDir+1] airborne controls (posY+angle, no accel)
  InputSet::inputIndex_t _groundCtrl[2][3][3]; // [turbo?][laneDir+1][angDir+1] ground controls (accel + steer)
  InputSet::inputIndex_t _airAngleRaise, _airAngleLower, _airAngleHold; // free per-frame airborne angle controls
  InputSet::inputIndex_t _groundLockB;                                  // B + no lane steer = "lock lane" while accelerating
  bool                   _searchMode; // master gate: false => raw inputs (pristine replay); true => search action space
  int                    _descentFreeBelowDist; // free the descent angle only when distToGround <= this (-1 => whole descent free)
  bool                   _airborneAngleCommit;  // false => fully free per-frame airborne angle (tol=0-exact)
  bool                   _groundAngleSteering;  // offer + hash ground angle steering (L/R) for takeoff setup
  bool                   _freeThrottle;         // offer both B and coast per ground frame (reference pulses at the cap)
  bool                   _groundDiagonals;      // offer lane+angle ground combos (reference uses U+R etc.)
  bool                   _laneLocked; // ground: true once the lane is committed (U/D removed until next landing)
  bool                   _angleLocked; // air: true once the angle is committed (L/R removed until apex/landing)
  int                    _airQ;       // airborne hash quantization (right-shift bits)
  bool _dropVelZ, _dropPosZ1, _dropPosZ2, _dropPosY, _dropVelY1, _dropVelY2, _dropAngle, _dropFlight, _dropFlight2, _dropGameCycle; // per-element hash drops
  int  _flight2Q; // dedicated flight2 airborne quantization (right-shift bits)
  int      _initialBlockTransitions; // seed value for _blockXTransitions (mid-run seed)
  bool     _firstPostHook;           // true until the first stateUpdatePostHook call
  uint8_t* _climbRemaining;          // $BC: >0 climbing, 0 at apex/descent

  bool _removeOpponents; // park CPU opponents off-track each frame
  int  _opponentParkY;   // posY to park opponents at (default 255)

  // --- Trace magnet (see constructor / ruleUpdatePostHook / calculateGameSpecificReward / "Set Trace Magnet") ----
  struct traceEntry_t
  {
    float posX;
    float posY;
    float posZ2;
    float velX;    // velX16 = velX2*256+velX1: the launch/coast speed carrier (reference reaches 1544 downhill)
    float flight2; // 0x380: the fractional airborne accumulator (sub-pixel carry phase); retains exact airborne sub-pixel timing
  };
  struct traceMagnet_t
  {
    float intensityX = 0.0f; // reward penalty per unit of posX    distance from the reference (usually 0: don't cap progress)
    float intensityY = 0.0f; // reward penalty per unit of posY    distance from the reference (lane profile)
    float intensityZ = 0.0f; // reward penalty per unit of posZ2   distance from the reference (airborne profile: the line distinguisher)
    float intensityV = 0.0f; // reward penalty per unit of velX    distance from the reference (speed: retains the exact launch/coast velocity)
    float intensityF = 0.0f; // reward penalty per unit of flight2 distance from the reference (sub-pixel airborne accumulator phase)
    int   offset     = 0;    // shift the trace index (currentStep + offset) to lead/lag the reference
  };
  std::string               _traceFilePath = "";
  bool                      _useTrace      = false;
  std::vector<traceEntry_t> _trace;
  traceMagnet_t             _traceMagnet;
  uint32_t                  _traceStep      = 0;
  float                     _traceDistanceX = 0.0f;
  float                     _traceDistanceY = 0.0f;
  float                     _traceDistanceZ = 0.0f;
  float                     _traceDistanceV = 0.0f;
  float                     _traceDistanceF = 0.0f;
  float                     _magnetPenalty  = 0.0f; // total penalty subtracted from the ranking reward this step (added back by getFloorReward)
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
