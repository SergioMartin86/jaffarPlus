#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <emulator.hpp>
#include <game.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/logger.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <set>
#include <string>
#include <vector>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

// Rockman (Mega Man 1, NES). Runs on the NesHawk ground-truth core (same UNROM/mapper-2 board as
// Prince of Persia). Target is the two-controller ending skip: a single controller-2 Left+Right+Select
// frame reaches the ending (Stage ID $0031 -> 0x0B). The win signal is that stage flip; the reward is a
// phase-alignment gradient toward the skip's setup (see calculateGameSpecificReward), with a
// situation-dependent input alphabet (getAdditionalAllowedInputs). The cycle-accurate core is required:
// the setup depends on NMI interrupting object processing at an exact instruction, which an
// instruction-granular core cannot reproduce.
class Rockman final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Rockman"; }

  Rockman(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // Camera-scroll level above which the arming rewards (bullet/fire-delay/levers) become active.
    // Below it only the traversal (camera) reward applies. Lower it toward a mid-stage seed's camera so
    // the arming gradient is live from the seed instead of flat until the setup spot. Optional; default 780.
    if (_gameConfigRemaining.contains("Setup Cam Threshold")) _setupCamThreshold = jaffarCommon::json::popNumber<uint16_t>(_gameConfigRemaining, "Setup Cam Threshold");
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Emulator's low (2KB) memory
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Native game properties (addresses per the user's RAM watch map + the DataCrystal RAM map).
    registerGameProperty("Game Timer", &_lowMem[0x0023], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Game Mode", &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pause State", &_lowMem[0x0041], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Pause Switch", &_lowMem[0x0045], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("P1 Input", &_lowMem[0x0014], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("P1 Hold", &_lowMem[0x0018], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Camera Pos X1", &_lowMem[0x001B], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos X2", &_lowMem[0x001A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera State", &_lowMem[0x001C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Camera Pos Y", &_lowMem[0x001E], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Player Pos X1", &_lowMem[0x0020], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X2", &_lowMem[0x0022], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X3", &_lowMem[0x0021], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y1", &_lowMem[0x0025], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y2", &_lowMem[0x0024], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Ground State", &_lowMem[0x002C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Invincibility", &_lowMem[0x0055], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Direction", &_lowMem[0x008D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player HP", &_lowMem[0x006C], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Vel X", &_lowMem[0x0098], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Special Frame", &_lowMem[0x00F3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Side Object", &_lowMem[0x0094], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bullet On Screen", &_lowMem[0x0060], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Bullet 1 Pos X", &_lowMem[0x0482], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bullet 2 Pos X", &_lowMem[0x0483], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bullet 3 Pos X", &_lowMem[0x0484], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bullet 1 Pos Y", &_lowMem[0x0602], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bullet 2 Pos Y", &_lowMem[0x0603], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bullet 3 Pos Y", &_lowMem[0x0604], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Lag Frame State", &_lowMem[0x001F], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Lag Frame Type", &_lowMem[0x01FE], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Object 0", &_lowMem[0x0600], Property::datatype_t::dt_uint8, Property::endianness_t::little);

#ifdef _NESHAWK_DETECT_BAD_ACCESS
    // Fires (==1) on the frame the CPU executes from RAM (PC < $0x8000) or runs an unofficial opcode.
    // The game-end glitch executes object RAM at $0600 -- so this is the sharpest "ACE fired" signal,
    // earlier than waiting for the Stage ID ($0031) to flip to the ending. Legit play never sets it.
    registerGameProperty("CPU Bad Access", _emulator->getProperty("CPU Bad Access").pointer, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    _cpuBadAccess = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("CPU Bad Access")]->getPointer();
#endif

    // Caching pointers used by game logic / trace
    _gameTimer     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Timer")]->getPointer();
    _gameMode      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Game Mode")]->getPointer();
    _pauseState    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Pause State")]->getPointer();
    _cameraX1      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Camera Pos X1")]->getPointer();
    _cameraX2      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Camera Pos X2")]->getPointer();
    _playerPosX1   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X1")]->getPointer();
    _playerPosX2   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X2")]->getPointer();
    _playerPosX3   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X3")]->getPointer();
    _playerPosY1   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y1")]->getPointer();
    _playerPosY2   = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y2")]->getPointer();
    _playerDir     = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Direction")]->getPointer();
    _playerHP      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player HP")]->getPointer();
    _playerVelX    = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Vel X")]->getPointer();
    _specialFrame  = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Special Frame")]->getPointer();
    _lagFrameState = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Lag Frame State")]->getPointer();
    _object0       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Object 0")]->getPointer();
    _bulletX       = &_lowMem[0x0482];
    _bulletY       = &_lowMem[0x0602];

    // Registering input alphabet
    registerInputs();
  }

  // Registers the controller inputs the search may issue. Both ports are standard joypads. P1 gets the
  // movement/fire set the glitch setup uses; controller 2 offers null plus the L+R+Select combo that
  // steers JMP ($0018) into the StageClear routine. The trigger combo is offered paired with a few P1
  // options so the search can fire it on any frame while still driving P1 positioning.
  __INLINE__ void registerInputs()
  {
    // P1-only moves (controller 2 idle). Each carries metadata (does it press A / B?) so the allowed set
    // can be pruned per-situation (see getAdditionalAllowedInputs). Pad order is UDLRSsBA -> B at index 6,
    // A at index 7.
    _in_null = _emulator->registerInput("|..|........|........|");
    for (const auto& p1 : _p1Alphabet) _inputTable.push_back({_emulator->registerInput("|..|" + p1 + "|........|"), p1[7] == 'A', p1[6] == 'B', false});

    // Controller-2 trigger (L+R+Select), flagged so it is offered ONLY when the skip is armed.
    for (const auto& p1 : _p2TriggerP1) _inputTable.push_back({_emulator->registerInput("|..|" + p1 + "|..LR.s..|"), p1[7] == 'A', p1[6] == 'B', true});
  }

  __INLINE__ std::set<std::string> getAllPossibleInputs() override
  {
    std::set<std::string> inputs = {"|..|........|........|"};
    for (const auto& p1 : _p1Alphabet) inputs.insert("|..|" + p1 + "|........|");
    for (const auto& p1 : _p2TriggerP1) inputs.insert("|..|" + p1 + "|..LR.s..|");
    return inputs;
  }

  // Situation-dependent input pruning: only offer inputs that can actually change the outcome, which
  // sharply cuts the branching factor. All three rules are verified against the reference movie (it never
  // uses a pruned input -- see the "no unallowed inputs" check):
  //   - A (jump) and B (shoot) only when $09 == 0x80 (an actionable state); both are pruned in the locked
  //     states ($09 in {0x00,0x23}: ladder/stun/animation) where neither can fire (verified: the reference
  //     issues no A or B there). NOTE: Mega Man uses hold-A for jump HEIGHT, so A IS valid mid-air -- we do
  //     NOT prune "A while airborne".
  //   - B (shoot) additionally pruned when 3 shots are already on screen (the on-screen cap; a 4th is inert).
  //   - The controller-2 trigger is offered ONLY once the skip is armed ($23 at a pass value and the two
  //     code-forming shots in place), since it is inert otherwise.
  __INLINE__ void getAdditionalAllowedInputs(std::vector<InputSet::inputIndex_t>& allowedInputSet) override
  {
    allowedInputSet.push_back(_in_null); // waiting is always available

    const bool canAct      = (_lowMem[0x09] == 0x80);
    const int  activeShots = (_bulletY[0] != 0xF8) + (_bulletY[1] != 0xF8) + (_bulletY[2] != 0xF8);
    const bool canShoot    = canAct && (activeShots < 3);
    const bool skipArmed   = (*_gameTimer == 0x55 || *_gameTimer == 0x56) && _bulletY[0] == 0x50 && _bulletY[1] == 0x4D;

    for (const auto& in : _inputTable)
    {
      if (in.isP2 && !skipArmed) continue; // controller-2 trigger inert until the skip is armed
      if (in.hasA && !canAct) continue;    // jump impossible in this state
      if (in.hasB && !canShoot) continue;  // locked state, or shot cap reached
      allowedInputSet.push_back(in.idx);
    }
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    _emulator->advanceState(input);
    _currentStep++;
    updateProgress();
  }

  // Latched progression state (all serialized). These only ever advance, so any reward built from them
  // is monotonically non-decreasing along ANY trajectory -- verified against the reference solution,
  // whose per-step reward rises smoothly from stage entry to the ending with no dips. The raw signals
  // (camera scroll, bullet Ys, fire-delay counters) are noisy/transient during stage traversal, so we
  // latch furthest-progress rather than reward the instantaneous value.
  __INLINE__ void updateProgress()
  {
    const bool     inGame = (*_gameMode == 0x01 || *_gameMode == 0x0B);
    const uint16_t cam    = (uint16_t)((uint16_t)_lowMem[0x1B] * 256u + _lowMem[0x1A]);

    // Camera scroll = stage-traversal progress. Ignore the pre-load artifact (a stale high camera value
    // from the previous scene) until the stage actually loads and the scroll resets near 0. A run that
    // BEGINS mid-stage (seeded state) has no artifact and no reset to observe, so on the first update we
    // adopt the current camera as the progress baseline.
    if (_progressInit == 0)
    {
      _progressInit = 1;
      if (inGame && cam >= 50 && cam <= 1024)
      {
        _camStarted = 1;
        _maxCam     = cam;
      }
    }
    if (inGame && cam < 50) _camStarted = 1;
    if (_camStarted && inGame && cam <= 1024 && cam > _maxCam) _maxCam = cam;

    // Arming signals are only meaningful once Rockman has reached the setup spot (camera near its max).
    if (_maxCam < _setupCamThreshold) return;

    // Bullet-arming proximity: pull the two code-forming shots to their target Ys ($602->0x50 forms the
    // BVC opcode with $603->0x4D). Bullet Y is static once fired (horizontal flight) so, unlike the
    // ticking counters, it is NOT phase-invariant -- credit it only in the pass approach window ($23
    // near 0x56), otherwise a shot parked at 0x50 at any random $23 is falsely rewarded. Latched furthest.
    const int t23 = (int)*_gameTimer;
    if (t23 >= 0x40 && t23 <= 0x56)
    {
      uint16_t bulProx = 0;
      if (_bulletY[0] != 0xF8) bulProx += (uint16_t)(64 - std::min(std::abs((int)_bulletY[0] - 0x50), 64));
      if (_bulletY[1] != 0xF8) bulProx += (uint16_t)(64 - std::min(std::abs((int)_bulletY[1] - 0x4D), 64));
      if (bulProx > _maxBulletProx) _maxBulletProx = bulProx;
    }

    // Fire-delay PHASE-ALIGNMENT (the skip's two-clock coincidence): the counters must equal target on
    // the same frame $23==0x56, so what matters is the OFFSET (counter - $23) matching (target - 0x56).
    // This is phase-invariant -- high ONLY when genuinely on track for the skip, unfarmable by a random
    // state whose counters merely pass through target at an unrelated $23. Latched furthest so the reward
    // advances monotonically (the reference's pause events transiently shift the offset before re-tuning).
    uint16_t align = 0;
    for (int i = 0; i < _fireDelayCount; i++)
    {
      const int offNow = (((int)_lowMem[_fireDelayAddr[i]] - t23) % 256 + 256) % 256;
      const int offTgt = (((int)_fireDelayValue[i] - 0x56) % 256 + 256) % 256;
      int       d      = std::abs(offNow - offTgt);
      if (d > 128) d = 256 - d;
      align += (uint16_t)(128 - d);
    }
    if (align > _maxAlign) _maxAlign = align;

    // Accumulation: number of active delay-object slots ($06F0[0..6]), which climbs 2->7 as half-enemies
    // are accumulated and converted. Latched furthest -- the setup-phase progress toward an armable stock.
    uint8_t accum = 0;
    for (int i = 0; i < 7; i++)
      if (_lowMem[0x6F0 + i] != 0) accum++;
    if (accum > _maxAccum) _maxAccum = accum;

    // Shots fired: firing a bullet is the ACTION that removes an enemy's bottom half (essential to the
    // accumulation mechanic), so reward it directly. A new shot = a bullet slot ($602-$604) transitioning
    // inactive(0xF8)->active. Count them cumulatively (latched) so the search is rewarded for shooting.
    uint8_t nowActive = 0;
    for (int i = 0; i < 3; i++)
    {
      const bool a = (_bulletY[i] != 0xF8);
      if (a) nowActive |= (uint8_t)(1u << i);
      if (a && (_prevBulletActive & (1u << i)) == 0) _shotsFired++; // inactive -> active = a new shot
    }
    _prevBulletActive = nowActive;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    // Dedup hash. The game-end-glitch state lives in the object system ($0480 X array, $0600 Y array,
    // $0650 fire-delay counters) plus the zero-page game logic, and every one of those distinctions can
    // matter to the arming -- so we keep the whole object page and the game-logic half of the zero page,
    // trimming only the noisy upper zero page ($00A0-$00FF: sound/RNG scratch) that inflated the state
    // count without affecting the glitch. The long-window frontier is bounded by the reward GRADIENT
    // (fire-delay/bullet magnets) via reward-guided eviction, not by coarse hashing (which prunes the
    // winning path -- verified).
    hashEngine.Update(&_lowMem[0x0000], 0xA0);  // zero-page game logic ($00-$9F)
    hashEngine.Update(&_lowMem[0x0480], 0x20);  // object X array (incl. bullets)
    hashEngine.Update(&_lowMem[0x0600], 0x100); // object system: Y array + fire-delay + object fields
  }

  __INLINE__ void stateUpdatePostHook() override {}

  __INLINE__ void ruleUpdatePreHook() override
  {
    _camProgressMagnet = 0.0f;
    _camCap            = 65535;
    _bulletArmMagnet   = 0.0f;
    _fireDelayMagnet   = 0.0f;
    _skipReadinessMagnet = 0.0f;
    _coexistBase         = 0.0f;
    _accumMagnet         = 0.0f;
    _shotFiredMagnet     = 0.0f;
    _leverBulletA = _leverBulletB = _leverSpawn650 = _leverSpawn653 = 0.0f;
    _bulletInstMagnet = _fireDelayInstMagnet = _objectAliveMagnet = 0.0f;
    _playerXMagnet = _playerYMagnet = _playerXCenter = _playerYCenter = 0.0f;
  }

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_progressInit, sizeof(_progressInit));
    serializer.pushContiguous(&_camStarted, sizeof(_camStarted));
    serializer.pushContiguous(&_maxCam, sizeof(_maxCam));
    serializer.pushContiguous(&_maxBulletProx, sizeof(_maxBulletProx));
    serializer.pushContiguous(&_maxFireDelayProx, sizeof(_maxFireDelayProx));
    serializer.pushContiguous(&_maxAlign, sizeof(_maxAlign));
    serializer.pushContiguous(&_maxAccum, sizeof(_maxAccum));
    serializer.pushContiguous(&_shotsFired, sizeof(_shotsFired));
    serializer.pushContiguous(&_prevBulletActive, sizeof(_prevBulletActive));
    serializer.pushContiguous(&_leverFlags, sizeof(_leverFlags));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer) override
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_progressInit, sizeof(_progressInit));
    deserializer.popContiguous(&_camStarted, sizeof(_camStarted));
    deserializer.popContiguous(&_maxCam, sizeof(_maxCam));
    deserializer.popContiguous(&_maxBulletProx, sizeof(_maxBulletProx));
    deserializer.popContiguous(&_maxFireDelayProx, sizeof(_maxFireDelayProx));
    deserializer.popContiguous(&_maxAlign, sizeof(_maxAlign));
    deserializer.popContiguous(&_maxAccum, sizeof(_maxAccum));
    deserializer.popContiguous(&_shotsFired, sizeof(_shotsFired));
    deserializer.popContiguous(&_prevBulletActive, sizeof(_prevBulletActive));
    deserializer.popContiguous(&_leverFlags, sizeof(_leverFlags));
  }

  __INLINE__ float calculateGameSpecificReward() const override
  {
    float reward = 0.0;

    // Position magnet: stage-traversal progress = furthest camera scroll reached (latched), capped at the
    // reference movie's endpoint (_camCap, default none). A SMALL rightward push fosters new enemies
    // spawning as the screen scrolls (feeding the accumulation); the cap stops rewarding overshoot past
    // where the reference performs the skip.
    reward += _camProgressMagnet * (float)std::min<uint16_t>(_maxCam, _camCap);

    // Bullet magnet: furthest progress bringing the two code-forming shots to their target Ys, credited
    // only in the pass window (see updateProgress). Latched.
    reward += _bulletArmMagnet * (float)_maxBulletProx;

    // Fire-delay PHASE-ALIGNMENT magnet (the fix): furthest on-track-ness reached toward the skip's
    // two-clock coincidence (see updateProgress). Latched, so it advances monotonically.
    reward += _fireDelayMagnet * (float)_maxAlign;

    // PRECISE SKIP-READINESS (instantaneous): the skip needs all 5 delay-object fire-delay slots
    // ($651-$655) to COEXIST and be phase-aligned to hit their targets (AD,22,1F,6C,18) on the same
    // frame $23 reaches a pass (0x56). The reference achieves this only at frame 1929; the earlier
    // passes have 0-1 of the 5 slots populated (the delay-objects flicker in and out). So each slot is
    // credited ONLY when populated (a delay-object exists there), scaled by its phase alignment -- this
    // rewards creating, keeping, AND phasing the objects together, and is maximized exactly at a fully
    // armed pass. Plus proximity of the two code-forming shots ($602->0x50, $603->0x4D). Instantaneous:
    // it measures current armed-ness (the skip requires simultaneity), guiding a from-level-start search
    // to the earliest fully-armed state.
    if (std::abs(_skipReadinessMagnet) > 0.0f)
    {
      static constexpr int     sa[5] = {0x651, 0x652, 0x653, 0x654, 0x655};
      static constexpr uint8_t sv[5] = {0xAD, 0x22, 0x1F, 0x6C, 0x18};
      const int                t23   = (int)*_gameTimer;
      float                    sr    = 0.0f;
      for (int i = 0; i < 5; i++)
      {
        if (_lowMem[sa[i]] == 0) continue; // slot empty -> no delay-object present -> no credit
        int offNow = (((int)_lowMem[sa[i]] - t23) % 256 + 256) % 256;
        int offTgt = (((int)sv[i] - 0x56) % 256 + 256) % 256;
        int d      = std::abs(offNow - offTgt);
        if (d > 128) d = 256 - d;
        sr += _coexistBase + (float)(128 - d); // coexistence (populated) + phase alignment
      }
      if (_bulletY[0] != 0xF8) sr += 0.5f * (float)(64 - std::min(std::abs((int)_bulletY[0] - 0x50), 64));
      if (_bulletY[1] != 0xF8) sr += 0.5f * (float)(64 - std::min(std::abs((int)_bulletY[1] - 0x4D), 64));
      reward += _skipReadinessMagnet * sr;
    }

    // ACCUMULATION reward: the skip is armed by accumulating delay-objects (the reference shoots the
    // bottom half of two-part enemies so the top half survives and follows/scrolls with the screen, then
    // converts the accumulated half-enemies into delay-objects ~frame 1700). Those delay-objects surface
    // as active slots in the $06D0/$06F0 parallel arrays, whose count climbs 2 -> 7 across the stage. This
    // is the setup-phase gradient the arming reward lacks (the $0650 timers only complete at the arming):
    // rewarding the number of accumulated delay-objects drives the search to shoot enemies and build the
    // stockpile that a later pass can arm. Latched furthest so it monotonically rewards a bigger stockpile.
    reward += _accumMagnet * (float)_maxAccum; // latched in updateProgress()

    // Shots fired: firing bullets at enemies (to strip their bottom half) is essential to the mechanic, so
    // reward the cumulative number of shots taken. Latched/monotonic; small weight so it nudges the search
    // to shoot without competing with the accumulation term that rewards the productive (enemy-hitting) shots.
    reward += _shotFiredMagnet * (float)_shotsFired; // latched in updateProgress()

    // Optional fine positioning magnet toward a target pixel (X=$22, Y=$25); off unless configured.
    if (std::abs(_playerXMagnet) > 0.0f) reward -= _playerXMagnet * std::abs((float)*_playerPosX2 - _playerXCenter);
    if (std::abs(_playerYMagnet) > 0.0f) reward -= _playerYMagnet * std::abs((float)*_playerPosY1 - _playerYCenter);

    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]   + Game Mode / Timer:        %02u / %02u\n", *_gameMode, *_gameTimer);
    jaffarCommon::logger::log("[J+]   + Pause State:              %02u\n", *_pauseState);
    jaffarCommon::logger::log("[J+]   + Player HP:                %02u\n", *_playerHP);
    jaffarCommon::logger::log("[J+]   + Player Pos X (1/2/3):     %02u / %02u / %02u\n", *_playerPosX1, *_playerPosX2, *_playerPosX3);
    jaffarCommon::logger::log("[J+]   + Player Pos Y (1/2):       %02u / %02u\n", *_playerPosY1, *_playerPosY2);
    jaffarCommon::logger::log("[J+]   + Player Dir / Vel X:       %02u / %02u\n", *_playerDir, *_playerVelX);
    jaffarCommon::logger::log("[J+]   + Camera X (1/2):           %02u / %02u\n", *_cameraX1, *_cameraX2);
    jaffarCommon::logger::log("[J+]   + Special Frame / Object0:  %02u / %02u\n", *_specialFrame, *_object0);
    jaffarCommon::logger::log("[J+]   + Progress: maxCam=%u bulProx=%u fdProx=%u levers=0x%02X\n", _maxCam, _maxBulletProx, _maxFireDelayProx, _leverFlags);
  }

  // Per-step diagnostic line (jaffar-player --dumpTrace). Used to characterize the game-end glitch:
  // watch Game Mode / Special Frame / player state as the controller-2 trigger fires.
  __INLINE__ std::string getTraceLine() const override
  {
    char buf[300];
#ifdef _NESHAWK_DETECT_BAD_ACCESS
    const unsigned bad = (_cpuBadAccess != nullptr) ? *_cpuBadAccess : 0;
#else
    const unsigned bad = 0;
#endif
    snprintf(buf, sizeof(buf),
             "mode=%u timer=%u pause=%u hp=%u posX=%u.%u.%u posY=%u.%u dir=%u velX=%u camX=%u.%u special=%u lag=%u obj0=%u bad=%u maxCam=%u bulProx=%u fdProx=%u lev=%02X",
             *_gameMode, *_gameTimer, *_pauseState, *_playerHP, *_playerPosX1, *_playerPosX3, *_playerPosX2, *_playerPosY2, *_playerPosY1, *_playerDir, *_playerVelX, *_cameraX2,
             *_cameraX1, *_specialFrame, *_lagFrameState, *_object0, bad, _maxCam, _maxBulletProx, _maxFireDelayProx, _leverFlags);
    return std::string(buf);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    // Position magnet: stage-traversal progress (latched furthest camera scroll).
    if (actionType == "Set Camera Progress Magnet")
    {
      auto     intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      uint16_t cap       = actionJs.contains("Cap") ? jaffarCommon::json::getNumber<uint16_t>(actionJs, "Cap") : (uint16_t)65535;
      rule.addAction([=, this]() {
        this->_camProgressMagnet = intensity;
        this->_camCap            = cap;
      });
      recognizedActionType = true;
    }

    // Bullet magnet: latched furthest progress bringing the code-forming bullet slots to their Ys.
    if (actionType == "Set Bullet Arm Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_bulletArmMagnet = intensity; });
      recognizedActionType = true;
    }

    // Fire-delay magnet: latched furthest progress of the $651-$655 arming counters toward target.
    if (actionType == "Set Fire Delay Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_fireDelayMagnet = intensity; });
      recognizedActionType = true;
    }

    // Precise skip-readiness magnet: instantaneous coexistence + phase alignment of the 5 arming slots
    // (+ bullet code-byte proximity). "Coexist Base" weights each populated slot's presence vs its phase.
    if (actionType == "Set Skip Readiness Magnet")
    {
      auto intensity   = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto coexistBase = jaffarCommon::json::getNumber<float>(actionJs, "Coexist Base");
      rule.addAction([=, this]() {
        this->_skipReadinessMagnet = intensity;
        this->_coexistBase         = coexistBase;
      });
      recognizedActionType = true;
    }

    // Accumulation magnet: reward for the number of accumulated delay-objects ($06F0 active slots, 2->7),
    // the setup-phase gradient (shoot enemy bottoms -> half-enemies follow -> convert to delay-objects).
    if (actionType == "Set Accumulation Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_accumMagnet = intensity; });
      recognizedActionType = true;
    }

    // Shots fired: reward the cumulative number of bullets fired (essential enemy-stripping action).
    if (actionType == "Set Shot Fired Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      rule.addAction([=, this]() { this->_shotFiredMagnet = intensity; });
      recognizedActionType = true;
    }

    // Discrete manipulation-lever rewards (each latched once its milestone is reached).
    if (actionType == "Set Lever Rewards")
    {
      auto bulletA  = jaffarCommon::json::getNumber<float>(actionJs, "Bullet A");
      auto bulletB  = jaffarCommon::json::getNumber<float>(actionJs, "Bullet B");
      auto spawn650 = jaffarCommon::json::getNumber<float>(actionJs, "Spawn 650");
      auto spawn653 = jaffarCommon::json::getNumber<float>(actionJs, "Spawn 653");
      rule.addAction(
          [=, this]()
          {
            this->_leverBulletA  = bulletA;
            this->_leverBulletB  = bulletB;
            this->_leverSpawn650 = spawn650;
            this->_leverSpawn653 = spawn653;
          });
      recognizedActionType = true;
    }

    // Fine per-frame gradient: instantaneous bullet/fire-delay proximity + delay-object creation.
    if (actionType == "Set Fine Gradient")
    {
      auto bullet      = jaffarCommon::json::getNumber<float>(actionJs, "Bullet");
      auto fireDelay   = jaffarCommon::json::getNumber<float>(actionJs, "Fire Delay");
      auto objectAlive = jaffarCommon::json::getNumber<float>(actionJs, "Object Alive");
      rule.addAction(
          [=, this]()
          {
            this->_bulletInstMagnet    = bullet;
            this->_fireDelayInstMagnet = fireDelay;
            this->_objectAliveMagnet   = objectAlive;
          });
      recognizedActionType = true;
    }

    if (actionType == "Set Player Position Magnet")
    {
      auto intensity = jaffarCommon::json::getNumber<float>(actionJs, "Intensity");
      auto xCenter   = jaffarCommon::json::getNumber<float>(actionJs, "X Center");
      auto yCenter   = jaffarCommon::json::getNumber<float>(actionJs, "Y Center");
      rule.addAction(
          [=, this]()
          {
            this->_playerXMagnet = intensity;
            this->_playerYMagnet = intensity;
            this->_playerXCenter = xCenter;
            this->_playerYCenter = yCenter;
          });
      recognizedActionType = true;
    }

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override { return jaffarCommon::hash::hash_t(); }

  // Input alphabet (P1 pad strings; controller-2 trigger is L+R+Select)
  // Full superset of every P1 input the reference solution uses (verified against gameEndGlitch.sol),
  // so the search can reproduce the reference and the reference reward floor is reachable. Includes the
  // A (jump) family -- ...R...A / .......A / ..L....A -- and P1 Select/Start, which the setup relies on.
  const std::vector<std::string> _p1Alphabet  = {".......A", "......B.", ".DL.....", "..L.....", "..L....A", "..L...B.", "..L...BA",
                                                 "..LR....", "...R....", "...R...A", "...R..B.", "...R..BA", ".....s..", "....S..."};
  const std::vector<std::string> _p2TriggerP1 = {"........", ".DL.....", "..L.....", "...R...."};

  // Registered input + per-situation-pruning metadata (see getAdditionalAllowedInputs).
  struct InputInfo
  {
    InputSet::inputIndex_t idx;
    bool                   hasA;
    bool                   hasB;
    bool                   isP2;
  };
  std::vector<InputInfo> _inputTable;

  // Fire-delay proximity target: the counter bytes $0651,$0652,$0654,$0655 must tick up to
  // (AD,22,6C,18) as $23 hits a pass (the 5th, $0653, is set at the spawn, not ticked -- so it is a
  // discrete lever, not part of the smooth proximity). These evolve ~+1 every 2 frames.
  static constexpr int     _fireDelayCount                  = 4;
  static constexpr int     _fireDelayAddr[_fireDelayCount]  = {0x651, 0x652, 0x654, 0x655};
  static constexpr uint8_t _fireDelayValue[_fireDelayCount] = {0xAD, 0x22, 0x6C, 0x18};

  // Camera-scroll threshold marking "Rockman has reached the setup spot"; arming rewards gate on it so
  // transient mid-stage bullets/counters don't fire them. Config-tunable ("Setup Cam Threshold").
  uint16_t _setupCamThreshold = 780;

  // Discrete manipulation-lever bits (latched in _leverFlags).
  static constexpr uint8_t LEVER_BULLET_A  = 0x01; // slot-2 bullet reached Y=0x50 (BVC opcode byte)
  static constexpr uint8_t LEVER_BULLET_B  = 0x02; // slot-3 bullet reached Y=0x4D (branch target)
  static constexpr uint8_t LEVER_SPAWN_650 = 0x04; // $0650 == 0x3C
  static constexpr uint8_t LEVER_SPAWN_653 = 0x08; // $0653 == 0x1F

  // Config magnet weights (set by rule actions, reset each rule pass)
  float    _camProgressMagnet = 0.0f;
  uint16_t _camCap            = 65535; // rightward-push cap (reference endpoint); no cap by default
  float    _bulletArmMagnet   = 0.0f;
  float _fireDelayMagnet   = 0.0f;
  float _skipReadinessMagnet = 0.0f; // precise instantaneous coexistence+phase reward
  float _coexistBase         = 0.0f; // per-populated-slot base (weights presence vs phase)
  float _accumMagnet         = 0.0f; // reward for accumulated delay-objects ($06F0 active count)
  float _leverBulletA = 0.0f, _leverBulletB = 0.0f, _leverSpawn650 = 0.0f, _leverSpawn653 = 0.0f;
  float _bulletInstMagnet = 0.0f, _fireDelayInstMagnet = 0.0f, _objectAliveMagnet = 0.0f; // fine per-frame gradient
  float _playerXMagnet = 0.0f, _playerYMagnet = 0.0f, _playerXCenter = 0.0f, _playerYCenter = 0.0f;

  // Latched progression state (serialized; only ever advances -> monotonic reward)
  uint8_t  _progressInit     = 0;
  uint8_t  _camStarted       = 0;
  uint16_t _maxCam           = 0;
  uint16_t _maxBulletProx    = 0;
  uint16_t _maxFireDelayProx = 0;
  uint16_t _maxAlign         = 0;
  uint8_t  _maxAccum         = 0;
  uint16_t _shotsFired       = 0; // cumulative bullets fired (latched)
  uint8_t  _prevBulletActive = 0; // bitmask of bullet slots active last frame (edge-detect new shots)
  float    _shotFiredMagnet  = 0.0f;
  uint8_t  _leverFlags       = 0;

  // Cached property pointers
  uint8_t* _gameTimer;
  uint8_t* _gameMode;
  uint8_t* _pauseState;
  uint8_t* _cameraX1;
  uint8_t* _cameraX2;
  uint8_t* _playerPosX1;
  uint8_t* _playerPosX2;
  uint8_t* _playerPosX3;
  uint8_t* _playerPosY1;
  uint8_t* _playerPosY2;
  uint8_t* _playerDir;
  uint8_t* _playerHP;
  uint8_t* _playerVelX;
  uint8_t* _specialFrame;
  uint8_t* _lagFrameState;
  uint8_t* _object0;
  uint8_t* _bulletX;
  uint8_t* _bulletY;
#ifdef _NESHAWK_DETECT_BAD_ACCESS
  uint8_t* _cpuBadAccess = nullptr;
#endif

  // Input indices
  InputSet::inputIndex_t _in_null;

  // Per-state step counter (serialized)
  uint32_t _currentStep = 0;

  // Pointer to emulator's low memory
  uint8_t* _lowMem;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus
