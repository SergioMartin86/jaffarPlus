#pragma once

#include <cstdlib>
#include <emulator.hpp>
#include <fstream>
#include <game.hpp>
#include <jaffarCommon/json.hpp>
#include <numeric>
#include <vector>

namespace jaffarPlus
{

namespace games
{

namespace nes
{

class IceClimber final : public jaffarPlus::Game
{
public:
  static __INLINE__ std::string getName() { return "NES / Ice Climber"; }

  IceClimber(std::unique_ptr<Emulator> emulator, const nlohmann::json& config) : jaffarPlus::Game(std::move(emulator), config)
  {
    // No game-specific configuration keys; reject any leftover (unrecognized) key in the game configuration.
  }

private:
  __INLINE__ void registerGameProperties() override
  {
    // Getting emulator's low memory pointer
    _lowMem = _emulator->getProperty("LRAM").pointer;

    // Registering native game properties

    registerGameProperty("Current Level", &_lowMem[0x0059], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos X", &_lowMem[0x0064], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Pos Y", &_lowMem[0x0066], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y", &_lowMem[0x0013], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y1", &_lowMem[0x008A], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y2", &_lowMem[0x008D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Screen Scroll Y3", &_lowMem[0x0090], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Floor", &_lowMem[0x0343], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Blocks Hit", &_lowMem[0x0361], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Vegetables Collected", &_lowMem[0x0362], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Nitpickers Hit", &_lowMem[0x0363], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bricks Broken", &_lowMem[0x0364], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 1", &_lowMem[0x0014], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Inputs 2", &_lowMem[0x0015], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Global Timer", &_lowMem[0x0050], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("SubTimer", &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("SubTimer", &_lowMem[0x0031], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bonus Stage Start", &_lowMem[0x0027], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Grabbed Bird", &_lowMem[0x004D], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Jumping", &_lowMem[0x00E0], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Player Is Dead", &_lowMem[0x0382], Property::datatype_t::dt_uint8, Property::endianness_t::little);

    registerGameProperty("Bird State", &_lowMem[0x00D3], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bird Pos X", &_lowMem[0x0243], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Bird Direction", &_lowMem[0x00D5], Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Is Bird Detected", &_isBirdDetected, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Is Condor Active", &_isCondorActive, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Left Bonus", &_leftBonus, Property::datatype_t::dt_uint8, Property::endianness_t::little);
    registerGameProperty("Min Bird Dist", &_minBirdDist, Property::datatype_t::dt_float32, Property::endianness_t::little);

    registerGameProperty("Player Real Pos Y", &_playerRealPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Best Pos Y", &_playerBestPosY, Property::datatype_t::dt_float32, Property::endianness_t::little);
    registerGameProperty("Player Real/Best Pos Y Diff", &_playerRealBestPosYDiff, Property::datatype_t::dt_float32, Property::endianness_t::little);

    _currentLevel        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Current Level")]->getPointer();
    _playerPosX          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos X")]->getPointer();
    _playerPosY          = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Pos Y")]->getPointer();
    _screenScrollY       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y")]->getPointer();
    _screenScrollY1      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y1")]->getPointer();
    _screenScrollY2      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y2")]->getPointer();
    _screenScrollY3      = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Screen Scroll Y3")]->getPointer();
    _playerFloor         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Floor")]->getPointer();
    _playerJumping       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Jumping")]->getPointer();
    _blocksHit           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Blocks Hit")]->getPointer();
    _vegetablesCollected = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Vegetables Collected")]->getPointer();
    _nitpickersHit       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Nitpickers Hit")]->getPointer();
    _bricksBroken        = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bricks Broken")]->getPointer();
    _playerInputs1       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 1")]->getPointer();
    _playerInputs2       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Player Inputs 2")]->getPointer();
    _globalTimer         = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Global Timer")]->getPointer();
    _subTimer            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("SubTimer")]->getPointer();
    _birdState           = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bird State")]->getPointer();
    _birdPosX            = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bird Pos X")]->getPointer();
    _birdDirection       = (uint8_t*)_propertyMap[jaffarCommon::hash::hashString("Bird Direction")]->getPointer();

    _pastBricksBroken  = 0;
    _isBirdDetected    = 0;
    _isCondorActive    = 0;
    _minBirdDist       = std::numeric_limits<float>::infinity();
    _fullScreenScrolls = 0;
    _playerBestPosY    = std::numeric_limits<float>::infinity();
    _bestReachY        = std::numeric_limits<float>::infinity();
    // Sweep override: if any ICE_W* env var is set, it pins the weights (used by the fast bonus
    // harness to tune a level). Otherwise the per-level table (levelStrat, keyed on $59) supplies them.
    if (auto* e = std::getenv("ICE_WDIST"))
    {
      _wDist      = std::stof(e);
      _envWeights = true;
    }
    if (auto* e = std::getenv("ICE_WAWAY"))
    {
      _wAway      = std::stof(e);
      _envWeights = true;
    }
    if (auto* e = std::getenv("ICE_WSTAND"))
    {
      _wStand     = std::stof(e);
      _envWeights = true;
    }
    // LEVEL INJECTION: pin the level counter ($59)/completed-count ($56) so a search whose Initial
    // Sequence ends just before a LEVEL TRANSITION loads ANY mountain (the transition's terrain-gen
    // reads the pinned $59). Works at a transition -- NOT at boot, where terrain-gen happens inside the
    // atomic initial-sequence replay (which bypasses this hook). 0-based ($59=N => mountain N+1); -1=off.
    if (auto* e = std::getenv("ICE_INJECT_LEVEL")) _injectLevel = std::stoi(e);
    // Load the imitation seed trajectory (per-frame "posX posY ..." waypoints) if ICE_REF is set.
    if (auto* e = std::getenv("ICE_REF"))
    {
      std::ifstream f(e);
      std::string   line;
      while (std::getline(f, line))
      {
        if (line.empty() || line[0] == '#') continue;
        int x, y;
        if (std::sscanf(line.c_str(), "%d %d", &x, &y) == 2)
        {
          _refX.push_back(x);
          _refY.push_back(y);
        }
      }
      _refN      = _refX.size();
      _refLoaded = _refN > 0;
    }
    updateRealPosY();

    // Initializing values
    _currentStep   = 0;
    _lastInputStep = 0;

    // Getting index for a non input
    _nullInputIdx = _emulator->registerInput("|..|........|");
  }

  __INLINE__ void updateRealPosY()
  {
    _playerRealPosY = (float)*_playerPosY;
    if (_playerRealPosY > 245.0) _playerRealPosY -= 255.0;
    _playerRealPosY += (float)*_screenScrollY;
    _playerRealPosY -= (float)_fullScreenScrolls * 240.0; // vertical scroll wraps at 240 (0..239), verified empirically: scroll 3->239 across a full screen
    _playerRealBestPosYDiff = _playerRealPosY - _playerBestPosY;

    // Airborne-inclusive best height ever reached. This is the dense climb gradient: to gain a
    // floor the player must jump up, bonk/break the ceiling, fall back, and jump through. A pure
    // *standing* best is flat through that whole maneuver, giving the search no signal. The jump
    // peak, in contrast, rises every time the ceiling opens a little further, so rewarding the
    // best reach pulls the search toward making the hole (and through it).
    if (_playerRealPosY < _bestReachY) _bestReachY = _playerRealPosY;

    // Standing best (secured height). Kept SEPARATE from _bestReachY so the fall-back prune
    // (realY - standingBest) tolerates the dip back to the current floor during hole-making.
    if (*_playerJumping == 0)
      if (_playerRealPosY < _playerBestPosY)
      {
        _playerBestPosY   = _playerRealPosY;
        _pastBricksBroken = *_bricksBroken;
      }
  }

  __INLINE__ void advanceStateImpl(const InputSet::inputIndex_t input) override
  {
    // Remembering current screen scroll
    auto curScroll = *_screenScrollY;

    _emulator->advanceState(input);

    // Increasing counter if input is null
    if (input != _nullInputIdx) _lastInputStep = _currentStep;
    _currentStep++;

    // Getting new scroll
    auto newScroll = *_screenScrollY;

    // If new scroll is larger than current, a new cycle is made
    if (newScroll > curScroll) _fullScreenScrolls++;
  }

  __INLINE__ void computeAdditionalHashing(MetroHash128& hashEngine) const override
  {
    hashEngine.Update(&_lowMem[0x0000], 0x000A - 0x0000); //
    hashEngine.Update(_lowMem[0x000F]);                   // No idea but seems important
    hashEngine.Update(&_lowMem[0x0051], 0x009D - 0x0051); // Skipping timers
    hashEngine.Update(&_lowMem[0x00A9], 0x00F6 - 0x00A9);
    hashEngine.Update(&_lowMem[0x0100], 0x01F7 - 0x0100);
    hashEngine.Update(&_lowMem[0x01FD], 0x0217 - 0x01FD);
    hashEngine.Update(&_lowMem[0x0400], 0x04C0 - 0x0400); // Brick states

    // Bonus-stage: hash ONLY the cloud immediately above the player (the one to land on next). Without
    // any cloud in the hash, states differing only in cloud position dedup together, so the search
    // can't represent "wait for the cloud to arrive" and can't time a landing. But hashing ALL clouds
    // explodes the state space with positions of clouds we've already passed or are far above -- which
    // is irrelevant noise. Hashing just the next cloud up lets the search time THAT landing without the
    // blowup. Gated on the bonus stage so the climb (where these bytes are static layout) is untouched.
    if (_isCondorActive == 1)
    {
      int pY  = (int)_lowMem[0x0066];
      int tgt = -1, tgtY = -1000;
      for (int i = 0; i < 4; i++)
      {
        if (_lowMem[0x0786 + i] == 0) continue;
        int cY = (int)_lowMem[0x06BE + i] + 32;
        if (cY < pY - 1 && cY > tgtY)
        {
          tgtY = cY;
          tgt  = i;
        }
      }
      // Hash the next cloud's X QUANTIZED to 8px buckets: the cloud is ~48px wide so the search only
      // needs its general screen position to time a landing, not the exact pixel -- this collapses
      // 256 cloud positions to 32, keeping the timing-relevant state space small.
      if (tgt >= 0)
      {
        uint8_t q = (uint8_t)(_lowMem[0x0682 + tgt] >> 3);
        hashEngine.Update(q);
      }
    }
  }

  // Updating derivative values after updating the internal state
  __INLINE__ void stateUpdatePostHook() override
  {
    if (_injectLevel >= 0)
    {
      _lowMem[0x0059] = (uint8_t)_injectLevel;
      _lowMem[0x0056] = (uint8_t)_injectLevel;
    }
    // "Left Bonus" latch: set once the player is NOT in a bonus (condor not flying). Lets a climb's win
    // rule require D7==21 AGAIN *after* a climb -- so an injected search starting at the previous
    // mountain's bonus-end doesn't win instantly, and a normal climb (starts at D7!=21) still works.
    // On FIRST entering the climb, reset the climb-Y/condor tracking: an injected search inherits the
    // previous mountain's scroll-wrap count and best-Y, which would corrupt this climb's realY/reward.
    if (_lowMem[0x00D7] != 21)
    {
      if (_leftBonus == 0)
      {
        _fullScreenScrolls = 0;
        _playerBestPosY    = std::numeric_limits<float>::infinity();
        _bestReachY        = std::numeric_limits<float>::infinity();
        _isBirdDetected    = 0;
        _isCondorActive    = 0;
        _minBirdDist       = std::numeric_limits<float>::infinity();
      }
      _leftBonus = 1;
    }
    updateRealPosY();

    // Select this level's bonus-stage reward weights from the per-level table (unless a sweep pinned
    // them via env). Keyed on $59 so the global multi-mountain run auto-switches per level.
    if (!_envWeights)
    {
      auto s  = levelStrat(_lowMem[0x0059]);
      _wDist  = s.wDist;
      _wAway  = s.wAway;
      _wStand = s.wStand;
    }

    // Advance the imitation seed: bump _refIdx (monotonic) to the FURTHEST upcoming reference waypoint
    // the player is currently near, scanning a small look-ahead window so frameskip (which skips frames
    // the per-frame reference visited) doesn't stall progress. The reward then pays for _refIdx, so the
    // beam is pulled along the demonstration.
    if (_refLoaded)
    {
      int    px = (int)_lowMem[0x0064], py = (int)_lowMem[0x0066];
      size_t limit = std::min((size_t)_refIdx + 8, _refN);
      for (size_t k = (size_t)_refIdx + 1; k < limit; k++)
        if (std::abs(px - _refX[k]) + std::abs(py - _refY[k]) <= 8) _refIdx = (uint16_t)k;
    }

    // The condor ("giant bird") is the grab target. Its position is $D6 (X) / $D7 (Y); once the
    // player nears the top it flies at a fixed top-screen altitude ($D7 == 21), circling and
    // wrapping around in X. Player ($64,$66) and condor ($D6,$D7) are BOTH in the same settled
    // top-screen coordinate frame, so the grab distance is computed in SCREEN coords -- NOT the
    // player's continuous climb-Y. (The old logic used $0243, which stays 112 until the grab
    // itself, and $D3, which only animates at the grab -- both useless for guiding the chase.)
    _isCondorActive = (_lowMem[0x00D7] == 21) ? 1 : 0;
    if (_isCondorActive) _isBirdDetected = 1; // latched flag kept for the config's reward rule
    if (_isCondorActive)
    {
      _distanceToBirdX = std::abs((float)*_playerPosX - (float)_lowMem[0x00D6]);
      if (_distanceToBirdX > 128.0f) _distanceToBirdX = 256.0f - _distanceToBirdX; // condor X wraps
      _distanceToBirdY = std::abs((float)*_playerPosY - (float)_lowMem[0x00D7]);
      _distanceToBird  = sqrt(_distanceToBirdX * _distanceToBirdX + _distanceToBirdY * _distanceToBirdY);

      // MONOTONIC closest approach. The condor circles continuously and outruns the player, so a
      // raw distance reward makes the beam chase it forever. Rewarding the closest-ever distance
      // (like the climb's _bestReachY) locks in progress: the beam keeps the best interception
      // states and drives the min toward 0 (overlap = grab), instead of following the condor around.
      if (_distanceToBird < _minBirdDist) _minBirdDist = _distanceToBird;
    }
    else { _distanceToBird = _distanceToBirdX = _distanceToBirdY = 0.0f; }
  }

  __INLINE__ void ruleUpdatePreHook() override {}

  __INLINE__ void ruleUpdatePostHook() override {}

  __INLINE__ void serializeStateImpl(jaffarCommon::serializer::Base& serializer) const override
  {
    serializer.pushContiguous(&_currentStep, sizeof(_currentStep));
    serializer.pushContiguous(&_lastInputStep, sizeof(_lastInputStep));
    serializer.pushContiguous(&_isBirdDetected, sizeof(_isBirdDetected));
    serializer.pushContiguous(&_playerRealPosY, sizeof(_playerRealPosY));
    serializer.pushContiguous(&_fullScreenScrolls, sizeof(_fullScreenScrolls));
    serializer.pushContiguous(&_playerBestPosY, sizeof(_playerBestPosY));
    serializer.pushContiguous(&_pastBricksBroken, sizeof(_pastBricksBroken));
    serializer.pushContiguous(&_playerRealBestPosYDiff, sizeof(_playerRealBestPosYDiff));
    serializer.pushContiguous(&_bestReachY, sizeof(_bestReachY));
    serializer.pushContiguous(&_minBirdDist, sizeof(_minBirdDist));
    serializer.pushContiguous(&_refIdx, sizeof(_refIdx));
    serializer.pushContiguous(&_leftBonus, sizeof(_leftBonus));
  }

  __INLINE__ void deserializeStateImpl(jaffarCommon::deserializer::Base& deserializer)
  {
    deserializer.popContiguous(&_currentStep, sizeof(_currentStep));
    deserializer.popContiguous(&_lastInputStep, sizeof(_lastInputStep));
    deserializer.popContiguous(&_isBirdDetected, sizeof(_isBirdDetected));
    deserializer.popContiguous(&_playerRealPosY, sizeof(_playerRealPosY));
    deserializer.popContiguous(&_fullScreenScrolls, sizeof(_fullScreenScrolls));
    deserializer.popContiguous(&_playerBestPosY, sizeof(_playerBestPosY));
    deserializer.popContiguous(&_pastBricksBroken, sizeof(_pastBricksBroken));
    deserializer.popContiguous(&_playerRealBestPosYDiff, sizeof(_playerRealBestPosYDiff));
    deserializer.popContiguous(&_bestReachY, sizeof(_bestReachY));
    deserializer.popContiguous(&_minBirdDist, sizeof(_minBirdDist));
    deserializer.popContiguous(&_refIdx, sizeof(_refIdx));
    deserializer.popContiguous(&_leftBonus, sizeof(_leftBonus));
  }

  __INLINE__ float calculateGameSpecificReward() const
  {
    // Getting rewards from rules
    float reward = 0.0;

    // Climb reward = SECURED (standing) height as the primary term, plus a smaller airborne-reach
    // bonus. Why the split: a pure airborne reward rewards jumping above the top of the current
    // screen into the void (posY wraps past 0) -- a dead end, since the camera only scrolls from a
    // STANDING position at posY<96. That void-peak then out-scores the land-and-scroll progression
    // and traps the beam (mountain 2 stalled at the 2nd-screen top). Making secured height dominant
    // commits the search to landing on real higher floors / triggering the next scroll, while the
    // airborne bonus still supplies the hole-making gradient (a jump that opens the ceiling reaches
    // higher). Small current-Y term breaks ties toward being high right now.
    // Climb reward: SECURED (standing) height dominant + a smaller airborne-reach bonus (the dense
    // hole-making gradient -- without it the climb stalls almost immediately). This is the recipe
    // that climbed mountains 1-2 to the top and mountain 3 to ~-365 (the grab, not the climb, is
    // the open problem there).
    // (Tried dropping the airborne term in the bonus stage to force align-then-jump; it removed the
    // drive to ascend and made things worse -- dist 69 vs 30 -- so the airborne bonus stays.)
    reward -= 1.0f * _playerBestPosY + 0.3f * _bestReachY + 0.01f * _playerRealPosY;

    // Reward making holes in the ceiling (hole-making progress).
    reward += 2.0 * (float)*_bricksBroken;

    // Condor proximity (monotonic closest approach). SMALL weight: the bonus stage is a vertical
    // SCROLLING climb and the condor rides at the top of the screen, scrolling WITH the camera, so the
    // screen-distance to it stays ~constant (~30) the whole climb -- it gives no climb gradient and,
    // worse, is reachable just by jumping (no landing), so a big weight TRAPS the search hovering
    // airborne near the condor on the first screen instead of doing the real work (land on a cloud ->
    // scroll -> repeat). Keep it as a small final-grab nudge; REAL height (below) drives the climb.
    if (_isBirdDetected == 1) reward += 0.1f * (256.0f - _minBirdDist);

    // IMITATION SEED reward (dominant when ICE_REF is loaded): pay for progress along the reference
    // demonstration (_refIdx, monotonic) plus a pull toward the next waypoint. The big per-waypoint
    // weight makes the demonstration the highest-reward path, so the beam follows it to the grab
    // (which the bare bonus reward can't reach) while still able to deviate/refine. Inert otherwise.
    if (_refLoaded)
    {
      reward += 50.0f * (float)_refIdx;
      size_t nx = std::min((size_t)_refIdx + 1, _refN - 1);
      reward -= (float)(std::abs((int)*_playerPosX - _refX[nx]) + std::abs((int)*_playerPosY - _refY[nx]));
    }

    // BONUS-STAGE CLOUD LADDER (secondary driver). The top of each mountain is a bonus stage where
    // the player must ascend by landing on moving cloud platforms before reaching the condor. There
    // are up to 4 clouds (slot i): screen span [ $0682+i , $06A0+i ] (~48px wide), screen Y =
    // $06BE+i + 32, active = $0786+i (all reverse-engineered; sub_E33C advances them each frame).
    // A pure height/condor reward gives no gradient to MEET a moving cloud, so the search jumps into
    // gaps. Here we reward horizontal alignment with the NEXT cloud above the player (the one it must
    // land on next). Kept a SIGNIFICANT SECONDARY term -- it differentiates states within a cloud
    // level (where height is ~constant) without overriding the primary height/condor drive.
    if (_isCondorActive == 1)
    {
      float pX = (float)*_playerPosX, pY = (float)*_playerPosY;
      int   tgt  = -1;
      float tgtY = -1.0e9f;
      for (int i = 0; i < 4; i++)
      {
        if (_lowMem[0x0786 + i] == 0) continue;        // inactive slot
        float cY = (float)_lowMem[0x06BE + i] + 32.0f; // cloud screen Y
        if (cY < pY - 1.0f && cY > tgtY)
        {
          tgtY = cY;
          tgt  = i;
        } // closest cloud ABOVE the player
      }
      if (tgt >= 0)
      {
        // DIRECTIONAL, WRAP-AWARE distance to the next cloud = how far the cloud must travel, IN ITS
        // DIRECTION OF MOTION (it wraps), to reach the player's X. This folds in both the wrap and the
        // direction the user asked for: a cloud APPROACHING along the short arc is near (small), the
        // SAME gap with the cloud moving AWAY is large (it must loop all the way around). Examples:
        // player 0, cloud 255 -> 1 if right-moving (wraps straight in), 255 if left-moving (full
        // traverse); player 120, cloud 140 -> 20 if left-moving (toward), 236 if right-moving (away).
        // Makes WAITING productive: an approaching cloud's distance shrinks each frame, so the reward
        // rises and the beam KEEPS the wait state -> the search can wait for the loop-around instead of
        // greedily hopping the nearest cloud (the trap). 0 when the span is currently over the player.
        // CIRCULAR distance + a MODEST away-penalty. Circular |gap| is smooth (the full directional
        // distance's -127 swing when the target cloud loops away disrupted the beam and lowered the
        // ascent). But direction still matters -- 40px toward != 40px away -- so add a small constant
        // when the cloud is moving to WIDEN the short-arc gap (looping away). Capped (not the full
        // 256-loop) so it biases toward approaching clouds without the disruptive swing.
        int  L = _lowMem[0x0682 + tgt], R = _lowMem[0x06A0 + tgt], P = (int)pX;
        bool over = (R >= L) ? (P >= L && P <= R) : (P >= L || P <= R); // span currently over player?
        int  dirD = 0;
        if (!over)
        {
          float cX  = (R >= L) ? (L + R) * 0.5f : std::fmod((L + R + 256.0f) * 0.5f, 256.0f); // center
          int   gap = (int)cX - P;
          if (gap > 128)
            gap -= 256;
          else if (gap < -128)
            gap += 256;                                           // signed short-arc, [-128,128]
          int8_t v    = (int8_t)_lowMem[0x07B7 + tgt];            // signed cloud velocity
          bool   away = (gap > 0 && v > 0) || (gap < 0 && v < 0); // cloud widening the short-arc gap
          dirD        = std::abs(gap) + (away ? (int)_wAway : 0);
        }
        reward -= _wDist * (float)dirD;
      }
      else
      {
        // No cloud above -> the player has ridden the clouds up to the top (solid) platform, and the
        // only thing left is to grab the condor. FOLLOW THE CONDOR by tracking its PREDICTED position:
        // it moves -1/+1 px per frame in direction $D5, and a grab jump takes ~LEAD frames to reach
        // its altitude, during which it slides. Reward aligning with where it WILL be, so the jump
        // intercepts it (instead of aiming where it is now and arriving late). Significant driver.
        float v     = (_lowMem[0x00D5] != 0) ? 1.0f : -1.0f;
        float predX = (float)_lowMem[0x00D6] + v * 10.0f; // lead by ~jump-rise time
        float dxc   = std::abs(pX - predX);
        if (dxc > 128.0f) dxc = 256.0f - dxc; // condor X wraps
        reward -= 1.0f * dxc;
      }

      // REWARD CLEAN CLOUD LANDINGS: standing (handler $6A==1) with the feet on an active cloud (Y
      // matches its surface, X within its span). Riding the ladder is a sequence of moving-cloud
      // landings; a horizontal-proximity reward alone leaves the player flailing airborne (handler 2)
      // and reaching cloud height by luck. Rewarding the actual STAND makes the search build the
      // c3->c2->c1->c0 ride step by step. SMALL tie-breaker only: if it's too high the beam grabs the
      // first reachable cloud-stand instead of waiting for the well-timed one (the directional term
      // rewards that wait), so keep it well below the reward gained by waiting for an approaching cloud.
      if (*_playerJumping == 0) // ON-GROUND (standing) -- NOT handler $6A==1, which also occurs mid-fall
        for (int i = 0; i < 4; i++)
        {
          if (_lowMem[0x0786 + i] == 0) continue;
          if (std::abs((int)pY - ((int)_lowMem[0x06BE + i] + 32)) > 6) continue; // feet on this surface
          int  L = _lowMem[0x0682 + i], R = _lowMem[0x06A0 + i], P = (int)pX;
          bool onSpan = (R >= L) ? (P >= L && P <= R) : (P >= L || P <= R);
          if (onSpan)
          {
            reward += _wStand;
            break;
          } // standing on a cloud
        }
    }

    // Huge bonus on the grab (= level complete), so a winning line is the global best-reward state
    // (=> best.sol is the win). Detected exactly like the win rule: grab timer $4D>0 with the
    // condor moved off-center ($0243 != 112).
    if (_lowMem[0x004D] > 0 && _lowMem[0x0243] != 112) reward += 1000000.0f;

    // Per-step time cost (=> faster solutions score higher; also orders dedup collisions toward the
    // earlier path). PHASE-CONDITIONAL, because in a reward-priority beam this weight controls how
    // tightly the frontier focuses on the leading edge:
    //   - Climbing (condor not yet seen): heavy (1.0) so the beam drops trailing states and races
    //     up the mountain. A light weight here climbs ~3x slower (beam clogs with stale states).
    //   - Grab phase (condor seen): light (0.05) so the beam KEEPS the patient states needed to
    //     wait for the slowly-circling condor to come around and intercept it. A heavy weight here
    //     starves the grab (mountain 2 exhausted its frontier before grabbing).
    reward -= (_isBirdDetected ? 0.05f : 1.0f) * (float)_currentStep;

    // Returning reward
    return reward;
  }

  void printInfoImpl() const override
  {
    jaffarCommon::logger::log("[J+]  + Player Real Pos Y:                  %.0f\n", _playerRealPosY);
    jaffarCommon::logger::log("[J+]  + Player Best Pos Y:                  %.0f\n", _playerBestPosY);
    jaffarCommon::logger::log("[J+]  + Player Best Reach Y (airborne):     %.0f\n", _bestReachY);
    jaffarCommon::logger::log("[J+]  + Player Best / Real Pos Y Diff:      %.0f\n", _playerRealBestPosYDiff);
    jaffarCommon::logger::log("[J+]  + Past Bricks Broken:                 %u\n", _pastBricksBroken);
    jaffarCommon::logger::log("[J+]  + Full Screen Scrolls:                %u\n", _fullScreenScrolls);
    if (_isBirdDetected == 1) jaffarCommon::logger::log("[J+]  + Distance To Bird:                  %.2f (%.2f, %.2f)\n", _distanceToBird, _distanceToBirdX, _distanceToBirdY);
  }

  bool parseRuleActionImpl(Rule& rule, const std::string& actionType, const nlohmann::json& actionJs) override
  {
    bool recognizedActionType = false;

    return recognizedActionType;
  }

  __INLINE__ jaffarCommon::hash::hash_t getStateInputHash() override
  {
    // There is no discriminating state element, so simply return a zero hash
    return jaffarCommon::hash::hash_t();
  }

  uint8_t* _currentLevel;
  uint8_t* _playerPosX;
  uint8_t* _playerPosY;
  uint8_t* _screenScrollY;
  uint8_t* _screenScrollY1;
  uint8_t* _screenScrollY2;
  uint8_t* _screenScrollY3;
  uint8_t* _playerFloor;
  uint8_t* _blocksHit;
  uint8_t* _vegetablesCollected;
  uint8_t* _nitpickersHit;
  uint8_t* _bricksBroken;
  uint8_t* _playerInputs1;
  uint8_t* _playerInputs2;
  uint8_t* _globalTimer;
  uint8_t* _subTimer;
  uint8_t* _birdState;
  uint8_t* _birdPosX;
  uint8_t* _lowMem;
  uint8_t  _isBirdDetected;
  uint8_t  _isCondorActive; // condor flying at top this frame ($D7==21) — gates the screen-coord chase
  float    _minBirdDist;    // closest the player has EVER come to the condor (monotonic) — drives grab
  uint8_t* _playerJumping;
  uint8_t* _birdDirection;

  float _distanceToBirdX;
  float _distanceToBirdY;
  float _distanceToBird;

  // Remember frame in which the last input was made
  uint16_t _lastInputStep;
  uint16_t _currentStep;
  uint8_t  _fullScreenScrolls;
  uint8_t  _pastBricksBroken;
  float    _playerRealPosY;
  float    _playerBestPosY; // best height reached while STANDING (secured) — drives fall-back prune
  float    _bestReachY;     // best height EVER reached incl. airborne (jump peaks) — drives reward
  float    _playerRealBestPosYDiff;

  // Active bonus-stage reward weights for the current frame (set per-level from levelStrat() unless an
  // ICE_W* env var pinned them for a sweep). ICE_WDIST=cloud distance weight, ICE_WAWAY=away-penalty,
  // ICE_WSTAND=clean-landing bonus.
  float   _wDist       = 0.5f;
  float   _wAway       = 40.0f;
  float   _wStand      = 8.0f;
  bool    _envWeights  = false; // true => env pinned the weights (sweep); skip the per-level table
  int     _injectLevel = -1;    // ICE_INJECT_LEVEL: pin $59/$56 so a pre-transition search loads any mtn
  uint8_t _leftBonus   = 0;     // latched once D7!=21 -- a climb win requires re-entering the bonus after

  // IMITATION SEED: the bonus stage's timed moving-cloud ride is unsearchable by a reward-priority beam
  // (no gradient through the multi-step ride -- it drops the path even when a solution exists). So we
  // SEED the beam with a reference demonstration trajectory (per-frame posX,posY from a real solution,
  // file in env ICE_REF) and reward FOLLOWING it: a monotonic "furthest reference waypoint reached"
  // term that dominates the bonus reward. This makes the demonstration the highest-reward path -> the
  // beam can't drop it and rides it to the grab, while still free to deviate (refine).
  std::vector<int> _refX, _refY;
  size_t           _refN      = 0;
  bool             _refLoaded = false;
  uint16_t         _refIdx    = 0; // furthest reference waypoint reached (monotonic; serialized per state)

  // PER-LEVEL STRATEGY TABLE. With only 32 mountains we hand-tune each level's bonus-stage strategy
  // (the cloud-ride is layout-specific). Keyed on the mountain index ($59), so a single-mountain
  // search uses that level's params and the global multi-mountain run auto-switches per level -- the
  // model the RNG-aware study needs. Common cloud machinery stays shared; only the params differ.
  struct LevelStrat
  {
    float wDist;
    float wAway;
    float wStand;
  };
  __INLINE__ LevelStrat levelStrat(uint8_t lvl) const
  {
    switch (lvl)
    {
      // mountain 3 ($59==2): baseline from the fast-harness sweep (to be finalized)
      case 2: return {0.5f, 40.0f, 8.0f};
      default: return {0.5f, 40.0f, 8.0f}; // general default until each level is hand-tuned
    }
  }

  // Null input index to remember the last valid input
  InputSet::inputIndex_t _nullInputIdx;
};

} // namespace nes

} // namespace games

} // namespace jaffarPlus