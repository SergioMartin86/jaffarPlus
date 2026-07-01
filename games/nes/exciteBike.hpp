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

class ExciteBike final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Excite Bike"; }

  /// @brief A unique input "token" string for the commit-and-hold scheme. Tokens are SELECTORS chosen by
  /// the search at air<->ground transitions (to pick a target angle / lane / "hold"); they are NEVER
  /// applied to the emulator -- advanceStateImpl decodes the token and applies the real control input
  /// (auto-level L/R, or lane U/D + accel) instead. So a token's button content only needs to be unique:
  /// the A bit (index 11) is set as a marker so tokens never collide with the real controls (null/L/R/B/
  /// B+U/B+D, none of which set A), and the index is encoded in U,D,L,R,B (indices 4,5,6,7,10).
  static __INLINE__ std::string commitToken(uint32_t i)
  {
    std::string s = "|..|........|";
    // Start (index 8) = marker so tokens never collide with the real controls (which use U/D/L/R/B/A only);
    // the index is encoded across the other 7 buttons -> 128 distinct tokens (enough for full per-posY lanes).
    if (i & 1) s[4] = 'U';
    if (i & 2) s[5] = 'D';
    if (i & 4) s[6] = 'L';
    if (i & 8) s[7] = 'R';
    if (i & 16) s[9] = 's'; // Select
    if (i & 32) s[10] = 'B';
    if (i & 64) s[11] = 'A';
    s[8] = 'S'; // Start = marker (tokens are never applied to the emulator; advanceStateImpl maps token -> control)
    return s;
  }

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
    // Commit-and-hold parameters (the whole allowed-input set is built in this .hpp, not in the config's
    // "Allowed Input Sets"). At each ground->air transition the search commits to a TARGET ANGLE held for
    // the jump; at each air->ground transition it commits to a TARGET LANE (posY) held on the ground.
    _commitAngleMin    = _gameConfigRemaining.contains("Commit Angle Min") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Commit Angle Min") : 2;
    _commitAngleMax    = _gameConfigRemaining.contains("Commit Angle Max") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Commit Angle Max") : 11;
    _commitLaneCount   = _gameConfigRemaining.contains("Commit Lane Count") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Commit Lane Count") : 6;
    const int laneYmin = _gameConfigRemaining.contains("Commit Lane PosY Min") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Commit Lane PosY Min") : 94;
    const int laneYmax = _gameConfigRemaining.contains("Commit Lane PosY Max") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Commit Lane PosY Max") : 151;
    // Full lane freedom: one committable lane per integer posY in [min,max] (the bike may commit to ANY ground
    // position, still held for the whole ground stretch). Otherwise, _commitLaneCount evenly-spaced lanes.
    const bool fullLaneFreedom = _gameConfigRemaining.contains("Full Lane Freedom") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Full Lane Freedom") : false;
    if (fullLaneFreedom)
    {
      for (int y = laneYmin; y <= laneYmax; y++) _laneTargets.push_back(y);
      _commitLaneCount = (int)_laneTargets.size();
    }
    else
    {
      for (int l = 0; l < _commitLaneCount; l++)
      {
        const int y = (_commitLaneCount == 1) ? laneYmin : laneYmin + (int)((double)l * (laneYmax - laneYmin) / (_commitLaneCount - 1) + 0.5);
        _laneTargets.push_back(y);
      }
    }

    // Opponent removal: the CPU racers (posY at 0x8D/0x8E/0x8F; player posY=0x8C) can knock a lane-locked
    // player off the bike even going straight. When enabled, every frame we park them out of the player's
    // lane band (default posY 255 = off the track) so they can never collide. See stateUpdatePostHook.
    _removeOpponents = _gameConfigRemaining.contains("Remove Opponents") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Remove Opponents") : false;
    _opponentParkY   = _gameConfigRemaining.contains("Opponent Park Y") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Opponent Park Y") : 255;

    // Auto-feather turbo: on the ground use A (turbo, faster but heats the engine, temp at 0x3B6) while temp
    // is below this limit, else fall back to B (normal accel, holds temp flat -> never overheats); airborne
    // jumps cool it back down. TAS peaks at 29, so default 29 (tune up toward the true redline to push harder).
    _turboTempLimit = _gameConfigRemaining.contains("Turbo Temp Limit") ? jaffarCommon::json::popNumber<int>(_gameConfigRemaining, "Turbo Temp Limit") : 29;

    // Per-frame airborne angle: instead of committing one airborne angle and holding it, branch the angle
    // EVERY airborne frame over {raise (L), lower (R), hold} -- full per-frame control of the angle arc
    // (nose-up climb -> nose-down descent), the expressiveness commit-and-hold's single airborne angle lacks.
    // Master gate for the search's commit-and-hold token system. When OFF (default) AND per-frame airborne is
    // off, advanceStateImpl applies inputs RAW (pristine behavior) so reference movies (race01a.tas.sol) replay
    // bit-exact. Search configs turn this (or Airborne Per Frame Angle) on to use the token action space.
    _commitAndHold         = _gameConfigRemaining.contains("Commit And Hold") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Commit And Hold") : false;
    _airbornePerFrameAngle = _gameConfigRemaining.contains("Airborne Per Frame Angle") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Airborne Per Frame Angle") : false;
    // Macro-airborne: collapse each jump into ONE search step. Disassembly proof: airborne velX is constant
    // (player air-friction ~0), so posX during a jump is a deterministic function of (entry posX, velX,
    // frames-aloft) and carries NO search information -- the only posX-relevant output of a jump is its
    // LANDING (frame + re-entry point). So instead of branching the angle every airborne frame (the
    // 3^airtime posZ2 explosion), branch only a small set of DESCENT-ANGLE policies at takeoff; each policy
    // runs the WHOLE jump internally (auto-leveling: climb to the ceiling while $BC>0, then descend toward the
    // committed target) and yields ONE landing successor. Ground stays commit-and-hold (one frame/step). This
    // kills the airborne breadth so the search can go deep enough to optimize the ground/sub-pixel run to the
    // finish -- the only place a frame can hide. See excitebike-disassembly-findings.
    _macroAirborne = _gameConfigRemaining.contains("Macro Airborne") ? jaffarCommon::json::popBoolean(_gameConfigRemaining, "Macro Airborne") : false;
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
    _dropGameCycle = has("GameCycle");

    // Trace magnet: a per-step reference trajectory ("posX posY posZ2 velX flight2", one line per search step,
    // generated by `player --dumpTrace`). A SMALL reward penalty proportional to the bike's distance from the trace at
    // the current step keeps reference-adjacent states competitive in the (reward-ranked) State DB, so a non-greedy
    // optimum (the reference line, temporarily lower posX than an apparent-better route) is not evicted once the DB
    // fills. Reward stays the sole eviction driver -- the magnet just grants legitimate reward for staying near the
    // known-good line. The reference itself has distance 0 (magnet 0), so the Reference Reward Floor is unaffected.
    // Intensities are set by the "Set Trace Magnet" rule action (default all 0 => magnet off, even with a trace loaded).
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
    _inNull = _nullInputIdx;
    // Airborne controls: [laneDir -1..1][angDir -1..1] -> steer posY (U/D) + angle (L/R), no accel.
    for (int ld = -1; ld <= 1; ld++)
      for (int ad = -1; ad <= 1; ad++) _airCtrl[ld + 1][ad + 1] = _emulator->registerInput(airInput(ld, ad));
    // Ground controls: [turbo?][laneDir -1..1][angDir -1..1] -> accel + lane steer (U/D) + angle steer (L/R).
    for (int t = 0; t < 2; t++)
      for (int ld = -1; ld <= 1; ld++)
        for (int ad = -1; ad <= 1; ad++) _groundCtrl[t][ld + 1][ad + 1] = _emulator->registerInput(groundInput(t == 1, ld, ad));

    // Commit tokens (selectors): one per target angle, one per target lane, plus a single "hold" token.
    // The search branches over these ONLY at transitions; advanceStateImpl decodes them.
    _angleTokens.clear();
    _laneTokens.clear();
    _tokenAngle.clear();
    _tokenLane.clear();
    uint32_t ti = 0;
    for (int a = _commitAngleMin; a <= _commitAngleMax; a++)
    {
      const auto idx = _emulator->registerInput(commitToken(ti++));
      _angleTokens.push_back(idx);
      _tokenAngle[idx] = a;
    }
    for (int l = 0; l < _commitLaneCount; l++)
    {
      const auto idx = _emulator->registerInput(commitToken(ti++));
      _laneTokens.push_back(idx);
      _tokenLane[idx] = _laneTargets[l];
    }
    _holdToken = _emulator->registerInput(commitToken(ti++));

    _committedAngle        = -1; // uncommitted: airborne jump angle (branched first when airborne)
    _committedAirLane      = -1; // uncommitted: airborne posY target (branched after the jump angle)
    _committedDescentAngle = -1; // uncommitted: airborne descent/landing angle (branched at the apex, BC==0)
    _committedLane         = -1; // uncommitted: ground lane (branched first when on the ground)
    _committedGroundAngle  = -1; // uncommitted: ground angle (branched after the lane)

    stateUpdatePostHook();
  }

  // The real control input to apply this frame, derived from the committed angle/lane and the CURRENT
  // (pre-advance) state. Airborne: auto-level toward the committed target angle (L raises, R lowers).
  // Ground: accelerate while steering toward the committed target lane (posY) -- U decreases posY, D
  // increases it. (The committed values were just set by a transition token, or held from earlier.)
  __INLINE__ InputSet::inputIndex_t computeControl() const
  {
    if (*_bikeAirMode > 0)
    {
      // Airborne: steer angle (L/R) toward the climb angle while climbing ($BC>0), or the descent/landing
      // angle once past the apex ($BC==0); steer posY (U/D) toward the committed air lane. All held.
      const int angTarget = (int)_committedAngle;
      int       angDir    = 0;
      if (angTarget >= 0)
      {
        if ((int)*_bikeAngle < angTarget)
          angDir = 1; // angle too low  -> L (raise)
        else if ((int)*_bikeAngle > angTarget)
          angDir = -1; // angle too high -> R (lower)
      }
      int laneDir = 0;
      if (_committedAirLane >= 0)
      {
        if ((int)*_bikePosY > _committedAirLane)
          laneDir = 1; // posY too high -> U (decrease)
        else if ((int)*_bikePosY < _committedAirLane)
          laneDir = -1; // posY too low  -> D (increase)
      }
      return _airCtrl[laneDir + 1][angDir + 1];
    }
    // Ground: accel (auto-feather turbo A under the redline, else B) + steer toward the committed lane (U/D)
    // AND the committed ground angle (L/R), both held until the next air<->ground transition.
    const bool turbo   = ((int)*_bikeEngineTemp < _turboTempLimit);
    int        laneDir = 0;
    if (_committedLane >= 0)
    {
      if ((int)*_bikePosY > _committedLane)
        laneDir = 1; // posY too high -> U (decrease)
      else if ((int)*_bikePosY < _committedLane)
        laneDir = -1; // posY too low  -> D (increase)
    }
    int angDir = 0;
    if (_committedGroundAngle >= 0)
    {
      if ((int)*_bikeAngle < _committedGroundAngle)
        angDir = 1; // angle too low  -> L (raise)
      else if ((int)*_bikeAngle > _committedGroundAngle)
        angDir = -1; // angle too high -> R (lower)
    }
    return _groundCtrl[turbo ? 1 : 0][laneDir + 1][angDir + 1];
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Raw (pristine) mode: when neither the commit-and-hold token system nor per-frame airborne is enabled,
    // apply the input directly. This keeps reference movies (race01a.tas.sol) replaying bit-exact.
    if (!_commitAndHold && !_airbornePerFrameAngle)
    {
      _emulator->advanceState(input);
      _currentStep++;
      _lastInput = input;
      return;
    }

    // Per-frame airborne angle: while airborne, the chosen input is a REAL control (raise/lower/hold) applied
    // directly (no commit-and-hold). Ground stays commit-and-hold.
    if (_airbornePerFrameAngle && *_bikeAirMode > 0)
    {
      const uint8_t prevAir = *_bikeAirMode;
      _emulator->advanceState(input);
      _currentStep++;
      if (prevAir > 0 && *_bikeAirMode == 0)
      {
        _committedLane        = -1;
        _committedGroundAngle = -1;
      } // air -> ground
      _lastInput = input;
      return;
    }

    // The search's chosen input is a commit TOKEN (at a transition) or the hold token (mid-phase). Decode
    // it into the committed target; the actual emulator input is computeControl() (auto-level / lane drive).
    // An angle token sets the AIRBORNE jump angle if airborne, else the GROUND-stretch angle (same 10 tokens,
    // routed by the current air mode). A lane token sets the committed lane. The hold token changes nothing.
    const auto ai = _tokenAngle.find(input);
    if (ai != _tokenAngle.end())
    {
      if (*_bikeAirMode > 0)
        _committedAngle = (int16_t)ai->second; // airborne jump angle
      else
        _committedGroundAngle = (int16_t)ai->second; // ground-stretch angle
    }
    else
    {
      const auto li = _tokenLane.find(input);
      if (li != _tokenLane.end())
      {
        if (*_bikeAirMode > 0)
          _committedAirLane = (int16_t)li->second; // airborne posY target
        else
          _committedLane = (int16_t)li->second; // ground lane
      }
    }

    const uint8_t prevAir = *_bikeAirMode;
    _emulator->advanceState(computeControl());
    _currentStep++;

    // Re-commit at every air<->ground transition: a new jump picks a fresh airborne angle; a new ground stretch
    // picks a fresh lane AND ground angle (-1 => the next frames in that phase branch over the targets).
    const uint8_t newAir = *_bikeAirMode;
    if (prevAir == 0 && newAir > 0) // ground -> air: commit fresh climb angle, air posY, descent angle
    {
      _committedAngle        = -1;
      _committedAirLane      = -1;
      _committedDescentAngle = -1;
    }
    if (prevAir > 0 && newAir == 0) // air -> ground: commit a fresh lane + ground angle
    {
      _committedLane        = -1;
      _committedGroundAngle = -1;
    }

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
    if (!_dropAngle && *_bikeAirMode > 0) hashEngine.Update(*_bikeAngle);
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
    if (!_dropPosY) hashEngine.Update((uint8_t)(aq ? (*_bikePosY >> _airQ) : *_bikePosY));
    if (!_dropVelY1) hashEngine.Update((uint8_t)(aq ? (*_bikeVelY1 >> _airQ) : *_bikeVelY1));
    if (!_dropVelY2) hashEngine.Update((uint8_t)(aq ? (*_bikeVelY2 >> _airQ) : *_bikeVelY2));
    if (!_dropFlight)
    {
      hashEngine.Update(*_bikeFlightMode1);
      hashEngine.Update(*_bikeFlightMode2);
      hashEngine.Update(*_bikeFlightMode3);
    }
    if (!_dropGameCycle) hashEngine.Update(*_gameCycle);
    hashEngine.Update(*_currBlockX);
    // Commit-and-hold targets: two states with identical RAM but different committed angle/lane have
    // DIFFERENT futures (they auto-level / steer to different targets), so they must NOT dedup.
    hashEngine.Update(_committedAngle);
    hashEngine.Update(_committedAirLane);
    hashEngine.Update(_committedDescentAngle);
    hashEngine.Update(_committedLane);
    hashEngine.Update(_committedGroundAngle);
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
    serializer.push(&_committedAngle, sizeof(_committedAngle));
    serializer.push(&_committedAirLane, sizeof(_committedAirLane));
    serializer.push(&_committedDescentAngle, sizeof(_committedDescentAngle));
    serializer.push(&_committedLane, sizeof(_committedLane));
    serializer.push(&_committedGroundAngle, sizeof(_committedGroundAngle));
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
    deserializer.pop(&_committedAngle, sizeof(_committedAngle));
    deserializer.pop(&_committedAirLane, sizeof(_committedAirLane));
    deserializer.pop(&_committedDescentAngle, sizeof(_committedDescentAngle));
    deserializer.pop(&_committedLane, sizeof(_committedLane));
    deserializer.pop(&_committedGroundAngle, sizeof(_committedGroundAngle));
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
    if (!_commitAndHold && !_airbornePerFrameAngle) return;

    if (_airbornePerFrameAngle && *_bikeAirMode > 0)
    {
      // Per-frame airborne: raise (L), lower (R), or hold the angle this frame (real controls, applied directly).
      allowedInputSet.push_back(_airCtrl[1][2]); // L (raise)
      allowedInputSet.push_back(_airCtrl[1][0]); // R (lower)
      allowedInputSet.push_back(_airCtrl[1][1]); // null (hold)
      return;
    }

    if (*_bikeAirMode > 0)
    {
      // Airborne: commit the jump angle, then the posY/air lane, then hold both (crash-free best machinery).
      if (_committedAngle < 0)
        allowedInputSet.insert(allowedInputSet.end(), _angleTokens.begin(), _angleTokens.end());
      else if (_committedAirLane < 0)
        allowedInputSet.insert(allowedInputSet.end(), _laneTokens.begin(), _laneTokens.end());
      else
        allowedInputSet.push_back(_holdToken);
    }
    else
    {
      // Ground: first commit the lane, then (still on the ground) commit the ground angle, then hold both.
      if (_committedLane < 0)
        allowedInputSet.insert(allowedInputSet.end(), _laneTokens.begin(), _laneTokens.end());
      else if (_committedGroundAngle < 0)
        allowedInputSet.insert(allowedInputSet.end(), _angleTokens.begin(), _angleTokens.end());
      else
        allowedInputSet.push_back(_holdToken);
    }
  }

  // Full alphabet of inputs the game may request (the real controls + every commit token).
  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    std::set<std::string> s = {"|..|........|"}; // null
    for (int ld = -1; ld <= 1; ld++)
      for (int ad = -1; ad <= 1; ad++) s.insert(airInput(ld, ad)); // 9 airborne combos
    for (int t = 0; t < 2; t++)
      for (int ld = -1; ld <= 1; ld++)
        for (int ad = -1; ad <= 1; ad++) s.insert(groundInput(t == 1, ld, ad)); // 18 ground combos
    const uint32_t nTokens = (uint32_t)((_commitAngleMax - _commitAngleMin + 1) + _commitLaneCount + 1);
    for (uint32_t i = 0; i < nTokens; i++) s.insert(commitToken(i)); // commit tokens
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
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    // Sets the trace-magnet strength on each anchored axis (posX, posY, posZ2, velX, flight2) plus an optional step
    // offset. A higher intensity holds the reference line harder (more penalty for straying); keep it SMALL so raw
    // posX still drives the search. Requires a "Trace File Path". See calculateGameSpecificReward.
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
  int                                 _commitAngleMin, _commitAngleMax, _commitLaneCount;
  std::vector<int>                    _laneTargets;           // target posY per lane
  InputSet::inputIndex_t              _inNull;                // null input
  InputSet::inputIndex_t              _airCtrl[3][3];         // [laneDir+1][angDir+1] airborne controls (posY+angle)
  InputSet::inputIndex_t              _groundCtrl[2][3][3];   // [turbo?][laneDir+1][angDir+1] ground controls
  int                                 _turboTempLimit;        // use A (turbo) on the ground while temp < this
  bool                                _commitAndHold;         // master gate: false => raw inputs (pristine replay)
  bool                                _airbornePerFrameAngle; // branch angle (L/R/hold) every airborne frame
  bool                                _macroAirborne = false; // collapse each jump into one search step (full-jump macro)
  int                                 _airQ;                  // airborne hash quantization (right-shift bits)
  bool                                _dropVelZ, _dropPosZ1, _dropPosZ2, _dropPosY, _dropVelY1, _dropVelY2, _dropAngle, _dropFlight, _dropGameCycle; // per-element hash drops
  int                                 _initialBlockTransitions;            // seed value for _blockXTransitions (mid-run seed)
  bool                                _firstPostHook;                      // true until the first stateUpdatePostHook call
  std::vector<InputSet::inputIndex_t> _angleTokens, _laneTokens;           // branch tokens (ordered)
  std::unordered_map<InputSet::inputIndex_t, int> _tokenAngle, _tokenLane; // token -> target value
  InputSet::inputIndex_t                          _holdToken;
  uint8_t*                                        _climbRemaining; // $BC: >0 climbing, 0 at apex/descent
  int16_t                                         _committedAngle, _committedAirLane, _committedDescentAngle, _committedLane, _committedGroundAngle; // -1=uncommitted

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
